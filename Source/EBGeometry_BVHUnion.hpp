// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHUnion.hpp
 * @brief  BVH-accelerated unions of many primitives of one or more types: BVHUnion and BVHSmoothUnion.
 * @details Device-callable value types. They need the BVH builders and the blend operators, but
 * not the expression builder, the Tape or the fused operations that EBGeometry_CSG.hpp also includes.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHUNION_HPP
#define EBGEOMETRY_BVHUNION_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_Blend.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_SurfaceRef.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace BVH {

/**
 * @brief How a BVH union is built: the construction method and, optionally, its leaf sizes.
 * @details Passed as one argument before the primitive arrays (see BVHUnion), so that the arrays can be a
 * parameter pack.
 */
struct BuildSettings
{
  /**
   * @brief Preset construction method; every BVH::Construction value is supported.
   */
  Construction m_construction = Construction::SAH;

  /**
   * @brief Leaf-size settings; empty for the union's defaultConstructionOptions().
   */
  std::optional<ConstructionOptions> m_options;
};

} // namespace BVH

/**
 * @brief Internal helpers shared by BVHUnion and BVHSmoothUnion.
 */
namespace CSGDetail {

/**
 * @brief How a BVH union stores a primitive of type P: as is, or, for a pool-resident handle, as the
 * handle's layout.
 * @details A handle's location would be copied verbatim into the union's pool and be wrong in any
 * mirror of it; its layout holds only offsets, which are right in every mirror. See PoolHandle.
 * @tparam P Primitive type.
 */
template <class P, bool = isPoolHandle<P>>
struct StoredAs
{
  /**
   * @brief The stored type: P itself.
   */
  using type = P;
};

/**
 * @brief How a BVH union stores a pool-resident handle: as its layout.
 * @tparam P Primitive type.
 */
template <class P>
struct StoredAs<P, true>
{
  /**
   * @brief The stored type: P's layout.
   */
  using type = typename P::Layout;
};

/**
 * @brief Shorthand for the type a BVH union stores for a primitive of type P.
 * @tparam P Primitive type.
 */
template <class P>
using Stored = typename StoredAs<P>::type;

/**
 * @brief Convert a primitive to the form a BVH union stores.
 * @tparam P Primitive type.
 * @param[in] a_primitive Primitive.
 * @return The primitive, or its layout if it is a pool-resident handle.
 */
template <class P>
[[nodiscard]] EBGEOMETRY_HOST
inline Stored<P>
toStored(const P& a_primitive) noexcept
{
  if constexpr (isPoolHandle<P>) {
    return a_primitive.layout();
  }
  else {
    return a_primitive;
  }
}

/**
 * @brief The primitive a stored entry stands for, resolving against @p a_location.
 * @details A pool-resident primitive is rebuilt from its layout and the union's own location, so it
 * resolves against the same pool the union does: the host pool, a host mirror or a device mirror.
 * Any other primitive is returned as stored.
 * @tparam P Primitive type.
 * @param[in] a_stored   Primitive as stored in the union's BVH.
 * @param[in] a_location The union's pool location.
 * @return The primitive.
 */
template <class P>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline P
fromStored(const Stored<P>& a_stored, [[maybe_unused]] const PoolLocation& a_location) noexcept
{
  if constexpr (isPoolHandle<P>) {
    return P(a_stored, a_location);
  }
  else {
    return a_stored;
  }
}

/**
 * @brief What a BVH union is built into: a BVH over the primitives whose bounding box is bounded,
 * and a single leaf (or nothing) holding those whose box is unbounded in some direction.
 * @details Internal helper; not part of the public API. Both live in the same Pool.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 */
template <class T, class P, size_t K>
struct UnionParts
{
  /**
   * @brief BVH over the primitives with bounded boxes.
   */
  BVH::PackedBVH<T, Stored<P>, K> bvh;

  /**
   * @brief The primitives with unbounded boxes, as one leaf; empty if there are none.
   */
  BVH::PackedBVH<T, Stored<P>, K> unbounded;
};

/**
 * @brief A leaf of a union of several primitive types: the primitive's type, and its index in that type's
 * array.
 * @details Internal helper; not part of the public API.
 */
struct MemberRef
{
  /**
   * @brief Index of the primitive's type among the union's types.
   */
  uint32_t m_type;

  /**
   * @brief Index of the primitive in its type's array.
   */
  uint32_t m_index;
};

/**
 * @brief Whether a union holds members that are containers themselves (see hasSurfaces), whose surfaces it
 * numbers through a table of first surfaces.
 * @details Internal helper; not part of the public API.
 * @tparam T  Floating-point precision.
 * @tparam Ps Primitive types.
 */
template <class T, class... Ps>
inline constexpr bool hasContainers = (hasSurfaces<Ps, T> || ...);

/**
 * @brief The arrays of a union of several primitive types, one per type in the order given: a recursive
 * list, so that it stays trivially copyable and device-visible.
 * @details Internal helper; not part of the public API.
 * @tparam Ps Primitive types.
 */
template <class... Ps>
struct UnionArrays
{
};

/**
 * @brief The arrays of a union of several primitive types: the first type's, and the rest.
 * @details Internal helper; not part of the public API.
 * @tparam P  First primitive type.
 * @tparam Ps The others.
 */
template <class P, class... Ps>
struct UnionArrays<P, Ps...>
{
  /**
   * @brief The first type's primitives, as stored (see Stored).
   */
  PODVector<Stored<P>> m_head;

