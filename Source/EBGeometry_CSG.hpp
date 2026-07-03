// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_CSG.hpp
  @brief  Declaration of a CSG operations for implicit functions. 
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_CSG_HPP
#define EBGEOMETRY_CSG_HPP

// Std includes
#include <vector>
#include <type_traits>

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/**
  @brief Convenience function for taking the union of a bunch of implicit functions.
  @tparam T Floating-point precision.
  @tparam P Primitive type; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunctions Implicit functions.
  @return Shared pointer to a UnionIF<T> wrapping all input functions.
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P = ImplicitFunction<T>>
std::shared_ptr<ImplicitFunction<T>>
Union(const std::vector<std::shared_ptr<P>>& a_implicitFunctions) noexcept;

/**
  @brief Convenience function for taking the union of two implicit functions.
  @tparam T  Floating-point precision.
  @tparam P1 Type of the first primitive; must derive from ImplicitFunction<T>.
  @tparam P2 Type of the second primitive; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunctionA First implicit function.
  @param[in] a_implicitFunctionB Second implicit function.
  @return Shared pointer to a UnionIF<T> containing both functions.
  @note P1 and P2 must derive from ImplicitFunction<T>
*/
template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Union(const std::shared_ptr<P1>& a_implicitFunctionA, const std::shared_ptr<P2>& a_implicitFunctionB) noexcept;

/**
  @brief Convenience function for taking the smooth union of a bunch of implicit functions.
  @tparam T Floating-point precision.
  @tparam P Primitive type; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunctions Implicit functions.
  @param[in] a_smooth Smoothing distance.
  @return Shared pointer to a SmoothUnionIF<T> wrapping all input functions.
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P = ImplicitFunction<T>>
std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth) noexcept;

/**
  @brief Convenience function for taking the smooth union of two implicit functions.
  @tparam T  Floating-point precision.
  @tparam P1 Type of the first primitive; must derive from ImplicitFunction<T>.
  @tparam P2 Type of the second primitive; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunctionA First implicit function.
  @param[in] a_implicitFunctionB Second implicit function.
  @param[in] a_smooth Smoothing distance.
  @return Shared pointer to a SmoothUnionIF<T> containing both functions.
  @note P1 and P2 must derive from ImplicitFunction<T>
*/
template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::shared_ptr<P1>& a_implicitFunctionA,
            const std::shared_ptr<P2>& a_implicitFunctionB,
            const T                    a_smooth) noexcept;

/**
  @brief Convenience function for taking the BVH-accelerated union of a bunch of implicit functions.
  @tparam T  Floating-point precision.
  @tparam P  Primitive type; must derive from ImplicitFunction<T>.
  @tparam BV Bounding volume type (e.g. AABBT<T>).
  @tparam K  BVH branching factor.
  @param[in] a_implicitFunctions Implicit functions.
  @param[in] a_boundingVolumes   Bounding volumes for each implicit function.
  @return Shared pointer to a FastUnionIF<T,P,BV,K> with an internal BVH.
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P, class BV, size_t K>
std::shared_ptr<ImplicitFunction<T>>
FastUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions,
          const std::vector<BV>&                 a_boundingVolumes) noexcept;

/**
  @brief Convenience function for taking the BVH-accelerated smooth union of a bunch of implicit functions.
  @tparam T  Floating-point precision.
  @tparam P  Primitive type; must derive from ImplicitFunction<T>.
  @tparam BV Bounding volume type (e.g. AABBT<T>).
  @tparam K  BVH branching factor.
  @param[in] a_implicitFunctions Implicit functions.
  @param[in] a_boundingVolumes   Bounding volumes for each implicit function.
  @param[in] a_smoothLen         Smoothing length.
  @return Shared pointer to a FastSmoothUnionIF<T,P,BV,K> with an internal BVH.
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P, class BV, size_t K>
std::shared_ptr<ImplicitFunction<T>>
FastSmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions,
                const std::vector<BV>&                 a_boundingVolumes,
                const T                                a_smoothLen) noexcept;

/**
  @brief Convenience function for taking the intersection of a bunch of implicit functions.
  @tparam T Floating-point precision.
  @tparam P Primitive type; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunctions Implicit functions.
  @return Shared pointer to an IntersectionIF<T> wrapping all input functions.
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions) noexcept;

/**
  @brief Convenience function for taking the intersection of two implicit functions.
  @tparam T  Floating-point precision.
  @tparam P1 Type of the first primitive; must derive from ImplicitFunction<T>.
  @tparam P2 Type of the second primitive; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunctionA First implicit function.
  @param[in] a_implicitFunctionB Second implicit function.
  @return Shared pointer to an IntersectionIF<T> containing both functions.
  @note P1 and P2 must derive from ImplicitFunction<T>
*/
template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::shared_ptr<std::shared_ptr<P1>>& a_implicitFunctionA,
             const std::shared_ptr<std::shared_ptr<P2>>& a_implicitFunctionB) noexcept;

