// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <array>
#include <stdexcept>
#include <vector>

#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

using TestPLY = PLY<double>;

// Build a raw (uncompressed) tetrahedron soup directly into a PLY<T> object, mimicking how a real
// PLY file stores each facet's vertex indices independently (i.e. with duplicates).
static TestPLY
buildTetrahedronSoup()
{
  TestPLY ply("tetrahedron-soup");

  auto& vertices = ply.getVertexCoordinates();
  auto& facets   = ply.getFacets();

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

  return ply;
}

TEST_CASE("PLY: default construction is empty", "[PLY]")
{
  TestPLY ply;

  REQUIRE(ply.getID().empty());
  REQUIRE(ply.getVertexCoordinates().empty());
  REQUIRE(ply.getFacets().empty());
}

TEST_CASE("PLY: id constructor sets the identifier only", "[PLY]")
{
  TestPLY ply("my-id");

  REQUIRE(ply.getID() == "my-id");
  REQUIRE(ply.getVertexCoordinates().empty());
  REQUIRE(ply.getFacets().empty());
}

TEST_CASE("PLY: setVertexProperties/setFaceProperties store data retrievable by name", "[PLY]")
{
  TestPLY ply;

  ply.setVertexProperties("nx", std::vector<double>{0.0, 0.0, 1.0});
  ply.setFaceProperties("quality", std::vector<double>{1.0, 2.0});

  REQUIRE(ply.getVertexProperties("nx") == std::vector<double>{0.0, 0.0, 1.0});
  REQUIRE(ply.getFaceProperties("quality") == std::vector<double>{1.0, 2.0});
}

TEST_CASE("PLY: getVertexProperties/getFaceProperties throw on an unknown property name", "[PLY]")
{
  TestPLY ply;
  ply.setVertexProperties("nx", std::vector<double>{1.0});

  REQUIRE_THROWS_AS(ply.getVertexProperties("does-not-exist"), std::out_of_range);
  REQUIRE_THROWS_AS(ply.getFaceProperties("does-not-exist"), std::out_of_range);
}

TEST_CASE("PLY: copy construction and copy assignment duplicate data independently", "[PLY]")
{
  auto src = buildTetrahedronSoup();
  src.setVertexProperties("nx", std::vector<double>{1.0, 2.0, 3.0, 4.0});

  TestPLY copyCtor(src);
  REQUIRE(copyCtor.getID() == src.getID());
  REQUIRE(copyCtor.getVertexCoordinates() == src.getVertexCoordinates());
  REQUIRE(copyCtor.getVertexProperties("nx") == src.getVertexProperties("nx"));

  // Mutating the copy must not affect the original.
  copyCtor.getVertexCoordinates()[0] = Vec3T<double>(999, 999, 999);
  REQUIRE(src.getVertexCoordinates()[0] != Vec3T<double>(999, 999, 999));

  TestPLY copyAssign;
  copyAssign = src;
  REQUIRE(copyAssign.getVertexCoordinates() == src.getVertexCoordinates());

  copyAssign.getFacets().clear();
  REQUIRE_FALSE(src.getFacets().empty());
}

TEST_CASE("PLY: move construction and move assignment transfer data", "[PLY]")
{
  auto       moveCtorSrc        = buildTetrahedronSoup();
  const auto expectedVertexSize = moveCtorSrc.getVertexCoordinates().size();

  TestPLY moved(std::move(moveCtorSrc));
  REQUIRE(moved.getID() == "tetrahedron-soup");
  REQUIRE(moved.getVertexCoordinates().size() == expectedVertexSize);

  auto    moveAssignSrc = buildTetrahedronSoup();
  TestPLY moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getID() == "tetrahedron-soup");
  REQUIRE(moveAssignDst.getVertexCoordinates().size() == expectedVertexSize);
}

TEST_CASE("PLY: convertToDCEL builds a mesh with the correct compressed vertex and face counts", "[PLY]")
{
  auto ply = buildTetrahedronSoup();

  // The raw soup has one vertex entry per facet corner (no compression yet).
  REQUIRE(ply.getVertexCoordinates().size() == 12);
  REQUIRE(ply.getFacets().size() == 4);

  auto mesh = ply.convertToDCEL<DCEL::DefaultMetaData>();

  REQUIRE(mesh != nullptr);
  REQUIRE(mesh->getVertices().size() == 4); // Compressed down to the 4 unique corners.
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getEdges().size() == 12); // 4 triangular faces * 3 half-edges each.

  for (const auto& e : mesh->getEdges()) {
    REQUIRE(e->getPairEdge() != nullptr); // Watertight: every half-edge has a pair.
  }
}
