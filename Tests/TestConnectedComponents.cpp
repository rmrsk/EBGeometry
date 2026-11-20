// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test suite for connectedComponents(): the components of a tape's regions, of groups of them and of a domain's
// subdomains, against models whose components are known by construction: a shell with a cavity, two spheres apart and
// overlapping, a bar cut into three, two balls joined by a neck below and above the resolution, and the rejected
// inputs.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

// The volume of a ball.
template <class T>
T
ballVolume(const T a_radius)
{
  return T(4.0 / 3.0 * M_PI) * a_radius * a_radius * a_radius;
}

} // namespace

TEMPLATE_TEST_CASE("ConnectedComponents: a hollow shell's cavity is an enclosed component of the outside",
                   "[ConnectedComponents]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const Tape<T> tape(pool, Difference(SphereSDF<T>(Vec3::zeros(), T(1)), SphereSDF<T>(Vec3::zeros(), T(0.5))));

  const auto components = connectedComponents(tape, ComponentOptions<T>{T(0.05)});

  REQUIRE(components.getNumGroups() == 2U);
  REQUIRE(components.getNumComponents() == 3U);

  // The outside: the exterior, reaching the box, then the cavity, enclosed.
  const std::vector<uint32_t> outside = components.componentsOf(0U);

  REQUIRE(outside.size() == 2U);

  const ConnectedComponent<T>& exterior = components.component(outside[0]);
  const ConnectedComponent<T>& cavity   = components.component(outside[1]);

  CHECK(exterior.m_reachesBox);
  CHECK_FALSE(cavity.m_reachesBox);
  CHECK(cavity.m_seed.length() < T(0.5));
  CHECK_THAT(cavity.m_volume, Catch::Matchers::WithinRel(ballVolume(T(0.5)), T(0.15)));

  // The shell, one component, touching both.
  const std::vector<uint32_t> shell = components.componentsOf(1U);

  REQUIRE(shell.size() == 1U);

  const ConnectedComponent<T>& solid = components.component(shell[0]);

  CHECK_FALSE(solid.m_reachesBox);
  CHECK_THAT(solid.m_volume, Catch::Matchers::WithinRel(ballVolume(T(1)) - ballVolume(T(0.5)), T(0.1)));
  CHECK(solid.m_seed.length() > T(0.5));
  CHECK(solid.m_seed.length() < T(1));
  CHECK(components.neighbors(shell[0]) == outside);
  CHECK(components.neighbors(outside[1]) == shell);

  // The box of the cavity is within h of the inner sphere's.
  for (size_t d = 0; d < 3; d++) {
    CHECK(std::abs(cavity.m_box.getLowCorner()[d] + T(0.5)) <= T(0.05));
    CHECK(std::abs(cavity.m_box.getHighCorner()[d] - T(0.5)) <= T(0.05));
  }

  // Points far from the surfaces are in their components; beyond the box in none.
  CHECK(components.componentAt(Vec3::zeros()) == outside[1]);
  CHECK(components.componentAt(Vec3(T(0.75), T(0), T(0))) == shell[0]);
  CHECK(components.componentAt(Vec3(T(1.05), T(1.05), T(0))) == outside[0]);
  CHECK(components.componentAt(Vec3(T(5), T(0), T(0))) == ConnectedComponents<T>::None);

  // The default box: the tape's grown by two cells, to a whole number of cells.
  CHECK(components.box().getLowCorner()[0] <= T(-1.1) + T(1e-4));
  CHECK(components.box().getHighCorner()[0] >= T(1.1) - T(1e-4));
  CHECK(components.resolution() == T(0.05));
  CHECK(components.getNumCells() > 0U);
}

