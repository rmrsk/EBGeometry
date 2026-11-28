// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test suite for building BVHs: TreeBVH's partitioners and PackedBVH's direct builds (top-down, bottom-up along a
// space-filling curve, cluster SAH), checked against brute-force scans and against each other.

#include "EBGeometry.hpp"
#include "TestBVHHelpers.hpp"
#include "TestConstructions.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestUtilities.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestBVH;
using Catch::Matchers::WithinRel;

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

// A branching factor at namespace scope, so that a lambda nested in another reads it without a capture (GCC and
// Clang disagree about capturing a local constexpr there).
constexpr size_t K4 = 4;

} // namespace

TEMPLATE_TEST_CASE("TreeBVH/PackedBVH: signedDistance agrees with the brute-force mesh scan, for "
                   "every partitioning strategy",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

  using Face = DCEL::FaceT<T>;

  BVH::PrimAndBVList<Face, AABB> primsAndBVs;
  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const auto& f = mesh.getFace(i);
    primsAndBVs.emplace_back(std::make_shared<const Face>(f), AABB(f.getAllVertexCoordinates(mesh)));
  }
  REQUIRE(primsAndBVs.size() == 36);

  const FlatMeshSDF<T> flat(mesh, pool);
  const auto           brute = [&flat](const Vec3T<T>& a_point) -> T { return flat.signedDistance(a_point); };

  // Every value of BVH::Construction is exercised through one BVH::TreeBVH built the same way MeshSDF
  // builds one internally (see MeshDistanceFunctionsDetail::buildDCELTreeBVH), so this covers the
  // partitioning strategies directly rather than only through the higher-level SDF wrapper.
  auto buildAndCheck = [&](const char* a_label, auto&& a_partitionFunction) {
    INFO("Build strategy: " << a_label);

    auto tree = std::make_shared<BVH::TreeBVH<T, Face, AABB, K>>(primsAndBVs);
    a_partitionFunction(*tree);

    const auto packed = tree->pack(pool);
    REQUIRE(packed.getPrimitives().size() == 36);

    // PackedBVH has no signedDistance() of its own -- callers build their own thin wrapper
    // around traverse(), exactly as MeshSDF/TriMeshSDF::signedDistance() do.
    const auto& faces = packed.getPrimitives();

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

      packed.traverse(BVH::PointQuery{p}, state, evalLeaf, pruneDist2);

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

// Regression test for the PackedBVH::appendTreeLeaf O(N^2) build bug: a per-leaf
// reserve(size + leafSize) that defeated std::vector's geometric growth, reallocating the whole
// primitive buffer on every leaf. Both build paths that append leaves one at a time -- the direct
// top-down PackedBVH constructor and TreeBVH::pack() -- hit it, so this exercises both and checks
// nearest-neighbor correctness against brute force. The primitive count is deliberately large: a
// reintroduced quadratic would blow well past the unit-test timeout here (minutes), while the
// linear build stays well under a second. Double only -- the append path is precision-independent,
// and one heavy build is enough of a guard.
TEST_CASE("PackedBVH: a large per-leaf build stays linear and correct", "[BVH][traverse][regression]")
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

  // Path 1: PackedBVH::buildTopDown(), which appends each leaf via appendTreeLeaf
  // as it linearizes.
  std::vector<std::pair<Pnt, AABB>> flat;
  flat.reserve(N);
  for (const auto& pos : positions) {
    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }
  const Packed directBVH = Packed::build(pool, std::move(flat), BVH::Construction::CentroidSplit);

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
  REQUIRE(packedBVH.getPrimitives().size() == N);

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

    a_bvh.traverse(BVH::PointQuery{a_query}, state, evalLeaf, pruneDist2);
    return state;
  };

  for (const auto& q : queryPoints<T>()) {
    T brute2 = std::numeric_limits<T>::max();
    for (const auto& pos : positions) {
      brute2 = std::min(brute2, (pos - q).length2());
    }

    REQUIRE_THAT(nearest2(directBVH, q), withinAbsT(brute2, traversalMargin<T>()));
    REQUIRE_THAT(nearest2(packedBVH, q), withinAbsT(brute2, traversalMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("TreeBVH: every node's box is exactly the union of its children's or primitives' boxes, "
                   "after both builds and after refit",
                   "[BVH][refit]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;
  using Tree = BVH::TreeBVH<T, Pnt, AABB, 4>;

  std::vector<std::shared_ptr<Pnt>> handles;
  for (int i = 0; i < 200; i++) {
    handles.emplace_back(std::make_shared<Pnt>(Pnt{Vec3(T(i % 7), T((i * 3) % 11), T((i * 5) % 13))}));
  }

  BVH::PrimAndBVList<Pnt, AABB> primsAndBVs;
  for (const auto& h : handles) {
    primsAndBVs.emplace_back(h, AABB(h->m_pos - Vec3::ones(), h->m_pos));
  }

  // Each node's box against the union of what it encloses, computed here with the list constructor.
  const auto checkUnions = [](const Tree& a_root) {
    std::vector<const Tree*> stack{&a_root};
    size_t                   numNodes = 0;

    while (!stack.empty()) {
      const Tree* node = stack.back();

      stack.pop_back();
      numNodes++;

      std::vector<AABB> parts;
      if (node->isLeaf()) {
        parts = node->getBoundingVolumes();
      }
      else {
        for (const auto& child : node->getChildren()) {
          parts.emplace_back(child->getBoundingVolume());
          stack.emplace_back(child.get());
        }
      }

      const AABB expected(parts);

      REQUIRE(node->getBoundingVolume().getLowCorner() == expected.getLowCorner());
      REQUIRE(node->getBoundingVolume().getHighCorner() == expected.getHighCorner());
    }

    REQUIRE(numNodes > 1);
  };

  const auto bvConstructor = [](const Pnt& a_p) noexcept -> AABB { return AABB(a_p.m_pos - Vec3::ones(), a_p.m_pos); };

  SECTION("top-down")
  {
    Tree tree(primsAndBVs);
    tree.topDownSortAndPartition();
    checkUnions(tree);

    for (auto& h : handles) {
      h->m_pos = h->m_pos * T(2) + Vec3::ones();
    }

    tree.refit(bvConstructor);
    checkUnions(tree);
  }

  SECTION("bottom-up")
  {
    Tree tree(primsAndBVs);
    tree.template bottomUpSortAndPartition<SFC::Morton>();
    checkUnions(tree);

    for (auto& h : handles) {
      h->m_pos = -h->m_pos;
    }

    tree.refit(bvConstructor);
    checkUnions(tree);
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
      REQUIRE(packed.getPrimitives().size() == a_positions.size());

      const auto& prims = packed.getPrimitives();

      // PackedBVH has no signedDistance() of its own -- do the nearest-neighbor search via
      // traverse(), whose State here is the running *squared* distance to the point cloud.
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

        packed.traverse(BVH::PointQuery{q}, state, evalLeaf, pruneDist2);

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

TEMPLATE_TEST_CASE("PackedBVH::build: each preset method builds the same tree as its named build, and a "
                   "zero leaf setting aborts",
                   "[BVH][Construction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using AABB   = BoundingVolumes::AABBT<T>;
  using Vec3   = Vec3T<T>;
  using Pnt    = BareTestPoint<T>;
  using Packed = BVH::PackedBVH<T, Pnt, 4>;
  using List   = std::vector<std::pair<Pnt, AABB>>;

  Pool pool(hostMemoryResource());

  List flat;
  for (int i = 0; i < 300; i++) {
    const Vec3 p(T((i * 7) % 17), T((i * 11) % 19), T((i * 13) % 23));

    flat.emplace_back(Pnt{p}, AABB(p, p + Vec3::ones()));
  }

  BVH::ConstructionOptions options = Packed::defaultConstructionOptions();
  options.maxLeafSize              = 5;
  options.targetLeafSize           = 6;
  options.cluster                  = BVH::ClusterSpec{7};

  const auto stopCrit = [](const BVH::TreeBVH<T, Pnt, AABB, 4>& a_node) noexcept {
    return a_node.getPrimitives().size() <= 5;
  };

  SECTION("every method")
  {
    REQUIRE(sameNodes(Packed::build(pool, flat, BVH::Construction::SAH, options),
                      Packed::buildTopDown(pool, flat, BVH::BinnedSAHPartitioner<T, Pnt, AABB, 4>, stopCrit)));
    REQUIRE(sameNodes(Packed::build(pool, flat, BVH::Construction::CentroidSplit, options),
                      Packed::buildTopDown(pool, flat, BVH::BVCentroidPartitioner<T, Pnt, AABB, 4>, stopCrit)));
    REQUIRE(sameNodes(Packed::build(pool, flat, BVH::Construction::MidpointSplit, options),
                      Packed::buildTopDown(pool, flat, BVH::MidpointPartitioner<T, Pnt, AABB, 4>, stopCrit)));
    REQUIRE(sameNodes(Packed::build(pool, flat, BVH::Construction::ClusterSAH, options),
                      Packed::buildClusterSAH(pool, flat, BVH::ClusterSpec{7})));
    REQUIRE(sameNodes(Packed::build(pool, flat, BVH::Construction::Morton, options),
                      Packed::template buildBottomUp<SFC::Morton>(pool, flat, 6)));
    REQUIRE(sameNodes(Packed::build(pool, flat, BVH::Construction::Nested, options),
                      Packed::template buildBottomUp<SFC::Nested>(pool, flat, 6)));
    REQUIRE(sameNodes(Packed::build(pool, flat, BVH::Construction::Hilbert, options),
                      Packed::template buildBottomUp<SFC::Hilbert>(pool, flat, 6)));
  }

  SECTION("the default options are the default leaf predicate's")
  {
    REQUIRE(sameNodes(Packed::build(pool, flat, BVH::Construction::CentroidSplit),
                      Packed::buildTopDown(pool, flat, BVH::BVCentroidPartitioner<T, Pnt, AABB, 4>)));
  }

  SECTION("a zero setting for the chosen method aborts")
  {
    REQUIRE(abortsWith(
      [&flat] {
        Pool p(hostMemoryResource());

        (void)Packed::build(p, flat, BVH::Construction::SAH, BVH::ConstructionOptions{0, 4, BVH::ClusterSpec{}});
      },
      "PackedBVH: ConstructionOptions::maxLeafSize must be positive"));

    REQUIRE(abortsWith(
      [&flat] {
        Pool p(hostMemoryResource());

        (void)Packed::build(p, flat, BVH::Construction::Morton, BVH::ConstructionOptions{4, 0, BVH::ClusterSpec{}});
      },
      "PackedBVH: ConstructionOptions::targetLeafSize must be positive"));
  }
}

TEMPLATE_TEST_CASE("PackedBVH::buildBottomUp (no TreeBVH) matches brute-force "
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

    const BVH::PackedBVH<T, Pnt, K> packed =
      BVH::PackedBVH<T, Pnt, K>::buildBottomUp(pool, std::move(primsAndBVs), targetLeafSize);

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

      packed.traverse(BVH::PointQuery{q}, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(positions, q), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH::buildBottomUp and buildTopDown with the MidpointPartitioner handle degenerate-axis "
                   "primitive sets, without an empty group",
                   "[BVH][DirectSFCBuild][DirectTopDownBuild]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;
  using Node = BVH::TreeBVH<T, Pnt, AABB, K4>;

  Pool pool(hostMemoryResource());

  // Builds over the points bottom-up and, unless a_bottomUpOnly, top-down with the MidpointPartitioner, and checks
  // each against a brute-force nearest-point scan.
  auto buildAndCheck = [&](const std::vector<Vec3>& a_positions, const bool a_bottomUpOnly = false) {
    std::vector<std::pair<Pnt, AABB>> primsAndBVs;
    primsAndBVs.reserve(a_positions.size());
    for (const auto& pos : a_positions) {
      primsAndBVs.emplace_back(Pnt{pos}, AABB(pos, pos));
    }

    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K4; };

    std::vector<BVH::PackedBVH<T, Pnt, K4>> builds{
      BVH::PackedBVH<T, Pnt, K4>::buildBottomUp(pool, primsAndBVs, size_t(4))};

    if (!a_bottomUpOnly) {
      builds.push_back(BVH::PackedBVH<T, Pnt, K4>::buildTopDown(
        pool, primsAndBVs, BVH::MidpointPartitioner<T, Pnt, AABB, K4>, stopCrit));
    }

    for (const auto& packed : builds) {
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

        packed.traverse(BVH::PointQuery{q}, state, evalLeaf, pruneDist2);

        REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(a_positions, q), traversalMargin<T>()));
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

  SECTION("Single primitive (root is itself a leaf, no internal nodes at all)")
  {
    const std::vector<Vec3> positions{Vec3(1, 2, 3)};
    buildAndCheck(positions, true);
  }
}

// A BVH whose primitive *is* a uint32_t index into a caller-owned array is the supported way to
// build an indexed BVH: PackedBVH<T, uint32_t, K> stores four bytes per primitive and the leaf
// callback resolves the index against whatever array the caller owns. Because uint32_t is just an
// ordinary primitive type, every build path accepts it -- this checks buildBottomUp() and
// TreeBVH::pack(), the latter being the one an index could *not* travel through while the primitive
// array was governed by a storage policy. Both must agree exactly -- bit-for-bit, not just to a
// tolerance -- with a BVH packing the primitives themselves over identical bounding volumes.
TEMPLATE_TEST_CASE("PackedBVH::buildBottomUp -- a uint32_t-index BVH agrees exactly "
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

  const BVH::PackedBVH<T, Pnt, K>      valueBVH = BVH::PackedBVH<T, Pnt, K>::buildBottomUp(pool, valuePrims, size_t(7));
  const BVH::PackedBVH<T, uint32_t, K> indexBVH =
    BVH::PackedBVH<T, uint32_t, K>::buildBottomUp(pool, indexPrims, size_t(7));

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
  REQUIRE(packedIndexBVH.getPrimitives().size() == owner.size());

  // Four bytes per primitive, and still trivially copyable -- so an indexed BVH mirrors to a device
  // exactly like any other.
  static_assert(sizeof(indexBVH.getPrimitives()[0]) == 4);
  static_assert(std::is_trivially_copyable_v<BVH::PackedBVH<T, uint32_t, K>>);

  const auto& valuePrimArray  = valueBVH.getPrimitives();
  const auto& indexPrimArray  = indexBVH.getPrimitives();
  const auto& packedPrimArray = packedIndexBVH.getPrimitives();

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

    valueBVH.traverse(BVH::PointQuery{q}, valueState, valueEval, pruneDist2);
    indexBVH.traverse(BVH::PointQuery{q}, indexState, indexEval, pruneDist2);
    packedIndexBVH.traverse(BVH::PointQuery{q}, packedState, packedEval, pruneDist2);

    T bruteMin2 = std::numeric_limits<T>::max();

    for (const auto& pnt : owner) {
      bruteMin2 = std::min(bruteMin2, (pnt.m_pos - q).length2());
    }

    REQUIRE(indexState == valueState);
    REQUIRE(packedState == valueState);
    REQUIRE_THAT(indexState, withinAbsT(bruteMin2, traversalMargin<T>()));
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
      const BVH::PackedBVH<T, Vec3, K> bvh = BVH::PackedBVH<T, Vec3, K>::buildTopDown(pool, primsAndBVs, lopsided);

      (void)bvh;
    },
    "BVH::PackedBVH: the partitioner returned an empty partition (1 of 4, from 10 primitives)"));
}

TEMPLATE_TEST_CASE("BVH builders: a cluster of points whose extent is subnormal builds and queries exactly",
                   "[BVH][traverse]",
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
    BVH::PackedBVH<T, Vec3, K>::buildTopDown(pool, primsAndBVs, BVH::BinnedSAHPartitioner<T, Vec3, AABB, K>),
    BVH::PackedBVH<T, Vec3, K>::buildTopDown(pool, primsAndBVs, BVH::BinnedSAHPartitioner<T, Vec3, AABB, K, true>),
    BVH::PackedBVH<T, Vec3, K>::buildClusterSAH(pool, primsAndBVs, BVH::ClusterSpec{2}),
    BVH::PackedBVH<T, Vec3, K>::buildTopDown(pool, primsAndBVs, BVH::MidpointPartitioner<T, Vec3, AABB, K>),
    BVH::PackedBVH<T, Vec3, K>::buildBottomUp(pool, primsAndBVs, K)};

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

      bvh.traverse(
        BVH::PointQuery{q},
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

TEMPLATE_TEST_CASE("TreeBVH::bottomUpSortAndPartition: the target leaf size picks the fewest power-of-K leaves",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;
  constexpr size_t N = 1000;

  using Tree = BVH::TreeBVH<T, Vec3, AABB, K>;

  BVH::PrimAndBVList<Vec3, AABB> list;

  for (size_t i = 0; i < N; i++) {
    const Vec3 x(T(i % 11), T((i / 11) % 13), T(i / 143));

    list.emplace_back(std::make_shared<const Vec3>(x), AABB(x, x));
  }

  // The leaf sizes of a tree, and the depth of each leaf.
  std::function<void(const Tree&, size_t, std::vector<size_t>&, std::vector<size_t>&)> leaves =
    [&](const Tree& a_node, const size_t a_depth, std::vector<size_t>& a_sizes, std::vector<size_t>& a_depths) {
      if (a_node.isLeaf()) {
        a_sizes.push_back(a_node.getPrimitives().size());
        a_depths.push_back(a_depth);

        return;
      }

      for (const auto& child : a_node.getChildren()) {
        leaves(*child, a_depth + 1, a_sizes, a_depths);
      }
    };

  // The deepest tree allowed has K^4 = 256 <= N leaves.
  constexpr size_t maxLeaves = K * K * K * K;

  for (const size_t target : {size_t(1),
                              size_t(3),
                              size_t(4),
                              size_t(5),
                              size_t(15),
                              size_t(16),
                              size_t(62),
                              size_t(63),
                              size_t(64),
                              size_t(250),
                              size_t(999),
                              size_t(1000),
                              size_t(5000)}) {
    auto tree = std::make_shared<Tree>(list);

    tree->template bottomUpSortAndPartition<SFC::Morton>(target);

    std::vector<size_t> sizes;
    std::vector<size_t> depths;

    leaves(*tree, 0, sizes, depths);

    const size_t numLeaves = sizes.size();
    const size_t largest   = *std::max_element(sizes.begin(), sizes.end());

    INFO("target = " << target << ", leaves = " << numLeaves << ", largest = " << largest);

    // Every leaf is on one level, so the leaf count is a power of K, and no deeper than N allows.
    REQUIRE(std::all_of(depths.begin(), depths.end(), [&](const size_t d) { return d == depths.front(); }));
    REQUIRE(numLeaves <= maxLeaves);
    REQUIRE(std::accumulate(sizes.begin(), sizes.end(), size_t(0)) == N);

    // The leaves meet the target unless the tree is as deep as it may go ...
    REQUIRE((largest <= target || numLeaves == maxLeaves));

    // ... and one level fewer would not have met it.
    if (numLeaves > 1) {
      REQUIRE((N + numLeaves / K - 1) / (numLeaves / K) > target);
    }
  }

  // The default target, 1, is the deepest tree.
  auto deepest = std::make_shared<Tree>(list);

  deepest->template bottomUpSortAndPartition<SFC::Morton>();

  std::vector<size_t> sizes;
  std::vector<size_t> depths;

  leaves(*deepest, 0, sizes, depths);

  REQUIRE(sizes.size() == maxLeaves);

  // A zero target is a caller error, rejected in every build.
  REQUIRE(abortsWith(
    [&] {
      auto tree = std::make_shared<Tree>(list);

      tree->template bottomUpSortAndPartition<SFC::Morton>(0);
    },
    "TreeBVH::bottomUpSortAndPartition: the target leaf size must be positive"));
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
      Pool pool(hostMemoryResource());
      (void)Packed::buildBottomUp(pool, List{}, size_t(4));
    },
    "PackedBVH: the SFC build needs at least one primitive"));

  REQUIRE(abortsWith(
    [&prims] {
      Pool pool(hostMemoryResource());
      (void)Packed::buildBottomUp(pool, prims, size_t(0));
    },
    "PackedBVH: the SFC build's target leaf size must be positive (0)"));

  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());
      (void)Packed::build(pool, List{}, BVH::Construction::CentroidSplit);
    },
    "PackedBVH: the top-down build needs at least one primitive"));

  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());
      (void)Packed::buildClusterSAH(pool, List{}, BVH::ClusterSpec{});
    },
    "PackedBVH: the ClusterSAH build needs at least one primitive"));

  REQUIRE(abortsWith(
    [&prims] {
      Pool pool(hostMemoryResource());
      (void)Packed::buildClusterSAH(pool, prims, BVH::ClusterSpec{0});
    },
    "PackedBVH: ClusterSpec::maxClusterSize must be positive (0)"));
}

