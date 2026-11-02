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
verdict: red
base: f1c1c40
base-fingerprint: 139b0db678361b912e9d5bbc988f0a34804c9ec8
head: def340e
head-fingerprint: 1759bc965c8f27541a01db9aadccd206bf503ecf
cases: 180
rounds: 2
reps: 3
date: 2026-11-02
-->

## Host comparison: red

**Accepted:** Yes. Accept them as noise.

`def340e` against `f1c1c40` (origin/main), 2026-11-02.

**VERDICT: MAJOR regression in 2 cases; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 2 major, 1 minor, 2 faster, 0 differ, 0 lost, 180 cases

- base: f1c1c40 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: def340e (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 345 ms | 345 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 185 ms | 185 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 537 ms | 537 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 367 ms | 367 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 194 ms | 194 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 591 ms | 593 ms | +0.3% | [-0.5%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 57.6 ms | 57.8 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 54.0 ms | 54.2 ms | +0.2% | [-1.1%, +1.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 67.9 ms | 67.7 ms | -0.2% | [-0.9%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 59.1 ms | 59.3 ms | +0.4% | [-0.4%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 63.7 ms | 63.2 ms | -0.9% | [-1.6%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 77.1 ms | 75.9 ms | -1.5% | [-2.2%, -0.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 77.2 ms | 77.8 ms | +0.8% | [+0.5%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 84.7 ms | 85.1 ms | +0.5% | [-1.5%, +2.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 69.4 ms | 69.4 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 109 ms | 108 ms | -1.3% | [-2.8%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 88.2 ms | 87.4 ms | -0.9% | [-1.9%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 128 ms | 125 ms | -2.4% | [-4.3%, -0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 20.3 ms | 20.2 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 46.8 ms | 46.0 ms | -1.6% | [-3.0%, -0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 79.0 ms | 78.8 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.9 ms | 20.9 ms | -0.1% | [-0.9%, +0.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 47.9 ms | 47.8 ms | -0.3% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 91.1 ms | 91.1 ms | +0.1% | [-2.5%, +2.7%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.56 ms | 4.62 ms | +1.4% | [-0.3%, +3.0%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/float/host` | 12.6 ms | 12.6 ms | -0.2% | [-1.2%, +0.9%] | - | ~ | tape/fused 2.06x |
| `Fused/chain16/signedDistance/float/host` | 1.01 ms | 999 µs | -1.1% | [-2.4%, +0.2%] | - | ~ | tape/fused 16.74x |
| `Fused/drilled-mesh/signedDistance/float/host` | 49.9 ms | 49.9 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 49.7 ms | 50.3 ms | +1.1% | [+0.4%, +1.8%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.67 ms | 4.71 ms | +1.0% | [-1.3%, +3.3%] | - | ~ | tape/fused 2.86x |
| `Fused/smooth/signedDistance/double/host` | 13.0 ms | 13.2 ms | +1.3% | [+0.1%, +2.5%] | - | ~ | tape/fused 2.09x |
| `Fused/chain16/signedDistance/double/host` | 4.01 ms | 4.02 ms | +0.2% | [-1.5%, +1.9%] | - | ~ | tape/fused 4.20x |
| `Fused/drilled-mesh/signedDistance/double/host` | 58.2 ms | 59.4 ms | +2.0% | [+1.1%, +2.9%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 59.9 ms | 59.7 ms | -0.4% | [-1.9%, +1.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 85.7 ms | 85.5 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 247 ms | 245 ms | -0.8% | [-1.7%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 117 ms | 117 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 293 ms | 293 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 73.7 ms | 73.2 ms | -0.6% | [-0.7%, -0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 233 ms | 231 ms | -1.1% | [-1.8%, -0.4%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 101 ms | 101 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 36.1 ms | 35.3 ms | -2.1% | [-3.4%, -0.7%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 893 ms | 880 ms | -1.4% | [-3.2%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 123 ms | 123 ms | +0.5% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 64.2 ms | 62.1 ms | -3.3% | [-5.9%, -0.5%] | - | faster |  |
| `MeshSDF/query-random/double/host` | 1.07 s | 1.07 s | -0.3% | [-2.3%, +1.8%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 28.6 ms | 28.5 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 28.0 ms | 28.3 ms | +1.0% | [+0.2%, +1.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 343 ms | 345 ms | +0.6% | [+0.6%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 249 ms | 250 ms | +0.4% | [+0.4%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 95.6 ms | 95.6 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | tape/fused 3.61x |
| `MeshCSG/smooth-fused/frame/float/host` | 104 ms | 105 ms | +0.4% | [+0.1%, +0.6%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 32.2 ms | 32.1 ms | -0.5% | [-1.3%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 31.8 ms | 31.7 ms | -0.3% | [-1.4%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 403 ms | 406 ms | +0.7% | [+0.1%, +1.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 272 ms | 271 ms | -0.2% | [-4.5%, +4.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 105 ms | 105 ms | -0.6% | [-3.6%, +2.4%] | +0.0% | ~ | tape/fused 3.88x |
| `MeshCSG/smooth-fused/frame/double/host` | 114 ms | 116 ms | +1.5% | [-0.5%, +3.4%] | +0.0% | ~ | tape/fused 2.35x |
| `PointCloudBVH/uniform/build/float/host` | 71.1 ms | 70.7 ms | -0.5% | [-1.5%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 170 ms | 172 ms | +0.7% | [+0.3%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 407 ms | 407 ms | -0.0% | [-0.5%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 84.9 ms | 85.0 ms | +0.2% | [-1.1%, +1.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.98 ms | 6.98 ms | +0.0% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 86.1 ms | 86.1 ms | -0.0% | [-1.3%, +1.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 185 ms | 185 ms | +0.2% | [-0.1%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 424 ms | 424 ms | +0.1% | [-0.0%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 97.4 ms | 117 ms | +20.1% | [+0.6%, +39.8%] | - | **MAJOR** |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.50 ms | 7.50 ms | -0.1% | [-2.3%, +2.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.2 ms | 11.2 ms | +0.1% | [-4.0%, +4.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 134 ms | 134 ms | +0.4% | [+0.3%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 566 ms | 567 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 161 ms | 160 ms | -0.7% | [-1.7%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 959 ms | 968 ms | +1.0% | [+0.2%, +1.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.7 ms | 15.7 ms | +0.2% | [-2.5%, +3.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 139 ms | 140 ms | +0.2% | [-0.5%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 592 ms | 593 ms | +0.2% | [-0.5%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 171 ms | 170 ms | -0.2% | [-1.6%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 1.06 s | 1.08 s | +1.2% | [-0.5%, +3.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 70.0 ms | 70.3 ms | +0.4% | [+0.0%, +0.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 127 ms | 128 ms | +0.4% | [+0.1%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 294 ms | 294 ms | -0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 120 ms | 121 ms | +0.9% | [+0.0%, +1.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 116 ms | 116 ms | -0.0% | [-1.2%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 84.9 ms | 86.0 ms | +1.3% | [+1.0%, +1.7%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 142 ms | 142 ms | -0.1% | [-1.1%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 316 ms | 316 ms | +0.3% | [-0.0%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 141 ms | 142 ms | +0.1% | [-0.7%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 138 ms | 139 ms | +0.9% | [+0.3%, +1.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 9.75 ms | 9.74 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 155 ms | 156 ms | +0.1% | [-3.8%, +4.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 429 ms | 430 ms | +0.2% | [-2.9%, +3.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 276 ms | 269 ms | -2.6% | [-4.8%, -0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 379 ms | 382 ms | +0.9% | [-0.1%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.7 ms | 13.5 ms | -1.0% | [-2.2%, +0.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 157 ms | 154 ms | -2.3% | [-4.6%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 447 ms | 440 ms | -1.6% | [-3.2%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 310 ms | 315 ms | +1.6% | [-0.3%, +3.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 411 ms | 410 ms | -0.3% | [-1.2%, +0.7%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 50.4 ms | 50.0 ms | -0.8% | [-1.0%, -0.5%] | +0.0% | ~ | hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled/float/host` | 49.3 ms | 49.4 ms | +0.2% | [-0.9%, +1.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 44.0 ms | 43.5 ms | -1.0% | [-1.4%, -0.6%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 94.3 ms | 92.6 ms | -1.8% | [-2.9%, -0.8%] | +0.0% | ~ | triangles/polygons 0.54x |
| `Ray/mesh-polygons/intersects/float/host` | 78.8 ms | 77.6 ms | -1.4% | [-2.7%, -0.2%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 6.01 ms | 5.99 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | tape/fused 1.35x |
| `Ray/csg-tape/firstHit/float/host` | 8.08 ms | 8.08 ms | -0.1% | [-1.0%, +0.8%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 57.6 ms | 58.3 ms | +1.2% | [+0.7%, +1.7%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 56.3 ms | 57.0 ms | +1.2% | [+0.9%, +1.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 49.8 ms | 49.6 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 110 ms | 109 ms | -1.2% | [-2.9%, +0.4%] | +0.0% | ~ | triangles/polygons 0.54x |
| `Ray/mesh-polygons/intersects/double/host` | 92.9 ms | 92.3 ms | -0.6% | [-1.7%, +0.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 6.10 ms | 6.12 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | tape/fused 1.41x |
| `Ray/csg-tape/firstHit/double/host` | 8.43 ms | 8.62 ms | +2.2% | [-0.5%, +4.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.9 ms | 18.6 ms | -1.9% | [-3.6%, -0.2%] | - | ~ | marched/traced 6.70x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 124 ms | 124 ms | -0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.7 ms | 22.6 ms | -0.4% | [-0.4%, -0.3%] | +0.0% | ~ | marched/traced 7.09x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 160 ms | 161 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 36.6 ms | 36.6 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.3 ms | 25.1 ms | +30.3% | [+7.0%, +53.7%] | - | **MAJOR** | marched/traced 5.21x (was 6.79x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 131 ms | 131 ms | -0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 24.2 ms | 24.3 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ | marched/traced 6.86x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 167 ms | 167 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 39.2 ms | 39.2 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 50.1 ms | 50.2 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 50.3 ms | 50.4 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.6 ms | 10.5 ms | -1.0% | [-2.3%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.93 ms | 8.90 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ | hit/crossing 1.18x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 198 ms | 198 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 179 ms | 177 ms | -1.1% | [-2.6%, +0.4%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 75.1 ms | 75.0 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 61.5 ms | 61.2 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ | hit/crossing 1.23x |
| `RayCrossing/mesh/firstHit/double/host` | 58.7 ms | 58.5 ms | -0.3% | [-1.7%, +1.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 59.1 ms | 58.8 ms | -0.5% | [-1.3%, +0.3%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.3 ms | 11.9 ms | +4.9% | [+0.0%, +9.9%] | +0.0% | minor |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.65 ms | 9.60 ms | -0.5% | [-0.9%, -0.0%] | +0.0% | ~ | hit/crossing 1.24x (was 1.18x) |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 231 ms | 231 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 206 ms | 207 ms | +0.8% | [-0.1%, +1.8%] | +0.0% | ~ | hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 88.0 ms | 88.3 ms | +0.3% | [-1.7%, +2.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 74.9 ms | 74.7 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ | hit/crossing 1.18x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.6 ms | 11.6 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | marched/traced 4.86x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 56.3 ms | 56.3 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.42 ms | 8.45 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.02 ms | 2.02 ms | -0.3% | [-1.2%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.08 ms | 7.03 ms | -0.7% | [-1.1%, -0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.4 ms | 12.5 ms | +0.8% | [-0.1%, +1.6%] | +0.0% | ~ | marched/traced 5.53x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 68.5 ms | 69.4 ms | +1.2% | [-0.2%, +2.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 9.18 ms | 9.30 ms | +1.3% | [-0.4%, +3.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.07 ms | 2.07 ms | +0.1% | [-0.9%, +1.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.29 ms | 7.29 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 252 ms | 252 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | marched/traced 1.41x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 357 ms | 356 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 250 ms | 250 ms | +0.2% | [-0.7%, +1.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 271 ms | 269 ms | -0.5% | [-0.5%, -0.4%] | +0.0% | ~ | marched/traced 1.56x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 419 ms | 420 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 269 ms | 269 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 24.6 ms | 24.3 ms | -1.0% | [-1.4%, -0.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 24.6 ms | 24.4 ms | -0.8% | [-1.1%, -0.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.6 ms | 19.8 ms | +0.9% | [+0.7%, +1.2%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.5 ms | 31.2 ms | +2.3% | [+2.2%, +2.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 30.7 ms | 31.5 ms | +2.5% | [+1.1%, +4.0%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 22.0 ms | 21.7 ms | -1.5% | [-2.2%, -0.8%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 13.3 ms | 13.3 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.72 ms | 4.66 ms | -1.2% | [-1.9%, -0.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.9 ms | 26.0 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 41.9 ms | 41.8 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.80 ms | 1.79 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 128 µs | 124 µs | -3.3% | [-6.4%, -0.1%] | - | faster |  |
| `Tape/sphere/signedDistance-x16/float/host` | 31.0 ms | 31.2 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.81 ms | 5.79 ms | -0.3% | [-1.4%, +0.8%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.7 ms | 16.7 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.77 ms | 4.82 ms | +1.0% | [-0.0%, +2.1%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.4 ms | 13.5 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.67 ms | 4.67 ms | +0.1% | [-1.6%, +1.7%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 27.5 ms | 27.6 ms | +0.5% | [-0.2%, +1.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 43.9 ms | 43.8 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.80 ms | 1.78 ms | -1.1% | [-3.0%, +0.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 264 µs | 271 µs | +2.6% | [+0.3%, +4.8%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 31.2 ms | 31.0 ms | -0.5% | [-0.9%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.88 ms | 8.08 ms | +2.4% | [+2.2%, +2.6%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.9 ms | 16.9 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.90 ms | 7.90 ms | +0.1% | [-0.0%, +0.2%] | - | ~ |  |

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: f1c1c40
base-fingerprint: 139b0db678361b912e9d5bbc988f0a34804c9ec8
head: def340e
head-fingerprint: 1759bc965c8f27541a01db9aadccd206bf503ecf
cases: 268
rounds: 2
reps: 3
date: 2026-11-02
-->

## Device comparison (CUDA): red

**Accepted:** Yes. Accept them as noise.

`def340e` against `f1c1c40` (origin/main), 2026-11-02.

**VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 1 major, 3 minor, 7 faster, 0 differ, 0 lost, 268 cases

- base: f1c1c40 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: def340e (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.63 ms | 1.62 ms | -0.5% | [-1.3%, +0.4%] | - | ~ | device 22.9x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.85 ms | 4.22 ms | +9.6% | [-1.1%, +22.9%] | - | ~ | device 15.2x faster than the host (was 17.0x) |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.06 ms | +1.2% | [-1.4%, +3.9%] | - | ~ | device 19.7x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.32 ms | 2.33 ms | +0.5% | [-0.2%, +1.3%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 7.10 ms | 7.03 ms | -1.1% | [-2.3%, +0.2%] | - | ~ | device 6.5x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 13.3 ms | 13.0 ms | -2.6% | [-5.1%, -0.2%] | - | ~ | device 5.0x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 36.7 ms | 36.7 ms | +0.0% | [-1.6%, +1.7%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 63.2 ms | 63.0 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 22.9 ms | 23.2 ms | +1.2% | [-1.0%, +3.4%] | - | ~ | device 0.9x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 52.9 ms | 52.7 ms | -0.3% | [-1.7%, +1.2%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 68.0 ms | 68.6 ms | +0.9% | [-1.8%, +3.6%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 122 ms | 123 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 0.6x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 389 µs | 387 µs | -0.6% | [-0.7%, -0.5%] | - | ~ | device 15.4x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 849 µs | 846 µs | -0.4% | [-0.9%, +0.0%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 5.31 ms | 5.29 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 5.03 ms | 5.42 ms | +7.9% | [+0.3%, +16.8%] | - | minor | device 1.6x faster than the host (was 1.8x) |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 582 µs | 584 µs | +0.4% | [+0.3%, +0.5%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.37 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 8.5x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 7.53 ms | 7.52 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 1.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.82 ms | 7.83 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 1.5x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 899 µs | 899 µs | +0.0% | [-0.4%, +0.4%] | - | ~ | device 14.7x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.22 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 8.2x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 10.3 ms | 10.3 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.9 ms | 10.9 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 1.8x faster than the host (was 1.7x) |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 320 µs | 317 µs | -0.8% | [-2.0%, +0.3%] | - | ~ | device 11.7x faster than the host (was 12.1x) |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.73 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 5.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 64.8 ms | 65.2 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 3.2x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 6.29 ms | 6.30 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.6x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 567 ms | 563 ms | -0.7% | [-2.0%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 48.0 µs | 48.0 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 23.9x faster than the host; tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.0 µs | 49.9 µs | -0.2% | [-0.6%, +0.3%] | - | ~ | device 44.1x faster than the host; tape/fused 7.21x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.4 µs | 48.3 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 10.0x faster than the host (was 9.5x); tape/fused 9.80x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 19.1 ms | 19.1 ms | -0.1% | [-1.5%, +1.4%] | - | ~ | device 6.3x faster than the host (was 6.9x); tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 34.5 ms | 34.4 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 3.5x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 811 µs | 810 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.56 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.11 ms | 2.10 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 314 ms | 306 ms | -2.7% | [-3.9%, -1.5%] | - | ~ | device 0.4x faster than the host; tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 312 ms | 306 ms | -1.8% | [-2.9%, -0.6%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 9.24 ms | 9.30 ms | +0.7% | [-0.7%, +2.1%] | +0.0% | ~ | device 10.8x faster than the host (was 11.2x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 9.20 ms | 9.29 ms | +0.9% | [-0.6%, +2.4%] | +0.0% | ~ | device 9.7x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 9.12 ms | 9.04 ms | -0.9% | [-3.6%, +1.9%] | +0.0% | ~ | device 10.0x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 9.26 ms | 9.29 ms | +0.4% | [-3.1%, +4.1%] | +0.0% | ~ | device 9.8x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 711 µs | 708 µs | -0.5% | [-0.5%, -0.4%] | +0.0% | ~ | device 4.7x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.74 ms | 1.74 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.41x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.04 ms | +0.1% | [-0.5%, +0.8%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.68x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 840 µs | 844 µs | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ | device 3.9x faster than the host; default/this 0.84x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 841 µs | 835 µs | -0.7% | [-1.2%, -0.2%] | +0.0% | ~ | device 11.8x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 973 µs | 973 µs | -0.0% | [-0.7%, +0.7%] | +0.0% | ~ | device 10.0x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 830 µs | 833 µs | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ | device 11.8x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 813 µs | 811 µs | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ | device 11.7x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 179 ms | 175 ms | -2.0% | [-6.0%, +2.1%] | +0.0% | ~ | device 0.5x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 177 ms | 174 ms | -1.6% | [-2.0%, -1.1%] | +0.0% | ~ | device 0.5x faster than the host (was 0.6x); default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 181 ms | 181 ms | +0.3% | [-0.4%, +1.1%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.97x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 178 ms | 176 ms | -0.9% | [-2.8%, +1.0%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | device 2.4x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.76 ms | 1.77 ms | +0.6% | [+0.4%, +0.8%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.91x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.93 ms | -1.0% | [-1.2%, -0.8%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.83x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.67 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ | device 2.2x faster than the host (was 2.3x); default/this 0.96x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 8.06 ms | 8.06 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 8.21 ms | 8.20 ms | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.98x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 8.07 ms | 8.09 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 8.15 ms | 8.22 ms | +0.9% | [-0.7%, +2.4%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.98x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 9.32 ms | 9.45 ms | +1.4% | [+0.4%, +2.5%] | - | ~ | device 10.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 18.2 ms | 18.3 ms | +0.4% | [-0.5%, +1.3%] | - | ~ | device 8.5x faster than the host (was 6.7x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 183 ms | 180 ms | -1.7% | [-5.5%, +2.1%] | - | ~ | device 0.5x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 316 ms | 309 ms | -2.2% | [-6.5%, +2.3%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 27.1 ms | 26.9 ms | -0.7% | [-1.8%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 64.8 ms | 64.9 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 2.3x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 171 ms | 176 ms | +2.6% | [+0.2%, +5.1%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 269 ms | 271 ms | +0.6% | [-1.1%, +2.3%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.96 ms | 3.34 ms | +12.7% | [-0.2%, +26.0%] | - | ~ | device 3.2x faster than the host (was 3.6x) |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.82 ms | 5.89 ms | +1.1% | [-0.1%, +2.4%] | - | ~ | device 2.4x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 16.0 ms | 15.9 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 135 ms | 132 ms | -2.3% | [-5.2%, +0.8%] | - | ~ | device 1.6x faster than the host (was 1.5x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 192 ms | 188 ms | -2.4% | [-3.3%, -1.4%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 404 ms | 401 ms | -0.8% | [-1.8%, +0.3%] | - | ~ | device 2.4x faster than the host (was 2.1x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 27.6 ms | 27.7 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 1.9x faster than the host (was 2.0x); tape/fused 4.77x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 29.4 ms | 29.7 ms | +1.1% | [+0.7%, +1.5%] | - | ~ | device 2.3x faster than the host (was 2.0x); tape/fused 6.32x (was 6.55x) |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 78.3 ms | 79.2 ms | +1.1% | [-0.1%, +2.3%] | - | ~ | device 4.1x faster than the host (was 4.2x); tape/fused 5.07x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 24.6 ms | 24.6 ms | -0.1% | [-2.2%, +2.1%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 48.3 ms | 48.5 ms | +0.5% | [-0.3%, +1.2%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 105 ms | 104 ms | -0.8% | [-1.3%, -0.4%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 584 ms | 590 ms | +1.1% | [-2.8%, +5.0%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 919 ms | 920 ms | +0.1% | [-0.4%, +0.7%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.48 s | 3.60 s | +3.5% | [-0.5%, +7.8%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 223 ms | 211 ms | -5.3% | [-9.1%, -1.2%] | - | faster | device 0.2x faster than the host (was 0.2x); tape/fused 2.80x (was 2.62x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 334 ms | 343 ms | +2.5% | [-0.0%, +5.0%] | - | ~ | device 0.2x faster than the host; tape/fused 2.68x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 652 ms | 667 ms | +2.3% | [-1.3%, +6.1%] | - | ~ | device 0.5x faster than the host; tape/fused 5.40x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 768 µs | 765 µs | -0.3% | [-0.4%, -0.2%] | - | ~ | device 10.9x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.20 ms | 2.19 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 5.9x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.17 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 8.4x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 7.15 ms | 7.20 ms | +0.6% | [-0.1%, +1.3%] | - | ~ | device 4.0x faster than the host (was 4.2x) |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 4.48 ms | 4.18 ms | -6.8% | [-16.7%, +5.1%] | - | ~ | device 4.4x faster than the host (was 4.0x) |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.89 ms | 7.86 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 8.61 ms | 8.58 ms | -0.4% | [-1.2%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 14.7 ms | 14.7 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 22.8 ms | 22.8 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 13.4 ms | 12.7 ms | -5.7% | [-5.8%, -5.6%] | - | faster | device 1.4x faster than the host (was 1.4x) |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.20 ms | 2.18 ms | -0.8% | [-2.3%, +0.7%] | - | ~ | device 5.0x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 6.77 ms | 6.68 ms | -1.2% | [-1.6%, -0.9%] | - | ~ | device 2.4x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 834 µs | 834 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 10.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 9.75 ms | 9.70 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 3.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 7.25 ms | 7.32 ms | +1.1% | [+0.4%, +1.8%] | - | ~ | device 3.7x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.93 ms | 8.87 ms | -0.6% | [-2.0%, +0.7%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 17.4 ms | 17.5 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.80 ms | 4.41 ms | -8.1% | [-16.5%, +0.3%] | - | ~ | device 2.1x faster than the host (was 1.9x) |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 22.9 ms | 22.8 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 17.0 ms | 16.8 ms | -1.1% | [-4.2%, +2.1%] | - | ~ | device 1.6x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.50 ms | 1.50 ms | -0.5% | [-1.3%, +0.3%] | - | ~ | device 8.8x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 5.47 ms | 5.08 ms | -7.1% | [-13.1%, -0.9%] | - | faster | device 3.8x faster than the host (was 3.6x) |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.39 ms | 1.37 ms | -1.2% | [-2.6%, +0.2%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.80 ms | 3.79 ms | -0.3% | [-19.6%, +23.6%] | - | ~ | device 5.1x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 5.34 ms | 5.38 ms | +0.8% | [+0.1%, +1.5%] | - | ~ | device 3.6x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 14.9 ms | 14.9 ms | -0.2% | [-1.2%, +0.8%] | - | ~ | device 0.9x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 18.8 ms | 19.1 ms | +1.3% | [+0.9%, +1.8%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.27 ms | 8.31 ms | +0.4% | [+0.0%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 12.2 ms | 12.6 ms | +3.2% | [-0.1%, +6.7%] | - | ~ | device 1.6x faster than the host (was 1.6x) |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 17.4 ms | 17.5 ms | +0.5% | [-0.3%, +1.2%] | - | ~ | device 1.1x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 5.68 ms | 5.70 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 3.1x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 17.7 ms | 17.7 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.62 ms | 1.63 ms | +0.6% | [+0.1%, +1.2%] | - | ~ | device 7.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 6.42 ms | 6.42 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 4.4x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 15.0 ms | 15.0 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 37.0 ms | 37.4 ms | +1.0% | [-0.6%, +2.6%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 76.8 ms | 76.9 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 20.3 ms | 20.2 ms | -0.6% | [-3.0%, +1.9%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 23.3 ms | 23.2 ms | -0.5% | [-1.0%, +0.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 43.4 ms | 43.0 ms | -1.0% | [-2.2%, +0.1%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 682 µs | 678 µs | -0.5% | [-2.4%, +1.3%] | - | ~ | device 17.0x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 14.7x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 643 µs | 645 µs | +0.2% | [-0.9%, +1.3%] | - | ~ | device 17.4x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.22 ms | -0.7% | [-1.3%, -0.2%] | - | ~ | device 15.6x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 615 µs | 619 µs | +0.8% | [-0.5%, +2.0%] | - | ~ | device 16.1x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | -0.5% | [-1.4%, +0.5%] | - | ~ | device 15.1x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.19 ms | +0.2% | [-0.4%, +0.9%] | - | ~ | device 12.3x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.47 ms | -0.6% | [-1.0%, -0.1%] | - | ~ | device 10.1x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 12.1x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | +0.0% | [-0.6%, +0.7%] | - | ~ | device 10.2x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 608 µs | 632 µs | +4.1% | [+0.3%, +8.1%] | - | minor | device 2.4x faster than the host (was 2.5x); tape/fused 1.12x (was 1.17x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 700 µs | 657 µs | -6.1% | [-10.8%, -0.9%] | - | faster | device 2.8x faster than the host (was 2.6x); tape/fused 1.28x (was 1.19x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 709 µs | 705 µs | -0.6% | [-1.4%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 833 µs | 838 µs | +0.7% | [+0.4%, +0.9%] | - | ~ | device 3.7x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 18.4 ms | 18.6 ms | +0.8% | [+0.5%, +1.1%] | - | ~ | device 0.6x faster than the host; hand/api 1.01x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 30.5 ms | 30.4 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 0.7x faster than the host; hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 18.6 ms | 18.7 ms | +0.9% | [-0.6%, +2.4%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 30.4 ms | 30.1 ms | -0.9% | [-1.6%, -0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.5% | [-1.5%, +0.4%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 26.8 ms | 27.1 ms | +1.1% | [+0.4%, +1.7%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 18.1 ms | 18.1 ms | +0.2% | [-1.1%, +1.6%] | - | ~ | device 0.8x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 26.8 ms | 27.0 ms | +0.8% | [-0.0%, +1.5%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.13x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 16.6 ms | 16.7 ms | +0.6% | [-0.1%, +1.4%] | - | ~ | device 0.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 25.2 ms | 25.3 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 0.9x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 1.1x faster than the host; tape/fused 1.09x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.64 ms | 2.61 ms | -1.0% | [-1.7%, -0.4%] | - | ~ | device 0.8x faster than the host; tape/fused 1.20x (was 1.03x) |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.61 ms | +0.3% | [-0.4%, +0.9%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 3.12 ms | +14.6% | [+1.0%, +28.3%] | - | **MAJOR** | device 1.1x faster than the host (was 1.2x) |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.58 ms | 2.58 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.0x faster than the host; marched/traced 3.80x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.40 ms | 3.02 ms | -11.1% | [-20.2%, +0.4%] | - | ~ | device 2.7x faster than the host (was 2.5x); marched/traced 5.10x (was 4.54x) |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.77 ms | 9.80 ms | +0.3% | [+0.0%, +0.5%] | - | ~ | device 4.1x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 15.4 ms | 15.4 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.17 ms | 2.17 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 3.9x faster than the host; marched/traced 18.24x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 6.17 ms | 6.17 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.5x faster than the host; marched/traced 12.76x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 39.6 ms | 39.6 ms | +0.0% | [-0.7%, +0.8%] | - | ~ | device 1.9x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 78.7 ms | 78.7 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.99 ms | 5.98 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 2.7x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 9.63 ms | 9.70 ms | +0.6% | [-0.7%, +2.0%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 7.03 ms | 7.02 ms | -0.1% | [-1.1%, +1.0%] | - | ~ | device 1.1x faster than the host; marched/traced 25.15x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 11.7 ms | 11.4 ms | -3.1% | [-6.1%, -0.1%] | - | faster | device 0.7x faster than the host; marched/traced 24.81x (was 23.91x) |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 176 ms | 177 ms | +0.5% | [-1.2%, +2.2%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 281 ms | 282 ms | +0.5% | [-1.0%, +1.9%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 5.61 ms | 5.62 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 1.6x faster than the host; marched/traced 21.87x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 13.7 ms | 13.7 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 1.2x faster than the host; marched/traced 17.60x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 123 ms | 123 ms | -0.0% | [-4.0%, +4.1%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 240 ms | 242 ms | +0.9% | [+0.1%, +1.7%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 21.4 ms | 21.3 ms | -0.5% | [-2.9%, +2.0%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 29.4 ms | 29.7 ms | +0.9% | [-0.1%, +2.0%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 675 µs | 676 µs | +0.1% | [-0.7%, +0.9%] | - | ~ | device 17.5x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.29 ms | +0.8% | [-0.6%, +2.2%] | - | ~ | device 15.5x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 686 µs | 677 µs | -1.3% | [-2.1%, -0.5%] | - | ~ | device 17.4x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.2% | [-1.1%, +0.7%] | - | ~ | device 15.9x faster than the host (was 15.3x); hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 704 µs | 709 µs | +0.6% | [+0.2%, +1.0%] | - | ~ | device 4.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 840 µs | 842 µs | +0.3% | [-0.2%, +0.7%] | - | ~ | device 4.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 616 µs | 615 µs | -0.1% | [-0.2%, +0.1%] | - | ~ | device 4.5x faster than the host; hit/crossing 1.15x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 637 µs | 638 µs | +0.2% | [-0.3%, +0.7%] | - | ~ | device 4.8x faster than the host; hit/crossing 1.32x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 5.44 ms | 5.30 ms | -2.5% | [-5.9%, +0.9%] | - | ~ | device 9.3x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 11.0 ms | 11.0 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 6.5x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.85 ms | 4.62 ms | -4.6% | [-13.5%, +4.6%] | - | ~ | device 9.5x faster than the host (was 8.9x); hit/crossing 1.15x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.87 ms | 9.85 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 6.5x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 290 ms | 285 ms | -1.8% | [-4.2%, +0.7%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 237 ms | 231 ms | -2.4% | [-5.1%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 60.6 ms | 61.0 ms | +0.6% | [-1.0%, +2.3%] | - | ~ | device 2.4x faster than the host; hit/crossing 4.68x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 78.0 ms | 77.2 ms | -1.1% | [-2.3%, +0.1%] | - | ~ | device 2.3x faster than the host; hit/crossing 2.99x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 18.8 ms | 18.9 ms | +0.8% | [-0.2%, +1.9%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 30.5 ms | 30.3 ms | -0.8% | [-1.0%, -0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 18.5 ms | 18.5 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 0.6x faster than the host (was 0.6x); hit/crossing 1.02x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 30.5 ms | 30.4 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.61 ms | +0.6% | [+0.4%, +0.7%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.73 ms | +0.3% | [-0.3%, +0.8%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.30 ms | 1.29 ms | -0.7% | [-1.0%, -0.3%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.25x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.35 ms | 1.36 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 2.6x faster than the host; hit/crossing 2.01x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 71.2 ms | 70.7 ms | -0.6% | [-1.5%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 135 ms | 135 ms | +0.0% | [-0.6%, +0.6%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 64.9 ms | 64.7 ms | -0.3% | [-1.0%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.09x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 122 ms | 123 ms | +1.2% | [+0.6%, +1.8%] | - | ~ | device 0.5x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 566 ms | 527 ms | -6.8% | [-12.7%, -0.5%] | - | faster | device 0.3x faster than the host (was 0.3x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 782 ms | 787 ms | +0.7% | [+0.0%, +1.3%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 429 ms | 449 ms | +4.8% | [-5.8%, +17.0%] | - | ~ | device 0.3x faster than the host (was 0.4x); hit/crossing 1.17x (was 1.32x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 576 ms | 565 ms | -1.8% | [-4.0%, +0.4%] | - | ~ | device 0.3x faster than the host (was 0.3x); hit/crossing 1.39x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.62 ms | 3.94 ms | +8.8% | [-2.1%, +19.7%] | - | ~ | device 1.1x faster than the host (was 1.2x); marched/traced 5.58x (was 6.04x) |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 5.48 ms | 5.47 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 0.8x faster than the host; marched/traced 8.05x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 21.9 ms | 22.0 ms | +0.4% | [-0.2%, +0.9%] | - | ~ | device 2.2x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 44.2 ms | 44.0 ms | -0.5% | [-1.3%, +0.4%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.25 ms | 1.26 ms | +1.0% | [-0.8%, +2.8%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.85 ms | +0.0% | [-0.9%, +1.0%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 94.9 µs | 97.9 µs | +3.1% | [+3.0%, +3.3%] | - | minor | device 14.8x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 151 µs | 152 µs | +0.7% | [-1.2%, +2.7%] | - | ~ | device 10.8x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 800 µs | 803 µs | +0.3% | [-0.0%, +0.6%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 935 µs | 940 µs | +0.6% | [+0.1%, +1.1%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 38.7 ms | 39.8 ms | +2.8% | [-3.6%, +9.5%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 3.35x (was 3.53x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 40.3 ms | 40.3 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 0.1x faster than the host; marched/traced 6.41x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 137 ms | 133 ms | -2.5% | [-4.0%, -0.9%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 257 ms | 258 ms | +0.6% | [+0.0%, +1.3%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 17.8 ms | 18.0 ms | +1.4% | [-2.6%, +5.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 24.0 ms | 24.0 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 0.1x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.07 ms | 1.11 ms | +3.7% | [-2.2%, +10.0%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.47 ms | 2.48 ms | +0.5% | [-1.5%, +2.6%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 6.72 ms | 6.69 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 7.46 ms | 7.50 ms | +0.5% | [-0.1%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 25.3 ms | 25.2 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 2.0x faster than the host; marched/traced 4.33x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 31.5 ms | 31.1 ms | -1.1% | [-1.9%, -0.4%] | - | ~ | device 1.8x faster than the host; marched/traced 5.73x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 108 ms | 109 ms | +1.0% | [-0.6%, +2.5%] | - | ~ | device 2.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 177 ms | 178 ms | +0.8% | [-0.0%, +1.7%] | - | ~ | device 1.4x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 15.3 ms | 15.3 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 3.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 19.0 ms | 18.9 ms | -0.5% | [-1.9%, +0.9%] | - | ~ | device 3.0x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 337 ms | 329 ms | -2.3% | [-5.3%, +0.8%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 1.86x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 388 ms | 382 ms | -1.6% | [-4.8%, +1.8%] | - | ~ | device 0.2x faster than the host; marched/traced 2.82x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 630 ms | 614 ms | -2.5% | [-3.7%, -1.4%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.09 s | 1.08 s | -0.9% | [-1.8%, +0.1%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 205 ms | 207 ms | +0.8% | [-2.8%, +4.6%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 269 ms | 257 ms | -4.5% | [-8.3%, -0.5%] | - | faster | device 0.2x faster than the host (was 0.2x) |
| `Render/csg/frame/float/hostdevice` | 5.38 ms | 5.34 ms | -0.9% | [-1.1%, -0.6%] | - | ~ | device 2.6x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 5.01 ms | 4.97 ms | -0.9% | [-14.6%, +14.9%] | - | ~ | device 3.6x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.89 ms | 1.92 ms | +1.6% | [-1.8%, +5.1%] | - | ~ | device 9.0x faster than the host |
| `Render/csg/frame/double/hostdevice` | 8.89 ms | 8.72 ms | -1.8% | [-2.1%, -1.5%] | - | ~ | device 1.8x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 8.87 ms | 8.70 ms | -2.0% | [-2.3%, -1.7%] | - | ~ | device 1.8x faster than the host (was 1.7x) |
| `Render/mesh/frame/double/hostdevice` | 25.8 ms | 25.8 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 218 µs | +0.2% | [+0.2%, +0.2%] | - | ~ | device 11.5x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 48.2 µs | +0.5% | [+0.2%, +0.8%] | - | ~ | device 23.0x faster than the host (was 24.6x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 360 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 12.9x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.01 ms | 2.01 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 7.3x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 73.6 µs | 72.7 µs | -1.3% | [-2.7%, +0.2%] | - | ~ | device 7.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 47.4 µs | -0.5% | [-0.8%, -0.3%] | - | ~ | device 4.8x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 656 µs | 652 µs | -0.6% | [-1.3%, +0.1%] | - | ~ | device 10.7x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.1% | [-0.3%, -0.0%] | - | ~ | device 12.3x faster than the host (was 11.9x) |
| `Tape/chain16/signedDistance/float/hostdevice` | 474 µs | 473 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 7.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.2 µs | 48.4 µs | +0.4% | [-0.0%, +0.9%] | - | ~ | device 27.7x faster than the host (was 26.3x) |
| `Tape/csg/signedDistance/double/hostdevice` | 857 µs | 855 µs | -0.2% | [-0.7%, +0.4%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 811 µs | 810 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 7.47 ms | 7.48 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 190 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.7x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.9x faster than the host (was 2.0x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.77 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 2.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.11 ms | 2.11 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.0x faster than the host |

<!-- END receipt cuda -->
