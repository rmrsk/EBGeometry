/* EBGeometry
 * Copyright © Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL_BVH.hpp
  @brief  File which contains partitioners and lambdas for enclosing DCEL faces in bounding volume hierarchies
  @author Robert Marskar
*/

#ifndef EBGeometry_DCEL_BVH
#define EBGeometry_DCEL_BVH

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  /*!
    @brief Alias for the primitive type bounded in the BVH. In this case a DCEL face with floating-point precision T
  */
  template <class T>
  using Prim = EBGeometry::DCEL::FaceT<T>;

  /*!
    @brief Alias for a vector of DCEL::FaceT<T> primitives and associated bounding volumes
  */
  template <class T, class BV>
  using PrimAndBVList = std::vector<std::pair<std::shared_ptr<const Prim<T>>, BV>>;

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
  EBGeometry::BVH::StopFunctionT<T, Prim<T>, BV, K> defaultStopFunction =
    [](const BVH::NodeT<T, Prim<T>, BV, K>& a_node) -> bool { return (a_node.getPrimitives()).size() < K; };

  /*!
    @brief Chunk-partitioner based on DCEL::FaceT<T> centroids
  */
  template <class T, class BV, size_t K>
  EBGeometry::BVH::PartitionerT<Prim<T>, BV, K> chunkPartitioner =
    [](const PrimAndBVList<T, BV>& a_primsAndBVs) -> std::array<PrimAndBVList<T, BV>, K> {
    Vec3T<T> lo = Vec3T<T>::max();
    Vec3T<T> hi = -Vec3T<T>::max();

    for (const auto& pbv : a_primsAndBVs) {
      lo = min(lo, pbv.first->getCentroid());
      hi = max(hi, pbv.first->getCentroid());
    }

    const size_t splitDir = (hi - lo).maxDir(true);

    // Sort the primitives along the above coordinate direction.
    PrimAndBVList<T, BV> sortedPrimsAndBVs(a_primsAndBVs);

    using PrimAndBV = std::pair<std::shared_ptr<const Prim<T>>, BV>;

    std::sort(sortedPrimsAndBVs.begin(),
              sortedPrimsAndBVs.end(),
              [splitDir](const PrimAndBV& pbv1, const PrimAndBV& pbv2) -> bool {
                return pbv1.first->getCentroid(splitDir) < pbv2.first->getCentroid(splitDir);
              });

    return BVH::equalCounts<PrimAndBV, K>(sortedPrimsAndBVs);
  };

  /*!
    @brief Alias for default partitioner.
  */
  template <class T, class BV, size_t K>
  EBGeometry::BVH::PartitionerT<Prim<T>, BV, K> defaultPartitioner = EBGeometry::DCEL::chunkPartitioner<T, BV, K>;

  /*!
    @brief One-liner for turning a DCEL mesh into a full-tree BVH. 
    @param[in] a_dcelMesh Input DCEL mesh. 
  */
  template <class T, class BV, size_t K>
  std::shared_ptr<EBGeometry::BVH::NodeT<T, FaceT<T>, BV, K>>
  buildFullBVH(const std::shared_ptr<EBGeometry::DCEL::MeshT<T>>& a_dcelMesh)
  {
    // Create the list of bounding volumes of primitives.
    PrimAndBVList<T, BV> primsAndBVs;

    for (const auto& f : a_dcelMesh->getFaces()) {
      primsAndBVs.emplace_back(std::make_pair(f, BV(f->getAllVertexCoordinates())));
    }

    // Partition the BVH using the default input arguments.
    auto bvh = std::make_shared<EBGeometry::BVH::NodeT<T, Prim<T>, BV, K>>(primsAndBVs);

    bvh->topDownSortAndPartition(EBGeometry::DCEL::defaultPartitioner<T, BV, K>,
                                 EBGeometry::DCEL::defaultStopFunction<T, BV, K>);

    return bvh;
  }
} // namespace DCEL

#include "EBGeometry_NamespaceFooter.hpp"

#endif
