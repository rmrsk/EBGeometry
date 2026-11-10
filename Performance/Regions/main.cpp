// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the regions of a tape: a matrix holding particles, each a shell around a grain, one Overlay
// of 1 + 2 x Particles regions, in float and double, at random points in the matrix. region() finds the region a point
// is in; regionDistances() gives every region's function in one pass, and the regionByRegion case the same values by
// asking signedDistance(point, region) for each (Case::relativeTo(), so that a comparison notes what the one pass
// saves); domainLabels is what a cut-cell code samples at a node, Domain::signedDistances() for a domain of the matrix
// and the shells (the grains excluded). A host build runs the host cases; a CUDA or HIP build runs them as host/device
// pairs, which agree when their values do. In a probe of its own, so that adding it changes no other probe's
// translation unit (see Performance/Fused). See ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Particles in the matrix, and queries per case, at full and at smoke size.
constexpr std::size_t Particles      = 32;
constexpr std::size_t SmokeParticles = 4;
constexpr std::size_t HostQueries    = 20000;
constexpr std::size_t PairQueries    = 262144;
constexpr std::size_t SmokeQueries   = 1000;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// What each case computes at a point.
enum class Operation
{
  Region,
  RegionDistances,
  RegionByRegion,
  DomainLabels
};

// A query's answer: the region a point is in (Region), or the sum of the values computed.
template <class T>
struct RegionHit
{
  uint32_t m_region;
  T        m_value;
};

template <class T, Operation Op>
struct Query
{
  EBGeometry::Tape<T> m_tape;
  EBGeometry::Domain  m_domain;

  EBGEOMETRY_HOST_DEVICE
  RegionHit<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    if constexpr (Op == Operation::Region) {
      return RegionHit<T>{m_tape.region(a_point), T(0)};
    }
    else if constexpr (Op == Operation::RegionDistances) {
      T sum = T(0);

      m_tape.regionDistances(a_point, [&](const uint32_t, const T a_value) { sum += a_value; });

      return RegionHit<T>{0U, sum};
    }
    else if constexpr (Op == Operation::RegionByRegion) {
      T sum = T(0);

      for (uint32_t k = 0; k < m_tape.getNumRegions(); k++) {
        sum += m_tape.signedDistance(a_point, k);
      }

      return RegionHit<T>{0U, sum};
    }
    else {
      T values[EBGeometry::Domain::MaxLabels];

      m_domain.signedDistances(m_tape, a_point, values);

      return RegionHit<T>{0U, values[0] + values[1] + values[2]};
    }
  }
};

// The sum of the regions and the values.
template <class T>
double
checksumOf(const std::vector<RegionHit<T>>& a_hits)
{
  double sum = 0.0;

  for (const auto& h : a_hits) {
    sum += static_cast<double>(h.m_region) + static_cast<double>(h.m_value);
  }

  return sum;
}

// A host and a device answer agree when their values are within Perf::agreementTolerance() (a region number is not
// compared: a point on a surface may fall on either side).
template <class T>
std::pair<double, bool>
agreeValue(const RegionHit<T>& a_host, const RegionHit<T>& a_device)
{
  const double vh   = static_cast<double>(a_host.m_value);
  const double diff = std::abs(vh - static_cast<double>(a_device.m_value));

  return {diff, diff > Perf::agreementTolerance<T>(std::abs(vh))};
}

// The matrix, a unit cube.
template <class T>
EBGeometry::BoxSDF<T>
matrix()
{
  return EBGeometry::BoxSDF<T>(Vec3<T>(T(0), T(0), T(0)), Vec3<T>(T(1), T(1), T(1)));
}

// The matrix with its particles: an Overlay of the matrix, then every particle's shell, then every grain (on top of its
// shell), so that the shells are one range of regions.
template <class T>
EBGeometry::Expr<T>
scene(const std::size_t a_particles)
{
  const auto centers = Perf::pointsInBox<T>(Perf::cube(T(0.1), T(0.9)), a_particles, Perf::Seed::Inclusions);
  const T    radius  = T(0.3) / T(std::cbrt(double(a_particles)));
  std::vector<EBGeometry::Expr<T>> operands{matrix<T>()};

  for (const auto& c : centers) {
    operands.emplace_back(EBGeometry::SphereSDF<T>(c, radius));
  }

  for (const auto& c : centers) {
    operands.emplace_back(EBGeometry::SphereSDF<T>(c, T(0.5) * radius));
  }

  return EBGeometry::Overlay(operands);
}

// The domain: the matrix, and the shells, two subdomains; the grains in none.
inline EBGeometry::Domain
domainOf(const std::size_t a_particles)
{
  EBGeometry::Domain domain;

  (void)domain.m_subdomains.addSubdomain({1U});

  std::vector<uint32_t> shells;

  for (std::size_t i = 0; i < a_particles; i++) {
    shells.push_back(static_cast<uint32_t>(2 + i));
  }

  (void)domain.m_subdomains.addSubdomain(shells);

  return domain;
}

const char*
operationName(const Operation a_op)
{
  switch (a_op) {
  case Operation::Region: {
    return "region";
  }
  case Operation::RegionDistances: {
    return "regionDistances";
  }
  case Operation::RegionByRegion: {
    return "regionByRegion";
  }
  default: {
    return "domainLabels";
  }
  }
}

template <class T, Operation Op>
void
addOperation(Perf::Suite& a_suite)
{
  const std::string prec = Perf::precisionName<T>();
  const std::string name = operationName(Op);

  a_suite.add(Perf::caseName({"Regions", "particles", name, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const std::size_t         particles = c.size(Particles, SmokeParticles);
    const EBGeometry::Tape<T> tape(pool, scene<T>(particles));
    const auto                points = Perf::pointsInBox<T>(
      matrix<T>().computeBoundingVolume(), c.size(HostQueries, SmokeQueries), Perf::Seed::TapeQueries);

    std::vector<RegionHit<T>> hits;

    Perf::measureHostQuery(c, Query<T, Op>{tape, domainOf(particles)}, points, hits);

    c.checksum(checksumOf(hits));
    c.poolBytes(pool.usedBytes());
    c.param("particles", particles);

    if constexpr (Op == Operation::RegionByRegion) {
      c.relativeTo(Perf::caseName({"Regions", "particles", "regionDistances", prec, "host"}), "each/one-pass");
    }
  });

  a_suite.add(Perf::caseName({"Regions", "particles", name, prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const std::size_t         particles = c.size(Particles, SmokeParticles);
    const EBGeometry::Tape<T> tape(pool, scene<T>(particles));
    const EBGeometry::Domain  domain = domainOf(particles);
    const auto                points = Perf::pointsInBox<T>(
      matrix<T>().computeBoundingVolume(), c.size(PairQueries, SmokeQueries), Perf::Seed::TapeQueries);

    const auto hits = Perf::measureHostAndDevice<RegionHit<T>>(
      c,
      pool,
      points,
      [&](const EBGeometry::Pool& a_pool) { return Query<T, Op>{tape.rebasedView(a_pool), domain}; },
      agreeValue<T>);

    c.checksum(checksumOf(hits));
    c.param("particles", particles);

    if constexpr (Op == Operation::RegionByRegion) {
      c.relativeTo(Perf::caseName({"Regions", "particles", "regionDistances", prec, "hostdevice"}), "each/one-pass");
    }
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  addOperation<T, Operation::Region>(a_suite);
  addOperation<T, Operation::RegionDistances>(a_suite);
  addOperation<T, Operation::RegionByRegion>(a_suite);
  addOperation<T, Operation::DomainLabels>(a_suite);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "Regions");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
