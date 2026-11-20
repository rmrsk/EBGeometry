// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of the CSG combinators (TestCSG.cpp): the BVH unions' signed distances on a device against the
// host. They are apart from TestCSG.cpp so that a GPU build compiles only them with the device compiler.

#include "EBGeometry.hpp"
#include "TestCSGHelpers.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"

#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestCSG;

namespace {

// One signed distance per query point, for any of the BVH unions, held by value as a kernel receives it.
template <class T, class Union>
struct UnionDistanceQuery
{
  Union m_union;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_union.signedDistance(a_point);
  }
};

} // namespace

TEMPLATE_TEST_CASE("BVH unions: device signedDistance matches the host", "[CSG][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const auto meshes = dodecahedronGrid<T>(pool);

  const BVHUnion<T, 4, SphereSDF<T>>                       sphereUnion(pool, sphereRow<T>(), sphereRowBVs<T>());
  const BVHSmoothUnion<T, 4, SmoothMinOp<T>, SphereSDF<T>> smoothUnion(pool, sphereRow<T>(), sphereRowBVs<T>(), T(0.6));
  const BVHUnion<T, 4, TestTriMesh<T>>                     meshUnion(pool, meshes, boundingVolumes(meshes));

  // Spheres and infinite cylinders: the unbounded leaf is scanned on the device too.
  const auto                                                      mixed = spheresAndCylinders<T>();
  const BVHUnion<T, 4, SphereOrCylinder<T>>                       mixedUnion(pool, mixed, boxesOf<T>(mixed));
  const BVHSmoothUnion<T, 4, SmoothMinOp<T>, SphereOrCylinder<T>> mixedSmooth(pool, mixed, boxesOf<T>(mixed), T(0.5));

  const auto manyTypes       = BVHUnion(pool, meshes, sphereRow<T>(), cylinderRow<T>());
  const auto manyTypesSmooth = BVHSmoothUnion(pool, T(0.6), sphereRow<T>(), boxRow<T>(), cylinderRow<T>());

  pool.freeze();

  Pool devicePool = Pool::mirror(pool, deviceTestResource());

  // The unit spheres sit at x = 0, 3, ..., 33 on the x-axis, and the dodecahedra (vertices within 1.62
  // of their centers) at x = 0, 4, 8 and y = 0, 4. Each grid covers points inside, outside, near the
  // surfaces and between the primitives of its union.
  const T    rowEnd       = T(3) * T(NumRowSpheres - 1) + T(2);
  const auto spherePoints = queryGrid<T>(Vec3T<T>(T(-2), T(-2), T(-2)), Vec3T<T>(rowEnd, T(2), T(2)), 16);
  const auto meshPoints   = queryGrid<T>(Vec3T<T>(T(-2), T(-2), T(-2)), Vec3T<T>(T(10), T(6), T(2)), 16);

  const auto check = [&](const auto& a_union, const std::vector<Vec3T<T>>& a_points) {
    using Union = std::decay_t<decltype(a_union)>;

    const Union view = a_union.rebasedView(devicePool);

    requireSameResults(evaluateOnDevice<T>(UnionDistanceQuery<T, Union>{view}, a_points),
                       evaluateOnHost<T>(UnionDistanceQuery<T, Union>{a_union}, a_points));
  };

  check(sphereUnion, spherePoints);
  check(smoothUnion, spherePoints);
  check(meshUnion, meshPoints);

  const auto mixedPoints = queryGrid<T>(Vec3T<T>(T(-8), T(-8), T(-8)), Vec3T<T>(T(8), T(8), T(8)), 16);

  check(mixedUnion, mixedPoints);
  check(mixedSmooth, mixedPoints);

  // Several types in one union: meshes, spheres and unbounded cylinders, sharp and smooth.
  check(manyTypes, meshPoints);
  check(manyTypesSmooth, spherePoints);
}
