// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of SphereCloudBVH (TestSphereCloudBVH.cpp): the queries, contacts and rays in a kernel against the
// host, the build in device memory, and a rebuild through GPU::parallelFor(), which runs as kernels in a GPU build.
// They are apart from TestSphereCloudBVH.cpp so that a GPU build compiles only them with the device compiler.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestSphereCloudBVHHelpers.hpp"

#include <cstdint>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestSphereCloud;

TEMPLATE_TEST_CASE("SphereCloudBVH: forEachContact on device matches the host",
                   "[SphereCloudBVH][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr uint32_t n = 900;

  Pool pool(hostMemoryResource());

  const SphereCloudBVH<T> cloud(pool, makeSpheres<T>(n, 81u), 8);

  pool.freeze();

  Pool                    devicePool = Pool::mirror(pool, deviceTestResource());
  const SphereCloudBVH<T> deviceView = cloud.rebasedView(devicePool);

  std::vector<uint32_t> indices(n);

  for (uint32_t i = 0; i < n; i++) {
    indices[i] = i;
  }

  const T gap = T(0.01);

  // Which spheres were found must match exactly, the separations to the device's rounding.
  REQUIRE(evaluateOnDevice<std::uint64_t>(ContactFingerprint<T>{deviceView, gap}, indices) ==
          evaluateOnHost<std::uint64_t>(ContactFingerprint<T>{cloud, gap}, indices));

  requireSameResults(evaluateOnDevice<T>(ContactSeparationSum<T>{deviceView, gap}, indices),
                     evaluateOnHost<T>(ContactSeparationSum<T>{cloud, gap}, indices));
}

TEMPLATE_TEST_CASE("SphereCloudBVH: a rebuild from moved spheres is the tree a new build gives, and reserves nothing",
                   "[SphereCloudBVH]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(1000, 21u);

  for (MemoryResource* resource : std::vector<MemoryResource*>{&hostMemoryResource(), &parallelBuildResource()}) {
    Pool pool(*resource);

    SphereCloudBVH<T> cloud = SphereCloudBVH<T>::build(pool, spheres, 16, 1200);

    REQUIRE(cloud.maxSpheres() == 1200);

    pool.freeze();

    const auto used = pool.usedBytes();

    // Moved, grown and fewer: a different tree in the same arrays.
    std::vector<SphereCloud::Sphere<T>> moved(spheres.begin(), spheres.begin() + 900);

    for (auto& sphere : moved) {
      sphere.m_center = Vec3T<T>(T(2) * sphere.m_center[1], sphere.m_center[0], T(0.5) * sphere.m_center[2]);
      sphere.m_radius = T(1.5) * sphere.m_radius;
    }

    // A build through GPU::parallelFor() reads its source where the build runs, so the spheres go to the pool's
    // memory there.
    Pool movedPool(*resource);

    PODVector<SphereCloud::Sphere<T>> stored;

    stored.reserveSizedFrom(movedPool, uint32_t(moved.size()));
    movedPool.resource().copy(stored.data(movedPool.base()),
                              movedPool.resource(),
                              moved.data(),
                              hostMemoryResource(),
                              sizeof(SphereCloud::Sphere<T>) * moved.size());

    cloud.rebuild(SphereCloud::FromArray<T>{stored.data(movedPool.base())}, uint32_t(moved.size()));

    Pool                    freshPool(hostMemoryResource());
    const SphereCloudBVH<T> fresh(freshPool, moved, 16);

    requireSameTree(cloud, fresh);
    CHECK(pool.usedBytes() == used);
  }
}

TEMPLATE_TEST_CASE("SphereCloudBVH: a rebased view answers on device and matches the host",
                   "[SphereCloudBVH][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const SphereCloudBVH<T> cloud(pool, makeSpheres<T>(800, 12u), 8);

  pool.freeze();

  Pool                    devicePool = Pool::mirror(pool, deviceTestResource());
  const SphereCloudBVH<T> deviceView = cloud.rebasedView(devicePool);

  const std::vector<Vec3T<T>> points =
    queryGrid<T>(Vec3T<T>(T(-0.25), T(-0.25), T(-0.25)), Vec3T<T>(T(1.25), T(1.25), T(1.25)), 10);

  // The index must match exactly, the distance to the device's rounding.
  const std::vector<std::size_t> deviceIndices =
    evaluateOnDevice<std::size_t>(ClosestSphereIndex<T>{deviceView}, points);
  const std::vector<std::size_t> hostIndices = evaluateOnHost<std::size_t>(ClosestSphereIndex<T>{cloud}, points);

  REQUIRE(deviceIndices == hostIndices);

  requireSameResults(evaluateOnDevice<T>(SignedDistance<T>{deviceView}, points),
                     evaluateOnHost<T>(SignedDistance<T>{cloud}, points));
}

TEMPLATE_TEST_CASE("SphereCloudBVH::build in device memory gives the host's tree and answers on device",
                   "[SphereCloudBVH][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(3000, 31u);

  std::vector<Vec3T<T>> centers;
  std::vector<T>        radii;

  for (const auto& sphere : spheres) {
    centers.push_back(sphere.m_center);
    radii.push_back(sphere.m_radius);
  }

  // Device memory in a GPU build, which the host cannot read, so the spheres are copied there first and the kernels
  // write the arrays in place; emulated device memory in a host build, where the steps run as host loops.
  Pool hostPool(hostMemoryResource());
  Pool pool(deviceTestResource());

  const SphereCloudBVH<T> reference(hostPool, spheres, 8);
  const SphereCloudBVH<T> cloud = SphereCloudBVH<T>::build(pool, centers, radii, 8);

  pool.freeze();

  const SphereCloudBVH<T> deviceView = cloud.rebasedView(pool);

  const std::vector<Vec3T<T>> points =
    queryGrid<T>(Vec3T<T>(T(-0.25), T(-0.25), T(-0.25)), Vec3T<T>(T(1.25), T(1.25), T(1.25)), 10);

  REQUIRE(evaluateOnDevice<std::size_t>(ClosestSphereIndex<T>{deviceView}, points) ==
          evaluateOnHost<std::size_t>(ClosestSphereIndex<T>{reference}, points));

  // Mirrored back to the host, the tree built in device memory is the host's tree.
  const Pool              onHost   = Pool::mirror(pool, hostMemoryResource());
  const SphereCloudBVH<T> hostView = cloud.rebasedView(onHost);

  requireSameTree(hostView, reference);
}

TEMPLATE_TEST_CASE("SphereCloudBVH: rays on device match the host",
                   "[SphereCloudBVH][Ray][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T     = TestType;
  using Cloud = SphereCloudBVH<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool pool(hostMemoryResource());

  const std::vector<SphereCloud::Sphere<T>> spheres = makeSpheres<T>(800, 44u);
  const Cloud                               cloud(pool, spheres);

  pool.freeze();

  const Pool devicePool = Pool::mirror(pool, deviceTestResource());

  const Cloud deviceCloud = cloud.rebasedView(devicePool);

  const std::vector<RayT<T>> rays = makeRays(spheres, 256, 45u);

  REQUIRE(evaluateOnDevice<std::size_t>(RaySphere<T>{deviceCloud}, rays) ==
          evaluateOnHost<std::size_t>(RaySphere<T>{cloud}, rays));

  requireSameResults(evaluateOnDevice<T>(RayDistance<T, Cloud>{deviceCloud}, rays),
                     evaluateOnHost<T>(RayDistance<T, Cloud>{cloud}, rays));
}
