// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_CSGRayForms.hpp
 * @brief  The ray forms (see RayForm) of compositions: the fused union, intersection, difference, complement
 * and affine maps (Fused::Node), and BVHUnion. Each traces its operands' spans along the ray (see
 * EBGeometry_RaySpans.hpp) and combines them, so that a composition of shapes with ray forms is traced
 * exactly; an operand without one is marched by itself, in its own box. The other fused operations
 * (smooth blends, offsets, elongation, repetition, blur, mollification) have no ray form, and are marched.
 * Tape's ray form, which does the same clause by clause, is in EBGeometry_Tape.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_CSGRAYFORMS_HPP
#define EBGEOMETRY_CSGRAYFORMS_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_AnalyticRayForms.hpp"
#include "EBGeometry_BVHUnion.hpp"
#include "EBGeometry_CSGOps.hpp"
#include "EBGeometry_Fused.hpp"
#include "EBGeometry_MeshRayForms.hpp"
#include "EBGeometry_Ray.hpp"

namespace EBGeometry {

namespace Fused {

namespace Detail {

/**
 * @brief Whether a fused operation is traced exactly from its operands' spans: union, intersection (and so
 * difference), complement and affine maps.
 * @tparam Op Operation.
 */
template <class Op>
struct TracedExactly : std::false_type
{
};

/**
 * @brief Union and intersection are traced exactly.
 * @tparam IsUnion Whether this is a union.
 */
template <bool IsUnion>
struct TracedExactly<CombineOp<IsUnion>> : std::true_type
{
};

/**
 * @brief Complement is traced exactly.
 */
template <>
struct TracedExactly<ComplementOp> : std::true_type
{
};

/**
 * @brief Affine maps are traced exactly.
 * @tparam T Floating-point precision.
 */
template <class T>
struct TracedExactly<AffineOp<T>> : std::true_type
{
};

} // namespace Detail

} // namespace Fused

namespace RayDetail {

/**
 * @brief Operand I of a fused node, as the shape it is.
 * @details Internal helper; not part of the public API.
 * @tparam I  Operand index.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 * @param[in] a_node The node.
 * @return The operand.
 */
template <size_t I, class Op, class... Cs>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline auto
fusedOperand(const Fused::Node<Op, Cs...>& a_node) noexcept
{
  using C = typename Fused::Detail::TypeAt<I, Cs...>::type;

  return CSGDetail::fromStored<C>(Fused::Detail::get<I>(a_node.layout().m_operands), a_node.location());
}

/**
 * @brief Fold a fused node's operands I, I + 1, ... into a list by union or intersection.
 * @details Internal helper; not part of the public API.
 * @tparam I       First operand to fold in.
 * @tparam IsUnion Whether to unite (or intersect).
 * @tparam T       Floating-point precision.
 * @tparam Op      Operation.
 * @tparam Cs      Operand types.
 * @param[in]     a_node      The node.
 * @param[in]     a_ray       The ray.
 * @param[in]     a_t0        Start of the interval.
 * @param[in]     a_t1        End of the interval.
 * @param[in]     a_options   How to march.
 * @param[in]     a_firstOnly Whether the operands need only their first crossings.
 * @param[in,out] a_spans     The list so far.
 * @param[in,out] a_operand   Scratch for an operand's list.
 * @param[in,out] a_scratch   Scratch for a combination.
 */
template <size_t I, bool IsUnion, class T, class Op, class... Cs>
EBGEOMETRY_HOST_DEVICE
inline void
fusedFold(const Fused::Node<Op, Cs...>& a_node,
          const RayT<T>&                a_ray,
          const T                       a_t0,
          const T                       a_t1,
          const RayOptions<T>&          a_options,
          const bool                    a_firstOnly,
          RaySpansT<T>&                 a_spans,
          RaySpansT<T>&                 a_operand,
          RaySpansT<T>&                 a_scratch) noexcept
{
  if constexpr (I < sizeof...(Cs)) {
    // Beyond what the list so far is known up to, the operand can change nothing known.
    RayDetail::shapeSpans(
      RayDetail::fusedOperand<I>(a_node), a_ray, a_t0, a_spans.m_horizon, a_options, a_firstOnly, a_operand);

    if constexpr (IsUnion) {
      RayDetail::uniteInto(a_spans, a_operand, a_scratch);

      if (a_firstOnly) {
        RayDetail::keepFirst(a_spans);
      }
    }
    else {
      RayDetail::intersectInto(a_spans, a_operand, a_scratch);
    }

    RayDetail::fusedFold<I + 1, IsUnion>(
      a_node, a_ray, a_t0, a_t1, a_options, a_firstOnly, a_spans, a_operand, a_scratch);
  }
}

} // namespace RayDetail

/**
 * @brief The ray form of a fused union, intersection, difference (an intersection with a complement),
 * complement or affine map: its operands' spans, combined. Exact where every leaf has a ray form, and
 * callable on a device.
 * @details A union asks its operands only for their first crossings unless the ray starts inside one, and
 * an intersection or complement asks for all their spans (see EBGeometry_RaySpans.hpp). An operand without
 * a ray form is marched in its own box. HitT::m_primitive is 0: a fused composition is one shape.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 */
template <class Op, class... Cs>
struct RayForm<Fused::Node<Op, Cs...>, std::enable_if_t<Fused::Detail::TracedExactly<Op>::value>>
{
  /**
   * @brief The node type.
   */
  using Node = Fused::Node<Op, Cs...>;

  /**
   * @brief Floating-point precision.
   */
  using T = typename Node::T;

