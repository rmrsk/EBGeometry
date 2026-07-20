// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Tests for the stage-wise node emission of PackedBVH's direct space-filling-curve constructor
// (BVH::paddedTreeDepth / paddedSubtreeSize / paddedDfsIndex / fillLeafNode / fillInteriorNode).
// The keystone is a field-for-field comparison of the constructor's node array against a
// test-local reference implementing the historical scratch-tree + recursive-relayout algorithm
// the stage-wise emission replaced -- certifying the closed-form emission reproduces the original
// build exactly.

#include "EBGeometry.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>
#include <set>
#include <utility>
#include <vector>

#include "TestFloatingPointUtils.hpp"

using namespace EBGeometry;

namespace {

// Brute-force recursive DFS pre-order enumeration of the complete K-ary tree: the ground truth
// paddedDfsIndex() must reproduce. Returns dfs[level][position] = pre-order index.
std::vector<std::vector<uint32_t>>
enumerateDfs(uint32_t a_depth, uint32_t a_K)
{
  std::vector<std::vector<uint32_t>> dfs(a_depth + 1);

  uint64_t width = 1;

  for (uint32_t level = 0; level <= a_depth; level++) {
    dfs[level].resize(width);
    width *= a_K;
  }

  uint32_t counter = 0;

  // NOLINTNEXTLINE(misc-no-recursion)
  const std::function<void(uint32_t, uint32_t)> visit = [&](uint32_t a_level, uint32_t a_position) -> void {
    dfs[a_level][a_position] = counter++;

    if (a_level < a_depth) {

      for (uint32_t c = 0; c < a_K; c++) {
        visit(a_level + 1, a_position * a_K + c);
      }
    }
  };

  visit(0, 0);

  return dfs;
}

// Reference node emission: the historical PackedBVH SFC-constructor algorithm (leaf cutting by a
// linear scan, bottom-up K-ary merge in a scratch array with padding slots aliasing the last real
// leaf's scratch index, then a recursive depth-first relayout with the root at index 0),
// reproduced verbatim from the pre-stage-wise implementation. The constructor must emit exactly
// this node array.
template <class T, size_t K>
std::vector<BVH::PackedNode<T, K>>
scratchRelayoutReference(const std::vector<BoundingVolumes::AABBT<T>>& a_sortedBVs, size_t a_targetLeafSize)
{
  using BV   = BoundingVolumes::AABBT<T>;
  using Node = BVH::PackedNode<T, K>;

  const size_t numPrimitives = a_sortedBVs.size();

  struct LeafRange
  {
    uint32_t offset;
    uint32_t count;
    BV       bv;
  };

  std::vector<LeafRange> leafRanges;

  for (size_t i = 0; i < numPrimitives;) {
    const size_t count = std::min(a_targetLeafSize, numPrimitives - i);

    std::vector<BV> leafBVs;

    for (size_t j = 0; j < count; j++) {
      leafBVs.push_back(a_sortedBVs[i + j]);
    }

    leafRanges.push_back({static_cast<uint32_t>(i), static_cast<uint32_t>(count), BV(leafBVs)});

    i += count;
  }

  const size_t numRealLeaves = leafRanges.size();

  size_t paddedLeafCount = 1;

  while (paddedLeafCount < numRealLeaves) {
    paddedLeafCount *= K;
  }

  std::vector<Node> scratch(paddedLeafCount);

  for (size_t i = 0; i < numRealLeaves; i++) {
    scratch[i].setBoundingVolume(leafRanges[i].bv);
    scratch[i].setPrimitivesOffset(leafRanges[i].offset);
    scratch[i].setNumPrimitives(leafRanges[i].count);
  }

  std::vector<uint32_t> levelIndices(paddedLeafCount);

  for (size_t i = 0; i < paddedLeafCount; i++) {
    levelIndices[i] = static_cast<uint32_t>(i < numRealLeaves ? i : numRealLeaves - 1);
  }

  while (levelIndices.size() > 1) {
    const size_t          numParents = levelIndices.size() / K;
    std::vector<uint32_t> parentIndices(numParents);

    for (size_t p = 0; p < numParents; p++) {
      const uint32_t parentIdx = static_cast<uint32_t>(scratch.size());
      scratch.emplace_back();

      std::vector<BV> childBVs;

      for (size_t c = 0; c < K; c++) {
        const uint32_t childIdx = levelIndices[p * K + c];
        scratch[parentIdx].setChildOffset(childIdx, c);
        childBVs.push_back(scratch[childIdx].getBoundingVolume());
      }

      scratch[parentIdx].setBoundingVolume(BV(childBVs));

      parentIndices[p] = parentIdx;
    }

    levelIndices = std::move(parentIndices);
  }

  std::vector<Node> linearNodes;
  linearNodes.reserve(scratch.size());

  // NOLINTNEXTLINE(misc-no-recursion)
  std::function<uint32_t(uint32_t)> relayout = [&](uint32_t a_scratchIdx) -> uint32_t {
    const uint32_t newIdx = static_cast<uint32_t>(linearNodes.size());

    linearNodes.push_back(scratch[a_scratchIdx]);

    if (!scratch[a_scratchIdx].isLeaf()) {

      for (size_t c = 0; c < K; c++) {
        const uint32_t newChildIdx = relayout(scratch[a_scratchIdx].getChildOffsets()[c]);

        linearNodes[newIdx].setChildOffset(newChildIdx, c);
      }
    }

    return newIdx;
  };

  relayout(levelIndices.front());

  return linearNodes;
}

// Deterministic pseudo-random points in [0,1)^3 (LCG; no <random> engine variability).
template <class T>
std::vector<Vec3T<T>>
randomPoints(size_t a_n, uint32_t a_seed)
{
  uint32_t state = a_seed;

  const auto next = [&state]() -> T {
    state = state * 1103515245u + 12345u;
    return T(state >> 8) / T(1u << 24);
  };

  std::vector<Vec3T<T>> points;
  points.reserve(a_n);

  for (size_t i = 0; i < a_n; i++) {
    const T x = next();
    const T y = next();
    const T z = next();

    points.emplace_back(x, y, z);
  }

  return points;
}

// Sort points by ascending Morton code with std::stable_sort, requiring all codes distinct (the
// precondition under which the constructor's unstable std::sort produces the same order).
template <class T>
std::vector<Vec3T<T>>
mortonSorted(const std::vector<Vec3T<T>>& a_points)
{
  const std::vector<SFC::Index> bins = SFC::computeBins<T>(a_points);

  std::vector<SFC::Code> codes;
  codes.reserve(a_points.size());

  for (const auto& bin : bins) {
    codes.push_back(SFC::Morton::encode(bin));
  }

  REQUIRE(std::set<SFC::Code>(codes.begin(), codes.end()).size() == a_points.size());

  std::vector<uint32_t> order(a_points.size());
  std::iota(order.begin(), order.end(), uint32_t(0));
  std::stable_sort(order.begin(), order.end(), [&codes](uint32_t a_lhs, uint32_t a_rhs) -> bool {
    return codes[a_lhs] < codes[a_rhs];
  });

  std::vector<Vec3T<T>> sorted;
  sorted.reserve(a_points.size());

  for (const auto idx : order) {
    sorted.push_back(a_points[idx]);
  }

  return sorted;
}

} // namespace