/**
  @brief Convenience function for taking the smooth intersection of a bunch of implicit functions.
  @tparam T Floating-point precision.
  @tparam P Primitive type; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunctions Implicit functions.
  @param[in] a_smooth Smoothing distance.
  @return Shared pointer to a SmoothIntersectionIF<T> wrapping all input functions.
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth) noexcept;

/**
  @brief Convenience function for taking the smooth intersection of two implicit functions.
  @tparam T  Floating-point precision.
  @tparam P1 Type of the first primitive; must derive from ImplicitFunction<T>.
  @tparam P2 Type of the second primitive; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunctionA First implicit function.
  @param[in] a_implicitFunctionB Second implicit function.
  @param[in] a_smooth Smoothing distance.
  @return Shared pointer to a SmoothIntersectionIF<T> containing both functions.
  @note P1 and P2 must derive from ImplicitFunction<T>
*/
template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::shared_ptr<P1>& a_implicitFunctionA,
                   const std::shared_ptr<P2>& a_implicitFunctionB,
                   const T                    a_smooth) noexcept;

/**
  @brief Convenience function for taking the CSG difference A \ B.
  @tparam T  Floating-point precision.
  @tparam P1 Type of the minuend primitive; must derive from ImplicitFunction<T>.
  @tparam P2 Type of the subtrahend primitive; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunctionA Implicit function (minuend).
  @param[in] a_implicitFunctionB Implicit function to subtract (subtrahend).
  @return Shared pointer to a DifferenceIF<T> representing A \ B.
  @note P1 and P2 must derive from ImplicitFunction<T>
*/
template <class T, class P1 = ImplicitFunction<T>, class P2 = ImplicitFunction<T>>
std::shared_ptr<ImplicitFunction<T>>
Difference(const std::shared_ptr<P1>& a_implicitFunctionA, const std::shared_ptr<P2>& a_implicitFunctionB) noexcept;

/**
  @brief Convenience function for taking the smooth CSG difference A \ B.
  @tparam T  Floating-point precision.
  @tparam P1 Type of the minuend primitive; must derive from ImplicitFunction<T>.
  @tparam P2 Type of the subtrahend primitive; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunctionA Implicit function (minuend).
  @param[in] a_implicitFunctionB Implicit function to subtract (subtrahend).
  @param[in] a_smoothLen         Smoothing length.
  @return Shared pointer to a SmoothDifferenceIF<T> representing the smooth A \ B.
  @note P1 and P2 must derive from ImplicitFunction<T>. This uses the default smoothMax function.
*/
template <class T, class P1 = ImplicitFunction<T>, class P2 = ImplicitFunction<T>>
std::shared_ptr<ImplicitFunction<T>>
SmoothDifference(const std::shared_ptr<P1>& a_implicitFunctionA,
                 const std::shared_ptr<P2>& a_implicitFunctionB,
                 const T                    a_smoothLen) noexcept;

