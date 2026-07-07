// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <array>
#include <vector>

#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

using TestOBJ = OBJ<double>;

// Build a raw (uncompressed) tetrahedron soup directly into an OBJ<T> object, mimicking how a
// real OBJ file stores each facet's vertex indices independently (i.e. with duplicates).
static TestOBJ
buildTetrahedronSoup()
{
  TestOBJ obj("tetrahedron-soup");

  auto& vertices = obj.getVertexCoordinates();
  auto& facets   = obj.getFacets();

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

  return obj;
}

TEST_CASE("OBJ: default construction is empty", "[OBJ]")
{
  TestOBJ obj;

  REQUIRE(obj.getID().empty());
  REQUIRE(obj.getVertexCoordinates().empty());
  REQUIRE(obj.getFacets().empty());
}

TEST_CASE("OBJ: id constructor sets the identifier only", "[OBJ]")
{
  TestOBJ obj("my-id");

  REQUIRE(obj.getID() == "my-id");
  REQUIRE(obj.getVertexCoordinates().empty());
  REQUIRE(obj.getFacets().empty());
}

TEST_CASE("OBJ: copy construction and copy assignment duplicate data independently", "[OBJ]")
{
  auto src = buildTetrahedronSoup();

  TestOBJ copyCtor(src);
  REQUIRE(copyCtor.getID() == src.getID());
  REQUIRE(copyCtor.getVertexCoordinates() == src.getVertexCoordinates());
  REQUIRE(copyCtor.getFacets() == src.getFacets());

  // Mutating the copy must not affect the original.
  copyCtor.getVertexCoordinates()[0] = Vec3T<double>(999, 999, 999);
  REQUIRE(src.getVertexCoordinates()[0] != Vec3T<double>(999, 999, 999));

  TestOBJ copyAssign;
  copyAssign = src;
  REQUIRE(copyAssign.getVertexCoordinates() == src.getVertexCoordinates());

  copyAssign.getFacets().clear();
  REQUIRE_FALSE(src.getFacets().empty());
}

TEST_CASE("OBJ: move construction and move assignment transfer data", "[OBJ]")
{
  auto       moveCtorSrc        = buildTetrahedronSoup();
  const auto expectedVertexSize = moveCtorSrc.getVertexCoordinates().size();

  TestOBJ moved(std::move(moveCtorSrc));
  REQUIRE(moved.getID() == "tetrahedron-soup");
  REQUIRE(moved.getVertexCoordinates().size() == expectedVertexSize);

  auto    moveAssignSrc = buildTetrahedronSoup();
  TestOBJ moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getID() == "tetrahedron-soup");
  REQUIRE(moveAssignDst.getVertexCoordinates().size() == expectedVertexSize);
}

TEST_CASE("OBJ: convertToDCEL builds a mesh with the correct compressed vertex and face counts", "[OBJ]")
{
  auto obj = buildTetrahedronSoup();

  // The raw soup has one vertex entry per facet corner (no compression yet).
  REQUIRE(obj.getVertexCoordinates().size() == 12);
  REQUIRE(obj.getFacets().size() == 4);

  auto mesh = obj.convertToDCEL<DCEL::DefaultMetaData>();

  REQUIRE(mesh != nullptr);
  REQUIRE(mesh->getVertices().size() == 4); // Compressed down to the 4 unique corners.
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getEdges().size() == 12); // 4 triangular faces * 3 half-edges each.

  for (const auto& e : mesh->getEdges()) {
    REQUIRE(e->getPairEdge() != nullptr); // Watertight: every half-edge has a pair.
  }
}
