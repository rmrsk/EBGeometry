// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TapeImplem.hpp
 * @brief  Implementation of EBGeometry_Tape.hpp.
 * @details Holds every out-of-line definition: the TapeBuilder methods, Tape::view, the free
 * compile function, the TapeView::value/Tape::value evaluators, and every flatten() override body
 * (the base opaque-host default, the leaf template, and all transform/combiner nodes). Included
 * last from EBGeometry_Tape.hpp so that Tape, TapeView, TapeBuilder, and every implicit-function
 * class are complete here.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TAPEIMPLEM_HPP
#define EBGEOMETRY_TAPEIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_CSG.hpp"
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Tape.hpp"
#include "EBGeometry_Transform.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

namespace TapeDetail {

// Dependent always-false for the paramArrayFor() fallback static_assert (only fires if an
// unregistered op reaches the final else of the constexpr-if chain).
template <class>
inline constexpr bool alwaysFalse = false;

} // namespace TapeDetail

// ── Tape::view ────────────────────────────────────────────────────────────────
template <class T>
TapeView<T>
Tape<T>::view() const noexcept
{
  TapeView<T> v;

  v.clauses    = m_clauses.data();
  v.numClauses = static_cast<uint32_t>(m_clauses.size());

#define EBGEOMETRY_TAPE_VIEW_ASSIGN(OP, MEMBER) v.MEMBER = m_##MEMBER.data();
  EBGEOMETRY_TAPE_PARAM_SOA(EBGEOMETRY_TAPE_VIEW_ASSIGN)
#undef EBGEOMETRY_TAPE_VIEW_ASSIGN

  v.scalarParams = m_scalarParams.data();
  v.hostNodes    = m_hostNodes.data();

  v.bvhNodes    = m_bvhNodes.data();
  v.bvhChildren = m_bvhChildren.data();
  v.bvhLeaves   = m_bvhLeaves.data();
  v.bvhBlocks   = m_bvhBlocks.data();

  v.numMainClauses = m_numMainClauses;
  v.numCoordSlots  = m_numCoordSlots;
  v.numValueSlots  = m_numValueSlots;
  v.rootValueSlot  = m_rootValueSlot;
  v.deviceEligible = m_deviceEligible;

  return v;
}

// ── TapeBuilder ───────────────────────────────────────────────────────────────
template <class T>
template <class Op>
std::vector<typename Op::Params>&
TapeBuilder<T>::paramArrayFor()
{
#define EBGEOMETRY_TAPE_PICK(OP, MEMBER)     \
  if constexpr (std::is_same_v<Op, OP<T>>) { \
    return m_tape.m_##MEMBER;                \
  }                                          \
  else
  EBGEOMETRY_TAPE_PARAM_SOA(EBGEOMETRY_TAPE_PICK)
  {
    static_assert(TapeDetail::alwaysFalse<Op>, "TapeBuilder::paramArrayFor: unregistered op");
  }
#undef EBGEOMETRY_TAPE_PICK
}

template <class T>
int
TapeBuilder<T>::newCoordSlot() noexcept
{
  const int slot = static_cast<int>(m_tape.m_numCoordSlots);

  m_tape.m_numCoordSlots++;

  return slot;
}

template <class T>
int
TapeBuilder<T>::newValueSlot() noexcept
{
  const int slot = static_cast<int>(m_tape.m_numValueSlots);

  m_tape.m_numValueSlots++;

  return slot;
}

template <class T>
template <class Op>
int
TapeBuilder<T>::emitLeaf(int a_inCoord, const typename Op::Params& a_params)
{
  const int out = this->newValueSlot();

  auto& array = this->template paramArrayFor<Op>();
  array.push_back(a_params);

  const uint32_t paramIndex = static_cast<uint32_t>(array.size() - 1);

  m_tape.m_clauses.push_back(
    TapeClause{OpcodeOf<Op>::value, paramIndex, static_cast<uint32_t>(a_inCoord), 0, static_cast<uint32_t>(out)});

  return out;
}

template <class T>
template <class Op>
int
TapeBuilder<T>::emitCoordTransform(Opcode a_opcode, int a_inCoord, const typename Op::Params& a_params)
{
  const int out = this->newCoordSlot();

  auto& array = this->template paramArrayFor<Op>();
  array.push_back(a_params);

  const uint32_t paramIndex = static_cast<uint32_t>(array.size() - 1);

  m_tape.m_clauses.push_back(
    TapeClause{a_opcode, paramIndex, static_cast<uint32_t>(a_inCoord), 0, static_cast<uint32_t>(out)});

  return out;
}

template <class T>
template <class Op>
int
TapeBuilder<T>::emitValueTransform(Opcode a_opcode, int a_inValue, const typename Op::Params& a_params)
{
  const int out = this->newValueSlot();

  auto& array = this->template paramArrayFor<Op>();
  array.push_back(a_params);

  const uint32_t paramIndex = static_cast<uint32_t>(array.size() - 1);

  m_tape.m_clauses.push_back(
    TapeClause{a_opcode, paramIndex, static_cast<uint32_t>(a_inValue), 0, static_cast<uint32_t>(out)});

  return out;
}

