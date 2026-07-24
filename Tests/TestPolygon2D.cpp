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
// signedDistance()) so the containment algorithms can be exercised directly.
template <class T>
struct AccessibleFace : public DCEL::FaceT<T, DCEL::DefaultMetaData>
{
  using DCEL::FaceT<T, DCEL::DefaultMetaData>::isPointInsideFace;
};

// A single DCEL face wired up from a vertex list, holding its vertices/edges alive so the face's
// on-the-fly (half-edge-walking) containment test can be queried. reconcile() derives the normal,
// centroid, and 2D-projection axes from the geometry, matching how a parsed mesh's faces are set up.
template <class T>
struct BuiltFace
{
  using Vertex = DCEL::VertexT<T, DCEL::DefaultMetaData>;
  using Edge   = DCEL::EdgeT<T, DCEL::DefaultMetaData>;
  using Face   = AccessibleFace<T>;

  std::shared_ptr<Face>                m_face;
  std::vector<std::shared_ptr<Vertex>> m_vertices;
  std::vector<std::shared_ptr<Edge>>   m_edges;

  explicit BuiltFace(const std::vector<Vec3T<T>>& a_positions)
  {
    const size_t N = a_positions.size();

    m_face = std::make_shared<Face>();

    for (size_t i = 0; i < N; i++) {
      m_vertices.push_back(std::make_shared<Vertex>(a_positions[i]));
      m_edges.push_back(std::make_shared<Edge>());
    }

    for (size_t i = 0; i < N; i++) {
      m_edges[i]->setVertex(m_vertices[i]);
      m_edges[i]->setNextEdge(m_edges[(i + 1) % N]);
      m_edges[i]->setFace(m_face);
    }

    m_face->setHalfEdge(m_edges[0]);
    m_face->reconcile();
  }

  [[nodiscard]] bool
  isPointInside(const Vec3T<T>& a_point, DCEL::InsideOutsideAlgorithm a_algorithm) const
  {
    m_face->setInsideOutsideAlgorithm(a_algorithm);

    return m_face->isPointInsideFace(a_point);
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

  const BuiltFace<T> square = unitSquare<T>();

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

  const BuiltFace<T> square = unitSquare<T>();

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

  const BuiltFace<T> square = unitSquare<T>();

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

  const BuiltFace<T> lPolygon = lShape<T>();

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
  const BuiltFace<T> triangle({Vec3(3, 0, 0), Vec3(0, 3, 0), Vec3(0, 0, 3)});

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

  const BuiltFace<T> lPolygon = lShape<T>();

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
