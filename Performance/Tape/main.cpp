// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for Tape's signed distance: the classic CSG solid compiled into a
// tape and the same solid written by hand, a tape of smooth unions, and a union of many spheres evaluated
// through a BVH, in float and double, at random points in each object's box. Two pairs of cases take the
// interpreter's cost apart: a single sphere as a tape and called directly (the fixed cost of one
// evaluation), and a union of 16 spheres evaluated clause by clause and as a hand-written loop (the cost
// per clause). A host build runs the host cases; a CUDA or HIP build runs them as host/device pairs, which
// show the gap between a tape and the hand-written solid on a device. See ../README.md and
// PerformanceTesting.rst.

#include <cstddef>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Queries per case, at full and at smoke size, and the number of spheres in the union (above the tape's
// BVH threshold, TapeOptions::m_bvhUnionThreshold, spelled out).
constexpr std::size_t HostQueries     = 400000;
constexpr std::size_t PairQueries     = 1048576;
constexpr std::size_t SmokeQueries    = 1000;
constexpr std::size_t UnionSpheres    = 64;
constexpr std::size_t BVHUnionMinimum = 8;

// The spheres of the union evaluated clause by clause, and the BVH threshold above that number which keeps
// the tape from evaluating it through a BVH.
constexpr std::size_t ChainSpheres = 16;
constexpr std::size_t NoBVHUnion   = 1000;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// The classic CSG solid: a cube with its corners cut by a ball, drilled through along x, y and z.
template <class T>
EBGeometry::Expr<T>
classicCSG()
{
  const EBGeometry::Expr<T> cube = EBGeometry::BoxSDF<T>(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1)));
  const EBGeometry::Expr<T> ball = EBGeometry::SphereSDF<T>(Vec3<T>::zeros(), T(1.35));

  std::vector<EBGeometry::Expr<T>> drills;

  for (std::size_t axis = 0; axis < 3; axis++) {
    drills.push_back(EBGeometry::InfiniteCylinderSDF<T>(Vec3<T>::zeros(), T(0.55), axis));
  }

  return EBGeometry::Difference(EBGeometry::Intersection(cube, ball), EBGeometry::Union(drills));
}

// The same solid written by hand: the reference a tape is compared with.
template <class T>
struct DirectCSG
{
  EBGeometry::BoxSDF<T>              m_box;
  EBGeometry::SphereSDF<T>           m_ball;
  EBGeometry::InfiniteCylinderSDF<T> m_drill0;
  EBGeometry::InfiniteCylinderSDF<T> m_drill1;
  EBGeometry::InfiniteCylinderSDF<T> m_drill2;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3<T>& a_point) const noexcept
  {
    namespace Math = EBGeometry::Math;

    const T body  = Math::max(m_box.signedDistance(a_point), m_ball.signedDistance(a_point));
    const T holes = Math::min(m_drill0.signedDistance(a_point),
                              Math::min(m_drill1.signedDistance(a_point), m_drill2.signedDistance(a_point)));

    return Math::max(body, -holes);
  }

  // Not a pool handle: rebasing changes nothing.
  [[nodiscard]] DirectCSG
  rebasedView(const EBGeometry::Pool& /*a_pool*/) const noexcept
  {
    return *this;
  }
};

// One sphere called directly: the reference for a one-clause tape.
template <class T>
struct DirectSphere
{
  EBGeometry::SphereSDF<T> m_sphere;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3<T>& a_point) const noexcept
  {
    return m_sphere.signedDistance(a_point);
  }

  // Not a pool handle: rebasing changes nothing.
  [[nodiscard]] DirectSphere
  rebasedView(const EBGeometry::Pool& /*a_pool*/) const noexcept
  {
    return *this;
  }
};

// The ChainSpheres spheres of chainOfSpheres(), as a hand-written loop: the reference for a tape that
// evaluates them clause by clause.
template <class T>
struct DirectChain
{
  EBGeometry::Array<EBGeometry::SphereSDF<T>, ChainSpheres> m_spheres;

  EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3<T>& a_point) const noexcept
  {
    T d = m_spheres[0].signedDistance(a_point);

    for (std::size_t i = 1; i < ChainSpheres; i++) {
      d = EBGeometry::Math::min(d, m_spheres[i].signedDistance(a_point));
    }

    return d;
  }

  // Not a pool handle: rebasing changes nothing.
  [[nodiscard]] DirectChain
  rebasedView(const EBGeometry::Pool& /*a_pool*/) const noexcept
  {
    return *this;
  }
};

// Sphere i of the chain: ChainSpheres spheres in a row along x, overlapping their neighbours.
template <class T>
EBGeometry::SphereSDF<T>
chainSphere(std::size_t a_index)
{
  return EBGeometry::SphereSDF<T>(Vec3<T>(T(0.3) * T(a_index) - T(2.25), T(0.1) * T(a_index % 3), T(0)), T(0.4));
}

template <class T>
EBGeometry::Expr<T>
chainOfSpheres()
{
  std::vector<EBGeometry::Expr<T>> spheres;

  for (std::size_t i = 0; i < ChainSpheres; i++) {
    spheres.push_back(chainSphere<T>(i));
  }

  return EBGeometry::Union(spheres);
}

template <class T>
DirectChain<T>
directChain()
{
  DirectChain<T> chain;

  for (std::size_t i = 0; i < ChainSpheres; i++) {
    chain.m_spheres[i] = chainSphere<T>(i);
  }

  return chain;
}

