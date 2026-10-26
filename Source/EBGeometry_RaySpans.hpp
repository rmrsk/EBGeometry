// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RaySpans.hpp
 * @brief  The span arithmetic that traces a composition along a ray: each operand's spans (where the ray
 * is inside it, see RaySpansT), combined by union, intersection and complement, mapped through affine
 * maps, and read for the first crossing. The fused operations, BVHUnion and Tape all trace through it.
 * @details Every list is complete up to its horizon. A combination is complete up to the smaller of its
 * operands' horizons, and a span that reaches past it ends there with an Open end. The first crossing of
 * the whole is read from the final list; where the list's horizon comes before any crossing, the trace
 * starts again from the horizon (traceSpans()). An operand under unions only needs its first crossing (the
 * union's first crossing is the nearest of its operands', unless the ray starts inside one), so it is
 * asked for that alone, and its horizon is there; an operand of an intersection or a complement is asked
 * for all its spans.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_RAYSPANS_HPP
#define EBGEOMETRY_RAYSPANS_HPP

// Std includes
#include <cmath>
#include <cstdint>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Ray.hpp"

namespace EBGeometry {

namespace RayDetail {

/**
 * @brief An end that is no crossing.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_t Where it is.
 * @return The end.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline SpanEndT<T>
openAt(const T a_t) noexcept
{
  return SpanEndT<T>{a_t, Vec3T<T>::zeros(), 0U, SpanEndT<T>::Open};
}

/**
 * @brief The end a hit is.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_hit The hit.
 * @return The end.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline SpanEndT<T>
endOf(const HitT<T>& a_hit) noexcept
{
  return SpanEndT<T>{
    a_hit.m_t, a_hit.m_normal, a_hit.m_primitive, a_hit.m_exact ? SpanEndT<T>::Exact : SpanEndT<T>::Marched};
}

/**
 * @brief The same end seen from the complement: its normal reversed.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_end The end.
 * @return The flipped end.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline SpanEndT<T>
flipped(const SpanEndT<T>& a_end) noexcept
{
  SpanEndT<T> end = a_end;

  end.m_normal = -end.m_normal;

  return end;
}

/**
 * @brief Copy a list, only as many spans as it holds.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in]  a_from The list.
 * @param[out] a_to   The copy.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
copySpans(const RaySpansT<T>& a_from, RaySpansT<T>& a_to) noexcept
{
  a_to.m_count   = a_from.m_count;
  a_to.m_horizon = a_from.m_horizon;

  for (uint32_t i = 0; i < a_from.m_count; i++) {
    a_to.m_spans[i] = a_from.m_spans[i];
  }
}

/**
 * @brief A span cut to a horizon: an end beyond it becomes an Open end there.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_span    The span; it begins at or before the horizon.
 * @param[in] a_horizon The horizon.
 * @return The span.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline typename RaySpansT<T>::Span
cut(const typename RaySpansT<T>::Span& a_span, const T a_horizon) noexcept
{
  typename RaySpansT<T>::Span span = a_span;

  if (span.m_out.m_t > a_horizon) {
    span.m_out = openAt(a_horizon);
  }

  return span;
}

/**
 * @brief The union of two lists: where the ray is inside either.
 * @details Internal helper; not part of the public API. Complete up to the smaller horizon. Where two ends
 * meet, the one that is no crossing wins, since the ray is inside the other operand there.
 * @tparam T Floating-point precision.
 * @param[in]  a_a   One list.
 * @param[in]  a_b   The other.
 * @param[out] a_out The union; not one of the two.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
unite(const RaySpansT<T>& a_a, const RaySpansT<T>& a_b, RaySpansT<T>& a_out) noexcept
{
  const T horizon = Math::min(a_a.m_horizon, a_b.m_horizon);

  a_out.reset(horizon);

  uint32_t i = 0;
  uint32_t j = 0;

  bool                        open = false;
  typename RaySpansT<T>::Span current{};

  while (true) {
    const bool fromA = i < a_a.m_count && a_a.m_spans[i].m_in.m_t <= horizon;
    const bool fromB = j < a_b.m_count && a_b.m_spans[j].m_in.m_t <= horizon;

    if (!fromA && !fromB) {
      break;
    }

    // The span that begins first, and of two that begin together the one already inside.
    bool takeA = fromA;

    if (fromA && fromB) {
      const SpanEndT<T>& a = a_a.m_spans[i].m_in;
      const SpanEndT<T>& b = a_b.m_spans[j].m_in;

      takeA = a.m_t < b.m_t || (a.m_t == b.m_t && !a.crossing());
    }

    const typename RaySpansT<T>::Span span = cut(takeA ? a_a.m_spans[i++] : a_b.m_spans[j++], horizon);

    if (!open) {
      current = span;
      open    = true;
    }
    else if (span.m_in.m_t <= current.m_out.m_t) {
      if (span.m_out.m_t > current.m_out.m_t || (span.m_out.m_t == current.m_out.m_t && !span.m_out.crossing())) {
        current.m_out = span.m_out;
      }
    }
    else {
      a_out.add(current.m_in, current.m_out);

      current = span;
    }
  }

  if (open) {
    a_out.add(current.m_in, current.m_out);
  }
}

/**
 * @brief The intersection of two lists: where the ray is inside both.
 * @details Internal helper; not part of the public API. Complete up to the smaller horizon. Where two ends
 * meet, the crossing wins, since the ray crosses the other operand's surface there.
 * @tparam T Floating-point precision.
 * @param[in]  a_a   One list.
 * @param[in]  a_b   The other.
 * @param[out] a_out The intersection; not one of the two.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
intersect(const RaySpansT<T>& a_a, const RaySpansT<T>& a_b, RaySpansT<T>& a_out) noexcept
{
  const T horizon = Math::min(a_a.m_horizon, a_b.m_horizon);

  a_out.reset(horizon);

  uint32_t i = 0;
  uint32_t j = 0;

  while (i < a_a.m_count && j < a_b.m_count) {
    if (a_a.m_spans[i].m_in.m_t > horizon || a_b.m_spans[j].m_in.m_t > horizon) {
      break;
    }

    const typename RaySpansT<T>::Span a = cut(a_a.m_spans[i], horizon);
    const typename RaySpansT<T>::Span b = cut(a_b.m_spans[j], horizon);

    const bool laterA = a.m_in.m_t > b.m_in.m_t || (a.m_in.m_t == b.m_in.m_t && a.m_in.crossing());
    const bool earlyA = a.m_out.m_t < b.m_out.m_t || (a.m_out.m_t == b.m_out.m_t && a.m_out.crossing());

    const SpanEndT<T>& in  = laterA ? a.m_in : b.m_in;
    const SpanEndT<T>& out = earlyA ? a.m_out : b.m_out;

    if (in.m_t <= out.m_t) {
      a_out.add(in, out);
    }

    // The span that ends first can meet nothing more.
    if (a.m_out.m_t <= b.m_out.m_t) {
      i++;
    }
    else {
      j++;
    }
  }
}

/**
 * @brief Cut a list to a horizon: spans that begin beyond it dropped, and ends beyond it Open ends there.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in]     a_horizon The horizon; no later than the list's.
 * @param[in,out] a_spans   The list.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
cutTo(const T a_horizon, RaySpansT<T>& a_spans) noexcept
{
  uint32_t count = 0;

  while (count < a_spans.m_count && a_spans.m_spans[count].m_in.m_t <= a_horizon) {
    if (a_spans.m_spans[count].m_out.m_t > a_horizon) {
      a_spans.m_spans[count].m_out = openAt(a_horizon);
    }

    count++;
  }

  a_spans.m_count   = count;
  a_spans.m_horizon = a_horizon;
}

/**
 * @brief Keep a list only up to its first crossing, which is all a union asked for its first crossing
 * needs: the horizon there.
 * @details Internal helper; not part of the public API. A shorter list makes the next union cheaper, and
 * the next operand is traced only up to it.
 * @tparam T Floating-point precision.
 * @param[in,out] a_spans The list.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
keepFirst(RaySpansT<T>& a_spans) noexcept
{
  if (a_spans.m_count == 0U) {
    return;
  }

  const typename RaySpansT<T>::Span& first = a_spans.m_spans[0];

  if (first.m_in.crossing()) {
    RayDetail::cutTo(first.m_in.m_t, a_spans);
  }
  else if (first.m_out.crossing()) {
    RayDetail::cutTo(first.m_out.m_t, a_spans);
  }
}

/**
 * @brief Unite a list into another, in place.
 * @details Internal helper; not part of the public API. An operand the ray is nowhere inside (the common
 * case in a union of many) only cuts the list to its horizon, with no copy.
 * @tparam T Floating-point precision.
 * @param[in,out] a_spans   The list.
 * @param[in]     a_operand The list to unite into it.
 * @param[in,out] a_scratch Scratch.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
uniteInto(RaySpansT<T>& a_spans, const RaySpansT<T>& a_operand, RaySpansT<T>& a_scratch) noexcept
{
  if (a_operand.m_count == 0U) {
    if (a_operand.m_horizon < a_spans.m_horizon) {
      RayDetail::cutTo(a_operand.m_horizon, a_spans);
    }
  }
  else if (a_spans.m_count == 0U && a_operand.m_horizon <= a_spans.m_horizon) {
    RayDetail::copySpans(a_operand, a_spans);
  }
  else {
    RayDetail::unite(a_spans, a_operand, a_scratch);
    RayDetail::copySpans(a_scratch, a_spans);
  }
}

/**
 * @brief Intersect a list with another, in place.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in,out] a_spans   The list.
 * @param[in]     a_operand The list to intersect it with.
 * @param[in,out] a_scratch Scratch.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
intersectInto(RaySpansT<T>& a_spans, const RaySpansT<T>& a_operand, RaySpansT<T>& a_scratch) noexcept
{
  if (a_operand.m_count == 0U || a_spans.m_count == 0U) {
    a_spans.reset(Math::min(a_spans.m_horizon, a_operand.m_horizon));

    return;
  }

  RayDetail::intersect(a_spans, a_operand, a_scratch);
  RayDetail::copySpans(a_scratch, a_spans);
}

/**
 * @brief The complement of a list over [a_t0, a_t1]: where the ray is outside, with the normals reversed.
 * @details Internal helper; not part of the public API. Complete up to the list's horizon.
 * @tparam T Floating-point precision.
 * @param[in]  a_a   The list.
 * @param[in]  a_t0  Start of the interval.
 * @param[out] a_out The complement; not the list.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
complement(const RaySpansT<T>& a_a, const T a_t0, RaySpansT<T>& a_out) noexcept
{
  const T horizon = a_a.m_horizon;

  a_out.reset(horizon);

  // The gap the ray is in: from where it starts, while there is one.
  SpanEndT<T> start = openAt(a_t0);
  bool        gap   = true;

  for (uint32_t i = 0; i < a_a.m_count && a_a.m_spans[i].m_in.m_t <= horizon; i++) {
    const typename RaySpansT<T>::Span span = cut(a_a.m_spans[i], horizon);

    // A span that begins at the start of the interval, inside, has no gap before it.
    if (span.m_in.crossing() && gap) {
      a_out.add(start, flipped(span.m_in));
    }

    // A span that ends where nothing more is known leaves no gap after it.
    gap = span.m_out.crossing();

    if (!gap) {
      break;
    }

    start = flipped(span.m_out);
  }

  if (gap) {
    a_out.add(start, openAt(horizon));
  }
}

/**
 * @brief A ray seen through an affine map, with the interval it is traced over: what the operand of the
 * map is traced with.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 */
template <class T>
struct MappedRay
{
  /**
   * @brief The mapped ray, with a unit direction.
   */
  RayT<T> m_ray;

