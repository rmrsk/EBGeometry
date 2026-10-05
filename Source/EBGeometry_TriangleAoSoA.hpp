// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TriangleAoSoA.hpp
 * @brief  Declaration of a face-id-carrying wrapper around TriangleSoAT.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRIANGLEAOSOA_HPP
#define EBGEOMETRY_TRIANGLEAOSOA_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_TriangleSoA.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Face-id-carrying wrapper around a single TriangleSoAT<T, W>.
 * @details Structurally this is an AoSoA (Array of Structures of Arrays): TriangleSoAT<T, W> itself
 * is true SoA (one flat array per coordinate, no per-triangle structure at all), and TriangleAoSoA
 * adds exactly one more member -- an Array<uint32_t, W> of face ids -- alongside it. The two are
 * never merged or interleaved: the hot signedDistance(const Vec3T<T>&) query is delegated straight
 * through to the embedded TriangleSoAT and never reads m_faceIds at all, so a pure signed-distance
 * traversal over TriangleAoSoA-packed leaves touches exactly the same bytes it would touch over bare
 * TriangleSoAT-packed leaves. The face ids are read only afterward, once a query already knows which
 * lane (and therefore which triangle) it cares about, via getFaceId() or the id-reporting
 * signedDistance(point, uint32_t&) overload -- so a caller can recover which face is closest at no
 * cost to the throughput signedDistance() path.
 * @warning Inherits the embedded TriangleSoAT's over-alignment (up to 64 bytes, for AVX-512F). The
 * library's own usage (PackedBVH storing groups in a Pool-backed PODVector<TriangleAoSoA>, reserved
 * at alignof(TriangleAoSoA) inside a PoolBaseAlign-aligned block) is safe. If you allocate a TriangleAoSoA yourself
 * outside of that path -- a raw `new`, a container with a custom/pre-C++17-style allocator,
 * placement-new into externally-owned storage, or a `malloc`'d buffer -- you are responsible for
 * ensuring the memory is aligned to `alignof(TriangleAoSoA<T, W>)`.
 * @tparam T    Floating-point precision.
 * @tparam W    SIMD width; see TriangleSoAT. Defaults to TriangleSoA::DefaultWidth<T>(), matching
 * TriangleSoAT's own default.
 */
template <class T, size_t W = TriangleSoA::DefaultWidth<T>()>
struct TriangleAoSoA
{
  static_assert(W > 0, "W must be positive");
  static_assert(std::is_floating_point_v<T>, "TriangleAoSoA requires a floating-point type T");

public:
  /**
   * @brief Pack a_count triangles from a_triangles[0..a_count-1] into this group.
   * @details Pads lanes a_count..W-1 by repeating the last real triangle (its coordinates *and* its
   * face id), matching TriangleSoAT::pack()'s own padding convention, so all W lanes hold valid
   * data. The coordinates go into the embedded TriangleSoAT; each triangle's face id (via
   * Triangle::getFaceId()) goes into the parallel m_faceIds array.
   * @param[in] a_triangles Source triangle array with at least a_count elements. Must not be null.
   * @param[in] a_count     Number of valid triangles to pack. Must satisfy 1 <= a_count <= W.
   */
  EBGEOMETRY_HOST
  void
  pack(const Triangle<T>* a_triangles, uint32_t a_count) noexcept;

  /**
   * @brief Evaluate signed distance from a_point to the closest triangle in this group.
   * @details Delegates entirely to the embedded TriangleSoAT<T, W>; never touches m_faceIds, so the
   * id array adds no cost to this hot (SIMD) path. See TriangleSoAT::signedDistance().
   * @param[in] a_point Query point. Must be finite.
   * @return Signed distance from a_point to the closest valid triangle.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Evaluate signed distance from a_point to the closest triangle *and* report that
   * triangle's face id.
   * @details The id-retrieving analogue of signedDistance(const Vec3T<T>&): it returns the same
   * minimum-absolute-value signed distance, but also writes the winning triangle's face id to
   * @p a_closestFaceId. Unlike the plain overload it takes the scalar per-lane path
   * (TriangleSoAT::signedDistances()) to recover *which* lane won, then reads that lane's id -- the
   * extra work the throughput path deliberately avoids (see issue #105).
   * @param[in]  a_point         Query point. Must be finite.
   * @param[out] a_closestFaceId Set to the face id of the winning (minimum-|distance|) triangle.
   * @return Signed distance from a_point to the closest valid triangle.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point, uint32_t& a_closestFaceId) const noexcept;

  /**
   * @brief Get the face id for one lane of this group.
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_lane Lane index. Must satisfy 0 <= a_lane < W (padded lanes return the last real
   * triangle's id -- see pack()).
   * @return Face id of the triangle at a_lane.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getFaceId(size_t a_lane) const noexcept;

  /**
   * @brief Compute the bounding volume enclosing all valid triangles in this group.
   * @details Delegates entirely to the embedded TriangleSoAT<T, W>.
   * @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a point array (a
   * Vec3T<T> pointer and a count).
   * @return Bounding volume enclosing all vertices of the valid triangles.
   */
  template <class BV>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BV
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief The wrapped, coordinate-only SoA block. SIMD-hot: this is all signedDistance() touches.
   */
  TriangleSoAT<T, W> m_triangles;

  /**
   * @brief Per-lane face ids, physically separate from m_triangles. m_faceIds[j] = face id of
   * triangle j. Never read by signedDistance(const Vec3T<T>&).
   */
  Array<uint32_t, W> m_faceIds;

  /**
   * @brief Number of valid (non-padded) triangles in this group (1..W).
   * @details Zero-initialized so that a default-constructed (not-yet-packed) group reliably fails
   * the EBGEOMETRY_EXPECT(m_validCount >= 1) precondition in getFaceId()/signedDistance(point,
   * uint32_t&), rather than reading whatever indeterminate value happened to be there. (The plain
   * signedDistance() gets the same protection for free from the embedded TriangleSoAT's own
   * m_validCount.)
   */
  uint32_t m_validCount = 0;
};

static_assert(std::is_trivially_copyable_v<TriangleAoSoA<float>>, "TriangleAoSoA<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TriangleAoSoA<double>>, "TriangleAoSoA<double> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_TriangleAoSoAImplem.hpp"

#endif
