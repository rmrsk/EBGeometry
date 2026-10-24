// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Fused.hpp
 * @brief  Fused CSG: the operations of the expression builder (EBGeometry_Expr.hpp) as types, which the
 * compiler evaluates fully inlined, on the host and on a device.
 * @details Fused::Union(a, b), Fused::Translate(a, shift), ... compose shapes like their namesakes in
 * namespace EBGeometry, but instead of an Expr to be compiled into a Tape they return a Fused::Node: a
 * trivially copyable shape (isShape) whose type spells out the whole composition. Its signedDistance()
 * is the composition's value, computed as a Tape computes it (both take each operation's arithmetic
 * from EBGeometry_CSGOps.hpp), but with no clauses to fetch and dispatch, so it runs as fast as the same
 * solid written out by hand. A fused shape can be used wherever a shape can: queried directly or
 * through GPU::transform(), held in a BVHUnion, or listed as a Tape primitive.
 *
 * Call the functions qualified (Fused::Union, not Union): the builder's functions of the same names
 * take shapes too, so an unqualified call is ambiguous.
 *
 * Mesh SDFs, BVH unions and tapes live in a Pool. A fused shape with one of them inside is a pool
 * handle itself (see PoolHandle): it stores its operands' layouts and one location, offers
 * rebasedView(), deepCopy(), layout() and a (Layout, PoolLocation) constructor, and its pool-resident
 * operands must all live in one pool, which must outlive the fused shape and every copy of it.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_FUSED_HPP
#define EBGEOMETRY_FUSED_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

// Our includes
#include "EBGeometry_Affine.hpp"
#include "EBGeometry_BVHUnion.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_CSGOps.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_PrimitiveRef.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Fused CSG: the expression builder's operations as types (see EBGeometry_Fused.hpp).
 */
namespace Fused {

template <class Op, class... Cs>
class Node;

/**
 * @brief Implementation details of the fused operations.
 * @details Internal helpers; not part of the public API.
 */
namespace Detail {

/**
 * @brief Whether a type is a fused node.
 * @tparam C Type.
 */
template <class C>
struct IsNode : std::false_type
{
};

/**
 * @brief A fused node is one.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 */
template <class Op, class... Cs>
struct IsNode<Node<Op, Cs...>> : std::true_type
{
};

/**
 * @brief Whether a type is a fused node.
 * @tparam C Type.
 */
template <class C>
inline constexpr bool isNode = IsNode<C>::value;

/**
 * @brief The number of leaves under an operand: a fused node's, or one for any other shape (a nested
 * composite such as a tape or a BVH union included).
 * @tparam C Operand type.
 * @return The number.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
constexpr uint32_t
leavesOf() noexcept
{
  if constexpr (isNode<C>) {
    return C::leafCount;
  }
  else {
    return 1U;
  }
}

/**
 * @brief The number of the first leaf under operand I: the leaves of the operands before it.
 * @tparam I    Operand index.
 * @tparam C    The first operand's type.
 * @tparam Rest The others.
 * @return The number.
 */
template <size_t I, class C, class... Rest>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
constexpr uint32_t
leafOffset() noexcept
{
  if constexpr (I == 0) {
    return 0U;
  }
  else {
    return leavesOf<C>() + leafOffset<I - 1, Rest...>();
  }
}

/**
 * @brief Whether an operand is, or has under it, a leaf of type P.
 * @tparam P Leaf type.
 * @tparam C Operand type.
 * @return True if it does.
 */
template <class P, class C>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
constexpr bool
hasLeaf() noexcept
{
  if constexpr (isNode<C>) {
    return C::template containsLeaf<P>();
  }
  else {
    return std::is_same_v<C, P>;
  }
}

/**
 * @brief The operands of a fused node, as stored: a recursive aggregate, so that it stays trivially
 * copyable and needs no std:: accessors in device code.
 * @tparam Ss Stored operand types.
 */
template <class... Ss>
struct Pack;

/**
 * @brief The empty pack.
 */
template <>
struct Pack<>
{
};

/**
 * @brief A pack: its first operand and the rest.
 * @tparam S    First stored operand type.
 * @tparam Rest The others.
 */
template <class S, class... Rest>
struct Pack<S, Rest...>
{
  /**
   * @brief First operand.
   */
  S m_head;

  /**
   * @brief The others.
   */
  Pack<Rest...> m_tail;
};

/**
 * @brief Operand I of a pack.
 * @tparam I    Index.
 * @tparam S    First stored operand type.
 * @tparam Rest The others.
 * @param[in] a_pack Pack.
 * @return The operand.
 */
template <size_t I, class S, class... Rest>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline const auto&
get(const Pack<S, Rest...>& a_pack) noexcept
{
  if constexpr (I == 0) {
    return a_pack.m_head;
  }
  else {
    return get<I - 1>(a_pack.m_tail);
  }
}

/**
 * @brief Operand I of a pack, for writing.
 * @tparam I    Index.
 * @tparam S    First stored operand type.
 * @tparam Rest The others.
 * @param[in,out] a_pack Pack.
 * @return The operand.
 */
template <size_t I, class S, class... Rest>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline auto&
get(Pack<S, Rest...>& a_pack) noexcept
{
  if constexpr (I == 0) {
    return a_pack.m_head;
  }
  else {
    return get<I - 1>(a_pack.m_tail);
  }
}

/**
 * @brief Type I of a list of types.
 * @tparam I  Index.
 * @tparam Cs Types.
 */
template <size_t I, class... Cs>
struct TypeAt;

/**
 * @brief Type 0: the first.
 * @tparam C    First type.
 * @tparam Rest The others.
 */
template <class C, class... Rest>
struct TypeAt<0, C, Rest...>
{
  /**
   * @brief The type.
   */
  using type = C;
};

/**
 * @brief Type I > 0: type I - 1 of the rest.
 * @tparam I    Index.
 * @tparam C    First type.
 * @tparam Rest The others.
 */
template <size_t I, class C, class... Rest>
struct TypeAt<I, C, Rest...>
{
  /**
   * @brief The type.
   */
  using type = typename TypeAt<I - 1, Rest...>::type;
};

/**
 * @brief Whether any of the operand types lives in a pool.
 * @tparam Cs Operand types.
 */
template <class... Cs>
inline constexpr bool anyPoolHandle = (isPoolHandle<Cs> || ...);

/**
 * @brief The worst distance quality among some operand types, and Bound (Exact < Bound < NotADistance).
 * @tparam Cs Operand types.
 * @return The quality.
 */
template <class... Cs>
[[nodiscard]] constexpr DistanceQuality
worstAndBound() noexcept
{
  const DistanceQuality qualities[] = {distanceQualityOf<Cs>...};

  DistanceQuality q = DistanceQuality::Bound;

  for (const DistanceQuality c : qualities) {
    q = (static_cast<int>(c) > static_cast<int>(q)) ? c : q;
  }

  return q;
}

/**
 * @brief A value node's base: no pool, and an unset location for the code that asks for one.
 * @tparam Handle Whether the node lives in a pool.
 */
template <bool Handle>
struct Base
{
  /**
   * @brief An unset location: a value node resolves nothing against a pool.
   * @return The location.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PoolLocation
  location() const noexcept
  {
    return PoolLocation{};
  }
};

/**
 * @brief A pool-resident node's base: a PoolHandle, holding the location its operands resolve against.
 */
template <>
struct Base<true> : PoolHandle
{
  /**
   * @brief An unset location.
   */
  Base() noexcept = default;

