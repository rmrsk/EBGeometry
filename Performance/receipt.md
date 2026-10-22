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
base: c548641
base-fingerprint: 16463cbe993d1610f833c1c5ff05e8aaa19ed961
head: 5dafc17
head-fingerprint: e1c38aaf700ab685c127f3926cd949377a9d4f57
cases: 88
rounds: 3
reps: 5
date: 2026-10-22
-->

## Host comparison: green

**Accepted:** -

`5dafc17` against `c548641` (origin/dev), 2026-10-22.

**VERDICT: no regression (0 major, 0 minor across 88 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 88 cases

- base: c548641 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 5dafc17 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 55.4 ms | 55.2 ms | -0.3% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.6 ms | 49.7 ms | -3.6% | [-5.3%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.0 ms | 65.2 ms | +0.3% | [-0.3%, +1.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.7 ms | 54.1 ms | -1.0% | [-2.1%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.7 ms | 61.1 ms | +0.7% | [-0.1%, +1.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 71.7 ms | 71.6 ms | -0.1% | [-1.6%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.5 ms | 74.9 ms | +0.5% | [-0.6%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 81.0 ms | 80.2 ms | -0.9% | [-1.1%, -0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.2 ms | 67.5 ms | +0.4% | [-1.6%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 98.6 ms | 98.0 ms | -0.5% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.9 ms | 83.6 ms | +0.9% | [-0.3%, +1.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 111 ms | 111 ms | +0.0% | [-1.3%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.8 ms | 82.7 ms | -0.2% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 243 ms | 243 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 114 ms | +0.5% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 289 ms | 289 ms | -0.2% | [-2.5%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 71.1 ms | 70.7 ms | -0.6% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 214 ms | 215 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.9 ms | 98.6 ms | +1.7% | [-0.3%, +2.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.4 ms | 30.2 ms | -0.8% | [-1.1%, +0.9%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 771 ms | 768 ms | -0.4% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 119 ms | 118 ms | -1.3% | [-4.0%, -0.4%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 45.4 ms | 44.3 ms | -2.3% | [-3.5%, +1.6%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 869 ms | 867 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 68.5 ms | 68.6 ms | +0.2% | [-1.0%, +1.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 150 ms | 150 ms | -0.0% | [-0.8%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 371 ms | 370 ms | -0.0% | [-0.2%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.1 ms | 78.0 ms | -0.1% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.65 ms | 6.63 ms | -0.2% | [-2.9%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 146 ms | 146 ms | -0.2% | [-0.7%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 338 ms | 338 ms | -0.1% | [-0.6%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 184 ms | 184 ms | +0.0% | [-0.1%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 81.7 ms | 81.8 ms | +0.1% | [-1.1%, +0.9%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 169 ms | 169 ms | -0.1% | [-1.5%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 412 ms | 412 ms | +0.1% | [-0.1%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 89.1 ms | 88.4 ms | -0.8% | [-0.8%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.01 ms | 7.00 ms | -0.2% | [-2.1%, +1.9%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 160 ms | 160 ms | +0.3% | [-1.3%, +1.9%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 363 ms | 362 ms | -0.3% | [-0.7%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 192 ms | 191 ms | -0.3% | [-0.8%, +1.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.6 ms | 11.5 ms | -0.9% | [-2.3%, +0.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 125 ms | 125 ms | -0.1% | [-1.3%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 535 ms | 536 ms | +0.1% | [-0.5%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 153 ms | 153 ms | -0.0% | [-0.2%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 863 ms | 863 ms | +0.0% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 121 ms | 120 ms | -0.1% | [-0.5%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 514 ms | 513 ms | -0.2% | [-0.6%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 362 ms | 362 ms | -0.1% | [-0.5%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 14.9 ms | -0.2% | [-1.1%, +2.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 128 ms | 128 ms | -0.1% | [-1.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 564 ms | 562 ms | -0.2% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 155 ms | 157 ms | +0.8% | [+0.3%, +2.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 955 ms | 957 ms | +0.3% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 122 ms | 122 ms | -0.1% | [-0.2%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 521 ms | 521 ms | -0.1% | [-0.9%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 367 ms | 368 ms | +0.0% | [-0.6%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.9 ms | 67.7 ms | -0.2% | [-1.5%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 112 ms | 113 ms | +1.0% | [-0.5%, +1.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 271 ms | 270 ms | -0.5% | [-7.1%, +4.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 112 ms | 113 ms | +0.5% | [+0.2%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 106 ms | 106 ms | -0.5% | [-0.8%, -0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 109 ms | 110 ms | +1.0% | [-0.7%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 238 ms | 238 ms | +0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 193 ms | 193 ms | +0.0% | [-0.0%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 81.2 ms | 80.8 ms | -0.5% | [-0.9%, +0.7%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 128 ms | 128 ms | -0.0% | [-1.6%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 305 ms | 305 ms | +0.1% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 128 ms | 129 ms | +1.0% | [+0.2%, +1.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 124 ms | 123 ms | -0.2% | [-0.7%, +1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 119 ms | 120 ms | +0.2% | [-0.8%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 257 ms | 258 ms | +0.1% | [-1.1%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 200 ms | 200 ms | -0.3% | [-0.3%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.81 ms | 8.86 ms | +0.5% | [-1.4%, +20.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 150 ms | 144 ms | -4.0% | [-4.1%, +4.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 401 ms | 413 ms | +3.1% | [-2.7%, +3.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 256 ms | 255 ms | -0.6% | [-1.2%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 334 ms | 335 ms | +0.2% | [-1.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 142 ms | 142 ms | +0.3% | [-4.1%, +4.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 378 ms | 377 ms | -0.3% | [-3.3%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 350 ms | 350 ms | -0.0% | [-0.7%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.5 ms | 12.5 ms | +0.5% | [-0.8%, +1.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 147 ms | 153 ms | +4.1% | [-0.7%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 424 ms | 424 ms | -0.0% | [-2.9%, +3.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 291 ms | 293 ms | +0.6% | [-5.6%, +4.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 376 ms | 376 ms | +0.1% | [-0.2%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 143 ms | 143 ms | +0.0% | [-0.1%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 376 ms | 376 ms | +0.0% | [-0.8%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 348 ms | 348 ms | +0.0% | [-0.6%, +0.2%] | - | ~ |  |

<!-- END receipt host -->
