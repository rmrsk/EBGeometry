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
  constexpr int Nsamp  = 100;
  constexpr T   delta  = (2.0 - 2 * M * radius)/(M+1);

  if(delta < 0.0) {
    std::cerr << "Error: 'delta < 0.0'" << std::endl;
    
    return 1;
  }

  for (int i = 1; i <= M; i++) {
    for (int j = 1; j <= M; j++) {
      for (int k = 1; k <= M; k++) {

	const T x = -1. + i * (delta + 2*radius) - radius;
        const T y = -1. + j * (delta + 2*radius) - radius;
        const T z = -1. + k * (delta + 2*radius) - radius;

	Vec3 center(x,y,z);

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

  for (size_t i = 0; i < Nsamp; i++) {
    // Time the computation distance using both the slow and fast unions.
    const Vec3 point = Vec3::one();

    const auto t1       = std::chrono::high_resolution_clock::now();
    const T    slowDist = slowUnion.value(point);
    const auto t2       = std::chrono::high_resolution_clock::now();

    const auto t3       = std::chrono::high_resolution_clock::now();
    const T    fastDist = fastUnion.value(point);
    const auto t4       = std::chrono::high_resolution_clock::now();

    if(fastDist != slowDist) {
      std::cerr << "Got wrong distance!" << std::endl;

      return 2;
    }

    const std::chrono::duration<T, std::milli> slowTime = t2 - t1;
    const std::chrono::duration<T, std::milli> fastTime = t4 - t3;    
  }

  // std::cout << "Distance and time using standard union = " << slowDist << ", which took " << slowTime.count()
  //           << " ms\n";
  // std::cout << "Distance and time using optimize union = " << fastDist << ", which took " << fastTime.count()
  //           << " ms\n";
  // std::cout << "Speedup = " << (1.0 * slowTime.count()) / (1.0 * fastTime.count()) << "\n";

  return 0;
}