/**
  @brief Convenience function for creating a periodically repeated implicit function.
  @details User inputs the implicit function and the repetition parameters.
  @tparam T Floating-point precision.
  @tparam P Primitive type; must derive from ImplicitFunction<T>.
  @param[in] a_implicitFunction Implicit function to be replicated.
  @param[in] a_period           Repetition period (in each coordinate direction).
  @param[in] a_repeatLo         Number of repetitions for decreasing coordinates (should contain integers).
  @param[in] a_repeatHi         Number of repetitions for increasing coordinates (should contain integers).
  @return Shared pointer to a FiniteRepetitionIF<T>.
*/
template <class T, class P = ImplicitFunction<T>>
std::shared_ptr<ImplicitFunction<T>>
FiniteRepetition(const std::shared_ptr<P>& a_implicitFunction,
                 const Vec3T<T>&           a_period,
                 const Vec3T<T>&           a_repeatLo,
                 const Vec3T<T>&           a_repeatHi) noexcept;

/**
  @brief Exponential smooth-minimum function for CSG.
  @tparam T Floating-point precision.
  @param[in] a First value.
  @param[in] b Second value.
  @param[in] s Smoothing distance (must be > 0).
  @return Smooth minimum of a and b with exponential blending.
*/
template <class T>
std::function<T(const T& a, const T& b, const T& s)> expMin = [](const T& a, const T& b, const T& s) -> T {
  T ret = exp(-a / s) + exp(-b / s);

  return -log(ret) * s;
};

/**
  @brief Polynomial smooth-minimum function for CSG.
  @tparam T Floating-point precision.
  @param[in] a First value.
  @param[in] b Second value.
  @param[in] s Smoothing distance (must be > 0).
  @return Smooth minimum of a and b with quadratic polynomial blending.
*/
template <class T>
std::function<T(const T& a, const T& b, const T& s)> smoothMin = [](const T& a, const T& b, const T& s) -> T {
  const T h = std::max(s - std::abs(a - b), 0.0) / s;

  return std::min(a, b) - 0.25 * h * h * s;
};

/**
  @brief Polynomial smooth-maximum function for CSG.
  @tparam T Floating-point precision.
  @param[in] a First value.
  @param[in] b Second value.
  @param[in] s Smoothing distance (must be > 0).
  @return Smooth maximum of a and b with quadratic polynomial blending.
*/
template <class T>
std::function<T(const T& a, const T& b, const T& s)> smoothMax = [](const T& a, const T& b, const T& s) -> T {
  const T h = std::max(s - std::abs(a - b), 0.0) / s;

  return std::max(a, b) + 0.25 * h * h * s;
};

/**
  @brief CSG union. Computes the minimum value of all input primitives.
  @tparam T Floating-point precision.
*/
template <class T>
class UnionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "UnionIF requires a floating-point type T");

  /**
    @brief Disallowed, use the full constructor
  */
  UnionIF() = delete;

  /**
    @brief Full constructor. Computes the CSG union
    @param[in] a_implicitFunctions List of primitives
  */
  UnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept;

  /**
    @brief Destructor (does nothing)
  */
  virtual ~UnionIF() = default;

  /**
    @brief Value function. Returns the minimum signed distance over all primitives.
    @param[in] a_point 3D query point.
    @return Minimum signed distance value among all stored primitives.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
    @brief List of primitives
  */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;
};

/**
  @brief Smooth CSG union. Computes a smooth minimum over all input primitives.
  @tparam T Floating-point precision.
*/
template <class T>
class SmoothUnionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "SmoothUnionIF requires a floating-point type T");

  /**
    @brief Disallowed, use the full constructor
  */
  SmoothUnionIF() = delete;

  /**
    @brief Full constructor. Computes the CSG union
    @param[in] a_implicitFunctions List of primitives
    @param[in] a_smoothLen Smoothing length
    @param[in] a_smoothMin Smooth min function. 
  */
  SmoothUnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&   a_implicitFunctions,
                const T                                                    a_smoothLen,
                const std::function<T(const T& a, const T& b, const T& s)> a_smoothMin = smoothMin<T>) noexcept;

  /**
    @brief Destructor (does nothing)
  */
  virtual ~SmoothUnionIF() = default;

  /**
    @brief Value function. Returns the smooth minimum over all primitives.
    @param[in] a_point 3D query point.
    @return Smooth minimum signed distance value.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
    @brief List of primitives
  */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;

  /**
    @brief Smoothing parameter
  */
  T m_smoothLen;

  /**
    @brief Function for taking the smooth minimum
  */
  std::function<T(const T&, const T&, const T&)> m_smoothMin;
};

