// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test suite for PackedBVH::traverse() and TreeBVH::traverse(): point and ray queries, the visit orders, the visit
// callback, user query types, and the order leaves are visited in.

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

// Nearest unsigned squared distance from each query point to a dodecahedron's faces, found through
// a PackedBVH with branching factor K. Used to pin PackedBVH::traverse()'s per-ISA
// child-distance paths against its scalar path: which of the two runs is chosen at compile time
// from (T, K) and the compiled ISA, so varying K over a build's SIMD and non-SIMD widths runs both
// within one binary.
//
// A pure minimum over the primitive set is deliberate. Its value cannot depend on the order leaves
// are visited in, so a disagreement between two K values is a real disagreement about the computed
// child distances (or about the pruning they drive), never a tie-break artifact of one traversal
// order versus another.
template <class T, size_t K>
std::vector<T>
nearestDist2PerQueryPoint(const DCEL::MeshT<T>&                                                a_mesh,
                          const BVH::PrimAndBVList<DCEL::FaceT<T>, BoundingVolumes::AABBT<T>>& a_primsAndBVs)
{
  using Face = DCEL::FaceT<T>;
  using AABB = BoundingVolumes::AABBT<T>;

  Pool pool(hostMemoryResource());

  auto tree = std::make_shared<BVH::TreeBVH<T, Face, AABB, K>>(a_primsAndBVs);
  tree->topDownSortAndPartition();

  const auto  packed = tree->pack(pool);
  const auto& prims  = packed.getPrimitives();

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

    packed.traverse(BVH::PointQuery{p}, state, evalLeaf, pruneDist2);

    result.push_back(state);
  }

  return result;
}

// The number of primitives an unbounded traverse() reaches from a_point, through a PackedBVH with
// branching factor K over a_primsAndBVs.
template <class T, size_t K>
size_t
primitivesReachedUnbounded(const BVH::PrimAndBVList<DCEL::FaceT<T>, BoundingVolumes::AABBT<T>>& a_primsAndBVs,
                           const Vec3T<T>&                                                      a_point)
{
  using Face = DCEL::FaceT<T>;
  using AABB = BoundingVolumes::AABBT<T>;

  Pool pool(hostMemoryResource());

  auto tree = std::make_shared<BVH::TreeBVH<T, Face, AABB, K>>(a_primsAndBVs);
  tree->topDownSortAndPartition();

  const auto packed = tree->pack(pool);

  size_t     reached    = 0;
  const auto evalLeaf   = [](size_t& a_reached, size_t, size_t a_count) noexcept { a_reached += a_count; };
  const auto pruneDist2 = [](const size_t&) noexcept -> T { return std::numeric_limits<T>::max(); };

  packed.traverse(BVH::PointQuery{a_point}, reached, evalLeaf, pruneDist2);

  return reached;
}

// Recursive references for the two traversals' visit order, with no stack of their own. Each records
// the primitive offset of every leaf it visits.
template <class T, class P, size_t K>
void
referenceNearestFirst(const BVH::PackedBVH<T, P, K>& a_bvh,
                      const uint32_t                 a_idx,
                      const Vec3T<T>&                a_query,
                      T&                             a_best,
                      std::vector<uint32_t>&         a_order)
{
  const auto  nodes = a_bvh.getNodes();
  const auto& node  = nodes[a_idx];

  if (node.isLeaf()) {
    a_order.push_back(node.getPrimitivesOffset());

    for (uint32_t i = 0; i < node.getNumPrimitives(); i++) {
      a_best = std::min(a_best, (a_bvh.getPrimitives()[node.getPrimitivesOffset() + i].m_pos - a_query).length2());
    }

    return;
  }

  // The children within the bound, nearest first; of equally near children, the higher slot first.
  std::vector<std::pair<T, size_t>> children;

  for (size_t k = 0; k < K; k++) {
    const T d2 = a_bvh.getDistanceToBoundingVolume2(nodes[node.getChildOffsets()[k]], a_query);

    if (d2 <= a_best) {
      children.emplace_back(d2, k);
    }
  }

  std::stable_sort(children.begin(), children.end(), [](const auto& a_lhs, const auto& a_rhs) {
    return a_lhs.first < a_rhs.first || (a_lhs.first == a_rhs.first && a_lhs.second > a_rhs.second);
  });

  // A child is tested again when it is reached, against the bound as it is then.
  for (size_t i = 0; i < children.size(); i++) {
    if (i > 0 && children[i].first > a_best) {
      break;
    }

    referenceNearestFirst(a_bvh, node.getChildOffsets()[children[i].second], a_query, a_best, a_order);
  }
}

