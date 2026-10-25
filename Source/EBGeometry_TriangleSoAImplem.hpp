// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TriangleSoAImplem.hpp
 * @brief  Implementation of EBGeometry_TriangleSoA.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRIANGLESOAIMPLEM_HPP
#define EBGEOMETRY_TRIANGLESOAIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_SIMD.hpp"
#include "EBGeometry_TriangleDistance.hpp"
#include "EBGeometry_TriangleRay.hpp"
#include "EBGeometry_TriangleSoA.hpp"

namespace EBGeometry {

template <class T, size_t W>
EBGEOMETRY_HOST
void
TriangleSoAT<T, W>::pack(const Triangle<T>* tris, uint32_t count) noexcept
{
  EBGEOMETRY_REQUIRE(tris != nullptr, "TriangleSoAT::pack: the triangle array must not be null");
  EBGEOMETRY_REQUIRE(count >= 1U && count <= W,
                     "TriangleSoAT::pack: the triangle count must be between 1 and %zu (%u)",
                     W,
                     unsigned(count));

  m_validCount = count;

  for (uint32_t j = 0; j < W; j++) {
    const uint32_t src = (j < count) ? j : (count - 1U);

    const auto& tri = tris[src];
    const auto& vp  = tri.getVertexPositions();
    const auto& vn  = tri.getVertexNormals();
    const auto& en  = tri.getEdgeNormals();
    const auto& n   = tri.getNormal();

    for (uint32_t v = 0; v < 3; v++) {
      m_vx[v][j] = vp[v][0];
      m_vy[v][j] = vp[v][1];
      m_vz[v][j] = vp[v][2];
    }

    m_nx[j] = n[0];
    m_ny[j] = n[1];
    m_nz[j] = n[2];

    for (uint32_t v = 0; v < 3; v++) {
      m_vnx[v][j] = vn[v][0];
      m_vny[v][j] = vn[v][1];
      m_vnz[v][j] = vn[v][2];
    }

    for (uint32_t e = 0; e < 3; e++) {
      m_enx[e][j] = en[e][0];
      m_eny[e][j] = en[e][1];
      m_enz[e][j] = en[e][2];
    }
  }
}

template <class T, size_t W>
template <size_t N>
EBGEOMETRY_HOST
void
TriangleSoAT<T, W>::signedDistancesSIMD(const Vec3T<T>& a_p, T* a_distances) const noexcept
{
  // Only the first array in each logical group (m_vx, m_nx, m_vnx, m_enx) carries an explicit
  // alignas in the class declaration: since each group's total size (a multiple of 3*W or W
  // elements) is itself a multiple of the group's required alignment, every other array in the
  // group inherits SIMD-safe alignment for free from sequential, padding-free layout. These
  // static_asserts pin that assumption down so that a future reordering or insertion of a member
  // fails to compile instead of segfaulting on a misaligned SIMD load. offsetof requires a complete
  // type, so this can only be checked here (in a member function body), not inside the class itself.
  using SoA = TriangleSoAT<T, W>;

  static_assert(offsetof(SoA, m_vy) % (W * sizeof(T)) == 0, "m_vy is not SIMD-aligned");
  static_assert(offsetof(SoA, m_vz) % (W * sizeof(T)) == 0, "m_vz is not SIMD-aligned");
  static_assert(offsetof(SoA, m_ny) % (W * sizeof(T)) == 0, "m_ny is not SIMD-aligned");
  static_assert(offsetof(SoA, m_nz) % (W * sizeof(T)) == 0, "m_nz is not SIMD-aligned");
  static_assert(offsetof(SoA, m_vny) % (W * sizeof(T)) == 0, "m_vny is not SIMD-aligned");
  static_assert(offsetof(SoA, m_vnz) % (W * sizeof(T)) == 0, "m_vnz is not SIMD-aligned");
  static_assert(offsetof(SoA, m_eny) % (W * sizeof(T)) == 0, "m_eny is not SIMD-aligned");
  static_assert(offsetof(SoA, m_enz) % (W * sizeof(T)) == 0, "m_enz is not SIMD-aligned");
  static_assert(W % N == 0, "TriangleSoAT: the SIMD lane count must divide W");

  using V = SIMD::Pack<T, N>;

  const TriangleDistance::Vec3V<V> point{V(a_p[0]), V(a_p[1]), V(a_p[2])};

  if constexpr (N == W) {
    // One register. No loop: GCC hoists every load of a one-trip loop to its top and spills them.
    TriangleDistance::signedDistance<V>(LaneSource<V>{this, 0}, point).store(a_distances);
  }
  else {
    // Chunks at or past m_validCount hold only padding (copies of the last triangle), so they are
    // skipped; their lanes are never read.
    for (size_t c = 0; c < m_validCount; c += N) {
      TriangleDistance::signedDistance<V>(LaneSource<V>{this, c}, point).store(a_distances + c);
    }
  }
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
TriangleSoAT<T, W>::signedDistance(const Vec3T<T>& a_p) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_p[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_p[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_p[2]));
  EBGEOMETRY_EXPECT(m_validCount >= 1U);
  EBGEOMETRY_EXPECT(m_validCount <= W);

  constexpr size_t N = SIMD::NativeWidth<T, W>();

  T best    = Math::Limits<T>::max();
  T bestAbs = Math::Limits<T>::max();

  if constexpr (N > 1) {
    alignas(N * sizeof(T)) T distances[W];

    this->template signedDistancesSIMD<N>(a_p, distances);

    for (uint32_t i = 0; i < m_validCount; i++) {
      const T ad = std::abs(distances[i]);

      if (ad < bestAbs) {
        best    = distances[i];
        bestAbs = ad;
      }
    }
  }
  else {
    for (uint32_t i = 0; i < m_validCount; i++) {
      const T d  = this->signedDistanceLane(i, a_p);
      const T ad = std::abs(d);

      if (ad < bestAbs) {
        best    = d;
        bestAbs = ad;
      }
    }
  }

  return best;
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
TriangleSoAT<T, W>::signedDistanceLane(uint32_t a_lane, const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(a_lane < W);

  return TriangleDistance::signedDistance<T>(LaneSource<T>{this, a_lane},
                                             TriangleDistance::Vec3V<T>{a_point[0], a_point[1], a_point[2]});
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
Array<T, W>
TriangleSoAT<T, W>::signedDistances(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));
  EBGEOMETRY_EXPECT(m_validCount >= 1U);
  EBGEOMETRY_EXPECT(m_validCount <= W);

  constexpr size_t N = SIMD::NativeWidth<T, W>();

  Array<T, W> distances{};

  if constexpr (N > 1) {
    alignas(N * sizeof(T)) T lanes[W];

    this->template signedDistancesSIMD<N>(a_point, lanes);

    for (uint32_t i = 0; i < m_validCount; i++) {
      distances[i] = lanes[i];
    }
  }
  else {
    for (uint32_t i = 0; i < m_validCount; i++) {
      distances[i] = this->signedDistanceLane(i, a_point);
    }
  }

  // Padded lanes (m_validCount..W-1) repeat the last real lane's distance, matching pack()'s padding
  // convention so a lane-iterating caller sees no fresh values.
  for (uint32_t i = m_validCount; i < W; i++) {
    distances[i] = distances[m_validCount - 1U];
  }

  return distances;
}

template <class T, size_t W>
template <size_t N>
EBGEOMETRY_HOST
void
TriangleSoAT<T, W>::rayCrossingsSIMD(
  const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, const T a_tMin, const T a_tMax, T* a_crossings) const noexcept
{
  static_assert(W % N == 0, "TriangleSoAT: the SIMD lane count must divide W");

  using V = SIMD::Pack<T, N>;

  const TriangleDistance::Vec3V<V> origin{V(a_origin[0]), V(a_origin[1]), V(a_origin[2])};
  const TriangleDistance::Vec3V<V> direction{V(a_direction[0]), V(a_direction[1]), V(a_direction[2])};

  if constexpr (N == W) {
    TriangleRay::crossing<V>(LaneSource<V>{this, 0}, origin, direction, V(a_tMin), V(a_tMax)).store(a_crossings);
  }
  else {
    for (size_t c = 0; c < m_validCount; c += N) {
      TriangleRay::crossing<V>(LaneSource<V>{this, c}, origin, direction, V(a_tMin), V(a_tMax)).store(a_crossings + c);
    }
  }
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
T
TriangleSoAT<T, W>::rayCrossing(const Vec3T<T>& a_origin,
                                const Vec3T<T>& a_direction,
                                const T         a_tMin,
                                const T         a_tMax,
                                uint32_t&       a_lane) const noexcept
{
  EBGEOMETRY_EXPECT(m_validCount >= 1U);
  EBGEOMETRY_EXPECT(m_validCount <= W);

  constexpr size_t N = SIMD::NativeWidth<T, W>();

  T best = Math::Limits<T>::infinity();

  if constexpr (N > 1) {
    alignas(N * sizeof(T)) T crossings[W];

    this->template rayCrossingsSIMD<N>(a_origin, a_direction, a_tMin, a_tMax, crossings);

    for (uint32_t i = 0; i < m_validCount; i++) {
      if (crossings[i] < best) {
        best   = crossings[i];
        a_lane = i;
      }
    }
  }
  else {
    const TriangleDistance::Vec3V<T> origin{a_origin[0], a_origin[1], a_origin[2]};
    const TriangleDistance::Vec3V<T> direction{a_direction[0], a_direction[1], a_direction[2]};

    for (uint32_t i = 0; i < m_validCount; i++) {
      const T t = TriangleRay::crossing<T>(LaneSource<T>{this, i}, origin, direction, a_tMin, a_tMax);

      if (t < best) {
        best   = t;
        a_lane = i;
      }
    }
  }

  return best;
}

template <class T, size_t W>
EBGEOMETRY_HOST_DEVICE
Vec3T<T>
TriangleSoAT<T, W>::getNormal(const uint32_t a_lane) const noexcept
{
  EBGEOMETRY_EXPECT(a_lane < W);

  return Vec3T<T>(m_nx[a_lane], m_ny[a_lane], m_nz[a_lane]);
}

template <class T, size_t W>
template <class BV>
EBGEOMETRY_HOST_DEVICE
BV
TriangleSoAT<T, W>::computeBoundingVolume() const noexcept
{
  EBGEOMETRY_EXPECT(m_validCount >= 1U);
  EBGEOMETRY_EXPECT(m_validCount <= W);

  // Zeroed, since only the first 3 * m_validCount entries are set, and the compiler cannot see that
  // BV reads no others.
  Vec3T<T> pts[3 * W]{};

  for (uint32_t j = 0; j < m_validCount; j++) {
    pts[3 * j + 0] = Vec3T<T>(m_vx[0][j], m_vy[0][j], m_vz[0][j]);
    pts[3 * j + 1] = Vec3T<T>(m_vx[1][j], m_vy[1][j], m_vz[1][j]);
    pts[3 * j + 2] = Vec3T<T>(m_vx[2][j], m_vy[2][j], m_vz[2][j]);
  }

  return BV(pts, 3 * m_validCount);
}

} // namespace EBGeometry

#endif
