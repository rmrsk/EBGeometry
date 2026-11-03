// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for BVH::refit(), which refits a PackedBVH where its pool's memory is: the host's sweep, and
// one launch per tree level in device memory as a host/device pair, for a BVH of 1,000,000 point ids whose points
// the caller keeps in the pool, in float and double. A mesh SDF's refit is Mesh's case. See ../README.md and
// PerformanceTesting.rst.

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The parameters that select code, spelled out rather than taken from the defaults: bottom-up build.
constexpr std::size_t K        = 4;
constexpr std::size_t LeafSize = 4;

// The cloud's size, at full and at smoke size.
constexpr std::size_t CloudPoints      = 1000000;
constexpr std::size_t SmokeCloudPoints = 2000;

// The refits a repetition does: one takes a few milliseconds on the host and well under one on a device.
constexpr std::size_t Refits = 10;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

template <class T>
using Packed = EBGeometry::BVH::PackedBVH<T, std::uint32_t, K>;

// The box of point a_id, read from the caller's points.
template <class T>
struct PointBox
{
  const Vec3<T>* m_points;

  EBGEOMETRY_HOST_DEVICE
  AABB<T>
  operator()(const std::uint32_t a_id) const noexcept
  {
    return AABB<T>(m_points[a_id], m_points[a_id]);
  }
};

// The BVH, its schedule, and the points in two positions (as built, and moved: shifted and stretched),
// all in one frozen host pool.
template <class T>
struct Setup
{
  EBGeometry::Pool               pool;
  EBGeometry::PODVector<Vec3<T>> first;
  EBGeometry::PODVector<Vec3<T>> second;
  Packed<T>                      bvh;
  EBGeometry::BVH::RefitSchedule schedule;

  explicit Setup(const std::vector<Vec3<T>>& a_points)
    : pool(EBGeometry::hostMemoryResource()), bvh(build(pool, first, second, a_points)), schedule(pool, bvh)
  {
    pool.freeze();
  }

  static Packed<T>
  build(EBGeometry::Pool&               a_pool,
        EBGeometry::PODVector<Vec3<T>>& a_first,
        EBGeometry::PODVector<Vec3<T>>& a_second,
        const std::vector<Vec3<T>>&     a_points)
  {
    const auto n = static_cast<std::uint32_t>(a_points.size());

    std::vector<Vec3<T>>                           moved(n);
    std::vector<std::pair<std::uint32_t, AABB<T>>> flat(n);

    for (std::uint32_t i = 0; i < n; i++) {
      const Vec3<T>& p = a_points[i];

      moved[i] = Vec3<T>(T(1.5) * p[0] + T(0.2), T(0.8) * p[1] - T(0.1), p[2] + T(0.3));
      flat[i]  = {i, AABB<T>(p, p)};
    }

    a_first.reserveFrom(a_pool, n);
    a_first.assign(a_pool.base(), a_points.data(), n);
    a_second.reserveFrom(a_pool, n);
    a_second.assign(a_pool.base(), moved.data(), n);

    return Packed<T>::buildBottomUp(a_pool, flat, LeafSize);
  }
};

// Every node's box, as the corners in node order: what a refit made, read on the host.
template <class T>
std::vector<T>
boxCorners(const Packed<T>& a_bvh)
{
  std::vector<T> corners;

  for (const auto& node : a_bvh.getNodes()) {
    const AABB<T> box = a_bvh.getBoundingVolume(node);

    for (int d = 0; d < 3; d++) {
      corners.push_back(box.getLowCorner()[d]);
      corners.push_back(box.getHighCorner()[d]);
    }
  }

  return corners;
}

template <class T>
void
addCases(Perf::Suite& a_suite)
{
  const std::string prefix = "BVHRefit/uniform";
  const std::string prec   = Perf::precisionName<T>();

  const auto points = [](Perf::Case& c) {
    const std::size_t n = c.size(CloudPoints, SmokeCloudPoints);

    c.param("points", n);
    c.param("K", K);
    c.param("leafSize", LeafSize);
    c.param("refits", Refits);

    return Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), n, Perf::Seed::Cloud);
  };

  // Refits alternating between the two positions, ending on the moved one.
  const auto refits = [](EBGeometry::Pool& a_pool, const Setup<T>& a_setup) {
    for (std::size_t r = 0; r < Refits; r++) {
      const auto& points = (r % 2 == 0) ? a_setup.first : a_setup.second;

      EBGeometry::BVH::refit(a_pool, a_setup.bvh, a_setup.schedule, PointBox<T>{points.data(a_pool.base())});
    }
  };

  // ── Host: PackedBVH::refit()'s sweep, through BVH::refit() on a host pool ──

  a_suite.add(Perf::caseName({prefix, "refit", prec, "host"}), [=](Perf::Case& c) {
    Setup<T> setup(points(c));

    c.poolBytes(setup.pool.usedBytes());

    c.measure([&] { refits(setup.pool, setup); }, Refits);

    c.checksum(Perf::checksumOf(boxCorners(setup.bvh)));
  });

  // ── Host/device pair: a launch per level in device memory, the host's sweep in host memory ──

  a_suite.add(Perf::caseName({prefix, "refit", prec, "hostdevice"}), [=](Perf::Case& c) {
    Setup<T> setup(points(c));

    EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(setup.pool, EBGeometry::GPU::memoryResource());

    c.poolBytes(devicePool.usedBytes());

    c.measure([&] { refits(devicePool, setup); }, Refits);

    const std::vector<double> hostTimes = c.repeat([&] { refits(setup.pool, setup); });

    const EBGeometry::Pool back   = EBGeometry::Pool::mirror(devicePool, EBGeometry::hostMemoryResource());
    const auto             host   = boxCorners(setup.bvh);
    const auto             device = boxCorners(setup.bvh.rebasedView(back));

    double mismatches = 0.0;

    for (std::size_t i = 0; i < host.size(); i++) {
      mismatches += (host[i] == device[i]) ? 0.0 : 1.0;
    }

    const double hostMs   = Perf::median(hostTimes);
    const double deviceMs = c.summary().median;

    c.metric(Perf::Key::HostMs, hostMs);
    c.metric(Perf::Key::HostThreads, 1.0);
    c.metric(Perf::Key::HostOverDevice, deviceMs > 0.0 ? hostMs / deviceMs : 0.0);
    c.metric(Perf::Key::MaxDiff, 0.0);
    c.metric(Perf::Key::Mismatches, mismatches);

    c.checksum(Perf::checksumOf(host));
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "BVHRefit");

  addCases<float>(suite);
  addCases<double>(suite);

  return suite.run();
}
