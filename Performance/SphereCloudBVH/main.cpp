// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for SphereCloudBVH, over 500,000 spheres of differing radii (centers uniform in the unit cube),
// in float and double. A host build runs the host cases (the build, a rebuild in place, the closest sphere to
// points in and around the cloud); a CUDA or HIP build runs the closest-sphere queries, in Morton and in random
// order, and the build and the rebuild in device memory, as host/device pairs. See ../README.md and
// PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The parameters that select code, spelled out rather than taken from the defaults: the branching factor, the
// group width and the target spheres per leaf (its default, 16 W).
constexpr std::size_t K              = 4;
constexpr std::size_t W              = 4;
constexpr std::size_t TargetLeafSize = 16 * W;

// The cloud's size and the numbers of queries, at full and at smoke size.
constexpr std::size_t CloudSpheres      = 500000;
constexpr std::size_t SmokeCloudSpheres = 2000;
constexpr std::size_t HostQueries       = 200000;
constexpr std::size_t PairQueries       = 262144;
constexpr std::size_t SmokeQueries      = 1000;

template <class T>
using BVH = EBGeometry::SphereCloudBVH<T, K, W>;

template <class T>
using Sphere = EBGeometry::SphereCloud::Sphere<T>;

template <class T>
using Hit = EBGeometry::SphereCloud::Hit<T>;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// The spheres: centers uniform in the unit cube, radii uniform in [0.002, 0.006], so that about one sphere in ten
// overlaps another and the spheres fill about a tenth of the cube.
template <class T>
std::vector<Sphere<T>>
spheres(const std::size_t a_n)
{
  const auto centers = Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), a_n, Perf::Seed::Cloud);
  const auto radii   = Perf::pointsInBox<T>(Perf::cube(T(0.002), T(0.006)), a_n, Perf::Seed::SphereRadii);

  std::vector<Sphere<T>> result(a_n);

  for (std::size_t i = 0; i < a_n; i++) {
    result[i] = Sphere<T>{centers[i], radii[i][0]};
  }

  return result;
}

// The same spheres moved and grown, so that a rebuild finds a new box and new boxes.
template <class T>
std::vector<Sphere<T>>
moved(const std::vector<Sphere<T>>& a_spheres)
{
  std::vector<Sphere<T>> result = a_spheres;

  for (auto& s : result) {
    s.m_center = Vec3<T>(T(1.5) * s.m_center[0] + T(0.2), T(0.8) * s.m_center[1] - T(0.1), s.m_center[2] + T(0.3));
    s.m_radius = T(1.25) * s.m_radius;
  }

  return result;
}

// Queries in and around the cloud: some inside a sphere, most between them.
template <class T>
std::vector<Vec3<T>>
queries(const std::size_t a_n, const std::uint64_t a_seed = Perf::Seed::NearQueries)
{
  return Perf::pointsInBox<T>(Perf::cube(T(-0.05), T(1.05)), a_n, a_seed);
}

// The closest sphere to a point.
template <class T>
struct ClosestSphere
{
  BVH<T> m_cloud;

  EBGEOMETRY_HOST_DEVICE
  Hit<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    return m_cloud.closestSphere(a_point);
  }
};

// Whether a host and a device hit agree: their signed distances are within agreementTolerance(), whichever spheres
// they found (equal distances to different spheres are ties, not disagreements).
template <class T>
std::pair<double, bool>
agreeSphere(const Hit<T>& a_host, const Hit<T>& a_device)
{
  const double dh   = static_cast<double>(a_host.distance);
  const double diff = std::abs(dh - static_cast<double>(a_device.distance));

  return {diff, diff > Perf::agreementTolerance<T>(std::abs(dh))};
}

// A case's checksums: the sum of the distances found, and an exact hash of the spheres found.
template <class T>
void
record(Perf::Case& a_case, const std::vector<Hit<T>>& a_hits)
{
  std::vector<T>      distances(a_hits.size());
  Perf::IndexChecksum indices;

  for (std::size_t i = 0; i < a_hits.size(); i++) {
    distances[i] = a_hits[i].distance;
    indices.add(a_hits[i].index);
  }

  a_case.checksum(Perf::checksumOf(distances));
  a_case.indexChecksum(indices.value());
}

