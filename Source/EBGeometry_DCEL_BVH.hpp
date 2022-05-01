/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL_BVH.hpp
  @brief  File which contains partitioners and lambdas for enclosing DCEL faces
  in bounding volume heirarchies
  @details This file contains various useful "default" routines for determining
  how a DCEL mesh should be partitioned in a bounding volume hierarchy. This
  includes the required functions for 1) Constructing bounding volumes
  (defaultBVConstructor). 2) Stopping the sub-division process
  (defaultStopFunction) 3) Partitioning one bounding volume into subvolumes.
  @author Robert Marskar
*/

#ifndef EBGeometry_DCEL_BVH
#define EBGeometry_DCEL_BVH

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

/*!
  @brief Alias for vector of primitives.
*/
template <class T>
using PrimitiveList = std::vector<std::shared_ptr<const EBGeometry::DCEL::FaceT<T>>>;

/*!
  @brief Bounding volume constructor for a DCEL face.
  @details With BVHs and DCEL, the object to be bounded is the polygon face
  (e.g., triangle). We assume that our BV constructor can enclose points, so we
  return an object that encloses all the vertices of the polygon.
  @param[in] a_primitive Primitive (facet) to be bounded.
  @return Returns a bounding volume which encloses the input face.
*/
template <class T, class BV>
EBGeometry::BVH::BVConstructorT<EBGeometry::DCEL::FaceT<T>, BV> defaultBVConstructor =
  [](const std::shared_ptr<const EBGeometry::DCEL::FaceT<T>>& a_primitive) -> BV {
  return BV(a_primitive->getAllVertexCoordinates());
};

/*!
  @brief Default stop function. This function terminates the division process if
  a BVH node has only one primitive.
  @details In this function, BVH::NodeT<T, FaceT<T>, BVH > is a BVH node. The
  interpretation of the parameters are: T is the precision, FaceT<T> is the
  primitive type in the BVH tree, and BV is the bounding volume type.
  @param[in] a_node Bounding volume hierarchy node.
  @return Returns true if the bounding volume shouldn't be split more and false
  otherwise.
*/
template <class T, class BV, size_t K>
EBGeometry::BVH::StopFunctionT<T, EBGeometry::DCEL::FaceT<T>, BV, K> defaultStopFunction =
  [](const BVH::NodeT<T, EBGeometry::DCEL::FaceT<T>, BV, K>& a_node) -> bool {
  return (a_node.getPrimitives()).size() < K;
};

/*!
  @brief Function which checks that all chunks are valid (i.e., contain at least
  one primitive
  @param[in] a_chunks Chunks.
*/
template <class T, size_t K>
auto validChunks = [](const std::array<PrimitiveList<T>, K>& a_chunks) -> bool {
  for (const auto& chunk : a_chunks) {
    if (chunk.empty())
      return false;
  }

  return true;
};

/*!
  @brief Function for partitioning an input list into K almost-equal-sized
  chunks
  @param[in] a_primitives Primitives to be partitioned.
*/
template <class T, size_t K>
auto equalCounts = [](const PrimitiveList<T>& a_primitives) -> std::array<PrimitiveList<T>, K> {
  int length = a_primitives.size() / K;
  int remain = a_primitives.size() % K;

  int begin = 0;
  int end = 0;

  std::array<PrimitiveList<T>, K> chunks;

  for (size_t k = 0; k < K; k++) {
    end += (remain > 0) ? length + 1 : length;
    remain--;

    chunks[k] = PrimitiveList<T>(a_primitives.begin() + begin, a_primitives.begin() + end);

    begin = end;
  }

  return chunks;
};

/*!
  @brief Partitioner function for subdividing into K sub-volumes with
  approximately the same number of primitives.
  @details This partitioner splits along one of the axis coordinates and sorts
  the primitives along the centroid.
  @param[in] a_primitives List of primitives to partition into sub-bounding
  volumes
*/
template <class T, class BV, size_t K>
EBGeometry::BVH::PartitionerT<EBGeometry::DCEL::FaceT<T>, BV, K> chunkPartitioner =
  [](const PrimitiveList<T>& a_primitives) -> std::array<PrimitiveList<T>, K> {
  Vec3T<T> lo = Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();
  for (const auto& p : a_primitives) {
    lo = min(lo, p->getCentroid());
    hi = max(hi, p->getCentroid());
  }

  const size_t splitDir = (hi - lo).maxDir(true);

  // Sort the primitives along the above coordinate direction.
  PrimitiveList<T> sortedPrimitives(a_primitives);

  std::sort(
    sortedPrimitives.begin(), sortedPrimitives.end(),
    [splitDir](const std::shared_ptr<const FaceT<T>>& f1, const std::shared_ptr<const FaceT<T>>& f2) -> bool {
      return f1->getCentroid(splitDir) < f2->getCentroid(splitDir);
    });

  return EBGeometry::DCEL::equalCounts<T, K>(sortedPrimitives);
};