TEMPLATE_TEST_CASE("ConnectedComponents: two spheres are one component when they overlap, two when apart",
                   "[ConnectedComponents]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const ComponentOptions<T> options{T(0.05)};

  for (const T offset : {T(0.9), T(1.5)}) {
    const SphereSDF<T> left(Vec3(-offset, T(0), T(0)), T(1));
    const SphereSDF<T> right(Vec3(offset, T(0), T(0)), T(1));

    const bool apart = offset > T(1);

    // One solid, the union.
    const Tape<T> solid(pool, Union(left, right));
    const auto    pieces = connectedComponents(solid, options);

    CHECK(pieces.componentsOf(0U).size() == 1U);
    REQUIRE(pieces.componentsOf(1U).size() == (apart ? 2U : 1U));

    if (apart) {
      const ConnectedComponent<T>& first  = pieces.component(pieces.componentsOf(1U)[0]);
      const ConnectedComponent<T>& second = pieces.component(pieces.componentsOf(1U)[1]);

      // One seed in each ball.
      CHECK(first.m_seed[0] * second.m_seed[0] < T(0));
      CHECK_THAT(first.m_volume, Catch::Matchers::WithinRel(ballVolume(T(1)), T(0.1)));
    }

    // Two regions: one component each, touching each other only where they overlap.
    const Tape<T> regions(pool, Overlay(left, right));
    const auto    each = connectedComponents(regions, options);

    REQUIRE(each.componentsOf(1U).size() == 1U);
    REQUIRE(each.componentsOf(2U).size() == 1U);

    const uint32_t a = each.componentsOf(1U)[0];
    const uint32_t b = each.componentsOf(2U)[0];

    CHECK((each.neighbors(a) == std::vector<uint32_t>{0U, b}) == !apart);
    CHECK((each.neighbors(a) == std::vector<uint32_t>{0U}) == apart);

    // The two regions as one group (one material): the components of the union.
    const auto grouped = connectedComponents(regions, std::vector<uint32_t>{0U, 1U, 1U}, options);

    CHECK(grouped.getNumGroups() == 2U);
    CHECK(grouped.componentsOf(1U).size() == (apart ? 2U : 1U));
  }
}

TEMPLATE_TEST_CASE("ConnectedComponents: a difference cuts a bar into three components",
                   "[ConnectedComponents]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const BoxSDF<T> bar(Vec3(T(-3), T(-1), T(-1)), Vec3(T(3), T(1), T(1)));
  const BoxSDF<T> cut1(Vec3(T(-1.1), T(-2), T(-2)), Vec3(T(-0.9), T(2), T(2)));
  const BoxSDF<T> cut2(Vec3(T(0.9), T(-2), T(-2)), Vec3(T(1.1), T(2), T(2)));

  const Tape<T> tape(pool, Difference(bar, Union(cut1, cut2)));

  const auto components = connectedComponents(tape, ComponentOptions<T>{T(0.05)});

  CHECK(components.componentsOf(0U).size() == 1U);

  const std::vector<uint32_t> pieces = components.componentsOf(1U);

  REQUIRE(pieces.size() == 3U);

  // By decreasing volume: the two ends (1.9 x 2 x 2), then the middle (1.8 x 2 x 2).
  CHECK_THAT(components.component(pieces[0]).m_volume, Catch::Matchers::WithinRel(T(7.6), T(0.05)));
  CHECK_THAT(components.component(pieces[1]).m_volume, Catch::Matchers::WithinRel(T(7.6), T(0.05)));
  CHECK_THAT(components.component(pieces[2]).m_volume, Catch::Matchers::WithinRel(T(7.2), T(0.05)));
  CHECK(std::abs(components.component(pieces[2]).m_seed[0]) < T(0.9));

  for (const uint32_t piece : pieces) {
    CHECK(components.neighbors(piece) == components.componentsOf(0U));
  }
}

TEMPLATE_TEST_CASE("ConnectedComponents: a neck thinner than the resolution may part two balls, a wider one joins them",
                   "[ConnectedComponents]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // h = 1/8 on a box whose grid lines pass through the neck's axis: the centers of the cells nearest the axis are
  // sqrt(2) h / 2 from it.
  ComponentOptions<T> options;

  options.m_resolution = T(0.125);
  options.m_box        = BoundingVolumes::AABBT<T>(Vec3(T(-4), T(-2), T(-2)), Vec3(T(4), T(2), T(2)));

  for (const T radius : {T(0.04), T(0.3)}) {
    const auto dumbbell =
      Union(Union(SphereSDF<T>(Vec3(T(-2), T(0), T(0)), T(1)), SphereSDF<T>(Vec3(T(2), T(0), T(0)), T(1))),
            CylinderSDF<T>(Vec3(T(-2), T(0), T(0)), Vec3(T(2), T(0), T(0)), radius));

    const Tape<T> tape(pool, dumbbell);

    const auto components = connectedComponents(tape, options);

    CHECK(components.componentsOf(1U).size() == (radius < options.m_resolution ? 2U : 1U));
    CHECK(components.box().getLowCorner() == options.m_box.getLowCorner());
    CHECK(components.box().getHighCorner() == options.m_box.getHighCorner());
  }
}

