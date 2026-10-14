// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// k-nearest-neighbor queries on a PointCloudBVH, run in a GPU kernel. The same source builds as plain
// C++ (the default), where the queries run in a host loop instead, or as CUDA or HIP, where they run on
// the device:
//
//   Pool pool(hostMemoryResource());
//   const Cloud cloud(pool, positions);                     // build on the host
//   pool.freeze();
//   Pool devicePool = Pool::mirror(pool, deviceMemoryResource());
//   const Cloud deviceCloud = cloud.rebasedView(devicePool); // pass this to a kernel by value
//
// The cloud is random points in the unit cube plus a few planted points outside it, at known places.
// Queries next to a planted point must find it first, at a known distance, and every query's k
// nearest points must match the library's brute-force scan. See README.md.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Random.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T = EBGEOMETRY_PRECISION;

using Vec3  = EBGeometry::Vec3T<T>;
using Cloud = EBGeometry::PointCloudBVH<T>;
using Hit   = Cloud::Hit;

// Run configuration.
constexpr std::size_t   numPoints     = 100000;
constexpr std::size_t   numRandom     = 1000; // random queries, checked against brute force
constexpr std::size_t   kNearest      = 4;
constexpr std::uint64_t pointSeed     = 20261014ULL;
constexpr std::uint64_t querySeed     = 42ULL;
constexpr T             plantedOffset = T(0.01); // how far each planted query sits from its point

// ── Running a query over many inputs: a kernel on a GPU, a loop on the host ─────────────────────────
//
// A query is a trivially copyable functor whose call operator is EBGEOMETRY_HOST_DEVICE. It holds what
// it searches by value (here a PointCloudBVH that resolves against the memory the query runs next to),
// is passed to the kernel by value, and maps one input to one result.
namespace {

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

// Kernel: a_out[i] = a_query(a_in[i]), one thread per input.
template <class Query, class In, class Out>
EBGEOMETRY_GLOBAL
void
queryKernel(const Query a_query, const In* a_in, Out* a_out, const unsigned int a_n)
{
  const unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

  if (i < a_n) {
    a_out[i] = a_query(a_in[i]);
  }
}

// Stop with a message if a GPU runtime call failed.
void
check(const EBGeometry::GPU::Error a_error, const char* a_what)
{
  if (a_error != EBGeometry::GPU::Success) {
    std::cerr << "GPU error in " << a_what << ": " << EBGeometry::GPU::getErrorString(a_error) << '\n';
    std::exit(EXIT_FAILURE);
  }
}

constexpr const char* runsOn = "a GPU kernel";

// Copy the inputs to the device, run the kernel, and copy the results back.
template <class Out, class In, class Query>
std::vector<Out>
runQuery(const Query& a_query, const std::vector<In>& a_inputs)
{
  static_assert(std::is_trivially_copyable_v<Query>, "a query is passed to the kernel by value");

  const auto n = static_cast<unsigned int>(a_inputs.size());

  In*  in  = nullptr;
  Out* out = nullptr;

  check(EBGeometry::GPU::memAlloc(reinterpret_cast<void**>(&in), sizeof(In) * n), "memAlloc");
  check(EBGeometry::GPU::memAlloc(reinterpret_cast<void**>(&out), sizeof(Out) * n), "memAlloc");
  check(EBGeometry::GPU::memcpy(in, a_inputs.data(), sizeof(In) * n, EBGeometry::GPU::MemcpyHostToDevice), "memcpy");

  constexpr unsigned int threads = 128;

  queryKernel<<<(n + threads - 1) / threads, threads>>>(a_query, in, out, n);

  check(EBGeometry::GPU::getLastError(), "kernel launch");
  check(EBGeometry::GPU::deviceSynchronize(), "kernel execution");

  std::vector<Out> results(n);

  check(EBGeometry::GPU::memcpy(results.data(), out, sizeof(Out) * n, EBGeometry::GPU::MemcpyDeviceToHost), "memcpy");
  check(EBGeometry::GPU::memFree(in), "memFree");
  check(EBGeometry::GPU::memFree(out), "memFree");

  return results;
}

#else

constexpr const char* runsOn = "a host loop (plain C++ build)";

// The same query, run in a loop on the host.
template <class Out, class In, class Query>
std::vector<Out>
runQuery(const Query& a_query, const std::vector<In>& a_inputs)
{
  std::vector<Out> results;

  results.reserve(a_inputs.size());

  for (const In& input : a_inputs) {
    results.push_back(a_query(input));
  }

  return results;
}

#endif

// ── The query ────────────────────────────────────────────────────────────────────────────────────────

// The k nearest cloud points to one query point, nearest first.
struct Neighbors
{
  Hit hits[kNearest];
};

struct KNearestQuery
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  Neighbors
  operator()(const Vec3& a_point) const noexcept
  {
    Neighbors result;

    m_cloud.closestPoints(a_point, kNearest, result.hits);

    return result;
  }
};

