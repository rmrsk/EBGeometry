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
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
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
 * @brief The name of an operation, for messages.
 * @details Internal helper; not part of the public API.
 * @param[in] a_kind Kind.
 * @return The builder function's name.
 */
[[nodiscard]] inline const char*
kindName(const Kind a_kind) noexcept
{
  switch (a_kind) {
  case Kind::Affine: {
    return "Transform";
  }
  case Kind::Elongate: {
    return "Elongate";
  }
  case Kind::Repeat: {
    return "FiniteRepetition";
  }
  case Kind::Complement: {
    return "Complement";
  }
  case Kind::Offset: {
    return "Offset";
  }
  case Kind::Annular: {
    return "Annular";
  }
  case Kind::Blur: {
    return "Blur";
  }
  case Kind::Mollify: {
    return "Mollify";
  }
  case Kind::Union: {
    return "Union";
  }
  case Kind::Intersection: {
    return "Intersection";
  }
  case Kind::SmoothUnion: {
    return "SmoothUnion";
  }
  case Kind::SmoothIntersection: {
    return "SmoothIntersection";
  }
  default: {
    return "Expr";
  }
  }
}

/**
 * @brief A new node of a given kind with the given children.
 * @details Internal helper; not part of the public API. Checks where a partition (an Overlay) and a material label
 * may be: a partition only in an Overlay, under a map, or as the first operand of an intersection (clipped by the
 * others, plain solids); a material label only on an operand of an Overlay or under maps.
 * @tparam T Floating-point precision.
 * @param[in] a_kind     Kind.
 * @param[in] a_children Children.
 * @param[in] a_overlay  Whether the node is an Overlay (a Union of regions).
 * @return The node.
 */
