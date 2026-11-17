// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

// The point of a mesh closest to a query point, and the gradient of the signed distance in closed form from
// it: one query where a finite-difference normal takes six, and a unit vector that never needs a step size.
// The same query then runs on a GPU. At the end it opens a window on the mesh, if built with the viewer.
// See README.md.

#include <cstddef>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;

constexpr std::size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();
constexpr std::size_t W = EBGeometry::TriangleSoA::DefaultWidth<T>();

using TriMeshSDF = EBGeometry::TriMeshSDF<T, K, W>;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE call
// operator maps one point to one result, here the unit gradient of the signed distance. The TriMeshSDF it
// holds is a descriptor of a mesh in a Pool, so a kernel can receive it by value once it describes a pool
// the device can read.
struct Gradient
{
  TriMeshSDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  Vec3
  operator()(const Vec3& a_point) const noexcept
  {
    return m_sdf.gradient(a_point);
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
  // Any watertight, orientable surface mesh in STL, PLY, VTK or OBJ. The default comes from the
  // common-3d-test-models submodule (git submodule update --init); pass another on the command line.
  // Arguments starting with -- are options (--no-view keeps the viewer shut).
  std::string file = "../../common-3d-test-models/data/cow.obj";

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]).rfind("--", 0) != 0) {
      file = argv[i];

      break;
    }
  }

  std::cout << "Reading '" << file << "'\n";

  // The mesh lives in the pool; the TriMeshSDF is a descriptor into it, so the pool must outlive it.
  // FlatMeshSDF and MeshSDF have the same closestPoint() and gradient().
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto sdf = EBGeometry::Parser::readIntoTriMeshSDF<T>(
    file, pool, EBGeometry::BVH::Construction::SAH, EBGeometry::Parser::OnDefect::Warn);

  const EBGeometry::BoundingVolumes::AABBT<T> box = sdf.computeBoundingVolume();

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // A few random points in a box twice the size of the mesh's. A fixed seed gives the same points on every
  // run.
  const Vec3 lo    = box.getLowCorner();
  const Vec3 delta = box.getHighCorner() - lo;

  std::mt19937_64                   rng(2024);
  std::uniform_real_distribution<T> unit(T(0), T(1));
  std::vector<Vec3>                 points;

  for (std::size_t i = 0; i < 4; i++) {
    points.emplace_back(lo - T(0.5) * delta + T(2) * delta * Vec3(unit(rng), unit(rng), unit(rng)));
  }

  // closestPoint() returns the signed distance (negative inside), the face the closest point lies on (its
  // index in the file), the closest point itself, and the pseudonormal of the face, edge or vertex there.
  // gradient() is the unit direction of steepest increase of the distance: away from the closest point
  // outside, toward it inside. So a point moves onto the surface in one step, p - s * gradient(p), with no
  // iteration.
  for (const Vec3& p : points) {
    const EBGeometry::ClosestPoint<T> closest  = sdf.closestPoint(p);
    const Vec3                        gradient = sdf.gradient(p);
    const Vec3                        onto     = p - closest.signedDistance * gradient;

    std::cout << "\nPoint " << p << '\n';
    std::cout << "  signed distance   " << closest.signedDistance << '\n';
    std::cout << "  closest point     " << closest.point << ", on face " << closest.faceId << '\n';
    std::cout << "  gradient          " << gradient << '\n';
    std::cout << "  p - s * gradient  " << onto << " (" << (onto - closest.point).length()
              << " from the closest point)\n";

    // The finite-difference normal() of any function, for comparison: six distance queries, and a step that
    // must be smaller than the mesh's features but larger than the rounding of the distance.
    std::cout << "  normal(), step 1e-3 of the box: " << EBGeometry::normal(sdf, p, T(1e-3) * delta.length()) << '\n';
  }

  // On the surface the direction p - c is zero, and the gradient is the pseudonormal of the feature the point
  // is on (a face's normal, or an angle-weighted average at an edge or a vertex): defined everywhere.
  const Vec3 surfacePoint = sdf.closestPoint(points[0]).point;

  std::cout << "\nOn the surface, at " << surfacePoint << ": gradient " << sdf.gradient(surfacePoint) << '\n';

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // freeze() the pool, mirror it into GPU::memoryResource() (device memory in a CUDA or HIP build), and give
  // the kernel a view of the SDF on the mirror. In a plain C++ build these lines run as a host loop.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "\nOn the device: skipped, no GPU is visible\n";
  }
  else {
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

    std::vector<Vec3> onDevice(points.size());

    EBGeometry::GPU::transform(Gradient{sdf.rebasedView(devicePool)}, points, onDevice);

    std::cout << "\nGradients through GPU::transform (backend: " << EBGeometry::GPU::backendName << "):\n";

    for (const Vec3& g : onDevice) {
      std::cout << "  " << g << '\n';
    }
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // Render::view() opens a window on the mesh (it needs SDL2: make VIEWER=SDL2). Given the pool, it renders
  // on the device in a GPU build. --no-view, or no display, skips it.
  EBGeometry::Render::view(sdf, pool, argc, argv);

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n'
            << "The default mesh is in the common-3d-test-models submodule: git submodule update --init\n";

  return 1;
}
