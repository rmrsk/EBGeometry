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
base: 0ed0e73
base-fingerprint: c52b275ef0f62896661b5450c135a236d0a2eb0c
head: f02fdfe
head-fingerprint: 2672bc09d5a6f2f5401c396931a62fe9caac005d
cases: 224
rounds: 2
reps: 3
date: 2026-11-06
-->

## Host comparison: yellow

**Accepted:** Accepted as noise

`f02fdfe` against `0ed0e73` (origin/main), 2026-11-06.

**VERDICT: 3 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 3 minor, 1 faster, 0 differ, 0 lost, 224 cases

- base: 0ed0e73 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: f02fdfe (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 331 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 514 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 348 ms | 347 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | -0.3% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 564 ms | 562 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.9 ms | 41.6 ms | -0.6% | [-2.0%, +0.8%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.3 ms | 75.4 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.3 ms | 55.6 ms | +0.6% | [-0.6%, +1.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 54.9 ms | +7.1% | [+0.3%, +14.0%] | +0.0% | minor |  |
| `BVHUnion/spheres/build/double/host` | 64.9 ms | 64.8 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.8 ms | 55.0 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 62.4 ms | 60.6 ms | -2.9% | [-6.4%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.2 ms | 72.4 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.6 ms | 74.4 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 78.1 ms | 77.6 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.8 ms | 66.7 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 100 ms | +0.2% | [-0.7%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.5 ms | 83.1 ms | -0.5% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 109 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.7 ms | 19.6 ms | -0.7% | [-1.5%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.4 ms | 46.5 ms | +2.4% | [+0.0%, +4.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.3 ms | 73.0 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.0 ms | 20.0 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 46.0 ms | 45.9 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.2 ms | 83.7 ms | +1.8% | [+0.6%, +3.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.49 ms | 4.46 ms | -0.7% | [-1.3%, -0.0%] | - | ~ | tape/fused 2.89x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | tape/fused 2.03x |
| `Fused/chain16/signedDistance/float/host` | 962 µs | 962 µs | +0.0% | [-1.6%, +1.7%] | - | ~ | tape/fused 16.87x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.4 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.2 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.55 ms | 4.56 ms | +0.2% | [-1.0%, +1.5%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | tape/fused 2.08x |
| `Fused/chain16/signedDistance/double/host` | 3.87 ms | 3.87 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | tape/fused 4.23x |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.1 ms | 52.8 ms | -0.5% | [-1.3%, +0.4%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.5 ms | 53.1 ms | +1.1% | [-0.1%, +2.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.9 ms | 82.0 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 230 ms | 229 ms | -0.1% | [-1.2%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 263 ms | 262 ms | -0.4% | [-1.3%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.0 ms | 70.1 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 215 ms | -0.1% | [-1.0%, +0.7%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.5 ms | 96.8 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.7 ms | 29.6 ms | -0.4% | [-1.6%, +0.8%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 758 ms | 759 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 117 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 42.9 ms | 43.6 ms | +1.6% | [+0.0%, +3.1%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 861 ms | 853 ms | -0.9% | [-1.3%, -0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.7 ms | 26.6 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.8 ms | 26.5 ms | -1.1% | [-2.0%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 319 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.1 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ | tape/fused 3.54x |
| `MeshCSG/smooth-fused/frame/float/host` | 100 ms | 99.8 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.4 ms | 29.7 ms | +0.8% | [+0.0%, +1.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.0 ms | 30.3 ms | +1.0% | [+0.1%, +1.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 363 ms | 359 ms | -1.2% | [-2.7%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 259 ms | 260 ms | +0.5% | [-3.1%, +4.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 99.6 ms | 101 ms | +0.9% | [+0.7%, +1.1%] | +0.0% | ~ | tape/fused 3.57x |
| `MeshCSG/smooth-fused/frame/double/host` | 111 ms | 111 ms | +0.3% | [-0.1%, +0.8%] | +0.0% | ~ | tape/fused 2.34x |
| `MeshReconcile/armadillo/reconcile/float/host` | 131 ms | 131 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 43.0 ms | 41.5 ms | -3.6% | [-6.6%, -0.4%] | +0.0% | faster | reconcile/scheduled 3.15x (was 3.04x) |
| `MeshReconcile/armadillo/reconcile/double/host` | 155 ms | 155 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.2 ms | 56.5 ms | +0.5% | [-0.1%, +1.2%] | +0.0% | ~ | reconcile/scheduled 2.73x |
| `PointCloudBVH/uniform/build/float/host` | 53.3 ms | 53.3 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 143 ms | +0.1% | [-0.2%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 353 ms | 354 ms | +0.4% | [+0.1%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 79.6 ms | 79.3 ms | -0.3% | [-0.5%, -0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.71 ms | 6.69 ms | -0.4% | [-1.6%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 56.0 ms | 55.9 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 156 ms | 156 ms | +0.0% | [-0.6%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 385 ms | 385 ms | +0.1% | [+0.0%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.9 ms | 86.5 ms | -0.4% | [-0.5%, -0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.90 ms | 6.95 ms | +0.7% | [-0.4%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.9 ms | 13.5 ms | -2.6% | [-5.3%, +0.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | +0.2% | [-0.4%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 476 ms | 476 ms | +0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 142 ms | 143 ms | +0.5% | [+0.3%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 801 ms | 800 ms | -0.0% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 14.8 ms | -1.0% | [-1.9%, -0.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | -0.1% | [-1.0%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 493 ms | 493 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 144 ms | -0.0% | [-0.6%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 789 ms | 791 ms | +0.3% | [-0.1%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.5 ms | 54.4 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 128 ms | 129 ms | +0.7% | [-0.5%, +1.9%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 260 ms | 259 ms | -0.5% | [-0.5%, -0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 148 ms | 148 ms | -0.0% | [-0.5%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 133 ms | 134 ms | +0.3% | [+0.3%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.3 ms | 57.5 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 139 ms | 139 ms | -0.0% | [-1.3%, +1.3%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 291 ms | 291 ms | -0.2% | [-0.3%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 163 ms | 162 ms | -1.0% | [-2.3%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 147 ms | 148 ms | +0.1% | [-0.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.5 ms | 11.7 ms | +1.4% | [+0.9%, +1.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 116 ms | 115 ms | -0.8% | [-1.7%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 317 ms | 319 ms | +0.8% | [+0.4%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 156 ms | +0.7% | [+0.2%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 320 ms | 320 ms | -0.1% | [-0.5%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.4 ms | 13.4 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 116 ms | +0.9% | [+0.1%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 349 ms | 349 ms | -0.2% | [-0.6%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 166 ms | 167 ms | +0.5% | [-0.9%, +1.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 309 ms | -0.2% | [-0.2%, -0.1%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.1 ms | 35.0 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.0 ms | 38.1 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.26 ms | 8.33 ms | +0.8% | [+0.5%, +1.1%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.37 ms | 9.28 ms | -0.9% | [-2.1%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.6 ms | 46.4 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.9 ms | 45.6 ms | -0.6% | [-1.3%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.6 ms | 40.9 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.8 ms | 78.8 ms | +0.0% | [-0.7%, +0.8%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 67.2 ms | 65.8 ms | -2.1% | [-3.5%, -0.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.19 ms | 5.18 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | tape/fused 1.47x |
| `Ray/csg-tape/firstHit/float/host` | 7.63 ms | 7.63 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.2 ms | 52.3 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.6 ms | 51.4 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.0 ms | 44.8 ms | -0.5% | [-1.0%, +0.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.5 ms | 88.2 ms | +0.8% | [+0.2%, +1.3%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/double/host` | 73.8 ms | 73.8 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.47 ms | 5.50 ms | +0.6% | [+0.0%, +1.3%] | - | ~ | tape/fused 1.44x |
| `Ray/csg-tape/firstHit/double/host` | 7.92 ms | 7.93 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.2 ms | 17.3 ms | +0.7% | [+0.0%, +1.3%] | - | ~ | marched/traced 6.74x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 117 ms | 117 ms | +0.1% | [+0.1%, +0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.9 ms | 21.9 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ | marched/traced 7.02x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 154 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.4 ms | 35.3 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.6 ms | 20.1 ms | +3.0% | [-0.3%, +6.5%] | - | ~ | marched/traced 6.15x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 124 ms | 124 ms | -0.1% | [-0.2%, -0.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.1 ms | 23.1 ms | -0.1% | [-1.4%, +1.3%] | +0.0% | ~ | marched/traced 6.94x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 160 ms | 160 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.5 ms | 37.5 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.2 ms | 46.3 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.2 ms | 46.4 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.80 ms | 7.71 ms | -1.1% | [-2.1%, -0.2%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.31 ms | 6.31 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 178 ms | 178 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 158 ms | 158 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.8 ms | 68.9 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.5 ms | 56.6 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.5 ms | 52.5 ms | -0.1% | [-0.9%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.5 ms | 52.6 ms | +0.1% | [-1.0%, +1.2%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.87 ms | 7.90 ms | +0.4% | [-0.4%, +1.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.22 ms | 6.20 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ | hit/crossing 1.27x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 200 ms | 199 ms | -0.2% | [-1.2%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 178 ms | 178 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 80.2 ms | 79.1 ms | -1.3% | [-2.5%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 65.2 ms | 65.5 ms | +0.6% | [+0.4%, +0.8%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.0 ms | -0.3% | [-1.1%, +0.4%] | +0.0% | ~ | marched/traced 4.69x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.8 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.02 ms | 8.02 ms | +0.1% | [-0.9%, +1.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.94 ms | 1.93 ms | -0.4% | [-2.1%, +1.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.81 ms | 6.80 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.6 ms | 11.7 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ | marched/traced 5.16x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.3 ms | 60.2 ms | -0.2% | [-1.1%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.61 ms | 8.68 ms | +0.8% | [+0.5%, +1.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.91 ms | 1.92 ms | +0.6% | [-1.0%, +2.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.16 ms | 7.40 ms | +3.3% | [+0.2%, +6.4%] | +0.0% | minor |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 239 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 328 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 238 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 251 ms | 252 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | marched/traced 1.48x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 376 ms | 372 ms | -0.8% | [-1.9%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 250 ms | 250 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 18.9 ms | 18.7 ms | -0.9% | [-1.2%, -0.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 18.8 ms | 18.7 ms | -0.3% | [-1.7%, +1.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 18.5 ms | -0.8% | [-1.4%, -0.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 21.7 ms | 21.6 ms | -0.6% | [-2.4%, +1.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.2 ms | 21.3 ms | +0.5% | [-0.2%, +1.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.4 ms | 20.5 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 32.9 ms | 33.0 ms | +0.2% | [-0.0%, +0.3%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 4.92 ms | 4.97 ms | +1.2% | [+0.7%, +1.7%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.14 ms | 4.18 ms | +0.9% | [-0.0%, +1.8%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.3 ms | 17.2 ms | -0.8% | [-1.5%, -0.1%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.30 ms | 4.34 ms | +0.9% | [+0.9%, +1.0%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.16 ms | 4.11 ms | -1.2% | [-2.6%, +0.1%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.21 ms | 2.24 ms | +1.1% | [-0.3%, +2.5%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.43 ms | 1.44 ms | +0.2% | [+0.1%, +0.4%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.04 ms | 2.03 ms | -0.3% | [-0.8%, +0.2%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.2 ms | 27.2 ms | -0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 46.2 ms | 46.1 ms | -0.1% | [-0.3%, -0.0%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.40 ms | 5.35 ms | -0.9% | [-1.5%, -0.4%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.48 ms | 4.51 ms | +0.6% | [-0.7%, +1.9%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 22.7 ms | 22.5 ms | -1.0% | [-1.2%, -0.7%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.55 ms | 4.51 ms | -0.8% | [-1.7%, +0.1%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.28 ms | 5.29 ms | +0.3% | [-1.1%, +1.6%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.47 ms | 2.56 ms | +3.5% | [+2.0%, +5.1%] | - | minor |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.78 ms | 1.77 ms | -0.4% | [-0.8%, +0.0%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.04 ms | 2.03 ms | -0.1% | [-1.3%, +1.0%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 29.8 ms | 30.5 ms | +2.3% | [+0.3%, +4.3%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.7 ms | 56.9 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.7 ms | 36.9 ms | +0.6% | [+0.0%, +1.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.3 ms | 92.2 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.2 ms | 64.4 ms | +0.4% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.8 ms | 42.9 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 107 ms | 107 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 75.1 ms | 75.0 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 296 ms | 297 ms | +0.5% | [+0.1%, +0.8%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 298 ms | 298 ms | -0.1% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 86.5 ms | 86.4 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 343 ms | 345 ms | +0.5% | [+0.4%, +0.7%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 348 ms | 346 ms | -0.6% | [-1.2%, +0.1%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 166 ms | 166 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 191 ms | 191 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.60 ms | 4.58 ms | -0.5% | [-1.1%, +0.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.7 ms | 24.9 ms | +0.9% | [+0.2%, +1.5%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.4 ms | 40.3 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.73 ms | 1.73 ms | +0.5% | [-1.5%, +2.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 138 µs | 139 µs | +0.6% | [-0.1%, +1.2%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.0 ms | 29.8 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.65 ms | 5.66 ms | +0.2% | [-0.4%, +0.7%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.2 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.64 ms | 4.62 ms | -0.5% | [-0.7%, -0.3%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.1 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.60 ms | 4.64 ms | +0.8% | [-0.6%, +2.1%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.4 ms | 26.4 ms | +0.1% | [-1.9%, +2.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.1 ms | 41.9 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.75 ms | 1.76 ms | +0.6% | [-1.6%, +2.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 275 µs | 285 µs | +3.4% | [-3.0%, +10.5%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.7 ms | 29.8 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.74 ms | 7.73 ms | -0.0% | [-0.3%, +0.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.3 ms | 16.4 ms | +0.8% | [-0.3%, +2.0%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.72 ms | 7.75 ms | +0.4% | [-0.1%, +0.9%] | - | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 440.2 s → 444.7 s (+1.0%), the largest peak 1265 MB → 1265 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 8.8 s | 8.9 s | +1.0% | 557 MB | 557 MB | -0.1% | ~ |
| `BVHRefit` | 3.8 s | 3.8 s | -0.0% | 454 MB | 453 MB | -0.1% | ~ |
| `BVHUnion` | 14.6 s | 14.9 s | +1.7% | 815 MB | 815 MB | +0.0% | ~ |
| `ClosestPrimitive` | 25.4 s | 25.8 s | +1.6% | 1166 MB | 1166 MB | -0.0% | ~ |
| `Fused` | 20.4 s | 21.1 s | +3.5% | 988 MB | 964 MB | -2.4% | ~ |
| `LaunchTuning` | 32.8 s | 33.9 s | +3.5% | 1107 MB | 1106 MB | -0.1% | ~ |
| `Mesh` | 14.0 s | 14.2 s | +1.6% | 775 MB | 775 MB | -0.1% | ~ |
| `MeshCSG` | 34.1 s | 34.5 s | +1.1% | 1153 MB | 1153 MB | -0.0% | ~ |
| `MeshReconcile` | 6.1 s | 6.2 s | +2.5% | 532 MB | 532 MB | -0.1% | ~ |
| `PointCloud` | 6.9 s | 7.2 s | +4.3% | 557 MB | 558 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.4 s | 3.6 s | +3.9% | 406 MB | 406 MB | -0.1% | ~ |
| `PointCloudHashGridBuild` | 2.9 s | 3.1 s | +4.4% | 403 MB | 402 MB | -0.1% | ~ |
| `Ray` | 32.3 s | 32.0 s | -0.9% | 1146 MB | 1147 MB | +0.0% | ~ |
| `RayCSG` | 29.3 s | 29.3 s | -0.1% | 962 MB | 961 MB | -0.1% | ~ |
| `RayCrossing` | 41.0 s | 41.1 s | +0.1% | 1265 MB | 1265 MB | +0.0% | ~ |
| `RayParticles` | 43.6 s | 43.8 s | +0.5% | 1260 MB | 1262 MB | +0.1% | ~ |
| `RayShell` | 33.2 s | 33.0 s | -0.6% | 1056 MB | 1056 MB | +0.0% | ~ |
| `Render` | 31.3 s | 31.6 s | +1.0% | 1054 MB | 1053 MB | -0.0% | ~ |
| `Shapes` | 9.0 s | 9.1 s | +1.4% | 552 MB | 552 MB | -0.0% | ~ |
| `SphereCloudBVH` | 4.5 s | 4.6 s | +1.8% | 476 MB | 476 MB | -0.0% | ~ |
| `SphereCloudRays` | 24.5 s | 24.8 s | +1.1% | 962 MB | 963 MB | +0.1% | ~ |
| `SphereContacts` | 4.2 s | 4.2 s | -0.2% | 424 MB | 424 MB | -0.0% | ~ |
| `Tape` | 13.9 s | 13.9 s | -0.2% | 826 MB | 825 MB | -0.1% | ~ |
| **all, in both** | 440.2 s | 444.7 s | +1.0% | 1265 MB | 1265 MB | +0.0% | ~ |

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 0ed0e73
base-fingerprint: c52b275ef0f62896661b5450c135a236d0a2eb0c
head: f02fdfe
head-fingerprint: 2672bc09d5a6f2f5401c396931a62fe9caac005d
cases: 322
rounds: 2
reps: 3
date: 2026-11-06
-->

## Device comparison (CUDA): yellow

**Accepted:** Accepted as noise

`f02fdfe` against `0ed0e73` (origin/main), 2026-11-06.

**VERDICT: 4 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 4 minor, 1 faster, 0 differ, 0 lost, 322 cases

- base: 0ed0e73 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: f02fdfe (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.60 ms | 1.60 ms | +0.1% | [-1.0%, +1.2%] | - | ~ | device 22.7x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.41 ms | 3.39 ms | -0.7% | [-1.0%, -0.4%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.06 ms | 1.05 ms | -0.7% | [-2.1%, +0.7%] | - | ~ | device 19.4x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.30 ms | 2.31 ms | +0.4% | [-0.1%, +0.8%] | - | ~ | device 16.5x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.14 ms | 6.19 ms | +0.9% | [-0.7%, +2.5%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.7 ms | 11.9 ms | +2.2% | [+1.0%, +3.4%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.4 ms | 33.3 ms | -0.5% | [-0.7%, -0.3%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 56.9 ms | 57.0 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.8 ms | 20.7 ms | -0.9% | [-2.4%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.1 ms | 47.9 ms | -0.4% | [-0.9%, +0.2%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.1 ms | 61.9 ms | +1.3% | [-0.0%, +2.7%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 108 ms | 111 ms | +2.3% | [+1.8%, +2.7%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.29 ms | 8.30 ms | +0.1% | [-2.1%, +2.3%] | +0.0% | ~ | device 4.8x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.88 ms | 9.98 ms | +1.0% | [-0.7%, +2.7%] | +0.0% | ~ | device 8.1x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 384 µs | 385 µs | +0.4% | [-0.2%, +0.9%] | - | ~ | device 15.3x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 840 µs | 839 µs | -0.1% | [-0.2%, -0.1%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.54 ms | 4.52 ms | -0.5% | [-1.0%, +0.0%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.66 ms | -0.2% | [-0.8%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 581 µs | 581 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 13.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.37 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.77 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.01 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 900 µs | 896 µs | -0.5% | [-0.6%, -0.3%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.47 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 313 µs | 312 µs | -0.1% | [-0.9%, +0.7%] | - | ~ | device 11.8x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.79 ms | 1.79 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 4.9x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.0 ms | 58.9 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.60 ms | 5.61 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 522 ms | 515 ms | -1.3% | [-2.4%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.2% | [-0.7%, +0.2%] | - | ~ | device 23.9x faster than the host; tape/fused 4.56x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.9 µs | 49.8 µs | -0.2% | [-0.4%, -0.0%] | - | ~ | device 44.3x faster than the host; tape/fused 7.20x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 8.9x faster than the host (was 9.5x); tape/fused 9.94x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.2 ms | -0.5% | [-0.9%, -0.0%] | - | ~ | device 6.8x faster than the host (was 8.9x); tape/fused 1.82x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.4 ms | 31.4 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 284 ms | 284 ms | +0.3% | [-2.2%, +2.8%] | - | ~ | device 0.4x faster than the host; tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 285 ms | 282 ms | -1.0% | [-2.3%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.24 ms | 8.23 ms | -0.2% | [-1.1%, +0.8%] | +0.0% | ~ | device 12.4x faster than the host (was 10.7x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.27 ms | 8.35 ms | +1.0% | [-0.0%, +2.0%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.25 ms | 8.25 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.29 ms | 8.25 ms | -0.6% | [-0.8%, -0.3%] | +0.0% | ~ | device 11.7x faster than the host (was 12.7x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 675 µs | 676 µs | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | device 3.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 585 µs | 585 µs | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ | device 4.4x faster than the host; default/this 1.16x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.04 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 786 µs | 787 µs | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 738 µs | 741 µs | +0.4% | [+0.0%, +0.9%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 826 µs | 827 µs | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 742 µs | 736 µs | -0.7% | [-1.4%, +0.0%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 727 µs | 724 µs | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 158 ms | 163 ms | +3.2% | [+1.6%, +4.8%] | +0.0% | minor | device 0.7x faster than the host (was 0.6x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 160 ms | 162 ms | +0.9% | [-1.5%, +3.3%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 163 ms | 161 ms | -1.3% | [-2.6%, +0.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x (was 0.97x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 165 ms | 166 ms | +1.0% | [-0.7%, +2.6%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.98x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ | device 2.0x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.76 ms | 2.76 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.96 ms | 1.95 ms | -0.6% | [-1.0%, -0.3%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.71x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.4%, +0.5%] | +0.0% | ~ | device 1.8x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.24 ms | 7.20 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.31 ms | 7.29 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.23 ms | 7.24 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.29 ms | 7.30 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.31 ms | 8.44 ms | +1.6% | [+1.1%, +2.1%] | - | ~ | device 11.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.8 ms | 16.6 ms | -0.9% | [-2.0%, +0.3%] | - | ~ | device 7.1x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 168 ms | 168 ms | -0.0% | [-2.0%, +1.9%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 288 ms | 290 ms | +0.5% | [-0.8%, +1.7%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.7 ms | 24.4 ms | -1.2% | [-1.8%, -0.5%] | - | ~ | device 5.0x faster than the host (was 4.0x) |
| `MeshSDF/query-random/float/hostdevice` | 58.7 ms | 58.8 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 160 ms | 157 ms | -1.4% | [-3.8%, +1.2%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 239 ms | 245 ms | +2.4% | [+1.1%, +3.7%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.55 ms | 2.53 ms | -0.7% | [-1.9%, +0.5%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.32 ms | 4.33 ms | +0.2% | [-0.4%, +0.7%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.7 ms | 13.7 ms | +0.4% | [-0.3%, +1.1%] | - | ~ | device 4.3x faster than the host (was 3.9x) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 64.4 ms | 65.0 ms | +0.9% | [-0.2%, +2.1%] | - | ~ | device 3.1x faster than the host (was 3.5x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 94.8 ms | 96.3 ms | +1.6% | [+0.4%, +2.8%] | - | ~ | device 2.2x faster than the host (was 2.5x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 250 ms | 250 ms | +0.1% | [-1.2%, +1.4%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.1 ms | 24.9 ms | -0.9% | [-3.2%, +1.5%] | - | ~ | device 2.0x faster than the host; tape/fused 2.61x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.8 ms | 26.9 ms | +0.5% | [-0.2%, +1.3%] | - | ~ | device 2.2x faster than the host; tape/fused 3.58x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.0 ms | 71.5 ms | +0.7% | [-0.6%, +2.0%] | - | ~ | device 4.1x faster than the host (was 4.3x); tape/fused 3.50x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.3 ms | 22.9 ms | +3.0% | [-3.5%, +9.7%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.2 ms | 44.6 ms | +1.1% | [-1.4%, +3.6%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 92.8 ms | 92.1 ms | -0.8% | [-1.7%, +0.2%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 521 ms | 563 ms | +8.2% | [+5.3%, +11.2%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 853 ms | 858 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.11 s | 2.05 s | -2.8% | [-6.3%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 189 ms | 203 ms | +7.3% | [+3.8%, +10.8%] | - | minor | device 0.2x faster than the host (was 0.3x); tape/fused 2.78x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 312 ms | 309 ms | -1.1% | [-3.6%, +1.5%] | - | ~ | device 0.2x faster than the host; tape/fused 2.78x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 597 ms | 594 ms | -0.5% | [-2.6%, +1.6%] | - | ~ | device 0.5x faster than the host; tape/fused 3.46x |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.31 ms | 5.31 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 10.4x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 680 µs | 680 µs | +0.1% | [-0.4%, +0.5%] | - | ~ | device 11.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.82 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 997 µs | 995 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 9.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.60 ms | 4.59 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.96 ms | 2.96 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.10 ms | 7.11 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.80 ms | 7.79 ms | -0.1% | [-0.7%, +0.6%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.47 ms | 1.48 ms | +0.3% | [-1.8%, +2.4%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.88 ms | 3.87 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 523 µs | 526 µs | +0.7% | [+0.6%, +0.8%] | - | ~ | device 15.7x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.46 ms | 4.53 ms | +1.5% | [+0.4%, +2.7%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.30 ms | 4.35 ms | +1.0% | [+0.9%, +1.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 7.96 ms | 8.07 ms | +1.4% | [-0.0%, +2.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.7 ms | 15.8 ms | +0.8% | [-0.5%, +2.2%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.01 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.7 ms | 17.7 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.8 ms | 15.0 ms | +1.7% | [-1.0%, +4.6%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.58 ms | -0.3% | [-1.0%, +0.5%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.59 ms | 4.60 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 831 µs | 835 µs | +0.5% | [-0.2%, +1.1%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.86 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.13 ms | 4.14 ms | +0.2% | [-0.9%, +1.3%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.6% | [-1.9%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 21.1 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.92 ms | 8.95 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.5 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.1 ms | 20.2 ms | +0.5% | [-0.1%, +1.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.75 ms | 2.76 ms | +0.4% | [-0.7%, +1.5%] | - | ~ | device 4.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.72 ms | 6.74 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.72 ms | 2.73 ms | +0.4% | [-0.3%, +1.1%] | - | ~ | device 7.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.42 ms | 6.42 ms | -0.1% | [-1.0%, +0.9%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 34.0 ms | 33.9 ms | -0.2% | [-1.7%, +1.3%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.6 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.3 ms | -0.6% | [-1.6%, +0.4%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.7 ms | 20.8 ms | +0.4% | [-0.0%, +0.9%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.0 ms | 38.0 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.7 ms | 12.8 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.67 ms | 9.66 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.9 ms | 12.8 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.27 ms | 3.30 ms | +0.8% | [+0.4%, +1.1%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.58 ms | 2.58 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.56 ms | 4.55 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.03 ms | 3.05 ms | +0.9% | [+0.6%, +1.2%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 675 µs | 673 µs | -0.4% | [-1.4%, +0.7%] | - | ~ | device 16.8x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 14.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 648 µs | 647 µs | -0.2% | [-1.9%, +1.5%] | - | ~ | device 17.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.22 ms | -0.5% | [-1.3%, +0.4%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 610 µs | 615 µs | +0.7% | [-0.6%, +2.1%] | - | ~ | device 15.8x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 14.8x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.08 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | +0.3% | [-0.1%, +0.8%] | - | ~ | device 9.7x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 608 µs | 609 µs | +0.2% | [-5.2%, +5.8%] | - | ~ | device 2.4x faster than the host; tape/fused 1.10x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 663 µs | 724 µs | +9.2% | [+8.7%, +9.7%] | - | minor | device 2.5x faster than the host (was 2.7x); tape/fused 1.17x (was 1.28x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 675 µs | 671 µs | -0.6% | [-1.0%, -0.2%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 846 µs | 844 µs | -0.2% | [-1.0%, +0.6%] | - | ~ | device 3.5x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.1 ms | 17.1 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.6% | [-0.0%, +1.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.9% | [+0.5%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.4 ms | -0.7% | [-1.7%, +0.2%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.04x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.5 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.2 ms | 15.1 ms | -0.9% | [-1.2%, -0.5%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.37 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 2.0x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.49 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 2.6x faster than the host; marched/traced 3.56x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.00 ms | 3.00 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.3x faster than the host; marched/traced 4.63x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 8.99 ms | 9.03 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 4.9x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.9 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.21 ms | 2.21 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.8x faster than the host; marched/traced 15.82x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.47 ms | 5.47 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 2.8x faster than the host; marched/traced 12.73x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.0 ms | 35.0 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 69.6 ms | 69.6 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.22 ms | 5.21 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.70 ms | 8.72 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.17 ms | 6.15 ms | -0.3% | [-1.1%, +0.4%] | - | ~ | device 1.2x faster than the host; marched/traced 25.47x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.5 ms | 10.6 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 0.8x faster than the host; marched/traced 24.21x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 161 ms | 157 ms | -2.9% | [-3.3%, -2.4%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 255 ms | 256 ms | +0.7% | [-0.2%, +1.6%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.86 ms | 4.85 ms | -0.3% | [-0.7%, +0.2%] | - | ~ | device 1.8x faster than the host; marched/traced 23.50x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.2 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 1.3x faster than the host; marched/traced 17.82x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 114 ms | 114 ms | +0.1% | [-0.9%, +1.0%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 219 ms | 218 ms | -0.5% | [-4.0%, +3.0%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.3 ms | -0.5% | [-1.6%, +0.7%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.5% | [-0.6%, -0.4%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 683 µs | 677 µs | -0.8% | [-1.5%, -0.2%] | - | ~ | device 17.0x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 673 µs | 679 µs | +0.9% | [-0.7%, +2.6%] | - | ~ | device 16.9x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 679 µs | 681 µs | +0.4% | [-0.1%, +0.8%] | - | ~ | device 3.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 842 µs | 846 µs | +0.5% | [+0.3%, +0.6%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 609 µs | 610 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.3x faster than the host; hit/crossing 1.12x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 631 µs | 633 µs | +0.3% | [-0.1%, +0.7%] | - | ~ | device 3.8x faster than the host; hit/crossing 1.34x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.68 ms | 4.69 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.17 ms | 4.17 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 10.0x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.07 ms | 9.08 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 57.6 ms | 57.5 ms | -0.1% | [-1.4%, +1.2%] | - | ~ | device 3.4x faster than the host (was 3.0x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 84.4 ms | 82.9 ms | -1.8% | [-3.2%, -0.3%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.5 ms | 48.7 ms | +0.4% | [-0.9%, +1.7%] | - | ~ | device 3.6x faster than the host (was 3.0x); hit/crossing 1.18x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.4 ms | 63.6 ms | +0.3% | [-0.7%, +1.3%] | - | ~ | device 2.4x faster than the host; hit/crossing 1.30x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 16.9 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.3 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 2.0x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.50 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.11 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.26x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.17 ms | +1.1% | [-0.1%, +2.3%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.13x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.5 ms | 64.6 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 121 ms | 122 ms | +1.0% | [+0.1%, +1.8%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.3 ms | 58.7 ms | +0.6% | [+0.3%, +1.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 111 ms | 111 ms | +0.4% | [-0.7%, +1.5%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 468 ms | 479 ms | +2.5% | [-3.1%, +8.1%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 709 ms | 698 ms | -1.6% | [-3.2%, +0.0%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 372 ms | 394 ms | +5.8% | [-0.4%, +12.3%] | - | ~ | device 0.4x faster than the host (was 0.4x); hit/crossing 1.22x (was 1.26x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 529 ms | 531 ms | +0.5% | [-2.0%, +3.1%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.31x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.50 ms | 3.55 ms | +1.4% | [-3.2%, +6.2%] | - | ~ | device 1.2x faster than the host; marched/traced 3.41x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.75 ms | 4.77 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 0.9x faster than the host; marched/traced 5.58x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.1 ms | +0.7% | [+0.0%, +1.4%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.6 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.23 ms | 1.23 ms | +0.5% | [-1.4%, +2.4%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.84 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.9 µs | 94.4 µs | -1.5% | [-3.3%, +0.2%] | - | ~ | device 15.6x faster than the host (was 14.8x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 150 µs | 152 µs | +1.1% | [-0.0%, +2.1%] | - | ~ | device 10.8x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 796 µs | 795 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 928 µs | 927 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.7 ms | 34.2 ms | -4.4% | [-5.9%, -2.9%] | - | faster | device 0.1x faster than the host (was 0.1x); marched/traced 3.57x (was 3.42x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.8 ms | 36.5 ms | -0.9% | [-2.6%, +0.9%] | - | ~ | device 0.1x faster than the host; marched/traced 6.50x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 122 ms | 122 ms | -0.2% | [-3.5%, +3.2%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 236 ms | 237 ms | +0.8% | [-0.6%, +2.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.6 ms | 16.4 ms | -0.8% | [-2.9%, +1.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 22.0 ms | 21.7 ms | -1.2% | [-2.3%, -0.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.06 ms | 1.07 ms | +0.9% | [-0.3%, +2.2%] | - | ~ | device 1.4x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.34 ms | 2.36 ms | +1.0% | [+0.1%, +2.0%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.99 ms | 6.00 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.71 ms | 6.70 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.0 ms | 23.0 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 2.2x faster than the host; marched/traced 2.71x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.2 ms | 28.0 ms | -0.7% | [-1.5%, +0.1%] | - | ~ | device 2.0x faster than the host; marched/traced 3.74x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.3 ms | 62.3 ms | -0.1% | [-0.9%, +0.8%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 105 ms | 105 ms | +0.0% | [-0.8%, +0.8%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.7 ms | 13.8 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.4 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 310 ms | 303 ms | -2.3% | [-5.8%, +1.4%] | - | ~ | device 0.2x faster than the host; marched/traced 1.87x (was 1.78x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 348 ms | 354 ms | +1.7% | [-0.8%, +4.3%] | - | ~ | device 0.2x faster than the host; marched/traced 2.81x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 552 ms | 567 ms | +2.7% | [+2.2%, +3.1%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 985 ms | 995 ms | +1.0% | [+0.3%, +1.7%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 191 ms | 191 ms | -0.4% | [-4.7%, +4.2%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 241 ms | 236 ms | -2.4% | [-3.3%, -1.4%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.14 ms | 4.14 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.15 ms | 4.15 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.90 ms | 1.89 ms | -0.5% | [-0.6%, -0.4%] | - | ~ | device 8.8x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.58 ms | 7.59 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 1.6x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.59 ms | 7.58 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.6x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.4 ms | 23.3 ms | -0.3% | [-1.2%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 155 µs | 155 µs | +0.1% | [-0.0%, +0.1%] | - | ~ | device 33.2x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.5 µs | 47.5 µs | -0.1% | [-0.2%, +0.1%] | - | ~ | device 18.8x faster than the host |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.1% | [-0.7%, +0.5%] | - | ~ | device 15.8x faster than the host |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 48.1 µs | 47.9 µs | -0.3% | [-0.6%, -0.1%] | - | ~ | device 65.1x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 47.6 µs | 47.7 µs | +0.3% | [-0.2%, +0.8%] | - | ~ | device 15.9x faster than the host |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.2% | [-0.4%, +0.7%] | - | ~ | device 17.4x faster than the host |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 47.8 µs | 48.0 µs | +0.6% | [+0.3%, +0.8%] | - | ~ | device 9.7x faster than the host (was 10.1x) |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.6 µs | 47.7 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 7.4x faster than the host |
| `Shapes/wedge/signedDistance/float/hostdevice` | 47.5 µs | 47.4 µs | -0.3% | [-0.8%, +0.2%] | - | ~ | device 9.0x faster than the host (was 8.6x) |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 557 µs | 558 µs | +0.3% | [-0.5%, +1.1%] | - | ~ | device 8.2x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.23 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 492 µs | 492 µs | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.0x faster than the host (was 2.0x) |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 421 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.9x faster than the host (was 2.0x) |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 358 µs | 359 µs | +0.3% | [+0.0%, +0.6%] | - | ~ | device 2.3x faster than the host |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 669 µs | 669 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host (was 1.7x) |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 413 µs | 414 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.3x faster than the host (was 1.3x) |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 340 µs | +0.2% | [-0.1%, +0.6%] | - | ~ | device 1.4x faster than the host (was 1.5x) |
| `Shapes/wedge/signedDistance/double/hostdevice` | 155 µs | 154 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 3.0x faster than the host (was 2.8x) |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.02 ms | 8.03 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 885 µs | 886 µs | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ | device 10.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.64 ms | -0.0% | [-0.9%, +0.9%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 13.9 ms | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.1 ms | 11.0 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.03 ms | +1.1% | [+1.0%, +1.3%] | +0.0% | ~ | device 7.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.75 ms | 4.77 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 5.1x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.1 ms | 24.1 ms | -0.2% | [-0.3%, +0.0%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.4 ms | 30.3 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.0 ms | 44.4 ms | +0.9% | [+0.5%, +1.3%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.6 ms | 45.7 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 122 ms | 122 ms | +0.3% | [-1.3%, +1.8%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.06x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 129 ms | 126 ms | -1.9% | [-4.5%, +0.8%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.06x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 128 ms | 129 ms | +0.8% | [-3.4%, +5.2%] | +0.0% | ~ | device 0.7x faster than the host (was 0.7x) |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 134 ms | 134 ms | -0.2% | [-4.2%, +4.1%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 10.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.2 ms | +0.1% | [+0.0%, +0.1%] | +0.0% | ~ | device 0.6x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 218 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 11.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 23.3x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 358 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.00 ms | 2.00 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.1 µs | 72.1 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 7.0x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.3 µs | 47.4 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 4.7x faster than the host (was 4.8x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 652 µs | +0.1% | [+0.1%, +0.2%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 11.7x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 475 µs | 476 µs | +0.1% | [+0.1%, +0.2%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 26.4x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 857 µs | 858 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.75 ms | 6.76 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 191 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 3.0x faster than the host (was 2.8x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.2% | [-0.0%, +0.3%] | - | ~ | device 1.8x faster than the host (was 2.1x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.77 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.0x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 2992.6 s → 2994.2 s (+0.1%), the largest peak 5256 MB → 5256 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.1 s | 12.1 s | -0.2% | 532 MB | 532 MB | -0.0% | ~ |
| `BVHRefit` | 6.0 s | 6.0 s | +0.8% | 461 MB | 461 MB | +0.0% | ~ |
| `BVHUnion` | 19.7 s | 19.8 s | +0.5% | 842 MB | 842 MB | +0.0% | ~ |
| `ClosestPrimitive` | 114.6 s | 115.0 s | +0.3% | 1215 MB | 1215 MB | +0.0% | ~ |
| `Fused` | 45.8 s | 45.4 s | -0.8% | 1027 MB | 1027 MB | +0.0% | ~ |
| `LaunchTuning` | 457.0 s | 460.2 s | +0.7% | 3385 MB | 3385 MB | +0.0% | ~ |
| `Mesh` | 19.0 s | 19.0 s | +0.0% | 733 MB | 734 MB | +0.2% | ~ |
| `MeshCSG` | 331.2 s | 330.9 s | -0.1% | 4506 MB | 4506 MB | +0.0% | ~ |
| `MeshReconcile` | 8.7 s | 9.0 s | +3.2% | 461 MB | 461 MB | +0.0% | ~ |
| `PointCloud` | 11.4 s | 11.5 s | +0.8% | 500 MB | 500 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 6.1 s | 6.1 s | +0.3% | 461 MB | 461 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 5.4 s | 5.3 s | -1.7% | 461 MB | 461 MB | +0.0% | ~ |
| `Ray` | 171.5 s | 171.6 s | +0.0% | 3281 MB | 3281 MB | +0.0% | ~ |
| `RayCSG` | 207.0 s | 207.6 s | +0.3% | 3632 MB | 3632 MB | +0.0% | ~ |
| `RayCrossing` | 388.8 s | 386.9 s | -0.5% | 3577 MB | 3577 MB | -0.0% | ~ |
| `RayParticles` | 442.5 s | 440.7 s | -0.4% | 5256 MB | 5256 MB | +0.0% | ~ |
| `RayShell` | 278.8 s | 278.3 s | -0.2% | 4858 MB | 4858 MB | -0.0% | ~ |
| `Render` | 201.9 s | 202.2 s | +0.2% | 4338 MB | 4338 MB | +0.0% | ~ |
| `Shapes` | 27.6 s | 27.7 s | +0.4% | 520 MB | 520 MB | -0.0% | ~ |
| `SphereCloudBVH` | 8.3 s | 8.2 s | -0.3% | 461 MB | 461 MB | +0.0% | ~ |
| `SphereCloudRays` | 161.9 s | 162.9 s | +0.6% | 3542 MB | 3542 MB | +0.0% | ~ |
| `SphereContacts` | 7.2 s | 7.4 s | +3.4% | 461 MB | 461 MB | +0.0% | ~ |
| `Tape` | 60.2 s | 60.2 s | +0.1% | 850 MB | 850 MB | -0.0% | ~ |
| **all, in both** | 2992.6 s | 2994.2 s | +0.1% | 5256 MB | 5256 MB | +0.0% | ~ |

Device kernels: 152 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

<!-- END receipt cuda -->
