// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_CSGOps.hpp
 * @brief  The arithmetic of each CSG operation, shared by the Tape's evaluator and the fused operations.
 * @details Internal helpers; not part of the public API. Each function computes one operation's value
 * or point the way both the Tape (EBGeometry_Tape.hpp) and the fused operations (EBGeometry_Fused.hpp)
 * do, in the same order of floating-point operations, so that the two give the same values (to
 * rounding: a compiler may fuse a multiply and an add into one instruction in one and not the other). The
 * functions that evaluate an operand several times (blur, mollification, repetition) take it as a
 * callable from a point to a value. Everything that runs per evaluation is device-callable; the
 * functions that set up an operation's parameters run on the host.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_CSGOPS_HPP
#define EBGEOMETRY_CSGOPS_HPP

// Std includes
#include <cmath>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_Affine.hpp"
#include "EBGeometry_Blend.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief The arithmetic of each CSG operation, shared by the Tape and the fused operations.
 * @details Internal helpers; not part of the public API.
 */
namespace CSGOps {

/**
 * @brief A value with the deciding leaf it comes from: the leaf whose surface the composed value is
 * taken from there, and the point that leaf was evaluated at (in its own frame).
 * @details The operations below work on a plain value T and on a Tagged<T> alike. On a Tagged<T> they
 * compute the value as on a T and carry the tag of the operand the value comes from (see
 * Tape::closestPrimitive() and Fused::Node::closestPrimitive()).
 * @tparam T Floating-point precision.
 */
template <class T>
struct Tagged
{
  /**
   * @brief The value.
   */
  T m_value;

  /**
   * @brief The deciding leaf, as the structure numbers it (NoTag for none).
   */
  uint32_t m_tag;

