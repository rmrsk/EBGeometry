// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// A composition read at run time: the classic CSG solid of the BasicCSG example, written in the .ebg language
// (solid.ebg) instead of composed in C++. The program reads it, compiles its object into a tape, and opens a window on
// it. Edit the model and run again: no compiling. See README.md.

#include <string>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Language.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T = EBGEOMETRY_PRECISION;

int
main(int argc, char* argv[])
{
  // The model file: solid.ebg, or the first argument. A model with errors throws them all, with their files, lines and
  // columns.
  const std::string file = (argc > 1 && argv[1][0] != '-') ? argv[1] : "solid.ebg";

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto model = EBGeometry::Language::read<T>(file, pool);

  // The object is an Expr, as the expression builder makes one, and compiles into a tape like any other: from here on,
  // nothing knows the solid came from a file.
  const EBGeometry::Tape<T> tape(pool, model.object(), model.tapeOptions());

  // A window on the solid (built with VIEWER=SDL2). Given the pool, it renders on the GPU in a GPU build.
  EBGeometry::Render::view(tape, pool, argc, argv);

  return 0;
}
