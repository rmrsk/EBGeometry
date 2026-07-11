// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Nearest-neighbor search over a random point cloud using the turnkey PointCloudBVH API. Where
// Examples/ClosestPointBVH queries with arbitrary external points, this queries with points that are
// *already in the cloud* -- the classic k-nearest-neighbor graph -- which lets the class seed each
// search from the point's own leaf (a strictly cheaper traversal). The whole pipeline is again
// hidden behind the constructor and one call:
//
//   PointCloudBVH<T> bvh(positions, metadata);       // build once
//   bvh.nearestNeighbor(i);                            // nearest OTHER point to point i
//   auto graph = bvh.allNearestNeighbors(kNN);         // kNN nearest of EVERY point, batched
//
// A sample of the batch result is checked against a brute-force scan. See README.md.

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

// The turnkey point-cloud BVH. K (branching) and W (SoA leaf width) default to the SIMD-optimal
// values for T, so they are not named here.
using PointCloud = EBGeometry::PointCloudBVH<T, std::size_t>;

// Run configuration. kNN is the number of nearest neighbors computed for every point.
constexpr std::size_t   numPoints  = 500000;
constexpr std::size_t   kNN        = 1;
constexpr std::size_t   sampleSize = 500; // how many points to verify against brute force
constexpr std::uint64_t pointSeed  = 123456789ULL;

int
main()
{
  std::cout << "NearestNeighborBVH: the " << kNN << " nearest neighbor(s) of every point in a " << numPoints
            << "-point cloud\n";
  std::cout << "  Precision T = " << (std::is_same_v<T, float> ? "float" : "double") << '\n';
  std::cout << "  Points      = " << numPoints << '\n';
  std::cout << "  kNN         = " << kNN << "\n\n";

  const std::vector<Vec3> positions = EBGeometry::Random::samplePoints<T>(numPoints, pointSeed);

  // Per-point user metadata; see Examples/ClosestPointBVH. Tag each point with its own index.
  std::vector<std::size_t> metadata(numPoints);

  for (std::size_t i = 0; i < numPoints; i++) {
    metadata[i] = i;
  }

  // Build once.
  EBGeometry::SimpleTimer timer;
  timer.start();
  const PointCloud bvh(positions, metadata);
  timer.stop();
  const double buildSeconds = timer.seconds();

  // The whole kNN graph in one batched, Hilbert-ordered call.
  timer.start();
  const std::vector<PointCloud::Hit> graph = bvh.allNearestNeighbors(kNN);
  timer.stop();
  const double graphSeconds = timer.seconds();

  // Verify a spread sample against the class's own O(N) brute-force reference (no hand-rolled scan),
  // and time that sample to extrapolate a fair per-point baseline (a full N^2 all-pairs scan is
  // infeasible at this size).
  constexpr T tolerance = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);

  std::vector<PointCloud::Hit> truth(kNN);
  const std::size_t            stride = numPoints / sampleSize;
  timer.start();

  for (std::size_t s = 0; s < sampleSize; s++) {
    const std::size_t i = s * stride;
    bvh.nearestNeighborsBruteForce(i, kNN, truth.data());

    for (std::size_t j = 0; j < kNN; j++) {
      const T got = graph[i * kNN + j].distanceSquared;
      EBGEOMETRY_EXPECT(std::abs(got - truth[j].distanceSquared) <=
                        tolerance * std::max(truth[j].distanceSquared, T(1.0)));
      (void)got;
    }
  }

  timer.stop();
  const double bruteSecondsPerPoint = timer.seconds() / double(sampleSize);

  const double graphSecondsPerPoint = graphSeconds / double(numPoints);

  std::cout << std::fixed;
  std::cout << "  Build         : " << std::setprecision(1) << 1.0e3 * buildSeconds << " ms\n";
  std::cout << "  Brute force   : " << std::setprecision(3) << 1.0e6 * bruteSecondsPerPoint << " us/point\n";
  std::cout << "  PointCloudBVH : " << std::setprecision(3) << 1.0e6 * graphSecondsPerPoint << " us/point"
            << "   (" << std::setprecision(1) << bruteSecondsPerPoint / graphSecondsPerPoint << "x faster)\n\n";

  // The single-point form: the nearest OTHER point to one point in the cloud.
  const PointCloud::Hit nn = bvh.nearestNeighbor(0);
  std::cout << "  nearestNeighbor(0): cloud index " << nn.index << " at distance " << std::setprecision(5)
            << std::sqrt(nn.distanceSquared) << "   (metadata " << bvh.metadata(nn.index) << ")\n";

  return 0;
}
