// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Nearest-neighbor search over a random point cloud using the turnkey PointCloudHashGrid API -- the
// uniform-grid counterpart to Examples/NearestNeighborBVH. PointCloudHashGrid exposes the same query
// interface as PointCloudBVH (same Hit, same methods), so switching structures is a one-line change:
//
//   PointCloudHashGrid<T> grid(positions, metadata);   // build once (counting-sort into cells)
//   grid.nearestNeighbor(i);                             // nearest OTHER point to point i
//   auto graph = grid.allNearestNeighbors(kNN);          // kNN nearest of EVERY point, batched
//
// For a near-uniform cloud the grid builds and queries faster than the tree; for a clustered cloud
// PointCloudBVH's density adaptivity wins (see Examples/NearestNeighborBVH). A sample of the batch
// result is checked against the class's own O(N) brute-force reference. See README.md.

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

// The turnkey point-cloud grid. Templated only on precision T and the user metadata type.
using PointCloud = EBGeometry::PointCloudHashGrid<T, std::size_t>;

// Run configuration. kNN is the number of nearest neighbors computed for every point.
constexpr std::size_t   numPoints  = 500000;
constexpr std::size_t   kNN        = 1;
constexpr std::size_t   sampleSize = 500; // how many points to verify against brute force
constexpr std::uint64_t pointSeed  = 123456789ULL;

int
main()
{
  std::cout << "NearestNeighborHashGrid: the " << kNN << " nearest neighbor(s) of every point in a " << numPoints
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

  // Build once (counting-sort the cloud into a uniform grid).
  EBGeometry::SimpleTimer timer;
  timer.start();
  const PointCloud grid(positions, metadata);
  timer.stop();
  const double buildSeconds = timer.seconds();

  // The whole kNN graph in one batched, spatially-ordered call.
  timer.start();
  const std::vector<PointCloud::Hit> graph = grid.allNearestNeighbors(kNN);
  timer.stop();
  const double graphSeconds = timer.seconds();

  // Verify a spread sample against the class's own O(N) brute-force reference, and time that sample to
  // extrapolate a fair per-point baseline (a full N^2 all-pairs scan is infeasible at this size).
  constexpr T tolerance = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);

  std::vector<PointCloud::Hit> truth(kNN);
  const std::size_t            stride = numPoints / sampleSize;
  timer.start();

  for (std::size_t s = 0; s < sampleSize; s++) {
    const std::size_t i = s * stride;
    grid.nearestNeighborsBruteForce(i, kNN, truth.data());

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
  std::cout << "  Build             : " << std::setprecision(1) << 1.0e3 * buildSeconds << " ms\n";
  std::cout << "  Brute force       : " << std::setprecision(3) << 1.0e6 * bruteSecondsPerPoint << " us/point\n";
  std::cout << "  PointCloudHashGrid: " << std::setprecision(3) << 1.0e6 * graphSecondsPerPoint << " us/point"
            << "   (" << std::setprecision(1) << bruteSecondsPerPoint / graphSecondsPerPoint << "x faster)\n\n";

  // The single-point form: the nearest OTHER point to one point in the cloud.
  const PointCloud::Hit nn = grid.nearestNeighbor(0);
  std::cout << "  nearestNeighbor(0): cloud index " << nn.index << " at distance " << std::setprecision(5)
            << std::sqrt(nn.distanceSquared) << "   (metadata " << grid.metadata(nn.index) << ")\n";

  return 0;
}