TEST_CASE("PackedBVH build: paddedTreeDepth matches the padding loop", "[BVH][PackedBVH]")
{
  for (uint32_t K : {2u, 4u, 8u}) {
    REQUIRE(BVH::paddedTreeDepth(1, K) == 0u);
    REQUIRE(BVH::paddedTreeDepth(2, K) == 1u);
    REQUIRE(BVH::paddedTreeDepth(K, K) == 1u);
    REQUIRE(BVH::paddedTreeDepth(K + 1, K) == 2u);
    REQUIRE(BVH::paddedTreeDepth(K * K, K) == 2u);
    REQUIRE(BVH::paddedTreeDepth(K * K + 1, K) == 3u);
  }
}

TEST_CASE("PackedBVH build: paddedSubtreeSize closed form", "[BVH][PackedBVH]")
{
  for (uint32_t K : {2u, 4u, 8u}) {

    for (uint32_t depth = 0; depth <= 4; depth++) {

      for (uint32_t level = 0; level <= depth; level++) {
        // Sum the level widths of the subtree directly.
        uint64_t expected = 0;
        uint64_t width    = 1;

        for (uint32_t l = level; l <= depth; l++) {
          expected += width;
          width *= K;
        }

        REQUIRE(BVH::paddedSubtreeSize(level, depth, K) == expected);
      }
    }
  }
}

