This folder contains examples of using EBGeometry on its own (header-only, C++17, no
third-party dependencies):

* `BuildBVH` For comparing BVH construction methods by build time: CentroidSplit, MidpointSplit, SAH, Morton, Nested and Hilbert through TreeBVH and through PackedBVH's direct constructors, plus the direct-only ClusterSAH.
* `CSGUnion` For composing objects of different types (a mesh placed several times, and spheres) with the expression builder and evaluating the result through a `Tape`.
* `CustomShape` For writing your own shape type, composing it into a `Tape` in managed memory, and evaluating it in a GPU kernel written out by hand.
* `ClosestPointBVH` For closest-point search over a point cloud using the turnkey `PointCloudBVH` class: build once from positions, then `closestPoint()` / `closestPoints()` for arbitrary query points, checked against brute force.
* `HostTuning` For choosing the BVH branching factor and SIMD width: the portable defaults against the values tuned to the host's SIMD flags, with timings.
* `MeshSDF` For reading a surface mesh and evaluating it with the DCEL/BVH signed-distance representations.
* `NearestNeighborBVH` For the k nearest neighbors of every point in a cloud (the k-NN graph) using `PointCloudBVH::allNearestNeighbors()`, checked against brute force.
* `NearestNeighborHashGrid` For the k-NN graph using the uniform-grid `PointCloudHashGrid` class (same interface as `PointCloudBVH`); faster to build on near-uniform clouds, checked against brute force.
* `NestedBVH` For a nested BVH: a BVH-accelerated union over several BVH-backed mesh SDFs.
* `OctreeBoundingVolume` For using the octree bounding-volume functionality.
* `PackedSpheres` For a scene composed of many analytic spheres.
* `RandomCity` For procedurally building a scene of many objects.
* `Shapes` For using the analytic signed distance functions.

Every example except `BuildBVH` and `HostTuning` also builds for a GPU: `make GPU=HIP` (or `CUDA`), or
CMake with `-DEBGEOMETRY_EXAMPLE_GPU=HIP` (or `CUDA`), compiles the same source for a device, and the
same run then repeats its queries there and compares them with the host's. Without a visible GPU the
device part is skipped.

Examples that couple EBGeometry to a third-party application code (AMReX, Chombo) live under
[`Integrations/`](../Integrations/README.md) at the repository root instead.

The mesh files used by the examples come from the [common-3d-test-models](https://github.com/alecjacobson/common-3d-test-models)
git submodule (`common-3d-test-models/`) at the repository root. Clone the repository with `--recurse-submodules` (or run
`git submodule update --init --recursive`); see the "Building and using" documentation for details.
