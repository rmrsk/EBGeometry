// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHQueryImplem.hpp
 * @brief  Implementation of EBGeometry_BVHQuery.hpp.
 * @details DRAFT (bvh_traversal, #230): placeholder bodies, so the interface can be reviewed. Each says
 * what the real one will do.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHQUERYIMPLEM_HPP
#define EBGEOMETRY_BVHQUERYIMPLEM_HPP

// Std includes
#include <type_traits>

#if (defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX512F__)) && !defined(EBGEOMETRY_DEVICE_COMPILE)
#include <immintrin.h>
#endif

// Our includes
#include "EBGeometry_BVHQuery.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

namespace BVH {

namespace QueryDetail {

/**
 * @brief The squared distance from a point to one child box of a row, on the scalar path.
 * @details Internal helper; not part of the public API. The per-axis clamp and the dx*dx + (dy*dy +
 * dz*dz) association are written out to match the SIMD paths of pointDistances2() exactly.
 * @tparam T Precision.
 * @tparam K Branching factor.
 * @param[in] a_row   The child boxes.
 * @param[in] a_point The point.
 * @param[in] a_slot  Child slot, below K.
 * @return The squared distance (0 inside the box).
 */
template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
pointDistance2(const ChildBoxes<T, K>& a_row, const Vec3T<T>& a_point, const size_t a_slot) noexcept
{
  T delta[3];

  for (size_t dir = 0; dir < 3; dir++) {
    const T p     = a_point[dir];
    const T lower = a_row.m_lo[dir][a_slot] - p;
    const T upper = p - a_row.m_hi[dir][a_slot];

    const T d = (upper > lower) ? upper : lower;

    delta[dir] = (d > T(0.0)) ? d : T(0.0);
  }

  return delta[0] * delta[0] + (delta[1] * delta[1] + delta[2] * delta[2]);
}

/**
 * @brief The squared distances from a point to all K child boxes of a row: the SIMD kernel.
 * @details Internal helper; not part of the public API. Formerly PackedBVH::computeChildDistances2(),
 * moved unchanged. Dispatches at compile time on (T, K) and the compiled ISA: AVX-512F for (double, K=8)
 * and (float, K=16), AVX for (double, K=4), (float, K=8) and (double, K=8) as two 4-wide passes, SSE4.1
 * for (float, K=4), and a scalar loop for every other combination and for device compilation. Every
 * path computes the same quantity in the same association order, so all agree bit for bit (a NaN
 * coordinate counts as inside along that axis), unless the compiler contracts the scalar path into
 * fused multiply-adds, which the intrinsics do not.
 * @tparam T Precision.
 * @tparam K Branching factor.
 * @param[in]  a_row   The child boxes.
 * @param[in]  a_point The point.
 * @param[out] a_dist2 One squared distance per child; needs no alignment (unaligned stores).
 */
template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
inline void
pointDistances2(const ChildBoxes<T, K>& a_soa, const Vec3T<T>& a_point, T* a_dist2) noexcept
{
  // Device code takes the scalar path unconditionally: the x86 ISA macros below may still be
  // defined during nvcc's/hipcc's *host* pass over this same __host__ __device__ function, so the
  // guard has to be on the compilation pass rather than on the intrinsics being available.
#if !defined(EBGEOMETRY_DEVICE_COMPILE)

  // ──────────────────────────────────────────────────────────────────────────────
  // AVX-512F paths: K==8/double and K==16/float.
  //
  // These appear first and return, so on -mavx512f hardware the compiler dead-code-eliminates the
  // corresponding AVX branches below.
  //
  // Alignment: ChildAABBSoA is alignas(sizeof(T)*K), which is 64 bytes for both (K=8, T=double) and
  // (K=16, T=float) -- exactly what _mm512_load_pd / _mm512_load_ps require. The static_asserts
  // catch any mismatch. The *store* into a_dist2 is unaligned (storeu), so callers need not align
  // their output buffer; on an aligned address storeu costs the same as store on every CPU that has
  // AVX-512 at all.
  //
  // Every path computes max(max(p - hi, lo - p), 0) per axis. An x86 max(a, b) is (a > b) ? a : b, so
  // with a NaN it returns b: with the operands in this order, each max is exactly the scalar path's
  // comparison below, and a NaN coordinate gives 0 for that axis on every path.
  //
  // Recommended configurations on AVX-512 hardware:
  //   float  -> K=16, W=16  (one _mm512_load_ps covers all children and one leaf group)
  //   double -> K=8,  W=8   (one _mm512_load_pd covers all children; AVX-512F replaces
  //                          the 2x_mm256_load_pd emulation in the AVX fallback below)
  // ──────────────────────────────────────────────────────────────────────────────
#if defined(__AVX512F__)
  if constexpr (K == 8 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildBoxes<T, K>) == sizeof(T) * K,
                  "ChildBoxes alignment mismatch: _mm512_load_pd requires 64-byte alignment");

    const __m512d px   = _mm512_set1_pd(a_point[0]);
    const __m512d py   = _mm512_set1_pd(a_point[1]);
    const __m512d pz   = _mm512_set1_pd(a_point[2]);
    const __m512d zero = _mm512_setzero_pd();

    const __m512d lo_x = _mm512_load_pd(a_soa.m_lo[0]);
    const __m512d lo_y = _mm512_load_pd(a_soa.m_lo[1]);
    const __m512d lo_z = _mm512_load_pd(a_soa.m_lo[2]);
    const __m512d hi_x = _mm512_load_pd(a_soa.m_hi[0]);
    const __m512d hi_y = _mm512_load_pd(a_soa.m_hi[1]);
    const __m512d hi_z = _mm512_load_pd(a_soa.m_hi[2]);

    const __m512d dx = _mm512_max_pd(_mm512_max_pd(_mm512_sub_pd(px, hi_x), _mm512_sub_pd(lo_x, px)), zero);
    const __m512d dy = _mm512_max_pd(_mm512_max_pd(_mm512_sub_pd(py, hi_y), _mm512_sub_pd(lo_y, py)), zero);
    const __m512d dz = _mm512_max_pd(_mm512_max_pd(_mm512_sub_pd(pz, hi_z), _mm512_sub_pd(lo_z, pz)), zero);
    const __m512d d2 =
      _mm512_add_pd(_mm512_mul_pd(dx, dx), _mm512_add_pd(_mm512_mul_pd(dy, dy), _mm512_mul_pd(dz, dz)));

    _mm512_storeu_pd(a_dist2, d2);

    return;
  }

  if constexpr (K == 16 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildBoxes<T, K>) == sizeof(T) * K,
                  "ChildBoxes alignment mismatch: _mm512_load_ps requires 64-byte alignment");

    const __m512 px   = _mm512_set1_ps((float)a_point[0]);
    const __m512 py   = _mm512_set1_ps((float)a_point[1]);
    const __m512 pz   = _mm512_set1_ps((float)a_point[2]);
    const __m512 zero = _mm512_setzero_ps();

    const __m512 lo_x = _mm512_load_ps(a_soa.m_lo[0]);
    const __m512 lo_y = _mm512_load_ps(a_soa.m_lo[1]);
    const __m512 lo_z = _mm512_load_ps(a_soa.m_lo[2]);
    const __m512 hi_x = _mm512_load_ps(a_soa.m_hi[0]);
    const __m512 hi_y = _mm512_load_ps(a_soa.m_hi[1]);
    const __m512 hi_z = _mm512_load_ps(a_soa.m_hi[2]);

    const __m512 dx = _mm512_max_ps(_mm512_max_ps(_mm512_sub_ps(px, hi_x), _mm512_sub_ps(lo_x, px)), zero);
    const __m512 dy = _mm512_max_ps(_mm512_max_ps(_mm512_sub_ps(py, hi_y), _mm512_sub_ps(lo_y, py)), zero);
    const __m512 dz = _mm512_max_ps(_mm512_max_ps(_mm512_sub_ps(pz, hi_z), _mm512_sub_ps(lo_z, pz)), zero);
    const __m512 d2 = _mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_add_ps(_mm512_mul_ps(dy, dy), _mm512_mul_ps(dz, dz)));

    _mm512_storeu_ps(a_dist2, d2);

    return;
  }
