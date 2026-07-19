// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_Tape.hpp: the flat linear-SSA tape and its single-pass interpreter.
// Tier 4 validates the tape against the recursive value() oracle -- for every covered tree,
// evaluate(flatten(tree), p) must reproduce tree.value(p). It also checks device-eligibility
// bookkeeping: pure-primitive trees are device-eligible, while opaque-host fallbacks (Perlin, Blur,
// smooth combiners over more than two children) are not.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <memory>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

template <class T>
using IF = ImplicitFunction<T>;

// Two independent traversals of the same trait statics (recursive value() vs. the linear tape)
// should agree up to floating-point reordering only -- no genuine approximation is involved, since
// the tape reuses the exact same eval/mapPoint/mapValue/combine formulas.
template <class T>
double
exactMargin()
{
  return std::is_same_v<T, float> ? 1.0e-4 : 1.0e-12;
}

// A slightly looser bound for smooth blends, whose closed form involves an sqrt-free but
// division-heavy polynomial.
template <class T>
double
formulaMargin()
{
  return std::is_same_v<T, float> ? 1.0e-3 : 1.0e-9;
}

// A spread of inside/outside/surface-ish query points that exercises every primitive and combiner.
template <class T>
std::vector<Vec3T<T>>
queryPoints()
{
  return {Vec3T<T>::zeros(),
          Vec3T<T>(0.5, 0, 0),
          Vec3T<T>(1.5, 0, 0),
          Vec3T<T>(-1.2, 0.7, 0.3),
          Vec3T<T>(2, 2, 2),
          Vec3T<T>(-3, 1, -2),
          Vec3T<T>(0.1, -0.4, 0.9),
          Vec3T<T>(3, 0, 0),
          Vec3T<T>(0, 3, 0),
          Vec3T<T>(-0.5, -0.5, -0.5)};
}

// Core oracle check: the tape must reproduce value() at every query point.
template <class T>
void
requireTapeMatchesOracle(const ImplicitFunction<T>& a_tree, const double a_margin)
{
  const Tape<T> tape = flatten(a_tree);

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(evaluate(tape, p), withinAbsT(a_tree.value(p), a_margin));
  }
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// (1) Every analytic primitive alone
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: every analytic primitive matches its value() oracle",
                   "[Tape][Primitive]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  requireTapeMatchesOracle<T>(PlaneSDF<T>(Vec3T<T>::zeros(), Vec3T<T>(0, 1, 0)), exactMargin<T>());
  requireTapeMatchesOracle<T>(SphereSDF<T>(Vec3T<T>::zeros(), T(1)), exactMargin<T>());
  requireTapeMatchesOracle<T>(BoxSDF<T>(Vec3T<T>(-1, -1, -1), Vec3T<T>(1, 1, 1)), exactMargin<T>());
  requireTapeMatchesOracle<T>(TorusSDF<T>(Vec3T<T>::zeros(), T(1), T(0.4)), exactMargin<T>());
  requireTapeMatchesOracle<T>(CylinderSDF<T>(Vec3T<T>(0, -1, 0), Vec3T<T>(0, 1, 0), T(0.7)), exactMargin<T>());
  requireTapeMatchesOracle<T>(InfiniteCylinderSDF<T>(Vec3T<T>::zeros(), T(0.7), 1), exactMargin<T>());
  requireTapeMatchesOracle<T>(CapsuleSDF<T>(Vec3T<T>(0, -1, 0), Vec3T<T>(0, 1, 0), T(0.5)), exactMargin<T>());
  requireTapeMatchesOracle<T>(InfiniteConeSDF<T>(Vec3T<T>::zeros(), T(30)), exactMargin<T>());
  requireTapeMatchesOracle<T>(ConeSDF<T>(Vec3T<T>::zeros(), T(1), T(30)), exactMargin<T>());
  requireTapeMatchesOracle<T>(RoundedBoxSDF<T>(Vec3T<T>(1, 1, 1), T(0.2)), exactMargin<T>());
  requireTapeMatchesOracle<T>(RoundedCylinderSDF<T>(T(1), T(0.2), T(1)), exactMargin<T>());
}

// ─────────────────────────────────────────────────────────────────────────────
// (2) Coordinate transforms over a sphere (incl. Scale's two-clause straddle)
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: single coordinate transforms over a sphere match value()",
                   "[Tape][Transform]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto sphere = std::make_shared<SphereSDF<T>>(Vec3T<T>(0.3, 0, 0), T(0.8));

  requireTapeMatchesOracle<T>(*Translate<T>(sphere, Vec3T<T>(1, 0.5, -0.2)), exactMargin<T>());
  requireTapeMatchesOracle<T>(*Rotate<T>(sphere, T(35), 2), exactMargin<T>());
  requireTapeMatchesOracle<T>(*Scale<T>(sphere, T(2)), exactMargin<T>());
  requireTapeMatchesOracle<T>(*Elongate<T>(sphere, Vec3T<T>(0.3, 0.1, 0.0)), exactMargin<T>());
  requireTapeMatchesOracle<T>(*Reflect<T>(sphere, size_t(0)), exactMargin<T>());
}

