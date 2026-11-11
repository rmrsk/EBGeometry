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
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Affine.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_FaceGroups.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_SurfaceRef.hpp"
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

/**
 * @brief How a Tape evaluates a union of many objects: as written, or through a BVH over the objects' boxes.
 */
enum class Accelerate : uint8_t
{
  None, ///< As written: the tape evaluates every operand, however many there are.
  BVH   ///< Through a BVH over the operands' boxes, however many there are.
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
 * to the surface (Exact), at most that distance (Bound), or nothing (NotADistance). The box holds the solid: the value
 * is not negative outside it (a BVH union skips a member whose box a query misses, and a renderer's cut does too); a
 * shape with no bound returns an unbounded box.
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

  /**
   * @brief For a container (see hasSurfaces): how many surfaces it holds. Null for a shape with one surface.
   */
  uint32_t (*m_numSurfaces)(const void*) = nullptr;

  /**
   * @brief For a shape with parts (see hasParts): how many it has. Null for a shape with one part.
   */
  uint32_t (*m_numParts)(const void*) = nullptr;
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
   * @brief Whether a union asks the tape for a BVH over its operands (Accelerate::BVH).
   */
  bool m_bvh = false;

  /**
   * @brief A box enclosing the node's object.
   */
  BoundingVolumes::AABBT<T> m_boundingVolume;

  /**
   * @brief How far the node's value can be trusted as a distance.
   */
  DistanceQuality m_quality = DistanceQuality::Exact;

  /**
   * @brief The name SurfaceLabel() gave the node; empty for none.
   */
  std::string m_surfaceLabel;

  /**
   * @brief For a primitive: the groups SurfaceLabel(shape, FaceGroups) put its parts in; null for none.
   */
  std::shared_ptr<const FaceGroups> m_partGroups;

  /**
   * @brief The name MaterialLabel() gave the node; empty for none.
   */
  std::string m_materialLabel;

  /**
   * @brief The name RegionName() gave the node; empty for none.
   */
  std::string m_regionName;

  /**
   * @brief Whether this union is an Overlay: its operands are regions, later ones on top (see Overlay()). A Tape
   * evaluates it as the union of its operands, the whole object.
   */
  bool m_overlay = false;

  /**
   * @brief Whether the node is a partition into regions: an Overlay, a map of one, or one clipped as the first
   * operand of an intersection (a mask).
   */
  bool m_partition = false;

  /**
   * @brief Whether the node, or a chain of maps below it, has a material label or a region name (MaterialLabel(),
   * RegionName()): what only a region can have.
   */
  bool m_namesRegion = false;
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

/**
 * @brief Whether a type is expression-like with a precision other than T (a shape or an Expr): the primary template,
 * for other types, is false.
 * @details Internal helper; not part of the public API. Keeps Expr<T>'s implicit constructor out of overload
 * resolution for such types, so that a braced list of shapes picks Overlay()'s overload of their precision.
 * @tparam S Type.
 * @tparam T Floating-point precision.
 */
template <class S, class T, class = void>
struct OtherPrecision : std::false_type
{
};

/**
 * @brief An expression-like type: true when its precision is not T.
 * @tparam S Shape or Expr type.
 * @tparam T Floating-point precision.
 */
template <class S, class T>
struct OtherPrecision<S, T, std::void_t<typename ExprLike<S>::type>>
  : std::bool_constant<!std::is_same_v<typename ExprLike<S>::type, T>>
{
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
                                     !std::is_convertible_v<S, std::shared_ptr<const ExprDetail::Node<T>>> &&
                                     !ExprDetail::OtherPrecision<std::decay_t<S>, T>::value>>
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
 * @details With Accelerate::BVH, a Tape evaluates the union through a BVH over the objects' boxes; with
 * Accelerate::None, as written, every object at every point (a large one prints a warning when the tape is
 * built, see TapeOptions::m_largeUnionWarning). In the BVH, an
 * object that is a shape, or a shape under maps (Translate, Rotate, Scale, ...), is evaluated directly at its
 * leaf; any other object is a sub-program the leaf calls. With exact distances the value is the plain minimum;
 * with operands that are only bounds it can be larger, but keeps the sign and is still a bound; with operands
 * that are not distances, only its sign means anything. A list of one object is that object, with no BVH.
 * @tparam A Shape or Expr type.
 * @param[in] a_operands   Objects (must not be empty).
 * @param[in] a_accelerate Whether the tape evaluates the union through a BVH.
 * @return The union.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Union(const std::vector<A>& a_operands, Accelerate a_accelerate = Accelerate::None);

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
 * minimum. A Tape evaluates it through a BVH as for Union(); a ray marches it.
 * @tparam A Shape or Expr type.
 * @param[in] a_operands   Objects (must not be empty).
 * @param[in] a_smooth     Smoothing length (> 0).
 * @param[in] a_blend      Blend kind.
 * @param[in] a_accelerate Whether the tape evaluates the union through a BVH.
 * @return The smooth union.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
SmoothUnion(const std::vector<A>&  a_operands,
            const ExprPrecision<A> a_smooth,
            const BlendKind        a_blend      = BlendKind::Polynomial,
            const Accelerate       a_accelerate = Accelerate::None);

/**
 * @brief Smooth union of any number of objects with the polynomial blend, through a BVH or as written.
 * @tparam A Shape or Expr type.
 * @param[in] a_operands   Objects (must not be empty).
 * @param[in] a_smooth     Smoothing length (> 0).
 * @param[in] a_accelerate Whether the tape evaluates the union through a BVH.
 * @return The smooth union.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
SmoothUnion(const std::vector<A>& a_operands, const ExprPrecision<A> a_smooth, const Accelerate a_accelerate);

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
 * @details The objects subtracted are one Union(), through a BVH with Accelerate::BVH.
 * @tparam A Shape or Expr type.
 * @tparam B Shape or Expr type of the same precision.
 * @param[in] a_first      Object to subtract from.
 * @param[in] a_others     Objects to subtract (must not be empty).
 * @param[in] a_accelerate Whether the tape evaluates the union of a_others through a BVH.
 * @return The difference.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Difference(const A& a_first, const std::vector<B>& a_others, Accelerate a_accelerate = Accelerate::None);

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
 * @details The value is the minimum of the object's value over all its copies, so it is a distance bound
 * whenever the object's value is one, whether or not the object fits inside its tile. The copy in the
 * tile p folds into, p_i - c_i round(clamp(p_i / c_i, -lo_i, hi_i)), is evaluated first, and only the
 * copies whose box comes closer to p than that value after it; an object well inside its tile has few.
 * A Tape evaluates the copies as a sub-program, which takes one level of its call depth. For now the copies are
 * one surface: Tape::closestSurface() reports the leaf they copy, whichever copy decides. A copy that needs an
 * identity of its own is placed as an object of its own, in a union (with Accelerate::BVH for many).
 * @tparam A Shape or Expr type.
 * @param[in] a_operand  Object to repeat; its home copy sits in the tile around the origin.
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
 * @details The weights are alpha^(3-m) ((1 - alpha)/2)^m for a tap m steps from the center, which sum to
 * one, so a 1-Lipschitz object stays 1-Lipschitz.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_blur    Blur distance d (finite, >= 0).
 * @param[in] a_alpha   Center weight factor in [0, 1]; 1 = no blur.
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
 * @param[in] a_samples Samples per axis n (between 1 and 64).
 * @return The mollified object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Mollify(const A& a_operand, const ExprPrecision<A> a_dist, const size_t a_samples);

/**
 * @brief Name an object: every leaf below it gets the label, unless a part of it has a label of its own.
 * @details Labels change nothing in how the object is evaluated; they say which surfaces belong together,
 * for boundary conditions, per-surface data and picking. A Tape reports a leaf's label on the host and on a
 * device (Tape::getSurfaceLabel()); ExprIndex holds the names and the expression's structure on the host. Labels
 * are numbered from 0 in the order their names first appear, depth first, so the same expression gives the
 * same numbers to both; several objects may share a name, and so a label. On a composition (a union, an
 * intersection, an Overlay, ...) the label is a default: an operand labeled itself keeps its own. Labeling an object
 * that already has a label, directly or through maps (a move does not make another object), replaces it, as
 * MaterialLabel() and RegionName() do: the replaced name is gone, and gets no number unless another object has it.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_name    Name (not empty).
 * @return The labeled object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
SurfaceLabel(const A& a_operand, std::string a_name);

/**
 * @brief Name a shape's parts by group (see hasParts): each part in a group gets the label of the group's name,
 * which wins over any label above it; a part in no group keeps those.
 * @details For a mesh read from a file, the groups are its face groups (PolygonSoup::getFaceGroups()): an OBJ
 * file's `g` and `o` groups, or an ASCII STL file's named solids, so that a mesh's patches (inlet, outlet, wall)
 * become labels. The names are numbered with every other label's (see SurfaceLabel()), in the order they first appear.
 * The operand must be one shape, not a composition (label it before translating or combining it) nor a collection
 * (a BVH union, a sphere cloud, a tape), with as many parts as the groups have faces; anything else aborts with a
 * message, in every build. A Tape reports the label in
 * SurfaceRef::m_surfaceLabel, from closestSurface() and firstSurface(), on the host and on a device.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand A shape with parts (a mesh SDF, a box, ...).
 * @param[in] a_groups  The group of each part, and the groups' names.
 * @return The labeled shape.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
SurfaceLabel(const A& a_operand, FaceGroups a_groups);

/**
 * @brief Regions by priority, a priority union: each operand is a region of its own, later operands on top of earlier
 * ones where they overlap; the outside, region 0, is the background.
 * @details Region k is the part of operand k that no later operand covers, so the regions tile the object, with no
 * gaps and no overlaps, by construction. Each region can have a material (MaterialLabel() on its operand). The
 * background is what lies beyond the object and has no material: a medium around an object (the air around a cable)
 * is a region of its own, an operand under the others, whose outer surface bounds the computational domain. A Tape
 * evaluates an Overlay as the union of its operands, the whole object, so signedDistance() is unchanged by it;
 * ExprIndex lists its regions, groups and materials.
 *
 * Where an Overlay may appear: at the top of the expression, under maps (Translate, Rotate, ...), as the first
 * operand of an Intersection or Difference whose other operands are plain solids (masks: they clip or drill every
 * region), and as an operand of another Overlay, where its operands take its place in the outer priority order
 * (it is kept as a group of regions). Anything else (a union with a solid or another Overlay, an offset, a blend, a
 * repetition of an Overlay) aborts with a message, in every build: it would not be clear which region a result
 * belongs to.
 * @tparam A Shape or Expr type.
 * @param[in] a_operands The regions, in priority order (must not be empty).
 * @return The partition.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Overlay(const std::vector<A>& a_operands);

/**
 * @brief Regions by priority, of two or more operands, each on top of the ones before it (see Overlay()).
 * @tparam A    Shape or Expr type.
 * @tparam B    Shape or Expr type of the same precision.
 * @tparam Rest Shape or Expr types of the same precision.
 * @param[in] a_first  The first region, at the bottom.
 * @param[in] a_second The second region, on top of the first.
 * @param[in] a_rest   Further regions, each on top of the ones before it.
 * @return The partition.
 */
template <class A, class B, class... Rest>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
Overlay(const A& a_first, const B& a_second, const Rest&... a_rest);

/**
 * @brief Regions by priority, of a braced list of shapes and Exprs of any types, double precision: Overlay({air,
 * jacket, CylinderSDF<double>(...)}) (see Overlay()).
 * @param[in] a_operands The regions, in priority order (must not be empty).
 * @return The partition.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<double>
Overlay(std::initializer_list<Expr<double>> a_operands);

/**
 * @brief Regions by priority, of a braced list of shapes and Exprs of any types, single precision (see Overlay()).
 * @param[in] a_operands The regions, in priority order (must not be empty).
 * @return The partition.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<float>
Overlay(std::initializer_list<Expr<float>> a_operands);

/**
 * @brief Name the material of a region: an operand of an Overlay, a whole Overlay nested in another (a default for
 * its regions without one of their own), or a whole object.
 * @details Materials are not surface labels (SurfaceLabel()): a material belongs to a volume, a surface label to
 * surfaces. ExprIndex numbers the material names, in the order they first appear, depth first, as MaterialLabelId. A material label on part of a solid (an operand of a Union, an Intersection, ...)
 * aborts with a message, in every build: a solid is one material; make the parts regions of an Overlay. Labeling an
 * object that already has a material replaces it.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_name    Name (not empty).
 * @return The labeled object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
MaterialLabel(const A& a_operand, std::string a_name);

/**
 * @brief Name a region: an operand of an Overlay ("fluid", "wall"), or a whole Overlay nested in another (its region
 * group).
 * @details A region's name says which region it is, its material (MaterialLabel()) what it is made of: two regions of
 * the same material have names of their own. ExprIndex gives the names (Region::m_name, RegionGroup::m_name) and
 * finds a region by its name (findRegion()); names are unique among the regions, and among the region groups, or
 * ExprIndex aborts with a message. A name on part of a solid aborts with a message, in every build, as a material
 * label does. Naming a named object replaces its name.
 * @tparam A Shape or Expr type.
 * @param[in] a_operand Object.
 * @param[in] a_name    Name (not empty).
 * @return The named object.
 */
template <class A>
[[nodiscard]] EBGEOMETRY_HOST
inline Expr<ExprPrecision<A>>
RegionName(const A& a_operand, std::string a_name);

/**
 * @brief The nodes of an expression, numbered, with the leaves below each and their labels: on the host, the
 * way from what a Tape reports (a leaf, see LeafRef::m_leaf and HitT::m_primitive) back to the
 * expression.
 * @details The nodes are numbered from 0 at the root, depth first, the operands in order, each occurrence of
 * a shared subexpression counted; the leaves (the shapes) are numbered the same way, as a Tape compiled from
 * the expression numbers them. So the leaves below a node are a contiguous range, [m_firstLeaf,
 * m_firstLeaf + m_numLeaves), and so are its surfaces, [m_firstSurface, m_firstSurface + m_numSurfaces): one
 * per leaf, or all of a container leaf's (a sphere cloud's spheres, a BVH union's members). A group of surfaces
 * can be handed to a device as two numbers. The nodes
 * The labels are numbered in the order their names first appear, depth first, a leaf's face groups (SurfaceLabel(shape,
 * FaceGroups)) after the leaf's own label. The nodes
 * are those the builder functions made: Difference(a, b) is an Intersection of a and a Complement of b,
 * and nested maps (Translate, Rotate, Scale, ...) are one Transform node.
 * @tparam T Floating-point precision.
 */
template <class T>
class ExprIndex
{
public:
  /**
   * @brief No node, leaf or label.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief One node of the expression.
   */
  struct Node
  {
    /**
     * @brief The node it is an operand of; None for the root.
     */
    uint32_t m_parent = None;

    /**
     * @brief The first leaf below it (itself, for a leaf).
     */
    uint32_t m_firstLeaf = 0;

    /**
     * @brief How many leaves are below it (1 for a leaf).
     */
    uint32_t m_numLeaves = 0;

    /**
     * @brief The first surface below it (see Tape::closestSurface()).
     */
    uint32_t m_firstSurface = 0;

    /**
     * @brief How many surfaces are below it: one per leaf, or a container leaf's own count (see hasSurfaces).
     */
    uint32_t m_numSurfaces = 0;

    /**
     * @brief Its own surface label, or none (the label its leaves inherit from further up is getSurfaceLabel()'s).
     */
    SurfaceLabelId m_surfaceLabel;

    /**
     * @brief Its own material label, or none (MaterialLabel()).
     */
    MaterialLabelId m_materialLabel;

    /**
     * @brief What it does: Primitive, Transform, Elongate, Repeat, Complement, Offset, Annulus, Blur, Mollify,
     * Union, Intersection, SmoothUnion, SmoothIntersection or Overlay, with " (BVH)" after a union built with
     * Accelerate::BVH.
     */
    std::string m_operation;
  };

  /**
   * @brief One region of the object (see Overlay()): the background (region 0), an operand of an Overlay, or, for
   * an expression with no Overlay, the whole object (region 1).
   */
  struct Region
  {
    /**
     * @brief Its operand's node; None for the background.
     */
    uint32_t m_node = None;

    /**
     * @brief The innermost group (nested Overlay) it is in; None for the background and a whole object.
     */
    uint32_t m_regionGroup = None;

    /**
     * @brief Its material: its operand's material label, or its group's, or none.
     */
    MaterialLabelId m_material;

    /**
     * @brief Its name (RegionName() on its operand, or on a map above it); empty for none and the background.
     */
    std::string m_name;
  };

  /**
   * @brief A group of regions: an Overlay, whose regions are a contiguous range (a nested Overlay a range inside
   * its parent's).
   */
  struct RegionGroup
  {
    /**
     * @brief The Overlay's node.
     */
    uint32_t m_node = None;

    /**
     * @brief The group it is nested in; None for the outermost.
     */
    uint32_t m_parent = None;

    /**
     * @brief Its first region.
     */
    uint32_t m_firstRegion = 0;

    /**
     * @brief How many regions it has, those of nested groups included.
     */
    uint32_t m_numRegions = 0;

    /**
     * @brief Its material label, the default of its regions without one of their own; or none.
     */
    MaterialLabelId m_material;

    /**
     * @brief Its name (RegionName() on the Overlay); empty for none.
     */
    std::string m_name;
  };

  /**
   * @brief No empty index.
   */
  ExprIndex() = delete;

  /**
   * @brief Number the nodes, leaves and labels of an expression.
   * @param[in] a_expr Expression.
   */
  EBGEOMETRY_HOST
  explicit ExprIndex(const Expr<T>& a_expr);

  /**
   * @brief The number of nodes.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumNodes() const noexcept;

  /**
   * @brief A node.
   * @param[in] a_node Node number (below getNumNodes()).
   * @return The node.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Node&
  getNode(uint32_t a_node) const;

  /**
   * @brief The number of leaves: Tape::getNumPrimitives() of a tape compiled from the expression.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumLeaves() const noexcept;

  /**
   * @brief The node of a leaf.
   * @param[in] a_leaf Leaf number (below getNumLeaves()).
   * @return The node number.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getLeafNode(uint32_t a_leaf) const;

  /**
   * @brief The nodes from the root down to a leaf: the levels a surface belongs to, from the whole object to
   * the shape.
   * @param[in] a_leaf Leaf number (below getNumLeaves()).
   * @return The node numbers, the root first and the leaf's node last.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::vector<uint32_t>
  pathOf(uint32_t a_leaf) const;

  /**
   * @brief The number of surfaces: Tape::getNumSurfaces() of a tape compiled from the expression.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumSurfaces() const noexcept;

  /**
   * @brief A leaf's first surface: the leaf covers [getFirstSurface(leaf), getFirstSurface(leaf + 1)), as
   * Tape::getFirstSurface() numbers them.
   * @param[in] a_leaf Leaf number, at most getNumLeaves() (which gives getNumSurfaces()).
   * @return The surface.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getFirstSurface(uint32_t a_leaf) const;

  /**
   * @brief The leaf a surface belongs to: SurfaceRef::m_leaf of a tape's closestSurface() that reported it.
   * @param[in] a_surface Surface (below getNumSurfaces()).
   * @return The leaf number.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getLeafOfSurface(uint32_t a_surface) const;

  /**
   * @brief A leaf's label: that of the nearest labeled node on its path, the leaf's own included, except that a label
   * above a chain of maps relabels the object below them (see SurfaceLabel()).
   * @param[in] a_leaf Leaf number (below getNumLeaves()).
   * @return The label, or none if no node on the path has one.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  SurfaceLabelId
  getSurfaceLabel(uint32_t a_leaf) const;

  /**
   * @brief How many parts a leaf's shape has (see hasParts): one for a shape without parts, a mesh's faces, a box's
   * six; 0 for a collection (a cloud, a BVH union, a tape), whose surfaces are its members, each with its own parts.
   * @param[in] a_leaf Leaf number (below getNumLeaves()).
   * @return The count, or 0.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumPartsOfLeaf(uint32_t a_leaf) const;

  /**
   * @brief The label of a part of a leaf (see hasParts): its face group's label when SurfaceLabel(shape, FaceGroups)
   * put it in one, and otherwise the leaf's (getSurfaceLabel()).
   * @param[in] a_leaf Leaf number (below getNumLeaves()).
   * @param[in] a_part Part of the leaf's shape (SurfaceRef::m_part).
   * @return The label, or none.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  SurfaceLabelId
  getPartSurfaceLabel(uint32_t a_leaf, uint32_t a_part) const;

  /**
   * @brief The labels of a leaf's parts, one per part, as getPartSurfaceLabel() gives them; empty for a leaf whose parts
   * SurfaceLabel(shape, FaceGroups) did not name.
   * @param[in] a_leaf Leaf number (below getNumLeaves()).
   * @return The labels.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<SurfaceLabelId>&
  getPartSurfaceLabels(uint32_t a_leaf) const;

  /**
   * @brief The number of distinct labels.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumSurfaceLabels() const noexcept;

  /**
   * @brief A label's name.
   * @param[in] a_label Label (below getNumSurfaceLabels()).
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::string&
  getSurfaceLabelName(SurfaceLabelId a_label) const;

  /**
   * @brief The label of a name.
   * @param[in] a_name Name.
   * @return The label, or none if no node has that name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  SurfaceLabelId
  findSurfaceLabel(std::string_view a_name) const;

  /**
   * @brief The number of regions: the background (region 0), and one per operand of the Overlays, flattened into one
   * priority order depth first (later regions on top); an expression with no Overlay has one, the whole object.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumRegions() const noexcept;

  /**
   * @brief A region.
   * @param[in] a_region Region number (below getNumRegions()).
   * @return The region.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Region&
  getRegion(uint32_t a_region) const;

  /**
   * @brief The region a leaf belongs to: the region whose operand holds it.
   * @param[in] a_leaf Leaf number (below getNumLeaves()).
   * @return The region, or None for a leaf of a mask (an operand that clips or drills the regions).
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getRegionOfLeaf(uint32_t a_leaf) const;

  /**
   * @brief A region's material (see Region::m_material).
   * @param[in] a_region Region number (below getNumRegions()).
   * @return The material, or none.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  MaterialLabelId
  getMaterialOf(uint32_t a_region) const;

  /**
   * @brief The regions of a group, for a subdomain (Subdomains::addSubdomain()).
   * @param[in] a_group Group number (below getNumRegionGroups()).
   * @return Its regions, a contiguous range.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::vector<uint32_t>
  getRegionsOfRegionGroup(uint32_t a_group) const;

  /**
   * @brief The regions of a material, for a subdomain (Subdomains::addSubdomain()).
   * @param[in] a_material Material label.
   * @return Its regions, in order; empty for a material no region has.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::vector<uint32_t>
  getRegionsOfMaterial(MaterialLabelId a_material) const;

  /**
   * @brief The pairs of regions a leaf's surface may separate, conservatively: every pair it does is listed, and
   * some listed may not occur (the boxes overlap, the shapes need not meet). A region's own surface separates it
   * from the background and from earlier regions whose boxes meet its own (a later region covers it, and the
   * interface is that region's surface); a mask's separates a region it cuts from the background or a region outside
   * the masked Overlay. Each pair smaller region first, sorted. For an expression with no Overlay: (0, 1).
   * @param[in] a_leaf Leaf number (below getNumLeaves()).
   * @return The pairs.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<std::pair<uint32_t, uint32_t>>&
  getSidesOfLeaf(uint32_t a_leaf) const;

  /**
   * @brief The pairs of regions any surface may separate (see getSidesOfLeaf()): the interfaces a solver can meet.
   * @return The pairs, smaller region first, sorted, without repeats.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::vector<std::pair<uint32_t, uint32_t>>
  getInterfaces() const;

  /**
   * @brief The number of groups: one per Overlay, numbered depth first.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumRegionGroups() const noexcept;

  /**
   * @brief A group.
   * @param[in] a_group Group number (below getNumRegionGroups()).
   * @return The group.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const RegionGroup&
  getRegionGroup(uint32_t a_group) const;

  /**
   * @brief The number of distinct material labels.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumMaterialLabels() const noexcept;

  /**
   * @brief A material label's name.
   * @param[in] a_label Label (below getNumMaterialLabels()).
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::string&
  getMaterialLabelName(MaterialLabelId a_label) const;

  /**
   * @brief The material label of a name.
   * @param[in] a_name Name.
   * @return The label, or none if no region has that material.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  MaterialLabelId
  findMaterialLabel(std::string_view a_name) const;

  /**
   * @brief The region with a name (RegionName()).
   * @param[in] a_name The name.
   * @return Its number, or None if no region has it.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  findRegion(std::string_view a_name) const;

  /**
   * @brief Warnings about names that cannot appear on the object: a surface label none of whose leaves (or parts
   * named through face groups) can reach the object's boundary, or that every leaf below it overrides with a label of
   * its own; a region with no volume (covered by later regions of its Overlay, or outside a mask); a material all of
   * whose regions have none.
   * @details A diagnostic only: it changes nothing, and the expression is evaluated as written; a correct geometry
   * is the user's to make. It is conservative: it reports only what it has proved, so a lost name can go unreported
   * (surfaces that coincide exactly, as a core filling a cavity, are never reported), but a reported one is lost. The
   * shapes are evaluated through the tape compiled from the expression (Tape::leafValue()), on the host, a few times
   * per leaf; a leaf the tape does not hold (inside a collection) is never evaluated.
   *
   * A leaf is proved hidden when, on its way to the root, its box lies strictly inside another operand of a union,
   * strictly inside later operands of an Overlay (its region has no volume; an earlier operand only lies under it,
   * which leaves an interface), or strictly outside another operand of an intersection (a mask included), and only
   * maps, Elongate, FiniteRepetition, complements, unions, Overlays and intersections lie on that way. An operation
   * that acts on values (Offset, Annular, Blur, Mollify, the smooth operations) on the way leaves it unproved. Inside
   * and outside are proved with bounds of the other operand's value over a ball around the box (a shape's value at
   * the center, plus or minus the radius, for a shape whose value is a distance or a bound), the box split into up to
   * 64 pieces when it is not proved whole. A region is proved empty the same way, from its operand.
   * The index does not hold the expression, so it is given again; it must be the one the index was built from
   * (aborts with a message, in every build, when its nodes do not match).
   * @tparam TapeType The Tape compiled from the same expression, its pool readable on the host.
   * @param[in] a_expr The expression the index was built from.
   * @param[in] a_tape The tape compiled from it.
   * @return The warnings, empty when there are none.
   */
  template <class TapeType>
  [[nodiscard]] EBGEOMETRY_HOST
  std::vector<std::string>
  checkLabels(const Expr<T>& a_expr, const TapeType& a_tape) const;

private:
  /**
   * @brief The nodes.
   */
  std::vector<Node> m_nodes;

  /**
   * @brief Each leaf's node.
   */
  std::vector<uint32_t> m_leafNodes;

  /**
   * @brief Each leaf's label (see getSurfaceLabel()).
   */
  std::vector<SurfaceLabelId> m_leafSurfaceLabels;

  /**
   * @brief Each leaf's first surface, and the number of surfaces last.
   */
  std::vector<uint32_t> m_leafSurfaces;

  /**
   * @brief Each leaf's part labels (see getPartSurfaceLabels()).
   */
  std::vector<std::vector<SurfaceLabelId>> m_leafPartSurfaceLabels;

  /**
   * @brief Each leaf's number of parts (see getNumPartsOfLeaf()).
   */
  std::vector<uint32_t> m_leafParts;

  /**
   * @brief The labels' names.
   */
  std::vector<std::string> m_surfaceLabelNames;

  /**
   * @brief Each name's label.
   */
  std::unordered_map<std::string, SurfaceLabelId> m_surfaceLabelIds;

  /**
   * @brief Each node's expression node, while the index is built (cleared after: the index does not hold the
   * expression).
   */
  std::vector<const ExprDetail::Node<T>*> m_exprNodes;

  /**
   * @brief Each node's operands' numbers.
   */
  std::vector<std::vector<uint32_t>> m_operands;

  /**
   * @brief Why a node (a leaf, or a region's operand) cannot reach the object's boundary (see checkLabels()).
   * @param[in] a_node      The node.
   * @param[in] a_nodes     Each node's expression node.
   * @param[in] a_leafValue A shape's value, through the tape.
   * @return The reason, or empty when it was not proved.
   */
  [[nodiscard]] std::string
  whyHidden(uint32_t                                                                    a_node,
            const std::vector<const ExprDetail::Node<T>*>&                              a_nodes,
            const std::function<bool(const ExprDetail::Node<T>&, const Vec3T<T>&, T&)>& a_leafValue) const;

  /**
   * @brief The regions.
   */
  std::vector<Region> m_regions;

  /**
   * @brief The groups.
   */
  std::vector<RegionGroup> m_regionGroups;

  /**
   * @brief Each leaf's region, or None.
   */
  std::vector<uint32_t> m_leafRegions;

  /**
   * @brief The material labels' names.
   */
  std::vector<std::string> m_materialLabelNames;

  /**
   * @brief Each material name's label.
   */
  std::unordered_map<std::string, MaterialLabelId> m_materialLabelIds;

  /**
   * @brief A mask of a partition (an operand of a clipping intersection), while the index is built.
   */
  struct Mask
  {
    /**
     * @brief Its node.
     */
    uint32_t m_node;

    /**
     * @brief Its box in the scene's frame.
     */
    BoundingVolumes::AABBT<T> m_box;

    /**
     * @brief The first region it cuts.
     */
    uint32_t m_firstRegion;

    /**
     * @brief How many regions it cuts (a contiguous range).
     */
    uint32_t m_numRegions;
  };

  /**
   * @brief Each region's box in the scene's frame (the background's is empty).
   */
  std::vector<BoundingVolumes::AABBT<T>> m_regionBoxes;

  /**
   * @brief The masks, while the index is built.
   */
  std::vector<Mask> m_masks;

  /**
   * @brief The maps above the node being numbered, outermost first, while the index is built.
   */
  std::vector<const ExprDetail::Node<T>*> m_maps;

  /**
   * @brief Each leaf's pairs of regions (see getSidesOfLeaf()).
   */
  std::vector<std::vector<std::pair<uint32_t, uint32_t>>> m_leafSides;

  /**
   * @brief A box in a node's frame, in the scene's: mapped through the maps above it.
   * @param[in] a_box The box.
   * @return The box in the scene's frame.
   */
  BoundingVolumes::AABBT<T>
  sceneBox(const BoundingVolumes::AABBT<T>& a_box) const;

  /**
   * @brief Number the pairs of regions each leaf's surface may separate (getSidesOfLeaf()).
   */
  void
  addSides();

  /**
   * @brief The material label of a name, numbered if it is new.
   * @param[in] a_name Name (not empty).
   * @return The label.
   */
  MaterialLabelId
  internMaterial(const std::string& a_name);

  /**
   * @brief Number the regions and groups at and below a node of a partition (see Overlay()).
   * @param[in] a_node     Node number.
   * @param[in] a_group    The group it is in, or None.
   * @param[in] a_material The material its regions inherit, or none.
   */
  void
  addRegions(uint32_t a_node, uint32_t a_group, MaterialLabelId a_material);

  /**
   * @brief Number a node and those below it.
   * @param[in] a_node   Node.
   * @param[in] a_parent Its parent's number, or None.
   * @param[in] a_label  The label it inherits, or none.
   * @param[in] a_object Whether that label names this very object, given above it with only maps between (a move
   * does not make another object, so it relabels this one), rather than a composition this node is part of.
   */
  void
  add(const ExprDetail::Node<T>& a_node, uint32_t a_parent, SurfaceLabelId a_label, bool a_object = false);
};

} // namespace EBGeometry

#include "EBGeometry_ExprImplem.hpp"

#endif
