// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
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

// BVH branching factor for the union tree.
constexpr size_t K = 4;

using T       = EBGEOMETRY_PRECISION;
using Meta    = short;
using Vec3    = EBGeometry::Vec3T<T>;
using BV      = EBGeometry::BoundingVolumes::AABBT<T>;
using IF      = EBGeometry::ImplicitFunction<T>;
using MeshSDF = EBGeometry::FlatMeshSDF<T, Meta>;

int
main(int argc, char* argv[])
{
  // This example shows how to merge two objects of *different* kinds into a single implicit function
  // using a BVH-accelerated CSG union (BVHUnionIF): a triangulated surface read from a mesh file, and
  // an analytic sphere. Because the two objects have different C++ types, they are combined through
  // their common base class EBGeometry::ImplicitFunction<T>. BVHUnion builds a bounding volume
  // hierarchy over the objects so a distance query only descends into the object(s) whose bounding
  // volume is near the query point.

  // Mesh to merge with the sphere. Pass a path on the command line, or fall back to an OBJ file from
  // the common-3d-test-models submodule (path is relative to this example's source folder, where the
  // executable is run). See the "Building and using" docs for how to fetch the submodule.
  std::string file = "../../Submodules/common-3d-test-models/data/cow.obj";
  if (argc >= 2) {
    file = std::string(argv[1]);
  }
  else {
    std::cout << "No mesh file given; defaulting to cow.obj from the submodule.\n"
              << "Usage: ./a.out <mesh-file>  (STL/PLY/VTK/OBJ)\n";
  }

  // Read the mesh into a signed distance function and bound it with an axis-aligned box.
  const auto meshSDF = EBGeometry::Parser::readIntoMesh<T, Meta>(file);
  const BV   meshBV(meshSDF->getMesh()->getAllVertexCoordinates());

  // Analytic sphere centred on the mesh, with radius equal to the shortest axis (half-extent) of the
  // mesh bounding box. It is bounded by its own axis-aligned box for the union hierarchy.
  const Vec3& lo     = meshBV.getLowCorner();
  const Vec3& hi     = meshBV.getHighCorner();
  const Vec3  extent = hi - lo;
  const Vec3  center = 0.5 * (lo + hi);
  const T     radius = 0.5 * std::min({extent[0], extent[1], extent[2]});

  const auto sphere = std::make_shared<EBGeometry::SphereSDF<T>>(center, radius);
  const BV   sphereBV(center - Vec3(radius, radius, radius), center + Vec3(radius, radius, radius));

  // Merge the mesh and the sphere with a BVH-accelerated union. Both are passed as the common base
  // type ImplicitFunction<T>; the result's value is the minimum signed distance over the two objects.
  const std::vector<std::shared_ptr<IF>> primitives      = {meshSDF, sphere};
  const std::vector<BV>                  boundingVolumes = {meshBV, sphereBV};

  const auto csgUnion = EBGeometry::BVHUnion<T, IF, BV, K>(primitives, boundingVolumes);

  std::cout << "Merged a triangulated surface with an analytic sphere (radius " << radius << ").\n";
  std::cout << "Mesh bounding box = " << lo << " -> " << hi << "\n";
  std::cout << "value(center) = " << csgUnion->value(center) << "\n";
  std::cout << "value(far)    = " << csgUnion->value(hi + extent) << "\n";

  return 0;
}