  /**
   * @brief The point the leaf was evaluated at.
   */
  Vec3T<T> m_point;
};

/**
 * @brief The tag of a value that comes from no leaf.
 */
inline constexpr uint32_t NoTag = 0xFFFFFFFFU;

/**
 * @brief Whether V is a tagged value.
 * @tparam V Value type.
 */
template <class V>
inline constexpr bool isTagged = false;

/**
 * @brief A Tagged<T> is a tagged value.
 * @tparam T Floating-point precision.
 */
template <class T>
inline constexpr bool isTagged<Tagged<T>> = true;

/**
 * @brief A plain value's value: itself.
 * @tparam T Floating-point precision.
 * @param[in] a_value Value.
 * @return The value.
 */
template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline T
valueOf(const T a_value) noexcept
{
  return a_value;
}

/**
 * @brief A tagged value's value.
 * @tparam T Floating-point precision.
 * @param[in] a_value Tagged value.
 * @return The value.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline T
valueOf(const Tagged<T>& a_value) noexcept
{
  return a_value.m_value;
}

/**
 * @brief A plain value replaced by another: the other.
 * @tparam T Floating-point precision.
 * @param[in] a_value The new value.
 * @return The new value.
 */
template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline T
withValue(const T /*a_like*/, const T a_value) noexcept
{
  return a_value;
}

/**
 * @brief A tagged value with its value replaced and its tag kept.
 * @tparam T Floating-point precision.
 * @param[in] a_like  The value whose tag is kept.
 * @param[in] a_value The new value.
 * @return The tagged value.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline Tagged<T>
withValue(const Tagged<T>& a_like, const T a_value) noexcept
{
  return Tagged<T>{a_value, a_like.m_tag, a_like.m_point};
}

/**
 * @brief A leaf's value: the value itself, or the value tagged with the leaf and the point.
 * @tparam V Value type (T or Tagged<T>).
 * @tparam T Floating-point precision.
 * @param[in] a_value The leaf's value.
 * @param[in] a_tag   The leaf, as the structure numbers it.
 * @param[in] a_point The point the leaf was evaluated at.
 * @return The value.
 */
template <class V, class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline V
leafValue(const T a_value, [[maybe_unused]] const uint32_t a_tag, [[maybe_unused]] const Vec3T<T>& a_point) noexcept
{
  if constexpr (isTagged<V>) {
    return V{a_value, a_tag, a_point};
  }
  else {
    return a_value;
  }
}

/**
 * @brief A value negated, its tag kept.
 * @tparam V Value type (T or Tagged<T>).
 * @param[in] a_value Value.
 * @return The negated value.
 */
template <class V>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline V
negated(const V& a_value) noexcept
{
  if constexpr (isTagged<V>) {
    return withValue(a_value, -a_value.m_value);
  }
  else {
    return -a_value;
  }
}

/**
 * @brief A value's magnitude, its tag kept.
 * @tparam V Value type (T or Tagged<T>).
 * @param[in] a_value Value.
 * @return The magnitude.
 */
template <class V>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline V
magnitude(const V& a_value) noexcept
{
  if constexpr (isTagged<V>) {
    return withValue(a_value, std::abs(a_value.m_value));
  }
  else {
    return std::abs(a_value);
  }
}

/**
 * @brief A value plus a constant, its tag kept.
 * @tparam V Value type (T or Tagged<T>).
 * @tparam T Floating-point precision.
 * @param[in] a_value    Value.
 * @param[in] a_constant Constant.
 * @return The sum.
 */
template <class V, class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline V
plus(const V& a_value, const T a_constant) noexcept
{
  if constexpr (isTagged<V>) {
    return withValue(a_value, a_value.m_value + a_constant);
  }
  else {
    return a_value + a_constant;
  }
}

/**
 * @brief A value times a constant, its tag kept.
 * @tparam V Value type (T or Tagged<T>).
 * @tparam T Floating-point precision.
 * @param[in] a_value    Value.
 * @param[in] a_constant Constant.
 * @return The product.
 */
template <class V, class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline V
times(const V& a_value, const T a_constant) noexcept
{
  if constexpr (isTagged<V>) {
    return withValue(a_value, a_value.m_value * a_constant);
  }
  else {
    return a_value * a_constant;
  }
}

/**
 * @brief The smaller of two plain values (Math::min()).
 * @tparam T Floating-point precision.
 * @param[in] a_first  First value.
 * @param[in] a_second Second value.
 * @return The smaller.
 */
template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline T
smaller(const T a_first, const T a_second) noexcept
{
  return Math::min(a_first, a_second);
}

/**
 * @brief The tagged value with the smaller value, as Math::min() picks it.
 * @tparam T Floating-point precision.
 * @param[in] a_first  First value.
 * @param[in] a_second Second value.
 * @return The smaller, with its tag.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline Tagged<T>
smaller(const Tagged<T>& a_first, const Tagged<T>& a_second) noexcept
{
  return (a_second.m_value < a_first.m_value) ? a_second : a_first;
}

/**
 * @brief The larger of two plain values (Math::max()).
 * @tparam T Floating-point precision.
 * @param[in] a_first  First value.
 * @param[in] a_second Second value.
 * @return The larger.
 */
template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline T
larger(const T a_first, const T a_second) noexcept
{
  return Math::max(a_first, a_second);
}

/**
 * @brief The tagged value with the larger value, as Math::max() picks it.
 * @tparam T Floating-point precision.
 * @param[in] a_first  First value.
 * @param[in] a_second Second value.
 * @return The larger, with its tag.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline Tagged<T>
larger(const Tagged<T>& a_first, const Tagged<T>& a_second) noexcept
{
  return (a_first.m_value < a_second.m_value) ? a_second : a_first;
}

/**
 * @brief An affine map's 12 coefficients: the three rows of its matrix, then its shift.
 * @tparam T Floating-point precision.
 */
template <class T>
struct AffineCoefficients
{
  /**
   * @brief Row 0, row 1 and row 2 of the matrix, then the shift.
   */
  T m_values[12];
};

/**
 * @brief An affine map's coefficients, laid out as affinePoint() reads them.
 * @tparam T Floating-point precision.
 * @param[in] a_map Map.
 * @return The coefficients.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline AffineCoefficients<T>
affineCoefficients(const Affine3T<T>& a_map) noexcept
{
  return AffineCoefficients<T>{{a_map.m_row0[0],
                                a_map.m_row0[1],
                                a_map.m_row0[2],
                                a_map.m_row1[0],
                                a_map.m_row1[1],
                                a_map.m_row1[2],
                                a_map.m_row2[0],
                                a_map.m_row2[1],
                                a_map.m_row2[2],
                                a_map.m_shift[0],
                                a_map.m_shift[1],
                                a_map.m_shift[2]}};
}

/**
 * @brief The point an affine map sends a query point to: the point the mapped object's operand sees.
 * @tparam T Floating-point precision.
 * @param[in] a_coefficients The map's 12 coefficients, as affineCoefficients() lays them out.
 * @param[in] a_point        Query point.
 * @return The mapped point.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
affinePoint(const T* a_coefficients, const Vec3T<T>& a_point) noexcept
{
  const T* m = a_coefficients;

  return Vec3T<T>(m[0] * a_point[0] + m[1] * a_point[1] + m[2] * a_point[2] + m[9],
                  m[3] * a_point[0] + m[4] * a_point[1] + m[5] * a_point[2] + m[10],
                  m[6] * a_point[0] + m[7] * a_point[1] + m[8] * a_point[2] + m[11]);
}

/**
 * @brief The point an affine map sends a query point to, from the map itself.
 * @tparam T Floating-point precision.
 * @param[in] a_map   Map.
 * @param[in] a_point Query point.
 * @return The mapped point, computed as affinePoint() computes it from the map's coefficients.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
affinePoint(const Affine3T<T>& a_map, const Vec3T<T>& a_point) noexcept
{
  const AffineCoefficients<T> coefficients = affineCoefficients(a_map);

  return affinePoint(coefficients.m_values, a_point);
}

/**
 * @brief The point an elongation evaluates its operand at: p - clamp(p, -h, h).
 * @tparam T Floating-point precision.
 * @param[in] a_halfLengths Half-lengths h of the inserted box.
 * @param[in] a_point       Query point.
 * @return The point.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
elongatePoint(const Vec3T<T>& a_halfLengths, const Vec3T<T>& a_point) noexcept
{
  return a_point - clamp(a_point,
                         Vec3T<T>(-a_halfLengths[0], -a_halfLengths[1], -a_halfLengths[2]),
                         Vec3T<T>(a_halfLengths[0], a_halfLengths[1], a_halfLengths[2]));
}

/**
 * @brief An offset's value: v - r, computed as v + (-r).
 * @tparam T Floating-point precision.
 * @param[in] a_value  Operand's value v.
 * @param[in] a_offset Offset r.
 * @return The value.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
offset(const T a_value, const T a_offset) noexcept
{
  return a_value + (-a_offset);
}

/**
 * @brief An annulus's value: |v| - delta, computed as |v| + (-delta).
 * @tparam T Floating-point precision.
 * @param[in] a_value Operand's value v.
 * @param[in] a_delta Half-thickness delta.
 * @return The value.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
annular(const T a_value, const T a_delta) noexcept
{
  return std::abs(a_value) + (-a_delta);
}

/**
 * @brief The two most extreme values seen so far, for an n-ary smooth union or intersection.
 * @details Only the two smallest (or largest) operands' values are blended, which differs from folding a
 * binary blend over the operands (see SmoothUnion()).
 * @tparam V Value type (T or Tagged<T>).
 */
template <class V>
struct Pair
{
  /**
   * @brief The most extreme value.
   */
  V m_first;

