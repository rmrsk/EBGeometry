// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHUnionImplem.hpp
 * @brief  Implementation of EBGeometry_BVHUnion.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHUNIONIMPLEM_HPP
#define EBGEOMETRY_BVHUNIONIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_BVHUnion.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_SFC.hpp"
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
 * @param[in]     a_construction           Preset construction method; every BVH::Construction value is supported.
 * @return The packed BVH.
 */
template <class T, class P, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
inline BVH::PackedBVH<T, P, K>
buildBVH(Pool&                                         a_pool,
         const std::vector<P>&                         a_primitives,
         const std::vector<BoundingVolumes::AABBT<T>>& a_boundingVolumes,
         const BVH::Construction                       a_construction)
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

  switch (a_construction) {
  case BVH::Construction::CentroidSplit: {
    return Root(a_pool, std::move(primsAndBVs), BVH::BVCentroidPartitioner<T, P, BV, K>);
  }
  case BVH::Construction::MidpointSplit: {
    return Root(a_pool, std::move(primsAndBVs), BVH::MidpointPartitioner<T, P, BV, K>);
  }
  case BVH::Construction::ClusterSAH: {
    return Root(a_pool, std::move(primsAndBVs), BVH::ClusterSpec{});
  }
  case BVH::Construction::Morton: {
    return Root(a_pool, std::move(primsAndBVs), K, SFC::Morton{});
  }
  case BVH::Construction::Nested: {
    return Root(a_pool, std::move(primsAndBVs), K, SFC::Nested{});
  }
  case BVH::Construction::Hilbert: {
    return Root(a_pool, std::move(primsAndBVs), K, SFC::Hilbert{});
  }
  case BVH::Construction::SAH:
  default: {
    EBGEOMETRY_REQUIRE(a_construction == BVH::Construction::SAH,
                       "BVHUnionIF: unknown BVH::Construction value (%d)",
                       static_cast<int>(a_construction));

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

template <class T, class P, size_t K>
EBGEOMETRY_HOST
BVHUnionIF<T, P, K>::BVHUnionIF(Pool&                   a_pool,
                                const std::vector<P>&   a_primitives,
                                const std::vector<BV>&  a_boundingVolumes,
                                const BVH::Construction a_construction)
  : m_bvh(CSGDetail::buildBVH<T, P, K>(a_pool, a_primitives, a_boundingVolumes, a_construction))
{}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
BVHUnionIF<T, P, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T minDist = Math::Limits<T>::infinity();

  const auto         primitives = m_bvh.getPrimitives();
  const PoolLocation location   = m_bvh.location();

  const auto evalLeaf = [&primitives, &a_point, &location](T& a_minDist, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T v = CSGDetail::signedDistance<T>(primitives[static_cast<uint32_t>(i)], a_point, location);

      EBGEOMETRY_EXPECT(!std::isnan(v));

      a_minDist = Math::min(a_minDist, v);
    }
  };

  // pruneTraverse prunes a node when its squared distance to a_point exceeds this bound, and visits
  // children nearest-first via a SIMD child-box test. A node is kept iff its (unsigned) bounding-volume
  // distance is within max(0, minDist). When the running best is negative (a_point is inside a
  // primitive) the bound collapses to 0, so only nodes whose box contains a_point are descended into.
  const auto pruneDist2 = [](const T& a_minDist) noexcept -> T {
    const T bound = Math::max(T(0), a_minDist);

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
BVHSmoothUnionIF<T, P, K, Blend>::BVHSmoothUnionIF(Pool&                   a_pool,
                                                   const std::vector<P>&   a_primitives,
                                                   const std::vector<BV>&  a_boundingVolumes,
                                                   const T                 a_smoothLen,
                                                   const Blend             a_blend,
                                                   const BVH::Construction a_construction)
  : m_bvh(CSGDetail::buildBVH<T, P, K>(a_pool, a_primitives, a_boundingVolumes, a_construction)),
    m_smoothLen(Math::max(a_smoothLen, Math::Limits<T>::min())),
    m_blend(a_blend)
{
  EBGEOMETRY_REQUIRE(
    a_smoothLen > T(0), "BVHSmoothUnionIF: the smoothing length must be positive (%g)", double(a_smoothLen));
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
    T a = Math::Limits<T>::infinity();
    T b = Math::Limits<T>::infinity();
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
    const T bound = Math::max(T(0), a_closest.b);

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

} // namespace EBGeometry

#endif
