// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A file that includes the shapes and EBGeometry_Fused.hpp, and no other header of the library, must see the
// ray forms of the fused compositions, the BVH unions and the offsets: firstHit() is a template (declared
// with the shapes), and a file that saw no ray form would march what another file traces exactly, two
// definitions of one function. This file includes nothing else of the library on purpose.

#include "Source/EBGeometry_AnalyticDistanceFunctions.hpp"
#include "Source/EBGeometry_Fused.hpp"
#include "TestFloatingPointUtils.hpp"

#include <type_traits>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

TEMPLATE_TEST_CASE(
  "The shapes and EBGeometry_Fused.hpp alone: compositions, BVH unions and offsets have their ray forms",
  "[Ray][headers]",
  EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const SphereSDF<T> ball(Vec3::zeros(), T(1));
  const BoxSDF<T>    box(Vec3(T(1.5), -T(0.5), -T(0.5)), Vec3(T(2.5), T(0.5), T(0.5)));

  const auto composition = Fused::Union(ball, box);
  const auto offset      = Fused::Offset(ball, T(0.25));

  CHECK(hasRayForm<std::decay_t<decltype(composition)>, T>);
  CHECK(hasRayForm<std::decay_t<decltype(offset)>, T>);
  CHECK(hasRayForm<BVHUnion<T, SphereSDF<T>, 4>, T>);

  // Traced, not marched: the hit is exact, and at the closed-form distance.
  const RayT<T> ray{Vec3(-T(3), T(0), T(0)), Vec3(T(1), T(0), T(0))};

  const auto onComposition = firstHit(composition, ray, T(0), T(10));
  const auto onOffset      = firstHit(offset, ray, T(0), T(10));

  REQUIRE(onComposition.hit());
  REQUIRE(onOffset.hit());
  CHECK(onComposition.m_exact);
  CHECK(onOffset.m_exact);
  CHECK_THAT(onComposition.m_t, withinAbsT<T>(T(2), 1e-6));
  CHECK_THAT(onOffset.m_t, withinAbsT<T>(T(1.75), 1e-6));
}
