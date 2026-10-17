// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <algorithm>
#include <cmath>
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

namespace {

// The points and zeroed room for the results in one pool, mirrored into device memory in one piece.
// The query runs through the span overload or the pool overload, and the results come back to the
// host; the inputs must come back unchanged.
template <class T>
std::vector<T>
runInMirroredPool(const SphereDistance<T>& a_query, const std::vector<Vec3T<T>>& a_points, const bool a_poolOverload)
{
  const auto n = static_cast<uint32_t>(a_points.size());

  Pool                 hostPool(hostMemoryResource());
  PODVector<Vec3T<T>>  in;
  PODVector<T>         out;
  const std::vector<T> zeros(n, T(0));

  in.reserveFrom(hostPool, n);
  out.reserveFrom(hostPool, n);
  in.assign(hostPool.base(), a_points.data(), n);
  out.assign(hostPool.base(), zeros.data(), n);
  hostPool.freeze();

  const Pool devicePool = Pool::mirror(hostPool, EBGeometryTestGPU::deviceTestResource());

  if (a_poolOverload) {
    GPU::transform(a_query, devicePool, in, out);
  }
  else {
    const void* const deviceBase = devicePool.base();

    GPU::transform(a_query, in.bind(deviceBase), out.bind(devicePool.base()));
  }

  const Pool back = Pool::mirror(devicePool, hostMemoryResource());

  std::vector<T> results(n);

  for (uint32_t i = 0; i < n; i++) {
    REQUIRE(in.at(back.base(), i) == a_points[i]);

    results[i] = out.at(back.base(), i);
  }

  return results;
}

} // namespace

TEMPLATE_TEST_CASE("GPU::transform over spans and over a pool: inputs and results stay in a mirrored pool",
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
  const std::vector<T>        onHost = evaluateOnHost<T>(query, points);

  requireSameResults(runInMirroredPool(query, points, false), onHost);
  requireSameResults(runInMirroredPool(query, points, true), onHost);
}

TEMPLATE_TEST_CASE("GPU::transform over vectors: the same results at any block size, into a reused output",
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
  const std::vector<T>        onHost = evaluateOnHost<T>(query, points);

  // One output, written over by every call.
  std::vector<T> results(points.size(), T(0));

  GPU::transform(query, points, results);
  requireSameResults(results, onHost);

  std::fill(results.begin(), results.end(), T(0));
  GPU::transform<64>(query, points, results);
  requireSameResults(results, onHost);

  std::fill(results.begin(), results.end(), T(0));
  GPU::transform<512>(query, points, results);
  requireSameResults(results, onHost);

  const std::vector<Vec3T<T>> none;
  std::vector<T>              noResults;

  GPU::transform(query, none, noResults);

  CHECK(noResults.empty());
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

TEST_CASE("GPU::parallelFor: more indices than a grid can hold abort", "[GPU][death]")
{
  REQUIRE(abortsWith(
    [] {
      // One past what 2^31 - 1 blocks of 64 threads cover. The functor's span is empty, so the check
      // must fire before anything runs.
      const std::size_t tooMany = ((std::size_t(1) << 31) - 1) * 64 + 1;

      GPU::parallelFor<64>(tooMany, WriteIndex{PODSpan<uint32_t>{nullptr, 0}}); // must abort
    },
    "GPU::parallelFor: at most (2^31 - 1) * 64 indices at this block size"));
}

TEST_CASE("GPU::transform over vectors: as many results as inputs, or abort", "[GPU][death]")
{
  REQUIRE(abortsWith(
    [] {
      const SphereDistance<double> query{SphereSDF<double>(Vec3T<double>::zeros(), 1.0)};

      const std::vector<Vec3T<double>> points(4, Vec3T<double>::ones());
      std::vector<double>              results; // not resized for you

      GPU::transform(query, points, results); // must abort
    },
    "GPU::transform: there must be as many results as inputs (4 inputs, 0 results)"));
}

TEST_CASE("GPU::transform over a pool: the pool must be frozen and device-accessible, or abort", "[GPU][death]")
{
  const SphereDistance<double> query{SphereSDF<double>(Vec3T<double>::zeros(), 1.0)};

  // Stages two points and two results in a pool over a_resource, frozen or not, and runs the query.
  const auto run = [query](MemoryResource& a_resource, const bool a_freeze) {
    Pool                     pool(a_resource);
    PODVector<Vec3T<double>> in;
    PODVector<double>        out;
    const Vec3T<double>      points[2] = {Vec3T<double>::ones(), Vec3T<double>::zeros()};
    const double             zeros[2]  = {0.0, 0.0};

    in.reserveFrom(pool, 2);
    out.reserveFrom(pool, 2);
    in.assign(pool.base(), points, 2);
    out.assign(pool.base(), zeros, 2);

    if (a_freeze) {
      pool.freeze();
    }

    GPU::transform(query, pool, in, out);
  };

  // A host pool is rejected in every build, including a host build where the emulated device memory
  // would be accepted.
  REQUIRE(abortsWith([&run] { run(hostMemoryResource(), true); },
                     "GPU::transform: the pool must be in device-accessible memory"));

  // Built directly in (emulated) device-accessible memory but never frozen.
  REQUIRE(
    abortsWith([&run] { run(emulatedDeviceMemoryResource(), false); }, "GPU::transform: the pool must be frozen"));
}

namespace {

// A query whose input and result types are the same, so that the results can overwrite the inputs.
template <class T>
struct Twice
{
  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const T& a_value) const noexcept
  {
    return T(2) * a_value;
  }
};

} // namespace

