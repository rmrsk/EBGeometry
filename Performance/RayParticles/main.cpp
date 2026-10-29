// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for particles colliding with a boundary: one short step per particle, the first crossing
// of the boundary on it (firstHit() over the step's length). The boundaries are a shell (Annular) around a
// mesh, as in Examples/Shell, whose level sets the ray traces exactly (EBGeometry_OffsetRayForms.hpp), through
// a tape (traced, and marched with RayOptions::m_march, which the traced case names) and fused; the solid
// mesh, for the cost of a mesh's own crossing; and a box's shell through a tape, for an analytic shape's.
// Each particle starts at a random point outside the boundary, in its box grown by a tenth of its diagonal,
// and moves a hundredth of the diagonal in a random direction. A host build runs the host cases; a CUDA or
// HIP build runs host/device pairs in Morton and random order of the starts. See ../README.md and
// PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The mesh's branching factor and leaf width, spelled out (see ../Mesh/main.cpp).
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

// The shell's half-thickness, and a step's length, as fractions of the boundary's box diagonal.
constexpr double HalfThickness = 0.005;
constexpr double StepLength    = 0.01;

// The numbers of particles, at full and at smoke size.
constexpr std::size_t HostQueries  = 50000;
constexpr std::size_t PairQueries  = 262144;
constexpr std::size_t SmokeQueries = 500;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

template <class T>
using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

// A tape with the mesh as a primitive type of its own (it is not the default TriMeshSDF in every build).
template <class T>
using MeshTape = EBGeometry::Tape<T, Mesh<T>>;

// One particle's step: where it starts, and its unit direction.
template <class T>
struct Step
{
  Vec3<T> m_origin;
  Vec3<T> m_direction;
};

// Where a step first crosses the boundary (-1 for no crossing), traced, or marched if m_march.
template <class T, class Shape>
struct Collide
{
  Shape m_shape;
  T     m_length;
  bool  m_march;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Step<T>& a_step) const noexcept
  {
    EBGeometry::RayOptions<T> options;

    options.m_march = m_march;

    const EBGeometry::HitT<T> hit =
      EBGeometry::firstHit(m_shape, EBGeometry::RayT<T>{a_step.m_origin, a_step.m_direction}, T(0), m_length, options);

    return hit.hit() ? hit.m_t : -T(1);
  }
};

// Whether a host and a device crossing agree: both miss, or both cross within a_tolerance, the march's (twice
// RayOptions::m_tolerance times the box diagonal). A step grazing the boundary may come within the tolerance
// on one side and not the other, and is counted as differing.
template <class T>
std::pair<double, bool>
agreeCrossing(T a_host, T a_device, double a_tolerance)
{
  const double diff = std::abs(static_cast<double>(a_host) - static_cast<double>(a_device));

  return {diff, (a_host < T(0)) != (a_device < T(0)) || diff > a_tolerance};
}

// The particles: starts outside the boundary in its box grown by a tenth of its diagonal (Morton or random
// order), each paired with a random direction.
template <class T, class Shape>
std::vector<Step<T>>
particles(const Shape& a_shape, const std::size_t a_n, const Perf::Order a_order)
{
  const AABB<T> box    = a_shape.computeBoundingVolume();
  const Vec3<T> margin = T(0.1) * (box.getHighCorner() - box.getLowCorner()).length() * Vec3<T>::ones();
  const AABB<T> grown(box.getLowCorner() - margin, box.getHighCorner() + margin);
  const AABB<T> unit(-Vec3<T>::ones(), Vec3<T>::ones());

  // Starts drawn in rounds until enough lie outside; the directions are drawn once.
  std::vector<Vec3<T>> starts;

  for (std::uint64_t round = 0; starts.size() < a_n; round++) {
    for (const Vec3<T>& p : Perf::pointsInBox<T>(grown, a_n, Perf::Seed::Particles + 1000 * round)) {
      if (starts.size() < a_n && a_shape.signedDistance(p) > T(0)) {
        starts.push_back(p);
      }
    }
  }

  const std::vector<Vec3<T>> ordered    = Perf::inOrder(a_order, starts);
  const std::vector<Vec3<T>> directions = Perf::pointsInBox<T>(unit, a_n, Perf::Seed::ParticleDirs);

  std::vector<Step<T>> steps(a_n);

  for (std::size_t i = 0; i < a_n; i++) {
    const T length = directions[i].length();

    steps[i] = Step<T>{ordered[i], (length > T(0)) ? directions[i] / length : Vec3<T>(T(1), T(0), T(0))};
  }

  return steps;
}

