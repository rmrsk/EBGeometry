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
  // A fixed seed gives the same result on every run.
  std::mt19937_64                   rng(12345);
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

  // Evaluate both unions at every point, timing each.
  std::vector<T> slowDist(randomPositions.size());
  std::vector<T> fastDist(randomPositions.size());

  const auto t1 = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < randomPositions.size(); i++) {
    slowDist[i] = slowUnion(randomPositions[i]);
  }

  const auto t2 = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < randomPositions.size(); i++) {
    fastDist[i] = fastUnion.signedDistance(randomPositions[i]);
  }

  const auto t3 = std::chrono::high_resolution_clock::now();

  // Both take the minimum over the same per-sphere values, and the BVH only skips spheres that
  // cannot hold it, so the two agree at every point, up to rounding: the compiler may evaluate the
  // same formula with different rounding in the two loops (contracting a multiply and an add into one
  // fused operation, for instance). The rounding scales with the size of the scene.
  const T tolerance = (std::is_same_v<T, float> ? T(1.0e-5) : T(1.0e-12)) * (hi - lo).length();

  size_t mismatches = 0;

  for (size_t i = 0; i < randomPositions.size(); i++) {
    mismatches += (std::abs(slowDist[i] - fastDist[i]) > tolerance) ? 1 : 0;
  }

  const std::chrono::duration<T, std::micro> slowTime = t2 - t1;
  const std::chrono::duration<T, std::micro> fastTime = t3 - t2;

  std::cout << "Time using slow union (us)   = " << slowTime.count() / Nsamp << "\n";
  std::cout << "Time using fast union (us)   = " << fastTime.count() / Nsamp << "\n\n";
  std::cout << "BVH speedup over naive       = " << (1.0 * slowTime.count()) / (1.0 * fastTime.count()) << "\n";
  std::cout << "Points where the two differ  = " << mismatches << " of " << randomPositions.size() << "\n";

  if (mismatches > 0) {
    std::cerr << "Got wrong distance!" << '\n';

    return 1;
  }

  return 0;
}
