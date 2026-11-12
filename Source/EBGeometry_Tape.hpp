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
 * function pointers, no exceptions); operations that evaluate a subtree several times (Blur, Mollify,
 * FiniteRepetition and BVH-accelerated unions) call a sub-program through a call-depth template, so nesting
 * is bounded at compile time.
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
#include "EBGeometry_LeafRef.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_OffsetRayForms.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Ray.hpp"
#include "EBGeometry_Subdomains.hpp"
#include "EBGeometry_SurfaceRef.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

template <class TapeType>
class RegionView;

/**
 * @brief Settings for compiling an Expr into a Tape.
 */
struct TapeOptions
{
  /**
   * @brief A union or smooth union evaluated as written (not built with Accelerate::BVH) of at least this
   * many objects prints a warning to stderr when the tape is built, suggesting Accelerate::BVH; 0 silences
   * it. Nested sharp unions count as one (Union(Union(a, b), c) is a union of three).
   * @details The default is where a BVH pays on the host and on a device alike. Measured on a lattice of
   * N spheres, 16 evaluations per point (one core of an i9-12900K, and an RTX 3080 through
   * GPU::transform()), the BVH beat the union as written from N = 12 on the host (2.6x faster at 64), but
   * only from about N = 50 to 60 on the device (12 to 15% faster at 64), where the operands' arithmetic is
   * cheap next to the traversal; at 32 the BVH was 1.6x faster on the host and up to 34% slower on the
   * device.
   */
  uint32_t m_largeUnionWarning = 64;

  /**
   * @brief How the BVHs of unions built with Accelerate::BVH are built.
   */
  BVH::Construction m_construction = BVH::Construction::SAH;
};

/**
 * @brief Optional tape features, listed among a Tape's template arguments: Tape<T, TapeFeature::ExactOffsets>.
 * @details A feature is code a tape compiles only when its type asks for it. A plain Tape<T> compiles none,
 * so its kernels are not slowed by code it never runs. Without a feature, the tape still computes the same
 * object; the operation the feature serves takes the general path (for a ray query, marching).
 *
 * Why features exist: a tape's evaluator and ray code are compiled once per tape type, for any expression a
 * tape of that type could hold, and on a device ptxas allocates registers and stack over all the code a kernel
 * can reach, run or not. Reaching the exact-offset code from the ray code, behind a flag no plain tape sets,
 * slowed plain float firstHit kernels by 10-16% (sm_86). So code that only some expressions need, such as a
 * new exact ray form for an operation, is a feature, and a new feature follows these rules:
 * - **The tape stays correct without it.** Without the feature the compiler emits what it emitted before the
 *   feature existed (for a ray form, a RayOp::Marched clause), so a tape that does not list it computes the
 *   same object by the general path. A feature changes speed or exactness, never which object is computed.
 * - **The compiler decides.** TapeDetail::Compiler receives the features and emits the feature's clauses,
 *   flags or parameters only when it is listed; clause flags a feature sets are never set otherwise.
 * - **The evaluator compiles the feature's code only under `if constexpr (hasFeature<F>)`**, at every place it
 *   is reached from (for ExactOffsets: rayLeaf(), rayMarchedWithin(), traceRay() and rayLeafHit()). A run-time test
 *   of a flag alone is not enough: the code behind it is still in every kernel.
 * - **A plain tape compiles exactly the code it did before.** Check that Tape<T>'s kernels are unchanged: the
 *   ptxas register, stack and spill figures (or the SASS) of the float tape kernels (signedDistance() once
 *   and sixteen times, and firstHit()) and the host evaluator's out-of-line function sizes (`nm -C -S` on
 *   Performance/Tape) match the base's, and the performance comparison shows no change in the tape cases.
 * - **Tests and probes use both.** The feature's tests use a tape that lists it, and at least one test checks
 *   that a tape without it gives the same answer by the general path. Probe cases that measure it use a tape
 *   that lists it; the existing tape cases keep plain tapes.
 * - **Documented** here, as a tag with a description, and in the user documentation's list of tape features.
 */
namespace TapeFeature {

/**
 * @brief Base of every feature: a Tape template argument derived from it is a feature, not a primitive type.
 */
struct Tag
{
};

/**
 * @brief Ray queries trace an Offset or Annular of a primitive with closed-form level sets (or of an affine map
 * of one) exactly, from its level sets (see RayDetail::LevelForm); without it they march it. The level passes
 * through Scale, Translate and Rotate to the primitive, divided by the map's value scale, so an annulus of a
 * scaled mesh is still traced exactly.
 */
struct ExactOffsets : Tag
{
};

} // namespace TapeFeature

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
  Leaf,         ///< Push the spans of primitive (m_type, m_a), leaf number m_b, along the top ray. With bit 3 of
                ///< m_flags, of its level set at params[m_c] (an Offset of it, see RayDetail::LevelView); with
                ///< bit 4 as well, of its annulus of that half-thickness (see rayLevelLeaf()).
  Marched,      ///< Push the spans of sub-program m_a along the top ray, marched in the box at
                ///< params[m_b..m_b+6), with distance quality m_c. With bit 1 of m_flags, it is an operand of an
                ///< intersection, marched only within the spans of the top list, the operands' before it (see
                ///< RayDetail::spansWithin()); with bit 2 as well, the intersection needs only its first
                ///< crossing. With bit 3, it is instead a mesh's level set deferred in an intersection (see
                ///< RayDetail::DeferredOperand): primitive (m_type, m_a), leaf number m_c, the level at params[m_b]
                ///< (an annulus with bit 4), and with bit 5 the affine map it is traced through at
                ///< params[m_b+1..m_b+13) (see rayLevelWithin()).
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
 * @details Internal helper; not part of the public API. The meaning of each field depends on m_op. The ray
 * programs cost about 16 bytes per expression node of the tape's pool.
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
   * @brief Primitive type index (Leaf, and a Marched clause holding a deferred level set).
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
 * @brief The bit a BVH union's leaf sets on its sub-program's index when that sub-program is one shape, maybe
 * under point maps and a value scale (its clauses are only PushAffine, Primitive, PopPoint and MulConstant, with
 * one Primitive): the leaf evaluates it directly (Tape::evaluateLeaf()), with no call into the interpreter.
 * @details Internal helper; not part of the public API.
 */
