// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHImplem.hpp
 * @brief  Implementation of EBGeometry_BVH.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHIMPLEM_HPP
#define EBGEOMETRY_BVHIMPLEM_HPP

// Std includes
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <stack>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_BoundingVolumes.hpp"

namespace EBGeometry {

namespace BVH {

template <class T, class P, class BV, size_t K>
inline TreeBVH<T, P, BV, K>::TreeBVH() noexcept
{
  for (auto& c : m_children) {
    c = nullptr;
  }

  m_primitives.resize(0);
  m_boundingVolumes.resize(0);

  m_partitioned = false;
}

template <class T, class P, class BV, size_t K>
inline TreeBVH<T, P, BV, K>::TreeBVH(const std::vector<PrimAndBV<P, BV>>& a_primsAndBVs) : TreeBVH<T, P, BV, K>()
{
  for (const auto& pbv : a_primsAndBVs) {
    m_primitives.emplace_back(pbv.first);
    m_boundingVolumes.emplace_back(pbv.second);
  }

  m_boundingVolume = BV(m_boundingVolumes);
  m_partitioned    = false;
}

template <class T, class P, class BV, size_t K>
inline TreeBVH<T, P, BV, K>::~TreeBVH() noexcept = default;

template <class T, class P, class BV, size_t K>
inline bool
TreeBVH<T, P, BV, K>::isLeaf() const noexcept
{
  return m_primitives.size() > 0;
}

template <class T, class P, class BV, size_t K>
inline bool
TreeBVH<T, P, BV, K>::isPartitioned() const noexcept
{
  return m_partitioned;
}

template <class T, class P, class BV, size_t K>
inline const BV&
TreeBVH<T, P, BV, K>::getBoundingVolume() const noexcept
{
  return m_boundingVolume;
}

template <class T, class P, class BV, size_t K>
inline PrimitiveList<P>&
TreeBVH<T, P, BV, K>::getPrimitives() noexcept
{
  return m_primitives;
}

template <class T, class P, class BV, size_t K>
inline const PrimitiveList<P>&
TreeBVH<T, P, BV, K>::getPrimitives() const noexcept
{
  return m_primitives;
}

template <class T, class P, class BV, size_t K>
inline std::vector<BV>&
TreeBVH<T, P, BV, K>::getBoundingVolumes() noexcept
{
  return m_boundingVolumes;
}

template <class T, class P, class BV, size_t K>
inline const std::vector<BV>&
TreeBVH<T, P, BV, K>::getBoundingVolumes() const noexcept
{
  return m_boundingVolumes;
}

template <class T, class P, class BV, size_t K>
inline const std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>&
TreeBVH<T, P, BV, K>::getChildren() const noexcept
{
  return m_children;
}

template <class T, class P, class BV, size_t K>
inline std::shared_ptr<TreeBVH<T, P, BV, K>>
TreeBVH<T, P, BV, K>::deepCopy() const
{
  auto copy = std::make_shared<TreeBVH<T, P, BV, K>>();

  // Copy this node's own state. m_primitives holds shared_ptr<const P>, copied by handle: the
  // primitives are immutable and intentionally shared (e.g. with a DCEL mesh), so they are not
  // cloned -- only the node hierarchy is.
  copy->m_boundingVolume  = m_boundingVolume;
  copy->m_primitives      = m_primitives;
  copy->m_boundingVolumes = m_boundingVolumes;
  copy->m_partitioned     = m_partitioned;

  // Recursively clone children so the returned tree owns independent nodes rather than aliasing
  // this tree's mutable sub-structure.
  for (size_t k = 0; k < K; k++) {
    if (m_children[k] != nullptr) {
      copy->m_children[k] = m_children[k]->deepCopy();
    }
  }

  return copy;
}

template <class T, class P, class BV, size_t K>
inline void
TreeBVH<T, P, BV, K>::topDownSortAndPartition(const Partitioner& a_partitioner, const LeafPredicate& a_stopCrit)
{
  // Check if this node should be split into more nodes.
  const auto numPrimsInThisNode = m_primitives.size();

  const bool stopRecursiveSplitting = a_stopCrit(*this);
  const bool hasEnoughPrimitives    = numPrimsInThisNode >= K;

  if (!stopRecursiveSplitting && hasEnoughPrimitives) {

    // Pack primitives and BVs.
    PrimAndBVList<P, BV> primsAndBVs;
    for (size_t i = 0; i < numPrimsInThisNode; i++) {
      primsAndBVs.emplace_back(std::make_pair(m_primitives[i], m_boundingVolumes[i]));
    }

    // Partition into sub-sets.
    const auto& newPartitions = a_partitioner(primsAndBVs);

    // Create children nodes.
    for (size_t c = 0; c < K; c++) {
      m_children[c] = std::make_shared<TreeBVH<T, P, BV, K>>(newPartitions[c]);
    }

    // This is no longer a leaf node.
    m_primitives.resize(0);
    m_boundingVolumes.resize(0);

    // Recursive partitioning.
    for (auto& c : m_children) {
      c->topDownSortAndPartition(a_partitioner, a_stopCrit);
    }
  }

  m_partitioned = true;
}

template <class T, class P, class BV, size_t K>
template <typename S>
inline void
TreeBVH<T, P, BV, K>::bottomUpSortAndPartition()
{
  std::vector<Vec3> centroids;
  centroids.reserve(m_boundingVolumes.size());
  for (const auto& bv : m_boundingVolumes) {
    centroids.push_back(bv.getCentroid());
  }

  const std::vector<SFC::Index> bins = SFC::computeBins<T>(centroids);

  // Sort the primitives, their BVs, and their spatial bins along the space-filling curves.
  using PrimBvAndCode = std::tuple<std::shared_ptr<const P>, BV, SFC::Code>;

  std::vector<PrimBvAndCode> sortedPrimitives;
  sortedPrimitives.reserve(m_primitives.size());

  for (size_t i = 0; i < m_primitives.size(); i++) {
    // Construct the tuple in place rather than std::make_tuple(...) into a temporary and then
    // moving/copying that into the vector.
    sortedPrimitives.emplace_back(m_primitives[i], m_boundingVolumes[i], S::encode(bins[i]));
  }

  auto sortCrit = [](const PrimBvAndCode& A, const PrimBvAndCode& B) -> bool {
    return std::get<2>(A) < std::get<2>(B);
  };

  std::sort(std::begin(sortedPrimitives), std::end(sortedPrimitives), sortCrit);

  // Go through the SFC and merge leaves that are nearby. We are trying to build a _balanced_
  // tree where all the leaves exist on the same level, so the numb
  // a root node in the end.
  const size_t numPrimitives = sortedPrimitives.size();
  const size_t treeDepth     = std::floor(std::log(numPrimitives) / std::log(K));
  const size_t numLeaves     = std::pow(K, treeDepth);
  const size_t primsPerLeaf  = numPrimitives / numLeaves;

  if (treeDepth > 0) {

    std::vector<std::vector<std::shared_ptr<TreeBVH<T, P, BV, K>>>> nodes(treeDepth + 1);
    nodes[treeDepth].reserve(numLeaves);

    // Build the leaves by partitioning the primitives along the SFC.
    size_t startIndex = 0;
    size_t endIndex   = 0;
    size_t remainder  = numPrimitives % numLeaves;

    for (size_t ileaf = 0; ileaf < numLeaves; ileaf++) {
      endIndex = startIndex + primsPerLeaf - 1;

      if (remainder > 0) {
        endIndex  = endIndex + 1;
        remainder = remainder - 1;
      }

      std::vector<BVH::PrimAndBV<P, BV>> primsAndBVs;
      primsAndBVs.reserve(endIndex - startIndex + 1);

      for (size_t i = startIndex; i <= endIndex; i++) {
        const auto& cur = sortedPrimitives[i];

        primsAndBVs.emplace_back(std::get<0>(cur), std::get<1>(cur));
      }

      nodes[treeDepth].emplace_back(std::make_shared<TreeBVH<T, P, BV, K>>(primsAndBVs));

      startIndex = endIndex + 1;
    }

    // Starting at the bottom of the tree, merge the nodes upward in clusters of K.
    for (int lvl = static_cast<int>(treeDepth) - 1; lvl >= 0; lvl--) {
      nodes[static_cast<size_t>(lvl)].resize(0);

      size_t numNodesAtLevel = 1;
      for (int l = 0; l < lvl; l++) {
        numNodesAtLevel *= K;
      }

      nodes[static_cast<size_t>(lvl)].reserve(numNodesAtLevel);

      for (size_t inode = 0; inode < numNodesAtLevel; inode++) {

        std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K> children;

        for (size_t child = 0; child < K; child++) {
          children[child] = nodes[static_cast<size_t>(lvl) + 1][inode * K + child];
        }

        if (lvl > 0) {
          nodes[static_cast<size_t>(lvl)].emplace_back(std::make_shared<TreeBVH<T, P, BV, K>>());
        }
        else {
          nodes[static_cast<size_t>(lvl)].emplace_back(this->shared_from_this());
        }

        nodes[static_cast<size_t>(lvl)].back()->setChildren(children);
      }
    }
  }

  m_partitioned = true;
}

template <class T, class P, class BV, size_t K>
inline void
TreeBVH<T, P, BV, K>::setChildren(const std::array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>& a_children) noexcept
{
  std::vector<BV> boundingVolumes;
  boundingVolumes.reserve(a_children.size());
  for (const auto& child : a_children) {
    boundingVolumes.emplace_back(child->getBoundingVolume());
  }

  m_primitives.resize(0);
  m_boundingVolumes.resize(0);

  m_boundingVolume = BV(boundingVolumes);
  m_children       = a_children;
  m_partitioned    = true;
}

template <class T, class P, class BV, size_t K>
inline T
TreeBVH<T, P, BV, K>::getDistanceToBoundingVolume(const Vec3& a_point) const noexcept
{
  return m_boundingVolume.getDistance(a_point);
}

template <class T, class P, class BV, size_t K>
template <class NodeKey>
inline void
TreeBVH<T, P, BV, K>::traverse(const BVH::LeafEvaluator<P>&               a_leafEvaluator,
                               const BVH::PrunePredicate<Node, NodeKey>&  a_prunePredicate,
                               const BVH::ChildOrderer<Node, NodeKey, K>& a_childOrderer,
                               const BVH::NodeKeyFactory<Node, NodeKey>&  a_nodeKeyFactory) const noexcept
{
  std::array<std::pair<std::shared_ptr<const Node>, NodeKey>, K> children;
  std::stack<std::pair<std::shared_ptr<const Node>, NodeKey>>    q;

  q.emplace(this->shared_from_this(), a_nodeKeyFactory(*this));

  while (!(q.empty())) {
    const auto& node    = q.top().first;
    const auto& nodeKey = q.top().second;

    q.pop();

    if (a_prunePredicate(*node, nodeKey)) {
      if (node->isLeaf()) {
        a_leafEvaluator(node->getPrimitives());
      }
      else {
        for (size_t k = 0; k < K; k++) {
          children[k].first  = node->getChildren()[k];
          children[k].second = a_nodeKeyFactory(*children[k].first);
        }

        // User-based visit pattern.
        a_childOrderer(children);

        for (const auto& child : children) {
          q.push(child);
        }
      }
    }
  }
}

template <class T, class P, class BV, size_t K>
template <class StoragePolicy>
inline std::shared_ptr<PackedBVH<T, P, K, StoragePolicy>>
TreeBVH<T, P, BV, K>::pack() const
{
  static_assert(std::is_same_v<BV, EBGeometry::BoundingVolumes::AABBT<T>>, "TreeBVH::pack requires BV == AABBT<T>");

  return std::make_shared<PackedBVH<T, P, K, StoragePolicy>>(*this);
}

template <class T, class P, class BV, size_t K>
template <class Q, class Converter, class StoragePolicy>
inline std::shared_ptr<PackedBVH<T, Q, K, StoragePolicy>>
TreeBVH<T, P, BV, K>::packWith(Converter&& a_converter) const
{
  static_assert(std::is_same_v<BV, EBGeometry::BoundingVolumes::AABBT<T>>, "TreeBVH::packWith requires BV == AABBT<T>");

  return std::make_shared<PackedBVH<T, Q, K, StoragePolicy>>(*this, std::forward<Converter>(a_converter));
}

template <class T, class P, size_t K, class StoragePolicy>
inline void
PackedBVH<T, P, K, StoragePolicy>::buildSoA()
{
  m_childAabbSoA.resize(m_linearNodes.size());

  for (size_t i = 0; i < m_linearNodes.size(); i++) {
    const auto& node = m_linearNodes[i];

    if (!node.isLeaf()) {
      const auto& offsets = node.getChildOffsets();
      auto&       soa     = m_childAabbSoA[i];

      for (size_t k = 0; k < K; k++) {
        const auto& bv = m_linearNodes[offsets[k]].getBoundingVolume();
        const auto& lo = bv.getLowCorner();
        const auto& hi = bv.getHighCorner();

        soa.m_lo[0][k] = lo[0];
        soa.m_lo[1][k] = lo[1];
        soa.m_lo[2][k] = lo[2];

        soa.m_hi[0][k] = hi[0];
        soa.m_hi[1][k] = hi[1];
        soa.m_hi[2][k] = hi[2];
      }
    }
  }
}

template <class T, class P, size_t K, class StoragePolicy>
inline PackedBVH<T, P, K, StoragePolicy>::PackedBVH(
  const TreeBVH<T, P, EBGeometry::BoundingVolumes::AABBT<T>, K>& a_tree)
{
  using AABBType = EBGeometry::BoundingVolumes::AABBT<T>;

  // Depth-first. Each call reserves a slot by index, then fills child offsets
  // after recursion. Indexing by position (not pointer) is safe across
  // vector reallocation; C++17 RHS-before-LHS ensures fresh re-fetch of
  // m_linearNodes[idx] after any push_back inside the recursive call.
  std::function<uint32_t(const TreeBVH<T, P, AABBType, K>&)> dfs =
    [&](const TreeBVH<T, P, AABBType, K>& node) -> uint32_t {
    const uint32_t idx = static_cast<uint32_t>(m_linearNodes.size());

    m_linearNodes.push_back({});
    m_linearNodes[idx].m_bv = node.getBoundingVolume();

    if (node.isLeaf()) {
      const auto& prims = node.getPrimitives();

      m_linearNodes[idx].m_primOff  = static_cast<uint32_t>(m_primitives.size());
      m_linearNodes[idx].m_numPrims = static_cast<uint32_t>(prims.size());

      StoragePolicy::appendTreeLeaf(m_primitives, prims);
    }
    else {
      m_linearNodes[idx].m_numPrims = 0U;
      m_linearNodes[idx].m_primOff  = 0U;

      const auto& children = node.getChildren();

      for (size_t k = 0; k < K; k++) {
        m_linearNodes[idx].m_childOff[k] = dfs(*children[k]);
      }
    }
    return idx;
  };

  dfs(a_tree);
  buildSoA();
}

template <class T, class P, size_t K, class StoragePolicy>
template <class Q, class Converter>
inline PackedBVH<T, P, K, StoragePolicy>::PackedBVH(
  const TreeBVH<T, Q, EBGeometry::BoundingVolumes::AABBT<T>, K>& a_tree, Converter&& a_converter)
{
  using AABBType = EBGeometry::BoundingVolumes::AABBT<T>;

  // Accumulate converted P-values into a single contiguous buffer; StoragePolicy::appendAliased
  // materialises it into m_primitives once every push_back below has completed (aliased
  // shared_ptrs, for the default SharedPtrStorage<P>, so no dangling pointers).
  auto dstStorage = std::make_shared<std::vector<P>>();

  std::function<uint32_t(const TreeBVH<T, Q, AABBType, K>&)> dfs =
    [&](const TreeBVH<T, Q, AABBType, K>& node) -> uint32_t {
    const uint32_t idx = static_cast<uint32_t>(m_linearNodes.size());

    m_linearNodes.push_back({});
    m_linearNodes[idx].m_bv = node.getBoundingVolume();

    if (node.isLeaf()) {
      const auto&    prims  = node.getPrimitives();
      const uint32_t dstOff = static_cast<uint32_t>(dstStorage->size());

      auto newVals = a_converter(prims, 0U, static_cast<uint32_t>(prims.size()));

      m_linearNodes[idx].m_primOff  = dstOff;
      m_linearNodes[idx].m_numPrims = static_cast<uint32_t>(newVals.size());

      for (auto&& v : newVals) {
        dstStorage->push_back(std::move(v));
      }
    }
    else {
      m_linearNodes[idx].m_numPrims = 0U;
      m_linearNodes[idx].m_primOff  = 0U;

      const auto& children = node.getChildren();

      for (size_t k = 0; k < K; k++) {
        m_linearNodes[idx].m_childOff[k] = dfs(*children[k]);
      }
    }

    return idx;
  };

  dfs(a_tree);

  StoragePolicy::appendAliased(m_primitives, dstStorage);

  this->buildSoA();
}

template <class T, class P, size_t K, class StoragePolicy>
template <class S>
inline PackedBVH<T, P, K, StoragePolicy>::PackedBVH(std::vector<std::pair<P, BV>> a_primsAndBVs,
                                                    size_t                        a_targetLeafSize,
                                                    S)
{
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());
  EBGEOMETRY_EXPECT(a_targetLeafSize > 0);

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

