// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Shared helpers for the in-suite device checks. The per-class test files (TestVec,
// TestBoundingVolumes, TestPool, TestPointAoSoA, TestTriangleSoA, ...) each carry a [gpu] test case
// that runs the class's device-callable queries through a kernel and compares them with the host.
//
// The pattern for a device test:
//
//   1. Write the check as a functor: a trivially copyable struct holding the objects under test (a
//      rebasedView() onto a mirror in deviceTestResource() for anything pool-resident) with an
//      EBGEOMETRY_HOST_DEVICE call operator that maps one query to one result. A struct rather than a
//      lambda, since nvcc accepts device lambdas only with --extended-lambda.
//   2. Run it over many queries with evaluateOnDevice() -- one thread per query, many blocks -- and
//      with evaluateOnHost() on the host-side objects.
//   3. Compare the two element by element with requireSameResults(), so a wrong sign or a wrong
//      element cannot hide in a sum.
//
// Under a GPU backend (EBGEOMETRY_CUDA/EBGEOMETRY_HIP) the functor runs in a real kernel over memory
// from deviceMemoryResource(). Every runtime call is checked, including the kernel launch and its
// completion, and a failure is a test failure that names the call. The tests skip when no device is
// visible.
//
// In a host build the same tests run in emulation: deviceTestResource() is host memory that reports
// itself device-accessible, so a rebasedView() onto it takes the device-view path (a snapshot of the
// location, no control block), and evaluateOnDevice() passes the functor by value to a host loop. That
// runs the device-view code and the functors in every ordinary build and under the sanitizers; only
// the device compiler, the address-space split and the kernel launch itself need a GPU lane.

#ifndef EBGEOMETRY_TESTGPU_HPP
#define EBGEOMETRY_TESTGPU_HPP

#include "EBGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace EBGeometryTestGPU {

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

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
  @brief Fail the current test unless a runtime call succeeded.
  @param[in] a_error Result of the call.
  @param[in] a_what  What the call did, for the message.
*/
inline void
requireSuccess(const GPU::Error a_error, const char* a_what)
{
  INFO(a_what << ": " << GPU::getErrorString(a_error));
  REQUIRE(a_error == GPU::Success);
}

/*!
  @brief RAII owner of an n-element device array.
  @tparam T Trivially copyable element type.
*/
template <class T>
class DeviceArray
{
public:
  static_assert(std::is_trivially_copyable_v<T>, "DeviceArray requires a trivially copyable type");

  /*!
    @brief Allocate a device array and copy the host elements into it.
    @param[in] a_host Elements to copy.
  */
  explicit DeviceArray(const std::vector<T>& a_host) : m_size(a_host.size())
  {
    requireSuccess(GPU::memAlloc(reinterpret_cast<void**>(&m_data), sizeof(T) * m_size), "device allocation");
    requireSuccess(GPU::memcpy(m_data, a_host.data(), sizeof(T) * m_size, GPU::MemcpyHostToDevice),
                   "copy to the device");
  }

  /*!
    @brief Allocate an uninitialized device array.
    @param[in] a_size Number of elements.
  */
  explicit DeviceArray(const std::size_t a_size) : m_size(a_size)
  {
    requireSuccess(GPU::memAlloc(reinterpret_cast<void**>(&m_data), sizeof(T) * m_size), "device allocation");
  }

  DeviceArray(const DeviceArray&) = delete;
  DeviceArray&
  operator=(const DeviceArray&) = delete;

  /*!
    @brief Free the device allocation.
  */
  ~DeviceArray() noexcept
  {
    if (m_data != nullptr) {
      (void)GPU::memFree(m_data);
    }
  }

  /*!
    @brief The device pointer, for passing to a kernel.
    @return Device pointer to the first element.
  */
  T*
  get() const noexcept
  {
    return m_data;
  }

  /*!
    @brief Copy the elements back to the host.
    @return The elements.
  */
  std::vector<T>
  download() const
  {
    std::vector<T> host(m_size);

    requireSuccess(GPU::memcpy(host.data(), m_data, sizeof(T) * m_size, GPU::MemcpyDeviceToHost),
                   "copy back from the device");

    return host;
  }

private:
  T*          m_data = nullptr;
  std::size_t m_size = 0;
};

/*!
  @brief Kernel: out[i] = f(in[i]) for every i < n, one thread per element.
*/
template <class F, class Q, class R>
EBGEOMETRY_GLOBAL
void
evaluateKernel(const F a_f, const Q* a_in, R* a_out, const unsigned int a_n)
{
  const unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

  if (i < a_n) {
    a_out[i] = a_f(a_in[i]);
  }
}

/*!
  @brief Evaluate a functor over a list of queries on the device, one thread per query.
  @details The functor is passed to the kernel by value, so it must be trivially copyable and hold
  device views (rebasedView()) of anything pool-resident. The launch uses many blocks of 128 threads,
  so a result that depends on the thread or block index shows up. Launch and execution errors fail
  the test.
  @tparam R Result type.
  @param[in] a_f       Functor with an EBGEOMETRY_HOST_DEVICE R operator()(const Q&).
  @param[in] a_queries Queries.
  @return One result per query.
*/
template <class R, class Q, class F>
std::vector<R>
evaluateOnDevice(const F& a_f, const std::vector<Q>& a_queries)
{
  static_assert(std::is_trivially_copyable_v<F>, "a device functor must be trivially copyable");

  const auto n = static_cast<unsigned int>(a_queries.size());

  DeviceArray<Q> in(a_queries);
  DeviceArray<R> out(a_queries.size());

  constexpr unsigned int threads = 128;
  const unsigned int     blocks  = (n + threads - 1) / threads;

  evaluateKernel<F, Q, R><<<blocks, threads>>>(a_f, in.get(), out.get(), n);

  requireSuccess(GPU::getLastError(), "kernel launch");
  requireSuccess(GPU::deviceSynchronize(), "kernel execution");

  return out.download();
}