  /**
   * @brief The other types' arrays.
   */
  UnionArrays<Ps...> m_tail;
};

/**
 * @brief What a union of several types adds to the one-type layout: its per-type arrays. Empty for one
 * type, whose leaves hold the primitives themselves, so that a union of one type holds nothing but its two
 * BVHs.
 * @details Internal helper; not part of the public API. A base of the union and of its Layout, so that
 * the empty case takes no space.
 * @tparam Ps Primitive types.
 */
template <class... Ps>
struct ArraysOf
{
  /**
   * @brief The per-type arrays.
   */
  UnionArrays<Ps...> m_arrays;
};

/**
 * @brief A union of one type has no per-type arrays.
 * @details Internal helper; not part of the public API.
 * @tparam P Primitive type.
 */
template <class P>
struct ArraysOf<P>
{
};

/**
 * @brief What a union's BVH leaves hold: the primitive itself for one type, a MemberRef for several.
 * @details Internal helper; not part of the public API.
 * @tparam Ps Primitive types.
 */
template <class... Ps>
struct LeafOf
{
  /**
   * @brief A reference into the per-type arrays.
   */
  using type = MemberRef;
};

/**
 * @brief What a union of one type's BVH leaves hold: the primitive, as stored.
 * @details Internal helper; not part of the public API.
 * @tparam P Primitive type.
 */
template <class P>
struct LeafOf<P>
{
  /**
   * @brief The stored primitive.
   */
  using type = Stored<P>;
};

/**
 * @brief Shorthand for what a union's BVH leaves hold.
 * @tparam Ps Primitive types.
 */
template <class... Ps>
using Leaf = typename LeafOf<Ps...>::type;

/**
 * @brief The precision of a primitive type: the T of the box its computeBoundingVolume() returns.
 * @details Internal helper; not part of the public API.
 * @tparam B The box type.
 */
template <class B>
struct BoxPrecision;

/**
 * @brief The precision of an AABBT<T>.
 * @tparam T Floating-point precision.
 */
template <class T>
struct BoxPrecision<BoundingVolumes::AABBT<T>>
{
  /**
   * @brief The precision.
   */
  using type = T;
};

/**
 * @brief The precision of primitive type P, for the unions' deduction guides.
 * @tparam P Primitive type.
 */
template <class P>
using PrimitivePrecision =
  typename BoxPrecision<std::decay_t<decltype(std::declval<const P&>().computeBoundingVolume())>>::type;

/**
 * @brief Whether a primitive type is of precision T: true if its computeBoundingVolume() returns an
 * AABBT<T>, and for a type without one (a one-type union given the caller's boxes), which says nothing.
 * @tparam P Primitive type.
 * @tparam T Floating-point precision.
 */
template <class P, class T, class = void>
inline constexpr bool hasPrecision = true;

/**
 * @brief Whether a primitive type with a computeBoundingVolume() is of precision T.
 * @tparam P Primitive type.
 * @tparam T Floating-point precision.
 */
template <class P, class T>
inline constexpr bool hasPrecision<P, T, std::void_t<decltype(std::declval<const P&>().computeBoundingVolume())>> =
  std::is_same_v<PrimitivePrecision<P>, T>;

/**
 * @brief Whether X is a box, which an array of primitives never holds: keeps the deduction guide for several
 * types from taking a one-type union's box array for a second type.
 * @tparam X Type to inspect.
 */
template <class X>
inline constexpr bool isBox = false;

/**
 * @brief An AABBT is a box.
 * @tparam T Floating-point precision.
 */
template <class T>
inline constexpr bool isBox<BoundingVolumes::AABBT<T>> = true;

/**
 * @brief Whether X is a std::vector: keeps the smooth unions' deduction guides that take a blend from taking
 * the first array for one.
 * @tparam X Type to inspect.
 */
template <class X>
inline constexpr bool isVector = false;

/**
 * @brief A std::vector is a vector.
 * @tparam V Element type.
 * @tparam A Allocator.
 */
template <class V, class A>
inline constexpr bool isVector<std::vector<V, A>> = true;

/**
 * @brief The weakest distance quality among primitive types: not a distance if any is not, else a bound.
 * @tparam Ps Primitive types.
 */
template <class... Ps>
inline constexpr DistanceQuality unionQuality =
  ((distanceQualityOf<Ps> == DistanceQuality::NotADistance) || ...) ? DistanceQuality::NotADistance
                                                                    : DistanceQuality::Bound;

/**
 * @brief Call @p a_f with the primitive a MemberRef stands for, testing its type against the arrays' types in
 * order (the last is taken without a test).
 * @details Internal helper; not part of the public API.
 * @tparam F  Callable on each primitive type, returning the same type for all.
 * @tparam P  First primitive type of the arrays.
 * @tparam Ps The others.
 * @param[in] a_arrays   The arrays, from the type a_type counts from.
 * @param[in] a_type     The type's index among these arrays.
 * @param[in] a_index    The primitive's index in its type's array.
 * @param[in] a_location The union's location, which its arrays and any pool-resident primitive resolve against.
 * @param[in] a_f        What to call.
 * @return What @p a_f returns.
 */
template <class F, class P, class... Ps>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline auto
visitArrays(const UnionArrays<P, Ps...>& a_arrays,
            const uint32_t               a_type,
            const uint32_t               a_index,
            const PoolLocation&          a_location,
            F&&                          a_f) noexcept
{
  if constexpr (sizeof...(Ps) == 0) {
    EBGEOMETRY_EXPECT(a_type == 0);

    return a_f(fromStored<P>(a_arrays.m_head.at(a_location.base(), a_index), a_location));
  }
  else {
    if (a_type == 0) {
      return a_f(fromStored<P>(a_arrays.m_head.at(a_location.base(), a_index), a_location));
    }

    return visitArrays(a_arrays.m_tail, a_type - 1, a_index, a_location, static_cast<F&&>(a_f));
  }
}

} // namespace CSGDetail

