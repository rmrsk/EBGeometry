/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_TransformOps.hpp
  @brief  Declaration of transformation operators for signed distance fields. 
  @author Robert Marskar
*/

#ifndef EBGeometry_TransformOps
#define EBGeometry_TransformOps

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Base class for transformation operators. 
*/
template <class T>
class TransformOp {
public:

  /*!
    @brief Default constructor
  */
  TransformOp() = default;

  /*!
    @brief Destructor
  */
  virtual ~TransformOp() = default;

  /*!
    @brief Transform input coordinate
    @param[in] a_inputPoint Input point
    @return Returns transformed point. 
  */
  virtual Vec3T<T> transform(const Vec3T<T>& a_inputPoint) const noexcept = 0;
};

/*!
  @brief Translation operator. Can translate an input point
*/
template <class T>
class TranslateOp : public TransformOp<T> {
public:

  /*!
    @brief Default constructor.
  */
  TranslateOp();

  /*!
    @brief Full constructor
  */
  TranslateOp(const Vec3T<T>& a_translation);

  /*!
    @brief Destructor
  */
  virtual ~TranslateOp() = default;

  /*!
    @brief Transform input point. 
  */
  Vec3T<T> transform(const Vec3T<T>& a_inputPoint) const noexcept override;

protected:

  /*!
    @brief Translation of input point
  */
  Vec3T<T> m_translation;
};

/*!
  @brief Rotation operator. Can scale an input point. 
*/
template <class T>
class RotateOp : public TransformOp<T> {
public:

  /*!
    @brief Weak constructor. 
  */
  RotateOp();

  /*!
    @brief Full constructor.
    @param[in] a_angle  Rotation angle
    @param[in] a_axis   Rotation axis
  */
  RotateOp(const T a_angle, const int a_axis) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~RotateOp() = default;

  /*!
    @brief Transform input point. 
  */
  Vec3T<T> transform(const Vec3T<T>& a_inputPoint) const noexcept override;

protected:

  /*!
    @brief Rotation axis. 0 = x, 1=y etc. 
  */
  int m_axis;

  /*!
    @brief Theta-rotation (degrees)
  */
  T m_cosAngle;

  /*!
    @brief Phi-rotation (degrees)
  */
  T m_sinAngle;  
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_TransformOpsImplem.hpp"

#endif