#endif // __AVX512F__

  // ──────────────────────────────────────────────────────────────────────────────
  // AVX paths: K==4/double (single pass), K==8/float (single pass),
  //            K==8/double (two 4-wide passes -- superseded by AVX-512F above).
  // ──────────────────────────────────────────────────────────────────────────────
#if defined(__AVX__)
  if constexpr (K == 4 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildBoxes<T, K>) == sizeof(T) * K,
                  "ChildBoxes alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px   = _mm256_set1_pd(a_point[0]);
    const __m256d py   = _mm256_set1_pd(a_point[1]);
    const __m256d pz   = _mm256_set1_pd(a_point[2]);
    const __m256d zero = _mm256_setzero_pd();

    const __m256d lo_x = _mm256_load_pd(a_soa.m_lo[0]);
    const __m256d lo_y = _mm256_load_pd(a_soa.m_lo[1]);
    const __m256d lo_z = _mm256_load_pd(a_soa.m_lo[2]);
    const __m256d hi_x = _mm256_load_pd(a_soa.m_hi[0]);
    const __m256d hi_y = _mm256_load_pd(a_soa.m_hi[1]);
    const __m256d hi_z = _mm256_load_pd(a_soa.m_hi[2]);

    const __m256d dx = _mm256_max_pd(_mm256_max_pd(_mm256_sub_pd(px, hi_x), _mm256_sub_pd(lo_x, px)), zero);
    const __m256d dy = _mm256_max_pd(_mm256_max_pd(_mm256_sub_pd(py, hi_y), _mm256_sub_pd(lo_y, py)), zero);
    const __m256d dz = _mm256_max_pd(_mm256_max_pd(_mm256_sub_pd(pz, hi_z), _mm256_sub_pd(lo_z, pz)), zero);
    const __m256d d2 =
      _mm256_add_pd(_mm256_mul_pd(dx, dx), _mm256_add_pd(_mm256_mul_pd(dy, dy), _mm256_mul_pd(dz, dz)));

    _mm256_storeu_pd(a_dist2, d2);

    return;
  }

  if constexpr (K == 8 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildBoxes<T, K>) == sizeof(T) * K,
                  "ChildBoxes alignment mismatch: _mm256_load_ps requires 32-byte alignment");

    const __m256 px   = _mm256_set1_ps((float)a_point[0]);
    const __m256 py   = _mm256_set1_ps((float)a_point[1]);
    const __m256 pz   = _mm256_set1_ps((float)a_point[2]);
    const __m256 zero = _mm256_setzero_ps();

    const __m256 lo_x = _mm256_load_ps(a_soa.m_lo[0]);
    const __m256 lo_y = _mm256_load_ps(a_soa.m_lo[1]);
    const __m256 lo_z = _mm256_load_ps(a_soa.m_lo[2]);
    const __m256 hi_x = _mm256_load_ps(a_soa.m_hi[0]);
    const __m256 hi_y = _mm256_load_ps(a_soa.m_hi[1]);
    const __m256 hi_z = _mm256_load_ps(a_soa.m_hi[2]);

    const __m256 dx = _mm256_max_ps(_mm256_max_ps(_mm256_sub_ps(px, hi_x), _mm256_sub_ps(lo_x, px)), zero);
    const __m256 dy = _mm256_max_ps(_mm256_max_ps(_mm256_sub_ps(py, hi_y), _mm256_sub_ps(lo_y, py)), zero);
    const __m256 dz = _mm256_max_ps(_mm256_max_ps(_mm256_sub_ps(pz, hi_z), _mm256_sub_ps(lo_z, pz)), zero);
    const __m256 d2 = _mm256_add_ps(_mm256_mul_ps(dx, dx), _mm256_add_ps(_mm256_mul_ps(dy, dy), _mm256_mul_ps(dz, dz)));

    _mm256_storeu_ps(a_dist2, d2);

    return;
  }

  if constexpr (K == 8 && std::is_same_v<T, double>) {
    static_assert(alignof(ChildBoxes<T, K>) == sizeof(T) * K,
                  "ChildBoxes alignment mismatch: _mm256_load_pd requires 32-byte alignment");

    const __m256d px   = _mm256_set1_pd(a_point[0]);
    const __m256d py   = _mm256_set1_pd(a_point[1]);
    const __m256d pz   = _mm256_set1_pd(a_point[2]);
    const __m256d zero = _mm256_setzero_pd();

    const __m256d lo_x0 = _mm256_load_pd(a_soa.m_lo[0]);
    const __m256d lo_y0 = _mm256_load_pd(a_soa.m_lo[1]);
    const __m256d lo_z0 = _mm256_load_pd(a_soa.m_lo[2]);
    const __m256d hi_x0 = _mm256_load_pd(a_soa.m_hi[0]);
    const __m256d hi_y0 = _mm256_load_pd(a_soa.m_hi[1]);
    const __m256d hi_z0 = _mm256_load_pd(a_soa.m_hi[2]);

    const __m256d dx0 = _mm256_max_pd(_mm256_max_pd(_mm256_sub_pd(px, hi_x0), _mm256_sub_pd(lo_x0, px)), zero);
    const __m256d dy0 = _mm256_max_pd(_mm256_max_pd(_mm256_sub_pd(py, hi_y0), _mm256_sub_pd(lo_y0, py)), zero);
    const __m256d dz0 = _mm256_max_pd(_mm256_max_pd(_mm256_sub_pd(pz, hi_z0), _mm256_sub_pd(lo_z0, pz)), zero);
    const __m256d d2_0 =
      _mm256_add_pd(_mm256_mul_pd(dx0, dx0), _mm256_add_pd(_mm256_mul_pd(dy0, dy0), _mm256_mul_pd(dz0, dz0)));

    const __m256d lo_x1 = _mm256_load_pd(a_soa.m_lo[0] + 4);
    const __m256d lo_y1 = _mm256_load_pd(a_soa.m_lo[1] + 4);
    const __m256d lo_z1 = _mm256_load_pd(a_soa.m_lo[2] + 4);
    const __m256d hi_x1 = _mm256_load_pd(a_soa.m_hi[0] + 4);
    const __m256d hi_y1 = _mm256_load_pd(a_soa.m_hi[1] + 4);
    const __m256d hi_z1 = _mm256_load_pd(a_soa.m_hi[2] + 4);

    const __m256d dx1 = _mm256_max_pd(_mm256_max_pd(_mm256_sub_pd(px, hi_x1), _mm256_sub_pd(lo_x1, px)), zero);
    const __m256d dy1 = _mm256_max_pd(_mm256_max_pd(_mm256_sub_pd(py, hi_y1), _mm256_sub_pd(lo_y1, py)), zero);
    const __m256d dz1 = _mm256_max_pd(_mm256_max_pd(_mm256_sub_pd(pz, hi_z1), _mm256_sub_pd(lo_z1, pz)), zero);
    const __m256d d2_1 =
      _mm256_add_pd(_mm256_mul_pd(dx1, dx1), _mm256_add_pd(_mm256_mul_pd(dy1, dy1), _mm256_mul_pd(dz1, dz1)));

    _mm256_storeu_pd(a_dist2, d2_0);
    _mm256_storeu_pd(a_dist2 + 4, d2_1);

    return;
  }