template <class T>
int
TapeBuilder<T>::emitCombine(Opcode a_opcode, int a_valA, int a_valB)
{
  const int out = this->newValueSlot();

  m_tape.m_clauses.push_back(
    TapeClause{a_opcode, 0, static_cast<uint32_t>(a_valA), static_cast<uint32_t>(a_valB), static_cast<uint32_t>(out)});

  return out;
}

template <class T>
int
TapeBuilder<T>::emitSmooth(Opcode a_opcode, int a_valA, int a_valB, T a_smoothLen)
{
  const int out = this->newValueSlot();

  m_tape.m_scalarParams.push_back(a_smoothLen);

  const uint32_t paramIndex = static_cast<uint32_t>(m_tape.m_scalarParams.size() - 1);

  m_tape.m_clauses.push_back(TapeClause{
    a_opcode, paramIndex, static_cast<uint32_t>(a_valA), static_cast<uint32_t>(a_valB), static_cast<uint32_t>(out)});

  return out;
}

template <class T>
int
TapeBuilder<T>::emitHostCallback(int a_inCoord, const ImplicitFunction<T>* a_node)
{
  const int out = this->newValueSlot();

  m_tape.m_hostNodes.push_back(a_node);

  const uint32_t paramIndex = static_cast<uint32_t>(m_tape.m_hostNodes.size() - 1);

  m_tape.m_clauses.push_back(
    TapeClause{Opcode::HOST_CALLBACK, paramIndex, static_cast<uint32_t>(a_inCoord), 0, static_cast<uint32_t>(out)});

  m_tape.m_deviceEligible = false;

  return out;
}

template <class T>
int
TapeBuilder<T>::emitBVHUnion(int                                       a_inCoord,
                             std::vector<BVHNodeInfo>&&                a_nodes,
                             std::vector<const ImplicitFunction<T>*>&& a_primitives,
                             bool                                      a_smooth,
                             Opcode                                    a_blend,
                             T                                         a_smoothLen)
{
  // Belt-and-braces nesting guard: the BVH flatten overrides already route a nested BVH union to
  // emitHostCallback while finish()'s deferred phase runs.
  EBGEOMETRY_EXPECT(!m_flatteningBVHLeaf);
  EBGEOMETRY_EXPECT(!a_nodes.empty());
  EBGEOMETRY_EXPECT(!a_primitives.empty());
  EBGEOMETRY_EXPECT(a_smooth || a_smoothLen == T(0));
  EBGEOMETRY_EXPECT(!a_smooth || a_smoothLen > T(0));

  const int out = this->newValueSlot();

  const size_t blockIndex = m_tape.m_bvhBlocks.size();

  EBGEOMETRY_EXPECT(blockIndex <= size_t(std::numeric_limits<uint32_t>::max()));

  // Globalise the block-local node/child/leaf indices into the tape's shared arrays (blocks are
  // appended back to back, so a block-local index maps to base + local).
  const uint32_t nodeBase = static_cast<uint32_t>(m_tape.m_bvhNodes.size());
  const uint32_t leafBase = static_cast<uint32_t>(m_tape.m_bvhLeaves.size());

  for (const auto& info : a_nodes) {
    TapeBVHNode<T> node;

    node.lo = info.aabb.getLowCorner();
    node.hi = info.aabb.getHighCorner();

    if (info.numPrims > 0) {
      EBGEOMETRY_EXPECT(info.children.empty());
      EBGEOMETRY_EXPECT(size_t(info.primOff) + size_t(info.numPrims) <= a_primitives.size());

      node.leafBegin = leafBase + info.primOff;
      node.leafCount = info.numPrims;
    }
    else {
      EBGEOMETRY_EXPECT(!info.children.empty());
      EBGEOMETRY_EXPECT(info.children.size() <= size_t(TapeBVHMaxChildren));

      node.childBegin = static_cast<uint32_t>(m_tape.m_bvhChildren.size());
      node.childCount = static_cast<uint32_t>(info.children.size());

      for (const uint32_t child : info.children) {
        EBGEOMETRY_EXPECT(size_t(child) < a_nodes.size());

        m_tape.m_bvhChildren.push_back(nodeBase + child);
      }
    }

    m_tape.m_bvhNodes.push_back(node);
  }

  // Placeholder leaf entries; finish()'s deferred phase fills them once each primitive's subtree
  // has been flattened into the tail clause region.
  m_tape.m_bvhLeaves.resize(size_t(leafBase) + a_primitives.size());

  TapeBVHBlock<T> block;

  block.smoothLen = a_smoothLen;
  block.rootNode  = nodeBase;
  block.numNodes  = static_cast<uint32_t>(a_nodes.size());
  block.leafBegin = leafBase;
  block.numLeaves = static_cast<uint32_t>(a_primitives.size());
  block.blend     = a_blend;

  m_tape.m_bvhBlocks.push_back(block);

  m_tape.m_clauses.push_back(TapeClause{a_smooth ? Opcode::BVH_SMOOTH_UNION : Opcode::BVH_UNION,
                                        static_cast<uint32_t>(blockIndex),
                                        static_cast<uint32_t>(a_inCoord),
                                        0,
                                        static_cast<uint32_t>(out)});

  m_pendingBlocks.push_back(PendingBVHBlock{static_cast<uint32_t>(blockIndex), a_inCoord, std::move(a_primitives)});

  return out;
}

