// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_FunctionQueries.hpp: approximateBoundingVolumeOctree, against an
// independent recursive subdivision and on its failure cases, and the finite-difference normal(). Both
// take any function of a point: a shape, an ImplicitFunction or a lambda.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestShapeIF.hpp"

#include <cstdint>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

// A recursive subdivision written independently of the library's: a cell whose center value is
// within the safety-scaled half-diagonal is subdivided, and kept at the last level. Corners are
// computed from integer cell positions as the library does, so the boxes agree exactly.
template <class T, class F>
void
referenceOctree(const F&        a_function,
                const Vec3T<T>& a_lo,
                const Vec3T<T>& a_size,
                const uint32_t  a_index[3],
                const unsigned  a_level,
                const unsigned  a_maxDepth,
                const T         a_safety,
                Vec3T<T>&       a_boxLo,
                Vec3T<T>&       a_boxHi,
                bool&           a_found)
{
  const T width = T(1) / static_cast<T>(uint64_t(1) << a_level);

  Vec3T<T> lo;
  Vec3T<T> hi;

  for (int dir = 0; dir < 3; dir++) {
    lo[dir] = a_lo[dir] + a_size[dir] * (static_cast<T>(a_index[dir]) * width);
    hi[dir] = a_lo[dir] + a_size[dir] * (static_cast<T>(a_index[dir] + 1) * width);
  }

  const Vec3T<T> center = T(0.5) * (lo + hi);

  if (!(std::abs(a_function.signedDistance(center)) <= (T(1) + a_safety) * (T(0.5) * (hi - lo)).length())) {
    return;
  }

  if (a_level == a_maxDepth) {
    a_boxLo = min(a_boxLo, lo);
    a_boxHi = max(a_boxHi, hi);
    a_found = true;

    return;
  }

  for (uint32_t i = 0; i < 2; i++) {
    for (uint32_t j = 0; j < 2; j++) {
      for (uint32_t k = 0; k < 2; k++) {
        const uint32_t child[3] = {2 * a_index[0] + i, 2 * a_index[1] + j, 2 * a_index[2] + k};

        referenceOctree(a_function, a_lo, a_size, child, a_level + 1, a_maxDepth, a_safety, a_boxLo, a_boxHi, a_found);
      }
    }
  }
}

// Whether a box is the maximal one approximateBoundingVolumeOctree returns on failure.
template <class T>
bool
isMaximal(const BoundingVolumes::AABBT<T>& a_box)
{
  return a_box.getLowCorner() == -Vec3T<T>::max() && a_box.getHighCorner() == Vec3T<T>::max();
}

// Device-test functors: one result per query, as TestGPU.hpp's harness evaluates them.

// Component (query % 6) of the octree box of a sphere at depth (query / 6): low corner, then high.
template <class T>
struct OctreeBoxQuery
{
  SphereSDF<T> m_sphere;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const uint32_t& a_query) const noexcept
  {
    const auto box = approximateBoundingVolumeOctree(
      m_sphere, Vec3T<T>(T(-2), T(-2), T(-2)), Vec3T<T>(T(2), T(2), T(2)), a_query / 6, T(0.1));

    const uint32_t component = a_query % 6;

    return (component < 3) ? box.getLowCorner()[component] : box.getHighCorner()[component - 3];
  }
};

// Component m_axis of the sphere's finite-difference normal at each point.
template <class T>
struct NormalQuery
{
  SphereSDF<T> m_sphere;
  std::size_t  m_axis;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return normal(m_sphere, a_point, T(1.0e-3))[m_axis];
  }
};

} // namespace

TEMPLATE_TEST_CASE("approximateBoundingVolumeOctree: matches an independent recursive subdivision",
                   "[FunctionQueries]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3(T(0.3), T(-0.2), T(0.1)), T(0.9));
  const BoxSDF<T>    box(Vec3(T(-0.7), T(-0.4), T(-1.1)), Vec3(T(0.5), T(0.8), T(0.6)));
  const TorusSDF<T>  torus(Vec3(T(0.1), T(0.2), T(-0.1)), T(1.1), T(0.25));

  const Vec3 initLo(T(-2), T(-2.5), T(-2));
  const Vec3 initHi(T(2.5), T(2), T(2));

  const auto compare = [&](const auto& a_function, const char* a_name) {
    for (const unsigned depth : {0U, 1U, 3U, 5U, 6U}) {
      for (const T safety : {T(0), T(0.5)}) {
        INFO(a_name << ", depth " << depth << ", safety " << safety);

        Vec3           lo       = Vec3::infinity();
        Vec3           hi       = -Vec3::infinity();
        bool           found    = false;
        const uint32_t root[3]  = {0, 0, 0};
        const auto     estimate = approximateBoundingVolumeOctree(a_function, initLo, initHi, depth, safety);

        referenceOctree(a_function, initLo, initHi - initLo, root, 0, depth, safety, lo, hi, found);

        REQUIRE(found);
        CHECK(estimate.getLowCorner() == lo);
        CHECK(estimate.getHighCorner() == hi);
      }
    }
  };

  compare(sphere, "sphere");
  compare(box, "box");
  compare(torus, "torus");
}

