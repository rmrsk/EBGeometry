// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_Polygon2D.hpp. Previously only exercised indirectly through
// DCEL::FaceT/MeshT's point-in-face containment logic (used by every TestDCEL.cpp signed-distance
// test); this covers the class directly, including concave polygons and non-axis-aligned planes
// that a mesh-level test wouldn't necessarily hit.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

template <class T>
using Poly = Polygon2D<T>;

namespace {

template <class T>
using Algo = typename Poly<T>::InsideOutsideAlgorithm;

template <class T>
std::array<Algo<T>, 3>
allAlgorithms()
{
  return {Algo<T>::WindingNumber, Algo<T>::CrossingNumber, Algo<T>::SubtendedAngle};
}

// A unit square in the z=0 plane, vertices ordered counterclockwise when viewed from +z (so the
// normal (0,0,1) matches the right-hand rule, though the sign shouldn't matter for containment).
template <class T>
Poly<T>
unitSquare()
{
  using Vec3 = Vec3T<T>;

  const std::vector<Vec3> points = {Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(1, 1, 0), Vec3(0, 1, 0)};
  return {Vec3(0, 0, 1), points};
}

// An L-shaped concave hexagon in the z=0 plane: the unit square with its top-right quadrant
// notched out. The point (0.75, 0.75, 0) sits in that notch -- inside the convex hull of the
// vertices, but outside the actual polygon.
template <class T>
Poly<T>
lShape()
{
  using Vec3 = Vec3T<T>;

  const std::vector<Vec3> points = {
    Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(1, 0.5, 0), Vec3(0.5, 0.5, 0), Vec3(0.5, 1, 0), Vec3(0, 1, 0)};
  return {Vec3(0, 0, 1), points};
}

} // namespace

TEMPLATE_TEST_CASE("Polygon2D: a point clearly inside a square reads as inside for every algorithm",
                   "[Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Poly<T> square = unitSquare<T>();

  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(square.isPointInside(Vec3(0.5, 0.5, 0), algo));
  }
}

TEMPLATE_TEST_CASE("Polygon2D: a point clearly outside a square reads as outside for every algorithm",
                   "[Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Poly<T> square = unitSquare<T>();

  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE_FALSE(square.isPointInside(Vec3(2, 2, 0), algo));
    REQUIRE_FALSE(square.isPointInside(Vec3(-1, 0.5, 0), algo));
  }
}

TEMPLATE_TEST_CASE("Polygon2D: containment ignores the coordinate normal to the polygon's plane",
                   "[Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Poly<T> square = unitSquare<T>();

  // The polygon lies in z=0, but containment only depends on (x, y); the projection direction
  // discards z entirely, so a point "floating" well above or below the plane still reads as
  // inside when its (x, y) projects into the square.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(square.isPointInside(Vec3(0.5, 0.5, 100), algo));
    REQUIRE(square.isPointInside(Vec3(0.5, 0.5, -100), algo));
  }
}

TEMPLATE_TEST_CASE("Polygon2D: dedicated per-algorithm methods agree with isPointInside's dispatch",
                   "[Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Poly<T> square = unitSquare<T>();

  for (const Vec3 p : {Vec3(0.5, 0.5, 0), Vec3(2, 2, 0)}) {
    REQUIRE(square.isPointInside(p, Algo<T>::WindingNumber) == square.isPointInsidePolygonWindingNumber(p));
    REQUIRE(square.isPointInside(p, Algo<T>::CrossingNumber) == square.isPointInsidePolygonCrossingNumber(p));
    REQUIRE(square.isPointInside(p, Algo<T>::SubtendedAngle) == square.isPointInsidePolygonSubtend(p));
  }
}

TEMPLATE_TEST_CASE("Polygon2D: a concave polygon correctly excludes points in its notch, for every algorithm",
                   "[Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Poly<T> lPolygon = lShape<T>();

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

TEMPLATE_TEST_CASE("Polygon2D: works for a triangle embedded in a non-axis-aligned plane",
                   "[Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A triangle in the plane x + y + z = 3, with an oblique (but not axis-aligned) normal.
  const std::vector<Vec3> points = {Vec3(3, 0, 0), Vec3(0, 3, 0), Vec3(0, 0, 3)};
  const Poly<T>           triangle(Vec3(1, 1, 1), points);

  // The triangle's centroid (1, 1, 1) must project to the inside.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE(triangle.isPointInside(Vec3(1, 1, 1), algo));
  }

  // A point outside the triangle but still in the same plane.
  for (const auto algo : allAlgorithms<T>()) {
    REQUIRE_FALSE(triangle.isPointInside(Vec3(3, 3, -3), algo));
  }
}

TEMPLATE_TEST_CASE("Polygon2D: the three algorithms agree with each other over a grid of query points",
                   "[Polygon2D]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Poly<T> lPolygon = lShape<T>();

  // Offset the grid so it never lands exactly on one of the L-shape's edges or vertices (all at
  // multiples of 0.25) -- boundary points are inherently ambiguous between algorithms and aren't
  // a meaningful place to expect exact agreement.
  for (int i = -2; i <= 6; i++) {
    for (int j = -2; j <= 6; j++) {
      const Vec3 p(T(0.25 * i + 0.07), T(0.25 * j + 0.11), T(0));

      const bool winding  = lPolygon.isPointInside(p, Algo<T>::WindingNumber);
      const bool crossing = lPolygon.isPointInside(p, Algo<T>::CrossingNumber);
      const bool subtend  = lPolygon.isPointInside(p, Algo<T>::SubtendedAngle);

      REQUIRE(winding == crossing);
      REQUIRE(winding == subtend);
    }
  }
}
