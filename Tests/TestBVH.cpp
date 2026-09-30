// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for the BVH -- the one host builder (BVH::buildTopology), the one node layout
// (BVH::WideNode) and the one traversal (PackedBVH::pruneTraverse) -- and for the
// TriMeshSDF/MeshSDF/FlatMeshSDF wrappers built on top of them.
//
// The builder and the traversal are tested directly over synthetic boxes and points: structural
// invariants of every topology, exact nearest-primitive queries against brute force, and the
// always-on checks on adopted node arrays. The mesh-level tests use a regular dodecahedron (20
// vertices, 36 triangulated faces, watertight and orientable) read from disk in all four supported
// formats (STL/PLY/OBJ/VTK), so that both the file parsers and the BVH machinery downstream of them
// get real, non-trivial coverage.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <random>
#include <sstream>
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
// build strategies) chain many more dot/cross products across the mesh's 36 faces, so tolerate
// more accumulated rounding error than a simple format round-trip.
template <class T>
double
traversalMargin()
{
  return std::is_same_v<T, float> ? 1.0e-2 : 1.0e-6;
}

// Every build specification the builder accepts: each strategy, and Strategy::SpaceFillingCurve
// along each curve, all with the given leaf size.
std::vector<BVH::BuildSpec>
allSpecs(const uint32_t a_maxLeafSize)
{
  using BVH::Curve;
  using BVH::Strategy;

  return {BVH::BuildSpec{Strategy::SAH, Curve::Morton, a_maxLeafSize},
          BVH::BuildSpec{Strategy::Centroid, Curve::Morton, a_maxLeafSize},
          BVH::BuildSpec{Strategy::Midpoint, Curve::Morton, a_maxLeafSize},
          BVH::BuildSpec{Strategy::ClusterSAH, Curve::Morton, a_maxLeafSize},
          BVH::BuildSpec{Strategy::SpaceFillingCurve, Curve::Morton, a_maxLeafSize},
          BVH::BuildSpec{Strategy::SpaceFillingCurve, Curve::Nested, a_maxLeafSize},
          BVH::BuildSpec{Strategy::SpaceFillingCurve, Curve::Hilbert, a_maxLeafSize}};
}

// The default strategy, SAH, with the given leaf size.
BVH::BuildSpec
sahSpec(const uint32_t a_maxLeafSize)
{
  return BVH::BuildSpec{BVH::Strategy::SAH, BVH::Curve::Morton, a_maxLeafSize};
}

// Human-readable name of a build specification, for INFO messages.
std::string
specName(const BVH::BuildSpec& a_spec)
{
  std::string name;

  switch (a_spec.strategy) {
  case BVH::Strategy::SAH: {
    name = "SAH";

    break;
  }
  case BVH::Strategy::Centroid: {
    name = "Centroid";

    break;
  }
  case BVH::Strategy::Midpoint: {
    name = "Midpoint";

    break;
  }
  case BVH::Strategy::ClusterSAH: {
    name = "ClusterSAH";

    break;
  }
  case BVH::Strategy::SpaceFillingCurve: {
    name = (a_spec.curve == BVH::Curve::Morton)   ? "SpaceFillingCurve/Morton"
           : (a_spec.curve == BVH::Curve::Nested) ? "SpaceFillingCurve/Nested"
                                                  : "SpaceFillingCurve/Hilbert";

    break;
  }
  default: {
    name = "unknown";

    break;
  }
  }

  return name + ", maxLeafSize " + std::to_string(a_spec.maxLeafSize);
}

// Squared distance from a point to a box, computed with exactly the arithmetic of
// WideNode::getDistance2(): per axis max(0, max(lo - p, p - hi)), then dx*dx + (dy*dy + dz*dz).
// Using it as the primitive distance makes a BVH query comparable to brute force bit for bit: a
// slot box encloses its primitives' boxes, and rounding is monotone, so the slot distance never
// exceeds the primitive distance and exact pruning is guaranteed.
//
// Never inlined: a compiler that contracts a*b + c into a fused multiply-add (GCC does by default in
// the gnu++17 dialect) may contract differently at each inlined copy, and two copies would then
// disagree in the last bit. One compiled body makes every comparison below exact.
template <class T>
[[gnu::noinline]] T
boxDistance2(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi, const Vec3T<T>& a_point) noexcept
{
  T delta[3];

  for (size_t dir = 0; dir < 3; dir++) {
    const T lower = a_lo[dir] - a_point[dir];
    const T upper = a_point[dir] - a_hi[dir];
    const T d     = (upper > lower) ? upper : lower;

    delta[dir] = (d > T(0)) ? d : T(0);
  }

  return delta[0] * delta[0] + (delta[1] * delta[1] + delta[2] * delta[2]);
}

template <class T>
T
boxDistance2(const BoundingVolumes::AABBT<T>& a_box, const Vec3T<T>& a_point) noexcept
{
  return boxDistance2(a_box.getLowCorner(), a_box.getHighCorner(), a_point);
}

// Squared distance between two points, as the box distance to a degenerate box.
template <class T>
T
pointDistance2(const Vec3T<T>& a_position, const Vec3T<T>& a_point) noexcept
{
  return boxDistance2(a_position, a_position, a_point);
}

// Random boxes in [-1, 1]^3 with half-extents up to 0.05; every fifth box is a single point.
template <class T>
std::vector<BoundingVolumes::AABBT<T>>
randomBoxes(const size_t a_count, const uint64_t a_seed)
{
  std::mt19937_64                        rng(a_seed);
  std::uniform_real_distribution<double> centre(-1.0, 1.0);
  std::uniform_real_distribution<double> extent(0.0, 0.05);

  std::vector<BoundingVolumes::AABBT<T>> boxes;

  boxes.reserve(a_count);

  for (size_t i = 0; i < a_count; i++) {
    const T cx = T(centre(rng));
    const T cy = T(centre(rng));
    const T cz = T(centre(rng));
    const T hx = T(extent(rng));
    const T hy = T(extent(rng));
    const T hz = T(extent(rng));

    const Vec3T<T> c(cx, cy, cz);
    const Vec3T<T> h = (i % 5 == 0) ? Vec3T<T>::zeros() : Vec3T<T>(hx, hy, hz);

    boxes.emplace_back(c - h, c + h);
  }

  return boxes;
}

// Centres of the three tight clusters in clusteredBoxes().
template <class T>
std::vector<Vec3T<T>>
clusterCentres()
{
  return {Vec3T<T>(T(0.3), T(-0.2), T(0.1)), Vec3T<T>(T(-0.6), T(0.5), T(-0.4)), Vec3T<T>(T(0.7), T(0.7), T(-0.8))};
}

// Three tight clusters of 300 small boxes each, over a uniform background of 600 boxes.
template <class T>
std::vector<BoundingVolumes::AABBT<T>>
clusteredBoxes()
{
  std::mt19937_64                        rng(20260930ULL);
  std::uniform_real_distribution<double> spread(-1.0e-3, 1.0e-3);
  std::uniform_real_distribution<double> extent(0.0, 1.0e-4);

  std::vector<BoundingVolumes::AABBT<T>> boxes = randomBoxes<T>(600, 7);

  for (const auto& c : clusterCentres<T>()) {
    for (int i = 0; i < 300; i++) {
      const T dx = T(spread(rng));
      const T dy = T(spread(rng));
      const T dz = T(spread(rng));
      const T h  = T(extent(rng));

      const Vec3T<T> p = c + Vec3T<T>(dx, dy, dz);

      boxes.emplace_back(p - h * Vec3T<T>::ones(), p + h * Vec3T<T>::ones());
    }
  }

  return boxes;
}

// Query points for the box sets above: random points around them, points inside each cluster,
// and points far away from everything.
template <class T>
std::vector<Vec3T<T>>
boxQueries()
{
  std::mt19937_64                        rng(4242ULL);
  std::uniform_real_distribution<double> coord(-1.5, 1.5);

  std::vector<Vec3T<T>> queries;

  for (int i = 0; i < 20; i++) {
    const T x = T(coord(rng));
    const T y = T(coord(rng));
    const T z = T(coord(rng));

    queries.emplace_back(x, y, z);
  }

  for (const auto& c : clusterCentres<T>()) {
    queries.push_back(c);
    queries.push_back(c + Vec3T<T>(T(1.0e-5), T(-2.0e-5), T(3.0e-5)));
    queries.push_back(c + Vec3T<T>(T(-4.0e-4), T(2.0e-4), T(0)));
  }

  queries.emplace_back(T(1.0e3), T(0), T(0));
  queries.emplace_back(T(0), T(-1.0e3), T(0));
  queries.emplace_back(T(500), T(500), T(500));
  queries.emplace_back(T(-1.0e4), T(1.0e4), T(3));

  return queries;
}

// First violation of the structural invariants of a topology built over a_boxes with the given
// leaf size, or an empty string if there is none. Returned rather than asserted so that the large
// sweeps below make one assertion per build, not one per slot.
template <class T, size_t K>
std::string
topologyDefect(const BVH::Topology<T, K>&                    a_topology,
               const std::vector<BoundingVolumes::AABBT<T>>& a_boxes,
               const uint32_t                                a_maxLeafSize)
{
  using AABB = BoundingVolumes::AABBT<T>;
  using Node = BVH::WideNode<T, K>;

  const auto&  nodes = a_topology.nodes;
  const auto&  order = a_topology.order;
  const size_t n     = a_boxes.size();

  std::ostringstream defect;

  if (n == 0) {
    if (!nodes.empty() || !order.empty()) {
      defect << "no primitives, but " << nodes.size() << " nodes and " << order.size() << " order entries";
    }

    return defect.str();
  }

  if (order.size() != n) {
    defect << "order has " << order.size() << " entries for " << n << " primitives";

    return defect.str();
  }

  std::vector<unsigned> seen(n, 0U);

  for (const uint32_t item : order) {
    if (item >= n) {
      defect << "order names primitive " << item << " of " << n;

      return defect.str();
    }

    seen[item]++;
  }

  for (size_t i = 0; i < n; i++) {
    if (seen[i] != 1U) {
      defect << "primitive " << i << " appears " << seen[i] << " times in order";

      return defect.str();
    }
  }

  const size_t maxNodes = BVH::PackedBVH<T, uint32_t, K>::maxNodeCount(n);

  if (maxNodes != std::max(size_t(1), n - 1)) {
    defect << "maxNodeCount(" << n << ") is " << maxNodes;

    return defect.str();
  }

  if (nodes.empty() || nodes.size() > maxNodes) {
    defect << nodes.size() << " nodes for " << n << " primitives (at most " << maxNodes << ")";

    return defect.str();
  }

  std::vector<unsigned>                      parents(nodes.size(), 0U);
  std::vector<std::pair<uint32_t, uint32_t>> leaves;

  for (size_t i = 0; i < nodes.size(); i++) {
    const Node& node = nodes[i];

    size_t occupied  = 0;
    bool   seenEmpty = false;

    for (size_t k = 0; k < K; k++) {
      if (node.isEmpty(k)) {
        seenEmpty = true;

        for (size_t dir = 0; dir < 3; dir++) {
          if (!(node.m_lo[dir][k] == std::numeric_limits<T>::infinity() &&
                node.m_hi[dir][k] == -std::numeric_limits<T>::infinity())) {
            defect << "node " << i << " slot " << k << " is empty but its box is not inverted";

            return defect.str();
          }
        }

        continue;
      }

      if (seenEmpty) {
        defect << "node " << i << " has an occupied slot " << k << " after an empty one";

        return defect.str();
      }

      occupied++;

      AABB expected;

      if (node.isLeaf(k)) {
        const uint32_t first = node.getPrimitivesOffset(k);
        const uint32_t count = node.getNumPrimitives(k);

        if (count < 1 || count > a_maxLeafSize) {
          defect << "node " << i << " slot " << k << " is a leaf of " << count << " primitives (max " << a_maxLeafSize
                 << ")";

          return defect.str();
        }

        if (size_t(first) + size_t(count) > n) {
          defect << "node " << i << " slot " << k << " ranges past the order array";

          return defect.str();
        }

        leaves.emplace_back(first, count);

        for (uint32_t j = first; j < first + count; j++) {
          expected = expected.merged(a_boxes[order[j]]);
        }
      }
      else {
        const uint32_t child = node.getChild(k);

        if (child <= i || child >= nodes.size()) {
          defect << "node " << i << " slot " << k << " names child " << child << " of " << nodes.size();

          return defect.str();
        }

        parents[child]++;

        expected = nodes[child].getBoundingVolume();
      }

      const AABB slot = node.getBoundingVolume(k);

      if (!(slot.getLowCorner() == expected.getLowCorner() && slot.getHighCorner() == expected.getHighCorner())) {
        defect << "node " << i << " slot " << k << " has box " << slot << ", but encloses " << expected;

        return defect.str();
      }
    }

    const bool rootWithOneLeaf = (i == 0) && (nodes.size() == 1) && (occupied == 1) && node.isLeaf(0);

    if (occupied < 2 && !rootWithOneLeaf) {
      defect << "node " << i << " has " << occupied << " occupied slots";

      return defect.str();
    }
  }

  for (size_t i = 0; i < nodes.size(); i++) {
    const unsigned expectedParents = (i == 0) ? 0U : 1U;

    if (parents[i] != expectedParents) {
      defect << "node " << i << " has " << parents[i] << " parents";

      return defect.str();
    }
  }

  std::sort(leaves.begin(), leaves.end());

  size_t covered = 0;

  for (const auto& leaf : leaves) {
    if (leaf.first != covered) {
      defect << "leaf ranges leave a gap or overlap at order index " << covered;

      return defect.str();
    }

    covered += leaf.second;
  }

  if (covered != n) {
    defect << "leaf ranges cover " << covered << " of " << n << " primitives";

    return defect.str();
  }

  const size_t depth = BVH::treeDepth(nodes.data(), nodes.size());

  if (depth > BVH::DeviceTraversalDepth) {
    defect << "the tree is " << depth << " levels deep";

    return defect.str();
  }

  return defect.str();
}

