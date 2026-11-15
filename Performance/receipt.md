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
base: 569fff8
base-fingerprint: eb69a7e3bccddc2ac0d8879ed8618df6c8084558
head: 897a647
head-fingerprint: f7653fee8c5b2beba423cac303718dc728ad6b3e
cases: 270
rounds: 2
reps: 3
date: 2026-11-15
-->

## Host comparison: yellow

**Accepted:** Side effect of improved rendering.

`897a647` against `569fff8` (origin/main), 2026-11-15.

**VERDICT: 6 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 6 minor, 5 faster, 0 differ, 0 lost, 270 cases

- base: 569fff8 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 897a647 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 332 ms | 329 ms | -0.7% | [-0.8%, -0.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 177 ms | 177 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 514 ms | 513 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 346 ms | 346 ms | -0.0% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 561 ms | 561 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.0 ms | 41.3 ms | +0.8% | [+0.5%, +1.0%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 74.6 ms | 74.7 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.8 ms | 55.8 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.4 ms | 51.3 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.2 ms | 64.2 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.3 ms | 54.7 ms | +0.7% | [+0.6%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.4 ms | 60.2 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.2 ms | 72.1 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 71.8 ms | 71.6 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.3 ms | 77.2 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.3 ms | 64.1 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.1 ms | 99.7 ms | +0.7% | [+0.1%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 76.5 ms | 76.8 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 107 ms | -1.5% | [-2.8%, -0.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 63.8 ms | 63.9 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 95.5 ms | 95.4 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 75.7 ms | 75.9 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 100 ms | 101 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.5 ms | 19.5 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.0 ms | 41.1 ms | +0.3% | [-0.5%, +1.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.8 ms | 72.8 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.5 ms | 20.1 ms | -2.0% | [-2.4%, -1.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.6 ms | 46.4 ms | +6.3% | [-0.3%, +12.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 80.6 ms | 80.9 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 140 ms | 139 ms | -0.9% | [-1.0%, -0.9%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 76.3 ms | 77.2 ms | +1.1% | [+0.0%, +2.3%] | +0.0% | ~ | surface/leaf 1.80x |
| `ClosestSurface/union/closestSurface/float/host` | 146 ms | 146 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 76.4 ms | 76.5 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | surface/leaf 1.90x |
| `ClosestSurface/cloud/closestSurface/double/host` | 156 ms | 156 ms | +0.4% | [+0.3%, +0.4%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 80.3 ms | 80.4 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ | surface/leaf 1.94x |
| `ClosestSurface/union/closestSurface/double/host` | 153 ms | 153 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 80.4 ms | 80.5 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ | surface/leaf 1.90x |
| `Domain/particles/labels/float/host` | 13.5 ms | 13.5 ms | -0.0% | [-0.7%, +0.6%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.77 ms | 6.76 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.38 ms | 1.36 ms | -1.5% | [-3.2%, +0.2%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.2 ms | 14.2 ms | -0.6% | [-1.4%, +0.3%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.14 ms | 7.11 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.36 ms | 1.37 ms | +0.8% | [+0.3%, +1.3%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.47 ms | 4.46 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | tape/fused 2.91x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.4 ms | +0.5% | [-0.3%, +1.3%] | - | ~ | tape/fused 2.02x |
| `Fused/chain16/signedDistance/float/host` | 985 µs | 960 µs | -2.6% | [-3.7%, -1.5%] | - | ~ | tape/fused 16.79x |
| `Fused/drilled-mesh/signedDistance/float/host` | 45.9 ms | 46.0 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.1 ms | 46.1 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.61 ms | 4.59 ms | -0.4% | [-1.2%, +0.4%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.8 ms | +0.1% | [-0.6%, +0.9%] | - | ~ | tape/fused 2.09x |
| `Fused/chain16/signedDistance/double/host` | 3.87 ms | 3.86 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | tape/fused 4.19x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.0 ms | 52.4 ms | +0.8% | [+0.5%, +1.0%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.4 ms | 52.6 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.7 ms | 82.4 ms | +0.8% | [+0.7%, +0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 228 ms | 228 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 258 ms | 261 ms | +1.3% | [+0.2%, +2.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.5 ms | 70.7 ms | +0.2% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 213 ms | 213 ms | +0.0% | [-0.8%, +0.8%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.6 ms | 96.7 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.6 ms | 28.6 ms | -3.5% | [-4.0%, -3.0%] | - | faster |  |
| `MeshSDF/query-random/float/host` | 752 ms | 752 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 118 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 42.0 ms | 41.4 ms | -1.6% | [-2.2%, -1.0%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 852 ms | 843 ms | -1.0% | [-1.9%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.9 ms | 26.5 ms | -1.5% | [-2.4%, -0.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.9 ms | 26.6 ms | -1.2% | [-2.1%, -0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 317 ms | -0.8% | [-1.0%, -0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 238 ms | 237 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.3 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ | tape/fused 3.51x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.6 ms | 101 ms | +1.4% | [+1.2%, +1.6%] | +0.0% | ~ | tape/fused 2.35x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.3 ms | 29.6 ms | +0.9% | [+0.1%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.0 ms | 30.0 ms | +0.1% | [-1.0%, +1.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 353 ms | 353 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 253 ms | 254 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 96.7 ms | 96.7 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | tape/fused 3.64x |
| `MeshCSG/smooth-fused/frame/double/host` | 107 ms | 109 ms | +2.3% | [+2.1%, +2.5%] | +0.0% | ~ | tape/fused 2.33x |
| `MeshReconcile/armadillo/reconcile/float/host` | 130 ms | 128 ms | -1.5% | [-1.9%, -1.1%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.8 ms | 41.0 ms | -1.8% | [-2.3%, -1.3%] | +0.0% | ~ | reconcile/scheduled 3.13x |
| `MeshReconcile/armadillo/reconcile/double/host` | 153 ms | 150 ms | -2.1% | [-3.5%, -0.8%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.0 ms | 55.9 ms | -0.1% | [-1.4%, +1.2%] | +0.0% | ~ | reconcile/scheduled 2.68x |
| `PointCloudBVH/uniform/build/float/host` | 53.4 ms | 53.1 ms | -0.4% | [-1.2%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 141 ms | 143 ms | +0.8% | [+0.5%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 350 ms | 360 ms | +3.0% | [+2.8%, +3.4%] | - | minor |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 79.5 ms | 80.1 ms | +0.7% | [+0.7%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.67 ms | 6.74 ms | +1.0% | [+0.7%, +1.3%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.7 ms | 55.4 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 154 ms | 155 ms | +0.2% | [+0.2%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 383 ms | 394 ms | +2.8% | [+2.7%, +2.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.2 ms | 86.5 ms | +0.4% | [-0.9%, +1.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.87 ms | 6.91 ms | +0.6% | [+0.2%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.6 ms | 13.8 ms | +1.7% | [-1.1%, +4.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 117 ms | 117 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 471 ms | 474 ms | +0.7% | [+0.7%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 140 ms | 142 ms | +1.1% | [+0.8%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 800 ms | 799 ms | -0.1% | [-0.1%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 14.8 ms | -0.8% | [-1.7%, +0.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 119 ms | -1.0% | [-1.5%, -0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 491 ms | 492 ms | +0.1% | [-0.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 143 ms | 144 ms | +0.2% | [+0.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 796 ms | 764 ms | -4.1% | [-4.1%, -4.0%] | - | faster |  |
| `PointCloudBVH/sphere/build/float/host` | 54.2 ms | 54.2 ms | +0.1% | [+0.0%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 127 ms | 128 ms | +1.1% | [+0.8%, +1.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 259 ms | 266 ms | +2.9% | [+2.7%, +3.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 147 ms | 149 ms | +1.4% | [+1.1%, +1.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 133 ms | 136 ms | +2.4% | [+2.2%, +2.7%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.5 ms | 56.7 ms | -1.4% | [-2.1%, -0.8%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 137 ms | 139 ms | +0.9% | [+0.6%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 290 ms | 298 ms | +3.0% | [+2.7%, +3.2%] | - | minor |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 161 ms | 162 ms | +0.6% | [+0.3%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 147 ms | 149 ms | +1.0% | [+0.6%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 12.1 ms | 11.9 ms | -1.3% | [-2.9%, +0.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 114 ms | 114 ms | -0.1% | [-0.7%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 315 ms | 316 ms | +0.3% | [+0.1%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 156 ms | +1.1% | [-0.3%, +2.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 322 ms | 321 ms | -0.3% | [-0.5%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.5 ms | 13.6 ms | +1.4% | [-0.2%, +2.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 116 ms | +1.0% | [+0.7%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 345 ms | 347 ms | +0.8% | [+0.7%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 164 ms | 164 ms | +0.4% | [-0.0%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 311 ms | 310 ms | -0.1% | [-1.2%, +1.1%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.1 ms | 35.1 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.1 ms | 38.4 ms | +0.7% | [-0.2%, +1.6%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.38 ms | 8.37 ms | -0.1% | [-2.1%, +1.9%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.41 ms | 9.54 ms | +1.4% | [-0.2%, +2.9%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.4 ms | 46.4 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.4 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.2 ms | 40.2 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.1 ms | 78.5 ms | +0.5% | [-0.1%, +1.0%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.2 ms | 65.1 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.33 ms | 5.24 ms | -1.7% | [-4.4%, +1.2%] | - | ~ | tape/fused 1.46x |
| `Ray/csg-tape/firstHit/float/host` | 7.77 ms | 7.66 ms | -1.5% | [-4.2%, +1.3%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.2 ms | 51.8 ms | -0.7% | [-1.0%, -0.4%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 50.9 ms | 50.9 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.5 ms | 44.4 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 86.4 ms | 86.7 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 72.5 ms | 72.6 ms | +0.2% | [-0.6%, +0.9%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.69 ms | 5.50 ms | -3.4% | [-6.9%, +0.3%] | - | ~ | tape/fused 1.44x |
| `Ray/csg-tape/firstHit/double/host` | 8.00 ms | 7.91 ms | -1.1% | [-1.3%, -0.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.3 ms | 17.4 ms | +0.7% | [+0.0%, +1.4%] | - | ~ | marched/traced 6.83x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 119 ms | -0.4% | [-0.5%, -0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 21.9 ms | -0.3% | [-1.8%, +1.2%] | +0.0% | ~ | marched/traced 6.62x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 145 ms | 145 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.4 ms | 35.2 ms | -0.6% | [-0.7%, -0.5%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 18.9 ms | 19.8 ms | +4.7% | [+3.6%, +6.8%] | - | minor | marched/traced 6.31x (was 6.70x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 127 ms | 125 ms | -1.5% | [-1.5%, -1.4%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.3 ms | 23.8 ms | +2.3% | [+2.1%, +2.5%] | +0.0% | ~ | marched/traced 6.30x (was 6.51x) |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 152 ms | 150 ms | -1.0% | [-2.1%, +0.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.5 ms | 37.8 ms | +0.8% | [+0.6%, +1.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.2 ms | 46.3 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.3 ms | 46.4 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.69 ms | 7.64 ms | -0.6% | [-1.2%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.13 ms | 6.16 ms | +0.4% | [-1.1%, +1.8%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 176 ms | 177 ms | +0.6% | [+0.5%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 155 ms | 157 ms | +1.0% | [+0.8%, +1.3%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.5 ms | 68.4 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.2 ms | 56.1 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.4 ms | 52.2 ms | -0.4% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.5 ms | 51.8 ms | -1.5% | [-1.7%, -1.2%] | +0.0% | ~ | hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.02 ms | 7.89 ms | -1.5% | [-3.9%, +0.9%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.24 ms | 6.30 ms | +1.0% | [+1.0%, +1.1%] | +0.0% | ~ | hit/crossing 1.25x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 200 ms | 199 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 177 ms | 177 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 76.8 ms | 77.0 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 63.5 ms | 63.5 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | -0.1% | [-1.3%, +1.2%] | +0.0% | ~ | marched/traced 4.65x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.6 ms | 51.6 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 7.96 ms | 8.09 ms | +1.7% | [-0.2%, +3.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.92 ms | 1.92 ms | +0.1% | [-1.6%, +1.8%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.03 ms | 6.81 ms | -3.2% | [-3.8%, -2.5%] | +0.0% | faster |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.7 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | marched/traced 5.15x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.3 ms | 60.2 ms | -1.9% | [-3.4%, -0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.62 ms | 8.66 ms | +0.4% | [-1.2%, +2.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.92 ms | 1.92 ms | -0.5% | [-1.6%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.22 ms | 7.45 ms | +3.2% | [-0.8%, +7.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 238 ms | 240 ms | +0.6% | [+0.0%, +1.1%] | +0.0% | ~ | marched/traced 1.36x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 327 ms | 327 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 236 ms | 237 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 253 ms | 253 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | marched/traced 1.44x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 366 ms | 365 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 252 ms | 251 ms | -0.7% | [-0.7%, -0.6%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.29 ms | 7.32 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.6 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 264 ms | 264 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.4 ms | 12.5 ms | +0.8% | [+0.6%, +1.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.1 ms | 30.3 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.1 ms | 22.0 ms | -0.4% | [-0.8%, +0.0%] | +0.0% | ~ | regions/hit 1.38x |
| `Regions/particles/region/double/host` | 7.65 ms | 7.64 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.9 ms | 12.8 ms | -0.7% | [-1.5%, -0.0%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 282 ms | 281 ms | -0.5% | [-0.5%, -0.5%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.0 ms | 12.9 ms | -0.8% | [-1.5%, -0.1%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.8 ms | 32.1 ms | -2.0% | [-3.6%, -0.4%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.7 ms | 23.5 ms | -0.9% | [-1.3%, -0.5%] | +0.0% | ~ | regions/hit 1.37x |
| `Render/csg/frame/float/host` | 20.9 ms | 20.8 ms | -0.4% | [-1.5%, +0.8%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 22.5 ms | 20.7 ms | -8.1% | [-14.5%, -0.8%] | +0.0% | faster |  |
| `Render/mesh/frame/float/host` | 18.2 ms | 18.4 ms | +1.1% | [+0.8%, +1.4%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 21.3 ms | 22.7 ms | +6.6% | [+5.3%, +7.7%] | +0.0% | minor |  |
| `Render/csg/frame-orthographic/double/host` | 21.4 ms | 22.7 ms | +6.0% | [+5.2%, +7.7%] | +0.0% | minor |  |
| `Render/mesh/frame/double/host` | 20.6 ms | 20.8 ms | +0.8% | [+0.1%, +1.4%] | +0.0% | ~ |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 33.1 ms | 33.1 ms | -0.1% | [-1.2%, +1.1%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 5.05 ms | 5.01 ms | -0.7% | [-1.7%, +0.2%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.18 ms | 4.20 ms | +0.4% | [+0.2%, +0.6%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.4 ms | 17.3 ms | -0.8% | [-1.6%, -0.1%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.33 ms | 4.31 ms | -0.6% | [-1.2%, +0.0%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.14 ms | 4.13 ms | -0.2% | [-1.4%, +1.1%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.22 ms | 2.31 ms | +3.8% | [+0.1%, +5.6%] | - | minor |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.45 ms | 1.46 ms | +0.3% | [-1.6%, +2.3%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.08 ms | 2.07 ms | -0.3% | [-3.1%, +2.6%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.1 ms | 27.1 ms | +0.1% | [+0.0%, +0.2%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 46.2 ms | 46.1 ms | -0.1% | [-0.2%, +0.0%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.37 ms | 5.39 ms | +0.4% | [-0.2%, +0.9%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.47 ms | 4.47 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 22.4 ms | 22.5 ms | +0.6% | [+0.2%, +1.1%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.55 ms | 4.54 ms | -0.1% | [-0.4%, +0.3%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.36 ms | 5.32 ms | -0.8% | [-1.0%, -0.5%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.49 ms | 2.49 ms | +0.2% | [-0.7%, +1.2%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.79 ms | 1.78 ms | -0.9% | [-1.7%, -0.1%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.06 ms | 2.11 ms | +2.1% | [+0.6%, +3.7%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 30.5 ms | 30.2 ms | -0.9% | [-2.9%, +1.2%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.5 ms | 56.2 ms | -0.5% | [-0.9%, -0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.6 ms | 36.0 ms | -1.6% | [-2.2%, -1.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 91.8 ms | 91.9 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.2 ms | 63.6 ms | -0.9% | [-1.2%, -0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.4 ms | 41.9 ms | -1.1% | [-1.6%, -0.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 105 ms | 105 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.6 ms | 74.6 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 293 ms | 295 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 293 ms | 295 ms | +0.6% | [+0.6%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 85.5 ms | 85.8 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 339 ms | 342 ms | +0.8% | [+0.5%, +1.1%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 343 ms | 341 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 165 ms | 165 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 189 ms | 189 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 143 ms | 143 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 89.6 ms | 89.6 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 130 ms | 130 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 72.7 ms | 72.1 ms | -0.8% | [-1.0%, -0.6%] | +0.0% | ~ | surface/leaf 1.80x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 61.1 ms | 61.0 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.1 ms | 34.0 ms | -0.5% | [-1.2%, +0.1%] | +0.0% | ~ | surface/hit 1.80x |
| `SurfaceParts/boxes/closestSurface/double/host` | 145 ms | 145 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 91.1 ms | 90.5 ms | -0.6% | [-0.8%, -0.4%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 143 ms | 144 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 80.5 ms | 80.8 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 67.9 ms | 67.8 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 38.7 ms | 38.6 ms | -0.3% | [-0.7%, -0.0%] | +0.0% | ~ | surface/hit 1.76x |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 13.0 ms | +1.1% | [+0.7%, +1.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.59 ms | 4.59 ms | -0.2% | [-0.6%, +0.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.1 ms | 25.1 ms | -0.1% | [-2.4%, +2.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 39.9 ms | 40.0 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.76 ms | 1.74 ms | -1.1% | [-2.8%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 136 µs | 133 µs | -2.1% | [-5.7%, +1.7%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.1 ms | 29.9 ms | -0.8% | [-2.0%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.67 ms | 5.67 ms | +0.0% | [-0.4%, +0.5%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.1 ms | 16.1 ms | +0.1% | [-1.0%, +1.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.67 ms | 4.62 ms | -1.0% | [-1.4%, -0.6%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.2 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.56 ms | 4.63 ms | +1.4% | [+0.6%, +2.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.6 ms | 26.7 ms | +0.4% | [-0.8%, +1.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.0 ms | 42.0 ms | -0.1% | [-0.8%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.77 ms | 1.77 ms | +0.3% | [-2.6%, +3.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 282 µs | 288 µs | +2.0% | [-9.6%, +14.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.8 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.73 ms | 7.73 ms | -0.0% | [-0.2%, +0.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.3 ms | 16.2 ms | -0.3% | [-1.8%, +1.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.72 ms | 7.75 ms | +0.4% | [-0.1%, +0.8%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.8 ms | 42.0 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 218 ms | 201 ms | -8.1% | [-8.5%, -7.8%] | +0.0% | faster |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 43.4 ms | 43.0 ms | -0.9% | [-1.4%, -0.3%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 219 ms | 217 ms | -0.7% | [-3.2%, +2.0%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 621.1 s → 617.4 s (-0.6%), the largest peak 1407 MB → 1398 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.0 s | -0.7% | 555 MB | 555 MB | -0.1% | ~ |
| `BVHRefit` | 3.9 s | 3.9 s | -1.0% | 483 MB | 482 MB | -0.1% | ~ |
| `BVHUnion` | 17.9 s | 17.8 s | -0.4% | 896 MB | 893 MB | -0.3% | ~ |
| `BVHUnionMixed` | 7.3 s | 7.3 s | -0.9% | 556 MB | 555 MB | -0.2% | ~ |
| `ClosestPrimitive` | 28.5 s | 28.9 s | +1.7% | 1262 MB | 1262 MB | -0.0% | ~ |
| `ClosestSurface` | 27.4 s | 27.2 s | -0.5% | 1221 MB | 1219 MB | -0.1% | ~ |
| `Domain` | 17.7 s | 17.5 s | -1.4% | 838 MB | 838 MB | -0.0% | ~ |
| `Fused` | 23.7 s | 23.7 s | +0.0% | 1126 MB | 1125 MB | -0.1% | ~ |
| `LaunchTuning` | 36.5 s | 35.1 s | -3.7% | 1271 MB | 1269 MB | -0.2% | ~ |
| `Mesh` | 14.8 s | 14.4 s | -2.6% | 785 MB | 785 MB | -0.0% | ~ |
| `MeshCSG` | 37.7 s | 37.8 s | +0.3% | 1284 MB | 1281 MB | -0.3% | ~ |
| `MeshReconcile` | 6.6 s | 6.6 s | -0.1% | 565 MB | 564 MB | -0.2% | ~ |
| `PointCloud` | 7.3 s | 7.5 s | +2.8% | 557 MB | 558 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.8 s | -0.5% | 468 MB | 469 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 3.2 s | 3.2 s | +1.6% | 452 MB | 451 MB | -0.1% | ~ |
| `Ray` | 35.3 s | 35.0 s | -0.7% | 1318 MB | 1316 MB | -0.2% | ~ |
| `RayCSG` | 31.6 s | 31.4 s | -0.6% | 1112 MB | 1113 MB | +0.1% | ~ |
| `RayCrossing` | 44.7 s | 44.3 s | -0.9% | 1341 MB | 1341 MB | -0.0% | ~ |
| `RayParticles` | 47.1 s | 47.9 s | +1.7% | 1312 MB | 1318 MB | +0.4% | ~ |
| `RayShell` | 36.1 s | 36.6 s | +1.4% | 1274 MB | 1269 MB | -0.4% | ~ |
| `Regions` | 28.7 s | 27.8 s | -3.0% | 1067 MB | 1066 MB | -0.1% | ~ |
| `Render` | 35.2 s | 34.8 s | -1.1% | 1265 MB | 1262 MB | -0.3% | ~ |
| `Shapes` | 9.3 s | 8.8 s | -5.4% | 554 MB | 553 MB | -0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.9 s | +2.3% | 508 MB | 510 MB | +0.4% | ~ |
| `SphereCloudRays` | 27.2 s | 27.4 s | +0.6% | 1061 MB | 1059 MB | -0.2% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | +0.4% | 482 MB | 483 MB | +0.2% | ~ |
| `SurfaceParts` | 40.3 s | 39.4 s | -2.2% | 1407 MB | 1398 MB | -0.6% | ~ |
| `Tape` | 16.5 s | 16.5 s | +0.0% | 819 MB | 816 MB | -0.4% | ~ |
| `TapeBVHUnion` | 14.8 s | 14.5 s | -1.8% | 820 MB | 821 MB | +0.1% | ~ |
| **all, in both** | 621.1 s | 617.4 s | -0.6% | 1407 MB | 1398 MB | -0.6% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (7 cases): PointCloudBVH/uniform/all8nn/float/host, PointCloudHashGrid/uniform/build/float/host, PointCloudBVH/sphere/all8nn/double/host, RayCSG/scene-fused/firstHit/double/host, Render/csg/frame/double/host, Render/csg/frame-orthographic/double/host, Shapes/cut-sphere/signedDistance/float/host

The probes differ between the sides (each side ran its own): Performance/Domain/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 569fff8
base-fingerprint: eb69a7e3bccddc2ac0d8879ed8618df6c8084558
head: 897a647
head-fingerprint: f7653fee8c5b2beba423cac303718dc728ad6b3e
cases: 368
rounds: 2
reps: 3
date: 2026-11-15
-->

## Device comparison (CUDA): yellow

**Accepted:** Side effect of improved rendering.

`897a647` against `569fff8` (origin/main), 2026-11-15.

**VERDICT: 2 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 2 minor, 2 faster, 0 differ, 0 lost, 368 cases

- base: 569fff8 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 897a647 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.60 ms | 1.61 ms | +0.9% | [+0.3%, +1.4%] | - | ~ | device 22.7x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.41 ms | 3.41 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 18.5x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.06 ms | +0.8% | [+0.0%, +1.6%] | - | ~ | device 19.3x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.30 ms | 2.31 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 16.5x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.29 ms | 6.13 ms | -2.5% | [-4.2%, -0.8%] | - | ~ | device 7.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 12.0 ms | 12.0 ms | -0.5% | [-3.6%, +2.7%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.5 ms | 33.3 ms | -0.7% | [-1.5%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 56.8 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.9 ms | 20.5 ms | -1.5% | [-2.1%, -0.8%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.9 ms | 48.0 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.6 ms | 61.6 ms | -0.0% | [-2.0%, +2.0%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 110 ms | +0.6% | [-3.5%, +4.9%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.23 ms | 8.18 ms | -0.7% | [-3.6%, +2.4%] | +0.0% | ~ | device 4.9x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.79 ms | 9.85 ms | +0.7% | [-1.2%, +2.6%] | +0.0% | ~ | device 8.1x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 390 µs | 387 µs | -0.9% | [-0.9%, -0.8%] | - | ~ | device 15.2x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 844 µs | 844 µs | +0.1% | [-0.3%, +0.5%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.51 ms | 4.53 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.67 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 584 µs | 586 µs | +0.3% | [-0.0%, +0.7%] | - | ~ | device 12.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.39 ms | 1.39 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 8.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.75 ms | 6.75 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.00 ms | 7.02 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 900 µs | 904 µs | +0.4% | [-0.1%, +0.9%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.26 ms | 2.25 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.49 ms | 9.45 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 846 µs | 848 µs | +0.2% | [-0.3%, +0.7%] | - | ~ | device 13.5x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.09 ms | 2.08 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 7.9x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.10 ms | 9.11 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.84 ms | 9.86 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 312 µs | 314 µs | +0.4% | [+0.4%, +0.4%] | - | ~ | device 11.7x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 6.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.38 ms | 5.39 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 1.6x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 517 ms | 517 ms | -0.1% | [-1.4%, +1.2%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.1 ms | 20.1 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 4.1x faster than the host |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.33 ms | 8.33 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 5.3x faster than the host (was 4.7x); surface/leaf 2.42x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.6 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 8.0x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.19 ms | 6.19 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 10.0x faster than the host; surface/leaf 2.52x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 146 ms | 147 ms | +0.5% | [-0.8%, +1.8%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.1 ms | 60.0 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.45x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 71.7 ms | 71.9 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.8x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.4x faster than the host; surface/leaf 2.57x |
| `Domain/particles/labels/float/hostdevice` | 4.09 ms | 4.07 ms | -0.5% | [-1.0%, +0.1%] | - | ~ | device 3.5x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.72 ms | 2.72 ms | +0.1% | [-0.7%, +0.9%] | - | ~ | device 2.7x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 120 µs | 121 µs | +0.6% | [+0.2%, +0.9%] | - | ~ | device 12.2x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.39 ms | 7.38 ms | -0.1% | [-2.7%, +2.6%] | - | ~ | device 1.9x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.39 ms | 3.39 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 2.1x faster than the host |
| `Domain/particles/surfaceGroups/double/hostdevice` | 130 µs | 127 µs | -1.8% | [-4.5%, +0.9%] | - | ~ | device 11.2x faster than the host (was 10.8x) |
| `Fused/csg/signedDistance/float/hostdevice` | 48.1 µs | 48.0 µs | -0.2% | [-0.4%, -0.0%] | - | ~ | device 23.5x faster than the host; tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.4 µs | 49.7 µs | -1.3% | [-2.6%, +0.1%] | - | ~ | device 45.1x faster than the host (was 43.3x); tape/fused 7.23x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.7 µs | 47.8 µs | +0.3% | [-0.1%, +0.7%] | - | ~ | device 9.3x faster than the host; tape/fused 9.94x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.4 ms | +0.5% | [-0.0%, +1.1%] | - | ~ | device 6.7x faster than the host; tape/fused 1.79x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.1 ms | 31.1 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.1% | [-0.3%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host (was 1.5x); tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 284 ms | 284 ms | -0.0% | [-0.9%, +0.8%] | - | ~ | device 0.4x faster than the host; tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 287 ms | 285 ms | -0.8% | [-1.0%, -0.5%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.20 ms | 8.23 ms | +0.3% | [-1.1%, +1.7%] | +0.0% | ~ | device 10.8x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.25 ms | 8.29 ms | +0.5% | [-0.0%, +1.0%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.17 ms | 8.20 ms | +0.4% | [-1.0%, +1.8%] | +0.0% | ~ | device 10.8x faster than the host (was 12.9x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.25 ms | 8.13 ms | -1.4% | [-2.8%, +0.1%] | +0.0% | ~ | device 10.9x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 685 µs | 687 µs | +0.3% | [-0.4%, +1.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.73 ms | 1.73 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.03 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.67x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 789 µs | 789 µs | -0.0% | [-0.7%, +0.6%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 736 µs | 740 µs | +0.5% | [-0.2%, +1.1%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 835 µs | 832 µs | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 738 µs | 740 µs | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 729 µs | 730 µs | +0.1% | [-1.2%, +1.4%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 156 ms | -2.3% | [-4.5%, -0.0%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 159 ms | 161 ms | +1.4% | [-0.8%, +2.7%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.97x (was 1.01x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 159 ms | 162 ms | +1.7% | [+1.0%, +2.4%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.97x (was 1.01x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 158 ms | 162 ms | +2.7% | [+0.4%, +5.1%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x); default/this 0.96x (was 1.01x) |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ | device 2.0x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.78 ms | 2.77 ms | -0.4% | [-0.6%, -0.3%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.93 ms | 1.93 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.72x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.55 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.25 ms | 7.29 ms | +0.5% | [-0.0%, +1.0%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.26 ms | 7.28 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.25 ms | 7.21 ms | -0.6% | [-0.7%, -0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.34 ms | 7.31 ms | -0.4% | [-1.3%, +0.4%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.54 ms | 8.41 ms | -1.5% | [-2.5%, -0.5%] | - | ~ | device 11.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.7 ms | 16.7 ms | -0.0% | [-0.7%, +0.7%] | - | ~ | device 7.4x faster than the host (was 7.1x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 166 ms | 168 ms | +0.9% | [-1.0%, +2.9%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 282 ms | 284 ms | +0.7% | [-1.5%, +3.1%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.5 ms | 24.6 ms | +0.2% | [-0.9%, +1.2%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.7 ms | 58.8 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 156 ms | 157 ms | +0.1% | [-2.0%, +2.2%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 244 ms | 246 ms | +0.8% | [-1.7%, +3.3%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.51 ms | 2.54 ms | +1.0% | [+0.3%, +1.8%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.28 ms | 4.30 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.7 ms | 13.8 ms | +0.7% | [+0.3%, +1.2%] | - | ~ | device 4.0x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.1 ms | 63.5 ms | +0.5% | [+0.0%, +1.0%] | - | ~ | device 3.5x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 94.0 ms | 92.0 ms | -2.1% | [-3.4%, -0.8%] | - | ~ | device 2.6x faster than the host (was 2.3x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 249 ms | 251 ms | +0.8% | [-0.1%, +1.8%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.8 ms | 24.7 ms | -0.2% | [-1.5%, +1.0%] | - | ~ | device 2.3x faster than the host (was 2.1x); tape/fused 2.56x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 26.6 ms | -1.0% | [-1.5%, -0.5%] | - | ~ | device 2.2x faster than the host; tape/fused 3.45x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.6 ms | 73.0 ms | +2.0% | [+1.4%, +3.3%] | - | ~ | device 4.0x faster than the host; tape/fused 3.43x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.3 ms | 22.5 ms | +1.2% | [-5.5%, +3.5%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.6 ms | 44.7 ms | +0.2% | [-1.7%, +2.1%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 93.2 ms | 95.6 ms | +2.5% | [+0.9%, +4.2%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 548 ms | 514 ms | -6.2% | [-9.4%, -3.0%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 848 ms | 845 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.04 s | 2.04 s | +0.0% | [-1.5%, +1.9%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 191 ms | 195 ms | +2.2% | [-1.6%, +6.9%] | - | ~ | device 0.3x faster than the host; tape/fused 2.64x (was 2.87x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 309 ms | 309 ms | +0.1% | [-5.3%, +5.7%] | - | ~ | device 0.2x faster than the host; tape/fused 2.73x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 598 ms | 596 ms | -0.4% | [-3.3%, +2.6%] | - | ~ | device 0.7x faster than the host (was 0.6x); tape/fused 3.43x |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 15.5x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.30 ms | 5.31 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 10.3x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 677 µs | 686 µs | +1.3% | [+0.5%, +2.2%] | - | ~ | device 11.0x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.83 ms | 1.84 ms | +0.2% | [-0.4%, +0.9%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 995 µs | 996 µs | +0.1% | [-0.1%, +0.2%] | - | ~ | device 9.4x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.61 ms | 4.63 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 5.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.97 ms | 2.98 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.07 ms | 7.11 ms | +0.5% | [+0.1%, +0.8%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.77 ms | 7.79 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.4 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 0.8x faster than the host (was 0.7x) |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.46 ms | 1.47 ms | +0.4% | [-2.3%, +3.1%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.87 ms | 3.87 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 526 µs | 524 µs | -0.4% | [-0.6%, -0.3%] | - | ~ | device 15.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.70 ms | 4.61 ms | -1.8% | [-2.9%, -0.7%] | - | ~ | device 6.6x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.31 ms | 4.42 ms | +2.5% | [+1.4%, +3.6%] | - | ~ | device 5.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.06 ms | 8.17 ms | +1.3% | [+0.7%, +2.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.7 ms | -0.7% | [-1.0%, -0.4%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.02 ms | +0.3% | [+0.0%, +0.5%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.4 ms | 17.5 ms | +0.5% | [+0.2%, +0.9%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.1% | [-0.8%, +0.7%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.57 ms | -0.8% | [-1.1%, -0.4%] | - | ~ | device 10.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.64 ms | 4.63 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 827 µs | 828 µs | +0.1% | [-0.8%, +1.1%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.87 ms | 2.88 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.15 ms | 4.18 ms | +0.7% | [+0.2%, +1.2%] | - | ~ | device 5.4x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.4 ms | 16.5 ms | +0.3% | [-0.5%, +1.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 21.0 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.96 ms | 8.93 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.5 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.3 ms | 20.3 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.73 ms | 2.75 ms | +0.7% | [-0.9%, +2.4%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.71 ms | 6.73 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.73 ms | 2.74 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 7.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.40 ms | 6.45 ms | +0.7% | [+0.3%, +1.0%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.8 ms | 33.7 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.5 ms | 68.5 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.5 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.9 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.1 ms | 38.3 ms | +0.6% | [-0.4%, +1.5%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.68 ms | 9.64 ms | -0.4% | [-0.8%, -0.1%] | +0.0% | ~ | device 4.0x faster than the host (was 3.5x) |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.8 ms | -0.0% | [-0.0%, -0.0%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.28 ms | 3.28 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.57 ms | 2.58 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.58 ms | 4.55 ms | -0.7% | [-1.0%, -0.3%] | +0.0% | ~ | device 2.5x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.03 ms | 3.04 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 674 µs | 676 µs | +0.4% | [-0.1%, +0.9%] | - | ~ | device 16.7x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.27 ms | 1.28 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 14.7x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 650 µs | 652 µs | +0.3% | [-3.2%, +4.0%] | - | ~ | device 17.0x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.21 ms | -0.4% | [-0.6%, -0.3%] | - | ~ | device 15.3x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 613 µs | 613 µs | -0.1% | [-0.8%, +0.6%] | - | ~ | device 15.8x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.11 ms | -1.3% | [-1.6%, -1.0%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.18 ms | -0.3% | [-1.1%, +0.6%] | - | ~ | device 12.0x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.51x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.08 ms | 1.07 ms | -0.6% | [-1.2%, -0.0%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.19 ms | 2.19 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 9.7x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 628 µs | 598 µs | -4.8% | [-5.6%, +3.0%] | - | ~ | device 2.5x faster than the host (was 2.4x); tape/fused 1.14x (was 1.09x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 694 µs | 693 µs | -0.2% | [-8.7%, +9.2%] | - | ~ | device 2.6x faster than the host; tape/fused 1.23x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 684 µs | 683 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 3.7x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 843 µs | 850 µs | +0.8% | [+0.0%, +1.5%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 14.8 ms | -0.8% | [-2.1%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.7% | [+0.2%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.5 ms | -0.0% | [-2.0%, +2.0%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.3% | [-0.4%, -0.1%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.1 ms | +0.7% | [+0.3%, +1.0%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 23.0 ms | -0.6% | [-1.0%, -0.3%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.24 ms | 1.25 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.36 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 2.0x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.48 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.9x faster than the host; marched/traced 3.54x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.99 ms | 2.99 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.7x faster than the host; marched/traced 4.63x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.01 ms | 9.00 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 4.4x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.8 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.17 ms | 2.18 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.9x faster than the host; marched/traced 13.39x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.48 ms | 5.47 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 2.8x faster than the host; marched/traced 10.10x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.0 ms | 29.1 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.1 ms | 55.2 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.25 ms | 5.21 ms | -0.9% | [-1.1%, -0.7%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.76 ms | 8.76 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.18 ms | 6.18 ms | +0.0% | [-0.7%, +0.8%] | - | ~ | device 1.3x faster than the host; marched/traced 26.32x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.5 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 0.8x faster than the host; marched/traced 24.04x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 163 ms | +2.5% | [+1.0%, +3.9%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 255 ms | 253 ms | -0.8% | [-2.4%, +0.9%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.88 ms | 4.87 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.8x faster than the host; marched/traced 22.57x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.3 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 1.3x faster than the host; marched/traced 17.72x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 109 ms | 110 ms | +0.5% | [-1.4%, +2.6%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 217 ms | 218 ms | +0.3% | [-2.8%, +3.4%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.2 ms | 19.3 ms | +0.6% | [-0.1%, +1.2%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 673 µs | 674 µs | +0.2% | [-0.8%, +1.2%] | - | ~ | device 17.1x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.29 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 14.9x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 677 µs | 677 µs | -0.1% | [-0.9%, +0.8%] | - | ~ | device 17.1x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 689 µs | 691 µs | +0.3% | [-0.0%, +0.5%] | - | ~ | device 3.7x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 851 µs | 846 µs | -0.5% | [-1.5%, +0.4%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 619 µs | 620 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.2x faster than the host; hit/crossing 1.11x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 636 µs | 636 µs | -0.0% | [-0.8%, +0.8%] | - | ~ | device 3.7x faster than the host; hit/crossing 1.33x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.69 ms | -0.2% | [-0.5%, -0.0%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 7.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.19 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 10.0x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.08 ms | 9.09 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 6.7x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 55.6 ms | 56.1 ms | +1.0% | [-0.1%, +2.1%] | - | ~ | device 3.6x faster than the host (was 3.1x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 79.8 ms | 81.2 ms | +1.7% | [+0.3%, +3.1%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.4 ms | 48.4 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.0x faster than the host; hit/crossing 1.16x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 62.8 ms | 64.0 ms | +1.9% | [+1.2%, +2.6%] | - | ~ | device 2.4x faster than the host (was 2.7x); hit/crossing 1.27x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.6 ms | 27.4 ms | -0.7% | [-1.0%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.2 ms | 17.0 ms | -1.2% | [-2.2%, -0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.3% | [-0.6%, +1.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 2.0x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.50 ms | -0.3% | [-0.8%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.10 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.27x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.17 ms | 1.16 ms | -1.0% | [-1.8%, -0.1%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.3 ms | 64.6 ms | +0.5% | [-0.1%, +1.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 123 ms | 122 ms | -0.4% | [-2.1%, +1.3%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.5 ms | 58.5 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.11x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 110 ms | 111 ms | +1.6% | [+0.6%, +2.7%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 480 ms | 468 ms | -2.6% | [-8.5%, +3.8%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 709 ms | 716 ms | +1.0% | [-0.4%, +2.5%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 391 ms | 368 ms | -5.7% | [-11.0%, +0.2%] | - | ~ | device 0.4x faster than the host (was 0.4x); hit/crossing 1.27x (was 1.23x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 535 ms | 527 ms | -1.5% | [-6.8%, +4.2%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.36x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.56 ms | 3.54 ms | -0.7% | [-3.7%, +2.3%] | - | ~ | device 1.2x faster than the host; marched/traced 3.40x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.79 ms | 4.84 ms | +1.0% | [-0.3%, +2.3%] | - | ~ | device 0.9x faster than the host; marched/traced 5.50x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.6 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.25 ms | 1.24 ms | -1.1% | [-1.7%, -0.6%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.86 ms | +0.7% | [-0.5%, +1.9%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.9 µs | 98.7 µs | +3.0% | [+0.1%, +5.9%] | - | ~ | device 14.8x faster than the host (was 15.4x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 152 µs | 150 µs | -1.4% | [-2.8%, +0.1%] | - | ~ | device 11.1x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 792 µs | 793 µs | +0.1% | [+0.0%, +0.3%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 928 µs | 926 µs | -0.2% | [-0.4%, -0.1%] | - | ~ | device 2.8x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.2 ms | 35.0 ms | -0.8% | [-2.8%, +1.3%] | - | ~ | device 0.1x faster than the host; marched/traced 3.50x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.8 ms | 36.6 ms | -0.6% | [-1.1%, -0.2%] | - | ~ | device 0.1x faster than the host; marched/traced 6.31x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 120 ms | 122 ms | +1.7% | [+0.6%, +2.9%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 230 ms | 231 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.1 ms | 16.3 ms | +1.4% | [-0.9%, +3.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.6 ms | 21.7 ms | +0.8% | [-0.7%, +2.3%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.11 ms | 1.06 ms | -4.4% | [-10.3%, +2.2%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.34 ms | 2.36 ms | +0.8% | [+0.5%, +1.0%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 5.97 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.73 ms | 6.72 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.3 ms | 23.2 ms | -0.6% | [-2.6%, +1.4%] | - | ~ | device 2.2x faster than the host; marched/traced 2.68x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.4 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 1.9x faster than the host; marched/traced 3.66x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.7 ms | 62.2 ms | -0.9% | [-1.1%, -0.6%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 103 ms | 104 ms | +0.7% | [-0.2%, +1.7%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.7 ms | -1.0% | [-2.0%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.4 ms | +0.1% | [-0.8%, +1.0%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 309 ms | 306 ms | -1.1% | [-6.7%, +4.8%] | - | ~ | device 0.2x faster than the host; marched/traced 1.82x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 351 ms | 347 ms | -1.3% | [-2.2%, -0.4%] | - | ~ | device 0.2x faster than the host; marched/traced 2.85x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 565 ms | 558 ms | -1.3% | [-3.1%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 986 ms | 991 ms | +0.4% | [-1.8%, +2.7%] | - | ~ | device 0.2x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 196 ms | 186 ms | -5.0% | [-6.0%, -4.0%] | - | faster | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 240 ms | 238 ms | -0.5% | [-3.0%, +2.1%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.74 ms | 2.75 ms | +0.5% | [+0.3%, +0.7%] | - | ~ | device 2.7x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.75 ms | 3.76 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 3.4x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 96.5 ms | 96.9 ms | +0.4% | [-0.3%, +1.1%] | - | ~ | device 3.6x faster than the host (was 3.2x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.66 ms | 3.66 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.51 ms | 6.50 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.9x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.42 ms | 5.41 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 3.3x faster than the host; regions/hit 1.20x |
| `Regions/particles/region/double/hostdevice` | 3.69 ms | 3.70 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 2.0x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.89 ms | 5.91 ms | +0.3% | [-0.5%, +1.0%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 138 ms | 137 ms | -0.3% | [-1.1%, +0.6%] | - | ~ | device 2.4x faster than the host (was 2.2x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.90 ms | 5.92 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 2.6x faster than the host (was 2.3x) |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.7 ms | 15.7 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 13.0 ms | 12.9 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 1.3x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 3.98 ms | 4.15 ms | +4.2% | [+4.1%, +4.4%] | - | minor | device 2.7x faster than the host (was 2.9x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 3.98 ms | 4.15 ms | +4.3% | [+4.2%, +4.4%] | - | minor | device 2.7x faster than the host (was 2.9x) |
| `Render/mesh/frame/float/hostdevice` | 1.96 ms | 2.01 ms | +2.2% | [+2.1%, +2.3%] | - | ~ | device 8.4x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.66 ms | 7.74 ms | +1.0% | [+0.7%, +1.3%] | - | ~ | device 1.6x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.67 ms | 7.73 ms | +0.8% | [+0.7%, +0.8%] | - | ~ | device 1.6x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.6 ms | 23.8 ms | +0.7% | [+0.1%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 155 µs | 155 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 33.2x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.6 µs | 47.6 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 18.3x faster than the host |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.8 µs | 48.0 µs | +0.4% | [-0.1%, +0.9%] | - | ~ | device 15.4x faster than the host (was 14.9x) |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.1% | [-0.3%, +0.4%] | - | ~ | device 65.7x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 47.8 µs | 47.7 µs | -0.2% | [-0.7%, +0.3%] | - | ~ | device 16.0x faster than the host |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.2% | [+0.0%, +0.4%] | - | ~ | device 17.5x faster than the host (was 16.9x) |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 48.1 µs | 47.9 µs | -0.3% | [-0.8%, +0.1%] | - | ~ | device 10.4x faster than the host (was 10.7x) |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.7 µs | 47.8 µs | +0.2% | [-0.0%, +0.4%] | - | ~ | device 7.8x faster than the host |
| `Shapes/wedge/signedDistance/float/hostdevice` | 47.6 µs | 47.6 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 9.0x faster than the host (was 8.2x) |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 555 µs | 556 µs | +0.2% | [-0.6%, +1.0%] | - | ~ | device 8.1x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.23 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 493 µs | 492 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.0x faster than the host (was 2.1x) |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 421 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 358 µs | 358 µs | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.4x faster than the host (was 2.3x) |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 669 µs | 670 µs | +0.0% | [+0.0%, +0.1%] | - | ~ | device 1.6x faster than the host |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 413 µs | 414 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 339 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.3x faster than the host (was 1.3x) |
| `Shapes/wedge/signedDistance/double/hostdevice` | 154 µs | 154 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.9x faster than the host (was 2.8x) |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.08 ms | 8.04 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 884 µs | 885 µs | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ | device 10.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.65 ms | 2.64 ms | -0.4% | [-0.8%, -0.1%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.0 ms | -0.4% | [-0.9%, -0.0%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 13.9 ms | 14.0 ms | +0.2% | [+0.2%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.02 ms | +0.7% | [+0.1%, +1.2%] | +0.0% | ~ | device 7.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.76 ms | 4.76 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 5.1x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.4 ms | 24.4 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 3.7x faster than the host; tape/fused 1.24x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.3 ms | 30.3 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 3.3x faster than the host (was 3.0x) |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.0 ms | 44.3 ms | +0.7% | [+0.3%, +1.2%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.5 ms | 45.7 ms | +0.6% | [-0.4%, +1.6%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 123 ms | 126 ms | +2.2% | [-1.2%, +5.8%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.02x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 128 ms | 125 ms | -2.8% | [-3.8%, -1.7%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.07x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 128 ms | 129 ms | +0.4% | [-2.4%, +3.3%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 133 ms | 133 ms | +0.0% | [-1.4%, +1.5%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ | device 10.6x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.1 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 0.6x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.32 ms | 9.33 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 11.4x faster than the host |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.93 ms | 5.93 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 9.9x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 96.9 ms | 97.4 ms | +0.5% | [-0.4%, +1.4%] | - | ~ | device 3.8x faster than the host (was 3.5x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.3 ms | 63.3 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 3.5x faster than the host; surface/leaf 1.54x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.1 ms | 29.2 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 5.0x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.88 ms | 9.89 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 6.9x faster than the host; surface/hit 2.95x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.0 ms | 59.0 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.5x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 780 ms | 778 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 0.6x faster than the host (was 0.4x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 525 ms | 529 ms | +0.8% | [+0.3%, +1.3%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.47x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 308 ms | 309 ms | +0.6% | [-1.7%, +3.0%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 122 ms | 118 ms | -3.0% | [-3.3%, -2.6%] | - | ~ | device 0.6x faster than the host (was 0.6x); surface/hit 2.62x (was 2.53x) |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 218 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 11.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 23.7x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 360 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 12.6x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 9.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.0 µs | 71.9 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 7.0x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.5 µs | 47.5 µs | +0.1% | [-0.4%, +0.5%] | - | ~ | device 4.6x faster than the host (was 4.8x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 652 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 12.1x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 477 µs | 476 µs | -0.3% | [-0.7%, +0.1%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.3 µs | +0.4% | [+0.1%, +0.8%] | - | ~ | device 27.2x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 857 µs | 859 µs | +0.2% | [+0.2%, +0.3%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.65 ms | 6.65 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 192 µs | +0.5% | [+0.1%, +0.9%] | - | ~ | device 2.9x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 164 µs | 163 µs | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.9x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.71 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 10.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.50 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.7 ms | 17.7 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4696.8 s → 4707.6 s (+0.2%), the largest peak 5632 MB → 5632 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.8 s | 12.9 s | +0.3% | 596 MB | 597 MB | +0.2% | ~ |
| `BVHRefit` | 6.5 s | 6.6 s | +1.0% | 503 MB | 503 MB | -0.1% | ~ |
| `BVHUnion` | 24.1 s | 24.3 s | +0.8% | 1019 MB | 1022 MB | +0.3% | ~ |
| `BVHUnionMixed` | 10.7 s | 10.8 s | +1.6% | 621 MB | 622 MB | +0.2% | ~ |
| `ClosestPrimitive` | 127.4 s | 128.4 s | +0.7% | 1343 MB | 1345 MB | +0.1% | ~ |
| `ClosestSurface` | 193.5 s | 192.4 s | -0.6% | 1844 MB | 1843 MB | -0.1% | ~ |
| `Domain` | 58.1 s | 58.0 s | -0.2% | 990 MB | 992 MB | +0.1% | ~ |
| `Fused` | 52.7 s | 52.2 s | -0.9% | 1154 MB | 1155 MB | +0.2% | ~ |
| `LaunchTuning` | 534.0 s | 533.2 s | -0.2% | 3806 MB | 3806 MB | +0.0% | ~ |
| `Mesh` | 20.2 s | 20.1 s | -0.3% | 798 MB | 799 MB | +0.1% | ~ |
| `MeshCSG` | 370.6 s | 383.0 s | +3.3% | 5160 MB | 5550 MB | +7.6% | ~ |
| `MeshReconcile` | 9.9 s | 10.0 s | +0.8% | 508 MB | 510 MB | +0.3% | ~ |
| `PointCloud` | 12.3 s | 12.6 s | +2.3% | 566 MB | 568 MB | +0.3% | ~ |
| `PointCloudBVHBuild` | 7.1 s | 7.0 s | -0.6% | 503 MB | 503 MB | -0.1% | ~ |
| `PointCloudHashGridBuild` | 6.3 s | 6.2 s | -0.6% | 504 MB | 503 MB | -0.1% | ~ |
| `Ray` | 188.6 s | 188.7 s | +0.0% | 3703 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 219.2 s | 219.7 s | +0.2% | 4086 MB | 4085 MB | -0.0% | ~ |
| `RayCrossing` | 435.5 s | 435.6 s | +0.0% | 4017 MB | 4017 MB | +0.0% | ~ |
| `RayParticles` | 673.6 s | 671.9 s | -0.2% | 5632 MB | 5632 MB | +0.0% | ~ |
| `RayShell` | 409.3 s | 409.2 s | -0.0% | 5346 MB | 5346 MB | +0.0% | ~ |
| `Regions` | 355.8 s | 355.6 s | -0.1% | 3835 MB | 3835 MB | -0.0% | ~ |
| `Render` | 226.2 s | 235.6 s | +4.2% | 5216 MB | 5386 MB | +3.3% | ~ |
| `Shapes` | 25.7 s | 25.5 s | -1.0% | 587 MB | 589 MB | +0.3% | ~ |
| `SphereCloudBVH` | 8.6 s | 8.6 s | -0.3% | 504 MB | 503 MB | -0.1% | ~ |
| `SphereCloudRays` | 188.3 s | 185.9 s | -1.3% | 4093 MB | 4093 MB | +0.0% | ~ |
| `SphereContacts` | 8.4 s | 8.1 s | -3.6% | 503 MB | 503 MB | -0.1% | ~ |
| `SurfaceParts` | 406.6 s | 401.7 s | -1.2% | 4242 MB | 4242 MB | +0.0% | ~ |
| `Tape` | 63.8 s | 62.9 s | -1.4% | 978 MB | 979 MB | +0.1% | ~ |
| `TapeBVHUnion` | 40.7 s | 40.9 s | +0.4% | 918 MB | 919 MB | +0.1% | ~ |
| **all, in both** | 4696.8 s | 4707.6 s | +0.2% | 5632 MB | 5632 MB | +0.0% | ~ |

Device kernels: 194 in both, 8 changed (8 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Fused::Node<Fused::SmoothCombineOp<…` | 168 → 168 | 4976 → 5040 | 1104/920 → 1116/976 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19376 → 19440 | 1388/1260 → 1456/1428 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Fused::Node<Fused::SmoothCombineOp<f…` | 168 → 168 | 2992 → 2992 | 368/488 → 376/500 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float, TriMeshSDF<float, 4ul, 4…` | 168 → 168 | 13360 → 13424 | 1280/948 → 1296/1108 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 18992 → 19120 | 1388/1260 → 1456/1428 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, TriMeshSDF<double, 4ul, 4ul> >, uns…` | 168 → 168 | 1840 → 1904 | 764/724 → 808/808 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float> >, unsigned int const, f…` | 168 → 168 | 13552 → 13616 | 1284/952 → 1296/1108 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, TriMeshSDF<float, 4ul, 4ul> >, unsig…` | 168 → 168 | 1136 → 1200 | 12/12 → 36/36 | **spills** |


Flagged after 2 rounds, and judged on 10 (8 cases): LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice, MeshCSG/smooth-fused/frame/float/hostdevice, MeshCSG/union-tape/firstHit-morton/double/hostdevice, MeshCSG/smooth-tape/frame/double/hostdevice, MeshCSG/smooth-fused/firstHit-morton/double/hostdevice, Ray/csg-fused/firstHit-morton/float/hostdevice, Render/csg/frame/float/hostdevice, Render/csg/frame-orthographic/float/hostdevice

The probes differ between the sides (each side ran its own): Performance/Domain/main.cpp

<!-- END receipt cuda -->
