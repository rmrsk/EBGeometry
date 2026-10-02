// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for TreeBVH, PackedBVH, and the TriMeshSDF/MeshSDF/FlatMeshSDF wrappers built on
// top of them. Uses a regular dodecahedron (20 vertices, 36 triangulated faces, watertight and
// orientable) as a fixture, read from disk in all four supported formats (STL/PLY/OBJ/VTK), so
// that both the file parsers and the BVH machinery downstream of them get real, non-trivial
// coverage -- unlike the hardcoded tetrahedron used elsewhere in this suite, a dodecahedron has
// enough primitives to meaningfully exercise BVH partitioning and traversal.

#include "EBGeometry.hpp"
#include "TestConstructions.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinRel;

namespace {

using Meta = DCEL::DefaultMetaData;

std::string
dataPath(const std::string& a_filename)
{
  return std::string(EBGEOMETRY_TEST_DATA_DIR) + "/" + a_filename;
}

// a_count + 1 evenly spaced values from a_lo to a_hi inclusive, for sweeping a query grid.
template <class T>
std::vector<T>
sweepValues(const T a_lo, const T a_hi, const int a_count)
{
  std::vector<T> values;
  values.reserve(static_cast<size_t>(a_count) + 1);

  for (int i = 0; i <= a_count; i++) {
    values.push_back(a_lo + (a_hi - a_lo) * T(i) / T(a_count));
  }

  return values;
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

// Nearest unsigned squared distance from each query point to a dodecahedron's faces, found through
// a PackedBVH with branching factor K. Used to pin PackedBVH::pruneTraverse()'s per-ISA
// child-distance paths against its scalar path: which of the two runs is chosen at compile time
// from (T, K) and the compiled ISA, so varying K over a build's SIMD and non-SIMD widths runs both
// within one binary.
//
// A pure minimum over the primitive set is deliberate. Its value cannot depend on the order leaves
// are visited in, so a disagreement between two K values is a real disagreement about the computed
// child distances (or about the pruning they drive), never a tie-break artefact of one traversal
// order versus another.
template <class T, size_t K>
std::vector<T>
nearestDist2PerQueryPoint(const DCEL::MeshT<T, Meta>&                                                a_mesh,
                          const BVH::PrimAndBVList<DCEL::FaceT<T, Meta>, BoundingVolumes::AABBT<T>>& a_primsAndBVs)
{
  using Face = DCEL::FaceT<T, Meta>;
  using AABB = BoundingVolumes::AABBT<T>;

  Pool pool(hostMemoryResource());

  auto tree = std::make_shared<BVH::TreeBVH<T, Face, AABB, K>>(a_primsAndBVs);
  tree->topDownSortAndPartition();

  const auto  packed = tree->pack(pool);
  const auto& prims  = packed->getPrimitives();

  std::vector<T> result;

  for (const auto& p : queryPoints<T>()) {
    T state = std::numeric_limits<T>::max();

    const auto evalLeaf = [&prims, &p, &a_mesh](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = prims[a_offset + i].unsignedDistance2(p, a_mesh);

        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    packed->pruneTraverse(p, state, evalLeaf, pruneDist2);

    result.push_back(state);
  }

  return result;
}

} // namespace

TEMPLATE_TEST_CASE("Dodecahedron: all four file formats parse into an identical, watertight DCEL mesh",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const auto meshSTL = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"), pool);
  const auto meshPLY = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.ply"), pool);
  const auto meshOBJ = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);
  const auto meshVTK = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.vtk"), pool);

  // Every mesh below is queried directly through its own accessors; each attaches to pool on its
  // first reserve inside readIntoDCEL, so nothing has to be frozen or bound first.
  for (const auto& mesh : {meshSTL, meshPLY, meshOBJ, meshVTK}) {
    REQUIRE(mesh.numVertices() == 20);
    REQUIRE(mesh.numFaces() == 36);
    REQUIRE(mesh.numEdges() == 108); // 54 undirected edges * 2 half-edges each.

    for (uint32_t i = 0; i < mesh.numEdges(); i++) {
      REQUIRE(mesh.getEdge(i).getPairEdgeIndex() != UINT32_MAX); // Watertight: every half-edge has a pair.
    }
  }

  // Same geometry regardless of source format: signed distance must agree at every query point.
  for (const auto& p : queryPoints<T>()) {
    const T d = meshSTL.signedDistance(p);

    REQUIRE_THAT(meshPLY.signedDistance(p), withinAbsT(d, formatMargin<T>()));
    REQUIRE_THAT(meshOBJ.signedDistance(p), withinAbsT(d, formatMargin<T>()));
    REQUIRE_THAT(meshVTK.signedDistance(p), withinAbsT(d, formatMargin<T>()));
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

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

  using Face = DCEL::FaceT<T, Meta>;

  BVH::PrimAndBVList<Face, AABB> primsAndBVs;
  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const auto& f = mesh.getFace(i);
    primsAndBVs.emplace_back(std::make_shared<const Face>(f), AABB(f.getAllVertexCoordinates(mesh)));
  }
  REQUIRE(primsAndBVs.size() == 36);

  const FlatMeshSDF<T, Meta> flat(mesh, pool);
  const auto                 brute = [&flat](const Vec3T<T>& a_point) -> T { return flat.signedDistance(a_point); };

  // Every value of BVH::Construction is exercised through one BVH::TreeBVH built the same way MeshSDF
  // builds one internally (see MeshDistanceFunctionsDetail::buildDCELTreeBVH), so this covers the
  // partitioning strategies directly rather than only through the higher-level SDF wrapper.
  auto buildAndCheck = [&](const char* a_label, auto&& a_partitionFunction) {
    INFO("Build strategy: " << a_label);

    auto tree = std::make_shared<BVH::TreeBVH<T, Face, AABB, K>>(primsAndBVs);
    a_partitionFunction(*tree);

    const auto packed = tree->pack(pool);
    REQUIRE(packed != nullptr);
    REQUIRE(packed->getPrimitives().size() == 36);

    // PackedBVH has no signedDistance() of its own -- callers build their own thin wrapper
    // around pruneTraverse(), exactly as MeshSDF/TriMeshSDF::signedDistance() do.
    const auto& faces = packed->getPrimitives();

    for (const auto& p : queryPoints<T>()) {
      T state = std::numeric_limits<T>::max();

      const auto evalLeaf = [&faces, &p, &mesh](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d = faces[a_offset + i].signedDistance(p, mesh);
          if (std::abs(d) < std::abs(a_state)) {
            a_state = d;
          }
        }
      };

      const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

      packed->pruneTraverse(p, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(brute(p), traversalMargin<T>()));
    }
  };

  buildAndCheck("TopDown (default BVCentroidPartitioner)", [](auto& a_tree) { a_tree.topDownSortAndPartition(); });

  buildAndCheck("TopDown (BinnedSAHPartitioner)", [](auto& a_tree) {
    using Node              = BVH::TreeBVH<T, Face, AABB, K>;
    using LeafPred          = typename Node::LeafPredicate;
    const LeafPred stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };

    a_tree.topDownSortAndPartition(BVH::BinnedSAHPartitioner<T, Face, AABB, K>, stopCrit);
  });

  buildAndCheck("TopDown (BinnedSAHPartitioner, longest-axis)", [](auto& a_tree) {
    using Node              = BVH::TreeBVH<T, Face, AABB, K>;
    using LeafPred          = typename Node::LeafPredicate;
    const LeafPred stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };

    a_tree.topDownSortAndPartition(BVH::BinnedSAHPartitioner<T, Face, AABB, K, true>, stopCrit);
  });

  buildAndCheck("BottomUp (Morton)", [](auto& a_tree) { a_tree.template bottomUpSortAndPartition<SFC::Morton>(); });

  buildAndCheck("BottomUp (Nested)", [](auto& a_tree) { a_tree.template bottomUpSortAndPartition<SFC::Nested>(); });
}

TEMPLATE_TEST_CASE("PackedBVH::pruneTraverse: every compiled SIMD child-distance path agrees "
                   "bit-for-bit with the scalar path",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Face = DCEL::FaceT<T, Meta>;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

  BVH::PrimAndBVList<Face, AABB> primsAndBVs;

  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const auto& f = mesh.getFace(i);

    primsAndBVs.emplace_back(std::make_shared<const Face>(f), AABB(f.getAllVertexCoordinates(mesh)));
  }

  REQUIRE(primsAndBVs.size() == 36);

  // pruneTraverse() dispatches its per-child box-distance computation on (T, K) and the compiled
  // ISA: AVX-512F claims (double, K=8) and (float, K=16), AVX claims (double, K=4), (float, K=8)
  // and (double, K=8), SSE4.1 claims (float, K=4), and everything else runs the scalar loop. No K
  // value has a SIMD path in every build, and K = 3, 5, 6, 7 never have one in any build -- so
  // sweeping K covers the scalar path and whichever vector paths this binary compiled, and requires
  // them to agree.
  //
  // Equality is exact, not within a margin. Each path computes the same quantity in the same
  // association order, so "close enough" would hide precisely the kind of drift this pins down.
  const std::vector<T> reference = nearestDist2PerQueryPoint<T, 3>(mesh, primsAndBVs);

  REQUIRE(reference.size() == queryPoints<T>().size());

  const auto requireAgreesWithReference = [&reference](const char* a_label, const std::vector<T>& a_values) {
    INFO("Branching factor: " << a_label);

    REQUIRE(a_values.size() == reference.size());

    for (size_t i = 0; i < reference.size(); i++) {
      INFO("Query point index: " << i);

      REQUIRE(a_values[i] == reference[i]);
    }
  };

  requireAgreesWithReference("K=2", nearestDist2PerQueryPoint<T, 2>(mesh, primsAndBVs));
  requireAgreesWithReference("K=4", nearestDist2PerQueryPoint<T, 4>(mesh, primsAndBVs));
  requireAgreesWithReference("K=5", nearestDist2PerQueryPoint<T, 5>(mesh, primsAndBVs));
  requireAgreesWithReference("K=6", nearestDist2PerQueryPoint<T, 6>(mesh, primsAndBVs));
  requireAgreesWithReference("K=7", nearestDist2PerQueryPoint<T, 7>(mesh, primsAndBVs));
  requireAgreesWithReference("K=8", nearestDist2PerQueryPoint<T, 8>(mesh, primsAndBVs));
  requireAgreesWithReference("K=16", nearestDist2PerQueryPoint<T, 16>(mesh, primsAndBVs));
}