// A BVH whose primitives are indices into a caller-owned box array.
template <class T, size_t K>
BVH::PackedBVH<T, uint32_t, K>
buildIndexBVH(Pool& a_pool, const std::vector<BoundingVolumes::AABBT<T>>& a_boxes, const BVH::BuildSpec& a_spec)
{
  std::vector<std::pair<uint32_t, BoundingVolumes::AABBT<T>>> items;

  items.reserve(a_boxes.size());

  for (uint32_t i = 0; i < a_boxes.size(); i++) {
    items.emplace_back(i, a_boxes[i]);
  }

  return BVH::PackedBVH<T, uint32_t, K>(a_pool, std::move(items), a_spec);
}

// Squared distance from a_point to the nearest box, found through an index BVH over a_boxes.
template <class T, size_t K>
T
nearestBox2(const BVH::PackedBVH<T, uint32_t, K>&         a_bvh,
            const std::vector<BoundingVolumes::AABBT<T>>& a_boxes,
            const Vec3T<T>&                               a_point)
{
  const auto prims = a_bvh.getPrimitives();

  T best = std::numeric_limits<T>::infinity();

  const auto evalLeaf = [&prims, &a_boxes, &a_point](T& a_best, const size_t a_offset, const size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      a_best = std::min(a_best, boxDistance2(a_boxes[prims[static_cast<uint32_t>(i)]], a_point));
    }
  };

  a_bvh.pruneTraverse(a_point, best, evalLeaf, [](const T& a_best) noexcept -> T { return a_best; });

  return best;
}

// Squared distance from a_point to the nearest box, by brute force.
template <class T>
T
bruteNearestBox2(const std::vector<BoundingVolumes::AABBT<T>>& a_boxes, const Vec3T<T>& a_point)
{
  T best = std::numeric_limits<T>::infinity();

  for (const auto& box : a_boxes) {
    best = std::min(best, boxDistance2(box, a_point));
  }

  return best;
}

// How often each stored primitive is visited by a traversal whose pruning bound never rejects
// anything. The last entry counts visits outside the primitive array.
template <class T, class P, size_t K>
std::vector<unsigned>
visitCounts(const BVH::PackedBVH<T, P, K>& a_bvh, const Vec3T<T>& a_point)
{
  const size_t numPrims = a_bvh.getPrimitives().size();

  std::vector<unsigned> visits(numPrims + 1, 0U);

  int state = 0;

  const auto evalLeaf = [&visits, numPrims](int&, const size_t a_offset, const size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      visits[std::min(i, numPrims)]++;
    }
  };

  const auto noPrune = [](const int&) noexcept -> T { return std::numeric_limits<T>::infinity(); };

  a_bvh.pruneTraverse(a_point, state, evalLeaf, noPrune);

  return visits;
}

// Stands in for CUDA/HIP managed memory without a GPU: host memory that a device can also address.
// A view rebased onto a mirror in it takes the device-view path, including its depth check.
class FakeManagedMemory final : public MemoryResource
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

// Device memory the host cannot reach, without a GPU: host memory that reports itself device-only.
// Only a mirror can live in it, and nothing built in it may be read on the host.
class FakeDeviceOnlyMemory final : public MemoryResource
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
    return false;
  }

  bool
  isDeviceAccessible() const noexcept override
  {
    return true;
  }

  void
  copy(void* a_dst, const MemoryResource&, const void* a_src, const MemoryResource&, size_t a_bytes)
    const noexcept override
  {
    std::memcpy(a_dst, a_src, a_bytes);
  }
};

// Bare point primitive with no signedDistance() (or any other) member at all -- used to show that
// PackedBVH::pruneTraverse() imposes no interface requirement on its primitive type, unlike a
// caller-built signed-distance wrapper (e.g. MeshSDF/TriMeshSDF::signedDistance()).
template <class T>
struct BareTestPoint
{
  Vec3T<T> m_pos;
};

// A modest, non-lattice point cloud of 125 points -- enough to force several leaves and levels
// without making a brute-force cross-check slow.
template <class T>
std::vector<Vec3T<T>>
gridPositions()
{
  std::vector<Vec3T<T>> positions;

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      for (int k = 0; k < 5; k++) {
        positions.emplace_back(T(i) + T(0.3) * T(j), T(j) - T(0.2) * T(k), T(k) + T(0.1) * T(i));
      }
    }
  }

  return positions;
}

// One (point, degenerate box) pair per position.
template <class T>
std::vector<std::pair<BareTestPoint<T>, BoundingVolumes::AABBT<T>>>
pointPrims(const std::vector<Vec3T<T>>& a_positions)
{
  std::vector<std::pair<BareTestPoint<T>, BoundingVolumes::AABBT<T>>> prims;

  prims.reserve(a_positions.size());

  for (const auto& pos : a_positions) {
    prims.emplace_back(BareTestPoint<T>{pos}, BoundingVolumes::AABBT<T>(pos, pos));
  }

  return prims;
}

// Squared distance from a_query to the nearest point of a point BVH.
template <class T, size_t K>
T
nearestPoint2(const BVH::PackedBVH<T, BareTestPoint<T>, K>& a_bvh, const Vec3T<T>& a_query)
{
  const auto prims = a_bvh.getPrimitives();

  T best = std::numeric_limits<T>::infinity();

  const auto evalLeaf = [&prims, &a_query](T& a_best, const size_t a_offset, const size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      a_best = std::min(a_best, pointDistance2(prims[static_cast<uint32_t>(i)].m_pos, a_query));
    }
  };

  a_bvh.pruneTraverse(a_query, best, evalLeaf, [](const T& a_best) noexcept -> T { return a_best; });

  return best;
}

// Squared distance from a_query to the nearest position, by brute force.
template <class T>
T
bruteNearestPoint2(const std::vector<Vec3T<T>>& a_positions, const Vec3T<T>& a_query)
{
  T best = std::numeric_limits<T>::infinity();

  for (const auto& pos : a_positions) {
    best = std::min(best, pointDistance2(pos, a_query));
  }

  return best;
}

// Every face of a mesh with its bounding box, as PackedBVH's building constructor takes them.
template <class T>
std::vector<std::pair<DCEL::FaceT<T, Meta>, BoundingVolumes::AABBT<T>>>
facesAndBoxes(const DCEL::MeshT<T, Meta>& a_mesh)
{
  std::vector<std::pair<DCEL::FaceT<T, Meta>, BoundingVolumes::AABBT<T>>> facesAndBVs;

  for (uint32_t i = 0; i < a_mesh.numFaces(); i++) {
    const auto& f = a_mesh.getFace(i);

    facesAndBVs.emplace_back(f, BoundingVolumes::AABBT<T>(f.getAllVertexCoordinates(a_mesh)));
  }

  return facesAndBVs;
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
nearestDist2PerQueryPoint(const DCEL::MeshT<T, Meta>&                                                    a_mesh,
                          const std::vector<std::pair<DCEL::FaceT<T, Meta>, BoundingVolumes::AABBT<T>>>& a_facesAndBVs)
{
  Pool pool(hostMemoryResource());

  const BVH::PackedBVH<T, DCEL::FaceT<T, Meta>, K> packed(pool, a_facesAndBVs);

  const auto prims = packed.getPrimitives();

  std::vector<T> result;

  for (const auto& p : queryPoints<T>()) {
    T state = std::numeric_limits<T>::max();

    const auto evalLeaf = [&prims, &p, &a_mesh](T& a_state, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = 0; i < a_count; i++) {
        const T d2 = prims[static_cast<uint32_t>(a_offset + i)].unsignedDistance2(p, a_mesh);

        if (d2 < a_state) {
          a_state = d2;
        }
      }
    };

    const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    packed.pruneTraverse(p, state, evalLeaf, pruneDist2);

    result.push_back(state);
  }

  return result;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// The builder: BVH::buildTopology
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Build a topology for every specification, primitive count and leaf size at one K, and check its
// structural invariants.
template <class T, size_t K>
void
requireTopologyInvariants()
{
  for (const size_t n : {size_t(0), size_t(1), size_t(2), K - 1, K, K + 1, size_t(100), size_t(5000)}) {
    const auto boxes = randomBoxes<T>(n, 1000 + n);

    for (const uint32_t leaf : {1U, 3U, 4U, 16U}) {
      for (const auto& spec : allSpecs(leaf)) {
        const BVH::Topology<T, K> topology = BVH::buildTopology<T, K>(boxes, spec);

        INFO("K = " << K << ", N = " << n << ", " << specName(spec));
        REQUIRE(topologyDefect<T, K>(topology, boxes, leaf) == "");
      }
    }
  }
}

} // namespace

TEMPLATE_TEST_CASE("BVH::buildTopology: structural invariants hold for every strategy, curve, branching factor, "
                   "primitive count and leaf size",
                   "[BVH][build]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Every leaf slot holds 1..maxLeafSize primitives; the leaf ranges partition the order array, which
  // is a permutation of [0, N); occupied slots come first; every interior child follows its parent
  // and has exactly one parent; every node has two occupied slots unless it is a root holding one
  // leaf, so there are at most max(1, N - 1) nodes; every slot box is exactly the union of what it
  // holds; the tree is shallow enough for a device; and no primitives give no nodes.
  requireTopologyInvariants<T, 2>();
  requireTopologyInvariants<T, 3>();
  requireTopologyInvariants<T, 4>();
  requireTopologyInvariants<T, 8>();
  requireTopologyInvariants<T, 16>();
}

