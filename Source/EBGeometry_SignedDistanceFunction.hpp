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

#include <memory>
#include <deque>

// Our includes
#include "EBGeometry_TransformOps.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Abstract representation of a signed distance function. 
  @details Users can put whatever they like in here, e.g. analytic functions, DCEL meshes, or DCEL meshes stored in full or compact BVH trees. The signedDistance
  function must be implemented by the user. When computing it, the user can apply transformation operators (rotations, scaling, translations) by calling transformPoint
  on the input coordinate. 
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
    @brief Translate signed distance function. 
    @param[in] a_translation Distance to translate the function. 
  */  
  inline
  void translate(const Vec3T<T>& a_translation) noexcept;

  /*!
    @brief Rotate the signed distance function around. 
    @param[in] a_angle Rotation angle
    @param[in] a_axis  Rotation axis. 0 = x, 1 = y etc. 
  */  
  inline
  void rotate(const T a_angle, const int a_axis) noexcept;    

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
