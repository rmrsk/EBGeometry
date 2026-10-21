// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for BVHUnion: unions of about 100,000 objects on a lattice, in float and double, in
// three families: spheres (well-filled boxes that do not overlap), capsules along the diagonal (the worst
// case for an axis-aligned box: 5% filled, heavily overlapping) and randomly oriented finite cylinders (a
// dearer distance and a mix of fills). A host build runs the host cases (build, signed distance); a CUDA
// or HIP build runs the signed distance as host/device pairs. See ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The branching factor, spelled out (see ../PointCloud/main.cpp).
constexpr std::size_t K = 4;

// The lattice's side (46^3 = 97,336 objects) and the numbers of queries, at full and at smoke size.
constexpr std::size_t Lattice      = 46;
constexpr std::size_t SmokeLattice = 6;
constexpr std::size_t HostQueries  = 200000;
constexpr std::size_t PairQueries  = 262144;
constexpr std::size_t SmokeQueries = 1000;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// Call a_f(i, j, k) for every point of an n x n x n lattice.
template <class F>
void
forLattice(std::size_t a_n, F&& a_f)
{
  for (std::size_t i = 0; i < a_n; i++) {
    for (std::size_t j = 0; j < a_n; j++) {
      for (std::size_t k = 0; k < a_n; k++) {
        a_f(i, j, k);
      }
    }
  }
}

// Unit spheres a radius apart (spacing 3): a sphere fills about half its box (pi/6), and the boxes do not
// overlap.
template <class T>
void
spheres(std::size_t a_n, std::vector<EBGeometry::SphereSDF<T>>& a_shapes)
{
  forLattice(a_n, [&](std::size_t i, std::size_t j, std::size_t k) {
    a_shapes.emplace_back(T(3) * Vec3<T>(T(i), T(j), T(k)), T(1));
  });
}

// Capsules 3 long (tip to tip), of radius 0.2, along the diagonal (1,1,1), at spacing 1.8: the worst case
// for an axis-aligned box. A capsule fills about 5% of its box, and every box overlaps its neighbours',
// so a query point lies in many leaf boxes. The capsules themselves do not touch (those on one diagonal
// are sqrt(3) * 1.8 = 3.1 apart along it).
template <class T>
void
diagonalCapsules(std::size_t a_n, std::vector<EBGeometry::CapsuleSDF<T>>& a_shapes)
{
  const Vec3<T> half = T(1.5) / std::sqrt(T(3)) * Vec3<T>(T(1), T(1), T(1));

  forLattice(a_n, [&](std::size_t i, std::size_t j, std::size_t k) {
    const Vec3<T> center = T(1.8) * Vec3<T>(T(i), T(j), T(k));

    a_shapes.emplace_back(center - half, center + half, T(0.2));
  });
}

// Finite cylinders 3 long, of radius 0.2, in directions drawn from a fixed seed, at the same spacing: a
// dearer distance than a capsule's (two flat caps), and a mix of box fills, as in a geometry of fibres.
// Neighbours intersect, which a union handles; the density matches the capsules'.
template <class T>
void
randomCylinders(std::size_t a_n, std::vector<EBGeometry::CylinderSDF<T>>& a_shapes)
{
  const auto  directions = Perf::pointsInBox<T>(Perf::cube(T(-1), T(1)), a_n * a_n * a_n, Perf::Seed::CylinderAxes);
  std::size_t d          = 0;

  forLattice(a_n, [&](std::size_t i, std::size_t j, std::size_t k) {
    const Vec3<T> center = T(1.8) * Vec3<T>(T(i), T(j), T(k));
    const Vec3<T> half   = T(1.5) * directions[d] / directions[d].length();

    a_shapes.emplace_back(center - half, center + half, T(0.2));
    d++;
  });
}

