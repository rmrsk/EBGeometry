// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <type_traits>
#include <utility>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinRel;

// ─────────────────────────────────────────────────────────────────────────────
// Vec3T
// ─────────────────────────────────────────────────────────────────────────────

namespace {

template <class V, class = void>
struct HasLess : std::false_type
{
};

template <class V>
struct HasLess<V, std::void_t<decltype(std::declval<const V&>() < std::declval<const V&>())>> : std::true_type
{
};

} // namespace

TEMPLATE_TEST_CASE("Vec3T: has no ordering operators, so it cannot silently break std::set or std::sort",
                   "[Vec3T]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // "Every component less" is not a strict weak ordering: (1, 0, 0) and (0, 1, 0) are each not less
  // than the other, so a std::set would treat them as equal.
  STATIC_REQUIRE_FALSE(HasLess<Vec3T<T>>::value);

  // lessLX is the lexicographic ordering to use instead.
  REQUIRE(Vec3T<T>(T(0), T(1), T(0)).lessLX(Vec3T<T>(T(1), T(0), T(0))));
  REQUIRE_FALSE(Vec3T<T>(T(1), T(0), T(0)).lessLX(Vec3T<T>(T(0), T(1), T(0))));
}

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

// ─────────────────────────────────────────────────────────────────────────────
// Device: the Vec3T query surface is callable from a kernel and matches the host
// ─────────────────────────────────────────────────────────────────────────────

// The Vec3T operations checked on the device: those before VecDot are vector-valued, the rest scalar.
enum VecOp : int
{
  VecSum,
  VecDifference,
  VecScaled,
  VecDivided,
  VecMin,
  VecMax,
  VecClamp,
  VecCross,
  VecDot,
  VecMemberDot,
  VecLength,
  VecLength2,
  NumVecOps
};

// One Vec3T operation between the query point a and a fixed vector b; vector results return
// component m_component.
template <class T>
struct VecQuery
{
  Vec3T<T> m_b;
  int      m_op;
  int      m_component;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_a) const noexcept
  {
    const Vec3T<T>& b = m_b;
    const Vec3T<T>  c = a_a + b - b * T(2.0) + a_a / T(2.0);

    Vec3T<T> v = Vec3T<T>::zeros();

    switch (m_op) {
    case VecSum:
      v = a_a + b;
      break;
    case VecDifference:
      v = a_a - b;
      break;
    case VecScaled:
      v = a_a * T(2.0);
      break;
    case VecDivided:
      v = a_a / T(2.0);
      break;
    case VecMin:
      v = min(a_a, b);
      break;
    case VecMax:
      v = max(a_a, b);
      break;
    case VecClamp:
      v = clamp(c, min(a_a, b), max(a_a, b));
      break;
    case VecCross:
      v = cross(a_a, b);
      break;
    case VecDot:
      return dot(a_a, b);
    case VecMemberDot:
      return clamp(c, min(a_a, b), max(a_a, b)).dot(max(a_a, b));
    case VecLength:
      return cross(a_a, b).length();
    default:
      return c.length2();
    }

    return v[static_cast<size_t>(m_component)];
  }
};

TEMPLATE_TEST_CASE("Vec3T: device query surface matches the host", "[Vec3T][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  // Points on both sides of b in every component, so min/max/clamp take either argument.
  const Vec3T<T> b(T(0.5), T(-1.25), T(2.0));
  const auto     points = queryGrid<T>(Vec3T<T>(T(-3), T(-3), T(-3)), Vec3T<T>(T(3), T(3), T(3)), 10);

  for (int op = 0; op < NumVecOps; op++) {
    const int components = (op < VecDot) ? 3 : 1;

    for (int component = 0; component < components; component++) {
      const VecQuery<T> query{b, op, component};

      INFO("operation " << op << ", component " << component);
      requireSameResults(evaluateOnDevice<T>(query, points), evaluateOnHost<T>(query, points));
    }
  }
}
