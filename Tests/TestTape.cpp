// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_Tape.hpp: the flat linear-SSA tape and its single-pass interpreter.
// Validates the tape against the recursive value() oracle -- for every covered tree,
// compile(tree).value(p) must reproduce tree.value(p). It also checks device-eligibility
// bookkeeping: pure-primitive trees (including binary smooth combiners with a registered Blend and
// the BVH-accelerated unions, which lower to native BVH clauses with an in-interpreter pruned
// traversal) are device-eligible, while opaque-host fallbacks (Perlin, Blur, smooth combiners over
// more than two children, custom unregistered blends, nested BVH unions) are not.

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
  const Tape<T> tape = compile(a_tree);

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(tape.value(p), withinAbsT(a_tree.value(p), a_margin));
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
// (6) Smooth combiners are templated on their Blend operator, so a binary combiner with a
//     registered blend (SmoothMinOp/SmoothMaxOp/ExpMinOp) lowers to a native SMOOTH_MIN/
//     SMOOTH_MAX/EXP_MIN clause: it matches value() AND is device-eligible. An unregistered
//     custom blend still matches value() exactly, but via an opaque host callback.
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: binary smooth combiners with default blends lower natively and are device-eligible",
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

  // The default blends (SmoothMinOp/SmoothMaxOp) have registered ISA opcodes, so every binary
  // smooth combiner now compiles to a native smooth clause -- no host callback involved.
  REQUIRE(compile<T>(*smoothUnion).isDeviceEligible());
  REQUIRE(compile<T>(*smoothInter).isDeviceEligible());
  REQUIRE(compile<T>(*smoothDiff).isDeviceEligible());
}

TEMPLATE_TEST_CASE("Tape: an explicitly chosen ExpMinOp blend lowers natively and matches value()",
                   "[Tape][CSG][Smooth]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto a = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));
  const auto b = std::make_shared<SphereSDF<T>>(Vec3T<T>(3, 0, 0), T(1));

  const T s = T(0.5);

  const auto expUnion = SmoothUnion<T, ExpMinOp<T>>(a, b, s);

  requireTapeMatchesOracle<T>(*expUnion, formulaMargin<T>());

  REQUIRE(compile<T>(*expUnion).isDeviceEligible());
}

namespace {

// A custom smooth blend with the right trait shape but no BlendOpcodeOf registration: the smooth
// combiners must still reproduce value() exactly (via an opaque host callback), but the tape
// cannot be device-eligible.
template <class T>
struct QuadraticBlend
{
  static T
  eval(T a, T b, T s) noexcept
  {
    const T h = std::max(s - std::abs(a - b), T(0)) / s;

    return std::min(a, b) - T(0.5) * h * h * s;
  }
};

} // namespace

TEMPLATE_TEST_CASE("Tape: an unregistered custom blend matches value() via an opaque host node",
                   "[Tape][CSG][Smooth]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto a = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));
  const auto b = std::make_shared<SphereSDF<T>>(Vec3T<T>(3, 0, 0), T(1));

  const T s = T(0.5);

  const auto customUnion = SmoothUnion<T, QuadraticBlend<T>>(a, b, s);

  requireTapeMatchesOracle<T>(*customUnion, formulaMargin<T>());

  REQUIRE_FALSE(compile<T>(*customUnion).isDeviceEligible());
}

TEMPLATE_TEST_CASE("Tape: a single-child smooth union lowers to the child itself",
                   "[Tape][CSG][Smooth]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto a = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));

  // value() returns the lone child unblended, so the lowering is the child's own clause -- exact
  // and device-eligible even for an unregistered blend.
  const SmoothUnionIF<T>                    oneDefault({a}, T(0.5));
  const SmoothUnionIF<T, QuadraticBlend<T>> oneCustom({a}, T(0.5));

  requireTapeMatchesOracle<T>(oneDefault, exactMargin<T>());
  requireTapeMatchesOracle<T>(oneCustom, exactMargin<T>());

  REQUIRE(compile<T>(oneDefault).isDeviceEligible());
  REQUIRE(compile<T>(oneCustom).isDeviceEligible());
}

