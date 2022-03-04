/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_SignedDistanceDcelImplem.H
  @brief  Implementation of EBGeometry_SignedDistanceDcel.H
  @author Robert Marskar
*/

#ifndef EBGeometry_SignedDistanceDcelImplem_H
#define EBGeometry_SignedDistanceDcelImplem_H

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_SignedDistanceDcel.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
SignedDistanceDcel<T>::SignedDistanceDcel(const std::shared_ptr<Mesh>& a_mesh, const bool a_flipSign) {
  this->m_mesh     = a_mesh;
  this->m_flipSign = a_flipSign;
}

template <class T>
SignedDistanceDcel<T>::SignedDistanceDcel(const SignedDistanceDcel& a_object) {
  this->m_mesh         = a_object.m_mesh;
  this->m_flipSign     = a_object.m_flipSign;
  this->m_transformOps = a_object.m_transformOps;  
}

template <class T>
SignedDistanceDcel<T>::~SignedDistanceDcel(){

}

template <class T>
T SignedDistanceDcel<T>::signedDistance(const Vec3T<T>& a_point) const noexcept {

  T sign = (m_flipSign) ? -1.0 : 1.0;
  
  return sign * m_mesh->signedDistance(this->transformPoint(a_point));
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
