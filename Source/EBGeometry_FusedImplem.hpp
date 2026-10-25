// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_FusedImplem.hpp
 * @brief  Implementation of EBGeometry_Fused.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_FUSEDIMPLEM_HPP
#define EBGEOMETRY_FUSEDIMPLEM_HPP

// Std includes
#include <cmath>

// Our includes
#include "EBGeometry_Fused.hpp"

namespace EBGeometry {
namespace Fused {

namespace Detail {

/**
 * @brief The empty pack.
 * @return The pack.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline Pack<>
makePack() noexcept
{
  return Pack<>{};
}

/**
 * @brief A pack of values.
 * @tparam S    First value type.
 * @tparam Rest The others.
 * @param[in] a_first  First value.
 * @param[in] a_others The others.
 * @return The pack.
 */
template <class S, class... Rest>
[[nodiscard]] EBGEOMETRY_HOST
inline Pack<S, Rest...>
makePack(const S& a_first, const Rest&... a_others)
{
  return Pack<S, Rest...>{a_first, makePack(a_others...)};
}

/**
 * @brief Call a function with each index below N, as a std::integral_constant.
 * @tparam F  Function type.
 * @tparam Is Indices.
 * @param[in] a_f Function.
 */
template <class F, size_t... Is>
EBGEOMETRY_HOST
inline void
forEachIndex(const F& a_f, std::index_sequence<Is...>)
{
  (a_f(std::integral_constant<size_t, Is>{}), ...);
}

/**
 * @brief Whether two locations resolve against the same pool (or the same snapshot of one).
 * @param[in] a_first  First location.
 * @param[in] a_second Second location.
 * @return True if they do.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline bool
sameLocation(const PoolLocation& a_first, const PoolLocation& a_second) noexcept
{
  return a_first.m_control == a_second.m_control && a_first.m_base == a_second.m_base;
}

/**
 * @brief Build a node of an operation and its operands, checking that the pool-resident operands all
 * live in one pool, whose location the node then takes.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 * @param[in] a_who      Name of the calling function, for the message.
 * @param[in] a_op       Operation.
 * @param[in] a_operands Operands.
 * @return The node.
 */
template <class Op, class... Cs>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<Op, Cs...>
make(const char* a_who, const Op& a_op, const Cs&... a_operands)
{
  static_assert((isShape<Cs> && ...), "Fused: every operand must be a shape (see isShape)");

  using N = Node<Op, Cs...>;

  const typename N::Layout layout{a_op, makePack(CSGDetail::toStored(a_operands)...)};

  if constexpr (N::isHandle) {
    PoolLocation location{};

    bool found = false;
    bool same  = true;

    const auto visit = [&location, &found, &same](const auto& a_operand) {
      using C = std::decay_t<decltype(a_operand)>;

      if constexpr (isPoolHandle<C>) {
        if (!found) {
          location = a_operand.location();
          found    = true;
        }
        else {
          same = same && sameLocation(location, a_operand.location());
        }
      }
    };

    (visit(a_operands), ...);

    EBGEOMETRY_REQUIRE(same, "%s: every pool-resident operand must live in the same pool", a_who);

    return N(layout, location);
  }
  else {
    return N(layout);
  }
}

/**
 * @brief Build a node of an operation from a list of operands.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 * @tparam Is Operand indices.
 * @param[in] a_who      Name of the calling function, for the message.
 * @param[in] a_op       Operation.
 * @param[in] a_operands Operands.
 * @return The node.
 */
template <class Op, class... Cs, size_t... Is>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<Op, Cs...>
makeFromList(const char* a_who, const Op& a_op, const OperandList<Cs...>& a_operands, std::index_sequence<Is...>)
{
  return make(a_who, a_op, get<Is>(a_operands.m_items)...);
}

/**
 * @brief Check a smoothing length, as the expression builder does.
 * @tparam T Floating-point precision.
 * @param[in] a_who    Name of the calling function, for the message.
 * @param[in] a_smooth Smoothing length.
 */
template <class T>
EBGEOMETRY_HOST
inline void
requireSmoothingLength(const char* a_who, const T a_smooth)
{
  EBGEOMETRY_REQUIRE(std::isfinite(a_smooth) && a_smooth > T(0),
                     "%s: the smoothing length must be positive and finite (%g)",
                     a_who,
                     double(a_smooth));
}

/**
 * @brief An affine map of a shape.
 * @tparam C Operand type.
 * @param[in] a_operand    Operand.
 * @param[in] a_map        Map from a query point to the point the operand sees.
 * @param[in] a_inverse    Its inverse.
 * @param[in] a_valueScale Value scale.
 * @return The mapped shape.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<AffineOp<ShapePrecision<C>>, C>
affine(const C&                           a_operand,
       const Affine3T<ShapePrecision<C>>& a_map,
       const Affine3T<ShapePrecision<C>>& a_inverse,
       const ShapePrecision<C>            a_valueScale)
{
  return make("Fused: an affine map", AffineOp<ShapePrecision<C>>{a_map, a_inverse, a_valueScale}, a_operand);
}

/**
 * @brief An affine map of an affine map: one map, composed as the expression builder composes it (the
 * outer map is applied to the query point first).
 * @tparam T Floating-point precision.
 * @tparam C The inner map's operand type.
 * @param[in] a_operand    The inner map.
 * @param[in] a_map        Outer map.
 * @param[in] a_inverse    Its inverse.
 * @param[in] a_valueScale Outer value scale.
 * @return The composed map.
 */
template <class T, class C>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<AffineOp<T>, C>
affine(const Node<AffineOp<T>, C>& a_operand,
       const Affine3T<T>&          a_map,
       const Affine3T<T>&          a_inverse,
       const T                     a_valueScale)
{
  using N = Node<AffineOp<T>, C>;

  const AffineOp<T>& inner = a_operand.layout().m_op;

  const typename N::Layout layout{
    AffineOp<T>{a_map.then(inner.m_map), inner.m_inverse.then(a_inverse), a_valueScale * inner.m_valueScale},
    a_operand.layout().m_operands};

  if constexpr (N::isHandle) {
    return N(layout, a_operand.location());
  }
  else {
    return N(layout);
  }
}

} // namespace Detail

template <class Op, class... Cs>
inline Node<Op, Cs...>
Node<Op, Cs...>::rebasedView(const Pool& a_pool) const noexcept
{
  if constexpr (!isHandle) {
    return *this;
  }
  else {
    // Rebasing each pool-resident operand checks its arrays; they all live in one pool, so they rebase
    // to one location.
    PoolLocation location{};

    Detail::forEachIndex(
      [this, &a_pool, &location](const auto a_index) {
        using C = typename Detail::TypeAt<decltype(a_index)::value, Cs...>::type;

        if constexpr (isPoolHandle<C>) {
          const C stored =
            CSGDetail::fromStored<C>(Detail::get<decltype(a_index)::value>(m_layout.m_operands), this->location());

          location = stored.rebasedView(a_pool).location();
        }
      },
      std::index_sequence_for<Cs...>{});

    return Node(m_layout, location);
  }
}

template <class Op, class... Cs>
inline Node<Op, Cs...>
Node<Op, Cs...>::deepCopy(Pool& a_dstPool) const
{
  if constexpr (!isHandle) {
    return *this;
  }
  else {
    Layout       copy = m_layout;
    PoolLocation location{};

    Detail::forEachIndex(
      [this, &a_dstPool, &copy, &location](const auto a_index) {
        constexpr size_t I = decltype(a_index)::value;

        using C = typename Detail::TypeAt<I, Cs...>::type;

        if constexpr (isPoolHandle<C>) {
          const C copied =
            CSGDetail::fromStored<C>(Detail::get<I>(m_layout.m_operands), this->location()).deepCopy(a_dstPool);

          Detail::get<I>(copy.m_operands) = copied.layout();

          location = copied.location();
        }
      },
      std::index_sequence_for<Cs...>{});

    return Node(copy, location);
  }
}

template <class T>
template <class O>
EBGEOMETRY_HOST_DEVICE
inline BoundingVolumes::AABBT<T>
RepetitionOp<T>::box(const O& a_operands) const
{
  // The operand, swept over the tiles, as the expression builder sweeps it.
  const BoundingVolumes::AABBT<T> box = a_operands.template box<0>();

  if (ExprDetail::isEmpty(box)) {
    return box;
  }

  Vec3T<T> lo;
  Vec3T<T> hi;

  for (size_t dir = 0; dir < 3; dir++) {
    lo[dir] = Math::max(box.getLowCorner()[dir] - m_repetition.m_period[dir] * m_repetition.m_low[dir],
                        -Math::Limits<T>::max());
    hi[dir] = Math::min(box.getHighCorner()[dir] + m_repetition.m_period[dir] * m_repetition.m_high[dir],
                        Math::Limits<T>::max());
  }

  return BoundingVolumes::AABBT<T>(lo, hi);
}

template <class... Cs>
inline OperandList<Cs...>
operands(const Cs&... a_operands)
{
  static_assert(sizeof...(Cs) > 0, "Fused::operands: the list of operands must not be empty");
  static_assert((isShape<Cs> && ...), "Fused::operands: every operand must be a shape (see isShape)");

  return OperandList<Cs...>{Detail::makePack(a_operands...)};
}

template <class C, class... Cs>
inline auto
Union(const C& a_first, const Cs&... a_others)
{
  static_assert(isShape<C>, "Fused::Union: every operand must be a shape (see isShape)");

  if constexpr (sizeof...(Cs) == 0) {
    return a_first;
  }
  else {
    return Detail::make("Fused::Union", UnionOp{}, a_first, a_others...);
  }
}

template <class C, class... Cs>
inline auto
Intersection(const C& a_first, const Cs&... a_others)
{
  static_assert(isShape<C>, "Fused::Intersection: every operand must be a shape (see isShape)");

  if constexpr (sizeof...(Cs) == 0) {
    return a_first;
  }
  else {
    return Detail::make("Fused::Intersection", IntersectionOp{}, a_first, a_others...);
  }
}

template <class A, class B, class... Bs>
inline auto
Difference(const A& a_first, const B& a_second, const Bs&... a_others)
{
  if constexpr (sizeof...(Bs) == 0) {
    return Fused::Intersection(a_first, Fused::Complement(a_second));
  }
  else {
    return Fused::Intersection(a_first, Fused::Complement(Fused::Union(a_second, a_others...)));
  }
}

template <class C>
inline Node<ComplementOp, C>
Complement(const C& a_operand)
{
  return Detail::make("Fused::Complement", ComplementOp{}, a_operand);
}

template <class A, class B>
inline Node<SmoothCombineOp<ShapePrecision<A>, true>, A, B>
SmoothUnion(const A& a_first, const B& a_second, const ShapePrecision<A> a_smooth, const BlendKind a_blend)
{
  using T = ShapePrecision<A>;

  Detail::requireSmoothingLength("Fused::SmoothUnion", a_smooth);

  return Detail::make(
    "Fused::SmoothUnion", SmoothCombineOp<T, true>{a_smooth, a_blend == BlendKind::Exponential}, a_first, a_second);
}

template <class... Cs>
inline auto
SmoothUnion(const OperandList<Cs...>&                                     a_operands,
            const ShapePrecision<typename Detail::TypeAt<0, Cs...>::type> a_smooth,
            const BlendKind                                               a_blend)
{
  using T = ShapePrecision<typename Detail::TypeAt<0, Cs...>::type>;

  Detail::requireSmoothingLength("Fused::SmoothUnion", a_smooth);

  if constexpr (sizeof...(Cs) == 1) {
    return Detail::get<0>(a_operands.m_items);
  }
  else {
    return Detail::makeFromList("Fused::SmoothUnion",
                                SmoothCombineOp<T, true>{a_smooth, a_blend == BlendKind::Exponential},
                                a_operands,
                                std::index_sequence_for<Cs...>{});
  }
}

template <class A, class B>
inline Node<SmoothCombineOp<ShapePrecision<A>, false>, A, B>
SmoothIntersection(const A& a_first, const B& a_second, const ShapePrecision<A> a_smooth, const BlendKind a_blend)
{
  using T = ShapePrecision<A>;

  Detail::requireSmoothingLength("Fused::SmoothIntersection", a_smooth);

  return Detail::make("Fused::SmoothIntersection",
                      SmoothCombineOp<T, false>{a_smooth, a_blend == BlendKind::Exponential},
                      a_first,
                      a_second);
}

template <class... Cs>
inline auto
SmoothIntersection(const OperandList<Cs...>&                                     a_operands,
                   const ShapePrecision<typename Detail::TypeAt<0, Cs...>::type> a_smooth,
                   const BlendKind                                               a_blend)
{
  using T = ShapePrecision<typename Detail::TypeAt<0, Cs...>::type>;

  Detail::requireSmoothingLength("Fused::SmoothIntersection", a_smooth);

  if constexpr (sizeof...(Cs) == 1) {
    return Detail::get<0>(a_operands.m_items);
  }
  else {
    return Detail::makeFromList("Fused::SmoothIntersection",
                                SmoothCombineOp<T, false>{a_smooth, a_blend == BlendKind::Exponential},
                                a_operands,
                                std::index_sequence_for<Cs...>{});
  }
}

template <class A, class B>
inline auto
SmoothDifference(const A& a_first, const B& a_second, const ShapePrecision<A> a_smooth, const BlendKind a_blend)
{
  Detail::requireSmoothingLength("Fused::SmoothDifference", a_smooth);

  return Fused::SmoothIntersection(a_first, Fused::Complement(a_second), a_smooth, a_blend);
}

namespace Detail {

/**
 * @brief The smooth intersection of an operand and the complements of a list of others.
 * @tparam A  Operand to subtract from.
 * @tparam Bs Operands to subtract.
 * @tparam Is Their indices.
 * @param[in] a_first  Operand to subtract from.
 * @param[in] a_others Operands to subtract.
 * @param[in] a_smooth Smoothing length.
 * @param[in] a_blend  Blend kind.
 * @return The smooth difference.
 */
template <class A, class... Bs, size_t... Is>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
smoothDifference(const A&                  a_first,
                 const OperandList<Bs...>& a_others,
                 const ShapePrecision<A>   a_smooth,
                 const BlendKind           a_blend,
                 std::index_sequence<Is...>)
{
  return Fused::SmoothIntersection(
    Fused::operands(a_first, Fused::Complement(get<Is>(a_others.m_items))...), a_smooth, a_blend);
}

} // namespace Detail

template <class A, class... Bs>
inline auto
SmoothDifference(const A&                  a_first,
                 const OperandList<Bs...>& a_others,
                 const ShapePrecision<A>   a_smooth,
                 const BlendKind           a_blend)
{
  Detail::requireSmoothingLength("Fused::SmoothDifference", a_smooth);

  return Detail::smoothDifference(a_first, a_others, a_smooth, a_blend, std::index_sequence_for<Bs...>{});
}

template <class C>
inline auto
Translate(const C& a_operand, const Vec3T<ShapePrecision<C>>& a_shift)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_shift),
                     "Fused::Translate: the shift must be finite (%g, %g, %g)",
                     double(a_shift[0]),
                     double(a_shift[1]),
                     double(a_shift[2]));

  return Detail::affine(a_operand, Affine3T<T>::translation(-a_shift), Affine3T<T>::translation(a_shift), T(1));
}

