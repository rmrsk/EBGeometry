// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Tape.hpp
 * @brief  Declaration of Tape, a composed object (an Expr) compiled into a pool, and its evaluator.
 * @details A Tape stores its shapes in one array per shape type and its operations as a list of clauses
 * that refer to them by type and index, so evaluating it calls no virtual function and follows no
 * pointer. It is a pool handle (see PoolHandle): trivially copyable, and evaluated against whichever
 * mirror of its pool it is rebased onto. The evaluator keeps to the device subset (no recursion, no
 * function pointers, no exceptions); operations that evaluate a subtree several times (Blur, Mollify
 * and BVH-accelerated unions) call a sub-program through a call-depth template, so nesting is bounded
 * at compile time.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TAPE_HPP
#define EBGEOMETRY_TAPE_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_AnalyticDistanceFunctions.hpp"
#include "EBGeometry_BVHUnion.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Settings for compiling an Expr into a Tape.
 */
struct TapeOptions
{
  /**
   * @brief A union or smooth union of at least this many objects is evaluated with a BVH over the
   * objects' boxes (unless one of them is not a distance); a smaller one is evaluated object by object.
   */
  uint32_t m_bvhUnionThreshold = 8;

  /**
   * @brief How those BVHs are built.
   */
  BVH::Construction m_construction = BVH::Construction::SAH;
};

namespace TapeDetail {

/**
 * @brief The tape's operations.
 * @details Internal helper; not part of the public API. A sub-program runs its clauses in order on a
 * stack of values and a stack of points; its input point is on the point stack when it starts, and its
 * value is the one value left when it ends.
 */
enum class Op : uint8_t
{
  Primitive,      ///< Push the value of primitive (m_type, m_a) at the top point.
  PushAffine,     ///< Push the top point mapped by the affine map at params[m_a..m_a+12).
  PushElongate,   ///< Push p - clamp(p, -h, h), h at params[m_a..m_a+3).
  PopPoint,       ///< Pop a point.
  Negate,         ///< v -> -v.
  Abs,            ///< v -> |v|.
  AddConstant,    ///< v -> v + params[m_a].
  MulConstant,    ///< v -> v * params[m_a].
  Min,            ///< Pop b and a, push min(a, b).
  Max,            ///< Pop b and a, push max(a, b).
  PairInit,       ///< Push the empty pair of extreme values (+inf, +inf), or (-inf, -inf) if m_flags is 1.
  PairInsert,     ///< Pop v and insert it into the pair below: the two smallest, or largest if m_flags is 1.
  PairBlend,      ///< Pop the pair and push its smooth min (or max if bit 0 of m_flags is set), with the
                  ///< exponential blend if bit 1 is set; smoothing length params[m_a].
  Blur,           ///< Push the 27-tap blur of sub-program m_a; distance and weights at params[m_b..m_b+5).
  Mollify,        ///< Push sum_k w_k f(p - o_k) over m_c taps of sub-program m_a; (o_k, w_k) at params[m_b..).
  Repeat,         ///< Push the minimum over the copies of sub-program m_a (CSGOps::repetition()); period,
                  ///< low and high counts and the object's box at params[m_b..m_b+15).
  UnionBVH,       ///< Push the minimum over the sub-programs in BVH union m_a.
  SmoothUnionBVH, ///< Push the smooth minimum of the two smallest sub-programs in BVH union m_a; smoothing
                  ///< length params[m_b], exponential blend if m_flags is 1.
};

/**
 * @brief One tape clause.
 * @details Internal helper; not part of the public API. The meaning of each field depends on m_op.
 */
struct Clause
{
  /**
   * @brief Operation.
   */
  Op m_op = Op::Primitive;

  /**
   * @brief Operation flags.
   */
  uint8_t m_flags = 0;

  /**
   * @brief Primitive type index (Primitive).
   */
  uint16_t m_type = 0;

  /**
   * @brief First operand.
   */
  uint32_t m_a = 0;

  /**
   * @brief Second operand.
   */
  uint32_t m_b = 0;

  /**
   * @brief Third operand.
   */
  uint32_t m_c = 0;
};

static_assert(sizeof(Clause) == 16, "TapeDetail::Clause must be 16 bytes");

/**
 * @brief A sub-program: a range of clauses.
 * @details Internal helper; not part of the public API.
 */
struct Program
{
  /**
   * @brief First clause.
   */
  uint32_t m_begin = 0;

