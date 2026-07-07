// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TransformImplem.hpp
 * @brief  Implementation of EBGeometry_Transform.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRANSFORMIMPLEM_HPP
#define EBGEOMETRY_TRANSFORMIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <memory>

// Our includes
#include "EBGeometry_AnalyticDistanceFunctions.hpp"
#include "EBGeometry_Constants.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Transform.hpp"

namespace EBGeometry {

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Complement(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction)
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  return std::make_shared<ComplementIF<T>>(a_implicitFunction);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Translate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const Vec3T<T>& a_shift)
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  return std::make_shared<TranslateIF<T>>(a_implicitFunction, a_shift);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Rotate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_angle, const size_t a_axis)
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  return std::make_shared<RotateIF<T>>(a_implicitFunction, a_angle, a_axis);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Scale(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_scale)
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  return std::make_shared<ScaleIF<T>>(a_implicitFunction, a_scale);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Offset(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_offset)
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  return std::make_shared<OffsetIF<T>>(a_implicitFunction, a_offset);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Annular(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_delta)
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  return std::make_shared<AnnularIF<T>>(a_implicitFunction, a_delta);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Blur(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_blur)
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  return std::make_shared<BlurIF<T>>(a_implicitFunction, a_blur);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Mollify(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_dist, const size_t a_mollifierSamples)
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  auto mollifier = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), std::abs(a_dist));

  return std::make_shared<MollifyIF<T>>(a_implicitFunction, mollifier, std::abs(a_dist), a_mollifierSamples);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Elongate(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const Vec3T<T>& a_elongation)
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  return std::make_shared<ElongateIF<T>>(a_implicitFunction, a_elongation);
}

template <class T>
std::shared_ptr<ImplicitFunction<T>>
Reflect(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const size_t& a_reflectPlane)
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  return std::make_shared<ReflectIF<T>>(a_implicitFunction, a_reflectPlane);
}

template <class T>
ComplementIF<T>::ComplementIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);

  m_implicitFunction = a_implicitFunction;
}

template <class T>
T
ComplementIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return -m_implicitFunction->value(a_point);
}

template <class T>
TranslateIF<T>::TranslateIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                            const Vec3T<T>&                             a_translation) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_translation[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_translation[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_translation[2]));

  m_implicitFunction = a_implicitFunction;
  m_shift            = a_translation;
}

template <class T>
T
TranslateIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return m_implicitFunction->value(a_point - m_shift);
}

template <class T>
RotateIF<T>::RotateIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                      const T                                     a_angle,
                      const size_t                                a_axis) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_angle));
  EBGEOMETRY_EXPECT(a_axis <= 2U);

  m_implicitFunction = a_implicitFunction;
  m_angle            = a_angle;
  m_axis             = a_axis;

  const T theta = m_angle * pi<T> / T(180);

  m_cosAngle = std::cos(theta);
  m_sinAngle = std::sin(theta);
}

template <class T>
T
RotateIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

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
  default: {
    EBGEOMETRY_EXPECT(false && "RotateIF: m_axis must be 0, 1, or 2");

    break;
  }
  }

  return m_implicitFunction->value(rotatedPoint);
}

template <class T>
OffsetIF<T>::OffsetIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_offset) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_offset));

  m_implicitFunction = a_implicitFunction;
  m_offset           = a_offset;
}

template <class T>
T
OffsetIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return m_implicitFunction->value(a_point) - m_offset;
}

template <class T>
ScaleIF<T>::ScaleIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_scale) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_scale));
  EBGEOMETRY_EXPECT(a_scale != T(0));

  m_implicitFunction = a_implicitFunction;
  m_scale            = a_scale;
}

template <class T>
T
ScaleIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return (m_implicitFunction->value(a_point / m_scale)) * m_scale;
}

template <class T>
AnnularIF<T>::AnnularIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction, const T a_delta) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_delta));

  m_implicitFunction = a_implicitFunction;
  m_delta            = a_delta;
}

template <class T>
T
AnnularIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return std::abs(m_implicitFunction->value(a_point)) - m_delta;
}

template <class T>
BlurIF<T>::BlurIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                  const T                                     a_blurDistance,
                  const T                                     a_alpha) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_blurDistance));
  EBGEOMETRY_EXPECT(a_blurDistance >= T(0));
  EBGEOMETRY_EXPECT(std::isfinite(a_alpha));
  EBGEOMETRY_EXPECT(a_alpha >= T(0) && a_alpha <= T(1));

  m_implicitFunction = a_implicitFunction;
  m_blurDistance     = a_blurDistance;
  m_alpha            = a_alpha;
}

