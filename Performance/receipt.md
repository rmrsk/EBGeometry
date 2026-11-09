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
base: ef19317
base-fingerprint: 1af8bc3b9e684b83ed82fa8ad34ba45b57fd75e2
head: 2f6ff08
head-fingerprint: dbe9a8dcc095d10be7b06a98ea17abbc361cf01d
cases: 252
rounds: 2
reps: 3
date: 2026-11-08
-->

## Host comparison: yellow

**Accepted:** Accept as side effect of algorithm

`2f6ff08` against `ef19317` (origin/main), 2026-11-08.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 2 faster, 0 differ, 0 lost, 252 cases

- base: ef19317 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 2f6ff08 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 332 ms | +0.6% | [+0.5%, +0.8%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 177 ms | 178 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 513 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 346 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 603 ms | +7.3% | [-0.1%, +14.8%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.4 ms | 41.9 ms | +1.2% | [+0.1%, +2.3%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.3 ms | 75.3 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.2 ms | 56.2 ms | +0.1% | [-0.6%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.2 ms | 51.3 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.6 ms | 64.7 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.5 ms | 54.8 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.6 ms | 60.6 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.2 ms | 72.1 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 72.1 ms | 72.0 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.8 ms | 77.3 ms | -0.6% | [-0.8%, -0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.3 ms | 64.4 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.9 ms | 100 ms | +0.4% | [-0.3%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 77.1 ms | 77.0 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 108 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 63.9 ms | 63.9 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 95.4 ms | 95.8 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 76.1 ms | 76.3 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 101 ms | 100 ms | -0.6% | [-0.9%, -0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.3 ms | 19.5 ms | +0.6% | [-0.6%, +1.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.4 ms | 44.4 ms | +7.4% | [-0.9%, +15.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.3 ms | 72.5 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 19.9 ms | 20.2 ms | +1.5% | [+1.2%, +1.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.6 ms | 43.4 ms | -0.4% | [-0.8%, -0.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 80.7 ms | 81.5 ms | +0.9% | [+0.7%, +1.2%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 139 ms | 140 ms | +0.8% | [+0.1%, +1.4%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 75.5 ms | 77.4 ms | +2.5% | [-0.1%, +5.1%] | +0.0% | ~ | surface/leaf 1.81x |
| `ClosestSurface/union/closestSurface/float/host` | 145 ms | 146 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 76.5 ms | 76.4 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | surface/leaf 1.91x |
| `ClosestSurface/cloud/closestSurface/double/host` | 155 ms | 156 ms | +0.6% | [+0.4%, +0.7%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 80.4 ms | 80.4 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | surface/leaf 1.94x |
| `ClosestSurface/union/closestSurface/double/host` | 156 ms | 153 ms | -1.9% | [-2.4%, -1.4%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 80.9 ms | 80.7 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ | surface/leaf 1.89x |
| `Fused/csg/signedDistance/float/host` | 4.50 ms | 4.43 ms | -1.4% | [-1.7%, -1.0%] | - | ~ | tape/fused 2.91x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.4 ms | +0.8% | [-0.4%, +2.1%] | - | ~ | tape/fused 2.01x |
| `Fused/chain16/signedDistance/float/host` | 960 µs | 982 µs | +2.3% | [-1.6%, +6.2%] | - | ~ | tape/fused 16.62x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.2 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.8 ms | +1.0% | [+0.2%, +1.8%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.60 ms | 4.56 ms | -0.8% | [-2.2%, +0.5%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | tape/fused 2.06x |
| `Fused/chain16/signedDistance/double/host` | 3.91 ms | 3.91 ms | +0.1% | [-2.3%, +2.5%] | - | ~ | tape/fused 4.18x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.6 ms | 52.4 ms | -0.5% | [-1.0%, +0.0%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.4 ms | 52.7 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.1 ms | 82.1 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 228 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 114 ms | 113 ms | -0.5% | [-1.3%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 263 ms | 260 ms | -1.2% | [-2.3%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.4 ms | 70.4 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 214 ms | 214 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.6 ms | 96.5 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.1 ms | 30.4 ms | +1.2% | [-0.6%, +2.9%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 756 ms | 757 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 42.3 ms | 43.6 ms | +3.0% | [+2.4%, +3.6%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 867 ms | 853 ms | -1.6% | [-3.3%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 27.2 ms | 26.6 ms | -2.2% | [-4.8%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.0 ms | 26.8 ms | -0.9% | [-1.3%, -0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 322 ms | 319 ms | -0.8% | [-1.3%, -0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 91.6 ms | 91.0 ms | -0.7% | [-2.9%, +1.5%] | +0.0% | ~ | tape/fused 3.51x |
| `MeshCSG/smooth-fused/frame/float/host` | 100 ms | 99.6 ms | -0.6% | [-1.0%, -0.1%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.7 ms | 29.8 ms | +0.4% | [-0.5%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.0 ms | 30.0 ms | +0.2% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 356 ms | 359 ms | +0.9% | [-0.8%, +2.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 255 ms | 254 ms | -0.3% | [-1.0%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.0 ms | 96.9 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | tape/fused 3.71x |
| `MeshCSG/smooth-fused/frame/double/host` | 107 ms | 107 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ | tape/fused 2.38x |
| `MeshReconcile/armadillo/reconcile/float/host` | 131 ms | 131 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.4 ms | 41.3 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | reconcile/scheduled 3.16x |
| `MeshReconcile/armadillo/reconcile/double/host` | 153 ms | 154 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.4 ms | 57.1 ms | +1.2% | [-0.4%, +2.8%] | +0.0% | ~ | reconcile/scheduled 2.69x |
| `PointCloudBVH/uniform/build/float/host` | 53.7 ms | 53.6 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 144 ms | +0.3% | [-0.5%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 354 ms | 353 ms | -0.4% | [-1.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 79.4 ms | 79.2 ms | -0.2% | [-0.3%, -0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.65 ms | 6.64 ms | -0.2% | [-0.5%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 56.0 ms | 55.9 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 158 ms | 157 ms | -0.6% | [-2.2%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 385 ms | 385 ms | +0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.0 ms | 105 ms | +20.4% | [-0.3%, +41.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.88 ms | 6.91 ms | +0.4% | [-0.2%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 14.0 ms | 13.9 ms | -0.8% | [-6.1%, +4.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 119 ms | +0.6% | [+0.4%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 476 ms | 473 ms | -0.6% | [-0.7%, -0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 142 ms | 142 ms | +0.2% | [-0.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 800 ms | 801 ms | +0.1% | [+0.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.3 ms | 15.0 ms | -2.4% | [-5.6%, +1.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | +0.5% | [+0.4%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 494 ms | 494 ms | +0.0% | [-0.0%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 145 ms | 145 ms | +0.1% | [-0.3%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 791 ms | 799 ms | +1.0% | [+0.5%, +1.4%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.6 ms | 54.6 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 130 ms | 128 ms | -1.4% | [-3.7%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 260 ms | 260 ms | -0.2% | [-1.0%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 148 ms | 147 ms | -0.1% | [-0.2%, -0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 133 ms | 133 ms | +0.1% | [-0.1%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.4 ms | 57.5 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 138 ms | 139 ms | +0.6% | [-0.3%, +1.5%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 291 ms | 291 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 162 ms | 162 ms | -0.3% | [-0.9%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 148 ms | 148 ms | +0.1% | [-0.4%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.7 ms | 11.9 ms | +1.8% | [-1.7%, +5.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 115 ms | 115 ms | +0.4% | [+0.1%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 317 ms | 318 ms | +0.4% | [-0.1%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 155 ms | +0.2% | [+0.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 320 ms | 320 ms | +0.2% | [+0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.5 ms | 13.7 ms | +0.9% | [-2.3%, +3.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 117 ms | +1.4% | [+1.2%, +1.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 348 ms | 349 ms | +0.4% | [+0.3%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 166 ms | 166 ms | -0.1% | [-1.3%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 310 ms | +0.6% | [+0.4%, +0.8%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.3 ms | 35.2 ms | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.1 ms | 38.1 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.57 ms | 11.6 ms | +35.5% | [-6.3%, +79.6%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.80 ms | 9.66 ms | -1.5% | [-6.3%, +3.7%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.3 ms | 46.2 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.8 ms | 45.6 ms | -0.5% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.5 ms | 40.7 ms | +0.5% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.4 ms | 78.5 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.6 ms | 65.5 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.20 ms | 5.22 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | tape/fused 1.46x |
| `Ray/csg-tape/firstHit/float/host` | 7.62 ms | 7.61 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.4 ms | 52.4 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.5 ms | 51.6 ms | +0.2% | [-1.8%, +2.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.7 ms | 44.7 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.6 ms | 87.6 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 74.0 ms | 73.7 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.50 ms | 5.51 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | tape/fused 1.42x |
| `Ray/csg-tape/firstHit/double/host` | 7.85 ms | 7.84 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 22.6 ms | 18.8 ms | -17.0% | [-31.9%, +6.2%] | - | ~ | marched/traced 6.33x (was 5.27x) |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 119 ms | -0.2% | [-0.3%, -0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 21.9 ms | -0.8% | [-2.0%, +0.4%] | +0.0% | ~ | marched/traced 6.82x (was 6.54x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 144 ms | 149 ms | +3.6% | [+2.9%, +3.9%] | +0.0% | minor |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.3 ms | 35.2 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 18.6 ms | 18.6 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | marched/traced 6.80x (was 7.03x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 131 ms | 127 ms | -3.4% | [-7.4%, +0.9%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 23.5 ms | -0.2% | [-1.4%, +1.0%] | +0.0% | ~ | marched/traced 6.40x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 151 ms | 150 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.8 ms | 37.9 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.2 ms | 46.4 ms | +0.4% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.4 ms | 46.4 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.72 ms | 7.73 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.19 ms | 6.15 ms | -0.7% | [-1.3%, -0.2%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 176 ms | 177 ms | +0.3% | [+0.3%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 157 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.6 ms | 68.6 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.4 ms | 56.4 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.1 ms | 52.4 ms | +0.7% | [+0.2%, +1.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.1 ms | 52.5 ms | +0.8% | [+0.3%, +1.3%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.01 ms | 8.05 ms | +0.5% | [-0.7%, +1.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.26 ms | 6.24 ms | -0.3% | [-1.0%, +0.3%] | +0.0% | ~ | hit/crossing 1.29x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 201 ms | 200 ms | -0.4% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 177 ms | 178 ms | +0.5% | [+0.0%, +1.0%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 78.3 ms | 77.5 ms | -1.0% | [-1.5%, -0.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.4 ms | 64.6 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ | hit/crossing 1.20x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.2 ms | 11.0 ms | -1.4% | [-2.4%, -0.5%] | +0.0% | ~ | marched/traced 4.69x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.7 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.05 ms | 7.99 ms | -0.7% | [-2.1%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.92 ms | 1.91 ms | -0.4% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.95 ms | 6.88 ms | -1.1% | [-1.4%, -0.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.2 ms | 11.7 ms | -4.3% | [-8.8%, +0.7%] | +0.0% | ~ | marched/traced 5.16x (was 4.98x) |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.7 ms | 60.2 ms | -0.9% | [-1.6%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.57 ms | 8.62 ms | +0.6% | [+0.3%, +0.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.93 ms | 1.92 ms | -0.6% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.16 ms | 7.30 ms | +2.0% | [+0.3%, +3.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 238 ms | 238 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 327 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 236 ms | 236 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 250 ms | 255 ms | +1.8% | [+1.3%, +2.3%] | +0.0% | ~ | marched/traced 1.44x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 368 ms | 367 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 249 ms | 253 ms | +2.0% | [+1.7%, +2.3%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 19.0 ms | 19.0 ms | +0.2% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 19.2 ms | 19.0 ms | -0.9% | [-1.1%, -0.7%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.9 ms | 18.5 ms | -2.2% | [-4.8%, +0.6%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 21.6 ms | 21.6 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.8 ms | 21.8 ms | +0.3% | [-1.8%, +2.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.4 ms | 20.4 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 33.0 ms | 33.3 ms | +0.9% | [+0.5%, +1.4%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 4.95 ms | 4.99 ms | +0.8% | [+0.7%, +1.0%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.18 ms | 4.18 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.3 ms | 17.4 ms | +0.4% | [+0.3%, +0.5%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.37 ms | 4.31 ms | -1.4% | [-2.1%, -0.6%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.11 ms | 4.11 ms | +0.1% | [-0.8%, +1.0%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.22 ms | 2.23 ms | +0.1% | [-0.9%, +1.2%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.44 ms | 1.44 ms | +0.2% | [-0.2%, +0.6%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.05 ms | 2.06 ms | +0.5% | [-0.4%, +1.4%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 28.2 ms | 27.1 ms | -3.8% | [-7.1%, -0.3%] | - | faster |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 46.3 ms | 46.2 ms | -0.2% | [-0.5%, +0.2%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.37 ms | 5.35 ms | -0.2% | [-0.6%, +0.2%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.48 ms | 4.47 ms | -0.1% | [-0.5%, +0.4%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 22.4 ms | 22.6 ms | +0.9% | [+0.3%, +1.4%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.59 ms | 4.55 ms | -0.8% | [-2.4%, +0.9%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.29 ms | 5.22 ms | -1.3% | [-2.0%, -0.6%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.52 ms | 2.48 ms | -1.5% | [-2.8%, -0.2%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.78 ms | 1.79 ms | +1.0% | [+0.0%, +2.0%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.07 ms | 2.09 ms | +0.8% | [-0.2%, +1.8%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 29.6 ms | 29.7 ms | +0.3% | [-0.3%, +1.0%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.7 ms | 56.7 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.6 ms | 36.6 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.1 ms | 92.1 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.5 ms | 64.5 ms | -0.0% | [-0.9%, +0.9%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.8 ms | 42.9 ms | +0.4% | [-0.6%, +1.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 106 ms | 107 ms | +0.7% | [+0.2%, +1.2%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.7 ms | 74.8 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 297 ms | 297 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 296 ms | 295 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 87.9 ms | 86.3 ms | -1.8% | [-3.1%, -0.4%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 345 ms | 344 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 346 ms | 346 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 166 ms | 166 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 190 ms | 191 ms | +0.5% | [+0.0%, +1.0%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | - | - | - | - | - | new |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | - | - | - | - | - | new | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | - | - | - | - | - | new |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | - | - | - | - | - | new | surface/leaf 1.80x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | - | - | - | - | - | new |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | - | - | - | - | - | new | surface/hit 1.80x |
| `SurfaceParts/boxes/closestSurface/double/host` | - | - | - | - | - | new |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | - | - | - | - | - | new | surface/leaf 1.67x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | - | - | - | - | - | new |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | - | - | - | - | - | new | surface/leaf 1.76x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | - | - | - | - | - | new |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | - | - | - | - | - | new | surface/hit 1.77x |
| `Tape/csg/signedDistance/float/host` | 13.1 ms | 12.9 ms | -1.2% | [-1.8%, -0.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.59 ms | 4.58 ms | -0.1% | [-0.4%, +0.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.8 ms | 25.0 ms | +0.5% | [-0.5%, +1.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 39.9 ms | 39.9 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.80 ms | 1.80 ms | -0.2% | [-3.7%, +3.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 135 µs | 134 µs | -0.4% | [-6.5%, +3.9%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.9 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.69 ms | 5.68 ms | -0.1% | [-0.7%, +0.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.3 ms | +0.1% | [-0.1%, +0.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.63 ms | 4.66 ms | +0.5% | [-0.5%, +1.6%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.1 ms | +0.5% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.63 ms | 4.64 ms | +0.3% | [-0.9%, +1.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.1 ms | 26.3 ms | +0.7% | [+0.3%, +1.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 41.8 ms | 42.3 ms | +1.2% | [+0.6%, +1.8%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 2.84 ms | 1.79 ms | -36.9% | [-54.3%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 280 µs | 289 µs | +3.4% | [-3.3%, +12.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.8 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.74 ms | 7.74 ms | +0.0% | [-0.1%, +0.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.2 ms | 16.4 ms | +1.0% | [+0.5%, +1.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.75 ms | 7.73 ms | -0.2% | [-0.3%, -0.0%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.8 ms | 42.1 ms | +0.6% | [+0.0%, +1.2%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 217 ms | 204 ms | -6.0% | [-6.3%, -5.6%] | +0.0% | faster |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 42.5 ms | 42.6 ms | +0.3% | [-0.0%, +0.7%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 213 ms | 219 ms | +2.7% | [-0.0%, +5.5%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 494.7 s → 496.4 s (+0.3%), the largest peak 1284 MB → 1294 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 8.8 s | 8.9 s | +2.0% | 556 MB | 555 MB | -0.2% | ~ |
| `BVHRefit` | 3.8 s | 3.8 s | +0.1% | 458 MB | 469 MB | +2.5% | ~ |
| `BVHUnion` | 17.7 s | 17.7 s | -0.4% | 821 MB | 854 MB | +4.1% | ~ |
| `BVHUnionMixed` | 7.1 s | 7.1 s | +0.4% | 554 MB | 556 MB | +0.4% | ~ |
| `ClosestPrimitive` | 25.6 s | 25.7 s | +0.6% | 1221 MB | 1211 MB | -0.8% | ~ |
| `ClosestSurface` | 24.5 s | 24.9 s | +1.5% | 1123 MB | 1152 MB | +2.7% | ~ |
| `Fused` | 21.1 s | 21.1 s | +0.1% | 1019 MB | 1052 MB | +3.2% | ~ |
| `LaunchTuning` | 33.3 s | 33.0 s | -0.8% | 1123 MB | 1192 MB | +6.2% | ~ |
| `Mesh` | 13.9 s | 14.2 s | +2.1% | 773 MB | 775 MB | +0.3% | ~ |
| `MeshCSG` | 34.7 s | 34.8 s | +0.2% | 1169 MB | 1228 MB | +5.0% | ~ |
| `MeshReconcile` | 6.1 s | 6.3 s | +3.2% | 537 MB | 547 MB | +1.9% | ~ |
| `PointCloud` | 7.2 s | 7.0 s | -3.6% | 557 MB | 557 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 3.7 s | 3.5 s | -4.4% | 414 MB | 450 MB | +8.6% | ~ |
| `PointCloudHashGridBuild` | 3.1 s | 3.0 s | -5.1% | 402 MB | 403 MB | +0.2% | ~ |
| `Ray` | 33.0 s | 32.6 s | -1.3% | 1164 MB | 1233 MB | +6.0% | ~ |
| `RayCSG` | 29.3 s | 29.7 s | +1.5% | 982 MB | 1054 MB | +7.3% | ~ |
| `RayCrossing` | 41.1 s | 41.0 s | -0.2% | 1284 MB | 1232 MB | -4.0% | ~ |
| `RayParticles` | 44.8 s | 44.5 s | -0.7% | 1284 MB | 1294 MB | +0.7% | ~ |
| `RayShell` | 33.7 s | 34.5 s | +2.4% | 1076 MB | 1151 MB | +7.0% | ~ |
| `Render` | 32.2 s | 32.4 s | +0.6% | 1129 MB | 1143 MB | +1.3% | ~ |
| `Shapes` | 9.0 s | 9.0 s | -0.7% | 551 MB | 552 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.7 s | 4.7 s | -0.5% | 488 MB | 492 MB | +0.8% | ~ |
| `SphereCloudRays` | 25.4 s | 25.7 s | +1.2% | 978 MB | 982 MB | +0.4% | ~ |
| `SphereContacts` | 4.1 s | 4.1 s | +0.0% | 453 MB | 464 MB | +2.4% | ~ |
| `SurfaceParts` | - | 37.9 s | | - | 1326 MB | | new |
| `Tape` | 14.3 s | 14.6 s | +2.1% | 823 MB | 816 MB | -0.9% | ~ |
| `TapeBVHUnion` | 12.4 s | 12.6 s | +1.7% | 805 MB | 810 MB | +0.6% | ~ |
| **all, in both** | 494.7 s | 496.4 s | +0.3% | 1284 MB | 1294 MB | +0.7% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (5 cases): PointCloudHashGrid/sphere/build/double/host, RayCSG/scene-tape/firstHit-marched/float/host, Tape/sphere-handwritten/signedDistance/float/host, Tape/chain16/signedDistance/float/host, Tape/sphere-handwritten/signedDistance/double/host

The probes differ between the sides (each side ran its own): Performance/SurfaceParts/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: ef19317
base-fingerprint: 1af8bc3b9e684b83ed82fa8ad34ba45b57fd75e2
head: 2f6ff08
head-fingerprint: dbe9a8dcc095d10be7b06a98ea17abbc361cf01d
cases: 350
rounds: 2
reps: 3
date: 2026-11-08
-->

## Device comparison (CUDA): green

**Accepted:** -

`2f6ff08` against `ef19317` (origin/main), 2026-11-08.

**VERDICT: no regression (0 major, 0 minor across 350 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 350 cases

- base: ef19317 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 2f6ff08 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.59 ms | 1.61 ms | +0.9% | [+0.1%, +1.8%] | - | ~ | device 22.7x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.39 ms | 3.39 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 18.7x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.04 ms | 1.05 ms | +0.8% | [+0.4%, +1.3%] | - | ~ | device 19.6x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.30 ms | 2.30 ms | -0.3% | [-0.8%, +0.3%] | - | ~ | device 16.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.23 ms | 6.11 ms | -1.9% | [-2.1%, -1.7%] | - | ~ | device 7.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.8 ms | 11.8 ms | +0.6% | [-0.1%, +1.4%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.2 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.2 ms | 56.9 ms | -0.4% | [-1.1%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.5 ms | 20.5 ms | +0.3% | [-1.0%, +1.6%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.8 ms | 47.8 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.0 ms | 61.2 ms | +0.2% | [-1.0%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 107 ms | -3.6% | [-4.1%, -3.1%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.17 ms | 8.22 ms | +0.5% | [-1.5%, +2.6%] | +0.0% | ~ | device 4.9x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.77 ms | 9.82 ms | +0.4% | [-1.2%, +2.1%] | +0.0% | ~ | device 8.3x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 385 µs | 384 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 15.5x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 842 µs | 840 µs | -0.3% | [-0.5%, -0.2%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.51 ms | 4.53 ms | +0.5% | [+0.3%, +0.6%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.64 ms | 4.64 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 577 µs | 579 µs | +0.5% | [+0.2%, +0.7%] | - | ~ | device 13.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.37 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.78 ms | 6.77 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.03 ms | 7.01 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 904 µs | 900 µs | -0.4% | [-1.3%, +0.4%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.20 ms | 2.21 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.45 ms | 9.49 ms | +0.3% | [+0.0%, +0.7%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.6% | [+0.4%, +0.8%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 842 µs | 840 µs | -0.1% | [-0.4%, +0.1%] | - | ~ | device 13.6x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.03 ms | 2.03 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 8.1x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.10 ms | 9.10 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.82 ms | 9.84 ms | +0.2% | [+0.0%, +0.5%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 314 µs | 314 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 11.8x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 6.3x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 59.0 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.2% | [-0.2%, -0.2%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.39 ms | 5.39 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.6x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 517 ms | 517 ms | -0.0% | [-1.9%, +1.9%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.1 ms | 20.3 ms | +1.0% | [+0.9%, +1.0%] | - | ~ | device 4.1x faster than the host (was 3.3x) |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.34 ms | 8.34 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 5.3x faster than the host; surface/leaf 2.43x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.4 ms | 15.5 ms | +0.7% | [+0.6%, +0.7%] | - | ~ | device 8.0x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.19 ms | 6.18 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 10.0x faster than the host; surface/leaf 2.51x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 145 ms | 146 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.1 ms | 60.0 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.42x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 71.8 ms | 71.9 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.3x faster than the host; surface/leaf 2.57x |
| `Fused/csg/signedDistance/float/hostdevice` | 48.0 µs | 48.1 µs | +0.0% | [-0.4%, +0.4%] | - | ~ | device 23.7x faster than the host; tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.1 µs | 49.8 µs | -0.7% | [-0.9%, -0.4%] | - | ~ | device 44.2x faster than the host; tape/fused 7.19x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.9 µs | 48.1 µs | +0.4% | [+0.0%, +0.9%] | - | ~ | device 10.1x faster than the host (was 9.1x); tape/fused 9.90x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.4 ms | +0.2% | [-0.8%, +1.2%] | - | ~ | device 6.8x faster than the host; tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.1 ms | 31.2 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 4.4x faster than the host (was 3.8x) |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.56 ms | 1.55 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 284 ms | 285 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 286 ms | 289 ms | +0.8% | [-0.6%, +2.1%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.26 ms | 8.27 ms | +0.1% | [-1.6%, +1.8%] | +0.0% | ~ | device 10.7x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.31 ms | 8.34 ms | +0.3% | [-1.1%, +1.8%] | +0.0% | ~ | device 10.6x faster than the host (was 11.0x); default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.17 ms | 8.20 ms | +0.3% | [-0.5%, +1.0%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.24 ms | 8.28 ms | +0.5% | [-0.9%, +1.9%] | +0.0% | ~ | device 11.8x faster than the host (was 10.8x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 681 µs | 676 µs | -0.7% | [-0.9%, -0.6%] | +0.0% | ~ | device 3.9x faster than the host (was 4.0x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.75 ms | 1.75 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.03 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.66x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 812 µs | 811 µs | -0.1% | [-0.3%, -0.0%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.83x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 737 µs | 739 µs | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 824 µs | 827 µs | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 742 µs | 738 µs | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 737 µs | 732 µs | -0.7% | [-2.9%, +1.6%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 161 ms | +0.3% | [-2.7%, +3.5%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 158 ms | 160 ms | +1.3% | [-0.0%, +2.3%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 165 ms | 161 ms | -2.5% | [-4.3%, -0.7%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 160 ms | 164 ms | +3.0% | [-1.3%, +7.6%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.98x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.40 ms | 1.39 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.78 ms | 2.79 ms | +0.5% | [+0.1%, +0.8%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.94 ms | 1.94 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.72x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.56 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.25 ms | 7.23 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.27 ms | 7.28 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.28 ms | 7.23 ms | -0.8% | [-1.4%, -0.1%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.27 ms | 7.28 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.40 ms | 8.41 ms | +0.0% | [-1.4%, +1.5%] | - | ~ | device 11.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.5 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 9.6x faster than the host (was 7.1x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 168 ms | 168 ms | +0.1% | [-3.2%, +3.6%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 290 ms | 286 ms | -1.3% | [-2.5%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.4 ms | 24.6 ms | +0.8% | [+0.2%, +1.4%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.5 ms | 58.7 ms | +0.2% | [+0.2%, +0.2%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 155 ms | 156 ms | +0.2% | [-0.9%, +1.4%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 243 ms | 245 ms | +0.8% | [-0.7%, +2.3%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.52 ms | 2.54 ms | +1.0% | [+0.7%, +1.3%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.28 ms | 4.27 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 3.3x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.8 ms | 13.7 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 4.0x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.5 ms | 63.2 ms | -0.4% | [-1.1%, +0.2%] | - | ~ | device 3.2x faster than the host (was 3.5x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 93.4 ms | 91.5 ms | -2.1% | [-3.4%, -0.7%] | - | ~ | device 2.7x faster than the host (was 2.3x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 247 ms | 246 ms | -0.2% | [-1.7%, +1.3%] | - | ~ | device 3.5x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.0 ms | 24.6 ms | -1.5% | [-1.5%, -1.4%] | - | ~ | device 2.0x faster than the host; tape/fused 2.57x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.8 ms | 27.0 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 2.2x faster than the host; tape/fused 3.39x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.2 ms | 71.2 ms | +0.1% | [-1.3%, +1.4%] | - | ~ | device 4.5x faster than the host (was 4.1x); tape/fused 3.46x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.6 ms | 23.0 ms | +1.9% | [-4.1%, +8.3%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 43.9 ms | 44.2 ms | +0.7% | [+0.4%, +1.0%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 95.5 ms | 97.0 ms | +1.5% | [-2.9%, +6.0%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 542 ms | 540 ms | -0.3% | [-4.5%, +4.3%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 850 ms | 864 ms | +1.6% | [-0.8%, +2.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.02 s | 2.03 s | +0.8% | [-0.4%, +1.9%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 193 ms | 198 ms | +2.7% | [-3.8%, +9.6%] | - | ~ | device 0.2x faster than the host; tape/fused 2.72x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 305 ms | 308 ms | +0.9% | [-3.2%, +5.2%] | - | ~ | device 0.2x faster than the host; tape/fused 2.81x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 590 ms | 602 ms | +2.1% | [+0.9%, +3.4%] | - | ~ | device 0.5x faster than the host; tape/fused 3.37x |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.65 ms | 2.66 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.31 ms | 5.31 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 10.4x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 682 µs | 676 µs | -0.9% | [-1.1%, -0.6%] | - | ~ | device 11.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.82 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 995 µs | 995 µs | +0.0% | [-0.4%, +0.4%] | - | ~ | device 9.4x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.60 ms | 4.61 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.97 ms | 2.96 ms | -0.3% | [-0.9%, +0.4%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.08 ms | 7.09 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.82 ms | 7.79 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.45 ms | 1.46 ms | +0.8% | [+0.3%, +1.3%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.87 ms | 3.87 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 525 µs | 524 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 15.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.43 ms | 4.49 ms | +1.3% | [-0.1%, +2.6%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.33 ms | 4.34 ms | +0.3% | [-0.7%, +1.3%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.06 ms | 8.08 ms | +0.3% | [-0.6%, +1.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.7 ms | 15.8 ms | +1.0% | [+0.7%, +1.3%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.00 ms | -0.3% | [-0.7%, +0.0%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.7 ms | 17.7 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.7% | [-0.2%, +1.6%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.57 ms | 1.58 ms | +0.5% | [-0.3%, +1.3%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.60 ms | 4.60 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 833 µs | 832 µs | -0.2% | [-0.8%, +0.5%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.86 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.14 ms | 4.10 ms | -0.8% | [-1.1%, -0.6%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.6% | [-1.0%, -0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 21.1 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.93 ms | 8.94 ms | +0.1% | [-0.4%, +0.7%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.5 ms | -0.5% | [-0.7%, -0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.2 ms | 20.2 ms | -0.0% | [-0.7%, +0.7%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.75 ms | 2.76 ms | +0.3% | [-0.8%, +1.5%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.73 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.73 ms | 2.71 ms | -0.6% | [-1.0%, -0.2%] | - | ~ | device 7.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.38 ms | 6.44 ms | +0.8% | [-0.0%, +1.7%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.5 ms | 33.9 ms | +1.0% | [+0.5%, +1.6%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.3 ms | 68.5 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.5 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.7 ms | 20.8 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.0 ms | 38.2 ms | +0.6% | [-0.5%, +1.7%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.7 ms | 12.7 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.66 ms | 9.62 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.8 ms | +0.2% | [+0.2%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.28 ms | 3.26 ms | -0.7% | [-1.2%, -0.3%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.57 ms | 2.57 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.64 ms | 4.58 ms | -1.3% | [-2.6%, +0.1%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.04 ms | 3.03 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 679 µs | 678 µs | -0.2% | [-1.1%, +0.7%] | - | ~ | device 16.6x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.27 ms | 1.28 ms | +0.6% | [-0.1%, +1.3%] | - | ~ | device 14.8x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 644 µs | 650 µs | +1.0% | [-1.5%, +3.4%] | - | ~ | device 17.1x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 612 µs | 614 µs | +0.4% | [-1.3%, +2.0%] | - | ~ | device 15.8x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 14.8x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.19 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 12.0x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.8% | [+0.2%, +1.3%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 609 µs | 628 µs | +3.1% | [-0.1%, +6.4%] | - | ~ | device 2.4x faster than the host (was 2.5x); tape/fused 1.08x (was 1.11x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 688 µs | 689 µs | +0.1% | [-8.9%, +9.8%] | - | ~ | device 2.6x faster than the host (was 2.6x); tape/fused 1.22x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 676 µs | 675 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 3.8x faster than the host (was 3.7x) |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 843 µs | 843 µs | -0.1% | [-0.9%, +0.6%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 0.7x faster than the host; hand/api 0.99x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.5 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.1 ms | 14.9 ms | -1.5% | [-2.5%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.5 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.4 ms | 24.6 ms | +0.8% | [+0.6%, +1.0%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.1 ms | +0.7% | [+0.4%, +0.9%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | -0.3% | [-1.2%, +0.6%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.38 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.53 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 3.0x faster than the host (was 2.9x); marched/traced 3.56x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.00 ms | 2.99 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.8x faster than the host (was 2.7x); marched/traced 4.63x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.02 ms | 9.01 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 4.4x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.20 ms | 2.19 ms | -0.2% | [-0.3%, -0.2%] | - | ~ | device 3.9x faster than the host; marched/traced 13.26x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.48 ms | 5.48 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.8x faster than the host; marched/traced 10.02x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.0 ms | 29.1 ms | +0.3% | [-0.2%, +0.9%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.0 ms | 54.9 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 2.1x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.20 ms | 5.23 ms | +0.6% | [+0.5%, +0.6%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.76 ms | 8.76 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.15 ms | 6.18 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 1.2x faster than the host; marched/traced 25.99x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 0.8x faster than the host; marched/traced 23.94x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 161 ms | 161 ms | +0.1% | [-2.2%, +2.4%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 259 ms | 254 ms | -2.2% | [-3.3%, -1.1%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.87 ms | 4.88 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 1.8x faster than the host; marched/traced 23.01x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.3 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 1.3x faster than the host; marched/traced 17.47x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 110 ms | 112 ms | +2.1% | [-1.3%, +3.3%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 216 ms | 215 ms | -0.5% | [-2.1%, +1.2%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.3 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.8 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 675 µs | 674 µs | -0.1% | [-1.0%, +0.7%] | - | ~ | device 17.1x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 679 µs | 672 µs | -1.0% | [-2.7%, +0.7%] | - | ~ | device 17.2x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 15.1x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 727 µs | 730 µs | +0.4% | [+0.2%, +0.7%] | - | ~ | device 3.5x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 847 µs | 849 µs | +0.3% | [+0.2%, +0.4%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 627 µs | 626 µs | -0.2% | [-0.4%, +0.1%] | - | ~ | device 3.2x faster than the host; hit/crossing 1.17x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 650 µs | 647 µs | -0.6% | [-0.8%, -0.4%] | - | ~ | device 3.5x faster than the host; hit/crossing 1.31x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.69 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 6.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.17 ms | 4.18 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 9.9x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.08 ms | 9.08 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 55.8 ms | 55.4 ms | -0.6% | [-1.7%, +0.6%] | - | ~ | device 3.5x faster than the host (was 3.2x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.9 ms | 81.3 ms | -0.7% | [-1.6%, +0.2%] | - | ~ | device 2.5x faster than the host (was 2.7x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.0 ms | 48.2 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 3.3x faster than the host (was 3.0x); hit/crossing 1.15x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.7 ms | 63.2 ms | -0.8% | [-1.5%, +0.0%] | - | ~ | device 2.5x faster than the host (was 2.7x); hit/crossing 1.29x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.1% | [-1.0%, +1.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 27.5 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.40 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.0% | [-0.6%, +0.6%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.10 ms | -0.6% | [-0.6%, -0.6%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.27x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.15 ms | 1.16 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.7 ms | 64.5 ms | -0.4% | [-0.8%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 123 ms | 122 ms | -0.2% | [-1.6%, +1.1%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.6 ms | 58.4 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 112 ms | 111 ms | -0.5% | [-1.4%, +0.4%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 493 ms | 498 ms | +0.9% | [-9.9%, +13.2%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 708 ms | 709 ms | +0.0% | [-2.8%, +2.9%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 384 ms | 395 ms | +2.9% | [-7.9%, +15.1%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.26x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 523 ms | 531 ms | +1.6% | [-1.3%, +4.5%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.34x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.55 ms | 3.62 ms | +1.9% | [+0.2%, +3.7%] | - | ~ | device 1.2x faster than the host; marched/traced 3.32x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.77 ms | 4.80 ms | +0.7% | [+0.3%, +1.1%] | - | ~ | device 0.9x faster than the host; marched/traced 5.54x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | +0.6% | [+0.1%, +1.2%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.6 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.26 ms | +1.4% | [+0.4%, +2.4%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.86 ms | 1.85 ms | -0.7% | [-0.7%, -0.6%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.6 µs | 94.9 µs | -0.8% | [-2.4%, +0.9%] | - | ~ | device 15.2x faster than the host (was 15.9x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 152 µs | 153 µs | +0.4% | [-1.3%, +2.2%] | - | ~ | device 11.0x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 791 µs | 793 µs | +0.3% | [-0.2%, +0.8%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 927 µs | 929 µs | +0.2% | [+0.0%, +0.3%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.3 ms | 34.5 ms | -2.5% | [-8.1%, +3.7%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 3.55x (was 3.38x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 37.2 ms | 36.5 ms | -1.8% | [-2.2%, -1.4%] | - | ~ | device 0.1x faster than the host; marched/traced 6.37x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 119 ms | 122 ms | +2.7% | [+1.4%, +4.0%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 232 ms | 232 ms | -0.0% | [-3.6%, +3.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.2 ms | 16.3 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.7 ms | 21.8 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.04 ms | 1.04 ms | -0.7% | [-3.5%, +2.2%] | - | ~ | device 1.5x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.35 ms | 2.34 ms | -0.5% | [-1.7%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 5.98 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.68 ms | 6.71 ms | +0.4% | [-0.2%, +1.1%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.2 ms | 23.0 ms | -1.1% | [-2.6%, +0.4%] | - | ~ | device 2.2x faster than the host; marched/traced 2.73x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.2 ms | 28.3 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 2.0x faster than the host; marched/traced 3.69x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.3 ms | 62.6 ms | +0.5% | [+0.2%, +0.9%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 105 ms | 104 ms | -0.5% | [-1.6%, +0.6%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.7 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 3.7x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.4 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 299 ms | 308 ms | +2.8% | [-1.4%, +7.1%] | - | ~ | device 0.2x faster than the host; marched/traced 1.85x (was 1.91x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 349 ms | 340 ms | -2.5% | [-6.1%, +1.4%] | - | ~ | device 0.2x faster than the host; marched/traced 2.91x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 572 ms | 569 ms | -0.4% | [-1.5%, +0.6%] | - | ~ | device 0.5x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 996 ms | 989 ms | -0.8% | [-1.4%, -0.2%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 190 ms | 188 ms | -1.1% | [-4.4%, +2.3%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 241 ms | 238 ms | -1.4% | [-7.1%, +4.6%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.26 ms | 4.27 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 2.6x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.26 ms | 4.27 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 2.6x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.87 ms | -0.5% | [-1.1%, +0.2%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.66 ms | 7.65 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.6x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.65 ms | 7.63 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 1.6x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.3 ms | 23.2 ms | -0.6% | [-0.8%, -0.5%] | - | ~ | device 0.8x faster than the host |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 156 µs | 156 µs | +0.3% | [-0.4%, +0.9%] | - | ~ | device 32.9x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.4 µs | 47.5 µs | +0.1% | [-0.4%, +0.7%] | - | ~ | device 18.4x faster than the host (was 19.5x) |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | +0.2% | [-0.0%, +0.3%] | - | ~ | device 15.4x faster than the host |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 48.6 µs | 48.1 µs | -1.0% | [-1.9%, -0.0%] | - | ~ | device 65.6x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 47.6 µs | 47.8 µs | +0.3% | [-0.3%, +1.0%] | - | ~ | device 15.6x faster than the host |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 48.1 µs | 48.0 µs | -0.3% | [-0.6%, +0.1%] | - | ~ | device 17.0x faster than the host |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 48.0 µs | 47.8 µs | -0.4% | [-0.9%, +0.1%] | - | ~ | device 9.3x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.5 µs | 47.6 µs | +0.3% | [-0.1%, +0.6%] | - | ~ | device 7.4x faster than the host (was 7.1x) |
| `Shapes/wedge/signedDistance/float/hostdevice` | 47.6 µs | 47.4 µs | -0.4% | [-0.4%, -0.3%] | - | ~ | device 8.9x faster than the host (was 9.2x) |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 555 µs | 555 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 8.3x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.23 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 492 µs | 492 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.1x faster than the host (was 2.0x) |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 421 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 357 µs | 358 µs | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 669 µs | 669 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 414 µs | 414 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.3x faster than the host (was 1.3x) |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 339 µs | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.3x faster than the host (was 1.4x) |
| `Shapes/wedge/signedDistance/double/hostdevice` | 154 µs | 154 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.7x faster than the host |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.03 ms | 8.03 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 888 µs | 886 µs | -0.3% | [-1.1%, +0.6%] | +0.0% | ~ | device 10.4x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.65 ms | +0.5% | [+0.2%, +0.7%] | +0.0% | ~ | device 4.7x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.1 ms | 10.2 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.00 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 7.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.75 ms | 4.77 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 5.1x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.1 ms | 24.1 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.3 ms | 30.3 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.2 ms | 44.2 ms | +0.0% | [-0.8%, +0.9%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.8 ms | 45.6 ms | -0.6% | [-1.0%, -0.1%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 124 ms | 123 ms | -0.2% | [-4.8%, +4.6%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 127 ms | 128 ms | +1.6% | [-0.2%, +3.3%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.03x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 129 ms | 128 ms | -0.5% | [-3.6%, +2.8%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 133 ms | 132 ms | -0.7% | [-2.1%, +0.8%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 10.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.2 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 0.6x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | - | - | - | - | - | new |  |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | - | - | - | - | - | new | surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | - | - | - | - | - | new |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | - | - | - | - | - | new | surface/leaf 1.49x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | - | - | - | - | - | new |  |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | - | - | - | - | - | new | surface/hit 2.95x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | - | - | - | - | - | new |  |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | - | - | - | - | - | new | surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | - | - | - | - | - | new |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | - | - | - | - | - | new | surface/leaf 1.47x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | - | - | - | - | - | new |  |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | - | - | - | - | - | new | surface/hit 2.60x |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 218 µs | +0.3% | [+0.2%, +0.4%] | - | ~ | device 11.2x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.1% | [-0.4%, +0.3%] | - | ~ | device 24.1x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 358 µs | -0.4% | [-0.5%, -0.3%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 9.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.1 µs | 72.0 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 7.3x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.4 µs | 47.3 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 4.6x faster than the host (was 4.8x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 651 µs | -0.0% | [-0.1%, -0.0%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.2% | [+0.2%, +0.3%] | - | ~ | device 12.1x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 476 µs | 477 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.3 µs | 48.3 µs | -0.0% | [-0.3%, +0.2%] | - | ~ | device 26.9x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 858 µs | 858 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.64 ms | 6.64 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 192 µs | 192 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [+0.1%, +0.2%] | - | ~ | device 1.8x faster than the host (was 2.1x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.71 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 10.8x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.51 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 3692.3 s → 3783.2 s (+2.5%), the largest peak 5632 MB → 5633 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.1 s | 12.4 s | +2.5% | 540 MB | 547 MB | +1.5% | ~ |
| `BVHRefit` | 6.0 s | 6.2 s | +2.1% | 470 MB | 474 MB | +0.7% | ~ |
| `BVHUnion` | 23.7 s | 23.7 s | -0.0% | 963 MB | 971 MB | +0.8% | ~ |
| `BVHUnionMixed` | 10.2 s | 10.3 s | +1.3% | 563 MB | 571 MB | +1.5% | ~ |
| `ClosestPrimitive` | 127.3 s | 128.1 s | +0.6% | 1236 MB | 1242 MB | +0.4% | ~ |
| `ClosestSurface` | 182.0 s | 193.3 s | +6.2% | 1766 MB | 1861 MB | +5.4% | ~ |
| `Fused` | 49.6 s | 49.7 s | +0.2% | 1047 MB | 1051 MB | +0.4% | ~ |
| `LaunchTuning` | 506.0 s | 520.3 s | +2.8% | 3806 MB | 3807 MB | +0.0% | ~ |
| `Mesh` | 19.1 s | 19.6 s | +2.9% | 742 MB | 750 MB | +1.0% | ~ |
| `MeshCSG` | 354.2 s | 353.9 s | -0.1% | 5107 MB | 5102 MB | -0.1% | ~ |
| `MeshReconcile` | 8.9 s | 9.5 s | +5.8% | 470 MB | 474 MB | +0.7% | ~ |
| `PointCloud` | 11.6 s | 11.9 s | +1.8% | 507 MB | 516 MB | +1.7% | ~ |
| `PointCloudBVHBuild` | 6.3 s | 6.2 s | -2.5% | 470 MB | 474 MB | +0.8% | ~ |
| `PointCloudHashGridBuild` | 5.4 s | 5.6 s | +3.1% | 470 MB | 474 MB | +0.8% | ~ |
| `Ray` | 187.8 s | 185.8 s | -1.1% | 3703 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 218.2 s | 219.2 s | +0.4% | 4086 MB | 4086 MB | +0.0% | ~ |
| `RayCrossing` | 420.2 s | 431.5 s | +2.7% | 4019 MB | 4019 MB | +0.0% | ~ |
| `RayParticles` | 624.3 s | 657.4 s | +5.3% | 5632 MB | 5633 MB | +0.0% | ~ |
| `RayShell` | 379.3 s | 391.8 s | +3.3% | 5346 MB | 5346 MB | +0.0% | ~ |
| `Render` | 213.1 s | 214.3 s | +0.5% | 4941 MB | 4944 MB | +0.1% | ~ |
| `Shapes` | 27.2 s | 27.0 s | -0.6% | 528 MB | 536 MB | +1.6% | ~ |
| `SphereCloudBVH` | 8.1 s | 7.9 s | -1.9% | 471 MB | 474 MB | +0.8% | ~ |
| `SphereCloudRays` | 178.2 s | 184.6 s | +3.6% | 4096 MB | 4094 MB | -0.0% | ~ |
| `SphereContacts` | 7.3 s | 7.3 s | -0.7% | 470 MB | 474 MB | +0.7% | ~ |
| `SurfaceParts` | - | 437.7 s | | - | 4335 MB | | new |
| `Tape` | 63.5 s | 66.1 s | +4.1% | 870 MB | 876 MB | +0.7% | ~ |
| `TapeBVHUnion` | 42.6 s | 39.7 s | -6.7% | 811 MB | 819 MB | +0.9% | ~ |
| **all, in both** | 3692.3 s | 3783.2 s | +2.5% | 5632 MB | 5633 MB | +0.0% | ~ |

Device kernels: 166 in both, 8 changed (4 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 12 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, BVHUnion<double, 4ul, …` | 168 → 168 | 7904 → 7936 | 80/200 → 80/260 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, SphereCloudBVH<double,…` | 168 → 168 | 7968 → 8032 | 72/176 → 80/260 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, BVHUnion<float, 4ul, Sph…` | 168 → 168 | 5008 → 5008 | 88/160 → 96/256 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, SphereCloudBVH<float, 4u…` | 168 → 168 | 5056 → 5088 | 76/156 → 96/256 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19104 → 19120 | 1048/1072 → 1048/1072 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float, TriMeshSDF<float, 4ul, 4…` | 168 → 168 | 13088 → 13104 | 720/736 → 720/736 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 18720 → 18736 | 1052/1076 → 1052/1076 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float> >, unsigned int const, f…` | 168 → 168 | 13296 → 13312 | 728/740 → 728/740 | ~ |


Flagged after 2 rounds, and judged on 10 (4 cases): LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice, MeshCSG/smooth-tape/firstHit-morton/double/hostdevice, MeshCSG/smooth-tape/firstHit-random/double/hostdevice, RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice

The probes differ between the sides (each side ran its own): Performance/SurfaceParts/main.cpp

<!-- END receipt cuda -->
