// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_CSGImplem.hpp
 * @brief  Implementation of EBGeometry_CSG.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_CSGIMPLEM_HPP
#define EBGEOMETRY_CSGIMPLEM_HPP

// Std includes
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_CSG.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Transform.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Internal helpers shared by BVHUnionIF and BVHSmoothUnionIF
 */
namespace CSGDetail {

/**
 * @brief Build a PackedBVH from primitive/BV pairs using the requested strategy.
 * @details Internal helper; not part of the public API.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type.
 * @tparam BV Bounding volume type (e.g. AABBT<T>).
 * @tparam K  BVH branching factor.
 * @param[in] a_primsAndBVs Primitives and their bounding volumes (must be non-empty).
 * @param[in] a_build       BVH construction strategy.
 * @return Shared pointer to the packed BVH root.
 */
template <class T, class P, class BV, size_t K>
[[nodiscard]] inline std::shared_ptr<EBGeometry::BVH::PackedBVH<T, P, K>>
buildBVH(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs, const BVH::Build a_build) noexcept
{
  static_assert(std::is_floating_point_v<T>, "CSGDetail::buildBVH requires a floating-point type T");

  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  auto root = std::make_shared<EBGeometry::BVH::TreeBVH<T, P, BV, K>>(a_primsAndBVs);

  switch (a_build) {
  case BVH::Build::TopDown: {
    root->topDownSortAndPartition();

    break;
  }
  case BVH::Build::Morton: {
    root->template bottomUpSortAndPartition<SFC::Morton>();

    break;
  }
  case BVH::Build::Nested: {
    root->template bottomUpSortAndPartition<SFC::Nested>();

    break;
  }
  case BVH::Build::SAH: {
    using Node     = EBGeometry::BVH::TreeBVH<T, P, BV, K>;
    using LeafPred = typename Node::LeafPredicate;

    const LeafPred stopCrit = [](const Node& n) noexcept -> bool { return n.getPrimitives().size() < K; };

    root->topDownSortAndPartition(EBGeometry::BVH::BinnedSAHPartitioner<T, P, BV, K>, stopCrit);

    break;
  }
  default: {
    EBGEOMETRY_EXPECT(false);

    break;
  }
  }

  return root->pack();
}

} // namespace CSGDetail

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
Union(const std::vector<std::shared_ptr<P>>& a_implicitFunctions)
{
  static_assert(std::is_floating_point_v<T>, "Union requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(!a_implicitFunctions.empty());

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.reserve(a_implicitFunctions.size());

  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<UnionIF<T>>(implicitFunctions);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Union(const std::shared_ptr<P1>& a_implicitFunction1, const std::shared_ptr<P2>& a_implicitFunction2)
{
  static_assert(std::is_floating_point_v<T>, "Union requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(a_implicitFunction1 != nullptr);
  EBGEOMETRY_EXPECT(a_implicitFunction2 != nullptr);

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<UnionIF<T>>(implicitFunctions);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth)
{
  static_assert(std::is_floating_point_v<T>, "SmoothUnion requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(!a_implicitFunctions.empty());
  EBGEOMETRY_EXPECT(a_smooth > T(0));

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.reserve(a_implicitFunctions.size());

  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<SmoothUnionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::shared_ptr<P1>& a_implicitFunction1,
            const std::shared_ptr<P2>& a_implicitFunction2,
            const T                    a_smooth)
{
  static_assert(std::is_floating_point_v<T>, "SmoothUnion requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(a_implicitFunction1 != nullptr);
  EBGEOMETRY_EXPECT(a_implicitFunction2 != nullptr);
  EBGEOMETRY_EXPECT(a_smooth > T(0));

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<SmoothUnionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P, class BV, size_t K>
std::shared_ptr<ImplicitFunction<T>>
BVHUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const std::vector<BV>& a_boundingVolumes)
{
  static_assert(std::is_floating_point_v<T>, "BVHUnion requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(!a_implicitFunctions.empty());
  EBGEOMETRY_EXPECT(a_implicitFunctions.size() == a_boundingVolumes.size());

  return std::make_shared<EBGeometry::BVHUnionIF<T, P, BV, K>>(a_implicitFunctions, a_boundingVolumes);
}

template <class T, class P, class BV, size_t K>
std::shared_ptr<ImplicitFunction<T>>
BVHSmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions,
               const std::vector<BV>&                 a_boundingVolumes,
               const T                                a_smoothLen) noexcept
{
  static_assert(std::is_floating_point_v<T>, "BVHSmoothUnion requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(!a_implicitFunctions.empty());
  EBGEOMETRY_EXPECT(a_implicitFunctions.size() == a_boundingVolumes.size());
  EBGEOMETRY_EXPECT(a_smoothLen > T(0));

  return std::make_shared<EBGeometry::BVHSmoothUnionIF<T, P, BV, K>>(
    a_implicitFunctions, a_boundingVolumes, a_smoothLen);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions)
{
  static_assert(std::is_floating_point_v<T>, "Intersection requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(!a_implicitFunctions.empty());

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.reserve(a_implicitFunctions.size());

  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<IntersectionIF<T>>(implicitFunctions);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::shared_ptr<P1>& a_implicitFunction1, const std::shared_ptr<P2>& a_implicitFunction2)
{
  static_assert(std::is_floating_point_v<T>, "Intersection requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(a_implicitFunction1 != nullptr);
  EBGEOMETRY_EXPECT(a_implicitFunction2 != nullptr);

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<IntersectionIF<T>>(implicitFunctions);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth)
{
  static_assert(std::is_floating_point_v<T>, "SmoothIntersection requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(!a_implicitFunctions.empty());
  EBGEOMETRY_EXPECT(a_smooth > T(0));

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  for (const auto& f : a_implicitFunctions) {
    implicitFunctions.emplace_back(f);
  }

  return std::make_shared<SmoothIntersectionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::shared_ptr<P1>& a_implicitFunction1,
                   const std::shared_ptr<P2>& a_implicitFunction2,
                   const T                    a_smooth)
{
  static_assert(std::is_floating_point_v<T>, "SmoothIntersection requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(a_implicitFunction1 != nullptr);
  EBGEOMETRY_EXPECT(a_implicitFunction2 != nullptr);
  EBGEOMETRY_EXPECT(a_smooth > T(0));

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.emplace_back(a_implicitFunction1);
  implicitFunctions.emplace_back(a_implicitFunction2);

  return std::make_shared<SmoothIntersectionIF<T>>(implicitFunctions, a_smooth);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Difference(const std::shared_ptr<P1>& a_implicitFunctionA, const std::shared_ptr<P2>& a_implicitFunctionB)
{
  static_assert(std::is_floating_point_v<T>, "Difference requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(a_implicitFunctionA != nullptr);
  EBGEOMETRY_EXPECT(a_implicitFunctionB != nullptr);

  return std::make_shared<DifferenceIF<T>>(a_implicitFunctionA, a_implicitFunctionB);
}

template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothDifference(const std::shared_ptr<P1>& a_implicitFunctionA,
                 const std::shared_ptr<P2>& a_implicitFunctionB,
                 const T                    a_smoothLen)
{
  static_assert(std::is_floating_point_v<T>, "SmoothDifference requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P1>, "P1 must derive from ImplicitFunction<T>");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P2>, "P2 must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(a_implicitFunctionA != nullptr);
  EBGEOMETRY_EXPECT(a_implicitFunctionB != nullptr);
  EBGEOMETRY_EXPECT(a_smoothLen > T(0));

  return std::make_shared<SmoothDifferenceIF<T>>(a_implicitFunctionA, a_implicitFunctionB, a_smoothLen);
}

template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
FiniteRepetition(const std::shared_ptr<P>& a_implicitFunction,
                 const Vec3T<T>&           a_period,
                 const Vec3T<T>&           a_repeatLo,
                 const Vec3T<T>&           a_repeatHi)
{
  static_assert(std::is_floating_point_v<T>, "FiniteRepetition requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "P must derive from ImplicitFunction<T>");

  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  for (size_t i = 0; i < 3; i++) {
    EBGEOMETRY_EXPECT(a_period[i] > T(0));
  }

  return std::make_shared<FiniteRepetitionIF<T>>(a_implicitFunction, a_period, a_repeatLo, a_repeatHi);
}

template <class T>
UnionIF<T>::UnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions)
{
  EBGEOMETRY_EXPECT(!a_implicitFunctions.empty());

  for (const auto& prim : a_implicitFunctions) {
    EBGEOMETRY_EXPECT(prim != nullptr);

    m_implicitFunctions.emplace_back(prim);
  }
}

template <class T>
T
UnionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = std::numeric_limits<T>::infinity();

  for (const auto& prim : m_implicitFunctions) {
    const T v = prim->value(a_point);

    EBGEOMETRY_EXPECT(!std::isnan(v));

    ret = std::min(ret, v);
  }

  return ret;
}

template <class T>
SmoothUnionIF<T>::SmoothUnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctions,
                                const T                                                     a_smoothLen,
                                const std::function<T(const T& a_, const T& b, const T& s)> a_smoothMin)
{
  EBGEOMETRY_EXPECT(!a_implicitFunctions.empty());
  EBGEOMETRY_EXPECT(a_smoothLen > T(0));

  for (const auto& prim : a_implicitFunctions) {
    EBGEOMETRY_EXPECT(prim != nullptr);

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

    EBGEOMETRY_EXPECT(!std::isnan(ret));
  }
  else if (m_implicitFunctions.size() > 1) {
    T a = std::numeric_limits<T>::infinity();
    T b = std::numeric_limits<T>::infinity();

    for (const auto& implicitFunction : m_implicitFunctions) {
      const T curValue = implicitFunction->value(a_point);

      EBGEOMETRY_EXPECT(!std::isnan(curValue));

      if (curValue < a) {
        b = a;
        a = curValue;
      }
      else if (curValue < b) {
        b = curValue;
      }
    }

    ret = m_smoothMin(a, b, m_smoothLen);
  }

  return ret;
}

template <class T, class P, class BV, size_t K>
BVHUnionIF<T, P, BV, K>::BVHUnionIF(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs)
{
  EBGEOMETRY_EXPECT(!a_primsAndBVs.empty());

  for ([[maybe_unused]] const auto& [prim, bv] : a_primsAndBVs) {
    EBGEOMETRY_EXPECT(prim != nullptr);
  }

  this->buildTree(a_primsAndBVs);
}

template <class T, class P, class BV, size_t K>
BVHUnionIF<T, P, BV, K>::BVHUnionIF(const std::vector<std::shared_ptr<P>>& a_primitives,
                                    const std::vector<BV>&                 a_boundingVolumes)
{
  EBGEOMETRY_EXPECT(!a_primitives.empty());
  EBGEOMETRY_EXPECT(a_primitives.size() == a_boundingVolumes.size());

  for ([[maybe_unused]] const auto& p : a_primitives) {
    EBGEOMETRY_EXPECT(p != nullptr);
  }

  std::vector<std::pair<std::shared_ptr<const P>, BV>> primsAndBVs;

  primsAndBVs.reserve(a_primitives.size());

  for (size_t i = 0; i < a_primitives.size(); i++) {
    primsAndBVs.emplace_back(std::make_pair(a_primitives[i], a_boundingVolumes[i]));
  }

  this->buildTree(primsAndBVs);
}

template <class T, class P, class BV, size_t K>
void
BVHUnionIF<T, P, BV, K>::buildTree(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs,
                                   const BVH::Build                                            a_build) noexcept
{
  m_bvh = CSGDetail::buildBVH<T, P, BV, K>(a_primsAndBVs, a_build);
}

template <class T, class P, class BV, size_t K>
T
BVHUnionIF<T, P, BV, K>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  T minDist = std::numeric_limits<T>::infinity();

  const auto& primitives = m_bvh->getPrimitives();

  const auto evalLeaf = [&primitives, &a_point](T& a_minDist, size_t a_offset, size_t a_count) noexcept -> void {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T v = primitives[i]->value(a_point);

      EBGEOMETRY_EXPECT(!std::isnan(v));

      a_minDist = std::min(a_minDist, v);
    }
  };

  // pruneTraverse prunes a node when its squared distance to a_point exceeds this bound, and visits
  // children nearest-first via a SIMD child-box test. The original four-callback prune kept a node
  // iff its (unsigned) bounding-volume distance was <= 0 or <= minDist, i.e. iff it was within
  // max(0, minDist); squaring that bound reproduces exactly the same set of visited nodes. When the
  // running best is negative (a_point is inside a primitive) the bound collapses to 0, so only nodes
  // whose box contains a_point are descended into -- identical to the previous behaviour.
  const auto pruneDist2 = [](const T& a_minDist) noexcept -> T {
    const T bound = std::max(T(0), a_minDist);

    return bound * bound;
  };

  m_bvh->pruneTraverse(a_point, minDist, evalLeaf, pruneDist2);

  return minDist;
}

template <class T, class P, class BV, size_t K>
const EBGeometry::BoundingVolumes::AABBT<T>&
BVHUnionIF<T, P, BV, K>::getBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return m_bvh->getBoundingVolume();
}

template <class T, class P, class BV, size_t K>
BVHSmoothUnionIF<T, P, BV, K>::BVHSmoothUnionIF(
  const std::vector<std::shared_ptr<P>>&               a_distanceFunctions,
  const std::vector<BV>&                               a_boundingVolumes,
  const T                                              a_smoothLen,
  const std::function<T(const T&, const T&, const T&)> a_smoothMin) noexcept
{
  EBGEOMETRY_EXPECT(!a_distanceFunctions.empty());
  EBGEOMETRY_EXPECT(a_distanceFunctions.size() == a_boundingVolumes.size());
  EBGEOMETRY_EXPECT(a_smoothLen > T(0));

  for ([[maybe_unused]] const auto& p : a_distanceFunctions) {
    EBGEOMETRY_EXPECT(p != nullptr);
  }

  std::vector<std::pair<std::shared_ptr<const P>, BV>> primsAndBVs;

  primsAndBVs.reserve(a_distanceFunctions.size());

  for (size_t i = 0; i < a_distanceFunctions.size(); i++) {
    primsAndBVs.emplace_back(std::make_pair(a_distanceFunctions[i], a_boundingVolumes[i]));
  }

  this->buildTree(primsAndBVs);

  m_smoothLen = std::max(a_smoothLen, std::numeric_limits<T>::min());
  m_smoothMin = a_smoothMin;
}

template <class T, class P, class BV, size_t K>
void
BVHSmoothUnionIF<T, P, BV, K>::buildTree(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs,
                                         const BVH::Build                                            a_build) noexcept
{
  m_bvh = CSGDetail::buildBVH<T, P, BV, K>(a_primsAndBVs, a_build);
}

template <class T, class P, class BV, size_t K>
T
BVHSmoothUnionIF<T, P, BV, K>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  // The smooth union blends only the two nearest primitive values, so the traversal state tracks the
  // two smallest values seen so far (a <= b).
  struct Closest
  {
    T a = std::numeric_limits<T>::infinity();
    T b = std::numeric_limits<T>::infinity();
  };

  Closest closest;

  const auto& primitives = m_bvh->getPrimitives();

  const auto evalLeaf = [&primitives, &a_point](Closest& a_closest, size_t a_offset, size_t a_count) noexcept -> void {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T d = primitives[i]->value(a_point);

      EBGEOMETRY_EXPECT(!std::isnan(d));

      if (d < a_closest.a) {
        a_closest.b = a_closest.a;
        a_closest.a = d;
      }
      else if (d < a_closest.b) {
        a_closest.b = d;
      }
    }
  };

  // Prune against the second-smallest value b, not the nearest a: a primitive whose box is farther
  // than b cannot be one of the two the blend uses, but one between a and b still can and must not be
  // pruned. This is the smooth-union counterpart of BVHUnionIF's single-nearest bound, and squaring
  // max(0, b) reproduces exactly the original four-callback prune (kept iff bvDist <= 0 || <= a ||
  // <= b, which reduces to <= b since a <= b). Loosening it further by the smoothing length is
  // unnecessary: the result depends only on the two smallest values, and the b-bound provably
  // retains both -- extra nodes it would admit hold only values larger than b, which cannot change a
  // or b.
  const auto pruneDist2 = [](const Closest& a_closest) noexcept -> T {
    const T bound = std::max(T(0), a_closest.b);

    return bound * bound;
  };

  m_bvh->pruneTraverse(a_point, closest, evalLeaf, pruneDist2);

  return m_smoothMin(closest.a, closest.b, m_smoothLen);
}

template <class T, class P, class BV, size_t K>
const EBGeometry::BoundingVolumes::AABBT<T>&
BVHSmoothUnionIF<T, P, BV, K>::getBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  return m_bvh->getBoundingVolume();
}

template <class T>
IntersectionIF<T>::IntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept
{
  EBGEOMETRY_EXPECT(!a_implicitFunctions.empty());

  for (const auto& prim : a_implicitFunctions) {
    EBGEOMETRY_EXPECT(prim != nullptr);

    m_implicitFunctions.emplace_back(prim);
  }
}

template <class T>
T
IntersectionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = -std::numeric_limits<T>::infinity();

  for (const auto& prim : m_implicitFunctions) {
    const T v = prim->value(a_point);

    EBGEOMETRY_EXPECT(!std::isnan(v));

    ret = std::max(ret, v);
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
  EBGEOMETRY_EXPECT(a_implicitFunctionA != nullptr);
  EBGEOMETRY_EXPECT(a_implicitFunctionB != nullptr);
  EBGEOMETRY_EXPECT(a_smoothLen > T(0));

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
  EBGEOMETRY_EXPECT(!a_implicitFunctions.empty());
  EBGEOMETRY_EXPECT(a_smoothLen > T(0));

  for (const auto& prim : a_implicitFunctions) {
    EBGEOMETRY_EXPECT(prim != nullptr);

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

    EBGEOMETRY_EXPECT(!std::isnan(ret));
  }
  else if (m_implicitFunctions.size() > 1) {
    T a = -std::numeric_limits<T>::infinity();
    T b = -std::numeric_limits<T>::infinity();

    for (const auto& implicitFunction : m_implicitFunctions) {
      const T curValue = implicitFunction->value(a_point);

      EBGEOMETRY_EXPECT(!std::isnan(curValue));

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
  EBGEOMETRY_EXPECT(a_implicitFunctionA != nullptr);
  EBGEOMETRY_EXPECT(a_implicitFunctionB != nullptr);

  m_implicitFunctionA = a_implicitFunctionA;
  m_implicitFunctionB = a_implicitFunctionB;
}

template <class T>
DifferenceIF<T>::DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>&              a_implicitFunctionA,
                              const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctionsB) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunctionA != nullptr);
  EBGEOMETRY_EXPECT(!a_implicitFunctionsB.empty());

  m_implicitFunctionA = a_implicitFunctionA;
  m_implicitFunctionB = EBGeometry::Union<T>(a_implicitFunctionsB);
}

template <class T>
T
DifferenceIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  const T a = m_implicitFunctionA->value(a_point);
  const T b = m_implicitFunctionB->value(a_point);

  EBGEOMETRY_EXPECT(!std::isnan(a));
  EBGEOMETRY_EXPECT(!std::isnan(b));

  return std::max(a, -b);
}

template <class T>
SmoothDifferenceIF<T>::SmoothDifferenceIF(
  const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
  const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionB,
  const T                                                     a_smoothLen,
  const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunctionA != nullptr);
  EBGEOMETRY_EXPECT(a_implicitFunctionB != nullptr);
  EBGEOMETRY_EXPECT(a_smoothLen > T(0));

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
  EBGEOMETRY_EXPECT(a_implicitFunctionA != nullptr);
  EBGEOMETRY_EXPECT(!a_implicitFunctionsB.empty());
  EBGEOMETRY_EXPECT(a_smoothLen > T(0));

  std::vector<std::shared_ptr<ImplicitFunction<T>>> implicitFunctions;

  implicitFunctions.reserve(1 + a_implicitFunctionsB.size());
  implicitFunctions.emplace_back(a_implicitFunctionA);

  for (const auto& subtractedFunction : a_implicitFunctionsB) {
    EBGEOMETRY_EXPECT(subtractedFunction != nullptr);

    implicitFunctions.emplace_back(EBGeometry::Complement<T>(subtractedFunction));
  }

  m_smoothIntersectionIF = std::make_shared<SmoothIntersectionIF<T>>(implicitFunctions, a_smoothLen, a_smoothMax);
}

template <class T>
T
SmoothDifferenceIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_smoothIntersectionIF != nullptr);

  return m_smoothIntersectionIF->value(a_point);
}

template <class T>
FiniteRepetitionIF<T>::FiniteRepetitionIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                                          const Vec3T<T>&                             a_period,
                                          const Vec3T<T>&                             a_repeatLo,
                                          const Vec3T<T>&                             a_repeatHi) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  for (size_t i = 0; i < 3; i++) {
    EBGEOMETRY_EXPECT(a_period[i] > T(0));
  }

  m_implicitFunction = a_implicitFunction;
  m_period           = a_period;
  m_repeatLo         = a_repeatLo;
  m_repeatHi         = a_repeatHi;
}

template <class T>
T
FiniteRepetitionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  Vec3T<T> q;

  for (size_t i = 0; i < 3; i++) {
    q[i] = a_point[i] - m_period[i] * std::round(std::clamp((a_point[i] / m_period[i]), -m_repeatLo[i], m_repeatHi[i]));
  }

  const T ret = m_implicitFunction->value(q);

  EBGEOMETRY_EXPECT(!std::isnan(ret));

  return ret;
}

} // namespace EBGeometry

#endif
