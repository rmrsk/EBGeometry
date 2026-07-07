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
#include <vector>

#if defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX512F__)
#include <immintrin.h>
#endif

// Our includes
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace for TriangleSoAT-related compile-time utilities.
 */
namespace TriangleSoA {

/**
 * @brief Returns the SIMD-optimal SoA width (triangles per TriangleSoAT group) for type T on the
 * current target ISA.
 * @details Maps the floating-point type and the compile-time ISA to the W that fills one SIMD
 * register exactly, matching the paths TriangleSoAT::signedDistance() actually implements. Not a
 * member of TriangleSoAT itself: that class is templated on W, so a member couldn't be used to
 * compute W's own default (the same reason BVH::DefaultBranchingRatio<T>() is a free function in
 * namespace BVH rather than a member of TreeBVH).
 *
 * | ISA       | T=float | T=double |
 * |-----------|---------|----------|
 * | AVX-512F  |   16    |    8     |
 * | AVX       |    8    |    4     |
 * | otherwise |    4    |    4     |
 *
 * Usage: `size_t W = EBGeometry::TriangleSoA::DefaultWidth<T>()` as a template-parameter default.
 * @tparam T Floating-point precision type (float or double).
 * @return Optimal W for the current ISA and T.
 */
template <typename T>
[[nodiscard]] constexpr size_t
DefaultWidth() noexcept
{
  static_assert(std::is_floating_point_v<T>, "EBGeometry::TriangleSoA::DefaultWidth requires a floating-point T");
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

} // namespace TriangleSoA

/**
 * @brief SoA (Structure of Arrays) layout for W triangles, enabling SIMD evaluation.
 * @details signedDistance() dispatches to a packed intrinsics path for (T,W) combinations that
 * fill a SIMD register exactly — SSE4.1 (float, W=4), AVX (float, W=8; double, W=4 or W=8), or
 * AVX-512F (float, W=16; double, W=8) — evaluating all W triangles simultaneously. Any other
 * (T,W) falls back to a scalar loop over m_validCount triangles.
 * @warning This type is over-aligned (up to 64 bytes, for AVX-512F) via alignas. The library's own
 * usage (PackedBVH storing groups inside a std::vector<TriangleSoAT>) is safe: C++17 mandates that
 * std::allocator respect over-alignment, which has been verified empirically for every (T,W)
 * combination this class supports. If you allocate a TriangleSoAT yourself outside of that path —
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
   * @tparam Meta Triangle meta-data type (forwarded from Triangle<T,Meta>).
   * @param[in] tris  Source triangle array with at least count elements. Must not be null.
   * @param[in] count Number of valid triangles to pack. Must satisfy 1 <= count <= W.
   */
  template <class Meta>
  void
  pack(const Triangle<T, Meta>* tris, uint32_t count) noexcept;

  /**
   * @brief Evaluate signed distance from a_point to the closest triangle in this group.
   * @details Returns the signed distance with minimum absolute value among m_validCount triangles.
   * Dispatches to an SSE4.1 packed-float path for (T=float, W=4), to AVX packed-float for
   * (T=float, W=8), to AVX packed-double for (T=double, W=4 or W=8), or to a scalar fallback.
   * Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @param[in] a_point Query point. Must be finite.
   * @return Signed distance from a_point to the closest valid triangle, with sign determined by
   * the outward normal of the nearest feature (face, edge, or vertex).
   */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Compute the bounding volume enclosing all valid triangles in this group.
   * @details Requires the group to have already been packed via pack() (1 <= m_validCount <= W).
   * @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a
   * std::vector<Vec3T<T>> of vertex positions.
   * @return Bounding volume enclosing all vertices of the m_validCount triangles.
   */
  template <class BV>
  [[nodiscard]] BV
  computeBoundingVolume() const noexcept;

protected:
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

} // namespace EBGeometry

#include "EBGeometry_TriangleSoAImplem.hpp"

#endif
