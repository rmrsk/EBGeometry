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
 *
 * Beside the clauses, the compiler writes a ray program for the tape and for each BVH union operand: the
 * expression's unions, intersections, complements and affine maps over the primitives that have ray forms,
 * in the same order, which Tape's ray form follows to trace the tape exactly along a ray (see
 * EBGeometry_RaySpans.hpp). A subtree with anything else in it is marched as a whole, through its clauses.
 * The evaluator does not read the ray programs.
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
#include "EBGeometry_CSGRayForms.hpp"
#include "EBGeometry_DistanceQuality.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_PrimitiveRef.hpp"
#include "EBGeometry_Ray.hpp"
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
 * @brief The operations of a ray program.
 * @details Internal helper; not part of the public API. A ray program runs its clauses in order on a stack
 * of span lists (RaySpansT) and a stack of rays; its ray is on the ray stack when it starts, and its list
 * is the one list left when it ends. Bit 0 of a clause's m_flags says that all its spans are needed, not
 * only its first crossing.
 */
enum class RayOp : uint8_t
{
  Leaf,         ///< Push the spans of primitive (m_type, m_a), leaf number m_b, along the top ray.
  Marched,      ///< Push the spans of sub-program m_a along the top ray, marched in the box at
                ///< params[m_b..m_b+6), with distance quality m_c. With bit 1 of m_flags, it is an operand of an
                ///< intersection, marched only within the spans of the top list, the operands' before it (see
                ///< RayDetail::spansWithin()); with bit 2 as well, the intersection needs only its first
                ///< crossing.
  Union,        ///< Pop two lists and push their union.
  Intersection, ///< Pop two lists and push their intersection.
  Complement,   ///< Replace the top list by its complement.
  PushAffine,   ///< Push the top ray mapped by the affine map at params[m_a..m_a+12).
  PopAffine,    ///< Pop a ray, and map the top list back through the map at params[m_a..m_a+12).
  UnionBVH      ///< Push the union of the spans of the sub-programs in BVH union m_a; its operands, with the
                ///< sub-programs they call, are sub-programs m_b up to m_c.
};

/**
 * @brief One ray program clause.
 * @details Internal helper; not part of the public API. The meaning of each field depends on m_op.
 */
struct RayClause
{
  /**
   * @brief Operation.
   */
  RayOp m_op = RayOp::Leaf;

  /**
   * @brief Operation flags.
   */
  uint8_t m_flags = 0;

  /**
   * @brief Primitive type index (Leaf).
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

static_assert(sizeof(RayClause) == 16, "TapeDetail::RayClause must be 16 bytes");

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
 * @brief A ray program clause, stored in the clause array (the two have the same fields).
 * @details Internal helper; not part of the public API.
 * @param[in] a_clause Ray program clause.
 * @return The clause it is stored as.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Clause
stored(const RayClause& a_clause) noexcept
{
  Clause c;

  c.m_op    = static_cast<Op>(a_clause.m_op);
  c.m_flags = a_clause.m_flags;
  c.m_type  = a_clause.m_type;
  c.m_a     = a_clause.m_a;
  c.m_b     = a_clause.m_b;
  c.m_c     = a_clause.m_c;

  return c;
}

/**
 * @brief The ray program clause a stored clause is.
 * @details Internal helper; not part of the public API.
 * @param[in] a_clause Stored clause.
 * @return The ray program clause.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline RayClause
rayClause(const Clause& a_clause) noexcept
{
  RayClause c;

  c.m_op    = static_cast<RayOp>(a_clause.m_op);
  c.m_flags = a_clause.m_flags;
  c.m_type  = a_clause.m_type;
  c.m_a     = a_clause.m_a;
  c.m_b     = a_clause.m_b;
  c.m_c     = a_clause.m_c;

  return c;
}

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
 * @brief The index of type P in a TypeList (its size if P is not in it).
 * @details Internal helper; not part of the public API.
 * @tparam P    Type.
 * @tparam List TypeList.
 */
template <class P, class List>
struct IndexOf;

/**
 * @brief P is not in the empty list.
 * @details Internal helper; not part of the public API.
 * @tparam P Type.
 */
template <class P>
struct IndexOf<P, TypeList<>>
{
  /**
   * @brief The index: past the end.
   */
  static constexpr size_t value = 0;
};

/**
 * @brief The index of P in a non-empty list.
 * @details Internal helper; not part of the public API.
 * @tparam P    Type.
 * @tparam Q    The list's first type.
 * @tparam Rest The others.
 */
template <class P, class Q, class... Rest>
struct IndexOf<P, TypeList<Q, Rest...>>
{
  /**
   * @brief The index.
   */
  static constexpr size_t value = std::is_same_v<P, Q> ? 0 : 1 + IndexOf<P, TypeList<Rest...>>::value;
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
   * @brief Span lists a ray program may hold at once; a subtree that would need more is marched.
   */
  static constexpr uint32_t RayStackSize = 4;

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
     * @brief Each sub-program's clause range; sub-program 0 is the whole object. After the m_numPrograms
     * sub-programs come the clause ranges a ray marches (see TapeDetail::RayOp::Marched), then from
     * m_rayPrograms on each sub-program's ray program, whose clauses (TapeDetail::RayClause, stored as
     * clauses) follow the sub-programs' in m_clauses, and after those each sub-program's parent, the
     * sub-program it was compiled for, in m_begin (UINT32_MAX for sub-program 0; m_end unused). The ray
     * programs begin in program order, which rayLeafHit() relies on to find a clause's by bisection.
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

