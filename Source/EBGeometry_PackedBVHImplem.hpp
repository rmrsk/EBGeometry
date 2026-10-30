// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PackedBVHImplem.hpp
 * @brief  Implementation of EBGeometry_PackedBVH.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_PACKEDBVHIMPLEM_HPP
#define EBGEOMETRY_PACKEDBVHIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PackedBVH.hpp"

namespace EBGeometry {

namespace BVH {

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PackedBVH<T, P, K>::PackedBVH(const Layout& a_layout, const PoolLocation& a_location) noexcept
  : PoolHandle(a_location),
    m_linearNodes(a_layout.m_linearNodes),
    m_primitives(a_layout.m_primitives),
    m_childAabbSoA(a_layout.m_childAabbSoA),
    m_rootBox(a_layout.m_rootBox)
{}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline typename PackedBVH<T, P, K>::Layout
PackedBVH<T, P, K>::layout() const noexcept
{
  return Layout{m_linearNodes, m_primitives, m_childAabbSoA, m_rootBox};
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>
PackedBVH<T, P, K>::rebasedView(const Pool& a_pool) const noexcept
{
  const uint64_t endByte = Math::max(Math::max(m_linearNodes.endByte(), m_primitives.endByte()),
                                     Math::max(m_childAabbSoA.endByte(), m_rootBox.endByte()));

  PackedBVH view = *this;

  view.setLocation(this->rebasedLocation(a_pool, endByte, "BVH::PackedBVH"));

  if (view.location().m_control == nullptr) {
    // A snapshot is what a kernel receives. The device traversal stack is smaller than the host's, so
    // a tree that finalize() accepted can still be too deep to traverse on device. This is the moment
    // the caller commits to that, and the last one that still runs on the host where it can say so.
    this->requireDepthFits(this->base(), DeviceTraversalDepth, "device view");
  }

  return view;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>
PackedBVH<T, P, K>::deepCopy(Pool& a_dstPool) const
{
  const void* srcBase = this->base();

  std::vector<Node> nodes(m_linearNodes.size());
  std::vector<BV>   boxes(m_linearNodes.size());
  std::vector<P>    prims(m_primitives.size());

  for (uint32_t i = 0; i < m_linearNodes.size(); i++) {
    nodes[i] = m_linearNodes.at(srcBase, i);
    boxes[i] = this->getBoundingVolume(nodes[i]);
  }

  for (uint32_t i = 0; i < m_primitives.size(); i++) {
    prims[i] = m_primitives.at(srcBase, i);
  }

  PackedBVH copy(Layout{}, PoolLocation{});

  copy.finalize(a_dstPool, nodes, boxes, prims);

  return copy;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::finalize(Pool&                    a_pool,
                             const std::vector<Node>& a_linearNodes,
                             const std::vector<BV>&   a_nodeBoxes,
                             const std::vector<P>&    a_primitives)
{
  EBGEOMETRY_REQUIRE(a_nodeBoxes.size() == a_linearNodes.size(),
                     "BVH::PackedBVH: %zu node boxes for %zu nodes; every node needs its box",
                     a_nodeBoxes.size(),
                     a_linearNodes.size());

  // Nodes and primitives are numbered with uint32_t, and the builders store their offsets that way.
  // The final counts bound every offset a builder stored, so checking them here, first, catches an
  // offset that wrapped before the checks below misread it.
  // Compared as uint64_t, so a 32-bit size_t does not make the check a tautology the compiler warns on.
  EBGEOMETRY_REQUIRE(uint64_t(a_linearNodes.size()) <= UINT32_MAX && uint64_t(a_primitives.size()) <= UINT32_MAX,
                     "BVH::PackedBVH: %zu nodes and %zu primitives do not fit the 32-bit indices",
                     a_linearNodes.size(),
                     a_primitives.size());

  // Every build path ends here, so this checks the library's own builders as well as adopted arrays:
  // a malformed array -- a leaf with no primitives read as an interior node, say -- would otherwise
  // surface as an out-of-bounds read in Release.
  PackedBVH::requireWellFormed(a_linearNodes, a_primitives.size());

  // Every array of one BVH must come from the same pool -- offsets from two different blocks cannot
  // both resolve against one base.
  this->attach(a_pool, "BVH::PackedBVH");

  m_linearNodes.reserveFrom(a_pool, static_cast<uint32_t>(a_linearNodes.size()));
  m_primitives.reserveFrom(a_pool, static_cast<uint32_t>(a_primitives.size()));

  // Resolve the base only after every reservation above: a reserve can grow the pool, which moves
  // the block and invalidates any address taken before it.
  void* poolBase = this->base();

  m_linearNodes.assign(poolBase, a_linearNodes.data(), static_cast<uint32_t>(a_linearNodes.size()));
  m_primitives.assign(poolBase, a_primitives.data(), static_cast<uint32_t>(a_primitives.size()));

  this->buildSoA(a_pool, a_nodeBoxes);

  // buildSoA() reserves, so re-resolve; after it nothing else moves the block. Reject here, once,
  // rather than letting traverse() walk off its fixed stack later with no way to notice.
  this->requireDepthFits(this->base(), HostTraversalDepth, "host build");
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::buildSoA(Pool& a_pool, const std::vector<BV>& a_nodeBoxes)
{
  // Only interior nodes have children, so only they get a row; a leaf, typically most of the nodes,
  // gets none.
  uint32_t numInterior = 0;

  for (uint32_t i = 0; i < m_linearNodes.size(); i++) {
    if (!m_linearNodes.at(this->base(), i).isLeaf()) {
      numInterior++;
    }
  }

  m_childAabbSoA.reserveFrom(a_pool, numInterior);
  m_rootBox.reserveFrom(a_pool, m_linearNodes.size() > 0 ? 1U : 0U);

  // Resolved after the reservations above, for the reason given in finalize().
  void* poolBase = this->base();

  // Assembled host-side and copied in one shot, like the node and primitive arrays: a PODVector
  // never reallocates, so it has no way to be filled element-by-element without its final size
  // already fixed, and this keeps the one finalize pattern everywhere.
  std::vector<ChildAABBSoA> soaCache(numInterior);

  if (m_linearNodes.size() > 0) {
    m_rootBox.push_back(poolBase, a_nodeBoxes[0]);

    m_linearNodes.at(poolBase, 0).m_boxSlot = RootBoxSlot;
  }

  uint32_t row = 0;

  for (uint32_t i = 0; i < m_linearNodes.size(); i++) {
    Node& node = m_linearNodes.at(poolBase, i);

    if (!node.isLeaf()) {
      const auto& offsets = node.getChildOffsets();
      auto&       soa     = soaCache[row];

      node.m_childBoxRow = row;

      for (size_t k = 0; k < K; k++) {
        PackedBVH::setRowBox(soa, k, a_nodeBoxes[offsets[k]]);

        // Each node is a child of exactly one interior node (requireWellFormed() checked the shape),
        // so this assigns every non-root node's slot once.
        m_linearNodes.at(poolBase, offsets[k]).m_boxSlot = static_cast<uint32_t>(row * K + k);
      }

      row++;
    }
  }

  m_childAabbSoA.assign(poolBase, soaCache.data(), numInterior);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline void
PackedBVH<T, P, K>::setRowBox(ChildAABBSoA& a_soa, size_t a_slot, const BV& a_bv) noexcept
{
  const auto& lo = a_bv.getLowCorner();
  const auto& hi = a_bv.getHighCorner();

  for (size_t axis = 0; axis < 3; axis++) {
    a_soa.m_lo[axis][a_slot] = lo[axis];
    a_soa.m_hi[axis][a_slot] = hi[axis];
  }
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline typename PackedBVH<T, P, K>::BV
PackedBVH<T, P, K>::rowBox(const ChildAABBSoA& a_soa, size_t a_slot) noexcept
{
  return BV(Vec3T<T>(a_soa.m_lo[0][a_slot], a_soa.m_lo[1][a_slot], a_soa.m_lo[2][a_slot]),
            Vec3T<T>(a_soa.m_hi[0][a_slot], a_soa.m_hi[1][a_slot], a_soa.m_hi[2][a_slot]));
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<const P>
PackedBVH<T, P, K>::getPrimitives() const noexcept
{
  return m_primitives.bind(this->base());
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<P>
PackedBVH<T, P, K>::getPrimitives() noexcept
{
  return m_primitives.bind(this->base());
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<const typename PackedBVH<T, P, K>::Node>
PackedBVH<T, P, K>::getNodes() const noexcept
{
  return m_linearNodes.bind(this->base());
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline const EBGeometry::BoundingVolumes::AABBT<T>&
PackedBVH<T, P, K>::getBoundingVolume() const noexcept
{
  // An empty BVH has no node 0 to read; computeBoundingVolume() returns the empty box instead.
  EBGEOMETRY_REQUIRE(m_linearNodes.size() > 0,
                     "BVH::PackedBVH::getBoundingVolume: the BVH is empty; computeBoundingVolume() handles that");

  return m_rootBox.at(this->base(), 0);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline EBGeometry::BoundingVolumes::AABBT<T>
PackedBVH<T, P, K>::computeBoundingVolume() const noexcept
{
  if (m_rootBox.size() == 0) {
    return EBGeometry::BoundingVolumes::AABBT<T>();
  }

  return m_rootBox.at(this->base(), 0);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline typename PackedBVH<T, P, K>::BV
PackedBVH<T, P, K>::getBoundingVolume(const Node& a_node) const noexcept
{
  const uint32_t slot = a_node.getBoxSlot();

  if (slot == RootBoxSlot) {
    return m_rootBox.at(this->base(), 0);
  }

  EBGEOMETRY_EXPECT(slot / K < m_childAabbSoA.size());

  return PackedBVH::rowBox(m_childAabbSoA.at(this->base(), slot / K), slot % K);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
PackedBVH<T, P, K>::getDistanceToBoundingVolume2(const Node& a_node, const Vec3T<T>& a_point) const noexcept
{
  const uint32_t slot = a_node.getBoxSlot();

  if (slot == RootBoxSlot) {
    return m_rootBox.at(this->base(), 0).getDistance2(a_point);
  }

  EBGEOMETRY_EXPECT(slot / K < m_childAabbSoA.size());

  return QueryDetail::pointDistance2(m_childAabbSoA.at(this->base(), slot / K), a_point, slot % K);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
PackedBVH<T, P, K>::getDistanceToChildBoundingVolume2(const Node&     a_parent,
                                                      size_t          a_k,
                                                      const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(!a_parent.isLeaf());
  EBGEOMETRY_EXPECT(a_k < K);
  EBGEOMETRY_EXPECT(a_parent.getChildBoxRow() < m_childAabbSoA.size());

  return QueryDetail::pointDistance2(m_childAabbSoA.at(this->base(), a_parent.getChildBoxRow()), a_point, a_k);
}

namespace PackedBVHDetail {

/**
 * @brief Whether the child keyed @p a_a (node @p a_nodeA) is visited before the child keyed @p a_b
 * (node @p a_nodeB), under a visit order.
 * @details Internal helper; not part of the public API. A comparator may take the keys alone, or the keys
 * with the children's node indices.
 * @tparam Key   Key type.
 * @tparam Order NearestFirst, FarthestFirst, AsStored, or a comparator.
 * @param[in] a_order The order.
 * @param[in] a_a     One child's key.
 * @param[in] a_nodeA That child's node index.
 * @param[in] a_b     Another child's key.
 * @param[in] a_nodeB That child's node index.
 * @return True if the first child goes first.
 */
template <class Key, class Order>
EBGEOMETRY_HOST_DEVICE
inline bool
visitsBefore(
  const Order& a_order, const Key& a_a, const uint32_t a_nodeA, const Key& a_b, const uint32_t a_nodeB) noexcept
{
  if constexpr (std::is_same_v<Order, NearestFirst>) {
    return a_a < a_b;
  }
  else if constexpr (std::is_same_v<Order, FarthestFirst>) {
    return a_b < a_a;
  }
  else if constexpr (std::is_same_v<Order, AsStored>) {
    // Children arrive in slot order, so an earlier slot is always visited first.
    return true;
  }
  else if constexpr (std::is_invocable_r_v<bool, const Order&, const Key&, uint32_t, const Key&, uint32_t>) {
    return a_order(a_a, a_nodeA, a_b, a_nodeB);
  }
  else {
    static_assert(std::is_invocable_r_v<bool, const Order&, const Key&, const Key&>,
                  "PackedBVH::traverse: an order is NearestFirst, FarthestFirst, AsStored, or a comparator "
                  "(keyA, keyB) -> bool or (keyA, nodeA, keyB, nodeB) -> bool");

    return a_order(a_a, a_b);
  }
}

/**
 * @brief The bound a walk compares keys with: the caller's bound, with +infinity replaced by the largest
 * finite value for a floating-point key.
 * @details Internal helper; not part of the public API. A key of +infinity means the query cannot reach
 * the node (a ray that misses a box), so it must never be entered, even under a bound of +infinity ("no
 * bound yet"). Comparing keys with this bound keeps one comparison per child (key <= bound) and still
 * excludes them; any finite key compares exactly as with the caller's bound. A key that is not a
 * floating-point type is compared with the caller's bound unchanged.
 * @tparam Key Key type.
 * @param[in] a_bound The caller's bound.
 * @return The bound to compare keys with.
 */
template <class Key>
EBGEOMETRY_HOST_DEVICE
inline Key
effectiveBound(const Key& a_bound) noexcept
{
  if constexpr (std::is_floating_point_v<Key>) {
    // Written so that a NaN bound stays NaN (every comparison with it false: nothing is entered).
    return (a_bound > Math::Limits<Key>::max()) ? Math::Limits<Key>::max() : a_bound;
  }
  else {
    return a_bound;
  }
}

} // namespace PackedBVHDetail

template <class T, class P, size_t K>
template <class Query, class State, class LeafEvaluator, class BoundFn, class VisitFn, class Order>
EBGEOMETRY_HOST_DEVICE
inline void
PackedBVH<T, P, K>::traverse(const Query&    a_query,
                             State&          a_state,
                             LeafEvaluator&& a_leaf,
                             BoundFn&&       a_bound,
                             VisitFn&&       a_visit,
                             Order           a_order) const noexcept
{
  static_assert(IsQuery<Query>::value, "PackedBVH::traverse: the first argument must be a query type (see PointQuery)");
  static_assert(QueryLanes<Query>::value == 1, "PackedBVH::traverse: one query per walk");
  static_assert(std::is_same_v<typename Query::Scalar, T>, "PackedBVH::traverse: the query's precision must be T");

  using Key = typename QueryKey<Query>::type;

  // A visit callback is called for every node entered; the default, which always descends, compiles away.
  constexpr bool visits = !std::is_same_v<std::decay_t<VisitFn>, DescendAll>;

  // An empty BVH has no root to descend from (PointCloudBVH represents an empty cloud this way).
  if (m_linearNodes.size() == 0) {
    return;
  }

  // One resolution for the whole walk: a query reserves nothing, so the pool base cannot move.
  const void* poolBase = this->base();

  // The root has no parent row; the query keys its box directly. A ray that misses the whole tree (its
  // key +infinity: unreachable), or a point farther than the bound, ends here.
  Key nodeKey = a_query.key(m_rootBox.at(poolBase, 0));

  if (nodeKey > PackedBVHDetail::effectiveBound(a_bound(a_state))) {
    return;
  }

  // One entry per interior node on the path from the root, so a tree of depth D needs at most D - 1
  // entries: the bound traversalStackDepth() gives, which every build checked the tree against.
  constexpr size_t stackDepth = PackedBVH::traversalStackDepth();

  StackEntry stack[stackDepth];
  size_t     top = 0;

  uint32_t nodeIdx = 0U;

  while (true) {
    // Enter nodeIdx (keyed nodeKey), and keep descending into the first child the order gives.
    while (true) {
      const Node& node = m_linearNodes.at(poolBase, nodeIdx);

      if constexpr (visits) {
        if (!a_visit(a_state, nodeIdx, node, static_cast<const Key&>(nodeKey))) {
          break;
        }
      }

      if (node.isLeaf()) {
        a_leaf(a_state, node.getPrimitivesOffset(), node.getNumPrimitives());

        break;
      }

      // The bound once per node expanded: any leaf visited before may have tightened it.
      const Key bound = PackedBVHDetail::effectiveBound(a_bound(a_state));

      // All K keys in one pass (the query's SIMD kernel).
      const ChildAABBSoA& row  = m_childAabbSoA.at(poolBase, node.getChildBoxRow());
      const Array<Key, K> keys = a_query.keys(row);

      const auto& offsets = node.getChildOffsets();

      // The children within the bound, sorted so that the one to visit first is last: it is visited now,
      // and the rest wait on the stack in the same order. Insertion sort, not std::sort: K is a handful,
      // and it is callable on a device. It is deterministic: of two children with equal keys, the later
      // slot goes first under NearestFirst and FarthestFirst.
      uint8_t slots[K];
      size_t  numSlots = 0;

      for (size_t k = 0; k < K; k++) {
        EBGEOMETRY_EXPECT(offsets[k] > nodeIdx);

        if (keys[k] <= bound) {
          size_t j = numSlots++;

          while (j > 0 && PackedBVHDetail::visitsBefore(
                            a_order, keys[slots[j - 1]], offsets[slots[j - 1]], keys[k], offsets[k])) {
            slots[j] = slots[j - 1];
            j--;
          }

          slots[j] = static_cast<uint8_t>(k);
        }
      }

      if (numSlots == 0) {
        break;
      }

      if (numSlots > 1) {
        EBGEOMETRY_EXPECT(top < stackDepth);

        StackEntry& entry = stack[top++];

        entry.m_node  = nodeIdx;
        entry.m_count = static_cast<uint8_t>(numSlots - 1);

        for (size_t i = 0; i + 1 < numSlots; i++) {
          entry.m_slots[i] = slots[i];
        }
      }

      nodeIdx = offsets[slots[numSlots - 1]];
      nodeKey = keys[slots[numSlots - 1]];
    }

    // Return to the deepest level with a child still within the bound. The bound may have tightened
    // since the level was sorted, so the next child's key is recomputed (bit for bit what keys() gave)
    // and compared again.
    bool resumed = false;

    while (top > 0 && !resumed) {
      StackEntry&   entry  = stack[top - 1];
      const Node&   parent = m_linearNodes.at(poolBase, entry.m_node);
      const uint8_t slot   = entry.m_slots[entry.m_count - 1];

      const Key key = a_query.key(m_childAabbSoA.at(poolBase, parent.getChildBoxRow()), size_t(slot));

      if (key > PackedBVHDetail::effectiveBound(a_bound(a_state))) {
        // Under NearestFirst the rest of the level is farther still: drop it. Under any other order a
        // later child may still be within the bound, so only this child is skipped.
        if constexpr (std::is_same_v<Order, NearestFirst>) {
          top--;
        }
        else if (--entry.m_count == 0) {
          top--;
        }

        continue;
      }

      if (--entry.m_count == 0) {
        top--;
      }

      nodeIdx = parent.getChildOffsets()[slot];
      nodeKey = key;
      resumed = true;
    }

    if (!resumed) {
      return;
    }
  }
}

template <class T, class P, size_t K>
inline size_t
PackedBVH<T, P, K>::maxNodeDepth(const void* a_base) const
{
  if (m_linearNodes.size() == 0) {
    return 0;
  }

  size_t maxDepth = 0;

  // (node index, depth of that node). A host-side build/mirror step, so std::vector is fine here,
  // unlike in traverse(), which must also run on a device.
  std::vector<std::pair<uint32_t, size_t>> stack;

  stack.reserve(64);
  stack.emplace_back(uint32_t(0), size_t(1));

  while (!stack.empty()) {
    const uint32_t idx   = stack.back().first;
    const size_t   depth = stack.back().second;

    stack.pop_back();

    if (depth > maxDepth) {
      maxDepth = depth;
    }

    const Node& node = m_linearNodes.at(a_base, idx);

    if (!node.isLeaf()) {
      const auto& offsets = node.getChildOffsets();

      for (size_t k = 0; k < K; k++) {
        stack.emplace_back(offsets[k], depth + 1);
      }
    }
  }

  return maxDepth;
}

template <class T, class P, size_t K>
inline void
PackedBVH<T, P, K>::requireDepthFits(const void* a_base, const size_t a_maxDepth, const char* a_context) const
{
  const size_t depth = this->maxNodeDepth(a_base);

  EBGEOMETRY_REQUIRE(depth <= a_maxDepth,
                     "BVH::PackedBVH: %s -- the tree is %zu levels deep, more than the %zu levels a traversal stack "
                     "holds.\n"
                     "  traverse() would overflow its fixed stack, which Release builds do not detect. Rebuild "
                     "with a larger leaf size, or a partitioner that splits more evenly, before querying this BVH.",
                     a_context,
                     depth,
                     a_maxDepth);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::requireWellFormed(const std::vector<Node>& a_linearNodes, const size_t a_numPrimitives)
{
  const size_t numNodes = a_linearNodes.size();

  // Report the first defect and stop: once one offset is wrong, later ones say nothing reliable. The
  // array must be a depth-first pre-order flattening: every child strictly after its parent and
  // inside the array, every leaf's primitives inside the primitive array.
  if (numNodes == 0) {
    EBGEOMETRY_REQUIRE(a_numPrimitives == 0,
                       "BVH::PackedBVH: adopted node array is malformed -- it is empty, but the primitive "
                       "array holds %zu primitives",
                       a_numPrimitives);

    return;
  }

  // Every node but the root has exactly one parent, so a traversal reaches each primitive once.
  std::vector<bool> hasParent(numNodes, false);

  for (size_t i = 0; i < numNodes; i++) {
    const Node& node = a_linearNodes[i];

    if (node.isLeaf()) {
      const size_t end = size_t(node.getPrimitivesOffset()) + size_t(node.getNumPrimitives());

      EBGEOMETRY_REQUIRE(end <= a_numPrimitives,
                         "BVH::PackedBVH: node array is malformed -- leaf %zu's primitive range ends at "
                         "%zu, past the primitive array's %zu primitives",
                         i,
                         end,
                         a_numPrimitives);

      continue;
    }

    // An interior node. A node meant as a leaf but holding no primitives lands here too, with its
    // child offsets unset (zero), and the first check below rejects it.
    for (const uint32_t child : node.getChildOffsets()) {
      EBGEOMETRY_REQUIRE(child > i && child < numNodes,
                         "BVH::PackedBVH: node array is malformed -- node %zu has child offset %zu, which "
                         "is not strictly after its parent and inside the array of %zu nodes (a leaf with no "
                         "primitives reads as an interior node like this)",
                         i,
                         size_t(child),
                         numNodes);

      EBGEOMETRY_REQUIRE(!hasParent[child],
                         "BVH::PackedBVH: node array is malformed -- node %zu names child %zu, which already has "
                         "another parent",
                         i,
                         size_t(child));

      hasParent[child] = true;
    }
  }

  for (size_t i = 1; i < numNodes; i++) {
    EBGEOMETRY_REQUIRE(hasParent[i], "BVH::PackedBVH: node array is malformed -- node %zu has no parent", i);
  }
}

template <class T, class P, size_t K>
template <class BVConstructor>
inline void
PackedBVH<T, P, K>::refit(const BVConstructor& a_bvConstructor)
{
  // m_linearNodes is a depth-first pre-order flattening, so every child has a higher index than its
  // parent. Sweeping the array in reverse therefore refits all of a node's children before the node
  // itself -- no recursion or explicit stack needed. A node's box is written where it is stored, in its
  // parent's row (the root's in m_rootBox), so an interior node reads its children's new boxes from its
  // own row. Boxes are merged pairwise, so a refit (meant as cheap per-frame maintenance) allocates
  // nothing.
  //
  // refit() reserves nothing, so one resolution covers the whole sweep.
  void* poolBase = this->base();

  for (uint32_t i = m_linearNodes.size(); i-- > 0;) {
    const Node& node = m_linearNodes.at(poolBase, i);

    BV bv;

    if (node.isLeaf()) {
      const uint32_t offset = node.getPrimitivesOffset();
      const uint32_t count  = node.getNumPrimitives();

      for (uint32_t p = 0; p < count; p++) {
        bv = bv.merged(a_bvConstructor(m_primitives.at(poolBase, offset + p)));
      }
    }
    else {
      const ChildAABBSoA& row = m_childAabbSoA.at(poolBase, node.getChildBoxRow());

      for (size_t k = 0; k < K; k++) {
        bv = bv.merged(PackedBVH::rowBox(row, k));
      }
    }

    const uint32_t slot = node.getBoxSlot();

    if (slot == RootBoxSlot) {
      m_rootBox.at(poolBase, 0) = bv;
    }
    else {
      PackedBVH::setRowBox(m_childAabbSoA.at(poolBase, slot / K), slot % K, bv);
    }
  }
}

} // namespace BVH

} // namespace EBGeometry

#endif
