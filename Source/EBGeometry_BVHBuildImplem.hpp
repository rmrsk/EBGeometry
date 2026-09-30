// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
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
#include <numeric>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_BVHBuild.hpp"
#include "EBGeometry_SFC.hpp"

namespace EBGeometry {

namespace BVH {

/**
 * @brief Internal pieces of the BVH builder; not part of the public API.
 */
namespace BuildDetail {

/**
 * @brief Top-down builder over a set of items, each with a box, a centroid and a weight.
 * @details The items are primitives for SAH, Centroid and Midpoint builds, and clusters of
 * primitives for the second phase of a ClusterSAH build. A range of items becomes a leaf once its
 * total weight is at most the leaf size, so a leaf never holds more than that many primitives.
 * @tparam T Floating-point precision.
 * @tparam K Branching factor.
 */
template <class T, size_t K>
struct TopDownBuilder
{
  using AABB = BoundingVolumes::AABBT<T>;
  using Node = WideNode<T, K>;

  /**
   * @brief Number of SAH bins per axis.
   */
  static constexpr int Bins = 32;

  /**
   * @brief Item boxes.
   */
  const std::vector<AABB>& boxes;

  /**
   * @brief Item centroids.
   */
  const std::vector<Vec3T<T>>& centroids;

  /**
   * @brief Item weights (primitives per item); empty means one each.
   */
  const std::vector<uint32_t>& weights;

  /**
   * @brief Split strategy: SAH, Centroid or Midpoint.
   */
  Strategy strategy;

  /**
   * @brief Maximum weight of a leaf.
   */
  uint64_t maxLeafWeight;

  /**
   * @brief Item permutation; leaf ranges index it.
   */
  std::vector<uint32_t> order;

  /**
   * @brief Nodes built so far.
   */
  std::vector<Node> nodes;

  /**
   * @brief Total weight of order[a_begin, a_end).
   */
  [[nodiscard]] uint64_t
  weight(const size_t a_begin, const size_t a_end) const noexcept
  {
    if (weights.empty()) {
      return a_end - a_begin;
    }

    uint64_t w = 0;

    for (size_t i = a_begin; i < a_end; i++) {
      w += weights[order[i]];
    }

    return w;
  }

  /**
   * @brief Union of the boxes of order[a_begin, a_end).
   */
  [[nodiscard]] AABB
  box(const size_t a_begin, const size_t a_end) const noexcept
  {
    AABB bv;

    for (size_t i = a_begin; i < a_end; i++) {
      bv = bv.merged(boxes[order[i]]);
    }

    return bv;
  }

  /**
   * @brief Longest axis of the centroid bounds of order[a_begin, a_end), and those bounds.
   */
  [[nodiscard]] size_t
  centroidBounds(const size_t a_begin, const size_t a_end, Vec3T<T>& a_lo, Vec3T<T>& a_hi) const noexcept
  {
    a_lo = +Vec3T<T>::infinity();
    a_hi = -Vec3T<T>::infinity();

    for (size_t i = a_begin; i < a_end; i++) {
      a_lo = min(a_lo, centroids[order[i]]);
      a_hi = max(a_hi, centroids[order[i]]);
    }

    return static_cast<size_t>((a_hi - a_lo).maxDir(true));
  }

  /**
   * @brief Put the item of rank a_mid - a_begin along a_axis at a_mid, smaller ones before it.
   */
  void
  splitByCount(const size_t a_begin, const size_t a_mid, const size_t a_end, const size_t a_axis) noexcept
  {
    std::nth_element(order.begin() + static_cast<std::ptrdiff_t>(a_begin),
                     order.begin() + static_cast<std::ptrdiff_t>(a_mid),
                     order.begin() + static_cast<std::ptrdiff_t>(a_end),
                     [this, a_axis](const uint32_t a_lhs, const uint32_t a_rhs) noexcept {
                       return centroids[a_lhs][a_axis] < centroids[a_rhs][a_axis];
                     });
  }

