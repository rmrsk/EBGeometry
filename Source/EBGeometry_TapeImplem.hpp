// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TapeImplem.hpp
 * @brief  Implementation of EBGeometry_Tape.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TAPEIMPLEM_HPP
#define EBGEOMETRY_TAPEIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Blend.hpp"
#include "EBGeometry_CSGOps.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Tape.hpp"

namespace EBGeometry {

namespace TapeDetail {

/**
 * @brief One host vector per primitive type, holding the type's stored form.
 * @details Internal helper; not part of the public API.
 * @tparam List TypeList of primitive types.
 */
template <class List>
struct StagingOf;

/**
 * @brief One host vector per primitive type, holding the type's stored form.
 * @details Internal helper; not part of the public API.
 * @tparam Ps Primitive types.
 */
template <class... Ps>
struct StagingOf<TypeList<Ps...>>
{
  /**
   * @brief The vectors.
   */
  using type = std::tuple<std::vector<CSGDetail::Stored<Ps>>...>;
};

template <class T, size_t K, class List>
struct Staging
{
  /**
   * @brief The primitives, by type.
   */
  typename StagingOf<List>::type m_primitives;

  /**
   * @brief The clauses of each sub-program.
   */
  std::vector<std::vector<Clause>> m_programs;

  /**
   * @brief The ray program of each sub-program (empty for one a ray never follows).
   */
  std::vector<std::vector<RayClause>> m_rayPrograms;

  /**
   * @brief The sub-program each sub-program was compiled for (UINT32_MAX for the whole object's).
   */
  std::vector<uint32_t> m_parents;

  /**
   * @brief The clause ranges a ray marches: sub-program, first clause and one past the last, within it.
   */
  std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> m_marched;

  /**
   * @brief Every clause, laid out as in the pool.
   */
  std::vector<Clause> m_flatClauses;

  /**
   * @brief Each sub-program's clause range, then each marched range, then each ray program's, then each
   * sub-program's parent (in m_begin).
   */
  std::vector<Program> m_flatPrograms;

  /**
   * @brief How many sub-programs there are, laid out first.
   */
  uint32_t m_flatNumPrograms = 0;

  /**
   * @brief Where the ray program ranges begin in m_flatPrograms.
   */
  uint32_t m_flatRayPrograms = 0;

  /**
   * @brief How deep each sub-program's calls nest.
   */
  std::vector<uint32_t> m_depth;

  /**
   * @brief The constant parameters.
   */
  std::vector<T> m_params;

  /**
   * @brief The BVH unions.
   */
  std::vector<UnionBVH<T, K>> m_unions;

  /**
   * @brief A box enclosing the object.
   */
  BoundingVolumes::AABBT<T> m_boundingVolume;

  /**
   * @brief The object's distance quality.
   */
  DistanceQuality m_quality = DistanceQuality::NotADistance;

  /**
   * @brief How many primitives (leaves) the tape has, each occurrence counted.
   */
  uint32_t m_numPrimitives = 0;

  /**
   * @brief The most objects in one union evaluated as written.
   */
  uint32_t m_largestPlainUnion = 0;

  /**
   * @brief Each leaf's label; empty when the expression has none.
   */
  std::vector<uint32_t> m_surfaceLabels;

  /**
   * @brief How many distinct labels the expression has.
   */
  uint32_t m_numLabels = 0;

  /**
   * @brief Each leaf's first surface, and the number of surfaces last; empty when no leaf is a container.
   */
  std::vector<uint32_t> m_surfaces;

  /**
   * @brief The part labels, laid out as Tape::Layout::m_partSurfaceLabels; empty when no leaf's parts are named.
   */
  std::vector<uint32_t> m_partSurfaceLabels;

  /**
   * @brief The regions' table, laid out as Tape::Layout::m_regionPrograms; empty without the
   * feature or an Overlay.
   */
  std::vector<uint32_t> m_regionPrograms;
};

/**
 * @brief Drill into the deciding leaf (see Tape::closestSurface()): a container adds its member's own surface to the
 * leaf's first, with the member's part and the point in the member's frame; a shape with parts reports its part.
 * @details Internal helper; not part of the public API. A functor rather than a lambda, so that its call
 * operator is a device function in a CUDA or HIP translation unit (see Tape::visitPrimitive()).
 * @tparam T Floating-point precision.
 */
template <class T>
struct Drill
{
  /**
   * @brief The surface being reported, with the leaf's first surface and the point in the leaf's frame.
   */
  SurfaceRef<T>* m_surface;

  /**
   * @brief Drill into a primitive: a container reports its own surface; any other shape is one surface.
   * @tparam P Primitive type.
   * @param[in] a_primitive The deciding leaf's primitive.
   */
  template <class P>
  EBGEOMETRY_HOST_DEVICE
  inline void
  operator()([[maybe_unused]] const P& a_primitive) const noexcept
  {
    if constexpr (hasSurfaces<P, T>) {
      const SurfaceRef<T> inner = SurfaceDetail::closestSurfaceOf<T>(a_primitive, m_surface->m_localPoint);

      // An empty collection names no surface (it decides nothing unless the tape holds nothing else).
      if (inner.m_surface != SurfaceRef<T>::None) {
        m_surface->m_surface += inner.m_surface;
        m_surface->m_part       = inner.m_part;
        m_surface->m_localPoint = inner.m_localPoint;
      }
      else {
        m_surface->m_surface = SurfaceRef<T>::None;
      }
    }
    else if constexpr (hasParts<P, T>) {
      m_surface->m_part = a_primitive.closestPart(m_surface->m_localPoint);
    }
  }
};

/**
 * @brief Lay the compiled sub-programs, marched ranges and ray programs out one after another, as in the
 * pool: a marched range becomes a sub-program after the compiled ones, and the ray clauses that march it
 * name it by that index; the ray programs' clauses follow the sub-programs' clauses, and their ranges the
 * marched ranges.
 * @details Internal helper; not part of the public API.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor.
 * @tparam List TypeList of primitive types.
 * @param[in,out] a_staging Staging.
 */
template <class T, size_t K, class List>
inline void
flatten(Staging<T, K, List>& a_staging)
{
  const auto numPrograms = static_cast<uint32_t>(a_staging.m_programs.size());

  for (const auto& program : a_staging.m_programs) {
    Program range;

    range.m_begin = static_cast<uint32_t>(a_staging.m_flatClauses.size());

    a_staging.m_flatClauses.insert(a_staging.m_flatClauses.end(), program.begin(), program.end());

    range.m_end = static_cast<uint32_t>(a_staging.m_flatClauses.size());

    a_staging.m_flatPrograms.push_back(range);
  }

  for (const auto& [program, begin, end] : a_staging.m_marched) {
    const uint32_t offset = a_staging.m_flatPrograms[program].m_begin;

    a_staging.m_flatPrograms.push_back(Program{offset + begin, offset + end});
  }

  a_staging.m_flatNumPrograms = numPrograms;
  a_staging.m_flatRayPrograms = static_cast<uint32_t>(a_staging.m_flatPrograms.size());

  for (const auto& program : a_staging.m_rayPrograms) {
    Program range;

    range.m_begin = static_cast<uint32_t>(a_staging.m_flatClauses.size());

    for (RayClause clause : program) {
      // A Marched clause's sub-program, though not a deferred level set's primitive (bit 3).
      if (clause.m_op == RayOp::Marched && (clause.m_flags & 8U) == 0U) {
        clause.m_a += numPrograms;
      }

      a_staging.m_flatClauses.push_back(stored(clause));
    }

    range.m_end = static_cast<uint32_t>(a_staging.m_flatClauses.size());

    a_staging.m_flatPrograms.push_back(range);
  }

  // Tape::rayLeafHit() finds a clause's ray program by bisection over where they begin, so they must begin
  // in program order, and walks up the parents, which come after them.
  for (uint32_t i = 1; i < numPrograms; i++) {
    EBGEOMETRY_EXPECT(a_staging.m_flatPrograms[a_staging.m_flatRayPrograms + i - 1].m_begin <=
                      a_staging.m_flatPrograms[a_staging.m_flatRayPrograms + i].m_begin);
  }

  for (const uint32_t parent : a_staging.m_parents) {
    a_staging.m_flatPrograms.push_back(Program{parent, 0U});
  }
}

/**
 * @brief Compiles an Expr into Staging.
 * @details Internal helper; not part of the public API. Host-only; recursion over the expression tree is
 * fine here, since only the evaluator runs on a device.
 * @tparam T        Floating-point precision.
 * @tparam K        BVH branching factor.
 * @tparam List     TypeList of primitive types.
 * @tparam Features TypeList of the features compiled (see TapeFeature).
 */
template <class T, size_t K, class List, class Features>
class Compiler
{
public:
  /**
   * @brief The number of primitives (leaves) emitted so far.
   * @return The count.
   */
  [[nodiscard]] uint32_t
  numPrimitives() const noexcept
  {
    return m_nextLeaf;
  }

  /**
   * @brief Compile a region's program: a subexpression emitted again, with the leaves of
   * the given subtrees numbered from the given leaves, as in the whole expression.
   * @param[in] a_node   The region's solid (its operand under the maps and masks above it).
   * @param[in] a_starts The first leaf of each subtree emitted again (the operand and the masks).
   * @return The program's index.
   */
  uint32_t
  compileRegion(const ExprDetail::Node<T>&                                          a_node,
                const std::vector<std::pair<const ExprDetail::Node<T>*, uint32_t>>& a_starts)
  {
    const uint32_t next = m_nextLeaf;

    m_leafStarts = a_starts;

    // Traced like the whole object, for the regions' ray queries.
    const RayEmit ray{false, 0U};

    const uint32_t program = this->compileProgram(a_node, &ray);

    m_leafStarts.clear();

    m_nextLeaf = next;

    return program;
  }

  /**
   * @brief The most objects in one union evaluated as written, over the unions emitted so far.
   * @return The count.
   */
  [[nodiscard]] uint32_t
  largestPlainUnion() const noexcept
  {
    return m_largestPlainUnion;
  }

  /**
   * @brief Set up.
   * @param[in,out] a_pool           Pool the BVHs are built in, and that pool-resident primitives must live in.
   * @param[in]     a_options        Compile settings.
   * @param[in]     a_valueStackSize Values a sub-program may hold.
   * @param[in]     a_pointStackSize Points a sub-program may hold.
   * @param[in]     a_rayStackSize   Span lists a ray program may hold.
   * @param[in,out] a_staging        Where the compiled arrays go.
   */
  Compiler(Pool&                a_pool,
           const TapeOptions&   a_options,
           const uint32_t       a_valueStackSize,
           const uint32_t       a_pointStackSize,
           const uint32_t       a_rayStackSize,
           Staging<T, K, List>& a_staging) noexcept
    : m_pool(a_pool),
      m_options(a_options),
      m_valueStackSize(a_valueStackSize),
      m_pointStackSize(a_pointStackSize),
      m_rayStackSize(a_rayStackSize),
      m_staging(a_staging)
  {}

  /**
   * @brief Whether a ray program needs all of an expression's spans, rather than only its first crossing.
   */
  struct RayEmit
  {
    /**
     * @brief Whether all the spans are needed.
     */
    bool m_all;

    /**
     * @brief Span lists on the ray stack below the expression's.
     */
    uint32_t m_level;

    /**
     * @brief Whether the expression is traced at a level set of its value (under an Offset or Annular): its
     * primitive is traced through RayDetail::LevelView.
     */
    bool m_atLevel = false;

    /**
     * @brief The level: the offset, or the annulus's half-thickness, in the expression's own values.
     */
    T m_amount = T(0);

    /**
     * @brief Whether it is an annulus.
     */
    bool m_annulus = false;
  };

  /**
   * @brief Compile an expression into a new sub-program, and into its ray program.
   * @param[in] a_node Expression.
   * @param[in] a_ray  For the ray program: whether all the spans are needed; null for a sub-program a ray
   * never follows (one that a marched subtree calls).
   * @return The sub-program's index.
   */
  uint32_t
  compileProgram(const ExprDetail::Node<T>& a_node, const RayEmit* a_ray)
  {
    const auto id = static_cast<uint32_t>(m_staging.m_programs.size());

    m_staging.m_programs.emplace_back();
    m_staging.m_rayPrograms.emplace_back();
    m_staging.m_parents.push_back(m_current);
    m_staging.m_depth.push_back(0U);

    const uint32_t parent = m_current;

    m_current = id;

    // The input point is on the point stack when the sub-program starts.
    State state{id, 0U, 1U, 0U, 1U};

    if (a_ray != nullptr) {
      const RayEmit ray{a_ray->m_all, 0U};

      this->emit(state, a_node, &ray);
    }
    else {
      this->emit(state, a_node, nullptr);
    }

    EBGEOMETRY_EXPECT(state.m_values == 1U && state.m_points == 1U);

    EBGEOMETRY_REQUIRE(state.m_maxValues <= m_valueStackSize,
                       "Tape: the expression needs %u values at once in one sub-program, but the tape holds at "
                       "most %u; operations nest too deeply",
                       unsigned(state.m_maxValues),
                       unsigned(m_valueStackSize));
    EBGEOMETRY_REQUIRE(state.m_maxPoints <= m_pointStackSize,
                       "Tape: the expression nests %u point maps (Translate, Rotate, Elongate, ...) in one "
                       "sub-program, but a tape allows at most %u",
                       unsigned(state.m_maxPoints - 1U),
                       unsigned(m_pointStackSize - 1U));

    m_current = parent;

    return id;
  }

private:
  /**
   * @brief Number of primitive types.
   */
  static constexpr size_t NumTypes = List::size;

  /**
   * @brief Stack use while a sub-program is emitted.
   */
  struct State
  {
    /**
     * @brief Sub-program index.
     */
    uint32_t m_program;

    /**
     * @brief Values on the stack.
     */
    uint32_t m_values;

    /**
     * @brief Points on the stack.
     */
    uint32_t m_points;

    /**
     * @brief Most values on the stack so far.
     */
    uint32_t m_maxValues;

    /**
     * @brief Most points on the stack so far.
     */
    uint32_t m_maxPoints;
  };

  /**
   * @brief Pool.
   */
  Pool& m_pool;

  /**
   * @brief Compile settings.
   */
  TapeOptions m_options;

  /**
   * @brief Value stack capacity.
   */
  uint32_t m_valueStackSize;

  /**
   * @brief Point stack capacity.
   */
  uint32_t m_pointStackSize;

  /**
   * @brief Ray program list stack capacity.
   */
  uint32_t m_rayStackSize;

  /**
   * @brief Output.
   */
  Staging<T, K, List>& m_staging;

  /**
   * @brief The sub-program being compiled (UINT32_MAX before the first).
   */
  uint32_t m_current = UINT32_MAX;

  /**
   * @brief The number the next primitive leaf gets (see Tape::closestLeaf()).
   */
  uint32_t m_nextLeaf = 0;

  /**
   * @brief While a region's program is compiled, the subtrees whose leaves are numbered from a given leaf.
   */
  std::vector<std::pair<const ExprDetail::Node<T>*, uint32_t>> m_leafStarts;

  /**
   * @brief The most objects in one union evaluated as written (see plainUnionSize()).
   */
  uint32_t m_largestPlainUnion = 0;

  /**
   * @brief Append a clause and track the stacks.
   * @param[in,out] a_state  Sub-program state.
   * @param[in]     a_clause Clause.
   * @param[in]     a_values Change in the number of values.
   * @param[in]     a_points Change in the number of points.
   */
  void
  push(State& a_state, const Clause& a_clause, const int a_values, const int a_points)
  {
    m_staging.m_programs[a_state.m_program].push_back(a_clause);

    a_state.m_values = static_cast<uint32_t>(static_cast<int>(a_state.m_values) + a_values);
    a_state.m_points = static_cast<uint32_t>(static_cast<int>(a_state.m_points) + a_points);

    a_state.m_maxValues = Math::max(a_state.m_maxValues, a_state.m_values);
    a_state.m_maxPoints = Math::max(a_state.m_maxPoints, a_state.m_points);
  }

  /**
   * @brief A clause.
   * @param[in] a_op    Operation.
   * @param[in] a_a     First operand.
   * @param[in] a_b     Second operand.
   * @param[in] a_c     Third operand.
   * @param[in] a_flags Flags.
   * @param[in] a_type  Primitive type.
   * @return The clause.
   */
  static Clause
  clause(const Op       a_op,
         const uint32_t a_a     = 0U,
         const uint32_t a_b     = 0U,
         const uint32_t a_c     = 0U,
         const uint8_t  a_flags = 0U,
         const uint16_t a_type  = 0U) noexcept
  {
    Clause c;

    c.m_op    = a_op;
    c.m_flags = a_flags;
    c.m_type  = a_type;
    c.m_a     = a_a;
    c.m_b     = a_b;
    c.m_c     = a_c;

    return c;
  }

  /**
   * @brief Append constant parameters.
   * @param[in] a_values Values.
   * @return Index of the first one.
   */
  uint32_t
  params(const std::vector<T>& a_values)
  {
    const auto first = static_cast<uint32_t>(m_staging.m_params.size());

    m_staging.m_params.insert(m_staging.m_params.end(), a_values.begin(), a_values.end());

    return first;
  }

  /**
   * @brief Record that a sub-program calls another.
   * @param[in] a_state  The caller's state.
   * @param[in] a_callee The callee.
   */
  void
  calls(const State& a_state, const uint32_t a_callee)
  {
    uint32_t& depth = m_staging.m_depth[a_state.m_program];

    depth = Math::max(depth, 1U + m_staging.m_depth[a_callee]);
  }

  /**
   * @brief The index of a primitive type, from I onwards; NumTypes if it is not a primitive type.
   * @tparam I First index to test.
   * @param[in] a_type Type.
   * @return The index.
   */
  template <size_t I>
  static uint32_t
  findType(const std::type_index& a_type) noexcept
  {
    if constexpr (I < NumTypes) {
      if (a_type == std::type_index(typeid(typename TypeAt<I, List>::type))) {
        return static_cast<uint32_t>(I);
      }

      return findType<I + 1>(a_type);
    }
    else {
      return static_cast<uint32_t>(NumTypes);
    }
  }

  /**
   * @brief Stage a primitive of type index a_type, from I onwards.
   * @tparam I First type index to test.
   * @param[in] a_type   Type index.
   * @param[in] a_object The primitive.
   * @return Its index within its type's array.
   */
  template <size_t I>
  uint32_t
  stage(const uint32_t a_type, const void* a_object)
  {
    if constexpr (I < NumTypes) {
      if (a_type == I) {
        using P = typename TypeAt<I, List>::type;

        auto& stored = std::get<I>(m_staging.m_primitives);

        stored.push_back(CSGDetail::toStored(*static_cast<const P*>(a_object)));

        return static_cast<uint32_t>(stored.size() - 1);
      }

      return this->stage<I + 1>(a_type, a_object);
    }
    else {
      EBGEOMETRY_EXPECT(false);

      return 0U;
    }
  }

