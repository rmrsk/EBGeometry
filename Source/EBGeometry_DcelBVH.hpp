/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DcelBVH.hpp
  @brief  File which contains partitioners and lambdas for enclosing DCEL faces in bounding volume heirarchies
  @details This file contains various useful "default" routines for determining how a DCEL mesh should be partitioned in a bounding volume hierarchy. This includes
  the required functions for 
  1) Constructing bounding volumes (defaultBVConstructor).
  2) Stopping the sub-division process (defaultStopFunction)
  3) Partitioning one bounding volume into subvolumes. 
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
    @brief Alias for vector of primitives.
  */
  template <class T>
  using PrimitiveList = std::vector<std::shared_ptr<const EBGeometry::Dcel::FaceT<T> > >;

  /*!
    @brief Bounding volume constructor for a DCEL face. 
    @details With BVHs and DCEL, the object to be bounded is the polygon face (e.g., triangle). We assume that our BV constructor can 
    enclose points, so we return an object that encloses all the vertices of the polygon. 
    @param[in] a_primitive Primitive (facet) to be bounded.
    @return Returns a bounding volume which encloses the input face. 
  */
  template <class T, class BV>
  EBGeometry::BVH::BVConstructorT<EBGeometry::Dcel::FaceT<T>, BV> defaultBVConstructor = [](const std::shared_ptr<const EBGeometry::Dcel::FaceT<T> >& a_primitive) -> BV {
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
  EBGeometry::BVH::StopFunctionT<T, EBGeometry::Dcel::FaceT<T>, BV, K> defaultStopFunction = [](const BVH::NodeT<T, EBGeometry::Dcel::FaceT<T>, BV, K>& a_node) -> bool {
    return (a_node.getPrimitives()).size() < K;
  };

  /*!
    @brief Default partitioner function for subdividing into K sub-volumes
    @param[in] a_primitives List of primitives to partition into sub-bounding volumes
    @details This is a very stupid partitioner which splits into equal chunks along the longest coordinate. 
  */
  template <class T, int K>
  EBGeometry::BVH::PartitionerT<EBGeometry::Dcel::FaceT<T>, K> equalChunkPartitioner = [](const PrimitiveList<T>& a_primitives) -> std::array<PrimitiveList<T>, K> {
    Vec3T<T> lo =  Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();
    for (const auto& p : a_primitives){
      lo = min(lo, p->getCentroid());
      hi = max(hi, p->getCentroid());
    }
    
    const int splitDir = (hi-lo).maxDir(true);

    // Sort the primitives along the above coordinate direction. 
    PrimitiveList<T> sortedPrimitives(a_primitives);
    
    std::sort(sortedPrimitives.begin(), sortedPrimitives.end(),
	      [=](const std::shared_ptr<const FaceT<T> >& f1,
		  const std::shared_ptr<const FaceT<T> >& f2) -> bool {
		return f1->getCentroid(splitDir) < f2->getCentroid(splitDir);
	      });

    // Put the sorted primitives into K equally sized chunks. 
    size_t length = a_primitives.size() / K;
    size_t remain = a_primitives.size() % K;

    size_t begin  = 0;
    size_t end    = 0;

    std::array<PrimitiveList<T>, K> chunks;
    
    for (size_t k = 0; k < K; k++){
      end += (remain > 0) ? (length + !!(remain--)) : length;      

      chunks[k] = PrimitiveList<T>(sortedPrimitives.begin() + begin, sortedPrimitives.begin() + end);

      begin = end;
    }

    return chunks;    
  };

  /*!
    @brief Alias for default partitioner. 
  */
  template <class T, int K>
  EBGeometry::BVH::PartitionerT<EBGeometry::Dcel::FaceT<T>, K> defaultPartitioner = EBGeometry::Dcel::equalChunkPartitioner<T, K>;

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
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
