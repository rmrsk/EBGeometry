// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <iostream>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using namespace EBGeometry;

using T    = EBGEOMETRY_PRECISION;
using Vec3 = Vec3T<T>;

int
main()
{
  // Numerically approximate a tight bounding box around a shape: start from a deliberately loose
  // box, then recursively subdivide into octree cells (each cell splits into 8 children) and
  // discard the ones that don't touch the shape, down to a maximum of 8 levels of refinement. The
  // last argument (0.0) is a safety margin on the intersection test: 0 means exact, larger values
  // pad the result to guard against a coarse initial box missing part of the shape.
  const Vec3         initLo   = -10 * Vec3::ones();
  const Vec3         initHi   = +10 * Vec3::ones();
  const unsigned int maxDepth = 8;

  // Any shape can be passed directly: here a finite cone with its tip at the origin, height 2 and a
  // 60 degree opening angle.
  const ConeSDF<T> cone(Vec3::zeros(), T(2.0), T(60.0));

  const auto coneBV = approximateBoundingVolumeOctree(cone, initLo, initHi, maxDepth, T(0.0));

  std::cout << "Approximate bounding volume of the cone = " << coneBV << '\n';

  // So can anything else that returns a distance for a point, such as a lambda. This one takes the
  // union (the pointwise minimum) of a sphere, a torus around it and a capsule poking out of it,
  // whose combined extent is no longer obvious from any one formula.
  const SphereSDF<T>  sphere(Vec3::zeros(), T(1.0));
  const TorusSDF<T>   torus(Vec3::zeros(), T(2.0), T(0.25));
  const CapsuleSDF<T> capsule(Vec3::zeros(), Vec3(T(1.0), T(2.0), T(3.0)), T(0.25));

  const auto shapeUnion = [&](const Vec3& a_point) -> T {
    return std::min({sphere.signedDistance(a_point), torus.signedDistance(a_point), capsule.signedDistance(a_point)});
  };

  const auto unionBV = approximateBoundingVolumeOctree(shapeUnion, initLo, initHi, maxDepth, T(0.0));

  std::cout << "Approximate bounding volume of the union = " << unionBV << '\n';

  return 0;
}
