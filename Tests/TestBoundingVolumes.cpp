// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using BoundingVolumes::AABBT;
using BoundingVolumes::SphereT;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// ─────────────────────────────────────────────────────────────────────────────
// AABBT
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("AABBT: construction from lo/hi corners", "[AABBT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T> lo(-1, -2, -3);
  const Vec3T<T> hi(1, 2, 3);
  AABBT<T>       box(lo, hi);

  REQUIRE(box.getLowCorner()[0] == T(-1.0));
  REQUIRE(box.getHighCorner()[2] == T(3.0));

  auto c = box.getCentroid();
  REQUIRE(c[0] == T(0.0));
  REQUIRE(c[1] == T(0.0));
  REQUIRE(c[2] == T(0.0));
}

TEMPLATE_TEST_CASE("AABBT: construction from point cloud", "[AABBT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const std::vector<Vec3T<T>> pts = {Vec3T<T>(0, 0, 0), Vec3T<T>(3, 0, 0), Vec3T<T>(0, 4, 0), Vec3T<T>(0, 0, 5)};
  AABBT<T>                    box(pts);

  REQUIRE(box.getLowCorner()[0] == T(0.0));
  REQUIRE(box.getHighCorner()[0] == T(3.0));
  REQUIRE(box.getHighCorner()[1] == T(4.0));
  REQUIRE(box.getHighCorner()[2] == T(5.0));
}

TEMPLATE_TEST_CASE("AABBT: pointer constructor matches the std::vector constructor",
                   "[AABBT]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const std::vector<Vec3T<T>> pts = {Vec3T<T>(0, 0, 0), Vec3T<T>(3, 0, 0), Vec3T<T>(0, 4, 0), Vec3T<T>(0, 0, 5)};
  const AABBT<T>              fromVector(pts);
  const AABBT<T>              fromPointer(pts.data(), pts.size());

  REQUIRE(fromPointer.getLowCorner() == fromVector.getLowCorner());
  REQUIRE(fromPointer.getHighCorner() == fromVector.getHighCorner());
}

TEMPLATE_TEST_CASE("AABBT: volume and surface area", "[AABBT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const AABBT<T> unit(Vec3T<T>(0, 0, 0), Vec3T<T>(1, 1, 1));
  REQUIRE_THAT(unit.getVolume(), WithinRel(T(1.0)));
  REQUIRE_THAT(unit.getArea(), WithinRel(T(6.0)));

  const AABBT<T> rect(Vec3T<T>(0, 0, 0), Vec3T<T>(2, 3, 4));
  REQUIRE_THAT(rect.getVolume(), WithinRel(T(24.0)));
}

TEMPLATE_TEST_CASE("AABBT: distance to point", "[AABBT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const AABBT<T> unit(Vec3T<T>(0, 0, 0), Vec3T<T>(1, 1, 1));

  // Inside: distance should be 0
  REQUIRE_THAT(unit.getDistance(Vec3T<T>(T(0.5), T(0.5), T(0.5))), WithinAbs(0.0, tightMargin<T>()));

  // Outside along one axis
  REQUIRE_THAT(unit.getDistance(Vec3T<T>(T(3.0), T(0.5), T(0.5))), WithinRel(T(2.0)));

  // Outside at a corner
  const Vec3T<T> corner_pt(2.0, 0.0, 0.0);
  REQUIRE_THAT(unit.getDistance(corner_pt), WithinRel(T(1.0)));
}

