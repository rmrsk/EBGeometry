// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the point clouds: PointCloudBVH and PointCloudHashGrid, over 500,000 points
// uniform in the unit cube or on the unit sphere, in float and double. A host build runs the host cases
// (the build, the batched k-nearest-neighbour graph, per-point queries); a CUDA or HIP build runs the
// per-point queries as host/device pairs. See ../README.md and PerformanceTesting.rst.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The branching factor and point-group width, spelled out rather than taken from the Default*
// functions, so that a change to those defaults shows as a change of these cases' parameters.
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// The number of neighbours the k-nearest cases ask for.
constexpr std::size_t KNearest = 8;

// The cloud's size and the numbers of queries, at full and at smoke size.
constexpr std::size_t CloudPoints      = 500000;
constexpr std::size_t SmokeCloudPoints = 2000;
constexpr std::size_t HostQueries      = 200000;
constexpr std::size_t PairQueries      = 262144;
constexpr std::size_t SmokeQueries     = 1000;
constexpr std::size_t SmokeFarQueries  = 10;

template <class T>
using Hit = EBGeometry::PointCloud::Hit<T>;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// The closest cloud point to an arbitrary point.
template <class Cloud, class T>
struct ClosestPoint
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  Hit<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    return m_cloud.closestPoint(a_point);
  }
};

// The nearest other point to a point already in the cloud.
template <class Cloud, class T>
struct NearestNeighbor
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  Hit<T>
  operator()(std::uint32_t a_point) const noexcept
  {
    return m_cloud.nearestNeighbor(a_point);
  }
};

// The KNearest nearest points to one query, nearest first: a fixed-size, trivially copyable result, so
// that GPU::transform can return it from a kernel.
template <class T>
struct KHits
{
  EBGeometry::Array<Hit<T>, KNearest> m_hits;
  std::uint32_t                       m_count = 0;
};

// The KNearest closest cloud points to an arbitrary point.
template <class Cloud, class T>
struct ClosestPoints
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  KHits<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    KHits<T> result;

    result.m_count = static_cast<std::uint32_t>(m_cloud.closestPoints(a_point, KNearest, result.m_hits.data()));

    return result;
  }
};

// The KNearest nearest other points to a point already in the cloud.
template <class Cloud, class T>
struct NearestNeighbors
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  KHits<T>
  operator()(std::uint32_t a_point) const noexcept
  {
    KHits<T> result;

    result.m_count = static_cast<std::uint32_t>(m_cloud.nearestNeighbors(a_point, KNearest, result.m_hits.data()));

    return result;
  }
};

// Record what a set of queries found: the sum of the squared distances, and the exact checksum of the
// points.
template <class T>
void
record(Perf::Case& a_case, const std::vector<Hit<T>>& a_hits)
{
  a_case.checksum(Perf::checksumOf(a_hits));
  a_case.indexChecksum(Perf::indexChecksumOf(a_hits));
}

// The same for sets of nearest points, rank by rank.
template <class T>
void
record(Perf::Case& a_case, const std::vector<KHits<T>>& a_results)
{
  double              sum = 0.0;
  Perf::IndexChecksum indices;

  for (const auto& r : a_results) {
    for (std::uint32_t j = 0; j < r.m_count; j++) {
      sum += static_cast<double>(r.m_hits[j].distanceSquared);
      indices.add(r.m_hits[j].index);
    }
  }

  a_case.checksum(sum);
  a_case.indexChecksum(indices.value());
}

// Whether a host and a device set of nearest points agree: the same count and, rank by rank, the same
// distances (equal distances to different points are ties, not disagreements).
template <class T>
std::pair<double, bool>
agreeKHits(const KHits<T>& a_host, const KHits<T>& a_device)
{
  double diff     = 0.0;
  bool   mismatch = a_host.m_count != a_device.m_count;

  for (std::uint32_t j = 0; j < a_host.m_count && j < a_device.m_count; j++) {
    const double dh = static_cast<double>(a_host.m_hits[j].distanceSquared);
    const double d  = std::abs(dh - static_cast<double>(a_device.m_hits[j].distanceSquared));

    diff     = std::max(diff, d);
    mismatch = mismatch || d > Perf::agreementTolerance<T>(dh);
  }

  return {diff, mismatch};
}

// The two point sets.
enum class Shape
{
  Uniform, // uniform in the unit cube
  Sphere   // on the unit sphere's surface
};

const char*
shapeName(Shape a_shape)
{
  return a_shape == Shape::Uniform ? "uniform" : "sphere";
}

// The cloud's points.
template <class T>
std::vector<Vec3<T>>
cloudPoints(Shape a_shape, std::size_t a_n)
{
  auto points = Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), a_n, Perf::Seed::Cloud);

  if (a_shape == Shape::Sphere) {
    for (auto& p : points) {
      const Vec3<T> c = T(2) * p - Vec3<T>(T(1), T(1), T(1));

      p = c / c.length();
    }
  }

  return points;
}

