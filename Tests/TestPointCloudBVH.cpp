// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for PointCloudBVH: the index-based build and the high-level closest-point / nearest-
// neighbor query API. Every query is checked against a brute-force O(N^2) scan on a small random
// cloud, so both the tree and the seed-from-own-leaf query path get real coverage in both precisions.
// The query functors shared with the PointCloudHashGrid tests are in TestPointCloudQueries.hpp.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestPointCloudQueries.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <set>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestPointCloud;

namespace {

template <class T>
using TestCloud = PointCloudBVH<T>;

} // namespace

TEMPLATE_TEST_CASE("PointCloudBVH queries match brute force", "[PointCloudBVH]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr std::size_t n = 2000;

  Pool pool(hostMemoryResource());

  const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 20260708u);

  const PointCloudBVH<T> bvh(pool, pos);

  REQUIRE(bvh.numPoints() == n);

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
    for (uint32_t i = 0; i < n; i += 137) {
      const auto hit = bvh.closestPoint(pos[i]);
      CHECK(hit.index == i);
      CHECK_THAT(hit.distanceSquared, withinAbsT<T>(T(0), tol));
    }
  }

  SECTION("nearestNeighbor (self query, excludes self) matches brute force")
  {
    for (uint32_t i = 0; i < n; i += 41) {
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

    typename PointCloudBVH<T>::Hit out[k];

    for (uint32_t i = 0; i < n; i += 53) {
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
    for (uint32_t i = 0; i < n; i += 29) {
      const auto single = bvh.nearestNeighbor(i);
      CHECK_THAT(all[i].distanceSquared, withinAbsT<T>(single.distanceSquared, tol));
    }
  }

  SECTION("accessors return the stored cloud data")
  {
    for (uint32_t i = 0; i < n; i += 313) {
      CHECK_THAT((bvh.position(i) - pos[i]).length2(), withinAbsT<T>(T(0), tol));
    }
  }
}

TEMPLATE_TEST_CASE("PointCloudBVH edge cases", "[PointCloudBVH]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  using Hit = typename PointCloudBVH<T>::Hit;

  Pool pool(hostMemoryResource());

  const T notFound = std::numeric_limits<T>::max();

  SECTION("empty cloud: no out-of-bounds access, queries report nothing")
  {
    const std::vector<Vec3T<T>> pos;
    const PointCloudBVH<T>      bvh(pool, pos);

    CHECK(bvh.numPoints() == 0);
    // Must not read m_linearNodes[0]; a miss is signalled by the sentinel distance.
    CHECK(bvh.closestPoint(Vec3T<T>(T(0), T(0), T(0))).distanceSquared == notFound);
    Hit               out[4];
    const std::size_t found = bvh.closestPoints(Vec3T<T>(T(0), T(0), T(0)), 4, out);
    CHECK(found == 0);
    CHECK(bvh.allNearestNeighbors(1).empty());
  }

  SECTION("single particle: external query finds it, self query finds nothing")
  {
    const std::vector<Vec3T<T>> pos = {Vec3T<T>(T(0.25), T(0.5), T(0.75))};
    const PointCloudBVH<T>      bvh(pool, pos);

    const auto hit = bvh.closestPoint(pos[0]);
    CHECK(hit.index == 0);
    CHECK_THAT(hit.distanceSquared, withinAbsT<T>(T(0), tightMargin<T>()));

    // nearestNeighbor excludes self, so with only one particle there is no neighbor.
    CHECK(bvh.nearestNeighbor(0).distanceSquared == notFound);
    CHECK_FALSE(bvh.nearestNeighbor(0).valid());
    CHECK(hit.valid());

    // Rows of allNearestNeighbors that cannot be filled hold "no match" results.
    const auto all = bvh.allNearestNeighbors(3);
    REQUIRE(all.size() == 3);
    CHECK_FALSE(all[0].valid());
    CHECK_FALSE(all[2].valid());
  }

  SECTION("deep tree (small leaves) still resolves seeded queries correctly")
  {
    // A small target leaf size forces many BVH levels, exercising the seeded fast-path DFS (and its
    // stack) far more than the default. Every self query is checked against brute force.
    constexpr std::size_t       n   = 3000;
    const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 555u);
    const PointCloudBVH<T>      bvh(pool, pos, /* targetLeafSize */ 2);

    for (uint32_t i = 0; i < n; i += 23) {
      const auto truth = bruteForce<T>(pos, pos[i], 1, i);
      CHECK_THAT(bvh.nearestNeighbor(i).distanceSquared, withinAbsT<T>(truth[0], tightMargin<T>()));
    }
  }

  SECTION("coincident points: many copies of one point beside a few others build a shallow tree")
  {
    // A midpoint split cannot separate points that share a coordinate. Real clouds have such
    // duplicates (repeated vertices, grid-snapped samples); the build must still terminate with a
    // tree shallow enough for the traversal stack, for the default and for a tiny leaf size.
    for (const std::size_t leafSize : {std::size_t(2), std::size_t(64)}) {
      std::vector<Vec3T<T>> pos(5000, Vec3T<T>(T(0.5), T(0.5), T(0.5)));

      pos.emplace_back(T(0), T(0), T(0));
      pos.emplace_back(T(1), T(0.25), T(0));

      const PointCloudBVH<T> bvh(pool, pos, leafSize);

      const auto nearOrigin = bvh.nearestNeighbor(5000);
      CHECK(nearOrigin.index < 5000);
      CHECK_THAT(nearOrigin.distanceSquared, withinAbsT<T>(T(0.75), tightMargin<T>()));

      const auto duplicate = bvh.nearestNeighbor(17);
      CHECK_THAT(duplicate.distanceSquared, withinAbsT<T>(T(0), tightMargin<T>()));
    }
  }

  SECTION("padded lanes never report a point twice, even when k exceeds the cloud")
  {
    // Five points in groups of W = 4 leave three padded lanes that repeat the last point. The leaf
    // scan stops at each group's real points, so k larger than the cloud returns every point once.
    const std::vector<Vec3T<T>>  pos = makeCloud<T>(5, 31u);
    const PointCloudBVH<T, 4, 4> bvh(pool, pos);

    Hit out[10];

    const std::size_t found = bvh.closestPoints(Vec3T<T>(T(0.5), T(0.5), T(0.5)), 10, out);

    REQUIRE(found == 5);

    std::set<uint32_t> seen;

    for (std::size_t j = 0; j < found; j++) {
      seen.insert(out[j].index);
    }

    CHECK(seen.size() == 5);
    CHECK_FALSE(out[5].valid());

    REQUIRE(bvh.nearestNeighbors(0, 10, out) == 4);

    seen.clear();

    for (std::size_t j = 0; j < 4; j++) {
      seen.insert(out[j].index);
    }

    CHECK(seen.size() == 4);
    CHECK(seen.count(0) == 0);
  }
}

