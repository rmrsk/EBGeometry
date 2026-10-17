// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Expr.hpp
 * @brief  The tape's front end: Expr, a host-side value that describes a composed object, and the free
 * functions that build it (Union, SmoothUnion, Intersection, Difference, Translate, Rotate, ...).
 * @details An Expr is an immutable tree of shapes and operations. It is cheap to copy (copies share the
 * tree) and is never evaluated itself: a Tape compiles it into a pool, and the tape is what is evaluated,
 * on the host or on a device. Any trivially copyable type with signedDistance() and
 * computeBoundingVolume() is a shape (isShape) and converts to an Expr.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_EXPR_HPP
#define EBGEOMETRY_EXPR_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Affine.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief How a smooth union, intersection or difference blends the two extreme values.
 */
enum class BlendKind : uint8_t
{
  Polynomial, ///< SmoothMinOp/SmoothMaxOp: quadratic blend over a band of width s; deepens by at most s/4.
  Exponential ///< ExpMinOp/ExpMaxOp: log-sum-exp blend with length scale s; deepens by at most s ln 2.
};

namespace ExprDetail {

/**
 * @brief The precision of a bounding box type: T for AABBT<T>, nothing otherwise.
 * @details Internal helper; not part of the public API.
 * @tparam B Bounding box type.
 */
template <class B>
struct BoxPrecision
{
};

/**
 * @brief The precision of AABBT<T>.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 */
template <class T>
struct BoxPrecision<BoundingVolumes::AABBT<T>>
{
  /**
   * @brief The precision.
   */
  using type = T;
};

/**
 * @brief Whether S has a const signedDistance(const Vec3T<T>&) returning something convertible to T.
 * @details Internal helper; not part of the public API.
 * @tparam S Candidate shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct HasSignedDistance : std::false_type
{
};

/**
 * @brief Specialization for types that have signedDistance().
 * @details Internal helper; not part of the public API.
 * @tparam S Candidate shape type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct HasSignedDistance<
  S,
  T,
  std::void_t<decltype(std::declval<const S&>().signedDistance(std::declval<const Vec3T<T>&>()))>>
  : std::is_convertible<decltype(std::declval<const S&>().signedDistance(std::declval<const Vec3T<T>&>())), T>
{
};

/**
 * @brief Shape detection: a type with computeBoundingVolume() returning AABBT<T>.
 * @details Internal helper; not part of the public API. The primary template is a non-shape.
 * @tparam S Candidate shape type.
 */
template <class S, class = void>
struct ShapeInfo
{
  /**
   * @brief Whether S is a shape.
   */
  static constexpr bool value = false;
};

/**
 * @brief Shape detection for types with computeBoundingVolume().
 * @details Internal helper; not part of the public API.
 * @tparam S Candidate shape type.
 */
template <class S>
struct ShapeInfo<
  S,
  std::void_t<typename BoxPrecision<std::decay_t<decltype(std::declval<const S&>().computeBoundingVolume())>>::type>>
{
  /**
   * @brief The shape's precision, read from its bounding box type.
   */
  using precision =
    typename BoxPrecision<std::decay_t<decltype(std::declval<const S&>().computeBoundingVolume())>>::type;

  /**
   * @brief Whether S is a shape: trivially copyable, with signedDistance() as well.
   */
  static constexpr bool value = std::is_trivially_copyable_v<S> && HasSignedDistance<S, precision>::value;
};

} // namespace ExprDetail

/**
 * @brief Whether S is a shape: a trivially copyable type with a const `signedDistance(const Vec3T<T>&)`
 * returning T and a const `computeBoundingVolume()` returning `BoundingVolumes::AABBT<T>`.
 * @details The analytic shapes, the mesh SDFs, the BVH unions and Tape are shapes. A shape may also have
 * a static constexpr `distanceQuality` member (see DistanceQuality); without one it counts as
 * DistanceQuality::Bound.
 *
 * Every shape's value goes by the one name `signedDistance`, whatever its distance quality, and the
 * two together say what the value means. The sign is the contract every shape keeps: negative inside,
 * positive outside, zero on the surface. The magnitude is what the distance quality says: the distance
 * to the surface (Exact), at most that distance (Bound), or nothing (NotADistance).
 * @tparam S Candidate type.
 */