template <class C>
inline auto
Rotate(const C& a_operand, const ShapePrecision<C> a_degrees, const size_t a_axis)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(
    std::isfinite(a_degrees), "Fused::Rotate: the rotation angle must be finite (%g)", double(a_degrees));
  EBGEOMETRY_REQUIRE(a_axis <= 2U, "Fused::Rotate: the rotation axis must be 0, 1 or 2 (%zu)", a_axis);

  return Detail::affine(
    a_operand, Affine3T<T>::rotation(-a_degrees, a_axis), Affine3T<T>::rotation(a_degrees, a_axis), T(1));
}

template <class C>
inline auto
Rotate(const C& a_operand, const ShapePrecision<C> a_degrees, const Vec3T<ShapePrecision<C>>& a_axis)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(
    std::isfinite(a_degrees), "Fused::Rotate: the rotation angle must be finite (%g)", double(a_degrees));
  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_axis) && a_axis.length() > T(0),
                     "Fused::Rotate: the rotation axis must be finite and non-zero (%g, %g, %g)",
                     double(a_axis[0]),
                     double(a_axis[1]),
                     double(a_axis[2]));

  return Detail::affine(
    a_operand, Affine3T<T>::rotation(-a_degrees, a_axis), Affine3T<T>::rotation(a_degrees, a_axis), T(1));
}