template <class T>
bool
TapeBuilder<T>::isFlatteningBVHLeaf() const noexcept
{
  return m_flatteningBVHLeaf;
}

template <class T>
Tape<T>
TapeBuilder<T>::finish(int a_rootValueSlot)
{
  // Freeze the main clause region; everything appended below lands in the deferred region
  // [numMainClauses, numClauses) and only runs on demand through a BVH clause.
  m_tape.m_numMainClauses = static_cast<uint32_t>(m_tape.m_clauses.size());

  // Slot-window reuse across BVH leaves: all leaf subtrees (of ALL blocks) share one coordinate/
  // value slot window starting at the main program's slot counts. This is safe because
  //
  //   1. leaf sub-ranges execute strictly sequentially -- the interpreter runs one leaf's clause
  //      range to completion and folds its valueSlot into the traversal state immediately, before
  //      any other leaf (of this or any other BVH clause) executes, so no two leaves' slots are
  //      ever live at the same time;
  //   2. a leaf subtree reads nothing from the main program except its input coordinate slot
  //      (block.coordSlot, allocated during the main phase and therefore below the window base),
  //      and writes only slots it allocates itself (inside the window) -- asserted below;
  //   3. a BVH clause's own output slot is likewise allocated during the main phase, so folding a
  //      block's result cannot be clobbered by a later block's leaves.
  //
  // Scratch is therefore sized as main program + max over leaves instead of main + sum over
  // leaves.
  const uint32_t coordBase = m_tape.m_numCoordSlots;
  const uint32_t valueBase = m_tape.m_numValueSlots;

  uint32_t maxCoordSlots = coordBase;
  uint32_t maxValueSlots = valueBase;

  const size_t numPendingBlocks = m_pendingBlocks.size();

  m_flatteningBVHLeaf = true;

  // Indexed loop rather than a range-for: nested BVH unions lower to host callbacks during this
  // phase (asserted below), but an index stays valid even if m_pendingBlocks were ever to grow
  // mid-loop, where a range-for iterator would be silently invalidated.
  for (size_t b = 0; b < m_pendingBlocks.size(); b++) {
    const PendingBVHBlock& block = m_pendingBlocks[b];

    EBGEOMETRY_EXPECT(block.coordSlot >= 0);
    EBGEOMETRY_EXPECT(static_cast<uint32_t>(block.coordSlot) < coordBase);

    for (size_t i = 0; i < block.primitives.size(); i++) {
      // Rewind the slot counters to the shared window base before each leaf.
      m_tape.m_numCoordSlots = coordBase;
      m_tape.m_numValueSlots = valueBase;

      const uint32_t begin = static_cast<uint32_t>(m_tape.m_clauses.size());
      const int      slot  = block.primitives[i]->flatten(*this, block.coordSlot);

      // Every flatten emits at least one clause and returns a freshly-allocated value slot, so the
      // sub-range is non-empty and the leaf's result lives inside the reused window.
      EBGEOMETRY_EXPECT(static_cast<uint32_t>(m_tape.m_clauses.size()) > begin);
      EBGEOMETRY_EXPECT(slot >= 0);
      EBGEOMETRY_EXPECT(static_cast<uint32_t>(slot) >= valueBase);

      maxCoordSlots = std::max(maxCoordSlots, m_tape.m_numCoordSlots);
      maxValueSlots = std::max(maxValueSlots, m_tape.m_numValueSlots);

      m_tape.m_bvhLeaves[size_t(m_tape.m_bvhBlocks[block.blockIndex].leafBegin) + i] =
        TapeBVHLeaf{begin, static_cast<uint32_t>(m_tape.m_clauses.size()) - begin, static_cast<uint32_t>(slot)};
    }
  }

  // Nested BVH unions lower to host callbacks during the deferred phase, so no new pending block
  // can have appeared while this loop ran.
  EBGEOMETRY_EXPECT(m_pendingBlocks.size() == numPendingBlocks);

  m_flatteningBVHLeaf = false;

  m_tape.m_numCoordSlots = maxCoordSlots;
  m_tape.m_numValueSlots = maxValueSlots;

  m_tape.m_rootValueSlot = static_cast<uint32_t>(a_rootValueSlot);

  return std::move(m_tape);
}

// ── Free function: compile ────────────────────────────────────────────────────
template <class T>
Tape<T>
compile(const ImplicitFunction<T>& a_tree)
{
  TapeBuilder<T> builder;

  const int rootCoordSlot = builder.newCoordSlot();
  const int rootValueSlot = a_tree.flatten(builder, rootCoordSlot);

  return builder.finish(rootValueSlot);
}

