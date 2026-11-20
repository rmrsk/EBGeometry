// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// SampledGridSDF: a distance field sampled on a grid, and a height field, against closed forms where trilinear
// interpolation is exact (a plane, a sloping terrain), the bound property, the handle's views and copies, a tape
// holding one, and the device.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cmath>
#include <random>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinAbs;

namespace {

// Random points in a box.
template <class T>
std::vector<Vec3T<T>>
pointsIn(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi, const int a_count)
{
  std::mt19937                      rng(13579);
  std::uniform_real_distribution<T> unit(T(0), T(1));
  std::vector<Vec3T<T>>             points;

  for (int i = 0; i < a_count; i++) {
    points.emplace_back(a_lo + Vec3T<T>(unit(rng), unit(rng), unit(rng)) * (a_hi - a_lo));
  }

  return points;
}

// A field sampled at the nodes of a grid, x fastest.
template <class T, class F>
std::vector<T>
sample(const Vec3T<T>& a_origin, const Vec3T<T>& a_spacing, const uint32_t a_n[3], const F& a_field)
{
  std::vector<T> values;

  for (uint32_t k = 0; k < a_n[2]; k++) {
    for (uint32_t j = 0; j < a_n[1]; j++) {
      for (uint32_t i = 0; i < a_n[0]; i++) {
        values.push_back(a_field(a_origin + Vec3T<T>(T(i), T(j), T(k)) * a_spacing));
      }
    }
  }

  return values;
}

// One signed distance per point, for a grid held by value as a kernel receives it.
template <class T>
struct GridDistance
{
  SampledGridSDF<T> m_grid;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_grid.signedDistance(a_point);
  }
};

} // namespace

TEMPLATE_TEST_CASE("SampledGridSDF: a sampled plane is the plane inside the grid, and the grid cuts it",
                   "[SampledGridSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // A plane through the box, its values doubled: the grid divides them by the field's slope, 2.
  const Vec3        origin(T(-1), T(-1), T(-1));
  const Vec3        spacing(T(0.25), T(0.2), T(0.5));
  const uint32_t    n[3] = {9, 11, 5};
  const PlaneSDF<T> plane(Vec3(T(0.1), T(0), T(0)), Vec3(T(1), T(2), T(-1)));

  const SampledGridSDF<T> grid(
    pool, origin, spacing, n[0], n[1], n[2], sample<T>(origin, spacing, n, [&](const Vec3& a_p) {
      return T(2) * plane.signedDistance(a_p);
    }));

  const BoundingVolumes::AABBT<T> box = grid.computeBoundingVolume();

  REQUIRE(box.getLowCorner() == origin);
  REQUIRE_THAT(box.getHighCorner()[0], WithinAbs(1.0, double(tightMargin<T>())));
  REQUIRE_THAT(box.getHighCorner()[1], WithinAbs(1.0, double(tightMargin<T>())));
  REQUIRE_THAT(box.getHighCorner()[2], WithinAbs(1.0, double(tightMargin<T>())));

  // Inside the box, and farther from its faces than from the plane, the value is the plane's distance.
  const BoxSDF<T> boxShape(origin, Vec3::ones());

  for (const Vec3& p : pointsIn<T>(origin, Vec3::ones(), 500)) {
    const T d = plane.signedDistance(p);

    if (std::abs(d) < -boxShape.signedDistance(p)) {
      REQUIRE_THAT(grid.signedDistance(p), withinAbsT<T>(d, double(looseMargin<T>())));
    }

    // Everywhere, the value is the intersection of the plane and the box, or a bound on it.
    const T exact = Math::max(d, boxShape.signedDistance(p));

    REQUIRE(std::abs(grid.signedDistance(p)) <= std::abs(exact) + looseMargin<T>());
    REQUIRE((grid.signedDistance(p) < T(0)) == (exact < T(0)));
  }

  // Outside the box, at least the distance to the box, and positive.
  for (const Vec3& p : pointsIn<T>(T(-3) * Vec3::ones(), T(3) * Vec3::ones(), 500)) {
    if (boxShape.signedDistance(p) > T(0)) {
      REQUIRE(grid.signedDistance(p) >= boxShape.signedDistance(p) - looseMargin<T>());
    }
  }
}

