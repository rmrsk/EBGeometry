// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
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
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;
using Vec3 = EBGeometry::Vec3T<T>;
using Box  = EBGeometry::BoxSDF<T>;

int
main()
{
  // TLDR: This program places some random boxes on a lattice; the boxes represent buildings in a "random city". The buildings are placed on a
  //       an MxM sized lattice where the building width, length, and height are drawn from a uniform distribution with user-specified parameters.

  // Building parameters:
  //
  // K         : BVH tree branching factor
  // N         : Number of random points to sample.
  // M         : Number of building *lots* per coordinate direction, indexed 0..M inclusive,
  //             so the lattice actually places (M+1) x (M+1) buildings.
  // dx        : Minimum separation between the buildings.
  // Wmin, Wmax: Minimum and maximum building width
  // Lmin, Lmax: Minimum and maximum building length
  // Hmin, Hmax: Minimum and maximum building height

  constexpr int    N    = 500;
  constexpr size_t K    = 4;
  constexpr int    M    = 234;
  constexpr T      dx   = 0.1;
  constexpr T      Wmin = 1;
  constexpr T      Wmax = 3;
  constexpr T      Lmin = 1;
  constexpr T      Lmax = 3;
  constexpr T      Hmin = 1;
  constexpr T      Hmax = 20;

  std::cout << "Domain is (0,0,0) to " << Vec3(M * (Wmax + dx), M * (Lmax + dx), M * (Hmax + dx)) << "\n";

  // Generate some random buildings on a lattice -- none of these should overlap.
  std::vector<Box>  buildings;
  std::vector<AABB> boundingVolumes;

  // A fixed seed gives the same result on every run.
  std::mt19937_64                   rng(12345);
  std::uniform_real_distribution<T> udist(0, 1.0);

  for (int i = 0; i <= M; i++) {
    for (int j = 0; j <= M; j++) {
      const T W = Wmin + udist(rng) * (Wmax - Wmin);
      const T L = Lmin + udist(rng) * (Lmax - Lmin);
      const T H = Hmin + udist(rng) * (Hmax - Hmin);

      const T xLo = i * (Wmax + dx) + 0.5 * (dx + Wmax - W);
      const T xHi = xLo + W;

      const T yLo = j * (Lmax + dx) + 0.5 * (dx + Lmax - L);
      const T yHi = yLo + L;

      const Vec3 lo(xLo, yLo, 0.0);
      const Vec3 hi(xHi, yHi, H);

      buildings.emplace_back(lo, hi);
      boundingVolumes.emplace_back(lo, hi);
    }
  }

  // Create a standard and an optimized union. The standard union scans every building; the optimized
  // one stores the buildings by value in a pool, behind a BVH over their bounding boxes.
  std::cout << "Partitioning " << buildings.size() << " buildings" << '\n';

  const auto slowUnion = [&buildings](const Vec3& a_point) -> T {
    T minDist = std::numeric_limits<T>::infinity();

    for (const auto& building : buildings) {
      minDist = std::min(minDist, building.signedDistance(a_point));
    }

    return minDist;
  };

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto fastUnion = EBGeometry::BVHUnion<T, Box, K>(pool, buildings, boundingVolumes);

  // Sample some random points in the bounding box of the BVH.
  Vec3 lo = Vec3::infinity();
  Vec3 hi = -Vec3::infinity();

  for (const auto& b : buildings) {
    lo = min(lo, b.getLowCorner());
    hi = max(hi, b.getHighCorner());
  }

  std::vector<Vec3> randomPositions;

  for (int i = 0; i < N; i++) {
    const T x = lo[0] + udist(rng) * (hi[0] - lo[0]);
    const T y = lo[1] + udist(rng) * (hi[1] - lo[1]);
    const T z = lo[2] + udist(rng) * (hi[2] - lo[2]);

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

  // Both take the minimum over the same per-building values, and the BVH only skips buildings that
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

  std::cout << "Time using slow union (us) = " << slowTime.count() / N << "\n";
  std::cout << "Time using fast union (us) = " << fastTime.count() / N << "\n";
  std::cout << "Average speedup = " << (1.0 * slowTime.count()) / (1.0 * fastTime.count()) << "\n";
  std::cout << "Points where the two differ = " << mismatches << " of " << randomPositions.size() << "\n";

  if (mismatches > 0) {
    std::cerr << "Got wrong distance!" << '\n';

    return 1;
  }

  return 0;
}
