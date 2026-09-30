// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cmath>
#include <cstring>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

// ─────────────────────────────────────────────────────────────────────────────
// SphereSDF
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("SphereSDF: exact signed distances", "[SphereSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Vec3T<T>     center(0, 0, 0);
  const SphereSDF<T> sphere(center, T(1.0));

  // On the surface
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(1, 0, 0)), WithinAbs(0.0, looseMargin<T>()));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(-1, 0, 0)), WithinAbs(0.0, looseMargin<T>()));

  // Outside: positive distance
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(2, 0, 0)), WithinRel(T(1.0)));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(0, 3, 0)), WithinRel(T(2.0)));

  // Inside: negative distance
  REQUIRE(sphere.signedDistance(Vec3T<T>(0, 0, 0)) < T(0.0));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(0, 0, 0)), WithinRel(T(-1.0)));
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(0.5, 0, 0)), WithinRel(T(-0.5)));
}

TEMPLATE_TEST_CASE("SphereSDF: off-centre sphere", "[SphereSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  SphereSDF<T> sphere(Vec3T<T>(1, 2, 3), T(2.0));

  // Point at centre
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(1, 2, 3)), WithinRel(T(-2.0)));

  // Point on surface along x
  REQUIRE_THAT(sphere.signedDistance(Vec3T<T>(3, 2, 3)), WithinAbs(0.0, looseMargin<T>()));

  // Getters
  REQUIRE(sphere.getCenter()[0] == T(1.0));
  REQUIRE(sphere.getRadius() == T(2.0));
}

// ─────────────────────────────────────────────────────────────────────────────
// PlaneSDF
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("PlaneSDF: signed distances from xy-plane", "[PlaneSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Plane z = 0 with outward normal +z
  const PlaneSDF<T> plane(Vec3T<T>(0, 0, 0), Vec3T<T>(0, 0, 1));

  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(0, 0, 3)), WithinRel(T(3.0)));
  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(0, 0, -2)), WithinRel(T(-2.0)));
  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(5, 7, 0)), WithinAbs(0.0, looseMargin<T>()));
}

TEMPLATE_TEST_CASE("PlaneSDF: non-unit normal is normalised", "[PlaneSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Normal (1,1,0): should be normalised internally
  const PlaneSDF<T> plane(Vec3T<T>(0, 0, 0), Vec3T<T>(2, 0, 0));

  // Point at (3, 0, 0): distance = 3 (along +x)
  REQUIRE_THAT(plane.signedDistance(Vec3T<T>(3, 0, 0)), WithinRel(T(3.0)));
}

// ─────────────────────────────────────────────────────────────────────────────
// BoxSDF
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("BoxSDF: axis-aligned unit box", "[BoxSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const BoxSDF<T> box(Vec3T<T>(-1, -1, -1), Vec3T<T>(1, 1, 1));

  // Inside: negative SDF
  REQUIRE(box.signedDistance(Vec3T<T>(0, 0, 0)) < T(0.0));
  REQUIRE_THAT(box.signedDistance(Vec3T<T>(0, 0, 0)), WithinRel(T(-1.0)));

  // On a face: zero SDF
  REQUIRE_THAT(box.signedDistance(Vec3T<T>(1, 0, 0)), WithinAbs(0.0, looseMargin<T>()));

  // Outside along one axis: positive SDF
  REQUIRE_THAT(box.signedDistance(Vec3T<T>(3, 0, 0)), WithinRel(T(2.0)));

  // Outside near a corner
  // Point (2, 2, 0): distance = sqrt(1+1) = sqrt(2)
  REQUIRE_THAT(box.signedDistance(Vec3T<T>(2, 2, 0)), WithinRel(std::sqrt(T(2.0)), T(1.0e-4)));
}

