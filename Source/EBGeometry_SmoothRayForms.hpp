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
 * @brief A fused smooth union restricted to where it can differ from the plain union of its operands: the value
 * max(u, m2 - w), u the smooth union's and m2 its second smallest operand's, w its blend length plus how far the
 * blend deepens (s + s/4). A polynomial blend of length s changes the minimum only where the two smallest
 * operands are within s of each other, and changes its sign only where the smallest is within s/4 of 0: both
 * within w. Where this value is negative, the smooth union is inside; everywhere else it is inside exactly where
 * the plain union is.
 * @details Internal helper; not part of the public API. A distance bound, as the maximum of two: far from either
 * of the two nearest operands its value is large, so a march of it takes long steps, even near the other.
 * @tparam T  Floating-point precision.
 * @tparam Cs Operand types.
 */
template <class T, class... Cs>
struct BlendZoneView
{
  /**
   * @brief The node type.
   */
  using Node = Fused::Node<Fused::SmoothCombineOp<T, true>, Cs...>;

  /**
   * @brief The quality of its value: a bound at best.
   */
  static constexpr DistanceQuality distanceQuality = distanceQualityOf<Node>;

  /**
   * @brief The smooth union.
   */
  Node m_node;

  /**
   * @brief Where two operands' boxes, grown by w, overlap: the blend zone lies within it.
   */
  BoundingVolumes::AABBT<T> m_box;

  /**
   * @brief The value.
   * @param[in] a_point The point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return this->value(a_point, std::index_sequence_for<Cs...>{});
  }

  /**
   * @brief The blend zone's box.
   * @return The box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_box;
  }

private:
  /**
   * @brief The value, from the two smallest operands.
   * @tparam Is Operand indices.
   * @param[in] a_point The point.
   * @return The value.
   */
  template <size_t... Is>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  value(const Vec3T<T>& a_point, std::index_sequence<Is...>) const noexcept
  {
    const T smooth = m_node.layout().m_op.m_smooth;

    CSGOps::Pair<T> pair = CSGOps::pairInit<T>(false);

    (CSGOps::pairInsert(pair, RayDetail::fusedOperand<Is>(m_node).signedDistance(a_point), false), ...);

    const T blended = CSGOps::pairBlend(pair, smooth, false, false);

    return Math::max(blended, pair.m_second - (smooth + RayDetail::blendDepth(smooth, false)));
  }
};

/**
 * @brief A blend zone view is called out of line on a device.
 * @tparam T  Floating-point precision.
 * @tparam Cs Operand types.
 */
template <class T, class... Cs>
struct IsComposition<BlendZoneView<T, Cs...>> : std::true_type
{
};

/**
 * @brief Where two of a fused node's operands' boxes, grown by a_grow, overlap: the box of each pair's overlap,
 * united into a_box (a_any says whether there is one yet).
 * @details Internal helper; not part of the public API.
 * @tparam T  Floating-point precision.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 * @tparam Is Operand indices.
 * @param[in]     a_node The node.
 * @param[in]     a_grow How much each box is grown.
 * @param[in,out] a_box  The box so far.
 * @param[in,out] a_any  Whether there is one.
 */
template <class T, class Op, class... Cs, size_t... Is>
EBGEOMETRY_HOST_DEVICE
inline void
pairOverlaps(const Fused::Node<Op, Cs...>& a_node,
             const T                       a_grow,
             BoundingVolumes::AABBT<T>&    a_box,
             bool&                         a_any,
             std::index_sequence<Is...>) noexcept
{
  const BoundingVolumes::AABBT<T> boxes[] = {RayDetail::fusedOperand<Is>(a_node).computeBoundingVolume()...};
  const Vec3T<T>                  grow    = a_grow * Vec3T<T>::ones();

  for (size_t i = 0; i < sizeof...(Is); i++) {
    for (size_t j = i + 1; j < sizeof...(Is); j++) {
      const Vec3T<T> lo = max(boxes[i].getLowCorner(), boxes[j].getLowCorner()) - grow;
      const Vec3T<T> hi = min(boxes[i].getHighCorner(), boxes[j].getHighCorner()) + grow;

      if (lo[0] <= hi[0] && lo[1] <= hi[1] && lo[2] <= hi[2]) {
        a_box = a_any ? BoundingVolumes::AABBT<T>(min(a_box.getLowCorner(), lo), max(a_box.getHighCorner(), hi))
                      : BoundingVolumes::AABBT<T>(lo, hi);
        a_any = true;
      }
    }
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
        // The plain union, exact, united with where the smooth union is inside and may differ from it, marched
        // up to what the plain union is known up to (see RayDetail::BlendZoneView).
        RayDetail::fusedOperandSpans<0>(a_shape, a_ray, a_tMin, a_tMax, a_options, a_firstOnly, a_spans);

        if (a_firstOnly) {
          RayDetail::keepFirst(a_spans);
        }

        RayDetail::fusedUnite<1>(a_shape, a_ray, a_tMin, a_options, a_firstOnly, a_spans, operand);

        // The blend zone view, marched where two operands' boxes grown by w overlap, up to what the plain union is
        // known up to (see RayDetail::BlendZoneView).
        const T level = op.m_smooth + RayDetail::blendDepth(op.m_smooth, false);

        BoundingVolumes::AABBT<T> zone(Vec3T<T>::zeros(), Vec3T<T>::zeros());

        bool any = false;

        RayDetail::pairOverlaps(a_shape, level, zone, any, std::index_sequence_for<Cs...>{});

        // With no overlap, the smooth union is the plain union.
        if (any) {
          RayDetail::shapeSpans(RayDetail::BlendZoneView<T, Cs...>{a_shape, zone},
                                a_ray,
                                a_tMin,
                                a_spans.m_horizon,
                                a_options,
                                a_firstOnly,
                                operand);
          RayDetail::uniteInto(a_spans, operand);

          if (a_firstOnly) {
            RayDetail::keepFirst(a_spans);
          }
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