TEMPLATE_TEST_CASE("MeshSDF: signedDistance agrees with FlatMeshSDF for every BVH::Construction strategy",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"), pool);

  const FlatMeshSDF<T, Meta> flat(mesh, pool);

  for (const auto build : allConstructions) {
    const MeshSDF<T, Meta, K> packed(mesh, pool, build);

    for (const auto& p : queryPoints<T>()) {
      REQUIRE_THAT(packed.signedDistance(p), withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE(
  "TriMeshSDF: signedDistance agrees with FlatMeshSDF and MeshSDF for every BVH::Construction strategy",
  "[BVH][Dodecahedron][TriMesh]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.ply"), pool);

  const FlatMeshSDF<T, Meta> flat(mesh, pool);
  const MeshSDF<T, Meta, K>  packed(mesh, pool, BVH::Construction::SAH);

  for (const auto build : allConstructions) {
    const TriMeshSDF<T, Meta, K, W> tri(mesh, pool, build, 2);

    for (const auto& p : queryPoints<T>()) {
      REQUIRE_THAT(tri.signedDistance(p), withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
      REQUIRE_THAT(tri.signedDistance(p), withinAbsT(packed.signedDistance(p), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Mesh SDFs: a zero-area sliver face leaves the signed distance unchanged",
                   "[BVH][Tetrahedron][Degenerate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // tetrahedron_sliver.stl is tetrahedron.stl with its bottom face split at the hypotenuse midpoint
  // and the resulting T-junction closed by a zero-area triangle lying along the hypotenuse -- the
  // kind of filler CAD exporters write. The sliver has no normal of its own, and it sits exactly on
  // the sharp edge between the bottom and the slanted face, so every pseudonormal near that edge
  // depends on how it is handled.
  Pool       pool(hostMemoryResource());
  const auto clean  = Parser::readIntoDCEL<T, Meta>(dataPath("tetrahedron.stl"), pool);
  const auto sliver = Parser::readIntoDCEL<T, Meta>(dataPath("tetrahedron_sliver.stl"), pool);

  for (uint32_t f = 0; f < sliver.numFaces(); f++) {
    const Vec3T<T>& n = sliver.getFace(f).getNormal();

    REQUIRE(std::isfinite(n[0]));
    REQUIRE(std::isfinite(n[1]));
    REQUIRE(std::isfinite(n[2]));
    REQUIRE_THAT(n.length(), withinAbsT(T(1), looseMargin<T>()));
  }

  const FlatMeshSDF<T, Meta>      reference(clean, pool);
  const FlatMeshSDF<T, Meta>      flat(sliver, pool);
  const MeshSDF<T, Meta, K>       packed(sliver, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, Meta, K, W> tri(sliver, pool, BVH::Construction::SAH, 2);

  for (const T x : sweepValues<T>(T(-0.5), T(1.0), 16)) {
    for (const T y : sweepValues<T>(T(-0.5), T(1.0), 16)) {
      for (const T z : sweepValues<T>(T(-0.5), T(1.0), 16)) {
        const Vec3T<T> p(x, y, z);
        const T        expected = reference.signedDistance(p);

        INFO("p = " << p);
        REQUIRE_THAT(flat.signedDistance(p), withinAbsT(expected, looseMargin<T>()));
        REQUIRE_THAT(packed.signedDistance(p), withinAbsT(expected, looseMargin<T>()));
        REQUIRE_THAT(tri.signedDistance(p), withinAbsT(expected, looseMargin<T>()));
      }
    }
  }
}

TEMPLATE_TEST_CASE("Mesh SDFs: signs around concave edges and vertices match an independent inside test",
                   "[BVH][Concave]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // lblock.stl is the L-shaped prism ([0,2]x[0,1] U [0,1]x[0,2]) x [0,1]. Its edge along x = y = 1 is
  // concave, which the convex fixtures never exercise. FlatMeshSDF shares the pseudonormal code with the
  // BVH-accelerated SDFs, so the reference here is analytic: inside is the union of two boxes, and
  // outside, the distance is the smaller of the two boxes' distances.
  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("lblock.stl"), pool);

  REQUIRE(mesh.numFaces() == 20);

  const BoxSDF<T> boxA(Vec3(T(0), T(0), T(0)), Vec3(T(2), T(1), T(1)));
  const BoxSDF<T> boxB(Vec3(T(0), T(0), T(0)), Vec3(T(1), T(2), T(1)));

  const auto inside = [](const Vec3& p) {
    const bool inZ = p[2] > T(0) && p[2] < T(1);
    const bool inA = p[0] > T(0) && p[0] < T(2) && p[1] > T(0) && p[1] < T(1);
    const bool inB = p[0] > T(0) && p[0] < T(1) && p[1] > T(0) && p[1] < T(2);

    return inZ && (inA || inB);
  };

  const FlatMeshSDF<T, Meta>      flat(mesh, pool);
  const MeshSDF<T, Meta, K>       packed(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, Meta, K, W> tri(mesh, pool, BVH::Construction::SAH, 2);

  // A grid centred on the concave edge's end point (1, 1, 1), where the concave edge, the concave
  // vertex and the faces around them all compete for the closest feature. The grid spacing avoids
  // landing exactly on a face.
  for (const T x : sweepValues<T>(T(0.3), T(1.7), 14)) {
    for (const T y : sweepValues<T>(T(0.3), T(1.7), 14)) {
      for (const T z : sweepValues<T>(T(0.45), T(1.55), 11)) {
        const Vec3 p(x + T(0.013), y + T(0.007), z + T(0.011));

        const bool in       = inside(p);
        const T    outside  = std::min(boxA.signedDistance(p), boxB.signedDistance(p));
        const T    distance = in ? T(-1) : outside;

        INFO("p = " << p);

        for (const T value : {flat.signedDistance(p), packed.signedDistance(p), tri.signedDistance(p)}) {
          REQUIRE((value < T(0)) == in);

          if (!in) {
            REQUIRE_THAT(value, withinAbsT(distance, looseMargin<T>()));
          }
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("Mesh SDFs: signs behind a sharp concave edge match an independent inside test",
                   "[BVH][Concave]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec2 = std::array<T, 2>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // notch.stl is a prism over the polygon below, z in [0,1]: a box with a narrow V-shaped notch cut
  // into its top side. The notch's walls meet at the tip (1, 0.4) in a concave edge along z. Behind
  // that edge, inside the solid, the edge is the closest feature, and only the sum of both walls'
  // normals gives the right sign there: either wall's normal alone points outward for part of that
  // region, because the notch is much narrower than 90 degrees.
  const std::array<Vec2, 7> polygon{
    {{T(0), T(0)}, {T(2), T(0)}, {T(2), T(1)}, {T(1.2), T(1)}, {T(1), T(0.4)}, {T(0.8), T(1)}, {T(0), T(1)}}};

  const auto inside = [&polygon](const Vec3& p) {
    if (!(p[2] > T(0) && p[2] < T(1))) {
      return false;
    }

    // Crossing-number point-in-polygon test.
    bool in = false;

    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
      const Vec2& a = polygon[i];
      const Vec2& b = polygon[j];

      if ((a[1] > p[1]) != (b[1] > p[1]) && p[0] < (b[0] - a[0]) * (p[1] - a[1]) / (b[1] - a[1]) + a[0]) {
        in = !in;
      }
    }

    return in;
  };

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("notch.stl"), pool);

  REQUIRE(mesh.numFaces() == 24);

  const FlatMeshSDF<T, Meta>      flat(mesh, pool);
  const MeshSDF<T, Meta, K>       packed(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, Meta, K, W> tri(mesh, pool, BVH::Construction::SAH, 2);

  // A grid around the notch's tip, including the region just behind it. The offsets keep the points
  // off the faces.
  for (const T x : sweepValues<T>(T(0.6), T(1.4), 17)) {
    for (const T y : sweepValues<T>(T(-0.1), T(1.1), 25)) {
      for (const T z : sweepValues<T>(T(-0.2), T(1.2), 8)) {
        const Vec3 p(x + T(0.0013), y + T(0.0007), z + T(0.011));

        const bool in = inside(p);

        INFO("p = " << p);

        for (const T value : {flat.signedDistance(p), packed.signedDistance(p), tri.signedDistance(p)}) {
          REQUIRE((value < T(0)) == in);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("TriMeshSDF: polygon faces are fan-triangulated, not truncated to their first three vertices",
                   "[BVH][Degenerate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool       pool(hostMemoryResource());
  const auto cube = Parser::readIntoDCEL<T, Meta>(dataPath("cube_quads.obj"), pool);

  REQUIRE(cube.numFaces() == 6);

  const BoxSDF<T>                 box(Vec3T<T>::zeros(), Vec3T<T>::ones());
  const TriMeshSDF<T, Meta, K, W> tri(cube, pool, BVH::Construction::SAH, 2);
  const auto                      triangles = Parser::readIntoTriangles<T, Meta>(dataPath("cube_quads.obj"));

  REQUIRE(triangles.size() == 12);

  for (const T x : sweepValues<T>(T(-0.5), T(1.5), 12)) {
    for (const T y : sweepValues<T>(T(-0.5), T(1.5), 12)) {
      for (const T z : sweepValues<T>(T(-0.5), T(1.5), 12)) {
        const Vec3T<T> p(x, y, z);

        INFO("p = " << p);
        REQUIRE_THAT(tri.signedDistance(p), withinAbsT(box.signedDistance(p), looseMargin<T>()));
      }
    }
  }
}

TEMPLATE_TEST_CASE("MeshSDF::getClosestFaces returns the correct number of candidate faces, sorted on request",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.vtk"), pool);

  const MeshSDF<T, Meta, K> packed(mesh, pool, BVH::Construction::SAH);

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

TEMPLATE_TEST_CASE("TriMeshSDF::getClosestTriangle reports the closest triangle's metadata and a "
                   "distance matching signedDistance()",
                   "[BVH][TriMesh][Meta]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool pool(hostMemoryResource());

  // A soup of well-separated triangles (3 apart along x), each tagged with a distinct metadata value
  // so the closest-triangle query's returned metadata is unambiguous. Enough triangles to force
  // several BVH leaves/levels.
  constexpr int N = 12;

  std::vector<Triangle<T, Meta>> tris;
  for (int i = 0; i < N; i++) {
    const Vec3 base(T(3 * i), T(0), T(0));

    Triangle<T, Meta> tri;
    tri.setVertexPositions({base + Vec3(0, 0, 0), base + Vec3(1, 0, 0), base + Vec3(0, 1, 0)});
    tri.setNormal(Vec3(0, 0, 1));
    tri.setVertexNormals({Vec3(0, 0, 1), Vec3(0, 0, 1), Vec3(0, 0, 1)});
    tri.setEdgeNormals({Vec3(0, 0, 1), Vec3(0, 0, 1), Vec3(0, 0, 1)});
    tri.setMetaData(static_cast<Meta>(100 + i));

    tris.emplace_back(tri);
  }

  for (const auto build : allConstructions) {
    const TriMeshSDF<T, Meta, K, W> tri(tris, pool, build, 2);

    for (int i = 0; i < N; i++) {
      const Vec3 q(T(3 * i) + T(0.25), T(0.25), T(0.2)); // unambiguously nearest to triangle i

      const auto closest = tri.getClosestTriangle(q);

      REQUIRE(closest.metaData == static_cast<Meta>(100 + i));
      REQUIRE_THAT(closest.signedDistance, withinAbsT(tri.signedDistance(q), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("TriMeshSDF: every BVH::Construction value finds the nearest triangle, and the top-down "
                   "methods keep each leaf within maxLeafGroups",
                   "[BVH][TriMesh][Construction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // A soup of 512 small, differently oriented triangles on a jittered grid: enough for many levels,
  // and no two triangles at the same distance from a query.
  std::mt19937                      rng(17);
  std::uniform_real_distribution<T> jitter(T(-0.2), T(0.2));

  std::vector<Triangle<T, Meta>> tris;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      for (int k = 0; k < 8; k++) {
        const Vec3 base(T(i) + jitter(rng), T(j) + jitter(rng), T(k) + jitter(rng));
        const Vec3 a = base + Vec3(T(0.3), jitter(rng), jitter(rng));
        const Vec3 b = base + Vec3(jitter(rng), T(0.3), jitter(rng));
        const Vec3 n = (a - base).cross(b - base) / (a - base).cross(b - base).length();

        Triangle<T, Meta> tri;
        tri.setVertexPositions({base, a, b});
        tri.setNormal(n);
        tri.setVertexNormals({n, n, n});
        tri.setEdgeNormals({n, n, n});

        tris.emplace_back(tri);
      }
    }
  }

  std::vector<Vec3> queries;

  for (int q = 0; q < 200; q++) {
    queries.emplace_back(T(9) * (jitter(rng) + T(0.5)) - T(0.5),
                         T(9) * (jitter(rng) + T(0.5)) - T(0.5),
                         T(9) * (jitter(rng) + T(0.5)) - T(0.5));
  }

  Pool pool(hostMemoryResource());

  for (const size_t maxLeafGroups : {size_t(1), size_t(2), size_t(3)}) {
    for (const auto build : allConstructions) {
      const TriMeshSDF<T, Meta, K, W> sdf(tris, pool, build, maxLeafGroups);

      const auto nodes = sdf.getRoot().getNodes();

      const bool boundsLeaves = build == BVH::Construction::CentroidSplit ||
                                build == BVH::Construction::MidpointSplit || build == BVH::Construction::SAH ||
                                build == BVH::Construction::ClusterSAH;

      for (uint32_t i = 0; i < nodes.size(); i++) {
        if (boundsLeaves && nodes[i].isLeaf()) {
          REQUIRE(nodes[i].getNumPrimitives() <= maxLeafGroups);
        }
      }

      for (const auto& q : queries) {
        T nearest = std::numeric_limits<T>::max();

        for (const auto& tri : tris) {
          nearest = std::min(nearest, std::abs(tri.signedDistance(q)));
        }

        REQUIRE_THAT(std::abs(sdf.signedDistance(q)), withinAbsT(nearest, traversalMargin<T>()));
      }
    }
  }
}

TEMPLATE_TEST_CASE("Mesh SDFs: a value outside BVH::Construction aborts",
                   "[BVH][Construction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // An EBGEOMETRY_REQUIRE, so it aborts in every build rather than packing an unpartitioned tree.
  REQUIRE(abortsWith(
    [] {
      Pool                      pool(hostMemoryResource());
      const auto                mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"), pool);
      const MeshSDF<T, Meta, K> sdf(mesh, pool, invalidConstruction);

      (void)sdf;
    },
    "MeshSDF: unknown BVH::Construction value (99)"));

  REQUIRE(abortsWith(
    [] {
      Pool                            pool(hostMemoryResource());
      const auto                      mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"), pool);
      const TriMeshSDF<T, Meta, K, W> sdf(mesh, pool, invalidConstruction, 2);

      (void)sdf;
    },
    "TriMeshSDF: unknown BVH::Construction value (99)"));
}

namespace {

// Bare point primitive with no signedDistance() (or any other) member at all -- used below to
// prove that PackedBVH::pruneTraverse() imposes no interface requirement on its primitive type,
// unlike a caller-built signed-distance wrapper (e.g. MeshSDF/TriMeshSDF::signedDistance()),
// which requires P::signedDistance(Vec3T<T>). Also reused by the degenerate-axis
// bottomUpSortAndPartition test further below, whose nearest-neighbor query likewise goes
// through pruneTraverse().
template <class T>
struct BareTestPoint
{
  Vec3T<T> m_pos;
};

} // namespace

TEMPLATE_TEST_CASE("PackedBVH::pruneTraverse: nearest-neighbor search over a primitive with no "
                   "signedDistance() matches a brute-force scan",
                   "[BVH][pruneTraverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  // A modest, non-lattice point cloud -- enough to force multiple leaves/levels through the
  // default partitioner without making the brute-force cross-check slow.
  std::vector<Vec3> positions;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }
  REQUIRE(positions.size() == 125);

  BVH::PrimAndBVList<Pnt, AABB> primsAndBVs;
  for (const auto& pos : positions) {
    primsAndBVs.emplace_back(std::make_shared<Pnt>(Pnt{pos}), AABB(pos, pos));
  }

  auto tree = std::make_shared<BVH::TreeBVH<T, Pnt, AABB, K>>(primsAndBVs);
  tree->topDownSortAndPartition();

  const auto packed = tree->pack(pool);
  REQUIRE(packed != nullptr);
  REQUIRE(packed->getPrimitives().size() == positions.size());

  const auto& prims = packed->getPrimitives();

  // State is a running *squared* distance -- no abs(), no extra squaring for pruning, unlike
  // signedDistance()'s state -- exactly the shape a point-cloud nearest-neighbor search wants.
  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  for (const auto& q : queryPoints<T>()) {
    T state = std::numeric_limits<T>::max();

    const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (prims[a_offset + i].m_pos - q).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    packed->pruneTraverse(q, state, evalLeaf, pruneDist2);

    T bruteMin2 = std::numeric_limits<T>::max();
    for (const auto& pos : positions) {
      bruteMin2 = std::min(bruteMin2, (pos - q).length2());
    }

    REQUIRE_THAT(state, withinAbsT(bruteMin2, traversalMargin<T>()));
  }
}

// Regression test for the PackedBVH::appendTreeLeaf O(N^2) build bug: a per-leaf
// reserve(size + leafSize) that defeated std::vector's geometric growth, reallocating the whole
// primitive buffer on every leaf. Both build paths that append leaves one at a time -- the direct
// top-down PackedBVH constructor and TreeBVH::pack() -- hit it, so this exercises both and checks
// nearest-neighbor correctness against brute force. The primitive count is deliberately large: a
// reintroduced quadratic would blow well past the unit-test timeout here (minutes), while the
// linear build stays well under a second. Double only -- the append path is precision-independent,
// and one heavy build is enough of a guard.
TEST_CASE("PackedBVH: a large per-leaf build stays linear and correct", "[BVH][pruneTraverse][regression]")
{
  using T    = double;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;
  constexpr size_t N = 60000;

  Pool pool(hostMemoryResource());

  std::mt19937_64                   rng(20260709ULL);
  std::uniform_real_distribution<T> dist(T(0), T(1));

  std::vector<Vec3> positions;
  positions.reserve(N);
  for (size_t i = 0; i < N; i++) {
    positions.emplace_back(dist(rng), dist(rng), dist(rng));
  }

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  // Path 1: the direct top-down PackedBVH constructor, which appends each leaf via appendTreeLeaf
  // as it linearizes.
  std::vector<std::pair<Pnt, AABB>> flat;
  flat.reserve(N);
  for (const auto& pos : positions) {
    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }
  const Packed directBVH(pool, std::move(flat));

  // Path 2: TreeBVH::pack(), which appends each leaf the same way.
  BVH::PrimAndBVList<Pnt, AABB> primsAndBVs;
  primsAndBVs.reserve(N);
  for (const auto& pos : positions) {
    primsAndBVs.emplace_back(std::make_shared<Pnt>(Pnt{pos}), AABB(pos, pos));
  }
  auto tree = std::make_shared<BVH::TreeBVH<T, Pnt, AABB, K>>(primsAndBVs);
  tree->topDownSortAndPartition();
  const auto packedBVH = tree->pack(pool);

  REQUIRE(directBVH.getPrimitives().size() == N);
  REQUIRE(packedBVH->getPrimitives().size() == N);

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  const auto nearest2 = [&pruneDist2](const Packed& a_bvh, const Vec3& a_query) noexcept -> T {
    const auto& prims = a_bvh.getPrimitives();

    T state = std::numeric_limits<T>::max();

    const auto evalLeaf = [&prims, &a_query](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (prims[a_offset + i].m_pos - a_query).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    a_bvh.pruneTraverse(a_query, state, evalLeaf, pruneDist2);
    return state;
  };

  for (const auto& q : queryPoints<T>()) {
    T brute2 = std::numeric_limits<T>::max();
    for (const auto& pos : positions) {
      brute2 = std::min(brute2, (pos - q).length2());
    }

    REQUIRE_THAT(nearest2(directBVH, q), withinAbsT(brute2, traversalMargin<T>()));
    REQUIRE_THAT(nearest2(*packedBVH, q), withinAbsT(brute2, traversalMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVH refit: TreeBVH::refit and PackedBVH::refit update bounding volumes for a "
                   "moving geometry, keeping queries correct without a rebuild",
                   "[BVH][refit]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  // A modest, non-lattice point cloud -- enough to force multiple leaves/levels through the default
  // partitioner while keeping the brute-force cross-check fast.
  std::vector<Vec3> positions;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }
  REQUIRE(positions.size() == 125);

  // Keep mutable handles so the geometry can be *moved* after the BVH is built. TreeBVH stores
  // these very objects as shared_ptr<const Pnt>, so mutating a handle is what a moving geometry does
  // for the tree. The packed BVH is different: packing copies each primitive out, so it owns its own
  // copies and cannot see a handle change. Its counterpart is to move the packed primitives directly
  // through the mutable getPrimitives(); the loop below moves both representations by the same
  // displacement so the two must still agree afterwards.
  std::vector<std::shared_ptr<Pnt>> handles;
  handles.reserve(positions.size());
  for (const auto& pos : positions) {
    handles.emplace_back(std::make_shared<Pnt>(Pnt{pos}));
  }

  BVH::PrimAndBVList<Pnt, AABB> primsAndBVs;
  for (const auto& h : handles) {
    primsAndBVs.emplace_back(h, AABB(h->m_pos, h->m_pos));
  }

  auto tree = std::make_shared<BVH::TreeBVH<T, Pnt, AABB, K>>(primsAndBVs);
  tree->topDownSortAndPartition();

  const auto packed = tree->pack(pool);
  REQUIRE(packed != nullptr);
  REQUIRE(packed->getPrimitives().size() == positions.size());

  // A single primitive's current bounding volume: a zero-extent box at its (possibly moved) position.
  const auto bvConstructor = [](const Pnt& a_p) noexcept -> AABB { return AABB(a_p.m_pos, a_p.m_pos); };

  const auto& prims      = packed->getPrimitives();
  const auto  pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  // Nearest-neighbor (squared) over the packed BVH via pruneTraverse. If refit left any node volume
  // too small to enclose its moved primitives, pruning would wrongly discard the true nearest and
  // this would disagree with the brute-force scan below.
  const auto nearest2 = [&prims, &packed, &pruneDist2](const Vec3& a_query) noexcept -> T {
    T state = std::numeric_limits<T>::max();

    const auto evalLeaf = [&prims, &a_query](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (prims[a_offset + i].m_pos - a_query).length2();

        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    packed->pruneTraverse(a_query, state, evalLeaf, pruneDist2);

    return state;
  };

  // Tight bounding box over the current handle positions -- what a correctly-refitted root equals.
  const auto tightRoot = [&handles]() noexcept -> AABB {
    Vec3 lo = +Vec3::max();
    Vec3 hi = -Vec3::max();

    for (const auto& h : handles) {
      lo = min(lo, h->m_pos);
      hi = max(hi, h->m_pos);
    }

    return AABB(lo, hi);
  };

  const auto bruteNearest2 = [&handles](const Vec3& a_query) noexcept -> T {
    T brute2 = std::numeric_limits<T>::max();

    for (const auto& h : handles) {
      brute2 = std::min(brute2, (h->m_pos - a_query).length2());
    }

    return brute2;
  };

  SECTION("refit with unchanged geometry reproduces the original bounding volumes")
  {
    const AABB before = packed->getBoundingVolume();

    tree->refit(bvConstructor);
    packed->refit(bvConstructor);

    for (int d = 0; d < 3; d++) {
      REQUIRE_THAT(packed->getBoundingVolume().getLowCorner()[d],
                   withinAbsT(before.getLowCorner()[d], tightMargin<T>()));
      REQUIRE_THAT(packed->getBoundingVolume().getHighCorner()[d],
                   withinAbsT(before.getHighCorner()[d], tightMargin<T>()));
    }

    for (const auto& q : queryPoints<T>()) {
      REQUIRE_THAT(nearest2(q), withinAbsT(bruteNearest2(q), traversalMargin<T>()));
    }
  }

  SECTION("refit after moving the geometry keeps every query correct")
  {
    // Displace every point by a per-point, non-uniform translation: primitives shift without
    // migrating between leaves -- exactly the refit use case.
    for (size_t i = 0; i < handles.size(); i++) {
      const T s = T(0.05) * T(i);

      handles[i]->m_pos += Vec3(T(2.0) + s, T(-1.0) - s, T(0.5) * s);
    }

    // The packed BVH owns its primitives by value, so move them through its own array. Packing
    // reorders primitives into leaf order, so the i-th packed primitive is not the i-th handle --
    // match them up by position instead, which is well defined here because the cloud has no
    // duplicate points.
    auto packedPrims = packed->getPrimitives();

    REQUIRE(packedPrims.size() == positions.size());

    for (auto& prim : packedPrims) {
      size_t src = positions.size();

      for (size_t i = 0; i < positions.size(); i++) {
        if (prim.m_pos == positions[i]) {
          src = i;

          break;
        }
      }

      REQUIRE(src < positions.size());

      prim.m_pos = handles[src]->m_pos;
    }

    tree->refit(bvConstructor);
    packed->refit(bvConstructor);

    // Both roots must agree with the tight box over the moved cloud.
    const AABB tight = tightRoot();

    for (int d = 0; d < 3; d++) {
      REQUIRE_THAT(tree->getBoundingVolume().getLowCorner()[d], withinAbsT(tight.getLowCorner()[d], tightMargin<T>()));
      REQUIRE_THAT(tree->getBoundingVolume().getHighCorner()[d],
                   withinAbsT(tight.getHighCorner()[d], tightMargin<T>()));
      REQUIRE_THAT(packed->getBoundingVolume().getLowCorner()[d],
                   withinAbsT(tight.getLowCorner()[d], tightMargin<T>()));
      REQUIRE_THAT(packed->getBoundingVolume().getHighCorner()[d],
                   withinAbsT(tight.getHighCorner()[d], tightMargin<T>()));
    }

    for (const auto& q : queryPoints<T>()) {
      REQUIRE_THAT(nearest2(q), withinAbsT(bruteNearest2(q), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Parser::readIntoPackedBVH matches MeshSDF built directly from the same mesh",
                   "[BVH][Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  // One pool for both: the direct MeshSDF below and readIntoPackedBVH's own independent build
  // share it, which is only sound because building one mesh no longer closes the pool to the next.
  Pool       pool(hostMemoryResource());
  const auto direct = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"), pool);

  const MeshSDF<T, Meta, K> expected(direct, pool, BVH::Construction::SAH);
  const auto                fromFile = Parser::readIntoPackedBVH<T, Meta, K>(dataPath("dodecahedron.stl"), pool);

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(fromFile.signedDistance(p), withinAbsT(expected.signedDistance(p), formatMargin<T>()));
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

  Pool pool(hostMemoryResource());

  SECTION("readIntoDCEL")
  {
    const auto meshes = Parser::readIntoDCEL<T, Meta>(files, pool);
    REQUIRE(meshes.size() == 2);

    std::vector<DCEL::MeshT<T, Meta>> singles;
    singles.reserve(files.size());
    for (const auto& file : files) {
      singles.push_back(Parser::readIntoDCEL<T, Meta>(file, pool));
    }

    for (size_t i = 0; i < files.size(); i++) {
      const auto& single = singles[i];
      REQUIRE(meshes[i].numVertices() == single.numVertices());
      REQUIRE(meshes[i].numFaces() == single.numFaces());
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(meshes[i].signedDistance(p), withinAbsT(single.signedDistance(p), formatMargin<T>()));
      }
    }
  }

  SECTION("readIntoMesh")
  {
    const auto flatSDFs = Parser::readIntoMesh<T, Meta>(files, pool);
    REQUIRE(flatSDFs.size() == 2);

    for (size_t i = 0; i < files.size(); i++) {
      const auto single = Parser::readIntoMesh<T, Meta>(files[i], pool);
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(flatSDFs[i].signedDistance(p), withinAbsT(single.signedDistance(p), formatMargin<T>()));
      }
    }
  }

  SECTION("readIntoPackedBVH")
  {
    const auto packedSDFs = Parser::readIntoPackedBVH<T, Meta, K>(files, pool);
    REQUIRE(packedSDFs.size() == 2);

    for (size_t i = 0; i < files.size(); i++) {
      const auto single = Parser::readIntoPackedBVH<T, Meta, K>(files[i], pool);
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(packedSDFs[i].signedDistance(p), withinAbsT(single.signedDistance(p), formatMargin<T>()));
      }
    }
  }

  SECTION("readIntoTriangleBVH")
  {
    const auto triSDFs = Parser::readIntoTriangleBVH<T, Meta>(files, pool);
    REQUIRE(triSDFs.size() == 2);

    for (size_t i = 0; i < files.size(); i++) {
      const auto single = Parser::readIntoTriangleBVH<T, Meta>(files[i], pool);
      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(triSDFs[i].signedDistance(p), withinAbsT(single.signedDistance(p), formatMargin<T>()));
      }
    }
  }
}

TEMPLATE_TEST_CASE("TreeBVH::bottomUpSortAndPartition handles primitive sets whose centroids are "
                   "degenerate along one or more axes",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  // bottomUpSortAndPartition() normalizes primitive centroids into the space-filling curve's
  // coordinate system via (centroid - minCoord) / delta, where delta is derived from the
  // centroid bounding box. If minCoord == maxCoord on some axis (every primitive's centroid
  // coincides there), delta is zero on that axis and this division must not blow up.
  auto buildAndCheck = [&](const std::vector<Vec3>& a_positions) {
    BVH::PrimAndBVList<Pnt, AABB> primsAndBVs;
    for (const auto& pos : a_positions) {
      primsAndBVs.emplace_back(std::make_shared<Pnt>(Pnt{pos}), AABB(pos, pos));
    }

    for (const auto& sfcLabel : {"Morton", "Nested"}) {
      INFO("Space-filling curve: " << sfcLabel);

      auto tree = std::make_shared<BVH::TreeBVH<T, Pnt, AABB, K>>(primsAndBVs);

      if (std::string(sfcLabel) == "Morton") {
        tree->template bottomUpSortAndPartition<SFC::Morton>();
      }
      else {
        tree->template bottomUpSortAndPartition<SFC::Nested>();
      }

      const auto packed = tree->pack(pool);
      REQUIRE(packed != nullptr);
      REQUIRE(packed->getPrimitives().size() == a_positions.size());

      const auto& prims = packed->getPrimitives();

      // PackedBVH has no signedDistance() of its own -- do the nearest-neighbor search via
      // pruneTraverse(), whose State here is the running *squared* distance to the point cloud.
      const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

      for (const auto& q : queryPoints<T>()) {
        T          state    = std::numeric_limits<T>::max();
        const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
          for (size_t i = 0; i < a_count; i++) {
            const T d2 = (prims[a_offset + i].m_pos - q).length2();
            if (d2 < a_state) {
              a_state = d2;
            }
          }
        };

        packed->pruneTraverse(q, state, evalLeaf, pruneDist2);

        T bruteMin2 = std::numeric_limits<T>::max();
        for (const auto& pos : a_positions) {
          bruteMin2 = std::min(bruteMin2, (pos - q).length2());
        }

        REQUIRE_THAT(state, withinAbsT(bruteMin2, traversalMargin<T>()));
      }
    }
  };

  SECTION("All primitives exactly coincident (degenerate on every axis)")
  {
    const std::vector<Vec3> positions(30, Vec3(2, -1, 3));
    buildAndCheck(positions);
  }

  SECTION("Planar point cloud (z == 0 for every primitive, x/y vary normally)")
  {
    std::vector<Vec3> positions;
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        positions.emplace_back(T(i), T(j), T(0));
      }
    }
    buildAndCheck(positions);
  }

  SECTION("Cluster of exact duplicates plus a few distinct outliers")
  {
    std::vector<Vec3> positions(20, Vec3(0, 0, 0));
    positions.emplace_back(5, 5, 5);
    positions.emplace_back(-5, -5, -5);
    positions.emplace_back(10, 0, 0);
    buildAndCheck(positions);
  }
}

TEMPLATE_TEST_CASE("PackedBVH: primitives are stored inline with no pointer indirection",
                   "[BVH][PrimitiveStorage]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  using Pnt = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  // The primitive array's element type is P itself -- packing must not add any indirection, and
  // the span handed to a leaf callback must be a span of primitives, not of handles to them.
  static_assert(std::is_same_v<decltype(std::declval<const Packed&>().getPrimitives()), PODSpan<const Pnt>>);
  static_assert(std::is_same_v<decltype(std::declval<Packed&>().getPrimitives()), PODSpan<Pnt>>);

  // ... which in turn requires P to be trivially copyable: that is what lets a completed BVH be
  // byte-copied into a device address space, and what a shared_ptr-based primitive array could
  // never satisfy.
  static_assert(std::is_trivially_copyable_v<Pnt>);
  static_assert(std::is_trivially_copyable_v<Packed>);
}

TEMPLATE_TEST_CASE("PackedBVH: appendAliased genuinely appends (not only on the single-call "
                   "direct-build path)",
                   "[BVH][PrimitiveStorage]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  // appendAliased is the build helper every constructor that materialises a contiguous conversion
  // buffer into the flat primitive array goes through. It is a free function in BVH::detail rather
  // than a PackedBVH member precisely so it can be tested directly -- PackedBVH is final.
  // Build a fresh contiguous conversion buffer of three points offset by a_base.
  const auto makeBlock = [](T a_base) {
    auto block = std::make_shared<std::vector<Pnt>>();
    for (int i = 0; i < 3; i++) {
      block->push_back(Pnt{Vec3(a_base + T(i), a_base, a_base)});
    }
    return block;
  };

  // Two calls: the first into an empty destination (the fast path that steals the buffer wholesale),
  // the second into a non-empty one. appendAliased must add to what is already there rather than
  // replace it, or a second call site would silently lose the first block.
  std::vector<Pnt> dst;

  BVH::detail::appendAliased(dst, makeBlock(T(0)));
  BVH::detail::appendAliased(dst, makeBlock(T(10)));

  REQUIRE(dst.size() == 6);
  REQUIRE(dst[0].m_pos == Vec3(0, 0, 0));
  REQUIRE(dst[3].m_pos == Vec3(10, 10, 10));
}

TEMPLATE_TEST_CASE("MeshSDF/TriMeshSDF: both pack their primitives by value",
                   "[BVH][PrimitiveStorage]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  using Face     = DCEL::FaceT<T, Meta>;
  using TriAoSoA = TriangleAoSoA<T, Meta, W>;

  // MeshSDF stores each packed face inline, by value. A DCEL::FaceT is a plain trivially-copyable
  // value, so the copy is cheap; what it is not is self-contained -- its point-in-face test walks
  // the face's half-edge loop into the mesh's edges and vertices -- which is why MeshSDF retains the
  // source mesh and hands it to every face query. TriMeshSDF's SoA groups are self-contained
  // (freshly built by packing, shared with nothing), so they need no such companion.
  // See ImplemBVH.rst's "Primitive storage" section for the full rationale.
  static_assert(std::is_same_v<typename MeshSDF<T, Meta, K>::Root, BVH::PackedBVH<T, Face, K>>);
  static_assert(std::is_same_v<typename TriMeshSDF<T, Meta, K, W>::Root, BVH::PackedBVH<T, TriAoSoA, K>>);
}

TEMPLATE_TEST_CASE("TreeBVH: copy is disallowed (would alias mutable child subtrees); move is allowed",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Pnt  = BareTestPoint<T>;
  using Tree = BVH::TreeBVH<T, Pnt, AABB, 4>;

  static_assert(!std::is_copy_constructible_v<Tree>);
  static_assert(!std::is_copy_assignable_v<Tree>);
  static_assert(std::is_move_constructible_v<Tree>);
  static_assert(std::is_move_assignable_v<Tree>);
}

TEMPLATE_TEST_CASE("PackedBVH: copy aliases the same pool storage; deepCopy() is what makes it "
                   "independent",
                   "[BVH][Pool]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  // Pool-backed storage changes what a copy means. A PackedBVH is now three PODVector descriptors
  // plus a control-block pointer -- trivially copyable by design, since that is what lets a rebased
  // view be byte-copied into a device address space -- so copying one copies offsets, not data.
  static_assert(std::is_trivially_copyable_v<Packed>);
  static_assert(std::is_copy_constructible_v<Packed>);
  static_assert(std::is_copy_assignable_v<Packed>);
  static_assert(std::is_move_constructible_v<Packed>);
  static_assert(std::is_move_assignable_v<Packed>);

  Pool pool(hostMemoryResource());

  std::vector<Vec3> positions;

  positions.reserve(5);

  for (int i = 0; i < 5; i++) {
    positions.emplace_back(T(i), T(i) * T(0.5), T(-i));
  }

  std::vector<std::pair<Pnt, AABB>> flat;

  flat.reserve(positions.size());

  for (const auto& pos : positions) {
    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  const Packed original(pool, flat, size_t(2));

  REQUIRE(original.getPrimitives().size() == positions.size());

  SECTION("a copy shares the original's storage")
  {
    Packed alias = original;

    REQUIRE(alias.getPrimitives().size() == original.getPrimitives().size());
    REQUIRE(alias.isAttachedTo(pool));

    // Same pool, same offsets, therefore literally the same memory -- writing through one is
    // visible through the other.
    REQUIRE(alias.getPrimitives().begin() == original.getPrimitives().begin());

    // Packing reorders primitives into leaf order, so slot 0 is not positions[0]; snapshot what is
    // actually there rather than assuming an order.
    const Vec3 before = original.getPrimitives()[0].m_pos;

    alias.getPrimitives()[0].m_pos = Vec3(T(99), T(99), T(99));

    REQUIRE(original.getPrimitives()[0].m_pos == Vec3(T(99), T(99), T(99)));

    // Put it back so the section leaves no trace for the next one.
    alias.getPrimitives()[0].m_pos = before;
  }

  SECTION("deepCopy() gives storage of its own, in a pool of its own")
  {
    Pool dstPool(hostMemoryResource());

    Packed independent = original.deepCopy(dstPool);

    REQUIRE(independent.isAttachedTo(dstPool));
    REQUIRE_FALSE(independent.isAttachedTo(pool));
    REQUIRE(independent.getPrimitives().size() == original.getPrimitives().size());
    REQUIRE(independent.getPrimitives().begin() != original.getPrimitives().begin());

    for (uint32_t i = 0; i < original.getPrimitives().size(); i++) {
      REQUIRE(independent.getPrimitives()[i].m_pos == original.getPrimitives()[i].m_pos);
    }

    // Mutating the copy leaves the original alone -- the point of a deep copy.
    const Vec3 before = original.getPrimitives()[0].m_pos;

    independent.getPrimitives()[0].m_pos = Vec3(T(-7), T(-7), T(-7));

    REQUIRE(original.getPrimitives()[0].m_pos == before);
  }
}

namespace {

// pruneTraverse() is templated on its callables rather than taking std::function, and the CUDA/HIP
// builds do not pass --extended-lambda, so device callers must hand it functors. These are those
// functors. They live outside the device-only guard so the host suite compiles and exercises the
// exact types the kernel uses -- the kernel launch itself is then the only unverified part.
template <class T>
struct NearestLeafEval
{
  EBGeometry::PODSpan<const BareTestPoint<T>> m_prims;
  Vec3T<T>                                    m_query;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(T& a_state, size_t a_offset, size_t a_count) const noexcept
  {
    for (size_t i = 0; i < a_count; i++) {
      const T d2 = (m_prims[a_offset + i].m_pos - m_query).length2();

      if (d2 < a_state) {
        a_state = d2;
      }
    }
  }
};

template <class T>
struct IdentityPruneDist2
{
  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const T& a_state) const noexcept
  {
    return a_state;
  }
};

/**
 * @brief The entire body of the device query functor further below (PackedBvhTraversalQuery),
 * factored out so the host suite compiles and runs the exact code the device runs -- not merely the
 * same functors.
 *
 * The BVH is taken by value and const, matching the device side exactly. Both halves matter: by value
 * because that is how a descriptor reaches a kernel, and const because a non-const PackedBVH
 * selects getPrimitives()'s mutable overload, whose PODSpan<P> does not convert to the functors'
 * PODSpan<const P>. A test that reaches pruneTraverse() only through a const reference exercises a
 * different overload than the kernel does, and so cannot catch that mismatch.
 */
template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
T
packedBvhTraversalProbe(const EBGeometry::BVH::PackedBVH<T, BareTestPoint<T>, K> a_bvh, const Vec3T<T> a_query) noexcept
{
  T state = EBGeometry::Math::Limits<T>::max();

  const NearestLeafEval<T>    evalLeaf{a_bvh.getPrimitives(), a_query};
  const IdentityPruneDist2<T> pruneDist2{};

  a_bvh.pruneTraverse(a_query, state, evalLeaf, pruneDist2);

  // Also exercise the plain accessors so a broken base() shows up even if traversal were to pass.
  return state + T(a_bvh.getPrimitives().size()) + a_bvh.getBoundingVolume().getLowCorner().length();
}

} // namespace

TEMPLATE_TEST_CASE("PackedBVH: a host-to-host rebasedView answers every query identically",
                   "[BVH][Pool][rebase]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  // Mirroring host-to-host exercises the whole rebase invariant -- offsets resolving against a
  // different base -- without needing a GPU, which is the only way it runs in CI at all.
  Pool pool(hostMemoryResource());

  std::vector<Vec3> positions;

  positions.reserve(40);

  for (int i = 0; i < 40; i++) {
    const T t = T(i);

    positions.emplace_back(std::sin(t) * t, std::cos(t) * t, T(0.3) * t);
  }

  std::vector<std::pair<Pnt, AABB>> flat;

  flat.reserve(positions.size());

  for (const auto& pos : positions) {
    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  const Packed bvh(pool, flat, size_t(3));

  pool.freeze();

  Pool         mirrorPool = Pool::mirror(pool, hostMemoryResource());
  const Packed rebased    = bvh.rebasedView(mirrorPool);

  REQUIRE(rebased.isAttachedTo(mirrorPool));
  REQUIRE_FALSE(rebased.isAttachedTo(pool));
  REQUIRE(rebased.getPrimitives().size() == bvh.getPrimitives().size());

  // Different address space, same answers, bit for bit.
  REQUIRE(rebased.getPrimitives().begin() != bvh.getPrimitives().begin());

  // Deliberately packedBvhTraversalProbe() rather than a lambda written to look like it: the probe
  // *is* what the device test evaluates per query point, so running it here compiles and checks that
  // path -- functors, by-value const descriptor and all -- on every build, GPU or not.
  for (const auto& q : queryPoints<T>()) {
    REQUIRE(packedBvhTraversalProbe<T, K>(rebased, q) == packedBvhTraversalProbe<T, K>(bvh, q));
  }
}

TEMPLATE_TEST_CASE("FlatMeshSDF/MeshSDF/TriMeshSDF: move constructor/assignment are usable (no "
                   "longer implicitly suppressed by the user-declared destructor)",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  static_assert(std::is_copy_constructible_v<FlatMeshSDF<T, Meta>>);
  static_assert(std::is_copy_assignable_v<FlatMeshSDF<T, Meta>>);
  static_assert(std::is_move_constructible_v<FlatMeshSDF<T, Meta>>);
  static_assert(std::is_move_assignable_v<FlatMeshSDF<T, Meta>>);

  static_assert(std::is_copy_constructible_v<MeshSDF<T, Meta, K>>);
  static_assert(std::is_copy_assignable_v<MeshSDF<T, Meta, K>>);
  static_assert(std::is_move_constructible_v<MeshSDF<T, Meta, K>>);
  static_assert(std::is_move_assignable_v<MeshSDF<T, Meta, K>>);

  static_assert(std::is_copy_constructible_v<TriMeshSDF<T, Meta, K, W>>);
  static_assert(std::is_copy_assignable_v<TriMeshSDF<T, Meta, K, W>>);
  static_assert(std::is_move_constructible_v<TriMeshSDF<T, Meta, K, W>>);
  static_assert(std::is_move_assignable_v<TriMeshSDF<T, Meta, K, W>>);
}

TEMPLATE_TEST_CASE("Parser::readIntoTriangles and TriMeshSDF's mesh constructor extract identical triangles",
                   "[BVH][TriMeshSDF][Parser]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh      = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);
  const auto triangles = Parser::readIntoTriangles<T, Meta>(dataPath("dodecahedron.obj"));

  REQUIRE(triangles.size() == mesh.numFaces());

  // Each triangle carries its face's metadata and its three half-edges' normals -- not the vertex
  // normals, which readIntoTriangles used to substitute for them.
  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const auto& face        = mesh.getFace(i);
    const auto  edgeIndices = face.gatherEdgeIndices(mesh);

    REQUIRE(edgeIndices.size() == 3);
    REQUIRE(triangles[i].getMetaData() == face.getMetaData());

    for (size_t e = 0; e < 3; e++) {
      REQUIRE(triangles[i].getEdgeNormals()[e] == mesh.getEdge(edgeIndices[e]).getNormal());
    }
  }

  // Built from identical triangles with the same strategy, the two constructors give identical trees.
  const TriMeshSDF<T, Meta, K, W> fromMesh(mesh, pool, BVH::Construction::SAH, 2);
  const TriMeshSDF<T, Meta, K, W> fromSoup(triangles, pool, BVH::Construction::SAH, 2);

  for (const auto& p : queryPoints<T>()) {
    REQUIRE(fromSoup.signedDistance(p) == fromMesh.signedDistance(p));
    REQUIRE(fromSoup.getClosestTriangle(p).metaData == fromMesh.getClosestTriangle(p).metaData);
  }
}

TEMPLATE_TEST_CASE("FlatMeshSDF::computeBoundingVolume is the vertex AABB, as MeshSDF's root box is",
                   "[BVH][FlatMeshSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

  const FlatMeshSDF<T, Meta> flat(mesh, pool);
  const MeshSDF<T, Meta, K>  meshSDF(mesh, pool, BVH::Construction::SAH);

  const AABB fromFlat     = flat.computeBoundingVolume();
  const AABB fromVertices = AABB(mesh.getAllVertexCoordinates());
  const AABB fromBVH      = meshSDF.computeBoundingVolume();

  REQUIRE(fromFlat.getLowCorner() == fromVertices.getLowCorner());
  REQUIRE(fromFlat.getHighCorner() == fromVertices.getHighCorner());
  REQUIRE(fromFlat.getLowCorner() == fromBVH.getLowCorner());
  REQUIRE(fromFlat.getHighCorner() == fromBVH.getHighCorner());
}

TEMPLATE_TEST_CASE("FlatMeshSDF/MeshSDF/TriMeshSDF: rebasedView and deepCopy answer exactly as the original does",
                   "[BVH][FlatMeshSDF][MeshSDF][TriMeshSDF][rebase]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  using Flat = FlatMeshSDF<T, Meta>;
  using Mesh = MeshSDF<T, Meta, K>;
  using Tri  = TriMeshSDF<T, Meta, K, W>;

  // Each class is itself what crosses to a device, so it must stay trivially copyable, and both
  // crossings must return the class itself rather than some narrower type.
  static_assert(std::is_trivially_copyable_v<Flat>);
  static_assert(std::is_trivially_copyable_v<Mesh>);
  static_assert(std::is_trivially_copyable_v<Tri>);
  static_assert(std::is_same_v<decltype(std::declval<const Flat&>().rebasedView(std::declval<const Pool&>())), Flat>);
  static_assert(std::is_same_v<decltype(std::declval<const Mesh&>().rebasedView(std::declval<const Pool&>())), Mesh>);
  static_assert(std::is_same_v<decltype(std::declval<const Tri&>().rebasedView(std::declval<const Pool&>())), Tri>);
  static_assert(std::is_same_v<decltype(std::declval<const Flat&>().deepCopy(std::declval<Pool&>())), Flat>);
  static_assert(std::is_same_v<decltype(std::declval<const Mesh&>().deepCopy(std::declval<Pool&>())), Mesh>);
  static_assert(std::is_same_v<decltype(std::declval<const Tri&>().deepCopy(std::declval<Pool&>())), Tri>);

  // The dodecahedron fixture is pre-triangulated, so TriMeshSDF accepts it too.
  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

  const Flat flat(mesh, pool);
  const Mesh meshSDF(mesh, pool, BVH::Construction::SAH);
  const Tri  triSDF(mesh, pool, BVH::Construction::SAH, 2);

  REQUIRE(flat.isAttachedTo(pool));
  REQUIRE(meshSDF.isAttachedTo(pool));
  REQUIRE(triSDF.isAttachedTo(pool));

  for (const auto& p : queryPoints<T>()) {
    REQUIRE(flat.signedDistance(p) == mesh.signedDistance(p));
  }

  // Same tree, same data: every answer must match the original's exactly, including the metadata
  // TriMeshSDF::getClosestTriangle() reports.
  const auto requireSame = [&flat, &meshSDF, &triSDF](const Flat& a_flat, const Mesh& a_mesh, const Tri& a_tri) {
    for (const auto& p : queryPoints<T>()) {
      REQUIRE(a_flat.signedDistance(p) == flat.signedDistance(p));
      REQUIRE(a_mesh.signedDistance(p) == meshSDF.signedDistance(p));
      REQUIRE(a_tri.signedDistance(p) == triSDF.signedDistance(p));

      const auto closest  = a_tri.getClosestTriangle(p);
      const auto expected = triSDF.getClosestTriangle(p);

      REQUIRE(closest.signedDistance == expected.signedDistance);
      REQUIRE(closest.metaData == expected.metaData);
    }
  };

  SECTION("deepCopy into a separate pool is independent storage")
  {
    Pool other(hostMemoryResource());

    const Flat flatCopy = flat.deepCopy(other);
    const Mesh meshCopy = meshSDF.deepCopy(other);
    const Tri  triCopy  = triSDF.deepCopy(other);

    REQUIRE(flatCopy.isAttachedTo(other));
    REQUIRE(meshCopy.isAttachedTo(other));
    REQUIRE(triCopy.isAttachedTo(other));
    REQUIRE_FALSE(flatCopy.isAttachedTo(pool));
    REQUIRE_FALSE(meshCopy.isAttachedTo(pool));
    REQUIRE_FALSE(triCopy.isAttachedTo(pool));

    requireSame(flatCopy, meshCopy, triCopy);
  }

  SECTION("deepCopy into its own pool survives the pool growing under it")
  {
    const Flat flatCopy = flat.deepCopy(pool);
    const Mesh meshCopy = meshSDF.deepCopy(pool);
    const Tri  triCopy  = triSDF.deepCopy(pool);

    REQUIRE(meshCopy.isAttachedTo(pool));

    requireSame(flatCopy, meshCopy, triCopy);
  }

  SECTION("a host-to-host rebasedView resolves against the mirror")
  {
    // The host-side analogue of the device crossing: same offsets, a different block, identical
    // answers.
    pool.freeze();

    Pool mirror = Pool::mirror(pool, hostMemoryResource());

    const Flat flatView = flat.rebasedView(mirror);
    const Mesh meshView = meshSDF.rebasedView(mirror);
    const Tri  triView  = triSDF.rebasedView(mirror);

    REQUIRE(flatView.isAttachedTo(mirror));
    REQUIRE(meshView.isAttachedTo(mirror));
    REQUIRE(triView.isAttachedTo(mirror));
    REQUIRE_FALSE(meshView.isAttachedTo(pool));

    requireSame(flatView, meshView, triView);
  }
}

namespace {

// Brute-force nearest-squared-distance scan, shared by the direct-SFC-build tests below.
template <class T>
T
bruteForceNearest2(const std::vector<Vec3T<T>>& a_positions, const Vec3T<T>& a_query)
{
  T best2 = std::numeric_limits<T>::max();
  for (const auto& pos : a_positions) {
    best2 = std::min(best2, (pos - a_query).length2());
  }
  return best2;
}

// Nearest-squared-distance query via pruneTraverse(), shared by the direct-SFC-build tests below.
// Works whether the packed primitive is the point itself or a uint32_t index into a caller-owned
// array, since resolving a stored element back to a point is hidden behind the caller-supplied
// evalLeaf in each test.

} // namespace

TEMPLATE_TEST_CASE("PackedBVH: direct SFC-build constructor (no TreeBVH) matches brute-force "
                   "nearest-neighbor, including target leaf sizes that don't divide evenly into a "
                   "power of K",
                   "[BVH][DirectSFCBuild]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  std::vector<Vec3> positions;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }
  REQUIRE(positions.size() == 125);

  // Target leaf sizes chosen so the resulting real-leaf count lands both exactly on a power of K
  // (125/25=5 real leaves -> pads to 8) and well off of one (125/7=18 real leaves -> pads to 32;
  // 125/1=125 real leaves -> pads to 128), exercising the padding path at different ratios.
  for (const size_t targetLeafSize : {size_t(1), size_t(7), size_t(25), size_t(125)}) {
    INFO("Target leaf size: " << targetLeafSize);

    std::vector<std::pair<Pnt, AABB>> primsAndBVs;
    primsAndBVs.reserve(positions.size());
    for (const auto& pos : positions) {
      primsAndBVs.emplace_back(Pnt{pos}, AABB(pos, pos));
    }

    const BVH::PackedBVH<T, Pnt, K> packed(pool, std::move(primsAndBVs), targetLeafSize);

    REQUIRE(packed.getPrimitives().size() == positions.size());

    const auto& prims = packed.getPrimitives();

    const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    for (const auto& q : queryPoints<T>()) {
      T          state    = std::numeric_limits<T>::max();
      const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d2 = (prims[a_offset + i].m_pos - q).length2();
          if (d2 < a_state) {
            a_state = d2;
          }
        }
      };

      packed.pruneTraverse(q, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(positions, q), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH: direct SFC-build constructor handles degenerate-axis primitive "
                   "sets, same as TreeBVH::bottomUpSortAndPartition",
                   "[BVH][DirectSFCBuild]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  auto buildAndCheck = [&](const std::vector<Vec3>& a_positions) {
    std::vector<std::pair<Pnt, AABB>> primsAndBVs;
    primsAndBVs.reserve(a_positions.size());
    for (const auto& pos : a_positions) {
      primsAndBVs.emplace_back(Pnt{pos}, AABB(pos, pos));
    }

    const BVH::PackedBVH<T, Pnt, K> packed(pool, std::move(primsAndBVs), size_t(4));

    REQUIRE(packed.getPrimitives().size() == a_positions.size());

    const auto& prims      = packed.getPrimitives();
    const auto  pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    for (const auto& q : queryPoints<T>()) {
      T          state    = std::numeric_limits<T>::max();
      const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d2 = (prims[a_offset + i].m_pos - q).length2();
          if (d2 < a_state) {
            a_state = d2;
          }
        }
      };

      packed.pruneTraverse(q, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(a_positions, q), traversalMargin<T>()));
    }
  };

  SECTION("All primitives exactly coincident (degenerate on every axis)")
  {
    const std::vector<Vec3> positions(30, Vec3(2, -1, 3));
    buildAndCheck(positions);
  }

  SECTION("Planar point cloud (z == 0 for every primitive, x/y vary normally)")
  {
    std::vector<Vec3> positions;
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        positions.emplace_back(T(i), T(j), T(0));
      }
    }
    buildAndCheck(positions);
  }

  SECTION("Cluster of exact duplicates plus a few distinct outliers")
  {
    std::vector<Vec3> positions(20, Vec3(0, 0, 0));
    positions.emplace_back(5, 5, 5);
    positions.emplace_back(-5, -5, -5);
    positions.emplace_back(10, 0, 0);
    buildAndCheck(positions);
  }

  SECTION("Single primitive (root is itself a leaf, no internal nodes at all)")
  {
    const std::vector<Vec3> positions{Vec3(1, 2, 3)};
    buildAndCheck(positions);
  }
}

// A BVH whose primitive *is* a uint32_t index into a caller-owned array is the supported way to
// build an indexed BVH: PackedBVH<T, uint32_t, K> stores four bytes per primitive and the leaf
// callback resolves the index against whatever array the caller owns. Because uint32_t is just an
// ordinary primitive type, every build path accepts it -- this checks the SFC-build constructor and
// TreeBVH::pack(), the latter being the one an index could *not* travel through while the primitive
// array was governed by a storage policy. Both must agree exactly -- bit-for-bit, not just to a
// tolerance -- with a BVH packing the primitives themselves over identical bounding volumes.
TEMPLATE_TEST_CASE("PackedBVH: direct SFC-build constructor -- a uint32_t-index BVH agrees exactly "
                   "with one packing the primitives themselves",
                   "[BVH][DirectSFCBuild][PrimitiveStorage]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  // One canonical primitive array. The by-value BVH copies these in; the indexed one stores only
  // uint32_t indices back into this array, which therefore has to outlive it -- an index is not an
  // owner. Both BVHs see exactly the same primitives, so every query must agree bit-for-bit.
  std::vector<Pnt> owner;

  owner.reserve(23);

  for (int i = 0; i < 23; i++) {
    const T t = T(i);

    owner.push_back(Pnt{Vec3(std::sin(t) * t, std::cos(t) * t, T(0.25) * t)});
  }

  std::vector<std::pair<Pnt, AABB>>      valuePrims;
  std::vector<std::pair<uint32_t, AABB>> indexPrims;

  valuePrims.reserve(owner.size());
  indexPrims.reserve(owner.size());

  for (uint32_t i = 0; i < owner.size(); i++) {
    const AABB bv(owner[i].m_pos, owner[i].m_pos);

    valuePrims.emplace_back(owner[i], bv);
    indexPrims.emplace_back(i, bv);
  }

  const BVH::PackedBVH<T, Pnt, K>      valueBVH(pool, valuePrims, size_t(7));
  const BVH::PackedBVH<T, uint32_t, K> indexBVH(pool, indexPrims, size_t(7));

  // The same index primitives, but flattened out of a TreeBVH instead of built directly.
  BVH::PrimAndBVList<uint32_t, AABB> treePrims;

  treePrims.reserve(owner.size());

  for (uint32_t i = 0; i < owner.size(); i++) {
    treePrims.emplace_back(std::make_shared<const uint32_t>(i), AABB(owner[i].m_pos, owner[i].m_pos));
  }

  auto indexTree = std::make_shared<BVH::TreeBVH<T, uint32_t, AABB, K>>(treePrims);
  indexTree->topDownSortAndPartition();

  const auto packedIndexBVH = indexTree->pack(pool);

  REQUIRE(valueBVH.getPrimitives().size() == owner.size());
  REQUIRE(indexBVH.getPrimitives().size() == owner.size());
  REQUIRE(packedIndexBVH->getPrimitives().size() == owner.size());

  // Four bytes per primitive, and still trivially copyable -- so an indexed BVH mirrors to a device
  // exactly like any other.
  static_assert(sizeof(indexBVH.getPrimitives()[0]) == 4);
  static_assert(std::is_trivially_copyable_v<BVH::PackedBVH<T, uint32_t, K>>);

  const auto& valuePrimArray  = valueBVH.getPrimitives();
  const auto& indexPrimArray  = indexBVH.getPrimitives();
  const auto& packedPrimArray = packedIndexBVH->getPrimitives();

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  for (const auto& q : queryPoints<T>()) {
    T valueState  = std::numeric_limits<T>::max();
    T indexState  = std::numeric_limits<T>::max();
    T packedState = std::numeric_limits<T>::max();

    const auto valueEval = [&valuePrimArray, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (valuePrimArray[a_offset + i].m_pos - q).length2();

        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    // The indexed counterpart: the leaf callback resolves each stored index against the array the
    // caller owns, rather than against anything the BVH holds.
    const auto indexEval = [&indexPrimArray, &owner, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (owner[indexPrimArray[a_offset + i]].m_pos - q).length2();

        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    const auto packedEval = [&packedPrimArray, &owner, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (owner[packedPrimArray[a_offset + i]].m_pos - q).length2();

        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    valueBVH.pruneTraverse(q, valueState, valueEval, pruneDist2);
    indexBVH.pruneTraverse(q, indexState, indexEval, pruneDist2);
    packedIndexBVH->pruneTraverse(q, packedState, packedEval, pruneDist2);

    T bruteMin2 = std::numeric_limits<T>::max();

    for (const auto& pnt : owner) {
      bruteMin2 = std::min(bruteMin2, (pnt.m_pos - q).length2());
    }

    REQUIRE(indexState == valueState);
    REQUIRE(packedState == valueState);
    REQUIRE_THAT(indexState, withinAbsT(bruteMin2, traversalMargin<T>()));
  }
}

namespace {

// Host memory that reports itself device-accessible, like managed memory: a view rebased onto a
// mirror in it is a device view, checked against DeviceTraversalDepth, and still readable here.
class DeviceAccessibleHostResource final : public MemoryResource
{
public:
  void*
  allocate(size_t a_bytes, size_t a_alignment) override
  {
    return hostMemoryResource().allocate(a_bytes, a_alignment);
  }

  void
  deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept override
  {
    hostMemoryResource().deallocate(a_ptr, a_bytes, a_alignment);
  }

  bool
  isHostAccessible() const noexcept override
  {
    return true;
  }

  bool
  isDeviceAccessible() const noexcept override
  {
    return true;
  }
};

} // namespace

TEMPLATE_TEST_CASE("PackedBVH: a tree too deep for pruneTraverse's fixed stack is rejected at build time, at every "
                   "branching factor",
                   "[BVH][death]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  // The stack is sized for HostTraversalDepth levels at every K, so a K = 16 tree may be as deep as a
  // K = 4 one. No builder gets near that depth, so the tree is written by hand and handed to the
  // node-array constructor: a chain of interior nodes, each with its first child the next node in the
  // chain and its other children leaves of its own. What is being tested is that exceeding the bound
  // fails loudly, rather than overflowing the stack, which in Release would be a silent
  // out-of-bounds write.
  const std::vector<Pnt> prims = {Pnt{Vec3(T(0), T(0), T(0))}};
  const AABB             box(Vec3(T(0), T(0), T(0)), Vec3(T(1), T(1), T(1)));

  const auto check = [&](auto a_k) {
    constexpr size_t K = decltype(a_k)::value;

    using Packed = BVH::PackedBVH<T, Pnt, K>;
    using Node   = typename Packed::Node;

    INFO("K = " << K);

    // a_interior interior nodes, then a final leaf: a tree a_interior + 1 levels deep.
    const auto chain = [&box](const size_t a_interior) {
      std::vector<Node> nodes(a_interior);

      const auto addLeaf = [&nodes, &box]() {
        Node leaf{};

        leaf.setBoundingVolume(box);
        leaf.setPrimitivesOffset(0);
        leaf.setNumPrimitives(1);

        nodes.push_back(leaf);

        return static_cast<uint32_t>(nodes.size() - 1);
      };

      for (size_t i = 0; i < a_interior; i++) {
        nodes[i].setBoundingVolume(box);

        const uint32_t next = (i + 1 < a_interior) ? static_cast<uint32_t>(i + 1) : addLeaf();

        nodes[i].setChildOffset(next, 0);

        for (size_t k = 1; k < K; k++) {
          const uint32_t leaf = addLeaf();

          nodes[i].setChildOffset(leaf, k);
        }
      }

      return nodes;
    };

    const size_t limit = BVH::HostTraversalDepth;

    REQUIRE_FALSE(aborts([&] {
      Pool         pool(hostMemoryResource());
      const Packed bvh(pool, chain(limit - 1), prims);

      (void)bvh;
    }));

    REQUIRE(abortsWith(
      [&] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, chain(limit), prims);

        (void)bvh;
      },
      "host build -- the tree is " + std::to_string(limit + 1) + " levels deep, more than the " +
        std::to_string(limit) + " levels a traversal stack holds"));

    // A tree at the host limit traverses without overflowing the stack sized for it, and visits
    // every leaf.
    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, chain(limit - 1), prims);

    size_t visits = 0;
    T      state  = T(0);

    bvh.pruneTraverse(
      Vec3(T(0.5), T(0.5), T(0.5)),
      state,
      [&visits](T&, size_t, size_t a_count) noexcept { visits += a_count; },
      [](const T&) noexcept -> T { return std::numeric_limits<T>::infinity(); });

    REQUIRE(visits == 1 + (limit - 1) * (K - 1));

    // A device view is checked against the shallower device limit when it is made.
    const size_t deviceLimit = BVH::DeviceTraversalDepth;

    const auto deviceView = [&](const size_t a_interior) {
      return [&, a_interior] {
        Pool         hostPool(hostMemoryResource());
        const Packed tree(hostPool, chain(a_interior), prims);

        hostPool.freeze();

        DeviceAccessibleHostResource managed;

        const Pool   mirror = Pool::mirror(hostPool, managed);
        const Packed view   = tree.rebasedView(mirror);

        (void)view;
      };
    };

    REQUIRE_FALSE(aborts(deviceView(deviceLimit - 1)));
    REQUIRE(abortsWith(deviceView(deviceLimit),
                       "device view -- the tree is " + std::to_string(deviceLimit + 1) +
                         " levels deep, more than the " + std::to_string(deviceLimit) +
                         " levels a traversal stack holds"));
  };

  check(std::integral_constant<size_t, 2>{});
  check(std::integral_constant<size_t, 4>{});
  check(std::integral_constant<size_t, 16>{});
}

TEMPLATE_TEST_CASE("PackedBVH: the traversal stack holds HostTraversalDepth levels at every branching factor",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Checked at run time, not with static_assert: in a GPU build this file is also compiled for the
  // device, where the same function returns the device stack size.
  REQUIRE(BVH::PackedBVH<T, Vec3T<T>, 2>::traversalStackDepth() == 1 + 1 * (BVH::HostTraversalDepth - 1));
  REQUIRE(BVH::PackedBVH<T, Vec3T<T>, 4>::traversalStackDepth() == 1 + 3 * (BVH::HostTraversalDepth - 1));
  REQUIRE(BVH::PackedBVH<T, Vec3T<T>, 16>::traversalStackDepth() == 1 + 15 * (BVH::HostTraversalDepth - 1));

  static_assert(BVH::HostTraversalDepth == 256);
  static_assert(BVH::DeviceTraversalDepth == 32);
}

TEST_CASE("PackedBVH: the adopt constructor rejects a malformed node array", "[BVH][death]")
{
  using T    = double;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;
  using Node   = typename Packed::Node;

  // A leaf over prims [0, 2) under a root whose children are nodes 1..4 -- well formed, as the
  // first REQUIRE confirms, so each death below is caused by the one defect it introduces.
  const std::vector<Pnt> prims = {Pnt{Vec3(T(0), T(0), T(0))}, Pnt{Vec3(T(1), T(1), T(1))}};
  const AABB             box(Vec3(T(0), T(0), T(0)), Vec3(T(1), T(1), T(1)));

  std::vector<Node> good(K + 1);

  good[0].setBoundingVolume(box);

  for (size_t k = 0; k < K; k++) {
    good[0].setChildOffset(static_cast<uint32_t>(k + 1), k);
    good[k + 1].setBoundingVolume(box);
    good[k + 1].setPrimitivesOffset(0);
    good[k + 1].setNumPrimitives(2);
  }

  REQUIRE_FALSE(aborts([&] {
    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, good, prims);

    (void)bvh;
  }));

  SECTION("a leaf whose primitive range runs past the primitive array")
  {
    std::vector<Node> bad = good;

    bad[K].setNumPrimitives(3);

    REQUIRE(abortsWith(
      [&] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, bad, prims);

        (void)bvh;
      },
      "leaf 4's primitive range ends at 3, past the primitive array's 2 primitives"));
  }

  SECTION("a child offset pointing back at its own parent")
  {
    std::vector<Node> bad = good;

    bad[0].setChildOffset(0, K - 1);

    REQUIRE(abortsWith(
      [&] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, bad, prims);

        (void)bvh;
      },
      "node 0 has child offset 0, which is not strictly after its parent"));
  }

  SECTION("a child offset past the end of the node array")
  {
    std::vector<Node> bad = good;

    bad[0].setChildOffset(static_cast<uint32_t>(K + 1), 0);

    REQUIRE(abortsWith(
      [&] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, bad, prims);

        (void)bvh;
      },
      "node 0 has child offset 5, which is not strictly after its parent"));
  }

  SECTION("an empty node array paired with a non-empty primitive array")
  {
    REQUIRE(abortsWith(
      [&] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, std::vector<Node>{}, prims);

        (void)bvh;
      },
      "it is empty, but the primitive array holds 2 primitives"));
  }

  SECTION("a leaf with no primitives, which reads as an interior node whose children are all node 0")
  {
    std::vector<Node> bad = good;

    bad[K].setNumPrimitives(0);

    REQUIRE(abortsWith(
      [&] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, bad, prims);

        (void)bvh;
      },
      "node 4 has child offset 0, which is not strictly after its parent"));
  }

  SECTION("a node that is the child of two slots")
  {
    std::vector<Node> bad = good;

    bad[0].setChildOffset(1, K - 1);

    REQUIRE(abortsWith(
      [&] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, bad, prims);

        (void)bvh;
      },
      "node 0 names child 1, which already has another parent"));
  }

  SECTION("a node no slot names")
  {
    std::vector<Node> bad = good;

    bad.push_back(good[1]);

    REQUIRE(abortsWith(
      [&] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, bad, prims);

        (void)bvh;
      },
      "node 5 has no parent"));
  }
}

TEMPLATE_TEST_CASE("TreeBVH/PackedBVH: a partitioner that returns an empty partition aborts the build",
                   "[BVH][death]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  using Tree = BVH::TreeBVH<T, Vec3, AABB, K>;

  // An empty partition would become a leaf with no primitives, which the packed layout cannot
  // represent. The built-in partitioners never return one; this one puts everything in the first.
  const BVH::Partitioner<Vec3, AABB, K> lopsided = [](BVH::PrimAndBVList<Vec3, AABB> a_list) {
    Array<BVH::PrimAndBVList<Vec3, AABB>, K> parts;

    parts[0] = std::move(a_list);

    return parts;
  };

  std::vector<std::pair<Vec3, AABB>> primsAndBVs;

  for (int i = 0; i < 10; i++) {
    const Vec3 x(T(i), T(0), T(0));

    primsAndBVs.emplace_back(x, AABB(x, x));
  }

  REQUIRE(abortsWith(
    [&] {
      BVH::PrimAndBVList<Vec3, AABB> list;

      for (const auto& pb : primsAndBVs) {
        list.emplace_back(std::make_shared<const Vec3>(pb.first), pb.second);
      }

      Tree tree(list);

      tree.topDownSortAndPartition(lopsided);
    },
    "BVH::TreeBVH::topDownSortAndPartition: the partitioner returned an empty partition (1 of 4, from 10 "
    "primitives)"));

  REQUIRE(abortsWith(
    [&] {
      Pool                             pool(hostMemoryResource());
      const BVH::PackedBVH<T, Vec3, K> bvh(pool, primsAndBVs, lopsided);

      (void)bvh;
    },
    "BVH::PackedBVH: the partitioner returned an empty partition (1 of 4, from 10 primitives)"));
}

TEMPLATE_TEST_CASE("PackedBVH: the stack's float distance bound never prunes a closer primitive",
                   "[BVH][pruneTraverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  // Points on a sphere of radius 1e6 around the query, whose squared distances (about 1e12) differ in
  // bits that float cannot hold. A bound rounded up instead of down would prune the closest one.
  const Vec3 query(T(1), T(2), T(3));

  std::vector<std::pair<Vec3, AABB>> primsAndBVs;

  std::mt19937                      rng(42);
  std::uniform_real_distribution<T> angle(T(0), T(6.283185307179586));
  std::uniform_real_distribution<T> jitter(T(-1e-3), T(1e-3));

  for (int i = 0; i < 400; i++) {
    const T    theta = angle(rng);
    const T    phi   = angle(rng) / T(2);
    const T    r     = T(1e6) + jitter(rng);
    const Vec3 x = query + r * Vec3(std::sin(phi) * std::cos(theta), std::sin(phi) * std::sin(theta), std::cos(phi));

    primsAndBVs.emplace_back(x, AABB(x, x));
  }

  T brute = std::numeric_limits<T>::infinity();

  for (const auto& pb : primsAndBVs) {
    brute = std::min(brute, (pb.first - query).length2());
  }

  Pool pool(hostMemoryResource());

  const BVH::PackedBVH<T, Vec3, K> sah(pool, primsAndBVs, BVH::BinnedSAHPartitioner<T, Vec3, AABB, K>);
  const BVH::PackedBVH<T, Vec3, K> sfc(pool, primsAndBVs, K);

  for (const auto* bvh : {&sah, &sfc}) {
    const auto prims = bvh->getPrimitives();

    T best = std::numeric_limits<T>::infinity();

    bvh->pruneTraverse(
      query,
      best,
      [&prims, &query](T& a_best, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = a_offset; i < a_offset + a_count; i++) {
          a_best = std::min(a_best, (prims[static_cast<uint32_t>(i)] - query).length2());
        }
      },
      [](const T& a_best) noexcept { return a_best; });

    REQUIRE(best == brute);
  }
}

