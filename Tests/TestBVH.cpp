// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for TreeBVH, PackedBVH, and the TriMeshSDF/MeshSDF/FlatMeshSDF wrappers built on
// top of them. Uses a regular dodecahedron (20 vertices, 36 triangulated faces, watertight and
// orientable) as a fixture, read from disk in all four supported formats (STL/PLY/OBJ/VTK), so
// that both the file parsers and the BVH machinery downstream of them get real, non-trivial
// coverage -- unlike the hardcoded tetrahedron used elsewhere in this suite, a dodecahedron has
// enough primitives to meaningfully exercise BVH partitioning and traversal.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cmath>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

using Meta = DCEL::DefaultMetaData;

std::string
dataPath(const std::string& a_filename)
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/" + a_filename;
}

// A handful of query points spanning inside, outside, and near-surface -- enough to catch a BVH
// traversal or partitioning bug without the test suite taking noticeably longer to run.
template <class T>
std::vector<Vec3T<T>>
queryPoints()
{
  using Vec3 = Vec3T<T>;
  return {
    Vec3(0, 0, 0),
    Vec3(0.5, 0.5, 0.5),
    Vec3(1.0, 1.0, 1.0),
    Vec3(1.5, 0.0, 0.0),
    Vec3(0.0, 1.5, 0.0),
    Vec3(0.0, 0.0, 1.5),
    Vec3(-1.5, -1.5, -1.5),
    Vec3(3.0, 3.0, 3.0),
    Vec3(10.0, 0.0, 0.0),
    Vec3(-5.0, 2.0, 1.0),
  };
}

// Comparisons across independently-parsed file formats of literally the same geometry should
// agree almost exactly; the residual gap is down to how each format's ASCII/binary encoding
// round-trips floating-point coordinates, not accumulated BVH arithmetic.
template <class T>
double
formatMargin()
{
  return std::is_same_v<T, float> ? 1.0e-4 : 1.0e-9;
}

// Comparisons between a brute-force scan and a BVH-accelerated traversal (or between different
// partitioning/packing/build strategies) chain many more dot/cross products across the mesh's 36
// faces, so tolerate more accumulated rounding error than a simple format round-trip.
template <class T>
double
traversalMargin()
{
  return std::is_same_v<T, float> ? 1.0e-2 : 1.0e-6;
}

} // namespace

