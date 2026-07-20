// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_MemoryResource.hpp
 * @brief  Placement-policy layer for the GPU memory foundation: the abstract MemoryResource and
 *         its concrete host/device/managed/pinned implementations.
 * @details A @ref EBGeometry::MemoryResource is a thin, runtime-virtual allocation policy: it
 * decides @e where a block of bytes physically lives (host RAM, device memory, unified/managed
 * memory, or page-locked host memory) without the caller knowing which. Allocation is a cold,
 * once-per-build operation, so the virtual dispatch cost is irrelevant and the runtime-swappable
 * placement is the whole point: the same POD scene structure (see @ref EBGeometry::Pool and
 * @ref EBGeometry::PODVector) can be built into a host resource for a CPU query, mirrored into a
 * device resource for a kernel, or built directly into managed memory for both.
 *
 * @ref EBGeometry::HostMemoryResource is always compiled (plain aligned host allocation). The
 * device-backed resources -- @ref EBGeometry::DeviceMemoryResource,
 * @ref EBGeometry::ManagedMemoryResource, @ref EBGeometry::PinnedMemoryResource -- are compiled
 * only when an offload backend is active (CUDA/HIP) or for documentation, behind the same guard
 * used elsewhere in the GPU port, and reach the backend exclusively through the backend-neutral
 * @ref EBGeometry::GPU alias layer.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_MEMORYRESOURCE_HPP
#define EBGEOMETRY_MEMORYRESOURCE_HPP

// Std includes
#include <cstddef>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPURuntime.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

/**
 * @brief Abstract placement policy: allocates and frees raw, correctly-aligned byte blocks.
 * @details Runtime-virtual so that placement (host/device/managed/pinned) is a runtime choice.
 * Allocation and deallocation are host-only operations -- memory is never obtained from device
 * code. The @c (bytes, alignment) pair is passed back to @ref deallocate (the @c std::pmr
 * convention), so a resource built on @c std::aligned_alloc / @c free needs no size bookkeeping.
 *
 * The type is deliberately non-copyable: a resource's identity matters (two pools over the same
 * resource share a placement), and copying an allocator has no meaning.
 *
 * @note This is one of the two deliberately non-trivial types of the memory foundation (it owns a
 * vtable); everything stored @e inside a @ref Pool is trivially copyable POD, but the resource and
 * the pool themselves are not, so no @c std::is_trivially_copyable_v static_assert applies here.
 */
class MemoryResource
{
public:
  /**
   * @brief Default constructor.
   */
  EBGEOMETRY_HOST
  MemoryResource() = default;

  /**
   * @brief Destructor.
   */
  EBGEOMETRY_HOST virtual ~MemoryResource() = default;

  /**
   * @brief Copy constructor. Deleted: a resource's identity is meaningful and non-copyable.
   */
  MemoryResource(const MemoryResource&) = delete;

  /**
   * @brief Copy assignment. Deleted: a resource's identity is meaningful and non-copyable.
   */
  MemoryResource&
  operator=(const MemoryResource&) = delete;

  /**
   * @brief Obtain a block of at least @p a_bytes bytes, aligned to @p a_alignment.
   * @details Never returns null on success; on failure the implementation aborts (via
   * @c EBGEOMETRY_GPU_CHECK / @c std::abort), matching the library's no-exceptions convention.
   * @param[in] a_bytes     Number of bytes to allocate (may be zero).
   * @param[in] a_alignment Required alignment in bytes. Must be a power of two.
   * @return Pointer to the allocated block.
   */
  [[nodiscard]] EBGEOMETRY_HOST virtual void*
  allocate(size_t a_bytes, size_t a_alignment) = 0;

  /**
   * @brief Release a block previously returned by @ref allocate.
   * @param[in] a_ptr       Pointer previously returned by @ref allocate (null is a no-op).
   * @param[in] a_bytes     The same byte count passed to the matching @ref allocate call.
   * @param[in] a_alignment The same alignment passed to the matching @ref allocate call.
   */
  EBGEOMETRY_HOST virtual void
  deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept = 0;

  /**
   * @brief Whether a plain host pointer dereference / @c std::memcpy is valid on returned blocks.
   * @return True if returned blocks are host-accessible.
   */
  [[nodiscard]] EBGEOMETRY_HOST virtual bool
  isHostAccessible() const noexcept = 0;

  /**
   * @brief Whether a device kernel may dereference returned blocks.
   * @return True if returned blocks are device-accessible.
   */
  [[nodiscard]] EBGEOMETRY_HOST virtual bool
  isDeviceAccessible() const noexcept = 0;
};

