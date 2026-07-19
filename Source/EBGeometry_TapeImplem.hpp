// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TapeImplem.hpp
 * @brief  Implementation of EBGeometry_Tape.hpp.
 * @details Holds every out-of-line definition: the TapeBuilder methods, Tape::view, the free
 * flatten/evaluate functions, and every flatten() override body (the base opaque-host default, the
 * leaf template, and all transform/combiner nodes). Included last from EBGeometry_Tape.hpp so that
 * Tape, TapeView, TapeBuilder, and every implicit-function class are complete here.
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

// ── Free functions: flatten / evaluate ────────────────────────────────────────
template <class T>
Tape<T>
flatten(const ImplicitFunction<T>& a_tree)
{
  TapeBuilder<T> builder;

  const int rootCoordSlot = builder.newCoordSlot();
  const int rootValueSlot = a_tree.flatten(builder, rootCoordSlot);

  return builder.finish(rootValueSlot);
}

template <class T>
EBGEOMETRY_HOST_DEVICE T
evaluate(const TapeView<T>& a_view, const Vec3T<T>& a_point, Vec3T<T>* a_coordScratch, T* a_valueScratch) noexcept
{
  a_coordScratch[0] = a_point;

  for (uint32_t i = 0; i < a_view.numClauses; i++) {
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

  return a_valueScratch[a_view.rootValueSlot];
}

template <class T>
T
evaluate(const Tape<T>& a_tape, const Vec3T<T>& a_point)
{
  std::vector<Vec3T<T>> coordScratch(a_tape.getNumCoordSlots());
  std::vector<T>        valueScratch(a_tape.getNumValueSlots());

  return evaluate(a_tape.view(), a_point, coordScratch.data(), valueScratch.data());
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

// Smooth combiners carry a user-overridable std::function blend operator (SmoothMinIF's m_smoothMin,
// SmoothIntersectionIF's m_smoothMax) that defaults to SmoothMin/SmoothMax but may be any callable --
// including the enumerated ExpMin or an arbitrary user lambda. A std::function cannot be introspected
// to recover which enumerated ISA op it is, so, per the project rule that user-supplied std::function
// blends stay host-only, every smooth combiner lowers to an opaque host callback (keeping
// evaluate == value exact for any operator, at the cost of device-eligibility). The SMOOTH_MIN/
// SMOOTH_MAX/EXP_MIN opcodes remain the device ISA for blends emitted with a known operator (e.g. the
// builder API directly, and the BVH smooth union of a later tier); they are not reachable from a
// std::function-carrying node here.
template <class T>
int
SmoothUnionIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  return a_builder.emitHostCallback(a_coordSlot, this);
}

template <class T>
int
SmoothIntersectionIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  return a_builder.emitHostCallback(a_coordSlot, this);
}

// Smooth difference is stored as a SmoothIntersectionIF of A with complement(B): recurse into it so
// it takes the same opaque-host path.
template <class T>
int
SmoothDifferenceIF<T>::flatten(TapeBuilder<T>& a_builder, int a_coordSlot) const
{
  return m_smoothIntersectionIF->flatten(a_builder, a_coordSlot);
}

} // namespace EBGeometry

#endif