template <class T>
[[nodiscard]] inline std::shared_ptr<Node<T>>
makeNode(const Kind a_kind, std::vector<Expr<T>> a_children, const bool a_overlay = false)
{
  for (size_t i = 0; i < a_children.size(); i++) {
    const Node<T>& child = a_children[i].node();

    if (child.m_partition) {
      const bool clipped = (a_kind == Kind::Intersection) && (i == 0);

      EBGEOMETRY_REQUIRE(a_overlay || a_kind == Kind::Affine || clipped,
                         "%s: an Overlay is a partition into regions, not a solid; it can be moved, clipped or drilled "
                         "as the first operand of an Intersection or Difference, or placed in another Overlay",
                         kindName(a_kind));
    }
    else if (child.m_namesRegion) {
      EBGEOMETRY_REQUIRE(a_overlay || a_kind == Kind::Affine,
                         "%s: MaterialLabel and RegionName name a region (an operand of an Overlay) or a whole "
                         "object, not a part of a solid; make the parts regions of an Overlay",
                         kindName(a_kind));
    }
  }

  auto node = std::make_shared<Node<T>>();

  const bool firstIsPartition = !a_children.empty() && a_children.front().node().m_partition;

  node->m_kind        = a_kind;
  node->m_overlay     = a_overlay;
  node->m_partition   = a_overlay || ((a_kind == Kind::Affine || a_kind == Kind::Intersection) && firstIsPartition);
  node->m_namesRegion = (a_kind == Kind::Affine) && !a_children.empty() && a_children.front().node().m_namesRegion;
  node->m_children    = std::move(a_children);

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
combine(const Kind           a_kind,
        std::vector<Expr<T>> a_operands,
        const T              a_smooth,
        const BlendKind      a_blend,
        const bool           a_bvh = false)
{
  if (a_operands.size() == 1) {
    return a_operands.front();
  }

  const bool isUnion = (a_kind == Kind::Union || a_kind == Kind::SmoothUnion);
  const bool smooth  = (a_kind == Kind::SmoothUnion || a_kind == Kind::SmoothIntersection);

  auto node = makeNode<T>(a_kind, std::move(a_operands));

  node->m_scalar0 = a_smooth;
  node->m_blend   = a_blend;
  node->m_bvh     = a_bvh;

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

  // The outer map is applied to the query point first: the child sees inner(outer(p)). A labeled map stays
  // a node of its own, so that its label keeps the leaves it names.
  const bool fold = (inner.m_kind == Kind::Affine) && inner.m_surfaceLabel.empty() && inner.m_materialLabel.empty() &&
                    inner.m_regionName.empty();
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

/**
 * @brief How many surfaces a container holds (see hasSurfaces).
 * @details Internal helper; not part of the public API.
 * @tparam S Container type.
 * @param[in] a_object The shape.
 * @return Its getNumSurfaces().
 */
template <class S>
[[nodiscard]] inline uint32_t
numSurfaces(const void* a_object)
{
  return static_cast<const S*>(a_object)->getNumSurfaces();
}

/**
 * @brief How many parts a shape has (see hasParts).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type with parts.
 * @param[in] a_object The shape.
 * @return Its getNumParts().
 */
template <class S>
[[nodiscard]] inline uint32_t
numParts(const void* a_object)
{
  return static_cast<const S*>(a_object)->getNumParts();
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

  if constexpr (hasSurfaces<S, T>) {
    node->m_leaf.m_numSurfaces = &ExprDetail::numSurfaces<S>;
  }

  if constexpr (hasParts<S, T>) {
    node->m_leaf.m_numParts = &ExprDetail::numParts<S>;
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
Union(const std::vector<A>& a_operands, const Accelerate a_accelerate)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(!a_operands.empty(), "Union: the list of operands must not be empty");

  return ExprDetail::combine<T>(ExprDetail::Kind::Union,
                                ExprDetail::toExprs(a_operands),
                                T(0),
                                BlendKind::Polynomial,
                                a_accelerate == Accelerate::BVH);
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
SmoothUnion(const std::vector<A>&  a_operands,
            const ExprPrecision<A> a_smooth,
            const BlendKind        a_blend,
            const Accelerate       a_accelerate)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(!a_operands.empty(), "SmoothUnion: the list of operands must not be empty");

  ExprDetail::requireSmoothingLength("SmoothUnion", a_smooth);

  return ExprDetail::combine<T>(
    ExprDetail::Kind::SmoothUnion, ExprDetail::toExprs(a_operands), a_smooth, a_blend, a_accelerate == Accelerate::BVH);
}

template <class A>
inline Expr<ExprPrecision<A>>
SmoothUnion(const std::vector<A>& a_operands, const ExprPrecision<A> a_smooth, const Accelerate a_accelerate)
{
  return SmoothUnion(a_operands, a_smooth, BlendKind::Polynomial, a_accelerate);
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
Difference(const A& a_first, const std::vector<B>& a_others, const Accelerate a_accelerate)
{
  using T = ExprPrecision<A>;

  static_assert(std::is_same_v<T, ExprPrecision<B>>, "Difference: all operands must have the same precision");

  EBGEOMETRY_REQUIRE(!a_others.empty(), "Difference: the list of objects to subtract must not be empty");

  return Intersection(toExpr(a_first), Complement(Union(a_others, a_accelerate)));
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

template <class A>
inline Expr<ExprPrecision<A>>
SurfaceLabel(const A& a_operand, std::string a_name)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(!a_name.empty(), "SurfaceLabel: the name must not be empty");

  // A copy of the node, so that other uses of the same subexpression keep their own label.
  auto node = std::make_shared<ExprDetail::Node<T>>(toExpr(a_operand).node());

  node->m_surfaceLabel = std::move(a_name);

  return Expr<T>(std::move(node));
}

template <class A>
inline Expr<ExprPrecision<A>>
SurfaceLabel(const A& a_operand, FaceGroups a_groups)
{
  using T = ExprPrecision<A>;

  const Expr<T> operand = toExpr(a_operand);

  EBGEOMETRY_REQUIRE(operand.node().m_kind == ExprDetail::Kind::Primitive,
                     "SurfaceLabel: face groups label the parts of one shape, not of a composition (label it first)");

  const ExprDetail::Leaf& leaf = operand.node().m_leaf;

  // A collection's parts are its members' (closestSurface() reports the member's part), not the leaf's own, so its
  // parts cannot be labeled by one table.
  EBGEOMETRY_REQUIRE(
    leaf.m_numSurfaces == nullptr,
    "SurfaceLabel: face groups label the parts of one shape, not of a collection (a union, a cloud, a tape)");

  const uint32_t parts = (leaf.m_numParts != nullptr) ? leaf.m_numParts(leaf.m_object.get()) : 1U;

  EBGEOMETRY_REQUIRE(a_groups.m_group.size() == parts,
                     "SurfaceLabel: the face groups name %zu parts of a shape with %u",
                     a_groups.m_group.size(),
                     unsigned(parts));

  for (size_t p = 0; p < a_groups.m_group.size(); p++) {
    EBGEOMETRY_REQUIRE(a_groups.m_group[p] == FaceGroups::None || a_groups.m_group[p] < a_groups.m_names.size(),
                       "SurfaceLabel: part %zu is in group %u of %zu",
                       p,
                       unsigned(a_groups.m_group[p]),
                       a_groups.m_names.size());
  }

  for (const std::string& name : a_groups.m_names) {
    EBGEOMETRY_REQUIRE(!name.empty(), "SurfaceLabel: the name must not be empty");
  }

  // A copy of the node, so that other uses of the same shape keep their own labels.
  auto node = std::make_shared<ExprDetail::Node<T>>(operand.node());

  node->m_partGroups = std::make_shared<const FaceGroups>(std::move(a_groups));

  return Expr<T>(std::move(node));
}

template <class A>
inline Expr<ExprPrecision<A>>
Overlay(const std::vector<A>& a_operands)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(!a_operands.empty(), "Overlay: the list of regions must not be empty");

  // A union with a flag: a Tape evaluates it as the union of its operands, the whole object.
  auto node = ExprDetail::makeNode<T>(ExprDetail::Kind::Union, ExprDetail::toExprs(a_operands), true);

  BoundingVolumes::AABBT<T> box;
  DistanceQuality           quality = DistanceQuality::Bound;

  for (const Expr<T>& child : node->m_children) {
    box     = box.merged(child.computeBoundingVolume());
    quality = ExprDetail::worse(quality, child.getDistanceQuality());
  }

  node->m_boundingVolume = box;
  node->m_quality        = quality;

  return Expr<T>(std::move(node));
}

template <class A, class B, class... Rest>
inline Expr<ExprPrecision<A>>
Overlay(const A& a_first, const B& a_second, const Rest&... a_rest)
{
  using T = ExprPrecision<A>;

  static_assert(!std::is_convertible_v<B, std::string> && !(std::is_convertible_v<Rest, std::string> || ...),
                "Overlay: every operand is a region; the outside has no material (make a medium around the object, "
                "such as air, a region of its own, under the others)");
  static_assert(std::is_same_v<T, ExprPrecision<B>> && (std::is_same_v<T, ExprPrecision<Rest>> && ...),
                "Overlay: all operands must have the same precision");

  return Overlay(std::vector<Expr<T>>{toExpr(a_first), toExpr(a_second), toExpr(a_rest)...});
}

inline Expr<double>
Overlay(std::initializer_list<Expr<double>> a_operands)
{
  return Overlay(std::vector<Expr<double>>(a_operands));
}

inline Expr<float>
Overlay(std::initializer_list<Expr<float>> a_operands)
{
  return Overlay(std::vector<Expr<float>>(a_operands));
}

template <class A>
inline Expr<ExprPrecision<A>>
MaterialLabel(const A& a_operand, std::string a_name)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(!a_name.empty(), "MaterialLabel: the name must not be empty");

  // A copy of the node, so that other uses of the same subexpression keep their own material.
  auto node = std::make_shared<ExprDetail::Node<T>>(toExpr(a_operand).node());

  node->m_materialLabel = std::move(a_name);
  node->m_namesRegion   = true;

  return Expr<T>(std::move(node));
}

template <class A>
inline Expr<ExprPrecision<A>>
RegionName(const A& a_operand, std::string a_name)
{
  using T = ExprPrecision<A>;

  EBGEOMETRY_REQUIRE(!a_name.empty(), "RegionName: the name must not be empty");

  // A copy of the node, so that other uses of the same subexpression keep their own name.
  auto node = std::make_shared<ExprDetail::Node<T>>(toExpr(a_operand).node());

  node->m_regionName  = std::move(a_name);
  node->m_namesRegion = true;

  return Expr<T>(std::move(node));
}

namespace ExprDetail {

/**
 * @brief Whether a node or any node below it has a label.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_node Node.
 * @return True if some node is labeled.
 */
template <class T>
[[nodiscard]] inline bool
hasSurfaceLabel(const Node<T>& a_node) noexcept
{
  if (!a_node.m_surfaceLabel.empty() || a_node.m_partGroups != nullptr) {
    return true;
  }

  for (const Expr<T>& child : a_node.m_children) {
    if (hasSurfaceLabel(child.node())) {
      return true;
    }
  }

  return false;
}

/**
 * @brief Whether a node, or one below it, is a container leaf (see hasSurfaces).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_node Node.
 * @return True if one is.
 */
template <class T>
[[nodiscard]] inline bool
hasContainer(const Node<T>& a_node) noexcept
{
  if (a_node.m_leaf.m_numSurfaces != nullptr) {
    return true;
  }

  for (const Expr<T>& child : a_node.m_children) {
    if (hasContainer(child.node())) {
      return true;
    }
  }

  return false;
}

/**
 * @brief What a node does, as ExprIndex::Node::m_operation names it.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_node Node.
 * @return The name.
 */
template <class T>
[[nodiscard]] inline std::string
operationName(const Node<T>& a_node)
{
  const std::string bvh = a_node.m_bvh ? " (BVH)" : "";

  switch (a_node.m_kind) {
  case Kind::Primitive: {
    return "Primitive";
  }
  case Kind::Affine: {
    return "Transform";
  }
  case Kind::Elongate: {
    return "Elongate";
  }
  case Kind::Repeat: {
    return "Repeat";
  }
  case Kind::Complement: {
    return "Complement";
  }
  case Kind::Offset: {
    return "Offset";
  }
  case Kind::Annular: {
    return "Annulus";
  }
  case Kind::Blur: {
    return "Blur";
  }
  case Kind::Mollify: {
    return "Mollify";
  }
  case Kind::Union: {
    return a_node.m_overlay ? std::string("Overlay") : "Union" + bvh;
  }
  case Kind::Intersection: {
    return "Intersection";
  }
  case Kind::SmoothUnion: {
    return "SmoothUnion" + bvh;
  }
  case Kind::SmoothIntersection: {
    return "SmoothIntersection";
  }
  default: {
    return "";
  }
  }
}

/**
 * @brief Bounds of a value over a ball: lo <= v(x) <= hi for every x in it.
 * @details Internal helper; not part of the public API. An unknown side is +-Math::Limits<T>::max().
 * @tparam T Floating-point precision.
 */
template <class T>
struct Bounds
{
  /**
   * @brief Lower bound.
   */
  T m_lo = -Math::Limits<T>::max();

  /**
   * @brief Upper bound.
   */
  T m_hi = Math::Limits<T>::max();
};

/**
 * @brief A shape's value at a point in its frame, for bounds (ExprIndex::checkLabels()): false when it cannot be read.
 * @details Internal helper; not part of the public API.
 */
template <class T>
using LeafValue = std::function<bool(const Node<T>&, const Vec3T<T>&, T&)>;

/**
 * @brief Bounds of a node's value over a ball (ExprIndex::checkLabels()).
 * @details Internal helper; not part of the public API. A shape whose value is a distance or a bound is 1-Lipschitz,
 * so its value at the center, plus or minus the radius, bounds it; operations combine their operands' bounds. Not
 * bounded: Repeat, Blur and Mollify, a shape whose value is not a distance, and one whose value cannot be read. A
 * node with an exact distance whose box the ball misses is not evaluated: its value is at least the distance to the
 * box. Recursive, on the host only. The shapes' values come from a_leafValue (a tape's leaves, Tape::leafValue()),
 * so that the expression holds no evaluation of its own.
 * @tparam T Floating-point precision.
 * @param[in] a_node      Node.
 * @param[in] a_center    The ball's center, in the node's frame.
 * @param[in] a_radius    The ball's radius.
 * @param[in] a_leafValue A shape's value.
 * @return The bounds.
 */
template <class T>
[[nodiscard]] inline Bounds<T>
boundsOver(const Node<T>& a_node, const Vec3T<T>& a_center, const T a_radius, const LeafValue<T>& a_leafValue)
{
  Bounds<T> bounds;

  if (a_node.m_quality == DistanceQuality::Exact && !isEmpty(a_node.m_boundingVolume)) {
    const T gap = a_node.m_boundingVolume.getDistance(a_center) - a_radius;

    if (gap > T(0)) {
      bounds.m_lo = gap;

      return bounds;
    }
  }

  // The operands' bounds.
  std::vector<Bounds<T>> operands;

  const auto all = [&](const Vec3T<T>& a_c, const T a_r) {
    for (const Expr<T>& child : a_node.m_children) {
      operands.push_back(boundsOver(child.node(), a_c, a_r, a_leafValue));
    }
  };

  switch (a_node.m_kind) {
  case Kind::Primitive: {
    T value = T(0);

    if (a_node.m_quality != DistanceQuality::NotADistance && a_leafValue(a_node, a_center, value)) {
      bounds.m_lo = value - a_radius;
      bounds.m_hi = value + a_radius;
    }

    break;
  }
  case Kind::Affine: {
    // The maps are similarities: the child's frame is scaled by 1 / m_valueScale.
    all(a_node.m_map(a_center), a_radius / a_node.m_valueScale);

    bounds.m_lo = operands.front().m_lo * a_node.m_valueScale;
    bounds.m_hi = operands.front().m_hi * a_node.m_valueScale;

    break;
  }
  case Kind::Elongate: {
    // p - clamp(p, -h, h) moves two points no farther apart, so the ball maps into one of the same radius.
    Vec3T<T> q;

    for (size_t dir = 0; dir < 3; dir++) {
      q[dir] = a_center[dir] - Math::clamp(a_center[dir], -a_node.m_vec0[dir], a_node.m_vec0[dir]);
    }

    all(q, a_radius);

    bounds = operands.front();

    break;
  }
  case Kind::Complement: {
    all(a_center, a_radius);

    bounds.m_lo = -operands.front().m_hi;
    bounds.m_hi = -operands.front().m_lo;

    break;
  }
  case Kind::Offset: {
    all(a_center, a_radius);

    bounds.m_lo = operands.front().m_lo - a_node.m_scalar0;
    bounds.m_hi = operands.front().m_hi - a_node.m_scalar0;

    break;
  }
  case Kind::Annular: {
    all(a_center, a_radius);

    const Bounds<T>& v = operands.front();

    // |v| - delta.
    const T lo = (v.m_lo >= T(0)) ? v.m_lo : ((v.m_hi <= T(0)) ? -v.m_hi : T(0));
    const T hi = Math::max(std::abs(v.m_lo), std::abs(v.m_hi));

    bounds.m_lo = lo - a_node.m_scalar0;
    bounds.m_hi = hi - a_node.m_scalar0;

    break;
  }
  case Kind::Union:
  case Kind::SmoothUnion: {
    all(a_center, a_radius);

    bounds.m_lo = Math::Limits<T>::max();
    bounds.m_hi = Math::Limits<T>::max();

    for (const Bounds<T>& v : operands) {
      bounds.m_lo = Math::min(bounds.m_lo, v.m_lo);
      bounds.m_hi = Math::min(bounds.m_hi, v.m_hi);
    }

    // A smooth minimum is at most the minimum, by an amount the bounds do not track.
    if (a_node.m_kind == Kind::SmoothUnion) {
      bounds.m_lo = -Math::Limits<T>::max();
    }

    break;
  }
  case Kind::Intersection:
  case Kind::SmoothIntersection: {
    all(a_center, a_radius);

    bounds.m_lo = -Math::Limits<T>::max();
    bounds.m_hi = -Math::Limits<T>::max();

    for (const Bounds<T>& v : operands) {
      bounds.m_lo = Math::max(bounds.m_lo, v.m_lo);
      bounds.m_hi = Math::max(bounds.m_hi, v.m_hi);
    }

    if (a_node.m_kind == Kind::SmoothIntersection) {
      bounds.m_hi = Math::Limits<T>::max();
    }

    break;
  }
  default: {
    break;
  }
  }

  return bounds;
}

/**
 * @brief Whether a box is proved strictly inside other operands of a node (a union, or an Overlay's later operands)
 * or strictly outside one (an intersection), other than the one it comes from (ExprIndex::checkLabels()).
 * @details Internal helper; not part of the public API. The ball around the box is tested against each other
 * operand; a box no operand decides on whole is split in eight, each piece tested again, a_depth times.
 * @tparam T Floating-point precision.
 * @param[in] a_node      A union, an Overlay or an intersection.
 * @param[in] a_skip      The operand the box comes from.
 * @param[in] a_box       The box, in the node's frame.
 * @param[in] a_depth     How many more times the box may be split.
 * @param[in] a_leafValue A shape's value.
 * @return True if proved.
 */
template <class T>
[[nodiscard]] inline bool
coveredBySibling(const Node<T>&                   a_node,
                 const size_t                     a_skip,
                 const BoundingVolumes::AABBT<T>& a_box,
                 const int                        a_depth,
                 const LeafValue<T>&              a_leafValue)
{
  const bool     isUnion = a_node.m_kind == Kind::Union;
  const Vec3T<T> center  = a_box.getCentroid();
  const T        radius  = T(0.5) * (a_box.getHighCorner() - a_box.getLowCorner()).length();

  // In an Overlay, only a later operand covers: an earlier one lies under it.
  for (size_t i = a_node.m_overlay ? a_skip + 1 : 0; i < a_node.m_children.size(); i++) {
    if (i == a_skip) {
      continue;
    }

    const Bounds<T> bounds = boundsOver(a_node.m_children[i].node(), center, radius, a_leafValue);

    if (isUnion ? (bounds.m_hi < T(0)) : (bounds.m_lo > T(0))) {
      return true;
    }
  }

  if (a_depth == 0) {
    return false;
  }

  // Each eighth of the box may be decided by another operand.
  for (int octant = 0; octant < 8; octant++) {
    Vec3T<T> lo;
    Vec3T<T> hi;

    for (size_t dir = 0; dir < 3; dir++) {
      const bool upper = ((octant >> dir) & 1) != 0;

      lo[dir] = upper ? center[dir] : a_box.getLowCorner()[dir];
      hi[dir] = upper ? a_box.getHighCorner()[dir] : center[dir];
    }

    if (!coveredBySibling(a_node, a_skip, BoundingVolumes::AABBT<T>(lo, hi), a_depth - 1, a_leafValue)) {
      return false;
    }
  }

  return true;
}

} // namespace ExprDetail

template <class T>
inline ExprIndex<T>::ExprIndex(const Expr<T>& a_expr) : m_leafSurfaces{0U}
{
  const ExprDetail::Node<T>& root = a_expr.node();

  // The background is region 0, with no material.
  m_regions.push_back(Region());
  m_regionBoxes.push_back(BoundingVolumes::AABBT<T>());

  this->add(root, None, SurfaceLabelId::none());

  m_leafRegions.assign(m_leafNodes.size(), None);

  this->addRegions(0U, None, MaterialLabelId::none());

  this->addSides();

  m_masks.clear();

  // The expression's nodes are not kept: the index does not hold the expression.
  m_exprNodes.clear();
  m_exprNodes.shrink_to_fit();
}

template <class T>
inline MaterialLabelId
ExprIndex<T>::internMaterial(const std::string& a_name)
{
  const auto [entry, added] =
    m_materialLabelIds.emplace(a_name, MaterialLabelId(static_cast<uint32_t>(m_materialLabelNames.size())));

  if (added) {
    m_materialLabelNames.push_back(a_name);
  }

  return entry->second;
}

template <class T>
inline void
ExprIndex<T>::addRegions(const uint32_t a_node, const uint32_t a_group, const MaterialLabelId a_material)
{
  const ExprDetail::Node<T>& expr = *m_exprNodes[a_node];

  // An operand that is not a partition is a region: its material is the first along its maps, or its group's.
  if (!expr.m_partition) {
    Region region;

    region.m_node        = a_node;
    region.m_regionGroup = a_group;
    region.m_material    = a_material;

    bool hasMaterial = false;

    for (uint32_t node = a_node;; node = m_operands[node].front()) {
      if (!hasMaterial && !m_nodes[node].m_materialLabel.isNone()) {
        region.m_material = m_nodes[node].m_materialLabel;
        hasMaterial       = true;
      }

      if (region.m_name.empty()) {
        region.m_name = m_exprNodes[node]->m_regionName;
      }

      if (m_exprNodes[node]->m_kind != ExprDetail::Kind::Affine) {
        break;
      }
    }

    EBGEOMETRY_REQUIRE(region.m_name.empty() || this->findRegion(region.m_name) == None,
                       "ExprIndex: two regions are named \"%s\"; a region's name says which region it is",
                       region.m_name.c_str());

    const auto number = static_cast<uint32_t>(m_regions.size());

    m_regions.push_back(region);
    m_regionBoxes.push_back(this->sceneBox(expr.m_boundingVolume));

    for (uint32_t leaf = 0; leaf < m_nodes[a_node].m_numLeaves; leaf++) {
      m_leafRegions[m_nodes[a_node].m_firstLeaf + leaf] = number;
    }

    return;
  }

  const MaterialLabelId material =
    m_nodes[a_node].m_materialLabel.isNone() ? a_material : m_nodes[a_node].m_materialLabel;

  if (expr.m_overlay) {
    RegionGroup group;

    group.m_node        = a_node;
    group.m_parent      = a_group;
    group.m_firstRegion = static_cast<uint32_t>(m_regions.size());
    group.m_material    = material;
    group.m_name        = expr.m_regionName;

    for (const RegionGroup& other : m_regionGroups) {
      EBGEOMETRY_REQUIRE(group.m_name.empty() || other.m_name != group.m_name,
                         "ExprIndex: two region groups are named \"%s\"",
                         group.m_name.c_str());
    }

    const auto number = static_cast<uint32_t>(m_regionGroups.size());

    m_regionGroups.push_back(group);

    for (const uint32_t operand : m_operands[a_node]) {
      this->addRegions(operand, number, material);
    }

    m_regionGroups[number].m_numRegions =
      static_cast<uint32_t>(m_regions.size()) - m_regionGroups[number].m_firstRegion;
  }
  else {
    // A map of a partition, or one clipped by masks: the partition is its first operand, the masks belong to no
    // region. Below a map, boxes are in the map's frame; a mask's operands are recorded with the regions they cut.
    const bool map = expr.m_kind == ExprDetail::Kind::Affine;

    if (map) {
      m_maps.push_back(&expr);
    }

    const auto first = static_cast<uint32_t>(m_regions.size());

    this->addRegions(m_operands[a_node].front(), a_group, material);

    if (map) {
      m_maps.pop_back();
    }
    else {
      for (size_t i = 1; i < m_operands[a_node].size(); i++) {
        const uint32_t operand = m_operands[a_node][i];

        m_masks.push_back(Mask{operand,
                               this->sceneBox(m_exprNodes[operand]->m_boundingVolume),
                               first,
                               static_cast<uint32_t>(m_regions.size()) - first});
      }
    }
  }
}

template <class T>
inline BoundingVolumes::AABBT<T>
ExprIndex<T>::sceneBox(const BoundingVolumes::AABBT<T>& a_box) const
{
  BoundingVolumes::AABBT<T> box = a_box;

  for (auto map = m_maps.rbegin(); map != m_maps.rend(); ++map) {
    box = (*map)->m_inverse.transform(box);
  }

  return box;
}

template <class T>
inline void
ExprIndex<T>::addSides()
{
  // Whether two boxes share a point (touching counts: two regions may meet on a face).
  const auto overlap = [](const BoundingVolumes::AABBT<T>& a_first, const BoundingVolumes::AABBT<T>& a_second) {
    for (size_t dir = 0; dir < 3; dir++) {
      if (a_first.getLowCorner()[dir] > a_second.getHighCorner()[dir] ||
          a_second.getLowCorner()[dir] > a_first.getHighCorner()[dir]) {
        return false;
      }
    }

    return true;
  };

  const auto pair = [](const uint32_t a_first, const uint32_t a_second) {
    return std::make_pair(Math::min(a_first, a_second), Math::max(a_first, a_second));
  };

  m_leafSides.assign(m_leafNodes.size(), {});

  // A region's own surface separates it from the background and from an earlier region it may meet: a later one
  // covers it there, and the interface is the later region's surface.
  for (uint32_t k = 1; k < m_regions.size(); k++) {
    std::vector<std::pair<uint32_t, uint32_t>> sides{pair(k, 0U)};

    for (uint32_t m = 1; m < k; m++) {
      if (overlap(m_regionBoxes[k], m_regionBoxes[m])) {
        sides.push_back(pair(k, m));
      }
    }

    const Node& node = m_nodes[m_regions[k].m_node];

    for (uint32_t leaf = node.m_firstLeaf; leaf < node.m_firstLeaf + node.m_numLeaves; leaf++) {
      m_leafSides[leaf] = sides;
    }
  }

  // A mask's surface separates a region it cuts from what the cut leaves there: the background, or a region outside
  // the masked Overlay that it may meet.
  for (const Mask& mask : m_masks) {
    std::vector<std::pair<uint32_t, uint32_t>> sides;

    for (uint32_t r = mask.m_firstRegion; r < mask.m_firstRegion + mask.m_numRegions; r++) {
      if (!overlap(m_regionBoxes[r], mask.m_box)) {
        continue;
      }

      sides.push_back(pair(r, 0U));

      for (uint32_t m = 1; m < m_regions.size(); m++) {
        const bool cut = m >= mask.m_firstRegion && m < mask.m_firstRegion + mask.m_numRegions;

        if (!cut && overlap(m_regionBoxes[m], mask.m_box)) {
          sides.push_back(pair(r, m));
        }
      }
    }

    const Node& node = m_nodes[mask.m_node];

    for (uint32_t leaf = node.m_firstLeaf; leaf < node.m_firstLeaf + node.m_numLeaves; leaf++) {
      m_leafSides[leaf].insert(m_leafSides[leaf].end(), sides.begin(), sides.end());
    }
  }

  for (auto& sides : m_leafSides) {
    std::sort(sides.begin(), sides.end());

    sides.erase(std::unique(sides.begin(), sides.end()), sides.end());
  }
}

template <class T>
inline const std::vector<std::pair<uint32_t, uint32_t>>&
ExprIndex<T>::getSidesOfLeaf(const uint32_t a_leaf) const
{
  EBGEOMETRY_REQUIRE(a_leaf < m_leafSides.size(), "ExprIndex: leaf %u of %zu", unsigned(a_leaf), m_leafSides.size());

  return m_leafSides[a_leaf];
}

template <class T>
inline std::vector<std::pair<uint32_t, uint32_t>>
ExprIndex<T>::getInterfaces() const
{
  std::vector<std::pair<uint32_t, uint32_t>> all;

  for (const auto& sides : m_leafSides) {
    all.insert(all.end(), sides.begin(), sides.end());
  }

  std::sort(all.begin(), all.end());

  all.erase(std::unique(all.begin(), all.end()), all.end());

  return all;
}

template <class T>
inline void
ExprIndex<T>::add(const ExprDetail::Node<T>& a_node,
                  const uint32_t             a_parent,
                  const SurfaceLabelId       a_label,
                  const bool                 a_object)
{
  const auto id = static_cast<uint32_t>(m_nodes.size());

  // A name's label: the next number for a name not seen before.
  const auto intern = [this](const std::string& a_name) {
    const auto [entry, added] =
      m_surfaceLabelIds.emplace(a_name, SurfaceLabelId(static_cast<uint32_t>(m_surfaceLabelNames.size())));

    if (added) {
      m_surfaceLabelNames.push_back(a_name);
    }

    return entry->second;
  };

  // A label on this very object (above it with only maps between) relabels it, as a material does, and the replaced
  // name gets no number of its own; one on a composition above is a default that a label of its own wins over.
  const bool           relabeled = a_object && !a_label.isNone();
  const SurfaceLabelId own =
    (relabeled || a_node.m_surfaceLabel.empty()) ? SurfaceLabelId::none() : intern(a_node.m_surfaceLabel);
  const SurfaceLabelId label = own.isNone() ? a_label : own;

  // What the operands inherit names the same object only below a map.
  const bool object = (a_node.m_kind == ExprDetail::Kind::Affine) && (relabeled || !own.isNone());

  Node node;

  node.m_parent       = a_parent;
  node.m_firstLeaf    = static_cast<uint32_t>(m_leafNodes.size());
  node.m_firstSurface = m_leafSurfaces.back();
  node.m_surfaceLabel = own;
  node.m_operation    = ExprDetail::operationName(a_node);

  if (!a_node.m_materialLabel.empty()) {
    node.m_materialLabel = this->internMaterial(a_node.m_materialLabel);
  }

  m_nodes.push_back(std::move(node));
  m_exprNodes.push_back(&a_node);
  m_operands.emplace_back();

  if (a_parent != None) {
    m_operands[a_parent].push_back(id);
  }

  // Leaves are numbered in the order the tape compiler meets them: depth first, the operands in order.
  if (a_node.m_kind == ExprDetail::Kind::Primitive) {
    const ExprDetail::Leaf& leaf = a_node.m_leaf;

    const uint64_t surfaces = (leaf.m_numSurfaces != nullptr) ? leaf.m_numSurfaces(leaf.m_object.get()) : 1U;
    const uint64_t end      = uint64_t(m_leafSurfaces.back()) + surfaces;

    EBGEOMETRY_REQUIRE(end < uint64_t(None),
                       "ExprIndex: the expression holds %llu surfaces, more than 32-bit ids can number",
                       static_cast<unsigned long long>(end));

    m_leafNodes.push_back(id);
    m_leafSurfaceLabels.push_back(label);
    m_leafSurfaces.push_back(static_cast<uint32_t>(end));
    m_leafPartSurfaceLabels.emplace_back();

    // A collection's parts are its members' (not counted here); a shape with parts has its own count, others one.
    m_leafParts.push_back((leaf.m_numSurfaces != nullptr) ? 0U
                          : (leaf.m_numParts != nullptr)  ? leaf.m_numParts(leaf.m_object.get())
                                                          : 1U);

    // The parts' labels: a part's group wins over the label the leaf has.
    if (a_node.m_partGroups != nullptr) {
      const FaceGroups& groups = *a_node.m_partGroups;

      std::vector<SurfaceLabelId> named;

      for (const std::string& name : groups.m_names) {
        named.push_back(intern(name));
      }

      std::vector<SurfaceLabelId>& parts = m_leafPartSurfaceLabels.back();

      parts.reserve(groups.m_group.size());

      for (const uint32_t group : groups.m_group) {
        parts.push_back((group == FaceGroups::None) ? label : named[group]);
      }
    }
  }

  for (const Expr<T>& child : a_node.m_children) {
    this->add(child.node(), id, label, object);
  }

  m_nodes[id].m_numLeaves   = static_cast<uint32_t>(m_leafNodes.size()) - m_nodes[id].m_firstLeaf;
  m_nodes[id].m_numSurfaces = m_leafSurfaces.back() - m_nodes[id].m_firstSurface;
}

template <class T>
inline uint32_t
ExprIndex<T>::getNumNodes() const noexcept
{
  return static_cast<uint32_t>(m_nodes.size());
}

template <class T>
inline const typename ExprIndex<T>::Node&
ExprIndex<T>::getNode(const uint32_t a_node) const
{
  EBGEOMETRY_REQUIRE(a_node < m_nodes.size(), "ExprIndex: node %u of %zu", unsigned(a_node), m_nodes.size());

  return m_nodes[a_node];
}

template <class T>
inline uint32_t
ExprIndex<T>::getNumLeaves() const noexcept
{
  return static_cast<uint32_t>(m_leafNodes.size());
}

template <class T>
inline uint32_t
ExprIndex<T>::getLeafNode(const uint32_t a_leaf) const
{
  EBGEOMETRY_REQUIRE(a_leaf < m_leafNodes.size(), "ExprIndex: leaf %u of %zu", unsigned(a_leaf), m_leafNodes.size());

  return m_leafNodes[a_leaf];
}

template <class T>
inline std::vector<uint32_t>
ExprIndex<T>::pathOf(const uint32_t a_leaf) const
{
  std::vector<uint32_t> path;

  for (uint32_t node = this->getLeafNode(a_leaf); node != None; node = m_nodes[node].m_parent) {
    path.push_back(node);
  }

  return std::vector<uint32_t>(path.rbegin(), path.rend());
}

template <class T>
inline SurfaceLabelId
ExprIndex<T>::getPartSurfaceLabel(const uint32_t a_leaf, const uint32_t a_part) const
{
  const std::vector<SurfaceLabelId>& parts = this->getPartSurfaceLabels(a_leaf);

  if (parts.empty()) {
    return m_leafSurfaceLabels[a_leaf];
  }

  EBGEOMETRY_REQUIRE(a_part < parts.size(), "ExprIndex: part %u of %zu", unsigned(a_part), parts.size());

  return parts[a_part];
}

template <class T>
inline uint32_t
ExprIndex<T>::getNumPartsOfLeaf(const uint32_t a_leaf) const
{
  EBGEOMETRY_REQUIRE(a_leaf < m_leafParts.size(), "ExprIndex: leaf %u of %zu", unsigned(a_leaf), m_leafParts.size());

  return m_leafParts[a_leaf];
}

template <class T>
inline const std::vector<SurfaceLabelId>&
ExprIndex<T>::getPartSurfaceLabels(const uint32_t a_leaf) const
{
  EBGEOMETRY_REQUIRE(a_leaf < m_leafPartSurfaceLabels.size(),
                     "ExprIndex: leaf %u of %zu",
                     unsigned(a_leaf),
                     m_leafPartSurfaceLabels.size());

  return m_leafPartSurfaceLabels[a_leaf];
}

template <class T>
inline uint32_t
ExprIndex<T>::getNumSurfaces() const noexcept
{
  return m_leafSurfaces.back();
}

template <class T>
inline uint32_t
ExprIndex<T>::getFirstSurface(const uint32_t a_leaf) const
{
  EBGEOMETRY_REQUIRE(
    a_leaf < m_leafSurfaces.size(), "ExprIndex: leaf %u of %zu", unsigned(a_leaf), m_leafSurfaces.size() - 1);

  return m_leafSurfaces[a_leaf];
}

template <class T>
inline uint32_t
ExprIndex<T>::getLeafOfSurface(const uint32_t a_surface) const
{
  EBGEOMETRY_REQUIRE(a_surface < this->getNumSurfaces(),
                     "ExprIndex: surface %u of %u",
                     unsigned(a_surface),
                     unsigned(this->getNumSurfaces()));

  // The last leaf starting at or before the surface: a leaf with no surfaces (an empty cloud) starts where the
  // next one does, and is passed over.
  const auto next = std::upper_bound(m_leafSurfaces.begin(), m_leafSurfaces.end(), a_surface);

  return static_cast<uint32_t>(std::distance(m_leafSurfaces.begin(), next)) - 1U;
}

template <class T>
inline SurfaceLabelId
ExprIndex<T>::getSurfaceLabel(const uint32_t a_leaf) const
{
  EBGEOMETRY_REQUIRE(
    a_leaf < m_leafSurfaceLabels.size(), "ExprIndex: leaf %u of %zu", unsigned(a_leaf), m_leafSurfaceLabels.size());

  return m_leafSurfaceLabels[a_leaf];
}

template <class T>
inline uint32_t
ExprIndex<T>::getNumSurfaceLabels() const noexcept
{
  return static_cast<uint32_t>(m_surfaceLabelNames.size());
}

template <class T>
inline const std::string&
ExprIndex<T>::getSurfaceLabelName(const SurfaceLabelId a_label) const
{
  EBGEOMETRY_REQUIRE(a_label.value() < m_surfaceLabelNames.size(),
                     "ExprIndex: surface label %u of %zu",
                     unsigned(a_label.value()),
                     m_surfaceLabelNames.size());

  return m_surfaceLabelNames[a_label.value()];
}

template <class T>
inline SurfaceLabelId
ExprIndex<T>::findSurfaceLabel(const std::string_view a_name) const
{
  const auto entry = m_surfaceLabelIds.find(std::string(a_name));

  return (entry != m_surfaceLabelIds.end()) ? entry->second : SurfaceLabelId::none();
}

template <class T>
inline uint32_t
ExprIndex<T>::getNumRegions() const noexcept
{
  return static_cast<uint32_t>(m_regions.size());
}

template <class T>
inline const typename ExprIndex<T>::Region&
ExprIndex<T>::getRegion(const uint32_t a_region) const
{
  EBGEOMETRY_REQUIRE(a_region < m_regions.size(), "ExprIndex: region %u of %zu", unsigned(a_region), m_regions.size());

  return m_regions[a_region];
}

template <class T>
inline uint32_t
ExprIndex<T>::getRegionOfLeaf(const uint32_t a_leaf) const
{
  EBGEOMETRY_REQUIRE(
    a_leaf < m_leafRegions.size(), "ExprIndex: leaf %u of %zu", unsigned(a_leaf), m_leafRegions.size());

  return m_leafRegions[a_leaf];
}

template <class T>
inline MaterialLabelId
ExprIndex<T>::getMaterialOf(const uint32_t a_region) const
{
  return this->getRegion(a_region).m_material;
}

template <class T>
inline std::vector<uint32_t>
ExprIndex<T>::getRegionsOfRegionGroup(const uint32_t a_group) const
{
  const RegionGroup& group = this->getRegionGroup(a_group);

  std::vector<uint32_t> regions(group.m_numRegions);

  for (uint32_t i = 0; i < group.m_numRegions; i++) {
    regions[i] = group.m_firstRegion + i;
  }

  return regions;
}

template <class T>
inline std::vector<uint32_t>
ExprIndex<T>::getRegionsOfMaterial(const MaterialLabelId a_material) const
{
  std::vector<uint32_t> regions;

  for (uint32_t k = 0; k < m_regions.size(); k++) {
    if (!a_material.isNone() && m_regions[k].m_material == a_material) {
      regions.push_back(k);
    }
  }

  return regions;
}

template <class T>
inline uint32_t
ExprIndex<T>::getNumRegionGroups() const noexcept
{
  return static_cast<uint32_t>(m_regionGroups.size());
}

template <class T>
inline const typename ExprIndex<T>::RegionGroup&
ExprIndex<T>::getRegionGroup(const uint32_t a_group) const
{
  EBGEOMETRY_REQUIRE(
    a_group < m_regionGroups.size(), "ExprIndex: region group %u of %zu", unsigned(a_group), m_regionGroups.size());

  return m_regionGroups[a_group];
}

template <class T>
inline uint32_t
ExprIndex<T>::getNumMaterialLabels() const noexcept
{
  return static_cast<uint32_t>(m_materialLabelNames.size());
}

template <class T>
inline const std::string&
ExprIndex<T>::getMaterialLabelName(const MaterialLabelId a_label) const
{
  EBGEOMETRY_REQUIRE(a_label.value() < m_materialLabelNames.size(),
                     "ExprIndex: material label %u of %zu",
                     unsigned(a_label.value()),
                     m_materialLabelNames.size());

  return m_materialLabelNames[a_label.value()];
}

template <class T>
inline MaterialLabelId
ExprIndex<T>::findMaterialLabel(const std::string_view a_name) const
{
  const auto entry = m_materialLabelIds.find(std::string(a_name));

  return (entry != m_materialLabelIds.end()) ? entry->second : MaterialLabelId::none();
}

template <class T>
inline std::string
ExprIndex<T>::whyHidden(const uint32_t                                                              a_node,
                        const std::vector<const ExprDetail::Node<T>*>&                              a_nodes,
                        const std::function<bool(const ExprDetail::Node<T>&, const Vec3T<T>&, T&)>& a_leafValue) const
{
  using Kind = ExprDetail::Kind;

  // An operation on the values (not on the sets) moves the boundary in ways the box does not follow.
  for (uint32_t node = m_nodes[a_node].m_parent; node != None; node = m_nodes[node].m_parent) {
    switch (a_nodes[node]->m_kind) {
    case Kind::Offset:
    case Kind::Annular:
    case Kind::Blur:
    case Kind::Mollify:
    case Kind::SmoothUnion:
    case Kind::SmoothIntersection: {
      return "";
    }
    default: {
      break;
    }
    }
  }

  BoundingVolumes::AABBT<T> box = a_nodes[a_node]->m_boundingVolume;

  if (ExprDetail::isEmpty(box)) {
    return "it is empty";
  }

  // The box, carried up in each node's frame, tested against the other operands of each union, Overlay and
  // intersection on the way.
  for (uint32_t child = a_node, node = m_nodes[a_node].m_parent; node != None;
       child = node, node = m_nodes[node].m_parent) {
    for (size_t dir = 0; dir < 3; dir++) {
      if (!(std::abs(box.getLowCorner()[dir]) < Math::Limits<T>::max()) ||
          !(std::abs(box.getHighCorner()[dir]) < Math::Limits<T>::max())) {
        return "";
      }
    }

    const ExprDetail::Node<T>& expr = *a_nodes[node];

    switch (expr.m_kind) {
    case Kind::Affine: {
      box = expr.m_inverse.transform(box);

      break;
    }
    case Kind::Elongate: {
      box = ExprDetail::grown(box, expr.m_vec0);

      break;
    }
    case Kind::Repeat: {
      Vec3T<T> lo = box.getLowCorner();
      Vec3T<T> hi = box.getHighCorner();

      for (size_t dir = 0; dir < 3; dir++) {
        lo[dir] -= expr.m_vec0[dir] * expr.m_vec1[dir];
        hi[dir] += expr.m_vec0[dir] * expr.m_vec2[dir];
      }

      box = BoundingVolumes::AABBT<T>(lo, hi);

      break;
    }
    case Kind::Union:
    case Kind::Intersection: {
      const std::vector<uint32_t>& operands = m_operands[node];

      const auto operand = static_cast<size_t>(std::find(operands.begin(), operands.end(), child) - operands.begin());

      if (ExprDetail::coveredBySibling(expr, operand, box, 2, a_leafValue)) {
        return expr.m_overlay ? "it lies under later regions of the Overlay at node " + std::to_string(node)
               : (expr.m_kind == Kind::Union)
                 ? "it lies inside another operand of the union at node " + std::to_string(node)
                 : "it lies outside another operand of the intersection at node " + std::to_string(node);
      }

      break;
    }
    default: {
      break;
    }
    }
  }

  return "";
}

template <class T>
template <class TapeType>
inline std::vector<std::string>
ExprIndex<T>::checkLabels(const Expr<T>& a_expr, const TapeType& a_tape) const
{
  // The expression's nodes, numbered as the index numbers them: depth first, the operands in order.
  std::vector<const ExprDetail::Node<T>*> nodes;

  const std::function<void(const ExprDetail::Node<T>&)> number = [&nodes, &number](const ExprDetail::Node<T>& a_node) {
    nodes.push_back(&a_node);

    for (const Expr<T>& child : a_node.m_children) {
      number(child.node());
    }
  };

  number(a_expr.node());

  EBGEOMETRY_REQUIRE(nodes.size() == m_nodes.size(),
                     "ExprIndex: checkLabels was given an expression of %zu nodes, the index's has %zu",
                     nodes.size(),
                     m_nodes.size());

  // A shape's value through the tape, by its leaf number.
  std::unordered_map<const ExprDetail::Node<T>*, uint32_t> leaves;

  for (uint32_t leaf = 0; leaf < this->getNumLeaves(); leaf++) {
    leaves.emplace(nodes[m_leafNodes[leaf]], leaf);
  }

  const std::function<bool(const ExprDetail::Node<T>&, const Vec3T<T>&, T&)> leafValue =
    [&leaves, &a_tape](const ExprDetail::Node<T>& a_node, const Vec3T<T>& a_point, T& a_value) {
      const auto found = leaves.find(&a_node);

      return found != leaves.end() && a_tape.leafValue(found->second, a_point, a_value);
    };

  std::vector<std::string> warnings;

  // Surface labels: lost when every leaf (or part) carrying one is.
  for (uint32_t label = 0; label < this->getNumSurfaceLabels(); label++) {
    const SurfaceLabelId id(label);
    const std::string    name = "surface label \"" + this->getSurfaceLabelName(id) + "\"";

    std::vector<std::string> reasons;

    size_t carriers = 0;

    for (uint32_t leaf = 0; leaf < this->getNumLeaves(); leaf++) {
      const std::vector<SurfaceLabelId>& parts = m_leafPartSurfaceLabels[leaf];

      const bool carries =
        parts.empty() ? (m_leafSurfaceLabels[leaf] == id) : (std::find(parts.begin(), parts.end(), id) != parts.end());

      if (carries) {
        carriers++;

        const std::string reason = this->whyHidden(m_leafNodes[leaf], nodes, leafValue);

        if (reason.empty()) {
          break;
        }

        reasons.push_back("leaf " + std::to_string(leaf) + ": " + reason);
      }
    }

    if (carriers == 0) {
      warnings.push_back(name + " is on no surface: every leaf below it has a label of its own");
    }
    else if (reasons.size() == carriers) {
      std::string warning = name + " cannot reach the boundary: ";

      for (size_t i = 0; i < reasons.size() && i < 3; i++) {
        warning += (i > 0 ? "; " : "") + reasons[i];
      }

      if (reasons.size() > 3) {
        warning += "; and " + std::to_string(reasons.size() - 3) + " more leaves";
      }

      warnings.push_back(std::move(warning));
    }
  }

  // Regions: empty when their operand is covered by later ones or cut away.
  std::vector<bool> empty(m_regions.size(), false);

  for (uint32_t k = 1; k < m_regions.size(); k++) {
    if (m_regions[k].m_node == None) {
      continue;
    }

    const std::string reason = this->whyHidden(m_regions[k].m_node, nodes, leafValue);

    if (!reason.empty()) {
      empty[k] = true;

      warnings.push_back("region " + std::to_string(k) +
                         (m_regions[k].m_name.empty() ? std::string() : " \"" + m_regions[k].m_name + "\"") +
                         " has no volume: " + reason);
    }
  }

  // Materials: lost when every region of one is empty.
  for (uint32_t material = 0; material < this->getNumMaterialLabels(); material++) {
    const std::vector<uint32_t> regions = this->getRegionsOfMaterial(MaterialLabelId(material));

    bool lost = !regions.empty();

    for (const uint32_t k : regions) {
      lost = lost && k < empty.size() && empty[k];
    }

    if (lost) {
      warnings.push_back("material \"" + this->getMaterialLabelName(MaterialLabelId(material)) +
                         "\" is on no region with volume");
    }
  }

  return warnings;
}

template <class T>
inline uint32_t
ExprIndex<T>::findRegion(const std::string_view a_name) const
{
  for (uint32_t k = 0; k < m_regions.size(); k++) {
    if (!a_name.empty() && m_regions[k].m_name == a_name) {
      return k;
    }
  }

  return None;
}

} // namespace EBGeometry

#endif