/**
 * @brief Host memory resource: plain aligned allocation in host RAM (@c std::aligned_alloc /
 * @c free family).
 * @details Always compiled, on every backend. Blocks are host-accessible and not device-accessible.
 */
class HostMemoryResource final : public MemoryResource
{
public:
  /**
   * @brief Default constructor.
   */
  EBGEOMETRY_HOST
  HostMemoryResource() = default;

  /**
   * @brief Destructor.
   */
  EBGEOMETRY_HOST ~HostMemoryResource() override = default;

  /**
   * @brief Allocate an aligned block of host memory.
   * @param[in] a_bytes     Number of bytes to allocate.
   * @param[in] a_alignment Required alignment in bytes (power of two).
   * @return Pointer to the allocated host block (never null on success).
   */
  [[nodiscard]] EBGEOMETRY_HOST void*
  allocate(size_t a_bytes, size_t a_alignment) override;

  /**
   * @brief Free a block previously returned by @ref allocate.
   * @param[in] a_ptr       Pointer to free (null is a no-op).
   * @param[in] a_bytes     Unused (host free needs no size).
   * @param[in] a_alignment Unused (host free needs no alignment).
   */
  EBGEOMETRY_HOST void
  deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept override;

  /**
   * @brief Host blocks are host-accessible.
   * @return Always true.
   */
  [[nodiscard]] EBGEOMETRY_HOST bool
  isHostAccessible() const noexcept override
  {
    return true;
  }

  /**
   * @brief Host blocks are not device-accessible.
   * @return Always false.
   */
  [[nodiscard]] EBGEOMETRY_HOST bool
  isDeviceAccessible() const noexcept override
  {
    return false;
  }
};

/**
 * @brief Process-wide default host memory resource.
 * @details Returns a reference to a function-local static @ref HostMemoryResource, so it is
 * constructed on first use and lives for the remainder of the program.
 * @return Reference to the shared host resource.
 */
[[nodiscard]] EBGEOMETRY_HOST HostMemoryResource&
hostMemoryResource() noexcept;

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP) || defined(EBGEOMETRY_DOXYGEN)

/**
 * @brief Device memory resource: raw device allocation (@c GPU::memAlloc / @c GPU::memFree).
 * @details Compiled only under an offload backend (CUDA/HIP) or for documentation. Blocks are
 * device-accessible and not host-accessible: a host pointer dereference of a returned block is
 * undefined. The backend allocator already returns a base aligned to at least
 * @ref EBGeometry::PoolBaseAlign, so the requested alignment is only validated, not enforced.
 */
class DeviceMemoryResource final : public MemoryResource
{
public:
  /**
   * @brief Default constructor.
   */
  EBGEOMETRY_HOST
  DeviceMemoryResource() = default;

  /**
   * @brief Destructor.
   */
  EBGEOMETRY_HOST ~DeviceMemoryResource() override = default;

  /**
   * @brief Allocate a block of device memory.
   * @param[in] a_bytes     Number of bytes to allocate.
   * @param[in] a_alignment Required alignment (validated to be <= @ref PoolBaseAlign; the backend
   *                        base already satisfies it).
   * @return Device pointer to the allocated block (never null on success; aborts on failure).
   */
  [[nodiscard]] EBGEOMETRY_HOST void*
  allocate(size_t a_bytes, size_t a_alignment) override;

  /**
   * @brief Free a block previously returned by @ref allocate.
   * @param[in] a_ptr       Device pointer to free (null is a no-op).
   * @param[in] a_bytes     Unused.
   * @param[in] a_alignment Unused.
   */
  EBGEOMETRY_HOST void
  deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept override;

  /**
   * @brief Device blocks are not host-accessible.
   * @return Always false.
   */
  [[nodiscard]] EBGEOMETRY_HOST bool
  isHostAccessible() const noexcept override
  {
    return false;
  }

  /**
   * @brief Device blocks are device-accessible.
   * @return Always true.
   */
  [[nodiscard]] EBGEOMETRY_HOST bool
  isDeviceAccessible() const noexcept override
  {
    return true;
  }
};

/**
 * @brief Managed (unified) memory resource: one allocation reachable from host and device
 *        (@c GPU::memAllocManaged / @c GPU::memFree).
 * @details Compiled only under an offload backend (CUDA/HIP) or for documentation. Blocks are both
 * host- and device-accessible: the same base pointer can be dereferenced by the CPU and passed to
 * a kernel, at the cost of driver-managed page migration.
 */