TEMPLATE_TEST_CASE("PointCloudBVH: rebasedView and deepCopy stay PointCloudBVHs and answer identically",
                   "[PointCloudBVH][Pool][rebase]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Cloud = PointCloudBVH<T>;
  using Hit   = typename Cloud::Hit;

  // Both used to compile and return a sliced PackedBVH, silently dropping every point-cloud query.
  static_assert(std::is_same_v<decltype(std::declval<const Cloud&>().rebasedView(std::declval<const Pool&>())), Cloud>);
  static_assert(std::is_same_v<decltype(std::declval<const Cloud&>().deepCopy(std::declval<Pool&>())), Cloud>);
  static_assert(std::is_trivially_copyable_v<Cloud>);

  constexpr std::size_t n = 700;
  constexpr std::size_t k = 4;

  const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 777u);

  const std::vector<Vec3T<T>> queries = makeCloud<T>(40, 31u);

  // Every query result of a_other must match a_cloud's exactly: same tree, same data.
  const auto requireSameAnswers = [&](const Cloud& a_cloud, const Cloud& a_other) {
    REQUIRE(a_other.numPoints() == a_cloud.numPoints());

    for (uint32_t i = 0; i < n; i += 11) {
      REQUIRE(a_other.position(i) == a_cloud.position(i));

      const Hit lhs = a_cloud.nearestNeighbor(i);
      const Hit rhs = a_other.nearestNeighbor(i);

      REQUIRE(rhs.index == lhs.index);
      REQUIRE(rhs.distanceSquared == lhs.distanceSquared);
    }

    for (const auto& q : queries) {
      Hit lhs[k];
      Hit rhs[k];

      REQUIRE(a_other.closestPoints(q, k, rhs) == a_cloud.closestPoints(q, k, lhs));

      for (std::size_t j = 0; j < k; j++) {
        REQUIRE(rhs[j].index == lhs[j].index);
        REQUIRE(rhs[j].distanceSquared == lhs[j].distanceSquared);
      }
    }

    const auto lhsAll = a_cloud.allNearestNeighbors(2);
    const auto rhsAll = a_other.allNearestNeighbors(2);

    REQUIRE(rhsAll.size() == lhsAll.size());

    for (std::size_t i = 0; i < lhsAll.size(); i++) {
      REQUIRE(rhsAll[i].index == lhsAll[i].index);
    }
  };

  Pool pool(hostMemoryResource());

  const Cloud cloud(pool, pos);

  REQUIRE(cloud.isAttachedTo(pool));
  REQUIRE(cloud.getBVH().isAttachedTo(pool));

  // Every one of the cloud's own answers is still checked against brute force, so the comparisons
  // below are against a known-good reference rather than merely self-consistent.
  for (uint32_t i = 0; i < n; i += 37) {
    const auto truth = bruteForce<T>(pos, pos[i], 1, i);

    REQUIRE_THAT(cloud.nearestNeighbor(i).distanceSquared, withinAbsT<T>(truth[0], tightMargin<T>()));
  }

  SECTION("deepCopy into a separate pool is independent storage")
  {
    Pool other(hostMemoryResource());

    const Cloud copy = cloud.deepCopy(other);

    REQUIRE(copy.isAttachedTo(other));
    REQUIRE_FALSE(copy.isAttachedTo(pool));
    requireSameAnswers(cloud, copy);
  }

  SECTION("deepCopy into its own pool survives the pool growing under it")
  {
    // The copy's reserves can grow (and so move) the very block the source reads from.
    const Cloud copy = cloud.deepCopy(pool);

    REQUIRE(copy.isAttachedTo(pool));
    REQUIRE(copy.base() == cloud.base());
    requireSameAnswers(cloud, copy);
  }

  SECTION("a host-to-host rebasedView resolves against the mirror")
  {
    pool.freeze();

    Pool        mirror = Pool::mirror(pool, hostMemoryResource());
    const Cloud view   = cloud.rebasedView(mirror);

    REQUIRE(view.isAttachedTo(mirror));
    REQUIRE(view.base() == mirror.base());
    REQUIRE(view.base() != cloud.base());
    requireSameAnswers(cloud, view);
  }
}

