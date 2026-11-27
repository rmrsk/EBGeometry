// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Performance cases for a tape that welds its regions (TapeFeature::Weld), in float and double. The particles: the
// Regions probe's matrix holding particles, each a shell around a grain, at random points in the matrix, where almost
// no point is near a weld; region() and regionDistances(), what the feature costs where it does nothing. The blocks: a
// stack of blocks, each a region, a hair-thin gap (narrower than the tolerance) between neighbors, at points within a
// few tolerances of the gaps, where every query reaches the weld; region() and closestSurface(), what a weld costs where
// it acts. A host build runs the host cases; a CUDA or HIP build runs them as host/device pairs, which agree when their
// values do. In a probe of its own, so that adding it changes no other probe's translation unit (see
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

// Particles in the matrix, blocks along each axis, and queries per case, at full and at smoke size.
constexpr std::size_t Particles      = 32;
constexpr std::size_t SmokeParticles = 4;
constexpr std::size_t Blocks         = 4;
constexpr std::size_t SmokeBlocks    = 2;
constexpr std::size_t HostQueries    = 20000;
constexpr std::size_t PairQueries    = 262144;
constexpr std::size_t SmokeQueries   = 1000;

// The interface tolerance, as a length, and the gaps between the blocks, a fraction of it.
constexpr double Tolerance   = 1e-4;
constexpr double GapFraction = 0.3;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using WeldTape = EBGeometry::Tape<T, EBGeometry::TapeFeature::Weld>;

// What each case computes at a point.
enum class Operation
{
  Region,
  RegionDistances,
  ClosestSurface
};

// A query's answer: a region or a surface, and a value.
template <class T>
struct Answer
{
  uint32_t m_index;
  T        m_value;
};

template <class T, Operation Op>
struct Query
{
  WeldTape<T> m_tape;

  EBGEOMETRY_HOST_DEVICE
  Answer<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    if constexpr (Op == Operation::Region) {
      return Answer<T>{m_tape.region(a_point), T(0)};
    }
    else if constexpr (Op == Operation::RegionDistances) {
      T sum = T(0);

      m_tape.regionDistances(a_point, [&](const uint32_t, const T a_value) { sum += a_value; });

      return Answer<T>{0U, sum};
    }
    else {
      const uint32_t                  region  = m_tape.region(a_point);
      const EBGeometry::SurfaceRef<T> surface = m_tape.closestSurface(a_point, region);

      return Answer<T>{surface.m_surface + 1000U * surface.m_neighborRegion, T(0)};
    }
  }
};

// The sum of the indices and the values.
template <class T>
double
checksumOf(const std::vector<Answer<T>>& a_answers)
{
  double sum = 0.0;

  for (const auto& a : a_answers) {
    sum += static_cast<double>(a.m_index) + static_cast<double>(a.m_value);
  }

  return sum;
}

// A host and a device answer agree when their values are within Perf::agreementTolerance() (an index is not compared:
// a point on a surface, or at a tie, may fall on either side).
template <class T>
std::pair<double, bool>
agreeValue(const Answer<T>& a_host, const Answer<T>& a_device)
{
  const double vh   = static_cast<double>(a_host.m_value);
  const double diff = std::abs(vh - static_cast<double>(a_device.m_value));

  return {diff, diff > Perf::agreementTolerance<T>(std::abs(vh))};
}

// The matrix with its particles, as in the Regions probe.
template <class T>
EBGeometry::Expr<T>
particles(const std::size_t a_particles)
{
  const auto centers = Perf::pointsInBox<T>(Perf::cube(T(0.1), T(0.9)), a_particles, Perf::Seed::Inclusions);
  const T    radius  = T(0.3) / T(std::cbrt(double(a_particles)));

  std::vector<EBGeometry::Expr<T>> operands{EBGeometry::BoxSDF<T>(Vec3<T>::zeros(), Vec3<T>::ones())};

  for (const auto& c : centers) {
    operands.emplace_back(EBGeometry::SphereSDF<T>(c, radius));
  }

  for (const auto& c : centers) {
    operands.emplace_back(EBGeometry::SphereSDF<T>(c, T(0.5) * radius));
  }

  return EBGeometry::Overlay(operands);
}

// The pitch of the blocks: a unit block and the gap after it.
template <class T>
T
pitch()
{
  return T(1) + T(GapFraction * Tolerance);
}

