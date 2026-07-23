// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinRel;

// ─────────────────────────────────────────────────────────────────────────────
// Vec3T
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Vec3T: default construction is zero", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Vec3T<T> v;
  REQUIRE(v[0] == T(0.0));
  REQUIRE(v[1] == T(0.0));
  REQUIRE(v[2] == T(0.0));
}

TEMPLATE_TEST_CASE("Vec3T: component construction fills correctly", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Vec3T<T> v(T(7.0), T(7.0), T(7.0));
  REQUIRE(v[0] == T(7.0));
  REQUIRE(v[1] == T(7.0));
  REQUIRE(v[2] == T(7.0));
}

TEMPLATE_TEST_CASE("Vec3T: component construction", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Vec3T<T> v(T(1.0), T(2.0), T(3.0));
  REQUIRE(v[0] == T(1.0));
  REQUIRE(v[1] == T(2.0));
  REQUIRE(v[2] == T(3.0));
}

TEMPLATE_TEST_CASE("Vec3T: static factories", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto z = Vec3T<T>::zeros();
  REQUIRE(z[0] == T(0.0));
  REQUIRE(z[1] == T(0.0));
  REQUIRE(z[2] == T(0.0));

  auto o = Vec3T<T>::ones();
  REQUIRE(o[0] == T(1.0));
  REQUIRE(o[1] == T(1.0));
  REQUIRE(o[2] == T(1.0));

  auto ex = Vec3T<T>::unit(0);
  REQUIRE(ex[0] == T(1.0));
  REQUIRE(ex[1] == T(0.0));
  REQUIRE(ex[2] == T(0.0));

  auto ey = Vec3T<T>::unit(1);
  REQUIRE(ey[1] == T(1.0));

  auto ez = Vec3T<T>::unit(2);
  REQUIRE(ez[2] == T(1.0));
}

TEMPLATE_TEST_CASE("Vec3T: arithmetic operators", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T> a(1, 2, 3);
  const Vec3T<T> b(4, 5, 6);

  SECTION("addition")
  {
    auto c = a + b;
    REQUIRE(c[0] == T(5.0));
    REQUIRE(c[1] == T(7.0));
    REQUIRE(c[2] == T(9.0));
  }
  SECTION("subtraction")
  {
    auto c = b - a;
    REQUIRE(c[0] == T(3.0));
    REQUIRE(c[1] == T(3.0));
    REQUIRE(c[2] == T(3.0));
  }
  SECTION("negation")
  {
    auto c = -a;
    REQUIRE(c[0] == T(-1.0));
    REQUIRE(c[1] == T(-2.0));
    REQUIRE(c[2] == T(-3.0));
  }
  SECTION("scalar multiply right")
  {
    auto c = a * T(3.0);
    REQUIRE(c[0] == T(3.0));
    REQUIRE(c[1] == T(6.0));
    REQUIRE(c[2] == T(9.0));
  }
  SECTION("scalar multiply left")
  {
    auto c = T(3.0) * a;
    REQUIRE(c[0] == T(3.0));
    REQUIRE(c[1] == T(6.0));
    REQUIRE(c[2] == T(9.0));
  }
  SECTION("scalar divide")
  {
    auto c = b / T(2.0);
    REQUIRE(c[0] == T(2.0));
    REQUIRE(c[1] == T(2.5));
    REQUIRE(c[2] == T(3.0));
  }
  SECTION("scalar / vec  (s/v[i] component-wise)")
  {
    const Vec3T<T> v(T(1.0), T(2.0), T(4.0));
    auto           c = T(4.0) / v;
    REQUIRE(c[0] == T(4.0));
    REQUIRE(c[1] == T(2.0));
    REQUIRE(c[2] == T(1.0));
  }
}

TEMPLATE_TEST_CASE("Vec3T: compound assignment operators", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Vec3T<T> v(1, 2, 3);
  v += Vec3T<T>(10, 20, 30);
  REQUIRE(v[0] == T(11.0));
  v -= Vec3T<T>(1, 2, 3);
  REQUIRE(v[0] == T(10.0));
  v *= T(2.0);
  REQUIRE(v[0] == T(20.0));
  v /= T(4.0);
  REQUIRE(v[0] == T(5.0));
}

TEMPLATE_TEST_CASE("Vec3T: dot product", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T> x(1, 0, 0);
  const Vec3T<T> y(0, 1, 0);
  const Vec3T<T> d(1, 1, 1);

  REQUIRE(dot(x, y) == T(0.0));
  REQUIRE(dot(x, d) == T(1.0));
  REQUIRE_THAT(dot(d, d), WithinRel(T(3.0)));
}

