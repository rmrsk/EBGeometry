// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_PointSoA.hpp (PointSoAT), the pure position-only SoA block. Mirrors
// TestTriangleSoA.cpp's structure. Metadata-carrying usage is tested separately in
// TestPointAoSoA.cpp.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <algorithm>
#include <array>

using namespace EBGeometry;

namespace {

constexpr size_t W = 4;

template <class T>
using SoA = PointSoAT<T, W>;

// Four disjoint, well-separated positions, spread out along x so each one is unambiguously the
// closest for a query point placed near it.
template <class T>
std::vector<Vec3T<T>>
fourPositions()
{
  using Vec3 = Vec3T<T>;

  std::vector<Vec3> positions;
  positions.reserve(4);
  for (int i = 0; i < 4; i++) {
    positions.emplace_back(T(3.0 * i), T(0), T(0));
  }
  return positions;
}

template <class T>
T
minDistance2(const std::vector<Vec3T<T>>& a_positions, const Vec3T<T>& a_query)
{
  T best2 = std::numeric_limits<T>::max();
  for (const auto& pos : a_positions) {
    best2 = std::min(best2, (pos - a_query).length2());
  }
  return best2;
}

template <class T>
T
maxDistance2(const std::vector<Vec3T<T>>& a_positions, const Vec3T<T>& a_query)
{
  T best2 = std::numeric_limits<T>::lowest();
  for (const auto& pos : a_positions) {
    best2 = std::max(best2, (pos - a_query).length2());
  }
  return best2;
}

template <class T>
std::vector<Vec3T<T>>
queryPoints()
{
  using Vec3 = Vec3T<T>;
  return {
    Vec3(0.1, 0.1, 0.1),
    Vec3(3.2, -0.1, 0.05),
    Vec3(6.0, 0.3, 0),
    Vec3(9.1, 0, 0),
    Vec3(-5, -5, -5),
    Vec3(20, 20, 20),
  };
}

} // namespace

