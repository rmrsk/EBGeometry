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

// A span list is left uninitialised past m_count (zeroing it costs local-memory stores on a device), and GCC
// cannot see that only the first m_count spans are ever read.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

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
  return SpanEndT<T>{a_t, 0U, SpanEndT<T>::Open};
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
  return SpanEndT<T>{a_hit.m_t, 0U, a_hit.m_exact ? SpanEndT<T>::Exact : SpanEndT<T>::Marched};
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
 * @brief Where a list is known to be empty from a point on: the point itself if the ray is inside there,
 * and otherwise the start of the next span, or the list's horizon.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_spans The list.
 * @param[in] a_t     The point, no later than the list's horizon.
 * @return Where the list stops being known empty.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
emptyUntil(const RaySpansT<T>& a_spans, const T a_t) noexcept
{
  for (uint32_t i = 0; i < a_spans.m_count; i++) {
    const typename RaySpansT<T>::Span& span = a_spans.m_spans[i];

    if (span.m_in.m_t > a_t) {
      return Math::min(span.m_in.m_t, a_spans.m_horizon);
    }

    if (span.m_out.m_t > a_t || span.m_in.m_t == a_t) {
      return a_t;
    }
  }

  return a_spans.m_horizon;
}

/**
 * @brief How far the intersection of two lists is known: up to the smaller horizon, and beyond it as far as
 * the other list is empty from there, since the intersection is empty wherever either is.
 * @details Internal helper; not part of the public API. Where a cut removes a part, a trace that found
 * nothing in front of the cut starts again past it, not from each crossing of the part inside it.
 * @tparam T Floating-point precision.
 * @param[in] a_a One list.
 * @param[in] a_b The other.
 * @return The intersection's horizon.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
intersectionHorizon(const RaySpansT<T>& a_a, const RaySpansT<T>& a_b) noexcept
{
  if (a_a.m_horizon < a_b.m_horizon) {
    return Math::max(a_a.m_horizon, RayDetail::emptyUntil(a_b, a_a.m_horizon));
  }

  if (a_b.m_horizon < a_a.m_horizon) {
    return Math::max(a_b.m_horizon, RayDetail::emptyUntil(a_a, a_b.m_horizon));
  }

  return a_a.m_horizon;
}

/**
 * @brief The intersection of two lists: where the ray is inside both.
 * @details Internal helper; not part of the public API. Its spans end at the smaller horizon, and it is
 * complete as far as intersectionHorizon() says. Where two ends meet, the crossing wins, since the ray
 * crosses the other operand's surface there.
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

  a_out.reset(RayDetail::intersectionHorizon(a_a, a_b));

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
 * @brief Unite a list into another, in place: each of the operand's spans merged with the spans it meets,
 * or inserted between them.
 * @details Internal helper; not part of the public API. Complete up to the smaller horizon. No list is
 * copied: on a device every span copied is local memory traffic, which is most of what tracing costs there.
 * @tparam T Floating-point precision.
 * @param[in,out] a_spans   The list.
 * @param[in]     a_operand The list to unite into it.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
uniteInto(RaySpansT<T>& a_spans, const RaySpansT<T>& a_operand) noexcept
{
  const T horizon = Math::min(a_spans.m_horizon, a_operand.m_horizon);

  if (horizon < a_spans.m_horizon) {
    RayDetail::cutTo(horizon, a_spans);
  }

  // Each of the operand's spans in turn: merged with the list's spans it meets, or inserted between them.
  for (uint32_t k = 0; k < a_operand.m_count && a_operand.m_spans[k].m_in.m_t <= horizon; k++) {
    typename RaySpansT<T>::Span span = RayDetail::cut(a_operand.m_spans[k], horizon);

    uint32_t first = 0;

    while (first < a_spans.m_count && a_spans.m_spans[first].m_out.m_t < span.m_in.m_t) {
      first++;
    }

    // Where two ends meet, the one that is no crossing wins: the ray is inside the other span there.
    uint32_t last = first;

    while (last < a_spans.m_count && a_spans.m_spans[last].m_in.m_t <= span.m_out.m_t) {
      const typename RaySpansT<T>::Span& other = a_spans.m_spans[last];

      if (other.m_in.m_t < span.m_in.m_t || (other.m_in.m_t == span.m_in.m_t && !other.m_in.crossing())) {
        span.m_in = other.m_in;
      }
      if (other.m_out.m_t > span.m_out.m_t || (other.m_out.m_t == span.m_out.m_t && !other.m_out.crossing())) {
        span.m_out = other.m_out;
      }

      last++;
    }

    const uint32_t merged = last - first;

    if (merged == 0U) {
      // A new span: a full list keeps its first spans, and is known only up to where the dropped one begins.
      if (a_spans.m_count == RaySpansT<T>::Capacity) {
        if (first == a_spans.m_count) {
          a_spans.m_horizon = Math::min(a_spans.m_horizon, span.m_in.m_t);

          break;
        }

        a_spans.m_horizon = Math::min(a_spans.m_horizon, a_spans.m_spans[a_spans.m_count - 1U].m_in.m_t);
        a_spans.m_count--;
      }

      for (uint32_t m = a_spans.m_count; m > first; m--) {
        a_spans.m_spans[m] = a_spans.m_spans[m - 1U];
      }

      a_spans.m_count++;
    }
    else if (merged > 1U) {
      for (uint32_t m = last; m < a_spans.m_count; m++) {
        a_spans.m_spans[m - merged + 1U] = a_spans.m_spans[m];
      }

      a_spans.m_count -= merged - 1U;
    }

    a_spans.m_spans[first] = span;
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
    a_spans.reset(RayDetail::intersectionHorizon(a_spans, a_operand));

    return;
  }

  RayDetail::intersect(a_spans, a_operand, a_scratch);
  RayDetail::copySpans(a_scratch, a_spans);
}

/**
 * @brief The complement of a list over [a_t0, a_t1]: where the ray is outside. A crossing is the same
 * crossing seen from the other side; its normal, worked out at the end, is reversed then.
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
      a_out.add(start, span.m_in);
    }

    // A span that ends where nothing more is known leaves no gap after it.
    gap = span.m_out.crossing();

    if (!gap) {
      break;
    }

    start = span.m_out;
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
 * the scale.
 * @details Internal helper; not part of the public API. The ends of the interval map back to exactly the
 * ends of the original one, so that a list complete over the mapped interval is complete over the original,
 * and every distance stays inside it.
 * @tparam T Floating-point precision.
 * @param[in]     a_mapped The mapped ray.
 * @param[in]     a_t0     Start of the original interval.
 * @param[in]     a_t1     End of the original interval.
 * @param[in,out] a_spans  The operand's list, mapped in place.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
mapBack(const MappedRay<T>& a_mapped, const T a_t0, const T a_t1, RaySpansT<T>& a_spans) noexcept
{
  const auto distance = [&a_mapped, a_t0, a_t1](const T a_t) noexcept -> T {
    if (a_t <= a_mapped.m_t0) {
      return a_t0;
    }
    if (a_t >= a_mapped.m_t1) {
      return a_t1;
    }

    // Rounded, the quotient can land just outside the original interval.
    return Math::clamp(a_t / a_mapped.m_scale, a_t0, a_t1);
  };

  a_spans.m_horizon = distance(a_spans.m_horizon);

  for (uint32_t i = 0; i < a_spans.m_count; i++) {
    a_spans.m_spans[i].m_in.m_t  = distance(a_spans.m_spans[i].m_in.m_t);
    a_spans.m_spans[i].m_out.m_t = distance(a_spans.m_spans[i].m_out.m_t);
  }
}

/**
 * @brief A normal of the operand of an affine map, mapped back to the original frame: by the transposed
 * matrix, and normalised.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_map    The map's coefficients.
 * @param[in] a_normal The operand's normal.
 * @return The normal.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
mapNormalBack(const T* a_map, const Vec3T<T>& a_normal) noexcept
{
  const T*        m = a_map;
  const Vec3T<T>& n = a_normal;
  const Vec3T<T>  g(m[0] * n[0] + m[3] * n[1] + m[6] * n[2],
                   m[1] * n[0] + m[4] * n[1] + m[7] * n[2],
                   m[2] * n[0] + m[5] * n[1] + m[8] * n[2]);
  const T         length = g.length();

  return (length > T(0)) ? g / length : g;
}

/**
 * @brief Name the leaf every crossing of a list lies on.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in]     a_leaf  The leaf.
 * @param[in,out] a_spans The list.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
nameLeaf(const uint32_t a_leaf, RaySpansT<T>& a_spans) noexcept
{
  for (uint32_t i = 0; i < a_spans.m_count; i++) {
    a_spans.m_spans[i].m_in.m_leaf  = a_leaf;
    a_spans.m_spans[i].m_out.m_leaf = a_leaf;
  }
}

/**
 * @brief Shift the leaves every crossing of a list names by an offset: an operand's own leaves, numbered
 * among its composition's.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in]     a_offset The operand's first leaf.
 * @param[in,out] a_spans  The list.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
inline void
offsetLeaves(const uint32_t a_offset, RaySpansT<T>& a_spans) noexcept
{
  for (uint32_t i = 0; i < a_spans.m_count; i++) {
    a_spans.m_spans[i].m_in.m_leaf += a_offset;
    a_spans.m_spans[i].m_out.m_leaf += a_offset;
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

  return RayDetail::valueAt(a_shape, a_point) < T(0);
}

/**
 * @brief A shape's first crossing along a ray (firstHit()).
 * @details Internal helper; not part of the public API. What the span code and leafHit() call on an
 * operand or a leaf: out of line on a device for every shape, so that a kernel holds one copy of each
 * shape's trace (see CalledOutOfLine).
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape   The shape.
 * @param[in] a_ray     The ray, with a unit direction.
 * @param[in] a_t0      Start of the interval.
 * @param[in] a_t1      End of the interval.
 * @param[in] a_options How to march.
 * @return The crossing, a miss, or an unresolved query.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline HitT<T>
shapeHit(const S& a_shape, const RayT<T>& a_ray, const T a_t0, const T a_t1, const RayOptions<T>& a_options) noexcept
{
  return EBGeometry::firstHit(a_shape, a_ray, a_t0, a_t1, a_options);
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

  const HitT<T> hit = RayDetail::shapeHit(a_shape, a_ray, a_t0, a_t1, a_options);

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
    const HitT<T> hit = RayDetail::shapeHit(a_shape, a_ray, t, a_t1, a_options);

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
 * @brief Where a shape's value changes sign along a ray, searched one way from a point: sphere tracing by
 * the value, which must be a distance or a bound on one, then bisection.
 * @details Internal helper; not part of the public API. What annularSpans() marches between its operand's
 * crossings: no normal and no primitive, which the crossing the trace ends at is given later. Out of line
 * on a device (see valueAt()).
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]     a_shape  The shape.
 * @param[in]     a_ray    The ray, with a unit direction.
 * @param[in]     a_from   Where the search starts.
 * @param[in]     a_to     Where it ends, on either side of a_from: the search runs from a_from towards it.
 * @param[in]     a_inside Whether the value is negative at a_from.
 * @param[in]     a_eps    How close to zero a value counts as the crossing, and how short the bisection ends.
 * @param[in,out] a_steps  Values the search may still ask for; what it asks for is taken off.
 * @return A hit at the sign change (HitT::m_t only), a miss if there is none before a_to, or an unresolved
 * query where the steps ran out.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline HitT<T>
signChange(const S&       a_shape,
           const RayT<T>& a_ray,
           const T        a_from,
           const T        a_to,
           const bool     a_inside,
           const T        a_eps,
           uint32_t&      a_steps) noexcept
{
  const T direction = (a_to < a_from) ? T(-1) : T(1);
  const T length    = std::abs(a_to - a_from);

  const auto value = [&a_shape, &a_ray, a_from, direction](const T a_s) noexcept -> T {
    return RayDetail::valueAt(a_shape, a_ray.at(a_from + direction * a_s));
  };

  HitT<T> hit;

  // Distances from a_from: the sign changes between sPrev and s once bracketed. A value within a_eps of zero
  // is the surface only once the search has been further from one: a search starting on a surface (a trace
  // starting again from a crossing) steps a_eps at a time until it is.
  T    s         = T(0);
  T    sPrev     = T(0);
  bool bracketed = false;
  bool away      = false;

  while (!bracketed) {
    if (a_steps == 0U) {
      hit.m_status = HitStatus::Unresolved;
      hit.m_t      = a_from + direction * s;

      return hit;
    }

    a_steps--;

    const T v = value(s);

    if ((v < T(0)) != a_inside) {
      bracketed = true;
    }
    else if (std::abs(v) < a_eps && away) {
      break;
    }
    else {
      away  = away || std::abs(v) >= a_eps;
      sPrev = s;
      s += Math::max(std::abs(v), a_eps);

      if (s > length) {
        return hit;
      }
    }
  }

  if (bracketed) {
    T a = sPrev;
    T b = s;

    for (int i = 0; i < 64 && b - a > a_eps; i++) {
      const T mid = T(0.5) * (a + b);

      if ((value(mid) < T(0)) == a_inside) {
        a = mid;
      }
      else {
        b = mid;
      }
    }

    s = T(0.5) * (a + b);
  }

  hit.m_status = HitStatus::Hit;
  hit.m_t      = a_from + direction * s;

  return hit;
}

/**
 * @brief The list of an annulus |d| - delta (a shell around its operand's surface) over an interval, up to
 * its first crossing, from its operand's list.
 * @details Internal helper; not part of the public API. Each of the operand's crossings is inside the shell
 * (the value there is -delta), and between two of them d keeps its sign, so the shell's value is the
 * distance to one offset surface there (or a bound on it). The shell is marched from the start towards the
 * first crossing, then from each crossing towards the next: from outside, the march must find the shell
 * before the crossing; from a crossing, a step at normal incidence lands on the surface. The list ends at
 * the first crossing found, its horizon there, whether or not more is needed: the trace starts again from it
 * if it needs more (see traceSpans()), so that an annulus under an intersection only finds the crossings
 * the trace reaches. The value must be a distance or a bound on one.
 * @tparam T Floating-point precision.
 * @tparam S Shape type: the annulus.
 * @param[in]  a_shell   The annulus, whose value is |d| - delta.
 * @param[in]  a_operand Its operand's list over the interval, all its spans.
 * @param[in]  a_ray     The ray, with a unit direction.
 * @param[in]  a_t0      Start of the interval.
 * @param[in]  a_t1      End of the interval.
 * @param[in]  a_options How to march.
 * @param[out] a_spans   The list; not a_operand.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline void
annularSpans(const S&             a_shell,
             const RaySpansT<T>&  a_operand,
             const RayT<T>&       a_ray,
             const T              a_t0,
             const T              a_t1,
             const RayOptions<T>& a_options,
             RaySpansT<T>&        a_spans) noexcept
{
  const T horizon = Math::max(a_t0, Math::min(a_t1, a_operand.m_horizon));

  a_spans.reset(horizon);

  // Outside the box, the ray is outside the shell.
  const auto box   = a_shell.computeBoundingVolume();
  const T    scale = RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);
  const T    eps   = a_options.m_tolerance * scale;

  const Vec3T<T> pad = eps * Vec3T<T>::ones();

  T a  = a_t0;
  T b1 = horizon;

  if (!RayDetail::clip(box.getLowCorner() - pad, box.getHighCorner() + pad, a_ray, a, b1)) {
    return;
  }

  // Inside at the start, the list begins with a span open there. On the shell's surface (a trace starting
  // again from a crossing), which side the ray goes on to is decided a little past it, beyond the march's
  // tolerance, where the walk starts.
  bool inside = false;

  if (a <= a_t0) {
    T v = RayDetail::valueAt(a_shell, a_ray.at(a));

    if (std::abs(v) < eps) {
      a = Math::min(a + T(2) * eps, b1);
      v = RayDetail::valueAt(a_shell, a_ray.at(a));
    }

    inside = v < T(0);
  }

  CrossingPairer<T> pairer;

  if (inside) {
    pairer.m_start  = openAt(a_t0);
    pairer.m_inside = true;
    pairer.m_first  = false;
  }

  uint32_t steps = a_options.m_maxSteps;
  uint32_t end   = 0;

  // From each point (the start, then each of the operand's crossings) to the next crossing, or to the end.
  while (true) {
    T    c      = b1;
    bool isSeed = false;

    for (; end < 2U * a_operand.m_count && !isSeed; end++) {
      const auto&        span = a_operand.m_spans[end / 2U];
      const SpanEndT<T>& e    = (end % 2U == 0U) ? span.m_in : span.m_out;

      if (e.crossing() && e.m_t > a && e.m_t < b1) {
        c      = e.m_t;
        isSeed = true;
      }
    }

    const HitT<T> hit = RayDetail::signChange(a_shell, a_ray, a, c, inside, eps, steps);

    if (hit.m_status == HitStatus::Unresolved) {
      pairer.finish(hit.m_t, a_spans);

      a_spans.m_horizon = Math::min(a_spans.m_horizon, hit.m_t);

      return;
    }

    if (hit.hit()) {
      pairer.take(SpanEndT<T>{hit.m_t, 0U, SpanEndT<T>::Marched}, !inside, a_t0, a_spans);
      pairer.finish(hit.m_t, a_spans);

      a_spans.m_horizon = hit.m_t;

      return;
    }

    if (!isSeed) {
      break;
    }

    a      = c;
    inside = true;
  }

  pairer.finish(horizon, a_spans);
}

/**
 * @brief A shape's crossing at a distance along a ray, with its normal and primitive: what a composition
 * asks the leaf its trace ended on.
 * @details Internal helper; not part of the public API. Calls the shape's firstHit() over a window of
 * RayOptions::m_tolerance times its scale around a_t, and takes the crossing nearest a_t. Where rounding
 * leaves the window empty, the normal is the shape's gradient.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_shape   The shape.
 * @param[in] a_ray     The ray.
 * @param[in] a_t       The crossing's distance.
 * @param[in] a_options How to march.
 * @return The crossing (its m_normal, m_primitive and m_exact).
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
leafHit(const S& a_shape, const RayT<T>& a_ray, const T a_t, const RayOptions<T>& a_options) noexcept
{
  const auto box   = a_shape.computeBoundingVolume();
  const T    delta = a_options.m_tolerance * RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);

  HitT<T> nearest;

  T from = a_t - delta;

  // At most three crossings in the window: a ray grazing an edge may cross two faces there.
  for (int k = 0; k < 3; k++) {
    const HitT<T> hit = RayDetail::shapeHit(a_shape, a_ray, from, a_t + delta, a_options);

    if (!hit.hit()) {
      break;
    }

    if (!nearest.hit() || std::abs(hit.m_t - a_t) < std::abs(nearest.m_t - a_t)) {
      nearest = hit;
    }

    if (hit.m_t >= a_t) {
      break;
    }

    from = hit.m_t + T(1e-3) * delta;
  }

  if (!nearest.hit()) {
    const Vec3T<T> point = a_ray.at(a_t);

    nearest.m_normal    = RayDetail::gradientNormal(a_shape, point, delta);
    nearest.m_primitive = RayDetail::primitiveOf(a_shape, point);
  }

  return nearest;
}

/**
 * @brief The first crossing of a composition along a ray, from its lists: the first crossing in the list,
 * if it comes no later than the list's horizon; otherwise the trace starts again from the horizon.
 * @details Internal helper; not part of the public API. A_trace(t0, t1, allSpans, spans) writes the
 * composition's list over [t0, t1]; allSpans asks every operand for all its spans, which a trace that
 * gets no further (its horizon at its start) is repeated with. A query that still gets no further, or
 * starts again too many times, is unresolved. A_resolve(t, leaf) then gives the crossing's normal and
 * primitive, from the leaf its end names.
 * @tparam T       Floating-point precision.
 * @tparam Trace   Callable writing the list.
 * @tparam Resolve Callable giving the crossing on a leaf.
 * @param[in] a_trace   The trace.
 * @param[in] a_resolve The crossing on a leaf.
 * @param[in] a_ray     The ray.
 * @param[in] a_tMin    Start of the interval.
 * @param[in] a_tMax    End of the interval.
 * @return The crossing, a miss, or an unresolved query.
 */
template <class T, class Trace, class Resolve>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
traceSpans(
  const Trace& a_trace, const Resolve& a_resolve, const RayT<T>& a_ray, const T a_tMin, const T a_tMax) noexcept
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
          const HitT<T> leaf = a_resolve(end.m_t, end.m_leaf);

          hit.m_status    = HitStatus::Hit;
          hit.m_t         = end.m_t;
          hit.m_point     = a_ray.at(end.m_t);
          hit.m_primitive = leaf.m_primitive;
          hit.m_normal    = leaf.m_normal;
          hit.m_entering  = (side == 0);
          hit.m_exact     = (end.m_kind == SpanEndT<T>::Exact);

          // The leaf's normal faces out of the leaf; the composition's faces the way the ray crosses. For a ray
          // along the surface the dot product is rounding noise, and either orientation may come out.
          if ((dot(hit.m_normal, a_ray.m_direction) < T(0)) != hit.m_entering) {
            hit.m_normal = -hit.m_normal;
          }

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

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

} // namespace EBGeometry

#endif
