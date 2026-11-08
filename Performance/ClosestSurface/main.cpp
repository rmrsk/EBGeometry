// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for closestSurface(), the surface that decides a tape's value at a point, drilled into a
// collection leaf: a box with a cloud of spheres cut out of it, the cloud a SphereCloudBVH or a BVHUnion of the
// same spheres, in float and double, at random points in the box. Each composition also has a closestLeaf()
// case, which stops at the cloud, and names the closestSurface() case (Case::relativeTo()), so that a
// comparison notes what the drilling costs. A closestSurface() query returns the surface and the deciding
// sphere's own distance at the local point (the box's value for the box), so that the checksum depends on both;
// a closestLeaf() query the leaf alone. A host
// build runs the host cases; a CUDA or HIP build runs them as host/device pairs, which agree when they name
// the same surface or two whose values tie. In a probe of its own, so that adding it changes no other probe's
// translation unit (see Performance/Fused). See ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The spheres' BVHs.
constexpr std::size_t K              = 4;
constexpr std::size_t W              = 4;
constexpr std::size_t TargetLeafSize = 16 * W;

// Spheres in the cloud, and queries per case, at full and at smoke size.
constexpr std::size_t CloudSpheres      = 65536;
constexpr std::size_t SmokeCloudSpheres = 1000;
constexpr std::size_t HostQueries       = 200000;
constexpr std::size_t PairQueries       = 1048576;
constexpr std::size_t SmokeQueries      = 1000;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using Cloud = EBGeometry::SphereCloudBVH<T, K, W>;

template <class T>
using Members = EBGeometry::BVHUnion<T, K, EBGeometry::SphereSDF<T>>;

template <class T>
using Sphere = EBGeometry::SphereCloud::Sphere<T>;

// A query's answer: the surface, and the deciding shape's own value.
template <class T>
struct SurfaceHit
{
  uint32_t m_surface;
  T        m_value;
};

// The surface that decides a tape's value at a point, with the deciding sphere's distance at the local point, read
// from the cloud of the same spheres (the tape's value where the box decides); or the leaf alone, with
// closestLeaf(), and no value, so that the two differ by the drilling only.
template <class T, class TapeType, bool Drill>
struct Query
{
  TapeType m_tape;
  Cloud<T> m_cloud;

  EBGEOMETRY_HOST_DEVICE
  SurfaceHit<T>
  operator()(const Vec3<T>& a_point) const noexcept
  {
    if constexpr (Drill) {
      const EBGeometry::SurfaceRef<T> s = m_tape.closestSurface(a_point);

      if (s.m_leaf == 1U) {
        const Sphere<T> sphere = m_cloud.sphere(s.m_surface - m_tape.getFirstSurface(1));

        return SurfaceHit<T>{s.m_surface, (s.m_localPoint - sphere.m_center).length() - sphere.m_radius};
      }

      return SurfaceHit<T>{s.m_surface, m_tape.signedDistance(a_point)};
    }
    else {
      return SurfaceHit<T>{m_tape.closestLeaf(a_point).m_leaf, T(0)};
    }
  }
};

// The sum of the surfaces and the values.
template <class T>
double
checksumOf(const std::vector<SurfaceHit<T>>& a_hits)
{
  double sum = 0.0;

  for (const auto& h : a_hits) {
    sum += static_cast<double>(h.m_surface) + static_cast<double>(h.m_value);
  }

  return sum;
}

// A host and a device answer agree when their values are within Perf::agreementTolerance(): the same surface, or
// two whose values tie.
template <class T>
std::pair<double, bool>
agreeSurface(const SurfaceHit<T>& a_host, const SurfaceHit<T>& a_device)
{
  const double vh   = static_cast<double>(a_host.m_value);
  const double diff = std::abs(vh - static_cast<double>(a_device.m_value));

  return {diff, diff > Perf::agreementTolerance<T>(std::abs(vh))};
}

// The spheres, in the unit cube, of radius 0.002 to 0.006 times the cloud's size factor.
template <class T>
std::vector<Sphere<T>>
spheres(const std::size_t a_n)
{
  const T    scale   = T(std::cbrt(double(CloudSpheres) / double(a_n)));
  const auto centers = Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), a_n, Perf::Seed::Cloud);
  const auto radii   = Perf::pointsInBox<T>(Perf::cube(T(0.002), T(0.006)), a_n, Perf::Seed::SphereRadii);

  std::vector<Sphere<T>> result(a_n);

  for (std::size_t i = 0; i < a_n; i++) {
    result[i] = Sphere<T>{centers[i], scale * radii[i][0]};
  }

  return result;
}