  /**
   * @brief One past the last clause.
   */
  uint32_t m_end = 0;
};

/**
 * @brief A BVH union's two parts, as layouts in the tape's pool: a BVH over the sub-programs with bounded
 * boxes, and a single leaf (or nothing) holding those with unbounded ones.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 */
template <class T, size_t K>
struct UnionBVH
{
  /**
   * @brief BVH over the sub-programs with bounded boxes.
   */
  typename BVH::PackedBVH<T, uint32_t, K>::Layout m_bvh;

  /**
   * @brief Leaf holding the sub-programs with unbounded boxes.
   */
  typename BVH::PackedBVH<T, uint32_t, K>::Layout m_unbounded;
};

/**
 * @brief A list of types.
 * @details Internal helper; not part of the public API.
 * @tparam Ps Types.
 */
template <class... Ps>
struct TypeList
{
  /**
   * @brief Number of types.
   */
  static constexpr size_t size = sizeof...(Ps);
};

/**
 * @brief The I-th type of a TypeList.
 * @details Internal helper; not part of the public API.
 * @tparam I    Index.
 * @tparam List TypeList.
 */
template <size_t I, class List>
struct TypeAt;

/**
 * @brief The I-th type of a TypeList.
 * @details Internal helper; not part of the public API.
 * @tparam I  Index.
 * @tparam Ps Types.
 */
template <size_t I, class... Ps>
struct TypeAt<I, TypeList<Ps...>>
{
  /**
   * @brief The type.
   */
  using type = std::tuple_element_t<I, std::tuple<Ps...>>;
};

/**
 * @brief One pool array per primitive type, holding the type's stored form (CSGDetail::Stored).
 * @details Internal helper; not part of the public API. The empty store ends the recursion.
 * @tparam Ps Primitive types.
 */
template <class... Ps>
struct PrimitiveStore
{
};

/**
 * @brief One pool array per primitive type: the first type's, then the rest.
 * @details Internal helper; not part of the public API.
 * @tparam P  First primitive type.
 * @tparam Ps Remaining primitive types.
 */
template <class P, class... Ps>
struct PrimitiveStore<P, Ps...>
{
  /**
   * @brief The first type's primitives.
   */
  PODVector<CSGDetail::Stored<P>> m_head;

  /**
   * @brief The remaining types' primitives.
   */
  PrimitiveStore<Ps...> m_tail;
};

/**
 * @brief The PrimitiveStore for the types of a TypeList.
 * @details Internal helper; not part of the public API.
 * @tparam List TypeList.
 */
template <class List>
struct StoreOf;

/**
 * @brief The PrimitiveStore for the types of a TypeList.
 * @details Internal helper; not part of the public API.
 * @tparam Ps Types.
 */
template <class... Ps>
struct StoreOf<TypeList<Ps...>>
{
  /**
   * @brief The store.
   */
  using type = PrimitiveStore<Ps...>;
};

/**
 * @brief The pool array of the I-th primitive type.
 * @details Internal helper; not part of the public API.
 * @tparam I  Type index.
 * @tparam P  First primitive type.
 * @tparam Ps Remaining primitive types.
 * @param[in] a_store Store.
 * @return The array.
 */
template <size_t I, class P, class... Ps>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline const auto&
storeAt(const PrimitiveStore<P, Ps...>& a_store) noexcept
{
  if constexpr (I == 0) {
    return a_store.m_head;
  }
  else {
    return storeAt<I - 1>(a_store.m_tail);
  }
}

/**
 * @brief The pool array of the I-th primitive type.
 * @details Internal helper; not part of the public API.
 * @tparam I  Type index.
 * @tparam P  First primitive type.
 * @tparam Ps Remaining primitive types.
 * @param[in,out] a_store Store.
 * @return The array.
 */
template <size_t I, class P, class... Ps>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline auto&
storeAt(PrimitiveStore<P, Ps...>& a_store) noexcept
{
  if constexpr (I == 0) {
    return a_store.m_head;
  }
  else {
    return storeAt<I - 1>(a_store.m_tail);
  }
}

/**
 * @brief The tape's primitive types: the twelve analytic shapes, the three mesh SDFs with the default
 * branching factor and SIMD width, then the user's types.
 * @details Internal helper; not part of the public API.
 * @tparam T            Floating-point precision.
 * @tparam UserPrimitives User primitive types.
 */
template <class T, class... UserPrimitives>
using Primitives = TypeList<PlaneSDF<T>,
                            SphereSDF<T>,
                            BoxSDF<T>,
                            TorusSDF<T>,
                            CylinderSDF<T>,
                            InfiniteCylinderSDF<T>,
                            CapsuleSDF<T>,
                            InfiniteConeSDF<T>,
                            ConeSDF<T>,
                            RoundedBoxSDF<T>,
                            RoundedCylinderSDF<T>,
                            PerlinSDF<T>,
                            FlatMeshSDF<T>,
                            MeshSDF<T, BVH::DefaultBranchingRatio<T>()>,
                            TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>,
                            UserPrimitives...>;

/**
 * @brief Host-side arrays the compiler fills before they are reserved in the pool.
 * @details Internal helper; not part of the public API.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor.
 * @tparam List TypeList of primitive types.
 */
template <class T, size_t K, class List>
struct Staging;

} // namespace TapeDetail

