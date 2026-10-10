// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Pool.hpp
 * @brief  Growable bump arena for the GPU memory foundation.
 * @details A @ref EBGeometry::Pool is a single contiguous byte block obtained from a
 * @ref EBGeometry::MemoryResource, into which many sub-regions are bump-reserved and from which
 * nothing is individually freed.
 *
 * Building and querying are not separate phases. @ref EBGeometry::Pool::reserve hands out byte
 * offsets and may grow (reallocate and move) the block at any time. Because every
 * @ref EBGeometry::PODVector stores a byte @e offset rather than a pointer, and pool-resident
 * objects re-read the base through the pool's @ref EBGeometry::PoolControl on every access, a grow
 * invalidates no @ref EBGeometry::PODVector and no such object: an object is queryable as soon as it
 * is built, while the same pool keeps being built into. What a grow does invalidate is any address
 * already resolved against the old base (a pointer, reference or @ref EBGeometry::PODSpan), hence
 * the rule @b resolve, @b use, @b discard -- see @ref EBGeometry::Pool::reserve.
 *
 * @ref EBGeometry::Pool::freeze seals the pool (a further reserve aborts, in every build) and is
 * the precondition for @ref EBGeometry::Pool::mirror, which copies the whole block (one
 * @c memcpy) into another resource -- the host-to-device upload. The same offsets resolve against
 * the new base with zero pointer patching. Freezing is not required to query anything.
 *
 * The @ref EBGeometry::Pool is the one owning, move-only, RAII (hence non-trivial) type of the
 * foundation; everything stored inside it is trivially copyable POD.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POOL_HPP
#define EBGEOMETRY_POOL_HPP

// Std includes
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_MemoryResource.hpp"

namespace EBGeometry {

/**
 * @brief Stable, heap-resident handle to a @ref Pool's current base address and identity.
 * @details A pool-resident object (e.g. @ref EBGeometry::DCEL::MeshT) must be able to re-read its
 * pool's base on every access, because @ref Pool::reserve may grow the block and move it. Pointing
 * such an object at the @ref Pool itself would work until the pool is @e moved -- pools are
 * move-only but movable (@ref Pool::mirror returns by value), and a @c Pool* would be left naming a
 * relocated, possibly destroyed object. The control block solves this: it is heap-allocated,
 * uniquely owned by the pool, and its @e address never changes for the lifetime of the pool,
 * however many times the pool itself is moved.
 *
 * @note This is host bookkeeping. It is never mirrored to a device and must never be dereferenced
 * from device code -- a device-bound view of a pool-resident object holds a plain base pointer
 * instead. See @ref EBGeometry::DCEL::MeshT::rebasedView.
 */
struct PoolControl
{
  /// @brief Current base address of the owning pool's block. Rewritten whenever a
  /// @ref Pool::reserve grows the block.
  void* m_base = nullptr;

  /// @brief Identity of the owning pool; see @ref Pool::id. Never zero for a live pool.
  uint64_t m_id = 0;

  /// @brief Identity of the root of the owning pool's mirror chain: its own identity for a pool that
  /// is not a mirror, else the identity of the pool the chain started from. See @ref Pool::rootId.
  uint64_t m_rootId = 0;
};

class Pool;

/**
 * @brief Where a pool-resident descriptor resolves its offsets, and the rules for moving it.
 * @details Every pool-resident descriptor (@ref EBGeometry::DCEL::MeshT, @ref EBGeometry::BVH::PackedBVH,
 * and the classes built on them) holds one PoolLocation, and delegates attaching, resolving and
 * rebasing to it. It is in one of three states, decided by its value:
 *
 * - @b Unset: nothing has been reserved yet.
 * - @b Following a pool: @c m_control is set. base() reads the pool's current base through the
 *   control block on every call, so a pool that grows and moves its block is invisible to the
 *   descriptor. Only host code can follow a pool.
 * - @b Snapshot: @c m_control is null and @c m_base holds the base of a frozen, device-accessible
 *   pool, captured by rebasedOnto(). This is what a kernel receives. If that memory is also
 *   host-accessible (managed or mapped memory), @c m_hostAccessible is set and host code may use the
 *   snapshot too.
 *
 * A descriptor normally changes location only through @c rebasedView(), which calls rebasedOnto().
 * The exception is a descriptor stored @e inside another object's pool -- a @c TriMeshSDF held in the
 * primitive array of a @ref EBGeometry::BVHUnion, for instance. Mirroring that pool copies the inner
 * descriptor's bytes verbatim, host control block included, and rebasing the outer object cannot
 * rewrite them in place. The outer object therefore reads its own location and applies it to a local
 * copy of each inner descriptor as it evaluates it (@c relocatedTo()), which is sound because both were
 * reserved from the same pool.
 */
struct PoolLocation
{
  /// @brief Control block of the pool the descriptor follows; null for a snapshot or when unset.
  const PoolControl* m_control = nullptr;

