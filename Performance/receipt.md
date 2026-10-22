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
verdict: red
accepted:
base: d3d6d00
base-fingerprint: 5d486a59bfb4cdfdfbfe20bbd6c0c73f42a9e748
head: ed6c5e3
head-fingerprint: c942f64e5c8919494388090035fdf32ba2359f5e
cases: 106
rounds: 3
reps: 5
date: 2026-10-22
-->

## Host comparison: red

`ed6c5e3` against `d3d6d00` (origin/claude/renderer-examples), 2026-10-22.

**VERDICT: MAJOR regression in 4 cases

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 2
side: cuda
verdict: yellow
accepted:
base: d3d6d00
base-fingerprint: 5d486a59bfb4cdfdfbfe20bbd6c0c73f42a9e748
head: ed6c5e3
head-fingerprint: c942f64e5c8919494388090035fdf32ba2359f5e
cases: 76
rounds: 3
reps: 5
date: 2026-10-22
-->

## Device comparison (CUDA): yellow

`ed6c5e3` against `d3d6d00` (origin/claude/renderer-examples), 2026-10-22.

**VERDICT: no regression, but 2 checksums changed: check those answers (see Flagged above)**

Performance comparison: 0 major, 0 minor, 2 faster, 0 differ, 0 lost, 76 cases

- base: d3d6d00 (gcc 13.3.0, simd none, gpu CUDA)
- head: ed6c5e3 (gcc 13.3.0, simd none, gpu CUDA)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/query-morton/float/hostdevice` | 415 µs | 415 µs | -0.1% | [-1.6%, +2.9%] | - | ~ | device 13.4x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 932 µs | 936 µs | +0.5% | [-0.9%, +1.2%] | - | ~ | device 8.6x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.41 ms | 4.43 ms | +0.5% | [-4.2%, +3.8%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.55 ms | 4.54 ms | -0.1% | [-4.0%, +4.7%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 691 µs | 693 µs | +0.3% | [-0.1%, +1.2%] | - | ~ | device 10.7x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.59 ms | 1.60 ms | +0.2% | [+0.0%, +0.8%] | - | ~ | device 7.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.60 ms | 6.63 ms | +0.4% | [+0.0%, +0.5%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 6.86 ms | 6.86 ms | +0.0% | [-0.6%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 1.12 ms | 1.13 ms | +0.4% | [-0.1%, +1.5%] | - | ~ | device 11.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.64 ms | 2.63 ms | -0.3% | [-2.4%, +1.2%] | - | ~ | device 6.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.30 ms | 9.29 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 9.88 ms | 9.91 ms | +0.4% | [-0.1%, +0.8%] | - | ~ | device 1.8x faster than the host |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 9.23 ms | 9.24 ms | +0.1% | [-2.4%, +3.1%] | - | ~ | device 10.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 22.3 ms | 22.4 ms | +0.7% | [-1.3%, +4.9%] | - | ~ | device 5.3x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 140 ms | 140 ms | +0.1% | [-2.8%, +4.3%] | - | ~ | device 0.7x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 263 ms | 263 ms | +0.1% | [-1.9%, +3.2%] | - | ~ | device 0.5x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 28.8 ms | 28.7 ms | -0.2% | [-1.3%, +0.4%] | - | ~ | device 3.4x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 68.0 ms | 68.1 ms | +0.2% | [-0.5%, +0.5%] | - | ~ | device 2.1x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 137 ms | 137 ms | +0.2% | [-3.7%, +2.0%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 228 ms | 229 ms | +0.3% | [-3.6%, +4.5%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 782 µs | 784 µs | +0.2% | [-0.2%, +0.3%] | - | ~ | device 10.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 3.10 ms | 3.09 ms | -0.4% | [-0.5%, +0.1%] | - | ~ | device 4.1x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.16 ms | 1.17 ms | +0.4% | [+0.2%, +0.9%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 11.2 ms | 11.2 ms | +0.1% | [-0.2%, +0.6%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 6.62 ms | 6.60 ms | -0.3% | [-1.3%, +0.7%] | - | ~ | device 2.6x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 6.96 ms | 6.94 ms | -0.3% | [-2.7%, +1.4%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.82 ms | 7.71 ms | -1.4% | [-2.1%, +0.9%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 12.0 ms | -9.5% | [-13.3%, +10.5%] | - | ~ | device 0.9x faster than the host (was 0.8x) |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 25.5 ms | 25.6 ms | +0.4% | [-0.5%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 14.7 ms | 14.8 ms | +0.6% | [-1.0%, +2.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.02 ms | 2.03 ms | +0.5% | [-1.1%, +1.3%] | - | ~ | device 5.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.16 ms | 5.15 ms | -0.3% | [-0.5%, +1.1%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 859 µs | 861 µs | +0.2% | [-7.0%, +7.7%] | - | ~ | device 10.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 11.7 ms | 11.7 ms | -0.1% | [-1.3%, +0.4%] | - | ~ | device 2.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 7.23 ms | 7.39 ms | +2.1% | [-1.6%, +3.3%] | - | ~ | device 3.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 7.20 ms | 7.29 ms | +1.3% | [-13.6%, +15.6%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.9 ms | +0.9% | [-12.0%, +7.5%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 3.37 ms | 3.38 ms | +0.2% | [-1.3%, +1.3%] | - | ~ | device 2.6x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 24.8 ms | 24.8 ms | -0.1% | [-0.8%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 16.6 ms | 16.6 ms | -0.1% | [-0.9%, +1.2%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.49 ms | 1.48 ms | -0.8% | [-2.8%, +1.1%] | - | ~ | device 8.6x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 6.46 ms | 6.43 ms | -0.5% | [-1.6%, -0.0%] | - | ~ | device 2.9x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.37 ms | 1.35 ms | -0.9% | [-12.4%, +13.6%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 5.63 ms | 5.64 ms | +0.2% | [-4.0%, +3.4%] | - | ~ | device 3.3x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 7.53 ms | 7.57 ms | +0.4% | [+0.0%, +1.1%] | - | ~ | device 2.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.3 ms | 13.2 ms | -0.4% | [-10.9%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 15.5 ms | 15.5 ms | +0.5% | [-6.0%, +3.3%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 6.47 ms | 6.45 ms | -0.3% | [-1.4%, +1.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 15.4 ms | 15.4 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 18.5 ms | 18.4 ms | -0.5% | [-2.0%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 6.39 ms | 6.38 ms | -0.1% | [-1.9%, +0.2%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 18.1 ms | 18.1 ms | +0.0% | [-0.5%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.92 ms | 1.91 ms | -0.1% | [-0.9%, +0.7%] | - | ~ | device 6.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.75 ms | 5.76 ms | +0.1% | [-0.1%, +0.8%] | - | ~ | device 4.7x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 14.7 ms | 14.7 ms | +0.2% | [-0.9%, +0.8%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 28.8 ms | 28.8 ms | +0.2% | [-1.6%, +3.1%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 58.8 ms | 58.8 ms | +0.0% | [-1.9%, +3.2%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 15.5 ms | 15.4 ms | -0.9% | [-1.0%, +1.1%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 19.8 ms | 20.0 ms | +1.0% | [-2.7%, +2.6%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 39.8 ms | 39.6 ms | -0.4% | [-1.7%, +2.9%] | - | ~ | device 0.6x faster than the host |
| `Render/csg/firstHit-camera/float/hostdevice` | 2.67 ms | 528 µs | -80.2% | [-80.4%, -80.1%] | - | faster | device 1.9x faster than the host (was 0.5x)

<!-- END receipt cuda -->
