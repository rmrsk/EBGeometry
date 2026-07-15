// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_CSG.hpp: the sharp and smoothly-blended CSG combinators (union,
// intersection, difference), their BVH-accelerated counterparts, finite periodic repetition, and
// the SmoothMin/SmoothMax/ExpMin blending primitives they're built on. Uses analytic spheres as
// fixtures so every expected value is hand-computable.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cmath>
#include <type_traits>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

template <class T>
using Sphere = SphereSDF<T>;

template <class T>
using IF = ImplicitFunction<T>;

template <class T>
using BV = BoundingVolumes::AABBT<T>;

template <class T>
BV<T>
sphereBV(const Vec3T<T>& a_center, const T a_radius)
{
  return {a_center - a_radius * Vec3T<T>::ones(), a_center + a_radius * Vec3T<T>::ones()};
}

// Two well-separated (non-overlapping) unit-ish spheres: centers 3 apart, radius 1 each, so
// there's a gap of 1 between them. Every sharp-CSG expected value below is hand-computable from
// these two spheres' own signedDistance().
template <class T>
std::shared_ptr<Sphere<T>>
sphereA()
{
  return std::make_shared<Sphere<T>>(Vec3T<T>::zeros(), T(1));
}

template <class T>
std::shared_ptr<Sphere<T>>
sphereB()
{
  return std::make_shared<Sphere<T>>(Vec3T<T>(3, 0, 0), T(1));
}

// Two substantially overlapping spheres, used where a non-trivial (both-negative) intersection
// region is needed.
template <class T>
std::shared_ptr<Sphere<T>>
sphereC()
{
  return std::make_shared<Sphere<T>>(Vec3T<T>::zeros(), T(2));
}

template <class T>
std::shared_ptr<Sphere<T>>
sphereD()
{
  return std::make_shared<Sphere<T>>(Vec3T<T>(1, 0, 0), T(2));
}

// Evenly-spaced sweep of `a_count` values in [a_lo, a_hi], inclusive of both endpoints. Used
// instead of a floating-point loop counter (flagged by clang-tidy's FloatLoopCounter check,
// since float accumulation drift can make the number of iterations non-obvious).
template <class T>
std::vector<T>
sweepValues(const T a_lo, const T a_hi, const int a_count)
{
  std::vector<T> values;
  values.reserve(static_cast<size_t>(a_count) + 1);
  for (int i = 0; i <= a_count; i++) {
    values.push_back(a_lo + (a_hi - a_lo) * T(i) / T(a_count));
  }
  return values;
}

// Two independent code paths computing literally the same closed-form expression (a free
// function vs. its *IF class, a pairwise vs. vector-of-functions constructor, ...) should agree
// up to floating-point reordering only -- no genuine approximation involved.
template <class T>
double
exactMargin()
{
  return std::is_same_v<T, float> ? 1.0e-4 : 1.0e-12;
}

// A hand-computed closed-form expected value (SmoothMin's exact blend formula, FiniteRepetition's
// tile folding, ...), or a "never worse than the sharp value" bound that holds by construction of
// the blending formula, swept over many sample points.
template <class T>
double
formulaMargin()
{
  return std::is_same_v<T, float> ? 1.0e-3 : 1.0e-9;
}

// A smooth CSG blend converging to its sharp counterpart far from the blend region: the residual
// decays with distance from the blend but never reaches exactly zero.
template <class T>
double
asymptoticMargin()
{
  return std::is_same_v<T, float> ? 1.0e-2 : 1.0e-6;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// SmoothMin / SmoothMax / ExpMin
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SmoothMin: reduces to the sharp minimum once |a - b| exceeds the smoothing length",
                   "[CSG][SmoothMin]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const T s = T(0.5);

  REQUIRE_THAT(SmoothMin<T>(T(1.0), T(3.0), s), withinAbsT(T(1.0), exactMargin<T>()));
  REQUIRE_THAT(SmoothMin<T>(T(3.0), T(1.0), s), withinAbsT(T(1.0), exactMargin<T>()));
  REQUIRE_THAT(SmoothMin<T>(T(-5.0), T(-1.0), s), withinAbsT(T(-5.0), exactMargin<T>()));
}

