// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Blend.hpp
 * @brief  The blend operators that smooth unions, intersections and differences use: ExpMinOp,
 * ExpMaxOp, SmoothMinOp, SmoothMaxOp and their constexpr instances.
 * @details Device-callable value types with no other dependency on the CSG layer.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BLEND_HPP
#define EBGEOMETRY_BLEND_HPP

// Std includes
#include <cmath>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"

namespace EBGeometry {

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

} // namespace EBGeometry

#endif