  /**
   * @brief Adopt a location.
   * @param[in] a_location Location.
   */
  EBGEOMETRY_HOST_DEVICE
  explicit Base(const PoolLocation& a_location) noexcept : PoolHandle(a_location)
  {}
};

/**
 * @brief A visitor that records whether the leaf it is called with is of type P (for isPrimitive()).
 * @details A struct rather than a lambda: nvcc treats a generic lambda's call operator as a host
 * function where a device function calls it.
 * @tparam P Leaf type.
 */
template <class P>
struct IsLeafOf
{
  /**
   * @brief Whether the leaf was a P.
   */
  bool m_is;

  /**
   * @brief Record whether a leaf is a P.
   * @tparam L Leaf type.
   */
  template <class L>
  EBGEOMETRY_HOST_DEVICE
  inline void
  operator()(const L& /*a_leaf*/) noexcept
  {
    m_is = std::is_same_v<L, P>;
  }
};

/**
 * @brief The operands of a node, with the location their pool-resident ones resolve against.
 * @tparam Cs Operand types.
 */
template <class V, class... Cs>
struct Operands
{
  /**
   * @brief The value type the operands are evaluated to: T, or CSGOps::Tagged<T> for closestPrimitive().
   */
  using Value = V;

  /**
   * @brief The operands, as stored.
   */
  const Pack<CSGDetail::Stored<Cs>...>& m_pack;

  /**
   * @brief The node's location.
   */
  PoolLocation m_location;

  /**
   * @brief The number of the node's first leaf, for tagged values.
   */
  uint32_t m_leafBase;

  /**
   * @brief Number of operands.
   */
  static constexpr size_t count = sizeof...(Cs);

  /**
   * @brief Operand I's value at a point.
   * @tparam I Operand index.
   * @tparam T Floating-point precision.
   * @param[in] a_point Point.
   * @return The value.
   */
  template <size_t I, class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline V
  value(const Vec3T<T>& a_point) const noexcept
  {
    using C = typename TypeAt<I, Cs...>::type;

    if constexpr (CSGOps::isTagged<V>) {
      // A fused operand numbers its own leaves from its first; any other operand is one leaf. Read in
      // place, as below: a copy of a node, which its operands are then read through, is not removed by
      // the compiler, and made closestPrimitive() several times slower.
      constexpr uint32_t offset = leafOffset<I, Cs...>();

      if constexpr (isPoolHandle<C>) {
        return tagged(C(get<I>(m_pack), m_location), a_point, m_leafBase + offset);
      }
      else {
        return tagged(get<I>(m_pack), a_point, m_leafBase + offset);
      }
    }
    else if constexpr (isPoolHandle<C>) {
      return C(get<I>(m_pack), m_location).signedDistance(a_point);
    }
    else {
      return get<I>(m_pack).signedDistance(a_point);
    }
  }

  /**
   * @brief An operand's tagged value: a fused node's own, or the operand's value tagged as one leaf.
   * @tparam C Operand type.
   * @tparam T Floating-point precision.
   * @param[in] a_operand Operand.
   * @param[in] a_point   Point.
   * @param[in] a_leaf    The number of the operand's first leaf.
   * @return The value.
   */
  template <class C, class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline V
  tagged(const C& a_operand, const Vec3T<T>& a_point, const uint32_t a_leaf) noexcept
  {
    if constexpr (isNode<C>) {
      return a_operand.template evaluateValue<V>(a_point, a_leaf);
    }
    else {
      return V{a_operand.signedDistance(a_point), a_leaf, a_point};
    }
  }

  /**
   * @brief Operand I's bounding box.
   * @tparam I Operand index.
   * @return The box.
   */
  template <size_t I>
  [[nodiscard]] EBGEOMETRY_HOST
  inline auto
  box() const
  {
    using C = typename TypeAt<I, Cs...>::type;

    return CSGDetail::fromStored<C>(get<I>(m_pack), m_location).computeBoundingVolume();
  }
};

} // namespace Detail

/**
 * @brief A fused composition: an operation applied to its operands, itself a shape (isShape).
 * @details The type every Fused:: function returns. Op holds the operation's parameters and its
 * arithmetic; the operands are stored as they are, or as their layouts if they live in a pool, in
 * which case the node is a pool handle itself (see EBGeometry_Fused.hpp).
 * @tparam Op Operation (Fused::UnionOp, Fused::AffineOp, ...).
 * @tparam Cs Operand types, all shapes of one precision.
 */
template <class Op, class... Cs>
class Node : public Detail::Base<Detail::anyPoolHandle<Cs...>>
{
public:
  static_assert(sizeof...(Cs) > 0, "Fused::Node needs an operand");
  static_assert((isShape<Cs> && ...), "Fused::Node: every operand must be a shape (see isShape)");

  /**
   * @brief Floating-point precision.
   */
  using T = ShapePrecision<typename Detail::TypeAt<0, Cs...>::type>;

