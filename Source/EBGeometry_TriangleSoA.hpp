// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TriangleSoA.hpp
 * @brief  Declaration of SoA triangle group for SIMD signed-distance evaluation.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRIANGLESOA_HPP
#define EBGEOMETRY_TRIANGLESOA_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX512F__)
#include <immintrin.h>
#endif

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace for TriangleSoAT-related compile-time utilities.
 */
namespace TriangleSoA {

/**
 * @brief The SoA width W (triangles per TriangleSoAT group) that fills one SIMD register for type T under the compiler's SIMD flags.
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
  static_assert(std::is_floating_point_v<T>, "EBGeometry::TriangleSoA::HostWidth requires a floating-point T");
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

/**
 * @brief The default SoA width W (triangles per TriangleSoAT group): 4, or HostWidth<T>() in a build that defines EBGEOMETRY_HOST_TUNED_DEFAULTS.
 * @details This is the value the library's class templates default to.
 *
 * By default it is 4 for both float and double and never depends on compiler flags, so a type
 * spelled with it has the same layout in a host-only file, in a file compiled with AVX, and in both
 * passes of a CUDA or HIP compile -- which is what lets an object be built on the host and used on a
 * device.
 *
 * A host-only build can define EBGEOMETRY_HOST_TUNED_DEFAULTS to make it return HostWidth<T>()
 * instead, the value that fills one SIMD register under the compiler's flags (8 for float under AVX,
 * for example). The macro must then be defined for every translation unit of the build, all compiled
 * with the same SIMD flags, or the same spelled type has different layouts in different files. It
 * is a compile error in a CUDA or HIP translation unit. EBGeometry's own CMake build defines it when
 * EBGeometry is the top-level project and no GPU backend is enabled.
 *
 * Usage: `size_t W = TriangleSoA::DefaultWidth<T>()` as a template-parameter default.
 * @tparam T Floating-point precision type (float or double).
 * @return 4, or HostWidth<T>() under EBGEOMETRY_HOST_TUNED_DEFAULTS.
 */
template <typename T>
[[nodiscard]] constexpr size_t
DefaultWidth() noexcept
{
  static_assert(std::is_floating_point_v<T>, "EBGeometry::TriangleSoA::DefaultWidth requires a floating-point T");

#if defined(EBGEOMETRY_HOST_TUNED_DEFAULTS)
  return HostWidth<T>();
#else
  return 4;
#endif
}

} // namespace TriangleSoA

/**
 * @brief SoA (Structure of Arrays) layout for W triangles, enabling SIMD evaluation.
 * @details signedDistance() dispatches to a packed intrinsics path for (T,W) combinations that
 * fill a SIMD register exactly — SSE4.1 (float, W=4), AVX (float, W=8; double, W=4 or W=8), or
 * AVX-512F (float, W=16; double, W=8) — evaluating all W triangles simultaneously. Any other
 * (T,W) falls back to a scalar loop over m_validCount triangles.
 * @warning This type is over-aligned (up to 64 bytes, for AVX-512F) via alignas. The library's own
 * usage is safe: PackedBVH stores groups in a Pool-backed PODVector<TriangleSoAT>, which reserves
 * each array at alignof(TriangleSoAT) inside a block aligned to PoolBaseAlign (256 bytes), and the
 * build-time std::vector staging relies on C++17's std::allocator respecting over-alignment. If you allocate a TriangleSoAT yourself outside of that path —
 * a raw `new`, a container with a custom/pre-C++17-style allocator, placement-new into
 * externally-owned storage, or a `malloc`'d buffer — you are responsible for ensuring the memory is
 * aligned to `alignof(TriangleSoAT<T, W>)`; nothing in this class enforces or checks that, and a
 * misaligned SIMD load will crash (or silently read the wrong data on some platforms) rather than
 * failing gracefully.
 * @tparam T  Floating-point precision.
 * @tparam W  SIMD width: 4 for SSE (128-bit), 8 for AVX (256-bit float) or AVX-512F (512-bit
 * double), 16 for AVX-512F (512-bit float).
 */
template <class T, size_t W>
struct TriangleSoAT
{
  static_assert(W > 0, "W must be positive");
  static_assert(std::is_floating_point_v<T>, "TriangleSoAT requires a floating-point type T");

public:
  /**
   * @brief Pack count triangles from tris[0..count-1] into this SoA group.
   * @details Pads lanes count..W-1 by repeating the last real triangle so that all W
   * lanes hold valid data and SIMD loads never read uninitialised memory.
   * @param[in] tris  Source triangle array with at least count elements. Must not be null.
   * @param[in] count Number of valid triangles to pack. Must satisfy 1 <= count <= W.
   */
  EBGEOMETRY_HOST
  void
  pack(const Triangle<T>* tris, uint32_t count) noexcept;

