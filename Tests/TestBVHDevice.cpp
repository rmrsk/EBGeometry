// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The device tests of the BVHs and the mesh SDFs (TestBVH.cpp): a rebased PackedBVH traversed in a kernel, with
// points, rays and a visit callback; the mesh SDFs' distances, closest points, gradients, closest faces and boxes;
// and a refit a level at a time on the device; each against the host. They are apart from TestBVH.cpp so that a GPU
// build compiles only them with the device compiler.

#include "EBGeometry.hpp"
#include "TestBVHHelpers.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestUtilities.hpp"

#include <cstdint>
#include <type_traits>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometryTestBVH;

// The closest point, gradient and signed distance of a mesh SDF, one component per query: for the device test
// below. The face id is left out: where the closest point is an edge or a vertex, every face meeting there is
// correct, and the device and the host, rounding the distances differently, may name different ones.
template <class S>
struct ClosestPointQuery
{
  S   m_sdf;
  int m_component;

  template <class T>
  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    const ClosestPoint<T> cp = m_sdf.closestPoint(a_point);
    const Vec3T<T>        g  = m_sdf.gradient(a_point);

    if (m_component < 3) {
      return cp.point[static_cast<size_t>(m_component)];
    }

    return m_component < 6 ? g[static_cast<size_t>(m_component - 3)] : cp.signedDistance;
  }
};

TEMPLATE_TEST_CASE("Mesh SDFs: closestPoint and gradient run on device and match the host",
                   "[BVH][MeshSDF][TriMeshSDF][FlatMeshSDF][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

  const FlatMeshSDF<T>      flat(mesh, pool);
  const MeshSDF<T, 4>       meshSDF(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, 4, 4> triSDF(mesh, pool, BVH::Construction::SAH, 2);

  pool.freeze();

  Pool devicePool = Pool::mirror(pool, deviceTestResource());

  const auto points = queryGrid<T>(Vec3(T(-3), T(-3), T(-3)), Vec3(T(3), T(3), T(3)), 6);

  const auto compare = [&](const auto& a_host, const auto& a_device) {
    using S = std::decay_t<decltype(a_host)>;

    for (int component = 0; component < 7; component++) {
      INFO("component " << component);

      requireSameResults(evaluateOnDevice<T>(ClosestPointQuery<S>{a_device, component}, points),
                         evaluateOnHost<T>(ClosestPointQuery<S>{a_host, component}, points));
    }
  };

  compare(flat, flat.rebasedView(devicePool));
  compare(meshSDF, meshSDF.rebasedView(devicePool));
  compare(triSDF, triSDF.rebasedView(devicePool));
}

// One traversal per query point: the probe the host suite also runs, so the device executes exactly
// the code the host checks, with the BVH held by value as a kernel receives it.
template <class T, size_t K>
struct PackedBvhTraversalQuery
{
  EBGeometry::BVH::PackedBVH<T, BareTestPoint<T>, K> m_bvh;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return packedBvhTraversalProbe<T, K>(m_bvh, a_point);
  }
};

TEMPLATE_TEST_CASE("PackedBVH: a rebased view traverses on device and matches the host",
                   "[BVH][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  Pool pool(hostMemoryResource());

  std::vector<Vec3> positions;

  positions.reserve(64);

  for (int i = 0; i < 64; i++) {
    const T t = T(i);

    positions.emplace_back(std::sin(t) * t, std::cos(t) * t, T(0.2) * t);
  }

  std::vector<std::pair<Pnt, AABB>> flat;

  flat.reserve(positions.size());

  for (const auto& pos : positions) {
    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  const Packed bvh = Packed::buildBottomUp(pool, flat, size_t(4));

  pool.freeze();

  Pool         devicePool = Pool::mirror(pool, deviceTestResource());
  const Packed deviceView = bvh.rebasedView(devicePool);

  // The points spiral out to a radius of 63 in the xy-plane and rise to z = 12.6, so this grid
  // covers query points among them as well as well outside the cloud.
  const auto points = queryGrid<T>(Vec3(T(-70), T(-70), T(-5)), Vec3(T(70), T(70), T(18)), 10);

  requireSameResults(evaluateOnDevice<T>(PackedBvhTraversalQuery<T, K>{deviceView}, points),
                     evaluateOnHost<T>(PackedBvhTraversalQuery<T, K>{bvh}, points));
}

// One custom traversal per query point, through the template PackedBVH::traverse().
template <class T, size_t K>
struct CustomTraversalQuery
{
  EBGeometry::BVH::PackedBVH<T, BareTestPoint<T>, K> m_bvh;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return customTraversalProbe<T, K>(m_bvh, a_point);
  }
};