  static_assert((std::is_same_v<ShapePrecision<Cs>, T> && ...), "Fused::Node: the operands' precisions differ");

  /**
   * @brief Whether the node lives in a pool: whether one of its operands does.
   */
  static constexpr bool isHandle = Detail::anyPoolHandle<Cs...>;

  /**
   * @brief How far signedDistance() can be trusted as a distance (see DistanceQuality).
   */
  static constexpr DistanceQuality distanceQuality = Op::template quality<Cs...>();

  /**
   * @brief The number of leaves under the node: each operand that is not itself a fused node counts once.
   */
  static constexpr uint32_t leafCount = (Detail::leavesOf<Cs>() + ...);

  /**
   * @brief Whether the node has a leaf of type P under it.
   * @tparam P Leaf type.
   * @return True if it does.
   */
  template <class P>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr bool
  containsLeaf() noexcept
  {
    return (Detail::hasLeaf<P, Cs>() || ...);
  }

  /**
   * @brief Everything the node holds: the operation's parameters and its operands (their layouts for
   * pool-resident ones). It holds only offsets into a pool, so it is right in every mirror of the pool.
   */
  struct Layout
  {
    /**
     * @brief The operation.
     */
    Op m_op;

    /**
     * @brief The operands, as stored.
     */
    Detail::Pack<CSGDetail::Stored<Cs>...> m_operands;
  };

  /**
   * @brief Disallowed: build a node with a Fused:: function.
   */
  Node() = delete;

  /**
   * @brief A node from its layout, for a node with no pool-resident operand.
   * @param[in] a_layout Layout.
   */
  template <bool H = isHandle, std::enable_if_t<!H, int> = 0>
  EBGEOMETRY_HOST_DEVICE
  explicit Node(const Layout& a_layout) noexcept : m_layout(a_layout)
  {}

  /**
   * @brief A node from its layout and the location its pool-resident operands resolve against.
   * @param[in] a_layout   Layout.
   * @param[in] a_location Location.
   */
  template <bool H = isHandle, std::enable_if_t<H, int> = 0>
  EBGEOMETRY_HOST_DEVICE
  Node(const Layout& a_layout, const PoolLocation& a_location) noexcept
    : Detail::Base<true>(a_location), m_layout(a_layout)
  {}

  /**
   * @brief The node's layout.
   * @return The layout.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Layout&
  layout() const noexcept
  {
    return m_layout;
  }

  /**
   * @brief The composition's value at a point.
   * @param[in] a_point Point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_layout.m_op.evaluate(this->template operands<T>(0U), a_point);
  }

  /**
   * @brief The deciding leaf at a point: the operand whose surface the value comes from there (see
   * PrimitiveRef), with the point in its own frame.
   * @details The leaves are numbered in expression order, depth first, each occurrence counted, as a Tape
   * of the same expression numbers them; m_type and m_index are None. An operand that is not a fused
   * node (a tape, a BVH union, any other shape) is one leaf. Computed by the same arithmetic as
   * signedDistance(), carrying the leaf with each value.
   * @param[in] a_point Query point.
   * @return The leaf and the point in its frame.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PrimitiveRef<T>
  closestPrimitive(const Vec3T<T>& a_point) const noexcept
  {
    const CSGOps::Tagged<T> value = this->template evaluateValue<CSGOps::Tagged<T>>(a_point, 0U);

    PrimitiveRef<T> ref;

    ref.m_leaf       = value.m_tag;
    ref.m_localPoint = value.m_point;

    return ref;
  }

  /**
   * @brief Whether a reference's leaf is of type P.
   * @tparam P Leaf type; the composition must have a leaf of this type (checked when compiling, as the
   * Tape's isPrimitive() checks P against its primitive types).
   * @param[in] a_ref Reference from closestPrimitive().
   * @return True if it is.
   */
  template <class P>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isPrimitive(const PrimitiveRef<T>& a_ref) const noexcept
  {
    static_assert(containsLeaf<P>(), "Fused::Node::isPrimitive: the composition has no leaf of type P");

    Detail::IsLeafOf<P> check{false};

    this->visitPrimitive(a_ref, check);

    return check.m_is;
  }

  /**
   * @brief The leaf a reference names, by value: the shape, or for a pool-resident one (a mesh SDF, a
   * BVH union, a tape) its handle, rebuilt with the node's location.
   * @tparam P Leaf type; the leaf must be one (isPrimitive<P>()).
   * @param[in] a_ref Reference from closestPrimitive().
   * @return The leaf.
   */
  template <class P>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline P
  primitive(const PrimitiveRef<T>& a_ref) const noexcept
  {
    static_assert(containsLeaf<P>(), "Fused::Node::primitive: the composition has no leaf of type P");

    return this->template leafAs<P, 0>(a_ref.m_leaf);
  }

  /**
   * @brief Call a function with the leaf a reference names, as primitive<P>() returns it, whatever its
   * type.
   * @tparam F Callable taking any of the composition's leaf types (a generic lambda in host code).
   * In a CUDA or HIP translation unit, pass a functor whose call operator is EBGEOMETRY_HOST_DEVICE: the
   * device compiler treats a lambda's call operator as a host function here.
   * @param[in] a_ref Reference from closestPrimitive().
   * @param[in] a_f   Function.
   */
  template <class F>
  EBGEOMETRY_HOST_DEVICE
  inline void
  visitPrimitive(const PrimitiveRef<T>& a_ref, F&& a_f) const noexcept
  {
    this->template visitLeaf<0>(a_ref.m_leaf, a_f);
  }

  /**
   * @brief The composition's value as a plain or a tagged value, its leaves numbered from a given one.
   * @details Internal helper of the fused nodes (an enclosing node evaluates its fused operands so); use
   * signedDistance() or closestPrimitive().
   * @tparam V Value type: T, or CSGOps::Tagged<T>.
   * @param[in] a_point    Point.
   * @param[in] a_leafBase The number of the node's first leaf.
   * @return The value.
   */
  template <class V>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline V
  evaluateValue(const Vec3T<T>& a_point, const uint32_t a_leafBase) const noexcept
  {
    return m_layout.m_op.evaluate(this->template operands<V>(a_leafBase), a_point);
  }

