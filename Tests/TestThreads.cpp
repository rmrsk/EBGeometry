// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// Unit tests for using EBGeometry from several host threads: queries on shared objects (a tape holding a
// mesh, a mesh SDF, the point clouds, a tape read from the .ebg language) and builds into one pool per
// thread from a shared expression and polygon soup. Each thread's answers must equal the single-threaded
// ones exactly. Run under ThreadSanitizer, these also check that the queries read no shared state a
// thread writes.

#include "EBGeometry.hpp"
#include "Source/EBGeometry_Language.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestFusedHelpers.hpp"
#include "TestPointCloudQueries.hpp"
#include "TestUtilities.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <string>
#include <thread>
#include <vector>

using namespace EBGeometry;

namespace {

constexpr int NumThreads = 8;

// Runs a_work(thread) on NumThreads threads at once and waits for all of them.
inline void
inThreads(const std::function<void(int)>& a_work)
{
  std::vector<std::thread> threads;

  for (int i = 0; i < NumThreads; i++) {
    threads.emplace_back(a_work, i);
  }

  for (auto& thread : threads) {
    thread.join();
  }
}

// Every thread computes a_query at every point, each starting at another point so that the threads
// interleave; each thread's results must equal a_reference.
template <class R, class Query>
void
checkInThreads(const std::vector<R>& a_reference, const std::size_t a_count, const Query& a_query)
{
  std::vector<std::vector<R>> results(NumThreads, std::vector<R>(a_count));

  inThreads([&](const int a_thread) {
    for (std::size_t n = 0; n < a_count; n++) {
      const std::size_t i = (n + std::size_t(a_thread) * 97U) % a_count;

      results[std::size_t(a_thread)][i] = a_query(i);
    }
  });

  for (int t = 0; t < NumThreads; t++) {
    CHECK(results[std::size_t(t)] == a_reference);
  }
}

} // namespace

TEMPLATE_TEST_CASE("Threads: queries on a shared tape holding a mesh equal the single-threaded ones",
                   "[Threads]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  Pool pool(hostMemoryResource());

  const auto                mesh = Parser::readIntoDCEL<T>(dodecahedron(), pool);
  const TriMeshSDF<T, K, W> triMesh(mesh, pool, BVH::Construction::SAH, 4);
  const SphereSDF<T>        sphere(Vec3(T(0.8), T(0.2), T(-0.1)), T(0.5));
  const BoxSDF<T>           box(Vec3(T(-0.3), T(-0.3), T(-0.3)), Vec3(T(0.3), T(0.3), T(0.3)));
  const Tape<T>             tape(pool, Difference(Union(triMesh, sphere), box));

  // Nothing is built into the pool while the threads read it.
  pool.freeze();

  const auto        points = EBGeometryTestFused::probes<T>();
  const std::size_t count  = points.size();

  const auto ray = [&points](const std::size_t a_i) {
    return RayT<T>{points[a_i] + Vec3(T(0), T(0), T(4)), Vec3(T(0), T(0), -T(1))};
  };

  std::vector<T>        distances(count);
  std::vector<uint32_t> leaves(count);
  std::vector<T>        hits(count);
  std::vector<T>        meshDistances(count);
  std::vector<T>        meshHits(count);

  for (std::size_t i = 0; i < count; i++) {
    distances[i]     = tape.signedDistance(points[i]);
    leaves[i]        = tape.closestLeaf(points[i]).m_leaf;
    hits[i]          = tape.firstHit(ray(i), T(0), T(8)).m_t;
    meshDistances[i] = triMesh.signedDistance(points[i]);
    meshHits[i]      = firstHit(triMesh, ray(i), T(0), T(8)).m_t;
  }

  checkInThreads(distances, count, [&](const std::size_t a_i) { return tape.signedDistance(points[a_i]); });
  checkInThreads(leaves, count, [&](const std::size_t a_i) { return tape.closestLeaf(points[a_i]).m_leaf; });
  checkInThreads(hits, count, [&](const std::size_t a_i) { return tape.firstHit(ray(a_i), T(0), T(8)).m_t; });
  checkInThreads(meshDistances, count, [&](const std::size_t a_i) { return triMesh.signedDistance(points[a_i]); });
  checkInThreads(meshHits, count, [&](const std::size_t a_i) { return firstHit(triMesh, ray(a_i), T(0), T(8)).m_t; });
}