inline constexpr uint32_t LeafProgram = uint32_t(1) << 31;

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
 * @brief Whether a TypeList holds a container (see hasSurfaces).
 * @details Internal helper; not part of the public API.
 * @tparam T    Floating-point precision.
 * @tparam List TypeList.
 */
template <class T, class List>
struct AnyContainer;

/**
 * @brief Whether a TypeList holds a container (see hasSurfaces).
 * @details Internal helper; not part of the public API.
 * @tparam T  Floating-point precision.
 * @tparam Ps Types.
 */
template <class T, class... Ps>
struct AnyContainer<T, TypeList<Ps...>> : std::bool_constant<(hasSurfaces<Ps, T> || ...)>
{
};

/**
 * @brief Whether a TypeList holds a container or a shape with parts (see hasSurfaces, hasParts): whether
 * closestSurface() drills into the deciding leaf at all.
 * @details Internal helper; not part of the public API.
 * @tparam T    Floating-point precision.
 * @tparam List TypeList.
 */
template <class T, class List>
struct AnyDrill;

/**
 * @brief Whether a TypeList holds a container or a shape with parts.
 * @details Internal helper; not part of the public API.
 * @tparam T  Floating-point precision.
 * @tparam Ps Types.
 */
template <class T, class... Ps>
struct AnyDrill<T, TypeList<Ps...>> : std::bool_constant<((hasSurfaces<Ps, T> || hasParts<Ps, T>) || ...)>
{
};

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
 * @brief The concatenation of TypeLists.
 * @details Internal helper; not part of the public API.
 * @tparam Lists TypeLists.
 */
template <class... Lists>
struct Concat
{
  /**
   * @brief The concatenation.
   */
  using type = TypeList<>;
};

/**
 * @brief The concatenation of TypeLists.
 * @details Internal helper; not part of the public API.
 * @tparam Ps   Types of the first list.
 * @tparam Rest The other lists.
 */
template <class... Ps, class... Rest>
struct Concat<TypeList<Ps...>, Rest...>
{
  /**
   * @brief The concatenation.
   */
  using type = typename Concat<TypeList<Ps...>, typename Concat<Rest...>::type>::type;
};

/**
 * @brief The concatenation of two TypeLists.
 * @details Internal helper; not part of the public API.
 * @tparam Ps Types of the first list.
 * @tparam Qs Types of the second list.
 */
template <class... Ps, class... Qs>
struct Concat<TypeList<Ps...>, TypeList<Qs...>>
{
  /**
   * @brief The concatenation.
   */
  using type = TypeList<Ps..., Qs...>;
};

/**
 * @brief Whether a Tape template argument is a feature (see TapeFeature).
 * @details Internal helper; not part of the public API.
 * @tparam A Template argument.
 */
template <class A>
inline constexpr bool IsFeature = std::is_base_of_v<TapeFeature::Tag, A>;

/**
 * @brief The features among a Tape's template arguments.
 * @details Internal helper; not part of the public API.
 * @tparam Args Template arguments after T.
 */
template <class... Args>
using FeaturesOf = typename Concat<std::conditional_t<IsFeature<Args>, TypeList<Args>, TypeList<>>...>::type;

/**
 * @brief The user primitive types among a Tape's template arguments: those that are not features.
 * @details Internal helper; not part of the public API.
 * @tparam Args Template arguments after T.
 */
template <class... Args>
using UserTypesOf = typename Concat<std::conditional_t<IsFeature<Args>, TypeList<>, TypeList<Args>>...>::type;

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
 * @brief The tape's primitive types: twelve analytic shapes (PlaneSDF to PerlinSDF), the three mesh SDFs with the
 * default branching factor and SIMD width, the user's types (its template arguments that are not features), then
 * nine more analytic shapes (EllipsoidSDF to WedgeSDF).
 * @details Internal helper; not part of the public API. The evaluator finds a primitive's type by testing the
 * types in this order, so a type further down costs a test more; on the host, every type from Tape::DispatchSplit
 * on is dispatched out of line. The user's types come before the last nine shapes, so that a tape of user types
 * (a BVH union, a mesh with another branching factor, a fused part) reaches them right after the first fifteen.
 * @tparam T            Floating-point precision.
 * @tparam UserPrimitives User primitive types and features.
 */
