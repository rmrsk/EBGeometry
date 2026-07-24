// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Point-in-face containment tests for DCEL::FaceT. The winding-number / crossing-number /
// subtended-angle inside/outside tests used to live in a standalone Polygon2D class; they are now
// FaceT helpers that project the face's vertices on the fly. This suite exercises them directly on
// small hand-built faces -- including concave polygons and non-axis-aligned planes that a mesh-level
// test wouldn't necessarily hit -- via a helper that wires up a single DCEL face from a vertex list.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <catch2/catch_template_test_macros.hpp>

#include <array>
#include <memory>
#include <vector>

using namespace EBGeometry;

namespace {

template <class T>
std::array<DCEL::InsideOutsideAlgorithm, 3>
allAlgorithms()
{
  return {DCEL::InsideOutsideAlgorithm::WindingNumber,
          DCEL::InsideOutsideAlgorithm::CrossingNumber,
          DCEL::InsideOutsideAlgorithm::SubtendedAngle};
}

// Test-only subclass that exposes FaceT's protected point-in-face test (used internally by
// signedDistance()) so the containment algorithms can be exercised directly. MeshT stores plain
// FaceT<T,Meta> by value (no polymorphism), so this can't be the type actually stored in a mesh's
// face array; instead, castIsPointInsideFace() below reinterprets a real mesh-owned FaceT as this
// subclass to call the promoted method. That's safe here because AccessibleFace adds no data
// members and no virtual functions -- the `using` declaration only changes name visibility, so the
// call resolves to the exact same non-virtual FaceT method on the exact same object.
template <class T>
struct AccessibleFace : public DCEL::FaceT<T, DCEL::DefaultMetaData>
{
  using DCEL::FaceT<T, DCEL::DefaultMetaData>::isPointInsideFace;
};

template <class T>
[[nodiscard]] bool
castIsPointInsideFace(DCEL::FaceT<T, DCEL::DefaultMetaData>&       a_face,
                      const Vec3T<T>&                              a_point,
                      const DCEL::MeshT<T, DCEL::DefaultMetaData>& a_mesh)
{
  return static_cast<AccessibleFace<T>&>(a_face).isPointInsideFace(a_point, a_mesh);
}

// A single DCEL face wired up from a vertex list, held inside a real DCEL::MeshT so the face's
// on-the-fly (half-edge-walking, index-based) containment test can be queried against it.
// reconcile() derives the normal, centroid, and 2D-projection axes from the geometry, matching how
// a parsed mesh's faces are set up.
template <class T>
struct BuiltFace
{
  using Meta = DCEL::DefaultMetaData;
  using Mesh = DCEL::MeshT<T, Meta>;

  Mesh m_mesh;

  explicit BuiltFace(const std::vector<Vec3T<T>>& a_positions)
  {
    const uint32_t N = static_cast<uint32_t>(a_positions.size());

    auto& vertices = m_mesh.getVertices();
    auto& edges    = m_mesh.getEdges();
    auto& faces    = m_mesh.getFaces();

    for (const auto& p : a_positions) {
      vertices.emplace_back(p);
    }

    for (uint32_t i = 0; i < N; i++) {
      edges.emplace_back(i); // half-edge i starts at vertex i
    }

    for (uint32_t i = 0; i < N; i++) {
      edges[i].setNextEdge((i + 1) % N);
      edges[i].setFace(0);
    }

    faces.emplace_back(0u); // half-edge index 0
    faces[0].reconcile(m_mesh);
  }

  [[nodiscard]] bool
  isPointInside(const Vec3T<T>& a_point, DCEL::InsideOutsideAlgorithm a_algorithm)
  {
    auto& face = m_mesh.getFaces()[0];

    face.setInsideOutsideAlgorithm(a_algorithm);

    return castIsPointInsideFace(face, a_point, m_mesh);
  }
};

// A unit square in the z=0 plane.
template <class T>
BuiltFace<T>
unitSquare()
{
  using Vec3 = Vec3T<T>;

  return BuiltFace<T>({Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(1, 1, 0), Vec3(0, 1, 0)});
}

// An L-shaped concave hexagon in the z=0 plane: the unit square with its top-right quadrant notched
// out. The point (0.75, 0.75, 0) sits in that notch -- inside the convex hull of the vertices, but
// outside the actual polygon.
template <class T>
BuiltFace<T>
lShape()
{
  using Vec3 = Vec3T<T>;

  return BuiltFace<T>(
    {Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(1, 0.5, 0), Vec3(0.5, 0.5, 0), Vec3(0.5, 1, 0), Vec3(0, 1, 0)});
}

} // namespace