TEMPLATE_TEST_CASE("approximateBoundingVolumeOctree: encloses a sphere within a margin that shrinks with depth",
                   "[FunctionQueries]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const T slop = T(1.0e4) * Math::Limits<T>::epsilon();

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  const Vec3 initLo = -2 * Vec3::ones();
  const Vec3 initHi = +2 * Vec3::ones();

  T previousMargin = Math::Limits<T>::max();

  for (const unsigned int depth : {2U, 4U, 6U, 8U}) {
    const auto bv = approximateBoundingVolumeOctree(sphere, initLo, initHi, depth, T(0.0));

    // Must actually enclose the true bounding box [-1, 1]^3 (never smaller than the real shape).
    for (int i = 0; i < 3; i++) {
      REQUIRE(bv.getLowCorner()[i] <= T(-1.0) + slop);
      REQUIRE(bv.getHighCorner()[i] >= T(1.0) - slop);
    }

    // The margin beyond the true bounds should shrink (or stay the same) as depth increases.
    const T margin = bv.getHighCorner()[0] - T(1.0);

    REQUIRE(margin <= previousMargin + slop);

    previousMargin = margin;
  }

  // At depth 8, the approximation should be tight to well within a tenth of the sphere's radius.
  REQUIRE(approximateBoundingVolumeOctree(sphere, initLo, initHi, 8U, T(0.0)).getHighCorner()[0] < T(1.1));

  // At depth 0 the initial box itself is the estimate.
  const auto coarse = approximateBoundingVolumeOctree(sphere, initLo, initHi, 0U, T(0.0));

  CHECK(coarse.getLowCorner() == initLo);
  CHECK(coarse.getHighCorner() == initHi);
}

TEMPLATE_TEST_CASE("approximateBoundingVolumeOctree: a larger safety factor never shrinks the sphere out of the result",
                   "[FunctionQueries]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const T slop = T(1.0e4) * Math::Limits<T>::epsilon();

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  for (const T safety : {T(0.0), T(0.25), T(1.0)}) {
    const auto bv = approximateBoundingVolumeOctree(sphere, -2 * Vec3::ones(), 2 * Vec3::ones(), 6U, safety);

    for (int i = 0; i < 3; i++) {
      REQUIRE(bv.getLowCorner()[i] <= T(-1.0) + slop);
      REQUIRE(bv.getHighCorner()[i] >= T(1.0) - slop);
    }
  }
}

TEMPLATE_TEST_CASE("approximateBoundingVolumeOctree: an empty or inverted box, or one that misses the surface, gives "
                   "the maximal box",
                   "[FunctionQueries]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> sphere(Vec3::zeros(), T(1));

  CHECK(isMaximal(approximateBoundingVolumeOctree(sphere, Vec3::ones(), Vec3::zeros(), 4U, T(0.0))));

  // Inverted or empty along one axis only.
  CHECK(
    isMaximal(approximateBoundingVolumeOctree(sphere, Vec3(T(-2), T(2), T(-2)), Vec3(T(2), T(-2), T(2)), 4U, T(0.0))));
  CHECK(
    isMaximal(approximateBoundingVolumeOctree(sphere, Vec3(T(-2), T(-2), T(1)), Vec3(T(2), T(2), T(1)), 4U, T(0.0))));

  // Far from the unit sphere, and small enough that it cannot reach it.
  CHECK(isMaximal(approximateBoundingVolumeOctree(sphere, Vec3(100, 100, 100), Vec3(101, 101, 101), 4U, T(0.0))));
}