TEMPLATE_TEST_CASE("PointCloudBVH: the device query functors agree across a host-to-host rebase",
                   "[PointCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const TestCloud<T> cloud(pool, functorTestCloud<T>(), 8);

  pool.freeze();

  Pool               mirror = Pool::mirror(pool, hostMemoryResource());
  const TestCloud<T> view   = cloud.rebasedView(mirror);

  const std::vector<Vec3T<T>> points  = functorTestPoints<T>();
  const std::vector<uint32_t> indices = allIndices(cloud.numPoints());

  // Runs the device test's functors on the host, against a rebased descriptor passed by value -- the
  // host-side analogue of what the kernel receives -- so they are compiled and checked on every build,
  // not only GPU ones. Same code, same data: every result must agree bit for bit.
  const auto same = [](const auto& a_first, const auto& a_second, const auto& a_queries) {
    for (std::size_t i = 0; i < a_queries.size(); i++) {
      INFO("query " << i);
      REQUIRE(a_first(a_queries[i]) == a_second(a_queries[i]));
    }
  };

  compareCloudQueries<TestCloud<T>, T>(view, cloud, points, indices, same, same);

  // The queries find real points, so the comparison above is not between two invalid hits.
  using PointQuery = CloudHitIndexQuery<TestCloud<T>, T, Vec3T<T>>;
  using SelfQuery  = CloudHitIndexQuery<TestCloud<T>, T, uint32_t>;

  REQUIRE(PointQuery{cloud, CloudQuery::ClosestPoint, 0}(points[0]) < cloud.numPoints());
  REQUIRE(SelfQuery{cloud, CloudQuery::NearestNeighbors, g_cloudK - 1}(uint32_t(0)) < cloud.numPoints());
}

