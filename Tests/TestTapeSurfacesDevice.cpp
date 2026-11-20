// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of the surfaces of a shape (TestTapeSurfaces.cpp): a tape's closestSurface() on a device against
// the host. They are apart from TestTapeSurfaces.cpp so that a GPU build compiles only them with the device compiler.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestTapeSurfacesHelpers.hpp"

#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestSurfaces;

namespace {

// Device functor: the surface that decides a tape's value at a point.
template <class T, class TapeType>
struct SurfaceQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_tape.closestSurface(a_point).m_surface;
  }
};

} // namespace

TEMPLATE_TEST_CASE("Tape: closestSurface on a device matches the host", "[Tape][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;
  using Ball = SphereCloudBVH<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const auto spheres = randomSpheres<T>(60, 5U);

  Pool pool(hostMemoryResource());

  const auto cloud = Ball::build(pool, spheres, 4);

  const Tape<T, Ball> tape(pool,
                           Difference(BoxSDF<T>(Vec3(T(-1), T(-1), T(-1)), Vec3(T(9), T(9), T(9))), toExpr(cloud)));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  const auto points = queryPoints<T>(400);

  const auto onDevice = evaluateOnDevice<uint32_t>(SurfaceQuery<T, Tape<T, Ball>>{tape.rebasedView(device)}, points);
  const auto onHost   = evaluateOnHost<uint32_t>(SurfaceQuery<T, Tape<T, Ball>>{tape}, points);

  size_t differ = 0;

  for (size_t i = 0; i < points.size(); i++) {
    differ += (onDevice[i] != onHost[i]) ? 1U : 0U;
  }

  // Where two surfaces are as close, the device's rounding may pick the other.
  CHECK(differ <= 4U);
}
