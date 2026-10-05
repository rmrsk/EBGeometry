// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for PointCloudHashGrid: the uniform-grid build and the high-level closest-point /
// nearest-neighbor query API. Every query is checked against an independent brute-force scan on a
// small random cloud, so both the grid and the expanding-shell query path (including its exact
// stopping rule, external queries, and queries outside the grid box) get real coverage in both
// precisions. The query functors shared with the PointCloudBVH tests are in TestPointCloudQueries.hpp.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestPointCloudQueries.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestPointCloud;

TEMPLATE_TEST_CASE("PointCloudHashGrid queries match brute force", "[PointCloudHashGrid]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr std::size_t n = 2000;

  const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 20260708u);

  Pool pool(hostMemoryResource());

  const PointCloudHashGrid<T> grid(pool, pos);

  REQUIRE(grid.numPoints() == n);

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
    for (uint32_t i = 0; i < n; i += 137) {
      const auto hit = grid.closestPoint(pos[i]);
      CHECK(hit.index == i);
      CHECK_THAT(hit.distanceSquared, withinAbsT<T>(T(0), tol));
    }
  }

  SECTION("nearestNeighbor (self query, excludes self) matches brute force")
  {
    for (uint32_t i = 0; i < n; i += 41) {
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

    typename PointCloudHashGrid<T>::Hit out[k];

    for (uint32_t i = 0; i < n; i += 53) {
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
    for (uint32_t i = 0; i < n; i += 29) {
      const auto single = grid.nearestNeighbor(i);
      CHECK_THAT(all[i].distanceSquared, withinAbsT<T>(single.distanceSquared, tol));
    }
  }

  SECTION("accessors return the stored cloud data")
  {
    for (uint32_t i = 0; i < n; i += 313) {
      CHECK_THAT((grid.position(i) - pos[i]).length2(), withinAbsT<T>(T(0), tol));
    }
  }
}

TEMPLATE_TEST_CASE("PointCloudHashGrid edge cases", "[PointCloudHashGrid]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T   = TestType;
  using Hit = typename PointCloudHashGrid<T>::Hit;

  Pool pool(hostMemoryResource());

  const T      notFound = std::numeric_limits<T>::max();
  const double tol      = tightMargin<T>();

  SECTION("empty cloud: no out-of-bounds access, queries report nothing")
  {
    const std::vector<Vec3T<T>> pos;
    const PointCloudHashGrid<T> grid(pool, pos);

    CHECK(grid.numPoints() == 0);
    CHECK(grid.closestPoint(Vec3T<T>(T(0), T(0), T(0))).distanceSquared == notFound);

    Hit               out[4];
    const std::size_t found = grid.closestPoints(Vec3T<T>(T(0), T(0), T(0)), 4, out);
    CHECK(found == 0);
    CHECK(grid.allNearestNeighbors(1).empty());
  }

  SECTION("single point: external query finds it, self query finds nothing")
  {
    const std::vector<Vec3T<T>> pos = {Vec3T<T>(T(0.25), T(0.5), T(0.75))};
    const PointCloudHashGrid<T> grid(pool, pos);

    const auto hit = grid.closestPoint(pos[0]);
    CHECK(hit.index == 0);
    CHECK_THAT(hit.distanceSquared, withinAbsT<T>(T(0), tol));

    // nearestNeighbor excludes self, so with only one point there is no neighbor.
    CHECK(grid.nearestNeighbor(0).distanceSquared == notFound);
    CHECK_FALSE(grid.nearestNeighbor(0).valid());
    CHECK(hit.valid());

    // Rows of allNearestNeighbors that cannot be filled hold "no match" results.
    const auto all = grid.allNearestNeighbors(3);
    REQUIRE(all.size() == 3);
    CHECK_FALSE(all[0].valid());
    CHECK_FALSE(all[2].valid());
  }

  SECTION("queries far beyond the grid, past the int range of cell coordinates, find the nearest point")
  {
    // A cell coordinate (x - lo)/h beyond 2^31 used to overflow its int conversion, clamp to the
    // wrong side of the grid, and end the search after one shell with a far point.
    const std::vector<Vec3T<T>> pos = makeCloud<T>(1000, 77u);
    const PointCloudHashGrid<T> grid(pool, pos);

    for (const T x : {T(1e8), T(-1e9), T(3e9)}) {
      const Vec3T<T> q(x, T(0.5), T(0.5));
      const auto     hit = grid.closestPoint(q);

      INFO("x = " << x);
      REQUIRE(hit.valid());
      REQUIRE(std::isfinite(hit.distanceSquared));

      // At this distance only double can tell the candidates apart: the nearest point is the one
      // with the largest (or smallest) x coordinate.
      if constexpr (std::is_same_v<T, double>) {
        uint32_t best = 0;

        for (uint32_t i = 1; i < pos.size(); i++) {
          best = (x > T(0)) == (pos[i][0] > pos[best][0]) ? i : best;
        }

        CHECK(hit.index == best);
      }
    }
  }

  SECTION("coincident points all land in one cell and still resolve")
  {
    constexpr std::size_t       n = 16;
    const std::vector<Vec3T<T>> pos(n, Vec3T<T>(T(1), T(2), T(3)));
    const PointCloudHashGrid<T> grid(pool, pos);

    // Nearest OTHER point is another coincident point at distance 0.
    const auto hit = grid.nearestNeighbor(0);
    CHECK(hit.index != 0);
    CHECK_THAT(hit.distanceSquared, withinAbsT<T>(T(0), tol));
  }

  SECTION("planar (zero-extent) cloud matches brute force")
  {
    // All z == 0: a degenerate, zero-volume bounding box exercises the cell-size fallback.
    constexpr std::size_t n   = 500;
    std::vector<Vec3T<T>> pos = makeCloud<T>(n, 7u);

    for (auto& p : pos) {
      p = Vec3T<T>(p[0], p[1], T(0));
    }

    const PointCloudHashGrid<T> grid(pool, pos);

    for (uint32_t i = 0; i < n; i += 17) {
      const auto truth = bruteForce<T>(pos, pos[i], 1, i);
      CHECK_THAT(grid.nearestNeighbor(i).distanceSquared, withinAbsT<T>(truth[0], tol));
    }
  }

  SECTION("k larger than the cloud returns every point")
  {
    const std::vector<Vec3T<T>> pos = makeCloud<T>(3, 8u);
    const PointCloudHashGrid<T> grid(pool, pos);

    Hit               out[10];
    const std::size_t found = grid.closestPoints(Vec3T<T>(T(0.5), T(0.5), T(0.5)), 10, out);
    CHECK(found == 3);
  }

  SECTION("extreme cell-size targets stay correct (grid-size clamp and coarse grid)")
  {
    constexpr std::size_t       n   = 800;
    const std::vector<Vec3T<T>> pos = makeCloud<T>(n, 9u);

    // A tiny target would demand an enormous grid (clamped) and a huge target collapses toward one
    // cell; both must still return exact results.
    const T targets[] = {T(1e-6), T(1000)};

    for (const T target : targets) {
      const PointCloudHashGrid<T> grid(pool, pos, target);

      for (uint32_t i = 0; i < n; i += 91) {
        const auto truth = bruteForce<T>(pos, pos[i], 1, i);
        CHECK_THAT(grid.nearestNeighbor(i).distanceSquared, withinAbsT<T>(truth[0], tol));
      }
    }
  }

  SECTION("a lattice, where many points tie, matches brute force for every k (the exact stopping rule)")
  {
    // Points exactly on a lattice put many candidates at the same distance and right on cell faces,
    // where the stopping rule's face reconstruction is tightest.
    std::vector<Vec3T<T>> pos;

    for (int i = 0; i < 1000; i++) {
      pos.emplace_back(T(i % 10), T((i / 10) % 10), T(i / 100));
    }

    for (const T target : {T(1), T(0.3), T(8)}) {
      const PointCloudHashGrid<T> grid(pool, pos, target);

      Hit out[20];

      for (uint32_t i = 0; i < pos.size(); i += 13) {
        for (const std::size_t k : {std::size_t(1), std::size_t(6), std::size_t(20)}) {
          const auto        truth = bruteForce<T>(pos, pos[i], k, i);
          const std::size_t found = grid.nearestNeighbors(i, k, out);

          REQUIRE(found == truth.size());

          for (std::size_t j = 0; j < found; j++) {
            CHECK(out[j].distanceSquared == truth[j]);
          }
        }
      }
    }
  }

  SECTION("clustered cloud with a distant outlier stays bounded and correct")
  {
    // A tight cluster plus one far outlier makes the bounding box highly anisotropic (a huge x-extent
    // with a tiny local spacing). The grid-size cap must keep the cell count bounded -- no runaway
    // allocation or int overflow -- while queries stay exact. The outlier's own index is visited too.
    constexpr std::size_t n   = 1000;
    std::vector<Vec3T<T>> pos = makeCloud<T>(n - 1, 12u); // cluster in the unit cube
    pos.push_back(Vec3T<T>(T(1e6), T(0.5), T(0.5)));      // distant outlier stretches the x-extent

    const PointCloudHashGrid<T> grid(pool, pos);

    REQUIRE(grid.numPoints() == n);

    for (uint32_t i = 0; i < n; i += 37) {
      const auto truth = bruteForce<T>(pos, pos[i], 1, i);
      CHECK_THAT(grid.nearestNeighbor(i).distanceSquared, withinAbsT<T>(truth[0], tol));
    }
  }
}

TEMPLATE_TEST_CASE("PointCloudHashGrid: rebasedView and deepCopy answer identically",
                   "[PointCloudHashGrid][Pool][rebase]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Grid = PointCloudHashGrid<T>;
  using Hit  = typename Grid::Hit;

  static_assert(std::is_trivially_copyable_v<Grid>);

  constexpr std::size_t n = 700;
  constexpr std::size_t k = 4;

  const std::vector<Vec3T<T>> pos     = makeCloud<T>(n, 777u);
  const std::vector<Vec3T<T>> queries = makeCloud<T>(40, 31u);

  // Every query result of a_other must match a_grid's exactly: same cells, same data.
  const auto requireSameAnswers = [&](const Grid& a_grid, const Grid& a_other) {
    REQUIRE(a_other.numPoints() == a_grid.numPoints());

    for (uint32_t i = 0; i < n; i += 11) {
      REQUIRE(a_other.position(i) == a_grid.position(i));

      const Hit lhs = a_grid.nearestNeighbor(i);
      const Hit rhs = a_other.nearestNeighbor(i);

      REQUIRE(rhs.index == lhs.index);
      REQUIRE(rhs.distanceSquared == lhs.distanceSquared);
    }

    for (const auto& q : queries) {
      Hit lhs[k];
      Hit rhs[k];

      REQUIRE(a_other.closestPoints(q, k, rhs) == a_grid.closestPoints(q, k, lhs));

      for (std::size_t j = 0; j < k; j++) {
        REQUIRE(rhs[j].index == lhs[j].index);
        REQUIRE(rhs[j].distanceSquared == lhs[j].distanceSquared);
      }
    }

    const auto lhsAll = a_grid.allNearestNeighbors(2);
    const auto rhsAll = a_other.allNearestNeighbors(2);

    REQUIRE(rhsAll.size() == lhsAll.size());

    for (std::size_t i = 0; i < lhsAll.size(); i++) {
      REQUIRE(rhsAll[i].index == lhsAll[i].index);
    }
  };

  Pool pool(hostMemoryResource());

  const Grid grid(pool, pos);

  REQUIRE(grid.isAttachedTo(pool));

  for (uint32_t i = 0; i < n; i += 37) {
    const auto truth = bruteForce<T>(pos, pos[i], 1, i);

    REQUIRE_THAT(grid.nearestNeighbor(i).distanceSquared, withinAbsT<T>(truth[0], tightMargin<T>()));
  }

  SECTION("deepCopy into a separate pool is independent storage")
  {
    Pool other(hostMemoryResource());

    const Grid copy = grid.deepCopy(other);

    REQUIRE(copy.isAttachedTo(other));
    REQUIRE_FALSE(copy.isAttachedTo(pool));
    requireSameAnswers(grid, copy);
  }

  SECTION("deepCopy into its own pool survives the pool growing under it")
  {
    const Grid copy = grid.deepCopy(pool);

    REQUIRE(copy.isAttachedTo(pool));
    REQUIRE(copy.base() == grid.base());
    requireSameAnswers(grid, copy);
  }

  SECTION("a host-to-host rebasedView resolves against the mirror, and the query functors agree")
  {
    pool.freeze();

    Pool       mirror = Pool::mirror(pool, hostMemoryResource());
    const Grid view   = grid.rebasedView(mirror);

    REQUIRE(view.isAttachedTo(mirror));
    REQUIRE(view.base() == mirror.base());
    REQUIRE(view.base() != grid.base());
    requireSameAnswers(grid, view);

    // The device test's functors, run on the host against the rebased descriptor passed by value.
    const auto same = [](const auto& a_first, const auto& a_second, const auto& a_queries) {
      for (std::size_t i = 0; i < a_queries.size(); i++) {
        INFO("query " << i);
        REQUIRE(a_first(a_queries[i]) == a_second(a_queries[i]));
      }
    };

    compareCloudQueries<Grid, T>(view, grid, functorTestPoints<T>(), allIndices(n), same, same);
  }
}

TEMPLATE_TEST_CASE("PointCloudHashGrid: a rebased view answers queries on device and matches the host",
                   "[PointCloudHashGrid][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Grid = PointCloudHashGrid<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const Grid grid(pool, functorTestCloud<T>());

  pool.freeze();

  Pool       devicePool = Pool::mirror(pool, deviceTestResource());
  const Grid deviceView = grid.rebasedView(devicePool);

  // 1000 points over [-0.25, 1.25]^3, around and inside the unit-cube cloud, and every cloud index.
  const std::vector<Vec3T<T>> points =
    queryGrid<T>(Vec3T<T>(T(-0.25), T(-0.25), T(-0.25)), Vec3T<T>(T(1.25), T(1.25), T(1.25)), 10);
  const std::vector<uint32_t> indices = allIndices(grid.numPoints());

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

  compareCloudQueries<Grid, T>(deviceView, grid, points, indices, sameIndices, sameValues);
}

TEST_CASE("PointCloudHashGrid: rejects a non-positive target occupancy, and a rebase onto a pool too small "
          "to hold it",
          "[PointCloudHashGrid][death]")
{
  using T = double;

  // EBGEOMETRY_REQUIREs, so they abort in every build.
  const std::vector<Vec3T<T>> pos = {Vec3T<T>(T(0), T(0), T(0)), Vec3T<T>(T(1), T(0), T(0))};

  REQUIRE(abortsWith(
    [&pos] {
      Pool                        pool(hostMemoryResource());
      const PointCloudHashGrid<T> grid(pool, pos, T(0));
    },
    "PointCloudHashGrid: the target points per cell must be positive (0)"));

  REQUIRE(abortsWith(
    [&pos] {
      Pool                        pool(hostMemoryResource());
      Pool                        unrelated(hostMemoryResource());
      const PointCloudHashGrid<T> grid(pool, pos);
      const PointCloudHashGrid<T> view = grid.rebasedView(unrelated);

      (void)view;
    },
    "PointCloudHashGrid::rebasedView: the pool must be the object's own pool or a mirror of it"));
}