// Queries near the cloud: for the cube, its bounding box grown by 5% on each side; for the sphere, a
// shell from radius 0.95 to 1.05 (its hollow inside is no nearer the cloud than the outside is).
template <class T>
std::vector<Vec3<T>>
nearQueries(Shape a_shape, std::size_t a_n)
{
  if (a_shape == Shape::Uniform) {
    return Perf::pointsInBox<T>(Perf::cube(T(-0.05), T(1.05)), a_n, Perf::Seed::NearQueries);
  }

  auto       q     = Perf::pointsInBox<T>(Perf::cube(T(-1), T(1)), a_n, Perf::Seed::NearQueries);
  const auto radii = Perf::pointsInBox<T>(
    EBGeometry::BoundingVolumes::AABBT<T>(Vec3<T>(T(0.95), T(0), T(0)), Vec3<T>(T(1.05), T(0), T(0))),
    a_n,
    Perf::Seed::ShellRadii);

  for (std::size_t i = 0; i < a_n; i++) {
    q[i] = radii[i][0] * q[i] / q[i].length();
  }

  return q;
}

// Every case for one cloud type, point set and precision. a_build(pool, points) builds the cloud.
// a_farQueries is the number of queries far outside the cloud, chosen per cloud type: a hash grid
// answers them slowly (it searches outward cell by cell), so it gets few, and every repetition takes
// milliseconds for either type without taking minutes.
template <class T, class Cloud, class Build>
void
addCases(Perf::Suite& a_suite, const std::string& a_cloud, Shape a_shape, std::size_t a_farQueries, Build a_build)
{
  const std::string prefix = Perf::caseName({a_cloud, shapeName(a_shape)});
  const std::string prec   = Perf::precisionName<T>();
  const std::string k      = std::to_string(KNearest);

  // The cloud's points, recorded as the case's parameters with the cloud's.
  const auto points = [=](Perf::Case& c) {
    const std::size_t n = c.size(CloudPoints, SmokeCloudPoints);

    c.param("points", n);
    c.param("K", K);
    c.param("W", W);

    return cloudPoints<T>(a_shape, n);
  };

  const auto near = [=](Perf::Case& c, std::size_t a_full) {
    return nearQueries<T>(a_shape, c.size(a_full, SmokeQueries));
  };

  // ── Host cases ──

  a_suite.add(Perf::caseName({prefix, "build", prec, "host"}), [=](Perf::Case& c) {
    const auto p     = points(c);
    const auto built = Perf::measureBuild<Cloud>(c, [&](EBGeometry::Pool& a_pool) { return a_build(a_pool, p); });

    c.poolBytes(built.pool->usedBytes());
    c.checksum(static_cast<double>(built.object->numPoints()));
  });

  for (const std::size_t nn : {std::size_t(1), KNearest}) {
    a_suite.add(Perf::caseName({prefix, "all" + std::to_string(nn) + "nn", prec, "host"}), [=](Perf::Case& c) {
      const auto          p = points(c);
      EBGeometry::Pool    pool(EBGeometry::hostMemoryResource());
      const Cloud         cloud = a_build(pool, p);
      std::vector<Hit<T>> graph;

      c.measure([&] { graph = cloud.allNearestNeighbors(nn); }, p.size());
      record(c, graph);
    });
  }

  a_suite.add(Perf::caseName({prefix, "closest", prec, "host"}), [=](Perf::Case& c) {
    const auto          p = points(c);
    const auto          q = near(c, HostQueries);
    EBGeometry::Pool    pool(EBGeometry::hostMemoryResource());
    std::vector<Hit<T>> hits;

    Perf::measureHostQuery(c, ClosestPoint<Cloud, T>{a_build(pool, p)}, q, hits);
    record(c, hits);
  });

  // Queries far outside the cloud: in a box five times the cloud's size.
  a_suite.add(Perf::caseName({prefix, "closest-outside", prec, "host"}), [=](Perf::Case& c) {
    const auto p = points(c);
    const auto q =
      Perf::pointsInBox<T>(Perf::cube(T(-2), T(3)), c.size(a_farQueries, SmokeFarQueries), Perf::Seed::FarQueries);
    EBGeometry::Pool    pool(EBGeometry::hostMemoryResource());
    std::vector<Hit<T>> hits;

    Perf::measureHostQuery(c, ClosestPoint<Cloud, T>{a_build(pool, p)}, q, hits);
    record(c, hits);
  });

  // Every point's nearest neighbour and its KNearest nearest, the points visited in Morton order, and the
  // KNearest closest points to queries near the cloud, also in Morton order: per point, as a device
  // thread asks (all8nn above is the batched host path).
  a_suite.add(Perf::caseName({prefix, "neighbor", prec, "host"}), [=](Perf::Case& c) {
    const auto          p = points(c);
    EBGeometry::Pool    pool(EBGeometry::hostMemoryResource());
    std::vector<Hit<T>> hits;

    Perf::measureHostQuery(c, NearestNeighbor<Cloud, T>{a_build(pool, p)}, Perf::mortonOrder(p), hits);
    record(c, hits);
  });

  a_suite.add(Perf::caseName({prefix, "neighbors" + k, prec, "host"}), [=](Perf::Case& c) {
    c.param("k", KNearest);

    const auto            p = points(c);
    EBGeometry::Pool      pool(EBGeometry::hostMemoryResource());
    std::vector<KHits<T>> results;

    Perf::measureHostQuery(c, NearestNeighbors<Cloud, T>{a_build(pool, p)}, Perf::mortonOrder(p), results);
    record(c, results);
  });

  a_suite.add(Perf::caseName({prefix, "closest" + k, prec, "host"}), [=](Perf::Case& c) {
    c.param("k", KNearest);

    const auto            p = points(c);
    const auto            q = Perf::mortonOrdered(near(c, PairQueries));
    EBGeometry::Pool      pool(EBGeometry::hostMemoryResource());
    std::vector<KHits<T>> results;

    Perf::measureHostQuery(c, ClosestPoints<Cloud, T>{a_build(pool, p)}, q, results);
    record(c, results);
  });

  // ── Host/device pairs: the same queries on both sides ──

  for (const Perf::Order order : Perf::Orders) {
    a_suite.add(Perf::caseName({prefix, std::string("closest-") + Perf::orderName(order), prec, "hostdevice"}),
                [=](Perf::Case& c) {
                  const auto       p = points(c);
                  const auto       q = Perf::inOrder(order, near(c, PairQueries));
                  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
                  const Cloud      cloud = a_build(pool, p);

                  const auto hits = Perf::measureHostAndDevice<Hit<T>>(
                    c,
                    pool,
                    q,
                    [&](const EBGeometry::Pool& a_pool) { return ClosestPoint<Cloud, T>{cloud.rebasedView(a_pool)}; },
                    Perf::agreeHit<T>);

                  record(c, hits);
                });
  }

  a_suite.add(Perf::caseName({prefix, "neighbor-morton", prec, "hostdevice"}), [=](Perf::Case& c) {
    const auto       p = points(c);
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const Cloud      cloud = a_build(pool, p);

    const auto hits = Perf::measureHostAndDevice<Hit<T>>(
      c,
      pool,
      Perf::mortonOrder(p),
      [&](const EBGeometry::Pool& a_pool) { return NearestNeighbor<Cloud, T>{cloud.rebasedView(a_pool)}; },
      Perf::agreeHit<T>);

    record(c, hits);
  });

  a_suite.add(Perf::caseName({prefix, "neighbors" + k + "-morton", prec, "hostdevice"}), [=](Perf::Case& c) {
    c.param("k", KNearest);

    const auto       p = points(c);
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const Cloud      cloud = a_build(pool, p);

    const auto results = Perf::measureHostAndDevice<KHits<T>>(
      c,
      pool,
      Perf::mortonOrder(p),
      [&](const EBGeometry::Pool& a_pool) { return NearestNeighbors<Cloud, T>{cloud.rebasedView(a_pool)}; },
      agreeKHits<T>);

    record(c, results);
  });

  a_suite.add(Perf::caseName({prefix, "closest" + k + "-morton", prec, "hostdevice"}), [=](Perf::Case& c) {
    c.param("k", KNearest);

    const auto       p = points(c);
    const auto       q = Perf::mortonOrdered(near(c, PairQueries));
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const Cloud      cloud = a_build(pool, p);

    const auto results = Perf::measureHostAndDevice<KHits<T>>(
      c,
      pool,
      q,
      [&](const EBGeometry::Pool& a_pool) { return ClosestPoints<Cloud, T>{cloud.rebasedView(a_pool)}; },
      agreeKHits<T>);

    record(c, results);
  });
}

template <class T>
using BVHCloud = EBGeometry::PointCloudBVH<T, K, W>;

template <class T>
using GridCloud = EBGeometry::PointCloudHashGrid<T>;

// Every case for one cloud type and one point set, both precisions together.
template <template <class> class Cloud>
void
addCloud(Perf::Suite& a_suite, const std::string& a_cloud, Shape a_shape, std::size_t a_farQueries)
{
  const auto build = [](EBGeometry::Pool& a_pool, const auto& a_points) {
    using T = std::decay_t<decltype(a_points[0][0])>;

    return Cloud<T>(a_pool, a_points);
  };

  addCases<float, Cloud<float>>(a_suite, a_cloud, a_shape, a_farQueries, build);
  addCases<double, Cloud<double>>(a_suite, a_cloud, a_shape, a_farQueries, build);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "PointCloud");

  for (const Shape shape : {Shape::Uniform, Shape::Sphere}) {
    addCloud<BVHCloud>(suite, "PointCloudBVH", shape, 20000);
    addCloud<GridCloud>(suite, "PointCloudHashGrid", shape, 100);
  }

  return suite.run();
}
