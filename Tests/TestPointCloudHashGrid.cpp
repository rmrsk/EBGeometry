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

  SECTION("a cloud far from the origin matches brute force for every k (the stopping rule's rounding slack)")
  {
    // The stopping rule subtracts a few ulps of |lo| + |q| + extent from each face distance. Far from
    // the origin, |lo| dominates that slack and the cell size can approach the coordinates' own
    // rounding (in float, 1e6 is resolved to 0.0625), so a slack too small would miss neighbors here.
    // Half the points sit exactly on a lattice, where they tie and lie on cell faces.
    for (const double offset : {1.0e3, 1.0e5, 1.0e6}) {
      for (const double spacing : {1.0e-2, 1.0}) {
        std::vector<Vec3T<T>> pos;

        for (int i = 0; i < 1500; i++) {
          const double jitter = (i % 2 == 0) ? 0.0 : 0.3 * double((i * 7919) % 1000) / 1000.0;

          pos.emplace_back(T(offset + spacing * (i % 12 + jitter)),
                           T(-offset + spacing * ((i / 12) % 12 + jitter)),
                           T(offset + spacing * (i / 144 + jitter)));
        }

        for (const T target : {T(0.3), T(1), T(4)}) {
          const PointCloudHashGrid<T> grid(pool, pos, target);

          Hit out[17];
          Hit ref[17];

          for (uint32_t i = 0; i < pos.size(); i += 29) {
            for (const std::size_t k : {std::size_t(1), std::size_t(5), std::size_t(17)}) {
              INFO("offset " << offset << ", spacing " << spacing << ", target " << target << ", point " << i << ", k "
                             << k);

              const auto numPos = static_cast<uint32_t>(pos.size());
              const auto found  = grid.nearestNeighbors(i, k, out);

              REQUIRE(found == PointCloud::closestPointsBruteForce(pos.data(), numPos, pos[i], k, ref, i));

              for (std::size_t j = 0; j < found; j++) {
                CHECK(out[j].distanceSquared == ref[j].distanceSquared);
              }

              // An external query between lattice points.
              const Vec3T<T> q = pos[i] + Vec3T<T>(T(0.5 * spacing), T(0.5 * spacing), T(0.25 * spacing));
              const auto     n = grid.closestPoints(q, k, out);

              REQUIRE(n == PointCloud::closestPointsBruteForce(pos.data(), numPos, q, k, ref));

              for (std::size_t j = 0; j < n; j++) {
                CHECK(out[j].distanceSquared == ref[j].distanceSquared);
              }
            }
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

// Two grids answer every query identically, ties included, and agree on every point's position.
template <class T>
void
requireSameAnswers(const PointCloudHashGrid<T>& a_grid, const PointCloudHashGrid<T>& a_reference)
{
  REQUIRE(a_grid.numPoints() == a_reference.numPoints());

  constexpr std::size_t k = 4;

  typename PointCloudHashGrid<T>::Hit lhs[k];
  typename PointCloudHashGrid<T>::Hit rhs[k];

  for (const auto& q : makeCloud<T>(100, 5u)) {
    REQUIRE(a_grid.closestPoints(q, k, lhs) == a_reference.closestPoints(q, k, rhs));

    for (std::size_t j = 0; j < k; j++) {
      CHECK(lhs[j].index == rhs[j].index);
      CHECK(lhs[j].distanceSquared == rhs[j].distanceSquared);
    }
  }

  for (uint32_t i = 0; i < uint32_t(a_reference.numPoints()); i++) {
    const auto a = a_grid.nearestNeighbor(i);
    const auto b = a_reference.nearestNeighbor(i);

    CHECK(a.index == b.index);
    CHECK(a.distanceSquared == b.distanceSquared);
    CHECK(a_grid.position(i) == a_reference.position(i));
  }
}

// A build in device-accessible memory, through GPU::parallelFor(), is the host build's grid.
template <class T>
void
checkSameGrid(const std::vector<Vec3T<T>>& a_pos, const T a_targetPerCell)
{
  INFO("points " << a_pos.size() << ", target per cell " << a_targetPerCell);

  Pool hostPool(hostMemoryResource());
  Pool pool(parallelBuildResource());

  const PointCloudHashGrid<T> reference(hostPool, a_pos, a_targetPerCell);
  const PointCloudHashGrid<T> grid = PointCloudHashGrid<T>::build(pool, a_pos, a_targetPerCell);

  requireSameAnswers(grid, reference);

  // Every point is where it was given.
  for (uint32_t i = 0; i < uint32_t(a_pos.size()); i++) {
    REQUIRE(grid.position(i) == a_pos[i]);
  }
}

// Positions kept apart, x, y and z in arrays of their own, in double precision whatever the grid's.
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

TEMPLATE_TEST_CASE("PointCloudHashGrid::build in device-accessible memory builds the host's grid",
                   "[PointCloudHashGrid]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // In a CUDA or HIP build the build's steps are kernels.
  if (GPU::enabled && !EBGeometryTestGPU::deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const std::vector<Vec3T<T>> uniform = makeCloud<T>(2000, 20261018u);

  for (const T target : {T(0.25), T(1), T(8)}) {
    checkSameGrid<T>(uniform, target);
  }

  // A tight cluster beside two outliers (the cell cap coarsens the grid), a flat cloud, coincident
  // points, and clouds of zero to three points.
  std::vector<Vec3T<T>> clustered = makeCloud<T>(500, 9u);

  for (auto& p : clustered) {
    p = Vec3T<T>(T(0.5), T(0.5), T(0.5)) + T(1e-3) * p;
  }

  clustered.emplace_back(T(10), T(0), T(0));
  clustered.emplace_back(T(-10), T(5), T(0));

  std::vector<Vec3T<T>> flat = uniform;

  for (auto& p : flat) {
    p[2] = T(0.5);
  }

  checkSameGrid<T>(clustered, T(1));
  checkSameGrid<T>(flat, T(1));
  checkSameGrid<T>(std::vector<Vec3T<T>>(37, Vec3T<T>(T(1), T(2), T(3))), T(1));

  for (const std::size_t n : {0, 1, 2, 3}) {
    checkSameGrid<T>(makeCloud<T>(n, 21u), T(1));
  }
}

TEMPLATE_TEST_CASE("PointCloudHashGrid::build reads positions through a source, in any layout and precision",
                   "[PointCloudHashGrid]",
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

  const PointCloudHashGrid<T> reference(hostPool, pos);

  // On the host, the source is read there.
  const PointCloudHashGrid<T> onHost =
    PointCloudHashGrid<T>::build(hostPool, FromSoA<T>{x.data(), y.data(), z.data()}, uint32_t(pos.size()));

  requireSameAnswers(onHost, reference);

  // Through GPU::parallelFor(), the source is read where the launches run, so its arrays must be there too: in a
  // pool of their own, since the grid's reservations can move its own pool's block.
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

  const PointCloudHashGrid<T> parallel = PointCloudHashGrid<T>::build(pool, source, uint32_t(pos.size()));

  requireSameAnswers(parallel, reference);

  // FromArray reads an array of Vec3T as it is.
  const PointCloudHashGrid<T> fromArray =
    PointCloudHashGrid<T>::build(hostPool, PointCloud::FromArray<T>{pos.data()}, uint32_t(pos.size()));

  requireSameAnswers(fromArray, reference);
}

TEMPLATE_TEST_CASE("PointCloudHashGrid::rebuild builds the grid of new positions in place, on a frozen pool",
                   "[PointCloudHashGrid]",
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

    PointCloudHashGrid<T> grid = PointCloudHashGrid<T>::build(pool, first, T(1), maxPoints);

    CHECK(grid.maxPoints() == maxPoints);

    pool.freeze();

    const std::size_t used = pool.usedBytes();

    // New clouds: more points, moved and scaled (a larger box), fewer points, and none. Each rebuild is the grid
    // a fresh build of the same points gives, and reserves nothing.
    std::vector<std::vector<Vec3T<T>>> clouds;

    clouds.push_back(makeCloud<T>(maxPoints, 4u));
    clouds.push_back(makeCloud<T>(1200, 5u));

    for (auto& p : clouds.back()) {
      p = T(3) * p + Vec3T<T>(T(5), T(-2), T(1));
    }

    clouds.push_back(makeCloud<T>(17, 6u));
    clouds.push_back({});

    for (const auto& cloud : clouds) {
      INFO("points " << cloud.size());

      Pool positions(*resource);

      PODVector<Vec3T<T>> stored;

      stored.reserveFrom(positions, uint32_t(cloud.size()));
      stored.assign(positions.base(), cloud.data(), uint32_t(cloud.size()));

      grid.rebuild(PointCloud::FromArray<T>{stored.data(positions.base())}, uint32_t(cloud.size()), T(1));

      Pool                        freshPool(hostMemoryResource());
      const PointCloudHashGrid<T> fresh(freshPool, cloud);

      requireSameAnswers(grid, fresh);
      CHECK(pool.usedBytes() == used);
    }
  }
}

TEMPLATE_TEST_CASE("PointCloudHashGrid::rebuild makes the cells larger when the box would need more than it has",
                   "[PointCloudHashGrid]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A grid with room for its own cells only, rebuilt with a cloud spread wide but for one dense corner: the
  // cell-size rule wants more cells than the grid has, so the rebuild coarsens them, and stays exact.
  std::vector<Vec3T<T>> cloud = makeCloud<T>(400, 8u);

  Pool pool(hostMemoryResource());

  PointCloudHashGrid<T> grid(pool, cloud, T(8));

  for (std::size_t i = 0; i < cloud.size(); i++) {
    cloud[i] = (i < 10) ? T(50) * cloud[i] : T(1e-2) * cloud[i];
  }

  grid.rebuild(PointCloud::FromArray<T>{cloud.data()}, uint32_t(cloud.size()), T(0.25));

  for (const auto& q : makeCloud<T>(50, 9u)) {
    const auto hit = grid.closestPoint(q);
    const auto ref = PointCloud::closestPointBruteForce(cloud.data(), uint32_t(cloud.size()), q);

    CHECK(hit.index == ref.index);
    CHECK(hit.distanceSquared == ref.distanceSquared);
  }
}

TEMPLATE_TEST_CASE("PointCloudHashGrid::build in device memory answers on device like the host's grid",
                   "[PointCloudHashGrid][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Grid = PointCloudHashGrid<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  // Device memory in a GPU build, which the host cannot read, so the positions are copied there first and the
  // kernels write the arrays in place; emulated device memory in a host build, where the steps run as host loops.
  Pool hostPool(hostMemoryResource());
  Pool pool(deviceTestResource());

  const Grid reference(hostPool, functorTestCloud<T>());
  const Grid grid = Grid::build(pool, functorTestCloud<T>());

  pool.freeze();

  const Grid deviceView = grid.rebasedView(pool);

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

  compareCloudQueries<Grid, T>(deviceView, reference, points, indices, sameIndices, sameValues);

  // Mirrored back to the host, the grid built in device memory answers there as the host's grid does.
  const Pool onHost   = Pool::mirror(pool, hostMemoryResource());
  const Grid hostView = grid.rebasedView(onHost);

  requireSameAnswers(hostView, reference);
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
    "PointCloudHashGrid::build: the target points per cell must be positive (0)"));

  REQUIRE(abortsWith(
    [&pos] {
      Pool                  pool(hostMemoryResource());
      PointCloudHashGrid<T> grid(pool, pos);

      grid.rebuild(PointCloud::FromArray<T>{pos.data()}, 3U);
    },
    "PointCloudHashGrid::rebuild: 3 points, but the grid has room for 2 (see build()'s a_maxPoints)"));

  // A point that is not finite, from a source: on the host, and through GPU::parallelFor() (emulated device
  // memory, in a host build, where the launches can read the host array), which finds it from the box.
  const std::vector<Vec3T<T>> bad = {Vec3T<T>(T(0), T(0), T(0)), Vec3T<T>(Math::Limits<T>::infinity(), T(0), T(0))};

  REQUIRE(abortsWith(
    [&pos, &bad] {
      Pool                  pool(hostMemoryResource());
      PointCloudHashGrid<T> grid(pool, pos);

      grid.rebuild(PointCloud::FromArray<T>{bad.data()}, 2U);
    },
    "PointCloudHashGrid::rebuild: point 1 of 2 has a non-finite coordinate"));

  if (!GPU::enabled) {
    REQUIRE(abortsWith(
      [&bad] {
        Pool pool(emulatedDeviceMemoryResource());

        (void)PointCloudHashGrid<T>::build(pool, PointCloud::FromArray<T>{bad.data()}, 2U);
      },
      "PointCloudHashGrid::build: a point has a non-finite coordinate"));
  }

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