TEMPLATE_TEST_CASE("PointSoAT: getMinimumDistance/getMinimumDistance2 match the closest position when fully "
                   "packed (count == W)",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto positions = fourPositions<T>();
  REQUIRE(positions.size() == W);

  SoA<T> group;
  group.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    const T expected2 = minDistance2<T>(positions, q);

    REQUIRE_THAT(group.getMinimumDistance2(q), withinAbsT(expected2, looseMargin<T>()));
    REQUIRE_THAT(group.getMinimumDistance(q), withinAbsT(std::sqrt(expected2), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointSoAT: getMinimumDistance/getMinimumDistance2 unaffected by padding when count < W",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto positions = fourPositions<T>();
  positions.resize(2); // Pack only the first 2 of the 4 disjoint positions; lanes 2 and 3 padded.

  SoA<T> group;
  group.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    const T expected2 = minDistance2<T>(positions, q);

    REQUIRE_THAT(group.getMinimumDistance2(q), withinAbsT(expected2, looseMargin<T>()));
    REQUIRE_THAT(group.getMinimumDistance(q), withinAbsT(std::sqrt(expected2), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointSoAT: getMinimumDistance/getMinimumDistance2 handle a single packed position (count == 1)",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto positions = fourPositions<T>();
  positions.resize(1);

  SoA<T> group;
  group.pack(positions.data(), 1);

  for (const auto& q : queryPoints<T>()) {
    const T expected2 = (positions.front() - q).length2();

    REQUIRE_THAT(group.getMinimumDistance2(q), withinAbsT(expected2, looseMargin<T>()));
    REQUIRE_THAT(group.getMinimumDistance(q), withinAbsT(std::sqrt(expected2), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointSoAT: getMaximumDistance/getMaximumDistance2 match the farthest position",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto positions = fourPositions<T>();

  SoA<T> group;
  group.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    const T expected2 = maxDistance2<T>(positions, q);

    REQUIRE_THAT(group.getMaximumDistance2(q), withinAbsT(expected2, looseMargin<T>()));
    REQUIRE_THAT(group.getMaximumDistance(q), withinAbsT(std::sqrt(expected2), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointSoAT: getMaximumDistance2 unaffected by padding (padded lanes duplicate a real point)",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto positions = fourPositions<T>();
  positions.resize(2); // lanes 2, 3 padded (repeat position 1); the max over reals must be unchanged.

  SoA<T> group;
  group.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    REQUIRE_THAT(group.getMaximumDistance2(q), withinAbsT(maxDistance2<T>(positions, q), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointSoAT: getDistances2/getDistances return every lane's distance (padded lanes repeat the "
                   "last real position)",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto positions = fourPositions<T>();
  positions.resize(3); // lanes 0,1,2 real; lane 3 padded (repeats position 2).

  SoA<T> group;
  group.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    const std::array<T, W> d2 = group.getDistances2(q);
    const std::array<T, W> d  = group.getDistances(q);

    for (size_t lane = 0; lane < W; lane++) {
      const size_t src      = (lane < positions.size()) ? lane : (positions.size() - 1); // padding source
      const T      expected = (positions[src] - q).length2();

      REQUIRE_THAT(d2[lane], withinAbsT(expected, looseMargin<T>()));
      REQUIRE_THAT(d[lane], withinAbsT(std::sqrt(expected), looseMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("PointSoAT::computeBoundingVolume matches an AABB built directly from the "
                   "positions",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  const auto positions = fourPositions<T>();

  SoA<T> group;
  group.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  const auto bv = group.template computeBoundingVolume<AABB>();

  const AABB expected(positions);

  for (int i = 0; i < 3; i++) {
    REQUIRE_THAT(bv.getLowCorner()[i], withinAbsT(expected.getLowCorner()[i], looseMargin<T>()));
    REQUIRE_THAT(bv.getHighCorner()[i], withinAbsT(expected.getHighCorner()[i], looseMargin<T>()));
  }
}

// W=8 coverage, in addition to the W=4 cases above: on this repo's supported compile targets,
// W=4 exercises the SSE4.1 (float) and single-pass AVX (double) SIMD branches, while W=8
// exercises the AVX (float) and two-pass AVX (double) branches -- a different pair of branches in
// PointSoAT::getMinimumDistance2()'s dispatch. (AVX-512F's W=16 float / W=8 double branches are not
// covered by a runtime test in this repo's CI hardware; they get a compile-only check instead --
// see Scripts/clang-tidy-check.sh's -DEBGEOMETRY_SIMD=avx512 configuration.)
namespace {

constexpr size_t W8 = 8;

template <class T>
using SoA8 = PointSoAT<T, W8>;

template <class T>
std::vector<Vec3T<T>>
eightPositions()
{
  using Vec3 = Vec3T<T>;

  std::vector<Vec3> positions;
  positions.reserve(W8);
  for (int i = 0; i < 8; i++) {
    positions.emplace_back(T(3.0 * i), T(0), T(0));
  }
  return positions;
}

} // namespace

TEMPLATE_TEST_CASE("PointSoAT (W=8): getMinimumDistance/getMinimumDistance2 match the closest position when "
                   "fully packed",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto positions = eightPositions<T>();
  REQUIRE(positions.size() == W8);

  SoA8<T> group;
  group.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    const T expected2 = minDistance2<T>(positions, q);

    REQUIRE_THAT(group.getMinimumDistance2(q), withinAbsT(expected2, looseMargin<T>()));
    REQUIRE_THAT(group.getMinimumDistance(q), withinAbsT(std::sqrt(expected2), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointSoAT (W=8): getMinimumDistance/getMinimumDistance2 unaffected by padding when count < W",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto positions = eightPositions<T>();
  positions.resize(3); // Pack only 3 of the 8; lanes 3-7 padded.

  SoA8<T> group;
  group.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    const T expected2 = minDistance2<T>(positions, q);

    REQUIRE_THAT(group.getMinimumDistance2(q), withinAbsT(expected2, looseMargin<T>()));
    REQUIRE_THAT(group.getMinimumDistance(q), withinAbsT(std::sqrt(expected2), looseMargin<T>()));
  }
}

// Leaving W unspecified must pick up PointSoA::DefaultWidth<T>() -- and, per that function's own
// table, float and double do not, in general, share a default width on the same ISA (e.g. AVX:
// float->8, double->4), so this is checked per-precision rather than assumed to match TestType's
// sibling.
TEMPLATE_TEST_CASE("PointSoAT: omitting W defaults to PointSoA::DefaultWidth<T>(), and is usable "
                   "end-to-end at that width",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T               = TestType;
  using DefaultWidthSoA = PointSoAT<T>;

  static_assert(std::is_same_v<DefaultWidthSoA, PointSoAT<T, PointSoA::DefaultWidth<T>()>>,
                "PointSoAT<T> (W omitted) must equal PointSoAT<T, PointSoA::DefaultWidth<T>()>");

  const size_t defaultWidth = PointSoA::DefaultWidth<T>();

  std::vector<Vec3T<T>> positions;
  positions.reserve(defaultWidth);
  for (size_t i = 0; i < defaultWidth; i++) {
    positions.emplace_back(T(3.0) * T(i), T(0), T(0));
  }

  DefaultWidthSoA group;
  group.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    const T expected2 = minDistance2<T>(positions, q);

    REQUIRE_THAT(group.getMinimumDistance2(q), withinAbsT(expected2, looseMargin<T>()));
    REQUIRE_THAT(group.getMinimumDistance(q), withinAbsT(std::sqrt(expected2), looseMargin<T>()));
  }
}
