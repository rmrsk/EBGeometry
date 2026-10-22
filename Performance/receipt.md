<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
-->

<!-- Written by Scripts/run-performance.sh compare --record and checked by CI (Performance/CheckReceipt.cmake).
     Commit it with the pull request. Do not edit it, except to accept a yellow or red verdict: write the
     reason after "accepted:" in that section. See Docs/Sphinx/source/PerformanceTesting.rst. -->

# Performance receipt

<!-- BEGIN receipt host -->
<!--receipt
version: 2
side: host
verdict: green
accepted:
base: 000723d
base-fingerprint: 17359840f763a281b107aeb1032fe79ff33f801c
head: 776dc07
head-fingerprint: 0d01808f51ae690edc0234181c00c581e14fd6fd
cases: 88
rounds: 3
reps: 5
date: 2026-10-22
-->

## Host comparison: green

`776dc07` against `000723d` (origin/dev), 2026-10-22.

**VERDICT: no regression (0 major, 0 minor across 88 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 88 cases

- base: 000723d (gcc 13.3.0, simd avx2, gpu host)
- head: 776dc07 (gcc 13.3.0, simd avx2, gpu host)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 53.7 ms | 53.7 ms | -0.1% | [-1.8%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 54.6 ms | 52.7 ms | -3.3% | [-8.9%, +2.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 70.5 ms | 70.1 ms | -0.6% | [-2.6%, +5.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 61.3 ms | 58.4 ms | -4.7% | [-14.2%, +14.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 62.0 ms | 61.3 ms | -1.0% | [-34.1%, +5.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 77.9 ms | 77.8 ms | -0.1% | [-17.1%, +21.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 73.2 ms | 73.1 ms | -0.1% | [-30.6%, +44.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 86.8 ms | 86.5 ms | -0.3% | [-19.2%, +23.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 63.5 ms | 63.8 ms | +0.4% | [-3.2%, +10.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 105 ms | 105 ms | +0.3% | [-10.6%, +10.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 81.6 ms | 81.5 ms | -0.1% | [-3.0%, +4.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 142 ms | 140 ms | -1.4% | [-16.8%, +19.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 71.5 ms | 71.9 ms | +0.6% | [-3.5%, +3.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 270 ms | 250 ms | -7.5% | [-8.2%, +6.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 86.8 ms | 88.8 ms | +2.2% | [-25.9%, +4.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 300 ms | 294 ms | -2.0% | [-10.7%, +12.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 52.5 ms | 52.0 ms | -0.9% | [-2.3%, +1.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 218 ms | 218 ms | +0.1% | [-3.9%, +6.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 89.5 ms | 90.8 ms | +1.4% | [-0.8%, +4.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 36.8 ms | 36.8 ms | +0.1% | [-3.2%, +4.0%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 793 ms | 815 ms | +2.7% | [-0.9%, +3.3%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 100 ms | 100 ms | +0.2% | [-2.7%, +1.4%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 73.1 ms | 76.3 ms | +4.3% | [-7.6%, +7.6%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 931 ms | 923 ms | -0.8% | [-3.1%, +5.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 69.7 ms | 69.7 ms | +0.0% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 157 ms | 157 ms | +0.0% | [-4.0%, +2.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 381 ms | 379 ms | -0.5% | [-2.5%, +1.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 83.4 ms | 83.0 ms | -0.5% | [-3.2%, +1.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.90 ms | 7.01 ms | +1.6% | [-1.5%, +3.3%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 155 ms | 152 ms | -1.8% | [-4.3%, +2.8%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 350 ms | 353 ms | +1.0% | [-8.1%, +5.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 184 ms | 183 ms | -0.5% | [-2.1%, +3.9%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 84.0 ms | 83.8 ms | -0.2% | [-1.1%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 167 ms | 165 ms | -1.0% | [-1.7%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 421 ms | 420 ms | -0.2% | [-3.8%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 93.5 ms | 93.2 ms | -0.3% | [-1.9%, +2.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.19 ms | 7.19 ms | -0.1% | [-1.7%, +2.3%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 156 ms | 158 ms | +1.6% | [-2.8%, +2.4%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 366 ms | 365 ms | -0.3% | [-4.1%, +2.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 195 ms | 193 ms | -1.3% | [-1.8%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.6 ms | 11.5 ms | -1.2% | [-2.2%, -0.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 124 ms | 125 ms | +0.3% | [-8.8%, +9.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 537 ms | 536 ms | -0.3% | [-1.2%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 153 ms | +0.2% | [-0.5%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 862 ms | 859 ms | -0.3% | [-1.0%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 120 ms | 121 ms | +0.4% | [-1.4%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 514 ms | 515 ms | +0.3% | [-0.7%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 364 ms | 361 ms | -1.0% | [-2.9%, +5.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 9.36 ms | 9.25 ms | -1.2% | [-1.7%, +1.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 129 ms | 129 ms | +0.2% | [-4.9%, +4.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 566 ms | 570 ms | +0.7% | [-5.4%, +3.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 158 ms | 159 ms | +0.6% | [-1.9%, +5.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 968 ms | 971 ms | +0.4% | [-2.9%, +3.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 123 ms | 124 ms | +0.6% | [-3.6%, +4.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 523 ms | 524 ms | +0.2% | [-0.6%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 368 ms | 368 ms | -0.1% | [-1.0%, +3.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 65.6 ms | 67.4 ms | +2.8% | [+0.7%, +5.0%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 113 ms | 114 ms | +0.4% | [-1.5%, +1.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 273 ms | 275 ms | +0.6% | [-2.0%, +5.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 119 ms | 118 ms | -0.5% | [-4.1%, +5.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 111 ms | 110 ms | -0.5% | [-4.4%, +3.5%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 111 ms | 111 ms | +0.2% | [-3.6%, +2.3%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 248 ms | 248 ms | +0.0% | [-6.8%, +4.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 191 ms | 192 ms | +0.4% | [-1.5%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 71.0 ms | 72.1 ms | +1.7% | [-4.7%, +3.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 127 ms | 125 ms | -0.9% | [-4.1%, +3.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 315 ms | 314 ms | -0.2% | [-3.5%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 137 ms | 139 ms | +1.0% | [-2.9%, +3.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 131 ms | 133 ms | +1.7% | [-2.5%, +5.5%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 116 ms | 116 ms | +0.0% | [-7.2%, +7.4%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 263 ms | 261 ms | -0.8% | [-6.7%, +3.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 202 ms | 202 ms | -0.3% | [-2.0%, +2.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.98 ms | 8.93 ms | -0.5% | [-3.5%, +1.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 147 ms | 142 ms | -3.3% | [-5.4%, -0.2%] | - | faster |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 398 ms | 398 ms | -0.2% | [-3.4%, +2.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 256 ms | 255 ms | -0.4% | [-3.7%, +3.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 338 ms | 339 ms | +0.1% | [-2.1%, +3.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 146 ms | 143 ms | -2.3% | [-3.0%, +3.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 384 ms | 378 ms | -1.7% | [-2.8%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 350 ms | 349 ms | -0.3% | [-1.8%, +2.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 6.44 ms | 6.40 ms | -0.6% | [-2.7%, +5.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 146 ms | 147 ms | +0.8% | [-1.4%, +5.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 430 ms | 437 ms | +1.6% | [-3.5%, +3.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 299 ms | 294 ms | -1.6% | [-4.3%, +4.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 379 ms | 374 ms | -1.2% | [-2.6%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 148 ms | 143 ms | -3.7% | [-5.3%, +5.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 375 ms | 377 ms | +0.5% | [-6.3%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 349 ms | 348 ms | -0.4% | [-4.4%, +3.9%] | - | ~ |  |


The baseline has no Performance/ folder

<!-- END receipt host -->