TEMPLATE_TEST_CASE("approximateBoundingVolumeOctree: a shape, an ImplicitFunction and a lambda give the same box",
                   "[FunctionQueries]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T>           sphere(Vec3(T(0.25), T(-0.5), T(0.125)), T(1));
  const TestUtils::SphereIF<T> sphereIF(Vec3(T(0.25), T(-0.5), T(0.125)), T(1));
  const ImplicitFunction<T>&   implicitFunction = sphereIF;

  const Vec3 initLo = -2 * Vec3::ones();
  const Vec3 initHi = +2 * Vec3::ones();

  const auto fromShape  = approximateBoundingVolumeOctree(sphere, initLo, initHi, 6U, T(0.25));
  const auto fromFree   = approximateBoundingVolumeOctree(implicitFunction, initLo, initHi, 6U, T(0.25));
  const auto fromMember = implicitFunction.approximateBoundingVolumeOctree(initLo, initHi, 6U, T(0.25));
  const auto fromLambda = approximateBoundingVolumeOctree(
    [&sphere](const Vec3& a_point) { return sphere.signedDistance(a_point); }, initLo, initHi, 6U, T(0.25));

  // All four evaluate the same formula at the same points.
  for (const auto& bv : {fromFree, fromMember, fromLambda}) {
    CHECK(bv.getLowCorner() == fromShape.getLowCorner());
    CHECK(bv.getHighCorner() == fromShape.getHighCorner());
  }
}

TEST_CASE("approximateBoundingVolumeOctree: a depth beyond MaxOctreeDepth, or a negative safety factor, is rejected",
          "[FunctionQueries][death]")
{
  using T = double;

  REQUIRE(abortsWith(
    [] {
      const SphereSDF<T> sphere(Vec3T<T>::zeros(), T(1));

      (void)approximateBoundingVolumeOctree(sphere, -Vec3T<T>::ones(), Vec3T<T>::ones(), MaxOctreeDepth + 1, T(0));
    },
    "approximateBoundingVolumeOctree: the depth 25 exceeds MaxOctreeDepth (24)"));

  REQUIRE(abortsWith(
    [] {
      const SphereSDF<T> sphere(Vec3T<T>::zeros(), T(1));

      (void)approximateBoundingVolumeOctree(sphere, -Vec3T<T>::ones(), Vec3T<T>::ones(), 2U, T(-0.5));
    },
    "approximateBoundingVolumeOctree: the safety factor must be non-negative"));
}

TEMPLATE_TEST_CASE("normal: the finite-difference normal of a sphere points away from its center",
                   "[FunctionQueries]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3                   center(T(0.25), T(-0.5), T(0.125));
  const SphereSDF<T>           sphere(center, T(1));
  const TestUtils::SphereIF<T> sphereIF(center, T(1));
  const T                      delta = T(1.0e-3);

  // Central differences are off by O(delta^2) from the curvature, about 1e-7 here, and in float the
  // cancellation in (hi - lo) / (2 delta) costs about eps / delta.
  const double tolerance = std::is_same_v<T, float> ? 1.0e-3 : 1.0e-5;

  for (const Vec3& x : {Vec3(T(1.5), T(0.2), T(-0.3)), Vec3(T(-0.4), T(-1.9), T(0.6)), Vec3(T(0.3), T(-0.4), T(2.0))}) {
    const Vec3 expected = (x - center) / (x - center).length();
    const Vec3 n        = normal(sphere, x, delta);

    for (int dir = 0; dir < 3; dir++) {
      CHECK_THAT(n[dir], withinAbsT(expected[dir], tolerance));
    }

    CHECK_THAT(n.length(), withinAbsT(T(1), tightMargin<T>()));

    // Through value() and through a callable: the same evaluations, so the same result.
    CHECK(normal(sphereIF, x, delta) == n);
    CHECK(normal([&sphere](const Vec3& a_point) { return sphere.signedDistance(a_point); }, x, delta) == n);
  }
}

TEMPLATE_TEST_CASE("approximateBoundingVolumeOctree and normal: device results match the host",
                   "[FunctionQueries][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const SphereSDF<T> sphere(Vec3T<T>(T(0.3), T(-0.2), T(0.1)), T(0.9));

  // Depths 0 to 6, six box components each.
  std::vector<uint32_t> boxQueries;

  for (uint32_t q = 0; q < 7 * 6; q++) {
    boxQueries.push_back(q);
  }

  requireSameResults(evaluateOnDevice<T>(OctreeBoxQuery<T>{sphere}, boxQueries),
                     evaluateOnHost<T>(OctreeBoxQuery<T>{sphere}, boxQueries));

  const auto points = queryGrid<T>(Vec3T<T>(T(-1.5), T(-1.5), T(-1.5)), Vec3T<T>(T(1.5), T(1.5), T(1.5)), 7);

  for (std::size_t axis = 0; axis < 3; axis++) {
    INFO("axis " << axis);
    requireSameResults(evaluateOnDevice<T>(NormalQuery<T>{sphere, axis}, points),
                       evaluateOnHost<T>(NormalQuery<T>{sphere, axis}, points));
  }
}
