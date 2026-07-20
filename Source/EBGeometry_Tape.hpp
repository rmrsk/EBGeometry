// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Tape.hpp
 * @brief  Flat linear-SSA "tape" plus single-pass interpreter for implicit-function DAGs.
 * @details The tape is a depth-first flattening of an implicit-function tree into a linear array of
 * trivially-copyable @ref EBGeometry::TapeClause "clauses" that a single forward pass
 * (@ref EBGeometry::TapeView::value) can execute identically on host and (later) device. Every clause
 * dispatches to the SAME trait static functions used by the recursive @c value() path -- @c SphereOp::eval,
 * @c TranslateOp::mapPoint, @c ScaleOp::mapValue, @c UnionOp::combine, @c SmoothMinOp::eval, ... --
 * so the tape is never a second copy of any distance or blend formula; it is a different traversal
 * order over the same single-source-of-truth math.
 *
 * The design mirrors the flat, SoA style of @c EBGeometry::BVH::PackedBVH: a host-owned
 * @ref EBGeometry::Tape holds the clause array and per-op parameter arrays in @c std::vector s and
 * hands out a trivially-copyable @ref EBGeometry::TapeView (raw pointers + counts) for the
 * interpreter to consume.
 *
 * @note An opaque host-callback clause stores a raw @c const @c ImplicitFunction<T>* into the source
 * tree; the tape is therefore only valid while the source tree is alive. A tape containing any host
 * callback is not device-eligible (see @ref EBGeometry::Tape::isDeviceEligible).
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TAPE_HPP
#define EBGEOMETRY_TAPE_HPP

// Std includes
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_AnalyticDistanceFunctions.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_CSG.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Transform.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/// @cond EBGEOMETRY_INTERNAL
// ── Opcode registry (X-macro lists) ───────────────────────────────────────────
// Each list is a single source of truth: it drives (a) the Opcode enum entries, (b) the per-op
// parameter SoA arrays in Tape/TapeView, (c) the OpcodeOf<Op> trait, and (d) the interpreter switch
// cases in EBGeometry_TapeImplem.hpp. Adding a primitive/transform/combiner is one line in the
// matching list plus a trait already present in the AnalyticDistanceFunctions/Transform/CSG headers.
//
// Leaf/transform entries carry (opcode tag, trait struct, SoA member name); combiner/smooth entries
// carry only (opcode tag, trait struct) because sharp combiners take no parameters and smooth
// combiners store only a scalar smoothing length in the shared scalarParams array.

// Leaf primitives: valueSlot[out] = XxxOp::eval(params, coordSlot[in0]).
#define EBGEOMETRY_TAPE_LEAF_OPS(X)                      \
  X(PLANE, PlaneOp, planeParams)                         \
  X(SPHERE, SphereOp, sphereParams)                      \
  X(BOX, BoxOp, boxParams)                               \
  X(TORUS, TorusOp, torusParams)                         \
  X(CYLINDER, CylinderOp, cylinderParams)                \
  X(INF_CYLINDER, InfiniteCylinderOp, infCylinderParams) \
  X(CAPSULE, CapsuleOp, capsuleParams)                   \
  X(INF_CONE, InfiniteConeOp, infConeParams)             \
  X(CONE, ConeOp, coneParams)                            \
  X(ROUNDED_BOX, RoundedBoxOp, roundedBoxParams)         \
  X(ROUNDED_CYLINDER, RoundedCylinderOp, roundedCylinderParams)

// Coordinate transforms: coordSlot[out] = XxxOp::mapPoint(params, coordSlot[in0]).
#define EBGEOMETRY_TAPE_MAPPOINT_OPS(X)      \
  X(TRANSLATE, TranslateOp, translateParams) \
  X(ROTATE, RotateOp, rotateParams)          \
  X(SCALE_MAPPOINT, ScaleOp, scaleParams)    \
  X(ELONGATE, ElongateOp, elongateParams)    \
  X(REFLECT, ReflectOp, reflectParams)

// Value transforms: valueSlot[out] = XxxOp::mapValue(params, valueSlot[in0]).
#define EBGEOMETRY_TAPE_MAPVALUE_OPS(X)         \
  X(COMPLEMENT, ComplementOp, complementParams) \
  X(OFFSET, OffsetOp, offsetParams)             \
  X(ANNULAR, AnnularOp, annularParams)          \
  X(SCALE_MAPVALUE, ScaleOp, scaleParams)

// Sharp combiners (no parameters): valueSlot[out] = XxxOp::combine(valueSlot[in0], valueSlot[in1]).
#define EBGEOMETRY_TAPE_COMBINE_OPS(X) \
  X(UNION, UnionOp)                    \
  X(INTERSECTION, IntersectionOp)      \
  X(DIFFERENCE, DifferenceOp)

// Smooth combiners (scalar smoothing length): valueSlot[out] =
//   XxxOp::eval(valueSlot[in0], valueSlot[in1], scalarParams[paramIndex]).
#define EBGEOMETRY_TAPE_SMOOTH_OPS(X) \
  X(SMOOTH_MIN, SmoothMinOp)          \
  X(SMOOTH_MAX, SmoothMaxOp)          \
  X(EXP_MIN, ExpMinOp)