namespace {

// Every build of a_boxes at one K, for both a single-primitive and a multi-primitive leaf size, is
// structurally sound, no deeper than a device traversal handles, and answers nearest-box queries
// exactly.
template <class T, size_t K>
void
requireShallowAndExact(const std::string&                            a_label,
                       const std::vector<BoundingVolumes::AABBT<T>>& a_boxes,
                       const std::vector<Vec3T<T>>&                  a_queries)
{
  std::vector<T> expected;

  for (const auto& q : a_queries) {
    expected.push_back(bruteNearestBox2(a_boxes, q));
  }

  for (const uint32_t leaf : {1U, 4U}) {
    for (const auto& spec : allSpecs(leaf)) {
      INFO(a_label << ", K = " << K << ", " << specName(spec));

      const BVH::Topology<T, K> topology = BVH::buildTopology<T, K>(a_boxes, spec);

      REQUIRE(topologyDefect<T, K>(topology, a_boxes, leaf) == "");
      REQUIRE(BVH::treeDepth(topology.nodes.data(), topology.nodes.size()) <= BVH::DeviceTraversalDepth);

      Pool       pool(hostMemoryResource());
      const auto bvh = buildIndexBVH<T, K>(pool, a_boxes, spec);

      for (size_t i = 0; i < a_queries.size(); i++) {
        REQUIRE(nearestBox2<T, K>(bvh, a_boxes, a_queries[i]) == expected[i]);
      }
    }
  }
}

template <class T>
void
requireShallowAndExactForEveryK(const std::string&                            a_label,
                                const std::vector<BoundingVolumes::AABBT<T>>& a_boxes,
                                const std::vector<Vec3T<T>>&                  a_queries)
{
  requireShallowAndExact<T, 2>(a_label, a_boxes, a_queries);
  requireShallowAndExact<T, 3>(a_label, a_boxes, a_queries);
  requireShallowAndExact<T, 4>(a_label, a_boxes, a_queries);
  requireShallowAndExact<T, 8>(a_label, a_boxes, a_queries);
  requireShallowAndExact<T, 16>(a_label, a_boxes, a_queries);
}

} // namespace

TEMPLATE_TEST_CASE("BVH::buildTopology: degenerate inputs keep the tree shallow and the queries exact, for every "
                   "strategy",
                   "[BVH][build]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  // Inputs on which a split by position alone makes no progress, or peels off one primitive at a
  // time: the space-filling-curve bins, the midpoint and the SAH bins all collapse. Each strategy
  // must still produce a sound tree no deeper than a device traversal handles, whether by its own
  // fallbacks or by the builder's rebuild with Strategy::Centroid.
  std::vector<Vec3> queries = queryPoints<T>();

  queries.emplace_back(T(0.25), T(0.25), T(0.25));
  queries.emplace_back(T(1.0e-20), T(0), T(0));
  queries.emplace_back(T(1.25), T(2.25), T(3.25));

  SECTION("All boxes coincident")
  {
    const std::vector<AABB> boxes(400, AABB(Vec3(T(1), T(2), T(3)), Vec3(T(1.5), T(2.5), T(3.5))));

    requireShallowAndExactForEveryK<T>("coincident", boxes, queries);
  }

  SECTION("All centroids on one plane")
  {
    std::vector<AABB> boxes;

    for (int i = 0; i < 25; i++) {
      for (int j = 0; j < 25; j++) {
        const Vec3 p(T(0.1) * T(i), T(0.1) * T(j), T(0));
        const Vec3 h(T(0.01) * T(i % 3), T(0.02) * T(j % 2), T(0.02));

        boxes.emplace_back(p - h, p + h);
      }
    }

    requireShallowAndExactForEveryK<T>("plane", boxes, queries);
  }

  SECTION("All centroids on one line")
  {
    std::vector<AABB> boxes;

    for (int i = 0; i < 500; i++) {
      const T    t = T(0.01) * T(i);
      const Vec3 p(t, T(2) * t, T(3) * t);

      boxes.emplace_back(p, p);
    }

    requireShallowAndExactForEveryK<T>("line", boxes, queries);
  }

  SECTION("One huge cluster of coincident points plus a few far outliers")
  {
    const Vec3        centre(T(0.25), T(0.25), T(0.25));
    std::vector<AABB> boxes(1000, AABB(centre, centre));

    for (const Vec3& outlier : {Vec3(T(1.0e3), T(0), T(0)),
                                Vec3(T(-1.0e3), T(5), T(0)),
                                Vec3(T(0), T(1.0e3), T(-1.0e3)),
                                Vec3(T(500), T(500), T(500)),
                                Vec3(T(-700), T(-700), T(10))}) {
      boxes.emplace_back(outlier, outlier);
    }

    requireShallowAndExactForEveryK<T>("cluster and outliers", boxes, queries);
  }

  SECTION("A geometric sequence of points")
  {
    // Points at 2^-i: every midpoint split peels off one point. The sequence runs into the
    // subnormal range, where the extent of a range of centroids is so small that the SAH bin scale
    // 32 / extent overflows to infinity; that axis must be left out, not binned.
    const int numPoints = std::is_same_v<T, float> ? 160 : 1100;

    std::vector<AABB> boxes;

    for (int i = 0; i < numPoints; i++) {
      const Vec3 p(T(std::ldexp(1.0, -i)), T(0), T(0));

      boxes.emplace_back(p, p);
    }

    requireShallowAndExactForEveryK<T>("geometric sequence", boxes, queries);
  }
}

TEMPLATE_TEST_CASE("BVH::buildTopology: every strategy's leaves never exceed the leaf size, on a point cloud whose "
                   "size is not a multiple of it",
                   "[BVH][build]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  constexpr size_t K = 4;

  const auto        positions = gridPositions<T>();
  std::vector<AABB> boxes;

  for (const auto& pos : positions) {
    boxes.emplace_back(pos, pos);
  }

  for (const uint32_t leaf : {1U, 2U, 7U, 25U, 124U, 125U, 126U}) {
    for (const auto& spec : allSpecs(leaf)) {
      INFO(specName(spec));

      const BVH::Topology<T, K> topology = BVH::buildTopology<T, K>(boxes, spec);

      REQUIRE(topologyDefect<T, K>(topology, boxes, leaf) == "");

      // A leaf size of at least N puts everything in the root's single leaf.
      if (leaf >= boxes.size()) {
        REQUIRE(topology.nodes.size() == 1);
        REQUIRE(topology.nodes[0].numChildren() == 1);
        REQUIRE(topology.nodes[0].getNumPrimitives(0) == boxes.size());
      }
    }
  }
}