TEMPLATE_TEST_CASE("PackedBVH: only interior nodes get a child-box row, numbered in node order",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  std::vector<std::pair<Vec3, AABB>> primsAndBVs;

  for (int i = 0; i < 300; i++) {
    const Vec3 x(T(i % 7), T((i * 13) % 11), T((i * 29) % 17));

    primsAndBVs.emplace_back(x, AABB(x, x));
  }

  Pool                             pool(hostMemoryResource());
  const BVH::PackedBVH<T, Vec3, K> bvh(pool, primsAndBVs, BVH::BinnedSAHPartitioner<T, Vec3, AABB, K>);

  uint32_t expectedRow = 0;
  size_t   numLeaves   = 0;

  for (const auto& node : bvh.getNodes()) {
    if (node.isLeaf()) {
      numLeaves++;
    }
    else {
      REQUIRE(node.getChildBoxRow() == expectedRow);

      expectedRow++;
    }
  }

  REQUIRE(numLeaves > expectedRow);
}

TEMPLATE_TEST_CASE("BVH builders: a cluster of points whose extent is subnormal builds and queries exactly",
                   "[BVH][pruneTraverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  // A random cloud plus a cluster of points spaced by a subnormal step at the origin. When the SAH
  // builders reach the cluster, its extent is so small that the bin scale 32 / extent overflows to
  // infinity; that axis must be skipped, not binned (converting an infinite bin index to int indexed
  // the bins out of bounds, and crashed).
  const T step = std::numeric_limits<T>::denorm_min() * T(8);

  std::vector<std::pair<Vec3, AABB>> primsAndBVs;

  std::mt19937                      rng(7);
  std::uniform_real_distribution<T> unit(T(1), T(2));

  for (int i = 0; i < 300; i++) {
    const Vec3 x(unit(rng), unit(rng), unit(rng));

    primsAndBVs.emplace_back(x, AABB(x, x));
  }

  for (int j = 0; j < 64; j++) {
    const Vec3 x(T(j) * step, T(0), T(0));

    primsAndBVs.emplace_back(x, AABB(x, x));
  }

  Pool pool(hostMemoryResource());

  const std::vector<BVH::PackedBVH<T, Vec3, K>> bvhs = {
    BVH::PackedBVH<T, Vec3, K>(pool, primsAndBVs, BVH::BinnedSAHPartitioner<T, Vec3, AABB, K>),
    BVH::PackedBVH<T, Vec3, K>(pool, primsAndBVs, BVH::BinnedSAHPartitioner<T, Vec3, AABB, K, true>),
    BVH::PackedBVH<T, Vec3, K>(pool, primsAndBVs, BVH::ClusterSpec{2}),
    BVH::PackedBVH<T, Vec3, K>(pool, primsAndBVs, BVH::MidpointPartitioner<T, Vec3, AABB, K>),
    BVH::PackedBVH<T, Vec3, K>(pool, primsAndBVs, K)};

  const std::vector<Vec3> queries = {
    Vec3(T(0), T(0), T(0)), Vec3(T(1.5), T(1.5), T(1.5)), Vec3(T(-1), T(0.5), T(0)), Vec3(T(3), T(-1), T(0.5))};

  for (const auto& bvh : bvhs) {
    const auto prims = bvh.getPrimitives();

    for (const auto& q : queries) {
      T brute = std::numeric_limits<T>::infinity();

      for (const auto& pb : primsAndBVs) {
        brute = std::min(brute, (pb.first - q).length2());
      }

      T best = std::numeric_limits<T>::infinity();

      bvh.pruneTraverse(
        q,
        best,
        [&prims, &q](T& a_best, size_t a_offset, size_t a_count) noexcept {
          for (size_t i = a_offset; i < a_offset + a_count; i++) {
            a_best = std::min(a_best, (prims[static_cast<uint32_t>(i)] - q).length2());
          }
        },
        [](const T& a_best) noexcept { return a_best; });

      REQUIRE(best == brute);
    }
  }
}

