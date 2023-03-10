/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_CSG.hpp
  @brief  Declaration of a CSG operations for implicit functions. 
  @author Robert Marskar
*/

#ifndef EBGeometry_CSG
#define EBGeometry_CSG

// Std includes
#include <vector>
#include <type_traits>

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Convenience function for taking the union of a bunch of a implicit functions
  @param[in] a_implicitFunctions Implicit functions
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P = ImplicitFunction<T>>
std::shared_ptr<ImplicitFunction<T>>
Union(const std::vector<std::shared_ptr<P>>& a_implicitFunctions) noexcept;

/*!
  @brief Convenience function for taking the union of two implicit functions
  @param[in] a_implicitFunctionA First implicit function. 
  @param[in] a_implicitFunctionB Second implicit function. 
  @note P1 and P2 must derive from ImplicitFunction<T>
*/
template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Union(const std::shared_ptr<P1>& a_implicitFunctionA, const std::shared_ptr<P2>& a_implicitFunctionB) noexcept;

/*!
  @brief Convenience function for taking the union of a bunch of a implicit functions
  @param[in] a_implicitFunctions Implicit functions
  @param[in] a_smooth Smoothing distance. 
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P = ImplicitFunction<T>>
std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth) noexcept;

/*!
  @brief Convenience function for taking the union of two implicit functions
  @param[in] a_implicitFunctionA First implicit function. 
  @param[in] a_implicitFunctionB Second implicit function. 
  @param[in] a_smooth Smoothing distance. 
  @note P1 and P2 must derive from ImplicitFunction<T>
*/
template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothUnion(const std::shared_ptr<P1>& a_implicitFunctionA,
            const std::shared_ptr<P2>& a_implicitFunctionB,
            const T                    a_smooth) noexcept;

/*!
  @brief Convenience function for taking the BVH-accelerated union of a bunch of a implicit functions
  @param[in] a_implicitFunctions Implicit functions
  @param[in] a_boundingVolumes Bounding volumes for implicit functions. 
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P, class BV, size_t K>
std::shared_ptr<ImplicitFunction<T>>
FastUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions,
          const std::vector<BV>&                 a_boundingVolumes) noexcept;

/*!
  @brief Convenience function for taking the BVH-accelerated union of a bunch of a implicit functions
  @param[in] a_implicitFunctions Implicit functions
  @param[in] a_boundingVolumes Bounding volumes for the implicit functions. 
  @param[in] a_smoothLen Smoothing length
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P, class BV, size_t K>
std::shared_ptr<ImplicitFunction<T>>
FastSmoothUnion(const std::vector<std::shared_ptr<P>>& a_implicitFunctions,
                const std::vector<BV>&                 a_boundingVolumes,
                const T                                a_smoothLen) noexcept;

/*!
  @brief Convenience function for taking the intersection of a bunch of a implicit functions
  @param[in] a_implicitFunctions Implicit functions
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions) noexcept;

/*!
  @brief Convenience function for taking the intersection of two implicit functions
  @param[in] a_implicitFunctionA First implicit function. 
  @param[in] a_implicitFunctionB Second implicit function. 
  @note P1 and P2 must derive from ImplicitFunction<T>
*/
template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
Intersection(const std::shared_ptr<std::shared_ptr<P1>>& a_implicitFunctionA,
             const std::shared_ptr<std::shared_ptr<P2>>& a_implicitFunctionB) noexcept;

/*!
  @brief Convenience function for taking the smooth intersection of a bunch of a implicit functions
  @param[in] a_implicitFunctions Implicit functions
  @param[in] a_smooth Smoothing distance. 
  @note P must derive from ImplicitFunction<T>
*/
template <class T, class P>
std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::vector<std::shared_ptr<P>>& a_implicitFunctions, const T a_smooth) noexcept;

/*!
  @brief Convenience function for taking the smooth intersection of two implicit functions
  @param[in] a_implicitFunctionA First implicit function. 
  @param[in] a_implicitFunctionB Second implicit function. 
  @param[in] a_smooth Smoothing distance. 
  @note P1 and P2 must derive from ImplicitFunction<T>
*/
template <class T, class P1, class P2>
std::shared_ptr<ImplicitFunction<T>>
SmoothIntersection(const std::shared_ptr<P1>& a_implicitFunctionA,
                   const std::shared_ptr<P2>& a_implicitFunctionB,
                   const T                    a_smooth) noexcept;

/*!
  @brief Convenience function for taking the CSG difference. 
  @param[in] a_implicitFunctionA Implicit function. 
  @param[in] a_implicitFunctionB Implicit function to subtract. 
  @note P1 and P2 must derive from ImplicitFunction<T>
*/
template <class T, class P1 = ImplicitFunction<T>, class P2 = ImplicitFunction<T>>
std::shared_ptr<ImplicitFunction<T>>
Difference(const std::shared_ptr<std::shared_ptr<P1>>& a_implicitFunctionA,
           const std::shared_ptr<std::shared_ptr<P2>>& a_implicitFunctionB) noexcept;