  /**
   * @brief Evaluate signed distance from a_point to the closest triangle in this group.
   * @details Returns the signed distance with minimum absolute value among m_validCount triangles.
   * Dispatches to an SSE4.1 packed-float path for (T=float, W=4), to AVX packed-float for
   * (T=float, W=8), to AVX packed-double for (T=double, W=4 or W=8), to AVX-512F for (T=float,
   * W=16) and (T=double, W=8, preferred over AVX when available), or to a scalar fallback.
   * Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Signed distance from a_point to the closest valid triangle, with sign determined by
   * the outward normal of the nearest feature (face, edge, or vertex).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Signed distances from a_point to every one of the W lane triangles.
   * @details The per-lane analogue of signedDistance(): signedDistance() horizontally reduces the W
   * per-triangle distances to the single minimum-|value| entry, whereas this returns all W of them so
   * a caller can recover *which* lane won -- the piece the SIMD reduction discards. Computed with a
   * scalar per-lane loop rather than the SIMD kernel, since it is used only by the face-id-retrieving
   * (non-throughput) path. All W lanes are filled: padded lanes (m_validCount..W-1) repeat the last
   * real triangle's distance, matching pack()'s padding, so a caller iterating lanes should stop at
   * m_validCount (or de-duplicate). Requires the group to have already been packed via pack()
   * (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Per-lane signed distances, one per W lanes.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  Array<T, W>
  signedDistances(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Compute the bounding volume enclosing all valid triangles in this group.
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a point array (a
   * Vec3T<T> pointer and a count).
   * @return Bounding volume enclosing all vertices of the m_validCount triangles.
   */
  template <class BV>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BV
  computeBoundingVolume() const noexcept;

protected:
  /**
   * @brief Signed distance from a_point to the single triangle stored in lane a_lane.
   * @details The scalar per-lane kernel shared by the scalar fallback of signedDistance() and by
   * signedDistances(). Reads only lane a_lane's coordinates/normals; performs no reduction.
   * @param[in] a_lane  Lane index. Must satisfy 0 <= a_lane < W.
   * @param[in] a_point Query point.
   * @return Signed distance from a_point to lane a_lane's triangle, sign from its nearest feature.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistanceLane(uint32_t a_lane, const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief x-coordinates of vertex positions. m_vx[i][j] = x-coord of vertex i for triangle j.
   */
  alignas(W * sizeof(T)) T m_vx[3][W];

  /**
   * @brief y-coordinates of vertex positions. m_vy[i][j] = y-coord of vertex i for triangle j.
   */
  T m_vy[3][W];

  /**
   * @brief z-coordinates of vertex positions. m_vz[i][j] = z-coord of vertex i for triangle j.
   */
  T m_vz[3][W];

  /**
   * @brief x-components of face normals. m_nx[j] = x-component of the face normal for triangle j.
   */
  alignas(W * sizeof(T)) T m_nx[W];

  /**
   * @brief y-components of face normals.
   */
  T m_ny[W];

  /**
   * @brief z-components of face normals.
   */
  T m_nz[W];

  /**
   * @brief x-components of vertex normals. m_vnx[i][j] = x of vertex normal i for triangle j.
   */
  alignas(W * sizeof(T)) T m_vnx[3][W];

  /**
   * @brief y-components of vertex normals. m_vny[i][j] = y of vertex normal i for triangle j.
   */
  T m_vny[3][W];

  /**
   * @brief z-components of vertex normals. m_vnz[i][j] = z of vertex normal i for triangle j.
   */
  T m_vnz[3][W];

  /**
   * @brief x-components of edge normals. m_enx[i][j] = x of edge normal i for triangle j.
   */
  alignas(W * sizeof(T)) T m_enx[3][W];

  /**
   * @brief y-components of edge normals. m_eny[i][j] = y of edge normal i for triangle j.
   */
  T m_eny[3][W];

  /**
   * @brief z-components of edge normals. m_enz[i][j] = z of edge normal i for triangle j.
   */
  T m_enz[3][W];

  /**
   * @brief Number of valid (non-padded) triangles in this group (1..W).
   * @details Zero-initialized so that a default-constructed (not-yet-packed) group reliably fails
   * the EBGEOMETRY_EXPECT(m_validCount >= 1) precondition in signedDistance() and
   * computeBoundingVolume(), rather than reading whatever indeterminate value happened to be there.
   */
  uint32_t m_validCount = 0;
};

static_assert(std::is_trivially_copyable_v<TriangleSoAT<float, 4>>, "TriangleSoAT<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TriangleSoAT<double, 4>>, "TriangleSoAT<double> must be trivially copyable");

} // namespace EBGeometry

#include "EBGeometry_TriangleSoAImplem.hpp"

#endif
