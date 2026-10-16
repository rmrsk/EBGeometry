// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

// ─────────────────────────────────────────────────────────────────────────────
// GPU::memoryResource
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("GPU::memoryResource: device memory in a GPU build, emulated device memory otherwise", "[GPU]")
{
  MemoryResource& resource = GPU::memoryResource();

  CHECK(resource.isDeviceAccessible());

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  CHECK(&resource == &static_cast<MemoryResource&>(deviceMemoryResource()));
#else
  CHECK(&resource == &static_cast<MemoryResource&>(emulatedDeviceMemoryResource()));
  CHECK(resource.isHostAccessible());
  CHECK_FALSE(GPU::deviceAvailable());
#endif
}

TEST_CASE("EmulatedDeviceMemoryResource: host memory that a mirror and a rebased view treat as a device's",
          "[GPU][MemoryResource]")
{
  EmulatedDeviceMemoryResource& resource = emulatedDeviceMemoryResource();

  CHECK(resource.isHostAccessible());
  CHECK(resource.isDeviceAccessible());

  // A pool mirrored into it is a byte-identical host copy.
  Pool                  pool(hostMemoryResource());
  PODVector<uint32_t>   values;
  const uint32_t        n = 17;
  std::vector<uint32_t> source(n);

  for (uint32_t i = 0; i < n; i++) {
    source[i] = 7 * i + 3;
  }

  values.reserveFrom(pool, n);
  values.assign(pool.base(), source.data(), n);
  pool.freeze();

  const Pool mirror = Pool::mirror(pool, resource);

  REQUIRE(mirror.isFrozen());
  REQUIRE(mirror.base() != pool.base());

  for (uint32_t i = 0; i < n; i++) {
    CHECK(values.at(mirror.base(), i) == source[i]);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// GPU::parallelFor
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Writes a value computed from the index into a span: every index must be written exactly once.
struct WriteIndex
{
  PODSpan<uint32_t> m_out;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    m_out[static_cast<uint32_t>(a_i)] += 3 * static_cast<uint32_t>(a_i) + 1;
  }
};

// Runs WriteIndex over n indices in a pool mirrored into the device memory, and returns what it wrote.
template <unsigned int BlockSize>
std::vector<uint32_t>
writeIndices(const uint32_t a_n)
{
  Pool                        hostPool(hostMemoryResource());
  PODVector<uint32_t>         out;
  const std::vector<uint32_t> zeros(a_n, 0);

  out.reserveFrom(hostPool, a_n);
  out.assign(hostPool.base(), zeros.data(), a_n);
  hostPool.freeze();

  const Pool devicePool = Pool::mirror(hostPool, EBGeometryTestGPU::deviceTestResource());

  GPU::parallelFor<BlockSize>(a_n, WriteIndex{out.bind(devicePool.base())});

  const Pool back = Pool::mirror(devicePool, hostMemoryResource());

  std::vector<uint32_t> result(a_n);

  for (uint32_t i = 0; i < a_n; i++) {
    result[i] = out.at(back.base(), i);
  }

  return result;
}

template <unsigned int BlockSize>
void
checkEveryIndexOnce(const uint32_t a_n)
{
  INFO("block size " << BlockSize << ", " << a_n << " indices");

  const std::vector<uint32_t> result = writeIndices<BlockSize>(a_n);

  for (uint32_t i = 0; i < a_n; i++) {
    REQUIRE(result[i] == 3 * i + 1);
  }
}

} // namespace

TEST_CASE("GPU::parallelFor: runs the functor once for every index, at any block size", "[GPU][gpu]")
{
  if (!EBGeometryTestGPU::deviceAvailable()) {
    SKIP("no GPU device available");
  }

  // Counts below, at and above a block, and one that leaves a partial last block.
  for (const uint32_t n : {1u, 64u, 128u, 1000u, 4099u}) {
    checkEveryIndexOnce<64>(n);
    checkEveryIndexOnce<GPU::DefaultBlockSize>(n);
    checkEveryIndexOnce<256>(n);
    checkEveryIndexOnce<1024>(n);
  }
}