    /**
     * @brief How many sub-programs there are (the first entries of m_programs).
     */
    uint32_t m_numPrograms = 0;

    /**
     * @brief Where in m_programs the ray programs begin: sub-program i's is entry m_rayPrograms + i (an empty
     * range for one a ray never follows).
     */
    uint32_t m_rayPrograms = 0;
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
   * @brief The deciding leaf at a point: the primitive whose surface the value comes from there (see
   * PrimitiveRef), with the point in its own frame.
   * @details Evaluates the tape as signedDistance() does, carrying with each value the leaf it comes from.
   * The leaves are numbered in expression order, depth first, each occurrence counted. A primitive that
   * is itself a composition (a fused one, a tape, a BVH union) is one leaf: the tape is the scene, its
   * leaves the parts. Callable on a device, like signedDistance().
   * @param[in] a_point Query point.
   * @return The leaf, its type and index in the tape, and the point in its frame.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline PrimitiveRef<T>
  closestPrimitive(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The first crossing of the object's surface along a ray, entering or leaving.
   * @details The tape is traced along its ray program: exactly through its unions, intersections,
   * complements and affine maps of primitives with ray forms, and marched in the parts that have anything
   * else (see EBGeometry_Ray.hpp). HitT::m_primitive is the leaf crossed (see closestPrimitive()). The same
   * as the free function firstHit(). Callable on a device.
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march.
   * @return The crossing, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitT<T>
  firstHit(const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options = RayOptions<T>()) const noexcept;

  /**
   * @brief The first crossing of the object's surface on the segment from a_from to a_to.
   * @details HitT::m_t is the distance from a_from. The same as the free function firstHit(). Callable on a
   * device.
   * @param[in] a_from    Start of the segment.
   * @param[in] a_to      End of the segment.
   * @param[in] a_tMin    Distance from a_from where the search starts.
   * @param[in] a_options How to march.
   * @return The crossing, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitT<T>
  firstHit(const Vec3T<T>&      a_from,
           const Vec3T<T>&      a_to,
           T                    a_tMin    = T(0),
           const RayOptions<T>& a_options = RayOptions<T>()) const noexcept;

  /**
   * @brief Whether a ray crosses the object's surface in [a_tMin, a_tMax].
   * @details The same as the free function intersects(). Callable on a device.
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march.
   * @return Hit, Miss, or Unresolved.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitStatus
  intersects(const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options = RayOptions<T>()) const noexcept;

  /**
   * @brief Whether the segment from a_from to a_to crosses the object's surface.
   * @details The same as the free function intersects(). Callable on a device.
   * @param[in] a_from    Start of the segment.
   * @param[in] a_to      End of the segment.
   * @param[in] a_tMin    Distance from a_from where the search starts.
   * @param[in] a_options How to march.
   * @return Hit, Miss, or Unresolved.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitStatus
  intersects(const Vec3T<T>&      a_from,
             const Vec3T<T>&      a_to,
             T                    a_tMin    = T(0),
             const RayOptions<T>& a_options = RayOptions<T>()) const noexcept;

  /**
   * @brief Whether a reference's leaf is a primitive of type P.
   * @tparam P Primitive type (one of the tape's types).
   * @param[in] a_ref Reference from closestPrimitive().
   * @return True if it is.
   */
  template <class P>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isPrimitive(const PrimitiveRef<T>& a_ref) const noexcept;

  /**
   * @brief The primitive a reference names, by value: the shape itself, or for a pool-resident type (a
   * mesh SDF, a BVH union, a tape) its handle, rebuilt with the tape's location, so that it resolves
   * against the same pool (or mirror) the tape does.
   * @tparam P Primitive type; the leaf must be one (isPrimitive<P>()).
   * @param[in] a_ref Reference from closestPrimitive().
   * @return The primitive.
   */
  template <class P>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline P
  primitive(const PrimitiveRef<T>& a_ref) const noexcept;

  /**
   * @brief Call a function with the primitive a reference names, as primitive<P>() returns it, whatever
   * its type.
   * @tparam F Callable taking any of the tape's primitive types (a generic lambda in host code).
   * In a CUDA or HIP translation unit, pass a functor whose call operator is EBGEOMETRY_HOST_DEVICE: the
   * device compiler treats a lambda's call operator as a host function here.
   * @param[in] a_ref Reference from closestPrimitive().
   * @param[in] a_f   Function.
   */
  template <class F>
  EBGEOMETRY_HOST_DEVICE
  inline void
  visitPrimitive(const PrimitiveRef<T>& a_ref, F&& a_f) const noexcept;

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
   * @brief Number of clauses over all sub-programs (the ray programs' not counted).
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
   * @brief Tape's ray form follows the ray programs.
   */
  friend struct RayForm<Tape>;

  /**
   * @brief The primitive of a crossing found by marching a subtree, which closestPrimitive() then names.
   */
  static constexpr uint32_t MarchedLeaf = UINT32_MAX;

  /**
   * @brief The arrays.
   */
  Layout m_layout;

  /**
   * @brief A sub-program as a shape: what a ray marches where the ray program says so.
   * @tparam D How deep the sub-program's own calls may still nest.
   */
  template <unsigned D>
  struct Subtree
  {
    /**
     * @brief The tape.
     */
    const Tape* m_tape;

    /**
     * @brief The sub-program.
     */
    uint32_t m_program;

    /**
     * @brief Its box.
     */
    BV m_box;

    /**
     * @brief Its distance quality.
     */
    DistanceQuality m_quality;

    /**
     * @brief The pool base.
     */
    const void* m_base;

    /**
     * @brief The pool location.
     */
    PoolLocation m_location;

    /**
     * @brief The sub-program's value.
     * @param[in] a_point Point.
     * @return The value.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline T
    signedDistance(const Vec3T<T>& a_point) const noexcept
    {
      return m_tape->template evaluate<D>(m_program, a_point, m_base, m_location);
    }

    /**
     * @brief Its box.
     * @return The box.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline BV
    computeBoundingVolume() const noexcept
    {
      return m_box;
    }

    /**
     * @brief Its distance quality.
     * @return The quality.
     */
    [[nodiscard]] EBGEOMETRY_HOST_DEVICE
    inline DistanceQuality
    getDistanceQuality() const noexcept
    {
      return m_quality;
    }
  };

  /**
   * @brief The first crossing along a ray, from the ray program (what Tape's ray form does).
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitT<T>
  traceRay(const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) const noexcept;

  /**
   * @brief Where a ray is inside the object over an interval, from the ray program.
   * @param[in]  a_ray       The ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   How to march the parts without a ray form.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  raySpans(const RayT<T>&       a_ray,
           T                    a_tMin,
           T                    a_tMax,
           const RayOptions<T>& a_options,
           bool                 a_firstOnly,
           RaySpansT<T>&        a_spans) const noexcept;

  /**
   * @brief Run a ray program.
   * @tparam D How deep the program's calls may still nest.
   * @param[in]  a_program  Sub-program whose ray program to run.
   * @param[in]  a_ray      The ray.
   * @param[in]  a_t0       Start of the interval.
   * @param[in]  a_t1       End of the interval.
   * @param[in]  a_options  How to march.
   * @param[in]  a_all      Whether every clause needs all its spans, whatever its flags say.
   * @param[in]  a_base     The pool base.
   * @param[in]  a_location The pool location.
   * @param[out] a_spans    The list.
   */
  template <unsigned D>
  EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline void
  rayEvaluate(uint32_t             a_program,
              const RayT<T>&       a_ray,
              T                    a_t0,
              T                    a_t1,
              const RayOptions<T>& a_options,
              bool                 a_all,
              const void*          a_base,
              const PoolLocation&  a_location,
              RaySpansT<T>&        a_spans) const noexcept;

  /**
   * @brief The crossing at a distance along a ray on a leaf of a ray program, as its spans named it (by its
   * clause): found again by following the program to the clause, through the affine maps and BVH unions
   * above it, and asking the leaf (a marched subtree gives its gradient). The normal is the leaf's outward
   * one, mapped back to the program's frame.
   * @tparam D How deep the program's calls may still nest.
   * @param[in] a_program  Sub-program whose ray program to follow.
   * @param[in] a_ray      The ray.
   * @param[in] a_t        The crossing's distance.
   * @param[in] a_clause   The leaf's clause.
   * @param[in] a_options  How to march.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The crossing, with HitT::m_primitive the leaf's number (MarchedLeaf for a marched subtree), or a
   * miss if the clause is not reached from the program.
   */
  template <unsigned D>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline HitT<T>
  rayLeafHit(uint32_t             a_program,
             const RayT<T>&       a_ray,
             T                    a_t,
             uint32_t             a_clause,
             const RayOptions<T>& a_options,
             const void*          a_base,
             const PoolLocation&  a_location) const noexcept;

  /**
   * @brief The crossing near a distance along a ray on a Leaf clause's primitive (see RayDetail::leafHit()).
   * @details Out of line on a device, like rayLeaf(): every level of rayLeafHit() calls it, and inlined, the
   * dispatch over every primitive type was copied into each.
   * @param[in] a_clause   The Leaf clause.
   * @param[in] a_ray      The ray.
   * @param[in] a_t        The crossing's distance.
   * @param[in] a_options  How to march.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The crossing on the primitive.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline HitT<T>
  primitiveLeafHit(const TapeDetail::RayClause& a_clause,
                   const RayT<T>&               a_ray,
                   T                            a_t,
                   const RayOptions<T>&         a_options,
                   const void*                  a_base,
                   const PoolLocation&          a_location) const noexcept;

  /**
   * @brief The crossing near a distance along a ray on a primitive, dispatching on its type index from I
   * onwards (see RayDetail::leafHit()).
   * @tparam I First type index to test.
   * @param[in] a_clause   The Leaf clause.
   * @param[in] a_ray      The ray.
   * @param[in] a_t        The crossing's distance.
   * @param[in] a_options  How to march.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The crossing on the primitive.
   */
  template <size_t I>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitT<T>
  dispatchLeafHit(const TapeDetail::RayClause& a_clause,
                  const RayT<T>&               a_ray,
                  T                            a_t,
                  const RayOptions<T>&         a_options,
                  const void*                  a_base,
                  const PoolLocation&          a_location) const noexcept;

  /**
   * @brief The spans of a Marched clause that is an operand of an intersection: its subtree marched only within
   * the spans of the operands before it (see TapeDetail::RayOp::Marched).
   * @details Out of line on a device: inlined beside the march of any other Marched clause, it made every
   * tape's ray program slower.
   * @tparam D Call depth left for the subtree.
   * @param[in]  a_clause    The clause.
   * @param[in]  a_ray       The ray.
   * @param[in]  a_options   How to march.
   * @param[in]  a_firstOnly Whether only the intersection's first crossing is needed.
   * @param[in]  a_within    The spans of the operands before it.
   * @param[in]  a_base      The pool base.
   * @param[in]  a_location  The pool location.
   * @param[out] a_spans     The list.
   */
  template <unsigned D>
  EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline void
  rayMarchedWithin(const TapeDetail::RayClause& a_clause,
                   const RayT<T>&               a_ray,
                   const RayOptions<T>&         a_options,
                   bool                         a_firstOnly,
                   const RaySpansT<T>&          a_within,
                   const void*                  a_base,
                   const PoolLocation&          a_location,
                   RaySpansT<T>&                a_spans) const noexcept;

  /**
   * @brief The spans of a Leaf clause's primitive.
   * @param[in]  a_clause    The clause.
   * @param[in]  a_ray       The ray.
   * @param[in]  a_t0        Start of the interval.
   * @param[in]  a_t1        End of the interval.
   * @param[in]  a_options   How to march.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[in]  a_base      The pool base.
   * @param[in]  a_location  The pool location.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline void
  rayLeaf(const TapeDetail::RayClause& a_clause,
          const RayT<T>&               a_ray,
          T                            a_t0,
          T                            a_t1,
          const RayOptions<T>&         a_options,
          bool                         a_firstOnly,
          const void*                  a_base,
          const PoolLocation&          a_location,
          RaySpansT<T>&                a_spans) const noexcept;

  /**
   * @brief The spans of a primitive, dispatching on its type index from I onwards.
   * @tparam I First type index to test.
   * @param[in]  a_clause    The Leaf clause.
   * @param[in]  a_ray       The ray.
   * @param[in]  a_t0        Start of the interval.
   * @param[in]  a_t1        End of the interval.
   * @param[in]  a_options   How to march.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[in]  a_base      The pool base.
   * @param[in]  a_location  The pool location.
   * @param[out] a_spans     The list.
   */
  template <size_t I>
  EBGEOMETRY_HOST_DEVICE
  inline void
  dispatchRayLeaf(const TapeDetail::RayClause& a_clause,
                  const RayT<T>&               a_ray,
                  T                            a_t0,
                  T                            a_t1,
                  const RayOptions<T>&         a_options,
                  bool                         a_firstOnly,
                  const void*                  a_base,
                  const PoolLocation&          a_location,
                  RaySpansT<T>&                a_spans) const noexcept;

  /**
   * @brief The spans of a BVH union clause: the union of its operands' spans, walking its BVH along the
   * ray.
   * @tparam D Call depth left for the operands.
   * @param[in]     a_clause   The clause.
   * @param[in]     a_ray      The ray.
   * @param[in]     a_t0       Start of the interval.
   * @param[in]     a_t1       End of the interval.
   * @param[in]     a_options  How to march.
   * @param[in]     a_all      Whether the operands need all their spans, whatever their flags say.
   * @param[in]     a_base     The pool base.
   * @param[in]     a_location The pool location.
   * @param[out]    a_spans    The list.
   */
  template <unsigned D>
  EBGEOMETRY_HOST_DEVICE
  inline void
  rayUnion(const TapeDetail::RayClause& a_clause,
           const RayT<T>&               a_ray,
           T                            a_t0,
           T                            a_t1,
           const RayOptions<T>&         a_options,
           bool                         a_all,
           const void*                  a_base,
           const PoolLocation&          a_location,
           RaySpansT<T>&                a_spans) const noexcept;

  /**
   * @brief Evaluate a sub-program at a point.
   * @tparam D How deep the sub-program's own calls may still nest.
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestPrimitive().
   * @param[in] a_program  Sub-program.
   * @param[in] a_point    Point.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location, for pool-resident primitives and BVHs.
   * @return The value.
   */
  template <unsigned D, class V = T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline V
  evaluate(const uint32_t      a_program,
           const Vec3T<T>&     a_point,
           const void*         a_base,
           const PoolLocation& a_location) const noexcept;

  /**
   * @brief The body of evaluate(), inlined into it, and into signedDistance() for a tape that calls no
   * sub-program (at depth 0 the evaluator calls no level below, so inlining it multiplies nothing).
   * @tparam D How deep the sub-program's own calls may still nest.
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestPrimitive().
   * @param[in] a_program  Sub-program.
   * @param[in] a_point    Point.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location, for pool-resident primitives and BVHs.
   * @return The value.
   */
  template <unsigned D, class V = T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline V
  evaluateBody(const uint32_t      a_program,
               const Vec3T<T>&     a_point,
               const void*         a_base,
               const PoolLocation& a_location) const noexcept;

  /**
   * @brief Call a function with the primitive a reference names, dispatching on its type index from I
   * onwards.
   * @tparam I First type index to test.
   * @tparam F Callable.
   * @param[in] a_ref Reference.
   * @param[in] a_f   Function.
   */
  template <size_t I, class F>
  EBGEOMETRY_HOST_DEVICE
  inline void
  dispatchVisit(const PrimitiveRef<T>& a_ref, F&& a_f) const noexcept;

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
   * @brief Where the host splits the primitive dispatch: the types from this index on are evaluated out of
   * line (dispatchRest()), so that the common analytic shapes before it are inlined into the evaluator
   * whatever else the translation unit holds.
   */
  static constexpr size_t DispatchSplit = 6;

  /**
   * @brief The primitive dispatch from DispatchSplit on, out of line in a host pass.
   * @param[in] a_type     Type index.
   * @param[in] a_index    Index within the type's array.
   * @param[in] a_point    Point.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_HOST_NOINLINE
  T
  dispatchRest(const uint32_t      a_type,
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
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestPrimitive().
   * @param[in] a_clause   The clause.
   * @param[in] a_point    Point.
   * @param[in] a_params   The parameters.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The blurred value.
   */
  template <unsigned D, class V = T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline V
  evaluateBlur(const TapeDetail::Clause& a_clause,
               const Vec3T<T>&           a_point,
               const T*                  a_params,
               const void*               a_base,
               const PoolLocation&       a_location) const noexcept;

  /**
   * @brief Evaluate a Repeat clause: the minimum of its sub-program over the copies (CSGOps::repetition()).
   * @details Inline in a device pass, unlike evaluateBlur(): as a further out-of-line function between
   * two call levels, it raised a kernel that evaluates a tape in a loop to 255 registers (from 168).
   * @tparam D Call depth left for the sub-program.
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestPrimitive().
   * @param[in] a_clause   The clause.
   * @param[in] a_point    Point.
   * @param[in] a_params   The parameters.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The value.
   */
  template <unsigned D, class V = T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline V
  evaluateRepeat(const TapeDetail::Clause& a_clause,
                 const Vec3T<T>&           a_point,
                 const T*                  a_params,
                 const void*               a_base,
                 const PoolLocation&       a_location) const noexcept;

  /**
   * @brief Evaluate a BVH union clause.
   * @tparam D Call depth left for the operands.
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestPrimitive().
   * @param[in] a_clause   The clause.
   * @param[in] a_point    Point.
   * @param[in] a_params   The parameters.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The union's value.
   */
  template <unsigned D, class V = T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline V
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

/**
 * @brief Tape's ray form: the tape traced along its ray program (see Tape::firstHit()). Callable on a device.
 * @tparam T              Floating-point precision.
 * @tparam UserPrimitives The tape's user primitive types.
 */
template <class T, class... UserPrimitives>
struct RayForm<Tape<T, UserPrimitives...>>
{
  /**
   * @brief The first crossing of the tape's object along a ray.
   * @param[in] a_shape   The tape.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const Tape<T, UserPrimitives...>& a_shape,
           const RayT<T>&                    a_ray,
           T                                 a_tMin,
           T                                 a_tMax,
           const RayOptions<T>&              a_options) noexcept
  {
    return a_shape.traceRay(a_ray, a_tMin, a_tMax, a_options);
  }

  /**
   * @brief Where a ray is inside the tape's object over an interval.
   * @param[in]  a_shape     The tape.
   * @param[in]  a_ray       Ray, with a unit direction.
   * @param[in]  a_tMin      Start of the interval.
   * @param[in]  a_tMax      End of the interval.
   * @param[in]  a_options   How to march the parts without a ray form.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const Tape<T, UserPrimitives...>& a_shape,
        const RayT<T>&                    a_ray,
        T                                 a_tMin,
        T                                 a_tMax,
        const RayOptions<T>&              a_options,
        bool                              a_firstOnly,
        RaySpansT<T>&                     a_spans) noexcept
  {
    a_shape.raySpans(a_ray, a_tMin, a_tMax, a_options, a_firstOnly, a_spans);
  }
};

} // namespace EBGeometry

#include "EBGeometry_TapeImplem.hpp"

#endif