TEMPLATE_TEST_CASE("FaceT: a point clearly inside a square reads as inside for every algorithm",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  BuiltFace<T> square = unitSquare<T>();

  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(square.isPointInside(Vec3(0.5, 0.5, 0), algo));
  }
}

TEMPLATE_TEST_CASE("FaceT: a point clearly outside a square reads as outside for every algorithm",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  BuiltFace<T> square = unitSquare<T>();

  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE_FALSE(square.isPointInside(Vec3(2, 2, 0), algo));
    REQUIRE_FALSE(square.isPointInside(Vec3(-1, 0.5, 0), algo));
  }
}

TEMPLATE_TEST_CASE("FaceT: containment ignores the coordinate normal to the polygon's plane",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  BuiltFace<T> square = unitSquare<T>();

  // The polygon lies in z=0, but containment only depends on (x, y): the projection direction
  // discards z, so a point "floating" well above or below the plane still reads as inside when its
  // (x, y) projects into the square (isPointInsideFace first projects onto the plane).
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(square.isPointInside(Vec3(0.5, 0.5, 100), algo));
    REQUIRE(square.isPointInside(Vec3(0.5, 0.5, -100), algo));
  }
}

TEMPLATE_TEST_CASE("FaceT: a concave polygon correctly excludes points in its notch, for every algorithm",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  BuiltFace<T> lPolygon = lShape<T>();

  // Inside the solid part of the L.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(lPolygon.isPointInside(Vec3(0.25, 0.25, 0), algo));
  }

  // Inside the convex hull of the vertices, but in the notched-out region: must read as outside.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE_FALSE(lPolygon.isPointInside(Vec3(0.75, 0.75, 0), algo));
  }

  // Clearly outside entirely.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE_FALSE(lPolygon.isPointInside(Vec3(-1, -1, 0), algo));
  }
}

TEMPLATE_TEST_CASE("FaceT: containment works for a triangle in a non-axis-aligned plane",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A triangle in the plane x + y + z = 3, with an oblique (but not axis-aligned) normal.
  BuiltFace<T> triangle({Vec3(3, 0, 0), Vec3(0, 3, 0), Vec3(0, 0, 3)});

  // The triangle's centroid (1, 1, 1) must project to the inside.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(triangle.isPointInside(Vec3(1, 1, 1), algo));
  }

  // A point outside the triangle but still in the same plane.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE_FALSE(triangle.isPointInside(Vec3(3, 3, -3), algo));
  }
}

TEMPLATE_TEST_CASE("FaceT: the three containment algorithms agree over a grid of query points",
                   "[DCEL][Face][Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  BuiltFace<T> lPolygon = lShape<T>();

  // Offset the grid so it never lands exactly on one of the L-shape's edges or vertices (all at
  // multiples of 0.25) -- boundary points are inherently ambiguous between algorithms and aren't a
  // meaningful place to expect exact agreement.
  for (int i = -2; i <= 6; i++) {
    for (int j = -2; j <= 6; j++) {
      const Vec3 p(T(0.25 * i + 0.07), T(0.25 * j + 0.11), T(0));

      const bool winding  = lPolygon.isPointInside(p, DCEL::InsideOutsideAlgorithm::WindingNumber);
      const bool crossing = lPolygon.isPointInside(p, DCEL::InsideOutsideAlgorithm::CrossingNumber);
      const bool subtend  = lPolygon.isPointInside(p, DCEL::InsideOutsideAlgorithm::SubtendedAngle);

      REQUIRE(winding == crossing);
      REQUIRE(winding == subtend);
    }
  }
}
