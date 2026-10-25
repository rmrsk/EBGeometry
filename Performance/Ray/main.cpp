// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for the ray queries (EBGeometry_Ray.hpp): rays cast at a TriMeshSDF, traced exactly
// through its BVH and triangles, both through firstHit() and through a hand-written traverse() with a
// RayQuery (what firstHit() costs over code a user writes), and intersects(); and rays cast at the classic
// CSG solid, marched, as a fused composition and through a tape. Every ray starts at a random point in
// the object's box grown to twice its size and points at the box's centre, so that rays from nearby
// origins run side by side. A host build runs the host cases; a CUDA or HIP build runs host/device pairs
// in Morton and random order. See ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The mesh's branching factor and leaf width, spelled out (see ../Mesh/main.cpp).
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// The numbers of rays, at full and at smoke size: a mesh ray costs about a microsecond, a marched one
// tens of signed distances.
constexpr std::size_t MeshHostQueries    = 100000;
constexpr std::size_t MarchedHostQueries = 50000;
constexpr std::size_t PairQueries        = 262144;
constexpr std::size_t SmokeQueries       = 1000;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

template <class T>
using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

// The ray from a_origin towards a_target.
template <class T>
EBGEOMETRY_HOST_DEVICE
EBGeometry::RayT<T>
rayTowards(const Vec3<T>& a_origin, const Vec3<T>& a_target) noexcept
{
  const Vec3<T> d = a_target - a_origin;

  return EBGeometry::RayT<T>{a_origin, d / d.length()};
}

// Where the ray from a point towards the target first crosses the shape (-1 for no crossing), through
// firstHit().
template <class T, class Shape>
struct FirstHit
{
  Shape   m_shape;
  Vec3<T> m_target;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    const EBGeometry::HitT<T> hit =
      EBGeometry::firstHit(m_shape, rayTowards(a_origin, m_target), T(0), EBGeometry::Math::Limits<T>::infinity());

    return hit.hit() ? hit.m_t : -T(1);
  }
};

// Whether the ray from a point towards the target crosses the shape (1 or 0), through intersects().
template <class T, class Shape>
struct Intersects
{
  Shape   m_shape;
  Vec3<T> m_target;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    const EBGeometry::HitStatus status =
      EBGeometry::intersects(m_shape, rayTowards(a_origin, m_target), T(0), EBGeometry::Math::Limits<T>::infinity());

    return status == EBGeometry::HitStatus::Hit ? T(1) : T(0);
  }
};

// The same crossing as FirstHit on the mesh, from a hand-written walk: RayQuery, nearest first, the bound
// the nearest crossing so far, and a leaf that asks each triangle group where the ray crosses it.
template <class T>
struct HandRolled
{
  Mesh<T> m_mesh;
  Vec3<T> m_target;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    const EBGeometry::RayT<T> ray    = rayTowards(a_origin, m_target);
    const auto                bvh    = m_mesh.getRoot();
    const auto                groups = bvh.getPrimitives();

    T nearest = EBGeometry::Math::Limits<T>::infinity();

    bvh.traverse(
      EBGeometry::BVH::RayQuery<T>(ray.m_origin, ray.m_direction),
      nearest,
      [&ray, &groups](T& a_nearest, std::size_t a_offset, std::size_t a_count) noexcept {
        for (std::size_t i = a_offset; i < a_offset + a_count; i++) {
          std::uint32_t face = UINT32_MAX;
          Vec3<T>       normal;

          const T t = groups[static_cast<std::uint32_t>(i)].rayCrossing(
            ray.m_origin, ray.m_direction, T(0), a_nearest, face, normal);

          a_nearest = (t < a_nearest) ? t : a_nearest;
        }
      },
      [](const T& a_nearest) noexcept { return a_nearest; });

    return nearest < EBGeometry::Math::Limits<T>::infinity() ? nearest : -T(1);
  }
};

// The classic CSG solid of the Tape and Fused probes: a cube rounded by a ball, drilled along each axis.
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

// The same solid as an expression, for a tape.
template <class T>
EBGeometry::Expr<T>
csgExpr()
{
  const EBGeometry::BoxSDF<T>    cube(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1)));
  const EBGeometry::SphereSDF<T> ball(Vec3<T>::zeros(), T(1.35));

  const std::vector<EBGeometry::Expr<T>> drills{EBGeometry::InfiniteCylinderSDF<T>(Vec3<T>::zeros(), T(0.55), 0),
                                                EBGeometry::InfiniteCylinderSDF<T>(Vec3<T>::zeros(), T(0.55), 1),
                                                EBGeometry::InfiniteCylinderSDF<T>(Vec3<T>::zeros(), T(0.55), 2)};

  return EBGeometry::Difference(EBGeometry::Intersection(cube, ball), EBGeometry::Union(drills));
}

// Ray origins: random points in a box grown to twice its size about its centre.
template <class T>
std::vector<Vec3<T>>
origins(const AABB<T>& a_box, std::size_t a_n)
{
  const Vec3<T> centre = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const Vec3<T> half   = a_box.getHighCorner() - centre;

  return Perf::pointsInBox<T>(AABB<T>(centre - T(2) * half, centre + T(2) * half), a_n, Perf::Seed::RayOrigins);
}

