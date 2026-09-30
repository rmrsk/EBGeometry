// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_CSG.hpp
 * @brief  Declaration of CSG operations for implicit functions.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_CSG_HPP
#define EBGEOMETRY_CSG_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Constructs an implicit function whose interior is the union of the interiors of all input functions.
 * @details The returned object evaluates to the minimum over all input functions at any query point.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunctions Input implicit functions.
 * @return Shared pointer to a UnionIF<T> wrapping all input functions.
 */
template <class T, class P = ImplicitFunction<T>>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Union(const std::vector<std::shared_ptr<P>>& a_implicitFunctions);

/**
 * @brief Constructs an implicit function whose interior is the union of the interiors of A and B.
 * @tparam T  Floating-point precision.
 * @tparam P1 Type of A; must derive from ImplicitFunction<T>.
 * @tparam P2 Type of B; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunctionA First implicit function.
 * @param[in] a_implicitFunctionB Second implicit function.
 * @return Shared pointer to a UnionIF<T> containing both functions.
 */
template <class T, class P1, class P2>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Union(const std::shared_ptr<P1>& a_implicitFunctionA, const std::shared_ptr<P2>& a_implicitFunctionB);

/**
 * @brief Constructs an implicit function whose interior is a smoothly blended union of all input function interiors.
 * @details Uses SmoothMin to blend the two closest values; the blend region width is a_smooth.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunctions Input implicit functions.
 * @param[in] a_smooth Smoothing length (must be > 0).
 * @return Shared pointer to a SmoothUnionIF<T> wrapping all input functions.
 */
template <class T, class P = ImplicitFunction<T>>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth);

/**
 * @brief Constructs an implicit function whose interior is a smoothly blended union of the interiors of A and B.
 * @tparam T  Floating-point precision.
 * @tparam P1 Type of A; must derive from ImplicitFunction<T>.
 * @tparam P2 Type of B; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunctionA First implicit function.
 * @param[in] a_implicitFunctionB Second implicit function.
 * @param[in] a_smooth Smoothing length (must be > 0).
 * @return Shared pointer to a SmoothUnionIF<T> containing both functions.
 */
template <class T, class P1, class P2>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::shared_ptr<P1>& a_implicitFunctionA,
            const std::shared_ptr<P2>& a_implicitFunctionB,
            const T                    a_smooth);

/**
 * @brief Constructs an implicit function whose interior is the intersection of the interiors of all input functions.
 * @details The returned object evaluates to the maximum over all input functions at any query point.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunctions Input implicit functions.
 * @return Shared pointer to an IntersectionIF<T> wrapping all input functions.
 */
template <class T, class P>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions);

/**
 * @brief Constructs an implicit function whose interior is the intersection of the interiors of A and B.
 * @tparam T  Floating-point precision.
 * @tparam P1 Type of A; must derive from ImplicitFunction<T>.
 * @tparam P2 Type of B; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunctionA First implicit function.
 * @param[in] a_implicitFunctionB Second implicit function.
 * @return Shared pointer to an IntersectionIF<T> containing both functions.
 */
template <class T, class P1, class P2>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::shared_ptr<P1>& a_implicitFunctionA, const std::shared_ptr<P2>& a_implicitFunctionB);

/**
 * @brief Constructs an implicit function whose interior is a smoothly blended intersection of all input function interiors.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunctions Input implicit functions.
 * @param[in] a_smooth Smoothing length (must be > 0).
 * @return Shared pointer to a SmoothIntersectionIF<T> wrapping all input functions.
 */
template <class T, class P>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth);

/**
 * @brief Constructs an implicit function whose interior is a smoothly blended intersection of the interiors of A and B.
 * @tparam T  Floating-point precision.
 * @tparam P1 Type of A; must derive from ImplicitFunction<T>.
 * @tparam P2 Type of B; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunctionA First implicit function.
 * @param[in] a_implicitFunctionB Second implicit function.
 * @param[in] a_smooth Smoothing length (must be > 0).
 * @return Shared pointer to a SmoothIntersectionIF<T> containing both functions.
 */
template <class T, class P1, class P2>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::shared_ptr<P1>& a_implicitFunctionA,
                   const std::shared_ptr<P2>& a_implicitFunctionB,
                   const T                    a_smooth);

