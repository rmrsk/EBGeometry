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
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_Macros.hpp"
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
 * @brief Constructs a BVH-accelerated union of implicit functions.
 * @details Wraps a PackedBVH over the inputs; at query time the BVH culls primitives whose bounding
 * volume is further than the current best distance, giving sub-linear cost on large scenes. Interior
 * semantics are identical to UnionIF.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type; must derive from ImplicitFunction<T>.
 * @tparam BV Bounding volume type (e.g. AABBT<T>).
 * @tparam K  BVH branching factor.
 * @param[in] a_implicitFunctions Input implicit functions.
 * @param[in] a_boundingVolumes   Bounding volumes for each implicit function.
 * @return Shared pointer to a BVHUnionIF<T,P,BV,K> with an internal BVH.
 */
template <class T, class P, class BV, size_t K>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
BVHUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const std::vector<BV>& a_boundingVolumes);

/**
 * @brief Constructs a BVH-accelerated smooth union of implicit functions.
 * @details Uses the BVH to locate the two nearest primitives and applies a smooth-minimum to their
 * values, yielding C1 (or better) blending across nearby surfaces. Sub-linear query cost.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type; must derive from ImplicitFunction<T>.
 * @tparam BV Bounding volume type (e.g. AABBT<T>).
 * @tparam K  BVH branching factor.
 * @param[in] a_implicitFunctions Input implicit functions.
 * @param[in] a_boundingVolumes   Bounding volumes for each implicit function.
 * @param[in] a_smoothLen         Smoothing length (must be > 0).
 * @return Shared pointer to a BVHSmoothUnionIF<T,P,BV,K> with an internal BVH.
 */
template <class T, class P, class BV, size_t K>
[[nodiscard]] std::shared_ptr<ImplicitFunction<T>>
BVHSmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions,
               const std::vector<BV>&                 a_boundingVolumes,
               const T                                a_smoothLen) noexcept;

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
 * The repetition count is clamped to [a_repeatLo, a_repeatHi] per axis; outside this range the
 * nearest tile is evaluated.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type; must derive from ImplicitFunction<T>.
 * @param[in] a_implicitFunction Base implicit function to tile.
 * @param[in] a_period           Tile period in each coordinate direction (all components must be > 0).
 * @param[in] a_repeatLo         Repetition count for decreasing coordinates (integer values).
 * @param[in] a_repeatHi         Repetition count for increasing coordinates (integer values).
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
 * @details Approximates min(a, b) with exponential weighting; the blend region width scales with s.
 * Useful when a differentiable interface is required. Approaches min(a, b) as s → 0.
 * @tparam T Floating-point precision.
 * @param[in] a First value.
 * @param[in] b Second value.
 * @param[in] s Smoothing length (must be > 0).
 * @return Exponentially blended approximation of min(a, b).
 */
template <class T>
std::function<T(const T& a, const T& b, const T& s)> ExpMin = [](const T& a, const T& b, const T& s) -> T {
  static_assert(std::is_floating_point_v<T>, "ExpMin requires a floating-point type T");

  EBGEOMETRY_EXPECT(s > T(0));
  T ret = std::exp(-a / s) + std::exp(-b / s);

  return -std::log(ret) * s;
};

/**
 * @brief Quadratic polynomial smooth minimum for blending two signed-distance values.
 * @details Approximates min(a, b) within the overlap region |a - b| < s; coincides exactly with
 * min(a, b) outside that region. Cheaper to evaluate than ExpMin and the default choice for CSG unions.
 * @tparam T Floating-point precision.
 * @param[in] a First value.
 * @param[in] b Second value.
 * @param[in] s Smoothing length (must be > 0). Controls the blend region width.
 * @return Polynomial-blended approximation of min(a, b).
 */