// ─────────────────────────────────────────────────────────────────────────────
// (3) Value transforms over a box
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: single value transforms over a box match value()",
                   "[Tape][Transform]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto box = std::make_shared<BoxSDF<T>>(Vec3T<T>(-1, -1, -1), Vec3T<T>(1, 1, 1));

  requireTapeMatchesOracle<T>(*Complement<T>(box), exactMargin<T>());
  requireTapeMatchesOracle<T>(*Offset<T>(box, T(0.25)), exactMargin<T>());
  requireTapeMatchesOracle<T>(*Annular<T>(box, T(0.15)), exactMargin<T>());
}

// ─────────────────────────────────────────────────────────────────────────────
// (4) Nested transforms
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: nested transform chains match value()", "[Tape][Transform]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto sphere = std::make_shared<SphereSDF<T>>(Vec3T<T>(0.3, 0, 0), T(0.8));
  const auto box    = std::make_shared<BoxSDF<T>>(Vec3T<T>(-1, -1, -1), Vec3T<T>(1, 1, 1));

  // Translate(Rotate(Scale(Sphere))).
  requireTapeMatchesOracle<T>(*Translate<T>(Rotate<T>(Scale<T>(sphere, T(1.5)), T(20), 1), Vec3T<T>(0.5, 0, 0)),
                              exactMargin<T>());

  // Offset(Annular(Translate(Box))).
  requireTapeMatchesOracle<T>(*Offset<T>(Annular<T>(Translate<T>(box, Vec3T<T>(0.3, 0, 0)), T(0.1)), T(0.2)),
                              exactMargin<T>());
}

// ─────────────────────────────────────────────────────────────────────────────
// (5) Sharp combiners
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: sharp union/intersection/difference match value(), binary and N>=3",
                   "[Tape][CSG]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto a = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));
  const auto b = std::make_shared<SphereSDF<T>>(Vec3T<T>(3, 0, 0), T(1));
  const auto d = std::make_shared<SphereSDF<T>>(Vec3T<T>(0.8, 0, 0), T(1));

  // Binary.
  requireTapeMatchesOracle<T>(*Union<T>(a, b), exactMargin<T>());
  requireTapeMatchesOracle<T>(*Intersection<T>(a, d), exactMargin<T>());
  requireTapeMatchesOracle<T>(*Difference<T>(a, d), exactMargin<T>());

  // N >= 3 (left fold of pairwise combines).
  const std::vector<std::shared_ptr<SphereSDF<T>>> three{a, b, d};
  requireTapeMatchesOracle<T>(*Union<T, SphereSDF<T>>(three), exactMargin<T>());
  requireTapeMatchesOracle<T>(*Intersection<T, SphereSDF<T>>(three), exactMargin<T>());

  // Difference(A, {B, E}) list constructor -- B becomes an internal UnionIF of subtrahends.
  const auto            e = std::make_shared<SphereSDF<T>>(Vec3T<T>(0.5, 0, 0), T(0.3));
  const DifferenceIF<T> diffList(a, std::vector<std::shared_ptr<IF<T>>>{b, e});
  requireTapeMatchesOracle<T>(diffList, exactMargin<T>());
}

// ─────────────────────────────────────────────────────────────────────────────
// (6) Smooth combiners lower to an opaque host callback (their blend operator is a
//     user-overridable std::function that cannot be verified against the ISA), so they still match
//     value() exactly but are not device-eligible.
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: smooth combiners match value() via an opaque host node",
                   "[Tape][CSG][Smooth]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto a = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));
  const auto b = std::make_shared<SphereSDF<T>>(Vec3T<T>(3, 0, 0), T(1));
  const auto d = std::make_shared<SphereSDF<T>>(Vec3T<T>(0.8, 0, 0), T(1));

  const T s = T(0.5);

  const auto smoothUnion = SmoothUnion<T>(a, b, s);
  const auto smoothInter = SmoothIntersection<T>(a, d, s);
  const auto smoothDiff  = SmoothDifference<T>(a, d, s);

  requireTapeMatchesOracle<T>(*smoothUnion, formulaMargin<T>());
  requireTapeMatchesOracle<T>(*smoothInter, formulaMargin<T>());
  requireTapeMatchesOracle<T>(*smoothDiff, formulaMargin<T>());

  // A std::function blend operator keeps every smooth combiner host-only.
  REQUIRE_FALSE(flatten<T>(*smoothUnion).isDeviceEligible());
  REQUIRE_FALSE(flatten<T>(*smoothInter).isDeviceEligible());
  REQUIRE_FALSE(flatten<T>(*smoothDiff).isDeviceEligible());
}