/**
 * @brief BVH-accelerated union of many primitives of one or more types.
 * @details A PackedBVH over the primitives, stored by value in a Pool. At query time the BVH culls
 * primitives whose bounding volume lies further than the current best distance, giving sub-linear
 * cost on large scenes. The value is the minimum over all primitives, negative inside the union.
 *
 * This is a plain, trivially copyable value type with no virtual functions. signedDistance() is
 * callable on the host and on a device: mirror the pool, call rebasedView(), and pass the result to a
 * kernel. The primitive types are deduced from the arrays given, `BVHUnion(pool, spheres, boxes, mine)`.
 * With one type, the BVH's leaves hold the primitives themselves and no type is tested. With several, each
 * type's primitives are one array in the pool, a leaf holds a primitive's type and index, and evaluating a
 * leaf tests its type against the types in the order given (the last without a test): put the most
 * numerous first.
 *
 * Each primitive type can be any trivially copyable type with an EBGEOMETRY_HOST_DEVICE
 * `signedDistance(const Vec3T<T>&)` and a `computeBoundingVolume()`: an analytic shape, a mesh distance
 * function, a fused composition, another BVH union, or a type of your own. Pruning needs only that each
 * bounding volume encloses its primitive's object (where the primitive's value is not positive): a
 * primitive skipped because its box is further away than the best value so far is positive at the query
 * point, so skipping it never changes the union's sign. Its value is a distance bound if the primitives'
 * values are, and only a sign if any type's is DistanceQuality::NotADistance (see distanceQuality).
 *
 * A primitive whose bounding box is unbounded in some direction (it reaches plus or minus
 * Math::Limits<T>::max(), as an InfiniteCylinderSDF's does along its axis) is kept out of the BVH:
 * such boxes would make every ancestor's box unbounded and the build heuristics meaningless. Those
 * primitives are evaluated at every query instead, before the traversal, so their values can prune
 * it. A union of many bounded primitives and a few unbounded ones costs the BVH query plus one
 * evaluation per unbounded primitive.
 *
 * A pool-resident primitive (a mesh distance function or a union; see PoolHandle) must have been
 * built in the same Pool as this union. The union stores its layout, and rebuilds it with the union's
 * own location as it evaluates it. The constructors check the pool, and that there is one bounding
 * volume per primitive, in every build, and abort with a message if either fails.
 * @tparam T  Floating-point precision.
 * @tparam K  BVH branching factor.
 * @tparam Ps Primitive types (at least one).
 */
template <class T, size_t K, class... Ps>
class BVHUnion : public PoolHandle, private CSGDetail::ArraysOf<Ps...>
{
public:
  static_assert(std::is_floating_point_v<T>, "BVHUnion requires a floating-point type T");
  static_assert(sizeof...(Ps) > 0, "BVHUnion requires at least one primitive type");
  static_assert((CSGDetail::hasPrecision<Ps, T> && ...),
                "BVHUnion requires every primitive type to have the union's precision T");
  static_assert((std::is_trivially_copyable_v<Ps> && ...), "BVHUnion requires trivially copyable primitive types");
  static_assert(K > 1, "BVHUnion BVH branching factor K must be at least 2");

  /**
   * @brief How far signedDistance() can be trusted as a distance: a bound, or only a sign if any
   * primitive type's values are not distances. With exact primitives the union is the exact minimum,
   * which is exact outside but underestimates inside overlapping primitives.
   */
  static constexpr DistanceQuality distanceQuality = CSGDetail::unionQuality<Ps...>;

  /**
   * @brief How many primitive types the union holds.
   */
  static constexpr size_t NumTypes = sizeof...(Ps);

  /**
   * @brief Whether a primitive type is a container (see hasSurfaces), whose surfaces the union numbers too.
   */
  static constexpr bool HasContainers = CSGDetail::hasContainers<T, Ps...>;

  /**
   * @brief Whether a primitive type has parts (see hasParts), which closestSurface() reports.
   */
  static constexpr bool HasParts = (hasParts<Ps, T> || ...);

  /**
   * @brief What the BVH's leaves hold: with one type the primitive (or its layout, for a pool-resident
   * one), with several a reference to it in its type's array; visit() resolves either.
   */
  using Leaf = CSGDetail::Leaf<Ps...>;

  /**
   * @brief Alias for the packed BVH type.
   */
  using Root = BVH::PackedBVH<T, Leaf, K>;

  /**
   * @brief Alias for the bounding volume type.
   */
  using BV = BoundingVolumes::AABBT<T>;

  /**
   * @brief The first primitive type: the only one for the constructors that take boxes.
   */
  using First = std::tuple_element_t<0, std::tuple<Ps...>>;

  /**
   * @brief Disallowed, use the full constructor.
   */
  BVHUnion() = delete;

  /**
   * @brief The leaf-size settings the constructors without options use.
   * @details K-1 primitives per leaf for the top-down methods (a node with fewer than K primitives is
   * a leaf), a target of K for the space-filling curves, and the default ClusterSpec for ClusterSAH.
   * Start from these to change one method's setting; see BVH::ConstructionOptions.
   * @return The default settings.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static BVH::ConstructionOptions
  defaultConstructionOptions() noexcept
  {
    return Root::defaultConstructionOptions();
  }

  /**
   * @brief Build the union of the given primitives, with SAH and the default leaf sizes.
   * @details Each primitive's box is its computeBoundingVolume().
   * @param[in,out] a_pool       Pool the union is reserved from; pool-resident primitives must live in it
   * too. Must outlive this object and every copy of it.
   * @param[in]     a_primitives One array per primitive type; empty arrays are allowed, but not all of them.
   */
  EBGEOMETRY_HOST
  explicit BVHUnion(Pool& a_pool, const std::vector<Ps>&... a_primitives);

  /**
   * @brief Build the union with a construction method and, optionally, leaf sizes.
   * @details As the constructor without settings. The chosen method reads only its own field of the leaf
   * sizes; see BVH::ConstructionOptions.
   * @param[in,out] a_pool       Pool the union is reserved from; see the constructor without settings.
   * @param[in]     a_settings   Construction method and leaf sizes.
   * @param[in]     a_primitives One array per primitive type; empty arrays are allowed, but not all of them.
   */
  EBGEOMETRY_HOST
  BVHUnion(Pool& a_pool, const BVH::BuildSettings& a_settings, const std::vector<Ps>&... a_primitives);

  /**
   * @brief Build a union of one type with the caller's bounding boxes, which may be tighter than the
   * primitives' own.
   * @param[in,out] a_pool            Pool the union is reserved from; see the constructor without settings.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_construction    Preset construction method; every BVH::Construction value is supported.
   * The leaf sizes are defaultConstructionOptions().
   */
  template <size_t N = sizeof...(Ps), std::enable_if_t<N == 1, int> = 0>
  EBGEOMETRY_HOST
  BVHUnion(Pool&                     a_pool,
           const std::vector<First>& a_primitives,
           const std::vector<BV>&    a_boundingVolumes,
           BVH::Construction         a_construction = BVH::Construction::SAH);

