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
base: ecb6340
base-fingerprint: f044c9e490dd5cd0f20636476f57f2d048418e9f
head: 4eaf9bf
head-fingerprint: fbf75b59076b6fbcc20ee17ec308dd941ac09f5d
cases: 88
rounds: 3
reps: 5
date: 2026-10-23
-->

## Host comparison: green

**Accepted:** -

`4eaf9bf` against `ecb6340` (origin/dev), 2026-10-23.

**VERDICT: no regression (0 major, 0 minor across 88 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 88 cases

- base: ecb6340 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 4eaf9bf (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 55.7 ms | 55.4 ms | -0.6% | [-1.2%, -0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 49.4 ms | 49.3 ms | -0.4% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.6 ms | 64.5 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.3 ms | 53.9 ms | -0.6% | [-0.9%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.3 ms | 60.1 ms | -0.4% | [-1.0%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 71.6 ms | 71.2 ms | -0.6% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.2 ms | 74.0 ms | -0.2% | [-1.2%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 80.7 ms | 80.5 ms | -0.2% | [-1.5%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.2 ms | 67.3 ms | +0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 98.6 ms | 99.1 ms | +0.5% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.0 ms | 82.7 ms | -0.4% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 110 ms | 110 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.3 ms | 82.3 ms | +0.1% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 242 ms | 239 ms | -1.5% | [-2.1%, -0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 115 ms | 115 ms | -0.0% | [-1.1%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 283 ms | 283 ms | +0.2% | [-3.4%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.7 ms | 70.9 ms | +0.3% | [-1.2%, +1.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 212 ms | 209 ms | -1.2% | [-2.1%, -0.9%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.8 ms | 97.0 ms | +0.2% | [-1.9%, +0.4%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.3 ms | 30.7 ms | +1.3% | [-0.2%, +2.2%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 772 ms | 773 ms | +0.1% | [-1.2%, +1.5%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | +0.6% | [+0.4%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.6 ms | 45.4 ms | +1.9% | [-1.9%, +4.8%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 863 ms | 857 ms | -0.7% | [-1.0%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 68.5 ms | 68.3 ms | -0.3% | [-0.9%, +1.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 149 ms | 149 ms | -0.3% | [-2.1%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 369 ms | 370 ms | +0.2% | [-0.1%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 77.7 ms | 78.0 ms | +0.4% | [-0.4%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.61 ms | 6.67 ms | +0.9% | [-0.8%, +2.0%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 145 ms | 145 ms | +0.2% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 336 ms | 336 ms | +0.1% | [-0.7%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 184 ms | 183 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 81.4 ms | 81.3 ms | -0.1% | [-0.8%, +0.8%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 168 ms | 167 ms | -0.7% | [-1.3%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 411 ms | 412 ms | +0.0% | [-0.8%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 88.7 ms | 88.8 ms | +0.2% | [-1.8%, +1.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.03 ms | 7.02 ms | -0.2% | [-0.6%, +1.6%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 159 ms | 159 ms | -0.2% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 361 ms | 361 ms | +0.2% | [+0.1%, +2.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 192 ms | 192 ms | +0.1% | [-0.5%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.5 ms | 11.6 ms | +1.6% | [+0.6%, +2.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 124 ms | 124 ms | -0.2% | [-0.7%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 536 ms | 538 ms | +0.3% | [-0.8%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 152 ms | -0.2% | [-0.5%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 861 ms | 859 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 120 ms | 120 ms | -0.0% | [-0.4%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 510 ms | 510 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 361 ms | 361 ms | -0.1% | [-0.9%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.1 ms | 15.0 ms | -0.8% | [-1.8%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 128 ms | 128 ms | -0.0% | [-0.9%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 560 ms | 564 ms | +0.6% | [+0.0%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 156 ms | 157 ms | +0.4% | [-0.8%, +5.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 956 ms | 959 ms | +0.2% | [-0.1%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 121 ms | 121 ms | +0.0% | [-0.6%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 518 ms | 516 ms | -0.3% | [-0.4%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 366 ms | 365 ms | -0.5% | [-1.5%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.7 ms | 68.3 ms | +0.9% | [-0.3%, +2.7%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 111 ms | 112 ms | +0.8% | [-1.7%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 268 ms | 268 ms | -0.0% | [-0.5%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 112 ms | 112 ms | +0.1% | [-0.4%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 106 ms | 106 ms | +0.2% | [-0.4%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 108 ms | 108 ms | -0.0% | [-0.5%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 237 ms | 237 ms | -0.0% | [-0.3%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 193 ms | 192 ms | -0.2% | [-2.0%, +0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 80.8 ms | 80.5 ms | -0.3% | [-0.5%, +1.6%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 127 ms | 127 ms | +0.3% | [-1.2%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 305 ms | 305 ms | -0.0% | [-0.0%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 129 ms | 128 ms | -0.2% | [-1.4%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 123 ms | 123 ms | +0.3% | [-0.7%, +1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 118 ms | 120 ms | +1.1% | [-0.7%, +15.3%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 255 ms | 255 ms | +0.0% | [-0.8%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 201 ms | 200 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.84 ms | 8.84 ms | -0.0% | [-1.3%, +1.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 143 ms | 143 ms | -0.3% | [-0.4%, +4.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 401 ms | 411 ms | +2.7% | [-2.7%, +3.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 256 ms | 255 ms | -0.7% | [-1.4%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 333 ms | 334 ms | +0.2% | [-0.2%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 141 ms | 141 ms | +0.0% | [-4.1%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 388 ms | 376 ms | -3.1% | [-3.3%, +3.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 349 ms | 349 ms | +0.0% | [-2.2%, +2.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.5 ms | 12.5 ms | -0.2% | [-1.3%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 147 ms | 153 ms | +4.2% | [-3.9%, +4.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 423 ms | 424 ms | +0.3% | [-2.7%, +3.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 291 ms | 292 ms | +0.2% | [-0.4%, +2.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 376 ms | 377 ms | +0.1% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 142 ms | 142 ms | +0.2% | [-5.3%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 387 ms | 386 ms | -0.3% | [-3.4%, +3.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 347 ms | 347 ms | -0.0% | [-3.0%, +3.0%] | - | ~ |  |

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: ecb6340
base-fingerprint: f044c9e490dd5cd0f20636476f57f2d048418e9f
head: 4eaf9bf
head-fingerprint: fbf75b59076b6fbcc20ee17ec308dd941ac09f5d
cases: 60
rounds: 3
reps: 5
date: 2026-10-23
-->

## Device comparison (CUDA): green

**Accepted:** -

`4eaf9bf` against `ecb6340` (origin/dev), 2026-10-23.

**VERDICT: no regression (0 major, 0 minor across 60 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 60 cases

- base: ecb6340 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 4eaf9bf (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/query-morton/float/hostdevice` | 417 µs | 418 µs | +0.1% | [-0.9%, +0.6%] | - | ~ | device 13.0x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 925 µs | 925 µs | -0.0% | [-0.4%, +0.1%] | - | ~ | device 8.6x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.41 ms | 4.39 ms | -0.4% | [-0.6%, +0.8%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.55 ms | 4.55 ms | -0.0% | [-0.2%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 690 µs | 692 µs | +0.2% | [-1.0%, +0.6%] | - | ~ | device 10.6x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.60 ms | 1.61 ms | +0.4% | [-0.3%, +0.5%] | - | ~ | device 6.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.62 ms | 6.64 ms | +0.3% | [-0.2%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 6.89 ms | 6.87 ms | -0.4% | [-0.6%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 1.11 ms | 1.12 ms | +0.6% | [+0.2%, +0.7%] | - | ~ | device 11.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.64 ms | 2.65 ms | +0.5% | [+0.1%, +1.1%] | - | ~ | device 6.6x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.29 ms | 9.31 ms | +0.2% | [-0.5%, +0.7%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 9.89 ms | 9.86 ms | -0.3% | [-0.4%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.81 ms | 8.59 ms | -2.5% | [-4.7%, -0.9%] | - | ~ | device 11.1x faster than the host (was 10.8x) |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 21.6 ms | 18.8 ms | -13.0% | [-13.5%, -11.6%] | - | faster | device 6.6x faster than the host (was 5.5x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 157 ms | 159 ms | +1.5% | [+0.8%, +4.7%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 262 ms | 264 ms | +0.7% | [-0.0%, +1.4%] | - | ~ | device 0.5x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 28.4 ms | 28.4 ms | -0.1% | [-1.0%, +1.1%] | - | ~ | device 3.5x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 67.1 ms | 67.1 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.1x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 153 ms | 152 ms | -0.4% | [-4.7%, +4.4%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 228 ms | 229 ms | +0.5% | [-2.8%, +3.6%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 832 µs | 830 µs | -0.3% | [-0.3%, +0.1%] | - | ~ | device 9.6x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 3.11 ms | 3.11 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.17 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 11.0 ms | 11.0 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 6.46 ms | 6.45 ms | -0.2% | [-0.8%, +0.5%] | - | ~ | device 2.6x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 6.94 ms | 6.98 ms | +0.6% | [+0.3%, +1.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.71 ms | 7.69 ms | -0.2% | [-0.4%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | +0.0% | [-0.0%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 25.3 ms | 25.3 ms | -0.1% | [-0.4%, +0.0%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 14.9 ms | 14.9 ms | -0.2% | [-0.9%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.24 ms | 2.25 ms | +0.4% | [-1.2%, +1.1%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.50 ms | 5.50 ms | -0.0% | [-0.2%, +0.4%] | - | ~ | device 2.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 931 µs | 932 µs | +0.2% | [-0.3%, +0.3%] | - | ~ | device 9.2x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 11.5 ms | 11.6 ms | +0.2% | [-1.3%, +1.0%] | - | ~ | device 2.9x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 7.37 ms | 7.35 ms | -0.3% | [-2.0%, +0.3%] | - | ~ | device 3.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.34 ms | 8.47 ms | +1.6% | [-0.8%, +3.4%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 16.2 ms | 16.3 ms | +0.4% | [-0.3%, +1.0%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.10 ms | 4.09 ms | -0.3% | [-0.7%, +0.3%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 24.5 ms | 24.4 ms | -0.4% | [-1.0%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 16.9 ms | 16.9 ms | -0.3% | [-3.2%, +1.3%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.60 ms | 1.60 ms | -0.1% | [-1.0%, +0.4%] | - | ~ | device 7.9x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 6.36 ms | 6.34 ms | -0.3% | [-0.7%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.52 ms | 1.52 ms | +0.4% | [-0.1%, +1.2%] | - | ~ | device 5.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 5.36 ms | 5.34 ms | -0.3% | [-0.7%, +0.5%] | - | ~ | device 3.5x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 7.38 ms | 7.36 ms | -0.2% | [-0.4%, +0.4%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.3 ms | 13.2 ms | -0.4% | [-1.0%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 17.2 ms | 17.2 ms | -0.0% | [-0.6%, +0.4%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 7.76 ms | 7.71 ms | -0.6% | [-1.2%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 15.3 ms | 15.2 ms | -0.3% | [-0.5%, +0.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 18.8 ms | 18.8 ms | -0.4% | [-0.7%, +1.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 6.74 ms | 6.74 ms | +0.1% | [-0.3%, +0.7%] | - | ~ | device 2.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 18.3 ms | 18.3 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 2.10 ms | 2.10 ms | +0.2% | [-0.5%, +0.8%] | - | ~ | device 5.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.97 ms | 5.97 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 4.5x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 14.7 ms | 14.6 ms | -0.7% | [-1.1%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 34.4 ms | 34.6 ms | +0.6% | [-0.4%, +0.9%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.3 ms | 68.5 ms | +0.2% | [-2.3%, +0.2%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.7 ms | 18.7 ms | -0.2% | [-0.4%, +0.7%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 21.7 ms | 21.6 ms | -0.3% | [-0.7%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 41.8 ms | 42.0 ms | +0.6% | [-1.5%, +1.7%] | - | ~ | device 0.6x faster than the host |

<!-- END receipt cuda -->