TEMPLATE_TEST_CASE("TreeBVH::bottomUpSortAndPartition: exact powers of K fill every leaf level",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  // K = 3, since floor(log(N) / log(K)) in floating point comes out one short at N = 3^5 = 243
  // (and at 10^3, 3^10, ...), though at no power of 4.
  constexpr size_t K = 3;

  using Tree = BVH::TreeBVH<T, Vec3, AABB, K>;

  // Depth of the tree and the size of its biggest leaf.
  std::function<std::pair<size_t, size_t>(const Tree&)> shape = [&](const Tree& a_node) {
    if (a_node.isLeaf()) {
      return std::make_pair(size_t(1), a_node.getPrimitives().size());
    }

    std::pair<size_t, size_t> deepest{0, 0};

    for (const auto& child : a_node.getChildren()) {
      const auto sub = shape(*child);

      deepest.first  = std::max(deepest.first, sub.first);
      deepest.second = std::max(deepest.second, sub.second);
    }

    return std::make_pair(deepest.first + 1, deepest.second);
  };

  // With K^d primitives the tree has K^d leaves of one primitive each, d + 1 levels deep. The
  // floating-point depth gave K^(d-1) leaves of K primitives instead.
  for (const size_t depth : {size_t(1), size_t(2), size_t(3), size_t(5), size_t(6)}) {
    size_t n = 1;

    for (size_t d = 0; d < depth; d++) {
      n *= K;
    }

    BVH::PrimAndBVList<Vec3, AABB> list;

    for (size_t i = 0; i < n; i++) {
      const Vec3 x(T(i % 17), T((i / 17) % 19), T(i / 323));

      list.emplace_back(std::make_shared<const Vec3>(x), AABB(x, x));
    }

    auto tree = std::make_shared<Tree>(list);

    tree->template bottomUpSortAndPartition<SFC::Morton>();

    INFO("N = " << n);
    REQUIRE(shape(*tree) == std::make_pair(depth + 1, size_t(1)));
  }
}

