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
| `Mesh/`       | `TriMeshSDF` and `MeshSDF`: builds, refits, signed distances    |
| `BVH/`        | `PackedBVH::traverse()` itself: rays and points through a BVH of the mesh's triangle boxes |
| `BVHUnion/`   | `BVHUnion` of spheres, diagonal capsules, random cylinders      |
| `Tape/`       | `Tape` signed distance (CSG, smooth, BVH union) against a hand-written solid |
| `Fused/`      | Fused CSG compositions (the Tape probe's solids, a drilled mesh fused and through a tape) |
| `Ray/`        | Ray queries: a `TriMeshSDF` (`firstHit()`, a hand-written walk, `intersects()`), a `MeshSDF`, and the CSG solid traced, fused and through a tape |
| `RayCSG/`     | Ray tracing a composition from its parts' spans: a scene of 50 solids less 10, fused and through a tape, traced and marched; a crowd of 4,096 balls through a tape |
| `RayShell/`   | Ray tracing a difference whose first operand is marched: a shell around the armadillo with a corner cut away, through a tape (traced and marched) and fused |
| `Render/`     | One frame of the renderer (shadows, ambient occlusion) of the CSG solid through a tape (also with an orthographic camera) and of the armadillo as a `TriMeshSDF` |
| `ClosestPrimitive/` | A tape's `closestPrimitive()`, the deciding leaf (CSG, a scene of fused parts, a drilled mesh) |
| `LaunchTuning/` | `GPU::transform()`'s launch settings (block size, minimum resident threads) on a mesh distance, a tape's rays and a point cloud; device only |
| `Common/`     | the harness every probe includes (`PerfHarness.hpp` includes it all) |
| `Tools/`      | `PerfCompare`, which compares two sets of results               |

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
Scripts/run-performance.sh compare origin/dev       # this tree against origin/dev (about 8 minutes)
Scripts/run-performance.sh compare origin/dev 'PointCloud*' --rounds 1   # times only, while developing
Scripts/run-performance.sh compare origin/dev --smoke   # a quick check: both build, every case runs
Scripts/run-performance.sh compare origin/dev --record  # a pull request's receipt, for CI (commit it)
```

A run prints a table per probe and saves the results to `build/performance/results/`. A comparison
builds both sides from their own `Performance/`, runs every case in a fresh process, alternating between
the sides, and prints the comparison (also written to `build/performance/compare/report.md`) with the
command to repeat it. A case is **minor** at 3% slower and **MAJOR** at 10% slower, in both cases only
outside the 95% confidence interval over the runs (fresh processes; at least two per side). The defaults,
2 rounds of 3 repetitions, are the least that gives a verdict; `--rounds 1` shows times without one,
and `--rounds 10` on a few cases confirms or rules out a flagged change.

CI times nothing. It checks the pull request's performance receipt, `receipt.md`: the result of the
comparison above with `--record` (a host section, and a device section with `--gpu`), committed with the
pull request, once the pull request is ready for review (a draft is not checked). Every section must
have measured the pull request's code, against its base as it is now,
and be green or accepted (`CheckReceipt.cmake`, `Receipt.cmake`).

CUDA 12.0's nvcc cannot compile the library; name a newer one with `--cuda-compiler`. `ctest --preset
perf` runs every case at its `--smoke` size (checks that they run, times nothing).