TEMPLATE_TEST_CASE("PackedBVH: the template traverse() runs on device and matches the host",
                   "[BVH][traverse][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;
  using Pnt  = BareTestPoint<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, Pnt, K>;

  Pool pool(hostMemoryResource());

  std::vector<std::pair<Pnt, AABB>> flat;

  for (int i = 0; i < 64; i++) {
    const T    t = T(i);
    const Vec3 pos(std::sin(t) * t, std::cos(t) * t, T(0.2) * t);

    flat.emplace_back(Pnt{pos}, AABB(pos, pos));
  }

  const Packed bvh = Packed::buildTopDown(pool, flat, BVH::BinnedSAHPartitioner<T, Pnt, AABB, K>);

  pool.freeze();

  Pool         devicePool = Pool::mirror(pool, deviceTestResource());
  const Packed deviceView = bvh.rebasedView(devicePool);

  const auto points = queryGrid<T>(Vec3(T(-70), T(-70), T(-5)), Vec3(T(70), T(70), T(18)), 10);

  requireSameResults(evaluateOnDevice<T>(CustomTraversalQuery<T, K>{deviceView}, points),
                     evaluateOnHost<T>(CustomTraversalQuery<T, K>{bvh}, points));
}

// Device functor: the nearest box entered along a ray from each query point, in the direction (1, 2, 3).
template <class T, size_t K>
struct NearestBoxEnteredQuery
{
  EBGeometry::BVH::PackedBVH<T, BoundingVolumes::AABBT<T>, K> m_bvh;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_origin) const noexcept
  {
    const Vec3T<T> d(T(1), T(2), T(3));

    return nearestBoxEntered<T, K>(m_bvh, EBGeometry::BVH::RayQuery<T>(a_origin, d / d.length()));
  }
};

// Where the ray of an edge case enters its box, through RayQuery::key().
template <class T>
struct EdgeRayEntry
{
  EdgeRay<T> m_cases[4];

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const uint32_t& a_index) const noexcept
  {
    const EdgeRay<T>& c = m_cases[a_index];

    const BoundingVolumes::AABBT<T> box(Vec3T<T>(c.m_lo[0], c.m_lo[1], c.m_lo[2]),
                                        Vec3T<T>(c.m_hi[0], c.m_hi[1], c.m_hi[2]));
    const BVH::RayQuery<T>          ray(Vec3T<T>(c.m_origin[0], c.m_origin[1], c.m_origin[2]),
                               Vec3T<T>(c.m_direction[0], c.m_direction[1], c.m_direction[2]));

    return ray.key(box);
  }
};

TEMPLATE_TEST_CASE("BVH::RayQuery: a ray that meets a box only along an edge enters it on a device too",
                   "[BVH][traverse][RayQuery][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  const std::vector<EdgeRay<T>> cases = edgeRays<T>();

  REQUIRE(cases.size() == 4);

  EdgeRayEntry<T> entry{};

  for (size_t i = 0; i < cases.size(); i++) {
    entry.m_cases[i] = cases[i];
  }

  const std::vector<uint32_t> indices = {0U, 1U, 2U, 3U};
  const std::vector<T>        device  = evaluateOnDevice<T>(entry, indices);

  for (const T t : device) {
    REQUIRE(t < std::numeric_limits<T>::infinity());
  }

  // The entry is not widened, so it is the host's bit for bit, fused widening or not.
  requireSameResults(device, evaluateOnHost<T>(entry, indices), T(0));
}

