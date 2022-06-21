/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <string>
#include <chrono>
#include <thread>
#include <random>
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

  // Make a sphere array consisting of about M^3 spheres. We assume
  // that the domain is x,y,z \in [-1,1] and set the number of spheres
  // and their radii.
  std::vector<std::shared_ptr<SDF>> spheres;

  constexpr T   radius = 0.02;
  constexpr int M      = 40;
  constexpr int Nsamp  = 1000;
  constexpr T   delta  = (2.0 - 2 * M * radius) / (M + 1);

  if (delta < 0.0) {
    std::cerr << "Error: 'delta < 0.0'" << std::endl;

    return 1;
  }
  else {
    std::cout << "delta = " << delta << std::endl;
  }

  for (int i = 1; i <= M; i++) {
    for (int j = 1; j <= M; j++) {
      for (int k = 1; k <= M; k++) {

        const T x = -1. + i * (delta + 2 * radius) - radius;
        const T y = -1. + j * (delta + 2 * radius) - radius;
        const T z = -1. + k * (delta + 2 * radius) - radius;

        Vec3 center(x, y, z);

        spheres.emplace_back(std::make_shared<Sphere>(center, radius, false));
      }
    }
  }

  // Make a standard union of these spheres. This is the union object which
  // iterates through each and every object in the scene.
  EBGeometry::Union<T> slowUnion(spheres, false);

  // Make a fast union. To do this we must have the SDF objects (our vector of
  // spheres) as well as a way for enclosing these objects. We need to define
  // ourselves a lambda that creates an appropriate bounding volumes for each
  // SDF.
  EBGeometry::BVH::BVConstructorT<SDF, AABB> aabbConstructor = [](const std::shared_ptr<const SDF>& a_prim) {
    const Sphere& sph = static_cast<const Sphere&>(*a_prim);

    const Vec3& c = sph.getCenter();
    const T&    r = sph.getRadius();

    const Vec3 lo = c - r * Vec3::one();
    const Vec3 hi = c + r * Vec3::one();

    return AABB(lo, hi);
  };

  std::cout << "Partitioning spheres" << std::endl;
  EBGeometry::UnionBVH<T, AABB, K> fastUnion(spheres, false, aabbConstructor);

  std::mt19937_64                   rng(std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<T> dist(-1.0, 1.0);

  std::chrono::duration<T, std::micro> slowTime(0.0);
  std::chrono::duration<T, std::micro> fastTime(0.0);

  std::vector<Vec3> ranPoints;
  for (size_t i = 0; i < Nsamp; i++) {
    ranPoints.emplace_back(Vec3(dist(rng), dist(rng), dist(rng)));
  }

  T sumSlow = 0.0;
  T sumFast = 0.0;

  const auto t1 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumSlow += slowUnion.value(x);
  }
  const auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
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
