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
#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>

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
  : PointCloudBVH(a_pool, buildTree(a_positions, a_targetLeafSize), a_positions)
{
  static_assert(std::is_floating_point_v<T>, "PointCloudBVH requires a floating-point type T");
  static_assert(K >= 2, "PointCloudBVH requires a branching factor K >= 2");
  static_assert(W >= 1, "PointCloudBVH requires a SIMD width W >= 1");

  // a_targetLeafSize is checked by buildTree(), before the build uses it.
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline PointCloudBVH<T, K, W>::PointCloudBVH(Pool&                        a_pool,
                                             const BuildResult&           a_build,
                                             const std::vector<Vec3T<T>>& a_positions)
  : m_bvh(a_pool, a_build.nodes, a_build.primitives)
{
  this->storeCloud(a_pool, a_positions, a_build.leafOff, a_build.leafCnt, a_build.order);
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline void
PointCloudBVH<T, K, W>::storeCloud(Pool&                             a_pool,
                                   const std::vector<Vec3T<T>>&      a_positions,
                                   const std::vector<std::uint32_t>& a_leafOff,
                                   const std::vector<std::uint32_t>& a_leafCnt,
                                   const std::vector<std::uint32_t>& a_order)
{
  // Every array must come from the pool the BVH was built in: they all resolve against its base.
  EBGEOMETRY_EXPECT(m_bvh.isAttachedTo(a_pool));

  PointCloudDetail::requireValidCloud("PointCloudBVH", a_positions);

  EBGEOMETRY_EXPECT(a_leafOff.size() == a_positions.size());
  EBGEOMETRY_EXPECT(a_leafCnt.size() == a_positions.size());
  EBGEOMETRY_EXPECT(a_order.size() == a_positions.size());

  const auto numPoints = static_cast<std::uint32_t>(a_positions.size());

  m_positions.reserveFrom(a_pool, numPoints);
  m_leafOff.reserveFrom(a_pool, numPoints);
  m_leafCnt.reserveFrom(a_pool, numPoints);
  m_order.reserveFrom(a_pool, numPoints);

  // Resolve the base only after every reservation above: a reserve can grow the pool, which moves
  // the block and invalidates any address taken before it.
  void* poolBase = const_cast<void*>(this->base());

  m_positions.assign(poolBase, a_positions.data(), numPoints);
  m_leafOff.assign(poolBase, a_leafOff.data(), numPoints);
  m_leafCnt.assign(poolBase, a_leafCnt.data(), numPoints);
  m_order.assign(poolBase, a_order.data(), numPoints);
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
  // The held BVH checks its own arrays; these are the cloud's, which live in the same pool.
  const size_t poolBytes = a_pool.usedBytes();

  EBGEOMETRY_REQUIRE(m_positions.endByte() <= poolBytes && m_leafOff.endByte() <= poolBytes &&
                       m_leafCnt.endByte() <= poolBytes && m_order.endByte() <= poolBytes,
                     "PointCloudBVH::rebasedView: the cloud's arrays must fit inside the pool (the pool holds %zu "
                     "bytes)",
                     poolBytes);

  // The cloud descriptors are pool-relative offsets, identical in any mirror of the pool, so only
  // the held BVH's attachment changes.
  PointCloudBVH view = *this;

  view.m_bvh = m_bvh.rebasedView(a_pool);

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

  const std::vector<Vec3T<T>>      positions = toHost(m_positions, srcBase);
  const std::vector<std::uint32_t> leafOff   = toHost(m_leafOff, srcBase);
  const std::vector<std::uint32_t> leafCnt   = toHost(m_leafCnt, srcBase);
  const std::vector<std::uint32_t> order     = toHost(m_order, srcBase);

  PointCloudBVH copy = *this;

  copy.m_bvh = m_bvh.deepCopy(a_dstPool);
  copy.storeCloud(a_dstPool, positions, leafOff, leafCnt, order);

  return copy;
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST
inline typename PointCloudBVH<T, K, W>::BuildResult
PointCloudBVH<T, K, W>::buildTree(const std::vector<Vec3T<T>>& a_positions, std::size_t a_leafSize)
{
  static_assert(std::is_floating_point_v<T>, "PointCloudBVH::buildTree requires a floating-point type T");
  static_assert(K >= 2, "PointCloudBVH::buildTree requires a branching factor K >= 2");
  static_assert(W >= 1, "PointCloudBVH::buildTree requires a SIMD width W >= 1");

  // Before anything is sized from it: the build stores uint32 indices.
  PointCloudDetail::requireValidCloud("PointCloudBVH", a_positions);

  EBGEOMETRY_REQUIRE(a_leafSize >= 1, "PointCloudBVH: the target leaf size must be at least 1 (%zu)", a_leafSize);

  const std::size_t numPoints = a_positions.size();

  BuildResult result;

  result.leafOff.assign(numPoints, 0);
  result.leafCnt.assign(numPoints, 0);
  result.nodes.reserve(numPoints > 0 ? 2 * numPoints / Math::max<std::size_t>(a_leafSize, 1) + 4 : 4);
  result.primitives.reserve(numPoints / W + 4);

  std::vector<std::uint32_t> indices(numPoints);
  std::iota(indices.begin(), indices.end(), std::uint32_t{0});

  // Recursion over index ranges [lo, hi) of `indices`. All partitioning is in place on `indices`;
  // leaves pack their points into PointGroups inline. Kept in a local struct so the recursive calls
  // do not thread every argument.
  struct Builder
  {
    const std::vector<Vec3T<T>>& positions;
    std::vector<std::uint32_t>&  indices;
    BuildResult&                 result;
    const std::size_t            leafSize;

    // Recursively split indices[lo, hi) into a_numParts index ranges by longest-axis midpoint (in
    // place), appending each final range to a_ranges.
    void
    partition(std::uint32_t                                         a_lo,
              std::uint32_t                                         a_hi,
              int                                                   a_numParts,
              std::vector<std::pair<std::uint32_t, std::uint32_t>>& a_ranges)
    {
      EBGEOMETRY_EXPECT(a_lo <= a_hi);
      EBGEOMETRY_EXPECT(a_numParts >= 1);

      if (a_numParts <= 1 || a_hi <= a_lo) {
        a_ranges.emplace_back(a_lo, a_hi);

        return;
      }

      // Bounding box of the points currently in this range -- split along its longest axis.
      Vec3T<T> rangeLo = +Vec3T<T>::max();
      Vec3T<T> rangeHi = -Vec3T<T>::max();

      for (std::uint32_t i = a_lo; i < a_hi; i++) {
        const Vec3T<T>& p = positions[indices[i]];

        rangeLo = min(rangeLo, p);
        rangeHi = max(rangeHi, p);
      }

      const int axis       = (rangeHi - rangeLo).maxDir(true);
      const T   splitCoord = T(0.5) * (rangeLo[axis] + rangeHi[axis]);

      EBGEOMETRY_EXPECT(axis >= 0 && axis < 3);

      const auto splitIter = std::partition(
        indices.begin() + a_lo, indices.begin() + a_hi, [this, axis, splitCoord](std::uint32_t a_id) noexcept {
          return positions[a_id][axis] < splitCoord;
        });

      std::uint32_t splitIndex = static_cast<std::uint32_t>(splitIter - indices.begin());

      const int leftParts  = a_numParts / 2;
      const int rightParts = a_numParts - leftParts;

      // The midpoint can leave one side with fewer points than its share of leaves: points that
      // coincide along the axis never separate, and a tight cluster beside a few outliers puts
      // nearly everything on one side. Clamping alone would then peel off only a few points per
      // level, and the tree depth would grow linearly with the cluster size. Split by count instead
      // (an object median along the same axis), which keeps the depth logarithmic.
      if (splitIndex < a_lo + leftParts || splitIndex > a_hi - rightParts) {
        const std::uint64_t count = a_hi - a_lo;

        splitIndex = a_lo + static_cast<std::uint32_t>(count * static_cast<std::uint64_t>(leftParts) /
                                                       static_cast<std::uint64_t>(a_numParts));

        std::nth_element(indices.begin() + a_lo,
                         indices.begin() + splitIndex,
                         indices.begin() + a_hi,
                         [this, axis](std::uint32_t a_lhs, std::uint32_t a_rhs) noexcept {
                           return positions[a_lhs][axis] < positions[a_rhs][axis];
                         });
      }

      // Keep each half populated enough to yield its share of non-empty leaves.
      splitIndex = Math::max<std::uint32_t>(a_lo + leftParts, Math::min<std::uint32_t>(a_hi - rightParts, splitIndex));

      EBGEOMETRY_EXPECT(splitIndex >= a_lo && splitIndex <= a_hi);

      partition(a_lo, splitIndex, leftParts, a_ranges);
      partition(splitIndex, a_hi, rightParts, a_ranges);
    }

    // Build the subtree over indices[lo, hi); returns its node index. Fills node bbox bottom-up.
    std::uint32_t
    build(std::uint32_t a_lo, std::uint32_t a_hi)
    {
      EBGEOMETRY_EXPECT(a_hi > a_lo);

      const std::uint32_t nodeIndex = static_cast<std::uint32_t>(result.nodes.size());
      result.nodes.emplace_back();

      // Make a leaf once the range is small enough -- but also never try to split a range with fewer
      // than K points, since partition() cannot produce K non-empty children from it (an empty child
      // would become a malformed 0-primitive leaf with an inverted bounding volume). With the default
      // leaf size (16*W >> K) this never binds; it only matters for very small targetLeafSize.
      if (a_hi - a_lo <= Math::max<std::size_t>(leafSize, K)) {
        const std::uint32_t firstGroup = static_cast<std::uint32_t>(result.primitives.size());

        Vec3T<T> boxLo = +Vec3T<T>::max();
        Vec3T<T> boxHi = -Vec3T<T>::max();

        for (std::uint32_t groupStart = a_lo; groupStart < a_hi; groupStart += static_cast<std::uint32_t>(W)) {
          const std::uint32_t count = Math::min<std::uint32_t>(static_cast<std::uint32_t>(W), a_hi - groupStart);

          EBGEOMETRY_EXPECT(count >= 1 && count <= W);

          Array<Vec3T<T>, W>      groupPositions;
          Array<std::uint32_t, W> groupIds;

          for (std::uint32_t j = 0; j < count; j++) {
            groupPositions[j] = positions[indices[groupStart + j]];
            groupIds[j]       = indices[groupStart + j];

            boxLo = min(boxLo, groupPositions[j]);
            boxHi = max(boxHi, groupPositions[j]);
          }

          PointGroup group;
          group.pack(groupPositions.data(), groupIds.data(), count);
          result.primitives.push_back(group);
        }

        const std::uint32_t groupCount = static_cast<std::uint32_t>(result.primitives.size()) - firstGroup;

        for (std::uint32_t i = a_lo; i < a_hi; i++) {
          result.leafOff[indices[i]] = firstGroup;
          result.leafCnt[indices[i]] = groupCount;
        }

        result.nodes[nodeIndex].setBoundingVolume(AABB(boxLo, boxHi));
        result.nodes[nodeIndex].setPrimitivesOffset(firstGroup);
        result.nodes[nodeIndex].setNumPrimitives(groupCount);

        return nodeIndex;
      }

      std::vector<std::pair<std::uint32_t, std::uint32_t>> ranges;
      ranges.reserve(K);
      this->partition(a_lo, a_hi, static_cast<int>(K), ranges);

      EBGEOMETRY_EXPECT(ranges.size() == K);

      Vec3T<T>                boxLo = +Vec3T<T>::max();
      Vec3T<T>                boxHi = -Vec3T<T>::max();
      Array<std::uint32_t, K> children;

      for (std::size_t k = 0; k < K; k++) {
        children[k]          = this->build(ranges[k].first, ranges[k].second);
        const AABB& childBox = result.nodes[children[k]].getBoundingVolume();

        boxLo = min(boxLo, childBox.getLowCorner());
        boxHi = max(boxHi, childBox.getHighCorner());
      }

      for (std::size_t k = 0; k < K; k++) {
        result.nodes[nodeIndex].setChildOffset(children[k], k); // interior: setNumPrimitives stays 0
      }

      result.nodes[nodeIndex].setBoundingVolume(AABB(boxLo, boxHi));

      return nodeIndex;
    }
  };

  if (numPoints > 0) {
    Builder builder{a_positions, indices, result, Math::max<std::size_t>(a_leafSize, 1)};
    builder.build(0, static_cast<std::uint32_t>(numPoints));
  }

  // After the build, `indices` is the point permutation in leaf (depth-first) order -- spatially
  // coherent for free. Keep it so batch queries can iterate in that order without re-sorting.
  result.order = std::move(indices);

  return result;
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
  const PODSpan<const Node>       nodes  = m_bvh.getNodes();
  const PODSpan<const PointGroup> groups = m_bvh.getPrimitives();

  EBGEOMETRY_EXPECT(a_seedCnt == 0 || (a_seedOff + a_seedCnt) <= groups.size());

  // The running a_k-best set (ascending by distance). It is the pruneTraverse() state, so the held
  // BVH's traversal prunes off its current bound.
  using Best = PointCloud::KBest<T>;

  Best best(a_out, a_k, a_exclude);

  // An empty cloud has no nodes; every traversal below would index node 0. Bail out.
  if (nodes.size() == 0) {
    return 0;
  }

  // Each point is in exactly one group, and the scan stops at the group's real points (the padded
  // lanes repeat the last one), so every point is offered once and the set needs no de-duplication.
  const auto scanLeaf = [&groups, &a_query](Best& a_best, std::size_t a_off, std::size_t a_cnt) noexcept {
    for (std::size_t g = 0; g < a_cnt; g++) {
      const PointGroup&   group     = groups[static_cast<std::uint32_t>(a_off + g)];
      const Array<T, W>   distances = group.getDistances2(a_query);
      const std::uint32_t numValid  = group.numValid();

      for (std::uint32_t lane = 0; lane < numValid; lane++) {
        a_best.insert(distances[lane], group.getPointId(lane));
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
    // descent -- without paying pruneTraverse's per-interior-node child-sort and its separate
    // m_childAabbSoA SIMD load. For these cheap SoA point leaves that makes the unordered DFS ~20%
    // faster. (This holds ONLY because of the seed: an unseeded search starts from an infinite bound,
    // where the near-first order is decisive.)
    //
    // Prune-before-push: a child is pushed only when its bounding volume is closer than the current
    // best, so the working set stays bounded to nodes that can still improve the result. It pushes
    // at most K children per visited node, exactly as pruneTraverse does, so the stack depth
    // PackedBVH already validated the tree against for this compilation pass (host, or the smaller
    // device budget at rebasedView()) bounds it too; the guard catches a pathological overrun in
    // debug builds.
    constexpr std::size_t maxStack = Packed::traversalStackDepth();

    std::uint32_t stack[maxStack];
    std::size_t   stackTop = 0;

    stack[stackTop++] = 0U;

    while (stackTop > 0) {
      const Node& node = nodes[stack[--stackTop]];

      if (node.getDistanceToBoundingVolume2(a_query) >= best.bound()) {
        continue; // stale: best tightened since this node was pushed
      }

      if (node.isLeaf()) {
        const std::uint32_t primOffset = node.getPrimitivesOffset();

        if (primOffset != a_seedOff) {
          scanLeaf(best, primOffset, node.getNumPrimitives());
        }
      }
      else {
        const auto& childOffsets = node.getChildOffsets();

        for (std::size_t k = 0; k < K; k++) {
          if (nodes[childOffsets[k]].getDistanceToBoundingVolume2(a_query) < best.bound()) {
            EBGEOMETRY_EXPECT(stackTop < maxStack);

            stack[stackTop++] = childOffsets[k];
          }
        }
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

  m_bvh.pruneTraverse(a_query, best, evalLeaf, pruneDist2);

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
