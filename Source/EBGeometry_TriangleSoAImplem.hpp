/* EBGeometry
 * Copyright © 2024 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_TriangleSoAImplem.hpp
  @brief  Implementation of EBGeometry_TriangleSoA.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_TriangleSoAImplem
#define EBGeometry_TriangleSoAImplem

#include <cmath>
#include <limits>
#include <type_traits>

#include "EBGeometry_TriangleSoA.hpp"

namespace EBGeometry {

template <class T, size_t W>
template <class Meta>
void
TriangleSoAT<T, W>::pack(const Triangle<T, Meta>* tris, uint32_t count) noexcept
{
  validCount = count;
  for (uint32_t j = 0; j < W; j++) {
    const uint32_t src = (j < count) ? j : (count - 1U);
    const auto&    tri = tris[src];
    const auto&    vp  = tri.getVertexPositions();
    const auto&    vn  = tri.getVertexNormals();
    const auto&    en  = tri.getEdgeNormals();
    const auto&    n   = tri.getNormal();

    for (uint32_t v = 0; v < 3; v++) {
      vx[v][j] = vp[v][0];
      vy[v][j] = vp[v][1];
      vz[v][j] = vp[v][2];
    }
    nx[j] = n[0];
    ny[j] = n[1];
    nz[j] = n[2];
    for (uint32_t v = 0; v < 3; v++) {
      vnx[v][j] = vn[v][0];
      vny[v][j] = vn[v][1];
      vnz[v][j] = vn[v][2];
    }
    for (uint32_t e = 0; e < 3; e++) {
      enx[e][j] = en[e][0];
      eny[e][j] = en[e][1];
      enz[e][j] = en[e][2];
    }
  }
}

template <class T, size_t W>
T
TriangleSoAT<T, W>::signedDistance(const Vec3T<T>& a_p) const noexcept
{
#if defined(__SSE4_1__)
  if constexpr (W == 4 && std::is_same_v<T, float>) {
    const __m128 zero = _mm_setzero_ps();
    const __m128 one  = _mm_set1_ps(1.0f);
    const __m128 mone = _mm_set1_ps(-1.0f);
    const __m128 two  = _mm_set1_ps(2.0f);

    const __m128 px = _mm_set1_ps(a_p[0]);
    const __m128 py = _mm_set1_ps(a_p[1]);
    const __m128 pz = _mm_set1_ps(a_p[2]);

    const __m128 v0x = _mm_load_ps(vx[0]), v0y = _mm_load_ps(vy[0]), v0z = _mm_load_ps(vz[0]);
    const __m128 v1x = _mm_load_ps(vx[1]), v1y = _mm_load_ps(vy[1]), v1z = _mm_load_ps(vz[1]);
    const __m128 v2x = _mm_load_ps(vx[2]), v2y = _mm_load_ps(vy[2]), v2z = _mm_load_ps(vz[2]);

    // p_i = point - vertex_i
    const __m128 p1x = _mm_sub_ps(px, v0x), p1y = _mm_sub_ps(py, v0y), p1z = _mm_sub_ps(pz, v0z);
    const __m128 p2x = _mm_sub_ps(px, v1x), p2y = _mm_sub_ps(py, v1y), p2z = _mm_sub_ps(pz, v1z);
    const __m128 p3x = _mm_sub_ps(px, v2x), p3y = _mm_sub_ps(py, v2y), p3z = _mm_sub_ps(pz, v2z);

    // Edge vectors: e0=v21, e1=v32, e2=v13
    const __m128 e0x = _mm_sub_ps(v1x, v0x), e0y = _mm_sub_ps(v1y, v0y), e0z = _mm_sub_ps(v1z, v0z);
    const __m128 e1x = _mm_sub_ps(v2x, v1x), e1y = _mm_sub_ps(v2y, v1y), e1z = _mm_sub_ps(v2z, v1z);
    const __m128 e2x = _mm_sub_ps(v0x, v2x), e2y = _mm_sub_ps(v0y, v2y), e2z = _mm_sub_ps(v0z, v2z);

    const __m128 nxv = _mm_load_ps(nx), nyv = _mm_load_ps(ny), nzv = _mm_load_ps(nz);

// 3-component dot and cross product helpers operating on __m128 lanes
#define DOT(ax, ay, az, bx, by, bz) \
  _mm_add_ps(_mm_add_ps(_mm_mul_ps(ax, bx), _mm_mul_ps(ay, by)), _mm_mul_ps(az, bz))
#define CRX(ax, ay, az, bx, by, bz) _mm_sub_ps(_mm_mul_ps(ay, bz), _mm_mul_ps(az, by))
#define CRY(ax, ay, az, bx, by, bz) _mm_sub_ps(_mm_mul_ps(az, bx), _mm_mul_ps(ax, bz))
#define CRZ(ax, ay, az, bx, by, bz) _mm_sub_ps(_mm_mul_ps(ax, by), _mm_mul_ps(ay, bx))

    // Inside-triangle test: sign(dot(edge_i × n, p_{i+1})) for 3 edges, sum >= 2 => inside
    const __m128 s0 = _mm_blendv_ps(mone, one,
      _mm_cmpgt_ps(DOT(CRX(e0x, e0y, e0z, nxv, nyv, nzv),
                       CRY(e0x, e0y, e0z, nxv, nyv, nzv),
                       CRZ(e0x, e0y, e0z, nxv, nyv, nzv), p1x, p1y, p1z), zero));
    const __m128 s1 = _mm_blendv_ps(mone, one,
      _mm_cmpgt_ps(DOT(CRX(e1x, e1y, e1z, nxv, nyv, nzv),
                       CRY(e1x, e1y, e1z, nxv, nyv, nzv),
                       CRZ(e1x, e1y, e1z, nxv, nyv, nzv), p2x, p2y, p2z), zero));
    const __m128 s2 = _mm_blendv_ps(mone, one,
      _mm_cmpgt_ps(DOT(CRX(e2x, e2y, e2z, nxv, nyv, nzv),
                       CRY(e2x, e2y, e2z, nxv, nyv, nzv),
                       CRZ(e2x, e2y, e2z, nxv, nyv, nzv), p3x, p3y, p3z), zero));
    const __m128 inside = _mm_cmpge_ps(_mm_add_ps(_mm_add_ps(s0, s1), s2), two);

    // Face distance: dot(n, p1) — only used for inside triangles
    const __m128 face_d = DOT(nxv, nyv, nzv, p1x, p1y, p1z);

    // Minimum squared distance tracker for edge/vertex region
    __m128 ret2 = DOT(p1x, p1y, p1z, p1x, p1y, p1z);
    __m128 rvx = p1x, rvy = p1y, rvz = p1z;
    __m128 rnx = _mm_load_ps(vnx[0]), rny = _mm_load_ps(vny[0]), rnz = _mm_load_ps(vnz[0]);

    // Conditionally update (ret2, rv, rn) when d2 < ret2
#define UPD(qx, qy, qz, normx, normy, normz)                    \
  {                                                              \
    const __m128 _d2 = DOT(qx, qy, qz, qx, qy, qz);            \
    const __m128 _m  = _mm_cmplt_ps(_d2, ret2);                 \
    ret2 = _mm_min_ps(ret2, _d2);                               \
    rvx  = _mm_blendv_ps(rvx, qx, _m);                          \
    rvy  = _mm_blendv_ps(rvy, qy, _m);                          \
    rvz  = _mm_blendv_ps(rvz, qz, _m);                          \
    rnx  = _mm_blendv_ps(rnx, normx, _m);                       \
    rny  = _mm_blendv_ps(rny, normy, _m);                       \
    rnz  = _mm_blendv_ps(rnz, normz, _m);                       \
  }

    UPD(p2x, p2y, p2z, _mm_load_ps(vnx[1]), _mm_load_ps(vny[1]), _mm_load_ps(vnz[1]));
    UPD(p3x, p3y, p3z, _mm_load_ps(vnx[2]), _mm_load_ps(vny[2]), _mm_load_ps(vnz[2]));

    // Edge projection update: t = dot(p, e)/dot(e,e); if 0<t<1 update with p - t*e
#define EDGE(px_, py_, pz_, ex_, ey_, ez_, enx_, eny_, enz_)                        \
  {                                                                                  \
    const __m128 _t  = _mm_div_ps(DOT(px_, py_, pz_, ex_, ey_, ez_),               \
                                   DOT(ex_, ey_, ez_, ex_, ey_, ez_));               \
    const __m128 _ok = _mm_and_ps(_mm_cmpgt_ps(_t, zero), _mm_cmplt_ps(_t, one));  \
    const __m128 _yx = _mm_sub_ps(px_, _mm_mul_ps(_t, ex_));                        \
    const __m128 _yy = _mm_sub_ps(py_, _mm_mul_ps(_t, ey_));                        \
    const __m128 _yz = _mm_sub_ps(pz_, _mm_mul_ps(_t, ez_));                        \
    const __m128 _d2 = DOT(_yx, _yy, _yz, _yx, _yy, _yz);                          \
    const __m128 _m  = _mm_and_ps(_ok, _mm_cmplt_ps(_d2, ret2));                   \
    ret2 = _mm_blendv_ps(ret2, _d2, _m);                                            \
    rvx  = _mm_blendv_ps(rvx, _yx, _m);                                             \
    rvy  = _mm_blendv_ps(rvy, _yy, _m);                                             \
    rvz  = _mm_blendv_ps(rvz, _yz, _m);                                             \
    rnx  = _mm_blendv_ps(rnx, enx_, _m);                                            \
    rny  = _mm_blendv_ps(rny, eny_, _m);                                            \
    rnz  = _mm_blendv_ps(rnz, enz_, _m);                                            \
  }

    EDGE(p1x, p1y, p1z, e0x, e0y, e0z,
         _mm_load_ps(enx[0]), _mm_load_ps(eny[0]), _mm_load_ps(enz[0]));
    EDGE(p2x, p2y, p2z, e1x, e1y, e1z,
         _mm_load_ps(enx[1]), _mm_load_ps(eny[1]), _mm_load_ps(enz[1]));
    EDGE(p3x, p3y, p3z, e2x, e2y, e2z,
         _mm_load_ps(enx[2]), _mm_load_ps(eny[2]), _mm_load_ps(enz[2]));

#undef DOT
#undef CRX
#undef CRY
#undef CRZ
#undef UPD
#undef EDGE

    // Sign from normal · ret_vec; edge distance = sqrt(ret2) * sign
    const __m128 sgn_mask = _mm_cmpgt_ps(
      _mm_add_ps(_mm_add_ps(_mm_mul_ps(rnx, rvx), _mm_mul_ps(rny, rvy)),
                 _mm_mul_ps(rnz, rvz)),
      zero);
    const __m128 edge_d = _mm_mul_ps(_mm_sqrt_ps(ret2), _mm_blendv_ps(mone, one, sgn_mask));

    // Select face distance for inside lanes, edge distance for outside lanes
    const __m128 result = _mm_blendv_ps(edge_d, face_d, inside);

    // Extract and find minimum |result| over the validCount real triangles
    alignas(16) float r[4];
    _mm_store_ps(r, result);

    float best     = r[0];
    float best_abs = std::abs(r[0]);
    for (uint32_t i = 1; i < validCount; i++) {
      const float a = std::abs(r[i]);
      if (a < best_abs) {
        best     = r[i];
        best_abs = a;
      }
    }
    return best;
  }
#endif

  // Scalar fallback: loop over validCount triangles
  T best     = std::numeric_limits<T>::max();
  T best_abs = std::numeric_limits<T>::max();

  for (uint32_t i = 0; i < validCount; i++) {
    const Vec3T<T> v0{vx[0][i], vy[0][i], vz[0][i]};
    const Vec3T<T> v1{vx[1][i], vy[1][i], vz[1][i]};
    const Vec3T<T> v2{vx[2][i], vy[2][i], vz[2][i]};
    const Vec3T<T> n{nx[i], ny[i], nz[i]};
    const Vec3T<T> vn0{vnx[0][i], vny[0][i], vnz[0][i]};
    const Vec3T<T> vn1{vnx[1][i], vny[1][i], vnz[1][i]};
    const Vec3T<T> vn2{vnx[2][i], vny[2][i], vnz[2][i]};
    const Vec3T<T> en0{enx[0][i], eny[0][i], enz[0][i]};
    const Vec3T<T> en1{enx[1][i], eny[1][i], enz[1][i]};
    const Vec3T<T> en2{enx[2][i], eny[2][i], enz[2][i]};

    auto sgn = [](const T x) -> int { return (x > T(0)) ? 1 : -1; };

    const Vec3T<T> v21 = v1 - v0;
    const Vec3T<T> v32 = v2 - v1;
    const Vec3T<T> v13 = v0 - v2;
    const Vec3T<T> p1  = a_p - v0;
    const Vec3T<T> p2  = a_p - v1;
    const Vec3T<T> p3  = a_p - v2;

    const T ss0 = sgn(dot(v21.cross(n), p1));
    const T ss1 = sgn(dot(v32.cross(n), p2));
    const T ss2 = sgn(dot(v13.cross(n), p3));

    T d;
    if (ss0 + ss1 + ss2 >= T(2)) {
      d = n.dot(p1);
    }
    else {
      T        r2  = p1.dot(p1);
      Vec3T<T> rv  = p1;
      Vec3T<T> rn  = vn0;

      const T p2d = p2.dot(p2);
      if (p2d < r2) { r2 = p2d; rv = p2; rn = vn1; }
      const T p3d = p3.dot(p3);
      if (p3d < r2) { r2 = p3d; rv = p3; rn = vn2; }

      const T t1 = dot(p1, v21) / dot(v21, v21);
      if (t1 > T(0) && t1 < T(1)) {
        const Vec3T<T> y1 = p1 - t1 * v21;
        const T        yd = y1.dot(y1);
        if (yd < r2) { r2 = yd; rv = y1; rn = en0; }
      }
      const T t2 = dot(p2, v32) / dot(v32, v32);
      if (t2 > T(0) && t2 < T(1)) {
        const Vec3T<T> y2 = p2 - t2 * v32;
        const T        yd = y2.dot(y2);
        if (yd < r2) { r2 = yd; rv = y2; rn = en1; }
      }
      const T t3 = dot(p3, v13) / dot(v13, v13);
      if (t3 > T(0) && t3 < T(1)) {
        const Vec3T<T> y3 = p3 - t3 * v13;
        const T        yd = y3.dot(y3);
        if (yd < r2) { r2 = yd; rv = y3; rn = en2; }
      }

      d = std::sqrt(r2) * sgn(rn.dot(rv));
    }

    const T ad = std::abs(d);
    if (ad < best_abs) {
      best     = d;
      best_abs = ad;
    }
  }

  return best;
}

template <class T, size_t W>
template <class BV>
BV
TriangleSoAT<T, W>::computeBoundingVolume() const noexcept
{
  std::vector<Vec3T<T>> pts;
  pts.reserve(3 * validCount);
  for (uint32_t j = 0; j < validCount; j++) {
    pts.emplace_back(vx[0][j], vy[0][j], vz[0][j]);
    pts.emplace_back(vx[1][j], vy[1][j], vz[1][j]);
    pts.emplace_back(vx[2][j], vy[2][j], vz[2][j]);
  }
  return BV(pts);
}

} // namespace EBGeometry

#endif
