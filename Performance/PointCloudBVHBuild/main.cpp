// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for building PointCloudBVH where its pool's memory is: a rebuild in place on the host, and the
// build and the rebuild in device memory as host/device pairs, over 500,000 points uniform in the unit cube, in
// float and double. The host's one-off build is PointCloud's case. See ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The parameters that select code, spelled out rather than taken from the defaults.
constexpr std::size_t K              = 4;
constexpr std::size_t W              = 4;
constexpr std::size_t TargetLeafSize = 16 * W;

// The cloud's size, at full and at smoke size.
constexpr std::size_t CloudPoints      = 500000;
constexpr std::size_t SmokeCloudPoints = 2000;

template <class T>
using BVH = EBGeometry::PointCloudBVH<T, K, W>;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using Hit = EBGeometry::PointCloud::Hit<T>;

// The cloud, and the same points moved: shifted and stretched, so that a rebuild finds a new box.
template <class T>
std::vector<Vec3<T>>
cloud(const std::size_t a_n)
{
  return Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), a_n, Perf::Seed::Cloud);
}

template <class T>
std::vector<Vec3<T>>
moved(const std::vector<Vec3<T>>& a_points)
{
  std::vector<Vec3<T>> points = a_points;

  for (auto& p : points) {
    p = Vec3<T>(T(1.5) * p[0] + T(0.2), T(0.8) * p[1] - T(0.1), p[2] + T(0.3));
  }

  return points;
}

// A BVH's closest points to the build checks' queries, on the host: a BVH in device memory is read through a host
// mirror of its frozen pool.
template <class T>
std::vector<Hit<T>>
answers(const BVH<T>& a_bvh, EBGeometry::Pool& a_pool)
{
  const auto queries =
    Perf::pointsInBox<T>(Perf::cube(T(-0.1), T(1.6)), Perf::BuildCheckPoints, Perf::Seed::BuildChecks);

  if (!a_pool.isFrozen()) {
    a_pool.freeze();
  }

  const EBGeometry::Pool host = EBGeometry::Pool::mirror(a_pool, EBGeometry::hostMemoryResource());
  const BVH<T>           view = a_bvh.rebasedView(host);

  std::vector<Hit<T>> hits(queries.size());

  for (std::size_t i = 0; i < queries.size(); i++) {
    hits[i] = view.closestPoint(queries[i]);
  }

  return hits;
}

// The pair's metrics: the host's time beside the device's, and whether their BVHs answer alike.
template <class T>
void
comparePair(Perf::Case&                a_case,
            const double               a_hostMs,
            const std::vector<Hit<T>>& a_host,
            const std::vector<Hit<T>>& a_device)
{
  double mismatches = 0.0;

  for (std::size_t i = 0; i < a_host.size(); i++) {
    const bool same = a_host[i].index == a_device[i].index && a_host[i].distanceSquared == a_device[i].distanceSquared;

    mismatches += same ? 0.0 : 1.0;
  }

  const double deviceMs = a_case.summary().median;

  a_case.metric(Perf::Key::HostMs, a_hostMs);
  a_case.metric(Perf::Key::HostThreads, 1.0);
  a_case.metric(Perf::Key::HostOverDevice, deviceMs > 0.0 ? a_hostMs / deviceMs : 0.0);
  a_case.metric(Perf::Key::MaxDiff, 0.0);
  a_case.metric(Perf::Key::Mismatches, mismatches);

  a_case.checksum(Perf::checksumOf(a_host));
  a_case.indexChecksum(Perf::indexChecksumOf(a_host));
}

// The positions in device memory, where the device build reads them.
template <class T>
struct OnDevice
{
  EBGeometry::Pool                     host;
  EBGeometry::Pool                     device;
  EBGeometry::PODVector<Vec3<T>>       positions;
  EBGeometry::PointCloud::FromArray<T> source;

  explicit OnDevice(const std::vector<Vec3<T>>& a_points)
    : host(EBGeometry::hostMemoryResource()), device(EBGeometry::hostMemoryResource())
  {
    const auto n = static_cast<std::uint32_t>(a_points.size());

    positions.reserveFrom(host, n);
    positions.assign(host.base(), a_points.data(), n);
    host.freeze();

    device = EBGeometry::Pool::mirror(host, EBGeometry::GPU::memoryResource());
    source = EBGeometry::PointCloud::FromArray<T>{positions.data(device.base())};
  }
};

template <class T>
void
addCases(Perf::Suite& a_suite)
{
  const std::string prefix = "PointCloudBVHBuild/uniform";
  const std::string prec   = Perf::precisionName<T>();

  const auto points = [](Perf::Case& c) {
    const std::size_t n = c.size(CloudPoints, SmokeCloudPoints);

    c.param("points", n);
    c.param("K", K);
    c.param("W", W);
    c.param("targetLeafSize", TargetLeafSize);

    return cloud<T>(n);
  };

  // ── Host: a rebuild in place, nothing reserved ──

  a_suite.add(Perf::caseName({prefix, "rebuild", prec, "host"}), [=](Perf::Case& c) {
    const auto first  = points(c);
    const auto second = moved(first);
    const auto n      = static_cast<std::uint32_t>(first.size());

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    BVH<T> bvh = BVH<T>::build(pool, first, TargetLeafSize, n);

    pool.freeze();
    c.poolBytes(pool.usedBytes());

    bool toSecond = true;

    c.measure(
      [&] {
        bvh.rebuild(EBGeometry::PointCloud::FromArray<T>{toSecond ? second.data() : first.data()}, n);
        toSecond = !toSecond;
      },
      n);

    bvh.rebuild(EBGeometry::PointCloud::FromArray<T>{second.data()}, n);

    const auto hits = answers(bvh, pool);

    c.checksum(Perf::checksumOf(hits));
    c.indexChecksum(Perf::indexChecksumOf(hits));
  });

  // ── Host/device pairs: the device's build and rebuild in device memory, the host's in host memory ──

  a_suite.add(Perf::caseName({prefix, "build", prec, "hostdevice"}), [=](Perf::Case& c) {
    const auto first = points(c);
    const auto n     = static_cast<std::uint32_t>(first.size());

    const OnDevice<T> input(first);

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
      hostBVH  = std::make_unique<BVH<T>>(BVH<T>::build(*hostPool, first, TargetLeafSize));
    });

    comparePair(c, Perf::median(hostTimes), answers(*hostBVH, *hostPool), answers(*deviceBVH, *devicePool));
  });

  a_suite.add(Perf::caseName({prefix, "rebuild", prec, "hostdevice"}), [=](Perf::Case& c) {
    const auto first  = points(c);
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
      hostBVH.rebuild(EBGeometry::PointCloud::FromArray<T>{toSecond ? second.data() : first.data()}, n);
      toSecond = !toSecond;
    });

    deviceBVH.rebuild(inputSecond.source, n);
    hostBVH.rebuild(EBGeometry::PointCloud::FromArray<T>{second.data()}, n);

    comparePair(c, Perf::median(hostTimes), answers(hostBVH, hostPool), answers(deviceBVH, devicePool));
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "PointCloudBVHBuild");

  addCases<float>(suite);
  addCases<double>(suite);

  return suite.run();
}
