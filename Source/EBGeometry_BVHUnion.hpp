// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHUnion.hpp
 * @brief  BVH-accelerated unions of many primitives of one type: BVHUnionIF, BVHSmoothUnionIF and
 * their factory functions.
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
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Internal helpers shared by BVHUnionIF and BVHSmoothUnionIF.
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
 * @brief The leaf-size settings BVHUnionIF and BVHSmoothUnionIF use without options.
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
 * union. A pool-resident primitive (a mesh distance function or a union) must have been built in
 * the same Pool as this union, and is relocated to the union's location as it is evaluated (see
 * PoolLocation). The constructor checks this, and that there is one bounding volume per primitive,
 * in every build, and aborts with a message if either fails.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 */
template <class T, class P, size_t K>
class BVHUnionIF
{
public:
  static_assert(std::is_floating_point_v<T>, "BVHUnionIF requires a floating-point type T");
  static_assert(std::is_trivially_copyable_v<P>, "BVHUnionIF requires a trivially copyable primitive type");
  static_assert(K > 1, "BVHUnionIF BVH branching factor K must be at least 2");

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
  BVHUnionIF() = delete;

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
  BVHUnionIF(Pool&                  a_pool,
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
  BVHUnionIF(Pool&                           a_pool,
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
   * @return The root bounding volume of the BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BV
  computeBoundingVolume() const noexcept;

  /**
   * @brief Get the packed BVH over the primitives.
   * @return The BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Root&
  getBVH() const noexcept;

  /**
   * @brief Produce a copy of this union that resolves against @p a_pool.
   * @details Rebases the BVH; see BVH::PackedBVH::rebasedView() for the contract. Pool-resident
   * primitives follow the union at evaluation time; each is still rebased once here, and the result
   * discarded, purely so that the checks rebasedView() makes (a mirror of the right pool, a BVH
   * shallow enough for the device traversal stack) run for them too. This is the one sanctioned
   * crossing to a device.
   * @param[in] a_pool Pool to rebase onto: the union's own pool or one in its mirror chain.
   * @return A BVHUnionIF resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BVHUnionIF
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate this union's storage, and that of any pool-resident primitive, into @p a_dstPool.
   * @param[in,out] a_dstPool Pool to reserve the copy from.
   * @return A BVHUnionIF with the same structure, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BVHUnionIF
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief A copy of this union resolving against @p a_location, so that a union can itself be the
   * primitive of an outer union. See PoolLocation.
   * @param[in] a_location Location to resolve against.
   * @return A BVHUnionIF resolving against @p a_location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BVHUnionIF
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
   * @brief Adopt an already built BVH; used by rebasedView(), deepCopy() and relocatedTo().
   * @param[in] a_bvh The BVH.
   */
  EBGEOMETRY_HOST_DEVICE
  explicit BVHUnionIF(const Root& a_bvh) noexcept : m_bvh(a_bvh)
  {}

  /**
   * @brief Packed BVH over all primitives, held by value.
   */
  Root m_bvh;
};

/**
 * @brief BVH-accelerated smooth union of many primitives of one type.
 * @details Uses the BVH to locate the two nearest primitives and blends their values with a
 * smooth-minimum operator, yielding C1 (or better) blending across nearby surfaces with
 * sub-linear query cost. Otherwise identical to BVHUnionIF: a plain, trivially copyable value type
 * whose signedDistance() is callable on the host and on a device.
 * @tparam T     Floating-point precision.
 * @tparam P     Primitive type; see BVHUnionIF.
 * @tparam K     BVH branching factor.
 * @tparam Blend Smooth-minimum operator: a trivially copyable function object with an
 * EBGEOMETRY_HOST_DEVICE `T operator()(const T& a, const T& b, const T& s) const`, such as
 * SmoothMinOp (the default) or ExpMinOp.
 */
template <class T, class P, size_t K, class Blend = SmoothMinOp<T>>
class BVHSmoothUnionIF
{
public:
  static_assert(std::is_floating_point_v<T>, "BVHSmoothUnionIF requires a floating-point type T");
  static_assert(std::is_trivially_copyable_v<P>, "BVHSmoothUnionIF requires a trivially copyable primitive type");
  static_assert(std::is_trivially_copyable_v<Blend>, "BVHSmoothUnionIF requires a trivially copyable blend operator");
  static_assert(K > 1, "BVHSmoothUnionIF BVH branching factor K must be at least 2");

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
  BVHSmoothUnionIF() = delete;

  /**
   * @brief The leaf-size settings the constructor without options uses; the same as BVHUnionIF's.
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
   * @param[in,out] a_pool            Pool the BVH is reserved from; see BVHUnionIF.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_smoothLen       Smoothing length (must be > 0).
   * @param[in]     a_blend           Smooth-minimum operator.
   * @param[in]     a_construction           Preset construction method; every BVH::Construction value is supported.
   * The leaf sizes are defaultConstructionOptions().
   */
  EBGEOMETRY_HOST
  BVHSmoothUnionIF(Pool&                  a_pool,
                   const std::vector<P>&  a_primitives,
                   const std::vector<BV>& a_boundingVolumes,
                   T                      a_smoothLen,
                   Blend                  a_blend        = Blend{},
                   BVH::Construction      a_construction = BVH::Construction::SAH);

  /**
   * @brief Build the smooth union with leaf-size settings.
   * @details As the constructor without options, which uses defaultConstructionOptions(). The chosen
   * method reads only its own field of @p a_options; see BVH::ConstructionOptions.
   * @param[in,out] a_pool            Pool the BVH is reserved from; see BVHUnionIF.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_smoothLen       Smoothing length (must be > 0).
   * @param[in]     a_blend           Smooth-minimum operator.
   * @param[in]     a_construction    Preset construction method; every BVH::Construction value is supported.
   * @param[in]     a_options         Leaf-size settings, in primitives.
   */
  EBGEOMETRY_HOST
  BVHSmoothUnionIF(Pool&                           a_pool,
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
   * @return The root bounding volume of the BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BV
  computeBoundingVolume() const noexcept;

  /**
   * @brief Get the packed BVH over the primitives.
   * @return The BVH.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Root&
  getBVH() const noexcept;

  /**
   * @brief Produce a copy of this smooth union that resolves against @p a_pool; see
   * BVHUnionIF::rebasedView().
   * @param[in] a_pool Pool to rebase onto: the union's own pool or one in its mirror chain.
   * @return A BVHSmoothUnionIF resolving against @p a_pool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BVHSmoothUnionIF
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Duplicate this smooth union's storage into @p a_dstPool; see BVHUnionIF::deepCopy().
   * @param[in,out] a_dstPool Pool to reserve the copy from.
   * @return A BVHSmoothUnionIF with the same structure, backed by @p a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BVHSmoothUnionIF
  deepCopy(Pool& a_dstPool) const;

  /**
   * @brief A copy of this smooth union resolving against @p a_location; see BVHUnionIF::relocatedTo().
   * @param[in] a_location Location to resolve against.
   * @return A BVHSmoothUnionIF resolving against @p a_location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BVHSmoothUnionIF
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
   * @brief Adopt an already built BVH; used by rebasedView(), deepCopy() and relocatedTo().
   * @param[in] a_bvh       The BVH.
   * @param[in] a_smoothLen Smoothing length.
   * @param[in] a_blend     Smooth-minimum operator.
   */
  EBGEOMETRY_HOST_DEVICE
  BVHSmoothUnionIF(const Root& a_bvh, T a_smoothLen, Blend a_blend) noexcept
    : m_bvh(a_bvh), m_smoothLen(a_smoothLen), m_blend(a_blend)
  {}

  /**
   * @brief Packed BVH over all primitives, held by value.
   */
  Root m_bvh;

  /**
   * @brief Smoothing length.
   */
  T m_smoothLen;

  /**
   * @brief Smooth-minimum operator.
   */
  Blend m_blend;
};

/**
 * @brief Build a BVH-accelerated union of many primitives of one type.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type; see BVHUnionIF.
 * @tparam K BVH branching factor.
 * @param[in,out] a_pool            Pool the BVH is reserved from; see BVHUnionIF.
 * @param[in]     a_primitives      Primitives (must be non-empty).
 * @param[in]     a_boundingVolumes Bounding box of each primitive.
 * @return The union, by value.
 */
template <class T, class P, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
BVHUnionIF<T, P, K>
BVHUnion(Pool&                                         a_pool,
         const std::vector<P>&                         a_primitives,
         const std::vector<BoundingVolumes::AABBT<T>>& a_boundingVolumes);

/**
 * @brief Build a BVH-accelerated smooth union of many primitives of one type, blended with
 * SmoothMinOp.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type; see BVHUnionIF.
 * @tparam K BVH branching factor.
 * @param[in,out] a_pool            Pool the BVH is reserved from; see BVHUnionIF.
 * @param[in]     a_primitives      Primitives (must be non-empty).
 * @param[in]     a_boundingVolumes Bounding box of each primitive.
 * @param[in]     a_smoothLen       Smoothing length (must be > 0).
 * @return The smooth union, by value.
 */
template <class T, class P, size_t K>
[[nodiscard]] EBGEOMETRY_HOST
BVHSmoothUnionIF<T, P, K>
BVHSmoothUnion(Pool&                                         a_pool,
               const std::vector<P>&                         a_primitives,
               const std::vector<BoundingVolumes::AABBT<T>>& a_boundingVolumes,
               T                                             a_smoothLen);

} // namespace EBGeometry

#include "EBGeometry_BVHUnionImplem.hpp"

#endif
