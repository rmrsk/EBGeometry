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
#include <utility>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PointCloudBVH.hpp"
#include "EBGeometry_PointCloudDetail.hpp"

namespace EBGeometry {

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline PointCloudBVH<T, Meta, K, W>::PointCloudBVH(Pool&                        a_pool,
                                                   const std::vector<Vec3T<T>>& a_positions,
                                                   const std::vector<Meta>&     a_metadata,
                                                   std::size_t                  a_targetLeafSize)
  : PointCloudBVH(a_pool, a_positions, a_metadata, a_targetLeafSize, BuildTables{})
{
  static_assert(std::is_floating_point_v<T>, "PointCloudBVH requires a floating-point type T");
  static_assert(K >= 2, "PointCloudBVH requires a branching factor K >= 2");
  static_assert(W >= 1, "PointCloudBVH requires a SIMD width W >= 1");

  // a_targetLeafSize is checked by buildBVH(), before the build uses it.
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline PointCloudBVH<T, Meta, K, W>::PointCloudBVH(Pool&                        a_pool,
                                                   const std::vector<Vec3T<T>>& a_positions,
                                                   const std::vector<Meta>&     a_metadata,
                                                   std::size_t                  a_targetLeafSize,
                                                   BuildTables&&                a_tables)
  : m_bvh(buildBVH(a_pool, a_positions, a_targetLeafSize, a_tables))
{
  this->storeCloud(a_pool, a_positions, a_metadata, a_tables.leafOff, a_tables.leafCnt, a_tables.order);
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline void
PointCloudBVH<T, Meta, K, W>::storeCloud(Pool&                             a_pool,
                                         const std::vector<Vec3T<T>>&      a_positions,
                                         const std::vector<Meta>&          a_metadata,
                                         const std::vector<std::uint32_t>& a_leafOff,
                                         const std::vector<std::uint32_t>& a_leafCnt,
                                         const std::vector<std::uint32_t>& a_order)
{
  // Every array must come from the pool the BVH was built in: they all resolve against its base.
  EBGEOMETRY_EXPECT(m_bvh.isAttachedTo(a_pool));

  PointCloudDetail::requireValidCloud("PointCloudBVH", a_positions, a_metadata.size());

  EBGEOMETRY_EXPECT(a_leafOff.size() == a_positions.size());
  EBGEOMETRY_EXPECT(a_leafCnt.size() == a_positions.size());
  EBGEOMETRY_EXPECT(a_order.size() == a_positions.size());

  const auto numPoints = static_cast<std::uint32_t>(a_positions.size());
  const auto numMeta   = static_cast<std::uint32_t>(a_metadata.size());

  m_positions.reserveFrom(a_pool, numPoints);
  m_metadata.reserveFrom(a_pool, numMeta);
  m_leafOff.reserveFrom(a_pool, numPoints);
  m_leafCnt.reserveFrom(a_pool, numPoints);
  m_order.reserveFrom(a_pool, numPoints);

  // Resolve the base only after every reservation above: a reserve can grow the pool, which moves
  // the block and invalidates any address taken before it.
  void* poolBase = const_cast<void*>(this->base());

  m_positions.assign(poolBase, a_positions.data(), numPoints);
  m_metadata.assign(poolBase, a_metadata.data(), numMeta);
  m_leafOff.assign(poolBase, a_leafOff.data(), numPoints);
  m_leafCnt.assign(poolBase, a_leafCnt.data(), numPoints);
  m_order.assign(poolBase, a_order.data(), numPoints);
}

template <class T, class Meta, size_t K, size_t W>
template <class U>
EBGEOMETRY_HOST
inline std::vector<U>
PointCloudBVH<T, Meta, K, W>::toHost(const PODVector<U>& a_array, const void* a_base)
{
  const U* first = a_array.data(a_base);

  return std::vector<U>(first, first + a_array.size());
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline PointCloudBVH<T, Meta, K, W>
PointCloudBVH<T, Meta, K, W>::rebasedView(const Pool& a_pool) const noexcept
{
  // The held BVH checks its own arrays; these are the cloud's, which live in the same pool.
  const size_t poolBytes = a_pool.usedBytes();

  EBGEOMETRY_REQUIRE(m_positions.endByte() <= poolBytes && m_metadata.endByte() <= poolBytes &&
                       m_leafOff.endByte() <= poolBytes && m_leafCnt.endByte() <= poolBytes &&
                       m_order.endByte() <= poolBytes,
                     "PointCloudBVH::rebasedView: the cloud's arrays must fit inside the pool (the pool holds %zu "
                     "bytes)",
                     poolBytes);

  // The cloud descriptors are pool-relative offsets, identical in any mirror of the pool, so only
  // the held BVH's attachment changes.
  PointCloudBVH view = *this;

  view.m_bvh = m_bvh.rebasedView(a_pool);

  return view;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline PointCloudBVH<T, Meta, K, W>
PointCloudBVH<T, Meta, K, W>::deepCopy(Pool& a_dstPool) const
{
  // Read everything out before reserving anything: a_dstPool may be this object's own pool, and a
  // reserve there can move the block under any address taken from it.
  const void* srcBase = this->base();

  const std::vector<Vec3T<T>>      positions = toHost(m_positions, srcBase);
  const std::vector<Meta>          metadata  = toHost(m_metadata, srcBase);
  const std::vector<std::uint32_t> leafOff   = toHost(m_leafOff, srcBase);
  const std::vector<std::uint32_t> leafCnt   = toHost(m_leafCnt, srcBase);
  const std::vector<std::uint32_t> order     = toHost(m_order, srcBase);

  PointCloudBVH copy = *this;

  copy.m_bvh = m_bvh.deepCopy(a_dstPool);
  copy.storeCloud(a_dstPool, positions, metadata, leafOff, leafCnt, order);

  return copy;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline typename PointCloudBVH<T, Meta, K, W>::Packed
PointCloudBVH<T, Meta, K, W>::buildBVH(Pool&                        a_pool,
                                       const std::vector<Vec3T<T>>& a_positions,
                                       std::size_t                  a_leafSize,
                                       BuildTables&                 a_tables)
{
  // Before anything is sized from it: the build stores uint32 indices.
  PointCloudDetail::requireValidCloud("PointCloudBVH", a_positions, a_positions.size());

  EBGEOMETRY_REQUIRE(a_leafSize >= 1 && a_leafSize <= std::size_t(Math::Limits<std::uint32_t>::max()),
                     "PointCloudBVH: the target leaf size must be at least 1 and fit in 32 bits (%zu)",
                     a_leafSize);

  const std::size_t numPoints = a_positions.size();

  std::vector<AABB> boxes;

  boxes.reserve(numPoints);

  for (const auto& position : a_positions) {
    boxes.emplace_back(position, position);
  }

  const BVH::BuildSpec spec{BVH::Strategy::Midpoint, BVH::Curve::Morton, static_cast<std::uint32_t>(a_leafSize)};

  const BVH::Topology<T, K> topology = BVH::buildTopology<T, K>(boxes, spec);

  a_tables.leafOff.assign(numPoints, 0);
  a_tables.leafCnt.assign(numPoints, 0);
  a_tables.order = topology.order;

  // Pack each leaf's points into SoA groups of W, each lane carrying its cloud index, and record
  // for every point the group range of its own leaf, which seeds a self-query.
  const auto packLeaf =
    [&a_positions, &a_tables](const std::uint32_t* a_items, std::uint32_t a_count, std::vector<PointGroup>& a_out) {
      const auto firstGroup = static_cast<std::uint32_t>(a_out.size());

      for (std::uint32_t first = 0; first < a_count; first += static_cast<std::uint32_t>(W)) {
        const std::uint32_t count = Math::min(static_cast<std::uint32_t>(W), a_count - first);

        Array<Vec3T<T>, W>    groupPositions;
        Array<std::size_t, W> groupMeta;

        for (std::uint32_t j = 0; j < count; j++) {
          groupPositions[j] = a_positions[a_items[first + j]];
          groupMeta[j]      = a_items[first + j];
        }

        PointGroup group;

        group.pack(groupPositions.data(), groupMeta.data(), count);

        a_out.push_back(group);
      }

      const auto groupCount = static_cast<std::uint32_t>(a_out.size()) - firstGroup;

      for (std::uint32_t j = 0; j < a_count; j++) {
        a_tables.leafOff[a_items[j]] = firstGroup;
        a_tables.leafCnt[a_items[j]] = groupCount;
      }
    };

  return Packed(a_pool, topology, packLeaf);
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
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

  // Resolved once per query: a query reserves nothing, so the pool block cannot move under them.
  const PODSpan<const Node>       nodes  = m_bvh.getNodes();
  const PODSpan<const PointGroup> groups = m_bvh.getPrimitives();

  EBGEOMETRY_EXPECT(a_seedCnt == 0 || (a_seedOff + a_seedCnt) <= groups.size());

  a_found = 0;

  // An empty cloud has no nodes; every traversal below would index node 0. Bail out.
  if (nodes.size() == 0) {
    return;
  }

  // Fast path for the single-nearest case (closestPoint / nearestNeighbor / all-NN with k==1), the
  // common one: a lean {distance, index} state instead of the general k-best set, so the per-lane
  // work is just a compare-and-maybe-store.
  if (a_k == 1) {
    struct Best
    {
      T           distanceSquared = Math::Limits<T>::max();
      std::size_t index           = s_none;
    };

    Best best;

    const auto scanLeafBest =
      [&groups, &a_query, a_exclude](Best& a_best, std::size_t a_off, std::size_t a_cnt) noexcept {
        for (std::size_t g = 0; g < a_cnt; g++) {
          const PointGroup& group     = groups[static_cast<std::uint32_t>(a_off + g)];
          const Array<T, W> distances = group.getDistances2(a_query);

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
      // without paying pruneTraverse's per-node sort of the slots. For these cheap SoA point leaves
      // that makes the unordered DFS faster. (This
      // holds ONLY because of the seed: see the external branch below.)
      scanLeafBest(best, a_seedOff, a_seedCnt);

      // Prune-before-push scalar DFS: a slot is pushed only when its box is closer than the current
      // best, so the working set stays bounded to subtrees that can still improve the result (the seed
      // gives a tight bound up front). This skips pruneTraverse's per-node sort of the slots. It
      // pushes at most K slots per node expanded, exactly as pruneTraverse does, so the stack depth
      // PackedBVH already validated the tree against for this compilation pass (host, or the smaller
      // device budget at rebasedView()) bounds it too; the guard catches a pathological overrun in
      // debug builds.
      constexpr std::size_t maxStack = Packed::traversalStackDepth();

      // Each entry is a node-and-slot index, node * K + slot, as in pruneTraverse.
      std::uint32_t stack[maxStack];
      std::size_t   stackTop = 0;

      std::uint32_t nodeToExpand = 0;
      bool          expand       = true;

      while (expand) {
        const Node& node = nodes[nodeToExpand];

        for (std::size_t k = 0; k < K && !node.isEmpty(k); k++) {
          if (node.getDistance2(k, a_query) < best.distanceSquared) {
            EBGEOMETRY_EXPECT(stackTop < maxStack);

            stack[stackTop++] = nodeToExpand * static_cast<std::uint32_t>(K) + static_cast<std::uint32_t>(k);
          }
        }

        expand = false;

        while (stackTop > 0) {
          const std::uint32_t entry  = stack[--stackTop];
          const Node&         parent = nodes[entry / static_cast<std::uint32_t>(K)];
          const std::size_t   slot   = entry % static_cast<std::uint32_t>(K);

          if (parent.getDistance2(slot, a_query) >= best.distanceSquared) {
            continue; // stale: best tightened since this slot was pushed
          }

          if (parent.isLeaf(slot)) {
            const std::uint32_t primOffset = parent.getPrimitivesOffset(slot);

            if (primOffset != a_seedOff) {
              scanLeafBest(best, primOffset, parent.getNumPrimitives(slot));
            }
          }
          else {
            nodeToExpand = parent.getChild(slot);
            expand       = true;

            break;
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

      m_bvh.pruneTraverse(a_query, best, evalLeaf, pruneDist2);
    }

    if (best.index != s_none) {
      a_out[0] = Hit{best.index, best.distanceSquared};
      a_found  = 1;
    }

    return;
  }

  // A running a_k-best set (ascending by distance) plus the config the leaf scan needs. It is the
  // pruneTraverse() state, so the held BVH's SIMD-optimized traversal prunes off its current bound.
  struct QState
  {
    Hit*        out;
    std::size_t k;
    std::size_t exclude;
    std::size_t found = 0;
    T           bound = Math::Limits<T>::max();

    EBGEOMETRY_HOST_DEVICE
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

      // Hand-rolled swap: std::swap is not callable from device code before C++20.
      while (slot > 0 && out[slot].distanceSquared < out[slot - 1].distanceSquared) {
        const Hit tmp = out[slot];

        out[slot]     = out[slot - 1];
        out[slot - 1] = tmp;
        slot--;
      }

      bound = (found < k) ? Math::Limits<T>::max() : out[k - 1].distanceSquared;
    }
  };

  QState state{a_out, a_k, a_exclude};

  const auto processLeaf = [&groups, &a_query](QState& a_state, std::size_t a_off, std::size_t a_cnt) noexcept {
    for (std::size_t g = 0; g < a_cnt; g++) {
      const PointGroup& group     = groups[static_cast<std::uint32_t>(a_off + g)];
      const Array<T, W> distances = group.getDistances2(a_query);

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

  m_bvh.pruneTraverse(a_query, state, evalLeaf, pruneDist2);

  a_found = state.found;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::closestPoint(const Vec3T<T>& a_query) const noexcept
{
  Hit         hit;
  std::size_t found = 0;

  this->query(a_query, 1, &hit, found, s_none, 0, 0);

  return hit;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
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
EBGEOMETRY_HOST_DEVICE
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::nearestNeighbor(std::size_t a_point) const noexcept
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());

  const void*         poolBase = this->base();
  const std::uint32_t p        = static_cast<std::uint32_t>(a_point);

  Hit         hit;
  std::size_t found = 0;

  this->query(
    m_positions.at(poolBase, p), 1, &hit, found, a_point, m_leafOff.at(poolBase, p), m_leafCnt.at(poolBase, p));

  return hit;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline std::size_t
PointCloudBVH<T, Meta, K, W>::nearestNeighbors(std::size_t a_point, std::size_t a_k, Hit* a_out) const noexcept
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);

  const void*         poolBase = this->base();
  const std::uint32_t p        = static_cast<std::uint32_t>(a_point);

  std::size_t found = 0;

  this->query(
    m_positions.at(poolBase, p), a_k, a_out, found, a_point, m_leafOff.at(poolBase, p), m_leafCnt.at(poolBase, p));

  return found;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline std::vector<typename PointCloudBVH<T, Meta, K, W>::Hit>
PointCloudBVH<T, Meta, K, W>::allNearestNeighbors(std::size_t a_k) const
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

    std::size_t found = 0;

    this->query(m_positions.at(poolBase, p),
                a_k,
                &result[p * a_k],
                found,
                p,
                m_leafOff.at(poolBase, p),
                m_leafCnt.at(poolBase, p));
  }

  return result;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::bruteForceOne(const Vec3T<T>& a_query, std::size_t a_exclude) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_query[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_query[2]));
  EBGEOMETRY_EXPECT(a_exclude == s_none || a_exclude < m_positions.size());

  const PODSpan<const Vec3T<T>> positions = m_positions.bind(this->base());

  Hit best;

  for (std::size_t i = 0; i < positions.size(); i++) {
    if (i == a_exclude) {
      continue;
    }

    const T distanceSquared = (positions[static_cast<std::uint32_t>(i)] - a_query).length2();

    if (distanceSquared < best.distanceSquared) {
      best.distanceSquared = distanceSquared;
      best.index           = i;
    }
  }

  return best;
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
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
  const PODSpan<const Vec3T<T>> positions = m_positions.bind(this->base());

  std::vector<Hit> all;
  all.reserve(positions.size());

  for (std::size_t i = 0; i < positions.size(); i++) {
    if (i == a_exclude) {
      continue;
    }

    all.push_back(Hit{i, (positions[static_cast<std::uint32_t>(i)] - a_query).length2()});
  }

  const std::size_t k = Math::min(a_k, all.size());

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
EBGEOMETRY_HOST_DEVICE
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::closestPointBruteForce(const Vec3T<T>& a_query) const noexcept
{
  return this->bruteForceOne(a_query, s_none);
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline std::size_t
PointCloudBVH<T, Meta, K, W>::closestPointsBruteForce(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const
{
  return this->bruteForceK(a_query, a_k, a_out, s_none);
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::nearestNeighborBruteForce(std::size_t a_point) const noexcept
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());

  return this->bruteForceOne(this->position(a_point), a_point);
}

template <class T, class Meta, size_t K, size_t W>
EBGEOMETRY_HOST
inline std::size_t
PointCloudBVH<T, Meta, K, W>::nearestNeighborsBruteForce(std::size_t a_point, std::size_t a_k, Hit* a_out) const
{
  EBGEOMETRY_EXPECT(a_point < m_positions.size());
  EBGEOMETRY_EXPECT(a_k >= 1);
  EBGEOMETRY_EXPECT(a_out != nullptr);

  return this->bruteForceK(this->position(a_point), a_k, a_out, a_point);
}

} // namespace EBGeometry

#endif
