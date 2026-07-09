// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PointSoAImplem.hpp
 * @brief  Implementation of EBGeometry_PointSoA.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POINTSOAIMPLEM_HPP
#define EBGEOMETRY_POINTSOAIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PointSoA.hpp"

namespace EBGeometry {

template <class T, size_t W, class Meta>
void
PointSoAT<T, W, Meta>::pack(const Point<T, Meta>* points, uint32_t count) noexcept
{
  EBGEOMETRY_EXPECT(points != nullptr);
  EBGEOMETRY_EXPECT(count >= 1U);
  EBGEOMETRY_EXPECT(count <= W);

  m_validCount = count;

  for (uint32_t j = 0; j < W; j++) {
    const uint32_t src = (j < count) ? j : (count - 1U);

    const auto& p   = points[src];
    const auto& pos = p.getPosition();

    m_x[j] = pos[0];
    m_y[j] = pos[1];
    m_z[j] = pos[2];

    m_metaData[j] = p.getMetaData();
  }
}

template <class T, size_t W, class Meta>
T
PointSoAT<T, W, Meta>::getDistance2(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
  EBGEOMETRY_EXPECT(m_validCount >= 1U);
  EBGEOMETRY_EXPECT(m_validCount <= W);

  // Scalar loop. A SIMD dispatch (SSE4.1/AVX/AVX-512F horizontal-min reduction, mirroring
  // TriangleSoAT::signedDistance()'s dispatch shape) is a natural follow-up once this shape is
  // settled -- a point-to-point distance has no inside/outside feature classification to do, so
  // that dispatch would be substantially simpler than TriangleSoAT's, but is not yet implemented.
  T best2 = std::numeric_limits<T>::max();

  for (uint32_t j = 0; j < m_validCount; j++) {
    const T dx = a_point[0] - m_x[j];
    const T dy = a_point[1] - m_y[j];
    const T dz = a_point[2] - m_z[j];

    const T d2 = dx * dx + dy * dy + dz * dz;

    if (d2 < best2) {
      best2 = d2;
    }
  }

  return best2;
}

template <class T, size_t W, class Meta>
T
PointSoAT<T, W, Meta>::getDistance(const Vec3T<T>& a_point) const noexcept
{
  return std::sqrt(this->getDistance2(a_point));
}

template <class T, size_t W, class Meta>
const Meta&
PointSoAT<T, W, Meta>::getMetaData(size_t a_lane) const noexcept
{
  EBGEOMETRY_EXPECT(a_lane < W);
  EBGEOMETRY_EXPECT(m_validCount >= 1U);

  return m_metaData[a_lane];
}

template <class T, size_t W, class Meta>
template <class BV>
BV
PointSoAT<T, W, Meta>::computeBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_validCount >= 1U);
  EBGEOMETRY_EXPECT(m_validCount <= W);

  std::vector<Vec3T<T>> positions;
  positions.reserve(m_validCount);

  for (uint32_t j = 0; j < m_validCount; j++) {
    positions.emplace_back(m_x[j], m_y[j], m_z[j]);
  }

  return BV(positions);
}

} // namespace EBGeometry

#endif
