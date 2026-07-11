// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_PointCloudBVHImplem.hpp
 * @brief   Implementation of EBGeometry_PointCloudBVH.hpp
 * @author  Robert Marskar
 */

#ifndef EBGeometry_PointCloudBVHImplem
#define EBGeometry_PointCloudBVHImplem

// Std includes
#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <utility>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PointCloudBVH.hpp"

namespace EBGeometry {

template <class T, class Meta, size_t K, size_t W>
inline PointCloudBVH<T, Meta, K, W>::PointCloudBVH(const std::vector<Vec3T<T>>& a_positions,
                                                   const std::vector<Meta>&     a_metadata,
                                                   std::size_t                  a_targetLeafSize)
  : PointCloudBVH(buildTree(a_positions, a_targetLeafSize), a_positions, a_metadata)
{
  static_assert(std::is_floating_point_v<T>, "PointCloudBVH requires a floating-point type T");
  static_assert(K >= 2, "PointCloudBVH requires a branching factor K >= 2");
  static_assert(W >= 1, "PointCloudBVH requires a SIMD width W >= 1");

  EBGEOMETRY_EXPECT(a_positions.size() == a_metadata.size());
  EBGEOMETRY_EXPECT(a_targetLeafSize >= 1);
}

template <class T, class Meta, size_t K, size_t W>
inline PointCloudBVH<T, Meta, K, W>::PointCloudBVH(BuildResult&&                a_build,
                                                   const std::vector<Vec3T<T>>& a_positions,
                                                   const std::vector<Meta>&     a_metadata)
  : Base(std::move(a_build.nodes), std::move(a_build.primitives)),
    m_positions(a_positions),
    m_metadata(a_metadata),
    m_leafOff(std::move(a_build.leafOff)),
    m_leafCnt(std::move(a_build.leafCnt)),
    m_order(std::move(a_build.order))
{
  // a_build was consumed above, so validate the arrays now living in this class against the cloud.
  EBGEOMETRY_EXPECT(m_metadata.size() == m_positions.size());
  EBGEOMETRY_EXPECT(m_leafOff.size() == m_positions.size());
  EBGEOMETRY_EXPECT(m_leafCnt.size() == m_positions.size());
  EBGEOMETRY_EXPECT(m_order.size() == m_positions.size());
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::BuildResult
PointCloudBVH<T, Meta, K, W>::buildTree(const std::vector<Vec3T<T>>& a_positions, std::size_t a_leafSize)
{
  static_assert(std::is_floating_point_v<T>, "PointCloudBVH::buildTree requires a floating-point type T");
  static_assert(K >= 2, "PointCloudBVH::buildTree requires a branching factor K >= 2");
  static_assert(W >= 1, "PointCloudBVH::buildTree requires a SIMD width W >= 1");

  const std::size_t numPoints = a_positions.size();

  BuildResult result;

  result.leafOff.assign(numPoints, 0);
  result.leafCnt.assign(numPoints, 0);
  result.nodes.reserve(numPoints > 0 ? 2 * numPoints / std::max<std::size_t>(a_leafSize, 1) + 4 : 4);
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

      // Keep each half populated enough to yield its share of non-empty leaves.
      splitIndex = std::max<std::uint32_t>(a_lo + leftParts, std::min<std::uint32_t>(a_hi - rightParts, splitIndex));

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
      if (a_hi - a_lo <= std::max<std::size_t>(leafSize, K)) {
        const std::uint32_t firstGroup = static_cast<std::uint32_t>(result.primitives.size());

        Vec3T<T> boxLo = +Vec3T<T>::max();
        Vec3T<T> boxHi = -Vec3T<T>::max();

        for (std::uint32_t groupStart = a_lo; groupStart < a_hi; groupStart += static_cast<std::uint32_t>(W)) {
          const std::uint32_t count = std::min<std::uint32_t>(static_cast<std::uint32_t>(W), a_hi - groupStart);

          EBGEOMETRY_EXPECT(count >= 1 && count <= W);

          std::array<Vec3T<T>, W>    groupPositions;
          std::array<std::size_t, W> groupMeta;

          for (std::uint32_t j = 0; j < count; j++) {
            groupPositions[j] = positions[indices[groupStart + j]];
            groupMeta[j]      = indices[groupStart + j];

            boxLo = min(boxLo, groupPositions[j]);
            boxHi = max(boxHi, groupPositions[j]);
          }

          PointGroup group;
          group.pack(groupPositions.data(), groupMeta.data(), count);
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

      Vec3T<T>                     boxLo = +Vec3T<T>::max();
      Vec3T<T>                     boxHi = -Vec3T<T>::max();
      std::array<std::uint32_t, K> children;

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
    Builder builder{a_positions, indices, result, std::max<std::size_t>(a_leafSize, 1)};
    builder.build(0, static_cast<std::uint32_t>(numPoints));
  }

  // After the build, `indices` is the point permutation in leaf (depth-first) order -- spatially
  // coherent for free. Keep it so batch queries can iterate in that order without re-sorting.
  result.order = std::move(indices);

  return result;
}

template <class T, class Meta, size_t K, size_t W>
inline void
PointCloudBVH<T, Meta, K, W>::query(const Vec3T<T>& a_query,
                                    std::size_t     a_k,
                                    Hit*            a_out,
                                    std::size_t&    a_found,
                                    std::size_t     a_exclude,
                                    std::uint32_t   a_seedOff,
                                    std::uint32_t   a_seedCnt) const noexcept
{
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_query[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[2]));
  EBGEOMETRY_EXPECT(a_exclude == s_none || a_exclude < m_positions.size());
  EBGEOMETRY_EXPECT(a_seedCnt == 0 || (a_seedOff + a_seedCnt) <= this->m_primitives.size());

  a_found = 0;

  // An empty cloud has no nodes; every traversal below would index m_linearNodes[0]. Bail out.
  if (this->m_linearNodes.empty()) {
    return;
  }

  // Fast path for the single-nearest case (closestPoint / nearestNeighbor / all-NN with k==1), the
  // common one: a lean {distance, index} state instead of the general k-best set, so the per-lane
  // work is just a compare-and-maybe-store.
  if (a_k == 1) {
    struct Best
    {
      T           distanceSquared = std::numeric_limits<T>::max();
      std::size_t index           = s_none;
    };

    Best best;

    const auto scanLeafBest = [this, &a_query, a_exclude](Best& a_best, std::size_t a_off, std::size_t a_cnt) noexcept {
      for (std::size_t g = 0; g < a_cnt; g++) {
        const PointGroup&      group     = this->m_primitives[a_off + g];
        const std::array<T, W> distances = group.getDistances2(a_query);

        for (std::size_t lane = 0; lane < W; lane++) {
          const std::size_t cloudIndex = group.getMetaData(lane);

          if (cloudIndex != a_exclude && distances[lane] < a_best.distanceSquared) {
            a_best.distanceSquared = distances[lane];
            a_best.index           = cloudIndex;
          }
        }
      }
    };

    if (a_seedCnt > 0) {
      // Seeded self-query: scanning the query point's own leaf first gives a tight prune bound
      // immediately, so a plain unordered scalar DFS prunes just as hard as an ordered descent --
      // without paying pruneTraverse's per-interior-node child-sort and its separate m_childAabbSoA
      // SIMD load. For these cheap SoA point leaves that makes the unordered DFS ~20% faster. (This
      // holds ONLY because of the seed: see the external branch below.)
      scanLeafBest(best, a_seedOff, a_seedCnt);

      // Prune-before-push scalar DFS: a child is pushed only when its bounding volume is closer than
      // the current best, so the working set stays bounded to nodes that can still improve the result
      // (the seed gives a tight bound up front). This keeps the stack small without pruneTraverse's
      // per-node child sort / SoA load. The 256-entry stack matches PackedBVH::pruneTraverse's
      // convention; the guard catches a pathological overrun in debug builds.
      constexpr int maxStack = 256;

      std::uint32_t stack[maxStack];
      int           stackTop = 0;

      stack[stackTop++] = 0U;

      while (stackTop > 0) {
        const Node& node = this->m_linearNodes[stack[--stackTop]];

        if (node.getDistanceToBoundingVolume2(a_query) >= best.distanceSquared) {
          continue; // stale: best tightened since this node was pushed
        }

        if (node.isLeaf()) {
          const std::uint32_t primOffset = node.getPrimitivesOffset();

          if (primOffset != a_seedOff) {
            scanLeafBest(best, primOffset, node.getNumPrimitives());
          }
        }
        else {
          const auto& childOffsets = node.getChildOffsets();

          for (std::size_t k = 0; k < K; k++) {
            if (this->m_linearNodes[childOffsets[k]].getDistanceToBoundingVolume2(a_query) < best.distanceSquared) {
              EBGEOMETRY_EXPECT(stackTop < maxStack);

              stack[stackTop++] = childOffsets[k];
            }
          }
        }
      }
    }
    else {
      // Unseeded external query: the prune bound starts at infinity, so the order in which leaves are
      // visited is decisive -- pruneTraverse's near-first ordered descent tightens the bound fast,
      // whereas an unordered DFS would explore huge far-away regions before finding a close point (it
      // is ~17x slower here). The ordering cost that does not pay for seeded self-queries is exactly
      // what makes external queries fast, so this branch keeps the base traversal.
      const auto evalLeaf = [&scanLeafBest](Best& a_best, std::size_t a_off, std::size_t a_cnt) noexcept {
        scanLeafBest(a_best, a_off, a_cnt);
      };

      const auto pruneDist2 = [](const Best& a_best) noexcept -> T { return a_best.distanceSquared; };

      this->pruneTraverse(a_query, best, evalLeaf, pruneDist2);
    }

    if (best.index != s_none) {
      a_out[0] = Hit{best.index, best.distanceSquared};
      a_found  = 1;
    }

    return;
  }

  // A running a_k-best set (ascending by distance) plus the config the leaf scan needs. It is the
  // pruneTraverse() state, so the inherited SIMD-optimized traversal prunes off its current bound.
  struct QState
  {
    Hit*        out;
    std::size_t k;
    std::size_t exclude;
    std::size_t found = 0;
    T           bound = std::numeric_limits<T>::max();

    inline void
    insert(T a_d2, std::size_t a_idx) noexcept
    {
      if (a_idx == exclude || a_d2 >= bound) {
        return; // self, or farther than the current worst -- cheap reject before the de-dupe scan
      }

      if (k > 1) {
        for (std::size_t i = 0; i < found; i++) {
          if (out[i].index == a_idx) {
            return; // already held (padded lanes / seed-then-traverse would otherwise double-count)
          }
        }
      }

      std::size_t slot = (found < k) ? found++ : (k - 1);
      out[slot]        = Hit{a_idx, a_d2};

      while (slot > 0 && out[slot].distanceSquared < out[slot - 1].distanceSquared) {
        std::swap(out[slot], out[slot - 1]);
        slot--;
      }

      bound = (found < k) ? std::numeric_limits<T>::max() : out[k - 1].distanceSquared;
    }
  };

  QState state{a_out, a_k, a_exclude};

  const auto processLeaf = [this, &a_query](QState& a_state, std::size_t a_off, std::size_t a_cnt) noexcept {
    for (std::size_t g = 0; g < a_cnt; g++) {
      const PointGroup&      group     = this->m_primitives[a_off + g];
      const std::array<T, W> distances = group.getDistances2(a_query);

      for (std::size_t lane = 0; lane < W; lane++) {
        a_state.insert(distances[lane], group.getMetaData(lane));
      }
    }
  };

  // Seed the bound from the query point's own leaf (self queries only), then skip that leaf below so
  // the descent prunes with an already-tight bound instead of starting from +inf.
  if (a_seedCnt > 0) {
    processLeaf(state, a_seedOff, a_seedCnt);
  }

  const auto evalLeaf = [&](QState& a_state, std::size_t a_off, std::size_t a_cnt) noexcept {
    if (a_seedCnt > 0 && static_cast<std::uint32_t>(a_off) == a_seedOff) {
      return; // own leaf already scanned in the seed
    }

    processLeaf(a_state, a_off, a_cnt);
  };

  const auto pruneDist2 = [](const QState& a_state) noexcept -> T { return a_state.bound; };

  this->pruneTraverse(a_query, state, evalLeaf, pruneDist2);

  a_found = state.found;
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::closestPoint(const Vec3T<T>& a_query) const noexcept
{
  Hit         hit;
  std::size_t found = 0;

  this->query(a_query, 1, &hit, found, s_none, 0, 0);

  return hit;
}

template <class T, class Meta, size_t K, size_t W>
inline std::size_t
PointCloudBVH<T, Meta, K, W>::closestPoints(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const noexcept
{
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);

  std::size_t found = 0;

  this->query(a_query, a_k, a_out, found, s_none, 0, 0);

  return found;
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::nearestNeighbor(std::size_t a_point) const noexcept
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());

  Hit         hit;
  std::size_t found = 0;

  this->query(m_positions[a_point], 1, &hit, found, a_point, m_leafOff[a_point], m_leafCnt[a_point]);

  return hit;
}

template <class T, class Meta, size_t K, size_t W>
inline std::size_t
PointCloudBVH<T, Meta, K, W>::nearestNeighbors(std::size_t a_point, std::size_t a_k, Hit* a_out) const noexcept
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);

