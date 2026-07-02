/* EBGeometry
 * Copyright © 2024 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_TriangleSoA.hpp
  @brief  Declaration of SoA triangle group for SIMD signed-distance evaluation.
  @author Robert Marskar
*/

#ifndef EBGeometry_TriangleSoA
#define EBGeometry_TriangleSoA

#include <cstdint>
#if defined(__SSE4_1__)
#include <smmintrin.h>
#endif

#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
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

  /*!
    @brief Vertex positions in SoA layout. vx[i][j] = x-coord of vertex i for triangle j.
  */
  alignas(W * sizeof(T)) T vx[3][W];
  T vy[3][W];
  T vz[3][W];

  /*!
    @brief Face normals in SoA layout.
  */
  alignas(W * sizeof(T)) T nx[W];
  T ny[W];
  T nz[W];

  /*!
    @brief Vertex normals in SoA layout. vnx[i][j] = x of vertex normal i for triangle j.
  */
  alignas(W * sizeof(T)) T vnx[3][W];
  T vny[3][W];
  T vnz[3][W];

  /*!
    @brief Edge normals in SoA layout. enx[i][j] = x of edge normal i for triangle j.
  */
  alignas(W * sizeof(T)) T enx[3][W];
  T eny[3][W];
  T enz[3][W];

  /*!
    @brief Number of valid (non-padded) triangles in this group (1..W).
  */
  uint32_t validCount;

  /*!
    @brief Pack count triangles from tris[0..count-1] into this SoA group.
    @details Pads lanes count..W-1 by repeating the last real triangle.
    @param[in] tris  Source triangle array.
    @param[in] count Number of valid triangles (1..W).
  */
  template <class Meta>
  void
  pack(const Triangle<T, Meta>* tris, uint32_t count) noexcept;

  /*!
    @brief Evaluate signed distance from a_point to the closest triangle in this group.
    @details Returns the signed distance with minimum absolute value among validCount triangles.
    Uses SSE4.1 packed float instructions for T=float, W=4 when __SSE4_1__ is defined;
    otherwise uses a scalar loop.
  */
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /*!
    @brief Compute the bounding volume enclosing all valid triangles in this group.
    @tparam BV Bounding volume type (e.g. AABBT<T>).
  */
  template <class BV>
  BV
  computeBoundingVolume() const noexcept;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_TriangleSoAImplem.hpp"

#endif
