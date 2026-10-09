// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_FunctionQueriesImplem.hpp
 * @brief  Implementation of EBGeometry_FunctionQueries.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_FUNCTIONQUERIESIMPLEM_HPP
#define EBGEOMETRY_FUNCTIONQUERIESIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

// Our includes
#include "EBGeometry_FunctionQueries.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

namespace FunctionQueriesDetail {

/**
 * @brief Detects whether F has a signedDistance(const Vec3T<T>&) member.
 */
template <class F, class T, class = void>
struct HasSignedDistance : std::false_type
{
};

/**
 * @brief Detects whether F has a signedDistance(const Vec3T<T>&) member (positive case).
 */
template <class F, class T>
struct HasSignedDistance<
  F,
  T,
  std::void_t<decltype(std::declval<const F&>().signedDistance(std::declval<const Vec3T<T>&>()))>> : std::true_type
{
};

/**
 * @brief Detects whether F has a value(const Vec3T<T>&) member.
 */
template <class F, class T, class = void>
struct HasValue : std::false_type
{
};

/**
 * @brief Detects whether F has a value(const Vec3T<T>&) member (positive case).
 */
template <class F, class T>
struct HasValue<F, T, std::void_t<decltype(std::declval<const F&>().value(std::declval<const Vec3T<T>&>()))>>
  : std::true_type
{
};

/**
 * @brief Evaluate a_function at a_point through signedDistance(), value() or operator(), in that order
 * of preference.
 * @tparam T Floating-point precision.
 * @tparam F Function type.
 * @param[in] a_function Function to evaluate.
 * @param[in] a_point    Evaluation point.
 * @return Function value at a_point.
 */
template <class T, class F>
EBGEOMETRY_HOST_DEVICE
inline T
evaluate(const F& a_function, const Vec3T<T>& a_point) noexcept
{
  if constexpr (HasSignedDistance<F, T>::value) {
    return static_cast<T>(a_function.signedDistance(a_point));
  }
  else if constexpr (HasValue<F, T>::value) {
    return static_cast<T>(a_function.value(a_point));
  }
  else {
    return static_cast<T>(a_function(a_point));
  }
}

/**
 * @brief An octree cell: its integer position among the 2^level cells per axis at its level.
 */
struct OctreeCell
{
  uint32_t index[3]; ///< Position along each axis, in [0, 2^level).
  uint32_t level;    ///< Depth below the initial box.
};

} // namespace FunctionQueriesDetail

template <class F, class T>
EBGEOMETRY_HOST_DEVICE
inline BoundingVolumes::AABBT<T>
approximateBoundingVolumeOctree(const F&           a_function,
                                const Vec3T<T>&    a_initialLowCorner,
                                const Vec3T<T>&    a_initialHighCorner,
                                const unsigned int a_maxTreeDepth,
                                const T&           a_safety) noexcept
{
  static_assert(std::is_floating_point_v<T>, "approximateBoundingVolumeOctree requires a floating-point T");

  using Vec3 = Vec3T<T>;
  using Cell = FunctionQueriesDetail::OctreeCell;

  EBGEOMETRY_REQUIRE(a_maxTreeDepth <= MaxOctreeDepth,
                     "approximateBoundingVolumeOctree: the depth %u exceeds MaxOctreeDepth (%u)",
                     a_maxTreeDepth,
                     MaxOctreeDepth);
  EBGEOMETRY_REQUIRE(a_safety >= T(0), "approximateBoundingVolumeOctree: the safety factor must be non-negative");

  const Vec3 size = a_initialHighCorner - a_initialLowCorner;

  // The corners of a cell, from its integer position: exact up to one rounding per coordinate.
  const auto corners = [&](const Cell& a_cell, Vec3& a_lo, Vec3& a_hi) {
    const T width = T(1) / static_cast<T>(uint64_t(1) << a_cell.level);

    for (int dir = 0; dir < 3; dir++) {
      a_lo[dir] = a_initialLowCorner[dir] + size[dir] * (static_cast<T>(a_cell.index[dir]) * width);
      a_hi[dir] = a_initialLowCorner[dir] + size[dir] * (static_cast<T>(a_cell.index[dir] + 1) * width);
    }
  };

  // Whether the surface may pass through a cell: the value at its center is within the safety-scaled
  // half-diagonal.
  const auto mayIntersect = [&](const Cell& a_cell) {
    Vec3 lo;
    Vec3 hi;

    corners(a_cell, lo, hi);

    const Vec3 center = T(0.5) * (lo + hi);
    const T    reach  = (T(1) + a_safety) * (T(0.5) * (hi - lo)).length();

    return std::abs(FunctionQueriesDetail::evaluate<T>(a_function, center)) <= reach;
  };

  const Cell root = {{0, 0, 0}, 0};

  const bool validBox = (size[0] > T(0)) && (size[1] > T(0)) && (size[2] > T(0));

  if (!validBox || !mayIntersect(root)) {
    return BoundingVolumes::AABBT<T>(-Vec3::max(), Vec3::max());
  }

  // Depth-first: when a cell at level k is expanded, each level from 1 to k has left at most seven
  // siblings waiting on the stack, and the expansion pushes at most eight children. Only cells above
  // the deepest level are expanded, so the peak is 7 (D - 1) + 8 = 7 D + 1 for D = MaxOctreeDepth.
  constexpr unsigned int StackSize = 7 * MaxOctreeDepth + 1;

  Cell         stack[StackSize];
  unsigned int top = 0;

  stack[top++] = root;

  Vec3 lo = Vec3::infinity();
  Vec3 hi = -Vec3::infinity();

  while (top > 0) {
    const Cell cell = stack[--top];

    if (cell.level == a_maxTreeDepth) {
      Vec3 cellLo;
      Vec3 cellHi;

      corners(cell, cellLo, cellHi);

      lo = min(lo, cellLo);
      hi = max(hi, cellHi);

      continue;
    }

    for (uint32_t octant = 0; octant < 8; octant++) {
      const Cell child = {{2 * cell.index[0] + (octant & 1U),
                           2 * cell.index[1] + ((octant >> 1U) & 1U),
                           2 * cell.index[2] + ((octant >> 2U) & 1U)},
                          cell.level + 1};

      if (mayIntersect(child)) {
        EBGEOMETRY_EXPECT(top < StackSize);

        stack[top++] = child;
      }
    }
  }

  // A cell the surface may cross can have children it rules out, all eight of them; then nothing
  // reaches the deepest level, and the root's own test is all there is to go on.
  if (!(lo[0] <= hi[0])) {
    return BoundingVolumes::AABBT<T>(-Vec3::max(), Vec3::max());
  }

  return BoundingVolumes::AABBT<T>(lo, hi);
}

template <class F, class T>
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
normal(const F& a_function, const Vec3T<T>& a_point, const T& a_delta) noexcept
{
  static_assert(std::is_floating_point_v<T>, "normal requires a floating-point T");

  EBGEOMETRY_EXPECT(a_delta > T(0));

  Vec3T<T> n = Vec3T<T>::zeros();

  for (size_t dir = 0; dir < 3; dir++) {
    const T hi = FunctionQueriesDetail::evaluate<T>(a_function, a_point + a_delta * Vec3T<T>::unit(dir));
    const T lo = FunctionQueriesDetail::evaluate<T>(a_function, a_point - a_delta * Vec3T<T>::unit(dir));

    n[dir] = (hi - lo) / (T(2) * a_delta);
  }

  EBGEOMETRY_EXPECT(n.length() > T(0));

  return n / n.length();
}

} // namespace EBGeometry

#endif
