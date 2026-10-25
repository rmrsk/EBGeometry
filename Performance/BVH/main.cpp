// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Performance cases for PackedBVH::traverse() itself: a BVH over the boxes of the probe mesh's triangles,
// walked with a ray (the nearest box entered, and whether any box is entered) and with a point (the
// nearest box). The leaf work is one box test per primitive, so the cases measure the walk: the query's
// child test, the sort, the stack. A host build runs the host cases; a CUDA or HIP build runs host/device
// pairs in Morton and random order. See ../README.md and PerformanceTesting.rst.

#include <cstddef>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include "../Common/PerfHarness.hpp"

namespace {

// The branching factor and the leaf size, spelled out (see ../PointCloud/main.cpp): bottom-up build,
// four boxes per leaf.
constexpr std::size_t K        = 4;
constexpr std::size_t LeafSize = 4;

// The numbers of queries, at full and at smoke size.
constexpr std::size_t HostQueries  = 200000;
constexpr std::size_t PairQueries  = 262144;
constexpr std::size_t SmokeQueries = 1000;

template <class T>
using Vec3 = EBGeometry::Vec3T<T>;

template <class T>
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

template <class T>
using Packed = EBGeometry::BVH::PackedBVH<T, AABB<T>, K>;

// A ray from a_origin towards the box's centre: rays from nearby origins run side by side, so Morton
// order gives a warp coherent rays.
template <class T>
EBGEOMETRY_HOST_DEVICE
EBGeometry::BVH::RayQuery<T>
rayTowards(const Vec3<T>& a_origin, const Vec3<T>& a_target) noexcept
{
  const Vec3<T> d = a_target - a_origin;

  return EBGeometry::BVH::RayQuery<T>(a_origin, d / d.length());
}

// The nearest box a ray enters (+infinity for none): RayQuery, NearestFirst, the bound the nearest entry.
template <class T>
struct RayNearest
{
  Packed<T> m_bvh;
  Vec3<T>   m_target;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    const auto ray   = rayTowards(a_origin, m_target);
    const auto boxes = m_bvh.getPrimitives();

    T best = EBGeometry::Math::Limits<T>::infinity();

    m_bvh.traverse(
      ray,
      best,
      [&ray, &boxes](T& a_best, std::size_t a_offset, std::size_t a_count) noexcept {
        for (std::size_t i = a_offset; i < a_offset + a_count; i++) {
          const T t = ray.key(boxes[static_cast<std::uint32_t>(i)]);

          a_best = (t < a_best) ? t : a_best;
        }
      },
      [](const T& a_best) noexcept { return a_best; });

    return best;
  }
};

// Whether a ray enters any box (1 or 0): AsStored, the bound dropping to -infinity at the first box.
template <class T>
struct RayAny
{
  Packed<T> m_bvh;
  Vec3<T>   m_target;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_origin) const noexcept
  {
    const auto ray   = rayTowards(a_origin, m_target);
    const auto boxes = m_bvh.getPrimitives();

    bool found = false;

    m_bvh.traverse(
      ray,
      found,
      [&ray, &boxes](bool& a_found, std::size_t a_offset, std::size_t a_count) noexcept {
        for (std::size_t i = a_offset; i < a_offset + a_count && !a_found; i++) {
          a_found = ray.key(boxes[static_cast<std::uint32_t>(i)]) < EBGeometry::Math::Limits<T>::infinity();
        }
      },
      [](const bool& a_found) noexcept {
        return a_found ? -EBGeometry::Math::Limits<T>::infinity() : EBGeometry::Math::Limits<T>::infinity();
      },
      EBGeometry::BVH::DescendAll(),
      EBGeometry::BVH::AsStored());

    return found ? T(1) : T(0);
  }
};

// The squared distance from a point to the nearest box: PointQuery, NearestFirst.
template <class T>
struct PointNearest
{
  Packed<T> m_bvh;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3<T>& a_point) const noexcept
  {
    const EBGeometry::BVH::PointQuery<T> point{a_point};
    const auto                           boxes = m_bvh.getPrimitives();

    T best = EBGeometry::Math::Limits<T>::infinity();

    m_bvh.traverse(
      point,
      best,
      [&point, &boxes](T& a_best, std::size_t a_offset, std::size_t a_count) noexcept {
        for (std::size_t i = a_offset; i < a_offset + a_count; i++) {
          const T d2 = point.key(boxes[static_cast<std::uint32_t>(i)]);

          a_best = (d2 < a_best) ? d2 : a_best;
        }
      },
      [](const T& a_best) noexcept { return a_best; });

    return best;
  }
};

