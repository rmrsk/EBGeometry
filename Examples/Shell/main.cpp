// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Shell extraction: a thin wall around a mesh's surface with Annular, the walls an embedded-boundary grid
// often needs from a closed surface mesh. Here Lucy, rotated upright, hollowed into a shell, with a quarter
// cut away so that the wall shows. See README.md.

#include <iomanip>
#include <iostream>
#include <string>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Expr = EBGeometry::Expr<T>;
using Tape = EBGeometry::Tape<T>;

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  // ── Read the mesh and stand it up ──────────────────────────────────────────────────────────────────
  //
  // The default is Lucy from the common-3d-test-models submodule (clone with --recurse-submodules, or run
  // git submodule update --init); pass another mesh on the command line. Arguments starting with -- are
  // options (--no-view keeps the viewer shut).
  std::string file = "../../common-3d-test-models/data/lucy.obj";

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]).rfind("--", 0) != 0) {
      file = argv[i];

      break;
    }
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto mesh = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool);

  // Lucy stands along z in the file: turn her about x so that she stands along y and about y so that she
  // faces the camera (+z), then scale her to height 2 and stand her on y = 0, centred in x and z.
  const Expr upright = EBGeometry::Rotate(EBGeometry::Rotate(mesh, T(-90), 0), T(180), 1);
  const auto box     = Tape(pool, upright).computeBoundingVolume();
  const Vec3 lo      = box.getLowCorner();
  const Vec3 hi      = box.getHighCorner();

  const Expr statue = EBGeometry::Scale(
    EBGeometry::Translate(upright, Vec3(T(-0.5) * (lo[0] + hi[0]), -lo[1], T(-0.5) * (lo[2] + hi[2]))),
    T(2) / (hi[1] - lo[1]));

  // ── Hollow it ──────────────────────────────────────────────────────────────────────────────────────
  //
  // Annular(e, d) keeps the points within d of the surface: a wall 2d thick, centred on it, outside and
  // inside alike. A Difference with a box then removes the quarter towards the camera, x > 0 and z > 0.
  const T thickness = T(0.02);

  const EBGeometry::BoxSDF<T> quarter(Vec3(T(0), T(-1), T(0)), Vec3(T(2), T(3), T(2)));

  const Tape shell(pool, EBGeometry::Difference(EBGeometry::Annular(statue, T(0.5) * thickness), quarter));
  const Tape solid(pool, statue);

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // Deep inside the statue the solid is negative and the shell positive: that is the hollow.
  std::cout << std::fixed << std::setprecision(4) << "Read '" << file << "', box after placing "
            << solid.computeBoundingVolume() << '\n';

  for (const Vec3& p : {Vec3(T(0), T(0.1), T(-0.1)), Vec3(T(0), T(0.6), T(-0.05))}) {
    std::cout << "  " << p << "   solid " << std::setw(8) << solid.signedDistance(p) << "   shell " << std::setw(8)
              << shell.signedDistance(p) << '\n';
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the shell (built with VIEWER=SDL2; otherwise, with --no-view or without a display, view()
  // prints why and returns).
  EBGeometry::Render::view(shell, argc, argv);

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n'
            << "The default mesh is in the common-3d-test-models submodule: git submodule update --init\n";

  return 1;
}