  /**
   * @brief Whether a union is evaluated with a BVH.
   * @param[in] a_node The union.
   * @return True if it was built with Accelerate::BVH.
   */
  [[nodiscard]] bool
  useBVH(const ExprDetail::Node<T>& a_node) const noexcept
  {
    return a_node.m_bvh;
  }

  /**
   * @brief How many objects a union evaluated as written evaluates: its operands, with those of the sharp
   * unions evaluated as written directly under a sharp one counted in its place.
   * @param[in] a_node The union (a Union or SmoothUnion not built with Accelerate::BVH).
   * @return The count.
   */
  [[nodiscard]] uint32_t
  plainUnionSize(const ExprDetail::Node<T>& a_node) const noexcept
  {
    using ExprDetail::Kind;

    uint32_t size = 0;

    for (const auto& child : a_node.m_children) {
      const ExprDetail::Node<T>& c = child.node();

      const bool nested = (a_node.m_kind == Kind::Union && c.m_kind == Kind::Union && !this->useBVH(c));

      size += nested ? this->plainUnionSize(c) : 1U;
    }

    return size;
  }

  /**
   * @brief Whether a sub-program is one shape, maybe under point maps and a value scale (see
   * TapeDetail::LeafProgram).
   * @param[in] a_program The sub-program.
   * @return True if its clauses are only PushAffine, Primitive, PopPoint and MulConstant, with one Primitive.
   */
  [[nodiscard]] bool
  isLeafProgram(const uint32_t a_program) const noexcept
  {
    uint32_t primitives = 0;

    for (const Clause& c : m_staging.m_programs[a_program]) {
      if (c.m_op == Op::Primitive) {
        primitives++;
      }
      else if (c.m_op != Op::PushAffine && c.m_op != Op::PopPoint && c.m_op != Op::MulConstant) {
        return false;
      }
    }

    return primitives == 1U && a_program < LeafProgram;
  }

  /**
   * @brief Compile each operand of a union into its own sub-program and build a BVH over them.
   * @param[in,out] a_state The caller's state.
   * @param[in]     a_node  The union.
   * @param[in]     a_ray   For the operands' ray programs, or null.
   * @return The BVH union's index.
   */
  uint32_t
  buildUnion(State& a_state, const ExprDetail::Node<T>& a_node, const RayEmit* a_ray)
  {
    std::vector<uint32_t>                  programs;
    std::vector<BoundingVolumes::AABBT<T>> boxes;

    programs.reserve(a_node.m_children.size());
    boxes.reserve(a_node.m_children.size());

    for (const auto& child : a_node.m_children) {
      const uint32_t program = this->compileProgram(child.node(), a_ray);

      this->calls(a_state, program);

      // An empty object (an intersection of disjoint objects, say) has no box to place in the tree; it
      // goes with the unbounded operands, which every query evaluates.
      const BoundingVolumes::AABBT<T>& box = child.computeBoundingVolume();

      programs.push_back(this->isLeafProgram(program) ? (program | LeafProgram) : program);
      boxes.push_back(ExprDetail::isEmpty(box) ? ExprDetail::everywhere<T>() : box);
    }

    const auto parts = CSGDetail::buildUnionParts<T, uint32_t, K>(
      m_pool, programs, boxes, m_options.m_construction, BVH::PackedBVH<T, uint32_t, K>::defaultConstructionOptions());

    UnionBVH<T, K> bvh;

    bvh.m_bvh       = parts.bvh.layout();
    bvh.m_unbounded = parts.unbounded.layout();

    m_staging.m_unions.push_back(bvh);

    return static_cast<uint32_t>(m_staging.m_unions.size() - 1);
  }

  /**
   * @brief Whether the primitive type of index a_type, from I onwards, has a ray form.
   * @tparam I First index to test.
   * @param[in] a_type Type index.
   * @return True if it has.
   */
  template <size_t I>
  [[nodiscard]] static bool
  hasRayForm(const uint32_t a_type) noexcept
  {
    if constexpr (I < NumTypes) {
      if (a_type == I) {
        return RayDetail::HasRayFirstHit<typename TypeAt<I, List>::type, T>::value;
      }

      return hasRayForm<I + 1>(a_type);
    }
    else {
      return false;
    }
  }

  /**
   * @brief Whether the primitive type of index a_type, from I onwards, has closed-form level sets (see
   * RayDetail::LevelForm).
   * @tparam I First index to test.
   * @param[in] a_type Type index.
   * @return True if it has.
   */
  template <size_t I>
  [[nodiscard]] static bool
  hasLevelForm(const uint32_t a_type) noexcept
  {
    if constexpr (I < NumTypes) {
      if (a_type == I) {
        return RayDetail::HasLevelForm<typename TypeAt<I, List>::type>::value;
      }

      return hasLevelForm<I + 1>(a_type);
    }
    else {
      return false;
    }
  }

  /**
   * @brief Whether the primitive type of index a_type, from I onwards, is a mesh whose level sets an
   * intersection traces only within its other operands' spans (see RayDetail::DeferredOperand).
   * @tparam I First index to test.
   * @param[in] a_type Type index.
   * @return True if it is.
   */
  template <size_t I>
  [[nodiscard]] static bool
  hasMeshLevel(const uint32_t a_type) noexcept
  {
    if constexpr (I < NumTypes) {
      if (a_type == I) {
        return RayDetail::IsMeshLevel<typename TypeAt<I, List>::type>::value;
      }

      return hasMeshLevel<I + 1>(a_type);
    }
    else {
      return false;
    }
  }

  /**
   * @brief Whether a ray program traces an expression's level sets exactly: a primitive with closed-form level
   * sets, or an affine map of one.
   * @param[in] a_node Expression.
   * @return True if traced.
   */
  [[nodiscard]] static bool
  levelTraced(const ExprDetail::Node<T>& a_node) noexcept
  {
    using ExprDetail::Kind;

    switch (a_node.m_kind) {
    case Kind::Primitive: {
      return hasLevelForm<0>(findType<0>(a_node.m_leaf.m_type));
    }
    case Kind::Affine: {
      return a_node.m_valueScale > T(0) && levelTraced(a_node.m_children.front().node());
    }
    default: {
      return false;
    }
    }
  }

  /**
   * @brief Whether a ray program traces an expression's node exactly, from its operands' spans: a union,
   * intersection, complement or affine map, a primitive with a ray form, or, with TapeFeature::ExactOffsets,
   * an offset or annulus of a primitive (or an affine map of one) with closed-form level sets. Anything else
   * is marched.
   * @param[in] a_node Expression.
   * @return True if traced.
   */
  [[nodiscard]] static bool
  traced(const ExprDetail::Node<T>& a_node) noexcept
  {
    using ExprDetail::Kind;

    switch (a_node.m_kind) {
    case Kind::Primitive: {
      return hasRayForm<0>(findType<0>(a_node.m_leaf.m_type));
    }
    case Kind::Offset:
    case Kind::Annular: {
      return IndexOf<TapeFeature::ExactOffsets, Features>::value < Features::size &&
             levelTraced(a_node.m_children.front().node());
    }
    case Kind::Affine:
    case Kind::Complement:
    case Kind::Union:
    case Kind::Intersection: {
      return true;
    }
    default: {
      return false;
    }
    }
  }

  /**
   * @brief How many span lists a ray program holds at once to trace an expression.
   * @param[in] a_node Expression.
   * @return The count.
   */
  [[nodiscard]] uint32_t
  raySlots(const ExprDetail::Node<T>& a_node) const noexcept
  {
    using ExprDetail::Kind;

    if (!traced(a_node)) {
      return 1U;
    }

    switch (a_node.m_kind) {
    case Kind::Affine:
    case Kind::Complement: {
      return this->raySlots(a_node.m_children.front().node());
    }
    case Kind::Union:
    case Kind::Intersection: {
      if (a_node.m_kind == Kind::Union && this->useBVH(a_node)) {
        return 1U;
      }

      uint32_t slots = this->raySlots(a_node.m_children.front().node());

      for (size_t i = 1; i < a_node.m_children.size(); i++) {
        slots = Math::max(slots, 1U + this->raySlots(a_node.m_children[i].node()));
      }

      return slots;
    }
    default: {
      return 1U;
    }
    }
  }

  /**
   * @brief Append an intersection's ray clauses, gathered per operand: those of the operands traced from
   * their spans first, then those of the operands marched (a Marched clause, or its complement) or holding a
   * mesh's deferred level set (rewritten into a Marched clause with bit 3), each traced only within the spans of
   * the operands before it (see TapeDetail::RayOp::Marched).
   * @details Moving a marched operand, which holds one span list, behind the others needs no more span lists
   * at once than their order as written.
   * @param[in] a_state    Sub-program state.
   * @param[in] a_operands Each operand's ray clauses, in order.
   * @param[in] a_ray      Whether all the intersection's spans are needed.
   */
  void
  orderIntersection(const State& a_state, std::vector<std::vector<RayClause>>& a_operands, const RayEmit& a_ray)
  {
    // A marched operand: one Marched clause, alone (it needs only the intersection's first crossing when the
    // intersection does) or under a complement (all its spans).
    const auto marchedClause = [](const std::vector<RayClause>& a_clauses) {
      return (a_clauses.size() == 1U && a_clauses[0].m_op == RayOp::Marched) ||
             (a_clauses.size() == 2U && a_clauses[0].m_op == RayOp::Marched && a_clauses[1].m_op == RayOp::Complement);
    };

    // A deferred operand: a Leaf clause of a mesh's level set, alone or under one affine map, alone or under a
    // complement. Its position among the clauses, or their count for none.
    const auto deferredLeaf = [](const std::vector<RayClause>& a_clauses) -> size_t {
      const size_t n    = a_clauses.size();
      const size_t core = (n >= 2U && a_clauses[n - 1].m_op == RayOp::Complement) ? n - 1 : n;

      const auto meshLevel = [](const RayClause& a_c) {
        return a_c.m_op == RayOp::Leaf && (a_c.m_flags & 8U) != 0U && hasMeshLevel<0>(a_c.m_type);
      };

      if (core == 1U && meshLevel(a_clauses[0])) {
        return 0U;
      }

      if (core == 3U && a_clauses[0].m_op == RayOp::PushAffine && meshLevel(a_clauses[1]) &&
          a_clauses[2].m_op == RayOp::PopAffine) {
        return 1U;
      }

      return n;
    };

    const auto marched = [&](const std::vector<RayClause>& a_clauses) {
      return marchedClause(a_clauses) || deferredLeaf(a_clauses) < a_clauses.size();
    };

    std::stable_partition(a_operands.begin(), a_operands.end(), [&](const std::vector<RayClause>& a_clauses) {
      return !marched(a_clauses);
    });

    std::vector<RayClause>& program = m_staging.m_rayPrograms[a_state.m_program];

    for (size_t i = 0; i < a_operands.size(); i++) {
      std::vector<RayClause>& clauses = a_operands[i];

      if (i > 0 && marchedClause(clauses)) {
        clauses[0].m_flags |= 2U;

        // Only where the intersection itself may stop at its first crossing; rayEvaluate() also checks that
        // the call asks for the first crossing only, since a tape is asked for all its spans too.
        if (clauses.size() == 1U && !a_ray.m_all) {
          clauses[0].m_flags |= 4U;
        }
      }
      else if (i > 0) {
        const size_t leaf = deferredLeaf(clauses);

        if (leaf < clauses.size()) {
          // One Marched clause holding the level set (bit 3, see TapeDetail::RayOp::Marched): the level, then the
          // affine map it is traced through, if any; the leaf number in m_c. The Leaf's own level and the map's
          // parameters stay where they are, unused: the parameter array is only appended to.
          const RayClause& level        = clauses[leaf];
          const bool       complemented = clauses.back().m_op == RayOp::Complement;

          std::vector<T> block{m_staging.m_params[level.m_c]};

          if (leaf == 1U) {
            const auto map = m_staging.m_params.begin() + static_cast<std::ptrdiff_t>(clauses[0].m_a);

            block.insert(block.end(), map, map + 12);
          }

          RayClause deferred = level;

          deferred.m_op = RayOp::Marched;
          deferred.m_b  = this->params(block);
          deferred.m_c  = level.m_b;

          deferred.m_flags |= (leaf == 1U) ? (2U | 32U) : 2U;

          if (!complemented && !a_ray.m_all) {
            deferred.m_flags |= 4U;
          }

          std::vector<RayClause> rewritten{deferred};

          if (complemented) {
            rewritten.push_back(clauses.back());
          }

          clauses = rewritten;
        }
      }

      program.insert(program.end(), clauses.begin(), clauses.end());

      if (i > 0) {
        this->pushRay(a_state, RayOp::Intersection, a_ray);
      }
    }
  }

  /**
   * @brief Append a clause to the current sub-program's ray program.
   * @param[in] a_state Sub-program state.
   * @param[in] a_op    Operation.
   * @param[in] a_ray   Whether all spans are needed.
   * @param[in] a_a     First operand.
   * @param[in] a_b     Second operand.
   * @param[in] a_c     Third operand.
   * @param[in] a_type  Primitive type.
   */
  void
  pushRay(const State&   a_state,
          const RayOp    a_op,
          const RayEmit& a_ray,
          const uint32_t a_a    = 0U,
          const uint32_t a_b    = 0U,
          const uint32_t a_c    = 0U,
          const uint16_t a_type = 0U)
  {
    RayClause c;

    c.m_op    = a_op;
    c.m_flags = a_ray.m_all ? 1U : 0U;
    c.m_type  = a_type;
    c.m_a     = a_a;
    c.m_b     = a_b;
    c.m_c     = a_c;

    m_staging.m_rayPrograms[a_state.m_program].push_back(c);
  }