TEMPLATE_TEST_CASE("AABBT: getDistance2 agrees with getDistance()*getDistance() but avoids the sqrt",
                   "[AABBT]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const AABBT<T> unit(Vec3T<T>(0, 0, 0), Vec3T<T>(1, 1, 1));

  const std::vector<Vec3T<T>> queryPoints = {
    Vec3T<T>(T(0.5), T(0.5), T(0.5)), // inside
    Vec3T<T>(T(3.0), T(0.5), T(0.5)), // outside along one axis
    Vec3T<T>(2.0, 0.0, 0.0),          // outside at a corner
    Vec3T<T>(-5.0, -5.0, -5.0),       // outside, far away on every axis
  };

  for (const auto& p : queryPoints) {
    const T d  = unit.getDistance(p);
    const T d2 = unit.getDistance2(p);

    REQUIRE_THAT(d2, WithinAbs(static_cast<double>(d * d), tightMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("AABBT: intersects", "[AABBT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const AABBT<T> a(Vec3T<T>(0, 0, 0), Vec3T<T>(2, 2, 2));
  const AABBT<T> b(Vec3T<T>(1, 1, 1), Vec3T<T>(3, 3, 3));
  const AABBT<T> c(Vec3T<T>(5, 5, 5), Vec3T<T>(6, 6, 6));

  REQUIRE(a.intersects(b));
  REQUIRE(b.intersects(a));
  REQUIRE(!a.intersects(c));
  REQUIRE(!c.intersects(a));
}

TEMPLATE_TEST_CASE("AABBT: overlapping volume", "[AABBT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const AABBT<T> a(Vec3T<T>(0, 0, 0), Vec3T<T>(2, 2, 2));
  const AABBT<T> b(Vec3T<T>(1, 1, 1), Vec3T<T>(3, 3, 3));

  // Overlap is a 1×1×1 cube
  REQUIRE_THAT(a.getOverlappingVolume(b), WithinRel(T(1.0)));

  // No overlap
  const AABBT<T> c(Vec3T<T>(5, 5, 5), Vec3T<T>(6, 6, 6));
  REQUIRE_THAT(a.getOverlappingVolume(c), WithinAbs(0.0, tightMargin<T>()));

  // Identical boxes
  REQUIRE_THAT(a.getOverlappingVolume(a), WithinRel(T(8.0)));
}

// ─────────────────────────────────────────────────────────────────────────────
// SphereT
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SphereT: construction", "[SphereT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T> c(1, 2, 3);
  SphereT<T>     s(c, T(5.0));

  REQUIRE(s.getRadius() == T(5.0));
  REQUIRE(s.getCentroid()[0] == T(1.0));
  REQUIRE(s.getCentroid()[1] == T(2.0));
  REQUIRE(s.getCentroid()[2] == T(3.0));
}

TEMPLATE_TEST_CASE("SphereT: getDistance/getDistance2 (zero inside, radial outside)",
                   "[SphereT]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const SphereT<T> s(Vec3T<T>(0, 0, 0), T(2.0));

  // Interior and surface points: unsigned distance is zero.
  REQUIRE_THAT(s.getDistance(Vec3T<T>(0, 0, 0)), WithinAbs(0.0, tightMargin<T>()));
  REQUIRE_THAT(s.getDistance2(Vec3T<T>(1, 0, 0)), WithinAbs(0.0, tightMargin<T>()));
  REQUIRE_THAT(s.getDistance(Vec3T<T>(2, 0, 0)), WithinAbs(0.0, tightMargin<T>()));

  // Exterior points: distance is |x - center| - radius, and getDistance2 == getDistance()^2.
  for (const auto& p : {Vec3T<T>(5, 0, 0), Vec3T<T>(0, -4, 0), Vec3T<T>(3, 4, 0)}) {
    const T d = s.getDistance(p);
    REQUIRE_THAT(d, WithinRel((p - s.getCentroid()).length() - T(2.0)));
    REQUIRE_THAT(s.getDistance2(p), withinAbsT(d * d, looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("SphereT: construction from point cloud (Ritter)", "[SphereT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const std::vector<Vec3T<T>> pts = {Vec3T<T>(-2, 0, 0), Vec3T<T>(2, 0, 0), Vec3T<T>(0, 2, 0), Vec3T<T>(0, -2, 0)};
  SphereT<T>                  s(pts); // uses Ritter algorithm by default
  // Ritter's algorithm is not guaranteed tight, but must enclose all points
  const T relativeMargin = T(1000) * std::numeric_limits<T>::epsilon();
  for (const auto& p : pts) {
    const T dist = (p - s.getCentroid()).length();
    REQUIRE(dist <= s.getRadius() + relativeMargin * s.getRadius());
  }
}

TEMPLATE_TEST_CASE("SphereT: pointer constructor matches the std::vector constructor",
                   "[SphereT]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const std::vector<Vec3T<T>> pts = {Vec3T<T>(-2, 0, 0), Vec3T<T>(2, 0, 0), Vec3T<T>(0, 2, 0), Vec3T<T>(0, -2, 0)};
  const SphereT<T>            fromVector(pts);
  const SphereT<T>            fromPointer(pts.data(), pts.size());

  REQUIRE(fromPointer.getCentroid() == fromVector.getCentroid());
  REQUIRE(fromPointer.getRadius() == fromVector.getRadius());
}

TEMPLATE_TEST_CASE("SphereT: intersects", "[SphereT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const SphereT<T> a(Vec3T<T>(0, 0, 0), T(1.0));
  const SphereT<T> b(Vec3T<T>(1.5, 0, 0), T(1.0)); // overlap
  const SphereT<T> c(Vec3T<T>(5, 0, 0), T(1.0));   // no overlap

  REQUIRE(a.intersects(b));
  REQUIRE(!a.intersects(c));
}

TEMPLATE_TEST_CASE("SphereT: overlapping volume (concentric)", "[SphereT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Identical spheres: overlap = full volume = 4/3 π r³
  const SphereT<T> a(Vec3T<T>(0, 0, 0), T(1.0));
  constexpr T      pi    = T(3.14159265358979323846);
  const T          exact = (T(4.0) / T(3.0)) * pi;
  REQUIRE_THAT(a.getOverlappingVolume(a), WithinRel(exact, T(1.0e-4)));

  // Non-overlapping: volume = 0
  const SphereT<T> b(Vec3T<T>(10, 0, 0), T(1.0));
  REQUIRE_THAT(a.getOverlappingVolume(b), WithinAbs(0.0, tightMargin<T>()));
}

TEMPLATE_TEST_CASE("SphereT: volume and area", "[SphereT]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  constexpr T      pi = T(3.14159265358979323846);
  const SphereT<T> s(Vec3T<T>(0, 0, 0), T(2.0));

  REQUIRE_THAT(s.getVolume(), WithinRel((T(4.0) / T(3.0)) * pi * T(8.0), T(1.0e-4)));
  REQUIRE_THAT(s.getArea(), WithinRel(T(4.0) * pi * T(4.0), T(1.0e-4)));
}

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
// ─────────────────────────────────────────────────────────────────────────────
// Device: the AABBT / SphereT surface is callable from a kernel and matches the host
// ─────────────────────────────────────────────────────────────────────────────

// The reduction over the AABBT/SphereT surface, shared by the device kernel and the host mirror so
// the two are guaranteed to compute the same thing.
template <class T>
EBGEOMETRY_HOST_DEVICE
T
bvReduction()
{
  const AABBT<T>   box(Vec3T<T>(0.0, 0.0, 0.0), Vec3T<T>(1.0, 1.0, 1.0));
  const AABBT<T>   other(Vec3T<T>(0.5, 0.5, 0.5), Vec3T<T>(2.0, 2.0, 2.0));
  const SphereT<T> sphere(Vec3T<T>(0.5, 0.5, 0.5), T(0.5));

  const Vec3T<T> p(2.0, 2.0, 2.0);

  T d = box.getDistance(p) + box.getVolume() + box.getArea() + box.getCentroid().length();
  d += (box.intersects(other) ? T(1.0) : T(0.0)) + box.getOverlappingVolume(other);
  d += sphere.getDistance(p) + sphere.getVolume() + sphere.getRadius();

  return d;
}

template <class T>
EBGEOMETRY_GLOBAL
void
bvDeviceKernel(T* a_out)
{
  a_out[0] = bvReduction<T>();
}

TEMPLATE_TEST_CASE("AABBT/SphereT: device query surface matches the host",
                   "[AABBT][SphereT][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  DeviceBuffer<T> deviceOut;

  bvDeviceKernel<T><<<1, 1>>>(deviceOut.get());
  (void)GPU::deviceSynchronize();

  REQUIRE_THAT(readScalar(deviceOut.get()), WithinRel(bvReduction<T>(), gpuTol<T>()));
}
#endif
