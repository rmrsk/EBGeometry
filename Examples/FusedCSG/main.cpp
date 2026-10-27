// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Fused CSG: the classic solid of the BasicCSG example built with the functions in namespace Fused, whose
// result is a shape whose type spells out the whole tree. The C++ compiler evaluates it as it would the
// solid written out by hand, with nothing to interpret: the program compares it with the same solid
// through a Tape, uses fused parts as the leaves of a tape, and evaluates the fused solid on a GPU with no
// pool at all. See README.md.

#include <chrono>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Expr = EBGeometry::Expr<T>;

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE call
// operator maps one point to one result. A fused solid of analytic shapes holds the shapes themselves, so
// the query holds it by value and it needs no pool.
template <class Shape>
struct Distance
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }
};

// Nanoseconds per evaluation over a list of points, on the host; the sum keeps the compiler from skipping
// the work.
template <class Shape>
double
nanosecondsPerPoint(const Shape& a_shape, const std::vector<Vec3>& a_points, T& a_sum)
{
  const auto start = std::chrono::steady_clock::now();

  for (const Vec3& p : a_points) {
    a_sum += a_shape.signedDistance(p);
  }

  return std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start).count() /
         double(a_points.size());
}

int
main(int argc, char* argv[])
{
  // ── Build the solid, fused ─────────────────────────────────────────────────────────────────────────
  //
  // The functions are the builder's, called qualified (Fused::Union, not Union, which is the builder's);
  // where the builder takes a vector, they take the operands as arguments.
  const EBGeometry::BoxSDF<T>      cube(-Vec3::ones(), Vec3::ones());
  const EBGeometry::SphereSDF<T>   ball(Vec3::zeros(), T(1.35));
  const EBGeometry::CylinderSDF<T> drillX(T(-1.5) * Vec3(1, 0, 0), T(1.5) * Vec3(1, 0, 0), T(0.55));
  const EBGeometry::CylinderSDF<T> drillY(T(-1.5) * Vec3(0, 1, 0), T(1.5) * Vec3(0, 1, 0), T(0.55));
  const EBGeometry::CylinderSDF<T> drillZ(T(-1.5) * Vec3(0, 0, 1), T(1.5) * Vec3(0, 0, 1), T(0.55));

  const auto fused = EBGeometry::Fused::Difference(EBGeometry::Fused::Intersection(cube, ball), drillX, drillY, drillZ);

  using Fused = std::decay_t<decltype(fused)>;

  // ── Compare it with the tape on the host ───────────────────────────────────────────────────────────
  //
  // The same solid through the builder and a Tape gives the same values (to rounding); the tape fetches
  // and dispatches each clause, which the fused solid does not.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const EBGeometry::Tape<T> tape(
    pool, EBGeometry::Difference(EBGeometry::Intersection(cube, ball), std::vector<Expr>{drillX, drillY, drillZ}));

  std::vector<Vec3> grid;

  for (int i = 0; i < 64000; i++) {
    grid.emplace_back(
      T(-1.5) + T(0.075) * T(i % 40), T(-1.5) + T(0.075) * T(i / 40 % 40), T(-1.5) + T(0.075) * T(i / 1600));
  }

  T sum = T(0);

  std::cout << std::fixed << std::setprecision(4) << "At (0.9, 0.9, 0.9): tape "
            << tape.signedDistance(Vec3(T(0.9), T(0.9), T(0.9))) << ", fused "
            << fused.signedDistance(Vec3(T(0.9), T(0.9), T(0.9))) << '\n';

  const double tapeTime  = nanosecondsPerPoint(tape, grid, sum);
  const double fusedTime = nanosecondsPerPoint(fused, grid, sum);

  std::cout << std::setprecision(1) << "Per point on the host: tape " << tapeTime << " ns, fused " << fusedTime
            << " ns (sum " << sum << ")\n";

  // ── Fused parts in a tape ──────────────────────────────────────────────────────────────────────────
  //
  // A fused solid is one shape with one surface: listed after T, it is one leaf of a tape, so a scene
  // known only at run time can place compiled parts. closestPrimitive() then names the part.
  const EBGeometry::Tape<T, Fused> scene(
    pool,
    EBGeometry::Union(Expr(fused), EBGeometry::Translate(EBGeometry::Rotate(fused, T(45), 1), Vec3(T(3), T(0), T(0)))));

  std::cout << "In a scene of two parts, the point (3.9, 0, 0) is decided by part "
            << scene.closestPrimitive(Vec3(T(3.9), T(0), T(0))).m_leaf << '\n';

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // The fused solid goes to the device as it is: nothing to mirror. In a plain C++ build GPU::transform()
  // is a host loop.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n";
  }
  else {
    std::vector<Vec3> line;

    for (int i = 0; i <= 8; i++) {
      line.emplace_back(T(-1.2) + T(0.3) * T(i), T(0.3), T(0.7));
    }

    std::vector<T> values(line.size());

    EBGeometry::GPU::transform(Distance<Fused>{fused}, line, values);

    std::cout << std::setprecision(4) << "Along a line at y = 0.3, z = 0.7, through GPU::transform ("
              << EBGeometry::GPU::backendName << "):\n";

    for (std::size_t i = 0; i < line.size(); i++) {
      std::cout << "  x = " << std::setw(7) << line[i][0] << "   value " << std::setw(8) << values[i] << '\n';
    }
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // The scene of two fused parts (built with VIEWER=SDL2; otherwise, with --no-view or without a display,
  // view() prints why and returns). P colours each part.
  EBGeometry::Render::view(scene, argc, argv);

  return 0;
}
