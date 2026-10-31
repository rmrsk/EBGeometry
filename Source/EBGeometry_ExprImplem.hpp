// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_ExprImplem.hpp
 * @brief  Implementation of EBGeometry_Expr.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_EXPRIMPLEM_HPP
#define EBGEOMETRY_EXPRIMPLEM_HPP

// Std includes
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"

namespace EBGeometry {

namespace ExprDetail {

/**
 * @brief The box that covers all of space, at plus or minus Math::Limits<T>::max().
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @return The box.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline BoundingVolumes::AABBT<T>
everywhere() noexcept
{
  return BoundingVolumes::AABBT<T>(-Vec3T<T>::max(), Vec3T<T>::max());
}

/**
 * @brief Whether a box is empty (some low coordinate above the high one).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_box Box.
 * @return True if empty.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline bool
isEmpty(const BoundingVolumes::AABBT<T>& a_box) noexcept
{
  for (size_t dir = 0; dir < 3; dir++) {
    if (!(a_box.getLowCorner()[dir] <= a_box.getHighCorner()[dir])) {
      return true;
    }
  }

  return false;
}

/**
 * @brief A box grown by a margin along every axis, saturating at plus or minus Math::Limits<T>::max().
 * @details Internal helper; not part of the public API. An empty box stays empty.
 * @tparam T Floating-point precision.
 * @param[in] a_box    Box.
 * @param[in] a_margin Margin along each axis (>= 0).
 * @return The grown box.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline BoundingVolumes::AABBT<T>
grown(const BoundingVolumes::AABBT<T>& a_box, const Vec3T<T>& a_margin) noexcept
{
  if (isEmpty(a_box)) {
    return a_box;
  }

  Vec3T<T> lo;
  Vec3T<T> hi;

  for (size_t dir = 0; dir < 3; dir++) {
    lo[dir] = Math::max(a_box.getLowCorner()[dir] - a_margin[dir], -Math::Limits<T>::max());
    hi[dir] = Math::min(a_box.getHighCorner()[dir] + a_margin[dir], Math::Limits<T>::max());
  }

  return BoundingVolumes::AABBT<T>(lo, hi);
}

/**
 * @brief A box grown by the same margin along every axis.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_box    Box.
 * @param[in] a_margin Margin (>= 0).
 * @return The grown box.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline BoundingVolumes::AABBT<T>
grown(const BoundingVolumes::AABBT<T>& a_box, const T a_margin) noexcept
{
  return grown(a_box, a_margin * Vec3T<T>::ones());
}

/**
 * @brief The intersection of two boxes (empty if they are disjoint).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_first  First box.
 * @param[in] a_second Second box.
 * @return The intersection.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline BoundingVolumes::AABBT<T>
intersected(const BoundingVolumes::AABBT<T>& a_first, const BoundingVolumes::AABBT<T>& a_second) noexcept
{
  if (isEmpty(a_first) || isEmpty(a_second)) {
    return BoundingVolumes::AABBT<T>();
  }

  const Vec3T<T> lo = max(a_first.getLowCorner(), a_second.getLowCorner());
  const Vec3T<T> hi = min(a_first.getHighCorner(), a_second.getHighCorner());

  // Check before constructing: AABBT rejects an inverted box.
  for (size_t dir = 0; dir < 3; dir++) {
    if (!(lo[dir] <= hi[dir])) {
      return BoundingVolumes::AABBT<T>();
    }
  }

  return BoundingVolumes::AABBT<T>(lo, hi);
}

/**
 * @brief The worse of two distance qualities (Exact < Bound < NotADistance).
 * @details Internal helper; not part of the public API.
 * @param[in] a_first  First quality.
 * @param[in] a_second Second quality.
 * @return The worse one.
 */
[[nodiscard]] inline DistanceQuality
worse(const DistanceQuality a_first, const DistanceQuality a_second) noexcept
{
  return static_cast<int>(a_first) >= static_cast<int>(a_second) ? a_first : a_second;
}

/**
 * @brief A new node of a given kind with the given children.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_kind     Kind.
 * @param[in] a_children Children.
 * @return The node.
 */
template <class T>
[[nodiscard]] inline std::shared_ptr<Node<T>>
makeNode(const Kind a_kind, std::vector<Expr<T>> a_children)
{
  auto node = std::make_shared<Node<T>>();

  node->m_kind     = a_kind;
  node->m_children = std::move(a_children);

  return node;
}

/**
 * @brief Convert a list of shapes or Exprs to Exprs.
 * @details Internal helper; not part of the public API.
 * @tparam A Shape or Expr type.
 * @param[in] a_operands Shapes or Exprs.
 * @return The Exprs.
 */
template <class A>
[[nodiscard]] inline std::vector<Expr<ExprPrecision<A>>>
toExprs(const std::vector<A>& a_operands)
{
  std::vector<Expr<ExprPrecision<A>>> exprs;

  exprs.reserve(a_operands.size());

  for (const auto& operand : a_operands) {
    exprs.emplace_back(toExpr(operand));
  }

  return exprs;
}

/**
 * @brief An n-ary union or intersection, sharp or smooth.
 * @details Internal helper; not part of the public API. A single operand is returned as it is, which
 * gives the same value for every kind.
 * @tparam T Floating-point precision.
 * @param[in] a_kind     Union, Intersection, SmoothUnion or SmoothIntersection.
 * @param[in] a_operands Operands (must not be empty).
 * @param[in] a_smooth   Smoothing length, for the smooth kinds.
 * @param[in] a_blend    Blend kind, for the smooth kinds.
 * @return The combination.
 */
template <class T>
[[nodiscard]] inline Expr<T>
combine(const Kind a_kind, std::vector<Expr<T>> a_operands, const T a_smooth, const BlendKind a_blend)
{
  if (a_operands.size() == 1) {
    return a_operands.front();
  }

  const bool isUnion = (a_kind == Kind::Union || a_kind == Kind::SmoothUnion);
  const bool smooth  = (a_kind == Kind::SmoothUnion || a_kind == Kind::SmoothIntersection);

  auto node = makeNode<T>(a_kind, std::move(a_operands));

  node->m_scalar0 = a_smooth;
  node->m_blend   = a_blend;

  BoundingVolumes::AABBT<T> box = isUnion ? BoundingVolumes::AABBT<T>() : everywhere<T>();
  DistanceQuality           q   = DistanceQuality::Bound;

  for (const auto& child : node->m_children) {
    box = isUnion ? box.merged(child.computeBoundingVolume()) : intersected(box, child.computeBoundingVolume());
    q   = worse(q, child.getDistanceQuality());
  }

  // A smooth minimum lies below the sharp one by at most s/4 (polynomial) or s ln 2 (exponential), so
  // the union's surface moves out by at most that much. A smooth maximum only shrinks the object.
  if (smooth && isUnion) {
    box = grown(box, a_blend == BlendKind::Polynomial ? T(0.25) * a_smooth : a_smooth * std::log(T(2)));
  }

  node->m_boundingVolume = box;
  node->m_quality        = q;

  return Expr<T>(std::move(node));
}

/**
 * @brief A chain of point maps folded into one Affine node: the child is evaluated at a_map(p) and its
 * value multiplied by a_valueScale.
 * @details Internal helper; not part of the public API. An operand that is itself an Affine node is
 * folded in, so a chain of Translate, Rotate, Reflect and Scale becomes one node.
 * @tparam T Floating-point precision.
 * @param[in] a_operand    Operand.
 * @param[in] a_map        Point map.
 * @param[in] a_inverse    Its inverse.
 * @param[in] a_valueScale Value multiplier (> 0).
 * @return The mapped object.
 */
template <class T>
[[nodiscard]] inline Expr<T>
affine(const Expr<T>& a_operand, const Affine3T<T>& a_map, const Affine3T<T>& a_inverse, const T a_valueScale)
{
  const Node<T>& inner = a_operand.node();

  // The outer map is applied to the query point first: the child sees inner(outer(p)).
  const bool fold  = (inner.m_kind == Kind::Affine);
  const auto child = fold ? inner.m_children.front() : a_operand;

  auto node = makeNode<T>(Kind::Affine, {child});

  node->m_map        = fold ? a_map.then(inner.m_map) : a_map;
  node->m_inverse    = fold ? inner.m_inverse.then(a_inverse) : a_inverse;
  node->m_valueScale = fold ? a_valueScale * inner.m_valueScale : a_valueScale;

  node->m_boundingVolume = node->m_inverse.transform(child.computeBoundingVolume());
  node->m_quality        = child.getDistanceQuality();

  return Expr<T>(std::move(node));
}

/**
 * @brief A node with one child that changes only the value, with the given box and quality.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_kind    Kind.
 * @param[in] a_operand Operand.
 * @param[in] a_scalar  The node's scalar parameter.
 * @param[in] a_box     The node's box.
 * @param[in] a_quality The node's quality.
 * @return The node.
 */
template <class T>
[[nodiscard]] inline std::shared_ptr<Node<T>>
unary(const Kind                       a_kind,
      const Expr<T>&                   a_operand,
      const T                          a_scalar,
      const BoundingVolumes::AABBT<T>& a_box,
      const DistanceQuality            a_quality)
{
  auto node = makeNode<T>(a_kind, {a_operand});

  node->m_scalar0        = a_scalar;
  node->m_boundingVolume = a_box;
  node->m_quality        = a_quality;

  return node;
}

/**
 * @brief Check that a smoothing length is positive and finite.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_who    Name of the calling function, for the message.
 * @param[in] a_smooth Smoothing length.
 */
template <class T>
inline void
requireSmoothingLength(const char* a_who, const T a_smooth)
{
  EBGEOMETRY_REQUIRE(std::isfinite(a_smooth) && a_smooth > T(0),
                     "%s: the smoothing length must be positive and finite (%g)",
                     a_who,
                     double(a_smooth));
}

/**
 * @brief Check that a vector is finite.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_v Vector.
 * @return True if every component is finite.
 */
template <class T>
[[nodiscard]] inline bool
isFinite(const Vec3T<T>& a_v) noexcept
{
  return std::isfinite(a_v[0]) && std::isfinite(a_v[1]) && std::isfinite(a_v[2]);
}

/**
 * @brief Whether a pool-resident shape lives in a pool.
 * @details Internal helper; not part of the public API.
 * @tparam S Pool-resident shape type.
 * @param[in] a_object The shape.
 * @param[in] a_pool   The pool.
 * @return True if the shape's arrays were reserved from a_pool.
 */
template <class S>
[[nodiscard]] inline bool
isAttachedTo(const void* a_object, const Pool& a_pool)
{
  return static_cast<const S*>(a_object)->isAttachedTo(a_pool);
}

} // namespace ExprDetail

template <class T>
template <class S, class>
Expr<T>::Expr(const S& a_shape)
{
  static_assert(isShape<S>,
                "Expr: a shape must be trivially copyable, with a const signedDistance(const Vec3T<T>&) returning T "
                "and a const computeBoundingVolume() returning BoundingVolumes::AABBT<T>");
  static_assert(std::is_same_v<ShapePrecision<S>, T>, "Expr: the shape's precision must be the expression's");

  auto node = std::make_shared<ExprDetail::Node<T>>();

  node->m_kind                = ExprDetail::Kind::Primitive;
  node->m_leaf.m_object       = std::make_shared<const S>(a_shape);
  node->m_leaf.m_type         = std::type_index(typeid(S));
  node->m_boundingVolume      = a_shape.computeBoundingVolume();
  node->m_quality             = distanceQualityOf<S>;
  node->m_leaf.m_isAttachedTo = nullptr;

  if constexpr (isPoolHandle<S>) {
    node->m_leaf.m_isAttachedTo = &ExprDetail::isAttachedTo<S>;
  }

  m_node = std::move(node);
}

template <class T>
Expr<T>::Expr(std::shared_ptr<const ExprDetail::Node<T>> a_node) noexcept : m_node(std::move(a_node))
{}

template <class T>
const typename Expr<T>::BV&
Expr<T>::computeBoundingVolume() const noexcept
{
  return m_node->m_boundingVolume;
}

template <class T>
DistanceQuality
Expr<T>::getDistanceQuality() const noexcept
{
  return m_node->m_quality;
}

template <class T>
const ExprDetail::Node<T>&
Expr<T>::node() const noexcept
{
  return *m_node;
}

template <class A>
inline Expr<ExprPrecision<A>>
toExpr(const A& a_operand)
{
  return Expr<ExprPrecision<A>>(a_operand);
}

template <class A, class B>
inline Expr<ExprPrecision<A>>
Union(const A& a_first, const B& a_second)
{
  using T = ExprPrecision<A>;

  static_assert(std::is_same_v<T, ExprPrecision<B>>, "Union: both operands must have the same precision");

  return ExprDetail::combine<T>(
    ExprDetail::Kind::Union, {toExpr(a_first), toExpr(a_second)}, T(0), BlendKind::Polynomial);
}

template <class A>
inline Expr<ExprPrecision<A>>
Union(const std::vector<A>& a_operands)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(!a_operands.empty(), "Union: the list of operands must not be empty");

