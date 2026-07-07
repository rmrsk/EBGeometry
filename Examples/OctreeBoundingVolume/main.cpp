// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using namespace EBGeometry;

using T    = EBGEOMETRY_PRECISION;
using Vec3 = Vec3T<T>;
using BV   = BoundingVolumes::AABBT<T>;

int
main()
{
  // Build a unit sphere at the origin, then chain a few transforms onto it so its exact
  // extent is no longer obvious just from the sphere's own formula:
  std::shared_ptr<ImplicitFunction<T>> func;

  func = std::make_shared<SphereSDF<T>>(Vec3::zeros(), T(1.0)); // Unit sphere at the origin.
  func = Translate(func, Vec3::ones());                         // Shift by (1, 1, 1).
  func = Scale(func, T(2.0));                                   // Scale (radius) by 2x.
  func = Mollify(func, T(0.25));                                // Slightly blur/round the surface.
  func = Rotate(func, T(45.0), 0);                              // Rotate 45 degrees about the x-axis.

  // Numerically approximate a tight bounding box around the transformed shape: start from a
  // deliberately loose box, then recursively subdivide into octree cells (each cell splits
  // into 8 children) and discard the ones that don't touch the shape, down to a maximum of
  // 8 levels of refinement. The last argument (0.0) is a safety margin on the intersection
  // test: 0 means exact, larger values pad the result to guard against a coarse initial box
  // missing part of the shape.
  const Vec3 initLo = -10 * Vec3::ones();
  const Vec3 initHi = +10 * Vec3::ones();

  const auto boundingVolume = func->approximateBoundingVolumeOctree<BV>(initLo, initHi, 8, 0.0);

  std::cout << "Approximate bounding volume = " << boundingVolume << '\n';

  return 0;
}
