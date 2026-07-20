// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPURuntime.hpp
 * @brief  Backend-neutral GPU runtime API layer (CUDA or HIP) plus the EBGEOMETRY_GPU_CHECK macro.
 * @details Unlike EBGeometry_GPU.hpp -- which only detects the offload compiler and defines the
 * decoration macros, pulling in no backend runtime headers -- this header IS the runtime-API
 * layer: it includes the backend runtime header (<cuda_runtime.h> or <hip/hip_runtime.h>) and
 * exposes the small memory-management/error surface EBGeometry needs through backend-neutral
 * aliases and inline wrappers in the EBGeometry::GPU namespace, so any translation unit that uses
 * it compiles unchanged whether built as a CUDA (.cu) or HIP (.hip) translation unit. On an
 * ordinary host compiler the whole header is inert (empty after preprocessing), so the umbrella
 * EBGeometry.hpp can include it unconditionally.
 *
 * HIP is checked before CUDA: hipcc on the NVIDIA platform defines both @c __HIPCC__ and
 * @c __CUDACC__, and there the hip* entry points exist and forward to cuda*, so mapping to hip*
 * whenever @c __HIPCC__ is defined is correct on both HIP platforms.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_GPURUNTIME_HPP
#define EBGEOMETRY_GPURUNTIME_HPP

// Our includes
#include "EBGeometry_GPU.hpp"

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP) || defined(EBGEOMETRY_DOXYGEN)

// Std includes (EBGEOMETRY_GPU_CHECK needs fprintf/abort; the wrappers need size_t).
#include <cstddef>
#include <cstdio>
#include <cstdlib>

// Backend runtime header. HIP is checked FIRST: hipcc on the NVIDIA platform defines both
// __HIPCC__ and __CUDACC__, and there the hip* entry points exist and forward to cuda*, so
// mapping to hip* whenever __HIPCC__ is defined is correct on both HIP platforms.
#if defined(EBGEOMETRY_HIP)
#include <hip/hip_runtime.h> // NOT pre-included by the compiler driver; must be explicit.
#elif defined(EBGEOMETRY_CUDA)
#include <cuda_runtime.h> // nvcc pre-includes this; spelled out for hygiene.
#endif

