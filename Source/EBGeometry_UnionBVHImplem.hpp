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

template <class T, class P, class BV, size_t K>
UnionBVH<T, P, BV, K>::UnionBVH(const std::vector<std::shared_ptr<P>>& a_distanceFunctions,
                                const bool                             a_flipSign,
                                const BVConstructor&                   a_bvConstructor)
{
  m_flipSign = a_flipSign;

  this->buildTree(a_distanceFunctions, a_bvConstructor);
}

template <class T, class P, class BV, size_t K>
void
UnionBVH<T, P, BV, K>::buildTree(const std::vector<std::shared_ptr<P>>& a_distanceFunctions,
                                 const BVConstructor&                   a_bvConstructor) noexcept
{
  using PrimList    = std::vector<std::shared_ptr<const P>>;
  using BuilderNode = EBGeometry::BVH::NodeT<T, P, BV, K>;

  // This function is a partitioning function taking the input primitives and
  // partitioning them into K subvolumes. Since the SDFs don't themselves have
  // vertices, centroids, etc., we use their bounding volumes as criteria for
  // subdividing them. We do this by computing the spatial centroid of each
  // bounding volume that encloses the SDFs. We then do a spatial subdivision
  // along the longest coordinate into K almost-equal chunks.
  EBGeometry::BVH::PartitionerT<P, BV, K> partitioner =
    [a_bvConstructor](const PrimList& a_primitives) -> std::array<PrimList, K> {
    const size_t numPrimitives = a_primitives.size();

    if (numPrimitives < K) {
      std::cerr << "UnionBVH<T, P, BV, K>::buildTree -- not enough primitives to "
                   "partition into K new nodes\n";
    }

    // 1. Compute the bounding volume centroids for each input SDF.
    std::vector<Vec3T<T>> bvCentroids;
    for (const auto& prim : a_primitives) {
      bvCentroids.emplace_back((a_bvConstructor(prim)).getCentroid());
    }

    // 2. Figure out which coordinate direction has the longest/smallest extent.
    // We split along the longest direction.
    auto lo = Vec3T<T>::infinity();
    auto hi = -Vec3T<T>::infinity();

    for (const auto& c : bvCentroids) {
      lo = min(lo, c);
      hi = max(hi, c);
    }

    const size_t splitDir = (hi - lo).maxDir(true);

    // 3. Sort input primitives based on the centroid location of their bounding
    // volumes. We do this by packing the SDFs and their BV centroids
    //    in a vector which we sort (I love C++).
    using Primitive = std::shared_ptr<const P>;
    using Centroid  = Vec3T<T>;
    using PC        = std::pair<Primitive, Centroid>;

    // Vector pack.
    std::vector<PC> primsAndCentroids;
    for (size_t i = 0; i < a_primitives.size(); i++) {
      primsAndCentroids.emplace_back(a_primitives[i], bvCentroids[i]);
    }

    // Vector sort.
    std::sort(primsAndCentroids.begin(), primsAndCentroids.end(), [splitDir](const PC& sdf1, const PC& sdf2) -> bool {
      return sdf1.second[splitDir] < sdf2.second[splitDir];
    });

    // Vector unpack. The input SDFs are not sorted based on their bounding
    // volume centroids.
    std::vector<Primitive> sortedPrimitives;
    for (size_t i = 0; i < numPrimitives; i++) {
      sortedPrimitives.emplace_back(primsAndCentroids[i].first);
    }

    // 4. Figure out where along the PC vector we should do our spatial splits.
    // We try to balance the chunks.
    const size_t almostEqualChunkSize = numPrimitives / K;
    size_t       remainder            = numPrimitives % K;

    std::array<size_t, K> startIndices;
    std::array<size_t, K> endIndices;

    startIndices[0]   = 0;
    endIndices[K - 1] = numPrimitives;

    for (size_t i = 1; i < K; i++) {
      startIndices[i] = startIndices[i - 1] + almostEqualChunkSize;

      if (remainder > 0) {
        startIndices[i]++;
        remainder--;
      }
    }

    for (size_t i = 0; i < K - 1; i++) {
      endIndices[i] = startIndices[i + 1];
    }

    // 5. Put the primitives in separate lists and return them like the API
    // says.
    std::array<PrimList, K> subVolumePrimitives;
    for (size_t i = 0; i < K; i++) {

      // God how I hate how the standard library handles iterator offsets. Fuck
      // you, long/unsigned long conversion.
      typename PrimList::const_iterator first =
        sortedPrimitives.cbegin() + static_cast<typename PrimList::difference_type>(startIndices[i]);
      typename PrimList::const_iterator last =
        sortedPrimitives.cbegin() + static_cast<typename PrimList::difference_type>(endIndices[i]);

      subVolumePrimitives[i] = PrimList(first, last);
    }

    return subVolumePrimitives;
  };

  // Stop function. Exists subdivision if there are not enough primitives left
  // to keep subdividing. We set the limit at 10 primitives.
  EBGeometry::BVH::StopFunctionT<T, P, BV, K> stopFunc = [](const BuilderNode& a_node) -> bool {
    const size_t numPrimsInNode = (a_node.getPrimitives()).size();
    return numPrimsInNode < K;
  };

  // Init the root node and partition the primitives.
  auto root = std::make_shared<BuilderNode>(a_distanceFunctions);

  root->topDownSortAndPartitionPrimitives(a_bvConstructor, partitioner, stopFunc);

  m_rootNode = root->flattenTree();
}

template <class T, class P, class BV, size_t K>
T
UnionBVH<T, P, BV, K>::value(const Vec3T<T>& a_point) const noexcept
{
  const T sign = (m_flipSign) ? -1.0 : 1.0;

  // For the CSG union we select the smallest value. This means that if a
  // point is inside one object but outside another one, we choose the
  // inside value. By design, LinearNode is a signed distance object, but the
  // BVH accelerator is still a valid accelerator that permits us to do all
  // kinds of accelerated traversals. For overlapping objects there is not signed
  // distance but there is still a CSG union, and the BVH is still a useful thing.
  // So, we can't use LinearNode::signedDistanceFunction because it returns the
  // closest object and not the object with the smallest value function.
  //
  // Fortunately, when I wrote the LinearNode accelerator I wrote it such that we can determine
  // how to update the "shortest" distance using externally supplied criteria.
  // So, we just update this as f = min(f1,f2,f3) etc, and prune nodes accordingly.
  // The criteria for that are below...

  BVH::Comparator<T> unionComparator = [](const T& dmin, const std::vector<T>& primDistances) -> T {
    T d = dmin;

    for (const auto& primDist : primDistances) {
      d = std::min(d, primDist);
    }

    return d;
  };

  BVH::Pruner<T> unionPruner = [](const T& bvDist, const T& minDist) -> bool {
    return bvDist <= 0.0 || bvDist <= minDist;
  };

  const T value = m_rootNode->stackPrune(a_point, unionComparator, unionPruner);

  return sign * value;
}

template <class T, class P, class BV, size_t K>
const BV&
UnionBVH<T, P, BV, K>::getBoundingVolume() const noexcept
{
  return m_rootNode->getBoundingVolume();
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
