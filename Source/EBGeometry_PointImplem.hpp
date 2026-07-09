// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PointImplem.hpp
 * @brief  Implementation of EBGeometry_Point.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POINTIMPLEM_HPP
#define EBGEOMETRY_POINTIMPLEM_HPP

// Std includes
#include <cmath>
#include <type_traits>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Point.hpp"

namespace EBGeometry {

template <class T, class Meta>
Point<T, Meta>::Point(const Vec3T<T>& a_position, const Meta& a_metaData) noexcept
{
  this->setPosition(a_position);
  this->setMetaData(a_metaData);
}

template <class T, class Meta>
void
Point<T, Meta>::setPosition(const Vec3T<T>& a_position) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_position[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_position[2]));

  m_position = a_position;
}

template <class T, class Meta>
void
Point<T, Meta>::setMetaData(const Meta& a_metaData) noexcept
{
  m_metaData = a_metaData;
}

template <class T, class Meta>
const Vec3T<T>&
Point<T, Meta>::getPosition() const noexcept
{
  return m_position;
}

template <class T, class Meta>
const Meta&
Point<T, Meta>::getMetaData() const noexcept
{
  return m_metaData;
}

template <class T, class Meta>
T
Point<T, Meta>::getDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return (a_point - m_position).length();
}

template <class T, class Meta>
T
Point<T, Meta>::getDistance2(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return (a_point - m_position).length2();
}

} // namespace EBGeometry

#endif