  /**
   * @brief The next.
   */
  V m_second;
};

/**
 * @brief The empty pair: both values +inf (for the smallest) or -inf (for the largest).
 * @tparam V Value type (T or Tagged<T>).
 * @param[in] a_largest Whether the pair tracks the largest values.
 * @return The pair.
 */
template <class V>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Pair<V>
pairInit(const bool a_largest) noexcept
{
  if constexpr (isTagged<V>) {
    using T = decltype(V::m_value);

    const T empty = a_largest ? -Math::Limits<T>::infinity() : Math::Limits<T>::infinity();
    const V tagged{empty, NoTag, Vec3T<T>::zeros()};

    return Pair<V>{tagged, tagged};
  }
  else {
    const V empty = a_largest ? -Math::Limits<V>::infinity() : Math::Limits<V>::infinity();

    return Pair<V>{empty, empty};
  }
}

/**
 * @brief Insert a value into a pair of extreme values.
 * @details The plain value has an overload of its own, written as it was before values could be tagged:
 * GCC weighs a callee's body when it decides what to inline into the tape's evaluator, and a generic body
 * changed what it inlined there (see the Blur kernel in blur()).
 * @tparam T Floating-point precision.
 * @param[in,out] a_pair    Pair.
 * @param[in]     a_value   Value.
 * @param[in]     a_largest Whether the pair tracks the largest values.
 */
template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
EBGEOMETRY_HOST_DEVICE
inline void
pairInsert(Pair<T>& a_pair, const T a_value, const bool a_largest) noexcept
{
  const bool beyondFirst  = a_largest ? (a_value > a_pair.m_first) : (a_value < a_pair.m_first);
  const bool beyondSecond = a_largest ? (a_value > a_pair.m_second) : (a_value < a_pair.m_second);

  if (beyondFirst) {
    a_pair.m_second = a_pair.m_first;
    a_pair.m_first  = a_value;
  }
  else if (beyondSecond) {
    a_pair.m_second = a_value;
  }
}

/**
 * @brief Insert a tagged value into a pair of extreme values.
 * @tparam T Floating-point precision.
 * @param[in,out] a_pair    Pair.
 * @param[in]     a_value   Tagged value.
 * @param[in]     a_largest Whether the pair tracks the largest values.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
pairInsert(Pair<Tagged<T>>& a_pair, const Tagged<T>& a_value, const bool a_largest) noexcept
{
  const T value  = a_value.m_value;
  const T first  = a_pair.m_first.m_value;
  const T second = a_pair.m_second.m_value;

  const bool beyondFirst  = a_largest ? (value > first) : (value < first);
  const bool beyondSecond = a_largest ? (value > second) : (value < second);

  if (beyondFirst) {
    a_pair.m_second = a_pair.m_first;
    a_pair.m_first  = a_value;
  }
  else if (beyondSecond) {
    a_pair.m_second = a_value;
  }
}

/**
 * @brief The smooth minimum (or maximum) of a pair's two values.
 * @details The plain value has an overload of its own, for the reason given in pairInsert().
 * @tparam T Floating-point precision.
 * @param[in] a_pair        Pair.
 * @param[in] a_smooth      Smoothing length.
 * @param[in] a_largest     Whether this is a smooth maximum.
 * @param[in] a_exponential Whether to blend exponentially (ExpMinOp/ExpMaxOp) rather than polynomially
 * (SmoothMinOp/SmoothMaxOp).
 * @return The blended value.
 */
template <class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
pairBlend(const Pair<T>& a_pair, const T a_smooth, const bool a_largest, const bool a_exponential) noexcept
{
  if (a_exponential) {
    return a_largest ? ExpMaxOp<T>{}(a_pair.m_first, a_pair.m_second, a_smooth)
                     : ExpMinOp<T>{}(a_pair.m_first, a_pair.m_second, a_smooth);
  }

  return a_largest ? SmoothMaxOp<T>{}(a_pair.m_first, a_pair.m_second, a_smooth)
                   : SmoothMinOp<T>{}(a_pair.m_first, a_pair.m_second, a_smooth);
}

/**
 * @brief The smooth minimum (or maximum) of a pair's two tagged values, tagged as the more extreme one.
 * @tparam T Floating-point precision.
 * @param[in] a_pair        Pair.
 * @param[in] a_smooth      Smoothing length.
 * @param[in] a_largest     Whether this is a smooth maximum.
 * @param[in] a_exponential Whether to blend exponentially (ExpMinOp/ExpMaxOp) rather than polynomially
 * (SmoothMinOp/SmoothMaxOp).
 * @return The blended value.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Tagged<T>
pairBlend(const Pair<Tagged<T>>& a_pair, const T a_smooth, const bool a_largest, const bool a_exponential) noexcept
{
  const Pair<T> values{a_pair.m_first.m_value, a_pair.m_second.m_value};

  return withValue(a_pair.m_first, pairBlend(values, a_smooth, a_largest, a_exponential));
}

/**
 * @brief A blur's distance and weights.
 * @tparam T Floating-point precision.
 */
template <class T>
struct BlurWeights
{
  /**
   * @brief The blur distance d, followed by the weight of a tap 0, 1, 2 and 3 steps from the centre.
   */
  T m_values[5];
};

/**
 * @brief A blur's distance and weights: the weight of a tap m steps from the centre is
 * alpha^(3-m) beta^m with beta = (1 - alpha)/2, and the 27 weights sum to (alpha + 2 beta)^3 = 1.
 * @tparam T Floating-point precision.
 * @param[in] a_distance Blur distance d.
 * @param[in] a_alpha    Centre weight factor alpha.
 * @return The distance and weights.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
inline BlurWeights<T>
blurWeights(const T a_distance, const T a_alpha)
{
  const T beta = (T(1) - a_alpha) / T(2);

  return BlurWeights<T>{{a_distance,
                         std::pow(a_alpha, T(3)) * std::pow(beta, T(0)),
                         std::pow(a_alpha, T(2)) * std::pow(beta, T(1)),
                         std::pow(a_alpha, T(1)) * std::pow(beta, T(2)),
                         std::pow(a_alpha, T(0)) * std::pow(beta, T(3))}};
}

/**
 * @brief A blur's value: the weighted sum of its operand's values at the 27 points p + d (i, j, k),
 * i, j, k in {-1, 0, 1}.
 * @tparam T Floating-point precision.
 * @tparam F Callable from a point to the operand's value.
 * @param[in] a_weights The distance d and weights, as blurWeights() lays them out.
 * @param[in] a_point   Query point p.
 * @param[in] a_operand The operand.
 * @return The value.
 */
template <class T, class F>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline auto
blur(const T* a_weights, const Vec3T<T>& a_point, const F& a_operand) noexcept
{
  using V = decltype(a_operand(a_point));

  const T  d = a_weights[0];
  const T* w = a_weights;

  // The 27 taps p + d (i, j, k), in this order; a tap m = |i| + |j| + |k| steps from the centre has weight
  // w[1 + m]. One loop that computes each tap's point itself: with the point computed in a callable
  // called once per tap, nvcc 12.5 placed some taps at the wrong points (issue #217).
  T value = T(0);

  if constexpr (isTagged<V>) {
    // The same sum, carrying the centre tap's tag (the largest weight).
    V centre{};

    for (int t = 0; t < 27; t++) {
      const int i = t / 9 - 1;
      const int j = (t / 3) % 3 - 1;
      const int k = t % 3 - 1;
      const int m = i * i + j * j + k * k;

      const V tapValue = a_operand(a_point + Vec3T<T>(T(i) * d, T(j) * d, T(k) * d));

      value += w[1 + m] * tapValue.m_value;

      if (m == 0) {
        centre = tapValue;
      }
    }

    return withValue(centre, value);
  }
  else {
    // Kept exactly as written for the plain value: holding each tap's value in a variable first made the
    // device compiler give a kernel that evaluates a tape in a loop 255 registers instead of 168.
    for (int t = 0; t < 27; t++) {
      const int i = t / 9 - 1;
      const int j = (t / 3) % 3 - 1;
      const int k = t % 3 - 1;
      const int m = i * i + j * j + k * k;

      const Vec3T<T> tap = a_point + Vec3T<T>(T(i) * d, T(j) * d, T(k) * d);

      value += w[1 + m] * a_operand(tap);
    }

    return value;
  }
}

/**
 * @brief The offset of mollification tap (i, j, k): -r + (i, j, k) dX, dX = 2r / (n - 1).
 * @tparam T Floating-point precision.
 * @param[in] a_radius  Mollification distance r.
 * @param[in] a_spacing Tap spacing dX.
 * @param[in] a_i       Tap index along x.
 * @param[in] a_j       Tap index along y.
 * @param[in] a_k       Tap index along z.
 * @return The offset.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
mollifyOffset(const T a_radius, const T a_spacing, const uint32_t a_i, const uint32_t a_j, const uint32_t a_k) noexcept
{
  return Vec3T<T>(-a_radius + T(a_i) * a_spacing, -a_radius + T(a_j) * a_spacing, -a_radius + T(a_k) * a_spacing);
}

/**
 * @brief The unnormalized weight of a mollification tap: the bump kernel max(0, 1 - |o|^2 / (2r)^2)^2.
 * @details The kernel's support has radius 2r, so every tap in [-r, r]^3, the corners included, gets a
 * positive weight.
 * @tparam T Floating-point precision.
 * @param[in] a_offset Tap offset o.
 * @param[in] a_radius Mollification distance r.
 * @return The weight.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
mollifyWeight(const Vec3T<T>& a_offset, const T a_radius) noexcept
{
  const T t = Math::max(T(0), T(1) - a_offset.length2() / (T(4) * a_radius * a_radius));

  return t * t;
}

/**
 * @brief A mollification's tap grid: n taps per axis over [-r, r]^3, and the sum of their weights.
 * @tparam T Floating-point precision.
 */
template <class T>
struct MollifyGrid
{
  /**
   * @brief Mollification distance r.
   */
  T m_radius;

