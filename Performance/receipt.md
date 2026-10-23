<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
-->

<!-- Written by Scripts/run-performance.sh compare --record and checked by CI (Performance/CheckReceipt.cmake).
     Commit it with the pull request. Do not edit it, except to accept a yellow or red verdict: replace the
     "-" after "Accepted:" under that section's heading with the reason; git history records who did.
     See Docs/Sphinx/source/PerformanceTesting.rst. -->

# Performance receipt

<!-- BEGIN receipt host -->
<!--receipt
version: 3
side: host
verdict: green
base: f3836bd
base-fingerprint: e1c38aaf700ab685c127f3926cd949377a9d4f57
head: acdd07f
head-fingerprint: f044c9e490dd5cd0f20636476f57f2d048418e9f
cases: 88
rounds: 3
reps: 5
date: 2026-10-23
-->

## Host comparison: green

**Accepted:** -

`acdd07f` against `f3836bd` (origin/dev), 2026-10-23.

**VERDICT: no regression (0 major, 0 minor across 88 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 88 cases

- base: f3836bd (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: acdd07f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 54.7 ms | 55.1 ms | +0.8% | [-0.8%, +2.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 49.0 ms | 49.0 ms | -0.0% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.6 ms | 64.3 ms | -0.5% | [-2.1%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 53.6 ms | 53.9 ms | +0.4% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.4 ms | 59.8 ms | -1.0% | [-1.5%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 70.8 ms | 70.9 ms | +0.1% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 73.5 ms | 73.5 ms | +0.0% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 79.1 ms | 80.1 ms | +1.2% | [+0.0%, +2.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.4 ms | 66.6 ms | -1.1% | [-1.5%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 97.4 ms | 98.1 ms | +0.8% | [+0.5%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.1 ms | 82.6 ms | +0.7% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 109 ms | +0.5% | [-0.5%, +0.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.7 ms | 81.5 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 241 ms | 241 ms | +0.2% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | -0.3% | [-0.7%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 282 ms | 278 ms | -1.4% | [-2.8%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 69.9 ms | 69.8 ms | -0.1% | [-1.0%, +1.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 212 ms | 211 ms | -0.8% | [-1.6%, +1.5%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 95.7 ms | 96.2 ms | +0.6% | [+0.3%, +1.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.6 ms | 29.6 ms | +0.3% | [-2.9%, +0.4%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 753 ms | 751 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 117 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 41.1 ms | 42.2 ms | +2.9% | [-0.6%, +3.0%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 838 ms | 837 ms | -0.1% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 67.7 ms | 67.8 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 147 ms | 147 ms | +0.0% | [-0.4%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 366 ms | 367 ms | +0.3% | [-1.5%, +1.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 77.2 ms | 77.4 ms | +0.3% | [-1.9%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.60 ms | 6.64 ms | +0.5% | [-2.1%, +1.3%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 145 ms | 144 ms | -0.9% | [-3.4%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 333 ms | 333 ms | -0.0% | [-2.0%, +2.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 183 ms | 182 ms | -0.1% | [-0.5%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 80.5 ms | 80.6 ms | +0.1% | [-3.1%, +2.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 164 ms | 164 ms | +0.1% | [-0.3%, +1.6%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 407 ms | 408 ms | +0.1% | [-1.0%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.6 ms | 87.0 ms | +0.5% | [-1.8%, +2.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.02 ms | 7.00 ms | -0.3% | [-0.9%, +2.0%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 156 ms | 157 ms | +0.4% | [-3.2%, +1.6%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 356 ms | 356 ms | +0.1% | [-1.6%, +2.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 191 ms | 190 ms | -0.3% | [-0.8%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.5 ms | 11.3 ms | -1.5% | [-2.6%, +0.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 124 ms | 124 ms | -0.1% | [-1.4%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 530 ms | 530 ms | +0.1% | [-0.9%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 152 ms | -0.0% | [-0.5%, +1.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 856 ms | 863 ms | +0.8% | [-1.8%, +1.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 121 ms | 121 ms | -0.4% | [-1.5%, +2.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 509 ms | 513 ms | +0.9% | [-0.7%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 360 ms | 363 ms | +0.7% | [-0.4%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 14.9 ms | -0.4% | [-2.3%, +1.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 128 ms | 126 ms | -1.1% | [-3.6%, +3.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 558 ms | 557 ms | -0.1% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 154 ms | 155 ms | +0.5% | [-1.4%, +1.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 952 ms | 962 ms | +1.1% | [-1.0%, +2.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 122 ms | 121 ms | -0.6% | [-3.0%, +1.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 514 ms | 515 ms | +0.0% | [-1.7%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 363 ms | 363 ms | -0.1% | [-0.3%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.6 ms | 67.1 ms | -0.6% | [-1.0%, +0.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 111 ms | 110 ms | -0.4% | [-2.9%, +3.7%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 264 ms | 265 ms | +0.1% | [-1.4%, +2.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 112 ms | 111 ms | -0.7% | [-2.4%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 105 ms | 105 ms | +0.2% | [-1.3%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 108 ms | 107 ms | -0.3% | [-2.0%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 235 ms | 235 ms | -0.2% | [-2.3%, +2.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 192 ms | 192 ms | +0.0% | [-0.6%, +1.6%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 80.4 ms | 79.8 ms | -0.7% | [-2.4%, +1.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 125 ms | 125 ms | -0.7% | [-3.7%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 303 ms | 303 ms | -0.0% | [-1.6%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 126 ms | 126 ms | -0.1% | [-1.5%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 121 ms | 121 ms | +0.3% | [-0.2%, +3.4%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 118 ms | 118 ms | -0.2% | [-6.2%, +1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 251 ms | 252 ms | +0.3% | [-4.2%, +5.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 200 ms | 200 ms | +0.2% | [-1.3%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.79 ms | 8.86 ms | +0.8% | [-2.3%, +2.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 143 ms | 143 ms | +0.0% | [-4.5%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 397 ms | 396 ms | -0.2% | [-1.8%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 254 ms | 255 ms | +0.2% | [-1.1%, +2.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 333 ms | 332 ms | -0.2% | [-0.9%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 147 ms | 147 ms | +0.2% | [-5.9%, +6.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 387 ms | 387 ms | -0.1% | [-3.4%, +2.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 356 ms | 356 ms | -0.1% | [-2.6%, +2.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.3 ms | 12.3 ms | +0.7% | [+0.4%, +1.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 147 ms | 147 ms | +0.3% | [-4.8%, +4.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 421 ms | 423 ms | +0.3% | [-2.3%, +2.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 287 ms | 286 ms | -0.2% | [-0.6%, +2.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 373 ms | 372 ms | -0.3% | [-0.8%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 141 ms | 141 ms | -0.1% | [-4.2%, +5.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 373 ms | 373 ms | +0.1% | [-0.9%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 345 ms | 345 ms | +0.1% | [-1.0%, +1.2%] | - | ~ |  |

<!-- END receipt host -->