  return ExprDetail::combine<T>(ExprDetail::Kind::Union, ExprDetail::toExprs(a_operands), T(0), BlendKind::Polynomial);
}

template <class A, class B>
inline Expr<ExprPrecision<A>>
SmoothUnion(const A& a_first, const B& a_second, const ExprPrecision<A> a_smooth, const BlendKind a_blend)
{
  using T = ExprPrecision<A>;

  static_assert(std::is_same_v<T, ExprPrecision<B>>, "SmoothUnion: both operands must have the same precision");

  ExprDetail::requireSmoothingLength("SmoothUnion", a_smooth);

  return ExprDetail::combine<T>(ExprDetail::Kind::SmoothUnion, {toExpr(a_first), toExpr(a_second)}, a_smooth, a_blend);
}

template <class A>
inline Expr<ExprPrecision<A>>
SmoothUnion(const std::vector<A>& a_operands, const ExprPrecision<A> a_smooth, const BlendKind a_blend)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(!a_operands.empty(), "SmoothUnion: the list of operands must not be empty");

  ExprDetail::requireSmoothingLength("SmoothUnion", a_smooth);

  return ExprDetail::combine<T>(ExprDetail::Kind::SmoothUnion, ExprDetail::toExprs(a_operands), a_smooth, a_blend);
}

