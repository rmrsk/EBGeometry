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
Union<T>::Union(const std::vector<std::shared_ptr<SDF> >& a_distanceFunctions, const bool a_flipSign) {
  m_distanceFunctions = a_distanceFunctions;
  m_flipSign          = a_flipSign;
}

template <class T>
T Union<T>::operator()(const Vec3T<T>& a_point) const noexcept {
  const int numDistanceFunctions = m_distanceFunctions.size();

  T ret = std::numeric_limits<T>::infinity();
  
  if(numDistanceFunctions > 0){
    for (const auto & sdf : m_distanceFunctions){
      const T cur = (*sdf)(a_point);

      ret = (std::abs(cur) < std::abs(ret)) ? cur : ret;
    }
  }

  T sign = (m_flipSign) ? -1.0 : 1.0;  
  
  return sign * ret;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
