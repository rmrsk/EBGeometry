// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using namespace EBGeometry;
using namespace EBGeometry::DCEL;

// Degree of bounding volume hierarchies. We use a 4-ary tree here, where each
// regular node has four children.
constexpr int K = 4;

using T    = EBGEOMETRY_PRECISION;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
using Vec3 = EBGeometry::Vec3T<T>;

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  // Path to a surface mesh (STL/PLY/VTK/OBJ). Pass one on the command line, e.g.
  //   ./a.out ../../common-3d-test-models/data/cow.obj
  // Paths are resolved relative to the run directory (this example's source folder when run via ctest).
  std::string file = "../../common-3d-test-models/data/armadillo.obj";

  if (argc == 2) {
    file = std::string(argv[1]);
  }
  else {
    std::cout << "No mesh file given; defaulting to '" << file << "'.\n"
              << "Usage: ./a.out <path-to-mesh>, e.g. a .obj from the common-3d-test-models submodule "
                 "(see the Building and using page in the docs for how to fetch it).\n";
  }

  // Three representations of the same object:
  //   dcelSDF  – FlatMeshSDF: O(N) brute-force over all faces.
  //   meshSDF  – MeshSDF: PackedBVH over DCEL faces (any polygon, SIMD traversal).
  //   triSDF   – TriMeshSDF: PackedBVH over SoA triangle groups (triangles only, SIMD leaf eval).
  //
  // Note that this reads the mesh and builds the BVH tree independently for each
  // representation. There are converters that avoid this, but users will almost always
  // only use one of these representations.
  // All three share one Pool. Each parse keeps reserving from it, growing (and moving) its block as
  // it goes, which is invisible to the meshes already built: a mesh resolves its storage through
  // the Pool's control block on every access rather than caching an address, so there is no point
  // at which the Pool has to be sealed off. dcelSDF and meshSDF retain their mesh for their whole
  // lifetime (see FlatMeshSDF/MeshSDF's docs), so the Pool must outlive them -- keeping it in
  // main()'s scope satisfies that.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto dcelSDF = EBGeometry::Parser::readIntoFlatMeshSDF<T>(file, pool);
  const auto meshSDF = EBGeometry::Parser::readIntoMeshSDF<T, K>(file, pool);
  const auto triSDF  = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool, BVH::Construction::SAH);

  // Sample some random points around the object. FlatMeshSDF visits every face for every point, so
  // a few hundred points already take minutes in an unoptimised build on a large mesh.
  constexpr size_t Nsamp = 100;

  Vec3 lo = Vec3::infinity();
  Vec3 hi = -Vec3::infinity();

  for (const auto& v : dcelSDF.getMesh().getAllVertexCoordinates()) {
    lo = min(lo, v);
    hi = max(hi, v);
  }

  const Vec3 delta = hi - lo;

  // The sample points fill a box three times the size of the mesh's bounding box, with the mesh at its
  // center, so most lie outside the mesh and some inside. A fixed seed gives the same points on every
  // run.
  std::mt19937_64                   rng(12345);
  std::uniform_real_distribution<T> dist(0.0, 1.0);
  std::vector<Vec3>                 ranPoints;

  for (size_t i = 0; i < Nsamp; i++) {
    ranPoints.emplace_back(lo - delta + T(3) * delta * Vec3(dist(rng), dist(rng), dist(rng)));
  }

  // Evaluate each representation at every point, timing each.
  std::vector<T> dcelDist(Nsamp);
  std::vector<T> meshDist(Nsamp);
  std::vector<T> triDist(Nsamp);

  const auto t0 = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < Nsamp; i++) {
    dcelDist[i] = dcelSDF.signedDistance(ranPoints[i]);
  }

  const auto t1 = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < Nsamp; i++) {
    meshDist[i] = meshSDF.signedDistance(ranPoints[i]);
  }

  const auto t2 = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < Nsamp; i++) {
    triDist[i] = triSDF.signedDistance(ranPoints[i]);
  }

  const auto t3 = std::chrono::high_resolution_clock::now();

  const std::chrono::duration<T, std::micro> dcelTime = t1 - t0;
  const std::chrono::duration<T, std::micro> meshTime = t2 - t1;
  const std::chrono::duration<T, std::micro> triTime  = t3 - t2;

  // The three must agree at every point, up to rounding: the BVHs only skip faces that cannot be the
  // closest, and TriMeshSDF's fan triangulation of a planar convex face gives the same distance. The
  // rounding scales with the size of the mesh, so the tolerance does too. A mesh with holes can
  // still disagree on the sign near a hole, since its inside is not well defined there.
  const T tolerance = (std::is_same_v<T, float> ? T(1.0e-5) : T(1.0e-10)) * delta.length();

  size_t meshMismatches = 0;
  size_t triMismatches  = 0;

  for (size_t i = 0; i < Nsamp; i++) {
    meshMismatches += (std::abs(meshDist[i] - dcelDist[i]) > tolerance) ? 1 : 0;
    triMismatches += (std::abs(triDist[i] - dcelDist[i]) > tolerance) ? 1 : 0;
  }

  // clang-format off
  std::cout << "Bounding box = " << lo << "\t" << hi << "\n";
  std::cout << "Time per query using FlatMeshSDF            = " << dcelTime.count() / Nsamp << " us\n";
  std::cout << "Time per query using MeshSDF (PackedBVH)    = " << meshTime.count() / Nsamp << " us\n";
  std::cout << "Time per query using TriMeshSDF (PackedBVH) = " << triTime.count()  / Nsamp << " us\n";
  std::cout << "Relative speedup MeshSDF vs FlatMeshSDF     = " << dcelTime.count() / meshTime.count() << "\n";
  std::cout << "Relative speedup TriMeshSDF vs FlatMeshSDF  = " << dcelTime.count() / triTime.count()  << "\n";
  std::cout << "Points where MeshSDF differs from FlatMeshSDF    = " << meshMismatches << " of " << Nsamp << "\n";
  std::cout << "Points where TriMeshSDF differs from FlatMeshSDF = " << triMismatches << " of " << Nsamp << "\n";
  // clang-format on

  if (meshMismatches > 0 || triMismatches > 0) {
    std::cerr << "The mesh SDFs disagree by more than " << tolerance << " (is the mesh watertight?)\n";

    return 1;
  }

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n';

  return 1;
}
