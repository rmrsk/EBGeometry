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
#include <limits>
#include <memory>
#include <random>
#include <type_traits>
#include <utility>
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

    // PackedBVH has no signedDistance() of its own -- callers build their own thin wrapper
    // around pruneTraverse(), exactly as MeshSDF/TriMeshSDF::signedDistance() do.
    const auto& faces = packed->getPrimitives();

    for (const auto& p : queryPoints<T>()) {
      T state = std::numeric_limits<T>::max();

      const auto evalLeaf = [&faces, &p](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d = faces[a_offset + i]->signedDistance(p);
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

TEMPLATE_TEST_CASE("TriMeshSDF::getClosestTriangle reports the closest triangle's metadata and a "
                   "distance matching signedDistance()",
                   "[BVH][TriMesh][Meta]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // A soup of well-separated triangles (3 apart along x), each tagged with a distinct metadata value
  // so the closest-triangle query's returned metadata is unambiguous. Enough triangles to force
  // several BVH leaves/levels.
  constexpr int N = 12;

  std::vector<std::shared_ptr<Triangle<T, Meta>>> tris;
  for (int i = 0; i < N; i++) {
    const Vec3 base(T(3 * i), T(0), T(0));

    auto tri = std::make_shared<Triangle<T, Meta>>();
    tri->setVertexPositions({base + Vec3(0, 0, 0), base + Vec3(1, 0, 0), base + Vec3(0, 1, 0)});
    tri->setNormal(Vec3(0, 0, 1));
    tri->setVertexNormals({Vec3(0, 0, 1), Vec3(0, 0, 1), Vec3(0, 0, 1)});
    tri->setEdgeNormals({Vec3(0, 0, 1), Vec3(0, 0, 1), Vec3(0, 0, 1)});
    tri->setMetaData(static_cast<Meta>(100 + i));

    tris.emplace_back(tri);
  }

  for (const auto build : {BVH::Build::TopDown, BVH::Build::SAH}) {
    const TriMeshSDF<T, Meta, K, W> tri(tris, build, 2);

    for (int i = 0; i < N; i++) {
      const Vec3 q(T(3 * i) + T(0.25), T(0.25), T(0.2)); // unambiguously nearest to triangle i

      const auto closest = tri.getClosestTriangle(q);

      REQUIRE(closest.metaData == static_cast<Meta>(100 + i));
      REQUIRE_THAT(closest.signedDistance, withinAbsT(tri.signedDistance(q), traversalMargin<T>()));
    }
  }
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

  const auto packed = tree->pack();
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
        const T d2 = (prims[a_offset + i]->m_pos - q).length2();
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

// Regression test for the ValueStorage::appendTreeLeaf O(N^2) build bug: a per-leaf
// reserve(size + leafSize) that defeated std::vector's geometric growth, reallocating the whole
// primitive buffer on every leaf. Both ValueStorage build paths that append leaves one at a time
// -- the direct top-down PackedBVH constructor and TreeBVH::pack<ValueStorage>() -- hit it, so this
// exercises both and checks nearest-neighbor correctness against brute force. The primitive count
// is deliberately large: a reintroduced quadratic would blow well past the unit-test timeout here
// (minutes), while the linear build stays well under a second. Double only -- the append path is
// precision-independent, and one heavy build is enough of a guard.
TEST_CASE("PackedBVH (ValueStorage): a large per-leaf build stays linear and correct",
          "[BVH][pruneTraverse][regression]")
{
  using T     = double;
  using AABB  = BoundingVolumes::AABBT<T>;
  using Vec3  = Vec3T<T>;
  using Pnt   = BareTestPoint<T>;
  using Store = BVH::ValueStorage<Pnt>;

  constexpr size_t K = 4;
  constexpr size_t N = 60000;

  std::mt19937_64                   rng(20260709ULL);
  std::uniform_real_distribution<T> dist(T(0), T(1));

  std::vector<Vec3> positions;
  positions.reserve(N);
  for (size_t i = 0; i < N; i++) {
    positions.emplace_back(dist(rng), dist(rng), dist(rng));
  }

  using Packed = BVH::PackedBVH<T, Pnt, K, Store>;

  // Path 1: the direct top-down PackedBVH constructor (ValueStorage), which appends each leaf via
  // appendTreeLeaf as it linearizes.
  std::vector<std::pair<Pnt, AABB>> flat;
  flat.reserve(N);
  for (const auto& pos : positions) {
    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }
  const Packed directBVH(std::move(flat));

  // Path 2: TreeBVH::pack<ValueStorage>(), which appends each leaf the same way.
  BVH::PrimAndBVList<Pnt, AABB> primsAndBVs;
  primsAndBVs.reserve(N);
  for (const auto& pos : positions) {
    primsAndBVs.emplace_back(std::make_shared<Pnt>(Pnt{pos}), AABB(pos, pos));
  }
  auto tree = std::make_shared<BVH::TreeBVH<T, Pnt, AABB, K>>(primsAndBVs);
  tree->topDownSortAndPartition();
  const auto packedBVH = tree->template pack<Store>();

  REQUIRE(directBVH.getPrimitives().size() == N);
  REQUIRE(packedBVH->getPrimitives().size() == N);

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  const auto nearest2 = [&pruneDist2](const Packed& a_bvh, const Vec3& a_query) noexcept -> T {
    const auto& prims = a_bvh.getPrimitives();

    T state = std::numeric_limits<T>::max();

    const auto evalLeaf = [&prims, &a_query](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (Store::get(prims[a_offset + i]).m_pos - a_query).length2();
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

  // Keep mutable handles so the geometry can be *moved* after the BVH is built. The TreeBVH stores
  // these very objects as shared_ptr<const Pnt>, and pack()'s default SharedPtrStorage aliases the
  // same handles, so mutating a handle's position is exactly what a moving geometry does in practice
  // -- both representations then see the new positions through the shared pointers.
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

  const auto packed = tree->pack();
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
        const T d2 = (prims[a_offset + i]->m_pos - a_query).length2();

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

      const auto packed = tree->pack();
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
            const T d2 = (prims[a_offset + i]->m_pos - q).length2();
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

TEMPLATE_TEST_CASE("PackedBVH: BVH::ValueStorage and the default BVH::SharedPtrStorage agree exactly",
                   "[BVH][StoragePolicy]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  std::vector<Vec3> positions;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }

  BVH::PrimAndBVList<Pnt, AABB> primsAndBVs;
  for (const auto& pos : positions) {
    primsAndBVs.emplace_back(std::make_shared<Pnt>(Pnt{pos}), AABB(pos, pos));
  }

  auto tree = std::make_shared<BVH::TreeBVH<T, Pnt, AABB, K>>(primsAndBVs);
  tree->topDownSortAndPartition();

  const auto sharedPacked = tree->pack();
  const auto valuePacked  = tree->template pack<BVH::ValueStorage<Pnt>>();

  REQUIRE(sharedPacked != nullptr);
  REQUIRE(valuePacked != nullptr);
  REQUIRE(sharedPacked->getPrimitives().size() == positions.size());
  REQUIRE(valuePacked->getPrimitives().size() == positions.size());

  const auto& sharedPrims = sharedPacked->getPrimitives();
  const auto& valuePrims  = valuePacked->getPrimitives();

  // Both policies were built from the same TreeBVH, so a leaf's primitives must land in the same
  // order regardless of how they are stored -- not just agree as sets.
  for (size_t i = 0; i < positions.size(); i++) {
    REQUIRE(sharedPrims[i]->m_pos == valuePrims[i].m_pos);
  }

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  for (const auto& q : queryPoints<T>()) {
    T sharedState = std::numeric_limits<T>::max();
    T valueState  = std::numeric_limits<T>::max();

    const auto sharedEval = [&sharedPrims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (sharedPrims[a_offset + i]->m_pos - q).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };
    const auto valueEval = [&valuePrims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (valuePrims[a_offset + i].m_pos - q).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    sharedPacked->pruneTraverse(q, sharedState, sharedEval, pruneDist2);
    valuePacked->pruneTraverse(q, valueState, valueEval, pruneDist2);

    REQUIRE(sharedState == valueState);
  }
}

TEMPLATE_TEST_CASE("PackedBVH: BVH::ValueStorage stores primitives inline with no pointer indirection",
                   "[BVH][StoragePolicy]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  using Pnt = BareTestPoint<T>;

  // BVH::ValueStorage<P>::StorageType is P itself -- packing must not add any indirection.
  static_assert(std::is_same_v<typename BVH::ValueStorage<Pnt>::StorageType, Pnt>);
  static_assert(sizeof(typename BVH::ValueStorage<Pnt>::StorageType) == sizeof(Pnt));

  // BVH::SharedPtrStorage<P>::StorageType is a shared_ptr<const P>, matching pre-StoragePolicy
  // behavior -- this is the "no behavior change for existing callers" guarantee made concrete.
  static_assert(std::is_same_v<typename BVH::SharedPtrStorage<Pnt>::StorageType, std::shared_ptr<const Pnt>>);
  static_assert(sizeof(typename BVH::SharedPtrStorage<Pnt>::StorageType) == sizeof(std::shared_ptr<const Pnt>));
}

TEMPLATE_TEST_CASE("StoragePolicy: appendAliased genuinely appends for both policies (not only on the "
                   "single-call packWith path)",
                   "[BVH][StoragePolicy]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  // Build a fresh contiguous conversion buffer of three points offset by a_base.
  const auto makeBlock = [](T a_base) {
    auto block = std::make_shared<std::vector<Pnt>>();
    for (int i = 0; i < 3; i++) {
      block->push_back(Pnt{Vec3(a_base + T(i), a_base, a_base)});
    }
    return block;
  };

  // Two calls: the first into an empty destination (the fast path), the second into a non-empty
  // one. appendAliased must add to what is already there, not replace it -- the two policies have
  // to agree on this regardless of call count, or a future second call site would silently lose
  // the first block under ValueStorage while working under SharedPtrStorage.
  {
    std::vector<Pnt> dst;
    BVH::ValueStorage<Pnt>::appendAliased(dst, makeBlock(T(0)));
    BVH::ValueStorage<Pnt>::appendAliased(dst, makeBlock(T(10)));

    REQUIRE(dst.size() == 6);
    REQUIRE(dst[0].m_pos == Vec3(0, 0, 0));
    REQUIRE(dst[3].m_pos == Vec3(10, 10, 10));
  }
  {
    std::vector<std::shared_ptr<const Pnt>> dst;
    BVH::SharedPtrStorage<Pnt>::appendAliased(dst, makeBlock(T(0)));
    BVH::SharedPtrStorage<Pnt>::appendAliased(dst, makeBlock(T(10)));

    REQUIRE(dst.size() == 6);
    REQUIRE(dst[0]->m_pos == Vec3(0, 0, 0));
    REQUIRE(dst[3]->m_pos == Vec3(10, 10, 10));
  }
}

TEMPLATE_TEST_CASE("TriMeshSDF: default StoragePolicy is BVH::ValueStorage<TriSoA>; MeshSDF has no "
                   "StoragePolicy choice at all",
                   "[BVH][StoragePolicy]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  using Face     = DCEL::FaceT<T, Meta>;
  using TriAoSoA = TriangleAoSoA<T, Meta, W>;

  // MeshSDF always shares each packed face with the DCEL mesh's own face list (no copy, and no
  // StoragePolicy template parameter to override it): DCEL::FaceT's copy constructor deliberately
  // does not copy its cached 2D polygon embedding (m_poly2, needed by signedDistance()'s
  // point-in-face test), so a naive by-value copy is unsafe -- see FaceT's copy-constructor
  // documentation. TriMeshSDF's SoA groups have no such restriction (freshly built by packing,
  // shared with nothing else), so it defaults to storing them inline with no indirection. See
  // ImplemBVH.rst's "Storage policy" section for the full rationale.
  static_assert(std::is_same_v<typename MeshSDF<T, Meta, K>::Root::StorageType, std::shared_ptr<const Face>>);
  static_assert(std::is_same_v<typename TriMeshSDF<T, Meta, K, W>::Root::StorageType, TriAoSoA>);
}

TEMPLATE_TEST_CASE("TriMeshSDF: explicit BVH::SharedPtrStorage<TriAoSoA> agrees with the default "
                   "BVH::ValueStorage<TriAoSoA>",
                   "[BVH][Dodecahedron][StoragePolicy]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  using TriAoSoA = TriangleAoSoA<T, Meta, W>;

  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"));
  REQUIRE(mesh != nullptr);

  const TriMeshSDF<T, Meta, K, W>                                  defaultStorage(mesh, BVH::Build::SAH, 2);
  const TriMeshSDF<T, Meta, K, W, BVH::SharedPtrStorage<TriAoSoA>> sharedStorage(mesh, BVH::Build::SAH, 2);

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(sharedStorage.signedDistance(p), withinAbsT(defaultStorage.signedDistance(p), traversalMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Parser::readIntoTriangleBVH accepts an explicit StoragePolicy override",
                   "[BVH][Parser][StoragePolicy]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  using TriAoSoA = TriangleAoSoA<T, Meta, W>;

  const auto defaultTri = Parser::readIntoTriangleBVH<T, Meta, K, W>(dataPath("dodecahedron.stl"));
  const auto sharedTri =
    Parser::readIntoTriangleBVH<T, Meta, K, W, BVH::SharedPtrStorage<TriAoSoA>>(dataPath("dodecahedron.stl"));

  REQUIRE(defaultTri != nullptr);
  REQUIRE(sharedTri != nullptr);

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(sharedTri->signedDistance(p), withinAbsT(defaultTri->signedDistance(p), formatMargin<T>()));
  }
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

TEMPLATE_TEST_CASE("PackedBVH: copy is allowed and produces an independent deep copy under both "
                   "StoragePolicy choices; move is allowed too",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed      = BVH::PackedBVH<T, Pnt, K>;
  using PackedValue = BVH::PackedBVH<T, Pnt, K, BVH::ValueStorage<Pnt>>;

  static_assert(std::is_copy_constructible_v<Packed>);
  static_assert(std::is_copy_assignable_v<Packed>);
  static_assert(std::is_move_constructible_v<Packed>);
  static_assert(std::is_move_assignable_v<Packed>);
  static_assert(std::is_copy_constructible_v<PackedValue>);
  static_assert(std::is_copy_assignable_v<PackedValue>);
  static_assert(std::is_move_constructible_v<PackedValue>);
  static_assert(std::is_move_assignable_v<PackedValue>);

  std::vector<Vec3> positions;
  positions.reserve(5);
  for (int i = 0; i < 5; i++) {
    positions.emplace_back(T(i), T(i) * T(0.5), T(-i));
  }

  BVH::PrimAndBVList<Pnt, AABB> primsAndBVs;
  primsAndBVs.reserve(positions.size());
  for (const auto& pos : positions) {
    primsAndBVs.emplace_back(std::make_shared<Pnt>(Pnt{pos}), AABB(pos, pos));
  }

  auto tree = std::make_shared<BVH::TreeBVH<T, Pnt, AABB, K>>(primsAndBVs);
  tree->topDownSortAndPartition();

  auto original = tree->template pack<BVH::ValueStorage<Pnt>>();
  REQUIRE(original != nullptr);

  const PackedValue copy(*original); // copy constructor
  REQUIRE(copy.getPrimitives().size() == original->getPrimitives().size());

  // Destroy the source entirely -- the copy must remain fully valid and independent, proving the
  // copy constructor performed a real deep copy rather than aliasing the source's storage.
  original.reset();

  const Vec3  q(0, 0, 0);
  T           state    = std::numeric_limits<T>::max();
  const auto& prims    = copy.getPrimitives();
  const auto  evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = 0; i < a_count; i++) {
      const T d2 = (prims[a_offset + i].m_pos - q).length2();
      if (d2 < a_state) {
        a_state = d2;
      }
    }
  };
  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  copy.pruneTraverse(q, state, evalLeaf, pruneDist2);

  T bruteMin2 = std::numeric_limits<T>::max();
  for (const auto& pos : positions) {
    bruteMin2 = std::min(bruteMin2, (pos - q).length2());
  }

  REQUIRE_THAT(state, withinAbsT(bruteMin2, traversalMargin<T>()));
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
// Works for both BVH::SharedPtrStorage and BVH::ValueStorage primitives since StorageType's `->`
// vs `.` access is hidden behind the caller-supplied evalLeaf in each test.

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

    const BVH::PackedBVH<T, Pnt, K> packed(std::move(primsAndBVs), targetLeafSize);

    REQUIRE(packed.getPrimitives().size() == positions.size());

    const auto& prims = packed.getPrimitives();

    const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    for (const auto& q : queryPoints<T>()) {
      T          state    = std::numeric_limits<T>::max();
      const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d2 = (prims[a_offset + i]->m_pos - q).length2();
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

  auto buildAndCheck = [&](const std::vector<Vec3>& a_positions) {
    std::vector<std::pair<Pnt, AABB>> primsAndBVs;
    primsAndBVs.reserve(a_positions.size());
    for (const auto& pos : a_positions) {
      primsAndBVs.emplace_back(Pnt{pos}, AABB(pos, pos));
    }

    const BVH::PackedBVH<T, Pnt, K> packed(std::move(primsAndBVs), size_t(4));

    REQUIRE(packed.getPrimitives().size() == a_positions.size());

    const auto& prims      = packed.getPrimitives();
    const auto  pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    for (const auto& q : queryPoints<T>()) {
      T          state    = std::numeric_limits<T>::max();
      const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d2 = (prims[a_offset + i]->m_pos - q).length2();
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

TEMPLATE_TEST_CASE("PackedBVH: direct SFC-build constructor -- BVH::ValueStorage agrees with the "
                   "default BVH::SharedPtrStorage",
                   "[BVH][DirectSFCBuild][StoragePolicy]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  std::vector<Vec3> positions;
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }

  auto buildPrims = [&]() {
    std::vector<std::pair<Pnt, AABB>> primsAndBVs;
    primsAndBVs.reserve(positions.size());
    for (const auto& pos : positions) {
      primsAndBVs.emplace_back(Pnt{pos}, AABB(pos, pos));
    }
    return primsAndBVs;
  };

  const BVH::PackedBVH<T, Pnt, K>                         sharedStorage(buildPrims(), size_t(7));
  const BVH::PackedBVH<T, Pnt, K, BVH::ValueStorage<Pnt>> valueStorage(buildPrims(), size_t(7));

  REQUIRE(sharedStorage.getPrimitives().size() == positions.size());
  REQUIRE(valueStorage.getPrimitives().size() == positions.size());

  const auto& sharedPrims = sharedStorage.getPrimitives();
  const auto& valuePrims  = valueStorage.getPrimitives();

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  for (const auto& q : queryPoints<T>()) {
    T sharedState = std::numeric_limits<T>::max();
    T valueState  = std::numeric_limits<T>::max();

    const auto sharedEval = [&sharedPrims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (sharedPrims[a_offset + i]->m_pos - q).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };
    const auto valueEval = [&valuePrims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (valuePrims[a_offset + i].m_pos - q).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    sharedStorage.pruneTraverse(q, sharedState, sharedEval, pruneDist2);
    valueStorage.pruneTraverse(q, valueState, valueEval, pruneDist2);

    REQUIRE_THAT(valueState, withinAbsT(sharedState, traversalMargin<T>()));
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

  const BVH::PackedBVH<T, Pnt, K> packed(std::move(primsAndBVs), size_t(6), SFC::Nested{});

  REQUIRE(packed.getPrimitives().size() == positions.size());

  const auto& prims      = packed.getPrimitives();
  const auto  pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  for (const auto& q : queryPoints<T>()) {
    T          state    = std::numeric_limits<T>::max();
    const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (prims[a_offset + i]->m_pos - q).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    packed.pruneTraverse(q, state, evalLeaf, pruneDist2);

    REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(positions, q), traversalMargin<T>()));
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

  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("tetrahedron.stl"));
  REQUIRE(mesh != nullptr);
  REQUIRE(mesh->getFaces().size() == 4);

  const auto triangles = Parser::readIntoTriangles<T, Meta>(dataPath("tetrahedron.stl"));
  REQUIRE(triangles.size() == 4);

  const FlatMeshSDF<T, Meta> flat(mesh);
  const auto                 brute = [&flat](const Vec3T<T>& a_point) -> T { return flat.signedDistance(a_point); };

  BVH::PrimAndBVList<Tri, AABB> primsAndBVs;
  for (const auto& tri : triangles) {
    const auto&                 vp = tri->getVertexPositions();
    const std::vector<Vec3T<T>> verts{vp[0], vp[1], vp[2]};
    primsAndBVs.emplace_back(tri, AABB(verts));
  }
  REQUIRE(primsAndBVs.size() == 4);

  auto buildAndCheckTree = [&](const char* a_label, auto&& a_partitionFunction) {
    INFO("Build strategy: " << a_label);

    auto tree = std::make_shared<BVH::TreeBVH<T, Tri, AABB, K>>(primsAndBVs);
    a_partitionFunction(*tree);

    const auto packed = tree->pack();
    REQUIRE(packed != nullptr);
    REQUIRE(packed->getPrimitives().size() == 4);

    const auto& prims = packed->getPrimitives();

    for (const auto& p : queryPoints<T>()) {
      T state = std::numeric_limits<T>::max();

      const auto evalLeaf = [&prims, &p](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d = prims[a_offset + i]->signedDistance(p);
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
          const T d = prims[a_offset + i]->signedDistance(p);
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
      const auto&                 vp = tri->getVertexPositions();
      const std::vector<Vec3T<T>> verts{vp[0], vp[1], vp[2]};
      flatPrimsAndBVs.emplace_back(*tri, AABB(verts));
    }
    return flatPrimsAndBVs;
  };

  checkDirect("PackedBVH direct SFC-build (Morton)", BVH::PackedBVH<T, Tri, K>(makeFlatPrims(), size_t(K)));

  checkDirect("PackedBVH direct top-down build (default BVCentroidPartitioner)",
              BVH::PackedBVH<T, Tri, K>(makeFlatPrims()));

  {
    using Node          = BVH::TreeBVH<T, Tri, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    checkDirect("PackedBVH direct top-down build (SAH)",
                BVH::PackedBVH<T, Tri, K>(makeFlatPrims(), BVH::BinnedSAHPartitioner<T, Tri, AABB, K>, stopCrit));
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
          const T d2 = (prims[a_offset + i]->m_pos - q).length2();
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
    const BVH::PackedBVH<T, Pnt, K> packed(makeFlatPrims());
    checkPacked("Default", packed);
  }

  SECTION("SAH partitioner (BinnedSAHPartitioner), matching custom leaf predicate")
  {
    using Node          = BVH::TreeBVH<T, Pnt, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    const BVH::PackedBVH<T, Pnt, K> packed(makeFlatPrims(), BVH::BinnedSAHPartitioner<T, Pnt, AABB, K>, stopCrit);
    checkPacked("SAH", packed);
  }

  SECTION("ClusterSAH partitioner (cluster primitives, then SAH over clusters)")
  {
    // Small maxClusterSize so several clusters form over the 125-point grid, exercising the
    // cluster-then-SAH path rather than collapsing to a single cluster.
    const BVH::PackedBVH<T, Pnt, K> packed(makeFlatPrims(), BVH::ClusterSpec{size_t(4)});
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
            a_state = std::min(a_state, (prims[a_offset + i]->m_pos - q).length2());
          }
        };

        a_packed.pruneTraverse(q, state, evalLeaf, pruneDist2);

        REQUIRE_THAT(state, withinAbsT((only - q).length2(), traversalMargin<T>()));
      }
    };

    using Node          = BVH::TreeBVH<T, Pnt, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };

    checkOne("Default", BVH::PackedBVH<T, Pnt, K>(onePrim()));
    checkOne("SAH", BVH::PackedBVH<T, Pnt, K>(onePrim(), BVH::BinnedSAHPartitioner<T, Pnt, AABB, K>, stopCrit));
    checkOne("ClusterSAH", BVH::PackedBVH<T, Pnt, K>(onePrim(), BVH::ClusterSpec{size_t(8)}));
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
  const auto viaTree = tree->pack();

  const BVH::PackedBVH<T, Pnt, K> direct(std::move(flatPrims));

  REQUIRE(direct.getPrimitives().size() == viaTree->getPrimitives().size());

  const auto& directPrims = direct.getPrimitives();
  const auto& treePrims   = viaTree->getPrimitives();

  const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

  for (const auto& q : queryPoints<T>()) {
    T directState = std::numeric_limits<T>::max();
    T treeState   = std::numeric_limits<T>::max();

    const auto directEval = [&directPrims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (directPrims[a_offset + i]->m_pos - q).length2();
        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };
    const auto treeEval = [&treePrims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = (treePrims[a_offset + i]->m_pos - q).length2();
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
          const T d2 = (prims[a_offset + i]->m_pos - q).length2();
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

    const auto packed = tree->pack();
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
    const BVH::PackedBVH<T, Pnt, K> packed(std::move(flatPrims), BVH::MidpointPartitioner<T, Pnt, AABB, K>, stopCrit);
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
    const BVH::PackedBVH<T, Pnt, K> packed(std::move(flatPrims), BVH::MidpointPartitioner<T, Pnt, AABB, K>, stopCrit);

    REQUIRE(packed.getPrimitives().size() == a_positions.size());

    const auto& prims      = packed.getPrimitives();
    const auto  pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    for (const auto& q : queryPoints<T>()) {
      T          state    = std::numeric_limits<T>::max();
      const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d2 = (prims[a_offset + i]->m_pos - q).length2();
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
  using IF   = ImplicitFunction<T>;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  // Two distinct triangle meshes read from the in-repo fixtures. Each TriMeshSDF owns an inner
  // PackedBVH over SoA triangle groups (ValueStorage by default) -- these are the inner BVHs that
  // the outer union BVH nests over.
  const auto dodec = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"));
  const auto tetra = Parser::readIntoDCEL<T, Meta>(dataPath("tetrahedron.stl"));
  REQUIRE(dodec != nullptr);
  REQUIRE(tetra != nullptr);

  // Spread several translated mesh SDFs out so the outer union BVH has real structure to partition
  // and prune, rather than collapsing to a single leaf.
  const std::vector<std::pair<std::shared_ptr<DCEL::MeshT<T, Meta>>, Vec3>> placements = {
    {dodec, Vec3(0, 0, 0)},
    {dodec, Vec3(4, 0, 0)},
    {tetra, Vec3(0, 4, 0)},
    {tetra, Vec3(-4, -4, 2)},
  };

  std::vector<std::shared_ptr<IF>> primitives;
  std::vector<BV>                  boundingVolumes;
  primitives.reserve(placements.size());
  boundingVolumes.reserve(placements.size());

  for (const auto& [mesh, shift] : placements) {
    // Inner BVH: a TriMeshSDF holds its own PackedBVH over the mesh's SoA triangle groups.
    const auto tri = std::make_shared<TriMeshSDF<T, Meta, K, W>>(mesh, BVH::Build::SAH, 2);
    const BV   bv  = tri->computeBoundingVolume();

    // Store each mesh SDF as the common base type, translated into place. Its bounding volume for
    // the outer union is the mesh's own AABB shifted by the same amount.
    primitives.emplace_back(Translate<T>(tri, shift));
    boundingVolumes.emplace_back(bv.getLowCorner() + shift, bv.getHighCorner() + shift);
  }

  // Outer BVH: a PackedBVH over std::shared_ptr<const ImplicitFunction<T>>, each element pointing
  // at a TriMeshSDF that owns an inner PackedBVH -- a genuine two-level BVH hierarchy.
  const auto nestedUnion = BVHUnion<T, IF, BV, K>(primitives, boundingVolumes);
  REQUIRE(nestedUnion != nullptr);

  // The union value is the minimum over all primitives; BVH pruning can never discard the actual
  // nearest one, so the nested traversal must agree exactly with a brute-force min over the same
  // translated primitives.
  for (const auto& p : queryPoints<T>()) {
    T bruteMin = std::numeric_limits<T>::infinity();
    for (const auto& prim : primitives) {
      bruteMin = std::min(bruteMin, prim->value(p));
    }

    REQUIRE_THAT(nestedUnion->value(p), withinAbsT(bruteMin, traversalMargin<T>()));
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

  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"));
  REQUIRE(mesh != nullptr);

  BVH::PrimAndBVList<Face, AABB> primsAndBVs;
  for (const auto& f : mesh->getFaces()) {
    primsAndBVs.emplace_back(f, AABB(f->getAllVertexCoordinates()));
  }

  const FlatMeshSDF<T, Meta> flat(mesh);
  const auto                 brute = [&flat](const Vec3T<T>& a_point) -> T { return flat.signedDistance(a_point); };

  // Pack a (partitioned) tree and query it the way MeshSDF does, comparing to the brute-force scan.
  const auto packAndCheck = [&](const auto& a_tree) {
    const auto  packed = a_tree->pack();
    const auto& faces  = packed->getPrimitives();

    for (const auto& p : queryPoints<T>()) {
      T          state    = std::numeric_limits<T>::max();
      const auto evalLeaf = [&faces, &p](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d = faces[a_offset + i]->signedDistance(p);
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