template <class T>
std::function<T(const T& a, const T& b, const T& s)> SmoothMin = [](const T& a, const T& b, const T& s) -> T {
  static_assert(std::is_floating_point_v<T>, "SmoothMin requires a floating-point type T");

  EBGEOMETRY_EXPECT(s > T(0));
  const T h = std::max(s - std::abs(a - b), T(0)) / s;

  return std::min(a, b) - T(0.25) * h * h * s;
};

/**
 * @brief Quadratic polynomial smooth maximum for blending two signed-distance values.
 * @details Approximates max(a, b) within the overlap region |a - b| < s; coincides exactly with
 * max(a, b) outside that region. Symmetric counterpart to SmoothMin; used for CSG intersections and differences.
 * @tparam T Floating-point precision.
 * @param[in] a First value.
 * @param[in] b Second value.
 * @param[in] s Smoothing length (must be > 0). Controls the blend region width.
 * @return Polynomial-blended approximation of max(a, b).
 */
template <class T>
std::function<T(const T& a, const T& b, const T& s)> SmoothMax = [](const T& a, const T& b, const T& s) -> T {
  static_assert(std::is_floating_point_v<T>, "SmoothMax requires a floating-point type T");

  EBGEOMETRY_EXPECT(s > T(0));
  const T h = std::max(s - std::abs(a - b), T(0)) / s;

  return std::max(a, b) + T(0.25) * h * h * s;
};

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
 * @brief BVH-accelerated union of implicit functions.
 * @details Wraps a PackedBVH over the input primitives. At query time the BVH culls primitives
 * whose bounding volume lies further than the current best distance, giving sub-linear cost on
 * large scenes. Interior semantics are identical to UnionIF.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type; must derive from ImplicitFunction<T>.
 * @tparam BV Bounding volume type (e.g. AABBT<T>).
 * @tparam K  BVH branching factor.
 */
template <class T, class P, class BV, size_t K>
class BVHUnionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "BVHUnionIF requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>, "BVHUnionIF requires an implicit function");
  static_assert(K > 0, "BVHUnionIF BVH branching factor K must be positive");

  /**
   * @brief Alias for the flat BVH type.
   */
  using Root = EBGeometry::BVH::PackedBVH<T, P, K>;

  /**
   * @brief Alias for a BVH node.
   */
  using Node = typename Root::Node;

  /**
   * @brief Disallowed, use the full constructor.
   */
  BVHUnionIF() = delete;

  /**
   * @brief Constructs the BVH-accelerated union from pre-paired primitives and bounding volumes.
   * @param[in] a_primsAndBVs Primitives and their bounding volumes (must be non-empty; no null primitives).
   */
  BVHUnionIF(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs);

  /**
   * @brief Constructs the BVH-accelerated union from separate primitive and bounding-volume lists.
   * @param[in] a_primitives      Input primitives (must be non-empty; same length as a_boundingVolumes; no nulls).
   * @param[in] a_boundingVolumes Bounding volumes for each primitive.
   */
  BVHUnionIF(const std::vector<std::shared_ptr<P>>& a_primitives, const std::vector<BV>& a_boundingVolumes);

  /**
   * @brief Destructor.
   */
  ~BVHUnionIF() override = default;

  /**
   * @brief Evaluates the signed distance at a_point using BVH traversal.
   * @details Returns the minimum value over all primitives not pruned by the BVH. Negative when
   * inside the union.
   * @param[in] a_point 3D query point.
   * @return Minimum signed distance among all non-culled primitives.
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

  /**
   * @brief Returns the axis-aligned bounding box enclosing all primitives.
   * @return Const reference to the root bounding volume of the BVH.
   */
  [[nodiscard]] const EBGeometry::BoundingVolumes::AABBT<T>&
  getBoundingVolume() const noexcept;

protected:
  /**
   * @brief Flat BVH over all input primitives.
   */
  std::shared_ptr<EBGeometry::BVH::PackedBVH<T, P, K>> m_bvh;

  /**
   * @brief Builds the internal BVH from primitive/BV pairs.
   * @param[in] a_primsAndBVs Primitives and their bounding volumes.
   * @param[in] a_build        BVH construction strategy.
   */
  inline void
  buildTree(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs,
            const BVH::Build                                            a_build = BVH::Build::SAH) noexcept;
};