/*!
  @brief Convenience function for taking the smooth CSG difference. 
  @param[in] a_implicitFunctionA Implicit function. 
  @param[in] a_implicitFunctionB Implicit function to subtract. 
  @param[in] a_smoothLen Smoothing length. 
  @note P1 and P2 must derive from ImplicitFunction<T>. This uses the default smoothMax function.
*/
template <class T, class P1 = ImplicitFunction<T>, class P2 = ImplicitFunction<T>>
std::shared_ptr<ImplicitFunction<T>>
SmoothDifference(const std::shared_ptr<std::shared_ptr<P1>>& a_implicitFunctionA,
                 const std::shared_ptr<std::shared_ptr<P2>>& a_implicitFunctionB,
                 const T                                     a_smoothLen) noexcept;

/*!
  @brief Exponential minimum function for CSG
  @param[in] a Smallest value
  @param[in] b Second smallest value
  @param[in] s Smoothing distance. 
*/
template <class T>
std::function<T(const T& a, const T& b, const T& s)> expMin = [](const T& a, const T& b, const T& s) -> T {
  T ret = exp(-a / s) + exp(-b / s);

  return -log(ret) * s;
};

/*!
  @brief Smooth minimum function for CSG
  @param[in] a Smallest value
  @param[in] b Second smallest value
  @param[in] s Smoothing distance. 
*/
template <class T>
std::function<T(const T& a, const T& b, const T& s)> smoothMin = [](const T& a, const T& b, const T& s) -> T {
  const T h = std::max(s - std::abs(a - b), 0.0) / s;

  return std::min(a, b) - 0.25 * h * h * s;
};

/*!
  @brief Smooth maximum function for CSG
  @param[in] a Largest value
  @param[in] b Second largest value
  @param[in] s Smoothing distance. 
*/
template <class T>
std::function<T(const T& a, const T& b, const T& s)> smoothMax = [](const T& a, const T& b, const T& s) -> T {
  const T h = std::max(s - std::abs(a - b), 0.0) / s;

  return std::max(a, b) + 0.25 * h * h * s;
};

/*!
  @brief CSG union. Computes the minimum value of all input primitives. 
*/
template <class T>
class UnionIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed, use the full constructor
  */
  UnionIF() = delete;

  /*!
    @brief Full constructor. Computes the CSG union
    @param[in] a_implicitFunctions List of primitives
  */
  UnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~UnionIF() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief List of primitives
  */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;
};

/*!
  @brief Smooth CSG union. Computes the minimum value of all input primitives.
*/
template <class T>
class SmoothUnionIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed, use the full constructor
  */
  SmoothUnionIF() = delete;

  /*!
    @brief Full constructor. Computes the CSG union
    @param[in] a_implicitFunctions List of primitives
    @param[in] a_smoothLen Smoothing length
    @param[in] a_smoothMin Smooth min function. 
  */
  SmoothUnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&   a_implicitFunctions,
                const T                                                    a_smoothLen,
                const std::function<T(const T& a, const T& b, const T& s)> a_smoothMin = smoothMin<T>) noexcept;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~SmoothUnionIF() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief List of primitives
  */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;

  /*!
    @brief Smoothing parameter
  */
  T m_smoothLen;

  /*!
    @brief Function for taking the smooth minimum
  */
  std::function<T(const T&, const T&, const T&)> m_smoothMin;
};

/*!
  @brief Implicit function union using BVHs. 
*/
template <class T, class P, class BV, size_t K>
class FastUnionIF : public ImplicitFunction<T>
{
public:
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P>::value,
                "FastUnionIF requires an implicit function");

  /*!
    @brief Alias for linear BVH type
  */
  using Root = typename EBGeometry::BVH::LinearBVH<T, P, BV, K>;

  /*!
    @brief Alias for linear BVH node
  */
  using Node = typename Root::LinearNode;

  /*!
    @brief Disallowed, use the full constructor
  */
  FastUnionIF() = delete;

  /*!
    @brief Full constructor - constructs bounding volumes in place. 
    @param[in] a_primsAndBVs Primitives and their bounding volumes. 
  */
  FastUnionIF(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs) noexcept;

  /*!
    @brief Full constructor - constructs bounding volumes in place. 
    @param[in] a_primitives Input primitives.
    @param[in] a_boundingVolumes Bounding volumes for primitives. 
  */
  FastUnionIF(const std::vector<std::shared_ptr<P>>& a_primitives, const std::vector<BV>& a_boundingVolumes) noexcept;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~FastUnionIF() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

  /*!
    @brief Get the bounding volume
  */
  const BV&
  getBoundingVolume() const noexcept;

protected:
  /*!
    @brief Root node for linearized BVH tree
  */
  std::shared_ptr<EBGeometry::BVH::LinearBVH<T, P, BV, K>> m_bvh;

  /*!
    @brief Build BVH tree for the input objects. 
    @param[in] a_primsAndBVs Geometric primitives and their bounding volumes. 
  */
  inline void
  buildTree(const std::vector<std::pair<std::shared_ptr<const P>, BV>>& a_primsAndBVs) noexcept;
};

