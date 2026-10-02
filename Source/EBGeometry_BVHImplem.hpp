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
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Math.hpp"

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
  // tree where all the leaves exist on the same level, so the number of leaves is a power of K and
  // merging in groups of K ends in a single root node.
  //
  // The depth is the largest d with K^d <= N, counted in integers: floor(log(N) / log(K)) in floating
  // point comes out one short at exact powers of K, and is undefined for N = 0.
  const size_t numPrimitives = sortedPrimitives.size();

  size_t treeDepth = 0;
  size_t numLeaves = 1;

  while (numLeaves <= numPrimitives / K) {
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

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::attachTo(Pool& a_pool) noexcept
{
  // Every array of one BVH must come from the same pool -- offsets from two different blocks cannot
  // both resolve against one base.
  m_location.attach(a_pool, "BVH::PackedBVH");
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline const void*
PackedBVH<T, P, K>::base() const noexcept
{
  return m_location.base();
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline bool
PackedBVH<T, P, K>::isAttachedTo(const Pool& a_pool) const noexcept
{
  return m_location.isAttachedTo(a_pool);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>
PackedBVH<T, P, K>::rebasedView(const Pool& a_pool) const noexcept
{
  const uint64_t endByte =
    Math::max(m_linearNodes.endByte(), Math::max(m_primitives.endByte(), m_childAabbSoA.endByte()));

  PackedBVH view = *this;

  view.m_location = m_location.rebasedOnto(a_pool, endByte, "BVH::PackedBVH");

  if (view.m_location.m_control == nullptr) {
    // A snapshot is what a kernel receives. The device traversal stack is smaller than the host's, so
    // a tree that finalize() accepted can still be too deep to traverse on device. This is the moment
    // the caller commits to that, and the last one that still runs on the host where it can say so.
    this->requireDepthFits(this->base(), DeviceTraversalDepth, "device view");
  }

  return view;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PoolLocation
PackedBVH<T, P, K>::location() const noexcept
{
  return m_location;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PackedBVH<T, P, K>
PackedBVH<T, P, K>::relocatedTo(const PoolLocation& a_location) const noexcept
{
  PackedBVH<T, P, K> view = *this;

  view.m_location = a_location;

  return view;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline PackedBVH<T, P, K>
PackedBVH<T, P, K>::deepCopy(Pool& a_dstPool) const
{
  const void* srcBase = this->base();

  std::vector<Node> nodes(m_linearNodes.size());
  std::vector<P>    prims(m_primitives.size());

  for (uint32_t i = 0; i < m_linearNodes.size(); i++) {
    nodes[i] = m_linearNodes.at(srcBase, i);
  }

  for (uint32_t i = 0; i < m_primitives.size(); i++) {
    prims[i] = m_primitives.at(srcBase, i);
  }

  PackedBVH copy = *this;

  copy.m_location     = PoolLocation{};
  copy.m_linearNodes  = PODVector<Node>{};
  copy.m_primitives   = PODVector<P>{};
  copy.m_childAabbSoA = PODVector<ChildAABBSoA>{};

  copy.finalize(a_dstPool, nodes, prims);

  return copy;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::finalize(Pool& a_pool, const std::vector<Node>& a_linearNodes, const std::vector<P>& a_primitives)
{
  // Every build path ends here, so this checks the library's own builders as well as adopted arrays:
  // a malformed array -- a leaf with no primitives read as an interior node, say -- would otherwise
  // surface as an out-of-bounds read in Release.
  PackedBVH::requireWellFormed(a_linearNodes, a_primitives.size());

  this->attachTo(a_pool);

  m_linearNodes.reserveFrom(a_pool, static_cast<uint32_t>(a_linearNodes.size()));
  m_primitives.reserveFrom(a_pool, static_cast<uint32_t>(a_primitives.size()));

  // Resolve the base only after every reservation above: a reserve can grow the pool, which moves
  // the block and invalidates any address taken before it.
  void* poolBase = const_cast<void*>(this->base());

  m_linearNodes.assign(poolBase, a_linearNodes.data(), static_cast<uint32_t>(a_linearNodes.size()));
  m_primitives.assign(poolBase, a_primitives.data(), static_cast<uint32_t>(a_primitives.size()));

  this->buildSoA(&a_pool);

  // buildSoA() reserves, so re-resolve; after it nothing else moves the block. Reject here, once,
  // rather than letting pruneTraverse walk off its fixed stack later with no way to notice.
  this->requireDepthFits(this->base(), HostTraversalDepth, "host build");
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::buildSoA(Pool* a_pool)
{
  // Only interior nodes have children, so only they get a row; a leaf, typically most of the nodes,
  // gets none.
  uint32_t numInterior = 0;

  for (uint32_t i = 0; i < m_linearNodes.size(); i++) {
    if (!m_linearNodes.at(this->base(), i).isLeaf()) {
      numInterior++;
    }
  }

  if (a_pool != nullptr) {
    m_childAabbSoA.reserveFrom(*a_pool, numInterior);
  }

  EBGEOMETRY_EXPECT(m_childAabbSoA.m_capacity == numInterior);

  // Resolved after the reservation above, for the reason given in finalize().
  void* poolBase = const_cast<void*>(this->base());

  // Assembled host-side and copied in one shot, like the node and primitive arrays: a PODVector
  // never reallocates, so it has no way to be filled element-by-element without its final size
  // already fixed, and this keeps the one finalize pattern everywhere.
  std::vector<ChildAABBSoA> soaCache(numInterior);

  uint32_t row = 0;

  for (uint32_t i = 0; i < m_linearNodes.size(); i++) {
    Node& node = m_linearNodes.at(poolBase, i);

    if (!node.isLeaf()) {
      const auto& offsets = node.getChildOffsets();
      auto&       soa     = soaCache[row];

      node.m_childBoxRow = row++;

      for (size_t k = 0; k < K; k++) {
        const auto& bv = m_linearNodes.at(poolBase, offsets[k]).getBoundingVolume();
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

  m_childAabbSoA.assign(poolBase, soaCache.data(), numInterior);
}

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

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<const P>
PackedBVH<T, P, K>::getPrimitives() const noexcept
{
  return m_primitives.bind(this->base());
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<P>
PackedBVH<T, P, K>::getPrimitives() noexcept
{
  return m_primitives.bind(const_cast<void*>(this->base()));
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<const typename PackedBVH<T, P, K>::Node>
PackedBVH<T, P, K>::getNodes() const noexcept
{
  return m_linearNodes.bind(this->base());
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline const EBGeometry::BoundingVolumes::AABBT<T>&
PackedBVH<T, P, K>::getBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_linearNodes.size() > 0);

  return m_linearNodes.at(this->base(), 0).getBoundingVolume();
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline EBGeometry::BoundingVolumes::AABBT<T>
PackedBVH<T, P, K>::computeBoundingVolume() const noexcept
{
  if (m_linearNodes.size() == 0) {
    return EBGeometry::BoundingVolumes::AABBT<T>();
  }

  return m_linearNodes.at(this->base(), 0).getBoundingVolume();
}

template <class T, class P, size_t K>
template <class NodeKey, class LeafEvaluator, class PrunePredicate, class ChildOrderer, class NodeKeyFactory>
EBGEOMETRY_HOST_DEVICE
inline void
PackedBVH<T, P, K>::traverse(LeafEvaluator&&  a_leafEvaluator,
                             PrunePredicate&& a_prunePredicate,
                             ChildOrderer&&   a_childOrderer,
                             NodeKeyFactory&& a_nodeKeyFactory) const noexcept
{
  // The key type: as given, or what the node-key factory returns.
  using Key = std::
    conditional_t<std::is_void_v<NodeKey>, std::decay_t<std::invoke_result_t<NodeKeyFactory&, const Node&>>, NodeKey>;
  using Entry = BVH::NodeAndKey<Key>;

  // An empty BVH has no root to descend from.
  if (m_linearNodes.size() == 0) {
    return;
  }

  // One resolution for the whole traversal. Safe because a query performs no reservation, so the
  // base cannot move underneath it.
  const void* poolBase = this->base();

  // Every child of an expanded node is pushed, so a tree of depth D needs at most 1 + (K-1)(D-1)
  // entries -- the bound traversalStackDepth() gives, and that every build checked the tree against.
  constexpr size_t stackDepth = PackedBVH::traversalStackDepth();

  Entry  stack[stackDepth];
  size_t top = 0;

  stack[top++] = Entry{0U, static_cast<Key>(a_nodeKeyFactory(m_linearNodes.at(poolBase, 0)))};

  Array<Entry, K> children;

  while (top > 0) {
    const Entry entry = stack[--top];
    const Node& node  = m_linearNodes.at(poolBase, entry.first);

    if (!a_prunePredicate(node, entry.second)) {
      continue;
    }

    if (node.isLeaf()) {
      a_leafEvaluator(m_primitives.bind(poolBase), size_t(node.getPrimitivesOffset()), size_t(node.getNumPrimitives()));
    }
    else {
      for (size_t k = 0; k < K; k++) {
        const uint32_t childIdx = node.getChildOffsets()[k];

        children[k] = Entry{childIdx, static_cast<Key>(a_nodeKeyFactory(m_linearNodes.at(poolBase, childIdx)))};
      }

      a_childOrderer(children);

      for (size_t k = 0; k < K; k++) {
        EBGEOMETRY_EXPECT(top < stackDepth);

        stack[top++] = children[k];
      }
    }
  }
}

template <class T, class P, size_t K>
inline size_t
PackedBVH<T, P, K>::maxNodeDepth(const void* a_base) const
{
  if (m_linearNodes.size() == 0) {
    return 0;
  }

  size_t maxDepth = 0;

  // (node index, depth of that node). A host-side build/mirror step, so std::vector is fine here --
  // this is precisely the heap stack pruneTraverse itself can no longer use.
  std::vector<std::pair<uint32_t, size_t>> stack;

  stack.reserve(64);
  stack.emplace_back(uint32_t(0), size_t(1));

  while (!stack.empty()) {
    const uint32_t idx   = stack.back().first;
    const size_t   depth = stack.back().second;

    stack.pop_back();

    if (depth > maxDepth) {
      maxDepth = depth;
    }

    const Node& node = m_linearNodes.at(a_base, idx);

    if (!node.isLeaf()) {
      const auto& offsets = node.getChildOffsets();

      for (size_t k = 0; k < K; k++) {
        stack.emplace_back(offsets[k], depth + 1);
      }
    }
  }

  return maxDepth;
}

template <class T, class P, size_t K>
inline void
PackedBVH<T, P, K>::requireDepthFits(const void* a_base, const size_t a_maxDepth, const char* a_context) const
{
  const size_t depth = this->maxNodeDepth(a_base);

  EBGEOMETRY_REQUIRE(depth <= a_maxDepth,
                     "BVH::PackedBVH: %s -- the tree is %zu levels deep, more than the %zu levels a traversal stack "
                     "holds.\n"
                     "  pruneTraverse would overflow its fixed stack, which Release builds do not detect. Rebuild "
                     "with a larger leaf size, or a partitioner that splits more evenly, before querying this BVH.",
                     a_context,
                     depth,
                     a_maxDepth);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline float
PackedBVH<T, P, K>::lowerBound(const T a_dist2) noexcept
{
  if constexpr (std::is_same_v<T, float>) {
    return a_dist2;
  }
  else {
    // A double narrows to the nearest float, which may be above it. Shrinking the value by 2^-22
    // first keeps the rounded float below the original, since rounding moves it by at most 2^-24
    // relative -- except for results that would be float subnormals, which go to zero, and values
    // past the float range, which go to the largest float.
    if (!(a_dist2 >= T(Math::Limits<float>::min()))) {
      return 0.0F;
    }

    if (a_dist2 >= T(Math::Limits<float>::max())) {
      return Math::Limits<float>::max();
    }

    return static_cast<float>(a_dist2 * (T(1) - T(1.0 / 4194304.0)));
  }
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
PackedBVH<T, P, K>::requireWellFormed(const std::vector<Node>& a_linearNodes, const size_t a_numPrimitives)
{
  const size_t numNodes = a_linearNodes.size();

  // Report the first defect and stop: once one offset is wrong, later ones say nothing reliable. The
  // array must be a depth-first pre-order flattening: every child strictly after its parent and
  // inside the array, every leaf's primitives inside the primitive array.
  if (numNodes == 0) {
    EBGEOMETRY_REQUIRE(a_numPrimitives == 0,
                       "BVH::PackedBVH: adopted node array is malformed -- it is empty, but the primitive "
                       "array holds %zu primitives",
                       a_numPrimitives);

    return;
  }

  // Every node but the root has exactly one parent, so a traversal reaches each primitive once.
  std::vector<bool> hasParent(numNodes, false);

  for (size_t i = 0; i < numNodes; i++) {
    const Node& node = a_linearNodes[i];

    if (node.isLeaf()) {
      const size_t end = size_t(node.getPrimitivesOffset()) + size_t(node.getNumPrimitives());

      EBGEOMETRY_REQUIRE(end <= a_numPrimitives,
                         "BVH::PackedBVH: node array is malformed -- leaf %zu's primitive range ends at "
                         "%zu, past the primitive array's %zu primitives",
                         i,
                         end,
                         a_numPrimitives);

      continue;
    }

    // An interior node. A node meant as a leaf but holding no primitives lands here too, with its
    // child offsets unset (zero), and the first check below rejects it.
    for (const uint32_t child : node.getChildOffsets()) {
      EBGEOMETRY_REQUIRE(child > i && child < numNodes,
                         "BVH::PackedBVH: node array is malformed -- node %zu has child offset %zu, which "
                         "is not strictly after its parent and inside the array of %zu nodes (a leaf with no "
                         "primitives reads as an interior node like this)",
                         i,
                         size_t(child),
                         numNodes);

      EBGEOMETRY_REQUIRE(!hasParent[child],
                         "BVH::PackedBVH: node array is malformed -- node %zu names child %zu, which already has "
                         "another parent",
                         i,
                         size_t(child));

      hasParent[child] = true;
    }
  }

  for (size_t i = 1; i < numNodes; i++) {
    EBGEOMETRY_REQUIRE(hasParent[i], "BVH::PackedBVH: node array is malformed -- node %zu has no parent", i);
  }
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline void
PackedBVH<T, P, K>::computeChildDistances2(const ChildAABBSoA& a_soa, const Vec3T<T>& a_point, T (&a_dist2)[K]) noexcept
{
  // Device code takes the scalar path unconditionally: the x86 ISA macros below may still be
  // defined during nvcc's/hipcc's *host* pass over this same __host__ __device__ function, so the
  // guard has to be on the compilation pass rather than on the intrinsics being available.
#if !defined(EBGEOMETRY_DEVICE_COMPILE)

  // ──────────────────────────────────────────────────────────────────────────────
  // AVX-512F paths: K==8/double and K==16/float.
  //
  // These appear first and return, so on -mavx512f hardware the compiler dead-code-eliminates the
  // corresponding AVX branches below.
  //
  // Alignment: ChildAABBSoA is alignas(sizeof(T)*K), which is 64 bytes for both (K=8, T=double) and
  // (K=16, T=float) -- exactly what _mm512_load_pd / _mm512_load_ps require. The static_asserts
  // catch any mismatch. The *store* into a_dist2 is unaligned (storeu), so callers need not align
  // their output buffer; on an aligned address storeu costs the same as store on every CPU that has
  // AVX-512 at all.
  //
  // Recommended configurations on AVX-512 hardware:
  //   float  -> K=16, W=16  (one _mm512_load_ps covers all children and one leaf group)
  //   double -> K=8,  W=8   (one _mm512_load_pd covers all children; AVX-512F replaces
  //                          the 2x_mm256_load_pd emulation in the AVX fallback below)
  // ──────────────────────────────────────────────────────────────────────────────
#if defined(__AVX512F__)
  if constexpr (K == 8 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm512_load_pd requires 64-byte alignment");

    const __m512d px   = _mm512_set1_pd(a_point[0]);
    const __m512d py   = _mm512_set1_pd(a_point[1]);
    const __m512d pz   = _mm512_set1_pd(a_point[2]);
    const __m512d zero = _mm512_setzero_pd();

    const __m512d lo_x = _mm512_load_pd(a_soa.m_lo[0]);
    const __m512d lo_y = _mm512_load_pd(a_soa.m_lo[1]);
    const __m512d lo_z = _mm512_load_pd(a_soa.m_lo[2]);
    const __m512d hi_x = _mm512_load_pd(a_soa.m_hi[0]);
    const __m512d hi_y = _mm512_load_pd(a_soa.m_hi[1]);
    const __m512d hi_z = _mm512_load_pd(a_soa.m_hi[2]);

    const __m512d dx = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_x, px), _mm512_sub_pd(px, hi_x)));
    const __m512d dy = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_y, py), _mm512_sub_pd(py, hi_y)));
    const __m512d dz = _mm512_max_pd(zero, _mm512_max_pd(_mm512_sub_pd(lo_z, pz), _mm512_sub_pd(pz, hi_z)));
    const __m512d d2 =
      _mm512_add_pd(_mm512_mul_pd(dx, dx), _mm512_add_pd(_mm512_mul_pd(dy, dy), _mm512_mul_pd(dz, dz)));

    _mm512_storeu_pd(a_dist2, d2);

    return;
  }

  if constexpr (K == 16 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm512_load_ps requires 64-byte alignment");

    const __m512 px   = _mm512_set1_ps((float)a_point[0]);
    const __m512 py   = _mm512_set1_ps((float)a_point[1]);
    const __m512 pz   = _mm512_set1_ps((float)a_point[2]);
    const __m512 zero = _mm512_setzero_ps();

    const __m512 lo_x = _mm512_load_ps(a_soa.m_lo[0]);
    const __m512 lo_y = _mm512_load_ps(a_soa.m_lo[1]);
    const __m512 lo_z = _mm512_load_ps(a_soa.m_lo[2]);
    const __m512 hi_x = _mm512_load_ps(a_soa.m_hi[0]);
    const __m512 hi_y = _mm512_load_ps(a_soa.m_hi[1]);
    const __m512 hi_z = _mm512_load_ps(a_soa.m_hi[2]);

    const __m512 dx = _mm512_max_ps(zero, _mm512_max_ps(_mm512_sub_ps(lo_x, px), _mm512_sub_ps(px, hi_x)));
    const __m512 dy = _mm512_max_ps(zero, _mm512_max_ps(_mm512_sub_ps(lo_y, py), _mm512_sub_ps(py, hi_y)));
    const __m512 dz = _mm512_max_ps(zero, _mm512_max_ps(_mm512_sub_ps(lo_z, pz), _mm512_sub_ps(pz, hi_z)));
    const __m512 d2 = _mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_add_ps(_mm512_mul_ps(dy, dy), _mm512_mul_ps(dz, dz)));

    _mm512_storeu_ps(a_dist2, d2);

    return;
  }
#endif // __AVX512F__

  // ──────────────────────────────────────────────────────────────────────────────
  // AVX paths: K==4/double (single pass), K==8/float (single pass),
  //            K==8/double (two 4-wide passes -- superseded by AVX-512F above).
  // ──────────────────────────────────────────────────────────────────────────────
#if defined(__AVX__)
  if constexpr (K == 4 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px   = _mm256_set1_pd(a_point[0]);
    const __m256d py   = _mm256_set1_pd(a_point[1]);
    const __m256d pz   = _mm256_set1_pd(a_point[2]);
    const __m256d zero = _mm256_setzero_pd();

    const __m256d lo_x = _mm256_load_pd(a_soa.m_lo[0]);
    const __m256d lo_y = _mm256_load_pd(a_soa.m_lo[1]);
    const __m256d lo_z = _mm256_load_pd(a_soa.m_lo[2]);
    const __m256d hi_x = _mm256_load_pd(a_soa.m_hi[0]);
    const __m256d hi_y = _mm256_load_pd(a_soa.m_hi[1]);
    const __m256d hi_z = _mm256_load_pd(a_soa.m_hi[2]);

    const __m256d dx = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x, px), _mm256_sub_pd(px, hi_x)));
    const __m256d dy = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y, py), _mm256_sub_pd(py, hi_y)));
    const __m256d dz = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z, pz), _mm256_sub_pd(pz, hi_z)));
    const __m256d d2 =
      _mm256_add_pd(_mm256_mul_pd(dx, dx), _mm256_add_pd(_mm256_mul_pd(dy, dy), _mm256_mul_pd(dz, dz)));

    _mm256_storeu_pd(a_dist2, d2);

    return;
  }

  if constexpr (K == 8 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm256_load_ps requires 32-byte alignment");

    const __m256 px   = _mm256_set1_ps((float)a_point[0]);
    const __m256 py   = _mm256_set1_ps((float)a_point[1]);
    const __m256 pz   = _mm256_set1_ps((float)a_point[2]);
    const __m256 zero = _mm256_setzero_ps();

    const __m256 lo_x = _mm256_load_ps(a_soa.m_lo[0]);
    const __m256 lo_y = _mm256_load_ps(a_soa.m_lo[1]);
    const __m256 lo_z = _mm256_load_ps(a_soa.m_lo[2]);
    const __m256 hi_x = _mm256_load_ps(a_soa.m_hi[0]);
    const __m256 hi_y = _mm256_load_ps(a_soa.m_hi[1]);
    const __m256 hi_z = _mm256_load_ps(a_soa.m_hi[2]);

    const __m256 dx = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_x, px), _mm256_sub_ps(px, hi_x)));
    const __m256 dy = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_y, py), _mm256_sub_ps(py, hi_y)));
    const __m256 dz = _mm256_max_ps(zero, _mm256_max_ps(_mm256_sub_ps(lo_z, pz), _mm256_sub_ps(pz, hi_z)));
    const __m256 d2 = _mm256_add_ps(_mm256_mul_ps(dx, dx), _mm256_add_ps(_mm256_mul_ps(dy, dy), _mm256_mul_ps(dz, dz)));

    _mm256_storeu_ps(a_dist2, d2);

    return;
  }

  if constexpr (K == 8 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px   = _mm256_set1_pd(a_point[0]);
    const __m256d py   = _mm256_set1_pd(a_point[1]);
    const __m256d pz   = _mm256_set1_pd(a_point[2]);
    const __m256d zero = _mm256_setzero_pd();

    const __m256d lo_x0 = _mm256_load_pd(a_soa.m_lo[0]);
    const __m256d lo_y0 = _mm256_load_pd(a_soa.m_lo[1]);
    const __m256d lo_z0 = _mm256_load_pd(a_soa.m_lo[2]);
    const __m256d hi_x0 = _mm256_load_pd(a_soa.m_hi[0]);
    const __m256d hi_y0 = _mm256_load_pd(a_soa.m_hi[1]);
    const __m256d hi_z0 = _mm256_load_pd(a_soa.m_hi[2]);

    const __m256d dx0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x0, px), _mm256_sub_pd(px, hi_x0)));
    const __m256d dy0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y0, py), _mm256_sub_pd(py, hi_y0)));
    const __m256d dz0 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z0, pz), _mm256_sub_pd(pz, hi_z0)));
    const __m256d d2_0 =
      _mm256_add_pd(_mm256_mul_pd(dx0, dx0), _mm256_add_pd(_mm256_mul_pd(dy0, dy0), _mm256_mul_pd(dz0, dz0)));

    const __m256d lo_x1 = _mm256_load_pd(a_soa.m_lo[0] + 4);
    const __m256d lo_y1 = _mm256_load_pd(a_soa.m_lo[1] + 4);
    const __m256d lo_z1 = _mm256_load_pd(a_soa.m_lo[2] + 4);
    const __m256d hi_x1 = _mm256_load_pd(a_soa.m_hi[0] + 4);
    const __m256d hi_y1 = _mm256_load_pd(a_soa.m_hi[1] + 4);
    const __m256d hi_z1 = _mm256_load_pd(a_soa.m_hi[2] + 4);

    const __m256d dx1 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_x1, px), _mm256_sub_pd(px, hi_x1)));
    const __m256d dy1 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_y1, py), _mm256_sub_pd(py, hi_y1)));
    const __m256d dz1 = _mm256_max_pd(zero, _mm256_max_pd(_mm256_sub_pd(lo_z1, pz), _mm256_sub_pd(pz, hi_z1)));
    const __m256d d2_1 =
      _mm256_add_pd(_mm256_mul_pd(dx1, dx1), _mm256_add_pd(_mm256_mul_pd(dy1, dy1), _mm256_mul_pd(dz1, dz1)));

    _mm256_storeu_pd(a_dist2, d2_0);
    _mm256_storeu_pd(a_dist2 + 4, d2_1);

    return;
  }