/*!
  @brief Partitioner function for subdividing into K sub-volumes with
  approximately the same number of primitives.
  @details Basically the same as chunkPartitioner, except that the centroids are
  based on the bounding volumes' centroids.
  @param[in] a_primitives List of primitives to partition into sub-bounding
  volumes
*/
template <class T, class BV, size_t K>
EBGeometry::BVH::PartitionerT<EBGeometry::DCEL::FaceT<T>, BV, K> bvPartitioner =
  [](const PrimitiveList<T>& a_primitives) -> std::array<PrimitiveList<T>, K> {
  Vec3T<T> lo = Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();

  // Pack primitives and their bounding volumes together.
  using P = std::pair<std::shared_ptr<const EBGeometry::DCEL::FaceT<T>>, BV>;

  std::vector<P> primsAndBVs;

  for (const auto& p : a_primitives) {
    const BV bv(p->getAllVertexCoordinates());

    primsAndBVs.emplace_back(p, bv);

    lo = min(lo, bv.getCentroid());
    hi = max(hi, bv.getCentroid());
  }

  const size_t splitDir = (hi - lo).maxDir(true);

  // Sort the primitives based on the centroid location of their BVs.
  std::sort(primsAndBVs.begin(), primsAndBVs.end(), [splitDir](const P& p1, const P& p2) {
    return (p1.second).getCentroid()[splitDir] < (p2.second).getCentroid()[splitDir];
  });

  // Unpack the vector and partition into equal counts.
  PrimitiveList<T> sortedPrimitives;
  for (const auto& p : primsAndBVs) {
    sortedPrimitives.emplace_back(p.first);
  }

  primsAndBVs.resize(0);

  return EBGeometry::DCEL::equalCounts<T, K>(sortedPrimitives);
};

/*!
  @brief Partitioner function for subdividing into K sub-volumes, partitioning
  on the primitive centroid midpoint(s).
  @param[in] a_primitives List of primitives to partition into sub-bounding
  volumes
*/
template <class T, class BV, size_t K>
EBGeometry::BVH::PartitionerT<EBGeometry::DCEL::FaceT<T>, BV, K> centroidPartitioner =
  [](const PrimitiveList<T>& a_primitives) -> std::array<PrimitiveList<T>, K> {
  Vec3T<T> lo = Vec3T<T>::max();
  Vec3T<T> hi = -Vec3T<T>::max();
  for (const auto& p : a_primitives) {
    lo = min(lo, p->getCentroid());
    hi = max(hi, p->getCentroid());
  }

  const size_t splitDir = (hi - lo).maxDir(true);
  const T delta = (hi - lo)[splitDir] / K;

  std::array<T, K> boundsLo;
  std::array<T, K> boundsHi;

  for (size_t k = 0; k < K; k++) {
    boundsLo[k] = lo[splitDir] + delta * k;
    boundsHi[k] = lo[splitDir] + delta * (k + 1);
  }

  // Given coord, find the bin.
  auto getBin = [&](const T& coord) -> size_t {
    for (size_t k = 0; k < K; k++) {
      if (coord >= boundsLo[k] && coord <= boundsHi[k]) {
        return k;
      }
    }

    return K - 1;
  };

  // Put primitives in their respective bins.
  std::array<PrimitiveList<T>, K> chunks;

  for (const auto& p : a_primitives) {
    const size_t k = getBin(p->getCentroid()[splitDir]);
    chunks[k].push_back(p);
  }

  // The centroid-based partitioner can end up with no primitives in one of the
  // leaves (a rare case). Use a different partitioner in that case.
  if (!(EBGeometry::DCEL::validChunks<T, K>(chunks))) {
    chunks = EBGeometry::DCEL::chunkPartitioner<T, BV, K>(a_primitives);
  }

  return chunks;
};

/*!
  @brief Alias for default partitioner.
*/
template <class T, class BV, size_t K>
EBGeometry::BVH::PartitionerT<EBGeometry::DCEL::FaceT<T>, BV, K> defaultPartitioner =
  EBGeometry::DCEL::chunkPartitioner<T, BV, K>;
} // namespace DCEL

#include "EBGeometry_NamespaceFooter.hpp"

#endif
