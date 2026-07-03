// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_TriangleSoAImplem.hpp
  @brief  Implementation of EBGeometry_TriangleSoA.hpp
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_TRIANGLESOAIMPLEM_HPP
#define EBGEOMETRY_TRIANGLESOAIMPLEM_HPP

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
      static_assert(alignof(TriangleSoAT<T, W>) == W * sizeof(T),
                    "TriangleSoAT alignment mismatch: _mm_load_ps requires 16-byte alignment");
      const __m128 px  = _mm_set1_ps(a_p[0]);
      const __m128 py  = _mm_set1_ps(a_p[1]);
      const __m128 pz  = _mm_set1_ps(a_p[2]);
      const __m128 one = _mm_set1_ps(1.f);
      const __m128 zer = _mm_setzero_ps();

      // v0, v1, v2 vertex positions
      const __m128 v0x = _mm_load_ps(vx[0]);
      const __m128 v0y = _mm_load_ps(vy[0]);
      const __m128 v0z = _mm_load_ps(vz[0]);
      const __m128 v1x = _mm_load_ps(vx[1]);
      const __m128 v1y = _mm_load_ps(vy[1]);
      const __m128 v1z = _mm_load_ps(vz[1]);
      const __m128 v2x = _mm_load_ps(vx[2]);
      const __m128 v2y = _mm_load_ps(vy[2]);
      const __m128 v2z = _mm_load_ps(vz[2]);

      // Face normals
      const __m128 fnx = _mm_load_ps(nx);
      const __m128 fny = _mm_load_ps(ny);
      const __m128 fnz = _mm_load_ps(nz);

      // Edge vectors v21 = v1 - v0, v32 = v2 - v1, v13 = v0 - v2
      const __m128 v21x = _mm_sub_ps(v1x, v0x);
      const __m128 v21y = _mm_sub_ps(v1y, v0y);
      const __m128 v21z = _mm_sub_ps(v1z, v0z);
      const __m128 v32x = _mm_sub_ps(v2x, v1x);
      const __m128 v32y = _mm_sub_ps(v2y, v1y);
      const __m128 v32z = _mm_sub_ps(v2z, v1z);
      const __m128 v13x = _mm_sub_ps(v0x, v2x);
      const __m128 v13y = _mm_sub_ps(v0y, v2y);
      const __m128 v13z = _mm_sub_ps(v0z, v2z);

      // p1 = a_p - v0, p2 = a_p - v1, p3 = a_p - v2
      const __m128 p1x = _mm_sub_ps(px, v0x);
      const __m128 p1y = _mm_sub_ps(py, v0y);
      const __m128 p1z = _mm_sub_ps(pz, v0z);
      const __m128 p2x = _mm_sub_ps(px, v1x);
      const __m128 p2y = _mm_sub_ps(py, v1y);
      const __m128 p2z = _mm_sub_ps(pz, v1z);
      const __m128 p3x = _mm_sub_ps(px, v2x);
      const __m128 p3y = _mm_sub_ps(py, v2y);
      const __m128 p3z = _mm_sub_ps(pz, v2z);

      // dot(v21 x fn, p1), etc. — cross products then dot
      // cross(v21, fn) = (v21y*fnz - v21z*fny, v21z*fnx - v21x*fnz, v21x*fny - v21y*fnx)
      auto dot3 =
        [](const __m128 ax, const __m128 ay, const __m128 az, const __m128 bx, const __m128 by, const __m128 bz)
        -> __m128 { return _mm_add_ps(_mm_add_ps(_mm_mul_ps(ax, bx), _mm_mul_ps(ay, by)), _mm_mul_ps(az, bz)); };
      auto crossdot = [&](const __m128 ax,
                          const __m128 ay,
                          const __m128 az,
                          const __m128 bx,
                          const __m128 by,
                          const __m128 bz,
                          const __m128 cx,
                          const __m128 cy,
                          const __m128 cz) -> __m128 {
        const __m128 ex = _mm_sub_ps(_mm_mul_ps(ay, bz), _mm_mul_ps(az, by));
        const __m128 ey = _mm_sub_ps(_mm_mul_ps(az, bx), _mm_mul_ps(ax, bz));
        const __m128 ez = _mm_sub_ps(_mm_mul_ps(ax, by), _mm_mul_ps(ay, bx));
        return dot3(ex, ey, ez, cx, cy, cz);
      };

      const __m128 s0d = crossdot(v21x, v21y, v21z, fnx, fny, fnz, p1x, p1y, p1z);
      const __m128 s1d = crossdot(v32x, v32y, v32z, fnx, fny, fnz, p2x, p2y, p2z);
      const __m128 s2d = crossdot(v13x, v13y, v13z, fnx, fny, fnz, p3x, p3y, p3z);

      // sgn: +1 if > 0, else -1
      const __m128 pos_mask = _mm_set1_ps(-0.f); // sign bit mask
      const __m128 s0       = _mm_or_ps(_mm_andnot_ps(pos_mask, one), _mm_and_ps(pos_mask, s0d));
      const __m128 s1       = _mm_or_ps(_mm_andnot_ps(pos_mask, one), _mm_and_ps(pos_mask, s1d));
      const __m128 s2       = _mm_or_ps(_mm_andnot_ps(pos_mask, one), _mm_and_ps(pos_mask, s2d));

      // Point-in-triangle test: abs(s0+s1+s2) >= 2 (works for both inward and outward normals)
      const __m128 ssum   = _mm_add_ps(_mm_add_ps(s0, s1), s2);
      const __m128 in_tri = _mm_cmpge_ps(_mm_andnot_ps(pos_mask, ssum), _mm_set1_ps(2.f));

      // face distance = dot(fn, p1)
      const __m128 face_d = dot3(fnx, fny, fnz, p1x, p1y, p1z);

      // t1 = dot(p1, v21) / dot(v21, v21), clamped implicitly by checking (t1>0 && t1<1)
      const __m128 dp1v21  = dot3(p1x, p1y, p1z, v21x, v21y, v21z);
      const __m128 dv21v21 = dot3(v21x, v21y, v21z, v21x, v21y, v21z);
      const __m128 t1      = _mm_div_ps(dp1v21, dv21v21);
      // y1 = p1 - t1*v21
      const __m128 y1x = _mm_sub_ps(p1x, _mm_mul_ps(t1, v21x));
      const __m128 y1y = _mm_sub_ps(p1y, _mm_mul_ps(t1, v21y));
      const __m128 y1z = _mm_sub_ps(p1z, _mm_mul_ps(t1, v21z));

      const __m128 dp2v32  = dot3(p2x, p2y, p2z, v32x, v32y, v32z);
      const __m128 dv32v32 = dot3(v32x, v32y, v32z, v32x, v32y, v32z);
      const __m128 t2      = _mm_div_ps(dp2v32, dv32v32);
      const __m128 y2x     = _mm_sub_ps(p2x, _mm_mul_ps(t2, v32x));
      const __m128 y2y     = _mm_sub_ps(p2y, _mm_mul_ps(t2, v32y));
      const __m128 y2z     = _mm_sub_ps(p2z, _mm_mul_ps(t2, v32z));

      const __m128 dp3v13  = dot3(p3x, p3y, p3z, v13x, v13y, v13z);
      const __m128 dv13v13 = dot3(v13x, v13y, v13z, v13x, v13y, v13z);
      const __m128 t3      = _mm_div_ps(dp3v13, dv13v13);
      const __m128 y3x     = _mm_sub_ps(p3x, _mm_mul_ps(t3, v13x));
      const __m128 y3y     = _mm_sub_ps(p3y, _mm_mul_ps(t3, v13y));
      const __m128 y3z     = _mm_sub_ps(p3z, _mm_mul_ps(t3, v13z));

      // Vertex normals
      const __m128 vn0x = _mm_load_ps(vnx[0]);
      const __m128 vn0y = _mm_load_ps(vny[0]);
      const __m128 vn0z = _mm_load_ps(vnz[0]);
      const __m128 vn1x = _mm_load_ps(vnx[1]);
      const __m128 vn1y = _mm_load_ps(vny[1]);
      const __m128 vn1z = _mm_load_ps(vnz[1]);
      const __m128 vn2x = _mm_load_ps(vnx[2]);
      const __m128 vn2y = _mm_load_ps(vny[2]);
      const __m128 vn2z = _mm_load_ps(vnz[2]);
      // Edge normals
      const __m128 en0x = _mm_load_ps(enx[0]);
      const __m128 en0y = _mm_load_ps(eny[0]);
      const __m128 en0z = _mm_load_ps(enz[0]);
      const __m128 en1x = _mm_load_ps(enx[1]);
      const __m128 en1y = _mm_load_ps(eny[1]);
      const __m128 en1z = _mm_load_ps(enz[1]);
      const __m128 en2x = _mm_load_ps(enx[2]);
      const __m128 en2y = _mm_load_ps(eny[2]);
      const __m128 en2z = _mm_load_ps(enz[2]);

      // Squared distances and signed distances for each feature
      const __m128 p1d2 = dot3(p1x, p1y, p1z, p1x, p1y, p1z);
      const __m128 p2d2 = dot3(p2x, p2y, p2z, p2x, p2y, p2z);
      const __m128 p3d2 = dot3(p3x, p3y, p3z, p3x, p3y, p3z);
      const __m128 y1d2 = dot3(y1x, y1y, y1z, y1x, y1y, y1z);
      const __m128 y2d2 = dot3(y2x, y2y, y2z, y2x, y2y, y2z);
      const __m128 y3d2 = dot3(y3x, y3y, y3z, y3x, y3y, y3z);

      // Extract sign bits only — sqrt is deferred to a single call after the blendv chain.
      auto sgn_of =
        [&](const __m128 ax, const __m128 ay, const __m128 az, const __m128 bx, const __m128 by, const __m128 bz)
        -> __m128 {
        const __m128 d = dot3(ax, ay, az, bx, by, bz);
        return _mm_or_ps(_mm_andnot_ps(pos_mask, one), _mm_and_ps(pos_mask, d));
      };

      const __m128 vs0 = sgn_of(vn0x, vn0y, vn0z, p1x, p1y, p1z);
      const __m128 vs1 = sgn_of(vn1x, vn1y, vn1z, p2x, p2y, p2z);
      const __m128 vs2 = sgn_of(vn2x, vn2y, vn2z, p3x, p3y, p3z);
      const __m128 es0 = sgn_of(en0x, en0y, en0z, y1x, y1y, y1z);
      const __m128 es1 = sgn_of(en1x, en1y, en1z, y2x, y2y, y2z);
      const __m128 es2 = sgn_of(en2x, en2y, en2z, y3x, y3y, y3z);

      // t valid masks: t > 0 and t < 1
      const __m128 t1_valid = _mm_and_ps(_mm_cmpgt_ps(t1, zer), _mm_cmplt_ps(t1, one));
      const __m128 t2_valid = _mm_and_ps(_mm_cmpgt_ps(t2, zer), _mm_cmplt_ps(t2, one));
      const __m128 t3_valid = _mm_and_ps(_mm_cmpgt_ps(t3, zer), _mm_cmplt_ps(t3, one));

      // Blendv chain: track (best_d2, best_sgn). No sqrt until the end.
      __m128 best_d2  = p1d2;
      __m128 best_sgn = vs0;

      const __m128 mask1 = _mm_cmplt_ps(p2d2, best_d2);
      best_sgn           = _mm_blendv_ps(best_sgn, vs1, mask1);
      best_d2            = _mm_blendv_ps(best_d2, p2d2, mask1);

      const __m128 mask2 = _mm_cmplt_ps(p3d2, best_d2);
      best_sgn           = _mm_blendv_ps(best_sgn, vs2, mask2);
      best_d2            = _mm_blendv_ps(best_d2, p3d2, mask2);

      const __m128 mask_e0 = _mm_and_ps(t1_valid, _mm_cmplt_ps(y1d2, best_d2));
      best_sgn             = _mm_blendv_ps(best_sgn, es0, mask_e0);
      best_d2              = _mm_blendv_ps(best_d2, y1d2, mask_e0);

      const __m128 mask_e1 = _mm_and_ps(t2_valid, _mm_cmplt_ps(y2d2, best_d2));
      best_sgn             = _mm_blendv_ps(best_sgn, es1, mask_e1);
      best_d2              = _mm_blendv_ps(best_d2, y2d2, mask_e1);

      const __m128 mask_e2 = _mm_and_ps(t3_valid, _mm_cmplt_ps(y3d2, best_d2));
      best_sgn             = _mm_blendv_ps(best_sgn, es2, mask_e2);
      best_d2              = _mm_blendv_ps(best_d2, y3d2, mask_e2);

      // Single sqrt for all vertex/edge lanes, then face override.
      const __m128 ev_d   = _mm_mul_ps(best_sgn, _mm_sqrt_ps(best_d2));
      const __m128 best_d = _mm_blendv_ps(ev_d, face_d, in_tri);

      // Horizontal reduction over W lanes (validCount lanes only)
      alignas(16) float d4[4];
      _mm_store_ps(d4, best_d);

      float best = std::numeric_limits<float>::max();
      float babs = std::numeric_limits<float>::max();
      for (uint32_t i = 0; i < validCount; i++) {
        const float ad = std::abs(d4[i]);
        if (ad < babs) {
          best = d4[i];
          babs = ad;
        }
      }
      return best;
    }