template <class T, class P, size_t K>
void
referenceTraverse(const BVH::PackedBVH<T, P, K>& a_bvh,
                  const uint32_t                 a_idx,
                  const T                        a_key,
                  const Vec3T<T>&                a_query,
                  T&                             a_best,
                  std::vector<uint32_t>&         a_order)
{
  const auto  nodes = a_bvh.getNodes();
  const auto& node  = nodes[a_idx];

  if (!(a_key <= a_best)) {
    return;
  }

  if (node.isLeaf()) {
    a_order.push_back(node.getPrimitivesOffset());

    TraverseNearestLeaf<T>{a_bvh.getPrimitives(), a_query}(a_best, node.getPrimitivesOffset(), node.getNumPrimitives());

    return;
  }

  // The children sorted farthest first (stable insertion sort), then visited from the last, so the
  // nearest first and equal keys in reverse slot order.
  uint32_t child[K];
  T        key[K];

  for (size_t k = 0; k < K; k++) {
    const uint32_t c = node.getChildOffsets()[k];
    const T        d = a_bvh.getDistanceToBoundingVolume2(nodes[c], a_query);

    size_t j = k;

    while (j > 0 && key[j - 1] < d) {
      child[j] = child[j - 1];
      key[j]   = key[j - 1];
      j--;
    }

    child[j] = c;
    key[j]   = d;
  }

  for (size_t i = K; i-- > 0;) {
    referenceTraverse(a_bvh, child[i], key[i], a_query, a_best, a_order);
  }
}

// Rays for rayTestBoxes(), whose faces many share: half random, half axis-aligned from a grid face plane.
template <class T>
std::vector<std::pair<Vec3T<T>, Vec3T<T>>>
rayTestRays(const int a_count)
{
  using Vec3 = Vec3T<T>;

  std::mt19937                      rng(13);
  std::uniform_real_distribution<T> coord(T(-1), T(1));

  std::vector<std::pair<Vec3, Vec3>> rays;

  for (int r = 0; r < a_count; r++) {
    if (r % 2 == 1) {
      Vec3 d(coord(rng), coord(rng), coord(rng));

      rays.emplace_back(T(2) * Vec3(coord(rng), coord(rng), coord(rng)), d / d.length());
    }
    else {
      const size_t axis = static_cast<size_t>(r % 3);
      const T      sign = (r % 4 < 2) ? T(1) : T(-1);

      Vec3 o(T(0.125) * T(r % 8) - T(0.0625), T(0.125) * T((r / 8) % 8) - T(0.0625), T(-0.5));
      Vec3 d = Vec3::zeros();

      d[axis] = sign;
      o[axis] = T(-2) * sign;

      rays.emplace_back(o, d);
    }
  }

  return rays;
}

// A user query type with a composite key: the squared distance and, as a tie-break, nothing (0). It
// declares no Lanes, as a query type written today does.
template <class T>
struct TieKey
{
  T   m_d2;
  int m_tie;

  EBGEOMETRY_HOST_DEVICE
  bool
  operator<(const TieKey& a_other) const noexcept
  {
    return (m_d2 < a_other.m_d2) || (m_d2 == a_other.m_d2 && m_tie < a_other.m_tie);
  }

  EBGEOMETRY_HOST_DEVICE
  bool
  operator<=(const TieKey& a_other) const noexcept
  {
    return !(a_other < *this);
  }

  EBGEOMETRY_HOST_DEVICE
  bool
  operator>(const TieKey& a_other) const noexcept
  {
    return a_other < *this;
  }
};

template <class T>
struct TieBreakPointQuery
{
  using Scalar = T;
  using Key    = TieKey<T>;

  EBGeometry::BVH::PointQuery<T> m_point;

  template <size_t K>
  EBGEOMETRY_HOST_DEVICE
  EBGeometry::Array<Key, K>
  keys(const EBGeometry::BVH::ChildBoxes<T, K>& a_row) const noexcept
  {
    const auto d2 = m_point.keys(a_row);

    EBGeometry::Array<Key, K> keys;

    for (size_t k = 0; k < K; k++) {
      keys[k] = Key{d2[k], 0};
    }

    return keys;
  }

  template <size_t K>
  EBGEOMETRY_HOST_DEVICE
  Key
  key(const EBGeometry::BVH::ChildBoxes<T, K>& a_row, size_t a_k) const noexcept
  {
    return Key{m_point.key(a_row, a_k), 0};
  }

  EBGEOMETRY_HOST_DEVICE
  Key
  key(const BoundingVolumes::AABBT<T>& a_box) const noexcept
  {
    return Key{m_point.key(a_box), 0};
  }
};

// The number of primitives in a node's subtree, counted through its leaves (an independent recursion).
template <class Packed>
size_t
subtreePrimitives(const Packed& a_bvh, const uint32_t a_node)
{
  const auto  nodes = a_bvh.getNodes();
  const auto& node  = nodes[a_node];

  if (node.isLeaf()) {
    return node.getNumPrimitives();
  }

  size_t count = 0;

  for (const auto child : node.getChildOffsets()) {
    count += subtreePrimitives(a_bvh, child);
  }

  return count;
}