  /**
   * @brief Emit the clauses that push an expression's value at the top point, and, for a ray program, the
   * ray clauses that push its spans along the top ray.
   * @details A node a ray program does not trace (see traced()), or that would hold more span lists than
   * Tape::RayStackSize, is emitted as usual and marched by the ray program over the clauses it emitted.
   * @param[in,out] a_state Sub-program state.
   * @param[in]     a_node  Expression.
   * @param[in]     a_ray   For the ray program, or null.
   */
  void
  emit(State& a_state, const ExprDetail::Node<T>& a_node, const RayEmit* a_ray)
  {
    using ExprDetail::Kind;

    // In a region's program, a subtree emitted again keeps its leaves' numbers. The subtrees are emitted in the
    // order they were listed, so each start is used once, in order, even for a subexpression listed twice.
    for (auto start = m_leafStarts.begin(); start != m_leafStarts.end(); ++start) {
      if (start->first == &a_node) {
        m_nextLeaf = start->second;

        m_leafStarts.erase(start);

        break;
      }
    }

    if (a_ray != nullptr && (!traced(a_node) || a_ray->m_level + this->raySlots(a_node) > m_rayStackSize)) {
      const auto begin = static_cast<uint32_t>(m_staging.m_programs[a_state.m_program].size());

      this->emit(a_state, a_node, nullptr);

      const auto end = static_cast<uint32_t>(m_staging.m_programs[a_state.m_program].size());

      const BoundingVolumes::AABBT<T>& box = a_node.m_boundingVolume;

      const uint32_t p = this->params({box.getLowCorner()[0],
                                       box.getLowCorner()[1],
                                       box.getLowCorner()[2],
                                       box.getHighCorner()[0],
                                       box.getHighCorner()[1],
                                       box.getHighCorner()[2]});

      m_staging.m_marched.emplace_back(a_state.m_program, begin, end);

      this->pushRay(a_state,
                    RayOp::Marched,
                    *a_ray,
                    static_cast<uint32_t>(m_staging.m_marched.size() - 1),
                    p,
                    static_cast<uint32_t>(a_node.m_quality));

      return;
    }

    switch (a_node.m_kind) {
    case Kind::Primitive: {
      const ExprDetail::Leaf& leaf = a_node.m_leaf;
      const uint32_t          type = findType<0>(leaf.m_type);

      EBGEOMETRY_REQUIRE(type < NumTypes,
                         "Tape: a shape's type is not one of the tape's primitive types; list it in Tape<T, ...>'s "
                         "user primitive types (%s)",
                         leaf.m_type.name());
      EBGEOMETRY_REQUIRE(leaf.m_isAttachedTo == nullptr || leaf.m_isAttachedTo(leaf.m_object.get(), m_pool),
                         "Tape: a pool-resident shape (a mesh SDF, a BVH union or another tape) lives in another "
                         "Pool; it must be built in the tape's own Pool (%s)",
                         leaf.m_type.name());

      const uint32_t index = this->stage<0>(type, leaf.m_object.get());

      // The leaf's number, in expression order (the compiler visits the expression depth first, the
      // operands of a BVH union and the sub-programs too), in the value clause's otherwise unused m_b, and
      // in the ray Leaf clause's m_b: the number closestLeaf() reports, which a hit reports as its
      // primitive (the spans name a leaf by its ray clause's position instead).
      if (a_ray != nullptr) {
        // At a level set: the level in params[m_c], flagged in m_flags (see TapeDetail::RayOp::Leaf).
        const uint32_t level = a_ray->m_atLevel ? this->params({a_ray->m_amount}) : 0U;

        this->pushRay(a_state, RayOp::Leaf, *a_ray, index, m_nextLeaf, level, static_cast<uint16_t>(type));

        if (a_ray->m_atLevel) {
          m_staging.m_rayPrograms[a_state.m_program].back().m_flags |= a_ray->m_annulus ? (8U | 16U) : 8U;
        }
      }

      this->push(a_state, clause(Op::Primitive, index, m_nextLeaf++, 0U, 0U, static_cast<uint16_t>(type)), 1, 0);

      break;
    }
    case Kind::Affine: {
      const CSGOps::AffineCoefficients<T> m = CSGOps::affineCoefficients(a_node.m_map);

      const uint32_t p = this->params(std::vector<T>(m.m_values, m.m_values + 12));

      this->push(a_state, clause(Op::PushAffine, p), 0, 1);

      if (a_ray != nullptr) {
        this->pushRay(a_state, RayOp::PushAffine, *a_ray, p);
      }

      // A level set of the map's value is its operand's at the level divided by the value scale.
      RayEmit operand = (a_ray != nullptr) ? *a_ray : RayEmit{false, 0U};

      operand.m_amount = operand.m_amount / a_node.m_valueScale;

      this->emit(a_state, a_node.m_children.front().node(), (a_ray != nullptr) ? &operand : nullptr);

      if (a_ray != nullptr) {
        this->pushRay(a_state, RayOp::PopAffine, *a_ray, p);
      }

      this->push(a_state, clause(Op::PopPoint), 0, -1);

      if (a_node.m_valueScale != T(1)) {
        this->push(a_state, clause(Op::MulConstant, this->params({a_node.m_valueScale})), 0, 0);
      }

      break;
    }
    case Kind::Elongate: {
      const uint32_t p = this->params({a_node.m_vec0[0], a_node.m_vec0[1], a_node.m_vec0[2]});

      this->push(a_state, clause(Op::PushElongate, p), 0, 1);
      this->emit(a_state, a_node.m_children.front().node(), nullptr);
      this->push(a_state, clause(Op::PopPoint), 0, -1);

      break;
    }
    case Kind::Repeat: {
      // The copies are evaluated as a sub-program, one call per copy the search reaches.
      const Expr<T>&                   child = a_node.m_children.front();
      const BoundingVolumes::AABBT<T>& box   = child.computeBoundingVolume();

      const uint32_t p = this->params({a_node.m_vec0[0],
                                       a_node.m_vec0[1],
                                       a_node.m_vec0[2],
                                       a_node.m_vec1[0],
                                       a_node.m_vec1[1],
                                       a_node.m_vec1[2],
                                       a_node.m_vec2[0],
                                       a_node.m_vec2[1],
                                       a_node.m_vec2[2],
                                       box.getLowCorner()[0],
                                       box.getLowCorner()[1],
                                       box.getLowCorner()[2],
                                       box.getHighCorner()[0],
                                       box.getHighCorner()[1],
                                       box.getHighCorner()[2]});

      const uint32_t sub = this->compileProgram(child.node(), nullptr);

      this->calls(a_state, sub);
      this->push(a_state, clause(Op::Repeat, sub, p), 1, 0);

      break;
    }
    case Kind::Complement: {
      // A complement needs all its operand's spans: past its first crossing, the ray may be outside again.
      if (a_ray != nullptr) {
        const RayEmit all{true, a_ray->m_level};

        this->emit(a_state, a_node.m_children.front().node(), &all);
        this->pushRay(a_state, RayOp::Complement, *a_ray);
      }
      else {
        this->emit(a_state, a_node.m_children.front().node(), nullptr);
      }

      this->push(a_state, clause(Op::Negate), 0, 0);

      break;
    }
    case Kind::Offset:
    case Kind::Annular: {
      const bool annulus = a_node.m_kind == Kind::Annular;

      // Traced (see traced()), the operand's primitive is traced at the level set the operation keeps.
      RayEmit level = (a_ray != nullptr) ? *a_ray : RayEmit{false, 0U};

      level.m_atLevel = true;
      level.m_amount  = a_node.m_scalar0;
      level.m_annulus = annulus;

      this->emit(a_state, a_node.m_children.front().node(), (a_ray != nullptr) ? &level : nullptr);

      if (annulus) {
        this->push(a_state, clause(Op::Abs), 0, 0);
      }

      this->push(a_state, clause(Op::AddConstant, this->params({-a_node.m_scalar0})), 0, 0);

      break;
    }
    case Kind::Blur: {
      const CSGOps::BlurWeights<T> w = CSGOps::blurWeights(a_node.m_scalar0, a_node.m_scalar1);

      const uint32_t p = this->params(std::vector<T>(w.m_values, w.m_values + 5));

      const uint32_t sub = this->compileProgram(a_node.m_children.front().node(), nullptr);

      this->calls(a_state, sub);
      this->push(a_state, clause(Op::Blur, sub, p), 1, 0);

      break;
    }
    case Kind::Mollify: {
      // The taps (offset and normalized weight) are stored, so the evaluator only sums.
      const CSGOps::MollifyGrid<T> grid = CSGOps::mollifyGrid(a_node.m_scalar0, a_node.m_count);
      const uint32_t               n    = grid.m_samples;

      std::vector<T> taps;

      taps.reserve(4U * n * n * n);

      for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
          for (uint32_t k = 0; k < n; k++) {
            const Vec3T<T> o = CSGOps::mollifyOffset(grid.m_radius, grid.m_spacing, i, j, k);

            taps.insert(taps.end(), {o[0], o[1], o[2], CSGOps::mollifyWeight(o, grid.m_radius) / grid.m_sum});
          }
        }
      }

      const uint32_t p   = this->params(taps);
      const uint32_t sub = this->compileProgram(a_node.m_children.front().node(), nullptr);

      this->calls(a_state, sub);
      this->push(a_state, clause(Op::Mollify, sub, p, n * n * n), 1, 0);

      break;
    }
    case Kind::Union:
    case Kind::Intersection: {
      if (a_node.m_kind == Kind::Union && this->useBVH(a_node)) {
        // The operands' sub-programs, with those they call, are numbered from first up to end.
        const auto     first = static_cast<uint32_t>(m_staging.m_programs.size());
        const uint32_t bvh   = this->buildUnion(a_state, a_node, a_ray);
        const auto     end   = static_cast<uint32_t>(m_staging.m_programs.size());

        if (a_ray != nullptr) {
          this->pushRay(a_state, RayOp::UnionBVH, *a_ray, bvh, first, end);
        }

        this->push(a_state, clause(Op::UnionBVH, bvh), 1, 0);
      }
      else {
        // An Overlay takes no BVH, so it is not counted: the warning's advice would not apply.
        if (a_node.m_kind == Kind::Union && !a_node.m_overlay) {
          m_largestPlainUnion = Math::max(m_largestPlainUnion, this->plainUnionSize(a_node));
        }

        const Op      op    = (a_node.m_kind == Kind::Union) ? Op::Min : Op::Max;
        const RayOp   rayOp = (a_node.m_kind == Kind::Union) ? RayOp::Union : RayOp::Intersection;
        const bool    all   = (a_ray != nullptr) && (a_ray->m_all || a_node.m_kind == Kind::Intersection);
        const RayEmit first = {all, (a_ray != nullptr) ? a_ray->m_level : 0U};
        const RayEmit next  = {all, first.m_level + 1U};

        // An intersection needs all its operands' spans: where one ends, the ray may still be inside the
        // others. Its ray clauses are gathered per operand, to be put in order once all are emitted.
        const bool                          gather = (a_ray != nullptr) && a_node.m_kind == Kind::Intersection;
        std::vector<std::vector<RayClause>> operands;

        const auto emitOperand = [&](const ExprDetail::Node<T>& a_operand, const RayEmit* a_emit) {
          std::vector<RayClause>& program = m_staging.m_rayPrograms[a_state.m_program];
          const size_t            begin   = program.size();

          this->emit(a_state, a_operand, a_emit);

          if (gather) {
            // Emitting may have added sub-programs, and moved the ray programs.
            std::vector<RayClause>& emitted = m_staging.m_rayPrograms[a_state.m_program];

            operands.emplace_back(emitted.begin() + static_cast<std::ptrdiff_t>(begin), emitted.end());
            emitted.resize(begin);
          }
        };

        emitOperand(a_node.m_children.front().node(), (a_ray != nullptr) ? &first : nullptr);

        for (size_t i = 1; i < a_node.m_children.size(); i++) {
          emitOperand(a_node.m_children[i].node(), (a_ray != nullptr) ? &next : nullptr);

          if (a_ray != nullptr && !gather) {
            this->pushRay(a_state, rayOp, *a_ray);
          }

          this->push(a_state, clause(op), -1, 0);
        }

        if (gather) {
          this->orderIntersection(a_state, operands, *a_ray);
        }
      }

      break;
    }
    case Kind::SmoothUnion:
    case Kind::SmoothIntersection: {
      const bool    isUnion     = (a_node.m_kind == Kind::SmoothUnion);
      const bool    exponential = (a_node.m_blend == BlendKind::Exponential);
      const uint8_t maxFlag     = isUnion ? 0U : 1U;

      if (isUnion && this->useBVH(a_node)) {
        const uint32_t s   = this->params({a_node.m_scalar0});
        const uint32_t bvh = this->buildUnion(a_state, a_node, nullptr);

        this->push(a_state, clause(Op::SmoothUnionBVH, bvh, s, 0U, exponential ? 1U : 0U), 1, 0);
      }
      else {
        if (isUnion) {
          m_largestPlainUnion = Math::max(m_largestPlainUnion, this->plainUnionSize(a_node));
        }

        // Only the two extreme values are blended, so they are tracked as the operands are evaluated.
        this->push(a_state, clause(Op::PairInit, 0U, 0U, 0U, maxFlag), 2, 0);

        for (const auto& child : a_node.m_children) {
          this->emit(a_state, child.node(), nullptr);
          this->push(a_state, clause(Op::PairInsert, 0U, 0U, 0U, maxFlag), -1, 0);
        }

        const uint8_t flags = static_cast<uint8_t>(maxFlag | (exponential ? 2U : 0U));

        this->push(a_state, clause(Op::PairBlend, this->params({a_node.m_scalar0}), 0U, 0U, flags), -1, 0);
      }

      break;
    }
    default: {
      EBGEOMETRY_EXPECT(false && "Tape: unknown expression kind");

      break;
    }
    }
  }
};

/**
 * @brief Reserve each primitive type's array in a pool, sized to the staged primitives.
 * @details Internal helper; not part of the public API.
 * @tparam I     Index of the store's first type among all the types.
 * @tparam Store PrimitiveStore type (the remaining types).
 * @tparam Tuple Staged primitives, one vector per type.
 * @param[in,out] a_pool   Pool.
 * @param[in,out] a_store  Store.
 * @param[in]     a_staged Staged primitives.
 */
template <size_t I, class Store, class Tuple>
inline void
reserveStore(Pool& a_pool, Store& a_store, const Tuple& a_staged)
{
  if constexpr (!std::is_same_v<Store, PrimitiveStore<>>) {
    a_store.m_head.reserveFrom(a_pool, static_cast<uint32_t>(std::get<I>(a_staged).size()));

    reserveStore<I + 1>(a_pool, a_store.m_tail, a_staged);
  }
}

/**
 * @brief Copy the staged primitives into each type's reserved array.
 * @details Internal helper; not part of the public API.
 * @tparam I     Index of the store's first type among all the types.
 * @tparam Store PrimitiveStore type (the remaining types).
 * @tparam Tuple Staged primitives, one vector per type.
 * @param[in]     a_base   Pool base.
 * @param[in,out] a_store  Store.
 * @param[in]     a_staged Staged primitives.
 */
template <size_t I, class Store, class Tuple>
inline void
assignStore(void* a_base, Store& a_store, const Tuple& a_staged)
{
  if constexpr (!std::is_same_v<Store, PrimitiveStore<>>) {
    const auto& staged = std::get<I>(a_staged);

    a_store.m_head.assign(a_base, staged.data(), static_cast<uint32_t>(staged.size()));

    assignStore<I + 1>(a_base, a_store.m_tail, a_staged);
  }
}

/**
 * @brief The largest end byte over a primitive store's arrays.
 * @details Internal helper; not part of the public API.
 * @tparam Store PrimitiveStore type.
 * @param[in] a_store Store.
 * @return The end byte.
 */
template <class Store>
[[nodiscard]] inline uint64_t
storeEndByte(const Store& a_store) noexcept
{
  if constexpr (std::is_same_v<Store, PrimitiveStore<>>) {
    return 0U;
  }
  else {
    return Math::max(a_store.m_head.endByte(), storeEndByte(a_store.m_tail));
  }
}

/**
 * @brief How many leaves an expression has, each occurrence counted (as the compiler numbers them).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_node Expression.
 * @return The count.
 */
template <class T>
[[nodiscard]] inline uint32_t
countLeaves(const ExprDetail::Node<T>& a_node) noexcept
{
  if (a_node.m_kind == ExprDetail::Kind::Primitive) {
    return 1U;
  }

  uint32_t count = 0;

  for (const Expr<T>& child : a_node.m_children) {
    count += countLeaves(child.node());
  }

  return count;
}

/**
 * @brief A region's solids: its operand under the maps and masks above it, once with every
 * mask (level 0) and once more for each outer mask left out (level l without the l outermost), with the first leaf
 * of the operand and of each mask in the whole expression.
 * @details Internal helper; not part of the public API. A mask above two regions removes the same space from both,
 * so region k's function applies it once, after subtracting the later region j: max(S_k, M, -S_j), never max(S_k,
 * M, -max(S_j, M)), which is zero on the mask's face inside S_j though nothing is there (a sheet the region's surface
 * does not have). For each later region the function therefore uses its solid without the masks the two share.
 * @tparam T Floating-point precision.
 */
template <class T>
struct RegionSolid
{
  /**
   * @brief The solid at each level, level 0 with every mask.
   */
  std::vector<Expr<T>> m_levels;

  /**
   * @brief The first leaf of each subtree each level holds from the whole expression.
   */
  std::vector<std::vector<std::pair<const ExprDetail::Node<T>*, uint32_t>>> m_starts;

  /**
   * @brief The masks above it (the clipping intersections), outermost first, each with its first leaf in the whole
   * expression: an occurrence, not a node, since a clipped subexpression used twice (under different maps) clips each
   * copy with a mask of its own.
   */
  std::vector<std::pair<const ExprDetail::Node<T>*, uint32_t>> m_masks;
};

/**
 * @brief An operand without its material labels: a region's solid is clipped by masks like any solid, and a
 * material there would be refused.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_operand Operand.
 * @return The operand, or a copy without material labels along its maps.
 */
template <class T>
[[nodiscard]] inline Expr<T>
withoutMaterial(const Expr<T>& a_operand)
{
  if (!a_operand.node().m_namesRegion) {
    return a_operand;
  }

  auto node = std::make_shared<ExprDetail::Node<T>>(a_operand.node());

  node->m_materialLabel.clear();
  node->m_namesRegion = false;

  if (node->m_kind == ExprDetail::Kind::Affine) {
    node->m_children.front() = withoutMaterial(node->m_children.front());
  }

  return Expr<T>(std::move(node));
}

/**
 * @brief The solids of the regions of a partition, in region order.
 * @details Internal helper; not part of the public API. Walks the partition depth first, as ExprIndex numbers the
 * regions: an Overlay's operands in order; a map or a clipping intersection wraps the solids below it in itself.
 * @tparam T Floating-point precision.
 * @param[in]     a_expr      A partition, or an operand of one.
 * @param[in]     a_firstLeaf Its first leaf in the whole expression.
 * @param[in]     a_path      The maps and clipping intersections above it, outermost first, with their first leaves.
 * @param[in,out] a_solids    The solids found so far.
 */
