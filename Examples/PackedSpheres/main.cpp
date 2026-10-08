// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

// Declare our precision.
using T = EBGEOMETRY_PRECISION;

// Aliases for cutting down on typing.
using AABB   = EBGeometry::BoundingVolumes::AABBT<T>;
using Vec3   = EBGeometry::Vec3T<T>;
using Sphere = EBGeometry::SphereSDF<T>;

int
main()
{
  // Tree branching factor
  constexpr size_t K = 4;

  // Make a sphere array consisting of M^3 spheres.
  std::vector<Sphere> spheres;
  std::vector<AABB>   boundingVolumes;

  constexpr T   radius = 1.0;
  constexpr int M      = 80;
  constexpr int Nsamp  = 1000;
  constexpr T   delta  = radius;

  for (int i = 0; i < M; i++) {
    for (int j = 0; j < M; j++) {
      for (int k = 0; k < M; k++) {

        const T x = i * (delta + 2 * radius);
        const T y = j * (delta + 2 * radius);
        const T z = k * (delta + 2 * radius);

        const Vec3 center(x, y, z);
        const Vec3 lo = center - radius * Vec3::ones();
        const Vec3 hi = center + radius * Vec3::ones();

        spheres.emplace_back(center, radius);
        boundingVolumes.emplace_back(lo, hi);
      }
    }
  }

  // The naive union iterates through each and every sphere in the scene and keeps the smallest
  // distance.
  const auto slowUnion = [&spheres](const Vec3& a_point) -> T {
    T minDist = std::numeric_limits<T>::infinity();

    for (const auto& sphere : spheres) {
      minDist = std::min(minDist, sphere.signedDistance(a_point));
    }

    return minDist;
  };

  // Make a fast (BVH-accelerated) union from the same spheres and their precomputed bounding
  // volumes: the BVH lets a query skip most of the spheres whose bounding box is nowhere near the
  // query point. The union stores the spheres by value in the pool, and is itself a plain value
  // type that could be copied to a GPU and evaluated there.
  std::cout << "Partitioning " << spheres.size() << " spheres\n" << '\n';

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const EBGeometry::BVHUnion<T, Sphere, K> fastUnion(pool, spheres, boundingVolumes);

  // Create some samples in the bounding box of the BVH
  std::cout << "Sampling distance fields... \n" << '\n';
  std::mt19937_64 rng(static_cast<size_t>(std::chrono::system_clock::now().time_since_epoch().count()));
  std::uniform_real_distribution<T> dist(0.0, 1.0);

  const AABB  bv = fastUnion.computeBoundingVolume();
  const Vec3& lo = bv.getLowCorner();
  const Vec3& hi = bv.getHighCorner();

  std::vector<Vec3> randomPositions;

  for (size_t i = 0; i < Nsamp; i++) {
    const T x = lo[0] + dist(rng) * (hi[0] - lo[0]);
    const T y = lo[1] + dist(rng) * (hi[1] - lo[1]);
    const T z = lo[2] + dist(rng) * (hi[2] - lo[2]);

    randomPositions.emplace_back(x, y, z);
  }

  // Time the results, using the naive union and the BVH-accelerated union.
  std::chrono::duration<T, std::micro> slowTime(0.0);
  std::chrono::duration<T, std::micro> fastTime(0.0);

  T sumSlow = 0.0;
  T sumFast = 0.0;

  const auto t1 = std::chrono::high_resolution_clock::now();

  for (const auto& x : randomPositions) {
    sumSlow += slowUnion(x);
  }

  const auto t2 = std::chrono::high_resolution_clock::now();

  for (const auto& x : randomPositions) {
    sumFast += fastUnion.signedDistance(x);
  }

  const auto t3 = std::chrono::high_resolution_clock::now();

  // Summing Nsamp values in a different order (naive scan vs. BVH traversal) is not bit-for-bit
  // reproducible -- floating-point addition isn't associative -- so compare the sums with a
  // relative tolerance rather than requiring exact agreement. float needs a looser tolerance than
  // double: accumulating Nsamp terms in a different order can land right at float's own ~1.19e-7
  // epsilon, which a flat 1e-7 tolerance intermittently flags as a mismatch (observed in CI).
  constexpr T relativeTolerance = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-7);

  const T fastScale = std::max(std::abs(sumSlow), std::abs(sumFast));

  if (std::abs(sumSlow - sumFast) > relativeTolerance * std::max(fastScale, T(1.0))) {
    std::cerr << "Got wrong distance!" << '\n';

    return 2;
  }

  slowTime += (t2 - t1);
  fastTime += (t3 - t2);

  std::cout << "Time using slow union (us)   = " << slowTime.count() / Nsamp << "\n";
  std::cout << "Time using fast union (us)   = " << fastTime.count() / Nsamp << "\n\n";
  std::cout << "BVH speedup over naive       = " << (1.0 * slowTime.count()) / (1.0 * fastTime.count()) << "\n";

  return 0;
}
