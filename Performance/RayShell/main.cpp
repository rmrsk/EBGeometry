// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for ray tracing a difference of a shell and a box: a shell (Annular) around a mesh, traced
// from the mesh's offset faces (EBGeometry_OffsetRayForms.hpp) or marched only within the box's spans
// (RayDetail::spansWithin() in EBGeometry_RaySpans.hpp), with a corner cut away by the box, as in
// Examples/Shell, through a tape (traced, and marched with RayOptions::m_march, which the traced case names)
// and fused. Every ray starts at a random point in the shell's box grown to twice its size and
// points at the box's centre. A host build runs the host cases; a CUDA or HIP build runs host/device pairs
// in Morton and random order. See ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The mesh's branching factor and leaf width, spelled out (see ../Mesh/main.cpp).
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// The shell's half-thickness, as a fraction of the mesh's box diagonal.
constexpr double HalfThickness = 0.005;

// The numbers of rays, at full and at smoke size: a shell ray costs several of the mesh's signed distances.
constexpr std::size_t HostQueries  = 20000;
constexpr std::size_t PairQueries  = 65536;
constexpr std::size_t SmokeQueries = 500;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

template <class T>
using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

// A tape with the mesh as a primitive type of its own (it is not the default TriMeshSDF in every build), tracing
// offsets and annuli exactly (TapeFeature::ExactOffsets).
template <class T>
using MeshTape = EBGeometry::Tape<T, EBGeometry::TapeFeature::ExactOffsets, Mesh<T>>;

// The ray from a_origin towards a_target.
template <class T>
EBGEOMETRY_HOST_DEVICE
EBGeometry::RayT<T>
rayTowards(const Vec3<T>& a_origin, const Vec3<T>& a_target) noexcept
{
  const Vec3<T> d = a_target - a_origin;

  return EBGeometry::RayT<T>{a_origin, d / d.length()};
}

// Where the ray from a point towards the target first crosses the shape (-1 for no crossing), traced, or
// marched if m_march.
template <class T, class Shape>
struct FirstHit
{
  Shape   m_shape;
  Vec3<T> m_target;
  bool    m_march;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    EBGeometry::RayOptions<T> options;

    options.m_march = m_march;

    const EBGeometry::HitT<T> hit = EBGeometry::firstHit(
      m_shape, rayTowards(a_origin, m_target), T(0), EBGeometry::Math::Limits<T>::infinity(), options);

    return hit.hit() ? hit.m_t : -T(1);
  }
};

// Whether a host and a device crossing agree: both miss, or both cross within a_tolerance, the march's
// (twice RayOptions::m_tolerance times the box diagonal; the armadillo is about 230 across). A ray grazing
// the shell may come within the tolerance on one side and not the other, and is counted as differing.
template <class T>
std::pair<double, bool>
agreeCrossing(T a_host, T a_device, double a_tolerance)
{
  const double diff = std::abs(static_cast<double>(a_host) - static_cast<double>(a_device));

  return {diff, (a_host < T(0)) != (a_device < T(0)) || diff > a_tolerance};
}

// Ray origins in the box grown to twice its size about its centre.
template <class T>
std::vector<Vec3<T>>
origins(const AABB<T>& a_box, std::size_t a_n)
{
  const Vec3<T> centre = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const Vec3<T> half   = a_box.getHighCorner() - centre;

  return Perf::pointsInBox<T>(AABB<T>(centre - T(2) * half, centre + T(2) * half), a_n, Perf::Seed::RayOrigins);
}

// The mesh (the armadillo, the dodecahedron under --smoke), its shell's half-thickness, and the box cutting
// away the corner above its centre in x and below it in z.
template <class T>
struct Shell
{
  Mesh<T>               mesh;
  T                     delta;
  EBGeometry::BoxSDF<T> corner;