TEMPLATE_TEST_CASE("TreeBVH::traverse visits every primitive once with a permissive pruning predicate",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  using Tree = BVH::TreeBVH<T, Vec3, AABB, K>;

  // traverse() used to keep a reference to the stack's top entry after popping it, so under
  // AddressSanitizer this read freed memory on the first interior node.
  BVH::PrimAndBVList<Vec3, AABB> list;

  for (int i = 0; i < 200; i++) {
    const Vec3 x(T(i % 9), T((i * 7) % 13), T((i * 3) % 5));

    list.emplace_back(std::make_shared<const Vec3>(x), AABB(x, x));
  }

  auto tree = std::make_shared<Tree>(list);

  tree->topDownSortAndPartition();

  size_t visits = 0;

  tree->template traverse<T>([&visits](const BVH::PrimitiveList<Vec3>& a_prims) noexcept { visits += a_prims.size(); },
                             [](const Tree&, const T&) noexcept { return true; },
                             [](Array<std::pair<std::shared_ptr<const Tree>, T>, K>&) noexcept {},
                             [](const Tree&) noexcept { return T(0); });

  REQUIRE(visits == list.size());
}

TEMPLATE_TEST_CASE("PackedBVH: an empty BVH has the empty bounding box and visits nothing",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Vec3, K>;

  Pool         pool(hostMemoryResource());
  const Packed bvh(pool, std::vector<typename Packed::Node>{}, std::vector<Vec3>{});

  const auto box = bvh.computeBoundingVolume();

  REQUIRE(box.getLowCorner()[0] > box.getHighCorner()[0]);

  size_t visits = 0;
  T      state  = T(0);

  bvh.pruneTraverse(
    Vec3(T(0), T(0), T(0)),
    state,
    [&visits](T&, size_t, size_t a_count) noexcept { visits += a_count; },
    [](const T&) noexcept { return std::numeric_limits<T>::infinity(); });

  REQUIRE(visits == 0);
}

