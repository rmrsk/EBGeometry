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
| `BVHUnion/`   | `BVHUnion` of many spheres: build and signed distance           |
| `Common/`     | `PerfHarness.hpp`, the harness every probe uses                 |
| `Compare/`    | `PerfCompare`, which compares two sets of results               |

Each probe holds many cases, named `component/input/operation/precision/side`, where the side is `host`,
`device` (the kernel alone) or `hostdevice` (the same inputs on both sides: each side's time, their
ratio, and whether the answers agree). The *Performance testing* page lists every case family with its
input and size; `--list` prints the exact names.

Run them
--------

From the repository root, through one script (needs only CMake, git and a compiler; `--help` lists
every option):

```bash
Scripts/run-performance.sh                          # build if needed, run every case
Scripts/run-performance.sh 'PointCloudBVH/*'        # a subset: one or more globs
Scripts/run-performance.sh list                     # the case names
Scripts/run-performance.sh --gpu cuda --gpu-arch 86 --cuda-compiler /path/to/nvcc   # + device cases
Scripts/run-performance.sh compare origin/dev       # this tree against origin/dev
```

A run prints a table per probe and saves the results to `build/performance/results/`. A comparison
builds both sides from their own `Performance/`, runs every case in a fresh process, alternating between
the sides, and prints the comparison (also written to `build/performance/compare/report.md`) with the
command to repeat it. A case is **minor** at 3% slower and **MAJOR** at 10% slower, in both cases only
outside the 95% confidence interval.

CUDA 12.0's nvcc cannot compile the library; name a newer one with `--cuda-compiler`. `ctest --preset
perf` runs every case at its `--smoke` size (checks that they run, times nothing).
