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
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PointSoA.hpp"

namespace EBGeometry {

template <class T, size_t W>
EBGEOMETRY_HOST
void
PointSoAT<T, W>::pack(const Vec3T<T>* a_positions, uint32_t a_count) noexcept
{
  EBGEOMETRY_EXPECT(a_positions != nullptr);
  EBGEOMETRY_EXPECT(a_count >= 1U);
  EBGEOMETRY_EXPECT(a_count <= W);

  // The distance kernels assume finite stored coordinates (getDistances2() only checks the query
  // point). Catch a non-finite input here, at the point where it enters the SoA, rather than as a
  // silent NaN surfacing in a later query.
  for (uint32_t i = 0; i < a_count; i++) {
    EBGEOMETRY_EXPECT(std::isfinite(a_positions[i][0]));
    EBGEOMETRY_EXPECT(std::isfinite(a_positions[i][1]));
    EBGEOMETRY_EXPECT(std::isfinite(a_positions[i][2]));
  }

  m_validCount = a_count;

  for (uint32_t j = 0; j < W; j++) {
    const uint32_t src = (j < a_count) ? j : (a_count - 1U);

    const auto& pos = a_positions[src];

    m_x[j] = pos[0];
    m_y[j] = pos[1];
    m_z[j] = pos[2];
  }
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
std::array<T, W>
PointSoAT<T, W>::getDistances2(const Vec3T<T>& a_point) const noexcept
{
  // Only m_x carries an explicit alignas in the class declaration: since each array's size (a
  // multiple of W elements) is itself a multiple of the required alignment, m_y and m_z inherit
  // SIMD-safe alignment for free from sequential, padding-free layout. Pinned here (offsetof
  // requires a complete type) so a future reordering/insertion fails to compile instead of
  // segfaulting on a misaligned SIMD load -- mirrors TriangleSoAT::signedDistance()'s same checks.
  using SoA = PointSoAT<T, W>;

  static_assert(offsetof(SoA, m_y) % (W * sizeof(T)) == 0, "m_y is not SIMD-aligned");
  static_assert(offsetof(SoA, m_z) % (W * sizeof(T)) == 0, "m_z is not SIMD-aligned");

  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
  EBGEOMETRY_EXPECT(m_validCount >= 1U);
  EBGEOMETRY_EXPECT(m_validCount <= W);

  // Unlike TriangleSoAT::signedDistance(), there is no inside/outside feature classification here
  // -- every branch below is just "squared distance to W points, computed simultaneously, then
  // stored per-lane." The horizontal reductions (min/max) live in the small wrappers below, so this
  // one kernel serves all of them and getDistances2() alike.
#if defined(__AVX512F__)
  if constexpr (W == 16 && std::is_same_v<T, float>) {
    static_assert(alignof(SoA) == W * sizeof(T),
                  "PointSoAT alignment mismatch: _mm512_load_ps requires 64-byte alignment");

    const __m512 px = _mm512_set1_ps(a_point[0]);
    const __m512 py = _mm512_set1_ps(a_point[1]);
    const __m512 pz = _mm512_set1_ps(a_point[2]);

    const __m512 dx = _mm512_sub_ps(px, _mm512_load_ps(m_x));
    const __m512 dy = _mm512_sub_ps(py, _mm512_load_ps(m_y));
    const __m512 dz = _mm512_sub_ps(pz, _mm512_load_ps(m_z));

    const __m512 d2 = _mm512_add_ps(_mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_mul_ps(dy, dy)), _mm512_mul_ps(dz, dz));

    std::array<T, W> distances;
    _mm512_storeu_ps(distances.data(), d2);

    return distances;
  }
  if constexpr (W == 8 && std::is_same_v<T, double>) {
    static_assert(alignof(SoA) == W * sizeof(T),
                  "PointSoAT alignment mismatch: _mm512_load_pd requires 64-byte alignment");

    const __m512d px = _mm512_set1_pd(a_point[0]);
    const __m512d py = _mm512_set1_pd(a_point[1]);
    const __m512d pz = _mm512_set1_pd(a_point[2]);

    const __m512d dx = _mm512_sub_pd(px, _mm512_load_pd(m_x));
    const __m512d dy = _mm512_sub_pd(py, _mm512_load_pd(m_y));
    const __m512d dz = _mm512_sub_pd(pz, _mm512_load_pd(m_z));

    const __m512d d2 =
      _mm512_add_pd(_mm512_add_pd(_mm512_mul_pd(dx, dx), _mm512_mul_pd(dy, dy)), _mm512_mul_pd(dz, dz));

    std::array<T, W> distances;
    _mm512_storeu_pd(distances.data(), d2);

    return distances;
  }
#endif
#if defined(__SSE4_1__)
  if constexpr (W == 4 && std::is_same_v<T, float>) {
    static_assert(alignof(SoA) == W * sizeof(T),
                  "PointSoAT alignment mismatch: _mm_load_ps requires 16-byte alignment");

    const __m128 px = _mm_set1_ps(a_point[0]);
    const __m128 py = _mm_set1_ps(a_point[1]);
    const __m128 pz = _mm_set1_ps(a_point[2]);

    const __m128 dx = _mm_sub_ps(px, _mm_load_ps(m_x));
    const __m128 dy = _mm_sub_ps(py, _mm_load_ps(m_y));
    const __m128 dz = _mm_sub_ps(pz, _mm_load_ps(m_z));

    const __m128 d2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(dx, dx), _mm_mul_ps(dy, dy)), _mm_mul_ps(dz, dz));

