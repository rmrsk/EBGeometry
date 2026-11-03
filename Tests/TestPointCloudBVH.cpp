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

// ─────────────────────────────────────────────────────────────────────────────
// build() where the pool's memory is, and rebuild(): one tree on every side
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Memory a build through GPU::parallelFor() writes and the host reads: managed memory in a CUDA or HIP build,
// where the build's steps run as kernels, and emulated device memory otherwise, where they run as host loops.
MemoryResource&
parallelBuildResource() noexcept
{
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  return managedMemoryResource();
#else
  return emulatedDeviceMemoryResource();
#endif
}

// Two BVHs are the same tree, array for array: every node (its children, primitive range, row and box slot) and its
// box, every group's lanes, and every point's position, leaf tables and place in the leaf order.
template <class Cloud>
void
requireSameTree(const Cloud& a_cloud, const Cloud& a_reference)
{
  using T = std::decay_t<decltype(a_cloud.position(0)[0])>;

  const auto bvh = a_cloud.getBVH();
  const auto ref = a_reference.getBVH();

  const auto nodes    = bvh.getNodes();
  const auto refNodes = ref.getNodes();

  REQUIRE(nodes.size() == refNodes.size());

  for (uint32_t i = 0; i < nodes.size(); i++) {
    INFO("node " << i);

    const auto& node     = nodes[i];
    const auto& expected = refNodes[i];

    REQUIRE(node.getNumPrimitives() == expected.getNumPrimitives());
    CHECK(node.getPrimitivesOffset() == expected.getPrimitivesOffset());
    CHECK(node.getBoxSlot() == expected.getBoxSlot());

    if (!expected.isLeaf()) {
      CHECK(node.getChildBoxRow() == expected.getChildBoxRow());
      CHECK(node.getChildOffsets() == expected.getChildOffsets());
    }

    const auto box    = bvh.getBoundingVolume(node);
    const auto refBox = ref.getBoundingVolume(expected);

    CHECK(box.getLowCorner() == refBox.getLowCorner());
    CHECK(box.getHighCorner() == refBox.getHighCorner());
  }

  const auto groups    = bvh.getPrimitives();
  const auto refGroups = ref.getPrimitives();

  REQUIRE(groups.size() == refGroups.size());

  const Vec3T<T> probe(T(0.3), T(-0.2), T(0.7));

  for (uint32_t g = 0; g < groups.size(); g++) {
    REQUIRE(groups[g].numValid() == refGroups[g].numValid());

    const auto distances    = groups[g].getDistances2(probe);
    const auto refDistances = refGroups[g].getDistances2(probe);

    for (uint32_t lane = 0; lane < groups[g].numValid(); lane++) {
      CHECK(groups[g].getPointId(lane) == refGroups[g].getPointId(lane));
      CHECK(distances[lane] == refDistances[lane]);
    }
  }

  REQUIRE(a_cloud.numPoints() == a_reference.numPoints());

  const auto order    = a_cloud.allNearestNeighbors(1);
  const auto refOrder = a_reference.allNearestNeighbors(1);

  for (uint32_t i = 0; i < uint32_t(a_reference.numPoints()); i++) {
    CHECK(a_cloud.position(i) == a_reference.position(i));
    CHECK(order[i].index == refOrder[i].index);
    CHECK(order[i].distanceSquared == refOrder[i].distanceSquared);
  }
}

