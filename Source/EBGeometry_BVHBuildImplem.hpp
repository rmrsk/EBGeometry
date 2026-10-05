// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHBuildImplem.hpp
 * @brief  Implementation of EBGeometry_BVHBuild.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHBUILDIMPLEM_HPP
#define EBGEOMETRY_BVHBUILDIMPLEM_HPP

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
#include "EBGeometry_BVHBuild.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Math.hpp"

namespace EBGeometry {

namespace BVH {

template <class T, class P, size_t K>
inline PackedBVH<T, P, K>::PackedBVH(Pool&                                                          a_pool,
                                     const TreeBVH<T, P, EBGeometry::BoundingVolumes::AABBT<T>, K>& a_tree)
{
  using AABBType = EBGeometry::BoundingVolumes::AABBT<T>;

  // Host-only scratch: the node count is not known until the walk finishes, and growth here costs
  // nothing because finalize() copies the result into pool storage in one shot.
  std::vector<Node> nodes;
  std::vector<P>    prims;

  // Depth-first. Each call reserves a slot by index, then fills child offsets
  // after recursion. Indexing by position (not pointer) is safe across
  // vector reallocation; C++17 RHS-before-LHS ensures fresh re-fetch of
  // nodes[idx] after any push_back inside the recursive call.
  std::function<uint32_t(const TreeBVH<T, P, AABBType, K>&)> dfs =
    [&](const TreeBVH<T, P, AABBType, K>& node) -> uint32_t {
    const uint32_t idx = static_cast<uint32_t>(nodes.size());

    nodes.push_back({});
    nodes[idx].m_bv = node.getBoundingVolume();

    if (node.isLeaf()) {
      const auto& leafPrims = node.getPrimitives();

      nodes[idx].m_primOff  = static_cast<uint32_t>(prims.size());
      nodes[idx].m_numPrims = static_cast<uint32_t>(leafPrims.size());

      detail::appendTreeLeaf(prims, leafPrims);
    }
    else {
      nodes[idx].m_numPrims = 0U;
      nodes[idx].m_primOff  = 0U;

      const auto& children = node.getChildren();

      for (size_t k = 0; k < K; k++) {
        EBGEOMETRY_REQUIRE(children[k] != nullptr,
                           "BVH::PackedBVH: the tree has a node with neither primitives nor children; partition "
                           "it before packing, and never into an empty partition");

        nodes[idx].m_childOff[k] = dfs(*children[k]);
      }
    }
    return idx;
  };

  dfs(a_tree);
  this->finalize(a_pool, nodes, prims);
}

template <class T, class P, size_t K>
template <class Q, class Converter>
inline PackedBVH<T, P, K>::PackedBVH(Pool&                                                          a_pool,
                                     const TreeBVH<T, Q, EBGeometry::BoundingVolumes::AABBT<T>, K>& a_tree,
                                     Converter&&                                                    a_converter)
{
  std::vector<Node> nodes;
  std::vector<P>    prims;

  using AABBType = EBGeometry::BoundingVolumes::AABBT<T>;

  // Accumulate converted P-values into a single contiguous buffer; appendAliased materialises it
  // into prims once every push_back below has completed -- a move of the whole buffer, so no
  // element is copied twice.
  auto dstStorage = std::make_shared<std::vector<P>>();

  std::function<uint32_t(const TreeBVH<T, Q, AABBType, K>&)> dfs =
    [&](const TreeBVH<T, Q, AABBType, K>& node) -> uint32_t {
    const uint32_t idx = static_cast<uint32_t>(nodes.size());

    nodes.push_back({});
    nodes[idx].m_bv = node.getBoundingVolume();

    if (node.isLeaf()) {
      const auto&    leafPrims = node.getPrimitives();
      const uint32_t dstOff    = static_cast<uint32_t>(dstStorage->size());

      auto newVals = a_converter(leafPrims, 0U, static_cast<uint32_t>(leafPrims.size()));

      nodes[idx].m_primOff  = dstOff;
      nodes[idx].m_numPrims = static_cast<uint32_t>(newVals.size());

      for (auto&& v : newVals) {
        dstStorage->push_back(std::move(v));
      }
    }
    else {
      nodes[idx].m_numPrims = 0U;
      nodes[idx].m_primOff  = 0U;

      const auto& children = node.getChildren();

      for (size_t k = 0; k < K; k++) {
        EBGEOMETRY_REQUIRE(children[k] != nullptr,
                           "BVH::PackedBVH: the tree has a node with neither primitives nor children; partition "
                           "it before packing, and never into an empty partition");

        nodes[idx].m_childOff[k] = dfs(*children[k]);
      }
    }

    return idx;
  };

  dfs(a_tree);

  detail::appendAliased(prims, dstStorage);

  this->finalize(a_pool, nodes, prims);
}

template <class T, class P, size_t K>
template <class S>
inline PackedBVH<T, P, K>::PackedBVH(Pool&                         a_pool,
                                     std::vector<std::pair<P, BV>> a_primsAndBVs,
                                     size_t                        a_targetLeafSize,
                                     S)
{
  EBGEOMETRY_REQUIRE(!a_primsAndBVs.empty(), "PackedBVH: the SFC build needs at least one primitive");
  EBGEOMETRY_REQUIRE(
    a_targetLeafSize > 0, "PackedBVH: the SFC build's target leaf size must be positive (%zu)", a_targetLeafSize);

  std::vector<Node> nodes;
  std::vector<P>    prims;

  const size_t numPrimitives = a_primsAndBVs.size();

  // Sort primitives along the space-filling curve, exactly like
  // TreeBVH::bottomUpSortAndPartition() (same binning helper), but keeping P by value throughout
  // -- no shared_ptr anywhere in this constructor.
  std::vector<Vec3T<T>> centroids;
  centroids.reserve(numPrimitives);
  for (const auto& pbv : a_primsAndBVs) {
    centroids.push_back(pbv.second.getCentroid());
  }

  const std::vector<SFC::Index> bins = SFC::computeBins<T>(centroids);

  using PrimBvAndCode = std::tuple<P, BV, SFC::Code>;

  std::vector<PrimBvAndCode> sortedPrimitives;

  sortedPrimitives.reserve(numPrimitives);

  for (size_t i = 0; i < numPrimitives; i++) {
    sortedPrimitives.emplace_back(std::move(a_primsAndBVs[i].first), a_primsAndBVs[i].second, S::encode(bins[i]));
  }
  a_primsAndBVs.clear();

  std::sort(sortedPrimitives.begin(),
            sortedPrimitives.end(),
            [](const PrimBvAndCode& a_lhs, const PrimBvAndCode& a_rhs) noexcept -> bool {
              return std::get<2>(a_lhs) < std::get<2>(a_rhs);
            });

  // Choose the leaf count. Every interior node has exactly K children, and a tree like that has
  // L = 1 (mod K - 1) leaves. Take the smallest such L that keeps leaves at or below the target
  // size; if that would leave some leaf empty (L > N), take the largest such L below it instead.
  // The primitives are then split as evenly as possible, so leaf sizes differ by at most one.
  const size_t minLeaves = (numPrimitives + a_targetLeafSize - 1) / a_targetLeafSize;

  size_t numLeaves = 1 + ((minLeaves - 1 + (K - 2)) / (K - 1)) * (K - 1);

  if (numLeaves > numPrimitives) {
    numLeaves = 1 + ((minLeaves - 1) / (K - 1)) * (K - 1);
  }

  struct LeafRange
  {
    uint32_t offset;
    uint32_t count;
    BV       bv;
  };

  std::vector<LeafRange> leafRanges;
  leafRanges.reserve(numLeaves);

  for (size_t leaf = 0, i = 0; leaf < numLeaves; leaf++) {
    const size_t count = numPrimitives / numLeaves + (leaf < numPrimitives % numLeaves ? 1 : 0);

    BV leafBV;

    for (size_t j = 0; j < count; j++) {
      leafBV = leafBV.merged(std::get<1>(sortedPrimitives[i + j]));
    }

    leafRanges.push_back({static_cast<uint32_t>(i), static_cast<uint32_t>(count), leafBV});

    i += count;
  }

  // Populate the primitive array once, in final sorted order -- independent of whatever order
  // the node array below ends up being built/relaid-out in.
  auto primBlock = std::make_shared<std::vector<P>>();

  primBlock->reserve(numPrimitives);

  for (auto& entry : sortedPrimitives) {
    primBlock->push_back(std::move(std::get<0>(entry)));
  }

  detail::appendAliased(prims, primBlock);

  // Build the K-ary structure bottom-up in a scratch array (reusing Node's own shape), then relay
  // it out into nodes in depth-first pre-order below -- a bottom-up merge naturally
  // produces the root last, but PackedBVH's traversal assumes the root is always at index 0.
  //
  // Each level merges consecutive groups of K nodes. When a level's node count is not a multiple of
  // K, the last (count mod K) nodes are carried up to the next level unmerged, keeping curve order.
  // Since the leaf count is 1 (mod K - 1), every level's count is too, and the merge ends at exactly
  // one root. Every node is referenced by exactly one parent, so a traversal reaches each primitive
  // once. A full K-ary tree with L leaves has L + (L - 1)/(K - 1) nodes.
  std::vector<Node> scratch(numLeaves);
  scratch.reserve(numLeaves + (numLeaves - 1) / (K - 1));

  for (size_t i = 0; i < numLeaves; i++) {
    scratch[i].setBoundingVolume(leafRanges[i].bv);
    scratch[i].setPrimitivesOffset(leafRanges[i].offset);
    scratch[i].setNumPrimitives(leafRanges[i].count);
  }

  std::vector<uint32_t> levelIndices(numLeaves);

  for (size_t i = 0; i < numLeaves; i++) {
    levelIndices[i] = static_cast<uint32_t>(i);
  }

  while (levelIndices.size() > 1) {
    const size_t numParents = levelIndices.size() / K;
    const size_t numCarried = levelIndices.size() % K;

    std::vector<uint32_t> nextLevel;
    nextLevel.reserve(numParents + numCarried);

    for (size_t p = 0; p < numParents; p++) {
      const uint32_t parentIdx = static_cast<uint32_t>(scratch.size());
      scratch.emplace_back();

      BV parentBV;

      for (size_t c = 0; c < K; c++) {
        const uint32_t childIdx = levelIndices[p * K + c];
        scratch[parentIdx].setChildOffset(childIdx, c);
        parentBV = parentBV.merged(scratch[childIdx].getBoundingVolume());
      }

      scratch[parentIdx].setBoundingVolume(parentBV);

      nextLevel.push_back(parentIdx);
    }

    for (size_t c = 0; c < numCarried; c++) {
      nextLevel.push_back(levelIndices[numParents * K + c]);
    }

    levelIndices = std::move(nextLevel);
  }

  const uint32_t scratchRoot = levelIndices.front();

  // Relay the scratch tree into nodes in depth-first pre-order (root at index 0), exactly
  // matching the invariant the other two constructors establish.
  nodes.reserve(scratch.size());

  std::function<uint32_t(uint32_t)> relayout = [&](uint32_t a_scratchIdx) -> uint32_t {
    const uint32_t newIdx = static_cast<uint32_t>(nodes.size());

    nodes.push_back(scratch[a_scratchIdx]);

    if (!scratch[a_scratchIdx].isLeaf()) {
      for (size_t c = 0; c < K; c++) {
        const uint32_t newChildIdx = relayout(scratch[a_scratchIdx].getChildOffsets()[c]);

        nodes[newIdx].setChildOffset(static_cast<uint32_t>(newChildIdx), c);
      }
    }

    return newIdx;
  };

  relayout(scratchRoot);

  this->finalize(a_pool, nodes, prims);
}

template <class T, class P, size_t K>
inline PackedBVH<T, P, K>::PackedBVH(Pool&                             a_pool,
                                     std::vector<std::pair<P, BV>>     a_primsAndBVs,
                                     const BVH::Partitioner<P, BV, K>& a_partitioner)
  : PackedBVH(a_pool, std::move(a_primsAndBVs), a_partitioner, DefaultLeafPredicate<T, P, BV, K>)
{}

template <class T, class P, size_t K>
inline PackedBVH<T, P, K>::PackedBVH(Pool& a_pool, std::vector<std::pair<P, BV>> a_primsAndBVs)
  : PackedBVH(a_pool, std::move(a_primsAndBVs), BVCentroidPartitioner<T, P, BV, K>, DefaultLeafPredicate<T, P, BV, K>)
{}

template <class T, class P, size_t K>
inline PackedBVH<T, P, K>::PackedBVH(Pool&                                  a_pool,
                                     std::vector<std::pair<P, BV>>          a_primsAndBVs,
                                     const BVH::Partitioner<P, BV, K>&      a_partitioner,
                                     const BVH::LeafPredicate<T, P, BV, K>& a_stopCrit)
{
  EBGEOMETRY_REQUIRE(!a_primsAndBVs.empty(), "PackedBVH: the top-down build needs at least one primitive");

  std::vector<Node> nodes;
  std::vector<P>    prims;

  // shared_ptr-wrap each primitive once, up front, so the existing Partitioner/LeafPredicate
  // machinery (which operates on PrimAndBVList) can be reused unchanged. This is not the cost
  // this constructor targets -- see its doxygen comment -- only the persistent, per-node
  // shared_ptr<TreeBVH> allocation is avoided.
  BVH::PrimAndBVList<P, BV> wrapped;

  wrapped.reserve(a_primsAndBVs.size());

  for (auto& pbv : a_primsAndBVs) {
    wrapped.emplace_back(std::make_shared<P>(std::move(pbv.first)), pbv.second);
  }

  a_primsAndBVs.clear();

  // Recursively partition top-down, writing nodes directly into nodes in pre-order (root
  // first) as the recursion unwinds -- unlike the SFC-build constructor above, top-down
  // partitioning visits the root before its children, so no relayout pass is needed here. At every
  // split, a lightweight, stack-local TreeBVH ("probe") is constructed purely to evaluate
  // a_stopCrit (whose signature expects an actual TreeBVH node, matching
  // TreeBVH::topDownSortAndPartition()'s own contract) and to read off its primitive list; it is
  // discarded immediately afterward, never linked into a persistent tree.
  std::function<uint32_t(BVH::PrimAndBVList<P, BV>)> build = [&](BVH::PrimAndBVList<P, BV> a_prims) -> uint32_t {
    const uint32_t idx = static_cast<uint32_t>(nodes.size());

    nodes.push_back({});

    const BVH::TreeBVH<T, P, BV, K> probe(a_prims);

    nodes[idx].setBoundingVolume(probe.getBoundingVolume());

    if (a_stopCrit(probe) || a_prims.size() < K) {
      const auto& leafPrims = probe.getPrimitives();

      nodes[idx].setPrimitivesOffset(static_cast<uint32_t>(prims.size()));
      nodes[idx].setNumPrimitives(static_cast<uint32_t>(leafPrims.size()));

      detail::appendTreeLeaf(prims, leafPrims);
    }
    else {
      // The partitioner takes its list by value and moves the sub-lists out; a_prims is not used
      // after this, so move it in, and move each child sub-list into the recursion.
      const size_t numInput = a_prims.size();

      Array<BVH::PrimAndBVList<P, BV>, K> children = a_partitioner(std::move(a_prims));

      detail::requireNonEmptyPartitions(children, numInput, "BVH::PackedBVH");

      for (size_t k = 0; k < K; k++) {
        const uint32_t childIdx = build(std::move(children[k]));
        nodes[idx].setChildOffset(childIdx, k);
      }
    }

    return idx;
  };

  build(std::move(wrapped));

  this->finalize(a_pool, nodes, prims);
}

template <class T, class P, size_t K>
inline PackedBVH<T, P, K>::PackedBVH(Pool& a_pool, std::vector<std::pair<P, BV>> a_primsAndBVs, BVH::ClusterSpec a_spec)
{
  static_assert(std::is_same_v<BV, EBGeometry::BoundingVolumes::AABBT<T>>, "ClusterSAH requires BV == AABBT<T>");

  EBGEOMETRY_REQUIRE(!a_primsAndBVs.empty(), "PackedBVH: the ClusterSAH build needs at least one primitive");
  EBGEOMETRY_REQUIRE(
    a_spec.maxClusterSize > 0, "PackedBVH: ClusterSpec::maxClusterSize must be positive (%zu)", a_spec.maxClusterSize);

  std::vector<Node> nodes;
  std::vector<P>    prims;

  const size_t maxC = a_spec.maxClusterSize;

  // A cluster: its bounding volume, centroid, and the primitives it owns (moved in).
  struct Cluster
  {
    BV                            bv;
    Vec3T<T>                      centroid;
    std::vector<std::pair<P, BV>> prims;
  };

  // ---- Phase 1: density-adaptive clustering --------------------------------------------------
  // Recursively split a_primsAndBVs[begin, end) at the midpoint of its centroid bounding box (longest
  // axis) until a range holds at most maxC primitives, then freeze it as a cluster. The midpoint
  // splits are cheap, the subdivision is shallow, and it follows the primitive density (fine where
  // dense, coarse where sparse), so no cluster overcrowds -- robust where a fixed grid would fail.
  std::vector<Cluster> clusters;
  clusters.reserve(a_primsAndBVs.size() / maxC + 1);

  std::function<void(size_t, size_t)> makeClusters = [&](size_t a_begin, size_t a_end) -> void {
    if (a_end - a_begin <= maxC) {
      Cluster cluster;
      cluster.prims.reserve(a_end - a_begin);

      Vec3T<T> lo = +Vec3T<T>::max();
      Vec3T<T> hi = -Vec3T<T>::max();
      for (size_t i = a_begin; i < a_end; i++) {
        lo = min(lo, a_primsAndBVs[i].second.getLowCorner());
        hi = max(hi, a_primsAndBVs[i].second.getHighCorner());
        cluster.prims.push_back(std::move(a_primsAndBVs[i]));
      }
      cluster.bv       = BV(lo, hi);
      cluster.centroid = cluster.bv.getCentroid();

      clusters.push_back(std::move(cluster));

      return;
    }

    Vec3T<T> clo = +Vec3T<T>::max();
    Vec3T<T> chi = -Vec3T<T>::max();
    for (size_t i = a_begin; i < a_end; i++) {
      const auto& c = a_primsAndBVs[i].second.getCentroid();
      clo           = min(clo, c);
      chi           = max(chi, c);
    }

    const size_t axis = (chi - clo).maxDir(true);
    const T      mid  = T(0.5) * (clo[axis] + chi[axis]);

    const auto pivot = std::partition(
      a_primsAndBVs.begin() + a_begin,
      a_primsAndBVs.begin() + a_end,
      [axis, mid](const std::pair<P, BV>& a_pb) noexcept { return a_pb.second.getCentroid()[axis] < mid; });

    size_t split = static_cast<size_t>(pivot - a_primsAndBVs.begin());
    if (split == a_begin || split == a_end) {
      split = a_begin + (a_end - a_begin) / 2; // all centroids on one side: fall back to equal counts
    }

    makeClusters(a_begin, split);
    makeClusters(split, a_end);
  };
  makeClusters(0, a_primsAndBVs.size());
  a_primsAndBVs.clear();

  // ---- Phase 2: binned SAH over the clusters -------------------------------------------------
  // A self-contained 2-way binned SAH over clusters[begin, end) -- mirrors SAH2WaySplit but over
  // cluster boxes, deliberately isolated so the primitive SAH path is untouched. Partitions the
  // clusters in place and returns the split index.
  constexpr int BINS = 32;

  const auto sah2Way = [&clusters](size_t a_begin, size_t a_end) noexcept -> size_t {
    const size_t nClusters = a_end - a_begin;

    Vec3T<T> clo = +Vec3T<T>::max();
    Vec3T<T> chi = -Vec3T<T>::max();
    for (size_t i = a_begin; i < a_end; i++) {
      clo = min(clo, clusters[i].centroid);
      chi = max(chi, clusters[i].centroid);
    }

    T   bestCost  = Math::Limits<T>::max();
    T   bestPlane = T(0);
    int bestAxis  = -1;

    Vec3T<T> binLo[BINS];
    Vec3T<T> binHi[BINS];
    int      binCnt[BINS];
    T        leftArea[BINS - 1];
    int      leftCnt[BINS - 1];

    for (int axis = 0; axis < 3; axis++) {
      const T lo  = clo[axis];
      const T hi  = chi[axis];
      const T ext = hi - lo;

      // An extent so small that BINS / ext overflows cannot be binned; leave that axis out.
      const T scale = T(BINS) / ext;

      if (!(ext > T(0)) || !std::isfinite(scale)) {
        continue;
      }

      for (int b = 0; b < BINS; b++) {
        binLo[b]  = Vec3T<T>::max();
        binHi[b]  = -Vec3T<T>::max();
        binCnt[b] = 0;
      }
      for (size_t i = a_begin; i < a_end; i++) {
        // Clamped in floating point before the conversion, which is undefined past int's range.
        const T   x = (clusters[i].centroid[axis] - lo) * scale;
        const int b = (x > T(0)) ? ((x < T(BINS - 1)) ? static_cast<int>(x) : BINS - 1) : 0;
        binLo[b]    = min(binLo[b], clusters[i].bv.getLowCorner());
        binHi[b]    = max(binHi[b], clusters[i].bv.getHighCorner());
        binCnt[b]   = binCnt[b] + 1;
      }

      Vec3T<T> rlo = +Vec3T<T>::max();
      Vec3T<T> rhi = -Vec3T<T>::max();

      int rcnt = 0;

      for (int b = 0; b < BINS - 1; b++) {
        if (binCnt[b] > 0) {
          rlo = min(rlo, binLo[b]);
          rhi = max(rhi, binHi[b]);
        }
        rcnt        = rcnt + binCnt[b];
        leftArea[b] = (rcnt > 0) ? BV(rlo, rhi).getArea() : T(0);
        leftCnt[b]  = rcnt;
      }

      Vec3T<T> rrlo = +Vec3T<T>::max();
      Vec3T<T> rrhi = -Vec3T<T>::max();

      int rrcnt = 0;

      for (int b = BINS - 1; b >= 1; b--) {
        if (binCnt[b] > 0) {
          rrlo = min(rrlo, binLo[b]);
          rrhi = max(rrhi, binHi[b]);
        }

        rrcnt += binCnt[b];

        if (leftCnt[b - 1] > 0 && rrcnt > 0) {
          const T cost = leftArea[b - 1] * T(leftCnt[b - 1]) + BV(rrlo, rrhi).getArea() * T(rrcnt);

          if (cost < bestCost) {
            bestCost  = cost;
            bestAxis  = axis;
            bestPlane = lo + static_cast<T>(b) / scale;
          }
        }
      }
    }

    if (bestAxis < 0) {
      return a_begin + nClusters / 2; // degenerate (all cluster centroids coincide): equal-count split
    }

    const auto pivot =
      std::partition(clusters.begin() + a_begin,
                     clusters.begin() + a_end,
                     [bestAxis, bestPlane](const Cluster& a_c) noexcept { return a_c.centroid[bestAxis] < bestPlane; });
    size_t split = static_cast<size_t>(pivot - clusters.begin());
    if (split == a_begin || split == a_end) {
      split = a_begin + nClusters / 2;
    }
    return split;
  };

  // K-way SAH split: recursively bisect into floor(K/2)/ceil(K/2), mirroring SAHKWaySplit.
  std::function<void(size_t, size_t, size_t, std::vector<std::pair<size_t, size_t>>&)> sahKWay =
    [&](size_t a_begin, size_t a_end, size_t a_K, std::vector<std::pair<size_t, size_t>>& a_groups) -> void {
    if (a_K <= 1 || a_begin >= a_end) {
      a_groups.emplace_back(a_begin, a_end);
      return;
    }

    const size_t K1 = a_K / 2;
    const size_t K2 = a_K - K1;

    const size_t raw = sah2Way(a_begin, a_end);
    const size_t mid = Math::max(a_begin + K1, Math::min(a_end - K2, raw));

    sahKWay(a_begin, mid, K1, a_groups);
    sahKWay(mid, a_end, K2, a_groups);
  };

  // Build the flat node array top-down (pre-order), populating the primitive block in DFS-leaf order
  // so each leaf's primitives are a contiguous [offset, count) range.
  auto primBlock = std::make_shared<std::vector<P>>();

  size_t total = 0;

  for (const auto& c : clusters) {
    total += c.prims.size();
  }

  primBlock->reserve(total);

  std::function<uint32_t(size_t, size_t)> build = [&](size_t a_begin, size_t a_end) -> uint32_t {
    const uint32_t idx = static_cast<uint32_t>(nodes.size());
    nodes.push_back({});

    Vec3T<T> nlo = +Vec3T<T>::max();
    Vec3T<T> nhi = -Vec3T<T>::max();

    for (size_t i = a_begin; i < a_end; i++) {
      nlo = min(nlo, clusters[i].bv.getLowCorner());
      nhi = max(nhi, clusters[i].bv.getHighCorner());
    }

    nodes[idx].setBoundingVolume(BV(nlo, nhi));

    if (a_end - a_begin < K) {
      // Leaf: fewer than K clusters -- append their primitives to the block, contiguously.
      nodes[idx].setPrimitivesOffset(static_cast<uint32_t>(primBlock->size()));

      uint32_t count = 0;

      for (size_t i = a_begin; i < a_end; i++) {
        for (auto& pb : clusters[i].prims) {
          primBlock->push_back(std::move(pb.first));
          count++;
        }
      }
      nodes[idx].setNumPrimitives(count);
    }
    else {
      std::vector<std::pair<size_t, size_t>> groups;
      groups.reserve(K);
      sahKWay(a_begin, a_end, K, groups);

      for (size_t k = 0; k < K; k++) {
        const uint32_t childIdx = build(groups[k].first, groups[k].second);
        nodes[idx].setChildOffset(childIdx, k);
      }
    }

    return idx;
  };
  build(0, clusters.size());

  detail::appendAliased(prims, primBlock);

  this->finalize(a_pool, nodes, prims);
}

} // namespace BVH

} // namespace EBGeometry

#endif
