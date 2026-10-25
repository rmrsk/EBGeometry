<!-- REMOVE this banner once downstream users have migrated (target: ~2026-Q4). -->
> [!IMPORTANT]
> **Git history rewritten (July 2026).** EBGeometry's history was rewritten to
> remove large binary files from old commits, so **every commit hash before this
> date has changed**. If you pin EBGeometry as a **submodule** at an older commit,
> follow the [migration guide](https://github.com/rmrsk/EBGeometry-legacy/releases/tag/pre-history-rewrite)
> (old&rarr;new commit map + how to repoint). If you just clone/build the library
> or vendor `EBGeometry.hpp`, no action is needed.

# EBGeometry

**Fast, portable geometry queries in a single C++ header.**

[![Documentation](https://github.com/rmrsk/EBGeometry/actions/workflows/docs.yml/badge.svg)](https://rmrsk.github.io/EBGeometry/)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Header-only](https://img.shields.io/badge/header--only-yes-brightgreen.svg)](EBGeometry.hpp)

EBGeometry turns meshes, analytic shapes, point clouds and compositions of them into geometry you
can ask questions: how far away is it, which surface is closest, what lies nearest. It answers them
fast on CPUs and on GPUs, from the same code, with no dependencies: include one header and compile
with C++17.

<!-- TODO once the renderer has landed: replace these two images with one rendered CSG scene. -->
<p align="center">
   <img src="Docs/Sphinx/source/_static/example_dcel.png" width="300" alt="Signed distance field from Armadillo geometry"/>
   <img src="Docs/Sphinx/source/_static/example_spheres.png" width="300" alt="Packed bed geometry"/>
</p>

## Why EBGeometry

* **Any geometry.** A triangle mesh read from STL, PLY, OBJ or VTK, an analytic shape, a point
  cloud, or any composition of them is queried the same way. Drill a hole through a mesh with an
  analytic cylinder, blend two shapes smoothly, or union ten thousand parts.
* **Any hardware.** The same query runs on the host (SIMD from SSE to AVX-512) and on NVIDIA and AMD
  GPUs (CUDA and HIP). Build a structure once, mirror its memory to the device, and evaluate it
  there.
* **Fast by construction.** Packed bounding volume hierarchies with any branching factor and a choice
  of builds (SAH, clustered SAH, Morton and Hilbert curves), SIMD triangle kernels, compiled CSG, and
  BVH-accelerated unions of many objects.
* **Measured on every change.** Every pull request comes with a performance comparison against its
  base, on the host and on a GPU, and every result is cross-checked. Unit tests check against brute
  force and closed forms, in both precisions, under sanitizers.
* **No dependencies.** One header, C++17, `float` or `double`. Nothing to build or link.

## What it does

**Geometry in**

* Triangle meshes, as a half-edge mesh or as SIMD triangle blocks, read from STL, PLY, OBJ and VTK.
* Analytic shapes (spheres, boxes, cylinders, tori, planes and more) and your own.
* Point clouds.

**Composition**

* Constructive solid geometry: unions, intersections, differences, smooth blends, offsets,
  repetitions, and translations, rotations and scalings.
* A scene compiled into a *tape* that runs on a GPU, or a part fused into compiled code.
* BVH-accelerated unions of thousands of objects.

**Queries**

* Signed distance (negative inside, positive outside), normals, and bounding boxes.
* The primitive that decides the value at a point, with the point in its own frame.
* Closest points and k nearest neighbours in point clouds.

## In code

```cpp
#include <EBGeometry.hpp>

using namespace EBGeometry;
using T = float;

// A query, as a trivially copyable functor: the same code runs on the host and in a GPU kernel.
struct Distance
{
  Tape<T> m_scene;

  EBGEOMETRY_HOST_DEVICE T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_scene.signedDistance(a_point);
  }
};

int
main()
{
  Pool pool(hostMemoryResource());

  // A triangle mesh as a signed distance field: SIMD triangle blocks under a BVH.
  const auto mesh = Parser::readIntoTriMeshSDF<T>("armadillo.obj", pool);

  // A scene: the mesh drilled through by a cylinder, next to a smooth blob, compiled into a tape.
  const auto drill = InfiniteCylinderSDF<T>(mesh.computeBoundingVolume().getCentroid(), T(10), 0);
  const auto blob  = SmoothUnion(SphereSDF<T>(Vec3T<T>(2, 0, 0), 0.5), SphereSDF<T>(Vec3T<T>(2.6, 0, 0), 0.4), 0.2);

  const Tape<T> scene(pool, Union(Difference(Expr<T>(mesh), drill), blob));

  // Ask it anything, anywhere.
  const Vec3T<T>        p(0.1, 0.2, 0.3);
  const T               d    = scene.signedDistance(p);    // negative inside, positive outside
  const PrimitiveRef<T> leaf = scene.closestPrimitive(p);  // which surface decides the value

  // The same query on a GPU: mirror the pool to the device and evaluate a million points in a kernel.
  std::vector<Vec3T<T>> points(1000000, p);
  std::vector<T>        distances(points.size());

  pool.freeze();

  const Pool device = Pool::mirror(pool, GPU::memoryResource());

  GPU::transform(Distance{scene.rebasedView(device)}, points, distances);
}
```

Compiled as plain C++, `GPU::transform()` runs the same functor in a host loop; compiled with nvcc or
hipcc, it runs it in a kernel. The [examples](https://rmrsk.github.io/EBGeometry/Examples.html) walk
through each part of this, one topic at a time.

## How it compares

EBGeometry overlaps with several excellent libraries: TriangleMeshDistance and fcpw for mesh
distances, libfive for functional solid modelling, nanoflann for neighbour search, Embree and OptiX
for ray tracing, and CGAL for exact computational geometry. What it adds is the combination: every
kind of geometry, composed freely, queried the same way on CPUs and GPUs, from one header. The
[comparison](https://rmrsk.github.io/EBGeometry/Comparison.html) says what each library is built
for and when it is the better choice, and the companion
[EBGeometry-benchmarks](https://github.com/rmrsk/EBGeometry-benchmarks) repository measures
EBGeometry beside several of them on shared tasks.

EBGeometry was written for embedded-boundary codes such as [AMReX](https://amrex-codes.github.io)
and [Chombo](https://commons.lbl.gov/display/chombo); [`Integrations/`](Integrations/README.md)
shows how to couple it to them. It is general-purpose.

## Get started

Clone the repository, including the submodule that provides the example mesh files:

```bash
git clone --recurse-submodules https://github.com/rmrsk/EBGeometry.git
cd EBGeometry
```

Build and run the `MeshSDF` example, which reads a triangulated surface mesh and evaluates it as a
signed distance function:

```bash
cd Examples/MeshSDF
g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o MeshSDF.ex
./MeshSDF.ex
```

This loads `armadillo.obj` from the `common-3d-test-models` submodule by default; pass a path to run
it on a different STL/PLY/OBJ/VTK file instead. Every example under `Examples/` can also be built with
[CMake](https://cmake.org/) or GNU make (see [Examples/MeshSDF/README.md](Examples/MeshSDF/README.md)),
and with `make GPU=CUDA` or `make GPU=HIP` for a GPU.

To build and run the whole test suite and every example, use the
[CMake presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html):

```bash
cmake --preset debug                # Debug, assertions on, tests and examples
cmake --build --preset debug --parallel $(nproc)

ctest --preset debug                # unit tests (Catch2), about 15 seconds
ctest --preset examples --parallel $(nproc)   # every example, several minutes in Debug
```

See [Contributing and testing](https://rmrsk.github.io/EBGeometry/Contributing.html) for the full
preset table, the sanitizer and release builds, and `Scripts/run-all-checks.sh`, which runs everything
the continuous integration pipeline checks in one local command.

## Documentation

* [User guide (HTML)](https://rmrsk.github.io/EBGeometry/): concepts, building, examples, and
  testing.
* [User guide (PDF)](https://rmrsk.github.io/EBGeometry/ebgeometry.pdf): the same, as one file.
* [API reference (Doxygen)](https://rmrsk.github.io/EBGeometry/doxygen/html/index.html): every class
  and function.

The documentation is rebuilt on every pull request and evolves with the code.

## Get help

* [GitHub issues](https://github.com/rmrsk/EBGeometry/issues): bug reports and feature requests.
* [GitHub discussions](https://github.com/rmrsk/EBGeometry/discussions): usage questions and
  everything else.

## Contribute

Contributions are welcome. Reach out to see whether a feature is already planned, or fork the
repository and open a pull request. See
[Contributing and testing](https://rmrsk.github.io/EBGeometry/Contributing.html) for how to build and
test locally, what continuous integration checks, the performance comparison every pull request
carries, and the code conventions. For GPU work, start from [PORTING.md](PORTING.md).

## How EBGeometry is developed

EBGeometry is human-designed, with AI-assisted implementation.

* **2022 to mid-2026:** designed and written by hand.
* **Since July 2026** (the overhaul in [#88](https://github.com/rmrsk/EBGeometry/pull/88)): the maintainer designs and decides: the architecture, the
  APIs, the algorithms, the priorities, and every non-obvious choice. [Claude Code](https://claude.com/claude-code)
  implements, tests, documents and measures, steered by the rules in [`CLAUDE.md`](CLAUDE.md), and
  brings every open decision back to the maintainer.
* **Every change,** whoever wrote it, passes the same gates: unit tests against independent
  references, continuous integration, a performance comparison against its base, an AI review, and a
  human review. The maintainer reviews and merges every pull request.

Contributions written with an AI coding agent are welcome on the same terms.