template <class C>
inline auto
Scale(const C& a_operand, const ShapePrecision<C> a_factor)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_factor) && a_factor != T(0),
                     "Fused::Scale: the scale factor must be finite and non-zero (%g)",
                     double(a_factor));

  return Detail::affine(
    a_operand, Affine3T<T>::scaling(T(1) / a_factor), Affine3T<T>::scaling(a_factor), std::abs(a_factor));
}

template <class C>
inline auto
Reflect(const C& a_operand, const size_t a_axis)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(a_axis <= 2U, "Fused::Reflect: the reflection plane must be 0, 1 or 2 (%zu)", a_axis);

  return Detail::affine(a_operand, Affine3T<T>::reflection(a_axis), Affine3T<T>::reflection(a_axis), T(1));
}

template <class C>
inline Node<ShiftOp<ShapePrecision<C>, false>, C>
Offset(const C& a_operand, const ShapePrecision<C> a_offset)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_offset), "Fused::Offset: the offset must be finite (%g)", double(a_offset));

  return Detail::make("Fused::Offset", ShiftOp<T, false>{a_offset}, a_operand);
}

template <class C>
inline Node<ShiftOp<ShapePrecision<C>, true>, C>
Annular(const C& a_operand, const ShapePrecision<C> a_delta)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_delta), "Fused::Annular: the half-thickness must be finite (%g)", double(a_delta));

  return Detail::make("Fused::Annular", ShiftOp<T, true>{a_delta}, a_operand);
}