TEST_CASE("PackedBVH: the direct builders reject an empty primitive list and a zero leaf or cluster size",
          "[BVH][death]")
{
  using T    = double;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;
  using List   = std::vector<std::pair<Pnt, AABB>>;

  List prims;

  for (int i = 0; i < 8; i++) {
    const Vec3 pos(T(i), T(0), T(0));

    prims.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  REQUIRE(abortsWith(
    [] {
      Pool         pool(hostMemoryResource());
      const Packed bvh(pool, List{}, size_t(4));
    },
    "PackedBVH: the SFC build needs at least one primitive"));

  REQUIRE(abortsWith(
    [&prims] {
      Pool         pool(hostMemoryResource());
      const Packed bvh(pool, prims, size_t(0));
    },
    "PackedBVH: the SFC build's target leaf size must be positive (0)"));

  REQUIRE(abortsWith(
    [] {
      Pool         pool(hostMemoryResource());
      const Packed bvh(pool, List{});
    },
    "PackedBVH: the top-down build needs at least one primitive"));

  REQUIRE(abortsWith(
    [] {
      Pool         pool(hostMemoryResource());
      const Packed bvh(pool, List{}, BVH::ClusterSpec{});
    },
    "PackedBVH: the ClusterSAH build needs at least one primitive"));

  REQUIRE(abortsWith(
    [&prims] {
      Pool         pool(hostMemoryResource());
      const Packed bvh(pool, prims, BVH::ClusterSpec{0});
    },
    "PackedBVH: ClusterSpec::maxClusterSize must be positive (0)"));
}

TEST_CASE("PackedBVH: rebasedView rejects a pool that is not a mirror of its own", "[BVH][rebase][death]")
{
  using T    = double;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());
      Pool unrelated(hostMemoryResource());

      std::vector<std::pair<Pnt, AABB>> prims;

      for (int i = 0; i < 8; i++) {
        const Vec3 pos(T(i), T(0), T(0));

        prims.emplace_back(Pnt{pos}, AABB(pos, pos));
      }

      const Packed bvh(pool, prims, size_t(2));
      const Packed view = bvh.rebasedView(unrelated);

      (void)view;
    },
    "BVH::PackedBVH::rebasedView: the pool must be the object's own pool or a mirror of it"));
}

TEST_CASE("FlatMeshSDF/MeshSDF/TriMeshSDF: the constructors reject a mismatched pool or a zero leaf size",
          "[BVH][FlatMeshSDF][MeshSDF][TriMeshSDF][death]")
{
  using T = double;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  REQUIRE(abortsWith(
    [] {
      Pool       pool(hostMemoryResource());
      Pool       other(hostMemoryResource());
      const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

      const FlatMeshSDF<T, Meta> sdf(mesh, other);
    },
    "FlatMeshSDF: the mesh must live in the pool passed in"));

  REQUIRE(abortsWith(
    [] {
      Pool       pool(hostMemoryResource());
      Pool       other(hostMemoryResource());
      const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

      const MeshSDF<T, Meta, K> sdf(mesh, other, BVH::Construction::SAH);
    },
    "MeshSDF: the mesh must live in the pool passed in"));

  REQUIRE(abortsWith(
    [] {
      Pool       pool(hostMemoryResource());
      const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

      const TriMeshSDF<T, Meta, K, W> sdf(mesh, pool, BVH::Construction::SAH, 0);
    },
    "TriMeshSDF: the maximum number of leaf groups must be positive (0)"));
}

TEMPLATE_TEST_CASE("PackedBVH: the adopt constructor rebuilds an identical BVH from getNodes() and "
                   "getPrimitives()",
                   "[BVH][adopt]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;
  using Node   = typename Packed::Node;

  Pool pool(hostMemoryResource());

  std::vector<std::pair<Pnt, AABB>> flat;

  flat.reserve(200);

  for (int i = 0; i < 200; i++) {
    const T    t = T(i) * T(0.1);
    const Vec3 p(std::sin(t) * t, std::cos(t) * t, T(0.3) * t);

    flat.emplace_back(Pnt{p}, AABB(p, p));
  }

  const Packed built(pool, flat, size_t(4));

  // Read the arrays back out through the public accessors a composing class uses, then adopt them.
  const auto nodeSpan = built.getNodes();
  const auto primSpan = built.getPrimitives();

  const std::vector<Node> nodes(nodeSpan.begin(), nodeSpan.end());
  const std::vector<Pnt>  prims(primSpan.begin(), primSpan.end());

  const Packed adopted(pool, nodes, prims);

  REQUIRE(adopted.getNodes().size() == built.getNodes().size());
  REQUIRE(adopted.getPrimitives().size() == built.getPrimitives().size());

  for (int i = 0; i < 25; i++) {
    const T    t = T(i) * T(0.7);
    const Vec3 query(t * T(0.5), T(1) - t, std::sin(t) * T(4));

    REQUIRE(packedBvhTraversalProbe<T, K>(adopted, query) == packedBvhTraversalProbe<T, K>(built, query));
  }
}

