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

#include "EBGeometry_SignedDistanceFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Signed distance field for sphere. 
*/
template <class T>
class SphereSDF : public SignedDistanceFunction<T> {
public:

  /*!
    @brief Disallowed weak construction. 
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
    @brief Destructor
  */
  virtual ~SphereSDF() = default;  

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
    @brief Signed distance function for sphere. 
    @param[in] a_point Position.
  */
  virtual T signedDistance(const Vec3T<T>& a_point) const noexcept override {
    const T sign = m_flipInside ? -1.0 : 1.0;
    
    return sign * ((a_point - m_center).length() - m_radius);
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

/*!
  @brief Signed distance field for an axis-aligned box. 
*/
template <class T>
class BoxSDF : public SignedDistanceFunction<T> {
public:

  /*!
    @brief Disallowed default constructor
  */
  BoxSDF() = delete;

  /*!
    @brief Full constructor. Sets the low and high corner
    @param[in] a_loCorner   Lower left corner
    @param[in] a_hiCorner   Upper right corner
    @param[in] a_flipInside Flip inside/outside. 
  */
  BoxSDF(const Vec3T<T>& a_loCorner,
	 const Vec3T<T>& a_hiCorner,
	 const bool      a_flipInside) {
    this->m_loCorner   = a_loCorner;
    this->m_hiCorner   = a_hiCorner;
    this->m_flipInside = a_flipInside;
  }

  /*!
    @brief Destructor (does nothing). 
  */
  virtual ~BoxSDF() {

  }

  /*!
    @brief Get lower-left corner
    @return m_loCorner
  */
  const Vec3T<T>& getLowCorner() const noexcept {
    return m_loCorner;
  }

  /*!
    @brief Get lower-left corner
    @return m_loCorner
  */
  Vec3T<T>& getLowCorner() noexcept {
    return m_loCorner;
  }

  /*!
    @brief Get upper-right corner
    @return m_hiCorner
  */
  const Vec3T<T>& getHighCorner() const noexcept {
    return m_hiCorner;
  }

  /*!
    @brief Get upper-right corner
    @return m_hiCorner
  */
  Vec3T<T>& getHighCorner() noexcept {
    return m_hiCorner;
  }

  /*!
    @brief Signed distance function for sphere. 
    @param[in] a_point Position.
  */
  virtual T signedDistance(const Vec3T<T>& a_point) const noexcept override {
    // For each coordinate direction, we have delta[dir] if a_point[dir] falls between xLo and xHi. In this case
    // delta[dir] will be the signed distance to the closest box face in the dir-direction. Otherwise, if a_point[dir] is
    // outside the corner we have delta[dir] > 0.
    const Vec3T<T> delta(std::max(m_loCorner[0] - a_point[0], a_point[0] - m_hiCorner[0]),  // < 0 if point falls between xLo and xHi.
			 std::max(m_loCorner[1] - a_point[1], a_point[1] - m_hiCorner[1]),  // < 0 if point falls between yLo and yHi.
			 std::max(m_loCorner[2] - a_point[2], a_point[2] - m_hiCorner[2])); // < 0 if point falls between zLo and zHi.

    // Note: max is max(Vec3T<T>, Vec3T<T>) and not std::max. It returns a vector with coordinate-wise largest components. Note that
    // the first part std::min(...) is the signed distance on the inside of the box (delta will have negative components). The other
    // part max(Vec3T<T>::zero(), ...) is for outside the box. 
    const T d = std::min(0.0, delta[delta.maxDir(false)]) + max(Vec3T<T>::zero(), delta).length(); 
    
    const T sign = m_flipInside ? -1.0 : 1.0;
    
    return sign * d;
  }

protected:

  /*!
    @brief Low box corner
  */
  Vec3T<T> m_loCorner;

  /*!
    @brief High box corner
  */
  Vec3T<T> m_hiCorner;

  /*!
    @brief Hook for making outside -> inside. 
  */
  bool m_flipInside;
};

/*!
  @brief Signed distance field for a torus. 
  @details This is constructed such that the donut lies in the xy-plane.
*/
template <class T>
class TorusSDF : public SignedDistanceFunction<T> {
public:

  /*!
    @brief Disallowed weak construction.
  */
  TorusSDF() = delete;

  /*!
    @brief Full constructor. Sets the low and high corner
    @param[in] a_loCorner   Lower left corner
    @param[in] a_hiCorner   Upper right corner
    @param[in] a_flipInside Flip inside/outside. 
  */
  TorusSDF(const Vec3T<T>& a_center,
	   const T&        a_majorRadius,
	   const T&        a_minorRadius,	   
	   const bool      a_flipInside) {
    this->m_center      = a_center;
    this->m_majorRadius = a_majorRadius;
    this->m_minorRadius = a_minorRadius;
    this->m_flipInside  = a_flipInside;
  }

  /*!
    @brief Destructor (does nothing). 
  */
  virtual ~TorusSDF() {

  }

  /*!
    @brief Get torus center. 
    @return m_center
  */
  const Vec3T<T>& getCenter() const noexcept {
    return m_center;
  }

  /*!
    @brief Get torus center.
    @return m_center
  */
  Vec3T<T>& getCenter() noexcept {
    return m_center;
  }

  /*!
    @brief Get major radius. 
    @return m_majorRadius
  */
  const T& getMajorRadius() const noexcept {
    return m_majorRadius;
  }

  /*!
    @brief Get major radius. 
    @return m_majorRadius
  */
  T& getMajorRadius() noexcept {
    return m_majorRadius;
  }

  /*!
    @brief Get minor radius. 
    @return m_minorRadius
  */
  const T& getMinorRadius() const noexcept {
    return m_minorRadius;
  }

  /*!
    @brief Get minor radius. 
    @return m_minorRadius
  */
  T& getMinorRadius() noexcept {
    return m_minorRadius;
  }    
  /*!
    @brief Signed distance function for a torus.
    @param[in] a_point Position.
  */
  virtual T signedDistance(const Vec3T<T>& a_point) const noexcept override {
    const Vec3T<T> p = a_point - m_center;
    const T rho  = sqrt(p[0]*p[0] + p[1]*p[1]) - m_majorRadius;
    const T d    = sqrt(rho *rho  + p[2]*p[2]) - m_minorRadius;
    
    const T sign = m_flipInside ? -1.0 : 1.0;
    
    return sign * d;
  }

protected:

  /*!
    @brief Torus center. 
  */
  Vec3T<T> m_center;

  /*!
    @brief Major torus radius. 
  */
  T m_majorRadius;

  /*!
    @brief Minor torus radius. 
  */
  T m_minorRadius;  

  /*!
    @brief Hook for making outside -> inside. 
  */
  bool m_flipInside;
};

/*!
  @brief Signed distance field for a cylinder. 
*/
template <class T>
class CylinderSDF : public SignedDistanceFunction<T> {
public:

  /*!
    @brief Disallowed weak construction. Use one of the full constructors. 
  */
  CylinderSDF()  = delete;

  /*!
    @brief Full constructor. Sets the low and high corner
    @param[in] a_loCorner   Lower left corner
    @param[in] a_hiCorner   Upper right corner
    @param[in] a_flipInside Flip inside/outside. 
  */
  CylinderSDF(const Vec3T<T>& a_center1,
	      const Vec3T<T>& a_center2,	      
	      const T&        a_radius,
	      const bool      a_flipInside) {
    this->m_center1    = a_center1;
    this->m_center2    = a_center2;    
    this->m_radius     = a_radius;
    this->m_flipInside = a_flipInside;

    // Some derived quantities that are needed for SDF computations.
    m_center = (m_center2 + m_center1) * 0.5;
    m_length = (m_center2 - m_center1).length();
    m_axis   = (m_center2 - m_center1)/m_length;
  }

  /*!
    @brief Destructor (does nothing). 
  */
  virtual ~CylinderSDF() {

  }

  /*!
    @brief Get one endpoint
    @return m_center1
  */
  const Vec3T<T>& getCenter1() const noexcept {
    return m_center1;
  }

  /*!
    @brief Get the other endpoint
    @return m_center2
  */
  const Vec3T<T>& getCenter2() const noexcept {
    return m_center2;
  }  

  /*!
    @brief Get radius. 
    @return m_radius. 
  */
  const T& getRadius() const noexcept {
    return m_radius;
  }

  /*!
    @brief Signed distance function for a torus.
    @param[in] a_point Position.
  */
  virtual T signedDistance(const Vec3T<T>& a_point) const noexcept override {
    T d = std::numeric_limits<T>::infinity();

    if(m_length > 0.0 && m_radius > 0.0){
      const Vec3T<T> point = a_point - m_center;
      const T        para  = point.dotProduct(m_axis); // Distance between a_point and cylinder center, projected onto cylinder axis. 
      const Vec3T<T> ortho = point - para * m_axis;    // Distance from cylinder axis. 

      const T w = ortho.length() - m_radius;     // Distance from cylinder wall. < 0 on inside and > 0 on outside. 
      const T h = std::abs(para) - 0.5*m_length; // Distance from cylinder top.  < 0 on inside and > 0 on outside.

      constexpr T zero = T(0.0);
      
      if(w <= zero && h <= zero){ // Inside cylinder
	d = (std::abs(w) < std::abs(h)) ? w : h;
      }
      else if (w <= zero && h > zero){ // Above one of the endcaps. 
	d = h;
      }
      else if(w > zero && h < zero) { // Outside radius but between the endcaps. 
	d = w;
      }
      else{ 
	d = sqrt(w*w + h*h);
      }
    }

    const T sign = m_flipInside ? -1.0 : 1.0;

    return sign * d;
  }

protected:

  /*!
    @brief One endpoint.
  */
  Vec3T<T> m_center1;

  /*!
    @brief The other endpoint.
  */
  Vec3T<T> m_center2;

  /*!
    @brief m_Halfway between center1 and m_center2
  */
  Vec3T<T> m_center;

  /*!
    @brief "Axis", pointing from m_center1 to m_center2.
  */
  Vec3T<T> m_axis;

  /*!
    @brief Cylinder length
  */
  T m_length;

  /*!
    @brief radius. 
  */
  T m_radius;

  /*!
    @brief Hook for making outside -> inside. 
  */
  bool m_flipInside;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_AnalyticDistanceFunctions.hpp"

#endif