  /**
   * @brief Build a union of one type with the caller's bounding boxes and leaf-size settings.
   * @param[in,out] a_pool            Pool the union is reserved from; see the constructor without settings.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_construction    Preset construction method; every BVH::Construction value is supported.
   * @param[in]     a_options         Leaf-size settings, in primitives.
   */
  template <size_t N = sizeof...(Ps), std::enable_if_t<N == 1, int> = 0>
  EBGEOMETRY_HOST
  BVHUnion(Pool&                           a_pool,
           const std::vector<First>&       a_primitives,
           const std::vector<BV>&          a_boundingVolumes,
           BVH::Construction               a_construction,
           const BVH::ConstructionOptions& a_options);

  /**
   * @brief Evaluate the union at a point.
   * @details Returns the minimum value over all primitives not pruned by the BVH. Negative when
   * inside the union.
   * @param[in] a_point 3D query point.
   * @return Minimum signed distance among all non-culled primitives.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief How many members (primitives) the union holds.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumMembers() const noexcept;

  /**
   * @brief A member's index in the order the constructor was given the primitives (type by type, in the order
   * of the arrays, each array in its own order), from its place in the union's storage: the unbounded members
   * first (getUnboundedPrimitives()), then the BVH's leaves in order (getBVH().getPrimitives()).
   * @param[in] a_position Place in the storage, below getNumMembers().
   * @return The member's index.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getMemberIndex(uint32_t a_position) const noexcept;

  /**
   * @brief How many surfaces the union numbers (see hasSurfaces): one per member, or a container member's own
   * getNumSurfaces().
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSurfaces() const noexcept;

  /**
   * @brief A member's first surface: member i covers [getFirstSurface(i), getFirstSurface(i + 1)).
   * @param[in] a_member Member index, in the constructor's order, at most getNumMembers().
   * @return The surface.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getFirstSurface(uint32_t a_member) const noexcept;

  /**
   * @brief The surface that decides the union's value at a point (see hasSurfaces): the member with the
   * smallest value, ties going to the lower member index (with several types, to the earlier type), drilled into
   * if it is a container.
   * @param[in] a_point Query point.
   * @return The surface, and the point in its member's frame.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceRef<T>
  closestSurface(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The axis-aligned bounding box enclosing all primitives.
   * @return The BVH's root box merged with the boxes of the unbounded primitives.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BV
  computeBoundingVolume() const noexcept;

  /**
   * @brief Get the packed BVH over the primitives whose bounding box is bounded.
   * @details The primitives with unbounded boxes are not in it; see getUnboundedPrimitives().
   * @return The BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Root
  getBVH() const noexcept;

  /**
   * @brief Get the leaves of the primitives whose bounding box is unbounded, which every query evaluates.
   * @details The span is a resolved address into pool memory; the lifetime caveat of
   * BVH::PackedBVH::getPrimitives() applies.
   * @return Span over those leaves; empty if there are none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PODSpan<const Leaf>
  getUnboundedPrimitives() const noexcept;

  /**
   * @brief Call @p a_f with the primitive a leaf stands for (one of the BVH's leaves, or of
   * getUnboundedPrimitives()), and return what it returns.
   * @details For the ray forms and other code that walks the union's leaves itself. With several types,
   * @p a_f is called with the primitive's own type, so it must accept each of them and return the same type
   * for all. Callable on the host and on a device.
   * @tparam F Callable on each primitive type.
   * @param[in] a_leaf A leaf of this union.
   * @param[in] a_f    What to call.
   * @return What @p a_f returns.
   */
  template <class F>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline auto
  visit(const Leaf& a_leaf, F&& a_f) const noexcept;

  /**
   * @brief Produce a copy of this union that resolves against @p a_pool.
   * @details Rebases the BVH; see BVH::PackedBVH::rebasedView() for the contract. Pool-resident
   * primitives are stored as layouts and resolve against the union's location; each is still rebased
   * once here, and the result discarded, purely so that the checks rebasedView() makes (a mirror of
   * the right pool, a BVH shallow enough for the device traversal stack) run for them too. This is
   * the one sanctioned crossing to a device.
   * @param[in] a_pool Pool to rebase onto: the union's own pool or one in its mirror chain.
   * @return A BVHUnion resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BVHUnion
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate this union's storage, and that of any pool-resident primitive, into @p a_dstPool.
   * @param[in,out] a_dstPool Pool to reserve the copy from.
   * @return A BVHUnion with the same structure, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BVHUnion
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief The union's arrays as offsets into its pool, with no location: what an outer union
   * stores when this union is its primitive (see PoolHandle). With several types it also holds the
   * per-type arrays (its base).
   */
  struct Layout : CSGDetail::ArraysOf<Ps...>
  {
    /**
     * @brief BVH over the primitives with bounded boxes.
     */
    typename Root::Layout m_bvh;

    /**
     * @brief The primitives with unbounded boxes, as one leaf.
     */
    typename Root::Layout m_unbounded;

    /**
     * @brief The members' table: with one type, each storage place's member index (see getMemberIndex()); then,
     * if a member type is a container, each member's first surface and the total.
     */
    PODVector<uint32_t> m_members;
  };

  /**
   * @brief Rebuild a handle from a stored layout and the location of the pool that holds it.
   * @details @p a_location must belong to the pool the layout was reserved from, or to a mirror of
   * it; nothing is checked, since a device has no pool to check against.
   * @param[in] a_layout   The union's arrays.
   * @param[in] a_location Where they live.
   */
  EBGEOMETRY_HOST_DEVICE
  BVHUnion(const Layout& a_layout, const PoolLocation& a_location) noexcept
    : PoolHandle(a_location),
      CSGDetail::ArraysOf<Ps...>(a_layout),
      m_bvh(a_layout.m_bvh),
      m_unbounded(a_layout.m_unbounded),
      m_members(a_layout.m_members)
  {}

