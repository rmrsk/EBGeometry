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
  // Create a sphere, translate it, scale it, mollify it, and rotate it.
  std::shared_ptr<ImplicitFunction<T>> func;

  func = std::make_shared<SphereSDF<T>>(Vec3::zeros(), T(1.0));
  func = Translate(func, Vec3::ones());
  func = Scale(func, T(2.0));
  func = Mollify(func, T(0.25));
  func = Rotate(func, T(45.0), 0);

  // Try to compute a bounding volume for the sphere.
  const Vec3 initLo = -10 * Vec3::ones();
  const Vec3 initHi = +10 * Vec3::ones();

  const auto boundingVolume = func->approximateBoundingVolumeOctree<BV>(initLo, initHi, 8, 0.0);

  std::cout << "Approximate bounding volume = " << boundingVolume << '\n';

  return 0;
}