template <class A, class B>
inline Expr<ExprPrecision<A>>
Intersection(const A& a_first, const B& a_second)
{
  using T = ExprPrecision<A>;

  static_assert(std::is_same_v<T, ExprPrecision<B>>, "Intersection: both operands must have the same precision");

  return ExprDetail::combine<T>(
    ExprDetail::Kind::Intersection, {toExpr(a_first), toExpr(a_second)}, T(0), BlendKind::Polynomial);
}

template <class A>
inline Expr<ExprPrecision<A>>
Intersection(const std::vector<A>& a_operands)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(!a_operands.empty(), "Intersection: the list of operands must not be empty");

  return ExprDetail::combine<T>(
    ExprDetail::Kind::Intersection, ExprDetail::toExprs(a_operands), T(0), BlendKind::Polynomial);
}

template <class A, class B>
inline Expr<ExprPrecision<A>>
SmoothIntersection(const A& a_first, const B& a_second, const ExprPrecision<A> a_smooth, const BlendKind a_blend)
{
  using T = ExprPrecision<A>;

  static_assert(std::is_same_v<T, ExprPrecision<B>>, "SmoothIntersection: both operands must have the same precision");

  ExprDetail::requireSmoothingLength("SmoothIntersection", a_smooth);

  return ExprDetail::combine<T>(
    ExprDetail::Kind::SmoothIntersection, {toExpr(a_first), toExpr(a_second)}, a_smooth, a_blend);
}

