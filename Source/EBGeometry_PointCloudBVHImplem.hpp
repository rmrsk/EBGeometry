// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudBVHImplem.hpp
 * @brief   Implementation of EBGeometry_PointCloudBVH.hpp
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_POINTCLOUDBVHIMPLEM_HPP
#define EBGEOMETRY_POINTCLOUDBVHIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PointCloudBVH.hpp"
#include "EBGeometry_PointCloudDetail.hpp"

namespace EBGeometry {

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline PointCloudBVH<T, K, W>::PointCloudBVH(Pool&                        a_pool,
                                             const std::vector<Vec3T<T>>& a_positions,
                                             std::size_t                  a_targetLeafSize)
{
  *this = build(a_pool, a_positions, a_targetLeafSize);
}

template <class T, size_t K, size_t W>
template <class U>
EBGEOMETRY_HOST
inline std::vector<U>
PointCloudBVH<T, K, W>::toHost(const PODVector<U>& a_array, const void* a_base)
{
  const U* first = a_array.data(a_base);

  return std::vector<U>(first, first + a_array.size());
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline PointCloudBVH<T, K, W>
PointCloudBVH<T, K, W>::rebasedView(const Pool& a_pool) const noexcept
{
  const auto end = [](const auto&... a_arrays) noexcept {
    uint64_t byte = 0;

    ((byte = Math::max(byte, a_arrays.endByte())), ...);

    return byte;
  };

  const uint64_t endByte = Math::max(end(m_bvhLayout.m_linearNodes,
                                         m_bvhLayout.m_primitives,
                                         m_bvhLayout.m_childAabbSoA,
                                         m_bvhLayout.m_rootBox,
                                         m_positions,
                                         m_leafOff,
                                         m_leafCnt,
                                         m_order),
                                     end(m_scratch.m_keys,
                                         m_scratch.m_box,
                                         m_scratch.m_nodes,
                                         m_scratch.m_boxes,
                                         m_scratch.m_rows,
                                         m_scratch.m_groups,
                                         m_scratch.m_scan));

  // Every array is a pool-relative offset, identical in any mirror of the pool, so only the location
  // changes.
  PointCloudBVH view = *this;

  view.setLocation(this->rebasedLocation(a_pool, endByte, "PointCloudBVH"));

  // A snapshot is what a kernel receives, and the device traversal stack is smaller than the host's. The
  // depth is the build's, so this reads nothing from a pool that may be in device memory.
  if (view.location().m_control == nullptr) {
    EBGEOMETRY_REQUIRE(m_depth <= BVH::DeviceTraversalDepth,
                       "PointCloudBVH::rebasedView: the tree is %u levels deep, more than a device traversal "
                       "allows (%zu)",
                       unsigned(m_depth),
                       BVH::DeviceTraversalDepth);
  }

  return view;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline PointCloudBVH<T, K, W>
PointCloudBVH<T, K, W>::deepCopy(Pool& a_dstPool) const
{
  // Read everything out before reserving anything: a_dstPool may be this object's own pool, and a
  // reserve there can move the block under any address taken from it.
  const void* srcBase = this->base();

  const auto nodes     = toHost(m_bvhLayout.m_linearNodes, srcBase);
  const auto groups    = toHost(m_bvhLayout.m_primitives, srcBase);
  const auto rows      = toHost(m_bvhLayout.m_childAabbSoA, srcBase);
  const auto rootBox   = toHost(m_bvhLayout.m_rootBox, srcBase);
  const auto positions = toHost(m_positions, srcBase);
  const auto leafOff   = toHost(m_leafOff, srcBase);
  const auto leafCnt   = toHost(m_leafCnt, srcBase);
  const auto order     = toHost(m_order, srcBase);

  const auto numPoints = static_cast<std::uint32_t>(positions.size());

  PointCloudBVH copy = *this;

  copy.setLocation(PoolLocation{});

  copy.m_scratch   = Scratch{};
  copy.m_maxPoints = 0;
  copy.m_parallel  = false;

  copy.reserveArrays(a_dstPool,
                     Sizes{static_cast<std::uint32_t>(nodes.size()),
                           static_cast<std::uint32_t>(rows.size()),
                           static_cast<std::uint32_t>(groups.size())},
                     numPoints);

  // Resolve the base only after every reservation above.
  void* base = copy.base();

  copy.m_bvhLayout.m_linearNodes.assign(base, nodes.data(), static_cast<std::uint32_t>(nodes.size()));
  copy.m_bvhLayout.m_primitives.assign(base, groups.data(), static_cast<std::uint32_t>(groups.size()));
  copy.m_bvhLayout.m_childAabbSoA.assign(base, rows.data(), static_cast<std::uint32_t>(rows.size()));
  copy.m_bvhLayout.m_rootBox.assign(base, rootBox.data(), static_cast<std::uint32_t>(rootBox.size()));
  copy.m_positions.assign(base, positions.data(), numPoints);
  copy.m_leafOff.assign(base, leafOff.data(), numPoints);
  copy.m_leafCnt.assign(base, leafCnt.data(), numPoints);
  copy.m_order.assign(base, order.data(), numPoints);

  return copy;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline std::size_t
PointCloudBVH<T, K, W>::query(const Vec3T<T>& a_query,
                              std::size_t     a_k,
                              Hit*            a_out,
                              uint32_t        a_exclude,
                              std::uint32_t   a_seedOff,
                              std::uint32_t   a_seedCnt) const noexcept
{
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_query[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[2]));
  EBGEOMETRY_EXPECT(a_exclude == PointCloud::InvalidIndex || a_exclude < m_positions.size());

  // Resolved once per query: a query reserves nothing, so the pool block cannot move under them.
  const Packed                    bvh    = this->getBVH();
  const PODSpan<const Node>       nodes  = bvh.getNodes();
  const PODSpan<const PointGroup> groups = bvh.getPrimitives();

  EBGEOMETRY_EXPECT(a_seedCnt == 0 || (a_seedOff + a_seedCnt) <= groups.size());

  // The running a_k-best set (ascending by distance). It is the traverse() state, so the held
  // BVH's traversal prunes off its current bound.
  using Best = PointCloud::KBest<T>;

  Best best(a_out, a_k, a_exclude);

  // An empty cloud has no nodes; every traversal below would index node 0. Bail out.
  if (nodes.size() == 0) {
    return 0;
  }

  // Each point is in exactly one group, and the scan skips the lanes past the group's real points (the
  // padded lanes repeat the last one), so every point is offered once and the set needs no de-duplication.
  //
  // The lane loop has the constant bound W, with the padding test inside it, so that the compiler unrolls
  // it and keeps the group's distances in registers. With the bound numValid, nvcc kept them in local
  // memory (a store and a reload per group visited): the device queries ran 9-26% slower in float
  // (closest point, the nearest other point, 8 nearest; 262,144 queries, RTX 3080) and 0-2% in double.
  const auto scanLeaf = [&groups, &a_query](Best& a_best, std::size_t a_off, std::size_t a_cnt) noexcept {
    for (std::size_t g = 0; g < a_cnt; g++) {
      const PointGroup&   group     = groups[static_cast<std::uint32_t>(a_off + g)];
      const Array<T, W>   distances = group.getDistances2(a_query);
      const std::uint32_t numValid  = group.numValid();

      for (std::uint32_t lane = 0; lane < W; lane++) {
        if (lane < numValid) {
          a_best.insert(distances[lane], group.getPointId(lane));
        }
      }
    }
  };

  const auto pruneDist2 = [](const Best& a_best) noexcept -> T { return a_best.bound(); };

  if (a_seedCnt > 0) {
    // Seed the bound from the query point's own leaf (self queries only), then skip that leaf below
    // so the descent prunes with an already-tight bound instead of starting from +inf.
    scanLeaf(best, a_seedOff, a_seedCnt);
  }

  if (a_seedCnt > 0 && a_k == 1) {
    // Seeded single-nearest self-query: scanning the query point's own leaf first gives a tight
    // prune bound immediately, so a plain unordered scalar DFS prunes just as hard as an ordered
    // descent -- without paying traverse()'s per-interior-node child-sort and its separate
    // m_childAabbSoA SIMD load. For these cheap SoA point leaves that makes the unordered DFS ~20%
    // faster. (This holds ONLY because of the seed: an unseeded search starts from an infinite bound,
    // where the near-first order is decisive.)
    //
    // One stack entry per level, as in PackedBVH's own traversals: the interior node and a mask of its
    // children whose boxes were closer than the best when it was expanded. Children are visited from
    // the highest slot down, each tested again when the traversal reaches it, so a bound tightened by
    // a leaf visited in between applies. The tree was validated against the stack depth PackedBVH
    // gives for this compilation pass (host, or the smaller device budget at rebasedView()); the guard
    // catches a pathological overrun in debug builds.
    using Mask = std::conditional_t<(K <= 32), std::uint32_t, std::uint64_t>;

    static_assert(K <= 64, "PointCloudBVH: a level's child mask holds at most 64 children");

    struct Level
    {
      std::uint32_t m_node;
      Mask          m_children;
    };

    constexpr std::size_t maxStack = Packed::traversalStackDepth();

    Level       stack[maxStack];
    std::size_t stackTop = 0;

    // The highest slot set in a non-empty mask.
    const auto highest = [](const Mask a_mask) noexcept -> std::size_t {
      std::size_t k = K - 1;

      while (((a_mask >> k) & Mask(1)) == 0) {
        k--;
      }

      return k;
    };

    if (!(bvh.getDistanceToBoundingVolume2(nodes[0], a_query) < best.bound())) {
      return best.found();
    }

    std::uint32_t nodeIdx = 0U;

    while (true) {
      // Visit nodeIdx, and keep descending into its highest child within the bound.
      while (true) {
        const Node& node = nodes[nodeIdx];

        if (node.isLeaf()) {
          const std::uint32_t primOffset = node.getPrimitivesOffset();

          if (primOffset != a_seedOff) {
            scanLeaf(best, primOffset, node.getNumPrimitives());
          }

          break;
        }

        const auto& childOffsets = node.getChildOffsets();

        Mask children = 0;

        // The children's boxes are read from this node's row, without loading the children.
        for (std::size_t k = 0; k < K; k++) {
          if (bvh.getDistanceToChildBoundingVolume2(node, k, a_query) < best.bound()) {
            children |= Mask(1) << k;
          }
        }

        if (children == 0) {
          break;
        }

        const std::size_t k = highest(children);

        children &= ~(Mask(1) << k);

        if (children != 0) {
          EBGEOMETRY_EXPECT(stackTop < maxStack);

          stack[stackTop++] = Level{nodeIdx, children};
        }

        nodeIdx = childOffsets[k];
      }

      // Return to the deepest level with a child still within the bound.
      bool resumed = false;

      while (stackTop > 0 && !resumed) {
        Level&            level = stack[stackTop - 1];
        const std::size_t k     = highest(level.m_children);

        level.m_children &= ~(Mask(1) << k);

        if (level.m_children == 0) {
          stackTop--;
        }

        const Node& parent = nodes[level.m_node];

        if (bvh.getDistanceToChildBoundingVolume2(parent, k, a_query) < best.bound()) {
          nodeIdx = parent.getChildOffsets()[k];
          resumed = true;
        }
      }

      if (!resumed) {
        break;
      }
    }

    return best.found();
  }

  // Every other query uses the held BVH's near-first ordered descent, skipping the seeded leaf.
  const auto evalLeaf = [&](Best& a_best, std::size_t a_off, std::size_t a_cnt) noexcept {
    if (a_seedCnt > 0 && static_cast<std::uint32_t>(a_off) == a_seedOff) {
      return; // own leaf already scanned in the seed
    }

    scanLeaf(a_best, a_off, a_cnt);
  };

  bvh.traverse(BVH::PointQuery<T>{a_query}, best, evalLeaf, pruneDist2);

  return best.found();
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename PointCloudBVH<T, K, W>::Hit
PointCloudBVH<T, K, W>::closestPoint(const Vec3T<T>& a_query) const noexcept
{
  Hit hit;

  this->query(a_query, 1, &hit, PointCloud::InvalidIndex, 0, 0);

  return hit;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline std::size_t
PointCloudBVH<T, K, W>::closestPoints(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const noexcept
{
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);

  return this->query(a_query, a_k, a_out, PointCloud::InvalidIndex, 0, 0);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename PointCloudBVH<T, K, W>::Hit
PointCloudBVH<T, K, W>::nearestNeighbor(uint32_t a_point) const noexcept
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());

  const void* poolBase = this->base();

  Hit hit;

  this->query(m_positions.at(poolBase, a_point),
              1,
              &hit,
              a_point,
              m_leafOff.at(poolBase, a_point),
              m_leafCnt.at(poolBase, a_point));

  return hit;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline std::size_t
PointCloudBVH<T, K, W>::nearestNeighbors(uint32_t a_point, std::size_t a_k, Hit* a_out) const noexcept
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);

  const void* poolBase = this->base();

  return this->query(m_positions.at(poolBase, a_point),
                     a_k,
                     a_out,
                     a_point,
                     m_leafOff.at(poolBase, a_point),
                     m_leafCnt.at(poolBase, a_point));
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline std::vector<typename PointCloudBVH<T, K, W>::Hit>
PointCloudBVH<T, K, W>::allNearestNeighbors(std::size_t a_k) const
{
  EBGEOMETRY_EXPECT(a_k >= 1);

  const std::size_t numPoints = m_positions.size();
  const void*       poolBase  = this->base();

  std::vector<Hit> result(numPoints * a_k);

  // Process points in leaf (build) order: the top-down build already grouped them spatially, so
  // consecutive queries touch nearby leaves and each seeded own-leaf stays hot in cache -- the same
  // benefit a Hilbert sort would give, but reusing m_order costs nothing per call (no re-sort).
  // Ordering affects only speed, not results.
  for (const std::uint32_t p : m_order.bind(poolBase)) {
    EBGEOMETRY_EXPECT(p < numPoints);

    this->query(m_positions.at(poolBase, p),
                a_k,
                &result[std::size_t(p) * a_k],
                p,
                m_leafOff.at(poolBase, p),
                m_leafCnt.at(poolBase, p));
  }

  return result;
}

} // namespace EBGeometry

#endif
