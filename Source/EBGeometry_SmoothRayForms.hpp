// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SmoothRayForms.hpp
 * @brief  Ray forms of smooth unions and intersections: marched only within a bracket traced exactly.
 * @details A smooth union deepens its operands' minimum by at most d (s/4 for the polynomial blend, s ln 2 for
 * the exponential one), so it lies within the union of its operands' offsets by d, its bracket: outside it,
 * every operand's value exceeds d and the blend's exceeds 0. A smooth intersection only raises its operands'
 * maximum, so it lies within their plain intersection. A ray traces the bracket (an operand's offset from its
 * level sets, see EBGeometry_OffsetRayForms.hpp, through unions of such operands, and marched otherwise),
 * misses exactly where it misses the bracket, and marches the smooth operation only within the bracket's spans.
 * The fused SmoothUnion, SmoothIntersection and SmoothDifference trace this way always; a tape's ray program
 * only when the tape lists TapeFeature::SmoothBrackets.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SMOOTHRAYFORMS_HPP
#define EBGEOMETRY_SMOOTHRAYFORMS_HPP

// Std includes
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <utility>

// Our includes
#include "EBGeometry_CSGRayForms.hpp"
#include "EBGeometry_Fused.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_OffsetRayForms.hpp"
#include "EBGeometry_Ray.hpp"
#include "EBGeometry_RaySpans.hpp"

namespace EBGeometry {

namespace RayDetail {

/**
 * @brief How far a smooth union's blend can deepen its operands' minimum: s/4 for the polynomial blend, s ln 2
 * for the exponential one.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_smooth      Smoothing length.
 * @param[in] a_exponential Whether the blend is exponential.
 * @return The depth.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
blendDepth(const T a_smooth, const bool a_exponential) noexcept
{
  return a_exponential ? a_smooth * T(0.6931471805599453) : T(0.25) * a_smooth;
}

/**
 * @brief A shape seen only through its value, with no ray form: what a smooth operation is marched as within
 * its bracket, and an operand's offset where it has no closed-form level sets.
 * @details Internal helper; not part of the public API. Its value is the shape's less a level, and its box
 * the shape's grown by the level.
 * @tparam S Shape type.
 */
template <class S>
struct MarchedView
{
  /**
   * @brief Floating-point precision.
   */
  using T = ShapePrecision<S>;

  /**
   * @brief The quality of its value: the shape's.
   */
  static constexpr DistanceQuality distanceQuality = distanceQualityOf<S>;

  /**
   * @brief The shape.
   */
  S m_shape;

  /**
   * @brief The level subtracted from its value: 0 for the shape itself, d for its offset by d.
   */
  T m_level;

