// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TriangleRay.hpp
 * @brief  The one triangle ray-crossing kernel, written once for a scalar and for every SIMD::Pack.
 * @details TriangleSoAT's lanes and SIMD groups call TriangleRay::crossing(), with V = T for one triangle
 * and V = SIMD::Pack<T, N> for N triangles at once.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRIANGLERAY_HPP
#define EBGEOMETRY_TRIANGLERAY_HPP

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_SIMD.hpp"
#include "EBGeometry_TriangleDistance.hpp"

namespace EBGeometry {

/**
 * @brief The triangle ray-crossing kernel.
 */
namespace TriangleRay {

/**
 * @brief Where a ray crosses a triangle, or one triangle per lane (Möller–Trumbore).
 * @details The ray is origin + t * direction. A crossing counts where t is in [a_tMin, a_tMax], from
 * either side of the triangle. The barycentric tests are widened by the rounding error of the crossing
 * point, a distance of a few ulps of the ray origin's and the triangle's coordinates, so that a ray through
 * an edge or vertex shared by two triangles crosses at least one of them however far it comes from; a ray
 * in the triangle's plane does not cross it. A ray passing outside a triangle within that distance counts
 * as crossing it (on the safe side, as a grazing marched ray does). The triangle is read through @p a_triangle, which provides vertex(k) for k = 0, 1, 2 as
 * TriangleDistance::Vec3V<V> (see TriangleDistance::signedDistance()). For a SIMD::Pack every lane
 * computes every test, so the kernel has no branches.
 * @tparam V      Scalar (float or double) or SIMD::Pack.
 * @tparam Source Triangle accessor.
 * @param[in] a_triangle  Triangle accessor.
 * @param[in] a_origin    Ray origin, the same in every lane.
 * @param[in] a_direction Ray direction, the same in every lane.
 * @param[in] a_tMin      Start of the interval.
 * @param[in] a_tMax      End of the interval.
 * @return The distance along the ray of the crossing, or +infinity where there is none.
 */
template <class V, class Source>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline V
crossing(const Source&                     a_triangle,
         const TriangleDistance::Vec3V<V>& a_origin,
         const TriangleDistance::Vec3V<V>& a_direction,
         const V                           a_tMin,
         const V                           a_tMax) noexcept
{
  using SIMD::abs;
  using SIMD::both;
  using SIMD::select;
  using SIMD::sqrt;
  using TriangleDistance::constant;
  using TriangleDistance::cross;
  using TriangleDistance::dot;
  using TriangleDistance::Vec3V;

  using T = typename SIMD::Scalar<V>::type;

  const Vec3V<V> v0 = a_triangle.vertex(0);
  const Vec3V<V> e1 = a_triangle.vertex(1) - v0;
  const Vec3V<V> e2 = a_triangle.vertex(2) - v0;

  const Vec3V<V> pv  = cross(a_direction, e2);
  const V        det = dot(e1, pv);
  const V        inv = constant<V>(1) / det;

  const Vec3V<V> s = a_origin - v0;
  const V        u = dot(s, pv) * inv;

  const Vec3V<V> q = cross(s, e1);
  const V        v = dot(a_direction, q) * inv;
  const V        t = dot(e2, q) * inv;

  // The barycentric tests are widened by the rounding error of the crossing point, a distance delta of a few
  // ulps of the coordinates, as each coordinate measures it: u by the height of vertex 1 over the edge v0-v2
  // (twice the area over |e2|), v by that of vertex 2 over v0-v1, and 1 - u - v by that of vertex 0 over
  // v1-v2. A point on an edge shared by two triangles is then inside both, however far the ray comes from;
  // the widening does not grow for a ray nearly in the triangle's plane. (The edge lengths are L1 norms,
  // which only widens a little more.) A zero det (a ray in the plane) makes u, v and t infinite or NaN,
  // which every test below rejects.
  const Vec3V<V> n     = cross(e1, e2);
  const V        area2 = sqrt(dot(n, n));
  const V        ulps  = V(T(16) * Math::Limits<T>::epsilon());
  const V delta   = ulps * (abs(a_origin.x) + abs(a_origin.y) + abs(a_origin.z) + abs(v0.x) + abs(v0.y) + abs(v0.z));
  const V perArea = delta / area2;

  const V tolU = ulps + perArea * (abs(e2.x) + abs(e2.y) + abs(e2.z));
  const V tolV = ulps + perArea * (abs(e1.x) + abs(e1.y) + abs(e1.z));
  const V tolW = ulps + perArea * (abs(e2.x - e1.x) + abs(e2.y - e1.y) + abs(e2.z - e1.z));

  const auto inside =
    both(both(u >= constant<V>(0) - tolU, v >= constant<V>(0) - tolV), constant<V>(1) + tolW >= u + v);
  const auto inRange = both(t >= a_tMin, a_tMax >= t);
  const auto crosses = both(both(inside, inRange), abs(det) > constant<V>(0));

  return select(crosses, t, V(Math::Limits<T>::infinity()));
}

} // namespace TriangleRay

} // namespace EBGeometry

#endif