/*!
  @brief The memory resource a device test mirrors its pools into.
  @return The device memory resource.
*/
inline EBGeometry::MemoryResource&
deviceTestResource() noexcept
{
  return EBGeometry::deviceMemoryResource();
}

#else

/*!
  @brief Host memory that reports itself device-accessible, standing in for device memory in a host
  build.
  @details A view rebased onto a pool in this resource takes the same path as a device view: its
  location is a snapshot with no control block.
*/
class EmulatedDeviceResource final : public EBGeometry::MemoryResource
{
public:
  void*
  allocate(size_t a_bytes, size_t a_alignment) override
  {
    return EBGeometry::hostMemoryResource().allocate(a_bytes, a_alignment);
  }

  void
  deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept override
  {
    EBGeometry::hostMemoryResource().deallocate(a_ptr, a_bytes, a_alignment);
  }

  bool
  isHostAccessible() const noexcept override
  {
    return true;
  }

  bool
  isDeviceAccessible() const noexcept override
  {
    return true;
  }
};

/*!
  @brief In emulation there is always a "device".
  @return True.
*/
inline bool
deviceAvailable() noexcept
{
  return true;
}

/*!
  @brief The memory resource a device test mirrors its pools into.
  @return The emulated device resource.
*/
inline EBGeometry::MemoryResource&
deviceTestResource() noexcept
{
  static EmulatedDeviceResource s_resource;

  return s_resource;
}

/*!
  @brief Evaluate a functor over a list of queries as a kernel would, on the host.
  @details The functor is copied first, as a kernel launch copies its arguments, so it must be
  trivially copyable.
  @tparam R Result type.
  @param[in] a_f       Functor with an EBGEOMETRY_HOST_DEVICE R operator()(const Q&).
  @param[in] a_queries Queries.
  @return One result per query.
*/
template <class R, class Q, class F>
std::vector<R>
evaluateOnDevice(const F& a_f, const std::vector<Q>& a_queries)
{
  static_assert(std::is_trivially_copyable_v<F>, "a device functor must be trivially copyable");

  const F        f = a_f;
  std::vector<R> results(a_queries.size());

  for (std::size_t i = 0; i < a_queries.size(); i++) {
    results[i] = f(a_queries[i]);
  }

  return results;
}

#endif

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

/*!
  @brief Evaluate a functor over a list of queries on the host.
  @tparam R Result type.
  @param[in] a_f       Functor.
  @param[in] a_queries Queries.
  @return One result per query.
*/
template <class R, class Q, class F>
std::vector<R>
evaluateOnHost(const F& a_f, const std::vector<Q>& a_queries)
{
  std::vector<R> results;

  results.reserve(a_queries.size());

  for (const Q& q : a_queries) {
    results.push_back(a_f(q));
  }

  return results;
}

/*!
  @brief Require each device result to match the host result, element by element.
  @details The host and device evaluate the same arithmetic, differing only by fused-multiply-add
  contraction and reassociation, so each pair must agree to gpuTol<T>() relative to the larger
  magnitude (or absolutely, near zero). Signs are compared as they are.
  @param[in] a_device Device results.
  @param[in] a_host   Host results.
*/
template <class T>
void
requireSameResults(const std::vector<T>& a_device, const std::vector<T>& a_host)
{
  static_assert(std::is_floating_point_v<T>, "requireSameResults compares floating-point results");

  REQUIRE(a_device.size() == a_host.size());

  for (std::size_t i = 0; i < a_host.size(); i++) {
    const T scale = std::max(T(1), std::max(std::abs(a_device[i]), std::abs(a_host[i])));

    INFO("query " << i << ": device " << a_device[i] << ", host " << a_host[i]);
    REQUIRE(std::abs(a_device[i] - a_host[i]) <= gpuTol<T>() * scale);
  }
}

/*!
  @brief Query points on a regular grid over a box, offset so that none lies on a grid-aligned face.
  @param[in] a_lo Lower corner.
  @param[in] a_hi Upper corner.
  @param[in] a_n  Points per axis.
  @return a_n^3 points.
*/
template <class T>
std::vector<EBGeometry::Vec3T<T>>
queryGrid(const EBGeometry::Vec3T<T>& a_lo, const EBGeometry::Vec3T<T>& a_hi, const int a_n)
{
  std::vector<EBGeometry::Vec3T<T>> points;

  points.reserve(static_cast<std::size_t>(a_n * a_n * a_n));

  const EBGeometry::Vec3T<T> step = (a_hi - a_lo) / T(a_n);

  for (int i = 0; i < a_n; i++) {
    for (int j = 0; j < a_n; j++) {
      for (int k = 0; k < a_n; k++) {
        const EBGeometry::Vec3T<T> u(T(i) + T(0.37), T(j) + T(0.53), T(k) + T(0.61));

        points.emplace_back(a_lo + u * step);
      }
    }
  }

  return points;
}

} // namespace EBGeometryTestGPU

#endif