#endif // __AVX__

#if defined(__SSE4_1__)
  if constexpr (K == 4 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildAABBSoA) == sizeof(T) * K,
                  "ChildAABBSoA alignment mismatch: _mm_load_ps requires 16-byte alignment");

    const __m128 px   = _mm_set1_ps((float)a_point[0]);
    const __m128 py   = _mm_set1_ps((float)a_point[1]);
    const __m128 pz   = _mm_set1_ps((float)a_point[2]);
    const __m128 zero = _mm_setzero_ps();

    const __m128 lo_x = _mm_load_ps(a_soa.m_lo[0]);
    const __m128 lo_y = _mm_load_ps(a_soa.m_lo[1]);
    const __m128 lo_z = _mm_load_ps(a_soa.m_lo[2]);
    const __m128 hi_x = _mm_load_ps(a_soa.m_hi[0]);
    const __m128 hi_y = _mm_load_ps(a_soa.m_hi[1]);
    const __m128 hi_z = _mm_load_ps(a_soa.m_hi[2]);

    const __m128 dx = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_x, px), _mm_sub_ps(px, hi_x)));
    const __m128 dy = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_y, py), _mm_sub_ps(py, hi_y)));
    const __m128 dz = _mm_max_ps(zero, _mm_max_ps(_mm_sub_ps(lo_z, pz), _mm_sub_ps(pz, hi_z)));
    const __m128 d2 = _mm_add_ps(_mm_mul_ps(dx, dx), _mm_add_ps(_mm_mul_ps(dy, dy), _mm_mul_ps(dz, dz)));

    _mm_storeu_ps(a_dist2, d2);

    return;
  }
