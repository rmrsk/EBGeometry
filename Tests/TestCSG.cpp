// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_CSG.hpp: the sharp and smoothly-blended CSG combinators (union,
// intersection, difference), their BVH-accelerated counterparts, finite periodic repetition, and
// the SmoothMin/SmoothMax/ExpMin blending primitives they're built on. Uses analytic spheres as
// fixtures so every expected value is hand-computable.

#include "EBGeometry.hpp"

#include <cmath>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinAbs;

namespace {

using T      = double;
using Vec3   = Vec3T<T>;
using Sphere = SphereSDF<T>;
using IF     = ImplicitFunction<T>;
using BV     = BoundingVolumes::AABBT<T>;

BV
sphereBV(const Vec3& a_center, const T a_radius)
{
  return {a_center - a_radius * Vec3::ones(), a_center + a_radius * Vec3::ones()};
}

// Two well-separated (non-overlapping) unit-ish spheres: centers 3 apart, radius 1 each, so
// there's a gap of 1 between them. Every sharp-CSG expected value below is hand-computable from
// these two spheres' own signedDistance().
std::shared_ptr<Sphere>
sphereA()
{
  return std::make_shared<Sphere>(Vec3::zeros(), T(1));
}

std::shared_ptr<Sphere>
sphereB()
{
  return std::make_shared<Sphere>(Vec3(3, 0, 0), T(1));
}

// Two substantially overlapping spheres, used where a non-trivial (both-negative) intersection
// region is needed.
std::shared_ptr<Sphere>
sphereC()
{
  return std::make_shared<Sphere>(Vec3::zeros(), T(2));
}

std::shared_ptr<Sphere>
sphereD()
{
  return std::make_shared<Sphere>(Vec3(1, 0, 0), T(2));
}

// Evenly-spaced sweep of `a_count` values in [a_lo, a_hi], inclusive of both endpoints. Used
// instead of a floating-point loop counter (flagged by clang-tidy's FloatLoopCounter check,
// since float accumulation drift can make the number of iterations non-obvious).
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

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// SmoothMin / SmoothMax / ExpMin
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SmoothMin: reduces to the sharp minimum once |a - b| exceeds the smoothing length", "[CSG][SmoothMin]")
{
  const T s = 0.5;

  REQUIRE_THAT(SmoothMin<T>(1.0, 3.0, s), WithinAbs(1.0, 1.0e-12));
  REQUIRE_THAT(SmoothMin<T>(3.0, 1.0, s), WithinAbs(1.0, 1.0e-12));
  REQUIRE_THAT(SmoothMin<T>(-5.0, -1.0, s), WithinAbs(-5.0, 1.0e-12));
}

TEST_CASE("SmoothMin: equal inputs blend to a - 0.25*s", "[CSG][SmoothMin]")
{
  const T a = 2.0;
  const T s = 0.4;

  REQUIRE_THAT(SmoothMin<T>(a, a, s), WithinAbs(a - T(0.25) * s, 1.0e-12));
}

TEST_CASE("SmoothMin: never returns a value smaller (more negative) than the sharp minimum", "[CSG][SmoothMin]")
{
  for (const T a : sweepValues(-3.0, 3.0, 16)) {
    for (const T b : sweepValues(-3.0, 3.0, 14)) {
      REQUIRE(SmoothMin<T>(a, b, 1.0) <= std::min(a, b) + 1.0e-12);
    }
  }
}

TEST_CASE("SmoothMax: reduces to the sharp maximum once |a - b| exceeds the smoothing length", "[CSG][SmoothMax]")
{
  const T s = 0.5;

  REQUIRE_THAT(SmoothMax<T>(1.0, 3.0, s), WithinAbs(3.0, 1.0e-12));
  REQUIRE_THAT(SmoothMax<T>(-5.0, -1.0, s), WithinAbs(-1.0, 1.0e-12));
}

TEST_CASE("SmoothMax: equal inputs blend to a + 0.25*s", "[CSG][SmoothMax]")
{
  const T a = 2.0;
  const T s = 0.4;

  REQUIRE_THAT(SmoothMax<T>(a, a, s), WithinAbs(a + T(0.25) * s, 1.0e-12));
}

TEST_CASE("SmoothMax: never returns a value larger than the sharp maximum", "[CSG][SmoothMax]")
{
  for (const T a : sweepValues(-3.0, 3.0, 16)) {
    for (const T b : sweepValues(-3.0, 3.0, 14)) {
      REQUIRE(SmoothMax<T>(a, b, 1.0) >= std::max(a, b) - 1.0e-12);
    }
  }
}

TEST_CASE("ExpMin: equal inputs blend to a - s*ln(2)", "[CSG][ExpMin]")
{
  const T a = 2.0;
  const T s = 0.4;

  REQUIRE_THAT(ExpMin<T>(a, a, s), WithinAbs(a - s * std::log(2.0), 1.0e-9));
}

TEST_CASE("ExpMin: never exceeds the sharp minimum", "[CSG][ExpMin]")
{
  for (const T a : sweepValues(-3.0, 3.0, 16)) {
    for (const T b : sweepValues(-3.0, 3.0, 14)) {
      REQUIRE(ExpMin<T>(a, b, 1.0) <= std::min(a, b) + 1.0e-9);
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// UnionIF / Union()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("UnionIF: value is the pointwise minimum of two disjoint spheres", "[CSG][Union]")
{
  const auto a = sphereA();
  const auto b = sphereB();

  const UnionIF<T> u({a, b});

  const Vec3 center = Vec3::zeros();
  const Vec3 far(10, 10, 10);

  REQUIRE_THAT(u.value(center), WithinAbs(std::min(a->signedDistance(center), b->signedDistance(center)), 1.0e-12));
  REQUIRE_THAT(u.value(far), WithinAbs(std::min(a->signedDistance(far), b->signedDistance(far)), 1.0e-12));

  // A point strictly inside A only.
  REQUIRE(u.value(center) < 0.0);

  // The gap between the two spheres (x = 1.5) is outside both.
  REQUIRE(u.value(Vec3(1.5, 0, 0)) > 0.0);
}

TEST_CASE("Union: two-argument free function matches the vector overload and UnionIF directly", "[CSG][Union]")
{
  const auto a = sphereA();
  const auto b = sphereB();

  const auto       twoArg    = Union<T>(a, b);
  const auto       vectorArg = Union<T, Sphere>(std::vector<std::shared_ptr<Sphere>>{a, b});
  const UnionIF<T> direct({a, b});

  for (const Vec3 p : {Vec3::zeros(), Vec3(3, 0, 0), Vec3(1.5, 0, 0), Vec3(-10, 4, 2)}) {
    REQUIRE_THAT(twoArg->value(p), WithinAbs(direct.value(p), 1.0e-12));
    REQUIRE_THAT(vectorArg->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// SmoothUnionIF / SmoothUnion()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SmoothUnionIF: converges to the sharp union far from the blend region", "[CSG][SmoothUnion]")
{
  const auto a = sphereA();
  const auto b = sphereB();

  const T                smoothLen = 0.1;
  const SmoothUnionIF<T> su({a, b}, smoothLen);
  const UnionIF<T>       sharp({a, b});

  // Deep inside A, far from B: |signedDistance(A) - signedDistance(B)| >> smoothLen.
  const Vec3 deepInA(0, 0, 0);
  REQUIRE_THAT(su.value(deepInA), WithinAbs(sharp.value(deepInA), 1.0e-9));
}

TEST_CASE("SmoothUnionIF: at least as deep (never shallower) than the sharp union everywhere", "[CSG][SmoothUnion]")
{
  const auto a = sphereA();
  const auto b = sphereB();

  const SmoothUnionIF<T> su({a, b}, 0.75);
  const UnionIF<T>       sharp({a, b});

  for (const T x : sweepValues(-2.0, 5.0, 28)) {
    const Vec3 p(x, 0, 0);
    REQUIRE(su.value(p) <= sharp.value(p) + 1.0e-12);
  }
}

TEST_CASE("SmoothUnion: free function matches SmoothUnionIF for both the pairwise and vector overloads",
          "[CSG][SmoothUnion]")
{
  const auto a = sphereA();
  const auto b = sphereB();

  const T                smoothLen = 0.5;
  const auto             pairwise  = SmoothUnion<T>(a, b, smoothLen);
  const auto             vectorArg = SmoothUnion<T, Sphere>(std::vector<std::shared_ptr<Sphere>>{a, b}, smoothLen);
  const SmoothUnionIF<T> direct({a, b}, smoothLen);

  for (const Vec3 p : {Vec3::zeros(), Vec3(1.5, 0, 0), Vec3(3, 0, 0)}) {
    REQUIRE_THAT(pairwise->value(p), WithinAbs(direct.value(p), 1.0e-12));
    REQUIRE_THAT(vectorArg->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// BVHUnionIF / BVHUnion() and BVHSmoothUnionIF / BVHSmoothUnion()
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// A small scene of spheres on a line, spaced so neighbors are disjoint -- enough primitives to
// give the BVH real partitioning depth (unlike the 2-sphere fixtures above).
constexpr int NumRowSpheres = 12;

std::vector<std::shared_ptr<Sphere>>
sphereRow()
{
  std::vector<std::shared_ptr<Sphere>> spheres;
  spheres.reserve(NumRowSpheres);
  for (int i = 0; i < NumRowSpheres; i++) {
    spheres.push_back(std::make_shared<Sphere>(Vec3(3.0 * i, 0, 0), T(1)));
  }
  return spheres;
}

std::vector<BV>
sphereRowBVs()
{
  std::vector<BV> bvs;
  bvs.reserve(NumRowSpheres);
  for (int i = 0; i < NumRowSpheres; i++) {
    bvs.push_back(sphereBV(Vec3(3.0 * i, 0, 0), T(1)));
  }
  return bvs;
}

std::vector<Vec3>
lineQueryPoints()
{
  std::vector<Vec3> pts;
  for (const T x : sweepValues(-2.0, 35.0, 28)) {
    pts.emplace_back(x, 0, 0);
  }
  return pts;
}

} // namespace

TEST_CASE("BVHUnionIF: agrees with the sharp UnionIF over a row of spheres", "[CSG][BVHUnion]")
{
  constexpr size_t K = 4;

  const auto spheres = sphereRow();

  const std::vector<BV> bvs = sphereRowBVs();

  const BVHUnionIF<T, Sphere, BV, K> bvhUnion(spheres, bvs);

  std::vector<std::shared_ptr<IF>> asIF(spheres.begin(), spheres.end());
  const UnionIF<T>                 sharpUnion(asIF);

  for (const auto& p : lineQueryPoints()) {
    REQUIRE_THAT(bvhUnion.value(p), WithinAbs(sharpUnion.value(p), 1.0e-9));
  }
}

TEST_CASE("BVHUnion: free function matches BVHUnionIF and the (primsAndBVs) constructor overload", "[CSG][BVHUnion]")
{
  constexpr size_t K = 4;

  const auto spheres = sphereRow();
  const auto bvs     = sphereRowBVs();

  std::vector<std::pair<std::shared_ptr<const Sphere>, BV>> primsAndBVs;
  primsAndBVs.reserve(bvs.size());
  for (size_t i = 0; i < bvs.size(); i++) {
    primsAndBVs.emplace_back(spheres[i], bvs[i]);
  }

  const auto                         freeFunc = BVHUnion<T, Sphere, BV, K>(spheres, bvs);
  const BVHUnionIF<T, Sphere, BV, K> fromPairs(primsAndBVs);
  const BVHUnionIF<T, Sphere, BV, K> fromLists(spheres, bvs);

  for (const auto& p : lineQueryPoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(fromPairs.value(p), 1.0e-12));
    REQUIRE_THAT(fromLists.value(p), WithinAbs(fromPairs.value(p), 1.0e-12));
  }
}

TEST_CASE("BVHSmoothUnionIF: agrees with SmoothUnionIF far from any blend region", "[CSG][BVHSmoothUnion]")
{
  constexpr size_t K         = 4;
  const T          smoothLen = 0.05; // Small relative to the 1-unit gap between spheres.

  const auto spheres = sphereRow();

  const std::vector<BV> bvs = sphereRowBVs();

  const BVHSmoothUnionIF<T, Sphere, BV, K> bvhSmooth(spheres, bvs, smoothLen);

  std::vector<std::shared_ptr<IF>> asIF(spheres.begin(), spheres.end());
  const SmoothUnionIF<T>           sharpSmooth(asIF, smoothLen);

  // Deep inside any one sphere, far from every other sphere's surface.
  for (int i = 0; i < 12; i++) {
    const Vec3 deepInside(3.0 * i, 0, 0);
    REQUIRE_THAT(bvhSmooth.value(deepInside), WithinAbs(sharpSmooth.value(deepInside), 1.0e-6));
  }
}

TEST_CASE("BVHSmoothUnion: free function matches BVHSmoothUnionIF", "[CSG][BVHSmoothUnion]")
{
  constexpr size_t K         = 4;
  const T          smoothLen = 0.2;

  const auto spheres = sphereRow();

  const std::vector<BV> bvs = sphereRowBVs();

  const auto                               freeFunc = BVHSmoothUnion<T, Sphere, BV, K>(spheres, bvs, smoothLen);
  const BVHSmoothUnionIF<T, Sphere, BV, K> direct(spheres, bvs, smoothLen);

  for (const auto& p : lineQueryPoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

TEST_CASE("BVHUnionIF::getBoundingVolume encloses every input sphere", "[CSG][BVHUnion]")
{
  constexpr size_t K = 4;

  const auto spheres = sphereRow();

  const std::vector<BV> bvs = sphereRowBVs();

  const BVHUnionIF<T, Sphere, BV, K> bvhUnion(spheres, bvs);
  const auto&                        rootBV = bvhUnion.getBoundingVolume();

  for (const auto& bv : bvs) {
    REQUIRE(rootBV.getLowCorner()[0] <= bv.getLowCorner()[0] + 1.0e-12);
    REQUIRE(rootBV.getHighCorner()[0] >= bv.getHighCorner()[0] - 1.0e-12);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// IntersectionIF / Intersection()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("IntersectionIF: value is the pointwise maximum, giving a non-trivial overlap region", "[CSG][Intersection]")
{
  const auto c = sphereC();
  const auto d = sphereD();

  const IntersectionIF<T> inter({c, d});

  // Inside both spheres.
  const Vec3 insideBoth(0.5, 0, 0);
  REQUIRE_THAT(inter.value(insideBoth),
               WithinAbs(std::max(c->signedDistance(insideBoth), d->signedDistance(insideBoth)), 1.0e-12));
  REQUIRE(inter.value(insideBoth) < 0.0);

  // Inside D only (outside C): must not be in the intersection.
  const Vec3 insideDOnly(2.5, 0, 0);
  REQUIRE(c->signedDistance(insideDOnly) > 0.0);
  REQUIRE(d->signedDistance(insideDOnly) < 0.0);
  REQUIRE(inter.value(insideDOnly) > 0.0);
}

TEST_CASE("Intersection: two-argument free function matches the vector overload and IntersectionIF",
          "[CSG][Intersection]")
{
  const auto c = sphereC();
  const auto d = sphereD();

  const auto              twoArg    = Intersection<T>(c, d);
  const auto              vectorArg = Intersection<T, Sphere>(std::vector<std::shared_ptr<Sphere>>{c, d});
  const IntersectionIF<T> direct({c, d});

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(2.5, 0, 0), Vec3(-5, 3, 1)}) {
    REQUIRE_THAT(twoArg->value(p), WithinAbs(direct.value(p), 1.0e-12));
    REQUIRE_THAT(vectorArg->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// SmoothIntersectionIF / SmoothIntersection()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SmoothIntersectionIF: converges to the sharp intersection far from the blend region",
          "[CSG][SmoothIntersection]")
{
  const auto c = sphereC();
  const auto d = sphereD();

  const T                       smoothLen = 0.05;
  const SmoothIntersectionIF<T> smooth(c, d, smoothLen);
  const IntersectionIF<T>       sharp({c, d});

  // Deep inside D only, far from C's surface (|C - D| there is >> smoothLen).
  const Vec3 deepOutsideC(4.5, 0, 0);
  REQUIRE_THAT(smooth.value(deepOutsideC), WithinAbs(sharp.value(deepOutsideC), 1.0e-6));
}

TEST_CASE("SmoothIntersectionIF: never shallower (never below) the sharp intersection", "[CSG][SmoothIntersection]")
{
  const auto c = sphereC();
  const auto d = sphereD();

  const SmoothIntersectionIF<T> smooth(c, d, 0.75);
  const IntersectionIF<T>       sharp({c, d});

  for (const T x : sweepValues(-3.0, 4.0, 28)) {
    const Vec3 p(x, 0, 0);
    REQUIRE(smooth.value(p) >= sharp.value(p) - 1.0e-12);
  }
}

TEST_CASE("SmoothIntersectionIF: vector-of-functions constructor matches the pairwise constructor",
          "[CSG][SmoothIntersection]")
{
  const auto c = sphereC();
  const auto d = sphereD();

  const T smoothLen = 0.4;

  const SmoothIntersectionIF<T> pairwise(c, d, smoothLen);
  const SmoothIntersectionIF<T> vectorCtor(std::vector<std::shared_ptr<IF>>{c, d}, smoothLen);

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(2.5, 0, 0), Vec3(-5, 3, 1)}) {
    REQUIRE_THAT(vectorCtor.value(p), WithinAbs(pairwise.value(p), 1.0e-12));
  }
}

TEST_CASE("SmoothIntersection: free function matches SmoothIntersectionIF", "[CSG][SmoothIntersection]")
{
  const auto c = sphereC();
  const auto d = sphereD();

  const T                       smoothLen = 0.4;
  const auto                    pairwise  = SmoothIntersection<T>(c, d, smoothLen);
  const SmoothIntersectionIF<T> direct(c, d, smoothLen);

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(2.5, 0, 0)}) {
    REQUIRE_THAT(pairwise->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// DifferenceIF / Difference()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("DifferenceIF: A \\ B is max(A, -B), inside A and outside B only", "[CSG][Difference]")
{
  const auto c = sphereC();
  const auto d = sphereD();

  const DifferenceIF<T> diff(c, d);

  // Inside C, but also inside D -- must be excluded from C \ D.
  const Vec3 insideBoth(0.5, 0, 0);
  REQUIRE(diff.value(insideBoth) > 0.0);
  REQUIRE_THAT(diff.value(insideBoth),
               WithinAbs(std::max(c->signedDistance(insideBoth), -d->signedDistance(insideBoth)), 1.0e-12));

  // Inside C, outside D.
  const Vec3 insideCOnly(-1.5, 0, 0);
  REQUIRE(c->signedDistance(insideCOnly) < 0.0);
  REQUIRE(d->signedDistance(insideCOnly) > 0.0);
  REQUIRE(diff.value(insideCOnly) < 0.0);
}

TEST_CASE("DifferenceIF: A minus a list of subtrahends excludes the union of all of them", "[CSG][Difference]")
{
  const auto a = sphereA(); // Center (0,0,0), radius 1: fully contains the origin.
  const auto b = sphereB(); // Center (3,0,0), radius 1: doesn't overlap A at all.
  const auto e = std::make_shared<Sphere>(Vec3(0.5, 0, 0), T(0.3)); // A small sphere carved out of A.

  const DifferenceIF<T> diff(a, std::vector<std::shared_ptr<IF>>{b, e});

  REQUIRE(diff.value(Vec3::zeros()) < 0.0);   // Inside A, outside both B and E.
  REQUIRE(diff.value(Vec3(0.5, 0, 0)) > 0.0); // Inside A and inside E: excluded.
  REQUIRE(diff.value(Vec3(3, 0, 0)) > 0.0);   // Not inside A at all.
}

TEST_CASE("Difference: free function matches DifferenceIF", "[CSG][Difference]")
{
  const auto c = sphereC();
  const auto d = sphereD();

  const auto            freeFunc = Difference<T>(c, d);
  const DifferenceIF<T> direct(c, d);

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(-1.5, 0, 0), Vec3(3, 0, 0)}) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// SmoothDifferenceIF / SmoothDifference()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SmoothDifferenceIF: converges to the sharp difference far from the blend region", "[CSG][SmoothDifference]")
{
  // A and B are disjoint (see sphereA()/sphereB()), so A(x) and -B(x) are never close to each
  // other -- unlike C/D, which overlap and would put the "inside C, outside D" test point close
  // to the smooth-difference crossover instead of far from it.
  const auto a = sphereA();
  const auto b = sphereB();

  const T                     smoothLen = 0.05;
  const SmoothDifferenceIF<T> smooth(a, b, smoothLen);
  const DifferenceIF<T>       sharp(a, b);

  const Vec3 insideAOnly = Vec3::zeros();
  REQUIRE_THAT(smooth.value(insideAOnly), WithinAbs(sharp.value(insideAOnly), 1.0e-6));
}

TEST_CASE("SmoothDifferenceIF: list-of-subtrahends constructor matches the pairwise constructor "
          "when there is only one subtrahend",
          "[CSG][SmoothDifference]")
{
  const auto c = sphereC();
  const auto d = sphereD();

  const T smoothLen = 0.4;

  const SmoothDifferenceIF<T> pairwise(c, d, smoothLen);
  const SmoothDifferenceIF<T> vectorCtor(c, std::vector<std::shared_ptr<IF>>{d}, smoothLen);

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(-1.5, 0, 0), Vec3(3, 0, 0)}) {
    REQUIRE_THAT(vectorCtor.value(p), WithinAbs(pairwise.value(p), 1.0e-12));
  }
}

TEST_CASE("SmoothDifference: free function matches SmoothDifferenceIF", "[CSG][SmoothDifference]")
{
  const auto c = sphereC();
  const auto d = sphereD();

  const T                     smoothLen = 0.3;
  const auto                  freeFunc  = SmoothDifference<T>(c, d, smoothLen);
  const SmoothDifferenceIF<T> direct(c, d, smoothLen);

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(-1.5, 0, 0)}) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// FiniteRepetitionIF / FiniteRepetition()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("FiniteRepetitionIF: matches the base function directly within the home tile", "[CSG][FiniteRepetition]")
{
  const auto base = std::make_shared<Sphere>(Vec3::zeros(), T(1));

  const Vec3 period(10, 10, 10);
  const Vec3 repeatLo(2, 2, 2);
  const Vec3 repeatHi(2, 2, 2);

  const FiniteRepetitionIF<T> tiled(base, period, repeatLo, repeatHi);

  for (const Vec3 p : {Vec3::zeros(), Vec3(0.5, 0, 0), Vec3(4.9, 0, 0), Vec3(-4.9, 0, 0)}) {
    REQUIRE_THAT(tiled.value(p), WithinAbs(base->signedDistance(p), 1.0e-9));
  }
}

TEST_CASE("FiniteRepetitionIF: is periodic within the repetition range", "[CSG][FiniteRepetition]")
{
  const auto base = std::make_shared<Sphere>(Vec3::zeros(), T(1));

  const Vec3 period(5, 5, 5);
  const Vec3 repeatLo(3, 0, 0);
  const Vec3 repeatHi(3, 0, 0);

  const FiniteRepetitionIF<T> tiled(base, period, repeatLo, repeatHi);

  // A point near the origin and the same point shifted by exactly N whole periods (N within the
  // repetition range) must evaluate to the same value.
  for (int n = -3; n <= 3; n++) {
    const Vec3 p(0.3, 0, 0);
    const Vec3 shifted = p + Vec3(5.0 * n, 0, 0);
    REQUIRE_THAT(tiled.value(shifted), WithinAbs(tiled.value(p), 1.0e-9));
    REQUIRE_THAT(tiled.value(shifted), WithinAbs(base->signedDistance(p), 1.0e-9));
  }
}

TEST_CASE("FiniteRepetitionIF: clamps to the boundary tile beyond the repetition range", "[CSG][FiniteRepetition]")
{
  const auto base = std::make_shared<Sphere>(Vec3::zeros(), T(1));

  const Vec3 period(5, 5, 5);
  const Vec3 repeatLo(1, 0, 0);
  const Vec3 repeatHi(1, 0, 0);

  const FiniteRepetitionIF<T> tiled(base, period, repeatLo, repeatHi);

  // Well beyond the +1 tile: folds relative to the tile-1 anchor (x - 1*period), not back toward
  // tile 0 and not extrapolated as if more tiles existed.
  const Vec3 farBeyond(23.0, 0, 0);
  const Vec3 expectedLocal = farBeyond - Vec3(5.0, 0, 0);
  REQUIRE_THAT(tiled.value(farBeyond), WithinAbs(base->signedDistance(expectedLocal), 1.0e-9));

  // Symmetric check on the negative side.
  const Vec3 farBelow(-23.0, 0, 0);
  const Vec3 expectedLocalNeg = farBelow + Vec3(5.0, 0, 0);
  REQUIRE_THAT(tiled.value(farBelow), WithinAbs(base->signedDistance(expectedLocalNeg), 1.0e-9));
}

TEST_CASE("FiniteRepetition: free function matches FiniteRepetitionIF", "[CSG][FiniteRepetition]")
{
  const auto base = std::make_shared<Sphere>(Vec3::zeros(), T(1));

  const Vec3 period(5, 5, 5);
  const Vec3 repeatLo(2, 2, 2);
  const Vec3 repeatHi(2, 2, 2);

  const auto                  freeFunc = FiniteRepetition<T>(base, period, repeatLo, repeatHi);
  const FiniteRepetitionIF<T> direct(base, period, repeatLo, repeatHi);

  for (const Vec3 p : {Vec3::zeros(), Vec3(7.3, -2.1, 0.4), Vec3(-12.0, 0, 0)}) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-9));
  }
}
