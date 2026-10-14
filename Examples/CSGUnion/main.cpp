// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;

// A mesh signed distance function with the library's default branching factor, which is one of the
// tape's built-in primitive types.
using Mesh = EBGeometry::MeshSDF<T, EBGeometry::BVH::DefaultBranchingRatio<T>()>;

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  // This example composes objects of *different* types -- a mesh signed distance function and
  // analytic spheres -- into one object, and evaluates it through a Tape. The objects are combined
  // with the expression builder (Union, Translate, ...), which describes the composed object, and
  // the Tape compiles that description into a Pool: one array per primitive type, and a list of
  // clauses that refer to the primitives by type and index. Evaluating the tape calls no virtual
  // function, and the tape is a plain value that can be mirrored to a GPU like any other pool handle.
  //
  // The mesh is read once and placed several times: each placement is Translate(mesh, shift), which
  // shares the mesh rather than copying it. With this many objects, the tape evaluates the union with
  // a BVH over the objects' bounding boxes, so a query only evaluates the objects near it.

  // Mesh to place. Pass a path on the command line, or fall back to the dodecahedron fixture shipped
  // in the repository (Tests/data), so this example needs no submodule. The path is relative to this
  // example's own folder, which is the working directory when the example is run.
  std::string file = "../../Tests/data/dodecahedron.stl";

  if (argc >= 2) {
    file = std::string(argv[1]);
  }
  else {
    std::cout << "No mesh file given; defaulting to the in-repo dodecahedron.stl fixture.\n"
              << "Usage: ./CSGUnion.ex <mesh-file>  (STL/PLY/VTK/OBJ)\n";
  }

  // The mesh, its BVH and the tape are all reserved from this one Pool, which must outlive them.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto mesh    = EBGeometry::Parser::readIntoDCEL<T>(file, pool);
  const Mesh meshSDF = Mesh(mesh, pool, EBGeometry::BVH::Construction::SAH);

  // The mesh's size sets the spacing of the placements and the spheres' radius.
  const BV   meshBox = meshSDF.computeBoundingVolume();
  const Vec3 extent  = meshBox.getHighCorner() - meshBox.getLowCorner();
  const Vec3 center  = T(0.5) * (meshBox.getLowCorner() + meshBox.getHighCorner());
  const T    size    = extent.length();
  const T    radius  = T(0.25) * size;

  // A row of placements of the mesh, each with a sphere above it.
  std::vector<Vec3>                     shifts;
  std::vector<EBGeometry::SphereSDF<T>> spheres;
  std::vector<EBGeometry::Expr<T>>      objects;

  for (int i = 0; i < 6; i++) {
    const Vec3 shift(T(1.5) * size * T(i), T(0), T(0));

    shifts.push_back(shift);
    spheres.emplace_back(center + shift + Vec3(T(0), T(0.75) * size, T(0)), radius);

    objects.emplace_back(EBGeometry::Translate(meshSDF, shift));
    objects.emplace_back(spheres.back());
  }

  // Compile the union into the pool.
  const EBGeometry::Tape<T> tape(pool, EBGeometry::Union(objects));

  std::cout << "Compiled a union of " << shifts.size() << " placements of one mesh and " << spheres.size()
            << " spheres into a tape: " << tape.getNumPrograms() << " sub-programs, " << tape.getNumClauses()
            << " clauses, " << tape.getNumBVHUnions() << " BVH union.\n";

  for (const Vec3& p : {center, center + Vec3(T(0), T(0.75) * size, T(0)), center + Vec3(T(0), T(5) * size, T(0))}) {
    std::cout << "value(" << p << ") = " << tape.signedDistance(p) << "\n";
  }

  // Check the tape against the smallest distance over every object, evaluated directly, at random
  // points in a box around the scene. The BVH only skips objects that cannot be the closest, and the
  // translation is applied the same way, so the two agree up to rounding. A fixed seed gives the same
  // points on every run.
  const BV   box         = tape.computeBoundingVolume();
  const Vec3 sceneExtent = box.getHighCorner() - box.getLowCorner();
  const T    tolerance   = (std::is_same_v<T, float> ? T(1.0e-5) : T(1.0e-12)) * sceneExtent.length();

  std::mt19937_64                   rng(12345);
  std::uniform_real_distribution<T> unit(T(0), T(1));

  constexpr int numSamples = 1000;

  int mismatches = 0;

  for (int i = 0; i < numSamples; i++) {
    const Vec3 p =
      box.getLowCorner() - T(0.25) * sceneExtent + T(1.5) * sceneExtent * Vec3(unit(rng), unit(rng), unit(rng));

    T closest = EBGeometry::Math::Limits<T>::infinity();

    for (size_t j = 0; j < shifts.size(); j++) {
      closest = EBGeometry::Math::min(closest, meshSDF.signedDistance(p - shifts[j]));
      closest = EBGeometry::Math::min(closest, spheres[j].signedDistance(p));
    }

    mismatches += (std::abs(tape.signedDistance(p) - closest) > tolerance) ? 1 : 0;
  }

  std::cout << "Points where the tape differs from a scan over the objects = " << mismatches << " of " << numSamples
            << "\n";

  return mismatches == 0 ? 0 : 1;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n';

  return 1;
}