  /**
   * @brief The union's arrays without its location, for storing it inside an outer union.
   * @return The layout.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Layout
  layout() const noexcept
  {
    Layout layout{};

    static_cast<CSGDetail::ArraysOf<Ps...>&>(layout) = *this;
    layout.m_bvh                                     = m_bvh;
    layout.m_unbounded                               = m_unbounded;
    layout.m_members                                 = m_members;

    return layout;
  }

private:
  /**
   * @brief Call @p a_f with the primitive a leaf stands for, resolved against a location the caller read once
   * (visit() with the location given).
   * @tparam F Callable on each primitive type.
   * @param[in] a_leaf     A leaf of this union.
   * @param[in] a_location The union's location.
   * @param[in] a_f        What to call.
   * @return What @p a_f returns.
   */
  template <class F>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline auto
  visitAt(const Leaf& a_leaf, const PoolLocation& a_location, F&& a_f) const noexcept;
  /**
   * @brief Adopt the built parts, which share one pool; used by the constructors.
   * @param[in] a_built The layout (the BVH, the unbounded primitives and, with several types, the arrays) and
   * where it lives.
   */
  EBGEOMETRY_HOST
  explicit BVHUnion(const std::pair<Layout, PoolLocation>& a_built) noexcept : BVHUnion(a_built.first, a_built.second)
  {}

  /**
   * @brief Packed BVH over the primitives with bounded boxes, as a layout.
   */
  typename Root::Layout m_bvh;

  /**
   * @brief The primitives with unbounded boxes, as a single leaf (or empty), as a layout.
   */
  typename Root::Layout m_unbounded;

  /**
   * @brief The members' table (see Layout::m_members).
   */
  PODVector<uint32_t> m_members;
};

/**
 * @brief A union of the given arrays' types, of the first type's precision and the default branching factor.
 * @tparam P  First primitive type.
 * @tparam Ps The others (none a box, which a one-type union's array of boxes is).
 * @param[in] a_pool Pool.
 * @param[in] a_first The first type's primitives.
 * @param[in] a_primitives The other types' primitives.
 */
template <class P, class... Ps, std::enable_if_t<!(CSGDetail::isBox<Ps> || ...), int> = 0>
BVHUnion(Pool& a_pool, const std::vector<P>& a_first, const std::vector<Ps>&... a_primitives)
  -> BVHUnion<CSGDetail::PrimitivePrecision<P>,
              BVH::DefaultBranchingRatio<CSGDetail::PrimitivePrecision<P>>(),
              P,
              Ps...>;

/**
 * @brief A union of the given arrays' types, built with settings.
 * @tparam P  First primitive type.
 * @tparam Ps The others.
 * @param[in] a_pool Pool.
 * @param[in] a_settings Construction method and leaf sizes.
 * @param[in] a_first The first type's primitives.
 * @param[in] a_primitives The other types' primitives.
 */
template <class P, class... Ps>
BVHUnion(Pool&                     a_pool,
         const BVH::BuildSettings& a_settings,
         const std::vector<P>&     a_first,
         const std::vector<Ps>&... a_primitives)
  -> BVHUnion<CSGDetail::PrimitivePrecision<P>,
              BVH::DefaultBranchingRatio<CSGDetail::PrimitivePrecision<P>>(),
              P,
              Ps...>;

/**
 * @brief A union of one type with the caller's boxes.
 * @tparam P Primitive type.
 * @tparam B Box type.
 * @param[in] a_pool Pool.
 * @param[in] a_first The first type's primitives.
 * @param[in] a_boxes The primitives' boxes.
 */
template <class P, class B, std::enable_if_t<CSGDetail::isBox<B>, int> = 0>
BVHUnion(Pool& a_pool, const std::vector<P>& a_first, const std::vector<B>& a_boxes)
  -> BVHUnion<CSGDetail::PrimitivePrecision<P>, BVH::DefaultBranchingRatio<CSGDetail::PrimitivePrecision<P>>(), P>;

/**
 * @brief A union of one type with the caller's boxes and a construction method.
 * @tparam P Primitive type.
 * @tparam B Box type.
 * @param[in] a_pool Pool.
 * @param[in] a_first The first type's primitives.
 * @param[in] a_boxes The primitives' boxes.
 * @param[in] a_construction Construction method.
 * @param[in] a_rest The leaf sizes (and for a smooth union, the construction method), if given.
 */
template <class P, class B, class... Rest, std::enable_if_t<CSGDetail::isBox<B>, int> = 0>
BVHUnion(Pool&                 a_pool,
         const std::vector<P>& a_first,
         const std::vector<B>& a_boxes,
         BVH::Construction     a_construction,
         const Rest&... a_rest)
  -> BVHUnion<CSGDetail::PrimitivePrecision<P>, BVH::DefaultBranchingRatio<CSGDetail::PrimitivePrecision<P>>(), P>;

/**
 * @brief BVH-accelerated smooth union of many primitives of one or more types.
 * @details Uses the BVH to find the two primitives with the smallest values and blends those values
 * with a smooth-minimum operator, yielding C1 (or better) blending across nearby surfaces with
 * sub-linear query cost. Otherwise identical to BVHUnion (the primitive types, deduced from the arrays,
 * and how several are stored): a plain, trivially copyable value type whose signedDistance() is callable on
 * the host and on a device.
 * @tparam T     Floating-point precision.
 * @tparam K     BVH branching factor.
 * @tparam Blend Smooth-minimum operator: a trivially copyable function object with an
 * EBGEOMETRY_HOST_DEVICE `T operator()(const T& a, const T& b, const T& s) const`, such as
 * SmoothMinOp (the default) or ExpMinOp.
 * @tparam Ps    Primitive types (at least one); see BVHUnion.
 */
template <class T, size_t K, class Blend = SmoothMinOp<T>, class... Ps>
class BVHSmoothUnion : public PoolHandle, private CSGDetail::ArraysOf<Ps...>
{
public:
  static_assert(std::is_floating_point_v<T>, "BVHSmoothUnion requires a floating-point type T");
  static_assert(sizeof...(Ps) > 0, "BVHSmoothUnion requires at least one primitive type");
  static_assert((CSGDetail::hasPrecision<Ps, T> && ...),
                "BVHSmoothUnion requires every primitive type to have the union's precision T");
  static_assert((std::is_trivially_copyable_v<Ps> && ...),
                "BVHSmoothUnion requires trivially copyable primitive types");
  static_assert(std::is_trivially_copyable_v<Blend>, "BVHSmoothUnion requires a trivially copyable blend operator");
  static_assert(K > 1, "BVHSmoothUnion BVH branching factor K must be at least 2");