TEMPLATE_TEST_CASE("PackedBVH::traverse with a RayQuery runs on device and matches the host",
                   "[BVH][traverse][RayQuery][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using AABB = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t K = 4;

  using Packed = BVH::PackedBVH<T, AABB, K>;

  Pool pool(hostMemoryResource());

  const Packed bvh = Packed::buildBottomUp(pool, rayTestBoxes<T>(500), size_t(4));

  pool.freeze();

  Pool         devicePool = Pool::mirror(pool, deviceTestResource());
  const Packed deviceView = bvh.rebasedView(devicePool);

  const auto points = queryGrid<T>(Vec3(T(-2), T(-2), T(-2)), Vec3(T(1), T(1), T(1)), 10);

  const auto deviceHits = evaluateOnDevice<T>(NearestBoxEnteredQuery<T, K>{deviceView}, points);
  const auto hostHits   = evaluateOnHost<T>(NearestBoxEnteredQuery<T, K>{bvh}, points);

  REQUIRE(deviceHits.size() == hostHits.size());

  for (size_t i = 0; i < hostHits.size(); i++) {
    INFO("query " << i);

    // Both infinite (a miss), or the same entry distance.
    if (hostHits[i] == std::numeric_limits<T>::infinity()) {
      REQUIRE(deviceHits[i] == hostHits[i]);
    }
    else {
      REQUIRE_THAT(deviceHits[i], withinAbsT(hostHits[i], double(gpuTol<T>() * (T(1) + std::abs(hostHits[i])))));
    }
  }
}

// One signed distance per query point. Any of FlatMeshSDF, MeshSDF, TriMeshSDF, held by value as a
// kernel receives it.
template <class T, class SDF>
struct SignedDistanceQuery
{
  SDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_sdf.signedDistance(a_point);
  }
};

// The id of the face closest to each query point, from getClosestFace(), which every mesh SDF
// provides on the device.
template <class T, class SDF>
struct ClosestFaceQuery
{
  SDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  int
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return static_cast<int>(m_sdf.getClosestFace(a_point).faceId);
  }
};

// The device-callable bounding box every mesh SDF provides: query i < 3 returns the upper corner's
// component i, and query i >= 3 the lower corner's component i - 3.
template <class T, class SDF>
struct BoundingBoxQuery
{
  SDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const int& a_i) const noexcept
  {
    const auto box = m_sdf.computeBoundingVolume();

    return (a_i < 3) ? box.getHighCorner()[static_cast<size_t>(a_i)] : box.getLowCorner()[static_cast<size_t>(a_i - 3)];
  }
};