  /**
   * @brief Distance along the mapped ray per distance along the original.
   */
  T m_scale;

  /**
   * @brief Start of the interval, along the mapped ray.
   */
  T m_t0;

  /**
   * @brief End of the interval, along the mapped ray.
   */
  T m_t1;
};

/**
 * @brief A ray mapped by an affine map (CSGOps::affinePoint()'s coefficients: the matrix's rows, then the
 * shift).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_map The map's coefficients.
 * @param[in] a_ray The ray.
 * @param[in] a_t0  Start of the interval.
 * @param[in] a_t1  End of the interval.
 * @return The mapped ray.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline MappedRay<T>
mapRay(const T* a_map, const RayT<T>& a_ray, const T a_t0, const T a_t1) noexcept
{
  const T*        m = a_map;
  const Vec3T<T>& o = a_ray.m_origin;
  const Vec3T<T>& d = a_ray.m_direction;

  const Vec3T<T> origin(m[0] * o[0] + m[1] * o[1] + m[2] * o[2] + m[9],
                        m[3] * o[0] + m[4] * o[1] + m[5] * o[2] + m[10],
                        m[6] * o[0] + m[7] * o[1] + m[8] * o[2] + m[11]);
  const Vec3T<T> direction(m[0] * d[0] + m[1] * d[1] + m[2] * d[2],
                           m[3] * d[0] + m[4] * d[1] + m[5] * d[2],
                           m[6] * d[0] + m[7] * d[1] + m[8] * d[2]);

  const T scale = direction.length();

  return MappedRay<T>{RayT<T>{origin, direction / scale}, scale, a_t0 * scale, a_t1 * scale};
}

/**
 * @brief Map an operand's list, traced along a mapped ray, back to the original ray: distances divided by
 * the scale, normals by the transposed matrix.
 * @details Internal helper; not part of the public API. The ends of the interval map back to exactly the
 * ends of the original one, so that a list complete over the mapped interval is complete over the original.
 * @tparam T Floating-point precision.
 * @param[in]  a_map    The map's coefficients.
 * @param[in]  a_mapped The mapped ray.
 * @param[in]  a_t0     Start of the original interval.
 * @param[in]  a_t1     End of the original interval.
 * @param[in]  a_from   The operand's list.
 * @param[out] a_to     The list along the original ray; may be a_from.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
mapBack(const T*            a_map,
        const MappedRay<T>& a_mapped,
        const T             a_t0,
        const T             a_t1,
        const RaySpansT<T>& a_from,
        RaySpansT<T>&       a_to) noexcept
{
  const T* m = a_map;

  const auto distance = [&a_mapped, a_t0, a_t1](const T a_t) noexcept -> T {
    if (a_t == a_mapped.m_t0) {
      return a_t0;
    }
    if (a_t >= a_mapped.m_t1) {
      return a_t1;
    }

    return a_t / a_mapped.m_scale;
  };

  const auto end = [&distance, m](const SpanEndT<T>& a_end) noexcept -> SpanEndT<T> {
    SpanEndT<T> e = a_end;

    e.m_t = distance(a_end.m_t);

    if (a_end.crossing()) {
      const Vec3T<T>& n = a_end.m_normal;
      const Vec3T<T>  g(m[0] * n[0] + m[3] * n[1] + m[6] * n[2],
                       m[1] * n[0] + m[4] * n[1] + m[7] * n[2],
                       m[2] * n[0] + m[5] * n[1] + m[8] * n[2]);
      const T         length = g.length();

      e.m_normal = (length > T(0)) ? g / length : g;
    }

    return e;
  };

  a_to.m_count   = a_from.m_count;
  a_to.m_horizon = distance(a_from.m_horizon);

  for (uint32_t i = 0; i < a_from.m_count; i++) {
    a_to.m_spans[i].m_in  = end(a_from.m_spans[i].m_in);
    a_to.m_spans[i].m_out = end(a_from.m_spans[i].m_out);
  }
}

/**
 * @brief Whether a point is inside a shape: within its box, and its value negative.
 * @details Internal helper; not part of the public API. What a list needs where the ray crosses nothing in
 * the interval: whether it is inside throughout.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape The shape.
 * @param[in] a_point The point.
 * @return True if inside.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline bool
insideAt(const S& a_shape, const Vec3T<T>& a_point) noexcept
{
  const auto      box = a_shape.computeBoundingVolume();
  const Vec3T<T>& lo  = box.getLowCorner();
  const Vec3T<T>& hi  = box.getHighCorner();

  for (size_t dir = 0; dir < 3; dir++) {
    if (!(a_point[dir] >= lo[dir] && a_point[dir] <= hi[dir])) {
      return false;
    }
  }

  return a_shape.signedDistance(a_point) < T(0);
}

/**
 * @brief The list up to a shape's first crossing, from firstHit(): a span from it if the ray enters there,
 * or to it if the ray starts inside; the horizon there.
 * @details Internal helper; not part of the public API. Where the ray crosses nothing, the list is the
 * whole interval if the ray is inside throughout, and empty otherwise. An unresolved query leaves the
 * horizon where it stopped.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]  a_shape   The shape.
 * @param[in]  a_ray     The ray.
 * @param[in]  a_t0      Start of the interval.
 * @param[in]  a_t1      End of the interval.
 * @param[in]  a_options How to march.
 * @param[out] a_spans   The list.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline void
firstSpan(const S&             a_shape,
          const RayT<T>&       a_ray,
          const T              a_t0,
          const T              a_t1,
          const RayOptions<T>& a_options,
          RaySpansT<T>&        a_spans) noexcept
{
  a_spans.reset(a_t1);

  const HitT<T> hit = EBGeometry::firstHit(a_shape, a_ray, a_t0, a_t1, a_options);

  switch (hit.m_status) {
  case HitStatus::Hit: {
    if (hit.m_entering) {
      a_spans.add(endOf(hit), openAt(hit.m_t));
    }
    else {
      a_spans.add(openAt(a_t0), endOf(hit));
    }

    a_spans.m_horizon = hit.m_t;

    break;
  }
  case HitStatus::Unresolved: {
    a_spans.m_horizon = Math::max(a_t0, Math::min(hit.m_t, a_t1));

    break;
  }
  default: {
    if (RayDetail::insideAt(a_shape, a_ray.at(a_t0))) {
      a_spans.add(openAt(a_t0), openAt(a_t1));
    }

    break;
  }
  }
}

/**
 * @brief Pairs crossings, in order, into spans: a span from each crossing the ray enters at to the next it
 * leaves at.
 * @details Internal helper; not part of the public API. A crossing the ray leaves at before any it enters
 * at closes a span from the start of the interval; a repeated crossing of the same kind (a ray through an
 * edge shared by two faces crosses both) is skipped.
 * @tparam T Floating-point precision.
 */
template <class T>
struct CrossingPairer
{
  /**
   * @brief Where the open span begins.
   */
  SpanEndT<T> m_start;

