// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for reconciling a DCEL mesh whose vertices moved: MeshT::reconcile(), and
// DCEL::reconcile() from a schedule, in host memory and, as a host/device pair, in device memory, for the
// armadillo mesh in float and double. See ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The reconciles a repetition does: one takes tens of milliseconds with MeshT::reconcile() and under one on a
// device.
constexpr std::size_t Reconciles = 4;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using Mesh = EBGeometry::DCEL::MeshT<T>;

// Moves every vertex: a stretch, so that every normal, centroid and area changes.
template <class T>
struct Stretch
{
  mutable Mesh<T> m_mesh;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    auto&         vertex = m_mesh.getVertex(static_cast<std::uint32_t>(a_i));
    const Vec3<T> x      = vertex.getPosition();

    vertex.setPosition(Vec3<T>(T(1.25) * x[0], T(0.75) * x[1], T(1.5) * x[2]));
  }
};

// The mesh, read and stretched, with its schedule, in one frozen host pool.
template <class T>
struct Setup
{
  EBGeometry::Pool                    pool;
  Mesh<T>                             mesh;
  EBGeometry::DCEL::ReconcileSchedule schedule;

  explicit Setup(const std::string& a_file)
    : pool(EBGeometry::hostMemoryResource()),
      mesh(EBGeometry::Parser::readIntoDCEL<T>(a_file, pool)),
      schedule(pool, mesh)
  {
    pool.freeze();

    const Stretch<T> stretch{mesh};

    for (std::uint32_t v = 0; v < mesh.numVertices(); v++) {
      stretch(v);
    }
  }
};

// What a reconcile wrote, read on the host: every vertex normal and every face's area.
template <class T>
std::vector<T>
results(const Mesh<T>& a_mesh)
{
  std::vector<T> values;

  for (std::uint32_t v = 0; v < a_mesh.numVertices(); v++) {
    for (int d = 0; d < 3; d++) {
      values.push_back(a_mesh.getVertex(v).getNormal()[d]);
    }
  }

  for (std::uint32_t f = 0; f < a_mesh.numFaces(); f++) {
    values.push_back(a_mesh.getFace(f).getArea());
  }

  return values;
}

template <class T>
void
addCases(Perf::Suite& a_suite)
{
  const std::string prefix = "MeshReconcile/armadillo";
  const std::string prec   = Perf::precisionName<T>();

  const std::string plain = Perf::caseName({prefix, "reconcile", prec, "host"});

  // ── Host: MeshT::reconcile(), which checks the topology and builds the vertex-to-face lists on every call ──

  a_suite.add(plain, [=](Perf::Case& c) {
    Setup<T> setup(Perf::meshFile(c.smoke()));

    c.poolBytes(setup.pool.usedBytes());
    c.param("reconciles", Reconciles);

    c.measure([&] {
      for (std::size_t r = 0; r < Reconciles; r++) {
        setup.mesh.reconcile();
      }
    });

    c.checksum(Perf::checksumOf(results(setup.mesh)));
  });

  // ── Host: DCEL::reconcile() from the schedule, in host loops ──

  a_suite.add(Perf::caseName({prefix, "reconcile-scheduled", prec, "host"}), [=](Perf::Case& c) {
    Setup<T> setup(Perf::meshFile(c.smoke()));

    c.poolBytes(setup.pool.usedBytes());
    c.param("reconciles", Reconciles);
    c.relativeTo(plain, "reconcile/scheduled");

    c.measure([&] {
      for (std::size_t r = 0; r < Reconciles; r++) {
        EBGeometry::DCEL::reconcile(setup.pool, setup.mesh, setup.schedule);
      }
    });

    c.checksum(Perf::checksumOf(results(setup.mesh)));
  });

  // ── Host/device pair: the passes in device memory, the host loops in host memory ──

  a_suite.add(Perf::caseName({prefix, "reconcile-scheduled", prec, "hostdevice"}), [=](Perf::Case& c) {
    Setup<T> setup(Perf::meshFile(c.smoke()));

    EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(setup.pool, EBGeometry::GPU::memoryResource());

    c.poolBytes(devicePool.usedBytes());
    c.param("reconciles", Reconciles);

    c.measure([&] {
      for (std::size_t r = 0; r < Reconciles; r++) {
        EBGeometry::DCEL::reconcile(devicePool, setup.mesh, setup.schedule);
      }
    });

    const std::vector<double> hostTimes = c.repeat([&] {
      for (std::size_t r = 0; r < Reconciles; r++) {
        EBGeometry::DCEL::reconcile(setup.pool, setup.mesh, setup.schedule);
      }
    });

    const EBGeometry::Pool back   = EBGeometry::Pool::mirror(devicePool, EBGeometry::hostMemoryResource());
    const auto             host   = results(setup.mesh);
    const auto             device = results(setup.mesh.rebasedView(back));

    double maxDiff    = 0.0;
    double mismatches = 0.0;

    for (std::size_t i = 0; i < host.size(); i++) {
      // Within the agreement tolerance of the host's value; unlike a distance, a normal component near zero
      // may change sign.
      const double h    = static_cast<double>(host[i]);
      const double diff = std::abs(h - static_cast<double>(device[i]));

      maxDiff = (diff > maxDiff) ? diff : maxDiff;
      mismatches += (diff > Perf::agreementTolerance<T>(std::abs(h))) ? 1.0 : 0.0;
    }

    const double hostMs   = Perf::median(hostTimes);
    const double deviceMs = c.summary().median;

    c.metric(Perf::Key::HostMs, hostMs);
    c.metric(Perf::Key::HostThreads, 1.0);
    c.metric(Perf::Key::HostOverDevice, deviceMs > 0.0 ? hostMs / deviceMs : 0.0);
    c.metric(Perf::Key::MaxDiff, maxDiff);
    c.metric(Perf::Key::Mismatches, mismatches);

    c.checksum(Perf::checksumOf(host));
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "MeshReconcile");

  addCases<float>(suite);
  addCases<double>(suite);

  return suite.run();
}
