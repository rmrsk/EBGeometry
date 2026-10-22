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
#include <cmath>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Blend.hpp"
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
   * @brief Leaves emitted so far; the next leaf's id.
   */
  uint32_t m_numPrimitives = 0;

  /**
   * @brief The clauses of each ray sub-program.
   */
  std::vector<std::vector<Clause>> m_rayPrograms;

  /**
   * @brief How deep each ray sub-program's calls nest.
   */
  std::vector<uint32_t> m_rayDepth;

  /**
   * @brief The ray program's BVH unions, over ray sub-programs.
   */
  std::vector<UnionBVH<T, K>> m_rayUnions;

  /**
   * @brief Whether the ray program needs more stack than the ray evaluator holds (it is then dropped).
   */
  bool m_rayTooDeep = false;

  /**
   * @brief Sub-programs of the expression itself, before the ray program's sphere-traced nodes.
   */
  uint32_t m_numOwnPrograms = 0;

  /**
   * @brief Their clauses.
   */
  uint32_t m_numOwnClauses = 0;
};

/**
 * @brief Compiles an Expr into Staging.
 * @details Internal helper; not part of the public API. Host-only; recursion over the expression tree is
 * fine here, since only the evaluator runs on a device.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor.
 * @tparam List TypeList of primitive types.
 */
template <class T, size_t K, class List>
class Compiler
{
public:
  /**
   * @brief Set up.
   * @param[in,out] a_pool           Pool the BVHs are built in, and that pool-resident primitives must live in.
   * @param[in]     a_options        Compile settings.
   * @param[in]     a_valueStackSize Values a sub-program may hold.
   * @param[in]     a_pointStackSize Points a sub-program may hold.
   * @param[in]     a_rayValueStackSize Span sets a ray sub-program may hold.
   * @param[in,out] a_staging        Where the compiled arrays go.
   */
  Compiler(Pool&                a_pool,
           const TapeOptions&   a_options,
           const uint32_t       a_valueStackSize,
           const uint32_t       a_pointStackSize,
           const uint32_t       a_rayValueStackSize,
           Staging<T, K, List>& a_staging) noexcept
    : m_pool(a_pool),
      m_options(a_options),
      m_valueStackSize(a_valueStackSize),
      m_pointStackSize(a_pointStackSize),
      m_rayValueStackSize(a_rayValueStackSize),
      m_staging(a_staging)
  {}

  /**
   * @brief Compile an expression into a new sub-program.
   * @param[in] a_node Expression.
   * @return The sub-program's index.
   */
  uint32_t
  compileProgram(const ExprDetail::Node<T>& a_node)
  {
    const auto id = static_cast<uint32_t>(m_staging.m_programs.size());

    m_staging.m_programs.emplace_back();
    m_staging.m_depth.push_back(0U);

    // The input point is on the point stack when the sub-program starts.
    State state{id, 0U, 1U, 0U, 1U, false};

    this->emit(state, a_node);

    EBGEOMETRY_EXPECT(state.m_values == 1U && state.m_points == 1U);

    EBGEOMETRY_REQUIRE(state.m_maxValues <= m_valueStackSize,
                       "Tape: the expression needs %u values at once in one sub-program, but the tape holds at "
                       "most %u; operations nest too deeply",
                       unsigned(state.m_maxValues),
                       unsigned(m_valueStackSize));
    EBGEOMETRY_REQUIRE(state.m_maxPoints <= m_pointStackSize,
                       "Tape: the expression nests %u point maps (Translate, Rotate, Elongate, ...) in one "
                       "sub-program, but the tape holds at most %u points",
                       unsigned(state.m_maxPoints - 1U),
                       unsigned(m_pointStackSize - 1U));

    return id;
  }

  /**
   * @brief Compile an expression into a new ray sub-program (see Op::RayPrimitive and the ray operations).
   * @details Leaves are numbered as compileProgram() numbers them, so a ray pass over the whole
   * expression started from leaf 0 gives every leaf its id. A ray sub-program that needs more span sets
   * than the ray evaluator holds sets Staging::m_rayTooDeep.
   * @param[in] a_node Expression.
   * @return The ray sub-program's index.
   */
  uint32_t
  compileRayProgram(const ExprDetail::Node<T>& a_node)
  {
    return this->compileRay(a_node);
  }