// A BVH's closest spheres to the build checks' queries, on the host: a BVH in device memory is read through a host
// mirror of its frozen pool.
template <class T>
std::vector<Hit<T>>
answers(const BVH<T>& a_bvh, EBGeometry::Pool& a_pool)
{
  const auto points = queries<T>(Perf::BuildCheckPoints, Perf::Seed::BuildChecks);

  if (!a_pool.isFrozen()) {
    a_pool.freeze();
  }

  const EBGeometry::Pool host = EBGeometry::Pool::mirror(a_pool, EBGeometry::hostMemoryResource());
  const BVH<T>           view = a_bvh.rebasedView(host);

  return Perf::queryAll(ClosestSphere<T>{view}, points);
}

// A build pair's metrics: the host's time beside the device's, and whether their BVHs answer alike.
template <class T>
void
compareBuilds(Perf::Case&                a_case,
              const double               a_hostMs,
              const std::vector<Hit<T>>& a_host,
              const std::vector<Hit<T>>& a_device)
{
  double mismatches = 0.0;

  for (std::size_t i = 0; i < a_host.size(); i++) {
    mismatches += (a_host[i].index == a_device[i].index && a_host[i].distance == a_device[i].distance) ? 0.0 : 1.0;
  }

  const double deviceMs = a_case.summary().median;

  a_case.metric(Perf::Key::HostMs, a_hostMs);
  a_case.metric(Perf::Key::HostThreads, 1.0);
  a_case.metric(Perf::Key::HostOverDevice, deviceMs > 0.0 ? a_hostMs / deviceMs : 0.0);
  a_case.metric(Perf::Key::MaxDiff, 0.0);
  a_case.metric(Perf::Key::Mismatches, mismatches);

  record(a_case, a_host);
}

// The spheres in device memory, where a device build reads them.
template <class T>
struct OnDevice
{
  EBGeometry::Pool                      host;
  EBGeometry::Pool                      device;
  EBGeometry::PODVector<Sphere<T>>      stored;
  EBGeometry::SphereCloud::FromArray<T> source;

  explicit OnDevice(const std::vector<Sphere<T>>& a_spheres)
    : host(EBGeometry::hostMemoryResource()), device(EBGeometry::hostMemoryResource())
  {
    const auto n = static_cast<std::uint32_t>(a_spheres.size());

    stored.reserveFrom(host, n);
    stored.assign(host.base(), a_spheres.data(), n);
    host.freeze();

    device = EBGeometry::Pool::mirror(host, EBGeometry::GPU::memoryResource());
    source = EBGeometry::SphereCloud::FromArray<T>{stored.data(device.base())};
  }
};

