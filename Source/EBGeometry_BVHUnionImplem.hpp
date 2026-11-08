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
#include <tuple>
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
  std::vector<BV>   unboundedNodeBoxes;

  if (!unboundedPrims.empty()) {
    Node leaf;

    leaf.setPrimitivesOffset(0U);
    leaf.setNumPrimitives(static_cast<uint32_t>(unboundedPrims.size()));

    unboundedNodes.push_back(leaf);
    unboundedNodeBoxes.push_back(unboundedBox);
  }

  Root unbounded(a_pool, unboundedNodes, unboundedNodeBoxes, unboundedPrims);

  if (primsAndBVs.empty()) {
    return UnionParts<T, P, K>{Root(a_pool, std::vector<Node>{}, std::vector<BV>{}, std::vector<S>{}), unbounded};
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

    std::vector<typename Root::BV> boxes;

    boxes.reserve(nodes.size());

    for (const Node& node : nodes) {
      boxes.push_back(a_bvh.getBoundingVolume(node));
    }

    const PoolLocation location = a_bvh.location();

    std::vector<Stored<P>> prims;

    prims.reserve(sourcePrims.size());

    for (const Stored<P>& primitive : sourcePrims) {
      prims.emplace_back(fromStored<P>(primitive, location).deepCopy(a_dstPool).layout());
    }

    return Root(a_dstPool, nodes, boxes, prims);
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

/**
 * @brief The layout of a union of one type from its built parts.
 * @details Internal helper; not part of the public API.
 * @tparam Layout The union's layout type.
 * @param[in] a_parts The BVH and the unbounded leaf.
 * @return The layout (its BVH and unbounded leaf set) and where it lives.
 */
template <class Layout, class T, class P, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
inline std::pair<Layout, PoolLocation>
fromParts(const UnionParts<T, P, K>& a_parts)
{
  Layout layout{};

  layout.m_bvh       = a_parts.bvh.layout();
  layout.m_unbounded = a_parts.unbounded.layout();

  return {layout, a_parts.bvh.location()};
}

/**
 * @brief Each primitive's own bounding box, its computeBoundingVolume().
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @param[in] a_primitives Primitives.
 * @return One box per primitive.
 */
template <class T, class P>
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<BoundingVolumes::AABBT<T>>
boxesOf(const std::vector<P>& a_primitives)
{
  std::vector<BoundingVolumes::AABBT<T>> boxes;

  boxes.reserve(a_primitives.size());

  for (const P& primitive : a_primitives) {
    boxes.push_back(primitive.computeBoundingVolume());
  }

  return boxes;
}

/**
 * @brief Abort if a pool-resident primitive lives in another pool than the union's.
 * @details Internal helper; not part of the public API. A pool-resident primitive is stored as a layout and
 * evaluated against the union's own pool location, so one from another pool would be read from the wrong
 * memory.
 * @tparam P Primitive type.
 * @param[in] a_pool       The union's pool.
 * @param[in] a_primitives Primitives of one type.
 */
template <class P>
EBGEOMETRY_HOST
inline void
requireSamePool([[maybe_unused]] const Pool& a_pool, [[maybe_unused]] const std::vector<P>& a_primitives)
{
  if constexpr (isPoolHandle<P>) {
    for (size_t i = 0; i < a_primitives.size(); i++) {
      EBGEOMETRY_REQUIRE(a_primitives[i].isAttachedTo(a_pool),
                         "BVHUnion: primitive %zu of %zu lives in another Pool; a pool-resident primitive must "
                         "be built in the union's own Pool",
                         i,
                         a_primitives.size());
    }
  }
}

/**
 * @brief The primitives of each type, as stored, in one host vector per type.
 * @details Internal helper; not part of the public API.
 * @tparam P Primitive type.
 * @param[in] a_primitives Primitives.
 * @return Their stored forms.
 */
template <class P>
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<Stored<P>>
storedOf(const std::vector<P>& a_primitives)
{
  std::vector<Stored<P>> stored;

  stored.reserve(a_primitives.size());

  for (const P& primitive : a_primitives) {
    stored.push_back(toStored(primitive));
  }

  return stored;
}

/**
 * @brief Reserve each type's array from a pool.
 * @details Internal helper; not part of the public API. An empty type reserves nothing.
 * @param[in,out] a_pool   Pool.
 * @param[out]    a_arrays The arrays.
 * @param[in]     a_stored One vector per type, in the arrays' order.
 */
template <class P, class... Ps, class V, class... Vs>
EBGEOMETRY_HOST
inline void
reserveArrays(Pool& a_pool, UnionArrays<P, Ps...>& a_arrays, const V& a_stored, const Vs&... a_rest)
{
  if (!a_stored.empty()) {
    a_arrays.m_head.reserveFrom(a_pool, static_cast<uint32_t>(a_stored.size()));
  }

  if constexpr (sizeof...(Ps) > 0) {
    reserveArrays(a_pool, a_arrays.m_tail, a_rest...);
  }
}

/**
 * @brief Copy each type's stored primitives into its reserved array.
 * @details Internal helper; not part of the public API. Called after every reservation of the build, since a
 * reservation can move the pool's block.
 * @param[in]  a_base   The pool's base.
 * @param[out] a_arrays The arrays.
 * @param[in]  a_stored One vector per type, in the arrays' order.
 */
template <class P, class... Ps, class V, class... Vs>
EBGEOMETRY_HOST
inline void
fillArrays(void* a_base, UnionArrays<P, Ps...>& a_arrays, const V& a_stored, const Vs&... a_rest)
{
  if (!a_stored.empty()) {
    a_arrays.m_head.assign(a_base, a_stored.data(), static_cast<uint32_t>(a_stored.size()));
  }

  if constexpr (sizeof...(Ps) > 0) {
    fillArrays(a_base, a_arrays.m_tail, a_rest...);
  }
}

/**
 * @brief Build a union of several primitive types: one array per type, and a BVH (and an unbounded leaf) over
 * references into them.
 * @details Internal helper; not part of the public API. Each primitive's box is its computeBoundingVolume().
 * @tparam T      Floating-point precision.
 * @tparam K      BVH branching factor.
 * @tparam Layout The union's layout type: its BVH, its unbounded leaf and its arrays are set.
 * @tparam Ps     Primitive types.
 * @param[in,out] a_pool         Pool the union is reserved from.
 * @param[in]     a_construction Construction method.
 * @param[in]     a_options      Leaf sizes.
 * @param[in]     a_primitives   One array per type.
 * @return The layout and where it lives.
 */
template <class T, size_t K, class Layout, class... Ps>
[[nodiscard]] EBGEOMETRY_HOST
inline std::pair<Layout, PoolLocation>
buildMixedUnion(Pool&                           a_pool,
                const BVH::Construction         a_construction,
                const BVH::ConstructionOptions& a_options,
                const std::vector<Ps>&... a_primitives)
{
  using BV   = BoundingVolumes::AABBT<T>;
  using Root = BVH::PackedBVH<T, MemberRef, K>;
  using Node = typename Root::Node;

  const size_t total = (a_primitives.size() + ...);

  EBGEOMETRY_REQUIRE(total > 0, "BVHUnion: a union needs at least one primitive");

  (requireSamePool(a_pool, a_primitives), ...);

  BVH::detail::requireLeafSetting("BVHUnion", a_construction, a_options);

  // A primitive with an unbounded box goes into one leaf that every query scans, as for one type.
  std::vector<std::pair<MemberRef, BV>> leavesAndBVs;
  std::vector<MemberRef>                unboundedLeaves;
  BV                                    unboundedBox;

  leavesAndBVs.reserve(total);

  uint32_t type = 0;

  const auto collect = [&](const auto& a_ofType) {
    for (size_t i = 0; i < a_ofType.size(); i++) {
      const BV        box = a_ofType[i].computeBoundingVolume();
      const MemberRef leaf{type, static_cast<uint32_t>(i)};

      if (isUnbounded(box)) {
        unboundedLeaves.push_back(leaf);

        unboundedBox = unboundedBox.merged(box);
      }
      else {
        leavesAndBVs.emplace_back(leaf, box);
      }
    }

    type++;
  };

  (collect(a_primitives), ...);

  Layout layout{};

  // The arrays are reserved first and filled last, after every reservation of the BVHs.
  reserveArrays(a_pool, layout.m_arrays, storedOf(a_primitives)...);

  std::vector<Node> unboundedNodes;
  std::vector<BV>   unboundedNodeBoxes;

  if (!unboundedLeaves.empty()) {
    Node leaf;

    leaf.setPrimitivesOffset(0U);
    leaf.setNumPrimitives(static_cast<uint32_t>(unboundedLeaves.size()));

    unboundedNodes.push_back(leaf);
    unboundedNodeBoxes.push_back(unboundedBox);
  }

  const Root unbounded(a_pool, unboundedNodes, unboundedNodeBoxes, unboundedLeaves);
  const Root bvh = leavesAndBVs.empty() ? Root(a_pool, std::vector<Node>{}, std::vector<BV>{}, std::vector<MemberRef>{})
                                        : Root::build(a_pool, std::move(leavesAndBVs), a_construction, a_options);

  fillArrays(a_pool.base(), layout.m_arrays, storedOf(a_primitives)...);

  layout.m_bvh       = bvh.layout();
  layout.m_unbounded = unbounded.layout();

  return {layout, bvh.location()};
}

/**
 * @brief Build a union of one or more primitive types, each primitive's box its computeBoundingVolume().
 * @details Internal helper; not part of the public API. One type is built by buildUnionParts(), its leaves
 * holding the primitives; several by buildMixedUnion().
 * @tparam T      Floating-point precision.
 * @tparam K      BVH branching factor.
 * @tparam Layout The union's layout type.
 * @tparam Ps     Primitive types.
 * @param[in,out] a_pool       Pool the union is reserved from.
 * @param[in]     a_settings   Construction method and leaf sizes.
 * @param[in]     a_defaults   The leaf sizes to use when a_settings has none.
 * @param[in]     a_primitives One array per type.
 * @return The layout and where it lives.
 */
template <class T, size_t K, class Layout, class... Ps>
[[nodiscard]] EBGEOMETRY_HOST
inline std::pair<Layout, PoolLocation>
buildUnion(Pool&                           a_pool,
           const BVH::BuildSettings&       a_settings,
           const BVH::ConstructionOptions& a_defaults,
           const std::vector<Ps>&... a_primitives)
{
  const BVH::ConstructionOptions options = a_settings.m_options.value_or(a_defaults);

  if constexpr (sizeof...(Ps) == 1) {
    using P = std::tuple_element_t<0, std::tuple<Ps...>>;

    const std::vector<P>& primitives = std::get<0>(std::forward_as_tuple(a_primitives...));

    return fromParts<Layout>(
      buildUnionParts<T, P, K>(a_pool, primitives, boxesOf<T>(primitives), a_settings.m_construction, options));
  }
  else {
    return buildMixedUnion<T, K, Layout>(a_pool, a_settings.m_construction, options, a_primitives...);
  }
}

/**
 * @brief Duplicate each type's array, and any pool-resident primitive in it, into another pool.
 * @details Internal helper; not part of the public API. Each array is read out before anything is reserved,
 * since the destination may be the source pool, whose block a reservation can move.
 * @param[in]     a_src      The arrays.
 * @param[in]     a_location Where they live.
 * @param[in,out] a_dstPool  Pool to copy into.
 * @param[out]    a_dst      The copies.
 */
template <class P, class... Ps>
EBGEOMETRY_HOST
inline void
deepCopyArrays(const UnionArrays<P, Ps...>& a_src,
               const PoolLocation&          a_location,
               Pool&                        a_dstPool,
               UnionArrays<P, Ps...>&       a_dst)
{
  const uint32_t count = a_src.m_head.size();

  if (count > 0) {
    const Stored<P>*       first = a_src.m_head.data(a_location.base());
    std::vector<Stored<P>> items(first, first + count);

    if constexpr (isPoolHandle<P>) {
      for (Stored<P>& item : items) {
        item = fromStored<P>(item, a_location).deepCopy(a_dstPool).layout();
      }
    }

    a_dst.m_head.reserveFrom(a_dstPool, count);
    a_dst.m_head.assign(a_dstPool.base(), items.data(), count);
  }

  if constexpr (sizeof...(Ps) > 0) {
    deepCopyArrays(a_src.m_tail, a_location, a_dstPool, a_dst.m_tail);
  }
}

/**
 * @brief Run the rebase checks for each type's array: every pool-resident primitive in it is rebased once, and
 * the result discarded (see BVHUnion::rebasedView()).
 * @details Internal helper; not part of the public API.
 * @param[in] a_arrays   The arrays.
 * @param[in] a_location Where they live.
 * @param[in] a_pool     Pool to rebase onto.
 * @return The first byte past the last array.
 */
template <class P, class... Ps>
[[nodiscard]] EBGEOMETRY_HOST
inline uint64_t
checkArraysRebase(const UnionArrays<P, Ps...>&         a_arrays,
                  [[maybe_unused]] const PoolLocation& a_location,
                  [[maybe_unused]] const Pool&         a_pool)
{
  if constexpr (isPoolHandle<P>) {
    for (uint32_t i = 0; i < a_arrays.m_head.size(); i++) {
      [[maybe_unused]] const P view =
        fromStored<P>(a_arrays.m_head.at(a_location.base(), i), a_location).rebasedView(a_pool);
    }
  }

  uint64_t endByte = a_arrays.m_head.endByte();

  if constexpr (sizeof...(Ps) > 0) {
    endByte = Math::max(endByte, checkArraysRebase(a_arrays.m_tail, a_location, a_pool));
  }

  return endByte;
}

} // namespace CSGDetail

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST
BVHUnion<T, K, Ps...>::BVHUnion(Pool& a_pool, const std::vector<Ps>&... a_primitives)
  : BVHUnion(a_pool, BVH::BuildSettings{}, a_primitives...)
{}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST
BVHUnion<T, K, Ps...>::BVHUnion(Pool&                     a_pool,
                                const BVH::BuildSettings& a_settings,
                                const std::vector<Ps>&... a_primitives)
  : BVHUnion(CSGDetail::buildUnion<T, K, Layout>(a_pool, a_settings, defaultConstructionOptions(), a_primitives...))
{}

