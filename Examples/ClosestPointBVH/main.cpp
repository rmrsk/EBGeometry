// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Closest-point search over a random point cloud using the turnkey PointCloudBVH API. The whole
// pipeline -- grouping points into SIMD PointAoSoA leaves, building the PackedBVH, and driving the
// pruned traversal -- is hidden behind a single constructor and two query methods:
//
//   PointCloudBVH<T> bvh(pool, positions);   // build once
//   bvh.closestPoint(q);                     // nearest point to an arbitrary point q
//   bvh.closestPoints(q, k, out);            // the k nearest, ascending by distance
//
// A query reports the point's cloud index (its position in `positions`); per-point user data lives
// in the caller's own array, indexed by it. Every query is checked against a brute-force scan. See
// README.md. The self-query counterpart (nearest neighbors of points already in the cloud) is
// Examples/NearestNeighborBVH.
//
// Compiled for a GPU (GPU=HIP or GPU=CUDA), the same run also answers the queries in a kernel: the
// cloud's pool is copied to device memory, the cloud is rebased onto the copy, and
// EBGeometry::GPU::transform() runs a query functor holding it once per point.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Random.hpp>
#include <Source/EBGeometry_SimpleTimer.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T = EBGEOMETRY_PRECISION;

using Vec3 = EBGeometry::Vec3T<T>;

// The turnkey point-cloud BVH. The branching factor K and SoA leaf width W are template parameters
// that default to the library's values (4, the same on every machine and on a GPU), so we do not
// name them here -- the whole point of this class is that the build and the traversal tuning are
// handled internally.
using Cloud = EBGeometry::PointCloudBVH<T>;

// Run configuration.
constexpr std::size_t   numPoints  = 500000;
constexpr std::size_t   numQueries = 500;
constexpr std::uint64_t pointSeed  = 123456789ULL;
constexpr std::uint64_t querySeed  = 987654321ULL;

namespace {

// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE
// call operator maps one point to one result. It holds the cloud by value; on a device, that is the
// cloud rebased onto device memory.
struct ClosestPoint
{
  Cloud cloud;

  EBGEOMETRY_HOST_DEVICE
  Cloud::Hit
  operator()(const Vec3& a_point) const noexcept
  {
    return cloud.closestPoint(a_point);
  }
};

} // namespace