  /**
   * @brief Split order[a_begin, a_end) in two, for a_leftParts and a_rightParts groups.
   * @details Each side keeps at least as many items as it has groups to fill, when there are
   * enough items, so that coincident or clustered centroids cannot make the tree deep: a strategy
   * whose split leaves one side too small falls back to an equal-count split.
   * @return The split index, strictly inside the range.
   */
  [[nodiscard]] size_t
  split2(const size_t a_begin, const size_t a_end, const size_t a_leftParts, const size_t a_rightParts) noexcept
  {
    const size_t count = a_end - a_begin;
    const size_t parts = a_leftParts + a_rightParts;

    EBGEOMETRY_EXPECT(count >= 2);

    const size_t lowest  = a_begin + Math::min(a_leftParts, count / 2);
    const size_t highest = a_end - Math::min(a_rightParts, count - count / 2);

    Vec3T<T>     lo;
    Vec3T<T>     hi;
    const size_t axis = this->centroidBounds(a_begin, a_end, lo, hi);

    // The equal-count split: proportional to the number of groups on each side.
    const auto equalCount = [&](const size_t a_axis) noexcept -> size_t {
      const size_t mid =
        Math::clamp(a_begin + static_cast<size_t>(uint64_t(count) * a_leftParts / parts), lowest, highest);

      this->splitByCount(a_begin, mid, a_end, a_axis);

      return mid;
    };

    switch (strategy) {
    case Strategy::Midpoint: {
      if (hi[axis] > lo[axis]) {
        const T midpoint = T(0.5) * (lo[axis] + hi[axis]);

        const auto pivot = std::partition(
          order.begin() + static_cast<std::ptrdiff_t>(a_begin),
          order.begin() + static_cast<std::ptrdiff_t>(a_end),
          [this, axis, midpoint](const uint32_t a_item) noexcept { return centroids[a_item][axis] < midpoint; });

        const size_t mid = static_cast<size_t>(pivot - order.begin());

        if (mid >= lowest && mid <= highest) {
          return mid;
        }
      }

      return equalCount(axis);
    }
    case Strategy::SAH: {
      return this->sahSplit(a_begin, a_end, lowest, highest, lo, hi, axis, equalCount);
    }
    case Strategy::Centroid:
    default: {
      return equalCount(axis);
    }
    }
  }

