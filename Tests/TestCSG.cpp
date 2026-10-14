// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for the CSG combinators: the SmoothMin/SmoothMax/ExpMin/ExpMax blending primitives; the
// sharp and smoothly-blended union, intersection and difference and finite periodic repetition, built
// as an Expr (EBGeometry_Expr.hpp) and evaluated through a compiled Tape (EBGeometry_Tape.hpp),
// including the Tape's BVH evaluation of unions of many objects and the bounding volumes and distance
// qualities the Expr works out; and the value-type BVHUnion/BVHSmoothUnion classes
// (EBGeometry_BVHUnion.hpp). Uses analytic spheres as fixtures so every expected value is
// hand-computable.

#include "EBGeometry.hpp"
#include "TestConstructions.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestTapeUtils.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

template <class T>
using Sphere = SphereSDF<T>;

template <class T>
using Compiled = TestUtils::Compiled<T>;

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
Sphere<T>
sphereA()
{
  return Sphere<T>(Vec3T<T>::zeros(), T(1));
}

template <class T>
Sphere<T>
sphereB()
{
  return Sphere<T>(Vec3T<T>(3, 0, 0), T(1));
}

// Two substantially overlapping spheres, used where a non-trivial (both-negative) intersection
// region is needed.
template <class T>
Sphere<T>
sphereC()
{
  return Sphere<T>(Vec3T<T>::zeros(), T(2));
}

template <class T>
Sphere<T>
sphereD()
{
  return Sphere<T>(Vec3T<T>(1, 0, 0), T(2));
}

