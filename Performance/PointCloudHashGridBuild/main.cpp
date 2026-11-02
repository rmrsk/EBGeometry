// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for building PointCloudHashGrid where its pool's memory is: a rebuild in place on the host,
// and the build and the rebuild in device memory as host/device pairs, over 500,000 points uniform in the unit
// cube, in float and double. The host's one-off build is PointCloud's case. See ../README.md and
// PerformanceTesting.rst.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The parameter that selects code, spelled out rather than taken from build()'s default argument.
constexpr std::size_t TargetPerCell = 1;

// The cloud's size, at full and at smoke size.
constexpr std::size_t CloudPoints      = 500000;
constexpr std::size_t SmokeCloudPoints = 2000;

template <class T>
using Grid = EBGeometry::PointCloudHashGrid<T>;

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

// A grid's closest points to the build checks' queries, on the host: a grid in device memory is read through a
// host mirror of its frozen pool.
template <class T>
std::vector<Hit<T>>
answers(const Grid<T>& a_grid, EBGeometry::Pool& a_pool)
{
  const auto queries =
    Perf::pointsInBox<T>(Perf::cube(T(-0.1), T(1.6)), Perf::BuildCheckPoints, Perf::Seed::BuildChecks);

  if (!a_pool.isFrozen()) {
    a_pool.freeze();
  }

  const EBGeometry::Pool host = EBGeometry::Pool::mirror(a_pool, EBGeometry::hostMemoryResource());
  const Grid<T>          view = a_grid.rebasedView(host);

  std::vector<Hit<T>> hits(queries.size());

  for (std::size_t i = 0; i < queries.size(); i++) {
    hits[i] = view.closestPoint(queries[i]);
  }

  return hits;
}

// The pair's metrics: the host's time beside the device's, and whether their grids answer alike.
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
  const std::string prefix = "PointCloudHashGridBuild/uniform";
  const std::string prec   = Perf::precisionName<T>();

  const auto points = [](Perf::Case& c) {
    const std::size_t n = c.size(CloudPoints, SmokeCloudPoints);

    c.param("points", n);
    c.param("targetPerCell", TargetPerCell);

    return cloud<T>(n);
  };

  // ── Host: a rebuild in place, nothing reserved ──

  a_suite.add(Perf::caseName({prefix, "rebuild", prec, "host"}), [=](Perf::Case& c) {
    const auto first  = points(c);
    const auto second = moved(first);
    const auto n      = static_cast<std::uint32_t>(first.size());

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    Grid<T> grid = Grid<T>::build(pool, first, T(TargetPerCell), n);

    pool.freeze();
    c.poolBytes(pool.usedBytes());

    bool toSecond = true;

    c.measure(
      [&] {
        grid.rebuild(
          EBGeometry::PointCloud::FromArray<T>{toSecond ? second.data() : first.data()}, n, T(TargetPerCell));
        toSecond = !toSecond;
      },
      n);

    grid.rebuild(EBGeometry::PointCloud::FromArray<T>{second.data()}, n, T(TargetPerCell));

    const auto hits = answers(grid, pool);

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
    std::unique_ptr<Grid<T>>          deviceGrid;

    c.measure(
      [&] {
        deviceGrid.reset();
        devicePool = std::make_unique<EBGeometry::Pool>(EBGeometry::GPU::memoryResource());
        deviceGrid = std::make_unique<Grid<T>>(Grid<T>::build(*devicePool, input.source, n, T(TargetPerCell)));
      },
      n);

    c.poolBytes(devicePool->usedBytes());

    // The host: the same build in host memory.
    std::unique_ptr<EBGeometry::Pool> hostPool;
    std::unique_ptr<Grid<T>>          hostGrid;

    const std::vector<double> hostTimes = c.repeat([&] {
      hostGrid.reset();
      hostPool = std::make_unique<EBGeometry::Pool>(EBGeometry::hostMemoryResource());
      hostGrid = std::make_unique<Grid<T>>(Grid<T>::build(*hostPool, first, T(TargetPerCell)));
    });

    comparePair(c, Perf::median(hostTimes), answers(*hostGrid, *hostPool), answers(*deviceGrid, *devicePool));
  });

  a_suite.add(Perf::caseName({prefix, "rebuild", prec, "hostdevice"}), [=](Perf::Case& c) {
    const auto first  = points(c);
    const auto second = moved(first);
    const auto n      = static_cast<std::uint32_t>(first.size());

    const OnDevice<T> inputFirst(first);
    const OnDevice<T> inputSecond(second);

    EBGeometry::Pool devicePool(EBGeometry::GPU::memoryResource());
    EBGeometry::Pool hostPool(EBGeometry::hostMemoryResource());

    Grid<T> deviceGrid = Grid<T>::build(devicePool, inputFirst.source, n, T(TargetPerCell), n);
    Grid<T> hostGrid   = Grid<T>::build(hostPool, first, T(TargetPerCell), n);

    devicePool.freeze();
    hostPool.freeze();
    c.poolBytes(devicePool.usedBytes());

    bool toSecond = true;

    c.measure(
      [&] {
        deviceGrid.rebuild(toSecond ? inputSecond.source : inputFirst.source, n, T(TargetPerCell));
        toSecond = !toSecond;
      },
      n);

    toSecond = true;

    const std::vector<double> hostTimes = c.repeat([&] {
      hostGrid.rebuild(
        EBGeometry::PointCloud::FromArray<T>{toSecond ? second.data() : first.data()}, n, T(TargetPerCell));
      toSecond = !toSecond;
    });

    deviceGrid.rebuild(inputSecond.source, n, T(TargetPerCell));
    hostGrid.rebuild(EBGeometry::PointCloud::FromArray<T>{second.data()}, n, T(TargetPerCell));

    comparePair(c, Perf::median(hostTimes), answers(hostGrid, hostPool), answers(deviceGrid, devicePool));
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "PointCloudHashGridBuild");

  addCases<float>(suite);
  addCases<double>(suite);

  return suite.run();
}
