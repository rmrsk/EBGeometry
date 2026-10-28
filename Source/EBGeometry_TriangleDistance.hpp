// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TriangleDistance.hpp
 * @brief  The one triangle signed-distance kernel, written once for a scalar and for every SIMD::Pack.
 * @details Triangle::signedDistance(), TriangleSoAT's lanes and TriangleSoAT's SIMD groups all call
 * TriangleDistance::signedDistance(), with V = T for one triangle and V = SIMD::Pack<T, N> for N
 * triangles at once.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRIANGLEDISTANCE_HPP
#define EBGEOMETRY_TRIANGLEDISTANCE_HPP

// Std includes
#include <type_traits>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_SIMD.hpp"

namespace EBGeometry {

/**
 * @brief The triangle signed-distance kernel.
 */
namespace TriangleDistance {

/**
 * @brief A 3D vector of V, where V is a scalar or a SIMD::Pack: x, y and z each hold one lane per
 * triangle.
 * @tparam V Scalar or pack type.
 */
template <class V>
struct Vec3V
{
  /**
   * @brief x components.
   */
  V x;

  /**
   * @brief y components.
   */
  V y;

  /**
   * @brief z components.
   */
  V z;
};

/// @cond
template <class V>
EBGEOMETRY_HOST_DEVICE
inline Vec3V<V>
operator-(const Vec3V<V>& a_u, const Vec3V<V>& a_v) noexcept
{
  return Vec3V<V>{a_u.x - a_v.x, a_u.y - a_v.y, a_u.z - a_v.z};
}

template <class V>
EBGEOMETRY_HOST_DEVICE
inline V
dot(const Vec3V<V>& a_u, const Vec3V<V>& a_v) noexcept
{
  return a_u.x * a_v.x + a_u.y * a_v.y + a_u.z * a_v.z;
}

template <class V>
EBGEOMETRY_HOST_DEVICE
inline Vec3V<V>
cross(const Vec3V<V>& a_u, const Vec3V<V>& a_v) noexcept
{
  return Vec3V<V>{a_u.y * a_v.z - a_u.z * a_v.y, a_u.z * a_v.x - a_u.x * a_v.z, a_u.x * a_v.y - a_u.y * a_v.x};
}

template <class V>
EBGEOMETRY_HOST_DEVICE
inline Vec3V<V>
scaled(const Vec3V<V>& a_u, const V a_s) noexcept
{
  return Vec3V<V>{a_s * a_u.x, a_s * a_u.y, a_s * a_u.z};
}

template <class V>
EBGEOMETRY_HOST_DEVICE
inline V
constant(const int a_value) noexcept
{
  using T = typename SIMD::Scalar<V>::type;

  return V(T(a_value));
}

// +1 where a_x > 0, else -1 (zero and NaN included).
template <class V>
EBGEOMETRY_HOST_DEVICE
inline V
sign(const V a_x) noexcept
{
  using SIMD::select;

  return select(a_x > constant<V>(0), constant<V>(1), constant<V>(-1));
}

// Whether a projection parameter lies strictly inside its edge, 0 < t < 1.
template <class V>
EBGEOMETRY_HOST_DEVICE
inline auto
onEdge(const V a_t) noexcept
{
  using SIMD::both;

  return both(a_t > constant<V>(0), a_t < constant<V>(1));
}
/// @endcond

/**
 * @brief Signed distance from @p a_point to a triangle, or to one triangle per lane.
 * @details Where the point projects inside the triangle, the distance is the signed distance to its
 * plane. Elsewhere it is the distance to the closest vertex or edge, signed by that feature's
 * pseudonormal. The candidates are compared by squared distance in the order vertex 0, 1, 2 and edge
 * 0, 1, 2, a later one replacing the best only when strictly closer, and there is one square root, at
 * the end. A sign test (s > 0) gives -1 at zero and at NaN. For a SIMD::Pack every lane computes
 * every candidate, so the kernel has no branches. For a scalar (a device thread, Triangle, a build
 * without SIMD) it branches instead: it returns at once for a point projecting inside the face, and
 * signs only the closest candidate, with the same candidates, order and arithmetic, so the result
 * is the same.
 *
 * The triangle is read through @p a_triangle, at the point each value is needed, so that a SIMD
 * caller loads straight from its arrays into registers rather than staging the whole triangle in
 * memory first. It provides, as Vec3V<V> by value:
 *
 * - vertex(k): vertex position k, for k = 0, 1, 2;
 * - normal(): the face normal;
 * - vertexNormal(k): the pseudonormal of vertex k;
 * - edgeNormal(k): the pseudonormal of edge k, from vertex k to vertex k + 1 (mod 3).
 * @tparam V      Scalar (float or double) or SIMD::Pack.
 * @tparam Source Triangle accessor, as above.
 * @param[in] a_triangle Triangle accessor.
 * @param[in] a_point    Query point, the same in every lane.
 * @return The signed distance, negative behind the face normal.
 */
template <class V, class Source>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline V
signedDistance(const Source& a_triangle, const Vec3V<V>& a_point) noexcept
{
  using SIMD::abs;
  using SIMD::both;
  using SIMD::select;
  using SIMD::sqrt;

  const Vec3V<V> v0 = a_triangle.vertex(0);
  const Vec3V<V> v1 = a_triangle.vertex(1);
  const Vec3V<V> v2 = a_triangle.vertex(2);
  const Vec3V<V> n  = a_triangle.normal();

  const Vec3V<V> v21 = v1 - v0;
  const Vec3V<V> v32 = v2 - v1;
  const Vec3V<V> v13 = v0 - v2;

  const Vec3V<V> p1 = a_point - v0;
  const Vec3V<V> p2 = a_point - v1;
  const Vec3V<V> p3 = a_point - v2;

  // With outward normals the three signs are all -1 for a point projecting inside (sum -3), with
  // inward normals all +1 (sum +3).
  const V s0 = sign(dot(cross(v21, n), p1));
  const V s1 = sign(dot(cross(v32, n), p2));
  const V s2 = sign(dot(cross(v13, n), p3));

  const auto inside = abs(s0 + s1 + s2) >= constant<V>(2);
  const V    faceD  = dot(n, p1);

  if constexpr (std::is_floating_point_v<V>) {
    // One lane can branch where a SIMD lane cannot. A point projecting inside the face needs no
    // vertex or edge, and only the closest candidate needs its sign: on a device this skips most of
    // the double-precision work, which bounds the kernel there.
    if (inside) {
      return faceD;
    }

    // The closest candidate so far: its squared distance, the vector from it to the point, and
    // which pseudonormal signs it (0-2 vertices, 3-5 edges). An edge's closest point is computed only
    // where the point projects onto the edge (0 < t < 1).
    V        bestD2  = dot(p1, p1);
    Vec3V<V> bestVec = p1;
    int      best    = 0;

    const V p2d2 = dot(p2, p2);

    if (p2d2 < bestD2) {
      bestD2  = p2d2;
      bestVec = p2;
      best    = 1;
    }

    const V p3d2 = dot(p3, p3);

    if (p3d2 < bestD2) {
      bestD2  = p3d2;
      bestVec = p3;
      best    = 2;
    }

    const V t1 = dot(p1, v21) / dot(v21, v21);

    if (onEdge(t1)) {
      const Vec3V<V> y1   = p1 - scaled(v21, t1);
      const V        y1d2 = dot(y1, y1);

      if (y1d2 < bestD2) {
        bestD2  = y1d2;
        bestVec = y1;
        best    = 3;
      }
    }

    const V t2 = dot(p2, v32) / dot(v32, v32);

    if (onEdge(t2)) {
      const Vec3V<V> y2   = p2 - scaled(v32, t2);
      const V        y2d2 = dot(y2, y2);

      if (y2d2 < bestD2) {
        bestD2  = y2d2;
        bestVec = y2;
        best    = 4;
      }
    }

    const V t3 = dot(p3, v13) / dot(v13, v13);

    if (onEdge(t3)) {
      const Vec3V<V> y3   = p3 - scaled(v13, t3);
      const V        y3d2 = dot(y3, y3);

      if (y3d2 < bestD2) {
        bestD2  = y3d2;
        bestVec = y3;
        best    = 5;
      }
    }

    const auto     k            = static_cast<size_t>(best);
    const Vec3V<V> pseudonormal = k < 3 ? a_triangle.vertexNormal(k) : a_triangle.edgeNormal(k - 3);

    return sign(dot(pseudonormal, bestVec)) * sqrt(bestD2);
  }

  const V t1 = dot(p1, v21) / dot(v21, v21);
  const V t2 = dot(p2, v32) / dot(v32, v32);
  const V t3 = dot(p3, v13) / dot(v13, v13);

  const Vec3V<V> y1 = p1 - scaled(v21, t1);
  const Vec3V<V> y2 = p2 - scaled(v32, t2);
  const Vec3V<V> y3 = p3 - scaled(v13, t3);

  // Vertices, then edges; a candidate replaces the best only when strictly closer, and an edge counts
  // only where the point projects inside it (0 < t < 1).
  V bestD2   = dot(p1, p1);
  V bestSign = sign(dot(a_triangle.vertexNormal(0), p1));

  const V    p2d2 = dot(p2, p2);
  const auto c2   = p2d2 < bestD2;
  bestSign        = select(c2, sign(dot(a_triangle.vertexNormal(1), p2)), bestSign);
  bestD2          = select(c2, p2d2, bestD2);

  const V    p3d2 = dot(p3, p3);
  const auto c3   = p3d2 < bestD2;
  bestSign        = select(c3, sign(dot(a_triangle.vertexNormal(2), p3)), bestSign);
  bestD2          = select(c3, p3d2, bestD2);

  const V    y1d2 = dot(y1, y1);
  const auto e1   = both(onEdge(t1), y1d2 < bestD2);
  bestSign        = select(e1, sign(dot(a_triangle.edgeNormal(0), y1)), bestSign);
  bestD2          = select(e1, y1d2, bestD2);

  const V    y2d2 = dot(y2, y2);
  const auto e2   = both(onEdge(t2), y2d2 < bestD2);
  bestSign        = select(e2, sign(dot(a_triangle.edgeNormal(1), y2)), bestSign);
  bestD2          = select(e2, y2d2, bestD2);

  const V    y3d2 = dot(y3, y3);
  const auto e3   = both(onEdge(t3), y3d2 < bestD2);
  bestSign        = select(e3, sign(dot(a_triangle.edgeNormal(2), y3)), bestSign);
  bestD2          = select(e3, y3d2, bestD2);

  return select(inside, faceD, bestSign * sqrt(bestD2));
}

/**
 * @brief The squared distance from a point to a triangle, without its sign: the candidates of
 * signedDistance() (the face where the point projects inside it, else the nearest vertex or edge), none
 * signed.
 * @details A nearest-triangle search compares these and signs only the winner, with signedDistance() on
 * that triangle: the six pseudonormal dot products and the square root are then paid once per query, not
 * once per triangle. The square root of this value equals |signedDistance()| to rounding, so the winner is
 * the triangle a search on signedDistance() picks, except where two triangles are equally near to rounding. With V a scalar, the inside case returns early, as signedDistance()'s does.
 * @tparam V      Scalar or SIMD::Pack.
 * @tparam Source A triangle accessor (see signedDistance()).
 * @param[in] a_triangle The triangle(s).
 * @param[in] a_point    The point, broadcast to every lane.
 * @return The squared distance, one per lane.
 */
template <class V, class Source>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline V
unsignedDistance2(const Source& a_triangle, const Vec3V<V>& a_point) noexcept
{
  using SIMD::abs;
  using SIMD::both;
  using SIMD::select;

  const Vec3V<V> v0 = a_triangle.vertex(0);
  const Vec3V<V> v1 = a_triangle.vertex(1);
  const Vec3V<V> v2 = a_triangle.vertex(2);
  const Vec3V<V> n  = a_triangle.normal();

  const Vec3V<V> v21 = v1 - v0;
  const Vec3V<V> v32 = v2 - v1;
  const Vec3V<V> v13 = v0 - v2;

  const Vec3V<V> p1 = a_point - v0;
  const Vec3V<V> p2 = a_point - v1;
  const Vec3V<V> p3 = a_point - v2;

  const V s0 = sign(dot(cross(v21, n), p1));
  const V s1 = sign(dot(cross(v32, n), p2));
  const V s2 = sign(dot(cross(v13, n), p3));

  const auto inside = abs(s0 + s1 + s2) >= constant<V>(2);
  const V    faceD  = dot(n, p1);

  if constexpr (std::is_floating_point_v<V>) {
    if (inside) {
      return faceD * faceD;
    }

    V bestD2 = dot(p1, p1);

    const V p2d2 = dot(p2, p2);
    const V p3d2 = dot(p3, p3);

    bestD2 = p2d2 < bestD2 ? p2d2 : bestD2;
    bestD2 = p3d2 < bestD2 ? p3d2 : bestD2;

    const V t1 = dot(p1, v21) / dot(v21, v21);

    if (onEdge(t1)) {
      const Vec3V<V> y1   = p1 - scaled(v21, t1);
      const V        y1d2 = dot(y1, y1);

      bestD2 = y1d2 < bestD2 ? y1d2 : bestD2;
    }

    const V t2 = dot(p2, v32) / dot(v32, v32);

    if (onEdge(t2)) {
      const Vec3V<V> y2   = p2 - scaled(v32, t2);
      const V        y2d2 = dot(y2, y2);

      bestD2 = y2d2 < bestD2 ? y2d2 : bestD2;
    }

    const V t3 = dot(p3, v13) / dot(v13, v13);

    if (onEdge(t3)) {
      const Vec3V<V> y3   = p3 - scaled(v13, t3);
      const V        y3d2 = dot(y3, y3);

      bestD2 = y3d2 < bestD2 ? y3d2 : bestD2;
    }

    return bestD2;
  }

  const V t1 = dot(p1, v21) / dot(v21, v21);
  const V t2 = dot(p2, v32) / dot(v32, v32);
  const V t3 = dot(p3, v13) / dot(v13, v13);

  const Vec3V<V> y1 = p1 - scaled(v21, t1);
  const Vec3V<V> y2 = p2 - scaled(v32, t2);
  const Vec3V<V> y3 = p3 - scaled(v13, t3);

  V bestD2 = dot(p1, p1);

  const V p2d2 = dot(p2, p2);
  bestD2       = select(p2d2 < bestD2, p2d2, bestD2);

  const V p3d2 = dot(p3, p3);
  bestD2       = select(p3d2 < bestD2, p3d2, bestD2);

  const V y1d2 = dot(y1, y1);
  bestD2       = select(both(onEdge(t1), y1d2 < bestD2), y1d2, bestD2);

  const V y2d2 = dot(y2, y2);
  bestD2       = select(both(onEdge(t2), y2d2 < bestD2), y2d2, bestD2);

  const V y3d2 = dot(y3, y3);
  bestD2       = select(both(onEdge(t3), y3d2 < bestD2), y3d2, bestD2);

  return select(inside, faceD * faceD, bestD2);
}

} // namespace TriangleDistance

} // namespace EBGeometry

#endif