template <class T, class... UserPrimitives>
using Primitives =
  typename Concat<TypeList<PlaneSDF<T>,
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
                           TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>>,
                  UserTypesOf<UserPrimitives...>,
                  TypeList<EllipsoidSDF<T>,
                           CappedConeSDF<T>,
                           RoundConeSDF<T>,
                           PolygonPrismSDF<T>,
                           TubeSDF<T>,
                           CappedTorusSDF<T>,
                           CutSphereSDF<T>,
                           CutHollowSphereSDF<T>,
                           WedgeSDF<T>>>::type;

/**
 * @brief Whether a tape's user primitive types are shapes of its precision.
 * @details Internal helper; not part of the public API.
 * @tparam T    Floating-point precision.
 * @tparam List TypeList of the user primitive types.
 */
template <class T, class List>
struct UserTypes;

/**
 * @brief Whether a tape's user primitive types are shapes of its precision.
 * @details Internal helper; not part of the public API.
 * @tparam T  Floating-point precision.
 * @tparam Ps User primitive types.
 */
template <class T, class... Ps>
struct UserTypes<T, TypeList<Ps...>>
{
  /**
   * @brief Whether every type is a shape (see isShape).
   */
  static constexpr bool shapes = (isShape<Ps> && ...);

  /**
   * @brief Whether every type has precision T.
   */
  static constexpr bool precision = (std::is_same_v<ShapePrecision<Ps>, T> && ...);
};

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
 * so the value can be larger than the plain minimum, but it keeps the sign and is still a bound. Blur, Mollify,
 * FiniteRepetition and BVH unions evaluate a sub-program several times; they nest
 * at most MaxCallDepth deep, and a sub-program needs at most ValueStackSize values and PointStackSize
 * points at once; the constructor rejects an expression that needs more.
 *
 * Optional features (see TapeFeature) are listed among the template arguments, before the user primitive
 * types by convention: Tape<T, TapeFeature::ExactOffsets, MyUnion>. A tape compiles only the features it
 * lists, and without one computes the same object by the general path.
 * @tparam T              Floating-point precision.
 * @tparam UserPrimitives Additional primitive types (each a shape of precision T), and features (see TapeFeature).
 */
template <class T, class... UserPrimitives>
class Tape : public PoolHandle
{
public:
  static_assert(std::is_floating_point_v<T>, "Tape requires a floating-point type T");
  static_assert(TapeDetail::UserTypes<T, TapeDetail::UserTypesOf<UserPrimitives...>>::shapes,
                "Tape: every user primitive type must be a shape (see isShape)");
  static_assert(TapeDetail::UserTypes<T, TapeDetail::UserTypesOf<UserPrimitives...>>::precision,
                "Tape: every user primitive type must have the tape's precision");

  /**
   * @brief The features the tape compiles (see TapeFeature): its template arguments derived from TapeFeature::Tag.
   */
  using Features = TapeDetail::FeaturesOf<UserPrimitives...>;

  /**
   * @brief Whether the tape compiles feature F (see TapeFeature).
   * @tparam F Feature.
   */
  template <class F>
  static constexpr bool hasFeature = TapeDetail::IndexOf<F, Features>::value < Features::size;

  /**
   * @brief Bounding box type.
   */
  using BV = BoundingVolumes::AABBT<T>;

  /**
   * @brief Branching factor of the BVHs over union operands.
   */
  static constexpr size_t BranchingRatio = 4;

  /**
   * @brief How deep Blur, Mollify, FiniteRepetition and BVH unions may nest.
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
     * @brief How many primitives (leaves) the tape has, each occurrence counted.
     */
    uint32_t m_numPrimitives = 0;

    /**
     * @brief The most objects in one union evaluated as written (see getLargestPlainUnion()).
     */
    uint32_t m_largestPlainUnion = 0;

    /**
     * @brief Where in m_programs the ray programs begin: sub-program i's is entry m_rayPrograms + i (an empty
     * range for one a ray never follows).
     */
    uint32_t m_rayPrograms = 0;

    /**
     * @brief Each leaf's label (see getSurfaceLabel()); empty when the expression has no Label.
     */
    PODVector<uint32_t> m_surfaceLabels;

    /**
     * @brief Each leaf's first surface, and the number of surfaces last (see getFirstSurface()); empty when no
     * leaf is a container, each leaf then being one surface, numbered as the leaf.
     */
    PODVector<uint32_t> m_surfaces;

    /**
     * @brief The part labels (see SurfaceLabel(shape, FaceGroups)): first, per leaf, where its parts' labels begin in
     * this array (None for a leaf whose parts are not named); then those labels. Empty when no leaf's parts are.
     */
    PODVector<uint32_t> m_partSurfaceLabels;

    /**
     * @brief The regions: the number n of the Overlays' operands (0 without an Overlay), then
     * each operand's record's offset, then each region's material (the background first; two for an object with no
     * Overlay), then the records: the number m of masks above the region, their numbers (outermost first), and its
     * m + 1 programs, the solid with every mask and then with the outermost 1, 2, ..., m left out. Empty without the
     * feature.
     */
    PODVector<uint32_t> m_regionPrograms;