  /**
   * @brief How far signedDistance() can be trusted as a distance: a bound, or only a sign if any
   * primitive type's values are not distances. SmoothMinOp and ExpMinOp weight the two primitives'
   * gradients by weights that sum to one, so they do not make the blend steeper than its inputs.
   */
  static constexpr DistanceQuality distanceQuality = CSGDetail::unionQuality<Ps...>;

  /**
   * @brief How many primitive types the union holds.
   */
  static constexpr size_t NumTypes = sizeof...(Ps);

  /**
   * @brief Whether a primitive type is a container (see hasSurfaces), whose surfaces the union numbers too.
   */
  static constexpr bool HasContainers = CSGDetail::hasContainers<T, Ps...>;

  /**
   * @brief Whether a primitive type has parts (see hasParts), which closestSurface() reports.
   */
  static constexpr bool HasParts = (hasParts<Ps, T> || ...);

  /**
   * @brief What the BVH's leaves hold; see BVHUnion::Leaf.
   */
  using Leaf = CSGDetail::Leaf<Ps...>;

  /**
   * @brief Alias for the packed BVH type.
   */
  using Root = BVH::PackedBVH<T, Leaf, K>;

  /**
   * @brief Alias for the bounding volume type.
   */
  using BV = BoundingVolumes::AABBT<T>;

  /**
   * @brief The first primitive type: the only one for the constructors that take boxes.
   */
  using First = std::tuple_element_t<0, std::tuple<Ps...>>;

  /**
   * @brief Disallowed, use the full constructor.
   */
  BVHSmoothUnion() = delete;

  /**
   * @brief The leaf-size settings the constructors without options use; the same as BVHUnion's.
   * @return The default settings.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static BVH::ConstructionOptions
  defaultConstructionOptions() noexcept
  {
    return Root::defaultConstructionOptions();
  }

  /**
   * @brief Build the smooth union of the given primitives with a default-constructed Blend, SAH and the default
   * leaf sizes.
   * @details Each primitive's box is its computeBoundingVolume().
   * @param[in,out] a_pool       Pool the union is reserved from; see BVHUnion.
   * @param[in]     a_smoothLen  Smoothing length (must be > 0).
   * @param[in]     a_primitives One array per primitive type; empty arrays are allowed, but not all of them.
   */
  EBGEOMETRY_HOST
  BVHSmoothUnion(Pool& a_pool, T a_smoothLen, const std::vector<Ps>&... a_primitives);

  /**
   * @brief Build the smooth union of the given primitives with a smooth-minimum operator.
   * @param[in,out] a_pool       Pool the union is reserved from; see BVHUnion.
   * @param[in]     a_smoothLen  Smoothing length (must be > 0).
   * @param[in]     a_blend      Smooth-minimum operator.
   * @param[in]     a_primitives One array per primitive type; empty arrays are allowed, but not all of them.
   */
  EBGEOMETRY_HOST
  BVHSmoothUnion(Pool& a_pool, T a_smoothLen, Blend a_blend, const std::vector<Ps>&... a_primitives);

  /**
   * @brief Build the smooth union with a construction method and, optionally, leaf sizes.
   * @param[in,out] a_pool       Pool the union is reserved from; see BVHUnion.
   * @param[in]     a_settings   Construction method and leaf sizes.
   * @param[in]     a_smoothLen  Smoothing length (must be > 0).
   * @param[in]     a_blend      Smooth-minimum operator.
   * @param[in]     a_primitives One array per primitive type; empty arrays are allowed, but not all of them.
   */
  EBGEOMETRY_HOST
  BVHSmoothUnion(Pool&                     a_pool,
                 const BVH::BuildSettings& a_settings,
                 T                         a_smoothLen,
                 Blend                     a_blend,
                 const std::vector<Ps>&... a_primitives);

  /**
   * @brief Build a smooth union of one type with the caller's bounding boxes.
   * @param[in,out] a_pool            Pool the union is reserved from; see BVHUnion.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_smoothLen       Smoothing length (must be > 0).
   * @param[in]     a_blend           Smooth-minimum operator.
   * @param[in]     a_construction    Preset construction method; every BVH::Construction value is supported.
   * The leaf sizes are defaultConstructionOptions().
   */
  template <size_t N = sizeof...(Ps), std::enable_if_t<N == 1, int> = 0>
  EBGEOMETRY_HOST
  BVHSmoothUnion(Pool&                     a_pool,
                 const std::vector<First>& a_primitives,
                 const std::vector<BV>&    a_boundingVolumes,
                 T                         a_smoothLen,
                 Blend                     a_blend        = Blend{},
                 BVH::Construction         a_construction = BVH::Construction::SAH);

  /**
   * @brief Build a smooth union of one type with the caller's bounding boxes and leaf-size settings.
   * @details The chosen method reads only its own field of @p a_options; see BVH::ConstructionOptions.
   * @param[in,out] a_pool            Pool the union is reserved from; see BVHUnion.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_smoothLen       Smoothing length (must be > 0).
   * @param[in]     a_blend           Smooth-minimum operator.
   * @param[in]     a_construction    Preset construction method; every BVH::Construction value is supported.
   * @param[in]     a_options         Leaf-size settings, in primitives.
   */
  template <size_t N = sizeof...(Ps), std::enable_if_t<N == 1, int> = 0>
  EBGEOMETRY_HOST
  BVHSmoothUnion(Pool&                           a_pool,
                 const std::vector<First>&       a_primitives,
                 const std::vector<BV>&          a_boundingVolumes,
                 T                               a_smoothLen,
                 Blend                           a_blend,
                 BVH::Construction               a_construction,
                 const BVH::ConstructionOptions& a_options);