TEMPLATE_TEST_CASE("ConnectedComponents: a domain's subdomains and its excluded regions",
                   "[ConnectedComponents]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  // A core inside a jacket: regions 1 (jacket) and 2 (core).
  const Tape<T> tape(pool, Overlay(SphereSDF<T>(Vec3::zeros(), T(1)), SphereSDF<T>(Vec3::zeros(), T(0.5))));

  const ComponentOptions<T> options{T(0.05)};

  // Computing in the core alone: it floats in the excluded regions, the jacket and the outside, which are one.
  DomainSetup coreOnly;

  (void)coreOnly.addSubdomain({2U});

  const Domain<T> core(pool, tape, coreOnly);
  const auto      inCore = connectedComponents(core, options);

  REQUIRE(inCore.getNumGroups() == 2U);
  REQUIRE(inCore.componentsOf(0U).size() == 1U);
  REQUIRE(inCore.componentsOf(1U).size() == 1U);

  const uint32_t body = inCore.componentsOf(0U)[0];

  CHECK_FALSE(inCore.component(body).m_reachesBox);
  CHECK(inCore.component(inCore.componentsOf(1U)[0]).m_reachesBox);
  CHECK(inCore.neighbors(body) == inCore.componentsOf(1U));

  // Computing in the jacket: the core is an excluded component apart from the outside.
  DomainSetup jacketOnly;

  (void)jacketOnly.addSubdomain({1U});

  const Domain<T> jacket(pool, tape, jacketOnly);
  const auto      inJacket = connectedComponents(jacket, options);

  CHECK(inJacket.componentsOf(0U).size() == 1U);
  REQUIRE(inJacket.componentsOf(1U).size() == 2U);
  CHECK_FALSE(inJacket.component(inJacket.componentsOf(1U)[1]).m_reachesBox);

  // A domain computing everywhere has an Excluded group with no component.
  DomainSetup everywhere;

  (void)everywhere.addSubdomain({0U, 1U, 2U});

  const Domain<T> all(pool, tape, everywhere);
  const auto      inAll = connectedComponents(all, options);

  CHECK(inAll.getNumGroups() == 2U);
  CHECK(inAll.componentsOf(0U).size() == 1U);
  CHECK(inAll.componentsOf(1U).empty());
}

TEMPLATE_TEST_CASE(
  "ConnectedComponents: rejects a value that is not a distance, a bad resolution, unbounded boxes and too many cells",
  "[ConnectedComponents]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  CHECK(abortsWith(
    [] {
      Pool          pool(hostMemoryResource());
      const Tape<T> tape(pool, Union(SphereSDF<T>(Vec3::zeros(), T(1)), PerlinSDF<T>()));

      (void)connectedComponents(tape, ComponentOptions<T>{T(0.1)});
    },
    "connectedComponents: the tape's value is not a distance"));

  CHECK(abortsWith(
    [] {
      Pool          pool(hostMemoryResource());
      const Tape<T> tape(pool, Expr<T>(SphereSDF<T>(Vec3::zeros(), T(1))));

      (void)connectedComponents(tape, ComponentOptions<T>{T(0)});
    },
    "connectedComponents: the resolution must be positive"));

  CHECK(abortsWith(
    [] {
      Pool          pool(hostMemoryResource());
      const Tape<T> tape(pool, Expr<T>(PlaneSDF<T>(Vec3::zeros(), Vec3(T(0), T(0), T(1)))));

      (void)connectedComponents(tape, ComponentOptions<T>{T(0.1)});
    },
    "connectedComponents: the box searched must be bounded"));

  CHECK(abortsWith(
    [] {
      Pool          pool(hostMemoryResource());
      const Tape<T> tape(pool, Expr<T>(SphereSDF<T>(Vec3::zeros(), T(1))));

      (void)connectedComponents(tape, std::vector<uint32_t>{0U}, ComponentOptions<T>{T(0.1)});
    },
    "connectedComponents: 1 groups given for 2 regions"));

  CHECK(abortsWith(
    [] {
      Pool          pool(hostMemoryResource());
      const Tape<T> tape(pool, Expr<T>(SphereSDF<T>(Vec3::zeros(), T(1))));

      ComponentOptions<T> capped;

      capped.m_resolution = T(0.125);
      capped.m_maxCells   = 10U;

      (void)connectedComponents(tape, capped);
    },
    "connectedComponents: the search at resolution 0.125 reached its cap of 10 cells (ComponentOptions::m_maxCells)"));

  // An unbounded object with a box given is searched in it.
  Pool          pool(hostMemoryResource());
  const Tape<T> halfSpace(pool, Expr<T>(PlaneSDF<T>(Vec3::zeros(), Vec3(T(0), T(0), T(1)))));

  ComponentOptions<T> options;

  options.m_resolution = T(0.25);
  options.m_box        = BoundingVolumes::AABBT<T>(Vec3(T(-1), T(-1), T(-1)), Vec3(T(1), T(1), T(1)));

  const auto components = connectedComponents(halfSpace, options);

  REQUIRE(components.componentsOf(1U).size() == 1U);
  CHECK(components.component(components.componentsOf(1U)[0]).m_reachesBox);
  CHECK_THAT(components.component(components.componentsOf(1U)[0]).m_volume, Catch::Matchers::WithinRel(T(4), T(1e-3)));
}