    std::array<T, W> distances;
    _mm_storeu_ps(distances.data(), d2);

    return distances;
  }
#endif
#if defined(__AVX__)
  if constexpr (W == 8 && std::is_same_v<T, float>) {
    static_assert(alignof(SoA) == W * sizeof(T),
                  "PointSoAT alignment mismatch: _mm256_load_ps requires 32-byte alignment");

    const __m256 px = _mm256_set1_ps(a_point[0]);
    const __m256 py = _mm256_set1_ps(a_point[1]);
    const __m256 pz = _mm256_set1_ps(a_point[2]);

    const __m256 dx = _mm256_sub_ps(px, _mm256_load_ps(m_x));
    const __m256 dy = _mm256_sub_ps(py, _mm256_load_ps(m_y));
    const __m256 dz = _mm256_sub_ps(pz, _mm256_load_ps(m_z));

    const __m256 d2 = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(dx, dx), _mm256_mul_ps(dy, dy)), _mm256_mul_ps(dz, dz));

    std::array<T, W> distances;
    _mm256_storeu_ps(distances.data(), d2);

    return distances;
  }
  if constexpr (W == 4 && std::is_same_v<T, double>) {
    static_assert(alignof(SoA) == W * sizeof(T),
                  "PointSoAT alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px = _mm256_set1_pd(a_point[0]);
    const __m256d py = _mm256_set1_pd(a_point[1]);
    const __m256d pz = _mm256_set1_pd(a_point[2]);

    const __m256d dx = _mm256_sub_pd(px, _mm256_load_pd(m_x));
    const __m256d dy = _mm256_sub_pd(py, _mm256_load_pd(m_y));
    const __m256d dz = _mm256_sub_pd(pz, _mm256_load_pd(m_z));

    const __m256d d2 =
      _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(dx, dx), _mm256_mul_pd(dy, dy)), _mm256_mul_pd(dz, dz));

    std::array<T, W> distances;
    _mm256_storeu_pd(distances.data(), d2);

    return distances;
  }
  if constexpr (W == 8 && std::is_same_v<T, double>) {
    static_assert(alignof(SoA) == W * sizeof(T),
                  "PointSoAT alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    // AVX has no 512-bit registers, so W=8 doubles is handled as two independent 256-bit (4-wide)
    // passes -- "lo" (lanes 0-3) and "hi" (lanes 4-7) -- run in parallel, mirroring
    // TriangleSoAT::signedDistance()'s own AVX (double, W=8) branch.
    const __m256d px = _mm256_set1_pd(a_point[0]);
    const __m256d py = _mm256_set1_pd(a_point[1]);
    const __m256d pz = _mm256_set1_pd(a_point[2]);

    const __m256d dx_lo = _mm256_sub_pd(px, _mm256_load_pd(m_x));
    const __m256d dx_hi = _mm256_sub_pd(px, _mm256_load_pd(m_x + 4));
    const __m256d dy_lo = _mm256_sub_pd(py, _mm256_load_pd(m_y));
    const __m256d dy_hi = _mm256_sub_pd(py, _mm256_load_pd(m_y + 4));
    const __m256d dz_lo = _mm256_sub_pd(pz, _mm256_load_pd(m_z));
    const __m256d dz_hi = _mm256_sub_pd(pz, _mm256_load_pd(m_z + 4));

    const __m256d d2_lo = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(dx_lo, dx_lo), _mm256_mul_pd(dy_lo, dy_lo)),
                                        _mm256_mul_pd(dz_lo, dz_lo));
    const __m256d d2_hi = _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(dx_hi, dx_hi), _mm256_mul_pd(dy_hi, dy_hi)),
                                        _mm256_mul_pd(dz_hi, dz_hi));

    std::array<T, W> distances;
    _mm256_storeu_pd(distances.data(), d2_lo);
    _mm256_storeu_pd(distances.data() + 4, d2_hi);

    return distances;
  }
