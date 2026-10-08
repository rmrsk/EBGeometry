// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for PolygonSoup: construction, properties, clean() and the index maps behind it, and the
// conversions to a DCEL mesh and to triangles. The file readers that return it are tested in
// TestParser.cpp.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

namespace {

// A raw soup in which every face lists its own copies of its vertices, as an STL file stores them.
template <class T>
PolygonSoup<T>
rawSoup(const std::string& a_id, const std::vector<std::vector<Vec3T<T>>>& a_faces)
{
  std::vector<Vec3T<T>>            vertices;
  std::vector<std::vector<size_t>> facets;

  for (const auto& face : a_faces) {
    std::vector<size_t> facet;

    for (const auto& x : face) {
      vertices.emplace_back(x);
      facet.emplace_back(vertices.size() - 1);
    }

    facets.emplace_back(facet);
  }

  return PolygonSoup<T>(a_id, vertices, facets);
}

// A tetrahedron with outward-facing triangles, twelve vertices before compression.
template <class T>
PolygonSoup<T>
tetrahedron()
{
  const Vec3T<T> A(0, 0, 0);
  const Vec3T<T> B(1, 0, 0);
  const Vec3T<T> C(0, 1, 0);
  const Vec3T<T> D(0, 0, 1);

  return rawSoup<T>("tetrahedron", {{A, C, B}, {A, B, D}, {A, D, C}, {B, C, D}});
}

// A unit square split along its diagonal 0-2, where one side of the diagonal is split at its midpoint
// M and the T-junction closed by the zero-area face {P2, P0, M}. Face 4 collapses to a segment once
// its repeated vertex is merged. Cleaning keeps faces 0, 1 and 3, and face 3 gains M.
template <class T>
PolygonSoup<T>
tJunctionSquare()
{
  const Vec3T<T> P0(0, 0, 0);
  const Vec3T<T> P1(1, 0, 0);
  const Vec3T<T> P2(1, 1, 0);
  const Vec3T<T> P3(0, 1, 0);
  const Vec3T<T> M(T(0.5), T(0.5), 0);

  return rawSoup<T>("t-junction", {{P0, P1, M}, {P1, P2, M}, {P2, P0, M}, {P0, P2, P3}, {P0, M, M}});
}

// The positions of a soup face's vertices, in order.
template <class T>
std::vector<Vec3T<T>>
facePositions(const PolygonSoup<T>& a_soup, const size_t a_face)
{
  std::vector<Vec3T<T>> positions;

  for (const size_t v : a_soup.getFacets()[a_face]) {
    positions.push_back(a_soup.getVertexCoordinates()[v]);
  }

  return positions;
}

// The positions of a mesh face's vertices, in order around the face.
template <class T>
std::vector<Vec3T<T>>
facePositions(const DCEL::MeshT<T>& a_mesh, const uint32_t a_face)
{
  std::vector<Vec3T<T>> positions;

  const uint32_t first = a_mesh.getFace(a_face).getHalfEdgeIndex();
  uint32_t       edge  = first;

  do {
    positions.push_back(a_mesh.getVertex(a_mesh.getEdge(edge).getVertexIndex()).getPosition());

    edge = a_mesh.getEdge(edge).getNextEdgeIndex();
  } while (edge != first);

  return positions;
}

} // namespace

TEMPLATE_TEST_CASE("PolygonSoup: default and id construction give an empty, unclean soup",
                   "[PolygonSoup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const PolygonSoup<T> empty;
  const PolygonSoup<T> named("my-id");

  CHECK(empty.getID().empty());
  CHECK(named.getID() == "my-id");

  for (const auto* soup : {&empty, &named}) {
    CHECK(soup->numVertices() == 0);
    CHECK(soup->numFacets() == 0);
    CHECK(soup->getVertexPropertyNames().empty());
    CHECK(soup->getFacePropertyNames().empty());
    CHECK_FALSE(soup->isClean());
  }
}

