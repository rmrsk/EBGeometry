// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for PointCloudHashGrid: the uniform-grid build and the high-level closest-point /
// nearest-neighbor query API. Every query is checked against an independent brute-force scan on a
// small random cloud, so both the grid and the expanding-shell query path (including its exact
// stopping rule, external queries, and queries outside the grid box) get real coverage in both
// precisions.

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

TEMPLATE_TEST_CASE("PointCloudHashGrid queries match brute force", "[PointCloudHashGrid]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr std::size_t n = 2000;

  const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 20260708u);
  std::vector<std::size_t>    meta(n);
  for (std::size_t i = 0; i < n; i++) {
    meta[i] = 7 * i + 3; // arbitrary user metadata, distinct from the cloud index
  }

  const PointCloudHashGrid<T, std::size_t> grid(pos, meta);

  REQUIRE(grid.numParticles() == n);

  const double tol = tightMargin<T>();

  SECTION("closestPoint (external query) matches brute force")
  {
    const std::vector<Vec3T<T>> queries = makeCloud<T>(200, 99u); // arbitrary external points
    for (const auto& q : queries) {
      const auto hit   = grid.closestPoint(q);
      const auto truth = bruteForce<T>(pos, q, 1, n); // exclude nothing (n is out of range)

      REQUIRE(hit.index < n);
      CHECK_THAT(std::sqrt(hit.distanceSquared), withinAbsT<T>(std::sqrt(truth[0]), tol));
      CHECK_THAT((pos[hit.index] - q).length2(), withinAbsT<T>(truth[0], tol));
    }
  }

  SECTION("query points outside the grid box still resolve correctly")
  {
    // Points well outside the unit cube exercise the clamped-cell + expanding-shell stopping rule.
    const std::vector<Vec3T<T>> outside = {Vec3T<T>(-3, T(0.5), T(0.5)),
                                           Vec3T<T>(T(0.5), 4, T(0.5)),
                                           Vec3T<T>(T(0.5), T(0.5), -2),
                                           Vec3T<T>(5, 5, 5),
                                           Vec3T<T>(-1, -1, -1)};
    for (const auto& q : outside) {
      const auto hit   = grid.closestPoint(q);
      const auto truth = bruteForce<T>(pos, q, 1, n);
      REQUIRE(hit.index < n);
      CHECK_THAT(hit.distanceSquared, withinAbsT<T>(truth[0], tol));
    }
  }

  SECTION("querying at a cloud point returns that point at distance 0")
  {
    for (std::size_t i = 0; i < n; i += 137) {
      const auto hit = grid.closestPoint(pos[i]);
      CHECK(hit.index == i);
      CHECK_THAT(hit.distanceSquared, withinAbsT<T>(T(0), tol));
    }
  }

  SECTION("nearestNeighbor (self query, excludes self) matches brute force")
  {
    for (std::size_t i = 0; i < n; i += 41) {
      const auto hit   = grid.nearestNeighbor(i);
      const auto truth = bruteForce<T>(pos, pos[i], 1, i);

      REQUIRE(hit.index < n);
      CHECK(hit.index != i);
      CHECK_THAT((pos[hit.index] - pos[i]).length2(), withinAbsT<T>(truth[0], tol));
    }
  }

  SECTION("k-nearest queries return k sorted, correct neighbors")
  {
    constexpr std::size_t k = 5;

    typename PointCloudHashGrid<T, std::size_t>::Hit out[k];

    for (std::size_t i = 0; i < n; i += 53) {
      const std::size_t found = grid.nearestNeighbors(i, k, out);
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
    const std::size_t found = grid.closestPoints(pos[0], k, out);
    REQUIRE(found == k);
    CHECK(out[0].index == 0);
    CHECK_THAT(out[0].distanceSquared, withinAbsT<T>(T(0), tol));
  }

  SECTION("allNearestNeighbors matches the single-query results")
  {
    const auto all = grid.allNearestNeighbors(1);
    REQUIRE(all.size() == n);
    for (std::size_t i = 0; i < n; i += 29) {
      const auto single = grid.nearestNeighbor(i);
      CHECK_THAT(all[i].distanceSquared, withinAbsT<T>(single.distanceSquared, tol));
    }
  }

  SECTION("brute-force reference methods match an independent scan (and the accelerated queries)")
  {
    constexpr std::size_t k = 4;

    const std::vector<Vec3T<T>>                      queries = makeCloud<T>(120, 4242u);
    typename PointCloudHashGrid<T, std::size_t>::Hit refOut[k];
    for (const auto& q : queries) {
      const auto ref   = grid.closestPointBruteForce(q);
      const auto truth = bruteForce<T>(pos, q, 1, n);
      REQUIRE(ref.index < n);
      CHECK_THAT(ref.distanceSquared, withinAbsT<T>(truth[0], tol));
      CHECK_THAT(grid.closestPoint(q).distanceSquared, withinAbsT<T>(ref.distanceSquared, tol));

      const std::size_t found  = grid.closestPointsBruteForce(q, k, refOut);
      const auto        truthK = bruteForce<T>(pos, q, k, n);
      REQUIRE(found == k);
      for (std::size_t j = 0; j < k; j++) {
        CHECK_THAT(refOut[j].distanceSquared, withinAbsT<T>(truthK[j], tol));
      }
    }

    for (std::size_t i = 0; i < n; i += 47) {
      const auto ref   = grid.nearestNeighborBruteForce(i);
      const auto truth = bruteForce<T>(pos, pos[i], 1, i);
      REQUIRE(ref.index < n);
      CHECK(ref.index != i);
      CHECK_THAT(ref.distanceSquared, withinAbsT<T>(truth[0], tol));
      CHECK_THAT(grid.nearestNeighbor(i).distanceSquared, withinAbsT<T>(ref.distanceSquared, tol));

      const std::size_t found  = grid.nearestNeighborsBruteForce(i, k, refOut);
      const auto        truthK = bruteForce<T>(pos, pos[i], k, i);
      REQUIRE(found == k);
      for (std::size_t j = 0; j < k; j++) {
        CHECK(refOut[j].index != i);
        CHECK_THAT(refOut[j].distanceSquared, withinAbsT<T>(truthK[j], tol));
      }
    }
  }

  SECTION("accessors return the stored cloud data")
  {
    for (std::size_t i = 0; i < n; i += 313) {
      CHECK(grid.metadata(i) == meta[i]);
      CHECK_THAT((grid.position(i) - pos[i]).length2(), withinAbsT<T>(T(0), tol));
    }
  }
}
