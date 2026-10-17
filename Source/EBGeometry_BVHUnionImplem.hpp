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
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Internal helpers shared by BVHUnion and BVHSmoothUnion
 */
namespace CSGDetail {

/**
 * @brief Whether a bounding box is unbounded in some direction.
 * @details Internal helper; not part of the public API. A box is unbounded where a corner reaches
 * plus or minus Math::Limits<T>::max() (or infinity), or is NaN.
 * @tparam T Floating-point precision.
 * @param[in] a_bv Bounding box.
 * @return True if the box is unbounded in at least one direction.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
inline bool
isUnbounded(const BoundingVolumes::AABBT<T>& a_bv) noexcept
{
  for (size_t dir = 0; dir < 3; dir++) {
    if (!(a_bv.getLowCorner()[dir] > -Math::Limits<T>::max()) ||
        !(a_bv.getHighCorner()[dir] < Math::Limits<T>::max())) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Build the parts of a BVH union using the requested strategy: a BVH over the primitives
 * with bounded boxes, and a single leaf holding those with unbounded ones.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 * @param[in,out] a_pool            Pool to reserve the BVH from.
 * @param[in]     a_primitives      Primitives (must be non-empty).
 * @param[in]     a_boundingVolumes Bounding box of each primitive.
 * @param[in]     a_construction    Preset construction method; every BVH::Construction value is supported.
 * @param[in]     a_options         Leaf-size settings; the chosen method reads only its own field.
 * @return The BVH and the unbounded leaf.
 */
template <class T, class P, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
inline UnionParts<T, P, K>
buildUnionParts(Pool&                                         a_pool,
                const std::vector<P>&                         a_primitives,
                const std::vector<BoundingVolumes::AABBT<T>>& a_boundingVolumes,
                const BVH::Construction                       a_construction,
                const BVH::ConstructionOptions&               a_options)
{
  using BV   = BoundingVolumes::AABBT<T>;
  using S    = Stored<P>;
  using Root = BVH::PackedBVH<T, S, K>;
  using Node = typename Root::Node;

  static_assert(std::is_trivially_copyable_v<S> && !isPoolHandle<S>,
                "BVHUnion: a primitive is stored as a trivially copyable value holding no pool location");

  // Each check guards against a mistake that a Release build would otherwise turn into a silent wrong
  // answer or an out-of-bounds read, and runs once at build time, costing nothing per evaluation.
  EBGEOMETRY_REQUIRE(!a_primitives.empty(), "BVHUnion: a union needs at least one primitive");

  EBGEOMETRY_REQUIRE(a_primitives.size() == a_boundingVolumes.size(),
                     "BVHUnion: need one bounding volume per primitive (%zu primitives, %zu bounding volumes)",
                     a_primitives.size(),
                     a_boundingVolumes.size());

  // A pool-resident primitive is stored as a layout and evaluated against the union's own pool location,
  // so it must have been reserved from the same pool; a primitive from another pool would be read from
  // the wrong memory.
  if constexpr (isPoolHandle<P>) {
    for (size_t i = 0; i < a_primitives.size(); i++) {
      EBGEOMETRY_REQUIRE(a_primitives[i].isAttachedTo(a_pool),
                         "BVHUnion: primitive %zu of %zu lives in another Pool; a pool-resident primitive must "
                         "be built in the union's own Pool",
                         i,
                         a_primitives.size());
    }
  }

  BVH::detail::requireLeafSetting("BVHUnion", a_construction, a_options);

  // A primitive with an unbounded box goes into one leaf that every query scans, so that its box
  // neither makes every ancestor's box unbounded nor reaches the build heuristics.
  std::vector<std::pair<S, BV>> primsAndBVs;
  std::vector<S>                unboundedPrims;
  BV                            unboundedBox;

  primsAndBVs.reserve(a_primitives.size());

  for (size_t i = 0; i < a_primitives.size(); i++) {
    if (isUnbounded(a_boundingVolumes[i])) {
      unboundedPrims.push_back(toStored(a_primitives[i]));

      unboundedBox = unboundedBox.merged(a_boundingVolumes[i]);
    }
    else {
      primsAndBVs.emplace_back(toStored(a_primitives[i]), a_boundingVolumes[i]);
    }
  }

  std::vector<Node> unboundedNodes;

  if (!unboundedPrims.empty()) {
    Node leaf;

    leaf.setBoundingVolume(unboundedBox);
    leaf.setPrimitivesOffset(0U);
    leaf.setNumPrimitives(static_cast<uint32_t>(unboundedPrims.size()));

    unboundedNodes.push_back(leaf);
  }

  Root unbounded(a_pool, unboundedNodes, unboundedPrims);

  if (primsAndBVs.empty()) {
    return UnionParts<T, P, K>{Root(a_pool, std::vector<Node>{}, std::vector<S>{}), unbounded};
  }

  return UnionParts<T, P, K>{BVH::PackedBVH<T, S, K>::build(a_pool, std::move(primsAndBVs), a_construction, a_options),
                             unbounded};
}

/**
 * @brief Duplicate a BVH union's packed BVH into another pool, deep-copying pool-resident
 * primitives too.
 * @details Internal helper; not part of the public API. PackedBVH::deepCopy() copies primitives
 * byte for byte, which is exact for a self-contained primitive but would leave a pool-resident one's
 * layout pointing at arrays in the source pool.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 * @param[in]     a_bvh     Source BVH.
 * @param[in,out] a_dstPool Pool to reserve the copy from.
 * @return The copied BVH.
 */
template <class T, class P, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
inline BVH::PackedBVH<T, Stored<P>, K>
deepCopyBVH(const BVH::PackedBVH<T, Stored<P>, K>& a_bvh, Pool& a_dstPool)
{
  using Root = BVH::PackedBVH<T, Stored<P>, K>;
  using Node = typename Root::Node;

  if constexpr (isPoolHandle<P>) {
    // Copy both arrays out before reserving anything: if a_dstPool is also the source pool, a
    // reserve can move its block and invalidate the spans.
    const auto nodeSpan = a_bvh.getNodes();
    const auto primSpan = a_bvh.getPrimitives();

    const std::vector<Node>      nodes(nodeSpan.begin(), nodeSpan.end());
    const std::vector<Stored<P>> sourcePrims(primSpan.begin(), primSpan.end());

    const PoolLocation location = a_bvh.location();

    std::vector<Stored<P>> prims;

    prims.reserve(sourcePrims.size());

    for (const Stored<P>& primitive : sourcePrims) {
      prims.emplace_back(fromStored<P>(primitive, location).deepCopy(a_dstPool).layout());
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
 * pool-resident primitive is stored as a layout and resolves against the union's location instead.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 * @param[in] a_bvh  The union's BVH, before rebasing.
 * @param[in] a_pool Pool the union is being rebased onto.
 */
template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline void
checkPrimitivesRebase([[maybe_unused]] const BVH::PackedBVH<T, Stored<P>, K>& a_bvh,
                      [[maybe_unused]] const Pool&                            a_pool)
{
  if constexpr (isPoolHandle<P>) {
    const PoolLocation location = a_bvh.location();

    for (const Stored<P>& primitive : a_bvh.getPrimitives()) {
      [[maybe_unused]] const P view = fromStored<P>(primitive, location).rebasedView(a_pool);
    }
  }
}

} // namespace CSGDetail

template <class T, class P, size_t K>
EBGEOMETRY_HOST
BVHUnion<T, P, K>::BVHUnion(Pool&                   a_pool,
                            const std::vector<P>&   a_primitives,
                            const std::vector<BV>&  a_boundingVolumes,
                            const BVH::Construction a_construction)
  : BVHUnion(a_pool, a_primitives, a_boundingVolumes, a_construction, BVHUnion::defaultConstructionOptions())
{}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
BVHUnion<T, P, K>::BVHUnion(Pool&                           a_pool,
                            const std::vector<P>&           a_primitives,
                            const std::vector<BV>&          a_boundingVolumes,
                            const BVH::Construction         a_construction,
                            const BVH::ConstructionOptions& a_options)
  : BVHUnion(CSGDetail::buildUnionParts<T, P, K>(a_pool, a_primitives, a_boundingVolumes, a_construction, a_options))
{}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
BVHUnion<T, P, K>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  T minDist = Math::Limits<T>::infinity();

  // The unbounded primitives first, so that their values can prune the traversal.
  const PoolLocation location = this->location();
  const Root         bvh      = this->getBVH();
  const Root         unboundedRoot(m_unbounded, location);
  const auto         unbounded = unboundedRoot.getPrimitives();

  for (uint32_t i = 0; i < unbounded.size(); i++) {
    minDist = Math::min(minDist, CSGDetail::fromStored<P>(unbounded[i], location).signedDistance(a_point));
  }

  const auto primitives = bvh.getPrimitives();

  const auto evalLeaf = [&primitives, &a_point, &location](T& a_minDist, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const T v = CSGDetail::fromStored<P>(primitives[static_cast<uint32_t>(i)], location).signedDistance(a_point);

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

  bvh.pruneTraverse(a_point, minDist, evalLeaf, pruneDist2);

  return minDist;
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline typename BVHUnion<T, P, K>::BV
BVHUnion<T, P, K>::computeBoundingVolume() const noexcept
{
  return this->getBVH().computeBoundingVolume().merged(Root(m_unbounded, this->location()).computeBoundingVolume());
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline typename BVHUnion<T, P, K>::Root
BVHUnion<T, P, K>::getBVH() const noexcept
{
  return Root(m_bvh, this->location());
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<const CSGDetail::Stored<P>>
BVHUnion<T, P, K>::getUnboundedPrimitives() const noexcept
{
  const Root unbounded(m_unbounded, this->location());

  return unbounded.getPrimitives();
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline BVHUnion<T, P, K>
BVHUnion<T, P, K>::rebasedView(const Pool& a_pool) const noexcept
{
  const Root bvh       = this->getBVH();
  const Root unbounded = Root(m_unbounded, this->location());

  CSGDetail::checkPrimitivesRebase<T, P, K>(bvh, a_pool);
  CSGDetail::checkPrimitivesRebase<T, P, K>(unbounded, a_pool);

  // Both parts live in one pool, so they rebase to one location. Rebasing each checks its arrays,
  // and the BVH its depth.
  const PoolLocation location = bvh.rebasedView(a_pool).location();

  [[maybe_unused]] const Root unboundedView = unbounded.rebasedView(a_pool);

  return BVHUnion(Layout{m_bvh, m_unbounded}, location);
}

template <class T, class P, size_t K>
EBGEOMETRY_HOST
inline BVHUnion<T, P, K>
BVHUnion<T, P, K>::deepCopy(Pool& a_dstPool) const
{
  // Both parts go into a_dstPool, so the copy resolves them against one pool, as the original does.
  const Root bvh       = CSGDetail::deepCopyBVH<T, P, K>(this->getBVH(), a_dstPool);
  const Root unbounded = CSGDetail::deepCopyBVH<T, P, K>(Root(m_unbounded, this->location()), a_dstPool);

  return BVHUnion(Layout{bvh.layout(), unbounded.layout()}, bvh.location());
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST
BVHSmoothUnion<T, P, K, Blend>::BVHSmoothUnion(Pool&                   a_pool,
                                               const std::vector<P>&   a_primitives,
                                               const std::vector<BV>&  a_boundingVolumes,
                                               const T                 a_smoothLen,
                                               const Blend             a_blend,
                                               const BVH::Construction a_construction)
  : BVHSmoothUnion(a_pool,
                   a_primitives,
                   a_boundingVolumes,
                   a_smoothLen,
                   a_blend,
                   a_construction,
                   BVHSmoothUnion::defaultConstructionOptions())
{}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST
BVHSmoothUnion<T, P, K, Blend>::BVHSmoothUnion(Pool&                           a_pool,
                                               const std::vector<P>&           a_primitives,
                                               const std::vector<BV>&          a_boundingVolumes,
                                               const T                         a_smoothLen,
                                               const Blend                     a_blend,
                                               const BVH::Construction         a_construction,
                                               const BVH::ConstructionOptions& a_options)
  : BVHSmoothUnion(
      CSGDetail::buildUnionParts<T, P, K>(a_pool, a_primitives, a_boundingVolumes, a_construction, a_options),
      a_smoothLen,
      a_blend)
{
  EBGEOMETRY_REQUIRE(
    a_smoothLen > T(0), "BVHSmoothUnion: the smoothing length must be positive (%g)", double(a_smoothLen));
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST_DEVICE
inline T
BVHSmoothUnion<T, P, K, Blend>::signedDistance(const Vec3T<T>& a_point) const noexcept
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

  const auto insert = [](Closest& a_closest, const T a_value) noexcept {
    EBGEOMETRY_EXPECT(!std::isnan(a_value));

    if (a_value < a_closest.a) {
      a_closest.b = a_closest.a;
      a_closest.a = a_value;
    }
    else if (a_value < a_closest.b) {
      a_closest.b = a_value;
    }
  };

  // The unbounded primitives first, so that their values can prune the traversal.
  const PoolLocation location = this->location();
  const Root         bvh      = this->getBVH();
  const Root         unboundedRoot(m_unbounded, location);
  const auto         unbounded = unboundedRoot.getPrimitives();

  for (uint32_t i = 0; i < unbounded.size(); i++) {
    insert(closest, CSGDetail::fromStored<P>(unbounded[i], location).signedDistance(a_point));
  }

  const auto primitives = bvh.getPrimitives();

  const auto evalLeaf =
    [&primitives, &a_point, &location, &insert](Closest& a_closest, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        insert(a_closest,
               CSGDetail::fromStored<P>(primitives[static_cast<uint32_t>(i)], location).signedDistance(a_point));
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

  bvh.pruneTraverse(a_point, closest, evalLeaf, pruneDist2);

  return m_blend(closest.a, closest.b, m_smoothLen);
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST_DEVICE
inline typename BVHSmoothUnion<T, P, K, Blend>::BV
BVHSmoothUnion<T, P, K, Blend>::computeBoundingVolume() const noexcept
{
  return this->getBVH().computeBoundingVolume().merged(Root(m_unbounded, this->location()).computeBoundingVolume());
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST_DEVICE
inline typename BVHSmoothUnion<T, P, K, Blend>::Root
BVHSmoothUnion<T, P, K, Blend>::getBVH() const noexcept
{
  return Root(m_bvh, this->location());
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<const CSGDetail::Stored<P>>
BVHSmoothUnion<T, P, K, Blend>::getUnboundedPrimitives() const noexcept
{
  const Root unbounded(m_unbounded, this->location());

  return unbounded.getPrimitives();
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST
inline BVHSmoothUnion<T, P, K, Blend>
BVHSmoothUnion<T, P, K, Blend>::rebasedView(const Pool& a_pool) const noexcept
{
  const Root bvh       = this->getBVH();
  const Root unbounded = Root(m_unbounded, this->location());

  CSGDetail::checkPrimitivesRebase<T, P, K>(bvh, a_pool);
  CSGDetail::checkPrimitivesRebase<T, P, K>(unbounded, a_pool);

  // Both parts live in one pool, so they rebase to one location. Rebasing each checks its arrays,
  // and the BVH its depth.
  const PoolLocation location = bvh.rebasedView(a_pool).location();

  [[maybe_unused]] const Root unboundedView = unbounded.rebasedView(a_pool);

  return BVHSmoothUnion(Layout{m_bvh, m_unbounded, m_smoothLen, m_blend}, location);
}

template <class T, class P, size_t K, class Blend>
EBGEOMETRY_HOST
inline BVHSmoothUnion<T, P, K, Blend>
BVHSmoothUnion<T, P, K, Blend>::deepCopy(Pool& a_dstPool) const
{
  // Both parts go into a_dstPool, so the copy resolves them against one pool, as the original does.
  const Root bvh       = CSGDetail::deepCopyBVH<T, P, K>(this->getBVH(), a_dstPool);
  const Root unbounded = CSGDetail::deepCopyBVH<T, P, K>(Root(m_unbounded, this->location()), a_dstPool);

  return BVHSmoothUnion(Layout{bvh.layout(), unbounded.layout(), m_smoothLen, m_blend}, bvh.location());
}

} // namespace EBGeometry

#endif
