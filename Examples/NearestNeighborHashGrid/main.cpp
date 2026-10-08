// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Nearest-neighbor search over a random point cloud using the turnkey PointCloudHashGrid API -- the
// uniform-grid counterpart to Examples/NearestNeighborBVH. PointCloudHashGrid exposes the same query
// interface as PointCloudBVH (same result type, same methods), so switching structures is a one-line
// change:
//
//   PointCloudHashGrid<T> grid(pool, positions);   // build once (counting-sort into cells)
//   grid.nearestNeighbor(i);                       // nearest OTHER point to point i
//   auto graph = grid.allNearestNeighbors(kNN);    // kNN nearest of EVERY point, batched
//
// For a near-uniform cloud the grid builds faster than the tree and queries about as fast; for a
// clustered cloud PointCloudBVH's density adaptivity wins (see Examples/NearestNeighborBVH). A
// sample of the batch result is checked against the library's O(N) brute-force reference. See
// README.md.

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

// The turnkey point-cloud grid. Templated only on precision T.
using Cloud = EBGeometry::PointCloudHashGrid<T>;

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

  // Per-point user data stays in the caller's own array, indexed by the cloud index a query reports;
  // see Examples/ClosestPointBVH. Here each point gets a label: which eighth of the unit cube it lies in.
  std::vector<int> octant(numPoints);

  for (std::size_t i = 0; i < numPoints; i++) {
    octant[i] =
      (positions[i][0] > T(0.5) ? 1 : 0) + (positions[i][1] > T(0.5) ? 2 : 0) + (positions[i][2] > T(0.5) ? 4 : 0);
  }

  // Build once (counting-sort the cloud into a uniform grid).
  EBGeometry::SimpleTimer timer;
  timer.start();
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
  const Cloud      grid(pool, positions);
  timer.stop();
  const double buildSeconds = timer.seconds();

  // The whole kNN graph in one batched, spatially-ordered call.
  timer.start();
  const std::vector<Cloud::Hit> graph = grid.allNearestNeighbors(kNN);
  timer.stop();
  const double graphSeconds = timer.seconds();

  // Verify a spread sample against the library's O(N) brute-force reference (no hand-rolled scan),
  // and time that sample to extrapolate a fair per-point baseline (a full N^2 all-pairs scan is
  // infeasible at this size). Results must match to rounding.
  constexpr T tolerance = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);

  std::vector<Cloud::Hit> truth(kNN);
  const std::size_t       stride     = numPoints / sampleSize;
  std::size_t             mismatches = 0;
  timer.start();

  for (std::size_t s = 0; s < sampleSize; s++) {
    const auto i = static_cast<std::uint32_t>(s * stride);

    EBGeometry::PointCloud::closestPointsBruteForce(
      positions.data(), static_cast<std::uint32_t>(numPoints), positions[i], kNN, truth.data(), i);

    for (std::size_t j = 0; j < kNN; j++) {
      const T got = graph[i * kNN + j].distanceSquared;

      if (std::abs(got - truth[j].distanceSquared) > tolerance * std::max(truth[j].distanceSquared, T(1.0))) {
        mismatches++;
      }
    }
  }

  timer.stop();
  const double bruteSecondsPerPoint = timer.seconds() / double(sampleSize);

  const double graphSecondsPerPoint = graphSeconds / double(numPoints);

  std::cout << std::fixed;
  std::cout << "  Build             : " << std::setprecision(1) << 1.0e3 * buildSeconds << " ms\n";
  std::cout << "  Brute force       : " << std::setprecision(3) << 1.0e6 * bruteSecondsPerPoint << " us/point\n";
  std::cout << "  PointCloudHashGrid: " << std::setprecision(3) << 1.0e6 * graphSecondsPerPoint << " us/point"
            << "   (" << std::setprecision(1) << bruteSecondsPerPoint / graphSecondsPerPoint << "x faster)\n";
  std::cout << "  Mismatches        : " << mismatches << " of " << sampleSize * kNN << " sampled\n\n";

  // The single-point form: the nearest OTHER point to one point in the cloud.
  const Cloud::Hit nn = grid.nearestNeighbor(0);
  std::cout << "  nearestNeighbor(0): cloud index " << nn.index << " at distance " << std::setprecision(5)
            << std::sqrt(nn.distanceSquared) << "   (octant " << octant[nn.index] << ")\n";

  return mismatches == 0 ? 0 : 1;
}
