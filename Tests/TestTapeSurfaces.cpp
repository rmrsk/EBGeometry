// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for the surfaces of a shape (EBGeometry_SurfaceRef.hpp): closestSurface() of a sphere cloud, of BVH
// unions of one type, of several and of containers, and of a tape, drilling into its container leaves and into a
// nested tape; the members of a BVH union numbered in the order given; a tape's first surfaces and ExprIndex's
// surface ranges; copies; and device evaluation.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

// A small linear congruential generator: the same numbers in every build and precision.
struct Lcg
{
  uint64_t m_state = 12345U;

  double
  next() noexcept
  {
    m_state = m_state * 6364136223846793005ULL + 1442695040888963407ULL;

    return double(m_state >> 11) / double(1ULL << 53);
  }
};

// Spheres of radius 0.2 to 0.5 at random places in [0, 8]^3, in no spatial order.
template <class T>
std::vector<SphereCloud::Sphere<T>>
randomSpheres(const size_t a_count, const uint64_t a_seed)
{
  Lcg rng{a_seed};

  std::vector<SphereCloud::Sphere<T>> spheres;

  for (size_t i = 0; i < a_count; i++) {
    const Vec3T<T> center(T(8 * rng.next()), T(8 * rng.next()), T(8 * rng.next()));

    spheres.push_back(SphereCloud::Sphere<T>{center, T(0.2 + 0.3 * rng.next())});
  }

  return spheres;
}

// Points spread over [-1, 9]^3.
template <class T>
std::vector<Vec3T<T>>
queryPoints(const size_t a_count)
{
  Lcg rng{777U};

  std::vector<Vec3T<T>> points;

  for (size_t i = 0; i < a_count; i++) {
    points.emplace_back(T(10 * rng.next() - 1), T(10 * rng.next() - 1), T(10 * rng.next() - 1));
  }

  return points;
}

// The signed distance from a point to a sphere's surface.
template <class T>
T
sphereDistance(const SphereCloud::Sphere<T>& a_sphere, const Vec3T<T>& a_point)
{
  return (a_point - a_sphere.m_center).length() - a_sphere.m_radius;
}

// The smallest signed distance from a point to any of the spheres, by brute force.
template <class T>
T
bruteMin(const std::vector<SphereCloud::Sphere<T>>& a_spheres, const Vec3T<T>& a_point)
{
  T best = Math::Limits<T>::infinity();

  for (const auto& sphere : a_spheres) {
    best = std::min(best, sphereDistance(sphere, a_point));
  }

  return best;
}

// The same spheres as analytic shapes, for the BVH unions.
template <class T>
std::vector<SphereSDF<T>>
asShapes(const std::vector<SphereCloud::Sphere<T>>& a_spheres)
{
  std::vector<SphereSDF<T>> shapes;

  for (const auto& sphere : a_spheres) {
    shapes.emplace_back(sphere.m_center, sphere.m_radius);
  }

  return shapes;
}

// Device functor: the surface that decides a tape's value at a point.
template <class T, class TapeType>
struct SurfaceQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_tape.closestSurface(a_point).m_surface;
  }
};

} // namespace

TEMPLATE_TEST_CASE("SphereCloudBVH: closestSurface names the sphere that decides, by its cloud index",
                   "[SurfaceRef][SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto spheres = randomSpheres<T>(200, 1U);

  Pool pool(hostMemoryResource());

  const auto cloud = SphereCloudBVH<T>::build(pool, spheres, 4);
  const auto roomy = SphereCloudBVH<T>::build(pool, spheres, 4, 300);

  static_assert(hasSurfaces<SphereCloudBVH<T>, T>);
  static_assert(!hasSurfaces<SphereSDF<T>, T>);

  // One surface per cloud index the cloud can hold, so that a rebuild keeps the numbering.
  CHECK(cloud.getNumSurfaces() == 200U);
  CHECK(roomy.getNumSurfaces() == 300U);

  for (const auto& p : queryPoints<T>(500)) {
    const SurfaceRef<T> surface = cloud.closestSurface(p);

    REQUIRE(surface.m_surface < spheres.size());
    CHECK_THAT(sphereDistance(spheres[surface.m_surface], p), withinAbsT(bruteMin(spheres, p), tightMargin<T>()));
    CHECK(surface.m_localPoint == p);
  }
}

