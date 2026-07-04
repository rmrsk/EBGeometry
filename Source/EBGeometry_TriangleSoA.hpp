// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_TriangleSoA.hpp
  @brief  Declaration of SoA triangle group for SIMD signed-distance evaluation.
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_TRIANGLESOA_HPP
#define EBGEOMETRY_TRIANGLESOA_HPP

#include <cstdint>

#if defined(__SSE4_1__) || defined(__AVX__)
#include <immintrin.h>
#endif

/**
  @def EBGEOMETRY_SOA_DEFAULT_WIDTH
  @brief Default SIMD lane width (number of triangles per SoA group) for the current target ISA.
  @details Set to 16 when @c __AVX512F__ is defined (512-bit ZMM register holds 16 floats),
           8 when @c __AVX__ is defined (256-bit YMM register holds 8 floats),
           4 when only @c __SSE4_1__ is defined (128-bit XMM register holds 4 floats),
           and 4 as a scalar-safe fallback when none of the above is available.
           Used as the default @c W template argument for @c TriangleSoAT, @c TriMeshSDF,
           and @c Parser::readIntoTriangleBVH.
*/
#if defined(__AVX512F__)
#define EBGEOMETRY_SOA_DEFAULT_WIDTH 16
#elif defined(__AVX__)
#define EBGEOMETRY_SOA_DEFAULT_WIDTH 8
#elif defined(__SSE4_1__)
#define EBGEOMETRY_SOA_DEFAULT_WIDTH 4
#else
#define EBGEOMETRY_SOA_DEFAULT_WIDTH 4
#endif

#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/**
  @brief SoA (Structure of Arrays) layout for W triangles, enabling SIMD evaluation.
  @details For T=float and W=4 on SSE4.1 machines, signedDistance() uses packed SSE
  instructions to evaluate all W triangles simultaneously. Otherwise falls back to a
  scalar loop over validCount triangles.
  @tparam T  Floating-point precision.
  @tparam W  SIMD width: 4 for SSE (128-bit float), 8 for AVX (256-bit float).
*/
template <class T, size_t W>
struct TriangleSoAT
{
  static_assert(W > 0, "W must be positive");
  static_assert(std::is_floating_point_v<T>, "TriangleSoAT requires a floating-point type T");

  /**
    @brief x-coordinates of vertex positions. vx[i][j] = x-coord of vertex i for triangle j.
  */
  alignas(W * sizeof(T)) T vx[3][W];

  /**
    @brief y-coordinates of vertex positions. vy[i][j] = y-coord of vertex i for triangle j.
  */
  T vy[3][W];

  /**
    @brief z-coordinates of vertex positions. vz[i][j] = z-coord of vertex i for triangle j.
  */
  T vz[3][W];

  /**
    @brief x-components of face normals. nx[j] = x-component of the face normal for triangle j.
  */
  alignas(W * sizeof(T)) T nx[W];

  /**
    @brief y-components of face normals.
  */
  T ny[W];

  /**
    @brief z-components of face normals.
  */
  T nz[W];

  /**
    @brief x-components of vertex normals. vnx[i][j] = x of vertex normal i for triangle j.
  */
  alignas(W * sizeof(T)) T vnx[3][W];

  /**
    @brief y-components of vertex normals. vny[i][j] = y of vertex normal i for triangle j.
  */
  T vny[3][W];

  /**
    @brief z-components of vertex normals. vnz[i][j] = z of vertex normal i for triangle j.
  */
  T vnz[3][W];

  /**
    @brief x-components of edge normals. enx[i][j] = x of edge normal i for triangle j.
  */
  alignas(W * sizeof(T)) T enx[3][W];

  /**
    @brief y-components of edge normals. eny[i][j] = y of edge normal i for triangle j.
  */
  T eny[3][W];

  /**
    @brief z-components of edge normals. enz[i][j] = z of edge normal i for triangle j.
  */
  T enz[3][W];

  /**
    @brief Number of valid (non-padded) triangles in this group (1..W).
  */
  uint32_t validCount;

  /**
    @brief Pack count triangles from tris[0..count-1] into this SoA group.
    @details Pads lanes count..W-1 by repeating the last real triangle so that all W
    lanes hold valid data and SIMD loads never read uninitialised memory.
    @tparam Meta Triangle meta-data type (forwarded from Triangle<T,Meta>).
    @param[in] tris  Source triangle array with at least count elements.
    @param[in] count Number of valid triangles to pack (1 ≤ count ≤ W).
  */
  template <class Meta>
  void
  pack(const Triangle<T, Meta>* tris, uint32_t count) noexcept;

  /**
    @brief Evaluate signed distance from a_point to the closest triangle in this group.
    @details Returns the signed distance with minimum absolute value among validCount triangles.
    Dispatches to an SSE4.1 packed-float path for (T=float, W=4), to AVX packed-float for
    (T=float, W=8), to AVX packed-double for (T=double, W=4 or W=8), or to a scalar fallback.
    @param[in] a_point Query point.
    @return Signed distance from a_point to the closest valid triangle, with sign determined by
    the outward normal of the nearest feature (face, edge, or vertex).
  */
  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
    @brief Compute the bounding volume enclosing all valid triangles in this group.
    @tparam BV Bounding volume type (e.g. AABBT<T>); must be constructible from a
    std::vector<Vec3T<T>> of vertex positions.
    @return Bounding volume enclosing all vertices of the validCount triangles.
  */
  template <class BV>
  [[nodiscard]] BV
  computeBoundingVolume() const noexcept;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_TriangleSoAImplem.hpp"

#endif
