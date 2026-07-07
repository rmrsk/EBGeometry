// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>
#include <cmath>
#include <random>
#include <string>
#include <thread>

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
using SDF    = EBGeometry::SignedDistanceFunction<T>;
using Sphere = EBGeometry::SphereSDF<T>;

using namespace std::chrono_literals;

int
main()
{
  // Tree branching factor
  constexpr int K = 4;

  // Make a sphere array consisting of about M^3 spheres.
  std::vector<std::shared_ptr<Sphere>> spheres;
  std::vector<AABB>                    boundingVolumes;

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

        spheres.emplace_back(std::make_shared<Sphere>(center, radius));
        boundingVolumes.emplace_back(lo, hi);
      }
    }
  }

  // Make a standard union of these spheres. This is the union object which
  // iterates through each and every object in the scene.
  auto slowUnion = EBGeometry::Union<T, Sphere>(spheres);

  // Make a fast (BVH-accelerated) union from the same spheres and their precomputed
  // bounding volumes above: the BVH lets a query skip most of the spheres whose bounding
  // box is nowhere near the query point.
  std::cout << "Partitioning " << spheres.size() << " spheres\n" << '\n';
  const EBGeometry::BVHUnionIF<T, Sphere, AABB, K> fastUnion(spheres, boundingVolumes);

  // A third representation: rather than storing all M^3 spheres explicitly, tile a single
  // sphere periodically. A query point is folded into its nearest tile before evaluating the
  // sphere, so this needs neither a list of spheres nor a search structure -- just the tile
  // period (center-to-center spacing) and how many tiles to repeat along the increasing
  // direction of each axis (0 in the decreasing direction, since the packed lattice above
  // only extends in the positive octant).
  auto sph         = std::make_shared<Sphere>(Vec3::zeros(), radius);
  auto sphereArray = EBGeometry::FiniteRepetition<T, Sphere>(
    sph, (delta + 2 * radius) * Vec3::ones(), Vec3::zeros(), 80.0 * Vec3::ones());

  // Create some samples in the bounding box of the BVH
  std::cout << "Sampling distance fields... \n" << '\n';
  std::mt19937_64 rng(static_cast<size_t>(std::chrono::system_clock::now().time_since_epoch().count()));
  std::uniform_real_distribution<T> dist(0.0, 1.0);

  const AABB& bv = fastUnion.getBoundingVolume();
  const Vec3& lo = bv.getLowCorner();
  const Vec3& hi = bv.getHighCorner();

  std::vector<Vec3> randomPositions;
  for (size_t i = 0; i < Nsamp; i++) {
    const T x = lo[0] + dist(rng) * (hi[0] - lo[0]);
    const T y = lo[1] + dist(rng) * (hi[1] - lo[1]);
    const T z = lo[2] + dist(rng) * (hi[2] - lo[2]);

    randomPositions.emplace_back(x, y, z);
  }

  // Time the results, using the standard union, the optimized union, and the finite repetition.
  std::chrono::duration<T, std::micro> slowTime(0.0);
  std::chrono::duration<T, std::micro> fastTime(0.0);
  std::chrono::duration<T, std::micro> arrayTime(0.0);

  T sumSlow  = 0.0;
  T sumFast  = 0.0;
  T sumArray = 0.0;

  const auto t1 = std::chrono::high_resolution_clock::now();
  for (const auto& x : randomPositions) {
    sumSlow += slowUnion->value(x);
  }
  const auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& x : randomPositions) {
    sumFast += fastUnion.value(x);
  }
  const auto t3 = std::chrono::high_resolution_clock::now();
  for (const auto& x : randomPositions) {
    sumArray += sphereArray->value(x);
  }
  const auto t4 = std::chrono::high_resolution_clock::now();

  if (sumSlow != sumFast || sumSlow != sumArray) {
    std::cerr << "Got wrong distance!" << '\n';

    return 2;
  }

  slowTime += (t2 - t1);
  fastTime += (t3 - t2);
  arrayTime += (t4 - t3);

  std::cout << "Time using slow union (us)   = " << slowTime.count() / Nsamp << "\n";
  std::cout << "Time using fast union (us)   = " << fastTime.count() / Nsamp << "\n";
  std::cout << "Time using sphere array (us) = " << arrayTime.count() / Nsamp << "\n\n";
  std::cout << "BVH speedup over naive       = " << (1.0 * slowTime.count()) / (1.0 * fastTime.count()) << "\n";
  std::cout << "BVH penalty over optimal     = " << (1.0 * fastTime.count()) / (1.0 * arrayTime.count()) << "\n";

  return 0;
}
