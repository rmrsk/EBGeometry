// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A mesh placed many times, two ways: shared between translated placements in a Tape alongside
// analytic shapes, and as separate copies under a BVHUnion, which nests one BVH (over the placements)
// above another (over each mesh's triangles). Both are evaluated on the host and, through the same
// query, on a GPU. See README.md.

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;

// The BVH branching factor (K) and triangle-group width (W) that the readers use by default. A
// TriMeshSDF with these is a built-in primitive of Tape<T>; any other must be listed as a user type.
constexpr std::size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();
constexpr std::size_t W = EBGeometry::TriangleSoA::DefaultWidth<T>();

using Mesh     = EBGeometry::TriMeshSDF<T, K, W>;
using Nested   = EBGeometry::BVHUnion<T, Mesh, K>;
using Composed = EBGeometry::Tape<T>;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE
// call operator maps one point to one result. It holds the tape and the nested union by value. Both
// are descriptors of what they built in a Pool, so a kernel can receive them once they describe a
// pool the device can read (see the last section of main()).
struct Values
{
  T composed;
  T nested;
};

struct BothScenes
{
  Composed m_composed;
  Nested   m_nested;

  EBGEOMETRY_HOST_DEVICE
  Values
  operator()(const Vec3& a_point) const noexcept
  {
    return {m_composed.signedDistance(a_point), m_nested.signedDistance(a_point)};
  }
};

} // namespace

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  // ── Read the mesh ──────────────────────────────────────────────────────────────────────────────────
  //
  // Any watertight, orientable surface mesh in STL, PLY, VTK or OBJ. The default is a small
  // dodecahedron in Examples/data; pass another on the command line. Arguments starting with -- are
  // options (--no-view keeps the viewer shut).
  std::string file = "../data/dodecahedron.stl";

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]).rfind("--", 0) != 0) {
      file = argv[i];

      break;
    }
  }

  std::cout << "Reading '" << file << "'\n";

  // The meshes, their BVHs, the tape and the union are all reserved from this one Pool, which must
  // outlive them and every copy of them.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Mesh mesh = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool);

  // The mesh's size sets the spacing of the placements.
  const BV   meshBox = mesh.computeBoundingVolume();
  const Vec3 center  = T(0.5) * (meshBox.getLowCorner() + meshBox.getHighCorner());
  const T    size    = (meshBox.getHighCorner() - meshBox.getLowCorner()).length();

  // ── One mesh, shared: a Tape ───────────────────────────────────────────────────────────────────────
  //
  // The expression builder describes the object: Translate(mesh, shift) places the mesh without
  // copying it, and Union combines objects of any type, here the placed mesh and spheres. The Tape
  // compiles the description into the pool. A union of eight or more operands is evaluated through a
  // BVH over their bounding boxes, so a query only evaluates the objects near it.
  std::vector<EBGeometry::Expr<T>> objects;

  for (int i = 0; i < 6; i++) {
    const Vec3 shift(T(1.5) * size * T(i), T(0), T(0));

    objects.emplace_back(EBGeometry::Translate(mesh, shift));
    objects.emplace_back(EBGeometry::SphereSDF<T>(center + shift + Vec3(T(0), T(0.75) * size, T(0)), T(0.25) * size));
  }

  const Composed composed(pool, EBGeometry::Union(objects));

  std::cout << "  Tape   : 6 placements of the mesh and 6 spheres, " << composed.getNumClauses() << " clauses, "
            << composed.getNumBVHUnions() << " BVH union\n";

  // ── Many copies: a nested BVH ──────────────────────────────────────────────────────────────────────
  //
  // A BVHUnion holds primitives of one type by value and builds a BVH over their bounding boxes. Its
  // primitives here are TriMeshSDFs, each with a BVH of its own over its triangles, so one query
  // descends two levels: the outer BVH finds the placements near the point, and each of those meshes'
  // own BVH finds the nearest triangle. Each placement is a separate mesh with its own (translated)
  // triangles, so genuinely different meshes of the same type could be placed the same way. The first
  // row of the 4 x 4 grid coincides with the tape's placements.
  const auto triangles = EBGeometry::Parser::readIntoTriangles<T>(file);

  std::vector<Mesh> copies;
  std::vector<BV>   boxes;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      const Vec3 shift(T(1.5) * size * T(i), T(0), T(1.5) * size * T(j));

      auto shifted = triangles;

      for (auto& triangle : shifted) {
        auto vertices = triangle.getVertexPositions();

        for (auto& v : vertices) {
          v = v + shift;
        }

        triangle.setVertexPositions(vertices);
      }

      copies.emplace_back(shifted, pool, EBGeometry::BVH::Construction::SAH, 4);
      boxes.push_back(copies.back().computeBoundingVolume());
    }
  }

  const Nested nested(pool, copies, boxes);

  std::cout << "  Nested : a BVH union of " << copies.size() << " copies of the mesh, each with its own BVH\n\n";

  // ── Use them on the host ───────────────────────────────────────────────────────────────────────────
  //
  // Both are shapes: signedDistance(point) is negative inside any placement and positive outside all of
  // them, and computeBoundingVolume() encloses every placement.
  // The points run along the first row of placements, which both scenes share: through the center of
  // a placement, then halfway to the next one, and so on.
  std::vector<Vec3> points;

  for (int i = 0; i <= 8; i++) {
    points.push_back(center + Vec3(T(0.75) * size * T(i), T(0), T(0)));
  }

  std::cout << "Along the first row of placements:\n";

  for (const Vec3& p : points) {
    std::cout << "  " << p << "   tape " << composed.signedDistance(p) << "   nested " << nested.signedDistance(p)
              << '\n';
  }

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // The pool is frozen, mirrored in one piece into GPU::memoryResource() (device memory in a CUDA or
  // HIP build), and the tape and the union are rebased onto the copy, every mesh with them.
  // GPU::transform() then runs the query once per point in a kernel, both levels of BVH traversal
  // included. In a plain C++ build the same lines run on the host. The values agree with the host's to
  // rounding, since a device compiler may fuse a multiply and an add where the host compiler does not.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "\nOn the device: skipped, no GPU is visible\n";
  }
  else {
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

    const BothScenes query{composed.rebasedView(devicePool), nested.rebasedView(devicePool)};

    std::vector<Values> onDevice(points.size());

    EBGeometry::GPU::transform(query, points, onDevice);

    std::cout << "\nThrough GPU::transform (backend: " << EBGeometry::GPU::backendName << "):\n";

    for (std::size_t i = 0; i < points.size(); i++) {
      std::cout << "  " << points[i] << "   tape " << onDevice[i].composed << "   nested " << onDevice[i].nested
                << '\n';
    }
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the tape that places the mesh among the spheres (built with VIEWER=SDL2; otherwise, with
  // --no-view or without a display, view() prints why and returns). Drag to orbit, scroll to zoom, X cuts it
  // open, P cycles the coloring.
  // Given the pool, view() renders on the GPU in a GPU build (G switches to the CPU and back).
  EBGeometry::Render::view(composed, pool, argc, argv);

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n';

  return 1;
}
