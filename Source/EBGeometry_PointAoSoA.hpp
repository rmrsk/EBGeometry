// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PointAoSoA.hpp
 * @brief  Declaration of a PointSoAT group that carries point ids.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POINTAOSOA_HPP
#define EBGEOMETRY_POINTAOSOA_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_PointSoA.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief A PointSoAT<T, W> group that also carries each point's cloud index.
 * @details Structurally this is an AoSoA (Array of Structures of Arrays): PointSoAT<T, W> itself
 * is true SoA (one flat array per coordinate, no per-point structure at all), and PointAoSoA adds
 * exactly one more member -- an Array<uint32_t, W> of point ids -- alongside it. The two are never
 * merged or interleaved: the distance queries (getMinimumDistance2(), getMaximumDistance2(),
 * getDistances2(), and their sqrt forms -- all delegated straight through to the embedded
 * PointSoAT) never read the ids at all, so a pure position-only distance traversal over
 * PointAoSoA-packed leaves touches exactly the same bytes it would touch over bare PointSoAT-packed
 * leaves -- an id is only ever read afterward, once a query already knows which lane (and therefore
 * which point) it cares about, via getPointId(). This is the point counterpart of TriangleAoSoA,
 * which carries face ids the same way.
 * @tparam T Floating-point precision.
 * @tparam W SIMD width; see PointSoAT. Defaults to PointSoA::DefaultWidth<T>(), matching
 * PointSoAT's own default.
 */
template <class T, size_t W = PointSoA::DefaultWidth<T>()>
struct PointAoSoA
{
  static_assert(W > 0, "W must be positive");
  static_assert(std::is_floating_point_v<T>, "PointAoSoA requires a floating-point type T");

public:
  /**
   * @brief Pack a_count (position, point id) pairs into this group.
   * @details Pads lanes a_count..W-1 by repeating the last real position and id, matching
   * PointSoAT::pack()'s own padding convention, so all W lanes hold valid data. Scans that must see
   * each point once stop at numValid().
   * @param[in] a_positions Source position array with at least a_count elements. Must not be null.
   * @param[in] a_pointIds  Source id array (typically cloud indices) with at least a_count elements,
   * same order and length as a_positions. Must not be null.
   * @param[in] a_count     Number of valid (position, id) pairs to pack. Must satisfy
   * 1 <= a_count <= W.
   */
  EBGEOMETRY_HOST
  void
  pack(const Vec3T<T>* a_positions, const uint32_t* a_pointIds, uint32_t a_count) noexcept;

  /**
   * @brief Squared unsigned distances from a_point to every one of the W lane points.
   * @details Delegates entirely to the embedded PointSoAT<T, W>; never touches the ids. Padded
   * lanes repeat the last real point's value; see PointSoAT::getDistances2().
   * @param[in] a_point Query point. Must be finite.
   * @return Per-lane squared distances, one per W lanes.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Array<T, W>
  getDistances2(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Unsigned distances from a_point to every one of the W lane points.
   * @details Delegates to PointSoAT<T, W>::getDistances(); the sqrt of each getDistances2() lane.
   * @param[in] a_point Query point. Must be finite.
   * @return Per-lane distances, one per W lanes.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Array<T, W>
  getDistances(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Shortest squared unsigned distance from a_point to the closest point in this group.
   * @details Delegates entirely to the embedded PointSoAT<T, W>; never touches the ids. Avoids
   * the sqrt that getMinimumDistance() pays -- prefer it whenever the caller only needs the distance
   * for comparison, not its actual magnitude.
   * @param[in] a_point Query point. Must be finite.
   * @return Squared distance from a_point to the closest valid point in this group.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  getMinimumDistance2(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Shortest unsigned distance from a_point to the closest point in this group.
   * @details Delegates to PointSoAT<T, W>::getMinimumDistance(); the sqrt of getMinimumDistance2().
   * @param[in] a_point Query point. Must be finite.
   * @return Distance from a_point to the closest valid point in this group.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  getMinimumDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Largest squared unsigned distance from a_point to the farthest point in this group.
   * @details Delegates entirely to the embedded PointSoAT<T, W>; never touches the ids.
   * @param[in] a_point Query point. Must be finite.
   * @return Squared distance from a_point to the farthest valid point in this group.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  getMaximumDistance2(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Largest unsigned distance from a_point to the farthest point in this group.
   * @details Delegates to PointSoAT<T, W>::getMaximumDistance(); the sqrt of getMaximumDistance2().
   * @param[in] a_point Query point. Must be finite.
   * @return Distance from a_point to the farthest valid point in this group.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  getMaximumDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Get the id of the point in one lane of this group.
   * @details Requires the group to have already been packed via pack().
   * @param[in] a_lane Lane index. Must satisfy 0 <= a_lane < W (padded lanes return the last real
   * point's id -- see pack()).
   * @return The id of the point at a_lane.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  getPointId(size_t a_lane) const noexcept;

  /**
   * @brief Number of real (non-padded) points in this group.
   * @return The count passed to pack(), 1..W (0 before pack()).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  numValid() const noexcept
  {
    return m_positions.numValid();
  }

  /**
   * @brief Compute the bounding volume enclosing all valid points in this group.
   * @details Delegates entirely to the embedded PointSoAT<T, W>.
   * @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a point array (a
   * Vec3T<T> pointer and a count).
   * @return Bounding volume enclosing all valid point positions.
   */
  template <class BV>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BV
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief The wrapped, position-only SoA block. SIMD-hot: this is all the distance queries ever
   * touch. It also holds the valid-point count.
   */
  PointSoAT<T, W> m_positions;

  /**
   * @brief Per-lane point ids, physically separate from m_positions. m_pointIds[j] = id of point j.
   * Never read by the distance queries.
   */
  Array<uint32_t, W> m_pointIds;
};

static_assert(std::is_trivially_copyable_v<PointAoSoA<float>>, "PointAoSoA<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PointAoSoA<double>>, "PointAoSoA<double> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_PointAoSoAImplem.hpp"

#endif
