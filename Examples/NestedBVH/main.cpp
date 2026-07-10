// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using namespace EBGeometry;

using T    = EBGEOMETRY_PRECISION;
using Meta = short;

// Branching factor for the outer union BVH. Defaults to the SIMD-optimal value for T on the
// compiled ISA -- the same default readIntoTriangleBVH uses for the inner mesh BVHs.
constexpr size_t K = BVH::DefaultBranchingRatio<T>();

using Vec3 = EBGeometry::Vec3T<T>;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
using IF   = EBGeometry::ImplicitFunction<T>;

int
main(int argc, char* argv[])
{
  // This example builds a *nested* bounding volume hierarchy: an outer BVH-accelerated CSG union
  // (BVHUnionIF) whose primitives are themselves BVH-backed mesh signed distance functions
  // (TriMeshSDF). Each TriMeshSDF owns an inner PackedBVH over its SoA triangle groups, so a single
  // distance query descends the outer union BVH to locate the nearby mesh(es), then descends each of
  // those meshes' own inner BVH -- two levels of BVH traversal for one query.
  //
  // The outer union stores its primitives as std::shared_ptr<const ImplicitFunction<T>> (the default
  // SharedPtrStorage): it shares each mesh SDF by pointer rather than copying it. This is the
  // recommended way to nest BVHs -- and nesting can recurse to any depth (a BVH of BVHs of BVHs...).
  // See the "Storage policy" section in the user documentation of the BVH implementation for why the
  // outer level must not use ValueStorage here: ImplicitFunction<T> is polymorphic so it
  // cannot be stored by value at all, and even for a concrete primitive by-value storage copies each
  // inner BVH's entire memory footprint, which becomes exceedingly expensive for large inner BVHs and
  // compounds with nesting depth.

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

  // Read the mesh directly into a BVH-backed TriMeshSDF using the library's default parameters
  // (SIMD-optimal branching factor and SoA width, ValueStorage, SAH build). Build it *once*, then
  // instance it at several positions below: because every placement is the same mesh, they all
  // share this single TriMeshSDF -- its (value-stored) inner packed BVH is built and stored exactly
  // once, and each placement is just an EBGeometry::Translate wrapper holding a shared_ptr to that
  // same object, shared by pointer and never rebuilt or copied per placement. This is the idiomatic
  // way to replicate one mesh across a scene. (To place genuinely different meshes instead, read one
  // TriMeshSDF per mesh file and translate each.)
  const auto tri   = EBGeometry::Parser::readIntoTriangleBVH<T, Meta>(file);
  const BV   triBV = tri->computeBoundingVolume();

  const std::vector<Vec3> shifts = {
    Vec3(0, 0, 0),
    Vec3(4, 0, 0),
    Vec3(0, 4, 0),
    Vec3(4, 4, 0),
    Vec3(2, 2, 4),
  };

  std::vector<std::shared_ptr<IF>> primitives;
  std::vector<BV>                  boundingVolumes;
  primitives.reserve(shifts.size());
  boundingVolumes.reserve(shifts.size());

  // Each placement shares the one inner mesh BVH; only its translation -- and its bounding volume
  // for the outer union (the mesh's own AABB shifted by the same offset) -- differs.
  for (const Vec3& shift : shifts) {
    primitives.emplace_back(EBGeometry::Translate<T>(tri, shift));
    boundingVolumes.emplace_back(triBV.getLowCorner() + shift, triBV.getHighCorner() + shift);
  }

  // Outer BVH: a BVH-accelerated union over the placements. This is the nested (two-level) BVH.
  const auto nestedUnion = EBGeometry::BVHUnion<T, IF, BV, K>(primitives, boundingVolumes);

  std::cout << "Built a BVH union over " << primitives.size()
            << " placements of one BVH-backed mesh SDF (a nested, two-level BVH).\n";

  // Evaluate at a few points: on a placement, between placements, and far outside everything.
  for (const Vec3& p : {Vec3(0, 0, 0), Vec3(2, 2, 0), Vec3(20, 20, 20)}) {
    std::cout << "value(" << p << ") = " << nestedUnion->value(p) << "\n";
  }

  return 0;
}