// ── Interpreter: TapeView::value / Tape::value ────────────────────────────────
namespace TapeDetail {

// Forward declaration: the clause loop and the BVH clause handler are mutually referential (a BVH
// clause executes leaf sub-ranges through the inner clause loop). The recursion is statically
// bounded at depth two -- executeClauses<false> -> executeBVHClause -> executeClauses<true> -- and
// executeClauses<true> treats a BVH clause as unreachable (the builder lowers nested BVH unions to
// host callbacks).
template <bool INNER, class T>
EBGEOMETRY_HOST_DEVICE void
executeClauses(
  const TapeView<T>& a_view, uint32_t a_begin, uint32_t a_end, Vec3T<T>* a_coordScratch, T* a_valueScratch) noexcept;

// One BVH_UNION (SMOOTH == false) or BVH_SMOOTH_UNION (SMOOTH == true) clause: an iterative,
// pruned branch-and-bound traversal mirroring PackedBVH::pruneTraverse (fixed 256-entry stack,
// fresh pop-time bound recheck, children sorted by descending box distance so the nearest pops
// first, push-time filter) with BVHUnionIF::value's running-minimum state (sharp) or
// BVHSmoothUnionIF::value's two-smallest state and max(0, b)^2 bound (smooth). Leaf "evaluation"
// executes the leaf's flattened clause sub-range from the tape's deferred region and folds its
// value slot.
//
// Known ULP caveat: the SIMD pruneTraverse paths sum dx^2 + (dy^2 + dz^2) in vector registers and
// use an unstable std::sort; this path uses AABBT::getDistance2 and a stable insertion sort.
// Visited *sets* can differ on ULP-level box-distance ties, but the folded result is identical
// whenever the box distance lower-bounds the leaf values (true for all metric primitives).
template <bool SMOOTH, class T>
EBGEOMETRY_HOST_DEVICE void
executeBVHClause(const TapeView<T>& a_view,
                 const TapeClause&  a_clause,
                 Vec3T<T>*          a_coordScratch,
                 T*                 a_valueScratch) noexcept
{
  const TapeBVHBlock<T>& block = a_view.bvhBlocks[a_clause.paramIndex];
  const Vec3T<T>         point = a_coordScratch[a_clause.in0];

  // Sharp: 'a' is the running minimum ('b' unused). Smooth: 'a' <= 'b' are the two smallest leaf
  // values, exactly as in BVHSmoothUnionIF::value.
  T a = std::numeric_limits<T>::infinity();
  T b = std::numeric_limits<T>::infinity();

  struct StackEntry
  {
    uint32_t node;
    T        dist2;
  };

  StackEntry stack[TapeBVHStackSize];

  int top = 0;

  stack[top++] = {block.rootNode, T(0)};

  while (top > 0) {
    const StackEntry entry = stack[--top];

    // Re-read the pruning bound fresh at every pop: max(0, best)^2 keyed on the running minimum
    // (sharp) or on the second-smallest value (smooth) -- identical to the pruneDist2 lambdas in
    // BVHUnionIF::value / BVHSmoothUnionIF::value.
    const T bound  = std::max(T(0), SMOOTH ? b : a);
    const T bound2 = bound * bound;

    if (entry.dist2 > bound2) {
      continue;
    }

    const TapeBVHNode<T>& node = a_view.bvhNodes[entry.node];

    if (node.leafCount > 0) {

      for (uint32_t l = 0; l < node.leafCount; l++) {
        const TapeBVHLeaf& leaf = a_view.bvhLeaves[node.leafBegin + l];

        executeClauses<true>(
          a_view, leaf.clauseBegin, leaf.clauseBegin + leaf.clauseCount, a_coordScratch, a_valueScratch);

        const T v = a_valueScratch[leaf.valueSlot];

        if constexpr (SMOOTH) {
          if (v < a) {
            b = a;
            a = v;
          }
          else if (v < b) {
            b = v;
          }
        }
        else {
          a = UnionOp<T>::combine(a, v);
        }
      }
    }
    else {
      T        childDist2[TapeBVHMaxChildren];
      uint32_t childIndex[TapeBVHMaxChildren];

      for (uint32_t k = 0; k < node.childCount; k++) {
        childIndex[k] = a_view.bvhChildren[node.childBegin + k];

        const TapeBVHNode<T>& child = a_view.bvhNodes[childIndex[k]];

        childDist2[k] = BoundingVolumes::AABBT<T>(child.lo, child.hi).getDistance2(point);
      }

      // Insertion sort, descending on box distance: the farthest child is pushed first so the
      // nearest is popped first (childCount <= 16; strict comparison keeps the sort stable and
      // deterministic).
      for (uint32_t k = 1; k < node.childCount; k++) {
        const T        d2  = childDist2[k];
        const uint32_t idx = childIndex[k];

        uint32_t j = k;

        while (j > 0 && childDist2[j - 1] < d2) {
          childDist2[j] = childDist2[j - 1];
          childIndex[j] = childIndex[j - 1];

          j--;
        }

        childDist2[j] = d2;
        childIndex[j] = idx;
      }

      // Push-time filter against the pop-time bound (the state cannot change between pop and push
      // within one interior visit, so re-reading it here would be identical); the pop-time recheck
      // above keeps this semantics-neutral as the bound only tightens.
      for (uint32_t k = 0; k < node.childCount; k++) {
        if (childDist2[k] <= bound2) {
          EBGEOMETRY_EXPECT(top < static_cast<int>(TapeBVHStackSize));

          stack[top++] = {childIndex[k], childDist2[k]};
        }
      }
    }
  }

  if constexpr (SMOOTH) {
    // Blend the two nearest values with the block's registered smooth operator. The one-primitive
    // edge case (b == +inf) flows into the blend unchanged, exactly as in
    // BVHSmoothUnionIF::value.
    switch (block.blend) {
#define EBGEOMETRY_TAPE_EVAL_BVH_BLEND(TAG, OP)                        \
  case Opcode::TAG:                                                    \
    a_valueScratch[a_clause.out] = OP<T>::eval(a, b, block.smoothLen); \
    break;
      EBGEOMETRY_TAPE_SMOOTH_OPS(EBGEOMETRY_TAPE_EVAL_BVH_BLEND)
#undef EBGEOMETRY_TAPE_EVAL_BVH_BLEND

    default: {
      EBGEOMETRY_EXPECT(false && "BVH_SMOOTH_UNION block carries an unregistered blend tag");

      break;
    }
    }
  }
  else {
    a_valueScratch[a_clause.out] = a;
  }
}

template <bool INNER, class T>
EBGEOMETRY_HOST_DEVICE void
executeClauses(
  const TapeView<T>& a_view, uint32_t a_begin, uint32_t a_end, Vec3T<T>* a_coordScratch, T* a_valueScratch) noexcept
{
  for (uint32_t i = a_begin; i < a_end; i++) {
    const TapeClause& c = a_view.clauses[i];

    switch (c.opcode) {
#define EBGEOMETRY_TAPE_EVAL_LEAF(TAG, OP, MEMBER)                                           \
  case Opcode::TAG:                                                                          \
    a_valueScratch[c.out] = OP<T>::eval(a_view.MEMBER[c.paramIndex], a_coordScratch[c.in0]); \
    break;
      EBGEOMETRY_TAPE_LEAF_OPS(EBGEOMETRY_TAPE_EVAL_LEAF)
#undef EBGEOMETRY_TAPE_EVAL_LEAF

#define EBGEOMETRY_TAPE_EVAL_MAPPOINT(TAG, OP, MEMBER)                                           \
  case Opcode::TAG:                                                                              \
    a_coordScratch[c.out] = OP<T>::mapPoint(a_view.MEMBER[c.paramIndex], a_coordScratch[c.in0]); \
    break;
      EBGEOMETRY_TAPE_MAPPOINT_OPS(EBGEOMETRY_TAPE_EVAL_MAPPOINT)
#undef EBGEOMETRY_TAPE_EVAL_MAPPOINT

#define EBGEOMETRY_TAPE_EVAL_MAPVALUE(TAG, OP, MEMBER)                                           \
  case Opcode::TAG:                                                                              \
    a_valueScratch[c.out] = OP<T>::mapValue(a_view.MEMBER[c.paramIndex], a_valueScratch[c.in0]); \
    break;
      EBGEOMETRY_TAPE_MAPVALUE_OPS(EBGEOMETRY_TAPE_EVAL_MAPVALUE)
#undef EBGEOMETRY_TAPE_EVAL_MAPVALUE

#define EBGEOMETRY_TAPE_EVAL_COMBINE(TAG, OP)                                             \
  case Opcode::TAG:                                                                       \
    a_valueScratch[c.out] = OP<T>::combine(a_valueScratch[c.in0], a_valueScratch[c.in1]); \
    break;
      EBGEOMETRY_TAPE_COMBINE_OPS(EBGEOMETRY_TAPE_EVAL_COMBINE)
#undef EBGEOMETRY_TAPE_EVAL_COMBINE

#define EBGEOMETRY_TAPE_EVAL_SMOOTH(TAG, OP)                                                        \
  case Opcode::TAG:                                                                                 \
    a_valueScratch[c.out] =                                                                         \
      OP<T>::eval(a_valueScratch[c.in0], a_valueScratch[c.in1], a_view.scalarParams[c.paramIndex]); \
    break;
      EBGEOMETRY_TAPE_SMOOTH_OPS(EBGEOMETRY_TAPE_EVAL_SMOOTH)
#undef EBGEOMETRY_TAPE_EVAL_SMOOTH

    case Opcode::BVH_UNION: {
      if constexpr (INNER) {
        // Unreachable by construction: the builder lowers a BVH union encountered inside a BVH
        // leaf subtree to a host callback, so the deferred region never contains a BVH clause.
        EBGEOMETRY_EXPECT(false && "nested BVH clause in a leaf sub-range");
      }
      else {
        executeBVHClause<false>(a_view, c, a_coordScratch, a_valueScratch);
      }

      break;
    }

    case Opcode::BVH_SMOOTH_UNION: {
      if constexpr (INNER) {
        EBGEOMETRY_EXPECT(false && "nested BVH clause in a leaf sub-range");
      }
      else {
        executeBVHClause<true>(a_view, c, a_coordScratch, a_valueScratch);
      }

      break;
    }

    case Opcode::HOST_CALLBACK: {
#ifndef EBGEOMETRY_DEVICE_COMPILE
      a_valueScratch[c.out] = a_view.hostNodes[c.paramIndex]->value(a_coordScratch[c.in0]);
#else
      EBGEOMETRY_EXPECT(false && "HOST_CALLBACK clause is not evaluable on device");
#endif
      break;
    }
    }
  }
}

} // namespace TapeDetail

