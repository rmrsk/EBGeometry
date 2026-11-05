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
head: 1da3177
head-fingerprint: 6a29ffbed623e4f006b63d0aa59ce6324c74cfab
cases: 204
rounds: 2
reps: 3
date: 2026-11-05
-->

## Host comparison: yellow

**Accepted:** Accepted as noise

`1da3177` against `03ed8f4` (origin/main), 2026-11-05.

**VERDICT: 1 minor regression; rerun it with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 1 minor, 1 faster, 0 differ, 0 lost, 204 cases

- base: 03ed8f4 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 1da3177 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 330 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 514 ms | 513 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 347 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 564 ms | 562 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 42.0 ms | 41.4 ms | -1.5% | [-3.1%, +0.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.2 ms | 75.5 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.5 ms | 55.2 ms | -0.5% | [-0.9%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.5 ms | 51.3 ms | -0.5% | [-0.9%, -0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.9 ms | 64.5 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.7 ms | 54.9 ms | +0.5% | [+0.0%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.4 ms | 60.4 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.2 ms | 72.3 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.5 ms | 74.7 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.5 ms | 77.8 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.5 ms | 66.6 ms | +0.2% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.8 ms | 100 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.1 ms | 83.1 ms | -0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | -0.6% | [-1.7%, +0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.6 ms | 19.5 ms | -0.2% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.1 ms | 45.1 ms | +0.0% | [-2.5%, +2.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.1 ms | 73.1 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.2 ms | 20.2 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.7 ms | 46.0 ms | +0.7% | [+0.5%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.2 ms | 81.6 ms | -0.7% | [-2.7%, +1.2%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.46 ms | 4.51 ms | +1.0% | [+0.0%, +2.1%] | - | ~ | tape/fused 2.84x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.4% | [+0.1%, +0.8%] | - | ~ | tape/fused 2.03x |
| `Fused/chain16/signedDistance/float/host` | 961 µs | 962 µs | +0.1% | [-1.6%, +1.7%] | - | ~ | tape/fused 16.78x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.0 ms | 46.1 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.5 ms | 46.2 ms | -0.6% | [-1.2%, +0.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.53 ms | 4.58 ms | +1.2% | [+0.5%, +1.8%] | - | ~ | tape/fused 2.87x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | tape/fused 2.07x |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.92 ms | +0.7% | [-0.5%, +2.0%] | - | ~ | tape/fused 4.12x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.9 ms | 52.7 ms | -0.3% | [-1.2%, +0.6%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.3 ms | 52.8 ms | -0.9% | [-1.2%, -0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.8 ms | 81.7 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 229 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | -0.5% | [-1.2%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 262 ms | 263 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 69.9 ms | 70.1 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 213 ms | -0.8% | [-1.6%, +0.0%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 97.0 ms | 96.3 ms | -0.8% | [-1.0%, -0.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.4 ms | 29.4 ms | -0.1% | [-2.2%, +2.1%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 757 ms | 758 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 117 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.5 ms | 43.6 ms | +0.2% | [-2.4%, +2.8%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 856 ms | 853 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.7 ms | 26.7 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.2 ms | 26.9 ms | -1.0% | [-2.6%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 320 ms | 319 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 240 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.6 ms | +0.2% | [-0.2%, +0.7%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.8 ms | 100 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ | tape/fused 2.39x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.7 ms | 29.8 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.1 ms | 30.3 ms | +0.7% | [-0.5%, +1.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 359 ms | 359 ms | -0.1% | [-2.8%, +2.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 254 ms | 265 ms | +4.3% | [+4.1%, +4.5%] | +0.0% | minor |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 98.4 ms | 98.2 ms | -0.2% | [-2.6%, +2.3%] | +0.0% | ~ | tape/fused 3.66x |
| `MeshCSG/smooth-fused/frame/double/host` | 111 ms | 109 ms | -1.8% | [-4.6%, +1.1%] | +0.0% | ~ | tape/fused 2.43x (was 2.29x) |
| `MeshReconcile/armadillo/reconcile/float/host` | 131 ms | 131 ms | -0.3% | [-1.2%, +0.6%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.3 ms | 41.2 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ | reconcile/scheduled 3.18x |
| `MeshReconcile/armadillo/reconcile/double/host` | 156 ms | 156 ms | +0.2% | [-1.5%, +1.9%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.7 ms | 56.4 ms | -0.6% | [-0.8%, -0.4%] | +0.0% | ~ | reconcile/scheduled 2.77x |
| `PointCloudBVH/uniform/build/float/host` | 53.4 ms | 53.4 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 143 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 354 ms | 354 ms | -0.1% | [-0.5%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 79.4 ms | 79.2 ms | -0.2% | [-0.4%, -0.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.66 ms | 6.68 ms | +0.3% | [-0.2%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.9 ms | 55.8 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 156 ms | 160 ms | +2.6% | [+0.1%, +5.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 386 ms | 386 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.9 ms | 86.4 ms | -0.6% | [-1.0%, -0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.91 ms | 8.84 ms | +27.9% | [-0.7%, +56.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.5 ms | 13.4 ms | -1.1% | [-1.4%, -0.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 476 ms | 476 ms | +0.0% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 142 ms | 142 ms | -0.2% | [-0.6%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 800 ms | 800 ms | +0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.8 ms | 14.8 ms | -0.1% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 494 ms | 493 ms | -0.1% | [-0.6%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 144 ms | +0.3% | [+0.3%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 791 ms | 790 ms | -0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.6 ms | 54.3 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 128 ms | 128 ms | +0.0% | [-0.4%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 261 ms | 262 ms | +0.3% | [-0.6%, +1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 148 ms | 147 ms | -0.2% | [-0.4%, +0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 133 ms | 134 ms | +0.3% | [+0.1%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.2 ms | 57.4 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 139 ms | 139 ms | +0.4% | [-0.3%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 290 ms | 291 ms | +0.1% | [-0.2%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 162 ms | 161 ms | -0.3% | [-0.9%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 148 ms | 148 ms | -0.3% | [-1.2%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.5 ms | 11.6 ms | +1.3% | [-0.3%, +2.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 116 ms | 115 ms | -0.7% | [-1.3%, -0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 316 ms | 317 ms | +0.0% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 155 ms | +0.2% | [-0.1%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 320 ms | 320 ms | +0.0% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.4 ms | 13.4 ms | +0.0% | [-0.6%, +0.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 115 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 348 ms | 347 ms | -0.1% | [-0.9%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 166 ms | 167 ms | +0.6% | [+0.1%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 308 ms | -0.3% | [-0.7%, +0.0%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.1 ms | 35.2 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.3 ms | 38.4 ms | +0.2% | [-0.9%, +1.2%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 10.9 ms | 8.26 ms | -24.0% | [-38.6%, -0.4%] | +0.0% | faster |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.29 ms | 9.28 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.3 ms | 46.6 ms | +0.6% | [+0.3%, +1.0%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.7 ms | 45.8 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.6 ms | 40.3 ms | -0.8% | [-1.4%, -0.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.5 ms | 78.9 ms | +0.5% | [+0.0%, +0.9%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.8 ms | 65.6 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.85 ms | 5.82 ms | -0.6% | [-1.3%, +0.2%] | - | ~ | tape/fused 1.37x |
| `Ray/csg-tape/firstHit/float/host` | 8.15 ms | 7.95 ms | -2.5% | [-3.4%, -1.6%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 53.0 ms | 52.5 ms | -0.9% | [-1.3%, -0.5%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.2 ms | 51.2 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.3 ms | 44.9 ms | -0.7% | [-1.9%, +0.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.1 ms | 88.2 ms | +0.1% | [-1.0%, +1.2%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 74.3 ms | 74.0 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.95 ms | 6.04 ms | +1.6% | [+1.2%, +2.0%] | - | ~ | tape/fused 1.38x |
| `Ray/csg-tape/firstHit/double/host` | 8.36 ms | 8.32 ms | -0.5% | [-1.0%, +0.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 19.1 ms | 18.8 ms | -1.3% | [-2.4%, -0.2%] | - | ~ | marched/traced 6.34x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 119 ms | -0.1% | [-0.1%, -0.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 21.9 ms | -0.5% | [-1.1%, +0.0%] | +0.0% | ~ | marched/traced 7.08x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 155 ms | 155 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.5 ms | 35.3 ms | -0.4% | [-0.9%, +0.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 20.1 ms | 19.9 ms | -1.2% | [-1.4%, -0.9%] | - | ~ | marched/traced 6.37x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 127 ms | +0.1% | [+0.0%, +0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.4 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ | marched/traced 6.83x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 160 ms | 160 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.0 ms | 37.9 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.3 ms | 51.9 ms | +12.0% | [-0.0%, +24.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.7 ms | 46.3 ms | -0.8% | [-1.8%, +0.3%] | +0.0% | ~ | hit/crossing 1.12x (was 0.99x) |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.1 ms | 10.1 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.79 ms | 8.74 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ | hit/crossing 1.16x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 181 ms | 181 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 161 ms | 160 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.7 ms | 68.7 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.3 ms | 56.3 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.4 ms | 52.7 ms | +0.4% | [-0.8%, +1.7%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.1 ms | 52.3 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ | hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.3 ms | 11.1 ms | -1.5% | [-2.7%, -0.2%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.35 ms | 9.45 ms | +1.1% | [+0.9%, +1.3%] | +0.0% | ~ | hit/crossing 1.17x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 207 ms | 207 ms | +0.1% | [-0.5%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 191 ms | 185 ms | -3.1% | [-6.2%, +0.2%] | +0.0% | ~ | hit/crossing 1.12x (was 1.08x) |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 79.0 ms | 79.3 ms | +0.5% | [-1.4%, +2.3%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 65.4 ms | 65.1 ms | -0.5% | [-2.5%, +1.6%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.0 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ | marched/traced 4.71x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.6 ms | 51.8 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.04 ms | 7.99 ms | -0.6% | [-1.3%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.92 ms | 1.95 ms | +1.5% | [+0.8%, +2.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.94 ms | 6.86 ms | -1.2% | [-1.6%, -0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 12.0 ms | +2.7% | [+0.0%, +5.5%] | +0.0% | ~ | marched/traced 5.10x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.7 ms | 61.1 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.61 ms | 8.63 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.94 ms | 1.92 ms | -1.0% | [-2.4%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.11 ms | 7.18 ms | +1.0% | [+0.1%, +1.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 238 ms | 238 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 327 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 236 ms | 237 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 250 ms | 251 ms | +0.4% | [+0.1%, +0.6%] | +0.0% | ~ | marched/traced 1.47x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 370 ms | 368 ms | -0.6% | [-1.6%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 250 ms | 249 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.9 ms | 23.8 ms | -0.2% | [-1.0%, +0.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.7 ms | 23.7 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.5 ms | 18.5 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.3 ms | 30.3 ms | +0.3% | [-0.8%, +1.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 29.9 ms | 29.8 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.7 ms | 20.6 ms | -0.5% | [-3.1%, +2.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.3 ms | 56.4 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.8 ms | 36.8 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.4 ms | 92.3 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.1 ms | 64.1 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.8 ms | 42.9 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 107 ms | 108 ms | +0.3% | [-0.3%, +1.0%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | - | - | - | - | - | new | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | - | - | - | - | - | new |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | - | - | - | - | - | new | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | - | - | - | - | - | new |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 166 ms | 166 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 191 ms | 191 ms | -0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.8 ms | -0.7% | [-1.9%, +0.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.57 ms | 4.53 ms | -0.7% | [-2.6%, +1.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.0 ms | 25.0 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.5 ms | 40.2 ms | -0.7% | [-1.1%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.74 ms | 1.73 ms | -0.3% | [-1.2%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 132 µs | 134 µs | +1.2% | [-9.0%, +12.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.9 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.66 ms | +0.1% | [-0.5%, +0.6%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.1 ms | 16.1 ms | +0.3% | [-1.0%, +1.6%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.63 ms | 4.63 ms | -0.0% | [-0.2%, +0.1%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.1 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.62 ms | 4.52 ms | -2.2% | [-2.6%, -1.9%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.2 ms | 26.3 ms | +0.4% | [+0.2%, +0.7%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.2 ms | 42.2 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.75 ms | 1.77 ms | +0.8% | [+0.0%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 288 µs | 289 µs | +0.4% | [-1.3%, +2.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.7 ms | -0.5% | [-1.4%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.75 ms | 7.74 ms | -0.1% | [-0.6%, +0.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.1 ms | 16.1 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.76 ms | 7.71 ms | -0.6% | [-1.3%, +0.0%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/SphereCloudRays/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 03ed8f4
base-fingerprint: 15816899ba51fec0b84d2a127f84fe70debfa842
head: 1da3177
head-fingerprint: 6a29ffbed623e4f006b63d0aa59ce6324c74cfab
cases: 302
rounds: 2
reps: 3
date: 2026-11-05
-->

## Device comparison (CUDA): yellow

**Accepted:** Accepted as noise

`1da3177` against `03ed8f4` (origin/main), 2026-11-05.

**VERDICT: 2 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 2 minor, 4 faster, 0 differ, 0 lost, 302 cases

- base: 03ed8f4 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 1da3177 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.60 ms | -1.1% | [-1.4%, -0.8%] | - | ~ | device 22.9x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.38 ms | 3.41 ms | +0.8% | [-0.1%, +1.6%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.04 ms | 1.04 ms | -0.2% | [-3.1%, +2.8%] | - | ~ | device 19.7x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.32 ms | +0.6% | [-0.2%, +1.4%] | - | ~ | device 16.5x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.14 ms | 6.19 ms | +0.9% | [-0.5%, +2.2%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.8 ms | 11.8 ms | +0.7% | [-0.9%, +2.3%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.4 ms | +0.2% | [-1.3%, +1.6%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.1 ms | 57.1 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.6 ms | 20.7 ms | +0.2% | [-1.2%, +1.7%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.8 ms | 47.9 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.0 ms | 61.4 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 110 ms | +0.9% | [-2.3%, +4.3%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.22 ms | 8.32 ms | +1.3% | [+0.3%, +2.3%] | +0.0% | ~ | device 4.9x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.88 ms | 9.82 ms | -0.7% | [-3.1%, +1.9%] | +0.0% | ~ | device 8.1x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 384 µs | 386 µs | +0.4% | [+0.0%, +0.9%] | - | ~ | device 15.3x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 839 µs | 840 µs | +0.2% | [+0.1%, +0.2%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.53 ms | 4.53 ms | -0.0% | [-0.7%, +0.7%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.68 ms | +0.5% | [+0.3%, +0.7%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 578 µs | 578 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.36 ms | 1.36 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.77 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.02 ms | 7.02 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 898 µs | 897 µs | -0.0% | [-0.8%, +0.8%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.50 ms | 9.46 ms | -0.4% | [-0.8%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 320 µs | 320 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 11.4x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 5.1x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.8 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.53 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 522 ms | 520 ms | -0.3% | [-1.3%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 24.7x faster than the host; tape/fused 4.55x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.7 µs | 49.6 µs | -0.1% | [-0.4%, +0.1%] | - | ~ | device 44.1x faster than the host; tape/fused 7.29x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.1 µs | 48.1 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 10.1x faster than the host (was 9.1x); tape/fused 9.85x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.2 ms | 17.2 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 7.3x faster than the host (was 6.8x); tape/fused 1.82x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.3 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 809 µs | 810 µs | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.5x faster than the host; tape/fused 1.05x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 285 ms | 281 ms | -1.5% | [-2.9%, -0.1%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 283 ms | 284 ms | +0.5% | [-1.1%, +2.2%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.24 ms | 8.26 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ | device 14.8x faster than the host (was 10.7x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.33 ms | 8.25 ms | -0.9% | [-2.6%, +0.7%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.22 ms | 8.28 ms | +0.7% | [-0.7%, +2.2%] | +0.0% | ~ | device 11.7x faster than the host (was 11.1x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.18 ms | 8.24 ms | +0.7% | [-0.5%, +1.9%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 688 µs | 691 µs | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ | device 4.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.72 ms | 1.72 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.06 ms | 1.06 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ | device 3.1x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 798 µs | 795 µs | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ | device 4.1x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 740 µs | 740 µs | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 831 µs | 836 µs | +0.7% | [+0.3%, +1.0%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 741 µs | 742 µs | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 735 µs | 731 µs | -0.5% | [-1.6%, +0.5%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 161 ms | 160 ms | -0.3% | [-1.6%, +0.9%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 157 ms | 159 ms | +1.3% | [-0.4%, +3.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 161 ms | 157 ms | -2.1% | [-3.1%, -1.1%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x); default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 161 ms | 163 ms | +1.1% | [-0.8%, +3.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.61 ms | +0.4% | [+0.0%, +0.8%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.77 ms | 1.77 ms | -0.2% | [-1.0%, +0.5%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.91x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.96 ms | 1.94 ms | -0.6% | [-1.0%, -0.3%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.83x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.68 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.96x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.23 ms | 7.23 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.28 ms | 7.29 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.25 ms | 7.25 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.29 ms | 7.29 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.45 ms | 8.46 ms | +0.1% | [-1.5%, +1.7%] | - | ~ | device 11.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.4 ms | -1.1% | [-1.4%, -0.7%] | - | ~ | device 7.2x faster than the host (was 8.7x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 165 ms | 165 ms | -0.2% | [-1.0%, +0.7%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 282 ms | 282 ms | -0.2% | [-2.0%, +1.7%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.4 ms | 24.3 ms | -0.4% | [-1.3%, +0.5%] | - | ~ | device 4.7x faster than the host (was 4.0x) |
| `MeshSDF/query-random/float/hostdevice` | 58.6 ms | 58.7 ms | +0.3% | [+0.2%, +0.3%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 156 ms | 158 ms | +1.4% | [-0.8%, +3.6%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 240 ms | 246 ms | +2.3% | [+1.0%, +3.8%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.93 ms | 2.89 ms | -1.4% | [-2.5%, -0.4%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.16 ms | 5.14 ms | -0.4% | [-0.8%, -0.0%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.5 ms | 14.5 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 3.8x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 119 ms | 120 ms | +1.0% | [+0.1%, +1.8%] | - | ~ | device 2.1x faster than the host (was 1.7x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 172 ms | 172 ms | +0.2% | [-2.2%, +2.7%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 377 ms | 372 ms | -1.3% | [-2.4%, -0.1%] | - | ~ | device 2.3x faster than the host (was 2.2x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.5 ms | 24.6 ms | +0.4% | [+0.3%, +0.6%] | - | ~ | device 2.0x faster than the host; tape/fused 4.89x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.8 ms | 26.9 ms | +0.5% | [+0.3%, +0.6%] | - | ~ | device 2.2x faster than the host; tape/fused 6.41x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.7 ms | 71.3 ms | -0.6% | [-1.6%, +0.4%] | - | ~ | device 4.7x faster than the host (was 4.0x); tape/fused 5.22x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 23.7 ms | 23.6 ms | -0.5% | [-4.0%, +3.0%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 43.9 ms | 44.6 ms | +1.6% | [-0.3%, +3.7%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 94.5 ms | 95.9 ms | +1.5% | [-2.3%, +5.4%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 519 ms | 560 ms | +7.7% | [+7.2%, +8.3%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 846 ms | 857 ms | +1.3% | [+0.7%, +1.8%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.24 s | 3.24 s | -0.0% | [-1.1%, +1.1%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 197 ms | 198 ms | +0.3% | [-2.3%, +3.1%] | - | ~ | device 0.2x faster than the host; tape/fused 2.83x (was 2.64x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 311 ms | 315 ms | +1.5% | [-7.1%, +10.6%] | - | ~ | device 0.2x faster than the host; tape/fused 2.72x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 592 ms | 601 ms | +1.5% | [+0.9%, +2.1%] | - | ~ | device 0.5x faster than the host; tape/fused 5.40x |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.65 ms | 2.65 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.33 ms | 5.33 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | device 10.3x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 680 µs | 684 µs | +0.6% | [-0.3%, +1.4%] | - | ~ | device 11.0x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.83 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 993 µs | 995 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 9.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.59 ms | 4.59 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.96 ms | 2.95 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.09 ms | 7.09 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.78 ms | 7.80 ms | +0.2% | [+0.0%, +0.5%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.4 ms | 13.3 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.47 ms | 1.46 ms | -0.7% | [-2.4%, +1.0%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.87 ms | 3.90 ms | +0.6% | [+0.6%, +0.7%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 525 µs | 523 µs | -0.4% | [-0.9%, +0.0%] | - | ~ | device 15.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.45 ms | 4.41 ms | -0.9% | [-1.8%, -0.0%] | - | ~ | device 6.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.31 ms | 4.29 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.09 ms | 8.15 ms | +0.8% | [+0.2%, +1.4%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.7 ms | 15.8 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.01 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.6 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.8% | [-1.5%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.60 ms | 1.58 ms | -1.0% | [-2.4%, +0.3%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.56 ms | 4.57 ms | +0.2% | [+0.2%, +0.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 834 µs | 829 µs | -0.6% | [-1.8%, +0.6%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.86 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.15 ms | 4.13 ms | -0.5% | [-1.0%, +0.1%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.6 ms | 16.6 ms | +0.1% | [-0.9%, +1.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.0 ms | 21.1 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.93 ms | 8.96 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.6 ms | +0.1% | [-0.4%, +0.7%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.1 ms | 20.3 ms | +1.1% | [+0.7%, +1.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.74 ms | 2.76 ms | +0.4% | [-0.8%, +1.6%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.74 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.39 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.71 ms | 2.73 ms | +0.6% | [+0.5%, +0.7%] | - | ~ | device 7.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.54 ms | 6.48 ms | -0.9% | [-1.3%, -0.6%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.8 ms | 34.1 ms | +0.8% | [+0.2%, +1.5%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.3 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.4 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.7 ms | 20.7 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.2 ms | 37.9 ms | -0.8% | [-1.2%, -0.4%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.7 ms | 12.8 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.66 ms | 9.65 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.8 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.29 ms | 3.27 ms | -0.6% | [-1.4%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.57 ms | 2.58 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.55 ms | 4.57 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.06 ms | 3.04 ms | -0.6% | [-1.0%, -0.1%] | +0.0% | ~ | device 3.3x faster than the host (was 3.2x) |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 681 µs | 676 µs | -0.7% | [-1.3%, -0.1%] | - | ~ | device 16.6x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 14.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 645 µs | 652 µs | +1.1% | [-0.4%, +2.5%] | - | ~ | device 17.1x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 608 µs | 610 µs | +0.4% | [-0.5%, +1.3%] | - | ~ | device 16.0x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | +0.6% | [-0.4%, +1.7%] | - | ~ | device 14.8x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.18 ms | -1.2% | [-1.6%, -0.7%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.47 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 617 µs | 595 µs | -3.5% | [-6.1%, -0.7%] | - | faster | device 2.5x faster than the host (was 2.4x); tape/fused 1.15x (was 1.11x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 692 µs | 692 µs | -0.0% | [-8.3%, +9.1%] | - | ~ | device 2.6x faster than the host; tape/fused 1.21x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 687 µs | 687 µs | +0.1% | [-0.6%, +0.7%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 836 µs | 839 µs | +0.3% | [-0.1%, +0.8%] | - | ~ | device 3.7x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.1 ms | +1.0% | [-0.2%, +2.2%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.3 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 0.7x faster than the host; hand/api 1.01x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.6% | [-0.1%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | -0.1% | [-1.0%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.1% | [-0.9%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.5 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.04x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.6 ms | 24.5 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.11x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | -0.5% | [-1.0%, +0.1%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.60 ms | 2.61 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.61 ms | 1.60 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.72 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.56 ms | 2.56 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 2.9x faster than the host; marched/traced 3.53x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.7x faster than the host; marched/traced 4.60x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.06 ms | 9.04 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.18 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 3.9x faster than the host; marched/traced 16.54x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.45 ms | 5.45 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 2.8x faster than the host; marched/traced 13.11x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 36.0 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.6 ms | 71.5 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.25 ms | 5.24 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.81 ms | 8.82 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 3.3x faster than the host (was 3.2x) |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.23 ms | 6.25 ms | +0.3% | [-0.1%, +0.8%] | - | ~ | device 1.2x faster than the host; marched/traced 25.78x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 0.8x faster than the host; marched/traced 24.10x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 158 ms | 161 ms | +1.8% | [+0.2%, +3.4%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 257 ms | 255 ms | -0.7% | [-2.3%, +0.9%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.84 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 1.8x faster than the host; marched/traced 22.77x (was 23.62x) |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.3x faster than the host; marched/traced 17.97x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 115 ms | 110 ms | -3.7% | [-4.3%, -3.1%] | - | faster | device 0.7x faster than the host (was 0.6x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 214 ms | 219 ms | +2.5% | [+1.6%, +3.3%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.2 ms | 19.3 ms | +0.6% | [+0.1%, +1.2%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.8 ms | +0.3% | [-0.1%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 670 µs | 665 µs | -0.7% | [-1.2%, -0.2%] | - | ~ | device 17.3x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.5% | [-1.3%, +0.3%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 677 µs | 673 µs | -0.6% | [-1.1%, -0.1%] | - | ~ | device 17.2x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 687 µs | 688 µs | +0.0% | [-0.2%, +0.3%] | - | ~ | device 4.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 840 µs | 840 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 4.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 626 µs | 628 µs | +0.3% | [+0.0%, +0.5%] | - | ~ | device 4.4x faster than the host; hit/crossing 1.10x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 647 µs | 643 µs | -0.6% | [-0.7%, -0.4%] | - | ~ | device 4.8x faster than the host; hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.71 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 10.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.19 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 10.1x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.12 ms | 9.10 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 270 ms | 263 ms | -2.3% | [-6.4%, +2.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 213 ms | 219 ms | +2.9% | [+1.1%, +4.7%] | - | ~ | device 0.9x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 54.8 ms | 54.5 ms | -0.6% | [-1.0%, -0.1%] | - | ~ | device 2.6x faster than the host; hit/crossing 4.83x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 71.4 ms | 71.3 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 2.2x faster than the host (was 2.4x); hit/crossing 3.08x (was 2.98x) |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.6% | [-0.5%, +1.7%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.61 ms | 1.60 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.73 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.29 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.35 ms | 1.35 ms | -0.2% | [-1.1%, +0.6%] | - | ~ | device 2.5x faster than the host; hit/crossing 2.02x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 65.1 ms | 64.8 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 123 ms | 122 ms | -0.1% | [-1.5%, +1.3%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.7 ms | 58.9 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 110 ms | 110 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 522 ms | 459 ms | -12.1% | [-14.1%, -9.9%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 709 ms | 723 ms | +1.9% | [+1.4%, +2.5%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 391 ms | 400 ms | +2.4% | [-5.2%, +10.4%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.15x (was 1.34x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 530 ms | 534 ms | +0.9% | [-1.4%, +3.1%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.35x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.59 ms | 3.59 ms | +0.2% | [-1.0%, +1.4%] | - | ~ | device 1.2x faster than the host; marched/traced 5.47x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.78 ms | 4.79 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 0.9x faster than the host; marched/traced 8.39x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.9 ms | 19.7 ms | -1.1% | [-1.5%, -0.7%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.2 ms | 40.2 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.26 ms | +1.1% | [-0.7%, +2.9%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.84 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.6 µs | 94.8 µs | -0.8% | [-2.8%, +1.4%] | - | ~ | device 15.3x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 153 µs | 151 µs | -1.2% | [-2.8%, +0.4%] | - | ~ | device 11.0x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 799 µs | 800 µs | +0.1% | [-0.1%, +0.4%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 934 µs | 935 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.6 ms | 34.7 ms | -2.6% | [-13.2%, +9.5%] | - | ~ | device 0.1x faster than the host; marched/traced 3.57x (was 3.44x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.9 ms | 36.6 ms | -0.8% | [-1.1%, -0.5%] | - | ~ | device 0.1x faster than the host; marched/traced 6.45x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 123 ms | 124 ms | +1.0% | [+0.0%, +2.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 237 ms | 236 ms | -0.2% | [-2.0%, +1.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.6 ms | 16.0 ms | -3.4% | [-9.4%, +3.0%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 22.0 ms | +0.8% | [-0.1%, +1.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.03 ms | 1.08 ms | +4.8% | [-1.1%, +10.9%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.36 ms | 2.37 ms | +0.5% | [-0.7%, +1.7%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.96 ms | 5.98 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.74 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.1 ms | 23.1 ms | +0.0% | [-0.6%, +0.7%] | - | ~ | device 2.2x faster than the host; marched/traced 4.34x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.3 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 1.9x faster than the host; marched/traced 5.54x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 99.3 ms | 100 ms | +0.8% | [-1.5%, +3.1%] | - | ~ | device 2.6x faster than the host (was 2.2x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 161 ms | 157 ms | -2.1% | [-3.1%, -1.1%] | - | ~ | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.7 ms | -0.8% | [-1.5%, -0.0%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.6 ms | 17.4 ms | -0.8% | [-1.5%, -0.1%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 302 ms | 310 ms | +2.7% | [+0.0%, +5.4%] | - | ~ | device 0.2x faster than the host; marched/traced 1.82x (was 1.89x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 345 ms | 347 ms | +0.4% | [-1.0%, +1.8%] | - | ~ | device 0.2x faster than the host; marched/traced 2.90x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 569 ms | 564 ms | -0.9% | [-1.4%, -0.4%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.00 s | 1.01 s | +0.4% | [+0.1%, +0.6%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 194 ms | 188 ms | -3.5% | [-5.2%, -1.8%] | - | faster | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 233 ms | 242 ms | +3.6% | [+0.8%, +6.5%] | - | minor | device 0.2x faster than the host (was 0.2x) |
| `Render/csg/frame/float/hostdevice` | 4.57 ms | 4.58 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.57 ms | 4.57 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.88 ms | +0.3% | [-0.4%, +1.1%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.97 ms | 7.96 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.94 ms | 7.95 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.3 ms | 23.6 ms | +1.1% | [-0.1%, +2.4%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 889 µs | 890 µs | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ | device 10.4x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.64 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.0 ms | 13.0 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 13.9 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.1 ms | 11.1 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | - | - | - | - | - | new | tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | - | - | - | - | - | new | tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | - | - | - | - | - | new | tape/fused 1.05x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | - | - | - | - | - | new | tape/fused 1.06x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | device 10.6x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.1 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 0.6x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 218 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 11.3x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 23.4x faster than the host (was 24.2x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 361 µs | 361 µs | +0.1% | [-0.1%, +0.4%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.02 ms | 2.02 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 7.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.8 µs | 72.6 µs | -0.2% | [-0.4%, -0.0%] | - | ~ | device 7.2x faster than the host (was 7.0x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.6 µs | 48.0 µs | +0.9% | [+0.3%, +1.4%] | - | ~ | device 5.0x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 651 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 11.7x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 473 µs | 474 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 47.8 µs | -0.4% | [-1.0%, +0.1%] | - | ~ | device 26.8x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 854 µs | 854 µs | -0.0% | [-0.1%, -0.0%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host (was 1.5x) |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.71 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 191 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 3.1x faster than the host (was 3.0x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.8x faster than the host (was 2.1x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.77 ms | 2.78 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |


The probes differ between the sides (each side ran its own): Performance/SphereCloudRays/main.cpp

<!-- END receipt cuda -->
