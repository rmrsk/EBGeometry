// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <cmath>
#include <cstring>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
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
  // Inside, the distance is to the nearest face (half-extent 1 + rounding 0.5), not the rounding
  // radius alone.
  REQUIRE_THAT(roundedBox.signedDistance(Vec3::zeros()), WithinRel(T(-1.5)));
  REQUIRE_THAT(roundedBox.signedDistance(Vec3(T(0.75), T(0), T(0))), WithinRel(T(-0.75)));

  // Before this class held its sphere by value, a copy shared it through a shared_ptr. A copy is now
  // an independent object, and a default-constructed box is unaffected by another box's radius.
  const RoundedBoxSDF<T> copy = roundedBox;
  const RoundedBoxSDF<T> other(Vec3(T(2), T(2), T(2)), T(0.25));

  REQUIRE(copy.signedDistance(Vec3(T(3), T(0), T(0))) == roundedBox.signedDistance(Vec3(T(3), T(0), T(0))));
  REQUIRE_THAT(other.signedDistance(Vec3(T(3), T(0), T(0))), WithinRel(T(1.75)));
  REQUIRE_THAT(RoundedBoxSDF<T>().signedDistance(Vec3(T(1), T(0), T(0))), WithinRel(T(0.4)));
}

namespace {

// Checks that a_shape behaves as an exact signed distance function at random points in the cube
// [-a_extent, a_extent]^3. For an exact SDF f and a point p off the medial axis, |grad f| = 1, and
// stepping back along the gradient by f(p) lands on the surface: f(p - f(p) grad f(p)) = 0. A
// formula that is constant somewhere (zero gradient), or that over- or underestimates the distance,
// fails one of the two. Points near the medial axis, where the gradient is discontinuous and a
// central difference straddles the ridge, are skipped; they must be rare. f must also be
// 1-Lipschitz between any two sample points.
template <class T, class Shape>
void
requireExactSDF(const Shape& a_shape, const T a_extent)
{
  using Vec3 = Vec3T<T>;

  const T h   = std::cbrt(std::numeric_limits<T>::epsilon()) * a_extent;
  const T tol = std::is_same_v<T, float> ? T(2e-2) : T(1e-5);

  std::mt19937                      rng(12345);
  std::uniform_real_distribution<T> coord(-a_extent, a_extent);

  constexpr int numPoints = 2000;

  int  ridgePoints = 0;
  Vec3 previous    = Vec3::zeros();

  for (int i = 0; i < numPoints; i++) {
    const Vec3 p(coord(rng), coord(rng), coord(rng));
    const T    f = a_shape.signedDistance(p);

    REQUIRE(std::isfinite(f));

    INFO("p = " << p << ", f = " << f);

    if (i > 0) {
      REQUIRE(std::abs(f - a_shape.signedDistance(previous)) <= (p - previous).length() * (T(1) + tol) + tol);
    }

    previous = p;

    Vec3 grad;

    for (size_t d = 0; d < 3; d++) {
      const Vec3 e = h * Vec3::unit(d);

      grad[d] = (a_shape.signedDistance(p + e) - a_shape.signedDistance(p - e)) / (T(2) * h);
    }

    if (std::abs(grad.length() - T(1)) > tol) {
      ridgePoints++;

      continue;
    }

    const Vec3 onSurface = p - f * grad / grad.length();

    REQUIRE(std::abs(a_shape.signedDistance(onSurface)) <= tol * std::max(T(1), std::abs(f)));
  }

  REQUIRE(ridgePoints <= numPoints / 50);
}

} // namespace

