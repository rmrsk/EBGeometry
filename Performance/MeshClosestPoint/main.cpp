// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Performance cases for the mesh SDFs' closestPoint() and gradient(): the armadillo (the dodecahedron under
// --smoke) as a TriMeshSDF and a MeshSDF, in float and double, at random points around it. A TriMeshSDF's
// signedDistance() at the same points is the reference the closest-point queries are timed against (their
// ratio, within this binary). A host build runs the host cases; a CUDA or HIP build runs closestPoint() as
// host/device pairs. In a probe of its own, so that adding it changes no other probe's translation unit
// (see Performance/Fused). See ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Queries per case, at full and at smoke size. MeshSDF tests polygon faces one at a time, so it gets fewer.
constexpr std::size_t TriHostQueries  = 100000;
constexpr std::size_t MeshHostQueries = 40000;
constexpr std::size_t PairQueries     = 262144;
constexpr std::size_t SmokeQueries    = 1000;

// The BVH branching factor and the triangle width, spelled out rather than the defaults.
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using Tri = EBGeometry::TriMeshSDF<T, K, W>;

template <class T>
using Mesh = EBGeometry::MeshSDF<T, K>;

// The signed distance at a point: the reference.
template <class T, class SDF>
struct Distance
{
  SDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_point) const noexcept
  {
    return m_sdf.signedDistance(a_point);
  }
};

// The closest point at a point.
template <class T, class SDF>
struct Closest
{
  SDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  EBGeometry::ClosestPoint<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    return m_sdf.closestPoint(a_point);
  }
};

// The gradient at a point.
template <class T, class SDF>
struct Gradient
{
  SDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  Vec3<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    return m_sdf.gradient(a_point);
  }
};

// The sum of every result, so that the checksum depends on all of them.
template <class T>
double
checksumOf(const std::vector<T>& a_values)
{
  double sum = 0.0;

  for (const T v : a_values) {
    sum += static_cast<double>(v);
  }

  return sum;
}

template <class T>
double
checksumOf(const std::vector<EBGeometry::ClosestPoint<T>>& a_hits)
{
  double sum = 0.0;

  for (const auto& h : a_hits) {
    sum += static_cast<double>(h.signedDistance) + static_cast<double>(h.faceId);

    for (std::size_t dir = 0; dir < 3; dir++) {
      sum += static_cast<double>(h.point[dir]) + static_cast<double>(h.normal[dir]);
    }
  }

  return sum;
}

template <class T>
double
checksumOf(const std::vector<Vec3<T>>& a_vectors)
{
  double sum = 0.0;

  for (const auto& v : a_vectors) {
    sum += static_cast<double>(v[0]) + static_cast<double>(v[1]) + static_cast<double>(v[2]);
  }

  return sum;
}

// A host and a device closest point agree when their distances do (Perf::agreeDistance()): at points equally
// near two faces the two sides may name different faces and points, both correct.
template <class T>
std::pair<double, bool>
agreeClosest(const EBGeometry::ClosestPoint<T>& a_host, const EBGeometry::ClosestPoint<T>& a_device)
{
  return Perf::agreeDistance<T>(a_host.signedDistance, a_device.signedDistance);
}

// Random points in a box grown by a quarter of the mesh's size on every side.
template <class T>
std::vector<Vec3<T>>
pointsAround(const EBGeometry::BoundingVolumes::AABBT<T>& a_box, const std::size_t a_n)
{
  const Vec3<T> grow = T(0.25) * (a_box.getHighCorner() - a_box.getLowCorner());

  return Perf::pointsInBox<T>(
    EBGeometry::BoundingVolumes::AABBT<T>(a_box.getLowCorner() - grow, a_box.getHighCorner() + grow),
    a_n,
    Perf::Seed::MeshQueries);
}

// The cases of one mesh SDF: closestPoint() on the host and as a host/device pair. a_read reads the mesh
// into a pool.
template <class T, class SDF, class Read>
void
addClosestPoint(Perf::Suite& a_suite, const std::string& a_input, const std::size_t a_hostQueries, Read a_read)
{
  const std::string prec = Perf::precisionName<T>();
  const std::string ref  = Perf::caseName({"MeshClosestPoint", "tri", "signedDistance", prec, "host"});

  a_suite.add(Perf::caseName({"MeshClosestPoint", a_input, "closestPoint", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const SDF  sdf    = a_read(c, pool);
    const auto points = pointsAround(sdf.computeBoundingVolume(), c.size(a_hostQueries, SmokeQueries));
    std::vector<EBGeometry::ClosestPoint<T>> hits;

    Perf::measureHostQuery(c, Closest<T, SDF>{sdf}, points, hits);

    c.checksum(checksumOf(hits));
    c.poolBytes(pool.usedBytes());

    if (a_hostQueries == TriHostQueries) {
      c.relativeTo(ref, "signedDistance/closestPoint");
    }
  });

  a_suite.add(Perf::caseName({"MeshClosestPoint", a_input, "closestPoint", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const SDF  sdf    = a_read(c, pool);
    const auto points = pointsAround(sdf.computeBoundingVolume(), c.size(PairQueries, SmokeQueries));

    const auto hits = Perf::measureHostAndDevice<EBGeometry::ClosestPoint<T>>(
      c,
      pool,
      points,
      [&](const EBGeometry::Pool& a_pool) { return Closest<T, SDF>{sdf.rebasedView(a_pool)}; },
      agreeClosest<T>);

    c.checksum(checksumOf(hits));
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  const std::string prec = Perf::precisionName<T>();

  const auto readTri = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    c.param("K", K);
    c.param("W", W);

    return EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(Perf::meshFile(c.smoke()), a_pool);
  };

  const auto readMesh = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    c.param("K", K);

    return EBGeometry::Parser::readIntoMeshSDF<T, K>(Perf::meshFile(c.smoke()), a_pool);
  };

  // The reference: signedDistance() at the same points.
  a_suite.add(Perf::caseName({"MeshClosestPoint", "tri", "signedDistance", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Tri<T>   sdf    = readTri(c, pool);
    const auto     points = pointsAround(sdf.computeBoundingVolume(), c.size(TriHostQueries, SmokeQueries));
    std::vector<T> values;

    Perf::measureHostQuery(c, Distance<T, Tri<T>>{sdf}, points, values);

    c.checksum(checksumOf(values));
    c.poolBytes(pool.usedBytes());
  });

  addClosestPoint<T, Tri<T>>(a_suite, "tri", TriHostQueries, readTri);
  addClosestPoint<T, Mesh<T>>(a_suite, "mesh", MeshHostQueries, readMesh);

  // gradient() is closestPoint() and a few operations more; one host case shows that it stays so.
  a_suite.add(Perf::caseName({"MeshClosestPoint", "tri", "gradient", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Tri<T>         sdf    = readTri(c, pool);
    const auto           points = pointsAround(sdf.computeBoundingVolume(), c.size(TriHostQueries, SmokeQueries));
    std::vector<Vec3<T>> gradients;

    Perf::measureHostQuery(c, Gradient<T, Tri<T>>{sdf}, points, gradients);

    c.checksum(checksumOf(gradients));
    c.poolBytes(pool.usedBytes());
    c.relativeTo(Perf::caseName({"MeshClosestPoint", "tri", "closestPoint", prec, "host"}), "closestPoint/gradient");
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "MeshClosestPoint");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