namespace EBGeometry {

/**
 * @brief Backend-neutral GPU runtime API surface (CUDA or HIP).
 * @details Thin aliases and inline forwarders over the backend runtime (cuda* under CUDA, hip*
 * under HIP) covering exactly the surface EBGeometry uses: memory allocation/copy/free, device
 * enumeration, error retrieval, and synchronization. The two runtimes mirror each other's
 * signatures 1:1 for this surface, so every wrapper is a one-line forwarder.
 */
namespace GPU {

#if defined(EBGEOMETRY_DOXYGEN)

/**
 * @brief Backend runtime error code (@c cudaError_t under CUDA, @c hipError_t under HIP).
 */
using Error = int;

/**
 * @brief Memory-copy direction (@c cudaMemcpyKind under CUDA, @c hipMemcpyKind under HIP).
 */
using MemcpyKind = int;

/**
 * @brief Success value of @ref Error (@c cudaSuccess / @c hipSuccess).
 */
inline constexpr Error Success = 0;

/**
 * @brief Host-to-device copy direction (@c cudaMemcpyHostToDevice / @c hipMemcpyHostToDevice).
 */
inline constexpr MemcpyKind MemcpyHostToDevice = 1;

/**
 * @brief Device-to-host copy direction (@c cudaMemcpyDeviceToHost / @c hipMemcpyDeviceToHost).
 */
inline constexpr MemcpyKind MemcpyDeviceToHost = 2;

/**
 * @brief Allocate device memory (@c cudaMalloc / @c hipMalloc).
 * @param[out] a_devicePtr Receives the device pointer to the allocation.
 * @param[in]  a_numBytes  Number of bytes to allocate.
 * @return Backend error code (@ref Success on success).
 */
[[nodiscard]] inline Error
memAlloc(void** a_devicePtr, size_t a_numBytes) noexcept;

/**
 * @brief Free device memory allocated with @ref memAlloc (@c cudaFree / @c hipFree).
 * @param[in] a_devicePtr Device pointer to free (null is a no-op).
 * @return Backend error code (@ref Success on success).
 */
[[nodiscard]] inline Error
memFree(void* a_devicePtr) noexcept;

/**
 * @brief Allocate managed (unified) memory accessible from host and device (@c cudaMallocManaged /
 * @c hipMallocManaged).
 * @param[out] a_ptr      Receives the managed pointer to the allocation.
 * @param[in]  a_numBytes Number of bytes to allocate.
 * @return Backend error code (@ref Success on success).
 */
[[nodiscard]] inline Error
memAllocManaged(void** a_ptr, size_t a_numBytes) noexcept;

/**
 * @brief Allocate page-locked (pinned) host memory (@c cudaHostAlloc / @c hipHostMalloc).
 * @param[out] a_ptr      Receives the host pointer to the pinned allocation.
 * @param[in]  a_numBytes Number of bytes to allocate.
 * @return Backend error code (@ref Success on success).
 */
[[nodiscard]] inline Error
memAllocHost(void** a_ptr, size_t a_numBytes) noexcept;

/**
 * @brief Free page-locked host memory allocated with @ref memAllocHost (@c cudaFreeHost /
 * @c hipHostFree).
 * @param[in] a_ptr Host pointer to free (null is a no-op).
 * @return Backend error code (@ref Success on success).
 */
[[nodiscard]] inline Error
memFreeHost(void* a_ptr) noexcept;

/**
 * @brief Copy memory between host and device (@c cudaMemcpy / @c hipMemcpy). Synchronous.
 * @param[out] a_dst      Destination pointer (host or device, per @p a_kind).
 * @param[in]  a_src      Source pointer (host or device, per @p a_kind).
 * @param[in]  a_numBytes Number of bytes to copy.
 * @param[in]  a_kind     Copy direction.
 * @return Backend error code (@ref Success on success).
 */
[[nodiscard]] inline Error
memcpy(void* a_dst, const void* a_src, size_t a_numBytes, MemcpyKind a_kind) noexcept;

/**
 * @brief Get the number of available GPU devices (@c cudaGetDeviceCount / @c hipGetDeviceCount).
 * @param[out] a_count Receives the device count.
 * @return Backend error code (@ref Success on success).
 */
[[nodiscard]] inline Error
getDeviceCount(int* a_count) noexcept;

/**
 * @brief Get and clear the last runtime error (@c cudaGetLastError / @c hipGetLastError).
 * @return Backend error code (@ref Success if no error is pending).
 */
[[nodiscard]] inline Error
getLastError() noexcept;

/**
 * @brief Block until the device has finished all preceding work (@c cudaDeviceSynchronize /
 * @c hipDeviceSynchronize).
 * @return Backend error code (@ref Success on success).
 */
[[nodiscard]] inline Error
deviceSynchronize() noexcept;

/**
 * @brief Get the human-readable description of an error code (@c cudaGetErrorString /
 * @c hipGetErrorString).
 * @param[in] a_error Backend error code.
 * @return Static string describing the error.
 */
[[nodiscard]] inline const char*
getErrorString(Error a_error) noexcept;

#elif defined(EBGEOMETRY_HIP)

using Error      = hipError_t;
using MemcpyKind = hipMemcpyKind;

inline constexpr Error      Success            = hipSuccess;
inline constexpr MemcpyKind MemcpyHostToDevice = hipMemcpyHostToDevice;
inline constexpr MemcpyKind MemcpyDeviceToHost = hipMemcpyDeviceToHost;

[[nodiscard]] inline Error
memAlloc(void** a_devicePtr, size_t a_numBytes) noexcept
{
  return hipMalloc(a_devicePtr, a_numBytes);
}

[[nodiscard]] inline Error
memFree(void* a_devicePtr) noexcept
{
  return hipFree(a_devicePtr);
}

[[nodiscard]] inline Error
memAllocManaged(void** a_ptr, size_t a_numBytes) noexcept
{
  return hipMallocManaged(a_ptr, a_numBytes);
}

[[nodiscard]] inline Error
memAllocHost(void** a_ptr, size_t a_numBytes) noexcept
{
  return hipHostMalloc(a_ptr, a_numBytes, 0u);
}

[[nodiscard]] inline Error
memFreeHost(void* a_ptr) noexcept
{
  return hipHostFree(a_ptr);
}

[[nodiscard]] inline Error
memcpy(void* a_dst, const void* a_src, size_t a_numBytes, MemcpyKind a_kind) noexcept
{
  return hipMemcpy(a_dst, a_src, a_numBytes, a_kind);
}

[[nodiscard]] inline Error
getDeviceCount(int* a_count) noexcept
{
  return hipGetDeviceCount(a_count);
}

[[nodiscard]] inline Error
getLastError() noexcept
{
  return hipGetLastError();
}

[[nodiscard]] inline Error
deviceSynchronize() noexcept
{
  return hipDeviceSynchronize();
}

[[nodiscard]] inline const char*
getErrorString(Error a_error) noexcept
{
  return hipGetErrorString(a_error);
}

#elif defined(EBGEOMETRY_CUDA)

using Error      = cudaError_t;
using MemcpyKind = cudaMemcpyKind;

inline constexpr Error      Success            = cudaSuccess;
inline constexpr MemcpyKind MemcpyHostToDevice = cudaMemcpyHostToDevice;
inline constexpr MemcpyKind MemcpyDeviceToHost = cudaMemcpyDeviceToHost;

[[nodiscard]] inline Error
memAlloc(void** a_devicePtr, size_t a_numBytes) noexcept
{
  return cudaMalloc(a_devicePtr, a_numBytes);
}

[[nodiscard]] inline Error
memFree(void* a_devicePtr) noexcept
{
  return cudaFree(a_devicePtr);
}

[[nodiscard]] inline Error
memAllocManaged(void** a_ptr, size_t a_numBytes) noexcept
{
  return cudaMallocManaged(a_ptr, a_numBytes, cudaMemAttachGlobal);
}

[[nodiscard]] inline Error
memAllocHost(void** a_ptr, size_t a_numBytes) noexcept
{
  return cudaHostAlloc(a_ptr, a_numBytes, cudaHostAllocDefault);
}

[[nodiscard]] inline Error
memFreeHost(void* a_ptr) noexcept
{
  return cudaFreeHost(a_ptr);
}

[[nodiscard]] inline Error
memcpy(void* a_dst, const void* a_src, size_t a_numBytes, MemcpyKind a_kind) noexcept
{
  return cudaMemcpy(a_dst, a_src, a_numBytes, a_kind);
}

[[nodiscard]] inline Error
getDeviceCount(int* a_count) noexcept
{
  return cudaGetDeviceCount(a_count);
}

[[nodiscard]] inline Error
getLastError() noexcept
{
  return cudaGetLastError();
}

[[nodiscard]] inline Error
deviceSynchronize() noexcept
{
  return cudaDeviceSynchronize();
}

[[nodiscard]] inline const char*
getErrorString(Error a_error) noexcept
{
  return cudaGetErrorString(a_error);
}

#endif

} // namespace GPU
} // namespace EBGeometry