// A stack of n x n x n unit blocks, each a region, with a gap of GapFraction tolerances between neighbors.
template <class T>
EBGeometry::Expr<T>
blocks(const std::size_t a_n)
{
  std::vector<EBGeometry::Expr<T>> operands;

  for (std::size_t i = 0; i < a_n; i++) {
    for (std::size_t j = 0; j < a_n; j++) {
      for (std::size_t k = 0; k < a_n; k++) {
        const Vec3<T> lo = pitch<T>() * Vec3<T>(T(i), T(j), T(k));

        operands.emplace_back(EBGeometry::BoxSDF<T>(lo, lo + Vec3<T>::ones()));
      }
    }
  }

  return EBGeometry::Overlay(operands);
}

// Points within a few tolerances of the gaps: random points of the stack, each moved across x onto the middle of the
// nearest gap between two blocks, then off it by up to two tolerances.
template <class T>
std::vector<Vec3<T>>
nearGaps(const std::size_t a_n, const std::size_t a_points)
{
  const T extent = pitch<T>() * T(a_n);

  std::vector<Vec3<T>> points = Perf::pointsInBox<T>(Perf::cube(T(0), extent), a_points, Perf::Seed::TapeQueries);

  const std::vector<Vec3<T>> offsets = Perf::pointsInBox<T>(Perf::cube(T(-2), T(2)), a_points, Perf::Seed::NearPart);

  for (std::size_t i = 0; i < points.size(); i++) {
    const T gap = EBGeometry::Math::clamp(std::floor(points[i][0] / pitch<T>()), T(1), T(a_n - 1));

    points[i][0] = gap * pitch<T>() - T(0.5 * GapFraction * Tolerance) + offsets[i][0] * T(Tolerance);
  }

  return points;
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
  default: {
    return "closestSurface";
  }
  }
}

template <class T, Operation Op, bool Blocks>
void
addCase(Perf::Suite& a_suite)
{
  const std::string prec  = Perf::precisionName<T>();
  const std::string name  = operationName(Op);
  const std::string input = Blocks ? "blocks" : "particles";

  const auto build = [](EBGeometry::Pool& a_pool, Perf::Case& a_case) {
    EBGeometry::TapeOptions options;

    options.m_interfaceLength = Tolerance;

    if constexpr (Blocks) {
      return WeldTape<T>(a_pool, blocks<T>(a_case.size(Blocks, SmokeBlocks)), options);
    }
    else {
      return WeldTape<T>(a_pool, particles<T>(a_case.size(Particles, SmokeParticles)), options);
    }
  };

  const auto inputs = [](Perf::Case& a_case, const std::size_t a_n) {
    if constexpr (Blocks) {
      return nearGaps<T>(a_case.size(Blocks, SmokeBlocks), a_n);
    }
    else {
      return Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), a_n, Perf::Seed::TapeQueries);
    }
  };

  a_suite.add(Perf::caseName({"Weld", input, name, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool  pool(EBGeometry::hostMemoryResource());
    const WeldTape<T> tape   = build(pool, c);
    const auto        points = inputs(c, c.size(HostQueries, SmokeQueries));

    std::vector<Answer<T>> answers;

    Perf::measureHostQuery(c, Query<T, Op>{tape}, points, answers);

    c.checksum(checksumOf(answers));
    c.poolBytes(pool.usedBytes());
    c.param("regions", std::size_t(tape.getNumRegions()));
  });

  a_suite.add(Perf::caseName({"Weld", input, name, prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool  pool(EBGeometry::hostMemoryResource());
    const WeldTape<T> tape   = build(pool, c);
    const auto        points = inputs(c, c.size(PairQueries, SmokeQueries));

    const auto answers = Perf::measureHostAndDevice<Answer<T>>(
      c,
      pool,
      points,
      [&](const EBGeometry::Pool& a_pool) { return Query<T, Op>{tape.rebasedView(a_pool)}; },
      agreeValue<T>);

    c.checksum(checksumOf(answers));
    c.param("regions", std::size_t(tape.getNumRegions()));
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  addCase<T, Operation::Region, false>(a_suite);
  addCase<T, Operation::RegionDistances, false>(a_suite);
  addCase<T, Operation::Region, true>(a_suite);
  addCase<T, Operation::ClosestSurface, true>(a_suite);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "Weld");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