TEMPLATE_TEST_CASE("PackedBVH::buildClusterSAH -- a uint32_t-index BVH agrees exactly "
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

  // Same construction as the SFC-build case above, but through buildClusterSAH(), which
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

  const BVH::PackedBVH<T, Pnt, K>      valueBVH = BVH::PackedBVH<T, Pnt, K>::buildClusterSAH(pool, valuePrims, spec);
  const BVH::PackedBVH<T, uint32_t, K> indexBVH =
    BVH::PackedBVH<T, uint32_t, K>::buildClusterSAH(pool, indexPrims, spec);

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

    valueBVH.traverse(BVH::PointQuery{q}, valueState, valueEval, pruneDist2);
    indexBVH.traverse(BVH::PointQuery{q}, indexState, indexEval, pruneDist2);

    T bruteMin2 = std::numeric_limits<T>::max();

    for (const auto& pnt : owner) {
      bruteMin2 = std::min(bruteMin2, (pnt.m_pos - q).length2());
    }

    REQUIRE(indexState == valueState);
    REQUIRE_THAT(indexState, withinAbsT(bruteMin2, traversalMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PackedBVH::buildBottomUp accepts an explicit SFC curve "
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

  const BVH::PackedBVH<T, Pnt, K> packed =
    BVH::PackedBVH<T, Pnt, K>::template buildBottomUp<SFC::Nested>(pool, std::move(primsAndBVs), size_t(6));

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

    packed.traverse(BVH::PointQuery{q}, state, evalLeaf, pruneDist2);

    REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(positions, q), traversalMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PackedBVH::buildBottomUp puts every primitive in exactly one leaf, "
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

    const BVH::PackedBVH<T, uint32_t, K> packed =
      BVH::PackedBVH<T, uint32_t, K>::buildBottomUp(pool, std::move(primsAndBVs), a_targetLeafSize);

    const auto&           prims = packed.getPrimitives();
    std::vector<unsigned> visits(a_numPrims, 0U);

    const auto evalLeaf = [&prims, &visits](int&, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        visits[prims[a_offset + i]]++;
      }
    };
    const auto noPrune = [](const int&) noexcept -> T { return std::numeric_limits<T>::max(); };

    int state = 0;
    packed.traverse(BVH::PointQuery{Vec3::zeros()}, state, evalLeaf, noPrune);

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
  using Tri  = Triangle<T>;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("tetrahedron.stl"), pool);
  REQUIRE(mesh.numFaces() == 4);

  const auto triangles = Parser::readIntoTriangles<T>(dataPath("tetrahedron.stl"));
  REQUIRE(triangles.size() == 4);

  const FlatMeshSDF<T> flat(mesh, pool);
  const auto           brute = [&flat](const Vec3T<T>& a_point) -> T { return flat.signedDistance(a_point); };

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
    REQUIRE(packed.getPrimitives().size() == 4);

    const auto& prims = packed.getPrimitives();

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

      packed.traverse(BVH::PointQuery{p}, state, evalLeaf, pruneDist2);

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

  // Remaining build methods go straight to PackedBVH, bypassing TreeBVH entirely -- Triangle<T>
  // (unlike DCEL::FaceT) has a genuine memberwise copy constructor -- see its class
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

      a_packed.traverse(BVH::PointQuery{p}, state, evalLeaf, pruneDist2);

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

  checkDirect("PackedBVH direct SFC-build (Morton)",
              BVH::PackedBVH<T, Tri, K>::buildBottomUp(pool, makeFlatPrims(), size_t(K)));

  checkDirect("PackedBVH direct top-down build (default BVCentroidPartitioner)",
              BVH::PackedBVH<T, Tri, K>::build(pool, makeFlatPrims(), BVH::Construction::CentroidSplit));

  {
    using Node          = BVH::TreeBVH<T, Tri, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    checkDirect("PackedBVH direct top-down build (SAH)",
                BVH::PackedBVH<T, Tri, K>::buildTopDown(
                  pool, makeFlatPrims(), BVH::BinnedSAHPartitioner<T, Tri, AABB, K>, stopCrit));
  }
}

