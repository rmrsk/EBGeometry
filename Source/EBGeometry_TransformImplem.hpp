/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_TransformImplem.hpp
  @brief  Implementation of EBGeometry_Transform.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_TransformImplem
#define EBGeometry_TransformImplem

// Our includes
#include "EBGeometry_Transform.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::translate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const Vec3T<T>& a_shift) noexcept
{
  return std::make_shared<TranslateIF<T>>(a_implicitFunction, a_shift);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::rotate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                  const T                                     a_angle,
                  const size_t                                a_axis) noexcept
{
  return std::make_shared<RotateIF<T>>(a_implicitFunction, a_angle, a_axis);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::scale(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_scale) noexcept
{
  return std::make_shared<ScaleIF<T>>(a_implicitFunction, a_scale);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::offset(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_offset) noexcept
{
  return std::make_shared<OffsetIF<T>>(a_implicitFunction, a_offset);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::annular(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_delta) noexcept
{
  return std::make_shared<AnnularIF<T>>(a_implicitFunction, a_delta);
}

template <class T>
TranslateIF<T>::TranslateIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                            const Vec3T<T>&                             a_translation) noexcept
{
  m_implicitFunction = a_implicitFunction;
  m_shift            = a_translation;
}

template <class T>
TranslateIF<T>::~TranslateIF()
{}

template <class T>
T
TranslateIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  return m_implicitFunction->value(a_point - m_shift);
}

template <class T>
RotateIF<T>::RotateIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                      const T                                     a_angle,
                      const size_t                                a_axis) noexcept
{

  m_implicitFunction = a_implicitFunction;
  m_angle            = a_angle;
  m_axis             = a_axis;

  const T theta = m_angle * M_PI / 180.0;

  m_cosAngle = std::cos(theta);
  m_sinAngle = std::sin(theta);
}

template <class T>
RotateIF<T>::~RotateIF()
{}

template <class T>
T
RotateIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  const T& x = a_point[0];
  const T& y = a_point[1];
  const T& z = a_point[2];

  Vec3T<T> rotatedPoint = a_point;

  switch (m_axis) {
  case 0: {
    rotatedPoint[1] = y * m_cosAngle + z * m_sinAngle;
    rotatedPoint[2] = -y * m_sinAngle + z * m_cosAngle;

    break;
  }
  case 1: {
    rotatedPoint[0] = x * m_cosAngle - z * m_sinAngle;
    rotatedPoint[2] = x * m_sinAngle + z * m_cosAngle;

    break;
  }
  case 2: {
    rotatedPoint[0] = x * m_cosAngle + y * m_sinAngle;
    rotatedPoint[1] = -x * m_sinAngle + y * m_cosAngle;

    break;
  }
  }

  return m_implicitFunction->value(rotatedPoint);
}

template <class T>
OffsetIF<T>::OffsetIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_offset) noexcept
{
  m_implicitFunction = a_implicitFunction;
  m_offset           = a_offset;
}

template <class T>
OffsetIF<T>::~OffsetIF()
{}

template <class T>
T
OffsetIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  return m_implicitFunction->value(a_point) - m_offset;
}

template <class T>
ScaleIF<T>::ScaleIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_scale) noexcept
{
  m_implicitFunction = a_implicitFunction;
  m_scale            = a_scale;
}

template <class T>
ScaleIF<T>::~ScaleIF() noexcept
{}

template <class T>
T
ScaleIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  return (m_implicitFunction->value(a_point / m_scale)) * m_scale;
}

template <class T>
AnnularIF<T>::AnnularIF(const std::shared_ptr<ImplicitFunction<T>> a_implicitFunction, const T a_delta)
{
  m_implicitFunction = a_implicitFunction;
  m_delta            = a_delta;
}

template <class T>
AnnularIF<T>::~AnnularIF()
{}

template <class T>
T
AnnularIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  return std::abs(m_implicitFunction->value(a_point)) - m_delta;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