TEST_CASE("BVH::buildTopology: REQUIRE checks reject a zero leaf size, a non-finite or inverted box, and an unknown "
          "strategy or curve",
          "[BVH][build][death]")
{
  using T    = double;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  const std::vector<AABB> good{AABB(Vec3(T(0), T(0), T(0)), Vec3(T(1), T(1), T(1))),
                               AABB(Vec3(T(2), T(0), T(0)), Vec3(T(3), T(1), T(1))),
                               AABB(Vec3(T(0), T(2), T(0)), Vec3(T(1), T(3), T(1)))};

  // Sanity: the unmodified input builds.
  REQUIRE_FALSE(aborts([&good] { (void)BVH::buildTopology<T, K>(good, BVH::BuildSpec{}); }));

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  SECTION("maxLeafSize 0")
  {
    REQUIRE(abortsWith([&good] { (void)BVH::buildTopology<T, K>(good, sahSpec(0)); },
                       "BVH::BuildSpec: the maximum leaf size must be positive"));

    // The same check guards every building constructor.
    REQUIRE(abortsWith(
      [] {
        Pool                                         pool(hostMemoryResource());
        const auto                                   prims = pointPrims<T>(gridPositions<T>());
        const BVH::PackedBVH<T, BareTestPoint<T>, K> bvh(pool, prims, sahSpec(0));

        (void)bvh;
      },
      "BVH::BuildSpec: the maximum leaf size must be positive"));
  }

  SECTION("A box with an infinite corner")
  {
    // Corners set through the mutable accessors, which (unlike the corner constructor) do not check
    // them, so what aborts is the builder's own check.
    std::vector<AABB> boxes = good;

    boxes[1].getLowCorner()[0] = -std::numeric_limits<T>::infinity();

    REQUIRE(abortsWith([&boxes] { (void)BVH::buildTopology<T, K>(boxes, BVH::BuildSpec{}); },
                       "primitive 1 of 3 is not finite, or is inverted"));
  }

  SECTION("A box with a NaN corner")
  {
    std::vector<AABB> boxes = good;

    boxes[2].getHighCorner()[1] = std::numeric_limits<T>::quiet_NaN();

    REQUIRE(abortsWith([&boxes] { (void)BVH::buildTopology<T, K>(boxes, BVH::BuildSpec{}); },
                       "primitive 2 of 3 is not finite, or is inverted"));
  }

  SECTION("An inverted box")
  {
    std::vector<AABB> boxes = good;

    boxes[0].getLowCorner() = Vec3(T(2), T(0), T(0));

    REQUIRE(abortsWith([&boxes] { (void)BVH::buildTopology<T, K>(boxes, BVH::BuildSpec{}); },
                       "primitive 0 of 3 is not finite, or is inverted"));
  }

  SECTION("The default (inverted, infinite) box")
  {
    std::vector<AABB> boxes = good;

    boxes.emplace_back();

    REQUIRE(abortsWith([&boxes] { (void)BVH::buildTopology<T, K>(boxes, BVH::BuildSpec{}); },
                       "primitive 3 of 4 is not finite, or is inverted"));
  }

  SECTION("An unknown strategy")
  {
    const BVH::BuildSpec spec{static_cast<BVH::Strategy>(99), BVH::Curve::Morton, 4};

    REQUIRE(abortsWith([&good, spec] { (void)BVH::buildTopology<T, K>(good, spec); },
                       "BVH::BuildSpec: unknown strategy (99)"));
  }

  SECTION("An unknown curve")
  {
    const BVH::BuildSpec spec{BVH::Strategy::SpaceFillingCurve, static_cast<BVH::Curve>(7), 4};

    REQUIRE(
      abortsWith([&good, spec] { (void)BVH::buildTopology<T, K>(good, spec); }, "BVH::BuildSpec: unknown curve (7)"));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// The traversal: PackedBVH::pruneTraverse
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Nearest-box queries through every build at one K agree exactly with brute force.
template <class T, size_t K>
void
requireExactNearest(const std::vector<BoundingVolumes::AABBT<T>>& a_boxes,
                    const std::vector<Vec3T<T>>&                  a_queries,
                    const std::vector<T>&                         a_expected)
{
  for (const uint32_t leaf : {1U, 4U, 16U}) {
    for (const auto& spec : allSpecs(leaf)) {
      Pool       pool(hostMemoryResource());
      const auto bvh = buildIndexBVH<T, K>(pool, a_boxes, spec);

      for (size_t i = 0; i < a_queries.size(); i++) {
        INFO("K = " << K << ", " << specName(spec) << ", query " << a_queries[i]);
        REQUIRE(nearestBox2<T, K>(bvh, a_boxes, a_queries[i]) == a_expected[i]);
      }
    }
  }
}

} // namespace

TEMPLATE_TEST_CASE("PackedBVH::pruneTraverse: the nearest box is exactly the brute-force one, for every strategy "
                   "and branching factor",
                   "[BVH][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // The primitive distance is the box distance with the node's own arithmetic, so pruning is exact
  // and the result must be bit-identical to brute force: random points around the boxes, points
  // inside the tight clusters, and points far from everything.
  const auto boxes   = clusteredBoxes<T>();
  const auto queries = boxQueries<T>();

  std::vector<T> expected;

  for (const auto& q : queries) {
    expected.push_back(bruteNearestBox2(boxes, q));
  }

  requireExactNearest<T, 2>(boxes, queries, expected);
  requireExactNearest<T, 3>(boxes, queries, expected);
  requireExactNearest<T, 4>(boxes, queries, expected);
  requireExactNearest<T, 8>(boxes, queries, expected);
  requireExactNearest<T, 16>(boxes, queries, expected);
}

namespace {

// With a pruning bound that rejects nothing, every build at one K visits every primitive exactly
// once, and the stored indices are a permutation of the input.
template <class T, size_t K>
void
requireVisitOnce()
{
  for (const size_t n : {size_t(0), size_t(1), size_t(2), K - 1, K, K + 1, size_t(100), size_t(1000)}) {
    const auto boxes = randomBoxes<T>(n, 77 + n);

    for (const uint32_t leaf : {1U, 3U, 4U, 16U}) {
      for (const auto& spec : allSpecs(leaf)) {
        INFO("K = " << K << ", N = " << n << ", " << specName(spec));

        Pool       pool(hostMemoryResource());
        const auto bvh    = buildIndexBVH<T, K>(pool, boxes, spec);
        const auto prims  = bvh.getPrimitives();
        const auto visits = visitCounts(bvh, Vec3T<T>(T(0.1), T(-0.2), T(0.3)));

        REQUIRE(prims.size() == n);
        REQUIRE(visits.back() == 0U);
        REQUIRE(std::count(visits.begin(), visits.end() - 1, 1U) == static_cast<std::ptrdiff_t>(n));

        std::vector<unsigned> seen(n, 0U);

        for (const uint32_t item : prims) {
          seen[item]++;
        }

        REQUIRE(std::count(seen.begin(), seen.end(), 1U) == static_cast<std::ptrdiff_t>(n));
      }
    }
  }
}

} // namespace

TEMPLATE_TEST_CASE("PackedBVH::pruneTraverse: with no pruning bound every primitive is visited exactly once",
                   "[BVH][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A reduction that is not idempotent -- counting, or keeping the two smallest values -- depends on
  // every primitive being reached exactly once, whatever the primitive count and leaf size.
  requireVisitOnce<T, 2>();
  requireVisitOnce<T, 3>();
  requireVisitOnce<T, 4>();
  requireVisitOnce<T, 8>();
  requireVisitOnce<T, 16>();
}

TEST_CASE("PackedBVH::pruneTraverse: rounding a stacked distance to float never prunes a strictly closer primitive",
          "[BVH][traverse]")
{
  using T    = double;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  // The traversal stack stores each slot's squared distance as a float. For T = double that float
  // must be a lower bound: rounded to nearest, a squared distance just below a float can round up
  // past the distance of a primitive found in the meantime, and the closer slot would be skipped.
  //
  // 1000000061440 = 15258790 * 2^16 is a float, and floats are 2^16 apart there. `closer` lies at a
  // squared distance just below it, `farther` about 0.02 above that -- far less than float spacing.
  const T floatValue = T(15258790.0 * 65536.0);
  const T x          = std::sqrt(floatValue - T(1));

  T y = x;

  for (int i = 0; i < 100; i++) {
    y = std::nextafter(y, T(2.0e6));
  }

  const Vec3 query = Vec3::zeros();
  const Vec3 closer(x, T(0), T(0));
  const Vec3 farther(T(0), y, T(0));
  const Vec3 decoy(T(-2.0e6), T(5.0e5), T(0));

  const T closer2  = pointDistance2(closer, query);
  const T farther2 = pointDistance2(farther, query);

  REQUIRE(closer2 < farther2);
  REQUIRE(static_cast<double>(static_cast<float>(closer2)) > farther2);
  REQUIRE(pointDistance2(decoy, query) > farther2);

  SECTION("A hand-made tree that visits the farther primitive first")
  {
    constexpr size_t K = 4;

    using Packed = BVH::PackedBVH<T, Pnt, K>;
    using Node   = typename Packed::Node;

    // Slot 0 is a leaf over {farther, decoy}, whose box reaches to within 5e5 of the query, so it is
    // expanded first and sets the bound to farther2. Slot 1 is a leaf over {closer}, pushed with a
    // float bound on closer2; rounding that bound up would skip it.
    std::vector<Node> nodes{Node::empty()};

    nodes[0].setLeaf(0, AABB(min(farther, decoy), max(farther, decoy)), 0, 2);
    nodes[0].setLeaf(1, AABB(closer, closer), 2, 1);

    REQUIRE(nodes[0].getDistance2(0, query) < nodes[0].getDistance2(1, query));

    const std::vector<Pnt> prims{Pnt{farther}, Pnt{decoy}, Pnt{closer}};

    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, nodes, prims);

    REQUIRE(nearestPoint2<T, K>(bvh, query) == closer2);
  }

  SECTION("Built trees over points whose squared distances all lie within one float spacing")
  {
    // 256 points in random directions at radii 1e6 + i * 1e-6, so their squared distances from the
    // origin differ by about 2 * i, far below the 2^16 float spacing around 1e12.
    std::mt19937_64                        rng(99ULL);
    std::uniform_real_distribution<double> angle(0.0, 6.283185307179586);
    std::uniform_real_distribution<double> height(-1.0, 1.0);

    std::vector<Vec3> positions;

    for (int i = 0; i < 256; i++) {
      const T phi = angle(rng);
      const T z   = height(rng);
      const T r   = std::sqrt(T(1) - z * z);
      const T rad = T(1.0e6) + T(i) * T(1.0e-6);

      positions.emplace_back(rad * r * std::cos(phi), rad * r * std::sin(phi), rad * z);
    }

    const auto prims = pointPrims<T>(positions);

    for (const Vec3& q : {query, Vec3(T(1.0e-3), T(0), T(0)), Vec3(T(0), T(-2.5e-3), T(1.0e-3))}) {
      const T expected = bruteNearestPoint2(positions, q);

      for (const auto& spec : allSpecs(1)) {
        INFO(specName(spec));

        Pool pool(hostMemoryResource());

        REQUIRE(nearestPoint2<T, 2>(BVH::PackedBVH<T, Pnt, 2>(pool, prims, spec), q) == expected);
        REQUIRE(nearestPoint2<T, 4>(BVH::PackedBVH<T, Pnt, 4>(pool, prims, spec), q) == expected);
        REQUIRE(nearestPoint2<T, 8>(BVH::PackedBVH<T, Pnt, 8>(pool, prims, spec), q) == expected);
      }
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH::traversalStackDepth is (K - 1) * HostTraversalDepth + 1 on the host",
                   "[BVH][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  static_assert(BVH::HostTraversalDepth == 64);
  static_assert(BVH::DeviceTraversalDepth == 32);

  // Checked at run time, not with static_assert: in a GPU build this file is also compiled for the
  // device, where the same function returns the device stack size.
  REQUIRE(BVH::PackedBVH<T, uint32_t, 2>::traversalStackDepth() == 1 * size_t(BVH::HostTraversalDepth) + 1);
  REQUIRE(BVH::PackedBVH<T, uint32_t, 3>::traversalStackDepth() == 2 * size_t(BVH::HostTraversalDepth) + 1);
  REQUIRE(BVH::PackedBVH<T, uint32_t, 4>::traversalStackDepth() == 3 * size_t(BVH::HostTraversalDepth) + 1);
  REQUIRE(BVH::PackedBVH<T, uint32_t, 8>::traversalStackDepth() == 7 * size_t(BVH::HostTraversalDepth) + 1);
  REQUIRE(BVH::PackedBVH<T, uint32_t, 16>::traversalStackDepth() == 15 * size_t(BVH::HostTraversalDepth) + 1);

  // maxNodeCount is max(1, leaves - 1).
  static_assert(BVH::PackedBVH<T, uint32_t, 4>::maxNodeCount(0) == 1);
  static_assert(BVH::PackedBVH<T, uint32_t, 4>::maxNodeCount(1) == 1);
  static_assert(BVH::PackedBVH<T, uint32_t, 4>::maxNodeCount(2) == 1);
  static_assert(BVH::PackedBVH<T, uint32_t, 4>::maxNodeCount(3) == 2);
  static_assert(BVH::PackedBVH<T, uint32_t, 4>::maxNodeCount(100) == 99);
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructors: leaf packing, adopting node arrays, the empty BVH
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Up to two input primitives stored as one, as TriMeshSDF stores a leaf's triangles in SIMD groups.
struct PackedPair
{
  uint32_t m_items[2];
  uint32_t m_count;
};

} // namespace

TEMPLATE_TEST_CASE("PackedBVH: the leaf-packing constructor stores what the packer appends, and each leaf ranges "
                   "over exactly its own items",
                   "[BVH][build]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, PackedPair, K>;

  const auto boxes = randomBoxes<T>(333, 99);

  std::vector<T> expected;

  for (const auto& q : boxQueries<T>()) {
    expected.push_back(bruteNearestBox2(boxes, q));
  }

  for (const auto& spec : allSpecs(5)) {
    INFO(specName(spec));

    const BVH::Topology<T, K> topology = BVH::buildTopology<T, K>(boxes, spec);

    size_t calls = 0;

    const auto packLeaf = [&calls](const uint32_t* a_items, const uint32_t a_count, std::vector<PackedPair>& a_out) {
      calls++;

      for (uint32_t i = 0; i < a_count; i += 2) {
        const uint32_t count = std::min(2U, a_count - i);

        a_out.push_back(PackedPair{{a_items[i], (count == 2) ? a_items[i + 1] : 0U}, count});
      }
    };

    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, topology, packLeaf);

    const auto nodes = bvh.getNodes();
    const auto prims = bvh.getPrimitives();

    // Same shape as the topology; each leaf now ranges over ceil(count / 2) stored primitives, which
    // hold exactly that leaf's input primitives, in order.
    REQUIRE(nodes.size() == topology.nodes.size());

    std::vector<std::pair<uint32_t, uint32_t>> ranges;

    for (uint32_t n = 0; n < nodes.size(); n++) {
      for (size_t k = 0; k < K; k++) {
        const auto& shape = topology.nodes[n];
        const auto& node  = nodes[n];

        REQUIRE(node.isEmpty(k) == shape.isEmpty(k));
        REQUIRE(node.isLeaf(k) == shape.isLeaf(k));
        REQUIRE(node.isInterior(k) == shape.isInterior(k));

        if (node.isInterior(k)) {
          REQUIRE(node.getChild(k) == shape.getChild(k));
        }

        if (node.isLeaf(k)) {
          const uint32_t first = node.getPrimitivesOffset(k);
          const uint32_t count = node.getNumPrimitives(k);

          REQUIRE(count == (shape.getNumPrimitives(k) + 1) / 2);

          std::vector<uint32_t> items;

          for (uint32_t p = first; p < first + count; p++) {
            for (uint32_t j = 0; j < prims[p].m_count; j++) {
              items.push_back(prims[p].m_items[j]);
            }
          }

          const auto begin = topology.order.begin() + shape.getPrimitivesOffset(k);
          const auto end   = begin + shape.getNumPrimitives(k);

          REQUIRE(items == std::vector<uint32_t>(begin, end));

          ranges.emplace_back(first, count);
        }
      }
    }

    // The packer ran once per leaf, and the leaf ranges partition the stored primitives.
    REQUIRE(calls == ranges.size());

    std::sort(ranges.begin(), ranges.end());

    uint32_t covered = 0;

    for (const auto& range : ranges) {
      REQUIRE(range.first == covered);

      covered += range.second;
    }

    REQUIRE(covered == prims.size());

    // Every stored primitive, and so every input primitive, is visited exactly once.
    const auto visits = visitCounts(bvh, Vec3T<T>::zeros());

    REQUIRE(visits.back() == 0U);
    REQUIRE(std::count(visits.begin(), visits.end() - 1, 1U) == static_cast<std::ptrdiff_t>(prims.size()));

    // And queries through the packed pairs are exact.
    const auto queries = boxQueries<T>();

    for (size_t i = 0; i < queries.size(); i++) {
      const Vec3T<T>& q = queries[i];

      T best = std::numeric_limits<T>::infinity();

      const auto evalLeaf = [&prims, &boxes, &q](T& a_best, const size_t a_offset, const size_t a_count) noexcept {
        for (size_t p = a_offset; p < a_offset + a_count; p++) {
          const PackedPair& pair = prims[static_cast<uint32_t>(p)];

          for (uint32_t j = 0; j < pair.m_count; j++) {
            a_best = std::min(a_best, boxDistance2(boxes[pair.m_items[j]], q));
          }
        }
      };

      bvh.pruneTraverse(q, best, evalLeaf, [](const T& a_best) noexcept -> T { return a_best; });

      REQUIRE(best == expected[i]);
    }
  }

  SECTION("An empty topology gives an empty BVH, without calling the packer")
  {
    const BVH::Topology<T, K> empty = BVH::buildTopology<T, K>(std::vector<AABB>{}, BVH::BuildSpec{});

    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, empty, [](const uint32_t*, uint32_t, std::vector<PackedPair>&) { FAIL("packer called"); });

    REQUIRE(bvh.getNodes().size() == 0);
    REQUIRE(bvh.getPrimitives().size() == 0);
  }
}

