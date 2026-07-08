// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <cmath>
#include <memory>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

namespace {

// WithinAbs has no type-specific default margin (unlike WithinRel), so exact-zero comparisons
// that are only "exact" up to a genuine floating-point computation (a sqrt, not a perfect square)
// scale their own margin by the precision in use.
template <class T>
double
looseMargin()
{
  return static_cast<double>(T(1.0e5) * std::numeric_limits<T>::epsilon());
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// SphereSDF
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SphereSDF: exact signed distances", "[SphereSDF]", float, double)
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

TEMPLATE_TEST_CASE("SphereSDF: off-centre sphere", "[SphereSDF]", float, double)
{
  using T = TestType;

  SphereSDF<T> sphere(Vec3T<T>(1, 2, 3), T(2.0));

  // Point at centre
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

TEMPLATE_TEST_CASE("PlaneSDF: signed distances from xy-plane", "[PlaneSDF]", float, double)
{
  using T = TestType;

  // Plane z = 0 with outward normal +z
  const PlaneSDF<T> plane(Vec3T<T>(0, 0, 0), Vec3T<T>(0, 0, 1));

  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(0, 0, 3)), WithinRel(T(3.0)));
  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(0, 0, -2)), WithinRel(T(-2.0)));
  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(5, 7, 0)), WithinAbs(0.0, looseMargin<T>()));
}

TEMPLATE_TEST_CASE("PlaneSDF: non-unit normal is normalised", "[PlaneSDF]", float, double)
{
  using T = TestType;

  // Normal (1,1,0): should be normalised internally
  const PlaneSDF<T> plane(Vec3T<T>(0, 0, 0), Vec3T<T>(2, 0, 0));

  // Point at (3, 0, 0): distance = 3 (along +x)
  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(3, 0, 0)), WithinRel(T(3.0)));
}

// ─────────────────────────────────────────────────────────────────────────────
// BoxSDF
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("BoxSDF: axis-aligned unit box", "[BoxSDF]", float, double)
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
// CylinderSDF (defined by two end-cap centres + radius)
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("CylinderSDF: unit cylinder along z-axis", "[CylinderSDF]", float, double)
{
  using T = TestType;

  // Cylinder from (0,0,-1) to (0,0,1), radius 1
  const CylinderSDF<T> cyl(Vec3T<T>(0, 0, -1), Vec3T<T>(0, 0, 1), T(1.0));

  // Centre of cylinder is inside
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

TEMPLATE_TEST_CASE("TorusSDF: point on surface", "[TorusSDF]", float, double)
{
  using T = TestType;

  // Torus centred at origin in xy-plane: major radius 2, minor radius 0.5
  const TorusSDF<T> torus(Vec3T<T>(0, 0, 0), T(2.0), T(0.5));

  // Point on surface: along +x from tube centre (major_radius + minor_radius, 0, 0)
  REQUIRE_THAT(torus.signedDistance(Vec3T<T>(2.5, 0, 0)), WithinAbs(0.0, looseMargin<T>()));

  // Point inside the tube
  REQUIRE(torus.signedDistance(Vec3T<T>(2.0, 0, 0)) < T(0.0));

  // Point at the origin: outside the torus
  REQUIRE(torus.signedDistance(Vec3T<T>(0, 0, 0)) > T(0.0));
}

// ─────────────────────────────────────────────────────────────────────────────
// CSG: Union and Complement
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("CSG Union of two spheres", "[CSG][Union]", float, double)
{
  using T  = TestType;
  using IF = ImplicitFunction<T>;

  auto s1 = std::make_shared<SphereSDF<T>>(Vec3T<T>(-2, 0, 0), T(1.0));
  auto s2 = std::make_shared<SphereSDF<T>>(Vec3T<T>(2, 0, 0), T(1.0));

  auto u = Union<T>(std::vector<std::shared_ptr<IF>>{s1, s2});

  // Points inside either sphere are inside the union
  REQUIRE(u->value(Vec3T<T>(-2, 0, 0)) < T(0.0));
  REQUIRE(u->value(Vec3T<T>(2, 0, 0)) < T(0.0));

  // Point between the spheres is outside both
  REQUIRE(u->value(Vec3T<T>(0, 0, 0)) > T(0.0));

  // Point far away is outside
  REQUIRE(u->value(Vec3T<T>(100, 0, 0)) > T(0.0));
}

TEMPLATE_TEST_CASE("CSG Complement of sphere inverts sign", "[CSG][Complement]", float, double)
{
  using T = TestType;

  auto sphere = std::make_shared<SphereSDF<T>>(Vec3T<T>(0, 0, 0), T(1.0));
  auto comp   = Complement<T>(sphere);

  // Inside sphere → positive after complement
  REQUIRE(comp->value(Vec3T<T>(0, 0, 0)) > T(0.0));

  // Outside sphere → negative after complement
  REQUIRE(comp->value(Vec3T<T>(5, 0, 0)) < T(0.0));
}

TEMPLATE_TEST_CASE("CSG Intersection of two overlapping spheres", "[CSG][Intersection]", float, double)
{
  using T  = TestType;
  using IF = ImplicitFunction<T>;

  // Two unit spheres 1 unit apart — they overlap in the middle
  auto s1 = std::make_shared<SphereSDF<T>>(Vec3T<T>(-0.5, 0, 0), T(1.0));
  auto s2 = std::make_shared<SphereSDF<T>>(Vec3T<T>(0.5, 0, 0), T(1.0));

  auto inter = Intersection<T>(std::vector<std::shared_ptr<IF>>{s1, s2});

  // Origin is inside both spheres → inside the intersection
  REQUIRE(inter->value(Vec3T<T>(0, 0, 0)) < T(0.0));

  // Far left: inside s1, outside s2 → outside intersection
  REQUIRE(inter->value(Vec3T<T>(-2, 0, 0)) > T(0.0));
}
