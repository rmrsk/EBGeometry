// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// The classic CSG solid: a cube rounded off by a ball (an intersection) and drilled through along all three
// axes (a difference with a union of three cylinders). The expression builder describes it, a Tape compiles
// it, and the same tape is evaluated on the host and on a GPU. See README.md.

#include <cstdint>
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

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE call
// operator maps one point to one result. A Tape is a descriptor of the program it compiled into a Pool, so
// the query holds it by value.
struct Distance
{
  Tape m_tape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return m_tape.signedDistance(a_point);
  }
};

int
main(int argc, char* argv[])
{
  // ── Describe the solid ─────────────────────────────────────────────────────────────────────────────
  //
  // Any shape converts to an Expr, and Union, Intersection and Difference combine them (a pair, or a
  // vector). Nothing is evaluated yet.
  const EBGeometry::BoxSDF<T>    cube(-Vec3::ones(), Vec3::ones());
  const EBGeometry::SphereSDF<T> ball(Vec3::zeros(), T(1.35));

  const T length = T(1.5);
  const T radius = T(0.55);

  const std::vector<Expr> drills{EBGeometry::CylinderSDF<T>(-length * Vec3(1, 0, 0), length * Vec3(1, 0, 0), radius),
                                 EBGeometry::CylinderSDF<T>(-length * Vec3(0, 1, 0), length * Vec3(0, 1, 0), radius),
                                 EBGeometry::CylinderSDF<T>(-length * Vec3(0, 0, 1), length * Vec3(0, 0, 1), radius)};

  const Expr solid = EBGeometry::Difference(EBGeometry::Intersection(cube, ball), EBGeometry::Union(drills));

  // ── Compile it and use it on the host ──────────────────────────────────────────────────────────────
  //
  // The Tape compiles the expression into the pool, which must outlive the tape and every copy of it.
  // signedDistance() is negative inside and positive outside. An intersection or a difference of exact
  // distances is a lower bound on the distance, which getDistanceQuality() reports.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Tape tape(pool, solid);

  std::cout << std::fixed << std::setprecision(4) << "The solid: " << tape.getNumClauses() << " clauses, box "
            << tape.computeBoundingVolume() << ", "
            << (tape.getDistanceQuality() == EBGeometry::DistanceQuality::Exact ? "exact" : "a bound") << '\n';

  // closestPrimitive() names the leaf the value comes from, numbered in expression order: the cube 0, the
  // ball 1, the drills along x, y and z 2, 3 and 4.
  for (const Vec3& p : {Vec3::zeros(), Vec3(T(0.95), T(0.2), T(0.1)), Vec3(T(0.8), T(0.8), T(0.8)), Vec3(2, 0, 0)}) {
    std::cout << "  " << p << "   value " << std::setw(8) << tape.signedDistance(p) << "   leaf "
              << tape.closestPrimitive(p).m_leaf << '\n';
  }

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // Freeze the pool, mirror it into GPU::memoryResource() (device memory in a CUDA or HIP build), and take
  // a view of the tape onto the copy. In a plain C++ build the same lines run in a host loop.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n";
  }
  else {
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

    std::vector<Vec3> points;

    for (int i = 0; i <= 8; i++) {
      points.emplace_back(T(-1.2) + T(0.3) * T(i), T(0.3), T(0.7));
    }

    std::vector<T> values(points.size());

    EBGeometry::GPU::transform(Distance{tape.rebasedView(devicePool)}, points, values);

    std::cout << "Along a line at y = 0.3, z = 0.7, through GPU::transform (" << EBGeometry::GPU::backendName << "):\n";

    for (std::size_t i = 0; i < points.size(); i++) {
      std::cout << "  x = " << std::setw(7) << points[i][0] << "   value " << std::setw(8) << values[i] << '\n';
    }
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the solid (built with VIEWER=SDL2): drag to orbit, scroll to zoom, X cuts it open, P
  // colours each leaf. Otherwise, with --no-view or without a display, view() prints why and returns.
  EBGeometry::Render::view(tape, argc, argv);

  return 0;
}