TEST_CASE("PackedBVH: the leaf-packing constructor rejects a packer that stores nothing for a leaf",
          "[BVH][build][death]")
{
  using T = double;

  constexpr size_t K = 4;

  REQUIRE(abortsWith(
    [] {
      const auto                boxes    = randomBoxes<T>(10, 1);
      const BVH::Topology<T, K> topology = BVH::buildTopology<T, K>(boxes, BVH::BuildSpec{});

      Pool                                 pool(hostMemoryResource());
      const BVH::PackedBVH<T, uint32_t, K> bvh(
        pool, topology, [](const uint32_t*, uint32_t, std::vector<uint32_t>&) {});

      (void)bvh;
    },
    "BVH::PackedBVH: the leaf packer must store at least one primitive per leaf"));
}

TEST_CASE("PackedBVH::requireWellFormed: each kind of malformed node array aborts with its own message",
          "[BVH][adopt][death]")
{
  using T    = double;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;
  using Node   = typename Packed::Node;

  // Root: two interior children and one leaf. Nodes 1 and 2: two leaves each. Five primitives, each
  // in one leaf -- well formed, as the first checks confirm, so each death below is caused by the
  // one defect it introduces.
  const AABB box(Vec3::zeros(), Vec3::ones());

  std::vector<Node> good(3, Node::empty());

  good[0].setInterior(0, box, 1);
  good[0].setInterior(1, box, 2);
  good[0].setLeaf(2, box, 0, 1);
  good[1].setLeaf(0, box, 1, 1);
  good[1].setLeaf(1, box, 2, 1);
  good[2].setLeaf(0, box, 3, 1);
  good[2].setLeaf(1, box, 4, 1);

  const std::vector<Pnt> prims(5, Pnt{Vec3::zeros()});

  // Adopt a node array through the host-array constructor, which runs requireWellFormed in every build.
  const auto adopt = [&prims](const std::vector<Node>& a_nodes) {
    return [&prims, a_nodes] {
      Pool         pool(hostMemoryResource());
      const Packed bvh(pool, a_nodes, prims);

      (void)bvh;
    };
  };

  REQUIRE_FALSE(aborts([&good] { Packed::requireWellFormed(good.data(), good.size(), 5); }));
  REQUIRE_FALSE(aborts(adopt(good)));

  const std::string prefix = "BVH::PackedBVH: adopted node array is malformed -- ";

  SECTION("A leaf range past the primitive array")
  {
    std::vector<Node> bad = good;

    bad[1].setLeaf(1, box, 2, 10);

    REQUIRE(abortsWith(adopt(bad),
                       prefix + "node 1 slot 1's primitive range ends at 12, past the primitive array's 5 primitives"));
  }

  SECTION("A child that is not after its parent")
  {
    std::vector<Node> bad = good;

    bad[0].setInterior(2, box, 0);

    REQUIRE(abortsWith(adopt(bad), prefix + "node 0 slot 2 names child 0, which is not after its parent"));
  }

  SECTION("A child outside the node array")
  {
    std::vector<Node> bad = good;

    bad[1].setInterior(1, box, 3);

    REQUIRE(abortsWith(adopt(bad), prefix + "node 1 slot 1 names child 3, which is not after its parent"));
  }

  SECTION("A child with two parents")
  {
    std::vector<Node> bad = good;

    bad.push_back(Node::empty());
    bad[3].setLeaf(0, box, 0, 1);
    bad[1].setInterior(1, box, 3);
    bad[2].setInterior(1, box, 3);

    REQUIRE(abortsWith(adopt(bad), prefix + "node 2 slot 1 names child 3, which is not after its parent"));
  }

  SECTION("An occupied slot after an empty one")
  {
    std::vector<Node> bad = good;

    bad[0].setEmpty(2);
    bad[0].setLeaf(3, box, 0, 1);

    REQUIRE(abortsWith(adopt(bad), prefix + "node 0 has an occupied slot 3 after an empty one"));
  }

  SECTION("A node with no occupied slot")
  {
    std::vector<Node> bad = good;

    bad.push_back(Node::empty());
    bad[0].setInterior(3, box, 3);

    REQUIRE(abortsWith(adopt(bad), prefix + "node 3 has no occupied slot"));

    // requireWellFormed is public, for a class that assembles a node array of its own.
    REQUIRE(abortsWith([bad] { Packed::requireWellFormed(bad.data(), bad.size(), 5); },
                       prefix + "node 3 has no occupied slot"));
  }

  SECTION("A node unreachable from the root")
  {
    std::vector<Node> bad = good;

    bad.push_back(Node::empty());
    bad[3].setLeaf(0, box, 0, 1);

    REQUIRE(abortsWith(adopt(bad), prefix + "node 3 is not reachable from the root"));
  }

  SECTION("An empty node array with a nonempty primitive array")
  {
    REQUIRE(abortsWith(adopt(std::vector<Node>{}), prefix + "it is empty, but the primitive array holds 5 primitives"));

    // An empty node array with no primitives is the empty BVH, and is accepted.
    REQUIRE_FALSE(aborts([] {
      Pool         pool(hostMemoryResource());
      const Packed bvh(pool, std::vector<Node>{}, std::vector<Pnt>{});

      (void)bvh;
    }));
  }
}

TEST_CASE("PackedBVH: a tree too deep for a traversal stack is rejected -- at build time for the host, at "
          "rebasedView for a device",
          "[BVH][death]")
{
  using T    = double;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;
  using Node   = typename Packed::Node;

  // No builder makes a deep tree, so the tree is written by hand and adopted through the host-array
  // constructor: a chain of nodes, each with an interior slot to the next node and a leaf slot. What
  // is being tested is that exceeding a traversal stack's depth fails loudly, rather than overflowing
  // the stack, which in Release would be a silent out-of-bounds write.
  const std::vector<Pnt> prims = {Pnt{Vec3::zeros()}};
  const AABB             box(Vec3::zeros(), Vec3::ones());

  const auto chain = [&box](const size_t a_depth) {
    std::vector<Node> nodes(a_depth, Node::empty());

    for (size_t i = 0; i + 1 < a_depth; i++) {
      nodes[i].setInterior(0, box, static_cast<uint32_t>(i + 1));
      nodes[i].setLeaf(1, box, 0, 1);
    }

    nodes[a_depth - 1].setLeaf(0, box, 0, 1);

    return nodes;
  };

  REQUIRE(BVH::treeDepth(chain(40).data(), 40) == 40);

  SECTION("The host limit")
  {
    // HostTraversalDepth levels traverse fine: every leaf is reached.
    Pool         pool(hostMemoryResource());
    const Packed deepest(pool, chain(BVH::HostTraversalDepth), prims);

    const auto visits = visitCounts(deepest, Vec3::zeros());

    REQUIRE(visits[0] == BVH::HostTraversalDepth);

    REQUIRE(abortsWith(
      [&] {
        Pool         otherPool(hostMemoryResource());
        const Packed bvh(otherPool, chain(BVH::HostTraversalDepth + 1), prims);

        (void)bvh;
      },
      "host build -- the tree is 65 levels deep, more than the 64 levels a traversal stack holds"));
  }

  SECTION("The device limit")
  {
    // A tree between the device and host limits builds and rebases onto a host mirror, but not onto a
    // device-accessible one.
    const auto deviceView = [&](const size_t a_depth) {
      return [&, a_depth] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, chain(a_depth), prims);

        pool.freeze();

        FakeManagedMemory managed;
        const Pool        mirror = Pool::mirror(pool, managed);
        const Packed      view   = bvh.rebasedView(mirror);

        (void)view;
      };
    };

    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, chain(40), prims);

    pool.freeze();

    const Pool   hostMirror = Pool::mirror(pool, hostMemoryResource());
    const Packed hostView   = bvh.rebasedView(hostMirror);

    REQUIRE(visitCounts(hostView, Vec3::zeros())[0] == 40U);

    REQUIRE_FALSE(aborts(deviceView(BVH::DeviceTraversalDepth)));
    REQUIRE(abortsWith(deviceView(40),
                       "device view -- the tree is 40 levels deep, more than the 32 levels a traversal stack holds"));
  }
}

namespace {

// Reserve node and index arrays in a_pool, as a builder writing in place (a device kernel, say)
// would, and fill them from a topology.
template <class T, size_t K>
std::pair<PODVector<BVH::WideNode<T, K>>, PODVector<uint32_t>>
reserveAndFill(Pool& a_pool, const BVH::Topology<T, K>& a_topology)
{
  using Packed = BVH::PackedBVH<T, uint32_t, K>;

  const size_t n = a_topology.order.size();

  PODVector<BVH::WideNode<T, K>> nodes;
  PODVector<uint32_t>            prims;

  nodes.reserveFrom(a_pool, static_cast<uint32_t>(Packed::maxNodeCount(n)));
  prims.reserveFrom(a_pool, static_cast<uint32_t>(n));

  // Resolve the base only after both reservations: a reserve can move the block.
  void* base = a_pool.base();

  std::copy(a_topology.nodes.begin(), a_topology.nodes.end(), nodes.data(base));
  std::copy(a_topology.order.begin(), a_topology.order.end(), prims.data(base));

  nodes.setSize(static_cast<uint32_t>(a_topology.nodes.size()));
  prims.setSize(static_cast<uint32_t>(n));

  return {nodes, prims};
}

} // namespace

