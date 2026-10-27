This folder contains examples of using EBGeometry on its own (header-only, C++17, no
third-party dependencies):

The examples teach; the unit tests in `Tests/` verify. An example shows the calls you would write,
in the order you would write them, and prints results you can read. It does not check its own
answers. Read them in this order:

1. `Shapes`: the built-in analytic shapes: constructing them, their signed distance, bounding box and distance quality.
2. `BasicCSG`: the classic CSG solid, with `Union`, `Intersection` and `Difference`, compiled into a `Tape`.
3. `SmoothCSG`: smooth unions, intersections and differences, and the two blend kinds.
4. `Transforms`: one object moved, turned, scaled, mirrored, stretched and repeated.
5. `ValueOperations`: `Offset`, `Annular`, `Blur` and `Mollify`.
6. `FusedCSG`: the classic solid as a fused composition (`Fused::`), against the tape.
7. `MeshSDF`: reading a surface mesh into a signed distance function; `FlatMeshSDF`, `MeshSDF` and `TriMeshSDF`, and which to pick.
8. `NestedBVH`: a mesh placed many times: in a `Tape` alongside shapes, and in a `BVHUnion` of meshes.
9. `ManyObjects`: a `BVHUnion` over many spheres and boxes, and how much it saves over a plain loop; device timing of the query alone.
10. `PointClouds`: closest points and the k-nearest-neighbor graph of a point cloud with `PointCloudBVH`, and the same with `PointCloudHashGrid`.
11. `BVHTraversal`: walking a BVH with a search of your own (`PackedBVH::traverse()`): the nearest object to a point, the first one a ray hits, whether a segment is blocked.
12. `RayTracing`: casting rays at a triangle mesh (`firstHit()`, `intersects()`): where a ray or a particle's step first crosses the surface, and the same walk of the BVH written by hand.
13. `OctreeBoundingVolume`: estimating a bounding box for a function with no closed-form one.
14. `CustomShape`: a shape type of your own in a `Tape`, in managed memory, called from a kernel written out by hand.
15. `HostTuning`: the BVH branching factor and SIMD width tuned to the host's SIMD flags, against the portable defaults.
16. `BuildBVH`: comparing the BVH construction methods by build time.

Every example that builds a shape (`Shapes`, the five CSG examples, `MeshSDF`, `NestedBVH`, `ManyObjects`,
`CustomShape`) ends by opening a window on it with `EBGeometry::Render::view()`, when built with the viewer
(`make VIEWER=SDL2`, or CMake with `-DEBGEOMETRY_VIEWER=SDL2`; it needs SDL2). Otherwise, with `--no-view`, or
without a display, `view()` prints why and returns, so the examples still run unattended (ctest passes
`--no-view`).

Every example that runs on a device has the same four parts, in this order: the query functor ("the
code that runs on either side"), the construction of the geometry, its use on the host through plain
calls, and the same query on a GPU.

Every example except `BuildBVH`, `HostTuning`, `SmoothCSG`, `Transforms` and `ValueOperations` also builds for a GPU: `make GPU=HIP` (or `CUDA`), or
CMake with `-DEBGEOMETRY_EXAMPLE_GPU=HIP` (or `CUDA`), compiles the same source for a device, and its last
section then runs its query in a kernel through `EBGeometry::GPU::transform`. In a plain C++ build that
section runs in a host loop; a GPU build without a visible GPU skips it.

Examples that couple EBGeometry to a third-party application code (AMReX, Chombo) live under
[`Integrations/`](../Integrations/README.md) at the repository root instead.

`data/` holds the small mesh (`dodecahedron.stl`) that several examples read by default. The larger
mesh files used by `MeshSDF` come from the [common-3d-test-models](https://github.com/alecjacobson/common-3d-test-models)
git submodule (`common-3d-test-models/`) at the repository root. Clone the repository with `--recurse-submodules` (or run
`git submodule update --init --recursive`); see the "Building and using" documentation for details.
