// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <cmath>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinAbs;

template <class T>
using Pnt = Point<T, short>;

TEMPLATE_TEST_CASE("Point: full constructor sets position and metadata", "[Point]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3   pos(1.0, 2.0, 3.0);
  const Pnt<T> p(pos, short(42));

  REQUIRE_THAT(p.getPosition()[0], WithinAbs(1.0, tightMargin<T>()));
  REQUIRE_THAT(p.getPosition()[1], WithinAbs(2.0, tightMargin<T>()));
  REQUIRE_THAT(p.getPosition()[2], WithinAbs(3.0, tightMargin<T>()));
  REQUIRE(p.getMetaData() == short(42));
}

TEMPLATE_TEST_CASE("Point: setPosition/setMetaData update independently", "[Point]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pnt<T> p(Vec3(0, 0, 0), short(1));

  p.setPosition(Vec3(5, 6, 7));
  REQUIRE_THAT(p.getPosition()[0], WithinAbs(5.0, tightMargin<T>()));
  REQUIRE(p.getMetaData() == short(1)); // Unaffected by setPosition().

  p.setMetaData(short(2));
  REQUIRE(p.getMetaData() == short(2));
  REQUIRE_THAT(p.getPosition()[0], WithinAbs(5.0, tightMargin<T>())); // Unaffected by setMetaData().
}

TEMPLATE_TEST_CASE("Point: getDistance/getDistance2 agree with a direct Euclidean computation",
                   "[Point]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Pnt<T> p(Vec3(1, 2, 3), short(0));

  const std::vector<Vec3> queryPoints = {
    Vec3(1, 2, 3), // coincident: distance zero
    Vec3(4, 2, 3), // outside along one axis
    Vec3(-5, -5, -5),
  };

  for (const auto& q : queryPoints) {
    const T expected2 = (q - Vec3(1, 2, 3)).length2();

    REQUIRE_THAT(p.getDistance2(q), WithinAbs(static_cast<double>(expected2), tightMargin<T>()));
    REQUIRE_THAT(p.getDistance(q), WithinAbs(static_cast<double>(std::sqrt(expected2)), tightMargin<T>()));
  }
}
