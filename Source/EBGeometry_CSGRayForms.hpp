// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_CSGRayForms.hpp
 * @brief  The ray forms (see RayForm) of compositions: the fused union, intersection, difference, complement
 * and affine maps (Fused::Node), and BVHUnion. Each traces its operands' spans along the ray (see
 * EBGeometry_RaySpans.hpp) and combines them, so that a composition of shapes with ray forms is traced
 * exactly; an operand without one is marched by itself, in its own box. An annulus of an operand traced
 * exactly throughout is marched from the operand's crossings. The other fused operations (smooth blends,
 * offsets, elongation, repetition, blur, mollification) have no ray form, and are marched.
 * Tape's ray form, which does the same clause by clause, is in EBGeometry_Tape.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_CSGRAYFORMS_HPP
#define EBGEOMETRY_CSGRAYFORMS_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

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
 * @brief Whether a shape is a fused node traced from its operands' spans.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S>
struct IsTracedFused : std::false_type
{
};

/**
 * @brief A fused node is, if its operation is traced exactly.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 */
template <class Op, class... Cs>
struct IsTracedFused<Fused::Node<Op, Cs...>> : Fused::Detail::TracedExactly<Op>
{
};

/**
 * @brief Whether a shape is traced exactly throughout: it has a ray form, and if it is a fused node, its
 * operation is traced exactly and so is each operand.
 * @details Internal helper; not part of the public API. What an annulus needs of its operand to be traced
 * from the operand's crossings: with a marched part in it, the operand's spans would each be marched.
 * @tparam S Shape type.
 */
template <class S>
struct TracedThroughout : std::bool_constant<HasRayFirstHit<S, ShapePrecision<S>>::value && !IsComposition<S>::value>
{
};

/**
 * @brief A fused node is, if its operation is traced exactly and so is each operand.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 */
template <class Op, class... Cs>
struct TracedThroughout<Fused::Node<Op, Cs...>>
  : std::bool_constant<Fused::Detail::TracedExactly<Op>::value && (TracedThroughout<Cs>::value && ...)>
{
};

/**
 * @brief How many leaves a shape has, as a fused composition numbers the crossings of its operands: its
 * operands' leaves for a traced fused node, and one for any other shape.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S>
struct LeafCount
{
  /**
   * @brief The count.
   */
  static constexpr uint32_t value = 1;
};

/**
 * @brief A fused node's leaves: its operands', one after another (one if it is not traced).
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 */
template <class Op, class... Cs>
struct LeafCount<Fused::Node<Op, Cs...>>
{
  /**
   * @brief The count.
   */
  static constexpr uint32_t value = Fused::Detail::TracedExactly<Op>::value ? (LeafCount<Cs>::value + ...) : 1U;
};

/**
 * @brief The first leaf of operand I among a fused node's leaves.
 * @details Internal helper; not part of the public API. A constant, not a function: a constexpr function is
 * host code to nvcc unless marked otherwise.
 * @tparam I  Operand index.
 * @tparam Cs Operand types.
 */
template <size_t I, class... Cs>
struct FirstLeaf;

/**
 * @brief Operand 0's first leaf is leaf 0.
 * @tparam C  First operand type.
 * @tparam Cs The others.
 */
template <class C, class... Cs>
struct FirstLeaf<0, C, Cs...>
{
  /**
   * @brief The leaf.
   */
  static constexpr uint32_t value = 0;
};

/**
 * @brief Operand I's first leaf is past the first operand's leaves.
 * @tparam I  Operand index.
 * @tparam C  First operand type.
 * @tparam Cs The others.
 */
template <size_t I, class C, class... Cs>
struct FirstLeaf<I, C, Cs...>
{
  /**
   * @brief The leaf.
   */
  static constexpr uint32_t value = LeafCount<C>::value + FirstLeaf<I - 1, Cs...>::value;
};

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
 * @brief Operand I's list of a fused node, its crossings named among the node's leaves.
 * @details Internal helper; not part of the public API.
 * @tparam I  Operand index.
 * @tparam T  Floating-point precision.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 * @param[in]  a_node      The node.
 * @param[in]  a_ray       The ray.
 * @param[in]  a_t0        Start of the interval.
 * @param[in]  a_t1        End of the interval.
 * @param[in]  a_options   How to march.
 * @param[in]  a_firstOnly Whether only the first crossing is needed.
 * @param[out] a_spans     The list.
 */
