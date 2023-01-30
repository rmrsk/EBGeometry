#include <iostream>

#include "../../EBGeometry.hpp"

using namespace EBGeometry;

using T    = float;
using Vec3 = Vec3T<T>;
using BV   = BoundingVolumes::BoundingSphereT<T>;

int
main()
{
  // Create a sphere.
  std::shared_ptr<ImplicitFunction<T>> sphere = std::make_shared<SphereSDF<T>>(Vec3::one(), 1.0);

  // Try to compute a bounding volume for the sphere.
  Vec3T<float> initLo = -10 * Vec3::one();
  Vec3T<float> initHi = +10 * Vec3::one();

  //  sphere = Translate(sphere, Vec3::one());
  sphere = Scale(sphere, (T)5.0);

  std::cout << sphere->value(Vec3::one()) << "\n";

  const auto boundingVolume = sphere->approximateBoundingVolumeOctree<BV>(initLo, initHi, 5, 0.0);

  std::cout << boundingVolume << std::endl;
}
