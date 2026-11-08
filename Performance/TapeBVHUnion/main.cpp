// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for a Tape's union through a BVH asked for with Accelerate::BVH: 64 shapes under point maps
// (boxes rotated and translated, spheres scaled and translated), whose operands the BVH evaluates directly at
// its leaves, against the same union evaluated operand by operand, in float and double, at random points
// around it. A host build runs the host cases; a CUDA or HIP build runs them as host/device pairs. See
// ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Queries per case, at full and at smoke size, and the number of shapes.
constexpr std::size_t HostQueries  = 400000;
constexpr std::size_t PairQueries  = 1048576;
constexpr std::size_t SmokeQueries = 1000;
constexpr std::size_t Shapes       = 64;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// 64 shapes on a 4 x 4 x 4 lattice, every one under maps: boxes turned about z and moved into place, spheres
// scaled and moved into place, in turn.
template <class T>
std::vector<EBGeometry::Expr<T>>
shapes()
{
  std::vector<EBGeometry::Expr<T>> objects;

  for (std::size_t i = 0; i < Shapes; i++) {
    const Vec3<T> place(T(i % 4), T((i / 4) % 4), T(i / 16));

    if (i % 2 == 0) {
      const EBGeometry::BoxSDF<T> box(-T(0.25) * Vec3<T>::ones(), T(0.25) * Vec3<T>::ones());

      objects.push_back(EBGeometry::Translate(EBGeometry::Rotate(box, T(15) * T(i % 6), 2), place));
    }
    else {
      const EBGeometry::SphereSDF<T> sphere(Vec3<T>::zeros(), T(0.2));

      objects.push_back(EBGeometry::Translate(EBGeometry::Scale(sphere, T(1) + T(0.1) * T(i % 3)), place));
    }
  }

  return objects;
}

// A shape's signed distance at a point, as the pairs need it.
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

// Random points in the union's box grown by a quarter of its size on every side.
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

// The cases for one way of evaluating the union: its signed distance on the host and as a host/device pair.
template <class T>
void
addUnion(Perf::Suite& a_suite, const std::string& a_input, const EBGeometry::Accelerate a_accelerate)
{
  using Tape = EBGeometry::Tape<T>;

  const std::string prec = Perf::precisionName<T>();
  const bool        bvh  = (a_accelerate == EBGeometry::Accelerate::BVH);

  const auto make = [=](Perf::Case& c, EBGeometry::Pool& a_pool) {
    // The union as written is large on purpose: no warning.
    EBGeometry::TapeOptions options;

    options.m_largeUnionWarning = 0;

    const Tape tape(a_pool, EBGeometry::Union(shapes<T>(), a_accelerate), options);

    c.param("shapes", Shapes);
    c.param("bvhUnions", tape.getNumBVHUnions());
    c.param("clauses", tape.getNumClauses());

    // The same answers as through the BVH, operand by operand.
    if (!bvh) {
      c.relativeTo(Perf::caseName({"TapeBVHUnion", "maps64-bvh", "signedDistance", prec}), "as-written/bvh");
    }

    return tape;
  };

  const auto box = EBGeometry::Union(shapes<T>()).computeBoundingVolume();

  a_suite.add(Perf::caseName({"TapeBVHUnion", a_input, "signedDistance", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Tape     tape   = make(c, pool);
    const auto     points = pointsAround(box, c.size(HostQueries, SmokeQueries));
    std::vector<T> distances;

    Perf::measureHostQuery(c, Distance<T, Tape>{tape}, points, distances);

    c.checksum(Perf::checksumOf(distances));
    c.poolBytes(pool.usedBytes());
  });

  a_suite.add(Perf::caseName({"TapeBVHUnion", a_input, "signedDistance", prec, "hostdevice"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Tape tape   = make(c, pool);
    const auto points = pointsAround(box, c.size(PairQueries, SmokeQueries));

    const auto distances = Perf::measureHostAndDevice<T>(
      c,
      pool,
      points,
      [&](const EBGeometry::Pool& a_pool) { return Distance<T, Tape>{tape.rebasedView(a_pool)}; },
      Perf::agreeDistance<T>);

    c.checksum(Perf::checksumOf(distances));
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "TapeBVHUnion");

  addUnion<float>(suite, "maps64-bvh", EBGeometry::Accelerate::BVH);
  addUnion<float>(suite, "maps64-as-written", EBGeometry::Accelerate::None);
  addUnion<double>(suite, "maps64-bvh", EBGeometry::Accelerate::BVH);
  addUnion<double>(suite, "maps64-as-written", EBGeometry::Accelerate::None);

  return suite.run();
}