  // Cut leaves via a single linear scan at the target leaf size (unlike
  // TreeBVH::bottomUpSortAndPartition(), which derives a leaf count of K^floor(log_K(N)) from N
  // and K alone).
  struct LeafRange
  {
    uint32_t offset;
    uint32_t count;
    BV       bv;
  };

  std::vector<LeafRange> leafRanges;
  leafRanges.reserve(numPrimitives / a_targetLeafSize + 1);
  for (size_t i = 0; i < numPrimitives;) {
    const size_t count = std::min(a_targetLeafSize, numPrimitives - i);

    std::vector<BV> leafBVs;
    leafBVs.reserve(count);
    for (size_t j = 0; j < count; j++) {
      leafBVs.push_back(std::get<1>(sortedPrimitives[i + j]));
    }

    leafRanges.push_back({static_cast<uint32_t>(i), static_cast<uint32_t>(count), BV(leafBVs)});
    i += count;
  }

  // Populate the primitive array once, in final sorted order -- independent of whatever order
  // the node array below ends up being built/relaid-out in.
  auto primBlock = std::make_shared<std::vector<P>>();
  primBlock->reserve(numPrimitives);
  for (auto& entry : sortedPrimitives) {
    primBlock->push_back(std::move(std::get<0>(entry)));
  }
  StoragePolicy::appendAliased(m_primitives, primBlock);