  /**
   * @brief Tap spacing dX = 2r / (n - 1).
   */
  T m_spacing;

  /**
   * @brief Sum of the unnormalized weights, which every weight is divided by.
   */
  T m_sum;

  /**
   * @brief Taps per axis n (at least 2).
   */
  uint32_t m_samples;
};

/**
 * @brief Set up a mollification's tap grid.
 * @tparam T Floating-point precision.
 * @param[in] a_radius  Mollification distance r (> 0).
 * @param[in] a_samples Taps per axis n (>= 2).
 * @return The grid.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
inline MollifyGrid<T>
mollifyGrid(const T a_radius, const uint32_t a_samples)
{
  MollifyGrid<T> grid{a_radius, T(2) * a_radius / T(a_samples - 1), T(0), a_samples};

  for (uint32_t i = 0; i < a_samples; i++) {
    for (uint32_t j = 0; j < a_samples; j++) {
      for (uint32_t k = 0; k < a_samples; k++) {
        grid.m_sum += mollifyWeight(mollifyOffset(a_radius, grid.m_spacing, i, j, k), a_radius);
      }
    }
  }

  return grid;
}

/**
 * @brief A mollification's value: sum_k w_k f(p - o_k) over the grid's taps, each weight divided by the
 * sum of all of them.
 * @tparam T Floating-point precision.
 * @tparam F Callable from a point to the operand's value.
 * @param[in] a_grid    Tap grid.
 * @param[in] a_point   Query point p.
 * @param[in] a_operand The operand.
 * @return The value.
 */
template <class T, class F>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline auto
mollify(const MollifyGrid<T>& a_grid, const Vec3T<T>& a_point, const F& a_operand) noexcept
{
  using V = decltype(a_operand(a_point));

  T value = T(0);

  if constexpr (isTagged<V>) {
    // The same sum, carrying the tag of the tap with the largest weight (the first, if several have it).
    V heaviest{};
    T heaviestWeight = -T(1);

    for (uint32_t i = 0; i < a_grid.m_samples; i++) {
      for (uint32_t j = 0; j < a_grid.m_samples; j++) {
        for (uint32_t k = 0; k < a_grid.m_samples; k++) {
          const Vec3T<T> o = mollifyOffset(a_grid.m_radius, a_grid.m_spacing, i, j, k);
          const V        f = a_operand(a_point - o);
          const T        w = mollifyWeight(o, a_grid.m_radius) / a_grid.m_sum;

          value += f.m_value * w;

          if (w > heaviestWeight) {
            heaviest       = f;
            heaviestWeight = w;
          }
        }
      }
    }

    return withValue(heaviest, value);
  }
  else {
    // Kept exactly as written for the plain value (see blur()).
    for (uint32_t i = 0; i < a_grid.m_samples; i++) {
      for (uint32_t j = 0; j < a_grid.m_samples; j++) {
        for (uint32_t k = 0; k < a_grid.m_samples; k++) {
          const Vec3T<T> o = mollifyOffset(a_grid.m_radius, a_grid.m_spacing, i, j, k);

          value += a_operand(a_point - o) * (mollifyWeight(o, a_grid.m_radius) / a_grid.m_sum);
        }
      }
    }

    return value;
  }
}

/**
 * @brief A finite repetition: copies of an object on a grid of tiles.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Repetition
{
  /**
   * @brief Tile period along each axis.
   */
  Vec3T<T> m_period;

