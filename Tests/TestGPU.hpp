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
// evaluateOnDevice() is the library's EBGeometry::GPU::transform(), so the tests run the same launch
// code that users call. Under a GPU backend (EBGEOMETRY_CUDA/EBGEOMETRY_HIP) the functor runs in a real
// kernel over memory from deviceMemoryResource(). Every runtime call is checked, including the kernel
// launch and its completion, and a failure aborts with the call that failed. The tests skip when no
// device is visible.
//
// In a host build the same tests run in emulation: deviceTestResource() is the library's
// emulatedDeviceMemoryResource(), host memory that reports itself device-accessible, so a
// rebasedView() onto it takes the device-view path (a snapshot of the location, no control block), and
// GPU::transform() passes the functor by value to a host loop. That
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

/*!
  @brief Whether the device tests can run: a GPU is visible in a CUDA or HIP build, and always in a
  host build, where they run in emulation.
  @return True if the device tests can run.
*/
inline bool
deviceAvailable() noexcept
{
  return !EBGeometry::GPU::enabled || EBGeometry::GPU::deviceAvailable();
}

/*!
  @brief The memory resource a device test mirrors its pools into: EBGeometry::GPU::memoryResource(),
  which is device memory in a CUDA or HIP build and emulated device memory (host memory that reports
  itself device-accessible) otherwise.
  @return The memory resource.
*/
inline EBGeometry::MemoryResource&
deviceTestResource() noexcept
{
  return EBGeometry::GPU::memoryResource();
}

/*!
  @brief Evaluate a functor over a list of queries on the device, one thread per query.
  @details EBGeometry::GPU::transform(), the library's own: under a GPU backend a kernel over many
  blocks of 128 threads, so a result that depends on the thread or block index shows up, with every
  runtime call checked (a failure aborts and names the call). In a host build the functor is copied
  once, as a launch copies it, and run in a loop. The functor must be trivially copyable and hold
  device views (rebasedView() onto a mirror in deviceTestResource()) of anything pool-resident.
  @tparam R Result type.
  @param[in] a_f       Functor with an EBGEOMETRY_HOST_DEVICE R operator()(const Q&).
  @param[in] a_queries Queries.
  @return One result per query.
*/
template <class R, class Q, class F>
std::vector<R>
evaluateOnDevice(const F& a_f, const std::vector<Q>& a_queries)
{
  std::vector<EBGeometry::GPU::TransformResult<F, Q>> results(a_queries.size());

  EBGeometry::GPU::transform(a_f, a_queries, results);

  return std::vector<R>(results.begin(), results.end());
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