TEMPLATE_TEST_CASE("SmoothMin: equal inputs blend to a - 0.25*s", "[CSG][SmoothMin]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const T a = T(2.0);
  const T s = T(0.4);

  REQUIRE_THAT(SmoothMin<T>(a, a, s), withinAbsT(a - T(0.25) * s, exactMargin<T>()));
}

TEMPLATE_TEST_CASE("SmoothMin: never returns a value smaller (more negative) than the sharp minimum",
                   "[CSG][SmoothMin]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  for (const T a : sweepValues<T>(T(-3.0), T(3.0), 16)) {
    for (const T b : sweepValues<T>(T(-3.0), T(3.0), 14)) {
      REQUIRE(SmoothMin<T>(a, b, T(1.0)) <= std::min(a, b) + T(exactMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("SmoothMax: reduces to the sharp maximum once |a - b| exceeds the smoothing length",
                   "[CSG][SmoothMax]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const T s = T(0.5);

  REQUIRE_THAT(SmoothMax<T>(T(1.0), T(3.0), s), withinAbsT(T(3.0), exactMargin<T>()));
  REQUIRE_THAT(SmoothMax<T>(T(-5.0), T(-1.0), s), withinAbsT(T(-1.0), exactMargin<T>()));
}

TEMPLATE_TEST_CASE("SmoothMax: equal inputs blend to a + 0.25*s", "[CSG][SmoothMax]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const T a = T(2.0);
  const T s = T(0.4);

  REQUIRE_THAT(SmoothMax<T>(a, a, s), withinAbsT(a + T(0.25) * s, exactMargin<T>()));
}

TEMPLATE_TEST_CASE("SmoothMax: never returns a value larger than the sharp maximum",
                   "[CSG][SmoothMax]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  for (const T a : sweepValues<T>(T(-3.0), T(3.0), 16)) {
    for (const T b : sweepValues<T>(T(-3.0), T(3.0), 14)) {
      REQUIRE(SmoothMax<T>(a, b, T(1.0)) >= std::max(a, b) - T(exactMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("ExpMin: equal inputs blend to a - s*ln(2)", "[CSG][ExpMin]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const T a = T(2.0);
  const T s = T(0.4);

  REQUIRE_THAT(ExpMin<T>(a, a, s), withinAbsT(a - s * std::log(T(2.0)), formulaMargin<T>()));
}

TEMPLATE_TEST_CASE("ExpMin: never exceeds the sharp minimum", "[CSG][ExpMin]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  for (const T a : sweepValues<T>(T(-3.0), T(3.0), 16)) {
    for (const T b : sweepValues<T>(T(-3.0), T(3.0), 14)) {
      REQUIRE(ExpMin<T>(a, b, T(1.0)) <= std::min(a, b) + T(formulaMargin<T>()));
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// UnionIF / Union()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("UnionIF: value is the pointwise minimum of two disjoint spheres",
                   "[CSG][Union]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();

  const UnionIF<T> u({a, b});

  const Vec3 center = Vec3::zeros();
  const Vec3 far(10, 10, 10);

  REQUIRE_THAT(u.value(center),
               withinAbsT(std::min(a->signedDistance(center), b->signedDistance(center)), exactMargin<T>()));
  REQUIRE_THAT(u.value(far), withinAbsT(std::min(a->signedDistance(far), b->signedDistance(far)), exactMargin<T>()));

  // A point strictly inside A only.
  REQUIRE(u.value(center) < T(0.0));

  // The gap between the two spheres (x = 1.5) is outside both.
  REQUIRE(u.value(Vec3(1.5, 0, 0)) > T(0.0));
}

TEMPLATE_TEST_CASE("Union: two-argument free function matches the vector overload and UnionIF directly",
                   "[CSG][Union]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();

  const auto       twoArg    = Union<T>(a, b);
  const auto       vectorArg = Union<T, Sphere<T>>(std::vector<std::shared_ptr<Sphere<T>>>{a, b});
  const UnionIF<T> direct({a, b});

  for (const Vec3 p : {Vec3::zeros(), Vec3(3, 0, 0), Vec3(1.5, 0, 0), Vec3(-10, 4, 2)}) {
    REQUIRE_THAT(twoArg->value(p), withinAbsT(direct.value(p), exactMargin<T>()));
    REQUIRE_THAT(vectorArg->value(p), withinAbsT(direct.value(p), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// SmoothUnionIF / SmoothUnion()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SmoothUnionIF: converges to the sharp union far from the blend region",
                   "[CSG][SmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();

  const T                smoothLen = T(0.1);
  const SmoothUnionIF<T> su({a, b}, smoothLen);
  const UnionIF<T>       sharp({a, b});

  // Deep inside A, far from B: |signedDistance(A) - signedDistance(B)| >> smoothLen.
  const Vec3 deepInA(0, 0, 0);
  REQUIRE_THAT(su.value(deepInA), withinAbsT(sharp.value(deepInA), asymptoticMargin<T>()));
}

TEMPLATE_TEST_CASE("SmoothUnionIF: at least as deep (never shallower) than the sharp union everywhere",
                   "[CSG][SmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();

  const SmoothUnionIF<T> su({a, b}, T(0.75));
  const UnionIF<T>       sharp({a, b});

  for (const T x : sweepValues<T>(T(-2.0), T(5.0), 28)) {
    const Vec3 p(x, 0, 0);
    REQUIRE(su.value(p) <= sharp.value(p) + T(exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("SmoothUnion: free function matches SmoothUnionIF for both the pairwise and vector overloads",
                   "[CSG][SmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();

  const T    smoothLen = T(0.5);
  const auto pairwise  = SmoothUnion<T>(a, b, smoothLen);
  const auto vectorArg = SmoothUnion<T, Sphere<T>>(std::vector<std::shared_ptr<Sphere<T>>>{a, b}, smoothLen);
  const SmoothUnionIF<T> direct({a, b}, smoothLen);

  for (const Vec3 p : {Vec3::zeros(), Vec3(1.5, 0, 0), Vec3(3, 0, 0)}) {
    REQUIRE_THAT(pairwise->value(p), withinAbsT(direct.value(p), exactMargin<T>()));
    REQUIRE_THAT(vectorArg->value(p), withinAbsT(direct.value(p), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// BVHUnionIF / BVHUnion() and BVHSmoothUnionIF / BVHSmoothUnion()
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// A small scene of spheres on a line, spaced so neighbors are disjoint -- enough primitives to
// give the BVH real partitioning depth (unlike the 2-sphere fixtures above).
constexpr int NumRowSpheres = 12;

template <class T>
std::vector<std::shared_ptr<Sphere<T>>>
sphereRow()
{
  std::vector<std::shared_ptr<Sphere<T>>> spheres;
  spheres.reserve(NumRowSpheres);
  for (int i = 0; i < NumRowSpheres; i++) {
    spheres.push_back(std::make_shared<Sphere<T>>(Vec3T<T>(3.0 * i, 0, 0), T(1)));
  }
  return spheres;
}

template <class T>
std::vector<BV<T>>
sphereRowBVs()
{
  std::vector<BV<T>> bvs;
  bvs.reserve(NumRowSpheres);
  for (int i = 0; i < NumRowSpheres; i++) {
    bvs.push_back(sphereBV<T>(Vec3T<T>(3.0 * i, 0, 0), T(1)));
  }
  return bvs;
}

template <class T>
std::vector<Vec3T<T>>
lineQueryPoints()
{
  std::vector<Vec3T<T>> pts;
  for (const T x : sweepValues<T>(T(-2.0), T(35.0), 28)) {
    pts.emplace_back(x, 0, 0);
  }
  return pts;
}

} // namespace

TEMPLATE_TEST_CASE("BVHUnionIF: agrees with the sharp UnionIF over a row of spheres",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  const auto spheres = sphereRow<T>();

  const std::vector<BV<T>> bvs = sphereRowBVs<T>();

  const BVHUnionIF<T, Sphere<T>, BV<T>, K> bvhUnion(spheres, bvs);

  std::vector<std::shared_ptr<IF<T>>> asIF(spheres.begin(), spheres.end());
  const UnionIF<T>                    sharpUnion(asIF);

  for (const auto& p : lineQueryPoints<T>()) {
    REQUIRE_THAT(bvhUnion.value(p), withinAbsT(sharpUnion.value(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHUnion: free function matches BVHUnionIF and the (primsAndBVs) constructor overload",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  const auto spheres = sphereRow<T>();
  const auto bvs     = sphereRowBVs<T>();

  std::vector<std::pair<std::shared_ptr<const Sphere<T>>, BV<T>>> primsAndBVs;
  primsAndBVs.reserve(bvs.size());
  for (size_t i = 0; i < bvs.size(); i++) {
    primsAndBVs.emplace_back(spheres[i], bvs[i]);
  }

  const auto                               freeFunc = BVHUnion<T, Sphere<T>, BV<T>, K>(spheres, bvs);
  const BVHUnionIF<T, Sphere<T>, BV<T>, K> fromPairs(primsAndBVs);
  const BVHUnionIF<T, Sphere<T>, BV<T>, K> fromLists(spheres, bvs);

  for (const auto& p : lineQueryPoints<T>()) {
    REQUIRE_THAT(freeFunc->value(p), withinAbsT(fromPairs.value(p), exactMargin<T>()));
    REQUIRE_THAT(fromLists.value(p), withinAbsT(fromPairs.value(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHSmoothUnionIF: agrees with SmoothUnionIF far from any blend region",
                   "[CSG][BVHSmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K         = 4;
  const T          smoothLen = T(0.05); // Small relative to the 1-unit gap between spheres.

  const auto spheres = sphereRow<T>();

  const std::vector<BV<T>> bvs = sphereRowBVs<T>();

  const BVHSmoothUnionIF<T, Sphere<T>, BV<T>, K> bvhSmooth(spheres, bvs, smoothLen);

  std::vector<std::shared_ptr<IF<T>>> asIF(spheres.begin(), spheres.end());
  const SmoothUnionIF<T>              sharpSmooth(asIF, smoothLen);

  // Deep inside any one sphere, far from every other sphere's surface.
  for (int i = 0; i < 12; i++) {
    const Vec3 deepInside(3.0 * i, 0, 0);
    REQUIRE_THAT(bvhSmooth.value(deepInside), withinAbsT(sharpSmooth.value(deepInside), asymptoticMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHSmoothUnionIF: matches a brute-force two-nearest smooth-min inside the blend region",
                   "[CSG][BVHSmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K         = 4;
  const T          smoothLen = T(0.6); // Large enough to actively blend across the 1-unit surface gaps.

  const auto spheres = sphereRow<T>();

  const std::vector<BV<T>> bvs = sphereRowBVs<T>();

  const BVHSmoothUnionIF<T, Sphere<T>, BV<T>, K> bvhSmooth(spheres, bvs, smoothLen);

  // Brute-force reference: the smooth-minimum of the two smallest sphere values, found by a full
  // linear scan instead of the pruned BVH traversal. This is exactly what BVHSmoothUnionIF computes,
  // so it validates that the (squared, b-based) pruning bound retains *both* blend inputs even in the
  // overlap region where the two nearest surfaces interpenetrate -- the migration's key invariant.
  const auto bruteTwoNearest = [&spheres, &smoothLen](const Vec3& a_point) -> T {
    T a = std::numeric_limits<T>::infinity();
    T b = std::numeric_limits<T>::infinity();

    for (const auto& sphere : spheres) {
      const T d = sphere->value(a_point);

      if (d < a) {
        b = a;
        a = d;
      }
      else if (d < b) {
        b = d;
      }
    }

    return SmoothMin<T>(a, b, smoothLen);
  };

  for (const auto& p : lineQueryPoints<T>()) {
    REQUIRE_THAT(bvhSmooth.value(p), withinAbsT(bruteTwoNearest(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHSmoothUnion: free function matches BVHSmoothUnionIF",
                   "[CSG][BVHSmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K         = 4;
  const T          smoothLen = T(0.2);

  const auto spheres = sphereRow<T>();

  const std::vector<BV<T>> bvs = sphereRowBVs<T>();

  const auto freeFunc = BVHSmoothUnion<T, Sphere<T>, BV<T>, K>(spheres, bvs, smoothLen);
  const BVHSmoothUnionIF<T, Sphere<T>, BV<T>, K> direct(spheres, bvs, smoothLen);

  for (const auto& p : lineQueryPoints<T>()) {
    REQUIRE_THAT(freeFunc->value(p), withinAbsT(direct.value(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHUnionIF::getBoundingVolume encloses every input sphere",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  const auto spheres = sphereRow<T>();

  const std::vector<BV<T>> bvs = sphereRowBVs<T>();

  const BVHUnionIF<T, Sphere<T>, BV<T>, K> bvhUnion(spheres, bvs);
  const auto&                              rootBV = bvhUnion.getBoundingVolume();

  for (const auto& bv : bvs) {
    REQUIRE(rootBV.getLowCorner()[0] <= bv.getLowCorner()[0] + T(exactMargin<T>()));
    REQUIRE(rootBV.getHighCorner()[0] >= bv.getHighCorner()[0] - T(exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// IntersectionIF / Intersection()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("IntersectionIF: value is the pointwise maximum, giving a non-trivial overlap region",
                   "[CSG][Intersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const IntersectionIF<T> inter({c, d});

  // Inside both spheres.
  const Vec3 insideBoth(0.5, 0, 0);
  REQUIRE_THAT(inter.value(insideBoth),
               withinAbsT(std::max(c->signedDistance(insideBoth), d->signedDistance(insideBoth)), exactMargin<T>()));
  REQUIRE(inter.value(insideBoth) < T(0.0));

  // Inside D only (outside C): must not be in the intersection.
  const Vec3 insideDOnly(2.5, 0, 0);
  REQUIRE(c->signedDistance(insideDOnly) > T(0.0));
  REQUIRE(d->signedDistance(insideDOnly) < T(0.0));
  REQUIRE(inter.value(insideDOnly) > T(0.0));
}

TEMPLATE_TEST_CASE("Intersection: two-argument free function matches the vector overload and IntersectionIF",
                   "[CSG][Intersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const auto              twoArg    = Intersection<T>(c, d);
  const auto              vectorArg = Intersection<T, Sphere<T>>(std::vector<std::shared_ptr<Sphere<T>>>{c, d});
  const IntersectionIF<T> direct({c, d});

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(2.5, 0, 0), Vec3(-5, 3, 1)}) {
    REQUIRE_THAT(twoArg->value(p), withinAbsT(direct.value(p), exactMargin<T>()));
    REQUIRE_THAT(vectorArg->value(p), withinAbsT(direct.value(p), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// SmoothIntersectionIF / SmoothIntersection()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SmoothIntersectionIF: converges to the sharp intersection far from the blend region",
                   "[CSG][SmoothIntersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const T                       smoothLen = T(0.05);
  const SmoothIntersectionIF<T> smooth(c, d, smoothLen);
  const IntersectionIF<T>       sharp({c, d});

  // Deep inside D only, far from C's surface (|C - D| there is >> smoothLen).
  const Vec3 deepOutsideC(4.5, 0, 0);
  REQUIRE_THAT(smooth.value(deepOutsideC), withinAbsT(sharp.value(deepOutsideC), asymptoticMargin<T>()));
}

TEMPLATE_TEST_CASE("SmoothIntersectionIF: never shallower (never below) the sharp intersection",
                   "[CSG][SmoothIntersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const SmoothIntersectionIF<T> smooth(c, d, T(0.75));
  const IntersectionIF<T>       sharp({c, d});

  for (const T x : sweepValues<T>(T(-3.0), T(4.0), 28)) {
    const Vec3 p(x, 0, 0);
    REQUIRE(smooth.value(p) >= sharp.value(p) - T(exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("SmoothIntersectionIF: vector-of-functions constructor matches the pairwise constructor",
                   "[CSG][SmoothIntersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const T smoothLen = T(0.4);

  const SmoothIntersectionIF<T> pairwise(c, d, smoothLen);
  const SmoothIntersectionIF<T> vectorCtor(std::vector<std::shared_ptr<IF<T>>>{c, d}, smoothLen);

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(2.5, 0, 0), Vec3(-5, 3, 1)}) {
    REQUIRE_THAT(vectorCtor.value(p), withinAbsT(pairwise.value(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("SmoothIntersection: free function matches SmoothIntersectionIF",
                   "[CSG][SmoothIntersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const T                       smoothLen = T(0.4);
  const auto                    pairwise  = SmoothIntersection<T>(c, d, smoothLen);
  const SmoothIntersectionIF<T> direct(c, d, smoothLen);

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(2.5, 0, 0)}) {
    REQUIRE_THAT(pairwise->value(p), withinAbsT(direct.value(p), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// DifferenceIF / Difference()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("DifferenceIF: A \\ B is max(A, -B), inside A and outside B only",
                   "[CSG][Difference]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const DifferenceIF<T> diff(c, d);

  // Inside C, but also inside D -- must be excluded from C \ D.
  const Vec3 insideBoth(0.5, 0, 0);
  REQUIRE(diff.value(insideBoth) > T(0.0));
  REQUIRE_THAT(diff.value(insideBoth),
               withinAbsT(std::max(c->signedDistance(insideBoth), -d->signedDistance(insideBoth)), exactMargin<T>()));

  // Inside C, outside D.
  const Vec3 insideCOnly(-1.5, 0, 0);
  REQUIRE(c->signedDistance(insideCOnly) < T(0.0));
  REQUIRE(d->signedDistance(insideCOnly) > T(0.0));
  REQUIRE(diff.value(insideCOnly) < T(0.0));
}

TEMPLATE_TEST_CASE("DifferenceIF: A minus a list of subtrahends excludes the union of all of them",
                   "[CSG][Difference]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>(); // Center (0,0,0), radius 1: fully contains the origin.
  const auto b = sphereB<T>(); // Center (3,0,0), radius 1: doesn't overlap A at all.
  const auto e = std::make_shared<Sphere<T>>(Vec3(0.5, 0, 0), T(0.3)); // A small sphere carved out of A.

  const DifferenceIF<T> diff(a, std::vector<std::shared_ptr<IF<T>>>{b, e});

  REQUIRE(diff.value(Vec3::zeros()) < T(0.0));   // Inside A, outside both B and E.
  REQUIRE(diff.value(Vec3(0.5, 0, 0)) > T(0.0)); // Inside A and inside E: excluded.
  REQUIRE(diff.value(Vec3(3, 0, 0)) > T(0.0));   // Not inside A at all.
}

TEMPLATE_TEST_CASE("Difference: free function matches DifferenceIF", "[CSG][Difference]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const auto            freeFunc = Difference<T>(c, d);
  const DifferenceIF<T> direct(c, d);

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(-1.5, 0, 0), Vec3(3, 0, 0)}) {
    REQUIRE_THAT(freeFunc->value(p), withinAbsT(direct.value(p), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// SmoothDifferenceIF / SmoothDifference()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SmoothDifferenceIF: converges to the sharp difference far from the blend region",
                   "[CSG][SmoothDifference]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A and B are disjoint (see sphereA()/sphereB()), so A(x) and -B(x) are never close to each
  // other -- unlike C/D, which overlap and would put the "inside C, outside D" test point close
  // to the smooth-difference crossover instead of far from it.
  const auto a = sphereA<T>();
  const auto b = sphereB<T>();

  const T                     smoothLen = T(0.05);
  const SmoothDifferenceIF<T> smooth(a, b, smoothLen);
  const DifferenceIF<T>       sharp(a, b);

  const Vec3 insideAOnly = Vec3::zeros();
  REQUIRE_THAT(smooth.value(insideAOnly), withinAbsT(sharp.value(insideAOnly), asymptoticMargin<T>()));
}

TEMPLATE_TEST_CASE("SmoothDifferenceIF: list-of-subtrahends constructor matches the pairwise constructor "
                   "when there is only one subtrahend",
                   "[CSG][SmoothDifference]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const T smoothLen = T(0.4);

  const SmoothDifferenceIF<T> pairwise(c, d, smoothLen);
  const SmoothDifferenceIF<T> vectorCtor(c, std::vector<std::shared_ptr<IF<T>>>{d}, smoothLen);

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(-1.5, 0, 0), Vec3(3, 0, 0)}) {
    REQUIRE_THAT(vectorCtor.value(p), withinAbsT(pairwise.value(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("SmoothDifference: free function matches SmoothDifferenceIF",
                   "[CSG][SmoothDifference]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const T                     smoothLen = T(0.3);
  const auto                  freeFunc  = SmoothDifference<T>(c, d, smoothLen);
  const SmoothDifferenceIF<T> direct(c, d, smoothLen);

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(-1.5, 0, 0)}) {
    REQUIRE_THAT(freeFunc->value(p), withinAbsT(direct.value(p), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// FiniteRepetitionIF / FiniteRepetition()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("FiniteRepetitionIF: matches the base function directly within the home tile",
                   "[CSG][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto base = std::make_shared<Sphere<T>>(Vec3::zeros(), T(1));

  const Vec3 period(10, 10, 10);
  const Vec3 repeatLo(2, 2, 2);
  const Vec3 repeatHi(2, 2, 2);

  const FiniteRepetitionIF<T> tiled(base, period, repeatLo, repeatHi);

  for (const Vec3 p : {Vec3::zeros(), Vec3(0.5, 0, 0), Vec3(4.9, 0, 0), Vec3(-4.9, 0, 0)}) {
    REQUIRE_THAT(tiled.value(p), withinAbsT(base->signedDistance(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("FiniteRepetitionIF: is periodic within the repetition range",
                   "[CSG][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto base = std::make_shared<Sphere<T>>(Vec3::zeros(), T(1));

  const Vec3 period(5, 5, 5);
  const Vec3 repeatLo(3, 0, 0);
  const Vec3 repeatHi(3, 0, 0);

  const FiniteRepetitionIF<T> tiled(base, period, repeatLo, repeatHi);

  // A point near the origin and the same point shifted by exactly N whole periods (N within the
  // repetition range) must evaluate to the same value.
  for (int n = -3; n <= 3; n++) {
    const Vec3 p(0.3, 0, 0);
    const Vec3 shifted = p + Vec3(5.0 * n, 0, 0);
    REQUIRE_THAT(tiled.value(shifted), withinAbsT(tiled.value(p), formulaMargin<T>()));
    REQUIRE_THAT(tiled.value(shifted), withinAbsT(base->signedDistance(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("FiniteRepetitionIF: clamps to the boundary tile beyond the repetition range",
                   "[CSG][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto base = std::make_shared<Sphere<T>>(Vec3::zeros(), T(1));

  const Vec3 period(5, 5, 5);
  const Vec3 repeatLo(1, 0, 0);
  const Vec3 repeatHi(1, 0, 0);

  const FiniteRepetitionIF<T> tiled(base, period, repeatLo, repeatHi);

  // Well beyond the +1 tile: folds relative to the tile-1 anchor (x - 1*period), not back toward
  // tile 0 and not extrapolated as if more tiles existed.
  const Vec3 farBeyond(23.0, 0, 0);
  const Vec3 expectedLocal = farBeyond - Vec3(5.0, 0, 0);
  REQUIRE_THAT(tiled.value(farBeyond), withinAbsT(base->signedDistance(expectedLocal), formulaMargin<T>()));

  // Symmetric check on the negative side.
  const Vec3 farBelow(-23.0, 0, 0);
  const Vec3 expectedLocalNeg = farBelow + Vec3(5.0, 0, 0);
  REQUIRE_THAT(tiled.value(farBelow), withinAbsT(base->signedDistance(expectedLocalNeg), formulaMargin<T>()));
}

TEMPLATE_TEST_CASE("FiniteRepetition: free function matches FiniteRepetitionIF",
                   "[CSG][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto base = std::make_shared<Sphere<T>>(Vec3::zeros(), T(1));

  const Vec3 period(5, 5, 5);
  const Vec3 repeatLo(2, 2, 2);
  const Vec3 repeatHi(2, 2, 2);

  const auto                  freeFunc = FiniteRepetition<T>(base, period, repeatLo, repeatHi);
  const FiniteRepetitionIF<T> direct(base, period, repeatLo, repeatHi);

  for (const Vec3 p : {Vec3::zeros(), Vec3(7.3, -2.1, 0.4), Vec3(-12.0, 0, 0)}) {
    REQUIRE_THAT(freeFunc->value(p), withinAbsT(direct.value(p), formulaMargin<T>()));
  }
}