template <class T, size_t K, class... Ps>
template <size_t N, std::enable_if_t<N == 1, int>>
EBGEOMETRY_HOST
BVHUnion<T, K, Ps...>::BVHUnion(Pool&                     a_pool,
                                const std::vector<First>& a_primitives,
                                const std::vector<BV>&    a_boundingVolumes,
                                const BVH::Construction   a_construction)
  : BVHUnion(a_pool, a_primitives, a_boundingVolumes, a_construction, BVHUnion::defaultConstructionOptions())
{}

template <class T, size_t K, class... Ps>
template <size_t N, std::enable_if_t<N == 1, int>>
EBGEOMETRY_HOST
BVHUnion<T, K, Ps...>::BVHUnion(Pool&                           a_pool,
                                const std::vector<First>&       a_primitives,
                                const std::vector<BV>&          a_boundingVolumes,
                                const BVH::Construction         a_construction,
                                const BVH::ConstructionOptions& a_options)
  : BVHUnion(CSGDetail::fromParts<Layout>(
      CSGDetail::buildUnionParts<T, First, K>(a_pool, a_primitives, a_boundingVolumes, a_construction, a_options)))
{}

template <class T, size_t K, class... Ps>
template <class F>
EBGEOMETRY_HOST_DEVICE
inline auto
BVHUnion<T, K, Ps...>::visit(const Leaf& a_leaf, F&& a_f) const noexcept
{
  return this->visitAt(a_leaf, this->location(), static_cast<F&&>(a_f));
}