  /**
   * @brief The composition's bounding box, by the rules of the expression builder.
   * @return The box.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline BoundingVolumes::AABBT<T>
  computeBoundingVolume() const
  {
    return m_layout.m_op.box(this->template operands<T>(0U));
  }

  /**
   * @brief The same node resolving against a mirror of its pool (see Pool::mirror()); a node with no
   * pool-resident operand is returned as is.
   * @param[in] a_pool Mirror of the pool the node's operands live in.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline Node
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief A copy of the node whose pool-resident operands are copied into another pool; a node with no
   * pool-resident operand is returned as is.
   * @param[in,out] a_dstPool Pool to copy into.
   * @return The copy.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline Node
  deepCopy(Pool& a_dstPool) const;

private:
  template <class Op2, class... Cs2>
  friend class Node;

  /**
   * @brief Everything the node holds.
   */
  Layout m_layout;

  /**
   * @brief The operands, with the location they resolve against.
   * @return The operands.
   */
  template <class V>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Detail::Operands<V, Cs...>
  operands(const uint32_t a_leafBase) const noexcept
  {
    return Detail::Operands<V, Cs...>{m_layout.m_operands, this->location(), a_leafBase};
  }

  /**
   * @brief Operand I, as a shape: as stored, or a pool-resident one rebuilt with the node's location.
   * @tparam I Operand index.
   * @return The operand.
   */
  template <size_t I>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename Detail::TypeAt<I, Cs...>::type
  operand() const noexcept
  {
    using C = typename Detail::TypeAt<I, Cs...>::type;

    return CSGDetail::fromStored<C>(Detail::get<I>(m_layout.m_operands), this->location());
  }

  /**
   * @brief Call a function with leaf `a_leaf` under operands I onwards.
   * @tparam I Operand index.
   * @tparam F Callable.
   * @param[in] a_leaf The leaf's number within this node.
   * @param[in] a_f    Function.
   */
  template <size_t I, class F>
  EBGEOMETRY_HOST_DEVICE
  inline void
  visitLeaf(const uint32_t a_leaf, F& a_f) const noexcept
  {
    if constexpr (I < sizeof...(Cs)) {
      using C = typename Detail::TypeAt<I, Cs...>::type;

      constexpr uint32_t offset = Detail::leafOffset<I, Cs...>();
      constexpr uint32_t count  = Detail::leavesOf<C>();

      // a_leaf - offset wraps around below offset, so one comparison covers both bounds.
      if (a_leaf - offset < count) {
        const C child = this->template operand<I>();

        if constexpr (Detail::isNode<C>) {
          child.template visitLeaf<0>(a_leaf - offset, a_f);
        }
        else {
          a_f(child);
        }

        return;
      }

      this->template visitLeaf<I + 1>(a_leaf, a_f);
    }
  }

  /**
   * @brief Leaf `a_leaf` under operands I onwards, as a P.
   * @details A leaf that is not a P is a caller's error (an assertion); it then returns the first leaf of
   * type P instead.
   * @tparam P Leaf type.
   * @tparam I Operand index.
   * @param[in] a_leaf The leaf's number within this node.
   * @return The leaf.
   */
  template <class P, size_t I>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline P
  leafAs(const uint32_t a_leaf) const noexcept
  {
    using C = typename Detail::TypeAt<I, Cs...>::type;

    constexpr uint32_t offset = Detail::leafOffset<I, Cs...>();
    constexpr uint32_t count  = Detail::leavesOf<C>();

    if constexpr (I + 1 < sizeof...(Cs)) {
      if (!(a_leaf - offset < count)) {
        return this->template leafAs<P, I + 1>(a_leaf);
      }
    }

    if constexpr (Detail::hasLeaf<P, C>()) {
      if constexpr (Detail::isNode<C>) {
        return this->template operand<I>().template leafAs<P, 0>(a_leaf - offset);
      }
      else {
        EBGEOMETRY_EXPECT(a_leaf == offset);

        return this->template operand<I>();
      }
    }
    else {
      EBGEOMETRY_EXPECT(false && "Fused::Node::primitive: the leaf is not of this type");

      return this->template firstLeafAs<P, 0>();
    }
  }

  /**
   * @brief The first leaf of type P under operands I onwards.
   * @tparam P Leaf type.
   * @tparam I Operand index.
   * @return The leaf.
   */
  template <class P, size_t I>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline P
  firstLeafAs() const noexcept
  {
    using C = typename Detail::TypeAt<I, Cs...>::type;

    if constexpr (Detail::hasLeaf<P, C>()) {
      if constexpr (Detail::isNode<C>) {
        return this->template operand<I>().template firstLeafAs<P, 0>();
      }
      else {
        return this->template operand<I>();
      }
    }
    else {
      return this->template firstLeafAs<P, I + 1>();
    }
  }
};

/**
 * @brief Union or intersection: the minimum or the maximum of the operands' values, in order.
 * @tparam IsUnion Whether this is a union.
 */
template <bool IsUnion>
struct CombineOp
{
  /**
   * @brief Distance quality: a bound at best.
   * @tparam Cs Operand types.
   * @return The quality.
   */
  template <class... Cs>
  [[nodiscard]] static constexpr DistanceQuality
  quality() noexcept
  {
    return Detail::worstAndBound<Cs...>();
  }

