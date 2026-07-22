// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Pool.hpp
 * @brief  Build-time-grow-then-freeze arena for the GPU memory foundation.
 * @details A @ref EBGeometry::Pool is a single contiguous byte block obtained from a
 * @ref EBGeometry::MemoryResource, into which many sub-regions are bump-reserved during a build
 * phase and from which nothing is individually freed. It has exactly two phases:
 *
 * - @b Build: @ref EBGeometry::Pool::reserve hands out byte offsets and may grow (reallocate) the
 *   block. Because every @ref EBGeometry::PODVector stores a byte @e offset rather than a pointer,
 *   a grow that moves the block invalidates no @ref EBGeometry::PODVector -- offsets are relative
 *   to @ref EBGeometry::Pool::base, which callers re-read.
 * - @b Query: after @ref EBGeometry::Pool::freeze the block is immutable, @ref EBGeometry::Pool::base
 *   is stable, and @ref EBGeometry::Pool::mirror can copy the whole block (one @c memcpy) into
 *   another resource -- the host-to-device upload. The same offsets resolve against the new base
 *   with zero pointer patching.
 *
 * The @ref EBGeometry::Pool is the one owning, move-only, RAII (hence non-trivial) type of the
 * foundation; everything stored inside it is trivially copyable POD.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POOL_HPP
#define EBGEOMETRY_POOL_HPP

// Std includes
#include <cstddef>
#include <cstdint>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_MemoryResource.hpp"

namespace EBGeometry {

/**
 * @brief Build-time-grow-then-freeze arena over a @ref MemoryResource.
 * @details Move-only, single-owner RAII. See the file-level documentation for the two-phase
 * (build / query) contract.
 *
 * @note This is one of the two deliberately non-trivial types of the memory foundation (it owns a
 * block and a resource pointer with RAII semantics); everything stored @e inside it is trivially
 * copyable POD, but the pool itself is not, so no @c std::is_trivially_copyable_v static_assert
 * applies here.
 */
class Pool
{
public:
  /**
   * @brief Construct an empty pool over @p a_resource, optionally pre-reserving a block.
   * @param[in] a_resource     Memory resource that backs every allocation of this pool. Must
   *                           outlive the pool.
   * @param[in] a_initialBytes If non-zero, the initial block byte capacity to reserve up front
   *                           (avoids the first grow). The block is still empty (@c usedBytes()==0).
   */
  EBGEOMETRY_HOST
  explicit Pool(MemoryResource& a_resource, size_t a_initialBytes = 0);

  /**
   * @brief Destructor. Returns the block to the backing resource.
   */
  EBGEOMETRY_HOST
  ~Pool() noexcept;

  /**
   * @brief Copy constructor. Deleted: a pool uniquely owns its block.
   */
  Pool(const Pool&) = delete;

  /**
   * @brief Copy assignment. Deleted: a pool uniquely owns its block.
   */
  Pool&
  operator=(const Pool&) = delete;

  /**
   * @brief Move constructor. Steals @p a_other's block and leaves it empty.
   * @param[in,out] a_other Source pool, left owning nothing.
   */
  EBGEOMETRY_HOST
  Pool(Pool&& a_other) noexcept;

  /**
   * @brief Move assignment. Frees any current block, then steals @p a_other's.
   * @param[in,out] a_other Source pool, left owning nothing.
   * @return Reference to this pool.
   */
  EBGEOMETRY_HOST
  Pool&
  operator=(Pool&& a_other) noexcept;

