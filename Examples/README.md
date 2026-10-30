This folder contains examples of using EBGeometry on its own (header-only, C++17, no
third-party dependencies):

The examples teach; the unit tests in `Tests/` verify. An example shows the calls you would write,
in the order you would write them, and prints results you can read. It does not check its own
answers.

**Several examples read a mesh from the `common-3d-test-models` git submodule by default** (`MeshSDF`,
`MeshCSG`, `Repetition`, `RayTracing`, `Twist`, `Shell`, `Materials`, `DeviceTuning`): clone the repository with `--recurse-submodules`, or run
`git submodule update --init` in it.

The reading order:

1. `Shapes`: the built-in analytic shapes: constructing them, their signed distance, bounding box and distance quality.
2. `BasicCSG`: the classic CSG solid, with `Union`, `Intersection` and `Difference`, compiled into a `Tape`.
3. `SmoothCSG`: smooth unions, intersections and differences, and the two blend kinds.
4. `Transforms`: one object moved, turned, scaled, mirrored and stretched.
5. `Repetition`: many copies of a mesh (the happy Buddha) from one `FiniteRepetition`.
6. `ValueOperations`: `Offset`, `Annular`, `Blur` and `Mollify`.
7. `FusedCSG`: the classic solid as a fused composition (`Fused::`), against the tape.
8. `MeshSDF`: reading a surface mesh into a signed distance function; `FlatMeshSDF`, `MeshSDF` and `TriMeshSDF`, and which to pick.
9. `MeshCSG`: a mesh joined to analytic shapes: the dragon on a table, blended by a smooth union.
10. `Shell`: a thin shell around a mesh (Lucy, rotated upright) with `Annular`, cut open.
11. `Materials`: which part of an object a query found (`closestPrimitive()`, `firstHit()`), and a material table indexed by it.
12. `NestedBVH`: a mesh placed many times: in a `Tape` alongside shapes, and in a `BVHUnion` of meshes.
13. `ManyObjects`: a `BVHUnion` over many spheres and boxes, and how much it saves over a plain loop; device timing of the query alone.
14. `PackedSpheres`: thousands of spheres at random places in a cylinder, joined by a smooth BVH union.
15. `PointClouds`: closest points and the k-nearest-neighbor graph of a point cloud with `PointCloudBVH`, and the same with `PointCloudHashGrid`.
16. `BVHTraversal`: walking a BVH with a search of your own (`PackedBVH::traverse()`): the nearest object to a point, the first one a ray hits, whether a segment is blocked.
17. `RayTracing`: casting rays at a triangle mesh (`firstHit()`, `intersects()`): where a ray or a particle's step first crosses the surface, and the same walk of the BVH written by hand.
18. `OctreeBoundingVolume`: estimating a bounding box for a function with no closed-form one.
19. `CustomShape`: a shape type of your own in a `Tape`, in managed memory, called from a kernel written out by hand.
20. `Twist`: an operation of your own, a twist about the y axis, applied to a mesh.
21. `HostTuning`: the BVH branching factor and SIMD width tuned to the host's SIMD flags, against the portable defaults.
22. `DeviceTuning`: `GPU::transform()`'s block size and register budget, timed on a sample of the queries, the fastest kept.
23. `BuildBVH`: comparing the BVH construction methods by build time.

Every example that builds a shape (`Shapes`, `BasicCSG`, `SmoothCSG`, `Transforms`, `Repetition`,
`ValueOperations`, `FusedCSG`, `MeshSDF`, `MeshCSG`, `Shell`, `Materials`, `NestedBVH`,
`ManyObjects`, `PackedSpheres`, `CustomShape`, `Twist`) ends by opening a window on it with `EBGeometry::Render::view()`, when built with the viewer
(`make VIEWER=SDL2`, or CMake with `-DEBGEOMETRY_VIEWER=SDL2`; it needs SDL2). Otherwise, with `--no-view`, or
without a display, `view()` prints why and returns, so the examples still run unattended (ctest passes
`--no-view`).

Every example that runs on a device has the same four parts, in this order: the query functor ("the
code that runs on either side"), the construction of the geometry, its use on the host through plain
calls, and the same query on a GPU.

Every example except `BuildBVH` and `HostTuning` also builds for a GPU: `make GPU=HIP` (or `CUDA`), or
CMake with `-DEBGEOMETRY_EXAMPLE_GPU=HIP` (or `CUDA`), compiles the same source for a device. Its window
then renders on the device, and an example with a last section of queries runs them in a kernel through
`EBGeometry::GPU::transform`. In a plain C++ build that section runs in a host loop; a GPU build without a
visible GPU skips it and renders on the host.

Examples that couple EBGeometry to a third-party application code live under
[`Integrations/`](../Integrations/README.md) at the repository root instead.

`data/` holds the small mesh (`dodecahedron.stl`) that `NestedBVH` and `HostTuning` read by default. The
larger mesh files used by the examples listed at the top come from the [common-3d-test-models](https://github.com/alecjacobson/common-3d-test-models)
git submodule (`common-3d-test-models/`) at the repository root. Clone the repository with `--recurse-submodules` (or run
`git submodule update --init --recursive`); see the "Building and using" documentation for details.
