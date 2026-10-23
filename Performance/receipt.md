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
verdict: yellow
base: ecb6340
base-fingerprint: f044c9e490dd5cd0f20636476f57f2d048418e9f
head: 0f752c6
head-fingerprint: 1c1b52d5016c9a164c49a4ca1100daf4e1b5ddea
cases: 88
rounds: 3
reps: 5
date: 2026-10-23
-->

## Host comparison: yellow

**Accepted:** The one minor flag, PointCloudHashGrid/sphere/all1nn/double (+4.4%), is in code this PR does not touch (it changes only the triangle kernel), and a 10-round recheck measured it within noise (-0.1%, interval -2.4% to +0.3%). Accepted by the maintainer.

`0f752c6` against `ecb6340` (origin/dev), 2026-10-23.

**VERDICT: 1 minor regression

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: ecb6340
base-fingerprint: f044c9e490dd5cd0f20636476f57f2d048418e9f
head: 0f752c6
head-fingerprint: 1c1b52d5016c9a164c49a4ca1100daf4e1b5ddea
cases: 60
rounds: 3
reps: 5
date: 2026-10-23
-->

## Device comparison (CUDA): green

**Accepted:** -

`0f752c6` against `ecb6340` (origin/dev), 2026-10-23.

**VERDICT: no regression (0 major, 0 minor across 60 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 60 cases

- base: ecb6340 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 0f752c6 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/query-morton/float/hostdevice` | 395 µs | 393 µs | -0.5% | [-6.7%, +6.7%] | - | ~ | device 13.7x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 912 µs | 943 µs | +3.4% | [-1.6%, +6.4%] | - | ~ | device 8.4x faster than the host (was 8.7x) |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 3.66 ms | 3.66 ms | -0.1% | [-18.8%, +22.1%] | - | ~ | device 1.6x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.54 ms | 3.74 ms | -17.6% | [-18.2%, +20.6%] | - | ~ | device 2.3x faster than the host (was 1.9x) |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 712 µs | 702 µs | -1.4% | [-2.8%, +2.4%] | - | ~ | device 10.5x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.67 ms | 2.55 ms | +52.9% | [-0.4%, +70.8%] | - | ~ | device 4.4x faster than the host (was 6.7x) |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.62 ms | 6.63 ms | +0.2% | [-4.3%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 6.88 ms | 6.85 ms | -0.4% | [-8.0%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 1.15 ms | 1.15 ms | +0.4% | [-0.1%, +1.6%] | - | ~ | device 11.0x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.72 ms | 2.70 ms | -0.5% | [-1.0%, +0.5%] | - | ~ | device 6.4x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.28 ms | 9.31 ms | +0.3% | [-0.4%, +0.6%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 9.87 ms | 9.92 ms | +0.5% | [+0.1%, +0.7%] | - | ~ | device 1.8x faster than the host |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 9.10 ms | 8.75 ms | -3.8% | [-10.7%, +1.2%] | - | ~ | device 10.9x faster than the host (was 10.4x) |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 21.9 ms | 19.2 ms | -12.4% | [-19.1%, -11.9%] | - | faster | device 6.3x faster than the host (was 5.5x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 141 ms | 145 ms | +2.3% | [-7.2%, +6.7%] | - | ~ | device 0.7x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 267 ms | 262 ms | -2.0% | [-3.7%, +1.3%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshSDF/query-morton/float/hostdevice` | 28.7 ms | 28.9 ms | +0.4% | [-0.3%, +1.9%] | - | ~ | device 3.4x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 74.1 ms | 74.2 ms | +0.1% | [-5.2%, +3.3%] | - | ~ | device 1.9x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 140 ms | 139 ms | -0.3% | [-2.9%, +3.7%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 230 ms | 231 ms | +0.1% | [-1.4%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 787 µs | 780 µs | -0.9% | [-1.6%, +0.7%] | - | ~ | device 10.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 3.10 ms | 3.08 ms | -0.5% | [-1.3%, +1.8%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.17 ms | -0.1% | [-0.2%, +0.6%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 11.3 ms | 11.3 ms | -0.1% | [-0.1%, +0.8%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 6.58 ms | 6.60 ms | +0.3% | [-0.8%, +1.2%] | - | ~ | device 2.6x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 6.95 ms | 6.95 ms | +0.0% | [-0.6%, +3.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.72 ms | 7.74 ms | +0.2% | [-0.3%, +1.4%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | +0.0% | [-9.6%, +12.9%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 25.6 ms | 25.4 ms | -0.5% | [-1.9%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 14.8 ms | 14.9 ms | +1.0% | [-0.1%, +1.7%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.04 ms | 2.05 ms | +0.4% | [-0.7%, +1.9%] | - | ~ | device 5.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.16 ms | 5.17 ms | +0.3% | [-0.5%, +1.7%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 862 µs | 863 µs | +0.2% | [-0.7%, +0.8%] | - | ~ | device 9.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 11.7 ms | 11.5 ms | -1.6% | [-1.9%, +0.7%] | - | ~ | device 2.9x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 7.37 ms | 7.31 ms | -0.7% | [-1.9%, +1.1%] | - | ~ | device 3.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 7.27 ms | 7.72 ms | +6.2% | [-13.0%, +22.4%] | - | ~ | device 1.4x faster than the host (was 1.5x) |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 16.2 ms | 16.1 ms | -0.7% | [-9.8%, +18.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 3.38 ms | 3.39 ms | +0.3% | [-0.8%, +1.1%] | - | ~ | device 2.6x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 25.0 ms | 24.8 ms | -0.6% | [-1.0%, +8.6%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 16.9 ms | 16.9 ms | +0.2% | [-1.8%, +1.1%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.51 ms | 1.49 ms | -1.5% | [-4.1%, +1.2%] | - | ~ | device 8.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 6.41 ms | 6.39 ms | -0.2% | [-15.9%, +0.4%] | - | ~ | device 2.9x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.37 ms | 1.37 ms | +0.5% | [-1.0%, +11.4%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 5.58 ms | 5.59 ms | +0.3% | [-0.5%, +0.5%] | - | ~ | device 3.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 7.54 ms | 7.53 ms | -0.3% | [-1.6%, +0.0%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.2 ms | 12.1 ms | -8.0% | [-18.6%, +0.9%] | - | ~ | device 1.1x faster than the host (was 1.0x) |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 15.7 ms | 15.7 ms | +0.1% | [-4.9%, +3.5%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 6.45 ms | 6.46 ms | +0.3% | [-0.9%, +0.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 15.4 ms | 15.4 ms | -0.1% | [-0.5%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 18.6 ms | 18.5 ms | -0.6% | [-2.3%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 6.34 ms | 6.36 ms | +0.3% | [+0.1%, +32.7%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 18.3 ms | 18.3 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.92 ms | 1.92 ms | +0.1% | [-1.1%, +10.2%] | - | ~ | device 6.1x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.71 ms | 5.72 ms | +0.2% | [-4.2%, +5.6%] | - | ~ | device 4.7x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 14.7 ms | 14.7 ms | +0.0% | [-0.4%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 28.9 ms | 29.1 ms | +0.5% | [-2.2%, +2.5%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 58.8 ms | 58.8 ms | +0.0% | [-1.7%, +1.6%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 15.4 ms | 15.4 ms | -0.4% | [-1.4%, +1.2%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.0 ms | 20.0 ms | -0.3% | [-1.5%, +1.5%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 39.8 ms | 39.7 ms | -0.3% | [-0.5%, +1.2%] | - | ~ | device 0.6x faster than the host |

<!-- END receipt cuda -->