TEMPLATE_TEST_CASE("PolygonSoup: properties are stored by name, and an unknown name throws",
                   "[PolygonSoup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  PolygonSoup<T> soup = tetrahedron<T>();

  soup.setVertexProperty("temperature", std::vector<T>(12, T(3)));
  soup.setFaceProperty("quality", {T(1), T(2), T(3), T(4)});
  soup.setFaceProperty("area", std::vector<T>(4, T(0)));

  CHECK(soup.hasVertexProperty("temperature"));
  CHECK_FALSE(soup.hasVertexProperty("quality"));
  CHECK(soup.hasFaceProperty("quality"));
  CHECK(soup.getFaceProperty("quality") == std::vector<T>{T(1), T(2), T(3), T(4)});
  CHECK(soup.getVertexPropertyNames() == std::vector<std::string>{"temperature"});
  CHECK(soup.getFacePropertyNames() == std::vector<std::string>{"area", "quality"});

  CHECK_THROWS_AS(soup.getVertexProperty("does-not-exist"), std::out_of_range);
  CHECK_THROWS_AS(soup.getFaceProperty("does-not-exist"), std::out_of_range);

  // Replacing a property keeps one entry.
  soup.setFaceProperty("quality", std::vector<T>(4, T(7)));

  CHECK(soup.getFacePropertyNames().size() == 2);
  CHECK(soup.getFaceProperty("quality")[0] == T(7));
}

TEMPLATE_TEST_CASE("PolygonSoup: a property of the wrong size is rejected", "[PolygonSoup]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  REQUIRE(abortsWith(
    [] {
      PolygonSoup<T> soup = tetrahedron<T>();

      soup.setFaceProperty("quality", std::vector<T>(3, T(0)));
    },
    "PolygonSoup: face property 'quality' has 3 values for 4 faces"));

  REQUIRE(abortsWith(
    [] {
      PolygonSoup<T> soup = tetrahedron<T>();

      soup.setVertexProperty("temperature", std::vector<T>(4, T(0)));
    },
    "PolygonSoup: vertex property 'temperature' has 4 values for 12 vertices"));
}

TEMPLATE_TEST_CASE("PolygonSoup: clean() merges duplicate vertices, and cleaning again changes nothing",
                   "[PolygonSoup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  PolygonSoup<T> soup = tetrahedron<T>();

  REQUIRE(soup.numVertices() == 12);
  REQUIRE_FALSE(soup.isClean());

  CHECK(soup.clean() == 0);
  CHECK(soup.isClean());
  CHECK(soup.numVertices() == 4);
  CHECK(soup.numFacets() == 4);

  const auto vertices = soup.getVertexCoordinates();
  const auto facets   = soup.getFacets();

  CHECK(soup.clean() == 0);
  CHECK(soup.getVertexCoordinates() == vertices);
  CHECK(soup.getFacets() == facets);
}

TEMPLATE_TEST_CASE("PolygonSoup: clean() removes degenerate faces and carries the properties along",
                   "[PolygonSoup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  PolygonSoup<T> soup = tJunctionSquare<T>();

  const std::vector<Vec3T<T>> raw = soup.getVertexCoordinates();

  REQUIRE(raw.size() == 15);

  // Each raw vertex's own index, and ten times each face's index.
  std::vector<T> rawIndex(raw.size());
  std::vector<T> faceTag(soup.numFacets());

  for (size_t v = 0; v < raw.size(); v++) {
    rawIndex[v] = T(v);
  }

  for (size_t f = 0; f < faceTag.size(); f++) {
    faceTag[f] = T(10 * f);
  }

  soup.setVertexProperty("rawIndex", rawIndex);
  soup.setFaceProperty("tag", faceTag);

  const std::vector<Vec3T<T>> repairedNeighbour = {
    Vec3T<T>(0, 0, 0), Vec3T<T>(T(0.5), T(0.5), 0), Vec3T<T>(1, 1, 0), Vec3T<T>(0, 1, 0)};

  // The filler and the collapsed face go; the other three keep their order and their tags.
  REQUIRE(soup.clean() == 2);
  REQUIRE(soup.numVertices() == 5);
  REQUIRE(soup.numFacets() == 3);
  CHECK(soup.getFaceProperty("tag") == std::vector<T>{T(0), T(10), T(30)});
  CHECK(facePositions(soup, 2) == repairedNeighbour);

  // A merged vertex takes its properties from the first raw vertex at its position.
  for (size_t v = 0; v < soup.numVertices(); v++) {
    size_t first = 0;

    while (raw[first] != soup.getVertexCoordinates()[v]) {
      first++;
    }

    CHECK(soup.getVertexProperty("rawIndex")[v] == T(first));
  }
}