// Every query of a built cloud against brute force: external closest points, self nearest neighbors (which seed
// from the point's own leaf, so they check the per-point tables), k-nearest, and that every point is in exactly
// one leaf lane.
template <class Cloud, class T>
void
checkBuild(const Cloud& a_cloud, const std::vector<Vec3T<T>>& a_pos)
{
  const std::size_t n = a_pos.size();

  REQUIRE(a_cloud.numPoints() == n);

  const double tol = tightMargin<T>();

  std::size_t        lanes = 0;
  std::set<uint32_t> ids;

  for (const auto& group : a_cloud.getBVH().getPrimitives()) {
    for (std::size_t lane = 0; lane < group.numValid(); lane++) {
      ids.insert(group.getPointId(lane));
      lanes++;
    }
  }

  CHECK(lanes == n);
  CHECK(ids.size() == n);

  if (n == 0) {
    CHECK_FALSE(a_cloud.closestPoint(Vec3T<T>(T(0), T(0), T(0))).valid());

    return;
  }

  for (const auto& q : makeCloud<T>(50, 7u)) {
    const auto hit   = a_cloud.closestPoint(q);
    const auto truth = bruteForce<T>(a_pos, q, 1, n);

    REQUIRE(hit.index < n);
    CHECK_THAT((a_pos[hit.index] - q).length2(), withinAbsT<T>(truth[0], tol));
  }

  constexpr std::size_t k = 3;

  typename Cloud::Hit out[k];

  for (uint32_t i = 0; i < n; i += 7) {
    const std::size_t found = a_cloud.nearestNeighbors(i, k, out);
    const auto        truth = bruteForce<T>(a_pos, a_pos[i], k, i);

    REQUIRE(found == Math::min(k, n - 1));

    for (std::size_t j = 0; j < found; j++) {
      CHECK(out[j].index != i);
      CHECK_THAT(out[j].distanceSquared, withinAbsT<T>(truth[j], tol));
    }
  }
}

// A cloud built on the host answers like brute force, and a build in device-accessible memory, through
// GPU::parallelFor(), is the same tree.
template <class Cloud, class T>
void
checkSameTree(const std::vector<Vec3T<T>>& a_pos, const std::size_t a_leafSize)
{
  INFO("points " << a_pos.size() << ", leaf size " << a_leafSize);

  Pool hostPool(hostMemoryResource());
  Pool pool(parallelBuildResource());

  const Cloud reference = Cloud::build(hostPool, a_pos, a_leafSize);
  const Cloud cloud     = Cloud::build(pool, a_pos, a_leafSize);

  checkBuild(reference, a_pos);
  requireSameTree(cloud, reference);
}

// Positions kept apart, x, y and z in arrays of their own, in double precision whatever the cloud's.
template <class T>
struct FromSoA
{
  const double* m_x;
  const double* m_y;
  const double* m_z;

  EBGEOMETRY_HOST_DEVICE
  Vec3T<T>
  operator()(const uint32_t a_i) const noexcept
  {
    return Vec3T<T>(T(m_x[a_i]), T(m_y[a_i]), T(m_z[a_i]));
  }
};

} // namespace