TEMPLATE_TEST_CASE("FlatMeshSDF/MeshSDF/TriMeshSDF: rebased copies evaluate and find the closest face on device, "
                   "matching the host",
                   "[BVH][FlatMeshSDF][MeshSDF][TriMeshSDF][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t K = 4;
  constexpr size_t W = 4;

  Pool       pool(hostMemoryResource());
  const auto mesh = Parser::readIntoDCEL<T>(dataPath("dodecahedron.obj"), pool);

  const FlatMeshSDF<T>      flat(mesh, pool);
  const MeshSDF<T, K>       meshSDF(mesh, pool, BVH::Construction::SAH);
  const TriMeshSDF<T, K, W> triSDF(mesh, pool, BVH::Construction::SAH, 2);

  pool.freeze();

  Pool devicePool = Pool::mirror(pool, deviceTestResource());

  // The dodecahedron is centered at the origin with vertices within 1.62 of it on each axis, so this
  // grid covers points inside, outside, and near the surface.
  const auto points  = queryGrid<T>(Vec3T<T>(T(-2), T(-2), T(-2)), Vec3T<T>(T(2), T(2), T(2)), 12);
  const auto corners = std::vector<int>{0, 1, 2, 3, 4, 5};

  const auto check = [&](const auto& a_sdf) {
    using SDF = std::decay_t<decltype(a_sdf)>;

    const SDF view = a_sdf.rebasedView(devicePool);

    requireSameResults(evaluateOnDevice<T>(SignedDistanceQuery<T, SDF>{view}, points),
                       evaluateOnHost<T>(SignedDistanceQuery<T, SDF>{a_sdf}, points));
    requireSameResults(evaluateOnDevice<T>(BoundingBoxQuery<T, SDF>{view}, corners),
                       evaluateOnHost<T>(BoundingBoxQuery<T, SDF>{a_sdf}, corners));

    const auto deviceFaces = evaluateOnDevice<int>(ClosestFaceQuery<T, SDF>{view}, points);
    const auto hostFaces   = evaluateOnHost<int>(ClosestFaceQuery<T, SDF>{a_sdf}, points);

    REQUIRE(deviceFaces.size() == hostFaces.size());

    // Several faces tie at a point nearest a shared edge or vertex, and the device's fused
    // multiply-adds may break the tie the other way, so a different face is accepted when it is as
    // close as the host's.
    for (size_t i = 0; i < points.size(); i++) {
      if (deviceFaces[i] != hostFaces[i]) {
        INFO("query " << i << ": device face " << deviceFaces[i] << ", host face " << hostFaces[i]);

        REQUIRE(deviceFaces[i] >= 0);
        REQUIRE(static_cast<uint32_t>(deviceFaces[i]) < mesh.numFaces());

        const T own  = mesh.getFace(static_cast<uint32_t>(deviceFaces[i])).signedDistance(points[i], mesh);
        const T host = mesh.getFace(static_cast<uint32_t>(hostFaces[i])).signedDistance(points[i], mesh);

        REQUIRE_THAT(std::abs(own), withinAbsT(std::abs(host), traversalMargin<T>()));
      }
    }
  };

  check(flat);
  check(meshSDF);
  check(triSDF);
}

TEMPLATE_TEST_CASE("BVH::refit: refits a mirrored BVH a level at a time on the device, after the caller's own "
                   "kernel moved the primitives, to the host refit's boxes",
                   "[BVH][refit][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  constexpr size_t   K = 4;
  constexpr uint32_t N = 5000;

  Pool pool(hostMemoryResource());

  PODVector<Vec3T<T>> points;

  const auto bvh = pointIdBVH<T, K>(pool, points, N);

  const BVH::RefitSchedule schedule(pool, bvh);

  pool.freeze();

  Pool devicePool = Pool::mirror(pool, deviceTestResource());

  // The caller's kernel moves the points in device memory; BVH::refit() then reads them there.
  GPU::parallelFor(N, MovePoints<T>{points.data(devicePool.base())});

  BVH::refit(devicePool, bvh, schedule, PointBoxById<T>{points.data(devicePool.base())});

  // The same move and refit on the host give the reference: PackedBVH::refit()'s sweep.
  for (uint32_t i = 0; i < N; i++) {
    MovePoints<T>{points.data(pool.base())}(i);
  }

  auto hostBVH = bvh;

  hostBVH.refit(PointBoxById<T>{points.data(pool.base())});

  const Pool back     = Pool::mirror(devicePool, hostMemoryResource());
  const auto backView = bvh.rebasedView(back);
  const auto nodes    = bvh.getNodes();

  for (uint32_t i = 0; i < nodes.size(); i++) {
    const auto got  = backView.getBoundingVolume(backView.getNodes()[i]);
    const auto want = hostBVH.getBoundingVolume(nodes[i]);

    for (int d = 0; d < 3; d++) {
      REQUIRE(got.getLowCorner()[d] == want.getLowCorner()[d]);
      REQUIRE(got.getHighCorner()[d] == want.getHighCorner()[d]);
    }
  }

  const Vec3T<T>* moved = points.data(back.base());

  requireTightAndQueryable<T, K>(backView, std::vector<Vec3T<T>>(moved, moved + N));
}