  /**
   * @brief Evaluate the smooth union at a point.
   * @details Blends the two smallest primitive values with the smooth-minimum operator.
   * @param[in] a_point 3D query point.
   * @return The blended value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief How many members (primitives) the union holds.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumMembers() const noexcept;

  /**
   * @brief A member's index in the order the constructor was given the primitives (type by type, in the order
   * of the arrays, each array in its own order), from its place in the union's storage: the unbounded members
   * first (getUnboundedPrimitives()), then the BVH's leaves in order (getBVH().getPrimitives()).
   * @param[in] a_position Place in the storage, below getNumMembers().
   * @return The member's index.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getMemberIndex(uint32_t a_position) const noexcept;

  /**
   * @brief How many surfaces the union numbers (see hasSurfaces): one per member, or a container member's own
   * getNumSurfaces().
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSurfaces() const noexcept;

  /**
   * @brief A member's first surface: member i covers [getFirstSurface(i), getFirstSurface(i + 1)).
   * @param[in] a_member Member index, in the constructor's order, at most getNumMembers().
   * @return The surface.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getFirstSurface(uint32_t a_member) const noexcept;

  /**
   * @brief The surface that decides the union's value at a point (see hasSurfaces): the member with the
   * smallest value (the one that dominates the blend), ties going to the lower member index (with several types,
   * to the earlier type), drilled into if it is a container.
   * @param[in] a_point Query point.
   * @return The surface, and the point in its member's frame.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceRef<T>
  closestSurface(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The axis-aligned bounding box enclosing all primitives.
   * @return The BVH's root box merged with the boxes of the unbounded primitives.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BV
  computeBoundingVolume() const noexcept;

  /**
   * @brief Get the packed BVH over the primitives whose bounding box is bounded.
   * @return The BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Root
  getBVH() const noexcept;

  /**
   * @brief Get the leaves of the primitives whose bounding box is unbounded, which every query evaluates.
   * @return Span over those leaves; empty if there are none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PODSpan<const Leaf>
  getUnboundedPrimitives() const noexcept;

  /**
   * @brief Call @p a_f with the primitive a leaf stands for; see BVHUnion::visit().
   * @tparam F Callable on each primitive type.
   * @param[in] a_leaf A leaf of this union.
   * @param[in] a_f    What to call.
   * @return What @p a_f returns.
   */
  template <class F>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline auto
  visit(const Leaf& a_leaf, F&& a_f) const noexcept;

  /**
   * @brief Produce a copy of this smooth union that resolves against @p a_pool; see BVHUnion.
   * @param[in] a_pool Pool to rebase onto: the union's own pool or one in its mirror chain.
   * @return A BVHSmoothUnion resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BVHSmoothUnion
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate this smooth union's storage, and that of any pool-resident primitive, into
   * @p a_dstPool.
   * @param[in,out] a_dstPool Pool to reserve the copy from.
   * @return A BVHSmoothUnion with the same structure, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BVHSmoothUnion
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief The smooth union's arrays and parameters, with no location: what an outer union stores
   * when this one is its primitive (see PoolHandle). With several types it also holds the per-type arrays
   * (its base).
   */
  struct Layout : CSGDetail::ArraysOf<Ps...>
  {
    /**
     * @brief BVH over the primitives with bounded boxes.
     */
    typename Root::Layout m_bvh;

    /**
     * @brief The primitives with unbounded boxes, as one leaf.
     */
    typename Root::Layout m_unbounded;

    /**
     * @brief Smoothing length.
     */
    T m_smoothLen;

    /**
     * @brief Smooth-minimum operator.
     */
    Blend m_blend;

    /**
     * @brief The members' table: with one type, each storage place's member index (see getMemberIndex()); then,
     * if a member type is a container, each member's first surface and the total.
     */
    PODVector<uint32_t> m_members;
  };

  /**
   * @brief Rebuild a handle from a stored layout and the location of the pool that holds it; see
   * BVHUnion.
   * @param[in] a_layout   The union's arrays and parameters.
   * @param[in] a_location Where they live.
   */
  EBGEOMETRY_HOST_DEVICE
  BVHSmoothUnion(const Layout& a_layout, const PoolLocation& a_location) noexcept
    : PoolHandle(a_location),
      CSGDetail::ArraysOf<Ps...>(a_layout),
      m_bvh(a_layout.m_bvh),
      m_unbounded(a_layout.m_unbounded),
      m_smoothLen(a_layout.m_smoothLen),
      m_blend(a_layout.m_blend),
      m_members(a_layout.m_members)
  {}

  /**
   * @brief The union's arrays and parameters without its location, for storing it inside an outer
   * union.
   * @return The layout.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Layout
  layout() const noexcept
  {
    Layout layout{};

    static_cast<CSGDetail::ArraysOf<Ps...>&>(layout) = *this;
    layout.m_bvh                                     = m_bvh;
    layout.m_unbounded                               = m_unbounded;
    layout.m_smoothLen                               = m_smoothLen;
    layout.m_blend                                   = m_blend;
    layout.m_members                                 = m_members;

    return layout;
  }

private:
  /**
   * @brief Call @p a_f with the primitive a leaf stands for, resolved against a location the caller read once
   * (visit() with the location given).
   * @tparam F Callable on each primitive type.
   * @param[in] a_leaf     A leaf of this union.
   * @param[in] a_location The union's location.
   * @param[in] a_f        What to call.
   * @return What @p a_f returns.
   */
  template <class F>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline auto
  visitAt(const Leaf& a_leaf, const PoolLocation& a_location, F&& a_f) const noexcept;
  /**
   * @brief Adopt the built parts; used by the constructors.
   * @param[in] a_built     The layout of the BVH, the unbounded primitives and, with several types, the
   * arrays (its smoothing length and blend unset), and where it lives.
   * @param[in] a_smoothLen Smoothing length; a nonpositive one is replaced by the smallest positive
   * value, and the public constructors reject it.
   * @param[in] a_blend     Smooth-minimum operator.
   */
  EBGEOMETRY_HOST
  BVHSmoothUnion(const std::pair<Layout, PoolLocation>& a_built, T a_smoothLen, Blend a_blend) noexcept
    : BVHSmoothUnion(a_built.first, a_built.second)
  {
    m_smoothLen = Math::max(a_smoothLen, Math::Limits<T>::min());
    m_blend     = a_blend;
  }

  /**
   * @brief Packed BVH over the primitives with bounded boxes, as a layout.
   */
  typename Root::Layout m_bvh;

