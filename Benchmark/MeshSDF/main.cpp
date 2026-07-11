// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Benchmark: EBGeometry TriMeshSDF vs fcpw (https://github.com/rohan-sawhney/fcpw) vs
// TriangleMeshDistance (https://github.com/InteractiveComputerGraphics/TriangleMeshDistance) on
// closest-point queries over a triangle mesh -- the task all three are built for (an acceleration
// structure over triangles, answering "closest point on the surface").
//
// The mesh is parsed once (shared, untimed) from the common-3d-test-models submodule. Each library
// then builds its own structure over the same triangles (timed) and answers the same random
// closest-point queries (timed). Results are cross-checked against TriMeshSDF's unsigned distance.
//
// Precision / vectorization caveats (each library on its own intended fast path):
//   - TriMeshSDF   -- float, SIMD-vectorized (this build's native ISA).
//   - fcpw         -- float; built with its Enoki CPU vectorization (FCPW_USE_ENOKI, vectorized BVH).
//                     fcpw returns an unsigned closest point; TriMeshSDF additionally computes a sign.
//   - TriangleMeshDistance -- double, scalar (header-only, no SIMD). A signed-distance library like
//                     TriMeshSDF; runs its queries in double, so it is not a same-precision comparison.

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include <fcpw/fcpw.h>
#include <tmd/TriangleMeshDistance.h>

using T    = float; // fcpw is float
using Vec3 = EBGeometry::Vec3T<T>;
using Meta = EBGeometry::DCEL::DefaultMetaData;

// SIMD-optimal branching factor / SoA triangle width for T (what readIntoTriangleBVH would pick).
constexpr std::size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();
constexpr std::size_t W = EBGeometry::TriangleSoA::DefaultWidth<T>();
using SDF               = EBGeometry::TriMeshSDF<T, Meta, K, W>;

