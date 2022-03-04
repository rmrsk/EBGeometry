/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_UnionBVHImplem.hpp
  @brief  Implementation of EBGeometry_UnionBVH.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_UnionBVHImplem
#define EBGeometry_UnionBVHImplem

// Our includes
#include "EBGeometry_UnionBVH.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T, class BV, int K>
UnionBVH<T, BV, K>::UnionBVH(const std::vector<std::shared_ptr<const SDF> >& a_distanceFunctions, const bool a_flipSign) {
  m_distanceFunctions = a_distanceFunctions;
  m_flipSign          = a_flipSign;
  m_isGood            = false;
}

template <class T, class BV, int K>
void UnionBVH<T, BV, K>::sortAndPartition(const BVConstructor& a_bvConstructor){

  

  m_isGood = true;
}

template <class T, class BV, int K>
T UnionBVH<T, BV, K>::operator()(const Vec3T<T>& a_point) const noexcept {
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
