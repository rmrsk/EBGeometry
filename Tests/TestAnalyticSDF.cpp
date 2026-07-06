// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <cmath>
#include <memory>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// ─────────────────────────────────────────────────────────────────────────────
// SphereSDF
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("SphereSDF: exact signed distances", "[SphereSDF]")
{
  const Vec3T<double>     center(0, 0, 0);
  const SphereSDF<double> sphere(center, 1.0);

  // On the surface
  REQUIRE_THAT(sphere.signedDistance(Vec3T<double>(1, 0, 0)), WithinAbs(0.0, 1e-12));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<double>(-1, 0, 0)), WithinAbs(0.0, 1e-12));

  // Outside: positive distance
  REQUIRE_THAT(sphere.signedDistance(Vec3T<double>(2, 0, 0)), WithinRel(1.0));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<double>(0, 3, 0)), WithinRel(2.0));

  // Inside: negative distance
  REQUIRE(sphere.signedDistance(Vec3T<double>(0, 0, 0)) < 0.0);
  REQUIRE_THAT(sphere.signedDistance(Vec3T<double>(0, 0, 0)), WithinRel(-1.0));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<double>(0.5, 0, 0)), WithinRel(-0.5));
}

TEST_CASE("SphereSDF: off-centre sphere", "[SphereSDF]")
{
  SphereSDF<double> sphere(Vec3T<double>(1, 2, 3), 2.0);

  // Point at centre
  REQUIRE_THAT(sphere.signedDistance(Vec3T<double>(1, 2, 3)), WithinRel(-2.0));

  // Point on surface along x
  REQUIRE_THAT(sphere.signedDistance(Vec3T<double>(3, 2, 3)), WithinAbs(0.0, 1e-12));

  // Getters
  REQUIRE(sphere.getCenter()[0] == 1.0);
  REQUIRE(sphere.getRadius() == 2.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// PlaneSDF
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("PlaneSDF: signed distances from xy-plane", "[PlaneSDF]")
{
  // Plane z = 0 with outward normal +z
  const PlaneSDF<double> plane(Vec3T<double>(0, 0, 0), Vec3T<double>(0, 0, 1));

  REQUIRE_THAT(plane.signedDistance(Vec3T<double>(0, 0, 3)), WithinRel(3.0));
  REQUIRE_THAT(plane.signedDistance(Vec3T<double>(0, 0, -2)), WithinRel(-2.0));
  REQUIRE_THAT(plane.signedDistance(Vec3T<double>(5, 7, 0)), WithinAbs(0.0, 1e-12));
}

TEST_CASE("PlaneSDF: non-unit normal is normalised", "[PlaneSDF]")
{
  // Normal (1,1,0): should be normalised internally
  const PlaneSDF<double> plane(Vec3T<double>(0, 0, 0), Vec3T<double>(2, 0, 0));

  // Point at (3, 0, 0): distance = 3 (along +x)
  REQUIRE_THAT(plane.signedDistance(Vec3T<double>(3, 0, 0)), WithinRel(3.0));
}

// ─────────────────────────────────────────────────────────────────────────────
// BoxSDF
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("BoxSDF: axis-aligned unit box", "[BoxSDF]")
{
  const BoxSDF<double> box(Vec3T<double>(-1, -1, -1), Vec3T<double>(1, 1, 1));

  // Inside: negative SDF
  REQUIRE(box.signedDistance(Vec3T<double>(0, 0, 0)) < 0.0);
  REQUIRE_THAT(box.signedDistance(Vec3T<double>(0, 0, 0)), WithinRel(-1.0));

  // On a face: zero SDF
  REQUIRE_THAT(box.signedDistance(Vec3T<double>(1, 0, 0)), WithinAbs(0.0, 1e-12));

  // Outside along one axis: positive SDF
  REQUIRE_THAT(box.signedDistance(Vec3T<double>(3, 0, 0)), WithinRel(2.0));

  // Outside near a corner
  // Point (2, 2, 0): distance = sqrt(1+1) = sqrt(2)
  REQUIRE_THAT(box.signedDistance(Vec3T<double>(2, 2, 0)), WithinRel(std::sqrt(2.0), 1e-6));
}

// ─────────────────────────────────────────────────────────────────────────────
// CylinderSDF (defined by two end-cap centres + radius)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("CylinderSDF: unit cylinder along z-axis", "[CylinderSDF]")
{
  // Cylinder from (0,0,-1) to (0,0,1), radius 1
  const CylinderSDF<double> cyl(Vec3T<double>(0, 0, -1), Vec3T<double>(0, 0, 1), 1.0);

  // Centre of cylinder is inside
  REQUIRE(cyl.signedDistance(Vec3T<double>(0, 0, 0)) < 0.0);

  // Point on the curved surface
  REQUIRE_THAT(cyl.signedDistance(Vec3T<double>(1, 0, 0)), WithinAbs(0.0, 1e-10));

  // Outside radially
  REQUIRE(cyl.signedDistance(Vec3T<double>(3, 0, 0)) > 0.0);

  // Outside axially beyond cap
  REQUIRE(cyl.signedDistance(Vec3T<double>(0, 0, 3)) > 0.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// TorusSDF
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("TorusSDF: point on surface", "[TorusSDF]")
{
  // Torus centred at origin in xy-plane: major radius 2, minor radius 0.5
  const TorusSDF<double> torus(Vec3T<double>(0, 0, 0), 2.0, 0.5);

  // Point on surface: along +x from tube centre (major_radius + minor_radius, 0, 0)
  REQUIRE_THAT(torus.signedDistance(Vec3T<double>(2.5, 0, 0)), WithinAbs(0.0, 1e-10));

  // Point inside the tube
  REQUIRE(torus.signedDistance(Vec3T<double>(2.0, 0, 0)) < 0.0);

  // Point at the origin: outside the torus
  REQUIRE(torus.signedDistance(Vec3T<double>(0, 0, 0)) > 0.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// CSG: Union and Complement
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("CSG Union of two spheres", "[CSG][Union]")
{
  using IF = ImplicitFunction<double>;

  auto s1 = std::make_shared<SphereSDF<double>>(Vec3T<double>(-2, 0, 0), 1.0);
  auto s2 = std::make_shared<SphereSDF<double>>(Vec3T<double>(2, 0, 0), 1.0);

  auto u = Union<double>(std::vector<std::shared_ptr<IF>>{s1, s2});

  // Points inside either sphere are inside the union
  REQUIRE(u->value(Vec3T<double>(-2, 0, 0)) < 0.0);
  REQUIRE(u->value(Vec3T<double>(2, 0, 0)) < 0.0);

  // Point between the spheres is outside both
  REQUIRE(u->value(Vec3T<double>(0, 0, 0)) > 0.0);

  // Point far away is outside
  REQUIRE(u->value(Vec3T<double>(100, 0, 0)) > 0.0);
}

TEST_CASE("CSG Complement of sphere inverts sign", "[CSG][Complement]")
{
  auto sphere = std::make_shared<SphereSDF<double>>(Vec3T<double>(0, 0, 0), 1.0);
  auto comp   = Complement<double>(sphere);

  // Inside sphere → positive after complement
  REQUIRE(comp->value(Vec3T<double>(0, 0, 0)) > 0.0);

  // Outside sphere → negative after complement
  REQUIRE(comp->value(Vec3T<double>(5, 0, 0)) < 0.0);
}

TEST_CASE("CSG Intersection of two overlapping spheres", "[CSG][Intersection]")
{
  using IF = ImplicitFunction<double>;

  // Two unit spheres 1 unit apart — they overlap in the middle
  auto s1 = std::make_shared<SphereSDF<double>>(Vec3T<double>(-0.5, 0, 0), 1.0);
  auto s2 = std::make_shared<SphereSDF<double>>(Vec3T<double>(0.5, 0, 0), 1.0);

  auto inter = Intersection<double>(std::vector<std::shared_ptr<IF>>{s1, s2});

  // Origin is inside both spheres → inside the intersection
  REQUIRE(inter->value(Vec3T<double>(0, 0, 0)) < 0.0);

  // Far left: inside s1, outside s2 → outside intersection
  REQUIRE(inter->value(Vec3T<double>(-2, 0, 0)) > 0.0);
}
