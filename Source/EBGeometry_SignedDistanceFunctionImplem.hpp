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
inline T
SignedDistanceFunction<T>::value(const Vec3T<T>& a_point) const noexcept
{
  return this->signedDistance(a_point);
}

template <class T>
inline Vec3T<T>
SignedDistanceFunction<T>::normal(const Vec3T<T>& a_point, const T& a_delta) const noexcept
{

  Vec3T<T> n = Vec3T<T>::zero();

  const T id = 1. / (2 * a_delta);

  for (size_t dir = 0; dir < 3; dir++) {
    const T hi = this->signedDistance(a_point + a_delta * Vec3T<T>::unit(dir));
    const T lo = this->signedDistance(a_point - a_delta * Vec3T<T>::unit(dir));

    n[dir] = (hi - lo) / id;
  }

  n /= n.length();

  return n;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
