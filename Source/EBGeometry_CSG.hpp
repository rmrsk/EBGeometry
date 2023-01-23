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

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace CSG {
  /*!
    @brief Convenience function for taking the union of a bunch of a implicit functions
    @param[in] a_implicitFunctions Implicit functions
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  Union(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept;

  /*!
    @brief Convenience function for taking the union of two implicit functions
    @param[in] a_implicitFunctions1 First implicit function. 
    @param[in] a_implicitFunctions2 Second implicit function. 
  */
  template <class T, class P1, class P2>
  std::shared_ptr<ImplicitFunction<T>>
  Union(const std::shared_ptr<P1>& a_implicitFunction1, const std::shared_ptr<P2>& a_implicitFunction2) noexcept;

  /*!
    @brief Convenience function for taking the union of a bunch of a implicit functions
    @param[in] a_implicitFunctions Implicit functions
    @param[in] a_smooth Smoothing distance. 
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  SmoothUnion(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions, const T a_smooth) noexcept;

  /*!
    @brief Convenience function for taking the union of two implicit functions
    @param[in] a_implicitFunctions1 First implicit function. 
    @param[in] a_implicitFunctions2 Second implicit function. 
    @param[in] a_smooth Smoothing distance. 
  */
  template <class T, class P1, class P2>
  std::shared_ptr<ImplicitFunction<T>>
  SmoothUnion(const std::shared_ptr<P1>& a_implicitFunction1,
              const std::shared_ptr<P2>& a_implicitFunction2,
              const T                    a_smooth) noexcept;

  /*!
    @brief Convenience function for taking the intersection of a bunch of a implicit functions
    @param[in] a_implicitFunctions Implicit functions
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  Intersection(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions) noexcept;

  /*!
    @brief Convenience function for taking the intersection of two implicit functions
    @param[in] a_implicitFunctions1 First implicit function. 
    @param[in] a_implicitFunctions2 Second implicit function. 
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  Intersection(const std::shared_ptr<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunction1,
               const std::shared_ptr<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunction2) noexcept;

  /*!
    @brief Convenience function for taking the smooth intersection of a bunch of a implicit functions
    @param[in] a_implicitFunctions Implicit functions
    @param[in] a_smooth Smoothing distance. 
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  SmoothIntersection(const std::vector<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctions,
                     const T                                                  a_smooth) noexcept;

  /*!
    @brief Convenience function for taking the smooth intersection of two implicit functions
    @param[in] a_implicitFunctions1 First implicit function. 
    @param[in] a_implicitFunctions2 Second implicit function. 
    @param[in] a_smooth Smoothing distance. 
  */
  template <class T, class P1, class P2>
  std::shared_ptr<ImplicitFunction<T>>
  SmoothIntersection(const std::shared_ptr<P1>& a_implicitFunction1,
                     const std::shared_ptr<P2>& a_implicitFunction2,
                     const T                    a_smooth) noexcept;

  /*!
    @brief Convenience function for taking the CSG difference. 
    @param[in] a_implicitFunctionsA Implicit function. 
    @param[in] a_implicitFunctionsB Implicit function to subtract. 
  */
  template <class T>
  std::shared_ptr<ImplicitFunction<T>>
  Difference(const std::shared_ptr<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctionA,
             const std::shared_ptr<std::shared_ptr<ImplicitFunction<T>>>& a_implicitFunctionB) noexcept;

} // namespace CSG

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
  */
  SmoothUnionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&   a_implicitFunctions,
                const T                                                    a_smooth,
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
    @brief Function for taking the smooth minimum
  */
  std::function<T(const T&, const T&, const T&)> m_smoothMin;

  /*!
    @brief Smoothing parameter
  */
  T m_smooth;
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
    @brief Full constructor. Computes the CSG union
    @param[in] a_implicitFunctions List of primitives
  */
  SmoothIntersectionIF(const std::vector<std::shared_ptr<ImplicitFunction<T>>>&    a_implicitFunctions,
                       const T                                                     a_smooth,
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
  T m_smooth;

  /*!
    @brief Smooth max operator.
  */
  std::function<T(const T& a, const T& b, const T& s)> m_smoothMax;
};

/*!
  @brief CSG difference. Computes C = A-B
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
    @brief Full constructor. Computes the CSG intersection.
    @param[in] a_implicitFunctionsA Implicit function
    @param[in] a_implicitFunctionsB Implicit function to subtract
  */
  DifferenceIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionA,
               const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunctionB) noexcept;

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

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_CSGImplem.hpp"

#endif
