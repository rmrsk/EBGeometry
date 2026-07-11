// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Closest-point search over a random point cloud using the turnkey PointCloudBVH API. The whole
// pipeline -- grouping points into SIMD PointAoSoA leaves, building the PackedBVH, and driving the
// pruned traversal -- is hidden behind a single constructor and two query methods:
//
//   PointCloudBVH<T> bvh(positions, metadata);   // build once
//   bvh.closestPoint(q);                          // nearest particle to an arbitrary point q
//   bvh.closestPoints(q, k, out);                 // the k nearest, ascending by distance
//
// Every query is checked against a brute-force scan. See README.md. The self-query counterpart
// (nearest neighbors of points already in the cloud) is Examples/NearestNeighborBVH.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T = EBGEOMETRY_PRECISION;

using Vec3 = EBGeometry::Vec3T<T>;

// The turnkey point-cloud BVH. The branching factor K and SoA leaf width W are template parameters
// that default to the SIMD-optimal values for T, so we do not name them here -- the whole point of
// this class is that the build and the traversal tuning are handled internally.
using PointCloud = EBGeometry::PointCloudBVH<T, std::size_t>;

// Run configuration.
constexpr std::size_t   numPoints  = 500000;
constexpr std::size_t   numQueries = 500;
constexpr std::uint64_t pointSeed  = 123456789ULL;
constexpr std::uint64_t querySeed  = 987654321ULL;

int
main()
{
  std::cout << "ClosestPointBVH: closest-point search over a " << numPoints << "-point cloud in the unit cube\n";
  std::cout << "  Precision T = " << (std::is_same_v<T, float> ? "float" : "double") << '\n';
  std::cout << "  Points      = " << numPoints << '\n';
  std::cout << "  Queries     = " << numQueries << "\n\n";

  const std::vector<Vec3> positions   = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);
  const std::vector<Vec3> queryPoints = EBGeometry::Random::samplePoints<T>(numQueries, querySeed);

  // Per-particle user metadata. PointCloudBVH stores each particle's cloud index internally (that is
  // what queries return); this parallel array is whatever the user wants to hang off a particle and
  // recover with metadata(). Here we tag each point with its own index to keep the example concrete.
  std::vector<std::size_t> metadata(numPoints);
  for (std::size_t i = 0; i < numPoints; i++) {
    metadata[i] = i;
  }

  // Build once. Everything (SoA grouping, PackedBVH construction) happens inside the constructor.
  EBGeometry::SimpleTimer timer;
  timer.start();
  const PointCloud bvh(positions, metadata);
  timer.stop();
  const double buildSeconds = timer.seconds();

  // Brute-force ground truth for every query, via the class's own O(N) reference method -- no need to
  // hand-roll a scan here. This also times the baseline the accelerated query beats.
  std::vector<PointCloud::Hit> truth(numQueries);
  timer.start();
  for (std::size_t q = 0; q < numQueries; q++) {
    truth[q] = bvh.closestPointBruteForce(queryPoints[q]);
  }
  timer.stop();
  const double bruteSeconds = timer.seconds();

  constexpr T tolerance = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);

  // Query every point via the BVH, timing the batch. EBGEOMETRY_EXPECT (opt-in via
  // EBGEOMETRY_ENABLE_ASSERTIONS) checks every result against the brute-force reference; a volatile
  // sink keeps the query live in no-assertion builds.
  volatile std::size_t sink = 0;
  timer.start();
  for (std::size_t q = 0; q < numQueries; q++) {
    const PointCloud::Hit hit = bvh.closestPoint(queryPoints[q]);

    EBGEOMETRY_EXPECT(std::abs(hit.distanceSquared - truth[q].distanceSquared) <=
                      tolerance * std::max(truth[q].distanceSquared, T(1.0)));

    sink += hit.index;
  }
  timer.stop();
  (void)sink;
  const double querySeconds = timer.seconds();

  std::cout << std::fixed;
  std::cout << "  Build         : " << std::setprecision(1) << 1.0e3 * buildSeconds << " ms\n";
  std::cout << "  Brute force   : " << std::setprecision(3) << 1.0e6 * bruteSeconds / double(numQueries)
            << " us/query\n";
  std::cout << "  PointCloudBVH : " << std::setprecision(3) << 1.0e6 * querySeconds / double(numQueries) << " us/query"
            << "   (" << std::setprecision(1) << bruteSeconds / querySeconds << "x faster)\n\n";

  // The k-nearest form: the k closest particles to one external point, nearest first.
  constexpr std::size_t k = 5;
  PointCloud::Hit       nearest[k];
  const std::size_t     found = bvh.closestPoints(queryPoints[0], k, nearest);
  std::cout << "  closestPoints(query[0], " << k << "): " << found << " nearest, ascending by distance:\n";
  for (std::size_t j = 0; j < found; j++) {
    std::cout << "    #" << j << "  cloud index " << std::setw(7) << nearest[j].index << "   distance "
              << std::setprecision(5) << std::sqrt(nearest[j].distanceSquared) << "   (metadata "
              << bvh.metadata(nearest[j].index) << ")\n";
  }

  return 0;
}
