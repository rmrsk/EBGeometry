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
#include "EBGeometry_Array.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace for PointSoAT-related compile-time utilities.
 */
namespace PointSoA {

/**
 * @brief The default SoA width W (points per PointSoAT group): 4, for both float and double, in every translation unit.
 * @details This is the value the library's class templates default to. It never depends on
 * compiler flags, so a type spelled with it has the same layout in a host-only file, in a file
 * compiled with AVX, and in both passes of a CUDA or HIP compile -- which is what lets an object be
 * built on the host and used on a device. On the benchmarks in the Sphinx page on configuration
 * options, it is also as fast on the host as the ISA-tuned value.
 *
 * For host-only code, HostWidth<T>() gives the value tuned to the compiler's SIMD flags instead.
 * EBGEOMETRY_HOST_TUNED_DEFAULTS does not change this value: it only widens the triangle groups
 * (TriangleSoA::DefaultWidth), since wider point groups measured no faster.
 * Usage: `size_t W = PointSoA::DefaultWidth<T>()` as a template-parameter default.
 * @tparam T Floating-point precision type (float or double).
 * @return 4.
 */
template <typename T>
[[nodiscard]] constexpr size_t
DefaultWidth() noexcept
{
  static_assert(std::is_floating_point_v<T>, "EBGeometry::PointSoA::DefaultWidth requires a floating-point T");

  return 4;
}

/**
 * @brief The SoA width W (points per PointSoAT group) that fills one SIMD register for type T under the compiler's SIMD flags.
 * @details Opt-in, for host-only code:
 *
 * | ISA       | T=float | T=double |
 * |-----------|---------|----------|
 * | AVX-512F  |   16    |    8     |
 * | AVX       |    8    |    4     |
 * | otherwise |    4    |    4     |
 *
 * The value depends on the flags each translation unit is compiled with, so a type spelled with it
 * can mean different types in different files, and different layouts in the host and device passes
 * of one CUDA or HIP compile. Never use it for a type that is shared with device code or with another
 * translation unit built with different flags; use DefaultWidth<T>() there.
 * @tparam T Floating-point precision type (float or double).
 * @return W for T under the current ISA.
 */
template <typename T>
[[nodiscard]] constexpr size_t
HostWidth() noexcept
{
  static_assert(std::is_floating_point_v<T>, "EBGeometry::PointSoA::HostWidth requires a floating-point T");
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
 * @brief True SoA (Structure of Arrays) layout for W point positions, enabling SIMD distance
 * evaluation against the whole group at once.
 * @details Deliberately carries positions only -- no ids, no orientation. A point has no
 * inside/outside notion, so there is no signedDistance() here, only unsigned distance queries. The
 * one SIMD kernel computes the squared distance from the query point to all W lane positions
 * simultaneously; everything the class exposes is a thin wrapper over it -- getDistances2()/
 * getDistances() return the whole W-lane array, and getMinimumDistance2()/getMaximumDistance2() (and
 * their sqrt counterparts) horizontally reduce it. Point ids, when needed, are carried by the separate
 * PointAoSoA<T, W> wrapper (see EBGeometry_PointAoSoA.hpp) rather than as a member here, so that a
 * pure position-only distance traversal never has id bytes anywhere near its hot data -- not merely
 * unused, but physically absent from this type.
 * @warning This type is over-aligned (up to 64 bytes, for AVX-512F) via alignas. The library's own
 * usage (PackedBVH storing groups in a Pool-backed PODVector, reserved at the group's alignof
 * inside a PoolBaseAlign-aligned block) is safe. If you allocate a PointSoAT yourself outside of that path
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
   * @brief Pack a_count positions from a_positions[0..a_count-1] into this SoA group.
   * @details Pads lanes a_count..W-1 by repeating the last real position so that all W lanes hold
   * valid data and SIMD loads never read uninitialised memory.
   * @param[in] a_positions Source position array with at least a_count elements. Must not be null.
   * @param[in] a_count     Number of valid positions to pack. Must satisfy 1 <= a_count <= W.
   */
  EBGEOMETRY_HOST
  void
  pack(const Vec3T<T>* a_positions, uint32_t a_count) noexcept;

  /**
   * @brief Squared unsigned distances from a_point to every one of the W lane positions.
   * @details This is the group's one SIMD kernel; every other distance query is a wrapper over it.
   * Squared, so sqrt-free -- prefer it (and getMinimumDistance2()/getMaximumDistance2()) whenever the
   * caller only needs distances for comparison, not their actual magnitude. All W lanes are returned:
   * padded lanes (indices m_validCount..W-1) repeat the last real position's squared distance,
   * matching pack()'s padding, so a caller iterating lanes should stop at the real count (or, if it
   * lacks that count, use numValid()).
   * Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Per-lane squared distances, one per W lanes.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Array<T, W>
  getDistances2(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Unsigned distances from a_point to every one of the W lane positions.
   * @details The sqrt of each getDistances2() lane; see it for the padding convention. Prefer
   * getDistances2() when only comparisons are needed.
   * @param[in] a_point Query point. Must be finite.
   * @return Per-lane distances, one per W lanes.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Array<T, W>
  getDistances(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Shortest squared unsigned distance from a_point to the closest position in this group.
   * @details Horizontal minimum over the real lanes of getDistances2(). Requires the group to have
   * already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Squared distance from a_point to the closest valid position in this group.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  getMinimumDistance2(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Shortest unsigned distance from a_point to the closest position in this group.
   * @details The sqrt of getMinimumDistance2(); prefer that when only comparisons are needed.
   * @param[in] a_point Query point. Must be finite.
   * @return Distance from a_point to the closest valid position in this group.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  getMinimumDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Largest squared unsigned distance from a_point to the farthest position in this group.
   * @details Horizontal maximum over the real lanes of getDistances2(). Requires the group to have
   * already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Squared distance from a_point to the farthest valid position in this group.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  getMaximumDistance2(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Largest unsigned distance from a_point to the farthest position in this group.
   * @details The sqrt of getMaximumDistance2(); prefer that when only comparisons are needed.
   * @param[in] a_point Query point. Must be finite.
   * @return Distance from a_point to the farthest valid position in this group.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  getMaximumDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Compute the bounding volume enclosing all valid positions in this group.
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a point array (a
   * Vec3T<T> pointer and a count).
   * @return Bounding volume enclosing all m_validCount valid positions.
   */
  template <class BV>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BV
  computeBoundingVolume() const noexcept;

  /**
   * @brief Number of real (non-padded) positions in this group.
   * @details Lanes 0..numValid()-1 hold the packed positions; the rest repeat the last one. A scan
   * that must see each point once stops here.
   * @return The count passed to pack(), 1..W (0 before pack()).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  numValid() const noexcept
  {
    return m_validCount;
  }

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

static_assert(std::is_trivially_copyable_v<PointSoAT<float>>, "PointSoAT<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PointSoAT<double>>, "PointSoAT<double> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_PointSoAImplem.hpp"

#endif