// The SMOOTH_MIN/SMOOTH_MAX/EXP_MIN opcodes are the device ISA for the registered blends. They are
// also exercised here directly through the builder API (the path a later BVH-smooth tier uses),
// decoupled from the smooth combiner nodes above.
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

      REQUIRE_THAT(tape.value(p), withinAbsT(expected, formulaMargin<T>()));
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

  // Every node here (including the binary smooth union, whose default SmoothMinOp blend lowers
  // natively) has a native clause, so the whole mixed tree is device-eligible.
  REQUIRE(compile<T>(*tree).isDeviceEligible());
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
  REQUIRE(compile<T>(*pureTree).isDeviceEligible());

  // Perlin noise is a direct-value node with no leaf trait -> opaque host callback.
  const PerlinSDF<T> perlin(T(1), Vec3T<T>(1, 1, 1), T(0.5), 2);
  const Tape<T>      perlinTape = compile<T>(perlin);
  REQUIRE_FALSE(perlinTape.isDeviceEligible());
  requireTapeMatchesOracle<T>(perlin, exactMargin<T>());

  // Blur wraps a child but overrides value() (no trait) -> opaque host callback.
  const auto    blurred  = Blur<T>(a, T(0.1));
  const Tape<T> blurTape = compile<T>(*blurred);
  REQUIRE_FALSE(blurTape.isDeviceEligible());
  requireTapeMatchesOracle<T>(*blurred, exactMargin<T>());
}

// ─────────────────────────────────────────────────────────────────────────────
// (9) Smooth combiner over more than two children -> opaque host fallback. value() blends only the
//     two closest of the N child values, which is not expressible as a fold of binary smooth
//     clauses, so even a registered blend must keep the host callback to stay exact.
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

  const auto    smoothN    = SmoothUnion<T>(three, T(0.5));
  const Tape<T> smoothTape = compile<T>(*smoothN);

  REQUIRE_FALSE(smoothTape.isDeviceEligible());
  requireTapeMatchesOracle<T>(*smoothN, formulaMargin<T>());
}

