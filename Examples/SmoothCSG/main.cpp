// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Smooth CSG: SmoothUnion, SmoothIntersection and SmoothDifference blend two objects over a length instead of
// meeting at a sharp crease. The program compares a sharp and a smooth union at their crease, with both blend
// kinds, and sets each smooth object beside its sharp counterpart in the viewer. See README.md.

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Expr = EBGeometry::Expr<T>;
using Tape = EBGeometry::Tape<T>;

int
main(int argc, char* argv[])
{
  // ── Describe the objects ───────────────────────────────────────────────────────────────────────────
  //
  // Two overlapping spheres for the union; a box and a ball for the intersection and the difference.
  const EBGeometry::SphereSDF<T> left(Vec3(T(-0.4), T(0), T(0)), T(0.55));
  const EBGeometry::SphereSDF<T> right(Vec3(T(0.4), T(0), T(0)), T(0.55));
  const EBGeometry::BoxSDF<T>    box(T(-0.7) * Vec3::ones(), T(0.7) * Vec3::ones());
  const EBGeometry::SphereSDF<T> ball(Vec3(T(0.5), T(0.5), T(0.5)), T(0.75));

  // The smooth variants take the blend length last. Polynomial (the default) blends over a band of that
  // width; Exponential blends everywhere, most near the crease, and rounds the result a little more.
  const T blend = T(0.4);

  const std::vector<Expr> sharp{
    EBGeometry::Union(left, right), EBGeometry::Intersection(box, ball), EBGeometry::Difference(box, ball)};

  const std::vector<Expr> smooth{EBGeometry::SmoothUnion(left, right, blend),
                                 EBGeometry::SmoothIntersection(box, ball, blend),
                                 EBGeometry::SmoothDifference(box, ball, blend)};

  const Expr exponential = EBGeometry::SmoothUnion(left, right, blend, EBGeometry::BlendKind::Exponential);

  // ── Compare them on the host ───────────────────────────────────────────────────────────────────────
  //
  // Where the two spheres' surfaces meet, the sharp union's value is zero; the smooth union fills the
  // crease with material, so its value there is negative, and more so with the exponential blend.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Vec3 crease(T(0), std::sqrt(T(0.55 * 0.55 - 0.4 * 0.4)), T(0));

  std::cout << std::fixed << std::setprecision(4) << "The union's value at the crease " << crease << ":\n"
            << "  sharp       " << std::setw(8) << Tape(pool, sharp[0]).signedDistance(crease) << '\n'
            << "  polynomial  " << std::setw(8) << Tape(pool, smooth[0]).signedDistance(crease) << '\n'
            << "  exponential " << std::setw(8) << Tape(pool, exponential).signedDistance(crease) << '\n';

  // ── Look at them ───────────────────────────────────────────────────────────────────────────────────
  //
  // The three sharp objects in a row at the back, the smooth ones in front (built with VIEWER=SDL2;
  // otherwise, with --no-view or without a display, view() prints why and returns).
  // Given the pool, view() renders on the GPU in a GPU build (G switches to the CPU and back).
  std::vector<Expr> row;

  for (int i = 0; i < 3; i++) {
    row.push_back(EBGeometry::Translate(sharp[i], Vec3(T(2) * T(i - 1), T(0), T(-1))));
    row.push_back(EBGeometry::Translate(smooth[i], Vec3(T(2) * T(i - 1), T(0), T(1))));
  }

  EBGeometry::Render::view(Tape(pool, EBGeometry::Union(row)), pool, argc, argv);

  return 0;
}
