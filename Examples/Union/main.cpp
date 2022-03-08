/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <string>
#include <chrono>
#include <thread>
#include <math.h>

#include "../../EBGeometry.hpp"

// Declare our precision.
using T = double;

// Aliases for cutting down on typing.
using AABB      = EBGeometry::BoundingVolumes::AABBT<T>;  
using Vec3      = EBGeometry::Vec3T<T>;  
using SDF       = EBGeometry::SignedDistanceFunction<T>;
using Sphere    = EBGeometry::SphereSDF<T>;

using namespace std::chrono_literals;

int main() {

  // Tree branching factor
  constexpr int K = 4;  

  // Make a sphere array consisting of about 10^6 spheres. The distance between the spheres is 2*radius
  std::vector<std::shared_ptr<SDF> > spheres;

  constexpr T radius = 1.0;
  constexpr int N  = 100;

  for (int i = 0; i < N; i++){
    for (int j = 0; j < N; j++){
      for (int k = 0; k < N; k++){

	const T x = i * (3 * radius);
	const T y = j * (3 * radius);
	const T z = k * (3 * radius);

	const Vec3 center(x,y,z);
	
	spheres.emplace_back(std::make_shared<Sphere>(center, radius, false));
      }
    }
  }

  // Make a standard union of these spheres. This is the union object which iterates through each and every
  // object in the scene. 
  EBGeometry::Union<T> slowUnion(spheres, false);

  // Make a fast union. To do this we must have the SDF objects (our vector of spheres) as well as a way for enclosing
  // these objects. We need to define ourselves a lambda that creates an appropriate bounding volumes for each SDF. 
  EBGeometry::BVH::BVConstructorT<SDF, AABB> aabbConstructor = [](const std::shared_ptr<const SDF>& a_prim){
    const Sphere& sph = static_cast<const Sphere&> (*a_prim);

    const Vec3& c = sph.getCenter();
    const T&    r = sph.getRadius();
    
    const Vec3 lo = c - r*Vec3::one();
    const Vec3 hi = c + r*Vec3::one();

    return AABB(lo, hi);
  };

  std::cout << "Partitioning spheres" << std::endl;
  EBGeometry::UnionBVH<T, AABB, K> fastUnion(spheres, false, aabbConstructor);

  // Time the computation distance using both the slow and fast unions.
  const Vec3 point = Vec3::zero();

  std::cout << "Computing distance with slow union\n";
  const auto t1 = std::chrono::high_resolution_clock::now();
  const T slowDist = slowUnion.signedDistance(point);
  const auto t2 = std::chrono::high_resolution_clock::now();
  
  std::cout << "Computing distance with fast union\n";  
  const auto t3 = std::chrono::high_resolution_clock::now();      
  const T fastDist = fastUnion.signedDistance(point);
  const auto t4 = std::chrono::high_resolution_clock::now();

  const std::chrono::duration<T, std::milli> slowTime = t2-t1;
  const std::chrono::duration<T, std::milli> fastTime = t4-t3;    

  std::cout << "Distance and time using standard union = " << slowDist << ", which took " << slowTime.count() << " ms\n";
  std::cout << "Distance and time using optimize union = " << fastDist << ", which took " << fastTime.count() << " ms\n";
  std::cout << "Speedup = " << (1.0 * slowTime.count())/(1.0*fastTime.count()) << "\n";
  
  return 0;
}
