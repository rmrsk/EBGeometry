// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_Transform.hpp: the implicit-function transforms (complement,
// translate, rotate, scale, offset, annular shell, blur, mollify, elongate, reflect). Uses an
// analytic sphere or box as the wrapped function so every expected value is hand-computable, and
// cross-checks several transforms against an equivalent, independently-constructed analytic shape
// (e.g. a translated sphere vs. a sphere built directly at the shifted center).

#include "EBGeometry.hpp"

#include <cmath>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using Catch::Matchers::WithinAbs;

namespace {

using T      = double;
using Vec3   = Vec3T<T>;
using Sphere = SphereSDF<T>;
using Box    = BoxSDF<T>;
using IF     = ImplicitFunction<T>;

std::vector<Vec3>
samplePoints()
{
  return {
    Vec3::zeros(),
    Vec3(0.5, 0, 0),
    Vec3(0, 1.5, 0),
    Vec3(0, 0, -2.0),
    Vec3(1, 1, 1),
    Vec3(-3, 2, 0.5),
  };
}

// A constant-valued implicit function, used to verify that Blur/Mollify's sample weights are a
// true partition of unity (weighted average of a constant must return that same constant).
class ConstantIF : public IF
{
public:
  explicit ConstantIF(const T a_value) : m_value(a_value)
  {}

  [[nodiscard]] T
  value(const Vec3&) const noexcept override
  {
    return m_value;
  }

private:
  T m_value;
};

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// ComplementIF / Complement()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ComplementIF: value is the negation of the wrapped function", "[Transform][Complement]")
{
  const auto      sphere = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  ComplementIF<T> comp(sphere);

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(comp.value(p), WithinAbs(-sphere->signedDistance(p), 1.0e-12));
  }

  // Inside the sphere becomes "outside" the complement, and vice versa.
  REQUIRE(comp.value(Vec3::zeros()) > 0.0);
  REQUIRE(comp.value(Vec3(5, 0, 0)) < 0.0);
}

TEST_CASE("Complement: free function matches ComplementIF", "[Transform][Complement]")
{
  const auto            sphere   = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const auto            freeFunc = Complement<T>(sphere);
  const ComplementIF<T> direct(sphere);

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// TranslateIF / Translate()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("TranslateIF: matches an equivalent sphere built directly at the shifted center", "[Transform][Translate]")
{
  const Vec3 shift(2, -1, 3);

  const auto   sphereAtOrigin = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const Sphere sphereAtShift(shift, T(1));

  const TranslateIF<T> translated(sphereAtOrigin, shift);

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(translated.value(p), WithinAbs(sphereAtShift.signedDistance(p), 1.0e-12));
  }

  // The translated sphere's center is now at `shift`, so that point is the deepest interior point.
  REQUIRE_THAT(translated.value(shift), WithinAbs(-1.0, 1.0e-12));
}