template <size_t I, class T, class Op, class... Cs>
EBGEOMETRY_HOST_DEVICE
inline void
fusedOperandSpans(const Fused::Node<Op, Cs...>& a_node,
                  const RayT<T>&                a_ray,
                  const T                       a_t0,
                  const T                       a_t1,
                  const RayOptions<T>&          a_options,
                  const bool                    a_firstOnly,
                  RaySpansT<T>&                 a_spans) noexcept
{
  using C = typename Fused::Detail::TypeAt<I, Cs...>::type;

  RayDetail::shapeSpans(RayDetail::fusedOperand<I>(a_node), a_ray, a_t0, a_t1, a_options, a_firstOnly, a_spans);

  if constexpr (IsTracedFused<C>::value) {
    RayDetail::offsetLeaves(FirstLeaf<I, Cs...>::value, a_spans);
  }
  else {
    RayDetail::nameLeaf(FirstLeaf<I, Cs...>::value, a_spans);
  }
}

/**
 * @brief Fold a fused node's operands I, I + 1, ... into a list by union or intersection.
 * @details Internal helper; not part of the public API. Each operand is traced only up to what the list
 * so far is known up to, beyond which it can change nothing known.
 * @tparam I       First operand to fold in.
 * @tparam IsUnion Whether to unite (or intersect).
 * @tparam T       Floating-point precision.
 * @tparam Op      Operation.
 * @tparam Cs      Operand types.
 * @param[in]     a_node      The node.
 * @param[in]     a_ray       The ray.
 * @param[in]     a_t0        Start of the interval.
 * @param[in]     a_options   How to march.
 * @param[in]     a_firstOnly Whether the operands need only their first crossings.
 * @param[in,out] a_spans     The list so far.
 * @param[in,out] a_operand   Scratch for an operand's list.
 * @param[in,out] a_scratch   Scratch for an intersection.
 */
template <size_t I, bool IsUnion, class T, class Op, class... Cs>
EBGEOMETRY_HOST_DEVICE
inline void
fusedFold(const Fused::Node<Op, Cs...>& a_node,
          const RayT<T>&                a_ray,
          const T                       a_t0,
          const RayOptions<T>&          a_options,
          const bool                    a_firstOnly,
          RaySpansT<T>&                 a_spans,
          RaySpansT<T>&                 a_operand,
          RaySpansT<T>&                 a_scratch) noexcept
{
  if constexpr (I < sizeof...(Cs)) {
    RayDetail::fusedOperandSpans<I>(a_node, a_ray, a_t0, a_spans.m_horizon, a_options, a_firstOnly, a_operand);

    if constexpr (IsUnion) {
      RayDetail::uniteInto(a_spans, a_operand);

      if (a_firstOnly) {
        RayDetail::keepFirst(a_spans);
      }
    }
    else {
      RayDetail::intersectInto(a_spans, a_operand, a_scratch);
    }

    RayDetail::fusedFold<I + 1, IsUnion>(a_node, a_ray, a_t0, a_options, a_firstOnly, a_spans, a_operand, a_scratch);
  }
}

/**
 * @brief The crossing at a distance along a ray on one of operand I's leaves of a fused node.
 * @details Internal helper; not part of the public API.
 * @tparam I  Operand index.
 * @tparam T  Floating-point precision.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 * @param[in] a_node    The node.
 * @param[in] a_ray     The ray.
 * @param[in] a_t       The crossing's distance.
 * @param[in] a_leaf    The leaf, among the node's.
 * @param[in] a_options How to march.
 * @return The crossing on the leaf.
 */
template <size_t I, class T, class Op, class... Cs>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
fusedOperandLeafHit(const Fused::Node<Op, Cs...>& a_node,
                    const RayT<T>&                a_ray,
                    const T                       a_t,
                    const uint32_t                a_leaf,
                    const RayOptions<T>&          a_options) noexcept
{
  using C = typename Fused::Detail::TypeAt<I, Cs...>::type;

  if constexpr (IsTracedFused<C>::value) {
    return RayForm<C>::leafHit(
      RayDetail::fusedOperand<I>(a_node), a_ray, a_t, a_leaf - FirstLeaf<I, Cs...>::value, a_options);
  }
  else {
    return RayDetail::leafHit(RayDetail::fusedOperand<I>(a_node), a_ray, a_t, a_options);
  }
}