  /// @brief Base address captured by a snapshot; null otherwise.
  void* m_base = nullptr;

  /// @brief Whether host code may dereference a snapshot's base (managed or mapped memory).
  bool m_hostAccessible = false;

  /**
   * @brief The base address the descriptor's offsets resolve against.
   * @details On the host, the followed pool's current base, or a snapshot's base if host code may use
   * it. On a device, the snapshot's base: a location that still follows a host pool means a host
   * descriptor was copied into a kernel without rebasedView(), which EBGEOMETRY_EXPECT catches.
   * @return The base address.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline void*
  base() const noexcept
  {
#if defined(EBGEOMETRY_DEVICE_COMPILE)
    EBGEOMETRY_EXPECT(m_control == nullptr);

    return m_base;
#else
    if (m_control != nullptr) {
      return m_control->m_base;
    }

    // Unset, or a snapshot of memory the host cannot reach (a device-only mirror).
    EBGEOMETRY_EXPECT(m_hostAccessible);

    return m_base;
#endif
  }

  /**
   * @brief Whether this location follows the given pool.
   * @param[in] a_pool Pool to compare with.
   * @return True if the descriptor follows @p a_pool's control block.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline bool
  isAttachedTo(const Pool& a_pool) const noexcept;

  /**
   * @brief Start following a pool, as the descriptor's first reservation from it does.
   * @details Aborts, in every build, if the location already follows a different pool: all of a
   * descriptor's arrays resolve against one base, so they must come from one pool.
   * @param[in] a_pool Pool to follow.
   * @param[in] a_who  Name of the descriptor class, for the message.
   */
  EBGEOMETRY_HOST
  inline void
  attach(const Pool& a_pool, const char* a_who) noexcept;

  /**
   * @brief The location of the same arrays in another pool of the same mirror chain.
   * @details The descriptor must follow a pool (not be a snapshot or unset), @p a_pool must belong to
   * the same mirror chain -- the pool itself, one of its mirrors, or a mirror of a mirror -- and the
   * descriptor's arrays must fit inside it. Each is checked in every build. A device-accessible
   * @p a_pool, which must be frozen so its base cannot move, gives a snapshot; a host pool gives a
   * location that follows it.
   * @param[in] a_pool    Pool to resolve against.
   * @param[in] a_endByte First byte past the descriptor's last array.
   * @param[in] a_who     Name of the descriptor class, for the messages.
   * @return The new location.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline PoolLocation
  rebasedOnto(const Pool& a_pool, uint64_t a_endByte, const char* a_who) const noexcept;
};
/**
 * @brief Growable bump arena over a @ref MemoryResource.
 * @details Move-only, single-owner RAII. See the file-level documentation for how growth, freezing
 * and mirroring interact, and @ref reserve for the resolve-use-discard rule.
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
   *          block (on a host-accessible resource only). Forbidden after freeze(): a reserve
   *          on a frozen pool, or an invalid @p a_alignment, aborts with a message in every build.
   *
   * @warning A grow does @b not extend the block in place: it allocates a new block, copies the
   * live bytes into it, and @e deallocates the old one. Every address that was already resolved
   * against the old base -- a raw pointer, a @c T& handed out by @ref PODVector::at or by a
   * pool-resident container's accessor, a @ref PODSpan from @ref PODVector::bind -- is dangling
   * afterwards. Every @ref PODVector is @e unaffected, because it stores a byte offset rather than
   * an address, and so is anything that re-resolves through @ref PoolControl::m_base.
   *
   * The rule is therefore: @b resolve, @b use, @b discard. A resolved address must not outlive the
   * next @ref reserve on the same pool. To carry data across a @ref reserve, carry it @e by value
   * and write it back through a fresh resolution:
   *
   * @code
   * Edge e = mesh.getEdge(3);      // snapshot; independent of any base
   * mesh.reserveFaces(pool, n);    // may grow, moving the block
   * e.setFace(7);
   * mesh.getEdge(3) = e;           // fresh resolution against the current base
   * @endcode
   *
   * Note that the hazard is intermittent -- a grow only happens when the request exceeds the
   * current capacity -- and its quiet form is worse than a crash: if the freed block is still
   * mapped, a write through a stale address lands in the abandoned copy and is silently lost.
   *
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
   * @details Idempotent and irreversible. The only precondition it establishes is the one
   *          @ref mirror needs (a block that can no longer be reallocated); a reserve after it
   *          aborts, in every build. Freezing is not required to query anything built in the pool.
   */
  EBGEOMETRY_HOST
  void
  freeze() noexcept;