/**
 * @brief A composed object compiled into a pool: a trivially copyable handle that evaluates the object's
 * signed distance (or implicit function value) on the host or on a device.
 * @details Build one from an Expr:
 * @code
 *   Pool pool(hostMemoryResource());
 *   const auto shape = Difference(Union(SphereSDF<T>(c, r), Translate(BoxSDF<T>(lo, hi), shift)), mesh);
 *   const Tape<T> tape(pool, shape);
 *   const T d = tape.signedDistance(p);
 * @endcode
 * The analytic shapes and the mesh SDFs (FlatMeshSDF, and MeshSDF and TriMeshSDF with the default branching
 * factor and width) are built-in primitive types; any other shape type, such as a BVHUnion, a MeshSDF with
 * another branching factor, another Tape or a type of your own, is listed in UserPrimitives. A
 * pool-resident primitive must live in the tape's pool. The pool must outlive the tape and every copy and
 * view of it (see PoolHandle).
 *
 * The compiler works out each sub-expression's bounding box and distance quality (see Expr). A union or
 * smooth union of many objects is evaluated with a BVH over their boxes, pruned as BVHUnion is, unless an
 * object is not a distance. With exact distances the value is the plain minimum; with operands that are
 * only bounds (DistanceQuality::Bound) pruning can skip an operand whose value is below its box's distance,
 * so the value can be larger than the plain minimum, but it keeps the sign and is still a bound. Blur, Mollify and BVH unions evaluate a sub-program several times; they nest
 * at most MaxCallDepth deep, and a sub-program needs at most ValueStackSize values and PointStackSize
 * points at once; the constructor rejects an expression that needs more.
 * @tparam T              Floating-point precision.
 * @tparam UserPrimitives Additional primitive types (each a shape of precision T).
 */
template <class T, class... UserPrimitives>
class Tape : public PoolHandle
{
public:
  static_assert(std::is_floating_point_v<T>, "Tape requires a floating-point type T");
  static_assert((isShape<UserPrimitives> && ...), "Tape: every user primitive type must be a shape (see isShape)");
  static_assert((std::is_same_v<ShapePrecision<UserPrimitives>, T> && ...),
                "Tape: every user primitive type must have the tape's precision");

  /**
   * @brief Bounding box type.
   */
  using BV = BoundingVolumes::AABBT<T>;

  /**
   * @brief Branching factor of the BVHs over union operands.
   */
  static constexpr size_t BranchingRatio = 4;

  /**
   * @brief How deep Blur, Mollify and BVH unions may nest.
   */
  static constexpr unsigned MaxCallDepth = 4;

  /**
   * @brief Values a sub-program may hold at once.
   */
  static constexpr uint32_t ValueStackSize = 16;

  /**
   * @brief Points a sub-program may hold at once (its input point included).
   */
  static constexpr uint32_t PointStackSize = 8;

  /**
   * @brief The primitive types: the built-in ones, then UserPrimitives.
   */
  using Primitives = TapeDetail::Primitives<T, UserPrimitives...>;

  /**
   * @brief The BVH type over union operands.
   */
  using UnionRoot = BVH::PackedBVH<T, uint32_t, BranchingRatio>;

  /**
   * @brief The tape's arrays as offsets into its pool, with no location (see PoolHandle).
   */
  struct Layout
  {
    /**
     * @brief One array per primitive type.
     */
    typename TapeDetail::StoreOf<Primitives>::type m_primitives;

    /**
     * @brief The clauses of every sub-program, one after another.
     */
    PODVector<TapeDetail::Clause> m_clauses;

    /**
     * @brief Each sub-program's clause range; sub-program 0 is the whole object.
     */
    PODVector<TapeDetail::Program> m_programs;