// Whether a host and a device marched crossing agree: both miss, or both cross within twice the march's
// tolerance (RayOptions::m_tolerance times the solid's box diagonal, under 4e-4 here).
template <class T>
std::pair<double, bool>
agreeMarched(T a_host, T a_device)
{
  const double diff = std::abs(static_cast<double>(a_host) - static_cast<double>(a_device));

  return {diff, (a_host < T(0)) != (a_device < T(0)) || diff > 1e-3};
}

// One operation on one input in one precision: the host case, and host/device pairs in each order.
// a_build(case, pool) builds the shape in the pool and returns it with its box; a_make(shape, centre)
// gives the query functor. a_relativeTo names a case computing the same answers, and the comparison notes
// its time over this one's as a_relativeLabel.
template <class T, class Build, class Make, class Agree>
void
addOperation(Perf::Suite&       a_suite,
             const std::string& a_input,
             const std::string& a_operation,
             const std::size_t  a_hostQueries,
             Build              a_build,
             Make               a_make,
             Agree              a_agree,
             const std::string& a_relativeTo    = "",
             const std::string& a_relativeLabel = "")
{
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"Ray", a_input, a_operation, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto     made   = a_build(c, pool);
    const Vec3<T>  centre = T(0.5) * (made.second.getLowCorner() + made.second.getHighCorner());
    const auto     q      = origins<T>(made.second, c.size(a_hostQueries, SmokeQueries));
    std::vector<T> out;

    Perf::measureHostQuery(c, a_make(made.first, centre), q, out);

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());

    if (!a_relativeTo.empty()) {
      c.relativeTo(Perf::caseName({"Ray", a_relativeTo, prec, "host"}), a_relativeLabel);
    }
  });

  for (const Perf::Order order : Perf::Orders) {
    const std::string operation = a_operation + "-" + Perf::orderName(order);

    a_suite.add(Perf::caseName({"Ray", a_input, operation, prec, "hostdevice"}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const auto    made   = a_build(c, pool);
      const Vec3<T> centre = T(0.5) * (made.second.getLowCorner() + made.second.getHighCorner());
      const auto    q      = Perf::inOrder(order, origins<T>(made.second, c.size(PairQueries, SmokeQueries)));

      const auto out = Perf::measureHostAndDevice<T>(
        c,
        pool,
        q,
        [&](const EBGeometry::Pool& a_pool) { return a_make(made.first.rebasedView(a_pool), centre); },
        a_agree);

      c.checksum(Perf::checksumOf(out));

      if (!a_relativeTo.empty()) {
        c.relativeTo(Perf::caseName({"Ray", a_relativeTo + "-" + Perf::orderName(order), prec, "hostdevice"}),
                     a_relativeLabel);
      }
    });
  }
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  using FusedCSG = decltype(fusedCSG<T>());

  // The mesh: the armadillo (the dodecahedron under --smoke).
  const auto mesh = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Mesh<T> m = EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(Perf::meshFile(c.smoke()), a_pool);

    c.param("K", K);
    c.param("W", W);

    return std::make_pair(m, m.computeBoundingVolume());
  };

  addOperation<T>(
    a_suite,
    "mesh",
    "firstHit",
    MeshHostQueries,
    mesh,
    [](const Mesh<T>& a_mesh, const Vec3<T>& a_centre) { return FirstHit<T, Mesh<T>>{a_mesh, a_centre}; },
    Perf::agreeDistance<T>,
    "mesh/firstHit-handrolled",
    "hand/api");
  addOperation<T>(
    a_suite,
    "mesh",
    "firstHit-handrolled",
    MeshHostQueries,
    mesh,
    [](const Mesh<T>& a_mesh, const Vec3<T>& a_centre) { return HandRolled<T>{a_mesh, a_centre}; },
    Perf::agreeDistance<T>);
  addOperation<T>(
    a_suite,
    "mesh",
    "intersects",
    MeshHostQueries,
    mesh,
    [](const Mesh<T>& a_mesh, const Vec3<T>& a_centre) { return Intersects<T, Mesh<T>>{a_mesh, a_centre}; },
    Perf::agreeDistance<T>);

  // The CSG solid, marched: fused, and through a tape.
  const auto fused = [](Perf::Case&, EBGeometry::Pool&) {
    const FusedCSG f = fusedCSG<T>();

    return std::make_pair(f, f.computeBoundingVolume());
  };

  const auto tape = [](Perf::Case&, EBGeometry::Pool& a_pool) {
    const EBGeometry::Tape<T> t(a_pool, csgExpr<T>());

    return std::make_pair(t, t.computeBoundingVolume());
  };

  addOperation<T>(
    a_suite,
    "csg-fused",
    "firstHit",
    MarchedHostQueries,
    fused,
    [](const FusedCSG& a_shape, const Vec3<T>& a_centre) { return FirstHit<T, FusedCSG>{a_shape, a_centre}; },
    agreeMarched<T>,
    "csg-tape/firstHit",
    "tape/fused");
  addOperation<T>(
    a_suite,
    "csg-tape",
    "firstHit",
    MarchedHostQueries,
    tape,
    [](const EBGeometry::Tape<T>& a_shape, const Vec3<T>& a_centre) {
      return FirstHit<T, EBGeometry::Tape<T>>{a_shape, a_centre};
    },
    agreeMarched<T>);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "Ray");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