  // Build the K-ary structure bottom-up in a scratch array (reusing Node's own shape), then relay
  // it out into m_linearNodes in depth-first pre-order below -- a bottom-up merge naturally
  // produces the root last, but PackedBVH's traversal assumes the root is always at index 0.
  const size_t numRealLeaves = leafRanges.size();

  size_t paddedLeafCount = 1;
  while (paddedLeafCount < numRealLeaves) {
    paddedLeafCount *= K;
  }

  std::vector<Node> scratch(paddedLeafCount);
  // Reserve the exact final node count up front -- for a full K-ary tree with paddedLeafCount
  // leaves (itself a power of K), the total node count across every level is
  // (paddedLeafCount * K - 1) / (K - 1) (geometric series 1 + K + K^2 + ... + paddedLeafCount) --
  // so the emplace_back() calls building interior nodes below never trigger a reallocation.
  scratch.reserve((paddedLeafCount * K - 1) / (K - 1));
  for (size_t i = 0; i < numRealLeaves; i++) {
    scratch[i].setBoundingVolume(leafRanges[i].bv);
    scratch[i].setPrimitivesOffset(leafRanges[i].offset);
    scratch[i].setNumPrimitives(leafRanges[i].count);
  }

  // Padding slots (present only when numRealLeaves isn't already a power of K) reuse the last
  // real leaf's scratch index rather than inventing an empty placeholder node: the resulting
  // duplicate Node entries (one per parent that references it) are cheap, and re-visiting the
  // same primitives more than once is harmless for any min-reduction query -- see the
  // constructor's doxygen comment.
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
      childBVs.reserve(K);
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

