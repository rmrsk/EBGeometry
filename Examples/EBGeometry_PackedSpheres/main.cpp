/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <string>
#include <chrono>
#include <random>
#include <thread>
#include <math.h>

#include "../../EBGeometry.hpp"

// Declare our precision.
using T = double;

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
        const Vec3 lo = center - radius * Vec3::one();
        const Vec3 hi = center + radius * Vec3::one();

        spheres.emplace_back(std::make_shared<Sphere>(center, radius));
        boundingVolumes.emplace_back(AABB(lo, hi));
      }
    }
  }

  // Make a standard union of these spheres. This is the union object which
  // iterates through each and every object in the scene.
  auto slowUnion = EBGeometry::Union<T, Sphere>(spheres);

  // Make a fast union. To do this we must have the SDF objects (our vector of
  // spheres) as well as a way for enclosing these objects. We need to define
  // ourselves a lambda that creates an appropriate bounding volumes for each
  // SDF.
  std::cout << "Partitioning " << std::pow(M, 3) << " spheres" << std::endl;
  EBGeometry::FastUnionIF<T, Sphere, AABB, K> fastUnion(spheres, boundingVolumes);

  // Create some samples in the bounding box of the BVH
  std::mt19937_64 rng(static_cast<size_t>(std::chrono::system_clock::now().time_since_epoch().count()));
  std::uniform_real_distribution<T> dist(0.0, 1.0);

  const AABB& bv = fastUnion.getBoundingVolume();
  const Vec3  lo = bv.getLowCorner();
  const Vec3  hi = bv.getHighCorner();

  std::vector<Vec3> randomPositions;
  for (size_t i = 0; i < Nsamp; i++) {
    const T x = lo[0] + dist(rng) * (hi[0] - lo[0]);
    const T y = lo[1] + dist(rng) * (hi[1] - lo[1]);
    const T z = lo[2] + dist(rng) * (hi[2] - lo[2]);

    randomPositions.emplace_back(Vec3(x, y, z));
  }

  // Time the results, using the standard union and the optimized union.
  std::chrono::duration<T, std::micro> slowTime(0.0);
  std::chrono::duration<T, std::micro> fastTime(0.0);

  T sumSlow = 0.0;
  T sumFast = 0.0;

  const auto t1 = std::chrono::high_resolution_clock::now();
  for (const auto& x : randomPositions) {
    sumSlow += slowUnion->value(x);
  }
  const auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& x : randomPositions) {
    sumFast += fastUnion.value(x);
  }
  const auto t3 = std::chrono::high_resolution_clock::now();

  if (sumSlow != sumFast) {
    std::cerr << "Got wrong distance!" << std::endl;

    return 2;
  }

  slowTime += (t2 - t1);
  fastTime += (t3 - t2);

  std::cout << "Time using slow union (us) = " << slowTime.count() / Nsamp << "\n";
  std::cout << "Time using fast union (us) = " << fastTime.count() / Nsamp << "\n";
  std::cout << "Average speedup = " << (1.0 * slowTime.count()) / (1.0 * fastTime.count()) << "\n";

  return 0;
}
