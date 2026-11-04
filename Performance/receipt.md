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
base: 5d1a826
base-fingerprint: 83ce77fa4c17a149f4eefac9983ef062abf7eaae
head: 80414d8
head-fingerprint: fd6d574e22a7504478fd46d99d3af214f5c008f3
cases: 190
rounds: 2
reps: 3
date: 2026-11-03
-->

## Host comparison: red

**Accepted:** Accept noise and checksum

`80414d8` against `5d1a826` (origin/main), 2026-11-03.

**VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 1 major, 0 minor, 0 faster, 0 differ, 0 lost, 190 cases

- base: 5d1a826 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 80414d8 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 332 ms | 331 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 177 ms | -0.6% | [-0.9%, -0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 514 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 353 ms | 347 ms | -1.5% | [-3.1%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 561 ms | 563 ms | +0.2% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.6 ms | 41.5 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.6 ms | 75.4 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.3 ms | 55.1 ms | -0.4% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.2 ms | 51.7 ms | +0.9% | [-0.1%, +1.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.3 ms | 65.4 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.5 ms | 54.7 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.3 ms | 61.6 ms | +0.5% | [-2.4%, +3.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.8 ms | 74.4 ms | +2.1% | [-1.7%, +6.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 75.3 ms | 75.6 ms | +0.4% | [-0.6%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.6 ms | 77.8 ms | +0.4% | [+0.0%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 71.4 ms | 67.0 ms | -6.2% | [-12.0%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.9 ms | 101 ms | +0.6% | [+0.1%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.9 ms | 83.8 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | +0.6% | [+0.2%, +0.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.5 ms | 19.5 ms | -0.3% | [-1.7%, +1.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 44.5 ms | 44.5 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.9 ms | 72.8 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.1 ms | 20.5 ms | +2.0% | [+1.7%, +2.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 46.9 ms | 45.8 ms | -2.4% | [-2.6%, -2.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 80.8 ms | 80.6 ms | -0.3% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.47 ms | 4.49 ms | +0.6% | [+0.3%, +0.9%] | - | ~ | tape/fused 2.86x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.2 ms | -0.4% | [-0.8%, -0.0%] | - | ~ | tape/fused 2.06x |
| `Fused/chain16/signedDistance/float/host` | 970 µs | 962 µs | -0.8% | [-1.5%, +0.0%] | - | ~ | tape/fused 16.76x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.3 ms | 45.9 ms | -0.9% | [-1.2%, -0.5%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.1 ms | -0.3% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.59 ms | 4.61 ms | +0.5% | [-0.3%, +1.4%] | - | ~ | tape/fused 3.60x (was 2.85x) |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | -0.0% | [-0.9%, +0.8%] | - | ~ | tape/fused 2.08x |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.92 ms | +0.6% | [-0.3%, +1.4%] | - | ~ | tape/fused 4.16x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.6 ms | 54.1 ms | +2.8% | [-0.9%, +6.6%] | +0.0% | ~ | tape/fused 0.98x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.0 ms | 53.1 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.5 ms | 82.5 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 228 ms | 228 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 114 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 261 ms | 261 ms | +0.1% | [-0.8%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.5 ms | 71.6 ms | +1.6% | [+0.2%, +2.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 214 ms | 214 ms | +0.3% | [-0.6%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.8 ms | 96.8 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.9 ms | 29.8 ms | -0.4% | [-2.9%, +2.2%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 755 ms | 757 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | -0.4% | [-0.4%, -0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.3 ms | 43.3 ms | -0.0% | [-2.7%, +2.7%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 855 ms | 856 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.6 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.0 ms | 27.0 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 320 ms | 321 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | checksum differs (13363.03424 -> 13363.01869) |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.3 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | tape/fused 3.55x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.6 ms | 99.6 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.5 ms | 29.8 ms | +0.8% | [+0.2%, +1.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.4 ms | 30.3 ms | -0.3% | [-2.2%, +1.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 359 ms | 363 ms | +1.0% | [-0.2%, +2.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 261 ms | 259 ms | -0.6% | [-4.8%, +3.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 98.5 ms | 97.2 ms | -1.3% | [-2.8%, +0.2%] | +0.0% | ~ | tape/fused 3.73x |
| `MeshCSG/smooth-fused/frame/double/host` | 106 ms | 109 ms | +2.3% | [-0.0%, +4.7%] | +0.0% | ~ | tape/fused 2.38x |
| `MeshReconcile/armadillo/reconcile/float/host` | - | - | - | - | - | new |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | - | - | - | - | - | new | reconcile/scheduled 3.19x |
| `MeshReconcile/armadillo/reconcile/double/host` | - | - | - | - | - | new |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | - | - | - | - | - | new | reconcile/scheduled 2.74x |
| `PointCloudBVH/uniform/build/float/host` | 53.2 ms | 53.2 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 143 ms | +0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 354 ms | 354 ms | +0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 77.9 ms | 77.9 ms | -0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.62 ms | 6.63 ms | +0.3% | [-1.2%, +1.7%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 56.3 ms | 56.3 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 153 ms | 154 ms | +0.4% | [-0.2%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 380 ms | 380 ms | -0.2% | [-0.6%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.2 ms | 87.1 ms | -0.1% | [-0.8%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.92 ms | 6.87 ms | -0.7% | [-1.0%, -0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 12.9 ms | 13.6 ms | +5.8% | [-0.8%, +13.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 477 ms | 476 ms | -0.2% | [-0.4%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 142 ms | 142 ms | +0.3% | [-0.3%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 802 ms | 804 ms | +0.3% | [-0.2%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 15.2 ms | +2.1% | [-0.8%, +5.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | -0.2% | [-0.5%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 493 ms | 493 ms | -0.1% | [-0.2%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 145 ms | +0.2% | [-0.1%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 787 ms | 792 ms | +0.6% | [+0.3%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.3 ms | 54.4 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 129 ms | 129 ms | +0.2% | [-0.7%, +1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 262 ms | 261 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 144 ms | 144 ms | -0.5% | [-0.8%, -0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 129 ms | 129 ms | -0.3% | [-0.5%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.8 ms | 57.8 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 137 ms | 139 ms | +1.4% | [-1.5%, +4.3%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 290 ms | 290 ms | -0.2% | [-0.8%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 169 ms | 163 ms | -3.1% | [-8.5%, +2.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 145 ms | 144 ms | -0.8% | [-1.9%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.6 ms | 11.8 ms | +1.9% | [+0.6%, +3.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 115 ms | 114 ms | -0.4% | [-1.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 317 ms | 317 ms | +0.0% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 155 ms | -0.1% | [-1.0%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 321 ms | 320 ms | -0.3% | [-0.7%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.5 ms | 13.5 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 116 ms | 116 ms | -0.3% | [-0.5%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 351 ms | 349 ms | -0.4% | [-0.7%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 166 ms | 167 ms | +0.2% | [-1.3%, +1.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 308 ms | -0.2% | [-0.4%, -0.1%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.1 ms | 35.1 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.6 ms | 38.6 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.20 ms | 8.27 ms | +0.8% | [+0.0%, +1.6%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.50 ms | 9.32 ms | -1.9% | [-4.7%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.5 ms | 46.5 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.5 ms | 46.3 ms | +1.8% | [-0.1%, +3.8%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.5 ms | 41.0 ms | +1.3% | [+1.0%, +1.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.7 ms | 78.6 ms | -0.2% | [-0.2%, -0.2%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.5 ms | 65.8 ms | +0.5% | [+0.0%, +0.9%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.89 ms | 5.83 ms | -1.0% | [-2.0%, -0.1%] | - | ~ | tape/fused 1.37x |
| `Ray/csg-tape/firstHit/float/host` | 7.88 ms | 7.97 ms | +1.2% | [+0.5%, +1.9%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.6 ms | 53.0 ms | +0.6% | [-0.6%, +1.9%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.3 ms | 51.3 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.0 ms | 44.9 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.1 ms | 87.7 ms | -0.4% | [-1.2%, +0.5%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.8 ms | 73.8 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 6.02 ms | 6.01 ms | -0.3% | [-2.5%, +2.1%] | - | ~ | tape/fused 1.39x |
| `Ray/csg-tape/firstHit/double/host` | 8.40 ms | 8.33 ms | -0.8% | [-1.7%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.8 ms | 18.9 ms | +0.6% | [-0.7%, +1.9%] | - | ~ | marched/traced 6.30x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 119 ms | -0.7% | [-1.4%, +0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 21.8 ms | -0.8% | [-1.1%, -0.5%] | +0.0% | ~ | marched/traced 7.07x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 154 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.7 ms | 35.4 ms | -1.1% | [-2.6%, +0.5%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.5 ms | 19.5 ms | +0.1% | [-6.2%, +6.8%] | - | ~ | marched/traced 6.47x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 127 ms | 126 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 24.5 ms | +4.0% | [-0.3%, +8.3%] | +0.0% | ~ | marched/traced 6.53x (was 6.79x) |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 160 ms | 160 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.0 ms | 38.0 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.4 ms | 46.4 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.4 ms | 46.4 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.3 ms | 10.2 ms | -0.7% | [-1.7%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.78 ms | 8.70 ms | -0.9% | [-1.3%, -0.6%] | +0.0% | ~ | hit/crossing 1.17x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 181 ms | 181 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 160 ms | 162 ms | +0.8% | [+0.3%, +1.4%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.8 ms | 68.8 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.5 ms | 56.4 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.9 ms | 52.5 ms | -0.7% | [-1.5%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.8 ms | 52.5 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.1 ms | 11.1 ms | -0.6% | [-1.7%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.40 ms | 9.36 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ | hit/crossing 1.18x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 206 ms | 207 ms | +0.4% | [+0.0%, +0.9%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 182 ms | 185 ms | +1.4% | [+1.2%, +1.5%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 79.0 ms | 77.6 ms | -1.8% | [-3.0%, -0.5%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 65.2 ms | 64.0 ms | -1.8% | [-2.0%, -1.5%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.0 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | marched/traced 4.69x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.9 ms | 51.7 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 7.99 ms | 7.98 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.93 ms | 1.93 ms | +0.2% | [-0.6%, +1.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.93 ms | 6.90 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.7 ms | -0.8% | [-1.1%, -0.5%] | +0.0% | ~ | marched/traced 5.26x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.3 ms | 61.3 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.55 ms | 8.65 ms | +1.2% | [+0.7%, +1.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.95 ms | 1.94 ms | -0.7% | [-1.3%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.25 ms | 7.16 ms | -1.2% | [-1.3%, -1.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 240 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 329 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 236 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 254 ms | 250 ms | -1.4% | [-1.6%, -1.2%] | +0.0% | ~ | marched/traced 1.47x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 367 ms | 367 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 252 ms | 249 ms | -1.1% | [-1.4%, -0.7%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.7 ms | 23.8 ms | +0.5% | [-0.1%, +1.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.5 ms | 23.6 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 18.7 ms | +0.5% | [-0.8%, +1.8%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.3 ms | 30.2 ms | -0.5% | [-1.4%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 30.1 ms | 30.2 ms | +0.5% | [-0.5%, +1.4%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.4 ms | 20.4 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 13.0 ms | 12.9 ms | -0.8% | [-1.9%, +0.3%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.60 ms | 4.56 ms | -0.9% | [-0.9%, -0.8%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.0 ms | 25.1 ms | +0.5% | [-0.4%, +1.5%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.1 ms | 40.3 ms | +0.5% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.75 ms | 1.73 ms | -1.4% | [-2.6%, -0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 140 µs | 138 µs | -1.7% | [-3.7%, +0.3%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.8 ms | 29.8 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.65 ms | -0.1% | [-0.6%, +0.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.2 ms | 16.1 ms | -0.2% | [-1.0%, +0.8%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.63 ms | 4.66 ms | +0.7% | [+0.3%, +1.1%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 16.6 ms | +27.0% | [+0.5%, +53.7%] | +0.0% | **MAJOR** |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.59 ms | 4.55 ms | -0.7% | [-2.0%, +0.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.4 ms | 26.5 ms | +0.5% | [-0.2%, +1.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.3 ms | 42.4 ms | +0.0% | [-0.8%, +0.9%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.77 ms | 1.76 ms | -0.4% | [-2.0%, +1.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 277 µs | 255 µs | -7.8% | [-14.9%, +0.2%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.9 ms | 29.9 ms | +0.0% | [-1.0%, +1.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.74 ms | 7.76 ms | +0.3% | [-0.2%, +0.7%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.2 ms | 16.3 ms | +0.3% | [-0.7%, +1.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.73 ms | 7.72 ms | -0.1% | [-0.2%, -0.0%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/MeshReconcile/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 5d1a826
base-fingerprint: 83ce77fa4c17a149f4eefac9983ef062abf7eaae
head: 80414d8
head-fingerprint: fd6d574e22a7504478fd46d99d3af214f5c008f3
cases: 280
rounds: 2
reps: 3
date: 2026-11-03
-->

## Device comparison (CUDA): yellow

**Accepted:** Accept as noise

`80414d8` against `5d1a826` (origin/main), 2026-11-03.

**VERDICT: 5 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 5 minor, 6 faster, 0 differ, 0 lost, 280 cases

- base: 5d1a826 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 80414d8 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.60 ms | 1.60 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 22.7x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.40 ms | -0.0% | [-0.8%, +0.8%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.07 ms | 1.04 ms | -3.0% | [-4.7%, -1.3%] | - | faster | device 19.7x faster than the host (was 19.1x) |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.30 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.26 ms | 6.20 ms | -1.0% | [-2.9%, +1.0%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.7 ms | 11.7 ms | -0.2% | [-1.1%, +0.8%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.3 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 56.8 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.3 ms | 20.9 ms | +2.8% | [+1.4%, +4.3%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.8 ms | 48.2 ms | +0.8% | [-0.7%, +2.3%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.6 ms | 61.7 ms | +0.1% | [-2.0%, +2.4%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 107 ms | -3.3% | [-4.0%, -2.5%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.41 ms | 8.40 ms | -0.0% | [-1.5%, +1.5%] | +0.0% | ~ | device 4.8x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.75 ms | 9.93 ms | +1.8% | [+0.0%, +3.7%] | +0.0% | ~ | device 8.2x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 385 µs | 384 µs | -0.3% | [-0.7%, +0.2%] | - | ~ | device 15.3x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 842 µs | 840 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.50 ms | 4.53 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.67 ms | 4.67 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 578 µs | 579 µs | +0.2% | [-0.3%, +0.7%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.37 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.77 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.02 ms | 7.01 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 898 µs | 898 µs | -0.0% | [-0.9%, +0.8%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.46 ms | 9.48 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 320 µs | 321 µs | +0.3% | [-0.5%, +1.2%] | - | ~ | device 11.3x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 5.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.55 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 518 ms | 516 ms | -0.5% | [-1.4%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 48.0 µs | 48.0 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 23.5x faster than the host; tape/fused 4.53x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.0 µs | 49.9 µs | -0.3% | [-0.6%, -0.0%] | - | ~ | device 43.6x faster than the host; tape/fused 7.23x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.4 µs | 48.4 µs | -0.1% | [-1.2%, +0.9%] | - | ~ | device 9.3x faster than the host (was 10.1x); tape/fused 9.80x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.2 ms | -0.7% | [-0.9%, -0.4%] | - | ~ | device 6.8x faster than the host (was 8.9x); tape/fused 1.83x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.4 ms | 31.3 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.56 ms | 1.55 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 283 ms | 282 ms | -0.4% | [-2.0%, +1.2%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 286 ms | 285 ms | -0.1% | [-0.8%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.28 ms | 8.35 ms | +0.8% | [-0.4%, +2.1%] | +0.0% | ~ | device 12.3x faster than the host (was 10.7x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.32 ms | 8.27 ms | -0.5% | [-2.7%, +1.7%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.20 ms | 8.21 ms | +0.0% | [-0.7%, +0.8%] | +0.0% | ~ | device 11.7x faster than the host (was 10.7x); default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.13 ms | 8.34 ms | +2.7% | [+0.9%, +4.5%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 688 µs | 689 µs | +0.1% | [+0.1%, +0.1%] | +0.0% | ~ | device 4.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.72 ms | 1.72 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.06 ms | 1.06 ms | +0.5% | [+0.4%, +0.7%] | +0.0% | ~ | device 3.1x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 800 µs | 798 µs | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ | device 4.1x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 832 µs | 828 µs | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ | device 10.7x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 970 µs | 972 µs | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | device 9.1x faster than the host; default/this 0.85x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 834 µs | 834 µs | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 819 µs | 806 µs | -1.5% | [-2.6%, -0.4%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 161 ms | 156 ms | -3.2% | [-6.3%, -0.0%] | +0.0% | faster | device 0.6x faster than the host (was 0.6x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 161 ms | 161 ms | -0.4% | [-4.3%, +3.7%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.97x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 158 ms | 160 ms | +1.4% | [-3.4%, +6.4%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.97x (was 1.02x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 163 ms | 161 ms | -1.4% | [-6.1%, +3.6%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.97x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.77 ms | 1.77 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.90x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.94 ms | -0.5% | [-0.9%, -0.0%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.83x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.68 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.28 ms | 7.27 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.31 ms | 7.34 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.26 ms | 7.26 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.34 ms | 7.33 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.41 ms | 8.47 ms | +0.7% | [-0.0%, +1.4%] | - | ~ | device 11.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.5 ms | -0.5% | [-1.1%, +0.2%] | - | ~ | device 7.2x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 169 ms | 166 ms | -1.4% | [-3.5%, +0.8%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 283 ms | 278 ms | -1.6% | [-3.3%, +0.1%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshSDF/query-morton/float/hostdevice` | 24.6 ms | 24.3 ms | -1.4% | [-2.1%, -0.7%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.6 ms | 58.5 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 160 ms | 160 ms | -0.2% | [-3.7%, +3.5%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 242 ms | 242 ms | -0.1% | [-1.9%, +1.8%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.93 ms | 2.94 ms | +0.1% | [-0.6%, +0.9%] | - | ~ | device 3.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.15 ms | 5.16 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.5 ms | 14.5 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 121 ms | 122 ms | +0.3% | [-3.5%, +4.2%] | - | ~ | device 1.7x faster than the host (was 2.1x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 170 ms | 173 ms | +1.8% | [-1.2%, +4.8%] | - | ~ | device 1.5x faster than the host (was 1.3x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 375 ms | 380 ms | +1.2% | [-1.1%, +3.5%] | - | ~ | device 2.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.0 ms | 24.7 ms | -1.3% | [-2.3%, -0.4%] | - | ~ | device 2.0x faster than the host (was 2.2x); tape/fused 4.94x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.7 ms | 26.7 ms | -0.1% | [-0.7%, +0.6%] | - | ~ | device 2.2x faster than the host; tape/fused 6.47x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.5 ms | 71.4 ms | -0.2% | [-1.3%, +0.9%] | - | ~ | device 4.1x faster than the host (was 4.4x); tape/fused 5.32x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 23.4 ms | 22.4 ms | -4.4% | [-6.1%, -2.6%] | - | faster | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.2 ms | 44.5 ms | +0.7% | [-2.2%, +3.6%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 92.2 ms | 95.4 ms | +3.5% | [-0.4%, +7.4%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 526 ms | 547 ms | +4.0% | [-1.4%, +10.0%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 882 ms | 860 ms | -2.5% | [-4.2%, -0.8%] | - | ~ | device 0.4x faster than the host (was 0.2x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.26 s | 3.26 s | +0.2% | [-0.9%, +1.3%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 201 ms | 200 ms | -0.5% | [-3.2%, +2.2%] | - | ~ | device 0.2x faster than the host; tape/fused 2.73x (was 2.61x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 296 ms | 306 ms | +3.5% | [+0.9%, +6.0%] | - | minor | device 0.2x faster than the host (was 0.2x); tape/fused 2.81x (was 2.98x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 591 ms | 603 ms | +2.2% | [-2.5%, +6.9%] | - | ~ | device 0.5x faster than the host; tape/fused 5.41x |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 761 µs | 762 µs | +0.1% | [-0.2%, +0.5%] | - | ~ | device 9.8x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.17 ms | 2.16 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 4.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.16 ms | 1.17 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 7.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.20 ms | 6.20 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 3.89 ms | 3.88 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.16 ms | 7.19 ms | +0.4% | [+0.0%, +0.7%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.89 ms | 7.87 ms | -0.3% | [-0.9%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.4 ms | 13.4 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 21.0 ms | 21.0 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 12.0 ms | 12.0 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.46 ms | 1.46 ms | -0.0% | [-1.5%, +1.4%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.90 ms | 3.88 ms | -0.5% | [-1.1%, +0.1%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 525 µs | 524 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 15.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.43 ms | 4.44 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 6.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.32 ms | 4.36 ms | +0.8% | [-0.6%, +2.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.09 ms | 8.09 ms | +0.0% | [-1.0%, +1.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.7 ms | -0.9% | [-1.5%, -0.2%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.01 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.7 ms | 17.6 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.9% | [-0.2%, +2.1%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.82 ms | 1.81 ms | -0.7% | [-1.7%, +0.5%] | - | ~ | device 9.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 5.55 ms | 5.55 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 4.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 921 µs | 921 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 9.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.70 ms | 3.70 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 5.46 ms | 5.47 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 4.1x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.9 ms | 16.8 ms | -0.5% | [-0.9%, -0.0%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.4 ms | 21.5 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.97 ms | 8.99 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.6 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.4 ms | 20.6 ms | +1.0% | [-1.0%, +3.0%] | - | ~ | device 1.1x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.75 ms | 2.72 ms | -1.2% | [-1.8%, -0.6%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.72 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.39 ms | 1.39 ms | +0.3% | [-0.5%, +1.2%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.72 ms | 2.72 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 7.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.49 ms | 6.53 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 2.9x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.8 ms | 33.9 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.3 ms | 68.4 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.5 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.7 ms | 20.8 ms | +0.4% | [+0.0%, +0.7%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.1 ms | 38.0 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.67 ms | 9.64 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.9 ms | 12.8 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.29 ms | 3.27 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.58 ms | 2.57 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.55 ms | 4.60 ms | +1.2% | [+0.6%, +1.8%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.03 ms | 3.04 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 677 µs | 670 µs | -1.1% | [-1.7%, -0.4%] | - | ~ | device 16.9x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 14.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 643 µs | 637 µs | -0.9% | [-1.6%, -0.3%] | - | ~ | device 17.5x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | -0.3% | [-1.0%, +0.3%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 608 µs | 606 µs | -0.4% | [-1.6%, +0.8%] | - | ~ | device 16.2x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | +0.2% | [-0.8%, +1.1%] | - | ~ | device 14.8x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.47 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | -0.5% | [-1.4%, +0.4%] | - | ~ | device 11.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.17 ms | -0.3% | [-0.5%, +0.0%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 631 µs | 610 µs | -3.3% | [-6.4%, -0.1%] | - | faster | device 2.5x faster than the host; tape/fused 1.13x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 693 µs | 692 µs | -0.0% | [-10.0%, +11.0%] | - | ~ | device 2.7x faster than the host; tape/fused 1.21x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 690 µs | 687 µs | -0.5% | [-0.7%, -0.3%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 838 µs | 836 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 3.7x faster than the host (was 3.6x) |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.3% | [-0.9%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.2% | [-1.3%, +1.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.4 ms | 16.5 ms | +0.7% | [+0.5%, +1.0%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.5 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.2% | [-0.5%, +1.0%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 22.9 ms | 23.0 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.60 ms | 2.61 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.72 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.56 ms | 2.56 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.0x faster than the host; marched/traced 3.54x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 2.7x faster than the host; marched/traced 4.61x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.03 ms | 9.04 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.18 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 3.9x faster than the host; marched/traced 16.52x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.44 ms | 5.45 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 2.8x faster than the host; marched/traced 13.13x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 36.0 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 2.0x faster than the host (was 2.3x) |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.5 ms | 71.6 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.24 ms | 5.25 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.82 ms | 8.82 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.24 ms | 6.21 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 1.3x faster than the host; marched/traced 25.98x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 0.8x faster than the host; marched/traced 24.18x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 161 ms | 161 ms | +0.4% | [-1.2%, +2.1%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 254 ms | 257 ms | +1.1% | [+0.2%, +2.1%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.84 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 23.03x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.3x faster than the host; marched/traced 18.14x (was 17.60x) |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 111 ms | 112 ms | +0.6% | [-2.2%, +3.6%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 215 ms | 222 ms | +3.2% | [+2.4%, +4.1%] | - | minor | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.2 ms | 19.4 ms | +1.0% | [+0.6%, +1.4%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.8 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 674 µs | 672 µs | -0.4% | [-1.8%, +1.0%] | - | ~ | device 17.2x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 673 µs | 678 µs | +0.7% | [-0.3%, +1.6%] | - | ~ | device 17.0x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 688 µs | 691 µs | +0.5% | [+0.0%, +1.0%] | - | ~ | device 4.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 844 µs | 837 µs | -0.7% | [-0.8%, -0.6%] | - | ~ | device 4.4x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 627 µs | 626 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 4.4x faster than the host; hit/crossing 1.10x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 648 µs | 646 µs | -0.3% | [-0.3%, -0.3%] | - | ~ | device 4.8x faster than the host; hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.71 ms | 4.70 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 10.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 7.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.18 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 10.1x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.08 ms | 9.11 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 267 ms | 259 ms | -2.9% | [-6.7%, +1.2%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 214 ms | 212 ms | -1.0% | [-2.5%, +0.6%] | - | ~ | device 0.9x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 54.7 ms | 55.2 ms | +0.8% | [+0.1%, +1.6%] | - | ~ | device 2.6x faster than the host; hit/crossing 4.70x (was 4.88x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 71.0 ms | 70.8 ms | -0.2% | [-1.4%, +0.9%] | - | ~ | device 2.2x faster than the host; hit/crossing 2.99x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.6% | [-1.0%, -0.3%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.3 ms | 27.4 ms | +0.4% | [+0.1%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.61 ms | 1.60 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.71 ms | 2.71 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.29 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.35 ms | 1.35 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.6x faster than the host; hit/crossing 2.00x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.8 ms | 64.6 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 124 ms | 121 ms | -1.9% | [-3.1%, -0.7%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.7 ms | 58.8 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 111 ms | 111 ms | +0.4% | [-2.7%, +3.6%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.09x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 469 ms | 480 ms | +2.4% | [-3.0%, +8.2%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 712 ms | 717 ms | +0.8% | [-1.3%, +2.9%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 409 ms | 407 ms | -0.6% | [-6.8%, +6.2%] | - | ~ | device 0.4x faster than the host (was 0.4x); hit/crossing 1.18x (was 1.15x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 538 ms | 539 ms | +0.1% | [-2.0%, +2.3%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.33x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.54 ms | 3.65 ms | +3.3% | [+2.1%, +4.4%] | - | minor | device 1.2x faster than the host; marched/traced 5.38x (was 5.60x) |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.80 ms | 4.75 ms | -1.0% | [-1.9%, -0.1%] | - | ~ | device 0.9x faster than the host; marched/traced 8.45x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.8 ms | 19.6 ms | -0.8% | [-1.4%, -0.2%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.1 ms | 40.2 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.23 ms | 1.22 ms | -0.6% | [-1.7%, +0.4%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.83 ms | 1.85 ms | +1.0% | [+0.8%, +1.3%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 94.5 µs | 95.8 µs | +1.4% | [-0.1%, +2.9%] | - | ~ | device 15.1x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 151 µs | 152 µs | +0.0% | [-0.5%, +0.6%] | - | ~ | device 11.0x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 799 µs | 799 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 936 µs | 934 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.2 ms | 36.4 ms | +3.5% | [+3.1%, +3.9%] | - | minor | device 0.1x faster than the host (was 0.1x); marched/traced 3.41x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.9 ms | 37.2 ms | +0.9% | [+0.1%, +1.8%] | - | ~ | device 0.1x faster than the host; marched/traced 6.41x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 123 ms | 124 ms | +1.2% | [-1.5%, +3.9%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 239 ms | 239 ms | -0.3% | [-3.4%, +2.9%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.9 ms | 15.8 ms | -6.1% | [-10.5%, -1.3%] | - | faster | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.6 ms | 21.8 ms | +0.8% | [+0.3%, +1.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.03 ms | 1.06 ms | +2.5% | [+0.8%, +4.3%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.32 ms | 2.35 ms | +1.2% | [-0.1%, +2.5%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.97 ms | 5.97 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.70 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.0 ms | 23.1 ms | +0.3% | [-0.5%, +1.0%] | - | ~ | device 2.2x faster than the host; marched/traced 4.36x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.6 ms | 28.7 ms | +0.5% | [-0.5%, +1.5%] | - | ~ | device 1.9x faster than the host; marched/traced 5.55x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 101 ms | 100 ms | -0.2% | [-1.6%, +1.2%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 160 ms | 160 ms | -0.5% | [-2.1%, +1.2%] | - | ~ | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.7 ms | -0.5% | [-1.3%, +0.2%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.5 ms | 17.4 ms | -0.6% | [-1.8%, +0.7%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 301 ms | 298 ms | -1.2% | [-4.0%, +1.7%] | - | ~ | device 0.2x faster than the host; marched/traced 1.92x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 345 ms | 346 ms | +0.1% | [-1.7%, +2.0%] | - | ~ | device 0.2x faster than the host; marched/traced 2.88x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 565 ms | 570 ms | +0.8% | [+0.4%, +1.3%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.00 s | 994 ms | -1.1% | [-2.0%, -0.1%] | - | ~ | device 0.2x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 185 ms | 192 ms | +3.5% | [+2.2%, +4.9%] | - | minor | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 239 ms | 240 ms | +0.4% | [-5.0%, +6.3%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.57 ms | 4.57 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.57 ms | 4.57 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.90 ms | 1.90 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 8.8x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.96 ms | 7.96 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.95 ms | 7.95 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.1 ms | 23.2 ms | +0.5% | [+0.0%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 217 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 11.4x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.2% | [-0.0%, +0.3%] | - | ~ | device 24.1x faster than the host (was 23.3x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 361 µs | 361 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.02 ms | 2.02 ms | -0.3% | [-0.3%, -0.3%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.7 µs | 72.5 µs | -0.3% | [-0.6%, +0.1%] | - | ~ | device 7.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.2% | [-1.0%, +1.4%] | - | ~ | device 4.6x faster than the host (was 5.0x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 653 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 10.7x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.5% | [+0.1%, +0.8%] | - | ~ | device 11.8x faster than the host (was 12.2x) |
| `Tape/chain16/signedDistance/float/hostdevice` | 473 µs | 474 µs | +0.2% | [-0.2%, +0.5%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.0 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 27.1x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 855 µs | 854 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 811 µs | 810 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.71 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 191 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.9x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.9x faster than the host (was 1.8x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |


The probes differ between the sides (each side ran its own): Performance/MeshReconcile/main.cpp

<!-- END receipt cuda -->
