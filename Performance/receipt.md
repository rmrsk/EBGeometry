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
head: 8007d3d
head-fingerprint: ea9c2f8b0f24a1f1671e3cad3eccfd38a5152ebb
cases: 204
rounds: 2
reps: 3
date: 2026-11-05
-->

## Host comparison: yellow

**Accepted:** Accepted as noise

`8007d3d` against `03ed8f4` (origin/main), 2026-11-05.

**VERDICT: 2 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 2 minor, 1 faster, 0 differ, 0 lost, 204 cases

- base: 03ed8f4 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8007d3d (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 331 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 515 ms | 515 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 349 ms | 350 ms | +0.4% | [-0.3%, +1.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 188 ms | +0.7% | [-0.0%, +1.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 568 ms | 566 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 42.4 ms | 43.2 ms | +1.9% | [-1.4%, +5.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 77.2 ms | 75.6 ms | -2.0% | [-4.4%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.3 ms | 55.5 ms | +0.3% | [-0.5%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.5 ms | 51.4 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.1 ms | 66.0 ms | +1.2% | [-0.5%, +3.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.8 ms | 55.3 ms | +0.9% | [+0.5%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.1 ms | 61.3 ms | +0.4% | [-0.8%, +1.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.9 ms | 72.5 ms | -0.5% | [-1.4%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.7 ms | 74.7 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 78.5 ms | 78.4 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.8 ms | 66.4 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 101 ms | +0.6% | [+0.2%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.4 ms | 83.1 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | +0.2% | [-0.5%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.5 ms | 19.4 ms | -0.4% | [-1.2%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.9 ms | 45.1 ms | -1.7% | [-2.9%, -0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.4 ms | 73.9 ms | +0.7% | [-0.0%, +1.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.5 ms | 20.4 ms | -0.6% | [-1.2%, -0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.9 ms | 46.2 ms | +0.5% | [-0.7%, +1.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.7 ms | 81.5 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.46 ms | 4.48 ms | +0.4% | [-0.4%, +1.3%] | - | ~ | tape/fused 2.89x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | tape/fused 2.03x |
| `Fused/chain16/signedDistance/float/host` | 967 µs | 964 µs | -0.4% | [-1.3%, +0.5%] | - | ~ | tape/fused 16.81x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.0 ms | 46.1 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.4 ms | 46.8 ms | +0.9% | [-0.0%, +1.8%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.60 ms | 4.62 ms | +0.3% | [-0.5%, +1.1%] | - | ~ | tape/fused 2.85x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.7 ms | -0.8% | [-0.8%, -0.8%] | - | ~ | tape/fused 2.08x |
| `Fused/chain16/signedDistance/double/host` | 3.90 ms | 3.89 ms | -0.1% | [-1.6%, +1.5%] | - | ~ | tape/fused 4.18x |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.4 ms | 53.3 ms | -0.2% | [-1.6%, +1.1%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 54.5 ms | 53.8 ms | -1.4% | [-2.2%, -0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.2 ms | 82.2 ms | -0.0% | [-0.9%, +0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 230 ms | 232 ms | +1.0% | [-0.4%, +2.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 114 ms | +0.3% | [-0.6%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 265 ms | 263 ms | -0.6% | [-0.9%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.4 ms | 70.6 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 216 ms | +0.5% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 97.5 ms | 96.9 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.6 ms | 30.0 ms | +1.4% | [+0.3%, +2.4%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 763 ms | 766 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 117 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.8 ms | 44.1 ms | -1.6% | [-2.0%, -1.2%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 873 ms | 875 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.8 ms | 27.1 ms | +1.0% | [-0.2%, +2.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.7 ms | 26.9 ms | +0.7% | [-0.2%, +1.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 321 ms | 321 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 240 ms | 240 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.9 ms | 90.5 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ | tape/fused 3.55x |
| `MeshCSG/smooth-fused/frame/float/host` | 101 ms | 100.0 ms | -1.1% | [-1.8%, -0.4%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 30.3 ms | 30.0 ms | -0.8% | [-1.2%, -0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.4 ms | 30.3 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 358 ms | 361 ms | +1.0% | [+0.2%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 261 ms | 260 ms | -0.2% | [-3.9%, +3.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 98.7 ms | 98.7 ms | +0.0% | [-2.5%, +2.5%] | +0.0% | ~ | tape/fused 3.66x |
| `MeshCSG/smooth-fused/frame/double/host` | 107 ms | 109 ms | +2.0% | [-0.4%, +4.3%] | +0.0% | ~ | tape/fused 2.39x |
| `MeshReconcile/armadillo/reconcile/float/host` | 132 ms | 139 ms | +5.5% | [-0.8%, +12.1%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.6 ms | 41.5 ms | -0.2% | [-1.4%, +1.0%] | +0.0% | ~ | reconcile/scheduled 3.36x (was 3.17x) |
| `MeshReconcile/armadillo/reconcile/double/host` | 164 ms | 156 ms | -4.9% | [-10.8%, +1.7%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 57.4 ms | 57.4 ms | +0.0% | [-3.3%, +3.5%] | +0.0% | ~ | reconcile/scheduled 2.72x (was 2.86x) |
| `PointCloudBVH/uniform/build/float/host` | 53.4 ms | 53.3 ms | -0.2% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 146 ms | 147 ms | +0.2% | [-2.9%, +3.4%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 358 ms | 355 ms | -0.8% | [-2.7%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 79.8 ms | 79.8 ms | -0.0% | [-1.2%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.68 ms | 6.68 ms | -0.1% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 56.7 ms | 56.2 ms | -0.9% | [-2.9%, +1.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 157 ms | 160 ms | +1.5% | [-2.2%, +5.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 387 ms | 391 ms | +1.2% | [-0.8%, +3.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 88.0 ms | 87.2 ms | -0.9% | [-1.8%, -0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.87 ms | 6.91 ms | +0.6% | [+0.3%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.8 ms | 13.8 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | -0.1% | [-0.8%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 477 ms | 476 ms | -0.2% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 144 ms | 143 ms | -0.7% | [-2.0%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 809 ms | 809 ms | +0.0% | [-1.0%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 14.7 ms | -0.9% | [-1.2%, -0.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | -0.2% | [-0.5%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 495 ms | 495 ms | +0.1% | [-0.9%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 145 ms | 147 ms | +0.9% | [-0.5%, +2.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 802 ms | 799 ms | -0.3% | [-2.1%, +1.5%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 55.4 ms | 54.5 ms | -1.7% | [-3.6%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 129 ms | 131 ms | +1.8% | [-1.4%, +5.0%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 263 ms | 266 ms | +1.0% | [-1.7%, +3.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 148 ms | 148 ms | +0.2% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 134 ms | 133 ms | -0.7% | [-1.4%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 58.7 ms | 58.1 ms | -1.1% | [-4.2%, +2.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 140 ms | 141 ms | +0.8% | [-1.9%, +3.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 293 ms | 305 ms | +4.1% | [-1.2%, +9.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 167 ms | 163 ms | -2.1% | [-4.8%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 150 ms | 149 ms | -0.5% | [-2.1%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.5 ms | 11.6 ms | +0.7% | [-0.4%, +1.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 115 ms | 115 ms | -0.2% | [-0.4%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 319 ms | 320 ms | +0.2% | [-0.6%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 157 ms | 156 ms | -0.8% | [-2.9%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 322 ms | 321 ms | -0.4% | [-1.5%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.7 ms | 14.4 ms | +5.2% | [-3.2%, +13.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 118 ms | 117 ms | -0.8% | [-2.2%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 349 ms | 350 ms | +0.1% | [-1.2%, +1.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 165 ms | 165 ms | +0.1% | [-0.7%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 310 ms | 309 ms | -0.5% | [-1.3%, +0.3%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.4 ms | 35.2 ms | -0.4% | [-1.6%, +0.8%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.2 ms | 38.3 ms | +0.3% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.24 ms | 8.27 ms | +0.4% | [-1.2%, +2.0%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.31 ms | 9.31 ms | -0.0% | [-1.3%, +1.3%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.9 ms | 47.5 ms | +1.3% | [-2.0%, +4.6%] | +0.0% | ~ | hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.8 ms | 45.7 ms | -0.1% | [-1.0%, +0.8%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.6 ms | 40.7 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.9 ms | 79.4 ms | +0.7% | [-0.8%, +2.2%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/float/host` | 66.1 ms | 65.9 ms | -0.4% | [-1.5%, +0.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.83 ms | 5.94 ms | +1.9% | [+0.1%, +3.7%] | - | ~ | tape/fused 1.35x |
| `Ray/csg-tape/firstHit/float/host` | 7.93 ms | 8.03 ms | +1.2% | [+0.8%, +1.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 53.5 ms | 52.9 ms | -1.2% | [-2.7%, +0.2%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.3 ms | 51.3 ms | -0.0% | [-0.8%, +0.8%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.0 ms | 45.4 ms | +0.8% | [-0.8%, +2.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.6 ms | 92.2 ms | +4.0% | [-2.2%, +10.4%] | +0.0% | ~ | triangles/polygons 0.57x (was 0.60x) |
| `Ray/mesh-polygons/intersects/double/host` | 84.4 ms | 76.8 ms | -9.0% | [-22.1%, +8.0%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 6.03 ms | 6.01 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | tape/fused 1.40x |
| `Ray/csg-tape/firstHit/double/host` | 8.33 ms | 8.42 ms | +1.1% | [+0.1%, +2.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.3 ms | 18.2 ms | -0.7% | [-2.5%, +1.1%] | - | ~ | marched/traced 6.58x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 119 ms | -0.5% | [-1.4%, +0.4%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 22.1 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ | marched/traced 7.01x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 155 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.8 ms | 35.2 ms | -1.5% | [-3.1%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.8 ms | 20.0 ms | +0.9% | [+0.3%, +1.4%] | - | ~ | marched/traced 6.32x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 126 ms | +0.3% | [+0.1%, +0.5%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 23.4 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ | marched/traced 6.86x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 161 ms | 160 ms | -0.4% | [-0.6%, -0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.0 ms | 38.0 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 47.3 ms | 51.8 ms | +9.4% | [-3.8%, +23.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.8 ms | 46.7 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ | hit/crossing 1.11x (was 1.01x) |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.2 ms | 10.3 ms | +0.6% | [-0.1%, +1.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.73 ms | 8.77 ms | +0.4% | [-0.8%, +1.7%] | +0.0% | ~ | hit/crossing 1.17x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 182 ms | 182 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 161 ms | 161 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 70.4 ms | 68.9 ms | -2.1% | [-4.7%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.7 ms | 56.9 ms | +0.3% | [-0.5%, +1.0%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayCrossing/mesh/firstHit/double/host` | 57.7 ms | 52.5 ms | -8.9% | [-16.4%, -0.0%] | +0.0% | faster |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.4 ms | 52.6 ms | +0.5% | [-1.0%, +1.9%] | +0.0% | ~ | hit/crossing 1.00x (was 1.10x) |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.2 ms | 11.1 ms | -0.7% | [-1.2%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.34 ms | 9.50 ms | +1.7% | [+0.0%, +3.4%] | +0.0% | ~ | hit/crossing 1.17x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 208 ms | 210 ms | +0.6% | [-1.2%, +2.5%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 186 ms | 185 ms | -0.2% | [-1.6%, +1.2%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 78.0 ms | 79.0 ms | +1.2% | [+0.3%, +2.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 66.1 ms | 66.3 ms | +0.3% | [-3.0%, +3.7%] | +0.0% | ~ | hit/crossing 1.19x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | +0.1% | [-1.5%, +1.6%] | +0.0% | ~ | marched/traced 4.68x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 52.1 ms | 51.9 ms | -0.4% | [-1.3%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.02 ms | 8.08 ms | +0.7% | [+0.2%, +1.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.93 ms | 1.93 ms | +0.0% | [-1.6%, +1.6%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.91 ms | 6.92 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.9 ms | +1.5% | [-0.6%, +3.6%] | +0.0% | ~ | marched/traced 5.47x (was 5.21x) |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.8 ms | 64.8 ms | +6.6% | [+0.5%, +12.8%] | +0.0% | minor |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.61 ms | 8.63 ms | +0.2% | [-0.5%, +1.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.93 ms | 1.96 ms | +1.8% | [+0.1%, +3.6%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.16 ms | 7.16 ms | -0.1% | [-1.1%, +1.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 240 ms | 241 ms | +0.4% | [-1.8%, +2.7%] | +0.0% | ~ | marched/traced 1.36x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 329 ms | +0.2% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 239 ms | 239 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 251 ms | 252 ms | +0.4% | [-0.5%, +1.4%] | +0.0% | ~ | marched/traced 1.48x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 374 ms | 373 ms | -0.2% | [-2.1%, +1.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 250 ms | 249 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.7 ms | 23.9 ms | +0.9% | [+0.3%, +1.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.6 ms | 23.8 ms | +0.9% | [+0.4%, +1.4%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.7 ms | 18.6 ms | -0.8% | [-1.8%, +0.3%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.1 ms | 30.1 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 30.0 ms | 30.1 ms | +0.2% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.4 ms | 20.9 ms | +2.4% | [-1.0%, +5.9%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.8 ms | 56.7 ms | -0.3% | [-1.2%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.7 ms | 37.0 ms | +0.8% | [+0.1%, +1.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.4 ms | 92.5 ms | +0.1% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.1 ms | 65.0 ms | +1.4% | [-0.5%, +3.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.8 ms | 43.0 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 108 ms | 107 ms | -0.9% | [-1.3%, -0.5%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | - | - | - | - | - | new | tape/fused 1.00x |
| `SphereCloudRays/polydisperse/firstHit/double/host` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | - | - | - | - | - | new | tape/fused 0.99x |
| `SphereContacts/polydisperse/force-morton/float/host` | 168 ms | 169 ms | +0.6% | [+0.5%, +0.7%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 192 ms | 193 ms | +0.3% | [-0.7%, +1.4%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 13.0 ms | 13.0 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.56 ms | 4.53 ms | -0.7% | [-1.6%, +0.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.9 ms | 25.0 ms | +0.3% | [-0.6%, +1.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.4 ms | 40.2 ms | -0.4% | [-0.9%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.75 ms | 1.75 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 132 µs | 139 µs | +5.1% | [+0.8%, +9.9%] | - | minor |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.0 ms | 30.1 ms | +0.4% | [-1.1%, +1.8%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.64 ms | 5.65 ms | +0.2% | [-0.0%, +0.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.2 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.60 ms | 4.64 ms | +0.7% | [+0.3%, +1.1%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.2 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.56 ms | 4.57 ms | +0.2% | [-0.5%, +0.9%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.3 ms | 26.4 ms | +0.7% | [+0.3%, +1.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.5 ms | 42.1 ms | -0.8% | [-1.4%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.77 ms | 1.76 ms | -0.9% | [-2.8%, +1.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 280 µs | 281 µs | +0.4% | [-6.4%, +7.5%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.7 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.73 ms | 7.74 ms | +0.2% | [+0.1%, +0.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.4 ms | 16.3 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.73 ms | 7.78 ms | +0.7% | [+0.3%, +1.0%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/SphereCloudRays/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 03ed8f4
base-fingerprint: 15816899ba51fec0b84d2a127f84fe70debfa842
head: 8007d3d
head-fingerprint: ea9c2f8b0f24a1f1671e3cad3eccfd38a5152ebb
cases: 302
rounds: 2
reps: 3
date: 2026-11-05
-->

## Device comparison (CUDA): red

**Accepted:** Accepted as noise

`8007d3d` against `03ed8f4` (origin/main), 2026-11-05.

**VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 1 major, 2 minor, 3 faster, 0 differ, 0 lost, 302 cases

- base: 03ed8f4 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8007d3d (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.60 ms | -0.5% | [-1.5%, +0.4%] | - | ~ | device 22.7x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.38 ms | 3.39 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.05 ms | +0.2% | [-0.6%, +1.0%] | - | ~ | device 19.6x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.31 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 16.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.16 ms | 6.11 ms | -0.8% | [-2.2%, +0.5%] | - | ~ | device 7.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.8 ms | 11.7 ms | -0.6% | [-2.5%, +1.3%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.4 ms | +0.3% | [-0.4%, +1.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 56.9 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.7 ms | 20.5 ms | -1.1% | [-2.0%, -0.3%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.9 ms | 47.9 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.3 ms | 62.1 ms | +1.4% | [+0.2%, +2.5%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 107 ms | -4.3% | [-5.0%, -3.5%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.35 ms | 8.32 ms | -0.4% | [-2.8%, +2.2%] | +0.0% | ~ | device 4.9x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 10.1 ms | 9.81 ms | -2.7% | [-4.7%, -0.7%] | +0.0% | ~ | device 8.3x faster than the host (was 8.0x) |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 386 µs | 385 µs | -0.1% | [-0.7%, +0.4%] | - | ~ | device 15.2x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 840 µs | 838 µs | -0.3% | [-0.6%, -0.0%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.53 ms | 4.52 ms | -0.2% | [-0.9%, +0.4%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.65 ms | 4.67 ms | +0.5% | [+0.1%, +1.0%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 579 µs | 577 µs | -0.4% | [-0.8%, -0.1%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.36 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.77 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.00 ms | 7.02 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 893 µs | 895 µs | +0.3% | [-0.3%, +0.9%] | - | ~ | device 14.6x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.46 ms | 9.49 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 319 µs | 320 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 11.4x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 5.1x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.8 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.53 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 520 ms | 516 ms | -0.8% | [-2.0%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 48.0 µs | 48.6 µs | +1.3% | [-0.6%, +3.2%] | - | ~ | device 23.7x faster than the host; tape/fused 4.49x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.9 µs | 49.8 µs | -0.3% | [-0.8%, +0.3%] | - | ~ | device 43.3x faster than the host; tape/fused 7.22x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.0 µs | 48.0 µs | +0.2% | [-0.4%, +0.7%] | - | ~ | device 9.0x faster than the host (was 9.4x); tape/fused 9.87x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.2 ms | 17.3 ms | +0.4% | [+0.1%, +0.8%] | - | ~ | device 7.0x faster than the host; tape/fused 1.82x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.4 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 809 µs | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.05x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.11 ms | 2.10 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 287 ms | 284 ms | -1.0% | [-2.8%, +0.8%] | - | ~ | device 0.4x faster than the host; tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 285 ms | 283 ms | -0.7% | [-3.3%, +2.0%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.24 ms | 8.29 ms | +0.5% | [-0.5%, +1.6%] | +0.0% | ~ | device 14.7x faster than the host (was 10.7x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.25 ms | 8.23 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.26 ms | 8.23 ms | -0.3% | [-1.8%, +1.1%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.25 ms | 8.22 ms | -0.4% | [-0.8%, -0.0%] | +0.0% | ~ | device 10.9x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 688 µs | 689 µs | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ | device 4.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.72 ms | 1.72 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.07 ms | 1.07 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 3.1x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 801 µs | 799 µs | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ | device 4.4x faster than the host (was 4.1x); default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 739 µs | 738 µs | -0.1% | [-0.9%, +0.7%] | +0.0% | ~ | device 12.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 829 µs | 833 µs | +0.5% | [-0.4%, +1.3%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 744 µs | 740 µs | -0.5% | [-0.7%, -0.4%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 728 µs | 736 µs | +1.1% | [-0.1%, +2.4%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 159 ms | 158 ms | -0.7% | [-1.5%, +0.2%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 157 ms | 162 ms | +2.7% | [-0.2%, +5.8%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x); default/this 0.98x (was 1.01x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 161 ms | 165 ms | +2.4% | [-0.0%, +4.8%] | +0.0% | ~ | device 0.5x faster than the host (was 0.7x); default/this 0.96x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 162 ms | 164 ms | +1.2% | [-1.7%, +4.4%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.97x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.76 ms | 1.77 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.91x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.96 ms | 1.96 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.82x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.67 ms | 1.68 ms | +0.4% | [+0.2%, +0.7%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.27 ms | 7.24 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.33 ms | 7.25 ms | -1.1% | [-1.8%, -0.5%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.24 ms | 7.23 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.29 ms | 7.33 ms | +0.5% | [-0.3%, +1.2%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.39 ms | 8.44 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 11.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.6 ms | -0.1% | [-1.4%, +1.1%] | - | ~ | device 7.1x faster than the host (was 9.4x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 167 ms | 168 ms | +0.4% | [-1.5%, +2.4%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 288 ms | 287 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.4 ms | 24.5 ms | +0.6% | [-0.1%, +1.3%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.8 ms | 58.6 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 157 ms | 160 ms | +1.5% | [+1.3%, +1.6%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 242 ms | 247 ms | +2.1% | [+0.3%, +4.0%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.87 ms | 2.90 ms | +1.1% | [+0.5%, +1.6%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.13 ms | 5.15 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.4 ms | 14.5 ms | +0.8% | [+0.2%, +1.3%] | - | ~ | device 3.8x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 120 ms | 121 ms | +1.0% | [-1.3%, +3.4%] | - | ~ | device 1.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 174 ms | 175 ms | +0.3% | [-3.5%, +4.3%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 378 ms | 375 ms | -0.7% | [-2.0%, +0.6%] | - | ~ | device 2.3x faster than the host (was 3.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.0 ms | 24.7 ms | -1.0% | [-2.9%, +0.9%] | - | ~ | device 2.0x faster than the host; tape/fused 4.90x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.6 ms | 26.8 ms | +1.1% | [-0.1%, +2.3%] | - | ~ | device 2.2x faster than the host; tape/fused 6.52x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.1 ms | 70.7 ms | -0.6% | [-2.2%, +1.0%] | - | ~ | device 4.3x faster than the host (was 4.6x); tape/fused 5.31x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.8 ms | 22.9 ms | +0.6% | [-2.5%, +3.7%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.7 ms | 45.0 ms | +0.6% | [-2.1%, +3.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 93.8 ms | 93.8 ms | -0.0% | [-0.9%, +0.9%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 542 ms | 550 ms | +1.5% | [-3.9%, +7.5%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 858 ms | 882 ms | +2.9% | [-0.1%, +5.9%] | - | ~ | device 0.2x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.25 s | 3.21 s | -1.1% | [-4.8%, +2.8%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 204 ms | 203 ms | -0.3% | [-1.7%, +1.1%] | - | ~ | device 0.2x faster than the host; tape/fused 2.71x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 322 ms | 310 ms | -3.5% | [-6.0%, -1.0%] | - | faster | device 0.2x faster than the host (was 0.2x); tape/fused 2.84x (was 2.67x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 589 ms | 599 ms | +1.7% | [-0.6%, +4.0%] | - | ~ | device 0.5x faster than the host; tape/fused 5.37x |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.33 ms | 5.33 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ | device 10.4x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 678 µs | 681 µs | +0.5% | [-0.1%, +1.1%] | - | ~ | device 11.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.82 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 996 µs | 994 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 9.4x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.60 ms | 4.61 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.96 ms | 2.96 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.13 ms | 7.08 ms | -0.7% | [-0.9%, -0.5%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.80 ms | 7.79 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.4 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.46 ms | 1.46 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.88 ms | 3.88 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 525 µs | 526 µs | +0.3% | [-0.7%, +1.3%] | - | ~ | device 15.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.43 ms | 4.44 ms | +0.3% | [-0.7%, +1.2%] | - | ~ | device 6.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.35 ms | 4.36 ms | +0.1% | [-2.2%, +2.4%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.07 ms | 8.10 ms | +0.3% | [-1.0%, +1.6%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.8 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.00 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.6 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.8 ms | 15.0 ms | +1.2% | [-0.0%, +2.5%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.57 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.56 ms | 4.56 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 832 µs | 832 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.86 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.14 ms | 4.14 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.2% | [-1.2%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.0 ms | 21.0 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.98 ms | 8.94 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.5 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.3 ms | 20.3 ms | +0.1% | [-0.8%, +1.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.77 ms | 2.76 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.75 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.39 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.71 ms | 2.73 ms | +0.6% | [+0.0%, +1.2%] | - | ~ | device 7.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.49 ms | 6.51 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.7 ms | 33.7 ms | +0.1% | [-0.9%, +1.2%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.5 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.4 ms | -0.7% | [-1.1%, -0.3%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.8 ms | +0.0% | [-1.3%, +1.3%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.4 ms | 38.4 ms | +0.1% | [-1.0%, +1.1%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.66 ms | 9.64 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.9 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.29 ms | 3.30 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.59 ms | 2.58 ms | -0.1% | [-1.0%, +0.8%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.56 ms | 4.58 ms | +0.4% | [-0.5%, +1.2%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.05 ms | 3.05 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 672 µs | 675 µs | +0.5% | [-0.1%, +1.0%] | - | ~ | device 16.7x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 14.5x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 648 µs | 637 µs | -1.6% | [-3.9%, +0.7%] | - | ~ | device 17.5x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 609 µs | 609 µs | -0.0% | [-0.7%, +0.7%] | - | ~ | device 15.9x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | -0.2% | [-0.8%, +0.5%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.47 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.08 ms | +0.8% | [+0.1%, +1.6%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.17 ms | 2.18 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 628 µs | 608 µs | -3.1% | [-6.3%, +0.1%] | - | ~ | device 2.5x faster than the host (was 2.4x); tape/fused 1.13x (was 1.09x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 658 µs | 726 µs | +10.4% | [+9.6%, +11.2%] | - | **MAJOR** | device 2.5x faster than the host (was 2.8x); tape/fused 1.15x (was 1.28x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 686 µs | 690 µs | +0.5% | [+0.2%, +0.9%] | - | ~ | device 3.9x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 842 µs | 832 µs | -1.1% | [-1.4%, -0.9%] | - | ~ | device 3.7x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.5% | [-0.5%, -0.4%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.3% | [-0.5%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 14.9 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.4 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.5 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.5% | [-0.2%, +1.1%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.11x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.3% | [-0.5%, +1.1%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.60 ms | 2.62 ms | +0.6% | [+0.1%, +1.0%] | - | ~ | device 0.8x faster than the host; tape/fused 1.03x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.61 ms | 1.60 ms | -0.5% | [-0.8%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.70 ms | -0.4% | [-1.1%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.55 ms | 2.56 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 2.9x faster than the host; marched/traced 3.53x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.7x faster than the host; marched/traced 4.60x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.05 ms | 9.03 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.17 ms | 2.18 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 3.9x faster than the host; marched/traced 16.50x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.44 ms | 5.45 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 2.8x faster than the host; marched/traced 13.14x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 36.0 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.7 ms | 71.6 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.23 ms | 5.25 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 3.1x faster than the host (was 3.0x) |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.80 ms | 8.82 ms | +0.3% | [-0.0%, +0.5%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.32 ms | 6.24 ms | -1.3% | [-1.6%, -1.1%] | - | ~ | device 1.3x faster than the host (was 1.2x); marched/traced 25.56x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.5% | [+0.1%, +0.8%] | - | ~ | device 0.8x faster than the host; marched/traced 23.86x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 161 ms | 159 ms | -0.9% | [-3.5%, +1.7%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 254 ms | 254 ms | -0.2% | [-2.7%, +2.3%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.84 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.8x faster than the host; marched/traced 23.31x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.3x faster than the host; marched/traced 17.86x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 113 ms | 113 ms | +0.3% | [-2.0%, +2.6%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 217 ms | 217 ms | +0.3% | [-2.7%, +3.3%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.2 ms | -0.2% | [-1.0%, +0.7%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 673 µs | 683 µs | +1.5% | [+1.1%, +1.8%] | - | ~ | device 16.9x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.5% | [-1.4%, +0.3%] | - | ~ | device 14.9x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 677 µs | 674 µs | -0.5% | [-0.8%, -0.2%] | - | ~ | device 17.1x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 687 µs | 688 µs | +0.1% | [+0.0%, +0.3%] | - | ~ | device 4.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 838 µs | 837 µs | -0.1% | [-0.4%, +0.3%] | - | ~ | device 4.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 627 µs | 627 µs | -0.1% | [-0.7%, +0.5%] | - | ~ | device 4.3x faster than the host; hit/crossing 1.10x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 646 µs | 645 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 4.7x faster than the host; hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.72 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 10.7x faster than the host (was 10.3x) |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.19 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 10.1x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.10 ms | 9.10 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 6.9x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 260 ms | 268 ms | +3.0% | [+2.0%, +3.9%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 218 ms | 211 ms | -3.3% | [-3.8%, -2.7%] | - | faster | device 0.9x faster than the host (was 0.9x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 55.2 ms | 55.1 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 2.6x faster than the host; hit/crossing 4.86x (was 4.71x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 70.9 ms | 71.1 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 2.2x faster than the host; hit/crossing 2.97x (was 3.08x) |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.6% | [-1.0%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.61 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.72 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.29 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.25x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.35 ms | 1.35 ms | +0.3% | [-0.5%, +1.1%] | - | ~ | device 2.5x faster than the host; hit/crossing 2.01x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.6 ms | 64.6 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 121 ms | 123 ms | +1.2% | [+0.4%, +1.9%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.9 ms | 58.7 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 111 ms | 110 ms | -0.5% | [-1.1%, +0.2%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 462 ms | 483 ms | +4.6% | [-1.3%, +10.6%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 725 ms | 712 ms | -1.8% | [-3.5%, -0.0%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 383 ms | 419 ms | +9.5% | [+4.9%, +14.4%] | - | minor | device 0.4x faster than the host (was 0.4x); hit/crossing 1.15x (was 1.21x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 536 ms | 536 ms | -0.1% | [-1.3%, +1.2%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.33x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.62 ms | 3.60 ms | -0.6% | [-2.5%, +1.3%] | - | ~ | device 1.2x faster than the host; marched/traced 5.51x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.78 ms | 4.80 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 0.9x faster than the host; marched/traced 8.37x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.6 ms | 19.8 ms | +1.0% | [+0.6%, +1.4%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.2 ms | 40.1 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.26 ms | +1.4% | [+0.5%, +2.4%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.85 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.2 µs | 93.1 µs | -2.2% | [-4.5%, +0.3%] | - | ~ | device 15.6x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 151 µs | 151 µs | -0.0% | [-1.1%, +1.1%] | - | ~ | device 11.0x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 800 µs | 800 µs | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 936 µs | 935 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 33.9 ms | 35.3 ms | +4.2% | [-2.7%, +11.5%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 3.49x (was 3.67x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 37.2 ms | 37.0 ms | -0.6% | [-0.9%, -0.2%] | - | ~ | device 0.1x faster than the host; marched/traced 6.33x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 124 ms | 123 ms | -1.0% | [-2.0%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 233 ms | 234 ms | +0.4% | [-1.3%, +2.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.4 ms | 16.9 ms | +2.9% | [-0.1%, +6.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 22.2 ms | +2.0% | [+1.5%, +2.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.08 ms | 1.06 ms | -1.8% | [-7.6%, +4.4%] | - | ~ | device 1.4x faster than the host (was 1.5x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.36 ms | 2.34 ms | -0.5% | [-1.9%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 6.00 ms | +0.4% | [-0.4%, +1.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.74 ms | 6.73 ms | -0.2% | [-0.8%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | -0.0% | [-1.2%, +1.2%] | - | ~ | device 2.2x faster than the host; marched/traced 4.28x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.4 ms | 28.5 ms | +0.4% | [-1.2%, +2.1%] | - | ~ | device 1.9x faster than the host; marched/traced 5.54x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 101 ms | 98.2 ms | -2.8% | [-3.6%, -2.0%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 160 ms | 158 ms | -1.0% | [-4.9%, +3.1%] | - | ~ | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.9 ms | 13.8 ms | -0.5% | [-0.9%, -0.0%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.4 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 299 ms | 303 ms | +1.1% | [+0.4%, +1.8%] | - | ~ | device 0.2x faster than the host; marched/traced 1.87x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 340 ms | 356 ms | +4.9% | [+4.5%, +5.2%] | - | minor | device 0.2x faster than the host (was 0.2x); marched/traced 2.78x (was 2.95x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 563 ms | 565 ms | +0.3% | [-1.3%, +2.1%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.00 s | 990 ms | -1.2% | [-1.7%, -0.6%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 190 ms | 189 ms | -0.5% | [-3.0%, +2.0%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 242 ms | 241 ms | -0.5% | [-2.8%, +2.0%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.57 ms | 4.57 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.57 ms | 4.57 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.89 ms | 1.88 ms | -0.6% | [-1.6%, +0.5%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.97 ms | 7.98 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.95 ms | 7.94 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.4 ms | 23.3 ms | -0.4% | [-1.4%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 886 µs | 887 µs | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 10.4x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.65 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ | device 3.8x faster than the host (was 3.6x) |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 13.9 ms | 14.0 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.1 ms | 11.1 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | - | - | - | - | - | new | tape/fused 0.73x |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | - | - | - | - | - | new | tape/fused 0.79x |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | - | - | - | - | - | new | tape/fused 0.98x |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | - | - | - | - | - | new | tape/fused 0.95x |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ | device 10.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.2 ms | 31.1 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ | device 0.6x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 218 µs | +0.4% | [+0.1%, +0.7%] | - | ~ | device 11.3x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 24.2x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 361 µs | 360 µs | -0.2% | [-0.4%, -0.1%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.02 ms | 2.02 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.5 µs | 72.5 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 6.8x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 48.4 µs | 47.6 µs | -1.6% | [-2.1%, -1.2%] | - | ~ | device 5.1x faster than the host (was 4.6x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 651 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 12.2x faster than the host (was 11.6x) |
| `Tape/chain16/signedDistance/float/hostdevice` | 473 µs | 474 µs | +0.3% | [+0.2%, +0.4%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 48.0 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 27.2x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 855 µs | 853 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.72 ms | 6.72 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 190 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [+0.1%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.77 ms | 2.78 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |


The probes differ between the sides (each side ran its own): Performance/SphereCloudRays/main.cpp

<!-- END receipt cuda -->
