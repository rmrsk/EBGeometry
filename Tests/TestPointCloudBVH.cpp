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

template <class T>
using TestCloud = PointCloudBVH<T, std::size_t>;

template <class T>
using TestHit = typename TestCloud<T>::Hit;

// The PointCloudBVH queries the functors below run. The first three take an arbitrary query point, the
// last three a cloud point's own index.
enum class CloudQuery
{
  ClosestPoint,
  ClosestPointBruteForce,
  ClosestPoints,
  NearestNeighbor,
  NearestNeighborBruteForce,
  NearestNeighbors
};

// k for the k-nearest queries (closestPoints/nearestNeighbors).
constexpr std::size_t g_cloudK = 3;

/**
 * @brief One hit of a query at an arbitrary point: the nearest, or the a_rank-th nearest (0 = nearest)
 * for ClosestPoints. A self query, or a rank past the hits found, gives an invalid Hit.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
TestHit<T>
cloudHit(const TestCloud<T>& a_cloud,
         const CloudQuery    a_query,
         const std::size_t   a_rank,
         const Vec3T<T>&     a_point) noexcept
{
  TestHit<T> out[g_cloudK];

  switch (a_query) {
  case CloudQuery::ClosestPoint:
    return a_cloud.closestPoint(a_point);
  case CloudQuery::ClosestPointBruteForce:
    return a_cloud.closestPointBruteForce(a_point);
  case CloudQuery::ClosestPoints:
    return (a_rank < a_cloud.closestPoints(a_point, g_cloudK, out)) ? out[a_rank] : TestHit<T>{};
  default:
    return TestHit<T>{};
  }
}

/**
 * @brief One hit of a query about cloud point a_point: its nearest other point, or the a_rank-th nearest
 * (0 = nearest) for NearestNeighbors. A point query, or a rank past the hits found, gives an invalid Hit.
 */
template <class T>
EBGEOMETRY_HOST_DEVICE
TestHit<T>
cloudHit(const TestCloud<T>& a_cloud,
         const CloudQuery    a_query,
         const std::size_t   a_rank,
         const std::size_t&  a_point) noexcept
{
  TestHit<T> out[g_cloudK];

  switch (a_query) {
  case CloudQuery::NearestNeighbor:
    return a_cloud.nearestNeighbor(a_point);
  case CloudQuery::NearestNeighborBruteForce:
    return a_cloud.nearestNeighborBruteForce(a_point);
  case CloudQuery::NearestNeighbors:
    return (a_rank < a_cloud.nearestNeighbors(a_point, g_cloudK, out)) ? out[a_rank] : TestHit<T>{};
  default:
    return TestHit<T>{};
  }
}

// The functors below are what the device test evaluates, one query (a point, or a cloud index) per
// thread. They live outside the device-only guard so the host suite runs exactly the same code. Each
// holds the cloud by value, as a kernel receives it.

// The matched point's index for each query (Q = Vec3T<T> for the point queries, std::size_t for the
// self queries).
template <class T, class Q>
struct CloudHitIndexQuery
{
  TestCloud<T> m_cloud;
  CloudQuery   m_query;
  std::size_t  m_rank;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const Q& a_q) const noexcept
  {
    return cloudHit<T>(m_cloud, m_query, m_rank, a_q).index;
  }
};

// The squared distance to the matched point for each query.
template <class T, class Q>
struct CloudHitDistanceQuery
{
  TestCloud<T> m_cloud;
  CloudQuery   m_query;
  std::size_t  m_rank;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Q& a_q) const noexcept
  {
    return cloudHit<T>(m_cloud, m_query, m_rank, a_q).distanceSquared;
  }
};

// The number of hits closestPoints() returns for each query point.
template <class T>
struct ClosestPointsFoundQuery
{
  TestCloud<T> m_cloud;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    TestHit<T> out[g_cloudK];

    return m_cloud.closestPoints(a_point, g_cloudK, out);
  }
};

