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
#include "EBGeometry_AnalyticDistanceFunctions.hpp"
#include "EBGeometry_Transform.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::Complement(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction) noexcept
{
  return std::make_shared<ComplementIF<T>>(a_implicitFunction);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::Translate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const Vec3T<T>& a_shift) noexcept
{
  return std::make_shared<TranslateIF<T>>(a_implicitFunction, a_shift);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::Rotate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                  const T                                     a_angle,
                  const size_t                                a_axis) noexcept
{
  return std::make_shared<RotateIF<T>>(a_implicitFunction, a_angle, a_axis);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::Scale(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_scale) noexcept
{
  return std::make_shared<ScaleIF<T>>(a_implicitFunction, a_scale);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::Offset(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_offset) noexcept
{
  return std::make_shared<OffsetIF<T>>(a_implicitFunction, a_offset);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::Annular(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_delta) noexcept
{
  return std::make_shared<AnnularIF<T>>(a_implicitFunction, a_delta);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::Blur(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_blur) noexcept
{
  return std::make_shared<BlurIF<T>>(a_implicitFunction, a_blur);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Transform::Mollify(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_dist) noexcept
{
  auto mollifier = std::make_shared<SphereSDF<T>>(Vec3T<T>::zero(), std::abs(a_dist), false);

  constexpr size_t numPoints = 3;

  return std::make_shared<MollifyIF<T>>(a_implicitFunction, mollifier, std::abs(a_dist), numPoints);
}

template <class T>
ComplementIF<T>::ComplementIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction) noexcept

{
  m_implicitFunction = a_implicitFunction;
}

template <class T>
ComplementIF<T>::~ComplementIF()
{}

template <class T>
T
ComplementIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  return -m_implicitFunction->value(a_point);
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

template <class T>
BlurIF<T>::BlurIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                  const T                                     a_blurDistance,
                  const T                                     a_alpha) noexcept
{
  m_implicitFunction = a_implicitFunction;
  m_blurDistance     = a_blurDistance;
  m_alpha            = a_alpha;
}

template <class T>
BlurIF<T>::~BlurIF() noexcept
{}

template <class T>
T
BlurIF<T>::value(const Vec3T<T>& a_point) const noexcept
{

  const T A = std::pow(m_alpha, 3.0) * std::pow((1.0 - m_alpha) / 2.0, 0.0);
  const T B = std::pow(m_alpha, 2.0) * std::pow((1.0 - m_alpha) / 2.0, 1.0);
  const T C = std::pow(m_alpha, 1.0) * std::pow((1.0 - m_alpha) / 2.0, 2.0);
  const T D = std::pow(m_alpha, 0.0) * std::pow((1.0 - m_alpha) / 2.0, 3.0);

  const Vec3T<T> p = a_point;
  const Vec3T<T> x = m_blurDistance * Vec3T<T>(1.0, 0.0, 0.0);
  const Vec3T<T> y = m_blurDistance * Vec3T<T>(0.0, 1.0, 0.0);
  const Vec3T<T> z = m_blurDistance * Vec3T<T>(0.0, 0.0, 1.0);

  const auto& f = *m_implicitFunction;

  T value = 0.0;

  value += A * f(p);
  value += B * (f(p + x) + f(p - x) + f(p + y) + f(p - y) + f(p + z) + f(p - z));
  value += C * (f(p + x + y) + f(p + x - y) + f(p - x + y) + f(p - x - y));
  value += C * (f(p + x + z) + f(p + x - z) + f(p - x + z) + f(p - x - z));
  value += C * (f(p + y + y) + f(p + y - y) + f(p - y + y) + f(p - y - y));
  value += C * (f(p + y + z) + f(p + y - z) + f(p - y + z) + f(p - y - z));
  value += D * (f(p + x + y + z) + f(p + x + y - z) + f(p + x - y + z) + f(p + x - y - z));
  value += D * (f(p - z + y + z) + f(p - z + y - z) + f(p - z - y + z) + f(p - z - y - z));

  return value;
}

template <class T>
MollifyIF<T>::MollifyIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                        const std::shared_ptr<ImplicitFunction<T>>& a_mollifier,
                        const T                                     a_maxValue,
                        const size_t                                a_numPoints) noexcept
{
  m_implicitFunction = a_implicitFunction;

  const T maxVal = std::abs(a_maxValue);

  if (maxVal > 0.0 && a_numPoints > 0) {
    const T dX = 2 * maxVal / (a_numPoints - 1);

    for (int i = 0; i < a_numPoints; i++) {
      for (int j = 0; j < a_numPoints; j++) {
        for (int k = 0; k < a_numPoints; k++) {
          const Vec3T<T> pos    = Vec3T<T>(-maxVal + i * dX, -maxVal + j * dX, -maxVal + k * dX);
          const T        weight = a_mollifier->value(pos);

          m_sampledMollifier.emplace_back(pos, weight);
        }
      }
    }

    // Normalize.
    T mollifierSum = 0.0;
    for (const auto& mol : m_sampledMollifier) {
      mollifierSum += mol.second;
    }

    for (auto& w : m_sampledMollifier) {
      w.second /= mollifierSum;
    }
  }
  else {
    m_sampledMollifier.emplace_back(Vec3T<T>::zero(), 1.0);
  }
}

template <class T>
MollifyIF<T>::~MollifyIF() noexcept
{}

template <class T>
T
MollifyIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  T ret = 0.0;

  for (const auto& mollifier : m_sampledMollifier) {
    ret += m_implicitFunction->value(a_point - mollifier.first) * mollifier.second;
  }

  return ret;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
