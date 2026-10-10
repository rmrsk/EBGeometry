// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DistanceQuality.hpp
 * @brief  How far a function's value can be trusted as a distance: DistanceQuality, and the
 * distanceQualityOf trait that reads it from a type.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DISTANCEQUALITY_HPP
#define EBGEOMETRY_DISTANCEQUALITY_HPP

// Std includes
#include <type_traits>

namespace EBGeometry {

/**
 * @brief How far the value of a signed distance function can be trusted, from strongest to weakest.
 * @details Every level keeps the sign: negative inside, positive outside, zero on the surface.
 */
enum class DistanceQuality
{
  /**
   * @brief The value's magnitude is the Euclidean distance to the surface.
   */
  Exact,

  /**
   * @brief The value's magnitude never exceeds the distance to the surface: the function is
   * 1-Lipschitz, so it may underestimate but never overestimates. Exact implies Bound.
   */
  Bound,

  /**
   * @brief Only the sign is meaningful, so the function describes a surface (its zero level set) but
   * says nothing about how far away it is. A BVH union cannot prune such a function.
   */
  NotADistance
};

/**
 * @brief The distance quality of a type: its static `distanceQuality` member if it has one, and
 * DistanceQuality::Bound otherwise.
 * @details A type without the member is taken at its word that it is a distance bound, since that is
 * what the BVH unions need of their primitives. A user type whose value is not a distance should
 * declare `static constexpr DistanceQuality distanceQuality = DistanceQuality::NotADistance;`.
 * @tparam P Type to inspect.
 */
template <class P, class = void>
struct DistanceQualityOf
{
  /**
   * @brief The type's distance quality.
   */
  static constexpr DistanceQuality value = DistanceQuality::Bound;
};

/**
 * @brief The distance quality of a type that declares one.
 * @tparam P Type to inspect.
 */
template <class P>
struct DistanceQualityOf<P, std::void_t<decltype(P::distanceQuality)>>
{
  /**
   * @brief The type's distance quality.
   */
  static constexpr DistanceQuality value = P::distanceQuality;
};

/**
 * @brief Shorthand for the value of DistanceQualityOf<P>.
 * @tparam P Type to inspect.
 */
template <class P>
inline constexpr DistanceQuality distanceQualityOf = DistanceQualityOf<P>::value;

} // namespace EBGeometry

#endif