// ─────────────────────────────────────────────────────────────────────────────
// CylinderSDF (defined by two end-cap centres + radius)
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("CylinderSDF: unit cylinder along z-axis", "[CylinderSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Cylinder from (0,0,-1) to (0,0,1), radius 1
  const CylinderSDF<T> cyl(Vec3T<T>(0, 0, -1), Vec3T<T>(0, 0, 1), T(1.0));

  // Centre of cylinder is inside
  REQUIRE(cyl.signedDistance(Vec3T<T>(0, 0, 0)) < T(0.0));

  // Point on the curved surface
  REQUIRE_THAT(cyl.signedDistance(Vec3T<T>(1, 0, 0)), WithinAbs(0.0, looseMargin<T>()));

  // Outside radially
  REQUIRE(cyl.signedDistance(Vec3T<T>(3, 0, 0)) > T(0.0));

  // Outside axially beyond cap
  REQUIRE(cyl.signedDistance(Vec3T<T>(0, 0, 3)) > T(0.0));
}

// ─────────────────────────────────────────────────────────────────────────────
// TorusSDF
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("TorusSDF: point on surface", "[TorusSDF]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Torus centred at origin in xy-plane: major radius 2, minor radius 0.5
  const TorusSDF<T> torus(Vec3T<T>(0, 0, 0), T(2.0), T(0.5));

  // Point on surface: along +x from tube centre (major_radius + minor_radius, 0, 0)
  REQUIRE_THAT(torus.signedDistance(Vec3T<T>(2.5, 0, 0)), WithinAbs(0.0, looseMargin<T>()));

  // Point inside the tube
  REQUIRE(torus.signedDistance(Vec3T<T>(2.0, 0, 0)) < T(0.0));

  // Point at the origin: outside the torus
  REQUIRE(torus.signedDistance(Vec3T<T>(0, 0, 0)) > T(0.0));
}

// ─────────────────────────────────────────────────────────────────────────────
// Plain value types
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Every analytic shape, with arguments matching the Shapes example. Shared by the value-type checks
// below and the device test at the bottom of this file.
template <class T>
struct AllShapes
{
  PlaneSDF<T>            plane{Vec3T<T>::zeros(), Vec3T<T>::ones()};
  SphereSDF<T>           sphere{Vec3T<T>::zeros(), T(1)};
  BoxSDF<T>              box{Vec3T<T>::zeros(), Vec3T<T>::ones()};
  TorusSDF<T>            torus{Vec3T<T>::zeros(), T(1), T(0.1)};
  CylinderSDF<T>         cylinder{Vec3T<T>::zeros(), Vec3T<T>::ones(), T(0.1)};
  InfiniteCylinderSDF<T> infiniteCylinder{Vec3T<T>::zeros(), T(0.1), 2};
  CapsuleSDF<T>          capsule{Vec3T<T>::zeros(), Vec3T<T>::ones(), T(0.1)};
  InfiniteConeSDF<T>     infiniteCone{Vec3T<T>::zeros(), T(45)};
  ConeSDF<T>             cone{Vec3T<T>::zeros(), T(1), T(45)};
  RoundedBoxSDF<T>       roundedBox{Vec3T<T>::ones(), T(0.1)};
  PerlinSDF<T>           perlin{T(1), Vec3T<T>::ones(), T(0.5), 4U};
  RoundedCylinderSDF<T>  roundedCylinder{T(1), T(0.1), T(1)};
};

// Sum of every shape's signed distance at a_point, so one number checks all twelve formulas.
template <class T>
EBGEOMETRY_HOST_DEVICE
T
sumOfShapes(const AllShapes<T>& a_shapes, const Vec3T<T>& a_point) noexcept
{
  return a_shapes.plane.signedDistance(a_point) + a_shapes.sphere.signedDistance(a_point) +
         a_shapes.box.signedDistance(a_point) + a_shapes.torus.signedDistance(a_point) +
         a_shapes.cylinder.signedDistance(a_point) + a_shapes.infiniteCylinder.signedDistance(a_point) +
         a_shapes.capsule.signedDistance(a_point) + a_shapes.infiniteCone.signedDistance(a_point) +
         a_shapes.cone.signedDistance(a_point) + a_shapes.roundedBox.signedDistance(a_point) +
         a_shapes.perlin.signedDistance(a_point) + a_shapes.roundedCylinder.signedDistance(a_point);
}

