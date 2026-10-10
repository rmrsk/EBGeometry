// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHUnion.hpp
 * @brief  BVH-accelerated unions of many primitives of one type: BVHUnion and BVHSmoothUnion.
 * @details Device-callable value types. They need the BVH builders and the blend operators, but
 * not the shared_ptr-based CSG layer in EBGeometry_CSG.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHUNION_HPP
#define EBGEOMETRY_BVHUNION_HPP

// Std includes
#include <cstddef>
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
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Internal helpers shared by BVHUnion and BVHSmoothUnion.
 */
namespace CSGDetail {

/**
 * @brief Detects a primitive that lives in a Pool and must be relocated before it is evaluated.
 * @details True for any P with a `relocatedTo(const PoolLocation&)` member -- the mesh distance
 * functions and the BVH unions themselves -- and false for a self-contained value type such as an
 * analytic shape. See PoolLocation for why such a primitive cannot be rebased in place.
 * @tparam P Primitive type.
 */
template <class P, class = void>
struct IsPoolResident : std::false_type
{
};

/**
 * @brief Detects a primitive that lives in a Pool (positive case).
 * @tparam P Primitive type.
 */
template <class P>
struct IsPoolResident<P,
                      std::void_t<decltype(std::declval<const P&>().relocatedTo(std::declval<const PoolLocation&>()))>>
  : std::true_type
{
};

/**
 * @brief Evaluate one primitive of a BVH union at a point.
 * @details A pool-resident primitive is first relocated to the union's own location, so it resolves
 * against the same pool the union does -- the host pool, a host mirror, or a device mirror. Any
 * other primitive is evaluated as stored.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @param[in] a_primitive Primitive as stored in the union's BVH.
 * @param[in] a_point     Query point.
 * @param[in] a_location  The union's pool location.
 * @return Signed distance from a_point to the primitive.
 */
template <class T, class P>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
signedDistance(const P& a_primitive, const Vec3T<T>& a_point, [[maybe_unused]] const PoolLocation& a_location) noexcept
{
  if constexpr (IsPoolResident<P>::value) {
    return a_primitive.relocatedTo(a_location).signedDistance(a_point);
  }
  else {
    return a_primitive.signedDistance(a_point);
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
  BVH::PackedBVH<T, P, K> bvh;

  /**
   * @brief The primitives with unbounded boxes, as one leaf; empty if there are none.
   */
  BVH::PackedBVH<T, P, K> unbounded;
};

/**
 * @brief The leaf-size settings BVHUnion and BVHSmoothUnion use without options.
 * @details Internal helper. Reproduces the trees the unions built before the settings existed: K-1
 * primitives per leaf for the top-down methods (a node with fewer than K becomes a leaf), a target
 * of K for the space-filling curves, and the default ClusterSpec.
 * @tparam K BVH branching factor.
 * @return The default settings.
 */
template <size_t K>
[[nodiscard]] EBGEOMETRY_HOST
inline BVH::ConstructionOptions
defaultUnionOptions() noexcept
{
  return BVH::ConstructionOptions{K - 1, K, BVH::ClusterSpec{}};
}

} // namespace CSGDetail

/**
 * @brief BVH-accelerated union of many primitives of one type.
 * @details A PackedBVH over the primitives, stored by value in a Pool. At query time the BVH culls
 * primitives whose bounding volume lies further than the current best distance, giving sub-linear
 * cost on large scenes. Interior semantics are those of UnionIF: the value is the minimum over all
 * primitives, negative inside the union.
 *
 * This is a plain, trivially copyable value type with no virtual functions, so it is not an
 * ImplicitFunction. signedDistance() is callable on the host and on a device: mirror the pool, call
 * rebasedView(), and pass the result to a kernel. Because every primitive has the same type P, no
 * runtime dispatch is needed; a union of primitives of different types waits for the tape.
 *
 * P can be any trivially copyable type with an EBGEOMETRY_HOST_DEVICE
 * `signedDistance(const Vec3T<T>&)`: an analytic shape, a mesh distance function, or another BVH
 * union. Pruning needs only that each bounding volume encloses its primitive's object (where the
 * primitive's value is not positive): a primitive skipped because its box is further away than the
 * best value so far is positive at the query point, so skipping it never changes the union's sign.
 * Its value is a distance bound if the primitives' values are, and only a sign if they are
 * DistanceQuality::NotADistance (see distanceQuality).
 *
 * A primitive whose bounding box is unbounded in some direction (it reaches plus or minus
 * Math::Limits<T>::max(), as an InfiniteCylinderSDF's does along its axis) is kept out of the BVH:
 * such boxes would make every ancestor's box unbounded and the build heuristics meaningless. Those
 * primitives are evaluated at every query instead, before the traversal, so their values can prune
 * it. A union of many bounded primitives and a few unbounded ones costs the BVH query plus one
 * evaluation per unbounded primitive.
 *
 * A pool-resident primitive (a mesh distance function or a union) must have been built in
 * the same Pool as this union, and is relocated to the union's location as it is evaluated (see
 * PoolLocation). The constructor checks this, and that there is one bounding volume per primitive,
 * in every build, and aborts with a message if either fails.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 */
template <class T, class P, size_t K>
class BVHUnion
{
public:
  static_assert(std::is_floating_point_v<T>, "BVHUnion requires a floating-point type T");
  static_assert(std::is_trivially_copyable_v<P>, "BVHUnion requires a trivially copyable primitive type");
  static_assert(K > 1, "BVHUnion BVH branching factor K must be at least 2");

