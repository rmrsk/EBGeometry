// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of the parts of a shape (TestSurfaceParts.cpp): a tape's part labels on a device against the host.
// They are apart from TestSurfaceParts.cpp so that a GPU build compiles only them with the device compiler.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestUtilities.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>

using namespace EBGeometry;

namespace {

// Device functor: the surface's label and part at a point.
template <class T, class TapeType>
struct PartLabelQuery
{
  TapeType m_tape;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    const SurfaceRef<T> s = m_tape.closestSurface(a_point);

    return 1000U * s.m_surfaceLabel.value() + s.m_part;
  }
};

// Device functor: whether the surface at a point is on a face of a mask, and the selection entry it gives.
template <class T, class TapeType>
struct FaceMaskQuery
{
  TapeType       m_tape;
  RegionSettings m_regions;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    const SurfaceRef<T> s = m_tape.closestSurface(a_point);

    return 10U * (m_regions.m_faceMask.contains(s) ? 1U : 0U) + (m_regions.selectionOf(s) == 0U ? 1U : 0U);
  }
};

} // namespace

TEMPLATE_TEST_CASE("Tape: part labels on a device match the host", "[Tape][gpu]", EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using V    = Vec3T<T>;
  using Mesh = TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const auto soup = Parser::readIntoPolygonSoup<T>(dataPath("cube_groups.obj"));
  const Mesh cube = Parser::readIntoTriMeshSDF<T>(dataPath("cube_groups.obj"), pool);

  const Tape<T> tape(
    pool,
    Difference(SurfaceLabel(cube, soup.getFaceGroups()), CylinderSDF<T>(V(0.5, 0.5, -1), V(0.5, 0.5, 2), T(0.2))));

  pool.freeze();

  const Pool device = Pool::mirror(pool, deviceTestResource());

  std::vector<V> points;

  for (int i = 0; i < 40; i++) {
    for (int j = 0; j < 10; j++) {
      points.emplace_back(T(-0.3) + T(0.04) * T(i), T(0.5), T(-0.3) + T(0.16) * T(j));
    }
  }

  const auto onDevice = evaluateOnDevice<uint32_t>(PartLabelQuery<T, Tape<T>>{tape.rebasedView(device)}, points);
  const auto onHost   = evaluateOnHost<uint32_t>(PartLabelQuery<T, Tape<T>>{tape}, points);

  size_t differ = 0;

  for (size_t i = 0; i < points.size(); i++) {
    differ += (onDevice[i] != onHost[i]) ? 1U : 0U;
  }

  // Where two faces are as close, the device's rounding may pick the other.
  CHECK(differ <= 8U);
}

TEMPLATE_TEST_CASE("FaceMask: faces picked one by one are selected on a device as on the host",
                   "[FaceMask][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using V    = Vec3T<T>;
  using Mesh = TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>;

  using namespace EBGeometryTestGPU;

  Pool pool(hostMemoryResource());

  // cube_groups.obj: faces 2 (y = 0) and 5 (x = 0) picked; the tape's one leaf is surface 0.
  const Mesh    cube = Parser::readIntoTriMeshSDF<T>(dataPath("cube_groups.obj"), pool);
  const Tape<T> tape(pool, toExpr(cube));

  Pool masks(hostMemoryResource());

  RegionSettings regions;

  regions.m_faceMask = FaceMask(masks, 0U, 6U, {2U, 5U, 5U});

  CHECK(regions.m_faceMask.size() == 2U);
  CHECK(regions.hasSelection());

  pool.freeze();
  masks.freeze();

  // Points just outside each face's middle: the picked ones are in the mask, and selected as entry 0.
  const std::vector<V> points{
    V(0.5, 0.5, -0.2), V(0.5, 0.5, 1.2), V(0.5, -0.2, 0.5), V(1.2, 0.5, 0.5), V(0.5, 1.2, 0.5), V(-0.2, 0.5, 0.5)};

  const std::vector<uint32_t> expected{0U, 0U, 11U, 0U, 0U, 11U};

  const auto onHost = evaluateOnHost<uint32_t>(FaceMaskQuery<T, Tape<T>>{tape, regions}, points);

  CHECK(onHost == expected);

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const Pool device     = Pool::mirror(pool, deviceTestResource());
  const Pool deviceMask = Pool::mirror(masks, deviceTestResource());

  RegionSettings deviceRegions = regions;

  deviceRegions.m_faceMask = regions.m_faceMask.rebasedView(deviceMask);

  const auto onDevice =
    evaluateOnDevice<uint32_t>(FaceMaskQuery<T, Tape<T>>{tape.rebasedView(device), deviceRegions}, points);

  CHECK(onDevice == expected);
}