template <class A>
inline Expr<ExprPrecision<A>>
SmoothIntersection(const std::vector<A>& a_operands, const ExprPrecision<A> a_smooth, const BlendKind a_blend)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(!a_operands.empty(), "SmoothIntersection: the list of operands must not be empty");

  ExprDetail::requireSmoothingLength("SmoothIntersection", a_smooth);

  return ExprDetail::combine<T>(
    ExprDetail::Kind::SmoothIntersection, ExprDetail::toExprs(a_operands), a_smooth, a_blend);
}

template <class A, class B>
inline Expr<ExprPrecision<A>>
Difference(const A& a_first, const B& a_second)
{
  using T = ExprPrecision<A>;

  static_assert(std::is_same_v<T, ExprPrecision<B>>, "Difference: both operands must have the same precision");

  return Intersection(toExpr(a_first), Complement(toExpr(a_second)));
}

template <class A, class B>
inline Expr<ExprPrecision<A>>
Difference(const A& a_first, const std::vector<B>& a_others)
{
  using T = ExprPrecision<A>;

  static_assert(std::is_same_v<T, ExprPrecision<B>>, "Difference: all operands must have the same precision");

  EBGEOMETRY_REQUIRE(!a_others.empty(), "Difference: the list of objects to subtract must not be empty");

  return Intersection(toExpr(a_first), Complement(Union(a_others)));
}

