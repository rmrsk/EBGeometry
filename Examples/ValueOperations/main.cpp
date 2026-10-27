// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Value operations: Offset, Annular, Blur and Mollify change an object's value rather than where it is.
// Offset grows or shrinks it, Annular hollows it into a shell, and Blur and Mollify average the value
// around each point, which rounds edges and corners. The program applies each to a cube and prints its
// value at the cube's corner and how far the value can be trusted as a distance. See README.md.

#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
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
  // ── Apply them ─────────────────────────────────────────────────────────────────────────────────────
  //
  // Offset(r) moves the surface outward by r (inward if negative). Annular(d) keeps a shell of
  // half-thickness d around the surface; a box removes the shell's top so that it can be seen. Blur(d)
  // averages the value over 27 points spaced d apart; Mollify(r, n) over an n x n x n grid of half-width r
  // with a smooth weight, which costs n^3 evaluations per point.
  const EBGeometry::BoxSDF<T> cube(T(-0.5) * Vec3::ones(), T(0.5) * Vec3::ones());
  const EBGeometry::BoxSDF<T> lid(Vec3(T(-1), T(0.2), T(-1)), Vec3(T(1), T(1), T(1)));

  const std::vector<std::pair<std::string, Expr>> operated{
    {"cube", cube},
    {"Offset 0.2", EBGeometry::Offset(cube, T(0.2))},
    {"Offset -0.2", EBGeometry::Offset(cube, T(-0.2))},
    {"Annular 0.05, lid off", EBGeometry::Difference(EBGeometry::Annular(cube, T(0.05)), lid)},
    {"Blur 0.2", EBGeometry::Blur(cube, T(0.2))},
    {"Mollify 0.2, 5 samples", EBGeometry::Mollify(cube, T(0.2), 5)}};

  // ── Use them on the host ───────────────────────────────────────────────────────────────────────────
  //
  // At the cube's corner the value is 0; an offset shifts it by the offset; blurring and mollifying round
  // the corner off, so the corner is outside them.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Vec3 corner = T(0.5) * Vec3::ones();

  std::cout << std::fixed << std::setprecision(4) << "Value at the cube's corner " << corner << ":\n";

  for (const auto& [name, expr] : operated) {
    const Tape tape(pool, expr);

    std::cout << "  " << std::setw(24) << std::left << name << std::right << std::setw(8) << tape.signedDistance(corner)
              << "   " << (tape.getDistanceQuality() == EBGeometry::DistanceQuality::Exact ? "exact" : "a bound")
              << '\n';
  }

  // ── Look at them ───────────────────────────────────────────────────────────────────────────────────
  //
  // Two rows of three (built with VIEWER=SDL2; otherwise, with --no-view or without a display, view()
  // prints why and returns).
  std::vector<Expr> rows;

  for (std::size_t i = 0; i < operated.size(); i++) {
    rows.push_back(EBGeometry::Translate(operated[i].second, Vec3(T(2) * (T(i % 3) - T(1)), T(0), T(2) * T(i / 3))));
  }

  EBGeometry::Render::view(Tape(pool, EBGeometry::Union(rows)), argc, argv);

  return 0;
}
