// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHRefitImplem.hpp
 * @brief  Implementation of EBGeometry_BVHRefit.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHREFITIMPLEM_HPP
#define EBGEOMETRY_BVHREFITIMPLEM_HPP

// Our includes
#include "EBGeometry_BVHRefit.hpp"

namespace EBGeometry {

namespace BVH {

template <class T, class P, size_t K>
EBGEOMETRY_HOST
RefitSchedule::RefitSchedule(Pool& a_pool, const PackedBVH<T, P, K>& a_bvh)
{
  EBGEOMETRY_REQUIRE(a_bvh.isAttachedTo(a_pool), "BVH::RefitSchedule: the pool must be the one the BVH lives in");
  EBGEOMETRY_REQUIRE(!a_pool.isFrozen(),
                     "BVH::RefitSchedule: the pool is frozen; build the schedule after the BVH and before freezing "
                     "the pool");

  m_rootId      = a_pool.rootId();
  m_nodesOffset = a_bvh.layout().m_linearNodes.m_offset;

  // Depth of every node: the array is a depth-first pre-order flattening, so a forward sweep sees every
  // parent before its children. The node span is read here, before the reserve below can move the block.
  const auto     nodes = a_bvh.getNodes();
  const uint32_t n     = static_cast<uint32_t>(nodes.size());

  std::vector<uint32_t> depth(n, 0);

  uint32_t maxDepth = 0;

  for (uint32_t i = 0; i < n; i++) {
    if (!nodes[i].isLeaf()) {
      for (const uint32_t child : nodes[i].getChildOffsets()) {
        depth[child] = depth[i] + 1;
        maxDepth     = Math::max(maxDepth, depth[child]);
      }
    }
  }

  // Counting sort by depth, deepest first; within a level, nodes stay in array order.
  const uint32_t numLevels = (n == 0) ? 0 : maxDepth + 1;

  std::vector<uint32_t> order(n);

  if (numLevels > 0) {
    m_levelBegin.assign(numLevels + 1, 0);

    for (uint32_t i = 0; i < n; i++) {
      m_levelBegin[maxDepth - depth[i] + 1]++;
    }

    for (uint32_t l = 0; l < numLevels; l++) {
      m_levelBegin[l + 1] += m_levelBegin[l];
    }

    std::vector<uint32_t> next(m_levelBegin.begin(), m_levelBegin.end() - 1);

    for (uint32_t i = 0; i < n; i++) {
      order[next[maxDepth - depth[i]]++] = i;
    }
  }

  m_nodes.reserveSizedFrom(a_pool, n);
  m_nodes.assign(a_pool.base(), order.data(), n);
}

template <class T, class P, size_t K, class BVConstructor>
EBGEOMETRY_HOST
inline void
refit(Pool&                     a_pool,
      const PackedBVH<T, P, K>& a_bvh,
      const RefitSchedule&      a_schedule,
      const BVConstructor&      a_bvConstructor)
{
  // The node count is in the layout's descriptor, so this reads no pool memory.
  const uint32_t numNodes     = a_bvh.layout().m_linearNodes.size();
  const uint32_t numScheduled = a_schedule.m_nodes.size();

  EBGEOMETRY_REQUIRE(a_schedule.m_rootId == a_pool.rootId(),
                     "BVH::refit: the schedule was built in a pool of another mirror chain; build it in the "
                     "BVH's pool");
  EBGEOMETRY_REQUIRE(numScheduled == numNodes,
                     "BVH::refit: the schedule covers %u nodes but the BVH has %u; build the schedule for this BVH",
                     unsigned(numScheduled),
                     unsigned(numNodes));
  EBGEOMETRY_REQUIRE(a_schedule.m_nodesOffset == a_bvh.layout().m_linearNodes.m_offset,
                     "BVH::refit: the schedule was built for another BVH in this pool, or for this one before a "
                     "rebuild; build the schedule for this BVH");

  PackedBVH<T, P, K> view = a_bvh.rebasedView(a_pool);

  if constexpr (!std::is_trivially_copyable_v<BVConstructor>) {
    EBGEOMETRY_REQUIRE(!a_pool.resource().isDeviceAccessible(),
                       "BVH::refit: refitting in a device-accessible pool needs a trivially copyable bounding-volume "
                       "constructor");

    view.refit(a_bvConstructor);
  }
  else {
    if (!a_pool.resource().isDeviceAccessible()) {
      view.refit(a_bvConstructor);

      return;
    }

    // Deepest level first: every child is one level deeper than its parent, so a level's nodes find
    // their children's boxes done, and no two nodes of a level write the same box.
    for (size_t l = 0; l < a_schedule.numLevels(); l++) {
      const uint32_t begin = a_schedule.m_levelBegin[l];
      const uint32_t end   = a_schedule.m_levelBegin[l + 1];

      GPU::parallelFor(
        end - begin,
        RefitDetail::Level<PackedBVH<T, P, K>, BVConstructor>{view, a_schedule.m_nodes, begin, a_bvConstructor});
    }
  }
}

} // namespace BVH

} // namespace EBGeometry

#endif
