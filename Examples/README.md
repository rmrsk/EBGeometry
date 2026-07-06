This folder contains basic and advanced examples of using EBGeometry.

Examples prefixed `EBGeometry_` use only EBGeometry (header-only, C++17):

* `EBGeometry_CSGUnion` For merging a surface mesh with an analytic sphere using a BVH-accelerated CSG union.
* `EBGeometry_DCEL` For reading a surface mesh and evaluating it with the DCEL/BVH signed-distance representations.
* `EBGeometry_OctreeBoundingVolume` For using the octree bounding-volume functionality.
* `EBGeometry_PackedSpheres` For a scene composed of many analytic spheres.
* `EBGeometry_RandomCity` For procedurally building a scene of many objects.
* `EBGeometry_Shapes` For using the analytic signed distance functions.

Examples prefixed `AMReX_` additionally require the [AMReX](https://amrex-codes.github.io/amrex/) library:

* `AMReX_DCEL`, `AMReX_PackedSpheres`, `AMReX_PaintEB`, `AMReX_RandomCity`, `AMReX_Shapes`.

The mesh files used by the examples come from the [common-3d-test-models](https://github.com/alecjacobson/common-3d-test-models)
git submodule (`common-3d-test-models/`) at the repository root. Clone the repository with `--recurse-submodules` (or run
`git submodule update --init --recursive`); see the "Building and using" documentation for details.
