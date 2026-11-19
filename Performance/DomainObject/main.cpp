// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Performance cases for a domain compiled on its own (regionNeighborhood()): an assembly of many parts in a box of air,
// each part a region, and a domain computing in one part. The whole model's tape evaluates every part; the domain's own
// tape keeps the part and the air around it (the regions whose boxes meet the part's), not the other parts. labels is
// what a cut-cell code samples at a node, every subdomain's function (Domain::signedDistances()); labelAt the subdomain
// of a point alone (Domain::subdomainAt()). In float and double, at random points in a shell around the part, where a
// solver of the domain samples and where the two tapes give the same values; each own case is noted relative to the
// whole tape's. A host build runs the host cases; a CUDA or HIP build runs them as host/device pairs, which agree when
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

// Parts along each axis of the assembly, and queries per case, at full and at smoke size.
constexpr std::size_t PartsPerAxis      = 6;
constexpr std::size_t SmokePartsPerAxis = 2;
constexpr std::size_t HostQueries       = 20000;
constexpr std::size_t PairQueries       = 262144;
constexpr std::size_t SmokeQueries      = 1000;

// The largest number of labels a query's buffer holds: the domain's one subdomain and the excluded regions'.
constexpr std::size_t MaxLabels = 2;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

enum class Operation
{
  Labels,
  LabelAt
};

template <class T>
struct Answer
{
  T m_value;
};

// Every subdomain's function at a point, or its subdomain.
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

      for (uint32_t l = 0; l < m_domain.getNumSubdomainFunctions(); l++) {
        sum += values[l];
      }

      return Answer<T>{sum};
    }
    else {
      return Answer<T>{T(m_domain.subdomainAt(a_point))};
    }
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

// A part's center and radius: a ball in each cell of a grid filling the unit cube, apart from its neighbors.
template <class T>
Vec3<T>
centerOf(const std::size_t a_i, const std::size_t a_j, const std::size_t a_k, const std::size_t a_perAxis)
{
  const T cell = T(1) / T(a_perAxis);

  return Vec3<T>(cell * (T(a_i) + T(0.5)), cell * (T(a_j) + T(0.5)), cell * (T(a_k) + T(0.5)));
}

template <class T>
T
radiusOf(const std::size_t a_perAxis)
{
  return T(0.3) / T(a_perAxis);
}

// The assembly: an Overlay of the air (region 1, the unit cube), then every part (2 on).
template <class T>
EBGeometry::Expr<T>
assembly(const std::size_t a_perAxis)
{
  std::vector<EBGeometry::Expr<T>> operands{
    EBGeometry::BoxSDF<T>(Vec3<T>(T(0), T(0), T(0)), Vec3<T>(T(1), T(1), T(1)))};

  for (std::size_t i = 0; i < a_perAxis; i++) {
    for (std::size_t j = 0; j < a_perAxis; j++) {
      for (std::size_t k = 0; k < a_perAxis; k++) {
        operands.emplace_back(EBGeometry::SphereSDF<T>(centerOf<T>(i, j, k, a_perAxis), radiusOf<T>(a_perAxis)));
      }
    }
  }

  return EBGeometry::Overlay(operands);
}

// The domain's part: the one in the middle cell (the last of the first half along each axis), and its region.
std::size_t
middleOf(const std::size_t a_perAxis)
{
  return (a_perAxis - 1) / 2;
}

uint32_t
partRegion(const std::size_t a_perAxis)
{
  const std::size_t m = middleOf(a_perAxis);

  return static_cast<uint32_t>(2 + (m * a_perAxis + m) * a_perAxis + m);
}

// Points in a shell around the domain's part, out to twice its radius: where its solver samples.
template <class T>
std::vector<Vec3<T>>
pointsNearPart(const std::size_t a_perAxis, const std::size_t a_n)
{
  const std::size_t m = middleOf(a_perAxis);
  const Vec3<T>     c = centerOf<T>(m, m, m, a_perAxis);
  const T           r = T(2) * radiusOf<T>(a_perAxis);

  return Perf::pointsInBox<T>(
    EBGeometry::BoundingVolumes::AABBT<T>(c - r * Vec3<T>::ones(), c + r * Vec3<T>::ones()), a_n, Perf::Seed::NearPart);
}

// The domain, computing in the part; its surface groups play no part in these queries.
EBGeometry::DomainSetup
setupOf(const std::size_t a_perAxis)
{
  EBGeometry::DomainSetup setup;

  (void)setup.addSubdomain({partRegion(a_perAxis)});

  return setup;
}

const char*
operationName(const Operation a_op)
{
  return (a_op == Operation::Labels) ? "labels" : "labelAt";
}

// The tape a case evaluates: the whole assembly's, or the domain's own; and the domain's setup for it.
template <class T>
std::pair<EBGeometry::Expr<T>, EBGeometry::DomainSetup>
objectOf(const std::size_t a_perAxis, const bool a_own)
{
  const EBGeometry::Expr<T> whole = assembly<T>(a_perAxis);

  if (!a_own) {
    return {whole, setupOf(a_perAxis)};
  }

  const auto neighborhood = EBGeometry::regionNeighborhood(whole, {partRegion(a_perAxis)});

  return {neighborhood.object(), neighborhood.setup(setupOf(a_perAxis))};
}

template <class T, Operation Op>
void
addOperation(Perf::Suite& a_suite, const bool a_own)
{
  const std::string prec  = Perf::precisionName<T>();
  const std::string name  = operationName(Op);
  const std::string input = a_own ? "partsOwn" : "partsWhole";

  for (const std::string side : {"host", "hostdevice"}) {
    a_suite.add(Perf::caseName({"DomainObject", input, name, prec, side}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const std::size_t           perAxis = c.size(PartsPerAxis, SmokePartsPerAxis);
      const auto                  object  = objectOf<T>(perAxis, a_own);
      const EBGeometry::Tape<T>   tape(pool, object.first);
      const EBGeometry::Domain<T> domain(pool, tape, object.second);

      if (side == "host") {
        const auto points = pointsNearPart<T>(perAxis, c.size(HostQueries, SmokeQueries));

        std::vector<Answer<T>> answers;

        Perf::measureHostQuery(c, Query<T, Op>{domain}, points, answers);

        c.checksum(checksumOf(answers));
        c.poolBytes(pool.usedBytes());
      }
      else {
        const auto points  = pointsNearPart<T>(perAxis, c.size(PairQueries, SmokeQueries));
        const auto answers = Perf::measureHostAndDevice<Answer<T>>(
          c,
          pool,
          points,
          [&](const EBGeometry::Pool& a_pool) { return Query<T, Op>{domain.rebasedView(a_pool)}; },
          agreeValue<T>);

        c.checksum(checksumOf(answers));
      }

      c.param("parts", perAxis * perAxis * perAxis);
      c.param("regions", std::size_t(tape.getNumRegions()));

      if (a_own) {
        c.relativeTo(Perf::caseName({"DomainObject", "partsWhole", name, prec, side}), "whole/own");
      }
    });
  }
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  for (const bool own : {false, true}) {
    addOperation<T, Operation::Labels>(a_suite, own);
    addOperation<T, Operation::LabelAt>(a_suite, own);
  }
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "DomainObject");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