template <class T>
EBGEOMETRY_HOST_DEVICE T
TapeView<T>::value(const Vec3T<T>& a_point, Vec3T<T>* a_coordScratch, T* a_valueScratch) const noexcept
{
  EBGEOMETRY_EXPECT(numCoordSlots > 0);
  EBGEOMETRY_EXPECT(numValueSlots > 0);

  a_coordScratch[0] = a_point;

  // The forward pass covers the main region only; the deferred region [numMainClauses, numClauses)
  // holds BVH leaf subtrees executed on demand by BVH clauses.
  TapeDetail::executeClauses<false>(*this, 0, numMainClauses, a_coordScratch, a_valueScratch);

  return a_valueScratch[rootValueSlot];
}

template <class T>
T
Tape<T>::value(const Vec3T<T>& a_point) const noexcept
{
  // Per-thread scratch, only ever grown: queries are allocation-free after each thread's first
  // call, and concurrent readers never share a buffer. The inUse flag guards re-entrancy: a
  // HOST_CALLBACK clause calls back into arbitrary user code, and if that code evaluates a tape on
  // this same thread the shared scratch would be resized/clobbered mid-pass -- so a nested call
  // detects the live outer pass and falls back to fresh local buffers instead.
  thread_local std::vector<Vec3T<T>> coordScratch;
  thread_local std::vector<T>        valueScratch;
  thread_local bool                  inUse = false;

  if (inUse) {
    std::vector<Vec3T<T>> localCoordScratch(m_numCoordSlots);
    std::vector<T>        localValueScratch(m_numValueSlots);

    return this->view().value(a_point, localCoordScratch.data(), localValueScratch.data());
  }

  if (coordScratch.size() < m_numCoordSlots) {
    coordScratch.resize(m_numCoordSlots);
  }

  if (valueScratch.size() < m_numValueSlots) {
    valueScratch.resize(m_numValueSlots);
  }

  inUse = true;

  const T ret = this->view().value(a_point, coordScratch.data(), valueScratch.data());

  inUse = false;

  return ret;
}