// Each node's depth (root 0), from the pre-order array.
template <class Packed>
std::vector<int>
nodeDepths(const Packed& a_bvh)
{
  const auto       nodes = a_bvh.getNodes();
  std::vector<int> depth(nodes.size(), 0);

  for (uint32_t i = 0; i < nodes.size(); i++) {
    if (!nodes[i].isLeaf()) {
      for (const auto child : nodes[i].getChildOffsets()) {
        depth[child] = depth[i] + 1;
      }
    }
  }

  return depth;
}

} // namespace

TEMPLATE_TEST_CASE("PackedBVH::traverse with a PointQuery: every compiled SIMD child-distance path agrees "
                   "bit-for-bit with the scalar path",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Face = DCEL::FaceT<T>;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

  BVH::PrimAndBVList<Face, AABB> primsAndBVs;

  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const auto& f = mesh.getFace(i);

    primsAndBVs.emplace_back(std::make_shared<const Face>(f), AABB(f.getAllVertexCoordinates(mesh)));
  }

  REQUIRE(primsAndBVs.size() == 36);

  // traverse() dispatches its per-child box-distance computation on (T, K) and the compiled
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

TEMPLATE_TEST_CASE("PackedBVH::traverse with a PointQuery: a NaN query coordinate gives the same child distances on "
                   "every path",
                   "[BVH][Dodecahedron][regression]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Face = DCEL::FaceT<T>;
  using Vec3 = Vec3T<T>;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

  BVH::PrimAndBVList<Face, AABB> primsAndBVs;

  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const auto& f = mesh.getFace(i);

    primsAndBVs.emplace_back(std::make_shared<const Face>(f), AABB(f.getAllVertexCoordinates(mesh)));
  }

  // The scalar path clamps a NaN axis to zero, so the child stays within any bound. The SIMD paths
  // used to take x86 max's operands the other way round, which returns the NaN instead, so every
  // child failed "dist2 <= bound" and an unbounded search reached nothing. K = 3 has no SIMD path in
  // any build; the other widths run whichever SIMD paths this binary compiled (see the test above).
  const T    nan      = std::numeric_limits<T>::quiet_NaN();
  const Vec3 points[] = {
    Vec3(nan, T(0.1), T(0.2)), Vec3(T(0.1), nan, T(0.2)), Vec3(T(0.1), T(0.2), nan), Vec3(nan, nan, nan)};

  for (const auto& p : points) {
    CHECK(primitivesReachedUnbounded<T, 3>(primsAndBVs, p) == primsAndBVs.size());
    CHECK(primitivesReachedUnbounded<T, 4>(primsAndBVs, p) == primsAndBVs.size());
    CHECK(primitivesReachedUnbounded<T, 8>(primsAndBVs, p) == primsAndBVs.size());
    CHECK(primitivesReachedUnbounded<T, 16>(primsAndBVs, p) == primsAndBVs.size());
  }
}