  /**
   * @brief The first crossing of the composition along a ray.
   * @param[in] a_shape   The composition.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the operands without a ray form.
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

    HitT<T> hit = RayDetail::traceSpans(trace, a_ray, a_tMin, a_tMax);

    hit.m_primitive = 0U;

    return hit;
  }

  /**
   * @brief Where a ray is inside the composition over an interval.
   * @param[in]  a_shape     The composition.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   How to march the operands without a ray form.
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
    RaySpansT<T> operand;

    if constexpr (std::is_same_v<Op, Fused::UnionOp> || std::is_same_v<Op, Fused::IntersectionOp>) {
      constexpr bool isUnion = std::is_same_v<Op, Fused::UnionOp>;

      // An intersection needs all of each operand's spans: where the first one ends, the ray may still be
      // inside the others.
      const bool firstOnly = isUnion && a_firstOnly;

      RaySpansT<T> scratch;

      RayDetail::shapeSpans(RayDetail::fusedOperand<0>(a_shape), a_ray, a_tMin, a_tMax, a_options, firstOnly, a_spans);

      if (firstOnly) {
        RayDetail::keepFirst(a_spans);
      }

      RayDetail::fusedFold<1, isUnion>(a_shape, a_ray, a_tMin, a_tMax, a_options, firstOnly, a_spans, operand, scratch);
    }
    else if constexpr (std::is_same_v<Op, Fused::ComplementOp>) {
      RayDetail::shapeSpans(RayDetail::fusedOperand<0>(a_shape), a_ray, a_tMin, a_tMax, a_options, false, operand);
      RayDetail::complement(operand, a_tMin, a_spans);
    }
    else {
      const CSGOps::AffineCoefficients<T> map    = CSGOps::affineCoefficients(a_shape.layout().m_op.m_map);
      const RayDetail::MappedRay<T>       mapped = RayDetail::mapRay(map.m_values, a_ray, a_tMin, a_tMax);

      RayDetail::shapeSpans(
        RayDetail::fusedOperand<0>(a_shape), mapped.m_ray, mapped.m_t0, mapped.m_t1, a_options, a_firstOnly, operand);
      RayDetail::mapBack(map.m_values, mapped, a_tMin, a_tMax, operand, a_spans);
    }
  }
};

/**
 * @brief BVHUnion's ray form: the union of the spans of the primitives whose boxes the ray meets, found by
 * walking the BVH nearest box first and leaving out the boxes beyond what the union so far is known up to.
 * Exact where the primitive type has a ray form (and marched one primitive at a time otherwise), and
 * callable on a device.
 * @details Unless the ray starts inside a primitive, only the first crossings are needed, and the walk
 * stops at the first box beyond the nearest. HitT::m_primitive is what the primitive names (a mesh's face
 * id), or 0.
 * @tparam T Floating-point precision.
 * @tparam P Primitive type.
 * @tparam K BVH branching factor.
 */
template <class T, class P, size_t K>
struct RayForm<BVHUnion<T, P, K>>
{
  /**
   * @brief The first crossing of the union along a ray.
   * @param[in] a_shape   The union.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march primitives without a ray form.
   * @return The crossing, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(
    const BVHUnion<T, P, K>& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept
  {
    const auto trace =
      [&a_shape, &a_ray, &a_options](const T a_t0, const T a_t1, const bool a_all, RaySpansT<T>& a_spans) noexcept {
        spans(a_shape, a_ray, a_t0, a_t1, a_options, !a_all, a_spans);
      };

    return RayDetail::traceSpans(trace, a_ray, a_tMin, a_tMax);
  }

  /**
   * @brief Where a ray is inside the union over an interval.
   * @param[in]  a_shape     The union.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   How to march primitives without a ray form.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const BVHUnion<T, P, K>& a_shape,
        const RayT<T>&           a_ray,
        T                        a_tMin,
        T                        a_tMax,
        const RayOptions<T>&     a_options,
        bool                     a_firstOnly,
        RaySpansT<T>&            a_spans) noexcept
  {
    using Root = typename BVHUnion<T, P, K>::Root;

    a_spans.reset(a_tMax);

    RaySpansT<T> operand;
    RaySpansT<T> scratch;

    const PoolLocation location   = a_shape.location();
    const Root         bvh        = a_shape.getBVH();
    const auto         unbounded  = a_shape.getUnboundedPrimitives();
    const auto         primitives = bvh.getPrimitives();

    // Each primitive is traced only up to what the union so far is known up to, beyond which it can change
    // nothing known.
    const auto add = [&a_ray, a_tMin, &a_options, a_firstOnly, &location, &operand, &scratch](
                       RaySpansT<T>& a_union, const CSGDetail::Stored<P>& a_primitive) noexcept {
      RayDetail::shapeSpans(CSGDetail::fromStored<P>(a_primitive, location),
                            a_ray,
                            a_tMin,
                            a_union.m_horizon,
                            a_options,
                            a_firstOnly,
                            operand);
      RayDetail::uniteInto(a_union, operand, scratch);

      if (a_firstOnly) {
        RayDetail::keepFirst(a_union);
      }
    };

    for (uint32_t i = 0; i < unbounded.size(); i++) {
      add(a_spans, unbounded[i]);
    }

    const auto leaf = [&primitives, &add](RaySpansT<T>& a_union, const size_t a_offset, const size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        add(a_union, primitives[static_cast<uint32_t>(i)]);
      }
    };

    // A box the ray enters beyond what the union is known up to can change nothing known.
    const auto bound = [](const RaySpansT<T>& a_union) noexcept -> T { return a_union.m_horizon; };

    bvh.traverse(BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction, a_tMin), a_spans, leaf, bound);
  }
};

} // namespace EBGeometry

#endif
