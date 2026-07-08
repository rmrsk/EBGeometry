// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <array>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

template <class T>
using Stl = STL<T>;

// Path to test data injected by CMake.
static const std::string g_dataDir = EBGEOMETRY_TEST_DATA_DIR;

// Build a raw (uncompressed) tetrahedron soup directly into an STL<T> object, mimicking how a
// real STL file stores each facet's vertices independently (i.e. with duplicates).
template <class T>
static Stl<T>
buildTetrahedronSoup()
{
  Stl<T> stl("tetrahedron-soup");

  auto& vertices = stl.getVertexCoordinates();
  auto& facets   = stl.getFacets();

  const Vec3T<T> A(0, 0, 0);
  const Vec3T<T> B(1, 0, 0);
  const Vec3T<T> C(0, 1, 0);
  const Vec3T<T> D(0, 0, 1);

  const std::vector<std::array<Vec3T<T>, 3>> faces = {
    {A, C, B},
    {A, B, D},
    {A, D, C},
    {B, C, D},
  };

  for (const auto& face : faces) {
    std::vector<size_t> facet;
    for (const auto& v : face) {
      vertices.emplace_back(v);
      facet.emplace_back(vertices.size() - 1);
    }
    facets.emplace_back(facet);
  }

  return stl;
}

TEMPLATE_TEST_CASE("STL: default construction is empty", "[STL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Stl<T> stl;

  REQUIRE(stl.getID().empty());
  REQUIRE(stl.getVertexCoordinates().empty());
  REQUIRE(stl.getFacets().empty());
}

TEMPLATE_TEST_CASE("STL: id constructor sets the identifier only", "[STL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Stl<T> stl("my-id");

  REQUIRE(stl.getID() == "my-id");
  REQUIRE(stl.getVertexCoordinates().empty());
  REQUIRE(stl.getFacets().empty());
}

TEMPLATE_TEST_CASE("STL: copy construction and copy assignment duplicate data independently",
                   "[STL]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto src = buildTetrahedronSoup<T>();

  Stl<T> copyCtor(src);
  REQUIRE(copyCtor.getID() == src.getID());
  REQUIRE(copyCtor.getVertexCoordinates() == src.getVertexCoordinates());
  REQUIRE(copyCtor.getFacets() == src.getFacets());

  // Mutating the copy must not affect the original.
  copyCtor.getVertexCoordinates()[0] = Vec3T<T>(999, 999, 999);
  REQUIRE(src.getVertexCoordinates()[0] != Vec3T<T>(999, 999, 999));

  Stl<T> copyAssign;
  copyAssign = src;
  REQUIRE(copyAssign.getVertexCoordinates() == src.getVertexCoordinates());

  copyAssign.getFacets().clear();
  REQUIRE_FALSE(src.getFacets().empty());
}

TEMPLATE_TEST_CASE("STL: move construction and move assignment transfer data", "[STL]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto       moveCtorSrc        = buildTetrahedronSoup<T>();
  const auto expectedVertexSize = moveCtorSrc.getVertexCoordinates().size();

  Stl<T> moved(std::move(moveCtorSrc));
  REQUIRE(moved.getID() == "tetrahedron-soup");
  REQUIRE(moved.getVertexCoordinates().size() == expectedVertexSize);

  auto   moveAssignSrc = buildTetrahedronSoup<T>();
  Stl<T> moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getID() == "tetrahedron-soup");
  REQUIRE(moveAssignDst.getVertexCoordinates().size() == expectedVertexSize);
}

TEMPLATE_TEST_CASE("STL: convertToDCEL builds a mesh with the correct compressed vertex and face counts",
                   "[STL]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto stl = buildTetrahedronSoup<T>();

  // The raw soup has one vertex entry per facet corner (no compression yet).
  REQUIRE(stl.getVertexCoordinates().size() == 12);
  REQUIRE(stl.getFacets().size() == 4);

  auto mesh = stl.template convertToDCEL<DCEL::DefaultMetaData>();

  REQUIRE(mesh != nullptr);
  REQUIRE(mesh->getVertices().size() == 4); // Compressed down to the 4 unique corners.
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getEdges().size() == 12); // 4 triangular faces * 3 half-edges each.

  // convertToDCEL() must not mutate the STL object's own (uncompressed) data.
  REQUIRE(stl.getVertexCoordinates().size() == 12);
}

TEMPLATE_TEST_CASE("Parser::readSTL reads the test tetrahedron file into raw vertex/facet data",
                   "[STL]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto stl = Parser::readSTL<T>(g_dataDir + "/tetrahedron.stl");

  REQUIRE_FALSE(stl.getVertexCoordinates().empty());
  REQUIRE(stl.getFacets().size() == 4); // A tetrahedron has 4 triangular faces.

  for (const auto& facet : stl.getFacets()) {
    REQUIRE(facet.size() == 3);
  }
}

TEMPLATE_TEST_CASE("Parser::readSTL + convertToDCEL round-trips into a valid, watertight DCEL mesh",
                   "[STL]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto stl  = Parser::readSTL<T>(g_dataDir + "/tetrahedron.stl");
  auto mesh = stl.template convertToDCEL<DCEL::DefaultMetaData>();

  REQUIRE(mesh != nullptr);
  REQUIRE(mesh->getVertices().size() == 4);
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getEdges().size() == 12);

  for (const auto& e : mesh->getEdges()) {
    REQUIRE(e->getPairEdge() != nullptr); // Watertight: every half-edge has a pair.
  }
}
