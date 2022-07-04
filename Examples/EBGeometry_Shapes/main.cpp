/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include "../../EBGeometry.hpp"

int
main()
{
  using T    = double;
  using Vec3 = EBGeometry::Vec3T<T>;

  // Various analytic shapes. Use ::signedDistance(Vec3) for computing the signed distance.
  const EBGeometry::PlaneSDF<T>            plane(Vec3::zero(), Vec3::one(), false);
  const EBGeometry::SphereSDF<T>           sphere(Vec3::zero(), 1.0, false);
  const EBGeometry::BoxSDF<T>              box(Vec3::zero(), Vec3::one(), false);
  const EBGeometry::TorusSDF<T>            torus(Vec3::zero(), 1.0, 0.1, false);
  const EBGeometry::CylinderSDF<T>         finiteCylinder(Vec3::zero(), Vec3::one(), 0.1, false);
  const EBGeometry::InfiniteCylinderSDF<T> infiniteCylinder(Vec3::zero(), 0.1, 2, false);
  const EBGeometry::CapsuleSDF<T>          capsule(Vec3::zero(), Vec3::one(), 0.1, false);
  const EBGeometry::InfiniteConeSDF<T>     infiniteCone(Vec3::zero(), 45.0, false);
  const EBGeometry::ConeSDF<T>             cone(Vec3::zero(), 1.0, 45.0, false);
}