  /**
   * @brief Base address of the block.
   * @return Pointer to the first byte of the block (null if nothing has been reserved yet, or if
   *         this pool has been moved from).
   */
  [[nodiscard]] EBGEOMETRY_HOST
  void*
  base() const noexcept
  {
    return (m_control != nullptr) ? m_control->m_base : nullptr;
  }

  /**
   * @brief This pool's stable control block, through which pool-resident objects re-resolve the
   * base on every access.
   * @details The returned address is fixed for the lifetime of the pool and survives every move of
   * the pool object, which is precisely why pool-resident objects store this rather than a
   * @c Pool*. It does @b not survive the pool's destruction: the pool must outlive every object
   * reserved from it.
   * @return Pointer to the control block (null only for a moved-from pool).
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const PoolControl*
  control() const noexcept
  {
    return m_control.get();
  }

  /**
   * @brief Identity of this pool, unique among all pools constructed in this process.
   * @details Assigned at construction from a monotonic counter and never reused; a mirror gets its
   *          own fresh identity. Used by @ref mirrorOf to record lineage.
   * @return This pool's identity (zero only for a moved-from pool).
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint64_t
  id() const noexcept
  {
    return (m_control != nullptr) ? m_control->m_id : 0;
  }

  /**
   * @brief Identity of the pool this one ultimately mirrors, or zero if it is not a mirror.
   * @details This names the @e root of the mirror chain, not the immediate source: mirroring
   *          host -> pinned staging -> device leaves the device pool naming the original host pool,
   *          not the staging pool. That is what lets a pool-resident object built in the host pool
   *          validate a rebase onto the device pool (see @ref EBGeometry::DCEL::MeshT::rebasedView)
   *          without having to know how many hops the block travelled.
   * @return Identity of the root source pool, or 0 if this pool is not a mirror.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint64_t
  mirrorOf() const noexcept
  {
    return m_mirrorOf;
  }

  /**
   * @brief Identity of the root of this pool's mirror chain.
   * @details This pool's own identity if it is not a mirror, else @ref mirrorOf. Two pools with the
   *          same root hold byte-identical copies of the root's block, as far as the root's frozen
   *          contents go, so a descriptor built in one can be rebased onto any other.
   * @return The root identity (zero only for a moved-from pool).
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint64_t
  rootId() const noexcept
  {
    return (m_control != nullptr) ? m_control->m_rootId : 0;
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
   * @param[in] a_src         Source pool. Must be frozen (@c a_src.isFrozen()); an unfrozen source
   *                          aborts, in every build.
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
  Pool(MemoryResource& a_resource, MirrorTag) : m_resource(&a_resource), m_control(makeControl())
  {}

  /**
   * @brief Allocate a fresh control block carrying the next pool identity.
   * @details The counter is an atomic because EBGeometry is header-only and nothing prevents two
   *          threads from constructing pools concurrently. Identities start at one, so zero is
   *          available as the "not a mirror" / "moved-from" sentinel.
   * @return Owning pointer to the new control block.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static std::unique_ptr<PoolControl>
  makeControl()
  {
    static std::atomic<uint64_t> s_nextID{1};

    auto control = std::make_unique<PoolControl>();

    control->m_id     = s_nextID.fetch_add(1, std::memory_order_relaxed);
    control->m_rootId = control->m_id;

    return control;
  }

  /// @brief Backing memory resource (non-owning; must outlive the pool).
  MemoryResource* m_resource = nullptr;

  /// @brief Stable handle to the block's base address and this pool's identity. Null if moved from.
  std::unique_ptr<PoolControl> m_control;

  /// @brief Bump cursor: bytes currently in use.
  size_t m_size = 0;

  /// @brief Bytes currently allocated.
  size_t m_capacity = 0;

  /// @brief Identity of the root pool this one mirrors; 0 if this pool is not a mirror.
  uint64_t m_mirrorOf = 0;

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
