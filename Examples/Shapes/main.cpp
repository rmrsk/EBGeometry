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

  // Various analytic shapes. Call ::signedDistance(Vec3) on any of them to evaluate the
  // signed distance at a point (see the class declarations in EBGeometry_AnalyticDistanceFunctions.hpp
  // for the full parameter documentation).

  // A plane through the origin, oriented by its normal vector.
  const EBGeometry::PlaneSDF<T> plane(Vec3::zeros(), Vec3::ones());

  // A sphere: center, radius.
  const EBGeometry::SphereSDF<T> sphere(Vec3::zeros(), 1.0);

  // An axis-aligned box: low corner, high corner.
  const EBGeometry::BoxSDF<T> box(Vec3::zeros(), Vec3::ones());

  // A torus lying in the xy-plane: center, major radius (center of the tube to the torus'
  // own center), minor radius (radius of the tube itself).
  const EBGeometry::TorusSDF<T> torus(Vec3::zeros(), 1.0, 0.1);

  // A finite (capped) cylinder: center of one end cap, center of the other end cap, radius.
  const EBGeometry::CylinderSDF<T> finiteCylinder(Vec3::zeros(), Vec3::ones(), 0.1);

  // An infinite cylinder: a point on its axis, radius, and which Cartesian axis (0=x, 1=y,
  // 2=z) it runs along -- here, the z-axis.
  const EBGeometry::InfiniteCylinderSDF<T> infiniteCylinder(Vec3::zeros(), 0.1, 2);

  // A capsule (a cylinder with hemispherical end caps): outer tip of one hemispherical cap,
  // outer tip of the other, radius.
  const EBGeometry::CapsuleSDF<T> capsule(Vec3::zeros(), Vec3::ones(), 0.1);

  // An infinite (single-nap) cone: apex position, full opening angle in degrees.
  const EBGeometry::InfiniteConeSDF<T> infiniteCone(Vec3::zeros(), 45.0);

  // A finite cone: apex position, height (apex to base), full opening angle in degrees.
  const EBGeometry::ConeSDF<T> cone(Vec3::zeros(), 1.0, 45.0);

  // A fractal Perlin noise field (not a distance function to a specific shape -- see this
  // example's README): amplitude, per-axis spatial frequency, per-octave amplitude decay
  // ("persistence"), and number of octaves summed together.
  const EBGeometry::PerlinSDF<T> perlin(1.0, Vec3::ones(), 0.5, 10);

  // A box with rounded edges and corners: full side lengths along each axis (before
  // rounding), corner/edge rounding radius.
  const EBGeometry::RoundedBoxSDF<T> roundBox(Vec3::ones(), 0.1);
}
