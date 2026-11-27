// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test suite for PackedBVH's layout in its pool: the node and box arrays, copies, deep copies and views, the
// adopt constructor, and the depth limits of the traversal stacks.

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

TEMPLATE_TEST_CASE("PackedBVH: every box is stored once, and any node's box is found through the node",
                   "[BVH][PackedBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using AABB   = BoundingVolumes::AABBT<T>;
  using Vec3   = Vec3T<T>;
  using Pnt    = BareTestPoint<T>;
  using Packed = BVH::PackedBVH<T, Pnt, 4>;
  using Node   = typename Packed::Node;

  // A node holds its leaf range, its child offsets, its box row and its box slot, and no box.
  STATIC_REQUIRE(sizeof(Node) == 32);

  std::vector<std::pair<Pnt, AABB>> flat;

  for (int i = 0; i < 300; i++) {
    const T    t = T(i) * T(0.07);
    const Vec3 p(std::sin(t) * t, std::cos(t) * t, T(0.2) * t);

    flat.emplace_back(Pnt{p}, AABB(p, p));
  }

  Pool         pool(hostMemoryResource());
  const Packed bvh = Packed::buildBottomUp(pool, flat, size_t(4));

  const auto sameBox = [](const AABB& a_lhs, const AABB& a_rhs) {
    return a_lhs.getLowCorner() == a_rhs.getLowCorner() && a_lhs.getHighCorner() == a_rhs.getHighCorner();
  };

  // Each node's box is the union of its primitives' (a leaf) or of its children's boxes (an interior
  // node), so the box found through the slot is the node's own; the root's is the BVH's.
  const auto checkBoxes = [&sameBox](const Packed& a_bvh) {
    const auto nodes = a_bvh.getNodes();
    const auto prims = a_bvh.getPrimitives();

    REQUIRE(nodes[0].getBoxSlot() == Packed::RootBoxSlot);
    REQUIRE(sameBox(a_bvh.getBoundingVolume(nodes[0]), a_bvh.getBoundingVolume()));

    for (uint32_t i = 0; i < nodes.size(); i++) {
      const Node& node = nodes[i];
      AABB        expected;

      if (node.isLeaf()) {
        for (uint32_t p = 0; p < node.getNumPrimitives(); p++) {
          const Vec3& x = prims[node.getPrimitivesOffset() + p].m_pos;

          expected = expected.merged(AABB(x, x));
        }
      }
      else {
        for (const uint32_t child : node.getChildOffsets()) {
          expected = expected.merged(a_bvh.getBoundingVolume(nodes[child]));
        }
      }

      const AABB box = a_bvh.getBoundingVolume(node);

      REQUIRE(sameBox(box, expected));

      const Vec3 q(T(3), T(-2), T(1));

      // The same box, without assembling it: equal up to the order of the operations.
      REQUIRE_THAT(a_bvh.getDistanceToBoundingVolume2(node, q), Catch::Matchers::WithinRel(box.getDistance2(q)));

      // A child's box read from this node's row is the child's own box (checked against its primitives
      // when the child is visited).
      if (!node.isLeaf()) {
        for (std::size_t k = 0; k < node.getChildOffsets().size(); k++) {
          const AABB childBox = a_bvh.getBoundingVolume(nodes[node.getChildOffsets()[k]]);

          REQUIRE_THAT(a_bvh.getDistanceToChildBoundingVolume2(node, k, q),
                       Catch::Matchers::WithinRel(childBox.getDistance2(q)));
        }
      }
    }
  };

  checkBoxes(bvh);

  SECTION("a refit through a copy is seen by the original, since every box is in the pool")
  {
    Packed copy = bvh;

    for (auto& p : copy.getPrimitives()) {
      p.m_pos += Vec3(T(1), T(0), T(0));
    }

    const AABB before = bvh.getBoundingVolume();

    copy.refit([](const Pnt& a_p) noexcept { return AABB(a_p.m_pos, a_p.m_pos); });

    REQUIRE(bvh.getBoundingVolume().getLowCorner()[0] == before.getLowCorner()[0] + T(1));
    REQUIRE(bvh.getBoundingVolume().getHighCorner()[0] == before.getHighCorner()[0] + T(1));

    checkBoxes(bvh);
  }

  SECTION("a deep copy and an adopted copy keep every node's box")
  {
    const Packed deep = bvh.deepCopy(pool);

    checkBoxes(deep);

    const auto        nodeSpan = bvh.getNodes();
    std::vector<Node> nodes(nodeSpan.begin(), nodeSpan.end());
    std::vector<AABB> boxes;

    for (const Node& node : nodes) {
      boxes.push_back(bvh.getBoundingVolume(node));
    }

    const auto       primSpan = bvh.getPrimitives();
    std::vector<Pnt> prims(primSpan.begin(), primSpan.end());

    const Packed adopted(pool, nodes, boxes, prims);

    checkBoxes(adopted);

    // The adopting constructor needs one box per node.
    boxes.pop_back();

    REQUIRE(abortsWith(
      [&] {
        Pool         other(hostMemoryResource());
        const Packed bad(other, nodes, boxes, prims);

        (void)bad;
      },
      "node boxes for"));
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

  const Packed original = Packed::buildBottomUp(pool, flat, size_t(2));

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

TEMPLATE_TEST_CASE("PackedBVH: a tree too deep for traverse()'s fixed stack is rejected at build time, at every "
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
    // The node array of the chain; every node has the same box (see boxesFor).
    const auto chain = [](const size_t a_interior) {
      std::vector<Node> nodes(a_interior);

      const auto addLeaf = [&nodes]() {
        Node leaf{};

        leaf.setPrimitivesOffset(0);
        leaf.setNumPrimitives(1);

        nodes.push_back(leaf);

        return static_cast<uint32_t>(nodes.size() - 1);
      };

      for (size_t i = 0; i < a_interior; i++) {
        const uint32_t next = (i + 1 < a_interior) ? static_cast<uint32_t>(i + 1) : addLeaf();

        nodes[i].setChildOffset(next, 0);

        for (size_t k = 1; k < K; k++) {
          const uint32_t leaf = addLeaf();

          nodes[i].setChildOffset(leaf, k);
        }
      }

      return nodes;
    };

    const auto boxesFor = [&box](const std::vector<Node>& a_nodes) { return std::vector<AABB>(a_nodes.size(), box); };

    const size_t limit = BVH::HostTraversalDepth;

    REQUIRE_FALSE(aborts([&] {
      Pool         pool(hostMemoryResource());
      const Packed bvh(pool, chain(limit - 1), boxesFor(chain(limit - 1)), prims);

      (void)bvh;
    }));

    REQUIRE(abortsWith(
      [&] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, chain(limit), boxesFor(chain(limit)), prims);

        (void)bvh;
      },
      "host build -- the tree is " + std::to_string(limit + 1) + " levels deep, more than the " +
        std::to_string(limit) + " levels a traversal stack holds"));

    // A tree at the host limit traverses without overflowing the stack sized for it, and visits
    // every leaf.
    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, chain(limit - 1), boxesFor(chain(limit - 1)), prims);

    size_t visits = 0;
    T      state  = T(0);

    bvh.traverse(
      BVH::PointQuery{Vec3(T(0.5), T(0.5), T(0.5))},
      state,
      [&visits](T&, size_t, size_t a_count) noexcept { visits += a_count; },
      [](const T&) noexcept -> T { return std::numeric_limits<T>::infinity(); });

    REQUIRE(visits == 1 + (limit - 1) * (K - 1));

    // A device view is checked against the shallower device limit when it is made.
    const size_t deviceLimit = BVH::DeviceTraversalDepth;

    const auto deviceView = [&](const size_t a_interior) {
      return [&, a_interior] {
        Pool         hostPool(hostMemoryResource());
        const Packed tree(hostPool, chain(a_interior), boxesFor(chain(a_interior)), prims);

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

TEMPLATE_TEST_CASE("PackedBVH: every build of points along a line, each half as far from the origin as the one before, "
                   "is shallow enough for a device, holds every point once in a box around it, and finds the nearest",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  // A split by space (SAH, the midpoint, ClusterSAH's clusters) cuts off about one point per level here, so these
  // builds made trees about as deep as there are points: up to 40 levels for these 120.
  constexpr int N = 120;

  std::vector<Vec3>                 positions;
  std::vector<std::pair<Pnt, AABB>> primsAndBVs;

  for (int i = 0; i < N; i++) {
    const Vec3 x(std::ldexp(T(1), -i), T(0), T(0));

    positions.push_back(x);
    primsAndBVs.emplace_back(Pnt{x}, AABB(x, x));
  }

  // Each point's number, from its distance to the origin (exactly 2^-i).
  const auto numberOf = [](const Pnt& a_point) { return -std::ilogb(a_point.m_pos[0]); };

  const auto check = [&](const Packed& a_bvh) {
    const auto nodes = a_bvh.getNodes();
    const auto prims = a_bvh.getPrimitives();

    std::vector<std::pair<uint32_t, size_t>> stack{{0U, size_t(1)}};
    std::vector<int>                         seen(N, 0);

    size_t depth = 0;

    while (!stack.empty()) {
      const auto [index, level] = stack.back();

      stack.pop_back();

      depth = std::max(depth, level);

      const auto& node = nodes[index];

      if (!node.isLeaf()) {
        for (const uint32_t child : node.getChildOffsets()) {
          stack.emplace_back(child, level + 1);
        }

        continue;
      }

      const AABB box = a_bvh.getBoundingVolume(node);

      for (uint32_t p = node.getPrimitivesOffset(); p < node.getPrimitivesOffset() + node.getNumPrimitives(); p++) {
        const Vec3& x = prims[p].m_pos;

        seen[numberOf(prims[p])]++;

        CHECK(x[0] >= box.getLowCorner()[0]);
        CHECK(x[0] <= box.getHighCorner()[0]);
      }
    }

    CHECK(depth <= BVH::DeviceTraversalDepth);
    CHECK(std::count(seen.begin(), seen.end(), 1) == N);

    const auto pruneDist2 = [](const T& a_state) noexcept -> T { return a_state; };

    for (int j = 0; j < N; j += 7) {
      const Vec3 q(T(0.75) * std::ldexp(T(1), -j), std::ldexp(T(1), -j), T(0));

      T state = std::numeric_limits<T>::max();

      const auto evalLeaf = [&prims, &q](T& a_state, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = 0; i < a_count; i++) {
          a_state = std::min(a_state, (prims[a_offset + i].m_pos - q).length2());
        }
      };

      a_bvh.traverse(BVH::PointQuery{q}, state, evalLeaf, pruneDist2);

      T brute = std::numeric_limits<T>::max();

      for (const auto& x : positions) {
        brute = std::min(brute, (x - q).length2());
      }

      CHECK(state == brute);
    }
  };

  // A device view of each, which is checked against the device depth.
  const auto mirrors = [](Pool& a_pool, const Packed& a_bvh) {
    return !aborts([&] {
      a_pool.freeze();

      DeviceAccessibleHostResource managed;

      const Pool   mirror = Pool::mirror(a_pool, managed);
      const Packed view   = a_bvh.rebasedView(mirror);

      (void)view;
    });
  };

  const std::pair<BVH::Construction, const char*> constructions[] = {
    {BVH::Construction::CentroidSplit, "CentroidSplit"},
    {BVH::Construction::MidpointSplit, "MidpointSplit"},
    {BVH::Construction::SAH, "SAH"},
    {BVH::Construction::ClusterSAH, "ClusterSAH"},
    {BVH::Construction::Morton, "Morton"},
    {BVH::Construction::Nested, "Nested"},
    {BVH::Construction::Hilbert, "Hilbert"}};

  for (const auto& [construction, name] : constructions) {
    INFO(name);

    Pool         pool(hostMemoryResource());
    const Packed bvh = Packed::build(pool, primsAndBVs, construction);

    check(bvh);
    CHECK(mirrors(pool, bvh));
  }

  // A TreeBVH, partitioned by SAH and packed.
  {
    INFO("TreeBVH, BinnedSAHPartitioner, packed");

    BVH::PrimAndBVList<Pnt, AABB> list;

    for (const auto& x : positions) {
      list.emplace_back(std::make_shared<Pnt>(Pnt{x}), AABB(x, x));
    }

    using Tree = BVH::TreeBVH<T, Pnt, AABB, K>;

    auto tree = std::make_shared<Tree>(list);

    const typename Tree::LeafPredicate stopCrit = [](const Tree& a_node) noexcept -> bool {
      return a_node.getPrimitives().size() < K;
    };

    tree->topDownSortAndPartition(BVH::BinnedSAHPartitioner<T, Pnt, AABB, K>, stopCrit);

    Pool         pool(hostMemoryResource());
    const Packed bvh = tree->pack(pool);

    check(bvh);
    CHECK(mirrors(pool, bvh));
  }
}

TEMPLATE_TEST_CASE("PackedBVH: the traversal stack holds one entry per level below the root, at every branching "
                   "factor",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Checked at run time, not with static_assert: in a GPU build this file is also compiled for the
  // device, where the same function returns the device stack size.
  REQUIRE(BVH::PackedBVH<T, Vec3T<T>, 2>::traversalStackDepth() == BVH::HostTraversalDepth - 1);
  REQUIRE(BVH::PackedBVH<T, Vec3T<T>, 4>::traversalStackDepth() == BVH::HostTraversalDepth - 1);
  REQUIRE(BVH::PackedBVH<T, Vec3T<T>, 16>::traversalStackDepth() == BVH::HostTraversalDepth - 1);

  static_assert(BVH::HostTraversalDepth == 256);
  static_assert(BVH::DeviceTraversalDepth == 32);
}

TEMPLATE_TEST_CASE("PackedBVH: a tree at the host depth limit, visited deepest first, fills the traversal stack "
                   "and every leaf is visited once",
                   "[BVH][traverse][traverse]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  // A chain of interior nodes, each with the next node in the chain as its first child and leaves as
  // its others. The chain's boxes contain the query point and the leaves' boxes lie beyond it, so
  // both traversals descend the chain first and keep every level's leaves waiting: at the bottom the
  // stack holds one entry per interior node, its full size. An overrun would trip the stack guard,
  // which this build checks.
  const std::vector<Pnt> prims = {Pnt{Vec3(T(0), T(0), T(0))}};
  const AABB             chainBox(Vec3(T(0), T(0), T(0)), Vec3(T(1), T(1), T(1)));
  const Vec3             query(T(0.5), T(0.5), T(0.5));

  const auto check = [&](auto a_k) {
    constexpr size_t K = decltype(a_k)::value;

    using Packed = BVH::PackedBVH<T, Pnt, K>;
    using Node   = typename Packed::Node;

    INFO("K = " << K);

    const size_t interior = BVH::HostTraversalDepth - 1;

    std::vector<Node> nodes(interior);
    std::vector<AABB> boxes(interior, chainBox);

    const auto addLeaf = [&nodes, &boxes](const size_t a_level, const size_t a_slot) {
      const T    x = T(2 + a_level % 7);
      const T    y = T(2 + a_slot);
      const AABB box(Vec3(x, y, T(0)), Vec3(x + T(1), y + T(1), T(1)));

      Node leaf{};

      leaf.setPrimitivesOffset(0);
      leaf.setNumPrimitives(1);

      nodes.push_back(leaf);
      boxes.push_back(box);

      return static_cast<uint32_t>(nodes.size() - 1);
    };

    for (size_t i = 0; i < interior; i++) {

      const uint32_t next = (i + 1 < interior) ? static_cast<uint32_t>(i + 1) : addLeaf(i, 0);

      nodes[i].setChildOffset(next, 0);

      for (size_t k = 1; k < K; k++) {
        const uint32_t leaf = addLeaf(i, k);

        nodes[i].setChildOffset(leaf, k);
      }
    }

    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, nodes, boxes, prims);

    const size_t numLeaves = 1 + interior * (K - 1);

    // traverse() with no bound: the chain child is nearest at every level.
    size_t visits = 0;
    T      state  = T(0);

    bvh.traverse(
      BVH::PointQuery{query},
      state,
      [&visits](T&, size_t, size_t) noexcept { visits++; },
      [](const T&) noexcept -> T { return std::numeric_limits<T>::infinity(); });

    REQUIRE(visits == numLeaves);

    // The same with FarthestFirst, which never drops the rest of a level: every leaf once.
    size_t farVisits = 0;

    bvh.traverse(
      BVH::PointQuery{query},
      state,
      [&farVisits](T&, size_t, size_t) noexcept { farVisits++; },
      [](const T&) noexcept -> T { return std::numeric_limits<T>::infinity(); },
      BVH::DescendAll{},
      BVH::FarthestFirst{});

    REQUIRE(farVisits == numLeaves);
  };

  check(std::integral_constant<size_t, 2>{});
  check(std::integral_constant<size_t, 4>{});
  check(std::integral_constant<size_t, 16>{});
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

  // Every node gets the same box; the defects below are in the node array alone.
  const auto boxesFor = [&box](const std::vector<Node>& a_nodes) { return std::vector<AABB>(a_nodes.size(), box); };

  for (size_t k = 0; k < K; k++) {
    good[0].setChildOffset(static_cast<uint32_t>(k + 1), k);
    good[k + 1].setPrimitivesOffset(0);
    good[k + 1].setNumPrimitives(2);
  }

  REQUIRE_FALSE(aborts([&] {
    Pool         pool(hostMemoryResource());
    const Packed bvh(pool, good, boxesFor(good), prims);

    (void)bvh;
  }));

  // Each row introduces one defect into the good node array, and the adopt constructor must abort with its message.
  using Defect = std::function<void(std::vector<Node>&)>;

  const std::vector<std::pair<Defect, std::string>> rows{
    // A leaf whose primitive range runs past the primitive array.
    {[&](std::vector<Node>& a_nodes) { a_nodes[K].setNumPrimitives(3); },
     "leaf 4's primitive range ends at 3, past the primitive array's 2 primitives"},
    // A child offset pointing back at its own parent.
    {[&](std::vector<Node>& a_nodes) { a_nodes[0].setChildOffset(0, K - 1); },
     "node 0 has child offset 0, which is not strictly after its parent"},
    // A child offset past the end of the node array.
    {[&](std::vector<Node>& a_nodes) { a_nodes[0].setChildOffset(static_cast<uint32_t>(K + 1), 0); },
     "node 0 has child offset 5, which is not strictly after its parent"},
    // An empty node array paired with a non-empty primitive array.
    {[&](std::vector<Node>& a_nodes) { a_nodes.clear(); }, "it is empty, but the primitive array holds 2 primitives"},
    // A leaf with no primitives, which reads as an interior node whose children are all node 0.
    {[&](std::vector<Node>& a_nodes) { a_nodes[K].setNumPrimitives(0); },
     "node 4 has child offset 0, which is not strictly after its parent"},
    // A node that is the child of two slots.
    {[&](std::vector<Node>& a_nodes) { a_nodes[0].setChildOffset(1, K - 1); },
     "node 0 names child 1, which already has another parent"},
    // A node no slot names.
    {[&](std::vector<Node>& a_nodes) { a_nodes.push_back(a_nodes[1]); }, "node 5 has no parent"},
  };

  for (const auto& [defect, message] : rows) {
    std::vector<Node> bad = good;

    defect(bad);

    INFO(message);
    REQUIRE(abortsWith(
      [&] {
        Pool         pool(hostMemoryResource());
        const Packed bvh(pool, bad, boxesFor(bad), prims);

        (void)bvh;
      },
      message));
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
  const BVH::PackedBVH<T, Vec3, K> bvh =
    BVH::PackedBVH<T, Vec3, K>::buildTopDown(pool, primsAndBVs, BVH::BinnedSAHPartitioner<T, Vec3, AABB, K>);

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

TEMPLATE_TEST_CASE("PackedBVH: an empty BVH has the empty bounding box and visits nothing",
                   "[BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Vec3, K>;

  Pool         pool(hostMemoryResource());
  const Packed bvh(pool, std::vector<typename Packed::Node>{}, std::vector<typename Packed::BV>{}, std::vector<Vec3>{});

  const auto box = bvh.computeBoundingVolume();

  REQUIRE(box.getLowCorner()[0] > box.getHighCorner()[0]);

  size_t visits = 0;
  T      state  = T(0);

  bvh.traverse(
    BVH::PointQuery{Vec3(T(0), T(0), T(0))},
    state,
    [&visits](T&, size_t, size_t a_count) noexcept { visits += a_count; },
    [](const T&) noexcept { return std::numeric_limits<T>::infinity(); });

  REQUIRE(visits == 0);
}

TEST_CASE("PackedBVH: getBoundingVolume on an empty BVH aborts", "[BVH][death]")
{
  using T    = double;
  using Vec3 = Vec3T<T>;

  using Packed = BVH::PackedBVH<T, Vec3, 4>;

  // It used to read node 0 of an empty node array in a build without assertions.
  REQUIRE(abortsWith(
    [] {
      Pool         pool(hostMemoryResource());
      const Packed bvh(
        pool, std::vector<typename Packed::Node>{}, std::vector<typename Packed::BV>{}, std::vector<Vec3>{});

      volatile T sink = bvh.getBoundingVolume().getLowCorner()[0];

      (void)sink;
    },
    "BVH::PackedBVH::getBoundingVolume: the BVH is empty; computeBoundingVolume() handles that"));
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

      const Packed bvh  = Packed::buildBottomUp(pool, prims, size_t(2));
      const Packed view = bvh.rebasedView(unrelated);

      (void)view;
    },
    "BVH::PackedBVH::rebasedView: the pool must be the object's own pool or a mirror of it"));
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

  const Packed built = Packed::buildBottomUp(pool, flat, size_t(4));

  // Read the arrays back out through the public accessors a composing class uses, then adopt them.
  const auto nodeSpan = built.getNodes();
  const auto primSpan = built.getPrimitives();

  const std::vector<Node> nodes(nodeSpan.begin(), nodeSpan.end());
  const std::vector<Pnt>  prims(primSpan.begin(), primSpan.end());
  std::vector<AABB>       boxes;

  for (const Node& node : nodes) {
    boxes.push_back(built.getBoundingVolume(node));
  }

  const Packed adopted(pool, nodes, boxes, prims);

  REQUIRE(adopted.getNodes().size() == built.getNodes().size());
  REQUIRE(adopted.getPrimitives().size() == built.getPrimitives().size());

  for (int i = 0; i < 25; i++) {
    const T    t = T(i) * T(0.7);
    const Vec3 query(t * T(0.5), T(1) - t, std::sin(t) * T(4));

    REQUIRE(packedBvhTraversalProbe<T, K>(adopted, query) == packedBvhTraversalProbe<T, K>(built, query));
  }
}

TEMPLATE_TEST_CASE("TreeBVH::deepCopy: independent clone -- distinct nodes, shared primitives, "
                   "identical queries; the copies partition independently",
                   "[BVH][Dodecahedron]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Face = DCEL::FaceT<T>;

  constexpr size_t K = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.stl"), pool);

  BVH::PrimAndBVList<Face, AABB> primsAndBVs;
  for (uint32_t i = 0; i < mesh.numFaces(); i++) {
    const auto& f = mesh.getFace(i);
    primsAndBVs.emplace_back(std::make_shared<const Face>(f), AABB(f.getAllVertexCoordinates(mesh)));
  }

  const FlatMeshSDF<T> flat(mesh, pool);
  const auto           brute = [&flat](const Vec3T<T>& a_point) -> T { return flat.signedDistance(a_point); };

  // Pack a (partitioned) tree and query it the way MeshSDF does, comparing to the brute-force scan.
  const auto packAndCheck = [&](const auto& a_tree) {
    const auto  packed = a_tree->pack(pool);
    const auto& faces  = packed.getPrimitives();

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

      packed.traverse(BVH::PointQuery{p}, state, evalLeaf, pruneDist2);

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