template <class A, class B>
inline Expr<ExprPrecision<A>>
SmoothDifference(const A& a_first, const B& a_second, const ExprPrecision<A> a_smooth, const BlendKind a_blend)
{
  using T = ExprPrecision<A>;

  static_assert(std::is_same_v<T, ExprPrecision<B>>, "SmoothDifference: both operands must have the same precision");

  ExprDetail::requireSmoothingLength("SmoothDifference", a_smooth);

  return SmoothIntersection(toExpr(a_first), Complement(toExpr(a_second)), a_smooth, a_blend);
}

template <class A, class B>
inline Expr<ExprPrecision<A>>
SmoothDifference(const A&               a_first,
                 const std::vector<B>&  a_others,
                 const ExprPrecision<A> a_smooth,
                 const BlendKind        a_blend)
{
  using T = ExprPrecision<A>;

  static_assert(std::is_same_v<T, ExprPrecision<B>>, "SmoothDifference: all operands must have the same precision");

  EBGEOMETRY_REQUIRE(!a_others.empty(), "SmoothDifference: the list of objects to subtract must not be empty");

  ExprDetail::requireSmoothingLength("SmoothDifference", a_smooth);

  std::vector<Expr<T>> operands;

  operands.reserve(1 + a_others.size());
  operands.emplace_back(toExpr(a_first));

  for (const auto& other : a_others) {
    operands.emplace_back(Complement(toExpr(other)));
  }

  return SmoothIntersection(operands, a_smooth, a_blend);
}

template <class A>
inline Expr<ExprPrecision<A>>
FiniteRepetition(const A&                       a_operand,
                 const Vec3T<ExprPrecision<A>>& a_period,
                 const Vec3T<ExprPrecision<A>>& a_repeatLo,
                 const Vec3T<ExprPrecision<A>>& a_repeatHi)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_period) && a_period[0] > T(0) && a_period[1] > T(0) && a_period[2] > T(0),
                     "FiniteRepetition: the period must be positive and finite in every direction (%g, %g, %g)",
                     double(a_period[0]),
                     double(a_period[1]),
                     double(a_period[2]));
  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_repeatLo) && a_repeatLo[0] >= T(0) && a_repeatLo[1] >= T(0) &&
                       a_repeatLo[2] >= T(0),
                     "FiniteRepetition: the low repetition counts must be finite and not negative (%g, %g, %g)",
                     double(a_repeatLo[0]),
                     double(a_repeatLo[1]),
                     double(a_repeatLo[2]));
  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_repeatHi) && a_repeatHi[0] >= T(0) && a_repeatHi[1] >= T(0) &&
                       a_repeatHi[2] >= T(0),
                     "FiniteRepetition: the high repetition counts must be finite and not negative (%g, %g, %g)",
                     double(a_repeatHi[0]),
                     double(a_repeatHi[1]),
                     double(a_repeatHi[2]));

  const Expr<T> child = toExpr(a_operand);

  auto node = ExprDetail::makeNode<T>(ExprDetail::Kind::Repeat, {child});

  node->m_vec0 = a_period;

  for (size_t dir = 0; dir < 3; dir++) {
    node->m_vec1[dir] = std::round(a_repeatLo[dir]);
    node->m_vec2[dir] = std::round(a_repeatHi[dir]);
  }

  // The object, swept over the tiles. The value is the minimum over all the copies (CSGOps::repetition()),
  // so it is a distance bound whenever the object's is.
  const BoundingVolumes::AABBT<T>& box = child.computeBoundingVolume();

  Vec3T<T> lo;
  Vec3T<T> hi;

  for (size_t dir = 0; dir < 3; dir++) {
    lo[dir] = Math::max(box.getLowCorner()[dir] - a_period[dir] * node->m_vec1[dir], -Math::Limits<T>::max());
    hi[dir] = Math::min(box.getHighCorner()[dir] + a_period[dir] * node->m_vec2[dir], Math::Limits<T>::max());
  }

  node->m_boundingVolume = ExprDetail::isEmpty(box) ? box : BoundingVolumes::AABBT<T>(lo, hi);
  node->m_quality        = ExprDetail::worse(child.getDistanceQuality(), DistanceQuality::Bound);

  return Expr<T>(std::move(node));
}

