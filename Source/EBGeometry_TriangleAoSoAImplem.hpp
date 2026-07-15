// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TriangleAoSoAImplem.hpp
 * @brief  Implementation of EBGeometry_TriangleAoSoA.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRIANGLEAOSOAIMPLEM_HPP
#define EBGEOMETRY_TRIANGLEAOSOAIMPLEM_HPP

// Std includes
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_TriangleAoSoA.hpp"

namespace EBGeometry {

template <class T, class Meta, size_t W>
void
TriangleAoSoA<T, Meta, W>::pack(const Triangle<T, Meta>* a_triangles, uint32_t a_count) noexcept
{
  EBGEOMETRY_EXPECT(a_triangles != nullptr);
  EBGEOMETRY_EXPECT(a_count >= 1U);
  EBGEOMETRY_EXPECT(a_count <= W);

  m_validCount = a_count;

  m_triangles.pack(a_triangles, a_count);

  // Same padding convention as TriangleSoAT::pack(): lanes a_count..W-1 repeat the last real entry.
  // The last real metadata is carried forward in a local rather than re-read via a_triangles[a_count
  // - 1]: the outer loop is bounded by the compile-time W and every a_triangles read is guarded by
  // j < a_count, so no index can run past the source array -- which also keeps GCC's -Warray-bounds
  // value analysis from mistaking the (assertion-guarded, hence Release-invisible) a_count >= 1
  // precondition for a possible a_count - 1 unsigned underflow.
  Meta lastMeta{};

  for (uint32_t j = 0; j < W; j++) {
    if (j < a_count) {
      lastMeta = a_triangles[j].getMetaData();
    }

    m_metaData[j] = lastMeta;
  }
}

template <class T, class Meta, size_t W>
T
TriangleAoSoA<T, Meta, W>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_triangles.signedDistance(a_point);
}

template <class T, class Meta, size_t W>
T
TriangleAoSoA<T, Meta, W>::signedDistance(const Vec3T<T>& a_point, Meta& a_closestMeta) const noexcept
{
  EBGEOMETRY_EXPECT(m_validCount >= 1U);
  EBGEOMETRY_EXPECT(m_validCount <= W);

  const std::array<T, W> distances = m_triangles.signedDistances(a_point);

  T        best     = distances[0];
  T        bestAbs  = std::abs(distances[0]);
  uint32_t bestLane = 0;

  for (uint32_t i = 1; i < m_validCount; i++) {
    const T ad = std::abs(distances[i]);

    if (ad < bestAbs) {
      best     = distances[i];
      bestAbs  = ad;
      bestLane = i;
    }
  }

  a_closestMeta = m_metaData[bestLane];

  return best;
}

template <class T, class Meta, size_t W>
const Meta&
TriangleAoSoA<T, Meta, W>::getMetaData(size_t a_lane) const noexcept
{
  EBGEOMETRY_EXPECT(a_lane < W);
  EBGEOMETRY_EXPECT(m_validCount >= 1U);

  return m_metaData[a_lane];
}

template <class T, class Meta, size_t W>
template <class BV>
BV
TriangleAoSoA<T, Meta, W>::computeBoundingVolume() const noexcept
{
  return m_triangles.template computeBoundingVolume<BV>();
}

} // namespace EBGeometry

#endif
