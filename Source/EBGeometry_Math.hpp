// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Math.hpp
 * @brief  Scalar helpers and numeric limits that compile for both host and device.
 * @details std::min, std::max, std::clamp and std::numeric_limits are constexpr host functions. nvcc
 * rejects them in device code unless every translation unit is compiled with
 * --expt-relaxed-constexpr, and a standard library update can make them unusable on a device
 * altogether (libstdc++ 14's std::clamp asserts through a host-only function). The library uses
 * these instead, everywhere under Source/; Scripts/CheckDeviceMath.py enforces that.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_MATH_HPP
#define EBGEOMETRY_MATH_HPP

#include <cfloat>
#include <climits>
#include <cmath>

#include "EBGeometry_GPU.hpp"

namespace EBGeometry {

/**
 * @brief Scalar helpers and numeric limits usable in host and device code.
 */
namespace Math {

/**
 * @brief The smaller of two values.
 * @details Returns @p a_a when the two compare equal, and when either is NaN returns @p a_a, like
 * std::min.
 * @tparam T Type with operator<.
 * @param[in] a_a First value.
 * @param[in] a_b Second value.
 * @return The smaller value.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
constexpr T
min(const T& a_a, const T& a_b) noexcept
{
  return (a_b < a_a) ? a_b : a_a;
}

/**
 * @brief The larger of two values.
 * @details Returns @p a_a when the two compare equal, and when either is NaN returns @p a_a, like
 * std::max.
 * @tparam T Type with operator<.
 * @param[in] a_a First value.
 * @param[in] a_b Second value.
 * @return The larger value.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
constexpr T
max(const T& a_a, const T& a_b) noexcept
{
  return (a_a < a_b) ? a_b : a_a;
}

/**
 * @brief A value limited to [lo, hi].
 * @details The caller must ensure lo <= hi. A NaN value is returned unchanged, like std::clamp.
 * @tparam T Type with operator<.
 * @param[in] a_value Value to limit.
 * @param[in] a_lo    Lower bound.
 * @param[in] a_hi    Upper bound.
 * @return @p a_lo if the value is below it, @p a_hi if it is above it, else the value.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
constexpr T
clamp(const T& a_value, const T& a_lo, const T& a_hi) noexcept
{
  return (a_value < a_lo) ? a_lo : ((a_hi < a_value) ? a_hi : a_value);
}

/**
 * @brief Numeric limits, with the same member functions and meanings as std::numeric_limits.
 * @details Specialized for float, double, and the standard integer types.
 * @tparam T Arithmetic type.
 */
template <class T>
struct Limits;

/**
 * @brief Numeric limits of float.
 */
template <>
struct Limits<float>
{
  /**
   * @brief Largest finite value.
   * @return FLT_MAX.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr float
  max() noexcept
  {
    return FLT_MAX;
  }

  /**
   * @brief Most negative finite value.
   * @return -FLT_MAX.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr float
  lowest() noexcept
  {
    return -FLT_MAX;
  }

  /**
   * @brief Smallest positive normal value.
   * @return FLT_MIN.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr float
  min() noexcept
  {
    return FLT_MIN;
  }

  /**
   * @brief Difference between 1 and the next representable value.
   * @return FLT_EPSILON.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr float
  epsilon() noexcept
  {
    return FLT_EPSILON;
  }

  /**
   * @brief Positive infinity.
   * @return HUGE_VALF.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr float
  infinity() noexcept
  {
    return HUGE_VALF;
  }
};

/**
 * @brief Numeric limits of double.
 */
template <>
struct Limits<double>
{
  /**
   * @brief Largest finite value.
   * @return DBL_MAX.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr double
  max() noexcept
  {
    return DBL_MAX;
  }

  /**
   * @brief Most negative finite value.
   * @return -DBL_MAX.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr double
  lowest() noexcept
  {
    return -DBL_MAX;
  }

  /**
   * @brief Smallest positive normal value.
   * @return DBL_MIN.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr double
  min() noexcept
  {
    return DBL_MIN;
  }

  /**
   * @brief Difference between 1 and the next representable value.
   * @return DBL_EPSILON.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr double
  epsilon() noexcept
  {
    return DBL_EPSILON;
  }

  /**
   * @brief Positive infinity.
   * @return HUGE_VAL.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr double
  infinity() noexcept
  {
    return HUGE_VAL;
  }
};

/**
 * @brief Numeric limits of an integer type, from its climits constants.
 * @tparam T      Integer type.
 * @tparam Lowest Its smallest value.
 * @tparam Max    Its largest value.
 */
template <class T, T Lowest, T Max>
struct IntegerLimits
{
  /**
   * @brief Largest value.
   * @return The type's maximum.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr T
  max() noexcept
  {
    return Max;
  }

  /**
   * @brief Smallest value.
   * @return The type's minimum.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr T
  lowest() noexcept
  {
    return Lowest;
  }

  /**
   * @brief Smallest value (for an integer type, the same as lowest()).
   * @return The type's minimum.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr T
  min() noexcept
  {
    return Lowest;
  }
};

/** @brief Numeric limits of int. */
template <>
struct Limits<int> : IntegerLimits<int, INT_MIN, INT_MAX>
{
};

/** @brief Numeric limits of unsigned int. */
template <>
struct Limits<unsigned int> : IntegerLimits<unsigned int, 0U, UINT_MAX>
{
};

/** @brief Numeric limits of long. */
template <>
struct Limits<long> : IntegerLimits<long, LONG_MIN, LONG_MAX>
{
};

/** @brief Numeric limits of unsigned long. */
template <>
struct Limits<unsigned long> : IntegerLimits<unsigned long, 0UL, ULONG_MAX>
{
};

/** @brief Numeric limits of long long. */
template <>
struct Limits<long long> : IntegerLimits<long long, LLONG_MIN, LLONG_MAX>
{
};

/** @brief Numeric limits of unsigned long long. */
template <>
struct Limits<unsigned long long> : IntegerLimits<unsigned long long, 0ULL, ULLONG_MAX>
{
};

} // namespace Math

} // namespace EBGeometry

#endif
