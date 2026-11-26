// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of the shape families (TestAnalyticShapeFamilies.cpp): their signed distances and ray crossings
// on a device against the host. They are apart from TestAnalyticShapeFamilies.cpp so that a GPU build compiles only
// them with the device compiler.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

namespace {

// One signed distance per query point, for any shape held by value, as a kernel receives it.
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

// The distance to the first crossing, or -1 for none.
template <class T, class Shape>
struct FirstCrossing
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const RayT<T>& a_ray) const noexcept
  {
    const HitT<T> hit = firstHit(m_shape, a_ray, T(0), T(10));

    return hit.hit() ? hit.m_t : -T(1);
  }
};

} // namespace

TEMPLATE_TEST_CASE("Shape families: device signedDistance and ray crossings match the host",
                   "[AnalyticSDF][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const auto points = queryGrid<T>(Vec3(T(-1.5), T(-1.5), T(-1.5)), Vec3(T(1.5), T(1.5), T(1.5)), 15);

  const auto check = [&](const auto& a_shape, const char* a_name) {
    using Shape = std::decay_t<decltype(a_shape)>;

    INFO(a_name);
    requireSameResults(evaluateOnDevice<T>(ShapeDistanceQuery<T, Shape>{a_shape}, points),
                       evaluateOnHost<T>(ShapeDistanceQuery<T, Shape>{a_shape}, points));
  };

  const auto icosahedron = ConvexPolyhedronSDF<T>::icosahedron(Vec3(T(0.1), T(0), T(0)), T(0.9));
  const auto frame       = BoxFrameSDF<T>(Vec3(T(-0.6), T(-0.5), T(-0.4)), Vec3(T(0.5), T(0.6), T(0.7)), T(0.15));
  const auto hyperboloid = QuadricSDF<T>::hyperboloid(Vec3::zeros(), T(0.5), T(60));
  const auto two         = QuadricSDF<T>::hyperboloidTwoSheets(Vec3::zeros(), T(0.4), T(90));
  const auto paraboloid  = QuadricSDF<T>::paraboloid(Vec3(T(0), T(0.5), T(0)), T(0.5));

  check(icosahedron, "icosahedron");
  check(frame, "BoxFrameSDF");
  check(hyperboloid, "hyperboloid");
  check(two, "hyperboloidTwoSheets");
  check(paraboloid, "paraboloid");
  check(SuperellipsoidSDF<T>(Vec3::zeros(), Vec3(T(1), T(0.6), T(0.8)), T(0.3), T(1.5)), "SuperellipsoidSDF");
  check(SolidAngleSDF<T>(Vec3::zeros(), T(1), T(250)), "SolidAngleSDF");
  check(VesicaSDF<T>(Vec3(T(0), T(-1), T(0)), Vec3(T(0.2), T(1), T(0)), T(0.4)), "VesicaSDF");
  check(HelixSDF<T>(Vec3(T(0), T(-1), T(0)), Vec3(T(0), T(1), T(0)), T(0.7), T(4.5), T(0.1), Vec3::unit(0)),
        "HelixSDF");
  check(LatticeSDF<T>(LatticeSDF<T>::Kind::Neovius, Vec3::ones(), T(0.1)), "LatticeSDF");

  // The rays from a sphere of radius 3 toward points near the origin.
  std::vector<RayT<T>> rays;

  for (const Vec3& target : queryGrid<T>(-Vec3::ones(), Vec3::ones(), 6)) {
    const Vec3 origin = T(3) * Vec3(T(0.6), T(0.48), T(0.64));
    const Vec3 dir    = target - origin;

    rays.push_back(RayT<T>{origin, dir / dir.length()});
  }

  const auto checkRays = [&](const auto& a_shape) {
    using S = std::decay_t<decltype(a_shape)>;

    requireSameResults(evaluateOnDevice<T>(FirstCrossing<T, S>{a_shape}, rays),
                       evaluateOnHost<T>(FirstCrossing<T, S>{a_shape}, rays));
  };

  checkRays(icosahedron);
  checkRays(frame);
  checkRays(hyperboloid);
  checkRays(two);
  checkRays(paraboloid);
}