template <class T>
void
addCases(Perf::Suite& a_suite)
{
  const std::string prefix = "SphereCloudBVH/polydisperse";
  const std::string prec   = Perf::precisionName<T>();

  const auto cloud = [](Perf::Case& c) {
    const std::size_t n = c.size(CloudSpheres, SmokeCloudSpheres);

    c.param("spheres", n);
    c.param("K", K);
    c.param("W", W);
    c.param("targetLeafSize", TargetLeafSize);

    return spheres<T>(n);
  };

  // ── Host cases ──

  a_suite.add(Perf::caseName({prefix, "build", prec, "host"}), [=](Perf::Case& c) {
    const auto s = cloud(c);
    const auto built =
      Perf::measureBuild<BVH<T>>(c, [&](EBGeometry::Pool& a_pool) { return BVH<T>(a_pool, s, TargetLeafSize); });

    c.poolBytes(built.pool->usedBytes());
    record(
      c, Perf::queryAll(ClosestSphere<T>{*built.object}, queries<T>(Perf::BuildCheckPoints, Perf::Seed::BuildChecks)));
  });

  a_suite.add(Perf::caseName({prefix, "rebuild", prec, "host"}), [=](Perf::Case& c) {
    const auto first  = cloud(c);
    const auto second = moved(first);
    const auto n      = static_cast<std::uint32_t>(first.size());

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    BVH<T> bvh = BVH<T>::build(pool, first, TargetLeafSize, n);

    pool.freeze();
    c.poolBytes(pool.usedBytes());

    bool toSecond = true;

    c.measure(
      [&] {
        bvh.rebuild(EBGeometry::SphereCloud::FromArray<T>{toSecond ? second.data() : first.data()}, n);
        toSecond = !toSecond;
      },
      n);

    bvh.rebuild(EBGeometry::SphereCloud::FromArray<T>{second.data()}, n);

    record(c, answers(bvh, pool));
  });

  a_suite.add(Perf::caseName({prefix, "closest-random", prec, "host"}), [=](Perf::Case& c) {
    const auto          s = cloud(c);
    const auto          q = queries<T>(c.size(HostQueries, SmokeQueries));
    EBGeometry::Pool    pool(EBGeometry::hostMemoryResource());
    std::vector<Hit<T>> hits;

    Perf::measureHostQuery(c, ClosestSphere<T>{BVH<T>(pool, s, TargetLeafSize)}, q, hits);
    c.poolBytes(pool.usedBytes());
    record(c, hits);
  });

  // ── Host/device pairs: the same queries on both sides, and the build and rebuild in device memory ──

  for (const Perf::Order order : Perf::Orders) {
    a_suite.add(Perf::caseName({prefix, std::string("closest-") + Perf::orderName(order), prec, "hostdevice"}),
                [=](Perf::Case& c) {
                  const auto       s = cloud(c);
                  const auto       q = Perf::inOrder(order, queries<T>(c.size(PairQueries, SmokeQueries)));
                  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
                  const BVH<T>     bvh(pool, s, TargetLeafSize);

                  c.poolBytes(pool.usedBytes());

                  const auto hits = Perf::measureHostAndDevice<Hit<T>>(
                    c,
                    pool,
                    q,
                    [&](const EBGeometry::Pool& a_pool) { return ClosestSphere<T>{bvh.rebasedView(a_pool)}; },
                    agreeSphere<T>);

                  record(c, hits);
                });
  }

  a_suite.add(Perf::caseName({prefix, "build", prec, "hostdevice"}), [=](Perf::Case& c) {
    const auto s = cloud(c);
    const auto n = static_cast<std::uint32_t>(s.size());

    const OnDevice<T> input(s);

    // The device: a fresh pool and a one-off build each time.
    std::unique_ptr<EBGeometry::Pool> devicePool;
    std::unique_ptr<BVH<T>>           deviceBVH;

    c.measure(
      [&] {
        deviceBVH.reset();
        devicePool = std::make_unique<EBGeometry::Pool>(EBGeometry::GPU::memoryResource());
        deviceBVH  = std::make_unique<BVH<T>>(BVH<T>::build(*devicePool, input.source, n, TargetLeafSize));
      },
      n);

    c.poolBytes(devicePool->usedBytes());

    // The host: the same build in host memory.
    std::unique_ptr<EBGeometry::Pool> hostPool;
    std::unique_ptr<BVH<T>>           hostBVH;

    const std::vector<double> hostTimes = c.repeat([&] {
      hostBVH.reset();
      hostPool = std::make_unique<EBGeometry::Pool>(EBGeometry::hostMemoryResource());
      hostBVH  = std::make_unique<BVH<T>>(BVH<T>::build(*hostPool, s, TargetLeafSize));
    });

    compareBuilds(c, Perf::median(hostTimes), answers(*hostBVH, *hostPool), answers(*deviceBVH, *devicePool));
  });

  a_suite.add(Perf::caseName({prefix, "rebuild", prec, "hostdevice"}), [=](Perf::Case& c) {
    const auto first  = cloud(c);
    const auto second = moved(first);
    const auto n      = static_cast<std::uint32_t>(first.size());

    const OnDevice<T> inputFirst(first);
    const OnDevice<T> inputSecond(second);

    EBGeometry::Pool devicePool(EBGeometry::GPU::memoryResource());
    EBGeometry::Pool hostPool(EBGeometry::hostMemoryResource());

    BVH<T> deviceBVH = BVH<T>::build(devicePool, inputFirst.source, n, TargetLeafSize, n);
    BVH<T> hostBVH   = BVH<T>::build(hostPool, first, TargetLeafSize, n);

    devicePool.freeze();
    hostPool.freeze();
    c.poolBytes(devicePool.usedBytes());

    bool toSecond = true;

    c.measure(
      [&] {
        deviceBVH.rebuild(toSecond ? inputSecond.source : inputFirst.source, n);
        toSecond = !toSecond;
      },
      n);

    toSecond = true;

    const std::vector<double> hostTimes = c.repeat([&] {
      hostBVH.rebuild(EBGeometry::SphereCloud::FromArray<T>{toSecond ? second.data() : first.data()}, n);
      toSecond = !toSecond;
    });

    deviceBVH.rebuild(inputSecond.source, n);
    hostBVH.rebuild(EBGeometry::SphereCloud::FromArray<T>{second.data()}, n);

    compareBuilds(c, Perf::median(hostTimes), answers(hostBVH, hostPool), answers(deviceBVH, devicePool));
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "SphereCloudBVH");

  addCases<float>(suite);
  addCases<double>(suite);

  return suite.run();
}