// Every case for one family of objects in one precision: the union's build, its signed distance on the
// host, and the same as host/device pairs. a_objects(n, objects) appends the family's objects for an
// n x n x n lattice.
template <class T, class Shape, class Objects>
void
addFamily(Perf::Suite& a_suite, const std::string& a_family, Objects a_objects)
{
  using AABB  = EBGeometry::BoundingVolumes::AABBT<T>;
  using Union = EBGeometry::BVHUnion<T, Shape, K>;

  const std::string prefix = Perf::caseName({"BVHUnion", a_family});
  const std::string prec   = Perf::precisionName<T>();

  // The family's objects and their boxes, recorded as the case's parameters.
  const auto objects = [=](Perf::Case& c, std::vector<Shape>& a_shapes, std::vector<AABB>& a_boxes) {
    a_objects(c.size(Lattice, SmokeLattice), a_shapes);

    for (const auto& shape : a_shapes) {
      a_boxes.push_back(shape.computeBoundingVolume());
    }

    c.param("objects", a_shapes.size());
    c.param("K", K);
  };

  // The union, built in a_pool, and random points in its box.
  const auto scene = [=](Perf::Case& c, EBGeometry::Pool& a_pool, std::size_t a_queries) {
    std::vector<Shape> shapes;
    std::vector<AABB>  boxes;

    objects(c, shapes, boxes);

    Union u(a_pool, shapes, boxes);

    return std::make_pair(u, Perf::pointsInBox<T>(u.computeBoundingVolume(), a_queries, Perf::Seed::UnionQueries));
  };

  a_suite.add(Perf::caseName({prefix, "build", prec, "host"}), [=](Perf::Case& c) {
    std::vector<Shape> shapes;
    std::vector<AABB>  boxes;

    objects(c, shapes, boxes);

    const auto built =
      Perf::measureBuild<Union>(c, [&](EBGeometry::Pool& a_pool) { return Union(a_pool, shapes, boxes); });

    c.poolBytes(built.pool->usedBytes());
    c.checksum(static_cast<double>(built.object->signedDistance(Vec3<T>(T(0.5), T(0.5), T(0.5)))));
  });

  a_suite.add(Perf::caseName({prefix, "query", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const auto       s = scene(c, pool, c.size(HostQueries, SmokeQueries));
    std::vector<T>   distances;

    Perf::measureHostQuery(c, Perf::SignedDistance<Union, T>{s.first}, s.second, distances);

    c.checksum(Perf::checksumOf(distances));
    c.poolBytes(pool.usedBytes());
  });

  for (const Perf::Order order : Perf::Orders) {
    a_suite.add(
      Perf::caseName({prefix, std::string("query-") + Perf::orderName(order), prec, "hostdevice"}), [=](Perf::Case& c) {
        EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
        const auto       s = scene(c, pool, c.size(PairQueries, SmokeQueries));
        const Union&     u = s.first;

        const auto distances = Perf::measureHostAndDevice<T>(
          c,
          pool,
          Perf::inOrder(order, s.second),
          [&](const EBGeometry::Pool& a_pool) { return Perf::SignedDistance<Union, T>{u.rebasedView(a_pool)}; },
          Perf::agreeDistance<T>);

        c.checksum(Perf::checksumOf(distances));
      });
  }
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "BVHUnion");

  // Family by family, both precisions of each together.
  addFamily<float, EBGeometry::SphereSDF<float>>(suite, "spheres", spheres<float>);
  addFamily<double, EBGeometry::SphereSDF<double>>(suite, "spheres", spheres<double>);
  addFamily<float, EBGeometry::CapsuleSDF<float>>(suite, "capsules-diagonal", diagonalCapsules<float>);
  addFamily<double, EBGeometry::CapsuleSDF<double>>(suite, "capsules-diagonal", diagonalCapsules<double>);
  addFamily<float, EBGeometry::CylinderSDF<float>>(suite, "cylinders-random", randomCylinders<float>);
  addFamily<double, EBGeometry::CylinderSDF<double>>(suite, "cylinders-random", randomCylinders<double>);

  return suite.run();
}
