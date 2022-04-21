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
inline
void SignedDistanceFunction<T>::translate(const Vec3T<T>& a_translation) noexcept {
  m_transformOps.emplace_back(std::make_shared<EBGeometry::TranslateOp<T> > (a_translation));
}

template <class T>
inline
void SignedDistanceFunction<T>::rotate(const T a_angle, const size_t a_axis) noexcept {
  m_transformOps.emplace_back(std::make_shared<EBGeometry::RotateOp<T> >(a_angle, a_axis));
}

template <class T>
inline
Vec3T<T> SignedDistanceFunction<T>::transformPoint(const Vec3T<T>& a_point) const noexcept {
  auto p = a_point;
  
  for (const auto& op : m_transformOps){
    p = op->transform(p);
  }

  return p;
}

template <class T>
inline
Vec3T<T> SignedDistanceFunction<T>::normal(const Vec3T<T>& a_point, const T& a_delta) const noexcept {

  Vec3T<T> n = Vec3T<T>::zero();

  const T id = 1./(2*a_delta);
  
  for (size_t dir = 0; dir < 3; dir++){
    const T hi  = this->signedDistance(a_point + a_delta * Vec3T<T>::unit(dir));
    const T lo  = this->signedDistance(a_point - a_delta * Vec3T<T>::unit(dir));

    n[dir] = (hi - lo)/id;
  }

  n /= n.length();

  return n;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
