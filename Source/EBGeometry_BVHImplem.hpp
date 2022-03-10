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

  template<class T, class P, class BV, int K>
  inline
  NodeT<T, P, BV, K>::NodeT() {
    m_parent = nullptr;

    for (auto& c : m_children){
      c = nullptr;
    }

    m_primitives.resize(0);

    m_depth    = 0;
    m_nodeType = NodeType::Regular;
  }

  template<class T, class P, class BV, int K>
  inline
  NodeT<T, P, BV, K>::NodeT(const NodePtr& a_parent) : NodeT<T, P, BV, K>() {
    m_parent   = a_parent;
    m_depth    = a_parent.m_depth + 1;
    m_nodeType = NodeType::Leaf;
  }

  template<class T, class P, class BV, int K>
  inline
  NodeT<T, P, BV, K>::NodeT(const std::vector<std::shared_ptr<P> >& a_primitives) : NodeT<T, P, BV, K>() {
    for (const auto& p : a_primitives){
      m_primitives.emplace_back(p);
    }
  
    m_nodeType = NodeType::Leaf;
    m_depth    = 0;
  }

  template<class T, class P, class BV, int K>
  inline
  NodeT<T, P, BV, K>::NodeT(const std::vector<std::shared_ptr<const P> >& a_primitives) : NodeT<T, P, BV, K>() {
    m_primitives = a_primitives;
  
    m_nodeType = NodeType::Leaf;
    m_depth    = 0;
  }

  template<class T, class P, class BV, int K>
  inline
  NodeT<T, P, BV, K>::~NodeT() {
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::setParent(const NodePtr& a_parent) noexcept {
    m_parent = a_parent;
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::setNodeType(const NodeType a_nodeType) noexcept {
    m_nodeType = a_nodeType;
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::setDepth(const int a_depth) noexcept {
    m_depth = a_depth;
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::setPrimitives(const PrimitiveList& a_primitives) noexcept {
    m_primitives = a_primitives;
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::setToRegularNode() noexcept {
    m_nodeType = NodeType::Regular;
    m_primitives.resize(0);
  }

  template<class T, class P, class BV, int K>
  inline
  NodeType NodeT<T, P, BV, K>::getNodeType() const noexcept {
    return m_nodeType;
  }

  template<class T, class P, class BV, int K>
  inline
  int NodeT<T, P, BV, K>::getDepth() const noexcept {
    return m_depth;
  }

  template<class T, class P, class BV, int K>
  inline
  PrimitiveListT<P>& NodeT<T, P, BV, K>::getPrimitives() noexcept {
    return (m_primitives);
  }

  template<class T, class P, class BV, int K>  
  inline
  const BV& NodeT<T, P, BV, K>::getBoundingVolume() const noexcept {
    return (m_boundingVolume);
  }

  template<class T, class P, class BV, int K>
  inline
  const PrimitiveListT<P>& NodeT<T, P, BV, K>::getPrimitives() const noexcept {
    return (m_primitives);
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::topDownSortAndPartitionPrimitives(const BVConstructor& a_bvConstructor,
							     const Partitioner&   a_partitioner,
							     const StopFunction&  a_stopCrit) noexcept {						     


    // Compute the bounding volume for this node.
    std::vector<BV> boundingVolumes;
    for (const auto& p : m_primitives){
      boundingVolumes.emplace_back(a_bvConstructor(p));
    }
    
    m_boundingVolume = BV(boundingVolumes);

    // Check if we can split this node into sub-bounding volumes. 
    const bool stopRecursiveSplitting = a_stopCrit(*this);
    const bool hasEnoughPrimitives    = m_primitives.size() >= K;

    if(!stopRecursiveSplitting && hasEnoughPrimitives){

      // Divide primitives into new partitions
      const auto& newPartitions = a_partitioner(m_primitives); // Divide this node's primitives into K new sub-volume primitives
      this->insertNodes(newPartitions);                        // Insert the K new nodes into the tree. 
      this->setToRegularNode();                                // This node is no longer a leaf node!

      // Partition children nodes further
      for (auto& c : m_children){
	c->topDownSortAndPartitionPrimitives(a_bvConstructor, a_partitioner, a_stopCrit);
      }
    }
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::insertNode(NodePtr& a_node, const PrimitiveList& a_primitives) noexcept {
    a_node = std::make_shared<NodeT<T, P, BV, K> >();

    a_node->setPrimitives(a_primitives);
    a_node->setParent(std::make_shared<NodeT<T, P, BV, K> >(*this));
    a_node->setNodeType(NodeType::Leaf);
    a_node->setDepth(m_depth+1);
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::insertNodes(const std::array<PrimitiveList, K>& a_primitives) noexcept {
    for (int l = 0; l < K; l++){
      m_children[l] = std::make_shared<NodeT<T, P, BV, K> >();

      m_children[l]->setPrimitives(a_primitives[l]);
      m_children[l]->setParent(std::make_shared<NodeT<T, P, BV, K> >(*this));
      m_children[l]->setNodeType(NodeType::Leaf);
      m_children[l]->setDepth(m_depth+1);
    }
  }

  template<class T, class P, class BV, int K>
  inline
  T NodeT<T, P, BV, K>::getDistanceToBoundingVolume(const Vec3& a_point) const noexcept{
    return m_boundingVolume.getDistance(a_point);
  }

  template<class T, class P, class BV, int K>
  inline
  T NodeT<T, P, BV, K>::getDistanceToBoundingVolume2(const Vec3& a_point) const noexcept{
    return m_boundingVolume.getDistance2(a_point);
  }

  template<class T, class P, class BV, int K>
  inline
  T NodeT<T, P, BV, K>::getDistanceToPrimitives(const Vec3& a_point) const noexcept {
    T minDist = std::numeric_limits<T>::max();

    for (const auto& p : m_primitives){
      const auto curDist = p->signedDistance(a_point);

      if(curDist*curDist < minDist*minDist){
	minDist = curDist;
      }
    }

    return minDist;
  }

  template<class T, class P, class BV, int K>
  inline
  T NodeT<T, P, BV, K>::signedDistance(const Vec3& a_point) const noexcept {
    return this->signedDistance(a_point, Prune::Ordered2);
  }

  template<class T, class P, class BV, int K>
  inline
  T NodeT<T, P, BV, K>::unsignedDistance2(const Vec3& a_point) const noexcept {
    const T d = this->signedDistance(a_point);
    
    return d*d;
  }  

  template<class T, class P, class BV, int K>
  inline
  T NodeT<T, P, BV, K>::signedDistance(const Vec3& a_point, const Prune a_pruning) const noexcept {
    T ret = std::numeric_limits<T>::infinity();
    
    switch(a_pruning){
    case Prune::Ordered:
      {
	this->pruneOrdered(ret, a_point);
	
	break;
      }
    case Prune::Ordered2:
      {
	ret = this->pruneOrdered2(a_point);
	
	break;
      }
    case Prune::Unordered:
      {
	ret = this->pruneUnordered(a_point);
	
	break;
      }
    case Prune::Unordered2:
      {
	ret = this->pruneUnordered2(a_point);
	
	break;
      }
    default:
      std::cerr << "In file EBGeometry_BVHImplem.hpp function NodeT<T, P, BV, K>::signedDistance(Vec3, Prune) -- bad input enum for 'Prune'\n";
    };

    return ret;
  }

  template<class T, class P, class BV, int K>
  inline
  T NodeT<T, P, BV, K>::pruneOrdered(const Vec3& a_point) const noexcept {

    // TLDR: This routine does a an ordered search through the tree, using the signed distance for pruning branches.
    
    T signedDistance = std::numeric_limits<T>::infinity();

    this->pruneOrdered(signedDistance, a_point);

    return signedDistance;
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::pruneOrdered(T& a_shortestDistanceSoFar, const Vec3& a_point) const noexcept  {

    // TLDR: Beginning at some node, this routine descends the branches in the tree. It always descends the branch with the shortest distance
    //       to the bounding volume first. The other branch is investigated only after the full sub-tree beneath the first branch has completed. Since the shortest
    //       distance to primitives is updated underway, there is a decent chance that the secondary subtree can be pruned. Hence why this routine is more efficient
    //       than prunedUnordered.
    switch(m_nodeType){
    case NodeType::Leaf:
      {
	// Compute the shortest signed distance to the primitives in this leaf node. If this is shorter than a_shortestDistanceSoFar, update it. Recall
	// that the comparison requires the absolute value since we're doing the SIGNED distance. 
	const T primDist = this->getDistanceToPrimitives(a_point);

	if(std::abs(primDist) < std::abs(a_shortestDistanceSoFar)){ 
	  a_shortestDistanceSoFar = primDist;
	}
	break;
      }
    case NodeType::Regular: 
      {
	// In this case we need to decide which subtree to move down. First, sort the children nodes by the distance between
	// a_point and the children node's bounding volume. Shortest distance goes first. 
	std::array<std::pair<T, NodePtr>, K> distancesAndNodes;
	for (int i = 0; i < K; i++){
	  distancesAndNodes[i] = std::make_pair(m_children[i]->getDistanceToBoundingVolume(a_point), m_children[i]);
	}

	// Comparator for sorting -- puts the node with the shortest distance to the bounding volume at the front of the vector. 
	auto comparator = [](const std::pair<T, NodePtr>& a_node1, const std::pair<T, NodePtr>& a_node2) -> bool{
	  return std::abs(a_node1.first) < std::abs(a_node2.first);
	};

	std::sort(distancesAndNodes.begin(), distancesAndNodes.end(), comparator);

	// Go through the children nodes -- closest node goes first. We prune branches
	// if the distance to the node's bounding volume is longer than the shortest distance we've found so far. 
	for (int i = 0; i < K; i++){
	  const std::pair<T, NodePtr>& curChildNode = distancesAndNodes[i];
	  
	  // a_shortestDistanceSoFar is the SIGNED distance, so we need the absolute value here. 
	  if(std::abs(curChildNode.first) <= std::abs(a_shortestDistanceSoFar)){ 
	    curChildNode.second->pruneOrdered(a_shortestDistanceSoFar, a_point);
	  }
	  else{ // Prune the rest of the children nodes. 
	    break;
	  }
	}
	break;
      }
    }
  }

  template<class T, class P, class BV, int K>
  inline
  T NodeT<T, P, BV, K>::pruneOrdered2(const Vec3& a_point) const noexcept {

    // TLDR: This routine does an ordered search through the tree, using the squared distance for pruning branches. This is slightly
    //       more efficient than using the signed distance. 

    T shortestSquareDistance = std::numeric_limits<T>::infinity(); // Our starting guess for the distance between a_point and the primitives. 
    std::shared_ptr<const P> closestPrimitive = nullptr;           // This will be a reference to the closest primitive. We only take the signed distance at the end.     

    // Move down the tree, pruning as we go along. This routine does all the comparison tests using the
    // unsigned square distance. When it terminates, closestPrimitive is the primitive which has the shortest
    // unsigned squared distance between itself and the point x. 
    this->pruneOrdered2(shortestSquareDistance, closestPrimitive, a_point); 

    // We have found the closest primitive -- return the signed distance
    return closestPrimitive->signedDistance(a_point);
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::pruneOrdered2(T& a_shortestSquareDistanceSoFar2, std::shared_ptr<const P>& a_closestPrimitiveSoFar, const Vec3& a_point) const noexcept  {
    // TLDR: Beginning at some node, this routine descends  branches in the tree. It always descends the branch with the shortest distance
    //       to the bounding volume first. The other branches are investigated only after the full sub-tree beneath the first branch has completed. Since the shortest
    //       distance to primitives is updated underway, there is a decent chance that the other subtrees can be pruned. Hence why this routine is more efficient
    //       than prunedUnordered. 

    switch(m_nodeType){
    case NodeType::Leaf:
      {
	// If we are at a leaf ndoe, compute the shortest unsigned square distance to the primitives in the node. If this is shorter
	// than a_shortestSquareDistanceSoFar, update the shortest distance and the closest primitive. 
	for (const auto& curPrimitive : m_primitives){
	  const auto curDist2 = curPrimitive->unsignedDistance2(a_point);

	  if(curDist2 < a_shortestSquareDistanceSoFar2){
	    a_shortestSquareDistanceSoFar2 = curDist2;
	    a_closestPrimitiveSoFar        = curPrimitive;
	  }
	}
	break;
      }
    case NodeType::Regular:
      {
	// In this case we are at a regular node, and we need to decide which subtree to move down. First, we sort
	// the children nodes by the distance between a_point and the children node's bounding volume. Shortest
	// distance goes first. 
	std::array<std::pair<T, NodePtr>, K> distancesAndNodes;
	for (int i = 0; i < K; i++){
	  distancesAndNodes[i] = std::make_pair(m_children[i]->getDistanceToBoundingVolume2(a_point), m_children[i]);
	}

	// Sorting criterion. Closest node goes first. 
	auto comparator = [](const std::pair<T, NodePtr>& a_node1, const std::pair<T, NodePtr>& a_node2) -> bool{
	  return std::abs(a_node1.first) < std::abs(a_node2.first);
	};

	std::sort(distancesAndNodes.begin(), distancesAndNodes.end(), comparator);

	// Next, we go through the children nodes -- closest node goes first. We prune branches
	// if the distance to the node's bounding volume is longer than the shortest distance we've found so far. 
	for (int i = 0; i < K; i++){
	  const std::pair<T, NodePtr>& node = distancesAndNodes[i];

	  if(node.first <= a_shortestSquareDistanceSoFar2){
	    node.second->pruneOrdered2(a_shortestSquareDistanceSoFar2, a_closestPrimitiveSoFar, a_point);
	  }
	  else{ // Prune the other subtrees. 
	    break;
	  }
	}
	break;
      }
    }
  }

  template<class T, class P, class BV, int K>
  inline
  T NodeT<T, P, BV, K>::pruneUnordered(const Vec3& a_point) const noexcept {
    // TLDR: This routine does an unordered search through the BVH. It visits nodes in the order in which they were created. This is
    //       way slower than an ordered search. This routine computes the signed distance and uses that in order to prune branches. 
    
    T signedDistance = std::numeric_limits<T>::infinity();

    this->pruneUnordered(signedDistance, a_point);

    return signedDistance;
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::pruneUnordered(T& a_shortestDistanceSoFar, const Vec3& a_point) const noexcept  {

    switch(m_nodeType){
    case NodeType::Leaf:
      {
	// Check if the distance to the primitives in this leaf is shorter than a_shortestDistanceSoFar. If it is, update the
	// shortest distance. 
	const T curSignedDistance = this->getDistanceToPrimitives(a_point);

	if(std::abs(curSignedDistance) < std::abs(a_shortestDistanceSoFar)){
	  a_shortestDistanceSoFar = curSignedDistance;
	}
	break;
      }
    case NodeType::Regular:
      {
	// Investigate subtrees. Prune subtrees if the distance to their bounding volumes are longer than the shortest distance
	// we've found so far. 
	for (const auto& child : m_children){
	  const T distanceToChildBoundingVolume = child->getDistanceToBoundingVolume(a_point);

	  if(std::abs(distanceToChildBoundingVolume) < std::abs(a_shortestDistanceSoFar)){
	    child->pruneUnordered(a_shortestDistanceSoFar, a_point);
	  }
	}
	break;
      }
    }
  }

  template<class T, class P, class BV, int K>
  inline
  T NodeT<T, P, BV, K>::pruneUnordered2(const Vec3& a_point) const noexcept {

    // TLDR: This routine does an unordered search through the BVH. It visits nodes in the order in which they were created. This is
    //       way slower than an ordered search. This routine computes the unsigned square distance and uses that in order to prune branches.  

    T shortestSquareDistance = std::numeric_limits<T>::infinity();   // Our initial guess for the shortest distance so far. 
    std::shared_ptr<const P> closestPrimitive = nullptr;             // After pruneUnordered2 below, this will be the closest primitive. 

    // Move down the tree, pruning as we go along. This routine does all the comparison tests using the
    // unsigned square distance. When it terminates, closestPrimitive is the primitive which has the shortest
    // unsigned squared distance between itself and the point x.     
    this->pruneUnordered2(shortestSquareDistance, closestPrimitive, a_point);

    // We now have the closest primitive -- return the signed distance to it. 
    return closestPrimitive->signedDistance(a_point);
  }

  template<class T, class P, class BV, int K>
  inline
  void NodeT<T, P, BV, K>::pruneUnordered2(T& a_shortestUnsignedSquareDistanceSoFar, std::shared_ptr<const P>& a_closestPrimitiveSoFar, const Vec3& a_point) const noexcept  {

    switch(m_nodeType){
    case NodeType::Leaf:
      {
	// Check if the squared distance to the primitives in this leaf is shorter than a_shortestUnsignedSquareDistanceSoFar. If it is, update the
	// shortest distance and primitive. 
	for (const auto& curPrim : m_primitives){
	  const auto curUnsignedSquareDistance = curPrim->unsignedDistance2(a_point);

	  if(curUnsignedSquareDistance < a_shortestUnsignedSquareDistanceSoFar){
	    a_shortestUnsignedSquareDistanceSoFar = curUnsignedSquareDistance;
	    a_closestPrimitiveSoFar  = curPrim;
	  }
	}
	break;
      }
    case NodeType::Regular:
      {
	// Investigate subtrees. Prune subtrees if the distance to their bounding volumes are longer than the shortest distance
	// we've found so far. 
	for (const auto& child : m_children){
	  const T squaredDistanceToChildBoundingVolume = child->getDistanceToBoundingVolume2(a_point);

	  if(squaredDistanceToChildBoundingVolume < a_shortestUnsignedSquareDistanceSoFar){
	    child->pruneUnordered2(a_shortestUnsignedSquareDistanceSoFar, a_closestPrimitiveSoFar, a_point);
	  }
	}
	break;	
      }
    }
  }

  template<class T, class P, class BV, int K>
  inline
  std::shared_ptr<LinearBVH<T, P, BV, K> > NodeT<T, P, BV, K>::flattenTree() {

    // Create a list of sorted primitives and nodes. 
    std::vector<std::shared_ptr<const P> > sortedPrimitives;
    std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K> > > linearNodes;

    // Track the offset into the linearized node array. 
    unsigned long offset = 0;

    // Flatten recursively. 
    this->flattenTree(linearNodes, sortedPrimitives, offset);

    // Return the root node. 
    return std::make_shared<LinearBVH<T, P, BV, K> >(linearNodes, sortedPrimitives);
  }

  template<class T, class P, class BV, int K>
  inline
  unsigned long NodeT<T, P, BV, K>::flattenTree(std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K> > >& a_linearNodes,
						std::vector<std::shared_ptr<const P> >&                   a_sortedPrimitives,
						unsigned long&                                            a_offset) const noexcept {

    // TLDR: This is the main routine for flattening the hierarchy beneath the current node. When this is called we insert
    //       this node into a_linearNodes and associate the array offsets so that we can find the children in the linearized array.

    // Current node we are dealing with. 
    const auto curNode = a_offset;
    
    // Insert a new node corresponding to this node and provide it with the current bounding volume. 
    a_linearNodes.emplace_back(std::make_shared<LinearNodeT<T, P, BV, K> >());
    a_linearNodes[curNode]->setBoundingVolume(m_boundingVolume);

    a_offset++;        

    switch(m_nodeType){
    case NodeType::Leaf:
      {
	// Insert primitives and offsets.
	a_linearNodes[curNode]->setNumPrimitives   (m_primitives.      size());
	a_linearNodes[curNode]->setPrimitivesOffset(a_sortedPrimitives.size());

	a_sortedPrimitives.insert(a_sortedPrimitives.end(), m_primitives.begin(), m_primitives.end());

	break;
      }
    case NodeType::Regular:
      {
	a_linearNodes[curNode]->setNumPrimitives   (0  );
	a_linearNodes[curNode]->setPrimitivesOffset(0UL);

	// Go through the children nodes and 
	for (int k = 0; k < K; k++){
	  const int offset = m_children[k]->flattenTree(a_linearNodes, a_sortedPrimitives, a_offset);
	  
	  a_linearNodes[curNode]->setChildOffset(offset, k);
	}

	break;
      }
    }

    return curNode;
  }

  template<class T, class P, class BV, int K>
  inline
  LinearNodeT<T, P, BV, K>::LinearNodeT() {

    // Initialize everything. 
    m_boundingVolume   = BV();
    m_primitivesOffset = 0UL;
    m_numPrimitives    = 0;

    for (auto& offset : m_childOffsets){
      offset = 0UL;
    }
  }

  template<class T, class P, class BV, int K>
  inline
  LinearNodeT<T, P, BV, K>::~LinearNodeT() {
  }

  template<class T, class P, class BV, int K>
  inline
  void LinearNodeT<T, P, BV, K>::setBoundingVolume(const BV& a_boundingVolume) noexcept {
    m_boundingVolume = a_boundingVolume;
  }

  template<class T, class P, class BV, int K>
  inline
  void LinearNodeT<T, P, BV, K>::setPrimitivesOffset(const unsigned long a_primitivesOffset) noexcept {
    m_primitivesOffset = a_primitivesOffset;
  }

  template<class T, class P, class BV, int K>
  inline
  void LinearNodeT<T, P, BV, K>::setNumPrimitives(const int a_numPrimitives) noexcept {
    m_numPrimitives = a_numPrimitives;
  }

  template<class T, class P, class BV, int K>
  inline
  void LinearNodeT<T, P, BV, K>::setChildOffset(const unsigned long a_childOffset, const int a_whichChild) noexcept {
    m_childOffsets[a_whichChild] = a_childOffset;
  }  

  template<class T, class P, class BV, int K>
  inline
  const BV& LinearNodeT<T, P, BV, K>::getBoundingVolume() const noexcept {
    return (m_boundingVolume);
  }

  template<class T, class P, class BV, int K>
  inline
  const unsigned long& LinearNodeT<T, P, BV, K>::getPrimitivesOffset() const noexcept {
    return (m_primitivesOffset);
  }

  template<class T, class P, class BV, int K>
  inline
  const unsigned long& LinearNodeT<T, P, BV, K>::getNumPrimitives() const noexcept {
    return (m_numPrimitives);
  }

  template<class T, class P, class BV, int K>
  inline
  const std::array<unsigned long, K>& LinearNodeT<T, P, BV, K>::getChildOffsets() const noexcept {
    return (m_childOffsets);
  }

  template<class T, class P, class BV, int K>
  inline
  bool LinearNodeT<T, P, BV, K>::isLeaf() const noexcept {
    return m_numPrimitives > 0;
  }

  template<class T, class P, class BV, int K>
  inline
  T LinearNodeT<T, P, BV, K>::getDistanceToBoundingVolume(const Vec3& a_point) const noexcept{
    return m_boundingVolume.getDistance(a_point);
  }

  template<class T, class P, class BV, int K>
  inline
  T LinearNodeT<T, P, BV, K>::getDistanceToBoundingVolume2(const Vec3& a_point) const noexcept{
    return m_boundingVolume.getDistance2(a_point);
  }  

  template<class T, class P, class BV, int K>
  inline
  T LinearNodeT<T, P, BV, K>::getDistanceToPrimitives(const Vec3T<T>& a_point, const std::vector<std::shared_ptr<const P> >& a_primitives) const noexcept {
    T minDist = std::numeric_limits<T>::infinity();

    for (unsigned int i = 0; i < m_numPrimitives; i++){
      const T curDist = a_primitives[m_primitivesOffset + i]->signedDistance(a_point);

      if(std::abs(curDist) < std::abs(minDist)){
	minDist = curDist;
      }
    }

    return minDist;
  }

  template<class T, class P, class BV, int K>
  inline
  T LinearNodeT<T, P, BV, K>::getUnsignedDistanceToPrimitives2(const Vec3T<T>& a_point, const std::vector<std::shared_ptr<const P> >& a_primitives) const noexcept {
    T minDist = std::numeric_limits<T>::infinity();

    for (unsigned int i = 0; i < m_numPrimitives; i++){
      const T curDist = a_primitives[m_primitivesOffset + i]->unsignedDistance2(a_point);


      minDist = std::min(curDist, minDist);
    }

    return minDist;
  }  

  template<class T, class P, class BV, int K>
  inline
  LinearBVH<T, P, BV, K>::LinearBVH(const std::vector<std::shared_ptr<const LinearNodeT<T, P, BV, K> > >& a_linearNodes,
				    const std::vector<std::shared_ptr<const P> >&                         a_primitives) {
    m_linearNodes = a_linearNodes;
    m_primitives  = a_primitives;
  }

  template<class T, class P, class BV, int K>
  inline
  LinearBVH<T, P, BV, K>::LinearBVH(const std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K> > >& a_linearNodes,
				    const std::vector<std::shared_ptr<const P> >&                   a_primitives) {

    for (const auto& p : a_linearNodes){
      m_linearNodes.emplace_back(p);
    }
    
    m_primitives  = a_primitives;
  }  

  template<class T, class P, class BV, int K>  
  inline
  LinearBVH<T, P, BV, K>::~LinearBVH() {

  }

  template<class T, class P, class BV, int K>
  inline
  T LinearBVH<T, P, BV, K>::signedDistance(const Vec3& a_point) const noexcept {
    // TLDR: This routine uses ordered traversal along the branches. Rather than calling itself recursively, it uses
    //       a stack for investigating the branches and nodes. We compute the unsigned square distance (which can be slightly faster)
    //       throughout the hierarchy in order to find the leaf node with the closest primitive. 

    // Shortest unsigned square distance. Initialize to something big so
    T minDist2 = std::numeric_limits<T>::infinity();

    // Index of closest leaf node. Initialize to -1 to shut up compiler. 
    unsigned long closest = -1;

    // Create temporary storage and and priority queue (our stack). 
    using NodeAndDist = std::pair<unsigned long, T>;

    std::array<NodeAndDist, K> childsAndDistances;    
    std::stack<NodeAndDist> q;

    // Initialize the stack with the root node. 
    q.emplace(0, m_linearNodes[0]->getDistanceToBoundingVolume2(a_point));

    // Stack loop -- always investigate the one at the top. 
    while(!(q.empty())){

      // Pop the top node off the stack. 
      const auto& curNode = (q.top()).first;
      const auto& bvDist2 = (q.top()).second;

      q.pop();

      // See if we really need to process this node. We only need to do it if its BV is closer than the shortest distance we've found so far. Otherwise
      // we are guaranteed that the distance to the primitives is larger than the shortest distance we've found so far. 
      if(bvDist2 <= minDist2){

	// If it's a leaf node, update the shortest distance so far. 
	if(m_linearNodes[curNode]->isLeaf()){
	  const T primDist2  = m_linearNodes[curNode]->getUnsignedDistanceToPrimitives2(a_point, m_primitives);
	  
	  if(primDist2 < minDist2) {
	    minDist2 = primDist2;
	    closest  = curNode;
	  }
	}
	else{
	  // Compute child indices and their BVH distance to a_point.
	  for (int k = 0; k < K; k++){
	    const unsigned long& curOff = m_linearNodes[curNode]->getChildOffsets()[k];
	    const T        distanceToBV = m_linearNodes[curOff] ->getDistanceToBoundingVolume2(a_point);

	    childsAndDistances[k] = std::make_pair(curOff, distanceToBV);
	  }

	  // Sort the child nodes and put them on the stack. On the next iteration we do the closest node first. This sorting
	  // is critical to the performance of the BVH. 
	  std::sort(childsAndDistances.begin(),
	  	    childsAndDistances.end(),
	  	    [](const std::pair<unsigned long, T>& node1, const std::pair<unsigned long, T>& node2) -> bool {
	  	      return node1.second > node2.second;
	  	    });

	  // Push onto stack if the BV is closer than minDist2.
	  for (const auto& child : childsAndDistances) {
	    if(child.second <= minDist2){
	      q.push(child);
	    }
	  }
	}
      }
    }

    // Only at the end do we compute the SIGNED distance. 
    return m_linearNodes[closest]->getDistanceToPrimitives(a_point, m_primitives);
  }

  template<class T, class P, class BV, int K>
  inline
  T LinearBVH<T, P, BV, K>::unsignedDistance2(const Vec3& a_point) const noexcept {
    const T d = this->signedDistance(a_point);

    return d*d;
  }
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
