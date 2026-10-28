// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Transforms: one object moved, turned, scaled, mirrored and stretched with Translate, Rotate, Scale, Reflect
// and Elongate (copies of it over a grid are the Repetition example). A transform applies to a whole
// expression and changes where the object is, not what it is: the program prints each result's bounding box.
// See README.md.

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
  // ── The object ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A ring around the y axis with a post on its +x side, so that every transform shows.
  const Expr object =
    EBGeometry::Union(EBGeometry::TorusSDF<T>(Vec3::zeros(), T(0.5), T(0.15)),
                      EBGeometry::CapsuleSDF<T>(Vec3(T(0.5), T(0), T(0)), Vec3(T(0.5), T(0.6), T(0)), T(0.1)));

  // ── Transform it ───────────────────────────────────────────────────────────────────────────────────
  //
  // Rotate turns about a coordinate axis (0, 1, 2) or any direction, by degrees; Scale is uniform, about
  // the origin, and keeps a distance a distance; Reflect mirrors in a coordinate plane; Elongate inserts a
  // box of the given half-lengths at the origin.
  const std::vector<std::pair<std::string, Expr>> transformed{
    {"as built", object},
    {"Rotate 90 degrees about z", EBGeometry::Rotate(object, T(90), 2)},
    {"Scale by 0.6", EBGeometry::Scale(object, T(0.6))},
    {"Reflect in x", EBGeometry::Reflect(object, 0)},
    {"Elongate 0.4 along x", EBGeometry::Elongate(object, Vec3(T(0.4), T(0), T(0)))}};

  // ── Use them on the host ───────────────────────────────────────────────────────────────────────────
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  std::cout << std::fixed << std::setprecision(2) << "Bounding boxes:\n";

  for (const auto& [name, expr] : transformed) {
    std::cout << "  " << std::setw(26) << std::left << name << Tape(pool, expr).computeBoundingVolume() << '\n';
  }

  // ── Look at them ───────────────────────────────────────────────────────────────────────────────────
  //
  // Translate sets them out in a row (built with VIEWER=SDL2; otherwise, with --no-view or without a
  // display, view() prints why and returns). A union of this many operands is evaluated through a BVH
  // over their boxes, so a point evaluates only the objects near it.
  // Given the pool, view() renders on the GPU in a GPU build (G switches to the CPU and back).
  std::vector<Expr> row;

  for (std::size_t i = 0; i < transformed.size(); i++) {
    row.push_back(EBGeometry::Translate(transformed[i].second, Vec3(T(2) * T(i) - T(4), T(0), T(0))));
  }

  EBGeometry::Render::view(Tape(pool, EBGeometry::Union(row)), pool, argc, argv);

  return 0;
}
