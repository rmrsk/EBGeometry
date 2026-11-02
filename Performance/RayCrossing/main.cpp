// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for firstCrossing() (EBGeometry_Ray.hpp): the first crossing without its normal or
// primitive, against firstHit() on the same rays, both timed here (hit/crossing): the mesh, the CSG solid
// through a tape, and two tapes holding the mesh, where the normal costs most: the mesh drilled along three axes,
// traced exactly (firstHit() traces the leaf it ends on again for its normal), and the mesh smoothly joined to a
// ball, marched (firstHit() takes the gradient). The firstHit() cases repeat code paths the Ray probe times, but
// a ratio across two probes compares two binaries and ray counts too: the CSG tape's firstCrossing() measured
// 0.89x the Ray probe's firstHit(), and 1.12x its own firstHit() on the same rays in one binary. Every
// ray starts at a random point in the object's box grown to twice its size and points at the box's center. A
// host build runs the host cases; a CUDA or HIP build runs host/device pairs in Morton and random order. See
// ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The mesh's branching factor and leaf width, spelled out, as in ../Ray/main.cpp.
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// The numbers of rays, as in ../Ray/main.cpp, and fewer for the marched tape, whose rays each take tens of mesh
// distances.
constexpr std::size_t MeshHostQueries    = 100000;
constexpr std::size_t MarchedHostQueries = 50000;
constexpr std::size_t SmoothHostQueries  = 5000;
constexpr std::size_t PairQueries        = 262144;
constexpr std::size_t SmoothPairQueries  = 65536;
constexpr std::size_t SmokeQueries       = 1000;

// The drills' radius and the smoothing length, relative to the mesh's box diagonal; the ball's radius too.
constexpr double DrillRadius = 0.05;
constexpr double Smoothing   = 0.05;
constexpr double BallRadius  = 0.15;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

template <class T>
using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

// A tape with the mesh as a primitive type of its own (it is not the default TriMeshSDF in every build).
template <class T>
using MeshTape = EBGeometry::Tape<T, Mesh<T>>;

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

// The same, through firstCrossing().
template <class T, class Shape>
struct FirstCrossing
{
  Shape   m_shape;
  Vec3<T> m_target;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    const EBGeometry::CrossingT<T> crossing =
      EBGeometry::firstCrossing(m_shape, rayTowards(a_origin, m_target), T(0), EBGeometry::Math::Limits<T>::infinity());

    return crossing.hit() ? crossing.m_t : -T(1);
  }
};

// The classic CSG solid of ../Ray/main.cpp, as an expression: a cube rounded by a ball, drilled along each axis.
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

// Ray origins: random points in a box grown to twice its size about its center, as in ../Ray/main.cpp.
template <class T>
std::vector<Vec3<T>>
origins(const AABB<T>& a_box, std::size_t a_n)
{
  const Vec3<T> center = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const Vec3<T> half   = a_box.getHighCorner() - center;

  return Perf::pointsInBox<T>(AABB<T>(center - T(2) * half, center + T(2) * half), a_n, Perf::Seed::RayOrigins);
}

// Whether a host and a device marched crossing agree: both miss, or both cross within 1e-3 (the march's
// tolerance is 1e-4 of the box diagonal).
template <class T>
std::pair<double, bool>
agreeMarched(T a_host, T a_device)
{
  const double diff = std::abs(static_cast<double>(a_host) - static_cast<double>(a_device));

  return {diff, (a_host < T(0)) != (a_device < T(0)) || diff > 1e-3 * (1.0 + std::abs(static_cast<double>(a_host)))};
}

