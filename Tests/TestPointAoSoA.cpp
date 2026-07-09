// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_PointAoSoA.hpp (PointAoSoA): the metadata-carrying wrapper around
// PointSoAT. Checks both that distance queries agree with a plain PointSoAT built from the same
// positions (i.e. metadata genuinely doesn't affect the distance path) and that metadata is
// retrieved correctly per lane, including for padded lanes.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

constexpr size_t W = 4;

template <class T>
using AoSoA = PointAoSoA<T, short, W>;

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
std::vector<short>
fourMetaData()
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

TEMPLATE_TEST_CASE("PointAoSoA: getDistance/getDistance2 agree exactly with a plain PointSoAT "
                   "built from the same positions",
                   "[PointAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto positions = fourPositions<T>();
  const auto metaData  = fourMetaData<T>();
  REQUIRE(positions.size() == W);
  REQUIRE(metaData.size() == W);

  AoSoA<T> withMeta;
  withMeta.pack(positions.data(), metaData.data(), static_cast<uint32_t>(positions.size()));

  SoA<T> positionOnly;
  positionOnly.pack(positions.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    // Bit-for-bit: both walk the identical scalar loop over the identical positions, so metadata
    // truly never enters the distance computation.
    REQUIRE(withMeta.getDistance2(q) == positionOnly.getDistance2(q));
    REQUIRE(withMeta.getDistance(q) == positionOnly.getDistance(q));
  }
}

TEMPLATE_TEST_CASE("PointAoSoA: getMetaData returns each lane's own metadata, and the padded "
                   "point's metadata for padding lanes",
                   "[PointAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto positions = fourPositions<T>();
  auto metaData  = fourMetaData<T>();
  positions.resize(2); // Pack only the first 2; lanes 2 and 3 get padded.
  metaData.resize(2);

  AoSoA<T> group;
  group.pack(positions.data(), metaData.data(), static_cast<uint32_t>(positions.size()));

  REQUIRE(group.getMetaData(0) == short(10));
  REQUIRE(group.getMetaData(1) == short(11));
  REQUIRE(group.getMetaData(2) == short(11)); // Padded: repeats the last real point's metadata.
  REQUIRE(group.getMetaData(3) == short(11));
}

TEMPLATE_TEST_CASE("PointAoSoA: getDistance/getDistance2 unaffected by padding when count < W",
                   "[PointAoSoA]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  auto positions = fourPositions<T>();
  auto metaData  = fourMetaData<T>();
  positions.resize(2);
  metaData.resize(2);

  AoSoA<T> group;
  group.pack(positions.data(), metaData.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    T best2 = std::numeric_limits<T>::max();
    for (const auto& pos : positions) {
      best2 = std::min(best2, (pos - q).length2());
    }

    REQUIRE_THAT(group.getDistance2(q), withinAbsT(best2, looseMargin<T>()));
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
  const auto metaData  = fourMetaData<T>();

  AoSoA<T> group;
  group.pack(positions.data(), metaData.data(), static_cast<uint32_t>(positions.size()));

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
  using DefaultWidthAoSoA = PointAoSoA<T, short>;

  static_assert(std::is_same_v<DefaultWidthAoSoA, PointAoSoA<T, short, PointSoA::DefaultWidth<T>()>>,
                "PointAoSoA<T, Meta> (W omitted) must equal PointAoSoA<T, Meta, PointSoA::DefaultWidth<T>()>");

  const size_t defaultWidth = PointSoA::DefaultWidth<T>();

  std::vector<Vec3T<T>> positions;
  std::vector<short>    metaData;
  positions.reserve(defaultWidth);
  metaData.reserve(defaultWidth);
  for (size_t i = 0; i < defaultWidth; i++) {
    positions.emplace_back(T(3.0) * T(i), T(0), T(0));
    metaData.emplace_back(static_cast<short>(10 + i));
  }

  DefaultWidthAoSoA group;
  group.pack(positions.data(), metaData.data(), static_cast<uint32_t>(positions.size()));

  for (const auto& q : queryPoints<T>()) {
    T best2 = std::numeric_limits<T>::max();
    for (const auto& pos : positions) {
      best2 = std::min(best2, (pos - q).length2());
    }

    REQUIRE_THAT(group.getDistance2(q), withinAbsT(best2, looseMargin<T>()));
  }

  for (size_t i = 0; i < defaultWidth; i++) {
    REQUIRE(group.getMetaData(i) == static_cast<short>(10 + i));
  }
}
