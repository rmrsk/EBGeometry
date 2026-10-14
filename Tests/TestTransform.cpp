// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for the point-map and value transforms of the expression builder (EBGeometry_Expr.hpp):
// complement, translate, rotate, scale, reflect, offset, annular shell, blur, mollify and elongate,
// each evaluated through a compiled Tape. Uses an analytic sphere or box as the wrapped shape so every
// expected value is hand-computable, checks each transform against its formula applied by hand to the
// wrapped shape, and cross-checks several against an equivalent, independently-constructed analytic
// shape (e.g. a translated sphere vs. a sphere built directly at the shifted center). Also checks that a
// chain of Translate/Rotate/Scale/Reflect (folded into one affine map) matches the maps composed by
// hand, and that the bounding volume of a mapped box is the mapped box.

#include "EBGeometry.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestTapeUtils.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;

namespace {

template <class T>
using Sphere = SphereSDF<T>;

template <class T>
using Box = BoxSDF<T>;

template <class T, class... UserPrimitives>
using Compiled = TestUtils::Compiled<T, UserPrimitives...>;

template <class T>
std::vector<Vec3T<T>>
samplePoints()
{
  using Vec3 = Vec3T<T>;
  return {
    Vec3::zeros(),
    Vec3(0.5, 0, 0),
    Vec3(0, 1.5, 0),
    Vec3(0, 0, -2.0),
    Vec3(1, 1, 1),
    Vec3(-3, 2, 0.5),
  };
}

// A box with no symmetry about any coordinate plane through the origin, so that getting a map's
// direction or sign wrong changes the value.
template <class T>
Box<T>
asymmetricBox()
{
  return Box<T>(Vec3T<T>(0.5, -1, 0.25), Vec3T<T>(2, 1.5, 1));
}

// The point a_point rotated counterclockwise (seen from the positive end of the axis) by a_degrees
// about coordinate axis a_axis, written out by hand.
template <class T>
Vec3T<T>
rotatedPoint(const Vec3T<T>& a_point, const T a_degrees, const size_t a_axis)
{
  const T theta = a_degrees * T(3.14159265358979323846) / T(180);
  const T c     = std::cos(theta);
  const T s     = std::sin(theta);

  const T x = a_point[0];
  const T y = a_point[1];
  const T z = a_point[2];

  switch (a_axis) {
  case 0:
    return Vec3T<T>(x, c * y - s * z, s * y + c * z);
  case 1:
    return Vec3T<T>(c * x + s * z, y, -s * x + c * z);
  default:
    return Vec3T<T>(c * x - s * y, s * x + c * y, z);
  }
}

// The point a_point reflected in the coordinate plane normal to a_axis.
template <class T>
Vec3T<T>
reflectedPoint(const Vec3T<T>& a_point, const size_t a_axis)
{
  Vec3T<T> q = a_point;

  q[a_axis] = -q[a_axis];

  return q;
}

// Two independent code paths computing literally the same closed-form expression (a tape vs. the
// wrapped shape evaluated at the transformed point, or an analytic shape built directly at the
// transformed position/size) should agree up to floating-point reordering only -- no genuine
// approximation involved.
template <class T>
double
exactMargin()
{
  return std::is_same_v<T, float> ? 1.0e-4 : 1.0e-12;
}

// A hand-computed closed-form expected value, or a partition-of-unity sum (Blur/Mollify sample
// weights), involving a handful more floating-point operations than the "exact" comparisons.
template <class T>
double
formulaMargin()
{
  return std::is_same_v<T, float> ? 1.0e-3 : 1.0e-9;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Complement()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Complement: value is the negation of the wrapped function",
                   "[Transform][Complement]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T>   sphere(Vec3::zeros(), T(1));
  const Compiled<T> comp(Complement(sphere));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(comp(p), withinAbsT(-sphere.signedDistance(p), exactMargin<T>()));
  }

  // Inside the sphere becomes "outside" the complement, and vice versa.
  REQUIRE(comp(Vec3::zeros()) > T(0.0));
  REQUIRE(comp(Vec3(5, 0, 0)) < T(0.0));
}

