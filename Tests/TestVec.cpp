// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinRel;

// ─────────────────────────────────────────────────────────────────────────────
// Vec3T
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Vec3T: default construction is zero", "[Vec3T]")
{
  Vec3T<double> v;
  REQUIRE(v[0] == 0.0);
  REQUIRE(v[1] == 0.0);
  REQUIRE(v[2] == 0.0);
}

TEST_CASE("Vec3T: component construction fills correctly", "[Vec3T]")
{
  Vec3T<double> v(7.0, 7.0, 7.0);
  REQUIRE(v[0] == 7.0);
  REQUIRE(v[1] == 7.0);
  REQUIRE(v[2] == 7.0);
}

TEST_CASE("Vec3T: component construction", "[Vec3T]")
{
  Vec3T<double> v(1.0, 2.0, 3.0);
  REQUIRE(v[0] == 1.0);
  REQUIRE(v[1] == 2.0);
  REQUIRE(v[2] == 3.0);
}

TEST_CASE("Vec3T: static factories", "[Vec3T]")
{
  auto z = Vec3T<double>::zero();
  REQUIRE(z[0] == 0.0);
  REQUIRE(z[1] == 0.0);
  REQUIRE(z[2] == 0.0);

  auto o = Vec3T<double>::one();
  REQUIRE(o[0] == 1.0);
  REQUIRE(o[1] == 1.0);
  REQUIRE(o[2] == 1.0);

  auto ex = Vec3T<double>::unit(0);
  REQUIRE(ex[0] == 1.0);
  REQUIRE(ex[1] == 0.0);
  REQUIRE(ex[2] == 0.0);

  auto ey = Vec3T<double>::unit(1);
  REQUIRE(ey[1] == 1.0);

  auto ez = Vec3T<double>::unit(2);
  REQUIRE(ez[2] == 1.0);
}

TEST_CASE("Vec3T: arithmetic operators", "[Vec3T]")
{
  const Vec3T<double> a(1, 2, 3);
  const Vec3T<double> b(4, 5, 6);

  SECTION("addition")
  {
    auto c = a + b;
    REQUIRE(c[0] == 5.0);
    REQUIRE(c[1] == 7.0);
    REQUIRE(c[2] == 9.0);
  }
  SECTION("subtraction")
  {
    auto c = b - a;
    REQUIRE(c[0] == 3.0);
    REQUIRE(c[1] == 3.0);
    REQUIRE(c[2] == 3.0);
  }
  SECTION("negation")
  {
    auto c = -a;
    REQUIRE(c[0] == -1.0);
    REQUIRE(c[1] == -2.0);
    REQUIRE(c[2] == -3.0);
  }
  SECTION("scalar multiply right")
  {
    auto c = a * 3.0;
    REQUIRE(c[0] == 3.0);
    REQUIRE(c[1] == 6.0);
    REQUIRE(c[2] == 9.0);
  }
  SECTION("scalar multiply left")
  {
    auto c = 3.0 * a;
    REQUIRE(c[0] == 3.0);
    REQUIRE(c[1] == 6.0);
    REQUIRE(c[2] == 9.0);
  }
  SECTION("scalar divide")
  {
    auto c = b / 2.0;
    REQUIRE(c[0] == 2.0);
    REQUIRE(c[1] == 2.5);
    REQUIRE(c[2] == 3.0);
  }
  SECTION("scalar / vec  (s/v[i] component-wise)")
  {
    const Vec3T<double> v(1.0, 2.0, 4.0);
    auto                c = 4.0 / v;
    REQUIRE(c[0] == 4.0);
    REQUIRE(c[1] == 2.0);
    REQUIRE(c[2] == 1.0);
  }
}