TEMPLATE_TEST_CASE("PackedBVH: direct ClusterSpec constructor -- a uint32_t-index BVH agrees exactly "
                   "with one packing the primitives themselves",
                   "[BVH][ClusterSAH][PrimitiveStorage]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  // Same construction as the SFC-build case above, but through the ClusterSpec constructor, which
  // has its own internal std::partition predicate over the (primitive, BV) pairs. That predicate
  // only reads the bounding volume, so a four-byte index has to travel through it exactly as a full
  // primitive does.
  std::vector<Pnt> owner;

  owner.reserve(29);

  for (int i = 0; i < 29; i++) {
    const T t = T(i);

    owner.push_back(Pnt{Vec3(std::cos(t) * t, std::sin(t) * t, T(0.4) * t)});
  }

  std::vector<std::pair<Pnt, AABB>>      valuePrims;
  std::vector<std::pair<uint32_t, AABB>> indexPrims;

  valuePrims.reserve(owner.size());
  indexPrims.reserve(owner.size());

  for (uint32_t i = 0; i < owner.size(); i++) {
    const AABB bv(owner[i].m_pos, owner[i].m_pos);

    valuePrims.emplace_back(owner[i], bv);
    indexPrims.emplace_back(i, bv);
  }

  const BVH::ClusterSpec spec{/* maxClusterSize */ 5};

  const BVH::PackedBVH<T, Pnt, K>      valueBVH(pool, valuePrims, spec);
  const BVH::PackedBVH<T, uint32_t, K> indexBVH(pool, indexPrims, spec);

  REQUIRE(valueBVH.getPrimitives().size() == owner.size());
  REQUIRE(indexBVH.getPrimitives().size() == owner.size());

  const auto& valuePrimArray = valueBVH.getPrimitives();
  const auto& indexPrimArray = indexBVH.getPrimitives();

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  for (const auto& q : queryPoints<T>()) {
    T valueState = std::numeric_limits<T>::max();
    T indexState = std::numeric_limits<T>::max();

    const auto valueEval = [&valuePrimArray, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (valuePrimArray[a_offset + i].m_pos - q).length2();

        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    const auto indexEval = [&indexPrimArray, &owner, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (owner[indexPrimArray[a_offset + i]].m_pos - q).length2();

        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    valueBVH.pruneTraverse(q, valueState, valueEval, pruneDist2);
    indexBVH.pruneTraverse(q, indexState, indexEval, pruneDist2);

    T bruteMin2 = std::numeric_limits<T>::max();

    for (const auto& pnt : owner) {
      bruteMin2 = std::min(bruteMin2, (pnt.m_pos - q).length2());
    }

    REQUIRE(indexState == valueState);
    REQUIRE_THAT(indexState, withinAbsT(bruteMin2, traversalMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PackedBVH: direct SFC-build constructor accepts an explicit SFC curve "
                   "(SFC::Nested)",
                   "[BVH][DirectSFCBuild]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  std::vector<Vec3> positions;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }

  std::vector<std::pair<Pnt, AABB>> primsAndBVs;
  primsAndBVs.reserve(positions.size());
  for (const auto& pos : positions) {
    primsAndBVs.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  const BVH::PackedBVH<T, Pnt, K> packed(pool, std::move(primsAndBVs), size_t(6), SFC::Nested{});

  REQUIRE(packed.getPrimitives().size() == positions.size());

  const auto& prims      = packed.getPrimitives();
  const auto  pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  for (const auto& q : queryPoints<T>()) {
    T          state    = std::numeric_limits<T>::max();
    const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (prims[a_offset + i].m_pos - q).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    packed.pruneTraverse(q, state, evalLeaf, pruneDist2);

    REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(positions, q), traversalMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PackedBVH: direct SFC-build constructor puts every primitive in exactly one leaf, "
                   "for any leaf count",
                   "[BVH][DirectSFCBuild]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  // An exhaustive traversal (the prune bound never rejects anything) must reach every primitive
  // exactly once, whatever the leaf count: a union that keeps the two smallest values, or any
  // other non-idempotent reduction, depends on it.
  const auto check = [](auto a_branching, size_t a_numPrims, size_t a_targetLeafSize) {
    constexpr size_t K = decltype(a_branching)::value;

    Pool pool(hostMemoryResource());

    std::vector<std::pair<uint32_t, AABB>> primsAndBVs;

    for (uint32_t i = 0; i < a_numPrims; i++) {
      const Vec3 p(T(i % 7), T((i / 7) % 5), T(i / 35));

      primsAndBVs.emplace_back(i, AABB(p, p));
    }

    const BVH::PackedBVH<T, uint32_t, K> packed(pool, std::move(primsAndBVs), a_targetLeafSize);

    const auto&           prims = packed.getPrimitives();
    std::vector<unsigned> visits(a_numPrims, 0U);

    const auto evalLeaf = [&prims, &visits](int&, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        visits[prims[a_offset + i]]++;
      }
    };
    const auto noPrune = [](const int&) noexcept -> T { return std::numeric_limits<T>::max(); };

    int state = 0;
    packed.pruneTraverse(Vec3::zeros(), state, evalLeaf, noPrune);

    for (size_t i = 0; i < a_numPrims; i++) {
      INFO("K = " << K << ", N = " << a_numPrims << ", leaf size = " << a_targetLeafSize << ", primitive " << i);
      REQUIRE(visits[i] == 1U);
    }

    // Every node is referenced once: a full K-ary tree with L leaves has L + (L - 1)/(K - 1) nodes.
    const auto& nodes    = packed.getNodes();
    size_t      leaves   = 0;
    size_t      smallest = a_numPrims;
    size_t      largest  = 0;

    for (size_t i = 0; i < nodes.size(); i++) {
      if (nodes[i].isLeaf()) {
        leaves++;
        smallest = std::min(smallest, size_t(nodes[i].getNumPrimitives()));
        largest  = std::max(largest, size_t(nodes[i].getNumPrimitives()));
      }
    }

    INFO("K = " << K << ", N = " << a_numPrims << ", leaf size = " << a_targetLeafSize);
    REQUIRE(nodes.size() == leaves + (leaves - 1) / (K - 1));
    REQUIRE(largest - smallest <= 1U);

    // A full K-ary tree needs a leaf count L = 1 (mod K - 1). When such an L lies between
    // ceil(N / target) and N, no leaf exceeds the target size.
    const size_t minLeaves  = (a_numPrims + a_targetLeafSize - 1) / a_targetLeafSize;
    bool         targetable = false;

    for (size_t l = minLeaves; l <= a_numPrims; l++) {
      targetable = targetable || ((l - 1) % (K - 1) == 0);
    }

    if (targetable) {
      REQUIRE(largest <= a_targetLeafSize);
    }
  };

  for (const size_t n : {1, 2, 3, 4, 5, 7, 16, 17, 64, 65, 68, 100, 257}) {
    for (const size_t leafSize : {1, 2, 3, 4, 8}) {
      check(std::integral_constant<size_t, 2>{}, n, leafSize);
      check(std::integral_constant<size_t, 4>{}, n, leafSize);
      check(std::integral_constant<size_t, 8>{}, n, leafSize);
    }
  }
}

TEMPLATE_TEST_CASE("TreeBVH/PackedBVH: signedDistance agrees with the brute-force mesh scan, for "
                   "every build method including PackedBVH's direct SFC-build (cheap fixture: "
                   "tetrahedron)",
                   "[BVH][Tetrahedron][DirectSFCBuild]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Tri  = Triangle<T, Meta>;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("tetrahedron.stl"), pool);
  REQUIRE(mesh.numFaces() == 4);

  const auto triangles = Parser::readIntoTriangles<T, Meta>(dataPath("tetrahedron.stl"));
  REQUIRE(triangles.size() == 4);

  const FlatMeshSDF<T, Meta> flat(mesh, pool);
  const auto                 brute = [&flat](const Vec3T<T>& a_point) -> T { return flat.signedDistance(a_point); };

  BVH::PrimAndBVList<Tri, AABB> primsAndBVs;
  for (const auto& tri : triangles) {
    const auto&                 vp = tri.getVertexPositions();
    const std::vector<Vec3T<T>> verts{vp[0], vp[1], vp[2]};
    primsAndBVs.emplace_back(std::make_shared<const Tri>(tri), AABB(verts));
  }
  REQUIRE(primsAndBVs.size() == 4);

  auto buildAndCheckTree = [&](const char* a_label, auto&& a_partitionFunction) {
    INFO("Build strategy: " << a_label);

    auto tree = std::make_shared<BVH::TreeBVH<T, Tri, AABB, K>>(primsAndBVs);
    a_partitionFunction(*tree);

    const auto packed = tree->pack(pool);
    REQUIRE(packed != nullptr);
    REQUIRE(packed->getPrimitives().size() == 4);

    const auto& prims = packed->getPrimitives();

    for (const auto& p : queryPoints<T>()) {
      T state = std::numeric_limits<T>::max();

      const auto evalLeaf = [&prims, &p](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d = prims[a_offset + i].signedDistance(p);
          if (std::abs(d) < std::abs(a_state)) {
            a_state = d;
          }
        }
      };
      const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

      packed->pruneTraverse(p, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(brute(p), traversalMargin<T>()));
    }
  };

  buildAndCheckTree("TopDown (default BVCentroidPartitioner)", [](auto& a_tree) { a_tree.topDownSortAndPartition(); });

  buildAndCheckTree("TopDown (BinnedSAHPartitioner)", [](auto& a_tree) {
    using Node              = BVH::TreeBVH<T, Tri, AABB, K>;
    using LeafPred          = typename Node::LeafPredicate;
    const LeafPred stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };

    a_tree.topDownSortAndPartition(BVH::BinnedSAHPartitioner<T, Tri, AABB, K>, stopCrit);
  });

  buildAndCheckTree("BottomUp (Morton)", [](auto& a_tree) { a_tree.template bottomUpSortAndPartition<SFC::Morton>(); });

  buildAndCheckTree("BottomUp (Nested)", [](auto& a_tree) { a_tree.template bottomUpSortAndPartition<SFC::Nested>(); });

  // Remaining build methods go straight to PackedBVH, bypassing TreeBVH entirely -- Triangle<T,
  // Meta> (unlike DCEL::FaceT) has a genuine memberwise copy constructor -- see its class
  // declaration -- so it is safe to use with constructors that take primitives by value; see
  // MeshSDF's own class doc for why DCEL::FaceT is not.
  auto checkDirect = [&](const char* a_label, const BVH::PackedBVH<T, Tri, K>& a_packed) {
    INFO(a_label);
    REQUIRE(a_packed.getPrimitives().size() == 4);

    const auto& prims = a_packed.getPrimitives();

    for (const auto& p : queryPoints<T>()) {
      T state = std::numeric_limits<T>::max();

      const auto evalLeaf = [&prims, &p](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d = prims[a_offset + i].signedDistance(p);
          if (std::abs(d) < std::abs(a_state)) {
            a_state = d;
          }
        }
      };
      const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

      a_packed.pruneTraverse(p, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(brute(p), traversalMargin<T>()));
    }
  };

  auto makeFlatPrims = [&]() {
    std::vector<std::pair<Tri, AABB>> flatPrimsAndBVs;
    for (const auto& tri : triangles) {
      const auto&                 vp = tri.getVertexPositions();
      const std::vector<Vec3T<T>> verts{vp[0], vp[1], vp[2]};
      flatPrimsAndBVs.emplace_back(tri, AABB(verts));
    }
    return flatPrimsAndBVs;
  };

  checkDirect("PackedBVH direct SFC-build (Morton)", BVH::PackedBVH<T, Tri, K>(pool, makeFlatPrims(), size_t(K)));

  checkDirect("PackedBVH direct top-down build (default BVCentroidPartitioner)",
              BVH::PackedBVH<T, Tri, K>(pool, makeFlatPrims()));

  {
    using Node          = BVH::TreeBVH<T, Tri, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    checkDirect("PackedBVH direct top-down build (SAH)",
                BVH::PackedBVH<T, Tri, K>(pool, makeFlatPrims(), BVH::BinnedSAHPartitioner<T, Tri, AABB, K>, stopCrit));
  }
}

TEMPLATE_TEST_CASE("PackedBVH: direct top-down/SAH-build constructor (no TreeBVH) matches "
                   "brute-force nearest-neighbor, for both the default and SAH partitioner",
                   "[BVH][DirectTopDownBuild]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  std::vector<Vec3> positions;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }
  REQUIRE(positions.size() == 125);

  auto makeFlatPrims = [&]() {
    std::vector<std::pair<Pnt, AABB>> primsAndBVs;
    primsAndBVs.reserve(positions.size());
    for (const auto& pos : positions) {
      primsAndBVs.emplace_back(Pnt{pos}, AABB(pos, pos));
    }
    return primsAndBVs;
  };

  auto checkPacked = [&](const char* a_label, const BVH::PackedBVH<T, Pnt, K>& a_packed) {
    INFO(a_label);
    REQUIRE(a_packed.getPrimitives().size() == positions.size());

    const auto& prims      = a_packed.getPrimitives();
    const auto  pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    for (const auto& q : queryPoints<T>()) {
      T          state    = std::numeric_limits<T>::max();
      const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d2 = (prims[a_offset + i].m_pos - q).length2();
          if (d2 < a_state) {
            a_state = d2;
          }
        }
      };

      a_packed.pruneTraverse(q, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(positions, q), traversalMargin<T>()));
    }
  };

  SECTION("Default partitioner (BVCentroidPartitioner), default leaf predicate")
  {
    const BVH::PackedBVH<T, Pnt, K> packed(pool, makeFlatPrims());
    checkPacked("Default", packed);
  }

  SECTION("SAH partitioner (BinnedSAHPartitioner), matching custom leaf predicate")
  {
    using Node          = BVH::TreeBVH<T, Pnt, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    const BVH::PackedBVH<T, Pnt, K> packed(pool, makeFlatPrims(), BVH::BinnedSAHPartitioner<T, Pnt, AABB, K>, stopCrit);
    checkPacked("SAH", packed);
  }

  SECTION("ClusterSAH partitioner (cluster primitives, then SAH over clusters)")
  {
    // Small maxClusterSize so several clusters form over the 125-point grid, exercising the
    // cluster-then-SAH path rather than collapsing to a single cluster.
    const BVH::PackedBVH<T, Pnt, K> packed(pool, makeFlatPrims(), BVH::ClusterSpec{size_t(4)});
    checkPacked("ClusterSAH", packed);
  }

  SECTION("Single primitive (root is itself a leaf) -- every direct partitioner")
  {
    const Vec3 only(T(1), T(2), T(3));

    const auto onePrim = [&]() {
      std::vector<std::pair<Pnt, AABB>> v;
      v.emplace_back(Pnt{only}, AABB(only, only));
      return v;
    };

    const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    const auto checkOne = [&](const char* a_label, const BVH::PackedBVH<T, Pnt, K>& a_packed) {
      INFO(a_label);
      REQUIRE(a_packed.getPrimitives().size() == 1);

      const auto& prims = a_packed.getPrimitives();
      for (const auto& q : queryPoints<T>()) {
        T          state    = std::numeric_limits<T>::max();
        const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
          for (size_t i = 0; i < a_count; i++) {
            a_state = std::min(a_state, (prims[a_offset + i].m_pos - q).length2());
          }
        };

        a_packed.pruneTraverse(q, state, evalLeaf, pruneDist2);

        REQUIRE_THAT(state, withinAbsT((only - q).length2(), traversalMargin<T>()));
      }
    };

    using Node          = BVH::TreeBVH<T, Pnt, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };

    checkOne("Default", BVH::PackedBVH<T, Pnt, K>(pool, onePrim()));
    checkOne("SAH", BVH::PackedBVH<T, Pnt, K>(pool, onePrim(), BVH::BinnedSAHPartitioner<T, Pnt, AABB, K>, stopCrit));
    checkOne("ClusterSAH", BVH::PackedBVH<T, Pnt, K>(pool, onePrim(), BVH::ClusterSpec{size_t(8)}));
  }
}

TEMPLATE_TEST_CASE("PackedBVH: direct top-down-build constructor agrees exactly with building via "
                   "TreeBVH::topDownSortAndPartition() then pack(), for the same partitioner",
                   "[BVH][DirectTopDownBuild]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  std::vector<Vec3> positions;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }

  BVH::PrimAndBVList<Pnt, AABB>     wrappedPrims;
  std::vector<std::pair<Pnt, AABB>> flatPrims;
  for (const auto& pos : positions) {
    wrappedPrims.emplace_back(std::make_shared<Pnt>(Pnt{pos}), AABB(pos, pos));
    flatPrims.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  auto tree = std::make_shared<BVH::TreeBVH<T, Pnt, AABB, K>>(wrappedPrims);
  tree->topDownSortAndPartition();
  const auto viaTree = tree->pack(pool);

  const BVH::PackedBVH<T, Pnt, K> direct(pool, std::move(flatPrims));

  REQUIRE(direct.getPrimitives().size() == viaTree->getPrimitives().size());

  const auto& directPrims = direct.getPrimitives();
  const auto& treePrims   = viaTree->getPrimitives();

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  for (const auto& q : queryPoints<T>()) {
    T directState = std::numeric_limits<T>::max();
    T treeState   = std::numeric_limits<T>::max();

    const auto directEval = [&directPrims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (directPrims[a_offset + i].m_pos - q).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };
    const auto treeEval = [&treePrims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (treePrims[a_offset + i].m_pos - q).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    direct.pruneTraverse(q, directState, directEval, pruneDist2);
    viaTree->pruneTraverse(q, treeState, treeEval, pruneDist2);

    REQUIRE(directState == treeState);
  }
}

TEMPLATE_TEST_CASE("BVH::MidpointPartitioner: matches brute-force nearest-neighbor via both "
                   "TreeBVH::topDownSortAndPartition() and PackedBVH's direct top-down constructor",
                   "[BVH][DirectTopDownBuild]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  std::vector<Vec3> positions;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }
  REQUIRE(positions.size() == 125);

  auto checkPacked = [&](const char* a_label, const BVH::PackedBVH<T, Pnt, K>& a_packed) {
    INFO(a_label);
    REQUIRE(a_packed.getPrimitives().size() == positions.size());

    const auto& prims      = a_packed.getPrimitives();
    const auto  pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    for (const auto& q : queryPoints<T>()) {
      T          state    = std::numeric_limits<T>::max();
      const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d2 = (prims[a_offset + i].m_pos - q).length2();
          if (d2 < a_state) {
            a_state = d2;
          }
        }
      };

      a_packed.pruneTraverse(q, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(positions, q), traversalMargin<T>()));
    }
  };

  SECTION("Via TreeBVH::topDownSortAndPartition() then pack()")
  {
    BVH::PrimAndBVList<Pnt, AABB> primsAndBVs;
    for (const auto& pos : positions) {
      primsAndBVs.emplace_back(std::make_shared<Pnt>(Pnt{pos}), AABB(pos, pos));
    }

    auto tree = std::make_shared<BVH::TreeBVH<T, Pnt, AABB, K>>(primsAndBVs);

    using Node          = BVH::TreeBVH<T, Pnt, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    tree->topDownSortAndPartition(BVH::MidpointPartitioner<T, Pnt, AABB, K>, stopCrit);

    const auto packed = tree->pack(pool);
    REQUIRE(packed != nullptr);
    checkPacked("Via TreeBVH", *packed);
  }

  SECTION("Via PackedBVH's direct top-down constructor")
  {
    std::vector<std::pair<Pnt, AABB>> flatPrims;
    flatPrims.reserve(positions.size());
    for (const auto& pos : positions) {
      flatPrims.emplace_back(Pnt{pos}, AABB(pos, pos));
    }

    using Node          = BVH::TreeBVH<T, Pnt, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    const BVH::PackedBVH<T, Pnt, K> packed(
      pool, std::move(flatPrims), BVH::MidpointPartitioner<T, Pnt, AABB, K>, stopCrit);
    checkPacked("Via direct constructor", packed);
  }
}