// The number of hits nearestNeighbors() returns for each cloud point.
template <class T>
struct NearestNeighborsFoundQuery
{
  TestCloud<T> m_cloud;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const std::size_t& a_point) const noexcept
  {
    TestHit<T> out[g_cloudK];

    return m_cloud.nearestNeighbors(a_point, g_cloudK, out);
  }
};

// Component m_axis of each cloud point's stored position.
template <class T>
struct CloudPositionQuery
{
  TestCloud<T> m_cloud;
  std::size_t  m_axis;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const std::size_t& a_point) const noexcept
  {
    return m_cloud.position(a_point)[m_axis];
  }
};

// Each cloud point's stored metadata.
template <class T>
struct CloudMetadataQuery
{
  TestCloud<T> m_cloud;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const std::size_t& a_point) const noexcept
  {
    return m_cloud.metadata(a_point);
  }
};

// The cloud's size, whatever the query.
template <class T>
struct CloudSizeQuery
{
  TestCloud<T> m_cloud;

  EBGEOMETRY_HOST_DEVICE
  std::size_t
  operator()(const std::size_t& /*a_point*/) const noexcept
  {
    return m_cloud.numPoints();
  }
};

/**
 * @brief Compare two clouds through every functor above.
 * @details For each functor, calls a_sameIndices(fFirst, fSecond, queries) for an integer result and
 * a_sameValues(fFirst, fSecond, queries) for a floating-point one, where fFirst holds a_first and
 * fSecond holds a_second. The callables decide where each runs and how closely the results must agree.
 */
template <class T, class SameIndices, class SameValues>
void
compareCloudQueries(const TestCloud<T>&             a_first,
                    const TestCloud<T>&             a_second,
                    const std::vector<Vec3T<T>>&    a_points,
                    const std::vector<std::size_t>& a_indices,
                    const SameIndices&              a_sameIndices,
                    const SameValues&               a_sameValues)
{
  using Vec3 = Vec3T<T>;

  const auto rankCount = [](const CloudQuery a_query) {
    return (a_query == CloudQuery::ClosestPoints || a_query == CloudQuery::NearestNeighbors) ? g_cloudK : 1;
  };

  for (const auto query : {CloudQuery::ClosestPoint, CloudQuery::ClosestPointBruteForce, CloudQuery::ClosestPoints}) {
    for (std::size_t rank = 0; rank < rankCount(query); rank++) {
      INFO("point query " << static_cast<int>(query) << ", rank " << rank);
      a_sameIndices(CloudHitIndexQuery<T, Vec3>{a_first, query, rank},
                    CloudHitIndexQuery<T, Vec3>{a_second, query, rank},
                    a_points);
      a_sameValues(CloudHitDistanceQuery<T, Vec3>{a_first, query, rank},
                   CloudHitDistanceQuery<T, Vec3>{a_second, query, rank},
                   a_points);
    }
  }

  for (const auto query :
       {CloudQuery::NearestNeighbor, CloudQuery::NearestNeighborBruteForce, CloudQuery::NearestNeighbors}) {
    for (std::size_t rank = 0; rank < rankCount(query); rank++) {
      INFO("self query " << static_cast<int>(query) << ", rank " << rank);
      a_sameIndices(CloudHitIndexQuery<T, std::size_t>{a_first, query, rank},
                    CloudHitIndexQuery<T, std::size_t>{a_second, query, rank},
                    a_indices);
      a_sameValues(CloudHitDistanceQuery<T, std::size_t>{a_first, query, rank},
                   CloudHitDistanceQuery<T, std::size_t>{a_second, query, rank},
                   a_indices);
    }
  }

  a_sameIndices(ClosestPointsFoundQuery<T>{a_first}, ClosestPointsFoundQuery<T>{a_second}, a_points);
  a_sameIndices(NearestNeighborsFoundQuery<T>{a_first}, NearestNeighborsFoundQuery<T>{a_second}, a_indices);

  for (std::size_t axis = 0; axis < 3; axis++) {
    INFO("axis " << axis);
    a_sameValues(CloudPositionQuery<T>{a_first, axis}, CloudPositionQuery<T>{a_second, axis}, a_indices);
  }

  a_sameIndices(CloudMetadataQuery<T>{a_first}, CloudMetadataQuery<T>{a_second}, a_indices);
  a_sameIndices(CloudSizeQuery<T>{a_first}, CloudSizeQuery<T>{a_second}, a_indices);
}