TEST_CASE("Vec3T: compound assignment operators", "[Vec3T]")
{
  Vec3T<double> v(1, 2, 3);
  v += Vec3T<double>(10, 20, 30);
  REQUIRE(v[0] == 11.0);
  v -= Vec3T<double>(1, 2, 3);
  REQUIRE(v[0] == 10.0);
  v *= 2.0;
  REQUIRE(v[0] == 20.0);
  v /= 4.0;
  REQUIRE(v[0] == 5.0);
}

TEST_CASE("Vec3T: dot product", "[Vec3T]")
{
  const Vec3T<double> x(1, 0, 0);
  const Vec3T<double> y(0, 1, 0);
  const Vec3T<double> d(1, 1, 1);

  REQUIRE(dot(x, y) == 0.0);
  REQUIRE(dot(x, d) == 1.0);
  REQUIRE_THAT(dot(d, d), WithinRel(3.0));
}

TEST_CASE("Vec3T: cross product", "[Vec3T]")
{
  const Vec3T<double> x(1, 0, 0);
  const Vec3T<double> y(0, 1, 0);
  auto                z = x.cross(y);
  REQUIRE(z[0] == 0.0);
  REQUIRE(z[1] == 0.0);
  REQUIRE(z[2] == 1.0);

  // Anti-commutativity
  auto mz = y.cross(x);
  REQUIRE(mz[2] == -1.0);
}

TEST_CASE("Vec3T: length", "[Vec3T]")
{
  const Vec3T<double> v(3, 4, 0);
  REQUIRE_THAT(v.length(), WithinRel(5.0));
  REQUIRE_THAT(v.length2(), WithinRel(25.0));

  const Vec3T<double> unit(1, 0, 0);
  REQUIRE_THAT(unit.length(), WithinRel(1.0));
}

TEST_CASE("Vec3T: component-wise min/max (free functions)", "[Vec3T]")
{
  const Vec3T<double> a(1, 5, 3);
  const Vec3T<double> b(4, 2, 6);

  auto lo = min(a, b);
  REQUIRE(lo[0] == 1.0);
  REQUIRE(lo[1] == 2.0);
  REQUIRE(lo[2] == 3.0);

  auto hi = max(a, b);
  REQUIRE(hi[0] == 4.0);
  REQUIRE(hi[1] == 5.0);
  REQUIRE(hi[2] == 6.0);
}

TEST_CASE("Vec3T: minDir and maxDir", "[Vec3T]")
{
  const Vec3T<double> v(3, 1, 2);
  REQUIRE(v.minDir(false) == 1); // index of smallest component (no abs-value)
  REQUIRE(v.maxDir(false) == 0); // index of largest component (no abs-value)
}

TEST_CASE("Vec3T: lexicographic ordering", "[Vec3T]")
{
  const Vec3T<double> a(1, 2, 3);
  const Vec3T<double> b(1, 2, 4);
  const Vec3T<double> c(2, 0, 0);

  REQUIRE(a.lessLX(b));
  REQUIRE(a.lessLX(c));
  REQUIRE(!b.lessLX(a));
}

// ─────────────────────────────────────────────────────────────────────────────
// Vec2T
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Vec2T: construction and operators", "[Vec2T]")
{
  const Vec2T<double> a(3, 4);
  REQUIRE_THAT(a.length(), WithinRel(5.0));
  REQUIRE_THAT(a.length2(), WithinRel(25.0));

  const Vec2T<double> b(1, 2);
  auto                c = a + b;
  REQUIRE(c.x == 4.0);
  REQUIRE(c.y == 6.0);

  // scalar / vec — should give (s/x, s/y)
  const Vec2T<double> v(2.0, 4.0);
  auto                r = 8.0 / v;
  REQUIRE(r.x == 4.0);
  REQUIRE(r.y == 2.0);
}

TEST_CASE("Vec2T: dot product", "[Vec2T]")
{
  const Vec2T<double> a(1, 0);
  const Vec2T<double> b(0, 1);
  REQUIRE(dot(a, b) == 0.0);
  REQUIRE(dot(a, a) == 1.0);
}