template <class T>
inline void
regionSolids(const Expr<T>&                                                a_expr,
             const uint32_t                                                a_firstLeaf,
             std::vector<std::pair<const ExprDetail::Node<T>*, uint32_t>>& a_path,
             std::vector<RegionSolid<T>>&                                  a_solids)
{
  using ExprDetail::Kind;

  const ExprDetail::Node<T>& node = a_expr.node();

  if (!node.m_partition) {
    RegionSolid<T> solid;

    for (const auto& step : a_path) {
      if (step.first->m_kind == Kind::Intersection) {
        solid.m_masks.push_back(step);
      }
    }

    const Expr<T> operand = withoutMaterial(a_expr);

    // Level l leaves out the l outermost masks.
    for (size_t level = 0; level <= solid.m_masks.size(); level++) {
      Expr<T> expr = operand;

      std::vector<std::pair<const ExprDetail::Node<T>*, uint32_t>> starts{{&operand.node(), a_firstLeaf}};

      // The masks left out are the outermost: counting from the innermost, the first (masks - level) are kept.
      size_t kept = solid.m_masks.size() - level;

      // Wrap in the maps and masks above, innermost first.
      for (auto step = a_path.rbegin(); step != a_path.rend(); ++step) {
        const ExprDetail::Node<T>& above = *step->first;

        if (above.m_kind == Kind::Affine) {
          auto wrapped = ExprDetail::makeNode<T>(Kind::Affine, {expr});

          wrapped->m_map            = above.m_map;
          wrapped->m_inverse        = above.m_inverse;
          wrapped->m_valueScale     = above.m_valueScale;
          wrapped->m_boundingVolume = above.m_inverse.transform(expr.computeBoundingVolume());
          wrapped->m_quality        = expr.getDistanceQuality();

          expr = Expr<T>(std::move(wrapped));
        }
        else if (kept > 0) {
          kept--;

          std::vector<Expr<T>> operands{expr};

          uint32_t leaf = step->second + countLeaves(above.m_children.front().node());

          for (size_t i = 1; i < above.m_children.size(); i++) {
            operands.push_back(above.m_children[i]);

            starts.emplace_back(&above.m_children[i].node(), leaf);

            leaf += countLeaves(above.m_children[i].node());
          }

          auto wrapped = ExprDetail::makeNode<T>(Kind::Intersection, std::move(operands));

          wrapped->m_boundingVolume = above.m_boundingVolume;
          wrapped->m_quality        = above.m_quality;

          expr = Expr<T>(std::move(wrapped));
        }
      }

      solid.m_levels.push_back(expr);
      solid.m_starts.push_back(std::move(starts));
    }

    a_solids.push_back(std::move(solid));

    return;
  }

  if (node.m_overlay) {
    uint32_t leaf = a_firstLeaf;

    for (const Expr<T>& child : node.m_children) {
      regionSolids(child, leaf, a_path, a_solids);

      leaf += countLeaves(child.node());
    }
  }
  else {
    a_path.emplace_back(&node, a_firstLeaf);

    regionSolids(node.m_children.front(), a_firstLeaf, a_path, a_solids);

    a_path.pop_back();
  }
}

} // namespace TapeDetail

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST
Tape<T, UserPrimitives...>::Tape(Pool& a_pool, const Expr<T>& a_expr, const TapeOptions& a_options) : PoolHandle()
{
  TapeDetail::Staging<T, BranchingRatio, Primitives> staging;

  TapeDetail::Compiler<T, BranchingRatio, Primitives, Features> compiler(
    a_pool, a_options, ValueStackSize, PointStackSize, RayStackSize, staging);

  // The whole object is traced for its first crossing; its intersections and complements ask for more.
  using RayEmit = typename TapeDetail::Compiler<T, BranchingRatio, Primitives, Features>::RayEmit;

  const RayEmit ray{false, 0U};

  [[maybe_unused]] const uint32_t root = compiler.compileProgram(a_expr.node(), &ray);

  EBGEOMETRY_EXPECT(root == 0U);

  EBGEOMETRY_REQUIRE(staging.m_depth.front() <= MaxCallDepth,
                     "Tape: Blur, Mollify, FiniteRepetition and BVH unions nest %u deep, but a tape allows at most %u",
                     unsigned(staging.m_depth.front()),
                     unsigned(MaxCallDepth));

  staging.m_boundingVolume    = a_expr.computeBoundingVolume();
  staging.m_quality           = a_expr.getDistanceQuality();
  staging.m_numPrimitives     = compiler.numPrimitives();
  staging.m_largestPlainUnion = compiler.largestPlainUnion();

  // Each region's program: its operand, under the maps and masks above it, with the leaves numbered as in the
  // whole expression.
  //
  // Without an Overlay the object is one region in the background, its material the label on it (or on a map above
  // it), if any: the table is written directly, without numbering the whole expression (a large union's index would
  // cost as much as the tape), and only when there is a material: an empty table is one region with none (the
  // queries read it so), and a plain tape's pool keeps its size.
  if (!a_expr.node().m_partition) {
    std::vector<uint32_t>& table = staging.m_regionPrograms;

    uint32_t material = MaterialLabelId::none().value();

    for (const ExprDetail::Node<T>* node = &a_expr.node();; node = &node->m_children.front().node()) {
      if (!node->m_materialLabel.empty()) {
        material = 0U;

        break;
      }

      if (node->m_kind != ExprDetail::Kind::Affine || node->m_children.empty()) {
        break;
      }
    }

    if (material != MaterialLabelId::none().value()) {
      table = {0U, MaterialLabelId::none().value(), material};
    }
  }
  else {
    std::vector<TapeDetail::RegionSolid<T>>                      solids;
    std::vector<std::pair<const ExprDetail::Node<T>*, uint32_t>> path;

    TapeDetail::regionSolids(a_expr, 0U, path, solids);

    // The table: the number of operands n, each operand's record's offset, each region's material (the background
    // first; two for an object with no Overlay), then the records: the number of masks above the region, their
    // numbers (outermost first), and its programs, level 0 (every mask) to the last.
    std::vector<uint32_t>&                                       table = staging.m_regionPrograms;
    std::vector<std::pair<const ExprDetail::Node<T>*, uint32_t>> masks;

    const ExprIndex<T> regions(a_expr);

    table.assign(1U + solids.size(), 0U);

    table[0] = static_cast<uint32_t>(solids.size());

    for (uint32_t k = 0; k < regions.getNumRegions(); k++) {
      table.push_back(regions.getMaterialOf(k).value());
    }

    for (size_t k = 0; k < solids.size(); k++) {
      const TapeDetail::RegionSolid<T>& solid = solids[k];

      table[1U + k] = static_cast<uint32_t>(table.size());

      table.push_back(static_cast<uint32_t>(solid.m_masks.size()));

      for (const auto& mask : solid.m_masks) {
        const auto found = std::find(masks.begin(), masks.end(), mask);

        table.push_back(static_cast<uint32_t>(found - masks.begin()));

        if (found == masks.end()) {
          masks.push_back(mask);
        }
      }

      for (size_t level = 0; level < solid.m_levels.size(); level++) {
        table.push_back(compiler.compileRegion(solid.m_levels[level].node(), solid.m_starts[level]));
      }
    }
  }

  // The labels and the surfaces, numbered as ExprIndex numbers them; a tape without labels stores none, one
  // without a container leaf no surfaces, and one with neither does not build the index (which allocates per
  // node).
  const bool labeled    = ExprDetail::hasSurfaceLabel(a_expr.node());
  const bool containers = ExprDetail::hasContainer(a_expr.node());

  if (labeled || containers) {
    const ExprIndex<T> index(a_expr);

    EBGEOMETRY_EXPECT(index.getNumLeaves() == staging.m_numPrimitives);

    if (labeled) {
      staging.m_numLabels = index.getNumSurfaceLabels();
      staging.m_surfaceLabels.resize(index.getNumLeaves());

      for (uint32_t leaf = 0; leaf < index.getNumLeaves(); leaf++) {
        staging.m_surfaceLabels[leaf] = index.getSurfaceLabel(leaf).value();
      }
    }

    if (containers) {
      staging.m_surfaces.resize(index.getNumLeaves() + 1);

      for (uint32_t leaf = 0; leaf <= index.getNumLeaves(); leaf++) {
        staging.m_surfaces[leaf] = index.getFirstSurface(leaf);
      }
    }

    // The part labels, only when some leaf's parts are named: per leaf where its labels begin, then the labels.
    bool partsNamed = false;

    for (uint32_t leaf = 0; leaf < index.getNumLeaves(); leaf++) {
      partsNamed = partsNamed || !index.getPartSurfaceLabels(leaf).empty();
    }

    if (partsNamed) {
      std::vector<uint32_t>& table = staging.m_partSurfaceLabels;

      table.assign(index.getNumLeaves(), SurfaceRef<T>::None);

      for (uint32_t leaf = 0; leaf < index.getNumLeaves(); leaf++) {
        const std::vector<SurfaceLabelId>& parts = index.getPartSurfaceLabels(leaf);

        if (!parts.empty()) {
          table[leaf] = static_cast<uint32_t>(table.size());

          for (const SurfaceLabelId part : parts) {
            table.push_back(part.value());
          }
        }
      }
    }
  }

  if (a_options.m_largeUnionWarning > 0U && staging.m_largestPlainUnion >= a_options.m_largeUnionWarning) {
    std::fprintf(stderr,
                 "EBGeometry::Tape: a union of %u objects is evaluated as written; Union(objects, "
                 "Accelerate::BVH) evaluates it through a BVH (TapeOptions::m_largeUnionWarning silences this)\n",
                 unsigned(staging.m_largestPlainUnion));
  }

  TapeDetail::flatten(staging);

  this->finalize(a_pool, staging);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST
void
Tape<T, UserPrimitives...>::finalize(Pool& a_pool, const TapeDetail::Staging<T, BranchingRatio, Primitives>& a_staging)
{
  // Reserve everything first: a reserve may move the pool's block, so the base is resolved only once
  // nothing more is reserved.
  const std::vector<TapeDetail::Clause>&  clauses  = a_staging.m_flatClauses;
  const std::vector<TapeDetail::Program>& programs = a_staging.m_flatPrograms;

  Layout layout;

  layout.m_clauses.reserveFrom(a_pool, static_cast<uint32_t>(clauses.size()));
  layout.m_programs.reserveFrom(a_pool, static_cast<uint32_t>(programs.size()));
  layout.m_params.reserveFrom(a_pool, static_cast<uint32_t>(a_staging.m_params.size()));
  layout.m_unions.reserveFrom(a_pool, static_cast<uint32_t>(a_staging.m_unions.size()));
  layout.m_surfaceLabels.reserveFrom(a_pool, static_cast<uint32_t>(a_staging.m_surfaceLabels.size()));
  layout.m_surfaces.reserveFrom(a_pool, static_cast<uint32_t>(a_staging.m_surfaces.size()));
  layout.m_partSurfaceLabels.reserveFrom(a_pool, static_cast<uint32_t>(a_staging.m_partSurfaceLabels.size()));
  layout.m_regionPrograms.reserveFrom(a_pool, static_cast<uint32_t>(a_staging.m_regionPrograms.size()));

  TapeDetail::reserveStore<0>(a_pool, layout.m_primitives, a_staging.m_primitives);

  this->attach(a_pool, "Tape");

  void* base = this->base();

  layout.m_clauses.assign(base, clauses.data(), static_cast<uint32_t>(clauses.size()));
  layout.m_programs.assign(base, programs.data(), static_cast<uint32_t>(programs.size()));
  layout.m_params.assign(base, a_staging.m_params.data(), static_cast<uint32_t>(a_staging.m_params.size()));
  layout.m_unions.assign(base, a_staging.m_unions.data(), static_cast<uint32_t>(a_staging.m_unions.size()));
  layout.m_surfaceLabels.assign(
    base, a_staging.m_surfaceLabels.data(), static_cast<uint32_t>(a_staging.m_surfaceLabels.size()));
  layout.m_surfaces.assign(base, a_staging.m_surfaces.data(), static_cast<uint32_t>(a_staging.m_surfaces.size()));
  layout.m_partSurfaceLabels.assign(
    base, a_staging.m_partSurfaceLabels.data(), static_cast<uint32_t>(a_staging.m_partSurfaceLabels.size()));
  layout.m_regionPrograms.assign(
    base, a_staging.m_regionPrograms.data(), static_cast<uint32_t>(a_staging.m_regionPrograms.size()));

  TapeDetail::assignStore<0>(base, layout.m_primitives, a_staging.m_primitives);

  layout.m_boundingVolume    = a_staging.m_boundingVolume;
  layout.m_quality           = a_staging.m_quality;
  layout.m_callDepth         = a_staging.m_depth.front();
  layout.m_numPrograms       = a_staging.m_flatNumPrograms;
  layout.m_numPrimitives     = a_staging.m_numPrimitives;
  layout.m_largestPlainUnion = a_staging.m_largestPlainUnion;
  layout.m_rayPrograms       = a_staging.m_flatRayPrograms;
  layout.m_numLabels         = a_staging.m_numLabels;

  m_layout = layout;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline T
Tape<T, UserPrimitives...>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // A tape that calls no sub-program is evaluated in place: at depth 0 the evaluator calls no level below,
  // so inlining it cannot multiply copies (the reason evaluate<D> is out of line), and it saves the call.
  if (m_layout.m_callDepth == 0U) {
    return this->template evaluateBody<0>(0U, a_point, this->base(), this->location());
  }

  return this->template evaluate<MaxCallDepth>(0U, a_point, this->base(), this->location());
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
Tape<T, UserPrimitives...>::firstHit(const RayT<T>&       a_ray,
                                     const T              a_tMin,
                                     const T              a_tMax,
                                     const RayOptions<T>& a_options) const noexcept
{
  return EBGeometry::firstHit(*this, a_ray, a_tMin, a_tMax, a_options);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
Tape<T, UserPrimitives...>::firstHit(const Vec3T<T>&      a_from,
                                     const Vec3T<T>&      a_to,
                                     const T              a_tMin,
                                     const RayOptions<T>& a_options) const noexcept
{
  return EBGeometry::firstHit(*this, a_from, a_to, a_tMin, a_options);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline HitStatus
Tape<T, UserPrimitives...>::intersects(const RayT<T>&       a_ray,
                                       const T              a_tMin,
                                       const T              a_tMax,
                                       const RayOptions<T>& a_options) const noexcept
{
  return EBGeometry::intersects(*this, a_ray, a_tMin, a_tMax, a_options);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline HitStatus
Tape<T, UserPrimitives...>::intersects(const Vec3T<T>&      a_from,
                                       const Vec3T<T>&      a_to,
                                       const T              a_tMin,
                                       const RayOptions<T>& a_options) const noexcept
{
  return EBGeometry::intersects(*this, a_from, a_to, a_tMin, a_options);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline SurfaceHitT<T>
Tape<T, UserPrimitives...>::firstSurface(const RayT<T>&       a_ray,
                                         const T              a_tMin,
                                         const T              a_tMax,
                                         const RayOptions<T>& a_options) const noexcept
{
  return EBGeometry::firstSurface(*this, a_ray, a_tMin, a_tMax, a_options);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline SurfaceHitT<T>
Tape<T, UserPrimitives...>::firstSurface(const Vec3T<T>&      a_from,
                                         const Vec3T<T>&      a_to,
                                         const T              a_tMin,
                                         const RayOptions<T>& a_options) const noexcept
{
  return EBGeometry::firstSurface(*this, a_from, a_to, a_tMin, a_options);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline LeafRef<T>
Tape<T, UserPrimitives...>::closestLeaf(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  // The evaluator with tagged values: each value carries the primitive clause it comes from. This is a
  // separate instantiation, so signedDistance() is compiled as without it.
  using V = CSGOps::Tagged<T>;

  const V value = (m_layout.m_callDepth == 0U)
                    ? this->template evaluateBody<0, V>(0U, a_point, this->base(), this->location())
                    : this->template evaluate<MaxCallDepth, V>(0U, a_point, this->base(), this->location());

  LeafRef<T> ref;

  if (value.m_tag != CSGOps::NoTag) {
    const TapeDetail::Clause& c = m_layout.m_clauses.at(this->base(), value.m_tag);

    ref.m_leaf       = c.m_b;
    ref.m_type       = c.m_type;
    ref.m_index      = c.m_a;
    ref.m_localPoint = value.m_point;
  }

  return ref;
}

template <class T, class... UserPrimitives>
template <class P>
EBGEOMETRY_HOST_DEVICE
inline bool
Tape<T, UserPrimitives...>::isPrimitive(const LeafRef<T>& a_ref) const noexcept
{
  constexpr size_t I = TapeDetail::IndexOf<P, Primitives>::value;

  static_assert(I < Primitives::size, "Tape::isPrimitive: P is not one of the tape's primitive types");

  return a_ref.m_type == I;
}

template <class T, class... UserPrimitives>
template <class P>
EBGEOMETRY_HOST_DEVICE
inline P
Tape<T, UserPrimitives...>::primitive(const LeafRef<T>& a_ref) const noexcept
{
  constexpr size_t I = TapeDetail::IndexOf<P, Primitives>::value;

  static_assert(I < Primitives::size, "Tape::primitive: P is not one of the tape's primitive types");

  EBGEOMETRY_EXPECT(a_ref.m_type == I);

  const auto& stored = TapeDetail::storeAt<I>(m_layout.m_primitives).at(this->base(), a_ref.m_index);

  // A pool-resident primitive is stored as its layout and resolves against the tape's own location.
  if constexpr (isPoolHandle<P>) {
    return P(stored, this->location());
  }
  else {
    return stored;
  }
}

template <class T, class... UserPrimitives>
template <class F>
EBGEOMETRY_HOST_DEVICE
inline void
Tape<T, UserPrimitives...>::visitPrimitive(const LeafRef<T>& a_ref, F&& a_f) const noexcept
{
  EBGEOMETRY_EXPECT(a_ref.m_type < Primitives::size);

  this->template dispatchVisit<0>(a_ref, a_f);
}

template <class T, class... UserPrimitives>
template <size_t I, class F>
EBGEOMETRY_HOST_DEVICE
inline void
Tape<T, UserPrimitives...>::dispatchVisit(const LeafRef<T>& a_ref, F&& a_f) const noexcept
{
  if constexpr (I < Primitives::size) {
    if (a_ref.m_type == I) {
      using P = typename TapeDetail::TypeAt<I, Primitives>::type;

      a_f(this->template primitive<P>(a_ref));

      return;
    }

    this->template dispatchVisit<I + 1>(a_ref, a_f);
  }
}

template <class T, class... UserPrimitives>
template <unsigned D, class V>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline V
Tape<T, UserPrimitives...>::evaluate(const uint32_t      a_program,
                                     const Vec3T<T>&     a_point,
                                     const void*         a_base,
                                     const PoolLocation& a_location) const noexcept
{
  return this->template evaluateBody<D, V>(a_program, a_point, a_base, a_location);
}

template <class T, class... UserPrimitives>
template <unsigned D, class V>
EBGEOMETRY_HOST_DEVICE
inline V
Tape<T, UserPrimitives...>::evaluateBody(const uint32_t      a_program,
                                         const Vec3T<T>&     a_point,
                                         const void*         a_base,
                                         const PoolLocation& a_location) const noexcept
{
  using TapeDetail::Op;

  const TapeDetail::Program range   = m_layout.m_programs.at(a_base, a_program);
  const TapeDetail::Clause* clauses = m_layout.m_clauses.data(a_base);
  const T*                  params  = m_layout.m_params.data(a_base);

  V        values[ValueStackSize];
  Vec3T<T> points[PointStackSize];

  uint32_t nv = 0;
  uint32_t np = 0;

  points[np++] = a_point;

  for (uint32_t pc = range.m_begin; pc < range.m_end; pc++) {
    const TapeDetail::Clause& c = clauses[pc];

    switch (c.m_op) {
    case Op::Primitive: {
      // A tagged value is tagged with the clause, which names the leaf, its type and its index.
      values[nv++] = CSGOps::leafValue<V>(
        this->evaluatePrimitive(c.m_type, c.m_a, points[np - 1], a_base, a_location), pc, points[np - 1]);

      EBGEOMETRY_EXPECT(!std::isnan(CSGOps::valueOf(values[nv - 1])));

      break;
    }
    // The point maps: tracedLeaf() applies them too, and a new one goes there as well.
    case Op::PushAffine: {
      points[np] = CSGOps::affinePoint(params + c.m_a, points[np - 1]);

      np++;

      break;
    }
    case Op::PushElongate: {
      const T* h = params + c.m_a;

      points[np] = CSGOps::elongatePoint(Vec3T<T>(h[0], h[1], h[2]), points[np - 1]);

      np++;

      break;
    }
    case Op::PopPoint: {
      np--;

      break;
    }
    case Op::Negate: {
      values[nv - 1] = CSGOps::negated(values[nv - 1]);

      break;
    }
    case Op::Abs: {
      values[nv - 1] = CSGOps::magnitude(values[nv - 1]);

      break;
    }
    case Op::AddConstant: {
      values[nv - 1] = CSGOps::plus(values[nv - 1], params[c.m_a]);

      break;
    }
    case Op::MulConstant: {
      values[nv - 1] = CSGOps::times(values[nv - 1], params[c.m_a]);

      break;
    }
    case Op::Min: {
      nv--;

      values[nv - 1] = CSGOps::smaller(values[nv - 1], values[nv]);

      break;
    }
    case Op::Max: {
      nv--;

      values[nv - 1] = CSGOps::larger(values[nv - 1], values[nv]);

      break;
    }
    case Op::PairInit: {
      // The pair occupies two value slots: the most extreme value so far, then the next.
      const CSGOps::Pair<V> pair = CSGOps::pairInit<V>(c.m_flags != 0U);

      values[nv++] = pair.m_first;
      values[nv++] = pair.m_second;

      break;
    }
    case Op::PairInsert: {
      const V v = values[--nv];

      CSGOps::Pair<V> pair{values[nv - 2], values[nv - 1]};

      CSGOps::pairInsert(pair, v, c.m_flags != 0U);

      values[nv - 2] = pair.m_first;
      values[nv - 1] = pair.m_second;

      break;
    }
    case Op::PairBlend: {
      const V b = values[--nv];
      const V a = values[nv - 1];

      values[nv - 1] =
        CSGOps::pairBlend(CSGOps::Pair<V>{a, b}, params[c.m_a], (c.m_flags & 1U) != 0U, (c.m_flags & 2U) != 0U);

      break;
    }
    case Op::Blur: {
      if constexpr (D > 0) {
        values[nv++] = this->template evaluateBlur<D, V>(c, points[np - 1], params, a_base, a_location);
      }
      else {
        EBGEOMETRY_EXPECT(false);

        values[nv++] = CSGOps::noLeaf<V>();
      }

      break;
    }
    case Op::Mollify: {
      if constexpr (D > 0) {
        const T*        taps = params + c.m_b;
        const Vec3T<T>& x    = points[np - 1];

        T value = T(0);

        if constexpr (CSGOps::isTagged<V>) {
          // The same sum, tagged with the tap of the largest weight (the first, if several have it).
          V heaviest       = CSGOps::noLeaf<V>();
          T heaviestWeight = -T(1);

          for (uint32_t k = 0; k < c.m_c; k++) {
            const T* tap = taps + 4U * k;
            const V  f =
              this->template evaluate<D - 1, V>(c.m_a, x - Vec3T<T>(tap[0], tap[1], tap[2]), a_base, a_location);

            value += f.m_value * tap[3];

            if (tap[3] > heaviestWeight) {
              heaviest       = f;
              heaviestWeight = tap[3];
            }
          }

          values[nv++] = CSGOps::withValue(heaviest, value);
        }
        else {
          for (uint32_t k = 0; k < c.m_c; k++) {
            const T* tap = taps + 4U * k;

            value +=
              this->template evaluate<D - 1>(c.m_a, x - Vec3T<T>(tap[0], tap[1], tap[2]), a_base, a_location) * tap[3];
          }

          values[nv++] = value;
        }
      }
      else {
        EBGEOMETRY_EXPECT(false);

        values[nv++] = CSGOps::noLeaf<V>();
      }

      break;
    }
    case Op::Repeat: {
      if constexpr (D > 0) {
        values[nv++] = this->template evaluateRepeat<D, V>(c, points[np - 1], params, a_base, a_location);
      }
      else {
        EBGEOMETRY_EXPECT(false);

        values[nv++] = CSGOps::noLeaf<V>();
      }

      break;
    }
    case Op::UnionBVH:
    case Op::SmoothUnionBVH: {
      if constexpr (D > 0) {
        values[nv++] = this->template evaluateUnion<D, V>(c, points[np - 1], params, a_base, a_location);
      }
      else {
        EBGEOMETRY_EXPECT(false);

        values[nv++] = CSGOps::noLeaf<V>();
      }

      break;
    }
    default: {
      EBGEOMETRY_EXPECT(false && "Tape: unknown clause");

      break;
    }
    }
  }

  EBGEOMETRY_EXPECT(nv == 1U && np == 1U);

  return values[0];
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline T
Tape<T, UserPrimitives...>::evaluatePrimitive(const uint32_t      a_type,
                                              const uint32_t      a_index,
                                              const Vec3T<T>&     a_point,
                                              const void*         a_base,
                                              const PoolLocation& a_location) const noexcept
{
  return this->template dispatchPrimitive<0, false>(a_type, a_index, a_point, a_base, a_location);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline T
Tape<T, UserPrimitives...>::evaluateHandle(const uint32_t      a_type,
                                           const uint32_t      a_index,
                                           const Vec3T<T>&     a_point,
                                           const void*         a_base,
                                           const PoolLocation& a_location) const noexcept
{
  return this->template dispatchPrimitive<0, true>(a_type, a_index, a_point, a_base, a_location);
}

template <class T, class... UserPrimitives>
template <size_t I, bool Handles>
EBGEOMETRY_HOST_DEVICE
inline T
Tape<T, UserPrimitives...>::dispatchPrimitive(const uint32_t      a_type,
                                              const uint32_t      a_index,
                                              const Vec3T<T>&     a_point,
                                              const void*         a_base,
                                              const PoolLocation& a_location) const noexcept
{
  if constexpr (I < Primitives::size) {
    if (a_type == I) {
      using P = typename TapeDetail::TypeAt<I, Primitives>::type;

      // A pool-resident primitive (a mesh SDF, a BVH union, a tape) is stored as its layout and resolves
      // against the tape's own location. It is evaluated out of line (evaluateHandle()), so that its
      // traversal stack is not part of the frame every analytic shape is evaluated in place in.
      if constexpr (isPoolHandle<P>) {
        if constexpr (Handles) {
          const auto& stored = TapeDetail::storeAt<I>(m_layout.m_primitives).at(a_base, a_index);

          return P(stored, a_location).signedDistance(a_point);
        }
        else {
          return this->evaluateHandle(a_type, a_index, a_point, a_base, a_location);
        }
      }
      else {
        return TapeDetail::storeAt<I>(m_layout.m_primitives).at(a_base, a_index).signedDistance(a_point);
      }
    }

    if constexpr (!Handles && I + 1 == DispatchSplit) {
      return this->dispatchRest(a_type, a_index, a_point, a_base, a_location);
    }
    else {
      return this->template dispatchPrimitive<I + 1, Handles>(a_type, a_index, a_point, a_base, a_location);
    }
  }
  else {
    EBGEOMETRY_EXPECT(false);

    return T(0);
  }
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_HOST_NOINLINE
T
Tape<T, UserPrimitives...>::dispatchRest(const uint32_t      a_type,
                                         const uint32_t      a_index,
                                         const Vec3T<T>&     a_point,
                                         const void*         a_base,
                                         const PoolLocation& a_location) const noexcept
{
  return this->template dispatchPrimitive<DispatchSplit, false>(a_type, a_index, a_point, a_base, a_location);
}

template <class T, class... UserPrimitives>
template <unsigned D, class V>
EBGEOMETRY_HOST_DEVICE
inline V
Tape<T, UserPrimitives...>::evaluateRepeat(const TapeDetail::Clause& a_clause,
                                           const Vec3T<T>&           a_point,
                                           const T*                  a_params,
                                           const void*               a_base,
                                           const PoolLocation&       a_location) const noexcept
{
  const uint32_t sub = a_clause.m_a;

  // Read from the parameters as the search needs them (see CSGOps::RepetitionView).
  const CSGOps::RepetitionView<T> repetition{a_params + a_clause.m_b};

  // A function of its own, so that the search's loops stay out of evaluateBody()'s text on the host.
  const auto f = [this, sub, a_base, &a_location](const Vec3T<T>& a_x) noexcept -> V {
    return this->template evaluate<D - 1, V>(sub, a_x, a_base, a_location);
  };

  return CSGOps::repetition<T>(repetition, a_point, f);
}

template <class T, class... UserPrimitives>
template <unsigned D, class V>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline V
Tape<T, UserPrimitives...>::evaluateBlur(const TapeDetail::Clause& a_clause,
                                         const Vec3T<T>&           a_point,
                                         const T*                  a_params,
                                         const void*               a_base,
                                         const PoolLocation&       a_location) const noexcept
{
  const uint32_t sub = a_clause.m_a;

  // In its own function rather than in evaluateBody(): there, the blur's loop (CSGOps::blur()) changed
  // how GCC inlines the primitive dispatch into the evaluator, and slowed tapes without a Blur by up to
  // 20%.
  const auto f = [this, sub, a_base, &a_location](const Vec3T<T>& a_x) noexcept -> V {
    return this->template evaluate<D - 1, V>(sub, a_x, a_base, a_location);
  };

  return CSGOps::blur(a_params + a_clause.m_b, a_point, f);
}

template <class T, class... UserPrimitives>
template <class V>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline V
Tape<T, UserPrimitives...>::evaluateLeaf(const uint32_t      a_program,
                                         const Vec3T<T>&     a_point,
                                         const void*         a_base,
                                         const PoolLocation& a_location) const noexcept
{
  using TapeDetail::Op;

  const TapeDetail::Program range   = m_layout.m_programs.at(a_base, a_program);
  const TapeDetail::Clause* clauses = m_layout.m_clauses.data(a_base);
  const T*                  params  = m_layout.m_params.data(a_base);

  // The clauses are those of evaluateBody(), with the point maps applied in turn: the shape is read at the point
  // the last map gives, and the value scaled after. The one Primitive clause sets the value (an untagged,
  // infinite start until then).
  Vec3T<T> point = a_point;
  V        value = CSGOps::pairInit<V>(false).m_first;

  for (uint32_t pc = range.m_begin; pc < range.m_end; pc++) {
    const TapeDetail::Clause& c = clauses[pc];

    if (c.m_op == Op::PushAffine) {
      point = CSGOps::affinePoint(params + c.m_a, point);
    }
    else if (c.m_op == Op::Primitive) {
      value = CSGOps::leafValue<V>(this->evaluatePrimitive(c.m_type, c.m_a, point, a_base, a_location), pc, point);
    }
    else if (c.m_op == Op::MulConstant) {
      value = CSGOps::times(value, params[c.m_a]);
    }
  }

  EBGEOMETRY_EXPECT(!std::isnan(CSGOps::valueOf(value)));

  return value;
}

template <class T, class... UserPrimitives>
template <unsigned D, class V>
EBGEOMETRY_HOST_DEVICE
inline V
Tape<T, UserPrimitives...>::evaluateUnion(const TapeDetail::Clause& a_clause,
                                          const Vec3T<T>&           a_point,
                                          const T*                  a_params,
                                          const void*               a_base,
                                          const PoolLocation&       a_location) const noexcept
{
  static_assert(D > 0, "Tape::evaluateUnion needs call depth left for its operands");

  const TapeDetail::UnionBVH<T, BranchingRatio>& parts = m_layout.m_unions.at(a_base, a_clause.m_a);

  const UnionRoot bvh(parts.m_bvh, a_location);
  const UnionRoot unbounded(parts.m_unbounded, a_location);

  const auto unboundedPrograms = unbounded.getPrimitives();
  const auto programs          = bvh.getPrimitives();

  // One traversal for both kinds, so that a device frame holds one traversal stack. It tracks the two
  // smallest values (a <= b), the unbounded operands first so that their values can prune it. A node is
  // kept iff its box is within max(0, a) of the point for a union, as in BVHUnion, and within max(0, b)
  // for a smooth union, as in BVHSmoothUnion.
  const bool smooth = (a_clause.m_op == TapeDetail::Op::SmoothUnionBVH);

  struct Closest
  {
    V a = CSGOps::pairInit<V>(false).m_first;
    V b = CSGOps::pairInit<V>(false).m_second;
  };

  const auto insert = [](Closest& a_closest, const V& a_value) noexcept {
    EBGEOMETRY_EXPECT(!std::isnan(CSGOps::valueOf(a_value)));

    if (CSGOps::valueOf(a_value) < CSGOps::valueOf(a_closest.a)) {
      a_closest.b = a_closest.a;
      a_closest.a = a_value;
    }
    else if (CSGOps::valueOf(a_value) < CSGOps::valueOf(a_closest.b)) {
      a_closest.b = a_value;
    }
  };

  Closest closest;

  // An operand that is one shape is evaluated directly, any other through its sub-program.
  const auto operand = [this, &a_point, a_base, &a_location](const uint32_t a_program) noexcept -> V {
    if ((a_program & TapeDetail::LeafProgram) != 0U) {
      return this->template evaluateLeaf<V>(a_program & ~TapeDetail::LeafProgram, a_point, a_base, a_location);
    }

    return this->template evaluate<D - 1, V>(a_program, a_point, a_base, a_location);
  };

  for (uint32_t i = 0; i < unboundedPrograms.size(); i++) {
    insert(closest, operand(unboundedPrograms[i]));
  }

  const auto evalLeaf =
    [&programs, &operand, &insert](Closest& a_closest, const size_t a_offset, const size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        insert(a_closest, operand(programs[static_cast<uint32_t>(i)]));
      }
    };

  const auto pruneDist2 = [smooth](const Closest& a_closest) noexcept -> T {
    const T bound = Math::max(T(0), CSGOps::valueOf(smooth ? a_closest.b : a_closest.a));

    return bound * bound;
  };

  bvh.traverse(BVH::PointQuery<T>{a_point}, closest, evalLeaf, pruneDist2);

  if (!smooth) {
    return closest.a;
  }

  const T s = a_params[a_clause.m_b];

  // A smooth union's value comes from the smaller operand's surface.
  const T a = CSGOps::valueOf(closest.a);
  const T b = CSGOps::valueOf(closest.b);

  return CSGOps::withValue(closest.a, (a_clause.m_flags == 0U) ? SmoothMinOp<T>{}(a, b, s) : ExpMinOp<T>{}(a, b, s));
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline typename Tape<T, UserPrimitives...>::BV
Tape<T, UserPrimitives...>::computeBoundingVolume() const noexcept
{
  return m_layout.m_boundingVolume;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline DistanceQuality
Tape<T, UserPrimitives...>::getDistanceQuality() const noexcept
{
  return m_layout.m_quality;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
Tape<T, UserPrimitives...>::traceRay(const RayT<T>&       a_ray,
                                     const T              a_tMin,
                                     const T              a_tMax,
                                     const RayOptions<T>& a_options) const noexcept
{
  return this->template traceBody<true>(0U, a_ray, a_tMin, a_tMax, a_options);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline HitT<T>
Tape<T, UserPrimitives...>::traceProgram(const uint32_t       a_program,
                                         const RayT<T>&       a_ray,
                                         const T              a_tMin,
                                         const T              a_tMax,
                                         const RayOptions<T>& a_options) const noexcept
{
  return this->template traceBody<false>(a_program, a_ray, a_tMin, a_tMax, a_options);
}

template <class T, class... UserPrimitives>
template <bool Whole>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
Tape<T, UserPrimitives...>::traceBody(const uint32_t       a_program,
                                      const RayT<T>&       a_ray,
                                      const T              a_tMin,
                                      const T              a_tMax,
                                      const RayOptions<T>& a_options) const noexcept
{
  // The whole object's program is 0. Not const: a constant (for the whole object) would need no capture in the lambdas
  // below, which clang rejects as an unused capture; the compiler folds it all the same.
  uint32_t program = Whole ? 0U : a_program;

  const void*        base     = this->base();
  const PoolLocation location = this->location();

  const auto trace = [this, program, &a_ray, &a_options, base, &location](
                       const T a_t0, const T a_t1, const bool a_all, RaySpansT<T>& a_spans) noexcept {
    this->template rayEvaluate<MaxCallDepth>(program, a_ray, a_t0, a_t1, a_options, a_all, base, location, a_spans);
  };

  HitT<T> hit;

  if constexpr (hasFeature<TapeFeature::ExactOffsets>) {
    // A crossing on a level set is marked, and resolved once the trace is done (see resolveLevelHit()).
    uint32_t levelClause = 0U;

    const auto resolve =
      [this, program, &a_ray, &a_options, base, &location, &levelClause](const T a_t, const uint32_t a_leaf) noexcept {
        const HitT<T> leafHit =
          this->template rayLeafHit<MaxCallDepth>(program, a_ray, a_t, a_leaf, a_options, base, location);

        if (leafHit.m_primitive == LevelLeaf) {
          levelClause = a_leaf;
        }

        return leafHit;
      };

    hit = RayDetail::traceSpans(trace, resolve, a_ray, a_tMin, a_tMax);

    if (hit.hit() && hit.m_primitive == LevelLeaf) {
      const HitT<T> level = this->resolveLevelHit(levelClause, a_ray, hit.m_t, a_options, base, location);

      hit.m_normal    = level.m_normal;
      hit.m_primitive = level.m_primitive;

      // Outward of the composition, as traceSpans() orients a resolved normal: against the ray where it enters.
      if ((dot(hit.m_normal, a_ray.m_direction) < T(0)) != hit.m_entering) {
        hit.m_normal = -hit.m_normal;
      }
    }
  }
  else {
    const auto resolve = [this, program, &a_ray, &a_options, base, &location](const T        a_t,
                                                                              const uint32_t a_leaf) noexcept {
      return this->template rayLeafHit<MaxCallDepth>(program, a_ray, a_t, a_leaf, a_options, base, location);
    };

    hit = RayDetail::traceSpans(trace, resolve, a_ray, a_tMin, a_tMax);
  }

  // A crossing found by marching a subtree is named by the leaf deciding the program's value there.
  if (hit.hit() && hit.m_primitive == MarchedLeaf) {
    if constexpr (Whole) {
      hit.m_primitive = this->closestLeaf(hit.m_point).m_leaf;
    }
    else {
      hit.m_primitive =
        this->leafOf(this->template evaluate<MaxCallDepth, CSGOps::Tagged<T>>(program, hit.m_point, base, location))
          .m_leaf;
    }
  }

  return hit;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline void
Tape<T, UserPrimitives...>::raySpans(const RayT<T>&       a_ray,
                                     const T              a_tMin,
                                     const T              a_tMax,
                                     const RayOptions<T>& a_options,
                                     const bool           a_firstOnly,
                                     RaySpansT<T>&        a_spans) const noexcept
{
  // As a leaf of another composition, the tape is named by it: its own leaves are not.
  this->template rayEvaluate<MaxCallDepth>(
    0U, a_ray, a_tMin, a_tMax, a_options, !a_firstOnly, this->base(), this->location(), a_spans);
}

template <class T, class... UserPrimitives>
template <unsigned D>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline void
Tape<T, UserPrimitives...>::rayEvaluate(const uint32_t       a_program,
                                        const RayT<T>&       a_ray,
                                        const T              a_t0,
                                        const T              a_t1,
                                        const RayOptions<T>& a_options,
                                        const bool           a_all,
                                        const void*          a_base,
                                        const PoolLocation&  a_location,
                                        RaySpansT<T>&        a_spans) const noexcept
{
  using TapeDetail::RayOp;

  const TapeDetail::Program range   = m_layout.m_programs.at(a_base, m_layout.m_rayPrograms + a_program);
  const TapeDetail::Clause* clauses = m_layout.m_clauses.data(a_base);
  const T*                  params  = m_layout.m_params.data(a_base);

  RaySpansT<T> lists[RayStackSize];
  RaySpansT<T> scratch;

  RayDetail::MappedRay<T> rays[PointStackSize];

  uint32_t nl = 0;
  uint32_t nr = 0;

  rays[nr++] = RayDetail::MappedRay<T>{a_ray, T(1), a_t0, a_t1};

  for (uint32_t pc = range.m_begin; pc < range.m_end; pc++) {
    const TapeDetail::RayClause    c         = TapeDetail::rayClause(clauses[pc]);
    const RayDetail::MappedRay<T>& top       = rays[nr - 1];
    const bool                     firstOnly = !a_all && (c.m_flags & 1U) == 0U;

    switch (c.m_op) {
    case RayOp::Leaf: {
      this->rayLeaf(c, top.m_ray, top.m_t0, top.m_t1, a_options, firstOnly, a_base, a_location, lists[nl]);

      // A crossing is named by its leaf's clause, which rayLeafHit() finds again.
      RayDetail::nameLeaf(pc, lists[nl++]);

      break;
    }
    case RayOp::Marched: {
      RaySpansT<T>& spans = lists[nl++];

      // An intersection's operand is marched only within the spans of the operands before it.
      if ((c.m_flags & 2U) != 0U) {
        this->template rayMarchedWithin<D>(
          c, top.m_ray, a_options, !a_all && (c.m_flags & 4U) != 0U, lists[nl - 2], a_base, a_location, spans);
      }
      else {
        const T* box = params + c.m_b;

        const Subtree<D> subtree{this,
                                 c.m_a,
                                 BV(Vec3T<T>(box[0], box[1], box[2]), Vec3T<T>(box[3], box[4], box[5])),
                                 static_cast<DistanceQuality>(c.m_c),
                                 a_base,
                                 a_location};

        RayDetail::shapeSpans(subtree, top.m_ray, top.m_t0, top.m_t1, a_options, firstOnly, spans);
      }

      RayDetail::nameLeaf(pc, spans);

      break;
    }
    case RayOp::Union: {
      nl--;

      RayDetail::uniteInto(lists[nl - 1], lists[nl]);

      if (firstOnly) {
        RayDetail::keepFirst(lists[nl - 1]);
      }

      break;
    }
    case RayOp::Intersection: {
      nl--;

      RayDetail::intersectInto(lists[nl - 1], lists[nl], scratch);

      break;
    }
    case RayOp::Complement: {
      RayDetail::complement(lists[nl - 1], top.m_t0, scratch);
      RayDetail::copySpans(scratch, lists[nl - 1]);

      break;
    }
    case RayOp::PushAffine: {
      rays[nr] = RayDetail::mapRay(params + c.m_a, top.m_ray, top.m_t0, top.m_t1);

      nr++;

      break;
    }
    case RayOp::PopAffine: {
      nr--;

      RayDetail::mapBack(rays[nr], rays[nr - 1].m_t0, rays[nr - 1].m_t1, lists[nl - 1]);

      break;
    }
    case RayOp::UnionBVH: {
      if constexpr (D > 0) {
        this->template rayUnion<D>(c, top.m_ray, top.m_t0, top.m_t1, a_options, a_all, a_base, a_location, lists[nl++]);
      }
      else {
        EBGEOMETRY_EXPECT(false);

        lists[nl++].reset(top.m_t1);
      }

      break;
    }
    default: {
      EBGEOMETRY_EXPECT(false && "Tape: unknown ray clause");

      break;
    }
    }
  }

  EBGEOMETRY_EXPECT(nl == 1U && nr == 1U);

  RayDetail::copySpans(lists[0], a_spans);
}

template <class T, class... UserPrimitives>
template <unsigned D>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline void
Tape<T, UserPrimitives...>::rayMarchedWithin(const TapeDetail::RayClause& a_clause,
                                             const RayT<T>&               a_ray,
                                             const RayOptions<T>&         a_options,
                                             const bool                   a_firstOnly,
                                             const RaySpansT<T>&          a_within,
                                             const void*                  a_base,
                                             const PoolLocation&          a_location,
                                             RaySpansT<T>&                a_spans) const noexcept
{
  // A mesh's level set deferred in an intersection (bit 3) is traced, not marched.
  if constexpr (hasFeature<TapeFeature::ExactOffsets>) {
    if ((a_clause.m_flags & 8U) != 0U) {
      this->rayLevelWithin(a_clause, a_ray, a_options, a_firstOnly, a_within, a_base, a_location, a_spans);

      return;
    }
  }

  const T* box = m_layout.m_params.data(a_base) + a_clause.m_b;

  const Subtree<D> subtree{this,
                           a_clause.m_a,
                           BV(Vec3T<T>(box[0], box[1], box[2]), Vec3T<T>(box[3], box[4], box[5])),
                           static_cast<DistanceQuality>(a_clause.m_c),
                           a_base,
                           a_location};

  RaySpansT<T> piece;

  RayDetail::spansWithin(subtree, a_ray, a_within, a_options, a_firstOnly, piece, a_spans);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline void
Tape<T, UserPrimitives...>::rayLeaf(const TapeDetail::RayClause& a_clause,
                                    const RayT<T>&               a_ray,
                                    const T                      a_t0,
                                    const T                      a_t1,
                                    const RayOptions<T>&         a_options,
                                    const bool                   a_firstOnly,
                                    const void*                  a_base,
                                    const PoolLocation&          a_location,
                                    RaySpansT<T>&                a_spans) const noexcept
{
  if constexpr (hasFeature<TapeFeature::ExactOffsets>) {
    if ((a_clause.m_flags & 8U) != 0U) {
      this->rayLevelLeaf(a_clause, a_ray, a_t0, a_t1, a_options, a_firstOnly, a_base, a_location, a_spans);

      return;
    }
  }

  this->template dispatchRayLeaf<0>(a_clause, a_ray, a_t0, a_t1, a_options, a_firstOnly, a_base, a_location, a_spans);
}

template <class T, class... UserPrimitives>
template <size_t I>
EBGEOMETRY_HOST_DEVICE
inline void
Tape<T, UserPrimitives...>::dispatchRayLeaf(const TapeDetail::RayClause& a_clause,
                                            const RayT<T>&               a_ray,
                                            const T                      a_t0,
                                            const T                      a_t1,
                                            const RayOptions<T>&         a_options,
                                            const bool                   a_firstOnly,
                                            const void*                  a_base,
                                            const PoolLocation&          a_location,
                                            RaySpansT<T>&                a_spans) const noexcept
{
  if constexpr (I < Primitives::size) {
    using P = typename TapeDetail::TypeAt<I, Primitives>::type;

    if constexpr (RayDetail::HasRayFirstHit<P, T>::value) {
      if (a_clause.m_type == I) {
        const auto& stored = TapeDetail::storeAt<I>(m_layout.m_primitives).at(a_base, a_clause.m_a);

        // A pool-resident primitive is stored as its layout and resolves against the tape's own location.
        if constexpr (isPoolHandle<P>) {
          RayDetail::shapeSpans(P(stored, a_location), a_ray, a_t0, a_t1, a_options, a_firstOnly, a_spans);
        }
        else {
          RayDetail::shapeSpans(stored, a_ray, a_t0, a_t1, a_options, a_firstOnly, a_spans);
        }

        return;
      }
    }

    this->template dispatchRayLeaf<I + 1>(
      a_clause, a_ray, a_t0, a_t1, a_options, a_firstOnly, a_base, a_location, a_spans);
  }
  else {
    EBGEOMETRY_EXPECT(false);

    a_spans.reset(a_t1);
  }
}

template <class T, class... UserPrimitives>
template <unsigned D>
EBGEOMETRY_HOST_DEVICE
inline void
Tape<T, UserPrimitives...>::rayUnion(const TapeDetail::RayClause& a_clause,
                                     const RayT<T>&               a_ray,
                                     const T                      a_t0,
                                     const T                      a_t1,
                                     const RayOptions<T>&         a_options,
                                     const bool                   a_all,
                                     const void*                  a_base,
                                     const PoolLocation&          a_location,
                                     RaySpansT<T>&                a_spans) const noexcept
{
  static_assert(D > 0, "Tape::rayUnion needs call depth left for its operands");

  const TapeDetail::UnionBVH<T, BranchingRatio>& parts = m_layout.m_unions.at(a_base, a_clause.m_a);

  const UnionRoot bvh(parts.m_bvh, a_location);
  const UnionRoot unbounded(parts.m_unbounded, a_location);

  const auto unboundedPrograms = unbounded.getPrimitives();
  const auto programs          = bvh.getPrimitives();

  a_spans.reset(a_t1);

  RaySpansT<T> operand;

  // Whether the union is asked only for its first crossing.
  const bool firstOnly = !a_all && (a_clause.m_flags & 1U) == 0U;

  // Each operand is traced only up to what the union so far is known up to, beyond which it can change
  // nothing known.
  const auto add = [this, &a_ray, a_t0, &a_options, a_all, firstOnly, a_base, &a_location, &operand](
                     RaySpansT<T>& a_union, const uint32_t a_program) noexcept {
    this->template rayEvaluate<D - 1>(a_program & ~TapeDetail::LeafProgram,
                                      a_ray,
                                      a_t0,
                                      a_union.m_horizon,
                                      a_options,
                                      a_all,
                                      a_base,
                                      a_location,
                                      operand);

    RayDetail::uniteInto(a_union, operand);

    if (firstOnly) {
      RayDetail::keepFirst(a_union);
    }
  };

  for (uint32_t i = 0; i < unboundedPrograms.size(); i++) {
    add(a_spans, unboundedPrograms[i]);
  }

  const auto leaf = [&programs, &add](RaySpansT<T>& a_union, const size_t a_offset, const size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      add(a_union, programs[static_cast<uint32_t>(i)]);
    }
  };

  // A box the ray enters beyond what the union is known up to can change nothing known.
  const auto bound = [](const RaySpansT<T>& a_union) noexcept -> T { return a_union.m_horizon; };

  bvh.traverse(BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction, a_t0), a_spans, leaf, bound);
}

template <class T, class... UserPrimitives>
template <unsigned D, bool Level>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline HitT<T>
Tape<T, UserPrimitives...>::rayLeafHit(const uint32_t       a_program,
                                       const RayT<T>&       a_ray,
                                       const T              a_t,
                                       const uint32_t       a_clause,
                                       const RayOptions<T>& a_options,
                                       const void*          a_base,
                                       const PoolLocation&  a_location) const noexcept
{
  using TapeDetail::RayOp;

  const TapeDetail::Program range   = m_layout.m_programs.at(a_base, m_layout.m_rayPrograms + a_program);
  const TapeDetail::Clause* clauses = m_layout.m_clauses.data(a_base);
  const T*                  params  = m_layout.m_params.data(a_base);

  // The ray in each affine frame down to the leaf, and the maps that made them.
  RayDetail::MappedRay<T> rays[PointStackSize];
  uint32_t                maps[PointStackSize];

  uint32_t nr = 0;

  rays[nr++] = RayDetail::MappedRay<T>{a_ray, T(1), a_t, a_t};

  HitT<T> hit;

  for (uint32_t pc = range.m_begin; pc < range.m_end && !hit.hit(); pc++) {
    const TapeDetail::RayClause    c   = TapeDetail::rayClause(clauses[pc]);
    const RayDetail::MappedRay<T>& top = rays[nr - 1];

    if (pc == a_clause) {
      if constexpr (hasFeature<TapeFeature::ExactOffsets>) {
        if ((c.m_flags & 8U) != 0U && (c.m_op == RayOp::Leaf || c.m_op == RayOp::Marched)) {
          // A level set's crossing: resolved only in the level-set walk (see resolveLevelHit()); its leaf number
          // is in m_b of a Leaf clause and in m_c of a deferred one.
          if constexpr (Level) {
            hit = this->levelClauseHit(c, top.m_ray, top.m_t0, a_options, a_base, a_location);

            hit.m_primitive = (c.m_op == RayOp::Leaf) ? c.m_b : c.m_c;
          }
          else {
            hit.m_t         = top.m_t0;
            hit.m_primitive = LevelLeaf;
          }

          hit.m_status = HitStatus::Hit;

          break;
        }
      }

      if (c.m_op == RayOp::Leaf) {
        hit = this->primitiveLeafHit(c, top.m_ray, top.m_t0, a_options, a_base, a_location);

        hit.m_primitive = c.m_b;
      }
      else {
        // A Marched clause is a leaf of the trace as a whole: its subtree has no ray program, so a trace never
        // names a clause inside it, and a hit there reports MarchedLeaf.
        const T* box = params + c.m_b;

        const Subtree<D> subtree{this,
                                 c.m_a,
                                 BV(Vec3T<T>(box[0], box[1], box[2]), Vec3T<T>(box[3], box[4], box[5])),
                                 static_cast<DistanceQuality>(c.m_c),
                                 a_base,
                                 a_location};

        hit = RayDetail::leafHit(subtree, top.m_ray, top.m_t0, a_options);

        hit.m_primitive = MarchedLeaf;
      }

      // Found even where the leaf's own search came up empty: its normal is then its gradient.
      hit.m_status = HitStatus::Hit;

      break;
    }

    switch (c.m_op) {
    case RayOp::PushAffine: {
      rays[nr] = RayDetail::mapRay(params + c.m_a, top.m_ray, top.m_t0, top.m_t0);
      maps[nr] = c.m_a;

      nr++;

      break;
    }
    case RayOp::PopAffine: {
      nr--;

      break;
    }
    case RayOp::UnionBVH: {
      if constexpr (D > 0) {
        // The operands' ray programs, with those they call, are laid out in turn (sub-programs c.m_b up to
        // c.m_c). The leaf's own ray program is the last that begins at or before it; from there up, the
        // sub-program compiled for this one is the operand that holds it, if it is one of this union's.
        const uint32_t numPrograms = m_layout.m_numPrograms;

        uint32_t lo = 0;
        uint32_t hi = numPrograms;

        while (hi - lo > 1U) {
          const uint32_t mid = lo + (hi - lo) / 2U;

          if (m_layout.m_programs.at(a_base, m_layout.m_rayPrograms + mid).m_begin <= a_clause) {
            lo = mid;
          }
          else {
            hi = mid;
          }
        }

        const uint32_t parents = m_layout.m_rayPrograms + numPrograms;

        uint32_t operand = lo;

        while (operand != UINT32_MAX && operand != a_program) {
          const uint32_t parent = m_layout.m_programs.at(a_base, parents + operand).m_begin;

          if (parent == a_program) {
            break;
          }

          operand = parent;
        }

        if (operand != UINT32_MAX && operand != a_program && operand >= c.m_b && operand < c.m_c) {
          hit = this->template rayLeafHit<D - 1, Level>(
            operand, top.m_ray, top.m_t0, a_clause, a_options, a_base, a_location);
        }
      }

      break;
    }
    default: {
      break;
    }
    }
  }

  // The trace named a leaf of this program or of the operands it calls, so the walk finds it.
  EBGEOMETRY_EXPECT(hit.hit());

  // The leaf's normal, in its frame, mapped back to the program's.
  for (uint32_t f = nr - 1; f > 0; f--) {
    hit.m_normal = RayDetail::mapNormalBack(params + maps[f], hit.m_normal);
  }

  return hit;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline HitT<T>
Tape<T, UserPrimitives...>::primitiveLeafHit(const TapeDetail::RayClause& a_clause,
                                             const RayT<T>&               a_ray,
                                             const T                      a_t,
                                             const RayOptions<T>&         a_options,
                                             const void*                  a_base,
                                             const PoolLocation&          a_location) const noexcept
{
  return this->template dispatchLeafHit<0>(a_clause, a_ray, a_t, a_options, a_base, a_location);
}

template <class T, class... UserPrimitives>
template <size_t I>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
Tape<T, UserPrimitives...>::dispatchLeafHit(const TapeDetail::RayClause& a_clause,
                                            const RayT<T>&               a_ray,
                                            const T                      a_t,
                                            const RayOptions<T>&         a_options,
                                            const void*                  a_base,
                                            const PoolLocation&          a_location) const noexcept
{
  if constexpr (I < Primitives::size) {
    using P = typename TapeDetail::TypeAt<I, Primitives>::type;

    if constexpr (RayDetail::HasRayFirstHit<P, T>::value) {
      if (a_clause.m_type == I) {
        const auto& stored = TapeDetail::storeAt<I>(m_layout.m_primitives).at(a_base, a_clause.m_a);

        if constexpr (isPoolHandle<P>) {
          return RayDetail::leafHit(P(stored, a_location), a_ray, a_t, a_options);
        }
        else {
          return RayDetail::leafHit(stored, a_ray, a_t, a_options);
        }
      }
    }

    return this->template dispatchLeafHit<I + 1>(a_clause, a_ray, a_t, a_options, a_base, a_location);
  }
  else {
    EBGEOMETRY_EXPECT(false);

    return HitT<T>();
  }
}

template <class T, class... UserPrimitives>
template <size_t I, bool MeshOnly, class Visit>
EBGEOMETRY_HOST_DEVICE
inline void
Tape<T, UserPrimitives...>::visitLevelPrimitive(const TapeDetail::RayClause& a_clause,
                                                const void*                  a_base,
                                                const PoolLocation&          a_location,
                                                const Visit&                 a_visit) const noexcept
{
  if constexpr (I < Primitives::size) {
    using P = typename TapeDetail::TypeAt<I, Primitives>::type;

    constexpr bool level = MeshOnly ? RayDetail::IsMeshLevel<P>::value : RayDetail::HasLevelForm<P>::value;

    if constexpr (level) {
      if (a_clause.m_type == I) {
        const auto& stored = TapeDetail::storeAt<I>(m_layout.m_primitives).at(a_base, a_clause.m_a);

        if constexpr (isPoolHandle<P>) {
          a_visit(P(stored, a_location));
        }
        else {
          a_visit(stored);
        }

        return;
      }
    }

    this->template visitLevelPrimitive<I + 1, MeshOnly>(a_clause, a_base, a_location, a_visit);
  }
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
EBGEOMETRY_HOST_NOINLINE
void
Tape<T, UserPrimitives...>::rayLevelLeaf(const TapeDetail::RayClause& a_clause,
                                         const RayT<T>&               a_ray,
                                         const T                      a_t0,
                                         const T                      a_t1,
                                         const RayOptions<T>&         a_options,
                                         const bool                   a_firstOnly,
                                         const void*                  a_base,
                                         const PoolLocation&          a_location,
                                         RaySpansT<T>&                a_spans) const noexcept
{
  const T    level   = m_layout.m_params.data(a_base)[a_clause.m_c];
  const bool annulus = (a_clause.m_flags & 16U) != 0U;

  a_spans.reset(a_t1);

  this->template visitLevelPrimitive<0, false>(a_clause, a_base, a_location, [&](const auto& a_shape) noexcept {
    using S = std::decay_t<decltype(a_shape)>;

    RayDetail::levelSpans(
      RayDetail::LevelView<S>{a_shape, level, annulus}, a_ray, a_t0, a_t1, a_options, a_firstOnly, a_spans);
  });
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
EBGEOMETRY_HOST_NOINLINE
void
Tape<T, UserPrimitives...>::rayLevelWithin(const TapeDetail::RayClause& a_clause,
                                           const RayT<T>&               a_ray,
                                           const RayOptions<T>&         a_options,
                                           const bool                   a_firstOnly,
                                           const RaySpansT<T>&          a_within,
                                           const void*                  a_base,
                                           const PoolLocation&          a_location,
                                           RaySpansT<T>&                a_spans) const noexcept
{
  const T*   block   = m_layout.m_params.data(a_base) + a_clause.m_b;
  const bool annulus = (a_clause.m_flags & 16U) != 0U;
  const bool mapped  = (a_clause.m_flags & 32U) != 0U;

  a_spans.reset(a_within.m_horizon);

  this->template visitLevelPrimitive<0, true>(a_clause, a_base, a_location, [&](const auto& a_shape) noexcept {
    using S = std::decay_t<decltype(a_shape)>;

    const RayDetail::LevelView<S> view{a_shape, block[0], annulus};

    if (!mapped) {
      RayDetail::levelSpansWithin(view, a_ray, a_within, T(1), a_options, a_firstOnly, a_spans);

      return;
    }

    // Traced along the ray mapped into the primitive's frame, the other operands' spans scaled into it, and the
    // list mapped back.
    const T t0 = (a_within.m_count > 0U) ? a_within.m_spans[0].m_in.m_t : a_within.m_horizon;

    const RayDetail::MappedRay<T> m = RayDetail::mapRay(block + 1, a_ray, t0, a_within.m_horizon);

    RayDetail::levelSpansWithin(view, m.m_ray, a_within, m.m_scale, a_options, a_firstOnly, a_spans);
    RayDetail::mapBack(m, t0, a_within.m_horizon, a_spans);
  });
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
EBGEOMETRY_HOST_NOINLINE
HitT<T>
Tape<T, UserPrimitives...>::levelClauseHit(const TapeDetail::RayClause& a_clause,
                                           const RayT<T>&               a_ray,
                                           const T                      a_t,
                                           const RayOptions<T>&         a_options,
                                           const void*                  a_base,
                                           const PoolLocation&          a_location) const noexcept
{
  const T*   params  = m_layout.m_params.data(a_base);
  const bool marched = a_clause.m_op == TapeDetail::RayOp::Marched;
  const T    level   = marched ? params[a_clause.m_b] : params[a_clause.m_c];
  const bool annulus = (a_clause.m_flags & 16U) != 0U;
  const bool mapped  = marched && (a_clause.m_flags & 32U) != 0U;

  HitT<T> hit;

  this->template visitLevelPrimitive<0, false>(a_clause, a_base, a_location, [&](const auto& a_shape) noexcept {
    using S = std::decay_t<decltype(a_shape)>;

    const RayDetail::LevelView<S> view{a_shape, level, annulus};

    if (!mapped) {
      hit = RayDetail::levelLeafHit(view, a_ray, a_t, a_options);

      return;
    }

    const RayDetail::MappedRay<T> m = RayDetail::mapRay(params + a_clause.m_b + 1, a_ray, a_t, a_t);

    hit = RayDetail::levelLeafHit(view, m.m_ray, m.m_t0, a_options);

    hit.m_t      = a_t;
    hit.m_point  = a_ray.at(a_t);
    hit.m_normal = RayDetail::mapNormalBack(params + a_clause.m_b + 1, hit.m_normal);
  });

  return hit;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
EBGEOMETRY_HOST_NOINLINE
HitT<T>
Tape<T, UserPrimitives...>::resolveLevelHit(const uint32_t       a_clause,
                                            const RayT<T>&       a_ray,
                                            const T              a_t,
                                            const RayOptions<T>& a_options,
                                            const void*          a_base,
                                            const PoolLocation&  a_location) const noexcept
{
  return this->template rayLeafHit<MaxCallDepth, true>(0U, a_ray, a_t, a_clause, a_options, a_base, a_location);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getNumPrograms() const noexcept
{
  return m_layout.m_numPrograms;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getNumClauses() const noexcept
{
  // The sub-programs' clauses come first, one after another; the ray programs' follow them.
  return m_layout.m_programs.at(this->base(), m_layout.m_numPrograms - 1U).m_end;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getNumBVHUnions() const noexcept
{
  return m_layout.m_unions.size();
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getNumPrimitives() const noexcept
{
  return m_layout.m_numPrimitives;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getLargestPlainUnion() const noexcept
{
  return m_layout.m_largestPlainUnion;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline SurfaceLabelId
Tape<T, UserPrimitives...>::getSurfaceLabel(const uint32_t a_leaf) const noexcept
{
  return (a_leaf < m_layout.m_surfaceLabels.size()) ? SurfaceLabelId(m_layout.m_surfaceLabels.at(this->base(), a_leaf))
                                                    : SurfaceLabelId::none();
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline SurfaceLabelId
Tape<T, UserPrimitives...>::getSurfaceLabel(const LeafRef<T>& a_ref) const noexcept
{
  return this->getSurfaceLabel(a_ref.m_leaf);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getNumSurfaceLabels() const noexcept
{
  return m_layout.m_numLabels;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline T
Tape<T, UserPrimitives...>::wholeValue(const Vec3T<T>& a_point) const noexcept
{
  return this->template evaluate<MaxCallDepth, T>(0U, a_point, this->base(), this->location());
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::numRegionOperands() const noexcept
{
  return (m_layout.m_regionPrograms.size() > 0U) ? m_layout.m_regionPrograms.at(this->base(), 0U) : 0U;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline MaterialLabelId
Tape<T, UserPrimitives...>::getMaterialOf(const uint32_t a_region) const noexcept
{
  const uint32_t n = this->numRegionOperands();

  if (m_layout.m_regionPrograms.size() == 0U || a_region >= ((n > 0U) ? n + 1U : 2U)) {
    return MaterialLabelId::none();
  }

  return MaterialLabelId(m_layout.m_regionPrograms.at(this->base(), 1U + n + a_region));
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getNumRegions() const noexcept
{
  const uint32_t n = this->numRegionOperands();

  return (n > 0U) ? n + 1U : 2U;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline T
Tape<T, UserPrimitives...>::regionSolid(const uint32_t  a_region,
                                        const uint32_t  a_level,
                                        const Vec3T<T>& a_point) const noexcept
{
  const void* base = this->base();

  // The region's record: its number of masks, their numbers, then its programs by level.
  const uint32_t record  = m_layout.m_regionPrograms.at(base, a_region);
  const uint32_t masks   = m_layout.m_regionPrograms.at(base, record);
  const uint32_t program = m_layout.m_regionPrograms.at(base, record + 1U + masks + a_level);

  EBGEOMETRY_EXPECT(a_level <= masks);

  return this->template evaluate<MaxCallDepth, T>(program, a_point, base, this->location());
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::sharedMasks(const uint32_t a_first, const uint32_t a_second) const noexcept
{
  const void* base = this->base();

  const uint32_t first  = m_layout.m_regionPrograms.at(base, a_first);
  const uint32_t second = m_layout.m_regionPrograms.at(base, a_second);
  const uint32_t n = Math::min(m_layout.m_regionPrograms.at(base, first), m_layout.m_regionPrograms.at(base, second));

  // The masks are listed outermost first, so those above both regions are a common prefix.
  uint32_t shared = 0;

  while (shared < n && m_layout.m_regionPrograms.at(base, first + 1U + shared) ==
                         m_layout.m_regionPrograms.at(base, second + 1U + shared)) {
    shared++;
  }

  return shared;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline uint32_t
Tape<T, UserPrimitives...>::region(const Vec3T<T>& a_point) const noexcept
{
  // An object with no Overlay is one region.
  if (this->numRegionOperands() == 0U) {
    return (this->wholeValue(a_point) < T(0)) ? 1U : 0U;
  }

  const uint32_t n = this->numRegionOperands();

  // The last region whose solid, with all its masks, holds the point: later regions are on top.
  for (uint32_t k = n; k > 0U; k--) {
    if (this->regionSolid(k, 0U, a_point) < T(0)) {
      return k;
    }
  }

  return 0U;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline T
Tape<T, UserPrimitives...>::signedDistance(const Vec3T<T>& a_point, const uint32_t a_region) const noexcept
{
  const uint32_t n = this->numRegionOperands();

  EBGEOMETRY_EXPECT(a_region <= Math::max(n, 1U));

  // The background is the outside of the whole object; an object with no Overlay is region 1.
  if (a_region == 0U) {
    return -this->wholeValue(a_point);
  }

  if (n == 0U) {
    return this->wholeValue(a_point);
  }

  // Region k is its solid with its masks, minus each later solid without the masks the two share (see
  // TapeDetail::RegionSolid): max(S_k, -min_{j>k} S_j).
  T later = Math::Limits<T>::max();

  for (uint32_t j = a_region + 1U; j <= n; j++) {
    later = Math::min(later, this->regionSolid(j, this->sharedMasks(a_region, j), a_point));
  }

  return Math::max(this->regionSolid(a_region, 0U, a_point), -later);
}

template <class T, class... UserPrimitives>
template <class Visit>
EBGEOMETRY_HOST_DEVICE
inline void
Tape<T, UserPrimitives...>::regionDistances(const Vec3T<T>& a_point, Visit&& a_visit) const noexcept
{
  const uint32_t n = this->numRegionOperands();

  // An object with no Overlay is region 1, the background its outside.
  if (n == 0U) {
    const T whole = this->wholeValue(a_point);

    a_visit(1U, whole);
    a_visit(0U, -whole);

    return;
  }

  // Whether every region has the same masks: then every later solid drops all of them.
  const void*    base    = this->base();
  const uint32_t masks   = m_layout.m_regionPrograms.at(base, m_layout.m_regionPrograms.at(base, 1U));
  bool           uniform = true;

  for (uint32_t k = 2; k <= n && uniform; k++) {
    uniform = m_layout.m_regionPrograms.at(base, m_layout.m_regionPrograms.at(base, k)) == masks &&
              this->sharedMasks(1U, k) == masks;
  }

  if (uniform) {
    T later = Math::Limits<T>::max();

    for (uint32_t k = n; k > 0U; k--) {
      const T masked   = this->regionSolid(k, 0U, a_point);
      const T unmasked = (masks == 0U) ? masked : this->regionSolid(k, masks, a_point);

      a_visit(k, Math::max(masked, -later));

      later = Math::min(later, unmasked);
    }
  }
  else {
    for (uint32_t k = n; k > 0U; k--) {
      a_visit(k, this->signedDistance(a_point, k));
    }
  }

  a_visit(0U, -this->wholeValue(a_point));
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline T
Tape<T, UserPrimitives...>::signedDistance(const Vec3T<T>&   a_point,
                                           const Subdomains& a_subdomains,
                                           const uint32_t    a_subdomain) const noexcept
{
  // Half the difference of the smallest function of a region in the subdomain and of one not in it: negative exactly in
  // the subdomain, zero only where a region in it meets one not in it. The smallest member's function alone would be
  // zero on the contact between two of its regions too (both functions are zero there), a sheet the subdomain's surface
  // does not have; the other term is strictly positive there. Each term is at most the distance to the subdomain's
  // boundary, so their average is a distance bound, exact where two regions' functions are.
  T in  = Math::Limits<T>::max();
  T out = Math::Limits<T>::max();

  this->regionDistances(a_point, [&](const uint32_t a_region, const T a_value) {
    if (a_subdomains.subdomainOf(a_region) == a_subdomain) {
      in = Math::min(in, a_value);
    }
    else {
      out = Math::min(out, a_value);
    }
  });

  return T(0.5) * (in - out);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline SurfaceRef<T>
Tape<T, UserPrimitives...>::closestSurface(const Vec3T<T>&   a_point,
                                           const Subdomains& a_subdomains,
                                           const uint32_t    a_subdomain) const noexcept
{
  T        value   = Math::Limits<T>::max();
  T        outside = Math::Limits<T>::max();
  uint32_t region  = Subdomains::None;
  uint32_t other   = Subdomains::None;

  // The subdomain's region whose function is smallest, and the smallest of the regions outside it; on a tie, the
  // lowest-numbered, as asked region by region.
  this->regionDistances(a_point, [&](const uint32_t a_region, const T a_value) {
    if (a_subdomains.subdomainOf(a_region) == a_subdomain) {
      if (a_value <= value) {
        value  = a_value;
        region = a_region;
      }
    }
    else if (a_value <= outside) {
      outside = a_value;
      other   = a_region;
    }
  });

  if (region == Subdomains::None) {
    return SurfaceRef<T>();
  }

  const SurfaceRef<T> inner = this->closestSurface(a_point, region);

  // A contact between two of the subdomain's own regions is not its surface, though the region's nearest one (near
  // where a region of it pokes out of another: the step where a core leaves its jacket): the nearest region outside
  // the subdomain names the surface instead, where it borders the subdomain, seen from the subdomain's side.
  if (a_subdomains.subdomainOf(inner.m_neighborRegion) == a_subdomain && other != Subdomains::None) {
    SurfaceRef<T> across = this->closestSurface(a_point, other);

    if (across.m_neighborRegion != Subdomains::None &&
        a_subdomains.subdomainOf(across.m_neighborRegion) == a_subdomain) {
      const uint32_t        owner    = across.m_ownerRegion;
      const MaterialLabelId material = across.m_ownerMaterial;

      across.m_ownerRegion      = across.m_neighborRegion;
      across.m_ownerMaterial    = across.m_neighborMaterial;
      across.m_neighborRegion   = owner;
      across.m_neighborMaterial = material;

      return across;
    }
  }

  return inner;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline SurfaceHitT<T>
Tape<T, UserPrimitives...>::firstRegionChange(const RayT<T>&       a_ray,
                                              const T              a_tMin,
                                              const T              a_tMax,
                                              const RayOptions<T>& a_options) const noexcept
{
  const void* base = this->base();

  const uint32_t n = this->numRegionOperands();

  // A nudge past a crossing, to name the region beyond it and to start the next query: a hundredth of the march's
  // tolerance at the object's scale. A whole tolerance named the region past a thin layer instead (where an outer
  // region's end face meets an inner one passing through it, the inner one: a seam with the end face's normal) and stepped over crossings that close.
  const BoundingVolumes::AABBT<T> box = m_layout.m_boundingVolume;

  const T diagonal = (box.getHighCorner() - box.getLowCorner()).length();
  const T scale    = (a_options.m_scale > T(0))                               ? a_options.m_scale
                     : (diagonal > T(0) && diagonal < Math::Limits<T>::max()) ? diagonal
                                                                              : T(1);
  const T nudge    = T(0.01) * a_options.m_tolerance * scale;

  SurfaceHitT<T> result;

  // An object with no Overlay is one region: every crossing of it changes the region, and whether the ray enters says
  // which way, so no distance is asked to name the two sides. Asking them was two searches of the whole object per
  // crossing (at the start and past the hit) on top of the one naming the surface; on an assembly of meshes, a view
  // of its one region (RegionView) rendered several times slower than the tape itself.
  if (n == 0U) {
    const HitT<T> hit = this->traceProgram(0U, a_ray, a_tMin + nudge, a_tMax, a_options);

    result.m_hit = hit;

    if (!hit.hit()) {
      return result;
    }

    result.m_surface = this->surfaceOf(this->tracedLeaf(0U, hit.m_primitive, hit.m_point));

    if (hit.m_entering) {
      result.m_hit.m_normal = -hit.m_normal;
    }

    result.m_surface.m_ownerRegion      = hit.m_entering ? 0U : 1U;
    result.m_surface.m_neighborRegion   = hit.m_entering ? 1U : 0U;
    result.m_surface.m_ownerMaterial    = this->getMaterialOf(result.m_surface.m_ownerRegion);
    result.m_surface.m_neighborMaterial = this->getMaterialOf(result.m_surface.m_neighborRegion);

    return result;
  }

  // A point outside the object's box is outside every region (region 0), with nothing evaluated: the box bounds every
  // region's solid (the builder's box rules never under-estimate, and the ray queries rely on it too). Where a ray
  // starts (from the camera, say) is usually there, and asking region() there would search the whole object (a mesh's
  // closest point) for every ray. A point within a nudge of the box is asked, so that a point on a face of the box is named
  // as region() names it.
  const auto regionAt = [this, &box, nudge](const Vec3T<T>& a_point) {
    const Vec3T<T> lo = box.getLowCorner();
    const Vec3T<T> hi = box.getHighCorner();

    for (int k = 0; k < 3; k++) {
      if (a_point[k] < lo[k] - nudge || a_point[k] > hi[k] + nudge) {
        return 0U;
      }
    }

    return this->region(a_point);
  };

  // A crossing whose two sides are the same region (found again from the surface the query starts on, or a crossing
  // of one solid inside a later one) changes nothing: the search goes on past it.
  T start = a_tMin + nudge;

  for (uint32_t tries = 0; tries < 16U && start < a_tMax; tries++) {
    const uint32_t from = regionAt(a_ray.m_origin + start * a_ray.m_direction);

    // The program whose crossing comes first: leaving the current region's solid (with all its masks), or entering a
    // later one's.
    HitT<T>  first;
    uint32_t program = 0U;
    uint32_t entered = 0U;

    first.m_t = a_tMax;

    for (uint32_t k = Math::max(from, 1U); k <= n; k++) {
      const uint32_t record = m_layout.m_regionPrograms.at(base, k);
      const uint32_t solid =
        m_layout.m_regionPrograms.at(base, record + 1U + m_layout.m_regionPrograms.at(base, record));

      const HitT<T> hit = this->traceProgram(solid, a_ray, start, first.m_t, a_options);

      // On a tie, the later region: it lies on top, as region() names the point.
      if (hit.hit() && (!first.hit() || hit.m_t <= first.m_t)) {
        first   = hit;
        program = solid;
        entered = k;
      }
    }

    if (!first.hit()) {
      result.m_hit = first;

      return result;
    }

    // Entering a later region's solid, the ray enters that region: it was in none of the later regions' solids, and
    // the crossing traced first is this one. Only a crossing leaving a solid asks which region lies beyond.
    uint32_t to = (first.m_entering && entered > from) ? entered : regionAt(first.m_point + nudge * a_ray.m_direction);

    // A marched crossing stops short of the surface: within the march's tolerance of it, and further along the ray
    // where the ray meets it at a shallow angle. The nudge can then stay on the near side, which reads as no change,
    // and a search starting there again would find the same crossing each time and miss the surface (a rounded edge,
    // a flat cap seen from low down). The crossing itself is where the traced solid's sign changes along
    // the ray: steps growing from the solid's distance find a point past it (within a hundred tolerances, or the ray
    // only touches the surface and passes it), and halving the step finds it, where it is then reported. An exact
    // crossing (a ray form) is on the surface and searches nothing.
    if (to == from) {
      const auto valueAt = [&](const T a_along) {
        return this->template evaluate<MaxCallDepth, T>(
          program, first.m_point + a_along * a_ray.m_direction, base, this->location());
      };

      const T    v0      = valueAt(T(0));
      const bool outside = v0 > T(0);

      if (std::abs(v0) > nudge) {
        const T reach = Math::max(T(100) * a_options.m_tolerance * scale, T(4) * std::abs(v0));

        T    lo      = T(0);
        T    hi      = std::abs(v0);
        bool flipped = false;

        while (hi <= reach) {
          if ((valueAt(hi) > T(0)) != outside) {
            flipped = true;

            break;
          }

          lo = hi;
          hi = T(2) * hi;
        }

        if (flipped) {
          for (int halving = 0; halving < 48 && hi - lo > nudge; halving++) {
            const T mid = T(0.5) * (lo + hi);

            if ((valueAt(mid) > T(0)) != outside) {
              hi = mid;
            }
            else {
              lo = mid;
            }
          }

          to = regionAt(first.m_point + (hi + nudge) * a_ray.m_direction);

          // The crossing is reported where it is, so that the next query, starting just past it, does not find it
          // again.
          if (to != from) {
            first.m_t += hi;
            first.m_point = first.m_point + hi * a_ray.m_direction;
          }
        }
      }
    }

    if (to != from) {
      // The surface crossed: the leaf the trace crossed, drilled.
      result.m_hit     = first;
      result.m_surface = this->surfaceOf(this->tracedLeaf(program, first.m_primitive, first.m_point));

      // The normal from the region left into the region entered: the traced solid's outward normal where the ray
      // leaves it, against it where the ray enters.
      if (first.m_entering) {
        result.m_hit.m_normal = -first.m_normal;
      }

      result.m_surface.m_ownerRegion      = from;
      result.m_surface.m_neighborRegion   = to;
      result.m_surface.m_ownerMaterial    = this->getMaterialOf(result.m_surface.m_ownerRegion);
      result.m_surface.m_neighborMaterial = this->getMaterialOf(result.m_surface.m_neighborRegion);

      return result;
    }

    start = first.m_t + nudge;
  }

  return result;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline SurfaceHitT<T>
Tape<T, UserPrimitives...>::firstRegionChange(const Vec3T<T>&      a_from,
                                              const Vec3T<T>&      a_to,
                                              const T              a_tMin,
                                              const RayOptions<T>& a_options) const noexcept
{
  const Vec3T<T> segment = a_to - a_from;
  const T        length  = segment.length();

  if (!(length > T(0))) {
    return SurfaceHitT<T>();
  }

  return this->firstRegionChange(RayT<T>{a_from, segment / length}, a_tMin, length, a_options);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline SurfaceHitT<T>
Tape<T, UserPrimitives...>::firstSurface(const RayT<T>&       a_ray,
                                         const T              a_tMin,
                                         const T              a_tMax,
                                         const Subdomains&    a_subdomains,
                                         const uint32_t       a_subdomain,
                                         const RayOptions<T>& a_options) const noexcept
{
  // Region by region along the ray, until it enters or leaves the subdomain: one region in it, the other not. Each
  // query starts past the crossing before it (see firstRegionChange()).
  T t = a_tMin;

  for (uint32_t steps = 0; steps < a_options.m_maxSteps; steps++) {
    const SurfaceHitT<T> change = this->firstRegionChange(a_ray, t, a_tMax, a_options);

    if (!change.m_hit.hit()) {
      return change;
    }

    const bool wasIn = a_subdomains.subdomainOf(change.m_surface.m_ownerRegion) == a_subdomain;
    const bool isIn  = a_subdomains.subdomainOf(change.m_surface.m_neighborRegion) == a_subdomain;

    if (wasIn != isIn) {
      return change;
    }

    t = change.m_hit.m_t;
  }

  return SurfaceHitT<T>();
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline CSGOps::Tagged<T>
Tape<T, UserPrimitives...>::regionSolidTagged(const uint32_t  a_region,
                                              const uint32_t  a_level,
                                              const Vec3T<T>& a_point) const noexcept
{
  const void* base = this->base();

  const uint32_t record  = m_layout.m_regionPrograms.at(base, a_region);
  const uint32_t masks   = m_layout.m_regionPrograms.at(base, record);
  const uint32_t program = m_layout.m_regionPrograms.at(base, record + 1U + masks + a_level);

  EBGEOMETRY_EXPECT(a_level <= masks);

  return this->template evaluate<MaxCallDepth, CSGOps::Tagged<T>>(program, a_point, base, this->location());
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline SurfaceRef<T>
Tape<T, UserPrimitives...>::closestSurface(const Vec3T<T>& a_point, const uint32_t a_region) const noexcept
{
  const uint32_t n = this->numRegionOperands();

  EBGEOMETRY_EXPECT(a_region <= Math::max(n, 1U));

  // An object with no Overlay is one region: its surface is the object's, between it and the background.
  if (n == 0U) {
    SurfaceRef<T> surface = this->surfaceOf(this->leafOf(
      this->template evaluate<MaxCallDepth, CSGOps::Tagged<T>>(0U, a_point, this->base(), this->location())));

    surface.m_ownerRegion      = a_region;
    surface.m_neighborRegion   = 1U - Math::min(a_region, 1U);
    surface.m_ownerMaterial    = this->getMaterialOf(surface.m_ownerRegion);
    surface.m_neighborMaterial = this->getMaterialOf(surface.m_neighborRegion);

    return surface;
  }

  // The background's surface is the whole object's; across it, the region whose solid comes nearest the point
  // (the later one on a tie).
  if (a_region == 0U) {
    SurfaceRef<T> surface = this->surfaceOf(this->leafOf(
      this->template evaluate<MaxCallDepth, CSGOps::Tagged<T>>(0U, a_point, this->base(), this->location())));

    T        nearest = Math::Limits<T>::max();
    uint32_t across  = 0U;

    for (uint32_t m = n; m > 0U; m--) {
      const T value = this->regionSolid(m, 0U, a_point);

      if (value < nearest) {
        nearest = value;
        across  = m;
      }
    }

    surface.m_ownerRegion      = 0U;
    surface.m_neighborRegion   = across;
    surface.m_ownerMaterial    = this->getMaterialOf(surface.m_ownerRegion);
    surface.m_neighborMaterial = this->getMaterialOf(surface.m_neighborRegion);

    return surface;
  }

  // The region's function, max(S_k, -min_{j>k} S_j), with the leaf each term comes from: the region's own operand
  // or its masks decide, or the later region it meets there.
  using V = CSGOps::Tagged<T>;

  const V own = this->regionSolidTagged(a_region, 0U, a_point);

  V later;

  later.m_value = Math::Limits<T>::max();
  later.m_tag   = CSGOps::NoTag;
  later.m_point = a_point;

  uint32_t met = 0U;

  // On a tie (two later solids sharing a mask's face, which decides both), the later one, which lies on top, where its
  // own operand reaches the point: elsewhere on the face the earlier one shows there.
  for (uint32_t j = a_region + 1U; j <= n; j++) {
    const V value = this->regionSolidTagged(j, this->sharedMasks(a_region, j), a_point);

    const bool tie = (value.m_value == later.m_value) && met != 0U;
    const bool top =
      tie &&
      this->regionSolid(
        j, m_layout.m_regionPrograms.at(this->base(), m_layout.m_regionPrograms.at(this->base(), j)), a_point) < T(0);

    if (value.m_value < later.m_value || top) {
      later = value;
      met   = j;
    }
  }

  const bool byLater = (met != 0U) && (-later.m_value > own.m_value);

  SurfaceRef<T> surface = this->surfaceOf(this->leafOf(byLater ? later : own));

  // Across an interface with a later region: that region, or one later still on top of it there. Across the
  // region's own surface: the last other region that holds the point, or the background.
  uint32_t across = byLater ? met : 0U;

  // Across a mask's cut, though, the regions the mask cuts are gone too: the last region the mask does not cut that
  // holds the point, or the background. The mask that decides is the outermost whose leaving out lowers the value
  // (masks combine by a maximum, so leaving out one that does not decide changes nothing, bit for bit).
  const void*    base   = this->base();
  const uint32_t record = m_layout.m_regionPrograms.at(base, a_region);
  const uint32_t masks  = m_layout.m_regionPrograms.at(base, record);

  uint32_t cutBy = Subdomains::None;

  if (!byLater && masks > 0U) {
    T value = own.m_value;

    for (uint32_t level = 0; level < masks && cutBy == Subdomains::None; level++) {
      const T without = this->regionSolid(a_region, level + 1U, a_point);

      if (without < value) {
        cutBy = m_layout.m_regionPrograms.at(base, record + 1U + level);
      }

      value = without;
    }
  }

  // Whether a region is cut by that mask.
  const auto cut = [&](const uint32_t a_other) {
    const uint32_t other = m_layout.m_regionPrograms.at(base, a_other);
    const uint32_t count = m_layout.m_regionPrograms.at(base, other);

    for (uint32_t i = 0; i < count; i++) {
      if (m_layout.m_regionPrograms.at(base, other + 1U + i) == cutBy) {
        return true;
      }
    }

    return false;
  };

  for (uint32_t m = n; m > (byLater ? met : 0U); m--) {
    if (m != a_region && (cutBy == Subdomains::None || !cut(m)) && this->regionSolid(m, 0U, a_point) < T(0)) {
      across = m;

      break;
    }
  }

  surface.m_ownerRegion      = a_region;
  surface.m_neighborRegion   = across;
  surface.m_ownerMaterial    = this->getMaterialOf(surface.m_ownerRegion);
  surface.m_neighborMaterial = this->getMaterialOf(surface.m_neighborRegion);

  return surface;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline SurfaceRef<T>
Tape<T, UserPrimitives...>::closestSurface(const Vec3T<T>& a_point) const noexcept
{
  return this->surfaceOf(this->closestLeaf(a_point));
}

template <class T, class... UserPrimitives>
inline bool
Tape<T, UserPrimitives...>::leafValue(const uint32_t a_leaf, const Vec3T<T>& a_point, T& a_value) const
{
  // The leaf's primitive clause names its type and its index in the type's array (the region programs emit the
  // same leaves again; the first is as good as any).
  const void* const base = this->base();

  for (uint32_t i = 0; i < m_layout.m_clauses.size(); i++) {
    const TapeDetail::Clause& c = m_layout.m_clauses.at(base, i);

    if (c.m_op == TapeDetail::Op::Primitive && c.m_b == a_leaf) {
      a_value = this->evaluatePrimitive(c.m_type, c.m_a, a_point, base, this->location());

      return true;
    }
  }

  return false;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline LeafRef<T>
Tape<T, UserPrimitives...>::leafOf(const CSGOps::Tagged<T>& a_value) const noexcept
{
  LeafRef<T> ref;

  if (a_value.m_tag != CSGOps::NoTag) {
    const TapeDetail::Clause& c = m_layout.m_clauses.at(this->base(), a_value.m_tag);

    ref.m_leaf       = c.m_b;
    ref.m_type       = c.m_type;
    ref.m_index      = c.m_a;
    ref.m_localPoint = a_value.m_point;
  }

  return ref;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline LeafRef<T>
Tape<T, UserPrimitives...>::tracedLeaf(const uint32_t  a_program,
                                       const uint32_t  a_leaf,
                                       const Vec3T<T>& a_point) const noexcept
{
  using TapeDetail::Op;

  const void* base = this->base();

  const TapeDetail::Program range   = m_layout.m_programs.at(base, a_program);
  const TapeDetail::Clause* clauses = m_layout.m_clauses.data(base);
  const T*                  params  = m_layout.m_params.data(base);

  // The point maps as evaluateBody() applies them, so the leaf's point is the one its evaluation sees.
  Vec3T<T> points[PointStackSize];

  uint32_t np = 0;

  points[np++] = a_point;

  for (uint32_t pc = range.m_begin; pc < range.m_end; pc++) {
    const TapeDetail::Clause& c = clauses[pc];

    if (c.m_op == Op::Primitive && c.m_b == a_leaf) {
      LeafRef<T> ref;

      ref.m_leaf       = c.m_b;
      ref.m_type       = c.m_type;
      ref.m_index      = c.m_a;
      ref.m_localPoint = points[np - 1];

      return ref;
    }

    if (c.m_op == Op::PushAffine) {
      points[np] = CSGOps::affinePoint(params + c.m_a, points[np - 1]);

      np++;
    }
    else if (c.m_op == Op::PushElongate) {
      const T* h = params + c.m_a;

      points[np] = CSGOps::elongatePoint(Vec3T<T>(h[0], h[1], h[2]), points[np - 1]);

      np++;
    }
    else if (c.m_op == Op::PopPoint) {
      np--;
    }
  }

  return this->leafOf(
    this->template evaluate<MaxCallDepth, CSGOps::Tagged<T>>(a_program, a_point, base, this->location()));
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline SurfaceRef<T>
Tape<T, UserPrimitives...>::surfaceOf(const LeafRef<T>& a_ref) const noexcept
{
  const LeafRef<T>& ref = a_ref;

  SurfaceRef<T> surface;

  if (ref.m_leaf == LeafRef<T>::None) {
    return surface;
  }

  surface.m_leaf         = ref.m_leaf;
  surface.m_surfaceLabel = this->getSurfaceLabel(ref.m_leaf);
  surface.m_surface      = this->getFirstSurface(ref.m_leaf);
  surface.m_localPoint   = ref.m_localPoint;

  // A tape whose types include no container and no shape with parts compiles no drilling at all.
  if constexpr (TapeDetail::AnyDrill<T, Primitives>::value) {
    this->visitPrimitive(ref, TapeDetail::Drill<T>{&surface});
  }

  // A part whose face group named it has that label, over the leaf's. The table starts with one entry per leaf:
  // where the leaf's labels begin, or None for a leaf whose parts are not labeled. Only a single shape's parts are
  // (SurfaceLabel() rejects a collection), so the part indexes the leaf's own labels; a label there may itself be None,
  // for a part in no group under an unlabeled leaf.
  if (m_layout.m_partSurfaceLabels.size() > 0) {
    const uint32_t begin = m_layout.m_partSurfaceLabels.at(this->base(), ref.m_leaf);

    if (begin != SurfaceRef<T>::None) {
      EBGEOMETRY_EXPECT(begin + surface.m_part < m_layout.m_partSurfaceLabels.size());

      surface.m_surfaceLabel = SurfaceLabelId(m_layout.m_partSurfaceLabels.at(this->base(), begin + surface.m_part));
    }
  }

  return surface;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getNumSurfaces() const noexcept
{
  const uint32_t n = m_layout.m_surfaces.size();

  return (n > 0) ? m_layout.m_surfaces.at(this->base(), n - 1) : m_layout.m_numPrimitives;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getFirstSurface(const uint32_t a_leaf) const noexcept
{
  if (m_layout.m_surfaces.size() > 0) {
    return (a_leaf < m_layout.m_surfaces.size()) ? m_layout.m_surfaces.at(this->base(), a_leaf) : SurfaceRef<T>::None;
  }

  return (a_leaf <= m_layout.m_numPrimitives) ? a_leaf : SurfaceRef<T>::None;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getCallDepth() const noexcept
{
  return m_layout.m_callDepth;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline const typename Tape<T, UserPrimitives...>::Layout&
Tape<T, UserPrimitives...>::layout() const noexcept
{
  return m_layout;
}

namespace TapeDetail {

/**
 * @brief Run rebasedView()'s checks on every pool-resident primitive of a store.
 * @details Internal helper; not part of the public API. The rebased primitives are discarded: they are
 * stored as layouts and resolve against the tape's location.
 * @tparam I    Type index.
 * @tparam List TypeList of primitive types.
 * @tparam Store PrimitiveStore type.
 * @param[in] a_store    Store.
 * @param[in] a_base     Pool base.
 * @param[in] a_location The tape's location.
 * @param[in] a_pool     Pool being rebased onto.
 */
template <size_t I, class List, class Store>
inline void
checkPrimitivesRebase(const Store& a_store, const void* a_base, const PoolLocation& a_location, const Pool& a_pool)
{
  if constexpr (I < List::size) {
    using P = typename TypeAt<I, List>::type;

    if constexpr (isPoolHandle<P>) {
      for (const auto& stored : storeAt<I>(a_store).bind(a_base)) {
        [[maybe_unused]] const P view = P(stored, a_location).rebasedView(a_pool);
      }
    }

    checkPrimitivesRebase<I + 1, List>(a_store, a_base, a_location, a_pool);
  }
}

/**
 * @brief Copy each primitive type's array out of the pool into staging, deep-copying pool-resident
 * primitives into another pool.
 * @details Internal helper; not part of the public API.
 * @tparam I     Type index.
 * @tparam List  TypeList of primitive types.
 * @tparam Store PrimitiveStore type.
 * @tparam Tuple Staged primitives, one vector per type.
 * @param[in]     a_store    Store.
 * @param[in]     a_location The tape's location.
 * @param[in,out] a_dstPool  Pool the pool-resident primitives are copied into.
 * @param[out]    a_staged   Staged primitives.
 */
template <size_t I, class List, class Store, class Tuple>
inline void
deepCopyStore(const Store& a_store, const PoolLocation& a_location, Pool& a_dstPool, Tuple& a_staged)
{
  if constexpr (I < List::size) {
    using P = typename TypeAt<I, List>::type;

    // Copy the array out before reserving anything: a_dstPool may be the source pool, and a reserve can
    // move its block.
    const auto span = storeAt<I>(a_store).bind(static_cast<const void*>(a_location.base()));

    auto& staged = std::get<I>(a_staged);

    staged.assign(span.begin(), span.end());

    if constexpr (isPoolHandle<P>) {
      for (auto& stored : staged) {
        stored = P(stored, a_location).deepCopy(a_dstPool).layout();
      }
    }

    deepCopyStore<I + 1, List>(a_store, a_location, a_dstPool, a_staged);
  }
}

} // namespace TapeDetail

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST
inline Tape<T, UserPrimitives...>
Tape<T, UserPrimitives...>::rebasedView(const Pool& a_pool) const noexcept
{
  const void*        base     = this->base();
  const PoolLocation location = this->location();

  TapeDetail::checkPrimitivesRebase<0, Primitives>(m_layout.m_primitives, base, location, a_pool);

  for (const auto& parts : m_layout.m_unions.bind(base)) {
    [[maybe_unused]] const UnionRoot bvh       = UnionRoot(parts.m_bvh, location).rebasedView(a_pool);
    [[maybe_unused]] const UnionRoot unbounded = UnionRoot(parts.m_unbounded, location).rebasedView(a_pool);
  }

  const uint64_t endByte = Math::max(
    Math::max(m_layout.m_clauses.endByte(), m_layout.m_programs.endByte()),
    Math::max(
      Math::max(m_layout.m_params.endByte(), m_layout.m_unions.endByte()),
      Math::max(Math::max(Math::max(m_layout.m_surfaceLabels.endByte(), m_layout.m_surfaces.endByte()),
                          Math::max(m_layout.m_partSurfaceLabels.endByte(), m_layout.m_regionPrograms.endByte())),
                TapeDetail::storeEndByte(m_layout.m_primitives))));

  Tape view = *this;

  view.setLocation(this->rebasedLocation(a_pool, endByte, "Tape"));

  return view;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST
inline Tape<T, UserPrimitives...>
Tape<T, UserPrimitives...>::deepCopy(Pool& a_dstPool) const
{
  const PoolLocation location = this->location();

  TapeDetail::Staging<T, BranchingRatio, Primitives> staging;

  // Copy every array out of the source pool before anything is reserved in a_dstPool, which may be the
  // same pool.
  {
    const void* base = this->base();

    const auto clauses  = m_layout.m_clauses.bind(base);
    const auto programs = m_layout.m_programs.bind(base);
    const auto params   = m_layout.m_params.bind(base);
    const auto unions   = m_layout.m_unions.bind(base);
    const auto labels   = m_layout.m_surfaceLabels.bind(base);
    const auto surfaces = m_layout.m_surfaces.bind(base);
    const auto parts    = m_layout.m_partSurfaceLabels.bind(base);
    const auto regions  = m_layout.m_regionPrograms.bind(base);

    // The arrays are copied as they are laid out: the marched ranges overlap the sub-programs.
    staging.m_flatClauses.assign(clauses.begin(), clauses.end());
    staging.m_flatPrograms.assign(programs.begin(), programs.end());
    staging.m_flatNumPrograms = m_layout.m_numPrograms;
    staging.m_flatRayPrograms = m_layout.m_rayPrograms;

    staging.m_params.assign(params.begin(), params.end());
    staging.m_unions.assign(unions.begin(), unions.end());
    staging.m_surfaceLabels.assign(labels.begin(), labels.end());
    staging.m_surfaces.assign(surfaces.begin(), surfaces.end());
    staging.m_partSurfaceLabels.assign(parts.begin(), parts.end());
    staging.m_regionPrograms.assign(regions.begin(), regions.end());
  }

  TapeDetail::deepCopyStore<0, Primitives>(m_layout.m_primitives, location, a_dstPool, staging.m_primitives);

  for (auto& parts : staging.m_unions) {
    parts.m_bvh       = UnionRoot(parts.m_bvh, location).deepCopy(a_dstPool).layout();
    parts.m_unbounded = UnionRoot(parts.m_unbounded, location).deepCopy(a_dstPool).layout();
  }

  staging.m_depth             = {m_layout.m_callDepth};
  staging.m_boundingVolume    = m_layout.m_boundingVolume;
  staging.m_quality           = m_layout.m_quality;
  staging.m_numPrimitives     = m_layout.m_numPrimitives;
  staging.m_largestPlainUnion = m_layout.m_largestPlainUnion;
  staging.m_numLabels         = m_layout.m_numLabels;

  Tape copy(Layout{}, PoolLocation{});

  copy.finalize(a_dstPool, staging);

  return copy;
}

} // namespace EBGeometry

#endif