template <class S>
inline constexpr bool isShape = ExprDetail::ShapeInfo<S>::value;

/**
 * @brief The precision of a shape type: T for a shape whose box type is AABBT<T>.
 * @tparam S Shape type.
 */
template <class S>
using ShapePrecision = typename ExprDetail::ShapeInfo<S>::precision;

template <class T>
class Expr;

namespace ExprDetail {

/**
 * @brief What an expression node does.
 * @details Internal helper; not part of the public API. Difference and SmoothDifference are built from
 * Intersection, SmoothIntersection, Complement and Union, so they have no kind of their own.
 */
enum class Kind : uint8_t
{
  Primitive,          ///< A shape.
  Affine,             ///< Evaluate the child at m_map(p) and multiply by m_valueScale.
  Elongate,           ///< Evaluate the child at p - clamp(p, -h, h).
  Repeat,             ///< Evaluate the child at p folded into the home tile.
  Complement,         ///< -v.
  Offset,             ///< v - r.
  Annular,            ///< |v| - delta.
  Blur,               ///< 27-tap weighted average of the child.
  Mollify,            ///< n^3-tap convolution of the child with a bump kernel.
  Union,              ///< Minimum over the children.
  Intersection,       ///< Maximum over the children.
  SmoothUnion,        ///< Smooth minimum of the two smallest children.
  SmoothIntersection, ///< Smooth maximum of the two largest children.
  Add,                ///< Sum of the children.
};

/**
 * @brief A shape held by an expression, with its type erased.
 * @details Internal helper; not part of the public API. Host-only: the object is a heap copy of the
 * shape, and the tape compiler casts it back once it has matched m_type against its primitive types.
 */
struct Leaf
{
  /**
   * @brief Copy of the shape.
   */
  std::shared_ptr<const void> m_object;

  /**
   * @brief The shape's type.
   */
  std::type_index m_type = std::type_index(typeid(void));

  /**
   * @brief For a pool-resident shape: whether it lives in a given pool. Null otherwise.
   */
  bool (*m_isAttachedTo)(const void*, const Pool&) = nullptr;
};

/**
 * @brief An expression node.
 * @details Internal helper; not part of the public API. Each node's bounding volume and distance
 * quality are computed when it is made, from its children's.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Node
{
  /**
   * @brief What the node does.
   */
  Kind m_kind = Kind::Primitive;

  /**
   * @brief Children (none for a primitive, one for a map or value operation, any number for a union or
   * intersection).
   */
  std::vector<Expr<T>> m_children;

  /**
   * @brief The shape, for a primitive.
   */
  Leaf m_leaf;

  /**
   * @brief Point map applied before the child is evaluated (Affine).
   */
  Affine3T<T> m_map = Affine3T<T>::identity();

  /**
   * @brief Inverse of m_map, which maps the child's box to the node's (Affine).
   */
  Affine3T<T> m_inverse = Affine3T<T>::identity();

  /**
   * @brief Value multiplier applied after the child is evaluated (Affine): |s| for a scaling by s.
   */
  T m_valueScale = T(1);

  /**
   * @brief Elongation half-lengths (Elongate) or tile period (Repeat).
   */
  Vec3T<T> m_vec0 = Vec3T<T>::zeros();

  /**
   * @brief Tiles in the decreasing direction of each axis, rounded to whole numbers (Repeat).
   */
  Vec3T<T> m_vec1 = Vec3T<T>::zeros();

  /**
   * @brief Tiles in the increasing direction of each axis, rounded to whole numbers (Repeat).
   */
  Vec3T<T> m_vec2 = Vec3T<T>::zeros();