TEMPLATE_TEST_CASE("Threads: queries on shared point clouds equal the single-threaded ones",
                   "[Threads]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const std::vector<Vec3T<T>> cloud   = EBGeometryTestPointCloud::makeCloud<T>(2000, 20261122U);
  const std::vector<Vec3T<T>> queries = EBGeometryTestPointCloud::makeCloud<T>(500, 7U);
  const PointCloudBVH<T>      bvh(pool, cloud);
  const PointCloudHashGrid<T> grid(pool, cloud);

  pool.freeze();

  const std::size_t     count = queries.size();
  std::vector<uint32_t> bvhNearest(count);
  std::vector<uint32_t> gridNearest(count);

  for (std::size_t i = 0; i < count; i++) {
    bvhNearest[i]  = bvh.closestPoint(queries[i]).index;
    gridNearest[i] = grid.closestPoint(queries[i]).index;
  }

  checkInThreads(bvhNearest, count, [&](const std::size_t a_i) { return bvh.closestPoint(queries[a_i]).index; });
  checkInThreads(gridNearest, count, [&](const std::size_t a_i) { return grid.closestPoint(queries[a_i]).index; });
}

TEMPLATE_TEST_CASE("Threads: queries on a shared tape read from the .ebg language equal the single-threaded ones",
                   "[Threads]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  const std::string text = R"(ebg 1
a = sphere((0, 0, 0), 0.6)
b = box((-0.2, -0.2, -0.9), (0.2, 0.2, 0.9))
object smooth_union(a, b, smooth = 0.1)
)";

  const auto    model = Language::parse<T>(text, "test.ebg", pool);
  const Tape<T> tape(pool, model.object());

  pool.freeze();

  const auto        points = EBGeometryTestFused::probes<T>();
  const std::size_t count  = points.size();
  std::vector<T>    distances(count);

  for (std::size_t i = 0; i < count; i++) {
    distances[i] = tape.signedDistance(points[i]);
  }

  checkInThreads(distances, count, [&](const std::size_t a_i) { return tape.signedDistance(points[a_i]); });
}

TEMPLATE_TEST_CASE("Threads: tapes and mesh SDFs built into one pool per thread from a shared expression and soup "
                   "equal the single-threaded build",
                   "[Threads]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T    = TestType;
  using Vec3 = Vec3T<T>;

  constexpr size_t K = BVH::DefaultBranchingRatio<T>();
  constexpr size_t W = TriangleSoA::DefaultWidth<T>();

  // Shared, read-only sources: a polygon soup and an expression of analytic shapes.
  const PolygonSoup<T> soup = Parser::readIntoPolygonSoup<T>(dodecahedron());
  const SphereSDF<T>   sphere(Vec3(T(0.8), T(0.2), T(-0.1)), T(0.5));
  const BoxSDF<T>      box(Vec3(T(-0.3), T(-0.3), T(-0.3)), Vec3(T(0.3), T(0.3), T(0.3)));
  const Expr<T>        expression = SmoothUnion(sphere, box, T(0.1));

  const auto        points = EBGeometryTestFused::probes<T>();
  const std::size_t count  = points.size();

  const auto build = [&](std::vector<T>& a_tape, std::vector<T>& a_mesh) {
    Pool pool(hostMemoryResource());

    const Tape<T>             tape(pool, expression);
    const TriMeshSDF<T, K, W> triMesh(soup.convertToTriangles(), pool, BVH::Construction::SAH, 4);

    for (std::size_t i = 0; i < count; i++) {
      a_tape[i] = tape.signedDistance(points[i]);
      a_mesh[i] = triMesh.signedDistance(points[i]);
    }
  };

  std::vector<T> tapeReference(count);
  std::vector<T> meshReference(count);

  build(tapeReference, meshReference);

  std::vector<std::vector<T>> tapes(NumThreads, std::vector<T>(count));
  std::vector<std::vector<T>> meshes(NumThreads, std::vector<T>(count));

  inThreads([&](const int a_thread) { build(tapes[std::size_t(a_thread)], meshes[std::size_t(a_thread)]); });

  for (int t = 0; t < NumThreads; t++) {
    CHECK(tapes[std::size_t(t)] == tapeReference);
    CHECK(meshes[std::size_t(t)] == meshReference);
  }
}