// Points that land inside, outside and near the surfaces of the shapes above.
template <class T>
std::vector<Vec3T<T>>
shapeSamplePoints()
{
  return {Vec3T<T>(T(0.1), T(0.2), T(0.3)),
          Vec3T<T>(T(0.5), T(0.5), T(0.5)),
          Vec3T<T>(T(1.05), T(0), T(0)),
          Vec3T<T>(T(-0.7), T(0.4), T(0.9)),
          Vec3T<T>(T(2), T(-1.5), T(3))};
}

} // namespace

TEMPLATE_TEST_CASE("Analytic shapes: plain, trivially copyable value types with no ImplicitFunction base",
                   "[AnalyticSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  STATIC_REQUIRE(std::is_trivially_copyable_v<AllShapes<T>>);
  STATIC_REQUIRE_FALSE(std::is_polymorphic_v<SphereSDF<T>>);
  STATIC_REQUIRE_FALSE(std::is_polymorphic_v<PerlinSDF<T>>);
  STATIC_REQUIRE_FALSE(std::is_polymorphic_v<RoundedBoxSDF<T>>);
  STATIC_REQUIRE_FALSE(std::is_base_of_v<ImplicitFunction<T>, SphereSDF<T>>);
  STATIC_REQUIRE_FALSE(std::is_base_of_v<ImplicitFunction<T>, BoxSDF<T>>);

  // A byte copy is a complete copy: it evaluates exactly like the original.
  const AllShapes<T> shapes;
  AllShapes<T>       bytes;
  std::memcpy(static_cast<void*>(&bytes), static_cast<const void*>(&shapes), sizeof(AllShapes<T>));

  for (const auto& p : shapeSamplePoints<T>()) {
    REQUIRE(sumOfShapes(bytes, p) == sumOfShapes(shapes, p));
  }
}

TEMPLATE_TEST_CASE("RoundedBoxSDF: distances, and copies no longer share the rounding sphere",
                   "[RoundedBoxSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Inner half-extents 1, rounding radius 0.5: the surface is at 1.5 along each axis.
  const RoundedBoxSDF<T> roundedBox(Vec3(T(2), T(2), T(2)), T(0.5));

  REQUIRE_THAT(roundedBox.signedDistance(Vec3(T(3), T(0), T(0))), WithinRel(T(1.5)));
  REQUIRE_THAT(roundedBox.signedDistance(Vec3(T(0), T(0), T(1.5))), WithinAbs(0.0, looseMargin<T>()));
  REQUIRE_THAT(roundedBox.signedDistance(Vec3::zeros()), WithinRel(T(-0.5)));

  // Before this class held its sphere by value, a copy shared it through a shared_ptr. A copy is now
  // an independent object, and a default-constructed box is unaffected by another box's radius.
  const RoundedBoxSDF<T> copy = roundedBox;
  const RoundedBoxSDF<T> other(Vec3(T(2), T(2), T(2)), T(0.25));

  REQUIRE(copy.signedDistance(Vec3(T(3), T(0), T(0))) == roundedBox.signedDistance(Vec3(T(3), T(0), T(0))));
  REQUIRE_THAT(other.signedDistance(Vec3(T(3), T(0), T(0))), WithinRel(T(1.75)));
  REQUIRE_THAT(RoundedBoxSDF<T>().signedDistance(Vec3(T(1), T(0), T(0))), WithinRel(T(0.4)));
}

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

namespace {

// Evaluates every shape at a_point on the device. The shapes arrive by value as a kernel argument,
// which is exactly what being trivially copyable is for.
template <class T>
EBGEOMETRY_GLOBAL
void
shapesDeviceKernel(const AllShapes<T> a_shapes, const Vec3T<T> a_point, T* a_out)
{
  a_out[0] = sumOfShapes(a_shapes, a_point);
}

} // namespace

TEMPLATE_TEST_CASE("Analytic shapes: device signedDistance matches the host",
                   "[AnalyticSDF][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const AllShapes<T> shapes;

  for (const auto& p : shapeSamplePoints<T>()) {
    DeviceBuffer<T> deviceOut;

    shapesDeviceKernel<T><<<1, 1>>>(shapes, p, deviceOut.get());
    (void)GPU::deviceSynchronize();

    REQUIRE_THAT(readScalar(deviceOut.get()), WithinRel(sumOfShapes(shapes, p), gpuTol<T>()));
  }
}

#endif