    /**
     * @brief Constant parameters the clauses refer to.
     */
    PODVector<T> m_params;

    /**
     * @brief The BVH unions.
     */
    PODVector<TapeDetail::UnionBVH<T, BranchingRatio>> m_unions;

    /**
     * @brief A box enclosing the object.
     */
    BV m_boundingVolume;

    /**
     * @brief How far the value can be trusted as a distance.
     */
    DistanceQuality m_quality = DistanceQuality::NotADistance;

    /**
     * @brief How deep sub-program calls nest from sub-program 0.
     */
    uint32_t m_callDepth = 0;
  };

  /**
   * @brief No empty tape.
   */
  Tape() = delete;

  /**
   * @brief Compile an expression into a pool.
   * @param[in,out] a_pool    Pool to compile into; pool-resident primitives must live in it too. Must
   * outlive this object and every copy of it.
   * @param[in]     a_expr    Expression.
   * @param[in]     a_options Compile settings.
   */
  EBGEOMETRY_HOST
  Tape(Pool& a_pool, const Expr<T>& a_expr, const TapeOptions& a_options = TapeOptions());

  /**
   * @brief Rebuild a handle from a stored layout and the location of the pool that holds it.
   * @details How an outer pool-resident object evaluates a Tape it stores as a layout. @p a_location must
   * belong to the pool the layout was reserved from, or to a mirror of it; nothing is checked, since a
   * device has no pool to check against.
   * @param[in] a_layout   The arrays.
   * @param[in] a_location Where they live.
   */
  EBGEOMETRY_HOST_DEVICE
  Tape(const Layout& a_layout, const PoolLocation& a_location) noexcept : PoolHandle(a_location), m_layout(a_layout)
  {}

  /**
   * @brief The object's value at a point: negative inside, positive outside, zero on the surface.
   * @details The sign is always meaningful. The magnitude is what getDistanceQuality() says: the
   * distance to the surface (Exact), at most that distance (Bound), or nothing (NotADistance). The
   * name is the same for every shape whatever its quality; see isShape. Callable on a device through a
   * view onto a mirrored pool (rebasedView()).
   * @param[in] a_point Query point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief A box enclosing the object.
   * @return The box (unbounded along directions where the object is).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BV
  computeBoundingVolume() const noexcept;

  /**
   * @brief How far signedDistance() can be trusted as a distance.
   * @details A runtime value, unlike a shape's static `distanceQuality`; a Tape used as another object's
   * primitive therefore counts as DistanceQuality::Bound there.
   * @return The quality.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline DistanceQuality
  getDistanceQuality() const noexcept;

  /**
   * @brief Number of sub-programs (1 plus one per Blur, Mollify and BVH union operand).
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumPrograms() const noexcept;

  /**
   * @brief Number of clauses over all sub-programs.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumClauses() const noexcept;

  /**
   * @brief Number of unions evaluated with a BVH.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumBVHUnions() const noexcept;

  /**
   * @brief How deep sub-program calls nest (0 if the tape has no Blur, Mollify or BVH union).
   * @return The depth.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getCallDepth() const noexcept;

  /**
   * @brief The tape's arrays, with no location: what an outer pool-resident object stores.
   * @return The layout.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Layout&
  layout() const noexcept;

  /**
   * @brief A view of this tape resolving against another pool of its mirror chain.
   * @details The pool must be this tape's own pool or a mirror of it (see PoolLocation::rebasedOnto()).
   * Pool-resident primitives follow the view, since they are stored as layouts.
   * @param[in] a_pool Pool to resolve against.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline Tape
  rebasedView(const Pool& a_pool) const noexcept;

  /**
   * @brief Copy the tape, and every pool-resident primitive and BVH it holds, into another pool.
   * @param[in,out] a_dstPool Pool to copy into.
   * @return The copy, resolving against a_dstPool.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline Tape
  deepCopy(Pool& a_dstPool) const;

private:
  /**
   * @brief The arrays.
   */
  Layout m_layout;

  /**
   * @brief Evaluate a sub-program at a point.
   * @tparam D How deep the sub-program's own calls may still nest.
   * @param[in] a_program  Sub-program.
   * @param[in] a_point    Point.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location, for pool-resident primitives and BVHs.
   * @return The value.
   */
  template <unsigned D>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  evaluate(const uint32_t      a_program,
           const Vec3T<T>&     a_point,
           const void*         a_base,
           const PoolLocation& a_location) const noexcept;

