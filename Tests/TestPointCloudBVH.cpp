// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for PointCloudBVH: the index-based build and the high-level closest-point / nearest-
// neighbor query API. Every query is checked against a brute-force O(N^2) scan on a small random
// cloud, so both the tree and the seed-from-own-leaf query path get real coverage in both precisions.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <random>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

namespace {

// A fixed, reproducible random cloud of n points in the unit cube.
template <class T>
std::vector<Vec3T<T>>
makeCloud(std::size_t a_n, unsigned a_seed)
{
  std::mt19937                      rng(a_seed);
  std::uniform_real_distribution<T> dist(T(0), T(1));
  std::vector<Vec3T<T>>             pos(a_n);
  for (std::size_t i = 0; i < a_n; i++) {
    pos[i] = Vec3T<T>(dist(rng), dist(rng), dist(rng));
  }
  return pos;
}

// Brute-force k nearest (squared) distances to a_query, optionally excluding one index. Sorted.
template <class T>
std::vector<T>
bruteForce(const std::vector<Vec3T<T>>& a_pos, const Vec3T<T>& a_query, std::size_t a_k, std::size_t a_exclude)
{
  std::vector<T> d2;
  d2.reserve(a_pos.size());
  for (std::size_t i = 0; i < a_pos.size(); i++) {
    if (i == a_exclude) {
      continue;
    }
    d2.push_back((a_pos[i] - a_query).length2());
  }
  std::sort(d2.begin(), d2.end());
  d2.resize(std::min(a_k, d2.size()));
  return d2;
}

} // namespace

TEMPLATE_TEST_CASE("PointCloudBVH queries match brute force", "[PointCloudBVH]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr std::size_t n = 2000;

  const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 20260708u);
  std::vector<std::size_t>    meta(n);
  for (std::size_t i = 0; i < n; i++) {
    meta[i] = 7 * i + 3; // arbitrary user metadata, distinct from the cloud index
  }

  const PointCloudBVH<T, std::size_t> bvh(pos, meta);

  REQUIRE(bvh.numParticles() == n);

  const double tol = tightMargin<T>();

  SECTION("closestPoint (external query) matches brute force")
  {
    const std::vector<Vec3T<T>> queries = makeCloud<T>(200, 99u); // arbitrary external points
    for (const auto& q : queries) {
      const auto hit   = bvh.closestPoint(q);
      const auto truth = bruteForce<T>(pos, q, 1, n); // exclude nothing (n is out of range)

      REQUIRE(hit.index < n);
      CHECK_THAT(std::sqrt(hit.distanceSquared), withinAbsT<T>(std::sqrt(truth[0]), tol));
      // The returned index must actually be that closest point.
      CHECK_THAT((pos[hit.index] - q).length2(), withinAbsT<T>(truth[0], tol));
    }
  }

  SECTION("querying at a cloud point returns that point at distance 0")
  {
    for (std::size_t i = 0; i < n; i += 137) {
      const auto hit = bvh.closestPoint(pos[i]);
      CHECK(hit.index == i);
      CHECK_THAT(hit.distanceSquared, withinAbsT<T>(T(0), tol));
    }
  }

  SECTION("nearestNeighbor (self query, excludes self) matches brute force")
  {
    for (std::size_t i = 0; i < n; i += 41) {
      const auto hit   = bvh.nearestNeighbor(i);
      const auto truth = bruteForce<T>(pos, pos[i], 1, i);

      REQUIRE(hit.index < n);
      CHECK(hit.index != i);
      CHECK_THAT((pos[hit.index] - pos[i]).length2(), withinAbsT<T>(truth[0], tol));
    }
  }

  SECTION("k-nearest queries return k sorted, correct neighbors")
  {
    constexpr std::size_t k = 5;

    typename PointCloudBVH<T, std::size_t>::Hit out[k];

    for (std::size_t i = 0; i < n; i += 53) {
      // Self k-NN (excludes self).
      const std::size_t found = bvh.nearestNeighbors(i, k, out);
      const auto        truth = bruteForce<T>(pos, pos[i], k, i);

      REQUIRE(found == k);
      for (std::size_t j = 0; j < k; j++) {
        CHECK(out[j].index != i);
        if (j > 0) {
          CHECK(out[j - 1].distanceSquared <= out[j].distanceSquared); // ascending
        }
        CHECK_THAT(out[j].distanceSquared, withinAbsT<T>(truth[j], tol));
      }
    }

    // External k-NN (excludes nothing): querying at a cloud point yields itself first (distance 0).
    const std::size_t found = bvh.closestPoints(pos[0], k, out);
    REQUIRE(found == k);
    CHECK(out[0].index == 0);
    CHECK_THAT(out[0].distanceSquared, withinAbsT<T>(T(0), tol));
  }

  SECTION("allNearestNeighbors matches the single-query results")
  {
    const auto all = bvh.allNearestNeighbors(1);
    REQUIRE(all.size() == n);
    for (std::size_t i = 0; i < n; i += 29) {
      const auto single = bvh.nearestNeighbor(i);
      CHECK_THAT(all[i].distanceSquared, withinAbsT<T>(single.distanceSquared, tol));
    }
  }

  SECTION("accessors return the stored cloud data")
  {
    for (std::size_t i = 0; i < n; i += 313) {
      CHECK(bvh.metadata(i) == meta[i]);
      CHECK_THAT((bvh.position(i) - pos[i]).length2(), withinAbsT<T>(T(0), tol));
    }
  }

  SECTION("brute-force reference methods match an independent scan (and the accelerated queries)")
  {
    constexpr std::size_t k = 4;

    // External: closestPoint(s)BruteForce vs the independent oracle, and vs the accelerated query.
    const std::vector<Vec3T<T>>                 queries = makeCloud<T>(120, 4242u);
    typename PointCloudBVH<T, std::size_t>::Hit refOut[k];
    for (const auto& q : queries) {
      const auto ref   = bvh.closestPointBruteForce(q);
      const auto truth = bruteForce<T>(pos, q, 1, n);
      REQUIRE(ref.index < n);
      CHECK_THAT(ref.distanceSquared, withinAbsT<T>(truth[0], tol));
      // The accelerated query must agree with the brute-force reference.
      CHECK_THAT(bvh.closestPoint(q).distanceSquared, withinAbsT<T>(ref.distanceSquared, tol));

      const std::size_t found  = bvh.closestPointsBruteForce(q, k, refOut);
      const auto        truthK = bruteForce<T>(pos, q, k, n);
      REQUIRE(found == k);
      for (std::size_t j = 0; j < k; j++) {
        if (j > 0) {
          CHECK(refOut[j - 1].distanceSquared <= refOut[j].distanceSquared); // ascending
        }
        CHECK_THAT(refOut[j].distanceSquared, withinAbsT<T>(truthK[j], tol));
      }
    }

    // Self (excludes the query particle itself).
    for (std::size_t i = 0; i < n; i += 47) {
      const auto ref   = bvh.nearestNeighborBruteForce(i);
      const auto truth = bruteForce<T>(pos, pos[i], 1, i);
      REQUIRE(ref.index < n);
      CHECK(ref.index != i);
      CHECK_THAT(ref.distanceSquared, withinAbsT<T>(truth[0], tol));
      CHECK_THAT(bvh.nearestNeighbor(i).distanceSquared, withinAbsT<T>(ref.distanceSquared, tol));

      const std::size_t found  = bvh.nearestNeighborsBruteForce(i, k, refOut);
      const auto        truthK = bruteForce<T>(pos, pos[i], k, i);
      REQUIRE(found == k);
      for (std::size_t j = 0; j < k; j++) {
        CHECK(refOut[j].index != i);
        CHECK_THAT(refOut[j].distanceSquared, withinAbsT<T>(truthK[j], tol));
      }
    }
  }
}

