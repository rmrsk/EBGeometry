// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_AnalyticRayFormsImplem.hpp
 * @brief  Implementation of EBGeometry_AnalyticRayForms.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_ANALYTICRAYFORMSIMPLEM_HPP
#define EBGEOMETRY_ANALYTICRAYFORMSIMPLEM_HPP

// Std includes
#include <cmath>
#include <type_traits>

// Our includes
#include "EBGeometry_AnalyticRayForms.hpp"

namespace EBGeometry {

namespace RayDetail {

/**
 * @brief The first crossing of a convex object along a ray, from the span [a_in, a_out] the ray is inside
 * it: entering at a_in if that is in [a_tMin, a_tMax], or, for a ray that starts inside, leaving at a_out.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_ray       Ray.
 * @param[in] a_in        Where the ray enters (or -infinity).
 * @param[in] a_inNormal  Outward normal there.
 * @param[in] a_out       Where it leaves (or +infinity); a_out < a_in for a ray that misses.
 * @param[in] a_outNormal Outward normal there.
 * @param[in] a_tMin      Start of the interval.
 * @param[in] a_tMax      End of the interval.
 * @return The crossing, or a miss.
 * @details Always inlined on the host, as are ballSpan() and tubeSpan(): several ray forms call each, and a tape
 * instantiates them all, so left to itself GCC keeps them out of line, and a fused composition's trace pays a call
 * per leaf.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline HitT<T>
convexHit(const RayT<T>&  a_ray,
          const T         a_in,
          const Vec3T<T>& a_inNormal,
          const T         a_out,
          const Vec3T<T>& a_outNormal,
          const T         a_tMin,
          const T         a_tMax) noexcept
{
  HitT<T> hit;

  if (!(a_in <= a_out)) {
    return hit;
  }

  const bool entering = a_in >= a_tMin && a_in <= a_tMax;
  const bool leaving  = !entering && a_in < a_tMin && a_out >= a_tMin && a_out <= a_tMax;

  if (entering || leaving) {
    hit.m_status   = HitStatus::Hit;
    hit.m_t        = entering ? a_in : a_out;
    hit.m_point    = a_ray.at(hit.m_t);
    hit.m_entering = entering;
    hit.m_exact    = true;
    hit.m_normal   = entering ? a_inNormal : a_outNormal;
  }

  return hit;
}

/**
 * @brief Where a ray is inside a slab lo <= x[axis] <= hi, with the normals at its ends.
 * @details Internal helper; not part of the public API. A ray along the slab is inside it everywhere or
 * nowhere.
 * @tparam T Floating-point precision.
 * @param[in]     a_ray       Ray.
 * @param[in]     a_axis      Axis.
 * @param[in]     a_lo        Low side.
 * @param[in]     a_hi        High side.
 * @param[in,out] a_in        Raised to where the ray enters the slab.
 * @param[in,out] a_inNormal  Set to the outward normal there, if raised.
 * @param[in,out] a_out       Lowered to where it leaves.
 * @param[in,out] a_outNormal Set to the outward normal there, if lowered.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
slab(const RayT<T>& a_ray,
     const size_t   a_axis,
     const T        a_lo,
     const T        a_hi,
     T&             a_in,
     Vec3T<T>&      a_inNormal,
     T&             a_out,
     Vec3T<T>&      a_outNormal) noexcept
{
  const T o = a_ray.m_origin[a_axis];
  const T d = a_ray.m_direction[a_axis];

  if (d == T(0)) {
    if (o < a_lo || o > a_hi) {
      a_in  = Math::Limits<T>::infinity();
      a_out = -Math::Limits<T>::infinity();
    }

    return;
  }

  const T tLow  = (a_lo - o) / d;
  const T tHigh = (a_hi - o) / d;

  Vec3T<T> nLo = Vec3T<T>::zeros();
  Vec3T<T> nHi = Vec3T<T>::zeros();

  nLo[a_axis] = T(-1);
  nHi[a_axis] = T(1);

  const T        tNear = (d > T(0)) ? tLow : tHigh;
  const T        tFar  = (d > T(0)) ? tHigh : tLow;
  const Vec3T<T> nNear = (d > T(0)) ? nLo : nHi;
  const Vec3T<T> nFar  = (d > T(0)) ? nHi : nLo;

  if (tNear > a_in) {
    a_in       = tNear;
    a_inNormal = nNear;
  }
  if (tFar < a_out) {
    a_out       = tFar;
    a_outNormal = nFar;
  }
}

/**
 * @brief Where a ray is inside a sphere.
 * @details Internal helper; not part of the public API. The discriminant is the squared radius less the
 * squared distance of the center from the ray's line, |oc x d|^2, not b^2 - c: for a small ball far from
 * the origin, |oc|^2 - r^2 cancels, and in float a ray passing within about 1e-3 of the radius of the
 * surface could go either way. The nearer root is c / q, which does not cancel either.
 * @tparam T Floating-point precision.
 * @param[in]  a_ray    Ray.
 * @param[in]  a_center Center.
 * @param[in]  a_radius Radius.
 * @param[out] a_in     Where it enters.
 * @param[out] a_out    Where it leaves (below a_in if it misses).
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline void
ballSpan(const RayT<T>& a_ray, const Vec3T<T>& a_center, const T a_radius, T& a_in, T& a_out) noexcept
{
  const Vec3T<T> oc   = a_ray.m_origin - a_center;
  const Vec3T<T> l    = cross(oc, a_ray.m_direction);
  const T        b    = dot(oc, a_ray.m_direction);
  const T        disc = a_radius * a_radius - dot(l, l);

  if (disc < T(0)) {
    a_in  = Math::Limits<T>::infinity();
    a_out = -Math::Limits<T>::infinity();

    return;
  }

  const T root = std::sqrt(disc);
  const T q    = -b - ((b >= T(0)) ? root : -root);

  // Both roots are zero: the ray starts on the surface, along it.
  if (q == T(0)) {
    a_in  = T(0);
    a_out = T(0);

    return;
  }

  const T c = dot(oc, oc) - a_radius * a_radius;

  a_in  = Math::min(q, c / q);
  a_out = Math::max(q, c / q);
}

/**
 * @brief Where a ray is inside an infinite cylinder around the line through a point along a unit axis.
 * @details Internal helper; not part of the public API. A ray along the axis is inside it everywhere or
 * nowhere. As for a sphere (see ballSpan()), the discriminant is a r^2 - |o x d|^2 rather than b^2 - a c,
 * and the nearer root c / q, so that a thin cylinder far from the origin does not cancel.
 * @tparam T Floating-point precision.
 * @param[in]  a_ray    Ray.
 * @param[in]  a_point  Point on the axis.
 * @param[in]  a_axis   Unit axis.
 * @param[in]  a_radius Radius.
 * @param[out] a_in     Where it enters.
 * @param[out] a_out    Where it leaves (below a_in if it misses).
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_ALWAYS_INLINE
inline void
tubeSpan(
  const RayT<T>& a_ray, const Vec3T<T>& a_point, const Vec3T<T>& a_axis, const T a_radius, T& a_in, T& a_out) noexcept
{
  const Vec3T<T> oc = a_ray.m_origin - a_point;
  const Vec3T<T> d  = a_ray.m_direction - dot(a_ray.m_direction, a_axis) * a_axis;
  const Vec3T<T> o  = oc - dot(oc, a_axis) * a_axis;

  const T a = dot(d, d);
  const T c = dot(o, o) - a_radius * a_radius;

  if (a == T(0)) {
    a_in  = (c <= T(0)) ? -Math::Limits<T>::infinity() : Math::Limits<T>::infinity();
    a_out = (c <= T(0)) ? Math::Limits<T>::infinity() : -Math::Limits<T>::infinity();

    return;
  }

  const Vec3T<T> l    = cross(o, d);
  const T        b    = dot(o, d);
  const T        disc = a * a_radius * a_radius - dot(l, l);

  if (disc < T(0)) {
    a_in  = Math::Limits<T>::infinity();
    a_out = -Math::Limits<T>::infinity();

    return;
  }

  const T root = std::sqrt(disc);
  const T q    = -b - ((b >= T(0)) ? root : -root);

  // Both roots are zero: the ray starts on the surface, along it.
  if (q == T(0)) {
    a_in  = T(0);
    a_out = T(0);

    return;
  }

  a_in  = Math::min(q / a, c / q);
  a_out = Math::max(q / a, c / q);
}

/**
 * @brief The outward normal of an infinite cylinder at a point on its surface.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_p     Point.
 * @param[in] a_point Point on the axis.
 * @param[in] a_axis  Unit axis.
 * @return The unit normal.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
tubeNormal(const Vec3T<T>& a_p, const Vec3T<T>& a_point, const Vec3T<T>& a_axis) noexcept
{
  const Vec3T<T> r      = a_p - a_point;
  const Vec3T<T> radial = r - dot(r, a_axis) * a_axis;
  const T        length = radial.length();

  return (length > T(0)) ? radial / length : Vec3T<T>::zeros();
}

/**
 * @brief Where a ray is inside a finite cylinder between two centers, with the normals at its ends.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in]  a_ray       Ray.
 * @param[in]  a_center1   Center of one cap.
 * @param[in]  a_center2   Center of the other.
 * @param[in]  a_radius    Radius.
 * @param[out] a_in        Where it enters.
 * @param[out] a_inNormal  Outward normal there.
 * @param[out] a_out       Where it leaves (below a_in if it misses).
 * @param[out] a_outNormal Outward normal there.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
cylinderSpan(const RayT<T>&  a_ray,
             const Vec3T<T>& a_center1,
             const Vec3T<T>& a_center2,
             const T         a_radius,
             T&              a_in,
             Vec3T<T>&       a_inNormal,
             T&              a_out,
             Vec3T<T>&       a_outNormal) noexcept
{
  const Vec3T<T> delta  = a_center2 - a_center1;
  const T        length = delta.length();
  const Vec3T<T> axis   = delta / length;

  tubeSpan(a_ray, a_center1, axis, a_radius, a_in, a_out);

  a_inNormal  = tubeNormal(a_ray.at(a_in), a_center1, axis);
  a_outNormal = tubeNormal(a_ray.at(a_out), a_center1, axis);

  // Clip by the slab 0 <= (x - center1) . axis <= length between the caps.
  const T o = dot(a_ray.m_origin - a_center1, axis);
  const T d = dot(a_ray.m_direction, axis);

  if (d == T(0)) {
    if (o < T(0) || o > length) {
      a_in  = Math::Limits<T>::infinity();
      a_out = -Math::Limits<T>::infinity();
    }

    return;
  }

  const T tLow  = -o / d;
  const T tHigh = (length - o) / d;

  const T        tNear = (d > T(0)) ? tLow : tHigh;
  const T        tFar  = (d > T(0)) ? tHigh : tLow;
  const Vec3T<T> nNear = (d > T(0)) ? -axis : axis;
  const Vec3T<T> nFar  = (d > T(0)) ? axis : -axis;

  if (tNear > a_in) {
    a_in       = tNear;
    a_inNormal = nNear;
  }
  if (tFar < a_out) {
    a_out       = tFar;
    a_outNormal = nFar;
  }
}

/**
 * @brief Where a ray is inside a capsule: the hull of where it is inside the cylinder between the centers and
 * the two balls at its ends, which is one span since the capsule is convex.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in]  a_ray     Ray.
 * @param[in]  a_center1 Center of one cap.
 * @param[in]  a_center2 Center of the other.
 * @param[in]  a_radius  Radius.
 * @param[out] a_in      Where it enters.
 * @param[out] a_out     Where it leaves (below a_in if it misses).
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
capsuleSpan(const RayT<T>&  a_ray,
            const Vec3T<T>& a_center1,
            const Vec3T<T>& a_center2,
            const T         a_radius,
            T&              a_in,
            T&              a_out) noexcept
{
  // The capsule is convex, so the ray is inside it over one span: the hull of its spans through the
  // cylinder between the centers and through the two balls.
  a_in  = Math::Limits<T>::infinity();
  a_out = -Math::Limits<T>::infinity();

  const auto widen = [&a_in, &a_out](const T a_lo, const T a_hi) noexcept {
    if (a_lo <= a_hi) {
      a_in  = Math::min(a_in, a_lo);
      a_out = Math::max(a_out, a_hi);
    }
  };

  T a;
  T b;

  RayDetail::ballSpan(a_ray, a_center1, a_radius, a, b);
  widen(a, b);
  RayDetail::ballSpan(a_ray, a_center2, a_radius, a, b);
  widen(a, b);

  if ((a_center2 - a_center1).length() > T(0)) {
    Vec3T<T> nA;
    Vec3T<T> nB;

    RayDetail::cylinderSpan(a_ray, a_center1, a_center2, a_radius, a, nA, b, nB);
    widen(a, b);
  }
}

/**
 * @brief Where a ray is inside a convex object: from m_in to m_out (m_out below m_in for a ray that
 * misses), with the outward normals there.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 */
template <class T>
struct ConvexSpan
{
  /**
   * @brief Where the ray enters (or -infinity).
   */
  T m_in;

