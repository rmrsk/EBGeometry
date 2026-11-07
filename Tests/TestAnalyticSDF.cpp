// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cmath>
#include <cstring>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// ─────────────────────────────────────────────────────────────────────────────
// SphereSDF
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SphereSDF: exact signed distances", "[SphereSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T>     center(0, 0, 0);
  const SphereSDF<T> sphere(center, T(1.0));

  // On the surface
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(1, 0, 0)), WithinAbs(0.0, looseMargin<T>()));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(-1, 0, 0)), WithinAbs(0.0, looseMargin<T>()));

  // Outside: positive distance
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(2, 0, 0)), WithinRel(T(1.0)));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(0, 3, 0)), WithinRel(T(2.0)));

  // Inside: negative distance
  REQUIRE(sphere.signedDistance(Vec3T<T>(0, 0, 0)) < T(0.0));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(0, 0, 0)), WithinRel(T(-1.0)));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(0.5, 0, 0)), WithinRel(T(-0.5)));
}

TEMPLATE_TEST_CASE("SphereSDF: off-center sphere", "[SphereSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  SphereSDF<T> sphere(Vec3T<T>(1, 2, 3), T(2.0));

  // Point at center
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(1, 2, 3)), WithinRel(T(-2.0)));

  // Point on surface along x
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(3, 2, 3)), WithinAbs(0.0, looseMargin<T>()));

  // Getters
  REQUIRE(sphere.getCenter()[0] == T(1.0));
  REQUIRE(sphere.getRadius() == T(2.0));
}

// ─────────────────────────────────────────────────────────────────────────────
// PlaneSDF
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("PlaneSDF: signed distances from xy-plane", "[PlaneSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Plane z = 0 with outward normal +z
  const PlaneSDF<T> plane(Vec3T<T>(0, 0, 0), Vec3T<T>(0, 0, 1));

  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(0, 0, 3)), WithinRel(T(3.0)));
  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(0, 0, -2)), WithinRel(T(-2.0)));
  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(5, 7, 0)), WithinAbs(0.0, looseMargin<T>()));
}

TEMPLATE_TEST_CASE("PlaneSDF: non-unit normal is normalized", "[PlaneSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Normal (1,1,0): should be normalized internally
  const PlaneSDF<T> plane(Vec3T<T>(0, 0, 0), Vec3T<T>(2, 0, 0));

  // Point at (3, 0, 0): distance = 3 (along +x)
  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(3, 0, 0)), WithinRel(T(3.0)));
}

// ─────────────────────────────────────────────────────────────────────────────
// BoxSDF
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("BoxSDF: axis-aligned unit box", "[BoxSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const BoxSDF<T> box(Vec3T<T>(-1, -1, -1), Vec3T<T>(1, 1, 1));

  // Inside: negative SDF
  REQUIRE(box.signedDistance(Vec3T<T>(0, 0, 0)) < T(0.0));
  REQUIRE_THAT(box.signedDistance(Vec3T<T>(0, 0, 0)), WithinRel(T(-1.0)));

  // On a face: zero SDF
  REQUIRE_THAT(box.signedDistance(Vec3T<T>(1, 0, 0)), WithinAbs(0.0, looseMargin<T>()));

  // Outside along one axis: positive SDF
  REQUIRE_THAT(box.signedDistance(Vec3T<T>(3, 0, 0)), WithinRel(T(2.0)));

  // Outside near a corner
  // Point (2, 2, 0): distance = sqrt(1+1) = sqrt(2)
  REQUIRE_THAT(box.signedDistance(Vec3T<T>(2, 2, 0)), WithinRel(std::sqrt(T(2.0)), T(1.0e-4)));
}

// ─────────────────────────────────────────────────────────────────────────────
// CylinderSDF (defined by two end-cap centers + radius)
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("CylinderSDF: unit cylinder along z-axis", "[CylinderSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Cylinder from (0,0,-1) to (0,0,1), radius 1
  const CylinderSDF<T> cyl(Vec3T<T>(0, 0, -1), Vec3T<T>(0, 0, 1), T(1.0));

  // Center of cylinder is inside
  REQUIRE(cyl.signedDistance(Vec3T<T>(0, 0, 0)) < T(0.0));

  // Point on the curved surface
  REQUIRE_THAT(cyl.signedDistance(Vec3T<T>(1, 0, 0)), WithinAbs(0.0, looseMargin<T>()));

  // Outside radially
  REQUIRE(cyl.signedDistance(Vec3T<T>(3, 0, 0)) > T(0.0));

  // Outside axially beyond cap
  REQUIRE(cyl.signedDistance(Vec3T<T>(0, 0, 3)) > T(0.0));
}

// ─────────────────────────────────────────────────────────────────────────────
// TorusSDF
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("TorusSDF: point on surface", "[TorusSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Torus centered at origin, its ring in the xz-plane around the y axis: major radius 2, minor radius 0.5
  const TorusSDF<T> torus(Vec3T<T>(0, 0, 0), T(2.0), T(0.5));

  // Points on surface: along +x and +z from the tube center, and above the tube center in y
  REQUIRE_THAT(torus.signedDistance(Vec3T<T>(2.5, 0, 0)), WithinAbs(0.0, looseMargin<T>()));
  REQUIRE_THAT(torus.signedDistance(Vec3T<T>(0, 0, -2.5)), WithinAbs(0.0, looseMargin<T>()));
  REQUIRE_THAT(torus.signedDistance(Vec3T<T>(0, 0.5, 2)), WithinAbs(0.0, looseMargin<T>()));

  // Along the axis, the nearest point is on the inner equator: sqrt(2^2 + 1^2) - 0.5
  REQUIRE_THAT(torus.signedDistance(Vec3T<T>(0, 1, 0)), WithinRel(std::sqrt(T(5)) - T(0.5)));

  // Point inside the tube
  REQUIRE(torus.signedDistance(Vec3T<T>(2.0, 0, 0)) < T(0.0));

  // Point at the origin: outside the torus
  REQUIRE(torus.signedDistance(Vec3T<T>(0, 0, 0)) > T(0.0));
}

// ─────────────────────────────────────────────────────────────────────────────
// Plain value types
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Every analytic shape, with arguments matching the Shapes example. Shared by the value-type checks
// below and the device test at the bottom of this file.
template <class T>
struct AllShapes
{
  PlaneSDF<T>            plane{Vec3T<T>::zeros(), Vec3T<T>::ones()};
  SphereSDF<T>           sphere{Vec3T<T>::zeros(), T(1)};
  BoxSDF<T>              box{Vec3T<T>::zeros(), Vec3T<T>::ones()};
  TorusSDF<T>            torus{Vec3T<T>::zeros(), T(1), T(0.1)};
  CylinderSDF<T>         cylinder{Vec3T<T>::zeros(), Vec3T<T>::ones(), T(0.1)};
  InfiniteCylinderSDF<T> infiniteCylinder{Vec3T<T>::zeros(), T(0.1), 2};
  CapsuleSDF<T>          capsule{Vec3T<T>::zeros(), Vec3T<T>::ones(), T(0.1)};
  InfiniteConeSDF<T>     infiniteCone{Vec3T<T>::zeros(), T(45)};
  ConeSDF<T>             cone{Vec3T<T>::zeros(), T(1), T(45)};
  RoundedBoxSDF<T>       roundedBox{Vec3T<T>::zeros(), T(1.2) * Vec3T<T>::ones(), T(0.1)};
  PerlinSDF<T>           perlin{T(1), Vec3T<T>::ones(), T(0.5), 4U};
  RoundedCylinderSDF<T>  roundedCylinder{Vec3T<T>::zeros(), T(1), T(0.1), T(1)};
  EllipsoidSDF<T>        ellipsoid{Vec3T<T>::zeros(), Vec3T<T>(T(1), T(0.5), T(0.25))};
  CappedConeSDF<T>       cappedCone{Vec3T<T>::zeros(), Vec3T<T>::ones(), T(0.5), T(0.1)};
  RoundConeSDF<T>        roundCone{Vec3T<T>::zeros(), Vec3T<T>::ones(), T(0.3), T(0.1)};
  PolygonPrismSDF<T>     polygonPrism{Vec3T<T>::zeros(), Vec3T<T>::ones(), T(0.3), 6U, Vec3T<T>::unit(0)};
  TubeSDF<T>             tube{Vec3T<T>::zeros(), Vec3T<T>::ones(), T(0.1), T(0.2)};
  CappedTorusSDF<T>      cappedTorus{Vec3T<T>::zeros(), T(1), T(0.1), T(120)};
  CutSphereSDF<T>        cutSphere{Vec3T<T>::zeros(), T(1), T(0.3)};
  CutHollowSphereSDF<T>  cutHollowSphere{Vec3T<T>::zeros(), T(1), T(0.3), T(0.1)};
  WedgeSDF<T>            wedge{Vec3T<T>::zeros(), T(60)};
};

