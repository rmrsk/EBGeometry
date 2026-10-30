// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the fused CSG operations (EBGeometry_Fused.hpp): the Tape probe's solids (the
// classic CSG solid, a smooth solid, a chain of 16 spheres) as fused compositions, and a mesh drilled
// through by a cylinder, fused and through a tape, in float and double, at random points around each
// object. A host build runs the host cases; a CUDA or HIP build runs them as host/device pairs. Compare
// with Performance/Tape, whose cases are the same solids through a tape and written by hand: each fused
// case names its tape counterpart (Case::relativeTo()), and a comparison notes time(tape)/time(fused)
// beside it, so that the cost of interpreting a tape is tracked. They are in a probe of their own because
// cases added to the Tape probe's translation unit change how GCC inlines the tape's evaluator there, and
// so the Tape cases' timings. See ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Queries per case, at full and at smoke size, as in the Tape probe; the mesh cases' queries traverse a
// BVH and cost microseconds each.
constexpr std::size_t HostQueries     = 400000;
constexpr std::size_t PairQueries     = 1048576;
constexpr std::size_t SmokeQueries    = 1000;
constexpr std::size_t MeshHostQueries = 20000;
constexpr std::size_t MeshPairQueries = 262144;

// The spheres of the chain.
constexpr std::size_t ChainSpheres = 16;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// Sphere i of the chain: ChainSpheres spheres in a row along x, overlapping their neighbors.
template <class T>
EBGeometry::SphereSDF<T>
chainSphere(std::size_t a_index)
{
  return EBGeometry::SphereSDF<T>(Vec3<T>(T(0.3) * T(a_index) - T(2.25), T(0.1) * T(a_index % 3), T(0)), T(0.4));
}

// The classic CSG solid as a fused composition (EBGeometry_Fused.hpp): the tape's expression, compiled
// as types instead of interpreted.
template <class T>
auto
fusedCSG()
{
  const EBGeometry::BoxSDF<T>    cube(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1)));
  const EBGeometry::SphereSDF<T> ball(Vec3<T>::zeros(), T(1.35));

  const EBGeometry::InfiniteCylinderSDF<T> drill0(Vec3<T>::zeros(), T(0.55), 0);
  const EBGeometry::InfiniteCylinderSDF<T> drill1(Vec3<T>::zeros(), T(0.55), 1);
  const EBGeometry::InfiniteCylinderSDF<T> drill2(Vec3<T>::zeros(), T(0.55), 2);

  return EBGeometry::Fused::Difference(EBGeometry::Fused::Intersection(cube, ball), drill0, drill1, drill2);
}

// The chain of spheres as a fused union.
template <class T, std::size_t... Is>
auto
fusedChain(std::index_sequence<Is...>)
{
  return EBGeometry::Fused::Union(chainSphere<T>(Is)...);
}

// The smooth object as a fused composition.
template <class T, std::size_t... Is>
auto
fusedSmooth(std::index_sequence<Is...>)
{
  const auto blob = [](const int i) {
    return EBGeometry::SphereSDF<T>(Vec3<T>(T(0.4) * T(i % 3) - T(0.4), T(0.5) * T(i / 3), T(0)), T(0.35));
  };

  return EBGeometry::Fused::SmoothDifference(
    EBGeometry::Fused::SmoothUnion(EBGeometry::Fused::operands(blob(int(Is))...), T(0.15)),
    EBGeometry::BoxSDF<T>(Vec3<T>(T(-1), T(0.3), T(-0.1)), Vec3<T>(T(1), T(0.4), T(0.1))),
    T(0.05));
}

// A shape's signed distance at a point, as the pairs need it; with R > 1, the sum of its signed distances
// at R points in a short row from it, so that one query (one device thread) evaluates the shape R times.
template <class T, class Shape, std::size_t R = 1>
struct Distance
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_point) const noexcept
  {
    T sum = T(0);

    for (std::size_t k = 0; k < R; k++) {
      sum += m_shape.signedDistance(a_point + T(0.01) * T(k) * Vec3<T>(T(1), T(0.5), T(0.25)));
    }

    return sum;
  }
};

// Random points in a box grown by a quarter of its size on every side.
template <class T>
std::vector<Vec3<T>>
pointsAround(const EBGeometry::BoundingVolumes::AABBT<T>& a_box, std::size_t a_n)
{
  const Vec3<T> grow = T(0.25) * (a_box.getHighCorner() - a_box.getLowCorner());

  return Perf::pointsInBox<T>(
    EBGeometry::BoundingVolumes::AABBT<T>(a_box.getLowCorner() - grow, a_box.getHighCorner() + grow),
    a_n,
    Perf::Seed::TapeQueries);
}

