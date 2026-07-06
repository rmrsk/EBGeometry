// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

int
main()
{
  using T    = EBGEOMETRY_PRECISION;
  using Vec3 = EBGeometry::Vec3T<T>;

  // Various analytic shapes. Use ::signedDistance(Vec3) for computing the signed distance.
  const EBGeometry::PlaneSDF<T>            plane(Vec3::zero(), Vec3::one());
  const EBGeometry::SphereSDF<T>           sphere(Vec3::zero(), 1.0);
  const EBGeometry::BoxSDF<T>              box(Vec3::zero(), Vec3::one());
  const EBGeometry::TorusSDF<T>            torus(Vec3::zero(), 1.0, 0.1);
  const EBGeometry::CylinderSDF<T>         finiteCylinder(Vec3::zero(), Vec3::one(), 0.1);
  const EBGeometry::InfiniteCylinderSDF<T> infiniteCylinder(Vec3::zero(), 0.1, 2);
  const EBGeometry::CapsuleSDF<T>          capsule(Vec3::zero(), Vec3::one(), 0.1);
  const EBGeometry::InfiniteConeSDF<T>     infiniteCone(Vec3::zero(), 45.0);
  const EBGeometry::ConeSDF<T>             cone(Vec3::zero(), 1.0, 45.0);
  const EBGeometry::PerlinSDF<T>           perlin(1.0, Vec3::one(), 0.5, 10);
  const EBGeometry::RoundedBoxSDF<T>       roundBox(Vec3::one(), 0.1);
}