// Sum of every shape's signed distance at a_point, so one number checks every formula.
template <class T>
EBGEOMETRY_HOST_DEVICE
T
sumOfShapes(const AllShapes<T>& a_shapes, const Vec3T<T>& a_point) noexcept
{
  return a_shapes.plane.signedDistance(a_point) + a_shapes.sphere.signedDistance(a_point) +
         a_shapes.box.signedDistance(a_point) + a_shapes.torus.signedDistance(a_point) +
         a_shapes.cylinder.signedDistance(a_point) + a_shapes.infiniteCylinder.signedDistance(a_point) +
         a_shapes.capsule.signedDistance(a_point) + a_shapes.infiniteCone.signedDistance(a_point) +
         a_shapes.cone.signedDistance(a_point) + a_shapes.roundedBox.signedDistance(a_point) +
         a_shapes.perlin.signedDistance(a_point) + a_shapes.roundedCylinder.signedDistance(a_point) +
         a_shapes.ellipsoid.signedDistance(a_point) + a_shapes.cappedCone.signedDistance(a_point) +
         a_shapes.roundCone.signedDistance(a_point) + a_shapes.polygonPrism.signedDistance(a_point) +
         a_shapes.tube.signedDistance(a_point) + a_shapes.cappedTorus.signedDistance(a_point) +
         a_shapes.cutSphere.signedDistance(a_point) + a_shapes.cutHollowSphere.signedDistance(a_point) +
         a_shapes.wedge.signedDistance(a_point);
}

// Points that land inside, outside and near the surfaces of the shapes above.
template <class T>
std::vector<Vec3T<T>>
shapeSamplePoints()
{
  return {Vec3T<T>(T(0.1), T(0.2), T(0.3)),
          Vec3T<T>(T(0.5), T(0.5), T(0.5)),
          Vec3T<T>(T(1.05), T(0), T(0)),
          Vec3T<T>(T(-0.7), T(0.4), T(0.9)),
          Vec3T<T>(T(2), T(-1.5), T(3))};
}

} // namespace

TEMPLATE_TEST_CASE("Analytic shapes: plain, trivially copyable value types and shapes in the sense of isShape",
                   "[AnalyticSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  STATIC_REQUIRE(std::is_trivially_copyable_v<AllShapes<T>>);
  STATIC_REQUIRE_FALSE(std::is_polymorphic_v<SphereSDF<T>>);
  STATIC_REQUIRE_FALSE(std::is_polymorphic_v<PerlinSDF<T>>);
  STATIC_REQUIRE_FALSE(std::is_polymorphic_v<RoundedBoxSDF<T>>);
  STATIC_REQUIRE(isShape<SphereSDF<T>>);
  STATIC_REQUIRE(isShape<BoxSDF<T>>);
  STATIC_REQUIRE(isShape<PerlinSDF<T>>);
  STATIC_REQUIRE(std::is_same_v<ShapePrecision<SphereSDF<T>>, T>);

  // A byte copy is a complete copy: it evaluates exactly like the original.
  const AllShapes<T> shapes;
  AllShapes<T>       bytes;
  std::memcpy(static_cast<void*>(&bytes), static_cast<const void*>(&shapes), sizeof(AllShapes<T>));

  for (const auto& p : shapeSamplePoints<T>()) {
    REQUIRE(sumOfShapes(bytes, p) == sumOfShapes(shapes, p));
  }
}

TEMPLATE_TEST_CASE("RoundedBoxSDF: distances, and copies no longer share the rounding sphere",
                   "[RoundedBoxSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Outer size 3, rounding radius 0.5: the surface is at 1.5 along each axis.
  const RoundedBoxSDF<T> roundedBox(Vec3::zeros(), Vec3(T(3), T(3), T(3)), T(0.5));

  REQUIRE_THAT(roundedBox.signedDistance(Vec3(T(3), T(0), T(0))), WithinRel(T(1.5)));
  REQUIRE_THAT(roundedBox.signedDistance(Vec3(T(0), T(0), T(1.5))), WithinAbs(0.0, looseMargin<T>()));
  // Inside, the distance is to the nearest face (half-extent 1 + rounding 0.5), not the rounding
  // radius alone.
  REQUIRE_THAT(roundedBox.signedDistance(Vec3::zeros()), WithinRel(T(-1.5)));
  REQUIRE_THAT(roundedBox.signedDistance(Vec3(T(0.75), T(0), T(0))), WithinRel(T(-0.75)));

  // Before this class held its sphere by value, a copy shared it through a shared_ptr. A copy is now
  // an independent object, and a default-constructed box is unaffected by another box's radius.
  const RoundedBoxSDF<T> copy = roundedBox;
  const RoundedBoxSDF<T> other(Vec3::zeros(), Vec3(T(2.5), T(2.5), T(2.5)), T(0.25));

  REQUIRE(copy.signedDistance(Vec3(T(3), T(0), T(0))) == roundedBox.signedDistance(Vec3(T(3), T(0), T(0))));
  REQUIRE_THAT(other.signedDistance(Vec3(T(3), T(0), T(0))), WithinRel(T(1.75)));

  // The default is the unit cube [-0.5, 0.5]^3, rounded.
  REQUIRE_THAT(RoundedBoxSDF<T>().signedDistance(Vec3(T(1), T(0), T(0))), WithinRel(T(0.5)));

  // The outer size is the box the rounded box fits exactly: the face centers touch it, the corners
  // are pulled in by the rounding.
  const Vec3             center(T(1), T(-2), T(3));
  const Vec3             size(T(2), T(1), T(0.6));
  const RoundedBoxSDF<T> placed(center, size, T(0.2));

  for (size_t d = 0; d < 3; d++) {
    const Vec3 faceCenter = center + T(0.5) * size[d] * Vec3::unit(d);

    REQUIRE_THAT(placed.signedDistance(faceCenter), WithinAbs(0.0, looseMargin<T>()));
  }

  REQUIRE(placed.signedDistance(center + T(0.5) * size) > T(0));
  REQUIRE_THAT(placed.signedDistance(center), WithinRel(T(-0.3)));
}

TEMPLATE_TEST_CASE("RoundedCylinderSDF: placed at its center, axis along y",
                   "[RoundedCylinderSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3                  center(T(1), T(2), T(-1));
  const RoundedCylinderSDF<T> cylinder(center, T(0.5), T(0.1), T(2));

  REQUIRE_THAT(cylinder.signedDistance(center), WithinRel(T(-0.5)));
  REQUIRE_THAT(cylinder.signedDistance(center + Vec3(T(0), T(1), T(0))), WithinAbs(0.0, looseMargin<T>()));
  REQUIRE_THAT(cylinder.signedDistance(center + Vec3(T(0.5), T(0), T(0))), WithinAbs(0.0, looseMargin<T>()));
  REQUIRE_THAT(cylinder.signedDistance(center + Vec3(T(0), T(0), T(-1.5))), WithinRel(T(1)));
}

TEMPLATE_TEST_CASE("InfiniteConeSDF and ConeSDF: open along -y from the tip", "[ConeSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A 90-degree cone: its surface leaves the tip at 45 degrees, so (r, -r) is on it for any r.
  const Vec3               tip(T(0.5), T(1), T(-0.5));
  const InfiniteConeSDF<T> infiniteCone(tip, T(90));
  const ConeSDF<T>         cone(tip, T(2), T(90));

  for (const auto& p : {tip + Vec3(T(1), T(-1), T(0)), tip + Vec3(T(0), T(-0.5), T(0.5))}) {
    REQUIRE_THAT(infiniteCone.signedDistance(p), WithinAbs(0.0, looseMargin<T>()));
    REQUIRE_THAT(cone.signedDistance(p), WithinAbs(0.0, looseMargin<T>()));
  }

  // Below the tip on the axis is inside; above it, and along x or z, is outside.
  REQUIRE(infiniteCone.signedDistance(tip - Vec3::unit(1)) < T(0));
  REQUIRE(cone.signedDistance(tip - Vec3::unit(1)) < T(0));
  REQUIRE_THAT(cone.signedDistance(tip + Vec3::unit(1)), WithinRel(T(1)));
  REQUIRE(cone.signedDistance(tip - Vec3::unit(2)) > T(0));

  // The finite cone's base disc is a height below the tip.
  REQUIRE_THAT(cone.signedDistance(tip - T(3) * Vec3::unit(1)), WithinRel(T(1)));
}

namespace {

// Random points in the cube [-a_extent, a_extent]^3, for comparing two shapes that should agree.
template <class T>
std::vector<Vec3T<T>>
randomPoints(const T a_extent, const int a_count)
{
  std::mt19937                      rng(2468);
  std::uniform_real_distribution<T> coord(-a_extent, a_extent);
  std::vector<Vec3T<T>>             points;

  for (int i = 0; i < a_count; i++) {
    points.emplace_back(coord(rng), coord(rng), coord(rng));
  }

  return points;
}

// Requires two shapes to give the same signed distance at random points.
template <class T, class A, class B>
void
requireSameDistances(const A& a_shape, const B& a_reference, const T a_extent)
{
  for (const auto& p : randomPoints<T>(a_extent, 500)) {
    INFO("p = " << p);
    REQUIRE_THAT(a_shape.signedDistance(p), withinAbsT<T>(a_reference.signedDistance(p), looseMargin<T>()));
  }
}

} // namespace