  std::size_t found = 0;

  this->query(m_positions[a_point], a_k, a_out, found, a_point, m_leafOff[a_point], m_leafCnt[a_point]);

  return found;
}

template <class T, class Meta, size_t K, size_t W>
inline std::vector<typename PointCloudBVH<T, Meta, K, W>::Hit>
PointCloudBVH<T, Meta, K, W>::allNearestNeighbors(std::size_t a_k) const
{
  EBGEOMETRY_EXPECT(a_k >= 1);

  const std::size_t numPoints = m_positions.size();

  std::vector<Hit> result(numPoints * a_k);

  // Process points in leaf (build) order: the top-down build already grouped them spatially, so
  // consecutive queries touch nearby leaves and each seeded own-leaf stays hot in cache -- the same
  // benefit a Hilbert sort would give, but reusing m_order costs nothing per call (no re-sort).
  // Ordering affects only speed, not results.
  for (const std::uint32_t p : m_order) {
    EBGEOMETRY_EXPECT(p < numPoints);

    std::size_t found = 0;

    this->query(m_positions[p], a_k, &result[p * a_k], found, p, m_leafOff[p], m_leafCnt[p]);
  }

  return result;
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::bruteForceOne(const Vec3T<T>& a_query, std::size_t a_exclude) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_query[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[2]));
  EBGEOMETRY_EXPECT(a_exclude == s_none || a_exclude < m_positions.size());

  Hit best;

  for (std::size_t i = 0; i < m_positions.size(); i++) {
    if (i == a_exclude) {
      continue;
    }

    const T distanceSquared = (m_positions[i] - a_query).length2();

    if (distanceSquared < best.distanceSquared) {
      best.distanceSquared = distanceSquared;
      best.index           = i;
    }
  }

  return best;
}

