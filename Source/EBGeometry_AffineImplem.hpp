// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_AffineImplem.hpp
 * @brief  Implementation of EBGeometry_Affine.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_AFFINEIMPLEM_HPP
#define EBGEOMETRY_AFFINEIMPLEM_HPP

// Std includes
#include <cmath>

// Our includes
#include "EBGeometry_Affine.hpp"
#include "EBGeometry_Constants.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"

namespace EBGeometry {

namespace AffineDetail {

/**
 * @brief Clamp a coordinate to plus or minus Math::Limits<T>::max(), so that box arithmetic on an
 * unbounded box stays finite.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_x Coordinate.
 * @return a_x clamped to [-max, max].
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
saturate(const T a_x) noexcept
{
  return Math::clamp(a_x, -Math::Limits<T>::max(), Math::Limits<T>::max());
}

} // namespace AffineDetail

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Affine3T<T>
Affine3T<T>::identity() noexcept
{
  Affine3T map;

  map.m_row0  = Vec3T<T>::unit(0);
  map.m_row1  = Vec3T<T>::unit(1);
  map.m_row2  = Vec3T<T>::unit(2);
  map.m_shift = Vec3T<T>::zeros();

  return map;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Affine3T<T>
Affine3T<T>::translation(const Vec3T<T>& a_shift) noexcept
{
  Affine3T map = identity();

  map.m_shift = a_shift;

  return map;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Affine3T<T>
Affine3T<T>::rotation(const T a_degrees, const size_t a_axis) noexcept
{
  EBGEOMETRY_EXPECT(a_axis <= 2);

  const T theta = a_degrees * pi<T> / T(180);
  const T c     = std::cos(theta);
  const T s     = std::sin(theta);

  Affine3T map = identity();

  switch (a_axis) {
  case 0: {
    map.m_row1 = Vec3T<T>(T(0), c, -s);
    map.m_row2 = Vec3T<T>(T(0), s, c);

    break;
  }
  case 1: {
    map.m_row0 = Vec3T<T>(c, T(0), s);
    map.m_row2 = Vec3T<T>(-s, T(0), c);

    break;
  }
  default: {
    map.m_row0 = Vec3T<T>(c, -s, T(0));
    map.m_row1 = Vec3T<T>(s, c, T(0));

    break;
  }
  }

  return map;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Affine3T<T>
Affine3T<T>::rotation(const T a_degrees, const Vec3T<T>& a_axis) noexcept
{
  EBGEOMETRY_EXPECT(a_axis.length() > T(0));

  const Vec3T<T> n     = a_axis / a_axis.length();
  const T        theta = a_degrees * pi<T> / T(180);
  const T        c     = std::cos(theta);
  const T        s     = std::sin(theta);
  const T        t     = T(1) - c;

  Affine3T map = identity();

  map.m_row0 = Vec3T<T>(c + t * n[0] * n[0], t * n[0] * n[1] - s * n[2], t * n[0] * n[2] + s * n[1]);
  map.m_row1 = Vec3T<T>(t * n[0] * n[1] + s * n[2], c + t * n[1] * n[1], t * n[1] * n[2] - s * n[0]);
  map.m_row2 = Vec3T<T>(t * n[0] * n[2] - s * n[1], t * n[1] * n[2] + s * n[0], c + t * n[2] * n[2]);

  return map;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Affine3T<T>
Affine3T<T>::scaling(const T a_factor) noexcept
{
  Affine3T map = identity();

  map.m_row0 = a_factor * Vec3T<T>::unit(0);
  map.m_row1 = a_factor * Vec3T<T>::unit(1);
  map.m_row2 = a_factor * Vec3T<T>::unit(2);

  return map;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Affine3T<T>
Affine3T<T>::reflection(const size_t a_axis) noexcept
{
  EBGEOMETRY_EXPECT(a_axis <= 2);

  Affine3T map = identity();

  switch (a_axis) {
  case 0: {
    map.m_row0 = -Vec3T<T>::unit(0);

    break;
  }
  case 1: {
    map.m_row1 = -Vec3T<T>::unit(1);

    break;
  }
  default: {
    map.m_row2 = -Vec3T<T>::unit(2);

    break;
  }
  }

  return map;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
Affine3T<T>::operator()(const Vec3T<T>& a_point) const noexcept
{
  return Vec3T<T>(m_row0.dot(a_point) + m_shift[0], m_row1.dot(a_point) + m_shift[1], m_row2.dot(a_point) + m_shift[2]);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Affine3T<T>
Affine3T<T>::then(const Affine3T& a_next) const noexcept
{
  // a_next(this(x)) = An (A x + b) + bn: the rows of An A are the rows of An times A, and the shift is
  // An b + bn.
  const Vec3T<T> col0(m_row0[0], m_row1[0], m_row2[0]);
  const Vec3T<T> col1(m_row0[1], m_row1[1], m_row2[1]);
  const Vec3T<T> col2(m_row0[2], m_row1[2], m_row2[2]);

  Affine3T map;

  map.m_row0  = Vec3T<T>(a_next.m_row0.dot(col0), a_next.m_row0.dot(col1), a_next.m_row0.dot(col2));
  map.m_row1  = Vec3T<T>(a_next.m_row1.dot(col0), a_next.m_row1.dot(col1), a_next.m_row1.dot(col2));
  map.m_row2  = Vec3T<T>(a_next.m_row2.dot(col0), a_next.m_row2.dot(col1), a_next.m_row2.dot(col2));
  map.m_shift = a_next(m_shift);

  return map;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline bool
Affine3T<T>::isAxisAligned() const noexcept
{
  const Vec3T<T>* rows[3] = {&m_row0, &m_row1, &m_row2};

  for (const Vec3T<T>* row : rows) {
    int nonZero = 0;

    for (size_t j = 0; j < 3; j++) {
      nonZero += ((*row)[j] != T(0)) ? 1 : 0;
    }

    if (nonZero > 1) {
      return false;
    }
  }

  return true;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline BoundingVolumes::AABBT<T>
Affine3T<T>::transform(const BoundingVolumes::AABBT<T>& a_box) const noexcept
{
  using BV = BoundingVolumes::AABBT<T>;

  const Vec3T<T>& lo = a_box.getLowCorner();
  const Vec3T<T>& hi = a_box.getHighCorner();

  bool empty     = false;
  bool unbounded = false;

  for (size_t dir = 0; dir < 3; dir++) {
    empty     = empty || !(lo[dir] <= hi[dir]);
    unbounded = unbounded || !(lo[dir] > -Math::Limits<T>::max()) || !(hi[dir] < Math::Limits<T>::max());
  }

  if (empty) {
    return BV();
  }

  const Vec3T<T>* rows[3] = {&m_row0, &m_row1, &m_row2};

  Vec3T<T> newLo;
  Vec3T<T> newHi;

  if (this->isAxisAligned()) {
    // Interval arithmetic along each output axis: an output coordinate depends on at most one input
    // coordinate.
    for (size_t i = 0; i < 3; i++) {
      T a = T(0);
      T b = T(0);

      for (size_t j = 0; j < 3; j++) {
        const T coeff = (*rows[i])[j];

        if (coeff != T(0)) {
          const T x0 = AffineDetail::saturate(coeff * AffineDetail::saturate(lo[j]));
          const T x1 = AffineDetail::saturate(coeff * AffineDetail::saturate(hi[j]));

          a = Math::min(x0, x1);
          b = Math::max(x0, x1);
        }
      }

      newLo[i] = AffineDetail::saturate(a + m_shift[i]);
      newHi[i] = AffineDetail::saturate(b + m_shift[i]);
    }
  }
  else if (unbounded) {
    newLo = -Vec3T<T>::max();
    newHi = Vec3T<T>::max();
  }
  else {
    newLo = Vec3T<T>::infinity();
    newHi = -Vec3T<T>::infinity();

    for (int corner = 0; corner < 8; corner++) {
      const Vec3T<T> x(
        (corner & 1) != 0 ? hi[0] : lo[0], (corner & 2) != 0 ? hi[1] : lo[1], (corner & 4) != 0 ? hi[2] : lo[2]);

      const Vec3T<T> y = (*this)(x);

      newLo = min(newLo, y);
      newHi = max(newHi, y);
    }
  }

  return BV(newLo, newHi);
}

} // namespace EBGeometry

#endif
