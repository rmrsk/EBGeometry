<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
-->

Performance
===========

Programs that measure what EBGeometry costs, so that every change can be compared with its base on the
same machine: `Examples/` shows how to use the library, `Tests/` checks that it is correct, and this
folder measures it. The full description is the *Performance testing* page of the user documentation
(`Docs/Sphinx/source/PerformanceTesting.rst`).

| Folder        | What it measures                                                |
|---------------|-----------------------------------------------------------------|
| `PointCloud/` | `PointCloudBVH` and `PointCloudHashGrid`: builds, k-NN, queries |
| `PointCloudBVHBuild/` | `PointCloudBVH`'s `build()` and `rebuild()` where its pool is: in place on the host, and in device memory |
| `PointCloudHashGridBuild/` | `PointCloudHashGrid`'s `build()` and `rebuild()` where its pool is: in place on the host, and in device memory |
| `SphereCloudBVH/` | `SphereCloudBVH`: its build and rebuild (in place on the host, and in device memory), and the closest sphere |
| `SphereContacts/` | `SphereCloudBVH::forEachContact()`: every particle's contacts and contact force, on the host and on a device |
| `SphereCloudRays/` | `SphereCloudBVH`'s ray form: rays through the cloud, and through a box with the cloud cut out of it, fused and in a tape, on the host and on a device |
| `Mesh/`       | `TriMeshSDF` and `MeshSDF`: builds, refits, signed distances    |
| `MeshReconcile/` | Reconciling a moved DCEL mesh: `MeshT::reconcile()`, and `DCEL::reconcile()` where its pool is, in host loops and in device memory |
| `BVH/`        | `PackedBVH::traverse()` itself: rays and points through a BVH of the mesh's triangle boxes |
| `BVHRefit/`   | `BVH::refit()` where its pool is: the host's sweep, and a launch per tree level in device memory |
| `BVHUnion/`   | `BVHUnion` of spheres, diagonal capsules, random cylinders      |
| `BVHUnionMixed/` | `BVHUnion` of the three types at once, interleaved          |
| `Shapes/`     | The analytic shapes `EllipsoidSDF` to `WedgeSDF`: each one's signed distance, and rays through a fused union of the six with closed-form ray forms, on the host and on a device |
| `TapeBVHUnion/` | A `Tape`'s union of 64 shapes under maps, through a BVH (`Accelerate::BVH`) and as written |
| `Tape/`       | `Tape` signed distance (CSG, smooth, BVH union) against a hand-written solid |
| `Fused/`      | Fused CSG compositions (the Tape probe's solids, a drilled mesh fused and through a tape) |
| `Ray/`        | Ray queries: a `TriMeshSDF` (`firstHit()`, a hand-written walk, `intersects()`), a `MeshSDF`, and the CSG solid traced, fused and through a tape |
| `RayCSG/`     | Ray tracing a composition from its parts' spans: a scene of 50 solids less 10, fused and through a tape, traced and marched; a crowd of 4,096 balls through a tape |
| `RayShell/`   | Ray tracing a difference of a shell and a box: a shell around the armadillo with a corner cut away, through a tape (traced and marched) and fused |
| `RayParticles/` | Particles colliding with a boundary, one short step each: the armadillo's shell through a tape (traced and marched) and fused, the solid armadillo, and a box's shell |
| `Render/`     | One frame of the renderer (shadows, ambient occlusion) of the CSG solid through a tape (also with an orthographic camera) and of the armadillo as a `TriMeshSDF` |
| `MeshCSG/`    | A mesh standing on a table, joined by a plain union through a tape and by a smooth union through a tape and fused: rays and renderer frames |
| `RayCrossing/` | `firstCrossing()`, the first crossing without its normal, against `firstHit()`: a mesh, a CSG tape, and tapes holding a mesh (drilled, traced; smoothly joined to a ball, marched) |
| `ClosestPrimitive/` | A tape's `closestLeaf()`, the deciding leaf (CSG, a scene of fused parts, a drilled mesh) |
| `ClosestSurface/` | A tape's `closestSurface()`, drilled into a cloud of spheres (a `SphereCloudBVH`, a `BVHUnion`), against `closestLeaf()` |
| `SurfaceParts/` | A tape's surfaces with their parts and labels: `closestSurface()` on a union of boxes and on a mesh labeled by face groups, and `firstSurface()` at that mesh, against `closestLeaf()` and `firstHit()` |
| `Regions/`    | A tape's regions: `region()`, every region's function in one pass (`regionDistances()`, against asking region by region) and a domain's label functions, for a matrix holding 32 two-layer particles; rays at a one-region assembly, `firstRegionChange()` against `firstHit()` |
| `Domain/`     | A domain with many subdomains and surface groups (a matrix and 32 shells each its own subdomain, 33 selectors in 8 surface groups): every label's function at a point, a point's label, and a surface's group |
| `LaunchTuning/` | `GPU::transform()`'s launch settings (block size, minimum resident threads) on a mesh distance, a tape's rays and a point cloud; device only |
| `Common/`     | the harness every probe includes (`PerfHarness.hpp` includes it all) |
| `Tools/`      | `PerfCompare`, which compares two sets of results; `BuildCost`, the compiler launcher that records what each compile costs, and `BuildCompare`, which compares two builds' costs |

Each probe holds many cases, named `component/input/operation/precision/side`. A host build runs the
`host` cases, on one core; a CUDA or HIP build runs only the `hostdevice` pairs: every device query with
the same inputs on both sides, the host side on every hardware thread but one (OpenMP), giving each side's time, the
device's speedup and whether the answers agree. A case that names another computing the same answers
(`Case::relativeTo()`: each `Fused` case names its tape counterpart) gets the ratio of their times in its
note, e.g. `tape/fused 2.78x`. The *Performance testing* page lists every case family with its
input and size; `--list` prints the exact names.

Run them
--------

From the repository root, through one script (needs only CMake, git and a compiler; `--help` lists
every option):

```bash
Scripts/run-performance.sh                          # build if needed, run every host case
Scripts/run-performance.sh 'PointCloudBVH/*'        # a subset: one or more globs
Scripts/run-performance.sh list                     # the case names
Scripts/run-performance.sh --gpu cuda --gpu-arch 86 --cuda-compiler /path/to/nvcc   # the host/device pairs
Scripts/run-performance.sh compare origin/<base>    # this tree against the base (about 8 minutes)
Scripts/run-performance.sh compare origin/<base> 'PointCloud*' --rounds 1   # times only, while developing
Scripts/run-performance.sh compare origin/<base> --smoke   # a quick check: both build, every case runs
Scripts/run-performance.sh compare origin/<base> --record  # a pull request's receipt, for CI (commit it)
```

`<base>` is the branch the pull request targets.

A run prints a table per probe and saves the results to `build/performance/results/`. A comparison
builds both sides from their own `Performance/`, runs every case in a fresh process, alternating between
the sides, and prints the comparison (also written to `build/performance/compare/report.md`) with the
command to repeat it. A probe that compiled to the same code on both sides is not run (`--no-skip` runs
it), and a side built before with the same code and settings is taken from `build/performance/cache`
(`--no-cache` builds it again). A case is **minor** at 3% slower and **MAJOR** at 10% slower, in both cases only
outside the 95% confidence interval over the runs (fresh processes; at least two per side). The defaults,
2 rounds of 3 repetitions, are the least that gives a verdict; a flagged case then runs 8 more rounds
(`--confirm-rounds`) and is judged on all of them, and `--rounds 1` shows times without a verdict.

CI times nothing. It checks the pull request's performance receipt, `receipt.md`: the result of the
comparison above with `--record` (a host section, and a device section with `--gpu`), committed with the
pull request, once the pull request is ready for review (a draft is not checked). Every section must
have measured the pull request's code, against its base as it is now,
and be green or accepted (`CheckReceipt.cmake`, `Receipt.cmake`).

CUDA 12.0's nvcc cannot compile the library; name a newer one with `--cuda-compiler`. `ctest --preset
perf` runs every case at its `--smoke` size (checks that they run, times nothing).