  /**
   * @brief Whole tiles in the decreasing direction of each axis.
   */
  Vec3T<T> m_low;

  /**
   * @brief Whole tiles in the increasing direction of each axis.
   */
  Vec3T<T> m_high;

  /**
   * @brief Low corner of the repeated object's box (its home copy's).
   */
  Vec3T<T> m_boxLow;

  /**
   * @brief High corner of the repeated object's box.
   */
  Vec3T<T> m_boxHigh;

  /**
   * @brief Period along an axis.
   * @param[in] a_dir Axis.
   * @return The period.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  period(const size_t a_dir) const noexcept
  {
    return m_period[a_dir];
  }

  /**
   * @brief Tiles in the decreasing direction of an axis.
   * @param[in] a_dir Axis.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  low(const size_t a_dir) const noexcept
  {
    return m_low[a_dir];
  }

  /**
   * @brief Tiles in the increasing direction of an axis.
   * @param[in] a_dir Axis.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  high(const size_t a_dir) const noexcept
  {
    return m_high[a_dir];
  }

  /**
   * @brief Low corner of the object's box along an axis.
   * @param[in] a_dir Axis.
   * @return The coordinate.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  boxLow(const size_t a_dir) const noexcept
  {
    return m_boxLow[a_dir];
  }

  /**
   * @brief High corner of the object's box along an axis.
   * @param[in] a_dir Axis.
   * @return The coordinate.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  boxHigh(const size_t a_dir) const noexcept
  {
    return m_boxHigh[a_dir];
  }
};

/**
 * @brief A finite repetition read from an array of 15 parameters (period, low and high counts, the
 * object's box corners), as the Tape stores it.
 * @details It reads the array whenever repetition() asks, rather than copying it, so that the search
 * keeps few values alive across the calls into the object, which a Tape's sub-program call is: copied,
 * they made a device kernel's every function need more registers.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RepetitionView
{
  /**
   * @brief The parameters.
   */
  const T* m_params;