TEST_CASE("GPU::parallelFor: zero indices launch nothing", "[GPU]")
{
  // A functor whose span is empty would write out of bounds if it ran.
  GPU::parallelFor(0, WriteIndex{PODSpan<uint32_t>{nullptr, 0}});
}

// ─────────────────────────────────────────────────────────────────────────────
// GPU::transform
// ─────────────────────────────────────────────────────────────────────────────

namespace {

template <class T>
struct SphereDistance
{
  SphereSDF<T> m_sphere;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_sphere.signedDistance(a_point);
  }
};

} // namespace

TEMPLATE_TEST_CASE("GPU::transform over spans: inputs and results stay in a mirrored pool",
                   "[GPU][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const SphereDistance<T> query{SphereSDF<T>(Vec3T<T>(T(0.1), T(-0.2), T(0.3)), T(0.7))};

  const std::vector<Vec3T<T>> points = queryGrid<T>(Vec3T<T>(T(-1), T(-1), T(-1)), Vec3T<T>(T(1), T(1), T(1)), 13);
  const auto                  n      = static_cast<uint32_t>(points.size());

  // The points and room for the results in one pool, mirrored into device memory in one piece.
  Pool                 hostPool(hostMemoryResource());
  PODVector<Vec3T<T>>  in;
  PODVector<T>         out;
  const std::vector<T> zeros(n, T(0));

  in.reserveFrom(hostPool, n);
  out.reserveFrom(hostPool, n);
  in.assign(hostPool.base(), points.data(), n);
  out.assign(hostPool.base(), zeros.data(), n);
  hostPool.freeze();

  const Pool devicePool = Pool::mirror(hostPool, deviceTestResource());

  const void* const deviceBase = devicePool.base();

  GPU::transform(query, in.bind(deviceBase), out.bind(devicePool.base()));

  // Bring the results back and compare with the host, element by element.
  const Pool back = Pool::mirror(devicePool, hostMemoryResource());

  std::vector<T> onDevice(n);

  for (uint32_t i = 0; i < n; i++) {
    onDevice[i] = out.at(back.base(), i);
  }

  requireSameResults(onDevice, evaluateOnHost<T>(query, points));

  // The inputs are still there: the call wrote only the results.
  for (uint32_t i = 0; i < n; i++) {
    REQUIRE(in.at(back.base(), i) == points[i]);
  }
}

TEMPLATE_TEST_CASE("GPU::transform over vectors: the same results at any block size",
                   "[GPU][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const SphereDistance<T> query{SphereSDF<T>(Vec3T<T>::zeros(), T(1))};

  const std::vector<Vec3T<T>> points = queryGrid<T>(Vec3T<T>(T(-2), T(-2), T(-2)), Vec3T<T>(T(2), T(2), T(2)), 11);

  const std::vector<T> onHost = evaluateOnHost<T>(query, points);

  requireSameResults(GPU::transform(query, points), onHost);
  requireSameResults(GPU::transform<64>(query, points), onHost);
  requireSameResults(GPU::transform<512>(query, points), onHost);

  CHECK(GPU::transform(query, std::vector<Vec3T<T>>{}).empty());
}

TEST_CASE("GPU::transform over spans: as many results as inputs, or abort", "[GPU][death]")
{
  REQUIRE(abortsWith(
    [] {
      const SphereDistance<double> query{SphereSDF<double>(Vec3T<double>::zeros(), 1.0)};

      std::vector<Vec3T<double>> points(4, Vec3T<double>::ones());
      std::vector<double>        results(3);

      GPU::transform(
        query, PODSpan<const Vec3T<double>>{points.data(), 4}, PODSpan<double>{results.data(), 3}); // must abort
    },
    "GPU::transform: there must be as many results as inputs"));
}
