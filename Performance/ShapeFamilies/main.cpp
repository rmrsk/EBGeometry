// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Performance cases for the shape families (EBGeometry_AnalyticDistanceFunctions.hpp, EBGeometry_SampledGridSDF.hpp),
// in float and double: each shape's signed distance at random points around it (a regular solid and a pyramid of
// ConvexPolyhedronSDF, BoxFrameSDF, the three QuadricSDF kinds, SuperellipsoidSDF, SolidAngleSDF, VesicaSDF, HelixSDF,
// LatticeSDF and SampledGridSDF), and the first crossing of rays through a fused union of the shapes with closed-form
// ray forms. A host build runs the host cases; a CUDA or HIP build runs them as host/device pairs. An analytic shape
// reads no memory but its own few values, so the order of its queries does not matter and its pairs take them as
// drawn; the sampled grid reads its samples from a pool, so its pairs take both orders. The cases are in a probe of
// their own, not in Shapes: cases added to a probe's file can move its old cases by changing how GCC inlines them.
// See ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Queries per case, at full and at smoke size.
constexpr std::size_t HostQueries  = 200000;
constexpr std::size_t PairQueries  = 1048576;
constexpr std::size_t HostRays     = 100000;
constexpr std::size_t PairRays     = 262144;
constexpr std::size_t SmokeQueries = 1000;

// The pyramid's number of sides, which sets its faces' loop, and the grid's nodes along each axis.
constexpr unsigned PyramidSides = 8;
constexpr uint32_t GridNodes    = 64;

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

// The cases for one analytic shape: its signed distance on the host and as a host/device pair, at points around a_box.
template <class T, class Shape>
void
addShape(Perf::Suite& a_suite, const std::string& a_input, const Shape& a_shape, const AABB<T>& a_box)
{
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"ShapeFamilies", a_input, "signedDistance", prec, "host"}), [=](Perf::Case& c) {
    const auto     points = pointsAround(a_box, c.size(HostQueries, SmokeQueries));
    std::vector<T> distances;

    Perf::measureHostQuery(c, Distance<T, Shape>{a_shape}, points, distances);

    c.checksum(Perf::checksumOf(distances));
  });

  a_suite.add(Perf::caseName({"ShapeFamilies", a_input, "signedDistance", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto points    = pointsAround(a_box, c.size(PairQueries, SmokeQueries));
    const auto distances = Perf::measureHostAndDevice<T>(
      c, pool, points, [&](const EBGeometry::Pool&) { return Distance<T, Shape>{a_shape}; }, Perf::agreeDistance<T>);

    c.checksum(Perf::checksumOf(distances));
  });
}

// A sampled ball of radius 0.8, on GridNodes^3 nodes over [-1, 1]^3.
template <class T>
EBGeometry::SampledGridSDF<T>
sampledBall(EBGeometry::Pool& a_pool)
{
  const T spacing = T(2) / T(GridNodes - 1U);

  std::vector<T> values;

  for (uint32_t k = 0; k < GridNodes; k++) {
    for (uint32_t j = 0; j < GridNodes; j++) {
      for (uint32_t i = 0; i < GridNodes; i++) {
        const Vec3<T> p = Vec3<T>(T(-1), T(-1), T(-1)) + spacing * Vec3<T>(T(i), T(j), T(k));

        values.push_back(p.length() - T(0.8));
      }
    }
  }

  return EBGeometry::SampledGridSDF<T>(
    a_pool, Vec3<T>(T(-1), T(-1), T(-1)), spacing * Vec3<T>::ones(), GridNodes, GridNodes, GridNodes, values);
}