  /**
   * @brief Whether a ray program would follow an expression rather than sphere trace it as a whole.
   * @param[in] a_node Expression.
   * @return True if so.
   */
  [[nodiscard]] bool
  followsRays(const ExprDetail::Node<T>& a_node) const
  {
    return this->capable(a_node);
  }

private:
  /**
   * @brief Compile a ray sub-program; see compileRayProgram().
   * @param[in] a_node Expression.
   * @return The ray sub-program's index.
   */
  uint32_t
  compileRay(const ExprDetail::Node<T>& a_node)
  {
    const auto id = static_cast<uint32_t>(m_staging.m_rayPrograms.size());

    m_staging.m_rayPrograms.emplace_back();
    m_staging.m_rayDepth.push_back(0U);

    // The input ray is on the ray stack when the ray sub-program starts.
    State state{id, 0U, 1U, 0U, 1U, true};

    this->emitRay(state, a_node);

    EBGEOMETRY_EXPECT(state.m_values == 1U && state.m_points == 1U);

    if (state.m_maxValues > m_rayValueStackSize || state.m_maxPoints > m_pointStackSize) {
      m_staging.m_rayTooDeep = true;
    }

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

    /**
     * @brief Whether the clauses go to a ray sub-program (whose stacks hold spans and rays).
     */
    bool m_ray;
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
   * @brief Ray evaluator span-set stack capacity.
   */
  uint32_t m_rayValueStackSize;

  /**
   * @brief Output.
   */
  Staging<T, K, List>& m_staging;

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
    if (a_state.m_ray) {
      m_staging.m_rayPrograms[a_state.m_program].push_back(a_clause);
    }
    else {
      m_staging.m_programs[a_state.m_program].push_back(a_clause);
    }

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
   * @return True if it has enough operands and all of them are distances.
   */
  [[nodiscard]] bool
  useBVH(const ExprDetail::Node<T>& a_node) const noexcept
  {
    if (a_node.m_children.size() < m_options.m_bvhUnionThreshold) {
      return false;
    }

    for (const auto& child : a_node.m_children) {
      if (child.getDistanceQuality() == DistanceQuality::NotADistance) {
        return false;
      }
    }

    return true;
  }

  /**
   * @brief Compile each operand of a union into its own sub-program and build a BVH over them.
   * @param[in,out] a_state The caller's state.
   * @param[in]     a_node  The union.
   * @return The BVH union's index.
   */
  uint32_t
  buildUnion(State& a_state, const ExprDetail::Node<T>& a_node)
  {
    std::vector<uint32_t>                  programs;
    std::vector<BoundingVolumes::AABBT<T>> boxes;

    programs.reserve(a_node.m_children.size());
    boxes.reserve(a_node.m_children.size());

    for (const auto& child : a_node.m_children) {
      const uint32_t program = this->compileProgram(child.node());

      this->calls(a_state, program);

      // An empty object (an intersection of disjoint objects, say) has no box to place in the tree; it
      // goes with the unbounded operands, which every query evaluates.
      const BoundingVolumes::AABBT<T>& box = child.computeBoundingVolume();

      programs.push_back(program);
      boxes.push_back(ExprDetail::isEmpty(box) ? ExprDetail::everywhere<T>() : box);
    }

    const auto parts = CSGDetail::buildUnionParts<T, uint32_t, K>(
      m_pool, programs, boxes, m_options.m_construction, CSGDetail::defaultUnionOptions<K>());

    UnionBVH<T, K> bvh;

    bvh.m_bvh       = parts.bvh.layout();
    bvh.m_unbounded = parts.unbounded.layout();

    m_staging.m_unions.push_back(bvh);

    return static_cast<uint32_t>(m_staging.m_unions.size() - 1);
  }

  /**
   * @brief Whether primitive type a_type, from I onwards, has a ray form (see RayForm).
   * @tparam I First type index to test.
   * @param[in] a_type Type index.
   * @return True if it has one.
   */
  template <size_t I>
  static bool
  hasRayForm(const uint32_t a_type) noexcept
  {
    if constexpr (I < NumTypes) {
      if (a_type == I) {
        return RayForm<typename TypeAt<I, List>::type>::available;
      }

      return hasRayForm<I + 1>(a_type);
    }
    else {
      return false;
    }
  }

  /**
   * @brief Whether the ray program follows an expression node by node, rather than sphere tracing it as a
   * whole: a primitive with a ray form, an affine map or complement of such a node, and a union or
   * intersection with at least one such operand (the others are sphere traced one by one).
   * @param[in] a_node Expression.
   * @return True if so.
   */
  [[nodiscard]] bool
  capable(const ExprDetail::Node<T>& a_node) const
  {
    using ExprDetail::Kind;

    switch (a_node.m_kind) {
    case Kind::Primitive: {
      const uint32_t type = findType<0>(a_node.m_leaf.m_type);

      return type < NumTypes && hasRayForm<0>(type);
    }
    case Kind::Affine:
    case Kind::Complement: {
      return this->capable(a_node.m_children.front().node());
    }
    case Kind::Union:
    case Kind::Intersection: {
      for (const auto& child : a_node.m_children) {
        if (this->capable(child.node())) {
          return true;
        }
      }

      return false;
    }
    default: {
      return false;
    }
    }
  }

  /**
   * @brief Record that a ray sub-program calls another.
   * @param[in] a_state  The caller's state.
   * @param[in] a_callee The callee.
   */
  void
  rayCalls(const State& a_state, const uint32_t a_callee)
  {
    uint32_t& depth = m_staging.m_rayDepth[a_state.m_program];

    depth = Math::max(depth, 1U + m_staging.m_rayDepth[a_callee]);
  }

  /**
   * @brief Compile each operand of a union into its own ray sub-program and build a BVH over them.
   * @param[in,out] a_state The caller's state.
   * @param[in]     a_node  The union.
   * @return The ray BVH union's index.
   */
  uint32_t
  buildRayUnion(State& a_state, const ExprDetail::Node<T>& a_node)
  {
    std::vector<uint32_t>                  programs;
    std::vector<BoundingVolumes::AABBT<T>> boxes;

    programs.reserve(a_node.m_children.size());
    boxes.reserve(a_node.m_children.size());

    for (const auto& child : a_node.m_children) {
      const uint32_t program = this->compileRay(child.node());

      this->rayCalls(a_state, program);

      const BoundingVolumes::AABBT<T>& box = child.computeBoundingVolume();

      programs.push_back(program);
      boxes.push_back(ExprDetail::isEmpty(box) ? ExprDetail::everywhere<T>() : box);
    }

    const auto parts = CSGDetail::buildUnionParts<T, uint32_t, K>(
      m_pool, programs, boxes, m_options.m_construction, CSGDetail::defaultUnionOptions<K>());

    UnionBVH<T, K> bvh;

    bvh.m_bvh       = parts.bvh.layout();
    bvh.m_unbounded = parts.unbounded.layout();

    m_staging.m_rayUnions.push_back(bvh);

    return static_cast<uint32_t>(m_staging.m_rayUnions.size() - 1);
  }

  /**
   * @brief Emit the clauses that push an expression's spans along the top ray.
   * @param[in,out] a_state Ray sub-program state.
   * @param[in]     a_node  Expression.
   */
  void
  emitRay(State& a_state, const ExprDetail::Node<T>& a_node)
  {
    using ExprDetail::Kind;

    // A node with no ray form is compiled into a scalar sub-program, which the ray evaluator sphere
    // traces along the ray inside the node's box.
    if (!this->capable(a_node)) {
      const BoundingVolumes::AABBT<T>& box = a_node.m_boundingVolume;

      const uint32_t sub = this->compileProgram(a_node);
      const uint32_t p   = this->params({box.getLowCorner()[0],
                                         box.getLowCorner()[1],
                                         box.getLowCorner()[2],
                                         box.getHighCorner()[0],
                                         box.getHighCorner()[1],
                                         box.getHighCorner()[2]});

      this->push(a_state, clause(Op::RayFallback, sub, p, 0U, static_cast<uint8_t>(a_node.m_quality)), 1, 0);

      return;
    }

    switch (a_node.m_kind) {
    case Kind::Primitive: {
      const ExprDetail::Leaf& leaf = a_node.m_leaf;
      const uint32_t          type = findType<0>(leaf.m_type);

      const uint32_t index = this->stage<0>(type, leaf.m_object.get());
      const uint32_t id    = m_staging.m_numPrimitives++;

      this->push(a_state, clause(Op::RayPrimitive, index, id, 0U, 0U, static_cast<uint16_t>(type)), 1, 0);

      break;
    }
    case Kind::Affine: {
      const Affine3T<T>& m = a_node.m_map;

      const uint32_t p = this->params({m.m_row0[0],
                                       m.m_row0[1],
                                       m.m_row0[2],
                                       m.m_row1[0],
                                       m.m_row1[1],
                                       m.m_row1[2],
                                       m.m_row2[0],
                                       m.m_row2[1],
                                       m.m_row2[2],
                                       m.m_shift[0],
                                       m.m_shift[1],
                                       m.m_shift[2]});

      this->push(a_state, clause(Op::RayPushAffine, p), 0, 1);
      this->emitRay(a_state, a_node.m_children.front().node());
      this->push(a_state, clause(Op::RayPopRay), 0, -1);

      break;
    }
    case Kind::Complement: {
      this->emitRay(a_state, a_node.m_children.front().node());
      this->push(a_state, clause(Op::RayComplement), 0, 0);

      break;
    }
    case Kind::Union:
    case Kind::Intersection: {
      if (a_node.m_kind == Kind::Union && this->useBVH(a_node)) {
        this->push(a_state, clause(Op::RayUnionBVH, this->buildRayUnion(a_state, a_node)), 1, 0);
      }
      else {
        const Op op = (a_node.m_kind == Kind::Union) ? Op::RayUnite : Op::RayIntersect;

        this->emitRay(a_state, a_node.m_children.front().node());

        for (size_t i = 1; i < a_node.m_children.size(); i++) {
          this->emitRay(a_state, a_node.m_children[i].node());
          this->push(a_state, clause(op), -1, 0);
        }
      }

      break;
    }
    default: {
      EBGEOMETRY_EXPECT(false && "Tape: a node the ray program cannot follow");

      break;
    }
    }
  }

  /**
   * @brief Emit the clauses that push an expression's value at the top point.
   * @param[in,out] a_state Sub-program state.
   * @param[in]     a_node  Expression.
   */
  void
  emit(State& a_state, const ExprDetail::Node<T>& a_node)
  {
    using ExprDetail::Kind;

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

      // Leaves are emitted in the order they appear in the expression, depth first, so this numbers them.
      const uint32_t id = m_staging.m_numPrimitives++;

      this->push(a_state, clause(Op::Primitive, index, id, 0U, 0U, static_cast<uint16_t>(type)), 1, 0);

      break;
    }
    case Kind::Affine: {
      const Affine3T<T>& m = a_node.m_map;

      const uint32_t p = this->params({m.m_row0[0],
                                       m.m_row0[1],
                                       m.m_row0[2],
                                       m.m_row1[0],
                                       m.m_row1[1],
                                       m.m_row1[2],
                                       m.m_row2[0],
                                       m.m_row2[1],
                                       m.m_row2[2],
                                       m.m_shift[0],
                                       m.m_shift[1],
                                       m.m_shift[2]});

      this->push(a_state, clause(Op::PushAffine, p), 0, 1);
      this->emit(a_state, a_node.m_children.front().node());
      this->push(a_state, clause(Op::PopPoint), 0, -1);

      if (a_node.m_valueScale != T(1)) {
        this->push(a_state, clause(Op::MulConstant, this->params({a_node.m_valueScale})), 0, 0);
      }

      break;
    }
    case Kind::Elongate: {
      const uint32_t p = this->params({a_node.m_vec0[0], a_node.m_vec0[1], a_node.m_vec0[2]});

      this->push(a_state, clause(Op::PushElongate, p), 0, 1);
      this->emit(a_state, a_node.m_children.front().node());
      this->push(a_state, clause(Op::PopPoint), 0, -1);

      break;
    }
    case Kind::Repeat: {
      const uint32_t p = this->params({a_node.m_vec0[0],
                                       a_node.m_vec0[1],
                                       a_node.m_vec0[2],
                                       a_node.m_vec1[0],
                                       a_node.m_vec1[1],
                                       a_node.m_vec1[2],
                                       a_node.m_vec2[0],
                                       a_node.m_vec2[1],
                                       a_node.m_vec2[2]});

      this->push(a_state, clause(Op::PushRepeat, p), 0, 1);
      this->emit(a_state, a_node.m_children.front().node());
      this->push(a_state, clause(Op::PopPoint), 0, -1);

      break;
    }
    case Kind::Complement: {
      this->emit(a_state, a_node.m_children.front().node());
      this->push(a_state, clause(Op::Negate), 0, 0);

      break;
    }
    case Kind::Offset: {
      this->emit(a_state, a_node.m_children.front().node());
      this->push(a_state, clause(Op::AddConstant, this->params({-a_node.m_scalar0})), 0, 0);

      break;
    }
    case Kind::Annular: {
      this->emit(a_state, a_node.m_children.front().node());
      this->push(a_state, clause(Op::Abs), 0, 0);
      this->push(a_state, clause(Op::AddConstant, this->params({-a_node.m_scalar0})), 0, 0);

      break;
    }
    case Kind::Blur: {
      const T alpha = a_node.m_scalar1;
      const T beta  = (T(1) - alpha) / T(2);

      // The weight of a tap m steps from the centre is alpha^(3-m) beta^m; the 27 weights sum to
      // (alpha + 2 beta)^3 = 1.
      const uint32_t p = this->params({a_node.m_scalar0,
                                       std::pow(alpha, T(3)) * std::pow(beta, T(0)),
                                       std::pow(alpha, T(2)) * std::pow(beta, T(1)),
                                       std::pow(alpha, T(1)) * std::pow(beta, T(2)),
                                       std::pow(alpha, T(0)) * std::pow(beta, T(3))});

      const uint32_t sub = this->compileProgram(a_node.m_children.front().node());

      this->calls(a_state, sub);
      this->push(a_state, clause(Op::Blur, sub, p), 1, 0);

      break;
    }
    case Kind::Mollify: {
      // Sample the bump kernel max(0, 1 - |x|^2 / (2r)^2)^2 on an n x n x n grid spanning [-r, r]^3 and
      // normalize the weights to sum to one. The kernel's support has radius 2r, so every sample, the
      // cube's corners included, gets a positive weight.
      const T        r  = a_node.m_scalar0;
      const uint32_t n  = a_node.m_count;
      const T        dX = T(2) * r / T(n - 1);

      std::vector<T> taps;

      taps.reserve(4U * n * n * n);

      T sum = T(0);

      for (uint32_t i = 0; i < n; i++) {
        for (uint32_t j = 0; j < n; j++) {
          for (uint32_t k = 0; k < n; k++) {
            const Vec3T<T> offset(-r + T(i) * dX, -r + T(j) * dX, -r + T(k) * dX);

            const T t = Math::max(T(0), T(1) - offset.length2() / (T(4) * r * r));
            const T w = t * t;

            taps.insert(taps.end(), {offset[0], offset[1], offset[2], w});

            sum += w;
          }
        }
      }

      for (size_t tap = 0; tap < taps.size(); tap += 4) {
        taps[tap + 3] /= sum;
      }

      const uint32_t p   = this->params(taps);
      const uint32_t sub = this->compileProgram(a_node.m_children.front().node());

      this->calls(a_state, sub);
      this->push(a_state, clause(Op::Mollify, sub, p, n * n * n), 1, 0);

      break;
    }
    case Kind::Union:
    case Kind::Intersection: {
      if (a_node.m_kind == Kind::Union && this->useBVH(a_node)) {
        this->push(a_state, clause(Op::UnionBVH, this->buildUnion(a_state, a_node)), 1, 0);
      }
      else {
        const Op op = (a_node.m_kind == Kind::Union) ? Op::Min : Op::Max;

        this->emit(a_state, a_node.m_children.front().node());

        for (size_t i = 1; i < a_node.m_children.size(); i++) {
          this->emit(a_state, a_node.m_children[i].node());
          this->push(a_state, clause(op), -1, 0);
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
        const uint32_t bvh = this->buildUnion(a_state, a_node);

        this->push(a_state, clause(Op::SmoothUnionBVH, bvh, s, 0U, exponential ? 1U : 0U), 1, 0);
      }
      else {
        // Only the two extreme values are blended, so they are tracked as the operands are evaluated.
        this->push(a_state, clause(Op::PairInit, 0U, 0U, 0U, maxFlag), 2, 0);

        for (const auto& child : a_node.m_children) {
          this->emit(a_state, child.node());
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

} // namespace TapeDetail

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST
Tape<T, UserPrimitives...>::Tape(Pool& a_pool, const Expr<T>& a_expr, const TapeOptions& a_options) : PoolHandle()
{
  EBGEOMETRY_REQUIRE(a_options.m_bvhUnionThreshold >= 2U,
                     "Tape: the BVH union threshold must be at least 2 (%u)",
                     unsigned(a_options.m_bvhUnionThreshold));

  TapeDetail::Staging<T, BranchingRatio, Primitives> staging;

  TapeDetail::Compiler<T, BranchingRatio, Primitives> compiler(
    a_pool, a_options, ValueStackSize, PointStackSize, RayValueStackSize, staging);

  [[maybe_unused]] const uint32_t root = compiler.compileProgram(a_expr.node());

  EBGEOMETRY_EXPECT(root == 0U);

  EBGEOMETRY_REQUIRE(staging.m_depth.front() <= MaxCallDepth,
                     "Tape: Blur, Mollify and BVH unions nest %u deep, but a tape allows at most %u",
                     unsigned(staging.m_depth.front()),
                     unsigned(MaxCallDepth));

  // The ray program walks the expression again, numbering the leaves from 0 as the scalar pass did. One
  // that needs more stack or deeper calls than the ray evaluator allows is dropped: firstHit() then
  // sphere traces the whole object.
  staging.m_numOwnPrograms = static_cast<uint32_t>(staging.m_programs.size());
  staging.m_numOwnClauses  = 0U;

  for (const auto& program : staging.m_programs) {
    staging.m_numOwnClauses += static_cast<uint32_t>(program.size());
  }

  // An object with no node a ray can follow is sphere traced as a whole, with no ray program.
  if (compiler.followsRays(a_expr.node())) {
    const uint32_t leaves = staging.m_numPrimitives;

    staging.m_numPrimitives = 0U;

    [[maybe_unused]] const uint32_t rayRoot = compiler.compileRayProgram(a_expr.node());

    EBGEOMETRY_EXPECT(rayRoot == 0U);
    EBGEOMETRY_EXPECT(staging.m_numPrimitives == leaves);

    staging.m_numPrimitives = leaves;

    if (staging.m_rayTooDeep || staging.m_rayDepth.front() > MaxCallDepth) {
      staging.m_rayPrograms.clear();
      staging.m_rayUnions.clear();
    }
  }

  staging.m_boundingVolume = a_expr.computeBoundingVolume();
  staging.m_quality        = a_expr.getDistanceQuality();

  this->finalize(a_pool, staging);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST
void
Tape<T, UserPrimitives...>::finalize(Pool& a_pool, const TapeDetail::Staging<T, BranchingRatio, Primitives>& a_staging)
{
  // Reserve everything first: a reserve may move the pool's block, so the base is resolved only once
  // nothing more is reserved.
  std::vector<TapeDetail::Clause>  clauses;
  std::vector<TapeDetail::Program> programs;

  for (const auto& program : a_staging.m_programs) {
    TapeDetail::Program range;

    range.m_begin = static_cast<uint32_t>(clauses.size());

    clauses.insert(clauses.end(), program.begin(), program.end());

    range.m_end = static_cast<uint32_t>(clauses.size());

    programs.push_back(range);
  }

  std::vector<TapeDetail::Clause>  rayClauses;
  std::vector<TapeDetail::Program> rayPrograms;

  for (const auto& program : a_staging.m_rayPrograms) {
    TapeDetail::Program range;

    range.m_begin = static_cast<uint32_t>(rayClauses.size());

    rayClauses.insert(rayClauses.end(), program.begin(), program.end());

    range.m_end = static_cast<uint32_t>(rayClauses.size());

    rayPrograms.push_back(range);
  }

  Layout layout;

  layout.m_rayClauses.reserveFrom(a_pool, static_cast<uint32_t>(rayClauses.size()));
  layout.m_rayPrograms.reserveFrom(a_pool, static_cast<uint32_t>(rayPrograms.size()));
  layout.m_rayUnions.reserveFrom(a_pool, static_cast<uint32_t>(a_staging.m_rayUnions.size()));
  layout.m_clauses.reserveFrom(a_pool, static_cast<uint32_t>(clauses.size()));
  layout.m_programs.reserveFrom(a_pool, static_cast<uint32_t>(programs.size()));
  layout.m_params.reserveFrom(a_pool, static_cast<uint32_t>(a_staging.m_params.size()));
  layout.m_unions.reserveFrom(a_pool, static_cast<uint32_t>(a_staging.m_unions.size()));

  TapeDetail::reserveStore<0>(a_pool, layout.m_primitives, a_staging.m_primitives);

  this->attach(a_pool, "Tape");

  void* base = this->base();

  layout.m_clauses.assign(base, clauses.data(), static_cast<uint32_t>(clauses.size()));
  layout.m_programs.assign(base, programs.data(), static_cast<uint32_t>(programs.size()));
  layout.m_params.assign(base, a_staging.m_params.data(), static_cast<uint32_t>(a_staging.m_params.size()));
  layout.m_unions.assign(base, a_staging.m_unions.data(), static_cast<uint32_t>(a_staging.m_unions.size()));
  layout.m_rayClauses.assign(base, rayClauses.data(), static_cast<uint32_t>(rayClauses.size()));
  layout.m_rayPrograms.assign(base, rayPrograms.data(), static_cast<uint32_t>(rayPrograms.size()));
  layout.m_rayUnions.assign(base, a_staging.m_rayUnions.data(), static_cast<uint32_t>(a_staging.m_rayUnions.size()));

  TapeDetail::assignStore<0>(base, layout.m_primitives, a_staging.m_primitives);

  layout.m_boundingVolume = a_staging.m_boundingVolume;
  layout.m_quality        = a_staging.m_quality;
  layout.m_callDepth      = a_staging.m_depth.front();
  layout.m_numPrimitives  = a_staging.m_numPrimitives;
  layout.m_numOwnPrograms = a_staging.m_numOwnPrograms;
  layout.m_numOwnClauses  = a_staging.m_numOwnClauses;

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

  return this->template evaluate<MaxCallDepth, false>(0U, a_point, this->base(), this->location());
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::primitiveAt(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return this->template evaluate<MaxCallDepth, true>(0U, a_point, this->base(), this->location()).m_primitive;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline HitT<T>
Tape<T, UserPrimitives...>::firstHit(const RayT<T>&       a_ray,
                                     const T              a_tMin,
                                     const T              a_tMax,
                                     const RayOptions<T>& a_options) const noexcept
{
  // Without a ray program (one too deep for the ray evaluator), the whole object is sphere traced.
  if (m_layout.m_rayPrograms.size() == 0U) {
    return RayDetail::march(*this, a_ray, a_tMin, a_tMax, a_options);
  }

  HitT<T> hit;

  // Only the part of the ray inside the object's box (widened by the tolerance) can cross its surface.
  const BV&      box = m_layout.m_boundingVolume;
  const T        eps = a_options.m_tolerance * RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);
  const Vec3T<T> pad = eps * Vec3T<T>::ones();

  T t0 = a_tMin;
  T t1 = a_tMax;

  if (!(a_tMin <= a_tMax) || !RayDetail::clip(box.getLowCorner() - pad, box.getHighCorner() + pad, a_ray, t0, t1)) {
    return hit;
  }

  Spans spans;

  this->template rayEvaluate<MaxCallDepth>(
    0U, a_ray.m_origin, a_ray.m_direction, t0, t1, a_options, this->base(), this->location(), spans);

  hit = RayDetail::firstOf(spans, a_tMin);

  if (hit.m_status != HitStatus::Miss) {
    hit.m_point = a_ray.at(hit.hit() ? hit.m_t : t1);
  }

  return hit;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline T
Tape<T, UserPrimitives...>::Fallback::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  return m_tape->template evaluate<MaxCallDepth, false>(m_program, a_point, m_base, *m_location);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline typename Tape<T, UserPrimitives...>::BV
Tape<T, UserPrimitives...>::Fallback::computeBoundingVolume() const noexcept
{
  return m_box;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline DistanceQuality
Tape<T, UserPrimitives...>::Fallback::getDistanceQuality() const noexcept
{
  return m_quality;
}

template <class T, class... UserPrimitives>
template <unsigned D>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline void
Tape<T, UserPrimitives...>::rayEvaluate(const uint32_t       a_program,
                                        const Vec3T<T>&      a_origin,
                                        const Vec3T<T>&      a_dir,
                                        const T              a_tMin,
                                        const T              a_tMax,
                                        const RayOptions<T>& a_options,
                                        const void*          a_base,
                                        const PoolLocation&  a_location,
                                        Spans&               a_spans) const noexcept
{
  using TapeDetail::Op;

  const TapeDetail::Program range   = m_layout.m_rayPrograms.at(a_base, a_program);
  const TapeDetail::Clause* clauses = m_layout.m_rayClauses.data(a_base);
  const T*                  params  = m_layout.m_params.data(a_base);

  // A stack of span sets, and a stack of rays. A mapped ray keeps the parameter t of the ray it was
  // mapped from (its direction is not of unit length), so every span set on the stack is in the same t.
  Spans    values[RayValueStackSize];
  Vec3T<T> origins[PointStackSize];
  Vec3T<T> dirs[PointStackSize];

  uint32_t nv = 0;
  uint32_t np = 0;

  origins[np] = a_origin;
  dirs[np]    = a_dir;

  np++;

  // The spans of a shape along the top ray: traced along the ray scaled to unit length, then scaled back.
  const auto along = [&](const auto& a_shape, Spans& a_out) noexcept {
    const T length = dirs[np - 1].length();

    if (!(length > T(0))) {
      a_out = Spans();

      return;
    }

    const RayT<T> unitRay{origins[np - 1], dirs[np - 1] / length};

    RayDetail::spansOf(a_shape, unitRay, a_tMin * length, a_tMax * length, a_options, a_out);

    for (uint32_t i = 0; i < a_out.m_count; i++) {
      a_out.m_spans[i].m_in /= length;
      a_out.m_spans[i].m_out /= length;
    }
  };

  for (uint32_t pc = range.m_begin; pc < range.m_end; pc++) {
    const TapeDetail::Clause& c = clauses[pc];

    switch (c.m_op) {
    case Op::RayPrimitive: {
      const T length = dirs[np - 1].length();

      Spans& out = values[nv++];

      out = Spans();

      if (length > T(0)) {
        const RayT<T> unitRay{origins[np - 1], dirs[np - 1] / length};

        this->rayPrimitive(
          c.m_type, c.m_a, unitRay, a_tMin * length, a_tMax * length, a_options, a_base, a_location, out);

        // The ends are named by the leaf, not by what the primitive names (a mesh's face).
        for (uint32_t i = 0; i < out.m_count; i++) {
          RaySpan<T>& span = out.m_spans[i];

          span.m_in /= length;
          span.m_out /= length;

          span.m_inPrimitive  = (span.m_inPrimitive == RaySpan<T>::Open) ? RaySpan<T>::Open : c.m_b;
          span.m_outPrimitive = (span.m_outPrimitive == RaySpan<T>::Open) ? RaySpan<T>::Open : c.m_b;
        }
      }

      break;
    }
    case Op::RayPushAffine: {
      const T*        m = params + c.m_a;
      const Vec3T<T>& o = origins[np - 1];
      const Vec3T<T>& d = dirs[np - 1];

      origins[np] = Vec3T<T>(m[0] * o[0] + m[1] * o[1] + m[2] * o[2] + m[9],
                             m[3] * o[0] + m[4] * o[1] + m[5] * o[2] + m[10],
                             m[6] * o[0] + m[7] * o[1] + m[8] * o[2] + m[11]);
      dirs[np]    = Vec3T<T>(m[0] * d[0] + m[1] * d[1] + m[2] * d[2],
                          m[3] * d[0] + m[4] * d[1] + m[5] * d[2],
                          m[6] * d[0] + m[7] * d[1] + m[8] * d[2]);

      np++;

      break;
    }
    case Op::RayPopRay: {
      np--;

      break;
    }
    case Op::RayComplement: {
      Spans out;

      RayDetail::complement(values[nv - 1], a_tMin, a_tMax, out);

      values[nv - 1] = out;

      break;
    }
    case Op::RayUnite:
    case Op::RayIntersect: {
      nv--;

      Spans out;

      if (c.m_op == Op::RayUnite) {
        RayDetail::unite(values[nv - 1], values[nv], out);
      }
      else {
        RayDetail::intersect(values[nv - 1], values[nv], out);
      }

      values[nv - 1] = out;

      break;
    }
    case Op::RayUnionBVH: {
      Spans& acc = values[nv++];

      acc = Spans();

      if constexpr (D > 0) {
        const TapeDetail::UnionBVH<T, BranchingRatio>& parts = m_layout.m_rayUnions.at(a_base, c.m_a);

        const UnionRoot bvh(parts.m_bvh, a_location);
        const UnionRoot unbounded(parts.m_unbounded, a_location);

        const Vec3T<T> o = origins[np - 1];
        const Vec3T<T> d = dirs[np - 1];

        // Add one operand's spans to the union.
        const auto add =
          [this, &acc, &o, &d, a_tMin, a_tMax, &a_options, a_base, &a_location](const uint32_t a_operand) noexcept {
            Spans operand;
            Spans merged;

            this->template rayEvaluate<D - 1>(a_operand, o, d, a_tMin, a_tMax, a_options, a_base, a_location, operand);

            RayDetail::unite(acc, operand, merged);

            acc = merged;
          };

        const auto unboundedOperands = unbounded.getPrimitives();
        const auto operands          = bvh.getPrimitives();

        for (uint32_t i = 0; i < unboundedOperands.size(); i++) {
          add(unboundedOperands[i]);
        }

        // Every operand whose box the ray meets in [tMin, tMax].
        const RayT<T> ray{o, d};

        const auto entry = [&bvh, &ray, a_tMin, a_tMax](const auto& a_node) noexcept -> T {
          const auto box = bvh.getBoundingVolume(a_node);

          T t0 = a_tMin;
          T t1 = a_tMax;

          return RayDetail::clip(box.getLowCorner(), box.getHighCorner(), ray, t0, t1) ? t0
                                                                                       : Math::Limits<T>::infinity();
        };

        const auto leaf = [&operands,
                           &add](const auto& /*a_primitives*/, const size_t a_offset, const size_t a_count) noexcept {
          for (size_t i = a_offset; i < a_offset + a_count; i++) {
            add(operands[static_cast<uint32_t>(i)]);
          }
        };

        const auto prune = [a_tMax](const auto& /*a_node*/, const T& a_key) noexcept -> bool {
          return a_key <= a_tMax;
        };
        const auto order = [](auto& /*a_children*/) noexcept {};

        bvh.template traverse<T>(leaf, prune, order, entry);
      }
      else {
        EBGEOMETRY_EXPECT(false);
      }

      break;
    }
    case Op::RayFallback: {
      const T* b = params + c.m_b;

      const Fallback fallback{this,
                              c.m_a,
                              a_base,
                              &a_location,
                              BV(Vec3T<T>(b[0], b[1], b[2]), Vec3T<T>(b[3], b[4], b[5])),
                              static_cast<DistanceQuality>(c.m_flags)};

      along(fallback, values[nv++]);

      break;
    }
    default: {
      EBGEOMETRY_EXPECT(false && "Tape: unknown ray clause");

      break;
    }
    }
  }

  EBGEOMETRY_EXPECT(nv == 1U && np == 1U);

  a_spans = values[0];
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline void
Tape<T, UserPrimitives...>::rayPrimitive(const uint32_t       a_type,
                                         const uint32_t       a_index,
                                         const RayT<T>&       a_ray,
                                         const T              a_tMin,
                                         const T              a_tMax,
                                         const RayOptions<T>& a_options,
                                         const void*          a_base,
                                         const PoolLocation&  a_location,
                                         Spans&               a_spans) const noexcept
{
  this->template dispatchRayPrimitive<0>(
    a_type, a_index, a_ray, a_tMin, a_tMax, a_options, a_base, a_location, a_spans);
}

template <class T, class... UserPrimitives>
template <size_t I>
EBGEOMETRY_HOST_DEVICE
inline void
Tape<T, UserPrimitives...>::dispatchRayPrimitive(const uint32_t       a_type,
                                                 const uint32_t       a_index,
                                                 const RayT<T>&       a_ray,
                                                 const T              a_tMin,
                                                 const T              a_tMax,
                                                 const RayOptions<T>& a_options,
                                                 const void*          a_base,
                                                 const PoolLocation&  a_location,
                                                 Spans&               a_spans) const noexcept
{
  if constexpr (I < Primitives::size) {
    if (a_type == I) {
      using P = typename TapeDetail::TypeAt<I, Primitives>::type;

      const auto& stored = TapeDetail::storeAt<I>(m_layout.m_primitives).at(a_base, a_index);

      // Only types with a ray form get a ray clause; the others are compiled into fallbacks.
      if constexpr (RayForm<P>::available) {
        if constexpr (isPoolHandle<P>) {
          RayDetail::spansOf(P(stored, a_location), a_ray, a_tMin, a_tMax, a_options, a_spans);
        }
        else {
          RayDetail::spansOf(stored, a_ray, a_tMin, a_tMax, a_options, a_spans);
        }
      }
      else {
        EBGEOMETRY_EXPECT(false);

        a_spans = Spans();
      }

      return;
    }

    this->template dispatchRayPrimitive<I + 1>(
      a_type, a_index, a_ray, a_tMin, a_tMax, a_options, a_base, a_location, a_spans);
  }
  else {
    EBGEOMETRY_EXPECT(false);

    a_spans = Spans();
  }
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline HitStatus
Tape<T, UserPrimitives...>::intersects(const Vec3T<T>&      a_from,
                                       const Vec3T<T>&      a_to,
                                       const RayOptions<T>& a_options) const noexcept
{
  return EBGeometry::intersects(*this, a_from, a_to, a_options);
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getNumPrimitives() const noexcept
{
  return m_layout.m_numPrimitives;
}

template <class T, class... UserPrimitives>
template <unsigned D, bool Track>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline typename Tape<T, UserPrimitives...>::template Result<Track>
Tape<T, UserPrimitives...>::evaluate(const uint32_t      a_program,
                                     const Vec3T<T>&     a_point,
                                     const void*         a_base,
                                     const PoolLocation& a_location) const noexcept
{
  using TapeDetail::Op;

  const TapeDetail::Program range   = m_layout.m_programs.at(a_base, a_program);
  const TapeDetail::Clause* clauses = m_layout.m_clauses.data(a_base);
  const T*                  params  = m_layout.m_params.data(a_base);

  T        values[ValueStackSize];
  Vec3T<T> points[PointStackSize];

  // The leaf of each value, kept only when tracking: without it this array is never touched.
  uint32_t ids[Track ? ValueStackSize : 1U];

  uint32_t nv = 0;
  uint32_t np = 0;

  points[np++] = a_point;

  for (uint32_t pc = range.m_begin; pc < range.m_end; pc++) {
    const TapeDetail::Clause& c = clauses[pc];

    switch (c.m_op) {
    case Op::Primitive: {
      if constexpr (Track) {
        ids[nv] = c.m_b;
      }

      values[nv++] = this->evaluatePrimitive(c.m_type, c.m_a, points[np - 1], a_base, a_location);

      EBGEOMETRY_EXPECT(!std::isnan(values[nv - 1]));

      break;
    }
    case Op::PushAffine: {
      const T*        m = params + c.m_a;
      const Vec3T<T>& x = points[np - 1];

      points[np] = Vec3T<T>(m[0] * x[0] + m[1] * x[1] + m[2] * x[2] + m[9],
                            m[3] * x[0] + m[4] * x[1] + m[5] * x[2] + m[10],
                            m[6] * x[0] + m[7] * x[1] + m[8] * x[2] + m[11]);

      np++;

      break;
    }
    case Op::PushElongate: {
      const T*        h = params + c.m_a;
      const Vec3T<T>& x = points[np - 1];

      points[np] = x - clamp(x, Vec3T<T>(-h[0], -h[1], -h[2]), Vec3T<T>(h[0], h[1], h[2]));

      np++;

      break;
    }
    case Op::PushRepeat: {
      const T*        r = params + c.m_a;
      const Vec3T<T>& x = points[np - 1];

      Vec3T<T> q;

      for (size_t i = 0; i < 3; i++) {
        q[i] = x[i] - r[i] * std::round(Math::min(Math::max(x[i] / r[i], -r[3 + i]), r[6 + i]));
      }

      points[np] = q;

      np++;

      break;
    }
    case Op::PopPoint: {
      np--;

      break;
    }
    case Op::Negate: {
      values[nv - 1] = -values[nv - 1];

      break;
    }
    case Op::Abs: {
      values[nv - 1] = std::abs(values[nv - 1]);

      break;
    }
    case Op::AddConstant: {
      values[nv - 1] += params[c.m_a];

      break;
    }
    case Op::MulConstant: {
      values[nv - 1] *= params[c.m_a];

      break;
    }
    case Op::Min: {
      nv--;

      if constexpr (Track) {
        if (values[nv] < values[nv - 1]) {
          ids[nv - 1] = ids[nv];
        }
      }

      values[nv - 1] = Math::min(values[nv - 1], values[nv]);

      break;
    }
    case Op::Max: {
      nv--;

      if constexpr (Track) {
        if (values[nv] > values[nv - 1]) {
          ids[nv - 1] = ids[nv];
        }
      }

      values[nv - 1] = Math::max(values[nv - 1], values[nv]);

      break;
    }
    case Op::PairInit: {
      const T empty = (c.m_flags == 0U) ? Math::Limits<T>::infinity() : -Math::Limits<T>::infinity();

      if constexpr (Track) {
        ids[nv]      = NoPrimitive;
        ids[nv + 1U] = NoPrimitive;
      }

      values[nv++] = empty;
      values[nv++] = empty;

      break;
    }
    case Op::PairInsert: {
      const T v = values[--nv];

      T& a = values[nv - 2];
      T& b = values[nv - 1];

      // a is the most extreme value so far and b the next.
      const bool beyondA = (c.m_flags == 0U) ? (v < a) : (v > a);
      const bool beyondB = (c.m_flags == 0U) ? (v < b) : (v > b);

      if (beyondA) {
        b = a;
        a = v;

        if constexpr (Track) {
          ids[nv - 1] = ids[nv - 2];
          ids[nv - 2] = ids[nv];
        }
      }
      else if (beyondB) {
        b = v;

        if constexpr (Track) {
          ids[nv - 1] = ids[nv];
        }
      }

      break;
    }
    case Op::PairBlend: {
      const T b = values[--nv];
      const T a = values[nv - 1];
      const T s = params[c.m_a];

      // The more extreme of the two decides: the smaller for a smooth minimum, the larger for a maximum.
      if constexpr (Track) {
        const bool max = (c.m_flags & 1U) != 0U;

        if (max ? (b > a) : (b < a)) {
          ids[nv - 1] = ids[nv];
        }
      }

      switch (c.m_flags) {
      case 0U: {
        values[nv - 1] = SmoothMinOp<T>{}(a, b, s);

        break;
      }
      case 1U: {
        values[nv - 1] = SmoothMaxOp<T>{}(a, b, s);

        break;
      }
      case 2U: {
        values[nv - 1] = ExpMinOp<T>{}(a, b, s);

        break;
      }
      default: {
        values[nv - 1] = ExpMaxOp<T>{}(a, b, s);

        break;
      }
      }

      break;
    }
    case Op::Blur: {
      if constexpr (D > 0) {
        const uint32_t  sub = c.m_a;
        const T*        w   = params + c.m_b;
        const T         d   = w[0];
        const Vec3T<T>& x   = points[np - 1];

        const auto f = [this, sub, d, &x, a_base, &a_location](const int i, const int j, const int k) noexcept -> T {
          return this->template evaluate<D - 1, false>(
            sub, x + Vec3T<T>(T(i) * d, T(j) * d, T(k) * d), a_base, a_location);
        };

        T value = T(0);

        // The centre tap has the largest weight, and its leaf decides.
        if constexpr (Track) {
          const auto centre = this->template evaluate<D - 1, true>(sub, x, a_base, a_location);

          value += w[1] * centre.m_value;
          ids[nv] = centre.m_primitive;
        }
        else {
          value += w[1] * f(0, 0, 0);
        }
        value += w[2] * (f(1, 0, 0) + f(-1, 0, 0) + f(0, 1, 0) + f(0, -1, 0) + f(0, 0, 1) + f(0, 0, -1));
        value += w[3] * (f(1, 1, 0) + f(1, -1, 0) + f(-1, 1, 0) + f(-1, -1, 0));
        value += w[3] * (f(1, 0, 1) + f(1, 0, -1) + f(-1, 0, 1) + f(-1, 0, -1));
        value += w[3] * (f(0, 1, 1) + f(0, 1, -1) + f(0, -1, 1) + f(0, -1, -1));
        value += w[4] * (f(1, 1, 1) + f(1, 1, -1) + f(1, -1, 1) + f(1, -1, -1));
        value += w[4] * (f(-1, 1, 1) + f(-1, 1, -1) + f(-1, -1, 1) + f(-1, -1, -1));

        values[nv++] = value;
      }
      else {
        EBGEOMETRY_EXPECT(false);

        if constexpr (Track) {
          ids[nv] = NoPrimitive;
        }

        values[nv++] = T(0);
      }

      break;
    }
    case Op::Mollify: {
      if constexpr (D > 0) {
        const T*        taps = params + c.m_b;
        const Vec3T<T>& x    = points[np - 1];

        T value = T(0);

        if constexpr (Track) {
          // The leaf of the tap with the largest weight (the centre one, or the first of the central ones).
          T heaviest = T(-1);

          ids[nv] = NoPrimitive;

          for (uint32_t k = 0; k < c.m_c; k++) {
            const T* tap = taps + 4U * k;

            const auto r =
              this->template evaluate<D - 1, true>(c.m_a, x - Vec3T<T>(tap[0], tap[1], tap[2]), a_base, a_location);

            value += r.m_value * tap[3];

            if (tap[3] > heaviest) {
              heaviest = tap[3];
              ids[nv]  = r.m_primitive;
            }
          }
        }
        else {
          for (uint32_t k = 0; k < c.m_c; k++) {
            const T* tap = taps + 4U * k;

            value +=
              this->template evaluate<D - 1, false>(c.m_a, x - Vec3T<T>(tap[0], tap[1], tap[2]), a_base, a_location) *
              tap[3];
          }
        }

        values[nv++] = value;
      }
      else {
        EBGEOMETRY_EXPECT(false);

        if constexpr (Track) {
          ids[nv] = NoPrimitive;
        }

        values[nv++] = T(0);
      }

      break;
    }
    case Op::UnionBVH:
    case Op::SmoothUnionBVH: {
      if constexpr (D > 0) {
        const auto r = this->template evaluateUnion<D, Track>(c, points[np - 1], params, a_base, a_location);

        if constexpr (Track) {
          ids[nv]      = r.m_primitive;
          values[nv++] = r.m_value;
        }
        else {
          values[nv++] = r;
        }
      }
      else {
        EBGEOMETRY_EXPECT(false);

        if constexpr (Track) {
          ids[nv] = NoPrimitive;
        }

        values[nv++] = T(0);
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

  if constexpr (Track) {
    return TapeDetail::Tracked<T>{values[0], ids[0]};
  }
  else {
    return values[0];
  }
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline T
Tape<T, UserPrimitives...>::evaluatePrimitive(const uint32_t      a_type,
                                              const uint32_t      a_index,
                                              const Vec3T<T>&     a_point,
                                              const void*         a_base,
                                              const PoolLocation& a_location) const noexcept
{
  return this->template dispatchPrimitive<0>(a_type, a_index, a_point, a_base, a_location);
}

template <class T, class... UserPrimitives>
template <size_t I>
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

      const auto& stored = TapeDetail::storeAt<I>(m_layout.m_primitives).at(a_base, a_index);

      // A pool-resident primitive is stored as its layout and resolves against the tape's own location.
      if constexpr (isPoolHandle<P>) {
        return P(stored, a_location).signedDistance(a_point);
      }
      else {
        return stored.signedDistance(a_point);
      }
    }

    return this->template dispatchPrimitive<I + 1>(a_type, a_index, a_point, a_base, a_location);
  }
  else {
    EBGEOMETRY_EXPECT(false);

    return T(0);
  }
}

template <class T, class... UserPrimitives>
template <unsigned D, bool Track>
EBGEOMETRY_HOST_DEVICE
inline typename Tape<T, UserPrimitives...>::template Result<Track>
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

  struct Values
  {
    T a = Math::Limits<T>::infinity();
    T b = Math::Limits<T>::infinity();
  };

  // When tracking, the leaves of a and b too.
  struct TrackedValues
  {
    T        a  = Math::Limits<T>::infinity();
    T        b  = Math::Limits<T>::infinity();
    uint32_t ia = NoPrimitive;
    uint32_t ib = NoPrimitive;
  };

  using Closest = std::conditional_t<Track, TrackedValues, Values>;

  const auto insert = [](Closest& a_closest, const Result<Track>& a_result) noexcept {
    if constexpr (Track) {
      const T v = a_result.m_value;

      EBGEOMETRY_EXPECT(!std::isnan(v));

      if (v < a_closest.a) {
        a_closest.b  = a_closest.a;
        a_closest.ib = a_closest.ia;
        a_closest.a  = v;
        a_closest.ia = a_result.m_primitive;
      }
      else if (v < a_closest.b) {
        a_closest.b  = v;
        a_closest.ib = a_result.m_primitive;
      }
    }
    else {
      EBGEOMETRY_EXPECT(!std::isnan(a_result));

      if (a_result < a_closest.a) {
        a_closest.b = a_closest.a;
        a_closest.a = a_result;
      }
      else if (a_result < a_closest.b) {
        a_closest.b = a_result;
      }
    }
  };

  Closest closest;

  for (uint32_t i = 0; i < unboundedPrograms.size(); i++) {
    insert(closest, this->template evaluate<D - 1, Track>(unboundedPrograms[i], a_point, a_base, a_location));
  }

  const auto evalLeaf = [this, &programs, &a_point, a_base, &a_location, &insert](
                          Closest& a_closest, const size_t a_offset, const size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      insert(a_closest,
             this->template evaluate<D - 1, Track>(programs[static_cast<uint32_t>(i)], a_point, a_base, a_location));
    }
  };

  const auto pruneDist2 = [smooth](const Closest& a_closest) noexcept -> T {
    const T bound = Math::max(T(0), smooth ? a_closest.b : a_closest.a);

    return bound * bound;
  };

  bvh.pruneTraverse(a_point, closest, evalLeaf, pruneDist2);

  T value = closest.a;

  if (smooth) {
    const T s = a_params[a_clause.m_b];

    value =
      (a_clause.m_flags == 0U) ? SmoothMinOp<T>{}(closest.a, closest.b, s) : ExpMinOp<T>{}(closest.a, closest.b, s);
  }

  // The smallest operand decides, smooth or not.
  if constexpr (Track) {
    return TapeDetail::Tracked<T>{value, closest.ia};
  }
  else {
    return value;
  }
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
inline uint32_t
Tape<T, UserPrimitives...>::getNumPrograms() const noexcept
{
  return m_layout.m_numOwnPrograms;
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getNumClauses() const noexcept
{
  return m_layout.m_numOwnClauses;
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

  for (const auto* unions : {&m_layout.m_unions, &m_layout.m_rayUnions}) {
    for (const auto& parts : unions->bind(base)) {
      [[maybe_unused]] const UnionRoot bvh       = UnionRoot(parts.m_bvh, location).rebasedView(a_pool);
      [[maybe_unused]] const UnionRoot unbounded = UnionRoot(parts.m_unbounded, location).rebasedView(a_pool);
    }
  }

  const uint64_t rayEnd = Math::max(Math::max(m_layout.m_rayClauses.endByte(), m_layout.m_rayPrograms.endByte()),
                                    m_layout.m_rayUnions.endByte());

  const uint64_t endByte =
    Math::max(Math::max(Math::max(m_layout.m_clauses.endByte(), m_layout.m_programs.endByte()),
                        Math::max(Math::max(m_layout.m_params.endByte(), m_layout.m_unions.endByte()),
                                  TapeDetail::storeEndByte(m_layout.m_primitives))),
              rayEnd);

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

    const auto clauses     = m_layout.m_clauses.bind(base);
    const auto programs    = m_layout.m_programs.bind(base);
    const auto params      = m_layout.m_params.bind(base);
    const auto unions      = m_layout.m_unions.bind(base);
    const auto rayClauses  = m_layout.m_rayClauses.bind(base);
    const auto rayPrograms = m_layout.m_rayPrograms.bind(base);
    const auto rayUnions   = m_layout.m_rayUnions.bind(base);

    for (const auto& range : programs) {
      staging.m_programs.emplace_back(clauses.begin() + range.m_begin, clauses.begin() + range.m_end);
    }

    for (const auto& range : rayPrograms) {
      staging.m_rayPrograms.emplace_back(rayClauses.begin() + range.m_begin, rayClauses.begin() + range.m_end);
    }

    staging.m_params.assign(params.begin(), params.end());
    staging.m_unions.assign(unions.begin(), unions.end());
    staging.m_rayUnions.assign(rayUnions.begin(), rayUnions.end());
  }

  TapeDetail::deepCopyStore<0, Primitives>(m_layout.m_primitives, location, a_dstPool, staging.m_primitives);

  for (auto* unions : {&staging.m_unions, &staging.m_rayUnions}) {
    for (auto& parts : *unions) {
      parts.m_bvh       = UnionRoot(parts.m_bvh, location).deepCopy(a_dstPool).layout();
      parts.m_unbounded = UnionRoot(parts.m_unbounded, location).deepCopy(a_dstPool).layout();
    }
  }

  staging.m_depth          = {m_layout.m_callDepth};
  staging.m_numPrimitives  = m_layout.m_numPrimitives;
  staging.m_numOwnPrograms = m_layout.m_numOwnPrograms;
  staging.m_numOwnClauses  = m_layout.m_numOwnClauses;
  staging.m_boundingVolume = m_layout.m_boundingVolume;
  staging.m_quality        = m_layout.m_quality;

  Tape copy(Layout{}, PoolLocation{});

  copy.finalize(a_dstPool, staging);

  return copy;
}

} // namespace EBGeometry

#endif
