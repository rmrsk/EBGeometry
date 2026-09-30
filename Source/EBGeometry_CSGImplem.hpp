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
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_CSG.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_SFC.hpp"
#include "EBGeometry_Transform.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Internal helpers shared by BVHUnionIF and BVHSmoothUnionIF
 */
namespace CSGDetail {

/**
 * @brief Build the packed BVH of a BVH union using the requested strategy.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 * @param[in,out] a_pool            Pool to reserve the BVH from.
 * @param[in]     a_primitives      Primitives (must be non-empty).
 * @param[in]     a_boundingVolumes Bounding box of each primitive.
 * @param[in]     a_build           BVH construction strategy.
 * @return The packed BVH.
 */
template <class T, class P, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
inline BVH::PackedBVH<T, P, K>
buildBVH(Pool&                                         a_pool,
         const std::vector<P>&                         a_primitives,
         const std::vector<BoundingVolumes::AABBT<T>>& a_boundingVolumes,
         const BVH::Build                              a_build)
{
  using BV   = BoundingVolumes::AABBT<T>;
  using Root = BVH::PackedBVH<T, P, K>;

  // Each check guards against a mistake that a Release build would otherwise turn into a silent wrong
  // answer or an out-of-bounds read, and runs once at build time, costing nothing per evaluation.
  EBGEOMETRY_REQUIRE(!a_primitives.empty(), "BVHUnionIF: a union needs at least one primitive");

  EBGEOMETRY_REQUIRE(a_primitives.size() == a_boundingVolumes.size(),
                     "BVHUnionIF: need one bounding volume per primitive (%zu primitives, %zu bounding volumes)",
                     a_primitives.size(),
                     a_boundingVolumes.size());

  // A pool-resident primitive is evaluated against the union's own pool location, so it must have
  // been reserved from the same pool; a primitive from another pool would be read from the wrong
  // memory.
  if constexpr (IsPoolResident<P>::value) {
    for (size_t i = 0; i < a_primitives.size(); i++) {
      EBGEOMETRY_REQUIRE(a_primitives[i].isAttachedTo(a_pool),
                         "BVHUnionIF: primitive %zu of %zu lives in another Pool; a pool-resident primitive must "
                         "be built in the union's own Pool",
                         i,
                         a_primitives.size());
    }
  }

  std::vector<std::pair<P, BV>> primsAndBVs;

  primsAndBVs.reserve(a_primitives.size());

  for (size_t i = 0; i < a_primitives.size(); i++) {
    primsAndBVs.emplace_back(a_primitives[i], a_boundingVolumes[i]);
  }

  switch (a_build) {
  case BVH::Build::TopDown: {
    return Root(a_pool, std::move(primsAndBVs));
  }
  case BVH::Build::Morton: {
    return Root(a_pool, std::move(primsAndBVs), K, SFC::Morton{});
  }
  case BVH::Build::Nested: {
    return Root(a_pool, std::move(primsAndBVs), K, SFC::Nested{});
  }
  case BVH::Build::SAH:
  default: {
    EBGEOMETRY_EXPECT(a_build == BVH::Build::SAH);

    return Root(a_pool, std::move(primsAndBVs), BVH::BinnedSAHPartitioner<T, P, BV, K>);
  }
  }
}

/**
 * @brief Duplicate a BVH union's packed BVH into another pool, deep-copying pool-resident
 * primitives too.
 * @details Internal helper; not part of the public API. PackedBVH::deepCopy() copies primitives
 * byte for byte, which is exact for a self-contained primitive but would leave a pool-resident one
 * pointing at the source pool.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 * @param[in]     a_bvh     Source BVH.
 * @param[in,out] a_dstPool Pool to reserve the copy from.
 * @return The copied BVH.
 */
template <class T, class P, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
inline BVH::PackedBVH<T, P, K>
deepCopyBVH(const BVH::PackedBVH<T, P, K>& a_bvh, Pool& a_dstPool)
{
  using Root = BVH::PackedBVH<T, P, K>;
  using Node = typename Root::Node;

  if constexpr (IsPoolResident<P>::value) {
    // Copy both arrays out before reserving anything: if a_dstPool is also the source pool, a
    // reserve can move its block and invalidate the spans.
    const auto nodeSpan = a_bvh.getNodes();
    const auto primSpan = a_bvh.getPrimitives();

    const std::vector<Node> nodes(nodeSpan.begin(), nodeSpan.end());
    const std::vector<P>    sourcePrims(primSpan.begin(), primSpan.end());

    const PoolLocation location = a_bvh.location();

    std::vector<P> prims;

    prims.reserve(sourcePrims.size());

    for (const P& primitive : sourcePrims) {
      prims.emplace_back(primitive.relocatedTo(location).deepCopy(a_dstPool));
    }

    return Root(a_dstPool, nodes, prims);
  }
  else {
    return a_bvh.deepCopy(a_dstPool);
  }
}

/**
 * @brief Run rebasedView()'s checks on every pool-resident primitive of a BVH union.
 * @details Internal helper; not part of the public API. The rebased primitives are discarded: a
 * pool-resident primitive follows the union's location at evaluation time instead.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 * @param[in] a_bvh  The union's BVH, before rebasing.
 * @param[in] a_pool Pool the union is being rebased onto.
 */
template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
checkPrimitivesRebase([[maybe_unused]] const BVH::PackedBVH<T, P, K>& a_bvh, [[maybe_unused]] const Pool& a_pool)
{
  if constexpr (IsPoolResident<P>::value) {
    const PoolLocation location = a_bvh.location();

    for (const P& primitive : a_bvh.getPrimitives()) {
      [[maybe_unused]] const P view = primitive.relocatedTo(location).rebasedView(a_pool);
    }
  }
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

template <class T, class P, size_t K>
EBGEOMETRY_HOST
BVHUnionIF<T, P, K>
BVHUnion(Pool&                                         a_pool,
         const std::vector<P>&                         a_primitives,
         const std::vector<BoundingVolumes::AABBT<T>>& a_boundingVolumes)
{
  return BVHUnionIF<T, P, K>(a_pool, a_primitives, a_boundingVolumes);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
BVHSmoothUnionIF<T, P, K>
BVHSmoothUnion(Pool&                                         a_pool,
               const std::vector<P>&                         a_primitives,
               const std::vector<BoundingVolumes::AABBT<T>>& a_boundingVolumes,
               const T                                       a_smoothLen)
{
  return BVHSmoothUnionIF<T, P, K>(a_pool, a_primitives, a_boundingVolumes, a_smoothLen);
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

template <class T, class P, size_t K>
EBGEOMETRY_HOST
BVHUnionIF<T, P, K>::BVHUnionIF(Pool&                  a_pool,
                                const std::vector<P>&  a_primitives,
                                const std::vector<BV>& a_boundingVolumes,
                                const BVH::Build       a_build)
  : m_bvh(CSGDetail::buildBVH<T, P, K>(a_pool, a_primitives, a_boundingVolumes, a_build))
{}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
BVHUnionIF<T, P, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T minDist = std::numeric_limits<T>::infinity();

  const auto         primitives = m_bvh.getPrimitives();
  const PoolLocation location   = m_bvh.location();

  const auto evalLeaf = [&primitives, &a_point, &location](T& a_minDist, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T v = CSGDetail::signedDistance<T>(primitives[static_cast<uint32_t>(i)], a_point, location);

      EBGEOMETRY_EXPECT(!std::isnan(v));

      a_minDist = std::min(a_minDist, v);
    }
  };

  // pruneTraverse prunes a node when its squared distance to a_point exceeds this bound, and visits
  // children nearest-first via a SIMD child-box test. A node is kept iff its (unsigned) bounding-volume
  // distance is within max(0, minDist). When the running best is negative (a_point is inside a
  // primitive) the bound collapses to 0, so only nodes whose box contains a_point are descended into.
  const auto pruneDist2 = [](const T& a_minDist) noexcept -> T {
    const T bound = std::max(T(0), a_minDist);

    return bound * bound;
  };

  m_bvh.pruneTraverse(a_point, minDist, evalLeaf, pruneDist2);

  return minDist;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline typename BVHUnionIF<T, P, K>::BV
BVHUnionIF<T, P, K>::computeBoundingVolume() const noexcept
{
  return m_bvh.computeBoundingVolume();
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline const typename BVHUnionIF<T, P, K>::Root&
BVHUnionIF<T, P, K>::getBVH() const noexcept
{
  return m_bvh;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline BVHUnionIF<T, P, K>
BVHUnionIF<T, P, K>::rebasedView(const Pool& a_pool) const noexcept
{
  CSGDetail::checkPrimitivesRebase(m_bvh, a_pool);

  return BVHUnionIF(m_bvh.rebasedView(a_pool));
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline BVHUnionIF<T, P, K>
BVHUnionIF<T, P, K>::deepCopy(Pool& a_dstPool) const
{
  return BVHUnionIF(CSGDetail::deepCopyBVH(m_bvh, a_dstPool));
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline BVHUnionIF<T, P, K>
BVHUnionIF<T, P, K>::relocatedTo(const PoolLocation& a_location) const noexcept
{
  return BVHUnionIF(m_bvh.relocatedTo(a_location));
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline bool
BVHUnionIF<T, P, K>::isAttachedTo(const Pool& a_pool) const noexcept
{
  return m_bvh.isAttachedTo(a_pool);
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST
BVHSmoothUnionIF<T, P, K, Blend>::BVHSmoothUnionIF(Pool&                  a_pool,
                                                   const std::vector<P>&  a_primitives,
                                                   const std::vector<BV>& a_boundingVolumes,
                                                   const T                a_smoothLen,
                                                   const Blend            a_blend,
                                                   const BVH::Build       a_build)
  : m_bvh(CSGDetail::buildBVH<T, P, K>(a_pool, a_primitives, a_boundingVolumes, a_build)),
    m_smoothLen(std::max(a_smoothLen, std::numeric_limits<T>::min())),
    m_blend(a_blend)
{
  EBGEOMETRY_EXPECT(a_smoothLen > T(0));
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST_DEVICE
inline T
BVHSmoothUnionIF<T, P, K, Blend>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // The smooth union blends only the two nearest primitive values, so the traversal state tracks the
  // two smallest values seen so far (a <= b).
  struct Closest
  {
    T a = std::numeric_limits<T>::infinity();
    T b = std::numeric_limits<T>::infinity();
  };

  Closest closest;

  const auto         primitives = m_bvh.getPrimitives();
  const PoolLocation location   = m_bvh.location();

  const auto evalLeaf =
    [&primitives, &a_point, &location](Closest& a_closest, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        const T d = CSGDetail::signedDistance<T>(primitives[static_cast<uint32_t>(i)], a_point, location);

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
  // pruned. The result depends only on the two smallest values, and the b-bound provably retains
  // both -- extra nodes it would admit hold only values larger than b, which cannot change a or b.
  const auto pruneDist2 = [](const Closest& a_closest) noexcept -> T {
    const T bound = std::max(T(0), a_closest.b);

    return bound * bound;
  };

  m_bvh.pruneTraverse(a_point, closest, evalLeaf, pruneDist2);

  return m_blend(closest.a, closest.b, m_smoothLen);
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST_DEVICE
inline typename BVHSmoothUnionIF<T, P, K, Blend>::BV
BVHSmoothUnionIF<T, P, K, Blend>::computeBoundingVolume() const noexcept
{
  return m_bvh.computeBoundingVolume();
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST_DEVICE
inline const typename BVHSmoothUnionIF<T, P, K, Blend>::Root&
BVHSmoothUnionIF<T, P, K, Blend>::getBVH() const noexcept
{
  return m_bvh;
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST
inline BVHSmoothUnionIF<T, P, K, Blend>
BVHSmoothUnionIF<T, P, K, Blend>::rebasedView(const Pool& a_pool) const noexcept
{
  CSGDetail::checkPrimitivesRebase(m_bvh, a_pool);

  return BVHSmoothUnionIF(m_bvh.rebasedView(a_pool), m_smoothLen, m_blend);
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST
inline BVHSmoothUnionIF<T, P, K, Blend>
BVHSmoothUnionIF<T, P, K, Blend>::deepCopy(Pool& a_dstPool) const
{
  return BVHSmoothUnionIF(CSGDetail::deepCopyBVH(m_bvh, a_dstPool), m_smoothLen, m_blend);
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST_DEVICE
inline BVHSmoothUnionIF<T, P, K, Blend>
BVHSmoothUnionIF<T, P, K, Blend>::relocatedTo(const PoolLocation& a_location) const noexcept
{
  return BVHSmoothUnionIF(m_bvh.relocatedTo(a_location), m_smoothLen, m_blend);
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST
inline bool
BVHSmoothUnionIF<T, P, K, Blend>::isAttachedTo(const Pool& a_pool) const noexcept
{
  return m_bvh.isAttachedTo(a_pool);
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

  m_implicitFunction = a_implicitFunction;
  m_period           = a_period;

  // Whole tiles only: rounding after clamping to a fractional bound would otherwise produce a tile
  // past it.
  for (size_t i = 0; i < 3; i++) {
    EBGEOMETRY_EXPECT(a_period[i] > T(0));
    EBGEOMETRY_EXPECT(a_repeatLo[i] >= T(0));
    EBGEOMETRY_EXPECT(a_repeatHi[i] >= T(0));

    m_repeatLo[i] = std::round(a_repeatLo[i]);
    m_repeatHi[i] = std::round(a_repeatHi[i]);
  }
}

template <class T>
T
FiniteRepetitionIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  Vec3T<T> q;

  for (size_t i = 0; i < 3; i++) {
    // std::min/std::max rather than std::clamp: std::clamp is undefined if the bounds cross.
    q[i] = a_point[i] -
           m_period[i] * std::round(std::min(std::max(a_point[i] / m_period[i], -m_repeatLo[i]), m_repeatHi[i]));
  }

  const T ret = m_implicitFunction->value(q);

  EBGEOMETRY_EXPECT(!std::isnan(ret));

  return ret;
}

} // namespace EBGeometry

#endif