TEMPLATE_TEST_CASE("EllipsoidSDF: a sphere when the radii agree, and the brute-force distance otherwise",
                   "[EllipsoidSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 center(T(0.3), T(-0.2), T(0.1));

  requireSameDistances<T>(EllipsoidSDF<T>(center, T(0.8) * Vec3::ones()), SphereSDF<T>(center, T(0.8)), T(2));

  // Along an axis the distance is plain: outside, beyond the semi-axis; at the center, the smallest one.
  const EllipsoidSDF<T> ellipsoid(center, Vec3(T(2), T(1), T(0.5)));

  REQUIRE_THAT(ellipsoid.signedDistance(center + Vec3(T(3), T(0), T(0))), WithinRel(T(1)));
  REQUIRE_THAT(ellipsoid.signedDistance(center + Vec3(T(0), T(-1.5), T(0))), WithinRel(T(0.5)));
  REQUIRE_THAT(ellipsoid.signedDistance(center), WithinRel(T(-0.5)));

  // Against the nearest of many points on the surface, (a cos u sin v, b cos v, c sin u sin v), at random
  // points inside and outside. The sampling overestimates by at most about (spacing / 2)^2 / (2 distance).
  const Vec3 radii(T(1.5), T(0.3), T(0.8));

  const EllipsoidSDF<T> flat(Vec3::zeros(), radii);

  constexpr int numU = 600;
  constexpr int numV = 300;

  std::vector<Vec3> surface;

  for (int i = 0; i < numU; i++) {
    for (int j = 0; j <= numV; j++) {
      const double u = 2.0 * pi<double> * i / numU;
      const double v = pi<double> * j / numV;

      surface.emplace_back(T(double(radii[0]) * std::cos(u) * std::sin(v)),
                           T(double(radii[1]) * std::cos(v)),
                           T(double(radii[2]) * std::sin(u) * std::sin(v)));
    }
  }

  // Random points, some on the coordinate planes and axes, where the closest point is found by the
  // algorithm's planar cases.
  std::vector<Vec3> points = randomPoints<T>(T(2), 56);

  for (size_t i = 0; i < points.size(); i++) {
    for (size_t d = 0; d < 3; d++) {
      if (((i % 8) >> d) & 1U) {
        points[i][d] = T(0);
      }
    }

    if (i % 3 == 0) {
      points[i] *= T(0.3);
    }
  }

  for (const auto& p : points) {
    T nearest = std::numeric_limits<T>::max();

    for (const auto& s : surface) {
      nearest = std::min(nearest, (p - s).length());
    }

    const T d = flat.signedDistance(p);

    INFO("p = " << p << ", d = " << d << ", sampled " << nearest);
    REQUIRE(std::abs(d) <= nearest + T(looseMargin<T>()));
    REQUIRE(std::abs(d) >= nearest - T(2e-3));
  }
}

TEMPLATE_TEST_CASE("EllipsoidSDF: a very flat and a very long ellipsoid converge to their distances",
                   "[EllipsoidSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Aspect ratios of 1000: a disc of radius 1 and half-thickness 1e-3, and a needle of half-length 1.
  const EllipsoidSDF<T> disc(Vec3::zeros(), Vec3(T(1), T(1e-3), T(1)));
  const EllipsoidSDF<T> needle(Vec3::zeros(), Vec3(T(1), T(1e-3), T(1e-3)));

  REQUIRE_THAT(disc.signedDistance(Vec3::zeros()), WithinRel(T(-1e-3), T(1e-4)));
  REQUIRE_THAT(disc.signedDistance(T(0.5) * Vec3::unit(1)), WithinRel(T(0.499), T(1e-4)));
  REQUIRE_THAT(disc.signedDistance(T(2) * Vec3::unit(0)), WithinRel(T(1), T(1e-4)));
  REQUIRE_THAT(needle.signedDistance(T(3) * Vec3::unit(0)), WithinRel(T(2), T(1e-4)));
  REQUIRE_THAT(needle.signedDistance(Vec3::unit(2)), WithinRel(T(1) - T(1e-3), T(1e-4)));

  // At random points: finite, of the sign of the implicit function, and 1-Lipschitz between neighbors.
  for (const auto* shape : {&disc, &needle}) {
    const Vec3 radii = shape->getRadii();
    Vec3       previous;
    T          previousValue = T(0);

    int i = 0;

    for (const auto& p : randomPoints<T>(T(2), 400)) {
      const T d = shape->signedDistance(p);
      const T g = (p[0] / radii[0]) * (p[0] / radii[0]) + (p[1] / radii[1]) * (p[1] / radii[1]) +
                  (p[2] / radii[2]) * (p[2] / radii[2]) - T(1);

      INFO("p = " << p << ", d = " << d);
      REQUIRE(std::isfinite(d));
      REQUIRE((d < T(0)) == (g < T(0)));

      if (i++ > 0) {
        REQUIRE(std::abs(d - previousValue) <= (p - previous).length() * (T(1) + T(1e-3)) + T(1e-5));
      }

      previous      = p;
      previousValue = d;
    }
  }
}

TEMPLATE_TEST_CASE("CappedConeSDF: a cylinder for equal radii, and ConeSDF for a zero radius",
                   "[CappedConeSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 c1(T(-0.2), T(-0.6), T(0.1));
  const Vec3 c2(T(0.3), T(0.7), T(-0.2));

  requireSameDistances<T>(CappedConeSDF<T>(c1, c2, T(0.4), T(0.4)), CylinderSDF<T>(c1, c2, T(0.4)), T(2));

  // ConeSDF's tip is at the top, opening downward: a frustum from a base of radius 0.5 to a tip 1.5 above it.
  const Vec3 tip(T(0.1), T(0.8), T(-0.1));
  const T    angle = T(2) * std::atan(T(0.5) / T(1.5)) * T(180) / pi<T>;

  requireSameDistances<T>(
    CappedConeSDF<T>(tip - T(1.5) * Vec3::unit(1), tip, T(0.5), T(0)), ConeSDF<T>(tip, T(1.5), angle), T(2));
}

TEMPLATE_TEST_CASE("RoundConeSDF: a capsule for equal radii, and spheres at its ends",
                   "[RoundConeSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 c1(T(-0.2), T(-0.6), T(0.1));
  const Vec3 c2(T(0.3), T(0.7), T(-0.2));
  const Vec3 axis = (c2 - c1) / (c2 - c1).length();

  // CapsuleSDF takes its outer tips, a radius beyond the centers.
  requireSameDistances<T>(
    RoundConeSDF<T>(c1, c2, T(0.3), T(0.3)), CapsuleSDF<T>(c1 - T(0.3) * axis, c2 + T(0.3) * axis, T(0.3)), T(2));

  // Beyond either end on the axis, the distance is to that end's sphere.
  const RoundConeSDF<T> cone(c1, c2, T(0.5), T(0.2));

  REQUIRE_THAT(cone.signedDistance(c1 - T(1) * axis), WithinRel(T(0.5)));
  REQUIRE_THAT(cone.signedDistance(c2 + T(1) * axis), WithinRel(T(0.8)));
}

TEMPLATE_TEST_CASE("PolygonPrismSDF: a square prism is a box, and a hexagon's edges are at its apothem",
                   "[PolygonPrismSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Four sides with vertices on the diagonals: a square of half-side R / sqrt(2), axis-aligned.
  const T half = T(0.5);

  requireSameDistances<T>(
    PolygonPrismSDF<T>(Vec3(T(0), T(-0.4), T(0)), Vec3(T(0), T(0.9), T(0)), half * std::sqrt(T(2)), 4U, Vec3(1, 0, 1)),
    BoxSDF<T>(Vec3(-half, T(-0.4), -half), Vec3(half, T(0.9), half)),
    T(2));

  // A hexagon along z, a vertex along x: vertices at radius 1, edges' middles at the apothem cos(30).
  const PolygonPrismSDF<T> hexagon(Vec3::zeros(), Vec3(T(0), T(0), T(2)), T(1), 6U, Vec3::unit(0));
  const T                  apothem = std::cos(pi<T> / T(6));

  REQUIRE_THAT(hexagon.signedDistance(Vec3(T(2), T(0), T(1))), WithinRel(T(1)));
  REQUIRE_THAT(hexagon.signedDistance(Vec3(T(0), T(2), T(1))), WithinRel(T(2) - apothem));
  REQUIRE_THAT(hexagon.signedDistance(Vec3(T(0), T(0), T(1))), WithinRel(-apothem));
  REQUIRE_THAT(hexagon.signedDistance(Vec3(T(0), T(0), T(3))), WithinRel(T(1)));

  // Turning the vertex direction turns the polygon: a vertex now along y.
  const PolygonPrismSDF<T> turned(Vec3::zeros(), Vec3(T(0), T(0), T(2)), T(1), 6U, Vec3(T(0), T(1), T(5)));

  REQUIRE_THAT(turned.signedDistance(Vec3(T(0), T(2), T(1))), WithinRel(T(1)));
  REQUIRE_THAT(turned.signedDistance(Vec3(T(2), T(0), T(1))), WithinRel(T(2) - apothem));
}

