// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PointSoA.hpp
 * @brief  Declaration of SoA point group for SIMD nearest-neighbor evaluation.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POINTSOA_HPP
#define EBGEOMETRY_POINTSOA_HPP

// Std includes
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX512F__)
#include <immintrin.h>
#endif

// Our includes
#include "EBGeometry_Point.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace for PointSoAT-related compile-time utilities.
 */
namespace PointSoA {

/**
 * @brief Returns the SIMD-optimal SoA width (points per PointSoAT group) for type T on the
 * current target ISA.
 * @details Same mapping as TriangleSoA::DefaultWidth<T>() -- the width that fills one SIMD
 * register exactly. Not a member of PointSoAT itself: that class is templated on W, so a member
 * couldn't be used to compute W's own default.
 *
 * | ISA       | T=float | T=double |
 * |-----------|---------|----------|
 * | AVX-512F  |   16    |    8     |
 * | AVX       |    8    |    4     |
 * | otherwise |    4    |    4     |
 *
 * Usage: `size_t W = EBGeometry::PointSoA::DefaultWidth<T>()` as a template-parameter default.
 * @tparam T Floating-point precision type (float or double).
 * @return Optimal W for the current ISA and T.
 */
template <typename T>
[[nodiscard]] constexpr size_t
DefaultWidth() noexcept
{
  static_assert(std::is_floating_point_v<T>, "EBGeometry::PointSoA::DefaultWidth requires a floating-point T");
#if defined(__AVX512F__)
  if constexpr (std::is_same_v<T, double>) {
    return 8;
  }
  else {
    return 16;
  }
#elif defined(__AVX__)
  if constexpr (std::is_same_v<T, double>) {
    return 4;
  }
  else {
    return 8;
  }
#else
  return 4;
#endif
}

} // namespace PointSoA

/**
 * @brief SoA (Structure of Arrays) layout for W points, enabling SIMD nearest-neighbor evaluation.
 * @details Unlike TriangleSoAT, a point has no orientation, so there is no signedDistance() here
 * -- only the unsigned getDistance()/getDistance2() (squared, sqrt-free) queries make sense for a
 * bare point. Each point's metadata is carried alongside its position (unlike TriangleSoAT, which
 * currently drops per-triangle metadata during packing -- see EBGeometry issue #105), retrievable
 * per lane via getMetaData().
 * @warning This type is over-aligned (up to 64 bytes, for AVX-512F) via alignas. The library's own
 * usage (PackedBVH storing groups inside a std::vector<PointSoAT>) is safe: C++17 mandates that
 * std::allocator respect over-alignment. If you allocate a PointSoAT yourself outside of that path
 * -- a raw `new`, a container with a custom/pre-C++17-style allocator, placement-new into
 * externally-owned storage, or a `malloc`'d buffer -- you are responsible for ensuring the memory
 * is aligned to `alignof(PointSoAT<T, W, Meta>)`; nothing in this class enforces or checks that.
 * @tparam T    Floating-point precision.
 * @tparam W    SIMD width: 4 for SSE (128-bit), 8 for AVX (256-bit float) or AVX-512F (512-bit
 * double), 16 for AVX-512F (512-bit float).
 * @tparam Meta User-defined metadata type stored with each point.
 */
template <class T, size_t W, class Meta>
struct PointSoAT
{
  static_assert(W > 0, "W must be positive");
  static_assert(std::is_floating_point_v<T>, "PointSoAT requires a floating-point type T");

public:
  /**
   * @brief Pack count points from points[0..count-1] into this SoA group.
   * @details Pads lanes count..W-1 by repeating the last real point (position and metadata alike)
   * so that all W lanes hold valid data and SIMD loads never read uninitialised memory.
   * @param[in] points Source point array with at least count elements. Must not be null.
   * @param[in] count  Number of valid points to pack. Must satisfy 1 <= count <= W.
   */
  void
  pack(const Point<T, Meta>* points, uint32_t count) noexcept;

  /**
   * @brief Evaluate the shortest unsigned distance from a_point to the closest point in this group.
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Distance from a_point to the closest valid point in this group.
   */
  [[nodiscard]] T
  getDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Evaluate the shortest squared unsigned distance from a_point to the closest point in
   * this group.
   * @details Avoids the sqrt that getDistance() pays -- prefer this whenever the caller only needs
   * the distance for comparison (e.g. BVH pruning, nearest-neighbor search), not its actual
   * magnitude. Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
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
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a
   * std::vector<Vec3T<T>> of point positions.
   * @return Bounding volume enclosing all m_validCount valid point positions.
   */
  template <class BV>
  [[nodiscard]] BV
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief x-coordinates of point positions. m_x[j] = x-coord of point j.
   */
  alignas(W * sizeof(T)) T m_x[W];

  /**
   * @brief y-coordinates of point positions.
   */
  T m_y[W];

  /**
   * @brief z-coordinates of point positions.
   */
  T m_z[W];

  /**
   * @brief Per-lane metadata. m_metaData[j] = metadata of point j.
   */
  std::array<Meta, W> m_metaData;

  /**
   * @brief Number of valid (non-padded) points in this group (1..W).
   * @details Zero-initialized so that a default-constructed (not-yet-packed) group reliably fails
   * the EBGEOMETRY_EXPECT(m_validCount >= 1) precondition in getDistance()/getDistance2()/
   * computeBoundingVolume(), rather than reading whatever indeterminate value happened to be there.
   */
  uint32_t m_validCount = 0;
};

} // namespace EBGeometry

#include "EBGeometry_PointSoAImplem.hpp"

#endif