// The cloud the query-functor tests below share: 500 points in the unit cube, with metadata.
template <class T>
TestCloud<T>
makeFunctorTestCloud(Pool& a_pool)
{
  constexpr std::size_t n = 500;

  const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 4321u);
  std::vector<std::size_t>    meta(n);

  for (std::size_t i = 0; i < n; i++) {
    meta[i] = i % 17;
  }

  return TestCloud<T>(a_pool, pos, meta, 8);
}

// Query points around and inside the unit-cube cloud.
template <class T>
std::vector<Vec3T<T>>
functorTestPoints()
{
  std::vector<Vec3T<T>> points;

  // A grid over [-0.25, 1.25]^3, offset so that no point lies on a grid-aligned plane.
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      for (int k = 0; k < 10; k++) {
        points.emplace_back(T(-0.25) + T(0.15) * (T(i) + T(0.37)),
                            T(-0.25) + T(0.15) * (T(j) + T(0.53)),
                            T(-0.25) + T(0.15) * (T(k) + T(0.61)));
      }
    }
  }

  return points;
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

namespace {

// Stands in for CUDA/HIP managed memory without a GPU: memory both the host and a device can address,
// so a rebasedView() onto a mirror in it is a device view and is checked against the device depth.
class FakeManagedResource final : public MemoryResource
{
public:
  void*
  allocate(size_t a_bytes, size_t a_alignment) override
  {
    return hostMemoryResource().allocate(a_bytes, a_alignment);
  }

  void
  deallocate(void* a_ptr, size_t a_bytes, size_t a_alignment) noexcept override
  {
    hostMemoryResource().deallocate(a_ptr, a_bytes, a_alignment);
  }

  bool
  isHostAccessible() const noexcept override
  {
    return true;
  }

  bool
  isDeviceAccessible() const noexcept override
  {
    return true;
  }
};

} // namespace

