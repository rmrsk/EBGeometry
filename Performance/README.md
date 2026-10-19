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

Probes print to the terminal and write no file (`--json > file.jsonl` keeps the results);
`Compare.cmake` writes `base.jsonl`, `head.jsonl` and `report.md` under `build/perf-compare/`.

Run a probe
-----------

```bash
cmake --preset perf && cmake --build --preset perf --parallel
build/perf/Performance/bin/PointCloud.ex --list
build/perf/Performance/bin/PointCloud.ex --case 'PointCloudBVH/uniform/*' --side host
ctest --preset perf          # every case at its --smoke size: checks they run, times nothing
```

Options: `--list`, `--case <glob>`, `--side host|device|all`, `--reps <n>`, `--smoke`, `--json`.

For the device cases, build as CUDA or HIP (`cmake --preset perf-cuda`, or
`cmake -S Performance -B build/perf-cuda -DEBGEOMETRY_PERF_GPU=CUDA -DCMAKE_CUDA_ARCHITECTURES=86`).
The folder also builds on its own against any EBGeometry tree (`-DEBGEOMETRY_HOME=...`).

Compare two commits
-------------------

```bash
cmake -DBASE=origin/dev -DHEAD=HEAD -P Performance/Compare.cmake        # or HEAD=WORKTREE
cmake -DBASE=origin/dev -DGPU=CUDA -DGPU_ARCH=86 -P Performance/Compare.cmake
```

Builds both sides from their own `Performance/`, runs every case in a fresh process, alternating
between the sides, and prints the comparison (also written to `build/perf-compare/report.md`) with the
commands to repeat it by hand. A case is **minor** at 3% slower and **MAJOR** at 10% slower, in both
cases only outside the 95% confidence interval. The variables are listed at the top of `Compare.cmake`.