/**
 * @brief The crossing at a distance along a ray on a leaf of a fused node: in the operand holding the leaf.
 * @details Internal helper; not part of the public API. A fold over the operands rather than a recursion,
 * which nvcc can leave without device code.
 * @tparam T  Floating-point precision.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 * @tparam Is Operand indices.
 * @param[in] a_node    The node.
 * @param[in] a_ray     The ray.
 * @param[in] a_t       The crossing's distance.
 * @param[in] a_leaf    The leaf, among the node's.
 * @param[in] a_options How to march.
 * @return The crossing on the leaf.
 */
template <class T, class Op, class... Cs, size_t... Is>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
fusedLeafHit(const Fused::Node<Op, Cs...>& a_node,
             const RayT<T>&                a_ray,
             const T                       a_t,
             const uint32_t                a_leaf,
             const RayOptions<T>&          a_options,
             std::index_sequence<Is...>) noexcept
{
  HitT<T> hit;

  const auto holds = [a_leaf](const uint32_t a_first, const uint32_t a_count) noexcept {
    return a_leaf >= a_first && a_leaf < a_first + a_count;
  };

  (void)((holds(FirstLeaf<Is, Cs...>::value, LeafCount<typename Fused::Detail::TypeAt<Is, Cs...>::type>::value)
            ? (hit = RayDetail::fusedOperandLeafHit<Is>(a_node, a_ray, a_t, a_leaf, a_options), true)
            : false) ||
         ...);

  return hit;
}

} // namespace RayDetail

/**
 * @brief The ray form of a fused union, intersection, difference (an intersection with a complement),
 * complement or affine map: its operands' spans, combined. Exact where every leaf has a ray form, and
 * callable on a device.
 * @details A union asks its operands only for their first crossings unless the ray starts inside one, and
 * an intersection or complement asks for all their spans (see EBGeometry_RaySpans.hpp). An operand without
 * a ray form is marched in its own box. The normal is the leaf's at the crossing the trace ends at, mapped
 * back through the affine maps above it. HitT::m_primitive is 0: a fused composition is one shape.
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

    const auto resolve = [&a_shape, &a_ray, &a_options](const T a_t, const uint32_t a_leaf) noexcept {
      return leafHit(a_shape, a_ray, a_t, a_leaf, a_options);
    };

    HitT<T> hit = RayDetail::traceSpans(trace, resolve, a_ray, a_tMin, a_tMax);

    hit.m_primitive = 0U;

    return hit;
  }

  /**
   * @brief Where a ray is inside the composition over an interval, its crossings named by the leaves they
   * lie on (depth first, see RayDetail::LeafCount).
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
    if constexpr (std::is_same_v<Op, Fused::UnionOp> || std::is_same_v<Op, Fused::IntersectionOp>) {
      constexpr bool isUnion = std::is_same_v<Op, Fused::UnionOp>;

      // An intersection needs all of each operand's spans: where the first one ends, the ray may still be
      // inside the others.
      const bool firstOnly = isUnion && a_firstOnly;

      RaySpansT<T> operand;
      RaySpansT<T> scratch;

      RayDetail::fusedOperandSpans<0>(a_shape, a_ray, a_tMin, a_tMax, a_options, firstOnly, a_spans);

      if (firstOnly) {
        RayDetail::keepFirst(a_spans);
      }

      RayDetail::fusedFold<1, isUnion>(a_shape, a_ray, a_tMin, a_options, firstOnly, a_spans, operand, scratch);
    }
    else if constexpr (std::is_same_v<Op, Fused::ComplementOp>) {
      RaySpansT<T> operand;

      RayDetail::fusedOperandSpans<0>(a_shape, a_ray, a_tMin, a_tMax, a_options, false, operand);
      RayDetail::complement(operand, a_tMin, a_spans);
    }
    else {
      const CSGOps::AffineCoefficients<T> map    = CSGOps::affineCoefficients(a_shape.layout().m_op.m_map);
      const RayDetail::MappedRay<T>       mapped = RayDetail::mapRay(map.m_values, a_ray, a_tMin, a_tMax);

      RayDetail::fusedOperandSpans<0>(a_shape, mapped.m_ray, mapped.m_t0, mapped.m_t1, a_options, a_firstOnly, a_spans);
      RayDetail::mapBack(mapped, a_tMin, a_tMax, a_spans);
    }
  }

  /**
   * @brief The crossing at a distance along a ray on one of the composition's leaves: its normal (outward
   * of the leaf, in the composition's frame) and its primitive.
   * @param[in] a_shape   The composition.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_t       The crossing's distance.
   * @param[in] a_leaf    The leaf, as spans() names it.
   * @param[in] a_options How to march the operands without a ray form.
   * @return The crossing on the leaf.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  leafHit(const Node& a_shape, const RayT<T>& a_ray, T a_t, uint32_t a_leaf, const RayOptions<T>& a_options) noexcept
  {
    if constexpr (std::is_same_v<Op, Fused::UnionOp> || std::is_same_v<Op, Fused::IntersectionOp> ||
                  std::is_same_v<Op, Fused::ComplementOp>) {
      return RayDetail::fusedLeafHit(a_shape, a_ray, a_t, a_leaf, a_options, std::index_sequence_for<Cs...>{});
    }
    else {
      const CSGOps::AffineCoefficients<T> map    = CSGOps::affineCoefficients(a_shape.layout().m_op.m_map);
      const RayDetail::MappedRay<T>       mapped = RayDetail::mapRay(map.m_values, a_ray, a_t, a_t);

      HitT<T> hit = RayDetail::fusedLeafHit(
        a_shape, mapped.m_ray, mapped.m_t0, a_leaf, a_options, std::index_sequence_for<Cs...>{});

      hit.m_normal = RayDetail::mapNormalBack(map.m_values, hit.m_normal);

      return hit;
    }
  }
};

/**
 * @brief The ray form of a fused annulus (Fused::Annular) of an operand traced exactly throughout: a shell
 * around the operand's surface, found from the operand's crossings (see RayDetail::annularSpans()).
 * @details Each of the operand's crossings is inside the shell, and its surfaces are marched to from there,
 * so that the operations around the annulus stay exact and only the shell itself is marched. An annulus
 * whose value is not a distance, or whose half-thickness is zero or less, is marched as a whole. The normal is the annulus's gradient. HitT::m_primitive
 * is 0.
 * @tparam T Floating-point precision.
 * @tparam C Operand type.
 */
