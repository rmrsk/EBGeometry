// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <cmath>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

using T   = double;
using Tri = Triangle<T, short>;

// Shared test geometry: a right triangle in the z = 0 plane with vertices
//   V0 = (0,0,0), V1 = (4,0,0), V2 = (0,4,0)
// Vertex ordering follows the right-hand rule, so the computed face normal is +z.
namespace {

Tri
makeRightTriangle()
{
  const Vec3T<T> v0(0.0, 0.0, 0.0);
  const Vec3T<T> v1(4.0, 0.0, 0.0);
  const Vec3T<T> v2(0.0, 4.0, 0.0);

  return Tri({v0, v1, v2});
}

} // namespace

TEST_CASE("Triangle: face normal is computed from vertex ordering", "[Triangle]")
{
  const Tri tri = makeRightTriangle();

  REQUIRE_THAT(tri.getNormal()[0], WithinAbs(0.0, 1e-12));
  REQUIRE_THAT(tri.getNormal()[1], WithinAbs(0.0, 1e-12));
  REQUIRE_THAT(tri.getNormal()[2], WithinAbs(1.0, 1e-12));
}

TEST_CASE("Triangle: signed distance for a point projecting to the face interior", "[Triangle]")
{
  // Point projects to (1,1,*), well inside the triangle's footprint (x + y < 4).
  Tri tri = makeRightTriangle();

  const Vec3T<T> unitZ(0.0, 0.0, 1.0);
  tri.setVertexNormals({unitZ, unitZ, unitZ});
  tri.setEdgeNormals({unitZ, unitZ, unitZ});

  // Directly above the face: distance is just the height above the plane.
  REQUIRE_THAT(tri.signedDistance(Vec3T<T>(1.0, 1.0, 5.0)), WithinRel(5.0));

  // Directly below the face: negative distance, same magnitude.
  REQUIRE_THAT(tri.signedDistance(Vec3T<T>(1.0, 1.0, -3.0)), WithinRel(-3.0));

  // On the surface, inside the footprint: zero distance.
  REQUIRE_THAT(tri.signedDistance(Vec3T<T>(1.0, 1.0, 0.0)), WithinAbs(0.0, 1e-12));
}

TEST_CASE("Triangle: signed distance for a point closest to an edge", "[Triangle]")
{
  // Point sits 1 unit outside the hypotenuse (V1-V2), perpendicular to its midpoint (2,2,0),
  // along the in-plane outward direction (1,1,0)/sqrt(2).
  Tri tri = makeRightTriangle();

  const Vec3T<T> unitZ(0.0, 0.0, 1.0);
  const Vec3T<T> outwardEdgeNormal = Vec3T<T>(1.0, 1.0, 0.0) / std::sqrt(2.0);

  tri.setVertexNormals({unitZ, unitZ, unitZ});
  // Edge normals are indexed 0: V0->V1, 1: V1->V2, 2: V2->V0. The hypotenuse is edge 1.
  tri.setEdgeNormals({unitZ, outwardEdgeNormal, unitZ});

  const T        offset = 1.0 / std::sqrt(2.0);
  const Vec3T<T> queryPoint(2.0 + offset, 2.0 + offset, 0.0);

  REQUIRE_THAT(tri.signedDistance(queryPoint), WithinRel(1.0));
}

TEST_CASE("Triangle: signed distance for a point closest to a vertex", "[Triangle]")
{
  // Point sits beyond V1 = (4,0,0), outside the range of both adjacent edges, so the
  // closest feature is the vertex itself.
  Tri tri = makeRightTriangle();

  const Vec3T<T> unitZ(0.0, 0.0, 1.0);
  const Vec3T<T> outwardVertexNormal = Vec3T<T>(1.0, -1.0, 0.0) / std::sqrt(2.0);

  // Vertex normals are indexed to match m_vertexPositions: 0 -> V0, 1 -> V1, 2 -> V2.
  tri.setVertexNormals({unitZ, outwardVertexNormal, unitZ});
  tri.setEdgeNormals({unitZ, unitZ, unitZ});

  const Vec3T<T> queryPoint(6.0, -2.0, 0.0);
  const T        expectedDistance = std::sqrt(8.0); // length((2,-2,0)) = 2*sqrt(2)

  REQUIRE_THAT(tri.signedDistance(queryPoint), WithinRel(expectedDistance));
}