/**
 * @brief BVH-accelerated smooth union of implicit functions.
 * @details Uses the BVH to locate the two nearest primitives and applies a smooth-minimum operator
 * to their values, yielding C1 (or better) blending across nearby surfaces. Query cost is sub-linear
 * in the number of primitives.
 * @tparam T  Floating-point precision.
 * @tparam P  Primitive type; must derive from ImplicitFunction<T>.
 * @tparam BV Bounding volume type (e.g. AABBT<T>).
 * @tparam K  BVH branching factor.
 */
template <class T, class P, class BV, size_t K>
class BVHSmoothUnionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "BVHSmoothUnionIF requires a floating-point type T");
  static_assert(std::is_base_of_v<EBGeometry::ImplicitFunction<T>, P>,
                "BVHSmoothUnionIF requires an implicit function");
  static_assert(K > 0, "BVHSmoothUnionIF BVH branching factor K must be positive");

  /**
   * @brief Alias for the flat BVH type.
   */
  using Root = EBGeometry::BVH::PackedBVH<T, P, K>;

  /**
   * @brief Alias for a BVH node.
   */
  using Node = typename Root::Node;

  /**
   * @brief Disallowed, use the full constructor.
   */
  BVHSmoothUnionIF() = delete;

  /**
   * @brief Constructs the BVH-accelerated smooth union.
   * @param[in] a_distanceFunctions Input implicit functions (must be non-empty; no null entries).
   * @param[in] a_boundingVolumes   Bounding volumes for each function (same length as a_distanceFunctions).
   * @param[in] a_smoothLen         Smoothing length (must be > 0).
   * @param[in] a_smoothMin         Smooth-minimum operator; defaults to SmoothMin<T>.
   */
  BVHSmoothUnionIF(const std::vector<std::shared_ptr<P>>&               a_distanceFunctions,
                   const std::vector<BV>&                               a_boundingVolumes,
                   const T                                              a_smoothLen,
                   const std::function<T(const T&, const T&, const T&)> a_smoothMin = SmoothMin<T>) noexcept;

  /**
   * @brief Destructor.
   */
  ~BVHSmoothUnionIF() override = default;

  /**
   * @brief Evaluates the smoothly blended signed distance at a_point using BVH traversal.
   * @details Finds the two nearest primitives via BVH and applies the stored smooth-minimum. Negative
   * when inside the smooth union.
   * @param[in] a_point 3D query point.
   * @return Smooth minimum signed distance blended from the two closest primitives.
   */
  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override;

  /**
   * @brief Returns the axis-aligned bounding box enclosing all primitives.
   * @return Const reference to the root bounding volume of the BVH.
   */
  [[nodiscard]] const EBGeometry::BoundingVolumes::AABBT<T>&
  getBoundingVolume() const noexcept;

protected:
  /**
   * @brief Flat BVH over all input primitives.
   */
  std::shared_ptr<EBGeometry::BVH::PackedBVH<T, P, K>> m_bvh;

  /**
   * @brief Smoothing length.
   */
  T m_smoothLen;

  /**
   * @brief Smooth-minimum operator.
   */
  std::function<T(const T&, const T&, const T&)> m_smoothMin;

  /**
   * @brief Builds the internal BVH from primitive/BV pairs.
   * @param[in] a_primsAndBVs Primitives and their bounding volumes.
   * @param[in] a_build        BVH construction strategy.
   */
  inline void
  buildTree(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs,
            const BVH::Build                                            a_build = BVH::Build::SAH) noexcept;
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
   * @param[in] a_repeatLo         Repetition count for decreasing coordinates (integer values).
   * @param[in] a_repeatHi         Repetition count for increasing coordinates (integer values).
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
