#include <iostream>

#include "../../EBGeometry.hpp"

using namespace EBGeometry;

using T    = double;
using Vec3 = Vec3T<T>;
using BV   = BoundingVolumes::AABBT<T>;

int
main()
{
  // Create a sphere, translate it, scale it, mollify it, and rotate it.
  std::shared_ptr<ImplicitFunction<T>> func;

  func = std::make_shared<SphereSDF<T>>(Vec3::zero(), 1.0);
  func = Translate(func, Vec3::one());
  func = Scale(func, 2.0);
  func = Mollify(func, 0.25);
  func = Rotate(func, 45.0, 0);

  // Try to compute a bounding volume for the sphere.
  Vec3 initLo = -10 * Vec3::one();
  Vec3 initHi = +10 * Vec3::one();

  const auto boundingVolume = func->approximateBoundingVolumeOctree<BV>(initLo, initHi, 8, 0.0);

  std::cout << "Approximate bounding volume = " << boundingVolume << std::endl;

  return 0;
}
