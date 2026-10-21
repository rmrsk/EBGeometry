// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for BVHUnion: the union of about 100,000 spheres on a lattice, in float and double.
// The build runs on the host; the union's signed distance also runs on the device in a CUDA or HIP
// build. See ../README.md.

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// Branching factor, spelled out (see ../PointCloud/main.cpp).
constexpr std::size_t K = 4;

template <class Union, class T>
struct Distance
{
  Union m_union;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const EBGeometry::Vec3T<T>& a_point) const noexcept
  {
    return m_union.signedDistance(a_point);
  }
};

template <class T>
double
sumOf(const std::vector<T>& a_values)
{
  double s = 0.0;

  for (const T v : a_values) {
    s += double(v);
  }

  return s;
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  using Sphere = EBGeometry::SphereSDF<T>;
  using AABB   = EBGeometry::BoundingVolumes::AABBT<T>;
  using Union  = EBGeometry::BVHUnion<T, Sphere, K>;

  const std::string prec   = std::is_same_v<T, float> ? "float" : "double";
  const std::string prefix = "BVHUnion/spheres/";

  // Unit spheres a radius apart on an n x n x n lattice.
  const auto lattice = [](std::size_t a_n, std::vector<Sphere>& a_spheres, std::vector<AABB>& a_boxes) {
    for (std::size_t i = 0; i < a_n; i++) {
      for (std::size_t j = 0; j < a_n; j++) {
        for (std::size_t k = 0; k < a_n; k++) {
          a_spheres.emplace_back(T(3) * EBGeometry::Vec3T<T>(T(i), T(j), T(k)), T(1));
          a_boxes.push_back(a_spheres.back().computeBoundingVolume());
        }
      }
    }
  };

  a_suite.add(prefix + "build/" + prec + "/host", [=](Perf::Case& c) {
    const std::size_t   n = c.size(46, 6);
    std::vector<Sphere> spheres;
    std::vector<AABB>   boxes;

    lattice(n, spheres, boxes);

    std::unique_ptr<EBGeometry::Pool> pool;
    std::unique_ptr<Union>            u;

    c.measureFresh(
      [&] {
        u.reset();
        pool = std::make_unique<EBGeometry::Pool>(EBGeometry::hostMemoryResource());
      },
      [&] { u = std::make_unique<Union>(*pool, spheres, boxes); });

    c.poolBytes(pool->usedBytes());
    c.checksum(double(u->signedDistance(EBGeometry::Vec3T<T>(T(0.5), T(0.5), T(0.5)))));
    c.param("objects", spheres.size());
    c.param("K", K);
  });

  const auto setup = [=](Perf::Case& c, EBGeometry::Pool& a_pool, std::size_t a_queries) {
    std::vector<Sphere> spheres;
    std::vector<AABB>   boxes;

    lattice(c.size(46, 6), spheres, boxes);

    c.param("objects", spheres.size());
    c.param("K", K);

    Union u(a_pool, spheres, boxes);

    return std::make_pair(u, Perf::pointsInBox<T>(u.computeBoundingVolume(), a_queries, 13u));
  };

  a_suite.add(prefix + "query/" + prec + "/host", [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    // Named members, not a structured binding: C++17 does not let a lambda capture one.
    const auto  scene = setup(c, pool, c.size(200000, 1000));
    const auto& u     = scene.first;
    const auto& q     = scene.second;

    const Distance<Union, T> query{u};
    std::vector<T>           d(q.size());

    c.measure(
      [&] {
        for (std::size_t i = 0; i < q.size(); i++) {
          d[i] = query(q[i]);
        }
      },
      q.size());

    c.checksum(sumOf(d));
    c.poolBytes(pool.usedBytes());
  });

  if (!Perf::Suite::deviceRuns()) {
    return;
  }

  a_suite.add(prefix + "query/" + prec + "/device", [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto  scene = setup(c, pool, c.size(1000000, 1000));
    const auto& u     = scene.first;
    const auto& q     = scene.second;

    const auto d = Perf::measureOnDevice<T>(
      c, pool, q, [&](const EBGeometry::Pool& a_device) { return Distance<Union, T>{u.rebasedView(a_device)}; });

    c.checksum(sumOf(d));
  });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "BVHUnion");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