TEMPLATE_TEST_CASE("PackedBVH: the pool-resident adopt constructor answers exactly as a normally built BVH",
                   "[BVH][adopt]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, uint32_t, K>;

  const auto boxes   = clusteredBoxes<T>();
  const auto queries = boxQueries<T>();

  for (const auto& spec : allSpecs(4)) {
    INFO(specName(spec));

    const BVH::Topology<T, K> topology = BVH::buildTopology<T, K>(boxes, spec);

    REQUIRE(topology.nodes.size() <= Packed::maxNodeCount(boxes.size()));

    Pool pool(hostMemoryResource());

    const auto   arrays = reserveAndFill<T, K>(pool, topology);
    const Packed adopted(pool, arrays.first, arrays.second);

    // Built after the adoption, so the pool grows under the adopted BVH; it follows the pool.
    const Packed built = buildIndexBVH<T, K>(pool, boxes, spec);

    REQUIRE(adopted.isAttachedTo(pool));
    REQUIRE(adopted.getNodes().size() == topology.nodes.size());
    REQUIRE(adopted.getNodes().size() == built.getNodes().size());
    REQUIRE(adopted.getPrimitives().size() == boxes.size());

    for (const auto& q : queries) {
      const T expected = bruteNearestBox2(boxes, q);

      REQUIRE(nearestBox2<T, K>(adopted, boxes, q) == expected);
      REQUIRE(nearestBox2<T, K>(built, boxes, q) == expected);
    }

    // And it rebases like any other BVH.
    pool.freeze();

    const Pool   mirror = Pool::mirror(pool, hostMemoryResource());
    const Packed view   = adopted.rebasedView(mirror);

    for (const auto& q : queries) {
      REQUIRE(nearestBox2<T, K>(view, boxes, q) == nearestBox2<T, K>(adopted, boxes, q));
    }
  }
}

TEST_CASE("PackedBVH: the pool-resident adopt constructor rejects a malformed array, and arrays that end past the "
          "pool's used bytes",
          "[BVH][adopt][death]")
{
  using T = double;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, uint32_t, K>;

  const auto                boxes    = randomBoxes<T>(200, 5);
  const BVH::Topology<T, K> topology = BVH::buildTopology<T, K>(boxes, BVH::BuildSpec{});

  REQUIRE_FALSE(aborts([&topology] {
    Pool         pool(hostMemoryResource());
    const auto   arrays = reserveAndFill<T, K>(pool, topology);
    const Packed bvh(pool, arrays.first, arrays.second);

    (void)bvh;
  }));

  REQUIRE(abortsWith(
    [&topology] {
      Pool       pool(hostMemoryResource());
      const auto arrays = reserveAndFill<T, K>(pool, topology);

      // Turn the root's first slot into an interior slot naming a node far past the array.
      auto& root = arrays.first.at(pool.base(), 0);

      root.m_child[0] = 1000;
      root.m_count[0] = 0;

      const Packed bvh(pool, arrays.first, arrays.second);

      (void)bvh;
    },
    "BVH::PackedBVH: adopted node array is malformed -- node 0 slot 0 names child 1000,"));

  REQUIRE(abortsWith(
    [&topology] {
      Pool       pool(hostMemoryResource());
      const auto arrays = reserveAndFill<T, K>(pool, topology);

      PODVector<Packed::Node> tooLong = arrays.first;

      tooLong.m_capacity += 4096;

      const Packed bvh(pool, tooLong, arrays.second);

      (void)bvh;
    },
    "BVH::PackedBVH: the adopted arrays end at byte"));
}

TEST_CASE("PackedBVH: the pool-resident adopt constructor does not read arrays in device-only memory", "[BVH][adopt]")
{
  using T = double;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, uint32_t, K>;
  using Node   = typename Packed::Node;

  // A pool in device-only memory cannot be read on the host, so arrays adopted in it are the
  // caller's responsibility: even garbage is accepted without being looked at. (A view of it must
  // not be queried on the host, and is not here.) The arrays are reserved in a host pool, filled
  // with garbage, and mirrored into the fake device-only memory, since only a mirror can live there.
  REQUIRE_FALSE(aborts([] {
    FakeDeviceOnlyMemory deviceOnly;
    Pool                 host(hostMemoryResource());

    PODVector<Node>     nodes;
    PODVector<uint32_t> prims;

    nodes.reserveFrom(host, 8);
    prims.reserveFrom(host, 8);

    std::memset(host.base(), 0xAB, host.usedBytes());

    nodes.setSize(8);
    prims.setSize(8);

    host.freeze();

    Pool         device = Pool::mirror(host, deviceOnly);
    const Packed bvh(device, nodes, prims);

    if (!bvh.isAttachedTo(device)) {
      std::abort();
    }
  }));

  // The same garbage in host memory is rejected.
  REQUIRE(aborts([] {
    Pool host(hostMemoryResource());

    PODVector<Node>     nodes;
    PODVector<uint32_t> prims;

    nodes.reserveFrom(host, 8);
    prims.reserveFrom(host, 8);

    std::memset(host.base(), 0xAB, host.usedBytes());

    nodes.setSize(8);
    prims.setSize(8);

    const Packed bvh(host, nodes, prims);

    (void)bvh;
  }));
}

TEMPLATE_TEST_CASE("PackedBVH: an empty BVH has an inverted bounding box and no nodes, visits nothing, and rebases",
                   "[BVH][build]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  const auto requireEmpty = [](const Packed& a_bvh) {
    const AABB box = a_bvh.getBoundingVolume();

    REQUIRE(box.getLowCorner() == Vec3::infinity());
    REQUIRE(box.getHighCorner() == -Vec3::infinity());
    REQUIRE(a_bvh.computeBoundingVolume().getLowCorner() == Vec3::infinity());
    REQUIRE(a_bvh.getNodes().size() == 0);
    REQUIRE(a_bvh.getPrimitives().size() == 0);

    // With nothing pruned, still nothing is visited.
    REQUIRE(visitCounts(a_bvh, Vec3::zeros()) == std::vector<unsigned>{0U});
  };

  REQUIRE(BVH::buildTopology<T, K>(std::vector<AABB>{}, BVH::BuildSpec{}).nodes.empty());

  Pool pool(hostMemoryResource());

  for (const auto& spec : allSpecs(4)) {
    INFO(specName(spec));

    requireEmpty(Packed(pool, std::vector<std::pair<Pnt, AABB>>{}, spec));
  }

  const Packed bvh(pool, std::vector<std::pair<Pnt, AABB>>{});

  requireEmpty(bvh);

  Pool other(hostMemoryResource());

  requireEmpty(bvh.deepCopy(other));

  pool.freeze();

  const Pool hostMirror = Pool::mirror(pool, hostMemoryResource());

  requireEmpty(bvh.rebasedView(hostMirror));

  FakeManagedMemory managed;
  const Pool        managedMirror = Pool::mirror(pool, managed);

  requireEmpty(bvh.rebasedView(managedMirror));
}

// ─────────────────────────────────────────────────────────────────────────────
// Mesh SDFs and the traversal over real geometry
// ─────────────────────────────────────────────────────────────────────────────

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

