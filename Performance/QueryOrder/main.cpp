// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Performance cases for queries in Morton order (EBGeometry_GPUOrder.hpp, and the ordered GPU::transform() of
// EBGeometry_GPUTransform.hpp): computing the order of a million random points where its pool is, and a
// TriMeshSDF's signed distance at random points in the armadillo's box, run as the points were drawn and in their
// Morton order, the second naming the first. On the host, and as host/device pairs. See ../README.md and
// PerformanceTesting.rst.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The mesh's branching factor and leaf width, spelled out (see ../Mesh/main.cpp).
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// The points ordered, and the queries, at full and at smoke size.
constexpr std::size_t OrderPoints  = 1000000;
constexpr std::size_t SmokeOrder   = 2000;
constexpr std::size_t MeshQueries  = 100000;
constexpr std::size_t SmokeQueries = 1000;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

template <class T>
using Distance = Perf::SignedDistance<Mesh<T>, T>;

// The order's exact checksum: a hash of its entries.
std::uint64_t
orderChecksum(const std::vector<std::uint32_t>& a_order)
{
  Perf::IndexChecksum sum;

  for (const std::uint32_t i : a_order) {
    sum.add(i);
  }

  return sum.value();
}

// The order of points, computed for a pool on a resource: on the host for host memory, on the device for device
// memory. The points, and the order's array, are in that pool; a_run times the computation.
template <class T, class Run>
std::vector<std::uint32_t>
orderOn(EBGeometry::MemoryResource& a_resource, const std::vector<Vec3<T>>& a_points, Run&& a_run)
{
  using EBGeometry::PODVector;
  using EBGeometry::Pool;

  const auto n = static_cast<std::uint32_t>(a_points.size());

  Pool                             host(EBGeometry::hostMemoryResource());
  PODVector<Vec3<T>>               points;
  PODVector<std::uint32_t>         order;
  const std::vector<std::uint32_t> zeros(n, 0U);

  points.reserveFrom(host, n);
  order.reserveFrom(host, n);
  points.assign(host.base(), a_points.data(), n);
  order.assign(host.base(), zeros.data(), n);
  host.freeze();

  const Pool there  = Pool::mirror(host, a_resource);
  const auto source = EBGeometry::PointCloud::FromArray<T>{points.data(static_cast<const void*>(there.base()))};

  a_run([&] { EBGeometry::GPU::mortonOrder(there, source, n, order); });

  const Pool                 back = Pool::mirror(there, EBGeometry::hostMemoryResource());
  std::vector<std::uint32_t> result(n);

  for (std::uint32_t i = 0; i < n; i++) {
    result[i] = order.at(back.base(), i);
  }

  return result;
}

// The mesh, read into a pool, and random points in its box, as drawn.
template <class T>
struct Setup
{
  Mesh<T>              mesh;
  std::vector<Vec3<T>> points;
};

template <class T>
Setup<T>
meshAndQueries(Perf::Case& a_case, EBGeometry::Pool& a_pool)
{
  const Mesh<T> mesh = EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(Perf::meshFile(a_case.smoke()), a_pool);

  a_case.param("K", K);
  a_case.param("W", W);

  return Setup<T>{mesh,
                  Perf::pointsInBox<T>(
                    mesh.computeBoundingVolume(), a_case.size(MeshQueries, SmokeQueries), Perf::Seed::MeshQueries)};
}

// The host's distances in an order, on hostThreads() threads, a block-sized chunk of the order at a time, as
// measureHostAndDevice() divides its host side.
template <class T>
void
hostInOrder(const Distance<T>&                a_query,
            const std::vector<Vec3<T>>&       a_in,
            std::vector<T>&                   a_out,
            const std::vector<std::uint32_t>& a_order)
{
  constexpr std::size_t chunk   = EBGeometry::GPU::DefaultBlockSize;
  const std::size_t     n       = a_in.size();
  const auto            chunks  = static_cast<std::int64_t>((n + chunk - 1) / chunk);
  const int             threads = Perf::hostThreads();

  (void)threads;

#ifdef _OPENMP
#pragma omp parallel for num_threads(threads) schedule(dynamic)
#endif
  for (std::int64_t k = 0; k < chunks; k++) {
    const std::size_t begin = static_cast<std::size_t>(k) * chunk;
    const std::size_t end   = std::min(begin + chunk, n);

    for (std::size_t i = begin; i < end; i++) {
      const std::uint32_t o = a_order[i];

      a_out[o] = a_query(a_in[o]);
    }
  }
}