// A small sphere carved out of A, and a third operand for the n-ary overloads.
template <class T>
Sphere<T>
sphereE()
{
  return Sphere<T>(Vec3T<T>(0.5, 0, 0), T(0.3));
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

// Blend the two smallest of a list of values (or the two largest, for a smooth maximum), the way an
// n-ary smooth union (intersection) does.
template <class T, class Blend>
T
blendTwoSmallest(std::vector<T> a_values, const T a_smoothLen, Blend a_blend)
{
  std::sort(a_values.begin(), a_values.end());

  return a_blend(a_values[0], a_values[1], a_smoothLen);
}

template <class T, class Blend>
T
blendTwoLargest(std::vector<T> a_values, const T a_smoothLen, Blend a_blend)
{
  std::sort(a_values.begin(), a_values.end());

  const size_t n = a_values.size();

  return a_blend(a_values[n - 2], a_values[n - 1], a_smoothLen);
}

// Two independent code paths computing literally the same closed-form expression (a tape vs. the
// formula computed by hand from the shapes, a pairwise vs. a vector overload, ...) should agree
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

TEMPLATE_TEST_CASE("ExpMin and ExpMax: finite and close to the sharp min/max far from the blend region",
                   "[CSG][ExpMin]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // exp(-a/s) under- or overflows once |a|/s passes ~100 (float) or ~700 (double); the blend must
  // not, since a BVHSmoothUnion evaluates it at arbitrary distances from the surface.
  const T s = T(1);

  for (const T a : {T(200), T(-100), T(1000), T(-800)}) {
    const T b = a + T(5);

    INFO("a = " << a);
    REQUIRE(std::isfinite(ExpMin<T>(a, b, s)));
    REQUIRE(std::isfinite(ExpMax<T>(a, b, s)));
    REQUIRE_THAT(ExpMin<T>(a, b, s), withinAbsT(a - s * std::log1p(std::exp(-T(5))), formulaMargin<T>()));
    REQUIRE_THAT(ExpMax<T>(a, b, s), withinAbsT(b + s * std::log1p(std::exp(-T(5))), formulaMargin<T>()));
  }

  REQUIRE_THAT(ExpMax<T>(T(2), T(2), s), withinAbsT(T(2) + s * std::log(T(2)), formulaMargin<T>()));
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
// Union()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Union: value is the pointwise minimum of two disjoint spheres",
                   "[CSG][Union]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();

  const Compiled<T> u(Union(a, b));

  const Vec3 center = Vec3::zeros();
  const Vec3 far(10, 10, 10);

  REQUIRE_THAT(u(center), withinAbsT(std::min(a.signedDistance(center), b.signedDistance(center)), exactMargin<T>()));
  REQUIRE_THAT(u(far), withinAbsT(std::min(a.signedDistance(far), b.signedDistance(far)), exactMargin<T>()));

  // A point strictly inside A only.
  REQUIRE(u(center) < T(0.0));

  // The gap between the two spheres (x = 1.5) is outside both.
  REQUIRE(u(Vec3(1.5, 0, 0)) > T(0.0));
}

TEMPLATE_TEST_CASE("Union: the pairwise and vector overloads agree with the hand-computed minimum",
                   "[CSG][Union]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();
  const auto e = sphereE<T>();

  const Compiled<T> pairwise(Union(a, b));
  const Compiled<T> vectorArg(Union(std::vector<Sphere<T>>{a, b}));
  const Compiled<T> three(Union(std::vector<Sphere<T>>{a, b, e}));
  const Compiled<T> nested(Union(Union(a, b), e));

  for (const Vec3 p : {Vec3::zeros(), Vec3(3, 0, 0), Vec3(1.5, 0, 0), Vec3(0.6, 0.1, 0), Vec3(-10, 4, 2)}) {
    const T minAB  = std::min(a.signedDistance(p), b.signedDistance(p));
    const T minABE = std::min(minAB, e.signedDistance(p));

    REQUIRE_THAT(pairwise(p), withinAbsT(minAB, exactMargin<T>()));
    REQUIRE_THAT(vectorArg(p), withinAbsT(pairwise(p), exactMargin<T>()));
    REQUIRE_THAT(three(p), withinAbsT(minABE, exactMargin<T>()));
    REQUIRE_THAT(nested(p), withinAbsT(three(p), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// SmoothUnion()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SmoothUnion: converges to the sharp union far from the blend region",
                   "[CSG][SmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();

  const T           smoothLen = T(0.1);
  const Compiled<T> su(SmoothUnion(a, b, smoothLen));
  const Compiled<T> sharp(Union(a, b));

  // Deep inside A, far from B: |signedDistance(A) - signedDistance(B)| >> smoothLen.
  const Vec3 deepInA(0, 0, 0);
  REQUIRE_THAT(su(deepInA), withinAbsT(sharp(deepInA), asymptoticMargin<T>()));
}

TEMPLATE_TEST_CASE("SmoothUnion: at least as deep (never shallower) than the sharp union everywhere",
                   "[CSG][SmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();

  for (const BlendKind kind : {BlendKind::Polynomial, BlendKind::Exponential}) {
    const Compiled<T> su(SmoothUnion(a, b, T(0.75), kind));
    const Compiled<T> sharp(Union(a, b));

    for (const T x : sweepValues<T>(T(-2.0), T(5.0), 28)) {
      const Vec3 p(x, 0, 0);
      REQUIRE(su(p) <= sharp(p) + T(exactMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("SmoothUnion: the pairwise and vector overloads agree with the hand-computed blend",
                   "[CSG][SmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();
  const auto e = sphereE<T>();

  const T           smoothLen = T(0.5);
  const Compiled<T> pairwise(SmoothUnion(a, b, smoothLen));
  const Compiled<T> vectorArg(SmoothUnion(std::vector<Sphere<T>>{a, b}, smoothLen));
  const Compiled<T> three(SmoothUnion(std::vector<Sphere<T>>{a, b, e}, smoothLen));

  for (const Vec3 p : {Vec3::zeros(), Vec3(1.5, 0, 0), Vec3(3, 0, 0), Vec3(0.6, 0.1, 0), Vec3(1, 0.5, 0)}) {
    const T da = a.signedDistance(p);
    const T db = b.signedDistance(p);
    const T de = e.signedDistance(p);

    REQUIRE_THAT(pairwise(p), withinAbsT(SmoothMinOp<T>{}(da, db, smoothLen), exactMargin<T>()));
    REQUIRE_THAT(vectorArg(p), withinAbsT(pairwise(p), exactMargin<T>()));

    // Only the two smallest values are blended.
    REQUIRE_THAT(three(p),
                 withinAbsT(blendTwoSmallest<T>({da, db, de}, smoothLen, SmoothMinOp<T>{}), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("SmoothUnion: BlendKind::Exponential blends the two smallest values with ExpMinOp",
                   "[CSG][SmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();
  const auto e = sphereE<T>();

  const T smoothLen = T(0.4);

  const Compiled<T> pairwise(SmoothUnion(a, b, smoothLen, BlendKind::Exponential));
  const Compiled<T> three(SmoothUnion(std::vector<Sphere<T>>{a, b, e}, smoothLen, BlendKind::Exponential));

  for (const Vec3 p : {Vec3::zeros(), Vec3(1.5, 0, 0), Vec3(3, 0, 0), Vec3(0.6, 0.1, 0), Vec3(-4, 2, 1)}) {
    const T da = a.signedDistance(p);
    const T db = b.signedDistance(p);
    const T de = e.signedDistance(p);

    REQUIRE_THAT(pairwise(p), withinAbsT(ExpMinOp<T>{}(da, db, smoothLen), formulaMargin<T>()));
    REQUIRE_THAT(three(p), withinAbsT(blendTwoSmallest<T>({da, db, de}, smoothLen, ExpMinOp<T>{}), formulaMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// BVHUnion / BVHUnion() and BVHSmoothUnion / BVHSmoothUnion()
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// A small scene of spheres on a line, spaced so neighbors are disjoint -- enough primitives to
// give the BVH real partitioning depth (unlike the 2-sphere fixtures above).
constexpr int NumRowSpheres = 12;

template <class T>
Vec3T<T>
rowCenter(const int a_i)
{
  return Vec3T<T>(T(3.0) * T(a_i), T(0), T(0));
}

// The row as plain value-type spheres, which is what the BVH unions store.
template <class T>
std::vector<SphereSDF<T>>
sphereRow()
{
  std::vector<SphereSDF<T>> spheres;

  spheres.reserve(NumRowSpheres);

  for (int i = 0; i < NumRowSpheres; i++) {
    spheres.emplace_back(rowCenter<T>(i), T(1));
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
    bvs.push_back(sphereBV<T>(rowCenter<T>(i), T(1)));
  }

  return bvs;
}

template <class T>
std::vector<Vec3T<T>>
lineQueryPoints()
{
  std::vector<Vec3T<T>> pts;

  for (const T x : sweepValues<T>(T(-2.0), T(35.0), 28)) {
    pts.emplace_back(x, T(0.25), T(-0.5));
  }

  return pts;
}

// Brute-force sharp union: the smallest primitive value found by a full linear scan.
template <class T, class Primitive>
T
bruteMin(const std::vector<Primitive>& a_primitives, const Vec3T<T>& a_point)
{
  T nearest = std::numeric_limits<T>::infinity();

  for (const auto& primitive : a_primitives) {
    nearest = std::min(nearest, primitive.signedDistance(a_point));
  }

  return nearest;
}

// Brute-force smooth union: blend the two smallest primitive values found by a full linear scan
// instead of the pruned BVH traversal.
template <class T, class Primitive, class Blend>
T
bruteTwoNearest(const std::vector<Primitive>& a_primitives, const Vec3T<T>& a_point, const T a_smoothLen, Blend a_blend)
{
  T a = std::numeric_limits<T>::infinity();
  T b = std::numeric_limits<T>::infinity();

  for (const auto& primitive : a_primitives) {
    const T d = primitive.signedDistance(a_point);

    if (d < a) {
      b = a;
      a = d;
    }
    else if (d < b) {
      b = d;
    }
  }

  return a_blend(a, b, a_smoothLen);
}

template <class T>
using TestTriMesh = TriMeshSDF<T, 4, 4>;

// A 3x2 grid of dodecahedra (circumradius ~1.4), each translated before its TriMeshSDF is built, all
// in a_pool. This is how a union of several copies of one mesh is built without a Translate wrapper.
template <class T>
std::vector<TestTriMesh<T>>
dodecahedronGrid(Pool& a_pool)
{
  const auto triangles = Parser::readIntoTriangles<T>(std::string(EBGEOMETRY_TEST_DATA_DIR) + "/dodecahedron.obj");

  std::vector<TestTriMesh<T>> meshes;

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      const Vec3T<T> shift(T(4) * T(i), T(4) * T(j), T(0));

      auto shifted = triangles;

      for (auto& triangle : shifted) {
        auto vertices = triangle.getVertexPositions();

        for (auto& v : vertices) {
          v = v + shift;
        }

        triangle.setVertexPositions(vertices);
      }

      meshes.emplace_back(shifted, a_pool, BVH::Construction::SAH, 1);
    }
  }

  return meshes;
}

template <class T>
std::vector<BV<T>>
boundingVolumes(const std::vector<TestTriMesh<T>>& a_meshes)
{
  std::vector<BV<T>> bvs;

  for (const auto& mesh : a_meshes) {
    bvs.push_back(mesh.computeBoundingVolume());
  }

  return bvs;
}

template <class T>
std::vector<Vec3T<T>>
gridQueryPoints()
{
  std::vector<Vec3T<T>> pts;

  for (const T x : sweepValues<T>(T(-2.0), T(10.0), 13)) {
    pts.emplace_back(x, T(1.5), T(0.3));
    pts.emplace_back(x, T(4.2), T(-1.0));
  }

  return pts;
}

} // namespace

TEMPLATE_TEST_CASE("BVHUnion: a plain, trivially copyable value type", "[CSG][BVHUnion]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  STATIC_REQUIRE(std::is_trivially_copyable_v<BVHUnion<T, SphereSDF<T>, 4>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<BVHSmoothUnion<T, SphereSDF<T>, 4>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<BVHSmoothUnion<T, SphereSDF<T>, 4, ExpMinOp<T>>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<BVHUnion<T, TestTriMesh<T>, 4>>);
  STATIC_REQUIRE_FALSE(std::is_polymorphic_v<BVHUnion<T, SphereSDF<T>, 4>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<SmoothMinOp<T>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<SmoothMaxOp<T>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<ExpMinOp<T>>);
}

TEMPLATE_TEST_CASE("BVHUnion: agrees with a brute-force minimum over a row of spheres",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const auto spheres = sphereRow<T>();

  const BVHUnion<T, SphereSDF<T>, 4> bvhUnion(pool, spheres, sphereRowBVs<T>());

  for (const auto& p : lineQueryPoints<T>()) {
    REQUIRE_THAT(bvhUnion.signedDistance(p), withinAbsT(bruteMin(spheres, p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHUnion: every build strategy gives the same union as the default one",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  const auto spheres = sphereRow<T>();
  const auto bvs     = sphereRowBVs<T>();

  const BVHUnion<T, SphereSDF<T>, K> reference(pool, spheres, bvs);

  for (const auto build : allConstructions) {
    const BVHUnion<T, SphereSDF<T>, K> bvhUnion(pool, spheres, bvs, build);

    for (const auto& p : lineQueryPoints<T>()) {
      REQUIRE_THAT(bvhUnion.signedDistance(p), withinAbsT(reference.signedDistance(p), exactMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("BVHSmoothUnion: agrees with the sharp union far from any blend region",
                   "[CSG][BVHSmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const T smoothLen = T(0.05); // Small relative to the 1-unit gap between spheres.

  Pool pool(hostMemoryResource());

  const auto spheres = sphereRow<T>();

  const BVHSmoothUnion<T, SphereSDF<T>, 4> bvhSmooth(pool, spheres, sphereRowBVs<T>(), smoothLen);

  // Deep inside any one sphere, far from every other sphere's surface.
  for (int i = 0; i < NumRowSpheres; i++) {
    const Vec3T<T> deepInside = rowCenter<T>(i);

    REQUIRE_THAT(bvhSmooth.signedDistance(deepInside),
                 withinAbsT(bruteMin(spheres, deepInside), asymptoticMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHSmoothUnion: matches a brute-force two-nearest blend inside the blend region",
                   "[CSG][BVHSmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const T smoothLen = T(0.6); // Large enough to actively blend across the 1-unit surface gaps.

  Pool pool(hostMemoryResource());

  const auto spheres = sphereRow<T>();
  const auto bvs     = sphereRowBVs<T>();

  // The pruning bound must retain *both* blend inputs even in the overlap region where the two
  // nearest surfaces interpenetrate. Checked for both the default and the exponential blend.
  const BVHSmoothUnion<T, SphereSDF<T>, 4>              polySmooth(pool, spheres, bvs, smoothLen);
  const BVHSmoothUnion<T, SphereSDF<T>, 4, ExpMinOp<T>> expSmooth(pool, spheres, bvs, smoothLen);

  for (const auto& p : lineQueryPoints<T>()) {
    REQUIRE_THAT(polySmooth.signedDistance(p),
                 withinAbsT(bruteTwoNearest(spheres, p, smoothLen, SmoothMinOp<T>{}), exactMargin<T>()));
    REQUIRE_THAT(expSmooth.signedDistance(p),
                 withinAbsT(bruteTwoNearest(spheres, p, smoothLen, ExpMinOp<T>{}), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHSmoothUnion: every build strategy matches brute force when the leaf count is not a "
                   "power of K",
                   "[CSG][BVHSmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  // 68 spheres give 17 leaves of K = 4 under the space-filling-curve builds. A builder that pads the
  // leaf count up to a power of K by repeating a leaf visits some spheres twice, and the smooth
  // union then blends a sphere with itself.
  std::mt19937                      rng(1);
  std::uniform_real_distribution<T> coord(T(0), T(10));

  std::vector<SphereSDF<T>> spheres;
  std::vector<BV<T>>        bvs;

  for (int i = 0; i < 68; i++) {
    const Vec3T<T> center(coord(rng), coord(rng), coord(rng));
    const T        radius = T(0.3);

    spheres.emplace_back(center, radius);
    bvs.emplace_back(center - radius * Vec3T<T>::ones(), center + radius * Vec3T<T>::ones());
  }

  std::vector<Vec3T<T>> queries;

  for (int i = 0; i < 2000; i++) {
    queries.emplace_back(coord(rng), coord(rng), coord(rng));
  }

  const T smoothLen = T(0.5);

  Pool pool(hostMemoryResource());

  for (const auto build : allConstructions) {
    const BVHSmoothUnion<T, SphereSDF<T>, K> smooth(pool, spheres, bvs, smoothLen, SmoothMinOp<T>{}, build);

    for (const auto& p : queries) {
      REQUIRE_THAT(smooth.signedDistance(p),
                   withinAbsT(bruteTwoNearest(spheres, p, smoothLen, SmoothMinOp<T>{}), formulaMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("BVHUnion/BVHSmoothUnion: ConstructionOptions set each construction method's leaf size",
                   "[CSG][BVHUnion][BVHSmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  using Union  = BVHUnion<T, SphereSDF<T>, K>;
  using Smooth = BVHSmoothUnion<T, SphereSDF<T>, K>;

  std::mt19937                      rng(5);
  std::uniform_real_distribution<T> coord(T(0), T(10));

  std::vector<SphereSDF<T>> spheres;
  std::vector<BV<T>>        bvs;

  for (int i = 0; i < 300; i++) {
    const Vec3T<T> center(coord(rng), coord(rng), coord(rng));
    const T        radius = T(0.2);

    spheres.emplace_back(center, radius);
    bvs.emplace_back(center - radius * Vec3T<T>::ones(), center + radius * Vec3T<T>::ones());
  }

  std::vector<Vec3T<T>> queries;

  for (int i = 0; i < 500; i++) {
    queries.emplace_back(coord(rng), coord(rng), coord(rng));
  }

  const T smoothLen = T(0.5);

  // The largest leaf of a union's BVH, in primitives.
  const auto largestLeaf = [](const auto& a_bvh) {
    const auto nodes   = a_bvh.getNodes();
    uint32_t   largest = 0;

    for (uint32_t i = 0; i < nodes.size(); i++) {
      if (nodes[i].isLeaf()) {
        largest = std::max(largest, nodes[i].getNumPrimitives());
      }
    }

    return largest;
  };

  Pool pool(hostMemoryResource());

  // Each method reads only its own field; the other two stay 0 to show they are unread. Targets of
  // 2 and up are always reachable for 300 primitives (one primitive per leaf is not, since a K-ary
  // tree with every interior node full has L = 1 (mod K - 1) leaves).
  for (const size_t setting : {size_t(2), size_t(7), size_t(40), spheres.size()}) {
    for (const auto build : allConstructions) {
      const bool topDown = build == BVH::Construction::CentroidSplit || build == BVH::Construction::MidpointSplit ||
                           build == BVH::Construction::SAH;
      const bool curve =
        build == BVH::Construction::Morton || build == BVH::Construction::Nested || build == BVH::Construction::Hilbert;

      BVH::ConstructionOptions options;

      if (topDown) {
        options.maxLeafSize = setting;
      }
      else if (curve) {
        options.targetLeafSize = setting;
      }
      else {
        options.cluster.maxClusterSize = setting;
      }

      const Union  sharp(pool, spheres, bvs, build, options);
      const Smooth smooth(pool, spheres, bvs, smoothLen, SmoothMinOp<T>{}, build, options);

      INFO("setting = " << setting);

      // A top-down split makes K non-empty children, so a node of fewer than K is a leaf anyway.
      const size_t bound = topDown ? std::max(setting, K - 1) : curve ? setting : (K - 1) * setting;

      REQUIRE(largestLeaf(sharp.getBVH()) <= bound);
      REQUIRE(largestLeaf(smooth.getBVH()) <= bound);

      if ((topDown || curve) && setting == spheres.size()) {
        REQUIRE(sharp.getBVH().getNodes().size() == 1);
      }

      for (const auto& p : queries) {
        T nearest = std::numeric_limits<T>::infinity();

        for (const auto& sphere : spheres) {
          nearest = std::min(nearest, sphere.signedDistance(p));
        }

        REQUIRE_THAT(sharp.signedDistance(p), withinAbsT(nearest, exactMargin<T>()));
        REQUIRE_THAT(smooth.signedDistance(p),
                     withinAbsT(bruteTwoNearest(spheres, p, smoothLen, SmoothMinOp<T>{}), formulaMargin<T>()));
      }
    }
  }

  // The constructors without options build the trees of defaultConstructionOptions().
  for (const auto build : allConstructions) {
    const Union  implicitSharp(pool, spheres, bvs, build);
    const Union  explicitSharp(pool, spheres, bvs, build, Union::defaultConstructionOptions());
    const Smooth implicitSmooth(pool, spheres, bvs, smoothLen, SmoothMinOp<T>{}, build);
    const Smooth explicitSmooth(
      pool, spheres, bvs, smoothLen, SmoothMinOp<T>{}, build, Smooth::defaultConstructionOptions());

    const auto lhs = implicitSharp.getBVH().getNodes();
    const auto rhs = explicitSharp.getBVH().getNodes();

    REQUIRE(lhs.size() == rhs.size());
    REQUIRE(implicitSmooth.getBVH().getNodes().size() == explicitSmooth.getBVH().getNodes().size());

    for (uint32_t i = 0; i < lhs.size(); i++) {
      REQUIRE(lhs[i].getPrimitivesOffset() == rhs[i].getPrimitivesOffset());
      REQUIRE(lhs[i].getNumPrimitives() == rhs[i].getNumPrimitives());
    }
  }

  // A zero setting is rejected when the method reads it, in every build.
  REQUIRE(abortsWith(
    [&] {
      Pool        local(hostMemoryResource());
      const Union bad(local, spheres, bvs, BVH::Construction::SAH, BVH::ConstructionOptions{0, 1, {}});

      (void)bad;
    },
    "BVHUnion: ConstructionOptions::maxLeafSize must be positive"));

  REQUIRE(abortsWith(
    [&] {
      Pool         local(hostMemoryResource());
      const Smooth bad(local,
                       spheres,
                       bvs,
                       smoothLen,
                       SmoothMinOp<T>{},
                       BVH::Construction::Morton,
                       BVH::ConstructionOptions{1, 0, {}});

      (void)bad;
    },
    "ConstructionOptions::targetLeafSize must be positive"));
}

TEMPLATE_TEST_CASE("BVHUnion: every build strategy handles many coincident primitives",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Identical bounding volumes give every splitting rule a zero extent to work with.
  std::vector<SphereSDF<T>> spheres(2000, SphereSDF<T>(Vec3T<T>::ones(), T(0.5)));
  std::vector<BV<T>>        bvs(2000, BV<T>(T(0.5) * Vec3T<T>::ones(), T(1.5) * Vec3T<T>::ones()));

  spheres.emplace_back(T(5) * Vec3T<T>::ones(), T(0.5));
  bvs.emplace_back(T(4.5) * Vec3T<T>::ones(), T(5.5) * Vec3T<T>::ones());

  Pool pool(hostMemoryResource());

  for (const auto build : allConstructions) {
    const BVHUnion<T, SphereSDF<T>, 4> bvhUnion(pool, spheres, bvs, build);

    REQUIRE_THAT(bvhUnion.signedDistance(Vec3T<T>::zeros()), withinAbsT(std::sqrt(T(3)) - T(0.5), formulaMargin<T>()));
    REQUIRE_THAT(bvhUnion.signedDistance(T(5) * Vec3T<T>::ones()), withinAbsT(T(-0.5), formulaMargin<T>()));
  }
}

namespace {

// A sphere whose value is ten times its distance: the right sign, but it overestimates the distance
// outside, so it is not a distance bound.
template <class T>
struct SteepSphere
{
  static constexpr DistanceQuality distanceQuality = DistanceQuality::NotADistance;

  SphereSDF<T> m_sphere;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return T(10) * m_sphere.signedDistance(a_point);
  }
};

} // namespace

TEMPLATE_TEST_CASE("BVHUnion: primitives that are not distances keep the union's sign",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Pruning only needs each box to enclose its primitive's object: a primitive skipped for a box
  // further away than the best value so far is positive there, whatever its value.
  std::mt19937                      rng(2468);
  std::uniform_real_distribution<T> coord(T(-4), T(4));

  std::vector<SteepSphere<T>> spheres;
  std::vector<BV<T>>          bvs;

  for (int i = 0; i < 200; i++) {
    const SphereSDF<T> sphere(Vec3(coord(rng), coord(rng), coord(rng)), T(0.3));

    spheres.push_back(SteepSphere<T>{sphere});
    bvs.push_back(sphere.computeBoundingVolume());
  }

  STATIC_REQUIRE(BVHUnion<T, SteepSphere<T>, 4>::distanceQuality == DistanceQuality::NotADistance);

  Pool pool(hostMemoryResource());

  for (const auto build : allConstructions) {
    const BVHUnion<T, SteepSphere<T>, 4> bvhUnion(pool, spheres, bvs, build);

    for (int i = 0; i < 2000; i++) {
      const Vec3 p(coord(rng), coord(rng), coord(rng));

      T bruteForce = Math::Limits<T>::max();

      for (const auto& sphere : spheres) {
        bruteForce = Math::min(bruteForce, sphere.signedDistance(p));
      }

      INFO("p = " << p << ", brute force " << bruteForce);
      REQUIRE((bvhUnion.signedDistance(p) < T(0)) == (bruteForce < T(0)));
    }
  }
}

TEMPLATE_TEST_CASE("BVHUnion: primitives with unbounded boxes match brute force",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Infinite cylinders are unbounded along their axis and infinite cones everywhere but above the
  // tip; their boxes reach the largest finite value there.
  std::mt19937                      rng(1357);
  std::uniform_real_distribution<T> coord(T(-5), T(5));

  std::vector<InfiniteCylinderSDF<T>> cylinders;
  std::vector<InfiniteConeSDF<T>>     cones;

  for (size_t i = 0; i < 30; i++) {
    cylinders.emplace_back(Vec3(coord(rng), coord(rng), coord(rng)), T(0.3), i % 3);
    cones.emplace_back(Vec3(coord(rng), coord(rng), coord(rng)), T(40));
  }

  const auto check = [&](const auto& a_primitives) {
    using P = typename std::decay_t<decltype(a_primitives)>::value_type;

    std::vector<BV<T>> bvs;

    for (const auto& primitive : a_primitives) {
      bvs.push_back(primitive.computeBoundingVolume());
    }

    Pool pool(hostMemoryResource());

    for (const auto build : allConstructions) {
      const BVHUnion<T, P, 4> bvhUnion(pool, a_primitives, bvs, build);

      // Every box is unbounded, so the BVH is empty and every primitive is scanned.
      REQUIRE(bvhUnion.getBVH().getPrimitives().size() == 0);
      REQUIRE(bvhUnion.getUnboundedPrimitives().size() == a_primitives.size());

      for (int i = 0; i < 500; i++) {
        const Vec3 p(T(2) * coord(rng), T(2) * coord(rng), T(2) * coord(rng));

        T bruteForce = Math::Limits<T>::max();

        for (const auto& primitive : a_primitives) {
          bruteForce = Math::min(bruteForce, primitive.signedDistance(p));
        }

        REQUIRE_THAT(bvhUnion.signedDistance(p), withinAbsT(bruteForce, formulaMargin<T>()));
      }
    }
  };

  check(cylinders);
  check(cones);
}

namespace {

// Either a sphere or an infinite cylinder: one primitive type with bounded and unbounded members.
template <class T>
struct SphereOrCylinder
{
  SphereSDF<T>           m_sphere;
  InfiniteCylinderSDF<T> m_cylinder;
  bool                   m_isCylinder = false;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_isCylinder ? m_cylinder.signedDistance(a_point) : m_sphere.signedDistance(a_point);
  }

  BV<T>
  computeBoundingVolume() const noexcept
  {
    return m_isCylinder ? m_cylinder.computeBoundingVolume() : m_sphere.computeBoundingVolume();
  }
};

// 300 small spheres scattered in [-6, 6]^3 and four thin infinite cylinders, one along x, two along
// y and one along z.
template <class T>
std::vector<SphereOrCylinder<T>>
spheresAndCylinders()
{
  std::mt19937                      rng(97531);
  std::uniform_real_distribution<T> coord(T(-6), T(6));

  std::vector<SphereOrCylinder<T>> primitives;

  for (int i = 0; i < 300; i++) {
    primitives.push_back({SphereSDF<T>(Vec3T<T>(coord(rng), coord(rng), coord(rng)), T(0.3)), {}, false});
  }

  for (const size_t axis : {size_t(0), size_t(1), size_t(2), size_t(1)}) {
    primitives.push_back(
      {{}, InfiniteCylinderSDF<T>(Vec3T<T>(coord(rng), coord(rng), coord(rng)), T(0.2), axis), true});
  }

  return primitives;
}

template <class T, class Primitive>
std::vector<BV<T>>
boxesOf(const std::vector<Primitive>& a_primitives)
{
  std::vector<BV<T>> bvs;

  bvs.reserve(a_primitives.size());

  for (const auto& primitive : a_primitives) {
    bvs.push_back(primitive.computeBoundingVolume());
  }

  return bvs;
}

} // namespace

TEMPLATE_TEST_CASE("BVH unions: primitives with unbounded boxes are kept out of the BVH and still evaluated",
                   "[CSG][BVHUnion][BVHSmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Prim = SphereOrCylinder<T>;

  const auto primitives = spheresAndCylinders<T>();
  const auto bvs        = boxesOf<T>(primitives);
  const T    smoothLen  = T(0.5);

  std::mt19937                      rng(8642);
  std::uniform_real_distribution<T> coord(T(-8), T(8));

  std::vector<Vec3> points;

  for (int i = 0; i < 400; i++) {
    points.emplace_back(coord(rng), coord(rng), coord(rng));
  }

  Pool pool(hostMemoryResource());

  for (const auto build : allConstructions) {
    const BVHUnion<T, Prim, 4>       sharp(pool, primitives, bvs, build);
    const BVHSmoothUnion<T, Prim, 4> smooth(pool, primitives, bvs, smoothLen, SmoothMinOp<T>{}, build);

    REQUIRE(sharp.getBVH().getPrimitives().size() == 300);
    REQUIRE(sharp.getUnboundedPrimitives().size() == 4);
    REQUIRE(smooth.getUnboundedPrimitives().size() == 4);

    for (const auto& p : points) {
      T bruteForce = Math::Limits<T>::max();

      for (const auto& primitive : primitives) {
        bruteForce = Math::min(bruteForce, primitive.signedDistance(p));
      }

      REQUIRE_THAT(sharp.signedDistance(p), withinAbsT(bruteForce, exactMargin<T>()));
      REQUIRE_THAT(smooth.signedDistance(p),
                   withinAbsT(bruteTwoNearest(primitives, p, smoothLen, SmoothMinOp<T>{}), exactMargin<T>()));
    }

    // The union's box takes in the cylinders' boxes: unbounded along x, y and z.
    const BV<T> box = sharp.computeBoundingVolume();

    for (size_t dir = 0; dir < 3; dir++) {
      REQUIRE(box.getLowCorner()[dir] == -Math::Limits<T>::max());
      REQUIRE(box.getHighCorner()[dir] == Math::Limits<T>::max());
    }
  }
}

TEMPLATE_TEST_CASE("BVHUnion: host-mirror, deep and nested copies keep the unbounded primitives",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Union  = BVHUnion<T, SphereOrCylinder<T>, 4>;
  using Smooth = BVHSmoothUnion<T, SphereOrCylinder<T>, 4>;
  using Nested = BVHUnion<T, Union, 4>;

  const auto queries = EBGeometryTestGPU::queryGrid<T>(Vec3T<T>(T(-8), T(-8), T(-8)), Vec3T<T>(T(8), T(8), T(8)), 12);

  std::vector<T> expected;
  std::vector<T> expectedSmooth;

  Pool mirror(hostMemoryResource());
  Pool copyPool(hostMemoryResource());

  std::optional<Union>  mirrorView;
  std::optional<Smooth> smoothMirrorView;
  std::optional<Nested> nestedMirrorView;
  std::optional<Union>  deepCopied;
  std::optional<Smooth> smoothDeepCopied;
  std::optional<Nested> nestedDeepCopied;

  {
    Pool pool(hostMemoryResource());

    const auto   primitives = spheresAndCylinders<T>();
    const auto   bvs        = boxesOf<T>(primitives);
    const Union  sharp(pool, primitives, bvs);
    const Smooth smooth(pool, primitives, bvs, T(0.5));

    // The inner union's box is unbounded, so the outer union holds it -- a pool-resident primitive --
    // in its own unbounded leaf.
    const Nested nested(pool, {sharp}, {sharp.computeBoundingVolume()});

    REQUIRE(nested.getUnboundedPrimitives().size() == 1);

    for (const auto& p : queries) {
      expected.push_back(sharp.signedDistance(p));
      expectedSmooth.push_back(smooth.signedDistance(p));
    }

    deepCopied.emplace(sharp.deepCopy(copyPool));
    smoothDeepCopied.emplace(smooth.deepCopy(copyPool));
    nestedDeepCopied.emplace(nested.deepCopy(copyPool));

    pool.freeze();
    mirror = Pool::mirror(pool, hostMemoryResource());

    mirrorView.emplace(sharp.rebasedView(mirror));
    smoothMirrorView.emplace(smooth.rebasedView(mirror));
    nestedMirrorView.emplace(nested.rebasedView(mirror));
  }

  // The source pool is gone; every copy resolves both its BVH and its unbounded primitives against
  // its own pool.
  REQUIRE(deepCopied->isAttachedTo(copyPool));
  REQUIRE(nestedDeepCopied->isAttachedTo(copyPool));

  for (size_t i = 0; i < queries.size(); i++) {
    REQUIRE(mirrorView->signedDistance(queries[i]) == expected[i]);
    REQUIRE(nestedMirrorView->signedDistance(queries[i]) == expected[i]);
    REQUIRE(deepCopied->signedDistance(queries[i]) == expected[i]);
    REQUIRE(nestedDeepCopied->signedDistance(queries[i]) == expected[i]);
    REQUIRE(smoothMirrorView->signedDistance(queries[i]) == expectedSmooth[i]);
    REQUIRE(smoothDeepCopied->signedDistance(queries[i]) == expectedSmooth[i]);
  }
}

TEMPLATE_TEST_CASE("BVHUnion::computeBoundingVolume encloses every input sphere",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const auto bvs = sphereRowBVs<T>();

  const BVHUnion<T, SphereSDF<T>, 4> bvhUnion(pool, sphereRow<T>(), bvs);
  const BV<T>                        rootBV = bvhUnion.computeBoundingVolume();

  for (const auto& bv : bvs) {
    for (size_t dir = 0; dir < 3; dir++) {
      REQUIRE(rootBV.getLowCorner()[dir] <= bv.getLowCorner()[dir] + T(exactMargin<T>()));
      REQUIRE(rootBV.getHighCorner()[dir] >= bv.getHighCorner()[dir] - T(exactMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("BVHUnion: a union of translated TriMeshSDFs, and a union of that union, match brute force",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Union  = BVHUnion<T, TestTriMesh<T>, 4>;
  using Nested = BVHUnion<T, Union, 4>;

  Pool pool(hostMemoryResource());

  const auto meshes = dodecahedronGrid<T>(pool);

  const Union  meshUnion(pool, meshes, boundingVolumes(meshes));
  const Nested nested(pool, {meshUnion}, {meshUnion.computeBoundingVolume()});

  for (const auto& p : gridQueryPoints<T>()) {
    T brute = std::numeric_limits<T>::infinity();

    for (const auto& mesh : meshes) {
      brute = std::min(brute, mesh.signedDistance(p));
    }

    REQUIRE_THAT(meshUnion.signedDistance(p), withinAbsT(brute, exactMargin<T>()));
    REQUIRE_THAT(nested.signedDistance(p), withinAbsT(brute, exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHUnion: host-mirror and deep copies of a TriMeshSDF union outlive the source pool",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Union  = BVHUnion<T, TestTriMesh<T>, 4>;
  using Nested = BVHUnion<T, Union, 4>;

  const auto queries = gridQueryPoints<T>();

  std::vector<T> expected;

  Pool mirror(hostMemoryResource());
  Pool copyPool(hostMemoryResource());

  std::optional<Union>  mirrorView;
  std::optional<Nested> nestedMirrorView;
  std::optional<Union>  deepCopied;

  {
    Pool pool(hostMemoryResource());

    const auto   meshes = dodecahedronGrid<T>(pool);
    const Union  meshUnion(pool, meshes, boundingVolumes(meshes));
    const Nested nested(pool, {meshUnion}, {meshUnion.computeBoundingVolume()});

    for (const auto& p : queries) {
      expected.push_back(meshUnion.signedDistance(p));
    }

    deepCopied.emplace(meshUnion.deepCopy(copyPool));

    pool.freeze();
    mirror = Pool::mirror(pool, hostMemoryResource());

    mirrorView.emplace(meshUnion.rebasedView(mirror));
    nestedMirrorView.emplace(nested.rebasedView(mirror));
  }

  // The source pool is gone. The meshes stored inside the union are layouts, with no location of their
  // own, so each view resolves them against its own pool by pairing them with the union's location.
  REQUIRE(deepCopied->isAttachedTo(copyPool));

  for (size_t i = 0; i < queries.size(); i++) {
    REQUIRE(mirrorView->signedDistance(queries[i]) == expected[i]);
    REQUIRE(nestedMirrorView->signedDistance(queries[i]) == expected[i]);
    REQUIRE(deepCopied->signedDistance(queries[i]) == expected[i]);
  }
}

TEMPLATE_TEST_CASE("BVHUnion: rejects a mesh from another pool and a missing bounding volume",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Union = BVHUnion<T, TestTriMesh<T>, 4>;

  // Both checks are EBGEOMETRY_REQUIREs, so they abort in every build.
  REQUIRE_FALSE(aborts([] {
    Pool        pool(hostMemoryResource());
    const auto  meshes = dodecahedronGrid<T>(pool);
    const Union meshUnion(pool, meshes, boundingVolumes(meshes));

    (void)meshUnion;
  }));

  REQUIRE(abortsWith(
    [] {
      Pool        meshPool(hostMemoryResource());
      Pool        unionPool(hostMemoryResource());
      const auto  meshes = dodecahedronGrid<T>(meshPool);
      const Union meshUnion(unionPool, meshes, boundingVolumes(meshes));

      (void)meshUnion;
    },
    "lives in another Pool"));

  REQUIRE(abortsWith(
    [] {
      Pool       pool(hostMemoryResource());
      const auto spheres = sphereRow<T>();
      auto       bvs     = sphereRowBVs<T>();

      bvs.pop_back();

      const BVHUnion<T, SphereSDF<T>, 4> sphereUnion(pool, spheres, bvs);

      (void)sphereUnion;
    },
    "need one bounding volume per primitive"));
}

namespace {

// One signed distance per query point, for any of the BVH unions, held by value as a kernel receives it.
template <class T, class Union>
struct UnionDistanceQuery
{
  Union m_union;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_union.signedDistance(a_point);
  }
};

} // namespace

TEMPLATE_TEST_CASE("BVH unions: device signedDistance matches the host", "[CSG][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const auto meshes = dodecahedronGrid<T>(pool);

  const BVHUnion<T, SphereSDF<T>, 4>       sphereUnion(pool, sphereRow<T>(), sphereRowBVs<T>());
  const BVHSmoothUnion<T, SphereSDF<T>, 4> smoothUnion(pool, sphereRow<T>(), sphereRowBVs<T>(), T(0.6));
  const BVHUnion<T, TestTriMesh<T>, 4>     meshUnion(pool, meshes, boundingVolumes(meshes));

  // Spheres and infinite cylinders: the unbounded leaf is scanned on the device too.
  const auto                                      mixed = spheresAndCylinders<T>();
  const BVHUnion<T, SphereOrCylinder<T>, 4>       mixedUnion(pool, mixed, boxesOf<T>(mixed));
  const BVHSmoothUnion<T, SphereOrCylinder<T>, 4> mixedSmooth(pool, mixed, boxesOf<T>(mixed), T(0.5));

  pool.freeze();

  Pool devicePool = Pool::mirror(pool, deviceTestResource());

  // The unit spheres sit at x = 0, 3, ..., 33 on the x-axis, and the dodecahedra (vertices within 1.62
  // of their centres) at x = 0, 4, 8 and y = 0, 4. Each grid covers points inside, outside, near the
  // surfaces and between the primitives of its union.
  const T    rowEnd       = T(3) * T(NumRowSpheres - 1) + T(2);
  const auto spherePoints = queryGrid<T>(Vec3T<T>(T(-2), T(-2), T(-2)), Vec3T<T>(rowEnd, T(2), T(2)), 16);
  const auto meshPoints   = queryGrid<T>(Vec3T<T>(T(-2), T(-2), T(-2)), Vec3T<T>(T(10), T(6), T(2)), 16);

  const auto check = [&](const auto& a_union, const std::vector<Vec3T<T>>& a_points) {
    using Union = std::decay_t<decltype(a_union)>;

    const Union view = a_union.rebasedView(devicePool);

    requireSameResults(evaluateOnDevice<T>(UnionDistanceQuery<T, Union>{view}, a_points),
                       evaluateOnHost<T>(UnionDistanceQuery<T, Union>{a_union}, a_points));
  };

  check(sphereUnion, spherePoints);
  check(smoothUnion, spherePoints);
  check(meshUnion, meshPoints);

  const auto mixedPoints = queryGrid<T>(Vec3T<T>(T(-8), T(-8), T(-8)), Vec3T<T>(T(8), T(8), T(8)), 16);

  check(mixedUnion, mixedPoints);
  check(mixedSmooth, mixedPoints);
}

// ─────────────────────────────────────────────────────────────────────────────
// Unions of many objects in a Tape: BVH evaluation and its fallbacks
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// 50 spheres of radius 0.4 scattered in [-5, 5]^3: enough operands for the tape to build a BVH over
// them with the default TapeOptions, and close enough for some to overlap.
template <class T>
std::vector<Sphere<T>>
scatteredSpheres()
{
  std::mt19937                      rng(4242);
  std::uniform_real_distribution<T> coord(T(-5), T(5));

  std::vector<Sphere<T>> spheres;

  for (int i = 0; i < 50; i++) {
    spheres.emplace_back(Vec3T<T>(coord(rng), coord(rng), coord(rng)), T(0.4));
  }

  return spheres;
}

template <class T>
std::vector<Vec3T<T>>
scatteredQueries(const int a_count, const unsigned a_seed)
{
  std::mt19937                      rng(a_seed);
  std::uniform_real_distribution<T> coord(T(-7), T(7));

  std::vector<Vec3T<T>> points;

  for (int i = 0; i < a_count; i++) {
    points.emplace_back(coord(rng), coord(rng), coord(rng));
  }

  return points;
}

// TapeOptions under which no union is evaluated with a BVH.
inline TapeOptions
linearScanOptions()
{
  TapeOptions options;

  options.m_bvhUnionThreshold = 1000;

  return options;
}

} // namespace

TEMPLATE_TEST_CASE("Union: a union and a smooth union of many spheres give the same values with and without a BVH",
                   "[CSG][Union][SmoothUnion][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto spheres   = scatteredSpheres<T>();
  const T    smoothLen = T(0.5);

  const Compiled<T> bvhUnion(Union(spheres));
  const Compiled<T> linearUnion(Union(spheres), linearScanOptions());
  const Compiled<T> bvhSmooth(SmoothUnion(spheres, smoothLen));
  const Compiled<T> linearSmooth(SmoothUnion(spheres, smoothLen), linearScanOptions());

  REQUIRE(bvhUnion.tape().getNumBVHUnions() == 1);
  REQUIRE(linearUnion.tape().getNumBVHUnions() == 0);
  REQUIRE(bvhSmooth.tape().getNumBVHUnions() == 1);
  REQUIRE(linearSmooth.tape().getNumBVHUnions() == 0);

  for (const auto& p : scatteredQueries<T>(1000, 11)) {
    INFO("p = " << p);

    REQUIRE_THAT(bvhUnion(p), withinAbsT(linearUnion(p), exactMargin<T>()));
    REQUIRE_THAT(bvhUnion(p), withinAbsT(bruteMin(spheres, p), exactMargin<T>()));

    REQUIRE_THAT(bvhSmooth(p), withinAbsT(linearSmooth(p), exactMargin<T>()));
    REQUIRE_THAT(bvhSmooth(p), withinAbsT(bruteTwoNearest(spheres, p, smoothLen, SmoothMinOp<T>{}), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Union: a union with an operand that is not a distance is scanned operand by operand",
                   "[CSG][Union][SmoothUnion][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T        = TestType;
  using Constant = TestUtils::ConstantSDF<T>;

  const auto     spheres = scatteredSpheres<T>();
  const Constant constant{T(0.25)};
  const T        smoothLen = T(0.5);

  // The constant shape, like a Perlin noise field, is NotADistance: a BVH could not prune by its box.
  std::vector<Expr<T>> operands(spheres.begin(), spheres.end());

  operands.insert(operands.begin() + 20, Expr<T>(constant));

  const TestUtils::Compiled<T, Constant> sharp(Union(operands));
  const TestUtils::Compiled<T, Constant> smooth(SmoothUnion(operands, smoothLen));

  REQUIRE(sharp.tape().getNumBVHUnions() == 0);
  REQUIRE(smooth.tape().getNumBVHUnions() == 0);
  REQUIRE(sharp.tape().getDistanceQuality() == DistanceQuality::NotADistance);

  for (const auto& p : scatteredQueries<T>(1000, 12)) {
    std::vector<T> values;

    for (const auto& sphere : spheres) {
      values.push_back(sphere.signedDistance(p));
    }

    values.push_back(constant.signedDistance(p));

    INFO("p = " << p);
    REQUIRE_THAT(sharp(p), withinAbsT(std::min(bruteMin(spheres, p), constant.m_value), exactMargin<T>()));
    REQUIRE_THAT(smooth(p), withinAbsT(blendTwoSmallest(values, smoothLen, SmoothMinOp<T>{}), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Union: unbounded operands inside a BVH union match brute force",
                   "[CSG][Union][SmoothUnion][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto spheres   = scatteredSpheres<T>();
  const T    smoothLen = T(0.5);

  // A tilted plane (everything below it is inside) and three thin infinite cylinders, one per axis.
  const PlaneSDF<T>                         plane(Vec3(0, 0, -6), Vec3(T(0.1), T(-0.2), T(1)));
  const std::vector<InfiniteCylinderSDF<T>> cylinders{InfiniteCylinderSDF<T>(Vec3(1, 2, 3), T(0.2), 0),
                                                      InfiniteCylinderSDF<T>(Vec3(-2, 0, 1), T(0.3), 1),
                                                      InfiniteCylinderSDF<T>(Vec3(3, -3, 0), T(0.25), 2)};

  std::vector<Expr<T>> operands(spheres.begin(), spheres.end());

  operands.insert(operands.begin() + 7, Expr<T>(plane));
  operands.insert(operands.end(), cylinders.begin(), cylinders.end());

  const Compiled<T> sharp(Union(operands));
  const Compiled<T> smooth(SmoothUnion(operands, smoothLen));

  REQUIRE(sharp.tape().getNumBVHUnions() == 1);
  REQUIRE(smooth.tape().getNumBVHUnions() == 1);

  for (const auto& p : scatteredQueries<T>(1000, 13)) {
    std::vector<T> values;

    for (const auto& sphere : spheres) {
      values.push_back(sphere.signedDistance(p));
    }

    for (const auto& cylinder : cylinders) {
      values.push_back(cylinder.signedDistance(p));
    }

    values.push_back(plane.signedDistance(p));

    INFO("p = " << p);
    REQUIRE_THAT(sharp(p), withinAbsT(*std::min_element(values.begin(), values.end()), exactMargin<T>()));
    REQUIRE_THAT(smooth(p), withinAbsT(blendTwoSmallest(values, smoothLen, SmoothMinOp<T>{}), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("SmoothUnion: an exponential smooth union of many spheres matches ExpMinOp on the two smallest",
                   "[CSG][SmoothUnion][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto spheres   = scatteredSpheres<T>();
  const T    smoothLen = T(0.3);

  const Compiled<T> bvhSmooth(SmoothUnion(spheres, smoothLen, BlendKind::Exponential));
  const Compiled<T> linearSmooth(SmoothUnion(spheres, smoothLen, BlendKind::Exponential), linearScanOptions());

  REQUIRE(bvhSmooth.tape().getNumBVHUnions() == 1);
  REQUIRE(linearSmooth.tape().getNumBVHUnions() == 0);

  for (const auto& p : scatteredQueries<T>(1000, 14)) {
    const T expected = bruteTwoNearest(spheres, p, smoothLen, ExpMinOp<T>{});

    INFO("p = " << p);
    REQUIRE_THAT(bvhSmooth(p), withinAbsT(expected, formulaMargin<T>()));
    REQUIRE_THAT(linearSmooth(p), withinAbsT(expected, formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Expr: the bounding volume of a union, intersection and difference",
                   "[CSG][Union][Intersection][Difference]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>(); // Box [-1, 1]^3.
  const auto b = sphereB<T>(); // Box [2, 4] x [-1, 1]^2.
  const auto c = sphereC<T>(); // Box [-2, 2]^3.
  const auto d = sphereD<T>(); // Box [-1, 3] x [-2, 2]^2.

  const auto requireBox = [](const BV<T>& a_box, const Vec3& a_lo, const Vec3& a_hi) {
    for (size_t dir = 0; dir < 3; dir++) {
      REQUIRE_THAT(a_box.getLowCorner()[dir], withinAbsT(a_lo[dir], exactMargin<T>()));
      REQUIRE_THAT(a_box.getHighCorner()[dir], withinAbsT(a_hi[dir], exactMargin<T>()));
    }
  };

  // A union's box is the merge of its operands' boxes.
  requireBox(Union(a, b).computeBoundingVolume(), Vec3(-1, -1, -1), Vec3(4, 1, 1));
  requireBox(Union(std::vector<Sphere<T>>{a, b, c}).computeBoundingVolume(), Vec3(-2, -2, -2), Vec3(4, 2, 2));

  // A smooth union moves the surface out by at most s/4 (polynomial) or s ln 2 (exponential).
  const T s = T(0.4);
  const T g = s * std::log(T(2));

  requireBox(SmoothUnion(a, b, s).computeBoundingVolume(), Vec3(-1.1, -1.1, -1.1), Vec3(4.1, 1.1, 1.1));
  requireBox(SmoothUnion(a, b, s, BlendKind::Exponential).computeBoundingVolume(),
             Vec3(-1 - g, -1 - g, -1 - g),
             Vec3(4 + g, 1 + g, 1 + g));

  // An intersection's box is the overlap of its operands' boxes.
  requireBox(Intersection(c, d).computeBoundingVolume(), Vec3(-1, -2, -2), Vec3(2, 2, 2));
  requireBox(SmoothIntersection(c, d, s).computeBoundingVolume(), Vec3(-1, -2, -2), Vec3(2, 2, 2));

  // A difference lies inside the object subtracted from.
  requireBox(Difference(c, d).computeBoundingVolume(), Vec3(-2, -2, -2), Vec3(2, 2, 2));
  requireBox(Difference(c, std::vector<Sphere<T>>{a, d}).computeBoundingVolume(), Vec3(-2, -2, -2), Vec3(2, 2, 2));
  requireBox(SmoothDifference(c, d, s).computeBoundingVolume(), Vec3(-2, -2, -2), Vec3(2, 2, 2));

  // The compiled tape reports the expression's box.
  const auto        diff = Difference(Union(a, b), c);
  const Compiled<T> tape(diff);

  requireBox(tape.tape().computeBoundingVolume(),
             diff.computeBoundingVolume().getLowCorner(),
             diff.computeBoundingVolume().getHighCorner());
  requireBox(diff.computeBoundingVolume(), Vec3(-1, -1, -1), Vec3(4, 1, 1));
}

TEMPLATE_TEST_CASE("Expr: distance quality of a union of exact shapes and of a finite repetition",
                   "[CSG][Union][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();

  STATIC_REQUIRE(Sphere<T>::distanceQuality == DistanceQuality::Exact);

  // A union of exact distances is a distance bound: exact outside, an underestimate inside where the
  // operands overlap.
  const auto u = Union(a, b);

  REQUIRE(Expr<T>(a).getDistanceQuality() == DistanceQuality::Exact);
  REQUIRE(u.getDistanceQuality() == DistanceQuality::Bound);
  REQUIRE(Intersection(sphereC<T>(), sphereD<T>()).getDistanceQuality() == DistanceQuality::Bound);
  REQUIRE(Difference(a, b).getDistanceQuality() == DistanceQuality::Bound);

  const Compiled<T> compiled(u);

  REQUIRE(compiled.tape().getDistanceQuality() == DistanceQuality::Bound);

  // A unit sphere fits a tile of period 2.5 but not one of period 1.5, where a neighbouring copy can be
  // closer than the folded value says.
  const auto fits     = FiniteRepetition(a, Vec3(2.5, 2.5, 2.5), Vec3::ones(), Vec3::ones());
  const auto overlaps = FiniteRepetition(a, Vec3(1.5, 2.5, 2.5), Vec3::ones(), Vec3::ones());

  REQUIRE(fits.getDistanceQuality() == DistanceQuality::Bound);
  REQUIRE(overlaps.getDistanceQuality() == DistanceQuality::NotADistance);

  const Compiled<T> compiledOverlaps(overlaps);

  REQUIRE(compiledOverlaps.tape().getDistanceQuality() == DistanceQuality::NotADistance);
}

// ─────────────────────────────────────────────────────────────────────────────
// Intersection()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Intersection: value is the pointwise maximum, giving a non-trivial overlap region",
                   "[CSG][Intersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const Compiled<T> inter(Intersection(c, d));

  // Inside both spheres.
  const Vec3 insideBoth(0.5, 0, 0);
  REQUIRE_THAT(inter(insideBoth),
               withinAbsT(std::max(c.signedDistance(insideBoth), d.signedDistance(insideBoth)), exactMargin<T>()));
  REQUIRE(inter(insideBoth) < T(0.0));

  // Inside D only (outside C): must not be in the intersection.
  const Vec3 insideDOnly(2.5, 0, 0);
  REQUIRE(c.signedDistance(insideDOnly) > T(0.0));
  REQUIRE(d.signedDistance(insideDOnly) < T(0.0));
  REQUIRE(inter(insideDOnly) > T(0.0));
}

TEMPLATE_TEST_CASE("Intersection: the pairwise and vector overloads agree with the hand-computed maximum",
                   "[CSG][Intersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();
  const auto e = sphereE<T>();

  const Compiled<T> twoArg(Intersection(c, d));
  const Compiled<T> vectorArg(Intersection(std::vector<Sphere<T>>{c, d}));
  const Compiled<T> three(Intersection(std::vector<Sphere<T>>{c, d, e}));

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(2.5, 0, 0), Vec3(0.6, 0.1, 0), Vec3(-5, 3, 1)}) {
    const T maxCD = std::max(c.signedDistance(p), d.signedDistance(p));

    REQUIRE_THAT(twoArg(p), withinAbsT(maxCD, exactMargin<T>()));
    REQUIRE_THAT(vectorArg(p), withinAbsT(twoArg(p), exactMargin<T>()));
    REQUIRE_THAT(three(p), withinAbsT(std::max(maxCD, e.signedDistance(p)), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// SmoothIntersection()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SmoothIntersection: converges to the sharp intersection far from the blend region",
                   "[CSG][SmoothIntersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const T           smoothLen = T(0.05);
  const Compiled<T> smooth(SmoothIntersection(c, d, smoothLen));
  const Compiled<T> sharp(Intersection(c, d));

  // Deep inside D only, far from C's surface (|C - D| there is >> smoothLen).
  const Vec3 deepOutsideC(4.5, 0, 0);
  REQUIRE_THAT(smooth(deepOutsideC), withinAbsT(sharp(deepOutsideC), asymptoticMargin<T>()));
}

TEMPLATE_TEST_CASE("SmoothIntersection: never shallower (never below) the sharp intersection",
                   "[CSG][SmoothIntersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  for (const BlendKind kind : {BlendKind::Polynomial, BlendKind::Exponential}) {
    const Compiled<T> smooth(SmoothIntersection(c, d, T(0.75), kind));
    const Compiled<T> sharp(Intersection(c, d));

    for (const T x : sweepValues<T>(T(-3.0), T(4.0), 28)) {
      const Vec3 p(x, 0, 0);
      REQUIRE(smooth(p) >= sharp(p) - T(exactMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("SmoothIntersection: the pairwise and vector overloads agree with the hand-computed blend",
                   "[CSG][SmoothIntersection]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();
  const auto e = sphereE<T>();

  const T smoothLen = T(0.4);

  const Compiled<T> pairwise(SmoothIntersection(c, d, smoothLen));
  const Compiled<T> vectorArg(SmoothIntersection(std::vector<Sphere<T>>{c, d}, smoothLen));
  const Compiled<T> three(SmoothIntersection(std::vector<Sphere<T>>{c, d, e}, smoothLen));
  const Compiled<T> exponential(SmoothIntersection(c, d, smoothLen, BlendKind::Exponential));

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(2.5, 0, 0), Vec3(0.6, 0.1, 0), Vec3(-1.9, 0.2, 0), Vec3(-5, 3, 1)}) {
    const T dc = c.signedDistance(p);
    const T dd = d.signedDistance(p);
    const T de = e.signedDistance(p);

    REQUIRE_THAT(pairwise(p), withinAbsT(SmoothMaxOp<T>{}(dc, dd, smoothLen), exactMargin<T>()));
    REQUIRE_THAT(vectorArg(p), withinAbsT(pairwise(p), exactMargin<T>()));
    REQUIRE_THAT(exponential(p), withinAbsT(ExpMaxOp<T>{}(dc, dd, smoothLen), formulaMargin<T>()));

    // Only the two largest values are blended.
    REQUIRE_THAT(three(p), withinAbsT(blendTwoLargest<T>({dc, dd, de}, smoothLen, SmoothMaxOp<T>{}), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Difference()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Difference: A \\ B is max(A, -B), inside A and outside B only",
                   "[CSG][Difference]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto c = sphereC<T>();
  const auto d = sphereD<T>();

  const Compiled<T> diff(Difference(c, d));

  // Inside C, but also inside D -- must be excluded from C \ D.
  const Vec3 insideBoth(0.5, 0, 0);
  REQUIRE(diff(insideBoth) > T(0.0));
  REQUIRE_THAT(diff(insideBoth),
               withinAbsT(std::max(c.signedDistance(insideBoth), -d.signedDistance(insideBoth)), exactMargin<T>()));

  // Inside C, outside D.
  const Vec3 insideCOnly(-1.5, 0, 0);
  REQUIRE(c.signedDistance(insideCOnly) < T(0.0));
  REQUIRE(d.signedDistance(insideCOnly) > T(0.0));
  REQUIRE(diff(insideCOnly) < T(0.0));
}

TEMPLATE_TEST_CASE("Difference: A minus a list of subtrahends excludes the union of all of them",
                   "[CSG][Difference]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>(); // Center (0,0,0), radius 1: fully contains the origin.
  const auto b = sphereB<T>(); // Center (3,0,0), radius 1: doesn't overlap A at all.
  const auto e = sphereE<T>(); // A small sphere carved out of A.

  const Compiled<T> diff(Difference(a, std::vector<Sphere<T>>{b, e}));

  REQUIRE(diff(Vec3::zeros()) < T(0.0));   // Inside A, outside both B and E.
  REQUIRE(diff(Vec3(0.5, 0, 0)) > T(0.0)); // Inside A and inside E: excluded.
  REQUIRE(diff(Vec3(3, 0, 0)) > T(0.0));   // Not inside A at all.
}

TEMPLATE_TEST_CASE("Difference: the pairwise and vector overloads agree with max(A, -min B)",
                   "[CSG][Difference]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();
  const auto c = sphereC<T>();
  const auto d = sphereD<T>();
  const auto e = sphereE<T>();

  const Compiled<T> pairwise(Difference(c, d));
  const Compiled<T> vectorArg(Difference(c, std::vector<Sphere<T>>{d}));
  const Compiled<T> twoSubtrahends(Difference(a, std::vector<Sphere<T>>{b, e}));

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(-1.5, 0, 0), Vec3(3, 0, 0), Vec3(0.1, 0.2, 0), Vec3(-4, 1, 2)}) {
    const T minBE = std::min(b.signedDistance(p), e.signedDistance(p));

    REQUIRE_THAT(pairwise(p), withinAbsT(std::max(c.signedDistance(p), -d.signedDistance(p)), exactMargin<T>()));
    REQUIRE_THAT(vectorArg(p), withinAbsT(pairwise(p), exactMargin<T>()));
    REQUIRE_THAT(twoSubtrahends(p), withinAbsT(std::max(a.signedDistance(p), -minBE), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// SmoothDifference()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SmoothDifference: converges to the sharp difference far from the blend region",
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

  const T           smoothLen = T(0.05);
  const Compiled<T> smooth(SmoothDifference(a, b, smoothLen));
  const Compiled<T> sharp(Difference(a, b));

  const Vec3 insideAOnly = Vec3::zeros();
  REQUIRE_THAT(smooth(insideAOnly), withinAbsT(sharp(insideAOnly), asymptoticMargin<T>()));
}

TEMPLATE_TEST_CASE("SmoothDifference: the pairwise and vector overloads agree with the hand-computed blend",
                   "[CSG][SmoothDifference]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto a = sphereA<T>();
  const auto b = sphereB<T>();
  const auto c = sphereC<T>();
  const auto d = sphereD<T>();
  const auto e = sphereE<T>();

  const T smoothLen = T(0.4);

  const Compiled<T> pairwise(SmoothDifference(c, d, smoothLen));
  const Compiled<T> vectorArg(SmoothDifference(c, std::vector<Sphere<T>>{d}, smoothLen));
  const Compiled<T> twoSubtrahends(SmoothDifference(a, std::vector<Sphere<T>>{b, e}, smoothLen));
  const Compiled<T> exponential(SmoothDifference(c, d, smoothLen, BlendKind::Exponential));

  for (const Vec3 p : {Vec3(0.5, 0, 0), Vec3(-1.5, 0, 0), Vec3(3, 0, 0), Vec3(0.1, 0.2, 0), Vec3(0.75, 0, 0)}) {
    const T da = a.signedDistance(p);
    const T db = b.signedDistance(p);
    const T dc = c.signedDistance(p);
    const T dd = d.signedDistance(p);
    const T de = e.signedDistance(p);

    // The smooth intersection of A and the complement of each subtrahend.
    REQUIRE_THAT(pairwise(p), withinAbsT(SmoothMaxOp<T>{}(dc, -dd, smoothLen), exactMargin<T>()));
    REQUIRE_THAT(vectorArg(p), withinAbsT(pairwise(p), exactMargin<T>()));
    REQUIRE_THAT(exponential(p), withinAbsT(ExpMaxOp<T>{}(dc, -dd, smoothLen), formulaMargin<T>()));
    REQUIRE_THAT(twoSubtrahends(p),
                 withinAbsT(blendTwoLargest<T>({da, -db, -de}, smoothLen, SmoothMaxOp<T>{}), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// FiniteRepetition()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("FiniteRepetition: matches the base function directly within the home tile",
                   "[CSG][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T> base(Vec3::zeros(), T(1));

  const Vec3 period(10, 10, 10);
  const Vec3 repeatLo(2, 2, 2);
  const Vec3 repeatHi(2, 2, 2);

  const Compiled<T> tiled(FiniteRepetition(base, period, repeatLo, repeatHi));

  for (const Vec3 p : {Vec3::zeros(), Vec3(0.5, 0, 0), Vec3(4.9, 0, 0), Vec3(-4.9, 0, 0)}) {
    REQUIRE_THAT(tiled(p), withinAbsT(base.signedDistance(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("FiniteRepetition: is periodic within the repetition range",
                   "[CSG][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T> base(Vec3::zeros(), T(1));

  const Vec3 period(5, 5, 5);
  const Vec3 repeatLo(3, 0, 0);
  const Vec3 repeatHi(3, 0, 0);

  const Compiled<T> tiled(FiniteRepetition(base, period, repeatLo, repeatHi));

  // A point near the origin and the same point shifted by exactly N whole periods (N within the
  // repetition range) must evaluate to the same value.
  for (int n = -3; n <= 3; n++) {
    const Vec3 p(0.3, 0, 0);
    const Vec3 shifted = p + Vec3(5.0 * n, 0, 0);
    REQUIRE_THAT(tiled(shifted), withinAbsT(tiled(p), formulaMargin<T>()));
    REQUIRE_THAT(tiled(shifted), withinAbsT(base.signedDistance(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("FiniteRepetition: clamps to the boundary tile beyond the repetition range",
                   "[CSG][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T> base(Vec3::zeros(), T(1));

  const Vec3 period(5, 5, 5);
  const Vec3 repeatLo(1, 0, 0);
  const Vec3 repeatHi(1, 0, 0);

  const Compiled<T> tiled(FiniteRepetition(base, period, repeatLo, repeatHi));

  // Well beyond the +1 tile: folds relative to the tile-1 anchor (x - 1*period), not back toward
  // tile 0 and not extrapolated as if more tiles existed.
  const Vec3 farBeyond(23.0, 0, 0);
  const Vec3 expectedLocal = farBeyond - Vec3(5.0, 0, 0);
  REQUIRE_THAT(tiled(farBeyond), withinAbsT(base.signedDistance(expectedLocal), formulaMargin<T>()));

  // Symmetric check on the negative side.
  const Vec3 farBelow(-23.0, 0, 0);
  const Vec3 expectedLocalNeg = farBelow + Vec3(5.0, 0, 0);
  REQUIRE_THAT(tiled(farBelow), withinAbsT(base.signedDistance(expectedLocalNeg), formulaMargin<T>()));
}

TEMPLATE_TEST_CASE("FiniteRepetition: fractional repetition counts round to whole tiles",
                   "[CSG][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T> base(Vec3::zeros(), T(0.25));

  const Compiled<T> roundsDown(
    FiniteRepetition(base, Vec3::ones(), Vec3(T(0.4), T(0), T(0)), Vec3(T(2.4), T(0), T(0))));
  const Compiled<T> roundsUp(FiniteRepetition(base, Vec3::ones(), Vec3(T(0.6), T(0), T(0)), Vec3(T(2.6), T(0), T(0))));
  const Compiled<T> two(FiniteRepetition(base, Vec3::ones(), Vec3(T(0), T(0), T(0)), Vec3(T(2), T(0), T(0))));
  const Compiled<T> three(FiniteRepetition(base, Vec3::ones(), Vec3(T(1), T(0), T(0)), Vec3(T(3), T(0), T(0))));

  for (const T x : {T(-2), T(-1), T(0), T(1), T(2), T(2.6), T(3), T(4)}) {
    const Vec3 p(x, T(0.1), T(0));

    INFO("x = " << x);
    REQUIRE_THAT(roundsDown(p), withinAbsT(two(p), exactMargin<T>()));
    REQUIRE_THAT(roundsUp(p), withinAbsT(three(p), exactMargin<T>()));
  }

  REQUIRE(two(Vec3(T(3), T(0), T(0))) > T(0));
  REQUIRE(three(Vec3(T(3), T(0), T(0))) < T(0));
}

TEMPLATE_TEST_CASE("FiniteRepetition: matches the base function at the hand-folded point",
                   "[CSG][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T> base(Vec3(T(0.2), T(-0.1), T(0)), T(1));

  const Vec3 period(5, 4, 3);
  const Vec3 repeatLo(2, 1, 0);
  const Vec3 repeatHi(1, 2, 3);

  const Compiled<T> tiled(FiniteRepetition(base, period, repeatLo, repeatHi));

  // p_i - c_i round(clamp(p_i / c_i, -lo_i, hi_i)).
  const auto fold = [&](const Vec3& a_point) {
    Vec3 local;

    for (size_t dir = 0; dir < 3; dir++) {
      const T tile = std::round(std::clamp(a_point[dir] / period[dir], -repeatLo[dir], repeatHi[dir]));

      local[dir] = a_point[dir] - period[dir] * tile;
    }

    return local;
  };

  for (const Vec3 p : {Vec3::zeros(), Vec3(7.3, -2.1, 0.4), Vec3(-12.0, 0, 0), Vec3(4.1, 9.5, -2.2), Vec3(1, 1, 8.9)}) {
    INFO("p = " << p);
    REQUIRE_THAT(tiled(p), withinAbsT(base.signedDistance(fold(p)), formulaMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Argument checks: EBGEOMETRY_REQUIREs, so these abort in every build.
// ─────────────────────────────────────────────────────────────────────────────

namespace {

using SphereList = std::vector<SphereSDF<double>>;

} // namespace

TEST_CASE("Union: rejects an empty list", "[CSG][Union]")
{
  REQUIRE(abortsWith([] { (void)Union(SphereList{}); }, "Union: the list of operands must not be empty"));
}

TEST_CASE("SmoothUnion: rejects an empty list and a non-positive or non-finite smoothing length", "[CSG][SmoothUnion]")
{
  REQUIRE(
    abortsWith([] { (void)SmoothUnion(SphereList{}, 0.1); }, "SmoothUnion: the list of operands must not be empty"));
  REQUIRE(abortsWith([] { (void)SmoothUnion(sphereA<double>(), sphereB<double>(), 0.0); },
                     "SmoothUnion: the smoothing length must be positive and finite (0)"));
  REQUIRE(abortsWith([] { (void)SmoothUnion(SphereList{sphereA<double>()}, -1.0); },
                     "SmoothUnion: the smoothing length must be positive and finite (-1)"));
  REQUIRE(
    abortsWith([] { (void)SmoothUnion(sphereA<double>(), sphereB<double>(), std::numeric_limits<double>::infinity()); },
               "SmoothUnion: the smoothing length must be positive and finite (inf)"));
}

TEST_CASE("BVHUnion and BVHSmoothUnion: reject an unknown build strategy and a non-positive smoothing length",
          "[CSG][BVHUnion][BVHSmoothUnion]")
{
  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());

      (void)BVHUnion<double, SphereSDF<double>, 4>(
        pool, sphereRow<double>(), sphereRowBVs<double>(), static_cast<BVH::Construction>(42));
    },
    "BVHUnion: unknown BVH::Construction value (42)"));

  REQUIRE(abortsWith(
    [] {
      Pool pool(hostMemoryResource());

      (void)BVHSmoothUnion<double, SphereSDF<double>, 4>(pool, sphereRow<double>(), sphereRowBVs<double>(), 0.0);
    },
    "BVHSmoothUnion: the smoothing length must be positive (0)"));
}

TEST_CASE("Intersection: rejects an empty list", "[CSG][Intersection]")
{
  REQUIRE(abortsWith([] { (void)Intersection(SphereList{}); }, "Intersection: the list of operands must not be empty"));
}

TEST_CASE("SmoothIntersection: rejects an empty list and a non-positive smoothing length", "[CSG][SmoothIntersection]")
{
  REQUIRE(abortsWith([] { (void)SmoothIntersection(SphereList{}, 0.1); },
                     "SmoothIntersection: the list of operands must not be empty"));
  REQUIRE(abortsWith([] { (void)SmoothIntersection(SphereList{sphereC<double>(), sphereD<double>()}, 0.0); },
                     "SmoothIntersection: the smoothing length must be positive and finite (0)"));
  REQUIRE(abortsWith([] { (void)SmoothIntersection(sphereC<double>(), sphereD<double>(), -0.5); },
                     "SmoothIntersection: the smoothing length must be positive and finite (-0.5)"));
}

TEST_CASE("Difference: rejects an empty subtrahend list", "[CSG][Difference]")
{
  REQUIRE(abortsWith([] { (void)Difference(sphereC<double>(), SphereList{}); },
                     "Difference: the list of objects to subtract must not be empty"));
}

TEST_CASE("SmoothDifference: rejects an empty subtrahend list and a non-positive smoothing length",
          "[CSG][SmoothDifference]")
{
  REQUIRE(abortsWith([] { (void)SmoothDifference(sphereC<double>(), SphereList{}, 0.1); },
                     "SmoothDifference: the list of objects to subtract must not be empty"));
  REQUIRE(abortsWith([] { (void)SmoothDifference(sphereC<double>(), sphereD<double>(), 0.0); },
                     "SmoothDifference: the smoothing length must be positive and finite (0)"));
  REQUIRE(abortsWith([] { (void)SmoothDifference(sphereC<double>(), SphereList{sphereD<double>()}, -2.0); },
                     "SmoothDifference: the smoothing length must be positive and finite (-2)"));
}

TEST_CASE("FiniteRepetition: rejects a non-positive period and negative repetition counts", "[CSG][FiniteRepetition]")
{
  using Vec3 = Vec3T<double>;

  REQUIRE(abortsWith([] { (void)FiniteRepetition(sphereA<double>(), Vec3(1, 0, 1), Vec3::ones(), Vec3::ones()); },
                     "FiniteRepetition: the period must be positive and finite in every direction (1, 0, 1)"));
  REQUIRE(abortsWith([] { (void)FiniteRepetition(sphereA<double>(), Vec3::ones(), Vec3(0, -1, 0), Vec3::ones()); },
                     "FiniteRepetition: the low repetition counts must be finite and not negative (0, -1, 0)"));
  REQUIRE(abortsWith([] { (void)FiniteRepetition(sphereA<double>(), Vec3::ones(), Vec3::ones(), Vec3(-2, 0, 0)); },
                     "FiniteRepetition: the high repetition counts must be finite and not negative (-2, 0, 0)"));
}
