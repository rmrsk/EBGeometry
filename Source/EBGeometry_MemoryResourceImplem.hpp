// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_MemoryResourceImplem.hpp
 * @brief  Implementation of EBGeometry_MemoryResource.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_MEMORYRESOURCEIMPLEM_HPP
#define EBGEOMETRY_MEMORYRESOURCEIMPLEM_HPP

// Std includes
#include <cstddef>
#include <cstdlib>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPURuntime.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_MemoryResource.hpp"

namespace EBGeometry {

inline void*
HostMemoryResource::allocate(size_t a_bytes, size_t a_alignment)
{
  EBGEOMETRY_EXPECT(a_alignment > 0);
  EBGEOMETRY_EXPECT((a_alignment & (a_alignment - 1)) == 0); // power of two

  // std::aligned_alloc requires the size to be a multiple of the alignment, and a zero-byte
  // request is implementation-defined, so round the byte count up to a non-zero multiple.
  size_t rounded = (a_bytes + a_alignment - 1) & ~(a_alignment - 1);

  if (rounded == 0) {
    rounded = a_alignment;
  }

  void* ptr = std::aligned_alloc(a_alignment, rounded);

  EBGEOMETRY_EXPECT(ptr != nullptr);

  return ptr;
}

inline void
HostMemoryResource::deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept
{
  (void)a_bytes;
  (void)a_alignment;

  std::free(a_ptr);
}

inline HostMemoryResource&
hostMemoryResource() noexcept
{
  static HostMemoryResource s_resource;

  return s_resource;
}

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP) || defined(EBGEOMETRY_DOXYGEN)

inline void*
DeviceMemoryResource::allocate(size_t a_bytes, [[maybe_unused]] size_t a_alignment)
{
  // The backend allocator already returns a base aligned to >= PoolBaseAlign (256), so we only
  // validate that no larger alignment was requested rather than enforce it. See PoolBaseAlign.
  EBGEOMETRY_EXPECT(a_alignment <= 256);

  void* ptr = nullptr;

  EBGEOMETRY_GPU_CHECK(GPU::memAlloc(&ptr, a_bytes));

  return ptr;
}

inline void
DeviceMemoryResource::deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept
{
  (void)a_bytes;
  (void)a_alignment;

  if (a_ptr != nullptr) {
    EBGEOMETRY_GPU_CHECK(GPU::memFree(a_ptr));
  }
}

inline void*
ManagedMemoryResource::allocate(size_t a_bytes, [[maybe_unused]] size_t a_alignment)
{
  EBGEOMETRY_EXPECT(a_alignment <= 256);

  void* ptr = nullptr;

  EBGEOMETRY_GPU_CHECK(GPU::memAllocManaged(&ptr, a_bytes));

  return ptr;
}

inline void
ManagedMemoryResource::deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept
{
  (void)a_bytes;
  (void)a_alignment;

  if (a_ptr != nullptr) {
    EBGEOMETRY_GPU_CHECK(GPU::memFree(a_ptr));
  }
}

inline void*
PinnedMemoryResource::allocate(size_t a_bytes, [[maybe_unused]] size_t a_alignment)
{
  EBGEOMETRY_EXPECT(a_alignment <= 256);

  void* ptr = nullptr;

  EBGEOMETRY_GPU_CHECK(GPU::memAllocHost(&ptr, a_bytes));

  return ptr;
}

inline void
PinnedMemoryResource::deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept
{
  (void)a_bytes;
  (void)a_alignment;

  if (a_ptr != nullptr) {
    EBGEOMETRY_GPU_CHECK(GPU::memFreeHost(a_ptr));
  }
}

inline void*
MappedMemoryResource::allocate(size_t a_bytes, [[maybe_unused]] size_t a_alignment)
{
  EBGEOMETRY_EXPECT(a_alignment <= 256);

  void* ptr = nullptr;

  EBGEOMETRY_GPU_CHECK(GPU::memAllocHostMapped(&ptr, a_bytes));

  return ptr;
}

inline void
MappedMemoryResource::deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept
{
  (void)a_bytes;
  (void)a_alignment;

  if (a_ptr != nullptr) {
    EBGEOMETRY_GPU_CHECK(GPU::memFreeHost(a_ptr));
  }
}

inline DeviceMemoryResource&
deviceMemoryResource() noexcept
{
  static DeviceMemoryResource s_resource;

  return s_resource;
}

inline ManagedMemoryResource&
managedMemoryResource() noexcept
{
  static ManagedMemoryResource s_resource;

  return s_resource;
}

inline PinnedMemoryResource&
pinnedMemoryResource() noexcept
{
  static PinnedMemoryResource s_resource;

  return s_resource;
}

inline MappedMemoryResource&
mappedMemoryResource() noexcept
{
  static MappedMemoryResource s_resource;

  return s_resource;
}

#endif // EBGEOMETRY_CUDA || EBGEOMETRY_HIP || EBGEOMETRY_DOXYGEN

} // namespace EBGeometry

#endif // EBGEOMETRY_MEMORYRESOURCEIMPLEM_HPP