// The SMOOTH_MIN/SMOOTH_MAX/EXP_MIN opcodes are the device ISA for blends emitted with a known
// operator. They are exercised here directly through the builder API (the path a later BVH-smooth
// tier uses), decoupled from the std::function-carrying smooth combiner nodes above.
TEMPLATE_TEST_CASE("Tape: smooth ISA opcodes match their trait eval via the builder",
                   "[Tape][Smooth][Opcode]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const typename SphereOp<T>::Params pa{Vec3T<T>::zeros(), T(1)};
  const typename SphereOp<T>::Params pb{Vec3T<T>(2, 0, 0), T(1)};

  const T s = T(0.5);

  for (const Opcode op : {Opcode::SMOOTH_MIN, Opcode::SMOOTH_MAX, Opcode::EXP_MIN}) {
    TapeBuilder<T> builder;

    const int coord0 = builder.newCoordSlot();
    const int valueA = builder.template emitLeaf<SphereOp<T>>(coord0, pa);
    const int valueB = builder.template emitLeaf<SphereOp<T>>(coord0, pb);
    const int root   = builder.emitSmooth(op, valueA, valueB, s);

    const Tape<T> tape = builder.finish(root);

    REQUIRE(tape.isDeviceEligible());

    for (const auto& p : queryPoints<T>()) {
      const T va       = SphereOp<T>::eval(pa, p);
      const T vb       = SphereOp<T>::eval(pb, p);
      const T expected = (op == Opcode::SMOOTH_MIN)   ? SmoothMinOp<T>::eval(va, vb, s)
                         : (op == Opcode::SMOOTH_MAX) ? SmoothMaxOp<T>::eval(va, vb, s)
                                                      : ExpMinOp<T>::eval(va, vb, s);

      REQUIRE_THAT(evaluate<T>(tape, p), withinAbsT(expected, formulaMargin<T>()));
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// (7) One deep, mixed tree
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: deep mixed tree matches value()", "[Tape][CSG][Smooth]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto sphere = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));
  const auto box    = std::make_shared<BoxSDF<T>>(Vec3T<T>(-0.8, -0.8, -0.8), Vec3T<T>(0.8, 0.8, 0.8));
  const auto cyl    = std::make_shared<CylinderSDF<T>>(Vec3T<T>(0, -1, 0), Vec3T<T>(0, 1, 0), T(0.5));

  const T s = T(0.4);

  // Difference(SmoothUnion(Translate(Sphere), Box, s), Rotate(Cylinder)).
  const auto smoothUnion = SmoothUnion<T>(Translate<T>(sphere, Vec3T<T>(0.2, 0, 0)), box, s);
  const auto tree        = Difference<T>(smoothUnion, Rotate<T>(cyl, T(25), 0));

  requireTapeMatchesOracle<T>(*tree, formulaMargin<T>());
}

// ─────────────────────────────────────────────────────────────────────────────
// (8) Opaque-host fallback and device eligibility
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: pure-primitive trees are device-eligible; opaque nodes are not",
                   "[Tape][DeviceEligible]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto a = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));
  const auto b = std::make_shared<SphereSDF<T>>(Vec3T<T>(3, 0, 0), T(1));
  const auto d = std::make_shared<SphereSDF<T>>(Vec3T<T>(0.8, 0, 0), T(1));

  // A pure primitive + transforms + sharp combiners tree stays device-eligible.
  const auto pureTree = Difference<T>(Union<T>(a, b), Scale<T>(d, T(1.5)));
  REQUIRE(flatten<T>(*pureTree).isDeviceEligible());

  // Perlin noise is a direct-value node with no leaf trait -> opaque host callback.
  const PerlinSDF<T> perlin(T(1), Vec3T<T>(1, 1, 1), T(0.5), 2);
  const Tape<T>      perlinTape = flatten<T>(perlin);
  REQUIRE_FALSE(perlinTape.isDeviceEligible());
  requireTapeMatchesOracle<T>(perlin, exactMargin<T>());

  // Blur wraps a child but overrides value() (no trait) -> opaque host callback.
  const auto    blurred  = Blur<T>(a, T(0.1));
  const Tape<T> blurTape = flatten<T>(*blurred);
  REQUIRE_FALSE(blurTape.isDeviceEligible());
  requireTapeMatchesOracle<T>(*blurred, exactMargin<T>());
}

// ─────────────────────────────────────────────────────────────────────────────
// (9) Smooth combiner over more than two children -> opaque host fallback
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: smooth union over three children falls back to an opaque host node",
                   "[Tape][DeviceEligible][Smooth]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto a = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));
  const auto b = std::make_shared<SphereSDF<T>>(Vec3T<T>(3, 0, 0), T(1));
  const auto c = std::make_shared<SphereSDF<T>>(Vec3T<T>(1.5, 0, 0), T(1));

  const std::vector<std::shared_ptr<SphereSDF<T>>> three{a, b, c};

  const auto    smoothN    = SmoothUnion<T, SphereSDF<T>>(three, T(0.5));
  const Tape<T> smoothTape = flatten<T>(*smoothN);

  REQUIRE_FALSE(smoothTape.isDeviceEligible());
  requireTapeMatchesOracle<T>(*smoothN, formulaMargin<T>());
}