template <class C>
inline Node<ElongateOp<ShapePrecision<C>>, C>
Elongate(const C& a_operand, const Vec3T<ShapePrecision<C>>& a_elongation)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_elongation) && a_elongation[0] >= T(0) && a_elongation[1] >= T(0) &&
                       a_elongation[2] >= T(0),
                     "Fused::Elongate: the elongation must be finite and non-negative (%g, %g, %g)",
                     double(a_elongation[0]),
                     double(a_elongation[1]),
                     double(a_elongation[2]));

  return Detail::make("Fused::Elongate", ElongateOp<T>{a_elongation}, a_operand);
}

template <class C>
inline Node<RepetitionOp<ShapePrecision<C>>, C>
FiniteRepetition(const C&                        a_operand,
                 const Vec3T<ShapePrecision<C>>& a_period,
                 const Vec3T<ShapePrecision<C>>& a_repeatLo,
                 const Vec3T<ShapePrecision<C>>& a_repeatHi)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_period) && a_period[0] > T(0) && a_period[1] > T(0) && a_period[2] > T(0),
                     "Fused::FiniteRepetition: the period must be positive and finite in every direction (%g, %g, %g)",
                     double(a_period[0]),
                     double(a_period[1]),
                     double(a_period[2]));
  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_repeatLo) && a_repeatLo[0] >= T(0) && a_repeatLo[1] >= T(0) &&
                       a_repeatLo[2] >= T(0),
                     "Fused::FiniteRepetition: the low repetition counts must be finite and not negative (%g, %g, %g)",
                     double(a_repeatLo[0]),
                     double(a_repeatLo[1]),
                     double(a_repeatLo[2]));
  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_repeatHi) && a_repeatHi[0] >= T(0) && a_repeatHi[1] >= T(0) &&
                       a_repeatHi[2] >= T(0),
                     "Fused::FiniteRepetition: the high repetition counts must be finite and not negative (%g, %g, %g)",
                     double(a_repeatHi[0]),
                     double(a_repeatHi[1]),
                     double(a_repeatHi[2]));

  const BoundingVolumes::AABBT<T> box = a_operand.computeBoundingVolume();

  CSGOps::Repetition<T> repetition{a_period, Vec3T<T>(), Vec3T<T>(), box.getLowCorner(), box.getHighCorner()};

  for (size_t dir = 0; dir < 3; dir++) {
    repetition.m_low[dir]  = std::round(a_repeatLo[dir]);
    repetition.m_high[dir] = std::round(a_repeatHi[dir]);
  }

  return Detail::make("Fused::FiniteRepetition", RepetitionOp<T>{repetition}, a_operand);
}

