// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// requireExactSDF(): a check that a shape is an exact signed distance function, shared by the analytic shapes' tests.

#ifndef EBGEOMETRY_TEST_EXACTSDF_HPP
#define EBGEOMETRY_TEST_EXACTSDF_HPP

#include "EBGeometry.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>

// Checks that a_shape behaves as an exact signed distance function at random points in the cube
// [-a_extent, a_extent]^3. For an exact SDF f and a point p off the medial axis, |grad f| = 1, and
// stepping back along the gradient by f(p) lands on the surface: f(p - f(p) grad f(p)) = 0. A
// formula that is constant somewhere (zero gradient), or that over- or underestimates the distance,
// fails one of the two. Points near the medial axis, where the gradient is discontinuous and a
// central difference straddles the ridge, are skipped; they must be rare. f must also be
// 1-Lipschitz between any two sample points.
template <class T, class Shape>
inline void
requireExactSDF(const Shape& a_shape, const T a_extent)
{
  using Vec3 = EBGeometry::Vec3T<T>;

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

    // The gradient by central differences with the step h and with half of it: where they differ, the wider
    // step straddles a ridge, even if the gradient it gives is close to unit length.
    Vec3 grad;
    Vec3 half;

    for (size_t d = 0; d < 3; d++) {
      const Vec3 e = h * Vec3::unit(d);

      grad[d] = (a_shape.signedDistance(p + e) - a_shape.signedDistance(p - e)) / (T(2) * h);
      half[d] = (a_shape.signedDistance(p + T(0.5) * e) - a_shape.signedDistance(p - T(0.5) * e)) / h;
    }

    if (std::abs(grad.length() - T(1)) > tol || (grad - half).length() > tol) {
      ridgePoints++;

      continue;
    }

    const Vec3 onSurface = p - f * grad / grad.length();

    REQUIRE(std::abs(a_shape.signedDistance(onSurface)) <= tol * std::max(T(1), std::abs(f)));
  }

  REQUIRE(ridgePoints <= numPoints / 50);
}

#endif