  /**
   * @brief Whether a span is open.
   */
  bool m_inside = false;

  /**
   * @brief Whether no crossing has been seen yet.
   */
  bool m_first = true;

  /**
   * @brief Take the next crossing.
   * @param[in]     a_end      The crossing.
   * @param[in]     a_entering Whether the ray enters there.
   * @param[in]     a_t0       Start of the interval.
   * @param[in,out] a_spans    The list.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  take(const SpanEndT<T>& a_end, const bool a_entering, const T a_t0, RaySpansT<T>& a_spans) noexcept
  {
    if (a_entering) {
      if (!m_inside) {
        m_start  = a_end;
        m_inside = true;
      }
    }
    else if (m_inside) {
      a_spans.add(m_start, a_end);

      m_inside = false;
    }
    else if (m_first) {
      a_spans.add(openAt(a_t0), a_end);
    }

    m_first = false;
  }

  /**
   * @brief Close the open span, if any, at an Open end.
   * @param[in]     a_t     Where it ends.
   * @param[in,out] a_spans The list.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  finish(const T a_t, RaySpansT<T>& a_spans) noexcept
  {
    if (m_inside) {
      a_spans.add(m_start, openAt(a_t));

      m_inside = false;
    }
  }
};

/**
 * @brief Every span of a shape over an interval, from firstHit() called from one crossing to the next.
 * @details Internal helper; not part of the public API. What a shape without spans() (see RayForm) gives an
 * intersection or a complement: each search starts a step (RayOptions::m_tolerance, times the scale) past
 * the last crossing.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]  a_shape   The shape.
 * @param[in]  a_ray     The ray.
 * @param[in]  a_t0      Start of the interval.
 * @param[in]  a_t1      End of the interval.
 * @param[in]  a_options How to march.
 * @param[out] a_spans   The list.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline void
crossingSpans(const S&             a_shape,
              const RayT<T>&       a_ray,
              const T              a_t0,
              const T              a_t1,
              const RayOptions<T>& a_options,
              RaySpansT<T>&        a_spans) noexcept
{
  a_spans.reset(a_t1);

  const auto box   = a_shape.computeBoundingVolume();
  const T    scale = RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);
  const T    step  = a_options.m_tolerance * scale;

  CrossingPairer<T> pairer;

  T t = a_t0;

  // Two crossings per span, and one more to find out the list is full.
  for (uint32_t k = 0; k < 2U * RaySpansT<T>::Capacity + 1U; k++) {
    const HitT<T> hit = EBGeometry::firstHit(a_shape, a_ray, t, a_t1, a_options);

    if (hit.m_status == HitStatus::Miss) {
      break;
    }

    if (hit.m_status == HitStatus::Unresolved) {
      const T horizon = Math::max(a_t0, Math::min(hit.m_t, a_t1));

      pairer.finish(horizon, a_spans);

      a_spans.m_horizon = Math::min(a_spans.m_horizon, horizon);

      return;
    }

    pairer.take(endOf(hit), hit.m_entering, a_t0, a_spans);

    if (a_spans.m_horizon < a_t1) {
      return;
    }

    t = hit.m_t + step;

    if (!(t <= a_t1)) {
      break;
    }

    if (k + 1U == 2U * RaySpansT<T>::Capacity + 1U) {
      // Out of searches with the interval not done: known only up to here.
      pairer.finish(t, a_spans);

      a_spans.m_horizon = Math::min(a_spans.m_horizon, t);

      return;
    }
  }

  if (pairer.m_first) {
    if (RayDetail::insideAt(a_shape, a_ray.at(a_t0))) {
      a_spans.add(openAt(a_t0), openAt(a_t1));
    }
  }
  else {
    pairer.finish(a_t1, a_spans);
  }
}

/**
 * @brief A shape's list over an interval: from its ray form's spans() if it has one, and otherwise from
 * firstHit(), up to the first crossing (firstSpan()) or from crossing to crossing (crossingSpans()).
 * @details Internal helper; not part of the public API. What a composition asks of each operand.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]  a_shape     The shape.
 * @param[in]  a_ray       The ray.
 * @param[in]  a_t0        Start of the interval.
 * @param[in]  a_t1        End of the interval.
 * @param[in]  a_options   How to march.
 * @param[in]  a_firstOnly Whether only the first crossing is needed.
 * @param[out] a_spans     The list.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline void
shapeSpans(const S&             a_shape,
           const RayT<T>&       a_ray,
           const T              a_t0,
           const T              a_t1,
           const RayOptions<T>& a_options,
           const bool           a_firstOnly,
           RaySpansT<T>&        a_spans) noexcept
{
  if constexpr (HasRaySpans<S, T>::value) {
    RayForm<S>::spans(a_shape, a_ray, a_t0, a_t1, a_options, a_firstOnly, a_spans);
  }
  else {
    if (a_firstOnly) {
      RayDetail::firstSpan(a_shape, a_ray, a_t0, a_t1, a_options, a_spans);
    }
    else {
      RayDetail::crossingSpans(a_shape, a_ray, a_t0, a_t1, a_options, a_spans);
    }
  }
}

/**
 * @brief The first crossing of a composition along a ray, from its lists: the first crossing in the list,
 * if it comes no later than the list's horizon; otherwise the trace starts again from the horizon.
 * @details Internal helper; not part of the public API. A_trace(t0, t1, allSpans, spans) writes the
 * composition's list over [t0, t1]; allSpans asks every operand for all its spans, which a trace that
 * gets no further (its horizon at its start) is repeated with. A query that still gets no further, or
 * starts again too many times, is unresolved.
 * @tparam T     Floating-point precision.
 * @tparam Trace Callable writing the list.
 * @param[in] a_trace The trace.
 * @param[in] a_ray   The ray.
 * @param[in] a_tMin  Start of the interval.
 * @param[in] a_tMax  End of the interval.
 * @return The crossing, a miss, or an unresolved query. HitT::m_primitive is the end's primitive.
 */
template <class T, class Trace>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
traceSpans(const Trace& a_trace, const RayT<T>& a_ray, const T a_tMin, const T a_tMax) noexcept
{
  constexpr uint32_t MaxPasses = 64;

  HitT<T> hit;

  if (!(a_tMin <= a_tMax)) {
    return hit;
  }

  T    t0  = a_tMin;
  bool all = false;

  RaySpansT<T> spans;

  for (uint32_t pass = 0; pass < MaxPasses; pass++) {
    a_trace(t0, a_tMax, all, spans);

    const T horizon = spans.m_horizon;

    for (uint32_t i = 0; i < spans.m_count; i++) {
      for (int side = 0; side < 2; side++) {
        const SpanEndT<T>& end = (side == 0) ? spans.m_spans[i].m_in : spans.m_spans[i].m_out;

        if (end.crossing() && end.m_t >= t0 && end.m_t <= horizon) {
          hit.m_status    = HitStatus::Hit;
          hit.m_t         = end.m_t;
          hit.m_point     = a_ray.at(end.m_t);
          hit.m_primitive = end.m_primitive;
          hit.m_normal    = end.m_normal;
          hit.m_entering  = (side == 0);
          hit.m_exact     = (end.m_kind == SpanEndT<T>::Exact);

          return hit;
        }
      }
    }

    // No crossing up to the horizon: none at all if that is the end, otherwise start again from it.
    if (!(horizon < a_tMax)) {
      return hit;
    }

    if (horizon > t0) {
      t0  = horizon;
      all = false;
    }
    else if (!all) {
      all = true;
    }
    else {
      break;
    }
  }

  hit.m_status = HitStatus::Unresolved;
  hit.m_t      = t0;
  hit.m_point  = a_ray.at(t0);

  return hit;
}

} // namespace RayDetail

} // namespace EBGeometry

#endif