// The sampled grid's cases: its signed distance on the host, and as host/device pairs in both orders.
template <class T>
void
addGrid(Perf::Suite& a_suite)
{
  using Grid = EBGeometry::SampledGridSDF<T>;

  const std::string prec = Perf::precisionName<T>();
  const AABB<T>     box  = Perf::cube(T(-1), T(1));

  a_suite.add(Perf::caseName({"ShapeFamilies", "sampled-grid", "signedDistance", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Grid     grid   = sampledBall<T>(pool);
    const auto     points = pointsAround(box, c.size(HostQueries, SmokeQueries));
    std::vector<T> distances;

    c.param("nodes", GridNodes);
    c.poolBytes(pool.usedBytes());

    Perf::measureHostQuery(c, Distance<T, Grid>{grid}, points, distances);

    c.checksum(Perf::checksumOf(distances));
  });

  for (const Perf::Order order : Perf::Orders) {
    a_suite.add(
      Perf::caseName(
        {"ShapeFamilies", std::string("sampled-grid-") + Perf::orderName(order), "signedDistance", prec, "hostdevice"}),
      [=](Perf::Case& c) {
        EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

        const Grid grid   = sampledBall<T>(pool);
        const auto points = Perf::inOrder(order, pointsAround(box, c.size(PairQueries, SmokeQueries)));

        c.param("nodes", GridNodes);
        c.poolBytes(pool.usedBytes());

        const auto distances = Perf::measureHostAndDevice<T>(
          c,
          pool,
          points,
          [&](const EBGeometry::Pool& a_pool) { return Distance<T, Grid>{grid.rebasedView(a_pool)}; },
          Perf::agreeDistance<T>);

        c.checksum(Perf::checksumOf(distances));
      });
  }
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
  using Polyhedron = EBGeometry::ConvexPolyhedronSDF<T>;
  using Quadric    = EBGeometry::QuadricSDF<T>;

  const Polyhedron icosahedron = Polyhedron::icosahedron(Vec3<T>::zeros(), T(1));
  const Polyhedron pyramid     = Polyhedron::pyramid(
    Vec3<T>(T(0), T(-0.6), T(0)), Vec3<T>(T(0.2), T(0.8), T(0.1)), T(0.8), PyramidSides, Vec3<T>::ones());
  const EBGeometry::BoxFrameSDF<T> frame(Vec3<T>(T(-0.8), T(-0.6), T(-0.7)), Vec3<T>(T(0.7), T(0.8), T(0.6)), T(0.15));

  const Quadric hyperboloid = Quadric::hyperboloid(Vec3<T>::zeros(), T(0.4), T(60));
  const Quadric twoSheets   = Quadric::hyperboloidTwoSheets(Vec3<T>::zeros(), T(0.3), T(80));
  const Quadric paraboloid  = Quadric::paraboloid(Vec3<T>(T(0), T(0.8), T(0)), T(0.4));

  const EBGeometry::SuperellipsoidSDF<T> superellipsoid(
    Vec3<T>::zeros(), Vec3<T>(T(1), T(0.7), T(0.8)), T(0.3), T(1.5));
  const EBGeometry::SolidAngleSDF<T> solidAngle(Vec3<T>::zeros(), T(1), T(100));
  const EBGeometry::VesicaSDF<T>     vesica(Vec3<T>(T(0), T(-1), T(0)), Vec3<T>(T(0.2), T(1), T(0)), T(0.4));
  const EBGeometry::HelixSDF<T>      helix(
    Vec3<T>(T(0), T(-1), T(0)), Vec3<T>(T(0), T(1), T(0)), T(0.7), T(5), T(0.1), Vec3<T>(T(1), T(0), T(0)));
  const EBGeometry::LatticeSDF<T> gyroid(EBGeometry::LatticeSDF<T>::Kind::Gyroid, Vec3<T>::ones(), T(0.1));

  const AABB<T> unit = Perf::cube(T(-1), T(1));

  addShape<T>(a_suite, "icosahedron", icosahedron, icosahedron.computeBoundingVolume());
  addShape<T>(a_suite, "pyramid", pyramid, pyramid.computeBoundingVolume());
  addShape<T>(a_suite, "box-frame", frame, frame.computeBoundingVolume());
  addShape<T>(a_suite, "hyperboloid", hyperboloid, unit);
  addShape<T>(a_suite, "hyperboloid-two-sheets", twoSheets, unit);
  addShape<T>(a_suite, "paraboloid", paraboloid, unit);
  addShape<T>(a_suite, "superellipsoid", superellipsoid, superellipsoid.computeBoundingVolume());
  addShape<T>(a_suite, "solid-angle", solidAngle, solidAngle.computeBoundingVolume());
  addShape<T>(a_suite, "vesica", vesica, vesica.computeBoundingVolume());
  addShape<T>(a_suite, "helix", helix, helix.computeBoundingVolume());
  addShape<T>(a_suite, "gyroid", gyroid, unit);
  addGrid<T>(a_suite);

  // The shapes with closed-form ray forms, side by side in a fused union, the unbounded ones cut by boxes.
  const auto at   = [](const T a_x, const T a_z) { return Vec3<T>(a_x, T(0), a_z); };
  const auto cell = [](const T a_x, const T a_z) {
    return EBGeometry::BoxSDF<T>(Vec3<T>(a_x - T(1), T(-1), a_z - T(1)), Vec3<T>(a_x + T(1), T(1), a_z + T(1)));
  };

  const auto scene = EBGeometry::Fused::Union(
    EBGeometry::Fused::Translate(icosahedron, at(T(-2), T(-2))),
    EBGeometry::Fused::Translate(pyramid, at(T(0), T(-2))),
    EBGeometry::Fused::Translate(frame, at(T(2), T(-2))),
    EBGeometry::Fused::Intersection(EBGeometry::Fused::Translate(hyperboloid, at(T(-2), T(1))), cell(T(-2), T(1))),
    EBGeometry::Fused::Intersection(EBGeometry::Fused::Translate(twoSheets, at(T(0), T(1))), cell(T(0), T(1))),
    EBGeometry::Fused::Intersection(EBGeometry::Fused::Translate(paraboloid, at(T(2), T(1))), cell(T(2), T(1))));

  using Scene = std::decay_t<decltype(scene)>;

  const AABB<T>     box(Vec3<T>(T(-3.5), T(-1.5), T(-3.5)), Vec3<T>(T(3.5), T(1.5), T(2.5)));
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"ShapeFamilies", "ray-forms", "firstHit", prec, "host"}), [=](Perf::Case& c) {
    std::vector<T> results;

    c.param("sides", PyramidSides);

    Perf::measureHostQuery(c, FirstHit<T, Scene>{scene}, rays<T>(box, c.size(HostRays, SmokeQueries)), results);

    c.checksum(Perf::checksumOf(results));
  });

  a_suite.add(Perf::caseName({"ShapeFamilies", "ray-forms", "firstHit", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    c.param("sides", PyramidSides);

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
  Perf::Suite suite(argc, argv, "ShapeFamilies");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