TEMPLATE_TEST_CASE("GPU::transform over a pool: a pool built in place, and results written over the inputs",
                   "[GPU][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  // Managed memory in a GPU build; in a host build, emulated device memory, which takes the same path:
  // the arrays are written in place, the pool is frozen, and nothing is mirrored.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  MemoryResource& resource = managedMemoryResource();
#else
  MemoryResource& resource = emulatedDeviceMemoryResource();
#endif

  const SphereDistance<T>     query{SphereSDF<T>(Vec3T<T>(T(0.2), T(0), T(-0.1)), T(0.5))};
  const std::vector<Vec3T<T>> points = queryGrid<T>(Vec3T<T>(T(-1), T(-1), T(-1)), Vec3T<T>(T(1), T(1), T(1)), 9);
  const auto                  n      = static_cast<uint32_t>(points.size());
  const std::vector<T>        zeros(n, T(0));

  std::vector<T> values(n);

  for (uint32_t i = 0; i < n; i++) {
    values[i] = T(i) / T(7);
  }

  Pool                pool(resource);
  PODVector<Vec3T<T>> in;
  PODVector<T>        out;
  PODVector<T>        inPlace;

  in.reserveFrom(pool, n);
  out.reserveFrom(pool, n);
  inPlace.reserveFrom(pool, n);
  in.assign(pool.base(), points.data(), n);
  out.assign(pool.base(), zeros.data(), n);
  inPlace.assign(pool.base(), values.data(), n);
  pool.freeze();

  GPU::transform(query, pool, in, out);
  GPU::transform(Twice<T>{}, pool, inPlace, inPlace);

  // Managed and emulated memory are both readable from the host once the call has returned.
  std::vector<T> onDevice(n);

  for (uint32_t i = 0; i < n; i++) {
    onDevice[i] = out.at(pool.base(), i);

    REQUIRE(inPlace.at(pool.base(), i) == T(2) * values[i]);
  }

  requireSameResults(onDevice, evaluateOnHost<T>(query, points));
}

TEST_CASE("GPU::transform over a pool: arrays that end past the pool abort", "[GPU][death]")
{
  REQUIRE(abortsWith(
    [] {
      const SphereDistance<double> query{SphereSDF<double>(Vec3T<double>::zeros(), 1.0)};

      // Two small arrays in a frozen pool, and a third reserved from another pool further along, so that
      // its offset runs past the first pool.
      Pool                     pool(emulatedDeviceMemoryResource());
      Pool                     other(emulatedDeviceMemoryResource());
      PODVector<Vec3T<double>> in;
      PODVector<double>        out;
      PODVector<double>        stray;

      in.reserveFrom(pool, 2);
      out.reserveFrom(pool, 2);
      (void)other.reserve(4096, 1, 1);
      stray.reserveFrom(other, 2);
      pool.freeze();

      GPU::transform(query, pool, in, stray); // must abort
    },
    "GPU::transform: the arrays must fit inside the pool"));
}

TEST_CASE("GPU::transform over spans: inputs and results that partly overlap abort", "[GPU][death]")
{
  REQUIRE(abortsWith(
    [] {
      std::vector<double> values(8, 1.0);

      // The results start one element into the inputs.
      GPU::transform(Twice<double>{}, PODSpan<const double>{values.data(), 4}, PODSpan<double>{values.data() + 1, 4});
    },
    "GPU::transform: the inputs and the results must be the same array or not overlap"));
}

namespace {

// Memory both the host and a launch can reach: managed memory in a GPU build, host memory otherwise.
MemoryResource&
sharedResource() noexcept
{
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  return managedMemoryResource();
#else
  return hostMemoryResource();
#endif
}

// A descending order, at namespace scope so that a kernel may be instantiated on it.
struct Descending
{
  EBGEOMETRY_HOST_DEVICE
  bool
  operator()(const uint64_t a_lhs, const uint64_t a_rhs) const noexcept
  {
    return a_lhs > a_rhs;
  }
};

} // namespace

