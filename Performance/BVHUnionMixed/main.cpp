// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for a BVHUnion of several shape types: about 100,000 objects on a lattice, spheres,
// capsules along the diagonal and randomly oriented finite cylinders interleaved (the three families of
// ../BVHUnion, one type per lattice point in turn), in float and double. A host build runs the host cases
// (build, signed distance); a CUDA or HIP build runs the signed distance as host/device pairs. See
// ../README.md and PerformanceTesting.rst.

#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The branching factor, the construction method and the leaf size, spelled out rather than taken from
// the defaults (see ../PointCloud/main.cpp): SAH, at most K - 1 objects per leaf (the union's default).
constexpr std::size_t K              = 4;
constexpr std::size_t MaxLeafObjects = K - 1;

// The lattice's side (46^3 = 97,336 objects) and the numbers of queries, at full and at smoke size.
constexpr std::size_t Lattice      = 46;
constexpr std::size_t SmokeLattice = 6;
constexpr std::size_t HostQueries  = 200000;
constexpr std::size_t PairQueries  = 262144;
constexpr std::size_t SmokeQueries = 1000;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using Sphere = EBGeometry::SphereSDF<T>;

template <class T>
using Capsule = EBGeometry::CapsuleSDF<T>;

template <class T>
using Cylinder = EBGeometry::CylinderSDF<T>;

template <class T>
using Union = EBGeometry::BVHUnion<T, K, Sphere<T>, Capsule<T>, Cylinder<T>>;

// The objects of an n x n x n lattice at spacing 1.8, one type per point in turn: unit spheres scaled to
// radius 0.8, capsules 3 long of radius 0.2 along the diagonal, and cylinders of the same size in directions
// drawn from a fixed seed.
template <class T>
struct Objects
{
  std::vector<Sphere<T>>   spheres;
  std::vector<Capsule<T>>  capsules;
  std::vector<Cylinder<T>> cylinders;

  explicit Objects(std::size_t a_n)
  {
    const auto    directions = Perf::pointsInBox<T>(Perf::cube(T(-1), T(1)), a_n * a_n * a_n, Perf::Seed::CylinderAxes);
    const Vec3<T> diagonal   = T(1.5) / std::sqrt(T(3)) * Vec3<T>(T(1), T(1), T(1));
    std::size_t   d          = 0;

    for (std::size_t i = 0; i < a_n; i++) {
      for (std::size_t j = 0; j < a_n; j++) {
        for (std::size_t k = 0; k < a_n; k++) {
          const Vec3<T> center = T(1.8) * Vec3<T>(T(i), T(j), T(k));

          switch ((i + j + k) % 3) {
          case 0: {
            spheres.emplace_back(center, T(0.8));

            break;
          }
          case 1: {
            capsules.emplace_back(center - diagonal, center + diagonal, T(0.2));

            break;
          }
          default: {
            const Vec3<T> half = T(1.5) * directions[d] / directions[d].length();

            cylinders.emplace_back(center - half, center + half, T(0.2));

            break;
          }
          }

          d++;
        }
      }
    }
  }

  [[nodiscard]] std::size_t
  size() const
  {
    return spheres.size() + capsules.size() + cylinders.size();
  }
};

// Every case in one precision: the union's build, its signed distance on the host, and the same as
// host/device pairs.
template <class T>
void
addMixed(Perf::Suite& a_suite)
{
  const std::string prefix = Perf::caseName({"BVHUnionMixed", "spheres-capsules-cylinders"});
  const std::string prec   = Perf::precisionName<T>();

  const EBGeometry::BVH::BuildSettings settings{EBGeometry::BVH::Construction::SAH,
                                                EBGeometry::BVH::ConstructionOptions{MaxLeafObjects, 1, {}}};

  // The objects, recorded as the case's parameters.
  const auto objects = [](Perf::Case& c) {
    Objects<T> o(c.size(Lattice, SmokeLattice));

    c.param("objects", o.size());
    c.param("types", std::size_t(3));
    c.param("K", K);
    c.param("maxLeafObjects", MaxLeafObjects);

    return o;
  };

  const auto build = [=](EBGeometry::Pool& a_pool, const Objects<T>& a_o) {
    return Union<T>(a_pool, settings, a_o.spheres, a_o.capsules, a_o.cylinders);
  };

  // The union, built in a_pool, and random points in its box.
  const auto scene = [=](Perf::Case& c, EBGeometry::Pool& a_pool, std::size_t a_queries) {
    const Objects<T> o = objects(c);
    const Union<T>   u = build(a_pool, o);

    return std::make_pair(u, Perf::pointsInBox<T>(u.computeBoundingVolume(), a_queries, Perf::Seed::UnionQueries));
  };

  a_suite.add(Perf::caseName({prefix, "build", prec, "host"}), [=](Perf::Case& c) {
    const Objects<T> o = objects(c);

    const auto built = Perf::measureBuild<Union<T>>(c, [&](EBGeometry::Pool& a_pool) { return build(a_pool, o); });

    c.poolBytes(built.pool->usedBytes());

    // What the build made, checked untimed: the signed distances at a few points in the union's box.
    const Union<T>& u = *built.object;
    const auto q = Perf::pointsInBox<T>(u.computeBoundingVolume(), Perf::BuildCheckPoints, Perf::Seed::BuildChecks);

    c.checksum(Perf::checksumOf(Perf::queryAll(Perf::SignedDistance<Union<T>, T>{u}, q)));
  });

  a_suite.add(Perf::caseName({prefix, "query-random", prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const auto       s = scene(c, pool, c.size(HostQueries, SmokeQueries));
    std::vector<T>   distances;

    Perf::measureHostQuery(c, Perf::SignedDistance<Union<T>, T>{s.first}, s.second, distances);

    c.checksum(Perf::checksumOf(distances));
    c.poolBytes(pool.usedBytes());
  });

  for (const Perf::Order order : Perf::Orders) {
    a_suite.add(
      Perf::caseName({prefix, std::string("query-") + Perf::orderName(order), prec, "hostdevice"}), [=](Perf::Case& c) {
        EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
        const auto       s = scene(c, pool, c.size(PairQueries, SmokeQueries));
        const Union<T>&  u = s.first;

        const auto distances = Perf::measureHostAndDevice<T>(
          c,
          pool,
          Perf::inOrder(order, s.second),
          [&](const EBGeometry::Pool& a_pool) { return Perf::SignedDistance<Union<T>, T>{u.rebasedView(a_pool)}; },
          Perf::agreeDistance<T>);

        c.checksum(Perf::checksumOf(distances));
      });
  }
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "BVHUnionMixed");

  addMixed<float>(suite);
  addMixed<double>(suite);

  return suite.run();
}
