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
base: 5e3febe
base-fingerprint: 866e6a5180c53f362ab3ae9e222322a43f91e901
head: 5594f7b
head-fingerprint: 5d486a59bfb4cdfdfbfe20bbd6c0c73f42a9e748
cases: 94
rounds: 3
reps: 5
date: 2026-10-22
-->

## Host comparison: green

`5594f7b` against `5e3febe` (origin/dev), 2026-10-22.

**VERDICT: no regression (0 major, 0 minor across 94 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 94 cases

- base: 5e3febe (gcc 13.3.0, simd avx2, gpu host)
- head: 5594f7b (gcc 13.3.0, simd avx2, gpu host)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 53.7 ms | 53.5 ms | -0.4% | [-1.8%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 49.2 ms | 49.4 ms | +0.4% | [-1.8%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 62.3 ms | 62.5 ms | +0.4% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.3 ms | 53.7 ms | -1.1% | [-2.1%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 58.1 ms | 58.2 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 71.2 ms | 71.0 ms | -0.3% | [-0.5%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 62.7 ms | 62.3 ms | -0.5% | [-1.3%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 80.0 ms | 80.3 ms | +0.3% | [-1.1%, +2.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.0 ms | 64.0 ms | +0.0% | [-1.2%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.1 ms | 99.4 ms | +0.2% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 76.1 ms | 76.0 ms | -0.1% | [-2.0%, +4.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 110 ms | 109 ms | -0.2% | [-1.7%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 70.9 ms | 70.9 ms | -0.0% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 244 ms | 243 ms | -0.3% | [-1.0%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 85.1 ms | 85.3 ms | +0.2% | [-1.1%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 285 ms | 286 ms | +0.1% | [-2.0%, +1.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 59.9 ms | 60.1 ms | +0.3% | [-1.0%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 216 ms | 213 ms | -1.3% | [-1.7%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 82.4 ms | 82.3 ms | -0.0% | [-1.7%, +1.6%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.7 ms | 30.5 ms | -0.6% | [-2.6%, +17.2%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 768 ms | 766 ms | -0.3% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 99.0 ms | 98.5 ms | -0.4% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 45.8 ms | 43.8 ms | -4.2% | [-4.8%, +2.2%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 869 ms | 866 ms | -0.3% | [-4.4%, +1.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 66.4 ms | 66.1 ms | -0.5% | [-1.0%, +3.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 151 ms | 150 ms | -0.4% | [-11.9%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 372 ms | 371 ms | -0.3% | [-5.1%, -0.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.0 ms | 79.1 ms | +1.4% | [-1.0%, +4.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.63 ms | 6.60 ms | -0.5% | [-1.8%, -0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 146 ms | 147 ms | +0.3% | [-0.2%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 339 ms | 339 ms | -0.0% | [-0.8%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 183 ms | 184 ms | +0.1% | [-0.1%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 83.6 ms | 84.0 ms | +0.4% | [-0.9%, +2.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 167 ms | 168 ms | +0.3% | [-0.6%, +1.6%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 414 ms | 414 ms | +0.0% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 88.2 ms | 88.3 ms | +0.1% | [-1.0%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.98 ms | 6.93 ms | -0.7% | [-3.5%, +1.4%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 159 ms | 159 ms | +0.0% | [-0.3%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 364 ms | 364 ms | -0.0% | [-0.7%, +6.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 191 ms | 192 ms | +0.3% | [-0.5%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.5 ms | 11.6 ms | +1.4% | [+0.5%, +7.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 125 ms | 125 ms | -0.2% | [-1.0%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 536 ms | 537 ms | +0.3% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 153 ms | 153 ms | +0.2% | [-0.5%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 864 ms | 866 ms | +0.2% | [-0.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 121 ms | 122 ms | +0.3% | [-0.7%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 514 ms | 514 ms | -0.0% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 361 ms | 362 ms | +0.2% | [-0.6%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 9.25 ms | 9.41 ms | +1.7% | [-8.6%, +9.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 128 ms | 128 ms | +0.5% | [-0.8%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 565 ms | 564 ms | -0.1% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 158 ms | 158 ms | -0.4% | [-1.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 958 ms | 956 ms | -0.2% | [-1.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 123 ms | 123 ms | -0.1% | [-0.2%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 517 ms | 518 ms | +0.2% | [-0.5%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 365 ms | 366 ms | +0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 65.8 ms | 65.8 ms | -0.1% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 112 ms | 111 ms | -0.4% | [-1.7%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 272 ms | 271 ms | -0.4% | [-0.4%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 113 ms | 113 ms | +0.2% | [-0.7%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 107 ms | 106 ms | -0.4% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 109 ms | 109 ms | +0.2% | [-0.4%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 238 ms | 238 ms | +0.2% | [-0.2%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 193 ms | 193 ms | -0.1% | [-0.2%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 81.8 ms | 82.2 ms | +0.4% | [-8.3%, +4.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 127 ms | 127 ms | -0.5% | [-1.4%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 309 ms | 309 ms | -0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 129 ms | 128 ms | -0.4% | [-0.8%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 126 ms | 124 ms | -1.5% | [-13.7%, +1.4%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 120 ms | 121 ms | +0.9% | [-1.0%, +23.7%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 257 ms | 259 ms | +0.5% | [-0.4%, +10.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 201 ms | 200 ms | -0.6% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.85 ms | 8.86 ms | +0.1% | [-4.2%, +1.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 142 ms | 142 ms | -0.2% | [-0.4%, +4.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 399 ms | 400 ms | +0.3% | [-3.4%, +3.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 255 ms | 258 ms | +1.3% | [-0.7%, +2.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 337 ms | 335 ms | -0.3% | [-1.0%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 141 ms | 141 ms | -0.2% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 387 ms | 389 ms | +0.4% | [-0.0%, +3.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 356 ms | 356 ms | +0.1% | [-0.6%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 6.44 ms | 6.39 ms | -0.7% | [-4.5%, +1.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 153 ms | 147 ms | -4.2% | [-4.5%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 426 ms | 425 ms | -0.1% | [-3.1%, +3.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 296 ms | 295 ms | -0.5% | [-1.7%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 379 ms | 382 ms | +1.0% | [-1.2%, +4.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 150 ms | 150 ms | -0.0% | [-6.0%, +6.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 389 ms | 389 ms | -0.0% | [-4.5%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 349 ms | 351 ms | +0.7% | [-3.2%, +1.7%] | - | ~ |  |
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
version: 2
side: cuda
verdict: green
accepted:
base: 5e3febe
base-fingerprint: 866e6a5180c53f362ab3ae9e222322a43f91e901
head: 5594f7b
head-fingerprint: 5d486a59bfb4cdfdfbfe20bbd6c0c73f42a9e748
cases: 64
rounds: 3
reps: 5
date: 2026-10-22
-->

## Device comparison (CUDA): green

`5594f7b` against `5e3febe` (origin/dev), 2026-10-22.

**VERDICT: no regression (0 major, 0 minor across 64 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 64 cases

- base: 5e3febe (gcc 13.3.0, simd none, gpu CUDA)
- head: 5594f7b (gcc 13.3.0, simd none, gpu CUDA)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/query-morton/float/hostdevice` | 393 µs | 398 µs | +1.4% | [-10.9%, +6.7%] | - | ~ | device 13.7x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 931 µs | 932 µs | +0.1% | [-0.3%, +1.4%] | - | ~ | device 8.6x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.42 ms | 4.40 ms | -0.4% | [-1.0%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.55 ms | 4.54 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 692 µs | 692 µs | -0.0% | [-1.0%, +1.0%] | - | ~ | device 10.7x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.59 ms | 1.60 ms | +0.3% | [-0.7%, +0.7%] | - | ~ | device 7.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.61 ms | 6.61 ms | +0.0% | [-0.2%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 6.87 ms | 6.86 ms | -0.2% | [-0.5%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 1.12 ms | 1.12 ms | -0.4% | [-1.7%, +0.3%] | - | ~ | device 11.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.68 ms | 2.68 ms | +0.0% | [-1.9%, +0.7%] | - | ~ | device 6.5x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.30 ms | 9.28 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 9.88 ms | 9.85 ms | -0.3% | [-0.5%, +0.0%] | - | ~ | device 1.9x faster than the host |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 9.14 ms | 9.28 ms | +1.5% | [-2.8%, +6.0%] | - | ~ | device 10.3x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 22.0 ms | 22.1 ms | +0.4% | [-1.8%, +1.7%] | - | ~ | device 5.4x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 140 ms | 140 ms | -0.2% | [-0.8%, +1.8%] | - | ~ | device 0.7x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 264 ms | 264 ms | +0.0% | [-2.6%, +1.7%] | - | ~ | device 0.5x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 28.6 ms | 28.6 ms | +0.2% | [-1.6%, +1.1%] | - | ~ | device 3.4x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 68.0 ms | 67.9 ms | -0.2% | [-1.0%, +0.0%] | - | ~ | device 2.1x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 138 ms | 137 ms | -0.4% | [-4.0%, +3.3%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 231 ms | 229 ms | -0.9% | [-5.0%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 778 µs | 777 µs | -0.1% | [-0.9%, +0.5%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 3.09 ms | 3.06 ms | -0.7% | [-2.1%, +0.7%] | - | ~ | device 4.1x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.16 ms | 1.16 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 11.2 ms | 11.2 ms | +0.1% | [-1.8%, +0.7%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 6.57 ms | 6.60 ms | +0.4% | [-0.1%, +1.6%] | - | ~ | device 2.6x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 6.93 ms | 6.98 ms | +0.6% | [-1.5%, +1.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.77 ms | 7.81 ms | +0.4% | [-0.2%, +0.7%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | -0.1% | [-10.3%, +6.9%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 25.6 ms | 25.6 ms | +0.2% | [-0.5%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 14.8 ms | 14.9 ms | +0.6% | [-0.5%, +1.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.04 ms | 2.05 ms | +0.3% | [-1.0%, +1.1%] | - | ~ | device 5.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.15 ms | 5.18 ms | +0.5% | [-0.7%, +7.7%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 858 µs | 856 µs | -0.2% | [-7.4%, +8.4%] | - | ~ | device 10.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 11.6 ms | 11.6 ms | +0.1% | [-1.4%, +2.5%] | - | ~ | device 2.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 7.33 ms | 7.31 ms | -0.3% | [-2.1%, +0.5%] | - | ~ | device 3.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 7.25 ms | 7.22 ms | -0.4% | [-13.1%, +18.4%] | - | ~ | device 1.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.7 ms | 14.0 ms | -10.6% | [-18.1%, +10.8%] | - | ~ | device 1.2x faster than the host (was 1.1x) |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 3.37 ms | 3.37 ms | -0.1% | [-1.3%, +0.8%] | - | ~ | device 2.6x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 24.7 ms | 24.8 ms | +0.3% | [-0.6%, +0.9%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 16.9 ms | 16.6 ms | -1.4% | [-2.5%, +1.9%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.48 ms | 1.47 ms | -0.9% | [-1.5%, +0.9%] | - | ~ | device 8.8x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 6.42 ms | 6.49 ms | +1.1% | [+0.0%, +2.2%] | - | ~ | device 2.9x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.52 ms | 1.51 ms | -0.7% | [-11.4%, +12.3%] | - | ~ | device 5.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 5.65 ms | 5.61 ms | -0.6% | [-4.4%, -0.1%] | - | ~ | device 3.3x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 7.53 ms | 7.54 ms | +0.2% | [-0.3%, +0.9%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.3 ms | 13.2 ms | -0.3% | [-11.4%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 15.6 ms | 15.4 ms | -1.0% | [-5.4%, +2.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 6.44 ms | 6.49 ms | +0.9% | [-1.1%, +1.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 15.5 ms | 15.5 ms | -0.1% | [-0.4%, +1.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 18.5 ms | 18.5 ms | +0.0% | [-0.8%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 6.39 ms | 6.44 ms | +0.7% | [-2.5%, +1.8%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 18.1 ms | 18.2 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.91 ms | 1.92 ms | +0.4% | [-7.2%, +10.4%] | - | ~ | device 6.1x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.74 ms | 5.76 ms | +0.2% | [-4.1%, +6.1%] | - | ~ | device 4.7x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 14.8 ms | 14.9 ms | +0.4% | [-2.1%, +0.6%] | - | ~ | device 1.6x faster than the host (was 1.9x) |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 29.2 ms | 29.2 ms | -0.2% | [-1.6%, +2.2%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 59.3 ms | 58.9 ms | -0.6% | [-1.9%, +2.5%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 15.4 ms | 15.5 ms | +0.4% | [-1.8%, +1.9%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.0 ms | 19.9 ms | -0.5% | [-1.8%, +1.7%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 39.8 ms | 39.7 ms | -0.3% | [-1.6%, +0.1%] | - | ~ | device 0.6x faster than the host |
| `Render/csg/firstHit-camera/float/hostdevice` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit-camera/float/hostdevice` | - | - | - | - | - | new |  |
| `Render/csg/firstHit-camera/double/hostdevice` | - | - | - | - | - | new |  |
| `Render/mesh/firstHit-camera/double/hostdevice` | - | - | - | - | - | new |  |


The probes differ between the sides (each side ran its own): Performance/Render/main.cpp

<!-- END receipt cuda -->
