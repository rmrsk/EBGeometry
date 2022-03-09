/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_TransformOpsImplem.hpp
  @brief  Implementation of EBGeometry_TransformOps.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_TransformOpsImplem
#define EBGeometry_TransformOpsImplem

// Our includes
#include "EBGeometry_TransformOps.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
TranslateOp<T>::TranslateOp() {
  m_translation = Vec3T<T>::zero();
}

template <class T>
TranslateOp<T>::TranslateOp(const Vec3T<T>& a_translation) {
  m_translation = a_translation;
}

template <class T>
Vec3T<T> TranslateOp<T>::transform(const Vec3T<T>& a_inputPoint) const noexcept {
  return a_inputPoint - m_translation;
}

template <class T>
RotateOp<T>::RotateOp() {
  m_axis     = Vec3T<T>::unit();
  m_cosAngle = std::cos(T(0.0));
  m_sinAngle = std::sin(T(0.0));
}

template <class T>
RotateOp<T>::RotateOp(const T a_angle, const int a_axis) noexcept {
  m_axis     = a_axis;
  m_cosAngle = std::cos(a_angle);
  m_sinAngle = std::sin(a_angle);
}

template <class T>
Vec3T<T> RotateOp<T>::transform(const Vec3T<T>& a_inputPoint) const noexcept {

  const T& x = a_inputPoint[0];
  const T& y = a_inputPoint[1];
  const T& z = a_inputPoint[2];

  Vec3T<T> rotatePoint = a_inputPoint;  
  
  switch(m_axis){
  case 0:
    {
      rotatePoint[1] =  y*m_cosAngle + z*m_sinAngle;
      rotatePoint[2] = -y*m_sinAngle + z*m_cosAngle;

      break;
    }
  case 1:
    {
      rotatePoint[0] =  x*m_cosAngle - z*m_sinAngle;
      rotatePoint[2] =  y*m_sinAngle + z*m_cosAngle;
      
      break;
    }
  case 2:
    {
      rotatePoint[0] =  x*m_cosAngle + y*m_sinAngle;
      rotatePoint[1] = -x*m_sinAngle + y*m_cosAngle;
      
      break;
    }
  }
  
  return rotatePoint;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
