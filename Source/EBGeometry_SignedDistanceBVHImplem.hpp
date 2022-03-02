/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_SignedDistanceBVHImplem.hpp
  @brief  Implementation of EBGeometry_SignedDistanceBVH.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_SignedDistanceBVHImplem
#define EBGeometry_SignedDistanceBVHImplem

// Our includes
#include "EBGeometry_SignedDistanceBVH.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

using namespace Dcel;
using namespace BVH;

template <class T, class BV, int K>
SignedDistanceBVH<T, BV, K>::SignedDistanceBVH(const std::shared_ptr<Node>& a_rootNode, const bool a_flipSign) {
  m_rootNode = a_rootNode;
  m_flipSign = a_flipSign;
}

template <class T, class BV, int K>
SignedDistanceBVH<T, BV, K>::SignedDistanceBVH(const SignedDistanceBVH& a_other) {
  m_rootNode = a_other.m_rootNode;
  m_flipSign = a_other.m_flipSign;
}

template <class T, class BV, int K>
SignedDistanceBVH<T, BV, K>::~SignedDistanceBVH() {

}

template <class T, class BV, int K>
T SignedDistanceBVH<T, BV, K>::operator()(const Vec3T<T>& a_point) const {

  const T sign = (m_flipSign) ? -1.0 : 1.0;

  return sign * m_rootNode->pruneTree(a_point);
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
