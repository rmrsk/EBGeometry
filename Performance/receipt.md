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
base: b682c83
base-fingerprint: 796228c6b11b9d148268f59f3c375346281f1050
head: 4bdb4ba
head-fingerprint: c52b275ef0f62896661b5450c135a236d0a2eb0c
cases: 224
rounds: 2
reps: 3
date: 2026-11-05
-->

## Host comparison: yellow

**Accepted:** Accepted, as explained

`4bdb4ba` against `b682c83` (origin/main), 2026-11-05.

**VERDICT: 1 minor regression; rerun it with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 1 minor, 17 faster, 0 differ, 0 lost, 224 cases

- base: b682c83 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 4bdb4ba (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 330 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 512 ms | 515 ms | +0.6% | [+0.0%, +1.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 348 ms | 348 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | +0.3% | [+0.3%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 562 ms | +0.1% | [+0.0%, +0.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.3 ms | 41.7 ms | +1.0% | [-0.4%, +2.4%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 76.0 ms | 75.1 ms | -1.1% | [-1.6%, -0.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.4 ms | 55.4 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.2 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.0 ms | 65.0 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.7 ms | 54.4 ms | -0.7% | [-0.9%, -0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.4 ms | 60.6 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.4 ms | 72.2 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.9 ms | 74.6 ms | -0.4% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.4 ms | 77.9 ms | +0.7% | [+0.3%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.7 ms | 66.8 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 100 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.2 ms | 83.3 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 109 ms | +0.2% | [-0.5%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.5 ms | +0.4% | [+0.2%, +0.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 44.5 ms | 45.0 ms | +1.2% | [-0.4%, +2.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.9 ms | 73.0 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.5 ms | 20.1 ms | -1.9% | [-3.2%, -0.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.8 ms | 46.0 ms | +0.5% | [+0.0%, +0.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 80.7 ms | 82.0 ms | +1.6% | [-0.4%, +3.6%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.43 ms | 4.45 ms | +0.5% | [-0.7%, +1.7%] | - | ~ | tape/fused 2.89x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.0% | [-0.9%, +0.9%] | - | ~ | tape/fused 2.05x |
| `Fused/chain16/signedDistance/float/host` | 975 µs | 971 µs | -0.5% | [-3.0%, +2.1%] | - | ~ | tape/fused 16.60x |
| `Fused/drilled-mesh/signedDistance/float/host` | 45.9 ms | 46.1 ms | +0.3% | [+0.0%, +0.7%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.2 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.58 ms | 4.54 ms | -0.8% | [-1.1%, -0.5%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | tape/fused 2.07x |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.88 ms | -0.3% | [-1.2%, +0.6%] | - | ~ | tape/fused 4.19x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.9 ms | 52.9 ms | +0.1% | [-1.3%, +1.6%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.9 ms | 52.8 ms | -0.2% | [-1.0%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.1 ms | 81.8 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 228 ms | 228 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 261 ms | +0.6% | [-0.1%, +1.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.3 ms | 70.5 ms | +0.3% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 213 ms | 213 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.7 ms | 96.8 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.6 ms | 29.8 ms | +0.5% | [-1.0%, +2.0%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 756 ms | 757 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 117 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 42.5 ms | 42.8 ms | +0.9% | [-1.3%, +3.2%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 848 ms | 854 ms | +0.8% | [+0.2%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.7 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.0 ms | 26.8 ms | -0.7% | [-1.8%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 319 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | -0.4% | [-1.0%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.1 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ | tape/fused 3.54x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.6 ms | 99.6 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.6 ms | 29.7 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.2 ms | 29.9 ms | -1.1% | [-2.7%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 354 ms | 362 ms | +2.3% | [+2.0%, +2.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 259 ms | 265 ms | +2.4% | [+0.4%, +4.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 96.8 ms | 98.3 ms | +1.5% | [+0.1%, +2.9%] | +0.0% | ~ | tape/fused 3.68x |
| `MeshCSG/smooth-fused/frame/double/host` | 112 ms | 107 ms | -4.1% | [-5.2%, -2.9%] | +0.0% | faster | tape/fused 2.47x (was 2.32x) |
| `MeshReconcile/armadillo/reconcile/float/host` | 131 ms | 132 ms | +0.5% | [-0.1%, +1.2%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.3 ms | 41.4 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ | reconcile/scheduled 3.18x |
| `MeshReconcile/armadillo/reconcile/double/host` | 154 ms | 155 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.2 ms | 60.3 ms | +7.2% | [+3.9%, +10.5%] | +0.0% | minor | reconcile/scheduled 2.58x (was 2.75x) |
| `PointCloudBVH/uniform/build/float/host` | 53.5 ms | 53.2 ms | -0.5% | [-1.0%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 143 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 353 ms | 352 ms | -0.3% | [-0.6%, +0.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 79.4 ms | 79.3 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.66 ms | 6.66 ms | +0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.9 ms | 55.7 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 155 ms | 156 ms | +0.6% | [-0.1%, +1.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 385 ms | 385 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.0 ms | 86.4 ms | -0.7% | [-1.8%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.93 ms | 6.92 ms | -0.2% | [-1.7%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.9 ms | 13.6 ms | -2.6% | [-5.1%, -0.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | -0.0% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 475 ms | 476 ms | +0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 142 ms | 142 ms | +0.2% | [-0.0%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 800 ms | 799 ms | -0.1% | [-0.1%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.8 ms | 14.7 ms | -0.7% | [-1.2%, -0.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 121 ms | 121 ms | -0.1% | [-0.7%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 494 ms | 493 ms | -0.2% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 144 ms | +0.1% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 786 ms | 791 ms | +0.5% | [+0.1%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.4 ms | 54.5 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 128 ms | 128 ms | +0.2% | [-0.2%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 260 ms | 260 ms | +0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 148 ms | 148 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 133 ms | 133 ms | +0.3% | [-0.1%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.4 ms | 57.3 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 139 ms | 139 ms | +0.0% | [-0.7%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 291 ms | 291 ms | -0.3% | [-0.6%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 161 ms | 162 ms | +0.6% | [-0.9%, +2.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 148 ms | 148 ms | -0.1% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.7 ms | 11.6 ms | -0.7% | [-1.6%, +0.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 116 ms | 116 ms | -0.7% | [-1.2%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 319 ms | 318 ms | -0.4% | [-0.9%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 156 ms | 157 ms | +0.7% | [-0.0%, +1.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 320 ms | 320 ms | +0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.4 ms | 13.5 ms | +0.6% | [-0.1%, +1.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 115 ms | +0.1% | [-0.0%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 349 ms | 349 ms | +0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 166 ms | 167 ms | +0.8% | [-0.3%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 308 ms | 308 ms | +0.1% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.1 ms | 35.2 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.2 ms | 38.1 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.24 ms | 8.21 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.55 ms | 9.27 ms | -2.9% | [-5.3%, -0.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.4 ms | 46.5 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.5 ms | -0.4% | [-0.4%, -0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.3 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.7 ms | 78.7 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.5 ms | 65.6 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.97 ms | 5.18 ms | -13.3% | [-15.4%, -11.1%] | - | faster | tape/fused 1.45x (was 1.33x) |
| `Ray/csg-tape/firstHit/float/host` | 7.94 ms | 7.53 ms | -5.2% | [-5.3%, -5.1%] | +0.0% | faster |  |
| `Ray/mesh/firstHit/double/host` | 52.6 ms | 52.2 ms | -0.8% | [-1.3%, -0.2%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.5 ms | 51.4 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.0 ms | 44.7 ms | -0.7% | [-0.9%, -0.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 93.6 ms | 87.4 ms | -6.6% | [-12.6%, +0.2%] | +0.0% | ~ | triangles/polygons 0.60x (was 0.56x) |
| `Ray/mesh-polygons/intersects/double/host` | 74.2 ms | 73.5 ms | -1.0% | [-1.8%, -0.2%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 6.03 ms | 5.50 ms | -8.9% | [-10.2%, -7.5%] | - | faster | tape/fused 1.46x (was 1.40x) |
| `Ray/csg-tape/firstHit/double/host` | 8.46 ms | 8.03 ms | -5.1% | [-6.6%, -3.6%] | +0.0% | faster |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.0 ms | 17.2 ms | -4.2% | [-5.8%, -2.7%] | - | faster | marched/traced 6.77x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 117 ms | -2.4% | [-2.6%, -2.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.9 ms | 22.0 ms | +0.6% | [-0.4%, +1.6%] | +0.0% | ~ | marched/traced 6.98x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 154 ms | -0.2% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.2 ms | 35.4 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 20.0 ms | 20.4 ms | +1.9% | [-0.1%, +3.9%] | - | ~ | marched/traced 6.07x (was 6.58x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 132 ms | 124 ms | -6.0% | [-9.6%, -2.2%] | - | faster |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 22.9 ms | -2.6% | [-3.4%, -1.8%] | +0.0% | ~ | marched/traced 6.99x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 161 ms | 160 ms | -0.5% | [-0.9%, -0.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.9 ms | 37.4 ms | -1.4% | [-1.8%, -1.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.4 ms | 46.3 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.5 ms | 46.3 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.2 ms | 7.80 ms | -23.8% | [-24.1%, -23.5%] | +0.0% | faster |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.75 ms | 6.24 ms | -28.7% | [-29.2%, -28.2%] | +0.0% | faster | hit/crossing 1.25x (was 1.17x) |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 181 ms | 177 ms | -1.7% | [-2.0%, -1.4%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 160 ms | 157 ms | -2.0% | [-2.2%, -1.7%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.6 ms | 68.8 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.4 ms | 56.4 ms | +0.1% | [-0.6%, +0.7%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.7 ms | 52.4 ms | -0.7% | [-1.1%, -0.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.4 ms | 52.8 ms | +0.7% | [-0.4%, +1.8%] | +0.0% | ~ | hit/crossing 0.99x |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.1 ms | 7.85 ms | -29.3% | [-29.8%, -28.7%] | +0.0% | faster |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.35 ms | 6.21 ms | -33.6% | [-33.6%, -33.6%] | +0.0% | faster | hit/crossing 1.26x (was 1.19x) |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 207 ms | 199 ms | -4.0% | [-4.4%, -3.5%] | +0.0% | faster |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 184 ms | 178 ms | -3.6% | [-3.8%, -3.5%] | +0.0% | faster | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 79.2 ms | 77.6 ms | -2.0% | [-2.5%, -1.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.7 ms | 64.3 ms | -0.5% | [-1.7%, +0.6%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.0 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | marched/traced 4.70x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.9 ms | +0.4% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.06 ms | 7.98 ms | -1.1% | [-1.5%, -0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.91 ms | 1.92 ms | +0.5% | [+0.2%, +0.9%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.91 ms | 6.80 ms | -1.7% | [-2.7%, -0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.6 ms | 11.7 ms | +0.8% | [+0.4%, +1.1%] | +0.0% | ~ | marched/traced 5.17x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.3 ms | 60.4 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.67 ms | 8.56 ms | -1.2% | [-2.2%, -0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.94 ms | 1.93 ms | -0.5% | [-2.0%, +1.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.17 ms | 7.19 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 240 ms | 240 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | marched/traced 1.36x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 327 ms | 328 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 238 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 250 ms | 251 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ | marched/traced 1.48x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 372 ms | 371 ms | -0.2% | [-1.3%, +0.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 249 ms | 249 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 24.6 ms | 18.5 ms | -24.8% | [-28.2%, -21.0%] | +0.0% | faster |  |
| `Render/csg/frame-orthographic/float/host` | 23.7 ms | 18.7 ms | -21.0% | [-21.4%, -20.7%] | +0.0% | faster |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 18.5 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.2 ms | 21.3 ms | -29.3% | [-29.9%, -28.8%] | +0.0% | faster |  |
| `Render/csg/frame-orthographic/double/host` | 30.0 ms | 21.5 ms | -28.5% | [-29.5%, -27.5%] | +0.0% | faster |  |
| `Render/mesh/frame/double/host` | 20.4 ms | 20.4 ms | +0.1% | [-1.0%, +1.3%] | +0.0% | ~ |  |
| `Shapes/ellipsoid/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Shapes/capped-cone/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Shapes/round-cone/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Shapes/polygon-prism/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Shapes/tube/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Shapes/capped-torus/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Shapes/cut-sphere/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Shapes/wedge/signedDistance/float/host` | - | - | - | - | - | new |  |
| `Shapes/ray-forms/firstHit/float/host` | - | - | - | - | - | new |  |
| `Shapes/ellipsoid/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Shapes/capped-cone/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Shapes/round-cone/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Shapes/polygon-prism/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Shapes/tube/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Shapes/capped-torus/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Shapes/cut-sphere/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Shapes/wedge/signedDistance/double/host` | - | - | - | - | - | new |  |
| `Shapes/ray-forms/firstHit/double/host` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.5 ms | 56.6 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.7 ms | 36.9 ms | +0.5% | [+0.2%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.1 ms | 92.1 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.2 ms | 64.2 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.7 ms | 42.7 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 108 ms | 107 ms | -1.2% | [-2.1%, -0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 75.3 ms | 74.9 ms | -0.6% | [-1.4%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 296 ms | 298 ms | +0.5% | [-0.6%, +1.7%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 297 ms | 300 ms | +1.3% | [-0.2%, +2.8%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 87.0 ms | 86.3 ms | -0.9% | [-1.5%, -0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 347 ms | 346 ms | -0.4% | [-1.4%, +0.6%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 347 ms | 347 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 166 ms | 166 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 191 ms | 191 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | -0.4% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.51 ms | 4.61 ms | +2.0% | [+0.6%, +3.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.7 ms | 25.1 ms | +1.8% | [+0.4%, +3.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.1 ms | 40.3 ms | +0.4% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.74 ms | 1.74 ms | +0.1% | [-2.0%, +2.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 126 µs | 132 µs | +4.5% | [-1.0%, +10.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.0 ms | 29.9 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.64 ms | 5.66 ms | +0.4% | [-0.3%, +1.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.2 ms | 16.1 ms | -0.4% | [-1.5%, +0.8%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.61 ms | 4.63 ms | +0.4% | [-0.7%, +1.5%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.1 ms | -1.1% | [-2.7%, +0.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.56 ms | 4.55 ms | -0.3% | [-1.6%, +0.9%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.3 ms | 26.3 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.2 ms | 42.1 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.73 ms | 1.73 ms | +0.3% | [-0.9%, +1.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 260 µs | 274 µs | +5.5% | [-1.1%, +12.2%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.9 ms | 29.7 ms | -0.6% | [-1.6%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.73 ms | 7.73 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.1 ms | 16.3 ms | +1.1% | [+0.4%, +1.8%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.75 ms | 7.76 ms | +0.1% | [-0.3%, +0.5%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/Shapes/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: b682c83
base-fingerprint: 796228c6b11b9d148268f59f3c375346281f1050
head: 4bdb4ba
head-fingerprint: c52b275ef0f62896661b5450c135a236d0a2eb0c
cases: 322
rounds: 2
reps: 3
date: 2026-11-06
-->

## Device comparison (CUDA): red

**Accepted:** Accepted, as explained

`4bdb4ba` against `b682c83` (origin/main), 2026-11-06.

**VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 1 major, 3 minor, 37 faster, 0 differ, 0 lost, 322 cases

- base: b682c83 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 4bdb4ba (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.60 ms | -0.4% | [-1.3%, +0.5%] | - | ~ | device 22.7x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.41 ms | +0.3% | [+0.0%, +0.5%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.06 ms | +0.6% | [-1.0%, +2.3%] | - | ~ | device 19.3x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.29 ms | -0.9% | [-1.5%, -0.4%] | - | ~ | device 16.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.16 ms | 6.22 ms | +1.0% | [+0.3%, +1.7%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.7 ms | 11.7 ms | -0.0% | [-0.8%, +0.7%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.1 ms | 33.3 ms | +0.4% | [+0.4%, +0.5%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 57.2 ms | +0.4% | [-0.2%, +0.9%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.5 ms | 20.7 ms | +0.8% | [+0.2%, +1.3%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.8 ms | 47.9 ms | +0.1% | [-1.2%, +1.3%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.4 ms | 62.1 ms | +1.0% | [-1.1%, +3.1%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 107 ms | -4.1% | [-4.8%, -3.4%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.43 ms | 8.24 ms | -2.3% | [-4.1%, -0.5%] | +0.0% | ~ | device 4.9x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.81 ms | 9.91 ms | +1.0% | [-0.5%, +2.5%] | +0.0% | ~ | device 8.1x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 388 µs | 385 µs | -0.7% | [-0.9%, -0.6%] | - | ~ | device 15.3x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 842 µs | 841 µs | -0.0% | [-0.4%, +0.3%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.55 ms | +0.7% | [+0.5%, +0.9%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.67 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 580 µs | 578 µs | -0.5% | [-0.6%, -0.4%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.36 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 8.4x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.75 ms | 6.77 ms | +0.3% | [-0.0%, +0.7%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.03 ms | 7.03 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 898 µs | 899 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.45 ms | 9.46 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 319 µs | 313 µs | -1.9% | [-2.0%, -1.7%] | - | ~ | device 11.9x faster than the host (was 11.4x) |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.79 ms | +3.8% | [+3.5%, +4.1%] | - | minor | device 4.9x faster than the host (was 5.1x) |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.0 ms | 59.1 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.53 ms | 5.60 ms | +1.2% | [+1.1%, +1.3%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 521 ms | 519 ms | -0.5% | [-1.0%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.8 µs | 48.1 µs | +0.5% | [+0.2%, +0.9%] | - | ~ | device 23.7x faster than the host (was 25.0x); tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.0 µs | 49.9 µs | -0.2% | [-0.7%, +0.4%] | - | ~ | device 43.5x faster than the host; tape/fused 7.19x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.2 µs | 48.2 µs | +0.0% | [-1.1%, +1.1%] | - | ~ | device 8.7x faster than the host (was 9.6x); tape/fused 9.87x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.3 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 6.7x faster than the host; tape/fused 1.81x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.4 ms | 31.4 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 809 µs | 810 µs | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.5x faster than the host (was 1.5x); tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 284 ms | 281 ms | -1.1% | [-3.2%, +0.9%] | - | ~ | device 0.4x faster than the host; tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 287 ms | 280 ms | -2.7% | [-4.1%, -1.2%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.21 ms | 8.24 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ | device 10.7x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.25 ms | 8.31 ms | +0.8% | [-0.3%, +1.9%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.27 ms | 8.23 ms | -0.4% | [-0.9%, +0.0%] | +0.0% | ~ | device 11.5x faster than the host (was 10.7x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.19 ms | 8.19 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ | device 10.9x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 687 µs | 673 µs | -2.1% | [-2.3%, -2.0%] | +0.0% | ~ | device 3.8x faster than the host (was 4.8x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.72 ms | 586 µs | -66.0% | [-66.1%, -66.0%] | +0.0% | faster | device 4.4x faster than the host (was 1.9x); default/this 1.15x (was 0.40x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.06 ms | 1.04 ms | -2.3% | [-3.0%, -1.7%] | +0.0% | ~ | device 2.5x faster than the host (was 3.1x); default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 798 µs | 790 µs | -0.9% | [-1.0%, -0.9%] | +0.0% | ~ | device 3.2x faster than the host (was 4.2x); default/this 0.85x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 739 µs | 737 µs | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 824 µs | 827 µs | +0.4% | [-0.7%, +1.4%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 741 µs | 739 µs | -0.3% | [-1.1%, +0.6%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 736 µs | 731 µs | -0.7% | [-1.0%, -0.5%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 159 ms | 160 ms | +0.6% | [-0.6%, +1.9%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 160 ms | 158 ms | -0.9% | [-3.0%, +1.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 162 ms | 163 ms | +0.2% | [-1.6%, +2.1%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 162 ms | 163 ms | +1.1% | [-1.3%, +3.5%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.98x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.39 ms | -13.2% | [-13.4%, -13.1%] | +0.0% | faster | device 2.0x faster than the host (was 2.3x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.78 ms | 2.77 ms | +55.7% | [+54.9%, +56.5%] | +0.0% | **MAJOR** | device 1.0x faster than the host (was 2.1x); default/this 0.50x (was 0.90x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.96 ms | +0.5% | [+0.4%, +0.6%] | +0.0% | ~ | device 1.4x faster than the host (was 1.9x); default/this 0.71x (was 0.82x) |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.55 ms | -8.0% | [-8.6%, -7.5%] | +0.0% | faster | device 1.7x faster than the host (was 2.2x); default/this 0.90x (was 0.95x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.24 ms | 7.26 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.26 ms | 7.32 ms | +0.9% | [-0.4%, +2.3%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.24 ms | 7.27 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.28 ms | 7.29 ms | +0.2% | [-0.4%, +0.9%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.39 ms | 8.33 ms | -0.7% | [-1.3%, -0.1%] | - | ~ | device 11.3x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.5 ms | -0.6% | [-0.8%, -0.4%] | - | ~ | device 7.1x faster than the host (was 9.5x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 169 ms | 168 ms | -0.2% | [-4.0%, +3.7%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 282 ms | 283 ms | +0.6% | [-1.7%, +3.0%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.6 ms | 24.6 ms | +0.1% | [-0.8%, +1.1%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.7 ms | 58.6 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 159 ms | 156 ms | -1.9% | [-3.3%, -0.5%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 241 ms | 247 ms | +2.6% | [+1.5%, +3.8%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.89 ms | 2.56 ms | -11.6% | [-13.5%, -9.7%] | - | faster | device 4.1x faster than the host (was 3.6x) |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.11 ms | 4.32 ms | -15.4% | [-15.7%, -15.1%] | - | faster | device 3.2x faster than the host (was 2.7x) |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.4 ms | 13.7 ms | -5.2% | [-5.5%, -4.9%] | - | faster | device 4.5x faster than the host (was 3.7x) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 120 ms | 64.7 ms | -46.3% | [-47.2%, -45.4%] | - | faster | device 3.1x faster than the host (was 1.8x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 176 ms | 95.9 ms | -45.5% | [-46.7%, -44.4%] | - | faster | device 2.2x faster than the host (was 1.4x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 383 ms | 252 ms | -34.1% | [-35.9%, -32.2%] | - | faster | device 3.6x faster than the host (was 2.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.7 ms | 24.8 ms | +0.3% | [-1.6%, +2.2%] | - | ~ | device 2.0x faster than the host; tape/fused 2.61x (was 4.88x) |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.0 ms | 26.9 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 2.2x faster than the host; tape/fused 3.56x (was 6.53x) |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 70.9 ms | 71.0 ms | +0.2% | [-1.0%, +1.4%] | - | ~ | device 4.1x faster than the host; tape/fused 3.55x (was 5.40x) |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.6 ms | 22.4 ms | -0.8% | [-5.8%, +4.4%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.2 ms | 44.0 ms | -0.4% | [-1.3%, +0.5%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 94.9 ms | 93.0 ms | -2.0% | [-4.7%, +0.8%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 558 ms | 518 ms | -7.2% | [-11.3%, -3.0%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 860 ms | 855 ms | -0.6% | [-1.2%, +0.0%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.22 s | 2.06 s | -36.1% | [-37.4%, -34.7%] | - | faster | device 0.4x faster than the host (was 0.3x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 204 ms | 199 ms | -2.2% | [-3.2%, -1.2%] | - | ~ | device 0.2x faster than the host; tape/fused 2.60x (was 2.74x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 300 ms | 302 ms | +0.7% | [-2.0%, +3.3%] | - | ~ | device 0.2x faster than the host; tape/fused 2.83x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 590 ms | 587 ms | -0.5% | [-2.7%, +1.6%] | - | ~ | device 0.5x faster than the host; tape/fused 3.51x (was 5.47x) |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.31 ms | 5.31 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | device 10.4x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 680 µs | 676 µs | -0.6% | [-1.1%, -0.1%] | - | ~ | device 11.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.82 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 999 µs | 993 µs | -0.6% | [-0.9%, -0.3%] | - | ~ | device 9.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.60 ms | 4.60 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.96 ms | 2.96 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.07 ms | 7.08 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.81 ms | 7.79 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.47 ms | 1.47 ms | +0.1% | [-1.6%, +1.9%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.90 ms | 3.90 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 523 µs | 527 µs | +0.8% | [+0.5%, +1.1%] | - | ~ | device 15.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.47 ms | 4.46 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 6.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.35 ms | 4.34 ms | -0.2% | [-1.5%, +1.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.09 ms | 8.06 ms | -0.4% | [-0.8%, +0.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.7 ms | 15.6 ms | -0.5% | [-0.6%, -0.3%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.00 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.7 ms | 17.7 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.9 ms | 14.9 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.57 ms | 1.57 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.60 ms | 4.59 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 835 µs | 833 µs | -0.2% | [-0.7%, +0.3%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.85 ms | 2.86 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 7.0x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.17 ms | 4.17 ms | -0.2% | [-1.2%, +0.9%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.8% | [-1.1%, -0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 21.0 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.94 ms | 8.94 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.5 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.3 ms | 20.0 ms | -1.3% | [-1.9%, -0.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.74 ms | 2.73 ms | -0.3% | [-1.7%, +1.2%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.74 ms | 6.74 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.4% | [+0.4%, +0.5%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.71 ms | 2.72 ms | +0.4% | [+0.3%, +0.4%] | - | ~ | device 7.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.42 ms | 6.46 ms | +0.6% | [+0.1%, +1.2%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.7 ms | 33.9 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.2 ms | 68.5 ms | +0.4% | [+0.1%, +0.8%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.5 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.7 ms | -0.4% | [-1.0%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.2 ms | 38.0 ms | -0.5% | [-1.3%, +0.3%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.7 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.63 ms | 9.67 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.8 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.32 ms | 3.28 ms | -1.1% | [-1.9%, -0.4%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.56 ms | 2.58 ms | +0.5% | [+0.0%, +1.0%] | +0.0% | ~ | device 3.5x faster than the host (was 3.6x) |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.56 ms | 4.57 ms | +0.1% | [-0.6%, +0.9%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.03 ms | 3.03 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 674 µs | 679 µs | +0.7% | [-0.2%, +1.5%] | - | ~ | device 16.6x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.4% | [-1.0%, +0.2%] | - | ~ | device 14.4x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 644 µs | 642 µs | -0.3% | [-0.9%, +0.3%] | - | ~ | device 17.3x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.22 ms | -0.7% | [-1.6%, +0.3%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 612 µs | 611 µs | -0.2% | [-1.3%, +1.0%] | - | ~ | device 15.9x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.11 ms | -1.0% | [-1.7%, -0.4%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 12.0x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.47 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.08 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 611 µs | 609 µs | -0.2% | [-5.3%, +5.1%] | - | ~ | device 2.5x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 696 µs | 660 µs | -5.2% | [-9.4%, -0.5%] | - | faster | device 2.7x faster than the host (was 2.7x); tape/fused 1.28x (was 1.21x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 689 µs | 673 µs | -2.2% | [-2.8%, -1.6%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 840 µs | 844 µs | +0.5% | [-0.0%, +1.1%] | - | ~ | device 3.6x faster than the host (was 3.7x) |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.1 ms | 16.9 ms | -1.0% | [-1.6%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 14.8 ms | -0.4% | [-0.8%, +0.0%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.4 ms | 16.4 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.4 ms | 24.4 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.2% | [-0.3%, +0.8%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.1 ms | +0.5% | [+0.4%, +0.7%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.25 ms | -15.5% | [-16.0%, -15.0%] | - | faster | device 1.3x faster than the host (was 1.1x); tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.60 ms | 2.38 ms | -8.7% | [-9.1%, -8.3%] | - | faster | device 0.8x faster than the host (was 0.8x); tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.39 ms | -13.2% | [-13.4%, -12.9%] | - | faster | device 1.9x faster than the host (was 1.7x) |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.71 ms | 2.50 ms | -7.9% | [-7.9%, -7.8%] | - | faster | device 1.3x faster than the host (was 1.2x) |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.59 ms | 2.54 ms | -2.0% | [-2.2%, -1.8%] | - | ~ | device 2.6x faster than the host (was 2.9x); marched/traced 3.55x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.03 ms | 3.00 ms | -1.0% | [-1.1%, -0.9%] | - | ~ | device 2.3x faster than the host (was 2.7x); marched/traced 4.61x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.05 ms | 9.02 ms | -0.3% | [-0.7%, +0.2%] | - | ~ | device 4.9x faster than the host (was 4.3x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.8 ms | -0.6% | [-0.7%, -0.6%] | - | ~ | device 3.7x faster than the host (was 3.2x) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.21 ms | +1.3% | [+1.2%, +1.5%] | - | ~ | device 3.8x faster than the host; marched/traced 15.85x (was 16.46x) |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.45 ms | 5.46 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.8x faster than the host; marched/traced 12.74x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 35.0 ms | -2.4% | [-2.8%, -2.1%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.5 ms | 69.6 ms | -2.7% | [-2.9%, -2.6%] | - | ~ | device 1.7x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.25 ms | 5.20 ms | -0.8% | [-0.9%, -0.7%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.81 ms | 8.72 ms | -1.1% | [-1.3%, -0.9%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.28 ms | 6.13 ms | -2.4% | [-2.8%, -2.1%] | - | ~ | device 1.2x faster than the host; marched/traced 25.69x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 0.8x faster than the host; marched/traced 24.52x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 162 ms | 157 ms | -2.7% | [-3.1%, -2.3%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 254 ms | 259 ms | +1.9% | [+1.6%, +2.3%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.85 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 1.8x faster than the host; marched/traced 23.52x (was 22.55x) |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 1.3x faster than the host; marched/traced 17.88x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 109 ms | 114 ms | +4.4% | [+3.2%, +5.7%] | - | minor | device 0.6x faster than the host (was 0.7x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 219 ms | 219 ms | -0.2% | [-1.8%, +1.4%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.1 ms | -1.5% | [-2.0%, -1.0%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 1.1x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 680 µs | 670 µs | -1.4% | [-2.6%, -0.2%] | - | ~ | device 17.2x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 677 µs | 671 µs | -1.0% | [-1.4%, -0.5%] | - | ~ | device 17.2x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 14.9x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 690 µs | 680 µs | -1.4% | [-2.2%, -0.7%] | - | ~ | device 3.8x faster than the host (was 4.8x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 840 µs | 840 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.6x faster than the host (was 4.5x) |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 629 µs | 609 µs | -3.2% | [-3.4%, -3.0%] | - | faster | device 3.3x faster than the host (was 4.3x); hit/crossing 1.12x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 647 µs | 636 µs | -1.8% | [-2.7%, -0.9%] | - | ~ | device 3.8x faster than the host (was 4.8x); hit/crossing 1.32x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.69 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.4% | [-0.5%, -0.4%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.17 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 10.0x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.11 ms | 9.08 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 266 ms | 57.8 ms | -78.3% | [-78.9%, -77.6%] | - | faster | device 3.1x faster than the host (was 0.7x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 216 ms | 84.1 ms | -61.0% | [-61.4%, -60.7%] | - | faster | device 2.2x faster than the host (was 0.9x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 54.9 ms | 47.8 ms | -12.9% | [-13.9%, -11.9%] | - | faster | device 3.0x faster than the host (was 2.6x); hit/crossing 1.21x (was 4.85x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 70.9 ms | 63.6 ms | -10.3% | [-10.6%, -9.9%] | - | faster | device 2.5x faster than the host (was 2.2x); hit/crossing 1.32x (was 3.04x) |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.2 ms | +1.3% | [+0.7%, +1.9%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.6 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.3% | [-0.9%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.61 ms | 1.39 ms | -13.5% | [-13.8%, -13.1%] | - | faster | device 2.0x faster than the host (was 2.3x) |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.71 ms | 2.48 ms | -8.5% | [-8.6%, -8.3%] | - | faster | device 1.3x faster than the host (was 1.5x) |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.11 ms | -14.0% | [-14.4%, -13.7%] | - | faster | device 1.9x faster than the host (was 2.5x); hit/crossing 1.25x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.35 ms | 1.17 ms | -13.4% | [-14.3%, -12.6%] | - | faster | device 2.1x faster than the host (was 2.6x); hit/crossing 2.12x (was 2.00x) |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.9 ms | 64.5 ms | -0.7% | [-0.7%, -0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 122 ms | 123 ms | +0.7% | [-1.1%, +2.5%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.6 ms | 58.6 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 112 ms | 112 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.09x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 482 ms | 462 ms | -4.1% | [-8.6%, +0.6%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 710 ms | 703 ms | -1.1% | [-2.6%, +0.5%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 396 ms | 390 ms | -1.5% | [-6.1%, +3.1%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.18x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 550 ms | 533 ms | -3.0% | [-3.9%, -2.1%] | - | ~ | device 0.3x faster than the host (was 0.3x); hit/crossing 1.32x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.59 ms | 3.53 ms | -1.4% | [-2.7%, -0.1%] | - | ~ | device 1.2x faster than the host; marched/traced 3.41x (was 5.47x) |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.86 ms | 4.77 ms | -1.8% | [-2.3%, -1.3%] | - | ~ | device 0.9x faster than the host; marched/traced 5.58x (was 8.29x) |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.6 ms | 12.1 ms | -38.6% | [-38.9%, -38.3%] | - | faster | device 3.9x faster than the host (was 2.4x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.3 ms | 26.6 ms | -33.9% | [-34.1%, -33.7%] | - | faster | device 2.0x faster than the host (was 1.3x) |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.26 ms | +1.3% | [+0.3%, +2.3%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.84 ms | 1.85 ms | +0.5% | [+0.1%, +1.0%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.8 µs | 97.3 µs | +1.6% | [+0.2%, +2.9%] | - | ~ | device 14.7x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 153 µs | 151 µs | -1.0% | [-5.5%, +3.7%] | - | ~ | device 10.6x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 800 µs | 793 µs | -0.9% | [-1.1%, -0.6%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 936 µs | 927 µs | -0.9% | [-1.2%, -0.7%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.4 ms | 35.4 ms | +2.9% | [-2.0%, +7.9%] | - | ~ | device 0.1x faster than the host; marched/traced 3.39x (was 3.58x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.6 ms | 37.2 ms | +1.5% | [+0.8%, +2.2%] | - | ~ | device 0.1x faster than the host; marched/traced 6.32x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 123 ms | 120 ms | -2.7% | [-5.3%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 236 ms | 235 ms | -0.5% | [-3.4%, +2.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 17.0 ms | 16.5 ms | -3.2% | [-6.3%, +0.1%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 21.9 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.13 ms | 1.04 ms | -7.9% | [-12.0%, -3.6%] | - | faster | device 1.4x faster than the host (was 1.3x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.37 ms | 2.35 ms | -0.8% | [-1.4%, -0.1%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.95 ms | 6.00 ms | +0.7% | [+0.5%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.71 ms | 6.72 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 2.2x faster than the host; marched/traced 2.69x (was 4.38x) |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.4 ms | 28.2 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 1.9x faster than the host; marched/traced 3.65x (was 5.63x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 100 ms | 61.6 ms | -38.4% | [-38.9%, -38.0%] | - | faster | device 3.6x faster than the host (was 2.2x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 160 ms | 103 ms | -35.5% | [-36.4%, -34.6%] | - | faster | device 2.3x faster than the host (was 1.5x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.8 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.5 ms | 17.4 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 304 ms | 296 ms | -2.8% | [-5.7%, +0.4%] | - | ~ | device 0.2x faster than the host; marched/traced 1.90x (was 1.84x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 355 ms | 353 ms | -0.6% | [-3.3%, +2.2%] | - | ~ | device 0.2x faster than the host; marched/traced 2.81x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 560 ms | 563 ms | +0.6% | [-0.1%, +1.4%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 999 ms | 993 ms | -0.6% | [-1.8%, +0.6%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 192 ms | 181 ms | -5.6% | [-9.4%, -1.8%] | - | faster | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 243 ms | 233 ms | -3.9% | [-5.6%, -2.2%] | - | faster | device 0.2x faster than the host (was 0.2x) |
| `Render/csg/frame/float/hostdevice` | 4.58 ms | 4.14 ms | -9.7% | [-9.7%, -9.6%] | - | faster | device 2.7x faster than the host (was 3.0x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.57 ms | 4.15 ms | -9.4% | [-9.5%, -9.2%] | - | faster | device 2.7x faster than the host (was 3.0x) |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.88 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.97 ms | 7.60 ms | -4.7% | [-4.9%, -4.6%] | - | faster | device 1.6x faster than the host (was 1.9x) |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.94 ms | 7.57 ms | -4.6% | [-4.8%, -4.4%] | - | faster | device 1.6x faster than the host (was 1.9x) |
| `Render/mesh/frame/double/hostdevice` | 23.5 ms | 23.2 ms | -1.0% | [-2.1%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/round-cone/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/tube/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/wedge/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/ray-forms/firstHit/float/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/round-cone/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/tube/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/wedge/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/ray-forms/firstHit/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 887 µs | 883 µs | -0.5% | [-1.6%, +0.7%] | +0.0% | ~ | device 10.6x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.64 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | +0.6% | [+0.2%, +1.1%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.0 ms | 13.1 ms | +0.6% | [+0.4%, +0.8%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.3 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 13.9 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.01 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 7.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.74 ms | 4.77 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 5.1x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.1 ms | 24.1 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 23.0 ms | 22.9 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.4 ms | 30.4 ms | -0.3% | [-0.3%, -0.2%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.1 ms | 44.3 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.8 ms | 45.6 ms | -0.5% | [-1.6%, +0.7%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 121 ms | 125 ms | +3.7% | [+1.5%, +5.9%] | +0.0% | minor | device 0.7x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 127 ms | 127 ms | -0.3% | [-2.8%, +2.2%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 127 ms | 130 ms | +2.3% | [+0.5%, +4.0%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 133 ms | 132 ms | -0.8% | [-2.3%, +0.7%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 10.6x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.2 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 0.6x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 218 µs | +0.5% | [+0.2%, +0.8%] | - | ~ | device 11.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 47.9 µs | -0.1% | [-0.7%, +0.5%] | - | ~ | device 23.6x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 361 µs | 359 µs | -0.7% | [-1.1%, -0.3%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.01 ms | 2.01 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.7 µs | 72.3 µs | -0.6% | [-1.0%, -0.2%] | - | ~ | device 7.2x faster than the host (was 7.5x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.4 µs | 47.4 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 4.8x faster than the host (was 5.9x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 653 µs | 651 µs | -0.3% | [-0.4%, -0.2%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 12.2x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 473 µs | 475 µs | +0.5% | [+0.1%, +0.9%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 48.2 µs | +0.4% | [+0.2%, +0.7%] | - | ~ | device 26.6x faster than the host (was 27.6x) |
| `Tape/csg/signedDistance/double/hostdevice` | 855 µs | 857 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [+0.0%, +0.1%] | - | ~ | device 1.5x faster than the host (was 1.6x) |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.72 ms | 6.75 ms | +0.6% | [+0.5%, +0.6%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 191 µs | +0.2% | [-0.0%, +0.4%] | - | ~ | device 2.8x faster than the host (was 2.9x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 165 µs | 163 µs | -0.9% | [-2.0%, +0.3%] | - | ~ | device 1.8x faster than the host (was 1.8x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.77 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.12 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.11 ms | 2.10 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 0.9x faster than the host |


The probes differ between the sides (each side ran its own): Performance/Shapes/main.cpp

<!-- END receipt cuda -->