/**
 * @brief Constructs an implicit function whose interior is the set difference A \ B.
 * @details A point is inside the result if and only if it is inside A and outside B.
 * @tparam T  Floating-point precision.
 * @tparam P1 Type of the minuend A; must derive from ImplicitFunction<T>.
 * @tparam P2 Type of the subtrahend B; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunctionA Minuend implicit function.
 * @param[in] a_implicitFunctionB Subtrahend implicit function.
 * @return Shared pointer to a DifferenceIF<T> representing A \ B.
 */
template <class T, class P1 = ImplicitFunction<T>, class P2 = ImplicitFunction<T>>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
Difference(const std::shared_ptr<P1>& a_implicitFunctionA, const std::shared_ptr<P2>& a_implicitFunctionB);

/**
 * @brief Constructs an implicit function representing a smoothly blended set difference A \ B.
 * @details Implemented as a smooth intersection of A with the complement of B.
 * @tparam T  Floating-point precision.
 * @tparam P1 Type of the minuend A; must derive from ImplicitFunction<T>.
 * @tparam P2 Type of the subtrahend B; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunctionA Minuend implicit function.
 * @param[in] a_implicitFunctionB Subtrahend implicit function.
 * @param[in] a_smoothLen         Smoothing length (must be > 0).
 * @return Shared pointer to a SmoothDifferenceIF<T> representing the smooth A \ B.
 */
template <class T, class P1 = ImplicitFunction<T>, class P2 = ImplicitFunction<T>>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
SmoothDifference(const std::shared_ptr<P1>& a_implicitFunctionA,
                 const std::shared_ptr<P2>& a_implicitFunctionB,
                 const T                    a_smoothLen);

/**
 * @brief Constructs a periodically tiled implicit function with finite extent.
 * @details The query point is folded into the nearest cell before evaluating the base function.
 * Along each axis the tiles are those at offsets -a_repeatLo, ..., 0, ..., a_repeatHi periods from
 * the base; outside this range the nearest tile is evaluated. The counts are whole numbers of
 * tiles: fractional values are rounded, and they must not be negative.
 *
 * The result is a distance bound only if the base shape fits inside its own cell,
 * [-a_period/2, a_period/2] on each axis. A shape that reaches into a neighbouring cell is cut off
 * where the cells meet, and the value there can overestimate the true distance.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunction Base implicit function to tile.
 * @param[in] a_period           Tile period in each coordinate direction (all components must be > 0).
 * @param[in] a_repeatLo         Number of tiles in the decreasing direction of each axis (>= 0).
 * @param[in] a_repeatHi         Number of tiles in the increasing direction of each axis (>= 0).
 * @return Shared pointer to a FiniteRepetitionIF<T>.
 */
template <class T, class P = ImplicitFunction<T>>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
FiniteRepetition(const std::shared_ptr<P>& a_implicitFunction,
                 const Vec3T<T>&           a_period,
                 const Vec3T<T>&           a_repeatLo,
                 const Vec3T<T>&           a_repeatHi);

/**
 * @brief Exponential smooth minimum for blending two signed-distance values.
 * @details Evaluates `-s log(exp(-a/s) + exp(-b/s))`, which approximates min(a, b) with exponential
 * weighting; the blend region width scales with s. Useful when a differentiable interface is
 * required. Approaches min(a, b) as s → 0. Computed as `min(a, b) - s log1p(exp(-|a - b| / s))`,
 * which equals the formula above but cannot overflow or underflow however far a and b are from zero.
 * A trivially copyable function object, so it can be stored in a BVHSmoothUnionIF and evaluated on
 * a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct ExpMinOp
{
  static_assert(std::is_floating_point_v<T>, "ExpMinOp requires a floating-point type T");

  /**
   * @brief Evaluate the exponential smooth minimum.
   * @param[in] a First value.
   * @param[in] b Second value.
   * @param[in] s Smoothing length (must be > 0).
   * @return Exponentially blended approximation of min(a, b).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  operator()(const T& a, const T& b, const T& s) const noexcept
  {
    EBGEOMETRY_EXPECT(s > T(0));

    return Math::min(a, b) - s * std::log1p(std::exp(-std::abs(a - b) / s));
  }
};

/**
 * @brief Exponential smooth maximum for blending two signed-distance values.
 * @details Evaluates `s log(exp(a/s) + exp(b/s))`, the counterpart of ExpMinOp for intersections
 * and differences: approximates max(a, b) and approaches it as s → 0. Computed as
 * `max(a, b) + s log1p(exp(-|a - b| / s))`, which cannot overflow. A trivially copyable function
 * object.
 * @tparam T Floating-point precision.
 */