  /**
   * @brief Period along an axis.
   * @param[in] a_dir Axis.
   * @return The period.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  period(const size_t a_dir) const noexcept
  {
    return m_params[a_dir];
  }

  /**
   * @brief Tiles in the decreasing direction of an axis.
   * @param[in] a_dir Axis.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  low(const size_t a_dir) const noexcept
  {
    return m_params[3 + a_dir];
  }

  /**
   * @brief Tiles in the increasing direction of an axis.
   * @param[in] a_dir Axis.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  high(const size_t a_dir) const noexcept
  {
    return m_params[6 + a_dir];
  }

  /**
   * @brief Low corner of the object's box along an axis.
   * @param[in] a_dir Axis.
   * @return The coordinate.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  boxLow(const size_t a_dir) const noexcept
  {
    return m_params[9 + a_dir];
  }

  /**
   * @brief High corner of the object's box along an axis.
   * @param[in] a_dir Axis.
   * @return The coordinate.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  boxHigh(const size_t a_dir) const noexcept
  {
    return m_params[12 + a_dir];
  }
};

/**
 * @brief A finite repetition's value: the minimum of the object's value over all its copies.
 * @details The copy in the tile the point folds into is evaluated first. Its value bounds the search for
 * the others: a copy whose box is farther from the point than max(0, best value so far) cannot be
 * closer, so only the tiles whose copy's box comes that near are evaluated. This is the plain minimum
 * over all copies for an object whose value is at least the distance to its box outside it (a distance
 * or a bound), and keeps the sign for any object, since a copy whose box holds the point is always
 * evaluated. An object that fits well inside its tile has few such neighbours.
 * @tparam T Floating-point precision.
 * @tparam R The repetition's storage: Repetition<T>, or RepetitionView<T> of a parameter array.
 * @tparam F Callable from a point to the object's value.
 * @param[in] a_repetition Repetition.
 * @param[in] a_point      Query point.
 * @param[in] a_object     The object.
 * @return The value.
 */
template <class T, class R, class F>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline auto
repetition(const R& a_repetition, const Vec3T<T>& a_point, const F& a_object) noexcept
{
  using V = decltype(a_object(a_point));

  // The tile the point folds into.
  int home[3];

  for (size_t dir = 0; dir < 3; dir++) {
    home[dir] = static_cast<int>(std::round(
      Math::min(Math::max(a_point[dir] / a_repetition.period(dir), -a_repetition.low(dir)), a_repetition.high(dir))));
  }

  V best = a_object(Vec3T<T>(a_point[0] - a_repetition.period(0) * T(home[0]),
                             a_point[1] - a_repetition.period(1) * T(home[1]),
                             a_point[2] - a_repetition.period(2) * T(home[2])));

  // The tiles whose copy's box comes within max(0, best) of the point, along each axis.
  int first[3];
  int last[3];

  for (size_t dir = 0; dir < 3; dir++) {
    const T reach = Math::max(valueOf(best), T(0));
    const T lo    = std::ceil((a_point[dir] - reach - a_repetition.boxHigh(dir)) / a_repetition.period(dir));
    const T hi    = std::floor((a_point[dir] + reach - a_repetition.boxLow(dir)) / a_repetition.period(dir));

    first[dir] = static_cast<int>(Math::max(lo, -a_repetition.low(dir)));
    last[dir]  = static_cast<int>(Math::min(hi, a_repetition.high(dir)));
  }

  // Nested loops over i, j and k kept their counters alive across the calls into the object, which in a
  // Tape's sub-program raised every function of a device kernel to 255 registers (from 168); one counter,
  // in the same order (k fastest), keeps the kernel's.
  const int64_t ni    = last[0] - first[0] + 1;
  const int64_t nj    = last[1] - first[1] + 1;
  const int64_t nk    = last[2] - first[2] + 1;
  const int64_t count = (ni > 0 && nj > 0 && nk > 0) ? ni * nj * nk : 0;

  for (int64_t n = 0; n < count; n++) {
    const int i = first[0] + static_cast<int>(n / (nj * nk));
    const int j = first[1] + static_cast<int>((n / nk) % nj);
    const int k = first[2] + static_cast<int>(n % nk);

    if (i == home[0] && j == home[1] && k == home[2]) {
      continue;
    }

    const int tile[3] = {i, j, k};
    const T   bound   = Math::max(valueOf(best), T(0));

    T dist2 = T(0);

    for (size_t dir = 0; dir < 3; dir++) {
      const T shift = a_repetition.period(dir) * T(tile[dir]);
      const T below = (a_repetition.boxLow(dir) + shift) - a_point[dir];
      const T above = a_point[dir] - (a_repetition.boxHigh(dir) + shift);
      const T d     = Math::max(Math::max(below, above), T(0));

      dist2 += d * d;
    }

    if (dist2 <= bound * bound) {
      best = smaller(best,
                     a_object(Vec3T<T>(a_point[0] - a_repetition.period(0) * T(i),
                                       a_point[1] - a_repetition.period(1) * T(j),
                                       a_point[2] - a_repetition.period(2) * T(k))));
    }
  }

  return best;
}

} // namespace CSGOps

} // namespace EBGeometry

#endif