TEMPLATE_TEST_CASE("PackedBVH::buildTopDown (no TreeBVH) matches "
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

      a_packed.traverse(BVH::PointQuery{q}, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(bruteForceNearest2(positions, q), traversalMargin<T>()));
    }
  };

  SECTION("Default partitioner (BVCentroidPartitioner), default leaf predicate")
  {
    const BVH::PackedBVH<T, Pnt, K> packed =
      BVH::PackedBVH<T, Pnt, K>::build(pool, makeFlatPrims(), BVH::Construction::CentroidSplit);
    checkPacked("Default", packed);
  }

  SECTION("SAH partitioner (BinnedSAHPartitioner), matching custom leaf predicate")
  {
    using Node          = BVH::TreeBVH<T, Pnt, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    const BVH::PackedBVH<T, Pnt, K> packed = BVH::PackedBVH<T, Pnt, K>::buildTopDown(
      pool, makeFlatPrims(), BVH::BinnedSAHPartitioner<T, Pnt, AABB, K>, stopCrit);
    checkPacked("SAH", packed);
  }

  SECTION("ClusterSAH partitioner (cluster primitives, then SAH over clusters)")
  {
    // Small maxClusterSize so several clusters form over the 125-point grid, exercising the
    // cluster-then-SAH path rather than collapsing to a single cluster.
    const BVH::PackedBVH<T, Pnt, K> packed =
      BVH::PackedBVH<T, Pnt, K>::buildClusterSAH(pool, makeFlatPrims(), BVH::ClusterSpec{size_t(4)});
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

        a_packed.traverse(BVH::PointQuery{q}, state, evalLeaf, pruneDist2);

        REQUIRE_THAT(state, withinAbsT((only - q).length2(), traversalMargin<T>()));
      }
    };

    using Node          = BVH::TreeBVH<T, Pnt, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };

    checkOne("Default", BVH::PackedBVH<T, Pnt, K>::build(pool, onePrim(), BVH::Construction::CentroidSplit));
    checkOne(
      "SAH",
      BVH::PackedBVH<T, Pnt, K>::buildTopDown(pool, onePrim(), BVH::BinnedSAHPartitioner<T, Pnt, AABB, K>, stopCrit));
    checkOne("ClusterSAH", BVH::PackedBVH<T, Pnt, K>::buildClusterSAH(pool, onePrim(), BVH::ClusterSpec{size_t(8)}));
  }
}