template <class A>
inline Expr<ExprPrecision<A>>
Complement(const A& a_operand)
{
  using T = ExprPrecision<A>;

  const Expr<T> child = toExpr(a_operand);

  return Expr<T>(ExprDetail::unary<T>(
    ExprDetail::Kind::Complement, child, T(0), ExprDetail::everywhere<T>(), child.getDistanceQuality()));
}

template <class A>
inline Expr<ExprPrecision<A>>
Translate(const A& a_operand, const Vec3T<ExprPrecision<A>>& a_shift)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_shift),
                     "Translate: the shift must be finite (%g, %g, %g)",
                     double(a_shift[0]),
                     double(a_shift[1]),
                     double(a_shift[2]));

  return ExprDetail::affine<T>(
    toExpr(a_operand), Affine3T<T>::translation(-a_shift), Affine3T<T>::translation(a_shift), T(1));
}

template <class A>
inline Expr<ExprPrecision<A>>
Rotate(const A& a_operand, const ExprPrecision<A> a_degrees, const size_t a_axis)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_degrees), "Rotate: the rotation angle must be finite (%g)", double(a_degrees));
  EBGEOMETRY_REQUIRE(a_axis <= 2U, "Rotate: the rotation axis must be 0, 1 or 2 (%zu)", a_axis);

  // The object turns by +angle, so the query point turns by -angle.
  return ExprDetail::affine<T>(
    toExpr(a_operand), Affine3T<T>::rotation(-a_degrees, a_axis), Affine3T<T>::rotation(a_degrees, a_axis), T(1));
}

template <class A>
inline Expr<ExprPrecision<A>>
Rotate(const A& a_operand, const ExprPrecision<A> a_degrees, const Vec3T<ExprPrecision<A>>& a_axis)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_degrees), "Rotate: the rotation angle must be finite (%g)", double(a_degrees));
  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_axis) && a_axis.length() > T(0),
                     "Rotate: the rotation axis must be finite and non-zero (%g, %g, %g)",
                     double(a_axis[0]),
                     double(a_axis[1]),
                     double(a_axis[2]));

  return ExprDetail::affine<T>(
    toExpr(a_operand), Affine3T<T>::rotation(-a_degrees, a_axis), Affine3T<T>::rotation(a_degrees, a_axis), T(1));
}

template <class A>
inline Expr<ExprPrecision<A>>
Scale(const A& a_operand, const ExprPrecision<A> a_factor)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_factor) && a_factor != T(0),
                     "Scale: the scale factor must be finite and non-zero (%g)",
                     double(a_factor));

  return ExprDetail::affine<T>(
    toExpr(a_operand), Affine3T<T>::scaling(T(1) / a_factor), Affine3T<T>::scaling(a_factor), std::abs(a_factor));
}

template <class A>
inline Expr<ExprPrecision<A>>
Reflect(const A& a_operand, const size_t a_axis)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(a_axis <= 2U, "Reflect: the reflection plane must be 0, 1 or 2 (%zu)", a_axis);

  return ExprDetail::affine<T>(
    toExpr(a_operand), Affine3T<T>::reflection(a_axis), Affine3T<T>::reflection(a_axis), T(1));
}