// ── flatten() overrides ───────────────────────────────────────────────────────
// Base default: any node without its own override lowers to an opaque host callback (Perlin, Blur,
// Mollify, FiniteRepetition, and any user subclass). BVH unions have their own native lowering
// below.
template <class T>
int
ImplicitFunction<T, void>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  return a_builder.emitHostCallback(a_coordSlot, this);
}

// Leaf: a registered primitive lowers to a native leaf clause; an unregistered custom Op falls back
// to an opaque host callback.
template <class T, class Op>
int
ImplicitFunction<T, Op>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  if constexpr (IsRegistered<Op>) {
    return a_builder.template emitLeaf<Op>(a_coordSlot, this->m_params);
  }
  else {
    return a_builder.emitHostCallback(a_coordSlot, this);
  }
}

// Value transform: recurse into the child, then remap its value.
template <class T>
int
ComplementIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  const int childValue = m_implicitFunction->flatten(a_builder, a_coordSlot);

  return a_builder.template emitValueTransform<ComplementOp<T>>(Opcode::COMPLEMENT, childValue, m_params);
}

// Coordinate transform: remap the frame, then recurse into the child under the new frame.
template <class T>
int
TranslateIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  const int childCoord =
    a_builder.template emitCoordTransform<TranslateOp<T>>(Opcode::TRANSLATE, a_coordSlot, m_params);

  return m_implicitFunction->flatten(a_builder, childCoord);
}

template <class T>
int
RotateIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  const int childCoord = a_builder.template emitCoordTransform<RotateOp<T>>(Opcode::ROTATE, a_coordSlot, m_params);

  return m_implicitFunction->flatten(a_builder, childCoord);
}

template <class T>
int
OffsetIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  const int childValue = m_implicitFunction->flatten(a_builder, a_coordSlot);

  return a_builder.template emitValueTransform<OffsetOp<T>>(Opcode::OFFSET, childValue, m_params);
}

// Scale straddles the child: pre-scale the point, recurse, then post-scale the value.
template <class T>
int
ScaleIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  const int childCoord =
    a_builder.template emitCoordTransform<ScaleOp<T>>(Opcode::SCALE_MAPPOINT, a_coordSlot, m_params);
  const int childValue = m_implicitFunction->flatten(a_builder, childCoord);

  return a_builder.template emitValueTransform<ScaleOp<T>>(Opcode::SCALE_MAPVALUE, childValue, m_params);
}

template <class T>
int
AnnularIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  const int childValue = m_implicitFunction->flatten(a_builder, a_coordSlot);

  return a_builder.template emitValueTransform<AnnularOp<T>>(Opcode::ANNULAR, childValue, m_params);
}

template <class T>
int
ElongateIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  const int childCoord = a_builder.template emitCoordTransform<ElongateOp<T>>(Opcode::ELONGATE, a_coordSlot, m_params);

  return m_implicitFunction->flatten(a_builder, childCoord);
}

template <class T>
int
ReflectIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  const int childCoord = a_builder.template emitCoordTransform<ReflectOp<T>>(Opcode::REFLECT, a_coordSlot, m_params);

  return m_implicitFunction->flatten(a_builder, childCoord);
}

// Sharp combiners: recurse all children on the same frame, then left-fold pairwise combines.
template <class T>
int
UnionIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  int accumulator = m_implicitFunctions[0]->flatten(a_builder, a_coordSlot);

  for (size_t i = 1; i < m_implicitFunctions.size(); i++) {
    accumulator =
      a_builder.emitCombine(Opcode::UNION, accumulator, m_implicitFunctions[i]->flatten(a_builder, a_coordSlot));
  }

  return accumulator;
}

