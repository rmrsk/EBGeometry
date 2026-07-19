// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>
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
using Meta = short;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
using Vec3 = EBGeometry::Vec3T<T>;

int
main(int argc, char* argv[])
{
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
  const auto dcelSDF = EBGeometry::Parser::readIntoMesh<T, Meta>(file);
  const auto meshSDF = EBGeometry::Parser::readIntoPackedBVH<T, Meta, K>(file);
  const auto triSDF  = EBGeometry::Parser::readIntoTriangleBVH<T, Meta>(file, 4, BVH::Build::SAH);

  // Sample some random points around the object.
  constexpr size_t Nsamp = 1000;

  Vec3 lo = Vec3::infinity();
  Vec3 hi = -Vec3::infinity();

  for (const auto& v : dcelSDF->getMesh()->getAllVertexCoordinates()) {
    lo = min(lo, v);
    hi = max(hi, v);
  }
  const Vec3 delta = hi - lo;

  std::mt19937_64 rng(static_cast<size_t>(std::chrono::system_clock::now().time_since_epoch().count()));

  std::uniform_real_distribution<T> dist(0.0, 1.0);
  std::vector<Vec3>                 ranPoints;

  for (size_t i = 0; i < Nsamp; i++) {
    ranPoints.emplace_back(3 * (lo + delta * Vec3(dist(rng), dist(rng), dist(rng))));
  }

  // Compute sum of distances to the random points and time each representation.
  T dcelSum = 0.0;
  T meshSum = 0.0;
  T triSum  = 0.0;

  const auto t0 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    dcelSum += dcelSDF->value(x);
  }
  const auto t1 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    meshSum += meshSDF->value(x);
  }
  const auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    triSum += triSDF->value(x);
  }
  const auto t3 = std::chrono::high_resolution_clock::now();

  const std::chrono::duration<T, std::micro> dcelTime = t1 - t0;
  const std::chrono::duration<T, std::micro> meshTime = t2 - t1;
  const std::chrono::duration<T, std::micro> triTime  = t3 - t2;

  // Summing Nsamp signed distances in a different order (brute-force scan vs. BVH traversal)
  // is not bit-for-bit reproducible -- floating-point addition isn't associative -- so compare
  // the two sums with a relative tolerance rather than requiring exact agreement. float needs a
  // looser tolerance than double: accumulating Nsamp terms in a different order can land right
  // at float's own ~1.19e-7 epsilon (observed relative diff ~1.2e-7 on the armadillo mesh, which
  // a flat 1e-7 tolerance flagged as a mismatch).
  constexpr T relativeTolerance = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-7);

  bool    mismatch  = false;
  const T meshScale = std::max(std::abs(dcelSum), std::abs(meshSum));
  if (std::abs(meshSum - dcelSum) > relativeTolerance * std::max(meshScale, T(1.0))) {
    std::cerr << "MeshSDF did not give same distance as FlatMeshSDF! Diff = " << meshSum - dcelSum << "\n";
    mismatch = true;
  }
  const T triScale = std::max(std::abs(dcelSum), std::abs(triSum));
  if (std::abs(triSum - dcelSum) > relativeTolerance * std::max(triScale, T(1.0))) {
    std::cerr << "TriMeshSDF did not give same distance as FlatMeshSDF! Diff = " << triSum - dcelSum << "\n";
    mismatch = true;
  }
  if (mismatch) {
    return 1;
  }

  // clang-format off
  std::cout << "Bounding box = " << lo << "\t" << hi << "\n";
  std::cout << "Accumulated distance and time using FlatMeshSDF           = " << dcelSum << ", which took " << dcelTime.count() / Nsamp << " us\n";
  std::cout << "Accumulated distance and time using MeshSDF (PackedBVH)   = " << meshSum << ", which took " << meshTime.count() / Nsamp << " us\n";
  std::cout << "Accumulated distance and time using TriMeshSDF (PackedBVH)= " << triSum  << ", which took " << triTime.count()  / Nsamp << " us\n";
  std::cout << "Relative speedup MeshSDF vs FlatMeshSDF                   = " << dcelTime.count() / meshTime.count() << "\n";
  std::cout << "Relative speedup TriMeshSDF vs FlatMeshSDF                = " << dcelTime.count() / triTime.count()  << "\n";
  // clang-format on

  return 0;
}