template <class T>
struct ExpMaxOp
{
  static_assert(std::is_floating_point_v<T>, "ExpMaxOp requires a floating-point type T");

  /**
   * @brief Evaluate the exponential smooth maximum.
   * @param[in] a First value.
   * @param[in] b Second value.
   * @param[in] s Smoothing length (must be > 0).
   * @return Exponentially blended approximation of max(a, b).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  operator()(const T& a, const T& b, const T& s) const noexcept
  {
    EBGEOMETRY_EXPECT(s > T(0));

    return Math::max(a, b) + s * std::log1p(std::exp(-std::abs(a - b) / s));
  }
};

/**
 * @brief Quadratic polynomial smooth minimum for blending two signed-distance values.
 * @details Approximates min(a, b) within the overlap region |a - b| < s; coincides exactly with
 * min(a, b) outside that region. Cheaper to evaluate than ExpMinOp and the default choice for CSG
 * unions. A trivially copyable function object, so it can be stored in a BVHSmoothUnionIF and
 * evaluated on a device.
 * @tparam T Floating-point precision.
 */
template <class T>
struct SmoothMinOp
{
  static_assert(std::is_floating_point_v<T>, "SmoothMinOp requires a floating-point type T");

  /**
   * @brief Evaluate the polynomial smooth minimum.
   * @param[in] a First value.
   * @param[in] b Second value.
   * @param[in] s Smoothing length (must be > 0). Controls the blend region width.
   * @return Polynomial-blended approximation of min(a, b).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  operator()(const T& a, const T& b, const T& s) const noexcept
  {
    EBGEOMETRY_EXPECT(s > T(0));

    const T h = Math::max(s - std::abs(a - b), T(0)) / s;

    return Math::min(a, b) - T(0.25) * h * h * s;
  }
};

/**
 * @brief Quadratic polynomial smooth maximum for blending two signed-distance values.
 * @details Approximates max(a, b) within the overlap region |a - b| < s; coincides exactly with
 * max(a, b) outside that region. Symmetric counterpart to SmoothMinOp; used for CSG intersections
 * and differences.
 * @tparam T Floating-point precision.
 */
template <class T>
struct SmoothMaxOp
{
  static_assert(std::is_floating_point_v<T>, "SmoothMaxOp requires a floating-point type T");

  /**
   * @brief Evaluate the polynomial smooth maximum.
   * @param[in] a First value.
   * @param[in] b Second value.
   * @param[in] s Smoothing length (must be > 0). Controls the blend region width.
   * @return Polynomial-blended approximation of max(a, b).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  operator()(const T& a, const T& b, const T& s) const noexcept
  {
    EBGEOMETRY_EXPECT(s > T(0));

    const T h = Math::max(s - std::abs(a - b), T(0)) / s;

    return Math::max(a, b) + T(0.25) * h * h * s;
  }
};

/**
 * @brief The exponential smooth minimum, as a ready-made function object: `ExpMin<T>(a, b, s)`.
 * @details Converts implicitly to the `std::function` the smooth CSG combinators take.
 * @tparam T Floating-point precision.
 */
template <class T>
inline constexpr ExpMinOp<T> ExpMin{};

/**
 * @brief The exponential smooth maximum, as a ready-made function object: `ExpMax<T>(a, b, s)`.
 * @details Converts implicitly to the `std::function` the smooth CSG combinators take.
 * @tparam T Floating-point precision.
 */
template <class T>
inline constexpr ExpMaxOp<T> ExpMax{};

/**
 * @brief The polynomial smooth minimum, as a ready-made function object: `SmoothMin<T>(a, b, s)`.
 * @details Converts implicitly to the `std::function` the smooth CSG combinators take.
 * @tparam T Floating-point precision.
 */
template <class T>
inline constexpr SmoothMinOp<T> SmoothMin{};

/**
 * @brief The polynomial smooth maximum, as a ready-made function object: `SmoothMax<T>(a, b, s)`.
 * @details Converts implicitly to the `std::function` the smooth CSG combinators take.
 * @tparam T Floating-point precision.
 */
template <class T>
inline constexpr SmoothMaxOp<T> SmoothMax{};

/**
 * @brief Implicit function whose interior is the union of all input function interiors.
 * @details A point is inside UnionIF if and only if it is inside at least one stored function.
 * The value at any point is the minimum over all stored functions; a negative value indicates
 * membership in the union.
 * @tparam T Floating-point precision.
 */
template <class T>
class UnionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "UnionIF requires a floating-point type T");

