This folder contains examples of using EBGeometry on its own (header-only, C++17, no
third-party dependencies):

* `BuildBVH` For comparing BVH build strategies (TreeBVH top-down/SAH/Morton/Nested/Hilbert, and PackedBVH's direct constructor) by build time.
* `ClosestPointBVH` For closest-point search over a point cloud using the turnkey `PointCloudBVH` class: build once from positions, then `closestPoint()` / `closestPoints()` for arbitrary query points, checked against brute force.
* `CSGUnion` For merging a surface mesh with an analytic sphere using a BVH-accelerated CSG union.
* `MeshSDF` For reading a surface mesh and evaluating it with the DCEL/BVH signed-distance representations.
* `NearestNeighborBVH` For the k nearest neighbors of every point in a cloud (the k-NN graph) using `PointCloudBVH::allNearestNeighbors()`, checked against brute force.
* `NestedBVH` For a nested BVH: a BVH-accelerated union over several BVH-backed mesh SDFs.
* `OctreeBoundingVolume` For using the octree bounding-volume functionality.
* `PackedSpheres` For a scene composed of many analytic spheres.
* `RandomCity` For procedurally building a scene of many objects.
* `Shapes` For using the analytic signed distance functions.

Examples that couple EBGeometry to a third-party application code (AMReX, Chombo) live under
[`Integrations/`](../Integrations/README.md) at the repository root instead.

The mesh files used by the examples come from the [common-3d-test-models](https://github.com/alecjacobson/common-3d-test-models)
git submodule (`Submodules/common-3d-test-models/`) at the repository root. Clone the repository with `--recurse-submodules` (or run
`git submodule update --init --recursive`); see the "Building and using" documentation for details.