TEMPLATE_TEST_CASE("SampledGridSDF: a sampled sphere changes no faster than the distance, and has its sign",
                   "[SampledGridSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Vec3         origin(T(-1), T(-1), T(-1));
  const Vec3         spacing(T(0.1), T(0.1), T(0.1));
  const uint32_t     n[3] = {21, 21, 21};
  const SphereSDF<T> sphere(Vec3(T(0.1), T(0), T(-0.1)), T(0.6));

  const SampledGridSDF<T> grid(
    pool, origin, spacing, n[0], n[1], n[2], sample<T>(origin, spacing, n, [&](const Vec3& a_p) {
      return sphere.signedDistance(a_p);
    }));

  const std::vector<Vec3> points = pointsIn<T>(T(-1.5) * Vec3::ones(), T(1.5) * Vec3::ones(), 2000);

  for (size_t i = 1; i < points.size(); i++) {
    REQUIRE(std::abs(grid.signedDistance(points[i]) - grid.signedDistance(points[i - 1])) <=
            (points[i] - points[i - 1]).length() + looseMargin<T>());

    // Away from the surface, the interpolated sphere has the sphere's sign.
    if (std::abs(sphere.signedDistance(points[i])) > T(0.05)) {
      REQUIRE((grid.signedDistance(points[i]) < T(0)) == (sphere.signedDistance(points[i]) < T(0)));
    }
  }
}

TEMPLATE_TEST_CASE("SampledGridSDF: a height field of a sloping plane is the distance to it above the floor",
                   "[SampledGridSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // h(x, z) = 0.5 + 0.3 x - 0.2 z over [0, 2] x [0, 3], on a floor at y = -1.
  const uint32_t nx = 9;
  const uint32_t nz = 13;
  const T        dx = T(0.25);
  const T        dz = T(0.25);

  std::vector<T> heights;

  for (uint32_t k = 0; k < nz; k++) {
    for (uint32_t i = 0; i < nx; i++) {
      heights.push_back(T(0.5) + T(0.3) * dx * T(i) - T(0.2) * dz * T(k));
    }
  }

  const SampledGridSDF<T> terrain =
    SampledGridSDF<T>::heightField(pool, Vec3(T(0), T(-1), T(0)), dx, dz, nx, nz, heights);

  REQUIRE(terrain.getNumNodes(0) == nx);
  REQUIRE(terrain.getNumNodes(1) == 2U);
  REQUIRE(terrain.getNumNodes(2) == nz);

  const T norm = std::sqrt(T(1) + T(0.09) + T(0.04));

  for (const Vec3& p : pointsIn<T>(Vec3(T(0.2), T(-0.6), T(0.2)), Vec3(T(1.8), T(1), T(2.8)), 500)) {
    const T d = (p[1] - (T(0.5) + T(0.3) * p[0] - T(0.2) * p[2])) / norm;

    // Far enough from the floor and the sides that the terrain's surface is nearer.
    if (std::abs(d) < T(0.2)) {
      REQUIRE_THAT(terrain.signedDistance(p), withinAbsT<T>(d, double(looseMargin<T>())));
    }
  }

  // Below the floor, outside.
  REQUIRE(terrain.signedDistance(Vec3(T(1), T(-1.5), T(1))) > T(0));
}

