// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_PointAoSoA.hpp (PointAoSoA): the PointSoAT group that carries point ids.
// Checks both that distance queries agree with a plain PointSoAT built from the same positions (i.e.
// the ids genuinely don't affect the distance path) and that ids and the valid-point count are
// retrieved correctly per lane, including for padded lanes.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <array>

using namespace EBGeometry;

namespace {

constexpr size_t W = 4;

template <class T>
using AoSoA = PointAoSoA<T, W>;

template <class T>
using SoA = PointSoAT<T, W>;

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
std::vector<uint32_t>
fourPointIds()
{
  return {10, 11, 12, 13};
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

TEMPLATE_TEST_CASE("PointAoSoA: getMinimumDistance/getMinimumDistance2 agree exactly with a plain PointSoAT "
                   "built from the same positions",
                   "[PointAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto positions = fourPositions<T>();
  const auto pointIds  = fourPointIds<T>();
  REQUIRE(positions.size() == W);
  REQUIRE(pointIds.size() == W);

  AoSoA<T> withIds;
  withIds.pack(positions.data(), pointIds.data(), static_cast<uint32_t>(positions.size()));

  SoA<T> positionOnly;
  positionOnly.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    // Bit-for-bit: both walk the identical scalar loop over the identical positions, so the ids
    // truly never enters the distance computation. Every distance query delegates straight through.
    REQUIRE(withIds.getMinimumDistance2(q) == positionOnly.getMinimumDistance2(q));
    REQUIRE(withIds.getMinimumDistance(q) == positionOnly.getMinimumDistance(q));
    REQUIRE(withIds.getMaximumDistance2(q) == positionOnly.getMaximumDistance2(q));
    REQUIRE(withIds.getMaximumDistance(q) == positionOnly.getMaximumDistance(q));
    REQUIRE(withIds.getDistances2(q) == positionOnly.getDistances2(q));
    REQUIRE(withIds.getDistances(q) == positionOnly.getDistances(q));
  }
}

TEMPLATE_TEST_CASE("PointAoSoA: getPointId returns each lane's own id, the padded point's id for padding "
                   "lanes, and numValid() the real count",
                   "[PointAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto positions = fourPositions<T>();
  auto pointIds  = fourPointIds<T>();
  positions.resize(2); // Pack only the first 2; lanes 2 and 3 get padded.
  pointIds.resize(2);

  AoSoA<T> group;
  group.pack(positions.data(), pointIds.data(), static_cast<uint32_t>(positions.size()));

  REQUIRE(group.getPointId(0) == 10U);
  REQUIRE(group.getPointId(1) == 11U);
  REQUIRE(group.getPointId(2) == 11U); // Padded: repeats the last real point's id.
  REQUIRE(group.getPointId(3) == 11U);
  REQUIRE(group.numValid() == 2U);
}

TEMPLATE_TEST_CASE("PointAoSoA: getMinimumDistance/getMinimumDistance2 unaffected by padding when count < W",
                   "[PointAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto positions = fourPositions<T>();
  auto pointIds  = fourPointIds<T>();
  positions.resize(2);
  pointIds.resize(2);

  AoSoA<T> group;
  group.pack(positions.data(), pointIds.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    T best2 = std::numeric_limits<T>::max();
    for (const auto& pos : positions) {
      best2 = std::min(best2, (pos - q).length2());
    }

    REQUIRE_THAT(group.getMinimumDistance2(q), withinAbsT(best2, looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointAoSoA::computeBoundingVolume matches an AABB built directly from the "
                   "positions",
                   "[PointAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;

  const auto positions = fourPositions<T>();
  const auto pointIds  = fourPointIds<T>();

  AoSoA<T> group;
  group.pack(positions.data(), pointIds.data(), static_cast<uint32_t>(positions.size()));

  const auto bv = group.template computeBoundingVolume<AABB>();

  const AABB expected(positions);

  for (int i = 0; i < 3; i++) {
    REQUIRE_THAT(bv.getLowCorner()[i], withinAbsT(expected.getLowCorner()[i], looseMargin<T>()));
    REQUIRE_THAT(bv.getHighCorner()[i], withinAbsT(expected.getHighCorner()[i], looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PointAoSoA: omitting W defaults to PointSoA::DefaultWidth<T>(), matching "
                   "PointSoAT's own default",
                   "[PointAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T                 = TestType;
  using DefaultWidthAoSoA = PointAoSoA<T>;

  static_assert(std::is_same_v<DefaultWidthAoSoA, PointAoSoA<T, PointSoA::DefaultWidth<T>()>>,
                "PointAoSoA<T> (W omitted) must equal PointAoSoA<T, PointSoA::DefaultWidth<T>()>");

  const size_t defaultWidth = PointSoA::DefaultWidth<T>();

  std::vector<Vec3T<T>> positions;
  std::vector<uint32_t> pointIds;
  positions.reserve(defaultWidth);
  pointIds.reserve(defaultWidth);
  for (size_t i = 0; i < defaultWidth; i++) {
    positions.emplace_back(T(3.0) * T(i), T(0), T(0));
    pointIds.emplace_back(static_cast<uint32_t>(10 + i));
  }

  DefaultWidthAoSoA group;
  group.pack(positions.data(), pointIds.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    T best2 = std::numeric_limits<T>::max();
    for (const auto& pos : positions) {
      best2 = std::min(best2, (pos - q).length2());
    }

    REQUIRE_THAT(group.getMinimumDistance2(q), withinAbsT(best2, looseMargin<T>()));
  }

  for (size_t i = 0; i < defaultWidth; i++) {
    REQUIRE(group.getPointId(i) == static_cast<uint32_t>(10 + i));
  }
}

TEST_CASE("PointAoSoA::pack rejects a null array and a count outside [1, W]", "[PointAoSoA][death]")
{
  using T = double;

  const Vec3T<T> point(T(0), T(0), T(0));
  const uint32_t id = 0;

  REQUIRE(abortsWith(
    [&point] {
      PointAoSoA<T, 4> group;
      group.pack(&point, nullptr, 1U);
    },
    "PointAoSoA::pack: the position and point id arrays must not be null"));

  REQUIRE(abortsWith(
    [&point, &id] {
      PointAoSoA<T, 4> group;
      group.pack(&point, &id, 5U);
    },
    "PointAoSoA::pack: the point count must be between 1 and 4 (5)"));
}

// ─────────────────────────────────────────────────────────────────────────────
// Device: a host-packed PointAoSoA is queried in a kernel and matches the host
// ─────────────────────────────────────────────────────────────────────────────

// The minimum squared distance from a query point to the group, which is held by value.
template <class T>
struct PointAoSoADistanceQuery
{
  AoSoA<T> m_group;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_group.getMinimumDistance2(a_point);
  }
};

// The point id of lane i % W.
template <class T>
struct PointAoSoAPointIdQuery
{
  AoSoA<T> m_group;

  EBGEOMETRY_HOST_DEVICE
  int
  operator()(const int& a_i) const noexcept
  {
    return static_cast<int>(m_group.getPointId(static_cast<size_t>(a_i) % W));
  }
};

TEMPLATE_TEST_CASE("PointAoSoA: device query surface matches the host", "[PointAoSoA][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const auto positions = fourPositions<T>();
  const auto pointIds  = fourPointIds<T>();

  AoSoA<T> group;
  group.pack(positions.data(), pointIds.data(), static_cast<uint32_t>(positions.size()));

  // The points lie at x = 0, 3, 6, 9 on the x-axis; the grid surrounds all of them, so each is the
  // closest one for some queries.
  const auto points = queryGrid<T>(Vec3T<T>(T(-2), T(-3), T(-3)), Vec3T<T>(T(11), T(3), T(3)), 10);

  requireSameResults(evaluateOnDevice<T>(PointAoSoADistanceQuery<T>{group}, points),
                     evaluateOnHost<T>(PointAoSoADistanceQuery<T>{group}, points));

  std::vector<int> lanes;

  for (int i = 0; i < 256; i++) {
    lanes.push_back(i);
  }

  const auto deviceIds = evaluateOnDevice<int>(PointAoSoAPointIdQuery<T>{group}, lanes);
  const auto hostIds   = evaluateOnHost<int>(PointAoSoAPointIdQuery<T>{group}, lanes);

  REQUIRE(deviceIds.size() == hostIds.size());

  for (size_t i = 0; i < hostIds.size(); i++) {
    INFO("query " << i);
    REQUIRE(hostIds[i] == int(pointIds[i % W]));
    REQUIRE(deviceIds[i] == hostIds[i]);
  }
}