// The box the spheres are cut out of.
template <class T>
EBGeometry::BoxSDF<T>
box()
{
  return EBGeometry::BoxSDF<T>(Vec3<T>(T(-0.05), T(-0.05), T(-0.05)), Vec3<T>(T(1.05), T(1.05), T(1.05)));
}

// The cases for one composition, closestSurface() and closestLeaf(), each on the host and as a host/device
// pair. a_build(case, pool, spheres) returns the tape.
template <class T, class Build>
void
addComposition(Perf::Suite& a_suite, const std::string& a_input, Build a_build)
{
  using TapeType = std::invoke_result_t<Build, Perf::Case&, EBGeometry::Pool&, const std::vector<Sphere<T>>&>;

  const std::string prec = Perf::precisionName<T>();

  const auto addOne = [&](const auto a_drill) {
    constexpr bool    drill     = decltype(a_drill)::value;
    const std::string operation = drill ? "closestSurface" : "closestLeaf";
    using Q                     = Query<T, TapeType, drill>;

    a_suite.add(Perf::caseName({"ClosestSurface", a_input, operation, prec, "host"}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const auto all    = spheres<T>(c.size(CloudSpheres, SmokeCloudSpheres));
      const auto cloud  = Cloud<T>::build(pool, all, TargetLeafSize);
      const auto tape   = a_build(c, pool, all);
      const auto points = Perf::pointsInBox<T>(
        box<T>().computeBoundingVolume(), c.size(HostQueries, SmokeQueries), Perf::Seed::TapeQueries);

      std::vector<SurfaceHit<T>> hits;

      Perf::measureHostQuery(c, Q{tape, cloud}, points, hits);

      c.checksum(checksumOf(hits));
      c.poolBytes(pool.usedBytes());
      c.param("spheres", all.size());

      if constexpr (!drill) {
        c.relativeTo(Perf::caseName({"ClosestSurface", a_input, "closestSurface", prec, "host"}), "surface/leaf");
      }
    });

    a_suite.add(Perf::caseName({"ClosestSurface", a_input, operation, prec, "hostdevice"}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const auto all    = spheres<T>(c.size(CloudSpheres, SmokeCloudSpheres));
      const auto cloud  = Cloud<T>::build(pool, all, TargetLeafSize);
      const auto tape   = a_build(c, pool, all);
      const auto points = Perf::pointsInBox<T>(
        box<T>().computeBoundingVolume(), c.size(PairQueries, SmokeQueries), Perf::Seed::TapeQueries);

      const auto hits = Perf::measureHostAndDevice<SurfaceHit<T>>(
        c,
        pool,
        points,
        [&](const EBGeometry::Pool& a_pool) { return Q{tape.rebasedView(a_pool), cloud.rebasedView(a_pool)}; },
        agreeSurface<T>);

      c.checksum(checksumOf(hits));
      c.param("spheres", all.size());

      if constexpr (!drill) {
        c.relativeTo(Perf::caseName({"ClosestSurface", a_input, "closestSurface", prec, "hostdevice"}), "surface/leaf");
      }
    });
  };

  addOne(std::true_type{});
  addOne(std::false_type{});
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  // The box with the cloud cut out of it, the cloud a SphereCloudBVH: one leaf drilled by closestSphere().
  addComposition<T>(
    a_suite, "cloud", [](Perf::Case& c, EBGeometry::Pool& a_pool, const std::vector<Sphere<T>>& a_spheres) {
      const auto cloud = Cloud<T>::build(a_pool, a_spheres, TargetLeafSize);

      c.param("K", K);
      c.param("W", W);

      return EBGeometry::Tape<T, Cloud<T>>(a_pool, EBGeometry::Difference(EBGeometry::Expr<T>(box<T>()), cloud));
    });

  // The same, the cloud a BVHUnion of the spheres as analytic shapes: drilled by the union's own search.
  addComposition<T>(
    a_suite, "union", [](Perf::Case& c, EBGeometry::Pool& a_pool, const std::vector<Sphere<T>>& a_spheres) {
      std::vector<EBGeometry::SphereSDF<T>> shapes;

      for (const auto& s : a_spheres) {
        shapes.emplace_back(s.m_center, s.m_radius);
      }

      const Members<T> members(a_pool, shapes);

      c.param("K", K);

      return EBGeometry::Tape<T, Members<T>>(a_pool, EBGeometry::Difference(EBGeometry::Expr<T>(box<T>()), members));
    });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "ClosestSurface");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