template <class T>
int
IntersectionIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  int accumulator = m_implicitFunctions[0]->flatten(a_builder, a_coordSlot);

  for (size_t i = 1; i < m_implicitFunctions.size(); i++) {
    accumulator =
      a_builder.emitCombine(Opcode::INTERSECTION, accumulator, m_implicitFunctions[i]->flatten(a_builder, a_coordSlot));
  }

  return accumulator;
}

// Difference is binary: max(A, -B). B may itself be a UnionIF of subtrahends (normal recursion).
template <class T>
int
DifferenceIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  const int valueA = m_implicitFunctionA->flatten(a_builder, a_coordSlot);
  const int valueB = m_implicitFunctionB->flatten(a_builder, a_coordSlot);

  return a_builder.emitCombine(Opcode::DIFFERENCE, valueA, valueB);
}

// Smooth combiners are templated on their blend operator (the Blend parameter, defaulting to
// SmoothMinOp/SmoothMaxOp), so the blend's identity is known at compile time and a registered blend
// (one with a BlendOpcodeOf specialisation: SmoothMinOp, SmoothMaxOp, ExpMinOp) lowers to its native
// SMOOTH_MIN/SMOOTH_MAX/EXP_MIN clause. The lowering must reproduce value() exactly, and value()
// blends only the two closest (union) / two largest (intersection) of its N child values in a single
// Blend::eval call -- so only the arities where that selection is trivial lower natively:
//
//   - one child:  value() returns the child unblended, so lower the child directly (exact for ANY
//                 Blend, registered or not);
//   - two children: value() blends exactly those two, so emit one native smooth clause (the
//                 registered blends are all symmetric in (a, b), so child order is immaterial);
//   - N > 2:      two-closest-of-N is NOT a left-fold of binary blend clauses, so lowering it as a
//                 fold would change results -- keep the opaque host callback;
//   - unregistered custom Blend: no ISA opcode exists -- keep the opaque host callback.
//
// The host-callback fallback keeps tape evaluation == value() exact in every case, at the cost of
// device-eligibility.
template <class T, class Blend>
int
SmoothUnionIF<T, Blend>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  if (m_implicitFunctions.size() == 1) {
    return m_implicitFunctions.front()->flatten(a_builder, a_coordSlot);
  }

  if constexpr (isRegisteredBlend<Blend>) {
    if (m_implicitFunctions.size() == 2) {
      const int valueA = m_implicitFunctions[0]->flatten(a_builder, a_coordSlot);
      const int valueB = m_implicitFunctions[1]->flatten(a_builder, a_coordSlot);

      return a_builder.emitSmooth(BlendOpcodeOf<Blend>::value, valueA, valueB, m_smoothLen);
    }
  }

  return a_builder.emitHostCallback(a_coordSlot, this);
}

template <class T, class Blend>
int
SmoothIntersectionIF<T, Blend>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  if (m_implicitFunctions.size() == 1) {
    return m_implicitFunctions.front()->flatten(a_builder, a_coordSlot);
  }

  if constexpr (isRegisteredBlend<Blend>) {
    if (m_implicitFunctions.size() == 2) {
      const int valueA = m_implicitFunctions[0]->flatten(a_builder, a_coordSlot);
      const int valueB = m_implicitFunctions[1]->flatten(a_builder, a_coordSlot);

      return a_builder.emitSmooth(BlendOpcodeOf<Blend>::value, valueA, valueB, m_smoothLen);
    }
  }

  return a_builder.emitHostCallback(a_coordSlot, this);
}

// Smooth difference is stored as a SmoothIntersectionIF of A with complement(B): recurse into it so
// it takes the same native-or-host path (native for the pairwise A \ B with a registered Blend,
// host callback for multiple subtrahends or an unregistered Blend).
template <class T, class Blend>
int
SmoothDifferenceIF<T, Blend>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  return m_smoothIntersectionIF->flatten(a_builder, a_coordSlot);
}