class ManagedMemoryResource final : public MemoryResource
{
public:
  /**
   * @brief Default constructor.
   */
  EBGEOMETRY_HOST
  ManagedMemoryResource() = default;

  /**
   * @brief Destructor.
   */
  EBGEOMETRY_HOST ~ManagedMemoryResource() override = default;

  /**
   * @brief Allocate a block of managed (unified) memory.
   * @param[in] a_bytes     Number of bytes to allocate.
   * @param[in] a_alignment Required alignment (validated to be <= @ref PoolBaseAlign).
   * @return Managed pointer to the allocated block (never null on success; aborts on failure).
   */
  [[nodiscard]] EBGEOMETRY_HOST void*
  allocate(size_t a_bytes, size_t a_alignment) override;

  /**
   * @brief Free a block previously returned by @ref allocate.
   * @param[in] a_ptr       Managed pointer to free (null is a no-op).
   * @param[in] a_bytes     Unused.
   * @param[in] a_alignment Unused.
   */
  EBGEOMETRY_HOST void
  deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept override;

  /**
   * @brief Managed blocks are host-accessible.
   * @return Always true.
   */
  [[nodiscard]] EBGEOMETRY_HOST bool
  isHostAccessible() const noexcept override
  {
    return true;
  }

  /**
   * @brief Managed blocks are device-accessible.
   * @return Always true.
   */
  [[nodiscard]] EBGEOMETRY_HOST bool
  isDeviceAccessible() const noexcept override
  {
    return true;
  }
};

/**
 * @brief Pinned (page-locked) host memory resource: fast host-to-device staging
 *        (@c GPU::memAllocHost / @c GPU::memFreeHost).
 * @details Compiled only under an offload backend (CUDA/HIP) or for documentation. Blocks are
 * host-accessible and not device-accessible; being page-locked, they support faster and
 * asynchronous transfers to a device than ordinary pageable host memory.
 */
class PinnedMemoryResource final : public MemoryResource
{
public:
  /**
   * @brief Default constructor.
   */
  EBGEOMETRY_HOST
  PinnedMemoryResource() = default;

  /**
   * @brief Destructor.
   */
  EBGEOMETRY_HOST ~PinnedMemoryResource() override = default;

  /**
   * @brief Allocate a block of page-locked host memory.
   * @param[in] a_bytes     Number of bytes to allocate.
   * @param[in] a_alignment Required alignment (validated to be <= @ref PoolBaseAlign).
   * @return Host pointer to the pinned block (never null on success; aborts on failure).
   */
  [[nodiscard]] EBGEOMETRY_HOST void*
  allocate(size_t a_bytes, size_t a_alignment) override;

  /**
   * @brief Free a block previously returned by @ref allocate.
   * @param[in] a_ptr       Host pointer to free (null is a no-op).
   * @param[in] a_bytes     Unused.
   * @param[in] a_alignment Unused.
   */
  EBGEOMETRY_HOST void
  deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept override;

  /**
   * @brief Pinned blocks are host-accessible.
   * @return Always true.
   */
  [[nodiscard]] EBGEOMETRY_HOST bool
  isHostAccessible() const noexcept override
  {
    return true;
  }

  /**
   * @brief Pinned blocks are not device-accessible.
   * @return Always false.
   */
  [[nodiscard]] EBGEOMETRY_HOST bool
  isDeviceAccessible() const noexcept override
  {
    return false;
  }
};

/**
 * @brief Process-wide default device memory resource.
 * @details Returns a reference to a function-local static @ref DeviceMemoryResource.
 * @return Reference to the shared device resource.
 */
[[nodiscard]] EBGEOMETRY_HOST DeviceMemoryResource&
deviceMemoryResource() noexcept;

/**
 * @brief Process-wide default managed (unified) memory resource.
 * @details Returns a reference to a function-local static @ref ManagedMemoryResource.
 * @return Reference to the shared managed resource.
 */
[[nodiscard]] EBGEOMETRY_HOST ManagedMemoryResource&
managedMemoryResource() noexcept;

/**
 * @brief Process-wide default pinned (page-locked) host memory resource.
 * @details Returns a reference to a function-local static @ref PinnedMemoryResource.
 * @return Reference to the shared pinned resource.
 */
[[nodiscard]] EBGEOMETRY_HOST PinnedMemoryResource&
pinnedMemoryResource() noexcept;

#endif // EBGEOMETRY_CUDA || EBGEOMETRY_HIP || EBGEOMETRY_DOXYGEN

} // namespace EBGeometry

#include "EBGeometry_MemoryResourceImplem.hpp"

#endif // EBGEOMETRY_MEMORYRESOURCE_HPP