// One operation on one input in one precision: the host case, and host/device pairs in each order, as in
// ../Ray/main.cpp. a_relativeTo names a case computing the same answers (a full name less its order and side),
// and the comparison notes its time over this one's as a_relativeLabel.
template <class T, class Build, class Make, class Agree>
void
addOperation(Perf::Suite&       a_suite,
             const std::string& a_input,
             const std::string& a_operation,
             const std::size_t  a_hostQueries,
             const std::size_t  a_pairQueries,
             Build              a_build,
             Make               a_make,
             Agree              a_agree,
             const std::string& a_relativeTo    = "",
             const std::string& a_relativeLabel = "")
{
  const std::string prec = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({"RayCrossing", a_input, a_operation, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto     made   = a_build(c, pool);
    const Vec3<T>  center = T(0.5) * (made.second.getLowCorner() + made.second.getHighCorner());
    const auto     q      = origins<T>(made.second, c.size(a_hostQueries, SmokeQueries));
    std::vector<T> out;

    Perf::measureHostQuery(c, a_make(made.first, center), q, out);

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());

    if (!a_relativeTo.empty()) {
      c.relativeTo(a_relativeTo + "/" + prec + "/host", a_relativeLabel);
    }
  });

  for (const Perf::Order order : Perf::Orders) {
    const std::string operation = a_operation + "-" + Perf::orderName(order);

    a_suite.add(Perf::caseName({"RayCrossing", a_input, operation, prec, "hostdevice"}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const auto    made   = a_build(c, pool);
      const Vec3<T> center = T(0.5) * (made.second.getLowCorner() + made.second.getHighCorner());
      const auto    q      = Perf::inOrder(order, origins<T>(made.second, c.size(a_pairQueries, SmokeQueries)));

      const auto out = Perf::measureHostAndDevice<T>(
        c,
        pool,
        q,
        [&](const EBGeometry::Pool& a_pool) { return a_make(made.first.rebasedView(a_pool), center); },
        a_agree);

      c.checksum(Perf::checksumOf(out));

      if (!a_relativeTo.empty()) {
        c.relativeTo(a_relativeTo + "-" + Perf::orderName(order) + "/" + prec + "/hostdevice", a_relativeLabel);
      }
    });
  }
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  // The mesh: the armadillo (the dodecahedron under --smoke).
  const auto readMesh = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    c.param("K", K);
    c.param("W", W);

    return EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(Perf::meshFile(c.smoke()), a_pool);
  };

  const auto mesh = [readMesh](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Mesh<T> m = readMesh(c, a_pool);

    return std::make_pair(m, m.computeBoundingVolume());
  };

  addOperation<T>(
    a_suite,
    "mesh",
    "firstHit",
    MeshHostQueries,
    PairQueries,
    mesh,
    [](const Mesh<T>& a_mesh, const Vec3<T>& a_center) { return FirstHit<T, Mesh<T>>{a_mesh, a_center}; },
    Perf::agreeDistance<T>);
  addOperation<T>(
    a_suite,
    "mesh",
    "firstCrossing",
    MeshHostQueries,
    PairQueries,
    mesh,
    [](const Mesh<T>& a_mesh, const Vec3<T>& a_center) { return FirstCrossing<T, Mesh<T>>{a_mesh, a_center}; },
    Perf::agreeDistance<T>,
    "RayCrossing/mesh/firstHit",
    "hit/crossing");

  // The CSG solid through a tape.
  const auto csg = [](Perf::Case&, EBGeometry::Pool& a_pool) {
    const EBGeometry::Tape<T> t(a_pool, csgExpr<T>());

    return std::make_pair(t, t.computeBoundingVolume());
  };

  addOperation<T>(
    a_suite,
    "csg-tape",
    "firstHit",
    MarchedHostQueries,
    PairQueries,
    csg,
    [](const EBGeometry::Tape<T>& a_shape, const Vec3<T>& a_center) {
      return FirstHit<T, EBGeometry::Tape<T>>{a_shape, a_center};
    },
    Perf::agreeDistance<T>);
  addOperation<T>(
    a_suite,
    "csg-tape",
    "firstCrossing",
    MarchedHostQueries,
    PairQueries,
    csg,
    [](const EBGeometry::Tape<T>& a_shape, const Vec3<T>& a_center) {
      return FirstCrossing<T, EBGeometry::Tape<T>>{a_shape, a_center};
    },
    Perf::agreeDistance<T>,
    "RayCrossing/csg-tape/firstHit",
    "hit/crossing");

  // The mesh drilled through its center along each axis, through a tape: traced exactly.
  const auto drilled = [readMesh](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Mesh<T> m      = readMesh(c, a_pool);
    const AABB<T> box    = m.computeBoundingVolume();
    const Vec3<T> center = T(0.5) * (box.getLowCorner() + box.getHighCorner());
    const T       radius = T(DrillRadius) * (box.getHighCorner() - box.getLowCorner()).length();

    c.param("drillRadius", DrillRadius);

    const std::vector<EBGeometry::Expr<T>> drills{EBGeometry::InfiniteCylinderSDF<T>(center, radius, 0),
                                                  EBGeometry::InfiniteCylinderSDF<T>(center, radius, 1),
                                                  EBGeometry::InfiniteCylinderSDF<T>(center, radius, 2)};

    const MeshTape<T> t(a_pool, EBGeometry::Difference(m, EBGeometry::Union(drills)));

    return std::make_pair(t, box);
  };

  addOperation<T>(
    a_suite,
    "drilled-mesh-tape",
    "firstHit",
    MeshHostQueries,
    PairQueries,
    drilled,
    [](const MeshTape<T>& a_shape, const Vec3<T>& a_center) { return FirstHit<T, MeshTape<T>>{a_shape, a_center}; },
    Perf::agreeDistance<T>);
  addOperation<T>(
    a_suite,
    "drilled-mesh-tape",
    "firstCrossing",
    MeshHostQueries,
    PairQueries,
    drilled,
    [](const MeshTape<T>& a_shape, const Vec3<T>& a_center) {
      return FirstCrossing<T, MeshTape<T>>{a_shape, a_center};
    },
    Perf::agreeDistance<T>,
    "RayCrossing/drilled-mesh-tape/firstHit",
    "hit/crossing");

  // The mesh smoothly joined to a ball at the top of its box, through a tape: marched.
  const auto smooth = [readMesh](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Mesh<T> m        = readMesh(c, a_pool);
    const AABB<T> box      = m.computeBoundingVolume();
    const T       diagonal = (box.getHighCorner() - box.getLowCorner()).length();
    const Vec3<T> top(T(0.5) * (box.getLowCorner()[0] + box.getHighCorner()[0]),
                      box.getHighCorner()[1],
                      T(0.5) * (box.getLowCorner()[2] + box.getHighCorner()[2]));

    c.param("smoothing", Smoothing);
    c.param("ballRadius", BallRadius);

    const EBGeometry::SphereSDF<T> ball(top, T(BallRadius) * diagonal);

    const MeshTape<T> t(a_pool, EBGeometry::SmoothUnion(m, ball, T(Smoothing) * diagonal));

    return std::make_pair(t, t.computeBoundingVolume());
  };

  addOperation<T>(
    a_suite,
    "smooth-mesh-tape",
    "firstHit",
    SmoothHostQueries,
    SmoothPairQueries,
    smooth,
    [](const MeshTape<T>& a_shape, const Vec3<T>& a_center) { return FirstHit<T, MeshTape<T>>{a_shape, a_center}; },
    agreeMarched<T>);
  addOperation<T>(
    a_suite,
    "smooth-mesh-tape",
    "firstCrossing",
    SmoothHostQueries,
    SmoothPairQueries,
    smooth,
    [](const MeshTape<T>& a_shape, const Vec3<T>& a_center) {
      return FirstCrossing<T, MeshTape<T>>{a_shape, a_center};
    },
    agreeMarched<T>,
    "RayCrossing/smooth-mesh-tape/firstHit",
    "hit/crossing");
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "RayCrossing");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