// ─────────────────────────────────────────────────────────────────────────────
// Device: a rebased PointCloudBVH answers queries in a kernel and agrees with the host
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("PointCloudBVH: a rebased view answers queries on device and matches the host",
                   "[PointCloudBVH][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const TestCloud<T> cloud(pool, functorTestCloud<T>(), 8);

  pool.freeze();

  Pool               devicePool = Pool::mirror(pool, deviceTestResource());
  const TestCloud<T> deviceView = cloud.rebasedView(devicePool);

  // 1000 points over [-0.25, 1.25]^3, around and inside the unit-cube cloud, and every cloud index.
  const std::vector<Vec3T<T>> points =
    queryGrid<T>(Vec3T<T>(T(-0.25), T(-0.25), T(-0.25)), Vec3T<T>(T(1.25), T(1.25), T(1.25)), 10);
  const std::vector<uint32_t> indices = allIndices(cloud.numPoints());

  // Indices and counts must match exactly; distances and positions to requireSameResults' tolerance,
  // since the device may contract or reorder the arithmetic.
  const auto sameIndices = [](const auto& a_device, const auto& a_host, const auto& a_queries) {
    const std::vector<std::size_t> device = evaluateOnDevice<std::size_t>(a_device, a_queries);
    const std::vector<std::size_t> host   = evaluateOnHost<std::size_t>(a_host, a_queries);

    REQUIRE(device.size() == host.size());

    for (std::size_t i = 0; i < host.size(); i++) {
      INFO("query " << i);
      REQUIRE(device[i] == host[i]);
    }
  };

  const auto sameValues = [](const auto& a_device, const auto& a_host, const auto& a_queries) {
    requireSameResults(evaluateOnDevice<T>(a_device, a_queries), evaluateOnHost<T>(a_host, a_queries));
  };

  compareCloudQueries<TestCloud<T>, T>(deviceView, cloud, points, indices, sameIndices, sameValues);
}

TEST_CASE("PointCloudBVH and PointCloudHashGrid reject a cloud they cannot index", "[PointCloudBVH][death]")
{
  using T = double;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  const std::vector<Vec3T<T>> bad = {Vec3T<T>(T(0), T(0), T(0)),
                                     Vec3T<T>(T(1), std::numeric_limits<T>::quiet_NaN(), T(0))};

  REQUIRE(abortsWith(
    [&bad] {
      Pool                   pool(hostMemoryResource());
      const PointCloudBVH<T> bvh(pool, bad);
    },
    "PointCloudBVH: point 1 of 2 has a non-finite coordinate"));

  REQUIRE(abortsWith(
    [&bad] {
      Pool                        pool(hostMemoryResource());
      const PointCloudHashGrid<T> grid(pool, bad);
    },
    "PointCloudHashGrid: point 1 of 2 has a non-finite coordinate"));
}

TEST_CASE("PointCloudBVH: rejects a zero leaf size, and a rebase onto a pool too small to hold it",
          "[PointCloudBVH][death]")
{
  using T = double;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  const std::vector<Vec3T<T>> pos = {Vec3T<T>(T(0), T(0), T(0)), Vec3T<T>(T(1), T(0), T(0))};

  REQUIRE(abortsWith(
    [&pos] {
      Pool                   pool(hostMemoryResource());
      const PointCloudBVH<T> bvh(pool, pos, 0);
    },
    "PointCloudBVH: the target leaf size must be at least 1 (0)"));

  REQUIRE(abortsWith(
    [&pos] {
      Pool                   pool(hostMemoryResource());
      Pool                   unrelated(hostMemoryResource());
      const PointCloudBVH<T> bvh(pool, pos);
      const PointCloudBVH<T> view = bvh.rebasedView(unrelated);

      (void)view;
    },
    "PointCloudBVH::rebasedView: the cloud's arrays must fit inside the pool"));
}
