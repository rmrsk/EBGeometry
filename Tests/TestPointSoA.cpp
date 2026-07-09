// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_PointSoA.hpp (PointSoAT), mirroring TestTriangleSoA.cpp's structure.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

template <class T>
using Pnt = Point<T, short>;

constexpr size_t W = 4;

template <class T>
using SoA = PointSoAT<T, W, short>;

// Four disjoint, well-separated points, spread out along x so each one is unambiguously the
// closest for a query point placed near it. Metadata is the point's own index, so getMetaData()
// can be checked against "which point is this."
template <class T>
std::vector<Pnt<T>>
fourPoints()
{
  using Vec3 = Vec3T<T>;

  std::vector<Pnt<T>> points;
  points.reserve(4);
  for (int i = 0; i < 4; i++) {
    points.emplace_back(Vec3(T(3.0 * i), T(0), T(0)), static_cast<short>(i));
  }
  return points;
}

template <class T>
T
minDistance2(const std::vector<Pnt<T>>& a_points, const Vec3T<T>& a_query)
{
  T best2 = std::numeric_limits<T>::max();
  for (const auto& p : a_points) {
    best2 = std::min(best2, p.getDistance2(a_query));
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

TEMPLATE_TEST_CASE("PointSoAT: getDistance/getDistance2 match the closest point when fully "
                   "packed (count == W)",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto points = fourPoints<T>();
  REQUIRE(points.size() == W);

  SoA<T> group;
  group.pack(points.data(), static_cast<uint32_t>(points.size()));

  for (const auto& q : queryPoints<T>()) {
    const T expected2 = minDistance2<T>(points, q);

    REQUIRE_THAT(group.getDistance2(q), withinAbsT(expected2, looseMargin<T>()));
    REQUIRE_THAT(group.getDistance(q), withinAbsT(std::sqrt(expected2), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointSoAT: getDistance/getDistance2 unaffected by padding when count < W",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto points = fourPoints<T>();
  points.resize(2); // Pack only the first 2 of the 4 disjoint points; lanes 2 and 3 get padded.

  SoA<T> group;
  group.pack(points.data(), static_cast<uint32_t>(points.size()));

  for (const auto& q : queryPoints<T>()) {
    const T expected2 = minDistance2<T>(points, q);

    REQUIRE_THAT(group.getDistance2(q), withinAbsT(expected2, looseMargin<T>()));
    REQUIRE_THAT(group.getDistance(q), withinAbsT(std::sqrt(expected2), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointSoAT: getDistance/getDistance2 handle a single packed point (count == 1)",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto points = fourPoints<T>();
  points.resize(1);

  SoA<T> group;
  group.pack(points.data(), 1);

  for (const auto& q : queryPoints<T>()) {
    const T expected2 = points.front().getDistance2(q);

    REQUIRE_THAT(group.getDistance2(q), withinAbsT(expected2, looseMargin<T>()));
    REQUIRE_THAT(group.getDistance(q), withinAbsT(std::sqrt(expected2), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointSoAT: getMetaData returns each lane's own metadata, and the padded "
                   "point's metadata for padding lanes",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto points = fourPoints<T>();
  points.resize(2); // Metadata 0 and 1; lanes 2 and 3 get padded with point 1's data (index 1).

  SoA<T> group;
  group.pack(points.data(), static_cast<uint32_t>(points.size()));

  REQUIRE(group.getMetaData(0) == short(0));
  REQUIRE(group.getMetaData(1) == short(1));
  REQUIRE(group.getMetaData(2) == short(1)); // Padded: repeats the last real point (index 1).
  REQUIRE(group.getMetaData(3) == short(1));
}

TEMPLATE_TEST_CASE("PointSoAT::computeBoundingVolume matches an AABB built directly from the "
                   "points' positions",
                   "[PointSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using AABB = BoundingVolumes::AABBT<T>;

  const auto points = fourPoints<T>();

  SoA<T> group;
  group.pack(points.data(), static_cast<uint32_t>(points.size()));

  const auto bv = group.template computeBoundingVolume<AABB>();

  std::vector<Vec3> positions;
  positions.reserve(points.size());
  for (const auto& p : points) {
    positions.push_back(p.getPosition());
  }
  const AABB expected(positions);

  for (int i = 0; i < 3; i++) {
    REQUIRE_THAT(bv.getLowCorner()[i], withinAbsT(expected.getLowCorner()[i], looseMargin<T>()));
    REQUIRE_THAT(bv.getHighCorner()[i], withinAbsT(expected.getHighCorner()[i], looseMargin<T>()));
  }
}
