// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for GPU::transform()'s launch settings (EBGeometry_GPUTransform.hpp): the block size and
// the threads the CUDA compiler is asked to keep resident on a multiprocessor (GPU::DefaultMinResidentThreads),
// which fixes a kernel's register budget. Three kernels whose best settings differ: a TriMeshSDF's signed
// distance, rays traced through a tape of the classic CSG solid, and a PointCloudBVH's closest point, each at
// the defaults and at a few other settings, which name the default case. Device only: a host build has no
// cases here. See ../README.md and PerformanceTesting.rst.

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

// The mesh's and the cloud's branching factor and leaf width, spelled out (see ../Mesh/main.cpp).
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// The cloud's points per leaf, as in ../PointCloud/main.cpp.
constexpr std::size_t TargetLeafSize = 8;

// The numbers of queries, at full and at smoke size, and of the cloud's points.
constexpr std::size_t PairQueries  = 262144;
constexpr std::size_t SmokeQueries = 1000;
constexpr std::size_t CloudPoints  = 1000000;
constexpr std::size_t SmokeCloud   = 1000;

// The launch settings measured: the defaults, no minimum of resident threads, a larger minimum (fewer
// registers), and twice the block size.
using Default   = Perf::Launch<128, 384>;
using NoMinimum = Perf::Launch<128, 0>;
using Larger    = Perf::Launch<128, 768>;
using Wider     = Perf::Launch<256, 384>;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

template <class T>
using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

template <class T>
using Cloud = EBGeometry::PointCloudBVH<T, K, W>;

// The ray from a_origin towards a_target.
template <class T>
EBGEOMETRY_HOST_DEVICE
EBGeometry::RayT<T>
rayTowards(const Vec3<T>& a_origin, const Vec3<T>& a_target) noexcept
{
  const Vec3<T> d = a_target - a_origin;

  return EBGeometry::RayT<T>{a_origin, d / d.length()};
}

// Where the ray from a point towards the target first crosses the shape (-1 for no crossing).
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

// The distance to the closest cloud point.
template <class T>
struct ClosestDistance
{
  Cloud<T> m_cloud;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_point) const noexcept
  {
    return std::sqrt(m_cloud.closestPoint(a_point).distanceSquared);
  }
};

// The classic CSG solid of the Tape and Ray probes: a cube rounded by a ball, drilled along each axis.
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

// Random points in a box grown to twice its size about its center, in Morton order.
template <class T>
std::vector<Vec3<T>>
around(const AABB<T>& a_box, std::size_t a_n, std::uint64_t a_seed)
{
  const Vec3<T> center = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const Vec3<T> half   = a_box.getHighCorner() - center;

  return Perf::mortonOrdered(Perf::pointsInBox<T>(AABB<T>(center - T(2) * half, center + T(2) * half), a_n, a_seed));
}

// Whether a host and a device crossing agree: both miss, or both cross within 1e-3 (the CSG solid is
// traced exactly; rounding differs between the sides).
template <class T>
std::pair<double, bool>
agreeCrossing(T a_host, T a_device)
{
  const double diff = std::abs(static_cast<double>(a_host) - static_cast<double>(a_device));

  return {diff, (a_host < T(0)) != (a_device < T(0)) || diff > 1e-3};
}

// The name of a launch setting in a case name, e.g. b128-r384.
template <class L>
std::string
launchName()
{
  return "b" + std::to_string(L::blockSize) + "-r" + std::to_string(L::minResidentThreads);
}

// One kernel at one launch setting, in one precision. a_build(case, pool) builds the structure in the pool
// and returns it with the queries and the query functor's maker; the case names the default setting's.
template <class T, class L, class Build, class Agree>
void
addCase(Perf::Suite& a_suite, const std::string& a_input, const std::string& a_operation, Build a_build, Agree a_agree)
{
  const std::string prec      = Perf::precisionName<T>();
  const std::string operation = a_operation + "-" + launchName<L>();

  a_suite.add(Perf::caseName({"LaunchTuning", a_input, operation, prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto made = a_build(c, pool);
    const auto out  = Perf::measureHostAndDevice<T>(c, pool, made.first, made.second, a_agree, L{});

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());

    if (!std::is_same_v<L, Default>) {
      c.relativeTo(
        Perf::caseName({"LaunchTuning", a_input, a_operation + "-" + launchName<Default>(), prec, "hostdevice"}),
        "default/this");
    }
  });
}

// One kernel at every launch setting.
template <class T, class Build, class Agree>
void
addSettings(
  Perf::Suite& a_suite, const std::string& a_input, const std::string& a_operation, Build a_build, Agree a_agree)
{
  addCase<T, Default>(a_suite, a_input, a_operation, a_build, a_agree);
  addCase<T, NoMinimum>(a_suite, a_input, a_operation, a_build, a_agree);
  addCase<T, Larger>(a_suite, a_input, a_operation, a_build, a_agree);
  addCase<T, Wider>(a_suite, a_input, a_operation, a_build, a_agree);
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  // The armadillo (the dodecahedron under --smoke), its signed distance at points around it.
  const auto mesh = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Mesh<T> m = EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(Perf::meshFile(c.smoke()), a_pool);

    c.param("K", K);
    c.param("W", W);

    return std::make_pair(
      around<T>(m.computeBoundingVolume(), c.size(PairQueries, SmokeQueries), Perf::Seed::MeshQueries),
      [m](const EBGeometry::Pool& a_on) { return Perf::SignedDistance<Mesh<T>, T>{m.rebasedView(a_on)}; });
  };

  // The CSG solid through a tape, traced along rays towards its center.
  const auto tape = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const EBGeometry::Tape<T> t(a_pool, csgExpr<T>());

    return std::make_pair(
      around<T>(t.computeBoundingVolume(), c.size(PairQueries, SmokeQueries), Perf::Seed::RayOrigins),
      [t](const EBGeometry::Pool& a_on) {
        return FirstHit<T, EBGeometry::Tape<T>>{t.rebasedView(a_on), Vec3<T>::zeros()};
      });
  };

  // A million points uniform in the unit cube, the closest to points around it.
  const auto cloud = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const auto points =
      Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), c.size(CloudPoints, SmokeCloud), Perf::Seed::Cloud);
    const Cloud<T> cl(a_pool, points, TargetLeafSize);

    c.param("K", K);
    c.param("W", W);
    c.param("targetLeafSize", TargetLeafSize);

    return std::make_pair(around<T>(Perf::cube(T(0), T(1)), c.size(PairQueries, SmokeQueries), Perf::Seed::FarQueries),
                          [cl](const EBGeometry::Pool& a_on) { return ClosestDistance<T>{cl.rebasedView(a_on)}; });
  };

  addSettings<T>(a_suite, "mesh", "signedDistance", mesh, Perf::agreeDistance<T>);
  addSettings<T>(a_suite, "csg-tape", "firstHit", tape, agreeCrossing<T>);
  addSettings<T>(a_suite, "cloud", "closestPoint", cloud, Perf::agreeDistance<T>);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "LaunchTuning");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