  /**
   * @brief The primitives with unbounded boxes, as a single leaf (or empty), as a layout.
   */
  typename Root::Layout m_unbounded;

  /**
   * @brief Smoothing length.
   */
  T m_smoothLen;

  /**
   * @brief Smooth-minimum operator.
   */
  Blend m_blend;

  /**
   * @brief The members' table (see Layout::m_members).
   */
  PODVector<uint32_t> m_members;
};

/**
 * @brief A smooth union of the given arrays' types with the default blend.
 * @tparam P  First primitive type.
 * @tparam Ps The others.
 * @param[in] a_pool Pool.
 * @param[in] a_smoothLen Smoothing length.
 * @param[in] a_first The first type's primitives.
 * @param[in] a_primitives The other types' primitives.
 */
template <class P, class... Ps, std::enable_if_t<!(CSGDetail::isBox<Ps> || ...), int> = 0>
BVHSmoothUnion(Pool&                            a_pool,
               CSGDetail::PrimitivePrecision<P> a_smoothLen,
               const std::vector<P>&            a_first,
               const std::vector<Ps>&... a_primitives)
  -> BVHSmoothUnion<CSGDetail::PrimitivePrecision<P>,
                    BVH::DefaultBranchingRatio<CSGDetail::PrimitivePrecision<P>>(),
                    SmoothMinOp<CSGDetail::PrimitivePrecision<P>>,
                    P,
                    Ps...>;

/**
 * @brief A smooth union of the given arrays' types with a blend.
 * @tparam B  Blend type.
 * @tparam P  First primitive type.
 * @tparam Ps The others.
 * @param[in] a_pool Pool.
 * @param[in] a_smoothLen Smoothing length.
 * @param[in] a_blend Smooth-minimum operator.
 * @param[in] a_first The first type's primitives.
 * @param[in] a_primitives The other types' primitives.
 */
template <class B, class P, class... Ps, std::enable_if_t<!CSGDetail::isVector<B>, int> = 0>
BVHSmoothUnion(Pool&                            a_pool,
               CSGDetail::PrimitivePrecision<P> a_smoothLen,
               B                                a_blend,
               const std::vector<P>&            a_first,
               const std::vector<Ps>&... a_primitives)
  -> BVHSmoothUnion<CSGDetail::PrimitivePrecision<P>,
                    BVH::DefaultBranchingRatio<CSGDetail::PrimitivePrecision<P>>(),
                    B,
                    P,
                    Ps...>;

/**
 * @brief A smooth union of the given arrays' types with settings and a blend.
 * @tparam B  Blend type.
 * @tparam P  First primitive type.
 * @tparam Ps The others.
 * @param[in] a_pool Pool.
 * @param[in] a_settings Construction method and leaf sizes.
 * @param[in] a_smoothLen Smoothing length.
 * @param[in] a_blend Smooth-minimum operator.
 * @param[in] a_first The first type's primitives.
 * @param[in] a_primitives The other types' primitives.
 */
template <class B, class P, class... Ps, std::enable_if_t<!CSGDetail::isVector<B>, int> = 0>
BVHSmoothUnion(Pool&                            a_pool,
               const BVH::BuildSettings&        a_settings,
               CSGDetail::PrimitivePrecision<P> a_smoothLen,
               B                                a_blend,
               const std::vector<P>&            a_first,
               const std::vector<Ps>&... a_primitives)
  -> BVHSmoothUnion<CSGDetail::PrimitivePrecision<P>,
                    BVH::DefaultBranchingRatio<CSGDetail::PrimitivePrecision<P>>(),
                    B,
                    P,
                    Ps...>;

/**
 * @brief A smooth union of one type with the caller's boxes and the default blend.
 * @tparam P Primitive type.
 * @tparam V Box type.
 * @param[in] a_pool Pool.
 * @param[in] a_first The first type's primitives.
 * @param[in] a_boxes The primitives' boxes.
 * @param[in] a_smoothLen Smoothing length.
 */
template <class P, class V, std::enable_if_t<CSGDetail::isBox<V>, int> = 0>
BVHSmoothUnion(Pool&                            a_pool,
               const std::vector<P>&            a_first,
               const std::vector<V>&            a_boxes,
               CSGDetail::PrimitivePrecision<P> a_smoothLen)
  -> BVHSmoothUnion<CSGDetail::PrimitivePrecision<P>,
                    BVH::DefaultBranchingRatio<CSGDetail::PrimitivePrecision<P>>(),
                    SmoothMinOp<CSGDetail::PrimitivePrecision<P>>,
                    P>;

/**
 * @brief A smooth union of one type with the caller's boxes and a blend.
 * @tparam P    Primitive type.
 * @tparam V    Box type.
 * @tparam B    Blend type.
 * @tparam Rest The construction method and leaf sizes, if given.
 * @param[in] a_pool Pool.
 * @param[in] a_first The first type's primitives.
 * @param[in] a_boxes The primitives' boxes.
 * @param[in] a_smoothLen Smoothing length.
 * @param[in] a_blend Smooth-minimum operator.
 * @param[in] a_rest The leaf sizes (and for a smooth union, the construction method), if given.
 */
template <class P, class V, class B, class... Rest, std::enable_if_t<CSGDetail::isBox<V>, int> = 0>
BVHSmoothUnion(Pool&                            a_pool,
               const std::vector<P>&            a_first,
               const std::vector<V>&            a_boxes,
               CSGDetail::PrimitivePrecision<P> a_smoothLen,
               B                                a_blend,
               const Rest&... a_rest) -> BVHSmoothUnion<CSGDetail::PrimitivePrecision<P>,
                                                        BVH::DefaultBranchingRatio<CSGDetail::PrimitivePrecision<P>>(),
                                                        B,
                                                        P>;

} // namespace EBGeometry

#include "EBGeometry_BVHUnionImplem.hpp"

// The ray forms, through the fused operations: firstHit() is a template, and a file that included this header
// without them would march a BVH union that another file traces exactly, two definitions of one function.
// Included last, after the classes they specialize for, since those headers include this one.
#include "EBGeometry_Fused.hpp"

#endif
