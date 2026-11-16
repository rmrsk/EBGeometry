# EBGeometry

**Fast, portable geometry queries in a single C++ header.**

[![CI](https://github.com/rmrsk/EBGeometry/actions/workflows/CI.yml/badge.svg)](https://github.com/rmrsk/EBGeometry/actions/workflows/CI.yml)
[![Documentation](https://github.com/rmrsk/EBGeometry/actions/workflows/docs.yml/badge.svg)](https://rmrsk.github.io/EBGeometry/)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Header-only](https://img.shields.io/badge/header--only-yes-brightgreen.svg)](EBGeometry.hpp)
[![Dependencies](https://img.shields.io/badge/dependencies-none-brightgreen.svg)](EBGeometry.hpp)
[![SIMD](https://img.shields.io/badge/SIMD-SSE4.1%20%7C%20AVX%20%7C%20AVX--512-blue.svg)](https://rmrsk.github.io/EBGeometry/ConfigurationOptions.html#simd-acceleration)
[![CUDA](https://img.shields.io/badge/GPU-CUDA-76B900.svg?logo=nvidia&logoColor=white)](https://rmrsk.github.io/EBGeometry/RunningOnGPU.html)
[![HIP](https://img.shields.io/badge/GPU-HIP-ED1C24.svg?logo=amd&logoColor=white)](https://rmrsk.github.io/EBGeometry/RunningOnGPU.html)

EBGeometry turns meshes, analytic shapes, point clouds and compositions of them into geometry you
can ask questions: how far away is it, which surface is closest, where a ray hits it, what lies
nearest. It answers them fast on CPUs and on GPUs, from the same code, with no dependencies: include
one header and compile with C++17.

<p align="center">
   <img src="Docs/Sphinx/source/_static/f18_assembly.gif" width="800" alt="A fighter jet assembled from its parts, then cut open to show its distance field, shown in X-ray, and as the ray tracer's cost per pixel"/>
   <br/>
   <sub>A 61-part, 310,000-triangle assembly in one BVH-accelerated union. Model adapted from the
   Boeing EA-18G Growler by Leo Zeng, from
   <a href="https://grabcad.com/library/boeing-ea-18g-growler-1">GrabCAD</a>.</sub>
</p>

## Why EBGeometry

* **Any geometry.** A triangle mesh read from STL, PLY, OBJ or VTK, an analytic shape, a point
  cloud, or any composition of them is queried the same way. Drill a hole through a mesh with an
  analytic cylinder, blend two shapes smoothly, or union ten thousand parts.
* **Several materials.** An object can be made of regions, each with its material, and a solver gets
  the implicit function of each region, or of each domain it computes in.
* **Models as text.** A model, with its regions, materials, boundary conditions and the domains a solver
  computes in, can be written in the `.ebg` language and read at run time, and checked and looked at with
  `ebgstudio`, without compiling anything.
* **Any hardware.** The same query runs on the host (SIMD from SSE to AVX-512) and on NVIDIA and AMD
  GPUs (CUDA and HIP). Build a structure once, mirror its memory to the device, and evaluate it
  there.
* **Fast by construction.** Packed bounding volume hierarchies with any branching factor and a choice
  of builds (SAH, clustered SAH, Morton and Hilbert curves), SIMD triangle kernels, compiled CSG, and
  BVH-accelerated unions of many objects.
* **Measured on every change.** Every pull request comes with a performance comparison against its
  base, on the host and on a GPU, which also checks that the answers did not change. Unit tests check
  against brute force and closed forms, in both precisions, under sanitizers.
* **No dependencies.** One header, C++17, `float` or `double`. Nothing to build or link.

<p align="center">
   <img src="Docs/Sphinx/source/_static/csg_dragon.gif" width="800" alt="CSG on a mesh: the dragon revealed by an intersection, joined to a table by a union that turns smooth, then cut open by a difference"/>
   <br/>
   <sub>A 250,000-triangle mesh composed with analytic shapes: an intersection, a union, a smooth
   union and a difference. Dragon: Stanford 3D Scanning Repository, from
   <a href="https://github.com/alecjacobson/common-3d-test-models">common-3d-test-models</a>.</sub>
</p>

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

**Materials**

* Regions with materials in one object, the surfaces between them kept, each with the materials on its
  two sides.
* An implicit function per region and per domain (the regions a solver computes in), and boundary
  conditions tagged on the surfaces.

**Models as text**

* The `.ebg` language: geometry, regions, materials, labels, domains, surface groups and couplings in plain
  files, with params a program or the command line can change.
* `ebgstudio`: checks a model line by line, says whether its domains are complete, and opens it in a window
  that reloads on every save, where surface groups are made by clicking surfaces.

**Queries**

* Signed distance (negative inside, positive outside), normals, and bounding boxes.
* The primitive that decides the value at a point, with the point in its own frame.
* Ray and segment hits (`firstHit()`, `intersects()`): exact for meshes, closed-form shapes and their
  unions, intersections, differences and affine maps (fused, BVH unions and tapes), sphere traced for
  everything else.
* Closest points and k nearest neighbors in point clouds.
* A renderer for any shape, on the host or a GPU, with an optional interactive window (SDL2).

## In code

```cpp
#include <iostream>
#include <vector>

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

  // A triangle mesh as a signed distance field: SIMD triangle blocks under a BVH. (The armadillo is in
  // the common-3d-test-models submodule, common-3d-test-models/data/armadillo.obj.)
  const auto mesh = Parser::readIntoTriMeshSDF<T>("armadillo.obj", pool);

  // Place the other shapes from the mesh's bounding box: the armadillo is about 130 units across.
  const auto     box    = mesh.computeBoundingVolume();
  const Vec3T<T> center = box.getCentroid();
  const Vec3T<T> beside(box.getHighCorner()[0] + T(30), center[1], center[2]);

  // A scene: the mesh drilled through along x (axis 0) by a cylinder of radius 10, and beside it a
  // smooth blob of two spheres. Compiled into a tape.
  const auto drill = InfiniteCylinderSDF<T>(center, T(10), 0);
  const auto big   = SphereSDF<T>(beside, T(15));
  const auto small = SphereSDF<T>(beside + Vec3T<T>(T(20), T(0), T(0)), T(10));
  const auto blob  = SmoothUnion(big, small, T(5));

  const Tape<T> scene(pool, Union(Difference(Expr<T>(mesh), drill), blob));

  // Ask it anything, anywhere: here, at a point in the blob.
  const Vec3T<T>   p    = beside;
  const T          d    = scene.signedDistance(p);   // negative inside, positive outside
  const LeafRef<T> leaf = scene.closestLeaf(p);      // which surface decides the value

  std::cout << "distance " << d << ", decided by leaf " << leaf.m_leaf << '\n';

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

## As a model file

The same kind of scene, written in the `.ebg` language: a cable in a box of air, a copper core in a plastic jacket,
and a flow past it.

```text
ebg 0

param r_core = 0.5

core   = cylinder((0, 0, -3), (0, 0, 3), radius = r_core)
jacket = tube((0, 0, -3), (0, 0, 3), inner_radius = r_core, outer_radius = 0.8)
air    = box((-3, -2, -3), (5, 2, 3)).label(xmin = "inlet", xmax = "outlet")

object overlay {                        # each region on top of the ones before it
  region air    = air material air
  region jacket = jacket material PTFE
  region core   = core material copper
}

material air    { density = 1.2, viscosity = 1.8e-5 }
material PTFE   { permittivity = 2.1 }
material copper { conductivity = 5.8e7 }

domain cfd {                            # what a flow solver computes on
  subdomain fluid = air
  group inlet  = label("inlet") { velocity = (2, 0, 0) }
  group outlet = label("outlet") { pressure = 0 }
  group walls  = remaining() { kind = "no_slip" }
}
```

```bash
cd Studio && make                       # builds ebgstudio
./ebgstudio check cable.ebg --domain cfd
./ebgstudio cable.ebg                   # the window; it reloads the model on every save
```

A program reads it with `Language::read()`, compiles its object into a tape, builds the `cfd` domain for it, and reads
the properties into tables of its own that run on a GPU beside the geometry. The
[language guide](https://rmrsk.github.io/EBGeometry/Language.html) and the
[ebgstudio tutorial](https://rmrsk.github.io/EBGeometry/StudioCoolingBlock.html) start there.

## How it compares

EBGeometry overlaps with several excellent libraries:
[TriangleMeshDistance](https://github.com/InteractiveComputerGraphics/TriangleMeshDistance) and
[fcpw](https://github.com/rohan-sawhney/fcpw) for mesh distances, [libfive](https://libfive.com) for
functional solid modeling, [nanoflann](https://github.com/jlblancoc/nanoflann) for neighbor search,
[Embree](https://www.embree.org) and [OptiX](https://developer.nvidia.com/rtx/ray-tracing/optix) for
ray tracing, and [CGAL](https://www.cgal.org) for exact computational geometry. What it adds is the
combination: every kind of geometry, composed freely, queried the same way on CPUs and GPUs, from one
header. [EBGeometry-benchmarks](https://github.com/rmrsk/EBGeometry-benchmarks) measures it beside
several of them.

EBGeometry was written for embedded-boundary codes; [`Integrations/`](Integrations/README.md) shows how to
couple it to one. It is general-purpose.

## Get started

Clone the repository, including the submodule that provides the example mesh files:

```bash
git clone --recurse-submodules https://github.com/rmrsk/EBGeometry.git
cd EBGeometry
```

Build and run the first example, which constructs every analytic shape and prints its signed distance
at a few points:

```bash
cd Examples/Shapes
g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o Shapes.ex
./Shapes.ex
```

Every example under `Examples/` can also be built with [CMake](https://cmake.org/) or GNU make (see
[Examples/README.md](Examples/README.md) for the reading order), and with `make GPU=CUDA` or
`make GPU=HIP` for a GPU. The [Quickstart](https://rmrsk.github.io/EBGeometry/Quickstart.html) writes a
first program of your own.

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
carries, and the code conventions.

## How EBGeometry is developed

EBGeometry is human-designed, with AI-assisted implementation.

* **2022 to mid-2026:** designed and written by hand.
* **Since July 2026:** the maintainer designs and decides: the architecture, the APIs, the algorithms, the priorities, and
  every non-obvious choice. [Claude Code](https://claude.com/claude-code) implements, tests, documents
  and measures, steered by the rules in [`CLAUDE.md`](CLAUDE.md), and brings every open decision back
  to the maintainer.
* **Every change,** whoever wrote it, passes the same gates: unit tests against independent
  references, continuous integration, a performance comparison against its base, an AI review, and a
  human review. The maintainer reviews and merges every pull request.

Contributions written with an AI coding agent are welcome on the same terms.
