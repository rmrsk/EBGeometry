// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Math::min/max/clamp and Math::Limits, which replace their std:: counterparts under Source/, must
// behave exactly like them, including for NaN. Array must behave like std::array where the library
// uses it.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <type_traits>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

TEMPLATE_TEST_CASE("Math::min, max and clamp match std::min, std::max and std::clamp",
                   "[Math]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const T nan = std::numeric_limits<T>::quiet_NaN();
  const T inf = std::numeric_limits<T>::infinity();

  const T values[] = {T(-2), T(-0.5), T(0), T(1.5), T(3), -inf, inf};

  for (const T a : values) {
    for (const T b : values) {
      REQUIRE(Math::min(a, b) == std::min(a, b));
      REQUIRE(Math::max(a, b) == std::max(a, b));

      if (a <= b) {
        for (const T v : values) {
          REQUIRE(Math::clamp(v, a, b) == std::clamp(v, a, b));
        }
      }
    }
  }

  // Like the std versions, the first argument wins when a comparison involves NaN.
  REQUIRE(std::isnan(Math::min(nan, T(1))));
  REQUIRE(Math::min(T(1), nan) == T(1));
  REQUIRE(std::isnan(Math::max(nan, T(1))));
  REQUIRE(Math::max(T(1), nan) == T(1));
  REQUIRE(std::isnan(Math::clamp(nan, T(0), T(1))));

  // Usable in constant expressions.
  static_assert(Math::min(T(1), T(2)) == T(1));
  static_assert(Math::max(T(1), T(2)) == T(2));
  static_assert(Math::clamp(T(5), T(0), T(1)) == T(1));
}

TEMPLATE_TEST_CASE("Math::Limits matches std::numeric_limits for floating-point types",
                   "[Math]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;
  using L = Math::Limits<T>;
  using S = std::numeric_limits<T>;

  static_assert(L::max() == S::max());
  static_assert(L::lowest() == S::lowest());
  static_assert(L::min() == S::min());
  static_assert(L::epsilon() == S::epsilon());
  static_assert(L::infinity() == S::infinity());
}

TEST_CASE("Math::Limits matches std::numeric_limits for integer types", "[Math]")
{
  const auto same = [](auto a_value) {
    using I = decltype(a_value);

    return Math::Limits<I>::max() == std::numeric_limits<I>::max() &&
           Math::Limits<I>::lowest() == std::numeric_limits<I>::lowest() &&
           Math::Limits<I>::min() == std::numeric_limits<I>::min();
  };

  REQUIRE(same(int(0)));
  REQUIRE(same(0U));
  REQUIRE(same(0L));
  REQUIRE(same(0UL));
  REQUIRE(same(0LL));
  REQUIRE(same(0ULL));
  REQUIRE(same(std::size_t(0)));
  REQUIRE(same(std::uint32_t(0)));
}

TEMPLATE_TEST_CASE("Array behaves like std::array where the library uses it",
                   "[Math][Array]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  static_assert(std::is_trivially_copyable_v<Array<T, 4>>);
  static_assert(std::is_aggregate_v<Array<T, 4>>);
  static_assert(sizeof(Array<T, 4>) == 4 * sizeof(T));
  static_assert(Array<T, 4>::size() == 4);

  Array<T, 4> a = {T(3), T(1), T(4), T(1)};

  REQUIRE(a[2] == T(4));
  REQUIRE(a.data() == &a[0]);
  REQUIRE(a.end() - a.begin() == 4);
  REQUIRE(std::accumulate(a.begin(), a.end(), T(0)) == T(9));

  // Sortable, as the BVH's child orderers do.
  std::sort(a.begin(), a.end());

  REQUIRE(a == Array<T, 4>{T(1), T(1), T(3), T(4)});
  REQUIRE(a != Array<T, 4>{T(1), T(1), T(3), T(5)});

  // Value-initialized means zeroed; fill() sets every element.
  const Array<T, 3> zeros{};

  REQUIRE(zeros == Array<T, 3>{T(0), T(0), T(0)});

  a.fill(T(7));

  for (const T x : a) {
    REQUIRE(x == T(7));
  }

  // Constant evaluation, as in the library's constexpr tables.
  constexpr Array<int, 3> c = {1, 2, 3};

  static_assert(c[1] == 2);
}