  /**
   * @brief Outward normal there.
   */
  Vec3T<T> m_inNormal;

  /**
   * @brief Where it leaves (or +infinity).
   */
  T m_out;

  /**
   * @brief Outward normal there.
   */
  Vec3T<T> m_outNormal;
};

/**
 * @brief Where a ray is behind a plane (its value along the ray is v0 + t * rate; the solid is where it is
 * negative).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_shape The plane.
 * @param[in] a_ray   Ray.
 * @return The span: everywhere or nowhere for a ray along the plane.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const PlaneSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  const Vec3T<T>& n    = a_shape.getNormal();
  const T         v0   = dot(a_ray.m_origin - a_shape.getPoint(), n);
  const T         rate = dot(a_ray.m_direction, n);
  const T         inf  = Math::Limits<T>::infinity();

  if (rate == T(0)) {
    return (v0 < T(0)) ? ConvexSpan<T>{-inf, n, inf, n} : ConvexSpan<T>{inf, n, -inf, n};
  }

  const T crossing = -v0 / rate;

  return (rate < T(0)) ? ConvexSpan<T>{crossing, n, inf, n} : ConvexSpan<T>{-inf, n, crossing, n};
}

/**
 * @brief Where a ray is inside a sphere.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_shape The sphere.
 * @param[in] a_ray   Ray.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const SphereSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  T in;
  T out;

  RayDetail::ballSpan(a_ray, a_shape.getCenter(), a_shape.getRadius(), in, out);

  if (!(in <= out)) {
    return ConvexSpan<T>{in, Vec3T<T>::zeros(), out, Vec3T<T>::zeros()};
  }

  const T r = a_shape.getRadius();

  return ConvexSpan<T>{in, (a_ray.at(in) - a_shape.getCenter()) / r, out, (a_ray.at(out) - a_shape.getCenter()) / r};
}

/**
 * @brief Where a ray is inside a box.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_shape The box.
 * @param[in] a_ray   Ray.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const BoxSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  ConvexSpan<T> span{-Math::Limits<T>::infinity(), Vec3T<T>::zeros(), Math::Limits<T>::infinity(), Vec3T<T>::zeros()};

  for (size_t axis = 0; axis < 3; axis++) {
    RayDetail::slab(a_ray,
                    axis,
                    a_shape.getLowCorner()[axis],
                    a_shape.getHighCorner()[axis],
                    span.m_in,
                    span.m_inNormal,
                    span.m_out,
                    span.m_outNormal);
  }

  return span;
}

/**
 * @brief Where a ray is inside a finite cylinder.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_shape The cylinder.
 * @param[in] a_ray   Ray.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const CylinderSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  ConvexSpan<T> span;

  RayDetail::cylinderSpan(a_ray,
                          a_shape.getCenter1(),
                          a_shape.getCenter2(),
                          a_shape.getRadius(),
                          span.m_in,
                          span.m_inNormal,
                          span.m_out,
                          span.m_outNormal);

  return span;
}

/**
 * @brief Where a ray is inside an infinite cylinder.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_shape The cylinder.
 * @param[in] a_ray   Ray.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const InfiniteCylinderSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  Vec3T<T> axis = Vec3T<T>::zeros();

  axis[a_shape.getAxis()] = T(1);

  T in;
  T out;

  RayDetail::tubeSpan(a_ray, a_shape.getCenter(), axis, a_shape.getRadius(), in, out);

  if (!(in <= out)) {
    return ConvexSpan<T>{in, Vec3T<T>::zeros(), out, Vec3T<T>::zeros()};
  }

  return ConvexSpan<T>{in,
                       RayDetail::tubeNormal(a_ray.at(in), a_shape.getCenter(), axis),
                       out,
                       RayDetail::tubeNormal(a_ray.at(out), a_shape.getCenter(), axis)};
}

/**
 * @brief Where a ray is inside a capsule.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_shape The capsule.
 * @param[in] a_ray   Ray.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const CapsuleSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  const Vec3T<T>& c1     = a_shape.getCenter1();
  const Vec3T<T>& c2     = a_shape.getCenter2();
  const T         length = (c2 - c1).length();

  ConvexSpan<T> span;

  RayDetail::capsuleSpan(a_ray, c1, c2, a_shape.getRadius(), span.m_in, span.m_out);

  if (!(span.m_in <= span.m_out)) {
    return span;
  }

  // The normal of a capsule at a surface point is away from the nearest point of its segment.
  const auto normal = [&c1, &c2, length](const Vec3T<T>& a_p) noexcept {
    const Vec3T<T> axis = (length > T(0)) ? (c2 - c1) / length : Vec3T<T>::zeros();
    const T        h    = Math::clamp(dot(a_p - c1, axis), T(0), length);
    const Vec3T<T> n    = a_p - (c1 + h * axis);
    const T        l    = n.length();

    return (l > T(0)) ? n / l : Vec3T<T>::zeros();
  };

  span.m_inNormal  = normal(a_ray.at(span.m_in));
  span.m_outNormal = normal(a_ray.at(span.m_out));

  return span;
}

/**
 * @brief Clips a span to the half-space where dot(x - a_point, a_normal) <= 0, behind a plane with the unit
 * outward normal a_normal.
 * @details Internal helper; not part of the public API. A ray along the plane is behind it everywhere or
 * nowhere.
 * @tparam T Floating-point precision.
 * @param[in]     a_ray       Ray.
 * @param[in]     a_point     A point on the plane.
 * @param[in]     a_normal    Unit outward normal.
 * @param[in,out] a_in        Raised to where the ray enters the half-space.
 * @param[in,out] a_inNormal  Set to a_normal, if raised.
 * @param[in,out] a_out       Lowered to where it leaves.
 * @param[in,out] a_outNormal Set to a_normal, if lowered.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
halfSpace(const RayT<T>&  a_ray,
          const Vec3T<T>& a_point,
          const Vec3T<T>& a_normal,
          T&              a_in,
          Vec3T<T>&       a_inNormal,
          T&              a_out,
          Vec3T<T>&       a_outNormal) noexcept
{
  const T v0   = dot(a_ray.m_origin - a_point, a_normal);
  const T rate = dot(a_ray.m_direction, a_normal);

  if (rate == T(0)) {
    if (v0 > T(0)) {
      a_in  = Math::Limits<T>::infinity();
      a_out = -Math::Limits<T>::infinity();
    }

    return;
  }

  const T crossing = -v0 / rate;

  if (rate < T(0)) {
    if (crossing > a_in) {
      a_in       = crossing;
      a_inNormal = a_normal;
    }
  }
  else if (crossing < a_out) {
    a_out       = crossing;
    a_outNormal = a_normal;
  }
}

/**
 * @brief Where a ray is inside an ellipsoid: the roots of a quadratic in coordinates scaled so that the
 * ellipsoid is the unit sphere.
 * @details Internal helper; not part of the public API. As for a sphere (see ballSpan()), the discriminant is
 * a - |o x d|^2 rather than b^2 - a c, and the nearer root c / q. The normal is the gradient of
 * |(x - center) / radii|^2.
 * @tparam T Floating-point precision.
 * @param[in] a_shape The ellipsoid.
 * @param[in] a_ray   Ray.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const EllipsoidSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  const Vec3T<T>& c     = a_shape.getCenter();
  const Vec3T<T>& radii = a_shape.getRadii();
  const T         inf   = Math::Limits<T>::infinity();

  const Vec3T<T> o = (a_ray.m_origin - c) / radii;
  const Vec3T<T> d = a_ray.m_direction / radii;
  const Vec3T<T> l = cross(o, d);

  const T a    = dot(d, d);
  const T b    = dot(o, d);
  const T disc = a - dot(l, l);

  if (disc < T(0)) {
    return ConvexSpan<T>{inf, Vec3T<T>::zeros(), -inf, Vec3T<T>::zeros()};
  }

  const T root = std::sqrt(disc);
  const T q    = -b - ((b >= T(0)) ? root : -root);

  // Both roots are zero: the ray starts on the surface, along it.
  const T in  = (q == T(0)) ? T(0) : Math::min(q / a, (dot(o, o) - T(1)) / q);
  const T out = (q == T(0)) ? T(0) : Math::max(q / a, (dot(o, o) - T(1)) / q);

  const auto normal = [&c, &radii](const Vec3T<T>& a_p) noexcept {
    const Vec3T<T> g      = (a_p - c) / (radii * radii);
    const T        length = g.length();

    return (length > T(0)) ? g / length : Vec3T<T>::zeros();
  };

  return ConvexSpan<T>{in, normal(a_ray.at(in)), out, normal(a_ray.at(out))};
}

/**
 * @brief Where a ray is inside a capped cone: inside the cone's quadratic, within the slab between its ends.
 * @details Internal helper; not part of the public API. Along the ray, the offset across the axis is
 * P0 + t Dp and the position along it y0 + t dy, so the side is the zero set of
 * Q(t) = |P0 + t Dp|^2 - (r1 + k (y0 + t dy))^2, k the radius' change per unit length. Within the slab the
 * radius is never negative, so the solid there is where Q <= 0: between the roots when the quadratic's leading
 * coefficient is positive, outside them when it is negative (the ray is steeper than the side, and would reach
 * the other nappe of the double cone beyond the slab). The frustum being convex, what is left in the slab is
 * one interval. The side's normal is the gradient of |P| - (r1 + k y).
 * @tparam T Floating-point precision.
 * @param[in] a_shape The frustum.
 * @param[in] a_ray   Ray.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const CappedConeSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  const Vec3T<T>& c1     = a_shape.getCenter1();
  const Vec3T<T>  delta  = a_shape.getCenter2() - c1;
  const T         length = delta.length();
  const Vec3T<T>  axis   = delta / length;
  const T         r1     = a_shape.getRadius1();
  const T         k      = (a_shape.getRadius2() - r1) / length;
  const T         inf    = Math::Limits<T>::infinity();

  const Vec3T<T> w0 = a_ray.m_origin - c1;
  const T        y0 = dot(w0, axis);
  const T        dy = dot(a_ray.m_direction, axis);
  const Vec3T<T> p0 = w0 - y0 * axis;
  const Vec3T<T> dp = a_ray.m_direction - dy * axis;

  const T radius0 = r1 + k * y0;
  const T qa      = dot(dp, dp) - k * k * dy * dy;
  const T qb      = dot(p0, dp) - k * dy * radius0;
  const T qc      = dot(p0, p0) - radius0 * radius0;

  // Where Q <= 0, as one interval, or two (below lo and above hi) when the leading coefficient is negative.
  T    lo       = -inf;
  T    hi       = inf;
  bool outsides = false;

  if (qa == T(0)) {
    if (qb == T(0)) {
      if (qc > T(0)) {
        return ConvexSpan<T>{inf, Vec3T<T>::zeros(), -inf, Vec3T<T>::zeros()};
      }
    }
    else if (qb > T(0)) {
      hi = -qc / (T(2) * qb);
    }
    else {
      lo = -qc / (T(2) * qb);
    }
  }
  else {
    const T disc = qb * qb - qa * qc;

    if (disc < T(0)) {
      if (qa > T(0)) {
        return ConvexSpan<T>{inf, Vec3T<T>::zeros(), -inf, Vec3T<T>::zeros()};
      }
    }
    else {
      const T root = std::sqrt(disc);
      const T q    = -qb - ((qb >= T(0)) ? root : -root);
      const T t1   = (q == T(0)) ? T(0) : Math::min(q / qa, qc / q);
      const T t2   = (q == T(0)) ? T(0) : Math::max(q / qa, qc / q);

      lo       = t1;
      hi       = t2;
      outsides = qa < T(0);
    }
  }

  // The slab between the ends.
  ConvexSpan<T> slab{-inf, Vec3T<T>::zeros(), inf, Vec3T<T>::zeros()};

  RayDetail::halfSpace(a_ray, c1, -axis, slab.m_in, slab.m_inNormal, slab.m_out, slab.m_outNormal);
  RayDetail::halfSpace(a_ray, c1 + delta, axis, slab.m_in, slab.m_inNormal, slab.m_out, slab.m_outNormal);

  const auto side = [&c1, &axis, k](const Vec3T<T>& a_p) noexcept {
    const Vec3T<T> w      = a_p - c1;
    const Vec3T<T> across = w - dot(w, axis) * axis;
    const T        r      = across.length();
    const Vec3T<T> g      = ((r > T(0)) ? across / r : Vec3T<T>::zeros()) - k * axis;
    const T        gl     = g.length();

    return (gl > T(0)) ? g / gl : Vec3T<T>::zeros();
  };

  // Intersect the slab with [a_lo, a_hi], the side's crossings giving the normals at finite ends.
  const auto clip = [&](const T a_lo, const T a_hi) noexcept {
    ConvexSpan<T> span = slab;

    if (a_lo > span.m_in) {
      span.m_in       = a_lo;
      span.m_inNormal = side(a_ray.at(a_lo));
    }
    if (a_hi < span.m_out) {
      span.m_out       = a_hi;
      span.m_outNormal = side(a_ray.at(a_hi));
    }

    return span;
  };

  if (!outsides) {
    return clip(lo, hi);
  }

  // Q <= 0 below lo and above hi; inside the slab only one of the two can be left.
  const ConvexSpan<T> below = clip(-inf, lo);
  const ConvexSpan<T> above = clip(hi, inf);

  return (below.m_in <= below.m_out) ? below : above;
}

/**
 * @brief Where a ray is inside a regular prism: behind each side's plane, within the slab between its ends.
 * @details Internal helper; not part of the public API. Side k's outward normal is at angle (2 k + 1) pi / n
 * from the vertex direction, and its plane at the apothem, the circumradius times cos(pi / n). The normals are
 * turned from one side to the next by a fixed rotation.
 * @tparam T Floating-point precision.
 * @param[in] a_shape The prism.
 * @param[in] a_ray   Ray.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const PolygonPrismSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  const Vec3T<T>& c1    = a_shape.getCenter1();
  const Vec3T<T>& c2    = a_shape.getCenter2();
  const Vec3T<T>  axis  = (c2 - c1) / (c2 - c1).length();
  const Vec3T<T>& u     = a_shape.getVertexDirection();
  const Vec3T<T>  v     = cross(axis, u);
  const unsigned  sides = a_shape.getSides();
  const T         half  = pi<T> / T(sides);
  const T         inf   = Math::Limits<T>::infinity();

  const T apothem = a_shape.getRadius() * std::cos(half);
  const T stepCos = std::cos(T(2) * half);
  const T stepSin = std::sin(T(2) * half);

  ConvexSpan<T> span{-inf, Vec3T<T>::zeros(), inf, Vec3T<T>::zeros()};

  RayDetail::halfSpace(a_ray, c1, -axis, span.m_in, span.m_inNormal, span.m_out, span.m_outNormal);
  RayDetail::halfSpace(a_ray, c2, axis, span.m_in, span.m_inNormal, span.m_out, span.m_outNormal);

  T cosAngle = std::cos(half);
  T sinAngle = std::sin(half);

  for (unsigned k = 0; k < sides && span.m_in <= span.m_out; k++) {
    const Vec3T<T> normal = cosAngle * u + sinAngle * v;

    RayDetail::halfSpace(
      a_ray, c1 + apothem * normal, normal, span.m_in, span.m_inNormal, span.m_out, span.m_outNormal);

    const T nextCos = cosAngle * stepCos - sinAngle * stepSin;

    sinAngle = sinAngle * stepCos + cosAngle * stepSin;
    cosAngle = nextCos;
  }

  return span;
}

/**
 * @brief Where a ray is inside a cut sphere: inside the sphere, and above the cut.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_shape The cut sphere.
 * @param[in] a_ray   Ray.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const CutSphereSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  const Vec3T<T>& c = a_shape.getCenter();
  const T         r = a_shape.getRadius();

  ConvexSpan<T> span;

  RayDetail::ballSpan(a_ray, c, r, span.m_in, span.m_out);

  if (!(span.m_in <= span.m_out)) {
    return span;
  }

  span.m_inNormal  = (a_ray.at(span.m_in) - c) / r;
  span.m_outNormal = (a_ray.at(span.m_out) - c) / r;

  RayDetail::halfSpace(a_ray,
                       c + a_shape.getHeight() * Vec3T<T>::unit(1),
                       -Vec3T<T>::unit(1),
                       span.m_in,
                       span.m_inNormal,
                       span.m_out,
                       span.m_outNormal);

  return span;
}

/**
 * @brief Where a ray is inside a wedge: behind both faces.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_shape The wedge.
 * @param[in] a_ray   Ray.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline ConvexSpan<T>
convexSpan(const WedgeSDF<T>& a_shape, const RayT<T>& a_ray) noexcept
{
  const Vec3T<T> n   = a_shape.getFaceNormal();
  const T        inf = Math::Limits<T>::infinity();

  ConvexSpan<T> span{-inf, Vec3T<T>::zeros(), inf, Vec3T<T>::zeros()};

  RayDetail::halfSpace(a_ray, a_shape.getTip(), n, span.m_in, span.m_inNormal, span.m_out, span.m_outNormal);
  RayDetail::halfSpace(
    a_ray, a_shape.getTip(), Vec3T<T>(-n[0], n[1], n[2]), span.m_in, span.m_inNormal, span.m_out, span.m_outNormal);

  return span;
}

/**
 * @brief Where a ray is inside a tube: inside the outer cylinder and outside the inner one, which can leave
 * two spans, before and after the hollow.
 * @details Internal helper; not part of the public API. A span that is empty has its m_out below its m_in.
 * @tparam T Floating-point precision.
 * @param[in]  a_shape  The tube.
 * @param[in]  a_ray    Ray.
 * @param[out] a_first  The earlier span.
 * @param[out] a_second The later span.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
tubeSpans(const TubeSDF<T>& a_shape, const RayT<T>& a_ray, ConvexSpan<T>& a_first, ConvexSpan<T>& a_second) noexcept
{
  const Vec3T<T>& c1   = a_shape.getCenter1();
  const Vec3T<T>& c2   = a_shape.getCenter2();
  const Vec3T<T>  axis = (c2 - c1) / (c2 - c1).length();
  const T         inf  = Math::Limits<T>::infinity();

  ConvexSpan<T> outer;

  RayDetail::cylinderSpan(
    a_ray, c1, c2, a_shape.getOuterRadius(), outer.m_in, outer.m_inNormal, outer.m_out, outer.m_outNormal);

  a_first  = outer;
  a_second = ConvexSpan<T>{inf, Vec3T<T>::zeros(), -inf, Vec3T<T>::zeros()};

  if (!(outer.m_in <= outer.m_out)) {
    return;
  }

  T holeIn;
  T holeOut;

  RayDetail::tubeSpan(a_ray, c1, axis, a_shape.getInnerRadius(), holeIn, holeOut);

  if (!(holeIn <= holeOut)) {
    return;
  }

  // The hollow's wall faces the axis.
  const Vec3T<T> wallIn  = -RayDetail::tubeNormal(a_ray.at(holeIn), c1, axis);
  const Vec3T<T> wallOut = -RayDetail::tubeNormal(a_ray.at(holeOut), c1, axis);

  a_first.m_out       = Math::min(outer.m_out, holeIn);
  a_first.m_outNormal = (holeIn < outer.m_out) ? wallIn : outer.m_outNormal;

  a_second.m_in        = Math::max(outer.m_in, holeOut);
  a_second.m_inNormal  = (holeOut > outer.m_in) ? wallOut : outer.m_inNormal;
  a_second.m_out       = outer.m_out;
  a_second.m_outNormal = outer.m_outNormal;

  // A span the hollow covers entirely is empty.
  if (!(a_first.m_in < a_first.m_out)) {
    a_first = a_second;

    a_second = ConvexSpan<T>{inf, Vec3T<T>::zeros(), -inf, Vec3T<T>::zeros()};
  }
  if (!(a_first.m_in < a_first.m_out)) {
    a_first = ConvexSpan<T>{inf, Vec3T<T>::zeros(), -inf, Vec3T<T>::zeros()};
  }
  if (!(a_second.m_in < a_second.m_out)) {
    a_second = ConvexSpan<T>{inf, Vec3T<T>::zeros(), -inf, Vec3T<T>::zeros()};
  }
}

/**
 * @brief The first crossing of a convex shape along a ray, from its span.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape The shape.
 * @param[in] a_ray   Ray.
 * @param[in] a_tMin  Start of the interval.
 * @param[in] a_tMax  End of the interval.
 * @return The crossing, or a miss.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
convexFirstHit(const S& a_shape, const RayT<T>& a_ray, const T a_tMin, const T a_tMax) noexcept
{
  const ConvexSpan<T> span = RayDetail::convexSpan(a_shape, a_ray);

  return RayDetail::convexHit(a_ray, span.m_in, span.m_inNormal, span.m_out, span.m_outNormal, a_tMin, a_tMax);
}

/**
 * @brief Where a ray is inside a convex shape, without the normals: its span's two ends.
 * @details Internal helper; not part of the public API. A span list keeps no normals, so for a shape whose span
 * computes its normals apart from its ends, the list takes the ends alone. Computed as a span and then dropped,
 * the normals are left to the compiler to remove, which it does not where it keeps the normal's function out of
 * line: a fused CSG solid of a box, a ball and three infinite cylinders then traced 20-35% slower on the host.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]  a_shape The shape.
 * @param[in]  a_ray   Ray.
 * @param[out] a_in    Where the ray enters (or -infinity).
 * @param[out] a_out   Where it leaves (or +infinity); below a_in for a ray that misses.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline void
convexInterval(const S& a_shape, const RayT<T>& a_ray, T& a_in, T& a_out) noexcept
{
  if constexpr (std::is_same_v<S, SphereSDF<T>>) {
    RayDetail::ballSpan(a_ray, a_shape.getCenter(), a_shape.getRadius(), a_in, a_out);
  }
  else if constexpr (std::is_same_v<S, InfiniteCylinderSDF<T>>) {
    Vec3T<T> axis = Vec3T<T>::zeros();

    axis[a_shape.getAxis()] = T(1);

    RayDetail::tubeSpan(a_ray, a_shape.getCenter(), axis, a_shape.getRadius(), a_in, a_out);
  }
  else if constexpr (std::is_same_v<S, CapsuleSDF<T>>) {
    RayDetail::capsuleSpan(a_ray, a_shape.getCenter1(), a_shape.getCenter2(), a_shape.getRadius(), a_in, a_out);
  }
  else {
    const ConvexSpan<T> span = RayDetail::convexSpan(a_shape, a_ray);

    a_in  = span.m_in;
    a_out = span.m_out;
  }
}

/**
 * @brief A convex shape's list over an interval: its one span, cut to the interval.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]  a_shape The shape.
 * @param[in]  a_ray   Ray.
 * @param[in]  a_tMin  Start of the interval.
 * @param[in]  a_tMax  End of the interval.
 * @param[out] a_spans The list.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline void
convexSpans(const S& a_shape, const RayT<T>& a_ray, const T a_tMin, const T a_tMax, RaySpansT<T>& a_spans) noexcept
{
  T in;
  T out;

  RayDetail::convexInterval(a_shape, a_ray, in, out);

  a_spans.reset(a_tMax);
  a_spans.addClipped(in, out, a_tMin, a_tMax);
}

} // namespace RayDetail

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<PlaneSDF<T>>::firstHit(const PlaneSDF<T>& a_shape,
                               const RayT<T>&     a_ray,
                               const T            a_tMin,
                               const T            a_tMax,
                               const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<PlaneSDF<T>>::spans(const PlaneSDF<T>& a_shape,
                            const RayT<T>&     a_ray,
                            const T            a_tMin,
                            const T            a_tMax,
                            const RayOptions<T>& /*a_options*/,
                            const bool /*a_firstOnly*/,
                            RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<SphereSDF<T>>::firstHit(const SphereSDF<T>& a_shape,
                                const RayT<T>&      a_ray,
                                const T             a_tMin,
                                const T             a_tMax,
                                const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<SphereSDF<T>>::spans(const SphereSDF<T>& a_shape,
                             const RayT<T>&      a_ray,
                             const T             a_tMin,
                             const T             a_tMax,
                             const RayOptions<T>& /*a_options*/,
                             const bool /*a_firstOnly*/,
                             RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<BoxSDF<T>>::firstHit(const BoxSDF<T>& a_shape,
                             const RayT<T>&   a_ray,
                             const T          a_tMin,
                             const T          a_tMax,
                             const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<BoxSDF<T>>::spans(const BoxSDF<T>& a_shape,
                          const RayT<T>&   a_ray,
                          const T          a_tMin,
                          const T          a_tMax,
                          const RayOptions<T>& /*a_options*/,
                          const bool /*a_firstOnly*/,
                          RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<CylinderSDF<T>>::firstHit(const CylinderSDF<T>& a_shape,
                                  const RayT<T>&        a_ray,
                                  const T               a_tMin,
                                  const T               a_tMax,
                                  const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<CylinderSDF<T>>::spans(const CylinderSDF<T>& a_shape,
                               const RayT<T>&        a_ray,
                               const T               a_tMin,
                               const T               a_tMax,
                               const RayOptions<T>& /*a_options*/,
                               const bool /*a_firstOnly*/,
                               RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<InfiniteCylinderSDF<T>>::firstHit(const InfiniteCylinderSDF<T>& a_shape,
                                          const RayT<T>&                a_ray,
                                          const T                       a_tMin,
                                          const T                       a_tMax,
                                          const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<InfiniteCylinderSDF<T>>::spans(const InfiniteCylinderSDF<T>& a_shape,
                                       const RayT<T>&                a_ray,
                                       const T                       a_tMin,
                                       const T                       a_tMax,
                                       const RayOptions<T>& /*a_options*/,
                                       const bool /*a_firstOnly*/,
                                       RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<CapsuleSDF<T>>::firstHit(const CapsuleSDF<T>& a_shape,
                                 const RayT<T>&       a_ray,
                                 const T              a_tMin,
                                 const T              a_tMax,
                                 const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<CapsuleSDF<T>>::spans(const CapsuleSDF<T>& a_shape,
                              const RayT<T>&       a_ray,
                              const T              a_tMin,
                              const T              a_tMax,
                              const RayOptions<T>& /*a_options*/,
                              const bool /*a_firstOnly*/,
                              RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<EllipsoidSDF<T>>::firstHit(const EllipsoidSDF<T>& a_shape,
                                   const RayT<T>&         a_ray,
                                   const T                a_tMin,
                                   const T                a_tMax,
                                   const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<EllipsoidSDF<T>>::spans(const EllipsoidSDF<T>& a_shape,
                                const RayT<T>&         a_ray,
                                const T                a_tMin,
                                const T                a_tMax,
                                const RayOptions<T>& /*a_options*/,
                                const bool /*a_firstOnly*/,
                                RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<CappedConeSDF<T>>::firstHit(const CappedConeSDF<T>& a_shape,
                                    const RayT<T>&          a_ray,
                                    const T                 a_tMin,
                                    const T                 a_tMax,
                                    const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<CappedConeSDF<T>>::spans(const CappedConeSDF<T>& a_shape,
                                 const RayT<T>&          a_ray,
                                 const T                 a_tMin,
                                 const T                 a_tMax,
                                 const RayOptions<T>& /*a_options*/,
                                 const bool /*a_firstOnly*/,
                                 RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<PolygonPrismSDF<T>>::firstHit(const PolygonPrismSDF<T>& a_shape,
                                      const RayT<T>&            a_ray,
                                      const T                   a_tMin,
                                      const T                   a_tMax,
                                      const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<PolygonPrismSDF<T>>::spans(const PolygonPrismSDF<T>& a_shape,
                                   const RayT<T>&            a_ray,
                                   const T                   a_tMin,
                                   const T                   a_tMax,
                                   const RayOptions<T>& /*a_options*/,
                                   const bool /*a_firstOnly*/,
                                   RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<CutSphereSDF<T>>::firstHit(const CutSphereSDF<T>& a_shape,
                                   const RayT<T>&         a_ray,
                                   const T                a_tMin,
                                   const T                a_tMax,
                                   const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<CutSphereSDF<T>>::spans(const CutSphereSDF<T>& a_shape,
                                const RayT<T>&         a_ray,
                                const T                a_tMin,
                                const T                a_tMax,
                                const RayOptions<T>& /*a_options*/,
                                const bool /*a_firstOnly*/,
                                RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<WedgeSDF<T>>::firstHit(const WedgeSDF<T>& a_shape,
                               const RayT<T>&     a_ray,
                               const T            a_tMin,
                               const T            a_tMax,
                               const RayOptions<T>& /*a_options*/) noexcept
{
  return RayDetail::convexFirstHit(a_shape, a_ray, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<WedgeSDF<T>>::spans(const WedgeSDF<T>& a_shape,
                            const RayT<T>&     a_ray,
                            const T            a_tMin,
                            const T            a_tMax,
                            const RayOptions<T>& /*a_options*/,
                            const bool /*a_firstOnly*/,
                            RaySpansT<T>& a_spans) noexcept
{
  RayDetail::convexSpans(a_shape, a_ray, a_tMin, a_tMax, a_spans);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<TubeSDF<T>>::firstHit(const TubeSDF<T>& a_shape,
                              const RayT<T>&    a_ray,
                              const T           a_tMin,
                              const T           a_tMax,
                              const RayOptions<T>& /*a_options*/) noexcept
{
  RayDetail::ConvexSpan<T> first;
  RayDetail::ConvexSpan<T> second;

  RayDetail::tubeSpans(a_shape, a_ray, first, second);

  // The spans are in order, so the first that gives a crossing in the interval gives the first crossing.
  const HitT<T> hit =
    RayDetail::convexHit(a_ray, first.m_in, first.m_inNormal, first.m_out, first.m_outNormal, a_tMin, a_tMax);

  return hit.hit() ? hit
                   : RayDetail::convexHit(
                       a_ray, second.m_in, second.m_inNormal, second.m_out, second.m_outNormal, a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<TubeSDF<T>>::spans(const TubeSDF<T>& a_shape,
                           const RayT<T>&    a_ray,
                           const T           a_tMin,
                           const T           a_tMax,
                           const RayOptions<T>& /*a_options*/,
                           const bool /*a_firstOnly*/,
                           RaySpansT<T>& a_spans) noexcept
{
  RayDetail::ConvexSpan<T> first;
  RayDetail::ConvexSpan<T> second;

  RayDetail::tubeSpans(a_shape, a_ray, first, second);

  a_spans.reset(a_tMax);
  a_spans.addClipped(first.m_in, first.m_out, a_tMin, a_tMax);
  a_spans.addClipped(second.m_in, second.m_out, a_tMin, a_tMax);
}

} // namespace EBGeometry

#endif