TEST_CASE("GPU::bitonicSort sorts any power-of-two count of keys", "[GPU]")
{
  if (GPU::enabled && !EBGeometryTestGPU::deviceAvailable()) {
    SKIP("no GPU device available");
  }

  for (const uint32_t n : {0U, 1U, 2U, 8U, 1024U}) {
    INFO("keys " << n);

    Pool pool(sharedResource());

    PODVector<uint64_t> keys;

    keys.reserveSizedFrom(pool, n);

    uint64_t* data = keys.data(pool.base());

    // A fixed pseudo-random sequence, with repeats.
    uint64_t state = 12345;

    for (uint32_t i = 0; i < n; i++) {
      state   = state * 6364136223846793005ULL + 1442695040888963407ULL;
      data[i] = state >> 54;
    }

    std::vector<uint64_t> expected(data, data + n);

    std::sort(expected.begin(), expected.end());

    GPU::bitonicSort(data, n);

    CHECK(std::vector<uint64_t>(data, data + n) == expected);

    // And descending, through the order functor.
    GPU::bitonicSort(data, n, Descending{});

    CHECK(std::vector<uint64_t>(data, data + n) == std::vector<uint64_t>(expected.rbegin(), expected.rend()));
  }
}

TEST_CASE("GPU::boundingBox is the exact box of the points", "[GPU]")
{
  if (GPU::enabled && !EBGeometryTestGPU::deviceAvailable()) {
    SKIP("no GPU device available");
  }

  using Vec3 = Vec3T<double>;

  for (const uint32_t n : {1U, 255U, 256U, 257U, 3000U}) {
    INFO("points " << n);

    Pool pool(sharedResource());

    PODVector<Vec3> points;
    PODVector<Vec3> scratch;

    points.reserveSizedFrom(pool, n);
    scratch.reserveSizedFrom(pool, GPU::boundingBoxScratchSize(n));

    Vec3* data = points.data(pool.base());

    Vec3 lo = Vec3::max();
    Vec3 hi = -Vec3::max();

    for (uint32_t i = 0; i < n; i++) {
      data[i] = Vec3(std::sin(0.1 * i) * i, std::cos(0.3 * i), 0.01 * i - 7.0);
      lo      = min(lo, data[i]);
      hi      = max(hi, data[i]);
    }

    const auto box = GPU::boundingBox(static_cast<const Vec3*>(data), n, scratch.data(pool.base()));

    CHECK(box.getLowCorner() == lo);
    CHECK(box.getHighCorner() == hi);
  }

  // No points: the empty box.
  const auto empty = GPU::boundingBox<double>(nullptr, 0, nullptr);

  CHECK_FALSE(empty.getLowCorner()[0] <= empty.getHighCorner()[0]);
}

TEST_CASE("GPU::bitonicSort: a count that is not a power of two aborts", "[GPU][death]")
{
  REQUIRE(abortsWith(
    [] {
      uint64_t keys[3] = {3, 1, 2};

      GPU::bitonicSort(keys, 3);
    },
    "GPU::bitonicSort: the key count must be a power of two (3)"));
}

TEST_CASE("GPU::exclusiveScan gives every exclusive prefix sum and the total", "[GPU]")
{
  if (GPU::enabled && !EBGeometryTestGPU::deviceAvailable()) {
    SKIP("no GPU device available");
  }

  for (const uint32_t n : {0U, 1U, 2U, 16U, 1024U}) {
    INFO("values " << n);

    Pool pool(sharedResource());

    PODVector<uint32_t> values;
    PODVector<uint32_t> total;

    values.reserveSizedFrom(pool, n);
    total.reserveSizedFrom(pool, 1);

    uint32_t* data = values.data(pool.base());

    std::vector<uint32_t> expected(n);

    uint32_t sum = 0;

    for (uint32_t i = 0; i < n; i++) {
      data[i]     = (i * 7U) % 5U;
      expected[i] = sum;
      sum += data[i];
    }

    total.at(pool.base(), 0) = 12345;

    GPU::exclusiveScan(data, n, total.data(pool.base()));

    CHECK(std::vector<uint32_t>(data, data + n) == expected);
    CHECK(total.at(pool.base(), 0) == (n == 0 ? 12345U : sum));
  }

  REQUIRE(abortsWith(
    [] {
      uint32_t values[3] = {1, 2, 3};

      GPU::exclusiveScan(values, 3);
    },
    "GPU::exclusiveScan: the value count must be a power of two (3)"));
}