#endif // __SSE4_1__

#endif // !EBGEOMETRY_DEVICE_COMPILE

  // Scalar path: every (T, K) with no compiled ISA path above, and all device code.
  //
  // The per-axis clamp and the dx*dx + (dy*dy + dz*dz) association below are written out so that
  // they match the SIMD paths exactly, and every path produces bit-identical results.

  for (size_t k = 0; k < K; k++) {
    T delta[3];

    for (size_t dir = 0; dir < 3; dir++) {
      const T p     = a_point[dir];
      const T lower = a_soa.m_lo[dir][k] - p;
      const T upper = p - a_soa.m_hi[dir][k];

      const T d = (upper > lower) ? upper : lower;

      delta[dir] = (d > T(0.0)) ? d : T(0.0);
    }

    a_dist2[k] = delta[0] * delta[0] + (delta[1] * delta[1] + delta[2] * delta[2]);
  }
}

template <class T, class P, size_t K>
template <class State, class LeafEvaluator, class PruneDistSquared>
EBGEOMETRY_HOST_DEVICE
inline void
PackedBVH<T, P, K>::pruneTraverse(const Vec3T<T>&    a_point,
                                  State&             a_state,
                                  LeafEvaluator&&    a_evalLeaf,
                                  PruneDistSquared&& a_pruneDist2) const noexcept
{
  // An empty BVH has no root to descend from. Nothing that builds through a TreeBVH or through the
  // direct constructors can reach this -- they assert a non-empty primitive list and would have
  // died long before -- but finalize() will happily produce a zero-node BVH from an empty build
  // result, which is how PointCloudBVH represents an empty cloud (it guards its own traversal the
  // same way). Guarding here rather than at each call site means the pool-backed build path cannot
  // grow a caller that reads node 0 out of bounds, which in Release is silent.
  if (m_linearNodes.size() == 0) {
    return;
  }

  // One resolution for the whole traversal. Safe because a query performs no reservation, so the
  // base cannot move underneath it.
  const void* poolBase = this->base();

  constexpr size_t stackDepth = PackedBVH::traversalStackDepth();

  StackEntry stack[stackDepth];

  int top = 0;

  stack[top++] = StackEntry{0U, 0.0F};

  while (top > 0) {
    const StackEntry entry = stack[--top];

    // Read the pruning bound once per node visit. A leaf visited anywhere earlier -- in any subtree,
    // not just this one -- may have tightened it since this entry was pushed, so an entry that
    // looked promising at push time can be discarded here without descending. The stored distance
    // never exceeds the true one, so this never discards an entry that could hold the answer.
    const T pruneDist2 = a_pruneDist2(a_state);

    if (T(entry.m_dist2) > pruneDist2) {
      continue;
    }

    const Node& node = m_linearNodes.at(poolBase, entry.m_idx);

    if (node.isLeaf()) {
      a_evalLeaf(a_state, node.getPrimitivesOffset(), node.getNumPrimitives());
    }
    else {
      T dist2[K];

      PackedBVH::computeChildDistances2(m_childAabbSoA.at(poolBase, node.getChildBoxRow()), a_point, dist2);

      const auto& offsets = node.getChildOffsets();

      // The children within the bound, sorted into descending distance order. The stack is LIFO, so
      // the nearest child ends up on top and is expanded first -- which is what makes the bound
      // tighten quickly. Filtering here as well as at pop time keeps hopeless children off the stack
      // entirely; nothing between the read above and here can have changed a_state (a_evalLeaf runs
      // only on the leaf branch), so the bound is the same value.
      //
      // Insertion sort, not std::sort: K is a handful of elements, where insertion sort is simply
      // faster than an introsort's setup, and std::sort is not callable from device code. It is also
      // stable, so children whose boxes are exactly equidistant keep their child-slot order rather
      // than an unspecified one, which keeps traversal order deterministic. It sorts on the exact
      // distances; only the stored copies are rounded.
      size_t slots[K];
      size_t numSlots = 0;

      for (size_t k = 0; k < K; k++) {
        EBGEOMETRY_EXPECT(offsets[k] > entry.m_idx);

        if (dist2[k] <= pruneDist2) {
          size_t j = numSlots++;

          while (j > 0 && dist2[slots[j - 1]] < dist2[k]) {
            slots[j] = slots[j - 1];
            j--;
          }

          slots[j] = k;
        }
      }

      for (size_t i = 0; i < numSlots; i++) {
        EBGEOMETRY_EXPECT(top < static_cast<int>(stackDepth));

        stack[top++] = StackEntry{offsets[slots[i]], PackedBVH::lowerBound(dist2[slots[i]])};
      }
    }
  }
}

