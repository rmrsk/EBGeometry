/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_ImplicitFunctionImplem.hpp
  @brief  Implementation of EBGeometry_ImplicitFunctionImplem.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_ImplicitFunctionImplem
#define EBGeometry_ImplicitFunctionImplem

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
inline void
ImplicitFunction<T>::translate(const Vec3T<T>& a_translation) noexcept
{
  m_transformOps.emplace_back(std::make_shared<EBGeometry::TranslateOp<T>>(a_translation));
}

template <class T>
inline void
ImplicitFunction<T>::rotate(const T a_angle, const size_t a_axis) noexcept
{
  m_transformOps.emplace_back(std::make_shared<EBGeometry::RotateOp<T>>(a_angle, a_axis));
}

template <class T>
inline Vec3T<T>
ImplicitFunction<T>::transformPoint(const Vec3T<T>& a_point) const noexcept
{
  auto p = a_point;

  for (const auto& op : m_transformOps) {
    p = op->transform(p);
  }

  return p;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
