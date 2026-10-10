// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cmath>
#include <iostream>
#include <string>
#include <type_traits>

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
  // for the full parameter documentation). Each shape is a plain value type, so it can also be
  // copied into a GPU kernel and evaluated there.

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

  // A cylinder along the z-axis with rounded edges: radius, edge rounding radius, height.
  const EBGeometry::RoundedCylinderSDF<T> roundCylinder(1.0, 0.1, 1.0);

  // Check each shape against its distance at a few points, worked out by hand from the description
  // above. Positive is outside, negative inside, zero on the surface.
  const T tolerance = std::is_same_v<T, float> ? T(1.0e-5) : T(1.0e-12);

  const T sqrt3        = std::sqrt(T(3));
  const T sinHalfAngle = std::sin(T(22.5) * EBGeometry::pi<T> / T(180)); // Half of the cones' 45 degrees.

  int failures = 0;

  const auto check = [&](const std::string& a_name, const T a_value, const T a_expected) {
    if (std::abs(a_value - a_expected) > tolerance * (T(1) + std::abs(a_expected))) {
      std::cerr << a_name << ": expected " << a_expected << ", got " << a_value << "\n";

      failures++;
    }
  };

  // The plane's normal is normalized, so the distance is along the unit diagonal.
  check("plane, along the normal", plane.signedDistance(Vec3::ones()), sqrt3);
  check("plane, against the normal", plane.signedDistance(-Vec3::ones()), -sqrt3);

  check("sphere, at its center", sphere.signedDistance(Vec3::zeros()), T(-1));
  check("sphere, outside", sphere.signedDistance(Vec3(2, 0, 0)), T(1));

  check("box, at its center", box.signedDistance(T(0.5) * Vec3::ones()), T(-0.5));
  check("box, outside a face", box.signedDistance(Vec3(2, 0.5, 0.5)), T(1));

  // The tube's centre-line is the unit circle in the xy-plane.
  check("torus, on its centre-line", torus.signedDistance(Vec3(1, 0, 0)), T(-0.1));
  check("torus, at its center", torus.signedDistance(Vec3::zeros()), T(0.9));

  check("cylinder, on its axis", finiteCylinder.signedDistance(T(0.5) * Vec3::ones()), T(-0.1));
  check("cylinder, beyond a cap", finiteCylinder.signedDistance(T(2) * Vec3::ones()), sqrt3);

  check("infinite cylinder, on its axis", infiniteCylinder.signedDistance(Vec3(0, 0, -3)), T(-0.1));
  check("infinite cylinder, outside", infiniteCylinder.signedDistance(Vec3(1, 0, 7)), T(0.9));

  // The capsule's tips are on its surface.
  check("capsule, at a tip", capsule.signedDistance(Vec3::zeros()), T(0));
  check("capsule, on its axis", capsule.signedDistance(T(0.5) * Vec3::ones()), T(-0.1));
  check("capsule, beyond a tip", capsule.signedDistance(T(2) * Vec3::ones()), sqrt3);

  // The cones open along -z from their tips at the origin.
  check("infinite cone, at the tip", infiniteCone.signedDistance(Vec3::zeros()), T(0));
  check("infinite cone, above the tip", infiniteCone.signedDistance(Vec3(0, 0, 1)), T(1));
  check("infinite cone, on its axis", infiniteCone.signedDistance(Vec3(0, 0, -1)), -sinHalfAngle);

  check("cone, above the tip", cone.signedDistance(Vec3(0, 0, 1)), T(1));
  check("cone, on its axis", cone.signedDistance(Vec3(0, 0, -0.5)), T(-0.5) * sinHalfAngle);
  check("cone, below its base", cone.signedDistance(Vec3(0, 0, -3)), T(2));

  // Noise is not a distance, but it is defined everywhere.
  check("Perlin noise, finite", std::isfinite(perlin.signedDistance(Vec3(0.3, 0.6, 0.9))) ? T(1) : T(0), T(1));

  // The rounded box is centred at the origin, its half-sides 0.5 grown by the rounding 0.1.
  check("rounded box, at its center", roundBox.signedDistance(Vec3::zeros()), T(-0.6));
  check("rounded box, outside a face", roundBox.signedDistance(Vec3(1.6, 0, 0)), T(1));

  // The rounded cylinder is centred at the origin with its axis along y, and half as high as wide.
  check("rounded cylinder, at its center", roundCylinder.signedDistance(Vec3::zeros()), T(-0.5));
  check("rounded cylinder, above a cap", roundCylinder.signedDistance(Vec3(0, 1.5, 0)), T(1));

  std::cout << "Shapes checked against hand-worked distances; " << failures << " failed\n";

  return failures == 0 ? 0 : 1;
}