bool
matches(const T a_value, const T a_expected)
{
  const T tolerance = std::is_same_v<T, float> ? T(1.0e-5) : T(1.0e-12);

  return std::abs(a_value - a_expected) <= tolerance * (T(1) + std::abs(a_expected));
}

} // namespace

int
main()
{
  std::cout << "DevicePointCloud: the " << kNearest << " nearest of " << numPoints + 8 << " points, from " << runsOn
            << "\n";
  std::cout << "  Precision T = " << (std::is_same_v<T, float> ? "float" : "double") << "\n\n";

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  int devices = 0;

  if (EBGeometry::GPU::getDeviceCount(&devices) != EBGeometry::GPU::Success || devices == 0) {
    std::cerr
      << "This is a GPU build, but no GPU is visible. Rebuild it as plain C++ to run the queries on the host.\n";

    return EXIT_FAILURE;
  }
#endif

  // Random points in the unit cube, then eight planted points well outside it, at the corners of the
  // cube [-1, 2]^3. Each is the only point near its corner.
  std::vector<Vec3> positions = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);

  const std::uint32_t firstPlanted = static_cast<std::uint32_t>(positions.size());

  for (int corner = 0; corner < 8; corner++) {
    positions.emplace_back(T((corner & 1) ? 2 : -1), T((corner & 2) ? 2 : -1), T((corner & 4) ? 2 : -1));
  }

  // Build on the host. In a GPU build, copy the finished pool to device memory in one piece and rebase
  // the cloud onto the copy: the rebased cloud is what the kernel receives. The pools must outlive every
  // query.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
  const Cloud      cloud(pool, positions);

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  pool.freeze();

  const EBGeometry::Pool devicePool  = EBGeometry::Pool::mirror(pool, EBGeometry::deviceMemoryResource());
  const Cloud            deviceCloud = cloud.rebasedView(devicePool);
#else
  const Cloud& deviceCloud = cloud;
#endif

  // The queries: one next to each planted point, then random points in a box around the cloud.
  std::vector<Vec3> queries;

  for (std::uint32_t p = firstPlanted; p < positions.size(); p++) {
    queries.push_back(positions[p] + plantedOffset * Vec3::ones());
  }

  for (const Vec3& q : EBGeometry::Random::samplePoints<T>(numRandom, querySeed)) {
    queries.push_back(T(1.4) * q - T(0.2) * Vec3::ones());
  }

  const std::vector<Neighbors> found = runQuery<Neighbors>(KNearestQuery{deviceCloud}, queries);

  std::size_t failures = 0;

  // A query next to a planted point finds that point first, at the planted offset.
  const T plantedDistance2 = T(3) * plantedOffset * plantedOffset;

  for (std::uint32_t c = 0; c < 8; c++) {
    const Hit& nearest = found[c].hits[0];

    if (nearest.index != firstPlanted + c || !matches(nearest.distanceSquared, plantedDistance2)) {
      std::cout << "  planted point " << c << ": found point " << nearest.index << " at squared distance "
                << nearest.distanceSquared << ", expected point " << firstPlanted + c << " at " << plantedDistance2
                << '\n';
      failures++;
    }
  }

  // Every query's k nearest points match a brute-force scan, distance for distance. (Indices are not
  // compared: two points at the same distance may come back in either order.)
  for (std::size_t q = 0; q < queries.size(); q++) {
    Hit expected[kNearest];

    EBGeometry::PointCloud::closestPointsBruteForce(
      positions.data(), static_cast<std::uint32_t>(positions.size()), queries[q], kNearest, expected);

    for (std::size_t j = 0; j < kNearest; j++) {
      if (!matches(found[q].hits[j].distanceSquared, expected[j].distanceSquared)) {
        failures++;
      }
    }
  }

  std::cout << "  Planted-point queries : 8\n";
  std::cout << "  Random queries        : " << numRandom << "\n";
  std::cout << "  Mismatches            : " << failures << "\n";

  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
