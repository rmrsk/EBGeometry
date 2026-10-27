// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// An operation of your own: a twist about the y axis, written as a shape that holds another shape and
// evaluates it at a point turned back by an angle that grows with height. Here it twists a mesh read from a
// file. A twist does not keep distances, so the value is scaled down to stay a safe lower bound on the
// distance, and the type says so. See README.md.

#include <cmath>
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
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;
using Expr = EBGeometry::Expr<T>;
using Tape = EBGeometry::Tape<T>;

// ── An operation of your own ─────────────────────────────────────────────────────────────────────────
//
// Twisted<S> turns the shape S by m_rate radians per unit of height, about the y axis. A point p is
// turned back by m_rate * p[1] and S is evaluated there. At distance r from the axis the map shears by
// a = m_rate * r, which stretches lengths by at most (a + sqrt(a^2 + 4)) / 2. Sphere tracing and BVH
// pruning evaluate a shape only inside its box, a square of half-width m_radius about the axis whose
// corners are sqrt(2) m_radius from it, so dividing by that factor at the corners (m_lipschitz) keeps the
// value a lower bound on the distance there. Like a shape of your own (the CustomShape example), it is trivially copyable, with
// signedDistance(), computeBoundingVolume() and distanceQuality.
template <class S>
struct Twisted
{
  S m_shape;
  T m_rate;
  T m_radius;    // The largest distance of the shape from the y axis.
  T m_lipschitz; // (a + sqrt(a^2 + 4)) / 2 with a = sqrt(2) m_rate m_radius: the stretch at the corners.

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3& a_point) const noexcept
  {
    const T c = std::cos(m_rate * a_point[1]);
    const T s = std::sin(m_rate * a_point[1]);

    return m_shape.signedDistance(Vec3(c * a_point[0] + s * a_point[2], a_point[1], c * a_point[2] - s * a_point[0])) /
           m_lipschitz;
  }

  // Turning about y moves a point anywhere on its circle about the axis, but never up or down.
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  AABB
  computeBoundingVolume() const noexcept
  {
    const AABB box = m_shape.computeBoundingVolume();

    return AABB(Vec3(-m_radius, box.getLowCorner()[1], -m_radius), Vec3(m_radius, box.getHighCorner()[1], m_radius));
  }

  static constexpr EBGeometry::DistanceQuality distanceQuality = EBGeometry::DistanceQuality::Bound;
};

// Twist a shape by a_rate radians per unit of height.
template <class S>
Twisted<S>
twist(const S& a_shape, const T a_rate)
{
  const AABB box = a_shape.computeBoundingVolume();
  const Vec3 lo  = box.getLowCorner();
  const Vec3 hi  = box.getHighCorner();
  const T    x   = EBGeometry::Math::max(std::abs(lo[0]), std::abs(hi[0]));
  const T    z   = EBGeometry::Math::max(std::abs(lo[2]), std::abs(hi[2]));
  const T    r   = std::sqrt(x * x + z * z);

  const T a = std::sqrt(T(2)) * a_rate * r;

  return Twisted<S>{a_shape, a_rate, r, T(0.5) * (a + std::sqrt(a * a + T(4)))};
}

static_assert(EBGeometry::isShape<Twisted<Tape>>, "Twisted must satisfy EBGeometry's shape requirements");

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  // ── Read the mesh and twist it ─────────────────────────────────────────────────────────────────────
  //
  // The default is the armadillo from the common-3d-test-models submodule (clone with
  // --recurse-submodules, or run git submodule update --init); pass another mesh on the command line.
  // Arguments starting with -- are options (--no-view keeps the viewer shut).
  std::string file = "../../common-3d-test-models/data/armadillo.obj";

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]).rfind("--", 0) != 0) {
      file = argv[i];

      break;
    }
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto mesh = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool);

  // Scale the mesh to height 2 and stand it on y = 0 around the y axis, the axis of the twist. Any shape
  // can be twisted; a tape of the placed mesh is one.
  const Vec3 lo = mesh.computeBoundingVolume().getLowCorner();
  const Vec3 hi = mesh.computeBoundingVolume().getHighCorner();

  const Expr placed =
    EBGeometry::Scale(EBGeometry::Translate(mesh, Vec3(T(-0.5) * (lo[0] + hi[0]), -lo[1], T(-0.5) * (lo[2] + hi[2]))),
                      T(2) / (hi[1] - lo[1]));

  const Tape statue(pool, placed);

  const auto twisted = twist(statue, T(1.2)); // About 140 degrees over its height.

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // At y = 0 the twist turns nothing, so there the twisted value is the statue's divided by the factor.
  std::cout << std::fixed << std::setprecision(4) << "Twisted '" << file << "': box " << twisted.computeBoundingVolume()
            << ", values divided by " << twisted.m_lipschitz << '\n';

  for (const Vec3& p : {Vec3(T(0.3), T(0), T(0)), Vec3(T(0.3), T(1), T(0)), Vec3(T(0.3), T(1.8), T(0))}) {
    std::cout << "  " << p << "   statue " << std::setw(8) << statue.signedDistance(p) << "   twisted " << std::setw(8)
              << twisted.signedDistance(p) << '\n';
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // The statue and its twisted copy side by side, in a tape: a type of your own is listed after T. A
  // window opens when built with VIEWER=SDL2; otherwise, with --no-view or without a display, view()
  // prints why and returns. The renderer sphere traces the twisted copy, which the bound makes safe.
  const EBGeometry::Tape<T, Twisted<Tape>> scene(
    pool, EBGeometry::Union(placed, EBGeometry::Translate(Expr(twisted), Vec3(T(1.6), T(0), T(0)))));

  EBGeometry::Render::view(scene, argc, argv);

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n'
            << "The default mesh is in the common-3d-test-models submodule: git submodule update --init\n";

  return 1;
}