  /**
   * @brief The value.
   * @tparam O Operands type.
   * @tparam T Floating-point precision.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O, class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  evaluate(const O& a_operands, const Vec3T<T>& a_point) const noexcept
  {
    return this->fold(a_operands, a_point, std::make_index_sequence<O::count - 1>{});
  }

  /**
   * @brief The box: the union, or the intersection, of the operands' boxes.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @return The box.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST
  inline auto
  box(const O& a_operands) const
  {
    return this->boxes(a_operands, std::make_index_sequence<O::count>{});
  }

private:
  /**
   * @brief Operand 0's value, combined in order with the others'.
   * @tparam O  Operands type.
   * @tparam T  Floating-point precision.
   * @tparam Is Indices of the operands after the first, less one.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O, class T, size_t... Is>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  fold(const O& a_operands, const Vec3T<T>& a_point, std::index_sequence<Is...>) const noexcept
  {
    typename O::Value value = a_operands.template value<0>(a_point);

    if constexpr (IsUnion) {
      ((value = CSGOps::smaller(value, a_operands.template value<Is + 1>(a_point))), ...);
    }
    else {
      ((value = CSGOps::larger(value, a_operands.template value<Is + 1>(a_point))), ...);
    }

    return value;
  }

  /**
   * @brief The operands' boxes, combined.
   * @tparam O  Operands type.
   * @tparam Is Operand indices.
   * @param[in] a_operands Operands.
   * @return The box.
   */
  template <class O, size_t... Is>
  [[nodiscard]] EBGEOMETRY_HOST
  inline auto
  boxes(const O& a_operands, std::index_sequence<Is...>) const
  {
    using Box = decltype(a_operands.template box<0>());
    using T   = std::decay_t<decltype(std::declval<Box>().getLowCorner()[0])>;

    Box box = IsUnion ? Box() : ExprDetail::everywhere<T>();

    if constexpr (IsUnion) {
      ((box = box.merged(a_operands.template box<Is>())), ...);
    }
    else {
      ((box = ExprDetail::intersected(box, a_operands.template box<Is>())), ...);
    }

    return box;
  }
};

/**
 * @brief A union: the minimum of the operands' values.
 */
using UnionOp = CombineOp<true>;

/**
 * @brief An intersection: the maximum of the operands' values.
 */
using IntersectionOp = CombineOp<false>;

/**
 * @brief Smooth union or intersection: a smooth minimum (maximum) of the two smallest (largest) values.
 * @tparam T       Floating-point precision.
 * @tparam IsUnion Whether this is a union.
 */
template <class T, bool IsUnion>
struct SmoothCombineOp
{
  /**
   * @brief Smoothing length.
   */
  T m_smooth;

  /**
   * @brief Whether the blend is exponential (BlendKind::Exponential) rather than polynomial.
   */
  bool m_exponential;

  /**
   * @brief Distance quality: a bound at best.
   * @tparam Cs Operand types.
   * @return The quality.
   */
  template <class... Cs>
  [[nodiscard]] static constexpr DistanceQuality
  quality() noexcept
  {
    return Detail::worstAndBound<Cs...>();
  }

  /**
   * @brief The value.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  evaluate(const O& a_operands, const Vec3T<T>& a_point) const noexcept
  {
    return this->blend(a_operands, a_point, std::make_index_sequence<O::count>{});
  }

  /**
   * @brief The box: as a union or intersection's, a union's grown by how far the blend can move its
   * surface out (s/4 polynomial, s ln 2 exponential).
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @return The box.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST
  inline BoundingVolumes::AABBT<T>
  box(const O& a_operands) const
  {
    const BoundingVolumes::AABBT<T> box = CombineOp<IsUnion>{}.box(a_operands);

    if constexpr (IsUnion) {
      return ExprDetail::grown(box, m_exponential ? m_smooth * std::log(T(2)) : T(0.25) * m_smooth);
    }
    else {
      return box;
    }
  }

private:
  /**
   * @brief The pair of extreme values over the operands, blended.
   * @tparam O  Operands type.
   * @tparam Is Operand indices.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O, size_t... Is>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  blend(const O& a_operands, const Vec3T<T>& a_point, std::index_sequence<Is...>) const noexcept
  {
    using V = typename O::Value;

    CSGOps::Pair<V> pair = CSGOps::pairInit<V>(!IsUnion);

    (CSGOps::pairInsert(pair, a_operands.template value<Is>(a_point), !IsUnion), ...);

    return CSGOps::pairBlend(pair, m_smooth, !IsUnion, m_exponential);
  }
};

/**
 * @brief Complement: -v, inside and outside swapped.
 */
struct ComplementOp
{
  /**
   * @brief Distance quality: the operand's.
   * @tparam C Operand type.
   * @return The quality.
   */
  template <class C>
  [[nodiscard]] static constexpr DistanceQuality
  quality() noexcept
  {
    return distanceQualityOf<C>;
  }

  /**
   * @brief The value.
   * @tparam O Operands type.
   * @tparam T Floating-point precision.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O, class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  evaluate(const O& a_operands, const Vec3T<T>& a_point) const noexcept
  {
    return CSGOps::negated(a_operands.template value<0>(a_point));
  }

  /**
   * @brief The box: everywhere.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @return The box.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST
  inline auto
  box(const O& a_operands) const
  {
    using Box = decltype(a_operands.template box<0>());
    using T   = std::decay_t<decltype(std::declval<Box>().getLowCorner()[0])>;

    return ExprDetail::everywhere<T>();
  }
};

/**
 * @brief An affine map (translation, rotation, scaling, reflection, or a composition of them): the
 * operand's value at the mapped point, times a value scale.
 * @tparam T Floating-point precision.
 */
template <class T>
struct AffineOp
{
  /**
   * @brief The map from a query point to the point the operand sees.
   */
  Affine3T<T> m_map;

  /**
   * @brief Its inverse, which maps the operand's box.
   */
  Affine3T<T> m_inverse;

  /**
   * @brief Value scale: |s| for a scaling by s, which keeps a distance a distance.
   */
  T m_valueScale;

  /**
   * @brief Distance quality: the operand's.
   * @tparam C Operand type.
   * @return The quality.
   */
  template <class C>
  [[nodiscard]] static constexpr DistanceQuality
  quality() noexcept
  {
    return distanceQualityOf<C>;
  }

  /**
   * @brief The value.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  evaluate(const O& a_operands, const Vec3T<T>& a_point) const noexcept
  {
    const typename O::Value value = a_operands.template value<0>(CSGOps::affinePoint(m_map, a_point));

    return (m_valueScale != T(1)) ? CSGOps::times(value, m_valueScale) : value;
  }

  /**
   * @brief The box: the operand's, mapped by the inverse.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @return The box.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST
  inline BoundingVolumes::AABBT<T>
  box(const O& a_operands) const
  {
    return m_inverse.transform(a_operands.template box<0>());
  }
};

/**
 * @brief Elongation: the operand's value at p - clamp(p, -h, h).
 * @tparam T Floating-point precision.
 */
template <class T>
struct ElongateOp
{
  /**
   * @brief Half-lengths h of the inserted box.
   */
  Vec3T<T> m_halfLengths;