  /**
   * @brief The value: f - c.
   * @param[in] a_point The point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point) - m_level;
  }

  /**
   * @brief The shape's box, grown by the level.
   * @return The box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const BoundingVolumes::AABBT<T> box  = m_shape.computeBoundingVolume();
    const Vec3T<T>                  grow = Math::max(m_level, T(0)) * Vec3T<T>::ones();

    return BoundingVolumes::AABBT<T>(box.getLowCorner() - grow, box.getHighCorner() + grow);
  }
};

/**
 * @brief A marched view is called out of line on a device where its shape is.
 * @tparam S Shape type.
 */
template <class S>
struct IsComposition<MarchedView<S>> : std::bool_constant<CalledOutOfLine<S>>
{
};

/**
 * @brief Whether a shape's offsets are traced from closed-form level sets: a shape with a LevelForm, or a fused
 * union of such shapes (whose offset is the union of theirs).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S>
struct TracedOffset : HasLevelForm<S>
{
};

/**
 * @brief A fused union's offset is traced if every operand's is.
 * @tparam Cs Operand types.
 */
template <class... Cs>
struct TracedOffset<Fused::Node<Fused::UnionOp, Cs...>> : std::bool_constant<(TracedOffset<Cs>::value && ...)>
{
};

template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline void
offsetSpans(const S&             a_shape,
            const T              a_level,
            const RayT<T>&       a_ray,
            const T              a_t0,
            const T              a_t1,
            const RayOptions<T>& a_options,
            const bool           a_firstOnly,
            RaySpansT<T>&        a_spans) noexcept;

/**
 * @brief Unite the offsets of a fused node's operands I, I + 1, ... into a list.
 * @details Internal helper; not part of the public API. Each operand is traced only up to what the list so far
 * is known up to.
 * @tparam I  First operand.
 * @tparam T  Floating-point precision.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 * @param[in]     a_node      The node.
 * @param[in]     a_level     The offset.
 * @param[in]     a_ray       The ray.
 * @param[in]     a_t0        Start of the interval.
 * @param[in]     a_options   How to march.
 * @param[in]     a_firstOnly Whether only the first crossing is needed.
 * @param[in,out] a_spans     The list so far.
 * @param[in,out] a_operand   Scratch for an operand's list.
 */
template <size_t I, class T, class Op, class... Cs>
EBGEOMETRY_HOST_DEVICE
inline void
uniteOffsets(const Fused::Node<Op, Cs...>& a_node,
             const T                       a_level,
             const RayT<T>&                a_ray,
             const T                       a_t0,
             const RayOptions<T>&          a_options,
             const bool                    a_firstOnly,
             RaySpansT<T>&                 a_spans,
             RaySpansT<T>&                 a_operand) noexcept
{
  if constexpr (I < sizeof...(Cs)) {
    RayDetail::offsetSpans(
      RayDetail::fusedOperand<I>(a_node), a_level, a_ray, a_t0, a_spans.m_horizon, a_options, a_firstOnly, a_operand);
    RayDetail::uniteInto(a_spans, a_operand);

    if (a_firstOnly) {
      RayDetail::keepFirst(a_spans);
    }

    RayDetail::uniteOffsets<I + 1>(a_node, a_level, a_ray, a_t0, a_options, a_firstOnly, a_spans, a_operand);
  }
}

/**
 * @brief Where a ray is inside a shape's offset {f <= c} over an interval: from its closed-form level sets
 * where it has them (see TracedOffset), and marched otherwise. Its crossings are named leaf 0.
 * @details Internal helper; not part of the public API. A fused union's offset is the union of its operands'.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]  a_shape     The shape.
 * @param[in]  a_level     The offset c.
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
offsetSpans(const S&             a_shape,
            const T              a_level,
            const RayT<T>&       a_ray,
            const T              a_t0,
            const T              a_t1,
            const RayOptions<T>& a_options,
            const bool           a_firstOnly,
            RaySpansT<T>&        a_spans) noexcept
{
  if constexpr (HasLevelForm<S>::value) {
    RayDetail::levelSpans(LevelView<S>{a_shape, a_level, false}, a_ray, a_t0, a_t1, a_options, a_firstOnly, a_spans);
  }
  else if constexpr (TracedOffset<S>::value) {
    RaySpansT<T> operand;

    RayDetail::offsetSpans(
      RayDetail::fusedOperand<0>(a_shape), a_level, a_ray, a_t0, a_t1, a_options, a_firstOnly, a_spans);

    if (a_firstOnly) {
      RayDetail::keepFirst(a_spans);
    }

    RayDetail::uniteOffsets<1>(a_shape, a_level, a_ray, a_t0, a_options, a_firstOnly, a_spans, operand);
  }
  else {
    RayDetail::shapeSpans(MarchedView<S>{a_shape, a_level}, a_ray, a_t0, a_t1, a_options, a_firstOnly, a_spans);
  }

  RayDetail::nameLeaf(0U, a_spans);
}

/**
 * @brief How costly a shape's offset is to trace over a whole ray: 0 from convex level sets, 1 marched, 2 from a
 * mesh's level sets (its offset faces walked along the ray).
 * @details Internal helper; not part of the public API. Of two offsets intersected, the cheaper is traced first
 * and the other only within its spans.
 * @tparam S Shape type.
 */
template <class S>
struct OffsetCost
{
  /**
   * @brief The cost.
   */
  static constexpr int value = IsMeshLevel<S>::value ? 2 : (TracedOffset<S>::value ? 0 : 1);
};

/**
 * @brief The largest of some shapes' offset costs (see OffsetCost).
 * @details Internal helper; not part of the public API.
 * @tparam Cs Shape types.
 * @return The cost.
 */
template <class... Cs>
[[nodiscard]] constexpr int
costliestOffset() noexcept
{
  int cost = 0;

  ((cost = Math::max(cost, OffsetCost<Cs>::value)), ...);

  return cost;
}

/**
 * @brief A fused union's offset costs what its costliest operand's does, if traced.
 * @tparam Cs Operand types.
 */
template <class... Cs>
struct OffsetCost<Fused::Node<Fused::UnionOp, Cs...>>
{
  /**
   * @brief The cost.
   */
  static constexpr int value = TracedOffset<Fused::Node<Fused::UnionOp, Cs...>>::value ? costliestOffset<Cs...>() : 1;
};

/**
 * @brief Where a ray is inside a shape's offset within another list's spans: the offset traced over each of
 * them in turn (offsetSpans()), all its spans there.
 * @details Internal helper; not part of the public API. Stops where a trace is known no further.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]  a_shape   The shape.
 * @param[in]  a_level   The offset.
 * @param[in]  a_ray     The ray.
 * @param[in]  a_within  The other list.
 * @param[in]  a_options How to march.
 * @param[out] a_piece   Scratch for one span's trace.
 * @param[out] a_spans   The list; not the other list.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline void
offsetSpansWithin(const S&             a_shape,
                  const T              a_level,
                  const RayT<T>&       a_ray,
                  const RaySpansT<T>&  a_within,
                  const RayOptions<T>& a_options,
                  RaySpansT<T>&        a_piece,
                  RaySpansT<T>&        a_spans) noexcept
{
  a_spans.reset(a_within.m_horizon);

  for (uint32_t i = 0; i < a_within.m_count && a_within.m_spans[i].m_in.m_t <= a_within.m_horizon; i++) {
    const T t0 = a_within.m_spans[i].m_in.m_t;
    const T t1 = Math::min(a_within.m_spans[i].m_out.m_t, a_within.m_horizon);

    RayDetail::offsetSpans(a_shape, a_level, a_ray, t0, t1, a_options, false, a_piece);

    for (uint32_t k = 0; k < a_piece.m_count && a_piece.m_spans[k].m_in.m_t <= a_piece.m_horizon; k++) {
      const typename RaySpansT<T>::Span span = RayDetail::cut(a_piece.m_spans[k], a_piece.m_horizon);

      a_spans.add(span.m_in, span.m_out);
    }

    if (a_piece.m_horizon < t1) {
      a_spans.m_horizon = Math::min(a_spans.m_horizon, a_piece.m_horizon);

      return;
    }
  }
}

/**
 * @brief Unite into a list where two of a fused node's operands, I and each of J, J + 1, ..., may both be within
 * a_level of their surfaces: the cheaper one's offset by a_level (see OffsetCost), intersected with the other's
 * only if that is traced from convex level sets too. A mesh's offset is not traced there: walking its faces
 * grown by the level along the ray costs more than marching the smooth operation where the cheaper offset is.
 * @details Internal helper; not part of the public API. All spans, over [a_t0, a_t1].
 * @tparam I  First operand of the pairs.
 * @tparam J  Second operand of the first pair.
 * @tparam T  Floating-point precision.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 * @param[in]     a_node    The node.
 * @param[in]     a_level   The offset.
 * @param[in]     a_ray     The ray.
 * @param[in]     a_t0      Start of the interval.
 * @param[in]     a_t1      End of the interval.
 * @param[in]     a_options How to march.
 * @param[in,out] a_zone    The list so far.
 * @param[in,out] a_pair    Scratch for a pair's list.
 * @param[in,out] a_first   Scratch for the first offset's list.
 * @param[in,out] a_piece   Scratch for one span's trace.
 */
template <size_t I, size_t J, class T, class Op, class... Cs>
EBGEOMETRY_HOST_DEVICE
inline void
uniteBlendZones(const Fused::Node<Op, Cs...>& a_node,
                const T                       a_level,
                const RayT<T>&                a_ray,
                const T                       a_t0,
                const T                       a_t1,
                const RayOptions<T>&          a_options,
                RaySpansT<T>&                 a_zone,
                RaySpansT<T>&                 a_pair,
                RaySpansT<T>&                 a_first,
                RaySpansT<T>&                 a_piece) noexcept
{
  if constexpr (J < sizeof...(Cs)) {
    using A = typename Fused::Detail::TypeAt<I, Cs...>::type;
    using B = typename Fused::Detail::TypeAt<J, Cs...>::type;

    constexpr bool   swap   = OffsetCost<B>::value < OffsetCost<A>::value;
    constexpr size_t first  = swap ? J : I;
    constexpr size_t second = swap ? I : J;

    using Second = typename Fused::Detail::TypeAt<second, Cs...>::type;

    if constexpr (OffsetCost<Second>::value == 0) {
      RayDetail::offsetSpans(
        RayDetail::fusedOperand<first>(a_node), a_level, a_ray, a_t0, a_t1, a_options, false, a_first);
      RayDetail::offsetSpansWithin(
        RayDetail::fusedOperand<second>(a_node), a_level, a_ray, a_first, a_options, a_piece, a_pair);
    }
    else {
      RayDetail::offsetSpans(
        RayDetail::fusedOperand<first>(a_node), a_level, a_ray, a_t0, a_t1, a_options, false, a_pair);
    }

    RayDetail::uniteInto(a_zone, a_pair);

    RayDetail::uniteBlendZones<I, J + 1>(
      a_node, a_level, a_ray, a_t0, a_t1, a_options, a_zone, a_pair, a_first, a_piece);
  }
  else if constexpr (I + 2 < sizeof...(Cs)) {
    RayDetail::uniteBlendZones<I + 1, I + 2>(
      a_node, a_level, a_ray, a_t0, a_t1, a_options, a_zone, a_pair, a_first, a_piece);
  }
}

} // namespace RayDetail

/**
 * @brief The ray form of a fused smooth union, intersection or difference (an intersection with a complement):
 * marched within its bracket, which is traced exactly where its operands allow. Callable on a device.
 * @details The bracket of a smooth union is the union of its operands' offsets by RayDetail::blendDepth(), that
 * of a smooth intersection the plain intersection of its operands (see EBGeometry_SmoothRayForms.hpp). The
 * operation is marched over each of the bracket's spans in turn (RayDetail::spansWithin()), so a crossing is
 * found to within RayOptions::m_tolerance (HitT::m_exact is false), and a ray that misses the bracket misses
 * exactly. HitT::m_primitive is 0: a fused composition is one shape.
 * @tparam T       Floating-point precision.
 * @tparam IsUnion Whether this is a union.
 * @tparam Cs      Operand types.
 */
template <class T, bool IsUnion, class... Cs>
struct RayForm<Fused::Node<Fused::SmoothCombineOp<T, IsUnion>, Cs...>>
{
  /**
   * @brief The node type.
   */
  using Node = Fused::Node<Fused::SmoothCombineOp<T, IsUnion>, Cs...>;