  /**
   * @brief Offset distance (Offset), shell half-thickness (Annular), blur distance (Blur),
   * mollification distance (Mollify) or smoothing length (smooth operations).
   */
  T m_scalar0 = T(0);

  /**
   * @brief Blend weight alpha (Blur).
   */
  T m_scalar1 = T(0);

  /**
   * @brief Samples per axis (Mollify).
   */
  uint32_t m_count = 0;

  /**
   * @brief Blend kind (smooth operations).
   */
  BlendKind m_blend = BlendKind::Polynomial;

  /**
   * @brief A box enclosing the node's object.
   */
  BoundingVolumes::AABBT<T> m_boundingVolume;

  /**
   * @brief How far the node's value can be trusted as a distance.
   */
  DistanceQuality m_quality = DistanceQuality::Exact;
};

/**
 * @brief Whether A is a shape or an Expr, and its precision: the primary template is neither.
 * @details Internal helper; not part of the public API.
 * @tparam A Candidate type.
 */
template <class A, class = void>
struct ExprLike
{
};

/**
 * @brief An Expr<T> is expression-like, with precision T.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 */
template <class T>
struct ExprLike<Expr<T>, void>
{
  /**
   * @brief The precision.
   */
  using type = T;
};

/**
 * @brief A shape is expression-like, with its own precision.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S>
struct ExprLike<S, std::enable_if_t<isShape<S>>>
{
  /**
   * @brief The precision.
   */
  using type = ShapePrecision<S>;
};

} // namespace ExprDetail

/**
 * @brief The precision of an expression-like type: T for Expr<T> or for a shape of precision T. Not
 * defined for other types, so a function taking it drops out of overload resolution.
 * @tparam A Expr<T> or shape type.
 */
template <class A>
using ExprPrecision = typename ExprDetail::ExprLike<A>::type;

/**
 * @brief A composed object: an immutable tree of shapes and operations, built on the host and compiled
 * by a Tape.
 * @details Copies share the tree, so passing an Expr by value is cheap and an Expr can be used several
 * times in one tree. Every shape converts to an Expr; the free functions in this header (Union,
 * Translate, ...) take shapes or Exprs and return an Expr. A pool-resident shape (a mesh SDF, a BVH
 * union) is held as a descriptor, so its pool must outlive any Tape compiled from the Expr, and must be
 * the pool the Tape is compiled into.
 * @tparam T Floating-point precision.
 */
template <class T>
class Expr
{
public:
  static_assert(std::is_floating_point_v<T>, "Expr requires a floating-point type T");

  /**
   * @brief Bounding box type.
   */
  using BV = BoundingVolumes::AABBT<T>;

  /**
   * @brief No empty expression.
   */
  Expr() = delete;

  /**
   * @brief Wrap a shape.
   * @details Checks at compile time that S is a shape of precision T (see isShape). The shape is copied.
   * @tparam S Shape type.
   * @param[in] a_shape Shape.
   */
  template <class S,
            class = std::enable_if_t<!std::is_same_v<std::decay_t<S>, Expr> &&
                                     !std::is_convertible_v<S, std::shared_ptr<const ExprDetail::Node<T>>>>>
  EBGEOMETRY_HOST
  Expr(const S& a_shape); // NOLINT(google-explicit-constructor): a shape is an expression

  /**
   * @brief Wrap a node; used by the builder functions.
   * @param[in] a_node Node.
   */
  EBGEOMETRY_HOST
  explicit Expr(std::shared_ptr<const ExprDetail::Node<T>> a_node) noexcept;