  /**
   * @brief The body of evaluate(), inlined into it, and into signedDistance() for a tape that calls no
   * sub-program (at depth 0 the evaluator calls no level below, so inlining it multiplies nothing).
   * @tparam D How deep the sub-program's own calls may still nest.
   * @param[in] a_program  Sub-program.
   * @param[in] a_point    Point.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location, for pool-resident primitives and BVHs.
   * @return The value.
   */
  template <unsigned D>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  evaluateBody(const uint32_t      a_program,
               const Vec3T<T>&     a_point,
               const void*         a_base,
               const PoolLocation& a_location) const noexcept;

  /**
   * @brief Evaluate a primitive, dispatching on its type index from I onwards.
   * @tparam I       First type index to test.
   * @tparam Handles Whether this is the out-of-line dispatch for pool-resident primitives (see
   * evaluateHandle()); otherwise they are handed to it.
   * @param[in] a_type     Type index.
   * @param[in] a_index    Index within the type's array.
   * @param[in] a_point    Point.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The value.
   */
  template <size_t I, bool Handles>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  dispatchPrimitive(const uint32_t      a_type,
                    const uint32_t      a_index,
                    const Vec3T<T>&     a_point,
                    const void*         a_base,
                    const PoolLocation& a_location) const noexcept;

  /**
   * @brief Evaluate a primitive: an analytic shape in place, a pool-resident one through evaluateHandle().
   * @param[in] a_type     Type index.
   * @param[in] a_index    Index within the type's array.
   * @param[in] a_point    Point.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  evaluatePrimitive(const uint32_t      a_type,
                    const uint32_t      a_index,
                    const Vec3T<T>&     a_point,
                    const void*         a_base,
                    const PoolLocation& a_location) const noexcept;

  /**
   * @brief Evaluate a pool-resident primitive (a mesh SDF, a BVH union, a tape).
   * @details Out of line in a device pass, so that the stack such a primitive needs (a mesh SDF's
   * traversal stack, say) is taken only when one is evaluated, once below the deepest call, rather than in
   * the frame every clause is evaluated in.
   * @param[in] a_type     Type index.
   * @param[in] a_index    Index within the type's array.
   * @param[in] a_point    Point.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  evaluateHandle(const uint32_t      a_type,
                 const uint32_t      a_index,
                 const Vec3T<T>&     a_point,
                 const void*         a_base,
                 const PoolLocation& a_location) const noexcept;

  /**
   * @brief Evaluate a Blur clause: the weighted average of its sub-program at 27 taps around the point.
   * @tparam D Call depth left for the sub-program.
   * @param[in] a_clause   The clause.
   * @param[in] a_point    Point.
   * @param[in] a_params   The parameters.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The blurred value.
   */
  template <unsigned D>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  evaluateBlur(const TapeDetail::Clause& a_clause,
               const Vec3T<T>&           a_point,
               const T*                  a_params,
               const void*               a_base,
               const PoolLocation&       a_location) const noexcept;

  /**
   * @brief Evaluate a Repeat clause: the minimum of its sub-program over the copies (CSGOps::repetition()).
   * @tparam D Call depth left for the sub-program.
   * @param[in] a_clause   The clause.
   * @param[in] a_point    Point.
   * @param[in] a_params   The parameters.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The value.
   */
  template <unsigned D>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  evaluateRepeat(const TapeDetail::Clause& a_clause,
                 const Vec3T<T>&           a_point,
                 const T*                  a_params,
                 const void*               a_base,
                 const PoolLocation&       a_location) const noexcept;

  /**
   * @brief Evaluate a BVH union clause.
   * @tparam D Call depth left for the operands.
   * @param[in] a_clause   The clause.
   * @param[in] a_point    Point.
   * @param[in] a_params   The parameters.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The union's value.
   */
  template <unsigned D>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  evaluateUnion(const TapeDetail::Clause& a_clause,
                const Vec3T<T>&           a_point,
                const T*                  a_params,
                const void*               a_base,
                const PoolLocation&       a_location) const noexcept;

  /**
   * @brief Reserve the staged arrays in a pool and attach to it.
   * @param[in,out] a_pool    Pool.
   * @param[in]     a_staging Staged arrays.
   */
  EBGEOMETRY_HOST
  void
  finalize(Pool& a_pool, const TapeDetail::Staging<T, BranchingRatio, Primitives>& a_staging);
};

} // namespace EBGeometry

#include "EBGeometry_TapeImplem.hpp"

#endif