TEMPLATE_TEST_CASE("PackedBVH::buildTopDown agrees exactly with building via "
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

  const BVH::PackedBVH<T, Pnt, K> direct =
    BVH::PackedBVH<T, Pnt, K>::build(pool, std::move(flatPrims), BVH::Construction::CentroidSplit);

  REQUIRE(direct.getPrimitives().size() == viaTree.getPrimitives().size());

  const auto& directPrims = direct.getPrimitives();
  const auto& treePrims   = viaTree.getPrimitives();

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

    direct.traverse(BVH::PointQuery{q}, directState, directEval, pruneDist2);
    viaTree.traverse(BVH::PointQuery{q}, treeState, treeEval, pruneDist2);

    REQUIRE(directState == treeState);
  }
}

TEMPLATE_TEST_CASE("BVH::MidpointPartitioner: matches brute-force nearest-neighbor via both "
                   "TreeBVH::topDownSortAndPartition() and PackedBVH::buildTopDown()",
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

      a_packed.traverse(BVH::PointQuery{q}, state, evalLeaf, pruneDist2);

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
    checkPacked("Via TreeBVH", packed);
  }

  SECTION("Via PackedBVH::buildTopDown()")
  {
    std::vector<std::pair<Pnt, AABB>> flatPrims;
    flatPrims.reserve(positions.size());
    for (const auto& pos : positions) {
      flatPrims.emplace_back(Pnt{pos}, AABB(pos, pos));
    }

    using Node          = BVH::TreeBVH<T, Pnt, AABB, K>;
    const auto stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };
    const BVH::PackedBVH<T, Pnt, K> packed = BVH::PackedBVH<T, Pnt, K>::buildTopDown(
      pool, std::move(flatPrims), BVH::MidpointPartitioner<T, Pnt, AABB, K>, stopCrit);
    checkPacked("Via buildTopDown()", packed);
  }
}