// ─────────────────────────────────────────────────────────────────────────────
// (10) Tape::value host convenience query (thread-local scratch)
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Tape: Tape::value matches the recursive value() oracle over mixed trees",
                   "[Tape][Value]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto sphere = std::make_shared<SphereSDF<T>>(Vec3T<T>(0.3, 0, 0), T(0.8));
  const auto box    = std::make_shared<BoxSDF<T>>(Vec3T<T>(-1, -1, -1), Vec3T<T>(1, 1, 1));

  // Union(Translate(Sphere), Box) and a nested-transform chain Offset(Rotate(Scale(Sphere))).
  const auto mixed  = Union<T>(Translate<T>(sphere, Vec3T<T>(0.4, -0.1, 0.2)), box);
  const auto nested = Offset<T>(Rotate<T>(Scale<T>(sphere, T(1.5)), T(30), 1), T(0.1));

  const Tape<T> mixedTape  = compile<T>(*mixed);
  const Tape<T> nestedTape = compile<T>(*nested);

  // Repeated queries on the same thread exercise the thread-local scratch reuse path.
  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(mixedTape.value(p), withinAbsT(mixed->value(p), exactMargin<T>()));
    REQUIRE_THAT(nestedTape.value(p), withinAbsT(nested->value(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: TapeView::value with caller-provided scratch matches Tape::value",
                   "[Tape][Value]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto a = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));
  const auto b = std::make_shared<SphereSDF<T>>(Vec3T<T>(3, 0, 0), T(1));
  const auto d = std::make_shared<SphereSDF<T>>(Vec3T<T>(0.8, 0, 0), T(1));

  const auto tree = Difference<T>(Union<T>(a, b), Scale<T>(d, T(1.5)));

  const Tape<T>     tape = compile<T>(*tree);
  const TapeView<T> view = tape.view();

  std::vector<Vec3T<T>> coordScratch(tape.getNumCoordSlots());
  std::vector<T>        valueScratch(tape.getNumValueSlots());

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(view.value(p, coordScratch.data(), valueScratch.data()), withinAbsT(tape.value(p), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// (11) BVH-accelerated unions -> native BVH clauses (Tier 6): the tape carries the PackedBVH's
//      topology and the interpreter runs a pruned traversal that executes leaf clause sub-ranges
//      from the deferred region on demand.
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// A row of disjoint unit spheres, centers a_spacing apart, with tight AABBs.
template <class T>
void
makeSphereRow(int                                         a_numSpheres,
              T                                           a_spacing,
              std::vector<std::shared_ptr<SphereSDF<T>>>& a_spheres,
              std::vector<BoundingVolumes::AABBT<T>>&     a_bvs)
{
  for (int i = 0; i < a_numSpheres; i++) {
    const Vec3T<T> center(T(i) * a_spacing, 0, 0);

    a_spheres.push_back(std::make_shared<SphereSDF<T>>(center, T(1)));
    a_bvs.emplace_back(center - Vec3T<T>::ones(), center + Vec3T<T>::ones());
  }
}

// A 6x6x6 grid (216 entries) of mixed, transformed primitives -- spheres, rotated boxes, scaled
// tori, offset capsules -- with conservative AABBs. Every entry is a metric SDF built from
// registered primitives/transforms only, so the whole scene lowers natively.
template <class T>
void
makeMixedScene(std::vector<std::shared_ptr<IF<T>>>& a_primitives, std::vector<BoundingVolumes::AABBT<T>>& a_bvs)
{
  constexpr int n = 6;

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < n; k++) {
        const Vec3T<T> center(T(3 * i), T(3 * j), T(3 * k));

        std::shared_ptr<IF<T>> primitive;

        switch ((i + j + k) % 4) {
        case 0: {
          primitive = Translate<T>(std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(0.8)), center);

          break;
        }
        case 1: {
          primitive = Translate<T>(
            Rotate<T>(std::make_shared<BoxSDF<T>>(Vec3T<T>(-0.7, -0.7, -0.7), Vec3T<T>(0.7, 0.7, 0.7)), T(30), 2),
            center);

          break;
        }
        case 2: {
          primitive =
            Translate<T>(Scale<T>(std::make_shared<TorusSDF<T>>(Vec3T<T>::zeros(), T(0.5), T(0.2)), T(1.5)), center);

          break;
        }
        default: {
          primitive = Translate<T>(
            Offset<T>(std::make_shared<CapsuleSDF<T>>(Vec3T<T>(0, -0.4, 0), Vec3T<T>(0, 0.4, 0), T(0.3)), T(0.1)),
            center);

          break;
        }
        }

        a_primitives.push_back(primitive);
        a_bvs.emplace_back(center - Vec3T<T>(1.5, 1.5, 1.5), center + Vec3T<T>(1.5, 1.5, 1.5));
      }
    }
  }
}

// A dense-ish grid of query points spanning the mixed scene (inside, on-surface, and far), plus
// the standard query spread.
template <class T>
std::vector<Vec3T<T>>
mixedScenePoints()
{
  std::vector<Vec3T<T>> points = queryPoints<T>();

  const T coords[6] = {T(-2.0), T(1.3), T(4.9), T(8.1), T(11.7), T(16.4)};

  for (const T x : coords) {
    for (const T y : coords) {
      for (const T z : coords) {
        points.emplace_back(x, y, z);
      }
    }
  }

  return points;
}

// Golden equivalence for one branching factor: the tape must be device-eligible and reproduce
// BVHUnionIF::value. Exact agreement is expected for metric primitives: both sides fold identical
// clause-evaluated leaf values, and branch-and-bound with a valid lower bound returns the exact
// minimum regardless of visited-set differences.
template <class T, size_t K>
void
requireBVHUnionGolden()
{
  using BV = BoundingVolumes::AABBT<T>;

  std::vector<std::shared_ptr<IF<T>>> primitives;
  std::vector<BV>                     bvs;

  makeMixedScene<T>(primitives, bvs);

  const BVHUnionIF<T, IF<T>, BV, K> bvhUnion(primitives, bvs);

  const Tape<T> tape = compile<T>(bvhUnion);

  REQUIRE(tape.isDeviceEligible());

  for (const auto& p : mixedScenePoints<T>()) {
    REQUIRE_THAT(tape.value(p), withinAbsT(bvhUnion.value(p), exactMargin<T>()));
  }
}

} // namespace

TEMPLATE_TEST_CASE("Tape: BVH union over mixed transformed primitives lowers natively (K in {2,4,8})",
                   "[Tape][BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  requireBVHUnionGolden<T, 2>();
  requireBVHUnionGolden<T, 4>();
  requireBVHUnionGolden<T, 8>();
}

