// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for PointCloudBVH: the index-based build and the high-level closest-point / nearest-
// neighbor query API. Every query is checked against a brute-force O(N^2) scan on a small random
// cloud, so both the tree and the seed-from-own-leaf query path get real coverage in both precisions.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

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

// Non-default user metadata, to check it round-trips through pool storage field by field.
struct TagMeta
{
  std::int32_t m_id;
  float        m_weight;
};

/**
 * @brief Fold every device-callable query into one scalar.
 * @details The whole body of the device kernel below, factored out so the host suite runs exactly
 * the code the kernel runs. Takes the cloud by value and const, as the kernel does.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
T
pointCloudProbe(const PointCloudBVH<T, std::size_t> a_cloud, const Vec3T<T> a_query) noexcept
{
  using Hit = typename PointCloudBVH<T, std::size_t>::Hit;

  constexpr std::size_t k = 3;

  Hit out[k];

  T sum = a_cloud.closestPoint(a_query).distanceSquared + a_cloud.closestPointBruteForce(a_query).distanceSquared;

  const std::size_t foundExternal = a_cloud.closestPoints(a_query, k, out);

  for (std::size_t j = 0; j < foundExternal; j++) {
    sum += out[j].distanceSquared + T(out[j].index);
  }

  for (std::size_t i = 0; i < a_cloud.numPoints(); i += 7) {
    const Hit nn = a_cloud.nearestNeighbor(i);

    sum += nn.distanceSquared + T(nn.index) + a_cloud.nearestNeighborBruteForce(i).distanceSquared;

    const std::size_t foundSelf = a_cloud.nearestNeighbors(i, k, out);

    for (std::size_t j = 0; j < foundSelf; j++) {
      sum += out[j].distanceSquared;
    }

    sum += a_cloud.position(i).length() + T(a_cloud.metadata(i));
  }

  return sum;
}

} // namespace

TEMPLATE_TEST_CASE("PointCloudBVH queries match brute force", "[PointCloudBVH]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr std::size_t n = 2000;

  Pool pool(hostMemoryResource());

  const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 20260708u);
  std::vector<std::size_t>    meta(n);
  for (std::size_t i = 0; i < n; i++) {
    meta[i] = 7 * i + 3; // arbitrary user metadata, distinct from the cloud index
  }

  const PointCloudBVH<T, std::size_t> bvh(pool, pos, meta);

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

  Pool pool(hostMemoryResource());

  const T notFound = std::numeric_limits<T>::max();

  SECTION("empty cloud: no out-of-bounds access, queries report nothing")
  {
    const std::vector<Vec3T<T>>         pos;
    const std::vector<std::size_t>      meta;
    const PointCloudBVH<T, std::size_t> bvh(pool, pos, meta);

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
    const std::vector<Vec3T<T>>         pos  = {Vec3T<T>(T(0.25), T(0.5), T(0.75))};
    const std::vector<std::size_t>      meta = {42};
    const PointCloudBVH<T, std::size_t> bvh(pool, pos, meta);

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
    constexpr std::size_t               n   = 3000;
    const std::vector<Vec3T<T>>         pos = makeCloud<T>(n, 555u);
    std::vector<std::size_t>            meta(n, 0);
    const PointCloudBVH<T, std::size_t> bvh(pool, pos, meta, /* targetLeafSize */ 2);

    for (std::size_t i = 0; i < n; i += 23) {
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

      const std::vector<std::size_t>      meta(pos.size(), 0);
      const PointCloudBVH<T, std::size_t> bvh(pool, pos, meta, leafSize);

      const auto nearOrigin = bvh.nearestNeighbor(5000);
      CHECK(nearOrigin.index < 5000);
      CHECK_THAT(nearOrigin.distanceSquared, withinAbsT<T>(T(0.75), tightMargin<T>()));

      const auto duplicate = bvh.nearestNeighbor(17);
      CHECK_THAT(duplicate.distanceSquared, withinAbsT<T>(T(0), tightMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("PointCloudBVH: rebasedView and deepCopy stay PointCloudBVHs and answer identically",
                   "[PointCloudBVH][Pool][rebase]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Cloud = PointCloudBVH<T, TagMeta>;
  using Hit   = typename Cloud::Hit;

  // Both used to compile and return a sliced PackedBVH, silently dropping every point-cloud query.
  static_assert(std::is_same_v<decltype(std::declval<const Cloud&>().rebasedView(std::declval<const Pool&>())), Cloud>);
  static_assert(std::is_same_v<decltype(std::declval<const Cloud&>().deepCopy(std::declval<Pool&>())), Cloud>);
  static_assert(std::is_trivially_copyable_v<Cloud>);

  constexpr std::size_t n = 700;
  constexpr std::size_t k = 4;

  const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 777u);
  std::vector<TagMeta>        meta(n);

  for (std::size_t i = 0; i < n; i++) {
    meta[i] = TagMeta{static_cast<std::int32_t>(3 * i + 1), float(i) * 0.5F};
  }

  const std::vector<Vec3T<T>> queries = makeCloud<T>(40, 31u);

  // Every query result of a_other must match a_cloud's exactly: same tree, same data.
  const auto requireSameAnswers = [&](const Cloud& a_cloud, const Cloud& a_other) {
    REQUIRE(a_other.numPoints() == a_cloud.numPoints());

    for (std::size_t i = 0; i < n; i += 11) {
      REQUIRE(a_other.position(i) == a_cloud.position(i));
      REQUIRE(a_other.metadata(i).m_id == a_cloud.metadata(i).m_id);
      REQUIRE(a_other.metadata(i).m_weight == a_cloud.metadata(i).m_weight);

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

  const Cloud cloud(pool, pos, meta);

  REQUIRE(cloud.isAttachedTo(pool));
  REQUIRE(cloud.getBVH().isAttachedTo(pool));

  // Every one of the cloud's own answers is still checked against brute force, so the comparisons
  // below are against a known-good reference rather than merely self-consistent.
  for (std::size_t i = 0; i < n; i += 37) {
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

TEMPLATE_TEST_CASE("PointCloudBVH: the device kernel's query probe agrees across a host-to-host rebase",
                   "[PointCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  constexpr std::size_t n = 500;

  const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 4321u);
  std::vector<std::size_t>    meta(n);

  for (std::size_t i = 0; i < n; i++) {
    meta[i] = i % 17;
  }

  const PointCloudBVH<T, std::size_t> cloud(pool, pos, meta, 8);

  const Vec3T<T> query(T(0.3), T(0.7), T(0.45));

  // Instantiates the kernel's probe on every build, not only GPU ones, and runs it against a
  // rebased descriptor passed by value -- the host-side analogue of what the kernel receives.
  const T probe = pointCloudProbe<T>(cloud, query);

  pool.freeze();

  Pool mirror = Pool::mirror(pool, hostMemoryResource());

  REQUIRE(std::isfinite(probe));
  REQUIRE(pointCloudProbe<T>(cloud.rebasedView(mirror), query) == probe);
}

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
// ─────────────────────────────────────────────────────────────────────────────
// Device: a rebased PointCloudBVH answers queries in a kernel and agrees with the host
// ─────────────────────────────────────────────────────────────────────────────

template <class T>
EBGEOMETRY_GLOBAL
void
pointCloudDeviceKernel(const PointCloudBVH<T, std::size_t> a_cloud, Vec3T<T> a_query, T* a_out)
{
  a_out[0] = pointCloudProbe<T>(a_cloud, a_query);
}

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

  constexpr std::size_t n = 500;

  const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 4321u);
  std::vector<std::size_t>    meta(n);

  for (std::size_t i = 0; i < n; i++) {
    meta[i] = i % 17;
  }

  const PointCloudBVH<T, std::size_t> cloud(pool, pos, meta, 8);

  const Vec3T<T> query(T(0.3), T(0.7), T(0.45));

  const T hostVal = pointCloudProbe<T>(cloud, query);

  pool.freeze();

  Pool                                devicePool = Pool::mirror(pool, deviceMemoryResource());
  const PointCloudBVH<T, std::size_t> deviceView = cloud.rebasedView(devicePool);

  DeviceBuffer<T> deviceOut;

  pointCloudDeviceKernel<T><<<1, 1>>>(deviceView, query, deviceOut.get());
  (void)GPU::deviceSynchronize();

  REQUIRE_THAT(readScalar(deviceOut.get()), Catch::Matchers::WithinRel(hostVal, gpuTol<T>()));
}
#endif

TEST_CASE("PointCloudBVH and PointCloudHashGrid reject a cloud they cannot index", "[PointCloudBVH][death]")
{
  using T = double;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  const std::vector<Vec3T<T>> pos = {Vec3T<T>(T(0), T(0), T(0)), Vec3T<T>(T(1), T(0), T(0))};

  REQUIRE(abortsWith(
    [&pos] {
      Pool                                pool(hostMemoryResource());
      const std::vector<std::size_t>      tooShort = {0};
      const PointCloudBVH<T, std::size_t> bvh(pool, pos, tooShort);
    },
    "PointCloudBVH: need one metadata entry per point (1 metadata entries, 2 points)"));

  REQUIRE(abortsWith(
    [&pos] {
      const std::vector<std::size_t>           tooShort = {0};
      const PointCloudHashGrid<T, std::size_t> grid(pos, tooShort);
    },
    "PointCloudHashGrid: need one metadata entry per point"));

  REQUIRE(abortsWith(
    [] {
      Pool                                pool(hostMemoryResource());
      const std::vector<Vec3T<T>>         bad  = {Vec3T<T>(T(0), std::numeric_limits<T>::quiet_NaN(), T(0))};
      const std::vector<std::size_t>      meta = {0};
      const PointCloudBVH<T, std::size_t> bvh(pool, bad, meta);
    },
    "PointCloudBVH: point 0 of 1 has a non-finite coordinate"));
}

TEST_CASE("PointCloudBVH: rejects a zero leaf size, and a rebase onto a pool too small to hold it",
          "[PointCloudBVH][death]")
{
  using T = double;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  const std::vector<Vec3T<T>>    pos  = {Vec3T<T>(T(0), T(0), T(0)), Vec3T<T>(T(1), T(0), T(0))};
  const std::vector<std::size_t> meta = {0, 1};

  REQUIRE(abortsWith(
    [&pos, &meta] {
      Pool                                pool(hostMemoryResource());
      const PointCloudBVH<T, std::size_t> bvh(pool, pos, meta, 0);
    },
    "PointCloudBVH: the target leaf size must be at least 1 (0)"));

  REQUIRE(abortsWith(
    [&pos, &meta] {
      Pool                                pool(hostMemoryResource());
      Pool                                unrelated(hostMemoryResource());
      const PointCloudBVH<T, std::size_t> bvh(pool, pos, meta);
      const PointCloudBVH<T, std::size_t> view = bvh.rebasedView(unrelated);

      (void)view;
    },
    "PointCloudBVH::rebasedView: the cloud's arrays must fit inside the pool"));
}
