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
   * @param[in,out] a_staging        Where the compiled arrays go.
   */
  Compiler(Pool&                a_pool,
           const TapeOptions&   a_options,
           const uint32_t       a_valueStackSize,
           const uint32_t       a_pointStackSize,
           Staging<T, K, List>& a_staging) noexcept
    : m_pool(a_pool),
      m_options(a_options),
      m_valueStackSize(a_valueStackSize),
      m_pointStackSize(a_pointStackSize),
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
    State state{id, 0U, 1U, 0U, 1U};

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
      m_pool, programs, boxes, m_options.m_construction, BVH::PackedBVH<T, uint32_t, K>::defaultConstructionOptions());

    UnionBVH<T, K> bvh;

    bvh.m_bvh       = parts.bvh.layout();
    bvh.m_unbounded = parts.unbounded.layout();

    m_staging.m_unions.push_back(bvh);

    return static_cast<uint32_t>(m_staging.m_unions.size() - 1);
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

      this->push(a_state, clause(Op::Primitive, index, 0U, 0U, 0U, static_cast<uint16_t>(type)), 1, 0);

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
    a_pool, a_options, ValueStackSize, PointStackSize, staging);

  [[maybe_unused]] const uint32_t root = compiler.compileProgram(a_expr.node());

  EBGEOMETRY_EXPECT(root == 0U);

  EBGEOMETRY_REQUIRE(staging.m_depth.front() <= MaxCallDepth,
                     "Tape: Blur, Mollify and BVH unions nest %u deep, but a tape allows at most %u",
                     unsigned(staging.m_depth.front()),
                     unsigned(MaxCallDepth));

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

  Layout layout;

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

  TapeDetail::assignStore<0>(base, layout.m_primitives, a_staging.m_primitives);

  layout.m_boundingVolume = a_staging.m_boundingVolume;
  layout.m_quality        = a_staging.m_quality;
  layout.m_callDepth      = a_staging.m_depth.front();

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
template <unsigned D>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline T
Tape<T, UserPrimitives...>::evaluate(const uint32_t      a_program,
                                     const Vec3T<T>&     a_point,
                                     const void*         a_base,
                                     const PoolLocation& a_location) const noexcept
{
  return this->template evaluateBody<D>(a_program, a_point, a_base, a_location);
}

