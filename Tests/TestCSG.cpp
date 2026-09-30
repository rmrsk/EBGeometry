// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_CSG.hpp: the sharp and smoothly-blended CSG combinators (union,
// intersection, difference), their BVH-accelerated counterparts, finite periodic repetition, and
// the SmoothMin/SmoothMax/ExpMin blending primitives they're built on. Uses analytic spheres as
// fixtures so every expected value is hand-computable.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestShapeIF.hpp"

#include <cmath>
#include <limits>
#include <memory>
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
using Sphere = TestUtils::SphereIF<T>;

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

TEMPLATE_TEST_CASE("ExpMin and ExpMax: finite and close to the sharp min/max far from the blend region",
                   "[CSG][ExpMin]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // exp(-a/s) under- or overflows once |a|/s passes ~100 (float) or ~700 (double); the blend must
  // not, since a BVHSmoothUnionIF evaluates it at arbitrary distances from the surface.
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

// The same row as ImplicitFunction objects, for the virtual UnionIF/SmoothUnionIF references.
template <class T>
std::vector<std::shared_ptr<IF<T>>>
sphereRowIF()
{
  std::vector<std::shared_ptr<IF<T>>> spheres;

  spheres.reserve(NumRowSpheres);

  for (int i = 0; i < NumRowSpheres; i++) {
    spheres.push_back(std::make_shared<Sphere<T>>(rowCenter<T>(i), T(1)));
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

// Brute-force smooth union: blend the two smallest sphere values found by a full linear scan instead
// of the pruned BVH traversal.
template <class T, class Blend>
T
bruteTwoNearest(const std::vector<SphereSDF<T>>& a_spheres, const Vec3T<T>& a_point, const T a_smoothLen, Blend a_blend)
{
  T a = std::numeric_limits<T>::infinity();
  T b = std::numeric_limits<T>::infinity();

  for (const auto& sphere : a_spheres) {
    const T d = sphere.signedDistance(a_point);

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

using TestMeta = DCEL::DefaultMetaData;

template <class T>
using TestTriMesh = TriMeshSDF<T, TestMeta, 4, 4>;

// A 3x2 grid of dodecahedra (circumradius ~1.4), each translated before its TriMeshSDF is built, all
// in a_pool. This is how a union of several copies of one mesh is built without a Translate wrapper.
template <class T>
std::vector<TestTriMesh<T>>
dodecahedronGrid(Pool& a_pool)
{
  const auto triangles =
    Parser::readIntoTriangles<T, TestMeta>(std::string(EBGEOMETRY_TEST_DATA_DIR) + "/dodecahedron.obj", a_pool);

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

      meshes.emplace_back(shifted, a_pool, BVH::Build::SAH, 1);
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

TEMPLATE_TEST_CASE("BVHUnionIF: a plain, trivially copyable value type", "[CSG][BVHUnion]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  STATIC_REQUIRE(std::is_trivially_copyable_v<BVHUnionIF<T, SphereSDF<T>, 4>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<BVHSmoothUnionIF<T, SphereSDF<T>, 4>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<BVHSmoothUnionIF<T, SphereSDF<T>, 4, ExpMinOp<T>>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<BVHUnionIF<T, TestTriMesh<T>, 4>>);
  STATIC_REQUIRE_FALSE(std::is_polymorphic_v<BVHUnionIF<T, SphereSDF<T>, 4>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<SmoothMinOp<T>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<SmoothMaxOp<T>>);
  STATIC_REQUIRE(std::is_trivially_copyable_v<ExpMinOp<T>>);
}

TEMPLATE_TEST_CASE("BVHUnionIF: agrees with the sharp UnionIF over a row of spheres",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const BVHUnionIF<T, SphereSDF<T>, 4> bvhUnion(pool, sphereRow<T>(), sphereRowBVs<T>());
  const UnionIF<T>                     sharpUnion(sphereRowIF<T>());

  for (const auto& p : lineQueryPoints<T>()) {
    REQUIRE_THAT(bvhUnion.signedDistance(p), withinAbsT(sharpUnion.value(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHUnionIF: every build strategy, and the free function, give the same union",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr size_t K = 4;

  Pool pool(hostMemoryResource());

  const auto spheres = sphereRow<T>();
  const auto bvs     = sphereRowBVs<T>();

  const auto freeFunc = BVHUnion<T, SphereSDF<T>, K>(pool, spheres, bvs);

  for (const auto build : {BVH::Build::TopDown, BVH::Build::SAH, BVH::Build::Morton, BVH::Build::Nested}) {
    const BVHUnionIF<T, SphereSDF<T>, K> bvhUnion(pool, spheres, bvs, build);

    for (const auto& p : lineQueryPoints<T>()) {
      REQUIRE_THAT(bvhUnion.signedDistance(p), withinAbsT(freeFunc.signedDistance(p), exactMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("BVHSmoothUnionIF: agrees with SmoothUnionIF far from any blend region",
                   "[CSG][BVHSmoothUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const T smoothLen = T(0.05); // Small relative to the 1-unit gap between spheres.

  Pool pool(hostMemoryResource());

  const BVHSmoothUnionIF<T, SphereSDF<T>, 4> bvhSmooth(pool, sphereRow<T>(), sphereRowBVs<T>(), smoothLen);
  const SmoothUnionIF<T>                     sharpSmooth(sphereRowIF<T>(), smoothLen);

  // Deep inside any one sphere, far from every other sphere's surface.
  for (int i = 0; i < NumRowSpheres; i++) {
    const Vec3T<T> deepInside = rowCenter<T>(i);

    REQUIRE_THAT(bvhSmooth.signedDistance(deepInside),
                 withinAbsT(sharpSmooth.value(deepInside), asymptoticMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHSmoothUnionIF: matches a brute-force two-nearest blend inside the blend region",
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
  const BVHSmoothUnionIF<T, SphereSDF<T>, 4>              polySmooth(pool, spheres, bvs, smoothLen);
  const BVHSmoothUnionIF<T, SphereSDF<T>, 4, ExpMinOp<T>> expSmooth(pool, spheres, bvs, smoothLen);
  const auto freeFunc = BVHSmoothUnion<T, SphereSDF<T>, 4>(pool, spheres, bvs, smoothLen);

  for (const auto& p : lineQueryPoints<T>()) {
    REQUIRE_THAT(polySmooth.signedDistance(p),
                 withinAbsT(bruteTwoNearest(spheres, p, smoothLen, SmoothMinOp<T>{}), exactMargin<T>()));
    REQUIRE_THAT(expSmooth.signedDistance(p),
                 withinAbsT(bruteTwoNearest(spheres, p, smoothLen, ExpMinOp<T>{}), formulaMargin<T>()));
    REQUIRE_THAT(freeFunc.signedDistance(p), withinAbsT(polySmooth.signedDistance(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHSmoothUnionIF: every build strategy matches brute force when the leaf count is not a "
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

  for (const auto build : {BVH::Build::TopDown, BVH::Build::SAH, BVH::Build::Morton, BVH::Build::Nested}) {
    const BVHSmoothUnionIF<T, SphereSDF<T>, K> smooth(pool, spheres, bvs, smoothLen, SmoothMinOp<T>{}, build);

    for (const auto& p : queries) {
      REQUIRE_THAT(smooth.signedDistance(p),
                   withinAbsT(bruteTwoNearest(spheres, p, smoothLen, SmoothMinOp<T>{}), formulaMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("BVHUnionIF: every build strategy handles many coincident primitives",
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

  for (const auto build : {BVH::Build::TopDown, BVH::Build::SAH, BVH::Build::Morton, BVH::Build::Nested}) {
    const BVHUnionIF<T, SphereSDF<T>, 4> bvhUnion(pool, spheres, bvs, build);

    REQUIRE_THAT(bvhUnion.signedDistance(Vec3T<T>::zeros()), withinAbsT(std::sqrt(T(3)) - T(0.5), formulaMargin<T>()));
    REQUIRE_THAT(bvhUnion.signedDistance(T(5) * Vec3T<T>::ones()), withinAbsT(T(-0.5), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("BVHUnionIF::computeBoundingVolume encloses every input sphere",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const auto bvs = sphereRowBVs<T>();

  const BVHUnionIF<T, SphereSDF<T>, 4> bvhUnion(pool, sphereRow<T>(), bvs);
  const BV<T>                          rootBV = bvhUnion.computeBoundingVolume();

  for (const auto& bv : bvs) {
    for (size_t dir = 0; dir < 3; dir++) {
      REQUIRE(rootBV.getLowCorner()[dir] <= bv.getLowCorner()[dir] + T(exactMargin<T>()));
      REQUIRE(rootBV.getHighCorner()[dir] >= bv.getHighCorner()[dir] - T(exactMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("BVHUnionIF: a union of translated TriMeshSDFs, and a union of that union, match brute force",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Union  = BVHUnionIF<T, TestTriMesh<T>, 4>;
  using Nested = BVHUnionIF<T, Union, 4>;

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

TEMPLATE_TEST_CASE("BVHUnionIF: host-mirror and deep copies of a TriMeshSDF union outlive the source pool",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Union  = BVHUnionIF<T, TestTriMesh<T>, 4>;
  using Nested = BVHUnionIF<T, Union, 4>;

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

  // The source pool is gone. The meshes stored inside the union were copied byte for byte, host
  // control block included, so each view resolves them against its own pool only because the union
  // relocates them as it evaluates them; without that, these reads would be use-after-free.
  REQUIRE(deepCopied->isAttachedTo(copyPool));

  for (size_t i = 0; i < queries.size(); i++) {
    REQUIRE(mirrorView->signedDistance(queries[i]) == expected[i]);
    REQUIRE(nestedMirrorView->signedDistance(queries[i]) == expected[i]);
    REQUIRE(deepCopied->signedDistance(queries[i]) == expected[i]);
  }
}

#if defined(EBGEOMETRY_ENABLE_ASSERTIONS)
TEMPLATE_TEST_CASE("BVHUnionIF: rejects a mesh from another pool and a missing bounding volume",
                   "[CSG][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Union = BVHUnionIF<T, TestTriMesh<T>, 4>;

  // Both checks are always on, not EBGEOMETRY_EXPECTs; the helper only runs where assertions are
  // enabled, which is where this suite's death tests live.
  REQUIRE_FALSE(abortsUnderAssertions([] {
    Pool        pool(hostMemoryResource());
    const auto  meshes = dodecahedronGrid<T>(pool);
    const Union meshUnion(pool, meshes, boundingVolumes(meshes));

    (void)meshUnion;
  }));

  REQUIRE(abortsUnderAssertions([] {
    Pool        meshPool(hostMemoryResource());
    Pool        unionPool(hostMemoryResource());
    const auto  meshes = dodecahedronGrid<T>(meshPool);
    const Union meshUnion(unionPool, meshes, boundingVolumes(meshes));

    (void)meshUnion;
  }));

  REQUIRE(abortsUnderAssertions([] {
    Pool       pool(hostMemoryResource());
    const auto spheres = sphereRow<T>();
    auto       bvs     = sphereRowBVs<T>();

    bvs.pop_back();

    const BVHUnionIF<T, SphereSDF<T>, 4> sphereUnion(pool, spheres, bvs);

    (void)sphereUnion;
  }));
}
#endif

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

using Catch::Matchers::WithinRel;

namespace {

// Evaluates a sphere union, a smooth sphere union and a TriMeshSDF union on the device. Each arrives
// by value as a kernel argument.
template <class T>
EBGEOMETRY_GLOBAL
void
unionsDeviceKernel(const BVHUnionIF<T, SphereSDF<T>, 4>       a_sphereUnion,
                   const BVHSmoothUnionIF<T, SphereSDF<T>, 4> a_smoothUnion,
                   const BVHUnionIF<T, TestTriMesh<T>, 4>     a_meshUnion,
                   const Vec3T<T>                             a_point,
                   T*                                         a_out)
{
  a_out[0] = a_sphereUnion.signedDistance(a_point) + T(2) * a_smoothUnion.signedDistance(a_point) +
             T(3) * a_meshUnion.signedDistance(a_point);
}

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

  const BVHUnionIF<T, SphereSDF<T>, 4>       sphereUnion(pool, sphereRow<T>(), sphereRowBVs<T>());
  const BVHSmoothUnionIF<T, SphereSDF<T>, 4> smoothUnion(pool, sphereRow<T>(), sphereRowBVs<T>(), T(0.6));
  const BVHUnionIF<T, TestTriMesh<T>, 4>     meshUnion(pool, meshes, boundingVolumes(meshes));

  pool.freeze();

  Pool devicePool = Pool::mirror(pool, deviceMemoryResource());

  const auto sphereView = sphereUnion.rebasedView(devicePool);
  const auto smoothView = smoothUnion.rebasedView(devicePool);
  const auto meshView   = meshUnion.rebasedView(devicePool);

  for (const auto& p : gridQueryPoints<T>()) {
    const T hostVal =
      sphereUnion.signedDistance(p) + T(2) * smoothUnion.signedDistance(p) + T(3) * meshUnion.signedDistance(p);

    DeviceBuffer<T> deviceOut;

    unionsDeviceKernel<T><<<1, 1>>>(sphereView, smoothView, meshView, p, deviceOut.get());
    (void)GPU::deviceSynchronize();

    REQUIRE_THAT(readScalar(deviceOut.get()), WithinRel(hostVal, gpuTol<T>()));
  }
}

#endif

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

TEMPLATE_TEST_CASE("FiniteRepetitionIF: fractional repetition counts round to whole tiles",
                   "[CSG][FiniteRepetition]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto base = std::make_shared<Sphere<T>>(Vec3::zeros(), T(0.25));

  const FiniteRepetitionIF<T> roundsDown(base, Vec3::ones(), Vec3(T(0.4), T(0), T(0)), Vec3(T(2.4), T(0), T(0)));
  const FiniteRepetitionIF<T> roundsUp(base, Vec3::ones(), Vec3(T(0.6), T(0), T(0)), Vec3(T(2.6), T(0), T(0)));
  const FiniteRepetitionIF<T> two(base, Vec3::ones(), Vec3(T(0), T(0), T(0)), Vec3(T(2), T(0), T(0)));
  const FiniteRepetitionIF<T> three(base, Vec3::ones(), Vec3(T(1), T(0), T(0)), Vec3(T(3), T(0), T(0)));

  for (const T x : {T(-2), T(-1), T(0), T(1), T(2), T(2.6), T(3), T(4)}) {
    const Vec3 p(x, T(0.1), T(0));

    INFO("x = " << x);
    REQUIRE_THAT(roundsDown.value(p), withinAbsT(two.value(p), exactMargin<T>()));
    REQUIRE_THAT(roundsUp.value(p), withinAbsT(three.value(p), exactMargin<T>()));
  }

  REQUIRE(two.value(Vec3(T(3), T(0), T(0))) > T(0));
  REQUIRE(three.value(Vec3(T(3), T(0), T(0))) < T(0));
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