  /**
   * @brief Bump-reserve @p a_count * @p a_elemSize bytes aligned to @p a_alignment.
   * @details Returns the byte offset of the reserved sub-region from @ref base. May grow the
   *          block (on a host-accessible resource only). Forbidden after @ref freeze.
   * @param[in] a_count     Number of elements.
   * @param[in] a_elemSize  Size of one element in bytes.
   * @param[in] a_alignment Required alignment of the sub-region (power of two, <= @ref PoolBaseAlign).
   * @return Byte offset of the sub-region from @ref base; @c base() + offset is @p a_alignment-aligned.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint64_t
  reserve(size_t a_count, size_t a_elemSize, size_t a_alignment);

  /**
   * @brief Freeze the block: forbid further @ref reserve, stabilize @ref base, enable @ref mirror.
   * @details Idempotent. This is the single synchronization point between the mutating build phase
   *          and the read-only query phase.
   */
  EBGEOMETRY_HOST
  void
  freeze() noexcept;

  /**
   * @brief Base address of the block.
   * @return Pointer to the first byte of the block (null if nothing has been reserved yet).
   */
  [[nodiscard]] EBGEOMETRY_HOST
  void*
  base() const noexcept
  {
    return m_base;
  }

  /**
   * @brief Bytes currently in use (the bump cursor).
   * @return Number of reserved bytes.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  size_t
  usedBytes() const noexcept
  {
    return m_size;
  }

  /**
   * @brief Bytes currently allocated for the block.
   * @return Block byte capacity.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  size_t
  capacityBytes() const noexcept
  {
    return m_capacity;
  }

  /**
   * @brief Whether the pool has been frozen.
   * @return True after @ref freeze.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  bool
  isFrozen() const noexcept
  {
    return m_frozen;
  }

  /**
   * @brief The backing memory resource.
   * @return Reference to the resource passed at construction.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  MemoryResource&
  resource() const noexcept
  {
    return *m_resource;
  }

  /**
   * @brief Mirror a frozen pool's whole block into another resource.
   * @details Allocates a fresh block of exactly @c a_src.usedBytes() from @p a_dstResource, copies
   *          the entire block byte-for-byte (via @c GPU::memcpy for a host/device transfer, or
   *          @c std::memcpy for a host-to-host copy), and returns the result @b frozen. Every
   *          @ref PODVector offset resolves unchanged against the new base -- this is the
   *          host-to-device mirror (and, host-to-host, an exact independent copy).
   * @param[in] a_src         Source pool. Must be frozen (@c a_src.isFrozen()).
   * @param[in] a_dstResource Destination resource for the mirrored block.
   * @return A new, frozen pool holding a byte-identical copy of @p a_src's block.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static Pool
  mirror(const Pool& a_src, MemoryResource& a_dstResource);

private:
  /// @brief Tag type selecting the private constructor used by @ref mirror for a destination pool,
  /// which may be device-resident (and therefore not host-accessible) and is never reserved into.
  struct MirrorTag
  {
  };

  /**
   * @brief Private constructor for @ref mirror's destination pool. Unlike the public constructor it
   * does not require a host-accessible resource, because the destination is never built into with
   * @ref reserve -- @ref mirror allocates its exact block directly and freezes it.
   * @param[in] a_resource Backing resource (may be device-resident).
   */
  EBGEOMETRY_HOST
  Pool(MemoryResource& a_resource, MirrorTag) noexcept : m_resource(&a_resource)
  {}

  /// @brief Backing memory resource (non-owning; must outlive the pool).
  MemoryResource* m_resource = nullptr;

  /// @brief Base address of the block.
  void* m_base = nullptr;

  /// @brief Bump cursor: bytes currently in use.
  size_t m_size = 0;

  /// @brief Bytes currently allocated.
  size_t m_capacity = 0;

  /// @brief Whether the pool has been frozen.
  bool m_frozen = false;

  /**
   * @brief Grow the block to hold at least @p a_need bytes, preserving current contents.
   * @details Host-only (a device block is never grown): geometric doubling, rounded up to
   *          @ref PoolBaseAlign. Copies the in-use bytes into the new block and frees the old one.
   * @param[in] a_need Minimum required byte capacity.
   */
  EBGEOMETRY_HOST
  void
  grow(size_t a_need);
};

} // namespace EBGeometry

#include "EBGeometry_PoolImplem.hpp"

#endif // EBGEOMETRY_POOL_HPP
