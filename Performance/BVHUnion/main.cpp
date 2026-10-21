// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for BVHUnion: the union of about 100,000 objects on a lattice, in float and double,
// for three families: spheres (well-filled boxes that do not overlap), capsules along the diagonal (the
// worst case for an axis-aligned box: 5% filled, heavily overlapping) and randomly oriented finite
// cylinders (a dearer distance and a mix of fills). A host build runs the host cases (build, signed
// distance); a CUDA or HIP build runs the signed distance as host/device pairs. See ../README.md.

#include <algorithm>
#include <cmath>
#include <functional>
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

// Every case for one family of objects, in one precision: the union's build, its signed distance on
// the host, and (in a GPU build) the same as host/device pairs. a_objects(n, objects) appends the
// family's objects for an n x n x n lattice.
template <class T, class Shape>
void
addFamily(Perf::Suite&                                          a_suite,
          const std::string&                                    a_family,
          std::function<void(std::size_t, std::vector<Shape>&)> a_objects)
{
  using AABB  = EBGeometry::BoundingVolumes::AABBT<T>;
  using Union = EBGeometry::BVHUnion<T, Shape, K>;

  const std::string prec   = std::is_same_v<T, float> ? "float" : "double";
  const std::string prefix = "BVHUnion/" + a_family + "/";

  // The objects and their bounding boxes.
  const auto scene = [a_objects](std::size_t a_n, std::vector<Shape>& a_shapes, std::vector<AABB>& a_boxes) {
    a_objects(a_n, a_shapes);

    for (const auto& shape : a_shapes) {
      a_boxes.push_back(shape.computeBoundingVolume());
    }
  };

  a_suite.add(prefix + "build/" + prec + "/host", [=](Perf::Case& c) {
    std::vector<Shape> shapes;
    std::vector<AABB>  boxes;

    scene(c.size(46, 6), shapes, boxes);

    std::unique_ptr<EBGeometry::Pool> pool;
    std::unique_ptr<Union>            u;

    c.measureFresh(
      [&] {
        u.reset();
        pool = std::make_unique<EBGeometry::Pool>(EBGeometry::hostMemoryResource());
      },
      [&] { u = std::make_unique<Union>(*pool, shapes, boxes); });

    c.poolBytes(pool->usedBytes());
    c.checksum(double(u->signedDistance(EBGeometry::Vec3T<T>(T(0.5), T(0.5), T(0.5)))));
    c.param("objects", shapes.size());
    c.param("K", K);
  });

  const auto setup = [=](Perf::Case& c, EBGeometry::Pool& a_pool, std::size_t a_queries) {
    std::vector<Shape> shapes;
    std::vector<AABB>  boxes;

    scene(c.size(46, 6), shapes, boxes);

    c.param("objects", shapes.size());
    c.param("K", K);

    Union u(a_pool, shapes, boxes);

    return std::make_pair(u, Perf::pointsInBox<T>(u.computeBoundingVolume(), a_queries, 13u));
  };

  a_suite.add(prefix + "query/" + prec + "/host", [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    // Named members, not a structured binding: C++17 does not let a lambda capture one.
    const auto  objects = setup(c, pool, c.size(200000, 1000));
    const auto& u       = objects.first;
    const auto& q       = objects.second;

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

  // The same points through the union on both sides, in Morton and in random order: the speed of each
  // side, and whether they agree (a different sign, or a difference beyond a thousand ulps).
  for (const std::string order : {"morton", "random"}) {
    a_suite.add(prefix + "query-" + order + "/" + prec + "/hostdevice", [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const auto  objects = setup(c, pool, c.size(262144, 1000));
      const auto& u       = objects.first;
      const auto  q       = order == "morton" ? Perf::mortonOrdered(objects.second) : objects.second;

      const auto agree = [](const T a_host, const T a_device) {
        const double diff = std::abs(double(a_host) - double(a_device));
        const double tol =
          1000.0 * double(EBGeometry::Math::Limits<T>::epsilon()) * std::max(1.0, std::abs(double(a_host)));

        return std::make_pair(diff, (a_host < T(0)) != (a_device < T(0)) || diff > tol);
      };

      const auto d = Perf::measureHostAndDevice<T>(
        c, pool, q, [&](const EBGeometry::Pool& a_pool) { return Distance<Union, T>{u.rebasedView(a_pool)}; }, agree);

      c.checksum(sumOf(d));
    });
  }
}

// Unit spheres a radius apart on a lattice: a sphere fills about half its box (pi/6), and the boxes do
// not overlap.
template <class T>
void
spheres(std::size_t a_n, std::vector<EBGeometry::SphereSDF<T>>& a_shapes)
{
  using Vec3 = EBGeometry::Vec3T<T>;

  for (std::size_t i = 0; i < a_n; i++) {
    for (std::size_t j = 0; j < a_n; j++) {
      for (std::size_t k = 0; k < a_n; k++) {
        a_shapes.emplace_back(T(3) * Vec3(T(i), T(j), T(k)), T(1));
      }
    }
  }
}

// Capsules 3 long (tip to tip), of radius 0.2, along the diagonal (1,1,1), on a lattice of spacing 1.8:
// the worst case for an axis-aligned box. A capsule fills about 5% of its box, and every box overlaps
// its neighbours', so a query point lies in many leaf boxes. The capsules themselves do not touch (those
// on one diagonal are sqrt(3) * 1.8 = 3.1 apart along it).
template <class T>
void
diagonalCapsules(std::size_t a_n, std::vector<EBGeometry::CapsuleSDF<T>>& a_shapes)
{
  using Vec3 = EBGeometry::Vec3T<T>;

  const Vec3 half = T(1.5) / std::sqrt(T(3)) * Vec3(T(1), T(1), T(1));

  for (std::size_t i = 0; i < a_n; i++) {
    for (std::size_t j = 0; j < a_n; j++) {
      for (std::size_t k = 0; k < a_n; k++) {
        const Vec3 center = T(1.8) * Vec3(T(i), T(j), T(k));

        a_shapes.emplace_back(center - half, center + half, T(0.2));
      }
    }
  }
}

// Finite cylinders 3 long, of radius 0.2, in directions drawn from a fixed seed, on the same lattice: a
// dearer distance than a capsule's (two flat caps), and a mix of box fills, as in a geometry of fibres.
// Neighbours intersect, which a union handles; the density matches the capsules'.
template <class T>
void
randomCylinders(std::size_t a_n, std::vector<EBGeometry::CylinderSDF<T>>& a_shapes)
{
  using Vec3 = EBGeometry::Vec3T<T>;

  const auto directions = Perf::pointsInBox<T>(
    EBGeometry::BoundingVolumes::AABBT<T>(Vec3(T(-1), T(-1), T(-1)), Vec3(T(1), T(1), T(1))), a_n * a_n * a_n, 21u);

  std::size_t d = 0;

  for (std::size_t i = 0; i < a_n; i++) {
    for (std::size_t j = 0; j < a_n; j++) {
      for (std::size_t k = 0; k < a_n; k++) {
        const Vec3 center = T(1.8) * Vec3(T(i), T(j), T(k));
        const Vec3 half   = T(1.5) * directions[d] / directions[d].length();

        a_shapes.emplace_back(center - half, center + half, T(0.2));
        d++;
      }
    }
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