// Unique (trait, SoA member) pairs for parameter storage. ScaleOp appears in both the mapPoint and
// mapValue lists (it is the one op with both a coordinate and a value remap) but owns a single SoA
// array, so it is listed here exactly once.
#define EBGEOMETRY_TAPE_PARAM_SOA(X)          \
  X(PlaneOp, planeParams)                     \
  X(SphereOp, sphereParams)                   \
  X(BoxOp, boxParams)                         \
  X(TorusOp, torusParams)                     \
  X(CylinderOp, cylinderParams)               \
  X(InfiniteCylinderOp, infCylinderParams)    \
  X(CapsuleOp, capsuleParams)                 \
  X(InfiniteConeOp, infConeParams)            \
  X(ConeOp, coneParams)                       \
  X(RoundedBoxOp, roundedBoxParams)           \
  X(RoundedCylinderOp, roundedCylinderParams) \
  X(TranslateOp, translateParams)             \
  X(RotateOp, rotateParams)                   \
  X(ScaleOp, scaleParams)                     \
  X(ElongateOp, elongateParams)               \
  X(ReflectOp, reflectParams)                 \
  X(ComplementOp, complementParams)           \
  X(OffsetOp, offsetParams)                   \
  X(AnnularOp, annularParams)
/// @endcond

#ifdef EBGEOMETRY_DOXYGEN
/**
 * @brief Interpreter opcode identifying the operation a single tape clause performs.
 * @details The real enum is generated from the X-macro registry lists above; this hand-written twin
 * exists only so Doxygen (which does not expand macros) can document every enumerator. The two must
 * stay in agreement -- add new opcodes to the corresponding registry list, not here.
 */
enum class Opcode : uint16_t
{
  PLANE,            ///< Leaf: signed distance to a plane (PlaneOp).
  SPHERE,           ///< Leaf: signed distance to a sphere (SphereOp).
  BOX,              ///< Leaf: signed distance to an axis-aligned box (BoxOp).
  TORUS,            ///< Leaf: signed distance to a torus (TorusOp).
  CYLINDER,         ///< Leaf: signed distance to a finite cylinder (CylinderOp).
  INF_CYLINDER,     ///< Leaf: signed distance to an infinite cylinder (InfiniteCylinderOp).
  CAPSULE,          ///< Leaf: signed distance to a capsule (CapsuleOp).
  INF_CONE,         ///< Leaf: signed distance to an infinite cone (InfiniteConeOp).
  CONE,             ///< Leaf: signed distance to a finite cone (ConeOp).
  ROUNDED_BOX,      ///< Leaf: signed distance to a rounded box (RoundedBoxOp).
  ROUNDED_CYLINDER, ///< Leaf: signed distance to a rounded cylinder (RoundedCylinderOp).
  TRANSLATE,        ///< Coordinate transform: translation (TranslateOp::mapPoint).
  ROTATE,           ///< Coordinate transform: rotation about a Cartesian axis (RotateOp::mapPoint).
  SCALE_MAPPOINT,   ///< Coordinate transform: pre-scale of the query point (ScaleOp::mapPoint).
  ELONGATE,         ///< Coordinate transform: elongation (ElongateOp::mapPoint).
  REFLECT,          ///< Coordinate transform: reflection (ReflectOp::mapPoint).
  COMPLEMENT,       ///< Value transform: complement/negation (ComplementOp::mapValue).
  OFFSET,           ///< Value transform: constant offset (OffsetOp::mapValue).
  ANNULAR,          ///< Value transform: shell/annulus (AnnularOp::mapValue).
  SCALE_MAPVALUE,   ///< Value transform: post-scale of the child value (ScaleOp::mapValue).
  UNION,            ///< Sharp combiner: union / minimum (UnionOp::combine).
  INTERSECTION,     ///< Sharp combiner: intersection / maximum (IntersectionOp::combine).
  DIFFERENCE,       ///< Sharp combiner: difference / max(a,-b) (DifferenceOp::combine).
  SMOOTH_MIN,       ///< Smooth combiner: polynomial smooth minimum (SmoothMinOp::eval).
  SMOOTH_MAX,       ///< Smooth combiner: polynomial smooth maximum (SmoothMaxOp::eval).
  EXP_MIN,          ///< Smooth combiner: exponential smooth minimum (ExpMinOp::eval).
  BVH_UNION,        ///< BVH-pruned sharp union: valueSlot[out] = min over all non-culled leaf
                    ///  subtrees. paramIndex indexes the TapeBVHBlock array; in0 is the input
                    ///  coordinate slot.
  BVH_SMOOTH_UNION, ///< BVH-pruned smooth union: the two nearest leaf-subtree values, blended with
                    ///  the block's registered smooth operator. Same operand layout as BVH_UNION.
  HOST_CALLBACK     ///< Opaque host node: valueSlot[out] = node->value(coordSlot[in0]). Host only.
};
#else
enum class Opcode : uint16_t
{
#define EBGEOMETRY_TAPE_ENUM_ENTRY(TAG, ...) TAG,
  EBGEOMETRY_TAPE_LEAF_OPS(EBGEOMETRY_TAPE_ENUM_ENTRY) EBGEOMETRY_TAPE_MAPPOINT_OPS(EBGEOMETRY_TAPE_ENUM_ENTRY)
    EBGEOMETRY_TAPE_MAPVALUE_OPS(EBGEOMETRY_TAPE_ENUM_ENTRY) EBGEOMETRY_TAPE_COMBINE_OPS(EBGEOMETRY_TAPE_ENUM_ENTRY)
      EBGEOMETRY_TAPE_SMOOTH_OPS(EBGEOMETRY_TAPE_ENUM_ENTRY)
#undef EBGEOMETRY_TAPE_ENUM_ENTRY
        BVH_UNION,
  BVH_SMOOTH_UNION,
  HOST_CALLBACK
};
#endif