  /**
   * @brief How far signedDistance() can be trusted as a distance: a bound, or only a sign if the
   * primitives' values are not distances. With exact primitives the union is the exact minimum,
   * which is exact outside but underestimates inside overlapping primitives.
   */
  static constexpr DistanceQuality distanceQuality =
    distanceQualityOf<P> == DistanceQuality::NotADistance ? DistanceQuality::NotADistance : DistanceQuality::Bound;

  /**
   * @brief Alias for the packed BVH type.
   */
  using Root = BVH::PackedBVH<T, P, K>;

  /**
   * @brief Alias for the bounding volume type.
   */
  using BV = BoundingVolumes::AABBT<T>;

  /**
   * @brief Disallowed, use the full constructor.
   */
  BVHUnion() = delete;

  /**
   * @brief The leaf-size settings the constructor without options uses.
   * @details K-1 primitives per leaf for the top-down methods (a node with fewer than K primitives is
   * a leaf), a target of K for the space-filling curves, and the default ClusterSpec for ClusterSAH.
   * Start from these to change one method's setting; see BVH::ConstructionOptions.
   * @return The default settings.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static BVH::ConstructionOptions
  defaultConstructionOptions() noexcept
  {
    return CSGDetail::defaultUnionOptions<K>();
  }

  /**
   * @brief Build the union.
   * @param[in,out] a_pool            Pool the BVH is reserved from; pool-resident primitives must live
   * in it too. Must outlive this object and every copy of it.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_construction           Preset construction method; every BVH::Construction value is supported.
   * The leaf sizes are defaultConstructionOptions().
   */
  EBGEOMETRY_HOST
  BVHUnion(Pool&                  a_pool,
           const std::vector<P>&  a_primitives,
           const std::vector<BV>& a_boundingVolumes,
           BVH::Construction      a_construction = BVH::Construction::SAH);

