// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Shared helpers for the in-suite GPU device checks. The per-class test files (TestVec,
// TestBoundingVolumes, TestPool, TestPointAoSoA, TestTriangleSoA, ...) each carry a device SECTION
// guarded by #if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP): when the file is compiled by
// an offload compiler (nvcc/hipcc) that block launches a kernel exercising the class's
// device-callable surface and REQUIREs the device result against the host result, skipping cleanly
// when no GPU device is present. When the file is compiled by an ordinary host compiler the whole
// block -- and everything in this header -- preprocesses away, so there is no host-build impact.

#ifndef EBGEOMETRY_TESTGPU_HPP
#define EBGEOMETRY_TESTGPU_HPP

#include "EBGeometry.hpp"

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

#include <type_traits>

namespace EBGeometryTestGPU {

namespace GPU = EBGeometry::GPU;

/*!
  @brief Whether at least one GPU device is visible to the runtime.
  @return True if the device count is nonzero.
*/
inline bool
deviceAvailable() noexcept
{
  int count = 0;

  (void)GPU::getDeviceCount(&count);

  return count > 0;
}

/*!
  @brief Allocate device storage for one trivially-copyable value and copy the host value into it.
  @param[in] a_host Host value to mirror. Its type must be trivially copyable.
  @return Device pointer to the mirrored value; free it with deviceFree().
*/
template <class T>
T*
mirrorToDevice(const T& a_host) noexcept
{
  static_assert(std::is_trivially_copyable_v<T>, "mirrorToDevice requires a trivially copyable type");

  T* devicePtr = nullptr;

  (void)GPU::memAlloc(reinterpret_cast<void**>(&devicePtr), sizeof(T));
  (void)GPU::memcpy(devicePtr, &a_host, sizeof(T), GPU::MemcpyHostToDevice);

  return devicePtr;
}

/*!
  @brief Allocate a single-double device output cell.
  @return Device pointer to the cell; free it with deviceFree().
*/
inline double*
deviceScalar() noexcept
{
  double* devicePtr = nullptr;

  (void)GPU::memAlloc(reinterpret_cast<void**>(&devicePtr), sizeof(double));

  return devicePtr;
}

/*!
  @brief Read one double back from the device.
  @param[in] a_devicePtr Device pointer previously filled by a kernel.
  @return The value copied back to the host.
*/
inline double
readScalar(double* a_devicePtr) noexcept
{
  double host = 0.0;

  (void)GPU::memcpy(&host, a_devicePtr, sizeof(double), GPU::MemcpyDeviceToHost);

  return host;
}

/*!
  @brief Free device memory allocated by one of the helpers above.
  @param[in] a_devicePtr Device pointer to free.
*/
inline void
deviceFree(void* a_devicePtr) noexcept
{
  (void)GPU::memFree(a_devicePtr);
}

} // namespace EBGeometryTestGPU

#endif

#endif
