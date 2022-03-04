/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_SignedDistanceFunction.hpp
  @brief  Abstract base class for representing a signed distance function. 
  @author Robert Marskar
*/

#ifndef EBGeometry_SignedDistanceFunction
#define EBGeometry_SignedDistanceFunction

#include <deque>

// Our includes
#include "EBGeometry_TransformOps.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Abstract representation of a signed distance function. 
*/
template <class T>
class SignedDistanceFunction {
public:

  /*!
    @brief Disallowed, use the full constructor
  */
  SignedDistanceFunction() = default;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~SignedDistanceFunction() = default;

  /*!
    @brief Signed distance function. 
    @param[in] a_point 3D point. 
  */
  virtual T signedDistance(const Vec3T<T>& a_point) const noexcept = 0;

  /*!
    @brief Unsigned distance function. Must return the distance^2. 
    @param[in] a_point 3D point. 
  */
  virtual T unsignedDistance2(const Vec3T<T>& a_point) const noexcept;

  /*!
    @brief Scale signed distance function. 
    @param[in] a_Scaling factor. 
  */
  inline
  void scale(const Vec3T<T>& a_scale) noexcept;

  /*!
    @brief Translate signed distance function. 
    @param[in] a_translation Distance to translate the function. 
  */  
  inline
  void translate(const Vec3T<T>& a_translation) noexcept;

  /*!
    @brief Rotate the signed distance function around. 
    @param[in] a_center Center-point for rotation. 
    @param[in] a_axis   Rotation axis
    @param[in] a_theta  Theta-rotation (degrees)
    @param[in] a_phi    Phi-rotation (degrees)
  */  
  inline
  void rotate(const Vec3T<T>& a_center, const Vec3T<T>& a_axis, const T a_theta, const T a_phi) noexcept;    

protected:

  /*!
    @brief List of transformation operators for the signed distance field. 
  */
  std::deque<std::shared_ptr<TransformOp<T> > > m_transformOps;

  /*!
    @brief Apply transformation operators and move point. 
  */
  inline
  Vec3T<T> transformPoint(const Vec3T<T>& a_point) const noexcept;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_SignedDistanceFunctionImplem.hpp"

#endif
