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

template <class P, class T>
Union<P, T>::Union(const std::vector<std::shared_ptr<P>>& a_primitives, const bool a_flipSign)
{
  for (const auto& prim : a_primitives) {
    m_primitives.emplace_back(prim);
  }

  m_flipSign = a_flipSign;
}

template <class P, class T>
T
Union<P, T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = std::numeric_limits<T>::infinity();

  for (const auto& prim : m_primitives) {
    ret = std::min(ret, prim->value(a_point));
  }

  T sign = (m_flipSign) ? -1.0 : 1.0;

  return sign * ret;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