TEMPLATE_TEST_CASE("PolygonSoup: clean() rejects an index out of range or a non-finite coordinate",
                   "[PolygonSoup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  PolygonSoup<T> outOfRange("bad-index", {Vec3T<T>(0, 0, 0), Vec3T<T>(1, 0, 0), Vec3T<T>(0, 1, 0)}, {{0, 1, 3}});
  PolygonSoup<T> nonFinite(
    "bad-coordinate", {Vec3T<T>(0, 0, 0), Vec3T<T>(1, 0, 0), Vec3T<T>(0, Math::Limits<T>::infinity(), 0)}, {{0, 1, 2}});

  CHECK_THROWS_AS(outOfRange.clean(), Parser::ParseError);
  CHECK_THROWS_AS(nonFinite.clean(), Parser::ParseError);

  Pool pool(hostMemoryResource());

  CHECK_THROWS_AS(outOfRange.convertToDCEL(pool), Parser::ParseError);
}

TEMPLATE_TEST_CASE("PolygonSoup: face i and vertex i of a clean soup are face i and vertex i of its mesh",
                   "[PolygonSoup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  for (auto soup : {tetrahedron<T>(), tJunctionSquare<T>()}) {
    INFO(soup.getID());

    soup.clean();

    const DCEL::MeshT<T> mesh = soup.convertToDCEL(pool);

    REQUIRE(mesh.numFaces() == soup.numFacets());
    REQUIRE(mesh.numVertices() == soup.numVertices());

    for (uint32_t f = 0; f < mesh.numFaces(); f++) {
      CHECK(facePositions(mesh, f) == facePositions(soup, f));
    }

    for (uint32_t v = 0; v < mesh.numVertices(); v++) {
      CHECK(mesh.getVertex(v).getPosition() == soup.getVertexCoordinates()[v]);
    }
  }
}

TEMPLATE_TEST_CASE("PolygonSoup: convertToDCEL() of an unclean soup cleans a copy and leaves the soup as it was",
                   "[PolygonSoup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const PolygonSoup<T> raw = tJunctionSquare<T>();
  PolygonSoup<T>       cleaned(raw);

  cleaned.clean();

  const DCEL::MeshT<T> fromRaw     = raw.convertToDCEL(pool);
  const DCEL::MeshT<T> fromCleaned = cleaned.convertToDCEL(pool);

  CHECK_FALSE(raw.isClean());
  CHECK(raw.numVertices() == 15);
  CHECK(raw.numFacets() == 5);

  REQUIRE(fromRaw.numFaces() == fromCleaned.numFaces());
  REQUIRE(fromRaw.numVertices() == fromCleaned.numVertices());

  for (uint32_t f = 0; f < fromRaw.numFaces(); f++) {
    CHECK(facePositions(fromRaw, f) == facePositions(fromCleaned, f));
  }
}

TEMPLATE_TEST_CASE("PolygonSoup: convertToDCEL() throws for a soup with no faces, or a face that visits a vertex twice",
                   "[PolygonSoup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const PolygonSoup<T> noFaces("no-faces", {Vec3T<T>(0, 0, 0), Vec3T<T>(1, 0, 0), Vec3T<T>(0, 1, 0)}, {});

  // A bow tie: one face through vertex 0 twice.
  const PolygonSoup<T> bowTie(
    "bow-tie",
    {Vec3T<T>(0, 0, 0), Vec3T<T>(1, 0, 0), Vec3T<T>(1, 1, 0), Vec3T<T>(-1, 0, 0), Vec3T<T>(-1, -1, 0)},
    {{0, 1, 2, 0, 3, 4}});

  CHECK_THROWS_WITH(noFaces.convertToDCEL(pool), "no-faces: the mesh has no faces");
  CHECK_THROWS_WITH(bowTie.convertToDCEL(pool), "bow-tie: face 0 visits the same vertex twice");
}

TEMPLATE_TEST_CASE("PolygonSoup: convertToTriangles() fans each face and tags each triangle with its face",
                   "[PolygonSoup]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  PolygonSoup<T> soup = tJunctionSquare<T>();

  soup.clean();

  const std::vector<Triangle<T>> triangles = soup.convertToTriangles();

  // Two triangles and a quadrilateral.
  REQUIRE(triangles.size() == 4);

  std::array<size_t, 3> perFace = {0, 0, 0};

  for (const auto& triangle : triangles) {
    REQUIRE(triangle.getFaceId() < 3);

    perFace[triangle.getFaceId()]++;
  }

  CHECK(perFace == std::array<size_t, 3>{1, 1, 2});
}
