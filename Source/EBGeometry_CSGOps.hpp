// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_CSGOps.hpp
 * @brief  The arithmetic of each CSG operation, shared by the Tape's evaluator and the fused operations.
 * @details Internal helpers; not part of the public API. Each function computes one operation's value
 * or point the way both the Tape (EBGeometry_Tape.hpp) and the fused operations (EBGeometry_Fused.hpp)
 * do, in the same order of floating-point operations, so that the two give the same values. The
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
 * @tparam T Floating-point precision.
 */
template <class T>
struct Pair
{
  /**
   * @brief The most extreme value.
   */
  T m_first;

  /**
   * @brief The next.
   */
  T m_second;
};

/**
 * @brief The empty pair: both values +inf (for the smallest) or -inf (for the largest).
 * @tparam T Floating-point precision.
 * @param[in] a_largest Whether the pair tracks the largest values.
 * @return The pair.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Pair<T>
pairInit(const bool a_largest) noexcept
{
  const T empty = a_largest ? -Math::Limits<T>::infinity() : Math::Limits<T>::infinity();

  return Pair<T>{empty, empty};
}

/**
 * @brief Insert a value into a pair of extreme values.
 * @tparam T Floating-point precision.
 * @param[in,out] a_pair    Pair.
 * @param[in]     a_value   Value.
 * @param[in]     a_largest Whether the pair tracks the largest values.
 */
template <class T>
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
 * @brief The smooth minimum (or maximum) of a pair's two values.
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
inline T
blur(const T* a_weights, const Vec3T<T>& a_point, const F& a_operand) noexcept
{
  const T  d = a_weights[0];
  const T* w = a_weights;

  // The 27 taps p + d (i, j, k), in this order; a tap m = |i| + |j| + |k| steps from the centre has weight
  // w[1 + m]. One loop that computes each tap's point itself: with the point computed in a callable
  // called once per tap, nvcc 12.5 placed some taps at the wrong points (issue #217).
  T value = T(0);

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
inline T
mollify(const MollifyGrid<T>& a_grid, const Vec3T<T>& a_point, const F& a_operand) noexcept
{
  T value = T(0);

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
 * @tparam F Callable from a point to the object's value.
 * @param[in] a_repetition Repetition.
 * @param[in] a_point      Query point.
 * @param[in] a_object     The object.
 * @return The value.
 */
template <class T, class F>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
repetition(const Repetition<T>& a_repetition, const Vec3T<T>& a_point, const F& a_object) noexcept
{
  const Vec3T<T>& period = a_repetition.m_period;

  // The tile the point folds into.
  Vec3T<T> home;

  for (size_t dir = 0; dir < 3; dir++) {
    home[dir] =
      std::round(Math::min(Math::max(a_point[dir] / period[dir], -a_repetition.m_low[dir]), a_repetition.m_high[dir]));
  }

  T best = a_object(a_point - period * home);

  // The tiles whose copy's box comes within max(0, best) of the point, along each axis.
  const T reach = Math::max(best, T(0));

  int first[3];
  int last[3];

  for (size_t dir = 0; dir < 3; dir++) {
    const T lo = std::ceil((a_point[dir] - reach - a_repetition.m_boxHigh[dir]) / period[dir]);
    const T hi = std::floor((a_point[dir] + reach - a_repetition.m_boxLow[dir]) / period[dir]);

    first[dir] = static_cast<int>(Math::max(lo, -a_repetition.m_low[dir]));
    last[dir]  = static_cast<int>(Math::min(hi, a_repetition.m_high[dir]));
  }

  for (int i = first[0]; i <= last[0]; i++) {
    for (int j = first[1]; j <= last[1]; j++) {
      for (int k = first[2]; k <= last[2]; k++) {
        const Vec3T<T> tile(static_cast<T>(i), static_cast<T>(j), static_cast<T>(k));

        if (tile == home) {
          continue;
        }

        // Squared distance from the point to this copy's box; prune against the best value so far.
        const Vec3T<T> shift = period * tile;
        const T        bound = Math::max(best, T(0));

        T dist2 = T(0);

        for (size_t dir = 0; dir < 3; dir++) {
          const T below = (a_repetition.m_boxLow[dir] + shift[dir]) - a_point[dir];
          const T above = a_point[dir] - (a_repetition.m_boxHigh[dir] + shift[dir]);
          const T d     = Math::max(Math::max(below, above), T(0));

          dist2 += d * d;
        }

        if (dist2 <= bound * bound) {
          best = Math::min(best, a_object(a_point - shift));
        }
      }
    }
  }

  return best;
}

} // namespace CSGOps

} // namespace EBGeometry

#endif