template <class A>
inline Expr<ExprPrecision<A>>
Offset(const A& a_operand, const ExprPrecision<A> a_offset)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_offset), "Offset: the offset must be finite (%g)", double(a_offset));

  const Expr<T>         child = toExpr(a_operand);
  const DistanceQuality q     = a_offset >= T(0) ? child.getDistanceQuality()
                                                 : ExprDetail::worse(child.getDistanceQuality(), DistanceQuality::Bound);

  return Expr<T>(ExprDetail::unary<T>(ExprDetail::Kind::Offset,
                                      child,
                                      a_offset,
                                      ExprDetail::grown(child.computeBoundingVolume(), Math::max(a_offset, T(0))),
                                      q));
}

template <class A>
inline Expr<ExprPrecision<A>>
Annular(const A& a_operand, const ExprPrecision<A> a_delta)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_delta), "Annular: the half-thickness must be finite (%g)", double(a_delta));

  const Expr<T> child = toExpr(a_operand);

  return Expr<T>(ExprDetail::unary<T>(ExprDetail::Kind::Annular,
                                      child,
                                      a_delta,
                                      ExprDetail::grown(child.computeBoundingVolume(), Math::max(a_delta, T(0))),
                                      child.getDistanceQuality()));
}

template <class A>
inline Expr<ExprPrecision<A>>
Elongate(const A& a_operand, const Vec3T<ExprPrecision<A>>& a_elongation)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(ExprDetail::isFinite(a_elongation) && a_elongation[0] >= T(0) && a_elongation[1] >= T(0) &&
                       a_elongation[2] >= T(0),
                     "Elongate: the elongation must be finite and non-negative (%g, %g, %g)",
                     double(a_elongation[0]),
                     double(a_elongation[1]),
                     double(a_elongation[2]));

  const Expr<T> child = toExpr(a_operand);

  auto node = ExprDetail::makeNode<T>(ExprDetail::Kind::Elongate, {child});

  node->m_vec0           = a_elongation;
  node->m_boundingVolume = ExprDetail::grown(child.computeBoundingVolume(), a_elongation);
  node->m_quality        = ExprDetail::worse(child.getDistanceQuality(), DistanceQuality::Bound);

  return Expr<T>(std::move(node));
}

template <class A>
inline Expr<ExprPrecision<A>>
Blur(const A& a_operand, const ExprPrecision<A> a_blur, const ExprPrecision<A> a_alpha)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_blur) && a_blur >= T(0),
                     "Blur: the blur distance must be finite and non-negative (%g)",
                     double(a_blur));
  EBGEOMETRY_REQUIRE(
    a_alpha >= T(0) && a_alpha <= T(1), "Blur: the blending weight alpha must lie in [0, 1] (%g)", double(a_alpha));

  const Expr<T>         child = toExpr(a_operand);
  const DistanceQuality q = child.getDistanceQuality() == DistanceQuality::NotADistance ? DistanceQuality::NotADistance
                                                                                        : DistanceQuality::Bound;

  auto node = ExprDetail::unary<T>(ExprDetail::Kind::Blur,
                                   child,
                                   a_blur,
                                   ExprDetail::grown(child.computeBoundingVolume(), std::sqrt(T(3)) * a_blur),
                                   q);

  node->m_scalar1 = a_alpha;

  return Expr<T>(std::move(node));
}

template <class A>
inline Expr<ExprPrecision<A>>
Mollify(const A& a_operand, const ExprPrecision<A> a_dist, const size_t a_samples)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(std::isfinite(a_dist), "Mollify: the mollification distance must be finite (%g)", double(a_dist));
  EBGEOMETRY_REQUIRE(a_samples >= 1U && a_samples <= 64U,
                     "Mollify: the number of samples per axis must be between 1 and 64 (%zu)",
                     a_samples);

  Expr<T> child = toExpr(a_operand);
  const T r     = std::abs(a_dist);

  // No samples to average over: the object itself.
  if (r == T(0) || a_samples == 1U) {
    return child;
  }

  const DistanceQuality q = child.getDistanceQuality() == DistanceQuality::NotADistance ? DistanceQuality::NotADistance
                                                                                        : DistanceQuality::Bound;

  auto node = ExprDetail::unary<T>(
    ExprDetail::Kind::Mollify, child, r, ExprDetail::grown(child.computeBoundingVolume(), std::sqrt(T(3)) * r), q);

  node->m_count = static_cast<uint32_t>(a_samples);

  return Expr<T>(std::move(node));
}

} // namespace EBGeometry

#endif