  Shell(Perf::Case& a_case, EBGeometry::Pool& a_pool)
    : mesh(EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(Perf::meshFile(a_case.smoke()), a_pool)),
      delta(T(0)),
      corner(Vec3<T>::zeros(), Vec3<T>::ones())
  {
    const AABB<T>  box    = mesh.computeBoundingVolume();
    const Vec3<T>& lo     = box.getLowCorner();
    const Vec3<T>& hi     = box.getHighCorner();
    const Vec3<T>  centre = T(0.5) * (lo + hi);
    const Vec3<T>  margin = hi - lo;

    delta  = T(HalfThickness) * (hi - lo).length();
    corner = EBGeometry::BoxSDF<T>(Vec3<T>(centre[0], lo[1] - margin[1], lo[2] - margin[2]),
                                   Vec3<T>(hi[0] + margin[0], hi[1] + margin[1], centre[2]));

    a_case.param("K", K);
    a_case.param("W", W);
  }
};

// One shape, traced or marched, in one precision: the host case, and host/device pairs in each order.
// a_build(case, pool) builds the shape in the pool. A traced tape case names its marched one.
template <class T, class Build>
void
addShape(Perf::Suite& a_suite, const std::string& a_input, const bool a_march, Build a_build, const bool a_marched)
{
  const std::string prec      = Perf::precisionName<T>();
  const std::string operation = a_march ? "firstHit-marched" : "firstHit";

  a_suite.add(Perf::caseName({"RayShell", a_input, operation, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto     shape  = a_build(c, pool);
    const auto     box    = shape.computeBoundingVolume();
    const Vec3<T>  centre = T(0.5) * (box.getLowCorner() + box.getHighCorner());
    const auto     q      = origins<T>(box, c.size(HostQueries, SmokeQueries));
    std::vector<T> out;

    Perf::measureHostQuery(c, FirstHit<T, std::decay_t<decltype(shape)>>{shape, centre, a_march}, q, out);

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());
    c.param("march", a_march ? 1 : 0);

    if (!a_march && a_marched) {
      c.relativeTo(Perf::caseName({"RayShell", a_input, "firstHit-marched", prec, "host"}), "marched/traced");
    }
  });

  for (const Perf::Order order : Perf::Orders) {
    const std::string ordered = operation + "-" + Perf::orderName(order);

    a_suite.add(Perf::caseName({"RayShell", a_input, ordered, prec, "hostdevice"}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const auto    shape  = a_build(c, pool);
      const auto    box    = shape.computeBoundingVolume();
      const Vec3<T> centre = T(0.5) * (box.getLowCorner() + box.getHighCorner());
      const auto    q      = Perf::inOrder(order, origins<T>(box, c.size(PairQueries, SmokeQueries)));

      using S = std::decay_t<decltype(shape)>;

      const double tolerance = 2.0 * double(EBGeometry::RayOptions<T>().m_tolerance) *
                               double((box.getHighCorner() - box.getLowCorner()).length());

      const auto out = Perf::measureHostAndDevice<T>(
        c,
        pool,
        q,
        [&](const EBGeometry::Pool& a_pool) { return FirstHit<T, S>{shape.rebasedView(a_pool), centre, a_march}; },
        [tolerance](T a_host, T a_device) { return agreeCrossing<T>(a_host, a_device, tolerance); });

      c.checksum(Perf::checksumOf(out));
      c.param("march", a_march ? 1 : 0);

      if (!a_march && a_marched) {
        c.relativeTo(
          Perf::caseName(
            {"RayShell", a_input, std::string("firstHit-marched-") + Perf::orderName(order), prec, "hostdevice"}),
          "marched/traced");
      }
    });
  }
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  const auto tape = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Shell<T> s(c, a_pool);

    return MeshTape<T>(a_pool, EBGeometry::Difference(EBGeometry::Annular(s.mesh, s.delta), s.corner));
  };

  const auto fused = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Shell<T> s(c, a_pool);

    return EBGeometry::Fused::Difference(EBGeometry::Fused::Annular(s.mesh, s.delta), s.corner);
  };

  addShape<T>(a_suite, "armadillo-tape", false, tape, true);
  addShape<T>(a_suite, "armadillo-tape", true, tape, true);
  addShape<T>(a_suite, "armadillo-fused", false, fused, false);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "RayShell");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
