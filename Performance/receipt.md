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
base: 0c9bb61
base-fingerprint: f8fd6afee855ba75cecf95146cd72404a65d1cf7
head: a441fd1
head-fingerprint: 120383b61744256bc0008209191322a4c852373b
cases: 196
rounds: 2
reps: 3
date: 2026-11-04
-->

## Host comparison: green

**Accepted:** -

`a441fd1` against `0c9bb61` (origin/main), 2026-11-04.

**VERDICT: no regression (0 major, 0 minor across 196 cases)**

Performance comparison: 0 major, 0 minor, 2 faster, 0 differ, 0 lost, 196 cases

- base: 0c9bb61 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: a441fd1 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 332 ms | +0.5% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 514 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 349 ms | 347 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 187 ms | 187 ms | -0.5% | [-1.4%, +0.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 563 ms | 565 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.8 ms | 41.9 ms | +0.2% | [-1.0%, +1.5%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.6 ms | 75.6 ms | +0.0% | [-1.2%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.5 ms | 55.4 ms | -0.2% | [-1.3%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.5 ms | 51.7 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.9 ms | 65.6 ms | -0.5% | [-2.3%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 55.4 ms | 55.6 ms | +0.3% | [-3.2%, +4.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.6 ms | 60.5 ms | -0.2% | [-1.1%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.4 ms | 72.3 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.6 ms | 74.2 ms | -0.5% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 78.2 ms | 77.9 ms | -0.4% | [-1.0%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.1 ms | 66.4 ms | -1.0% | [-1.5%, -0.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 100 ms | +0.0% | [-0.6%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.9 ms | 83.0 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 110 ms | 108 ms | -1.4% | [-3.1%, +0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.3 ms | 19.7 ms | +2.0% | [+1.5%, +2.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.1 ms | 45.2 ms | +0.2% | [-2.3%, +2.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.9 ms | 73.3 ms | +0.5% | [+0.4%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.2 ms | 20.4 ms | +0.7% | [-0.0%, +1.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.9 ms | 46.1 ms | +0.4% | [-0.5%, +1.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.2 ms | 82.0 ms | +1.0% | [-0.8%, +2.9%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.49 ms | 4.45 ms | -0.9% | [-2.8%, +1.1%] | - | ~ | tape/fused 2.90x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | tape/fused 2.02x |
| `Fused/chain16/signedDistance/float/host` | 961 µs | 972 µs | +1.1% | [-1.6%, +4.0%] | - | ~ | tape/fused 16.51x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.2 ms | 46.1 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.2 ms | 46.2 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.56 ms | 4.60 ms | +0.9% | [+0.2%, +1.7%] | - | ~ | tape/fused 2.84x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.8 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | tape/fused 2.07x |
| `Fused/chain16/signedDistance/double/host` | 3.87 ms | 3.90 ms | +0.9% | [-0.2%, +2.0%] | - | ~ | tape/fused 4.12x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.9 ms | 52.9 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.2 ms | 52.8 ms | -0.7% | [-1.3%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.7 ms | 82.1 ms | +0.5% | [+0.2%, +0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 229 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 262 ms | 260 ms | -0.6% | [-1.5%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 69.8 ms | 69.9 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 214 ms | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.4 ms | 96.5 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 31.5 ms | 29.5 ms | -6.5% | [-9.3%, -3.6%] | - | faster |  |
| `MeshSDF/query-random/float/host` | 757 ms | 757 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 117 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 42.5 ms | 42.9 ms | +0.8% | [+0.2%, +1.3%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 862 ms | 858 ms | -0.5% | [-1.4%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.8 ms | 26.9 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.9 ms | 27.2 ms | +1.1% | [+0.2%, +1.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 320 ms | 320 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 240 ms | 240 ms | -0.1% | [-1.2%, +1.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.3 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | tape/fused 3.55x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.9 ms | 99.8 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 30.0 ms | 29.7 ms | -0.9% | [-2.0%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.1 ms | 30.4 ms | +1.1% | [+0.0%, +2.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 363 ms | 364 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 266 ms | 265 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 98.9 ms | 100 ms | +1.1% | [-1.8%, +4.1%] | +0.0% | ~ | tape/fused 3.64x |
| `MeshCSG/smooth-fused/frame/double/host` | 111 ms | 107 ms | -3.2% | [-3.3%, -3.0%] | +0.0% | faster | tape/fused 2.47x (was 2.40x) |
| `MeshReconcile/armadillo/reconcile/float/host` | 131 ms | 131 ms | +0.3% | [-0.6%, +1.1%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.4 ms | 41.4 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ | reconcile/scheduled 3.17x |
| `MeshReconcile/armadillo/reconcile/double/host` | 157 ms | 156 ms | -0.6% | [-2.2%, +1.1%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.6 ms | 56.3 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ | reconcile/scheduled 2.77x |
| `PointCloudBVH/uniform/build/float/host` | 52.9 ms | 53.3 ms | +0.9% | [+0.7%, +1.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 144 ms | +0.6% | [+0.2%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 354 ms | 353 ms | -0.0% | [-0.8%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 79.6 ms | 79.4 ms | -0.2% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.75 ms | 6.86 ms | +1.7% | [-2.4%, +5.9%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 56.3 ms | 55.7 ms | -1.0% | [-1.7%, -0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 157 ms | 156 ms | -1.0% | [-2.6%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 386 ms | 386 ms | +0.1% | [-0.5%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.5 ms | 87.1 ms | +0.7% | [+0.1%, +1.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.93 ms | 6.86 ms | -1.0% | [-1.4%, -0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.5 ms | 13.6 ms | +0.4% | [-0.9%, +1.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | -0.1% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 474 ms | 476 ms | +0.4% | [+0.2%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 141 ms | 143 ms | +1.4% | [+1.2%, +1.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 799 ms | 801 ms | +0.2% | [+0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 14.7 ms | -1.4% | [-4.9%, +2.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | -0.4% | [-0.7%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 494 ms | 493 ms | -0.3% | [-0.5%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 144 ms | +0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 783 ms | 790 ms | +0.8% | [+0.0%, +1.6%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.1 ms | 54.4 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 128 ms | 129 ms | +0.6% | [-0.1%, +1.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 260 ms | 262 ms | +0.8% | [+0.0%, +1.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 148 ms | 148 ms | -0.2% | [-1.5%, +1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 134 ms | 133 ms | -0.3% | [-1.0%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.5 ms | 57.3 ms | -0.4% | [-1.3%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 140 ms | 140 ms | +0.6% | [-0.7%, +1.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 290 ms | 291 ms | +0.3% | [+0.1%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 162 ms | 162 ms | -0.1% | [-0.7%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 148 ms | 148 ms | -0.6% | [-1.0%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.6 ms | 11.8 ms | +1.8% | [-0.4%, +4.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 115 ms | 116 ms | +0.2% | [-0.5%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 317 ms | 317 ms | +0.1% | [-0.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 154 ms | 156 ms | +1.1% | [+0.3%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 320 ms | 320 ms | -0.1% | [-0.1%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.4 ms | 13.5 ms | +0.6% | [-0.4%, +1.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 117 ms | 116 ms | -0.6% | [-2.3%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 348 ms | 347 ms | -0.4% | [-0.7%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 166 ms | 168 ms | +1.2% | [-0.1%, +2.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 309 ms | +0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.1 ms | 35.0 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.7 ms | 38.1 ms | -1.4% | [-1.5%, -1.3%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.54 ms | 8.23 ms | -3.6% | [-8.1%, +1.3%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.37 ms | 9.33 ms | -0.5% | [-0.9%, +0.0%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.3 ms | 46.5 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.7 ms | 45.7 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.5 ms | 40.3 ms | -0.4% | [-0.4%, -0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.7 ms | 78.8 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.9 ms | 65.6 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.83 ms | 5.85 ms | +0.3% | [-0.4%, +1.0%] | - | ~ | tape/fused 1.36x |
| `Ray/csg-tape/firstHit/float/host` | 8.02 ms | 7.97 ms | -0.7% | [-1.4%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.5 ms | 52.7 ms | +0.4% | [+0.0%, +0.8%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.1 ms | 51.0 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.0 ms | 44.9 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.5 ms | 87.6 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.6 ms | 73.9 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.99 ms | 5.96 ms | -0.4% | [-1.8%, +1.0%] | - | ~ | tape/fused 1.40x |
| `Ray/csg-tape/firstHit/double/host` | 8.34 ms | 8.36 ms | +0.2% | [-0.0%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.2 ms | 18.0 ms | -0.9% | [-1.4%, -0.5%] | - | ~ | marched/traced 6.63x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 119 ms | +0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.8 ms | 22.2 ms | +1.7% | [+1.1%, +2.3%] | +0.0% | ~ | marched/traced 6.96x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 154 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.2 ms | 35.2 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 20.0 ms | 20.0 ms | +0.1% | [-1.3%, +1.6%] | - | ~ | marched/traced 6.32x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 126 ms | +0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.6 ms | +0.6% | [-0.8%, +1.9%] | +0.0% | ~ | marched/traced 6.79x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 163 ms | 160 ms | -2.1% | [-3.9%, -0.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.3 ms | 38.2 ms | -0.3% | [-1.1%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.4 ms | 46.3 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.3 ms | 46.2 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.2 ms | 10.2 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.75 ms | 8.79 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ | hit/crossing 1.16x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 181 ms | 181 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 161 ms | 161 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.7 ms | 68.9 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.6 ms | 56.5 ms | -0.2% | [-0.3%, +0.0%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.1 ms | 52.2 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.3 ms | 52.4 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.1 ms | 11.2 ms | +0.8% | [-1.6%, +3.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.33 ms | 9.42 ms | +1.0% | [+0.9%, +1.0%] | +0.0% | ~ | hit/crossing 1.19x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 207 ms | 208 ms | +0.3% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 185 ms | 185 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 78.3 ms | 78.4 ms | +0.2% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.9 ms | 65.5 ms | +0.9% | [+0.6%, +1.1%] | +0.0% | ~ | hit/crossing 1.20x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.0 ms | -0.4% | [-0.8%, +0.0%] | +0.0% | ~ | marched/traced 4.69x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.6 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.02 ms | 8.03 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.93 ms | 1.93 ms | +0.0% | [-1.2%, +1.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.92 ms | 6.87 ms | -0.6% | [-1.9%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.0 ms | 12.0 ms | +0.0% | [-5.9%, +6.3%] | +0.0% | ~ | marched/traced 5.09x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.7 ms | 61.0 ms | +0.5% | [-1.0%, +2.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.81 ms | 8.67 ms | -1.7% | [-4.7%, +1.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.93 ms | 1.93 ms | -0.4% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.17 ms | 7.17 ms | -0.0% | [-0.0%, +0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 238 ms | 239 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 329 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 242 ms | 237 ms | -1.8% | [-3.5%, -0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 250 ms | 250 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ | marched/traced 1.49x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 369 ms | 372 ms | +0.9% | [+0.0%, +1.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 248 ms | 249 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.7 ms | 23.8 ms | +0.3% | [-0.4%, +1.1%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.6 ms | 23.6 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.7 ms | 18.5 ms | -1.1% | [-1.6%, -0.5%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.1 ms | 30.0 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 30.2 ms | 30.1 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.9 ms | 20.5 ms | -1.5% | [-4.0%, +1.0%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/build/double/host` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | - | - | - | - | - | new |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.56 ms | 4.56 ms | +0.2% | [-0.5%, +0.9%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.9 ms | 24.7 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.2 ms | 40.2 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.72 ms | 1.73 ms | +0.3% | [-1.0%, +1.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 127 µs | 127 µs | +0.1% | [-1.0%, +1.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.9 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.66 ms | -0.0% | [-0.5%, +0.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.2 ms | 16.1 ms | -0.7% | [-2.2%, +0.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.60 ms | 4.64 ms | +0.8% | [-0.3%, +1.8%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.0 ms | -0.6% | [-1.4%, +0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.50 ms | 4.55 ms | +1.2% | [+0.4%, +2.0%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.5 ms | 26.4 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.2 ms | 42.2 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.75 ms | 1.75 ms | -0.0% | [-1.6%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 263 µs | 264 µs | +0.6% | [-0.2%, +1.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.8 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.77 ms | 7.71 ms | -0.8% | [-1.2%, -0.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.2 ms | 16.1 ms | -0.5% | [-1.7%, +0.8%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.74 ms | 7.75 ms | +0.2% | [-0.9%, +1.3%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/SphereCloudBVH/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 0c9bb61
base-fingerprint: f8fd6afee855ba75cecf95146cd72404a65d1cf7
head: a441fd1
head-fingerprint: 120383b61744256bc0008209191322a4c852373b
cases: 288
rounds: 2
reps: 3
date: 2026-11-04
-->

## Device comparison (CUDA): yellow

**Accepted:** Noise: both cases are within noise over 10 rounds (+3.7% [-1.3%, +6.9%], +0.4% [-3.2%, +5.1%]); the PR does not touch the device ray code.

`a441fd1` against `0c9bb61` (origin/main), 2026-11-04.

**VERDICT: 2 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 2 minor, 4 faster, 0 differ, 0 lost, 288 cases

- base: 0c9bb61 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: a441fd1 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.60 ms | -0.3% | [-0.8%, +0.3%] | - | ~ | device 22.8x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.41 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | device 19.0x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.06 ms | +0.7% | [-0.2%, +1.7%] | - | ~ | device 19.3x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.30 ms | -0.4% | [-1.0%, +0.2%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.23 ms | 6.21 ms | -0.3% | [-2.9%, +2.2%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.8 ms | 11.7 ms | -0.5% | [-2.0%, +1.0%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.3 ms | -0.1% | [-1.1%, +0.9%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.1 ms | 57.1 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.7 ms | 20.6 ms | -0.4% | [-2.6%, +1.8%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.8 ms | 48.1 ms | +0.5% | [-0.1%, +1.0%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.7 ms | 61.2 ms | -0.9% | [-1.8%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 111 ms | +1.8% | [+1.7%, +1.8%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.29 ms | 8.16 ms | -1.6% | [-2.4%, -0.7%] | +0.0% | ~ | device 5.0x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.90 ms | 9.69 ms | -2.1% | [-2.4%, -1.8%] | +0.0% | ~ | device 8.3x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 384 µs | 386 µs | +0.6% | [+0.1%, +1.1%] | - | ~ | device 15.2x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 837 µs | 838 µs | +0.1% | [+0.0%, +0.1%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.54 ms | 4.51 ms | -0.7% | [-0.8%, -0.5%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.67 ms | 4.66 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 580 µs | 579 µs | -0.2% | [-1.0%, +0.7%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.36 ms | 1.37 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.77 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.02 ms | 7.02 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 894 µs | 894 µs | -0.0% | [-0.5%, +0.5%] | - | ~ | device 14.5x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.47 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 320 µs | 319 µs | -0.3% | [-0.7%, +0.1%] | - | ~ | device 11.5x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 5.1x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 59.0 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.54 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 516 ms | 518 ms | +0.3% | [-0.6%, +1.3%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.3 µs | +0.8% | [+0.1%, +1.5%] | - | ~ | device 23.7x faster than the host; tape/fused 4.50x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.8 µs | 50.1 µs | +0.6% | [+0.4%, +0.7%] | - | ~ | device 44.0x faster than the host; tape/fused 7.19x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.0 µs | 48.1 µs | +0.2% | [-0.8%, +1.2%] | - | ~ | device 9.5x faster than the host (was 8.9x); tape/fused 9.84x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.2 ms | -0.6% | [-1.5%, +0.3%] | - | ~ | device 6.8x faster than the host; tape/fused 1.82x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.3 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 285 ms | 290 ms | +1.6% | [+0.5%, +2.7%] | - | ~ | device 0.4x faster than the host; tape/fused 0.95x (was 1.00x) |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 285 ms | 276 ms | -3.1% | [-4.5%, -1.6%] | - | faster | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.31 ms | 8.23 ms | -1.0% | [-1.4%, -0.6%] | +0.0% | ~ | device 10.8x faster than the host (was 11.4x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.32 ms | 8.24 ms | -1.0% | [-2.0%, -0.0%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.24 ms | 8.18 ms | -0.6% | [-2.3%, +1.0%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.17 ms | 8.19 ms | +0.3% | [-1.3%, +2.0%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 688 µs | 684 µs | -0.7% | [-0.8%, -0.5%] | +0.0% | ~ | device 4.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.73 ms | 1.73 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.06 ms | 1.06 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 3.1x faster than the host; default/this 0.64x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 796 µs | 798 µs | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ | device 4.1x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 742 µs | 738 µs | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ | device 12.1x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 830 µs | 834 µs | +0.5% | [+0.0%, +1.0%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 746 µs | 741 µs | -0.7% | [-1.5%, +0.1%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 734 µs | 728 µs | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 165 ms | 162 ms | -1.9% | [-2.6%, -1.2%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 160 ms | 162 ms | +1.4% | [-0.2%, +2.9%] | +0.0% | ~ | device 0.5x faster than the host; default/this 1.00x (was 1.03x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 164 ms | 159 ms | -3.1% | [-4.8%, -1.4%] | +0.0% | faster | device 0.6x faster than the host (was 0.5x); default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 162 ms | 161 ms | -0.6% | [-3.6%, +2.6%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.61 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.77 ms | 1.77 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.91x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.95 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.82x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.68 ms | -0.5% | [-1.1%, +0.2%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.96x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.24 ms | 7.21 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.30 ms | 7.30 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.24 ms | 7.29 ms | +0.6% | [+0.5%, +0.7%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.25 ms | 7.29 ms | +0.6% | [+0.4%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.42 ms | 8.45 ms | +0.3% | [-0.4%, +1.1%] | - | ~ | device 11.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.7 ms | +0.4% | [-1.2%, +2.1%] | - | ~ | device 7.1x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 167 ms | 169 ms | +1.0% | [+0.3%, +1.8%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 286 ms | 286 ms | +0.2% | [-1.6%, +2.1%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.3 ms | 24.4 ms | +0.4% | [-0.0%, +0.9%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.7 ms | 58.7 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 160 ms | 162 ms | +1.7% | [-1.3%, +4.8%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `MeshSDF/query-random/double/hostdevice` | 245 ms | 247 ms | +0.5% | [-0.4%, +1.4%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.91 ms | 2.96 ms | +1.5% | [-0.1%, +3.1%] | - | ~ | device 3.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.15 ms | 5.15 ms | -0.1% | [-0.8%, +0.5%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.5 ms | 14.4 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 122 ms | 123 ms | +0.8% | [-1.3%, +2.8%] | - | ~ | device 1.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 177 ms | 176 ms | -0.4% | [-2.0%, +1.2%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 376 ms | 376 ms | -0.0% | [-1.3%, +1.3%] | - | ~ | device 2.2x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.0 ms | 24.8 ms | -1.0% | [-2.1%, +0.2%] | - | ~ | device 2.0x faster than the host; tape/fused 4.94x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.0 ms | 27.0 ms | -0.2% | [-1.0%, +0.7%] | - | ~ | device 2.2x faster than the host; tape/fused 6.54x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.5 ms | 70.9 ms | -0.8% | [-2.7%, +1.0%] | - | ~ | device 4.1x faster than the host (was 4.5x); tape/fused 5.30x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.6 ms | 23.3 ms | +3.0% | [+1.0%, +5.0%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 45.0 ms | 44.8 ms | -0.5% | [-2.4%, +1.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 94.3 ms | 94.3 ms | -0.0% | [-1.8%, +1.8%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 553 ms | 556 ms | +0.5% | [-2.8%, +4.0%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 871 ms | 875 ms | +0.4% | [-1.4%, +2.3%] | - | ~ | device 0.4x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.18 s | 3.25 s | +2.2% | [-0.9%, +5.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 194 ms | 203 ms | +4.9% | [+2.8%, +7.1%] | - | minor | device 0.2x faster than the host (was 0.3x); tape/fused 2.74x (was 2.86x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 317 ms | 302 ms | -4.8% | [-7.1%, -2.3%] | - | faster | device 0.2x faster than the host (was 0.2x); tape/fused 2.90x (was 2.74x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 597 ms | 581 ms | -2.7% | [-6.5%, +1.3%] | - | ~ | device 0.5x faster than the host (was 0.5x); tape/fused 5.59x (was 5.32x) |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.65 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.32 ms | 5.33 ms | +0.1% | [+0.0%, +0.1%] | +0.0% | ~ | device 10.4x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 677 µs | 681 µs | +0.6% | [+0.6%, +0.6%] | - | ~ | device 11.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.82 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 996 µs | 997 µs | +0.1% | [-0.6%, +0.7%] | - | ~ | device 9.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.61 ms | 4.60 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.95 ms | 2.95 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.07 ms | 7.07 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.81 ms | 7.80 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.45 ms | 1.47 ms | +1.1% | [-0.5%, +2.8%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.88 ms | 3.90 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 526 µs | 525 µs | -0.1% | [-0.9%, +0.8%] | - | ~ | device 15.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.45 ms | 4.42 ms | -0.6% | [-1.2%, +0.1%] | - | ~ | device 6.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.35 ms | 4.42 ms | +1.6% | [+0.8%, +2.4%] | - | ~ | device 5.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.08 ms | 8.14 ms | +0.7% | [-1.8%, +3.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.7 ms | -0.5% | [-1.1%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.00 ms | 4.02 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.6 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.0 ms | 14.8 ms | -0.9% | [-1.6%, -0.3%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.59 ms | +0.5% | [-0.3%, +1.3%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.55 ms | 4.56 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 826 µs | 836 µs | +1.2% | [+0.4%, +2.0%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.86 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.15 ms | 4.13 ms | -0.6% | [-1.0%, -0.3%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.8% | [-2.2%, +0.7%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 21.0 ms | -0.5% | [-0.7%, -0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.92 ms | 8.94 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.5 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.2 ms | 20.2 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.77 ms | 2.74 ms | -1.1% | [-2.5%, +0.4%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.72 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.72 ms | 2.73 ms | +0.2% | [-0.4%, +0.9%] | - | ~ | device 7.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.46 ms | 6.50 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.8 ms | 33.9 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.1 ms | -0.4% | [-0.8%, +0.1%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.4 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.8 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.5 ms | 38.1 ms | -1.0% | [-1.4%, -0.7%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.7 ms | 12.8 ms | +0.8% | [+0.6%, +0.9%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.66 ms | 9.67 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.8 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.27 ms | 3.28 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.61 ms | 2.61 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ | device 3.4x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.57 ms | 4.57 ms | -0.1% | [-0.8%, +0.5%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.04 ms | 3.03 ms | -0.0% | [-0.9%, +0.8%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 671 µs | 670 µs | -0.2% | [-1.2%, +0.8%] | - | ~ | device 16.9x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 14.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 644 µs | 639 µs | -0.7% | [-1.1%, -0.3%] | - | ~ | device 17.4x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.22 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 608 µs | 606 µs | -0.3% | [-1.1%, +0.5%] | - | ~ | device 16.0x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.11 ms | 1.12 ms | +0.3% | [-0.1%, +0.8%] | - | ~ | device 14.8x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.48 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 11.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 608 µs | 590 µs | -3.0% | [-6.1%, +0.3%] | - | ~ | device 2.5x faster than the host; tape/fused 1.17x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 691 µs | 692 µs | +0.2% | [-9.8%, +11.3%] | - | ~ | device 2.7x faster than the host; tape/fused 1.21x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 691 µs | 690 µs | -0.2% | [-1.1%, +0.8%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 832 µs | 834 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 3.7x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 16.9 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 16.9 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.5 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 14.8 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.6 ms | 24.5 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.6 ms | +0.4% | [-0.8%, +1.6%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.02x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.4 ms | 24.6 ms | +0.6% | [+0.3%, +1.0%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.1 ms | +1.1% | [+1.0%, +1.3%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 23.1 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.1x faster than the host; tape/fused 1.09x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.61 ms | 2.60 ms | -0.5% | [-0.7%, -0.2%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.71 ms | 2.71 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.56 ms | 2.56 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.9x faster than the host; marched/traced 3.53x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.7x faster than the host; marched/traced 4.60x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.03 ms | 9.05 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.18 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 3.9x faster than the host; marched/traced 16.52x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.45 ms | 5.43 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 2.8x faster than the host; marched/traced 13.13x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 36.0 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.4 ms | 71.3 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.24 ms | 5.24 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.81 ms | 8.80 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.23 ms | 6.22 ms | -0.1% | [-0.8%, +0.7%] | - | ~ | device 1.3x faster than the host; marched/traced 25.88x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 0.8x faster than the host; marched/traced 23.92x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 158 ms | 161 ms | +1.7% | [+0.5%, +2.8%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 259 ms | 254 ms | -1.8% | [-2.7%, -0.8%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.84 ms | 4.84 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.8x faster than the host; marched/traced 23.02x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 1.3x faster than the host; marched/traced 18.02x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 112 ms | 112 ms | -0.7% | [-4.5%, +3.1%] | - | ~ | device 0.6x faster than the host (was 0.8x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 220 ms | 220 ms | +0.1% | [-3.5%, +3.9%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.5 ms | 19.4 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.9 ms | 26.7 ms | -0.5% | [-1.1%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 677 µs | 673 µs | -0.6% | [-2.0%, +0.7%] | - | ~ | device 17.1x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.2% | [-1.2%, +0.7%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 677 µs | 669 µs | -1.1% | [-2.2%, +0.0%] | - | ~ | device 17.2x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 691 µs | 688 µs | -0.4% | [-1.2%, +0.3%] | - | ~ | device 4.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 836 µs | 838 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 4.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 628 µs | 629 µs | +0.0% | [-0.2%, +0.3%] | - | ~ | device 4.3x faster than the host; hit/crossing 1.09x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 647 µs | 647 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 4.8x faster than the host; hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.71 ms | 4.71 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 10.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.18 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 10.1x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.11 ms | 9.10 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 266 ms | 263 ms | -1.2% | [-1.8%, -0.5%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 215 ms | 214 ms | -0.4% | [-3.2%, +2.4%] | - | ~ | device 0.9x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 55.3 ms | 55.6 ms | +0.5% | [+0.1%, +1.0%] | - | ~ | device 2.6x faster than the host (was 3.1x); hit/crossing 4.72x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 70.6 ms | 70.9 ms | +0.5% | [-0.5%, +1.5%] | - | ~ | device 2.4x faster than the host (was 2.2x); hit/crossing 3.02x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.5 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.9% | [+0.7%, +1.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.6 ms | 27.4 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.61 ms | 1.60 ms | -0.5% | [-0.7%, -0.2%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.73 ms | 2.72 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.29 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.35 ms | 1.35 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 2.6x faster than the host; hit/crossing 2.02x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.6 ms | 64.6 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 124 ms | 121 ms | -2.2% | [-3.1%, -1.4%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 59.2 ms | 59.3 ms | +0.1% | [-0.7%, +0.9%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.09x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 112 ms | 113 ms | +1.1% | [+0.1%, +2.1%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.07x (was 1.11x) |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 472 ms | 493 ms | +4.3% | [+1.8%, +6.9%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 706 ms | 719 ms | +1.8% | [+0.6%, +3.1%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 379 ms | 386 ms | +2.0% | [+0.2%, +3.7%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.28x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 535 ms | 532 ms | -0.4% | [-1.8%, +1.0%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.35x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.63 ms | 3.54 ms | -2.5% | [-3.0%, -2.0%] | - | ~ | device 1.2x faster than the host (was 1.1x); marched/traced 5.55x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.79 ms | 4.75 ms | -1.0% | [-1.6%, -0.3%] | - | ~ | device 0.9x faster than the host; marched/traced 8.46x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.9 ms | 19.7 ms | -1.2% | [-1.7%, -0.7%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.1 ms | 40.1 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.26 ms | +1.6% | [+1.6%, +1.7%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.84 ms | 1.85 ms | +0.6% | [+0.3%, +0.8%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.1 µs | 94.9 µs | -0.2% | [-2.0%, +1.6%] | - | ~ | device 15.9x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 151 µs | 155 µs | +2.3% | [-0.0%, +4.7%] | - | ~ | device 10.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 801 µs | 799 µs | -0.3% | [-0.5%, -0.2%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 935 µs | 935 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 36.5 ms | 34.5 ms | -5.4% | [-8.2%, -2.6%] | - | faster | device 0.1x faster than the host (was 0.1x); marched/traced 3.58x (was 3.42x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.3 ms | 36.7 ms | +1.0% | [+0.5%, +1.5%] | - | ~ | device 0.1x faster than the host; marched/traced 6.38x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 125 ms | 124 ms | -1.0% | [-2.7%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 235 ms | 234 ms | -0.6% | [-4.2%, +3.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.4 ms | 16.2 ms | -0.8% | [-1.2%, -0.3%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 22.0 ms | 21.7 ms | -1.5% | [-2.8%, -0.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.11 ms | 1.15 ms | +3.8% | [-3.4%, +11.4%] | - | ~ | device 1.3x faster than the host (was 1.3x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.33 ms | 2.37 ms | +1.6% | [+0.9%, +2.2%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 5.96 ms | -0.5% | [-1.1%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.71 ms | -0.2% | [-1.1%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.3 ms | 23.2 ms | -0.7% | [-1.5%, +0.1%] | - | ~ | device 2.2x faster than the host; marched/traced 4.25x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.2 ms | 28.3 ms | +0.5% | [+0.1%, +0.8%] | - | ~ | device 1.9x faster than the host; marched/traced 5.56x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 99.5 ms | 98.6 ms | -1.0% | [-2.2%, +0.3%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 155 ms | 157 ms | +1.4% | [+0.0%, +2.8%] | - | ~ | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.9 ms | 13.7 ms | -1.1% | [-2.2%, +0.0%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.4 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 310 ms | 301 ms | -2.9% | [-4.4%, -1.5%] | - | ~ | device 0.2x faster than the host; marched/traced 1.87x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 346 ms | 351 ms | +1.3% | [-2.9%, +5.8%] | - | ~ | device 0.2x faster than the host; marched/traced 2.85x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 569 ms | 564 ms | -0.9% | [-3.6%, +1.9%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 987 ms | 1.00 s | +1.5% | [+0.2%, +2.8%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 191 ms | 189 ms | -0.9% | [-1.8%, -0.0%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 234 ms | 233 ms | -0.3% | [-3.6%, +3.1%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.57 ms | 4.57 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.57 ms | 4.58 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.90 ms | 1.88 ms | -1.1% | [-1.5%, -0.7%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.97 ms | 7.99 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.94 ms | 7.94 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.7 ms | 23.1 ms | -2.3% | [-3.7%, -0.9%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | - | - | - | - | - | new |  |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | - | - | - | - | - | new |  |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 217 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 11.2x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 47.8 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 23.8x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 361 µs | 360 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.02 ms | 2.02 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.6 µs | 72.7 µs | +0.2% | [-0.1%, +0.6%] | - | ~ | device 7.2x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.7 µs | -0.3% | [-1.2%, +0.5%] | - | ~ | device 4.8x faster than the host (was 4.6x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 652 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.1% | [+0.1%, +0.1%] | - | ~ | device 11.9x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 474 µs | 474 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.2% | [-0.3%, +0.7%] | - | ~ | device 27.1x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 854 µs | 855 µs | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.9x faster than the host (was 3.0x) |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.71 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 191 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.9x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 164 µs | 163 µs | -0.5% | [-1.1%, +0.0%] | - | ~ | device 2.0x faster than the host (was 1.8x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |


The probes differ between the sides (each side ran its own): Performance/SphereCloudBVH/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt cuda -->
