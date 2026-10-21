// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the point clouds: PointCloudBVH and PointCloudHashGrid, on points uniform in
// the unit cube and on the unit-sphere surface, in float and double. A host build runs the host cases
// (builds, the batched k-nearest-neighbour graph, per-point queries); a CUDA or HIP build runs the
// per-point queries as host/device pairs. See ../README.md.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>
#include <Source/EBGeometry_Random.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The branching factor and point-group width, spelled out rather than taken from the Default*
// functions, so a change to those defaults shows up as a change to these cases' names, not their times.
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// Closest cloud point to an arbitrary point: one query per input, on either side.
template <class Cloud, class T>
struct ClosestPoint
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  EBGeometry::PointCloud::Hit<T>
  operator()(const EBGeometry::Vec3T<T>& a_point) const noexcept
  {
    return m_cloud.closestPoint(a_point);
  }
};

// Nearest other point to a point already in the cloud.
template <class Cloud, class T>
struct NearestNeighbor
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  EBGeometry::PointCloud::Hit<T>
  operator()(const uint32_t& a_point) const noexcept
  {
    return m_cloud.nearestNeighbor(a_point);
  }
};

// The number of neighbours the k-nearest cases ask for.
constexpr std::size_t KNearest = 8;

// The KNearest nearest points to one query, nearest first: a fixed-size, trivially copyable result, so
// that GPU::transform can return it from a kernel.
template <class T>
struct KHits
{
  EBGeometry::PointCloud::Hit<T> m_hits[KNearest];
  uint32_t                       m_count = 0;
};

// The KNearest closest cloud points to an arbitrary point.
template <class Cloud, class T>
struct ClosestPoints
{
  Cloud m_cloud;