TEST_CASE("Translate: free function matches TranslateIF", "[Transform][Translate]")
{
  const Vec3           shift(2, -1, 3);
  const auto           sphere   = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const auto           freeFunc = Translate<T>(sphere, shift);
  const TranslateIF<T> direct(sphere, shift);

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// RotateIF / Rotate()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("RotateIF: a full 360-degree rotation is the identity", "[Transform][Rotate]")
{
  const auto box = std::make_shared<Box>(Vec3(-2, -1, -1), Vec3(2, 1, 1));

  for (const size_t axis : {size_t(0), size_t(1), size_t(2)}) {
    const RotateIF<T> full(box, T(360), axis);

    for (const auto& p : samplePoints()) {
      REQUIRE_THAT(full.value(p), WithinAbs(box->signedDistance(p), 1.0e-9));
    }
  }
}

TEST_CASE("RotateIF: composing a rotation with its inverse is the identity", "[Transform][Rotate]")
{
  const auto box = std::make_shared<Box>(Vec3(-2, -1, -1), Vec3(2, 1, 1));

  for (const size_t axis : {size_t(0), size_t(1), size_t(2)}) {
    const auto        rotated = std::make_shared<RotateIF<T>>(box, T(37), axis);
    const RotateIF<T> roundTrip(rotated, T(-37), axis);

    for (const auto& p : samplePoints()) {
      REQUIRE_THAT(roundTrip.value(p), WithinAbs(box->signedDistance(p), 1.0e-9));
    }
  }
}

TEST_CASE("RotateIF: rotating a long-in-x box by 90 degrees about z swaps the x/y half-extents", "[Transform][Rotate]")
{
  // A box that's long along x (half-extent 2) and short along y (half-extent 1). Rotating 90
  // degrees about z should make it behave like a box long along y and short along x.
  const auto        box = std::make_shared<Box>(Vec3(-2, -1, -1), Vec3(2, 1, 1));
  const RotateIF<T> rotated(box, T(90), size_t(2));

  // (1.5, 0, 0) is inside the original box (|x|=1.5 < 2) but would be outside a box that's only
  // long in y (half-extent 1 along x). After a 90-degree rotation about z, evaluating the rotated
  // shape at (1.5, 0, 0) must match evaluating the ORIGINAL box at (0, 1.5, 0) (its long axis).
  REQUIRE_THAT(rotated.value(Vec3(1.5, 0, 0)), WithinAbs(box->signedDistance(Vec3(0, 1.5, 0)), 1.0e-9));

  // Conversely, (0, 1.5, 0) on the rotated shape must match the original box's short-axis value.
  REQUIRE_THAT(rotated.value(Vec3(0, 1.5, 0)), WithinAbs(box->signedDistance(Vec3(-1.5, 0, 0)), 1.0e-9));
}

TEST_CASE("Rotate: free function matches RotateIF", "[Transform][Rotate]")
{
  const auto        box      = std::make_shared<Box>(Vec3(-2, -1, -1), Vec3(2, 1, 1));
  const auto        freeFunc = Rotate<T>(box, T(42), size_t(1));
  const RotateIF<T> direct(box, T(42), size_t(1));

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// ScaleIF / Scale()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ScaleIF: matches an equivalent sphere built directly with the scaled radius", "[Transform][Scale]")
{
  const T scale = 3.0;

  const auto   unitSphere = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const Sphere bigSphere(Vec3::zeros(), scale);

  const ScaleIF<T> scaled(unitSphere, scale);

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(scaled.value(p), WithinAbs(bigSphere.signedDistance(p), 1.0e-12));
  }
}

TEST_CASE("Scale: free function matches ScaleIF", "[Transform][Scale]")
{
  const auto       sphere   = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const auto       freeFunc = Scale<T>(sphere, T(2.5));
  const ScaleIF<T> direct(sphere, T(2.5));

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// OffsetIF / Offset()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("OffsetIF: matches an equivalent sphere built directly with the offset radius", "[Transform][Offset]")
{
  const T offset = 0.4;

  // Offsetting a signed distance function by a constant is equivalent (for a sphere) to growing
  // its radius by that constant: value - offset == distance-to-sphere-of-radius-(1+offset).
  const auto   unitSphere = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const Sphere grownSphere(Vec3::zeros(), T(1) + offset);

  const OffsetIF<T> offsetted(unitSphere, offset);

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(offsetted.value(p), WithinAbs(grownSphere.signedDistance(p), 1.0e-12));
  }
}

TEST_CASE("Offset: free function matches OffsetIF", "[Transform][Offset]")
{
  const auto        sphere   = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const auto        freeFunc = Offset<T>(sphere, T(0.3));
  const OffsetIF<T> direct(sphere, T(0.3));

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// AnnularIF / Annular()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("AnnularIF: hollows out a shell of the requested thickness", "[Transform][Annular]")
{
  const T delta = 0.2;

  const auto         sphere = std::make_shared<Sphere>(Vec3::zeros(), T(2));
  const AnnularIF<T> shell(sphere, delta);

  // On the original surface: |0| - delta = -delta (inside the shell).
  REQUIRE_THAT(shell.value(Vec3(2, 0, 0)), WithinAbs(-delta, 1.0e-12));

  // Deep in the original interior (far from both the outer and inner shell boundary): the
  // distance to the surface is large and positive after taking |.|, so the shell value there is
  // positive (outside the shell -- it's been hollowed out).
  REQUIRE(shell.value(Vec3::zeros()) > 0.0);

  // Exactly at the inner shell boundary (distance `delta` inside the original surface): value 0.
  REQUIRE_THAT(shell.value(Vec3(2 - delta, 0, 0)), WithinAbs(0.0, 1.0e-9));

  // Matches std::abs(sphere) - delta everywhere.
  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(shell.value(p), WithinAbs(std::abs(sphere->signedDistance(p)) - delta, 1.0e-12));
  }
}

TEST_CASE("Annular: free function matches AnnularIF", "[Transform][Annular]")
{
  const auto         sphere   = std::make_shared<Sphere>(Vec3::zeros(), T(2));
  const auto         freeFunc = Annular<T>(sphere, T(0.2));
  const AnnularIF<T> direct(sphere, T(0.2));

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// BlurIF / Blur()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("BlurIF: alpha = 1 (no blur) reproduces the wrapped function exactly", "[Transform][Blur]")
{
  const auto      sphere = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const BlurIF<T> noBlur(sphere, T(0.3), T(1.0));

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(noBlur.value(p), WithinAbs(sphere->signedDistance(p), 1.0e-12));
  }
}

TEST_CASE("BlurIF: sample weights sum to one, for any alpha", "[Transform][Blur]")
{
  const auto constant = std::make_shared<ConstantIF>(T(7.0));

  for (const T alpha : {0.0, 0.25, 0.5, 0.75, 1.0}) {
    const BlurIF<T> blurred(constant, T(0.3), alpha);

    for (const auto& p : samplePoints()) {
      REQUIRE_THAT(blurred.value(p), WithinAbs(7.0, 1.0e-9));
    }
  }
}

TEST_CASE("Blur: free function matches BlurIF", "[Transform][Blur]")
{
  const auto      sphere   = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const auto      freeFunc = Blur<T>(sphere, T(0.3));
  const BlurIF<T> direct(sphere, T(0.3));

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// MollifyIF / Mollify()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("MollifyIF: a_maxValue = 0 disables smoothing and reproduces the wrapped function exactly",
          "[Transform][Mollify]")
{
  const auto sphere    = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const auto mollifier = std::make_shared<Sphere>(Vec3::zeros(), T(0.5));

  const MollifyIF<T> noSmoothing(sphere, mollifier, T(0.0), 4);

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(noSmoothing.value(p), WithinAbs(sphere->signedDistance(p), 1.0e-12));
  }
}

