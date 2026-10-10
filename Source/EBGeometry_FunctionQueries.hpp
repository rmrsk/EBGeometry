// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_FunctionQueries.hpp
 * @brief  Queries that work on any function of a point: a bounding-volume estimate and a normal.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_FUNCTIONQUERIES_HPP
#define EBGEOMETRY_FUNCTIONQUERIES_HPP

// Std includes
#include <cstdint>
#include <type_traits>
#include <utility>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Deepest subdivision approximateBoundingVolumeOctree() accepts.
 * @details A cell at this depth is 2^-24 of the initial box along each axis, below the resolution of
 * a float coordinate in that box. The bound sizes the function's fixed traversal stack.
 */
constexpr unsigned int MaxOctreeDepth = 24;

/**
 * @brief Estimate the bounding box of a function's zero level set by octree subdivision.
 * @details Subdivides the initial box depth-first into octants, keeping a cell when the function's
 * value at its center is within (1 + a_safety) times the cell's half-diagonal, that is, when the
 * surface may pass through it, and returns the box around the kept cells at depth a_maxTreeDepth.
 * This relies on the function being close to a signed distance: a function whose value changes by
 * at most the distance moved cannot have a surface farther from a cell's center than its value
 * there. The cells are never stored; the traversal keeps a fixed stack and a running minimum and
 * maximum, so the function allocates nothing and is callable on a device if the function is.
 *
 * The function can be anything that is evaluated at a point: an object with a
 * `signedDistance(const Vec3T<T>&)` member (an analytic shape or a mesh SDF), an object with a
 * `value(const Vec3T<T>&)` member (an ImplicitFunction), or a callable taking a `const Vec3T<T>&`,
 * tried in that order.
 *
 * If the initial box is empty or inverted along any axis, or the function's value at its center
 * already rules the surface out of it, or no cell is kept at depth a_maxTreeDepth, the result is the
 * maximal box, from -Vec3T<T>::max() to Vec3T<T>::max(): a sign that the initial box must be chosen
 * more generously, or that the function is far from a signed distance.
 * @tparam F  Function type.
 * @tparam T  Floating-point precision.
 * @param[in] a_function          Function whose zero level set is bounded.
 * @param[in] a_initialLowCorner  Low corner of the initial box.
 * @param[in] a_initialHighCorner High corner of the initial box.
 * @param[in] a_maxTreeDepth      Subdivision depth; at most MaxOctreeDepth.
 * @param[in] a_safety            Safety factor for the cell test; 0 = exact, 1 = a margin of one
 * half-diagonal. Must be non-negative.
 * @return Box enclosing the kept cells.
 */
template <class F, class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline BoundingVolumes::AABBT<T>
approximateBoundingVolumeOctree(const F&           a_function,
                                const Vec3T<T>&    a_initialLowCorner,
                                const Vec3T<T>&    a_initialHighCorner,
                                const unsigned int a_maxTreeDepth,
                                const T&           a_safety = T(0)) noexcept;

/**
 * @brief Outward unit normal of a function's level set, by central finite differences.
 * @details Approximates the gradient at a_point with step a_delta along each axis and normalizes
 * it. The function is evaluated as in approximateBoundingVolumeOctree(). Callable on a device if the
 * function is.
 * @tparam F  Function type.
 * @tparam T  Floating-point precision.
 * @param[in] a_function Function to differentiate.
 * @param[in] a_point    Query point.
 * @param[in] a_delta    Finite-difference step. Must be positive.
 * @return Unit normal at a_point. Undefined if the finite-difference gradient is zero (a_point at
 * a local extremum of the function).
 */
template <class F, class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
normal(const F& a_function, const Vec3T<T>& a_point, const T& a_delta) noexcept;

} // namespace EBGeometry

#include "EBGeometry_FunctionQueriesImplem.hpp"

#endif