  const uint32_t scratchRoot = levelIndices.front();

  // Relay the scratch tree into m_linearNodes in depth-first pre-order (root at index 0), exactly
  // matching the invariant the other two constructors establish.
  m_linearNodes.reserve(scratch.size());

  std::function<uint32_t(uint32_t)> relayout = [&](uint32_t a_scratchIdx) -> uint32_t {
    const uint32_t newIdx = static_cast<uint32_t>(m_linearNodes.size());
    m_linearNodes.push_back(scratch[a_scratchIdx]);

    if (!scratch[a_scratchIdx].isLeaf()) {
      for (size_t c = 0; c < K; c++) {
        const uint32_t newChildIdx = relayout(scratch[a_scratchIdx].getChildOffsets()[c]);
        m_linearNodes[newIdx].setChildOffset(static_cast<uint32_t>(newChildIdx), c);
      }
    }

    return newIdx;
  };

  relayout(scratchRoot);

  buildSoA();
}

template <class T, class P, size_t K, class StoragePolicy>
inline PackedBVH<T, P, K, StoragePolicy>::PackedBVH(std::vector<std::pair<P, BV>>          a_primsAndBVs,
                                                    const BVH::Partitioner<P, BV, K>&      a_partitioner,
                                                    const BVH::LeafPredicate<T, P, BV, K>& a_stopCrit)
{
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

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

  // Recursively partition top-down, writing nodes directly into m_linearNodes in pre-order (root
  // first) as the recursion unwinds -- unlike the SFC-build constructor above, top-down
  // partitioning visits the root before its children, so no relayout pass is needed here. At every
  // split, a lightweight, stack-local TreeBVH ("probe") is constructed purely to evaluate
  // a_stopCrit (whose signature expects an actual TreeBVH node, matching
  // TreeBVH::topDownSortAndPartition()'s own contract) and to read off its primitive list; it is
  // discarded immediately afterward, never linked into a persistent tree.
  std::function<uint32_t(const BVH::PrimAndBVList<P, BV>&)> build =
    [&](const BVH::PrimAndBVList<P, BV>& a_prims) -> uint32_t {
    const uint32_t idx = static_cast<uint32_t>(m_linearNodes.size());
    m_linearNodes.push_back({});

    const BVH::TreeBVH<T, P, BV, K> probe(a_prims);
    m_linearNodes[idx].setBoundingVolume(probe.getBoundingVolume());

    if (a_stopCrit(probe) || a_prims.size() < K) {
      const auto& leafPrims = probe.getPrimitives();

      m_linearNodes[idx].setPrimitivesOffset(static_cast<uint32_t>(m_primitives.size()));
      m_linearNodes[idx].setNumPrimitives(static_cast<uint32_t>(leafPrims.size()));

      StoragePolicy::appendTreeLeaf(m_primitives, leafPrims);
    }
    else {
      const auto children = a_partitioner(a_prims);

      for (size_t k = 0; k < K; k++) {
        const uint32_t childIdx = build(children[k]);
        m_linearNodes[idx].setChildOffset(childIdx, k);
      }
    }

    return idx;
  };

  build(wrapped);

  buildSoA();
}

template <class T, class P, size_t K, class StoragePolicy>
inline const std::vector<typename PackedBVH<T, P, K, StoragePolicy>::StorageType>&
PackedBVH<T, P, K, StoragePolicy>::getPrimitives() const noexcept
{
  return m_primitives;
}

template <class T, class P, size_t K, class StoragePolicy>
inline const EBGeometry::BoundingVolumes::AABBT<T>&
PackedBVH<T, P, K, StoragePolicy>::getBoundingVolume() const noexcept
{
  return m_linearNodes.front().getBoundingVolume();
}

template <class T, class P, size_t K, class StoragePolicy>
inline EBGeometry::BoundingVolumes::AABBT<T>
PackedBVH<T, P, K, StoragePolicy>::computeBoundingVolume() const noexcept
{
  return m_linearNodes.front().getBoundingVolume();
}

template <class T, class P, size_t K, class StoragePolicy>
template <class NodeKey>
inline void
PackedBVH<T, P, K, StoragePolicy>::traverse(const BVH::PackedLeafEvaluator<P, StoragePolicy>& a_leafEvaluator,
                                            const BVH::PrunePredicate<Node, NodeKey>&         a_prunePredicate,
                                            const BVH::PackedChildOrderer<NodeKey, K>&        a_childOrderer,
                                            const BVH::NodeKeyFactory<Node, NodeKey>& a_nodeKeyFactory) const noexcept
{
  std::array<std::pair<uint32_t, NodeKey>, K> children;

  // Vector-backed stack avoids deque chunk allocations; reserve avoids reallocs.
  std::vector<std::pair<uint32_t, NodeKey>> q;

  q.reserve(64);
  q.emplace_back(static_cast<uint32_t>(0), a_nodeKeyFactory(m_linearNodes[0]));

  while (!q.empty()) {
    const uint32_t nodeIdx = q.back().first;
    const NodeKey  nodeKey = q.back().second;
    q.pop_back();

    const Node& node = m_linearNodes[nodeIdx];

    if (a_prunePredicate(node, nodeKey)) {
      if (node.isLeaf()) {
        a_leafEvaluator(m_primitives, node.getPrimitivesOffset(), node.getNumPrimitives());
      }
      else {
        for (size_t k = 0; k < K; k++) {
          const uint32_t childIdx = node.getChildOffsets()[k];
          children[k].first       = childIdx;
          children[k].second      = a_nodeKeyFactory(m_linearNodes[childIdx]);
        }

        a_childOrderer(children);

        for (const auto& child : children) {
          q.push_back(child);
        }
      }
    }
  }
}

template <class T, class P, size_t K, class StoragePolicy>
template <class State, class LeafEvaluator, class PruneDistSquared>
inline void
PackedBVH<T, P, K, StoragePolicy>::pruneTraverse(const Vec3T<T>&    a_point,
                                                 State&             a_state,
                                                 LeafEvaluator&&    a_evalLeaf,
                                                 PruneDistSquared&& a_pruneDist2) const noexcept
{
  struct StackEntry
  {
    uint32_t idx;
    T        dist2;
  };

  // ──────────────────────────────────────────────────────────────────────────────
  // AVX-512F paths: K==8/double and K==16/float.
  //
  // When compiled with -mavx512f these are selected in preference to the AVX
  // paths below because they appear first and each path ends with a return.
  // The compiler will dead-code-eliminate the corresponding AVX branches.
  //
  // Alignment: ChildAABBSoA uses alignas(sizeof(T)*K), which equals 64 bytes
  // for both (K=8, T=double) and (K=16, T=float).  _mm512_load_pd / _mm512_load_ps
  // both require 64-byte alignment — the static_assert below catches any mismatch.
  //
  // Recommended configurations on AVX-512 hardware:
  //   float  → K=16, W=16  (one _mm512_load_ps covers all children and one leaf group)
  //   double → K=8,  W=8   (one _mm512_load_pd covers all children; AVX-512F replaces
  //                          the 2×_mm256_load_pd emulation in the AVX fallback below)
  // ──────────────────────────────────────────────────────────────────────────────
#if defined(__AVX512F__)
  if constexpr (K == 8 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm512_load_pd requires 64-byte alignment");

    const __m512d px   = _mm512_set1_pd(a_point[0]);
    const __m512d py   = _mm512_set1_pd(a_point[1]);
    const __m512d pz   = _mm512_set1_pd(a_point[2]);
    const __m512d zero = _mm512_setzero_pd();

    alignas(64) StackEntry stack[256];

    int top      = 0;
    stack[top++] = {0U, 0.0};

    while (top > 0) {
      const StackEntry entry    = stack[--top];
      const double     curBest2 = static_cast<double>(a_pruneDist2(a_state));

      if (entry.dist2 > curBest2) {
        continue;
      }

      const Node& node = m_linearNodes[entry.idx];

      if (node.isLeaf()) {
        a_evalLeaf(a_state, node.getPrimitivesOffset(), node.getNumPrimitives());
      }
      else {
        const auto& soa = m_childAabbSoA[entry.idx];

        const __m512d lo_x = _mm512_load_pd(soa.m_lo[0]);
        const __m512d lo_y = _mm512_load_pd(soa.m_lo[1]);
        const __m512d lo_z = _mm512_load_pd(soa.m_lo[2]);
        const __m512d hi_x = _mm512_load_pd(soa.m_hi[0]);
        const __m512d hi_y = _mm512_load_pd(soa.m_hi[1]);
        const __m512d hi_z = _mm512_load_pd(soa.m_hi[2]);

        const __m512d dx = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_x, px), _mm512_sub_pd(px, hi_x)));
        const __m512d dy = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_y, py), _mm512_sub_pd(py, hi_y)));
        const __m512d dz = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_z, pz), _mm512_sub_pd(pz, hi_z)));
        const __m512d d2 =
          _mm512_add_pd(_mm512_mul_pd(dx, dx), _mm512_add_pd(_mm512_mul_pd(dy, dy), _mm512_mul_pd(dz, dz)));

        alignas(64) double dist2[K];
        _mm512_store_pd(dist2, d2);

        const auto&                                offsets = node.getChildOffsets();
        std::array<std::pair<double, uint32_t>, K> children;

        for (size_t k = 0; k < K; k++) {
          children[k] = {dist2[k], offsets[k]};
        }

        std::sort(children.begin(),
                  children.end(),
                  [](const std::pair<double, uint32_t>& a, const std::pair<double, uint32_t>& b) noexcept {
                    return a.first > b.first;
                  });

        const double newBest2 = static_cast<double>(a_pruneDist2(a_state));

        for (const auto& [d, idx] : children) {
          if (d <= newBest2) {
            stack[top++] = {idx, d};
          }
        }
      }
    }

    return;
  }

  if constexpr (K == 16 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm512_load_ps requires 64-byte alignment");

    const __m512 px   = _mm512_set1_ps((float)a_point[0]);
    const __m512 py   = _mm512_set1_ps((float)a_point[1]);
    const __m512 pz   = _mm512_set1_ps((float)a_point[2]);
    const __m512 zero = _mm512_setzero_ps();

    alignas(64) StackEntry stack[256];

    int top      = 0;
    stack[top++] = {0U, 0.f};

    while (top > 0) {
      const StackEntry entry    = stack[--top];
      const float      curBest2 = static_cast<float>(a_pruneDist2(a_state));

      if (entry.dist2 > curBest2) {
        continue;
      }

      const Node& node = m_linearNodes[entry.idx];

      if (node.isLeaf()) {
        a_evalLeaf(a_state, node.getPrimitivesOffset(), node.getNumPrimitives());
      }
      else {
        const auto& soa = m_childAabbSoA[entry.idx];

        const __m512 lo_x = _mm512_load_ps(soa.m_lo[0]);
        const __m512 lo_y = _mm512_load_ps(soa.m_lo[1]);
        const __m512 lo_z = _mm512_load_ps(soa.m_lo[2]);
        const __m512 hi_x = _mm512_load_ps(soa.m_hi[0]);
        const __m512 hi_y = _mm512_load_ps(soa.m_hi[1]);
        const __m512 hi_z = _mm512_load_ps(soa.m_hi[2]);

        const __m512 dx = _mm512_max_ps(zero, _mm512_max_ps(_mm512_sub_ps(lo_x, px), _mm512_sub_ps(px, hi_x)));
        const __m512 dy = _mm512_max_ps(zero, _mm512_max_ps(_mm512_sub_ps(lo_y, py), _mm512_sub_ps(py, hi_y)));
        const __m512 dz = _mm512_max_ps(zero, _mm512_max_ps(_mm512_sub_ps(lo_z, pz), _mm512_sub_ps(pz, hi_z)));
        const __m512 d2 =
          _mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_add_ps(_mm512_mul_ps(dy, dy), _mm512_mul_ps(dz, dz)));

        alignas(64) float dist2[K];
        _mm512_store_ps(dist2, d2);

        const auto&                               offsets = node.getChildOffsets();
        std::array<std::pair<float, uint32_t>, K> children;

        for (size_t k = 0; k < K; k++) {
          children[k] = {dist2[k], offsets[k]};
        }

        std::sort(children.begin(),
                  children.end(),
                  [](const std::pair<float, uint32_t>& a, const std::pair<float, uint32_t>& b) noexcept {
                    return a.first > b.first;
                  });

        const float newBest2 = static_cast<float>(a_pruneDist2(a_state));

        for (const auto& [d, idx] : children) {
          if (d <= newBest2) {
            stack[top++] = {idx, d};
          }
        }
      }
    }

    return;
  }
