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
ScaleOp<T>::ScaleOp() {
  m_scale = Vec3T<T>::unit();
}

template <class T>
ScaleOp<T>::ScaleOp(const Vec3T<T>& a_translation) {
  m_scale = a_translation;
}

template <class T>
Vec3T<T> ScaleOp<T>::transform(const Vec3T<T>& a_inputPoint) const noexcept {
  return a_inputPoint/m_scale;
}

template <class T>
RotationOp<T>::RotationOp() {
  m_axis     = Vec3T<T>::unit();
  m_cosAngle = std::cos(T(0.0));
  m_sinAngle = std::sin(T(0.0));
}

template <class T>
RotationOp<T>::RotationOp(const Vec3T<T>& a_axis, const T a_angle) noexcept {
  m_axis     = a_axis;
  m_cosAngle = std::cos(a_angle);
  m_sinAngle = std::sin(a_angle);
}

template <class T>
Vec3T<T> RotationOp<T>::transform(const Vec3T<T>& a_inputPoint) const noexcept {
  return a_inputPoint;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
