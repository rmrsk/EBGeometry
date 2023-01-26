/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_CSGImplem.hpp
  @brief  Implementation of EBGeometry_CSG.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_CSGImplem
#define EBGeometry_CSGImplem

// Our includes
#include "EBGeometry_CSG.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
Union(const std::vector<std::shared_ptr<P>>& a_implicitFunctions) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P>::value, "P must derive from ImplicitFunction<T>");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;
  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<UnionIF<T>>(implicitFunctions);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Union(const std::shared_ptr<P1>& a_implicitFunction1, const std::shared_ptr<P2>& a_implicitFunction2) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P1>::value, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P2>::value, "P2 must derive from ImplicitFunction<T>");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<UnionIF<T>>(implicitFunctions);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P>::value, "P must derive from ImplicitFunction<T>");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;
  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<SmoothUnionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::shared_ptr<P1>& a_implicitFunction1,
            const std::shared_ptr<P2>& a_implicitFunction2,
            const T                    a_smooth) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P1>::value, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P2>::value, "P2 must derive from ImplicitFunction<T>");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<SmoothUnionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P, class BV, size_t K>
std::shared_ptr<ImplicitFunction<T>>
FastUnion(const std::vector<std::shared_ptr<P>>&        a_implicitFunctions,
          const EBGeometry::BVH::BVConstructorT<P, BV>& a_bvConstructor) noexcept
{
  return std::make_shared<EBGeometry::FastUnionIF<T, P, BV, K>>(a_implicitFunctions, a_bvConstructor);
}

template <class T, class P, class BV, size_t K>
std::shared_ptr<ImplicitFunction<T>>
FastSmoothUnion(const std::vector<std::shared_ptr<P>>&        a_implicitFunctions,
                const EBGeometry::BVH::BVConstructorT<P, BV>& a_bvConstructor,
                const T                                       a_smoothLen) noexcept
{
  return std::make_shared<EBGeometry::FastSmoothUnionIF<T, P, BV, K>>(
    a_implicitFunctions, a_bvConstructor, a_smoothLen);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P>::value, "P must derive from ImplicitFunction<T>");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;
  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<IntersectionIF<T>>(implicitFunctions);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::shared_ptr<P1>& a_implicitFunction1, const std::shared_ptr<P2>& a_implicitFunction2) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P1>::value, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P2>::value, "P2 must derive from ImplicitFunction<T>");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<IntersectionIF<T>>(implicitFunctions);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P>::value, "P must derive from ImplicitFunction<T>");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;
  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<SmoothIntersection<T>>(implicitFunctions, a_smooth);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::shared_ptr<P1>& a_implicitFunction1,
                   const std::shared_ptr<P2>& a_implicitFunction2,
                   const T                    a_smooth) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P1>::value, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P2>::value, "P2 must derive from ImplicitFunction<T>");

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<SmoothIntersectionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P, class BV, size_t K>
std::shared_ptr<ImplicitFunction<T>>
FastIntersection(const std::vector<std::shared_ptr<P>>&        a_implicitFunctions,
                 const EBGeometry::BVH::BVConstructorT<P, BV>& a_bvConstructor) noexcept
{
  return std::make_shared<EBGeometry::FastIntersection<T, P, BV, K>>(a_implicitFunctions, a_bvConstructor);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Difference(const std::shared_ptr<P1>& a_implicitFunctionA, const std::shared_ptr<P2>& a_implicitFunctionB) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P1>::value, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P2>::value, "P2 must derive from ImplicitFunction<T>");

  return std::make_shared<DifferenceIF<T>>(a_implicitFunctionA, a_implicitFunctionB);
}

template <class T>
UnionIF<T>::UnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept
{
  for (const auto& prim : a_implicitFunctions) {
    m_implicitFunctions.emplace_back(prim);
  }
}

template <class T>
T
UnionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = std::numeric_limits<T>::infinity();

  for (const auto& prim : m_implicitFunctions) {
    ret = std::min(ret, prim->value(a_point));
  }

  return ret;
}