/**
 * @brief Compile-time map from a leaf distance-formula trait to its interpreter @ref Opcode.
 * @details The primary template is deliberately unspecialised (@c registered is @c false) so that an
 * unregistered custom @c Op falls back to an opaque host callback in the leaf @c flatten. The
 * X-macro-generated specialisations (one per leaf op) set @c registered to @c true and expose the
 * matching opcode as @c value. Only leaf ops are registered: transform and combiner nodes pass their
 * opcodes to the builder explicitly and never consult this trait.
 * @tparam Op Distance-formula trait (e.g. @c SphereOp<T>).
 */
template <class Op>
struct OpcodeOf
{
  /**
   * @brief True iff @c Op has a registered opcode.
   */
  static constexpr bool registered = false;
};

/// @cond EBGEOMETRY_INTERNAL
#define EBGEOMETRY_TAPE_OPCODEOF(TAG, OP, MEMBER)     \
  template <class T>                                  \
  struct OpcodeOf<OP<T>>                              \
  {                                                   \
    static constexpr bool   registered = true;        \
    static constexpr Opcode value      = Opcode::TAG; \
  };
EBGEOMETRY_TAPE_LEAF_OPS(EBGEOMETRY_TAPE_OPCODEOF)
#undef EBGEOMETRY_TAPE_OPCODEOF
/// @endcond

/**
 * @brief Detection trait: true iff @c Op has a registered @ref Opcode (i.e. @c OpcodeOf<Op> is
 * specialised).
 * @details Used by the leaf @c flatten with @c if @c constexpr so that a registered primitive lowers
 * to a native leaf clause while an unregistered custom @c Op lowers to an opaque host callback.
 * @tparam Op Distance-formula trait.
 */
template <class Op>
inline constexpr bool IsRegistered = OpcodeOf<Op>::registered;

/**
 * @brief Compile-time map from a smooth-blend operator trait to its interpreter @ref Opcode.
 * @details The primary template is deliberately empty (no nested @c value) so that a smooth
 * combiner instantiated with an unregistered custom @c Blend falls back to an opaque host callback
 * in its @c flatten. The X-macro-generated specialisations (one per smooth op in the ISA) expose
 * the matching opcode as @c value.
 * @tparam Blend Smooth-blend operator trait (e.g. @c SmoothMinOp<T>).
 */
template <class Blend>
struct BlendOpcodeOf
{
};

/// @cond EBGEOMETRY_INTERNAL
#define EBGEOMETRY_TAPE_BLEND_OPCODEOF(TAG, OP)  \
  template <class T>                             \
  struct BlendOpcodeOf<OP<T>>                    \
  {                                              \
    static constexpr Opcode value = Opcode::TAG; \
  };
EBGEOMETRY_TAPE_SMOOTH_OPS(EBGEOMETRY_TAPE_BLEND_OPCODEOF)
#undef EBGEOMETRY_TAPE_BLEND_OPCODEOF
/// @endcond

/**
 * @brief Detection trait: true iff @c Blend has a registered smooth @ref Opcode (i.e.
 * @ref BlendOpcodeOf exposes a nested @c value for it).
 * @details Used by the smooth combiners' @c flatten with @c if @c constexpr so that a registered
 * blend (SmoothMinOp, SmoothMaxOp, ExpMinOp) lowers to a native smooth clause while an unregistered
 * custom blend lowers to an opaque host callback.
 * @tparam Blend Smooth-blend operator trait.
 */
template <class Blend, class = void>
inline constexpr bool isRegisteredBlend = false;

/// @cond EBGEOMETRY_INTERNAL
template <class Blend>
inline constexpr bool isRegisteredBlend<Blend, std::void_t<decltype(BlendOpcodeOf<Blend>::value)>> = true;
/// @endcond

/**
 * @brief Fixed traversal-stack capacity for the tape-internal BVH traversal.
 * @details Mirrors the fixed 256-entry stacks used by every SIMD path of
 * @c BVH::PackedBVH::pruneTraverse; guarded by @c EBGEOMETRY_EXPECT at push time.
 */
inline constexpr uint32_t TapeBVHStackSize = 256;

/**
 * @brief Maximum number of children per tape-internal BVH node.
 * @details Covers every SIMD-supported branching factor K of @c BVH::PackedBVH; enforced by
 * @c static_assert in the BVH-union @c flatten overrides and by @c EBGEOMETRY_EXPECT in
 * @ref TapeBuilder::emitBVHUnion.
 */
inline constexpr uint32_t TapeBVHMaxChildren = 16;

/**
 * @brief One flattened BVH node stored in the tape's shared BVH node array.
 * @details All index fields are global indices into the tape's shared @c bvhNodes / @c bvhChildren
 * / @c bvhLeaves arrays (blocks are appended back to back, so no per-block base arithmetic is
 * needed at query time). The bounding box is stored as raw corners rather than an
 * @c BoundingVolumes::AABBT (keeping this struct a bare bundle of corners and indices); the
 * interpreter rebuilds an @c AABBT on the fly and evaluates its
 * @c getDistance2 -- the single source of truth for the point-to-box distance.
 * @tparam T Floating-point precision.
 */
template <class T>
struct TapeBVHNode
{
  /**
   * @brief Low corner of this node's axis-aligned bounding box.
   */
  Vec3T<T> lo;