TEMPLATE_TEST_CASE("PackedBVH::traverse with a PointQuery: nearest-neighbor search over a primitive with no "
                   "signedDistance() matches a brute-force scan",
                   "[BVH][traverse]",
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
  REQUIRE(packed.getPrimitives().size() == positions.size());

  const auto& prims = packed.getPrimitives();

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

    packed.traverse(BVH::PointQuery{q}, state, evalLeaf, pruneDist2);

    T bruteMin2 = std::numeric_limits<T>::max();
    for (const auto& pos : positions) {
      bruteMin2 = std::min(bruteMin2, (pos - q).length2());
    }

    REQUIRE_THAT(state, withinAbsT(bruteMin2, traversalMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PackedBVH::traverse: a nearest-point search over template callbacks matches a brute-force scan",
                   "[BVH][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  std::mt19937                      rng(5);
  std::uniform_real_distribution<T> coord(T(-1), T(1));

  std::vector<std::pair<Pnt, AABB>> flat;

  for (int i = 0; i < 500; i++) {
    const Vec3 pos(coord(rng), coord(rng), coord(rng));

    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  std::vector<Vec3> queries;

  for (int i = 0; i < 100; i++) {
    queries.emplace_back(T(2) * coord(rng), T(2) * coord(rng), T(2) * coord(rng));
  }

  const auto check = [&](const auto& a_bvh) {
    for (const auto& q : queries) {
      T nearest = std::numeric_limits<T>::max();

      for (const auto& pb : flat) {
        nearest = std::min(nearest, (pb.first.m_pos - q).length2());
      }

      REQUIRE(customTraversalProbe(a_bvh, q) == nearest);
    }
  };

  Pool pool(hostMemoryResource());

  SECTION("K = 2")
  {
    check(BVH::PackedBVH<T, Pnt, 2>::buildTopDown(pool, flat, BVH::BinnedSAHPartitioner<T, Pnt, AABB, 2>));
  }

  SECTION("K = 4")
  {
    check(BVH::PackedBVH<T, Pnt, 4>::buildTopDown(pool, flat, BVH::BinnedSAHPartitioner<T, Pnt, AABB, 4>));
  }

  SECTION("K = 8")
  {
    check(BVH::PackedBVH<T, Pnt, 8>::template buildBottomUp<SFC::Morton>(pool, flat, size_t(8)));
  }
}

TEMPLATE_TEST_CASE("PackedBVH::traverse: with no bound every primitive is visited once, AsStored visits the leaves in "
                   "stored order, and an empty BVH calls nothing",
                   "[BVH][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;
  using Node   = typename Packed::Node;

  std::vector<std::pair<Pnt, AABB>> flat;

  for (int i = 0; i < 300; i++) {
    const Vec3 pos(T(i % 7), T(i % 11), T(i % 13));

    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  Pool         pool(hostMemoryResource());
  const Packed bvh = Packed::build(pool, flat, BVH::Construction::CentroidSplit);

  // No bound, as stored: every primitive once, and the leaves in the order of the node array (depth-first
  // pre-order, first slot first).
  std::vector<int>      visits(flat.size(), 0);
  std::vector<uint32_t> leafOrder;
  int                   state = 0;

  const auto countVisits = [&visits, &leafOrder](int&, size_t a_offset, size_t a_count) {
    leafOrder.push_back(static_cast<uint32_t>(a_offset));

    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      visits[i]++;
    }
  };

  const auto noBound = [](const int&) { return std::numeric_limits<T>::infinity(); };

  bvh.traverse(BVH::PointQuery{Vec3::zeros()}, state, countVisits, noBound, BVH::DescendAll{}, BVH::AsStored{});

  for (size_t i = 0; i < visits.size(); i++) {
    REQUIRE(visits[i] == 1);
  }

  std::vector<uint32_t> storedOrder;

  for (const auto& node : bvh.getNodes()) {
    if (node.isLeaf()) {
      storedOrder.push_back(node.getPrimitivesOffset());
    }
  }

  REQUIRE(leafOrder == storedOrder);

  // An empty BVH has no root: nothing is visited, and nothing is read.
  const Packed empty(pool, std::vector<Node>{}, std::vector<AABB>{}, std::vector<Pnt>{});

  bool called = false;

  empty.traverse(
    BVH::PointQuery{Vec3::zeros()},
    state,
    [&called](int&, size_t, size_t) { called = true; },
    [&called](const int&) {
      called = true;
      return T(0);
    },
    [&called](int&, uint32_t, const Node&, const T&) { return called = true; });

  REQUIRE_FALSE(called);
}

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

  const Packed bvh = Packed::buildBottomUp(pool, flat, size_t(3));

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

TEMPLATE_TEST_CASE("PackedBVH: traverse() and traverse visit leaves in the order of a recursive depth-first search",
                   "[BVH][traverse][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  std::mt19937                      rng(11);
  std::uniform_real_distribution<T> coord(T(-1), T(1));

  std::vector<std::pair<Pnt, AABB>> flat;

  for (int i = 0; i < 2000; i++) {
    const Vec3 pos(coord(rng), coord(rng), coord(rng));

    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  std::vector<Vec3> queries;

  for (int i = 0; i < 200; i++) {
    queries.emplace_back(T(1.5) * coord(rng), T(1.5) * coord(rng), T(1.5) * coord(rng));
  }

  const auto check = [&](const auto& a_bvh) {
    for (const auto& q : queries) {
      // NearestFirst, against the recursive reference
      std::vector<uint32_t> order;
      std::vector<uint32_t> expected;

      T state = std::numeric_limits<T>::max();
      T best  = std::numeric_limits<T>::max();

      a_bvh.traverse(
        BVH::PointQuery{q},
        state,
        [&](T& a_state, size_t a_offset, size_t a_count) noexcept {
          order.push_back(static_cast<uint32_t>(a_offset));

          NearestLeafEval<T>{a_bvh.getPrimitives(), q}(a_state, a_offset, a_count);
        },
        IdentityPruneDist2<T>{});

      referenceNearestFirst(a_bvh, 0U, q, best, expected);

      REQUIRE(order == expected);
      REQUIRE(state == best);

      // traverse with a visit callback: the same leaves in the same order, every node entered reported
      order.clear();
      expected.clear();

      T found = std::numeric_limits<T>::max();
      T ref   = std::numeric_limits<T>::max();

      uint32_t entered = 0;

      a_bvh.traverse(
        BVH::PointQuery{q},
        found,
        [&](T& a_state, size_t a_offset, size_t a_count) noexcept {
          order.push_back(static_cast<uint32_t>(a_offset));

          TraverseNearestLeaf<T>{a_bvh.getPrimitives(), q}(a_state, a_offset, a_count);
        },
        IdentityPruneDist2<T>{},
        TraverseCountNodes<T>{&entered});

      REQUIRE(entered >= order.size());

      referenceTraverse(a_bvh, 0U, a_bvh.getDistanceToBoundingVolume2(a_bvh.getNodes()[0], q), q, ref, expected);

      REQUIRE(order == expected);
      REQUIRE(found == ref);
    }
  };

  Pool pool(hostMemoryResource());

  SECTION("K = 2")
  {
    check(BVH::PackedBVH<T, Pnt, 2>::buildTopDown(pool, flat, BVH::BinnedSAHPartitioner<T, Pnt, AABB, 2>));
  }

  SECTION("K = 4")
  {
    check(BVH::PackedBVH<T, Pnt, 4>::buildTopDown(pool, flat, BVH::BinnedSAHPartitioner<T, Pnt, AABB, 4>));
  }

  SECTION("K = 8")
  {
    check(BVH::PackedBVH<T, Pnt, 8>::template buildBottomUp<SFC::Morton>(pool, flat, size_t(4)));
  }
}

TEMPLATE_TEST_CASE("PackedBVH: a level resumed with exact distances never prunes a closer primitive",
                   "[BVH][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  // Points on a sphere of radius 1e6 around the query, whose squared distances (about 1e12) differ in
  // bits that float cannot hold. A level is resumed by comparing its next child's distance, computed
  // again exactly as it was sorted, with the bound; a distance kept in less precision and rounded up
  // would prune the closest one.
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

  const BVH::PackedBVH<T, Vec3, K> sah =
    BVH::PackedBVH<T, Vec3, K>::buildTopDown(pool, primsAndBVs, BVH::BinnedSAHPartitioner<T, Vec3, AABB, K>);
  const BVH::PackedBVH<T, Vec3, K> sfc = BVH::PackedBVH<T, Vec3, K>::buildBottomUp(pool, primsAndBVs, K);

  for (const auto* bvh : {&sah, &sfc}) {
    const auto prims = bvh->getPrimitives();

    T best = std::numeric_limits<T>::infinity();

    bvh->traverse(
      BVH::PointQuery{query},
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

TEMPLATE_TEST_CASE("TreeBVH: a root that no shared_ptr owns builds both ways and traverses",
                   "[BVH][regression]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  using Tree = BVH::TreeBVH<T, Vec3, AABB, K>;

  // traverse() and bottomUpSortAndPartition() used to call shared_from_this() on the root, which
  // terminated the program (traverse() was noexcept) for a root on the stack.
  BVH::PrimAndBVList<Vec3, AABB> list;

  for (int i = 0; i < 200; i++) {
    const Vec3 x(T(i % 9), T((i * 7) % 13), T((i * 3) % 5));

    list.emplace_back(std::make_shared<const Vec3>(x), AABB(x, x));
  }

  const auto countVisits = [&list](Tree& a_tree) {
    size_t visits = 0;

    a_tree.template traverse<T>(
      [&visits](const BVH::PrimitiveList<Vec3>& a_prims) noexcept { visits += a_prims.size(); },
      [](const Tree&, const T&) noexcept { return true; },
      [](Array<std::pair<std::shared_ptr<const Tree>, T>, K>&) noexcept {},
      [](const Tree&) noexcept { return T(0); });

    REQUIRE(a_tree.isPartitioned());
    REQUIRE_FALSE(a_tree.isLeaf());
    REQUIRE(visits == list.size());
  };

  SECTION("Top-down")
  {
    Tree tree(list);

    tree.topDownSortAndPartition();

    countVisits(tree);
  }

  SECTION("Bottom-up")
  {
    Tree tree(list);

    tree.template bottomUpSortAndPartition<SFC::Morton>();

    countVisits(tree);
  }
}

TEMPLATE_TEST_CASE("TreeBVH::traverse passes an exception from a callback on to the caller",
                   "[BVH][regression]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  using Tree = BVH::TreeBVH<T, Vec3, AABB, K>;

  // traverse() used to be noexcept, so a throwing callback (or running out of memory for its stack)
  // terminated the program instead.
  BVH::PrimAndBVList<Vec3, AABB> list;

  for (int i = 0; i < 64; i++) {
    const Vec3 x(T(i % 4), T((i / 4) % 4), T(i / 16));

    list.emplace_back(std::make_shared<const Vec3>(x), AABB(x, x));
  }

  Tree tree(list);

  tree.topDownSortAndPartition();

  REQUIRE_THROWS_AS(tree.template traverse<T>([](const BVH::PrimitiveList<Vec3>&) { throw std::runtime_error("leaf"); },
                                              [](const Tree&, const T&) { return true; },
                                              [](Array<std::pair<std::shared_ptr<const Tree>, T>, K>&) {},
                                              [](const Tree&) { return T(0); }),
                    std::runtime_error);
}

TEMPLATE_TEST_CASE("BVH::RayQuery: the entry distance into the unit cube matches the slab answer",
                   "[BVH][traverse][RayQuery]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  const T    inf = std::numeric_limits<T>::infinity();
  const T    s3  = std::sqrt(T(3));
  const AABB cube(Vec3::zeros(), Vec3::ones());

  struct Case
  {
    const char* name;
    Vec3        origin;
    Vec3        direction;
    T           tMin;
    T           expected;
  };

  const std::vector<Case> cases = {
    {"along +x through the middle", Vec3(T(-1), T(0.5), T(0.5)), Vec3(T(1), T(0), T(0)), T(0), T(1)},
    {"along -y through the middle", Vec3(T(0.5), T(3), T(0.5)), Vec3(T(0), T(-1), T(0)), T(0), T(2)},
    {"from inside", Vec3(T(0.5), T(0.5), T(0.5)), Vec3(T(0), T(0), T(1)), T(0), T(0)},
    {"along the diagonal", -Vec3::ones(), Vec3::ones() / s3, T(0), s3},
    {"past the cube", Vec3(T(-1), T(2), T(0.5)), Vec3(T(1), T(0), T(0)), T(0), inf},
    {"pointing away", Vec3(T(2), T(0.5), T(0.5)), Vec3(T(1), T(0), T(0)), T(0), inf},
    {"in the plane of a face (y = 0)", Vec3(T(-1), T(0), T(0.5)), Vec3(T(1), T(0), T(0)), T(0), T(1)},
    {"in the plane of a face (y = 1)", Vec3(T(-1), T(1), T(0.5)), Vec3(T(1), T(0), T(0)), T(0), T(1)},
    {"along an edge (y = z = 1)", Vec3(T(-1), T(1), T(1)), Vec3(T(1), T(0), T(0)), T(0), T(1)},
    {"with a negative zero component", Vec3(T(-1), T(0.5), T(0.5)), Vec3(T(1), -T(0), -T(0)), T(0), T(1)},
    {"backwards, tMin before the cube", Vec3(T(2), T(0.5), T(0.5)), Vec3(T(-1), T(0), T(0)), T(0.5), T(1)},
    {"backwards, tMin inside the cube", Vec3(T(2), T(0.5), T(0.5)), Vec3(T(-1), T(0), T(0)), T(1.5), T(1.5)},
    {"backwards, tMin past the cube", Vec3(T(2), T(0.5), T(0.5)), Vec3(T(-1), T(0), T(0)), T(2.5), inf},
  };

  // The same cube in every slot of a row, so the K-wide path is checked too.
  BVH::ChildBoxes<T, 4> row;

  for (size_t axis = 0; axis < 3; axis++) {
    for (size_t k = 0; k < 4; k++) {
      row.m_lo[axis][k] = T(0);
      row.m_hi[axis][k] = T(1);
    }
  }

  for (const auto& c : cases) {
    INFO(c.name);

    const BVH::RayQuery<T> ray(c.origin, c.direction, c.tMin);

    const T single  = ray.key(cube);
    const T inRow   = ray.key(row, 2);
    const T fromRow = ray.keys(row)[1];

    if (c.expected == inf) {
      REQUIRE(single == inf);
    }
    else {
      REQUIRE_THAT(single, withinAbsT(c.expected, tightMargin<T>()));
    }

    REQUIRE(inRow == single);
    REQUIRE(fromRow == single);
  }
}

// The slab test without the widening, as a reference: each case must be one it misses.
template <class T>
T
unwidenedEntry(const EdgeRay<T>& a_case)
{
  T entry = T(0);
  T exit  = std::numeric_limits<T>::infinity();

  for (int axis = 0; axis < 3; axis++) {
    const T inv = T(1) / a_case.m_direction[axis];
    const T t0  = (a_case.m_lo[axis] - a_case.m_origin[axis]) * inv;
    const T t1  = (a_case.m_hi[axis] - a_case.m_origin[axis]) * inv;

    entry = std::max(entry, std::min(t0, t1));
    exit  = std::min(exit, std::max(t0, t1));
  }

  return (exit >= entry) ? entry : std::numeric_limits<T>::infinity();
}

TEMPLATE_TEST_CASE("BVH::RayQuery: a ray that meets a box only along an edge enters it, which it would not "
                   "without the widening",
                   "[BVH][traverse][RayQuery]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  const T inf = std::numeric_limits<T>::infinity();

  for (const EdgeRay<T>& c : edgeRays<T>()) {
    const Vec3 lo(c.m_lo[0], c.m_lo[1], c.m_lo[2]);
    const Vec3 hi(c.m_hi[0], c.m_hi[1], c.m_hi[2]);

    const BVH::RayQuery<T> ray(Vec3(c.m_origin[0], c.m_origin[1], c.m_origin[2]),
                               Vec3(c.m_direction[0], c.m_direction[1], c.m_direction[2]));

    // The case needs the widening: rounded without it, the ray misses the box.
    REQUIRE(unwidenedEntry(c) == inf);

    // One box, and the same box in every slot of a row (the SIMD path where this build has one).
    BVH::ChildBoxes<T, 4> row;

    for (size_t axis = 0; axis < 3; axis++) {
      for (size_t k = 0; k < 4; k++) {
        row.m_lo[axis][k] = lo[axis];
        row.m_hi[axis][k] = hi[axis];
      }
    }

    const T single = ray.key(AABB(lo, hi));

    REQUIRE(single < inf);
    REQUIRE(ray.key(row, 1) == single);

    for (const T entry : ray.keys(row)) {
      REQUIRE(entry == single);
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH::traverse with a RayQuery: the nearest box entered matches a brute-force scan",
                   "[BVH][traverse][RayQuery]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  const auto boxes = rayTestBoxes<T>(1500);
  const auto rays  = rayTestRays<T>(600);

  const auto check = [&](const auto& a_bvh) {
    size_t hits = 0;

    for (const auto& [origin, direction] : rays) {
      const BVH::RayQuery<T> ray(origin, direction);

      T brute = std::numeric_limits<T>::infinity();

      for (const auto& b : boxes) {
        brute = std::min(brute, ray.key(b.first));
      }

      REQUIRE(nearestBoxEntered(a_bvh, ray) == brute);

      hits += (brute < std::numeric_limits<T>::infinity()) ? 1 : 0;
    }

    // Both kinds of outcome are exercised.
    REQUIRE(hits > 0);
    REQUIRE(hits < rays.size());
  };

  Pool pool(hostMemoryResource());

  SECTION("K = 2")
  {
    check(BVH::PackedBVH<T, AABB, 2>::buildBottomUp(pool, boxes, size_t(4)));
  }

  SECTION("K = 4")
  {
    check(BVH::PackedBVH<T, AABB, 4>::buildBottomUp(pool, boxes, size_t(4)));
  }

  SECTION("K = 8")
  {
    check(BVH::PackedBVH<T, AABB, 8>::buildBottomUp(pool, boxes, size_t(4)));
  }
}

TEMPLATE_TEST_CASE("PackedBVH::traverse with a RayQuery: a box the ray misses is never entered, even with no bound",
                   "[BVH][traverse][RayQuery]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const auto bvh = BVH::PackedBVH<T, AABB, 4>::buildBottomUp(pool, rayTestBoxes<T>(500), size_t(4));

  // A missed box keys +infinity, and a bound that starts at +infinity ("none yet") must not let the walk
  // into it: a ray past every box reaches no leaf, and a ray into the boxes reaches far fewer than all.
  const auto leavesReached = [&](const Vec3& a_origin, const Vec3& a_direction) {
    size_t leaves = 0;
    T      best   = std::numeric_limits<T>::infinity();

    bvh.traverse(
      BVH::RayQuery<T>(a_origin, a_direction),
      best,
      [&leaves](T&, size_t, size_t) { leaves++; },
      [](const T& a_best) { return a_best; });

    return leaves;
  };

  size_t numLeaves = 0;

  for (const auto& node : bvh.getNodes()) {
    numLeaves += node.isLeaf() ? 1 : 0;
  }

  REQUIRE(leavesReached(Vec3(T(5), T(5), T(5)), Vec3(T(1), T(0), T(0))) == 0);
  REQUIRE(leavesReached(Vec3(T(-3), T(0.4), T(0.4)), Vec3(T(1), T(0), T(0))) < numLeaves / 4);

  // A NaN bound (a caller's bug) compares false with every key, so nothing below the root is entered,
  // rather than being read as "no bound" and entering everything.
  size_t nanLeaves = 0;
  T      nanState  = std::numeric_limits<T>::quiet_NaN();

  bvh.traverse(
    BVH::RayQuery<T>(Vec3(T(-3), T(0.4), T(0.4)), Vec3(T(1), T(0), T(0))),
    nanState,
    [&nanLeaves](T&, size_t, size_t) { nanLeaves++; },
    [](const T& a_state) { return a_state; });

  REQUIRE(nanLeaves == 0);
}

TEMPLATE_TEST_CASE("PackedBVH::traverse: the visit orders, and comparators on keys and on node indices",
                   "[BVH][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using AABB   = BoundingVolumes::AABBT<T>;
  using Vec3   = Vec3T<T>;
  using Pnt    = BareTestPoint<T>;
  using Packed = BVH::PackedBVH<T, Pnt, 4>;

  std::mt19937                      rng(17);
  std::uniform_real_distribution<T> coord(T(-1), T(1));

  std::vector<std::pair<Pnt, AABB>> flat;

  for (int i = 0; i < 800; i++) {
    const Vec3 pos(coord(rng), coord(rng), coord(rng));

    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  Pool         pool(hostMemoryResource());
  const Packed bvh = Packed::buildBottomUp(pool, flat, size_t(4));

  const Vec3 q(T(0.3), T(-0.2), T(0.5));

  // The leaves a walk visits, in order, with no bound.
  const auto leafOrder = [&](const auto& a_order) {
    std::vector<uint32_t> order;
    int                   state = 0;

    bvh.traverse(
      BVH::PointQuery{q},
      state,
      [&order](int&, size_t a_offset, size_t) { order.push_back(static_cast<uint32_t>(a_offset)); },
      [](const int&) { return std::numeric_limits<T>::infinity(); },
      BVH::DescendAll{},
      a_order);

    return order;
  };

  size_t numLeaves = 0;

  for (const auto& node : bvh.getNodes()) {
    numLeaves += node.isLeaf() ? 1 : 0;
  }

  const auto nearest  = leafOrder(BVH::NearestFirst{});
  const auto farthest = leafOrder(BVH::FarthestFirst{});
  const auto stored   = leafOrder(BVH::AsStored{});

  REQUIRE(nearest.size() == numLeaves);
  REQUIRE(farthest.size() == numLeaves);
  REQUIRE(stored.size() == numLeaves);

  // Nearest first visits the leaf containing the nearest primitive's box first; farthest first does not.
  REQUIRE(nearest != farthest);

  // A key comparator that says "nearer first" is NearestFirst.
  REQUIRE(leafOrder([](const T& a_a, const T& a_b) { return a_a < a_b; }) == nearest);

  // A comparator on node indices that says "lower index first" is AsStored (pre-order, first slot first).
  REQUIRE(leafOrder([](const T&, uint32_t a_nodeA, const T&, uint32_t a_nodeB) { return a_nodeA < a_nodeB; }) ==
          stored);
}

TEMPLATE_TEST_CASE("PackedBVH::traverse: a visit callback can account for whole subtrees, and stop the walk",
                   "[BVH][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using AABB   = BoundingVolumes::AABBT<T>;
  using Vec3   = Vec3T<T>;
  using Pnt    = BareTestPoint<T>;
  using Packed = BVH::PackedBVH<T, Pnt, 4>;
  using Node   = typename Packed::Node;

  std::mt19937                      rng(19);
  std::uniform_real_distribution<T> coord(T(-1), T(1));

  std::vector<std::pair<Pnt, AABB>> flat;

  for (int i = 0; i < 1000; i++) {
    const Vec3 pos(coord(rng), coord(rng), coord(rng));

    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  Pool         pool(hostMemoryResource());
  const Packed bvh = Packed::buildBottomUp(pool, flat, size_t(4));

  const auto depth = nodeDepths(bvh);

  REQUIRE(*std::max_element(depth.begin(), depth.end()) > 2);

  // A far-field style walk: a node at depth 2 is accounted for as a whole (its primitive count, from the
  // caller's own per-node data) and not descended into; the leaves above that depth count their own.
  std::vector<size_t> summary(bvh.getNodes().size(), 0);

  for (uint32_t i = 0; i < summary.size(); i++) {
    summary[i] = subtreePrimitives(bvh, i);
  }

  size_t summarized = 0;
  size_t counted    = 0;

  bvh.traverse(
    BVH::PointQuery{Vec3::zeros()},
    counted,
    [](size_t& a_counted, size_t, size_t a_count) { a_counted += a_count; },
    [](const size_t&) { return std::numeric_limits<T>::infinity(); },
    [&](size_t& a_counted, uint32_t a_node, const Node&, const T&) {
      if (depth[a_node] == 2) {
        a_counted += summary[a_node];
        summarized++;

        return false;
      }

      return true;
    });

  REQUIRE(counted == flat.size());
  REQUIRE(summarized > 0);

  // Refusing the root reaches nothing.
  size_t reached = 0;

  bvh.traverse(
    BVH::PointQuery{Vec3::zeros()},
    reached,
    [](size_t& a_reached, size_t, size_t a_count) { a_reached += a_count; },
    [](const size_t&) { return std::numeric_limits<T>::infinity(); },
    [](size_t&, uint32_t, const Node&, const T&) { return false; });

  REQUIRE(reached == 0);
}

TEMPLATE_TEST_CASE("PackedBVH::traverse: a user query type with a composite key finds the nearest point",
                   "[BVH][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using AABB   = BoundingVolumes::AABBT<T>;
  using Vec3   = Vec3T<T>;
  using Pnt    = BareTestPoint<T>;
  using Packed = BVH::PackedBVH<T, Pnt, 4>;

  static_assert(BVH::IsQuery<TieBreakPointQuery<T>>::value);
  static_assert(BVH::QueryLanes<TieBreakPointQuery<T>>::value == 1);
  static_assert(std::is_same_v<typename BVH::QueryKey<TieBreakPointQuery<T>>::type, TieKey<T>>);

  std::mt19937                      rng(23);
  std::uniform_real_distribution<T> coord(T(-1), T(1));

  std::vector<std::pair<Pnt, AABB>> flat;

  for (int i = 0; i < 600; i++) {
    const Vec3 pos(coord(rng), coord(rng), coord(rng));

    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  Pool         pool(hostMemoryResource());
  const Packed bvh = Packed::buildBottomUp(pool, flat, size_t(4));

  for (int i = 0; i < 50; i++) {
    const Vec3 q(T(1.5) * coord(rng), T(1.5) * coord(rng), T(1.5) * coord(rng));

    T best = std::numeric_limits<T>::max();

    bvh.traverse(TieBreakPointQuery<T>{BVH::PointQuery{q}},
                 best,
                 NearestLeafEval<T>{bvh.getPrimitives(), q},
                 [](const T& a_best) { return TieKey<T>{a_best, 1}; });

    T brute = std::numeric_limits<T>::max();

    for (const auto& pb : flat) {
      brute = std::min(brute, (pb.first.m_pos - q).length2());
    }

    REQUIRE(best == brute);
  }
}
