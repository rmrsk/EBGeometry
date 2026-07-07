// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <array>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

using TestSTL = STL<double>;

// Path to test data injected by CMake.
static const std::string g_dataDir = EBGEOMETRY_TEST_DATA_DIR;

// Build a raw (uncompressed) tetrahedron soup directly into an STL<T> object, mimicking how a
// real STL file stores each facet's vertices independently (i.e. with duplicates).
static TestSTL
buildTetrahedronSoup()
{
  TestSTL stl("tetrahedron-soup");

  auto& vertices = stl.getVertexCoordinates();
  auto& facets   = stl.getFacets();

  const Vec3T<double> A(0, 0, 0);
  const Vec3T<double> B(1, 0, 0);
  const Vec3T<double> C(0, 1, 0);
  const Vec3T<double> D(0, 0, 1);

  const std::vector<std::array<Vec3T<double>, 3>> faces = {
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

TEST_CASE("STL: default construction is empty", "[STL]")
{
  TestSTL stl;

  REQUIRE(stl.getID().empty());
  REQUIRE(stl.getVertexCoordinates().empty());
  REQUIRE(stl.getFacets().empty());
}

TEST_CASE("STL: id constructor sets the identifier only", "[STL]")
{
  TestSTL stl("my-id");

  REQUIRE(stl.getID() == "my-id");
  REQUIRE(stl.getVertexCoordinates().empty());
  REQUIRE(stl.getFacets().empty());
}

TEST_CASE("STL: copy construction and copy assignment duplicate data independently", "[STL]")
{
  auto src = buildTetrahedronSoup();

  TestSTL copyCtor(src);
  REQUIRE(copyCtor.getID() == src.getID());
  REQUIRE(copyCtor.getVertexCoordinates() == src.getVertexCoordinates());
  REQUIRE(copyCtor.getFacets() == src.getFacets());

  // Mutating the copy must not affect the original.
  copyCtor.getVertexCoordinates()[0] = Vec3T<double>(999, 999, 999);
  REQUIRE(src.getVertexCoordinates()[0] != Vec3T<double>(999, 999, 999));

  TestSTL copyAssign;
  copyAssign = src;
  REQUIRE(copyAssign.getVertexCoordinates() == src.getVertexCoordinates());

  copyAssign.getFacets().clear();
  REQUIRE_FALSE(src.getFacets().empty());
}

TEST_CASE("STL: move construction and move assignment transfer data", "[STL]")
{
  auto       moveCtorSrc        = buildTetrahedronSoup();
  const auto expectedVertexSize = moveCtorSrc.getVertexCoordinates().size();

  TestSTL moved(std::move(moveCtorSrc));
  REQUIRE(moved.getID() == "tetrahedron-soup");
  REQUIRE(moved.getVertexCoordinates().size() == expectedVertexSize);

  auto    moveAssignSrc = buildTetrahedronSoup();
  TestSTL moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getID() == "tetrahedron-soup");
  REQUIRE(moveAssignDst.getVertexCoordinates().size() == expectedVertexSize);
}

TEST_CASE("STL: convertToDCEL builds a mesh with the correct compressed vertex and face counts", "[STL]")
{
  auto stl = buildTetrahedronSoup();

  // The raw soup has one vertex entry per facet corner (no compression yet).
  REQUIRE(stl.getVertexCoordinates().size() == 12);
  REQUIRE(stl.getFacets().size() == 4);

  auto mesh = stl.convertToDCEL<DCEL::DefaultMetaData>();

  REQUIRE(mesh != nullptr);
  REQUIRE(mesh->getVertices().size() == 4); // Compressed down to the 4 unique corners.
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getEdges().size() == 12); // 4 triangular faces * 3 half-edges each.

  // convertToDCEL() must not mutate the STL object's own (uncompressed) data.
  REQUIRE(stl.getVertexCoordinates().size() == 12);
}

TEST_CASE("Parser::readSTL reads the test tetrahedron file into raw vertex/facet data", "[STL]")
{
  auto stl = Parser::readSTL<double>(g_dataDir + "/tetrahedron.stl");

  REQUIRE_FALSE(stl.getVertexCoordinates().empty());
  REQUIRE(stl.getFacets().size() == 4); // A tetrahedron has 4 triangular faces.

  for (const auto& facet : stl.getFacets()) {
    REQUIRE(facet.size() == 3);
  }
}

TEST_CASE("Parser::readSTL + convertToDCEL round-trips into a valid, watertight DCEL mesh", "[STL]")
{
  auto stl  = Parser::readSTL<double>(g_dataDir + "/tetrahedron.stl");
  auto mesh = stl.convertToDCEL<DCEL::DefaultMetaData>();

  REQUIRE(mesh != nullptr);
  REQUIRE(mesh->getVertices().size() == 4);
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getEdges().size() == 12);

  for (const auto& e : mesh->getEdges()) {
    REQUIRE(e->getPairEdge() != nullptr); // Watertight: every half-edge has a pair.
  }
}