#endif
#if defined(__AVX__)
    if constexpr (W == 8 && std::is_same_v<T, float>) {
      static_assert(alignof(TriangleSoAT<T, W>) == W * sizeof(T),
                    "TriangleSoAT alignment mismatch: _mm256_load_ps requires 32-byte alignment");
      const __m256 px  = _mm256_set1_ps(a_p[0]);
      const __m256 py  = _mm256_set1_ps(a_p[1]);
      const __m256 pz  = _mm256_set1_ps(a_p[2]);
      const __m256 one = _mm256_set1_ps(1.f);
      const __m256 zer = _mm256_setzero_ps();

      const __m256 v0x = _mm256_load_ps(vx[0]), v0y = _mm256_load_ps(vy[0]), v0z = _mm256_load_ps(vz[0]);
      const __m256 v1x = _mm256_load_ps(vx[1]), v1y = _mm256_load_ps(vy[1]), v1z = _mm256_load_ps(vz[1]);
      const __m256 v2x = _mm256_load_ps(vx[2]), v2y = _mm256_load_ps(vy[2]), v2z = _mm256_load_ps(vz[2]);

      const __m256 fnx = _mm256_load_ps(nx), fny = _mm256_load_ps(ny), fnz = _mm256_load_ps(nz);

      const __m256 v21x = _mm256_sub_ps(v1x, v0x), v21y = _mm256_sub_ps(v1y, v0y), v21z = _mm256_sub_ps(v1z, v0z);
      const __m256 v32x = _mm256_sub_ps(v2x, v1x), v32y = _mm256_sub_ps(v2y, v1y), v32z = _mm256_sub_ps(v2z, v1z);
      const __m256 v13x = _mm256_sub_ps(v0x, v2x), v13y = _mm256_sub_ps(v0y, v2y), v13z = _mm256_sub_ps(v0z, v2z);

      const __m256 p1x = _mm256_sub_ps(px, v0x), p1y = _mm256_sub_ps(py, v0y), p1z = _mm256_sub_ps(pz, v0z);
      const __m256 p2x = _mm256_sub_ps(px, v1x), p2y = _mm256_sub_ps(py, v1y), p2z = _mm256_sub_ps(pz, v1z);
      const __m256 p3x = _mm256_sub_ps(px, v2x), p3y = _mm256_sub_ps(py, v2y), p3z = _mm256_sub_ps(pz, v2z);

      auto dot3 =
        [](const __m256 ax, const __m256 ay, const __m256 az, const __m256 bx, const __m256 by, const __m256 bz)
        -> __m256 {
        return _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(ax, bx), _mm256_mul_ps(ay, by)), _mm256_mul_ps(az, bz));
      };
      auto crossdot = [&](const __m256 ax,
                          const __m256 ay,
                          const __m256 az,
                          const __m256 bx,
                          const __m256 by,
                          const __m256 bz,
                          const __m256 cx,
                          const __m256 cy,
                          const __m256 cz) -> __m256 {
        const __m256 ex = _mm256_sub_ps(_mm256_mul_ps(ay, bz), _mm256_mul_ps(az, by));
        const __m256 ey = _mm256_sub_ps(_mm256_mul_ps(az, bx), _mm256_mul_ps(ax, bz));
        const __m256 ez = _mm256_sub_ps(_mm256_mul_ps(ax, by), _mm256_mul_ps(ay, bx));
        return dot3(ex, ey, ez, cx, cy, cz);
      };

      const __m256 s0d = crossdot(v21x, v21y, v21z, fnx, fny, fnz, p1x, p1y, p1z);
      const __m256 s1d = crossdot(v32x, v32y, v32z, fnx, fny, fnz, p2x, p2y, p2z);
      const __m256 s2d = crossdot(v13x, v13y, v13z, fnx, fny, fnz, p3x, p3y, p3z);

      const __m256 pos_mask = _mm256_set1_ps(-0.f);
      const __m256 s0       = _mm256_or_ps(_mm256_andnot_ps(pos_mask, one), _mm256_and_ps(pos_mask, s0d));
      const __m256 s1       = _mm256_or_ps(_mm256_andnot_ps(pos_mask, one), _mm256_and_ps(pos_mask, s1d));
      const __m256 s2       = _mm256_or_ps(_mm256_andnot_ps(pos_mask, one), _mm256_and_ps(pos_mask, s2d));

      const __m256 ssum   = _mm256_add_ps(_mm256_add_ps(s0, s1), s2);
      const __m256 in_tri = _mm256_cmp_ps(_mm256_andnot_ps(pos_mask, ssum), _mm256_set1_ps(2.f), _CMP_GE_OQ);
      const __m256 face_d = dot3(fnx, fny, fnz, p1x, p1y, p1z);

      const __m256 dp1v21  = dot3(p1x, p1y, p1z, v21x, v21y, v21z);
      const __m256 dv21v21 = dot3(v21x, v21y, v21z, v21x, v21y, v21z);
      const __m256 t1      = _mm256_div_ps(dp1v21, dv21v21);
      const __m256 y1x     = _mm256_sub_ps(p1x, _mm256_mul_ps(t1, v21x));
      const __m256 y1y     = _mm256_sub_ps(p1y, _mm256_mul_ps(t1, v21y));
      const __m256 y1z     = _mm256_sub_ps(p1z, _mm256_mul_ps(t1, v21z));

      const __m256 dp2v32  = dot3(p2x, p2y, p2z, v32x, v32y, v32z);
      const __m256 dv32v32 = dot3(v32x, v32y, v32z, v32x, v32y, v32z);
      const __m256 t2      = _mm256_div_ps(dp2v32, dv32v32);
      const __m256 y2x     = _mm256_sub_ps(p2x, _mm256_mul_ps(t2, v32x));
      const __m256 y2y     = _mm256_sub_ps(p2y, _mm256_mul_ps(t2, v32y));
      const __m256 y2z     = _mm256_sub_ps(p2z, _mm256_mul_ps(t2, v32z));

      const __m256 dp3v13  = dot3(p3x, p3y, p3z, v13x, v13y, v13z);
      const __m256 dv13v13 = dot3(v13x, v13y, v13z, v13x, v13y, v13z);
      const __m256 t3      = _mm256_div_ps(dp3v13, dv13v13);
      const __m256 y3x     = _mm256_sub_ps(p3x, _mm256_mul_ps(t3, v13x));
      const __m256 y3y     = _mm256_sub_ps(p3y, _mm256_mul_ps(t3, v13y));
      const __m256 y3z     = _mm256_sub_ps(p3z, _mm256_mul_ps(t3, v13z));

      const __m256 vn0x = _mm256_load_ps(vnx[0]), vn0y = _mm256_load_ps(vny[0]), vn0z = _mm256_load_ps(vnz[0]);
      const __m256 vn1x = _mm256_load_ps(vnx[1]), vn1y = _mm256_load_ps(vny[1]), vn1z = _mm256_load_ps(vnz[1]);
      const __m256 vn2x = _mm256_load_ps(vnx[2]), vn2y = _mm256_load_ps(vny[2]), vn2z = _mm256_load_ps(vnz[2]);
      const __m256 en0x = _mm256_load_ps(enx[0]), en0y = _mm256_load_ps(eny[0]), en0z = _mm256_load_ps(enz[0]);
      const __m256 en1x = _mm256_load_ps(enx[1]), en1y = _mm256_load_ps(eny[1]), en1z = _mm256_load_ps(enz[1]);
      const __m256 en2x = _mm256_load_ps(enx[2]), en2y = _mm256_load_ps(eny[2]), en2z = _mm256_load_ps(enz[2]);

      const __m256 p1d2 = dot3(p1x, p1y, p1z, p1x, p1y, p1z);
      const __m256 p2d2 = dot3(p2x, p2y, p2z, p2x, p2y, p2z);
      const __m256 p3d2 = dot3(p3x, p3y, p3z, p3x, p3y, p3z);
      const __m256 y1d2 = dot3(y1x, y1y, y1z, y1x, y1y, y1z);
      const __m256 y2d2 = dot3(y2x, y2y, y2z, y2x, y2y, y2z);
      const __m256 y3d2 = dot3(y3x, y3y, y3z, y3x, y3y, y3z);

      // Extract sign bits only — sqrt is deferred to a single call after the blendv chain.
      auto sgn_of =
        [&](const __m256 ax, const __m256 ay, const __m256 az, const __m256 bx, const __m256 by, const __m256 bz)
        -> __m256 {
        const __m256 d = dot3(ax, ay, az, bx, by, bz);
        return _mm256_or_ps(_mm256_andnot_ps(pos_mask, one), _mm256_and_ps(pos_mask, d));
      };

      const __m256 vs0 = sgn_of(vn0x, vn0y, vn0z, p1x, p1y, p1z);
      const __m256 vs1 = sgn_of(vn1x, vn1y, vn1z, p2x, p2y, p2z);
      const __m256 vs2 = sgn_of(vn2x, vn2y, vn2z, p3x, p3y, p3z);
      const __m256 es0 = sgn_of(en0x, en0y, en0z, y1x, y1y, y1z);
      const __m256 es1 = sgn_of(en1x, en1y, en1z, y2x, y2y, y2z);
      const __m256 es2 = sgn_of(en2x, en2y, en2z, y3x, y3y, y3z);

      const __m256 t1_valid = _mm256_and_ps(_mm256_cmp_ps(t1, zer, _CMP_GT_OQ), _mm256_cmp_ps(t1, one, _CMP_LT_OQ));
      const __m256 t2_valid = _mm256_and_ps(_mm256_cmp_ps(t2, zer, _CMP_GT_OQ), _mm256_cmp_ps(t2, one, _CMP_LT_OQ));
      const __m256 t3_valid = _mm256_and_ps(_mm256_cmp_ps(t3, zer, _CMP_GT_OQ), _mm256_cmp_ps(t3, one, _CMP_LT_OQ));

      // Blendv chain: track (best_d2, best_sgn). No sqrt until the end.
      __m256 best_d2  = p1d2;
      __m256 best_sgn = vs0;

      const __m256 mask1 = _mm256_cmp_ps(p2d2, best_d2, _CMP_LT_OQ);
      best_sgn           = _mm256_blendv_ps(best_sgn, vs1, mask1);
      best_d2            = _mm256_blendv_ps(best_d2, p2d2, mask1);

      const __m256 mask2 = _mm256_cmp_ps(p3d2, best_d2, _CMP_LT_OQ);
      best_sgn           = _mm256_blendv_ps(best_sgn, vs2, mask2);
      best_d2            = _mm256_blendv_ps(best_d2, p3d2, mask2);

      const __m256 mask_e0 = _mm256_and_ps(t1_valid, _mm256_cmp_ps(y1d2, best_d2, _CMP_LT_OQ));
      best_sgn             = _mm256_blendv_ps(best_sgn, es0, mask_e0);
      best_d2              = _mm256_blendv_ps(best_d2, y1d2, mask_e0);

      const __m256 mask_e1 = _mm256_and_ps(t2_valid, _mm256_cmp_ps(y2d2, best_d2, _CMP_LT_OQ));
      best_sgn             = _mm256_blendv_ps(best_sgn, es1, mask_e1);
      best_d2              = _mm256_blendv_ps(best_d2, y2d2, mask_e1);

      const __m256 mask_e2 = _mm256_and_ps(t3_valid, _mm256_cmp_ps(y3d2, best_d2, _CMP_LT_OQ));
      best_sgn             = _mm256_blendv_ps(best_sgn, es2, mask_e2);
      best_d2              = _mm256_blendv_ps(best_d2, y3d2, mask_e2);

      // Single sqrt for all vertex/edge lanes, then face override.
      const __m256 ev_d   = _mm256_mul_ps(best_sgn, _mm256_sqrt_ps(best_d2));
      const __m256 best_d = _mm256_blendv_ps(ev_d, face_d, in_tri);

      alignas(32) float d8[8];
      _mm256_store_ps(d8, best_d);

      float best = std::numeric_limits<float>::max();
      float babs = std::numeric_limits<float>::max();
      for (uint32_t i = 0; i < validCount; i++) {
        const float ad = std::abs(d8[i]);
        if (ad < babs) {
          best = d8[i];
          babs = ad;
        }
      }
      return best;
    }
    if constexpr (W == 8 && std::is_same_v<T, double>) {
      static_assert(alignof(TriangleSoAT<T, W>) == W * sizeof(T),
                    "TriangleSoAT alignment mismatch: _mm256_load_pd requires 32-byte alignment");
      const __m256d px  = _mm256_set1_pd(a_p[0]);
      const __m256d py  = _mm256_set1_pd(a_p[1]);
      const __m256d pz  = _mm256_set1_pd(a_p[2]);
      const __m256d one = _mm256_set1_pd(1.0);
      const __m256d zer = _mm256_setzero_pd();

      // lo = triangles 0-3, hi = triangles 4-7
      const __m256d v0x_lo = _mm256_load_pd(vx[0]), v0x_hi = _mm256_load_pd(vx[0] + 4);
      const __m256d v0y_lo = _mm256_load_pd(vy[0]), v0y_hi = _mm256_load_pd(vy[0] + 4);
      const __m256d v0z_lo = _mm256_load_pd(vz[0]), v0z_hi = _mm256_load_pd(vz[0] + 4);
      const __m256d v1x_lo = _mm256_load_pd(vx[1]), v1x_hi = _mm256_load_pd(vx[1] + 4);
      const __m256d v1y_lo = _mm256_load_pd(vy[1]), v1y_hi = _mm256_load_pd(vy[1] + 4);
      const __m256d v1z_lo = _mm256_load_pd(vz[1]), v1z_hi = _mm256_load_pd(vz[1] + 4);
      const __m256d v2x_lo = _mm256_load_pd(vx[2]), v2x_hi = _mm256_load_pd(vx[2] + 4);
      const __m256d v2y_lo = _mm256_load_pd(vy[2]), v2y_hi = _mm256_load_pd(vy[2] + 4);
      const __m256d v2z_lo = _mm256_load_pd(vz[2]), v2z_hi = _mm256_load_pd(vz[2] + 4);

      const __m256d fnx_lo = _mm256_load_pd(nx), fnx_hi = _mm256_load_pd(nx + 4);
      const __m256d fny_lo = _mm256_load_pd(ny), fny_hi = _mm256_load_pd(ny + 4);
      const __m256d fnz_lo = _mm256_load_pd(nz), fnz_hi = _mm256_load_pd(nz + 4);

      const __m256d v21x_lo = _mm256_sub_pd(v1x_lo, v0x_lo), v21x_hi = _mm256_sub_pd(v1x_hi, v0x_hi);
      const __m256d v21y_lo = _mm256_sub_pd(v1y_lo, v0y_lo), v21y_hi = _mm256_sub_pd(v1y_hi, v0y_hi);
      const __m256d v21z_lo = _mm256_sub_pd(v1z_lo, v0z_lo), v21z_hi = _mm256_sub_pd(v1z_hi, v0z_hi);
      const __m256d v32x_lo = _mm256_sub_pd(v2x_lo, v1x_lo), v32x_hi = _mm256_sub_pd(v2x_hi, v1x_hi);
      const __m256d v32y_lo = _mm256_sub_pd(v2y_lo, v1y_lo), v32y_hi = _mm256_sub_pd(v2y_hi, v1y_hi);
      const __m256d v32z_lo = _mm256_sub_pd(v2z_lo, v1z_lo), v32z_hi = _mm256_sub_pd(v2z_hi, v1z_hi);
      const __m256d v13x_lo = _mm256_sub_pd(v0x_lo, v2x_lo), v13x_hi = _mm256_sub_pd(v0x_hi, v2x_hi);
      const __m256d v13y_lo = _mm256_sub_pd(v0y_lo, v2y_lo), v13y_hi = _mm256_sub_pd(v0y_hi, v2y_hi);
      const __m256d v13z_lo = _mm256_sub_pd(v0z_lo, v2z_lo), v13z_hi = _mm256_sub_pd(v0z_hi, v2z_hi);

      const __m256d p1x_lo = _mm256_sub_pd(px, v0x_lo), p1x_hi = _mm256_sub_pd(px, v0x_hi);
      const __m256d p1y_lo = _mm256_sub_pd(py, v0y_lo), p1y_hi = _mm256_sub_pd(py, v0y_hi);
      const __m256d p1z_lo = _mm256_sub_pd(pz, v0z_lo), p1z_hi = _mm256_sub_pd(pz, v0z_hi);
      const __m256d p2x_lo = _mm256_sub_pd(px, v1x_lo), p2x_hi = _mm256_sub_pd(px, v1x_hi);
      const __m256d p2y_lo = _mm256_sub_pd(py, v1y_lo), p2y_hi = _mm256_sub_pd(py, v1y_hi);
      const __m256d p2z_lo = _mm256_sub_pd(pz, v1z_lo), p2z_hi = _mm256_sub_pd(pz, v1z_hi);
      const __m256d p3x_lo = _mm256_sub_pd(px, v2x_lo), p3x_hi = _mm256_sub_pd(px, v2x_hi);
      const __m256d p3y_lo = _mm256_sub_pd(py, v2y_lo), p3y_hi = _mm256_sub_pd(py, v2y_hi);
      const __m256d p3z_lo = _mm256_sub_pd(pz, v2z_lo), p3z_hi = _mm256_sub_pd(pz, v2z_hi);

      auto dot3 =
        [](const __m256d ax, const __m256d ay, const __m256d az, const __m256d bx, const __m256d by, const __m256d bz)
        -> __m256d {
        return _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(ax, bx), _mm256_mul_pd(ay, by)), _mm256_mul_pd(az, bz));
      };
      auto crossdot = [&](const __m256d ax,
                          const __m256d ay,
                          const __m256d az,
                          const __m256d bx,
                          const __m256d by,
                          const __m256d bz,
                          const __m256d cx,
                          const __m256d cy,
                          const __m256d cz) -> __m256d {
        const __m256d ex = _mm256_sub_pd(_mm256_mul_pd(ay, bz), _mm256_mul_pd(az, by));
        const __m256d ey = _mm256_sub_pd(_mm256_mul_pd(az, bx), _mm256_mul_pd(ax, bz));
        const __m256d ez = _mm256_sub_pd(_mm256_mul_pd(ax, by), _mm256_mul_pd(ay, bx));
        return dot3(ex, ey, ez, cx, cy, cz);
      };

      const __m256d s0d_lo = crossdot(v21x_lo, v21y_lo, v21z_lo, fnx_lo, fny_lo, fnz_lo, p1x_lo, p1y_lo, p1z_lo);
      const __m256d s0d_hi = crossdot(v21x_hi, v21y_hi, v21z_hi, fnx_hi, fny_hi, fnz_hi, p1x_hi, p1y_hi, p1z_hi);
      const __m256d s1d_lo = crossdot(v32x_lo, v32y_lo, v32z_lo, fnx_lo, fny_lo, fnz_lo, p2x_lo, p2y_lo, p2z_lo);
      const __m256d s1d_hi = crossdot(v32x_hi, v32y_hi, v32z_hi, fnx_hi, fny_hi, fnz_hi, p2x_hi, p2y_hi, p2z_hi);
      const __m256d s2d_lo = crossdot(v13x_lo, v13y_lo, v13z_lo, fnx_lo, fny_lo, fnz_lo, p3x_lo, p3y_lo, p3z_lo);
      const __m256d s2d_hi = crossdot(v13x_hi, v13y_hi, v13z_hi, fnx_hi, fny_hi, fnz_hi, p3x_hi, p3y_hi, p3z_hi);

      const __m256d pos_mask = _mm256_set1_pd(-0.0);
      const __m256d s0_lo    = _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, s0d_lo));
      const __m256d s0_hi    = _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, s0d_hi));
      const __m256d s1_lo    = _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, s1d_lo));
      const __m256d s1_hi    = _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, s1d_hi));
      const __m256d s2_lo    = _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, s2d_lo));
      const __m256d s2_hi    = _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, s2d_hi));

      const __m256d two       = _mm256_set1_pd(2.0);
      const __m256d ssum_lo   = _mm256_add_pd(_mm256_add_pd(s0_lo, s1_lo), s2_lo);
      const __m256d ssum_hi   = _mm256_add_pd(_mm256_add_pd(s0_hi, s1_hi), s2_hi);
      const __m256d in_tri_lo = _mm256_cmp_pd(_mm256_andnot_pd(pos_mask, ssum_lo), two, _CMP_GE_OQ);
      const __m256d in_tri_hi = _mm256_cmp_pd(_mm256_andnot_pd(pos_mask, ssum_hi), two, _CMP_GE_OQ);
      const __m256d face_d_lo = dot3(fnx_lo, fny_lo, fnz_lo, p1x_lo, p1y_lo, p1z_lo);
      const __m256d face_d_hi = dot3(fnx_hi, fny_hi, fnz_hi, p1x_hi, p1y_hi, p1z_hi);

      const __m256d dp1v21_lo  = dot3(p1x_lo, p1y_lo, p1z_lo, v21x_lo, v21y_lo, v21z_lo);
      const __m256d dp1v21_hi  = dot3(p1x_hi, p1y_hi, p1z_hi, v21x_hi, v21y_hi, v21z_hi);
      const __m256d dv21v21_lo = dot3(v21x_lo, v21y_lo, v21z_lo, v21x_lo, v21y_lo, v21z_lo);
      const __m256d dv21v21_hi = dot3(v21x_hi, v21y_hi, v21z_hi, v21x_hi, v21y_hi, v21z_hi);
      const __m256d t1_lo      = _mm256_div_pd(dp1v21_lo, dv21v21_lo);
      const __m256d t1_hi      = _mm256_div_pd(dp1v21_hi, dv21v21_hi);
      const __m256d y1x_lo     = _mm256_sub_pd(p1x_lo, _mm256_mul_pd(t1_lo, v21x_lo));
      const __m256d y1x_hi     = _mm256_sub_pd(p1x_hi, _mm256_mul_pd(t1_hi, v21x_hi));
      const __m256d y1y_lo     = _mm256_sub_pd(p1y_lo, _mm256_mul_pd(t1_lo, v21y_lo));
      const __m256d y1y_hi     = _mm256_sub_pd(p1y_hi, _mm256_mul_pd(t1_hi, v21y_hi));
      const __m256d y1z_lo     = _mm256_sub_pd(p1z_lo, _mm256_mul_pd(t1_lo, v21z_lo));
      const __m256d y1z_hi     = _mm256_sub_pd(p1z_hi, _mm256_mul_pd(t1_hi, v21z_hi));

      const __m256d dp2v32_lo  = dot3(p2x_lo, p2y_lo, p2z_lo, v32x_lo, v32y_lo, v32z_lo);
      const __m256d dp2v32_hi  = dot3(p2x_hi, p2y_hi, p2z_hi, v32x_hi, v32y_hi, v32z_hi);
      const __m256d dv32v32_lo = dot3(v32x_lo, v32y_lo, v32z_lo, v32x_lo, v32y_lo, v32z_lo);
      const __m256d dv32v32_hi = dot3(v32x_hi, v32y_hi, v32z_hi, v32x_hi, v32y_hi, v32z_hi);
      const __m256d t2_lo      = _mm256_div_pd(dp2v32_lo, dv32v32_lo);
      const __m256d t2_hi      = _mm256_div_pd(dp2v32_hi, dv32v32_hi);
      const __m256d y2x_lo     = _mm256_sub_pd(p2x_lo, _mm256_mul_pd(t2_lo, v32x_lo));
      const __m256d y2x_hi     = _mm256_sub_pd(p2x_hi, _mm256_mul_pd(t2_hi, v32x_hi));
      const __m256d y2y_lo     = _mm256_sub_pd(p2y_lo, _mm256_mul_pd(t2_lo, v32y_lo));
      const __m256d y2y_hi     = _mm256_sub_pd(p2y_hi, _mm256_mul_pd(t2_hi, v32y_hi));
      const __m256d y2z_lo     = _mm256_sub_pd(p2z_lo, _mm256_mul_pd(t2_lo, v32z_lo));
      const __m256d y2z_hi     = _mm256_sub_pd(p2z_hi, _mm256_mul_pd(t2_hi, v32z_hi));

      const __m256d dp3v13_lo  = dot3(p3x_lo, p3y_lo, p3z_lo, v13x_lo, v13y_lo, v13z_lo);
      const __m256d dp3v13_hi  = dot3(p3x_hi, p3y_hi, p3z_hi, v13x_hi, v13y_hi, v13z_hi);
      const __m256d dv13v13_lo = dot3(v13x_lo, v13y_lo, v13z_lo, v13x_lo, v13y_lo, v13z_lo);
      const __m256d dv13v13_hi = dot3(v13x_hi, v13y_hi, v13z_hi, v13x_hi, v13y_hi, v13z_hi);
      const __m256d t3_lo      = _mm256_div_pd(dp3v13_lo, dv13v13_lo);
      const __m256d t3_hi      = _mm256_div_pd(dp3v13_hi, dv13v13_hi);
      const __m256d y3x_lo     = _mm256_sub_pd(p3x_lo, _mm256_mul_pd(t3_lo, v13x_lo));
      const __m256d y3x_hi     = _mm256_sub_pd(p3x_hi, _mm256_mul_pd(t3_hi, v13x_hi));
      const __m256d y3y_lo     = _mm256_sub_pd(p3y_lo, _mm256_mul_pd(t3_lo, v13y_lo));
      const __m256d y3y_hi     = _mm256_sub_pd(p3y_hi, _mm256_mul_pd(t3_hi, v13y_hi));
      const __m256d y3z_lo     = _mm256_sub_pd(p3z_lo, _mm256_mul_pd(t3_lo, v13z_lo));
      const __m256d y3z_hi     = _mm256_sub_pd(p3z_hi, _mm256_mul_pd(t3_hi, v13z_hi));

      const __m256d vn0x_lo = _mm256_load_pd(vnx[0]), vn0x_hi = _mm256_load_pd(vnx[0] + 4);
      const __m256d vn0y_lo = _mm256_load_pd(vny[0]), vn0y_hi = _mm256_load_pd(vny[0] + 4);
      const __m256d vn0z_lo = _mm256_load_pd(vnz[0]), vn0z_hi = _mm256_load_pd(vnz[0] + 4);
      const __m256d vn1x_lo = _mm256_load_pd(vnx[1]), vn1x_hi = _mm256_load_pd(vnx[1] + 4);
      const __m256d vn1y_lo = _mm256_load_pd(vny[1]), vn1y_hi = _mm256_load_pd(vny[1] + 4);
      const __m256d vn1z_lo = _mm256_load_pd(vnz[1]), vn1z_hi = _mm256_load_pd(vnz[1] + 4);
      const __m256d vn2x_lo = _mm256_load_pd(vnx[2]), vn2x_hi = _mm256_load_pd(vnx[2] + 4);
      const __m256d vn2y_lo = _mm256_load_pd(vny[2]), vn2y_hi = _mm256_load_pd(vny[2] + 4);
      const __m256d vn2z_lo = _mm256_load_pd(vnz[2]), vn2z_hi = _mm256_load_pd(vnz[2] + 4);
      const __m256d en0x_lo = _mm256_load_pd(enx[0]), en0x_hi = _mm256_load_pd(enx[0] + 4);
      const __m256d en0y_lo = _mm256_load_pd(eny[0]), en0y_hi = _mm256_load_pd(eny[0] + 4);
      const __m256d en0z_lo = _mm256_load_pd(enz[0]), en0z_hi = _mm256_load_pd(enz[0] + 4);
      const __m256d en1x_lo = _mm256_load_pd(enx[1]), en1x_hi = _mm256_load_pd(enx[1] + 4);
      const __m256d en1y_lo = _mm256_load_pd(eny[1]), en1y_hi = _mm256_load_pd(eny[1] + 4);
      const __m256d en1z_lo = _mm256_load_pd(enz[1]), en1z_hi = _mm256_load_pd(enz[1] + 4);
      const __m256d en2x_lo = _mm256_load_pd(enx[2]), en2x_hi = _mm256_load_pd(enx[2] + 4);
      const __m256d en2y_lo = _mm256_load_pd(eny[2]), en2y_hi = _mm256_load_pd(eny[2] + 4);
      const __m256d en2z_lo = _mm256_load_pd(enz[2]), en2z_hi = _mm256_load_pd(enz[2] + 4);

      const __m256d p1d2_lo = dot3(p1x_lo, p1y_lo, p1z_lo, p1x_lo, p1y_lo, p1z_lo);
      const __m256d p1d2_hi = dot3(p1x_hi, p1y_hi, p1z_hi, p1x_hi, p1y_hi, p1z_hi);
      const __m256d p2d2_lo = dot3(p2x_lo, p2y_lo, p2z_lo, p2x_lo, p2y_lo, p2z_lo);
      const __m256d p2d2_hi = dot3(p2x_hi, p2y_hi, p2z_hi, p2x_hi, p2y_hi, p2z_hi);
      const __m256d p3d2_lo = dot3(p3x_lo, p3y_lo, p3z_lo, p3x_lo, p3y_lo, p3z_lo);
      const __m256d p3d2_hi = dot3(p3x_hi, p3y_hi, p3z_hi, p3x_hi, p3y_hi, p3z_hi);
      const __m256d y1d2_lo = dot3(y1x_lo, y1y_lo, y1z_lo, y1x_lo, y1y_lo, y1z_lo);
      const __m256d y1d2_hi = dot3(y1x_hi, y1y_hi, y1z_hi, y1x_hi, y1y_hi, y1z_hi);
      const __m256d y2d2_lo = dot3(y2x_lo, y2y_lo, y2z_lo, y2x_lo, y2y_lo, y2z_lo);
      const __m256d y2d2_hi = dot3(y2x_hi, y2y_hi, y2z_hi, y2x_hi, y2y_hi, y2z_hi);
      const __m256d y3d2_lo = dot3(y3x_lo, y3y_lo, y3z_lo, y3x_lo, y3y_lo, y3z_lo);
      const __m256d y3d2_hi = dot3(y3x_hi, y3y_hi, y3z_hi, y3x_hi, y3y_hi, y3z_hi);

      // Extract sign bits only — sqrt deferred until after blendv chains.
      auto sgn_of =
        [&](const __m256d ax, const __m256d ay, const __m256d az, const __m256d bx, const __m256d by, const __m256d bz)
        -> __m256d {
        const __m256d d = dot3(ax, ay, az, bx, by, bz);
        return _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, d));
      };

      const __m256d vs0_lo = sgn_of(vn0x_lo, vn0y_lo, vn0z_lo, p1x_lo, p1y_lo, p1z_lo);
      const __m256d vs0_hi = sgn_of(vn0x_hi, vn0y_hi, vn0z_hi, p1x_hi, p1y_hi, p1z_hi);
      const __m256d vs1_lo = sgn_of(vn1x_lo, vn1y_lo, vn1z_lo, p2x_lo, p2y_lo, p2z_lo);
      const __m256d vs1_hi = sgn_of(vn1x_hi, vn1y_hi, vn1z_hi, p2x_hi, p2y_hi, p2z_hi);
      const __m256d vs2_lo = sgn_of(vn2x_lo, vn2y_lo, vn2z_lo, p3x_lo, p3y_lo, p3z_lo);
      const __m256d vs2_hi = sgn_of(vn2x_hi, vn2y_hi, vn2z_hi, p3x_hi, p3y_hi, p3z_hi);
      const __m256d es0_lo = sgn_of(en0x_lo, en0y_lo, en0z_lo, y1x_lo, y1y_lo, y1z_lo);
      const __m256d es0_hi = sgn_of(en0x_hi, en0y_hi, en0z_hi, y1x_hi, y1y_hi, y1z_hi);
      const __m256d es1_lo = sgn_of(en1x_lo, en1y_lo, en1z_lo, y2x_lo, y2y_lo, y2z_lo);
      const __m256d es1_hi = sgn_of(en1x_hi, en1y_hi, en1z_hi, y2x_hi, y2y_hi, y2z_hi);
      const __m256d es2_lo = sgn_of(en2x_lo, en2y_lo, en2z_lo, y3x_lo, y3y_lo, y3z_lo);
      const __m256d es2_hi = sgn_of(en2x_hi, en2y_hi, en2z_hi, y3x_hi, y3y_hi, y3z_hi);

      const __m256d t1v_lo =
        _mm256_and_pd(_mm256_cmp_pd(t1_lo, zer, _CMP_GT_OQ), _mm256_cmp_pd(t1_lo, one, _CMP_LT_OQ));
      const __m256d t1v_hi =
        _mm256_and_pd(_mm256_cmp_pd(t1_hi, zer, _CMP_GT_OQ), _mm256_cmp_pd(t1_hi, one, _CMP_LT_OQ));
      const __m256d t2v_lo =
        _mm256_and_pd(_mm256_cmp_pd(t2_lo, zer, _CMP_GT_OQ), _mm256_cmp_pd(t2_lo, one, _CMP_LT_OQ));
      const __m256d t2v_hi =
        _mm256_and_pd(_mm256_cmp_pd(t2_hi, zer, _CMP_GT_OQ), _mm256_cmp_pd(t2_hi, one, _CMP_LT_OQ));
      const __m256d t3v_lo =
        _mm256_and_pd(_mm256_cmp_pd(t3_lo, zer, _CMP_GT_OQ), _mm256_cmp_pd(t3_lo, one, _CMP_LT_OQ));
      const __m256d t3v_hi =
        _mm256_and_pd(_mm256_cmp_pd(t3_hi, zer, _CMP_GT_OQ), _mm256_cmp_pd(t3_hi, one, _CMP_LT_OQ));

      // Blendv chains: lo and hi run in parallel, no sqrt until the end.
      __m256d best_d2_lo = p1d2_lo, best_d2_hi = p1d2_hi;
      __m256d best_sgn_lo = vs0_lo, best_sgn_hi = vs0_hi;

      const __m256d m1_lo = _mm256_cmp_pd(p2d2_lo, best_d2_lo, _CMP_LT_OQ);
      const __m256d m1_hi = _mm256_cmp_pd(p2d2_hi, best_d2_hi, _CMP_LT_OQ);
      best_sgn_lo         = _mm256_blendv_pd(best_sgn_lo, vs1_lo, m1_lo);
      best_sgn_hi         = _mm256_blendv_pd(best_sgn_hi, vs1_hi, m1_hi);
      best_d2_lo          = _mm256_blendv_pd(best_d2_lo, p2d2_lo, m1_lo);
      best_d2_hi          = _mm256_blendv_pd(best_d2_hi, p2d2_hi, m1_hi);

      const __m256d m2_lo = _mm256_cmp_pd(p3d2_lo, best_d2_lo, _CMP_LT_OQ);
      const __m256d m2_hi = _mm256_cmp_pd(p3d2_hi, best_d2_hi, _CMP_LT_OQ);
      best_sgn_lo         = _mm256_blendv_pd(best_sgn_lo, vs2_lo, m2_lo);
      best_sgn_hi         = _mm256_blendv_pd(best_sgn_hi, vs2_hi, m2_hi);
      best_d2_lo          = _mm256_blendv_pd(best_d2_lo, p3d2_lo, m2_lo);
      best_d2_hi          = _mm256_blendv_pd(best_d2_hi, p3d2_hi, m2_hi);

      const __m256d me0_lo = _mm256_and_pd(t1v_lo, _mm256_cmp_pd(y1d2_lo, best_d2_lo, _CMP_LT_OQ));
      const __m256d me0_hi = _mm256_and_pd(t1v_hi, _mm256_cmp_pd(y1d2_hi, best_d2_hi, _CMP_LT_OQ));
      best_sgn_lo          = _mm256_blendv_pd(best_sgn_lo, es0_lo, me0_lo);
      best_sgn_hi          = _mm256_blendv_pd(best_sgn_hi, es0_hi, me0_hi);
      best_d2_lo           = _mm256_blendv_pd(best_d2_lo, y1d2_lo, me0_lo);
      best_d2_hi           = _mm256_blendv_pd(best_d2_hi, y1d2_hi, me0_hi);

      const __m256d me1_lo = _mm256_and_pd(t2v_lo, _mm256_cmp_pd(y2d2_lo, best_d2_lo, _CMP_LT_OQ));
      const __m256d me1_hi = _mm256_and_pd(t2v_hi, _mm256_cmp_pd(y2d2_hi, best_d2_hi, _CMP_LT_OQ));
      best_sgn_lo          = _mm256_blendv_pd(best_sgn_lo, es1_lo, me1_lo);
      best_sgn_hi          = _mm256_blendv_pd(best_sgn_hi, es1_hi, me1_hi);
      best_d2_lo           = _mm256_blendv_pd(best_d2_lo, y2d2_lo, me1_lo);
      best_d2_hi           = _mm256_blendv_pd(best_d2_hi, y2d2_hi, me1_hi);

      const __m256d me2_lo = _mm256_and_pd(t3v_lo, _mm256_cmp_pd(y3d2_lo, best_d2_lo, _CMP_LT_OQ));
      const __m256d me2_hi = _mm256_and_pd(t3v_hi, _mm256_cmp_pd(y3d2_hi, best_d2_hi, _CMP_LT_OQ));
      best_sgn_lo          = _mm256_blendv_pd(best_sgn_lo, es2_lo, me2_lo);
      best_sgn_hi          = _mm256_blendv_pd(best_sgn_hi, es2_hi, me2_hi);
      best_d2_lo           = _mm256_blendv_pd(best_d2_lo, y3d2_lo, me2_lo);
      best_d2_hi           = _mm256_blendv_pd(best_d2_hi, y3d2_hi, me2_hi);

      // Single sqrt per half, then face override.
      const __m256d ev_d_lo   = _mm256_mul_pd(best_sgn_lo, _mm256_sqrt_pd(best_d2_lo));
      const __m256d ev_d_hi   = _mm256_mul_pd(best_sgn_hi, _mm256_sqrt_pd(best_d2_hi));
      const __m256d best_d_lo = _mm256_blendv_pd(ev_d_lo, face_d_lo, in_tri_lo);
      const __m256d best_d_hi = _mm256_blendv_pd(ev_d_hi, face_d_hi, in_tri_hi);

      alignas(32) double d8[8];
      _mm256_store_pd(d8, best_d_lo);
      _mm256_store_pd(d8 + 4, best_d_hi);

      double best = std::numeric_limits<double>::max();
      double babs = std::numeric_limits<double>::max();
      for (uint32_t i = 0; i < validCount; i++) {
        const double ad = std::abs(d8[i]);
        if (ad < babs) {
          best = d8[i];
          babs = ad;
        }
      }
      return static_cast<T>(best);
    }
    if constexpr (W == 4 && std::is_same_v<T, double>) {
      static_assert(alignof(TriangleSoAT<T, W>) == W * sizeof(T),
                    "TriangleSoAT alignment mismatch: _mm256_load_pd requires 32-byte alignment");
      const __m256d px  = _mm256_set1_pd(a_p[0]);
      const __m256d py  = _mm256_set1_pd(a_p[1]);
      const __m256d pz  = _mm256_set1_pd(a_p[2]);
      const __m256d one = _mm256_set1_pd(1.0);
      const __m256d zer = _mm256_setzero_pd();

      const __m256d v0x = _mm256_load_pd(vx[0]), v0y = _mm256_load_pd(vy[0]), v0z = _mm256_load_pd(vz[0]);
      const __m256d v1x = _mm256_load_pd(vx[1]), v1y = _mm256_load_pd(vy[1]), v1z = _mm256_load_pd(vz[1]);
      const __m256d v2x = _mm256_load_pd(vx[2]), v2y = _mm256_load_pd(vy[2]), v2z = _mm256_load_pd(vz[2]);

      const __m256d fnx = _mm256_load_pd(nx), fny = _mm256_load_pd(ny), fnz = _mm256_load_pd(nz);

      const __m256d v21x = _mm256_sub_pd(v1x, v0x), v21y = _mm256_sub_pd(v1y, v0y), v21z = _mm256_sub_pd(v1z, v0z);
      const __m256d v32x = _mm256_sub_pd(v2x, v1x), v32y = _mm256_sub_pd(v2y, v1y), v32z = _mm256_sub_pd(v2z, v1z);
      const __m256d v13x = _mm256_sub_pd(v0x, v2x), v13y = _mm256_sub_pd(v0y, v2y), v13z = _mm256_sub_pd(v0z, v2z);

      const __m256d p1x = _mm256_sub_pd(px, v0x), p1y = _mm256_sub_pd(py, v0y), p1z = _mm256_sub_pd(pz, v0z);
      const __m256d p2x = _mm256_sub_pd(px, v1x), p2y = _mm256_sub_pd(py, v1y), p2z = _mm256_sub_pd(pz, v1z);
      const __m256d p3x = _mm256_sub_pd(px, v2x), p3y = _mm256_sub_pd(py, v2y), p3z = _mm256_sub_pd(pz, v2z);

      auto dot3 =
        [](const __m256d ax, const __m256d ay, const __m256d az, const __m256d bx, const __m256d by, const __m256d bz)
        -> __m256d {
        return _mm256_add_pd(_mm256_add_pd(_mm256_mul_pd(ax, bx), _mm256_mul_pd(ay, by)), _mm256_mul_pd(az, bz));
      };
      auto crossdot = [&](const __m256d ax,
                          const __m256d ay,
                          const __m256d az,
                          const __m256d bx,
                          const __m256d by,
                          const __m256d bz,
                          const __m256d cx,
                          const __m256d cy,
                          const __m256d cz) -> __m256d {
        const __m256d ex = _mm256_sub_pd(_mm256_mul_pd(ay, bz), _mm256_mul_pd(az, by));
        const __m256d ey = _mm256_sub_pd(_mm256_mul_pd(az, bx), _mm256_mul_pd(ax, bz));
        const __m256d ez = _mm256_sub_pd(_mm256_mul_pd(ax, by), _mm256_mul_pd(ay, bx));
        return dot3(ex, ey, ez, cx, cy, cz);
      };

      const __m256d s0d = crossdot(v21x, v21y, v21z, fnx, fny, fnz, p1x, p1y, p1z);
      const __m256d s1d = crossdot(v32x, v32y, v32z, fnx, fny, fnz, p2x, p2y, p2z);
      const __m256d s2d = crossdot(v13x, v13y, v13z, fnx, fny, fnz, p3x, p3y, p3z);

      const __m256d pos_mask = _mm256_set1_pd(-0.0);
      const __m256d s0       = _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, s0d));
      const __m256d s1       = _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, s1d));
      const __m256d s2       = _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, s2d));

      const __m256d ssum   = _mm256_add_pd(_mm256_add_pd(s0, s1), s2);
      const __m256d in_tri = _mm256_cmp_pd(_mm256_andnot_pd(pos_mask, ssum), _mm256_set1_pd(2.0), _CMP_GE_OQ);
      const __m256d face_d = dot3(fnx, fny, fnz, p1x, p1y, p1z);

      const __m256d dp1v21  = dot3(p1x, p1y, p1z, v21x, v21y, v21z);
      const __m256d dv21v21 = dot3(v21x, v21y, v21z, v21x, v21y, v21z);
      const __m256d t1      = _mm256_div_pd(dp1v21, dv21v21);
      const __m256d y1x     = _mm256_sub_pd(p1x, _mm256_mul_pd(t1, v21x));
      const __m256d y1y     = _mm256_sub_pd(p1y, _mm256_mul_pd(t1, v21y));
      const __m256d y1z     = _mm256_sub_pd(p1z, _mm256_mul_pd(t1, v21z));

      const __m256d dp2v32  = dot3(p2x, p2y, p2z, v32x, v32y, v32z);
      const __m256d dv32v32 = dot3(v32x, v32y, v32z, v32x, v32y, v32z);
      const __m256d t2      = _mm256_div_pd(dp2v32, dv32v32);
      const __m256d y2x     = _mm256_sub_pd(p2x, _mm256_mul_pd(t2, v32x));
      const __m256d y2y     = _mm256_sub_pd(p2y, _mm256_mul_pd(t2, v32y));
      const __m256d y2z     = _mm256_sub_pd(p2z, _mm256_mul_pd(t2, v32z));

      const __m256d dp3v13  = dot3(p3x, p3y, p3z, v13x, v13y, v13z);
      const __m256d dv13v13 = dot3(v13x, v13y, v13z, v13x, v13y, v13z);
      const __m256d t3      = _mm256_div_pd(dp3v13, dv13v13);
      const __m256d y3x     = _mm256_sub_pd(p3x, _mm256_mul_pd(t3, v13x));
      const __m256d y3y     = _mm256_sub_pd(p3y, _mm256_mul_pd(t3, v13y));
      const __m256d y3z     = _mm256_sub_pd(p3z, _mm256_mul_pd(t3, v13z));

      const __m256d vn0x = _mm256_load_pd(vnx[0]), vn0y = _mm256_load_pd(vny[0]), vn0z = _mm256_load_pd(vnz[0]);
      const __m256d vn1x = _mm256_load_pd(vnx[1]), vn1y = _mm256_load_pd(vny[1]), vn1z = _mm256_load_pd(vnz[1]);
      const __m256d vn2x = _mm256_load_pd(vnx[2]), vn2y = _mm256_load_pd(vny[2]), vn2z = _mm256_load_pd(vnz[2]);
      const __m256d en0x = _mm256_load_pd(enx[0]), en0y = _mm256_load_pd(eny[0]), en0z = _mm256_load_pd(enz[0]);
      const __m256d en1x = _mm256_load_pd(enx[1]), en1y = _mm256_load_pd(eny[1]), en1z = _mm256_load_pd(enz[1]);
      const __m256d en2x = _mm256_load_pd(enx[2]), en2y = _mm256_load_pd(eny[2]), en2z = _mm256_load_pd(enz[2]);

      const __m256d p1d2 = dot3(p1x, p1y, p1z, p1x, p1y, p1z);
      const __m256d p2d2 = dot3(p2x, p2y, p2z, p2x, p2y, p2z);
      const __m256d p3d2 = dot3(p3x, p3y, p3z, p3x, p3y, p3z);
      const __m256d y1d2 = dot3(y1x, y1y, y1z, y1x, y1y, y1z);
      const __m256d y2d2 = dot3(y2x, y2y, y2z, y2x, y2y, y2z);
      const __m256d y3d2 = dot3(y3x, y3y, y3z, y3x, y3y, y3z);

      // Extract sign bits only — sqrt is deferred to a single call after the blendv chain.
      auto sgn_of =
        [&](const __m256d ax, const __m256d ay, const __m256d az, const __m256d bx, const __m256d by, const __m256d bz)
        -> __m256d {
        const __m256d d = dot3(ax, ay, az, bx, by, bz);
        return _mm256_or_pd(_mm256_andnot_pd(pos_mask, one), _mm256_and_pd(pos_mask, d));
      };

      const __m256d vs0 = sgn_of(vn0x, vn0y, vn0z, p1x, p1y, p1z);
      const __m256d vs1 = sgn_of(vn1x, vn1y, vn1z, p2x, p2y, p2z);
      const __m256d vs2 = sgn_of(vn2x, vn2y, vn2z, p3x, p3y, p3z);
      const __m256d es0 = sgn_of(en0x, en0y, en0z, y1x, y1y, y1z);
      const __m256d es1 = sgn_of(en1x, en1y, en1z, y2x, y2y, y2z);
      const __m256d es2 = sgn_of(en2x, en2y, en2z, y3x, y3y, y3z);

      const __m256d t1_valid = _mm256_and_pd(_mm256_cmp_pd(t1, zer, _CMP_GT_OQ), _mm256_cmp_pd(t1, one, _CMP_LT_OQ));
      const __m256d t2_valid = _mm256_and_pd(_mm256_cmp_pd(t2, zer, _CMP_GT_OQ), _mm256_cmp_pd(t2, one, _CMP_LT_OQ));
      const __m256d t3_valid = _mm256_and_pd(_mm256_cmp_pd(t3, zer, _CMP_GT_OQ), _mm256_cmp_pd(t3, one, _CMP_LT_OQ));

      // Blendv chain: track (best_d2, best_sgn). No sqrt until the end.
      __m256d best_d2  = p1d2;
      __m256d best_sgn = vs0;

      const __m256d mask1 = _mm256_cmp_pd(p2d2, best_d2, _CMP_LT_OQ);
      best_sgn            = _mm256_blendv_pd(best_sgn, vs1, mask1);
      best_d2             = _mm256_blendv_pd(best_d2, p2d2, mask1);

      const __m256d mask2 = _mm256_cmp_pd(p3d2, best_d2, _CMP_LT_OQ);
      best_sgn            = _mm256_blendv_pd(best_sgn, vs2, mask2);
      best_d2             = _mm256_blendv_pd(best_d2, p3d2, mask2);

      const __m256d mask_e0 = _mm256_and_pd(t1_valid, _mm256_cmp_pd(y1d2, best_d2, _CMP_LT_OQ));
      best_sgn              = _mm256_blendv_pd(best_sgn, es0, mask_e0);
      best_d2               = _mm256_blendv_pd(best_d2, y1d2, mask_e0);

      const __m256d mask_e1 = _mm256_and_pd(t2_valid, _mm256_cmp_pd(y2d2, best_d2, _CMP_LT_OQ));
      best_sgn              = _mm256_blendv_pd(best_sgn, es1, mask_e1);
      best_d2               = _mm256_blendv_pd(best_d2, y2d2, mask_e1);

      const __m256d mask_e2 = _mm256_and_pd(t3_valid, _mm256_cmp_pd(y3d2, best_d2, _CMP_LT_OQ));
      best_sgn              = _mm256_blendv_pd(best_sgn, es2, mask_e2);
      best_d2               = _mm256_blendv_pd(best_d2, y3d2, mask_e2);

      // Single sqrt for all vertex/edge lanes, then face override.
      const __m256d ev_d   = _mm256_mul_pd(best_sgn, _mm256_sqrt_pd(best_d2));
      const __m256d best_d = _mm256_blendv_pd(ev_d, face_d, in_tri);

      alignas(32) double d4[4];
      _mm256_store_pd(d4, best_d);

      double best = std::numeric_limits<double>::max();
      double babs = std::numeric_limits<double>::max();
      for (uint32_t i = 0; i < validCount; i++) {
        const double ad = std::abs(d4[i]);
        if (ad < babs) {
          best = d4[i];
          babs = ad;
        }
      }
      return static_cast<T>(best);
    }