TEMPLATE_TEST_CASE("Analytic shapes: every exact shape is a true signed distance function",
                   "[AnalyticSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  SECTION("SphereSDF")
  {
    requireExactSDF<T>(SphereSDF<T>(Vec3(T(0.1), T(-0.2), T(0.3)), T(0.8)), T(2));
  }
  SECTION("PlaneSDF")
  {
    requireExactSDF<T>(PlaneSDF<T>(Vec3(T(0.1), T(0.2), T(0.3)), Vec3(T(1), T(2), T(-1))), T(2));
  }
  SECTION("BoxSDF")
  {
    requireExactSDF<T>(BoxSDF<T>(Vec3(T(-0.5), T(-0.3), T(-0.8)), Vec3(T(0.7), T(0.4), T(0.2))), T(2));
  }
  SECTION("TorusSDF")
  {
    requireExactSDF<T>(TorusSDF<T>(Vec3(T(0.1), T(0), T(-0.1)), T(1), T(0.3)), T(2));
  }
  SECTION("CylinderSDF")
  {
    requireExactSDF<T>(CylinderSDF<T>(Vec3(T(-0.2), T(-0.5), T(0.1)), Vec3(T(0.3), T(0.6), T(-0.2)), T(0.4)), T(2));
  }
  SECTION("InfiniteCylinderSDF")
  {
    requireExactSDF<T>(InfiniteCylinderSDF<T>(Vec3(T(0.1), T(0.2), T(0)), T(0.5), 1), T(2));
  }
  SECTION("CapsuleSDF")
  {
    requireExactSDF<T>(CapsuleSDF<T>(Vec3(T(0), T(0), T(-1)), Vec3(T(0.2), T(0.1), T(1.5)), T(0.4)), T(2));
  }
  SECTION("ConeSDF")
  {
    requireExactSDF<T>(ConeSDF<T>(Vec3(T(0), T(0), T(0.8)), T(1.5), T(60)), T(2));
  }
  SECTION("InfiniteConeSDF")
  {
    requireExactSDF<T>(InfiniteConeSDF<T>(Vec3(T(0), T(0), T(0.5)), T(50)), T(2));
  }
  SECTION("RoundedBoxSDF")
  {
    requireExactSDF<T>(RoundedBoxSDF<T>(Vec3(T(1), T(0.6), T(0.4)), T(0.1)), T(1));
  }
  SECTION("RoundedCylinderSDF")
  {
    requireExactSDF<T>(RoundedCylinderSDF<T>(T(0.8), T(0.2), T(1.6)), T(1.5));
  }
}

