/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_ImplicitFunction.hpp
  @brief  Abstract base class for representing an implicit function.
  @author Robert Marskar
*/

#ifndef EBGeometry_ImplicitFunction
#define EBGeometry_ImplicitFunction

// Std includes
#include <memory>
#include <deque>

// Our includes
#include "EBGeometry_TransformOps.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Abstract representation of an implicit function function (not necessarily signed 
  distance). 

  The value function must be implemented by subclasses. When computing it,
  the user can apply transformation operators (rotations, scaling, translations)
  by calling transformPoint on the input coordinate.
*/
template <class T>
class ImplicitFunction
{
public:
  /*!
    @brief Disallowed, use the full constructor
  */
  ImplicitFunction() = default;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~ImplicitFunction() = default;

  /*!
    @brief Value function. Points are outside the object if value > 0.0 and inside
    if value < 0.0
    @param[in] a_point 3D point.
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept = 0;

  /*!
    @brief Translate signed distance function.
    @param[in] a_translation Distance to translate the function.
  */
  inline void
  translate(const Vec3T<T>& a_translation) noexcept;

  /*!
    @brief Rotate the signed distance function around.
    @param[in] a_angle Rotation angle
    @param[in] a_axis  Rotation axis. 0 = x, 1 = y etc.
  */
  inline void
  rotate(const T a_angle, const size_t a_axis) noexcept;

  /*!
    @brief Apply transformation operators and move point.
    @param[in] a_point Point to transform
  */
  inline Vec3T<T>
  transformPoint(const Vec3T<T>& a_point) const noexcept;

protected:
  /*!
    @brief List of transformation operators for the signed distance field.
  */
  std::deque<std::shared_ptr<TransformOp<T>>> m_transformOps;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_ImplicitFunctionImplem.hpp"

#endif