// The cases for one object: its signed distance on the host and as a host/device pair. a_make(case, pool)
// builds it in the pool; a_box is the box its points are drawn around.
template <class T, class Shape, std::size_t R = 1, class Make>
void
addObject(Perf::Suite&                                 a_suite,
          const std::string&                           a_input,
          const EBGeometry::BoundingVolumes::AABBT<T>& a_box,
          Make                                         a_make)
{
  const std::string prec      = Perf::precisionName<T>();
  const std::string operation = (R == 1) ? "signedDistance" : "signedDistance-x" + std::to_string(R);

  a_suite.add(Perf::caseName({"Fused", a_input, operation, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Shape    shape  = a_make(c, pool);
    const auto     points = pointsAround(a_box, c.size(HostQueries, SmokeQueries));
    std::vector<T> distances;

    Perf::measureHostQuery(c, Distance<T, Shape, R>{shape}, points, distances);

    c.checksum(Perf::checksumOf(distances));
    c.poolBytes(pool.usedBytes());
    c.param("repeats", R);
    c.relativeTo(Perf::caseName({"Tape", a_input, operation, prec, "host"}), "tape/fused");
  });

  a_suite.add(Perf::caseName({"Fused", a_input, operation, prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Shape shape  = a_make(c, pool);
    const auto  points = pointsAround(a_box, c.size(PairQueries, SmokeQueries));

    const auto distances = Perf::measureHostAndDevice<T>(
      c,
      pool,
      points,
      [&](const EBGeometry::Pool& a_pool) { return Distance<T, Shape, R>{shape.rebasedView(a_pool)}; },
      Perf::agreeDistance<T>);

    c.checksum(Perf::checksumOf(distances));
    c.param("repeats", R);
    c.relativeTo(Perf::caseName({"Tape", a_input, operation, prec, "hostdevice"}), "tape/fused");
  });
}

// A mesh drilled through along x by an infinite cylinder: through a tape (Fused = false) or as a fused
// composition. The mesh is the armadillo (the dodecahedron under --smoke), so its box, the drill and the
// query points come from the mesh once it is read.
template <class T, bool Fused>
void
addDrilledMesh(Perf::Suite& a_suite)
{
  using Mesh = EBGeometry::TriMeshSDF<T, 4, 4>;

  const std::string input = Fused ? "drilled-mesh" : "drilled-mesh-tape";
  const std::string prec  = Perf::precisionName<T>();

  const auto drillFor = [](const EBGeometry::BoundingVolumes::AABBT<T>& a_box) {
    const Vec3<T> size = a_box.getHighCorner() - a_box.getLowCorner();
    const Vec3<T> mid  = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());

    return EBGeometry::InfiniteCylinderSDF<T>(mid, T(0.12) * size[1], 0);
  };

  // The shape and its box, built in a_pool; the tape is built in the mesh's own pool.
  const auto build = [drillFor](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Mesh mesh  = EBGeometry::Parser::readIntoTriMeshSDF<T, 4, 4>(Perf::meshFile(c.smoke()), a_pool);
    const auto box   = mesh.computeBoundingVolume();
    const auto drill = drillFor(box);

    c.param("K", 4);
    c.param("W", 4);

    if constexpr (Fused) {
      return std::make_pair(EBGeometry::Fused::Difference(mesh, drill), box);
    }
    else {
      return std::make_pair(EBGeometry::Tape<T, Mesh>(a_pool, EBGeometry::Difference(EBGeometry::Expr<T>(mesh), drill)),
                            box);
    }
  };

  using Shape = typename std::invoke_result_t<decltype(build), Perf::Case&, EBGeometry::Pool&>::first_type;

  a_suite.add(Perf::caseName({"Fused", input, "signedDistance", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto     made   = build(c, pool);
    const auto     points = pointsAround(made.second, c.size(MeshHostQueries, SmokeQueries));
    std::vector<T> distances;

    Perf::measureHostQuery(c, Distance<T, Shape>{made.first}, points, distances);

    c.checksum(Perf::checksumOf(distances));
    c.poolBytes(pool.usedBytes());

    if constexpr (Fused) {
      c.relativeTo(Perf::caseName({"Fused", "drilled-mesh-tape", "signedDistance", prec, "host"}), "tape/fused");
    }
  });

  a_suite.add(Perf::caseName({"Fused", input, "signedDistance", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto made   = build(c, pool);
    const auto points = pointsAround(made.second, c.size(MeshPairQueries, SmokeQueries));

    const auto distances = Perf::measureHostAndDevice<T>(
      c,
      pool,
      points,
      [&](const EBGeometry::Pool& a_pool) { return Distance<T, Shape>{made.first.rebasedView(a_pool)}; },
      Perf::agreeDistance<T>);

    c.checksum(Perf::checksumOf(distances));

    if constexpr (Fused) {
      c.relativeTo(Perf::caseName({"Fused", "drilled-mesh-tape", "signedDistance", prec, "hostdevice"}), "tape/fused");
    }
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

  using FusedCSG    = decltype(fusedCSG<T>());
  using FusedSmooth = decltype(fusedSmooth<T>(std::make_index_sequence<6>{}));
  using FusedChain  = decltype(fusedChain<T>(std::make_index_sequence<ChainSpheres>{}));

  const AABB cube(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1)));

  addObject<T, FusedCSG>(a_suite, "csg", cube, [](Perf::Case&, EBGeometry::Pool&) { return fusedCSG<T>(); });

  const FusedSmooth smooth = fusedSmooth<T>(std::make_index_sequence<6>{});

  addObject<T, FusedSmooth>(
    a_suite, "smooth", smooth.computeBoundingVolume(), [smooth](Perf::Case&, EBGeometry::Pool&) { return smooth; });

  const FusedChain chain = fusedChain<T>(std::make_index_sequence<ChainSpheres>{});

  addObject<T, FusedChain>(
    a_suite, "chain16", chain.computeBoundingVolume(), [chain](Perf::Case&, EBGeometry::Pool&) { return chain; });

  // A mesh with a hole, fused and through a tape.
  addDrilledMesh<T, true>(a_suite);
  addDrilledMesh<T, false>(a_suite);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "Fused");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