TEMPLATE_TEST_CASE("CapsuleSDF: tips exactly two radii apart give a sphere, not NaN",
                   "[CapsuleSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const CapsuleSDF<T> capsule(Vec3(T(0), T(0), T(-0.5)), Vec3(T(0), T(0), T(0.5)), T(0.5));
  const SphereSDF<T>  sphere(Vec3::zeros(), T(0.5));

  for (const auto& p : {Vec3::zeros(), Vec3(T(1), T(0), T(0)), Vec3(T(0.2), T(-0.3), T(0.9))}) {
    REQUIRE_THAT(capsule.signedDistance(p), WithinAbs(double(sphere.signedDistance(p)), looseMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("PerlinSDF: the default constructor and zero persistence give defined noise",
                   "[PerlinSDF]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // The default constructor is the full constructor with the documented defaults.
  const PerlinSDF<T> byDefault;
  const PerlinSDF<T> explicitDefaults(T(1), Vec3::ones(), T(0.5), 1U);

  // Zero persistence leaves every octave after the first with zero amplitude; it must not divide
  // the frequency by zero.
  const PerlinSDF<T> zeroPersistence(T(1), Vec3::ones(), T(0), 3U);
  const PerlinSDF<T> oneOctave(T(1), Vec3::ones(), T(0.5), 1U);

  for (const auto& p : {Vec3(T(0.3), T(0.6), T(0.1)), Vec3(T(0.7), T(0.2), T(0.9)), Vec3(T(-1.3), T(2.1), T(0.4))}) {
    REQUIRE(byDefault.signedDistance(p) == explicitDefaults.signedDistance(p));
    REQUIRE(std::isfinite(zeroPersistence.signedDistance(p)));
    REQUIRE_THAT(zeroPersistence.signedDistance(p), WithinAbs(double(oneOctave.signedDistance(p)), looseMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor argument checks (EBGEOMETRY_REQUIRE, on in every build)
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Analytic shapes: invalid constructor arguments abort with a message", "[AnalyticSDF][death]")
{
  using T    = double;
  using Vec3 = Vec3T<T>;

  REQUIRE(abortsWith(
    [] {
      const PlaneSDF<T> plane(Vec3(0, 0, 0), Vec3(0, 0, 0));

      (void)plane;
    },
    "PlaneSDF: the normal must be nonzero (0, 0, 0)"));

  REQUIRE(abortsWith(
    [] {
      const SphereSDF<T> sphere(Vec3(0, 0, 0), T(-1));

      (void)sphere;
    },
    "SphereSDF: the radius must be finite and positive (-1)"));

  REQUIRE(abortsWith(
    [] {
      const BoxSDF<T> box(Vec3(0, 0, 0), Vec3(1, -1, 1));

      (void)box;
    },
    "BoxSDF: the low corner (0, 0, 0) must be below the high corner (1, -1, 1) in every direction"));

  REQUIRE(abortsWith(
    [] {
      const TorusSDF<T> torus(Vec3(0, 0, 0), T(1), T(2));

      (void)torus;
    },
    "TorusSDF: the minor radius (2) must be less than the major radius (1)"));

  REQUIRE(abortsWith(
    [] {
      const CylinderSDF<T> cylinder(Vec3(1, 2, 3), Vec3(1, 2, 3), T(1));

      (void)cylinder;
    },
    "CylinderSDF: the two centers must differ (1, 2, 3)"));

  REQUIRE(abortsWith(
    [] {
      const InfiniteCylinderSDF<T> cylinder(Vec3(0, 0, 0), T(1), 3);

      (void)cylinder;
    },
    "InfiniteCylinderSDF: the axis must be 0, 1 or 2 (3)"));

  REQUIRE(abortsWith(
    [] {
      const CapsuleSDF<T> capsule(Vec3(0, 0, 0), Vec3(0, 1, 0), T(1));

      (void)capsule;
    },
    "CapsuleSDF: the tips must be at least two radii apart (distance 1, radius 1)"));

  REQUIRE(abortsWith(
    [] {
      const InfiniteConeSDF<T> cone(Vec3(0, 0, 0), T(180));

      (void)cone;
    },
    "InfiniteConeSDF: the angle must be in (0, 180) degrees (180)"));

  REQUIRE(abortsWith(
    [] {
      const ConeSDF<T> cone(Vec3(0, 0, 0), T(0), T(30));

      (void)cone;
    },
    "ConeSDF: the height must be finite and positive (0)"));

  REQUIRE(abortsWith(
    [] {
      const RoundedBoxSDF<T> box(Vec3(1, 0, 1), T(0.1));

      (void)box;
    },
    "RoundedBoxSDF: the dimensions must be finite and positive (1, 0, 1)"));

  REQUIRE(abortsWith(
    [] {
      const PerlinSDF<T> perlin(std::numeric_limits<T>::infinity(), Vec3::ones(), T(0.5), 1U);

      (void)perlin;
    },
    "PerlinSDF: the noise amplitude must be finite (inf)"));

  REQUIRE(abortsWith(
    [] {
      const RoundedCylinderSDF<T> cylinder(T(1), T(0.5), T(1));

      (void)cylinder;
    },
    "RoundedCylinderSDF: twice the curvature (0.5) must be less than the height (1)"));
}

namespace {

// One signed distance per query point, for any one of the analytic shapes. The shape is held by value,
// as a kernel receives it, which is exactly what being trivially copyable is for.
template <class T, class Shape>
struct ShapeDistanceQuery
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }
};

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

  // The shapes are centred at (or anchored to) the origin and extend at most ~1.1 from it, so this grid
  // covers points inside, outside and near every surface, including the thin (radius 0.1) tubes.
  // PerlinSDF is a positive noise field with no surface, so it is simply sampled over the same box.
  const auto points = queryGrid<T>(Vec3T<T>(T(-1.5), T(-1.5), T(-1.5)), Vec3T<T>(T(1.5), T(1.5), T(1.5)), 15);

  const auto check = [&](const auto& a_shape, const char* a_name) {
    using Shape = std::decay_t<decltype(a_shape)>;

    INFO(a_name);
    requireSameResults(evaluateOnDevice<T>(ShapeDistanceQuery<T, Shape>{a_shape}, points),
                       evaluateOnHost<T>(ShapeDistanceQuery<T, Shape>{a_shape}, points));
  };

  check(shapes.plane, "PlaneSDF");
  check(shapes.sphere, "SphereSDF");
  check(shapes.box, "BoxSDF");
  check(shapes.torus, "TorusSDF");
  check(shapes.cylinder, "CylinderSDF");
  check(shapes.infiniteCylinder, "InfiniteCylinderSDF");
  check(shapes.capsule, "CapsuleSDF");
  check(shapes.infiniteCone, "InfiniteConeSDF");
  check(shapes.cone, "ConeSDF");
  check(shapes.roundedBox, "RoundedBoxSDF");
  check(shapes.perlin, "PerlinSDF");
  check(shapes.roundedCylinder, "RoundedCylinderSDF");
}