TEMPLATE_TEST_CASE("TubeSDF: hollow along its axis, solid in its wall", "[TubeSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const TubeSDF<T> tube(Vec3::zeros(), Vec3(T(0), T(0), T(4)), T(0.5), T(1));

  REQUIRE_THAT(tube.signedDistance(Vec3(T(0), T(0), T(2))), WithinRel(T(0.5)));
  REQUIRE_THAT(tube.signedDistance(Vec3(T(0.75), T(0), T(2))), WithinRel(T(-0.25)));
  REQUIRE_THAT(tube.signedDistance(Vec3(T(0), T(3), T(2))), WithinRel(T(2)));
  REQUIRE_THAT(tube.signedDistance(Vec3(T(0), T(-0.75), T(5))), WithinRel(T(1)));
  REQUIRE_THAT(tube.signedDistance(Vec3(T(0), T(0), T(5))), WithinRel(std::sqrt(T(1.25))));
  REQUIRE_THAT(tube.signedDistance(Vec3(T(0.75), T(0), T(0.1))), WithinRel(T(-0.1)));
}

TEMPLATE_TEST_CASE("CappedTorusSDF: the torus inside its angle, flat ends outside it, the torus when whole",
                   "[CappedTorusSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 center(T(0.1), T(-0.2), T(0.3));

  requireSameDistances<T>(CappedTorusSDF<T>(center, T(1), T(0.3), T(360)), TorusSDF<T>(center, T(1), T(0.3)), T(2));

  // Half a torus, about +x: its ends are the discs around (0, 0, +-1) in the plane x = 0.
  const CappedTorusSDF<T> half(center, T(1), T(0.3), T(180));
  const TorusSDF<T>       torus(center, T(1), T(0.3));

  REQUIRE_THAT(half.signedDistance(center + Vec3(T(-0.4), T(0), T(1))), WithinRel(T(0.4)));
  REQUIRE_THAT(half.signedDistance(center + Vec3(T(-0.4), T(0.1), T(-1))), WithinRel(T(0.4)));
  REQUIRE_THAT(half.signedDistance(center + Vec3(T(-0.4), T(0.5), T(-1))), WithinRel(std::sqrt(T(0.2))));

  // Inside its angle, and outside the tube, it is the torus.
  for (const auto& p : {Vec3(T(1.5), T(0.2), T(0.1)), Vec3(T(0.2), T(0.5), T(-0.9)), Vec3(T(0), T(0), T(0))}) {
    REQUIRE_THAT(half.signedDistance(center + p), withinAbsT<T>(torus.signedDistance(center + p), looseMargin<T>()));
  }

  // Inside the tube next to an end, the end is nearer than the tube wall.
  REQUIRE_THAT(half.signedDistance(center + Vec3(T(0.05), T(0), T(1))), WithinRel(T(-0.05)));
}

TEMPLATE_TEST_CASE("CutSphereSDF: a dome above its cut", "[CutSphereSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 center(T(0.1), T(-0.2), T(0.3));

  // The upper half: below the center is outside, the center is on the flat face.
  const CutSphereSDF<T> half(center, T(1), T(0));

  REQUIRE_THAT(half.signedDistance(center - T(0.5) * Vec3::unit(1)), WithinRel(T(0.5)));
  REQUIRE_THAT(half.signedDistance(center + T(1.5) * Vec3::unit(1)), WithinRel(T(0.5)));
  REQUIRE_THAT(half.signedDistance(center), WithinAbs(0.0, looseMargin<T>()));
  REQUIRE_THAT(half.signedDistance(center + T(0.5) * Vec3::unit(1)), WithinRel(T(-0.5)));

  // A dome cut 0.5 up: the rim is at radius sqrt(0.75), and the center is 0.5 below the flat face.
  const CutSphereSDF<T> dome(center, T(1), T(0.5));
  const T               rim = std::sqrt(T(0.75));

  REQUIRE_THAT(dome.signedDistance(center), WithinRel(T(0.5)));
  REQUIRE_THAT(dome.signedDistance(center + Vec3(rim + T(0.5), T(0.5), T(0))), WithinRel(T(0.5)));

  // Cut 0.5 down: more than a hemisphere, whose center is inside.
  const CutSphereSDF<T> ball(center, T(1), T(-0.5));

  REQUIRE_THAT(ball.signedDistance(center), WithinRel(T(-0.5)));
}

TEMPLATE_TEST_CASE("CutHollowSphereSDF: a shell above its cut, with rounded rims",
                   "[CutHollowSphereSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3                  center(T(0.1), T(-0.2), T(0.3));
  const CutHollowSphereSDF<T> shell(center, T(1), T(0.5), T(0.2));
  const T                     rim = std::sqrt(T(0.75));

  REQUIRE_THAT(shell.signedDistance(center), WithinRel(T(0.9)));
  REQUIRE_THAT(shell.signedDistance(center + Vec3::unit(1)), WithinRel(T(-0.1)));
  REQUIRE_THAT(shell.signedDistance(center + Vec3(rim, T(0.5), T(0))), WithinRel(T(-0.1)));

  // Below the rim, the nearest point is on the rounded rim, not the sphere.
  REQUIRE_THAT(shell.signedDistance(center + Vec3(rim, T(0), T(0))), WithinRel(T(0.4)));
  REQUIRE_THAT(shell.signedDistance(center - Vec3::unit(1)), WithinRel(std::sqrt(T(0.75) + T(2.25)) - T(0.1)));
}

TEMPLATE_TEST_CASE("WedgeSDF: inside below its edge, distances to a face or the edge outside",
                   "[WedgeSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A right-angled wedge: its faces leave the edge at 45 degrees below horizontal.
  const Vec3        tip(T(1), T(2), T(3));
  const WedgeSDF<T> wedge(tip, T(90));
  const T           c = std::sqrt(T(0.5));

  REQUIRE_THAT(wedge.signedDistance(tip - T(2) * Vec3::unit(1)), WithinRel(T(-2) * c));
  REQUIRE_THAT(wedge.signedDistance(tip + T(2) * Vec3::unit(1)), WithinRel(T(2)));
  REQUIRE_THAT(wedge.signedDistance(tip + T(2) * Vec3::unit(0)), WithinRel(T(2) * c));
  REQUIRE_THAT(wedge.signedDistance(tip + Vec3(T(-1), T(-1), T(7))), WithinAbs(0.0, looseMargin<T>()));
  REQUIRE_THAT(wedge.signedDistance(tip + Vec3(T(1), T(1), T(0))), WithinRel(std::sqrt(T(2))));
}

namespace {

// Checks that a_shape behaves as an exact signed distance function at random points in the cube
// [-a_extent, a_extent]^3. For an exact SDF f and a point p off the medial axis, |grad f| = 1, and
// stepping back along the gradient by f(p) lands on the surface: f(p - f(p) grad f(p)) = 0. A
// formula that is constant somewhere (zero gradient), or that over- or underestimates the distance,
// fails one of the two. Points near the medial axis, where the gradient is discontinuous and a
// central difference straddles the ridge, are skipped; they must be rare. f must also be
// 1-Lipschitz between any two sample points.
template <class T, class Shape>
void
requireExactSDF(const Shape& a_shape, const T a_extent)
{
  using Vec3 = Vec3T<T>;

  const T h   = std::cbrt(std::numeric_limits<T>::epsilon()) * a_extent;
  const T tol = std::is_same_v<T, float> ? T(2e-2) : T(1e-5);

  std::mt19937                      rng(12345);
  std::uniform_real_distribution<T> coord(-a_extent, a_extent);

  constexpr int numPoints = 2000;

  int  ridgePoints = 0;
  Vec3 previous    = Vec3::zeros();

  for (int i = 0; i < numPoints; i++) {
    const Vec3 p(coord(rng), coord(rng), coord(rng));
    const T    f = a_shape.signedDistance(p);

    REQUIRE(std::isfinite(f));

    INFO("p = " << p << ", f = " << f);

    if (i > 0) {
      REQUIRE(std::abs(f - a_shape.signedDistance(previous)) <= (p - previous).length() * (T(1) + tol) + tol);
    }

    previous = p;

    // The gradient by central differences with the step h and with half of it: where they differ, the wider
    // step straddles a ridge, even if the gradient it gives is close to unit length.
    Vec3 grad;
    Vec3 half;

    for (size_t d = 0; d < 3; d++) {
      const Vec3 e = h * Vec3::unit(d);

      grad[d] = (a_shape.signedDistance(p + e) - a_shape.signedDistance(p - e)) / (T(2) * h);
      half[d] = (a_shape.signedDistance(p + T(0.5) * e) - a_shape.signedDistance(p - T(0.5) * e)) / h;
    }

    if (std::abs(grad.length() - T(1)) > tol || (grad - half).length() > tol) {
      ridgePoints++;

      continue;
    }

    const Vec3 onSurface = p - f * grad / grad.length();

    REQUIRE(std::abs(a_shape.signedDistance(onSurface)) <= tol * std::max(T(1), std::abs(f)));
  }

  REQUIRE(ridgePoints <= numPoints / 50);
}

} // namespace