TEMPLATE_TEST_CASE("BVHUnion: members are numbered in the order given, and closestSurface names the smallest",
                   "[SurfaceRef][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto spheres = randomSpheres<T>(100, 2U);
  const auto shapes  = asShapes(spheres);

  Pool pool(hostMemoryResource());

  const BVHUnion<T, 4, SphereSDF<T>>                       sharp(pool, shapes);
  const BVHSmoothUnion<T, 4, SmoothMinOp<T>, SphereSDF<T>> smooth(pool, T(0.1), shapes);

  // The storage order is the BVH's, and every place maps to a different member.
  std::vector<uint32_t> members;
  bool                  reordered = false;

  for (uint32_t place = 0; place < sharp.getNumMembers(); place++) {
    members.push_back(sharp.getMemberIndex(place));

    reordered = reordered || (members.back() != place);
  }

  std::sort(members.begin(), members.end());

  CHECK(reordered);
  CHECK(std::unique(members.begin(), members.end()) == members.end());
  CHECK(members.back() == shapes.size() - 1);
  CHECK(sharp.getNumSurfaces() == shapes.size());
  CHECK(smooth.getNumSurfaces() == shapes.size());
  CHECK(sharp.getFirstSurface(7) == 7U);

  for (const auto& p : queryPoints<T>(500)) {
    const T best = bruteMin(spheres, p);

    for (const SurfaceRef<T>& surface : {sharp.closestSurface(p), smooth.closestSurface(p)}) {
      REQUIRE(surface.m_surface < shapes.size());
      CHECK_THAT(shapes[surface.m_surface].signedDistance(p), withinAbsT(best, tightMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("BVHUnion: a union of several types numbers its members type by type, unbounded ones included",
                   "[SurfaceRef][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const std::vector<SphereSDF<T>>           balls{SphereSDF<T>(Vec3(T(6), T(0), T(0)), T(1)),
                                        SphereSDF<T>(Vec3(T(0), T(0), T(0)), T(1))};
  const std::vector<BoxSDF<T>>              boxes{BoxSDF<T>(Vec3(T(2), T(-1), T(-1)), Vec3(T(4), T(1), T(1)))};
  const std::vector<InfiniteCylinderSDF<T>> rods{
    InfiniteCylinderSDF<T>(Vec3(T(0), T(20), T(0)), T(1), 2)}; // along z, far above

  Pool pool(hostMemoryResource());

  const BVHUnion<T, 4, SphereSDF<T>, BoxSDF<T>, InfiniteCylinderSDF<T>> scene(pool, balls, boxes, rods);

  CHECK(scene.getNumSurfaces() == 4U);

  // Balls 0 and 1, the box 2, the rod (stored first, as an unbounded member) 3.
  CHECK(scene.closestSurface(Vec3(T(6), T(0), T(1.5))).m_surface == 0U);
  CHECK(scene.closestSurface(Vec3(T(0), T(0), T(1.5))).m_surface == 1U);
  CHECK(scene.closestSurface(Vec3(T(3), T(0), T(1.5))).m_surface == 2U);
  CHECK(scene.closestSurface(Vec3(T(0), T(19.5), T(0))).m_surface == 3U);
  CHECK(scene.getMemberIndex(0) == 3U);
}

TEMPLATE_TEST_CASE("BVHUnion: a union of sphere clouds numbers each cloud's spheres after the clouds before it",
                   "[SurfaceRef][BVHUnion]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  std::vector<std::vector<SphereCloud::Sphere<T>>> parts;
  std::vector<SphereCloudBVH<T>>                   clouds;

  for (uint64_t c = 0; c < 3; c++) {
    parts.push_back(randomSpheres<T>(20 + 10 * c, 10U + c));
    clouds.push_back(SphereCloudBVH<T>::build(pool, parts.back(), 4));
  }

  const BVHUnion<T, 4, SphereCloudBVH<T>> scene(pool, clouds);

  static_assert(decltype(scene)::HasContainers);

  CHECK(scene.getFirstSurface(0) == 0U);
  CHECK(scene.getFirstSurface(1) == 20U);
  CHECK(scene.getFirstSurface(2) == 50U);
  CHECK(scene.getNumSurfaces() == 90U);

  // All the spheres in surface order, for the brute force.
  std::vector<SphereCloud::Sphere<T>> all;

  for (const auto& part : parts) {
    all.insert(all.end(), part.begin(), part.end());
  }

  for (const auto& p : queryPoints<T>(400)) {
    const SurfaceRef<T> surface = scene.closestSurface(p);

    REQUIRE(surface.m_surface < all.size());
    CHECK_THAT(sphereDistance(all[surface.m_surface], p), withinAbsT(bruteMin(all, p), tightMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Tape: closestSurface drills into a container leaf, numbering surfaces across the scene",
                   "[SurfaceRef][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const auto spheres = randomSpheres<T>(60, 3U);

  Pool pool(hostMemoryResource());

  const auto cloud = SphereCloudBVH<T>::build(pool, spheres, 4);
  const auto box   = BoxSDF<T>(Vec3(T(-1), T(-1), T(-1)), Vec3(T(9), T(9), T(9)));
  const auto lid   = SphereSDF<T>(Vec3(T(4), T(4), T(12)), T(2));

  // Leaves: the box 0, the cloud 1 (its spheres cut out of the box), the lid 2.
  const Expr<T> expr =
    Union(Difference(SurfaceLabel(box, "wall"), SurfaceLabel(cloud, "particles")), SurfaceLabel(lid, "lid"));

  const Tape<T, SphereCloudBVH<T>> tape(pool, expr);
  const ExprIndex<T>               index(expr);

  CHECK(tape.getNumSurfaces() == 62U);
  CHECK(tape.getFirstSurface(0) == 0U);
  CHECK(tape.getFirstSurface(1) == 1U);
  CHECK(tape.getFirstSurface(2) == 61U);
  CHECK(tape.getFirstSurface(3) == 62U);
  CHECK(tape.getFirstSurface(4) == SurfaceRef<T>::None);

  // The index numbers them the same, every node a contiguous range.
  CHECK(index.getNumSurfaces() == 62U);
  CHECK(index.getNode(0).m_firstSurface == 0U);
  CHECK(index.getNode(0).m_numSurfaces == 62U);

  for (uint32_t surface = 0; surface < index.getNumSurfaces(); surface++) {
    const uint32_t leaf = index.getLeafOfSurface(surface);

    CHECK(index.getFirstSurface(leaf) <= surface);
    CHECK(surface < index.getFirstSurface(leaf + 1));
  }

  size_t onCloud = 0;

  for (const auto& p : queryPoints<T>(600)) {
    const LeafRef<T>    leaf    = tape.closestLeaf(p);
    const SurfaceRef<T> surface = tape.closestSurface(p);

    REQUIRE(surface.m_leaf == leaf.m_leaf);
    CHECK(surface.m_surfaceLabel == tape.getSurfaceLabel(leaf));
    CHECK(index.getLeafOfSurface(surface.m_surface) == leaf.m_leaf);

    if (leaf.m_leaf == 1U) {
      const uint32_t sphere = surface.m_surface - tape.getFirstSurface(1);

      REQUIRE(sphere < spheres.size());
      CHECK_THAT(sphereDistance(spheres[sphere], p), withinAbsT(bruteMin(spheres, p), tightMargin<T>()));

      onCloud++;
    }
    else {
      CHECK(surface.m_surface == tape.getFirstSurface(leaf.m_leaf));
    }
  }

  CHECK(onCloud > 50U);

  // A copy numbers them the same.
  Pool       other(hostMemoryResource());
  const auto copy = tape.deepCopy(other);

  for (const auto& p : queryPoints<T>(100)) {
    CHECK(copy.closestSurface(p).m_surface == tape.closestSurface(p).m_surface);
  }
}

TEMPLATE_TEST_CASE("Tape: without a container leaf, a surface is its leaf and nothing more is stored",
                   "[SurfaceRef][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  Pool pool(hostMemoryResource());

  const auto expr =
    Difference(BoxSDF<T>(Vec3(T(-1), T(-1), T(-1)), Vec3(T(1), T(1), T(1))),
               Union(SphereSDF<T>(Vec3(T(1), T(0), T(0)), T(0.5)), SphereSDF<T>(Vec3(T(-1), T(0), T(0)), T(0.5))));

  const Tape<T> tape(pool, expr);

  CHECK(tape.layout().m_surfaces.size() == 0U);
  CHECK(tape.getNumSurfaces() == 3U);
  CHECK(ExprIndex<T>(expr).getNumSurfaces() == 3U);

  for (const auto& p : queryPoints<T>(200)) {
    CHECK(tape.closestSurface(p).m_surface == tape.closestLeaf(p).m_leaf);
  }
}

TEMPLATE_TEST_CASE("Tape: closestSurface drills through a tape listed as a primitive into its cloud",
                   "[SurfaceRef][Tape]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Ball = SphereCloudBVH<T>;

  const auto spheres = randomSpheres<T>(40, 4U);

  Pool pool(hostMemoryResource());

  const auto cloud = Ball::build(pool, spheres, 4);

  // The inner tape: a ball (surface 0) and the cloud (surfaces 1 to 40). The outer: a far box, then the inner.
  const Tape<T, Ball>          inner(pool, Union(SphereSDF<T>(Vec3(T(-5), T(4), T(4)), T(1)), toExpr(cloud)));
  const Tape<T, Tape<T, Ball>> outer(
    pool, Union(BoxSDF<T>(Vec3(T(20), T(20), T(20)), Vec3(T(21), T(21), T(21))), toExpr(inner)));

  static_assert(hasSurfaces<Tape<T, Ball>, T>);

  CHECK(inner.getNumSurfaces() == 41U);
  CHECK(outer.getNumSurfaces() == 42U);

  for (const auto& p : queryPoints<T>(300)) {
    const SurfaceRef<T> surface = outer.closestSurface(p);

    REQUIRE(surface.m_surface != SurfaceRef<T>::None);

    if (surface.m_surface >= 2U) {
      CHECK_THAT(sphereDistance(spheres[surface.m_surface - 2U], p),
                 withinAbsT(bruteMin(spheres, p), tightMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Tape: an empty collection names no surface", "[SurfaceRef][Tape]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const auto empty = SphereCloudBVH<T>::build(pool, std::vector<SphereCloud::Sphere<T>>{}, 4);

  const Tape<T, SphereCloudBVH<T>> tape(pool, toExpr(empty));

  CHECK(empty.closestSurface(Vec3T<T>::zeros()).m_surface == SurfaceRef<T>::None);
  CHECK(tape.getNumSurfaces() == 0U);
  CHECK(tape.closestSurface(Vec3T<T>::zeros()).m_surface == SurfaceRef<T>::None);
}

TEMPLATE_TEST_CASE("ExprIndex: rejects a surface or a leaf out of range", "[ExprIndex]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  REQUIRE(abortsWith(
    [] {
      const ExprIndex<T> index(toExpr(SphereSDF<T>(Vec3T<T>::zeros(), T(1))));

      (void)index.getLeafOfSurface(1);
    },
    "ExprIndex: surface 1 of 1"));
  REQUIRE(abortsWith(
    [] {
      const ExprIndex<T> index(toExpr(SphereSDF<T>(Vec3T<T>::zeros(), T(1))));

      (void)index.getFirstSurface(2);
    },
    "ExprIndex: leaf 2 of 1"));
}

TEMPLATE_TEST_CASE("Tape: closestSurface on a device matches the host", "[Tape][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Ball = SphereCloudBVH<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const auto spheres = randomSpheres<T>(60, 5U);

  Pool pool(hostMemoryResource());

  const auto cloud = Ball::build(pool, spheres, 4);

  const Tape<T, Ball> tape(pool,
                           Difference(BoxSDF<T>(Vec3(T(-1), T(-1), T(-1)), Vec3(T(9), T(9), T(9))), toExpr(cloud)));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  const auto points = queryPoints<T>(400);

  const auto onDevice = evaluateOnDevice<uint32_t>(SurfaceQuery<T, Tape<T, Ball>>{tape.rebasedView(device)}, points);
  const auto onHost   = evaluateOnHost<uint32_t>(SurfaceQuery<T, Tape<T, Ball>>{tape}, points);

  size_t differ = 0;

  for (size_t i = 0; i < points.size(); i++) {
    differ += (onDevice[i] != onHost[i]) ? 1U : 0U;
  }

  // Where two surfaces are as close, the device's rounding may pick the other.
  CHECK(differ <= 4U);
}