#endif // __AVX512F__

  // ──────────────────────────────────────────────────────────────────────────────
  // AVX paths: K==4/double (single pass), K==8/float (single pass),
  //            K==8/double (two 4-wide passes — superseded by AVX-512F above).
  // ──────────────────────────────────────────────────────────────────────────────
#if defined(__AVX__)
  if constexpr (K == 4 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px   = _mm256_set1_pd(a_point[0]);
    const __m256d py   = _mm256_set1_pd(a_point[1]);
    const __m256d pz   = _mm256_set1_pd(a_point[2]);
    const __m256d zero = _mm256_setzero_pd();

    alignas(32) StackEntry stack[256];

    int top      = 0;
    stack[top++] = {0U, 0.0};

    while (top > 0) {
      const StackEntry entry    = stack[--top];
      const double     curBest2 = static_cast<double>(a_pruneDist2(a_state));

      if (entry.dist2 > curBest2) {
        continue;
      }

      const Node& node = m_linearNodes[entry.idx];
      if (node.isLeaf()) {
        a_evalLeaf(a_state, node.getPrimitivesOffset(), node.getNumPrimitives());
      }
      else {
        const auto& soa = m_childAabbSoA[entry.idx];

        const __m256d lo_x = _mm256_load_pd(soa.m_lo[0]);
        const __m256d lo_y = _mm256_load_pd(soa.m_lo[1]);
        const __m256d lo_z = _mm256_load_pd(soa.m_lo[2]);
        const __m256d hi_x = _mm256_load_pd(soa.m_hi[0]);
        const __m256d hi_y = _mm256_load_pd(soa.m_hi[1]);
        const __m256d hi_z = _mm256_load_pd(soa.m_hi[2]);

        const __m256d dx = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x, px), _mm256_sub_pd(px, hi_x)));
        const __m256d dy = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y, py), _mm256_sub_pd(py, hi_y)));
        const __m256d dz = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z, pz), _mm256_sub_pd(pz, hi_z)));
        const __m256d d2 =
          _mm256_add_pd(_mm256_mul_pd(dx, dx), _mm256_add_pd(_mm256_mul_pd(dy, dy), _mm256_mul_pd(dz, dz)));

        alignas(32) double dist2[K];

        _mm256_store_pd(dist2, d2);

        const auto&                                offsets = node.getChildOffsets();
        std::array<std::pair<double, uint32_t>, K> children;

        for (size_t k = 0; k < K; k++) {
          children[k] = {dist2[k], offsets[k]};
        }

        std::sort(children.begin(),
                  children.end(),
                  [](const std::pair<double, uint32_t>& a, const std::pair<double, uint32_t>& b) noexcept {
                    return a.first > b.first;
                  });

        const double newBest2 = static_cast<double>(a_pruneDist2(a_state));

        for (const auto& [d, idx] : children) {
          if (d <= newBest2)
            stack[top++] = {idx, d};
        }
      }
    }

    return;
  }
  if constexpr (K == 8 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm256_load_ps requires 32-byte alignment");

    const __m256 px   = _mm256_set1_ps((float)a_point[0]);
    const __m256 py   = _mm256_set1_ps((float)a_point[1]);
    const __m256 pz   = _mm256_set1_ps((float)a_point[2]);
    const __m256 zero = _mm256_setzero_ps();

    alignas(32) StackEntry stack[256];

    int top      = 0;
    stack[top++] = {0U, 0.f};

    while (top > 0) {
      const StackEntry entry    = stack[--top];
      const float      curBest2 = static_cast<float>(a_pruneDist2(a_state));

      if (entry.dist2 > curBest2) {
        continue;
      }

      const Node& node = m_linearNodes[entry.idx];
      if (node.isLeaf()) {
        a_evalLeaf(a_state, node.getPrimitivesOffset(), node.getNumPrimitives());
      }
      else {
        const auto& soa = m_childAabbSoA[entry.idx];

        const __m256 lo_x = _mm256_load_ps(soa.m_lo[0]);
        const __m256 lo_y = _mm256_load_ps(soa.m_lo[1]);
        const __m256 lo_z = _mm256_load_ps(soa.m_lo[2]);
        const __m256 hi_x = _mm256_load_ps(soa.m_hi[0]);
        const __m256 hi_y = _mm256_load_ps(soa.m_hi[1]);
        const __m256 hi_z = _mm256_load_ps(soa.m_hi[2]);

        const __m256 dx = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_x, px), _mm256_sub_ps(px, hi_x)));
        const __m256 dy = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_y, py), _mm256_sub_ps(py, hi_y)));
        const __m256 dz = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_z, pz), _mm256_sub_ps(pz, hi_z)));
        const __m256 d2 =
          _mm256_add_ps(_mm256_mul_ps(dx, dx), _mm256_add_ps(_mm256_mul_ps(dy, dy), _mm256_mul_ps(dz, dz)));

        alignas(32) float dist2[K];
        _mm256_store_ps(dist2, d2);

        const auto& offsets = node.getChildOffsets();

        std::array<std::pair<float, uint32_t>, K> children;

        for (size_t k = 0; k < K; k++) {
          children[k] = {dist2[k], offsets[k]};
        }

        std::sort(children.begin(),
                  children.end(),
                  [](const std::pair<float, uint32_t>& a, const std::pair<float, uint32_t>& b) noexcept {
                    return a.first > b.first;
                  });

        const float newBest2 = static_cast<float>(a_pruneDist2(a_state));

        for (const auto& [d, idx] : children) {
          if (d <= newBest2) {
            stack[top++] = {idx, d};
          }
        }
      }
    }

    return;
  }

  if constexpr (K == 8 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px   = _mm256_set1_pd(a_point[0]);
    const __m256d py   = _mm256_set1_pd(a_point[1]);
    const __m256d pz   = _mm256_set1_pd(a_point[2]);
    const __m256d zero = _mm256_setzero_pd();

    alignas(32) StackEntry stack[256];

    int top      = 0;
    stack[top++] = {0U, 0.0};

    while (top > 0) {
      const StackEntry entry    = stack[--top];
      const double     curBest2 = static_cast<double>(a_pruneDist2(a_state));

      if (entry.dist2 > curBest2) {
        continue;
      }

      const Node& node = m_linearNodes[entry.idx];

      if (node.isLeaf()) {
        a_evalLeaf(a_state, node.getPrimitivesOffset(), node.getNumPrimitives());
      }
      else {
        const auto& soa = m_childAabbSoA[entry.idx];

        const __m256d lo_x0 = _mm256_load_pd(soa.m_lo[0]);
        const __m256d lo_y0 = _mm256_load_pd(soa.m_lo[1]);
        const __m256d lo_z0 = _mm256_load_pd(soa.m_lo[2]);
        const __m256d hi_x0 = _mm256_load_pd(soa.m_hi[0]);
        const __m256d hi_y0 = _mm256_load_pd(soa.m_hi[1]);
        const __m256d hi_z0 = _mm256_load_pd(soa.m_hi[2]);

        const __m256d dx0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x0, px), _mm256_sub_pd(px, hi_x0)));
        const __m256d dy0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y0, py), _mm256_sub_pd(py, hi_y0)));
        const __m256d dz0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z0, pz), _mm256_sub_pd(pz, hi_z0)));
        const __m256d d2_0 =
          _mm256_add_pd(_mm256_mul_pd(dx0, dx0), _mm256_add_pd(_mm256_mul_pd(dy0, dy0), _mm256_mul_pd(dz0, dz0)));

        const __m256d lo_x1 = _mm256_load_pd(soa.m_lo[0] + 4);
        const __m256d lo_y1 = _mm256_load_pd(soa.m_lo[1] + 4);
        const __m256d lo_z1 = _mm256_load_pd(soa.m_lo[2] + 4);
        const __m256d hi_x1 = _mm256_load_pd(soa.m_hi[0] + 4);
        const __m256d hi_y1 = _mm256_load_pd(soa.m_hi[1] + 4);
        const __m256d hi_z1 = _mm256_load_pd(soa.m_hi[2] + 4);

        const __m256d dx1 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x1, px), _mm256_sub_pd(px, hi_x1)));
        const __m256d dy1 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y1, py), _mm256_sub_pd(py, hi_y1)));
        const __m256d dz1 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z1, pz), _mm256_sub_pd(pz, hi_z1)));
        const __m256d d2_1 =
          _mm256_add_pd(_mm256_mul_pd(dx1, dx1), _mm256_add_pd(_mm256_mul_pd(dy1, dy1), _mm256_mul_pd(dz1, dz1)));

        alignas(32) double dist2[K];
        _mm256_store_pd(dist2, d2_0);
        _mm256_store_pd(dist2 + 4, d2_1);

        const auto& offsets = node.getChildOffsets();

        std::array<std::pair<double, uint32_t>, K> children;

        for (size_t k = 0; k < K; k++) {
          children[k] = {dist2[k], offsets[k]};
        }

        std::sort(children.begin(),
                  children.end(),
                  [](const std::pair<double, uint32_t>& a, const std::pair<double, uint32_t>& b) noexcept {
                    return a.first > b.first;
                  });

        const double newBest2 = static_cast<double>(a_pruneDist2(a_state));

        for (const auto& [d, idx] : children) {
          if (d <= newBest2) {
            stack[top++] = {idx, d};
          }
        }
      }
    }

    return;
  }
