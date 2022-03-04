/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_AnalyticDistanceFunctions.hpp
  @brief  Declaration of various analytic distance functions.
  @author Robert Marskar
*/

#ifndef EBGeometry_AnalyticDistanceFunctions
#define EBGeometry_AnalyticDistanceFunctions

#include <chrono>
#include <thread>

#include "EBGeometry_SignedDistanceFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Signed distance field for sphere. 
*/
template <class T>
class SphereSDF : public SignedDistanceFunction<T> {
public:

  /*!
    @brief Disallowed weak construction. Use one of the full constructors. 
  */
  SphereSDF() = delete;

  /*!
    @brief Default constructor
    @param[in] a_center Sphere center
    @param[in] a_radius Sphere radius
    @param[in] a_flipInside Flip inside or not
  */
  SphereSDF(const Vec3T<T>& a_center, const T& a_radius, const bool a_flipInside) {
    this->m_center     = a_center;
    this->m_radius     = a_radius;
    this->m_flipInside = a_flipInside;
  }

  /*!
    @brief Copy constructor
  */
  SphereSDF(const SphereSDF& a_other) {
    this->m_center       = a_other.m_center;
    this->m_radius       = a_other.m_radius;
    this->m_flipInside   = a_other.m_flipInside;            
    this->m_transformOps = a_other.m_transformOps;    
  }

  /*!
    @brief Get center
  */
  const Vec3T<T>& getCenter() const noexcept {
    return m_center;
  }

  /*!
    @brief Get center
  */
  Vec3T<T>& getCenter() noexcept {
    return m_center;
  }

  /*!
    @brief Get radius
  */
  const T& getRadius() const noexcept {
    return m_radius;
  }

  /*!
    @brief Get radius
  */
  T& getRadius() noexcept {
    return m_radius;
  }

  /*!
    @brief Destructor
  */
  virtual ~SphereSDF() = default;

  /*!
    @brief Signed distance function
  */
  virtual T signedDistance(const Vec3T<T>& a_point) const noexcept override {
    const int sign = m_flipInside ? -1 : 1;
    
    return sign * ( (a_point-m_center).length() - m_radius );
  }

  virtual T unsignedDistance2(const Vec3T<T>& a_point) const noexcept override {
    
    return (a_point - m_center).length2() - m_radius*m_radius;
  }
  
protected:

  /*!
    @brief Sphere center
  */
  Vec3T<T> m_center;

  /*!
    @brief Sphere radius
  */
  T m_radius;

  /*!
    @brief For flipping sign. 
  */
  bool m_flipInside;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_AnalyticDistanceFunctions.hpp"

#endif