template <class T>
DirectCSG<T>
directCSG()
{
  return DirectCSG<T>{EBGeometry::BoxSDF<T>(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1))),
                      EBGeometry::SphereSDF<T>(Vec3<T>::zeros(), T(1.35)),
                      EBGeometry::InfiniteCylinderSDF<T>(Vec3<T>::zeros(), T(0.55), 0),
                      EBGeometry::InfiniteCylinderSDF<T>(Vec3<T>::zeros(), T(0.55), 1),
                      EBGeometry::InfiniteCylinderSDF<T>(Vec3<T>::zeros(), T(0.55), 2)};
}

// A smooth object: two smooth unions of spheres, smoothly cut by a box.
template <class T>
EBGeometry::Expr<T>
smoothBlobs()
{
  std::vector<EBGeometry::Expr<T>> blobs;

  for (int i = 0; i < 6; i++) {
    blobs.push_back(EBGeometry::SphereSDF<T>(Vec3<T>(T(0.4) * T(i % 3) - T(0.4), T(0.5) * T(i / 3), T(0)), T(0.35)));
  }

  return EBGeometry::SmoothDifference(
    EBGeometry::SmoothUnion(blobs, T(0.15)),
    EBGeometry::BoxSDF<T>(Vec3<T>(T(-1), T(0.3), T(-0.1)), Vec3<T>(T(1), T(0.4), T(0.1))),
    T(0.05));
}

// UnionSpheres spheres on a 4 x 4 x 4 lattice: one union, evaluated through a BVH.
template <class T>
EBGeometry::Expr<T>
manySpheres()
{
  std::vector<EBGeometry::Expr<T>> spheres;

  for (std::size_t i = 0; i < UnionSpheres; i++) {
    spheres.push_back(
      EBGeometry::SphereSDF<T>(Vec3<T>(T(i % 4), T((i / 4) % 4), T(i / 16)), T(0.3) + T(0.05) * T(i % 3)));
  }

  return EBGeometry::Union(spheres);
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

// Evaluations per query in the repeated cases: they tell a cost paid once per query (per device thread)
// from one paid on every evaluation.
constexpr std::size_t Repeats = 16;

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

  a_suite.add(Perf::caseName({"Tape", a_input, operation, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Shape    shape  = a_make(c, pool);
    const auto     points = pointsAround(a_box, c.size(HostQueries, SmokeQueries));
    std::vector<T> distances;

    Perf::measureHostQuery(c, Distance<T, Shape, R>{shape}, points, distances);

    c.checksum(Perf::checksumOf(distances));
    c.poolBytes(pool.usedBytes());
    c.param("repeats", R);
  });

  a_suite.add(Perf::caseName({"Tape", a_input, operation, prec, "hostdevice"}), [=](Perf::Case& c) {
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
  });
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  using Tape = EBGeometry::Tape<T>;
  using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

  EBGeometry::TapeOptions options;

  options.m_bvhUnionThreshold = BVHUnionMinimum;

  const auto tapeWith = [](const EBGeometry::Expr<T>& a_expr, const EBGeometry::TapeOptions& a_options) {
    return [a_options, a_expr](Perf::Case& c, EBGeometry::Pool& a_pool) {
      const Tape tape(a_pool, a_expr, a_options);

      c.param("clauses", tape.getNumClauses());
      c.param("bvhUnionMinimum", a_options.m_bvhUnionThreshold);

      return tape;
    };
  };

  const auto tapeOf = [&tapeWith, options](const EBGeometry::Expr<T>& a_expr) { return tapeWith(a_expr, options); };

  EBGeometry::TapeOptions linear;

  linear.m_bvhUnionThreshold = NoBVHUnion;

  const AABB cube(Vec3<T>(T(-1), T(-1), T(-1)), Vec3<T>(T(1), T(1), T(1)));

  addObject<T, Tape>(a_suite, "csg", cube, tapeOf(classicCSG<T>()));
  addObject<T, DirectCSG<T>>(
    a_suite, "csg-handwritten", cube, [](Perf::Case&, EBGeometry::Pool&) { return directCSG<T>(); });
  addObject<T, Tape>(a_suite, "smooth", smoothBlobs<T>().computeBoundingVolume(), tapeOf(smoothBlobs<T>()));
  addObject<T, Tape>(a_suite, "union64", manySpheres<T>().computeBoundingVolume(), tapeOf(manySpheres<T>()));

  // The interpreter's cost taken apart: per evaluation, and per clause.
  const EBGeometry::SphereSDF<T> sphere(Vec3<T>::zeros(), T(1));

  addObject<T, Tape>(a_suite, "sphere", sphere.computeBoundingVolume(), tapeOf(EBGeometry::Expr<T>(sphere)));
  addObject<T, DirectSphere<T>>(a_suite,
                                "sphere-handwritten",
                                sphere.computeBoundingVolume(),
                                [sphere](Perf::Case&, EBGeometry::Pool&) { return DirectSphere<T>{sphere}; });

  // The same sphere evaluated Repeats times per query.
  addObject<T, Tape, Repeats>(a_suite, "sphere", sphere.computeBoundingVolume(), tapeOf(EBGeometry::Expr<T>(sphere)));
  addObject<T, DirectSphere<T>, Repeats>(a_suite,
                                         "sphere-handwritten",
                                         sphere.computeBoundingVolume(),
                                         [sphere](Perf::Case&, EBGeometry::Pool&) { return DirectSphere<T>{sphere}; });

  const AABB chainBox = chainOfSpheres<T>().computeBoundingVolume();

  addObject<T, Tape>(a_suite, "chain16", chainBox, tapeWith(chainOfSpheres<T>(), linear));
  addObject<T, DirectChain<T>>(
    a_suite, "chain16-handwritten", chainBox, [](Perf::Case&, EBGeometry::Pool&) { return directChain<T>(); });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "Tape");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