TEMPLATE_TEST_CASE("Tape: BVH tape structure: deferred clause region and block/leaf tables",
                   "[Tape][BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T  = TestType;
  using BV = BoundingVolumes::AABBT<T>;

  constexpr int numSpheres = 4;

  std::vector<std::shared_ptr<SphereSDF<T>>> spheres;
  std::vector<BV>                            bvs;

  makeSphereRow<T>(numSpheres, T(3), spheres, bvs);

  const BVHUnionIF<T, SphereSDF<T>, BV, 2> bvhUnion(spheres, bvs);

  const Tape<T> tape = compile<T>(bvhUnion);

  // The whole main program is the single BVH clause; every sphere's subtree lives in the deferred
  // region behind it.
  REQUIRE(tape.getNumMainClauses() == 1);
  REQUIRE(tape.getNumMainClauses() < static_cast<uint32_t>(tape.getClauses().size()));
  REQUIRE(tape.getBVHBlocks().size() == 1);
  REQUIRE(tape.getBVHBlocks()[0].numLeaves == static_cast<uint32_t>(numSpheres));
  REQUIRE(tape.getBVHBlocks()[0].numNodes == static_cast<uint32_t>(tape.getBVHNodes().size()));
  REQUIRE(tape.getBVHLeaves().size() == static_cast<size_t>(numSpheres));

  for (const auto& leaf : tape.getBVHLeaves()) {
    REQUIRE(leaf.clauseBegin >= tape.getNumMainClauses());
    REQUIRE(leaf.clauseCount > 0);
    REQUIRE(leaf.clauseBegin + leaf.clauseCount <= static_cast<uint32_t>(tape.getClauses().size()));
  }

  // A tape without BVH clauses has no deferred region.
  const auto sphereA = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));
  const auto sphereB = std::make_shared<SphereSDF<T>>(Vec3T<T>(3, 0, 0), T(1));

  const Tape<T> plainTape = compile<T>(*Union<T>(sphereA, sphereB));

  REQUIRE(plainTape.getNumMainClauses() == static_cast<uint32_t>(plainTape.getClauses().size()));
  REQUIRE(plainTape.getBVHBlocks().empty());
}

namespace {

// A sphere that counts its value() evaluations. It has no flatten() override and no leaf trait, so
// inside a BVH leaf subtree it lowers to a HOST_CALLBACK clause -- allowed (the tape merely
// becomes host-only), and the counter then observes directly whether the interpreter's traversal
// executed or skipped that leaf's clause sub-range.
template <class T>
class CountingSphereIF : public ImplicitFunction<T>
{
public:
  CountingSphereIF(const Vec3T<T>& a_center, const T a_radius, int* a_counter) noexcept
    : m_sphere(a_center, a_radius), m_counter(a_counter)
  {}

  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override
  {
    (*m_counter)++;

    return m_sphere.value(a_point);
  }

private:
  SphereSDF<T> m_sphere;
  int*         m_counter;
};

} // namespace

TEMPLATE_TEST_CASE("Tape: BVH traversal prunes leaf clause sub-ranges (observed via a counting leaf)",
                   "[Tape][BVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T  = TestType;
  using BV = BoundingVolumes::AABBT<T>;

  int counter = 0;

  // Four unit spheres near the origin plus a counting sphere far away at x = 30.
  std::vector<std::shared_ptr<IF<T>>> primitives;
  std::vector<BV>                     bvs;

  for (int i = 0; i < 4; i++) {
    const Vec3T<T> center(T(3 * i), 0, 0);

    primitives.push_back(std::make_shared<SphereSDF<T>>(center, T(1)));
    bvs.emplace_back(center - Vec3T<T>::ones(), center + Vec3T<T>::ones());
  }

  const Vec3T<T> farCenter(T(30), 0, 0);

  primitives.push_back(std::make_shared<CountingSphereIF<T>>(farCenter, T(1), &counter));
  bvs.emplace_back(farCenter - Vec3T<T>::ones(), farCenter + Vec3T<T>::ones());

  const BVHUnionIF<T, IF<T>, BV, 2> bvhUnion(primitives, bvs);

  const Tape<T> tape = compile<T>(bvhUnion);

  // The counting leaf's subtree is a host callback, so the tape is host-only -- but the BVH clause
  // itself is still native.
  REQUIRE_FALSE(tape.isDeviceEligible());
  REQUIRE(counter == 0);

  // Querying inside the first sphere gives a negative running best, collapsing the pruning bound
  // to zero: the far leaf's sub-range must never execute.
  const T nearValue = tape.value(Vec3T<T>::zeros());

  REQUIRE(counter == 0);
  REQUIRE_THAT(nearValue, withinAbsT(T(-1), exactMargin<T>()));

  // Querying at the far sphere's center must execute its sub-range.
  const T farValue = tape.value(farCenter);

  REQUIRE(counter > 0);
  REQUIRE_THAT(farValue, withinAbsT(T(-1), exactMargin<T>()));
}