TEMPLATE_TEST_CASE("BVH::MidpointPartitioner: handles degenerate-axis primitive sets without "
                   "producing an empty group",
                   "[BVH][DirectTopDownBuild]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  auto buildAndCheck = [&](const std::vector<Vec3>& a_positions) {
    std::vector<std::pair<Pnt, AABB>> flatPrims;
    flatPrims.reserve(a_positions.size());
    for (const auto& pos : a_positions) {
      flatPrims.emplace_back(Pnt{pos}, AABB(pos, pos));
    }

    using Node = BVH::TreeBVH<T, Pnt, AABB, K>;
    // A capture-default (not an explicit [K]) is deliberate: this lambda is nested inside another,
    // and the two toolchains disagree about the constexpr K. GCC requires it be captured (errors
    // "K is not captured" with no capture list), while Clang -Werror rejects an explicit [K] as an
    // unused capture. A [=] default satisfies GCC and is not flagged by Clang. (Do not use [&]
    // here: g++ 13 ICEs on a by-reference capture in this exact nesting.)
    const auto stopCrit = [=](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    const BVH::PackedBVH<T, Pnt, K> packed(
      pool, std::move(flatPrims), BVH::MidpointPartitioner<T, Pnt, AABB, K>, stopCrit);

    REQUIRE(packed.getPrimitives().size() == a_positions.size());

    const auto& prims      = packed.getPrimitives();
    const auto  pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    for (const auto& q : queryPoints<T>()) {
      T          state    = std::numeric_limits<T>::max();
      const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d2 = (prims[a_offset + i].m_pos - q).length2();
          if (d2 < a_state) {
            a_state = d2;
          }
        }
      };

      packed.pruneTraverse(q, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(a_positions, q), traversalMargin<T>()));
    }
  };

  SECTION("All primitives exactly coincident (degenerate on every axis)")
  {
    const std::vector<Vec3> positions(30, Vec3(2, -1, 3));
    buildAndCheck(positions);
  }

  SECTION("Planar point cloud (z == 0 for every primitive, x/y vary normally)")
  {
    std::vector<Vec3> positions;
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
        positions.emplace_back(T(i), T(j), T(0));
      }
    }
    buildAndCheck(positions);
  }

  SECTION("Cluster of exact duplicates plus a few distinct outliers")
  {
    std::vector<Vec3> positions(20, Vec3(0, 0, 0));
    positions.emplace_back(5, 5, 5);
    positions.emplace_back(-5, -5, -5);
    positions.emplace_back(10, 0, 0);
    buildAndCheck(positions);
  }
}

TEMPLATE_TEST_CASE("Nested BVH: a BVHUnion over several TriMeshSDF objects nests each mesh's inner "
                   "PackedBVH inside the outer union PackedBVH and matches a brute-force min",
                   "[BVH][CSG][BVHUnion][Nested]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using BV   = BoundingVolumes::AABBT<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  using Tri = TriMeshSDF<T, Meta, K, W>;

  // Two distinct triangle meshes read from the in-repo fixtures. Each TriMeshSDF owns an inner
  // PackedBVH over SoA triangle groups -- these are the inner BVHs that the outer union BVH nests
  // over. Both are the same C++ type, which is all a BVHUnion needs.
  Pool       pool(hostMemoryResource());
  const auto dodec = Parser::readIntoTriangles<T, Meta>(dataPath("dodecahedron.stl"));
  const auto tetra = Parser::readIntoTriangles<T, Meta>(dataPath("tetrahedron.stl"));

  // Spread several translated mesh SDFs out so the outer union BVH has real structure to partition
  // and prune, rather than collapsing to a single leaf. Each copy is translated before its
  // TriMeshSDF is built.
  const std::vector<std::pair<std::vector<Triangle<T, Meta>>, Vec3>> placements = {
    {dodec, Vec3(0, 0, 0)},
    {dodec, Vec3(4, 0, 0)},
    {tetra, Vec3(0, 4, 0)},
    {tetra, Vec3(-4, -4, 2)},
  };

  std::vector<Tri> primitives;
  std::vector<BV>  boundingVolumes;

  for (const auto& [triangles, shift] : placements) {
    auto shifted = triangles;

    for (auto& triangle : shifted) {
      auto vertices = triangle.getVertexPositions();

      for (auto& v : vertices) {
        v = v + shift;
      }

      triangle.setVertexPositions(vertices);
    }

    primitives.emplace_back(shifted, pool, BVH::Construction::SAH, 2);
    boundingVolumes.push_back(primitives.back().computeBoundingVolume());
  }

  // Outer BVH: a PackedBVH over the TriMeshSDFs by value, each owning an inner PackedBVH in the same
  // pool -- a genuine two-level BVH hierarchy.
  const auto nestedUnion = BVHUnion<T, Tri, K>(pool, primitives, boundingVolumes);

  // The union value is the minimum over all primitives; BVH pruning can never discard the actual
  // nearest one, so the nested traversal must agree exactly with a brute-force min over the same
  // translated primitives.
  for (const auto& p : queryPoints<T>()) {
    T bruteMin = std::numeric_limits<T>::infinity();

    for (const auto& prim : primitives) {
      bruteMin = std::min(bruteMin, prim.signedDistance(p));
    }

    REQUIRE_THAT(nestedUnion.signedDistance(p), withinAbsT(bruteMin, traversalMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("TreeBVH::deepCopy: independent clone -- distinct nodes, shared primitives, "
                   "identical queries; the copies partition independently",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Face = DCEL::FaceT<T, Meta>;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"), pool);

  BVH::PrimAndBVList<Face, AABB> primsAndBVs;
  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const auto& f = mesh.getFace(i);
    primsAndBVs.emplace_back(std::make_shared<const Face>(f), AABB(f.getAllVertexCoordinates(mesh)));
  }

  const FlatMeshSDF<T, Meta> flat(mesh, pool);
  const auto                 brute = [&flat](const Vec3T<T>& a_point) -> T { return flat.signedDistance(a_point); };

  // Pack a (partitioned) tree and query it the way MeshSDF does, comparing to the brute-force scan.
  const auto packAndCheck = [&](const auto& a_tree) {
    const auto  packed = a_tree->pack(pool);
    const auto& faces  = packed->getPrimitives();

    for (const auto& p : queryPoints<T>()) {
      T          state    = std::numeric_limits<T>::max();
      const auto evalLeaf = [&faces, &p, &mesh](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d = faces[a_offset + i].signedDistance(p, mesh);
          if (std::abs(d) < std::abs(a_state)) {
            a_state = d;
          }
        }
      };
      const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

      packed->pruneTraverse(p, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(brute(p), traversalMargin<T>()));
    }
  };

  SECTION("clone of an unpartitioned tree shares primitives; the copies then partition independently")
  {
    auto tree = std::make_shared<BVH::TreeBVH<T, Face, AABB, K>>(primsAndBVs);
    REQUIRE_FALSE(tree->isPartitioned());

    const auto clone = tree->deepCopy();
    REQUIRE(clone != nullptr);
    REQUIRE(clone.get() != tree.get());
    REQUIRE(clone->isPartitioned() == tree->isPartitioned());

    // Primitives are shared by handle, not cloned: same underlying Face objects, same order.
    // (std::as_const selects the public const getPrimitives() overload; the mutable one is protected.)
    const auto& treePrims  = std::as_const(*tree).getPrimitives();
    const auto& clonePrims = std::as_const(*clone).getPrimitives();
    REQUIRE(clonePrims.size() == treePrims.size());
    for (size_t i = 0; i < treePrims.size(); i++) {
      REQUIRE(clonePrims[i].get() == treePrims[i].get());
    }

    // Partition the two copies with different strategies. Each must build correctly, and neither
    // may disturb the other -- the guarantee the deleted shallow copy could not provide.
    tree->topDownSortAndPartition();
    clone->template bottomUpSortAndPartition<SFC::Morton>();

    REQUIRE(tree->isPartitioned());
    REQUIRE(clone->isPartitioned());
    packAndCheck(tree);
    packAndCheck(clone);
  }

  SECTION("clone of a partitioned tree duplicates the node hierarchy but shares primitives")
  {
    auto tree = std::make_shared<BVH::TreeBVH<T, Face, AABB, K>>(primsAndBVs);
    tree->topDownSortAndPartition();
    REQUIRE(tree->isPartitioned());

    const auto clone = tree->deepCopy();
    REQUIRE(clone != nullptr);
    REQUIRE(clone->isPartitioned());

    // Interior child nodes are brand-new objects, not aliases of the original's -- this is exactly
    // what a shallow copy would have failed to do (36 faces guarantee the root is interior).
    bool anyInterior = false;
    for (size_t k = 0; k < K; k++) {
      if (tree->getChildren()[k] != nullptr) {
        anyInterior = true;
        REQUIRE(clone->getChildren()[k] != nullptr);
        REQUIRE(clone->getChildren()[k].get() != tree->getChildren()[k].get());
      }
      else {
        REQUIRE(clone->getChildren()[k] == nullptr);
      }
    }
    REQUIRE(anyInterior);

    // Behaviorally identical, and independent: destroying the original leaves the clone fully valid
    // (its nodes are its own; the shared faces are kept alive by the mesh).
    packAndCheck(clone);
    tree.reset();
    packAndCheck(clone);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Device: a rebased PackedBVH traverses in a kernel and agrees with the host
// ─────────────────────────────────────────────────────────────────────────────

// One traversal per query point: the probe the host suite also runs, so the device executes exactly
// the code the host checks, with the BVH held by value as a kernel receives it.
template <class T, size_t K>
struct PackedBvhTraversalQuery
{
  EBGeometry::BVH::PackedBVH<T, BareTestPoint<T>, K> m_bvh;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return packedBvhTraversalProbe<T, K>(m_bvh, a_point);
  }
};

TEMPLATE_TEST_CASE("PackedBVH: a rebased view traverses on device and matches the host",
                   "[BVH][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  Pool pool(hostMemoryResource());

  std::vector<Vec3> positions;

  positions.reserve(64);

  for (int i = 0; i < 64; i++) {
    const T t = T(i);

    positions.emplace_back(std::sin(t) * t, std::cos(t) * t, T(0.2) * t);
  }

  std::vector<std::pair<Pnt, AABB>> flat;

  flat.reserve(positions.size());

  for (const auto& pos : positions) {
    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  const Packed bvh(pool, flat, size_t(4));

  pool.freeze();

  Pool         devicePool = Pool::mirror(pool, deviceTestResource());
  const Packed deviceView = bvh.rebasedView(devicePool);

  // The points spiral out to a radius of 63 in the xy-plane and rise to z = 12.6, so this grid
  // covers query points among them as well as well outside the cloud.
  const auto points = queryGrid<T>(Vec3(T(-70), T(-70), T(-5)), Vec3(T(70), T(70), T(18)), 10);

  requireSameResults(evaluateOnDevice<T>(PackedBvhTraversalQuery<T, K>{deviceView}, points),
                     evaluateOnHost<T>(PackedBvhTraversalQuery<T, K>{bvh}, points));
}

// ─────────────────────────────────────────────────────────────────────────────
// Device: rebased FlatMeshSDF/MeshSDF/TriMeshSDF evaluate in a kernel and agree with the host
// ─────────────────────────────────────────────────────────────────────────────

// One signed distance per query point. Any of FlatMeshSDF, MeshSDF, TriMeshSDF, held by value as a
// kernel receives it.
template <class T, class SDF>
struct SignedDistanceQuery
{
  SDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_sdf.signedDistance(a_point);
  }
};

// The device-callable bounding box every mesh SDF provides: query i < 3 returns the upper corner's
// component i, and query i >= 3 the lower corner's component i - 3.
template <class T, class SDF>
struct BoundingBoxQuery
{
  SDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const int& a_i) const noexcept
  {
    const auto box = m_sdf.computeBoundingVolume();

    return (a_i < 3) ? box.getHighCorner()[static_cast<size_t>(a_i)] : box.getLowCorner()[static_cast<size_t>(a_i - 3)];
  }
};

TEMPLATE_TEST_CASE("FlatMeshSDF/MeshSDF/TriMeshSDF: rebased copies evaluate on device and match the host",
                   "[BVH][FlatMeshSDF][MeshSDF][TriMeshSDF][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

  const FlatMeshSDF<T, Meta>      flat(mesh, pool);
  const MeshSDF<T, Meta, K>       meshSDF(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, Meta, K, W> triSDF(mesh, pool, BVH::Construction::SAH, 2);

  pool.freeze();

  Pool devicePool = Pool::mirror(pool, deviceTestResource());

  // The dodecahedron is centred at the origin with vertices within 1.62 of it on each axis, so this
  // grid covers points inside, outside, and near the surface.
  const auto points  = queryGrid<T>(Vec3T<T>(T(-2), T(-2), T(-2)), Vec3T<T>(T(2), T(2), T(2)), 12);
  const auto corners = std::vector<int>{0, 1, 2, 3, 4, 5};

  const auto check = [&](const auto& a_sdf) {
    using SDF = std::decay_t<decltype(a_sdf)>;

    const SDF view = a_sdf.rebasedView(devicePool);

    requireSameResults(evaluateOnDevice<T>(SignedDistanceQuery<T, SDF>{view}, points),
                       evaluateOnHost<T>(SignedDistanceQuery<T, SDF>{a_sdf}, points));
    requireSameResults(evaluateOnDevice<T>(BoundingBoxQuery<T, SDF>{view}, corners),
                       evaluateOnHost<T>(BoundingBoxQuery<T, SDF>{a_sdf}, corners));
  };

  check(flat);
  check(meshSDF);
  check(triSDF);
}

TEST_CASE("Default K and W are 4 whatever the compiler flags; host-tuned values follow the SIMD flags", "[BVH]")
{
  // The defaults must never depend on ISA macros: a type spelled with them has to be the same type in
  // every translation unit and in both passes of a GPU compile.
  static_assert(BVH::DefaultBranchingRatio<float>() == 4 && BVH::DefaultBranchingRatio<double>() == 4);
  static_assert(TriangleSoA::DefaultWidth<float>() == 4 && TriangleSoA::DefaultWidth<double>() == 4);
  static_assert(PointSoA::DefaultWidth<float>() == 4 && PointSoA::DefaultWidth<double>() == 4);

#if defined(__AVX512F__)
  constexpr size_t hostFloat  = 16;
  constexpr size_t hostDouble = 8;
#elif defined(__AVX__)
  constexpr size_t hostFloat  = 8;
  constexpr size_t hostDouble = 4;
#else
  constexpr size_t hostFloat  = 4;
  constexpr size_t hostDouble = 4;
#endif

  static_assert(BVH::HostBranchingRatio<float>() == hostFloat && BVH::HostBranchingRatio<double>() == hostDouble);
  static_assert(TriangleSoA::HostWidth<float>() == hostFloat && TriangleSoA::HostWidth<double>() == hostDouble);
  static_assert(PointSoA::HostWidth<float>() == hostFloat && PointSoA::HostWidth<double>() == hostDouble);

  SUCCEED();
}