/**
  @brief Implicit function union accelerated by a BVH.
  @details Uses a PackedBVH to cull primitives before evaluating, giving sub-linear query cost.
  @tparam T  Floating-point precision.
  @tparam P  Primitive type; must derive from ImplicitFunction<T>.
  @tparam BV Bounding volume type (e.g. AABBT<T>).
  @tparam K  BVH branching factor.
*/
template <class T, class P, class BV, size_t K>
class FastUnionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "FastUnionIF requires a floating-point type T");
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P>::value,
                "FastUnionIF requires an implicit function");
  static_assert(K > 0, "FastUnionIF BVH branching factor K must be positive");

  /**
    @brief Alias for linearized BVH type
  */
  using Root = EBGeometry::BVH::PackedBVH<T, P, K>;

  /**
    @brief Alias for linearized BVH node
  */
  using Node = typename Root::LinearNode;

  /**
    @brief Disallowed, use the full constructor
  */
  FastUnionIF() = delete;

  /**
    @brief Full constructor - constructs bounding volumes in place.
    @param[in] a_primsAndBVs Primitives and their bounding volumes.
  */
  FastUnionIF(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs) noexcept;

  /**
    @brief Full constructor - constructs bounding volumes in place.
    @param[in] a_primitives Input primitives.
    @param[in] a_boundingVolumes Bounding volumes for primitives.
  */
  FastUnionIF(const std::vector<std::shared_ptr<P>>& a_primitives, const std::vector<BV>& a_boundingVolumes) noexcept;

  /**
    @brief Destructor (does nothing)
  */
  virtual ~FastUnionIF() = default;

  /**
    @brief Value function. Returns the minimum value over all primitives in the BVH.
    @param[in] a_point 3D query point.
    @return Minimum signed distance among all primitives, culled via BVH.
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

  /**
    @brief Get the axis-aligned bounding box enclosing all primitives.
    @return Const reference to the root bounding volume of the BVH.
  */
  const EBGeometry::BoundingVolumes::AABBT<T>&
  getBoundingVolume() const noexcept;

protected:
  /**
    @brief Linearized BVH tree
  */
  std::shared_ptr<EBGeometry::BVH::PackedBVH<T, P, K>> m_bvh;

  /**
    @brief Build BVH tree for the input objects. 
    @param[in] a_primsAndBVs Geometric primitives and their bounding volumes.
    @param[in] a_build Build method (see EBGeometry_BVH.hpp)
  */
  inline void
  buildTree(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs,
            const BVH::Build                                            a_build = BVH::Build::TopDown) noexcept;
};

/**
  @brief BVH-accelerated smooth union of implicit functions.
  @details Extends FastUnionIF by applying a smooth-minimum operator to the two nearest primitives.
  @tparam T  Floating-point precision.
  @tparam P  Primitive type; must derive from ImplicitFunction<T>.
  @tparam BV Bounding volume type (e.g. AABBT<T>).
  @tparam K  BVH branching factor.
*/
template <class T, class P, class BV, size_t K>
class FastSmoothUnionIF : public FastUnionIF<T, P, BV, K>
{
public:
  static_assert(std::is_floating_point_v<T>, "FastSmoothUnionIF requires a floating-point type T");
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P>::value,
                "FastSmoothUnionIF requires an implicit function");
  static_assert(K > 0, "FastSmoothUnionIF BVH branching factor K must be positive");

  /**
    @brief Alias for linearized BVH type
  */
  using Root = EBGeometry::BVH::PackedBVH<T, P, K>;

  /**
    @brief Alias for linearized BVH node
  */
  using Node = typename Root::LinearNode;

  /**
    @brief Disallowed, use the full constructor
  */
  FastSmoothUnionIF() = delete;

  /**
    @brief Full constructor - constructs bounding volumes in place. 
    @param[in] a_distanceFunctions Signed distance functions.
    @param[in] a_boundingVolumes Bounding volumes for the distance fields. 
    @param[in] a_smoothLen Smoothing length
    @param[in] a_smoothMin How to compute the smooth minimum. 
  */
  FastSmoothUnionIF(const std::vector<std::shared_ptr<P>>&               a_distanceFunctions,
                    const std::vector<BV>&                               a_boundingVolumes,
                    const T                                              a_smoothLen,
                    const std::function<T(const T&, const T&, const T&)> a_smoothMin = smoothMin<T>) noexcept;

  /**
    @brief Destructor (does nothing)
  */
  virtual ~FastSmoothUnionIF() = default;

  /**
    @brief Value function. Returns the smooth minimum over the two nearest primitives.
    @param[in] a_point 3D query point.
    @return Smooth minimum signed distance, blending the two closest primitives.
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
    @brief Smoothing length
  */
  T m_smoothLen;

  /**
    @brief Smooth min operator.
  */
  std::function<T(const T&, const T&, const T&)> m_smoothMin;
};

