// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
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

using T = EBGEOMETRY_PRECISION;

// Branching factor for the outer union BVH: the library default (4 on every machine and on a GPU),
// the same default readIntoTriMeshSDF uses for the inner mesh BVHs.
constexpr size_t K = BVH::DefaultBranchingRatio<T>();

using Vec3 = EBGeometry::Vec3T<T>;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
using Mesh = EBGeometry::TriMeshSDF<T, K, EBGeometry::TriangleSoA::DefaultWidth<T>()>;

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  // This example builds a *nested* bounding volume hierarchy: an outer BVH-accelerated union
  // (BVHUnion) whose primitives are themselves BVH-backed mesh signed distance functions
  // (TriMeshSDF). Each TriMeshSDF owns an inner PackedBVH over its SoA triangle groups, so a single
  // distance query descends the outer union BVH to locate the nearby mesh(es), then descends each of
  // those meshes' own inner BVH -- two levels of BVH traversal for one query.
  //
  // The outer union stores its primitives by value, in the same Pool as the meshes' own BVHs, and is
  // itself a plain value type: the whole two-level hierarchy can be mirrored to a GPU in one piece
  // and evaluated there. Nesting can recurse -- a union of such unions works the same way.

  // Mesh to place at several positions. Pass a path on the command line, or fall back to the
  // dodecahedron fixture shipped in the repository (Tests/data), so this example needs no
  // submodule. The path is relative to this example's own folder, which is the working directory
  // when the example is run.
  std::string file = "../../Tests/data/dodecahedron.stl";

  if (argc >= 2) {
    file = std::string(argv[1]);
  }
  else {
    std::cout << "No mesh file given; defaulting to the in-repo dodecahedron.stl fixture.\n"
              << "Usage: ./NestedBVH.ex <mesh-file>  (STL/PLY/VTK/OBJ, triangles only)\n";
  }

  // Every mesh, and the outer union, is reserved from this one Pool, which must outlive them all.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  // Read the mesh's triangles once. Each placement below gets its own translated copy, from which a
  // TriMeshSDF (with its own inner BVH) is built using the library's default parameters. A union
  // holds primitives of a single type, so placing genuinely different meshes works the same way, as
  // long as they are all TriMeshSDF<T, K, W> with the same parameters.
  const auto triangles = EBGeometry::Parser::readIntoTriangles<T>(file);

  const std::vector<Vec3> shifts = {
    Vec3(0, 0, 0),
    Vec3(4, 0, 0),
    Vec3(0, 4, 0),
    Vec3(4, 4, 0),
    Vec3(2, 2, 4),
  };

  std::vector<Mesh> primitives;
  std::vector<BV>   boundingVolumes;

  primitives.reserve(shifts.size());
  boundingVolumes.reserve(shifts.size());

  for (const Vec3& shift : shifts) {
    auto shifted = triangles;

    for (auto& triangle : shifted) {
      auto vertices = triangle.getVertexPositions();

      for (auto& v : vertices) {
        v = v + shift;
      }

      triangle.setVertexPositions(vertices);
    }

    primitives.emplace_back(shifted, pool, EBGeometry::BVH::Construction::SAH, 4);
    boundingVolumes.push_back(primitives.back().computeBoundingVolume());
  }

  // Outer BVH: a BVH-accelerated union over the placements. This is the nested (two-level) BVH.
  const auto nestedUnion = EBGeometry::BVHUnion<T, Mesh, K>(pool, primitives, boundingVolumes);

  std::cout << "Built a BVH union over " << primitives.size()
            << " placements of a BVH-backed mesh SDF (a nested, two-level BVH).\n";

  // Evaluate at a few points: on a placement, between placements, and far outside everything.
  for (const Vec3& p : {Vec3(0, 0, 0), Vec3(2, 2, 0), Vec3(20, 20, 20)}) {
    std::cout << "value(" << p << ") = " << nestedUnion.signedDistance(p) << "\n";
  }

  // Check the nested union against the smallest distance over every placement, at random points in
  // a box twice the size of the union's bounding box. The outer BVH only skips placements that cannot
  // be the closest, so the two agree up to rounding, which scales with the size of the scene. A fixed
  // seed gives the same points on every run.
  const BV   box       = nestedUnion.computeBoundingVolume();
  const Vec3 extent    = box.getHighCorner() - box.getLowCorner();
  const T    tolerance = (std::is_same_v<T, float> ? T(1.0e-5) : T(1.0e-12)) * extent.length();

  std::mt19937_64                   rng(12345);
  std::uniform_real_distribution<T> unit(T(0), T(1));

  constexpr int numSamples = 1000;

  int mismatches = 0;

  for (int i = 0; i < numSamples; i++) {
    const Vec3 p = box.getLowCorner() - T(0.5) * extent + T(2) * extent * Vec3(unit(rng), unit(rng), unit(rng));

    T closest = std::numeric_limits<T>::infinity();

    for (const Mesh& mesh : primitives) {
      closest = std::min(closest, mesh.signedDistance(p));
    }

    mismatches += (std::abs(nestedUnion.signedDistance(p) - closest) > tolerance) ? 1 : 0;
  }

  std::cout << "Points where the nested union differs from a scan over the placements = " << mismatches << " of "
            << numSamples << "\n";

  return mismatches == 0 ? 0 : 1;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n';

  return 1;
}