TEMPLATE_TEST_CASE("PointCloudBVH edge cases", "[PointCloudBVH]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  using Hit = typename PointCloudBVH<T, std::size_t>::Hit;

  const T notFound = std::numeric_limits<T>::max();

  SECTION("empty cloud: no out-of-bounds access, queries report nothing")
  {
    const std::vector<Vec3T<T>>         pos;
    const std::vector<std::size_t>      meta;
    const PointCloudBVH<T, std::size_t> bvh(pos, meta);

    CHECK(bvh.numParticles() == 0);
    // Must not read m_linearNodes[0]; a miss is signalled by the sentinel distance.
    CHECK(bvh.closestPoint(Vec3T<T>(T(0), T(0), T(0))).distanceSquared == notFound);
    Hit               out[4];
    const std::size_t found = bvh.closestPoints(Vec3T<T>(T(0), T(0), T(0)), 4, out);
    CHECK(found == 0);
    CHECK(bvh.allNearestNeighbors(1).empty());
  }

  SECTION("single particle: external query finds it, self query finds nothing")
  {
    const std::vector<Vec3T<T>>         pos  = {Vec3T<T>(T(0.25), T(0.5), T(0.75))};
    const std::vector<std::size_t>      meta = {42};
    const PointCloudBVH<T, std::size_t> bvh(pos, meta);

    const auto hit = bvh.closestPoint(pos[0]);
    CHECK(hit.index == 0);
    CHECK_THAT(hit.distanceSquared, withinAbsT<T>(T(0), tightMargin<T>()));

    // nearestNeighbor excludes self, so with only one particle there is no neighbor.
    CHECK(bvh.nearestNeighbor(0).distanceSquared == notFound);
  }

  SECTION("deep tree (small leaves) still resolves seeded queries correctly")
  {
    // A small target leaf size forces many BVH levels, exercising the seeded fast-path DFS (and its
    // stack) far more than the default. Every self query is checked against brute force.
    constexpr std::size_t               n   = 3000;
    const std::vector<Vec3T<T>>         pos = makeCloud<T>(n, 555u);
    std::vector<std::size_t>            meta(n, 0);
    const PointCloudBVH<T, std::size_t> bvh(pos, meta, /* targetLeafSize */ 2);

    for (std::size_t i = 0; i < n; i += 23) {
      const auto truth = bruteForce<T>(pos, pos[i], 1, i);
      CHECK_THAT(bvh.nearestNeighbor(i).distanceSquared, withinAbsT<T>(truth[0], tightMargin<T>()));
    }
  }
}
