// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SignedDistanceFunctionImplem.hpp
 * @brief  Implementation of EBGeometry_SignedDistanceFunctionImplem.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SIGNEDDISTANCEFUNCTIONIMPLEM_HPP
#define EBGEOMETRY_SIGNEDDISTANCEFUNCTIONIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_SignedDistanceFunction.hpp"

namespace EBGeometry {

template <class T>
inline T
SignedDistanceFunction<T>::value(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return this->signedDistance(a_point);
}

template <class T>
inline Vec3T<T>
SignedDistanceFunction<T>::normal(const Vec3T<T>& a_point, const T& a_delta) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
  EBGEOMETRY_EXPECT(a_delta > T(0));

  Vec3T<T> n = Vec3T<T>::zeros();

  const T id = T(1.) / (2 * a_delta);

  for (size_t dir = 0; dir < 3; dir++) {
    const T hi = this->signedDistance(a_point + a_delta * Vec3T<T>::unit(dir));
    const T lo = this->signedDistance(a_point - a_delta * Vec3T<T>::unit(dir));

    n[dir] = (hi - lo) * id;
  }

  EBGEOMETRY_EXPECT(n.length() > T(0));

  n /= n.length();

  return n;
}

} // namespace EBGeometry

#endif
