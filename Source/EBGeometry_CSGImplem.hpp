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
#include "EBGeometry_Transform.hpp"
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
FastUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const std::vector<BV>& a_boundingVolumes) noexcept
{
  return std::make_shared<EBGeometry::FastUnionIF<T, P, BV, K>>(a_implicitFunctions, a_boundingVolumes);
}

template <class T, class P, class BV, size_t K>
std::shared_ptr<ImplicitFunction<T>>
FastSmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions,
                const std::vector<BV>&                 a_boundingVolumes,
                const T                                a_smoothLen) noexcept
{
  return std::make_shared<EBGeometry::FastSmoothUnionIF<T, P, BV, K>>(
    a_implicitFunctions, a_boundingVolumes, a_smoothLen);
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

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Difference(const std::shared_ptr<P1>& a_implicitFunctionA, const std::shared_ptr<P2>& a_implicitFunctionB) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P1>::value, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P2>::value, "P2 must derive from ImplicitFunction<T>");

  return std::make_shared<DifferenceIF<T>>(a_implicitFunctionA, a_implicitFunctionB);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothDifference(const std::shared_ptr<P1>& a_implicitFunctionA,
                 const std::shared_ptr<P2>& a_implicitFunctionB,
                 const T                    a_smoothLen) noexcept
{
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P1>::value, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P2>::value, "P2 must derive from ImplicitFunction<T>");

  return std::make_shared<SmoothDifferenceIF<T>>(a_implicitFunctionA, a_implicitFunctionB, a_smoothLen);
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
FastUnionIF<T, P, BV, K>::FastUnionIF(
  const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs) noexcept
{
  this->buildTree(a_primsAndBVs);
}

template <class T, class P, class BV, size_t K>
FastUnionIF<T, P, BV, K>::FastUnionIF(const std::vector<std::shared_ptr<P>>& a_primitives,
                                      const std::vector<BV>&                 a_boundingVolumes) noexcept
{
  if (a_primitives.size() != a_boundingVolumes.size()) {
    std::cerr << "FastUnionIF(std::vector, std::vector) - input arguments not same length!\n";
  }
  else {
    std::vector<std::pair<std::shared_ptr<const P>, BV>> primsAndBVs;

    for (size_t i = 0; i < a_primitives.size(); i++) {
      primsAndBVs.emplace_back(std::make_pair(a_primitives[i], a_boundingVolumes[i]));
    }

    this->buildTree(primsAndBVs);
  }
}

template <class T, class P, class BV, size_t K>
void
FastUnionIF<T, P, BV, K>::buildTree(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs) noexcept
{
  // Init the root node, partition it, and flatten it.
  auto root = std::make_shared<EBGeometry::BVH::NodeT<T, P, BV, K>>(a_primsAndBVs);

  root->topDownSortAndPartition();

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
  const std::vector<BV>&                               a_boundingVolumes,
  const T                                              a_smoothLen,
  const std::function<T(const T&, const T&, const T&)> a_smoothMin) noexcept
  : FastUnionIF<T, P, BV, K>(a_distanceFunctions, a_boundingVolumes)
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
  const std::shared_ptr<ImplicitFunction<T>>&           a_implicitFunctionA,
  const std::shared_ptr<ImplicitFunction<T>>&           a_implicitFunctionB,
  const T                                               a_smoothLen,
  const std::function<T(const T&, const T&, const T&)>& a_smoothMax) noexcept
{

  m_implicitFunctions.emplace_back(a_implicitFunctionA);
  m_implicitFunctions.emplace_back(a_implicitFunctionB);

  m_smoothLen = std::max(a_smoothLen, std::numeric_limits<T>::min());
  m_smoothMax = a_smoothMax;
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
DifferenceIF<T>::DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>&              a_implicitFunctionA,
                              const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctionsB) noexcept
{
  m_implicitFunctionA = a_implicitFunctionA;
  m_implicitFunctionB = EBGeometry::Union<T>(a_implicitFunctionsB);
}

template <class T>
T
DifferenceIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  return std::max(m_implicitFunctionA->value(a_point), -m_implicitFunctionB->value(a_point));
}

template <class T>
SmoothDifferenceIF<T>::SmoothDifferenceIF(
  const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
  const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionB,
  const T                                                     a_smoothLen,
  const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax) noexcept
{

  m_smoothIntersectionIF = std::make_shared<SmoothIntersectionIF<T>>(
    a_implicitFunctionA, EBGeometry::Complement<T>(a_implicitFunctionB), a_smoothLen, a_smoothMax);
}

template <class T>
SmoothDifferenceIF<T>::SmoothDifferenceIF(
  const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
  const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctionsB,
  const T                                                     a_smoothLen,
  const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax) noexcept
{

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunctionA);

  for (const auto& subtractedFunction : a_implicitFunctionsB) {
    implicitFunctions.emplace_back(EBGeometry::Complement<T>(subtractedFunction));
  }

  m_smoothIntersectionIF = std::make_shared<SmoothIntersectionIF<T>>(implicitFunctions, a_smoothLen, a_smoothMax);
}

template <class T>
T
SmoothDifferenceIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  return m_smoothIntersectionIF->value(a_point);
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