/**
 * @brief Always-on GPU runtime API error check: prints the failing call and aborts.
 * @details Host-side only. Deliberately NOT gated on EBGEOMETRY_ENABLE_ASSERTIONS (unlike
 * EBGEOMETRY_EXPECT): a failed allocation or copy leaves the caller holding a null or dangling
 * device pointer, which a release build must not silently proceed to use. Abort (rather than an
 * exception) matches the library's no-exceptions convention.
 * @param x GPU runtime API call (or any expression) yielding an EBGeometry::GPU::Error.
 */
#define EBGEOMETRY_GPU_CHECK(x)                                                      \
  do {                                                                               \
    const EBGeometry::GPU::Error ebgeometry_gpu_err = (x);                           \
    if (ebgeometry_gpu_err != EBGeometry::GPU::Success) {                            \
      std::fprintf(stderr,                                                           \
                   "EBGeometry GPU error: %s\n  call: %s\n  file: %s\n  line: %d\n", \
                   EBGeometry::GPU::getErrorString(ebgeometry_gpu_err),              \
                   #x,                                                               \
                   __FILE__,                                                         \
                   __LINE__);                                                        \
      std::abort();                                                                  \
    }                                                                                \
  } while (0)

#endif // EBGEOMETRY_CUDA || EBGEOMETRY_HIP || EBGEOMETRY_DOXYGEN

#endif // EBGEOMETRY_GPURUNTIME_HPP
