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

/*!
  @brief Precise sleep function. Because std::this_thread::sleep_for well and truly stinks. 
*/
void preciseSleep(double seconds) {
    using namespace std;
    using namespace std::chrono;

    static double estimate = 5e-3;
    static double mean = 5e-3;
    static double m2 = 0;
    static int64_t count = 1;

    while (seconds > estimate) {
        auto start = high_resolution_clock::now();
        this_thread::sleep_for(milliseconds(1));
        auto end = high_resolution_clock::now();

        double observed = (end - start).count() / 1e9;
        seconds -= observed;

        ++count;
        double delta = observed - mean;
        mean += delta / count;
        m2   += delta * (observed - mean);
        double stddev = sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    auto start = high_resolution_clock::now();
    while ((high_resolution_clock::now() - start).count() / 1e9 < seconds);
}

/*!
  @brief Signed distance function class which, on purpose, takes a long time to call. This is done in order to shift
  the computation time to the SDF itself rather than the BVH traversal. 
*/
class slowSphere : public EBGeometry::SphereSDF<T> {
public:

  slowSphere() : EBGeometry::SphereSDF<T>() {}
  slowSphere(const Vec3& a_center, const T& a_radius, const bool a_flipInside) : EBGeometry::SphereSDF<T> (a_center, a_radius, a_flipInside) {};
  slowSphere(const slowSphere& a_other) {
    this->m_center       = a_other.m_center;
    this->m_radius       = a_other.m_radius;
    this->m_flipInside   = a_other.m_flipInside;            
    this->m_transformOps = a_other.m_transformOps;    
  }
  virtual ~slowSphere(){}

  T signedDistance(const Vec3& a_point) const noexcept override final {
    preciseSleep(100E-9); // Wait one hundred nanoseconds. 

    return EBGeometry::SphereSDF<T>::signedDistance(a_point);
  }

  T unsignedDistance2(const Vec3& a_point) const noexcept override final {
    preciseSleep(100E-9); // Wait one hundred nanoseconds.     

    return EBGeometry::SphereSDF<T>::unsignedDistance2(a_point);    
  } 
};

int main() {

  // Degree of bounding volume hierarchies. 
  constexpr int K = 2;  

  // Make a sphere array consisting of about 10^6 spheres. The distance between the spheres is 2*radius
  std::vector<std::shared_ptr<SDF> > spheres;

  constexpr T radius = 0.1;
  constexpr int Nx = 100;
  constexpr int Ny = 100;
  constexpr int Nz = 100;

  for (int i = 0; i < Nx; i++){
    for (int j = 0; j < Ny; j++){
      for (int k = 0; k < Nz; k++){

	const T x = i * (2 * radius);      
	const T y = j * (2 * radius);      
	const T z = k * (2 * radius);

	const Vec3 center(x,y,z);
	
	spheres.emplace_back(std::make_shared<slowSphere>( center, radius, false));
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
  
  return 0;
}
