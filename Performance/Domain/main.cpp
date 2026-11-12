// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for a domain with many subdomains and surface groups: a matrix holding particles, each a shell
// around a grain (the Regions probe's scene), with the matrix and every shell a subdomain of its own (33 subdomains,
// one region each, the grains excluded), and 8 surface groups of 33 selectors. labels is what a cut-cell code samples
// at a node, Domain::signedDistances(); labelAt the label of a point alone; surfaceGroups the surface group of a surface
// already found (Domain::surfaceGroupOf()), every selector tried for most. In float and double, at random points in
// the matrix. A host build runs the host cases; a CUDA or HIP build runs them as host/device pairs, which agree when
// their values do. In a probe of its own, so that adding it changes no other probe's translation unit (see
// Performance/Fused). See ../README.md and PerformanceTesting.rst.

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

// The surface groups the selectors are dealt into.
constexpr uint32_t SurfaceGroups = 8;

// The largest number of labels a query's buffer holds: the matrix's, every shell's, and the excluded regions'.
constexpr std::size_t MaxLabels = Particles + 2;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

enum class Operation
{
  Labels,
  LabelAt,
  SurfaceGroups
};

template <class T>
struct Answer
{
  T m_value;
};

// A point's labels or label; a surface's group.
template <class T, Operation Op>
struct Query
{
  EBGeometry::Domain<T> m_domain;

  EBGEOMETRY_HOST_DEVICE
  Answer<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    if constexpr (Op == Operation::Labels) {
      T values[MaxLabels];

      m_domain.signedDistances(a_point, values);

      T sum = T(0);

      for (uint32_t l = 0; l < m_domain.getNumLabels(); l++) {
        sum += values[l];
      }

      return Answer<T>{sum};
    }
    else {
      return Answer<T>{T(m_domain.labelAt(a_point))};
    }
  }

  EBGEOMETRY_HOST_DEVICE
  Answer<T>
  operator()(const EBGeometry::SurfaceRef<T>& a_surface) const noexcept
  {
    return Answer<T>{T(m_domain.surfaceGroupOf(a_surface))};
  }
};

template <class T>
double
checksumOf(const std::vector<Answer<T>>& a_answers)
{
  double sum = 0.0;

  for (const auto& a : a_answers) {
    sum += static_cast<double>(a.m_value);
  }

  return sum;
}

// A host and a device answer agree when their values are within Perf::agreementTolerance().
template <class T>
std::pair<double, bool>
agreeValue(const Answer<T>& a_host, const Answer<T>& a_device)
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

// The matrix with its particles: an Overlay of the matrix (region 1), then every particle's shell (2 to 1 + n), then
// every grain (on top of its shell).
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

// The domain: the matrix and every shell a subdomain of its own; the grains in none. Its surface groups: the interface
// of each shell with its grain, and the remaining surfaces, dealt into the surface groups in turn, so that a surface
// facing the matrix tries every selector before the remaining surfaces hold it.
EBGeometry::DomainSetup
setupOf(const std::size_t a_particles)
{
  EBGeometry::DomainSetup setup;

  (void)setup.addSubdomain({1U});

  for (std::size_t i = 0; i < a_particles; i++) {
    (void)setup.addSubdomain({static_cast<uint32_t>(2 + i)});
  }

  for (uint32_t g = 0; g < SurfaceGroups; g++) {
    (void)setup.addSurfaceGroup();
  }

  const uint32_t n = static_cast<uint32_t>(a_particles);

  for (uint32_t i = 0; i < n; i++) {
    setup.add(i % SurfaceGroups, EBGeometry::SurfaceSelector::interface(2U + i, 2U + n + i));
  }

  setup.add(SurfaceGroups - 1U, EBGeometry::SurfaceSelector::rest());

  return setup;
}

const char*
operationName(const Operation a_op)
{
  switch (a_op) {
  case Operation::Labels: {
    return "labels";
  }
  case Operation::LabelAt: {
    return "labelAt";
  }
  default: {
    return "surfaceGroups";
  }
  }
}

// The points of a case, or for SurfaceGroups the surfaces of their regions closest to them (found untimed).
template <class T, Operation Op>
auto
inputsOf(const EBGeometry::Tape<T>& a_tape, const std::vector<Vec3<T>>& a_points)
{
  if constexpr (Op == Operation::SurfaceGroups) {
    std::vector<EBGeometry::SurfaceRef<T>> surfaces;

    surfaces.reserve(a_points.size());

    for (const auto& p : a_points) {
      surfaces.push_back(a_tape.closestSurface(p, a_tape.region(p)));
    }

    return surfaces;
  }
  else {
    return a_points;
  }
}

template <class T, Operation Op>
void
addOperation(Perf::Suite& a_suite)
{
  const std::string prec = Perf::precisionName<T>();
  const std::string name = operationName(Op);

  a_suite.add(Perf::caseName({"Domain", "particles", name, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const std::size_t           particles = c.size(Particles, SmokeParticles);
    const EBGeometry::Tape<T>   tape(pool, scene<T>(particles));
    const EBGeometry::Domain<T> domain(pool, tape, setupOf(particles));
    const auto                  inputs = inputsOf<T, Op>(tape,
                                        Perf::pointsInBox<T>(matrix<T>().computeBoundingVolume(),
                                                             c.size(HostQueries, SmokeQueries),
                                                             Perf::Seed::TapeQueries));

    std::vector<Answer<T>> answers;

    Perf::measureHostQuery(c, Query<T, Op>{domain}, inputs, answers);

    c.checksum(checksumOf(answers));
    c.poolBytes(pool.usedBytes());
    c.param("particles", particles);
    c.param("surfaceGroups", std::size_t(SurfaceGroups));
  });

  a_suite.add(Perf::caseName({"Domain", "particles", name, prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const std::size_t           particles = c.size(Particles, SmokeParticles);
    const EBGeometry::Tape<T>   tape(pool, scene<T>(particles));
    const EBGeometry::Domain<T> domain(pool, tape, setupOf(particles));
    const auto                  inputs = inputsOf<T, Op>(tape,
                                        Perf::pointsInBox<T>(matrix<T>().computeBoundingVolume(),
                                                             c.size(PairQueries, SmokeQueries),
                                                             Perf::Seed::TapeQueries));

    const auto answers = Perf::measureHostAndDevice<Answer<T>>(
      c,
      pool,
      inputs,
      [&](const EBGeometry::Pool& a_pool) { return Query<T, Op>{domain.rebasedView(a_pool)}; },
      agreeValue<T>);

    c.checksum(checksumOf(answers));
    c.param("particles", particles);
    c.param("surfaceGroups", std::size_t(SurfaceGroups));
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  addOperation<T, Operation::Labels>(a_suite);
  addOperation<T, Operation::LabelAt>(a_suite);
  addOperation<T, Operation::SurfaceGroups>(a_suite);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "Domain");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
