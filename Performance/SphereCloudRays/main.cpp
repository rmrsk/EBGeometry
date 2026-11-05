// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for SphereCloudBVH's ray form, over 500,000 spheres of differing radii (centers uniform in the
// unit cube), in float and double: the first crossing of rays starting at random points in the cloud's box, in a
// random direction, many of them inside a sphere. On the cloud alone ("polydisperse"), and on a box with the spheres
// cut out of it, what a particle code's geometry is: through a tape listing the cloud ("bed-in-box"), and fused
// ("bed-in-box-fused"), whose cases name the tape's for the ratio of their times. A host build runs the host
// cases; a CUDA or HIP build runs host/device pairs in Morton and random order of the starts. The cases live apart
// from Performance/SphereCloudBVH so that they cannot move the inlining of its cases. See ../README.md and
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

// The parameters that select code, spelled out rather than taken from the defaults: the branching factor, the
// group width and the target spheres per leaf (its default, 16 W).
constexpr std::size_t K              = 4;
constexpr std::size_t W              = 4;
constexpr std::size_t TargetLeafSize = 16 * W;

// The cloud's size, and the number of rays, at full and at smoke size.
constexpr std::size_t CloudSpheres      = 500000;
constexpr std::size_t SmokeCloudSpheres = 2000;
constexpr std::size_t HostRays          = 100000;
constexpr std::size_t PairRays          = 262144;
constexpr std::size_t SmokeRays         = 500;

// How far a ray is followed: past the whole box, so that a ray finds its crossing or leaves the cloud.
constexpr double RayLength = 2.0;

template <class T>
using Cloud = EBGeometry::SphereCloudBVH<T, K, W>;

template <class T>
using CloudTape = EBGeometry::Tape<T, Cloud<T>>;

template <class T>
using Sphere = EBGeometry::SphereCloud::Sphere<T>;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

// The spheres, as Performance/SphereCloudBVH draws them: centers uniform in the unit cube, radii uniform in
// [0.002, 0.006].
template <class T>
std::vector<Sphere<T>>
spheres(const std::size_t a_n)
{
  const auto centers = Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), a_n, Perf::Seed::Cloud);
  const auto radii   = Perf::pointsInBox<T>(Perf::cube(T(0.002), T(0.006)), a_n, Perf::Seed::SphereRadii);

  std::vector<Sphere<T>> result(a_n);

  for (std::size_t i = 0; i < a_n; i++) {
    result[i] = Sphere<T>{centers[i], radii[i][0]};
  }

  return result;
}

// The rays: starts uniform in the unit cube (Morton or random order), each with a random unit direction.
template <class T>
std::vector<EBGeometry::RayT<T>>
rays(const std::size_t a_n, const Perf::Order a_order)
{
  const auto starts = Perf::inOrder(a_order, Perf::pointsInBox<T>(Perf::cube(T(0), T(1)), a_n, Perf::Seed::RayOrigins));
  const auto directions = Perf::pointsInBox<T>(Perf::cube(T(-1), T(1)), a_n, Perf::Seed::ParticleDirs);

  std::vector<EBGeometry::RayT<T>> result(a_n);

  for (std::size_t i = 0; i < a_n; i++) {
    const T length = directions[i].length();

    result[i] = EBGeometry::RayT<T>{starts[i], (length > T(0)) ? directions[i] / length : Vec3<T>(T(1), T(0), T(0))};
  }

  return result;
}

// What one ray's query gives: the distance to its first crossing (-1 for none) and the primitive there.
template <class T>
struct Crossing
{
  T             t;
  std::uint32_t primitive;
};

template <class T, class Shape>
struct FirstHit
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  Crossing<T>
  operator()(const EBGeometry::RayT<T>& a_ray) const noexcept
  {
    const EBGeometry::HitT<T> hit = EBGeometry::firstHit(m_shape, a_ray, T(0), T(RayLength));

    return hit.hit() ? Crossing<T>{hit.m_t, hit.m_primitive} : Crossing<T>{-T(1), 0U};
  }
};

// Whether a host and a device crossing agree: both miss, or both cross the same primitive within
// agreementTolerance(). The crossings are exact, so a ray grazing a sphere is the only way the sides can differ.
template <class T>
std::pair<double, bool>
agreeCrossing(const Crossing<T>& a_host, const Crossing<T>& a_device)
{
  const double diff = std::abs(static_cast<double>(a_host.t) - static_cast<double>(a_device.t));

  return {diff,
          (a_host.t < T(0)) != (a_device.t < T(0)) || a_host.primitive != a_device.primitive ||
            diff > Perf::agreementTolerance<T>(1.0)};
}