// One boundary, traced or marched, in one precision: the host case, and host/device pairs in each order.
// a_build(case, pool) builds the boundary in the pool. A traced case with a marched one names it.
template <class T, class Build>
void
addBoundary(Perf::Suite& a_suite, const std::string& a_input, const bool a_march, Build a_build, const bool a_marched)
{
  const std::string prec      = Perf::precisionName<T>();
  const std::string operation = a_march ? "step-marched" : "step";

  a_suite.add(Perf::caseName({"RayParticles", a_input, operation, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto     shape  = a_build(c, pool);
    const auto     box    = shape.computeBoundingVolume();
    const T        length = T(StepLength) * (box.getHighCorner() - box.getLowCorner()).length();
    const auto     q      = particles<T>(shape, c.size(HostQueries, SmokeQueries), Perf::Order::Random);
    std::vector<T> out;

    Perf::measureHostQuery(c, Collide<T, std::decay_t<decltype(shape)>>{shape, length, a_march}, q, out);

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());
    c.param("march", a_march ? 1 : 0);

    if (!a_march && a_marched) {
      c.relativeTo(Perf::caseName({"RayParticles", a_input, "step-marched", prec, "host"}), "marched/traced");
    }
  });

  for (const Perf::Order order : Perf::Orders) {
    const std::string ordered = operation + "-" + Perf::orderName(order);

    a_suite.add(Perf::caseName({"RayParticles", a_input, ordered, prec, "hostdevice"}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const auto shape  = a_build(c, pool);
      const auto box    = shape.computeBoundingVolume();
      const T    length = T(StepLength) * (box.getHighCorner() - box.getLowCorner()).length();
      const auto q      = particles<T>(shape, c.size(PairQueries, SmokeQueries), order);

      using S = std::decay_t<decltype(shape)>;

      const double tolerance = 2.0 * double(EBGeometry::RayOptions<T>().m_tolerance) *
                               double((box.getHighCorner() - box.getLowCorner()).length());

      const auto out = Perf::measureHostAndDevice<T>(
        c,
        pool,
        q,
        [&](const EBGeometry::Pool& a_pool) { return Collide<T, S>{shape.rebasedView(a_pool), length, a_march}; },
        [tolerance](T a_host, T a_device) { return agreeCrossing<T>(a_host, a_device, tolerance); });

      c.checksum(Perf::checksumOf(out));
      c.param("march", a_march ? 1 : 0);

      if (!a_march && a_marched) {
        c.relativeTo(
          Perf::caseName(
            {"RayParticles", a_input, std::string("step-marched-") + Perf::orderName(order), prec, "hostdevice"}),
          "marched/traced");
      }
    });
  }
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  // The mesh (the armadillo, the dodecahedron under --smoke) and its shell's half-thickness.
  const auto read = [](Perf::Case& c, EBGeometry::Pool& a_pool) {
    c.param("K", K);
    c.param("W", W);

    return EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(Perf::meshFile(c.smoke()), a_pool);
  };

  const auto delta = [](const Mesh<T>& a_mesh) {
    const AABB<T> box = a_mesh.computeBoundingVolume();

    return T(HalfThickness) * (box.getHighCorner() - box.getLowCorner()).length();
  };

  const auto shellTape = [=](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Mesh<T> mesh = read(c, a_pool);

    return MeshTape<T>(a_pool, EBGeometry::Annular(mesh, delta(mesh)));
  };

  const auto shellFused = [=](Perf::Case& c, EBGeometry::Pool& a_pool) {
    const Mesh<T> mesh = read(c, a_pool);

    return EBGeometry::Fused::Annular(mesh, delta(mesh));
  };

  const auto solid = [=](Perf::Case& c, EBGeometry::Pool& a_pool) { return read(c, a_pool); };

  // A box's shell, a tenth of its shortest side thick.
  const auto boxShell = [](Perf::Case&, EBGeometry::Pool& a_pool) {
    const EBGeometry::BoxSDF<T> box(Vec3<T>(T(-1), T(-0.5), T(-0.25)), Vec3<T>(T(1), T(0.5), T(0.25)));

    return EBGeometry::Tape<T>(a_pool, EBGeometry::Annular(box, T(0.025)));
  };

  addBoundary<T>(a_suite, "armadillo-shell-tape", false, shellTape, true);
  addBoundary<T>(a_suite, "armadillo-shell-tape", true, shellTape, true);
  addBoundary<T>(a_suite, "armadillo-shell-fused", false, shellFused, false);
  addBoundary<T>(a_suite, "armadillo", false, solid, false);
  addBoundary<T>(a_suite, "box-shell-tape", false, boxShell, false);
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "RayParticles");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