TEST_CASE("Default K and W are 4, except the triangle-group width under EBGEOMETRY_HOST_TUNED_DEFAULTS; "
          "host-tuned values follow the SIMD flags",
          "[BVH]")
{
  // The branching factor and the point-group width never depend on ISA macros: a type spelled with
  // them has to be the same type in every translation unit and in both passes of a GPU compile.
  static_assert(BVH::DefaultBranchingRatio<float>() == 4 && BVH::DefaultBranchingRatio<double>() == 4);
  static_assert(PointSoA::DefaultWidth<float>() == 4 && PointSoA::DefaultWidth<double>() == 4);

#if defined(EBGEOMETRY_HOST_TUNED_DEFAULTS)
  // A host-only build that opted in: the triangle-group width is the host-tuned value.
  static_assert(TriangleSoA::DefaultWidth<float>() == TriangleSoA::HostWidth<float>());
  static_assert(TriangleSoA::DefaultWidth<double>() == TriangleSoA::HostWidth<double>());
#else
  static_assert(TriangleSoA::DefaultWidth<float>() == 4 && TriangleSoA::DefaultWidth<double>() == 4);
#endif

#if defined(__AVX512F__)
  constexpr size_t hostFloat  = 16;
  constexpr size_t hostDouble = 8;
#elif defined(__AVX__)
  constexpr size_t hostFloat  = 8;
  constexpr size_t hostDouble = 4;
#else
  // SSE4.1, NEON (whose 16-byte registers hold four floats) and the scalar build.
  constexpr size_t hostFloat  = 4;
  constexpr size_t hostDouble = 4;
#endif

  static_assert(BVH::HostBranchingRatio<float>() == hostFloat && BVH::HostBranchingRatio<double>() == hostDouble);
  static_assert(TriangleSoA::HostWidth<float>() == hostFloat && TriangleSoA::HostWidth<double>() == hostDouble);
  static_assert(PointSoA::HostWidth<float>() == hostFloat && PointSoA::HostWidth<double>() == hostDouble);

  SUCCEED();
}
