// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Reading a surface mesh into a signed distance function, in each of the three representations
// EBGeometry offers, and evaluating it on the host and, through the same query, on a GPU. The timings
// show why the BVH-backed representations exist. At the end it opens a window on the mesh, if built
// with the viewer. See README.md.

#include <chrono>
#include <cstddef>
#include <iostream>
#include <optional>
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

// The BVH branching factor (K) and triangle-group width (W) that the readers use by default.
constexpr std::size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();
constexpr std::size_t W = EBGeometry::TriangleSoA::DefaultWidth<T>();

using TriMeshSDF = EBGeometry::TriMeshSDF<T, K, W>;

// The number of random points the representations are timed over. FlatMeshSDF visits every face for
// every point, so a few hundred points already take minutes on a large mesh in an unoptimised build.
constexpr std::size_t numPoints = 100;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE
// call operator maps one point to one result. It holds a mesh SDF by value. A MeshSDF owns no memory:
// it is a descriptor of the mesh and BVH in a Pool, so a copy is cheap and a kernel can receive it by
// value once it describes a pool the device can read (see the last section of main()).
struct Distance
{
  TriMeshSDF m_sdf;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return m_sdf.signedDistance(a_point);
  }
};

// Microseconds per point for evaluating an SDF at every point.
template <class SDF>
T
timePerPoint(const SDF& a_sdf, const std::vector<Vec3>& a_points, std::vector<T>& a_distances)
{
  const auto start = std::chrono::steady_clock::now();

  for (std::size_t i = 0; i < a_points.size(); i++) {
    a_distances[i] = a_sdf.signedDistance(a_points[i]);
  }

  const std::chrono::duration<T, std::micro> elapsed = std::chrono::steady_clock::now() - start;

  return elapsed.count() / T(a_points.size());
}

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
  std::string file = "../../common-3d-test-models/data/armadillo.obj";

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]).rfind("--", 0) != 0) {
      file = argv[i];

      break;
    }
  }

  std::cout << "Reading '" << file << "'\n";

  // Everything the SDFs refer to (vertices, faces, BVH nodes) lives in one Pool. The SDFs are
  // descriptors into it, so the pool must outlive them and every copy of them. Keeping it in main()'s
  // scope does.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  // Three representations of the same surface:
  //   FlatMeshSDF: the half-edge mesh with no acceleration; every query visits every face.
  //   MeshSDF:     the half-edge mesh under a BVH; any planar convex polygon faces.
  //   TriMeshSDF:  triangles (polygons are fan-triangulated) grouped for SIMD under a BVH; usually the
  //                fastest, and the one to pick unless you need the polygon faces themselves.
  // Each reader parses the file and builds its own structure. In an application you would pick one.
  const auto flatSDF = EBGeometry::Parser::readIntoFlatMeshSDF<T>(file, pool);
  const auto meshSDF = EBGeometry::Parser::readIntoMeshSDF<T>(file, pool);
  const auto triSDF  = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool, EBGeometry::BVH::Construction::SAH);

  const EBGeometry::BoundingVolumes::AABBT<T> box = triSDF.computeBoundingVolume();

  std::cout << "  Faces        : " << meshSDF.getMesh().numFaces() << '\n';
  std::cout << "  Bounding box : " << box.getLowCorner() << " to " << box.getHighCorner() << "\n\n";

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // Random points in a box three times the size of the mesh's, with the mesh at its center: most lie
  // outside, some inside. A fixed seed gives the same points on every run.
  const Vec3 lo    = box.getLowCorner();
  const Vec3 delta = box.getHighCorner() - lo;

  std::mt19937_64                   rng(12345);
  std::uniform_real_distribution<T> unit(T(0), T(1));
  std::vector<Vec3>                 points;

  for (std::size_t i = 0; i < numPoints; i++) {
    points.emplace_back(lo - delta + T(3) * delta * Vec3(unit(rng), unit(rng), unit(rng)));
  }

  // signedDistance(point) is negative inside the mesh and positive outside. The three representations
  // give the same values; they differ only in how many faces they visit to find them.
  std::vector<T> flat(numPoints);
  std::vector<T> mesh(numPoints);
  std::vector<T> tri(numPoints);

  const T flatTime = timePerPoint(flatSDF, points, flat);
  const T meshTime = timePerPoint(meshSDF, points, mesh);
  const T triTime  = timePerPoint(triSDF, points, tri);

  std::cout << "Time per query over " << numPoints << " points:\n";
  std::cout << "  FlatMeshSDF : " << flatTime << " us\n";
  std::cout << "  MeshSDF     : " << meshTime << " us (" << flatTime / meshTime << "x faster)\n";
  std::cout << "  TriMeshSDF  : " << triTime << " us (" << flatTime / triTime << "x faster)\n\n";

  std::cout << "The first few points:\n";

  for (std::size_t i = 0; i < 4; i++) {
    std::cout << "  " << points[i] << "   FlatMeshSDF " << flat[i] << "   MeshSDF " << mesh[i] << "   TriMeshSDF "
              << tri[i] << '\n';
  }

  // getClosestFace() also says which face is nearest: its index in the order the file lists the faces,
  // which is how to look up per-face data of your own.
  const EBGeometry::ClosestFace<T> nearest = triSDF.getClosestFace(points[0]);

  std::cout << "  The face nearest the first point is face " << nearest.faceId << ", at distance "
            << nearest.signedDistance << "\n";

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // A kernel cannot read host memory, so the pool goes to the device first:
  //   1. freeze() the pool, so that nothing can be added to it and its block stays where it is;
  //   2. Pool::mirror() copies the whole block, every mesh and BVH in it, in one piece into
  //      GPU::memoryResource() (device memory in a CUDA or HIP build);
  //   3. rebasedView() gives a descriptor of the same SDF that resolves against the copy.
  // GPU::transform() then runs the query once per point in a kernel and writes the results into a host
  // vector. In a plain C++ build, memoryResource() is host memory standing in for a device and transform()
  // is a host loop, so these lines also run there. The values agree with the host's to rounding, since
  // a device compiler may fuse a multiply and an add where the host compiler does not.
  // The device copy is kept until the end of main(), for the viewer below.
  std::optional<EBGeometry::Pool> devicePool;

  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "\nOn the device: skipped, no GPU is visible\n";
  }
  else {
    pool.freeze();

    devicePool.emplace(EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource()));

    const Distance query{triSDF.rebasedView(*devicePool)};

    std::vector<T> onDevice(points.size());

    EBGeometry::GPU::transform(query, points, onDevice);

    std::cout << "\nTriMeshSDF through GPU::transform (backend: " << EBGeometry::GPU::backendName << "):\n";

    for (std::size_t i = 0; i < 4; i++) {
      std::cout << "  " << points[i] << "   " << onDevice[i] << '\n';
    }
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // Render::view() opens a window on any shape: drag to orbit, scroll to zoom, X to cut it open with a
  // section plane, P to colour each face differently. A surface seen from inside the solid is drawn red,
  // which is what an inverted mesh (faces wound the wrong way) looks like. The window needs SDL2: build
  // with make VIEWER=SDL2 or cmake -DEBGEOMETRY_VIEWER=SDL2. Otherwise, with --no-view, or without a
  // display, view() prints why and returns. The camera starts fitted to the mesh's bounding box. In a GPU
  // build it renders on the device, through the same device view as the query above (G switches to the
  // CPU and back).
  if (EBGeometry::GPU::enabled && devicePool) {
    const auto camera = EBGeometry::Render::Camera<T>::fit(box);

    EBGeometry::Render::view(
      triSDF, triSDF.rebasedView(*devicePool), camera, EBGeometry::Render::Settings<T>(), argc, argv);
  }
  else {
    EBGeometry::Render::view(triSDF, argc, argv);
  }

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n'
            << "The default mesh is in the common-3d-test-models submodule: git submodule update --init\n";

  return 1;
}