template <class T>
SmoothUnionIF<T>::SmoothUnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctions,
                                const T                                                     a_smoothLen,
                                const std::function<T(const T& a_, const T& b, const T& s)> a_smoothMin) noexcept
{
  for (const auto& prim : a_implicitFunctions) {
    m_implicitFunctions.emplace_back(prim);
  }

  m_smoothLen = std::max(a_smoothLen, std::numeric_limits<T>::min());
  m_smoothMin = a_smoothMin;
}

template <class T>
T
SmoothUnionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = std::numeric_limits<T>::infinity();

  if (m_implicitFunctions.size() == 1) {
    ret = m_implicitFunctions.front()->value(a_point);
  }
  else if (m_implicitFunctions.size() > 1) {
    T a = std::numeric_limits<T>::infinity();
    T b = std::numeric_limits<T>::infinity();

    for (const auto& implicitFunction : m_implicitFunctions) {
      const T curValue = implicitFunction->value(a_point);

      if (curValue < a) {
        b = a;
        a = curValue;
      }
      else if (curValue < b) {
        b = curValue;
      }
    }

    // Smooth minimum function.
    ret = m_smoothMin(a, b, m_smoothLen);
  }

  return ret;
}

template <class T, class P, class BV, size_t K>
FastUnionIF<T, P, BV, K>::FastUnionIF(const std::vector<std::shared_ptr<P>>& a_distanceFunctions,
                                      const BVConstructor&                   a_bvConstructor)
{
  this->buildTree(a_distanceFunctions, a_bvConstructor);
}

template <class T, class P, class BV, size_t K>
void
FastUnionIF<T, P, BV, K>::buildTree(const std::vector<std::shared_ptr<P>>& a_distanceFunctions,
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
      std::cerr << "FastUnionIF<T, P, BV, K>::buildTree -- not enough primitives to "
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

  m_bvh = root->flattenTree();
}