#endif // __AVX__

#if defined(__SSE4_1__)
  if constexpr (K == 4 && std::is_same_v<T, float>) {
    static_assert(alignof(ChildBoxes<T, K>) == sizeof(T) * K,
                  "ChildBoxes alignment mismatch: _mm_load_ps requires 16-byte alignment");

    const __m128 px   = _mm_set1_ps((float)a_point[0]);
    const __m128 py   = _mm_set1_ps((float)a_point[1]);
    const __m128 pz   = _mm_set1_ps((float)a_point[2]);
    const __m128 zero = _mm_setzero_ps();

    const __m128 lo_x = _mm_load_ps(a_soa.m_lo[0]);
    const __m128 lo_y = _mm_load_ps(a_soa.m_lo[1]);
    const __m128 lo_z = _mm_load_ps(a_soa.m_lo[2]);
    const __m128 hi_x = _mm_load_ps(a_soa.m_hi[0]);
    const __m128 hi_y = _mm_load_ps(a_soa.m_hi[1]);
    const __m128 hi_z = _mm_load_ps(a_soa.m_hi[2]);

    const __m128 dx = _mm_max_ps(_mm_max_ps(_mm_sub_ps(px, hi_x), _mm_sub_ps(lo_x, px)), zero);
    const __m128 dy = _mm_max_ps(_mm_max_ps(_mm_sub_ps(py, hi_y), _mm_sub_ps(lo_y, py)), zero);
    const __m128 dz = _mm_max_ps(_mm_max_ps(_mm_sub_ps(pz, hi_z), _mm_sub_ps(lo_z, pz)), zero);
    const __m128 d2 = _mm_add_ps(_mm_mul_ps(dx, dx), _mm_add_ps(_mm_mul_ps(dy, dy), _mm_mul_ps(dz, dz)));

    _mm_storeu_ps(a_dist2, d2);

    return;
  }
