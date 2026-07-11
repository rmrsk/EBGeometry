// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Benchmark: EBGeometry TriMeshSDF vs fcpw (https://github.com/rohan-sawhney/fcpw) on closest-point
// queries over a triangle mesh -- the task both libraries are built for (a BVH over triangles,
// answering "closest point on the surface"). Float precision (fcpw is float).
//
// The mesh is parsed once (shared, untimed) from the common-3d-test-models submodule. Each library
// then builds its own BVH over the same triangles (timed) and answers the same random closest-point
// queries (timed). Results are cross-checked: |TriMeshSDF signed distance| vs fcpw's unsigned
// closest-surface distance. Note TriMeshSDF additionally computes the sign (it is a signed-distance
// library); fcpw's findClosestPoint returns the unsigned closest point.

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

#include <fcpw/fcpw.h>

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
  scene.build(fcpw::AggregateType::Bvh_SurfaceArea, false /* vectorize (needs Enoki) */);
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

  // Cross-check on a spread sample: unsigned closest-surface distance must agree.
  std::size_t bad = 0;
  for (std::size_t s = 0; s < sampleSize; s++) {
    const std::size_t i = s * (numQueries / sampleSize);
    if (std::abs(ebDist[i] - fcpwDist[i]) > T(1.0e-3) * std::max(fcpwDist[i], T(1))) {
      bad++;
    }
  }

  std::printf("  %-12s  build %7.1f ms   query %7.3f us/query\n", "TriMeshSDF", ebBuildMs, ebQueryUs);
  std::printf("  %-12s  build %7.1f ms   query %7.3f us/query\n", "fcpw", fcpwBuildMs, fcpwQueryUs);
  std::printf("  cross-check: %zu/%zu sample mismatches (|TriMeshSDF| vs fcpw unsigned distance)\n", bad, sampleSize);
  return 0;
}