// BVH unions lower to a native BVH clause plus deferred leaf subtrees. The builder receives a
// K-erased copy of the PackedBVH's node topology (bounding boxes, child tables, leaf primitive
// ranges) and the primitive list in leaf-traversal order -- exactly the order the nodes'
// primitive offsets index. Child tables are copied verbatim, including any duplicate entries from
// PackedBVH's power-of-K padding, so the tape traversal visits exactly what pruneTraverse would.
//
// On that padding: the BVH unions build their internal BVH exclusively through TreeBVH + pack()
// (CSGDetail::buildBVH), and both TreeBVH builds give every interior node K distinct children --
// only PackedBVH's direct SFC constructor pads a non-power-of-K leaf level by repeating the last
// real leaf, and no BVH-union code path reaches it today. Should such duplicates ever appear,
// copying them verbatim is still the right call: a duplicate leaf visit is idempotent for the
// sharp union's min-fold, and while it is NOT idempotent for the smooth union's two-smallest state
// in isolation (revisiting a leaf could seat the same primitive's value in both slots), today's
// pruneTraverse-based BVHSmoothUnionIF::value would visit those same duplicates through the same
// push/pop logic -- so the tape reproduces its behavior by construction either way.
namespace TapeDetail {

template <class T, class PackedBVHType>
[[nodiscard]] EBGEOMETRY_HOST std::vector<typename TapeBuilder<T>::BVHNodeInfo>
                              collectBVHNodeInfo(const PackedBVHType& a_bvh)
{
  const auto& nodes = a_bvh.getNodes();

  std::vector<typename TapeBuilder<T>::BVHNodeInfo> info(nodes.size());

  for (size_t i = 0; i < nodes.size(); i++) {
    info[i].aabb = nodes[i].getBoundingVolume();

    if (nodes[i].isLeaf()) {
      info[i].primOff  = nodes[i].getPrimitivesOffset();
      info[i].numPrims = nodes[i].getNumPrimitives();
    }
    else {
      const auto& offsets = nodes[i].getChildOffsets();

      info[i].children.assign(offsets.begin(), offsets.end());
    }
  }

  return info;
}

// Worst-case occupancy of the interpreter's fixed traversal stack for one BVH block: when the
// traversal descends into a child, the child's up-to-(childCount - 1) siblings stay stacked, so the
// occupancy along any root-to-leaf path is bounded by depth * (maxChildCount - 1) + 1. Computed
// iteratively (explicit stack, no recursion) so an adversarially deep, unbalanced tree -- the very
// case this guards against -- cannot overflow the host call stack while being measured.
template <class T>
[[nodiscard]] EBGEOMETRY_HOST uint32_t
tapeBVHStackBound(const std::vector<typename TapeBuilder<T>::BVHNodeInfo>& a_nodes)
{
  uint32_t maxDepth      = 0;
  uint32_t maxChildCount = 1;

  std::vector<std::pair<uint32_t, uint32_t>> stack;

  stack.emplace_back(0, 0);

  while (!stack.empty()) {
    const auto [node, depth] = stack.back();

    stack.pop_back();

    maxDepth = std::max(maxDepth, depth);

    for (const uint32_t child : a_nodes[node].children) {
      stack.emplace_back(child, depth + 1);
    }

    maxChildCount = std::max(maxChildCount, static_cast<uint32_t>(a_nodes[node].children.size()));
  }

  return maxDepth * (maxChildCount - 1) + 1;
}

template <class T, class PackedBVHType>
[[nodiscard]] EBGEOMETRY_HOST std::vector<const ImplicitFunction<T>*>
                              collectBVHLeafPrimitives(const PackedBVHType& a_bvh)
{
  const auto& primitives = a_bvh.getPrimitives();

  std::vector<const ImplicitFunction<T>*> leafPrimitives;

  leafPrimitives.reserve(primitives.size());

  for (const auto& primitive : primitives) {
    leafPrimitives.push_back(primitive.get());
  }

  return leafPrimitives;
}

} // namespace TapeDetail

template <class T, class P, class BV, size_t K>
int
BVHUnionIF<T, P, BV, K>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  static_assert(K <= TapeBVHMaxChildren, "BVHUnionIF::flatten: K exceeds the tape traversal child bound");

  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  // Nested BVH union (this union is a leaf primitive of an enclosing BVH union): Tier 6 keeps it
  // opaque -- the host callback runs this union's own pruneTraverse, which is exact but marks the
  // tape device-ineligible.
  if (a_builder.isFlatteningBVHLeaf()) {
    return a_builder.emitHostCallback(a_coordSlot, this);
  }

  auto nodes = TapeDetail::collectBVHNodeInfo<T>(*m_bvh);

  // An adversarially unbalanced tree (e.g. chain splits from degenerate centroid distributions)
  // could exceed the interpreter's fixed traversal stack; keep such a block opaque rather than risk
  // an out-of-bounds stack write in release builds.
  if (TapeDetail::tapeBVHStackBound<T>(nodes) > TapeBVHStackSize) {
    return a_builder.emitHostCallback(a_coordSlot, this);
  }

  return a_builder.emitBVHUnion(a_coordSlot,
                                std::move(nodes),
                                TapeDetail::collectBVHLeafPrimitives<T>(*m_bvh),
                                false,
                                Opcode::SMOOTH_MIN, // blend tag is unused for the sharp union
                                T(0));
}

template <class T, class P, class BV, size_t K, class Blend>
int
BVHSmoothUnionIF<T, P, BV, K, Blend>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  static_assert(K <= TapeBVHMaxChildren, "BVHSmoothUnionIF::flatten: K exceeds the tape traversal child bound");

  EBGEOMETRY_EXPECT(m_bvh != nullptr);

  // Native lowering requires a registered blend (one with a smooth ISA opcode) and must not run
  // inside another BVH union's leaf subtree (see BVHUnionIF::flatten above); everything else stays
  // an opaque host callback, which reproduces value() exactly.
  if constexpr (isRegisteredBlend<Blend>) {
    if (!a_builder.isFlatteningBVHLeaf()) {
      auto nodes = TapeDetail::collectBVHNodeInfo<T>(*m_bvh);

      // Same fixed-traversal-stack guard as BVHUnionIF::flatten above.
      if (TapeDetail::tapeBVHStackBound<T>(nodes) <= TapeBVHStackSize) {
        return a_builder.emitBVHUnion(a_coordSlot,
                                      std::move(nodes),
                                      TapeDetail::collectBVHLeafPrimitives<T>(*m_bvh),
                                      true,
                                      BlendOpcodeOf<Blend>::value,
                                      m_smoothLen);
      }
    }
  }

  return a_builder.emitHostCallback(a_coordSlot, this);
}

} // namespace EBGeometry

#endif
