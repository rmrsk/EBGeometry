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
  @brief RAII owner of a single-element device allocation.
  @details Frees the allocation in its destructor, so a device buffer is released even when a Catch2
  REQUIRE/REQUIRE_THAT throws out of the test before the end of the block is reached. Move-only.
  @tparam T Trivially-copyable element type held on the device.
*/
template <class T>
class DeviceBuffer
{
public:
  static_assert(std::is_trivially_copyable_v<T>, "DeviceBuffer requires a trivially copyable type");

  /*!
    @brief Allocate storage for one T on the device.
  */
  DeviceBuffer() noexcept
  {
    (void)GPU::memAlloc(reinterpret_cast<void**>(&m_data), sizeof(T));
  }

  DeviceBuffer(const DeviceBuffer&) = delete;
  DeviceBuffer&
  operator=(const DeviceBuffer&) = delete;

  /*!
    @brief Take ownership of another buffer's allocation, leaving it empty.
  */
  DeviceBuffer(DeviceBuffer&& a_other) noexcept : m_data(a_other.m_data)
  {
    a_other.m_data = nullptr;
  }

  /*!
    @brief Free this buffer's allocation and take ownership of another's.
  */
  DeviceBuffer&
  operator=(DeviceBuffer&& a_other) noexcept
  {
    if (this != &a_other) {
      if (m_data != nullptr) {
        (void)GPU::memFree(m_data);
      }

      m_data         = a_other.m_data;
      a_other.m_data = nullptr;
    }

    return *this;
  }

  /*!
    @brief Free the device allocation.
  */
  ~DeviceBuffer() noexcept
  {
    if (m_data != nullptr) {
      (void)GPU::memFree(m_data);
    }
  }

  /*!
    @brief The raw device pointer, for passing to a kernel.
    @return Device pointer to the single T.
  */
  T*
  get() const noexcept
  {
    return m_data;
  }

private:
  T* m_data = nullptr;
};

/*!
  @brief Allocate device storage for one trivially-copyable value and copy the host value into it.
  @param[in] a_host Host value to mirror. Its type must be trivially copyable.
  @return An RAII DeviceBuffer owning the mirrored value.
*/
template <class T>
DeviceBuffer<T>
mirrorToDevice(const T& a_host) noexcept
{
  DeviceBuffer<T> buffer;

  (void)GPU::memcpy(buffer.get(), &a_host, sizeof(T), GPU::MemcpyHostToDevice);

  return buffer;
}

/*!
  @brief Read one value back from the device.
  @param[in] a_devicePtr Device pointer previously filled by a kernel.
  @return The value copied back to the host.
*/
template <class T>
T
readScalar(const T* a_devicePtr) noexcept
{
  T host{};

  (void)GPU::memcpy(&host, a_devicePtr, sizeof(T), GPU::MemcpyDeviceToHost);

  return host;
}

/*!
  @brief Relative tolerance for comparing a device reduction against its host counterpart.
  @details The host and device evaluate the same scalar arithmetic, so they differ only by
  fused-multiply-add contraction and reassociation; this margin is loose enough to absorb that in
  either precision while still catching a genuinely wrong result.
  @return A relative tolerance appropriate to T (looser for float than for double).
*/
template <class T>
constexpr T
gpuTol() noexcept
{
  return std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);
}

} // namespace EBGeometryTestGPU

#endif

#endif