TEMPLATE_TEST_CASE("Tape: a nested BVH union lowers to a host callback and stays exact",
                   "[Tape][BVH][DeviceEligible]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T  = TestType;
  using BV = BoundingVolumes::AABBT<T>;

  std::vector<std::shared_ptr<SphereSDF<T>>> innerSpheres;
  std::vector<BV>                            innerBVs;

  makeSphereRow<T>(4, T(3), innerSpheres, innerBVs);

  const auto inner = std::make_shared<BVHUnionIF<T, SphereSDF<T>, BV, 2>>(innerSpheres, innerBVs);

  // Outer union: the inner BVH union is one leaf primitive among ordinary spheres.
  std::vector<std::shared_ptr<IF<T>>> outerPrimitives;
  std::vector<BV>                     outerBVs;

  outerPrimitives.push_back(inner);
  outerBVs.push_back(inner->getBoundingVolume());

  for (int i = 0; i < 3; i++) {
    const Vec3T<T> center(T(3 * i), T(5), 0);

    outerPrimitives.push_back(std::make_shared<SphereSDF<T>>(center, T(1)));
    outerBVs.emplace_back(center - Vec3T<T>::ones(), center + Vec3T<T>::ones());
  }

  const BVHUnionIF<T, IF<T>, BV, 2> outer(outerPrimitives, outerBVs);

  const Tape<T> tape = compile<T>(outer);

  // The inner union's leaf subtree is an opaque host callback (Tier 6 forbids nested BVH clauses),
  // so the tape is host-only -- but values still match the recursive oracle exactly.
  REQUIRE_FALSE(tape.isDeviceEligible());

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(tape.value(p), withinAbsT(outer.value(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: BVH smooth union with the default SmoothMinOp blend lowers natively",
                   "[Tape][BVH][Smooth]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T  = TestType;
  using BV = BoundingVolumes::AABBT<T>;

  // Overlapping spheres (centers 1.5 apart, radius 1) so the two-nearest blend is active over much
  // of the row.
  std::vector<std::shared_ptr<SphereSDF<T>>> spheres;
  std::vector<BV>                            bvs;

  makeSphereRow<T>(8, T(1.5), spheres, bvs);

  const T smoothLen = T(0.25);

  const BVHSmoothUnionIF<T, SphereSDF<T>, BV, 4> smooth(spheres, bvs, smoothLen);

  const Tape<T> tape = compile<T>(smooth);

  REQUIRE(tape.isDeviceEligible());

  for (const auto& p : mixedScenePoints<T>()) {
    REQUIRE_THAT(tape.value(p), withinAbsT(smooth.value(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: BVH smooth union with an explicit ExpMinOp blend lowers natively",
                   "[Tape][BVH][Smooth]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T  = TestType;
  using BV = BoundingVolumes::AABBT<T>;

  std::vector<std::shared_ptr<SphereSDF<T>>> spheres;
  std::vector<BV>                            bvs;

  makeSphereRow<T>(8, T(1.5), spheres, bvs);

  const T smoothLen = T(0.25);

  const BVHSmoothUnionIF<T, SphereSDF<T>, BV, 4, ExpMinOp<T>> smooth(spheres, bvs, smoothLen);

  const Tape<T> tape = compile<T>(smooth);

  REQUIRE(tape.isDeviceEligible());

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(tape.value(p), withinAbsT(smooth.value(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: BVH smooth union with an unregistered custom blend falls back to a host callback",
                   "[Tape][BVH][Smooth][DeviceEligible]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T  = TestType;
  using BV = BoundingVolumes::AABBT<T>;

  std::vector<std::shared_ptr<SphereSDF<T>>> spheres;
  std::vector<BV>                            bvs;

  makeSphereRow<T>(8, T(1.5), spheres, bvs);

  const T smoothLen = T(0.25);

  const BVHSmoothUnionIF<T, SphereSDF<T>, BV, 4, QuadraticBlend<T>> smooth(spheres, bvs, smoothLen);

  const Tape<T> tape = compile<T>(smooth);

  REQUIRE_FALSE(tape.isDeviceEligible());

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(tape.value(p), withinAbsT(smooth.value(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: single-primitive BVH smooth union blends against +inf, matching value()",
                   "[Tape][BVH][Smooth]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T  = TestType;
  using BV = BoundingVolumes::AABBT<T>;

  std::vector<std::shared_ptr<SphereSDF<T>>> spheres{std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1))};
  std::vector<BV>                            bvs{BV(-Vec3T<T>::ones(), Vec3T<T>::ones())};

  const BVHSmoothUnionIF<T, SphereSDF<T>, BV, 2> smooth(spheres, bvs, T(0.25));

  const Tape<T> tape = compile<T>(smooth);

  REQUIRE(tape.isDeviceEligible());

  // With a single primitive the second-nearest value stays +inf and the blend must reduce to the
  // primitive's own value, exactly as in BVHSmoothUnionIF::value.
  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(tape.value(p), withinAbsT(smooth.value(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: BVH leaf subtrees share one scratch slot window",
                   "[Tape][BVH][Slots]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T  = TestType;
  using BV = BoundingVolumes::AABBT<T>;

  constexpr int numLeaves = 128;

  // Every leaf is Translate(Sphere): one coordinate slot (the translated frame) plus one value
  // slot (the sphere) per leaf if slots were disjoint.
  std::vector<std::shared_ptr<IF<T>>> primitives;
  std::vector<BV>                     bvs;

  for (int i = 0; i < numLeaves; i++) {
    const Vec3T<T> center(T(3 * i), 0, 0);

    primitives.push_back(Translate<T>(std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1)), center));
    bvs.emplace_back(center - Vec3T<T>::ones(), center + Vec3T<T>::ones());
  }

  const BVHUnionIF<T, IF<T>, BV, 8> bvhUnion(primitives, bvs);

  const Tape<T> tape = compile<T>(bvhUnion);

  REQUIRE(tape.isDeviceEligible());

  // Leaf sub-ranges execute strictly sequentially and each leaf's value slot is folded immediately
  // after its range runs, so all 128 leaves share one slot window: scratch is main program + max
  // over leaves, not main + sum. Disjoint slots would need >= numLeaves entries here.
  REQUIRE(tape.getNumValueSlots() < static_cast<uint32_t>(numLeaves));
  REQUIRE(tape.getNumCoordSlots() < static_cast<uint32_t>(numLeaves));

  // Exact budget for this tape: {BVH output, shared leaf value} and {root frame, shared translated
  // frame}.
  REQUIRE(tape.getNumValueSlots() == 2);
  REQUIRE(tape.getNumCoordSlots() == 2);

  const BVHUnionIF<T, IF<T>, BV, 8>& oracle = bvhUnion;

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(tape.value(p), withinAbsT(oracle.value(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: TapeView::value with exactly-sized caller scratch works on a BVH tape",
                   "[Tape][BVH][Value]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T  = TestType;
  using BV = BoundingVolumes::AABBT<T>;

  std::vector<std::shared_ptr<IF<T>>> primitives;
  std::vector<BV>                     bvs;

  makeMixedScene<T>(primitives, bvs);

  const BVHUnionIF<T, IF<T>, BV, 4> bvhUnion(primitives, bvs);

  const Tape<T>     tape = compile<T>(bvhUnion);
  const TapeView<T> view = tape.view();

  // Buffers sized to exactly the advertised slot counts: the deferred leaf sub-ranges must fit in
  // the same scratch as the main pass (the shared slot window), with no out-of-bounds access
  // (verified under ASan in the debug-san preset).
  std::vector<Vec3T<T>> coordScratch(tape.getNumCoordSlots());
  std::vector<T>        valueScratch(tape.getNumValueSlots());

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(view.value(p, coordScratch.data(), valueScratch.data()), withinAbsT(tape.value(p), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// (12) Re-entrancy: a host-callback node whose value() itself evaluates a tape
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// An implicit function backed by its own compiled tape -- the natural adapter a user writes once
// tapes exist. It has no flatten() override, so an enclosing compile() lowers it to an opaque host
// callback, and evaluating the enclosing tape re-enters Tape::value on the same thread mid-pass.
// Tape::value must detect this and give the nested pass its own scratch instead of resizing or
// clobbering the outer pass's thread-local buffers.
template <class T>
class TapeBackedIF : public ImplicitFunction<T>
{
public:
  explicit TapeBackedIF(const std::shared_ptr<ImplicitFunction<T>>& a_tree) : m_tree(a_tree), m_tape(compile(*a_tree))
  {}

  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override
  {
    return m_tape.value(a_point);
  }

private:
  std::shared_ptr<ImplicitFunction<T>> m_tree;
  Tape<T>                              m_tape;
};

} // namespace

TEMPLATE_TEST_CASE("Tape: nested tape evaluation through a host callback is safe",
                   "[Tape][HostCallback][Reentrancy]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // The inner tree is deliberately deeper (more slots) than the outer tape, so a naively shared
  // scratch buffer would be regrown and overwritten by the nested pass.
  const auto box   = std::make_shared<BoxSDF<T>>(Vec3T<T>(-1, -1, -1), Vec3T<T>(1, 1, 1));
  const auto inner = Offset<T>(
    Annular<T>(Translate<T>(Rotate<T>(Scale<T>(box, T(1.5)), T(20), 1), Vec3T<T>(0.5, 0, 0)), T(0.1)), T(0.2));

  const auto adapter = std::make_shared<TapeBackedIF<T>>(inner);
  const auto sphere  = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));

  // Union(sphere, adapter): the outer tape computes the sphere's value slot first, then runs the
  // adapter's host callback (which evaluates the inner tape on this same thread), then combines --
  // so a clobbered sphere slot or a reallocated scratch buffer would corrupt the result.
  const auto outer = Union<T>(sphere, adapter);

  const Tape<T> tape = compile<T>(*outer);

  REQUIRE_FALSE(tape.isDeviceEligible());

  for (const auto& p : queryPoints<T>()) {
    REQUIRE_THAT(tape.value(p), withinAbsT(outer->value(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: nested tape evaluation through a BVH leaf host callback is safe",
                   "[Tape][BVH][HostCallback][Reentrancy]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T  = TestType;
  using BV = BoundingVolumes::AABBT<T>;

  // The adapter's inner tree is deliberately deeper (more slots) than the enclosing BVH tape, so a
  // naively shared scratch buffer would be regrown and overwritten when the BVH leaf's HOST_CALLBACK
  // re-enters Tape::value mid-traversal on this same thread.
  const Vec3T<T> adapterCenter(T(9), 0, 0);

  const auto box   = std::make_shared<BoxSDF<T>>(Vec3T<T>(-1, -1, -1), Vec3T<T>(1, 1, 1));
  const auto inner = Offset<T>(Translate<T>(Rotate<T>(Scale<T>(box, T(0.5)), T(20), 1), adapterCenter), T(0.1));

  const auto adapter = std::make_shared<TapeBackedIF<T>>(inner);

  std::vector<std::shared_ptr<IF<T>>> primitives;
  std::vector<BV>                     bvs;

  for (int i = 0; i < 3; i++) {
    const Vec3T<T> center(T(3 * i), 0, 0);

    primitives.push_back(std::make_shared<SphereSDF<T>>(center, T(1)));
    bvs.emplace_back(center - Vec3T<T>::ones(), center + Vec3T<T>::ones());
  }

  primitives.push_back(adapter);
  bvs.emplace_back(adapterCenter - Vec3T<T>(2, 2, 2), adapterCenter + Vec3T<T>(2, 2, 2));

  const BVHUnionIF<T, IF<T>, BV, 2> bvhUnion(primitives, bvs);

  const Tape<T> tape = compile<T>(bvhUnion);

  REQUIRE_FALSE(tape.isDeviceEligible());

  // Points near the adapter force its leaf sub-range (and thus the nested tape evaluation) to run;
  // the standard spread covers the pruned side too.
  std::vector<Vec3T<T>> points = queryPoints<T>();

  points.push_back(adapterCenter);
  points.emplace_back(T(8.4), T(0.2), T(-0.1));

  for (const auto& p : points) {
    REQUIRE_THAT(tape.value(p), withinAbsT(bvhUnion.value(p), exactMargin<T>()));
  }
}