TEMPLATE_TEST_CASE("Analytic shapes: every exact shape is a true signed distance function",
                   "[AnalyticSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  SECTION("SphereSDF")
  {
    requireExactSDF<T>(SphereSDF<T>(Vec3(T(0.1), T(-0.2), T(0.3)), T(0.8)), T(2));
  }
  SECTION("PlaneSDF")
  {
    requireExactSDF<T>(PlaneSDF<T>(Vec3(T(0.1), T(0.2), T(0.3)), Vec3(T(1), T(2), T(-1))), T(2));
  }
  SECTION("BoxSDF")
  {
    requireExactSDF<T>(BoxSDF<T>(Vec3(T(-0.5), T(-0.3), T(-0.8)), Vec3(T(0.7), T(0.4), T(0.2))), T(2));
  }
  SECTION("TorusSDF")
  {
    requireExactSDF<T>(TorusSDF<T>(Vec3(T(0.1), T(0), T(-0.1)), T(1), T(0.3)), T(2));
  }
  SECTION("CylinderSDF")
  {
    requireExactSDF<T>(CylinderSDF<T>(Vec3(T(-0.2), T(-0.5), T(0.1)), Vec3(T(0.3), T(0.6), T(-0.2)), T(0.4)), T(2));
  }
  SECTION("InfiniteCylinderSDF")
  {
    requireExactSDF<T>(InfiniteCylinderSDF<T>(Vec3(T(0.1), T(0.2), T(0)), T(0.5), 1), T(2));
  }
  SECTION("CapsuleSDF")
  {
    requireExactSDF<T>(CapsuleSDF<T>(Vec3(T(0), T(0), T(-1)), Vec3(T(0.2), T(0.1), T(1.5)), T(0.4)), T(2));
  }
  SECTION("ConeSDF")
  {
    requireExactSDF<T>(ConeSDF<T>(Vec3(T(0), T(0.8), T(0)), T(1.5), T(60)), T(2));
  }
  SECTION("InfiniteConeSDF")
  {
    requireExactSDF<T>(InfiniteConeSDF<T>(Vec3(T(0), T(0.5), T(0)), T(50)), T(2));
  }
  SECTION("RoundedBoxSDF")
  {
    requireExactSDF<T>(RoundedBoxSDF<T>(Vec3(T(0.1), T(-0.1), T(0.2)), Vec3(T(1.2), T(0.8), T(0.6)), T(0.1)), T(1));
  }
  SECTION("RoundedCylinderSDF")
  {
    requireExactSDF<T>(RoundedCylinderSDF<T>(Vec3(T(-0.1), T(0.2), T(0.1)), T(0.8), T(0.2), T(1.6)), T(1.5));
  }
  SECTION("EllipsoidSDF")
  {
    requireExactSDF<T>(EllipsoidSDF<T>(Vec3(T(0.1), T(-0.2), T(0.1)), Vec3(T(1.2), T(0.4), T(0.7))), T(2));
    requireExactSDF<T>(EllipsoidSDF<T>(Vec3::zeros(), Vec3(T(0.1), T(1.5), T(0.6))), T(2));
  }
  SECTION("CappedConeSDF")
  {
    requireExactSDF<T>(CappedConeSDF<T>(Vec3(T(-0.2), T(-0.6), T(0.1)), Vec3(T(0.3), T(0.7), T(-0.2)), T(0.8), T(0.3)),
                       T(2));
    requireExactSDF<T>(CappedConeSDF<T>(Vec3(T(0.1), T(-0.5), T(0)), Vec3(T(-0.2), T(0.8), T(0.3)), T(0), T(0.6)),
                       T(2));
  }
  SECTION("RoundConeSDF")
  {
    requireExactSDF<T>(RoundConeSDF<T>(Vec3(T(-0.2), T(-0.6), T(0.1)), Vec3(T(0.3), T(0.7), T(-0.2)), T(0.7), T(0.2)),
                       T(2));
  }
  SECTION("PolygonPrismSDF")
  {
    for (const unsigned sides : {3U, 4U, 6U, 11U}) {
      requireExactSDF<T>(
        PolygonPrismSDF<T>(Vec3(T(-0.2), T(-0.6), T(0.1)), Vec3(T(0.3), T(0.7), T(-0.2)), T(0.9), sides, Vec3::ones()),
        T(2));
    }
  }
  SECTION("TubeSDF")
  {
    requireExactSDF<T>(TubeSDF<T>(Vec3(T(-0.2), T(-0.6), T(0.1)), Vec3(T(0.3), T(0.7), T(-0.2)), T(0.5), T(0.9)), T(2));
  }
  SECTION("CappedTorusSDF")
  {
    for (const T angle : {T(60), T(180), T(270), T(360)}) {
      requireExactSDF<T>(CappedTorusSDF<T>(Vec3(T(0.1), T(0), T(-0.1)), T(1), T(0.3), angle), T(2));
    }
  }
  SECTION("CutSphereSDF")
  {
    requireExactSDF<T>(CutSphereSDF<T>(Vec3(T(0.1), T(-0.2), T(0.1)), T(1.2), T(0.4)), T(2));
    requireExactSDF<T>(CutSphereSDF<T>(Vec3(T(0.1), T(-0.2), T(0.1)), T(1.2), T(-0.5)), T(2));
  }
  SECTION("CutHollowSphereSDF")
  {
    requireExactSDF<T>(CutHollowSphereSDF<T>(Vec3(T(0.1), T(-0.2), T(0.1)), T(1.2), T(0.4), T(0.2)), T(2));
    requireExactSDF<T>(CutHollowSphereSDF<T>(Vec3(T(0.1), T(-0.2), T(0.1)), T(1.2), T(-0.5), T(0.3)), T(2));
  }
  SECTION("WedgeSDF")
  {
    requireExactSDF<T>(WedgeSDF<T>(Vec3(T(0.1), T(0.3), T(-0.2)), T(50)), T(2));
    requireExactSDF<T>(WedgeSDF<T>(Vec3(T(0.1), T(0.3), T(-0.2)), T(150)), T(2));
  }
}

namespace {

// Requires a_box to have the given corners.
template <class T>
void
requireBox(const BoundingVolumes::AABBT<T>& a_box, const Vec3T<T>& a_lo, const Vec3T<T>& a_hi)
{
  INFO("box [" << a_box.getLowCorner() << ", " << a_box.getHighCorner() << "], expected [" << a_lo << ", " << a_hi
               << "]");

  for (size_t d = 0; d < 3; d++) {
    REQUIRE_THAT(a_box.getLowCorner()[d], WithinRel(a_lo[d], T(10) * std::numeric_limits<T>::epsilon()));
    REQUIRE_THAT(a_box.getHighCorner()[d], WithinRel(a_hi[d], T(10) * std::numeric_limits<T>::epsilon()));
  }
}

// Requires a_shape's surface to lie inside its bounding box: at random points outside the box, the
// signed distance is at least the distance to the box.
template <class T, class Shape>
void
requireSurfaceInsideBox(const Shape& a_shape, const T a_extent)
{
  using Vec3 = Vec3T<T>;

  const auto box = a_shape.computeBoundingVolume();
  const T    tol = std::is_same_v<T, float> ? T(1e-5) : T(1e-12);

  std::mt19937                      rng(54321);
  std::uniform_real_distribution<T> coord(-a_extent, a_extent);

  int outside = 0;

  for (int i = 0; i < 2000; i++) {
    const Vec3 p(coord(rng), coord(rng), coord(rng));
    const T    boxDistance = box.getDistance(p);

    if (boxDistance > T(0)) {
      INFO("p = " << p);
      REQUIRE(a_shape.signedDistance(p) >= boxDistance - tol * (T(1) + boxDistance));

      outside++;
    }
  }

  // A box that missed most of the sampled cube would make the check above vacuous.
  REQUIRE(outside > 100);
}

} // namespace