  /**
   * @brief Disallowed, use the full constructor.
   */
  UnionIF() = delete;

  /**
   * @brief Constructs the union of the given implicit functions.
   * @param[in] a_implicitFunctions List of implicit functions (must be non-empty; no null entries).
   */
  UnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions);

  /**
   * @brief Destructor.
   */
  ~UnionIF() override = default;

  /**
   * @brief Evaluates the signed distance at a_point.
   * @details Returns the minimum over all stored functions. Negative when inside the union.
   * @param[in] a_point 3D query point.
   * @return Minimum signed distance value among all stored functions.
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Stored implicit functions.
   */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;
};

/**
 * @brief Implicit function whose interior is a smoothly blended union of all input function interiors.
 * @details Replaces the sharp min of UnionIF with a smooth-minimum operator, yielding a C1 (or better)
 * interface where interiors overlap. Only the two closest values are blended; the blend region width
 * is controlled by m_smoothLen.
 * @tparam T Floating-point precision.
 */
template <class T>
class SmoothUnionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "SmoothUnionIF requires a floating-point type T");

  /**
   * @brief Disallowed, use the full constructor.
   */
  SmoothUnionIF() = delete;

  /**
   * @brief Constructs the smooth union of the given implicit functions.
   * @param[in] a_implicitFunctions List of implicit functions (must be non-empty; no null entries).
   * @param[in] a_smoothLen         Smoothing length (must be > 0).
   * @param[in] a_smoothMin         Smooth-minimum operator; defaults to SmoothMin<T>.
   */
  SmoothUnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&   a_implicitFunctions,
                const T                                                    a_smoothLen,
                const std::function<T(const T& a, const T& b, const T& s)> a_smoothMin = SmoothMin<T>);

  /**
   * @brief Destructor.
   */
  ~SmoothUnionIF() override = default;

  /**
   * @brief Evaluates the smoothly blended signed distance at a_point.
   * @details Finds the two closest function values and applies the stored smooth-minimum operator.
   * Negative when inside the smooth union.
   * @param[in] a_point 3D query point.
   * @return Smooth minimum signed distance value.
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Stored implicit functions.
   */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;

  /**
   * @brief Smoothing length.
   */
  T m_smoothLen;

  /**
   * @brief Smooth-minimum operator.
   */
  std::function<T(const T&, const T&, const T&)> m_smoothMin;
};

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
   * @brief Build the union.
   * @param[in,out] a_pool            Pool the BVH is reserved from; pool-resident primitives must live
   * in it too. Must outlive this object and every copy of it.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_build           BVH construction strategy.
   */
  EBGEOMETRY_HOST
  BVHUnionIF(Pool&                  a_pool,
             const std::vector<P>&  a_primitives,
             const std::vector<BV>& a_boundingVolumes,
             BVH::Build             a_build = BVH::Build::SAH);

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
   * @param[in] a_pool Pool to rebase onto; must be a mirror of this union's own pool.
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
   * @brief Build the smooth union.
   * @param[in,out] a_pool            Pool the BVH is reserved from; see BVHUnionIF.
   * @param[in]     a_primitives      Primitives (must be non-empty).
   * @param[in]     a_boundingVolumes Bounding box of each primitive (same length as a_primitives).
   * @param[in]     a_smoothLen       Smoothing length (must be > 0).
   * @param[in]     a_blend           Smooth-minimum operator.
   * @param[in]     a_build           BVH construction strategy.
   */
  EBGEOMETRY_HOST
  BVHSmoothUnionIF(Pool&                  a_pool,
                   const std::vector<P>&  a_primitives,
                   const std::vector<BV>& a_boundingVolumes,
                   T                      a_smoothLen,
                   Blend                  a_blend = Blend{},
                   BVH::Build             a_build = BVH::Build::SAH);

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
   * @param[in] a_pool Pool to rebase onto; must be a mirror of this union's own pool.
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

/**
 * @brief Implicit function whose interior is the intersection of all input function interiors.
 * @details A point is inside IntersectionIF if and only if it is inside every stored function.
 * The value at any point is the maximum over all stored functions; a negative value indicates
 * membership in the intersection.
 * @tparam T Floating-point precision.
 */
