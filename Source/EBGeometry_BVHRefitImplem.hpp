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
  EBGEOMETRY_REQUIRE(a_bvh.isAttachedTo(a_pool), "RefitSchedule: the pool must be the one the BVH lives in");

  const auto     nodes = a_bvh.getNodes();
  const uint32_t n     = static_cast<uint32_t>(nodes.size());

  // Depth of every node: the array is a depth-first pre-order flattening, so a forward sweep sees
  // every parent before its children.
  std::vector<uint32_t> depth(n, 0);
  uint32_t              maxDepth = 0;

  for (uint32_t i = 0; i < n; i++) {
    if (!nodes[i].isLeaf()) {
      for (const uint32_t child : nodes[i].getChildOffsets()) {
        depth[child] = depth[i] + 1;
        maxDepth     = (depth[child] > maxDepth) ? depth[child] : maxDepth;
      }
    }
  }

  // Counting sort by depth, deepest first.
  const uint32_t numLevels = (n == 0) ? 0 : maxDepth + 1;

  std::vector<uint32_t> count(numLevels, 0);

  for (uint32_t i = 0; i < n; i++) {
    count[maxDepth - depth[i]]++;
  }

  m_levelBegin.assign(numLevels + 1, 0);

  for (uint32_t l = 0; l < numLevels; l++) {
    m_levelBegin[l + 1] = m_levelBegin[l] + count[l];
  }

  std::vector<uint32_t> order(n);
  std::vector<uint32_t> next(m_levelBegin.begin(), m_levelBegin.end() - (numLevels > 0 ? 1 : 0));

  for (uint32_t i = 0; i < n; i++) {
    order[next[maxDepth - depth[i]]++] = i;
  }

  m_nodes.reserveFrom(a_pool, n);
  m_nodes.assign(a_pool.base(), order.data(), n);
}

template <class T, class P, size_t K, class BVConstructor>
EBGEOMETRY_HOST
inline void
refitParallel(PackedBVH<T, P, K>& a_bvh, const RefitSchedule& a_schedule, const BVConstructor& a_bvConstructor)
{
  static_assert(std::is_trivially_copyable_v<BVConstructor>,
                "BVH::refitParallel: the bounding-volume constructor must be trivially copyable");

  // The node count is in the layout's descriptor, so this reads no pool memory, which a device view's
  // host code cannot reach.
  const uint32_t numNodes     = a_bvh.layout().m_linearNodes.size();
  const uint32_t numScheduled = a_schedule.levelBegin(a_schedule.numLevels());

  EBGEOMETRY_REQUIRE(numScheduled == numNodes,
                     "BVH::refitParallel: the schedule covers %u nodes but the BVH has %u; build the schedule "
                     "for this BVH",
                     unsigned(numScheduled),
                     unsigned(numNodes));

  for (size_t l = 0; l < a_schedule.numLevels(); l++) {
    const uint32_t begin = a_schedule.levelBegin(l);
    const uint32_t end   = a_schedule.levelBegin(l + 1);

    GPU::parallelFor(end - begin,
                     RefitLevel<PackedBVH<T, P, K>, BVConstructor>{a_bvh, a_schedule.nodes(), begin, a_bvConstructor});
  }
}

} // namespace BVH

} // namespace EBGeometry

#endif
