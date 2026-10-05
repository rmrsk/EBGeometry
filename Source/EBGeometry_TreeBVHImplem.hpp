// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TreeBVHImplem.hpp
 * @brief  Implementation of EBGeometry_TreeBVH.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TREEBVHIMPLEM_HPP
#define EBGEOMETRY_TREEBVHIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <stack>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_TreeBVH.hpp"

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
  // Copy overload (lvalue callers, e.g. the direct top-down constructor's per-node probe): copy each
  // element straight into the members -- no intermediate parameter vector, no extra move pass.
  m_primitives.reserve(a_primsAndBVs.size());
  m_boundingVolumes.reserve(a_primsAndBVs.size());

  for (const auto& pbv : a_primsAndBVs) {
    m_primitives.emplace_back(pbv.first);
    m_boundingVolumes.emplace_back(pbv.second);
  }

  m_boundingVolume = BV(m_boundingVolumes);
  m_partitioned    = false;
}

template <class T, class P, class BV, size_t K>
inline TreeBVH<T, P, BV, K>::TreeBVH(std::vector<PrimAndBV<P, BV>>&& a_primsAndBVs) : TreeBVH<T, P, BV, K>()
{
  // Move overload (rvalue callers, e.g. topDownSortAndPartition moving a partitioner's sub-list into a
  // child): transfer each element in without copying or shared_ptr refcount churn.
  m_primitives.reserve(a_primsAndBVs.size());
  m_boundingVolumes.reserve(a_primsAndBVs.size());
  for (auto& pbv : a_primsAndBVs) {
    m_primitives.emplace_back(std::move(pbv.first));
    m_boundingVolumes.emplace_back(std::move(pbv.second));
  }

  m_boundingVolume = BV(m_boundingVolumes);
  m_partitioned    = false;
}

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
inline const Array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>&
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

    // Pack primitives and BVs, moving them out of this node (it is about to stop being a leaf, so its
    // own lists are cleared below) -- no shared_ptr refcount churn.
    PrimAndBVList<P, BV> primsAndBVs;

    primsAndBVs.reserve(numPrimsInThisNode);

    for (size_t i = 0; i < numPrimsInThisNode; i++) {
      primsAndBVs.emplace_back(std::move(m_primitives[i]), std::move(m_boundingVolumes[i]));
    }

    // This is no longer a leaf node.
    m_primitives.resize(0);
    m_boundingVolumes.resize(0);

    // Partition into sub-sets (the partitioner takes the list by value and moves the sub-lists out),
    // then move each sub-list into its child node.
    Array<PrimAndBVList<P, BV>, K> newPartitions = a_partitioner(std::move(primsAndBVs));

    detail::requireNonEmptyPartitions(newPartitions, numPrimsInThisNode, "BVH::TreeBVH::topDownSortAndPartition");

    for (size_t c = 0; c < K; c++) {
      m_children[c] = std::make_shared<TreeBVH<T, P, BV, K>>(std::move(newPartitions[c]));
    }

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
TreeBVH<T, P, BV, K>::bottomUpSortAndPartition(const size_t a_targetLeafSize)
{
  EBGEOMETRY_REQUIRE(a_targetLeafSize > 0, "TreeBVH::bottomUpSortAndPartition: the target leaf size must be positive");

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
  // tree where all the leaves exist on the same level, so the number of leaves is a power of K and
  // merging in groups of K ends in a single root node.
  //
  // The depth is the smallest d whose leaves hold at most a_targetLeafSize primitives, capped at the
  // largest d with K^d <= N, counted in integers: floor(log(N) / log(K)) in floating point comes out
  // one short at exact powers of K, and is undefined for N = 0. A target of 1 always reaches the cap.
  const size_t numPrimitives = sortedPrimitives.size();

  size_t treeDepth = 0;
  size_t numLeaves = 1;

  while (numLeaves <= numPrimitives / K && (numPrimitives + numLeaves - 1) / numLeaves > a_targetLeafSize) {
    numLeaves *= K;
    treeDepth++;
  }

  const size_t primsPerLeaf = numPrimitives / numLeaves;

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

        Array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K> children;

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
TreeBVH<T, P, BV, K>::setChildren(const Array<std::shared_ptr<TreeBVH<T, P, BV, K>>, K>& a_children) noexcept
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
  Array<std::pair<std::shared_ptr<const Node>, NodeKey>, K>   children;
  std::stack<std::pair<std::shared_ptr<const Node>, NodeKey>> q;

  q.emplace(this->shared_from_this(), a_nodeKeyFactory(*this));

  while (!(q.empty())) {
    // Copied, not referenced: pop() destroys the entry, and with it a reference's target.
    const auto node    = q.top().first;
    const auto nodeKey = q.top().second;

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
template <class BVConstructor>
inline const BV&
TreeBVH<T, P, BV, K>::refit(const BVConstructor& a_bvConstructor)
{
  if (this->isLeaf()) {
    for (size_t i = 0; i < m_primitives.size(); i++) {
      m_boundingVolumes[i] = a_bvConstructor(*m_primitives[i]);
    }

    m_boundingVolume = BV(m_boundingVolumes);
  }
  else {
    std::vector<BV> childBoundingVolumes;
    childBoundingVolumes.reserve(K);

    for (const auto& child : m_children) {
      childBoundingVolumes.emplace_back(child->refit(a_bvConstructor));
    }

    m_boundingVolume = BV(childBoundingVolumes);
  }

  return m_boundingVolume;
}

template <class T, class P, class BV, size_t K>
inline std::shared_ptr<PackedBVH<T, P, K>>
TreeBVH<T, P, BV, K>::pack(Pool& a_pool) const
{
  static_assert(std::is_same_v<BV, EBGeometry::BoundingVolumes::AABBT<T>>, "TreeBVH::pack requires BV == AABBT<T>");

  return std::make_shared<PackedBVH<T, P, K>>(a_pool, *this);
}

template <class T, class P, class BV, size_t K>
template <class Q, class Converter>
inline std::shared_ptr<PackedBVH<T, Q, K>>
TreeBVH<T, P, BV, K>::packWith(Pool& a_pool, Converter&& a_converter) const
{
  static_assert(std::is_same_v<BV, EBGeometry::BoundingVolumes::AABBT<T>>, "TreeBVH::packWith requires BV == AABBT<T>");

  return std::make_shared<PackedBVH<T, Q, K>>(a_pool, *this, std::forward<Converter>(a_converter));
}

} // namespace BVH

} // namespace EBGeometry

#endif
