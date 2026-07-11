// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PointAoSoAImplem.hpp
 * @brief  Implementation of EBGeometry_PointAoSoA.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POINTAOSOAIMPLEM_HPP
#define EBGEOMETRY_POINTAOSOAIMPLEM_HPP

// Std includes
#include <array>
#include <cstddef>
#include <cstdint>

#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PointAoSoA.hpp"

namespace EBGeometry {

template <class T, class Meta, size_t W>
void
PointAoSoA<T, Meta, W>::pack(const Vec3T<T>* a_positions, const Meta* a_metaData, uint32_t a_count) noexcept
{
  EBGEOMETRY_EXPECT(a_positions != nullptr);
  EBGEOMETRY_EXPECT(a_metaData != nullptr);
  EBGEOMETRY_EXPECT(a_count >= 1U);
  EBGEOMETRY_EXPECT(a_count <= W);

  m_validCount = a_count;

  m_positions.pack(a_positions, a_count);

  // Same padding convention as PointSoAT::pack(): lanes count..W-1 repeat the last real entry.
  for (uint32_t j = 0; j < W; j++) {
    const uint32_t src = (j < a_count) ? j : (a_count - 1U);

    m_metaData[j] = a_metaData[src];
  }
}

template <class T, class Meta, size_t W>
std::array<T, W>
PointAoSoA<T, Meta, W>::getDistances2(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getDistances2(a_point);
}

template <class T, class Meta, size_t W>
std::array<T, W>
PointAoSoA<T, Meta, W>::getDistances(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getDistances(a_point);
}

template <class T, class Meta, size_t W>
T
PointAoSoA<T, Meta, W>::getMinimumDistance2(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getMinimumDistance2(a_point);
}

template <class T, class Meta, size_t W>
T
PointAoSoA<T, Meta, W>::getMinimumDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getMinimumDistance(a_point);
}

template <class T, class Meta, size_t W>
T
PointAoSoA<T, Meta, W>::getMaximumDistance2(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getMaximumDistance2(a_point);
}

template <class T, class Meta, size_t W>
T
PointAoSoA<T, Meta, W>::getMaximumDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getMaximumDistance(a_point);
}

template <class T, class Meta, size_t W>
const Meta&
PointAoSoA<T, Meta, W>::getMetaData(size_t a_lane) const noexcept
{
  EBGEOMETRY_EXPECT(a_lane < W);
  EBGEOMETRY_EXPECT(m_validCount >= 1U);

  return m_metaData[a_lane];
}

template <class T, class Meta, size_t W>
template <class BV>
BV
PointAoSoA<T, Meta, W>::computeBoundingVolume() const noexcept
{
  return m_positions.template computeBoundingVolume<BV>();
}

} // namespace EBGeometry

#endif