// A case's checksums: the sum of the distances, and an exact hash of the primitives crossed.
template <class T>
void
record(Perf::Case& a_case, const std::vector<Crossing<T>>& a_results)
{
  std::vector<T>      distances;
  Perf::IndexChecksum primitives;

  distances.reserve(a_results.size());

  for (const auto& r : a_results) {
    distances.push_back(r.t);
    primitives.add(r.primitive);
  }

  a_case.checksum(Perf::checksumOf(distances));
  a_case.indexChecksum(primitives.value());
}

// One input, in one precision: the host case, and host/device pairs in each order. A_build(pool, cloud) builds the
// shape the rays query, in the pool the cloud is in. A case names the same case of a_tape, if given, for the ratio of
// their times (the tape's over this one's).
template <class T, class Build>
void
addInput(Perf::Suite& a_suite, const std::string& a_input, Build a_build, const std::string& a_tape = "")
{
  const std::string prefix = "SphereCloudRays/" + a_input;
  const std::string prec   = Perf::precisionName<T>();

  const auto cloud = [](Perf::Case& c) {
    const std::size_t n = c.size(CloudSpheres, SmokeCloudSpheres);

    c.param("spheres", n);
    c.param("K", K);
    c.param("W", W);
    c.param("targetLeafSize", TargetLeafSize);

    return spheres<T>(n);
  };

  a_suite.add(Perf::caseName({prefix, "firstHit", prec, "host"}), [=](Perf::Case& c) {
    const auto               s = cloud(c);
    EBGeometry::Pool         pool(EBGeometry::hostMemoryResource());
    const Cloud<T>           bvh(pool, s, TargetLeafSize);
    const auto               shape = a_build(pool, bvh);
    std::vector<Crossing<T>> results;

    using S = std::decay_t<decltype(shape)>;

    Perf::measureHostQuery(
      c, FirstHit<T, S>{shape}, rays<T>(c.size(HostRays, SmokeRays), Perf::Order::Random), results);
    c.poolBytes(pool.usedBytes());
    record(c, results);

    if (!a_tape.empty()) {
      c.relativeTo(Perf::caseName({"SphereCloudRays/" + a_tape, "firstHit", prec, "host"}), "tape/fused");
    }
  });

  for (const Perf::Order order : Perf::Orders) {
    a_suite.add(
      Perf::caseName({prefix, std::string("firstHit-") + Perf::orderName(order), prec, "hostdevice"}),
      [=](Perf::Case& c) {
        const auto       s = cloud(c);
        EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
        const Cloud<T>   bvh(pool, s, TargetLeafSize);
        const auto       shape = a_build(pool, bvh);

        using S = std::decay_t<decltype(shape)>;

        c.poolBytes(pool.usedBytes());

        const auto results = Perf::measureHostAndDevice<Crossing<T>>(
          c,
          pool,
          rays<T>(c.size(PairRays, SmokeRays), order),
          [&](const EBGeometry::Pool& a_pool) { return FirstHit<T, S>{shape.rebasedView(a_pool)}; },
          agreeCrossing<T>);

        record(c, results);

        if (!a_tape.empty()) {
          c.relativeTo(
            Perf::caseName(
              {"SphereCloudRays/" + a_tape, std::string("firstHit-") + Perf::orderName(order), prec, "hostdevice"}),
            "tape/fused");
        }
      });
  }
}

template <class T>
void
addCases(Perf::Suite& a_suite)
{
  addInput<T>(a_suite, "polydisperse", [](EBGeometry::Pool&, const Cloud<T>& a_cloud) { return a_cloud; });

  // The box around the cloud with the spheres cut out of it: fused, and through a tape listing the cloud as a
  // primitive.
  const EBGeometry::BoxSDF<T> box(Vec3<T>(T(-0.05), T(-0.05), T(-0.05)), Vec3<T>(T(1.05), T(1.05), T(1.05)));

  addInput<T>(
    a_suite,
    "bed-in-box-fused",
    [box](EBGeometry::Pool&, const Cloud<T>& a_cloud) { return EBGeometry::Fused::Difference(box, a_cloud); },
    "bed-in-box");

  addInput<T>(a_suite, "bed-in-box", [box](EBGeometry::Pool& a_pool, const Cloud<T>& a_cloud) {
    return CloudTape<T>(a_pool, EBGeometry::Difference(EBGeometry::Expr<T>(box), EBGeometry::Expr<T>(a_cloud)));
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "SphereCloudRays");

  addCases<float>(suite);
  addCases<double>(suite);

  return suite.run();
}
