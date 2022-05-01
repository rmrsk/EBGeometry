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

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace BVH {

template <class T, class P, class BV, size_t K>
inline NodeT<T, P, BV, K>::NodeT()
{
  for (auto& c : m_children) {
    c = nullptr;
  }

  m_primitives.resize(0);
}

template <class T, class P, class BV, size_t K>
inline NodeT<T, P, BV, K>::NodeT(const std::vector<std::shared_ptr<P>>& a_primitives) : NodeT<T, P, BV, K>()
{
  for (const auto& p : a_primitives) {
    m_primitives.emplace_back(p);
  }

  for (auto& c : m_children) {
    c = nullptr;
  }
}

template <class T, class P, class BV, size_t K>
inline NodeT<T, P, BV, K>::NodeT(const std::vector<std::shared_ptr<const P>>& a_primitives) : NodeT<T, P, BV, K>()
{
  m_primitives = a_primitives;

  for (auto& c : m_children) {
    c = nullptr;
  }
}

template <class T, class P, class BV, size_t K>
inline NodeT<T, P, BV, K>::~NodeT()
{
}

template <class T, class P, class BV, size_t K>
inline void
NodeT<T, P, BV, K>::setPrimitives(const PrimitiveList& a_primitives) noexcept
{
  m_primitives = a_primitives;
}

template <class T, class P, class BV, size_t K>
inline bool
NodeT<T, P, BV, K>::isLeaf() const noexcept
{
  return m_primitives.size() > 0;
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
inline const BV&
NodeT<T, P, BV, K>::getBoundingVolume() const noexcept
{
  return (m_boundingVolume);
}

template <class T, class P, class BV, size_t K>
inline const std::array<std::shared_ptr<NodeT<T, P, BV, K>>, K>&
NodeT<T, P, BV, K>::getChildren() const noexcept
{
  return (m_children);
}

template <class T, class P, class BV, size_t K>
inline void
NodeT<T, P, BV, K>::topDownSortAndPartitionPrimitives(
  const BVConstructor& a_bvConstructor, const Partitioner& a_partitioner, const StopFunction& a_stopCrit) noexcept
{

  // Compute the bounding volume for this node.
  std::vector<BV> boundingVolumes;
  for (const auto& p : m_primitives) {
    boundingVolumes.emplace_back(a_bvConstructor(p));
  }

  m_boundingVolume = BV(boundingVolumes);

  // Check if we can split this node into sub-bounding volumes.
  const bool stopRecursiveSplitting = a_stopCrit(*this);
  const bool hasEnoughPrimitives = m_primitives.size() >= K;

  if (!stopRecursiveSplitting && hasEnoughPrimitives) {

    // Divide primitives into new partitions
    const auto& newPartitions = a_partitioner(m_primitives); // Divide this node's primitives into K new
                                                             // sub-volume primitives
    this->insertChildren(newPartitions);                     // Insert the K new nodes into the tree.

    m_primitives.resize(0); // This node is no longer a leaf node.

    // Partition children nodes further
    for (auto& c : m_children) {
      c->topDownSortAndPartitionPrimitives(a_bvConstructor, a_partitioner, a_stopCrit);
    }
  }
}

template <class T, class P, class BV, size_t K>
inline void
NodeT<T, P, BV, K>::insertChildren(const std::array<PrimitiveList, K>& a_primitives) noexcept
{
  for (size_t l = 0; l < K; l++) {
    m_children[l] = std::make_shared<NodeT<T, P, BV, K>>();

    m_children[l]->setPrimitives(a_primitives[l]);
  }
}

template <class T, class P, class BV, size_t K>
inline T
NodeT<T, P, BV, K>::getDistanceToBoundingVolume(const Vec3& a_point) const noexcept
{
  return m_boundingVolume.getDistance(a_point);
}

template <class T, class P, class BV, size_t K>
inline T
NodeT<T, P, BV, K>::getDistanceToPrimitives(const Vec3& a_point) const noexcept
{
  T minDist = std::numeric_limits<T>::infinity();

  for (const auto& p : m_primitives) {
    const auto curDist = p->signedDistance(a_point);

    if (curDist * curDist < minDist * minDist) {
      minDist = curDist;
    }
  }

  return minDist;
}

template <class T, class P, class BV, size_t K>
inline T
NodeT<T, P, BV, K>::signedDistance(const Vec3& a_point) const noexcept
{
  return this->signedDistance(a_point, Prune::Stack);
}

template <class T, class P, class BV, size_t K>
inline T
NodeT<T, P, BV, K>::signedDistance(const Vec3& a_point, const Prune a_pruning) const noexcept
{
  T ret = std::numeric_limits<T>::infinity();

  switch (a_pruning) {
  case Prune::Stack: {
    ret = this->pruneStack(a_point);

    break;
  }
  case Prune::Ordered: {
    this->pruneOrdered(ret, a_point);

    break;
  }
  case Prune::Unordered: {
    this->pruneUnordered(ret, a_point);

    break;
  }
  default:
    std::cerr << "In file EBGeometry_BVHImplem.hpp function NodeT<T, P, BV, "
                 "K>::signedDistance(Vec3, Prune) -- bad input enum for 'Prune'\n";
  };

  return ret;
}

template <class T, class P, class BV, size_t K>
inline T
NodeT<T, P, BV, K>::pruneStack(const Vec3& a_point) const noexcept
{
  // TLDR: This routine uses ordered traversal along the branches. Rather than
  // calling itself recursively, it uses
  //       a stack for investigating the branches and nodes.

  // Shortest distance. Initialize to something big.
  T minDist = std::numeric_limits<T>::infinity();

  // Create temporary storage and and priority queue (our stack).
  using RawNode = const NodeT<T, P, BV, K>*;
  using NodeAndDist = std::pair<RawNode, T>;

  std::array<NodeAndDist, K> childrenAndDistances;
  std::stack<NodeAndDist> q;

  // Initialize the stack with the root node.
  q.emplace(this, this->getDistanceToBoundingVolume(a_point));

  // Stack loop -- always investigate the one at the top.
  while (!(q.empty())) {

    // Pop the top node off the stack.
    const auto& curNode = (q.top()).first;
    const auto& bvDist = (q.top()).second;

    q.pop();

    // See if we really need to process this node. We only need to do it if its
    // BV is closer than the shortest distance we've found so far. Otherwise we
    // are guaranteed that the distance to the primitives is larger than the
    // shortest distance we've found so far. If the current node is a leaf node
    // we just update the shortest distance. Otherwise we fetch the child nodes
    // and process them (in a very specific order!).
    if (bvDist <= std::abs(minDist)) {
      if (curNode->isLeaf()) {
        const T primDist = curNode->getDistanceToPrimitives(a_point);

        if (std::abs(primDist) < std::abs(minDist)) {
          minDist = primDist;
        }
      }
      else {
        // If it's a regular node, sort the child nodes and put them on the
        // stack. On the next iteration we do the closest node first. This
        // sorting is critical to the performance of the BVH.

        // Get the nodes's children.
        const std::array<NodePtr, K>& children = curNode->getChildren();

        for (size_t k = 0; k < K; k++) {
          const RawNode& child = &(*children[k]);

          childrenAndDistances[k] = std::make_pair(child, child->getDistanceToBoundingVolume(a_point));
        }

        std::sort(
          childrenAndDistances.begin(), childrenAndDistances.end(),
          [](const NodeAndDist& node1, const NodeAndDist& node2) -> bool { return node1.second > node2.second; });

        // Push the children onto the stack.
        for (const auto& child : childrenAndDistances) {
          q.push(child);
        }
      }
    }
  }

  return minDist;
}

template <class T, class P, class BV, size_t K>
inline void
NodeT<T, P, BV, K>::pruneOrdered(T& a_shortestDistanceSoFar, const Vec3& a_point) const noexcept
{

  // TLDR: Beginning at some node, this routine descends the branches in the
  // tree. It always descends the branch with the shortest distance
  //       to the bounding volume first. The other branch is investigated only
  //       after the full sub-tree beneath the first branch has completed. Since
  //       the shortest distance to primitives is updated underway, there is a
  //       decent chance that the secondary subtree can be pruned. Hence why
  //       this routine is more efficient than prunedUnordered.
  if (this->isLeaf()) {
    // Compute the shortest signed distance to the primitives in this leaf node.
    // If this is shorter than a_shortestDistanceSoFar, update it. Recall that
    // the comparison requires the absolute value since we're doing the SIGNED
    // distance.
    const T primDist = this->getDistanceToPrimitives(a_point);

    if (std::abs(primDist) < std::abs(a_shortestDistanceSoFar)) {
      a_shortestDistanceSoFar = primDist;
    }
  }
  else {
    // In this case we need to decide which subtree to move down. First, sort
    // the children nodes by the distance between a_point and the children
    // node's bounding volume. Shortest distance goes first.
    std::array<std::pair<T, NodePtr>, K> distancesAndNodes;
    for (size_t i = 0; i < K; i++) {
      distancesAndNodes[i] = std::make_pair(m_children[i]->getDistanceToBoundingVolume(a_point), m_children[i]);
    }

    // Comparator for sorting -- puts the node with the shortest distance to the
    // bounding volume at the front of the vector.
    auto comparator = [](const std::pair<T, NodePtr>& a_node1, const std::pair<T, NodePtr>& a_node2) -> bool {
      return std::abs(a_node1.first) < std::abs(a_node2.first);
    };

    std::sort(distancesAndNodes.begin(), distancesAndNodes.end(), comparator);

    // Go through the children nodes -- closest node goes first. We prune
    // branches if the distance to the node's bounding volume is longer than the
    // shortest distance we've found so far.
    for (size_t i = 0; i < K; i++) {
      const std::pair<T, NodePtr>& curChildNode = distancesAndNodes[i];

      // a_shortestDistanceSoFar is the SIGNED distance, so we need the absolute
      // value here.
      if (std::abs(curChildNode.first) <= std::abs(a_shortestDistanceSoFar)) {
        curChildNode.second->pruneOrdered(a_shortestDistanceSoFar, a_point);
      }
      else { // Prune the rest of the children nodes.
        break;
      }
    }
  }
}

template <class T, class P, class BV, size_t K>
inline void
NodeT<T, P, BV, K>::pruneUnordered(T& a_shortestDistanceSoFar, const Vec3& a_point) const noexcept
{
  if (this->isLeaf()) {
    // Check if the distance to the primitives in this leaf is shorter than
    // a_shortestDistanceSoFar. If it is, update the shortest distance.
    const T curSignedDistance = this->getDistanceToPrimitives(a_point);

    if (std::abs(curSignedDistance) < std::abs(a_shortestDistanceSoFar)) {
      a_shortestDistanceSoFar = curSignedDistance;
    }
  }
  else {
    // Investigate subtrees. Prune subtrees if the distance to their bounding
    // volumes are longer than the shortest distance we've found so far.
    for (const auto& child : m_children) {
      const T distanceToChildBoundingVolume = child->getDistanceToBoundingVolume(a_point);

      if (std::abs(distanceToChildBoundingVolume) < std::abs(a_shortestDistanceSoFar)) {
        child->pruneUnordered(a_shortestDistanceSoFar, a_point);
      }
    }
  }
}

template <class T, class P, class BV, size_t K>
inline std::shared_ptr<LinearBVH<T, P, BV, K>>
NodeT<T, P, BV, K>::flattenTree() const noexcept
{

  // Create a list of sorted primitives and nodes.
  std::vector<std::shared_ptr<const P>> sortedPrimitives;
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
NodeT<T, P, BV, K>::flattenTree(
  std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K>>>& a_linearNodes,
  std::vector<std::shared_ptr<const P>>& a_sortedPrimitives,
  size_t& a_offset) const noexcept
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
  m_boundingVolume = BV();
  m_primitivesOffset = 0UL;
  m_numPrimitives = 0;

  for (auto& offset : m_childOffsets) {
    offset = 0UL;
  }
}

template <class T, class P, class BV, size_t K>
inline LinearNodeT<T, P, BV, K>::~LinearNodeT()
{
}

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
inline T
LinearNodeT<T, P, BV, K>::getDistanceToBoundingVolume(const Vec3& a_point) const noexcept
{
  return m_boundingVolume.getDistance(a_point);
}

template <class T, class P, class BV, size_t K>
inline T
LinearNodeT<T, P, BV, K>::getDistanceToPrimitives(
  const Vec3T<T>& a_point, const std::vector<std::shared_ptr<const P>>& a_primitives) const noexcept
{
  T minDist = std::numeric_limits<T>::infinity();

  for (size_t i = 0; i < m_numPrimitives; i++) {
    const T curDist = a_primitives[m_primitivesOffset + i]->signedDistance(a_point);

    if (std::abs(curDist) < std::abs(minDist)) {
      minDist = curDist;
    }
  }

  return minDist;
}

template <class T, class P, class BV, size_t K>
inline LinearBVH<T, P, BV, K>::LinearBVH(
  const std::vector<std::shared_ptr<const LinearNodeT<T, P, BV, K>>>& a_linearNodes,
  const std::vector<std::shared_ptr<const P>>& a_primitives)
{
  m_linearNodes = a_linearNodes;
  m_primitives = a_primitives;
}

template <class T, class P, class BV, size_t K>
inline LinearBVH<T, P, BV, K>::LinearBVH(
  const std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K>>>& a_linearNodes,
  const std::vector<std::shared_ptr<const P>>& a_primitives)
{

  for (const auto& p : a_linearNodes) {
    m_linearNodes.emplace_back(p);
  }

  m_primitives = a_primitives;
}

template <class T, class P, class BV, size_t K>
inline LinearBVH<T, P, BV, K>::~LinearBVH()
{
}

template <class T, class P, class BV, size_t K>
inline T
LinearBVH<T, P, BV, K>::signedDistance(const Vec3& a_point) const noexcept
{
  // TLDR: This routine uses ordered traversal along the branches. Rather than
  // calling itself recursively, it uses
  //       a stack for investigating the branches and nodes.

  // Shortest unsigned square distance. Initialize to something big.
  T minDist = std::numeric_limits<T>::infinity();

  // Create temporary storage and and priority queue (our stack).
  using NodeAndDist = std::pair<size_t, T>;

  std::array<NodeAndDist, K> children;
  std::stack<NodeAndDist> q;

  // Initialize the stack with the root node.
  q.emplace(0, m_linearNodes[0]->getDistanceToBoundingVolume(a_point));

  // Stack loop -- always investigate the one at the top.
  while (!(q.empty())) {

    // Pop the top node off the stack.
    const auto& curNode = (q.top()).first;
    const auto& bvDist = (q.top()).second;

    q.pop();

    // See if we really need to process this node. We only need to do it if its
    // BV is closer than the shortest distance we've found so far. Otherwise we
    // are guaranteed that the distance to the primitives is larger than the
    // shortest distance we've found so far.
    if (bvDist <= std::abs(minDist)) {

      // If it's a leaf node, update the shortest distance so far.
      if (m_linearNodes[curNode]->isLeaf()) {
        const T primDist = m_linearNodes[curNode]->getDistanceToPrimitives(a_point, m_primitives);

        if (std::abs(primDist) < std::abs(minDist)) {
          minDist = primDist;
        }
      }
      else {
        // Compute child indices and their BVH distance to a_point.
        for (size_t k = 0; k < K; k++) {
          const size_t& curOff = m_linearNodes[curNode]->getChildOffsets()[k];
          const T distanceToBV = m_linearNodes[curOff]->getDistanceToBoundingVolume(a_point);

          children[k] = std::make_pair(curOff, distanceToBV);
        }

        // Sort the child nodes and put them on the stack. On the next iteration
        // we do the closest node first. This sorting is critical to the
        // performance of the BVH.
        std::sort(
          children.begin(), children.end(),
          [](const std::pair<size_t, T>& node1, const std::pair<size_t, T>& node2) -> bool {
            return node1.second > node2.second;
          });

        // Push onto stack if the BV is closer than minDist.
        for (const auto& child : children) {
          q.push(child);
        }
      }
    }
  }

  return minDist;
}
} // namespace BVH

#include "EBGeometry_NamespaceFooter.hpp"

#endif
