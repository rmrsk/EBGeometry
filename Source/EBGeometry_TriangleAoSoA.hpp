// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TriangleAoSoA.hpp
 * @brief  Declaration of a metadata-carrying wrapper around TriangleSoAT.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRIANGLEAOSOA_HPP
#define EBGEOMETRY_TRIANGLEAOSOA_HPP

// Std includes
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_TriangleSoA.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Metadata-carrying wrapper around a single TriangleSoAT<T, W>.
 * @details Structurally this is an AoSoA (Array of Structures of Arrays): TriangleSoAT<T, W> itself
 * is true SoA (one flat array per coordinate, no per-triangle structure at all), and TriangleAoSoA
 * adds exactly one more member -- a std::array<Meta, W> -- alongside it. The two are never merged or
 * interleaved: the hot signedDistance(const Vec3T<T>&) query is delegated straight through to the
 * embedded TriangleSoAT and never reads m_metaData at all, so a pure signed-distance traversal over
 * TriangleAoSoA-packed leaves touches exactly the same bytes it would touch over bare
 * TriangleSoAT-packed leaves -- metadata is only ever read afterward, once a query already knows
 * which lane (and therefore which triangle) it cares about, via getMetaData() or the
 * metadata-reporting signedDistance(point, Meta&) overload. This is the exact same relationship
 * PointAoSoA<T, Meta, W> has with PointSoAT<T, W>, and it is what lets TriMeshSDF answer "which
 * triangle (and its metadata) is closest" -- the SIMD analogue of MeshSDF::getClosestFaces() -- with
 * no cost to its throughput signedDistance() path.
 * @warning Inherits the embedded TriangleSoAT's over-alignment (up to 64 bytes, for AVX-512F). The
 * library's own usage (PackedBVH storing groups inside a std::vector<TriangleAoSoA>) is safe: C++17
 * mandates that std::allocator respect over-alignment. If you allocate a TriangleAoSoA yourself
 * outside of that path -- a raw `new`, a container with a custom/pre-C++17-style allocator,
 * placement-new into externally-owned storage, or a `malloc`'d buffer -- you are responsible for
 * ensuring the memory is aligned to `alignof(TriangleAoSoA<T, Meta, W>)`.
 * @tparam T    Floating-point precision.
 * @tparam Meta Per-triangle user metadata type stored with each triangle.
 * @tparam W    SIMD width; see TriangleSoAT. Defaults to TriangleSoA::DefaultWidth<T>(), matching
 * TriangleSoAT's own default.
 */
template <class T, class Meta, size_t W = TriangleSoA::DefaultWidth<T>()>
struct TriangleAoSoA
{
  static_assert(W > 0, "W must be positive");
  static_assert(std::is_floating_point_v<T>, "TriangleAoSoA requires a floating-point type T");

public:
  /**
   * @brief Pack a_count triangles from a_triangles[0..a_count-1] into this group.
   * @details Pads lanes a_count..W-1 by repeating the last real triangle (its coordinates *and* its
   * metadata), matching TriangleSoAT::pack()'s own padding convention, so all W lanes hold valid
   * data. The coordinates go into the embedded TriangleSoAT; each triangle's metadata (via
   * Triangle::getMetaData()) goes into the parallel m_metaData array.
   * @param[in] a_triangles Source triangle array with at least a_count elements. Must not be null.
   * @param[in] a_count     Number of valid triangles to pack. Must satisfy 1 <= a_count <= W.
   */
  void
  pack(const Triangle<T, Meta>* a_triangles, uint32_t a_count) noexcept;

  /**
   * @brief Evaluate signed distance from a_point to the closest triangle in this group.
   * @details Delegates entirely to the embedded TriangleSoAT<T, W>; never touches m_metaData, so the
   * metadata array adds no cost to this hot (SIMD) path. See TriangleSoAT::signedDistance().
   * @param[in] a_point Query point. Must be finite.
   * @return Signed distance from a_point to the closest valid triangle.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Evaluate signed distance from a_point to the closest triangle *and* report that
   * triangle's metadata.
   * @details The metadata-retrieving analogue of signedDistance(const Vec3T<T>&): it returns the
   * same minimum-absolute-value signed distance, but also writes the winning triangle's metadata to
   * @p a_closestMeta. Unlike the plain overload it takes the scalar per-lane path
   * (TriangleSoAT::signedDistances()) to recover *which* lane won, then reads that lane's metadata --
   * the extra work the throughput path deliberately avoids (see issue #105).
   * @param[in]  a_point       Query point. Must be finite.
   * @param[out] a_closestMeta Set to the metadata of the winning (minimum-|distance|) triangle.
   * @return Signed distance from a_point to the closest valid triangle.
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point, Meta& a_closestMeta) const noexcept;

  /**
   * @brief Get the metadata for one lane of this group.
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * Mirrors PointAoSoA::getMetaData().
   * @param[in] a_lane Lane index. Must satisfy 0 <= a_lane < W (padded lanes return the last real
   * triangle's metadata -- see pack()).
   * @return Metadata for the triangle at a_lane.
   */
  [[nodiscard]] const Meta&
  getMetaData(size_t a_lane) const noexcept;

  /**
   * @brief Compute the bounding volume enclosing all valid triangles in this group.
   * @details Delegates entirely to the embedded TriangleSoAT<T, W>.
   * @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a
   * std::vector<Vec3T<T>> of vertex positions.
   * @return Bounding volume enclosing all vertices of the valid triangles.
   */
  template <class BV>
  [[nodiscard]] BV
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief The wrapped, coordinate-only SoA block. SIMD-hot: this is all signedDistance() touches.
   */
  TriangleSoAT<T, W> m_triangles;

  /**
   * @brief Per-lane metadata, physically separate from m_triangles. m_metaData[j] = metadata of
   * triangle j. Never read by signedDistance(const Vec3T<T>&).
   */
  std::array<Meta, W> m_metaData;

  /**
   * @brief Number of valid (non-padded) triangles in this group (1..W).
   * @details Zero-initialized so that a default-constructed (not-yet-packed) group reliably fails
   * the EBGEOMETRY_EXPECT(m_validCount >= 1) precondition in getMetaData()/signedDistance(point,
   * Meta&), rather than reading whatever indeterminate value happened to be there. (The plain
   * signedDistance() gets the same protection for free from the embedded TriangleSoAT's own
   * m_validCount.)
   */
  uint32_t m_validCount = 0;
};

} // namespace EBGeometry

#include "EBGeometry_TriangleAoSoAImplem.hpp"

#endif
