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
#include <cstddef>
#include <cstdint>

#include "EBGeometry_Array.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PointAoSoA.hpp"

namespace EBGeometry {

template <class T, size_t W>
EBGEOMETRY_HOST
void
PointAoSoA<T, W>::pack(const Vec3T<T>* a_positions, const uint32_t* a_pointIds, uint32_t a_count) noexcept
{
  EBGEOMETRY_REQUIRE(a_positions != nullptr && a_pointIds != nullptr,
                     "PointAoSoA::pack: the position and point id arrays must not be null");
  EBGEOMETRY_REQUIRE(a_count >= 1U && a_count <= W,
                     "PointAoSoA::pack: the point count must be between 1 and %zu (%u)",
                     W,
                     unsigned(a_count));

  m_positions.pack(a_positions, a_count);

  // Same padding convention as PointSoAT::pack(): lanes count..W-1 repeat the last real entry.
  for (uint32_t j = 0; j < W; j++) {
    const uint32_t src = (j < a_count) ? j : (a_count - 1U);

    m_pointIds[j] = a_pointIds[src];
  }
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
Array<T, W>
PointAoSoA<T, W>::getDistances2(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getDistances2(a_point);
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
Array<T, W>
PointAoSoA<T, W>::getDistances(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getDistances(a_point);
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
PointAoSoA<T, W>::getMinimumDistance2(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getMinimumDistance2(a_point);
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
PointAoSoA<T, W>::getMinimumDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getMinimumDistance(a_point);
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
PointAoSoA<T, W>::getMaximumDistance2(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getMaximumDistance2(a_point);
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
PointAoSoA<T, W>::getMaximumDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_positions.getMaximumDistance(a_point);
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
uint32_t
PointAoSoA<T, W>::getPointId(size_t a_lane) const noexcept
{
  EBGEOMETRY_EXPECT(a_lane < W);
  EBGEOMETRY_EXPECT(m_positions.numValid() >= 1U);

  return m_pointIds[a_lane];
}

template <class T, size_t W>
template <class BV>
EBGEOMETRY_HOST_DEVICE
BV
PointAoSoA<T, W>::computeBoundingVolume() const noexcept
{
  return m_positions.template computeBoundingVolume<BV>();
}

} // namespace EBGeometry

#endif
