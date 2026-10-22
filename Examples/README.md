This folder contains examples of using EBGeometry on its own (header-only, C++17, no
third-party dependencies):

The examples teach; the unit tests in `Tests/` verify. An example shows the calls you would write,
in the order you would write them, and prints results you can read. It does not check its own
answers. Read them in this order:

1. `Shapes`: the built-in analytic shapes: constructing them, their signed distance, bounding box and distance quality.
2. `CSG`: composing shapes with unions, intersections, differences, transforms and smooth blends, compiled into a `Tape`.
3. `MeshSDF`: reading a surface mesh into a signed distance function; `FlatMeshSDF`, `MeshSDF` and `TriMeshSDF`, and which to pick; then a look at it in the viewer (`Render::view()`).
4. `NestedBVH`: a mesh placed many times: in a `Tape` alongside shapes, and in a `BVHUnion` of meshes.
5. `ManyObjects`: a `BVHUnion` over many spheres and boxes, and how much it saves over a plain loop; device timing of the query alone.
6. `PointClouds`: closest points and the k-nearest-neighbor graph of a point cloud with `PointCloudBVH`, and the same with `PointCloudHashGrid`.
7. `OctreeBoundingVolume`: estimating a bounding box for a function with no closed-form one.
8. `CustomShape`: a shape type of your own in a `Tape`, in managed memory, called from a kernel written out by hand.
9. `HostTuning`: the BVH branching factor and SIMD width tuned to the host's SIMD flags, against the portable defaults.
10. `BuildBVH`: comparing the BVH construction methods by build time.

Every example that builds a shape (`Shapes`, `CSG`, `MeshSDF`, `NestedBVH`, `ManyObjects`, `CustomShape`)
ends by opening a window on it with `EBGeometry::Render::view()`, when built with the viewer (`make
VIEWER=SDL2`, or CMake with `-DEBGEOMETRY_VIEWER=SDL2`; it needs SDL2). Otherwise, with `--no-view`, or
without a display, `view()` prints why and returns, so the examples still run unattended (ctest passes
`--no-view`).

Every example that runs on a device has the same four parts, in this order: the query functor ("the
code that runs on either side"), the construction of the geometry, its use on the host through plain
calls, and the same query on a GPU.

Every example except `BuildBVH` and `HostTuning` also builds for a GPU: `make GPU=HIP` (or `CUDA`), or
CMake with `-DEBGEOMETRY_EXAMPLE_GPU=HIP` (or `CUDA`), compiles the same source for a device, and its last
section then runs its query in a kernel through `EBGeometry::GPU::transform`. In a plain C++ build that
section runs in a host loop; a GPU build without a visible GPU skips it.

Examples that couple EBGeometry to a third-party application code (AMReX, Chombo) live under
[`Integrations/`](../Integrations/README.md) at the repository root instead.

`data/` holds the small mesh (`dodecahedron.stl`) that several examples read by default. The larger
mesh files used by `MeshSDF` come from the [common-3d-test-models](https://github.com/alecjacobson/common-3d-test-models)
git submodule (`common-3d-test-models/`) at the repository root. Clone the repository with `--recurse-submodules` (or run
`git submodule update --init --recursive`); see the "Building and using" documentation for details.