template <class C>
inline Node<BlurOp<ShapePrecision<C>>, C>
Blur(const C& a_operand, const ShapePrecision<C> a_blur, const ShapePrecision<C> a_alpha)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_blur) && a_blur >= T(0),
                     "Fused::Blur: the blur distance must be finite and non-negative (%g)",
                     double(a_blur));
  EBGEOMETRY_REQUIRE(a_alpha >= T(0) && a_alpha <= T(1),
                     "Fused::Blur: the blending weight alpha must lie in [0, 1] (%g)",
                     double(a_alpha));

  return Detail::make("Fused::Blur", BlurOp<T>{CSGOps::blurWeights(a_blur, a_alpha)}, a_operand);
}

template <class C>
inline Node<MollifyOp<ShapePrecision<C>>, C>
Mollify(const C& a_operand, const ShapePrecision<C> a_dist, const size_t a_samples)
{
  using T = ShapePrecision<C>;

  EBGEOMETRY_REQUIRE(
    std::isfinite(a_dist), "Fused::Mollify: the mollification distance must be finite (%g)", double(a_dist));
  EBGEOMETRY_REQUIRE(a_samples >= 1U && a_samples <= 64U,
                     "Fused::Mollify: the number of samples per axis must be between 1 and 64 (%zu)",
                     a_samples);

  const T    r        = std::abs(a_dist);
  const bool identity = (r == T(0) || a_samples == 1U);

  const CSGOps::MollifyGrid<T> grid =
    identity ? CSGOps::MollifyGrid<T>{r, T(0), T(1), 1U} : CSGOps::mollifyGrid(r, static_cast<uint32_t>(a_samples));

  return Detail::make("Fused::Mollify", MollifyOp<T>{grid, identity}, a_operand);
}

} // namespace Fused
} // namespace EBGeometry

#endif