TEMPLATE_TEST_CASE("PointCloudBVH::build in device-accessible memory builds the host's tree, array for array",
                   "[PointCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // In a CUDA or HIP build the build's steps are kernels.
  if (GPU::enabled && !EBGeometryTestGPU::deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const std::vector<Vec3T<T>> uniform = makeCloud<T>(2000, 20261017u);

  SECTION("default branching and width, over leaf sizes from one point to the whole cloud")
  {
    for (const std::size_t leafSize :
         {std::size_t{1}, std::size_t{7}, std::size_t{16 * PointSoA::DefaultWidth<T>()}, std::size_t{5000}}) {
      checkSameTree<PointCloudBVH<T>, T>(uniform, leafSize);
    }
  }

  SECTION("other branching factors and widths")
  {
    checkSameTree<PointCloudBVH<T, 2, 1>, T>(uniform, 4);
    checkSameTree<PointCloudBVH<T, 3, 2>, T>(uniform, 8);
    checkSameTree<PointCloudBVH<T, 8, 8>, T>(uniform, 16);
  }

  SECTION("clouds of zero to a few groups")
  {
    for (const std::size_t n : {0, 1, 2, 3, 5, 9, 17}) {
      checkSameTree<PointCloudBVH<T>, T>(makeCloud<T>(n, 3u + static_cast<unsigned>(n)), 1);
    }
  }

  SECTION("a flat cloud, coincident points, a cluster beside a few outliers, and a cloud wider than the largest "
          "finite value")
  {
    std::vector<Vec3T<T>> flat = uniform;

    for (auto& p : flat) {
      p[2] = T(0.5);
    }

    checkSameTree<PointCloudBVH<T>, T>(flat, 16);
    checkSameTree<PointCloudBVH<T>, T>(std::vector<Vec3T<T>>(300, Vec3T<T>(T(1), T(2), T(3))), 4);

    std::vector<Vec3T<T>> clustered = makeCloud<T>(1000, 11u);

    for (auto& p : clustered) {
      p = Vec3T<T>(T(0.5), T(0.5), T(0.5)) + T(1e-3) * p;
    }

    clustered.emplace_back(T(10), T(0), T(0));
    clustered.emplace_back(T(-10), T(5), T(0));

    checkSameTree<PointCloudBVH<T>, T>(clustered, 16);

    // The box's extent overflows unless the codes are computed in halves.
    std::vector<Vec3T<T>> huge = makeCloud<T>(200, 13u);

    for (auto& p : huge) {
      p = T(0.9) * Math::Limits<T>::max() * (T(2) * p - Vec3T<T>::ones());
    }

    Pool                   pool(hostMemoryResource());
    const PointCloudBVH<T> wide = PointCloudBVH<T>::build(pool, huge, 4);

    for (uint32_t i = 0; i < huge.size(); i += 17) {
      CHECK(wide.closestPoint(huge[i]).index == i);
    }

    // The codes spread the points: the tree is cut by them, not by count alone.
    CHECK(wide.getBVH().getNodes().size() > 1);
  }
}

TEMPLATE_TEST_CASE("PointCloudBVH::build reads positions through a source, in any layout and precision",
                   "[PointCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  if (GPU::enabled && !EBGeometryTestGPU::deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const std::vector<Vec3T<T>> pos = makeCloud<T>(700, 77u);

  std::vector<double> x(pos.size());
  std::vector<double> y(pos.size());
  std::vector<double> z(pos.size());

  for (std::size_t i = 0; i < pos.size(); i++) {
    x[i] = double(pos[i][0]);
    y[i] = double(pos[i][1]);
    z[i] = double(pos[i][2]);
  }

  Pool hostPool(hostMemoryResource());

  const PointCloudBVH<T> reference(hostPool, pos);

  // On the host, the source is read there.
  const PointCloudBVH<T> onHost =
    PointCloudBVH<T>::build(hostPool, FromSoA<T>{x.data(), y.data(), z.data()}, uint32_t(pos.size()));

  requireSameTree(onHost, reference);

  // Through GPU::parallelFor(), the source is read where the launches run, so its arrays must be there too: in a
  // pool of their own, since the BVH's reservations can move its own pool's block.
  Pool sourcePool(parallelBuildResource());
  Pool pool(parallelBuildResource());

  PODVector<double> xs;
  PODVector<double> ys;
  PODVector<double> zs;

  xs.reserveFrom(sourcePool, uint32_t(pos.size()));
  ys.reserveFrom(sourcePool, uint32_t(pos.size()));
  zs.reserveFrom(sourcePool, uint32_t(pos.size()));

  void* sourceBase = sourcePool.base();

  xs.assign(sourceBase, x.data(), uint32_t(pos.size()));
  ys.assign(sourceBase, y.data(), uint32_t(pos.size()));
  zs.assign(sourceBase, z.data(), uint32_t(pos.size()));

  const FromSoA<T> source{xs.data(sourceBase), ys.data(sourceBase), zs.data(sourceBase)};

  const PointCloudBVH<T> parallel = PointCloudBVH<T>::build(pool, source, uint32_t(pos.size()));

  requireSameTree(parallel, reference);

  // FromArray reads an array of Vec3T as it is.
  const PointCloudBVH<T> fromArray =
    PointCloudBVH<T>::build(hostPool, PointCloud::FromArray<T>{pos.data()}, uint32_t(pos.size()));

  requireSameTree(fromArray, reference);
}

TEMPLATE_TEST_CASE("PointCloudBVH::rebuild builds the tree of new positions in place, on a frozen pool",
                   "[PointCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  if (GPU::enabled && !EBGeometryTestGPU::deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr uint32_t maxPoints = 1500;

  const std::vector<Vec3T<T>> first = makeCloud<T>(1000, 3u);

  MemoryResource* const resources[] = {&hostMemoryResource(), &parallelBuildResource()};

  for (MemoryResource* resource : resources) {
    Pool pool(*resource);

    PointCloudBVH<T> cloud = PointCloudBVH<T>::build(pool, first, 16 * PointSoA::DefaultWidth<T>(), maxPoints);

    CHECK(cloud.maxPoints() == maxPoints);

    pool.freeze();

    const std::size_t used = pool.usedBytes();

    // New clouds: as many points as there is room for, moved and scaled, all in one tight cluster (the most
    // nodes), fewer points, one point, and none. Each rebuild is the tree a fresh build of the same points
    // gives, and reserves nothing.
    std::vector<std::vector<Vec3T<T>>> clouds;

    clouds.push_back(makeCloud<T>(maxPoints, 4u));
    clouds.push_back(makeCloud<T>(1200, 5u));

    for (auto& p : clouds.back()) {
      p = T(3) * p + Vec3T<T>(T(5), T(-2), T(1));
    }

    clouds.push_back(makeCloud<T>(maxPoints, 6u));

    for (std::size_t i = 0; i < clouds.back().size(); i++) {
      clouds.back()[i] = (i % 97 == 0) ? T(100) * clouds.back()[i] : T(1e-4) * clouds.back()[i];
    }

    // Points ever closer together along a line, x = 2^-i, so that every cut of the Morton codes peels off a few
    // points: the deepest and least balanced tree the quota allows.
    clouds.emplace_back();

    for (uint32_t i = 0; i < maxPoints; i++) {
      clouds.back().emplace_back(T(std::ldexp(1.0, -int(i % 100))) + T(i) * T(1e-9), T(i % 3), T(0));
    }

    clouds.push_back(makeCloud<T>(17, 7u));
    clouds.push_back(makeCloud<T>(1, 8u));
    clouds.push_back({});

    for (const auto& points : clouds) {
      INFO("points " << points.size());

      Pool positions(*resource);

      PODVector<Vec3T<T>> stored;

      stored.reserveFrom(positions, uint32_t(points.size()));
      stored.assign(positions.base(), points.data(), uint32_t(points.size()));

      cloud.rebuild(PointCloud::FromArray<T>{stored.data(positions.base())}, uint32_t(points.size()));

      Pool                   freshPool(hostMemoryResource());
      const PointCloudBVH<T> fresh(freshPool, points);

      requireSameTree(cloud, fresh);
      CHECK(pool.usedBytes() == used);
    }
  }
}

TEMPLATE_TEST_CASE("PointCloudBVH::build in device memory answers on device like the host's tree",
                   "[PointCloudBVH][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  // Device memory in a GPU build, which the host cannot read, so the positions are copied there first and the
  // kernels write the arrays in place; emulated device memory in a host build, where the steps run as host loops.
  Pool hostPool(hostMemoryResource());
  Pool pool(deviceTestResource());

  const TestCloud<T> reference(hostPool, functorTestCloud<T>(), 8);
  const TestCloud<T> cloud = TestCloud<T>::build(pool, functorTestCloud<T>(), 8);

  pool.freeze();

  const TestCloud<T> deviceView = cloud.rebasedView(pool);

  const std::vector<Vec3T<T>> points =
    queryGrid<T>(Vec3T<T>(T(-0.25), T(-0.25), T(-0.25)), Vec3T<T>(T(1.25), T(1.25), T(1.25)), 10);
  const std::vector<uint32_t> indices = allIndices(reference.numPoints());

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

  compareCloudQueries<TestCloud<T>, T>(deviceView, reference, points, indices, sameIndices, sameValues);

  // Mirrored back to the host, the tree built in device memory is the host's tree.
  const Pool         onHost   = Pool::mirror(pool, hostMemoryResource());
  const TestCloud<T> hostView = cloud.rebasedView(onHost);

  requireSameTree(hostView, reference);
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
    "PointCloudBVH::build: point 1 of 2 has a non-finite coordinate"));

  REQUIRE(abortsWith(
    [&bad] {
      Pool                        pool(hostMemoryResource());
      const PointCloudHashGrid<T> grid(pool, bad);
    },
    "PointCloudHashGrid::build: point 1 of 2 has a non-finite coordinate"));
}