template <class T, class P, class BV, size_t K>
T
FastUnionIF<T, P, BV, K>::value(const Vec3T<T>& a_point) const noexcept
{
  T minDist = std::numeric_limits<T>::infinity();

  BVH::Updater<P> updater = [&minDist,
                             &a_point](const std::vector<std::shared_ptr<const P>>& a_implicitFunctions) -> void {
    for (const auto& implicitFunction : a_implicitFunctions) {
      minDist = std::min(minDist, implicitFunction->value(a_point));
    }
  };

  BVH::Visiter<Node, T> visiter = [&minDist](const Node& a_node, const T& a_bvDist) -> bool {
    return a_bvDist <= 0.0 || a_bvDist <= minDist;
  };

  BVH::Sorter<Node, T, K> sorter =
    [&a_point](std::array<std::pair<std::shared_ptr<const Node>, T>, K>& a_leaves) -> void {
    std::sort(
      a_leaves.begin(),
      a_leaves.end(),
      [&a_point](const std::pair<std::shared_ptr<const Node>, T>& n1,
                 const std::pair<std::shared_ptr<const Node>, T>& n2) -> bool { return n1.second > n2.second; });
  };

  BVH::MetaUpdater<Node, T> metaUpdater = [&a_point](const Node& a_node) -> T {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  m_bvh->traverse(updater, visiter, sorter, metaUpdater);

  return minDist;
}

template <class T, class P, class BV, size_t K>
const BV&
FastUnionIF<T, P, BV, K>::getBoundingVolume() const noexcept
{
  return m_bvh->getBoundingVolume();
}

template <class T, class P, class BV, size_t K>
FastSmoothUnionIF<T, P, BV, K>::FastSmoothUnionIF(
  const std::vector<std::shared_ptr<P>>&               a_distanceFunctions,
  const BVConstructor&                                 a_bvConstructor,
  const T                                              a_smoothLen,
  const std::function<T(const T&, const T&, const T&)> a_smoothMin) noexcept
  : FastUnionIF<T, P, BV, K>(a_distanceFunctions, a_bvConstructor)
{
  m_smoothLen = std::max(a_smoothLen, std::numeric_limits<T>::min());
  m_smoothMin = a_smoothMin;
}

template <class T, class P, class BV, size_t K>
T
FastSmoothUnionIF<T, P, BV, K>::value(const Vec3T<T>& a_point) const noexcept
{
  // For the smoothed CSG union we use a smooth min operator on the two closest
  // primitives.

  // Closest and next closest primitives.
  T a = std::numeric_limits<T>::infinity();
  T b = std::numeric_limits<T>::infinity();

  BVH::Updater<P> updater =
    [&a, &b, &a_point](const std::vector<std::shared_ptr<const P>>& a_implicitFunctions) -> void {
    for (const auto& implicitFunction : a_implicitFunctions) {
      const auto& d = implicitFunction->value(a_point);

      if (d < a) {
        b = a;
        a = d;
      }
      else if (d < b) {
        b = d;
      }
    }
  };

  BVH::Visiter<Node, T> visiter = [&a, &b](const Node& a_node, const T& a_bvDist) -> bool {
    return a_bvDist <= 0.0 || a_bvDist <= a || a_bvDist <= b;
  };

  BVH::Sorter<Node, T, K> sorter =
    [&a_point](std::array<std::pair<std::shared_ptr<const Node>, T>, K>& a_leaves) -> void {
    std::sort(
      a_leaves.begin(),
      a_leaves.end(),
      [&a_point](const std::pair<std::shared_ptr<const Node>, T>& n1,
                 const std::pair<std::shared_ptr<const Node>, T>& n2) -> bool { return n1.second > n2.second; });
  };

  BVH::MetaUpdater<Node, T> metaUpdater = [&a_point](const Node& a_node) -> T {
    return a_node.getDistanceToBoundingVolume(a_point);
  };

  this->m_bvh->traverse(updater, visiter, sorter, metaUpdater);

  return m_smoothMin(a, b, m_smoothLen);
}

template <class T>
IntersectionIF<T>::IntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept
{
  for (const auto& prim : a_implicitFunctions) {
    m_implicitFunctions.emplace_back(prim);
  }
}

template <class T>
T
IntersectionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = -std::numeric_limits<T>::infinity();

  for (const auto& prim : m_implicitFunctions) {
    ret = std::max(ret, prim->value(a_point));
  }

  return ret;
}

template <class T>
SmoothIntersectionIF<T>::SmoothIntersectionIF(
  const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions,
  const T                                                  a_smoothLen,
  const std::function<T(const T&, const T&, const T&)>&    a_smoothMax) noexcept
{
  for (const auto& prim : a_implicitFunctions) {
    m_implicitFunctions.emplace_back(prim);
  }

  m_smoothLen = std::max(a_smoothLen, std::numeric_limits<T>::min());
  m_smoothMax = a_smoothMax;
}

template <class T>
T
SmoothIntersectionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = std::numeric_limits<T>::infinity();

  if (m_implicitFunctions.size() == 1) {
    ret = m_implicitFunctions.front()->value(a_point);
  }
  else if (m_implicitFunctions.size() > 1) {
    T a = -std::numeric_limits<T>::infinity();
    T b = -std::numeric_limits<T>::infinity();

    for (const auto& implicitFunction : m_implicitFunctions) {
      const T curValue = implicitFunction->value(a_point);

      if (curValue > a) {
        b = a;
        a = curValue;
      }
      else if (curValue > b) {
        b = curValue;
      }
    }

    ret = m_smoothMax(a, b, m_smoothLen);
  }

  return ret;
}

template <class T>
DifferenceIF<T>::DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionA,
                              const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionB) noexcept
{
  m_implicitFunctionA = a_implicitFunctionA;
  m_implicitFunctionB = a_implicitFunctionB;
}

template <class T>
T
DifferenceIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  return std::min(m_implicitFunctionA->value(a_point), -m_implicitFunctionB->value(a_point));
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
