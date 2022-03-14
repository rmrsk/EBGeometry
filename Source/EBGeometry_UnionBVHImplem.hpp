/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_UnionBVHImplem.hpp
  @brief  Implementation of EBGeometry_UnionBVH.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_UnionBVHImplem
#define EBGeometry_UnionBVHImplem

// Our includes
#include "EBGeometry_UnionBVH.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T, class BV, int K>
UnionBVH<T, BV, K>::UnionBVH(const std::vector<std::shared_ptr<SDF> >& a_distanceFunctions, const bool a_flipSign) {
  for (const auto& sdf : a_distanceFunctions){
    m_distanceFunctions.emplace_back(sdf);
  }
  
  m_flipSign = a_flipSign;
  m_isGood   = false;
}

template <class T, class BV, int K>
UnionBVH<T, BV, K>::UnionBVH(const std::vector<std::shared_ptr<SDF> >& a_distanceFunctions, const bool a_flipSign, const BVConstructor& a_bvConstructor) :
  UnionBVH<T, BV, K>(a_distanceFunctions, a_flipSign) {
  
  this->buildTree(a_bvConstructor);
}

template <class T, class BV, int K>
void UnionBVH<T, BV, K>::buildTree(const BVConstructor& a_bvConstructor) {
  
  // This function is a partitioning function taking the input SDFs and partitioning them into K subvolumes. Since the SDFs don't themselves
  // have vertices, centroids, etc., we use their bounding volumes as criteria for subdividing them. We do this by computing the spatial centroid
  // of each bounding volume that encloses the SDFs. We then do a spatial subdivision along the longest coordinate into K almost-equal chunks. 
  EBGeometry::BVH::PartitionerT<SDF, BV, K> partitioner = [a_bvConstructor] (const SDFList& a_primitives) -> std::array<SDFList, K> {
    const int numPrimitives = a_primitives.size();

    if(numPrimitives < K){
      std::cerr << "UnionBVH<T, BV, K>::buildTree -- not enough primitives to partition into K new nodes\n";
    }

    // 1. Compute the bounding volume centroids for each input SDF. 
    std::vector<Vec3T<T> > bvCentroids;
    for (const auto& P : a_primitives){
      bvCentroids.emplace_back((a_bvConstructor(P)).getCentroid());
    }

    // 2. Figure out which coordinate direction has the longest/smallest extent. We split along the longest direction. 
    auto lo =  Vec3T<T>::infinity();
    auto hi = -Vec3T<T>::infinity();

    for (const auto& c : bvCentroids){
      lo = min(lo, c);
      hi = max(hi, c);
    }

    const int splitDir = (hi-lo).maxDir(true);

    // 3. Sort input primitives based on the centroid location of their bounding volumes. We do this by packing the SDFs and their BV centroids
    //    in a vector which we sort (I love C++).
    using Primitive = std::shared_ptr<const SDF>;
    using Centroid  = Vec3T<T>;
    using PC        = std::pair<Primitive, Centroid>;

    // Vector pack. 
    std::vector<PC> primsAndCentroids;
    for (unsigned int i = 0; i < a_primitives.size(); i++){
      primsAndCentroids.emplace_back(a_primitives[i], bvCentroids[i]);
    }

    // Vector sort. 
    std::sort(primsAndCentroids.begin(),
    	      primsAndCentroids.end(),
    	      [splitDir](const PC& sdf1, const PC& sdf2) -> bool {
    		return sdf1.second[splitDir] < sdf2.second[splitDir];
	      });

    // Vector unpack. The input SDFs are not sorted based on their bounding volume centroids. 
    std::vector<Primitive> sortedPrimitives;
    for (int i = 0 ; i < numPrimitives; i++){
      sortedPrimitives.emplace_back(primsAndCentroids[i].first);
    }

    // 4. Figure out where along the PC vector we should do our spatial splits. We try to balance the chunks. 
    const int almostEqualChunkSize = numPrimitives / K;
    int       remainder            = numPrimitives % K;

    std::array<int, K> startIndices;
    std::array<int, K> endIndices;    

    startIndices[0] = 0;
    endIndices  [K-1] = numPrimitives;
    
    for (unsigned int i = 1; i < K; i++){
      startIndices[i] = startIndices[i-1] + almostEqualChunkSize;

      if(remainder > 0){
	startIndices[i]++;
	remainder--;
      }
    }
    
    for (unsigned int i = 0; i < K-1; i++){
      endIndices[i] = startIndices[i+1];
    }

    // 5. Put the primitives in separate lists and return them like the API says. 
    std::array<SDFList, K> subVolumePrimitives;
    for (int i = 0; i < K; i++){
      typename SDFList::const_iterator first = sortedPrimitives.begin() + startIndices[i];
      typename SDFList::const_iterator last  = sortedPrimitives.begin() + endIndices  [i];
      
      subVolumePrimitives[i] = SDFList(first, last);
    }  
    
    return subVolumePrimitives;
  };

  // Stop function. Exists subdivision if there are not enough primitives left to keep subdividing. We set the limit at 10 primitives. 
  EBGeometry::BVH::StopFunctionT<T, SDF, BV, K> stopFunc = [] (const BuilderNode& a_node) -> bool {
    const int numPrimsInNode = (a_node.getPrimitives()).size();
    return numPrimsInNode < K;
  };

  // Init the root node and partition the primitives. 
  auto root = std::make_shared<BuilderNode>(m_distanceFunctions);
  
  root->topDownSortAndPartitionPrimitives(a_bvConstructor,
					  partitioner,
					  stopFunc);

  m_rootNode = root->flattenTree();
  
  m_isGood = true;
}

template <class T, class BV, int K>
T UnionBVH<T, BV, K>::signedDistance(const Vec3T<T>& a_point) const noexcept {
  const T sign = (m_flipSign) ? -1.0 : 1.0;

  return sign * m_rootNode->signedDistance(a_point);  
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
