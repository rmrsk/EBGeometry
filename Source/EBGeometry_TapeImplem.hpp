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
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
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

  const uint16_t paramIndex = static_cast<uint16_t>(array.size() - 1);

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

  const uint16_t paramIndex = static_cast<uint16_t>(array.size() - 1);

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

  const uint16_t paramIndex = static_cast<uint16_t>(array.size() - 1);

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

  const uint16_t paramIndex = static_cast<uint16_t>(m_tape.m_scalarParams.size() - 1);

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

  const uint16_t paramIndex = static_cast<uint16_t>(m_tape.m_hostNodes.size() - 1);

  m_tape.m_clauses.push_back(
    TapeClause{Opcode::HOST_CALLBACK, paramIndex, static_cast<uint32_t>(a_inCoord), 0, static_cast<uint32_t>(out)});

  m_tape.m_deviceEligible = false;

  return out;
}

template <class T>
Tape<T>
TapeBuilder<T>::finish(int a_rootValueSlot)
{
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
template <class T>
EBGEOMETRY_HOST_DEVICE T
TapeView<T>::value(const Vec3T<T>& a_point, Vec3T<T>* a_coordScratch, T* a_valueScratch) const noexcept
{
  EBGEOMETRY_EXPECT(numCoordSlots > 0);
  EBGEOMETRY_EXPECT(numValueSlots > 0);

  a_coordScratch[0] = a_point;

  for (uint32_t i = 0; i < numClauses; i++) {
    const TapeClause& c = clauses[i];

    switch (c.opcode) {
#define EBGEOMETRY_TAPE_EVAL_LEAF(TAG, OP, MEMBER)                                    \
  case Opcode::TAG:                                                                   \
    a_valueScratch[c.out] = OP<T>::eval(MEMBER[c.paramIndex], a_coordScratch[c.in0]); \
    break;
      EBGEOMETRY_TAPE_LEAF_OPS(EBGEOMETRY_TAPE_EVAL_LEAF)
#undef EBGEOMETRY_TAPE_EVAL_LEAF

#define EBGEOMETRY_TAPE_EVAL_MAPPOINT(TAG, OP, MEMBER)                                    \
  case Opcode::TAG:                                                                       \
    a_coordScratch[c.out] = OP<T>::mapPoint(MEMBER[c.paramIndex], a_coordScratch[c.in0]); \
    break;
      EBGEOMETRY_TAPE_MAPPOINT_OPS(EBGEOMETRY_TAPE_EVAL_MAPPOINT)
#undef EBGEOMETRY_TAPE_EVAL_MAPPOINT

#define EBGEOMETRY_TAPE_EVAL_MAPVALUE(TAG, OP, MEMBER)                                    \
  case Opcode::TAG:                                                                       \
    a_valueScratch[c.out] = OP<T>::mapValue(MEMBER[c.paramIndex], a_valueScratch[c.in0]); \
    break;
      EBGEOMETRY_TAPE_MAPVALUE_OPS(EBGEOMETRY_TAPE_EVAL_MAPVALUE)
#undef EBGEOMETRY_TAPE_EVAL_MAPVALUE

#define EBGEOMETRY_TAPE_EVAL_COMBINE(TAG, OP)                                             \
  case Opcode::TAG:                                                                       \
    a_valueScratch[c.out] = OP<T>::combine(a_valueScratch[c.in0], a_valueScratch[c.in1]); \
    break;
      EBGEOMETRY_TAPE_COMBINE_OPS(EBGEOMETRY_TAPE_EVAL_COMBINE)
#undef EBGEOMETRY_TAPE_EVAL_COMBINE

#define EBGEOMETRY_TAPE_EVAL_SMOOTH(TAG, OP)                                                                       \
  case Opcode::TAG:                                                                                                \
    a_valueScratch[c.out] = OP<T>::eval(a_valueScratch[c.in0], a_valueScratch[c.in1], scalarParams[c.paramIndex]); \
    break;
      EBGEOMETRY_TAPE_SMOOTH_OPS(EBGEOMETRY_TAPE_EVAL_SMOOTH)
#undef EBGEOMETRY_TAPE_EVAL_SMOOTH

    case Opcode::HOST_CALLBACK: {
#ifndef EBGEOMETRY_DEVICE_COMPILE
      a_valueScratch[c.out] = hostNodes[c.paramIndex]->value(a_coordScratch[c.in0]);
#else
      EBGEOMETRY_EXPECT(false && "HOST_CALLBACK clause is not evaluable on device");
#endif
      break;
    }
    }
  }

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
// Mollify, FiniteRepetition, BVH unions, and any user subclass).
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

} // namespace EBGeometry

#endif