TEMPLATE_TEST_CASE("Analytic shapes: computeBoundingVolume is the shape's bounding box",
                   "[AnalyticSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const T big = Math::Limits<T>::max();
  const T r30 = T(1.5) * std::tan(pi<T> / T(6));

  SECTION("SphereSDF")
  {
    const SphereSDF<T> shape(Vec3(T(0.1), T(-0.2), T(0.3)), T(0.8));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-0.7), T(-1), T(-0.5)), Vec3(T(0.9), T(0.6), T(1.1)));
    requireSurfaceInsideBox<T>(shape, T(2));
  }
  SECTION("BoxSDF")
  {
    const BoxSDF<T> shape(Vec3(T(-0.5), T(-0.3), T(-0.8)), Vec3(T(0.7), T(0.4), T(0.2)));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-0.5), T(-0.3), T(-0.8)), Vec3(T(0.7), T(0.4), T(0.2)));
    requireSurfaceInsideBox<T>(shape, T(2));
  }
  SECTION("TorusSDF")
  {
    const TorusSDF<T> shape(Vec3(T(0.1), T(0), T(-0.1)), T(1), T(0.3));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-1.2), T(-0.3), T(-1.4)), Vec3(T(1.4), T(0.3), T(1.2)));
    requireSurfaceInsideBox<T>(shape, T(2));
  }
  SECTION("CylinderSDF")
  {
    const CylinderSDF<T> straight(Vec3(T(0), T(0), T(-1)), Vec3(T(0), T(0), T(1)), T(0.5));

    requireBox<T>(straight.computeBoundingVolume(), Vec3(T(-0.5), T(-0.5), T(-1)), Vec3(T(0.5), T(0.5), T(1)));

    // Tilted 45 degrees in the xy-plane, each cap disc reaches r sqrt(1/2) beyond its center in x and y.
    const CylinderSDF<T> tilted(Vec3::zeros(), Vec3(T(1), T(1), T(0)), T(0.5));
    const T              reach = T(0.5) * std::sqrt(T(0.5));

    requireBox<T>(tilted.computeBoundingVolume(), Vec3(-reach, -reach, T(-0.5)), Vec3(1 + reach, 1 + reach, T(0.5)));
    requireSurfaceInsideBox<T>(tilted, T(2));
    requireSurfaceInsideBox<T>(CylinderSDF<T>(Vec3(T(-0.2), T(-0.5), T(0.1)), Vec3(T(0.3), T(0.6), T(-0.2)), T(0.4)),
                               T(2));
  }
  SECTION("InfiniteCylinderSDF")
  {
    const InfiniteCylinderSDF<T> shape(Vec3(T(0.1), T(0.2), T(0)), T(0.5), 1);

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-0.4), -big, T(-0.5)), Vec3(T(0.6), big, T(0.5)));
    requireSurfaceInsideBox<T>(shape, T(2));
  }
  SECTION("CapsuleSDF")
  {
    const CapsuleSDF<T> shape(Vec3(T(0), T(0), T(-1)), Vec3(T(0), T(0), T(1)), T(0.4));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-0.4), T(-0.4), T(-1)), Vec3(T(0.4), T(0.4), T(1)));
    requireSurfaceInsideBox<T>(CapsuleSDF<T>(Vec3(T(0), T(0), T(-1)), Vec3(T(0.2), T(0.1), T(1.5)), T(0.4)), T(2));
  }
  SECTION("InfiniteConeSDF")
  {
    const InfiniteConeSDF<T> shape(Vec3(T(0), T(0.5), T(0)), T(50));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(-big, -big, -big), Vec3(big, T(0.5), big));
    requireSurfaceInsideBox<T>(shape, T(2));
  }
  SECTION("ConeSDF")
  {
    const ConeSDF<T> shape(Vec3(T(0), T(0.8), T(0)), T(1.5), T(60));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(-r30, T(-0.7), -r30), Vec3(r30, T(0.8), r30));
    requireSurfaceInsideBox<T>(shape, T(2));
  }
  SECTION("RoundedBoxSDF")
  {
    const RoundedBoxSDF<T> shape(Vec3(T(0.1), T(-0.1), T(0.2)), Vec3(T(1.2), T(0.8), T(0.6)), T(0.1));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-0.5), T(-0.5), T(-0.1)), Vec3(T(0.7), T(0.3), T(0.5)));
    requireSurfaceInsideBox<T>(shape, T(2));
  }
  SECTION("RoundedCylinderSDF")
  {
    const RoundedCylinderSDF<T> shape(Vec3(T(-0.1), T(0.2), T(0.1)), T(0.8), T(0.2), T(1.6));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-0.9), T(-0.6), T(-0.7)), Vec3(T(0.7), T(1), T(0.9)));
    requireSurfaceInsideBox<T>(shape, T(2));
  }
  SECTION("EllipsoidSDF")
  {
    const EllipsoidSDF<T> shape(Vec3(T(0.1), T(-0.2), T(0.1)), Vec3(T(1.2), T(0.4), T(0.7)));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-1.1), T(-0.6), T(-0.6)), Vec3(T(1.3), T(0.2), T(0.8)));
    requireSurfaceInsideBox<T>(shape, T(2));
  }
  SECTION("CappedConeSDF")
  {
    const CappedConeSDF<T> straight(Vec3(T(0), T(0), T(-1)), Vec3(T(0), T(0), T(1)), T(0.5), T(0.2));

    requireBox<T>(straight.computeBoundingVolume(), Vec3(T(-0.5), T(-0.5), T(-1)), Vec3(T(0.5), T(0.5), T(1)));
    requireSurfaceInsideBox<T>(
      CappedConeSDF<T>(Vec3(T(-0.2), T(-0.6), T(0.1)), Vec3(T(0.3), T(0.7), T(-0.2)), T(0.8), T(0.3)), T(2));
  }
  SECTION("RoundConeSDF")
  {
    const RoundConeSDF<T> shape(Vec3(T(0), T(0), T(-1)), Vec3(T(0), T(0), T(1)), T(0.5), T(0.2));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-0.5), T(-0.5), T(-1.5)), Vec3(T(0.5), T(0.5), T(1.2)));
    requireSurfaceInsideBox<T>(
      RoundConeSDF<T>(Vec3(T(-0.2), T(-0.6), T(0.1)), Vec3(T(0.3), T(0.7), T(-0.2)), T(0.7), T(0.2)), T(2));
  }
  SECTION("PolygonPrismSDF")
  {
    const PolygonPrismSDF<T> shape(Vec3(T(0), T(-1), T(0)), Vec3(T(0), T(1), T(0)), T(0.5), 6U, Vec3::unit(0));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-0.5), T(-1), T(-0.5)), Vec3(T(0.5), T(1), T(0.5)));
    requireSurfaceInsideBox<T>(
      PolygonPrismSDF<T>(Vec3(T(-0.2), T(-0.6), T(0.1)), Vec3(T(0.3), T(0.7), T(-0.2)), T(0.9), 5U, Vec3::ones()),
      T(2));
  }
  SECTION("TubeSDF")
  {
    const TubeSDF<T> shape(Vec3(T(0), T(0), T(-1)), Vec3(T(0), T(0), T(1)), T(0.3), T(0.5));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(T(-0.5), T(-0.5), T(-1)), Vec3(T(0.5), T(0.5), T(1)));
    requireSurfaceInsideBox<T>(
      TubeSDF<T>(Vec3(T(-0.2), T(-0.6), T(0.1)), Vec3(T(0.3), T(0.7), T(-0.2)), T(0.5), T(0.9)), T(2));
  }
  SECTION("CappedTorusSDF")
  {
    // 90 degrees about +x: x from cos(45) (R - r) to R + r, z within (R + r) sin(45).
    const T c = std::sqrt(T(0.5));

    requireBox<T>(CappedTorusSDF<T>(Vec3::zeros(), T(1), T(0.25), T(90)).computeBoundingVolume(),
                  Vec3(T(0.75) * c, T(-0.25), T(-1.25) * c),
                  Vec3(T(1.25), T(0.25), T(1.25) * c));

    // 270 degrees: past +-90, so z reaches R + r; x down to (R + r) cos(135).
    requireBox<T>(CappedTorusSDF<T>(Vec3::zeros(), T(1), T(0.25), T(270)).computeBoundingVolume(),
                  Vec3(T(-1.25) * c, T(-0.25), T(-1.25)),
                  Vec3(T(1.25), T(0.25), T(1.25)));

    for (const T angle : {T(60), T(180), T(270), T(360)}) {
      requireSurfaceInsideBox<T>(CappedTorusSDF<T>(Vec3(T(0.1), T(0), T(-0.1)), T(1), T(0.3), angle), T(2));
    }
  }
  SECTION("CutSphereSDF")
  {
    const T rim = std::sqrt(T(1.2) * T(1.2) - T(0.4) * T(0.4));

    requireBox<T>(CutSphereSDF<T>(Vec3::zeros(), T(1.2), T(0.4)).computeBoundingVolume(),
                  Vec3(-rim, T(0.4), -rim),
                  Vec3(rim, T(1.2), rim));
    requireBox<T>(CutSphereSDF<T>(Vec3::zeros(), T(1.2), T(-0.4)).computeBoundingVolume(),
                  Vec3(T(-1.2), T(-0.4), T(-1.2)),
                  Vec3(T(1.2), T(1.2), T(1.2)));
    requireSurfaceInsideBox<T>(CutSphereSDF<T>(Vec3(T(0.1), T(-0.2), T(0.1)), T(1.2), T(0.4)), T(2));
    requireSurfaceInsideBox<T>(CutSphereSDF<T>(Vec3(T(0.1), T(-0.2), T(0.1)), T(1.2), T(-0.5)), T(2));
  }
  SECTION("CutHollowSphereSDF")
  {
    requireBox<T>(CutHollowSphereSDF<T>(Vec3::zeros(), T(1), T(-0.5), T(0.2)).computeBoundingVolume(),
                  Vec3(T(-1.1), T(-0.6), T(-1.1)),
                  Vec3(T(1.1), T(1.1), T(1.1)));
    requireSurfaceInsideBox<T>(CutHollowSphereSDF<T>(Vec3(T(0.1), T(-0.2), T(0.1)), T(1.2), T(0.4), T(0.2)), T(2));
    requireSurfaceInsideBox<T>(CutHollowSphereSDF<T>(Vec3(T(0.1), T(-0.2), T(0.1)), T(1.2), T(-0.5), T(0.3)), T(2));
  }
  SECTION("WedgeSDF")
  {
    const WedgeSDF<T> shape(Vec3(T(0.1), T(0.3), T(-0.2)), T(50));

    requireBox<T>(shape.computeBoundingVolume(), Vec3(-big, -big, -big), Vec3(big, T(0.3), big));
    requireSurfaceInsideBox<T>(shape, T(2));
  }
  SECTION("PlaneSDF and PerlinSDF fill all of space")
  {
    requireBox<T>(PlaneSDF<T>().computeBoundingVolume(), Vec3(-big, -big, -big), Vec3(big, big, big));
    requireBox<T>(PerlinSDF<T>().computeBoundingVolume(), Vec3(-big, -big, -big), Vec3(big, big, big));
  }
}