TEMPLATE_TEST_CASE("SampledGridSDF: its layout, a view on a mirror and a deep copy answer alike, also in a tape",
                   "[SampledGridSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Vec3         origin(T(-1), T(-1), T(-1));
  const Vec3         spacing(T(0.2), T(0.2), T(0.2));
  const uint32_t     n[3] = {11, 11, 11};
  const SphereSDF<T> sphere(Vec3::zeros(), T(0.7));

  const SampledGridSDF<T> grid(
    pool, origin, spacing, n[0], n[1], n[2], sample<T>(origin, spacing, n, [&](const Vec3& a_p) {
      return sphere.signedDistance(a_p);
    }));

  STATIC_REQUIRE(isPoolHandle<SampledGridSDF<T>>);
  STATIC_REQUIRE(isShape<SampledGridSDF<T>>);

  const Tape<T> tape(pool, Union(grid, SphereSDF<T>(Vec3(T(2), T(0), T(0)), T(0.5))));

  Pool copyPool(hostMemoryResource());

  const SampledGridSDF<T> copy = grid.deepCopy(copyPool);

  pool.freeze();

  Pool mirror = Pool::mirror(pool, hostMemoryResource());

  const SampledGridSDF<T> view = grid.rebasedView(mirror);
  const SampledGridSDF<T> rebuilt(grid.layout(), grid.location());

  for (const Vec3& p : pointsIn<T>(T(-1.5) * Vec3::ones(), T(1.5) * Vec3::ones(), 300)) {
    const T d = grid.signedDistance(p);

    REQUIRE(view.signedDistance(p) == d);
    REQUIRE(copy.signedDistance(p) == d);
    REQUIRE(rebuilt.signedDistance(p) == d);
    REQUIRE_THAT(
      tape.signedDistance(p),
      withinAbsT<T>(Math::min(d, T((p - Vec3(T(2), T(0), T(0))).length()) - T(0.5)), double(tightMargin<T>())));
  }
}

TEST_CASE("SampledGridSDF: invalid arguments abort with a message", "[SampledGridSDF][death]")
{
  using T    = double;
  using Vec3 = Vec3T<T>;

  REQUIRE(abortsWith(
    [] {
      Pool                    pool(hostMemoryResource());
      const SampledGridSDF<T> g(pool, Vec3::zeros(), Vec3::ones(), 2, 1, 2, std::vector<T>(4));

      (void)g;
    },
    "SampledGridSDF: the grid must have at least 2 nodes along each axis (2, 1, 2)"));

  REQUIRE(abortsWith(
    [] {
      Pool                    pool(hostMemoryResource());
      const SampledGridSDF<T> g(pool, Vec3::zeros(), Vec3::ones(), 2, 2, 2, std::vector<T>(7));

      (void)g;
    },
    "SampledGridSDF: the grid of 2 x 2 x 2 nodes needs as many values (7)"));

  REQUIRE(abortsWith(
    [] {
      Pool                    pool(hostMemoryResource());
      const SampledGridSDF<T> g = SampledGridSDF<T>::heightField(pool, Vec3::zeros(), T(1), T(1), 2, 2, {1, 1, 0, 1});

      (void)g;
    },
    "SampledGridSDF::heightField: height 2 (0) must be finite and above the floor (0)"));
}

TEMPLATE_TEST_CASE("SampledGridSDF: device signedDistance matches the host",
                   "[SampledGridSDF][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const Vec3         origin(T(-1), T(-1), T(-1));
  const Vec3         spacing(T(0.1), T(0.1), T(0.1));
  const uint32_t     n[3] = {21, 21, 21};
  const SphereSDF<T> sphere(Vec3(T(0.1), T(0), T(-0.1)), T(0.6));

  const SampledGridSDF<T> grid(
    pool, origin, spacing, n[0], n[1], n[2], sample<T>(origin, spacing, n, [&](const Vec3& a_p) {
      return sphere.signedDistance(a_p);
    }));

  pool.freeze();

  Pool                    devicePool = Pool::mirror(pool, deviceTestResource());
  const SampledGridSDF<T> deviceView = grid.rebasedView(devicePool);

  const auto points = queryGrid<T>(T(-1.5) * Vec3::ones(), T(1.5) * Vec3::ones(), 15);

  requireSameResults(evaluateOnDevice<T>(GridDistance<T>{deviceView}, points),
                     evaluateOnHost<T>(GridDistance<T>{grid}, points));
}