  /**
   * @brief Build the union with leaf-size settings.
   * @details As the constructor without options, which uses defaultConstructionOptions(). The chosen
   * method reads only its own field of @p a_options; see BVH::ConstructionOptions.
   * @param[in,out] a_pool            Pool the BVH is reserved from; see the constructor without options.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_construction    Preset construction method; every BVH::Construction value is supported.
   * @param[in]     a_options         Leaf-size settings, in primitives.
   */
  EBGEOMETRY_HOST
  BVHUnion(Pool&                           a_pool,
           const std::vector<P>&           a_primitives,
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
  inline const Root&
  getBVH() const noexcept;

  /**
   * @brief Get the primitives whose bounding box is unbounded, which every query evaluates.
   * @details The span is a resolved address into pool memory; the lifetime caveat of
   * BVH::PackedBVH::getPrimitives() applies.
   * @return Span over those primitives; empty if there are none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PODSpan<const P>
  getUnboundedPrimitives() const noexcept;

  /**
   * @brief Produce a copy of this union that resolves against @p a_pool.
   * @details Rebases the BVH; see BVH::PackedBVH::rebasedView() for the contract. Pool-resident
   * primitives follow the union at evaluation time; each is still rebased once here, and the result
   * discarded, purely so that the checks rebasedView() makes (a mirror of the right pool, a BVH
   * shallow enough for the device traversal stack) run for them too. This is the one sanctioned
   * crossing to a device.
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
   * @brief A copy of this union resolving against @p a_location, so that a union can itself be the
   * primitive of an outer union. See PoolLocation.
   * @param[in] a_location Location to resolve against.
   * @return A BVHUnion resolving against @p a_location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BVHUnion
  relocatedTo(const PoolLocation& a_location) const noexcept;

  /**
   * @brief Check whether this union's storage was reserved from @p a_pool.
   * @param[in] a_pool Pool to test against.
   * @return True if this union is attached to @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline bool
  isAttachedTo(const Pool& a_pool) const noexcept;

private:
  /**
   * @brief Adopt the built parts; used by the constructors.
   * @param[in] a_parts The BVH and the unbounded primitives.
   */
  EBGEOMETRY_HOST
  explicit BVHUnion(const CSGDetail::UnionParts<T, P, K>& a_parts) noexcept
    : m_bvh(a_parts.bvh), m_unbounded(a_parts.unbounded)
  {}

  /**
   * @brief Adopt an already built BVH and unbounded leaf; used by rebasedView(), deepCopy() and
   * relocatedTo().
   * @param[in] a_bvh       The BVH.
   * @param[in] a_unbounded The unbounded primitives.
   */
  EBGEOMETRY_HOST_DEVICE
  BVHUnion(const Root& a_bvh, const Root& a_unbounded) noexcept : m_bvh(a_bvh), m_unbounded(a_unbounded)
  {}

  /**
   * @brief Packed BVH over the primitives with bounded boxes, held by value.
   */
  Root m_bvh;

  /**
   * @brief The primitives with unbounded boxes, as a single leaf (or empty), held by value.
   */
  Root m_unbounded;
};

/**
 * @brief BVH-accelerated smooth union of many primitives of one type.
 * @details Uses the BVH to locate the two nearest primitives and blends their values with a
 * smooth-minimum operator, yielding C1 (or better) blending across nearby surfaces with
 * sub-linear query cost. Otherwise identical to BVHUnion: a plain, trivially copyable value type
 * whose signedDistance() is callable on the host and on a device.
 * @tparam T     Floating-point precision.
 * @tparam P     Primitive type; see BVHUnion.
 * @tparam K     BVH branching factor.
 * @tparam Blend Smooth-minimum operator: a trivially copyable function object with an
 * EBGEOMETRY_HOST_DEVICE `T operator()(const T& a, const T& b, const T& s) const`, such as
 * SmoothMinOp (the default) or ExpMinOp.
 */
template <class T, class P, size_t K, class Blend = SmoothMinOp<T>>
class BVHSmoothUnion
{
public:
  static_assert(std::is_floating_point_v<T>, "BVHSmoothUnion requires a floating-point type T");
  static_assert(std::is_trivially_copyable_v<P>, "BVHSmoothUnion requires a trivially copyable primitive type");
  static_assert(std::is_trivially_copyable_v<Blend>, "BVHSmoothUnion requires a trivially copyable blend operator");
  static_assert(K > 1, "BVHSmoothUnion BVH branching factor K must be at least 2");

  /**
   * @brief How far signedDistance() can be trusted as a distance: a bound, or only a sign if the
   * primitives' values are not distances. SmoothMinOp and ExpMinOp weight the two primitives'
   * gradients by weights that sum to one, so they do not make the blend steeper than its inputs.
   */
  static constexpr DistanceQuality distanceQuality =
    distanceQualityOf<P> == DistanceQuality::NotADistance ? DistanceQuality::NotADistance : DistanceQuality::Bound;

  /**
   * @brief Alias for the packed BVH type.
   */
  using Root = BVH::PackedBVH<T, P, K>;

  /**
   * @brief Alias for the bounding volume type.
   */
  using BV = BoundingVolumes::AABBT<T>;

  /**
   * @brief Disallowed, use the full constructor.
   */
  BVHSmoothUnion() = delete;

  /**
   * @brief The leaf-size settings the constructor without options uses; the same as BVHUnion's.
   * @return The default settings.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static BVH::ConstructionOptions
  defaultConstructionOptions() noexcept
  {
    return CSGDetail::defaultUnionOptions<K>();
  }

  /**
   * @brief Build the smooth union.
   * @param[in,out] a_pool            Pool the BVH is reserved from; see BVHUnion.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_smoothLen       Smoothing length (must be > 0).
   * @param[in]     a_blend           Smooth-minimum operator.
   * @param[in]     a_construction           Preset construction method; every BVH::Construction value is supported.
   * The leaf sizes are defaultConstructionOptions().
   */
  EBGEOMETRY_HOST
  BVHSmoothUnion(Pool&                  a_pool,
                 const std::vector<P>&  a_primitives,
                 const std::vector<BV>& a_boundingVolumes,
                 T                      a_smoothLen,
                 Blend                  a_blend        = Blend{},
                 BVH::Construction      a_construction = BVH::Construction::SAH);

