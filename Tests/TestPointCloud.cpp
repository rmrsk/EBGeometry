// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_PointCloud.hpp: the Hit result, the KBest set and the brute-force
// references that PointCloudBVH and PointCloudHashGrid share. The structures' own tests run these on
// the host and in a device kernel; this file checks the helpers' contracts directly.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestPointCloudQueries.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestPointCloud;

TEMPLATE_TEST_CASE("PointCloud::Hit: a default Hit is a miss", "[PointCloud]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const PointCloud::Hit<T> miss;

  CHECK_FALSE(miss.valid());
  CHECK(miss.index == PointCloud::InvalidIndex);
  CHECK(miss.distanceSquared == Math::Limits<T>::max());

  // Cloud index 0 is a real point, unlike the old {0, max} miss it used to be confused with.
  CHECK(PointCloud::Hit<T>{0, T(1)}.valid());
}

TEMPLATE_TEST_CASE("PointCloud::KBest keeps the k nearest in order, honours the exclusion, and prunes",
                   "[PointCloud]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  using Hit = PointCloud::Hit<T>;

  Hit out[3];

  PointCloud::KBest<T> best(out, 3, /*exclude*/ 7);

  CHECK(best.found() == 0);
  CHECK(best.bound() == Math::Limits<T>::max());

  best.insert(T(5), 0);
  best.insert(T(2), 1);
  best.insert(T(0), 7); // excluded: never reported, however close

  CHECK(best.found() == 2);
  CHECK(best.bound() == Math::Limits<T>::max()); // not yet k held

  best.insert(T(9), 2);

  CHECK(best.found() == 3);
  CHECK(best.bound() == T(9));

  best.insert(T(9), 3); // a tie with the k-th is not closer: rejected
  best.insert(T(3), 4); // displaces the farthest

  REQUIRE(best.found() == 3);
  CHECK(out[0].index == 1);
  CHECK(out[1].index == 4);
  CHECK(out[2].index == 0);
  CHECK(out[0].distanceSquared == T(2));
  CHECK(out[1].distanceSquared == T(3));
  CHECK(out[2].distanceSquared == T(5));
  CHECK(best.bound() == T(5));

  // Equal distances below the bound keep the one inserted first ahead.
  best.insert(T(3), 5);

  CHECK(out[1].index == 4);
  CHECK(out[2].index == 5);
}

TEMPLATE_TEST_CASE("PointCloud::closestPoint(s)BruteForce match an independent scan",
                   "[PointCloud]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  using Hit = PointCloud::Hit<T>;

  constexpr std::size_t n = 300;
  constexpr std::size_t k = 5;

  const std::vector<Vec3T<T>> pos     = makeCloud<T>(n, 11u);
  const std::vector<Vec3T<T>> queries = makeCloud<T>(50, 12u);
  const auto                  numPos  = static_cast<uint32_t>(n);

  Hit out[k];

  for (const auto& q : queries) {
    const auto truth = bruteForce<T>(pos, q, k, n);

    REQUIRE(PointCloud::closestPointsBruteForce(pos.data(), numPos, q, k, out) == k);

    for (std::size_t j = 0; j < k; j++) {
      CHECK(out[j].distanceSquared == truth[j]);
      CHECK((pos[out[j].index] - q).length2() == truth[j]);
    }

    const Hit hit = PointCloud::closestPointBruteForce(pos.data(), numPos, q);

    CHECK(hit.distanceSquared == truth[0]);
  }

  // Self queries: the excluded point is never reported.
  for (uint32_t i = 0; i < n; i += 17) {
    const auto truth = bruteForce<T>(pos, pos[i], k, i);

    REQUIRE(PointCloud::closestPointsBruteForce(pos.data(), numPos, pos[i], k, out, i) == k);

    for (std::size_t j = 0; j < k; j++) {
      CHECK(out[j].index != i);
      CHECK(out[j].distanceSquared == truth[j]);
    }

    CHECK(PointCloud::closestPointBruteForce(pos.data(), numPos, pos[i], i).index != i);
  }

  // Fewer points than k: every point once, then misses.
  Hit few[k];

  REQUIRE(PointCloud::closestPointsBruteForce(pos.data(), 3U, queries[0], k, few) == 3);
  CHECK_FALSE(few[3].valid());

  REQUIRE(PointCloud::closestPointsBruteForce(pos.data(), 3U, pos[1], k, few, 1) == 2);

  // An empty cloud is a miss.
  CHECK_FALSE(PointCloud::closestPointBruteForce<T>(nullptr, 0U, queries[0]).valid());
}