template <class T, class P, size_t K>
template <class BVConstructor>
inline void
PackedBVH<T, P, K>::refit(const BVConstructor& a_bvConstructor)
{
  // m_linearNodes is a depth-first pre-order flattening, so every child has a higher index than its
  // parent. Sweeping the array in reverse therefore refits all of a node's children before the node
  // itself -- no recursion or explicit stack needed. Boxes are merged pairwise, so a refit (meant as
  // cheap per-frame maintenance) allocates nothing.
  //
  // refit() reserves nothing, so one resolution covers the whole sweep.
  void* poolBase = const_cast<void*>(this->base());

  for (uint32_t i = m_linearNodes.size(); i-- > 0;) {
    Node& node = m_linearNodes.at(poolBase, i);

    BV bv;

    if (node.isLeaf()) {
      const uint32_t offset = node.getPrimitivesOffset();
      const uint32_t count  = node.getNumPrimitives();

      for (uint32_t p = 0; p < count; p++) {
        bv = bv.merged(a_bvConstructor(m_primitives.at(poolBase, offset + p)));
      }
    }
    else {
      const auto& childOffsets = node.getChildOffsets();

      for (size_t k = 0; k < K; k++) {
        bv = bv.merged(m_linearNodes.at(poolBase, childOffsets[k]).getBoundingVolume());
      }
    }

    node.setBoundingVolume(bv);
  }

  // Node count is unchanged, so the existing cache is refilled rather than re-reserved.
  this->buildSoA(nullptr);
}

} // namespace BVH

} // namespace EBGeometry

#endif