  /**
   * @brief A box enclosing the object.
   * @details Computed from the shapes' boxes by the rules on the tape's documentation page; for
   * Complement, and anything containing an unbounded shape such as a plane, it reaches plus or minus
   * Math::Limits<T>::max() along the unbounded directions.
   * @return The box.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const BV&
  computeBoundingVolume() const noexcept;

  /**
   * @brief How far the object's value can be trusted as a distance.
   * @return Exact for a shape whose own quality is Exact and maps that keep distances; Bound for unions,
   * intersections and the other operations that give a 1-Lipschitz lower bound; NotADistance when a
   * shape is not a distance or an operation does not keep one.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  DistanceQuality
  getDistanceQuality() const noexcept;

  /**
   * @brief The root node, for the tape compiler.
   * @return The node.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const ExprDetail::Node<T>&
  node() const noexcept;

private:
  /**
   * @brief The root node.
   */
  std::shared_ptr<const ExprDetail::Node<T>> m_node;
};

/**
 * @brief Convert a shape or an Expr to an Expr.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Shape or Expr.
 * @return The Expr.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
toExpr(const A& a_operand);

/**
 * @brief Union of two objects: the minimum of their values.
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision.
 * @param[in] a_first  First object.
 * @param[in] a_second Second object.
 * @return The union.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Union(const A& a_first, const B& a_second);

/**
 * @brief Union of any number of objects: the minimum of their values.
 * @details A Tape evaluates a union of many objects with a BVH over their boxes (see TapeOptions), unless
 * one of them is not a distance. With exact distances the value is the plain minimum; with operands that
 * are only bounds it can be larger, but keeps the sign and is still a bound.
 * @tparam A Shape or Expr type.
 * @param[in] a_operands Objects (must not be empty).
 * @return The union.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Union(const std::vector<A>& a_operands);

/**
 * @brief Smooth union of two objects: a smooth minimum of their values.
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision.
 * @param[in] a_first  First object.
 * @param[in] a_second Second object.
 * @param[in] a_smooth Smoothing length (> 0).
 * @param[in] a_blend  Blend kind.
 * @return The smooth union.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
SmoothUnion(const A&               a_first,
            const B&               a_second,
            const ExprPrecision<A> a_smooth,
            const BlendKind        a_blend = BlendKind::Polynomial);

/**
 * @brief Smooth union of any number of objects: a smooth minimum of the two smallest values.
 * @details Only the two smallest values are blended, which differs from folding a binary smooth
 * minimum. A Tape evaluates a smooth union of many objects with a BVH, as for Union().
 * @tparam A Shape or Expr type.
 * @param[in] a_operands Objects (must not be empty).
 * @param[in] a_smooth   Smoothing length (> 0).
 * @param[in] a_blend    Blend kind.
 * @return The smooth union.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
SmoothUnion(const std::vector<A>&  a_operands,
            const ExprPrecision<A> a_smooth,
            const BlendKind        a_blend = BlendKind::Polynomial);

/**
 * @brief Intersection of two objects: the maximum of their values.
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision.
 * @param[in] a_first  First object.
 * @param[in] a_second Second object.
 * @return The intersection.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Intersection(const A& a_first, const B& a_second);

/**
 * @brief Intersection of any number of objects: the maximum of their values.
 * @tparam A Shape or Expr type.
 * @param[in] a_operands Objects (must not be empty).
 * @return The intersection.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Intersection(const std::vector<A>& a_operands);

/**
 * @brief Smooth intersection of two objects: a smooth maximum of their values.
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision.
 * @param[in] a_first  First object.
 * @param[in] a_second Second object.
 * @param[in] a_smooth Smoothing length (> 0).
 * @param[in] a_blend  Blend kind.
 * @return The smooth intersection.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
SmoothIntersection(const A&               a_first,
                   const B&               a_second,
                   const ExprPrecision<A> a_smooth,
                   const BlendKind        a_blend = BlendKind::Polynomial);

/**
 * @brief Smooth intersection of any number of objects: a smooth maximum of the two largest values.
 * @tparam A Shape or Expr type.
 * @param[in] a_operands Objects (must not be empty).
 * @param[in] a_smooth   Smoothing length (> 0).
 * @param[in] a_blend    Blend kind.
 * @return The smooth intersection.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
SmoothIntersection(const std::vector<A>&  a_operands,
                   const ExprPrecision<A> a_smooth,
                   const BlendKind        a_blend = BlendKind::Polynomial);

/**
 * @brief Difference A \ B: max(A, -B), inside A and outside B.
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision.
 * @param[in] a_first  Object to subtract from.
 * @param[in] a_second Object to subtract.
 * @return The difference.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Difference(const A& a_first, const B& a_second);

/**
 * @brief Difference A \ (B1 u B2 u ...): max(A, -min_i Bi).
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision.
 * @param[in] a_first  Object to subtract from.
 * @param[in] a_others Objects to subtract (must not be empty).
 * @return The difference.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Difference(const A& a_first, const std::vector<B>& a_others);

/**
 * @brief Smooth difference A \ B: the smooth intersection of A and the complement of B.
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision.
 * @param[in] a_first  Object to subtract from.
 * @param[in] a_second Object to subtract.
 * @param[in] a_smooth Smoothing length (> 0).
 * @param[in] a_blend  Blend kind.
 * @return The smooth difference.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
SmoothDifference(const A&               a_first,
                 const B&               a_second,
                 const ExprPrecision<A> a_smooth,
                 const BlendKind        a_blend = BlendKind::Polynomial);

/**
 * @brief Smooth difference A \ (B1, B2, ...): the smooth intersection of A and the complement of each Bi.
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision.
 * @param[in] a_first  Object to subtract from.
 * @param[in] a_others Objects to subtract (must not be empty).
 * @param[in] a_smooth Smoothing length (> 0).
 * @param[in] a_blend  Blend kind.
 * @return The smooth difference.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
SmoothDifference(const A&               a_first,
                 const std::vector<B>&  a_others,
                 const ExprPrecision<A> a_smooth,
                 const BlendKind        a_blend = BlendKind::Polynomial);

/**
 * @brief Finite repetition: copies of an object on a grid of tiles, `a_repeatLo` tiles below and
 * `a_repeatHi` tiles above the home tile along each axis.
 * @details The value at p is the object's value at p folded into its tile,
 * p_i - c_i round(clamp(p_i / c_i, -lo_i, hi_i)). It is a distance bound only if the object fits inside
 * its tile ([-c/2, c/2] along each axis); otherwise it is not a distance.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand  Object to repeat, centred on the origin.
 * @param[in] a_period   Tile period along each axis (> 0).
 * @param[in] a_repeatLo Tiles in the decreasing direction of each axis (>= 0; rounded to whole tiles).
 * @param[in] a_repeatHi Tiles in the increasing direction of each axis (>= 0; rounded to whole tiles).
 * @return The repetition.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
FiniteRepetition(const A&                       a_operand,
                 const Vec3T<ExprPrecision<A>>& a_period,
                 const Vec3T<ExprPrecision<A>>& a_repeatLo,
                 const Vec3T<ExprPrecision<A>>& a_repeatHi);

/**
 * @brief Complement: inside and outside swapped, -v.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @return The complement.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Complement(const A& a_operand);

/**
 * @brief Translate an object by a shift.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_shift   Shift (finite).
 * @return The translated object, whose value at p is the object's value at p - a_shift.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Translate(const A& a_operand, const Vec3T<ExprPrecision<A>>& a_shift);

/**
 * @brief Rotate an object about a coordinate axis through the origin.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_degrees Angle in degrees, counterclockwise seen from the positive end of the axis (finite).
 * @param[in] a_axis    Coordinate axis (0, 1 or 2).
 * @return The rotated object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Rotate(const A& a_operand, const ExprPrecision<A> a_degrees, const size_t a_axis);

/**
 * @brief Rotate an object about an arbitrary axis through the origin.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_degrees Angle in degrees, counterclockwise seen from the positive end of the axis (finite).
 * @param[in] a_axis    Axis direction (finite, non-zero; need not be normalized).
 * @return The rotated object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Rotate(const A& a_operand, const ExprPrecision<A> a_degrees, const Vec3T<ExprPrecision<A>>& a_axis);

/**
 * @brief Scale an object uniformly about the origin.
 * @details The value at p is |s| times the object's value at p / s, which keeps a distance a distance.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_factor  Scale factor s (finite, non-zero; a negative factor also reflects through the
 * origin).
 * @return The scaled object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Scale(const A& a_operand, const ExprPrecision<A> a_factor);

/**
 * @brief Reflect an object in a coordinate plane through the origin.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_axis    Axis normal to the plane (0, 1 or 2).
 * @return The reflected object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Reflect(const A& a_operand, const size_t a_axis);

/**
 * @brief Offset an object's surface outward by a distance, v - r.
 * @details Exact for a dilation (r >= 0) of an exact distance; a bound for an erosion.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_offset  Offset distance r (finite); negative erodes.
 * @return The offset object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Offset(const A& a_operand, const ExprPrecision<A> a_offset);

/**
 * @brief The sum of two objects' values, v1 + v2.
 * @details Not a distance, and nothing bounds where its zero set lies, so its box is all of space. For
 * a displacement of bounded size, Displace() gives the same value with a box.
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision.
 * @param[in] a_first  First object.
 * @param[in] a_second Second object.
 * @return The sum.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Add(const A& a_first, const B& a_second);

/**
 * @brief Displace an object's surface by a field, v + d: noise or texture on a surface.
 * @details @p a_amplitude bounds |d| everywhere, which nothing checks; the box is the object's grown by
 * it, or all of space if the object's value is not a distance (another displacement, say), since such
 * a value can be small far from its object. The result is not a distance, even for an exact object: a
 * displacement that varies by L per unit length makes it a (1 + L)-Lipschitz function.
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision: the displacement field.
 * @param[in] a_operand      Object.
 * @param[in] a_displacement Displacement field d.
 * @param[in] a_amplitude    Bound on |d| (finite, >= 0).
 * @return The displaced object.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Displace(const A& a_operand, const B& a_displacement, const ExprPrecision<A> a_amplitude);

/**
 * @brief A shell around an object's surface, |v| - delta.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_delta   Shell half-thickness (finite).
 * @return The shell.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Annular(const A& a_operand, const ExprPrecision<A> a_delta);

/**
 * @brief Elongate an object: stretch it by inserting a box of half-lengths h at the origin.
 * @details The value at p is the object's value at p - clamp(p, -h, h); exact outside the object only.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand    Object.
 * @param[in] a_elongation Half-lengths h (finite, >= 0).
 * @return The elongated object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Elongate(const A& a_operand, const Vec3T<ExprPrecision<A>>& a_elongation);

/**
 * @brief Blur an object: a weighted average of its values at the 27 points p + d (i, j, k), i, j, k in
 * {-1, 0, 1}.
 * @details The weights are alpha^(3-m) ((1 - alpha)/2)^m for a tap m steps from the centre, which sum to
 * one, so a 1-Lipschitz object stays 1-Lipschitz.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_blur    Blur distance d (finite, >= 0).
 * @param[in] a_alpha   Centre weight factor in [0, 1]; 1 = no blur.
 * @return The blurred object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Blur(const A& a_operand, const ExprPrecision<A> a_blur, const ExprPrecision<A> a_alpha = ExprPrecision<A>(0.5));

/**
 * @brief Mollify an object: convolve it with a smooth, non-negative bump kernel sampled on an
 * n x n x n grid spanning [-r, r]^3.
 * @details The kernel weights are normalized to sum to one, so a 1-Lipschitz object stays 1-Lipschitz.
 * With r = 0 or n = 1 the object is returned unchanged.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_dist    Mollification distance r: the half-width of the sampling cube (finite).
 * @param[in] a_samples Samples per axis n (>= 1).
 * @return The mollified object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Mollify(const A& a_operand, const ExprPrecision<A> a_dist, const size_t a_samples);

} // namespace EBGeometry

#include "EBGeometry_ExprImplem.hpp"

#endif
