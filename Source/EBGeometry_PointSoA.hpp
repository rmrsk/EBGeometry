// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PointSoA.hpp
 * @brief  Declaration of a true SoA point-position group for SIMD nearest-neighbor evaluation.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POINTSOA_HPP
#define EBGEOMETRY_POINTSOA_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX512F__)
#include <immintrin.h>
#endif

// Our includes
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
 * @brief True SoA (Structure of Arrays) layout for W point positions, enabling SIMD
 * nearest-neighbor evaluation.
 * @details Deliberately carries positions only -- no metadata, no orientation. A point has no
 * inside/outside notion, so there is no signedDistance() here, only the unsigned
 * getDistance()/getDistance2() (squared, sqrt-free) queries. Metadata, when needed, is carried by
 * the separate PointAoSoA<T, Meta, W> wrapper (see EBGeometry_PointAoSoA.hpp) rather than as a
 * member here, so that a pure position-only nearest-neighbor traversal never has metadata bytes
 * anywhere near its hot data -- not merely unused, but physically absent from this type.
 * @warning This type is over-aligned (up to 64 bytes, for AVX-512F) via alignas. The library's own
 * usage (PackedBVH storing groups inside a std::vector<PointSoAT>) is safe: C++17 mandates that
 * std::allocator respect over-alignment. If you allocate a PointSoAT yourself outside of that path
 * -- a raw `new`, a container with a custom/pre-C++17-style allocator, placement-new into
 * externally-owned storage, or a `malloc`'d buffer -- you are responsible for ensuring the memory
 * is aligned to `alignof(PointSoAT<T, W>)`; nothing in this class enforces or checks that.
 * @tparam T Floating-point precision.
 * @tparam W SIMD width: 4 for SSE (128-bit), 8 for AVX (256-bit float) or AVX-512F (512-bit
 * double), 16 for AVX-512F (512-bit float). Defaults to PointSoA::DefaultWidth<T>(), the width
 * that fills one SIMD register exactly for T on the current target ISA -- note this default is
 * itself different for float and double (see the table above), so PointSoAT<float> and
 * PointSoAT<double> do not, in general, share a width even both left at their defaults.
 */
template <class T, size_t W = PointSoA::DefaultWidth<T>()>
struct PointSoAT
{
  static_assert(W > 0, "W must be positive");
  static_assert(std::is_floating_point_v<T>, "PointSoAT requires a floating-point type T");

public:
  /**
   * @brief Pack count positions from positions[0..count-1] into this SoA group.
   * @details Pads lanes count..W-1 by repeating the last real position so that all W lanes hold
   * valid data and SIMD loads never read uninitialised memory.
   * @param[in] positions Source position array with at least count elements. Must not be null.
   * @param[in] count     Number of valid positions to pack. Must satisfy 1 <= count <= W.
   */
  void
  pack(const Vec3T<T>* positions, uint32_t count) noexcept;

  /**
   * @brief Evaluate the shortest unsigned distance from a_point to the closest position in this
   * group.
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Distance from a_point to the closest valid position in this group.
   */
  [[nodiscard]] T
  getDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Evaluate the shortest squared unsigned distance from a_point to the closest position
   * in this group.
   * @details Avoids the sqrt that getDistance() pays -- prefer this whenever the caller only needs
   * the distance for comparison (e.g. BVH pruning, nearest-neighbor search), not its actual
   * magnitude. Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Squared distance from a_point to the closest valid position in this group.
   */
  [[nodiscard]] T
  getDistance2(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Compute the bounding volume enclosing all valid positions in this group.
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a
   * std::vector<Vec3T<T>> of positions.
   * @return Bounding volume enclosing all m_validCount valid positions.
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
   * @brief Number of valid (non-padded) positions in this group (1..W).
   * @details Zero-initialized so that a default-constructed (not-yet-packed) group reliably fails
   * the EBGEOMETRY_EXPECT(m_validCount >= 1) precondition in getDistance()/getDistance2()/
   * computeBoundingVolume(), rather than reading whatever indeterminate value happened to be there.
   */
  uint32_t m_validCount = 0;
};

} // namespace EBGeometry

#include "EBGeometry_PointSoAImplem.hpp"

#endif