template <class T>
class IntersectionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "IntersectionIF requires a floating-point type T");

  /**
   * @brief Disallowed, use the full constructor.
   */
  IntersectionIF() = delete;

  /**
   * @brief Constructs the intersection of the given implicit functions.
   * @param[in] a_implicitFunctions List of implicit functions (must be non-empty; no null entries).
   */
  IntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept;

  /**
   * @brief Destructor.
   */
  ~IntersectionIF() override = default;

  /**
   * @brief Evaluates the signed distance at a_point.
   * @details Returns the maximum over all stored functions. Negative when inside the intersection.
   * @param[in] a_point 3D query point.
   * @return Maximum signed distance value among all stored functions.
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Stored implicit functions.
   */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;
};

/**
 * @brief Implicit function whose interior is a smoothly blended intersection of all input function interiors.
 * @details Replaces the sharp max of IntersectionIF with a smooth-maximum operator, yielding a C1 (or better)
 * interface at intersection boundaries. Only the two largest values are blended; the blend region width
 * is controlled by m_smoothLen.
 * @tparam T Floating-point precision.
 */
template <class T>
class SmoothIntersectionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "SmoothIntersectionIF requires a floating-point type T");

  /**
   * @brief Disallowed, use the full constructor.
   */
  SmoothIntersectionIF() = delete;

  /**
   * @brief Constructs the smooth intersection of two implicit functions.
   * @param[in] a_implicitFunctionA First implicit function (must not be null).
   * @param[in] a_implicitFunctionB Second implicit function (must not be null).
   * @param[in] a_smoothLen         Smoothing length (must be > 0).
   * @param[in] a_smoothMax         Smooth-maximum operator; defaults to SmoothMax<T>.
   */
  SmoothIntersectionIF(const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
                       const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionB,
                       const T                                                     a_smoothLen,
                       const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax = SmoothMax<T>) noexcept;

  /**
   * @brief Constructs the smooth intersection of a list of implicit functions.
   * @param[in] a_implicitFunctions List of implicit functions (must be non-empty; no null entries).
   * @param[in] a_smoothLen         Smoothing length (must be > 0).
   * @param[in] a_smoothMax         Smooth-maximum operator; defaults to SmoothMax<T>.
   */
  SmoothIntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctions,
                       const T                                                     a_smoothLen,
                       const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax = SmoothMax<T>) noexcept;

  /**
   * @brief Destructor.
   */
  ~SmoothIntersectionIF() override = default;

  /**
   * @brief Evaluates the smoothly blended signed distance at a_point.
   * @details Finds the two largest function values and applies the stored smooth-maximum operator.
   * Negative when inside the smooth intersection.
   * @param[in] a_point 3D query point.
   * @return Smooth maximum signed distance value.
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Stored implicit functions.
   */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;

  /**
   * @brief Smoothing length.
   */
  T m_smoothLen;

  /**
   * @brief Smooth-maximum operator.
   */
  std::function<T(const T& a, const T& b, const T& s)> m_smoothMax;
};

/**
 * @brief Implicit function whose interior is the set difference A \ B.
 * @details A point is inside DifferenceIF if and only if it is inside A and outside B.
 * The value function returns max(A(x), -B(x)).
 * @tparam T Floating-point precision.
 */
template <class T>
class DifferenceIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "DifferenceIF requires a floating-point type T");

  /**
   * @brief Disallowed, use the full constructor.
   */
  DifferenceIF() = delete;

  /**
   * @brief Constructs A \ B from two implicit functions.
   * @param[in] a_implicitFunctionA Minuend (must not be null).
   * @param[in] a_implicitFunctionB Subtrahend (must not be null).
   */
  DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionA,
               const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionB) noexcept;

  /**
   * @brief Constructs A \ union(Bs) from A and a list of subtrahends.
   * @param[in] a_implicitFunctionA  Minuend (must not be null).
   * @param[in] a_implicitFunctionsB Subtrahends (must be non-empty; no null entries).
   */
  DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>&              a_implicitFunctionA,
               const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctionsB) noexcept;

  /**
   * @brief Destructor.
   */
  ~DifferenceIF() override = default;

  /**
   * @brief Evaluates the signed distance at a_point.
   * @details Returns max(A(x), -B(x)). Negative when inside A and outside B.
   * @param[in] a_point 3D query point.
   * @return Signed distance representing A \ B.
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Minuend implicit function.
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunctionA;

  /**
   * @brief Subtrahend implicit function (may wrap a union of multiple subtrahends).
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunctionB;
};

/**
 * @brief Implicit function representing a smoothly blended set difference A \ B.
 * @details Implemented as a smooth intersection of A with the complement of B. A point is
 * near-inside if it is inside A and outside (or near-outside) B; the interface is smoothed
 * over a region of width m_smoothLen.
 * @tparam T Floating-point precision.
 */
