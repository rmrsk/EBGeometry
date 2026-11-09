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
#include "EBGeometry_MemoryResource.hpp"
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
 * @param[out]    a_order           If not null: each storage place's index in a_primitives (the unbounded
 * primitives first, then the BVH's leaves in order). The BVH is then built over the indices, in a scratch pool,
 * and adopted with the primitives in its order: the builds partition by the boxes alone, so the tree is the one
 * a direct build makes.
 * @return The BVH and the unbounded leaf.
 */
template <class T, class P, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
inline UnionParts<T, P, K>
buildUnionParts(Pool&                                         a_pool,
                const std::vector<P>&                         a_primitives,
                const std::vector<BoundingVolumes::AABBT<T>>& a_boundingVolumes,
                const BVH::Construction                       a_construction,
                const BVH::ConstructionOptions&               a_options,
                std::vector<uint32_t>*                        a_order = nullptr)
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
  std::vector<std::pair<S, BV>>        primsAndBVs;
  std::vector<std::pair<uint32_t, BV>> indicesAndBVs;
  std::vector<S>                       unboundedPrims;
  BV                                   unboundedBox;

  if (a_order != nullptr) {
    a_order->clear();
    indicesAndBVs.reserve(a_primitives.size());
  }
  else {
    primsAndBVs.reserve(a_primitives.size());
  }

  for (size_t i = 0; i < a_primitives.size(); i++) {
    if (isUnbounded(a_boundingVolumes[i])) {
      unboundedPrims.push_back(toStored(a_primitives[i]));

      unboundedBox = unboundedBox.merged(a_boundingVolumes[i]);

      if (a_order != nullptr) {
        a_order->push_back(static_cast<uint32_t>(i));
      }
    }
    else if (a_order != nullptr) {
      indicesAndBVs.emplace_back(static_cast<uint32_t>(i), a_boundingVolumes[i]);
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

  if (primsAndBVs.empty() && indicesAndBVs.empty()) {
    return UnionParts<T, P, K>{Root(a_pool, std::vector<Node>{}, std::vector<BV>{}, std::vector<S>{}), unbounded};
  }

  if (a_order != nullptr) {
    Pool scratch(hostMemoryResource());

    const auto indexBVH =
      BVH::PackedBVH<T, uint32_t, K>::build(scratch, std::move(indicesAndBVs), a_construction, a_options);

    // The node types differ by primitive type, but not their shape: each node is copied through its setters
    // (its box row and slot are assigned again when the BVH is adopted).
    std::vector<Node> nodes;
    std::vector<BV>   boxes;
    std::vector<S>    prims;

    nodes.reserve(indexBVH.getNodes().size());
    boxes.reserve(indexBVH.getNodes().size());

    for (const auto& indexNode : indexBVH.getNodes()) {
      Node node;

      node.setPrimitivesOffset(indexNode.getPrimitivesOffset());
      node.setNumPrimitives(indexNode.getNumPrimitives());

      for (size_t k = 0; k < K; k++) {
        node.setChildOffset(indexNode.getChildOffsets()[k], k);
      }

      nodes.push_back(node);
      boxes.push_back(indexBVH.getBoundingVolume(indexNode));
    }

    for (const uint32_t index : indexBVH.getPrimitives()) {
      prims.push_back(toStored(a_primitives[index]));

      a_order->push_back(index);
    }

    return UnionParts<T, P, K>{Root(a_pool, nodes, boxes, prims), unbounded};
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
 * @brief Write a union's members' table into its pool (see BVHUnion::Layout::m_members): a one-type union's
 * storage order, then, if a member type is a container, each member's first surface and the total.
 * @details Internal helper; not part of the public API. A union of several types with no container member has
 * no table: a member's index is read from its MemberRef.
 * @tparam T      Floating-point precision.
 * @tparam Layout The union's layout type.
 * @tparam Ps     Primitive types.
 * @param[in,out] a_pool       The union's pool.
 * @param[in]     a_built      The built union's layout and location.
 * @param[in]     a_order      Each storage place's member index (one type), or empty (several).
 * @param[in]     a_primitives One array per type, in the constructor's order.
 * @return The layout, with its table, and where it lives.
 */
template <class T, class Layout, class... Ps>
[[nodiscard]] EBGEOMETRY_HOST
inline std::pair<Layout, PoolLocation>
attachMembers(Pool&                           a_pool,
              std::pair<Layout, PoolLocation> a_built,
              const std::vector<uint32_t>&    a_order,
              const std::vector<Ps>&... a_primitives)
{
  std::vector<uint32_t> members = a_order;

  if constexpr (hasContainers<T, Ps...>) {
    uint64_t first = 0;

    const auto add = [&members, &first](const auto& a_ofType) {
      for (const auto& primitive : a_ofType) {
        members.push_back(static_cast<uint32_t>(first));

        first += SurfaceDetail::numSurfacesOf<T>(primitive);
      }
    };

    (add(a_primitives), ...);

    EBGEOMETRY_REQUIRE(first < uint64_t(SurfaceRef<T>::None),
                       "BVHUnion: the members hold %llu surfaces, more than 32-bit ids can number",
                       static_cast<unsigned long long>(first));

    members.push_back(static_cast<uint32_t>(first));
  }

  if (!members.empty()) {
    a_built.first.m_members.reserveFrom(a_pool, static_cast<uint32_t>(members.size()));
    a_built.first.m_members.assign(a_pool.base(), members.data(), static_cast<uint32_t>(members.size()));
  }

  return a_built;
}

/**
 * @brief Build a union of one type with the caller's boxes, and its members' table.
 * @details Internal helper; not part of the public API.
 * @tparam T      Floating-point precision.
 * @tparam K      BVH branching factor.
 * @tparam Layout The union's layout type.
 * @tparam P      Primitive type.
 * @param[in,out] a_pool            Pool the union is reserved from.
 * @param[in]     a_primitives      Primitives.
 * @param[in]     a_boundingVolumes Bounding box of each primitive.
 * @param[in]     a_construction    Construction method.
 * @param[in]     a_options         Leaf sizes.
 * @return The layout and where it lives.
 */
template <class T, size_t K, class Layout, class P>
[[nodiscard]] EBGEOMETRY_HOST
inline std::pair<Layout, PoolLocation>
buildOneType(Pool&                                         a_pool,
             const std::vector<P>&                         a_primitives,
             const std::vector<BoundingVolumes::AABBT<T>>& a_boundingVolumes,
             const BVH::Construction                       a_construction,
             const BVH::ConstructionOptions&               a_options)
{
  std::vector<uint32_t> order;

  const auto parts =
    buildUnionParts<T, P, K>(a_pool, a_primitives, a_boundingVolumes, a_construction, a_options, &order);

  return attachMembers<T>(a_pool, fromParts<Layout>(parts), order, a_primitives);
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

    return buildOneType<T, K, Layout>(a_pool, primitives, boxesOf<T>(primitives), a_settings.m_construction, options);
  }
  else {
    return attachMembers<T>(a_pool,
                            buildMixedUnion<T, K, Layout>(a_pool, a_settings.m_construction, options, a_primitives...),
                            std::vector<uint32_t>{},
                            a_primitives...);
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

/**
 * @brief The index of a type's first member: the sizes of the arrays before it.
 * @details Internal helper; not part of the public API.
 * @param[in] a_arrays The arrays, from the type a_type counts from.
 * @param[in] a_type   The type's index among these arrays.
 * @return The index.
 */
template <class P, class... Ps>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline uint32_t
typeStart(const UnionArrays<P, Ps...>& a_arrays, const uint32_t a_type) noexcept
{
  if constexpr (sizeof...(Ps) == 0) {
    return 0U;
  }
  else {
    return (a_type == 0) ? 0U : a_arrays.m_head.size() + typeStart(a_arrays.m_tail, a_type - 1);
  }
}

/**
 * @brief How many members a BVH union (BVHUnion or BVHSmoothUnion) holds.
 * @details Internal helper; not part of the public API.
 * @tparam U Union type.
 * @param[in] a_union The union.
 * @return The count.
 */
template <class U>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline uint32_t
numMembersOf(const U& a_union) noexcept
{
  return a_union.getUnboundedPrimitives().size() + a_union.getBVH().getPrimitives().size();
}

/**
 * @brief A BVH union's member index from its place in the storage (see BVHUnion::getMemberIndex()).
 * @details Internal helper; not part of the public API.
 * @tparam U Union type.
 * @param[in] a_union    The union.
 * @param[in] a_position Place in the storage.
 * @return The member's index.
 */
template <class U>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline uint32_t
memberIndexOf(const U& a_union, const uint32_t a_position) noexcept
{
  EBGEOMETRY_EXPECT(a_position < numMembersOf(a_union));

  const typename U::Layout layout = a_union.layout();

  if constexpr (U::NumTypes == 1) {
    return layout.m_members.at(a_union.location().base(), a_position);
  }
  else {
    const auto unbounded = a_union.getUnboundedPrimitives();
    const auto leaf      = (a_position < unbounded.size()) ? unbounded[a_position]
                                                           : a_union.getBVH().getPrimitives()[a_position - unbounded.size()];

    return typeStart(layout.m_arrays, leaf.m_type) + leaf.m_index;
  }
}

/**
 * @brief A BVH union's member's first surface (see BVHUnion::getFirstSurface()).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam U Union type.
 * @param[in] a_union  The union.
 * @param[in] a_member Member index, at most the number of members.
 * @return The surface.
 */
template <class T, class U>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline uint32_t
firstSurfaceOf(const U& a_union, const uint32_t a_member) noexcept
{
  if constexpr (U::HasContainers) {
    const uint32_t offset = (U::NumTypes == 1) ? numMembersOf(a_union) : 0U;

    EBGEOMETRY_EXPECT(a_member <= numMembersOf(a_union));

    return a_union.layout().m_members.at(a_union.location().base(), offset + a_member);
  }
  else {
    return a_member;
  }
}

/**
 * @brief The surface that decides a BVH union's value at a point (see BVHUnion::closestSurface()): the member
 * with the smallest value, ties to the lower member index, drilled into if it is a container.
 * @details Internal helper; not part of the public API. The traversal is signedDistance()'s for a sharp union,
 * which visits every member whose value can be the smallest; for a smooth union that member is the one that
 * dominates its blend.
 * @tparam T Floating-point precision.
 * @tparam U Union type.
 * @param[in] a_union The union.
 * @param[in] a_point Query point.
 * @return The surface.
 */
template <class T, class U>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline SurfaceRef<T>
closestSurfaceOfUnion(const U& a_union, const Vec3T<T>& a_point) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  struct Best
  {
    T        value    = Math::Limits<T>::infinity();
    uint32_t member   = SurfaceRef<T>::None;
    uint32_t position = SurfaceRef<T>::None;
  };

  const auto bvh        = a_union.getBVH();
  const auto unbounded  = a_union.getUnboundedPrimitives();
  const auto primitives = bvh.getPrimitives();

  const auto valueOf = [&a_point](const auto& a_member) noexcept -> T { return a_member.signedDistance(a_point); };

  const auto consider = [&a_union](Best& a_best, const T a_value, const uint32_t a_position) noexcept {
    EBGEOMETRY_EXPECT(!std::isnan(a_value));

    if (a_value <= a_best.value) {
      const uint32_t member = memberIndexOf(a_union, a_position);

      if (a_value < a_best.value || member < a_best.member) {
        a_best.value    = a_value;
        a_best.member   = member;
        a_best.position = a_position;
      }
    }
  };

  Best best;

  for (uint32_t i = 0; i < unbounded.size(); i++) {
    consider(best, a_union.visit(unbounded[i], valueOf), i);
  }

  const uint32_t numUnbounded = unbounded.size();

  const auto evalLeaf = [&](Best& a_best, size_t a_offset, size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      const uint32_t place = static_cast<uint32_t>(i);

      consider(a_best, a_union.visit(primitives[place], valueOf), numUnbounded + place);
    }
  };

  // As signedDistance() prunes: a node further than the best value so far holds no smaller one. A tie at the
  // bound is kept, so the lower member index can win it.
  const auto pruneDist2 = [](const Best& a_best) noexcept -> T {
    const T bound = Math::max(T(0), a_best.value);

    return bound * bound;
  };

  if (bvh.getNodes().size() > 0) {
    bvh.traverse(BVH::PointQuery<T>{a_point}, best, evalLeaf, pruneDist2);
  }

  SurfaceRef<T> surface;

  if (best.member == SurfaceRef<T>::None) {
    return surface;
  }

  surface.m_surface    = firstSurfaceOf<T>(a_union, best.member);
  surface.m_localPoint = a_point;

  if constexpr (U::HasContainers || U::HasParts) {
    const auto leaf =
      (best.position < numUnbounded) ? unbounded[best.position] : primitives[best.position - numUnbounded];

    const SurfaceRef<T> inner = a_union.visit(leaf, [&a_point](const auto& a_member) noexcept {
      return SurfaceDetail::closestSurfaceOf<T>(a_member, a_point);
    });

    // An empty collection names no surface.
    if (inner.m_surface != SurfaceRef<T>::None) {
      surface.m_surface += inner.m_surface;
      surface.m_part       = inner.m_part;
      surface.m_localPoint = inner.m_localPoint;
    }
    else {
      surface.m_surface = SurfaceRef<T>::None;
    }
  }

  return surface;
}

/**
 * @brief Copy a BVH union's members' table into another pool.
 * @details Internal helper; not part of the public API. The table is read out before anything is reserved.
 * @param[in]     a_members  The table.
 * @param[in]     a_location Where it lives.
 * @param[in,out] a_dstPool  Pool to copy into.
 * @return The copy's table.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline PODVector<uint32_t>
deepCopyMembers(const PODVector<uint32_t>& a_members, const PoolLocation& a_location, Pool& a_dstPool)
{
  PODVector<uint32_t> copy;

  if (a_members.size() > 0) {
    const auto                  span = a_members.bind(a_location.base());
    const std::vector<uint32_t> items(span.begin(), span.end());

    copy.reserveFrom(a_dstPool, static_cast<uint32_t>(items.size()));
    copy.assign(a_dstPool.base(), items.data(), static_cast<uint32_t>(items.size()));
  }

  return copy;
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
  : BVHUnion(CSGDetail::buildOneType<T, K, Layout>(a_pool, a_primitives, a_boundingVolumes, a_construction, a_options))
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
EBGEOMETRY_HOST_DEVICE
inline uint32_t
BVHUnion<T, K, Ps...>::getNumMembers() const noexcept
{
  return CSGDetail::numMembersOf(*this);
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
BVHUnion<T, K, Ps...>::getMemberIndex(const uint32_t a_position) const noexcept
{
  return CSGDetail::memberIndexOf(*this, a_position);
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
BVHUnion<T, K, Ps...>::getNumSurfaces() const noexcept
{
  return CSGDetail::firstSurfaceOf<T>(*this, this->getNumMembers());
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
BVHUnion<T, K, Ps...>::getFirstSurface(const uint32_t a_member) const noexcept
{
  return CSGDetail::firstSurfaceOf<T>(*this, a_member);
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline SurfaceRef<T>
BVHUnion<T, K, Ps...>::closestSurface(const Vec3T<T>& a_point) const noexcept
{
  return CSGDetail::closestSurfaceOfUnion<T>(*this, a_point);
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

  if (m_members.size() > 0) {
    [[maybe_unused]] const PoolLocation members = this->rebasedLocation(a_pool, m_members.endByte(), "BVHUnion");
  }

  return BVHUnion(this->layout(), location);
}

template <class T, size_t K, class... Ps>
EBGEOMETRY_HOST
inline BVHUnion<T, K, Ps...>
BVHUnion<T, K, Ps...>::deepCopy(Pool& a_dstPool) const
{
  // All parts go into a_dstPool, so the copy resolves them against one pool, as the original does.
  Layout layout = this->layout();

  layout.m_members = CSGDetail::deepCopyMembers(m_members, this->location(), a_dstPool);

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
  : BVHSmoothUnion(
      CSGDetail::buildOneType<T, K, Layout>(a_pool, a_primitives, a_boundingVolumes, a_construction, a_options),
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
EBGEOMETRY_HOST_DEVICE
inline uint32_t
BVHSmoothUnion<T, K, Blend, Ps...>::getNumMembers() const noexcept
{
  return CSGDetail::numMembersOf(*this);
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
BVHSmoothUnion<T, K, Blend, Ps...>::getMemberIndex(const uint32_t a_position) const noexcept
{
  return CSGDetail::memberIndexOf(*this, a_position);
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
BVHSmoothUnion<T, K, Blend, Ps...>::getNumSurfaces() const noexcept
{
  return CSGDetail::firstSurfaceOf<T>(*this, this->getNumMembers());
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
BVHSmoothUnion<T, K, Blend, Ps...>::getFirstSurface(const uint32_t a_member) const noexcept
{
  return CSGDetail::firstSurfaceOf<T>(*this, a_member);
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST_DEVICE
inline SurfaceRef<T>
BVHSmoothUnion<T, K, Blend, Ps...>::closestSurface(const Vec3T<T>& a_point) const noexcept
{
  return CSGDetail::closestSurfaceOfUnion<T>(*this, a_point);
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

  if (m_members.size() > 0) {
    [[maybe_unused]] const PoolLocation members = this->rebasedLocation(a_pool, m_members.endByte(), "BVHSmoothUnion");
  }

  return BVHSmoothUnion(this->layout(), location);
}

template <class T, size_t K, class Blend, class... Ps>
EBGEOMETRY_HOST
inline BVHSmoothUnion<T, K, Blend, Ps...>
BVHSmoothUnion<T, K, Blend, Ps...>::deepCopy(Pool& a_dstPool) const
{
  // All parts go into a_dstPool, so the copy resolves them against one pool, as the original does.
  Layout layout = this->layout();

  layout.m_members = CSGDetail::deepCopyMembers(m_members, this->location(), a_dstPool);

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