  /**
   * @brief The first crossing along a ray.
   * @param[in] a_shape   The node.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march.
   * @return The crossing, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const Node& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept
  {
    const auto trace =
      [&a_shape, &a_ray, &a_options](const T a_t0, const T a_t1, const bool a_all, RaySpansT<T>& a_spans) noexcept {
        spans(a_shape, a_ray, a_t0, a_t1, a_options, !a_all, a_spans);
      };

    // A marched crossing's normal is the gradient there; marching again around it would find no better one.
    const auto resolve = [&a_shape, &a_ray, &a_options](const T a_t, const uint32_t) noexcept {
      const auto box = a_shape.computeBoundingVolume();
      const T    h   = a_options.m_tolerance * RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);

      HitT<T> hit;

      hit.m_normal = RayDetail::gradientNormal(RayDetail::MarchedView<Node>{a_shape, T(0)}, a_ray.at(a_t), h);

      return hit;
    };

    HitT<T> hit = RayDetail::traceSpans(trace, resolve, a_ray, a_tMin, a_tMax);

    hit.m_primitive = 0U;

    return hit;
  }

  /**
   * @brief Where a ray is inside over an interval: the operation marched within the bracket's spans.
   * @param[in]  a_shape     The node.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   How to march.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const Node&          a_shape,
        const RayT<T>&       a_ray,
        T                    a_tMin,
        T                    a_tMax,
        const RayOptions<T>& a_options,
        bool                 a_firstOnly,
        RaySpansT<T>&        a_spans) noexcept
  {
    RaySpansT<T> scratch;
    RaySpansT<T> operand;

    if constexpr (IsUnion) {
      const Fused::SmoothCombineOp<T, IsUnion>& op = a_shape.layout().m_op;

      if (op.m_exponential) {
        // An exponential blend differs from the plain union everywhere, if only slightly: marched whole.
        RayDetail::shapeSpans(
          RayDetail::MarchedView<Node>{a_shape, T(0)}, a_ray, a_tMin, a_tMax, a_options, a_firstOnly, a_spans);
      }
      else {
        // The plain union, exact, then the blend zones up to what it is known up to: where two operands are both
        // within s + s/4 of their surfaces. Outside them the smooth union is inside exactly where the plain union
        // is; within them it is marched, and contains the plain union there too.
        const T level = op.m_smooth + RayDetail::blendDepth(op.m_smooth, false);

        RaySpansT<T> zone;
        RaySpansT<T> first;

        RayDetail::fusedOperandSpans<0>(a_shape, a_ray, a_tMin, a_tMax, a_options, a_firstOnly, a_spans);

        if (a_firstOnly) {
          RayDetail::keepFirst(a_spans);
        }

        RayDetail::fusedUnite<1>(a_shape, a_ray, a_tMin, a_options, a_firstOnly, a_spans, operand);

        zone.reset(a_spans.m_horizon);

        RayDetail::uniteBlendZones<0, 1>(
          a_shape, level, a_ray, a_tMin, a_spans.m_horizon, a_options, zone, operand, first, scratch);
        RayDetail::spansWithin(
          RayDetail::MarchedView<Node>{a_shape, T(0)}, a_ray, zone, a_options, a_firstOnly, scratch, operand);
        RayDetail::uniteInto(a_spans, operand);

        if (a_firstOnly) {
          RayDetail::keepFirst(a_spans);
        }
      }
    }
    else {
      // Marched within the plain intersection, as a fused intersection traces it: the traced operands first, the
      // marched ones only within them. All its spans: where the smooth intersection is outside in one, it may be
      // inside in the next.
      constexpr size_t first = RayDetail::FirstTraced<T, Cs...>::value;

      RaySpansT<T> bracket;

      RayDetail::fusedOperandSpans<first>(a_shape, a_ray, a_tMin, a_tMax, a_options, false, bracket);
      RayDetail::fusedIntersect<0, false, first>(a_shape, a_ray, a_tMin, a_options, false, bracket, operand, scratch);
      RayDetail::fusedIntersect<0, true, first>(a_shape, a_ray, a_tMin, a_options, false, bracket, operand, scratch);
      RayDetail::spansWithin(
        RayDetail::MarchedView<Node>{a_shape, T(0)}, a_ray, bracket, a_options, a_firstOnly, scratch, a_spans);
    }

    RayDetail::nameLeaf(0U, a_spans);
  }
};

} // namespace EBGeometry

#endif
