// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the analytic shapes EllipsoidSDF to WedgeSDF (EBGeometry_AnalyticDistanceFunctions.hpp),
// in float and double: each shape's signed distance at random points around it, and the first crossing of rays
// through a fused union of the six of them with closed-form ray forms. The ellipsoid's exact distance is found by
// a short iteration, the others in closed form. A host build runs the host cases; a CUDA or HIP build runs them as
// host/device pairs. See ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Queries per case, at full and at smoke size.
constexpr std::size_t HostQueries  = 400000;
constexpr std::size_t PairQueries  = 1048576;
constexpr std::size_t HostRays     = 100000;
constexpr std::size_t PairRays     = 262144;
constexpr std::size_t SmokeQueries = 1000;

// The prism's number of sides, which sets its ray form's loop.
constexpr unsigned PrismSides = 6;

// How far a ray is followed: past the whole scene.
constexpr double RayLength = 8.0;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

// A shape's signed distance at a point.
template <class T, class Shape>
struct Distance
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }
};

// What one ray's query gives: the distance to its first crossing, -1 for none.
template <class T, class Shape>
struct FirstHit
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const EBGeometry::RayT<T>& a_ray) const noexcept
  {
    const EBGeometry::HitT<T> hit = EBGeometry::firstHit(m_shape, a_ray, T(0), T(RayLength));

    return hit.hit() ? hit.m_t : -T(1);
  }
};

// Random points in a box grown by a quarter of its size on every side.
template <class T>
std::vector<Vec3<T>>
pointsAround(const AABB<T>& a_box, const std::size_t a_n)
{
  const Vec3<T> grow = T(0.25) * (a_box.getHighCorner() - a_box.getLowCorner());

  return Perf::pointsInBox<T>(
    AABB<T>(a_box.getLowCorner() - grow, a_box.getHighCorner() + grow), a_n, Perf::Seed::TapeQueries);
}

// The cases for one shape: its signed distance on the host and as a host/device pair, at points around a_box.
template <class T, class Shape>
void
addShape(Perf::Suite& a_suite, const std::string& a_input, const Shape& a_shape, const AABB<T>& a_box)
{
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"Shapes", a_input, "signedDistance", prec, "host"}), [=](Perf::Case& c) {
    const auto     points = pointsAround(a_box, c.size(HostQueries, SmokeQueries));
    std::vector<T> distances;

    Perf::measureHostQuery(c, Distance<T, Shape>{a_shape}, points, distances);

    c.checksum(Perf::checksumOf(distances));
  });

  a_suite.add(Perf::caseName({"Shapes", a_input, "signedDistance", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto points    = pointsAround(a_box, c.size(PairQueries, SmokeQueries));
    const auto distances = Perf::measureHostAndDevice<T>(
      c, pool, points, [&](const EBGeometry::Pool&) { return Distance<T, Shape>{a_shape}; }, Perf::agreeDistance<T>);

    c.checksum(Perf::checksumOf(distances));
  });
}