TEST_CASE("PackedBVH build: paddedDfsIndex equals brute-force recursive DFS enumeration", "[BVH][PackedBVH]")
{
  for (uint32_t K : {2u, 4u, 8u}) {

    for (uint32_t depth = 0; depth <= 4; depth++) {
      const auto dfs = enumerateDfs(depth, K);

      for (uint32_t level = 0; level <= depth; level++) {

        for (uint32_t position = 0; position < dfs[level].size(); position++) {
          REQUIRE(BVH::paddedDfsIndex(level, position, depth, K) == dfs[level][position]);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH build: fill-node invariants (padding aliasing, BV containment)",
                   "[BVH][PackedBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  constexpr size_t K = 4;

  const uint32_t leafSize = 3;

  const std::vector<Vec3T<T>> points = mortonSorted(randomPoints<T>(29, 3u)); // 10 real leaves, padded to 16

  std::vector<AABB> sortedBVs;

  for (const auto& p : points) {
    sortedBVs.emplace_back(p, p);
  }

  const uint32_t numPoints     = uint32_t(points.size());
  const uint32_t numRealLeaves = (numPoints + leafSize - 1) / leafSize;
  const uint32_t depth         = BVH::paddedTreeDepth(numRealLeaves, uint32_t(K));

  REQUIRE(numRealLeaves == 10u);
  REQUIRE(depth == 2u);

  // Padding slots are exact copies of the last real leaf.
  BVH::PackedNode<T, K> lastReal;
  BVH::fillLeafNode<T, K>(lastReal, sortedBVs.data(), numRealLeaves - 1, numRealLeaves, numPoints, leafSize);

  REQUIRE(lastReal.getPrimitivesOffset() == (numRealLeaves - 1) * leafSize);
  REQUIRE(lastReal.getNumPrimitives() == numPoints - (numRealLeaves - 1) * leafSize);

  for (uint32_t slot = numRealLeaves; slot < 16; slot++) {
    BVH::PackedNode<T, K> padded;
    BVH::fillLeafNode<T, K>(padded, sortedBVs.data(), slot, numRealLeaves, numPoints, leafSize);

    REQUIRE(padded.getPrimitivesOffset() == lastReal.getPrimitivesOffset());
    REQUIRE(padded.getNumPrimitives() == lastReal.getNumPrimitives());
    REQUIRE(padded.getBoundingVolume().getLowCorner() == lastReal.getBoundingVolume().getLowCorner());
    REQUIRE(padded.getBoundingVolume().getHighCorner() == lastReal.getBoundingVolume().getHighCorner());
  }

  // Constructor-built tree: every leaf BV contains its primitives; every interior BV contains its
  // children's BVs.
  std::vector<std::pair<uint32_t, AABB>> primsAndBVs;

  for (uint32_t i = 0; i < numPoints; i++) {
    primsAndBVs.emplace_back(i, sortedBVs[i]);
  }

  const BVH::PackedBVH<T, uint32_t, K, BVH::ValueStorage<uint32_t>> bvh(std::move(primsAndBVs), leafSize);

  const auto& nodes = bvh.getNodes();

  for (const auto& node : nodes) {

    if (node.isLeaf()) {

      for (uint32_t i = 0; i < node.getNumPrimitives(); i++) {
        const Vec3T<T>& p = points[node.getPrimitivesOffset() + i];

        REQUIRE(node.getBoundingVolume().getDistance2(p) == T(0));
      }
    }
    else {

      for (const auto childIndex : node.getChildOffsets()) {
        REQUIRE(childIndex < nodes.size());

        const auto& childBV = nodes[childIndex].getBoundingVolume();

        for (size_t dir = 0; dir < 3; dir++) {
          REQUIRE(node.getBoundingVolume().getLowCorner()[dir] <= childBV.getLowCorner()[dir]);
          REQUIRE(node.getBoundingVolume().getHighCorner()[dir] >= childBV.getHighCorner()[dir]);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH build: SFC constructor equals the scratch-relayout reference (keystone)",
                   "[BVH][PackedBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  using AABB   = BoundingVolumes::AABBT<T>;
  using Packed = BVH::PackedBVH<T, uint32_t, K, BVH::ValueStorage<uint32_t>>;

  uint32_t seed = 20260720u;

  const size_t pointCounts[] = {1, 2, 5, 16, 17, 64, 100, 1000};

  for (const size_t numPoints : pointCounts) {

    for (const uint32_t leafSize : {1u, 3u, 4u, 16u}) {
      const std::vector<Vec3T<T>> points = randomPoints<T>(numPoints, ++seed);
      const std::vector<Vec3T<T>> sorted = mortonSorted(points);

      // The constructor input is deliberately *unsorted*: its internal sort stage must reproduce
      // the stable-sorted order (all Morton codes are distinct -- asserted in mortonSorted() --
      // and AABB(p, p).getCentroid() is exactly p, so the constructor bins the very same points).
      std::vector<std::pair<uint32_t, AABB>> primsAndBVs;
      primsAndBVs.reserve(numPoints);

      for (size_t i = 0; i < numPoints; i++) {
        primsAndBVs.emplace_back(uint32_t(i), AABB(points[i], points[i]));
      }

      const Packed bvh(std::move(primsAndBVs), leafSize);

      std::vector<AABB> sortedBVs;
      sortedBVs.reserve(numPoints);

      for (const auto& p : sorted) {
        sortedBVs.emplace_back(p, p);
      }

      const std::vector<BVH::PackedNode<T, K>> reference = scratchRelayoutReference<T, K>(sortedBVs, leafSize);

      REQUIRE(bvh.getNodes().size() == reference.size());

      for (size_t i = 0; i < reference.size(); i++) {
        const auto& node = bvh.getNodes()[i];
        const auto& ref  = reference[i];

        REQUIRE(node.getPrimitivesOffset() == ref.getPrimitivesOffset());
        REQUIRE(node.getNumPrimitives() == ref.getNumPrimitives());
        REQUIRE(node.getChildOffsets() == ref.getChildOffsets());

        // Bit-exact bounding volumes: both sides are pure min/max fits of the same values.
        for (size_t dir = 0; dir < 3; dir++) {
          REQUIRE(node.getBoundingVolume().getLowCorner()[dir] == ref.getBoundingVolume().getLowCorner()[dir]);
          REQUIRE(node.getBoundingVolume().getHighCorner()[dir] == ref.getBoundingVolume().getHighCorner()[dir]);
        }
      }
    }
  }
}

TEMPLATE_TEST_CASE("PackedBVH build: single-leaf cloud builds a one-node tree",
                   "[BVH][PackedBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  const std::vector<Vec3T<T>> points = randomPoints<T>(7, 99u);

  std::vector<std::pair<uint32_t, AABB>> primsAndBVs;

  for (uint32_t i = 0; i < 7; i++) {
    primsAndBVs.emplace_back(i, AABB(points[i], points[i]));
  }

  const BVH::PackedBVH<T, uint32_t, 4, BVH::ValueStorage<uint32_t>> bvh(std::move(primsAndBVs), 16);

  REQUIRE(bvh.getNodes().size() == 1u);
  REQUIRE(bvh.getNodes()[0].isLeaf());
  REQUIRE(bvh.getNodes()[0].getPrimitivesOffset() == 0u);
  REQUIRE(bvh.getNodes()[0].getNumPrimitives() == 7u);
}