int
main()
{
  std::cout << "ClosestPointBVH: closest-point search over a " << numPoints << "-point cloud in the unit cube\n";
  std::cout << "  Precision T = " << (std::is_same_v<T, float> ? "float" : "double") << '\n';
  std::cout << "  Points      = " << numPoints << '\n';
  std::cout << "  Queries     = " << numQueries << "\n\n";

  const std::vector<Vec3> positions   = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);
  const std::vector<Vec3> queryPoints = EBGeometry::Random::samplePoints<T>(numQueries, querySeed);

  // Per-point user data stays in the caller's own array, indexed by the cloud index a query reports.
  // Here each point gets a label: which eighth of the unit cube it lies in.
  std::vector<int> octant(numPoints);

  for (std::size_t i = 0; i < numPoints; i++) {
    octant[i] =
      (positions[i][0] > T(0.5) ? 1 : 0) + (positions[i][1] > T(0.5) ? 2 : 0) + (positions[i][2] > T(0.5) ? 4 : 0);
  }

  // Build once. Everything (SoA grouping, PackedBVH construction) happens inside the constructor.
  EBGeometry::SimpleTimer timer;
  timer.start();
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
  const Cloud      bvh(pool, positions);
  timer.stop();
  const double buildSeconds = timer.seconds();

  // Brute-force ground truth for every query, via the library's O(N) reference scan -- no need to
  // hand-roll one here. This also times the baseline the accelerated query beats.
  std::vector<Cloud::Hit> truth(numQueries);
  timer.start();

  for (std::size_t q = 0; q < numQueries; q++) {
    truth[q] = EBGeometry::PointCloud::closestPointBruteForce(
      positions.data(), static_cast<std::uint32_t>(numPoints), queryPoints[q]);
  }

  timer.stop();
  const double bruteSeconds = timer.seconds();

  // Query every point via the BVH, timing the batch.
  const ClosestPoint      closestPoint{bvh};
  std::vector<Cloud::Hit> hits(numQueries);
  timer.start();

  for (std::size_t q = 0; q < numQueries; q++) {
    hits[q] = closestPoint(queryPoints[q]);
  }

  timer.stop();
  const double querySeconds = timer.seconds();

  // Every result must match the brute-force reference, to rounding (the BVH's SIMD distance kernel
  // may round differently from the scalar scan).
  constexpr T tolerance  = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);
  std::size_t mismatches = 0;

  for (std::size_t q = 0; q < numQueries; q++) {
    if (std::abs(hits[q].distanceSquared - truth[q].distanceSquared) >
        tolerance * std::max(truth[q].distanceSquared, T(1.0))) {
      mismatches++;
    }
  }

  std::cout << std::fixed;
  std::cout << "  Build         : " << std::setprecision(1) << 1.0e3 * buildSeconds << " ms\n";
  std::cout << "  Brute force   : " << std::setprecision(3) << 1.0e6 * bruteSeconds / double(numQueries)
            << " us/query\n";
  std::cout << "  PointCloudBVH : " << std::setprecision(3) << 1.0e6 * querySeconds / double(numQueries) << " us/query"
            << "   (" << std::setprecision(1) << bruteSeconds / querySeconds << "x faster)\n";
  std::cout << "  Mismatches    : " << mismatches << " of " << numQueries << "\n";

  // ── The same queries on a GPU ──────────────────────────────────────────────────────────────────────
  //
  // The cloud is built on the host. Its pool is frozen and copied to device memory in one piece, and
  // the cloud is rebased onto the copy; the rebased cloud is what the kernel receives, inside the query.
  // This is the pattern for large geometry: the device reads its own copy at full speed. The device
  // results agree with the host's to rounding, since a device compiler may fuse a multiply and an add
  // where the host compiler does not. The time includes copying the queries and results.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  if (!EBGeometry::GPU::deviceAvailable()) {
    std::cout << "  On the device : skipped, no GPU is visible\n";
  }
  else {
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::deviceMemoryResource());
    const ClosestPoint     onDevice{bvh.rebasedView(devicePool)};

    timer.start();
    const std::vector<Cloud::Hit> deviceHits = EBGeometry::GPU::transform(onDevice, queryPoints);
    timer.stop();

    // Distances, not cloud indices: points at equal distance can tie either way under other rounding.
    std::size_t differ = 0;

    for (std::size_t q = 0; q < numQueries; q++) {
      if (std::abs(deviceHits[q].distanceSquared - hits[q].distanceSquared) >
          tolerance * std::max(hits[q].distanceSquared, T(1.0))) {
        differ++;
      }
    }

    std::cout << "  On the device : " << std::setprecision(3) << 1.0e6 * timer.seconds() / double(numQueries)
              << " us/query (" << EBGeometry::GPU::backendName << "), " << differ << " differ from the host\n";

    mismatches += differ;
  }
#else
  std::cout << "  On the device : not built (plain C++); compile with GPU=HIP or GPU=CUDA to run it\n";
#endif

  std::cout << '\n';

  // The k-nearest form: the k closest points to one external point, nearest first.
  constexpr std::size_t k = 5;
  Cloud::Hit            nearest[k];
  const std::size_t     found = bvh.closestPoints(queryPoints[0], k, nearest);
  std::cout << "  closestPoints(query[0], " << k << "): " << found << " nearest, ascending by distance:\n";

  for (std::size_t j = 0; j < found; j++) {
    std::cout << "    #" << j << "  cloud index " << std::setw(7) << nearest[j].index << "   distance "
              << std::setprecision(5) << std::sqrt(nearest[j].distanceSquared) << "   (octant "
              << octant[nearest[j].index] << ")\n";
  }

  return mismatches == 0 ? 0 : 1;
}
