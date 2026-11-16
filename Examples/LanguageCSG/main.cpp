// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A composition read at run time: the classic CSG solid of the BasicCSG example, written in the .ebg language
// (solid.ebg) instead of composed in C++. The program reads it, compiles its object into a tape, and evaluates the tape.
// Change the model, or one of its params from the command line, and run again: no compiling. The tape runs on a GPU as
// any tape does (see BasicCSG).
// See README.md.

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Language.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Tape = EBGeometry::Tape<T>;

int
main(int argc, char* argv[])
{
  // ── Read the model ─────────────────────────────────────────────────────────────────────────────────
  //
  // The arguments: the model file (solid.ebg by default), and -DNAME=VALUE to override one of its params, as ebgstudio
  // takes them. read() throws every error in the model at once, with its file, line and column.
  std::string                     file = "solid.ebg";
  EBGeometry::Language::Overrides overrides;

  for (int i = 1; i < argc; i++) {
    const std::string arg    = argv[i];
    const size_t      equals = arg.find('=');

    if (arg.rfind("-D", 0) == 0 && equals != std::string::npos) {
      overrides[arg.substr(2, equals - 2)] = arg.substr(equals + 1);
    }
    else if (arg[0] != '-') {
      file = arg;
    }
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto model = [&]() {
    try {
      return EBGeometry::Language::read<T>(file, pool, overrides);
    } catch (const EBGeometry::Language::Error& e) {
      std::cerr << e.what() << "\n";

      std::exit(1);
    }
  }();

  for (const auto& warning : model.warnings()) {
    std::cout << warning.str() << "\n";
  }

  std::cout << "Read " << file << ", params:";

  for (const auto& param : model.params()) {
    std::cout << " " << param.m_name << " = " << param.m_value << (param.m_overridden ? " (overridden)" : "");
  }

  std::cout << "\n";

  // ── Compile it and use it on the host ──────────────────────────────────────────────────────────────
  //
  // The object is an Expr, as the expression builder makes one, and compiles into a tape like any other: from here on,
  // nothing knows the solid came from a file.
  const Tape tape(pool, model.object());

  std::cout << std::fixed << std::setprecision(4) << "The solid: " << tape.getNumClauses() << " clauses, box "
            << tape.computeBoundingVolume() << "\n";

  for (const Vec3& p : {Vec3::zeros(), Vec3(T(0.95), T(0.2), T(0.1)), Vec3(T(0.8), T(0.8), T(0.8)), Vec3(2, 0, 0)}) {
    std::cout << "  " << p << "   value " << std::setw(8) << tape.signedDistance(p) << "\n";
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the solid (built with VIEWER=SDL2). ebgstudio does the same, and reloads the model on every save.
  EBGeometry::Render::view(tape, pool, argc, argv);

  return 0;
}
