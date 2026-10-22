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
head: 72d1c86
head-fingerprint: 1bdd8c21d545ff6258ed0842567011e018b95043
cases: 94
rounds: 3
reps: 5
date: 2026-10-22
-->

## Host comparison: green

**Accepted:** -

`72d1c86` against `c548641` (origin/dev), 2026-10-22.

**VERDICT: no regression (0 major, 0 minor across 94 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 94 cases

- base: c548641 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 72d1c86 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 55.4 ms | 55.7 ms | +0.5% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 49.3 ms | 49.5 ms | +0.3% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.8 ms | 65.1 ms | +0.5% | [-0.8%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.4 ms | 54.4 ms | +0.0% | [-0.9%, +2.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.4 ms | 60.9 ms | -0.7% | [-1.3%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 71.7 ms | 71.4 ms | -0.5% | [-1.0%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 73.9 ms | 74.2 ms | +0.5% | [-1.2%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 80.2 ms | 80.7 ms | +0.7% | [-0.8%, +2.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.2 ms | 67.3 ms | +0.2% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 98.4 ms | 98.3 ms | -0.1% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.8 ms | 82.8 ms | +0.0% | [-0.6%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 111 ms | 111 ms | +0.1% | [-0.7%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.5 ms | 82.5 ms | -0.0% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 243 ms | 244 ms | +0.2% | [-0.8%, +1.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 114 ms | 114 ms | +0.1% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 288 ms | 282 ms | -2.0% | [-3.1%, +2.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.8 ms | 70.9 ms | +0.1% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 216 ms | +0.1% | [-1.2%, +1.5%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.6 ms | 96.8 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.7 ms | 30.4 ms | -0.8% | [-7.0%, +0.1%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 768 ms | 772 ms | +0.5% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 119 ms | +0.4% | [+0.1%, +1.4%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.8 ms | 44.8 ms | +0.1% | [-6.2%, +1.5%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 871 ms | 868 ms | -0.4% | [-1.8%, +0.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 68.5 ms | 68.7 ms | +0.2% | [+0.0%, +0.8%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 151 ms | 150 ms | -0.1% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 370 ms | 372 ms | +0.4% | [-0.2%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.2 ms | 77.4 ms | -1.0% | [-4.1%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.65 ms | 6.74 ms | +1.3% | [+0.1%, +2.7%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 146 ms | 146 ms | +0.3% | [-0.0%, +4.2%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 338 ms | 338 ms | +0.0% | [-1.7%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 184 ms | 183 ms | -0.1% | [-0.3%, -0.0%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 81.6 ms | 81.4 ms | -0.2% | [-1.2%, +1.4%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 169 ms | 168 ms | -0.5% | [-1.5%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 411 ms | 412 ms | +0.0% | [-0.8%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 88.3 ms | 89.4 ms | +1.2% | [+0.0%, +9.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.01 ms | 6.96 ms | -0.7% | [-2.9%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 160 ms | 160 ms | -0.1% | [-0.9%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 363 ms | 363 ms | -0.1% | [-1.2%, +2.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 193 ms | 192 ms | -0.1% | [-0.5%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.6 ms | 11.6 ms | +0.2% | [-5.8%, +0.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 125 ms | 125 ms | -0.4% | [-0.9%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 537 ms | 536 ms | -0.2% | [-0.6%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 153 ms | 153 ms | +0.1% | [-0.3%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 861 ms | 862 ms | +0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 121 ms | 121 ms | +0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 512 ms | 512 ms | -0.1% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 362 ms | 362 ms | -0.0% | [-0.3%, +4.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.2 ms | 15.0 ms | -1.4% | [-2.4%, -0.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 128 ms | 128 ms | -0.2% | [-1.2%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 562 ms | 563 ms | +0.1% | [-0.8%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 158 ms | 158 ms | -0.1% | [-0.5%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 957 ms | 961 ms | +0.4% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 122 ms | 122 ms | +0.2% | [-0.9%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 519 ms | 519 ms | -0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 367 ms | 367 ms | -0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 68.2 ms | 67.6 ms | -0.9% | [-1.1%, +0.0%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 112 ms | 112 ms | -0.6% | [-1.4%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 269 ms | 268 ms | -0.3% | [-0.7%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 113 ms | 112 ms | -0.4% | [-0.8%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 107 ms | 106 ms | -0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 108 ms | 109 ms | +0.2% | [-0.5%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 238 ms | 239 ms | +0.2% | [-0.4%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 193 ms | 193 ms | -0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 80.6 ms | 80.4 ms | -0.3% | [-0.9%, +1.0%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 127 ms | 128 ms | +0.2% | [-0.9%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 305 ms | 306 ms | +0.1% | [-0.5%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 128 ms | 129 ms | +0.4% | [-1.1%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 123 ms | 125 ms | +1.0% | [+0.1%, +59.7%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 120 ms | 119 ms | -0.8% | [-1.7%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 258 ms | 257 ms | -0.4% | [-1.3%, +2.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 201 ms | 201 ms | +0.0% | [-0.4%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.89 ms | 10.4 ms | +16.9% | [-1.8%, +18.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 149 ms | 143 ms | -4.0% | [-4.2%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 401 ms | 400 ms | -0.1% | [-3.0%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 256 ms | 256 ms | -0.1% | [-0.6%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 334 ms | 335 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 142 ms | 142 ms | -0.0% | [-7.9%, +2.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 388 ms | 377 ms | -2.9% | [-3.1%, +3.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 350 ms | 350 ms | +0.2% | [-2.1%, +2.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.6 ms | 12.5 ms | -0.6% | [-1.3%, +0.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 146 ms | 146 ms | +0.3% | [+0.1%, +4.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 422 ms | 423 ms | +0.2% | [-0.1%, +2.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 292 ms | 291 ms | -0.5% | [-1.4%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 377 ms | 376 ms | -0.2% | [-0.6%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 143 ms | 149 ms | +4.3% | [-4.1%, +4.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 377 ms | 376 ms | -0.2% | [-3.2%, +3.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 349 ms | 347 ms | -0.4% | [-4.5%, +0.2%] | - | ~ |  |
| `Render/csg/firstHit/float/host` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit/float/host` | - | - | - | - | - | new |  |
| `Render/csg/frame/float/host` | - | - | - | - | - | new |  |
| `Render/csg/firstHit/double/host` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit/double/host` | - | - | - | - | - | new |  |
| `Render/csg/frame/double/host` | - | - | - | - | - | new |  |


The probes differ between the sides (each side ran its own): Performance/Render/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: c548641
base-fingerprint: 16463cbe993d1610f833c1c5ff05e8aaa19ed961
head: 72d1c86
head-fingerprint: 1bdd8c21d545ff6258ed0842567011e018b95043
cases: 64
rounds: 3
reps: 5
date: 2026-10-22
-->

## Device comparison (CUDA): green

**Accepted:** -

`72d1c86` against `c548641` (origin/dev), 2026-10-22.

**VERDICT: no regression (0 major, 0 minor across 64 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 64 cases

- base: c548641 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 72d1c86 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/query-morton/float/hostdevice` | 392 µs | 393 µs | +0.3% | [-6.3%, +6.7%] | - | ~ | device 13.7x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 937 µs | 937 µs | -0.0% | [-1.2%, +1.8%] | - | ~ | device 8.5x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.42 ms | 4.41 ms | -0.2% | [-1.0%, +0.7%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.54 ms | 4.54 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 693 µs | 696 µs | +0.5% | [-0.3%, +1.5%] | - | ~ | device 10.5x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.59 ms | 1.60 ms | +0.4% | [-0.6%, +1.1%] | - | ~ | device 7.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.63 ms | 6.61 ms | -0.3% | [-0.7%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 6.87 ms | 6.87 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 1.13 ms | 1.11 ms | -1.1% | [-1.6%, +0.4%] | - | ~ | device 11.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.67 ms | 2.67 ms | +0.2% | [-0.0%, +1.7%] | - | ~ | device 6.5x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.29 ms | 9.29 ms | +0.0% | [-0.5%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 9.87 ms | 9.87 ms | +0.1% | [-0.5%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.99 ms | 8.94 ms | -0.5% | [-1.4%, +1.9%] | - | ~ | device 10.5x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 22.1 ms | 21.9 ms | -0.8% | [-1.8%, +1.7%] | - | ~ | device 6.5x faster than the host (was 5.4x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 140 ms | 141 ms | +0.0% | [-0.7%, +3.6%] | - | ~ | device 0.7x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 263 ms | 263 ms | -0.2% | [-2.2%, +2.6%] | - | ~ | device 0.5x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 28.7 ms | 28.8 ms | +0.3% | [-0.6%, +1.0%] | - | ~ | device 3.4x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 68.1 ms | 68.2 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.1x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 139 ms | 139 ms | -0.2% | [-4.1%, +1.5%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 230 ms | 230 ms | -0.2% | [-3.4%, +4.6%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 784 µs | 783 µs | -0.2% | [-1.0%, +0.5%] | - | ~ | device 10.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 3.11 ms | 3.10 ms | -0.3% | [-1.1%, +0.5%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.16 ms | -0.1% | [-0.7%, +0.7%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 11.2 ms | 11.2 ms | +0.2% | [-1.2%, +0.4%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 6.63 ms | 6.58 ms | -0.8% | [-1.4%, -0.1%] | - | ~ | device 2.6x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 6.95 ms | 6.94 ms | -0.2% | [-1.0%, +0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.80 ms | 7.74 ms | -0.7% | [-2.0%, +0.9%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | -0.0% | [-14.0%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 25.6 ms | 25.6 ms | -0.1% | [-0.7%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 14.8 ms | 14.7 ms | -0.4% | [-1.1%, +0.9%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.03 ms | 2.03 ms | +0.0% | [-2.0%, +0.8%] | - | ~ | device 5.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.18 ms | 5.17 ms | -0.1% | [-0.4%, +0.6%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 856 µs | 856 µs | +0.0% | [-0.6%, +0.4%] | - | ~ | device 10.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 11.6 ms | 11.5 ms | -0.6% | [-1.9%, +0.8%] | - | ~ | device 2.9x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 7.40 ms | 7.31 ms | -1.1% | [-2.5%, +0.5%] | - | ~ | device 3.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 7.63 ms | 7.75 ms | +1.6% | [-17.5%, +21.0%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 14.9 ms | 13.9 ms | -7.1% | [-14.7%, +7.2%] | - | ~ | device 1.2x faster than the host (was 1.1x) |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 3.38 ms | 3.38 ms | -0.1% | [-0.9%, +1.1%] | - | ~ | device 2.6x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 24.9 ms | 24.9 ms | -0.0% | [-0.9%, +0.7%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 16.8 ms | 16.7 ms | -0.9% | [-1.7%, +3.3%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.48 ms | 1.48 ms | +0.3% | [-0.4%, +1.3%] | - | ~ | device 8.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 6.43 ms | 6.44 ms | +0.0% | [-0.7%, +0.5%] | - | ~ | device 2.9x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.51 ms | 1.51 ms | +0.2% | [-11.5%, +13.1%] | - | ~ | device 5.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 5.63 ms | 5.63 ms | +0.1% | [-1.3%, +1.1%] | - | ~ | device 3.3x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 7.57 ms | 7.57 ms | +0.0% | [-0.3%, +1.2%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.2 ms | 13.1 ms | -1.2% | [-10.9%, +11.4%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 15.6 ms | 15.5 ms | -0.2% | [-3.3%, +3.5%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 6.51 ms | 6.47 ms | -0.7% | [-8.6%, +9.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 15.4 ms | 15.5 ms | +0.4% | [-0.3%, +1.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 18.5 ms | 18.5 ms | -0.0% | [-0.6%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 6.39 ms | 6.49 ms | +1.5% | [+0.5%, +2.9%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 18.2 ms | 18.2 ms | -0.1% | [-0.4%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.91 ms | 1.91 ms | +0.2% | [-10.0%, +11.1%] | - | ~ | device 6.1x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.77 ms | 5.82 ms | +0.9% | [-4.0%, +3.9%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 14.9 ms | 14.9 ms | +0.0% | [-1.1%, +0.9%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 29.0 ms | 28.8 ms | -0.5% | [-1.6%, +3.9%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 58.9 ms | 58.7 ms | -0.3% | [-2.2%, +3.0%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 15.4 ms | 15.4 ms | +0.5% | [-0.4%, +1.1%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 19.8 ms | 20.1 ms | +1.4% | [-2.1%, +1.8%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 39.8 ms | 39.9 ms | +0.2% | [-1.6%, +3.3%] | - | ~ | device 0.6x faster than the host |
| `Render/csg/firstHit-camera/float/hostdevice` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit-camera/float/hostdevice` | - | - | - | - | - | new |  |
| `Render/csg/firstHit-camera/double/hostdevice` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit-camera/double/hostdevice` | - | - | - | - | - | new |  |


The probes differ between the sides (each side ran its own): Performance/Render/main.cpp

<!-- END receipt cuda -->
