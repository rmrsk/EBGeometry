/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_SignedDistanceFunctionImplem.hpp
  @brief  Implementation of EBGeometry_SignedDistanceFunctionImplem.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_SignedDistanceFunctionImplem
#define EBGeometry_SignedDistanceFunctionImplem

// Our includes
#include "EBGeometry_SignedDistanceFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
T SignedDistanceFunction<T>::unsignedDistance2(const Vec3T<T>& a_point) const noexcept {
  return std::pow(this->signedDistance(a_point), 2);
}

template <class T>
void SignedDistanceFunction<T>::scale(const Vec3T<T>& a_scale) noexcept {
  m_transformOps.emplace_back(std::make_shared<EBGeometry::ScaleOp<T> > (a_scale));
}

template <class T>
void SignedDistanceFunction<T>::translate(const Vec3T<T>& a_translation) noexcept {
  m_transformOps.emplace_back(std::make_shared<EBGeometry::TranslateOp<T> > (a_translation));
}

template <class T>
void SignedDistanceFunction<T>::rotate(const Vec3T<T>& a_point, const Vec3T<T>& a_axis, const T a_theta, const T a_phi) noexcept {

}

template <class T>
Vec3T<T> SignedDistanceFunction<T>::transformPoint(const Vec3T<T>& a_point) const noexcept {
  auto p = a_point;
  
  for (const auto& op : m_transformOps){
    p = op->transform(p);
  }

  return p;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