  /**
   * @brief High corner of this node's axis-aligned bounding box.
   */
  Vec3T<T> hi;

  /**
   * @brief Interior nodes: index of the first entry in the shared child array (entries are global
   * node indices). Unused for leaves.
   */
  uint32_t childBegin = 0;

  /**
   * @brief Interior nodes: number of children (the K of the source PackedBVH). Zero for leaves.
   */
  uint32_t childCount = 0;

  /**
   * @brief Leaves: index of the first entry in the shared leaf array. Unused for interior nodes.
   */
  uint32_t leafBegin = 0;

  /**
   * @brief Leaves: number of leaf primitives; a value greater than zero marks this node as a leaf
   * (mirroring @c PackedBVH::Node::isLeaf).
   */
  uint32_t leafCount = 0;
};

static_assert(std::is_trivially_copyable_v<TapeBVHNode<float>>, "TapeBVHNode must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TapeBVHNode<double>>, "TapeBVHNode must be trivially copyable");

/**
 * @brief One BVH leaf primitive: a clause sub-range of the same tape holding its flattened subtree.
 * @details The sub-range lives in the tape's deferred region @c [numMainClauses, numClauses) and is
 * executed on demand by a BVH clause; after the range has run, @ref valueSlot holds the subtree's
 * value at the query point.
 */
struct TapeBVHLeaf
{
  /**
   * @brief First clause of the subtree, inside the deferred region.
   */
  uint32_t clauseBegin = 0;

  /**
   * @brief Number of clauses in the subtree (always at least one).
   */
  uint32_t clauseCount = 0;

  /**
   * @brief Value slot holding the subtree's result after the sub-range has executed.
   */
  uint32_t valueSlot = 0;
};

static_assert(std::is_trivially_copyable_v<TapeBVHLeaf>, "TapeBVHLeaf must be trivially copyable");

/**
 * @brief Per-clause header of one BVH-union block, indexed by the clause's @c paramIndex.
 * @details Ties one BVH_UNION / BVH_SMOOTH_UNION clause to its slice of the tape's shared BVH
 * arrays. The blend tag is a tape-internal selector (the smooth opcode of the block's registered
 * blend operator); the public choice of blend is the @c Blend template parameter of
 * @c BVHSmoothUnionIF.
 * @tparam T Floating-point precision.
 */
template <class T>
struct TapeBVHBlock
{
  /**
   * @brief Smoothing length (BVH_SMOOTH_UNION only; zero for BVH_UNION).
   */
  T smoothLen = T(0);

  /**
   * @brief Global index of this block's root node in the shared BVH node array.
   */
  uint32_t rootNode = 0;

  /**
   * @brief Number of nodes in this block (validation/debugging).
   */
  uint32_t numNodes = 0;

  /**
   * @brief Index of this block's first entry in the shared leaf array.
   */
  uint32_t leafBegin = 0;

  /**
   * @brief Total number of leaf primitives in this block.
   */
  uint32_t numLeaves = 0;

  /**
   * @brief Tape-internal blend tag (BVH_SMOOTH_UNION only): the smooth opcode (SMOOTH_MIN,
   * SMOOTH_MAX, or EXP_MIN) whose trait the interpreter's epilogue applies to the two nearest
   * values. Unused for BVH_UNION.
   */
  Opcode blend = Opcode::SMOOTH_MIN;
};

static_assert(std::is_trivially_copyable_v<TapeBVHBlock<float>>, "TapeBVHBlock must be trivially copyable");
static_assert(std::is_trivially_copyable_v<TapeBVHBlock<double>>, "TapeBVHBlock must be trivially copyable");

/**
 * @brief A single trivially-copyable clause in the flat tape (20 bytes).
 * @details One clause performs exactly one operation. The @ref opcode selects which trait static
 * runs and which slot files @ref in0 / @ref in1 / @ref out address (coordinate slots for coordinate
 * transforms, value slots otherwise); the interpreter never branches on anything but @ref opcode.
 */
struct TapeClause
{
  /**
   * @brief Which operation this clause performs.
   */
  Opcode opcode;

  /**
   * @brief Index into this op's parameter array. For a smooth combiner it indexes the shared scalar
   * (smoothing length) array; for a BVH clause it indexes the BVH block array; for a HOST_CALLBACK
   * it indexes the host-node array; unused for sharp combiners. 32 bits wide so that unions over
   * hundreds of thousands of primitives -- the BVH_UNION use case -- cannot overflow it (two padding
   * bytes follow the opcode; the clause is 20 bytes).
   */
  uint32_t paramIndex;

  /**
   * @brief First input slot. Coordinate slot for leaves, coordinate transforms, and host callbacks;
   * first value slot for value transforms and combiners.
   */
  uint32_t in0;

  /**
   * @brief Second input value slot (combiners only; unused otherwise).
   */
  uint32_t in1;

  /**
   * @brief Output slot. Coordinate slot for coordinate transforms; value slot otherwise.
   */
  uint32_t out;
};

static_assert(sizeof(TapeClause) == 20, "TapeClause must be 20 bytes");
static_assert(std::is_trivially_copyable_v<TapeClause>, "TapeClause must be trivially copyable");

/**
 * @brief Forward declaration of the host-side tape builder (defined below and in
 * EBGeometry_TapeImplem.hpp).
 * @tparam T Floating-point precision.
 */
template <class T>
class TapeBuilder;

/**
 * @brief Forward declaration of the CUDA device-side tape uploader (defined in
 * EBGeometry_DeviceTape.hpp; friend of @ref Tape). The class is only defined when an offload
 * compiler translates the TU, so this declaration is harmless on plain host compilers.
 * @tparam T Floating-point precision.
 */
template <class T>
class DeviceTape;

/**
 * @brief Trivially-copyable, non-owning view of a @ref Tape suitable for passing to the interpreter
 * (and, in a later tier, to device code).
 * @details Holds raw pointers into a host-owned @ref Tape's arrays plus the slot counts and root
 * slot. It owns nothing; the backing @ref Tape (and, for host-callback tapes, the source implicit
 * function tree) must outlive every view.
 * @tparam T Floating-point precision.
 */
template <class T>
struct TapeView
{
  /**
   * @brief Pointer to the first clause.
   */
  const TapeClause* clauses = nullptr;

  /**
   * @brief Number of clauses.
   */
  uint32_t numClauses = 0;

  /// @cond EBGEOMETRY_INTERNAL
  // One typed parameter pointer per op-parameter SoA array, generated from the registry so the
  // interpreter can write e.g. view.sphereParams[c.paramIndex].
#define EBGEOMETRY_TAPE_VIEW_PTR(OP, MEMBER) const typename OP<T>::Params* MEMBER = nullptr;
  EBGEOMETRY_TAPE_PARAM_SOA(EBGEOMETRY_TAPE_VIEW_PTR)
#undef EBGEOMETRY_TAPE_VIEW_PTR
  /// @endcond

  /**
   * @brief Pointer to the shared scalar (smoothing-length) array indexed by smooth combiners.
   */
  const T* scalarParams = nullptr;

  /**
   * @brief Pointer to the array of opaque host nodes indexed by HOST_CALLBACK clauses.
   */
  const ImplicitFunction<T>* const* hostNodes = nullptr;

  /**
   * @brief Pointer to the shared BVH node array indexed (via TapeBVHBlock) by BVH clauses.
   */
  const TapeBVHNode<T>* bvhNodes = nullptr;

  /**
   * @brief Pointer to the shared BVH child-index array (entries are global node indices).
   */
  const uint32_t* bvhChildren = nullptr;

  /**
   * @brief Pointer to the shared BVH leaf array (clause sub-ranges of this same tape).
   */
  const TapeBVHLeaf* bvhLeaves = nullptr;

  /**
   * @brief Pointer to the per-clause BVH block headers indexed by a BVH clause's paramIndex.
   */
  const TapeBVHBlock<T>* bvhBlocks = nullptr;

  /**
   * @brief Main-pass clause bound: the forward pass executes clauses [0, numMainClauses); the
   * deferred region [numMainClauses, numClauses) holds BVH leaf subtrees executed on demand by
   * BVH clauses. Equal to numClauses for a tape without BVH clauses.
   */
  uint32_t numMainClauses = 0;

  /**
   * @brief Number of coordinate slots the interpreter's coordinate scratch buffer must hold.
   */
  uint32_t numCoordSlots = 0;

  /**
   * @brief Number of value slots the interpreter's value scratch buffer must hold.
   */
  uint32_t numValueSlots = 0;

  /**
   * @brief Value slot holding the whole tree's result after a full pass.
   */
  uint32_t rootValueSlot = 0;

  /**
   * @brief Whether the tape is free of host callbacks (and thus, in a later tier, device-evaluable).
   */
  bool deviceEligible = true;

  /**
   * @brief Evaluate the tape at a point via a single forward pass (host and device).
   * @details Runs each main-region clause (indices [0, @ref numMainClauses)) in order, dispatching
   * to the shared trait static functions; a BVH clause additionally runs a pruned, iterative
   * traversal that executes non-culled leaf sub-ranges from the deferred clause region on demand.
   * The caller supplies scratch buffers sized to the tape's slot counts (@ref numCoordSlots
   * coordinate slots and @ref numValueSlots value slots). A HOST_CALLBACK clause is only valid on
   * the host.
   * @param[in]  a_point        Query point.
   * @param[out] a_coordScratch Coordinate scratch buffer (at least @ref numCoordSlots entries).
   * @param[out] a_valueScratch Value scratch buffer (at least @ref numValueSlots entries).
   * @return The value at @p a_point (the tape's root value slot after the pass).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE T
  value(const Vec3T<T>& a_point, Vec3T<T>* a_coordScratch, T* a_valueScratch) const noexcept;
};

// TapeView is passed BY VALUE as a device kernel argument (see EBGeometry_DeviceTape.hpp), so it
// must stay trivially copyable for both precisions.
static_assert(std::is_trivially_copyable_v<TapeView<float>>, "TapeView must be trivially copyable (kernel argument)");
static_assert(std::is_trivially_copyable_v<TapeView<double>>, "TapeView must be trivially copyable (kernel argument)");

/**
 * @brief Host-owned flat tape: the clause array plus every per-op parameter array.
 * @details Built by @ref TapeBuilder (via the free @ref compile function) and consumed through a
 * trivially-copyable @ref view. The distance/blend math itself lives entirely in the shared trait
 * static functions; a Tape stores only clauses and parameters, never a formula.
 * @tparam T Floating-point precision.
 */
template <class T>
class Tape
{
  static_assert(std::is_floating_point_v<T>, "Tape<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Constructs an empty tape (populated by @ref TapeBuilder).
   */
  Tape() = default;

  /**
   * @brief Build a trivially-copyable, non-owning view of this tape.
   * @details The returned view holds raw pointers into this tape's arrays and is valid only while
   * this tape (and, for host-callback tapes, the source tree) is alive.
   * @return A @ref TapeView referencing this tape's storage.
   */
  [[nodiscard]] EBGEOMETRY_HOST TapeView<T>
                                view() const noexcept;

  /**
   * @brief Evaluate the tape at a point, managing scratch internally (host convenience).
   * @details Delegates to @ref TapeView::value with two thread-local scratch buffers grown to this
   * tape's slot counts, so each thread's queries are allocation-free after its first call and
   * concurrent readers never share scratch. Host only.
   * @param[in] a_point Query point.
   * @return The value at @p a_point.
   */
  [[nodiscard]] EBGEOMETRY_HOST T
  value(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief Get the number of coordinate slots the interpreter's coordinate scratch must hold.
   * @return Coordinate slot count.
   */
  [[nodiscard]] uint32_t
  getNumCoordSlots() const noexcept
  {
    return m_numCoordSlots;
  }

  /**
   * @brief Get the number of value slots the interpreter's value scratch must hold.
   * @return Value slot count.
   */
  [[nodiscard]] uint32_t
  getNumValueSlots() const noexcept
  {
    return m_numValueSlots;
  }

  /**
   * @brief Get the value slot holding the whole tree's result.
   * @return Root value slot index.
   */
  [[nodiscard]] uint32_t
  getRootValueSlot() const noexcept
  {
    return m_rootValueSlot;
  }

  /**
   * @brief Whether the tape contains no host callbacks and is thus (in a later tier) device-eligible.
   * @return True if no clause is a HOST_CALLBACK.
   */
  [[nodiscard]] bool
  isDeviceEligible() const noexcept
  {
    return m_deviceEligible;
  }

  /**
   * @brief Get the clause array.
   * @return Const reference to the clause vector.
   */
  [[nodiscard]] const std::vector<TapeClause>&
  getClauses() const noexcept
  {
    return m_clauses;
  }

  /**
   * @brief Get the main-pass clause bound.
   * @details The forward pass executes clauses [0, numMainClauses); clauses in
   * [numMainClauses, numClauses) are BVH leaf subtrees executed on demand by BVH clauses. Equal to
   * the total clause count for a tape without BVH clauses.
   * @return Number of main-region clauses.
   */
  [[nodiscard]] uint32_t
  getNumMainClauses() const noexcept
  {
    return m_numMainClauses;
  }

  /**
   * @brief Get the shared BVH node array.
   * @return Const reference to the BVH node vector.
   */
  [[nodiscard]] const std::vector<TapeBVHNode<T>>&
  getBVHNodes() const noexcept
  {
    return m_bvhNodes;
  }

  /**
   * @brief Get the shared BVH child-index array (entries are global node indices).
   * @return Const reference to the BVH child-index vector.
   */
  [[nodiscard]] const std::vector<uint32_t>&
  getBVHChildren() const noexcept
  {
    return m_bvhChildren;
  }

  /**
   * @brief Get the shared BVH leaf array.
   * @return Const reference to the BVH leaf vector.
   */
  [[nodiscard]] const std::vector<TapeBVHLeaf>&
  getBVHLeaves() const noexcept
  {
    return m_bvhLeaves;
  }

  /**
   * @brief Get the per-clause BVH block headers.
   * @return Const reference to the BVH block vector.
   */
  [[nodiscard]] const std::vector<TapeBVHBlock<T>>&
  getBVHBlocks() const noexcept
  {
    return m_bvhBlocks;
  }

private:
  friend class TapeBuilder<T>;
  friend class DeviceTape<T>;

  /**
   * @brief The flat clause array, in single-forward-pass order.
   */
  std::vector<TapeClause> m_clauses;

  /// @cond EBGEOMETRY_INTERNAL
  // One parameter SoA array per op, generated from the registry (ScaleOp shares one array).
#define EBGEOMETRY_TAPE_SOA_MEMBER(OP, MEMBER) std::vector<typename OP<T>::Params> m_##MEMBER;
  EBGEOMETRY_TAPE_PARAM_SOA(EBGEOMETRY_TAPE_SOA_MEMBER)
#undef EBGEOMETRY_TAPE_SOA_MEMBER
  /// @endcond

  /**
   * @brief Shared scalar (smoothing-length) array indexed by smooth combiners.
   */
  std::vector<T> m_scalarParams;

  /**
   * @brief Opaque host nodes indexed by HOST_CALLBACK clauses (raw, non-owning back-pointers).
   */
  std::vector<const ImplicitFunction<T>*> m_hostNodes;

  /**
   * @brief Shared BVH node array (all blocks appended back to back; global indices).
   */
  std::vector<TapeBVHNode<T>> m_bvhNodes;

  /**
   * @brief Shared BVH child-index array (entries are global node indices; may contain duplicates
   * when the source PackedBVH padded an interior node by repeating a child).
   */
  std::vector<uint32_t> m_bvhChildren;

  /**
   * @brief Shared BVH leaf array (clause sub-ranges of this same tape).
   */
  std::vector<TapeBVHLeaf> m_bvhLeaves;

  /**
   * @brief Per-clause BVH block headers indexed by a BVH clause's paramIndex.
   */
  std::vector<TapeBVHBlock<T>> m_bvhBlocks;

  /**
   * @brief Main-pass clause bound (see @ref getNumMainClauses).
   */
  uint32_t m_numMainClauses = 0;

  /**
   * @brief Number of coordinate slots.
   */
  uint32_t m_numCoordSlots = 0;

  /**
   * @brief Number of value slots.
   */
  uint32_t m_numValueSlots = 0;

  /**
   * @brief Root value slot.
   */
  uint32_t m_rootValueSlot = 0;

  /**
   * @brief Device eligibility (false once any host callback is emitted).
   */
  bool m_deviceEligible = true;
};

/**
 * @brief Host-side builder that lowers an implicit-function tree into a @ref Tape.
 * @details The @c flatten overrides on the implicit-function nodes drive the builder: each node
 * emits its clause(s) and recurses into its children, allocating coordinate/value slots as it goes.
 * Every @c emit* helper appends to the correct registry-generated parameter array. Host only.
 * @tparam T Floating-point precision.
 */
template <class T>
class TapeBuilder
{
  static_assert(std::is_floating_point_v<T>, "TapeBuilder<T>: T must be a floating-point type");

public:
  /**
   * @brief Block-local, K-erased description of one PackedBVH node (host-only build type).
   * @details Produced by the BVH-union @c flatten overrides from @c PackedBVH::getNodes and
   * consumed by @ref emitBVHUnion, which globalises the block-local indices into the tape's shared
   * BVH arrays.
   */
  struct BVHNodeInfo
  {
    /**
     * @brief Axis-aligned bounding box of this node's subtree.
     */
    BoundingVolumes::AABBT<T> aabb;

    /**
     * @brief Leaves: block-local index of the first primitive (leaf-traversal order).
     */
    uint32_t primOff = 0;

    /**
     * @brief Leaves: number of primitives; a value greater than zero marks this node as a leaf.
     */
    uint32_t numPrims = 0;

    /**
     * @brief Interior nodes: block-local node indices of the children (size K of the source
     * PackedBVH; duplicates from PackedBVH's power-of-K padding are kept verbatim).
     */
    std::vector<uint32_t> children;
  };

  /**
   * @brief Default constructor. Constructs a builder over an empty tape.
   */
  TapeBuilder() = default;

  /**
   * @brief Allocate a fresh coordinate slot.
   * @return Index of the new coordinate slot.
   */
  [[nodiscard]] EBGEOMETRY_HOST int
  newCoordSlot() noexcept;

  /**
   * @brief Allocate a fresh value slot.
   * @return Index of the new value slot.
   */
  [[nodiscard]] EBGEOMETRY_HOST int
  newValueSlot() noexcept;

  /**
   * @brief Emit a leaf clause: valueSlot[out] = Op::eval(params, coordSlot[a_inCoord]).
   * @tparam Op Leaf distance-formula trait (must be registered).
   * @param[in] a_inCoord Input coordinate slot.
   * @param[in] a_params  Trait parameter bundle.
   * @return Value slot holding the leaf result.
   */
  template <class Op>
  [[nodiscard]] EBGEOMETRY_HOST int
  emitLeaf(int a_inCoord, const typename Op::Params& a_params);

  /**
   * @brief Emit a coordinate-transform clause: coordSlot[out] = Op::mapPoint(params, coordSlot[a_inCoord]).
   * @tparam Op Coordinate-transform trait.
   * @param[in] a_opcode  Opcode for this transform.
   * @param[in] a_inCoord Input coordinate slot.
   * @param[in] a_params  Trait parameter bundle.
   * @return New coordinate slot holding the transformed point.
   */
  template <class Op>
  [[nodiscard]] EBGEOMETRY_HOST int
  emitCoordTransform(Opcode a_opcode, int a_inCoord, const typename Op::Params& a_params);

  /**
   * @brief Emit a value-transform clause: valueSlot[out] = Op::mapValue(params, valueSlot[a_inValue]).
   * @tparam Op Value-transform trait.
   * @param[in] a_opcode  Opcode for this transform.
   * @param[in] a_inValue Input value slot.
   * @param[in] a_params  Trait parameter bundle.
   * @return New value slot holding the transformed value.
   */
  template <class Op>
  [[nodiscard]] EBGEOMETRY_HOST int
  emitValueTransform(Opcode a_opcode, int a_inValue, const typename Op::Params& a_params);

  /**
   * @brief Emit a sharp-combiner clause: valueSlot[out] = combine(valueSlot[a_valA], valueSlot[a_valB]).
   * @param[in] a_opcode Sharp-combiner opcode (UNION, INTERSECTION, or DIFFERENCE).
   * @param[in] a_valA   First input value slot.
   * @param[in] a_valB   Second input value slot.
   * @return New value slot holding the combined value.
   */
  [[nodiscard]] EBGEOMETRY_HOST int
  emitCombine(Opcode a_opcode, int a_valA, int a_valB);

  /**
   * @brief Emit a smooth-combiner clause: valueSlot[out] = eval(valueSlot[a_valA], valueSlot[a_valB], a_smoothLen).
   * @param[in] a_opcode    Smooth-combiner opcode (SMOOTH_MIN, SMOOTH_MAX, or EXP_MIN).
   * @param[in] a_valA      First input value slot.
   * @param[in] a_valB      Second input value slot.
   * @param[in] a_smoothLen Smoothing length.
   * @return New value slot holding the blended value.
   */
  [[nodiscard]] EBGEOMETRY_HOST int
  emitSmooth(Opcode a_opcode, int a_valA, int a_valB, T a_smoothLen);

  /**
   * @brief Emit an opaque host-callback clause and mark the tape device-ineligible.
   * @details Stores a raw back-pointer to @p a_node; the tape is valid only while the source tree is
   * alive. At evaluation the interpreter calls @c a_node->value(coordSlot[a_inCoord]) on the host.
   * @param[in] a_inCoord Input coordinate slot.
   * @param[in] a_node    Non-owning pointer to the source implicit function.
   * @return Value slot holding the node's value.
   */
  [[nodiscard]] EBGEOMETRY_HOST int
  emitHostCallback(int a_inCoord, const ImplicitFunction<T>* a_node);

  /**
   * @brief Emit a BVH-pruned union clause (sharp or smooth) and record its leaf primitives for
   * deferred flattening.
   * @details Globalises @p a_nodes into the tape's shared BVH arrays, appends the block header and
   * the BVH clause, and records the primitives on a pending list; @ref finish later flattens every
   * pending primitive's subtree into the tape's deferred clause region. Every leaf subtree is
   * flattened against the same input frame @p a_inCoord (matching UnionIF::flatten's convention).
   * @param[in] a_inCoord    Input coordinate slot.
   * @param[in] a_nodes      Block-local node descriptions, depth-first with the root first.
   * @param[in] a_primitives Leaf primitives in leaf-traversal order (non-owning; must stay alive
   * until @ref finish has run).
   * @param[in] a_smooth     True for BVH_SMOOTH_UNION, false for BVH_UNION.
   * @param[in] a_blend      Tape-internal blend tag (SMOOTH_MIN, SMOOTH_MAX, or EXP_MIN); only
   * read for the smooth variant.
   * @param[in] a_smoothLen  Smoothing length (must be positive for the smooth variant; pass zero
   * for the sharp union).
   * @return Value slot holding the union's result.
   */
  [[nodiscard]] EBGEOMETRY_HOST int
  emitBVHUnion(int                                       a_inCoord,
               std::vector<BVHNodeInfo>&&                a_nodes,
               std::vector<const ImplicitFunction<T>*>&& a_primitives,
               bool                                      a_smooth,
               Opcode                                    a_blend,
               T                                         a_smoothLen);

  /**
   * @brief Whether the builder is currently flattening a BVH leaf subtree (finish's deferred
   * phase).
   * @details Used by the BVH-union @c flatten overrides to route a nested BVH union to an opaque
   * host callback: the tape's interpreter executes leaf sub-ranges through an inner clause loop
   * that does not itself handle BVH clauses (that would require a per-nesting-level traversal
   * stack), so a BVH union inside a BVH leaf must stay opaque in this tier.
   * @return True while @ref finish is flattening pending BVH leaf subtrees.
   */
  [[nodiscard]] EBGEOMETRY_HOST bool
  isFlatteningBVHLeaf() const noexcept;

  /**
   * @brief Finalise and hand back the built tape.
   * @details Freezes the main clause region, then flattens every pending BVH block's leaf
   * primitives into the deferred clause region (reusing one shared coordinate/value slot window
   * across all leaves; see the implementation for why that is safe).
   * @param[in] a_rootValueSlot Value slot holding the whole tree's result.
   * @return The completed tape (moved out of the builder).
   */
  [[nodiscard]] EBGEOMETRY_HOST Tape<T>
                                finish(int a_rootValueSlot);

private:
  /**
   * @brief Select the parameter SoA array for a given op trait.
   * @tparam Op Distance-formula trait.
   * @return Reference to the matching parameter array in the tape under construction.
   */
  template <class Op>
  [[nodiscard]] EBGEOMETRY_HOST std::vector<typename Op::Params>&
                                paramArrayFor();

  /**
   * @brief One BVH block recorded by @ref emitBVHUnion whose leaf subtrees still await flattening.
   */
  struct PendingBVHBlock
  {
    /**
     * @brief Index of the block in the tape's BVH block array.
     */
    uint32_t blockIndex = 0;

    /**
     * @brief Coordinate slot every leaf subtree of this block is flattened against.
     */
    int coordSlot = 0;

    /**
     * @brief Leaf primitives in leaf-traversal order (non-owning).
     */
    std::vector<const ImplicitFunction<T>*> primitives;
  };

  /**
   * @brief The tape being assembled; moved out by @ref finish.
   */
  Tape<T> m_tape;

  /**
   * @brief BVH blocks awaiting the deferred leaf-flattening phase of @ref finish.
   */
  std::vector<PendingBVHBlock> m_pendingBlocks;

  /**
   * @brief True while @ref finish is flattening pending BVH leaf subtrees (see
   * @ref isFlatteningBVHLeaf).
   */
  bool m_flatteningBVHLeaf = false;
};

/**
 * @brief Compile an implicit-function tree into a flat @ref Tape.
 * @details Seeds coordinate slot 0 with the query point's frame and drives the tree's @c flatten
 * lowering overrides. Host only. Evaluate the result with @ref Tape::value (host convenience) or
 * @ref TapeView::value (caller-provided scratch).
 * @tparam T Floating-point precision.
 * @param[in] a_tree Source implicit-function tree (must outlive any host-callback tape produced).
 * @return The compiled tape.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST Tape<T>
                              compile(const ImplicitFunction<T>& a_tree);

} // namespace EBGeometry

#include "EBGeometry_TapeImplem.hpp"

#endif