TEMPLATE_TEST_CASE("PointCloudBVH: many coincident points build a tree a device can traverse, and queries stay exact",
                   "[PointCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Cloud = PointCloudBVH<T, std::size_t>;
  using Hit   = typename Cloud::Hit;

  // Two stacks of coincident points (no split can separate a stack) and a handful of outliers.
  std::vector<Vec3T<T>> pos(3000, Vec3T<T>(T(0.5), T(0.5), T(0.5)));

  pos.insert(pos.end(), 1000, Vec3T<T>(T(0.25), T(0.75), T(0.5)));

  const std::vector<Vec3T<T>> outliers = makeCloud<T>(12, 4242u);

  pos.insert(pos.end(), outliers.begin(), outliers.end());

  const std::size_t              n = pos.size();
  const std::vector<std::size_t> meta(n, 0);
  const std::vector<Vec3T<T>>    queries = makeCloud<T>(200, 99u);

  constexpr std::size_t k = 3;

  for (const std::size_t leafSize : {std::size_t(1), std::size_t(3), std::size_t(64)}) {
    INFO("targetLeafSize = " << leafSize);

    Pool pool(hostMemoryResource());

    const Cloud cloud(pool, pos, meta, leafSize);

    const auto nodes = cloud.getBVH().getNodes();

    REQUIRE(nodes.size() > 0);
    REQUIRE(BVH::treeDepth(nodes.begin(), nodes.size()) <= BVH::DeviceTraversalDepth);

    // Arbitrary query points: the closest point and the k closest, against brute force.
    for (const auto& q : queries) {
      const auto truth = bruteForce<T>(pos, q, k, n);

      REQUIRE_THAT(cloud.closestPoint(q).distanceSquared, withinAbsT<T>(truth[0], tightMargin<T>()));

      Hit out[k];

      REQUIRE(cloud.closestPoints(q, k, out) == k);

      for (std::size_t j = 0; j < k; j++) {
        REQUIRE_THAT(out[j].distanceSquared, withinAbsT<T>(truth[j], tightMargin<T>()));
      }
    }

    // Self queries on the outliers and on a sample of the stacked points, against brute force.
    for (std::size_t i = 0; i < n; i += (i < 4000 ? 97 : 1)) {
      const auto truth = bruteForce<T>(pos, pos[i], k, i);

      REQUIRE_THAT(cloud.nearestNeighbor(i).distanceSquared, withinAbsT<T>(truth[0], tightMargin<T>()));

      Hit out[k];

      REQUIRE(cloud.nearestNeighbors(i, k, out) == k);

      for (std::size_t j = 0; j < k; j++) {
        REQUIRE(out[j].index != i);
        REQUIRE_THAT(out[j].distanceSquared, withinAbsT<T>(truth[j], tightMargin<T>()));
      }
    }

    // A view onto a device-accessible mirror is checked against the device traversal depth; it must
    // be accepted, and answer as the host tree does.
    pool.freeze();

    FakeManagedResource managed;
    const Pool          mirror = Pool::mirror(pool, managed);
    const Cloud         view   = cloud.rebasedView(mirror);

    for (const auto& q : queries) {
      const Hit lhs = cloud.closestPoint(q);
      const Hit rhs = view.closestPoint(q);

      REQUIRE(rhs.index == lhs.index);
      REQUIRE(rhs.distanceSquared == lhs.distanceSquared);
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

TEMPLATE_TEST_CASE("PointCloudBVH: the device query functors agree across a host-to-host rebase",
                   "[PointCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const TestCloud<T> cloud = makeFunctorTestCloud<T>(pool);

  pool.freeze();

  Pool               mirror = Pool::mirror(pool, hostMemoryResource());
  const TestCloud<T> view   = cloud.rebasedView(mirror);

  const std::vector<Vec3T<T>> points = functorTestPoints<T>();
  std::vector<std::size_t>    indices(cloud.numPoints());

  for (std::size_t i = 0; i < indices.size(); i++) {
    indices[i] = i;
  }

  // Runs the device test's functors on the host, against a rebased descriptor passed by value -- the
  // host-side analogue of what the kernel receives -- so they are compiled and checked on every build,
  // not only GPU ones. Same code, same data: every result must agree bit for bit.
  const auto same = [](const auto& a_first, const auto& a_second, const auto& a_queries) {
    for (std::size_t i = 0; i < a_queries.size(); i++) {
      INFO("query " << i);
      REQUIRE(a_first(a_queries[i]) == a_second(a_queries[i]));
    }
  };

  compareCloudQueries<T>(view, cloud, points, indices, same, same);

  // The queries find real points, so the comparison above is not between two invalid hits.
  REQUIRE(CloudHitIndexQuery<T, Vec3T<T>>{cloud, CloudQuery::ClosestPoint, 0}(points[0]) < cloud.numPoints());
  REQUIRE(CloudHitIndexQuery<T, std::size_t>{cloud, CloudQuery::NearestNeighbors, g_cloudK - 1}(std::size_t(0)) <
          cloud.numPoints());
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

  const TestCloud<T> cloud = makeFunctorTestCloud<T>(pool);

  pool.freeze();

  Pool               devicePool = Pool::mirror(pool, deviceTestResource());
  const TestCloud<T> deviceView = cloud.rebasedView(devicePool);

  // 1000 points over [-0.25, 1.25]^3, around and inside the unit-cube cloud, and every cloud index.
  const std::vector<Vec3T<T>> points =
    queryGrid<T>(Vec3T<T>(T(-0.25), T(-0.25), T(-0.25)), Vec3T<T>(T(1.25), T(1.25), T(1.25)), 10);
  std::vector<std::size_t> indices(cloud.numPoints());

  for (std::size_t i = 0; i < indices.size(); i++) {
    indices[i] = i;
  }

  // Indices, counts and metadata must match exactly; distances and positions to requireSameResults'
  // tolerance, since the device may contract or reorder the arithmetic.
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

  compareCloudQueries<T>(deviceView, cloud, points, indices, sameIndices, sameValues);
}

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
    "PointCloudBVH: the target leaf size must be at least 1 and fit in 32 bits (0)"));

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