  /**
   * @brief Distance quality: a bound at best.
   * @tparam C Operand type.
   * @return The quality.
   */
  template <class C>
  [[nodiscard]] static constexpr DistanceQuality
  quality() noexcept
  {
    return Detail::worstAndBound<C>();
  }

  /**
   * @brief The value.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  evaluate(const O& a_operands, const Vec3T<T>& a_point) const noexcept
  {
    return a_operands.template value<0>(CSGOps::elongatePoint(m_halfLengths, a_point));
  }

  /**
   * @brief The box: the operand's, grown by h.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @return The box.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST
  inline BoundingVolumes::AABBT<T>
  box(const O& a_operands) const
  {
    return ExprDetail::grown(a_operands.template box<0>(), m_halfLengths);
  }
};

/**
 * @brief Offset (v - r) or annulus (|v| - delta).
 * @tparam T         Floating-point precision.
 * @tparam IsAnnulus Whether this is an annulus.
 */
template <class T, bool IsAnnulus>
struct ShiftOp
{
  /**
   * @brief The offset r, or the half-thickness delta.
   */
  T m_amount;

  /**
   * @brief Distance quality. An annulus keeps the operand's; an offset is a bound at best, since a
   * negative offset (an erosion) is only a bound, whatever the operand.
   * @tparam C Operand type.
   * @return The quality.
   */
  template <class C>
  [[nodiscard]] static constexpr DistanceQuality
  quality() noexcept
  {
    return IsAnnulus ? distanceQualityOf<C> : Detail::worstAndBound<C>();
  }

  /**
   * @brief The value.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  evaluate(const O& a_operands, const Vec3T<T>& a_point) const noexcept
  {
    const typename O::Value value = a_operands.template value<0>(a_point);

    if constexpr (IsAnnulus) {
      return CSGOps::withValue(value, CSGOps::annular(CSGOps::valueOf(value), m_amount));
    }
    else {
      return CSGOps::withValue(value, CSGOps::offset(CSGOps::valueOf(value), m_amount));
    }
  }

  /**
   * @brief The box: the operand's, grown by max(0, r) or max(0, delta).
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @return The box.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST
  inline BoundingVolumes::AABBT<T>
  box(const O& a_operands) const
  {
    return ExprDetail::grown(a_operands.template box<0>(), Math::max(m_amount, T(0)));
  }
};

/**
 * @brief Finite repetition: the minimum of the operand's value over all its copies
 * (CSGOps::repetition()).
 * @tparam T Floating-point precision.
 */
template <class T>
struct RepetitionOp
{
  /**
   * @brief The repetition: period, tile counts and the operand's box.
   */
  CSGOps::Repetition<T> m_repetition;

  /**
   * @brief Distance quality: a bound at best.
   * @tparam C Operand type.
   * @return The quality.
   */
  template <class C>
  [[nodiscard]] static constexpr DistanceQuality
  quality() noexcept
  {
    return Detail::worstAndBound<C>();
  }

  /**
   * @brief The value.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  evaluate(const O& a_operands, const Vec3T<T>& a_point) const noexcept
  {
    const auto f = [&a_operands](const Vec3T<T>& a_x) noexcept ->
      typename O::Value { return a_operands.template value<0>(a_x); };

    return CSGOps::repetition<T>(m_repetition, a_point, f);
  }

  /**
   * @brief The box: the operand's, swept over the tiles.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @return The box.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST
  inline BoundingVolumes::AABBT<T>
  box(const O& a_operands) const;
};

/**
 * @brief Blur: a weighted average of the operand's values at 27 points (CSGOps::blur()).
 * @tparam T Floating-point precision.
 */
template <class T>
struct BlurOp
{
  /**
   * @brief Blur distance and weights.
   */
  CSGOps::BlurWeights<T> m_weights;

  /**
   * @brief Distance quality: a bound, unless the operand is not a distance.
   * @tparam C Operand type.
   * @return The quality.
   */
  template <class C>
  [[nodiscard]] static constexpr DistanceQuality
  quality() noexcept
  {
    return Detail::worstAndBound<C>();
  }

  /**
   * @brief The value.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  evaluate(const O& a_operands, const Vec3T<T>& a_point) const noexcept
  {
    const auto f = [&a_operands](const Vec3T<T>& a_x) noexcept ->
      typename O::Value { return a_operands.template value<0>(a_x); };

    return CSGOps::blur(m_weights.m_values, a_point, f);
  }

  /**
   * @brief The box: the operand's, grown by sqrt(3) d.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @return The box.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST
  inline BoundingVolumes::AABBT<T>
  box(const O& a_operands) const
  {
    return ExprDetail::grown(a_operands.template box<0>(), std::sqrt(T(3)) * m_weights.m_values[0]);
  }
};

/**
 * @brief Mollification: the operand convolved with a bump kernel sampled on a grid (CSGOps::mollify()).
 * @details With one sample per axis, or a zero distance, the value is the operand's, as Mollify() in the
 * expression builder returns the operand itself.
 * @tparam T Floating-point precision.
 */
template <class T>
struct MollifyOp
{
  /**
   * @brief The tap grid.
   */
  CSGOps::MollifyGrid<T> m_grid;

  /**
   * @brief Whether there is nothing to average over (one sample per axis, or a zero distance).
   */
  bool m_identity;

  /**
   * @brief Distance quality: a bound, unless the operand is not a distance.
   * @tparam C Operand type.
   * @return The quality.
   */
  template <class C>
  [[nodiscard]] static constexpr DistanceQuality
  quality() noexcept
  {
    return Detail::worstAndBound<C>();
  }

