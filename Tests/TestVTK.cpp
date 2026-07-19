// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <array>
#include <stdexcept>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

template <class T>
using Vtk = VTK<T>;

// Build a raw (uncompressed) tetrahedron soup directly into a VTK<T> object, mimicking how a real
// VTK polydata file stores each facet's vertex indices independently (i.e. with duplicates).
template <class T>
static Vtk<T>
buildTetrahedronSoup()
{
  Vtk<T> vtk("tetrahedron-soup");

  auto& vertices = vtk.getVertexCoordinates();
  auto& facets   = vtk.getFacets();

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

  return vtk;
}

TEMPLATE_TEST_CASE("VTK: default construction is empty", "[VTK]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Vtk<T> vtk;

  REQUIRE(vtk.getID().empty());
  REQUIRE(vtk.getVertexCoordinates().empty());
  REQUIRE(vtk.getFacets().empty());
}

TEMPLATE_TEST_CASE("VTK: id constructor sets the identifier only", "[VTK]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Vtk<T> vtk("my-id");

  REQUIRE(vtk.getID() == "my-id");
  REQUIRE(vtk.getVertexCoordinates().empty());
  REQUIRE(vtk.getFacets().empty());
}

TEMPLATE_TEST_CASE("VTK: setPointDataScalars/setCellDataScalars store data retrievable by name",
                   "[VTK]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Vtk<T> vtk;

  vtk.setPointDataScalars("temperature", std::vector<T>{1.0, 2.0, 3.0});
  vtk.setCellDataScalars("pressure", std::vector<T>{4.0, 5.0});

  REQUIRE(vtk.getPointDataScalars("temperature") == std::vector<T>{1.0, 2.0, 3.0});
  REQUIRE(vtk.getCellDataScalars("pressure") == std::vector<T>{4.0, 5.0});
}

TEMPLATE_TEST_CASE("VTK: getPointDataScalars/getCellDataScalars throw on an unknown array name",
                   "[VTK]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Vtk<T> vtk;
  vtk.setPointDataScalars("temperature", std::vector<T>{1.0});

  REQUIRE_THROWS_AS(vtk.getPointDataScalars("does-not-exist"), std::out_of_range);
  REQUIRE_THROWS_AS(vtk.getCellDataScalars("does-not-exist"), std::out_of_range);
}

TEMPLATE_TEST_CASE("VTK: copy construction and copy assignment duplicate data independently",
                   "[VTK]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto src = buildTetrahedronSoup<T>();
  src.setPointDataScalars("temperature", std::vector<T>{1.0, 2.0, 3.0, 4.0});

  Vtk<T> copyCtor(src);
  REQUIRE(copyCtor.getID() == src.getID());
  REQUIRE(copyCtor.getVertexCoordinates() == src.getVertexCoordinates());
  REQUIRE(copyCtor.getPointDataScalars("temperature") == src.getPointDataScalars("temperature"));

  // Mutating the copy must not affect the original.
  copyCtor.getVertexCoordinates()[0] = Vec3T<T>(999, 999, 999);
  REQUIRE(src.getVertexCoordinates()[0] != Vec3T<T>(999, 999, 999));

  Vtk<T> copyAssign;
  copyAssign = src;
  REQUIRE(copyAssign.getVertexCoordinates() == src.getVertexCoordinates());

  copyAssign.getFacets().clear();
  REQUIRE_FALSE(src.getFacets().empty());
}

TEMPLATE_TEST_CASE("VTK: move construction and move assignment transfer data", "[VTK]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto       moveCtorSrc        = buildTetrahedronSoup<T>();
  const auto expectedVertexSize = moveCtorSrc.getVertexCoordinates().size();

  Vtk<T> moved(std::move(moveCtorSrc));
  REQUIRE(moved.getID() == "tetrahedron-soup");
  REQUIRE(moved.getVertexCoordinates().size() == expectedVertexSize);

  auto   moveAssignSrc = buildTetrahedronSoup<T>();
  Vtk<T> moveAssignDst;
  moveAssignDst = std::move(moveAssignSrc);
  REQUIRE(moveAssignDst.getID() == "tetrahedron-soup");
  REQUIRE(moveAssignDst.getVertexCoordinates().size() == expectedVertexSize);
}

TEMPLATE_TEST_CASE("VTK: convertToDCEL builds a mesh with the correct compressed vertex and face counts",
                   "[VTK]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto vtk = buildTetrahedronSoup<T>();

  // The raw soup has one vertex entry per facet corner (no compression yet).
  REQUIRE(vtk.getVertexCoordinates().size() == 12);
  REQUIRE(vtk.getFacets().size() == 4);

  auto mesh = vtk.template convertToDCEL<DCEL::DefaultMetaData>();

  REQUIRE(mesh != nullptr);
  REQUIRE(mesh->getVertices().size() == 4); // Compressed down to the 4 unique corners.
  REQUIRE(mesh->getFaces().size() == 4);
  REQUIRE(mesh->getEdges().size() == 12); // 4 triangular faces * 3 half-edges each.

  for (const auto& e : mesh->getEdges()) {
    REQUIRE(e.getPairEdge() != DCEL::InvalidIndex); // Watertight: every half-edge has a pair.
  }
}
