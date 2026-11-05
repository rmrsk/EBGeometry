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
base: 03ed8f4
base-fingerprint: 15816899ba51fec0b84d2a127f84fe70debfa842
head: 87fe378
head-fingerprint: 796228c6b11b9d148268f59f3c375346281f1050
cases: 204
rounds: 2
reps: 3
date: 2026-11-05
-->

## Host comparison: yellow

**Accepted:** Accepted as noise

`87fe378` against `03ed8f4` (origin/main), 2026-11-05.

**VERDICT: 2 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 2 minor, 0 faster, 0 differ, 0 lost, 204 cases

- base: 03ed8f4 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 87fe378 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 331 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 514 ms | 513 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 348 ms | 347 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 564 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.9 ms | 41.5 ms | -0.9% | [-1.6%, -0.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.3 ms | 75.5 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.2 ms | 55.1 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.5 ms | 51.4 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.1 ms | 64.6 ms | -0.9% | [-1.3%, -0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 55.0 ms | 54.9 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.2 ms | 60.3 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.2 ms | 72.2 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.4 ms | 74.6 ms | +0.3% | [-0.6%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.7 ms | 78.0 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.8 ms | 66.7 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 100 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.9 ms | 83.1 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | -0.8% | [-1.9%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.6 ms | 19.4 ms | -1.1% | [-1.9%, -0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.6 ms | 45.6 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.0 ms | 73.1 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.2 ms | 20.3 ms | +0.4% | [-0.4%, +1.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.9 ms | 46.0 ms | +0.2% | [-0.9%, +1.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.7 ms | 81.2 ms | -1.8% | [-3.2%, -0.4%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.49 ms | 4.49 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/float/host` | 12.2 ms | 12.3 ms | +0.6% | [-0.3%, +1.6%] | - | ~ | tape/fused 2.04x |
| `Fused/chain16/signedDistance/float/host` | 968 µs | 967 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | tape/fused 16.76x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.2 ms | 46.1 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.3 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.55 ms | 4.55 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | tape/fused 2.89x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | tape/fused 2.05x |
| `Fused/chain16/signedDistance/double/host` | 3.90 ms | 3.87 ms | -0.7% | [-2.1%, +0.6%] | - | ~ | tape/fused 4.19x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.8 ms | 52.8 ms | +0.0% | [-0.8%, +0.9%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.9 ms | 52.9 ms | -0.1% | [-1.0%, +0.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.7 ms | 81.7 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 228 ms | 229 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 112 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 261 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 69.9 ms | 70.1 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 215 ms | -0.1% | [-1.2%, +1.0%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.9 ms | 96.3 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.4 ms | 29.7 ms | +0.9% | [+0.5%, +1.4%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 757 ms | 756 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 117 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.4 ms | 43.5 ms | +0.3% | [-0.9%, +1.5%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 860 ms | 856 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.7 ms | +0.1% | [+0.1%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.9 ms | 27.1 ms | +0.9% | [-0.0%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 320 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.6 ms | 90.8 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 100 ms | 101 ms | +0.8% | [-0.2%, +1.8%] | +0.0% | ~ | tape/fused 2.37x |
| `MeshCSG/union-tape/firstHit/double/host` | 30.1 ms | 29.7 ms | -1.4% | [-2.5%, -0.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.1 ms | 30.4 ms | +0.8% | [-1.5%, +3.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 362 ms | 364 ms | +0.7% | [-1.8%, +3.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 257 ms | 267 ms | +3.9% | [+3.0%, +4.8%] | +0.0% | minor |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 99.1 ms | 99.1 ms | -0.1% | [-3.9%, +4.0%] | +0.0% | ~ | tape/fused 3.68x |
| `MeshCSG/smooth-fused/frame/double/host` | 111 ms | 107 ms | -3.0% | [-3.6%, -2.4%] | +0.0% | ~ | tape/fused 2.49x (was 2.32x) |
| `MeshReconcile/armadillo/reconcile/float/host` | 132 ms | 131 ms | -0.7% | [-2.0%, +0.6%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.4 ms | 41.7 ms | +0.7% | [-0.4%, +1.9%] | +0.0% | ~ | reconcile/scheduled 3.16x |
| `MeshReconcile/armadillo/reconcile/double/host` | 156 ms | 160 ms | +2.4% | [-1.0%, +5.7%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.6 ms | 58.1 ms | +2.7% | [-1.1%, +6.5%] | +0.0% | ~ | reconcile/scheduled 2.75x |
| `PointCloudBVH/uniform/build/float/host` | 53.6 ms | 53.4 ms | -0.4% | [-1.3%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 144 ms | 144 ms | -0.1% | [-1.6%, +1.5%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 356 ms | 358 ms | +0.5% | [-0.9%, +1.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 80.1 ms | 79.5 ms | -0.7% | [-1.3%, -0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.70 ms | 6.68 ms | -0.3% | [-0.9%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.7 ms | 56.0 ms | +0.5% | [+0.4%, +0.7%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 157 ms | 156 ms | -0.6% | [-0.6%, -0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 387 ms | 387 ms | +0.1% | [-0.5%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.3 ms | 87.1 ms | -0.2% | [-0.8%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.86 ms | 6.84 ms | -0.3% | [-1.0%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.6 ms | 13.6 ms | -0.2% | [-2.1%, +1.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 119 ms | +0.3% | [-0.9%, +1.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 476 ms | 476 ms | +0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 142 ms | 142 ms | -0.0% | [-0.5%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 801 ms | 802 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.8 ms | 14.9 ms | +0.9% | [+0.0%, +1.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 121 ms | 120 ms | -0.2% | [-1.2%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 495 ms | 495 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 146 ms | 145 ms | -0.4% | [-1.9%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 793 ms | 793 ms | +0.0% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.6 ms | 54.6 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 128 ms | 130 ms | +1.3% | [-0.5%, +3.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 262 ms | 261 ms | -0.2% | [-1.7%, +1.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 148 ms | 148 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 134 ms | 133 ms | -0.2% | [-0.4%, +0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.4 ms | 57.2 ms | -0.5% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 139 ms | 145 ms | +4.0% | [+1.8%, +6.2%] | - | minor |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 293 ms | 291 ms | -0.7% | [-1.3%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 162 ms | 162 ms | -0.4% | [-1.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 152 ms | 150 ms | -1.5% | [-4.1%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.8 ms | 11.6 ms | -1.4% | [-2.6%, -0.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 115 ms | 116 ms | +1.1% | [+0.0%, +2.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 317 ms | 318 ms | +0.5% | [-0.1%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 155 ms | -0.2% | [-0.4%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 321 ms | 320 ms | -0.1% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.5 ms | 13.5 ms | +0.1% | [-0.6%, +0.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 115 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 347 ms | 348 ms | +0.5% | [+0.3%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 169 ms | 167 ms | -1.5% | [-2.4%, -0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 309 ms | -0.0% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.2 ms | 35.2 ms | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.2 ms | 38.2 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.28 ms | 8.28 ms | +0.0% | [-1.3%, +1.3%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.25 ms | 9.29 ms | +0.5% | [-0.3%, +1.3%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.6 ms | 46.4 ms | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ | hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.8 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.5 ms | 40.6 ms | +0.2% | [-0.0%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.7 ms | 79.8 ms | +1.4% | [-0.6%, +3.3%] | +0.0% | ~ | triangles/polygons 0.58x |
| `Ray/mesh-polygons/intersects/float/host` | 66.0 ms | 65.8 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.85 ms | 5.86 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | tape/fused 1.36x |
| `Ray/csg-tape/firstHit/float/host` | 8.09 ms | 7.99 ms | -1.3% | [-3.1%, +0.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.6 ms | 52.9 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.3 ms | 51.2 ms | -0.1% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.8 ms | 45.1 ms | +0.8% | [+0.5%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.3 ms | 88.5 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.9 ms | 74.0 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 6.37 ms | 6.00 ms | -5.9% | [-12.7%, +2.1%] | - | ~ | tape/fused 1.39x (was 1.31x) |
| `Ray/csg-tape/firstHit/double/host` | 8.33 ms | 8.33 ms | +0.1% | [-0.6%, +0.7%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.9 ms | 18.4 ms | -2.2% | [-5.0%, +0.7%] | - | ~ | marched/traced 6.47x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 119 ms | +0.1% | [+0.0%, +0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 22.0 ms | -0.3% | [-1.2%, +0.7%] | +0.0% | ~ | marched/traced 7.03x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 154 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.2 ms | 35.2 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.8 ms | 19.8 ms | -0.0% | [-1.0%, +0.9%] | - | ~ | marched/traced 6.39x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 127 ms | 127 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 23.7 ms | +1.0% | [-0.8%, +2.7%] | +0.0% | ~ | marched/traced 6.75x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 160 ms | 160 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.9 ms | 38.0 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.4 ms | 46.4 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.3 ms | 46.4 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.2 ms | 10.1 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.74 ms | 8.69 ms | -0.6% | [-1.0%, -0.1%] | +0.0% | ~ | hit/crossing 1.17x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 181 ms | 181 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 161 ms | 160 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.9 ms | 68.8 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.5 ms | 56.4 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.3 ms | 52.5 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.3 ms | 52.4 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.0 ms | 11.0 ms | -0.2% | [-1.5%, +1.2%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.40 ms | 9.34 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ | hit/crossing 1.18x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 208 ms | 207 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 185 ms | 185 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 78.3 ms | 78.5 ms | +0.3% | [-0.4%, +1.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 66.0 ms | 66.1 ms | +0.0% | [-1.9%, +1.9%] | +0.0% | ~ | hit/crossing 1.19x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.0 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ | marched/traced 4.73x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 52.1 ms | 51.9 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.01 ms | 8.02 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.91 ms | 1.91 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.93 ms | 6.92 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.6 ms | 11.6 ms | -0.5% | [-0.5%, -0.5%] | +0.0% | ~ | marched/traced 5.26x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.7 ms | 60.9 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.68 ms | 8.66 ms | -0.2% | [-1.1%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.94 ms | 1.98 ms | +2.2% | [+0.7%, +3.7%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.12 ms | 7.21 ms | +1.3% | [-0.4%, +3.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 238 ms | 239 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 328 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 237 ms | +0.0% | [-0.6%, +0.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 256 ms | 251 ms | -2.0% | [-4.7%, +0.8%] | +0.0% | ~ | marched/traced 1.48x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 374 ms | 370 ms | -1.2% | [-2.0%, -0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 249 ms | 249 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.7 ms | 24.2 ms | +1.9% | [+1.7%, +2.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.7 ms | 23.7 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 18.8 ms | +1.3% | [+0.8%, +1.9%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.4 ms | 30.3 ms | -0.2% | [-1.5%, +1.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 30.2 ms | 30.2 ms | -0.1% | [-2.7%, +2.7%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.3 ms | 20.4 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.8 ms | 56.6 ms | -0.4% | [-0.5%, -0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.6 ms | 36.6 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.8 ms | 92.1 ms | -0.7% | [-1.4%, +0.0%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.2 ms | 64.3 ms | +0.2% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.7 ms | 42.5 ms | -0.5% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 107 ms | 107 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | - | - | - | - | - | new | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | - | - | - | - | - | new |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | - | - | - | - | - | new | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | - | - | - | - | - | new |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 166 ms | 166 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 191 ms | 191 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 13.0 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.57 ms | 4.55 ms | -0.4% | [-0.7%, +0.0%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.9 ms | 25.1 ms | +0.8% | [+0.4%, +1.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.1 ms | 40.3 ms | +0.3% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.74 ms | 1.75 ms | +0.6% | [-1.1%, +2.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 138 µs | 138 µs | -0.1% | [-0.2%, +0.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.8 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.65 ms | -0.0% | [-0.2%, +0.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.1 ms | 16.2 ms | +0.7% | [+0.0%, +1.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.65 ms | 4.62 ms | -0.7% | [-2.0%, +0.5%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.2 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.54 ms | 4.60 ms | +1.3% | [-0.4%, +3.0%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.3 ms | 26.1 ms | -0.5% | [-0.6%, -0.4%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.3 ms | 42.3 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.76 ms | 1.74 ms | -1.0% | [-1.8%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 261 µs | 266 µs | +1.7% | [+0.1%, +3.3%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.9 ms | +0.2% | [-0.8%, +1.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.81 ms | 7.74 ms | -0.9% | [-1.5%, -0.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.2 ms | 16.2 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.75 ms | 7.74 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/SphereCloudRays/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 03ed8f4
base-fingerprint: 15816899ba51fec0b84d2a127f84fe70debfa842
head: 87fe378
head-fingerprint: 796228c6b11b9d148268f59f3c375346281f1050
cases: 302
rounds: 2
reps: 3
date: 2026-11-05
-->

## Device comparison (CUDA): yellow

**Accepted:** Accepted as noise

`87fe378` against `03ed8f4` (origin/main), 2026-11-05.

**VERDICT: 1 minor regression; rerun it with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 1 minor, 6 faster, 0 differ, 0 lost, 302 cases

- base: 03ed8f4 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 87fe378 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.60 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 22.8x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.41 ms | 3.40 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 18.7x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.03 ms | 1.06 ms | +2.1% | [-0.5%, +4.7%] | - | ~ | device 19.3x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.32 ms | 2.32 ms | +0.1% | [-1.4%, +1.7%] | - | ~ | device 16.5x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.19 ms | 6.24 ms | +0.8% | [-1.9%, +3.5%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.7 ms | 11.8 ms | +0.7% | [+0.2%, +1.2%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.1 ms | 33.5 ms | +1.1% | [+1.0%, +1.2%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 56.9 ms | 56.9 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.6 ms | 20.5 ms | -0.6% | [-2.3%, +1.2%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.0 ms | 47.9 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.8 ms | 61.8 ms | -0.0% | [-0.7%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 110 ms | 110 ms | -0.1% | [-1.9%, +1.7%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.20 ms | 8.25 ms | +0.6% | [-0.7%, +2.0%] | +0.0% | ~ | device 4.9x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.95 ms | 9.91 ms | -0.4% | [-2.0%, +1.1%] | +0.0% | ~ | device 8.1x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 385 µs | 386 µs | +0.3% | [-0.4%, +1.0%] | - | ~ | device 15.2x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 840 µs | 839 µs | -0.2% | [-0.8%, +0.4%] | - | ~ | device 10.2x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.53 ms | +0.3% | [-0.7%, +1.4%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.65 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 579 µs | 579 µs | -0.0% | [-0.6%, +0.5%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.36 ms | 1.36 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.76 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.02 ms | 7.01 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 895 µs | 899 µs | +0.5% | [-0.3%, +1.3%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.46 ms | 9.51 ms | +0.5% | [+0.3%, +0.8%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 320 µs | 319 µs | -0.4% | [-0.9%, +0.1%] | - | ~ | device 11.3x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 5.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.55 ms | 5.54 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 521 ms | 519 ms | -0.5% | [-1.0%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.8 µs | 48.8 µs | +1.9% | [+0.7%, +3.2%] | - | ~ | device 22.7x faster than the host (was 24.1x); tape/fused 4.46x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.7 µs | 49.9 µs | +0.5% | [+0.2%, +0.7%] | - | ~ | device 44.0x faster than the host; tape/fused 7.22x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.3% | [-0.4%, -0.2%] | - | ~ | device 9.7x faster than the host; tape/fused 9.90x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.2 ms | -0.3% | [-0.9%, +0.4%] | - | ~ | device 9.2x faster than the host; tape/fused 1.82x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.3 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 809 µs | 810 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.05x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.56 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 282 ms | 286 ms | +1.1% | [-0.7%, +3.0%] | - | ~ | device 0.4x faster than the host; tape/fused 1.02x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 287 ms | 290 ms | +1.1% | [-0.6%, +2.8%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.28 ms | 8.19 ms | -1.1% | [-1.9%, -0.2%] | +0.0% | ~ | device 10.8x faster than the host (was 12.8x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.34 ms | 8.17 ms | -2.0% | [-3.1%, -1.0%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.15 ms | 8.21 ms | +0.7% | [-0.6%, +2.0%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.21 ms | 8.15 ms | -0.7% | [-1.4%, -0.0%] | +0.0% | ~ | device 10.9x faster than the host (was 11.8x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 687 µs | 687 µs | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 5.5x faster than the host (was 4.8x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.73 ms | 1.72 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.06 ms | 1.06 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ | device 3.1x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 797 µs | 797 µs | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ | device 4.1x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 737 µs | 739 µs | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 833 µs | 842 µs | +1.1% | [+0.2%, +1.9%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 740 µs | 741 µs | +0.2% | [-0.5%, +1.0%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 728 µs | 727 µs | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 159 ms | 160 ms | +0.8% | [-0.6%, +2.2%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 162 ms | 160 ms | -0.7% | [-2.7%, +1.4%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 161 ms | 164 ms | +1.8% | [+1.2%, +2.4%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 162 ms | 161 ms | -0.9% | [-1.3%, -0.5%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.77 ms | 1.76 ms | -0.6% | [-1.4%, +0.2%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.91x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.94 ms | 1.95 ms | +0.7% | [+0.7%, +0.8%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.82x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.68 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.26 ms | 7.25 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.37 ms | 7.31 ms | -0.8% | [-1.7%, +0.1%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.23 ms | 7.24 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.27 ms | 7.26 ms | -0.1% | [-0.7%, +0.4%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.42 ms | 8.43 ms | +0.2% | [-0.8%, +1.2%] | - | ~ | device 11.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.5 ms | 16.7 ms | +1.5% | [+0.0%, +3.0%] | - | ~ | device 7.0x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 173 ms | 164 ms | -5.1% | [-5.7%, -4.6%] | - | faster | device 0.6x faster than the host (was 0.5x) |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 282 ms | 285 ms | +1.1% | [-1.1%, +3.3%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.6 ms | 24.6 ms | +0.2% | [-0.9%, +1.3%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.6 ms | 58.6 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 158 ms | 159 ms | +0.3% | [-3.9%, +4.9%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 242 ms | 248 ms | +2.4% | [+0.8%, +4.1%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.91 ms | 2.90 ms | -0.4% | [-2.0%, +1.3%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.17 ms | 5.14 ms | -0.4% | [-1.1%, +0.2%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.4 ms | 14.5 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 122 ms | 122 ms | -0.3% | [-2.9%, +2.4%] | - | ~ | device 1.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 176 ms | 174 ms | -0.9% | [-2.6%, +0.7%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 382 ms | 375 ms | -1.7% | [-4.1%, +0.7%] | - | ~ | device 2.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.9 ms | 24.9 ms | -0.3% | [-1.5%, +0.9%] | - | ~ | device 2.0x faster than the host; tape/fused 4.90x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.8 ms | 27.1 ms | +1.2% | [-0.2%, +2.6%] | - | ~ | device 2.2x faster than the host; tape/fused 6.42x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.4 ms | 71.7 ms | +0.5% | [-1.9%, +3.0%] | - | ~ | device 4.0x faster than the host; tape/fused 5.23x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.5 ms | 22.8 ms | +1.4% | [-0.0%, +2.9%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.9 ms | 44.1 ms | -1.8% | [-3.1%, -0.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 96.0 ms | 94.2 ms | -1.9% | [-4.7%, +1.0%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 559 ms | 551 ms | -1.5% | [-3.4%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 852 ms | 854 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.24 s | 3.28 s | +1.3% | [-1.2%, +3.7%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 195 ms | 198 ms | +1.4% | [-0.7%, +3.5%] | - | ~ | device 0.2x faster than the host; tape/fused 2.78x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 296 ms | 303 ms | +2.3% | [-0.7%, +5.3%] | - | ~ | device 0.2x faster than the host; tape/fused 2.82x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 587 ms | 594 ms | +1.2% | [-2.3%, +4.8%] | - | ~ | device 0.6x faster than the host; tape/fused 5.52x |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.65 ms | 2.66 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.33 ms | 5.33 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 10.4x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 676 µs | 681 µs | +0.8% | [+0.0%, +1.5%] | - | ~ | device 11.0x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.82 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 997 µs | 996 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 9.4x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.60 ms | 4.60 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.95 ms | 2.97 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.09 ms | 7.12 ms | +0.4% | [+0.1%, +0.8%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.78 ms | 7.79 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.45 ms | 1.45 ms | +0.1% | [-1.9%, +2.1%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.88 ms | 3.88 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 523 µs | 522 µs | -0.2% | [-0.5%, +0.0%] | - | ~ | device 15.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.40 ms | 4.42 ms | +0.5% | [-0.4%, +1.3%] | - | ~ | device 6.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.37 ms | 4.34 ms | -0.6% | [-1.0%, -0.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.05 ms | 8.07 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.7 ms | 15.8 ms | +0.6% | [+0.0%, +1.2%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.01 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.6 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.9 ms | 14.9 ms | +0.2% | [-0.8%, +1.3%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.57 ms | -0.5% | [-1.0%, +0.0%] | - | ~ | device 10.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.56 ms | 4.56 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 829 µs | 834 µs | +0.7% | [-0.1%, +1.4%] | - | ~ | device 10.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.85 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.13 ms | 4.15 ms | +0.6% | [+0.1%, +1.0%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.5 ms | 16.5 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.0 ms | 21.1 ms | +0.5% | [+0.0%, +0.9%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.93 ms | 8.95 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.6 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.3 ms | 20.2 ms | -0.7% | [-1.9%, +0.6%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.77 ms | 2.74 ms | -1.1% | [-1.6%, -0.6%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.74 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.39 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.72 ms | 2.72 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 7.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.51 ms | 6.51 ms | +0.0% | [-0.8%, +0.8%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.9 ms | 33.6 ms | -0.7% | [-1.7%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.6 ms | 68.1 ms | -0.7% | [-0.9%, -0.4%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.4 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.7 ms | -0.1% | [-1.0%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.2 ms | 38.2 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.68 ms | 9.65 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.9 ms | 12.8 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.30 ms | 3.32 ms | +0.6% | [-0.8%, +2.0%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.58 ms | 2.61 ms | +1.4% | [+1.0%, +1.8%] | +0.0% | ~ | device 3.4x faster than the host (was 3.6x) |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.57 ms | 4.57 ms | +0.1% | [-0.6%, +0.7%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.05 ms | 3.04 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 670 µs | 683 µs | +1.9% | [+1.8%, +2.0%] | - | ~ | device 16.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-1.1%, +1.0%] | - | ~ | device 14.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 637 µs | 647 µs | +1.5% | [+0.0%, +3.0%] | - | ~ | device 17.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.22 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 608 µs | 612 µs | +0.7% | [-1.1%, +2.6%] | - | ~ | device 15.9x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.11 ms | -0.6% | [-1.5%, +0.2%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.18 ms | -0.7% | [-1.2%, -0.2%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.58x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.4% | [+0.0%, +0.7%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.17 ms | 2.17 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 9.9x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 629 µs | 597 µs | -5.1% | [-5.9%, -4.2%] | - | faster | device 2.5x faster than the host (was 2.4x); tape/fused 1.15x (was 1.10x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 661 µs | 727 µs | +10.0% | [+9.4%, +10.5%] | - | minor | device 2.5x faster than the host (was 2.8x); tape/fused 1.15x (was 1.26x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 690 µs | 689 µs | -0.2% | [-0.2%, -0.1%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 834 µs | 836 µs | +0.2% | [-0.1%, +0.6%] | - | ~ | device 3.7x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.2% | [-0.7%, +1.1%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.7% | [-1.0%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.6% | [-1.4%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.4 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.4 ms | 16.6 ms | +1.2% | [+0.3%, +2.0%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.02x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.4 ms | 24.5 ms | +0.6% | [+0.4%, +0.7%] | - | ~ | device 1.1x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.5% | [-0.7%, +1.8%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 22.9 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.61 ms | 2.61 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 0.7x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.73 ms | 2.72 ms | -0.5% | [-0.9%, -0.2%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.56 ms | 2.56 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.0x faster than the host; marched/traced 3.52x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.7x faster than the host; marched/traced 4.59x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.03 ms | 9.03 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.18 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 3.9x faster than the host; marched/traced 16.48x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.44 ms | 5.44 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 2.8x faster than the host; marched/traced 13.14x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 35.9 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.6 ms | 71.5 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.24 ms | 5.24 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.83 ms | 8.80 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.20 ms | 6.24 ms | +0.7% | [-0.1%, +1.5%] | - | ~ | device 1.3x faster than the host; marched/traced 25.50x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.8x faster than the host; marched/traced 24.15x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 159 ms | +0.1% | [-3.7%, +4.0%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 259 ms | 256 ms | -0.9% | [-1.7%, -0.1%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.85 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 1.8x faster than the host; marched/traced 23.09x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.3x faster than the host; marched/traced 18.13x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 114 ms | 112 ms | -2.1% | [-2.7%, -1.5%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 216 ms | 221 ms | +2.1% | [+1.5%, +2.6%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.3 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.8 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 678 µs | 673 µs | -0.7% | [-0.8%, -0.7%] | - | ~ | device 17.2x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.4% | [+0.2%, +0.5%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 670 µs | 669 µs | -0.2% | [-0.5%, +0.2%] | - | ~ | device 17.3x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.3% | [-0.3%, +0.8%] | - | ~ | device 14.9x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 692 µs | 693 µs | +0.2% | [-0.2%, +0.5%] | - | ~ | device 4.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 839 µs | 839 µs | +0.1% | [-0.4%, +0.6%] | - | ~ | device 4.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 625 µs | 628 µs | +0.4% | [+0.1%, +0.6%] | - | ~ | device 4.4x faster than the host; hit/crossing 1.10x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 647 µs | 646 µs | -0.2% | [-0.4%, +0.1%] | - | ~ | device 4.8x faster than the host; hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.71 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 10.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.19 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 10.1x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.10 ms | 9.11 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 268 ms | 260 ms | -3.1% | [-4.5%, -1.7%] | - | faster | device 0.7x faster than the host (was 0.7x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 214 ms | 213 ms | -0.5% | [-4.1%, +3.2%] | - | ~ | device 0.9x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 54.8 ms | 54.8 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 2.6x faster than the host; hit/crossing 4.74x (was 4.89x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 71.1 ms | 71.1 ms | -0.1% | [-0.8%, +0.7%] | - | ~ | device 2.2x faster than the host (was 2.4x); hit/crossing 3.00x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.1 ms | 16.9 ms | -0.7% | [-0.9%, -0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.3 ms | 27.4 ms | +0.3% | [-0.3%, +0.8%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.61 ms | +0.2% | [-0.3%, +0.8%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.73 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.29 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.25x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.35 ms | 1.35 ms | +0.0% | [-0.7%, +0.7%] | - | ~ | device 2.6x faster than the host; hit/crossing 2.02x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.7 ms | 64.7 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 121 ms | 123 ms | +1.7% | [+0.7%, +2.6%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.7 ms | 58.8 ms | +0.3% | [+0.2%, +0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 111 ms | 111 ms | +0.1% | [-2.4%, +2.6%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 459 ms | 464 ms | +1.2% | [-1.1%, +3.5%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 714 ms | 711 ms | -0.4% | [-0.9%, +0.2%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 406 ms | 383 ms | -5.6% | [-9.1%, -1.9%] | - | faster | device 0.4x faster than the host (was 0.4x); hit/crossing 1.21x (was 1.13x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 531 ms | 533 ms | +0.4% | [-0.7%, +1.6%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.33x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.58 ms | 3.53 ms | -1.5% | [-2.7%, -0.3%] | - | ~ | device 1.2x faster than the host; marched/traced 5.63x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.81 ms | 4.81 ms | -0.2% | [-1.1%, +0.8%] | - | ~ | device 0.9x faster than the host; marched/traced 8.37x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.7 ms | 19.9 ms | +0.6% | [-0.3%, +1.6%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.2 ms | 40.2 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.25 ms | 1.23 ms | -0.9% | [-2.7%, +0.8%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.84 ms | -0.3% | [-1.7%, +1.1%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 94.7 µs | 94.7 µs | -0.0% | [-1.5%, +1.5%] | - | ~ | device 15.2x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 153 µs | 152 µs | -1.1% | [-1.5%, -0.6%] | - | ~ | device 11.0x faster than the host (was 10.5x) |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 798 µs | 801 µs | +0.3% | [+0.1%, +0.6%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 936 µs | 934 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.9 ms | 34.6 ms | -3.6% | [-5.8%, -1.4%] | - | faster | device 0.1x faster than the host (was 0.1x); marched/traced 3.56x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.6 ms | 36.9 ms | +0.7% | [-0.6%, +2.1%] | - | ~ | device 0.1x faster than the host; marched/traced 6.33x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 125 ms | 123 ms | -1.2% | [-2.9%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 232 ms | 234 ms | +0.7% | [-0.1%, +1.5%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.4 ms | 17.2 ms | +4.9% | [-2.8%, +12.6%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 21.9 ms | +0.3% | [-0.9%, +1.5%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.09 ms | 1.06 ms | -2.3% | [-3.4%, -1.1%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.35 ms | 2.36 ms | +0.3% | [-2.4%, +3.1%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.97 ms | 5.97 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.72 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.1 ms | 23.2 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 2.2x faster than the host; marched/traced 4.33x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.5 ms | 28.8 ms | +0.9% | [+0.3%, +1.4%] | - | ~ | device 1.9x faster than the host; marched/traced 5.45x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 100 ms | 100 ms | +0.1% | [-1.3%, +1.6%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 158 ms | 157 ms | -0.6% | [-2.1%, +0.9%] | - | ~ | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.7 ms | 13.8 ms | +0.3% | [+0.0%, +0.5%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.4 ms | -0.1% | [-1.1%, +0.8%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 302 ms | 308 ms | +2.1% | [-2.0%, +6.3%] | - | ~ | device 0.2x faster than the host; marched/traced 1.83x (was 1.89x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 350 ms | 356 ms | +1.6% | [+0.3%, +3.0%] | - | ~ | device 0.2x faster than the host; marched/traced 2.77x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 569 ms | 563 ms | -1.0% | [-4.4%, +2.7%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 987 ms | 985 ms | -0.3% | [-0.8%, +0.3%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 192 ms | 185 ms | -4.0% | [-5.9%, -2.0%] | - | faster | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 243 ms | 235 ms | -3.0% | [-4.5%, -1.5%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `Render/csg/frame/float/hostdevice` | 4.57 ms | 4.57 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.57 ms | 4.57 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.88 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.96 ms | 7.97 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.96 ms | 7.94 ms | -0.3% | [-0.9%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.4 ms | 23.1 ms | -1.0% | [-1.4%, -0.7%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 882 µs | 890 µs | +0.9% | [+0.3%, +1.5%] | +0.0% | ~ | device 10.4x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.65 ms | +0.4% | [+0.0%, +0.9%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.0 ms | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 13.9 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.1 ms | 11.1 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | - | - | - | - | - | new | tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | - | - | - | - | - | new | tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | - | - | - | - | - | new | tape/fused 0.99x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | - | - | - | - | - | new | tape/fused 1.05x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 10.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.1 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 0.6x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 217 µs | -0.3% | [-0.7%, +0.2%] | - | ~ | device 11.4x faster than the host (was 11.0x) |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 47.8 µs | -0.4% | [-0.8%, -0.1%] | - | ~ | device 23.6x faster than the host (was 24.4x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 360 µs | +0.2% | [-0.1%, +0.4%] | - | ~ | device 12.6x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.02 ms | 2.02 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.6 µs | 72.7 µs | +0.2% | [-0.1%, +0.6%] | - | ~ | device 7.2x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 47.8 µs | -0.4% | [-2.3%, +1.6%] | - | ~ | device 4.8x faster than the host (was 4.6x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 653 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.1% | [-0.2%, -0.1%] | - | ~ | device 11.6x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 474 µs | 474 µs | -0.0% | [-0.3%, +0.2%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 48.0 µs | -0.1% | [-0.5%, +0.3%] | - | ~ | device 26.5x faster than the host (was 27.3x) |
| `Tape/csg/signedDistance/double/hostdevice` | 853 µs | 854 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.71 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 190 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.0x faster than the host (was 3.3x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.2% | [+0.1%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.77 ms | 2.78 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |


The probes differ between the sides (each side ran its own): Performance/SphereCloudRays/main.cpp

<!-- END receipt cuda -->
