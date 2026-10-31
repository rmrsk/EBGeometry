// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A file that includes the shapes and EBGeometry_BVHUnion.hpp, and no other header of the library, must see
// the ray form of the BVH unions: firstHit() is a template (declared with the shapes), and a file that saw no
// ray form would march what another file traces exactly, two definitions of one function. This file includes
// nothing else of the library on purpose.

#include "Source/EBGeometry_AnalyticDistanceFunctions.hpp"
#include "Source/EBGeometry_BVHUnion.hpp"
#include "TestFloatingPointUtils.hpp"

#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

TEMPLATE_TEST_CASE("The shapes and EBGeometry_BVHUnion.hpp alone: a BVH union has its ray form",
                   "[Ray][headers]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  const std::vector<SphereSDF<T>> balls{SphereSDF<T>(Vec3::zeros(), T(1)), SphereSDF<T>(Vec3(T(3), T(0), T(0)), T(1))};
  const std::vector<BoundingVolumes::AABBT<T>> boxes{balls[0].computeBoundingVolume(),
                                                     balls[1].computeBoundingVolume()};

  Pool pool(hostMemoryResource());

  const BVHUnion<T, SphereSDF<T>, 4> scene(pool, balls, boxes);

  CHECK(hasRayForm<BVHUnion<T, SphereSDF<T>, 4>, T>);

  // Traced, not marched: the hit is exact, and at the closed-form distance.
  const RayT<T> ray{Vec3(-T(3), T(0), T(0)), Vec3(T(1), T(0), T(0))};
  const auto    hit = firstHit(scene, ray, T(0), T(10));

  REQUIRE(hit.hit());
  CHECK(hit.m_exact);
  CHECK_THAT(hit.m_t, withinAbsT<T>(T(2), 1e-6));
}
