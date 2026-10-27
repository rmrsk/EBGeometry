// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Repetition: many copies of one object on a grid, with FiniteRepetition; here a mesh, the happy Buddha.
// The tape stores the object once and folds each point into the nearest copy, so its size and cost do not
// grow with the number of copies; the program compares it with the same copies placed one by one in a
// union. See README.md.

#include <chrono>
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

namespace {

// Nanoseconds per evaluation over a list of points; the sum keeps the compiler from skipping the work.
double
nanosecondsPerPoint(const Tape& a_tape, const std::vector<Vec3>& a_points, T& a_sum)
{
  const auto start = std::chrono::steady_clock::now();

  for (const Vec3& p : a_points) {
    a_sum += a_tape.signedDistance(p);
  }

  return std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start).count() /
         double(a_points.size());
}

} // namespace

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  // ── The object and its copies ──────────────────────────────────────────────────────────────────────
  //
  // The default mesh is the happy Buddha from the common-3d-test-models submodule (clone with
  // --recurse-submodules, or run git submodule update --init); pass another on the command line. Arguments
  // starting with -- are options (--no-view keeps the viewer shut).
  std::string file = "../../common-3d-test-models/data/happy.obj";

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]).rfind("--", 0) != 0) {
      file = argv[i];

      break;
    }
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto mesh = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool);

  // Scale the mesh to height 1 and stand it on y = 0, centred in x and z.
  const Vec3 lo    = mesh.computeBoundingVolume().getLowCorner();
  const Vec3 hi    = mesh.computeBoundingVolume().getHighCorner();
  const T    scale = T(1) / (hi[1] - lo[1]);

  const Expr statue = EBGeometry::Scale(
    EBGeometry::Translate(mesh, Vec3(T(-0.5) * (lo[0] + hi[0]), -lo[1], T(-0.5) * (lo[2] + hi[2]))), scale);

  // FiniteRepetition(object, period, lo, hi) tiles it with the given period, lo and hi tiles to either
  // side of the home tile along each axis: here 7 x 7 copies in the xz plane.
  const int  n = 3;
  const Vec3 period(T(0.8), T(1), T(0.8));
  const Expr grid = EBGeometry::FiniteRepetition(statue, period, Vec3(T(n), T(0), T(n)), Vec3(T(n), T(0), T(n)));

  // The same copies one by one, for comparison.
  std::vector<Expr> copies;

  for (int i = -n; i <= n; i++) {
    for (int k = -n; k <= n; k++) {
      copies.push_back(EBGeometry::Translate(statue, Vec3(period[0] * T(i), T(0), period[2] * T(k))));
    }
  }

  // ── Compare them on the host ───────────────────────────────────────────────────────────────────────
  //
  // Both give the same values. The repetition stores the statue once; the union stores every copy and
  // finds the ones near a point through a BVH over their boxes.
  const Tape repeated(pool, grid);
  const Tape placed(pool, EBGeometry::Union(copies));

  std::vector<Vec3> points;

  for (int i = 0; i < 20000; i++) {
    points.emplace_back(T(-3) + T(0.06) * T(i % 100), T(0.5), T(-3) + T(0.03) * T(i / 100));
  }

  T sum = T(0);

  const Vec3 probe(T(0.85), T(0.5), T(-0.75));

  std::cout << std::fixed << std::setprecision(1) << copies.size() << " copies\n"
            << "  FiniteRepetition: " << repeated.getNumClauses() << " clauses, "
            << nanosecondsPerPoint(repeated, points, sum) << " ns per point\n"
            << "  Union of copies:  " << placed.getNumClauses() << " clauses, "
            << nanosecondsPerPoint(placed, points, sum) << " ns per point\n"
            << std::setprecision(4) << "  at " << probe << ": " << repeated.signedDistance(probe) << " and "
            << placed.signedDistance(probe) << " (sum " << sum << ")\n";

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the grid of statues (built with VIEWER=SDL2; otherwise, with --no-view or without a display,
  // view() prints why and returns).
  EBGeometry::Render::view(repeated, argc, argv);

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n'
            << "The default mesh is in the common-3d-test-models submodule: git submodule update --init\n";

  return 1;
}