  EBGEOMETRY_HOST_DEVICE
  KHits<T>
  operator()(const EBGeometry::Vec3T<T>& a_point) const noexcept
  {
    KHits<T> result;

    result.m_count = static_cast<uint32_t>(m_cloud.closestPoints(a_point, KNearest, result.m_hits));

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
  operator()(const uint32_t& a_point) const noexcept
  {
    KHits<T> result;

    result.m_count = static_cast<uint32_t>(m_cloud.nearestNeighbors(a_point, KNearest, result.m_hits));

    return result;
  }
};

// The cloud's points: uniform in the unit cube, or projected onto the unit sphere.
template <class T>
std::vector<EBGeometry::Vec3T<T>>
cloudPoints(const std::string& a_shape, std::size_t a_n)
{
  auto points = EBGeometry::Random::samplePoints<T>(a_n, 20261019u);

  if (a_shape == "sphere") {
    for (auto& p : points) {
      const EBGeometry::Vec3T<T> c = T(2) * p - EBGeometry::Vec3T<T>(T(1), T(1), T(1));

      p = c / c.length();
    }
  }

  return points;
}

template <class T>
double
sumOf(const std::vector<EBGeometry::PointCloud::Hit<T>>& a_hits)
{
  double s = 0.0;

  for (const auto& h : a_hits) {
    s += double(h.distanceSquared) + 1e-9 * double(h.index);
  }

  return s;
}

template <class T>
double
sumOf(const std::vector<KHits<T>>& a_results)
{
  double s = 0.0;

  for (const auto& r : a_results) {
    for (uint32_t j = 0; j < r.m_count; j++) {
      s += double(r.m_hits[j].distanceSquared) + 1e-9 * double(r.m_hits[j].index);
    }
  }

  return s;
}

// Every case for one cloud type, one precision and one point distribution.
template <class T, class Cloud, class Build>
void
addCases(Perf::Suite& a_suite, const std::string& a_cloud, const std::string& a_shape, Build a_build)
{
  using Hit = EBGeometry::PointCloud::Hit<T>;

  const std::string prec   = std::is_same_v<T, float> ? "float" : "double";
  const std::string prefix = a_cloud + "/" + a_shape + "/";

  const auto common = [=](Perf::Case& c, std::size_t a_n) {
    c.param("points", a_n);
    c.param("K", K);
    c.param("W", W);
  };

  a_suite.add(prefix + "build/" + prec + "/host", [=](Perf::Case& c) {
    const std::size_t n      = c.size(500000, 2000);
    const auto        points = cloudPoints<T>(a_shape, n);

    std::unique_ptr<EBGeometry::Pool> pool;
    std::size_t                       count = 0;

    c.measureFresh([&] { pool = std::make_unique<EBGeometry::Pool>(EBGeometry::hostMemoryResource()); },
                   [&] { count = a_build(*pool, points).numPoints(); });

    c.poolBytes(pool->usedBytes());
    c.checksum(double(count));
    common(c, n);
  });

  for (const std::size_t k : {std::size_t(1), std::size_t(8)}) {
    a_suite.add(prefix + "all" + std::to_string(k) + "nn/" + prec + "/host", [=](Perf::Case& c) {
      const std::size_t n      = c.size(500000, 2000);
      const auto        points = cloudPoints<T>(a_shape, n);

      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
      const Cloud      cloud = a_build(pool, points);
      std::vector<Hit> graph;

      c.measure([&] { graph = cloud.allNearestNeighbors(k); }, n);

      c.checksum(sumOf(graph));
      common(c, n);
    });
  }

  // Query points near the cloud: for the cube, its bounding box grown by 5% on each side; for the
  // sphere, a shell from radius 0.95 to 1.05 (its hollow inside is no nearer the cloud than the outside
  // is). Queries far from the cloud are a separate case, closest-outside, below.
  const auto near = [=](std::size_t a_n) {
    using Vec3 = EBGeometry::Vec3T<T>;
    using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

    if (a_shape != "sphere") {
      return Perf::pointsInBox<T>(AABB(Vec3(T(-0.05), T(-0.05), T(-0.05)), Vec3(T(1.05), T(1.05), T(1.05))), a_n, 7u);
    }

    auto       q     = Perf::pointsInBox<T>(AABB(Vec3(T(-1), T(-1), T(-1)), Vec3(T(1), T(1), T(1))), a_n, 7u);
    const auto radii = Perf::pointsInBox<T>(AABB(Vec3(T(0.95), T(0), T(0)), Vec3(T(1.05), T(0), T(0))), a_n, 8u);

    for (std::size_t i = 0; i < a_n; i++) {
      q[i] = radii[i][0] * q[i] / q[i].length();
    }

    return q;
  };

  const auto queries = near;

  a_suite.add(prefix + "closest/" + prec + "/host", [=](Perf::Case& c) {
    const std::size_t n      = c.size(500000, 2000);
    const auto        points = cloudPoints<T>(a_shape, n);
    const auto        q      = queries(c.size(200000, 1000));

    EBGeometry::Pool             pool(EBGeometry::hostMemoryResource());
    const ClosestPoint<Cloud, T> query{a_build(pool, points)};
    std::vector<Hit>             hits(q.size());

    c.measure(
      [&] {
        for (std::size_t i = 0; i < q.size(); i++) {
          hits[i] = query(q[i]);
        }
      },
      q.size());

    c.checksum(sumOf(hits));
    common(c, n);
  });

  // Queries outside the cloud: a box five times the cloud's size, in which most points are far from
  // every cloud point. A hash grid searches outward cell by cell, which takes milliseconds per query
  // out there, so it gets few queries; the BVH gets enough for a repetition to take milliseconds too.
  const std::size_t outside = a_cloud == "PointCloudHashGrid" ? 100 : 20000;

  a_suite.add(prefix + "closest-outside/" + prec + "/host", [=](Perf::Case& c) {
    const std::size_t n      = c.size(500000, 2000);
    const auto        points = cloudPoints<T>(a_shape, n);
    const auto q = Perf::pointsInBox<T>(EBGeometry::BoundingVolumes::AABBT<T>(EBGeometry::Vec3T<T>(T(-2), T(-2), T(-2)),
                                                                              EBGeometry::Vec3T<T>(T(3), T(3), T(3))),
                                        c.size(outside, 10),
                                        9u);

    EBGeometry::Pool             pool(EBGeometry::hostMemoryResource());
    const ClosestPoint<Cloud, T> query{a_build(pool, points)};
    std::vector<Hit>             hits(q.size());

    c.measure(
      [&] {
        for (std::size_t i = 0; i < q.size(); i++) {
          hits[i] = query(q[i]);
        }
      },
      q.size());

    c.checksum(sumOf(hits));
    common(c, n);
  });

  a_suite.add(prefix + "neighbor/" + prec + "/host", [=](Perf::Case& c) {
    const std::size_t n      = c.size(500000, 2000);
    const auto        points = cloudPoints<T>(a_shape, n);

    EBGeometry::Pool                pool(EBGeometry::hostMemoryResource());
    const NearestNeighbor<Cloud, T> query{a_build(pool, points)};
    std::vector<Hit>                hits(n);

    c.measure(
      [&] {
        for (uint32_t i = 0; i < n; i++) {
          hits[i] = query(i);
        }
      },
      n);

    c.checksum(sumOf(hits));
    common(c, n);
  });

  // The KNearest nearest points: per point, the way a caller without the batch call (and a device
  // thread) asks for them, queries in Morton order. all8nn above is the batched host path.
  const auto mortonIndices = [](const std::vector<EBGeometry::Vec3T<T>>& a_points) {
    return EBGeometry::SFC::order<EBGeometry::SFC::Morton>(a_points);
  };

  a_suite.add(prefix + "neighbors" + std::to_string(KNearest) + "/" + prec + "/host", [=](Perf::Case& c) {
    const std::size_t n       = c.size(500000, 2000);
    const auto        points  = cloudPoints<T>(a_shape, n);
    const auto        indices = mortonIndices(points);

    EBGeometry::Pool                 pool(EBGeometry::hostMemoryResource());
    const NearestNeighbors<Cloud, T> query{a_build(pool, points)};
    std::vector<KHits<T>>            results(n);

    c.measure(
      [&] {
        for (std::size_t i = 0; i < n; i++) {
          results[i] = query(indices[i]);
        }
      },
      n);

    c.checksum(sumOf(results));
    c.param("k", KNearest);
    common(c, n);
  });

  a_suite.add(prefix + "closest" + std::to_string(KNearest) + "/" + prec + "/host", [=](Perf::Case& c) {
    const std::size_t n      = c.size(500000, 2000);
    const auto        points = cloudPoints<T>(a_shape, n);
    const auto        q      = Perf::mortonOrdered(queries(c.size(262144, 1000)));

    EBGeometry::Pool              pool(EBGeometry::hostMemoryResource());
    const ClosestPoints<Cloud, T> query{a_build(pool, points)};
    std::vector<KHits<T>>         results(q.size());

    c.measure(
      [&] {
        for (std::size_t i = 0; i < q.size(); i++) {
          results[i] = query(q[i]);
        }
      },
      q.size());

    c.checksum(sumOf(results));
    c.param("k", KNearest);
    common(c, n);
  });

  // The same queries on both sides: the speed of each, and whether they agree. Equal distances to
  // different points are ties, not disagreements.
  const auto agree = [](const Hit& a_host, const Hit& a_device) {
    const double diff = std::abs(double(a_host.distanceSquared) - double(a_device.distanceSquared));
    const double tol =
      1000.0 * double(EBGeometry::Math::Limits<T>::epsilon()) * std::max(1.0, double(a_host.distanceSquared));

    return std::make_pair(diff, a_host.index != a_device.index && diff > tol);
  };

  for (const std::string order : {"morton", "random"}) {
    a_suite.add(prefix + "closest-" + order + "/" + prec + "/hostdevice", [=](Perf::Case& c) {
      const std::size_t n      = c.size(500000, 2000);
      const auto        points = cloudPoints<T>(a_shape, n);
      const auto        random = queries(c.size(262144, 1000));
      const auto        q      = order == "morton" ? Perf::mortonOrdered(random) : random;

      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
      const Cloud      cloud = a_build(pool, points);

      const auto hits = Perf::measureHostAndDevice<Hit>(
        c,
        pool,
        q,
        [&](const EBGeometry::Pool& a_pool) { return ClosestPoint<Cloud, T>{cloud.rebasedView(a_pool)}; },
        agree);

      c.checksum(sumOf(hits));
      common(c, n);
    });
  }

  // Every point's nearest neighbour, the points visited in Morton order.
  a_suite.add(prefix + "neighbor-morton/" + prec + "/hostdevice", [=](Perf::Case& c) {
    const std::size_t n      = c.size(500000, 2000);
    const auto        points = cloudPoints<T>(a_shape, n);

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const Cloud      cloud   = a_build(pool, points);
    const auto       indices = EBGeometry::SFC::order<EBGeometry::SFC::Morton>(points);

    const auto hits = Perf::measureHostAndDevice<Hit>(
      c,
      pool,
      indices,
      [&](const EBGeometry::Pool& a_pool) { return NearestNeighbor<Cloud, T>{cloud.rebasedView(a_pool)}; },
      agree);

    c.checksum(sumOf(hits));
    common(c, n);
  });

  // The KNearest nearest points on both sides: on the device one thread per query, each keeping its
  // own KNearest best, queries in Morton order. There is no batched device k-NN (allNearestNeighbors is
  // host-only), so this is what a device caller has today.
  const auto agreeK = [](const KHits<T>& a_host, const KHits<T>& a_device) {
    double diff     = 0.0;
    bool   mismatch = a_host.m_count != a_device.m_count;

    // Distances, rank by rank: equal distances to different points are ties, not disagreements.
    for (uint32_t j = 0; j < a_host.m_count && j < a_device.m_count; j++) {
      const double dh  = double(a_host.m_hits[j].distanceSquared);
      const double d   = std::abs(dh - double(a_device.m_hits[j].distanceSquared));
      const double tol = 1000.0 * double(EBGeometry::Math::Limits<T>::epsilon()) * std::max(1.0, dh);

      diff     = std::max(diff, d);
      mismatch = mismatch || d > tol;
    }

    return std::make_pair(diff, mismatch);
  };

  const std::string k = std::to_string(KNearest);

  a_suite.add(prefix + "neighbors" + k + "-morton/" + prec + "/hostdevice", [=](Perf::Case& c) {
    const std::size_t n       = c.size(500000, 2000);
    const auto        points  = cloudPoints<T>(a_shape, n);
    const auto        indices = EBGeometry::SFC::order<EBGeometry::SFC::Morton>(points);

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const Cloud      cloud = a_build(pool, points);

    const auto results = Perf::measureHostAndDevice<KHits<T>>(
      c,
      pool,
      indices,
      [&](const EBGeometry::Pool& a_pool) { return NearestNeighbors<Cloud, T>{cloud.rebasedView(a_pool)}; },
      agreeK);

    c.checksum(sumOf(results));
    c.param("k", KNearest);
    common(c, n);
  });

  a_suite.add(prefix + "closest" + k + "-morton/" + prec + "/hostdevice", [=](Perf::Case& c) {
    const std::size_t n      = c.size(500000, 2000);
    const auto        points = cloudPoints<T>(a_shape, n);
    const auto        q      = Perf::mortonOrdered(queries(c.size(262144, 1000)));

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const Cloud      cloud = a_build(pool, points);

    const auto results = Perf::measureHostAndDevice<KHits<T>>(
      c,
      pool,
      q,
      [&](const EBGeometry::Pool& a_pool) { return ClosestPoints<Cloud, T>{cloud.rebasedView(a_pool)}; },
      agreeK);

    c.checksum(sumOf(results));
    c.param("k", KNearest);
    common(c, n);
  });
}

// Every case for one cloud type and one point set, both precisions together.
template <template <class> class Cloud>
void
addCloud(Perf::Suite& a_suite, const std::string& a_cloud, const std::string& a_shape)
{
  addCases<float, Cloud<float>>(a_suite, a_cloud, a_shape, [](EBGeometry::Pool& a_pool, const auto& a_points) {
    return Cloud<float>(a_pool, a_points);
  });
  addCases<double, Cloud<double>>(a_suite, a_cloud, a_shape, [](EBGeometry::Pool& a_pool, const auto& a_points) {
    return Cloud<double>(a_pool, a_points);
  });
}

template <class T>
using BVHCloud = EBGeometry::PointCloudBVH<T, K, W>;

template <class T>
using GridCloud = EBGeometry::PointCloudHashGrid<T>;

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "PointCloud");

  for (const std::string shape : {"uniform", "sphere"}) {
    addCloud<BVHCloud>(suite, "PointCloudBVH", shape);
    addCloud<GridCloud>(suite, "PointCloudHashGrid", shape);
  }

  return suite.run();
}