  /**
   * @brief Binned SAH split of order[a_begin, a_end), over all three axes.
   * @details Falls back to an equal-count split when every centroid coincides, and clamps the
   * split into [a_lowest, a_highest] by rank along the chosen axis.
   */
  template <class EqualCount>
  [[nodiscard]] size_t
  sahSplit(const size_t      a_begin,
           const size_t      a_end,
           const size_t      a_lowest,
           const size_t      a_highest,
           const Vec3T<T>&   a_lo,
           const Vec3T<T>&   a_hi,
           const size_t      a_longestAxis,
           const EqualCount& a_equalCount) noexcept
  {
    T   bestCost  = Math::Limits<T>::infinity();
    int bestAxis  = -1;
    int bestSplit = 0;

    AABB     binBox[Bins];
    uint64_t binWeight[Bins];
    T        leftArea[Bins];
    uint64_t leftWeight[Bins];

    for (int axis = 0; axis < 3; axis++) {
      const T extent = a_hi[axis] - a_lo[axis];

      // An extent so small that Bins / extent overflows cannot be binned; leave that axis out.
      const T scale = T(Bins) / extent;

      if (!(extent > T(0)) || !std::isfinite(scale)) {
        continue;
      }

      for (int b = 0; b < Bins; b++) {
        binBox[b]    = AABB();
        binWeight[b] = 0;
      }

      for (size_t i = a_begin; i < a_end; i++) {
        const uint32_t item = order[i];
        const int      b    = this->binOf(centroids[item][axis], a_lo[axis], scale);

        binBox[b]    = binBox[b].merged(boxes[item]);
        binWeight[b] = binWeight[b] + (weights.empty() ? 1 : weights[item]);
      }

      AABB     sweep;
      uint64_t sweepWeight = 0;

      for (int b = 0; b < Bins - 1; b++) {
        sweep       = sweep.merged(binBox[b]);
        sweepWeight = sweepWeight + binWeight[b];

        leftArea[b]   = (sweepWeight > 0) ? sweep.getArea() : T(0);
        leftWeight[b] = sweepWeight;
      }

      sweep       = AABB();
      sweepWeight = 0;

      for (int b = Bins - 1; b >= 1; b--) {
        sweep       = sweep.merged(binBox[b]);
        sweepWeight = sweepWeight + binWeight[b];

        if (leftWeight[b - 1] > 0 && sweepWeight > 0) {
          const T cost = leftArea[b - 1] * T(leftWeight[b - 1]) + sweep.getArea() * T(sweepWeight);

          if (cost < bestCost) {
            bestCost  = cost;
            bestAxis  = axis;
            bestSplit = b;
          }
        }
      }
    }

    if (bestAxis < 0) {
      return a_equalCount(a_longestAxis);
    }

    const T   lo    = a_lo[bestAxis];
    const T   scale = T(Bins) / (a_hi[bestAxis] - lo);
    const int axis  = bestAxis;
    const int split = bestSplit;

    const auto pivot = std::partition(order.begin() + static_cast<std::ptrdiff_t>(a_begin),
                                      order.begin() + static_cast<std::ptrdiff_t>(a_end),
                                      [this, axis, lo, scale, split](const uint32_t a_item) noexcept {
                                        return this->binOf(centroids[a_item][axis], lo, scale) < split;
                                      });

    const size_t mid = static_cast<size_t>(pivot - order.begin());

    if (mid >= a_lowest && mid <= a_highest) {
      return mid;
    }

    const size_t clamped = Math::clamp(mid, a_lowest, a_highest);

    this->splitByCount(a_begin, clamped, a_end, static_cast<size_t>(axis));

    return clamped;
  }

  /**
   * @brief SAH bin of a centroid coordinate.
   */
  [[nodiscard]] static int
  binOf(const T a_x, const T a_lo, const T a_scale) noexcept
  {
    // Clamped in floating point before the conversion, which is undefined for a value past int's range.
    const T b = (a_x - a_lo) * a_scale;

    return (b > T(0)) ? ((b < T(Bins - 1)) ? static_cast<int>(b) : Bins - 1) : 0;
  }

  /**
   * @brief Split order[a_begin, a_end) into at most a_parts groups by recursive bisection.
   * @details A range stops splitting once it holds a single item or fits in one leaf.
   */
  void
  split(const size_t                            a_begin,
        const size_t                            a_end,
        const size_t                            a_parts,
        std::vector<std::pair<size_t, size_t>>& a_groups)
  {
    if (a_parts <= 1 || a_end - a_begin <= 1 || this->weight(a_begin, a_end) <= maxLeafWeight) {
      a_groups.emplace_back(a_begin, a_end);

      return;
    }

    const size_t leftParts  = a_parts / 2;
    const size_t rightParts = a_parts - leftParts;
    const size_t mid        = this->split2(a_begin, a_end, leftParts, rightParts);

    EBGEOMETRY_EXPECT(mid > a_begin && mid < a_end);

    this->split(a_begin, mid, leftParts, a_groups);
    this->split(mid, a_end, rightParts, a_groups);
  }