template <class T, class Meta, size_t K, size_t W>
inline std::size_t
PointCloudBVH<T, Meta, K, W>::bruteForceK(const Vec3T<T>& a_query,
                                          std::size_t     a_k,
                                          Hit*            a_out,
                                          std::size_t     a_exclude) const
{
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_query[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[2]));
  EBGEOMETRY_EXPECT(a_exclude == s_none || a_exclude < m_positions.size());

  // Full scan into a scratch buffer, then partial_sort the a_k smallest to the front. Deliberately
  // simple and obviously correct -- this is the reference path, not a hot one.
  std::vector<Hit> all;
  all.reserve(m_positions.size());

  for (std::size_t i = 0; i < m_positions.size(); i++) {
    if (i == a_exclude) {
      continue;
    }

    all.push_back(Hit{i, (m_positions[i] - a_query).length2()});
  }

  const std::size_t k = std::min(a_k, all.size());

  std::partial_sort(
    all.begin(),
    all.begin() + static_cast<std::ptrdiff_t>(k),
    all.end(),
    [](const Hit& a_lhs, const Hit& a_rhs) noexcept { return a_lhs.distanceSquared < a_rhs.distanceSquared; });

  for (std::size_t j = 0; j < k; j++) {
    a_out[j] = all[j];
  }

  return k;
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::closestPointBruteForce(const Vec3T<T>& a_query) const noexcept
{
  return this->bruteForceOne(a_query, s_none);
}

template <class T, class Meta, size_t K, size_t W>
inline std::size_t
PointCloudBVH<T, Meta, K, W>::closestPointsBruteForce(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const
{
  return this->bruteForceK(a_query, a_k, a_out, s_none);
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::nearestNeighborBruteForce(std::size_t a_point) const noexcept
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());

  return this->bruteForceOne(m_positions[a_point], a_point);
}

template <class T, class Meta, size_t K, size_t W>
inline std::size_t
PointCloudBVH<T, Meta, K, W>::nearestNeighborsBruteForce(std::size_t a_point, std::size_t a_k, Hit* a_out) const
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);

  return this->bruteForceK(m_positions[a_point], a_k, a_out, a_point);
}

} // namespace EBGeometry

#endif