// The BVH over the mesh's triangle boxes, built in a_pool, and its box.
template <class T>
Packed<T>
buildBoxes(Perf::Case& c, EBGeometry::Pool& a_pool)
{
  const auto triangles = EBGeometry::Parser::readIntoTriangles<T>(Perf::meshFile(c.smoke()));

  std::vector<std::pair<AABB<T>, AABB<T>>> boxes;

  for (const auto& tri : triangles) {
    const auto& v  = tri.getVertexPositions();
    const auto  lo = EBGeometry::min(EBGeometry::min(v[0], v[1]), v[2]);
    const auto  hi = EBGeometry::max(EBGeometry::max(v[0], v[1]), v[2]);

    boxes.emplace_back(AABB<T>(lo, hi), AABB<T>(lo, hi));
  }

  c.param("boxes", boxes.size());
  c.param("K", K);
  c.param("leafSize", LeafSize);

  return Packed<T>::buildBottomUp(a_pool, boxes, LeafSize);
}

// Query origins: random points in the BVH's box grown to twice its size about its centre.
template <class T>
std::vector<Vec3<T>>
origins(const AABB<T>& a_box, std::size_t a_n)
{
  const Vec3<T> centre = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const Vec3<T> half   = a_box.getHighCorner() - centre;

  return Perf::pointsInBox<T>(AABB<T>(centre - T(2) * half, centre + T(2) * half), a_n, Perf::Seed::RayOrigins);
}

// One operation in one precision: the host case, and host/device pairs in each order. a_make(bvh, centre)
// gives the query functor.
template <class T, class Make>
void
addOperation(Perf::Suite& a_suite, const std::string& a_operation, Make a_make)
{
  const std::string prefix = Perf::caseName({"BVH", "mesh-boxes", a_operation});
  const std::string prec   = Perf::precisionName<T>();

  a_suite.add(Perf::caseName({prefix, prec, "host"}), [=](Perf::Case& c) {
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
    const auto       bvh    = buildBoxes<T>(c, pool);
    const auto       box    = bvh.getBoundingVolume();
    const Vec3<T>    centre = T(0.5) * (box.getLowCorner() + box.getHighCorner());
    const auto       q      = origins<T>(box, c.size(HostQueries, SmokeQueries));
    std::vector<T>   out;

    Perf::measureHostQuery(c, a_make(bvh, centre), q, out);

    c.checksum(Perf::checksumOf(out));
    c.poolBytes(pool.usedBytes());
  });

  for (const Perf::Order order : Perf::Orders) {
    a_suite.add(Perf::caseName({prefix + "-" + Perf::orderName(order), prec, "hostdevice"}), [=](Perf::Case& c) {
      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());
      const auto       bvh    = buildBoxes<T>(c, pool);
      const auto       box    = bvh.getBoundingVolume();
      const Vec3<T>    centre = T(0.5) * (box.getLowCorner() + box.getHighCorner());
      const auto       q      = Perf::inOrder(order, origins<T>(box, c.size(PairQueries, SmokeQueries)));

      const auto out = Perf::measureHostAndDevice<T>(
        c,
        pool,
        q,
        [&](const EBGeometry::Pool& a_pool) { return a_make(bvh.rebasedView(a_pool), centre); },
        Perf::agreeDistance<T>);

      c.checksum(Perf::checksumOf(out));
    });
  }
}

template <class T>
void
addPrecision(Perf::Suite& a_suite)
{
  addOperation<T>(a_suite, "ray-nearest", [](const Packed<T>& a_bvh, const Vec3<T>& a_centre) {
    return RayNearest<T>{a_bvh, a_centre};
  });
  addOperation<T>(
    a_suite, "ray-any", [](const Packed<T>& a_bvh, const Vec3<T>& a_centre) { return RayAny<T>{a_bvh, a_centre}; });
  addOperation<T>(
    a_suite, "point-nearest", [](const Packed<T>& a_bvh, const Vec3<T>&) { return PointNearest<T>{a_bvh}; });
}

} // namespace

int
main(int argc, char** argv)
{
  Perf::Suite suite(argc, argv, "BVH");

  addPrecision<float>(suite);
  addPrecision<double>(suite);

  return suite.run();
}