TEST_CASE("PointCloudBVH: rejects a zero leaf size, a rebuild without room or of a point not finite, and a rebase "
          "onto a pool that is not its own",
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
    "PointCloudBVH::build: the target leaf size must be at least 1 (0)"));

  REQUIRE(abortsWith(
    [&pos] {
      Pool             pool(hostMemoryResource());
      PointCloudBVH<T> bvh(pool, pos);

      bvh.rebuild(PointCloud::FromArray<T>{pos.data()}, 2U);
    },
    "PointCloudBVH::rebuild: the BVH has no room for a rebuild (see build()'s a_maxPoints)"));

  REQUIRE(abortsWith(
    [&pos] {
      Pool             pool(hostMemoryResource());
      PointCloudBVH<T> bvh = PointCloudBVH<T>::build(pool, pos, 4, 2);

      bvh.rebuild(PointCloud::FromArray<T>{pos.data()}, 3U);
    },
    "PointCloudBVH::rebuild: 3 points, but the BVH has room for 2 (see build()'s a_maxPoints)"));

  REQUIRE(abortsWith(
    [&pos] {
      Pool                   pool(hostMemoryResource());
      const PointCloudBVH<T> bvh = PointCloudBVH<T>::build(pool, pos, 4, 1);

      (void)bvh;
    },
    "PointCloudBVH::build: room for 1 points in a rebuild, fewer than the 2 built"));

  // A point that is not finite, from a source: on the host, and through GPU::parallelFor() (emulated device
  // memory, in a host build, where the launches can read the host array), which finds it from the box.
  const std::vector<Vec3T<T>> bad = {Vec3T<T>(T(0), T(0), T(0)), Vec3T<T>(Math::Limits<T>::infinity(), T(0), T(0))};

  REQUIRE(abortsWith(
    [&pos, &bad] {
      Pool             pool(hostMemoryResource());
      PointCloudBVH<T> bvh = PointCloudBVH<T>::build(pool, pos, 4, 2);

      bvh.rebuild(PointCloud::FromArray<T>{bad.data()}, 2U);
    },
    "PointCloudBVH::rebuild: point 1 of 2 has a non-finite coordinate"));

  if (!GPU::enabled) {
    REQUIRE(abortsWith(
      [&bad] {
        Pool                   pool(emulatedDeviceMemoryResource());
        const PointCloudBVH<T> bvh = PointCloudBVH<T>::build(pool, PointCloud::FromArray<T>{bad.data()}, 2U);

        (void)bvh;
      },
      "PointCloudBVH::build: a point has a non-finite coordinate"));
  }

  REQUIRE(abortsWith(
    [&pos] {
      Pool                   pool(hostMemoryResource());
      Pool                   unrelated(hostMemoryResource());
      const PointCloudBVH<T> bvh(pool, pos);
      const PointCloudBVH<T> view = bvh.rebasedView(unrelated);

      (void)view;
    },
    "PointCloudBVH::rebasedView: the pool must be the object's own pool or a mirror of it"));
}
