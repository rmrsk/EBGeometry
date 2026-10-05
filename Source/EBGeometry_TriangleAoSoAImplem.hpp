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
#include <cmath>
#include <cstddef>
#include <cstdint>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_TriangleAoSoA.hpp"

namespace EBGeometry {

template <class T, size_t W>
EBGEOMETRY_HOST
void
TriangleAoSoA<T, W>::pack(const Triangle<T>* a_triangles, uint32_t a_count) noexcept
{
  EBGEOMETRY_REQUIRE(a_triangles != nullptr, "TriangleAoSoA::pack: the triangle array must not be null");
  EBGEOMETRY_REQUIRE(a_count >= 1U && a_count <= W,
                     "TriangleAoSoA::pack: the triangle count must be between 1 and %zu (%u)",
                     W,
                     unsigned(a_count));

  m_validCount = a_count;

  m_triangles.pack(a_triangles, a_count);

  // Same padding convention as TriangleSoAT::pack(): lanes a_count..W-1 repeat the last real entry.
  // The last real face id is carried forward in a local rather than re-read via a_triangles[a_count
  // - 1]: the outer loop is bounded by the compile-time W and every a_triangles read is guarded by
  // j < a_count, so no index can run past the source array -- which also keeps GCC's -Warray-bounds
  // value analysis from mistaking the a_count >= 1 precondition for a possible a_count - 1 unsigned
  // underflow.
  uint32_t lastFaceId = UINT32_MAX;

  for (uint32_t j = 0; j < W; j++) {
    if (j < a_count) {
      lastFaceId = a_triangles[j].getFaceId();
    }

    m_faceIds[j] = lastFaceId;
  }
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
TriangleAoSoA<T, W>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_triangles.signedDistance(a_point);
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
TriangleAoSoA<T, W>::signedDistance(const Vec3T<T>& a_point, uint32_t& a_closestFaceId) const noexcept
{
  EBGEOMETRY_EXPECT(m_validCount >= 1U);
  EBGEOMETRY_EXPECT(m_validCount <= W);

  const Array<T, W> distances = m_triangles.signedDistances(a_point);

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

  a_closestFaceId = m_faceIds[bestLane];

  return best;
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
uint32_t
TriangleAoSoA<T, W>::getFaceId(size_t a_lane) const noexcept
{
  EBGEOMETRY_EXPECT(a_lane < W);
  EBGEOMETRY_EXPECT(m_validCount >= 1U);

  return m_faceIds[a_lane];
}

template <class T, size_t W>
template <class BV>
EBGEOMETRY_HOST_DEVICE
BV
TriangleAoSoA<T, W>::computeBoundingVolume() const noexcept
{
  return m_triangles.template computeBoundingVolume<BV>();
}

} // namespace EBGeometry

#endif
