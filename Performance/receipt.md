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
base: f8a5e86
base-fingerprint: 120383b61744256bc0008209191322a4c852373b
head: b98861b
head-fingerprint: 15816899ba51fec0b84d2a127f84fe70debfa842
cases: 198
rounds: 2
reps: 3
date: 2026-11-05
-->

## Host comparison: green

**Accepted:** -

`b98861b` against `f8a5e86` (origin/main), 2026-11-05.

**VERDICT: no regression (0 major, 0 minor across 198 cases)**

Performance comparison: 0 major, 0 minor, 2 faster, 0 differ, 0 lost, 198 cases

- base: f8a5e86 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: b98861b (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 331 ms | 330 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 512 ms | 513 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 347 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 563 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.7 ms | 41.5 ms | -0.6% | [-1.4%, +0.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.2 ms | 75.9 ms | +0.9% | [+0.5%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.5 ms | 55.3 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.2 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.9 ms | 65.2 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.9 ms | 54.7 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.4 ms | 60.4 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.2 ms | 72.4 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.6 ms | 74.9 ms | +0.3% | [-0.6%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.8 ms | 77.6 ms | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.4 ms | 66.7 ms | -1.1% | [-2.3%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 100 ms | +0.4% | [-0.3%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.5 ms | 82.9 ms | -0.7% | [-1.1%, -0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.5 ms | 19.4 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 44.6 ms | 45.1 ms | +1.2% | [+0.1%, +2.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.0 ms | 72.9 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.4 ms | 20.4 ms | -0.1% | [-1.4%, +1.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.8 ms | 45.7 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.1 ms | 81.9 ms | -0.2% | [-3.1%, +2.7%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.49 ms | 4.52 ms | +0.6% | [+0.1%, +1.0%] | - | ~ | tape/fused 2.85x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | -0.3% | [-0.9%, +0.2%] | - | ~ | tape/fused 2.02x |
| `Fused/chain16/signedDistance/float/host` | 972 µs | 969 µs | -0.3% | [-2.2%, +1.7%] | - | ~ | tape/fused 16.46x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.0 ms | 46.0 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.2 ms | 46.2 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.57 ms | 4.55 ms | -0.5% | [-1.8%, +0.8%] | - | ~ | tape/fused 2.89x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | tape/fused 2.05x |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.88 ms | -0.3% | [-1.4%, +0.9%] | - | ~ | tape/fused 4.18x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.8 ms | 52.8 ms | -0.0% | [-1.8%, +1.8%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.0 ms | 53.0 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.9 ms | 81.7 ms | -0.2% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 229 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 112 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 260 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.0 ms | 69.9 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 217 ms | 214 ms | -1.3% | [-3.3%, +0.9%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.6 ms | 96.6 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.7 ms | 29.4 ms | -0.9% | [-1.4%, -0.3%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 756 ms | 758 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 117 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.3 ms | 43.3 ms | +0.0% | [-1.1%, +1.2%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 855 ms | 866 ms | +1.4% | [+0.2%, +2.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.7 ms | 26.7 ms | -0.1% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.8 ms | 27.1 ms | +0.9% | [+0.1%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 319 ms | +0.1% | [-0.0%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.6 ms | 90.3 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ | tape/fused 3.53x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.6 ms | 99.7 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.6 ms | 29.8 ms | +0.6% | [-0.3%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.0 ms | 29.8 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 361 ms | 359 ms | -0.5% | [-2.6%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 259 ms | 254 ms | -1.9% | [-3.8%, +0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.0 ms | 96.9 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ | tape/fused 3.71x |
| `MeshCSG/smooth-fused/frame/double/host` | 108 ms | 107 ms | -1.5% | [-2.6%, -0.4%] | +0.0% | ~ | tape/fused 2.39x |
| `MeshReconcile/armadillo/reconcile/float/host` | 131 ms | 131 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.5 ms | 41.3 ms | -0.5% | [-1.5%, +0.5%] | +0.0% | ~ | reconcile/scheduled 3.17x |
| `MeshReconcile/armadillo/reconcile/double/host` | 154 ms | 156 ms | +1.1% | [+0.5%, +1.8%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.6 ms | 56.6 ms | +0.2% | [-1.3%, +1.6%] | +0.0% | ~ | reconcile/scheduled 2.75x |
| `PointCloudBVH/uniform/build/float/host` | 53.2 ms | 53.0 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 144 ms | +0.4% | [-0.5%, +1.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 353 ms | 353 ms | -0.0% | [-0.5%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 79.4 ms | 79.4 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.66 ms | 6.68 ms | +0.4% | [-0.8%, +1.5%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.8 ms | 55.7 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 155 ms | 155 ms | -0.1% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 385 ms | 385 ms | +0.0% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.2 ms | 87.0 ms | +0.8% | [-0.2%, +1.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.85 ms | 6.89 ms | +0.6% | [+0.3%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.5 ms | 13.6 ms | +1.0% | [-0.0%, +2.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 117 ms | -0.2% | [-0.3%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 475 ms | 476 ms | +0.2% | [+0.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 142 ms | 143 ms | +0.6% | [+0.1%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 800 ms | 800 ms | +0.0% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.8 ms | 14.8 ms | -0.1% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | +0.1% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 493 ms | 493 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 144 ms | -0.2% | [-0.4%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 790 ms | 790 ms | -0.1% | [-0.5%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.5 ms | 54.3 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 128 ms | 128 ms | -0.1% | [-0.2%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 262 ms | 259 ms | -1.1% | [-1.5%, -0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 147 ms | 148 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 133 ms | 133 ms | +0.1% | [-0.0%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.3 ms | 57.1 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 139 ms | 139 ms | +0.1% | [-0.8%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 291 ms | 291 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 162 ms | 162 ms | -0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 148 ms | 148 ms | +0.1% | [-0.7%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.7 ms | 11.6 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 116 ms | 115 ms | -0.8% | [-1.2%, -0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 318 ms | 318 ms | +0.0% | [-0.9%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 156 ms | +0.8% | [+0.2%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 320 ms | 320 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.5 ms | 13.4 ms | -0.9% | [-2.2%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 115 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 347 ms | 348 ms | +0.1% | [-0.4%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 169 ms | 165 ms | -2.5% | [-3.8%, -1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 308 ms | -0.2% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.1 ms | 35.1 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.1 ms | 38.1 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.22 ms | 8.28 ms | +0.8% | [-0.8%, +2.5%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.28 ms | 9.31 ms | +0.4% | [-1.0%, +1.7%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.5 ms | 46.6 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.5 ms | 45.5 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.7 ms | +0.7% | [-0.1%, +1.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.7 ms | 78.6 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.5 ms | 65.8 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.86 ms | 5.84 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | tape/fused 1.38x |
| `Ray/csg-tape/firstHit/float/host` | 7.97 ms | 8.06 ms | +1.1% | [+0.1%, +2.0%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.2 ms | 52.4 ms | +0.4% | [+0.0%, +0.7%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.1 ms | 51.1 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.8 ms | 45.1 ms | +0.7% | [+0.5%, +0.9%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.4 ms | 87.8 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.9 ms | 73.7 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.98 ms | 5.96 ms | -0.5% | [-1.6%, +0.7%] | - | ~ | tape/fused 1.40x |
| `Ray/csg-tape/firstHit/double/host` | 8.32 ms | 8.34 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.4 ms | 18.2 ms | -0.9% | [-2.5%, +0.7%] | - | ~ | marched/traced 6.55x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 119 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.8 ms | 22.4 ms | +2.5% | [+0.6%, +4.3%] | +0.0% | ~ | marched/traced 6.89x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 154 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.2 ms | 35.2 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.2 ms | 19.4 ms | +0.8% | [-5.7%, +7.6%] | - | ~ | marched/traced 6.50x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 126 ms | -0.3% | [-0.5%, -0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.6 ms | 23.5 ms | -0.2% | [-2.0%, +1.6%] | +0.0% | ~ | marched/traced 6.79x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 161 ms | 160 ms | -0.9% | [-1.5%, -0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.9 ms | 37.9 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.2 ms | 46.4 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.4 ms | 46.2 ms | -0.3% | [-0.4%, -0.3%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.3 ms | 10.3 ms | -0.9% | [-3.8%, +2.0%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.71 ms | 8.78 ms | +0.9% | [+0.8%, +1.0%] | +0.0% | ~ | hit/crossing 1.17x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 181 ms | 181 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 160 ms | 160 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.8 ms | 68.6 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.3 ms | 56.3 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.4 ms | 52.3 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.3 ms | 52.3 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.1 ms | 11.1 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.63 ms | 9.42 ms | -2.2% | [-4.9%, +0.8%] | +0.0% | ~ | hit/crossing 1.17x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 207 ms | 207 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 185 ms | 184 ms | -0.5% | [-0.6%, -0.3%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.8 ms | 79.3 ms | +1.9% | [+0.4%, +3.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 66.2 ms | 66.2 ms | +0.1% | [-0.8%, +0.9%] | +0.0% | ~ | hit/crossing 1.20x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.0 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ | marched/traced 4.69x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.7 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 7.99 ms | 7.99 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.93 ms | 1.92 ms | -0.4% | [-2.8%, +2.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.91 ms | 6.94 ms | +0.5% | [-0.3%, +1.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 12.0 ms | +2.9% | [-0.2%, +6.0%] | +0.0% | ~ | marched/traced 5.07x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.9 ms | 61.0 ms | +0.1% | [-0.9%, +1.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.61 ms | 8.68 ms | +0.8% | [-0.4%, +2.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.94 ms | 1.95 ms | +0.9% | [+0.4%, +1.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.20 ms | 7.18 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 240 ms | 239 ms | -0.4% | [-1.0%, +0.1%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 328 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 237 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 251 ms | 251 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ | marched/traced 1.48x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 373 ms | 372 ms | -0.2% | [-1.6%, +1.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 249 ms | 251 ms | +0.6% | [-0.3%, +1.4%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.7 ms | 23.8 ms | +0.4% | [-0.4%, +1.1%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 24.9 ms | 23.6 ms | -5.2% | [-5.6%, -4.8%] | +0.0% | faster |  |
| `Render/mesh/frame/float/host` | 18.5 ms | 18.8 ms | +1.2% | [-0.3%, +2.7%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.0 ms | 29.9 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 29.9 ms | 30.0 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.2 ms | 20.4 ms | -3.7% | [-7.0%, -0.3%] | +0.0% | faster |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.8 ms | 56.6 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.7 ms | 36.8 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.2 ms | 92.1 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.4 ms | 64.1 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.5 ms | 42.7 ms | +0.5% | [+0.4%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 107 ms | 108 ms | +0.4% | [-0.3%, +1.0%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | - | - | - | - | - | new |  |
| `SphereContacts/polydisperse/force-morton/double/host` | - | - | - | - | - | new |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.55 ms | 4.52 ms | -0.7% | [-1.8%, +0.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.9 ms | 24.9 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.2 ms | 40.2 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.73 ms | 1.75 ms | +0.8% | [-0.2%, +1.8%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 138 µs | 138 µs | -0.1% | [-0.7%, +0.5%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.0 ms | 29.9 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.68 ms | +0.3% | [+0.1%, +0.6%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.1 ms | 16.0 ms | -0.7% | [-1.6%, +0.1%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.65 ms | 4.63 ms | -0.4% | [-1.3%, +0.5%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.2 ms | +0.5% | [-0.1%, +1.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.62 ms | 4.59 ms | -0.6% | [-1.5%, +0.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.3 ms | 26.2 ms | -0.4% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.1 ms | 42.1 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.74 ms | 1.73 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 277 µs | 263 µs | -4.9% | [-9.7%, +0.5%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.7 ms | 29.7 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.75 ms | 7.73 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.6 ms | 16.2 ms | -2.1% | [-5.4%, +1.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.77 ms | 7.73 ms | -0.5% | [-0.9%, -0.1%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/SphereContacts/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: f8a5e86
base-fingerprint: 120383b61744256bc0008209191322a4c852373b
head: b98861b
head-fingerprint: 15816899ba51fec0b84d2a127f84fe70debfa842
cases: 290
rounds: 2
reps: 3
date: 2026-11-05
-->

## Device comparison (CUDA): yellow

**Accepted:** Noise: all five flagged cases are within noise over 10 rounds (+0.4%, +0.6%, +2.4% [-2.3%, +6.3%], +0.0%, -1.2%); the PR does not touch the device ray or mesh code.

`b98861b` against `f8a5e86` (origin/main), 2026-11-05.

**VERDICT: 5 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 5 minor, 2 faster, 0 differ, 0 lost, 290 cases

- base: f8a5e86 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: b98861b (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.62 ms | +1.2% | [+0.4%, +2.0%] | - | ~ | device 22.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.41 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.03 ms | 1.07 ms | +3.2% | [+0.6%, +5.8%] | - | minor | device 19.2x faster than the host (was 19.8x) |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.30 ms | 2.30 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.22 ms | 6.11 ms | -1.8% | [-3.4%, -0.2%] | - | ~ | device 7.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.8 ms | 11.7 ms | -0.9% | [-1.7%, -0.0%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.4 ms | +0.3% | [-0.7%, +1.3%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 57.0 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.4 ms | 20.6 ms | +1.1% | [+0.5%, +1.6%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.0 ms | 47.9 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 62.3 ms | 61.3 ms | -1.6% | [-2.5%, -0.6%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 110 ms | 111 ms | +0.5% | [-1.2%, +2.2%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.28 ms | 8.36 ms | +1.0% | [+0.8%, +1.2%] | +0.0% | ~ | device 4.9x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.94 ms | 9.85 ms | -0.9% | [-2.9%, +1.1%] | +0.0% | ~ | device 8.2x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 387 µs | 385 µs | -0.5% | [-0.8%, -0.2%] | - | ~ | device 15.2x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 840 µs | 838 µs | -0.3% | [-0.3%, -0.2%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.50 ms | 4.52 ms | +0.5% | [+0.4%, +0.7%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.68 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 582 µs | 579 µs | -0.6% | [-0.8%, -0.3%] | - | ~ | device 13.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.36 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.76 ms | 6.76 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.04 ms | 7.01 ms | -0.5% | [-0.8%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 902 µs | 896 µs | -0.6% | [-1.4%, +0.3%] | - | ~ | device 14.5x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.46 ms | 9.46 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 320 µs | 320 µs | -0.0% | [-0.4%, +0.3%] | - | ~ | device 11.5x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 5.1x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.2% | [-0.0%, +0.3%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.54 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 523 ms | 523 ms | +0.1% | [-0.8%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.0% | [-0.7%, +0.7%] | - | ~ | device 23.7x faster than the host; tape/fused 4.53x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.6 µs | 50.0 µs | +0.7% | [+0.5%, +0.9%] | - | ~ | device 43.6x faster than the host; tape/fused 7.22x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.3 µs | 48.0 µs | -0.5% | [-1.0%, -0.1%] | - | ~ | device 9.0x faster than the host; tape/fused 9.87x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.4 ms | +0.7% | [-0.7%, +2.0%] | - | ~ | device 6.7x faster than the host; tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.3 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 811 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 1.5x faster than the host; tape/fused 1.05x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 282 ms | 288 ms | +2.3% | [+0.2%, +4.4%] | - | ~ | device 0.4x faster than the host (was 0.4x); tape/fused 0.98x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 281 ms | 281 ms | +0.1% | [-2.6%, +2.9%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.10 ms | 8.21 ms | +1.3% | [+0.5%, +2.2%] | +0.0% | ~ | device 10.8x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.25 ms | 8.27 ms | +0.2% | [-0.5%, +1.0%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.22 ms | 8.33 ms | +1.4% | [+0.4%, +2.4%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.23 ms | 8.20 ms | -0.3% | [-1.9%, +1.2%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 689 µs | 688 µs | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 4.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.72 ms | 1.73 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.07 ms | 1.06 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ | device 3.1x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 800 µs | 797 µs | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ | device 4.1x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 742 µs | 742 µs | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 831 µs | 834 µs | +0.4% | [+0.0%, +0.9%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 743 µs | 742 µs | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 736 µs | 731 µs | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 161 ms | 159 ms | -1.3% | [-4.0%, +1.5%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 162 ms | 162 ms | -0.2% | [-2.8%, +2.6%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 160 ms | 163 ms | +1.3% | [-2.6%, +5.3%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 160 ms | 164 ms | +2.4% | [-0.7%, +5.5%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.97x (was 1.00x) |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.76 ms | 1.76 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.91x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.96 ms | 1.95 ms | -0.1% | [-1.0%, +0.8%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.82x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.69 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.23 ms | 7.25 ms | +0.3% | [-0.0%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.30 ms | 7.24 ms | -0.8% | [-1.5%, +0.0%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.22 ms | 7.26 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.30 ms | 7.28 ms | -0.3% | [-1.1%, +0.4%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.43 ms | 8.45 ms | +0.3% | [-0.7%, +1.3%] | - | ~ | device 11.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.5 ms | 16.6 ms | +0.4% | [-0.3%, +1.1%] | - | ~ | device 8.2x faster than the host (was 7.1x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 172 ms | 166 ms | -3.3% | [-6.0%, -0.5%] | - | faster | device 0.6x faster than the host (was 0.5x) |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 278 ms | 287 ms | +3.2% | [+2.4%, +4.0%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `MeshSDF/query-morton/float/hostdevice` | 24.4 ms | 24.5 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.7 ms | 58.6 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 159 ms | 159 ms | +0.1% | [-4.1%, +4.5%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 246 ms | 238 ms | -3.2% | [-4.4%, -1.9%] | - | faster | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.94 ms | 2.89 ms | -1.7% | [-3.0%, -0.4%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.13 ms | 5.15 ms | +0.4% | [+0.0%, +0.7%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.4 ms | 14.5 ms | +0.7% | [-0.5%, +2.0%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 123 ms | 122 ms | -0.7% | [-2.6%, +1.3%] | - | ~ | device 1.8x faster than the host (was 1.6x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 173 ms | 172 ms | -0.2% | [-1.1%, +0.7%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 374 ms | 376 ms | +0.5% | [-1.3%, +2.3%] | - | ~ | device 2.4x faster than the host (was 2.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.7 ms | 24.7 ms | +0.0% | [-1.6%, +1.6%] | - | ~ | device 2.0x faster than the host; tape/fused 4.93x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 26.8 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 2.2x faster than the host; tape/fused 6.43x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.4 ms | 72.4 ms | +1.4% | [+0.6%, +2.1%] | - | ~ | device 4.5x faster than the host (was 4.2x); tape/fused 5.20x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.5 ms | 22.4 ms | -0.6% | [-2.0%, +0.9%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.5 ms | 44.8 ms | +0.7% | [-0.7%, +2.2%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 93.1 ms | 93.2 ms | +0.1% | [-1.5%, +1.6%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 550 ms | 558 ms | +1.3% | [-8.7%, +12.0%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 872 ms | 869 ms | -0.3% | [-1.9%, +1.4%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.21 s | 3.26 s | +1.5% | [-1.2%, +4.2%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 202 ms | 197 ms | -2.2% | [-5.6%, +1.4%] | - | ~ | device 0.2x faster than the host; tape/fused 2.83x (was 2.73x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 309 ms | 307 ms | -0.6% | [-3.4%, +2.2%] | - | ~ | device 0.2x faster than the host; tape/fused 2.84x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 587 ms | 574 ms | -2.4% | [-3.3%, -1.4%] | - | ~ | device 0.5x faster than the host; tape/fused 5.69x (was 5.47x) |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.65 ms | 2.66 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.32 ms | 5.32 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 10.3x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 675 µs | 677 µs | +0.4% | [-0.4%, +1.2%] | - | ~ | device 11.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.82 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 996 µs | 995 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 9.4x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.59 ms | 4.59 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.95 ms | 2.96 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.07 ms | 7.07 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.79 ms | 7.77 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | -0.2% | [-0.2%, -0.2%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.47 ms | 1.47 ms | +0.1% | [-3.2%, +3.5%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.90 ms | 3.88 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 525 µs | 523 µs | -0.3% | [-1.1%, +0.5%] | - | ~ | device 15.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.42 ms | 4.44 ms | +0.4% | [-0.6%, +1.5%] | - | ~ | device 6.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.35 ms | 4.34 ms | -0.3% | [-1.9%, +1.3%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.10 ms | 8.04 ms | -0.8% | [-0.9%, -0.6%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.7 ms | 15.8 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.00 ms | 4.02 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.7 ms | 17.6 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.57 ms | 1.58 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.56 ms | 4.57 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 833 µs | 829 µs | -0.5% | [-1.2%, +0.3%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.86 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.13 ms | 4.13 ms | +0.0% | [-0.7%, +0.8%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.5 ms | 16.5 ms | +0.5% | [-0.3%, +1.4%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.0 ms | 21.0 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.96 ms | 8.95 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.5 ms | +0.2% | [-0.0%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.1 ms | 20.5 ms | +2.1% | [+1.1%, +3.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.72 ms | 2.77 ms | +1.6% | [+0.1%, +3.2%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.73 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.71 ms | 2.72 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 7.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.51 ms | 6.50 ms | -0.3% | [-0.8%, +0.3%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.7 ms | 33.9 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.4 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.5 ms | +0.8% | [+0.5%, +1.0%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.7 ms | -0.6% | [-0.8%, -0.5%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.2 ms | 38.5 ms | +0.7% | [-0.2%, +1.5%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.7 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.67 ms | 9.65 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.9 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.31 ms | 3.28 ms | -0.8% | [-1.2%, -0.4%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.60 ms | 2.60 ms | -0.1% | [-1.7%, +1.5%] | +0.0% | ~ | device 3.4x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.59 ms | 4.56 ms | -0.7% | [-1.0%, -0.3%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.03 ms | 3.05 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 676 µs | 674 µs | -0.3% | [-1.4%, +0.8%] | - | ~ | device 16.8x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 14.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 646 µs | 637 µs | -1.4% | [-2.5%, -0.3%] | - | ~ | device 17.5x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | +0.0% | [-0.9%, +1.0%] | - | ~ | device 15.1x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 612 µs | 603 µs | -1.4% | [-1.5%, -1.3%] | - | ~ | device 16.1x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.11 ms | -0.4% | [-1.3%, +0.5%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | -0.1% | [-0.8%, +0.7%] | - | ~ | device 11.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.17 ms | 2.19 ms | +0.7% | [+0.4%, +1.0%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 613 µs | 610 µs | -0.4% | [-5.7%, +5.1%] | - | ~ | device 2.5x faster than the host (was 2.4x); tape/fused 1.13x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 727 µs | 696 µs | -4.3% | [-8.9%, +0.2%] | - | ~ | device 2.7x faster than the host (was 2.5x); tape/fused 1.21x (was 1.15x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 689 µs | 688 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 3.9x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 835 µs | 842 µs | +0.8% | [+0.3%, +1.4%] | - | ~ | device 3.7x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.1 ms | 17.1 ms | -0.1% | [-1.6%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.8% | [+0.3%, +1.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.6% | [+0.3%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.5 ms | +0.0% | [-1.2%, +1.3%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.4 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.1 ms | 15.0 ms | -1.0% | [-2.2%, +0.2%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 23.0 ms | -0.4% | [-1.2%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | +0.5% | [-0.4%, +1.3%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.61 ms | 2.60 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.70 ms | 2.73 ms | +0.8% | [+0.1%, +1.6%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.56 ms | 2.56 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.0x faster than the host; marched/traced 3.53x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 2.7x faster than the host; marched/traced 4.60x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.04 ms | 9.04 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.18 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.9x faster than the host; marched/traced 16.48x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.47 ms | 5.45 ms | -0.3% | [-0.3%, -0.3%] | - | ~ | device 2.8x faster than the host; marched/traced 13.15x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 35.8 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.6 ms | 71.6 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 1.8x faster than the host (was 1.7x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.25 ms | 5.23 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.79 ms | 8.84 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.21 ms | 6.27 ms | +0.9% | [-0.1%, +1.8%] | - | ~ | device 1.2x faster than the host; marched/traced 25.65x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 0.8x faster than the host; marched/traced 23.91x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 160 ms | 161 ms | +0.2% | [-2.0%, +2.4%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 255 ms | 253 ms | -0.8% | [-2.5%, +0.8%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.84 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 1.8x faster than the host; marched/traced 23.07x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.1 ms | 12.2 ms | +0.5% | [+0.3%, +0.8%] | - | ~ | device 1.3x faster than the host; marched/traced 17.93x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 111 ms | 112 ms | +0.5% | [-2.5%, +3.7%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 218 ms | 219 ms | +0.5% | [-1.6%, +2.6%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.2 ms | 19.3 ms | +0.3% | [-0.4%, +1.0%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.8 ms | +0.3% | [+0.0%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 679 µs | 678 µs | -0.2% | [-2.3%, +2.0%] | - | ~ | device 17.0x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.29 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 15.1x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 681 µs | 681 µs | -0.1% | [-1.3%, +1.1%] | - | ~ | device 16.9x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.29 ms | +0.9% | [+0.5%, +1.3%] | - | ~ | device 14.8x faster than the host; hit/crossing 0.99x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 687 µs | 687 µs | -0.0% | [-0.5%, +0.4%] | - | ~ | device 4.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 840 µs | 838 µs | -0.2% | [-0.6%, +0.3%] | - | ~ | device 4.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 629 µs | 623 µs | -1.0% | [-1.2%, -0.7%] | - | ~ | device 4.4x faster than the host; hit/crossing 1.10x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 647 µs | 645 µs | -0.3% | [-0.6%, -0.1%] | - | ~ | device 4.8x faster than the host; hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.71 ms | 4.71 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 10.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.18 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 10.1x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.09 ms | 9.10 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 266 ms | 267 ms | +0.3% | [-1.7%, +2.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 216 ms | 214 ms | -0.9% | [-2.9%, +1.1%] | - | ~ | device 0.9x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 55.1 ms | 55.0 ms | -0.2% | [-1.8%, +1.4%] | - | ~ | device 2.6x faster than the host; hit/crossing 4.85x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 70.4 ms | 71.0 ms | +0.7% | [+0.0%, +1.4%] | - | ~ | device 2.4x faster than the host (was 2.2x); hit/crossing 3.01x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.73 ms | 2.72 ms | -0.6% | [-1.6%, +0.4%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.28 ms | -0.7% | [-1.0%, -0.5%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.25x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.35 ms | 1.35 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 2.6x faster than the host; hit/crossing 2.01x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.7 ms | 64.6 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 121 ms | 124 ms | +2.2% | [+2.0%, +2.4%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.7 ms | 58.8 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 111 ms | 113 ms | +1.0% | [-0.7%, +2.7%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 477 ms | 493 ms | +3.3% | [+0.3%, +6.3%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 705 ms | 718 ms | +1.8% | [+0.4%, +3.3%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 389 ms | 387 ms | -0.4% | [-3.7%, +2.9%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.27x (was 1.23x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 532 ms | 549 ms | +3.3% | [+0.7%, +5.8%] | - | minor | device 0.3x faster than the host (was 0.3x); hit/crossing 1.31x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.59 ms | 3.61 ms | +0.5% | [-0.9%, +1.9%] | - | ~ | device 1.2x faster than the host; marched/traced 5.48x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.77 ms | 4.85 ms | +1.7% | [+1.0%, +2.4%] | - | ~ | device 0.9x faster than the host; marched/traced 8.29x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.8 ms | 19.8 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.1 ms | 40.2 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.24 ms | +0.5% | [-1.2%, +2.2%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.83 ms | 1.83 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.2 µs | 96.7 µs | +1.6% | [+0.6%, +2.6%] | - | ~ | device 14.6x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 151 µs | 154 µs | +1.7% | [-0.3%, +3.9%] | - | ~ | device 10.6x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 797 µs | 799 µs | +0.3% | [+0.2%, +0.4%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 934 µs | 934 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.0 ms | 35.9 ms | +2.8% | [+0.1%, +5.6%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 3.36x (was 3.50x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.5 ms | 36.7 ms | +0.6% | [-0.9%, +2.1%] | - | ~ | device 0.1x faster than the host; marched/traced 6.29x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 123 ms | 121 ms | -1.5% | [-2.1%, -1.0%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 234 ms | 231 ms | -1.2% | [-2.1%, -0.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.3 ms | 16.8 ms | +3.4% | [-0.7%, +7.8%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 22.0 ms | 21.8 ms | -0.7% | [-2.4%, +1.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.06 ms | 1.04 ms | -1.1% | [-6.3%, +4.3%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.38 ms | 2.38 ms | +0.1% | [-1.1%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.95 ms | 5.99 ms | +0.6% | [+0.5%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.71 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.1 ms | 23.3 ms | +1.2% | [+0.5%, +1.9%] | - | ~ | device 2.1x faster than the host; marched/traced 4.31x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.4 ms | +0.1% | [-1.1%, +1.3%] | - | ~ | device 1.9x faster than the host; marched/traced 5.59x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 99.7 ms | 100 ms | +0.7% | [-0.5%, +1.9%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 159 ms | 159 ms | -0.1% | [-2.0%, +1.9%] | - | ~ | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.7 ms | 13.6 ms | -0.5% | [-1.8%, +0.7%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.4 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 300 ms | 310 ms | +3.5% | [+0.7%, +6.4%] | - | minor | device 0.2x faster than the host (was 0.2x); marched/traced 1.85x (was 1.91x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 348 ms | 348 ms | -0.1% | [-2.8%, +2.6%] | - | ~ | device 0.2x faster than the host; marched/traced 2.85x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 574 ms | 574 ms | +0.1% | [-2.5%, +2.8%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.00 s | 991 ms | -1.4% | [-2.4%, -0.3%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 190 ms | 189 ms | -0.2% | [-1.6%, +1.3%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 239 ms | 237 ms | -1.1% | [-2.8%, +0.6%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.57 ms | 4.57 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.58 ms | 4.57 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.89 ms | 1.89 ms | +0.0% | [-0.7%, +0.7%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.96 ms | 7.98 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.94 ms | 7.93 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.1 ms | 23.4 ms | +1.0% | [+0.2%, +1.7%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 888 µs | 883 µs | -0.5% | [-1.4%, +0.4%] | +0.0% | ~ | device 10.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.64 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.1 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.0 ms | 13.0 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | +0.2% | [-0.4%, +0.9%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.1 ms | 11.0 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 217 µs | -0.0% | [-0.5%, +0.4%] | - | ~ | device 11.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 47.8 µs | +0.2% | [-0.0%, +0.4%] | - | ~ | device 23.1x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 361 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 12.6x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.02 ms | 2.02 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.6 µs | 72.8 µs | +0.3% | [-0.1%, +0.6%] | - | ~ | device 7.0x faster than the host (was 7.4x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.6 µs | 47.9 µs | +0.7% | [-0.4%, +1.7%] | - | ~ | device 4.8x faster than the host (was 5.0x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 652 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 11.9x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 474 µs | 474 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 48.2 µs | +0.4% | [-0.4%, +1.3%] | - | ~ | device 27.1x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 854 µs | 854 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.5x faster than the host (was 1.6x) |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.71 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 191 µs | +0.2% | [-0.0%, +0.4%] | - | ~ | device 3.0x faster than the host (was 2.8x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.8x faster than the host (was 2.1x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.77 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.0x faster than the host |


The probes differ between the sides (each side ran its own): Performance/SphereContacts/main.cpp

<!-- END receipt cuda -->
