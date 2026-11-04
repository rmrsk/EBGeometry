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
base: 117b658
base-fingerprint: fd6d574e22a7504478fd46d99d3af214f5c008f3
head: 20b9964
head-fingerprint: f8fd6afee855ba75cecf95146cd72404a65d1cf7
cases: 190
rounds: 2
reps: 3
date: 2026-11-04
-->

## Host comparison: yellow

**Accepted:** Accept it: 3% host on sphere closest-outside for 9-26% on device

`20b9964` against `117b658` (origin/main), 2026-11-04.

**VERDICT: 2 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 2 minor, 0 faster, 0 differ, 0 lost, 190 cases

- base: 117b658 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 20b9964 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 331 ms | 330 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 514 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 348 ms | 348 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 563 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.9 ms | 41.3 ms | -1.2% | [-2.0%, -0.5%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.3 ms | 75.3 ms | +0.1% | [-0.5%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.3 ms | 55.0 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.5 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.8 ms | 64.8 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 55.4 ms | 54.5 ms | -1.5% | [-2.5%, -0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.3 ms | 60.5 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.4 ms | 72.4 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.7 ms | 75.0 ms | +0.4% | [-0.5%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.6 ms | 78.0 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.9 ms | 66.8 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 100 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 84.0 ms | 83.7 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 109 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.3 ms | 19.5 ms | +1.1% | [+0.2%, +1.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.0 ms | 45.1 ms | +0.1% | [-2.0%, +2.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.0 ms | 73.0 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.3 ms | 20.5 ms | +0.6% | [-0.3%, +1.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.9 ms | 45.9 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.3 ms | 80.7 ms | -0.8% | [-1.8%, +0.3%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.45 ms | 4.44 ms | -0.2% | [-1.2%, +0.7%] | - | ~ | tape/fused 2.90x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.4% | [+0.0%, +0.8%] | - | ~ | tape/fused 2.03x |
| `Fused/chain16/signedDistance/float/host` | 963 µs | 962 µs | -0.2% | [-1.6%, +1.2%] | - | ~ | tape/fused 16.86x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.0 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.2 ms | 46.2 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.53 ms | 4.58 ms | +1.2% | [+0.4%, +1.9%] | - | ~ | tape/fused 2.86x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.9 ms | +0.9% | [-0.1%, +1.8%] | - | ~ | tape/fused 2.06x |
| `Fused/chain16/signedDistance/double/host` | 3.87 ms | 3.92 ms | +1.4% | [+0.9%, +1.8%] | - | ~ | tape/fused 4.15x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.9 ms | 52.4 ms | -1.0% | [-2.0%, +0.0%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.7 ms | 52.9 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.3 ms | 82.6 ms | +0.4% | [+0.0%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 228 ms | 228 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 114 ms | 114 ms | -0.5% | [-0.6%, -0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 262 ms | +0.5% | [-0.5%, +1.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.7 ms | 71.0 ms | +0.4% | [-0.3%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 213 ms | 214 ms | +0.4% | [-0.3%, +1.1%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.9 ms | 98.1 ms | +1.2% | [+0.0%, +2.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.0 ms | 31.0 ms | +3.1% | [-3.0%, +9.4%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 760 ms | 762 ms | +0.3% | [-1.1%, +1.7%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 119 ms | 119 ms | -0.0% | [-1.5%, +1.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.3 ms | 43.6 ms | +0.8% | [-0.2%, +1.7%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 857 ms | 856 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.8 ms | 26.7 ms | -0.3% | [-1.5%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.2 ms | 26.7 ms | -1.8% | [-2.2%, -1.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 320 ms | 320 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.3 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ | tape/fused 3.54x |
| `MeshCSG/smooth-fused/frame/float/host` | 100 ms | 101 ms | +0.4% | [-0.1%, +1.0%] | +0.0% | ~ | tape/fused 2.37x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.9 ms | 29.9 ms | +0.3% | [-1.1%, +1.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.3 ms | 30.1 ms | -0.5% | [-1.4%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 356 ms | 360 ms | +0.9% | [-0.1%, +1.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 259 ms | 255 ms | -1.7% | [-3.7%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 98.7 ms | 98.2 ms | -0.5% | [-3.1%, +2.2%] | +0.0% | ~ | tape/fused 3.66x |
| `MeshCSG/smooth-fused/frame/double/host` | 108 ms | 110 ms | +1.9% | [-0.2%, +4.1%] | +0.0% | ~ | tape/fused 2.31x (was 2.39x) |
| `MeshReconcile/armadillo/reconcile/float/host` | 132 ms | 131 ms | -0.3% | [-1.2%, +0.5%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.9 ms | 41.3 ms | -1.4% | [-2.7%, -0.1%] | +0.0% | ~ | reconcile/scheduled 3.18x |
| `MeshReconcile/armadillo/reconcile/double/host` | 155 ms | 156 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.6 ms | 56.8 ms | +0.2% | [-0.2%, +0.7%] | +0.0% | ~ | reconcile/scheduled 2.74x |
| `PointCloudBVH/uniform/build/float/host` | 53.7 ms | 53.4 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 143 ms | +0.6% | [+0.5%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 353 ms | 354 ms | +0.4% | [+0.3%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 77.8 ms | 79.5 ms | +2.1% | [+1.9%, +2.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.64 ms | 6.67 ms | +0.5% | [+0.1%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 56.6 ms | 56.4 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 153 ms | 156 ms | +2.0% | [+1.5%, +2.6%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 409 ms | 386 ms | -5.6% | [-11.9%, +1.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.8 ms | 86.6 ms | -1.3% | [-2.2%, -0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.97 ms | 6.86 ms | -1.6% | [-2.8%, -0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.5 ms | 13.6 ms | +0.8% | [+0.3%, +1.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | -0.3% | [-0.9%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 476 ms | 474 ms | -0.5% | [-0.6%, -0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 141 ms | 141 ms | -0.2% | [-0.4%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 801 ms | 802 ms | +0.1% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 14.8 ms | -0.8% | [-1.7%, +0.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 121 ms | 120 ms | -0.8% | [-1.2%, -0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 494 ms | 495 ms | +0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 144 ms | +0.2% | [-0.4%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 789 ms | 782 ms | -0.9% | [-1.2%, -0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.4 ms | 54.5 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 128 ms | 128 ms | -0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 262 ms | 263 ms | +0.6% | [-0.0%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 144 ms | 147 ms | +2.4% | [+2.3%, +2.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 129 ms | 133 ms | +3.1% | [+3.0%, +3.2%] | - | minor |  |
| `PointCloudBVH/sphere/build/double/host` | 57.9 ms | 58.0 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 137 ms | 140 ms | +2.1% | [+1.5%, +2.7%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 290 ms | 290 ms | +0.3% | [+0.2%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 162 ms | 162 ms | -0.0% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 144 ms | 149 ms | +3.1% | [+2.7%, +3.5%] | - | minor |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.7 ms | 11.7 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 114 ms | 115 ms | +0.1% | [-0.8%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 317 ms | 316 ms | -0.1% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 156 ms | 153 ms | -1.4% | [-2.2%, -0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 321 ms | 320 ms | -0.1% | [-0.2%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.5 ms | 13.4 ms | -0.4% | [-1.6%, +0.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 116 ms | 117 ms | +1.3% | [+0.4%, +2.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 350 ms | 350 ms | +0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 166 ms | 165 ms | -0.7% | [-1.7%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 308 ms | 310 ms | +0.7% | [+0.4%, +0.9%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.2 ms | 35.1 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.6 ms | 38.9 ms | +0.9% | [+0.3%, +1.5%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.27 ms | 8.25 ms | -0.2% | [-1.0%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.33 ms | 9.42 ms | +1.0% | [+0.5%, +1.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.4 ms | 46.5 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.6 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.6 ms | 40.5 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 79.5 ms | 78.9 ms | -0.8% | [-1.6%, -0.1%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.7 ms | 65.6 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.84 ms | 5.99 ms | +2.6% | [-0.2%, +5.5%] | - | ~ | tape/fused 1.33x |
| `Ray/csg-tape/firstHit/float/host` | 7.97 ms | 7.97 ms | -0.0% | [-1.1%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.8 ms | 53.2 ms | +0.7% | [+0.5%, +0.9%] | +0.0% | ~ | hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.0 ms | 51.2 ms | +0.5% | [+0.3%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.0 ms | 45.1 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.2 ms | 88.0 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 74.3 ms | 73.6 ms | -0.9% | [-1.1%, -0.7%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.99 ms | 5.96 ms | -0.4% | [-1.5%, +0.8%] | - | ~ | tape/fused 1.39x |
| `Ray/csg-tape/firstHit/double/host` | 8.32 ms | 8.30 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.3 ms | 18.5 ms | +1.4% | [-1.0%, +3.8%] | - | ~ | marched/traced 6.43x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 119 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.9 ms | 21.8 ms | -0.3% | [-1.1%, +0.6%] | +0.0% | ~ | marched/traced 7.06x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 154 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.3 ms | 35.3 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.9 ms | 19.9 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | marched/traced 6.34x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 126 ms | +0.1% | [-0.2%, +0.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.6 ms | 23.7 ms | +0.3% | [-1.5%, +2.2%] | +0.0% | ~ | marched/traced 6.83x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 160 ms | 162 ms | +1.1% | [+0.4%, +1.9%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.9 ms | 38.2 ms | +0.8% | [+0.4%, +1.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.5 ms | 46.4 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.3 ms | 46.6 ms | +0.7% | [+0.6%, +0.8%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.2 ms | 10.2 ms | +0.4% | [-0.3%, +1.2%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.75 ms | 8.73 ms | -0.3% | [-1.4%, +0.8%] | +0.0% | ~ | hit/crossing 1.17x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 181 ms | 181 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 161 ms | 161 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.9 ms | 68.9 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.6 ms | 56.3 ms | -0.4% | [-0.4%, -0.3%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.3 ms | 53.2 ms | +1.7% | [+1.4%, +2.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.4 ms | 52.8 ms | +0.7% | [+0.0%, +1.4%] | +0.0% | ~ | hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.1 ms | 11.1 ms | -0.7% | [-1.7%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.54 ms | 9.35 ms | -2.0% | [-3.2%, -0.8%] | +0.0% | ~ | hit/crossing 1.18x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 208 ms | 207 ms | -0.6% | [-0.8%, -0.4%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 184 ms | 184 ms | +0.2% | [-0.5%, +0.8%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.7 ms | 78.2 ms | +0.6% | [-0.1%, +1.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.7 ms | 64.7 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.1 ms | +0.9% | [+0.7%, +1.1%] | +0.0% | ~ | marched/traced 4.68x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.8 ms | 51.8 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.01 ms | 8.02 ms | +0.2% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.92 ms | 1.92 ms | +0.4% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.92 ms | 6.93 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.7 ms | +0.6% | [+0.3%, +1.0%] | +0.0% | ~ | marched/traced 5.18x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.0 ms | 60.7 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.84 ms | 8.66 ms | -2.1% | [-4.4%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.94 ms | 2.14 ms | +10.3% | [-2.2%, +22.9%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.24 ms | 7.16 ms | -1.1% | [-2.3%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 242 ms | +1.3% | [+0.3%, +2.2%] | +0.0% | ~ | marched/traced 1.36x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 328 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 237 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 250 ms | 251 ms | +0.4% | [+0.0%, +0.8%] | +0.0% | ~ | marched/traced 1.51x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 375 ms | 380 ms | +1.3% | [-0.8%, +3.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 249 ms | 250 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.7 ms | 23.8 ms | +0.6% | [+0.2%, +1.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 24.0 ms | 23.9 ms | -0.4% | [-2.1%, +1.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.5 ms | 18.9 ms | +2.1% | [+1.4%, +2.7%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.1 ms | 30.4 ms | +1.0% | [+0.7%, +1.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 29.9 ms | 30.1 ms | +0.6% | [-0.3%, +1.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.4 ms | 20.6 ms | +1.4% | [+0.9%, +1.9%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.54 ms | 4.56 ms | +0.4% | [+0.1%, +0.6%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.9 ms | 25.1 ms | +0.6% | [+0.1%, +1.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.3 ms | 40.2 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.75 ms | 1.73 ms | -1.1% | [-2.7%, +0.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 131 µs | 133 µs | +1.3% | [-6.9%, +10.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.8 ms | -0.2% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.66 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.2 ms | 16.2 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.66 ms | 4.68 ms | +0.5% | [-1.8%, +2.8%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.1 ms | -0.4% | [-1.3%, +0.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.62 ms | 4.57 ms | -1.1% | [-1.8%, -0.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.2 ms | 26.5 ms | +1.2% | [+0.4%, +1.9%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.7 ms | 50.2 ms | +17.7% | [-2.0%, +38.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.77 ms | 1.76 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 273 µs | 262 µs | -4.2% | [-8.7%, +0.7%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.0 ms | 29.8 ms | -0.7% | [-1.7%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.76 ms | 7.73 ms | -0.4% | [-0.7%, -0.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.2 ms | 16.2 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.75 ms | 7.75 ms | +0.0% | [-0.3%, +0.4%] | - | ~ |  |

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 117b658
base-fingerprint: fd6d574e22a7504478fd46d99d3af214f5c008f3
head: 20b9964
head-fingerprint: f8fd6afee855ba75cecf95146cd72404a65d1cf7
cases: 280
rounds: 2
reps: 3
date: 2026-11-04
-->

## Device comparison (CUDA): yellow

**Accepted:** Accept as noise

`20b9964` against `117b658` (origin/main), 2026-11-04.

**VERDICT: 6 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 6 minor, 16 faster, 0 differ, 0 lost, 280 cases

- base: 117b658 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 20b9964 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.63 ms | 1.60 ms | -2.0% | [-2.1%, -1.9%] | - | ~ | device 22.8x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.42 ms | 3.42 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 18.5x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.06 ms | 1.08 ms | +1.9% | [+0.3%, +3.7%] | - | ~ | device 18.9x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.32 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 16.4x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.31 ms | 6.16 ms | -2.5% | [-5.2%, +0.3%] | - | ~ | device 7.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.8 ms | -0.8% | [-2.6%, +1.1%] | - | ~ | device 5.5x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.2 ms | 33.4 ms | +0.7% | [+0.4%, +0.9%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.1 ms | 57.0 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.9 ms | 20.1 ms | -3.7% | [-4.4%, -3.0%] | - | faster | device 1.0x faster than the host (was 1.0x) |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.1 ms | 47.8 ms | -0.5% | [-0.6%, -0.4%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.0 ms | 60.9 ms | -0.2% | [-1.9%, +1.6%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 110 ms | +0.4% | [-2.3%, +3.0%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.24 ms | 8.36 ms | +1.5% | [-1.2%, +4.2%] | +0.0% | ~ | device 4.8x faster than the host (was 5.0x) |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.72 ms | 9.92 ms | +2.1% | [+1.2%, +2.9%] | +0.0% | ~ | device 8.2x faster than the host (was 8.7x) |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 386 µs | 387 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 15.1x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 842 µs | 841 µs | -0.2% | [-0.4%, +0.1%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.54 ms | 4.50 ms | -0.9% | [-1.4%, -0.4%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.66 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 580 µs | 581 µs | +0.2% | [-0.8%, +1.2%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.36 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.76 ms | 6.76 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.02 ms | 7.01 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 894 µs | 898 µs | +0.4% | [-0.0%, +0.7%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.46 ms | 9.48 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 320 µs | 320 µs | -0.1% | [-0.5%, +0.3%] | - | ~ | device 11.4x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 5.1x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 3.3x faster than the host (was 3.8x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.54 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 523 ms | 522 ms | -0.2% | [-0.5%, -0.0%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 48.1 µs | 47.8 µs | -0.5% | [-0.7%, -0.3%] | - | ~ | device 23.8x faster than the host; tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.8 µs | 49.9 µs | +0.2% | [-0.4%, +0.9%] | - | ~ | device 44.0x faster than the host; tape/fused 7.23x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.9 µs | 48.2 µs | +0.6% | [-0.1%, +1.4%] | - | ~ | device 9.3x faster than the host; tape/fused 9.84x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.2 ms | 17.2 ms | -0.1% | [-0.9%, +0.7%] | - | ~ | device 8.9x faster than the host (was 7.8x); tape/fused 1.83x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.4 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 275 ms | 277 ms | +0.7% | [-0.1%, +1.6%] | - | ~ | device 0.4x faster than the host; tape/fused 1.03x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 288 ms | 286 ms | -0.8% | [-2.9%, +1.4%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.27 ms | 8.17 ms | -1.2% | [-1.5%, -0.9%] | +0.0% | ~ | device 10.8x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.17 ms | 8.20 ms | +0.3% | [-0.6%, +1.2%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.19 ms | 8.17 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.25 ms | 8.18 ms | -0.9% | [-3.6%, +1.8%] | +0.0% | ~ | device 10.9x faster than the host (was 11.8x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 689 µs | 689 µs | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 5.7x faster than the host (was 4.8x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.72 ms | 1.72 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.07 ms | 1.06 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | device 3.1x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 799 µs | 801 µs | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ | device 4.2x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 836 µs | 742 µs | -11.2% | [-11.7%, -10.8%] | +0.0% | faster | device 12.0x faster than the host (was 10.6x) |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 971 µs | 841 µs | -13.4% | [-14.8%, -12.0%] | +0.0% | faster | device 10.6x faster than the host (was 9.1x); default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 833 µs | 741 µs | -11.1% | [-11.5%, -10.7%] | +0.0% | faster | device 12.0x faster than the host (was 10.6x); default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 808 µs | 729 µs | -9.7% | [-10.4%, -9.0%] | +0.0% | faster | device 12.0x faster than the host (was 10.8x); default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 158 ms | 164 ms | +3.8% | [+1.3%, +6.3%] | +0.0% | minor | device 0.5x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 161 ms | 162 ms | +0.7% | [-1.7%, +3.3%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x (was 0.98x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 161 ms | 160 ms | -0.5% | [-2.2%, +1.3%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.02x (was 0.98x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 161 ms | 164 ms | +1.8% | [+1.0%, +2.5%] | +0.0% | ~ | device 0.5x faster than the host (was 0.6x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.77 ms | 1.77 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.90x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.94 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.82x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.68 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.29 ms | 7.22 ms | -1.0% | [-1.3%, -0.6%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.36 ms | 7.27 ms | -1.3% | [-1.7%, -0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.27 ms | 7.27 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.30 ms | 7.32 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.34 ms | 8.33 ms | -0.1% | [-1.6%, +1.3%] | - | ~ | device 11.3x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.5 ms | -0.2% | [-1.9%, +1.6%] | - | ~ | device 9.3x faster than the host (was 7.1x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 169 ms | 164 ms | -2.7% | [-5.8%, +0.5%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 283 ms | 284 ms | +0.1% | [-2.3%, +2.6%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.5 ms | 24.5 ms | +0.4% | [-0.1%, +0.8%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.7 ms | 58.8 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 156 ms | 159 ms | +1.8% | [+0.6%, +3.0%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 247 ms | 244 ms | -1.3% | [-3.5%, +0.9%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.93 ms | 2.92 ms | -0.5% | [-3.5%, +2.7%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.16 ms | 5.14 ms | -0.3% | [-1.6%, +1.1%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.5 ms | 14.5 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 121 ms | 123 ms | +1.8% | [-0.1%, +3.8%] | - | ~ | device 1.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 173 ms | 174 ms | +0.5% | [-1.7%, +2.7%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 379 ms | 380 ms | +0.5% | [-1.5%, +2.5%] | - | ~ | device 2.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.5 ms | 24.9 ms | +1.5% | [+1.2%, +1.7%] | - | ~ | device 2.1x faster than the host (was 2.2x); tape/fused 4.93x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 26.9 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 2.2x faster than the host; tape/fused 6.47x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.4 ms | 71.7 ms | +0.4% | [-1.1%, +1.9%] | - | ~ | device 4.0x faster than the host; tape/fused 5.31x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.8 ms | 22.4 ms | -1.8% | [-3.7%, +0.2%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 45.1 ms | 44.8 ms | -0.5% | [-2.1%, +1.1%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 92.9 ms | 96.2 ms | +3.6% | [+1.8%, +5.4%] | - | minor | device 0.6x faster than the host (was 0.6x) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 529 ms | 557 ms | +5.3% | [+3.7%, +6.9%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 854 ms | 857 ms | +0.4% | [-0.8%, +1.7%] | - | ~ | device 0.4x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.20 s | 3.21 s | +0.3% | [-1.5%, +2.1%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 205 ms | 198 ms | -3.2% | [-8.1%, +2.1%] | - | ~ | device 0.2x faster than the host (was 0.2x); tape/fused 2.81x (was 2.58x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 303 ms | 302 ms | -0.3% | [-2.8%, +2.3%] | - | ~ | device 0.2x faster than the host; tape/fused 2.83x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 606 ms | 585 ms | -3.6% | [-5.4%, -1.9%] | - | faster | device 0.7x faster than the host (was 0.6x); tape/fused 5.50x (was 5.28x) |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | +0.0% | [+0.0%, +0.0%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.33 ms | 5.34 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | device 10.5x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 766 µs | 678 µs | -11.5% | [-12.3%, -10.7%] | - | faster | device 11.1x faster than the host (was 9.7x) |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.17 ms | 1.81 ms | -16.6% | [-16.8%, -16.5%] | - | faster | device 5.7x faster than the host (was 4.7x) |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 995 µs | -14.7% | [-14.8%, -14.5%] | - | faster | device 9.3x faster than the host (was 7.8x) |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.20 ms | 4.60 ms | -25.8% | [-25.9%, -25.7%] | - | faster | device 5.5x faster than the host (was 4.0x) |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 3.88 ms | 2.96 ms | -23.9% | [-24.0%, -23.7%] | - | faster | device 5.2x faster than the host (was 3.9x) |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.17 ms | 7.09 ms | -1.2% | [-1.6%, -0.7%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.90 ms | 7.79 ms | -1.3% | [-1.5%, -1.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.4 ms | 13.3 ms | -0.5% | [-0.5%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 12.0 ms | 11.9 ms | -0.6% | [-1.3%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.46 ms | 1.48 ms | +1.4% | [-1.2%, +4.1%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.88 ms | 3.89 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 524 µs | 523 µs | -0.1% | [-0.5%, +0.2%] | - | ~ | device 16.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.45 ms | 4.42 ms | -0.7% | [-1.0%, -0.4%] | - | ~ | device 6.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.36 ms | 4.33 ms | -0.8% | [-2.3%, +0.7%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.15 ms | 8.08 ms | -0.8% | [-2.3%, +0.7%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.7 ms | 15.8 ms | +0.6% | [-0.4%, +1.7%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.01 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.7 ms | 17.6 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.8 ms | 14.9 ms | +0.1% | [-0.4%, +0.7%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.83 ms | 1.57 ms | -14.1% | [-14.8%, -13.5%] | - | faster | device 10.5x faster than the host (was 8.9x) |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 5.53 ms | 4.56 ms | -17.5% | [-17.6%, -17.5%] | - | faster | device 5.3x faster than the host (was 4.3x) |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 918 µs | 833 µs | -9.2% | [-9.8%, -8.7%] | - | faster | device 10.2x faster than the host (was 9.2x) |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.71 ms | 2.86 ms | -23.0% | [-23.2%, -22.8%] | - | faster | device 6.9x faster than the host (was 5.3x) |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 5.50 ms | 4.13 ms | -24.8% | [-25.1%, -24.5%] | - | faster | device 5.5x faster than the host (was 4.1x) |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.7 ms | 16.5 ms | -1.3% | [-1.9%, -0.8%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.4 ms | 21.0 ms | -1.9% | [-2.3%, -1.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.97 ms | 8.94 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.6 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.4 ms | 20.3 ms | -0.5% | [-1.2%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.76 ms | 2.77 ms | +0.4% | [+0.2%, +0.5%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.72 ms | 6.72 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.2% | [-0.6%, +0.9%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.72 ms | 2.72 ms | -0.2% | [-0.8%, +0.3%] | - | ~ | device 7.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.51 ms | 6.52 ms | +0.2% | [-0.7%, +1.1%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.7 ms | 34.1 ms | +1.2% | [+0.4%, +2.0%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.2 ms | -0.3% | [-0.4%, -0.3%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.4 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.7 ms | 20.9 ms | +0.7% | [-0.2%, +1.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.3 ms | 37.8 ms | -1.3% | [-1.6%, -1.1%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.7 ms | 12.7 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.65 ms | 9.64 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.8 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.28 ms | 3.27 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.59 ms | 2.57 ms | -0.9% | [-1.0%, -0.7%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.55 ms | 4.56 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ | device 2.7x faster than the host (was 2.6x) |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.03 ms | 3.06 ms | +0.7% | [-0.1%, +1.5%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 675 µs | 674 µs | -0.0% | [-0.4%, +0.4%] | - | ~ | device 16.8x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.29 ms | +1.0% | [+0.7%, +1.3%] | - | ~ | device 14.4x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 648 µs | 647 µs | -0.1% | [-1.6%, +1.5%] | - | ~ | device 17.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.22 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 15.1x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 604 µs | 611 µs | +1.3% | [+0.1%, +2.5%] | - | ~ | device 15.9x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.11 ms | 1.12 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.47 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 613 µs | 614 µs | +0.1% | [-5.2%, +5.7%] | - | ~ | device 2.5x faster than the host; tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 695 µs | 728 µs | +4.7% | [-0.2%, +10.2%] | - | ~ | device 2.5x faster than the host (was 2.6x); tape/fused 1.15x (was 1.21x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 690 µs | 687 µs | -0.4% | [-0.7%, -0.2%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 838 µs | 839 µs | +0.1% | [-1.2%, +1.5%] | - | ~ | device 3.7x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.3 ms | -0.6% | [-1.2%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.7% | [-0.6%, +1.9%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.4 ms | 24.5 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.4 ms | -0.5% | [-1.1%, +0.1%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.04x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.3 ms | 24.5 ms | +0.8% | [+0.3%, +1.3%] | - | ~ | device 1.1x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 14.9 ms | 15.1 ms | +1.1% | [+0.1%, +2.2%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.60 ms | 2.62 ms | +0.9% | [+0.0%, +1.7%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.61 ms | 1.60 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.71 ms | 2.73 ms | +0.5% | [+0.1%, +0.8%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.57 ms | 2.56 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 2.9x faster than the host; marched/traced 3.54x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 2.7x faster than the host; marched/traced 4.61x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.03 ms | 9.06 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.18 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 3.9x faster than the host; marched/traced 16.50x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.45 ms | 5.46 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.8x faster than the host; marched/traced 13.07x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.8 ms | 36.0 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 2.0x faster than the host (was 2.2x) |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.6 ms | 71.4 ms | -0.4% | [-1.0%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.24 ms | 5.24 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.81 ms | 8.80 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.21 ms | 6.19 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 1.3x faster than the host; marched/traced 25.40x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.3% | [-0.4%, +1.1%] | - | ~ | device 0.8x faster than the host; marched/traced 24.23x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 161 ms | 157 ms | -2.4% | [-3.7%, -1.1%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 256 ms | 258 ms | +0.6% | [-0.6%, +1.9%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.86 ms | 4.84 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 1.8x faster than the host; marched/traced 23.42x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | -0.2% | [-0.3%, -0.2%] | - | ~ | device 1.3x faster than the host; marched/traced 18.34x (was 17.81x) |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 111 ms | 113 ms | +2.3% | [+0.4%, +4.2%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 218 ms | 224 ms | +2.8% | [+1.6%, +4.0%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.2 ms | 19.3 ms | +0.7% | [+0.4%, +1.0%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 674 µs | 678 µs | +0.6% | [-0.1%, +1.3%] | - | ~ | device 17.2x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 671 µs | 673 µs | +0.2% | [-0.2%, +0.6%] | - | ~ | device 17.1x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.29 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 14.9x faster than the host; hit/crossing 0.99x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 689 µs | 687 µs | -0.3% | [-0.6%, +0.1%] | - | ~ | device 4.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 835 µs | 837 µs | +0.2% | [-0.2%, +0.7%] | - | ~ | device 4.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 627 µs | 628 µs | +0.2% | [-0.0%, +0.4%] | - | ~ | device 4.3x faster than the host; hit/crossing 1.09x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 648 µs | 648 µs | +0.1% | [-0.3%, +0.4%] | - | ~ | device 4.7x faster than the host; hit/crossing 1.29x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.70 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 10.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 8.0x faster than the host (was 8.3x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.18 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 10.1x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.11 ms | 9.11 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 265 ms | 274 ms | +3.4% | [-2.7%, +9.7%] | - | ~ | device 0.6x faster than the host (was 0.7x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 213 ms | 214 ms | +0.3% | [-2.8%, +3.5%] | - | ~ | device 0.9x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 55.4 ms | 55.1 ms | -0.6% | [-2.0%, +0.9%] | - | ~ | device 2.6x faster than the host; hit/crossing 4.97x (was 4.78x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 70.6 ms | 70.5 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.2x faster than the host; hit/crossing 3.04x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.3% | [-0.0%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.61 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.72 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.29 ms | +0.4% | [-0.0%, +0.9%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.35 ms | 1.35 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 2.6x faster than the host; hit/crossing 2.02x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.6 ms | 64.6 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 122 ms | 123 ms | +0.4% | [-1.0%, +1.8%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.9 ms | 58.8 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 111 ms | 111 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 462 ms | 505 ms | +9.2% | [+7.0%, +11.5%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 712 ms | 725 ms | +1.8% | [+0.2%, +3.4%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 385 ms | 388 ms | +1.0% | [-5.7%, +8.2%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.30x (was 1.20x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 532 ms | 538 ms | +1.2% | [-1.7%, +4.2%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.35x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.57 ms | 3.62 ms | +1.6% | [+0.9%, +2.2%] | - | ~ | device 1.2x faster than the host; marched/traced 5.45x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.78 ms | 4.77 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 0.9x faster than the host; marched/traced 8.42x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.8 ms | 19.7 ms | -0.4% | [-1.2%, +0.3%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.1 ms | 40.2 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.23 ms | 1.24 ms | +1.2% | [+1.1%, +1.3%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.84 ms | -0.7% | [-1.3%, -0.1%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.7 µs | 100 µs | +4.7% | [-0.1%, +9.6%] | - | ~ | device 14.3x faster than the host (was 15.1x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 152 µs | 151 µs | -0.7% | [-1.4%, +0.1%] | - | ~ | device 10.9x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 800 µs | 800 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 936 µs | 935 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.1 ms | 36.2 ms | +6.2% | [+2.6%, +10.0%] | - | minor | device 0.1x faster than the host (was 0.1x); marched/traced 3.40x (was 3.62x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.3 ms | 36.7 ms | +0.9% | [-0.0%, +1.9%] | - | ~ | device 0.1x faster than the host; marched/traced 6.34x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 123 ms | 123 ms | -0.1% | [-2.3%, +2.2%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 233 ms | 232 ms | -0.1% | [-1.4%, +1.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.2 ms | 16.5 ms | +2.0% | [-0.7%, +4.9%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.6 ms | 21.6 ms | -0.3% | [-0.7%, +0.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.07 ms | 1.10 ms | +3.0% | [+0.7%, +5.4%] | - | minor | device 1.4x faster than the host (was 1.5x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.36 ms | 2.37 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.97 ms | 6.00 ms | +0.5% | [+0.3%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.68 ms | -0.5% | [-1.0%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 22.9 ms | 23.0 ms | +0.5% | [+0.3%, +0.6%] | - | ~ | device 2.2x faster than the host; marched/traced 4.36x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.6 ms | 28.2 ms | -1.6% | [-1.9%, -1.3%] | - | ~ | device 1.9x faster than the host; marched/traced 5.56x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 98.8 ms | 100 ms | +1.2% | [+0.3%, +2.1%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 157 ms | 157 ms | +0.1% | [-1.7%, +2.0%] | - | ~ | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.9 ms | 13.8 ms | -1.1% | [-1.7%, -0.5%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.3 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 305 ms | 307 ms | +0.5% | [-4.8%, +5.9%] | - | ~ | device 0.2x faster than the host; marched/traced 1.82x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 347 ms | 344 ms | -0.9% | [-2.4%, +0.7%] | - | ~ | device 0.2x faster than the host; marched/traced 2.87x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 565 ms | 559 ms | -1.2% | [-3.0%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.01 s | 987 ms | -2.0% | [-3.1%, -1.0%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 194 ms | 189 ms | -2.8% | [-4.0%, -1.7%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 246 ms | 241 ms | -2.0% | [-5.6%, +1.7%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.57 ms | 4.57 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.57 ms | 4.57 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.88 ms | -0.3% | [-0.7%, +0.2%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.96 ms | 7.96 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.95 ms | 7.93 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.5 ms | 23.3 ms | -0.8% | [-1.7%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 217 µs | -0.6% | [-0.7%, -0.5%] | - | ~ | device 11.3x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | +0.0% | [-0.4%, +0.4%] | - | ~ | device 24.4x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 361 µs | 361 µs | +0.0% | [-0.1%, +0.2%] | - | ~ | device 12.6x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.02 ms | 2.02 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.2 µs | 72.6 µs | +0.6% | [+0.0%, +1.2%] | - | ~ | device 7.6x faster than the host (was 6.9x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.5 µs | 47.3 µs | -0.4% | [-0.7%, -0.1%] | - | ~ | device 4.7x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 652 µs | -0.1% | [-0.2%, +0.1%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 12.0x faster than the host (was 12.8x) |
| `Tape/chain16/signedDistance/float/hostdevice` | 474 µs | 474 µs | +0.0% | [-0.5%, +0.5%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.1% | [-0.5%, +0.3%] | - | ~ | device 27.2x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 854 µs | 855 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.71 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 191 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 3.0x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 164 µs | +0.8% | [-0.2%, +1.7%] | - | ~ | device 2.0x faster than the host (was 2.1x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |

<!-- END receipt cuda -->