#endif

  // Scalar fallback for every (T, W) combination not covered above. Computes all W lanes; padded
  // lanes hold the last real position (see pack()), so their squared distance is a valid duplicate.
  std::array<T, W> distances;

  for (uint32_t j = 0; j < W; j++) {
    const T dx = a_point[0] - m_x[j];
    const T dy = a_point[1] - m_y[j];
    const T dz = a_point[2] - m_z[j];

    distances[j] = dx * dx + dy * dy + dz * dz;
  }

  return distances;
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
std::array<T, W>
PointSoAT<T, W>::getDistances(const Vec3T<T>& a_point) const noexcept
{
  std::array<T, W> distances = this->getDistances2(a_point);
  for (size_t j = 0; j < W; j++) {
    distances[j] = std::sqrt(distances[j]);
  }

  return distances;
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
PointSoAT<T, W>::getMinimumDistance2(const Vec3T<T>& a_point) const noexcept
{
  const std::array<T, W> distances = this->getDistances2(a_point);

  T best2 = std::numeric_limits<T>::max();

  for (uint32_t i = 0; i < m_validCount; i++) {
    best2 = (distances[i] < best2) ? distances[i] : best2;
  }

  return best2;
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
PointSoAT<T, W>::getMinimumDistance(const Vec3T<T>& a_point) const noexcept
{
  return std::sqrt(this->getMinimumDistance2(a_point));
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
PointSoAT<T, W>::getMaximumDistance2(const Vec3T<T>& a_point) const noexcept
{
  const std::array<T, W> distances = this->getDistances2(a_point);

  T best2 = std::numeric_limits<T>::lowest();

  for (uint32_t i = 0; i < m_validCount; i++) {
    best2 = (distances[i] > best2) ? distances[i] : best2;
  }

  return best2;
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
PointSoAT<T, W>::getMaximumDistance(const Vec3T<T>& a_point) const noexcept
{
  return std::sqrt(this->getMaximumDistance2(a_point));
}

template <class T, size_t W>
template <class BV>
EBGEOMETRY_HOST_DEVICE
BV
PointSoAT<T, W>::computeBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_validCount >= 1U);
  EBGEOMETRY_EXPECT(m_validCount <= W);

  Vec3T<T> positions[W];

  for (uint32_t j = 0; j < m_validCount; j++) {
    positions[j] = Vec3T<T>(m_x[j], m_y[j], m_z[j]);
  }

  return BV(positions, m_validCount);
}

} // namespace EBGeometry

#endif
