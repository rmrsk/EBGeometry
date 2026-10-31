// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cmath>
#include <limits>
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

template <class S, class V, class = void>
struct HasScalarLeftOps : std::false_type
{
};

template <class S, class V>
struct HasScalarLeftOps<S,
                        V,
                        std::void_t<decltype(std::declval<const S&>() * std::declval<const V&>()),
                                    decltype(std::declval<const S&>() / std::declval<const V&>())>> : std::true_type
{
};

template <class V, class = void>
struct HasPublicX : std::false_type
{
};

template <class V>
struct HasPublicX<V, std::void_t<decltype(std::declval<V&>().x)>> : std::true_type
{
};

// A type that is not a scalar, for checking that the scalar-left operators reject it.
struct NotAScalar
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

TEMPLATE_TEST_CASE("Vec2T and Vec3T: trivially default constructible, and value initialization is zero",
                   "[Vec2T][Vec3T]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A trivial default constructor is what lets a vector be the element type of a CUDA/HIP __shared__
  // array, which may not have an initializer.
  STATIC_REQUIRE(std::is_trivially_default_constructible_v<Vec2T<T>>);
  STATIC_REQUIRE(std::is_trivially_default_constructible_v<Vec3T<T>>);

  const Vec2T<T> u{};
  REQUIRE(u[0] == T(0.0));
  REQUIRE(u[1] == T(0.0));

  const Vec3T<T> v{};
  REQUIRE(v[0] == T(0.0));
  REQUIRE(v[1] == T(0.0));
  REQUIRE(v[2] == T(0.0));

  const Vec3T<T> w = Vec3T<T>();
  REQUIRE(w == Vec3T<T>::zeros());
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

TEMPLATE_TEST_CASE("Vec3T: clamp holds each component to its own bounds", "[Vec3T]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T> lo(T(-1), T(0), T(2));
  const Vec3T<T> hi(T(1), T(4), T(2));

  // Below, inside and above the bounds, one component each; the z bounds coincide.
  CHECK(clamp(Vec3T<T>(T(-3), T(1), T(5)), lo, hi) == Vec3T<T>(T(-1), T(1), T(2)));
  CHECK(clamp(Vec3T<T>(T(0.5), T(7), T(-9)), lo, hi) == Vec3T<T>(T(0.5), T(4), T(2)));
  CHECK(clamp(Vec3T<T>(T(3), T(-2), T(2)), lo, hi) == Vec3T<T>(T(1), T(0), T(2)));

  // A component on a bound stays there, and a vector inside the box is unchanged.
  CHECK(clamp(Vec3T<T>(T(-1), T(4), T(2)), lo, hi) == Vec3T<T>(T(-1), T(4), T(2)));
  CHECK(clamp(Vec3T<T>(T(0.25), T(3), T(2)), lo, hi) == Vec3T<T>(T(0.25), T(3), T(2)));

  // Infinite components clamp to the finite bound.
  const T inf = Math::Limits<T>::infinity();
  CHECK(clamp(Vec3T<T>(-inf, inf, inf), lo, hi) == Vec3T<T>(T(-1), T(4), T(2)));

  // A NaN component compares false against both bounds and passes through, as with std::clamp.
  const Vec3T<T> c = clamp(Vec3T<T>(std::numeric_limits<T>::quiet_NaN(), T(9), T(0)), lo, hi);
  CHECK(std::isnan(c[0]));
  CHECK(c[1] == T(4));
  CHECK(c[2] == T(2));

  // clamp is constexpr.
  STATIC_REQUIRE(clamp(Vec3T<T>(T(5), T(-5), T(1)), Vec3T<T>::zeros(), Vec3T<T>::ones()) == Vec3T<T>(T(1), T(0), T(1)));
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
  REQUIRE(c[0] == T(4.0));
  REQUIRE(c[1] == T(6.0));

  // scalar / vec — should give (s/x, s/y)
  const Vec2T<T> v(T(2.0), T(4.0));
  auto           r = T(8.0) / v;
  REQUIRE(r[0] == T(4.0));
  REQUIRE(r[1] == T(2.0));
}

TEMPLATE_TEST_CASE("Vec2T: components are reached through operator[], as in Vec3T",
                   "[Vec2T]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Neither type has public named components.
  STATIC_REQUIRE_FALSE(HasPublicX<Vec2T<T>>::value);
  STATIC_REQUIRE_FALSE(HasPublicX<Vec3T<T>>::value);

  Vec2T<T> u(T(1), T(2));
  CHECK(u[0] == T(1));
  CHECK(u[1] == T(2));

  u[0] = T(5);
  u[1] -= T(4);
  CHECK(u[0] == T(5));
  CHECK(u[1] == T(-2));

  const Vec2T<T>& c = u;
  CHECK(c[0] == T(5));
  CHECK(c[1] == T(-2));

  // A Vec2T is its two components and nothing else.
  STATIC_REQUIRE(sizeof(Vec2T<T>) == 2 * sizeof(T));
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

TEMPLATE_TEST_CASE("Vec2T/Vec3T: dividing by a scalar divides each component",
                   "[Vec2T][Vec3T]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // x * (1/3) differs from x / 3 in the last bit for x = 5, 7 and 10, in float and in double. Division
  // used to multiply by the reciprocal.
  const T s = T(3);

  const Vec3T<T> u(T(5), T(7), T(10));
  Vec3T<T>       w = u;

  w /= s;

  for (int i = 0; i < 3; i++) {
    CHECK((u / s)[i] == u[i] / s);
    CHECK(w[i] == u[i] / s);
  }

  const Vec2T<T> a(T(5), T(7));
  Vec2T<T>       b = a;

  b /= s;

  CHECK((a / s)[0] == a[0] / s);
  CHECK((a / s)[1] == a[1] / s);
  CHECK(b[0] == a[0] / s);
  CHECK(b[1] == a[1] / s);
}

TEMPLATE_TEST_CASE("Vec2T/Vec3T: a scalar of any arithmetic type multiplies and divides from the left",
                   "[Vec2T][Vec3T]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec2T<T> u(T(1), T(4));
  const Vec3T<T> v(T(1), T(2), T(4));

  // A double, a float and an int scalar, whatever T is: each is converted to T first. Vec2T has no
  // operator==, so its components are compared.
  const auto equals2 = [](const Vec2T<T>& a, const T x, const T y) { return a[0] == x && a[1] == y; };

  CHECK(equals2(2.0 * u, T(2), T(8)));
  CHECK(equals2(2.0F * u, T(2), T(8)));
  CHECK(equals2(2 * u, T(2), T(8)));
  CHECK(equals2(8.0 / u, T(8), T(2)));
  CHECK(equals2(8 / u, T(8), T(2)));

  CHECK(2.0 * v == Vec3T<T>(T(2), T(4), T(8)));
  CHECK(2.0F * v == Vec3T<T>(T(2), T(4), T(8)));
  CHECK(2 * v == Vec3T<T>(T(2), T(4), T(8)));
  CHECK(8.0 / v == Vec3T<T>(T(8), T(4), T(2)));
  CHECK(8 / v == Vec3T<T>(T(8), T(4), T(2)));

  // A non-arithmetic left operand is not a scalar: no overload takes it.
  STATIC_REQUIRE(HasScalarLeftOps<double, Vec2T<T>>::value);
  STATIC_REQUIRE(HasScalarLeftOps<int, Vec3T<T>>::value);
  STATIC_REQUIRE_FALSE(HasScalarLeftOps<NotAScalar, Vec2T<T>>::value);
  STATIC_REQUIRE_FALSE(HasScalarLeftOps<NotAScalar, Vec3T<T>>::value);
}

TEMPLATE_TEST_CASE("Vec2T/Vec3T: min and max keep the first vector's component for a NaN, like Math::min/max",
                   "[Vec2T][Vec3T]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // They used to return the second vector's component, so the result depended on the argument order.
  const T nan = std::numeric_limits<T>::quiet_NaN();

  const Vec3T<T> withNaN(nan, T(1), T(2));
  const Vec3T<T> finite(T(0), T(0), T(3));

  CHECK(std::isnan(min(withNaN, finite)[0]));
  CHECK(std::isnan(max(withNaN, finite)[0]));
  CHECK(min(finite, withNaN)[0] == T(0));
  CHECK(max(finite, withNaN)[0] == T(0));

  CHECK(min(withNaN, finite)[1] == T(0));
  CHECK(max(withNaN, finite)[2] == T(3));

  const Vec2T<T> a(nan, T(1));
  const Vec2T<T> b(T(0), T(2));

  CHECK(std::isnan(min(a, b)[0]));
  CHECK(std::isnan(max(a, b)[0]));
  CHECK(min(b, a)[0] == T(0));
  CHECK(max(b, a)[0] == T(0));
  CHECK(min(a, b)[1] == T(1));
  CHECK(max(a, b)[1] == T(2));
}

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
