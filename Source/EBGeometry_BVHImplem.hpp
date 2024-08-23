/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_BVHImplem.hpp
  @brief  Implementation of EBGeometry_BVH.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_BVHImplem
#define EBGeometry_BVHImplem

// Std includes
#include <stack>
#include <tuple>

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace BVH {

  template <class T, class P, class BV, size_t K>
  inline NodeT<T, P, BV, K>::NodeT() noexcept
  {
    for (auto& c : m_children) {
      c = nullptr;
    }

    m_primitives.resize(0);
    m_boundingVolumes.resize(0);

    m_partitioned = false;
  }

  template <class T, class P, class BV, size_t K>
  inline NodeT<T, P, BV, K>::NodeT(const std::vector<PrimAndBV<P, BV>>& a_primsAndBVs) noexcept : NodeT<T, P, BV, K>()
  {
    for (const auto& pbv : a_primsAndBVs) {
      m_primitives.emplace_back(pbv.first);
      m_boundingVolumes.emplace_back(pbv.second);
    }

    m_boundingVolume = BV(m_boundingVolumes);

    m_partitioned = false;
  }

  template <class T, class P, class BV, size_t K>
  inline NodeT<T, P, BV, K>::~NodeT() noexcept
  {}

  template <class T, class P, class BV, size_t K>
  inline bool
  NodeT<T, P, BV, K>::isLeaf() const noexcept
  {
    return m_primitives.size() > 0;
  }

  template <class T, class P, class BV, size_t K>
  inline bool
  NodeT<T, P, BV, K>::isPartitioned() const noexcept
  {
    return m_partitioned;
  }

  template <class T, class P, class BV, size_t K>
  inline const BV&
  NodeT<T, P, BV, K>::getBoundingVolume() const noexcept
  {
    return (m_boundingVolume);
  }

  template <class T, class P, class BV, size_t K>
  inline PrimitiveListT<P>&
  NodeT<T, P, BV, K>::getPrimitives() noexcept
  {
    return (m_primitives);
  }

  template <class T, class P, class BV, size_t K>
  inline const PrimitiveListT<P>&
  NodeT<T, P, BV, K>::getPrimitives() const noexcept
  {
    return (m_primitives);
  }

  template <class T, class P, class BV, size_t K>
  inline std::vector<BV>&
  NodeT<T, P, BV, K>::getBoundingVolumes() noexcept
  {
    return (m_boundingVolumes);
  }

  template <class T, class P, class BV, size_t K>
  inline const std::vector<BV>&
  NodeT<T, P, BV, K>::getBoundingVolumes() const noexcept
  {
    return (m_boundingVolumes);
  }

  template <class T, class P, class BV, size_t K>
  inline const std::array<std::shared_ptr<NodeT<T, P, BV, K>>, K>&
  NodeT<T, P, BV, K>::getChildren() const noexcept
  {
    return (m_children);
  }

  template <class T, class P, class BV, size_t K>
  inline void
  NodeT<T, P, BV, K>::topDownSortAndPartition(const Partitioner& a_partitioner, const StopFunction& a_stopCrit) noexcept
  {
    // Check if this node should be split into more nodes.
    const auto numPrimsInThisNode = m_primitives.size();

    const bool stopRecursiveSplitting = a_stopCrit(*this);
    const bool hasEnoughPrimitives    = numPrimsInThisNode >= K;

    if (!stopRecursiveSplitting && hasEnoughPrimitives) {

      // Pack primitives and BVs.
      PrimAndBVListT<P, BV> primsAndBVs;
      for (size_t i = 0; i < numPrimsInThisNode; i++) {
        primsAndBVs.emplace_back(std::make_pair(m_primitives[i], m_boundingVolumes[i]));
      }

      // Partition into sub-sets.
      const auto& newPartitions = a_partitioner(primsAndBVs);

      // Create children nodes.
      for (size_t c = 0; c < K; c++) {
        m_children[c] = std::make_shared<NodeT<T, P, BV, K>>(newPartitions[c]);
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
  NodeT<T, P, BV, K>::bottomUpSortAndPartition() noexcept
  {
    // The space-filling curves operate on positive coordinates only, using up to 2^21 valid bits
    // per coordinate direction. The centroids of the bounding volumes use this coordinate system
    // rather than the "real" coordinates.
    std::vector<SFC::Index> bins;

    Vec3 minCoord = Vec3::infinity();
    Vec3 maxCoord = -Vec3::infinity();

    for (const auto& bv : m_boundingVolumes) {
      const auto& curCentroid = bv.getCentroid();

      minCoord = min(minCoord, curCentroid);
      maxCoord = max(maxCoord, curCentroid);
    }

    const Vec3 delta = (maxCoord - minCoord) / SFC::ValidSpan;

    for (const auto& bv : m_boundingVolumes) {
      const Vec3 curBin = (bv.getCentroid() - minCoord) / delta;

      bins.emplace_back(SFC::Index{
        (unsigned int)std::floor(curBin[0]), (unsigned int)std::floor(curBin[1]), (unsigned int)std::floor(curBin[2])});
    }

    // Sort the primitives, their BVs, and their spatial bins along the space-filling curves.
    using PrimBvAndCode = std::tuple<std::shared_ptr<const P>, BV, SFC::Code>;

    std::vector<PrimBvAndCode> sortedPrimitives;

    for (int i = 0; i < m_primitives.size(); i++) {
      sortedPrimitives.emplace_back(std::make_tuple(m_primitives[i], m_boundingVolumes[i], S::encode(bins[i])));
    }

    auto sortCrit = [](const PrimBvAndCode& A, const PrimBvAndCode& B) -> bool {
      return std::get<2>(A) < std::get<2>(B);
    };

    std::sort(std::begin(sortedPrimitives), std::end(sortedPrimitives), sortCrit);

    // Go through the SFC and merge leaves that are nearby. We are trying to build a _balanced_
    // tree where all the leaves exist on the same level, so the numb
    // a root node in the end.
    const size_t numPrimitives = sortedPrimitives.size();
    const size_t treeDepth     = std::floor(log(numPrimitives) / log(K));
    const size_t numLeaves     = std::pow(K, treeDepth);
    const size_t primsPerLeaf  = numPrimitives / numLeaves;

    if (treeDepth > 0) {

      std::vector<std::vector<std::shared_ptr<NodeT<T, P, BV, K>>>> nodes(treeDepth + 1);

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

        for (size_t i = startIndex; i <= endIndex; i++) {
          const auto& cur = sortedPrimitives[i];

          primsAndBVs.emplace_back(std::get<0>(cur), std::get<1>(cur));
        }

        nodes[treeDepth].emplace_back(std::make_shared<NodeT<T, P, BV, K>>(primsAndBVs));

        startIndex = endIndex + 1;
      }

      // Starting at the bottom of the tree, merge the nodes upward in clusters of K.
      for (int lvl = treeDepth - 1; lvl >= 0; lvl--) {
        const size_t numNodes = std::pow(K, lvl);

        for (int inode = 0; inode < numNodes; inode++) {

          std::array<std::shared_ptr<NodeT<T, P, BV, K>>, K> children;

          for (int child = 0; child < K; child++) {
            children[child] = nodes[lvl - 1][inode * K + child];
          }

          // The root node (i.e., this node) already exists so don't reset it.
          if (lvl > 0) {
            nodes[lvl].emplace_back(std::make_shared<NodeT<T, P, BV, K>>());
          }
          else {
            nodes[lvl].emplace_back(this->shared_from_this());
          }

          nodes[lvl].back()->setChildren(children);
        }
      }
    }

    m_partitioned = true;
  }

  template <class T, class P, class BV, size_t K>
  inline void
  NodeT<T, P, BV, K>::setChildren(const std::array<std::shared_ptr<NodeT<T, P, BV, K>>, K>& a_children) noexcept
  {

    std::vector<BV> boundingVolumes;
    for (const auto& child : a_children) {
      boundingVolumes.emplace_back(child->getBoundingVolume());
    }

    m_primitives.resize(0);
    m_boundingVolumes.resize(0);

    m_boundingVolume = BV(m_boundingVolumes);
    m_children       = a_children;
    m_partitioned    = true;
  }

  template <class T, class P, class BV, size_t K>
  inline T
  NodeT<T, P, BV, K>::getDistanceToBoundingVolume(const Vec3& a_point) const noexcept
  {
    return m_boundingVolume.getDistance(a_point);
  }

  template <class T, class P, class BV, size_t K>
  template <class Meta>
  inline void
  NodeT<T, P, BV, K>::traverse(const BVH::Updater<P>&              a_updater,
                               const BVH::Visiter<Node, Meta>&     a_visiter,
                               const BVH::Sorter<Node, Meta, K>&   a_sorter,
                               const BVH::MetaUpdater<Node, Meta>& a_metaUpdater) const noexcept
  {
    std::array<std::pair<std::shared_ptr<const Node>, Meta>, K> children;
    std::stack<std::pair<std::shared_ptr<const Node>, Meta>>    q;

    q.emplace(this->shared_from_this(), a_metaUpdater(*this));

    while (!(q.empty())) {
      const auto& node = q.top().first;
      const auto& meta = q.top().second;

      q.pop();

      if (a_visiter(*node, meta)) {
        if (node->isLeaf()) {
          a_updater(node->getPrimitives());
        }
        else {
          for (size_t k = 0; k < K; k++) {
            children[k].first  = node->getChildren()[k];
            children[k].second = a_metaUpdater(*children[k].first);
          }

          // User-based visit pattern.
          a_sorter(children);

          for (const auto& child : children) {
            q.push(child);
          }
        }
      }
    }
  }

  template <class T, class P, class BV, size_t K>
  inline std::shared_ptr<LinearBVH<T, P, BV, K>>
  NodeT<T, P, BV, K>::flattenTree() const noexcept
  {

    // Create a list of sorted primitives and nodes.
    std::vector<std::shared_ptr<const P>>                  sortedPrimitives;
    std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K>>> linearNodes;

    // Track the offset into the linearized node array.
    size_t offset = 0;

    // Flatten recursively.
    this->flattenTree(linearNodes, sortedPrimitives, offset);

    // Return the root node.
    return std::make_shared<LinearBVH<T, P, BV, K>>(linearNodes, sortedPrimitives);
  }

  template <class T, class P, class BV, size_t K>
  inline size_t
  NodeT<T, P, BV, K>::flattenTree(std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K>>>& a_linearNodes,
                                  std::vector<std::shared_ptr<const P>>&                  a_sortedPrimitives,
                                  size_t&                                                 a_offset) const noexcept
  {

    // TLDR: This is the main routine for flattening the hierarchy beneath the
    // current node. When this is called we insert
    //       this node into a_linearNodes and associate the array offsets so that
    //       we can find the children in the linearized array.

    // Current node we are dealing with.
    const auto curNode = a_offset;

    // Insert a new node corresponding to this node and provide it with the
    // current bounding volume.
    a_linearNodes.emplace_back(std::make_shared<LinearNodeT<T, P, BV, K>>());
    a_linearNodes[curNode]->setBoundingVolume(m_boundingVolume);

    a_offset++;

    if (this->isLeaf()) {
      // Insert primitives and offsets.
      a_linearNodes[curNode]->setNumPrimitives(m_primitives.size());
      a_linearNodes[curNode]->setPrimitivesOffset(a_sortedPrimitives.size());

      a_sortedPrimitives.insert(a_sortedPrimitives.end(), m_primitives.begin(), m_primitives.end());
    }
    else {
      a_linearNodes[curNode]->setNumPrimitives(0);
      a_linearNodes[curNode]->setPrimitivesOffset(0UL);

      // Go through the children nodes and
      for (size_t k = 0; k < K; k++) {
        const size_t offset = m_children[k]->flattenTree(a_linearNodes, a_sortedPrimitives, a_offset);

        a_linearNodes[curNode]->setChildOffset(offset, k);
      }
    }

    return curNode;
  }

  template <class T, class P, class BV, size_t K>
  inline LinearNodeT<T, P, BV, K>::LinearNodeT() noexcept
  {

    // Initialize everything.
    m_boundingVolume   = BV();
    m_primitivesOffset = 0UL;
    m_numPrimitives    = 0;

    for (auto& offset : m_childOffsets) {
      offset = 0UL;
    }
  }

  template <class T, class P, class BV, size_t K>
  inline LinearNodeT<T, P, BV, K>::~LinearNodeT()
  {}

  template <class T, class P, class BV, size_t K>
  inline void
  LinearNodeT<T, P, BV, K>::setBoundingVolume(const BV& a_boundingVolume) noexcept
  {
    m_boundingVolume = a_boundingVolume;
  }

  template <class T, class P, class BV, size_t K>
  inline void
  LinearNodeT<T, P, BV, K>::setPrimitivesOffset(const size_t a_primitivesOffset) noexcept
  {
    m_primitivesOffset = a_primitivesOffset;
  }

  template <class T, class P, class BV, size_t K>
  inline void
  LinearNodeT<T, P, BV, K>::setNumPrimitives(const size_t a_numPrimitives) noexcept
  {
    m_numPrimitives = a_numPrimitives;
  }

  template <class T, class P, class BV, size_t K>
  inline void
  LinearNodeT<T, P, BV, K>::setChildOffset(const size_t a_childOffset, const size_t a_whichChild) noexcept
  {
    m_childOffsets[a_whichChild] = a_childOffset;
  }

  template <class T, class P, class BV, size_t K>
  inline const BV&
  LinearNodeT<T, P, BV, K>::getBoundingVolume() const noexcept
  {
    return (m_boundingVolume);
  }

  template <class T, class P, class BV, size_t K>
  inline const size_t&
  LinearNodeT<T, P, BV, K>::getPrimitivesOffset() const noexcept
  {
    return (m_primitivesOffset);
  }

  template <class T, class P, class BV, size_t K>
  inline const size_t&
  LinearNodeT<T, P, BV, K>::getNumPrimitives() const noexcept
  {
    return (m_numPrimitives);
  }

  template <class T, class P, class BV, size_t K>
  inline const std::array<size_t, K>&
  LinearNodeT<T, P, BV, K>::getChildOffsets() const noexcept
  {
    return (m_childOffsets);
  }

  template <class T, class P, class BV, size_t K>
  inline bool
  LinearNodeT<T, P, BV, K>::isLeaf() const noexcept
  {
    return m_numPrimitives > 0;
  }
  template <class T, class P, class BV, size_t K>
  inline bool
  LinearNodeT<T, P, BV, K>::isPartitioned() const noexcept
  {
    return m_partitioned;
  }

  template <class T, class P, class BV, size_t K>
  inline T
  LinearNodeT<T, P, BV, K>::getDistanceToBoundingVolume(const Vec3& a_point) const noexcept
  {
    return m_boundingVolume.getDistance(a_point);
  }

  template <class T, class P, class BV, size_t K>
  inline std::vector<T>
  LinearNodeT<T, P, BV, K>::getDistances(const Vec3T<T>&                              a_point,
                                         const std::vector<std::shared_ptr<const P>>& a_primitives) const noexcept
  {
    std::vector<T> distances;

    for (size_t i = 0; i < m_numPrimitives; i++) {
      const T curDist = a_primitives[m_primitivesOffset + i]->signedDistance(a_point);

      distances.emplace_back(curDist);
    }

    return distances;
  }

  template <class T, class P, class BV, size_t K>
  inline LinearBVH<T, P, BV, K>::LinearBVH(
    const std::vector<std::shared_ptr<const LinearNodeT<T, P, BV, K>>>& a_linearNodes,
    const std::vector<std::shared_ptr<const P>>&                        a_primitives)
  {
    m_linearNodes = a_linearNodes;
    m_primitives  = a_primitives;
  }

  template <class T, class P, class BV, size_t K>
  inline LinearBVH<T, P, BV, K>::LinearBVH(const std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K>>>& a_linearNodes,
                                           const std::vector<std::shared_ptr<const P>>&                  a_primitives)
  {

    for (const auto& p : a_linearNodes) {
      m_linearNodes.emplace_back(p);
    }

    m_primitives = a_primitives;
  }

  template <class T, class P, class BV, size_t K>
  inline LinearBVH<T, P, BV, K>::~LinearBVH()
  {}

  template <class T, class P, class BV, size_t K>
  inline const BV&
  LinearBVH<T, P, BV, K>::getBoundingVolume() const noexcept
  {
    return m_linearNodes.front()->getBoundingVolume();
  }

  template <class T, class P, class BV, size_t K>
  template <class Meta>
  inline void
  LinearBVH<T, P, BV, K>::traverse(const BVH::Updater<P>&                    a_updater,
                                   const BVH::Visiter<LinearNode, Meta>&     a_visiter,
                                   const BVH::Sorter<LinearNode, Meta, K>&   a_sorter,
                                   const BVH::MetaUpdater<LinearNode, Meta>& a_metaUpdater) const noexcept
  {
    std::array<std::pair<std::shared_ptr<const LinearNode>, Meta>, K> children;
    std::stack<std::pair<std::shared_ptr<const LinearNode>, Meta>>    q;

    q.emplace(m_linearNodes[0], a_metaUpdater(*m_linearNodes[0]));

    while (!(q.empty())) {
      const auto& node = q.top().first;
      const auto& meta = q.top().second;

      q.pop();

      // User-based criterion for whether or not we need to check this node.
      if (a_visiter(*node, meta)) {

        if (node->isLeaf()) {
          const size_t& offset  = node->getPrimitivesOffset();
          const size_t& numPrim = node->getNumPrimitives();

          const auto first = m_primitives.begin() + offset;
          const auto last  = first + numPrim;

          // User-based update rule.
          a_updater(PrimitiveList(first, last));
        }
        else {

          // Update the children visitation pattern.
          for (size_t k = 0; k < K; k++) {
            children[k].first  = m_linearNodes[node->getChildOffsets()[k]];
            children[k].second = a_metaUpdater(*children[k].first);
          }

          // User-based visit pattern.
          a_sorter(children);

          for (const auto& child : children) {
            q.push(child);
          }
        }
      }
    }
  }

} // namespace BVH

#include "EBGeometry_NamespaceFooter.hpp"

#endif