#endif
#if defined(__SSE4_1__)
  if constexpr (K == 4 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm_load_ps requires 16-byte alignment");

    const __m128 px   = _mm_set1_ps(a_point[0]);
    const __m128 py   = _mm_set1_ps(a_point[1]);
    const __m128 pz   = _mm_set1_ps(a_point[2]);
    const __m128 zero = _mm_setzero_ps();

    alignas(16) StackEntry stack[256];
    int                    top = 0;
    stack[top++]               = {0U, 0.0f};

    while (top > 0) {
      const StackEntry entry    = stack[--top];
      const float      curBest2 = static_cast<float>(a_pruneDist2(a_state));

      if (entry.dist2 > curBest2) {
        continue;
      }

      const Node& node = m_linearNodes[entry.idx];

      if (node.isLeaf()) {
        a_evalLeaf(a_state, node.getPrimitivesOffset(), node.getNumPrimitives());
      }
      else {
        const auto& soa = m_childAabbSoA[entry.idx];

        const __m128 lo_x = _mm_load_ps(soa.m_lo[0]);
        const __m128 lo_y = _mm_load_ps(soa.m_lo[1]);
        const __m128 lo_z = _mm_load_ps(soa.m_lo[2]);
        const __m128 hi_x = _mm_load_ps(soa.m_hi[0]);
        const __m128 hi_y = _mm_load_ps(soa.m_hi[1]);
        const __m128 hi_z = _mm_load_ps(soa.m_hi[2]);

        const __m128 dx = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_x, px), _mm_sub_ps(px, hi_x)));
        const __m128 dy = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_y, py), _mm_sub_ps(py, hi_y)));
        const __m128 dz = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_z, pz), _mm_sub_ps(pz, hi_z)));
        const __m128 d2 = _mm_add_ps(_mm_mul_ps(dx, dx), _mm_add_ps(_mm_mul_ps(dy, dy), _mm_mul_ps(dz, dz)));

        alignas(16) float dist2[K];
        _mm_store_ps(dist2, d2);

        const auto& offsets = node.getChildOffsets();

        std::array<std::pair<float, uint32_t>, K> children;

        for (size_t k = 0; k < K; k++) {
          children[k] = {dist2[k], offsets[k]};
        }

        std::sort(children.begin(),
                  children.end(),
                  [](const std::pair<float, uint32_t>& a, const std::pair<float, uint32_t>& b) noexcept {
                    return a.first > b.first;
                  });

        const float newBest2 = static_cast<float>(a_pruneDist2(a_state));

        for (const auto& [d, idx] : children) {
          if (d <= newBest2) {
            stack[top++] = {idx, d};
          }
        }
      }
    }

    return;
  }