template <class T>
T
BlurIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  const T A = std::pow(m_alpha, 3.0) * std::pow((1.0 - m_alpha) / 2.0, 0.0);
  const T B = std::pow(m_alpha, 2.0) * std::pow((1.0 - m_alpha) / 2.0, 1.0);
  const T C = std::pow(m_alpha, 1.0) * std::pow((1.0 - m_alpha) / 2.0, 2.0);
  const T D = std::pow(m_alpha, 0.0) * std::pow((1.0 - m_alpha) / 2.0, 3.0);

  const Vec3T<T>& p = a_point;
  const Vec3T<T>  x = m_blurDistance * Vec3T<T>::unit(0);
  const Vec3T<T>  y = m_blurDistance * Vec3T<T>::unit(1);
  const Vec3T<T>  z = m_blurDistance * Vec3T<T>::unit(2);

  const auto& f = *m_implicitFunction;

  T value = 0.0;

  value += A * f(p);
  value += B * (f(p + x) + f(p - x) + f(p + y) + f(p - y) + f(p + z) + f(p - z));
  value += C * (f(p + x + y) + f(p + x - y) + f(p - x + y) + f(p - x - y));
  value += C * (f(p + x + z) + f(p + x - z) + f(p - x + z) + f(p - x - z));
  value += C * (f(p + y + z) + f(p + y - z) + f(p - y + z) + f(p - y - z));
  value += D * (f(p + x + y + z) + f(p + x + y - z) + f(p + x - y + z) + f(p + x - y - z));
  value += D * (f(p - x + y + z) + f(p - x + y - z) + f(p - x - y + z) + f(p - x - y - z));

  return value;
}

template <class T>
MollifyIF<T>::MollifyIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                        const std::shared_ptr<ImplicitFunction<T>>& a_mollifier,
                        const T                                     a_maxValue,
                        const size_t                                a_numPoints) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(a_mollifier != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_maxValue));

  m_implicitFunction = a_implicitFunction;
  m_mollifier        = a_mollifier;

  const T maxVal = std::abs(a_maxValue);

  if (maxVal > 0.0 && a_numPoints > 1) {
    const T dX = 2 * maxVal / (a_numPoints - 1);

    for (size_t i = 0; i < a_numPoints; i++) {
      for (size_t j = 0; j < a_numPoints; j++) {
        for (size_t k = 0; k < a_numPoints; k++) {
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

    EBGEOMETRY_EXPECT(mollifierSum != T(0));

    for (auto& w : m_sampledMollifier) {
      w.second /= mollifierSum;
    }
  }
  else {
    m_sampledMollifier.emplace_back(Vec3T<T>::zeros(), 1.0);
  }
}

template <class T>
T
MollifyIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
  EBGEOMETRY_EXPECT(!m_sampledMollifier.empty());

  T ret = 0.0;

  for (const auto& mollifier : m_sampledMollifier) {
    ret += m_implicitFunction->value(a_point - mollifier.first) * mollifier.second;
  }

  return ret;
}

template <class T>
ElongateIF<T>::ElongateIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                          const Vec3T<T>&                             a_elongation) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_elongation[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_elongation[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_elongation[2]));
  EBGEOMETRY_EXPECT(a_elongation[0] >= T(0));
  EBGEOMETRY_EXPECT(a_elongation[1] >= T(0));
  EBGEOMETRY_EXPECT(a_elongation[2] >= T(0));

  m_implicitFunction = a_implicitFunction;
  m_elongation       = a_elongation;
}

template <class T>
T
ElongateIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return m_implicitFunction->value(a_point - clamp(a_point, -m_elongation, m_elongation));
}

template <class T>
ReflectIF<T>::ReflectIF(const std::shared_ptr<ImplicitFunction<T>>& a_implicitFunction,
                        const size_t&                               a_reflectPlane) noexcept
{
  EBGEOMETRY_EXPECT(a_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(a_reflectPlane <= 2U);

  m_implicitFunction = a_implicitFunction;
  m_reflectParams    = Vec3T<T>::ones();

  if (a_reflectPlane <= 2) {
    m_reflectParams[a_reflectPlane] = -1;
  }
}

template <class T>
T
ReflectIF<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(m_implicitFunction != nullptr);
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return m_implicitFunction->value(a_point * m_reflectParams);
}

} // namespace EBGeometry

#endif
