/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_UnionImplem.hpp
  @brief  Implementation of EBGeometry_Union.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_UnionImplem
#define EBGeometry_UnionImplem

// Our includes
#include "EBGeometry_Union.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
Union<T>::Union(const std::vector<std::shared_ptr<ImpFunc>>& a_implicitFunctions, const bool a_flipSign)
{
  for (const auto& impFunc : a_implicitFunctions) {
    m_implicitFunctions.emplace_back(impFunc);
  }

  m_flipSign = a_flipSign;
}

template <class T>
Union<T>::Union(const std::vector<std::shared_ptr<SDF>>& a_distanceFunctions, const bool a_flipSign)
{
  for (const auto& sdf : a_distanceFunctions) {
    m_implicitFunctions.emplace_back(sdf);
  }

  m_flipSign = a_flipSign;
}

template <class T>
T
Union<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = std::numeric_limits<T>::infinity();

  for (const auto& impFunc : m_implicitFunctions) {
    ret = std::min(ret, impFunc->value(a_point));
  }

  T sign = (m_flipSign) ? -1.0 : 1.0;

  return sign * ret;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
