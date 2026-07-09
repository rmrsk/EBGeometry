// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PointAoSoA.hpp
 * @brief  Declaration of a metadata-carrying wrapper around PointSoAT.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POINTAOSOA_HPP
#define EBGEOMETRY_POINTAOSOA_HPP

// Std includes
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_PointSoA.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Metadata-carrying wrapper around a single PointSoAT<T, W>.
 * @details Structurally this is an AoSoA (Array of Structures of Arrays): PointSoAT<T, W> itself
 * is true SoA (one flat array per coordinate, no per-point structure at all), and PointAoSoA adds
 * exactly one more member -- a std::array<Meta, W> -- alongside it. The two are never merged or
 * interleaved: getDistance()/getDistance2() (delegated straight through to the embedded
 * PointSoAT) never read m_metaData at all, so a pure nearest-neighbor traversal over
 * PointAoSoA-packed leaves touches exactly the same bytes it would touch over bare
 * PointSoAT-packed leaves -- metadata is only ever read afterward, once a query already knows
 * which lane (and therefore which point) won, via getMetaData().
 * @tparam T    Floating-point precision.
 * @tparam Meta User-defined metadata type stored with each point.
 * @tparam W    SIMD width; see PointSoAT. Defaults to PointSoA::DefaultWidth<T>(), matching
 * PointSoAT's own default.
 */
template <class T, class Meta, size_t W = PointSoA::DefaultWidth<T>()>
struct PointAoSoA
{
  static_assert(W > 0, "W must be positive");
  static_assert(std::is_floating_point_v<T>, "PointAoSoA requires a floating-point type T");

public:
  /**
   * @brief Pack count (position, metadata) pairs into this group.
   * @details Pads lanes count..W-1 by repeating the last real position and metadata, matching
   * PointSoAT::pack()'s own padding convention, so all W lanes hold valid data.
   * @param[in] positions Source position array with at least count elements. Must not be null.
   * @param[in] metaData  Source metadata array with at least count elements, same order and
   * length as positions. Must not be null.
   * @param[in] count     Number of valid (position, metadata) pairs to pack. Must satisfy
   * 1 <= count <= W.
   */
  void
  pack(const Vec3T<T>* positions, const Meta* metaData, uint32_t count) noexcept;

  /**
   * @brief Evaluate the shortest unsigned distance from a_point to the closest point in this group.
   * @details Delegates entirely to the embedded PointSoAT<T, W>; never touches m_metaData.
   * @param[in] a_point Query point. Must be finite.
   * @return Distance from a_point to the closest valid point in this group.
   */
  [[nodiscard]] T
  getDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Evaluate the shortest squared unsigned distance from a_point to the closest point in
   * this group.
   * @details Delegates entirely to the embedded PointSoAT<T, W>; never touches m_metaData. Avoids
   * the sqrt that getDistance() pays -- prefer this whenever the caller only needs the distance
   * for comparison, not its actual magnitude.
   * @param[in] a_point Query point. Must be finite.
   * @return Squared distance from a_point to the closest valid point in this group.
   */
  [[nodiscard]] T
  getDistance2(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Get the metadata for one lane of this group.
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_lane Lane index. Must satisfy 0 <= a_lane < W (padded lanes return the last real
   * point's metadata -- see pack()).
   * @return Metadata for the point at a_lane.
   */
  [[nodiscard]] const Meta&
  getMetaData(size_t a_lane) const noexcept;

  /**
   * @brief Compute the bounding volume enclosing all valid points in this group.
   * @details Delegates entirely to the embedded PointSoAT<T, W>.
   * @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a
   * std::vector<Vec3T<T>> of point positions.
   * @return Bounding volume enclosing all valid point positions.
   */
  template <class BV>
  [[nodiscard]] BV
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief The wrapped, position-only SoA block. SIMD-hot: this is all getDistance()/
   * getDistance2() ever touch.
   */
  PointSoAT<T, W> m_positions;

  /**
   * @brief Per-lane metadata, physically separate from m_positions. m_metaData[j] = metadata of
   * point j. Never read by getDistance()/getDistance2().
   */
  std::array<Meta, W> m_metaData;

  /**
   * @brief Number of valid (non-padded) points in this group (1..W).
   * @details Zero-initialized so that a default-constructed (not-yet-packed) group reliably fails
   * the EBGEOMETRY_EXPECT(m_validCount >= 1) precondition in getMetaData(), rather than reading
   * whatever indeterminate value happened to be there. (getDistance()/getDistance2() get the same
   * protection for free from the embedded PointSoAT's own m_validCount.)
   */
  uint32_t m_validCount = 0;
};

} // namespace EBGeometry

#include "EBGeometry_PointAoSoAImplem.hpp"

#endif