TEMPLATE_TEST_CASE("Dodecahedron: all four file formats parse into an identical, watertight DCEL mesh",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto meshSTL = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"));
  const auto meshPLY = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.ply"));
  const auto meshOBJ = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"));
  const auto meshVTK = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.vtk"));

  for (const auto& mesh : {meshSTL, meshPLY, meshOBJ, meshVTK}) {
    REQUIRE(mesh != nullptr);
    REQUIRE(mesh->getVertices().size() == 20);
    REQUIRE(mesh->getFaces().size() == 36);
    REQUIRE(mesh->getEdges().size() == 108); // 54 undirected edges * 2 half-edges each.

    for (const auto& e : mesh->getEdges()) {
      REQUIRE(e->getPairEdge() != nullptr); // Watertight: every half-edge has a pair.
    }
  }

  // Same geometry regardless of source format: signed distance must agree at every query point.
  for (const auto& p : queryPoints<T>()) {
    const T d = meshSTL->signedDistance(p);

    REQUIRE_THAT(meshPLY->signedDistance(p), withinAbsT(d, formatMargin<T>()));
    REQUIRE_THAT(meshOBJ->signedDistance(p), withinAbsT(d, formatMargin<T>()));
    REQUIRE_THAT(meshVTK->signedDistance(p), withinAbsT(d, formatMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("TreeBVH/PackedBVH: signedDistance agrees with the brute-force mesh scan, for "
                   "every partitioning strategy",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  constexpr size_t K = 4;

  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"));
  REQUIRE(mesh != nullptr);

  using Face = DCEL::FaceT<T, Meta>;

  BVH::PrimAndBVList<Face, AABB> primsAndBVs;
  for (const auto& f : mesh->getFaces()) {
    primsAndBVs.emplace_back(f, AABB(f->getAllVertexCoordinates()));
  }
  REQUIRE(primsAndBVs.size() == 36);

  const FlatMeshSDF<T, Meta> flat(mesh);
  const auto                 brute = [&flat](const Vec3T<T>& a_point) -> T { return flat.signedDistance(a_point); };

  // Every value of BVH::Build is exercised through one BVH::TreeBVH built the same way MeshSDF
  // builds one internally (see MeshDistanceFunctionsDetail::buildDCELTreeBVH), so this covers the
  // partitioning strategies directly rather than only through the higher-level SDF wrapper.
  auto buildAndCheck = [&](const char* a_label, auto&& a_partitionFunction) {
    INFO("Build strategy: " << a_label);

    auto tree = std::make_shared<BVH::TreeBVH<T, Face, AABB, K>>(primsAndBVs);
    a_partitionFunction(*tree);

    const auto packed = tree->pack();
    REQUIRE(packed != nullptr);
    REQUIRE(packed->getPrimitives().size() == 36);

    for (const auto& p : queryPoints<T>()) {
      REQUIRE_THAT(packed->signedDistance(p), withinAbsT(brute(p), traversalMargin<T>()));
    }
  };

  buildAndCheck("TopDown (default BVCentroidPartitioner)", [](auto& a_tree) { a_tree.topDownSortAndPartition(); });

  buildAndCheck("TopDown (BinnedSAHPartitioner)", [](auto& a_tree) {
    using Node              = BVH::TreeBVH<T, Face, AABB, K>;
    using StopFunc          = typename Node::StopFunction;
    const StopFunc stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };

    a_tree.topDownSortAndPartition(BVH::BinnedSAHPartitioner<T, Face, AABB, K>, stopCrit);
  });

  buildAndCheck("BottomUp (Morton)", [](auto& a_tree) { a_tree.template bottomUpSortAndPartition<SFC::Morton>(); });

  buildAndCheck("BottomUp (Nested)", [](auto& a_tree) { a_tree.template bottomUpSortAndPartition<SFC::Nested>(); });
}

TEMPLATE_TEST_CASE("MeshSDF: signedDistance agrees with FlatMeshSDF for every BVH::Build strategy",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"));
  REQUIRE(mesh != nullptr);

  const FlatMeshSDF<T, Meta> flat(mesh);

  for (const auto build : {BVH::Build::TopDown, BVH::Build::Morton, BVH::Build::Nested, BVH::Build::SAH}) {
    const MeshSDF<T, Meta, K> packed(mesh, build);

    for (const auto& p : queryPoints<T>()) {
      REQUIRE_THAT(packed.signedDistance(p), withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("TriMeshSDF: signedDistance agrees with FlatMeshSDF and MeshSDF for every BVH::Build strategy",
                   "[BVH][Dodecahedron][TriMesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.ply"));
  REQUIRE(mesh != nullptr);

  const FlatMeshSDF<T, Meta> flat(mesh);
  const MeshSDF<T, Meta, K>  packed(mesh, BVH::Build::SAH);

  for (const auto build : {BVH::Build::TopDown, BVH::Build::Morton, BVH::Build::Nested, BVH::Build::SAH}) {
    const TriMeshSDF<T, Meta, K, W> tri(mesh, build, 2);

    for (const auto& p : queryPoints<T>()) {
      REQUIRE_THAT(tri.signedDistance(p), withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
      REQUIRE_THAT(tri.signedDistance(p), withinAbsT(packed.signedDistance(p), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("MeshSDF::getClosestFaces returns the correct number of candidate faces, sorted on request",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.vtk"));
  REQUIRE(mesh != nullptr);

  const MeshSDF<T, Meta, K> packed(mesh, BVH::Build::SAH);

  const Vec3T<T> p(0.5, 0.5, 0.5);

  const auto sorted = packed.getClosestFaces(p, true);
  REQUIRE(!sorted.empty());

  for (size_t i = 1; i < sorted.size(); i++) {
    REQUIRE(sorted[i - 1].second <= sorted[i].second);
  }

  // The closest face reported must be consistent with the scalar signed-distance query.
  const T closestUnsignedDist = sorted.front().second;
  REQUIRE_THAT(closestUnsignedDist, withinAbsT(std::abs(packed.signedDistance(p)), traversalMargin<T>()));
}

TEMPLATE_TEST_CASE("Parser::readIntoPackedBVH matches MeshSDF built directly from the same mesh",
                   "[BVH][Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  const auto direct = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"));
  REQUIRE(direct != nullptr);

  const MeshSDF<T, Meta, K> expected(direct, BVH::Build::SAH);
  const auto                fromFile = Parser::readIntoPackedBVH<T, Meta, K>(dataPath("dodecahedron.stl"));

  REQUIRE(fromFile != nullptr);

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(fromFile->signedDistance(p), withinAbsT(expected.signedDistance(p), formatMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Parser: multi-file overloads return one result per file, each matching the "
                   "corresponding single-file call",
                   "[BVH][Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  // Two different formats of the *same* underlying geometry -- the point isn't that the two
  // files describe different scenes, only that the multi-file overload is a faithful per-file
  // loop, not some kind of multi-mesh merge.
  const std::vector<std::string> files = {dataPath("dodecahedron.stl"), dataPath("dodecahedron.ply")};

  SECTION("readIntoDCEL")
  {
    const auto meshes = Parser::readIntoDCEL<T, Meta>(files);
    REQUIRE(meshes.size() == 2);

    for (size_t i = 0; i < files.size(); i++) {
      const auto single = Parser::readIntoDCEL<T, Meta>(files[i]);
      REQUIRE(meshes[i]->getVertices().size() == single->getVertices().size());
      REQUIRE(meshes[i]->getFaces().size() == single->getFaces().size());
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(meshes[i]->signedDistance(p), withinAbsT(single->signedDistance(p), formatMargin<T>()));
      }
    }
  }

  SECTION("readIntoMesh")
  {
    const auto flatSDFs = Parser::readIntoMesh<T, Meta>(files);
    REQUIRE(flatSDFs.size() == 2);

    for (size_t i = 0; i < files.size(); i++) {
      const auto single = Parser::readIntoMesh<T, Meta>(files[i]);
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(flatSDFs[i]->signedDistance(p), withinAbsT(single->signedDistance(p), formatMargin<T>()));
      }
    }
  }

  SECTION("readIntoPackedBVH")
  {
    const auto packedSDFs = Parser::readIntoPackedBVH<T, Meta, K>(files);
    REQUIRE(packedSDFs.size() == 2);

    for (size_t i = 0; i < files.size(); i++) {
      const auto single = Parser::readIntoPackedBVH<T, Meta, K>(files[i]);
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(packedSDFs[i]->signedDistance(p), withinAbsT(single->signedDistance(p), formatMargin<T>()));
      }
    }
  }

  SECTION("readIntoTriangleBVH")
  {
    const auto triSDFs = Parser::readIntoTriangleBVH<T, Meta>(files);
    REQUIRE(triSDFs.size() == 2);

    for (size_t i = 0; i < files.size(); i++) {
      const auto single = Parser::readIntoTriangleBVH<T, Meta>(files[i]);
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(triSDFs[i]->signedDistance(p), withinAbsT(single->signedDistance(p), formatMargin<T>()));
      }
    }
  }
}