template <class T, class C>
struct RayForm<Fused::Node<Fused::ShiftOp<T, true>, C>, std::enable_if_t<RayDetail::TracedThroughout<C>::value>>
{
  /**
   * @brief The node type.
   */
  using Node = Fused::Node<Fused::ShiftOp<T, true>, C>;

  /**
   * @brief Whether the annulus is traced from its operand's crossings: its value is a distance (or a bound),
   * and its half-thickness is positive, so that each of the operand's crossings is inside the shell.
   * Otherwise it is marched as a whole.
   * @param[in] a_shape The annulus.
   * @return True if traced from the operand's crossings.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline bool
  seeded(const Node& a_shape) noexcept
  {
    return RayDetail::qualityOf(a_shape) != DistanceQuality::NotADistance && a_shape.layout().m_op.m_amount > T(0);
  }

  /**
   * @brief The first crossing of the annulus along a ray.
   * @param[in] a_shape   The annulus.
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
    if (!seeded(a_shape)) {
      return RayDetail::march(a_shape, a_ray, a_tMin, a_tMax, a_options);
    }

    const auto trace =
      [&a_shape, &a_ray, &a_options](const T a_t0, const T a_t1, const bool a_all, RaySpansT<T>& a_spans) noexcept {
        spans(a_shape, a_ray, a_t0, a_t1, a_options, !a_all, a_spans);
      };

    const auto resolve = [&a_shape, &a_ray, &a_options](const T a_t, const uint32_t) noexcept {
      const auto box = a_shape.computeBoundingVolume();
      const T    h   = a_options.m_tolerance * RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);

      HitT<T> hit;

      hit.m_normal = RayDetail::gradientNormal(a_shape, a_ray.at(a_t), h);

      return hit;
    };

    HitT<T> hit = RayDetail::traceSpans(trace, resolve, a_ray, a_tMin, a_tMax);

    hit.m_primitive = 0U;

    return hit;
  }

  /**
   * @brief Where a ray is inside the annulus over an interval.
   * @param[in]  a_shape     The annulus.
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
    if (!seeded(a_shape)) {
      RayOptions<T> marched = a_options;

      marched.m_march = true;

      if (a_firstOnly) {
        RayDetail::firstSpan(a_shape, a_ray, a_tMin, a_tMax, marched, a_spans);
      }
      else {
        RayDetail::crossingSpans(a_shape, a_ray, a_tMin, a_tMax, marched, a_spans);
      }

      return;
    }

    RaySpansT<T> operand;

    RayDetail::shapeSpans(RayDetail::fusedOperand<0>(a_shape), a_ray, a_tMin, a_tMax, a_options, false, operand);
    RayDetail::annularSpans(a_shape, operand, a_ray, a_tMin, a_tMax, a_options, a_spans);
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

    const auto resolve = [&a_shape, &a_ray, &a_options](const T a_t, const uint32_t a_leaf) noexcept {
      return leafHit(a_shape, a_ray, a_t, a_leaf, a_options);
    };

    return RayDetail::traceSpans(trace, resolve, a_ray, a_tMin, a_tMax);
  }

  /**
   * @brief Where a ray is inside the union over an interval, each crossing named by its primitive: the
   * unbounded ones first, then the BVH's, in their stored order.
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

    const PoolLocation location   = a_shape.location();
    const Root         bvh        = a_shape.getBVH();
    const auto         unbounded  = a_shape.getUnboundedPrimitives();
    const auto         primitives = bvh.getPrimitives();

    // Each primitive is traced only up to what the union so far is known up to, beyond which it can change
    // nothing known.
    const auto add = [&a_ray, a_tMin, &a_options, a_firstOnly, &location, &operand](
                       RaySpansT<T>& a_union, const CSGDetail::Stored<P>& a_primitive, const uint32_t a_leaf) noexcept {
      RayDetail::shapeSpans(CSGDetail::fromStored<P>(a_primitive, location),
                            a_ray,
                            a_tMin,
                            a_union.m_horizon,
                            a_options,
                            a_firstOnly,
                            operand);
      RayDetail::nameLeaf(a_leaf, operand);
      RayDetail::uniteInto(a_union, operand);

      if (a_firstOnly) {
        RayDetail::keepFirst(a_union);
      }
    };

    for (uint32_t i = 0; i < unbounded.size(); i++) {
      add(a_spans, unbounded[i], i);
    }

    const uint32_t numUnbounded = unbounded.size();

    const auto leaf =
      [&primitives, &add, numUnbounded](RaySpansT<T>& a_union, const size_t a_offset, const size_t a_count) noexcept {
        for (size_t i = a_offset; i < a_offset + a_count; i++) {
          add(a_union, primitives[static_cast<uint32_t>(i)], numUnbounded + static_cast<uint32_t>(i));
        }
      };

    // A box the ray enters beyond what the union is known up to can change nothing known.
    const auto bound = [](const RaySpansT<T>& a_union) noexcept -> T { return a_union.m_horizon; };

    bvh.traverse(BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction, a_tMin), a_spans, leaf, bound);
  }

  /**
   * @brief The crossing at a distance along a ray on one of the union's primitives.
   * @param[in] a_shape   The union.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_t       The crossing's distance.
   * @param[in] a_leaf    The primitive, as spans() names it.
   * @param[in] a_options How to march primitives without a ray form.
   * @return The crossing on the primitive.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  leafHit(const BVHUnion<T, P, K>& a_shape,
          const RayT<T>&           a_ray,
          T                        a_t,
          uint32_t                 a_leaf,
          const RayOptions<T>&     a_options) noexcept
  {
    const PoolLocation location  = a_shape.location();
    const auto         unbounded = a_shape.getUnboundedPrimitives();

    const auto primitives = a_shape.getBVH().getPrimitives();

    const CSGDetail::Stored<P> stored =
      (a_leaf < unbounded.size()) ? unbounded[a_leaf] : primitives[a_leaf - unbounded.size()];

    return RayDetail::leafHit(CSGDetail::fromStored<P>(stored, location), a_ray, a_t, a_options);
  }
};

} // namespace EBGeometry

#endif