/**
  @brief CSG intersection. Computes the maximum value of all input primitives.
  @tparam T Floating-point precision.
*/
template <class T>
class IntersectionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "IntersectionIF requires a floating-point type T");

  /**
    @brief Disallowed, use the full constructor
  */
  IntersectionIF() = delete;

  /**
    @brief Full constructor. Computes the CSG intersection.
    @param[in] a_implicitFunctions List of primitives
  */
  IntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept;

  /**
    @brief Destructor (does nothing)
  */
  virtual ~IntersectionIF() = default;

  /**
    @brief Value function. Returns the maximum signed distance over all primitives.
    @param[in] a_point 3D query point.
    @return Maximum signed distance value among all stored primitives.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
    @brief List of primitives
  */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;
};

/**
  @brief Smooth CSG intersection. Computes a smooth maximum over all input primitives.
  @tparam T Floating-point precision.
*/
template <class T>
class SmoothIntersectionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "SmoothIntersectionIF requires a floating-point type T");

  /**
    @brief Disallowed, use the full constructor
  */
  SmoothIntersectionIF() = delete;

  /**
    @brief Full constructor. Computes the CSG intersection.
    @param[in] a_implicitFunctionA First implicit function.
    @param[in] a_implicitFunctionB Second implicit function.
    @param[in] a_smoothLen Smoothing length
    @param[in] a_smoothMax Smooth max operator
  */
  SmoothIntersectionIF(const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
                       const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionB,
                       const T                                                     a_smoothLen,
                       const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax = smoothMax<T>) noexcept;

  /**
    @brief Full constructor. Computes the CSG intersection.
    @param[in] a_implicitFunctions List of primitives
    @param[in] a_smoothLen Smoothing length
    @param[in] a_smoothMax Smooth max operator
  */
  SmoothIntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctions,
                       const T                                                     a_smoothLen,
                       const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax = smoothMax<T>) noexcept;

  /**
    @brief Destructor (does nothing)
  */
  virtual ~SmoothIntersectionIF() = default;

  /**
    @brief Value function. Returns the smooth maximum over all primitives.
    @param[in] a_point 3D query point.
    @return Smooth maximum signed distance value.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
    @brief List of primitives
  */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;

  /**
    @brief Smoothing parameter
  */
  T m_smoothLen;

  /**
    @brief Smooth max operator.
  */
  std::function<T(const T& a, const T& b, const T& s)> m_smoothMax;
};