#endif // __SSE4_1__

#endif // !EBGEOMETRY_DEVICE_COMPILE

  // Scalar path: every (T, K) with no compiled ISA path above, and all device code.
  //
  // The per-axis clamp and the dx*dx + (dy*dy + dz*dz) association below are written out so that
  // they match the SIMD paths exactly, and every path produces bit-identical results, a NaN coordinate
  // included, unless the compiler contracts the multiply and add below into a fused multiply-add.

  for (size_t k = 0; k < K; k++) {
    a_dist2[k] = pointDistance2(a_soa, a_point, k);
  }
}

/**
 * @brief One child's squared distance, bit for bit the value pointDistances2() gives it.
 * @details Internal helper; not part of the public API. A device pass has only the scalar path and
 * computes the one child; a host pass recomputes the row through the path that sorted it, since a
 * compiler may contract the scalar expression into fused multiply-adds where the intrinsics do not.
 * @tparam T Precision.
 * @tparam K Branching factor.
 * @param[in] a_row   The child boxes.
 * @param[in] a_point The point.
 * @param[in] a_slot  Child slot, below K.
 * @return The squared distance.
 */
template <class T, size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
pointDistance2InRow(const ChildBoxes<T, K>& a_row, const Vec3T<T>& a_point, const size_t a_slot) noexcept
{
#if defined(EBGEOMETRY_DEVICE_COMPILE)
  return pointDistance2(a_row, a_point, a_slot);
#else
  T dist2[K];

  pointDistances2(a_row, a_point, dist2);

  return dist2[a_slot];
#endif
}

} // namespace QueryDetail