/*!
  @brief Implicit function smoothed union using BVHs. 
  @note If the BVH-enabled union is to make sense, the primitives must be 
  distance fields (I think). There's a static_assert to make sure of that. 
*/
template <class T, class P, class BV, size_t K>
class FastSmoothUnionIF : public FastUnionIF<T, P, BV, K>
{
public:
  static_assert(std::is_base_of<EBGeometry::ImplicitFunction<T>, P>::value,
                "FastSmoothUnionIF requires an implicit function");

  /*!
    @brief Alias for linear BVH type
  */
  using Root = typename EBGeometry::BVH::LinearBVH<T, P, BV, K>;

  /*!
    @brief Alias for linear BVH node
  */
  using Node = typename Root::LinearNode;

  /*!
    @brief Disallowed, use the full constructor
  */
  FastSmoothUnionIF() = delete;

  /*!
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

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~FastSmoothUnionIF() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Smoothing length
  */
  T m_smoothLen;

  /*!
    @brief Smooth min operator. 
  */
  std::function<T(const T&, const T&, const T&)> m_smoothMin;
};

/*!
  @brief CSG intersection. Computes the maximum value of all input primitives. 
*/
template <class T>
class IntersectionIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed, use the full constructor
  */
  IntersectionIF() = delete;

  /*!
    @brief Full constructor. Computes the CSG intersection.
    @param[in] a_implicitFunctions List of primitives
  */
  IntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~IntersectionIF() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief List of primitives
  */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;
};

/*!
  @brief Smooth intersection 
*/
template <class T>
class SmoothIntersectionIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed, use the full constructor
  */
  SmoothIntersectionIF() = delete;

  /*!
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

  /*!
    @brief Full constructor. Computes the CSG intersection.
    @param[in] a_implicitFunctions List of primitives
    @param[in] a_smoothLen Smoothing length
    @param[in] a_smoothMax Smooth max operator
  */
  SmoothIntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctions,
                       const T                                                     a_smoothLen,
                       const std::function<T(const T& a, const T& b, const T& s)>& a_smoothMax = smoothMax<T>) noexcept;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~SmoothIntersectionIF() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief List of primitives
  */
  std::vector<std::shared_ptr<const ImplicitFunction<T>>> m_implicitFunctions;

  /*!
    @brief Smoothing parameter
  */
  T m_smoothLen;

  /*!
    @brief Smooth max operator.
  */
  std::function<T(const T& a, const T& b, const T& s)> m_smoothMax;
};

/*!
  @brief CSG difference between two implicit functions. 
*/
template <class T>
class DifferenceIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed, use the full constructor
  */
  DifferenceIF() = delete;

  /*!
    @brief Full constructor. Computes the CSG difference.
    @param[in] a_implicitFunctionA Implicit function
    @param[in] a_implicitFunctionB Implicit function to subtract
  */
  DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionA,
               const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionB) noexcept;

  /*!
    @brief Full constructor. Computes the CSG difference between A and the union of the B's
    @param[in] a_implicitFunctionA Implicit function
    @param[in] a_implicitFunctionsB Implicit functions to subtract
  */
  DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>&              a_implicitFunctionA,
               const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctionsB) noexcept;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~DifferenceIF() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief First implicit function. 
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunctionA;

  /*!
    @brief Subtracted implicit function. 
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunctionB;
};

/*!
  @brief CSG difference between two implicit functions. 
*/
template <class T>
class SmoothDifferenceIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed, use the full constructor
  */
  SmoothDifferenceIF() = delete;

  /*!
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

  /*!
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

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~SmoothDifferenceIF() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Resulting smooth intersection.
  */
  std::shared_ptr<SmoothIntersectionIF<T>> m_smoothIntersectionIF;
};

/*!
  @brief Class which creates a periodic repetition of an implicit function.
  @details The user will specify the period (i.e., spacing), and the number of repetitions along increasing and
  decreasing coordinate directions.
*/
template <class T>
class FiniteRepetitionIF : public ImplicitFunction<T>
{
public:
  /*!
    @brief Disallowed - use the full constructor
  */
  FiniteRepetitionIF() = delete;

  /*!
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

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~FiniteRepetitionIF() = default;

  /*!
    @brief Value function
    @param[in] a_point 3D point.
  */
  T
  value(const Vec3T<T>& a_point) const noexcept override;

protected:
  /*!
    @brief Repetition period
  */
  Vec3T<T> m_period;

  /*!
    @brief Number of repetition over increasing coordinate direction
  */
  Vec3T<T> m_repeatHi;

  /*!
    @brief Number of repetition over increasing coordinate direction
  */
  Vec3T<T> m_repeatLo;

  /*!
    @brief Underlying implicit function
  */
  std::shared_ptr<ImplicitFunction<T>> m_implicitFunction;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_CSGImplem.hpp"

#endif