template <class T, class... UserPrimitives>
template <unsigned D>
EBGEOMETRY_HOST_DEVICE
inline T
Tape<T, UserPrimitives...>::evaluateBody(const uint32_t      a_program,
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

  uint32_t nv = 0;
  uint32_t np = 0;

  points[np++] = a_point;

  for (uint32_t pc = range.m_begin; pc < range.m_end; pc++) {
    const TapeDetail::Clause& c = clauses[pc];

    switch (c.m_op) {
    case Op::Primitive: {
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

      values[nv - 1] = Math::min(values[nv - 1], values[nv]);

      break;
    }
    case Op::Max: {
      nv--;

      values[nv - 1] = Math::max(values[nv - 1], values[nv]);

      break;
    }
    case Op::PairInit: {
      const T empty = (c.m_flags == 0U) ? Math::Limits<T>::infinity() : -Math::Limits<T>::infinity();

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
      }
      else if (beyondB) {
        b = v;
      }

      break;
    }
    case Op::PairBlend: {
      const T b = values[--nv];
      const T a = values[nv - 1];
      const T s = params[c.m_a];

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

        // The 27 taps p + d (i, j, k); a tap m = |i| + |j| + |k| steps from the centre has weight w[1 + m].
        T value = T(0);

        for (int t = 0; t < 27; t++) {
          const int i = t / 9 - 1;
          const int j = (t / 3) % 3 - 1;
          const int k = t % 3 - 1;
          const int m = (i != 0) + (j != 0) + (k != 0);

          const Vec3T<T> tap = x + Vec3T<T>(T(i) * d, T(j) * d, T(k) * d);

          value += w[1 + m] * this->template evaluate<D - 1>(sub, tap, a_base, a_location);
        }

        values[nv++] = value;
      }
      else {
        EBGEOMETRY_EXPECT(false);

        values[nv++] = T(0);
      }

      break;
    }
    case Op::Mollify: {
      if constexpr (D > 0) {
        const T*        taps = params + c.m_b;
        const Vec3T<T>& x    = points[np - 1];

        T value = T(0);

        for (uint32_t k = 0; k < c.m_c; k++) {
          const T* tap = taps + 4U * k;

          value +=
            this->template evaluate<D - 1>(c.m_a, x - Vec3T<T>(tap[0], tap[1], tap[2]), a_base, a_location) * tap[3];
        }

        values[nv++] = value;
      }
      else {
        EBGEOMETRY_EXPECT(false);

        values[nv++] = T(0);
      }

      break;
    }
    case Op::UnionBVH:
    case Op::SmoothUnionBVH: {
      if constexpr (D > 0) {
        values[nv++] = this->template evaluateUnion<D>(c, points[np - 1], params, a_base, a_location);
      }
      else {
        EBGEOMETRY_EXPECT(false);

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

    return this->template dispatchPrimitive<I + 1, Handles>(a_type, a_index, a_point, a_base, a_location);
  }
  else {
    EBGEOMETRY_EXPECT(false);

    return T(0);
  }
}

template <class T, class... UserPrimitives>
template <unsigned D>
EBGEOMETRY_HOST_DEVICE
inline T
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
    T a = Math::Limits<T>::infinity();
    T b = Math::Limits<T>::infinity();
  };

  const auto insert = [](Closest& a_closest, const T a_value) noexcept {
    EBGEOMETRY_EXPECT(!std::isnan(a_value));

    if (a_value < a_closest.a) {
      a_closest.b = a_closest.a;
      a_closest.a = a_value;
    }
    else if (a_value < a_closest.b) {
      a_closest.b = a_value;
    }
  };

  Closest closest;

  for (uint32_t i = 0; i < unboundedPrograms.size(); i++) {
    insert(closest, this->template evaluate<D - 1>(unboundedPrograms[i], a_point, a_base, a_location));
  }

  const auto evalLeaf = [this, &programs, &a_point, a_base, &a_location, &insert](
                          Closest& a_closest, const size_t a_offset, const size_t a_count) noexcept {
    for (size_t i = a_offset; i < a_offset + a_count; i++) {
      insert(a_closest,
             this->template evaluate<D - 1>(programs[static_cast<uint32_t>(i)], a_point, a_base, a_location));
    }
  };

  const auto pruneDist2 = [smooth](const Closest& a_closest) noexcept -> T {
    const T bound = Math::max(T(0), smooth ? a_closest.b : a_closest.a);

    return bound * bound;
  };

  bvh.pruneTraverse(a_point, closest, evalLeaf, pruneDist2);

  if (!smooth) {
    return closest.a;
  }

  const T s = a_params[a_clause.m_b];

  return (a_clause.m_flags == 0U) ? SmoothMinOp<T>{}(closest.a, closest.b, s) : ExpMinOp<T>{}(closest.a, closest.b, s);
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
  return m_layout.m_programs.size();
}

template <class T, class... UserPrimitives>
EBGEOMETRY_HOST_DEVICE
inline uint32_t
Tape<T, UserPrimitives...>::getNumClauses() const noexcept
{
  return m_layout.m_clauses.size();
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

  for (const auto& parts : m_layout.m_unions.bind(base)) {
    [[maybe_unused]] const UnionRoot bvh       = UnionRoot(parts.m_bvh, location).rebasedView(a_pool);
    [[maybe_unused]] const UnionRoot unbounded = UnionRoot(parts.m_unbounded, location).rebasedView(a_pool);
  }

  const uint64_t endByte = Math::max(Math::max(m_layout.m_clauses.endByte(), m_layout.m_programs.endByte()),
                                     Math::max(Math::max(m_layout.m_params.endByte(), m_layout.m_unions.endByte()),
                                               TapeDetail::storeEndByte(m_layout.m_primitives)));

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

    for (const auto& range : programs) {
      staging.m_programs.emplace_back(clauses.begin() + range.m_begin, clauses.begin() + range.m_end);
    }

    staging.m_params.assign(params.begin(), params.end());
    staging.m_unions.assign(unions.begin(), unions.end());
  }

  TapeDetail::deepCopyStore<0, Primitives>(m_layout.m_primitives, location, a_dstPool, staging.m_primitives);

  for (auto& parts : staging.m_unions) {
    parts.m_bvh       = UnionRoot(parts.m_bvh, location).deepCopy(a_dstPool).layout();
    parts.m_unbounded = UnionRoot(parts.m_unbounded, location).deepCopy(a_dstPool).layout();
  }

  staging.m_depth          = {m_layout.m_callDepth};
  staging.m_boundingVolume = m_layout.m_boundingVolume;
  staging.m_quality        = m_layout.m_quality;

  Tape copy(Layout{}, PoolLocation{});

  copy.finalize(a_dstPool, staging);

  return copy;
}

} // namespace EBGeometry

#endif