template <class T>
template <size_t K>
EBGEOMETRY_HOST_DEVICE
inline Array<T, K>
PointQuery<T>::keys(const ChildBoxes<T, K>& a_row) const noexcept
{
  Array<T, K> keys;

  QueryDetail::pointDistances2(a_row, m_point, keys.data());

  return keys;
}

template <class T>
template <size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
PointQuery<T>::key(const ChildBoxes<T, K>& a_row, const size_t a_k) const noexcept
{
  EBGEOMETRY_EXPECT(a_k < K);

  return QueryDetail::pointDistance2InRow(a_row, m_point, a_k);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline T
PointQuery<T>::key(const BoundingVolumes::AABBT<T>& a_box) const noexcept
{
  // The same per-axis clamp and association as the child boxes' scalar path.
  const auto& lo = a_box.getLowCorner();
  const auto& hi = a_box.getHighCorner();

  T delta[3];

  for (size_t dir = 0; dir < 3; dir++) {
    const T lower = lo[dir] - m_point[dir];
    const T upper = m_point[dir] - hi[dir];

    const T d = (upper > lower) ? upper : lower;

    delta[dir] = (d > T(0.0)) ? d : T(0.0);
  }

  return delta[0] * delta[0] + (delta[1] * delta[1] + delta[2] * delta[2]);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline RayQuery<T>::RayQuery(const Vec3T<T>& a_origin, const Vec3T<T>& a_direction, const T a_tMin) noexcept
  : m_origin(a_origin),
    m_inverseDirection(T(1) / a_direction[0], T(1) / a_direction[1], T(1) / a_direction[2]),
    m_tMin(a_tMin)
{
  // TODO: EBGEOMETRY_EXPECT a finite origin and a unit direction.
}

template <class T>
template <size_t K>
EBGEOMETRY_HOST_DEVICE
inline Array<T, K>
RayQuery<T>::keys(const ChildBoxes<T, K>& /*a_row*/) const noexcept
{
  // TODO: per axis, t0 = (lo - o) * invD and t1 = (hi - o) * invD across the K children (one SIMD
  // register per axis on the host); entry = max(tMin, min(t0, t1) over the axes), exit = min(max(t0, t1)
  // over the axes); key = entry if entry <= exit, else +infinity. Min/max ordered so a NaN (0 * inf)
  // loses; boxes padded by a few ulps of their extent.
  Array<T, K> keys;

  for (size_t k = 0; k < K; k++) {
    keys[k] = Math::Limits<T>::infinity();
  }

  return keys;
}

template <class T>
template <size_t K>
EBGEOMETRY_HOST_DEVICE
inline T
RayQuery<T>::key(const ChildBoxes<T, K>& /*a_row*/, const size_t /*a_k*/) const noexcept
{
  // TODO: the slab test for child a_k, bit for bit the value keys() gives it.
  return Math::Limits<T>::infinity();
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline T
RayQuery<T>::key(const BoundingVolumes::AABBT<T>& /*a_box*/) const noexcept
{
  // TODO: the slab test for one box.
  return Math::Limits<T>::infinity();
}

} // namespace BVH

} // namespace EBGeometry

#endif