TEMPLATE_TEST_CASE("PackedBVH over DCEL faces: signedDistance agrees with the brute-force mesh scan, for every "
                   "build specification",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Face = DCEL::FaceT<T, Meta>;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

  const auto facesAndBVs = facesAndBoxes(mesh);

  REQUIRE(facesAndBVs.size() == 36);

  const FlatMeshSDF<T, Meta> flat(mesh, pool);

  for (const uint32_t leaf : {1U, 4U}) {
    for (const auto& spec : allSpecs(leaf)) {
      INFO(specName(spec));

      const BVH::PackedBVH<T, Face, K> packed(pool, facesAndBVs, spec);

      REQUIRE(packed.getPrimitives().size() == 36);

      // PackedBVH has no signedDistance() of its own -- callers build their own thin wrapper around
      // pruneTraverse(), exactly as MeshSDF/TriMeshSDF::signedDistance() do.
      const auto faces = packed.getPrimitives();

      for (const auto& p : queryPoints<T>()) {
        T state = std::numeric_limits<T>::max();

        const auto evalLeaf = [&faces, &p, &mesh](T& a_state, size_t a_offset, size_t a_count) noexcept {
          for (size_t i = 0; i < a_count; i++) {
            const T d = faces[static_cast<uint32_t>(a_offset + i)].signedDistance(p, mesh);

            if (std::abs(d) < std::abs(a_state)) {
              a_state = d;
            }
          }
        };

        const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

        packed.pruneTraverse(p, state, evalLeaf, pruneDist2);

        REQUIRE_THAT(state, withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
      }
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH::pruneTraverse: every compiled SIMD child-distance path agrees "
                   "bit-for-bit with the scalar path",
                   "[BVH][Dodecahedron][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

  const auto facesAndBVs = facesAndBoxes(mesh);

  REQUIRE(facesAndBVs.size() == 36);

  // pruneTraverse() dispatches its per-child box-distance computation on (T, K) and the compiled
  // ISA: AVX-512F claims (double, K=8) and (float, K=16), AVX claims (double, K=4), (float, K=8)
  // and (double, K=8), SSE4.1 claims (float, K=4), and everything else runs the scalar loop. No K
  // value has a SIMD path in every build, and K = 3, 5, 6, 7 never have one in any build -- so
  // sweeping K covers the scalar path and whichever vector paths this binary compiled, and requires
  // them to agree.
  //
  // Equality is exact, not within a margin. Each path computes the same quantity in the same
  // association order, so "close enough" would hide precisely the kind of drift this pins down.
  const std::vector<T> reference = nearestDist2PerQueryPoint<T, 3>(mesh, facesAndBVs);

  REQUIRE(reference.size() == queryPoints<T>().size());

  const auto requireAgreesWithReference = [&reference](const char* a_label, const std::vector<T>& a_values) {
    INFO("Branching factor: " << a_label);

    REQUIRE(a_values.size() == reference.size());

    for (size_t i = 0; i < reference.size(); i++) {
      INFO("Query point index: " << i);

      REQUIRE(a_values[i] == reference[i]);
    }
  };

  requireAgreesWithReference("K=2", nearestDist2PerQueryPoint<T, 2>(mesh, facesAndBVs));
  requireAgreesWithReference("K=4", nearestDist2PerQueryPoint<T, 4>(mesh, facesAndBVs));
  requireAgreesWithReference("K=5", nearestDist2PerQueryPoint<T, 5>(mesh, facesAndBVs));
  requireAgreesWithReference("K=6", nearestDist2PerQueryPoint<T, 6>(mesh, facesAndBVs));
  requireAgreesWithReference("K=7", nearestDist2PerQueryPoint<T, 7>(mesh, facesAndBVs));
  requireAgreesWithReference("K=8", nearestDist2PerQueryPoint<T, 8>(mesh, facesAndBVs));
  requireAgreesWithReference("K=16", nearestDist2PerQueryPoint<T, 16>(mesh, facesAndBVs));
}

TEMPLATE_TEST_CASE("MeshSDF: signedDistance agrees with FlatMeshSDF for every build specification",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.stl"), pool);

  const FlatMeshSDF<T, Meta> flat(mesh, pool);

  for (const auto& spec : allSpecs(4)) {
    INFO(specName(spec));

    const MeshSDF<T, Meta, K> packed(mesh, pool, spec);

    for (const auto& p : queryPoints<T>()) {
      REQUIRE_THAT(packed.signedDistance(p), withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("TriMeshSDF: signedDistance agrees with FlatMeshSDF and MeshSDF for every build specification",
                   "[BVH][Dodecahedron][TriMesh]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.ply"), pool);

  const FlatMeshSDF<T, Meta> flat(mesh, pool);
  const MeshSDF<T, Meta, K>  packed(mesh, pool, BVH::BuildSpec{});

  // Leaf sizes below, at, and above one SIMD group of W triangles.
  for (const uint32_t leaf : {1U, uint32_t(W), uint32_t(2 * W)}) {
    for (const auto& spec : allSpecs(leaf)) {
      INFO(specName(spec));

      const TriMeshSDF<T, Meta, K, W> tri(mesh, pool, spec);

      for (const auto& p : queryPoints<T>()) {
        REQUIRE_THAT(tri.signedDistance(p), withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
        REQUIRE_THAT(tri.signedDistance(p), withinAbsT(packed.signedDistance(p), traversalMargin<T>()));
      }
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
  const MeshSDF<T, Meta, K>       packed(sliver, pool, BVH::BuildSpec{});
  const TriMeshSDF<T, Meta, K, W> tri(sliver, pool, sahSpec(2 * W));

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
  const MeshSDF<T, Meta, K>       packed(mesh, pool, BVH::BuildSpec{});
  const TriMeshSDF<T, Meta, K, W> tri(mesh, pool, sahSpec(2 * W));

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
  const MeshSDF<T, Meta, K>       packed(mesh, pool, BVH::BuildSpec{});
  const TriMeshSDF<T, Meta, K, W> tri(mesh, pool, sahSpec(2 * W));

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
  const TriMeshSDF<T, Meta, K, W> tri(cube, pool, sahSpec(2 * W));
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

  const MeshSDF<T, Meta, K> packed(mesh, pool, BVH::BuildSpec{});

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

  for (const auto& spec : allSpecs(2)) {
    INFO(specName(spec));

    const TriMeshSDF<T, Meta, K, W> tri(tris, pool, spec);

    for (int i = 0; i < N; i++) {
      const Vec3 q(T(3 * i) + T(0.25), T(0.25), T(0.2)); // unambiguously nearest to triangle i

      const auto closest = tri.getClosestTriangle(q);

      REQUIRE(closest.metaData == static_cast<Meta>(100 + i));
      REQUIRE_THAT(closest.signedDistance, withinAbsT(tri.signedDistance(q), traversalMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH::pruneTraverse: nearest-neighbor search over a primitive with no "
                   "signedDistance() matches a brute-force scan",
                   "[BVH][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  const auto positions = gridPositions<T>();

  REQUIRE(positions.size() == 125);

  // The state is a running *squared* distance -- no abs(), no extra squaring for pruning, unlike
  // signedDistance()'s state -- exactly the shape a point-cloud nearest-neighbor search wants.
  const BVH::PackedBVH<T, BareTestPoint<T>, K> packed(pool, pointPrims(positions));

  REQUIRE(packed.getPrimitives().size() == positions.size());

  for (const auto& q : queryPoints<T>()) {
    REQUIRE(nearestPoint2<T, K>(packed, q) == bruteNearestPoint2(positions, q));
  }
}

// Regression test for an O(N^2) build: a per-leaf reserve(size + leafSize) that defeated
// std::vector's geometric growth once reallocated the whole primitive buffer on every leaf. The
// primitive count is deliberately large: a reintroduced quadratic would blow well past the unit-test
// timeout here (minutes), while the linear build stays well under a second. Double only -- the
// append path is precision-independent, and one heavy build is enough of a guard.
TEST_CASE("PackedBVH: a large build stays linear and correct", "[BVH][traverse][regression]")
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
    const T x = dist(rng);
    const T y = dist(rng);
    const T z = dist(rng);

    positions.emplace_back(x, y, z);
  }

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  // Both building constructors: from (primitive, box) pairs, and from a topology with a packer that
  // appends one leaf at a time.
  const Packed direct(pool, pointPrims(positions));

  std::vector<AABB> boxes;

  boxes.reserve(N);

  for (const auto& pos : positions) {
    boxes.emplace_back(pos, pos);
  }

  const Packed packed(pool,
                      BVH::buildTopology<T, K>(boxes, BVH::BuildSpec{}),
                      [&positions](const uint32_t* a_items, const uint32_t a_count, std::vector<Pnt>& a_out) {
                        for (uint32_t i = 0; i < a_count; i++) {
                          a_out.push_back(Pnt{positions[a_items[i]]});
                        }
                      });

  REQUIRE(direct.getPrimitives().size() == N);
  REQUIRE(packed.getPrimitives().size() == N);

  for (const auto& q : queryPoints<T>()) {
    const T brute2 = bruteNearestPoint2(positions, q);

    REQUIRE(nearestPoint2<T, K>(direct, q) == brute2);
    REQUIRE(nearestPoint2<T, K>(packed, q) == brute2);
  }
}

namespace {

// First slot of a BVH whose box is not exactly the union of what it holds, or an empty string.
template <class T, class P, size_t K, class BVConstructor>
std::string
refitDefect(const BVH::PackedBVH<T, P, K>& a_bvh, const BVConstructor& a_bvConstructor)
{
  const auto nodes = a_bvh.getNodes();
  const auto prims = a_bvh.getPrimitives();

  std::ostringstream defect;

  for (uint32_t n = 0; n < nodes.size(); n++) {
    for (size_t k = 0; k < K; k++) {
      if (nodes[n].isEmpty(k)) {
        continue;
      }

      BoundingVolumes::AABBT<T> expected;

      if (nodes[n].isLeaf(k)) {
        const uint32_t first = nodes[n].getPrimitivesOffset(k);

        for (uint32_t p = first; p < first + nodes[n].getNumPrimitives(k); p++) {
          expected = expected.merged(a_bvConstructor(prims[p]));
        }
      }
      else {
        expected = nodes[nodes[n].getChild(k)].getBoundingVolume();
      }

      const auto slot = nodes[n].getBoundingVolume(k);

      if (!(slot.getLowCorner() == expected.getLowCorner() && slot.getHighCorner() == expected.getHighCorner())) {
        defect << "node " << n << " slot " << k << " has box " << slot << ", but encloses " << expected;

        return defect.str();
      }
    }
  }

  return defect.str();
}

} // namespace

TEMPLATE_TEST_CASE("PackedBVH::refit: after the primitives move, every slot box is again exactly the union of what "
                   "it holds, and queries stay exact",
                   "[BVH][refit]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;
  using Node   = typename Packed::Node;

  // A single primitive's current bounding volume: a zero-extent box at its (possibly moved) position.
  const auto bvConstructor = [](const Pnt& a_p) noexcept -> AABB { return AABB(a_p.m_pos, a_p.m_pos); };

  const auto positions = gridPositions<T>();

  for (const auto& spec : allSpecs(3)) {
    INFO(specName(spec));

    Pool   pool(hostMemoryResource());
    Packed bvh(pool, pointPrims(positions), spec);

    REQUIRE(refitDefect(bvh, bvConstructor) == "");

    // Refitting unchanged geometry reproduces every box exactly.
    const auto              nodeSpan = bvh.getNodes();
    const std::vector<Node> before(nodeSpan.begin(), nodeSpan.end());

    bvh.refit(bvConstructor);

    for (uint32_t n = 0; n < before.size(); n++) {
      for (size_t k = 0; k < K; k++) {
        for (size_t dir = 0; dir < 3; dir++) {
          REQUIRE(bvh.getNodes()[n].m_lo[dir][k] == before[n].m_lo[dir][k]);
          REQUIRE(bvh.getNodes()[n].m_hi[dir][k] == before[n].m_hi[dir][k]);
        }
      }
    }

    // Displace every point by a per-point, non-uniform translation, through the BVH's own mutable
    // primitive array: primitives shift without migrating between leaves -- the refit use case.
    auto prims = bvh.getPrimitives();

    std::vector<Vec3> moved;

    for (uint32_t i = 0; i < prims.size(); i++) {
      const T s = T(0.05) * T(i);

      prims[i].m_pos += Vec3(T(2.0) + s, T(-1.0) - s, T(0.5) * s);

      moved.push_back(prims[i].m_pos);
    }

    bvh.refit(bvConstructor);

    REQUIRE(refitDefect(bvh, bvConstructor) == "");

    // The root is the tight box over the moved cloud.
    Vec3 lo = Vec3::infinity();
    Vec3 hi = -Vec3::infinity();

    for (const auto& pos : moved) {
      lo = min(lo, pos);
      hi = max(hi, pos);
    }

    REQUIRE(bvh.getBoundingVolume().getLowCorner() == lo);
    REQUIRE(bvh.getBoundingVolume().getHighCorner() == hi);

    for (const auto& q : queryPoints<T>()) {
      REQUIRE(nearestPoint2<T, K>(bvh, q) == bruteNearestPoint2(moved, q));
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

  const MeshSDF<T, Meta, K> expected(direct, pool, BVH::BuildSpec{});
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
  // never satisfy. The nodes are trivially copyable for the same reason.
  static_assert(std::is_trivially_copyable_v<Pnt>);
  static_assert(std::is_trivially_copyable_v<Packed>);
  static_assert(std::is_trivially_copyable_v<typename Packed::Node>);
  static_assert(std::is_same_v<typename Packed::Node, BVH::WideNode<T, K>>);
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
  static_assert(std::is_same_v<typename MeshSDF<T, Meta, K>::Root, BVH::PackedBVH<T, Face, K>>);
  static_assert(std::is_same_v<typename TriMeshSDF<T, Meta, K, W>::Root, BVH::PackedBVH<T, TriAoSoA, K>>);
}

TEMPLATE_TEST_CASE("PackedBVH: copy aliases the same pool storage; deepCopy() is what makes it "
                   "independent",
                   "[BVH][Pool]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  // Pool-backed storage changes what a copy means. A PackedBVH is two PODVector descriptors plus a
  // pool location -- trivially copyable by design, since that is what lets a rebased view be
  // byte-copied into a device address space -- so copying one copies offsets, not data.
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

  const Packed original(
    pool, pointPrims(positions), BVH::BuildSpec{BVH::Strategy::SpaceFillingCurve, BVH::Curve::Morton, 2});

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
    REQUIRE(independent.getNodes().size() == original.getNodes().size());

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
      const T d2 = (m_prims[static_cast<uint32_t>(a_offset + i)].m_pos - m_query).length2();

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
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, BareTestPoint<T>, K>;

  // Mirroring host-to-host exercises the whole rebase invariant -- offsets resolving against a
  // different base -- without needing a GPU, which is the only way it runs in CI at all.
  std::vector<Vec3> positions;

  positions.reserve(40);

  for (int i = 0; i < 40; i++) {
    const T t = T(i);

    positions.emplace_back(std::sin(t) * t, std::cos(t) * t, T(0.3) * t);
  }

  for (const auto& spec : allSpecs(3)) {
    INFO(specName(spec));

    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, pointPrims(positions), spec);

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
  const TriMeshSDF<T, Meta, K, W> fromMesh(mesh, pool, sahSpec(2 * W));
  const TriMeshSDF<T, Meta, K, W> fromSoup(triangles, pool, sahSpec(2 * W));

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
  const AABB                 fromFlat     = flat.computeBoundingVolume();
  const AABB                 fromVertices = AABB(mesh.getAllVertexCoordinates());

  REQUIRE(fromFlat.getLowCorner() == fromVertices.getLowCorner());
  REQUIRE(fromFlat.getHighCorner() == fromVertices.getHighCorner());

  for (const auto& spec : allSpecs(4)) {
    INFO(specName(spec));

    const MeshSDF<T, Meta, K> meshSDF(mesh, pool, spec);
    const AABB                fromBVH = meshSDF.computeBoundingVolume();

    REQUIRE(fromFlat.getLowCorner() == fromBVH.getLowCorner());
    REQUIRE(fromFlat.getHighCorner() == fromBVH.getHighCorner());
  }
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
  const Mesh meshSDF(mesh, pool, BVH::BuildSpec{});
  const Tri  triSDF(mesh, pool, sahSpec(2 * W));

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

TEMPLATE_TEST_CASE("PackedBVH: every build specification finds the brute-force nearest point, for leaf sizes that "
                   "do and do not divide the point count, and for a single primitive",
                   "[BVH][build][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  const auto positions = gridPositions<T>();

  // Leaf sizes that divide 125 into whole leaves and leaf sizes that leave a remainder, up to one
  // leaf holding everything.
  for (const uint32_t leaf : {1U, 4U, 7U, 25U, 125U}) {
    for (const auto& spec : allSpecs(leaf)) {
      INFO(specName(spec));

      const BVH::PackedBVH<T, Pnt, K> packed(pool, pointPrims(positions), spec);

      REQUIRE(packed.getPrimitives().size() == positions.size());

      for (const auto& q : queryPoints<T>()) {
        REQUIRE(nearestPoint2<T, K>(packed, q) == bruteNearestPoint2(positions, q));
      }
    }
  }

  SECTION("A single primitive: the root holds one leaf")
  {
    const std::vector<Vec3> only{Vec3(T(1), T(2), T(3))};

    for (const uint32_t leaf : {1U, 8U}) {
      for (const auto& spec : allSpecs(leaf)) {
        INFO(specName(spec));

        const BVH::PackedBVH<T, Pnt, K> packed(pool, pointPrims(only), spec);

        REQUIRE(packed.getNodes().size() == 1);
        REQUIRE(packed.getNodes()[0].numChildren() == 1);
        REQUIRE(packed.getNodes()[0].isLeaf(0));

        for (const auto& q : queryPoints<T>()) {
          REQUIRE(nearestPoint2<T, K>(packed, q) == pointDistance2(only[0], q));
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH: a uint32_t-index BVH agrees exactly with one storing the primitives themselves, for "
                   "every build specification",
                   "[BVH][build][PrimitiveStorage]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  // A BVH whose primitive *is* a uint32_t index into a caller-owned array is the supported way to
  // build an indexed BVH: PackedBVH<T, uint32_t, K> stores four bytes per primitive and the leaf
  // callback resolves the index against whatever array the caller owns. The builder only reads the
  // boxes, so both BVHs have the same shape and must agree exactly -- bit for bit, not just to a
  // tolerance.
  Pool pool(hostMemoryResource());

  std::vector<Pnt> owner;

  owner.reserve(29);

  for (int i = 0; i < 29; i++) {
    const T t = T(i);

    owner.push_back(Pnt{Vec3(std::cos(t) * t, std::sin(t) * t, T(0.4) * t)});
  }

  std::vector<std::pair<Pnt, AABB>>      valuePrims;
  std::vector<std::pair<uint32_t, AABB>> indexPrims;

  for (uint32_t i = 0; i < owner.size(); i++) {
    const AABB bv(owner[i].m_pos, owner[i].m_pos);

    valuePrims.emplace_back(owner[i], bv);
    indexPrims.emplace_back(i, bv);
  }

  // Four bytes per primitive, and still trivially copyable -- so an indexed BVH mirrors to a device
  // exactly like any other.
  static_assert(sizeof(uint32_t) == 4);
  static_assert(std::is_trivially_copyable_v<BVH::PackedBVH<T, uint32_t, K>>);

  for (const auto& spec : allSpecs(5)) {
    INFO(specName(spec));

    const BVH::PackedBVH<T, Pnt, K>      valueBVH(pool, valuePrims, spec);
    const BVH::PackedBVH<T, uint32_t, K> indexBVH(pool, indexPrims, spec);

    REQUIRE(valueBVH.getPrimitives().size() == owner.size());
    REQUIRE(indexBVH.getPrimitives().size() == owner.size());

    const auto indexPrimArray = indexBVH.getPrimitives();

    for (const auto& q : queryPoints<T>()) {
      T indexState = std::numeric_limits<T>::infinity();

      // The leaf callback resolves each stored index against the array the caller owns, rather than
      // against anything the BVH holds.
      const auto indexEval = [&indexPrimArray, &owner, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = a_offset; i < a_offset + a_count; i++) {
          a_state = std::min(a_state, pointDistance2(owner[indexPrimArray[static_cast<uint32_t>(i)]].m_pos, q));
        }
      };

      indexBVH.pruneTraverse(q, indexState, indexEval, [](const T& a_state) noexcept -> T { return a_state; });

      T bruteMin2 = std::numeric_limits<T>::infinity();

      for (const auto& pnt : owner) {
        bruteMin2 = std::min(bruteMin2, pointDistance2(pnt.m_pos, q));
      }

      REQUIRE(indexState == nearestPoint2<T, K>(valueBVH, q));
      REQUIRE(indexState == bruteMin2);
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH: the (primitive, box) constructor agrees exactly with buildTopology plus the "
                   "leaf-packing constructor",
                   "[BVH][build]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  // The two ways to build the same BVH: the constructor that builds and stores in one step, and the
  // shape from buildTopology() stored through a packer that copies each leaf's items unchanged. They
  // must produce identical node arrays and identical primitive arrays.
  const auto positions = gridPositions<T>();

  std::vector<AABB> boxes;

  for (const auto& pos : positions) {
    boxes.emplace_back(pos, pos);
  }

  const auto identity = [&positions](const uint32_t* a_items, const uint32_t a_count, std::vector<Pnt>& a_out) {
    for (uint32_t i = 0; i < a_count; i++) {
      a_out.push_back(Pnt{positions[a_items[i]]});
    }
  };

  for (const uint32_t leaf : {1U, 3U, 8U}) {
    for (const auto& spec : allSpecs(leaf)) {
      INFO(specName(spec));

      Pool         pool(hostMemoryResource());
      const Packed direct(pool, pointPrims(positions), spec);
      const Packed viaTopology(pool, BVH::buildTopology<T, K>(boxes, spec), identity);

      const auto directNodes = direct.getNodes();
      const auto otherNodes  = viaTopology.getNodes();

      REQUIRE(directNodes.size() == otherNodes.size());
      REQUIRE(direct.getPrimitives().size() == viaTopology.getPrimitives().size());

      for (uint32_t n = 0; n < directNodes.size(); n++) {
        for (size_t k = 0; k < K; k++) {
          REQUIRE(directNodes[n].m_child[k] == otherNodes[n].m_child[k]);
          REQUIRE(directNodes[n].m_count[k] == otherNodes[n].m_count[k]);

          for (size_t dir = 0; dir < 3; dir++) {
            REQUIRE(directNodes[n].m_lo[dir][k] == otherNodes[n].m_lo[dir][k]);
            REQUIRE(directNodes[n].m_hi[dir][k] == otherNodes[n].m_hi[dir][k]);
          }
        }
      }

      for (uint32_t i = 0; i < direct.getPrimitives().size(); i++) {
        REQUIRE(direct.getPrimitives()[i].m_pos == viaTopology.getPrimitives()[i].m_pos);
      }
    }
  }
}

TEST_CASE("PackedBVH: rebasedView rejects a pool that is not a mirror of its own", "[BVH][rebase][death]")
{
  using T = double;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, BareTestPoint<T>, K>;

  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());
      Pool unrelated(hostMemoryResource());

      std::vector<Vec3T<T>> positions;

      for (int i = 0; i < 8; i++) {
        positions.emplace_back(T(i), T(0), T(0));
      }

      const Packed bvh(pool, pointPrims(positions), sahSpec(2));
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

      const MeshSDF<T, Meta, K> sdf(mesh, other, BVH::BuildSpec{});
    },
    "MeshSDF: the mesh must live in the pool passed in"));

  REQUIRE(abortsWith(
    [] {
      Pool       pool(hostMemoryResource());
      const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

      const MeshSDF<T, Meta, K> sdf(mesh, pool, sahSpec(0));
    },
    "BVH::BuildSpec: the maximum leaf size must be positive"));

  REQUIRE(abortsWith(
    [] {
      Pool       pool(hostMemoryResource());
      const auto mesh = Parser::readIntoDCEL<T, Meta>(dataPath("dodecahedron.obj"), pool);

      const TriMeshSDF<T, Meta, K, W> sdf(mesh, pool, sahSpec(0));
    },
    "BVH::BuildSpec: the maximum leaf size must be positive"));
}

TEMPLATE_TEST_CASE("PackedBVH: the host-array adopt constructor rebuilds an identical BVH from getNodes() and "
                   "getPrimitives()",
                   "[BVH][adopt]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;
  using Node   = typename Packed::Node;

  Pool pool(hostMemoryResource());

  std::vector<Vec3> positions;

  for (int i = 0; i < 200; i++) {
    const T t = T(i) * T(0.1);

    positions.emplace_back(std::sin(t) * t, std::cos(t) * t, T(0.3) * t);
  }

  const Packed built(pool, pointPrims(positions));

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

TEMPLATE_TEST_CASE("PackedBVH over triangles: signedDistance agrees with the brute-force mesh scan, for every build "
                   "specification and both building constructors (cheap fixture: tetrahedron)",
                   "[BVH][Tetrahedron][build]",
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

  std::vector<std::pair<Tri, AABB>> trisAndBVs;
  std::vector<AABB>                 boxes;

  for (const auto& tri : triangles) {
    const auto&                 vp = tri.getVertexPositions();
    const std::vector<Vec3T<T>> verts{vp[0], vp[1], vp[2]};

    trisAndBVs.emplace_back(tri, AABB(verts));
    boxes.emplace_back(verts);
  }

  const auto check = [&](const BVH::PackedBVH<T, Tri, K>& a_packed) {
    REQUIRE(a_packed.getPrimitives().size() == 4);

    const auto prims = a_packed.getPrimitives();

    for (const auto& p : queryPoints<T>()) {
      T state = std::numeric_limits<T>::max();

      const auto evalLeaf = [&prims, &p](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          const T d = prims[static_cast<uint32_t>(a_offset + i)].signedDistance(p);

          if (std::abs(d) < std::abs(a_state)) {
            a_state = d;
          }
        }
      };

      const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state * a_state; };

      a_packed.pruneTraverse(p, state, evalLeaf, pruneDist2);

      REQUIRE_THAT(state, withinAbsT(flat.signedDistance(p), traversalMargin<T>()));
    }
  };

  const auto copyLeaf = [&triangles](const uint32_t* a_items, const uint32_t a_count, std::vector<Tri>& a_out) {
    for (uint32_t i = 0; i < a_count; i++) {
      a_out.push_back(triangles[a_items[i]]);
    }
  };

  for (const uint32_t leaf : {1U, 2U, 4U}) {
    for (const auto& spec : allSpecs(leaf)) {
      INFO(specName(spec));

      check(BVH::PackedBVH<T, Tri, K>(pool, trisAndBVs, spec));
      check(BVH::PackedBVH<T, Tri, K>(pool, BVH::buildTopology<T, K>(boxes, spec), copyLeaf));
    }
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

    primitives.emplace_back(shifted, pool, sahSpec(2 * W));
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
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, BareTestPoint<T>, K>;

  std::vector<Vec3> positions;

  positions.reserve(64);

  for (int i = 0; i < 64; i++) {
    const T t = T(i);

    positions.emplace_back(std::sin(t) * t, std::cos(t) * t, T(0.2) * t);
  }

  // The points spiral out to a radius of 63 in the xy-plane and rise to z = 12.6, so this grid
  // covers query points among them as well as well outside the cloud.
  const auto points = queryGrid<T>(Vec3(T(-70), T(-70), T(-5)), Vec3(T(70), T(70), T(18)), 10);

  for (const auto& spec : allSpecs(4)) {
    INFO(specName(spec));

    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, pointPrims(positions), spec);

    pool.freeze();

    Pool         devicePool = Pool::mirror(pool, deviceTestResource());
    const Packed deviceView = bvh.rebasedView(devicePool);

    requireSameResults(evaluateOnDevice<T>(PackedBvhTraversalQuery<T, K>{deviceView}, points),
                       evaluateOnHost<T>(PackedBvhTraversalQuery<T, K>{bvh}, points));
  }
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
  const MeshSDF<T, Meta, K>       meshSDF(mesh, pool, BVH::BuildSpec{});
  const TriMeshSDF<T, Meta, K, W> triSDF(mesh, pool, sahSpec(2 * W));

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