template <class T, size_t K, class... Ps>
template <class F>
EBGEOMETRY_HOST_DEVICE
inline auto
BVHUnion<T, K, Ps...>::visitAt(const Leaf& a_leaf, const PoolLocation& a_location, F&& a_f) const noexcept
{
  if constexpr (sizeof...(Ps) == 1) {
    return a_f(CSGDetail::fromStored<First>(a_leaf, a_location));
  }
  else {
    return CSGDetail::visitArrays(this->m_arrays, a_leaf.m_type, a_leaf.m_index, a_location, static_cast<F&&>(a_f));
  }
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline T
BVHUnion<T, K, Ps...>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  // One type: the leaf is the primitive, evaluated directly, with no type test (this code is kept apart so
  // that a union of one type compiles to the code it needs and nothing more). Several: each leaf resolved
  // through visitAt().
  if constexpr (sizeof...(Ps) == 1) {
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
      minDist = Math::min(minDist, CSGDetail::fromStored<First>(unbounded[i], location).signedDistance(a_point));
    }

    const auto primitives = bvh.getPrimitives();

    const auto evalLeaf = [&primitives, &a_point, &location](T& a_minDist, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        const T v =
          CSGDetail::fromStored<First>(primitives[static_cast<uint32_t>(i)], location).signedDistance(a_point);

        EBGEOMETRY_EXPECT(!std::isnan(v));

        a_minDist = Math::min(a_minDist, v);
      }
    };

    // traverse() prunes a node when its squared distance to a_point exceeds this bound, and visits
    // children nearest-first via a SIMD child-box test. A node is kept iff its (unsigned) bounding-volume
    // distance is within max(0, minDist). When the running best is negative (a_point is inside a
    // primitive) the bound collapses to 0, so only nodes whose box contains a_point are descended into.
    const auto pruneDist2 = [](const T& a_minDist) noexcept -> T {
      const T bound = Math::max(T(0), a_minDist);

      return bound * bound;
    };

    bvh.traverse(BVH::PointQuery<T>{a_point}, minDist, evalLeaf, pruneDist2);

    return minDist;
  }
  else {
    EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

    T minDist = Math::Limits<T>::infinity();

    const auto value = [&a_point](const auto& a_primitive) noexcept -> T {
      return a_primitive.signedDistance(a_point);
    };

    // The unbounded primitives first, so that their values can prune the traversal.
    const PoolLocation location = this->location();
    const Root         bvh      = this->getBVH();
    const Root         unboundedRoot(m_unbounded, location);
    const auto         unbounded = unboundedRoot.getPrimitives();

    for (uint32_t i = 0; i < unbounded.size(); i++) {
      minDist = Math::min(minDist, this->visitAt(unbounded[i], location, value));
    }

    const auto primitives = bvh.getPrimitives();

    const auto evalLeaf =
      [this, &primitives, &location, &value](T& a_minDist, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = a_offset; i < a_offset + a_count; i++) {
          const T v = this->visitAt(primitives[static_cast<uint32_t>(i)], location, value);

          EBGEOMETRY_EXPECT(!std::isnan(v));

          a_minDist = Math::min(a_minDist, v);
        }
      };

    const auto pruneDist2 = [](const T& a_minDist) noexcept -> T {
      const T bound = Math::max(T(0), a_minDist);

      return bound * bound;
    };

    bvh.traverse(BVH::PointQuery<T>{a_point}, minDist, evalLeaf, pruneDist2);

    return minDist;
  }
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline typename BVHUnion<T, K, Ps...>::BV
BVHUnion<T, K, Ps...>::computeBoundingVolume() const noexcept
{
  return this->getBVH().computeBoundingVolume().merged(Root(m_unbounded, this->location()).computeBoundingVolume());
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline typename BVHUnion<T, K, Ps...>::Root
BVHUnion<T, K, Ps...>::getBVH() const noexcept
{
  return Root(m_bvh, this->location());
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<const typename BVHUnion<T, K, Ps...>::Leaf>
BVHUnion<T, K, Ps...>::getUnboundedPrimitives() const noexcept
{
  const Root unbounded(m_unbounded, this->location());

  return unbounded.getPrimitives();
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST
inline BVHUnion<T, K, Ps...>
BVHUnion<T, K, Ps...>::rebasedView(const Pool& a_pool) const noexcept
{
  const Root bvh       = this->getBVH();
  const Root unbounded = Root(m_unbounded, this->location());

  if constexpr (sizeof...(Ps) == 1) {
    CSGDetail::checkPrimitivesRebase<T, First, K>(bvh, a_pool);
    CSGDetail::checkPrimitivesRebase<T, First, K>(unbounded, a_pool);
  }
  else {
    const uint64_t endByte = CSGDetail::checkArraysRebase(this->m_arrays, this->location(), a_pool);

    [[maybe_unused]] const PoolLocation arrays = this->rebasedLocation(a_pool, endByte, "BVHUnion");
  }

  // All parts live in one pool, so they rebase to one location. Rebasing each checks its arrays,
  // and the BVH its depth.
  const PoolLocation location = bvh.rebasedView(a_pool).location();

  [[maybe_unused]] const Root unboundedView = unbounded.rebasedView(a_pool);

  return BVHUnion(this->layout(), location);
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST
inline BVHUnion<T, K, Ps...>
BVHUnion<T, K, Ps...>::deepCopy(Pool& a_dstPool) const
{
  // All parts go into a_dstPool, so the copy resolves them against one pool, as the original does.
  Layout layout = this->layout();

  if constexpr (sizeof...(Ps) == 1) {
    const Root bvh       = CSGDetail::deepCopyBVH<T, First, K>(this->getBVH(), a_dstPool);
    const Root unbounded = CSGDetail::deepCopyBVH<T, First, K>(Root(m_unbounded, this->location()), a_dstPool);

    layout.m_bvh       = bvh.layout();
    layout.m_unbounded = unbounded.layout();

    return BVHUnion(layout, bvh.location());
  }
  else {
    CSGDetail::deepCopyArrays(this->m_arrays, this->location(), a_dstPool, layout.m_arrays);

    const Root bvh       = this->getBVH().deepCopy(a_dstPool);
    const Root unbounded = Root(m_unbounded, this->location()).deepCopy(a_dstPool);

    layout.m_bvh       = bvh.layout();
    layout.m_unbounded = unbounded.layout();

    return BVHUnion(layout, bvh.location());
  }
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST
BVHSmoothUnion<T, K, Blend, Ps...>::BVHSmoothUnion(Pool&   a_pool,
                                                   const T a_smoothLen,
                                                   const std::vector<Ps>&... a_primitives)
  : BVHSmoothUnion(a_pool, BVH::BuildSettings{}, a_smoothLen, Blend{}, a_primitives...)
{}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST
BVHSmoothUnion<T, K, Blend, Ps...>::BVHSmoothUnion(Pool&       a_pool,
                                                   const T     a_smoothLen,
                                                   const Blend a_blend,
                                                   const std::vector<Ps>&... a_primitives)
  : BVHSmoothUnion(a_pool, BVH::BuildSettings{}, a_smoothLen, a_blend, a_primitives...)
{}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST
BVHSmoothUnion<T, K, Blend, Ps...>::BVHSmoothUnion(Pool&                     a_pool,
                                                   const BVH::BuildSettings& a_settings,
                                                   const T                   a_smoothLen,
                                                   const Blend               a_blend,
                                                   const std::vector<Ps>&... a_primitives)
  : BVHSmoothUnion(
      CSGDetail::buildUnion<T, K, Layout>(a_pool, a_settings, defaultConstructionOptions(), a_primitives...),
      a_smoothLen,
      a_blend)
{
  EBGEOMETRY_REQUIRE(
    a_smoothLen > T(0), "BVHSmoothUnion: the smoothing length must be positive (%g)", double(a_smoothLen));
}

template <class T, size_t K, class Blend, class... Ps>
template <size_t N, std::enable_if_t<N == 1, int>>
EBGEOMETRY_HOST
BVHSmoothUnion<T, K, Blend, Ps...>::BVHSmoothUnion(Pool&                     a_pool,
                                                   const std::vector<First>& a_primitives,
                                                   const std::vector<BV>&    a_boundingVolumes,
                                                   const T                   a_smoothLen,
                                                   const Blend               a_blend,
                                                   const BVH::Construction   a_construction)
  : BVHSmoothUnion(a_pool,
                   a_primitives,
                   a_boundingVolumes,
                   a_smoothLen,
                   a_blend,
                   a_construction,
                   BVHSmoothUnion::defaultConstructionOptions())
{}

template <class T, size_t K, class Blend, class... Ps>
template <size_t N, std::enable_if_t<N == 1, int>>
EBGEOMETRY_HOST
BVHSmoothUnion<T, K, Blend, Ps...>::BVHSmoothUnion(Pool&                           a_pool,
                                                   const std::vector<First>&       a_primitives,
                                                   const std::vector<BV>&          a_boundingVolumes,
                                                   const T                         a_smoothLen,
                                                   const Blend                     a_blend,
                                                   const BVH::Construction         a_construction,
                                                   const BVH::ConstructionOptions& a_options)
  : BVHSmoothUnion(CSGDetail::fromParts<Layout>(CSGDetail::buildUnionParts<T, First, K>(
                     a_pool, a_primitives, a_boundingVolumes, a_construction, a_options)),
                   a_smoothLen,
                   a_blend)
{
  EBGEOMETRY_REQUIRE(
    a_smoothLen > T(0), "BVHSmoothUnion: the smoothing length must be positive (%g)", double(a_smoothLen));
}

template <class T, size_t K, class Blend, class... Ps>
template <class F>
EBGEOMETRY_HOST_DEVICE
inline auto
BVHSmoothUnion<T, K, Blend, Ps...>::visit(const Leaf& a_leaf, F&& a_f) const noexcept
{
  return this->visitAt(a_leaf, this->location(), static_cast<F&&>(a_f));
}

template <class T, size_t K, class Blend, class... Ps>
template <class F>
EBGEOMETRY_HOST_DEVICE
inline auto
BVHSmoothUnion<T, K, Blend, Ps...>::visitAt(const Leaf& a_leaf, const PoolLocation& a_location, F&& a_f) const noexcept
{
  if constexpr (sizeof...(Ps) == 1) {
    return a_f(CSGDetail::fromStored<First>(a_leaf, a_location));
  }
  else {
    return CSGDetail::visitArrays(this->m_arrays, a_leaf.m_type, a_leaf.m_index, a_location, static_cast<F&&>(a_f));
  }
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline T
BVHSmoothUnion<T, K, Blend, Ps...>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  // One type: the leaf is the primitive, evaluated directly, with no type test (this code is kept apart so
  // that a union of one type compiles to the code it needs and nothing more). Several: each leaf resolved
  // through visitAt().
  if constexpr (sizeof...(Ps) == 1) {
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
      insert(closest, CSGDetail::fromStored<First>(unbounded[i], location).signedDistance(a_point));
    }

    const auto primitives = bvh.getPrimitives();

    const auto evalLeaf =
      [&primitives, &a_point, &location, &insert](Closest& a_closest, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = a_offset; i < a_offset + a_count; i++) {
          insert(a_closest,
                 CSGDetail::fromStored<First>(primitives[static_cast<uint32_t>(i)], location).signedDistance(a_point));
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

    bvh.traverse(BVH::PointQuery<T>{a_point}, closest, evalLeaf, pruneDist2);

    return m_blend(closest.a, closest.b, m_smoothLen);
  }
  else {
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

    const auto value = [&a_point](const auto& a_primitive) noexcept -> T {
      return a_primitive.signedDistance(a_point);
    };

    // The unbounded primitives first, so that their values can prune the traversal.
    const PoolLocation location = this->location();
    const Root         bvh      = this->getBVH();
    const Root         unboundedRoot(m_unbounded, location);
    const auto         unbounded = unboundedRoot.getPrimitives();

    for (uint32_t i = 0; i < unbounded.size(); i++) {
      insert(closest, this->visitAt(unbounded[i], location, value));
    }

    const auto primitives = bvh.getPrimitives();

    const auto evalLeaf =
      [this, &primitives, &location, &value, &insert](Closest& a_closest, size_t a_offset, size_t a_count) noexcept {
        for (size_t i = a_offset; i < a_offset + a_count; i++) {
          insert(a_closest, this->visitAt(primitives[static_cast<uint32_t>(i)], location, value));
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

    bvh.traverse(BVH::PointQuery<T>{a_point}, closest, evalLeaf, pruneDist2);

    return m_blend(closest.a, closest.b, m_smoothLen);
  }
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline typename BVHSmoothUnion<T, K, Blend, Ps...>::BV
BVHSmoothUnion<T, K, Blend, Ps...>::computeBoundingVolume() const noexcept
{
  return this->getBVH().computeBoundingVolume().merged(Root(m_unbounded, this->location()).computeBoundingVolume());
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline typename BVHSmoothUnion<T, K, Blend, Ps...>::Root
BVHSmoothUnion<T, K, Blend, Ps...>::getBVH() const noexcept
{
  return Root(m_bvh, this->location());
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline PODSpan<const typename BVHSmoothUnion<T, K, Blend, Ps...>::Leaf>
BVHSmoothUnion<T, K, Blend, Ps...>::getUnboundedPrimitives() const noexcept
{
  const Root unbounded(m_unbounded, this->location());

  return unbounded.getPrimitives();
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST
inline BVHSmoothUnion<T, K, Blend, Ps...>
BVHSmoothUnion<T, K, Blend, Ps...>::rebasedView(const Pool& a_pool) const noexcept
{
  const Root bvh       = this->getBVH();
  const Root unbounded = Root(m_unbounded, this->location());

  if constexpr (sizeof...(Ps) == 1) {
    CSGDetail::checkPrimitivesRebase<T, First, K>(bvh, a_pool);
    CSGDetail::checkPrimitivesRebase<T, First, K>(unbounded, a_pool);
  }
  else {
    const uint64_t endByte = CSGDetail::checkArraysRebase(this->m_arrays, this->location(), a_pool);

    [[maybe_unused]] const PoolLocation arrays = this->rebasedLocation(a_pool, endByte, "BVHSmoothUnion");
  }

  // All parts live in one pool, so they rebase to one location. Rebasing each checks its arrays,
  // and the BVH its depth.
  const PoolLocation location = bvh.rebasedView(a_pool).location();

  [[maybe_unused]] const Root unboundedView = unbounded.rebasedView(a_pool);

  return BVHSmoothUnion(this->layout(), location);
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST
inline BVHSmoothUnion<T, K, Blend, Ps...>
BVHSmoothUnion<T, K, Blend, Ps...>::deepCopy(Pool& a_dstPool) const
{
  // All parts go into a_dstPool, so the copy resolves them against one pool, as the original does.
  Layout layout = this->layout();

  if constexpr (sizeof...(Ps) == 1) {
    const Root bvh       = CSGDetail::deepCopyBVH<T, First, K>(this->getBVH(), a_dstPool);
    const Root unbounded = CSGDetail::deepCopyBVH<T, First, K>(Root(m_unbounded, this->location()), a_dstPool);

    layout.m_bvh       = bvh.layout();
    layout.m_unbounded = unbounded.layout();

    return BVHSmoothUnion(layout, bvh.location());
  }
  else {
    CSGDetail::deepCopyArrays(this->m_arrays, this->location(), a_dstPool, layout.m_arrays);

    const Root bvh       = this->getBVH().deepCopy(a_dstPool);
    const Root unbounded = Root(m_unbounded, this->location()).deepCopy(a_dstPool);

    layout.m_bvh       = bvh.layout();
    layout.m_unbounded = unbounded.layout();

    return BVHSmoothUnion(layout, bvh.location());
  }
}

} // namespace EBGeometry

#endif
