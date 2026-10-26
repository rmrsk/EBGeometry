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
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
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
 * squared distance of the centre from the ray's line, |oc x d|^2, not b^2 - c: for a small ball far from
 * the origin, |oc|^2 - r^2 cancels, and in float a ray passing within about 1e-3 of the radius of the
 * surface could go either way. The nearer root is c / q, which does not cancel either.
 * @tparam T Floating-point precision.
 * @param[in]  a_ray    Ray.
 * @param[in]  a_center Centre.
 * @param[in]  a_radius Radius.
 * @param[out] a_in     Where it enters.
 * @param[out] a_out    Where it leaves (below a_in if it misses).
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
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
 * @brief Where a ray is inside a finite cylinder between two centres, with the normals at its ends.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in]  a_ray       Ray.
 * @param[in]  a_center1   Centre of one cap.
 * @param[in]  a_center2   Centre of the other.
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
 * @brief Where a ray is inside a capsule: the hull of where it is inside the cylinder between the centres and
 * the two balls at its ends, which is one span since the capsule is convex.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in]  a_ray     Ray.
 * @param[in]  a_center1 Centre of one cap.
 * @param[in]  a_center2 Centre of the other.
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
  // cylinder between the centres and through the two balls.
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
  const ConvexSpan<T> span = RayDetail::convexSpan(a_shape, a_ray);

  a_spans.reset(a_tMax);
  a_spans.addClipped(span.m_in, span.m_out, a_tMin, a_tMax);
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

} // namespace EBGeometry

#endif