/**
  @brief CSG difference between two implicit functions: A \ B.
  @tparam T Floating-point precision.
*/
template <class T>
class DifferenceIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "DifferenceIF requires a floating-point type T");

  /**
    @brief Disallowed, use the full constructor
  */
  DifferenceIF() = delete;

  /**
    @brief Full constructor. Computes the CSG difference.
    @param[in] a_implicitFunctionA Implicit function
    @param[in] a_implicitFunctionB Implicit function to subtract
  */
  DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionA,
               const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionB) noexcept;

  /**
    @brief Full constructor. Computes the CSG difference between A and the union of the B's
    @param[in] a_implicitFunctionA Implicit function
    @param[in] a_implicitFunctionsB Implicit functions to subtract
  */
  DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>&              a_implicitFunctionA,
               const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctionsB) noexcept;

  /**
    @brief Destructor (does nothing)
  */
  virtual ~DifferenceIF() = default;

  /**
    @brief Value function. Returns max(A, -B).
    @param[in] a_point 3D query point.
    @return Signed distance representing A \ B.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
    @brief First implicit function.
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunctionA;

  /**
    @brief Subtracted implicit function. 
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunctionB;
};

/**
  @brief Smooth CSG difference between two implicit functions: smooth(A \ B).
  @tparam T Floating-point precision.
*/
template <class T>
class SmoothDifferenceIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "SmoothDifferenceIF requires a floating-point type T");

  /**
    @brief Disallowed, use the full constructor
  */
  SmoothDifferenceIF() = delete;

  /**
    @brief Full constructor. Computes the smooth CSG difference.
    @param[in] a_implicitFunctionA Implicit function
    @param[in] a_implicitFunctionB Implicit function to subtract
    @param[in] a_smoothLen          Smoothing length
    @param[in] a_smoothMax          Which smooth-max function to use. 
  */
  SmoothDifferenceIF(const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
                     const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionB,
                     const T                                                     a_smoothLen,
                     const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax = smoothMax<T>) noexcept;

  /**
    @brief Full constructor. Computes the CSG difference between A and the union of the B's
    @param[in] a_implicitFunctionA  Implicit function
    @param[in] a_implicitFunctionsB Implicit functions to subtract
    @param[in] a_smoothLen          Smoothing length
    @param[in] a_smoothMax          Which smooth-max function to use. 
  */
  SmoothDifferenceIF(const std::shared_ptr<ImplicitFunction<T>>&                 a_implicitFunctionA,
                     const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctionsB,
                     const T                                                     a_smoothLen,
                     const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax = smoothMax<T>) noexcept;

  /**
    @brief Destructor (does nothing)
  */
  virtual ~SmoothDifferenceIF() = default;

  /**
    @brief Value function. Returns the smooth difference A \ B via a smooth intersection.
    @param[in] a_point 3D query point.
    @return Smooth signed distance representing A \ B.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
    @brief Resulting smooth intersection.
  */
  std::shared_ptr<SmoothIntersectionIF<T>> m_smoothIntersectionIF;
};

/**
  @brief Periodically repeated implicit function with finite extent.
  @details The user specifies the period and the number of repetitions in each coordinate direction.
  @tparam T Floating-point precision.
*/
template <class T>
class FiniteRepetitionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_floating_point_v<T>, "FiniteRepetitionIF requires a floating-point type T");

  /**
    @brief Disallowed - use the full constructor
  */
  FiniteRepetitionIF() = delete;

  /**
    @brief Full constructor
    @param[in] a_implicitFunction Implicit function to be replicated
    @param[in] a_period           Repetition period (in each coordinate direction)
    @param[in] a_repeatLo         Number of repetitions for decreasing coordinates (should contain integers)
    @param[in] a_repeatHi         Number of repetitions for increasing coordinates (should contain integers)
  */
  FiniteRepetitionIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                     const Vec3T<T>&                             a_period,
                     const Vec3T<T>&                             a_repeatLo,
                     const Vec3T<T>&                             a_repeatHi) noexcept;

  /**
    @brief Destructor (does nothing)
  */
  virtual ~FiniteRepetitionIF() = default;

  /**
    @brief Value function. Folds the query point into the fundamental domain and evaluates the wrapped function.
    @param[in] a_point 3D query point.
    @return Signed distance from the folded point to the underlying implicit function.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /**
    @brief Repetition period
  */
  Vec3T<T> m_period;

  /**
    @brief Number of repetition over increasing coordinate direction
  */
  Vec3T<T> m_repeatHi;

  /**
    @brief Number of repetition over increasing coordinate direction
  */
  Vec3T<T> m_repeatLo;

  /**
    @brief Underlying implicit function
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_CSGImplem.hpp"

#endif