template <class T>
class SmoothDifferenceIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "SmoothDifferenceIF requires a floating-point type T");

  /**
   * @brief Disallowed, use the full constructor.
   */
  SmoothDifferenceIF() = delete;

  /**
   * @brief Constructs the smooth difference A \ B from two implicit functions.
   * @param[in] a_implicitFunctionA Minuend (must not be null).
   * @param[in] a_implicitFunctionB Subtrahend (must not be null).
   * @param[in] a_smoothLen         Smoothing length (must be > 0).
   * @param[in] a_smoothMax         Smooth-maximum operator; defaults to SmoothMax<T>.
   */
  SmoothDifferenceIF(const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
                     const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionB,
                     const T                                                     a_smoothLen,
                     const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax = SmoothMax<T>) noexcept;

  /**
   * @brief Constructs the smooth difference A \ union(Bs) from A and a list of subtrahends.
   * @param[in] a_implicitFunctionA  Minuend (must not be null).
   * @param[in] a_implicitFunctionsB Subtrahends (must be non-empty; no null entries).
   * @param[in] a_smoothLen          Smoothing length (must be > 0).
   * @param[in] a_smoothMax          Smooth-maximum operator; defaults to SmoothMax<T>.
   */
  SmoothDifferenceIF(const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
                     const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctionsB,
                     const T                                                     a_smoothLen,
                     const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax = SmoothMax<T>) noexcept;

  /**
   * @brief Destructor.
   */
  ~SmoothDifferenceIF() override = default;

  /**
   * @brief Evaluates the smoothly blended signed distance at a_point.
   * @details Delegates to the internal SmoothIntersectionIF of A with the complement of B.
   * @param[in] a_point 3D query point.
   * @return Smooth signed distance representing A \ B.
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Internal smooth intersection implementing smooth(A ∩ complement(B)).
   */
  std::shared_ptr<SmoothIntersectionIF<T>> m_smoothIntersectionIF;
};

/**
 * @brief Implicit function that tiles a base function periodically within a finite repetition count.
 * @details The query point is folded into the nearest tile before evaluating the base function.
 * Outside the repetition range, the nearest boundary tile is evaluated.
 * @tparam T Floating-point precision.
 */
template <class T>
class FiniteRepetitionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "FiniteRepetitionIF requires a floating-point type T");

  /**
   * @brief Disallowed, use the full constructor.
   */
  FiniteRepetitionIF() = delete;

  /**
   * @brief Constructs the periodically tiled implicit function.
   * @param[in] a_implicitFunction Base function to tile (must not be null).
   * @param[in] a_period           Tile period per coordinate direction (all components must be > 0).
   * @param[in] a_repeatLo         Number of tiles in the decreasing direction of each axis (>= 0;
   * rounded to a whole number).
   * @param[in] a_repeatHi         Number of tiles in the increasing direction of each axis (>= 0;
   * rounded to a whole number).
   */
  FiniteRepetitionIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                     const Vec3T<T>&                             a_period,
                     const Vec3T<T>&                             a_repeatLo,
                     const Vec3T<T>&                             a_repeatHi) noexcept;

  /**
   * @brief Destructor.
   */
  ~FiniteRepetitionIF() override = default;

  /**
   * @brief Evaluates the signed distance at a_point by folding into the nearest tile.
   * @param[in] a_point 3D query point.
   * @return Signed distance from the folded point to the base implicit function.
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
   * @brief Tile period in each coordinate direction.
   */
  Vec3T<T> m_period;

  /**
   * @brief Repetition count for increasing coordinate directions.
   */
  Vec3T<T> m_repeatHi;

  /**
   * @brief Repetition count for decreasing coordinate directions.
   */
  Vec3T<T> m_repeatLo;

  /**
   * @brief Base implicit function to tile.
   */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction;
};

} // namespace EBGeometry

#include "EBGeometry_CSGImplem.hpp"

#endif
