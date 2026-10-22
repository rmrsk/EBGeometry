// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RayImplem.hpp
 * @brief  Implementation of EBGeometry_Ray.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RAYIMPLEM_HPP
#define EBGEOMETRY_RAYIMPLEM_HPP

// Std includes
#include <cmath>
#include <type_traits>
#include <utility>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Ray.hpp"

namespace EBGeometry {

namespace RayDetail {

/**
 * @brief Whether a shape names the leaf at a point (Tape::primitiveAt()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasPrimitiveAt : std::false_type
{
};

/**
 * @brief Whether a shape names the leaf at a point (Tape::primitiveAt()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasPrimitiveAt<S,
                      T,
                      std::void_t<decltype(std::declval<const S&>().primitiveAt(std::declval<const Vec3T<T>&>()))>>
  : std::true_type
{
};

/**
 * @brief Whether a shape names the closest face to a point (the mesh SDFs' getClosestFace()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasClosestFace : std::false_type
{
};

/**
 * @brief Whether a shape names the closest face to a point (the mesh SDFs' getClosestFace()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasClosestFace<
  S,
  T,
  std::void_t<decltype(std::declval<const S&>().getClosestFace(std::declval<const Vec3T<T>&>()).faceId)>>
  : std::true_type
{
};

/**
 * @brief Whether a shape reports its distance quality at run time (Tape::getDistanceQuality()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S, class = void>
struct HasRuntimeQuality : std::false_type
{
};

/**
 * @brief Whether a shape reports its distance quality at run time (Tape::getDistanceQuality()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S>
struct HasRuntimeQuality<S, std::void_t<decltype(std::declval<const S&>().getDistanceQuality())>> : std::true_type
{
};

/**
 * @brief Whether a shape's ray form gives the span a ray is inside it in one step (RayForm<S>::span()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasSpan : std::false_type
{
};

/**
 * @brief Whether a shape's ray form gives the span a ray is inside it in one step (RayForm<S>::span()).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasSpan<S,
               T,
               std::void_t<decltype(RayForm<S>::span(
                 std::declval<const S&>(), std::declval<const RayT<T>&>(), std::declval<T&>(), std::declval<T&>()))>>
  : std::true_type
{
};

/**
 * @brief A shape's distance quality: its run-time one if it has one, its static one otherwise.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 * @param[in] a_shape Shape.
 * @return The quality.
 */
template <class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline DistanceQuality
qualityOf(const S& a_shape) noexcept
{
  if constexpr (HasRuntimeQuality<S>::value) {
    return a_shape.getDistanceQuality();
  }
  else {
    return distanceQualityOf<S>;
  }
}

/**
 * @brief The finest element a shape can name at a point (see HitT::m_primitive).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape Shape.
 * @param[in] a_point Point.
 * @return The leaf, the closest face, or 0.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline uint32_t
primitiveOf(const S& a_shape, const Vec3T<T>& a_point) noexcept
{
  if constexpr (HasPrimitiveAt<S, T>::value) {
    return a_shape.primitiveAt(a_point);
  }
  else if constexpr (HasClosestFace<S, T>::value) {
    return a_shape.getClosestFace(a_point).faceId;
  }
  else {
    return 0U;
  }
}

/**
 * @brief Clip a ray's interval to a box.
 * @details Internal helper; not part of the public API. Handles unbounded boxes and directions parallel
 * to a face.
 * @tparam T Floating-point precision.
 * @param[in]     a_lo  Low corner.
 * @param[in]     a_hi  High corner.
 * @param[in]     a_ray Ray.
 * @param[in,out] a_t0  Start of the interval.
 * @param[in,out] a_t1  End of the interval.
 * @return Whether anything of the interval is left.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline bool
clip(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi, const RayT<T>& a_ray, T& a_t0, T& a_t1) noexcept
{
  for (size_t dir = 0; dir < 3; dir++) {
    const T o = a_ray.m_origin[dir];
    const T d = a_ray.m_direction[dir];

    if (!(a_lo[dir] <= a_hi[dir])) {
      return false;
    }

    if (d == T(0)) {
      if (o < a_lo[dir] || o > a_hi[dir]) {
        return false;
      }
    }
    else {
      const T inv = T(1) / d;

      T tNear = (a_lo[dir] - o) * inv;
      T tFar  = (a_hi[dir] - o) * inv;

      if (tNear > tFar) {
        const T swap = tNear;

        tNear = tFar;
        tFar  = swap;
      }

      a_t0 = Math::max(a_t0, tNear);
      a_t1 = Math::min(a_t1, tFar);
    }
  }

  return a_t0 <= a_t1;
}

/**
 * @brief The length scale of a query (see RayOptions).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_lo      Low corner of the shape's box.
 * @param[in] a_hi      High corner.
 * @param[in] a_options Options.
 * @return The scale.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
scaleOf(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi, const RayOptions<T>& a_options) noexcept
{
  if (a_options.m_scale > T(0)) {
    return a_options.m_scale;
  }

  const T diagonal = (a_hi - a_lo).length();

  return (std::isfinite(diagonal) && diagonal > T(0)) ? diagonal : T(1);
}

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
 * @details Internal helper; not part of the public API.
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
  const T        b    = dot(oc, a_ray.m_direction);
  const T        c    = dot(oc, oc) - a_radius * a_radius;
  const T        disc = b * b - c;

  if (disc < T(0)) {
    a_in  = Math::Limits<T>::infinity();
    a_out = -Math::Limits<T>::infinity();

    return;
  }

  const T root = std::sqrt(disc);

  a_in  = -b - root;
  a_out = -b + root;
}

/**
 * @brief Where a ray is inside an infinite cylinder around the line through a point along a unit axis.
 * @details Internal helper; not part of the public API. A ray along the axis is inside it everywhere or
 * nowhere.
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

  const T b    = dot(o, d);
  const T disc = b * b - a * c;

  if (disc < T(0)) {
    a_in  = Math::Limits<T>::infinity();
    a_out = -Math::Limits<T>::infinity();

    return;
  }

  const T root = std::sqrt(disc);

  a_in  = (-b - root) / a;
  a_out = (-b + root) / a;
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
 * @brief The spans of the union of two sets of spans.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam N Most spans held.
 * @param[in]  a_a   One.
 * @param[in]  a_b   The other.
 * @param[out] a_out Their union (not one of the two).
 */
template <class T, uint32_t N>
EBGEOMETRY_HOST_DEVICE
inline void
unite(const RaySpans<T, N>& a_a, const RaySpans<T, N>& a_b, RaySpans<T, N>& a_out) noexcept
{
  RaySpans<T, N>& out = a_out;

  out.clear();

  out.m_complete = a_a.m_complete && a_b.m_complete;
  out.m_exact    = a_a.m_exact && a_b.m_exact;
  out.m_resolved = a_a.m_resolved && a_b.m_resolved;

  uint32_t i = 0;
  uint32_t j = 0;

  bool       open = false;
  RaySpan<T> cur{};

  while (i < a_a.m_count || j < a_b.m_count) {
    const bool       fromA = j >= a_b.m_count || (i < a_a.m_count && a_a.m_spans[i].m_in <= a_b.m_spans[j].m_in);
    const RaySpan<T> next  = fromA ? a_a.m_spans[i++] : a_b.m_spans[j++];

    if (!open) {
      cur  = next;
      open = true;
    }
    else if (next.m_in <= cur.m_out) {
      if (next.m_out > cur.m_out) {
        cur.m_out          = next.m_out;
        cur.m_outPrimitive = next.m_outPrimitive;
      }
    }
    else {
      out.add(cur.m_in, cur.m_out, cur.m_inPrimitive, cur.m_outPrimitive);
      cur = next;
    }
  }

  if (open) {
    out.add(cur.m_in, cur.m_out, cur.m_inPrimitive, cur.m_outPrimitive);
  }
}

/**
 * @brief The spans of the intersection of two sets of spans.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam N Most spans held.
 * @param[in]  a_a   One.
 * @param[in]  a_b   The other.
 * @param[out] a_out Their intersection (not one of the two).
 */
template <class T, uint32_t N>
EBGEOMETRY_HOST_DEVICE
inline void
intersect(const RaySpans<T, N>& a_a, const RaySpans<T, N>& a_b, RaySpans<T, N>& a_out) noexcept
{
  RaySpans<T, N>& out = a_out;

  out.clear();

  out.m_complete = a_a.m_complete && a_b.m_complete;
  out.m_exact    = a_a.m_exact && a_b.m_exact;
  out.m_resolved = a_a.m_resolved && a_b.m_resolved;

  uint32_t i = 0;
  uint32_t j = 0;

  while (i < a_a.m_count && j < a_b.m_count) {
    const RaySpan<T>& a = a_a.m_spans[i];
    const RaySpan<T>& b = a_b.m_spans[j];

    const bool aStarts = a.m_in >= b.m_in;
    const bool aEnds   = a.m_out <= b.m_out;
    const T    in      = aStarts ? a.m_in : b.m_in;
    const T    out_    = aEnds ? a.m_out : b.m_out;

    if (in < out_) {
      out.add(in, out_, aStarts ? a.m_inPrimitive : b.m_inPrimitive, aEnds ? a.m_outPrimitive : b.m_outPrimitive);
    }

    if (aEnds) {
      i++;
    }
    else {
      j++;
    }
  }
}

/**
 * @brief The spans of the complement of a set of spans, within [a_tMin, a_tMax].
 * @details Internal helper; not part of the public API. The ends a span has at a_tMin or a_tMax, which
 * are not crossings, are RaySpan::Open.
 * @tparam T Floating-point precision.
 * @tparam N Most spans held.
 * @param[in]  a_a    The spans.
 * @param[in]  a_tMin Start of the interval.
 * @param[in]  a_tMax End of the interval.
 * @param[out] a_out  The complement (not a_a).
 */
template <class T, uint32_t N>
EBGEOMETRY_HOST_DEVICE
inline void
complement(const RaySpans<T, N>& a_a, const T a_tMin, const T a_tMax, RaySpans<T, N>& a_out) noexcept
{
  RaySpans<T, N>& out = a_out;

  out.clear();

  out.m_complete = a_a.m_complete;
  out.m_exact    = a_a.m_exact;
  out.m_resolved = a_a.m_resolved;

  T        start   = a_tMin;
  uint32_t startId = RaySpan<T>::Open;

  for (uint32_t i = 0; i < a_a.m_count; i++) {
    const RaySpan<T>& span = a_a.m_spans[i];

    if (span.m_in > start) {
      out.add(start, span.m_in, startId, span.m_inPrimitive);
    }

    start   = span.m_out;
    startId = span.m_outPrimitive;
  }

  if (start < a_tMax) {
    out.add(start, a_tMax, startId, RaySpan<T>::Open);
  }
}

/**
 * @brief The first crossing among a set of spans at or after a_tMin: the first end that is not open.
 * @details Internal helper; not part of the public API. HitT::m_point is left for the caller.
 * @tparam T Floating-point precision.
 * @tparam N Most spans held.
 * @param[in] a_spans The spans.
 * @param[in] a_tMin  Start of the interval.
 * @return The crossing, a miss, or unresolved if a march gave up before any crossing was found.
 */
template <class T, uint32_t N>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
firstOf(const RaySpans<T, N>& a_spans, const T a_tMin) noexcept
{
  HitT<T> hit;

  for (uint32_t i = 0; i < a_spans.m_count; i++) {
    const RaySpan<T>& span = a_spans.m_spans[i];

    for (int end = 0; end < 2; end++) {
      const T        t  = (end == 0) ? span.m_in : span.m_out;
      const uint32_t id = (end == 0) ? span.m_inPrimitive : span.m_outPrimitive;

      if (id != RaySpan<T>::Open && t >= a_tMin) {
        hit.m_status    = HitStatus::Hit;
        hit.m_t         = t;
        hit.m_primitive = id;
        hit.m_entering  = (end == 0);
        hit.m_exact     = a_spans.m_exact;

        return hit;
      }
    }
  }

  if (!a_spans.m_resolved) {
    hit.m_status = HitStatus::Unresolved;
  }

  return hit;
}
} // namespace RayDetail

template <class T>
EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
RayT<T>::at(const T a_t) const noexcept
{
  return m_origin + a_t * m_direction;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline bool
HitT<T>::hit() const noexcept
{
  return m_status == HitStatus::Hit;
}

template <class T, uint32_t N>
EBGEOMETRY_HOST_DEVICE
inline void
RaySpans<T, N>::add(const T a_in, const T a_out, const uint32_t a_inPrimitive, const uint32_t a_outPrimitive) noexcept
{
  if (m_count < N) {
    m_spans[m_count++] = RaySpan<T>{a_in, a_out, a_inPrimitive, a_outPrimitive};
  }
  else {
    m_complete = false;
  }
}

template <class T, uint32_t N>
EBGEOMETRY_HOST_DEVICE
inline void
RaySpans<T, N>::clear() noexcept
{
  m_count    = 0;
  m_complete = true;
  m_exact    = true;
  m_resolved = true;
}

template <class T, uint32_t N>
EBGEOMETRY_HOST_DEVICE
inline void
RaySpans<T, N>::copyFrom(const RaySpans& a_other) noexcept
{
  for (uint32_t i = 0; i < a_other.m_count; i++) {
    m_spans[i] = a_other.m_spans[i];
  }

  m_count    = a_other.m_count;
  m_complete = a_other.m_complete;
  m_exact    = a_other.m_exact;
  m_resolved = a_other.m_resolved;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<PlaneSDF<T>>::firstHit(const PlaneSDF<T>& a_shape,
                               const RayT<T>&     a_ray,
                               const T            a_tMin,
                               const T            a_tMax,
                               const RayOptions<T>& /*a_options*/) noexcept
{
  // The plane's value along the ray is v0 + t * rate; the solid is where it is negative.
  const Vec3T<T>& n    = a_shape.getNormal();
  const T         v0   = dot(a_ray.m_origin - a_shape.getPoint(), n);
  const T         rate = dot(a_ray.m_direction, n);
  const T         inf  = Math::Limits<T>::infinity();

  if (rate == T(0)) {
    return HitT<T>();
  }

  const T crossing = -v0 / rate;

  return (rate < T(0)) ? RayDetail::convexHit(a_ray, crossing, n, inf, n, a_tMin, a_tMax)
                       : RayDetail::convexHit(a_ray, -inf, n, crossing, n, a_tMin, a_tMax);
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
  T in;
  T out;

  RayDetail::ballSpan(a_ray, a_shape.getCenter(), a_shape.getRadius(), in, out);

  const T r = a_shape.getRadius();

  return RayDetail::convexHit(a_ray,
                              in,
                              (a_ray.at(in) - a_shape.getCenter()) / r,
                              out,
                              (a_ray.at(out) - a_shape.getCenter()) / r,
                              a_tMin,
                              a_tMax);
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
  T        in        = -Math::Limits<T>::infinity();
  T        out       = Math::Limits<T>::infinity();
  Vec3T<T> normalIn  = Vec3T<T>::zeros();
  Vec3T<T> normalOut = Vec3T<T>::zeros();

  for (size_t axis = 0; axis < 3; axis++) {
    RayDetail::slab(
      a_ray, axis, a_shape.getLowCorner()[axis], a_shape.getHighCorner()[axis], in, normalIn, out, normalOut);
  }

  return RayDetail::convexHit(a_ray, in, normalIn, out, normalOut, a_tMin, a_tMax);
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
  T        in;
  T        out;
  Vec3T<T> normalIn;
  Vec3T<T> normalOut;

  RayDetail::cylinderSpan(
    a_ray, a_shape.getCenter1(), a_shape.getCenter2(), a_shape.getRadius(), in, normalIn, out, normalOut);

  return RayDetail::convexHit(a_ray, in, normalIn, out, normalOut, a_tMin, a_tMax);
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
  Vec3T<T> axis = Vec3T<T>::zeros();

  axis[a_shape.getAxis()] = T(1);

  T in;
  T out;

  RayDetail::tubeSpan(a_ray, a_shape.getCenter(), axis, a_shape.getRadius(), in, out);

  return RayDetail::convexHit(a_ray,
                              in,
                              RayDetail::tubeNormal(a_ray.at(in), a_shape.getCenter(), axis),
                              out,
                              RayDetail::tubeNormal(a_ray.at(out), a_shape.getCenter(), axis),
                              a_tMin,
                              a_tMax);
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
  const Vec3T<T>& c1     = a_shape.getCenter1();
  const Vec3T<T>& c2     = a_shape.getCenter2();
  const T         length = (c2 - c1).length();

  T in;
  T out;

  span(a_shape, a_ray, in, out);

  if (!(in <= out)) {
    return HitT<T>();
  }

  // The normal of a capsule at a surface point is away from the nearest point of its segment.
  const auto normal = [&c1, &c2, length](const Vec3T<T>& a_p) noexcept {
    const Vec3T<T> axis = (length > T(0)) ? (c2 - c1) / length : Vec3T<T>::zeros();
    const T        h    = Math::clamp(dot(a_p - c1, axis), T(0), length);
    const Vec3T<T> n    = a_p - (c1 + h * axis);
    const T        l    = n.length();

    return (l > T(0)) ? n / l : Vec3T<T>::zeros();
  };

  return RayDetail::convexHit(a_ray, in, normal(a_ray.at(in)), out, normal(a_ray.at(out)), a_tMin, a_tMax);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<PlaneSDF<T>>::span(const PlaneSDF<T>& a_shape, const RayT<T>& a_ray, T& a_in, T& a_out) noexcept
{
  const Vec3T<T>& n    = a_shape.getNormal();
  const T         v0   = dot(a_ray.m_origin - a_shape.getPoint(), n);
  const T         rate = dot(a_ray.m_direction, n);
  const T         inf  = Math::Limits<T>::infinity();

  if (rate == T(0)) {
    a_in  = (v0 < T(0)) ? -inf : inf;
    a_out = (v0 < T(0)) ? inf : -inf;

    return;
  }

  const T crossing = -v0 / rate;

  a_in  = (rate < T(0)) ? crossing : -inf;
  a_out = (rate < T(0)) ? inf : crossing;
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<SphereSDF<T>>::span(const SphereSDF<T>& a_shape, const RayT<T>& a_ray, T& a_in, T& a_out) noexcept
{
  RayDetail::ballSpan(a_ray, a_shape.getCenter(), a_shape.getRadius(), a_in, a_out);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<BoxSDF<T>>::span(const BoxSDF<T>& a_shape, const RayT<T>& a_ray, T& a_in, T& a_out) noexcept
{
  Vec3T<T> normalIn;
  Vec3T<T> normalOut;

  a_in  = -Math::Limits<T>::infinity();
  a_out = Math::Limits<T>::infinity();

  for (size_t axis = 0; axis < 3; axis++) {
    RayDetail::slab(
      a_ray, axis, a_shape.getLowCorner()[axis], a_shape.getHighCorner()[axis], a_in, normalIn, a_out, normalOut);
  }
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<CylinderSDF<T>>::span(const CylinderSDF<T>& a_shape, const RayT<T>& a_ray, T& a_in, T& a_out) noexcept
{
  Vec3T<T> normalIn;
  Vec3T<T> normalOut;

  RayDetail::cylinderSpan(
    a_ray, a_shape.getCenter1(), a_shape.getCenter2(), a_shape.getRadius(), a_in, normalIn, a_out, normalOut);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<InfiniteCylinderSDF<T>>::span(const InfiniteCylinderSDF<T>& a_shape,
                                      const RayT<T>&                a_ray,
                                      T&                            a_in,
                                      T&                            a_out) noexcept
{
  Vec3T<T> axis = Vec3T<T>::zeros();

  axis[a_shape.getAxis()] = T(1);

  RayDetail::tubeSpan(a_ray, a_shape.getCenter(), axis, a_shape.getRadius(), a_in, a_out);
}

template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
RayForm<CapsuleSDF<T>>::span(const CapsuleSDF<T>& a_shape, const RayT<T>& a_ray, T& a_in, T& a_out) noexcept
{
  // The capsule is convex, so the ray is inside it over one span: the hull of its spans through the
  // cylinder between the centres and through the two balls.
  const Vec3T<T>& c1 = a_shape.getCenter1();
  const Vec3T<T>& c2 = a_shape.getCenter2();
  const T         r  = a_shape.getRadius();

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

  RayDetail::ballSpan(a_ray, c1, r, a, b);
  widen(a, b);
  RayDetail::ballSpan(a_ray, c2, r, a, b);
  widen(a, b);

  if ((c2 - c1).length() > T(0)) {
    Vec3T<T> nA;
    Vec3T<T> nB;

    RayDetail::cylinderSpan(a_ray, c1, c2, r, a, nA, b, nB);
    widen(a, b);
  }
}

template <class T, size_t K, size_t W>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
RayForm<TriMeshSDF<T, K, W>>::firstHit(const TriMeshSDF<T, K, W>& a_sdf,
                                       const RayT<T>&             a_ray,
                                       const T                    a_tMin,
                                       const T                    a_tMax,
                                       const RayOptions<T>& /*a_options*/) noexcept
{
  HitT<T> hit;

  if (!(a_tMin <= a_tMax)) {
    return hit;
  }

  using Root = typename TriMeshSDF<T, K, W>::Root;

  const Root bvh = a_sdf.getRoot();

  T        best     = a_tMax;
  bool     found    = false;
  uint32_t face     = 0;
  bool     entering = false;
  Vec3T<T> normal   = Vec3T<T>::zeros();

  // A node's key is where the ray enters its box, or infinity if it misses it (or meets it only
  // beyond the nearest crossing so far, which the prune then skips).
  const auto entry = [&bvh, &a_ray, a_tMin, &best](const auto& a_node) noexcept -> T {
    const auto box = bvh.getBoundingVolume(a_node);

    T t0 = a_tMin;
    T t1 = best;

    return RayDetail::clip(box.getLowCorner(), box.getHighCorner(), a_ray, t0, t1) ? t0 : Math::Limits<T>::infinity();
  };

  const auto leaf = [&a_ray, a_tMin, &best, &found, &face, &entering, &normal](
                      const auto& a_groups, const size_t a_offset, const size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      if (a_groups[static_cast<uint32_t>(i)].rayHit(
            a_ray.m_origin, a_ray.m_direction, a_tMin, best, face, normal, entering)) {
        found = true;
      }
    }
  };

  const auto prune = [&best](const auto& /*a_node*/, const T& a_key) noexcept -> bool { return a_key <= best; };

  // The nearest box last, since the last child is visited first.
  const auto order = [](auto& a_children) noexcept {
    for (size_t i = 1; i < K; i++) {
      for (size_t j = i; j > 0 && a_children[j - 1].second < a_children[j].second; j--) {
        const auto swap   = a_children[j - 1];
        a_children[j - 1] = a_children[j];
        a_children[j]     = swap;
      }
    }
  };

  bvh.template traverse<T>(leaf, prune, order, entry);

  if (found) {
    hit.m_status    = HitStatus::Hit;
    hit.m_t         = best;
    hit.m_point     = a_ray.at(best);
    hit.m_primitive = face;
    hit.m_entering  = entering;
    hit.m_exact     = true;
    hit.m_normal    = normal;
  }

  return hit;
}

namespace RayDetail {

/**
 * @brief The first crossing of a shape's surface along a ray, by marching its signed distance: sphere
 * tracing where the value is a distance or a bound on one, fixed steps and bisection where it is not.
 * @details Internal helper; not part of the public API. What firstHit() does for a shape with no ray
 * form, and what a ray form may fall back on.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape   Shape.
 * @param[in] a_ray     Ray, with a unit direction.
 * @param[in] a_tMin    Start of the interval.
 * @param[in] a_tMax    End of the interval.
 * @param[in] a_options How to march.
 * @return The crossing, a miss, or an unresolved query.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
march(const S& a_shape, const RayT<T>& a_ray, const T a_tMin, const T a_tMax, const RayOptions<T>& a_options) noexcept
{
  EBGEOMETRY_EXPECT(std::abs(a_ray.m_direction.length() - T(1)) < T(1e-3));

  const auto box = a_shape.computeBoundingVolume();
  const T    eps = a_options.m_tolerance * RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);

  HitT<T> hit;

  // Only the part of the ray inside the box (widened by the tolerance) can cross the surface.
  const Vec3T<T> pad = eps * Vec3T<T>::ones();

  T t0 = a_tMin;
  T t1 = a_tMax;

  if (!(a_tMin <= a_tMax) || !RayDetail::clip(box.getLowCorner() - pad, box.getHighCorner() + pad, a_ray, t0, t1)) {
    return hit;
  }

  const auto value = [&a_shape, &a_ray](const T a_t) noexcept -> T { return a_shape.signedDistance(a_ray.at(a_t)); };

  const auto found = [&a_shape, &a_ray, &hit](const T a_t, const bool a_entering) noexcept -> HitT<T> {
    hit.m_status    = HitStatus::Hit;
    hit.m_t         = a_t;
    hit.m_point     = a_ray.at(a_t);
    hit.m_primitive = RayDetail::primitiveOf(a_shape, hit.m_point);
    hit.m_entering  = a_entering;

    return hit;
  };

  // The crossing between a and b, where the value changes sign: halve the interval until it is shorter
  // than the tolerance. a keeps the starting side's sign.
  const auto bisect = [&value, eps](T a_a, T a_b, const bool a_inside) noexcept -> T {
    for (int i = 0; i < 64 && a_b - a_a > eps; i++) {
      const T mid = T(0.5) * (a_a + a_b);

      if ((value(mid) < T(0)) == a_inside) {
        a_a = mid;
      }
      else {
        a_b = mid;
      }
    }

    return T(0.5) * (a_a + a_b);
  };

  const T v0 = value(t0);

  // Starting on the surface (as the march does where a face of the object lies on its box): the ray
  // enters if the value falls along it, and leaves if it rises.
  if (std::abs(v0) < eps) {
    return found(t0, value(Math::min(t0 + T(2) * eps, t1)) < v0);
  }

  const bool inside = v0 < T(0);

  T t     = t0;
  T tPrev = t0;

  if (RayDetail::qualityOf(a_shape) != DistanceQuality::NotADistance) {
    // Sphere tracing: |value| is at most the distance to the surface, so a step of |value| cannot cross it.
    for (uint32_t step = 0; step < a_options.m_maxSteps; step++) {
      const T v = (step == 0U) ? v0 : value(t);

      if ((v < T(0)) != inside) {
        // A value that is not quite a bound (rounding, say) stepped across: find the crossing.
        return found(bisect(tPrev, t, inside), !inside);
      }

      if (std::abs(v) < eps) {
        return found(t, !inside);
      }

      tPrev = t;
      t += std::abs(v);

      if (t > t1) {
        return hit;
      }
    }
  }
  else {
    // Fixed steps, then bisection on the first change of sign.
    const T h = a_options.m_step * RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);

    for (uint32_t step = 0; step < a_options.m_maxSteps; step++) {
      tPrev = t;
      t     = Math::min(t + h, t1);

      if ((value(t) < T(0)) != inside) {
        return found(bisect(tPrev, t, inside), !inside);
      }

      if (t >= t1) {
        return hit;
      }
    }
  }

  hit.m_status = HitStatus::Unresolved;
  hit.m_t      = t;
  hit.m_point  = a_ray.at(t);

  return hit;
}

} // namespace RayDetail

template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
firstHit(const S&                       a_shape,
         const RayT<T>&                 a_ray,
         const typename RayT<T>::Scalar a_tMin,
         const typename RayT<T>::Scalar a_tMax,
         const RayOptions<T>&           a_options) noexcept
{
  EBGEOMETRY_EXPECT(std::abs(a_ray.m_direction.length() - T(1)) < T(1e-3));

  if constexpr (RayForm<S>::available) {
    return RayForm<S>::firstHit(a_shape, a_ray, a_tMin, a_tMax, a_options);
  }
  else {
    return RayDetail::march(a_shape, a_ray, a_tMin, a_tMax, a_options);
  }
}

namespace RayDetail {

/**
 * @brief The spans of a ray inside a shape, within [a_tMin, a_tMax]: the crossings firstHit() finds one
 * after another, at most N spans.
 * @details Internal helper; not part of the public API. Each search starts just past the crossing before
 * it: by a few units in the last place after an exact crossing, by a few tolerances after a sphere-traced
 * one, and further if it found the same crossing again.
 * @tparam T Floating-point precision.
 * @tparam N Most spans held.
 * @tparam S Shape type.
 * @param[in]  a_shape   Shape.
 * @param[in]  a_ray     Ray, with a unit direction.
 * @param[in]  a_tMin    Start of the interval.
 * @param[in]  a_tMax    End of the interval.
 * @param[in]  a_options How to march.
 * @param[out] a_spans   The spans.
 */
template <class T, uint32_t N, class S>
EBGEOMETRY_HOST_DEVICE
inline void
spansOf(const S&             a_shape,
        const RayT<T>&       a_ray,
        const T              a_tMin,
        const T              a_tMax,
        const RayOptions<T>& a_options,
        RaySpans<T, N>&      a_spans) noexcept
{
  a_spans.clear();

  // A convex shape gives its one span at once.
  if constexpr (HasSpan<S, T>::value) {
    T in;
    T out;

    RayForm<S>::span(a_shape, a_ray, in, out);

    const T lo = Math::max(in, a_tMin);
    const T hi = Math::min(out, a_tMax);

    if (in <= out && lo <= hi) {
      a_spans.add(lo, hi, (in >= a_tMin) ? 0U : RaySpan<T>::Open, (out <= a_tMax) ? 0U : RaySpan<T>::Open);
    }

    return;
  }

  const auto box   = a_shape.computeBoundingVolume();
  const T    scale = scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);

  bool     inside  = a_shape.signedDistance(a_ray.at(a_tMin)) < T(0);
  T        start   = a_tMin;
  uint32_t startId = RaySpan<T>::Open;
  T        t       = a_tMin;

  for (uint32_t k = 0; k < 4U * N + 4U && t <= a_tMax; k++) {
    const HitT<T> hit = firstHit(a_shape, a_ray, t, a_tMax, a_options);

    if (!hit.hit()) {
      a_spans.m_resolved = a_spans.m_resolved && (hit.m_status != HitStatus::Unresolved);

      break;
    }

    a_spans.m_exact = a_spans.m_exact && hit.m_exact;

    if (hit.m_entering) {
      if (!inside) {
        start   = hit.m_t;
        startId = hit.m_primitive;
        inside  = true;
      }
    }
    else {
      a_spans.add(inside ? start : a_tMin, hit.m_t, inside ? startId : RaySpan<T>::Open, hit.m_primitive);

      inside = false;
    }

    // Past the crossing: just past an exact one, a few tolerances past a sphere-traced one.
    const T past = hit.m_exact ? T(64) * Math::Limits<T>::epsilon() * Math::max(std::abs(hit.m_t), scale)
                               : T(4) * a_options.m_tolerance * scale;

    t = Math::max(hit.m_t + past, t + past);
  }

  if (inside) {
    a_spans.add(start, a_tMax, startId, RaySpan<T>::Open);
  }
}

} // namespace RayDetail

template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline HitStatus
intersects(const S& a_shape, const Vec3T<T>& a_from, const Vec3T<T>& a_to, const RayOptions<T>& a_options) noexcept
{
  const T a = a_shape.signedDistance(a_from);
  const T b = a_shape.signedDistance(a_to);

  if ((a < T(0)) != (b < T(0)) || a == T(0) || b == T(0)) {
    return HitStatus::Hit;
  }

  const Vec3T<T> d = a_to - a_from;
  const T        L = d.length();

  if (!(L > T(0))) {
    return HitStatus::Miss;
  }

  const RayT<T> ray{a_from, d / L};

  return firstHit(a_shape, ray, T(0), L, a_options).m_status;
}

} // namespace EBGeometry

#endif