  /**
   * @brief Build the smooth union with leaf-size settings.
   * @details As the constructor without options, which uses defaultConstructionOptions(). The chosen
   * method reads only its own field of @p a_options; see BVH::ConstructionOptions.
   * @param[in,out] a_pool            Pool the BVH is reserved from; see BVHUnion.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_smoothLen       Smoothing length (must be > 0).
   * @param[in]     a_blend           Smooth-minimum operator.
   * @param[in]     a_construction    Preset construction method; every BVH::Construction value is supported.
   * @param[in]     a_options         Leaf-size settings, in primitives.
   */
  EBGEOMETRY_HOST
  BVHSmoothUnion(Pool&                           a_pool,
                 const std::vector<P>&           a_primitives,
                 const std::vector<BV>&          a_boundingVolumes,
                 T                               a_smoothLen,
                 Blend                           a_blend,
                 BVH::Construction               a_construction,
                 const BVH::ConstructionOptions& a_options);

  /**
   * @brief Evaluate the smooth union at a point.
   * @details Finds the two nearest primitives via the BVH and blends their values. Negative when
   * inside the smooth union.
   * @param[in] a_point 3D query point.
   * @return Smooth minimum of the values of the two closest primitives.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

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
  inline const Root&
  getBVH() const noexcept;

  /**
   * @brief Get the primitives whose bounding box is unbounded, which every query evaluates.
   * @details The span is a resolved address into pool memory; the lifetime caveat of
   * BVH::PackedBVH::getPrimitives() applies.
   * @return Span over those primitives; empty if there are none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PODSpan<const P>
  getUnboundedPrimitives() const noexcept;

  /**
   * @brief Produce a copy of this smooth union that resolves against @p a_pool; see
   * BVHUnion::rebasedView().
   * @param[in] a_pool Pool to rebase onto: the union's own pool or one in its mirror chain.
   * @return A BVHSmoothUnion resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BVHSmoothUnion
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate this smooth union's storage into @p a_dstPool; see BVHUnion::deepCopy().
   * @param[in,out] a_dstPool Pool to reserve the copy from.
   * @return A BVHSmoothUnion with the same structure, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BVHSmoothUnion
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief A copy of this smooth union resolving against @p a_location; see BVHUnion::relocatedTo().
   * @param[in] a_location Location to resolve against.
   * @return A BVHSmoothUnion resolving against @p a_location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BVHSmoothUnion
  relocatedTo(const PoolLocation& a_location) const noexcept;

  /**
   * @brief Check whether this smooth union's storage was reserved from @p a_pool.
   * @param[in] a_pool Pool to test against.
   * @return True if this union is attached to @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline bool
  isAttachedTo(const Pool& a_pool) const noexcept;

private:
  /**
   * @brief Adopt the built parts; used by the constructors.
   * @param[in] a_parts     The BVH and the unbounded primitives.
   * @param[in] a_smoothLen Smoothing length; a nonpositive one is replaced by the smallest positive
   * value, and the public constructors reject it.
   * @param[in] a_blend     Smooth-minimum operator.
   */
  EBGEOMETRY_HOST
  BVHSmoothUnion(const CSGDetail::UnionParts<T, P, K>& a_parts, T a_smoothLen, Blend a_blend) noexcept
    : m_bvh(a_parts.bvh),
      m_unbounded(a_parts.unbounded),
      m_smoothLen(Math::max(a_smoothLen, Math::Limits<T>::min())),
      m_blend(a_blend)
  {}

  /**
   * @brief Adopt an already built BVH and unbounded leaf; used by rebasedView(), deepCopy() and
   * relocatedTo().
   * @param[in] a_bvh       The BVH.
   * @param[in] a_unbounded The unbounded primitives.
   * @param[in] a_smoothLen Smoothing length.
   * @param[in] a_blend     Smooth-minimum operator.
   */
  EBGEOMETRY_HOST_DEVICE
  BVHSmoothUnion(const Root& a_bvh, const Root& a_unbounded, T a_smoothLen, Blend a_blend) noexcept
    : m_bvh(a_bvh), m_unbounded(a_unbounded), m_smoothLen(a_smoothLen), m_blend(a_blend)
  {}

  /**
   * @brief Packed BVH over the primitives with bounded boxes, held by value.
   */
  Root m_bvh;

  /**
   * @brief The primitives with unbounded boxes, as a single leaf (or empty), held by value.
   */
  Root m_unbounded;

  /**
   * @brief Smoothing length.
   */
  T m_smoothLen;

  /**
   * @brief Smooth-minimum operator.
   */
  Blend m_blend;
};

} // namespace EBGeometry

#include "EBGeometry_BVHUnionImplem.hpp"

#endif