TEMPLATE_TEST_CASE("Vec3T: cross product", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T> x(1, 0, 0);
  const Vec3T<T> y(0, 1, 0);
  auto           z = x.cross(y);
  REQUIRE(z[0] == T(0.0));
  REQUIRE(z[1] == T(0.0));
  REQUIRE(z[2] == T(1.0));

  // Anti-commutativity
  auto mz = y.cross(x);
  REQUIRE(mz[2] == T(-1.0));
}

TEMPLATE_TEST_CASE("Vec3T: length", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T> v(3, 4, 0);
  REQUIRE_THAT(v.length(), WithinRel(T(5.0)));
  REQUIRE_THAT(v.length2(), WithinRel(T(25.0)));

  const Vec3T<T> unit(1, 0, 0);
  REQUIRE_THAT(unit.length(), WithinRel(T(1.0)));
}

TEMPLATE_TEST_CASE("Vec3T: component-wise min/max (free functions)", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T> a(1, 5, 3);
  const Vec3T<T> b(4, 2, 6);

  auto lo = min(a, b);
  REQUIRE(lo[0] == T(1.0));
  REQUIRE(lo[1] == T(2.0));
  REQUIRE(lo[2] == T(3.0));

  auto hi = max(a, b);
  REQUIRE(hi[0] == T(4.0));
  REQUIRE(hi[1] == T(5.0));
  REQUIRE(hi[2] == T(6.0));
}

TEMPLATE_TEST_CASE("Vec3T: minDir and maxDir", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T> v(3, 1, 2);
  REQUIRE(v.minDir(false) == 1); // index of smallest component (no abs-value)
  REQUIRE(v.maxDir(false) == 0); // index of largest component (no abs-value)
}

TEMPLATE_TEST_CASE("Vec3T: lexicographic ordering", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T> a(1, 2, 3);
  const Vec3T<T> b(1, 2, 4);
  const Vec3T<T> c(2, 0, 0);

  REQUIRE(a.lessLX(b));
  REQUIRE(a.lessLX(c));
  REQUIRE(!b.lessLX(a));
}

// ─────────────────────────────────────────────────────────────────────────────
// Vec2T
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Vec2T: construction and operators", "[Vec2T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec2T<T> a(3, 4);
  REQUIRE_THAT(a.length(), WithinRel(T(5.0)));
  REQUIRE_THAT(a.length2(), WithinRel(T(25.0)));

  const Vec2T<T> b(1, 2);
  auto           c = a + b;
  REQUIRE(c.x == T(4.0));
  REQUIRE(c.y == T(6.0));

  // scalar / vec — should give (s/x, s/y)
  const Vec2T<T> v(T(2.0), T(4.0));
  auto           r = T(8.0) / v;
  REQUIRE(r.x == T(4.0));
  REQUIRE(r.y == T(2.0));
}

TEMPLATE_TEST_CASE("Vec2T: dot product", "[Vec2T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec2T<T> a(1, 0);
  const Vec2T<T> b(0, 1);
  REQUIRE(dot(a, b) == T(0.0));
  REQUIRE(dot(a, a) == T(1.0));
}

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
// ─────────────────────────────────────────────────────────────────────────────
// Device: the Vec3T query surface is callable from a kernel and matches the host
// ─────────────────────────────────────────────────────────────────────────────

EBGEOMETRY_GLOBAL
void
vecDeviceKernel(Vec3T<double> a_a, Vec3T<double> a_b, double* a_out)
{
  const Vec3T<double> c  = a_a + a_b - a_b * 2.0 + a_a / 2.0;
  const Vec3T<double> mn = min(a_a, a_b);
  const Vec3T<double> mx = max(a_a, a_b);
  const Vec3T<double> cl = clamp(c, mn, mx);

  a_out[0] = dot(a_a, a_b) + cross(a_a, a_b).length() + c.length2() + cl.dot(mx);
}

TEST_CASE("Vec3T: device query surface matches the host", "[Vec3T][gpu]")
{
  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const Vec3T<double> a(1.0, 2.0, 3.0);
  const Vec3T<double> b = Vec3T<double>::ones();

  const Vec3T<double> c    = a + b - b * 2.0 + a / 2.0;
  const Vec3T<double> mn   = min(a, b);
  const Vec3T<double> mx   = max(a, b);
  const double        host = dot(a, b) + cross(a, b).length() + c.length2() + clamp(c, mn, mx).dot(mx);

  double* deviceOut = deviceScalar();

  vecDeviceKernel<<<1, 1>>>(a, b, deviceOut);
  (void)GPU::deviceSynchronize();

  REQUIRE_THAT(readScalar(deviceOut), WithinRel(host, 1.0e-9));

  deviceFree(deviceOut);
}
#endif