// Rays from random points in a_box, in random directions.
template <class T>
std::vector<EBGeometry::RayT<T>>
rays(const AABB<T>& a_box, const std::size_t a_n)
{
  const auto starts     = Perf::pointsInBox<T>(a_box, a_n, Perf::Seed::RayOrigins);
  const auto directions = Perf::pointsInBox<T>(Perf::cube(T(-1), T(1)), a_n, Perf::Seed::ParticleDirs);

  std::vector<EBGeometry::RayT<T>> result(a_n);

  for (std::size_t i = 0; i < a_n; i++) {
    const T length = directions[i].length();

    result[i] = EBGeometry::RayT<T>{starts[i], (length > T(0)) ? directions[i] / length : Vec3<T>(T(1), T(0), T(0))};
  }

  return result;
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  const EBGeometry::EllipsoidSDF<T>  ellipsoid(Vec3<T>::zeros(), Vec3<T>(T(1), T(0.4), T(0.7)));
  const EBGeometry::CappedConeSDF<T> cappedCone(
    Vec3<T>(T(0), T(-0.6), T(0)), Vec3<T>(T(0.2), T(0.6), T(0.1)), T(0.8), T(0.3));
  const EBGeometry::RoundConeSDF<T> roundCone(
    Vec3<T>(T(0), T(-0.5), T(0)), Vec3<T>(T(0.2), T(0.7), T(0.1)), T(0.6), T(0.25));
  const EBGeometry::PolygonPrismSDF<T> prism(
    Vec3<T>(T(0), T(-0.6), T(0)), Vec3<T>(T(0.2), T(0.6), T(0.1)), T(0.8), PrismSides, Vec3<T>::ones());
  const EBGeometry::TubeSDF<T> tube(Vec3<T>(T(0), T(-0.6), T(0)), Vec3<T>(T(0.2), T(0.6), T(0.1)), T(0.5), T(0.8));
  const EBGeometry::CappedTorusSDF<T>     cappedTorus(Vec3<T>::zeros(), T(1), T(0.3), T(120));
  const EBGeometry::CutSphereSDF<T>       cutSphere(Vec3<T>::zeros(), T(1), T(0.2));
  const EBGeometry::CutHollowSphereSDF<T> cutHollowSphere(Vec3<T>::zeros(), T(1), T(0), T(0.15));
  const EBGeometry::WedgeSDF<T>           wedge(Vec3<T>::zeros(), T(60));

  addShape<T>(a_suite, "ellipsoid", ellipsoid, ellipsoid.computeBoundingVolume());
  addShape<T>(a_suite, "capped-cone", cappedCone, cappedCone.computeBoundingVolume());
  addShape<T>(a_suite, "round-cone", roundCone, roundCone.computeBoundingVolume());
  addShape<T>(a_suite, "polygon-prism", prism, prism.computeBoundingVolume());
  addShape<T>(a_suite, "tube", tube, tube.computeBoundingVolume());
  addShape<T>(a_suite, "capped-torus", cappedTorus, cappedTorus.computeBoundingVolume());
  addShape<T>(a_suite, "cut-sphere", cutSphere, cutSphere.computeBoundingVolume());
  addShape<T>(a_suite, "cut-hollow-sphere", cutHollowSphere, cutHollowSphere.computeBoundingVolume());
  addShape<T>(a_suite, "wedge", wedge, Perf::cube(T(-1), T(1)));

  // The six shapes with closed-form ray forms, side by side in a fused union, the wedge cut off by a box.
  const auto at = [](const T a_x, const T a_z) { return Vec3<T>(a_x, T(0), a_z); };

  const auto scene = EBGeometry::Fused::Union(
    EBGeometry::Fused::Translate(ellipsoid, at(T(-2), T(-2))),
    EBGeometry::Fused::Translate(cappedCone, at(T(0), T(-2))),
    EBGeometry::Fused::Translate(prism, at(T(2), T(-2))),
    EBGeometry::Fused::Translate(tube, at(T(-2), T(1))),
    EBGeometry::Fused::Translate(cutSphere, at(T(0), T(1))),
    EBGeometry::Fused::Intersection(EBGeometry::Fused::Translate(wedge, at(T(2), T(1))),
                                    EBGeometry::BoxSDF<T>(Vec3<T>(T(1), T(-1), T(0)), Vec3<T>(T(3), T(1), T(2)))));

  using Scene = std::decay_t<decltype(scene)>;

  const AABB<T>     box(Vec3<T>(T(-3.5), T(-1.5), T(-3.5)), Vec3<T>(T(3.5), T(1.5), T(2.5)));
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"Shapes", "ray-forms", "firstHit", prec, "host"}), [=](Perf::Case& c) {
    std::vector<T> results;

    c.param("sides", PrismSides);

    Perf::measureHostQuery(c, FirstHit<T, Scene>{scene}, rays<T>(box, c.size(HostRays, SmokeQueries)), results);

    c.checksum(Perf::checksumOf(results));
  });

  a_suite.add(Perf::caseName({"Shapes", "ray-forms", "firstHit", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    c.param("sides", PrismSides);

    const auto results = Perf::measureHostAndDevice<T>(
      c,
      pool,
      rays<T>(box, c.size(PairRays, SmokeQueries)),
      [&](const EBGeometry::Pool&) { return FirstHit<T, Scene>{scene}; },
      Perf::agreeDistance<T>);

    c.checksum(Perf::checksumOf(results));
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "Shapes");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
