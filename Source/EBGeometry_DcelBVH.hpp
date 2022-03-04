/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DcelBVH.hpp
  @brief  File which contains partitioners and lambdas for enclosing dcel_face in bounding volume heirarchies
  @details This file contains various useful "default" routines for determining how a DCEL mesh should be partitioned in a bounding volume hierarchy. This includes
  the required functions for 
  1) Constructing bounding volumes (defaultBVConstructor).
  2) Stopping the sub-division process (defaultStopFunction)
  3) Partitioning one bounding volume into subvolumes. These are the functions
  a) defaultPartitionFunction(...). This partitions the primitives into two half-spaces based on where the primitive centroids are located.
  b) partitionMinimumOverlap(...).  This splist the primitive list down the middle (ignoring centroids and element size) and selects the splitting direction where the
  sub-bounding volumes have the smallest overlap. 
  3) partitionSAH(...). This implements the common "surface area heuristic" rule for constructing bounding volumes. 
  @author Robert Marskar
*/

#ifndef EBGeometry_DcelBVH
#define EBGeometry_DcelBVH

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_DcelFace.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace Dcel {

  /*!
    @brief This is the lowest number of a primitives that a BVH node is allowed to enclose. 
  */
  constexpr int primitivesPerLeafNode = 1;

  /*!
    @brief Alias for which primitives are used in the BVH. For DCEL meshes the primitive is a polygon face.  
  */
  template <class T>
  using PrimitiveList = std::vector<std::shared_ptr<const Dcel::FaceT<T> > >;

  /*!
    @brief Bounding volume constructor for a primitive list. 
    @details In DCEL, the input list is a list of FaceT<T> objects. BV is the bounding volume class which encloses the primitives. It is a template parameter which
    can be constructed from a list of 3D coordinates that it must enclose (had this been C++20, we REALLY would have use concepts here).
    @param[in] a_primitives List of primitives to enclose in a bounding volume. 
    @return Returns a bounding volume which encloses the input primitives list. 
  */
  template <class T, class BV>
  BVH::BVConstructorT<FaceT<T>, BV> defaultBVConstructor = [](const std::shared_ptr<const FaceT<T> >& a_primitive){
    return BV(a_primitive->getAllVertexCoordinates());
  };  

  /*!
    @brief Default stop function. This function terminates the division process if a BVH node has only one primitive. 
    @details In this function, BVH::NodeT<T, FaceT<T>, BVH > is a BVH node. The interpretation of the parameters are: T is the precision, FaceT<T> is the primitive type
    in the BVH tree, and BV is the bounding volume type. 
    @param[in] a_node Bounding volume hierarchy node. 
    @return Returns true if the bounding volume shouldn't be split more and false otherwise. 
  */
  template <class T, class BV, int K> 
  BVH::StopFunctionT<T, FaceT<T>, BV, K> defaultStopFunction = [](const BVH::NodeT<T, FaceT<T>, BV, K>& a_node){
    const auto& primitives = a_node.getPrimitives();
    const int numPrims     = primitives.size();
    
    return numPrims <= primitivesPerLeafNode || numPrims < K;
  };

  /*!
    @brief Default partitioner function for subdividing into K sub-volumes
    @param[in] a_primitives List of primitives to partition into sub-bounding volumes
    @details This is a very stupid partitioner which splits into equal chunks along the longest coordinate. 
  */
  template <class T, int K>
  BVH::PartitionerT<FaceT<T>, K> spatialSplitPartitioner = [](const PrimitiveList<T>& a_primitives){

    const int numPrimitives = a_primitives.size();
    
    if(numPrimitives < K){
      std::cerr << "In file EBGeometry_DcelBVH.H function BVH::PartitionerT<FaceT<T>, K>::spatialSplitPartitioner -- not enough primitives to subdivide into subvolumes\n";
    }
    
    // Compute the coordinate direction with the longest extent. This will be our splitting direction.  
    auto lo = Vec3T<T>::max();
    auto hi = Vec3T<T>::min();
    for (const auto& p : a_primitives){
      lo = min(lo, p->getCentroid());
      hi = max(hi, p->getCentroid());
    }
    const auto delta   = (hi-lo);
    const int splitDir = delta.maxDir(true);

    // Sort the primitives along the above coordinate direction. 
    PrimitiveList<T> sortedPrimitives(a_primitives);
    
    std::sort(sortedPrimitives.begin(), sortedPrimitives.end(),
	      [=](const std::shared_ptr<const FaceT<T> >& f1, const std::shared_ptr<const FaceT<T> >& f2) -> bool {
		return f1->getCentroid(splitDir) < f2->getCentroid(splitDir);
	      });


    // Figure out the indices in the vector where we do the splits. 
    const int almostEqualChunkSize = numPrimitives / K;
    int remainder                  = numPrimitives % K;

    std::array<int, K> startIndices;
    std::array<int, K> endIndices;

    startIndices[0]   = 0;
    endIndices  [K-1] = numPrimitives;
    
    for (int i = 1; i < K; i++){
      startIndices[i] = startIndices[i-1] + almostEqualChunkSize;

      if(remainder > 0){
	startIndices[i]++;
	remainder--;
      }
    }
    for (int i = 0; i < K-1; i++){
      endIndices[i] = startIndices[i+1];
    }


    // Put the primitives in separate lists and return them back to the BVH node. 
    std::array<PrimitiveList<T>, K> subVolumePrimitives;
    for (int i = 0; i < K; i++){
      typename PrimitiveList<T>::const_iterator first = sortedPrimitives.begin() + startIndices[i];
      typename PrimitiveList<T>::const_iterator last  = sortedPrimitives.begin() + endIndices  [i];
      
      subVolumePrimitives[i] = PrimitiveList<T>(first, last);
    }

    // Return as we must.
    return subVolumePrimitives;
  };

  /*!
    @brief Binary partitioner based on spatial splits. 
    @param[in] a_primitives List of primitives to partition into sub-bounding volumes
    @details This is a partitioner that calls the spatialSplitPartitioner in order to recursively subdivided volumes into 2^K subvolumes. 
  */  
  template <class T, int K>
  BVH::PartitionerT<FaceT<T>, K> spatialSplitBinaryPartitioner = [](const PrimitiveList<T>& a_primitives){
    const int numPrimitives = a_primitives.size();
    
    if(numPrimitives < K){
      std::cerr << "In file EBGeometry_DcelBVH.H function BVH::PartitionerT<FaceT<T>, K>::spatialSplitBinaryPartitioner -- not enough primitives to subdivide into subvolumes\n";
    }

    // Error -- this is a binary partitioner!
    constexpr bool isPowerOfTwo = (K > 0) && ((K & (K-1)) == 0);
    if(!isPowerOfTwo){
      std::cerr << "In file EBGeometry_DcelBVH.H function BVH::PartitionerT<FaceT<T>, K>::spatialSplitBinaryPartitioner -- template parameter K is not a factor of 2!!!";
    }

    // Initialize the list of subvolumes. 
    std::vector<PrimitiveList<T> > subVolumeList(1);
    subVolumeList[0] = a_primitives;

    int numSubVolumes = 1;
    while (numSubVolumes < K){

      std::vector<PrimitiveList<T> > newSubVolumeList(0);
      for (const auto& subVolume : subVolumeList){
	const std::array<PrimitiveList<T>, 2>& binarySplit = spatialSplitPartitioner<T, 2>(subVolume);

	newSubVolumeList.push_back(binarySplit[0]);
	newSubVolumeList.push_back(binarySplit[1]);	
      }

      subVolumeList = newSubVolumeList;
      
      numSubVolumes *= 2;
    }

    // We have a vector but need an array:
    std::array<PrimitiveList<T>, K> subVolumePrimitives;
    for (int i = 0; i < K; i++){
      subVolumePrimitives[i] = subVolumeList[i];
    }

    return subVolumePrimitives;
  };

  /*!
    @brief Volume overlap partitioning function. 
    @param[in] a_primitives List of primitives. 
    @details This checks all 3 spatial coordinates, sorting the primitives along the coordinate direction and divides them into two halves with the same number of primitives. 
    The coordinate direction which yielded the smallest volumetric overlap between the two corresponding bounding volumes is used. 
    @return Returns a pair of primitives. The first/second entry in the pair is the primitives contain in the left/right sub-bounding volumes
  */  
  // template <class T, class BV>
  // BVH::PartitionFunctionT<FaceT<T> > partitionMinimumOverlap = [](const PrimitiveList<T>& a_primitives){
  //   constexpr int DIM = 3;

  //   // Always split the input primitives down the middle. 
  //   const int splitIndex    = (a_primitives.size() - 1)/2;

  //   // This is the smallest volumetric overlap so far. 
  //   T minOverlap = std::numeric_limits<T>::infinity();

  //   // This is the return list. 
  //   std::pair<PrimitiveList<T>, PrimitiveList<T> > ret;

  //   // For each coordinate direction, sort the primitives along the coordinate and compute the bounding volumes
  //   for (int dir = 0; dir < DIM; dir++){

  //     PrimitiveList<T> sortedPrims(a_primitives);
  //     std::sort(sortedPrims.begin(), sortedPrims.end(),
  // 		[=](const std::shared_ptr<const FaceT<T> >& f1, const std::shared_ptr<const FaceT<T> >& f2){
  // 		  return f1->getCentroid(dir) < f2->getCentroid(dir);
  // 		});

  //     // Put the sorted primitives into separate lists
  //     PrimitiveList<T> lPrims(sortedPrims.begin(), sortedPrims.begin() + splitIndex+1);
  //     PrimitiveList<T> rPrims(sortedPrims.begin() + splitIndex + 1, sortedPrims.end());

  //     // Compute the bounding volumes for the left and right subvolumes. 
  //     const BV leftBV  = defaultBVConstructor<T, BV>(lPrims);
  //     const BV rightBV = defaultBVConstructor<T, BV>(rPrims);

  //     // Compute the overlapping volume between the left and right bounding volumes. 
  //     const T curOverlap = getOverlappingVolume(leftBV, rightBV);

  //     // Keep the one with the smallest volume. 
  //     if (curOverlap < minOverlap){
  // 	minOverlap = curOverlap;

  // 	ret = std::make_pair(lPrims, rPrims);
  //     }
  //   }

  //   return ret;
  // };

  // /*!
  //   @brief Surface area heuristic (SAH) partitioning function. 
  //   @param[in] a_primitives List of primitives. 
  //   @return Returns a pair of primitives. The first/second entry in the pair is the primitives contain in the left/right sub-bounding volumes
  // */    
  // template <class T, class BV>
  // BVH::PartitionFunctionT<FaceT<T> > partitionSAH = [](const PrimitiveList<T>& a_primitives){
  //   constexpr int DIM   = 3; 
  //   constexpr int nBins = 16;
  //   constexpr T invBins = 1./nBins;
  //   constexpr T Ct      = 0.0;
  //   constexpr T Ci      = 1.0;

  //   const auto curBV   = defaultBVConstructor<T, BV>(a_primitives);
  //   const auto curArea = curBV.getArea();

  //   auto lo = Vec3T<T>::max();
  //   auto hi = Vec3T<T>::min();
  //   for (const auto& p : a_primitives){
  //     lo = min(lo, p->getCentroid());
  //     hi = max(hi, p->getCentroid());
  //   }
  //   const auto delta = (hi-lo)*invBins;

  //   T minCost = std::numeric_limits<T>::max();

  //   std::pair<PrimitiveList<T>, PrimitiveList<T> > ret;
  
  //   for (int dir = 0; dir < DIM; dir++){

  //     for (int ibin = 0; ibin <= nBins; ibin++){
  // 	const Vec3T<T> pos = lo + T(1.0*ibin)*delta;

  // 	PrimitiveList<T> lPrims;
  // 	PrimitiveList<T> rPrims;
	
  // 	for (const auto& p : a_primitives){
  // 	  if(p->getCentroid()[dir] <= pos[dir]){
  // 	    lPrims.emplace_back(p);
  // 	  }
  // 	  else{
  // 	    rPrims.emplace_back(p);
  // 	  }
  // 	}

  // 	const auto numLeft  = lPrims.size();
  // 	const auto numRight = rPrims.size();
	
  // 	if(numLeft == 0 || numRight == 0) continue;

  // 	const BV bvLeft  = defaultBVConstructor<T, BV>(lPrims);
  // 	const BV bvRight = defaultBVConstructor<T, BV>(rPrims);

  // 	const T leftArea  = bvLeft.getArea();
  // 	const T rightArea = bvRight.getArea();

  // 	const T C = Ct + (leftArea/curArea)*Ci*numLeft + (rightArea/curArea)*Ci*numRight;

  // 	if(C < minCost){
  // 	  minCost = C;
  // 	  ret     = std::make_pair(lPrims, rPrims);
  // 	}
  //     }
  //   }

  //   return ret;
  // };
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