  /**
   * @brief Build the tree over every item.
   */
  void
  run()
  {
    const size_t numItems = boxes.size();

    order.resize(numItems);
    std::iota(order.begin(), order.end(), uint32_t(0));

    nodes.clear();

    if (numItems == 0) {
      return;
    }

    nodes.push_back(Node::empty());

    if (this->weight(0, numItems) <= maxLeafWeight) {
      nodes[0].setLeaf(0, this->box(0, numItems), 0, static_cast<uint32_t>(numItems));

      return;
    }

    struct Work
    {
      size_t   begin;
      size_t   end;
      uint32_t node;
    };

    std::vector<Work>                      work{Work{0, numItems, 0}};
    std::vector<std::pair<size_t, size_t>> groups;

    while (!work.empty()) {
      const Work w = work.back();

      work.pop_back();

      groups.clear();

      this->split(w.begin, w.end, K, groups);

      EBGEOMETRY_EXPECT(groups.size() >= 2 && groups.size() <= K);

      for (size_t k = 0; k < groups.size(); k++) {
        const size_t begin = groups[k].first;
        const size_t end   = groups[k].second;
        const AABB   bv    = this->box(begin, end);

        if (end - begin == 1 || this->weight(begin, end) <= maxLeafWeight) {
          nodes[w.node].setLeaf(k, bv, static_cast<uint32_t>(begin), static_cast<uint32_t>(end - begin));
        }
        else {
          const uint32_t child = static_cast<uint32_t>(nodes.size());

          nodes.push_back(Node::empty());
          nodes[w.node].setInterior(k, bv, child);

          work.push_back(Work{begin, end, child});
        }
      }
    }
  }
};

/**
 * @brief Centroid of each box.
 */
template <class T>
[[nodiscard]] std::vector<Vec3T<T>>
centroidsOf(const std::vector<BoundingVolumes::AABBT<T>>& a_boxes)
{
  std::vector<Vec3T<T>> centroids;

  centroids.reserve(a_boxes.size());

  for (const auto& box : a_boxes) {
    centroids.push_back(box.getCentroid());
  }

  return centroids;
}

/**
 * @brief SAH, Centroid or Midpoint build.
 */
template <class T, size_t K>
[[nodiscard]] Topology<T, K>
buildTopDown(const std::vector<BoundingVolumes::AABBT<T>>& a_boxes, const Strategy a_strategy, const uint32_t a_maxLeaf)
{
  const std::vector<Vec3T<T>> centroids = centroidsOf(a_boxes);
  const std::vector<uint32_t> noWeights;

  TopDownBuilder<T, K> builder{a_boxes, centroids, noWeights, a_strategy, a_maxLeaf, {}, {}};

  builder.run();

  return Topology<T, K>{std::move(builder.nodes), std::move(builder.order)};
}

/**
 * @brief ClusterSAH build: leaf-sized clusters by midpoint subdivision, then binned SAH over them.
 * @details The first phase is cheap and follows the primitive density. SAH then partitions one box
 * per cluster instead of one per primitive, which is where the speedup over a plain SAH build
 * comes from. A leaf holds whole clusters, at most a_maxLeaf primitives in total.
 */
template <class T, size_t K>
[[nodiscard]] Topology<T, K>
buildClusterSAH(const std::vector<BoundingVolumes::AABBT<T>>& a_boxes, const uint32_t a_maxLeaf)
{
  using AABB = BoundingVolumes::AABBT<T>;

  const size_t                numItems  = a_boxes.size();
  const std::vector<Vec3T<T>> centroids = centroidsOf(a_boxes);

  // Phase 1: clusters of at most a_maxLeaf primitives, as contiguous ranges of itemOrder.
  std::vector<uint32_t> itemOrder(numItems);

  std::iota(itemOrder.begin(), itemOrder.end(), uint32_t(0));

  std::vector<std::pair<size_t, size_t>> clusterRanges;
  std::vector<std::pair<size_t, size_t>> pending;

  if (numItems > 0) {
    pending.emplace_back(0, numItems);
  }

  while (!pending.empty()) {
    const size_t begin = pending.back().first;
    const size_t end   = pending.back().second;

    pending.pop_back();

    if (end - begin <= a_maxLeaf) {
      clusterRanges.emplace_back(begin, end);

      continue;
    }

    Vec3T<T> lo = +Vec3T<T>::infinity();
    Vec3T<T> hi = -Vec3T<T>::infinity();

    for (size_t i = begin; i < end; i++) {
      lo = min(lo, centroids[itemOrder[i]]);
      hi = max(hi, centroids[itemOrder[i]]);
    }

    const size_t axis     = static_cast<size_t>((hi - lo).maxDir(true));
    const T      midpoint = T(0.5) * (lo[axis] + hi[axis]);

    auto first = itemOrder.begin() + static_cast<std::ptrdiff_t>(begin);
    auto last  = itemOrder.begin() + static_cast<std::ptrdiff_t>(end);

    size_t mid = static_cast<size_t>(
      std::partition(first, last, [&](const uint32_t a_item) noexcept { return centroids[a_item][axis] < midpoint; }) -
      itemOrder.begin());

    // Coincident centroids along the axis: split by count instead.
    if (mid == begin || mid == end) {
      mid = begin + (end - begin) / 2;

      std::nth_element(first,
                       itemOrder.begin() + static_cast<std::ptrdiff_t>(mid),
                       last,
                       [&](const uint32_t a_lhs, const uint32_t a_rhs) noexcept {
                         return centroids[a_lhs][axis] < centroids[a_rhs][axis];
                       });
    }

    pending.emplace_back(mid, end);
    pending.emplace_back(begin, mid);
  }

  std::vector<AABB>     clusterBoxes;
  std::vector<Vec3T<T>> clusterCentroids;
  std::vector<uint32_t> clusterWeights;

  clusterBoxes.reserve(clusterRanges.size());
  clusterCentroids.reserve(clusterRanges.size());
  clusterWeights.reserve(clusterRanges.size());

  for (const auto& range : clusterRanges) {
    AABB bv;

    for (size_t i = range.first; i < range.second; i++) {
      bv = bv.merged(a_boxes[itemOrder[i]]);
    }

    clusterBoxes.push_back(bv);
    clusterCentroids.push_back(bv.getCentroid());
    clusterWeights.push_back(static_cast<uint32_t>(range.second - range.first));
  }

  // Phase 2: SAH over the clusters, weighted by their primitive counts.
  TopDownBuilder<T, K> builder{clusterBoxes, clusterCentroids, clusterWeights, Strategy::SAH, a_maxLeaf, {}, {}};

  builder.run();

  // Leaf ranges index the cluster order; turn them into ranges of primitives.
  Topology<T, K> topology;

  std::vector<uint32_t> clusterStart(builder.order.size() + 1, 0);

  topology.order.reserve(numItems);

  for (size_t c = 0; c < builder.order.size(); c++) {
    const auto& range = clusterRanges[builder.order[c]];

    clusterStart[c] = static_cast<uint32_t>(topology.order.size());

    for (size_t i = range.first; i < range.second; i++) {
      topology.order.push_back(itemOrder[i]);
    }
  }

  clusterStart[builder.order.size()] = static_cast<uint32_t>(topology.order.size());

  topology.nodes = std::move(builder.nodes);

  for (auto& node : topology.nodes) {
    for (size_t k = 0; k < K; k++) {
      if (node.isLeaf(k)) {
        const uint32_t first = node.m_child[k];
        const uint32_t last  = first + node.m_count[k];

        node.m_child[k] = clusterStart[first];
        node.m_count[k] = clusterStart[last] - clusterStart[first];
      }
    }
  }

  return topology;
}

/**
 * @brief Space-filling-curve build: sort along the curve, cut into leaves, merge K at a time.
 * @details Leaves hold consecutive runs along the curve, as equal in size as possible and at most
 * a_maxLeaf each. Each level merges consecutive groups of up to K entries into a node; a single
 * entry left over at the end of a level is carried up rather than given a node of its own.
 */
template <class T, size_t K>
[[nodiscard]] Topology<T, K>
buildSpaceFillingCurve(const std::vector<BoundingVolumes::AABBT<T>>& a_boxes,
                       const Curve                                   a_curve,
                       const uint32_t                                a_maxLeaf)
{
  using AABB = BoundingVolumes::AABBT<T>;
  using Node = WideNode<T, K>;

  const size_t numItems = a_boxes.size();

  Topology<T, K> topology;

  if (numItems == 0) {
    return topology;
  }

  const std::vector<SFC::Index> bins = SFC::computeBins<T>(centroidsOf(a_boxes));

  std::vector<SFC::Code> codes;

  codes.reserve(numItems);

  for (const auto& bin : bins) {
    switch (a_curve) {
    case Curve::Nested: {
      codes.push_back(SFC::Nested::encode(bin));

      break;
    }
    case Curve::Hilbert: {
      codes.push_back(SFC::Hilbert::encode(bin));

      break;
    }
    case Curve::Morton:
    default: {
      codes.push_back(SFC::Morton::encode(bin));

      break;
    }
    }
  }

  topology.order.resize(numItems);

  std::iota(topology.order.begin(), topology.order.end(), uint32_t(0));

  std::stable_sort(topology.order.begin(), topology.order.end(), [&codes](const uint32_t a_lhs, const uint32_t a_rhs) {
    return codes[a_lhs] < codes[a_rhs];
  });

  // One entry per subtree of the level being merged: a leaf range, or a node made at a lower level.
  struct Entry
  {
    bool     leaf;
    uint32_t first;
    uint32_t count;
    AABB     box;
  };

  const size_t numLeaves = (numItems + a_maxLeaf - 1) / a_maxLeaf;

  std::vector<Entry> level;

  level.reserve(numLeaves);

  for (size_t leaf = 0, i = 0; leaf < numLeaves; leaf++) {
    const size_t count = numItems / numLeaves + ((leaf < numItems % numLeaves) ? 1 : 0);

    AABB bv;

    for (size_t j = i; j < i + count; j++) {
      bv = bv.merged(a_boxes[topology.order[j]]);
    }

    level.push_back(Entry{true, static_cast<uint32_t>(i), static_cast<uint32_t>(count), bv});

    i += count;
  }

  const auto setSlot = [](Node& a_node, const size_t a_k, const Entry& a_entry) noexcept {
    if (a_entry.leaf) {
      a_node.setLeaf(a_k, a_entry.box, a_entry.first, a_entry.count);
    }
    else {
      a_node.setInterior(a_k, a_entry.box, a_entry.first);
    }
  };

  // Nodes in the order they are made, bottom-up; the root comes last.
  std::vector<Node> made;

  if (level.size() == 1) {
    made.push_back(Node::empty());

    setSlot(made.back(), 0, level.front());
  }

  while (level.size() > 1) {
    std::vector<Entry> next;

    next.reserve(level.size() / K + 1);

    for (size_t i = 0; i < level.size();) {
      const size_t run = Math::min(K, level.size() - i);

      if (run == 1) {
        next.push_back(level[i]);

        i++;

        continue;
      }

      Node node = Node::empty();
      AABB bv;

      for (size_t k = 0; k < run; k++) {
        setSlot(node, k, level[i + k]);

        bv = bv.merged(level[i + k].box);
      }

      next.push_back(Entry{false, static_cast<uint32_t>(made.size()), 0, bv});
      made.push_back(node);

      i += run;
    }

    level = std::move(next);
  }

  // Reverse the creation order so that the root is node 0 and every child follows its parent.
  const uint32_t numNodes = static_cast<uint32_t>(made.size());

  topology.nodes.assign(made.rbegin(), made.rend());

  for (auto& node : topology.nodes) {
    for (size_t k = 0; k < K; k++) {
      if (node.isInterior(k)) {
        node.m_child[k] = numNodes - 1 - node.m_child[k];
      }
    }
  }

  return topology;
}

} // namespace BuildDetail

template <class T, size_t K>
EBGEOMETRY_HOST
size_t
treeDepth(const WideNode<T, K>* a_nodes, const size_t a_count)
{
  if (a_count == 0) {
    return 0;
  }

  size_t maxDepth = 0;

  std::vector<std::pair<uint32_t, size_t>> stack{{0U, size_t(1)}};

  while (!stack.empty()) {
    const uint32_t node  = stack.back().first;
    const size_t   depth = stack.back().second;

    stack.pop_back();

    maxDepth = Math::max(maxDepth, depth);

    for (size_t k = 0; k < K; k++) {
      if (a_nodes[node].isInterior(k)) {
        stack.emplace_back(a_nodes[node].m_child[k], depth + 1);
      }
    }
  }

  return maxDepth;
}

template <class T, size_t K>
EBGEOMETRY_HOST
Topology<T, K>
buildTopology(const std::vector<BoundingVolumes::AABBT<T>>& a_boxes, const BuildSpec& a_spec)
{
  static_assert(std::is_floating_point_v<T>, "BVH::buildTopology requires a floating-point type T");
  static_assert(K >= 2, "BVH::buildTopology: the branching factor K must be at least 2");

  EBGEOMETRY_REQUIRE(a_spec.maxLeafSize > 0, "BVH::BuildSpec: the maximum leaf size must be positive");
  // A traversal indexes a slot as node * K + slot in 32 bits, and a tree over N primitives has fewer
  // than N nodes.
  EBGEOMETRY_REQUIRE(a_boxes.size() <= size_t(WideNode<T, K>::EmptySlot) / K,
                     "BVH::buildTopology: %zu primitives is more than a BVH with K = %zu can index",
                     a_boxes.size(),
                     K);

  for (size_t i = 0; i < a_boxes.size(); i++) {
    const Vec3T<T>& lo = a_boxes[i].getLowCorner();
    const Vec3T<T>& hi = a_boxes[i].getHighCorner();

    bool valid = true;

    for (size_t dir = 0; dir < 3; dir++) {
      valid = valid && std::isfinite(lo[dir]) && std::isfinite(hi[dir]) && lo[dir] <= hi[dir];
    }

    EBGEOMETRY_REQUIRE(valid,
                       "BVH::buildTopology: the bounding box of primitive %zu of %zu is not finite, or is inverted "
                       "(lo = (%g, %g, %g), hi = (%g, %g, %g))",
                       i,
                       a_boxes.size(),
                       double(lo[0]),
                       double(lo[1]),
                       double(lo[2]),
                       double(hi[0]),
                       double(hi[1]),
                       double(hi[2]));
  }

  Topology<T, K> topology;

  switch (a_spec.strategy) {
  case Strategy::SAH:
  case Strategy::Centroid:
  case Strategy::Midpoint: {
    topology = BuildDetail::buildTopDown<T, K>(a_boxes, a_spec.strategy, a_spec.maxLeafSize);

    break;
  }
  case Strategy::ClusterSAH: {
    topology = BuildDetail::buildClusterSAH<T, K>(a_boxes, a_spec.maxLeafSize);

    break;
  }
  case Strategy::SpaceFillingCurve: {
    EBGEOMETRY_REQUIRE(a_spec.curve == Curve::Morton || a_spec.curve == Curve::Nested || a_spec.curve == Curve::Hilbert,
                       "BVH::BuildSpec: unknown curve (%d)",
                       static_cast<int>(a_spec.curve));

    topology = BuildDetail::buildSpaceFillingCurve<T, K>(a_boxes, a_spec.curve, a_spec.maxLeafSize);

    break;
  }
  default: {
    EBGEOMETRY_REQUIRE(false, "BVH::BuildSpec: unknown strategy (%d)", static_cast<int>(a_spec.strategy));
  }
  }

  // Equal-count splits divide every range by K, so they give the shallowest tree; use them if the
  // requested strategy made a tree too deep to traverse on a device.
  if (a_spec.strategy != Strategy::Centroid &&
      treeDepth(topology.nodes.data(), topology.nodes.size()) > DeviceTraversalDepth) {
    topology = BuildDetail::buildTopDown<T, K>(a_boxes, Strategy::Centroid, a_spec.maxLeafSize);
  }

  return topology;
}

} // namespace BVH

} // namespace EBGeometry

#endif