TEMPLATE_TEST_CASE("Complement: complementing twice is the identity",
                   "[Transform][Complement]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Box<T>      box = asymmetricBox<T>();
  const Compiled<T> twice(Complement(Complement(box)));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(twice(p), withinAbsT(box.signedDistance(p), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Translate()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Translate: matches an equivalent sphere built directly at the shifted center",
                   "[Transform][Translate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3 shift(2, -1, 3);

  const Sphere<T> sphereAtOrigin(Vec3::zeros(), T(1));
  const Sphere<T> sphereAtShift(shift, T(1));

  const Compiled<T> translated(Translate(sphereAtOrigin, shift));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(translated(p), withinAbsT(sphereAtShift.signedDistance(p), exactMargin<T>()));
  }

  // The translated sphere's center is now at `shift`, so that point is the deepest interior point.
  REQUIRE_THAT(translated(shift), withinAbsT(T(-1.0), exactMargin<T>()));
}

TEMPLATE_TEST_CASE("Translate: matches the wrapped shape evaluated at the shifted-back point",
                   "[Transform][Translate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3        shift(2, -1, 3);
  const Box<T>      box = asymmetricBox<T>();
  const Compiled<T> translated(Translate(box, shift));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(translated(p), withinAbsT(box.signedDistance(p - shift), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Rotate()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Rotate: a full 360-degree rotation is the identity",
                   "[Transform][Rotate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Box<T> box(Vec3(-2, -1, -1), Vec3(2, 1, 1));

  for (const size_t axis : {size_t(0), size_t(1), size_t(2)}) {
    const Compiled<T> full(Rotate(box, T(360), axis));

    for (const auto& p : samplePoints<T>()) {
      REQUIRE_THAT(full(p), withinAbsT(box.signedDistance(p), formulaMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Rotate: composing a rotation with its inverse is the identity",
                   "[Transform][Rotate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Box<T> box(Vec3(-2, -1, -1), Vec3(2, 1, 1));

  for (const size_t axis : {size_t(0), size_t(1), size_t(2)}) {
    const Compiled<T> roundTrip(Rotate(Rotate(box, T(37), axis), T(-37), axis));

    for (const auto& p : samplePoints<T>()) {
      REQUIRE_THAT(roundTrip(p), withinAbsT(box.signedDistance(p), formulaMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Rotate: rotating a long-in-x box by 90 degrees about z swaps the x/y half-extents",
                   "[Transform][Rotate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A box that's long along x (half-extent 2) and short along y (half-extent 1). Rotating 90
  // degrees about z should make it behave like a box long along y and short along x.
  const Box<T>      box(Vec3(-2, -1, -1), Vec3(2, 1, 1));
  const Compiled<T> rotated(Rotate(box, T(90), size_t(2)));

  // (1.5, 0, 0) is inside the original box (|x|=1.5 < 2) but would be outside a box that's only
  // long in y (half-extent 1 along x). After a 90-degree rotation about z, evaluating the rotated
  // shape at (1.5, 0, 0) must match evaluating the ORIGINAL box at (0, 1.5, 0) (its long axis).
  REQUIRE_THAT(rotated(Vec3(1.5, 0, 0)), withinAbsT(box.signedDistance(Vec3(0, 1.5, 0)), formulaMargin<T>()));

  // Conversely, (0, 1.5, 0) on the rotated shape must match the original box's short-axis value.
  REQUIRE_THAT(rotated(Vec3(0, 1.5, 0)), withinAbsT(box.signedDistance(Vec3(-1.5, 0, 0)), formulaMargin<T>()));
}

TEMPLATE_TEST_CASE("Rotate: the rotation is counterclockwise seen from the positive end of the axis",
                   "[Transform][Rotate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // A quarter turn about z takes +x to +y, about x takes +y to +z, and about y takes +z to +x.
  const Sphere<T> sphereOnX(Vec3(2, 0, 0), T(1));
  const Sphere<T> sphereOnY(Vec3(0, 2, 0), T(1));
  const Sphere<T> sphereOnZ(Vec3(0, 0, 2), T(1));

  const Compiled<T> aboutZ(Rotate(sphereOnX, T(90), size_t(2)));
  const Compiled<T> aboutX(Rotate(sphereOnY, T(90), size_t(0)));
  const Compiled<T> aboutY(Rotate(sphereOnZ, T(90), size_t(1)));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(aboutZ(p), withinAbsT(sphereOnY.signedDistance(p), formulaMargin<T>()));
    REQUIRE_THAT(aboutX(p), withinAbsT(sphereOnZ.signedDistance(p), formulaMargin<T>()));
    REQUIRE_THAT(aboutY(p), withinAbsT(sphereOnX.signedDistance(p), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Rotate: matches the wrapped shape evaluated at the inversely-rotated point",
                   "[Transform][Rotate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Box<T> box = asymmetricBox<T>();

  for (const size_t axis : {size_t(0), size_t(1), size_t(2)}) {
    const Compiled<T> rotated(Rotate(box, T(42), axis));

    for (const auto& p : samplePoints<T>()) {
      const auto back = rotatedPoint(p, T(-42), axis);

      REQUIRE_THAT(rotated(p), withinAbsT(box.signedDistance(back), formulaMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Rotate: a rotation about a coordinate direction matches the rotation about that axis index",
                   "[Transform][Rotate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Box<T> box = asymmetricBox<T>();

  // The direction need not be normalized.
  const std::vector<Vec3> directions = {Vec3(1, 0, 0), Vec3(0, 3, 0), Vec3(0, 0, 1)};

  for (const size_t axis : {size_t(0), size_t(1), size_t(2)}) {
    const Compiled<T> aboutIndex(Rotate(box, T(37), axis));
    const Compiled<T> aboutVector(Rotate(box, T(37), directions[axis]));

    for (const auto& p : samplePoints<T>()) {
      REQUIRE_THAT(aboutVector(p), withinAbsT(aboutIndex(p), formulaMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Rotate: 120 degrees about (1, 1, 1) permutes the coordinates",
                   "[Transform][Rotate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // The rotation takes x to y, y to z and z to x, so the rotated shape's value at (x, y, z) is the
  // original's at (y, z, x). The axis need not be normalized.
  const Box<T> box = asymmetricBox<T>();

  for (const Vec3& axis : {Vec3(1, 1, 1), Vec3(2.5, 2.5, 2.5)}) {
    const Compiled<T> rotated(Rotate(box, T(120), axis));

    for (const auto& p : samplePoints<T>()) {
      REQUIRE_THAT(rotated(p), withinAbsT(box.signedDistance(Vec3(p[1], p[2], p[0])), formulaMargin<T>()));
    }
  }

  // The sphere on the x-axis lands on the y-axis.
  const Compiled<T> sphere(Rotate(Sphere<T>(Vec3(2, 0, 0), T(1)), T(120), Vec3(1, 1, 1)));

  REQUIRE_THAT(sphere(Vec3(0, 2, 0)), withinAbsT(T(-1), formulaMargin<T>()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Scale()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Scale: matches an equivalent sphere built directly with the scaled radius",
                   "[Transform][Scale]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const T scale = T(3.0);

  const Sphere<T> unitSphere(Vec3::zeros(), T(1));
  const Sphere<T> bigSphere(Vec3::zeros(), scale);

  const Compiled<T> scaled(Scale(unitSphere, scale));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(scaled(p), withinAbsT(bigSphere.signedDistance(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Scale: a negative factor reflects the shape through the origin without turning it inside out",
                   "[Transform][Scale]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Scaling by -2 maps the sphere at (1, 0, 0) with radius 0.5 to the sphere at (-2, 0, 0) with
  // radius 1: negative inside, positive outside, and distances scaled by |s| = 2.
  const Sphere<T>   sphere(Vec3(T(1), T(0), T(0)), T(0.5));
  const Compiled<T> scaled(Scale(sphere, T(-2)));

  REQUIRE_THAT(scaled(Vec3(T(-2), T(0), T(0))), withinAbsT(T(-1), exactMargin<T>()));
  REQUIRE_THAT(scaled(Vec3(T(10), T(0), T(0))), withinAbsT(T(11), exactMargin<T>()));
  REQUIRE_THAT(scaled(Vec3(T(-2), T(3), T(0))), withinAbsT(T(2), exactMargin<T>()));
}

TEMPLATE_TEST_CASE("Scale: matches |s| times the wrapped shape evaluated at p / s",
                   "[Transform][Scale]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Box<T> box = asymmetricBox<T>();

  for (const T s : {T(2.5), T(-0.75)}) {
    const Compiled<T> scaled(Scale(box, s));

    for (const auto& p : samplePoints<T>()) {
      REQUIRE_THAT(scaled(p), withinAbsT(std::abs(s) * box.signedDistance(p / s), formulaMargin<T>()));
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Offset()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Offset: matches an equivalent sphere built directly with the offset radius",
                   "[Transform][Offset]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const T offset = T(0.4);

  // Offsetting a signed distance function by a constant is equivalent (for a sphere) to growing
  // its radius by that constant: value - offset == distance-to-sphere-of-radius-(1+offset).
  const Sphere<T> unitSphere(Vec3::zeros(), T(1));
  const Sphere<T> grownSphere(Vec3::zeros(), T(1) + offset);

  const Compiled<T> offsetted(Offset(unitSphere, offset));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(offsetted(p), withinAbsT(grownSphere.signedDistance(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Offset: matches the wrapped value minus the offset",
                   "[Transform][Offset]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Box<T> box = asymmetricBox<T>();

  for (const T r : {T(0.3), T(-0.2)}) {
    const Compiled<T> offsetted(Offset(box, r));

    for (const auto& p : samplePoints<T>()) {
      REQUIRE_THAT(offsetted(p), withinAbsT(box.signedDistance(p) - r, exactMargin<T>()));
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Annular()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Annular: hollows out a shell of the requested thickness",
                   "[Transform][Annular]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const T delta = T(0.2);

  const Sphere<T>   sphere(Vec3::zeros(), T(2));
  const Compiled<T> shell(Annular(sphere, delta));

  // On the original surface: |0| - delta = -delta (inside the shell).
  REQUIRE_THAT(shell(Vec3(2, 0, 0)), withinAbsT(-delta, exactMargin<T>()));

  // Deep in the original interior (far from both the outer and inner shell boundary): the
  // distance to the surface is large and positive after taking |.|, so the shell value there is
  // positive (outside the shell -- it's been hollowed out).
  REQUIRE(shell(Vec3::zeros()) > T(0.0));

  // Exactly at the inner shell boundary (distance `delta` inside the original surface): value 0.
  REQUIRE_THAT(shell(Vec3(2 - delta, 0, 0)), withinAbsT(T(0.0), formulaMargin<T>()));
}

TEMPLATE_TEST_CASE("Annular: matches the absolute wrapped value minus the thickness",
                   "[Transform][Annular]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Box<T>      box   = asymmetricBox<T>();
  const T           delta = T(0.2);
  const Compiled<T> shell(Annular(box, delta));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(shell(p), withinAbsT(std::abs(box.signedDistance(p)) - delta, exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Blur()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Blur: alpha = 1 (no blur) reproduces the wrapped function exactly",
                   "[Transform][Blur]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T>   sphere(Vec3::zeros(), T(1));
  const Compiled<T> noBlur(Blur(sphere, T(0.3), T(1.0)));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(noBlur(p), withinAbsT(sphere.signedDistance(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Blur: sample weights sum to one, for any alpha", "[Transform][Blur]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T        = TestType;
  using Constant = TestUtils::ConstantSDF<T>;

  const Constant constant{T(7.0)};

  for (const T alpha : {T(0.0), T(0.25), T(0.5), T(0.75), T(1.0)}) {
    const Compiled<T, Constant> blurred(Blur(constant, T(0.3), alpha));

    for (const auto& p : samplePoints<T>()) {
      REQUIRE_THAT(blurred(p), withinAbsT(T(7.0), formulaMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Blur: matches the 27-tap weighted average of the wrapped function",
                   "[Transform][Blur]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Box<T> box = asymmetricBox<T>();
  const T      d   = T(0.3);

  for (const T alpha : {T(0.5), T(0.8)}) {
    const Compiled<T> blurred(Blur(box, d, alpha));

    // A tap m steps from the centre has weight alpha^(3-m) ((1 - alpha)/2)^m.
    const T beta = (T(1) - alpha) / T(2);

    for (const auto& p : samplePoints<T>()) {
      T expected = T(0);

      for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
          for (int k = -1; k <= 1; k++) {
            const int m = std::abs(i) + std::abs(j) + std::abs(k);
            const T   w = std::pow(alpha, T(3 - m)) * std::pow(beta, T(m));

            expected += w * box.signedDistance(p + d * Vec3(T(i), T(j), T(k)));
          }
        }
      }

      REQUIRE_THAT(blurred(p), withinAbsT(expected, formulaMargin<T>()));
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Mollify()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Mollify: a zero distance disables smoothing and reproduces the wrapped function exactly",
                   "[Transform][Mollify]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T>   sphere(Vec3::zeros(), T(1));
  const Compiled<T> noSmoothing(Mollify(sphere, T(0.0), 4));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(noSmoothing(p), withinAbsT(sphere.signedDistance(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Mollify: one sample per axis disables smoothing and reproduces the wrapped function exactly",
                   "[Transform][Mollify]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T>   sphere(Vec3::zeros(), T(1));
  const Compiled<T> noSmoothing(Mollify(sphere, T(0.5), 1));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(noSmoothing(p), withinAbsT(sphere.signedDistance(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Mollify: sample weights are normalized to sum to one",
                   "[Transform][Mollify]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T        = TestType;
  using Constant = TestUtils::ConstantSDF<T>;

  const Compiled<T, Constant> mollified(Mollify(Constant{T(-3.5)}, T(0.4), 3));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(mollified(p), withinAbsT(T(-3.5), formulaMargin<T>()));
  }
}

namespace {

// |x|^2, a convex function whose mollified value at the origin is the kernel's weighted mean of
// |offset|^2 over the sample grid.
template <class T>
struct SquaredNormSDF
{
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return a_point.length2();
  }

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(-Vec3T<T>::max(), Vec3T<T>::max());
  }

  static constexpr DistanceQuality distanceQuality = DistanceQuality::NotADistance;
};

} // namespace

TEMPLATE_TEST_CASE("Mollify: the default kernel is non-negative and weights the centre most",
                   "[Transform][Mollify]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Over the sample grid in [-r, r]^3, the unweighted mean of |offset|^2 is three times the mean of
  // the squared 1D sample positions. A kernel that peaks at the centre and decays outward pulls the
  // weighted mean below that; with non-negative weights it stays positive. The old kernel (the
  // sphere SDF |o| - r) gave the centre a negative weight and the corners the largest, pushing the
  // mean above it.
  const T r = T(0.5);

  for (const size_t samples : {size_t(3), size_t(5)}) {
    T uniformMean = T(0);

    for (size_t k = 0; k < samples; k++) {
      const T x = -r + T(2) * r * T(k) / T(samples - 1);

      uniformMean += T(3) * x * x / T(samples);
    }

    const Compiled<T, SquaredNormSDF<T>> mollified(Mollify(SquaredNormSDF<T>{}, r, samples));
    const T                              mean = mollified(Vec3T<T>::zeros());

    INFO("samples = " << samples << ", uniform mean = " << uniformMean);
    REQUIRE(mean > T(0));
    REQUIRE(mean < uniformMean);
  }
}

TEMPLATE_TEST_CASE("Mollify: a small smoothing distance gives a finite value close to the wrapped function",
                   "[Transform][Mollify]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Sphere<T>   sphere(Vec3T<T>::zeros(), T(1));
  const Compiled<T> mollified(Mollify(sphere, T(0.25), 3));

  // Mollify()'s kernel shape is an implementation detail; just confirm it produces *some* smoothed
  // value that is finite and reasonably close to the unsmoothed value for a small smoothing
  // distance, rather than asserting exact agreement with a hand-picked kernel here.
  for (const auto& p : samplePoints<T>()) {
    REQUIRE(std::isfinite(mollified(p)));
    REQUIRE_THAT(mollified(p), withinAbsT(sphere.signedDistance(p), 0.5));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Elongate()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Elongate: matches an equivalent capsule-like union built from translated spheres",
                   "[Transform][Elongate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Elongating a sphere along x by `e` is exactly a capsule: the union of every sphere whose
  // center slides from -e to +e along x. Check the two independent, easy-to-verify special
  // cases instead: the center of the elongated shape (must be as deep as the original sphere),
  // and a point beyond the elongated tip (must match a sphere translated to that tip).
  const Vec3      elongation(1.5, 0, 0);
  const Sphere<T> sphere(Vec3::zeros(), T(1));

  const Compiled<T> elongated(Elongate(sphere, elongation));

  // At the shape's own center: clamp(0, -e, e) = 0, so value equals the base sphere's own center value.
  REQUIRE_THAT(elongated(Vec3::zeros()), withinAbsT(sphere.signedDistance(Vec3::zeros()), exactMargin<T>()));

  // Beyond the elongated tip (x = 3): clamp(3, -1.5, 1.5) = 1.5, so this matches a sphere
  // translated to (1.5, 0, 0) evaluated at (3, 0, 0).
  const Sphere<T> tipSphere(Vec3(1.5, 0, 0), T(1));
  REQUIRE_THAT(elongated(Vec3(3, 0, 0)), withinAbsT(tipSphere.signedDistance(Vec3(3, 0, 0)), exactMargin<T>()));

  // Directly above the elongated "barrel" (not past either tip): clamp(0.5, -1.5, 1.5) = 0.5,
  // matching a sphere translated to (0.5, 0, 0).
  const Sphere<T> midSphere(Vec3(0.5, 0, 0), T(1));
  REQUIRE_THAT(elongated(Vec3(0.5, 1.5, 0)), withinAbsT(midSphere.signedDistance(Vec3(0.5, 1.5, 0)), exactMargin<T>()));
}

TEMPLATE_TEST_CASE("Elongate: matches the wrapped shape evaluated at p - clamp(p, -h, h)",
                   "[Transform][Elongate]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Vec3        h(1, 0.5, 0);
  const Box<T>      box = asymmetricBox<T>();
  const Compiled<T> elongated(Elongate(box, h));

  for (const auto& p : samplePoints<T>()) {
    Vec3 q;

    for (size_t dir = 0; dir < 3; dir++) {
      q[dir] = p[dir] - std::clamp(p[dir], -h[dir], h[dir]);
    }

    REQUIRE_THAT(elongated(p), withinAbsT(box.signedDistance(q), exactMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Reflect()
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Reflect: matches an equivalent sphere built directly at the mirrored center",
                   "[Transform][Reflect]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T> offsetSphere(Vec3(2, 3, -1), T(1));

  // Reflecting across the yz-plane (plane 0) flips x.
  const Compiled<T> reflectedX(Reflect(offsetSphere, size_t(0)));
  const Sphere<T>   mirroredX(Vec3(-2, 3, -1), T(1));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(reflectedX(p), withinAbsT(mirroredX.signedDistance(p), exactMargin<T>()));
  }

  // Reflecting across the xz-plane (plane 1) flips y.
  const Compiled<T> reflectedY(Reflect(offsetSphere, size_t(1)));
  const Sphere<T>   mirroredY(Vec3(2, -3, -1), T(1));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(reflectedY(p), withinAbsT(mirroredY.signedDistance(p), exactMargin<T>()));
  }

  // Reflecting across the xy-plane (plane 2) flips z.
  const Compiled<T> reflectedZ(Reflect(offsetSphere, size_t(2)));
  const Sphere<T>   mirroredZ(Vec3(2, 3, 1), T(1));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(reflectedZ(p), withinAbsT(mirroredZ.signedDistance(p), exactMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Reflect: reflecting twice across the same plane is the identity",
                   "[Transform][Reflect]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Sphere<T> sphere(Vec3(2, 3, -1), T(1));

  for (const size_t plane : {size_t(0), size_t(1), size_t(2)}) {
    const Compiled<T> twice(Reflect(Reflect(sphere, plane), plane));

    for (const auto& p : samplePoints<T>()) {
      REQUIRE_THAT(twice(p), withinAbsT(sphere.signedDistance(p), exactMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Reflect: matches the wrapped shape evaluated at the reflected point",
                   "[Transform][Reflect]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const Box<T> box = asymmetricBox<T>();

  for (const size_t plane : {size_t(0), size_t(1), size_t(2)}) {
    const Compiled<T> reflected(Reflect(box, plane));

    for (const auto& p : samplePoints<T>()) {
      REQUIRE_THAT(reflected(p), withinAbsT(box.signedDistance(reflectedPoint(p, plane)), exactMargin<T>()));
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Chains of point maps (folded into one affine map)
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("Point maps: a chain of Translate, Rotate, Scale and Reflect matches the maps composed by hand",
                   "[Transform][Affine]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Box<T> box = asymmetricBox<T>();
  const Vec3   shift(0.5, -1.5, 2);

  for (const T s : {T(2), T(-1.5)}) {
    // The outermost map is applied to the query point first: undo the translation, then the
    // rotation, then the scaling, then the reflection; the value is multiplied by |s|.
    const auto        chain = Translate(Rotate(Scale(Reflect(box, size_t(0)), s), T(30), size_t(2)), shift);
    const Compiled<T> compiled(chain);

    for (const auto& p : samplePoints<T>()) {
      const Vec3 q1 = p - shift;
      const Vec3 q2 = rotatedPoint(q1, T(-30), size_t(2));
      const Vec3 q3 = q2 / s;
      const Vec3 q4 = reflectedPoint(q3, size_t(0));

      REQUIRE_THAT(compiled(p), withinAbsT(std::abs(s) * box.signedDistance(q4), formulaMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("Point maps: consecutive maps of one kind combine as expected",
                   "[Transform][Affine]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Box<T> box = asymmetricBox<T>();

  // Two translations add, two scalings multiply, two rotations about the same axis add their angles.
  const Compiled<T> translated(Translate(Translate(box, Vec3(1, 0, -2)), Vec3(0.5, 3, 1)));
  const Compiled<T> scaled(Scale(Scale(box, T(2)), T(-1.5)));
  const Compiled<T> rotated(Rotate(Rotate(box, T(25), size_t(1)), T(40), size_t(1)));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(translated(p), withinAbsT(box.signedDistance(p - Vec3(1.5, 3, -1)), formulaMargin<T>()));
    REQUIRE_THAT(scaled(p), withinAbsT(T(3) * box.signedDistance(p / T(-3)), formulaMargin<T>()));
    REQUIRE_THAT(rotated(p), withinAbsT(box.signedDistance(rotatedPoint(p, T(-65), size_t(1))), formulaMargin<T>()));
  }
}

TEMPLATE_TEST_CASE("Point maps: rotations about different axes do not commute",
                   "[Transform][Affine]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Rotate(Rotate(box, a, x), b, z) turns the box about x first, then about z; its value at p is
  // the box's value at Rx(-a) Rz(-b) p. Swapping the order would give a different value.
  const Box<T>      box = asymmetricBox<T>();
  const Compiled<T> xThenZ(Rotate(Rotate(box, T(90), size_t(0)), T(90), size_t(2)));
  const Compiled<T> zThenX(Rotate(Rotate(box, T(90), size_t(2)), T(90), size_t(0)));

  for (const auto& p : samplePoints<T>()) {
    const Vec3 a = rotatedPoint(rotatedPoint(p, T(-90), size_t(2)), T(-90), size_t(0));
    const Vec3 b = rotatedPoint(rotatedPoint(p, T(-90), size_t(0)), T(-90), size_t(2));

    REQUIRE_THAT(xThenZ(p), withinAbsT(box.signedDistance(a), formulaMargin<T>()));
    REQUIRE_THAT(zThenX(p), withinAbsT(box.signedDistance(b), formulaMargin<T>()));
  }

  REQUIRE(std::abs(xThenZ(Vec3(1, 1, 1)) - zThenX(Vec3(1, 1, 1))) > T(0.1));
}

TEMPLATE_TEST_CASE("Point maps: a value operation between two maps stops them from folding",
                   "[Transform][Affine]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  // Scale(Offset(Translate(box, t), r), s): value at p is |s| (box(p / s - t) - r).
  const Box<T>      box = asymmetricBox<T>();
  const Vec3        t(1, -0.5, 0.25);
  const T           r = T(0.2);
  const T           s = T(2);
  const Compiled<T> compiled(Scale(Offset(Translate(box, t), r), s));

  for (const auto& p : samplePoints<T>()) {
    REQUIRE_THAT(compiled(p), withinAbsT(s * (box.signedDistance(p / s - t) - r), formulaMargin<T>()));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Bounding volumes of mapped shapes
// ─────────────────────────────────────────────────────────────────────────────

namespace {

template <class T>
void
requireBoxNear(const BoundingVolumes::AABBT<T>& a_box,
               const Vec3T<T>&                  a_lo,
               const Vec3T<T>&                  a_hi,
               const double                     a_margin)
{
  for (size_t dir = 0; dir < 3; dir++) {
    REQUIRE_THAT(a_box.getLowCorner()[dir], withinAbsT(a_lo[dir], a_margin));
    REQUIRE_THAT(a_box.getHighCorner()[dir], withinAbsT(a_hi[dir], a_margin));
  }
}

} // namespace

TEMPLATE_TEST_CASE("Point maps: the bounding volume of a translated or scaled box is the mapped box",
                   "[Transform][Affine]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const Box<T> box(Vec3(-1, -2, -3), Vec3(1, 2, 3));

  // Translation: both corners shift.
  {
    const auto        expr = Translate(box, Vec3(2, -1, 3));
    const Compiled<T> compiled(expr);

    requireBoxNear(expr.computeBoundingVolume(), Vec3(1, -3, 0), Vec3(3, 1, 6), exactMargin<T>());
    requireBoxNear(compiled.tape().computeBoundingVolume(), Vec3(1, -3, 0), Vec3(3, 1, 6), exactMargin<T>());
  }

  // Positive scaling: both corners scale about the origin.
  {
    const auto        expr = Scale(box, T(2));
    const Compiled<T> compiled(expr);

    requireBoxNear(expr.computeBoundingVolume(), Vec3(-2, -4, -6), Vec3(2, 4, 6), exactMargin<T>());
    requireBoxNear(compiled.tape().computeBoundingVolume(), Vec3(-2, -4, -6), Vec3(2, 4, 6), exactMargin<T>());
  }

  // Negative scaling of a box off the origin: the corners swap.
  {
    const auto        expr = Scale(Box<T>(Vec3(0, 0, 0), Vec3(1, 2, 3)), T(-2));
    const Compiled<T> compiled(expr);

    requireBoxNear(expr.computeBoundingVolume(), Vec3(-2, -4, -6), Vec3(0, 0, 0), exactMargin<T>());
    requireBoxNear(compiled.tape().computeBoundingVolume(), Vec3(-2, -4, -6), Vec3(0, 0, 0), exactMargin<T>());
  }

  // Scale, then translate: the box scales about the origin, then shifts.
  {
    const auto        expr = Translate(Scale(box, T(0.5)), Vec3(10, 0, -1));
    const Compiled<T> compiled(expr);

    requireBoxNear(expr.computeBoundingVolume(), Vec3(9.5, -1, -2.5), Vec3(10.5, 1, 0.5), exactMargin<T>());
    requireBoxNear(compiled.tape().computeBoundingVolume(), Vec3(9.5, -1, -2.5), Vec3(10.5, 1, 0.5), exactMargin<T>());
  }

  // A quarter turn about z swaps the x and y extents.
  {
    const auto expr = Rotate(box, T(90), size_t(2));

    requireBoxNear(expr.computeBoundingVolume(), Vec3(-2, -1, -3), Vec3(2, 1, 3), formulaMargin<T>());
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// Argument checks: EBGEOMETRY_REQUIREs, so these abort in every build.
// ─────────────────────────────────────────────────────────────────────────────

namespace {

SphereSDF<double>
unitSphere()
{
  return SphereSDF<double>(Vec3T<double>::zeros(), 1.0);
}

} // namespace

TEST_CASE("Translate: rejects a non-finite shift", "[Transform][Translate]")
{
  using Vec3 = Vec3T<double>;

  const double inf = std::numeric_limits<double>::infinity();

  REQUIRE(abortsWith([inf] { (void)Translate(unitSphere(), Vec3(0, inf, 0)); }, "Translate: the shift must be finite"));
}

TEST_CASE("Rotate: rejects an invalid axis index, a zero axis direction and a non-finite angle", "[Transform][Rotate]")
{
  using Vec3 = Vec3T<double>;

  const double nan = std::numeric_limits<double>::quiet_NaN();

  REQUIRE(
    abortsWith([] { (void)Rotate(unitSphere(), 45.0, size_t(3)); }, "Rotate: the rotation axis must be 0, 1 or 2 (3)"));
  REQUIRE(abortsWith([] { (void)Rotate(unitSphere(), 45.0, Vec3::zeros()); },
                     "Rotate: the rotation axis must be finite and non-zero (0, 0, 0)"));
  REQUIRE(
    abortsWith([nan] { (void)Rotate(unitSphere(), nan, size_t(0)); }, "Rotate: the rotation angle must be finite"));
}

TEST_CASE("Scale: rejects a zero scale factor", "[Transform][Scale]")
{
  REQUIRE(
    abortsWith([] { (void)Scale(unitSphere(), 0.0); }, "Scale: the scale factor must be finite and non-zero (0)"));
}

TEST_CASE("Offset: rejects a non-finite offset", "[Transform][Offset]")
{
  const double nan = std::numeric_limits<double>::quiet_NaN();

  REQUIRE(abortsWith([nan] { (void)Offset(unitSphere(), nan); }, "Offset: the offset must be finite"));
}

TEST_CASE("Annular: rejects a non-finite thickness", "[Transform][Annular]")
{
  const double inf = std::numeric_limits<double>::infinity();

  REQUIRE(abortsWith([inf] { (void)Annular(unitSphere(), inf); }, "Annular: the shell thickness must be finite"));
}

TEST_CASE("Blur: rejects a negative distance and alpha outside [0, 1]", "[Transform][Blur]")
{
  REQUIRE(abortsWith([] { (void)Blur(unitSphere(), -0.1); },
                     "Blur: the blur distance must be finite and non-negative (-0.1)"));
  REQUIRE(
    abortsWith([] { (void)Blur(unitSphere(), 0.1, 1.5); }, "Blur: the blending weight alpha must lie in [0, 1] (1.5)"));
}

TEST_CASE("Mollify: rejects a sample count outside [1, 64]", "[Transform][Mollify]")
{
  REQUIRE(abortsWith([] { (void)Mollify(unitSphere(), 0.1, 0); },
                     "Mollify: the number of samples per axis must be between 1 and 64 (0)"));
  REQUIRE(abortsWith([] { (void)Mollify(unitSphere(), 0.1, 65); },
                     "Mollify: the number of samples per axis must be between 1 and 64 (65)"));
}

TEST_CASE("Elongate: rejects a negative elongation", "[Transform][Elongate]")
{
  using Vec3 = Vec3T<double>;

  REQUIRE(abortsWith([] { (void)Elongate(unitSphere(), Vec3(1, -1, 0)); },
                     "Elongate: the elongation must be finite and non-negative (1, -1, 0)"));
}

TEST_CASE("Reflect: rejects an invalid plane", "[Transform][Reflect]")
{
  REQUIRE(
    abortsWith([] { (void)Reflect(unitSphere(), size_t(3)); }, "Reflect: the reflection plane must be 0, 1 or 2 (3)"));
}