    /**
     * @brief How many distinct labels the expression has.
     */
    uint32_t m_numLabels = 0;
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
   * LeafRef), with the point in its own frame.
   * @details Evaluates the tape as signedDistance() does, carrying with each value the leaf it comes from.
   * The leaves are numbered in expression order, depth first, each occurrence counted. A primitive that
   * is itself a composition (a fused one, a tape, a BVH union) is one leaf: the tape is the scene, its
   * leaves the parts. closestSurface() goes on into a leaf that is a collection. Callable on a device, like signedDistance().
   * @param[in] a_point Query point.
   * @return The leaf, its type and index in the tape, and the point in its frame.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline LeafRef<T>
  closestLeaf(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The first crossing of the object's surface along a ray, entering or leaving.
   * @details The tape is traced along its ray program: exactly through its unions, intersections,
   * complements and affine maps of primitives with ray forms, and marched in the parts that have anything
   * else (see EBGeometry_Ray.hpp). HitT::m_primitive is the leaf crossed (see closestLeaf()). The same
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
   * @brief The first crossing along a ray, and the surface crossed: the ray counterpart of closestSurface(), as
   * firstHit() is of signedDistance(). The same as the free function firstSurface(). Callable on a device.
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march.
   * @return The crossing and its surface, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceHitT<T>
  firstSurface(const RayT<T>&       a_ray,
               T                    a_tMin,
               T                    a_tMax,
               const RayOptions<T>& a_options = RayOptions<T>()) const noexcept;

  /**
   * @brief The first crossing on the segment from a_from to a_to, and the surface crossed (see firstSurface() for a
   * ray). Callable on a device.
   * @param[in] a_from    Start of the segment.
   * @param[in] a_to      End of the segment.
   * @param[in] a_tMin    Distance from a_from where the search starts.
   * @param[in] a_options How to march.
   * @return The crossing and its surface, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceHitT<T>
  firstSurface(const Vec3T<T>&      a_from,
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
   * @param[in] a_ref Reference from closestLeaf().
   * @return True if it is.
   */
  template <class P>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isPrimitive(const LeafRef<T>& a_ref) const noexcept;

  /**
   * @brief The primitive a reference names, by value: the shape itself, or for a pool-resident type (a
   * mesh SDF, a BVH union, a tape) its handle, rebuilt with the tape's location, so that it resolves
   * against the same pool (or mirror) the tape does.
   * @tparam P Primitive type; the leaf must be one (isPrimitive<P>()).
   * @param[in] a_ref Reference from closestLeaf().
   * @return The primitive.
   */
  template <class P>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline P
  primitive(const LeafRef<T>& a_ref) const noexcept;

  /**
   * @brief Call a function with the primitive a reference names, as primitive<P>() returns it, whatever
   * its type.
   * @tparam F Callable taking any of the tape's primitive types (a generic lambda in host code).
   * In a CUDA or HIP translation unit, pass a functor whose call operator is EBGEOMETRY_HOST_DEVICE: the
   * device compiler treats a lambda's call operator as a host function here.
   * @param[in] a_ref Reference from closestLeaf().
   * @param[in] a_f   Function.
   */
  template <class F>
  EBGEOMETRY_HOST_DEVICE
  inline void
  visitPrimitive(const LeafRef<T>& a_ref, F&& a_f) const noexcept;

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
   * @brief Number of sub-programs (1 plus one per Blur, Mollify, FiniteRepetition and BVH union operand).
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
   * @brief Number of unions evaluated with a BVH (those built with Accelerate::BVH).
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumBVHUnions() const noexcept;

  /**
   * @brief Number of primitives (leaves) in the tape, each occurrence counted.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumPrimitives() const noexcept;

  /**
   * @brief The most objects in one union evaluated as written (one not built with Accelerate::BVH), nested
   * sharp unions counted as one: what TapeOptions::m_largeUnionWarning is compared with. An Overlay, which is evaluated
   * as written and takes no BVH, is not counted.
   * @return The count (0 if the tape has no such union).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getLargestPlainUnion() const noexcept;

  /**
   * @brief A leaf's label, as SurfaceLabel() resolves it (ExprIndex::getSurfaceLabel()): the nearest label above the
   * leaf, the leaf's own included, but a label above a chain of maps relabels the object below them.
   * @details Labels are numbered from 0 in the order their names first appear in the expression, depth first,
   * as ExprIndex numbers them, which also holds the names. A device reads the label from the tape's pool, so
   * per-label data (a boundary condition, a restitution coefficient) is an array indexed by it.
   * @param[in] a_leaf Leaf number: LeafRef::m_leaf from closestLeaf(), or HitT::m_primitive of a hit.
   * @return The label, or SurfaceLabelId::none() if the leaf has none (or is not a leaf of the tape).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceLabelId
  getSurfaceLabel(uint32_t a_leaf) const noexcept;

  /**
   * @brief The label of the leaf a reference names (see getSurfaceLabel(uint32_t)).
   * @param[in] a_ref Reference from closestLeaf().
   * @return The label, or SurfaceLabelId::none().
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceLabelId
  getSurfaceLabel(const LeafRef<T>& a_ref) const noexcept;

  /**
   * @brief How many distinct labels the expression has (ExprIndex::getNumSurfaceLabels()).
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSurfaceLabels() const noexcept;

  /**
   * @brief A leaf's own value at a point in its frame (as LeafRef::m_localPoint gives one): its shape's
   * signedDistance(), on the host, for diagnostics (ExprIndex::checkLabels()). The tape's pool must be readable on the
   * host.
   * @param[in]  a_leaf  The leaf, numbered as ExprIndex numbers them.
   * @param[in]  a_point The point, in the leaf's frame.
   * @param[out] a_value Its value.
   * @return Whether the tape holds the leaf (a leaf inside a collection is not one of the tape's).
   */
  [[nodiscard]] EBGEOMETRY_HOST
  bool
  leafValue(uint32_t a_leaf, const Vec3T<T>& a_point, T& a_value) const;

  /**
   * @brief How many regions the object has: the background (region 0) and one per operand of
   * its Overlays, numbered as ExprIndex numbers them; 2 for an object with no Overlay (the outside and the object).
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumRegions() const noexcept;

  /**
   * @brief A region's material, as ExprIndex numbers the material labels. Callable on a device.
   * @param[in] a_region Region.
   * @return The material, or none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline MaterialLabelId
  getMaterialOf(uint32_t a_region) const noexcept;

  /**
   * @brief The region a point is in: the last operand of the Overlays whose solid holds it
   * (later operands are on top), or the background (0). Exact in sign: every point is in exactly one region.
   * Callable on a device.
   * @param[in] a_point Point.
   * @return The region.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline uint32_t
  region(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief A region's implicit function: negative exactly inside the region, its zero
   * contour the region's surface, interfaces with other regions included. For region k it is max(S_k, -min of the
   * later S_j), S_j being operand j's solid; for the background, minus signedDistance(). A distance bound, exact
   * near a smooth piece of surface of exact operands; what a cut-cell code cuts for one region. Callable on a
   * device.
   * @param[in] a_point  Point.
   * @param[in] a_region Region (below getNumRegions()).
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  signedDistance(const Vec3T<T>& a_point, uint32_t a_region) const noexcept;

  /**
   * @brief Every region's function at a point (signedDistance(point, region), the same values), in one pass: calls
   * a_visit(region, value) for each region, from the last one down to the background (0). Each operand's solid is
   * evaluated once, twice when the regions share masks, where asking signedDistance() region by region evaluates the
   * later solids again for each; regions with different masks are asked one by one. Callable on a device.
   * @details Region k's function is max(S_k, -min of the later solids without the masks the two share). With the
   * same masks on every region (none, usually), every later solid drops all of them, so a running minimum of the
   * unmasked solids, taken from the top down, serves every region: n solids, or 2n with masks, instead of n^2 / 2.
   * @tparam Visit A callable taking (uint32_t region, T value).
   * @param[in] a_point Point.
   * @param[in] a_visit What to do with each region's value.
   */
  template <class Visit>
  EBGEOMETRY_HOST_DEVICE
  inline void
  regionDistances(const Vec3T<T>& a_point, Visit&& a_visit) const noexcept;

  /**
   * @brief The surface of a region that decides its function at a point: the region's own
   * boundary, an interface with a later region, or a mask's cut; drilled, with its part and surface label, as
   * closestSurface() reports a surface, and with the regions on its two sides at that point: m_ownerRegion the
   * region asked about (inside), m_neighborRegion the region across. The sides belong to the point, not to the surface:
   * one surface can face different regions in different places. For the background, the whole object's surface.
   * Callable on a device.
   * @param[in] a_point  Point, on or near the region's surface.
   * @param[in] a_region Region (below getNumRegions()).
   * @return The surface.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline SurfaceRef<T>
  closestSurface(const Vec3T<T>& a_point, uint32_t a_region) const noexcept;

  /**
   * @brief A subdomain's implicit function: half the difference of the smallest function of a
   * region in the subdomain and the smallest of one not in it (the background included), negative exactly inside the
   * subdomain and zero only on its surface: where one of its regions meets a region not in it, never on the contact
   * between two of its own. A distance bound, exact near a smooth piece of surface; what a cut-cell solver cuts for
   * the subdomain, and the function of a union of materials (a subdomain of their regions). Callable on a device.
   * @param[in] a_point  Point.
   * @param[in] a_subdomains The subdomains (built from this tape's regions).
   * @param[in] a_subdomain  Subdomain (below a_subdomains.getNumSubdomains()).
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  signedDistance(const Vec3T<T>& a_point, const Subdomains& a_subdomains, uint32_t a_subdomain) const noexcept;

  /**
   * @brief The surface of a subdomain that decides its function at a point: that of its region
   * whose function is smallest there (see closestSurface(p, region)), with the regions on its two sides; the
   * surface's role for the solver is a_subdomains.role(surface). A contact between two of the subdomain's own regions
   * is not its surface: where that region's nearest surface is one, the nearest region outside the subdomain names the
   * surface instead, seen from the subdomain's side, if it borders the subdomain there. Callable on a device.
   * @param[in] a_point  Point, on or near the subdomain's surface.
   * @param[in] a_subdomains The subdomains.
   * @param[in] a_subdomain  Subdomain.
   * @return The surface.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline SurfaceRef<T>
  closestSurface(const Vec3T<T>& a_point, const Subdomains& a_subdomains, uint32_t a_subdomain) const noexcept;

  /**
   * @brief The first point along a ray where the region changes: for a particle crossing
   * layers, from inside any region, the background included. The crossing is the nearer of leaving the current
   * region's solid and entering a later one, each traced exactly where the operands have ray forms (and marched
   * where they do not). m_hit is the crossing, its normal pointing from the region left into the region entered;
   * m_surface the surface crossed (leaf, part, surface label), its m_ownerRegion the region left and m_neighborRegion
   * the region entered. The ray starts counting a hundredth of the march's tolerance at the object's scale past
   * a_tMin (RayOptions::m_tolerance times m_scale, or the box's diagonal), so a ray crossing several layers asks
   * again from the last crossing's t, and layers thinner than that are passed over. Callable on a device.
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline SurfaceHitT<T>
  firstRegionChange(const RayT<T>&       a_ray,
                    T                    a_tMin,
                    T                    a_tMax,
                    const RayOptions<T>& a_options = RayOptions<T>()) const noexcept;

  /**
   * @brief The first change of region along a particle's step from one point to another (see
   * firstRegionChange(ray)), t measured from a_from.
   * @param[in] a_from    Start of the step.
   * @param[in] a_to      End of the step.
   * @param[in] a_tMin    Distance from a_from where the step starts counting.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceHitT<T>
  firstRegionChange(const Vec3T<T>&      a_from,
                    const Vec3T<T>&      a_to,
                    T                    a_tMin    = T(0),
                    const RayOptions<T>& a_options = RayOptions<T>()) const noexcept;

  /**
   * @brief The first point along a ray where it crosses a subdomain's boundary, entering or
   * leaving the subdomain: changes between two of its regions are passed through. For rendering, the subdomain is the set of
   * visible regions: hidden ones are seen through. Reported as firstRegionChange() reports a crossing. Callable on a
   * device.
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_subdomains  The subdomains.
   * @param[in] a_subdomain   Subdomain.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline SurfaceHitT<T>
  firstSurface(const RayT<T>&       a_ray,
               T                    a_tMin,
               T                    a_tMax,
               const Subdomains&    a_subdomains,
               uint32_t             a_subdomain,
               const RayOptions<T>& a_options = RayOptions<T>()) const noexcept;

  /**
   * @brief The surface that decides the value at a point: the deciding leaf (closestLeaf()), drilled into when
   * it is a container (a sphere cloud, a BVH union, a nested tape; see hasSurfaces) down to the member that
   * decides there, at any depth.
   * @details The surfaces are numbered across the scene: each leaf covers [getFirstSurface(leaf),
   * getFirstSurface(leaf + 1)), one surface for a leaf that is not a container, and a container's are its own
   * numbers (a cloud's cloud indices, a union's members in the order it was given them) after the leaf's first.
   * A container costs one more query of that leaf (a second search of a cloud, say), paid only here; for a
   * tape without one, the surface is the leaf. For a ray hit, closestSurface(hit.m_point) names the surface
   * crossed. Labels of a nested tape are its own, not reported here. Callable on a device.
   * @param[in] a_point Query point.
   * @return The surface, its leaf and label, and the point in the frame of the deepest shape reached.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceRef<T>
  closestSurface(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief How many surfaces the tape numbers (see closestSurface()).
   * @return The count: the number of leaves when no leaf is a container.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSurfaces() const noexcept;

  /**
   * @brief A leaf's first surface: the leaf covers [getFirstSurface(leaf), getFirstSurface(leaf + 1)), so
   * per-member data of a container leaf (a sphere's) is an array indexed by SurfaceRef::m_surface minus it.
   * @param[in] a_leaf Leaf number, at most getNumPrimitives() (which gives getNumSurfaces()).
   * @return The surface, or SurfaceRef<T>::None past that.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getFirstSurface(uint32_t a_leaf) const noexcept;

  /**
   * @brief How deep sub-program calls nest (0 if the tape has no Blur, Mollify, FiniteRepetition or BVH union).
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
   * @brief A view of the tape's regions traces the whole object through traceProgram(), out of line.
   */
  template <class TapeType>
  friend class RegionView;

  /**
   * @brief The primitive of a crossing found by marching a subtree, which closestLeaf() then names.
   */
  static constexpr uint32_t MarchedLeaf = UINT32_MAX;

  /**
   * @brief The primitive of a crossing on a level-set clause before resolveLevelHit() resolves it.
   */
  static constexpr uint32_t LevelLeaf = UINT32_MAX - 1;

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
   * @brief The first crossing along a ray of one program's value (the whole object's is program 0; a region's solid
   * has its own), from its ray program.
   * @param[in] a_program The program.
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline HitT<T>
  traceProgram(
    uint32_t a_program, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) const noexcept;

  /**
   * @brief The body of traceRay() (the whole object, program 0, inlined into it) and of traceProgram() (any program,
   * out of line).
   * @details The whole object's trace compiles as it did before region programs existed: the program is the constant
   * 0, and the naming of a marched crossing for another program is compiled out (if constexpr). Reached from
   * traceRay() through an out-of-line traceProgram(), or with that naming merely reachable, a plain float tape's
   * firstHit() kernel went from 40 to 255 registers and ran 12% slower on a device (sm_86).
   * @tparam Whole Whether the program is the whole object's.
   * @param[in] a_program The program (0 when Whole).
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, a miss, or an unresolved query.
   */
  template <bool Whole>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitT<T>
  traceBody(
    uint32_t a_program, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) const noexcept;

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
   * @tparam D     How deep the program's calls may still nest.
   * @tparam Level Whether a level-set clause (bit 3, see TapeDetail::RayOp) is resolved (levelClauseHit());
   * without, its crossing is only marked LevelLeaf, so that this walk, which plain tapes run, never reaches the
   * level-set code (see resolveLevelHit()).
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
  template <unsigned D, bool Level = false>
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
   * @brief The spans of a Leaf clause's level set of its primitive (bit 3 of m_flags; see
   * TapeDetail::RayOp::Leaf), out of line on the host and on a device: a tape's evaluator and leaf dispatch keep
   * the code they have without offsets.
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
  EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  EBGEOMETRY_HOST_NOINLINE
  void
  rayLevelLeaf(const TapeDetail::RayClause& a_clause,
               const RayT<T>&               a_ray,
               T                            a_t0,
               T                            a_t1,
               const RayOptions<T>&         a_options,
               bool                         a_firstOnly,
               const void*                  a_base,
               const PoolLocation&          a_location,
               RaySpansT<T>&                a_spans) const noexcept;

  /**
   * @brief The spans of a Marched clause holding a mesh's deferred level set (bit 3 of m_flags; see
   * TapeDetail::RayOp::Marched): within the spans of the intersection's operands before it, through the clause's
   * affine map (bit 5) if it has one. Out of line on the host and on a device.
   * @param[in]  a_clause    The Marched clause.
   * @param[in]  a_ray       The ray.
   * @param[in]  a_options   How to march.
   * @param[in]  a_firstOnly Whether only the intersection's first crossing is needed.
   * @param[in]  a_within    The spans of the operands before it.
   * @param[in]  a_base      The pool base.
   * @param[in]  a_location  The pool location.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  EBGEOMETRY_HOST_NOINLINE
  void
  rayLevelWithin(const TapeDetail::RayClause& a_clause,
                 const RayT<T>&               a_ray,
                 const RayOptions<T>&         a_options,
                 bool                         a_firstOnly,
                 const RaySpansT<T>&          a_within,
                 const void*                  a_base,
                 const PoolLocation&          a_location,
                 RaySpansT<T>&                a_spans) const noexcept;

  /**
   * @brief The crossing at a distance on a level-set clause's primitive (a Leaf clause with bit 3, or a Marched
   * clause holding a deferred level set): its normal, through the clause's own affine map if it has one. Out of
   * line on the host and on a device.
   * @param[in] a_clause   The clause.
   * @param[in] a_ray      The ray.
   * @param[in] a_t        The crossing's distance.
   * @param[in] a_options  How to march.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The crossing.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  EBGEOMETRY_HOST_NOINLINE
  HitT<T>
  levelClauseHit(const TapeDetail::RayClause& a_clause,
                 const RayT<T>&               a_ray,
                 T                            a_t,
                 const RayOptions<T>&         a_options,
                 const void*                  a_base,
                 const PoolLocation&          a_location) const noexcept;

  /**
   * @brief The crossing at a distance on a level-set clause, found again from the tape's top program
   * (rayLeafHit() with its level-set resolution): what traceRay() asks once its trace ended on one. Out of line
   * on the host and on a device: the walk plain tapes run keeps no call into level-set code, which made ptxas
   * spill more in every tape's ray kernel.
   * @param[in] a_clause   The clause.
   * @param[in] a_ray      The ray.
   * @param[in] a_t        The crossing's distance.
   * @param[in] a_options  How to march.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The crossing.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  EBGEOMETRY_HOST_NOINLINE
  HitT<T>
  resolveLevelHit(uint32_t             a_clause,
                  const RayT<T>&       a_ray,
                  T                    a_t,
                  const RayOptions<T>& a_options,
                  const void*          a_base,
                  const PoolLocation&  a_location) const noexcept;

  /**
   * @brief Call a_visit(primitive) with a clause's primitive (m_type, m_a) if its type, from I onwards, has
   * closed-form level sets (RayDetail::LevelForm; with a_meshOnly, only a mesh's, RayDetail::IsMeshLevel).
   * @tparam I        First type index to test.
   * @tparam MeshOnly Whether only mesh types are visited.
   * @tparam Visit    Callback.
   * @param[in] a_clause   The clause.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @param[in] a_visit    The callback.
   */
  template <size_t I, bool MeshOnly, class Visit>
  EBGEOMETRY_HOST_DEVICE
  inline void
  visitLevelPrimitive(const TapeDetail::RayClause& a_clause,
                      const void*                  a_base,
                      const PoolLocation&          a_location,
                      const Visit&                 a_visit) const noexcept;

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
   * @details One function per call level D, out of line in a device pass so that a device compile stays small:
   * HIP inlines every device function, and this one calls the level below from many places, so inlined, the
   * copies multiply with every level. Because D is a template parameter, a kernel reserves stack for all
   * MaxCallDepth + 1 levels whatever the tape needs (a tape of depth 0 included, since signedDistance() still
   * contains the call to the full depth): each level holds its value and point stacks, a BVH union's traversal
   * stack and saved registers, a few kilobytes per thread in all. A tape listed as another tape's primitive adds
   * its own levels on top.
   * @tparam D How deep the sub-program's own calls may still nest.
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestLeaf().
   * @param[in] a_program  Sub-program.
   * @param[in] a_point    Point.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location, for pool-resident primitives and BVHs.
   * @return The value.
   */
  /**
   * @brief The whole object's value, through the evaluator's out-of-line entry: signedDistance()
   * inlines the evaluator's body for a tape that calls no sub-program, which the region code would copy at each site.
   * @param[in] a_point Point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  wholeValue(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief How many operands the Overlays have: the regions but the background; 0 without
   * an Overlay.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  numRegionOperands() const noexcept;

  /**
   * @brief The leaf a tagged value comes from.
   * @param[in] a_value The value.
   * @return The leaf, or none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline LeafRef<T>
  leafOf(const CSGOps::Tagged<T>& a_value) const noexcept;

  /**
   * @brief A leaf a trace of a program crossed, with the point in its frame.
   * @details Walks the program's clauses, applying only its point maps, to the leaf's clause: no shape is evaluated.
   * Evaluating the program tagged named the same leaf at the crossing, but evaluated every shape of it there (a
   * mesh's closest point far from the mesh, where the ray crossed another shape). A leaf in a sub-program (a
   * repetition, a blur, a BVH union's operands) is not among the program's clauses, and is found by that evaluation.
   * @param[in] a_program Program traced.
   * @param[in] a_leaf    Leaf the trace reported (HitT::m_primitive).
   * @param[in] a_point   Crossing.
   * @return The leaf, or none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline LeafRef<T>
  tracedLeaf(const uint32_t a_program, const uint32_t a_leaf, const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief A leaf's surface: drilled into a collection, with its part and its label (see closestSurface()).
   * @param[in] a_ref The leaf.
   * @return The surface, or none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline SurfaceRef<T>
  surfaceOf(const LeafRef<T>& a_ref) const noexcept;

  /**
   * @brief A region's solid with the leaf its value comes from (see regionSolid()).
   * @param[in] a_region Region.
   * @param[in] a_level  How many of its outermost masks to leave out.
   * @param[in] a_point  Point.
   * @return The tagged value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline CSGOps::Tagged<T>
  regionSolidTagged(uint32_t a_region, uint32_t a_level, const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief A region's solid: its operand under the maps above it and its masks but the
   * outermost a_level.
   * @param[in] a_region Region (1 to the number of regions less one).
   * @param[in] a_level  How many of its outermost masks to leave out.
   * @param[in] a_point  Point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  regionSolid(uint32_t a_region, uint32_t a_level, const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief How many masks two regions share: those above both, which region k's function
   * applies once rather than to each later region too (see TapeDetail::RegionSolid).
   * @param[in] a_first  A region.
   * @param[in] a_second Another region.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  sharedMasks(uint32_t a_first, uint32_t a_second) const noexcept;

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
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestLeaf().
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
  dispatchVisit(const LeafRef<T>& a_ref, F&& a_f) const noexcept;

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
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestLeaf().
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
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestLeaf().
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
   * @brief Evaluate a sub-program that is one shape under point maps and a value scale (see
   * TapeDetail::LeafProgram), as evaluateBody() would, without its stacks.
   * @details Out of line in a device pass: one copy of the shapes' dispatch per kernel, which every BVH union
   * at every call depth shares.
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestLeaf().
   * @param[in] a_program  The sub-program.
   * @param[in] a_point    Point.
   * @param[in] a_base     The pool base.
   * @param[in] a_location The pool location.
   * @return The shape's value, scaled.
   */
  template <class V>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline V
  evaluateLeaf(uint32_t            a_program,
               const Vec3T<T>&     a_point,
               const void*         a_base,
               const PoolLocation& a_location) const noexcept;

  /**
   * @brief Evaluate a BVH union clause.
   * @tparam D Call depth left for the operands.
   * @tparam V Value type: T, or CSGOps::Tagged<T> for closestLeaf().
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
   * @brief The first crossing of the tape's object along a ray, without the normal or primitive there: the trace
   * of firstHit() without asking the leaf it ends on.
   * @param[in] a_shape   The tape.
   * @param[in] a_ray     Ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline CrossingT<T>
  firstCrossing(const Tape<T, UserPrimitives...>& a_shape,
                const RayT<T>&                    a_ray,
                T                                 a_tMin,
                T                                 a_tMax,
                const RayOptions<T>&              a_options) noexcept
  {
    const auto trace =
      [&a_shape, &a_ray, &a_options](const T a_t0, const T a_t1, const bool a_all, RaySpansT<T>& a_spans) noexcept {
        a_shape.raySpans(a_ray, a_t0, a_t1, a_options, !a_all, a_spans);
      };

    return RayDetail::traceCrossing(trace, a_ray, a_tMin, a_tMax);
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
