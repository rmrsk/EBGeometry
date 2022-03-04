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

// Parent class for transformation operator.
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
  @brief Scale operator. Can also be used as a reflection operator. 
*/
template <class T>
class ScaleOp : public TransformOp<T> {
public:

  /*!
    @brief Default constructor.
  */
  ScaleOp();

  /*!
    @brief Full constructor
  */
  ScaleOp(const Vec3T<T>& a_scale);

  /*!
    @brief Destructor
  */
  virtual ~ScaleOp() = default;

  /*!
    @brief Transform input point. 
  */
  Vec3T<T> transform(const Vec3T<T>& a_inputPoint) const noexcept override;

protected:

  /*!
    @brief Scaling of input point. 
  */
  Vec3T<T> m_scale;
};

/*!
  @brief Rotation operator. Can scale an input point. 
*/
template <class T>
class RotationOp : public TransformOp<T> {
public:

  /*!
    @brief Weak constructor. 
  */
  RotationOp();

  /*!
    @brief Full constructor.
    @param[in] a_axis   Rotation axis
    @param[in] a_theta  Theta-rotation (degrees)
    @param[in] a_phi    Phi-rotation (degrees)
  */
  RotationOp(const Vec3T<T>& a_axis, const T a_angle) noexcept;

  /*!
    @brief Destructor
  */
  virtual ~RotationOp() = default;

  /*!
    @brief Transform input point. 
  */
  Vec3T<T> transform(const Vec3T<T>& a_inputPoint) const noexcept override;

protected:

  /*!
    @brief Rotation axis. 
  */
  Vec3T<T> m_axis;

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
