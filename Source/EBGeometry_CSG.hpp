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
#include "EBGeometry_BVHUnion.hpp"
#include "EBGeometry_Blend.hpp"
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