#endif

  // Scalar fallback for all other (T, K) combinations.
  const BVH::PackedLeafEvaluator<P, StoragePolicy> leafEvaluator =
    [&a_state, &a_evalLeaf](const std::vector<StorageType>&, size_t offset, size_t count) noexcept -> void {
    a_evalLeaf(a_state, offset, count);
  };

  // The node key is the *squared* box distance (getDistanceToBoundingVolume2), matching the squared
  // pruning bound a_pruneDist2 returns -- so the predicate compares directly, with no sqrt anywhere.
  // (Squaring is monotonic, so ordering children by squared distance is identical to ordering by
  // distance.) The SIMD paths above already work entirely in squared distance; this keeps the
  // scalar fallback consistent instead of taking a square root only to square it back.
  const BVH::PrunePredicate<Node, T> prunePredicate =
    [&a_state, &a_pruneDist2](const Node& /*n*/, const T& d2) noexcept -> bool { return d2 <= a_pruneDist2(a_state); };

  const BVH::PackedChildOrderer<T, K> childOrderer = [](std::array<std::pair<uint32_t, T>, K>& ch) noexcept -> void {
    std::sort(ch.begin(), ch.end(), [](const std::pair<uint32_t, T>& a, const std::pair<uint32_t, T>& b) noexcept {
      return a.second > b.second;
    });
  };

  const BVH::NodeKeyFactory<Node, T> nodeKeyFactory = [&a_point](const Node& n) noexcept -> T {
    return n.getDistanceToBoundingVolume2(a_point);
  };

  this->traverse(leafEvaluator, prunePredicate, childOrderer, nodeKeyFactory);
}

} // namespace BVH

} // namespace EBGeometry

#endif