  /**
   * @brief The value.
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @param[in] a_point    Point.
   * @return The value.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline typename O::Value
  evaluate(const O& a_operands, const Vec3T<T>& a_point) const noexcept
  {
    if (m_identity) {
      return a_operands.template value<0>(a_point);
    }

    const auto f = [&a_operands](const Vec3T<T>& a_x) noexcept ->
      typename O::Value { return a_operands.template value<0>(a_x); };

    return CSGOps::mollify(m_grid, a_point, f);
  }

  /**
   * @brief The box: the operand's, grown by sqrt(3) r (the operand's own if there is nothing to average).
   * @tparam O Operands type.
   * @param[in] a_operands Operands.
   * @return The box.
   */
  template <class O>
  [[nodiscard]] EBGEOMETRY_HOST
  inline BoundingVolumes::AABBT<T>
  box(const O& a_operands) const
  {
    const BoundingVolumes::AABBT<T> box = a_operands.template box<0>();

    return m_identity ? box : ExprDetail::grown(box, std::sqrt(T(3)) * m_grid.m_radius);
  }
};

/**
 * @brief A list of operands, for the n-ary smooth operations (whose smoothing parameters follow the
 * operands): Fused::SmoothUnion(Fused::operands(a, b, c), s).
 * @tparam Cs Operand types.
 */
template <class... Cs>
struct OperandList
{
  /**
   * @brief The operands.
   */
  Detail::Pack<Cs...> m_items;
};

/**
 * @brief A list of operands for an n-ary smooth operation.
 * @tparam Cs Operand types (shapes).
 * @param[in] a_operands Operands.
 * @return The list.
 */
template <class... Cs>
[[nodiscard]] EBGEOMETRY_HOST
inline OperandList<Cs...>
operands(const Cs&... a_operands);

/**
 * @brief The shape type a Fused:: function returns for an operation and its operands.
 * @tparam Op Operation.
 * @tparam Cs Operand types.
 */
template <class Op, class... Cs>
using NodeOf = Node<Op, Cs...>;

/**
 * @brief Union of shapes: the minimum of their values. One shape is returned as is.
 * @tparam C  First operand type.
 * @tparam Cs The others.
 * @param[in] a_first  First operand.
 * @param[in] a_others The others.
 * @return The union.
 */
template <class C, class... Cs>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
Union(const C& a_first, const Cs&... a_others);

/**
 * @brief Intersection of shapes: the maximum of their values. One shape is returned as is.
 * @tparam C  First operand type.
 * @tparam Cs The others.
 * @param[in] a_first  First operand.
 * @param[in] a_others The others.
 * @return The intersection.
 */
template <class C, class... Cs>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
Intersection(const C& a_first, const Cs&... a_others);

/**
 * @brief Difference A \ (B1 u B2 u ...): max(A, -min_i Bi).
 * @tparam A  Operand to subtract from.
 * @tparam B  First operand to subtract.
 * @tparam Bs The others.
 * @param[in] a_first  Operand to subtract from.
 * @param[in] a_second First operand to subtract.
 * @param[in] a_others The others.
 * @return The difference.
 */
template <class A, class B, class... Bs>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
Difference(const A& a_first, const B& a_second, const Bs&... a_others);

/**
 * @brief Complement: inside and outside swapped, -v.
 * @tparam C Operand type.
 * @param[in] a_operand Operand.
 * @return The complement.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<ComplementOp, C>
Complement(const C& a_operand);

/**
 * @brief Smooth union of two shapes: a smooth minimum of their values.
 * @tparam A First operand type.
 * @tparam B Second operand type.
 * @param[in] a_first  First operand.
 * @param[in] a_second Second operand.
 * @param[in] a_smooth Smoothing length (> 0).
 * @param[in] a_blend  Blend kind.
 * @return The smooth union.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<SmoothCombineOp<ShapePrecision<A>, true>, A, B>
SmoothUnion(const A&                a_first,
            const B&                a_second,
            const ShapePrecision<A> a_smooth,
            const BlendKind         a_blend = BlendKind::Polynomial);

/**
 * @brief Smooth union of any number of shapes: a smooth minimum of the two smallest values (see
 * EBGeometry::SmoothUnion()).
 * @tparam Cs Operand types.
 * @param[in] a_operands Operands (Fused::operands(a, b, ...)). One is returned as is.
 * @param[in] a_smooth   Smoothing length (> 0).
 * @param[in] a_blend    Blend kind.
 * @return The smooth union.
 */
template <class... Cs>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
SmoothUnion(const OperandList<Cs...>&                                     a_operands,
            const ShapePrecision<typename Detail::TypeAt<0, Cs...>::type> a_smooth,
            const BlendKind                                               a_blend = BlendKind::Polynomial);

/**
 * @brief Smooth intersection of two shapes: a smooth maximum of their values.
 * @tparam A First operand type.
 * @tparam B Second operand type.
 * @param[in] a_first  First operand.
 * @param[in] a_second Second operand.
 * @param[in] a_smooth Smoothing length (> 0).
 * @param[in] a_blend  Blend kind.
 * @return The smooth intersection.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<SmoothCombineOp<ShapePrecision<A>, false>, A, B>
SmoothIntersection(const A&                a_first,
                   const B&                a_second,
                   const ShapePrecision<A> a_smooth,
                   const BlendKind         a_blend = BlendKind::Polynomial);

/**
 * @brief Smooth intersection of any number of shapes: a smooth maximum of the two largest values.
 * @tparam Cs Operand types.
 * @param[in] a_operands Operands (Fused::operands(a, b, ...)). One is returned as is.
 * @param[in] a_smooth   Smoothing length (> 0).
 * @param[in] a_blend    Blend kind.
 * @return The smooth intersection.
 */
template <class... Cs>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
SmoothIntersection(const OperandList<Cs...>&                                     a_operands,
                   const ShapePrecision<typename Detail::TypeAt<0, Cs...>::type> a_smooth,
                   const BlendKind                                               a_blend = BlendKind::Polynomial);

/**
 * @brief Smooth difference A \ B: the smooth intersection of A and the complement of B.
 * @tparam A Operand to subtract from.
 * @tparam B Operand to subtract.
 * @param[in] a_first  Operand to subtract from.
 * @param[in] a_second Operand to subtract.
 * @param[in] a_smooth Smoothing length (> 0).
 * @param[in] a_blend  Blend kind.
 * @return The smooth difference.
 */
template <class A, class B>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
SmoothDifference(const A&                a_first,
                 const B&                a_second,
                 const ShapePrecision<A> a_smooth,
                 const BlendKind         a_blend = BlendKind::Polynomial);

/**
 * @brief Smooth difference A \ (B1, B2, ...): the smooth intersection of A and the complement of each Bi.
 * @tparam A  Operand to subtract from.
 * @tparam Bs Operands to subtract.
 * @param[in] a_first  Operand to subtract from.
 * @param[in] a_others Operands to subtract (Fused::operands(b1, b2, ...)).
 * @param[in] a_smooth Smoothing length (> 0).
 * @param[in] a_blend  Blend kind.
 * @return The smooth difference.
 */
template <class A, class... Bs>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
SmoothDifference(const A&                  a_first,
                 const OperandList<Bs...>& a_others,
                 const ShapePrecision<A>   a_smooth,
                 const BlendKind           a_blend = BlendKind::Polynomial);

/**
 * @brief Translate a shape by a shift.
 * @tparam C Operand type.
 * @param[in] a_operand Operand.
 * @param[in] a_shift   Shift (finite).
 * @return The translated shape. A map of a map is one map, as in the expression builder.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
Translate(const C& a_operand, const Vec3T<ShapePrecision<C>>& a_shift);

/**
 * @brief Rotate a shape about a coordinate axis through the origin.
 * @tparam C Operand type.
 * @param[in] a_operand Operand.
 * @param[in] a_degrees Angle in degrees, counterclockwise seen from the positive end of the axis (finite).
 * @param[in] a_axis    Coordinate axis (0, 1 or 2).
 * @return The rotated shape.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
Rotate(const C& a_operand, const ShapePrecision<C> a_degrees, const size_t a_axis);

/**
 * @brief Rotate a shape about an arbitrary axis through the origin.
 * @tparam C Operand type.
 * @param[in] a_operand Operand.
 * @param[in] a_degrees Angle in degrees, counterclockwise seen from the positive end of the axis (finite).
 * @param[in] a_axis    Axis direction (finite, non-zero; need not be normalized).
 * @return The rotated shape.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
Rotate(const C& a_operand, const ShapePrecision<C> a_degrees, const Vec3T<ShapePrecision<C>>& a_axis);

/**
 * @brief Scale a shape uniformly about the origin: |s| times its value at p / s.
 * @tparam C Operand type.
 * @param[in] a_operand Operand.
 * @param[in] a_factor  Scale factor s (finite, non-zero; a negative factor also reflects through the
 * origin).
 * @return The scaled shape.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
Scale(const C& a_operand, const ShapePrecision<C> a_factor);

/**
 * @brief Reflect a shape in a coordinate plane through the origin.
 * @tparam C Operand type.
 * @param[in] a_operand Operand.
 * @param[in] a_axis    Axis normal to the plane (0, 1 or 2).
 * @return The reflected shape.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline auto
Reflect(const C& a_operand, const size_t a_axis);

/**
 * @brief Offset a shape's surface outward by a distance, v - r.
 * @details A bound (an offset of an exact distance by r >= 0 is exact, but the type cannot tell the
 * sign of r).
 * @tparam C Operand type.
 * @param[in] a_operand Operand.
 * @param[in] a_offset  Offset distance r (finite); negative erodes.
 * @return The offset shape.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<ShiftOp<ShapePrecision<C>, false>, C>
Offset(const C& a_operand, const ShapePrecision<C> a_offset);

/**
 * @brief A shell around a shape's surface, |v| - delta.
 * @tparam C Operand type.
 * @param[in] a_operand Operand.
 * @param[in] a_delta   Shell half-thickness (finite).
 * @return The shell.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<ShiftOp<ShapePrecision<C>, true>, C>
Annular(const C& a_operand, const ShapePrecision<C> a_delta);

/**
 * @brief Elongate a shape: stretch it by inserting a box of half-lengths h at the origin.
 * @tparam C Operand type.
 * @param[in] a_operand    Operand.
 * @param[in] a_elongation Half-lengths h (finite, >= 0).
 * @return The elongated shape.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<ElongateOp<ShapePrecision<C>>, C>
Elongate(const C& a_operand, const Vec3T<ShapePrecision<C>>& a_elongation);

/**
 * @brief Finite repetition: copies of a shape on a grid of tiles, the minimum over all of them (see
 * EBGeometry::FiniteRepetition()).
 * @tparam C Operand type.
 * @param[in] a_operand  Shape to repeat; its home copy sits in the tile around the origin.
 * @param[in] a_period   Tile period along each axis (> 0).
 * @param[in] a_repeatLo Tiles in the decreasing direction of each axis (>= 0; rounded to whole tiles).
 * @param[in] a_repeatHi Tiles in the increasing direction of each axis (>= 0; rounded to whole tiles).
 * @return The repetition.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<RepetitionOp<ShapePrecision<C>>, C>
FiniteRepetition(const C&                        a_operand,
                 const Vec3T<ShapePrecision<C>>& a_period,
                 const Vec3T<ShapePrecision<C>>& a_repeatLo,
                 const Vec3T<ShapePrecision<C>>& a_repeatHi);

/**
 * @brief Blur a shape: a weighted average of its values at the 27 points p + d (i, j, k), i, j, k in
 * {-1, 0, 1} (see EBGeometry::Blur()).
 * @tparam C Operand type.
 * @param[in] a_operand Operand.
 * @param[in] a_blur    Blur distance d (finite, >= 0).
 * @param[in] a_alpha   Centre weight factor in [0, 1]; 1 = no blur.
 * @return The blurred shape.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<BlurOp<ShapePrecision<C>>, C>
Blur(const C& a_operand, const ShapePrecision<C> a_blur, const ShapePrecision<C> a_alpha = ShapePrecision<C>(0.5));

/**
 * @brief Mollify a shape: convolve it with a smooth bump kernel sampled on an n x n x n grid over
 * [-r, r]^3 (see EBGeometry::Mollify()).
 * @tparam C Operand type.
 * @param[in] a_operand Operand.
 * @param[in] a_dist    Mollification distance r: the half-width of the sampling cube (finite).
 * @param[in] a_samples Samples per axis n (between 1 and 64).
 * @return The mollified shape.
 */
template <class C>
[[nodiscard]] EBGEOMETRY_HOST
inline Node<MollifyOp<ShapePrecision<C>>, C>
Mollify(const C& a_operand, const ShapePrecision<C> a_dist, const size_t a_samples);

} // namespace Fused

} // namespace EBGeometry

#include "EBGeometry_FusedImplem.hpp"

#endif