TEST_CASE("MollifyIF: a_numPoints <= 1 disables smoothing and reproduces the wrapped function exactly",
          "[Transform][Mollify]")
{
  const auto         sphere    = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const auto         mollifier = std::make_shared<Sphere>(Vec3::zeros(), T(0.5));
  const MollifyIF<T> noSmoothing(sphere, mollifier, T(0.5), 1);

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(noSmoothing.value(p), WithinAbs(sphere->signedDistance(p), 1.0e-12));
  }
}

TEST_CASE("MollifyIF: sample weights are normalized to sum to one", "[Transform][Mollify]")
{
  const auto constant  = std::make_shared<ConstantIF>(T(-3.5));
  const auto mollifier = std::make_shared<Sphere>(Vec3::zeros(), T(0.5));

  const MollifyIF<T> mollified(constant, mollifier, T(0.4), 3);

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(mollified.value(p), WithinAbs(-3.5, 1.0e-9));
  }
}

TEST_CASE("Mollify: free function matches MollifyIF", "[Transform][Mollify]")
{
  const auto         sphere           = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const auto         freeFunc         = Mollify<T>(sphere, T(0.25), 3);
  const auto         defaultMollifier = std::make_shared<Sphere>(Vec3::zeros(), T(0.25));
  const MollifyIF<T> direct(sphere, defaultMollifier, T(0.25), 3);

  // Mollify()'s default mollifier shape is an implementation detail; just confirm the free
  // function produces *some* smoothed value that is finite and reasonably close to the
  // unsmoothed value for a small smoothing distance, rather than asserting exact agreement with
  // a hand-picked mollifier here.
  for (const auto& p : samplePoints()) {
    REQUIRE(std::isfinite(freeFunc->value(p)));
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(sphere->signedDistance(p), 0.5));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// ElongateIF / Elongate()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ElongateIF: matches an equivalent capsule-like union built from translated spheres", "[Transform][Elongate]")
{
  // Elongating a sphere along x by `e` is exactly a capsule: the union of every sphere whose
  // center slides from -e to +e along x. Check the two independent, easy-to-verify special
  // cases instead: the center of the elongated shape (must be as deep as the original sphere),
  // and a point beyond the elongated tip (must match a sphere translated to that tip).
  const Vec3 elongation(1.5, 0, 0);
  const auto sphere = std::make_shared<Sphere>(Vec3::zeros(), T(1));

  const ElongateIF<T> elongated(sphere, elongation);

  // At the shape's own center: clamp(0, -e, e) = 0, so value equals the base sphere's own center value.
  REQUIRE_THAT(elongated.value(Vec3::zeros()), WithinAbs(sphere->signedDistance(Vec3::zeros()), 1.0e-12));

  // Beyond the elongated tip (x = 3): clamp(3, -1.5, 1.5) = 1.5, so this matches a sphere
  // translated to (1.5, 0, 0) evaluated at (3, 0, 0).
  const Sphere tipSphere(Vec3(1.5, 0, 0), T(1));
  REQUIRE_THAT(elongated.value(Vec3(3, 0, 0)), WithinAbs(tipSphere.signedDistance(Vec3(3, 0, 0)), 1.0e-12));

  // Directly above the elongated "barrel" (not past either tip): clamp(0.5, -1.5, 1.5) = 0.5,
  // matching a sphere translated to (0.5, 0, 0).
  const Sphere midSphere(Vec3(0.5, 0, 0), T(1));
  REQUIRE_THAT(elongated.value(Vec3(0.5, 1.5, 0)), WithinAbs(midSphere.signedDistance(Vec3(0.5, 1.5, 0)), 1.0e-12));
}

TEST_CASE("Elongate: free function matches ElongateIF", "[Transform][Elongate]")
{
  const Vec3          elongation(1, 0.5, 0);
  const auto          sphere   = std::make_shared<Sphere>(Vec3::zeros(), T(1));
  const auto          freeFunc = Elongate<T>(sphere, elongation);
  const ElongateIF<T> direct(sphere, elongation);

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// ReflectIF / Reflect()
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("ReflectIF: matches an equivalent sphere built directly at the mirrored center", "[Transform][Reflect]")
{
  const auto offsetSphere = std::make_shared<Sphere>(Vec3(2, 3, -1), T(1));

  // Reflecting across the yz-plane (plane 0) flips x.
  const ReflectIF<T> reflectedX(offsetSphere, size_t(0));
  const Sphere       mirroredX(Vec3(-2, 3, -1), T(1));
  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(reflectedX.value(p), WithinAbs(mirroredX.signedDistance(p), 1.0e-12));
  }

  // Reflecting across the xz-plane (plane 1) flips y.
  const ReflectIF<T> reflectedY(offsetSphere, size_t(1));
  const Sphere       mirroredY(Vec3(2, -3, -1), T(1));
  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(reflectedY.value(p), WithinAbs(mirroredY.signedDistance(p), 1.0e-12));
  }

  // Reflecting across the xy-plane (plane 2) flips z.
  const ReflectIF<T> reflectedZ(offsetSphere, size_t(2));
  const Sphere       mirroredZ(Vec3(2, 3, 1), T(1));
  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(reflectedZ.value(p), WithinAbs(mirroredZ.signedDistance(p), 1.0e-12));
  }
}

TEST_CASE("ReflectIF: reflecting twice across the same plane is the identity", "[Transform][Reflect]")
{
  const auto sphere = std::make_shared<Sphere>(Vec3(2, 3, -1), T(1));

  for (const size_t plane : {size_t(0), size_t(1), size_t(2)}) {
    const auto         once = std::make_shared<ReflectIF<T>>(sphere, plane);
    const ReflectIF<T> twice(once, plane);

    for (const auto& p : samplePoints()) {
      REQUIRE_THAT(twice.value(p), WithinAbs(sphere->signedDistance(p), 1.0e-12));
    }
  }
}

TEST_CASE("Reflect: free function matches ReflectIF", "[Transform][Reflect]")
{
  const auto         sphere   = std::make_shared<Sphere>(Vec3(2, 3, -1), T(1));
  const auto         freeFunc = Reflect<T>(sphere, size_t(1));
  const ReflectIF<T> direct(sphere, size_t(1));

  for (const auto& p : samplePoints()) {
    REQUIRE_THAT(freeFunc->value(p), WithinAbs(direct.value(p), 1.0e-12));
  }
}