int
main(int argc, char** argv)
{
  const std::string     objFile    = (argc > 1) ? argv[1] : "../../Submodules/common-3d-test-models/data/armadillo.obj";
  constexpr std::size_t numQueries = 100000;
  constexpr std::size_t sampleSize = 500;
  constexpr std::size_t maxLeafGroups = 4;

  // Parse the mesh once -- shared preamble, not part of either library's timed build.
  const auto        tris = EBGeometry::Parser::readIntoTriangles<T, Meta>(objFile);
  const std::size_t nTri = tris.size();

  std::printf("MeshSDF closest-point: EBGeometry TriMeshSDF vs fcpw\n");
  std::printf("  Mesh = %s (%zu triangles, float)\n  Queries = %zu\n\n", objFile.c_str(), nTri, numQueries);

  EBGeometry::SimpleTimer timer;

  // Random query points in the mesh bounding box, expanded to 1.5x so queries sit inside and around.
  Vec3 lo = +Vec3::max();
  Vec3 hi = -Vec3::max();
  for (const auto& tri : tris) {
    for (const auto& p : tri->getVertexPositions()) {
      lo = min(lo, p);
      hi = max(hi, p);
    }
  }
  const Vec3                        center = T(0.5) * (lo + hi);
  const Vec3                        half   = T(0.75) * (hi - lo);
  std::mt19937                      rng(12345u);
  std::uniform_real_distribution<T> u(T(-1), T(1));
  std::vector<Vec3>                 queries(numQueries);
  for (auto& q : queries) {
    q = center + Vec3(u(rng) * half[0], u(rng) * half[1], u(rng) * half[2]);
  }

  // ── EBGeometry TriMeshSDF ──
  timer.start();
  const SDF sdf(tris, EBGeometry::BVH::Build::SAH, maxLeafGroups);
  timer.stop();
  const double ebBuildMs = 1.0e3 * timer.seconds();

  std::vector<T> ebDist(numQueries);
  timer.start();
  for (std::size_t i = 0; i < numQueries; i++) {
    ebDist[i] = std::abs(sdf.signedDistance(queries[i]));
  }
  timer.stop();
  const double ebQueryUs = 1.0e6 * timer.seconds() / double(numQueries);

  // ── fcpw ──
  // fcpw needs vertices + triangle indices. Use an unwelded soup (3 vertices per triangle); closest
  // point on triangles is unaffected by vertex sharing. Building this soup is fcpw's setup, not parse.
  std::vector<fcpw::Vector<3>> V;
  std::vector<fcpw::Vector3i>  F;
  V.reserve(3 * nTri);
  F.reserve(nTri);
  timer.start();
  for (const auto& tri : tris) {
    const auto& p    = tri->getVertexPositions();
    const int   base = static_cast<int>(V.size());
    for (int k = 0; k < 3; k++) {
      V.emplace_back(fcpw::Vector<3>(p[k][0], p[k][1], p[k][2]));
    }
    F.emplace_back(fcpw::Vector3i(base, base + 1, base + 2));
  }
  fcpw::Scene<3> scene;
  scene.setObjectCount(1);
  scene.setObjectVertices(V, 0);
  scene.setObjectTriangles(F, 0);
  scene.build(fcpw::AggregateType::Bvh_SurfaceArea, true /* vectorize (Enoki MBVH) */);
  timer.stop();
  const double fcpwBuildMs = 1.0e3 * timer.seconds();

  std::vector<T> fcpwDist(numQueries);
  timer.start();
  for (std::size_t i = 0; i < numQueries; i++) {
    fcpw::Interaction<3> it;
    scene.findClosestPoint(fcpw::Vector<3>(queries[i][0], queries[i][1], queries[i][2]), it);
    fcpwDist[i] = it.d;
  }
  timer.stop();
  const double fcpwQueryUs = 1.0e6 * timer.seconds() / double(numQueries);

  // ── TriangleMeshDistance (double, scalar, header-only) ──
  // Same unwelded triangle soup, in double. Building the arrays + the structure is its timed setup,
  // mirroring how fcpw's soup construction is folded into fcpw's build above.
  std::vector<double> tmdVertices;
  std::vector<int>    tmdTriangles;
  tmdVertices.reserve(9 * nTri);
  tmdTriangles.reserve(3 * nTri);
  timer.start();
  for (const auto& tri : tris) {
    const auto& p    = tri->getVertexPositions();
    const int   base = static_cast<int>(tmdVertices.size() / 3);
    for (int k = 0; k < 3; k++) {
      tmdVertices.push_back(double(p[k][0]));
      tmdVertices.push_back(double(p[k][1]));
      tmdVertices.push_back(double(p[k][2]));
    }
    tmdTriangles.push_back(base);
    tmdTriangles.push_back(base + 1);
    tmdTriangles.push_back(base + 2);
  }
  const tmd::TriangleMeshDistance tmdMesh(
    tmdVertices.data(), tmdVertices.size() / 3, tmdTriangles.data(), tmdTriangles.size() / 3);
  timer.stop();
  const double tmdBuildMs = 1.0e3 * timer.seconds();

  std::vector<T> tmdDist(numQueries);
  timer.start();
  for (std::size_t i = 0; i < numQueries; i++) {
    const tmd::Result r =
      tmdMesh.signed_distance({double(queries[i][0]), double(queries[i][1]), double(queries[i][2])});
    tmdDist[i] = T(std::abs(r.distance));
  }
  timer.stop();
  const double tmdQueryUs = 1.0e6 * timer.seconds() / double(numQueries);

  // Cross-check on a spread sample: unsigned closest-surface distance must agree across all three.
  std::size_t badFcpw = 0;
  std::size_t badTmd  = 0;
  for (std::size_t s = 0; s < sampleSize; s++) {
    const std::size_t i = s * (numQueries / sampleSize);
    if (std::abs(ebDist[i] - fcpwDist[i]) > T(1.0e-3) * std::max(fcpwDist[i], T(1))) {
      badFcpw++;
    }
    if (std::abs(ebDist[i] - tmdDist[i]) > T(1.0e-3) * std::max(tmdDist[i], T(1))) {
      badTmd++;
    }
  }

  std::printf("  %-22s  build %7.1f ms   query %7.3f us/query\n", "TriMeshSDF", ebBuildMs, ebQueryUs);
  std::printf("  %-22s  build %7.1f ms   query %7.3f us/query\n", "fcpw (Enoki)", fcpwBuildMs, fcpwQueryUs);
  std::printf("  %-22s  build %7.1f ms   query %7.3f us/query\n", "TriangleMeshDistance", tmdBuildMs, tmdQueryUs);
  std::printf("  cross-check vs TriMeshSDF: fcpw %zu/%zu, TriangleMeshDistance %zu/%zu sample mismatches\n",
              badFcpw,
              sampleSize,
              badTmd,
              sampleSize);
  return 0;
}