namespace {

// A type without a distanceQuality member.
struct NoQualityDeclared
{
};

} // namespace

TEMPLATE_TEST_CASE("Analytic shapes: distance quality", "[AnalyticSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr auto Exact = DistanceQuality::Exact;

  // Every shape but the noise passes the exactness oracle above.
  STATIC_REQUIRE(PlaneSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(SphereSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(BoxSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(TorusSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(CylinderSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(InfiniteCylinderSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(CapsuleSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(InfiniteConeSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(ConeSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(RoundedBoxSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(RoundedCylinderSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(PerlinSDF<T>::distanceQuality == DistanceQuality::NotADistance);
  STATIC_REQUIRE(EllipsoidSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(CappedConeSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(RoundConeSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(PolygonPrismSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(TubeSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(CappedTorusSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(CutSphereSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(CutHollowSphereSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(WedgeSDF<T>::distanceQuality == Exact);

  STATIC_REQUIRE(FlatMeshSDF<T>::distanceQuality == Exact);
  STATIC_REQUIRE(MeshSDF<T, 4>::distanceQuality == Exact);
  STATIC_REQUIRE(TriMeshSDF<T, 4, 4>::distanceQuality == Exact);

  // A union is exact outside its primitives but only a bound inside where they overlap, and only a
  // sign if its primitives are.
  STATIC_REQUIRE(BVHUnion<T, 4, SphereSDF<T>>::distanceQuality == DistanceQuality::Bound);
  STATIC_REQUIRE(BVHSmoothUnion<T, 4, SmoothMinOp<T>, SphereSDF<T>>::distanceQuality == DistanceQuality::Bound);
  STATIC_REQUIRE(BVHUnion<T, 4, PerlinSDF<T>>::distanceQuality == DistanceQuality::NotADistance);
  STATIC_REQUIRE(BVHSmoothUnion<T, 4, SmoothMinOp<T>, PerlinSDF<T>>::distanceQuality == DistanceQuality::NotADistance);

  // The trait reads the member, and takes a type without one to be a bound.
  STATIC_REQUIRE(distanceQualityOf<SphereSDF<T>> == Exact);
  STATIC_REQUIRE(distanceQualityOf<PerlinSDF<T>> == DistanceQuality::NotADistance);
  STATIC_REQUIRE(distanceQualityOf<NoQualityDeclared> == DistanceQuality::Bound);
}

TEMPLATE_TEST_CASE("CapsuleSDF: tips exactly two radii apart give a sphere, not NaN",
                   "[CapsuleSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const CapsuleSDF<T> capsule(Vec3(T(0), T(0), T(-0.5)), Vec3(T(0), T(0), T(0.5)), T(0.5));
  const SphereSDF<T>  sphere(Vec3::zeros(), T(0.5));

  for (const auto& p : {Vec3::zeros(), Vec3(T(1), T(0), T(0)), Vec3(T(0.2), T(-0.3), T(0.9))}) {
    REQUIRE_THAT(capsule.signedDistance(p), WithinAbs(double(sphere.signedDistance(p)), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PerlinSDF: the default constructor and zero persistence give defined noise",
                   "[PerlinSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // The default constructor is the full constructor with the documented defaults.
  const PerlinSDF<T> byDefault;
  const PerlinSDF<T> explicitDefaults(T(1), Vec3::ones(), T(0.5), 1U);

  // Zero persistence leaves every octave after the first with zero amplitude; it must not divide
  // the frequency by zero.
  const PerlinSDF<T> zeroPersistence(T(1), Vec3::ones(), T(0), 3U);
  const PerlinSDF<T> oneOctave(T(1), Vec3::ones(), T(0.5), 1U);

  for (const auto& p : {Vec3(T(0.3), T(0.6), T(0.1)), Vec3(T(0.7), T(0.2), T(0.9)), Vec3(T(-1.3), T(2.1), T(0.4))}) {
    REQUIRE(byDefault.signedDistance(p) == explicitDefaults.signedDistance(p));
    REQUIRE(std::isfinite(zeroPersistence.signedDistance(p)));
    REQUIRE_THAT(zeroPersistence.signedDistance(p), WithinAbs(double(oneOctave.signedDistance(p)), looseMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor argument checks (EBGEOMETRY_REQUIRE, on in every build)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Analytic shapes: invalid constructor arguments abort with a message", "[AnalyticSDF][death]")
{
  using T    = double;
  using Vec3 = Vec3T<T>;

  REQUIRE(abortsWith(
    [] {
      const PlaneSDF<T> plane(Vec3(0, 0, 0), Vec3(0, 0, 0));

      (void)plane;
    },
    "PlaneSDF: the normal must be nonzero (0, 0, 0)"));

  REQUIRE(abortsWith(
    [] {
      const SphereSDF<T> sphere(Vec3(0, 0, 0), T(-1));

      (void)sphere;
    },
    "SphereSDF: the radius must be finite and positive (-1)"));

  REQUIRE(abortsWith(
    [] {
      const BoxSDF<T> box(Vec3(0, 0, 0), Vec3(1, -1, 1));

      (void)box;
    },
    "BoxSDF: the low corner (0, 0, 0) must be below the high corner (1, -1, 1) in every direction"));

  REQUIRE(abortsWith(
    [] {
      const TorusSDF<T> torus(Vec3(0, 0, 0), T(1), T(2));

      (void)torus;
    },
    "TorusSDF: the minor radius (2) must be less than the major radius (1)"));

  REQUIRE(abortsWith(
    [] {
      const CylinderSDF<T> cylinder(Vec3(1, 2, 3), Vec3(1, 2, 3), T(1));

      (void)cylinder;
    },
    "CylinderSDF: the two centers must differ, but both are (1, 2, 3)"));

  REQUIRE(abortsWith(
    [] {
      const InfiniteCylinderSDF<T> cylinder(Vec3(0, 0, 0), T(1), 3);

      (void)cylinder;
    },
    "InfiniteCylinderSDF: the axis must be 0, 1 or 2 (3)"));

  REQUIRE(abortsWith(
    [] {
      const CapsuleSDF<T> capsule(Vec3(0, 0, 0), Vec3(0, 1, 0), T(1));

      (void)capsule;
    },
    "CapsuleSDF: the tips must be at least two radii apart (distance 1, radius 1)"));

  REQUIRE(abortsWith(
    [] {
      const InfiniteConeSDF<T> cone(Vec3(0, 0, 0), T(180));

      (void)cone;
    },
    "InfiniteConeSDF: the angle must be in (0, 180) degrees (180)"));

  REQUIRE(abortsWith(
    [] {
      const ConeSDF<T> cone(Vec3(0, 0, 0), T(0), T(30));

      (void)cone;
    },
    "ConeSDF: the height must be finite and positive (0)"));

  REQUIRE(abortsWith(
    [] {
      const RoundedBoxSDF<T> box(Vec3(0, 0, 0), Vec3(1, 0.2, 1), T(0.1));

      (void)box;
    },
    "RoundedBoxSDF: the size (1, 0.2, 1) must be finite and exceed twice the curvature (0.1)"));

  REQUIRE(abortsWith(
    [] {
      const PerlinSDF<T> perlin(std::numeric_limits<T>::infinity(), Vec3::ones(), T(0.5), 1U);

      (void)perlin;
    },
    "PerlinSDF: the noise amplitude must be finite (inf)"));

  REQUIRE(abortsWith(
    [] {
      const RoundedCylinderSDF<T> cylinder(Vec3(0, 0, 0), T(1), T(0.5), T(1));

      (void)cylinder;
    },
    "RoundedCylinderSDF: twice the curvature (0.5) must be less than the height (1)"));
  REQUIRE(abortsWith(
    [] {
      const EllipsoidSDF<T> e(Vec3(0, 0, 0), Vec3(1, 0, 1));

      (void)e;
    },
    "EllipsoidSDF: the radii must be finite and positive (1, 0, 1)"));

  REQUIRE(abortsWith(
    [] {
      const CappedConeSDF<T> c(Vec3(0, 0, 0), Vec3(0, 1, 0), T(0), T(0));

      (void)c;
    },
    "CappedConeSDF: the radii must be finite, at least 0 and not both 0 (0, 0)"));

  REQUIRE(abortsWith(
    [] {
      const CappedConeSDF<T> c(Vec3(1, 2, 3), Vec3(1, 2, 3), T(1), T(0));

      (void)c;
    },
    "CappedConeSDF: the two centers must differ, but both are (1, 2, 3)"));

  REQUIRE(abortsWith(
    [] {
      const RoundConeSDF<T> c(Vec3(0, 0, 0), Vec3(0, 1, 0), T(2), T(0.5));

      (void)c;
    },
    "RoundConeSDF: the difference of the radii (2, 0.5) must be less than the distance between the centers (1)"));

  REQUIRE(abortsWith(
    [] {
      const PolygonPrismSDF<T> p(Vec3(0, 0, 0), Vec3(0, 1, 0), T(1), 2U, Vec3(1, 0, 0));

      (void)p;
    },
    "PolygonPrismSDF: the number of sides must be at least 3 (2)"));

  REQUIRE(abortsWith(
    [] {
      const PolygonPrismSDF<T> p(Vec3(0, 0, 0), Vec3(0, 1, 0), T(1), 6U, Vec3(0, 2, 0));

      (void)p;
    },
    "PolygonPrismSDF: the vertex direction (0, 2, 0) must have a part across the axis"));

  REQUIRE(abortsWith(
    [] {
      const TubeSDF<T> t(Vec3(0, 0, 0), Vec3(0, 1, 0), T(1), T(0.5));

      (void)t;
    },
    "TubeSDF: the radii must satisfy 0 < inner (1) < outer (0.5)"));

  REQUIRE(abortsWith(
    [] {
      const CappedTorusSDF<T> t(Vec3(0, 0, 0), T(1), T(0.5), T(400));

      (void)t;
    },
    "CappedTorusSDF: the angle must be in (0, 360] degrees (400)"));

  REQUIRE(abortsWith(
    [] {
      const CappedTorusSDF<T> t(Vec3(0, 0, 0), T(1), T(1), T(90));

      (void)t;
    },
    "CappedTorusSDF: the radii must satisfy 0 < minor (1) < major (1)"));

  REQUIRE(abortsWith(
    [] {
      const CutSphereSDF<T> c(Vec3(0, 0, 0), T(1), T(1));

      (void)c;
    },
    "CutSphereSDF: the height (1) must be between minus and plus the radius (1)"));

  REQUIRE(abortsWith(
    [] {
      const CutHollowSphereSDF<T> c(Vec3(0, 0, 0), T(1), T(0), T(2));

      (void)c;
    },
    "CutHollowSphereSDF: the thickness (2) must be positive and less than twice the radius (1)"));

  REQUIRE(abortsWith(
    [] {
      const WedgeSDF<T> w(Vec3(0, 0, 0), T(180));

      (void)w;
    },
    "WedgeSDF: the angle must be in (0, 180) degrees (180)"));
}

namespace {

// One signed distance per query point, for any one of the analytic shapes. The shape is held by value,
// as a kernel receives it, which is exactly what being trivially copyable is for.
template <class T, class Shape>
struct ShapeDistanceQuery
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }
};

} // namespace

TEMPLATE_TEST_CASE("Analytic shapes: device signedDistance matches the host",
                   "[AnalyticSDF][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const AllShapes<T> shapes;

  // The shapes are centered at (or anchored to) the origin and extend at most ~1.1 from it, so this grid
  // covers points inside, outside and near every surface, including the thin (radius 0.1) tubes.
  // PerlinSDF is a positive noise field with no surface, so it is simply sampled over the same box.
  const auto points = queryGrid<T>(Vec3T<T>(T(-1.5), T(-1.5), T(-1.5)), Vec3T<T>(T(1.5), T(1.5), T(1.5)), 15);

  const auto check = [&](const auto& a_shape, const char* a_name) {
    using Shape = std::decay_t<decltype(a_shape)>;

    INFO(a_name);
    requireSameResults(evaluateOnDevice<T>(ShapeDistanceQuery<T, Shape>{a_shape}, points),
                       evaluateOnHost<T>(ShapeDistanceQuery<T, Shape>{a_shape}, points));
  };

  check(shapes.plane, "PlaneSDF");
  check(shapes.sphere, "SphereSDF");
  check(shapes.box, "BoxSDF");
  check(shapes.torus, "TorusSDF");
  check(shapes.cylinder, "CylinderSDF");
  check(shapes.infiniteCylinder, "InfiniteCylinderSDF");
  check(shapes.capsule, "CapsuleSDF");
  check(shapes.infiniteCone, "InfiniteConeSDF");
  check(shapes.cone, "ConeSDF");
  check(shapes.roundedBox, "RoundedBoxSDF");
  check(shapes.perlin, "PerlinSDF");
  check(shapes.roundedCylinder, "RoundedCylinderSDF");
  check(shapes.ellipsoid, "EllipsoidSDF");
  check(shapes.cappedCone, "CappedConeSDF");
  check(shapes.roundCone, "RoundConeSDF");
  check(shapes.polygonPrism, "PolygonPrismSDF");
  check(shapes.tube, "TubeSDF");
  check(shapes.cappedTorus, "CappedTorusSDF");
  check(shapes.cutSphere, "CutSphereSDF");
  check(shapes.cutHollowSphere, "CutHollowSphereSDF");
  check(shapes.wedge, "WedgeSDF");
}

namespace {

// One bounding-box coordinate per query: 0-2 are the low corner, 3-5 the high corner.
template <class T, class Shape>
struct ShapeBoxQuery
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const int& a_coordinate) const noexcept
  {
    const BoundingVolumes::AABBT<T> box = m_shape.computeBoundingVolume();

    return a_coordinate < 3 ? box.getLowCorner()[size_t(a_coordinate)] : box.getHighCorner()[size_t(a_coordinate - 3)];
  }
};

} // namespace

TEMPLATE_TEST_CASE("Analytic shapes: device computeBoundingVolume matches the host",
                   "[AnalyticSDF][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const AllShapes<T>     shapes;
  const std::vector<int> coordinates{0, 1, 2, 3, 4, 5};

  const auto check = [&](const auto& a_shape, const char* a_name) {
    using Shape = std::decay_t<decltype(a_shape)>;

    INFO(a_name);
    requireSameResults(evaluateOnDevice<T>(ShapeBoxQuery<T, Shape>{a_shape}, coordinates),
                       evaluateOnHost<T>(ShapeBoxQuery<T, Shape>{a_shape}, coordinates));
  };

  check(shapes.plane, "PlaneSDF");
  check(shapes.sphere, "SphereSDF");
  check(shapes.box, "BoxSDF");
  check(shapes.torus, "TorusSDF");
  check(shapes.cylinder, "CylinderSDF");
  check(shapes.infiniteCylinder, "InfiniteCylinderSDF");
  check(shapes.capsule, "CapsuleSDF");
  check(shapes.infiniteCone, "InfiniteConeSDF");
  check(shapes.cone, "ConeSDF");
  check(shapes.roundedBox, "RoundedBoxSDF");
  check(shapes.perlin, "PerlinSDF");
  check(shapes.roundedCylinder, "RoundedCylinderSDF");
  check(shapes.ellipsoid, "EllipsoidSDF");
  check(shapes.cappedCone, "CappedConeSDF");
  check(shapes.roundCone, "RoundConeSDF");
  check(shapes.polygonPrism, "PolygonPrismSDF");
  check(shapes.tube, "TubeSDF");
  check(shapes.cappedTorus, "CappedTorusSDF");
  check(shapes.cutSphere, "CutSphereSDF");
  check(shapes.cutHollowSphere, "CutHollowSphereSDF");
  check(shapes.wedge, "WedgeSDF");
}