#endif

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
      if (std::abs(ss0 + ss1 + ss2) >= T(2)) {
        d = n.dot(p1);
      }
      else {
        T        r2 = p1.dot(p1);
        Vec3T<T> rv = p1;
        Vec3T<T> rn = vn0;

        const T p2d = p2.dot(p2);
        if (p2d < r2) {
          r2 = p2d;
          rv = p2;
          rn = vn1;
        }
        const T p3d = p3.dot(p3);
        if (p3d < r2) {
          r2 = p3d;
          rv = p3;
          rn = vn2;
        }

        const T t1 = dot(p1, v21) / dot(v21, v21);
        if (t1 > T(0) && t1 < T(1)) {
          const Vec3T<T> y1 = p1 - t1 * v21;
          const T        yd = y1.dot(y1);
          if (yd < r2) {
            r2 = yd;
            rv = y1;
            rn = en0;
          }
        }
        const T t2 = dot(p2, v32) / dot(v32, v32);
        if (t2 > T(0) && t2 < T(1)) {
          const Vec3T<T> y2 = p2 - t2 * v32;
          const T        yd = y2.dot(y2);
          if (yd < r2) {
            r2 = yd;
            rv = y2;
            rn = en1;
          }
        }
        const T t3 = dot(p3, v13) / dot(v13, v13);
        if (t3 > T(0) && t3 < T(1)) {
          const Vec3T<T> y3 = p3 - t3 * v13;
          const T        yd = y3.dot(y3);
          if (yd < r2) {
            r2 = yd;
            rv = y3;
            rn = en2;
          }
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