template <class T>
void
addCases(Perf::Suite& a_suite)
{
  const std::string prec = Perf::precisionName<T>();

  const std::string arrived = "armadillo/distance-arrived";
  const std::string ordered = "armadillo/distance-ordered";

  const auto randomPoints = [](Perf::Case& c) {
    const std::size_t n = c.size(OrderPoints, SmokeOrder);

    c.param("points", n);

    return Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), n, Perf::Seed::Cloud);
  };

  // ── Host: the order, and the distances as drawn and in the order ──

  a_suite.add(Perf::caseName({"QueryOrder", "random/mortonOrder", prec, "host"}), [=](Perf::Case& c) {
    const auto points = randomPoints(c);
    const auto order  = orderOn<T>(
      EBGeometry::hostMemoryResource(), points, [&](const auto& a_body) { c.measure(a_body, points.size()); });

    c.checksum(static_cast<double>(order.size()));
    c.indexChecksum(orderChecksum(order));
  });

  a_suite.add(Perf::caseName({"QueryOrder", arrived, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const Setup<T>   setup  = meshAndQueries<T>(c, pool);
    const Mesh<T>&   mesh   = setup.mesh;
    const auto&      points = setup.points;
    std::vector<T>   out;

    Perf::measureHostQuery(c, Distance<T>{mesh}, points, out);

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());
  });

  a_suite.add(Perf::caseName({"QueryOrder", ordered, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool                 pool(EBGeometry::hostMemoryResource());
    const Setup<T>                   setup  = meshAndQueries<T>(c, pool);
    const Mesh<T>&                   mesh   = setup.mesh;
    const auto&                      points = setup.points;
    const auto                       n      = static_cast<std::uint32_t>(points.size());
    const std::vector<std::uint32_t> order =
      orderOn<T>(EBGeometry::hostMemoryResource(), points, [](const auto& a_body) { a_body(); });
    std::vector<T> out(n);

    c.measure(
      [&] {
        EBGeometry::GPU::transform(Distance<T>{mesh},
                                   EBGeometry::PODSpan<const Vec3<T>>{points.data(), n},
                                   EBGeometry::PODSpan<T>{out.data(), n},
                                   EBGeometry::PODSpan<const std::uint32_t>{order.data(), n});
      },
      n);

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());
    c.relativeTo(Perf::caseName({"QueryOrder", arrived, prec, "host"}), "arrived/ordered");
  });

  // ── Host/device pairs: the order where the pool is, and the distances as drawn and in the order ──

  a_suite.add(Perf::caseName({"QueryOrder", "random/mortonOrder", prec, "hostdevice"}), [=](Perf::Case& c) {
    const auto points = randomPoints(c);
    const auto device = orderOn<T>(
      EBGeometry::GPU::memoryResource(), points, [&](const auto& a_body) { c.measure(a_body, points.size()); });

    std::vector<double> hostTimes;

    const auto host =
      orderOn<T>(EBGeometry::hostMemoryResource(), points, [&](const auto& a_body) { hostTimes = c.repeat(a_body); });

    double mismatches = 0.0;

    for (std::size_t i = 0; i < host.size(); i++) {
      mismatches += (host[i] == device[i]) ? 0.0 : 1.0;
    }

    const double hostMs   = Perf::median(hostTimes);
    const double deviceMs = c.summary().median;

    c.metric(Perf::Key::HostMs, hostMs);
    c.metric(Perf::Key::HostThreads, 1.0);
    c.param("blockSize", EBGeometry::GPU::DefaultBlockSize);
    c.param("minResidentThreads", EBGeometry::GPU::DefaultMinResidentThreads);
    c.metric(Perf::Key::HostOverDevice, deviceMs > 0.0 ? hostMs / deviceMs : 0.0);
    c.metric(Perf::Key::MaxDiff, 0.0);
    c.metric(Perf::Key::Mismatches, mismatches);
    c.checksum(static_cast<double>(host.size()));
    c.indexChecksum(orderChecksum(host));
  });

  a_suite.add(Perf::caseName({"QueryOrder", arrived, prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const Setup<T>   setup  = meshAndQueries<T>(c, pool);
    const Mesh<T>&   mesh   = setup.mesh;
    const auto&      points = setup.points;
    const auto       out    = Perf::measureHostAndDevice<T>(
      c,
      pool,
      points,
      [mesh](const EBGeometry::Pool& a_on) { return Distance<T>{mesh.rebasedView(a_on)}; },
      [](T a_host, T a_device) { return Perf::agreeDistance<T>(a_host, a_device); });

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());
  });

  a_suite.add(Perf::caseName({"QueryOrder", ordered, prec, "hostdevice"}), [=](Perf::Case& c) {
    using EBGeometry::PODVector;
    using EBGeometry::Pool;

    Pool pool(EBGeometry::hostMemoryResource());

    const Setup<T>                   setup  = meshAndQueries<T>(c, pool);
    const Mesh<T>&                   mesh   = setup.mesh;
    const auto&                      points = setup.points;
    const auto                       n      = static_cast<std::uint32_t>(points.size());
    const std::vector<std::uint32_t> order =
      orderOn<T>(EBGeometry::hostMemoryResource(), points, [](const auto& a_body) { a_body(); });

    // The device: the queries, room for the results and the order in one pool, mirrored into device memory, as
    // measureOnDevice() lays them out.
    Pool                     io(EBGeometry::hostMemoryResource());
    PODVector<Vec3<T>>       in;
    PODVector<T>             out;
    PODVector<std::uint32_t> stored;
    const std::vector<T>     zeros(n, T(0));

    in.reserveFrom(io, n);
    out.reserveFrom(io, n);
    stored.reserveFrom(io, n);
    in.assign(io.base(), points.data(), n);
    out.assign(io.base(), zeros.data(), n);
    stored.assign(io.base(), order.data(), n);
    io.freeze();
    pool.freeze();

    const Pool deviceMesh = Pool::mirror(pool, EBGeometry::GPU::memoryResource());
    const Pool deviceIO   = Pool::mirror(io, EBGeometry::GPU::memoryResource());
    const auto query      = Distance<T>{mesh.rebasedView(deviceMesh)};

    c.measure([&] { EBGeometry::GPU::transform(query, deviceIO, in, out, stored); }, n);
    c.param("blockSize", EBGeometry::GPU::DefaultBlockSize);
    c.param("minResidentThreads", EBGeometry::GPU::DefaultMinResidentThreads);

    const Pool     back = Pool::mirror(deviceIO, EBGeometry::hostMemoryResource());
    std::vector<T> device(n);

    for (std::uint32_t i = 0; i < n; i++) {
      device[i] = out.at(back.base(), i);
    }

    // The host: the same order, on every hardware thread but one.
    std::vector<T> host(n);

    const std::vector<double> hostTimes = c.repeat([&] { hostInOrder<T>(Distance<T>{mesh}, points, host, order); });

    double      maxDiff    = 0.0;
    std::size_t mismatches = 0;

    for (std::uint32_t i = 0; i < n; i++) {
      const auto [diff, mismatch] = Perf::agreeDistance<T>(host[i], device[i]);

      maxDiff = std::max(maxDiff, diff);
      mismatches += mismatch ? 1 : 0;
    }

    const double hostMs   = Perf::median(hostTimes);
    const double deviceMs = c.summary().median;

    c.metric(Perf::Key::HostMs, hostMs);
    c.metric(Perf::Key::HostThreads, static_cast<double>(Perf::hostThreads()));
    c.metric(Perf::Key::HostOverDevice, deviceMs > 0.0 ? hostMs / deviceMs : 0.0);
    c.metric(Perf::Key::MaxDiff, maxDiff);
    c.metric(Perf::Key::Mismatches, static_cast<double>(mismatches));
    c.checksum(Perf::checksumOf(host));
    c.poolBytes(pool.usedBytes());
    c.relativeTo(Perf::caseName({"QueryOrder", arrived, prec, "hostdevice"}), "arrived/ordered");
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "QueryOrder");

  addCases<float>(suite);
  addCases<double>(suite);

  return suite.run();
}
