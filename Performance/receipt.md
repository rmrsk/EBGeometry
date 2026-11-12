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
base: 5005dab
base-fingerprint: 1a1304a7e01f1c6dde66162828773f28c10c5885
head: ba7ccd8
head-fingerprint: 288e7982d874dba46dbf1425b20f6c00a5c00063
cases: 246
rounds: 2
reps: 3
date: 2026-11-11
-->

## Host comparison: yellow

**Accepted:** Accept them as noise.

`ba7ccd8` against `5005dab` (origin/main), 2026-11-11.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 3 faster, 0 differ, 0 lost, 246 cases

- base: 5005dab (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: ba7ccd8 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 332 ms | 332 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 177 ms | 177 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 514 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 346 ms | 347 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 604 ms | +7.4% | [-0.0%, +14.9%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.6 ms | 42.0 ms | +1.0% | [+0.1%, +1.9%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.4 ms | 75.3 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.9 ms | 55.9 ms | +0.0% | [+0.0%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.5 ms | 51.4 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.5 ms | 64.5 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.8 ms | 55.0 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.5 ms | 60.6 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.3 ms | 72.2 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 72.0 ms | 72.0 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.1 ms | 77.9 ms | +1.0% | [+0.3%, +1.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.6 ms | 64.2 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.7 ms | 99.8 ms | +0.0% | [-0.8%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 76.9 ms | 77.2 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 108 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 64.2 ms | 63.7 ms | -0.8% | [-1.1%, -0.4%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 95.9 ms | 95.5 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 75.9 ms | 75.8 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 101 ms | 101 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.4 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.1 ms | 41.1 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.0 ms | 73.0 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.1 ms | 20.4 ms | +1.5% | [+0.7%, +2.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.5 ms | 43.8 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.1 ms | 81.2 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 140 ms | 139 ms | -0.8% | [-1.4%, -0.3%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 77.5 ms | 76.3 ms | -1.5% | [-2.5%, -0.4%] | +0.0% | ~ | surface/leaf 1.82x |
| `ClosestSurface/union/closestSurface/float/host` | 147 ms | 147 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 76.8 ms | 76.9 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | surface/leaf 1.91x |
| `ClosestSurface/cloud/closestSurface/double/host` | 156 ms | 156 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 80.2 ms | 80.9 ms | +0.9% | [+0.5%, +1.3%] | +0.0% | ~ | surface/leaf 1.93x |
| `ClosestSurface/union/closestSurface/double/host` | 153 ms | 154 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 81.2 ms | 80.9 ms | -0.3% | [-0.4%, -0.3%] | +0.0% | ~ | surface/leaf 1.90x |
| `Domain/particles/labels/float/host` | - | - | - | - | - | new |  |
| `Domain/particles/labelAt/float/host` | - | - | - | - | - | new |  |
| `Domain/particles/surfaceGroups/float/host` | - | - | - | - | - | new |  |
| `Domain/particles/labels/double/host` | - | - | - | - | - | new |  |
| `Domain/particles/labelAt/double/host` | - | - | - | - | - | new |  |
| `Domain/particles/surfaceGroups/double/host` | - | - | - | - | - | new |  |
| `Fused/csg/signedDistance/float/host` | 4.47 ms | 4.48 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | tape/fused 2.87x |
| `Fused/smooth/signedDistance/float/host` | 12.2 ms | 12.3 ms | +0.7% | [-0.5%, +1.9%] | - | ~ | tape/fused 2.02x |
| `Fused/chain16/signedDistance/float/host` | 971 µs | 973 µs | +0.2% | [-0.2%, +0.5%] | - | ~ | tape/fused 16.75x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.1 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.4 ms | 46.3 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.62 ms | 4.58 ms | -0.9% | [-1.4%, -0.3%] | - | ~ | tape/fused 2.89x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | +0.4% | [-0.4%, +1.3%] | - | ~ | tape/fused 2.09x |
| `Fused/chain16/signedDistance/double/host` | 3.91 ms | 3.89 ms | -0.4% | [-2.1%, +1.4%] | - | ~ | tape/fused 4.21x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.6 ms | 52.9 ms | +0.5% | [-0.5%, +1.6%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.9 ms | 53.2 ms | +0.5% | [-0.4%, +1.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.9 ms | 82.3 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 229 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | +0.3% | [-0.0%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 261 ms | 261 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.2 ms | 70.2 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 213 ms | 213 ms | -0.1% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 97.0 ms | 96.9 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.7 ms | 29.6 ms | -3.3% | [-4.8%, -1.9%] | - | faster |  |
| `MeshSDF/query-random/float/host` | 755 ms | 757 ms | +0.2% | [-0.0%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | -0.3% | [-0.4%, -0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 42.9 ms | 42.3 ms | -1.3% | [-2.2%, -0.5%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 867 ms | 856 ms | -1.3% | [-3.4%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.5 ms | 26.6 ms | +0.4% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.5 ms | 27.2 ms | -1.2% | [-2.2%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 319 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 240 ms | 239 ms | -0.3% | [-1.1%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.3 ms | +0.1% | [+0.1%, +0.1%] | +0.0% | ~ | tape/fused 3.53x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.9 ms | 99.9 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.7 ms | 29.3 ms | -1.2% | [-2.3%, +0.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.1 ms | 29.8 ms | -1.0% | [-1.3%, -0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 357 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 255 ms | 254 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 96.8 ms | 96.9 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | tape/fused 3.68x |
| `MeshCSG/smooth-fused/frame/double/host` | 107 ms | 107 ms | -0.6% | [-1.3%, +0.0%] | +0.0% | ~ | tape/fused 2.39x |
| `MeshReconcile/armadillo/reconcile/float/host` | 131 ms | 131 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.2 ms | 41.3 ms | +0.3% | [-0.0%, +0.7%] | +0.0% | ~ | reconcile/scheduled 3.18x |
| `MeshReconcile/armadillo/reconcile/double/host` | 154 ms | 154 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.5 ms | 56.2 ms | -0.6% | [-0.9%, -0.3%] | +0.0% | ~ | reconcile/scheduled 2.74x |
| `PointCloudBVH/uniform/build/float/host` | 53.4 ms | 53.4 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 142 ms | -0.3% | [-0.5%, -0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 353 ms | 352 ms | -0.4% | [-1.0%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 79.5 ms | 79.7 ms | +0.3% | [-0.3%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.65 ms | 6.71 ms | +1.0% | [+0.3%, +1.7%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 56.0 ms | 56.1 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 155 ms | 154 ms | -0.7% | [-1.6%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 385 ms | 385 ms | -0.0% | [-0.5%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.7 ms | 86.7 ms | -0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.88 ms | 6.89 ms | +0.1% | [-0.8%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 17.0 ms | 13.7 ms | -19.7% | [-33.5%, +1.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | -0.0% | [-1.2%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 476 ms | 472 ms | -0.7% | [-0.8%, -0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 142 ms | 141 ms | -0.9% | [-1.0%, -0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 800 ms | 802 ms | +0.2% | [+0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.3 ms | 15.1 ms | -0.8% | [-3.1%, +1.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | +0.7% | [+0.5%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 493 ms | 493 ms | +0.0% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 145 ms | 144 ms | -0.5% | [-0.6%, -0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 791 ms | 800 ms | +1.2% | [+0.8%, +1.6%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.5 ms | 54.5 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 128 ms | 128 ms | -0.0% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 260 ms | 259 ms | -0.5% | [-0.8%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 148 ms | 148 ms | -0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 134 ms | 133 ms | -0.5% | [-1.1%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.2 ms | 57.3 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 138 ms | 139 ms | +0.4% | [-0.0%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 293 ms | 292 ms | -0.1% | [-0.9%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 162 ms | 162 ms | -0.2% | [-0.4%, -0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 148 ms | 148 ms | -0.1% | [-0.9%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 12.1 ms | 12.0 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 114 ms | 114 ms | +0.1% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 317 ms | 316 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 155 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 320 ms | 322 ms | +0.5% | [+0.4%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.5 ms | 13.5 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 115 ms | -0.1% | [-0.5%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 347 ms | 347 ms | -0.1% | [-0.2%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 165 ms | 164 ms | -0.4% | [-0.7%, -0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 310 ms | +0.2% | [+0.0%, +0.4%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.3 ms | 46.6 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.8 ms | 45.5 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.3 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 79.0 ms | 78.7 ms | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.5 ms | 65.6 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.19 ms | 5.28 ms | +1.6% | [+0.1%, +3.2%] | - | ~ | tape/fused 1.45x |
| `Ray/csg-tape/firstHit/float/host` | 7.73 ms | 7.64 ms | -1.2% | [-2.2%, -0.2%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.6 ms | 52.8 ms | +0.5% | [-0.4%, +1.3%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.2 ms | 51.4 ms | +0.4% | [-0.1%, +0.8%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.1 ms | 44.9 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.7 ms | 88.1 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 74.2 ms | 73.3 ms | -1.2% | [-2.3%, -0.2%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.49 ms | 5.54 ms | +0.9% | [+0.6%, +1.3%] | - | ~ | tape/fused 1.45x |
| `Ray/csg-tape/firstHit/double/host` | 7.87 ms | 8.01 ms | +1.8% | [+1.4%, +2.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.4 ms | 17.3 ms | -1.0% | [-1.3%, -0.7%] | - | ~ | marched/traced 6.89x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 119 ms | -0.9% | [-1.1%, -0.6%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 28.0 ms | +27.1% | [-0.1%, +54.2%] | +0.0% | ~ | marched/traced 5.18x (was 6.57x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 145 ms | 145 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.3 ms | 35.3 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.3 ms | 19.0 ms | -1.7% | [-2.4%, -1.1%] | - | ~ | marched/traced 6.70x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 127 ms | 127 ms | +0.5% | [+0.3%, +0.8%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.6 ms | 23.3 ms | -0.9% | [-1.4%, -0.4%] | +0.0% | ~ | marched/traced 6.42x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 150 ms | 150 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.9 ms | 37.4 ms | -1.4% | [-1.9%, -0.9%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.6 ms | 46.4 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.4 ms | 46.3 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.72 ms | 7.64 ms | -1.1% | [-1.9%, -0.4%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.10 ms | 6.14 ms | +0.7% | [-0.2%, +1.6%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 177 ms | 176 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 156 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.6 ms | 68.7 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 57.0 ms | 56.3 ms | -1.2% | [-2.4%, -0.0%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.0 ms | 52.5 ms | +0.9% | [+0.6%, +1.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.5 ms | 52.4 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.96 ms | 7.89 ms | -0.9% | [-1.8%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.23 ms | 6.22 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | hit/crossing 1.27x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 200 ms | 201 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 180 ms | 179 ms | -0.7% | [-1.1%, -0.3%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.7 ms | 77.3 ms | -0.5% | [-1.5%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.8 ms | 64.6 ms | -0.3% | [-1.7%, +1.2%] | +0.0% | ~ | hit/crossing 1.20x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.2 ms | 11.2 ms | +0.1% | [-1.1%, +1.4%] | +0.0% | ~ | marched/traced 4.68x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.6 ms | 52.4 ms | +1.4% | [+0.5%, +2.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.06 ms | 8.27 ms | +2.7% | [-0.5%, +5.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.93 ms | 1.92 ms | -0.6% | [-0.8%, -0.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.80 ms | 7.02 ms | +3.3% | [+2.9%, +4.1%] | +0.0% | minor |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.8 ms | 11.8 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ | marched/traced 5.10x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.1 ms | 60.1 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.63 ms | 8.60 ms | -0.3% | [-1.5%, +0.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.91 ms | 1.91 ms | -0.4% | [-1.0%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.24 ms | 7.21 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 239 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 329 ms | 327 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 237 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 255 ms | 254 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ | marched/traced 1.44x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 367 ms | 367 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 252 ms | 253 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.31 ms | 7.32 ms | +0.2% | [-0.6%, +0.9%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.6 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 264 ms | 265 ms | +0.3% | [+0.3%, +0.4%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 13.6 ms | 12.5 ms | -8.3% | [-8.8%, -7.8%] | +0.0% | faster |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.0 ms | 30.3 ms | +1.1% | [+0.5%, +1.6%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 21.8 ms | 21.9 ms | +0.5% | [-0.1%, +1.0%] | +0.0% | ~ | regions/hit 1.38x |
| `Regions/particles/region/double/host` | 7.63 ms | 7.65 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 13.0 ms | 12.9 ms | -0.7% | [-1.2%, -0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 283 ms | 282 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.9 ms | 13.0 ms | -6.6% | [-6.6%, -6.5%] | +0.0% | faster |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.2 ms | 32.0 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.7 ms | 23.5 ms | -0.7% | [-0.8%, -0.5%] | +0.0% | ~ | regions/hit 1.36x |
| `Render/csg/frame/float/host` | 19.4 ms | 19.6 ms | +1.0% | [+0.5%, +1.5%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 19.3 ms | 19.6 ms | +1.7% | [+1.6%, +1.8%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.2 ms | 18.3 ms | +0.4% | [+0.0%, +0.7%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 21.2 ms | 21.8 ms | +2.7% | [+1.6%, +3.5%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.2 ms | 21.8 ms | +2.7% | [+1.7%, +3.8%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.8 ms | 20.7 ms | -0.9% | [-1.6%, -0.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.8 ms | 56.7 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.7 ms | 36.7 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.3 ms | 92.1 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.6 ms | 64.4 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.8 ms | 42.9 ms | +0.1% | [-0.9%, +1.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 107 ms | 107 ms | +0.4% | [-0.5%, +1.2%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.8 ms | 74.7 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 296 ms | 294 ms | -0.5% | [-0.9%, -0.2%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 296 ms | 296 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 86.5 ms | 87.7 ms | +1.3% | [+0.6%, +2.1%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 346 ms | 345 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 346 ms | 347 ms | +0.2% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 166 ms | 166 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 190 ms | 192 ms | +0.6% | [+0.4%, +0.8%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 143 ms | 143 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 90.3 ms | 89.7 ms | -0.7% | [-1.3%, -0.1%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 131 ms | 131 ms | -0.3% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 73.3 ms | 73.5 ms | +0.1% | [-0.9%, +1.2%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 61.8 ms | 61.6 ms | -0.4% | [-1.3%, +0.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.2 ms | 34.2 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ | surface/hit 1.80x |
| `SurfaceParts/boxes/closestSurface/double/host` | 145 ms | 145 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 90.3 ms | 91.2 ms | +1.0% | [+0.8%, +1.2%] | +0.0% | ~ | surface/leaf 1.59x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 147 ms | 144 ms | -2.4% | [-4.8%, +0.0%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 80.8 ms | 80.7 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 68.2 ms | 68.1 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 38.8 ms | 38.9 ms | +0.3% | [+0.2%, +0.3%] | +0.0% | ~ | surface/hit 1.75x |
| `Tape/csg/signedDistance/float/host` | 13.2 ms | 12.9 ms | -2.7% | [-2.8%, -2.5%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.52 ms | 4.57 ms | +1.3% | [+1.0%, +1.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.3 ms | 24.9 ms | -1.6% | [-2.5%, -0.8%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.1 ms | 39.8 ms | -0.8% | [-1.1%, -0.5%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.76 ms | 1.76 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 129 µs | 134 µs | +4.0% | [-0.9%, +9.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.9 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.67 ms | 5.72 ms | +1.0% | [+0.2%, +1.7%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.0 ms | 16.3 ms | +1.7% | [+1.4%, +1.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.68 ms | 4.65 ms | -0.5% | [-1.0%, +0.1%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.2 ms | +0.8% | [-0.2%, +1.7%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.64 ms | 4.64 ms | +0.0% | [-2.0%, +2.1%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.5 ms | 26.6 ms | +0.6% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 41.1 ms | 42.0 ms | +2.2% | [+2.1%, +2.3%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.77 ms | 1.78 ms | +0.7% | [-1.0%, +2.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 295 µs | 288 µs | -2.5% | [-7.8%, +3.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 30.0 ms | +0.8% | [-0.2%, +1.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.83 ms | 7.74 ms | -1.2% | [-1.6%, -0.8%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.4 ms | 16.4 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.73 ms | 7.77 ms | +0.5% | [+0.0%, +1.0%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.9 ms | 43.3 ms | +3.4% | [-0.2%, +6.9%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 217 ms | 217 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 42.5 ms | 43.6 ms | +2.6% | [+2.1%, +3.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 215 ms | 213 ms | -0.8% | [-0.9%, -0.6%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 595.0 s → 597.3 s (+0.4%), the largest peak 1410 MB → 1406 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.2 s | +0.9% | 555 MB | 555 MB | +0.0% | ~ |
| `BVHRefit` | 3.9 s | 3.9 s | +0.0% | 481 MB | 482 MB | +0.1% | ~ |
| `BVHUnion` | 17.9 s | 17.8 s | -0.6% | 893 MB | 896 MB | +0.3% | ~ |
| `BVHUnionMixed` | 7.3 s | 7.3 s | -0.1% | 555 MB | 555 MB | +0.1% | ~ |
| `ClosestPrimitive` | 27.6 s | 27.7 s | +0.1% | 1250 MB | 1262 MB | +0.9% | ~ |
| `ClosestSurface` | 27.1 s | 27.0 s | -0.4% | 1221 MB | 1220 MB | -0.0% | ~ |
| `Domain` | - | 17.6 s | | - | 836 MB | | new |
| `Fused` | 23.7 s | 23.5 s | -0.9% | 1122 MB | 1126 MB | +0.3% | ~ |
| `LaunchTuning` | 35.0 s | 35.2 s | +0.4% | 1269 MB | 1271 MB | +0.2% | ~ |
| `Mesh` | 14.3 s | 14.5 s | +1.0% | 784 MB | 784 MB | +0.1% | ~ |
| `MeshCSG` | 37.2 s | 37.6 s | +1.0% | 1275 MB | 1281 MB | +0.5% | ~ |
| `MeshReconcile` | 6.3 s | 6.4 s | +0.8% | 563 MB | 564 MB | +0.2% | ~ |
| `PointCloud` | 7.1 s | 7.1 s | -0.6% | 557 MB | 557 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.6 s | 3.6 s | -0.1% | 467 MB | 468 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 3.1 s | 3.1 s | +1.0% | 450 MB | 451 MB | +0.2% | ~ |
| `Ray` | 34.3 s | 35.1 s | +2.4% | 1310 MB | 1316 MB | +0.5% | ~ |
| `RayCSG` | 31.1 s | 31.1 s | +0.0% | 1111 MB | 1113 MB | +0.2% | ~ |
| `RayCrossing` | 43.7 s | 44.5 s | +1.9% | 1340 MB | 1342 MB | +0.1% | ~ |
| `RayParticles` | 47.3 s | 47.0 s | -0.6% | 1314 MB | 1315 MB | +0.1% | ~ |
| `RayShell` | 35.6 s | 35.6 s | +0.1% | 1243 MB | 1241 MB | -0.2% | ~ |
| `Regions` | 28.0 s | 28.1 s | +0.3% | 1061 MB | 1065 MB | +0.4% | ~ |
| `Render` | 34.6 s | 34.7 s | +0.3% | 1204 MB | 1262 MB | +4.9% | ~ |
| `Shapes` | 9.2 s | 9.1 s | -0.4% | 554 MB | 554 MB | +0.0% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.8 s | +0.2% | 506 MB | 508 MB | +0.4% | ~ |
| `SphereCloudRays` | 27.7 s | 27.5 s | -0.5% | 1057 MB | 1060 MB | +0.3% | ~ |
| `SphereContacts` | 4.2 s | 4.3 s | +0.7% | 481 MB | 482 MB | +0.3% | ~ |
| `SurfaceParts` | 40.3 s | 40.3 s | +0.1% | 1410 MB | 1406 MB | -0.3% | ~ |
| `Tape` | 16.5 s | 16.6 s | +0.2% | 820 MB | 819 MB | -0.0% | ~ |
| `TapeBVHUnion` | 14.4 s | 14.8 s | +2.7% | 815 MB | 820 MB | +0.7% | ~ |
| **all, in both** | 595.0 s | 597.3 s | +0.4% | 1410 MB | 1406 MB | -0.3% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): RayParticles/box-shell-tape/step/float/host, Render/csg/frame/double/host

These probes compiled to the same code on both sides and were not run (24 cases): PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20)

The probes differ between the sides (each side ran its own): Performance/Domain/main.cpp, Performance/Regions/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 5005dab
base-fingerprint: 1a1304a7e01f1c6dde66162828773f28c10c5885
head: ba7ccd8
head-fingerprint: 288e7982d874dba46dbf1425b20f6c00a5c00063
cases: 340
rounds: 2
reps: 3
date: 2026-11-12
-->

## Device comparison (CUDA): yellow

**Accepted:** Accept them as noise.

`ba7ccd8` against `5005dab` (origin/main), 2026-11-12.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 5 faster, 0 differ, 0 lost, 340 cases

- base: 5005dab (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: ba7ccd8 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.61 ms | +0.4% | [-0.7%, +1.5%] | - | ~ | device 22.6x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.41 ms | 3.43 ms | +0.6% | [-0.5%, +1.6%] | - | ~ | device 18.7x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.04 ms | 1.05 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 19.4x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.30 ms | 2.32 ms | +0.8% | [+0.2%, +1.5%] | - | ~ | device 16.5x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.17 ms | 6.20 ms | +0.5% | [-0.7%, +1.8%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 12.0 ms | +1.3% | [-0.2%, +2.9%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.2 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 56.9 ms | 56.8 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.6 ms | 20.7 ms | +0.6% | [-1.0%, +2.2%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.0 ms | 47.8 ms | -0.4% | [-0.8%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.5 ms | 61.8 ms | +0.4% | [-1.9%, +2.8%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 109 ms | +0.4% | [-2.2%, +3.1%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.21 ms | 8.25 ms | +0.6% | [-1.8%, +3.0%] | +0.0% | ~ | device 4.9x faster than the host (was 5.4x) |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.78 ms | 9.92 ms | +1.5% | [-0.0%, +3.0%] | +0.0% | ~ | device 8.2x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 388 µs | 385 µs | -0.7% | [-1.6%, +0.2%] | - | ~ | device 15.4x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 847 µs | 847 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.51 ms | 4.53 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.66 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 586 µs | 588 µs | +0.3% | [-0.4%, +1.0%] | - | ~ | device 12.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 8.2x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.75 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.03 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 906 µs | 906 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.24 ms | 2.23 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.47 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 850 µs | 850 µs | +0.0% | [-0.3%, +0.4%] | - | ~ | device 13.5x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.07 ms | 2.07 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 7.9x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.15 ms | 9.09 ms | -0.7% | [-1.2%, -0.1%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.82 ms | 9.79 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 314 µs | 314 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 11.7x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 6.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.3x faster than the host (was 3.8x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.39 ms | 5.39 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 1.6x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 521 ms | 519 ms | -0.3% | [-1.4%, +0.8%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.1 ms | 20.1 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 5.4x faster than the host (was 4.9x) |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.34 ms | 8.33 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 4.7x faster than the host; surface/leaf 2.42x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.6 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 8.0x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.18 ms | 6.18 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 10.0x faster than the host; surface/leaf 2.52x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 146 ms | 149 ms | +1.6% | [+0.9%, +2.3%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.1 ms | 60.1 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.47x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 71.9 ms | 71.9 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.5x faster than the host (was 2.3x); surface/leaf 2.57x |
| `Domain/particles/labels/float/hostdevice` | - | - | - | - | - | new |  |
| `Domain/particles/labelAt/float/hostdevice` | - | - | - | - | - | new |  |
| `Domain/particles/surfaceGroups/float/hostdevice` | - | - | - | - | - | new |  |
| `Domain/particles/labels/double/hostdevice` | - | - | - | - | - | new |  |
| `Domain/particles/labelAt/double/hostdevice` | - | - | - | - | - | new |  |
| `Domain/particles/surfaceGroups/double/hostdevice` | - | - | - | - | - | new |  |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.1 µs | +0.3% | [-0.1%, +0.6%] | - | ~ | device 24.3x faster than the host; tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.8 µs | 49.9 µs | +0.1% | [-0.4%, +0.6%] | - | ~ | device 44.0x faster than the host; tape/fused 7.20x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.9 µs | 47.9 µs | -0.0% | [-0.4%, +0.3%] | - | ~ | device 9.5x faster than the host (was 9.0x); tape/fused 9.93x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.4 ms | 17.4 ms | +0.0% | [-0.9%, +0.9%] | - | ~ | device 6.7x faster than the host; tape/fused 1.79x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.1 ms | 31.1 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 286 ms | 278 ms | -2.7% | [-5.0%, -0.5%] | - | ~ | device 0.4x faster than the host (was 0.5x); tape/fused 1.02x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 283 ms | 283 ms | +0.1% | [-1.5%, +1.8%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.18 ms | 8.24 ms | +0.8% | [-0.6%, +2.2%] | +0.0% | ~ | device 10.7x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.23 ms | 8.26 ms | +0.3% | [-0.8%, +1.4%] | +0.0% | ~ | device 11.1x faster than the host (was 10.7x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.25 ms | 8.27 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ | device 10.7x faster than the host (was 11.1x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.30 ms | 8.26 ms | -0.5% | [-1.9%, +1.0%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 688 µs | 686 µs | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.73 ms | 1.73 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.04 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.66x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 779 µs | 779 µs | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 3.3x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 739 µs | 743 µs | +0.5% | [-0.2%, +1.1%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 821 µs | 818 µs | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.91x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 740 µs | 741 µs | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 732 µs | 723 µs | -1.3% | [-3.2%, +0.6%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 159 ms | 162 ms | +1.8% | [-1.8%, +5.7%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 159 ms | 157 ms | -1.0% | [-1.6%, -0.4%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 161 ms | 167 ms | +3.6% | [-1.4%, +8.8%] | +0.0% | ~ | device 0.5x faster than the host (was 0.6x); default/this 0.97x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 161 ms | 160 ms | -0.5% | [-1.3%, +0.3%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ | device 2.0x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.79 ms | 2.79 ms | -0.2% | [-0.9%, +0.6%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.94 ms | 1.94 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.72x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.56 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.24 ms | 7.25 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.27 ms | 7.29 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.25 ms | 7.28 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.30 ms | 7.33 ms | +0.4% | [+0.0%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.44 ms | 8.32 ms | -1.4% | [-2.0%, -0.9%] | - | ~ | device 11.4x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.8 ms | 16.7 ms | -0.4% | [-1.5%, +0.7%] | - | ~ | device 7.2x faster than the host (was 9.5x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 168 ms | 166 ms | -1.0% | [-1.6%, -0.5%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 285 ms | 280 ms | -1.8% | [-4.2%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.5 ms | 24.7 ms | +0.9% | [-0.8%, +2.7%] | - | ~ | device 3.9x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.4 ms | 58.6 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 2.5x faster than the host (was 2.6x) |
| `MeshSDF/query-morton/double/hostdevice` | 159 ms | 158 ms | -0.3% | [-1.7%, +1.1%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 243 ms | 240 ms | -1.2% | [-1.7%, -0.8%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | -0.2% | [-1.2%, +0.9%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.26 ms | 4.29 ms | +0.7% | [-0.0%, +1.4%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.7 ms | 13.7 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 4.0x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 62.5 ms | 63.2 ms | +1.1% | [+0.0%, +2.1%] | - | ~ | device 3.2x faster than the host (was 3.5x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 92.6 ms | 93.5 ms | +1.0% | [+0.3%, +1.7%] | - | ~ | device 2.5x faster than the host (was 2.3x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 251 ms | 251 ms | +0.1% | [-1.3%, +1.5%] | - | ~ | device 3.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.8 ms | 24.8 ms | -0.1% | [-1.9%, +1.8%] | - | ~ | device 2.0x faster than the host; tape/fused 2.55x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.8 ms | 26.9 ms | +0.2% | [-0.4%, +0.7%] | - | ~ | device 2.2x faster than the host; tape/fused 3.48x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.5 ms | 71.1 ms | -0.5% | [-2.0%, +1.0%] | - | ~ | device 4.1x faster than the host; tape/fused 3.53x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.4 ms | 22.2 ms | -0.9% | [-6.2%, +5.4%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.5 ms | 44.8 ms | +0.7% | [-1.2%, +2.6%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 92.8 ms | 92.2 ms | -0.6% | [-3.5%, +2.4%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 540 ms | 527 ms | -2.5% | [-7.4%, +2.6%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 841 ms | 845 ms | +0.4% | [-2.3%, +3.2%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.10 s | 2.02 s | -3.9% | [-6.2%, -1.6%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 194 ms | 200 ms | +3.1% | [-0.9%, +5.2%] | - | ~ | device 0.2x faster than the host; tape/fused 2.64x (was 2.79x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 313 ms | 297 ms | -5.0% | [-7.0%, -3.0%] | - | faster | device 0.2x faster than the host (was 0.2x); tape/fused 2.84x (was 2.69x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 597 ms | 579 ms | -3.0% | [-4.8%, -1.2%] | - | faster | device 0.5x faster than the host (was 0.5x); tape/fused 3.48x |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.67 ms | +0.3% | [+0.2%, +0.3%] | +0.0% | ~ | device 15.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.32 ms | 5.32 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 10.3x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 681 µs | 675 µs | -0.8% | [-0.9%, -0.7%] | - | ~ | device 11.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.83 ms | 1.83 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 996 µs | 993 µs | -0.3% | [-0.5%, -0.1%] | - | ~ | device 9.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.62 ms | 4.62 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 5.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.99 ms | 2.98 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.10 ms | 7.08 ms | -0.4% | [-0.8%, +0.0%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.78 ms | 7.78 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.4 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 12.0 ms | 12.0 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.49 ms | 1.47 ms | -1.0% | [-2.3%, +0.3%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.86 ms | 3.89 ms | +0.8% | [+0.0%, +1.5%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 525 µs | 523 µs | -0.3% | [-0.5%, -0.1%] | - | ~ | device 15.7x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.68 ms | 4.68 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.32 ms | 4.38 ms | +1.5% | [+0.5%, +2.6%] | - | ~ | device 5.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.13 ms | 8.14 ms | +0.1% | [-3.0%, +3.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.7 ms | -0.5% | [-1.5%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.02 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.5 ms | 17.6 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.8 ms | 14.9 ms | +0.8% | [-0.6%, +2.2%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.57 ms | -0.5% | [-0.9%, -0.0%] | - | ~ | device 10.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.60 ms | 4.61 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 836 µs | 834 µs | -0.3% | [-0.8%, +0.3%] | - | ~ | device 10.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.87 ms | 2.88 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.19 ms | 4.17 ms | -0.6% | [-0.7%, -0.5%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.7 ms | 16.5 ms | -0.9% | [-1.6%, -0.3%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.0 ms | 21.0 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.96 ms | 8.94 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.5 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.2 ms | 20.1 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.73 ms | 2.72 ms | -0.4% | [-0.8%, -0.1%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.74 ms | 6.72 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.73 ms | 2.73 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 7.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.49 ms | 6.47 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.8 ms | 33.8 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.5 ms | 68.4 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.5 ms | +0.9% | [+0.5%, +1.3%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.8 ms | -0.0% | [-1.0%, +0.9%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.1 ms | 38.0 ms | -0.3% | [-1.2%, +0.6%] | - | ~ | device 0.5x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 675 µs | 686 µs | +1.8% | [+1.4%, +2.2%] | - | ~ | device 16.5x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 14.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 643 µs | 643 µs | -0.1% | [-0.2%, +0.1%] | - | ~ | device 17.3x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.22 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 15.3x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 603 µs | 616 µs | +2.2% | [+0.0%, +4.5%] | - | ~ | device 15.8x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.11 ms | 1.12 ms | +0.8% | [+0.3%, +1.3%] | - | ~ | device 14.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.19 ms | +0.7% | [+0.5%, +0.9%] | - | ~ | device 12.0x faster than the host; triangles/polygons 0.58x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.50 ms | 2.49 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.51x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.08 ms | 1.07 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.19 ms | 2.19 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 9.7x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 612 µs | 613 µs | +0.1% | [-5.6%, +6.2%] | - | ~ | device 2.4x faster than the host; tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 728 µs | 731 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 2.5x faster than the host; tape/fused 1.15x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 682 µs | 686 µs | +0.6% | [+0.4%, +0.7%] | - | ~ | device 3.7x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 843 µs | 838 µs | -0.6% | [-1.2%, +0.0%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.3% | [-0.6%, +1.3%] | - | ~ | device 0.7x faster than the host; hand/api 1.01x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 17.1 ms | +1.1% | [+0.9%, +1.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.0% | [-0.7%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.4 ms | 24.4 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.4 ms | -0.6% | [-1.5%, +0.2%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.1 ms | 15.2 ms | +0.7% | [-0.3%, +1.7%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 22.9 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 1.3x faster than the host; tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.37 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 2.0x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.48 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.55 ms | 2.54 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 3.0x faster than the host (was 2.6x); marched/traced 3.54x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.99 ms | 2.99 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.7x faster than the host (was 2.4x); marched/traced 4.65x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 8.99 ms | 9.01 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 4.3x faster than the host (was 5.1x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 3.2x faster than the host (was 3.7x) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.19 ms | 2.19 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 3.9x faster than the host; marched/traced 13.24x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.51 ms | 5.50 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 2.8x faster than the host; marched/traced 10.05x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.1 ms | 29.0 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.2 ms | 55.3 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 2.3x faster than the host (was 2.0x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.22 ms | 5.22 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.76 ms | 8.76 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.19 ms | 6.23 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 1.3x faster than the host; marched/traced 25.59x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.5% | [-0.2%, +1.1%] | - | ~ | device 0.8x faster than the host; marched/traced 23.61x (was 24.45x) |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 161 ms | 159 ms | -0.7% | [-3.0%, +1.8%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 258 ms | 251 ms | -3.0% | [-4.6%, -1.3%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.88 ms | 4.86 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 22.76x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.3 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.3x faster than the host; marched/traced 17.56x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 110 ms | 111 ms | +0.7% | [-1.4%, +2.7%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 217 ms | 215 ms | -0.7% | [-3.0%, +1.6%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.3 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.8 ms | +0.5% | [+0.1%, +1.0%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 680 µs | 670 µs | -1.5% | [-2.4%, -0.5%] | - | ~ | device 17.2x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.2% | [-0.7%, +1.0%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 673 µs | 673 µs | +0.0% | [-1.2%, +1.2%] | - | ~ | device 17.3x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 684 µs | 687 µs | +0.4% | [+0.0%, +0.7%] | - | ~ | device 3.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 850 µs | 850 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 607 µs | 608 µs | +0.3% | [+0.0%, +0.6%] | - | ~ | device 3.2x faster than the host; hit/crossing 1.13x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 625 µs | 629 µs | +0.6% | [+0.3%, +0.9%] | - | ~ | device 3.8x faster than the host; hit/crossing 1.35x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.69 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 7.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.19 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 9.9x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.07 ms | 9.08 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 55.9 ms | 55.9 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 3.1x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.5 ms | 82.2 ms | +0.8% | [-0.3%, +1.9%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.4 ms | 48.6 ms | +0.4% | [-0.4%, +1.3%] | - | ~ | device 2.9x faster than the host (was 3.3x); hit/crossing 1.15x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.5 ms | 63.8 ms | +0.5% | [+0.3%, +0.7%] | - | ~ | device 3.0x faster than the host (was 2.4x); hit/crossing 1.29x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.4% | [-0.1%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.4 ms | 27.3 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.48 ms | 2.48 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.11 ms | 1.11 ms | -0.2% | [-0.9%, +0.4%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.25x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.13x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.7 ms | 64.3 ms | -0.5% | [-1.1%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 122 ms | 122 ms | -0.2% | [-2.2%, +1.8%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.6 ms | 58.5 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 111 ms | 112 ms | +0.4% | [-0.9%, +1.7%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.09x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 488 ms | 473 ms | -3.1% | [-7.4%, +1.4%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 713 ms | 705 ms | -1.1% | [-2.3%, +0.2%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 379 ms | 375 ms | -1.2% | [-5.9%, +3.7%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.26x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 539 ms | 528 ms | -2.1% | [-4.4%, +0.3%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.34x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.53 ms | 3.55 ms | +0.4% | [-0.8%, +1.7%] | - | ~ | device 1.2x faster than the host; marched/traced 3.38x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.77 ms | 4.78 ms | +0.1% | [-0.7%, +0.9%] | - | ~ | device 0.9x faster than the host; marched/traced 5.57x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.1 ms | 12.0 ms | -0.6% | [-1.8%, +0.6%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.6 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.26 ms | 1.26 ms | +0.6% | [-1.5%, +2.8%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.84 ms | 1.83 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 93.9 µs | 94.6 µs | +0.7% | [-2.1%, +3.6%] | - | ~ | device 15.7x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 151 µs | 156 µs | +3.2% | [-0.3%, +6.6%] | - | ~ | device 10.6x faster than the host (was 10.9x) |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 793 µs | 795 µs | +0.2% | [+0.1%, +0.4%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 930 µs | 930 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 36.1 ms | 34.9 ms | -3.2% | [-9.4%, +3.4%] | - | ~ | device 0.1x faster than the host; marched/traced 3.49x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 37.1 ms | 36.8 ms | -0.8% | [-2.1%, +0.5%] | - | ~ | device 0.1x faster than the host; marched/traced 6.38x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 123 ms | 122 ms | -0.6% | [-1.3%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 236 ms | 235 ms | -0.5% | [-2.3%, +1.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.6 ms | 16.6 ms | +0.5% | [-2.5%, +3.6%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.7 ms | 21.8 ms | +0.7% | [-0.2%, +1.6%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.07 ms | 1.01 ms | -5.3% | [-6.4%, -4.3%] | - | faster | device 1.5x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.35 ms | 2.34 ms | -0.4% | [-0.9%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 5.95 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.75 ms | 6.71 ms | -0.5% | [-1.1%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.0 ms | 23.0 ms | +0.3% | [-0.5%, +1.2%] | - | ~ | device 2.2x faster than the host; marched/traced 2.70x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.4 ms | 28.3 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 2.0x faster than the host; marched/traced 3.69x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.4 ms | 62.3 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 104 ms | 105 ms | +0.6% | [-0.7%, +1.8%] | - | ~ | device 2.3x faster than the host (was 2.6x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.7 ms | 13.7 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.6 ms | +1.3% | [+0.9%, +1.7%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 298 ms | 306 ms | +2.5% | [-0.5%, +4.6%] | - | ~ | device 0.2x faster than the host; marched/traced 1.82x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 359 ms | 351 ms | -2.1% | [-5.1%, +0.9%] | - | ~ | device 0.2x faster than the host; marched/traced 2.81x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 553 ms | 555 ms | +0.3% | [-0.3%, +1.0%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 990 ms | 986 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 0.2x faster than the host (was 0.4x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 194 ms | 187 ms | -3.6% | [-4.8%, -2.3%] | - | faster | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 239 ms | 233 ms | -2.5% | [-4.6%, -0.4%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `Regions/particles/region/float/hostdevice` | 2.74 ms | 2.74 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 2.8x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.72 ms | 3.72 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 3.5x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 100 ms | 101 ms | +0.3% | [-0.9%, +1.5%] | - | ~ | device 3.8x faster than the host (was 3.5x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.65 ms | 3.70 ms | +1.4% | [+0.6%, +2.2%] | - | ~ | device 3.6x faster than the host (was 4.0x) |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.53 ms | 6.53 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.8x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.42 ms | 5.42 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.3x faster than the host; regions/hit 1.20x |
| `Regions/particles/region/double/hostdevice` | 3.39 ms | 3.41 ms | +0.6% | [-0.3%, +1.5%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.91 ms | 5.90 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 133 ms | 134 ms | +1.1% | [+0.4%, +1.8%] | - | ~ | device 2.2x faster than the host; each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.73 ms | 6.00 ms | +4.9% | [+4.1%, +5.2%] | - | minor | device 2.3x faster than the host (was 2.5x) |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.7 ms | 15.7 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 13.0 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 1.3x faster than the host; regions/hit 1.21x |
| `Render/csg/frame/float/hostdevice` | 3.99 ms | 3.98 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 2.8x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 3.99 ms | 3.99 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.8x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.98 ms | 1.98 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 8.5x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.67 ms | 7.68 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.6x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.65 ms | 7.65 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.2 ms | 23.4 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 888 µs | 888 µs | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ | device 10.4x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.65 ms | 2.64 ms | -0.3% | [-0.7%, +0.2%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 2.99 ms | 3.01 ms | +0.7% | [+0.1%, +1.2%] | +0.0% | ~ | device 7.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.71 ms | 4.71 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 6.2x faster than the host (was 5.1x); tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.0 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | -0.1% | [-0.1%, -0.1%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.3 ms | 30.3 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.2 ms | 44.3 ms | +0.1% | [-0.5%, +0.8%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.5 ms | 45.6 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 121 ms | 124 ms | +1.7% | [-1.2%, +3.8%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.05x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 128 ms | 124 ms | -2.4% | [-2.7%, -2.1%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.08x (was 1.04x) |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 127 ms | 130 ms | +2.0% | [+0.6%, +3.3%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 133 ms | 134 ms | +1.0% | [-0.7%, +2.7%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 10.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.1 ms | -0.1% | [-0.3%, -0.0%] | +0.0% | ~ | device 0.6x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.33 ms | 9.32 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 11.4x faster than the host |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.93 ms | 5.93 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 9.9x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 96.2 ms | 97.3 ms | +1.1% | [-0.7%, +3.0%] | - | ~ | device 3.5x faster than the host (was 4.1x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.3 ms | 63.3 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.6x faster than the host (was 3.3x); surface/leaf 1.54x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.2 ms | 29.1 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 5.0x faster than the host (was 5.5x) |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.90 ms | 9.90 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 6.9x faster than the host; surface/hit 2.94x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.1 ms | 59.0 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.5x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 782 ms | 783 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 0.6x faster than the host (was 0.4x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 530 ms | 530 ms | -0.2% | [-1.0%, +0.7%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.48x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 313 ms | 309 ms | -1.3% | [-1.9%, -0.7%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 118 ms | 121 ms | +2.4% | [+0.8%, +4.1%] | - | ~ | device 0.6x faster than the host; surface/hit 2.56x (was 2.66x) |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 218 µs | +0.1% | [-0.1%, +0.2%] | - | ~ | device 11.2x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 24.1x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 359 µs | -0.4% | [-0.9%, +0.2%] | - | ~ | device 12.8x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 9.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.1 µs | 71.9 µs | -0.2% | [-0.7%, +0.2%] | - | ~ | device 7.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 48.1 µs | +0.7% | [-0.4%, +1.7%] | - | ~ | device 4.6x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 653 µs | 651 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.1% | [+0.0%, +0.1%] | - | ~ | device 12.2x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 476 µs | 475 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.2% | [-0.4%, +0.8%] | - | ~ | device 27.2x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 859 µs | 858 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.65 ms | 6.64 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 192 µs | 192 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 2.0x faster than the host (was 1.9x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.11 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 0.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.70 ms | 3.71 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 10.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.50 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.5x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 4 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 3981.9 s → 3979.5 s (-0.1%), the largest peak 5632 MB → 5631 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 11.8 s | 11.9 s | +0.1% | 590 MB | 594 MB | +0.6% | ~ |
| `BVHRefit` | 6.1 s | 6.1 s | +0.2% | 502 MB | 503 MB | +0.2% | ~ |
| `BVHUnion` | 22.0 s | 22.2 s | +0.7% | 1017 MB | 1020 MB | +0.4% | ~ |
| `BVHUnionMixed` | 9.9 s | 10.0 s | +0.4% | 615 MB | 619 MB | +0.6% | ~ |
| `ClosestPrimitive` | 110.8 s | 109.5 s | -1.1% | 1339 MB | 1343 MB | +0.3% | ~ |
| `ClosestSurface` | 159.1 s | 156.7 s | -1.5% | 1844 MB | 1844 MB | +0.0% | ~ |
| `Domain` | - | 52.1 s | | - | 990 MB | | new |
| `Fused` | 47.1 s | 49.2 s | +4.5% | 1149 MB | 1154 MB | +0.4% | ~ |
| `LaunchTuning` | 448.8 s | 451.6 s | +0.6% | 3806 MB | 3806 MB | -0.0% | ~ |
| `Mesh` | 19.2 s | 18.7 s | -2.2% | 793 MB | 797 MB | +0.5% | ~ |
| `MeshCSG` | 312.1 s | 312.1 s | +0.0% | 5161 MB | 5160 MB | -0.0% | ~ |
| `MeshReconcile` | 9.4 s | 9.2 s | -2.1% | 503 MB | 508 MB | +1.0% | ~ |
| `PointCloud` | 11.5 s | 11.5 s | -0.4% | 561 MB | 565 MB | +0.8% | ~ |
| `PointCloudBVHBuild` | 6.2 s | 6.2 s | +0.8% | 502 MB | 503 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 5.6 s | 5.6 s | -0.6% | 501 MB | 503 MB | +0.3% | ~ |
| `Ray` | 162.1 s | 162.6 s | +0.3% | 3702 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 188.1 s | 187.8 s | -0.1% | 4085 MB | 4085 MB | -0.0% | ~ |
| `RayCrossing` | 356.9 s | 358.2 s | +0.4% | 4017 MB | 4017 MB | +0.0% | ~ |
| `RayParticles` | 602.0 s | 598.8 s | -0.5% | 5632 MB | 5631 MB | -0.0% | ~ |
| `RayShell` | 333.2 s | 336.1 s | +0.9% | 5345 MB | 5345 MB | -0.0% | ~ |
| `Regions` | 297.5 s | 301.8 s | +1.5% | 3879 MB | 3879 MB | +0.0% | ~ |
| `Render` | 191.0 s | 192.1 s | +0.6% | 5216 MB | 5216 MB | +0.0% | ~ |
| `Shapes` | 23.0 s | 23.2 s | +1.2% | 581 MB | 586 MB | +0.9% | ~ |
| `SphereCloudBVH` | 7.2 s | 7.5 s | +4.1% | 502 MB | 503 MB | +0.3% | ~ |
| `SphereCloudRays` | 160.9 s | 159.4 s | -0.9% | 4095 MB | 4093 MB | -0.0% | ~ |
| `SphereContacts` | 6.9 s | 7.4 s | +7.1% | 501 MB | 503 MB | +0.3% | ~ |
| `SurfaceParts` | 381.1 s | 370.4 s | -2.8% | 4241 MB | 4242 MB | +0.0% | ~ |
| `Tape` | 57.2 s | 57.4 s | +0.3% | 973 MB | 977 MB | +0.4% | ~ |
| `TapeBVHUnion` | 35.4 s | 36.2 s | +2.2% | 913 MB | 917 MB | +0.4% | ~ |
| **all, in both** | 3981.9 s | 3979.5 s | -0.1% | 5632 MB | 5631 MB | -0.0% | ~ |

Device kernels: 190 in both, 2 changed (2 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 4 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)3>, Vec3T<double> const,…` | 168 → 168 | 4432 → 4208 | 108/68 → 228/272 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)3>, Vec3T<float> const, R…` | 168 → 168 | 3056 → 2944 | 96/60 → 180/232 | **spills** |


Flagged after 2 rounds, and judged on 10 (5 cases): MeshCSG/union-tape/firstHit-morton/double/hostdevice, MeshCSG/smooth-fused/firstHit-morton/double/hostdevice, RayShell/armadillo-tape/firstHit-morton/double/hostdevice, Regions/particles/domainLabels/double/hostdevice, SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice

These probes compiled to the same code on both sides and were not run (28 cases): PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Shapes (20)

The probes differ between the sides (each side ran its own): Performance/Domain/main.cpp, Performance/Regions/main.cpp

<!-- END receipt cuda -->
