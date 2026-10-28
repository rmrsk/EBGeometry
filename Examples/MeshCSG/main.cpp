// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// CSG with a mesh: a triangle mesh is a shape like any other, so the expression builder moves and scales it
// and joins it to analytic shapes. Here the xyzrgb dragon stands on a table, blended into it by a smooth
// union. See README.md.

#include <iomanip>
#include <iostream>
#include <string>
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

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  // ── Read the mesh ──────────────────────────────────────────────────────────────────────────────────
  //
  // The default is the dragon from the common-3d-test-models submodule (clone with --recurse-submodules,
  // or run git submodule update --init); pass another mesh on the command line. Arguments starting with --
  // are options (--no-view keeps the viewer shut). OnDefect::Warn loads a mesh with small defects with a
  // warning instead of refusing it.
  std::string file = "../../common-3d-test-models/data/xyzrgb_dragon.obj";

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]).rfind("--", 0) != 0) {
      file = argv[i];

      break;
    }
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto mesh = EBGeometry::Parser::readIntoTriMeshSDF<T>(
    file, pool, EBGeometry::BVH::Construction::SAH, EBGeometry::Parser::OnDefect::Warn);

  // ── Place it on a table ────────────────────────────────────────────────────────────────────────────
  //
  // Scale the mesh to 2.4 along its longest side and stand it on y = 0, centred in x and z. The table is a
  // rounded top, its upper face at y = 0, on four legs.
  const Vec3 lo    = mesh.computeBoundingVolume().getLowCorner();
  const Vec3 hi    = mesh.computeBoundingVolume().getHighCorner();
  const T    scale = T(2.4) / (hi - lo)[(hi - lo).maxDir(true)];
  const Vec3 shift(T(-0.5) * (lo[0] + hi[0]), -lo[1], T(-0.5) * (lo[2] + hi[2]));

  const Expr dragon = EBGeometry::Scale(EBGeometry::Translate(mesh, shift), scale);

  const T hx = T(0.5) * scale * (hi - lo)[0] + T(0.15);
  const T hz = T(0.5) * scale * (hi - lo)[2] + T(0.15);

  std::vector<Expr> table{
    EBGeometry::RoundedBoxSDF<T>(Vec3(T(0), T(-0.06), T(0)), Vec3(2 * hx, T(0.12), 2 * hz), T(0.03))};

  for (const T x : {-hx + T(0.12), hx - T(0.12)}) {
    for (const T z : {-hz + T(0.12), hz - T(0.12)}) {
      table.push_back(EBGeometry::CylinderSDF<T>(Vec3(x, T(-0.75), z), Vec3(x, T(-0.06), z), T(0.06)));
    }
  }

  // A plain union meets the table at a sharp edge; the smooth union fills a fillet of about the blend
  // length where the feet touch it.
  const Tape sharp(pool, EBGeometry::Union(dragon, EBGeometry::Union(table)));
  const Tape smooth(pool, EBGeometry::SmoothUnion(dragon, EBGeometry::Union(table), T(0.3)));

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  std::cout << std::fixed << std::setprecision(4) << "Read '" << file << "', box after placing "
            << smooth.computeBoundingVolume() << '\n';

  for (const Vec3& p : {Vec3(T(0), T(0.05), T(0)), Vec3(T(0), T(0.3), T(0)), Vec3(hx, T(0.2), hz)}) {
    std::cout << "  " << p << "   union " << std::setw(8) << sharp.signedDistance(p) << "   smooth union "
              << std::setw(8) << smooth.signedDistance(p) << '\n';
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the smooth union (built with VIEWER=SDL2; otherwise, with --no-view or without a display,
  // view() prints why and returns). P colours the dragon and each part of the table.
  // Given the pool, view() renders on the GPU in a GPU build (G switches to the CPU and back).
  EBGeometry::Render::view(smooth, pool, argc, argv);

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n'
            << "The default mesh is in the common-3d-test-models submodule: git submodule update --init\n";

  return 1;
}
