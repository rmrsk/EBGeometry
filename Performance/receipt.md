<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: BSD-3-Clause
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
base: 8c7958f
base-fingerprint: 444341ad48073521e98b9c88662429cb15c2889c
head: 3bc8201
head-fingerprint: 263791791f122733f874f9739b94c83cd96178ea
cases: 276
rounds: 2
reps: 3
date: 2026-11-18
-->

## Host comparison: green

**Accepted:** -

`3bc8201` against `8c7958f` (origin/main), 2026-11-18.

**VERDICT: no regression (0 major, 0 minor across 276 cases)**

Performance comparison: 0 major, 0 minor, 10 faster, 0 differ, 0 lost, 276 cases

- base: 8c7958f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3bc8201 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 331 ms | 332 ms | +0.2% | [-0.6%, +1.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 521 ms | 518 ms | -0.5% | [-0.5%, -0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 352 ms | 351 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 189 ms | 188 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 569 ms | 570 ms | +0.2% | [-1.3%, +1.7%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 44.9 ms | 42.6 ms | -4.9% | [-12.0%, +3.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 78.0 ms | 76.6 ms | -1.7% | [-3.4%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.9 ms | 57.1 ms | +0.3% | [-0.9%, +1.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 52.0 ms | 52.6 ms | +1.1% | [+0.6%, +1.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.4 ms | 65.9 ms | +0.8% | [-2.0%, +3.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 56.7 ms | 55.8 ms | -1.6% | [-5.7%, +2.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.7 ms | 61.1 ms | +0.6% | [+0.5%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 74.0 ms | 73.4 ms | -0.8% | [-3.7%, +2.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 72.3 ms | 72.5 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 79.3 ms | 78.4 ms | -1.1% | [-3.4%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.9 ms | 65.3 ms | +0.6% | [-0.6%, +1.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 101 ms | 101 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 77.3 ms | 77.3 ms | -0.1% | [-0.9%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 111 ms | 112 ms | +1.2% | [-0.6%, +3.0%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 67.4 ms | 64.4 ms | -4.4% | [-7.8%, -0.9%] | +0.0% | faster |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 98.2 ms | 97.7 ms | -0.4% | [-2.0%, +1.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 77.4 ms | 76.4 ms | -1.4% | [-3.6%, +0.9%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 102 ms | 104 ms | +2.2% | [+0.0%, +4.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.9 ms | 19.5 ms | -1.8% | [-2.9%, -0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.4 ms | 41.5 ms | +0.4% | [+0.4%, +0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 74.8 ms | 74.0 ms | -1.0% | [-2.1%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.3 ms | 20.1 ms | -1.3% | [-2.1%, -0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 44.1 ms | 43.9 ms | -0.4% | [-1.5%, +0.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.3 ms | 82.9 ms | +0.7% | [-0.4%, +1.8%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 143 ms | 140 ms | -1.9% | [-2.8%, -1.1%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 75.8 ms | 76.8 ms | +1.4% | [+0.4%, +2.3%] | +0.0% | ~ | surface/leaf 1.82x (was 1.88x) |
| `ClosestSurface/union/closestSurface/float/host` | 147 ms | 149 ms | +1.4% | [+1.4%, +1.5%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 78.0 ms | 77.2 ms | -1.0% | [-2.3%, +0.2%] | +0.0% | ~ | surface/leaf 1.93x |
| `ClosestSurface/cloud/closestSurface/double/host` | 159 ms | 158 ms | -1.1% | [-1.6%, -0.5%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 82.3 ms | 81.0 ms | -1.6% | [-2.7%, -0.4%] | +0.0% | ~ | surface/leaf 1.95x |
| `ClosestSurface/union/closestSurface/double/host` | 157 ms | 156 ms | -0.6% | [-3.5%, +2.5%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 82.3 ms | 81.4 ms | -1.1% | [-2.0%, -0.3%] | +0.0% | ~ | surface/leaf 1.92x |
| `Domain/particles/labels/float/host` | 13.6 ms | 13.6 ms | -0.0% | [-0.9%, +0.9%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.79 ms | 6.85 ms | +0.9% | [+0.0%, +1.8%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.36 ms | 1.37 ms | +0.9% | [-0.7%, +2.5%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.3 ms | 14.3 ms | -0.0% | [-1.5%, +1.5%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.28 ms | 7.24 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.37 ms | 1.37 ms | -0.3% | [-1.8%, +1.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 40.2 ms | 39.8 ms | -0.8% | [-2.5%, +1.0%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.4 ms | 21.7 ms | +1.4% | [-0.6%, +3.3%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 570 µs | 560 µs | -1.8% | [-3.9%, +0.4%] | +0.0% | ~ | own/whole 71.18x |
| `DomainObject/partsOwn/labelAt/float/host` | 305 µs | 305 µs | -0.2% | [-2.7%, +2.3%] | +0.0% | ~ | own/whole 71.13x |
| `DomainObject/partsWhole/labels/double/host` | 40.0 ms | 40.5 ms | +1.1% | [-0.9%, +3.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 23.0 ms | 22.7 ms | -1.0% | [-1.9%, -0.0%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 582 µs | 588 µs | +1.0% | [+0.2%, +1.9%] | +0.0% | ~ | own/whole 68.85x |
| `DomainObject/partsOwn/labelAt/double/host` | 315 µs | 315 µs | +0.1% | [-0.9%, +1.1%] | +0.0% | ~ | own/whole 72.08x |
| `Fused/csg/signedDistance/float/host` | 4.53 ms | 4.53 ms | +0.1% | [-2.8%, +3.1%] | - | ~ | tape/fused 2.94x |
| `Fused/smooth/signedDistance/float/host` | 12.4 ms | 12.3 ms | -0.4% | [-1.2%, +0.3%] | - | ~ | tape/fused 2.11x |
| `Fused/chain16/signedDistance/float/host` | 991 µs | 972 µs | -1.9% | [-3.6%, -0.2%] | - | ~ | tape/fused 17.29x |
| `Fused/drilled-mesh/signedDistance/float/host` | 48.0 ms | 46.9 ms | -2.2% | [-4.5%, +0.2%] | +0.0% | ~ | tape/fused 1.02x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 47.7 ms | 47.8 ms | +0.1% | [-3.5%, +3.7%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.58 ms | 4.52 ms | -1.2% | [-2.3%, +0.0%] | - | ~ | tape/fused 2.97x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | -0.1% | [-1.4%, +1.2%] | - | ~ | tape/fused 2.14x (was 2.26x) |
| `Fused/chain16/signedDistance/double/host` | 3.88 ms | 3.92 ms | +0.9% | [-0.6%, +2.4%] | - | ~ | tape/fused 4.34x |
| `Fused/drilled-mesh/signedDistance/double/host` | 54.1 ms | 54.0 ms | -0.2% | [-1.5%, +1.1%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 54.3 ms | 54.1 ms | -0.3% | [-3.1%, +2.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 83.8 ms | 83.3 ms | -0.6% | [-2.7%, +1.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 231 ms | 232 ms | +0.3% | [-0.8%, +1.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 114 ms | 114 ms | +0.1% | [-0.8%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 267 ms | 266 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 71.9 ms | 71.8 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 224 ms | 215 ms | -3.7% | [-4.9%, -2.5%] | +0.0% | faster |  |
| `MeshSDF/build/float/host` | 99.2 ms | 98.7 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.4 ms | 29.9 ms | +1.9% | [-2.6%, +6.5%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 776 ms | 784 ms | +1.0% | [-0.4%, +2.5%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 119 ms | 119 ms | +0.2% | [-0.5%, +1.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.3 ms | 44.8 ms | +1.0% | [-0.8%, +2.8%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 907 ms | 877 ms | -3.3% | [-6.9%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.3 ms | 26.0 ms | -1.0% | [-2.6%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.8 ms | 16.1 ms | +2.1% | [-0.7%, +5.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 322 ms | 321 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 236 ms | 232 ms | -1.9% | [-3.3%, -0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 91.0 ms | 91.8 ms | +0.9% | [+0.2%, +1.7%] | +0.0% | ~ | tape/fused 3.49x |
| `MeshCSG/smooth-fused/frame/float/host` | 82.2 ms | 81.8 ms | -0.5% | [-1.6%, +0.5%] | +0.0% | ~ | tape/fused 2.84x |
| `MeshCSG/union-tape/firstHit/double/host` | 28.9 ms | 28.4 ms | -2.0% | [-3.3%, -0.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 18.2 ms | 17.2 ms | -5.6% | [-11.8%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 365 ms | 365 ms | +0.1% | [-1.8%, +2.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 250 ms | 249 ms | -0.5% | [-0.9%, -0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 100 ms | 98.0 ms | -2.3% | [-3.8%, -0.7%] | +0.0% | ~ | tape/fused 3.72x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.9 ms | 88.0 ms | +0.1% | [-2.2%, +2.4%] | +0.0% | ~ | tape/fused 2.83x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 230 ms | 232 ms | +0.8% | [-0.7%, +2.3%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 239 ms | 237 ms | -0.8% | [-3.3%, +1.8%] | +0.0% | ~ | closestPoint/signedDistance 0.98x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 321 ms | 313 ms | -2.3% | [-5.6%, +1.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 240 ms | 234 ms | -2.3% | [-4.0%, -0.6%] | +0.0% | ~ | gradient/closestPoint 1.01x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 270 ms | 265 ms | -1.9% | [-3.5%, -0.2%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 269 ms | 270 ms | +0.1% | [-1.1%, +1.3%] | +0.0% | ~ | closestPoint/signedDistance 0.98x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 348 ms | 352 ms | +1.3% | [-0.9%, +3.5%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 272 ms | 268 ms | -1.4% | [-2.9%, +0.1%] | +0.0% | ~ | gradient/closestPoint 1.00x |
| `MeshTape/mesh/firstHit/float/host` | 47.1 ms | 47.0 ms | -0.4% | [-1.8%, +1.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 46.8 ms | 47.1 ms | +0.6% | [-0.5%, +1.7%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.3 ms | 19.1 ms | -1.0% | [-1.9%, -0.1%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 78.1 ms | 79.9 ms | +2.2% | [+1.3%, +3.1%] | +0.0% | ~ | tape/mesh 0.59x |
| `MeshTape/tape/firstCrossing/float/host` | 49.1 ms | 48.5 ms | -1.2% | [-2.5%, +0.2%] | +0.0% | ~ | tape/mesh 0.97x |
| `MeshTape/tape/frame/float/host` | 26.1 ms | 23.6 ms | -9.3% | [-14.2%, -3.9%] | +0.0% | faster | tape/mesh 0.81x (was 0.74x) |
| `MeshTape/view/firstHit/float/host` | 81.5 ms | 80.8 ms | -0.8% | [-1.5%, -0.1%] | +0.0% | ~ | view/mesh 0.58x |
| `MeshTape/view/frame/float/host` | 25.6 ms | 25.5 ms | -0.3% | [-0.4%, -0.3%] | +0.0% | ~ | view/mesh 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 53.2 ms | 53.4 ms | +0.4% | [-0.5%, +1.2%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 53.3 ms | 54.7 ms | +2.6% | [-1.8%, +6.9%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.2 ms | 20.5 ms | +1.8% | [+0.0%, +3.5%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 89.7 ms | 87.5 ms | -2.5% | [-3.7%, -1.2%] | +0.0% | ~ | tape/mesh 0.61x |
| `MeshTape/tape/firstCrossing/double/host` | 57.4 ms | 57.2 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ | tape/mesh 0.96x |
| `MeshTape/tape/frame/double/host` | 27.6 ms | 27.1 ms | -1.6% | [-5.1%, +2.1%] | +0.0% | ~ | tape/mesh 0.76x (was 0.73x) |
| `MeshTape/view/firstHit/double/host` | 92.0 ms | 90.3 ms | -1.9% | [-2.5%, -1.2%] | +0.0% | ~ | view/mesh 0.59x |
| `MeshTape/view/frame/double/host` | 28.9 ms | 28.8 ms | -0.4% | [-0.9%, +0.2%] | +0.0% | ~ | view/mesh 0.71x |
| `PointCloudBVH/uniform/build/float/host` | 54.0 ms | 53.7 ms | -0.7% | [-2.6%, +1.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 147 ms | 149 ms | +1.1% | [-1.8%, +4.0%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 369 ms | 375 ms | +1.7% | [-0.0%, +3.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 81.0 ms | 81.0 ms | +0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.77 ms | 6.74 ms | -0.4% | [-1.1%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 56.0 ms | 57.0 ms | +1.8% | [-0.7%, +4.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 167 ms | 158 ms | -5.7% | [-6.3%, -5.1%] | - | faster |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 403 ms | 406 ms | +0.7% | [-0.2%, +1.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 93.3 ms | 92.7 ms | -0.6% | [-10.6%, +10.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.06 ms | 7.09 ms | +0.4% | [-4.5%, +5.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 15.5 ms | 13.7 ms | -11.1% | [-19.7%, -0.7%] | +0.0% | faster |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 121 ms | 122 ms | +0.2% | [-2.2%, +2.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 486 ms | 488 ms | +0.5% | [-0.6%, +1.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 143 ms | 143 ms | +0.4% | [-0.5%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 815 ms | 806 ms | -1.1% | [-1.6%, -0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 16.3 ms | 15.3 ms | -6.1% | [-10.9%, -1.0%] | +0.0% | faster |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 122 ms | 123 ms | +0.7% | [-2.7%, +4.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 503 ms | 505 ms | +0.3% | [-1.3%, +1.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 147 ms | 148 ms | +1.0% | [-0.8%, +2.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 785 ms | 797 ms | +1.5% | [+0.9%, +2.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 55.1 ms | 55.4 ms | +0.5% | [-2.6%, +3.6%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 133 ms | 134 ms | +0.9% | [-1.0%, +2.9%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 285 ms | 283 ms | -0.7% | [-6.9%, +5.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 152 ms | 151 ms | -0.8% | [-1.6%, +0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 137 ms | 137 ms | -0.1% | [-0.9%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.8 ms | 58.2 ms | +0.8% | [-0.2%, +1.8%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 148 ms | 149 ms | +0.7% | [-3.5%, +5.1%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 304 ms | 302 ms | -0.5% | [-1.1%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 166 ms | 165 ms | -0.5% | [-1.7%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 151 ms | 151 ms | -0.5% | [-3.5%, +2.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 12.0 ms | 11.8 ms | -2.0% | [-5.5%, +1.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 117 ms | 117 ms | -0.7% | [-3.9%, +2.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 321 ms | 324 ms | +1.1% | [-0.8%, +3.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 157 ms | 157 ms | -0.2% | [-0.4%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 324 ms | 324 ms | +0.1% | [-0.4%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 16.1 ms | 13.9 ms | -14.0% | [-25.1%, +0.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 117 ms | 117 ms | -0.0% | [-1.2%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 353 ms | 352 ms | -0.3% | [-0.8%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 167 ms | 169 ms | +1.2% | [+0.6%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 314 ms | 313 ms | -0.2% | [-0.4%, +0.0%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.4 ms | 35.4 ms | +0.0% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.4 ms | 38.3 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 47.0 ms | 46.9 ms | -0.2% | [-1.1%, +0.6%] | +0.0% | ~ | hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled/float/host` | 46.2 ms | 46.3 ms | +0.2% | [-1.1%, +1.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.5 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 80.3 ms | 80.0 ms | -0.3% | [-3.2%, +2.5%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 66.7 ms | 66.7 ms | -0.1% | [-2.3%, +2.2%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.30 ms | 5.21 ms | -1.6% | [-3.3%, +0.2%] | - | ~ | tape/fused 1.46x |
| `Ray/csg-tape/firstHit/float/host` | 7.75 ms | 7.62 ms | -1.7% | [-3.7%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 54.6 ms | 53.0 ms | -2.9% | [-4.1%, -1.7%] | +0.0% | ~ | hand/api 1.00x (was 0.95x) |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.9 ms | 52.7 ms | +1.7% | [-0.0%, +3.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.7 ms | 45.6 ms | -0.4% | [-2.0%, +1.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 90.8 ms | 90.3 ms | -0.5% | [-1.7%, +0.6%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/double/host` | 87.0 ms | 87.8 ms | +0.9% | [-1.3%, +7.9%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.57 ms | 5.56 ms | -0.2% | [-2.1%, +1.7%] | - | ~ | tape/fused 1.42x |
| `Ray/csg-tape/firstHit/double/host` | 8.01 ms | 7.89 ms | -1.5% | [-2.7%, -0.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.9 ms | 17.6 ms | -1.4% | [-2.2%, -0.5%] | - | ~ | marched/traced 6.81x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 122 ms | 120 ms | -1.9% | [-3.9%, +0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.2 ms | 21.9 ms | -1.1% | [-1.4%, -0.9%] | +0.0% | ~ | marched/traced 6.68x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 152 ms | 146 ms | -3.8% | [-6.3%, -1.2%] | +0.0% | faster |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 36.4 ms | 36.3 ms | -0.2% | [-2.3%, +1.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.4 ms | 19.2 ms | -1.1% | [-2.7%, +0.6%] | - | ~ | marched/traced 6.71x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 130 ms | 129 ms | -1.1% | [-3.2%, +1.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.7 ms | +1.3% | [+0.5%, +2.1%] | +0.0% | ~ | marched/traced 6.34x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 152 ms | 150 ms | -1.0% | [-1.2%, -0.7%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.0 ms | 38.6 ms | +1.7% | [-0.3%, +3.8%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.8 ms | 46.9 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.8 ms | 46.9 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.66 ms | 7.74 ms | +1.0% | [-0.9%, +3.0%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.33 ms | 6.13 ms | -3.2% | [-7.8%, +1.9%] | +0.0% | ~ | hit/crossing 1.26x (was 1.21x) |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 178 ms | 179 ms | +0.6% | [-0.4%, +1.7%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 158 ms | 158 ms | +0.3% | [-0.6%, +1.1%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 69.8 ms | 69.4 ms | -0.6% | [-1.3%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 63.4 ms | 57.2 ms | -9.7% | [-17.7%, -0.0%] | +0.0% | faster | hit/crossing 1.21x (was 1.10x) |
| `RayCrossing/mesh/firstHit/double/host` | 54.8 ms | 53.1 ms | -3.0% | [-5.7%, -0.2%] | +0.0% | faster |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 58.9 ms | 53.6 ms | -9.0% | [-16.9%, +0.5%] | +0.0% | ~ | hit/crossing 0.99x (was 0.93x) |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.97 ms | 7.91 ms | -0.7% | [-1.3%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.29 ms | 6.30 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 206 ms | 221 ms | +7.6% | [-3.2%, +18.7%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 187 ms | 191 ms | +1.9% | [-3.3%, +7.4%] | +0.0% | ~ | hit/crossing 1.16x (was 1.10x) |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 79.8 ms | 79.2 ms | -0.8% | [-1.4%, -0.2%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 66.6 ms | 65.5 ms | -1.6% | [-3.1%, -0.2%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.2 ms | 11.3 ms | +0.4% | [-0.5%, +1.2%] | +0.0% | ~ | marched/traced 4.75x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 53.1 ms | 53.6 ms | +0.9% | [-2.7%, +4.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.02 ms | 8.13 ms | +1.3% | [+0.1%, +2.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.94 ms | 1.93 ms | -1.0% | [-2.1%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.83 ms | 6.88 ms | +0.8% | [-0.5%, +2.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.8 ms | 11.8 ms | -0.3% | [-1.7%, +1.1%] | +0.0% | ~ | marched/traced 5.31x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.4 ms | 62.5 ms | +1.8% | [+0.5%, +3.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.79 ms | 8.76 ms | -0.4% | [-2.5%, +1.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.97 ms | 1.92 ms | -2.5% | [-6.7%, +2.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.16 ms | 7.20 ms | +0.7% | [+0.3%, +1.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 241 ms | 241 ms | +0.3% | [-0.0%, +0.7%] | +0.0% | ~ | marched/traced 1.41x (was 1.54x) |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 370 ms | 341 ms | -8.0% | [-20.2%, +7.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 247 ms | 249 ms | +0.5% | [-6.1%, +7.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 268 ms | 268 ms | -0.0% | [-7.4%, +8.1%] | +0.0% | ~ | marched/traced 1.64x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 438 ms | 439 ms | +0.2% | [-24.4%, +33.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 270 ms | 272 ms | +0.6% | [-11.3%, +14.0%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.59 ms | 7.57 ms | -0.3% | [-6.2%, +5.9%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 13.1 ms | 13.0 ms | -0.7% | [-6.4%, +5.3%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 278 ms | 281 ms | +1.4% | [-4.5%, +7.7%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.9 ms | 13.0 ms | +0.9% | [-3.7%, +5.9%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 32.0 ms | 31.9 ms | -0.4% | [-6.1%, +5.6%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 23.1 ms | 23.1 ms | +0.1% | [-6.1%, +6.6%] | +0.0% | ~ | regions/hit 1.38x |
| `Regions/particles/region/double/host` | 7.90 ms | 7.89 ms | -0.2% | [-6.0%, +6.0%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 13.4 ms | 13.4 ms | +0.0% | [-5.8%, +6.1%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 297 ms | 299 ms | +0.4% | [-4.7%, +5.9%] | +0.0% | ~ | each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/host` | 13.4 ms | 13.4 ms | -0.1% | [-6.1%, +6.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 33.8 ms | 33.9 ms | +0.1% | [-5.7%, +6.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 24.7 ms | 25.4 ms | +2.6% | [-4.5%, +10.2%] | +0.0% | ~ | regions/hit 1.33x |
| `Render/csg/frame/float/host` | 14.3 ms | 13.2 ms | -7.1% | [-11.7%, -2.3%] | +0.0% | faster |  |
| `Render/csg/frame-orthographic/float/host` | 13.2 ms | 13.5 ms | +1.9% | [-2.5%, +6.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.4 ms | 20.2 ms | -0.8% | [-10.3%, +9.5%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.3 ms | 14.6 ms | +2.1% | [-3.0%, +7.5%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.4 ms | 14.4 ms | +0.1% | [-5.5%, +6.0%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 22.0 ms | 21.6 ms | -1.9% | [-14.3%, +12.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 61.7 ms | 62.0 ms | +0.5% | [-15.6%, +19.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 39.6 ms | 39.4 ms | -0.5% | [-15.8%, +17.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 107 ms | 108 ms | +1.6% | [-23.5%, +34.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 67.0 ms | 67.0 ms | -0.0% | [-8.2%, +8.8%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 44.2 ms | 44.6 ms | +0.9% | [-8.7%, +11.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 120 ms | 120 ms | +0.4% | [-19.4%, +25.2%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 81.7 ms | 82.0 ms | +0.4% | [-15.1%, +18.7%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 311 ms | 327 ms | +5.3% | [-7.9%, +19.5%] | +0.0% | ~ | tape/fused 0.94x (was 0.99x) |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 309 ms | 308 ms | -0.4% | [-7.6%, +7.3%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 92.7 ms | 92.8 ms | +0.1% | [-11.9%, +13.7%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 367 ms | 367 ms | -0.2% | [-11.3%, +12.4%] | +0.0% | ~ | tape/fused 1.02x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 371 ms | 375 ms | +0.8% | [-9.0%, +11.6%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 173 ms | 173 ms | -0.3% | [-3.9%, +3.4%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 208 ms | 206 ms | -0.9% | [-6.9%, +5.6%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 147 ms | 145 ms | -1.3% | [-2.1%, -0.6%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 91.3 ms | 91.1 ms | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 133 ms | 133 ms | -0.2% | [-4.2%, +4.0%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 80.9 ms | 78.7 ms | -2.6% | [-15.7%, +12.8%] | +0.0% | ~ | surface/leaf 1.69x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 67.4 ms | 69.3 ms | +2.9% | [-11.7%, +19.9%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 38.8 ms | 39.3 ms | +1.2% | [-19.3%, +26.8%] | +0.0% | ~ | surface/hit 1.76x |
| `SurfaceParts/boxes/closestSurface/double/host` | 148 ms | 149 ms | +0.5% | [-2.6%, +3.8%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 104 ms | 97.9 ms | -5.9% | [-21.6%, +13.7%] | +0.0% | ~ | surface/leaf 1.52x (was 1.43x) |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 167 ms | 168 ms | +0.6% | [-22.0%, +29.4%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 99.2 ms | 96.2 ms | -3.0% | [-29.4%, +34.7%] | +0.0% | ~ | surface/leaf 1.75x (was 1.69x) |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 79.0 ms | 79.3 ms | +0.4% | [-22.9%, +30.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 45.0 ms | 44.6 ms | -0.8% | [-25.2%, +31.5%] | +0.0% | ~ | surface/hit 1.78x |
| `Tape/csg/signedDistance/float/host` | 13.4 ms | 13.3 ms | -0.5% | [-7.0%, +6.5%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.73 ms | 4.71 ms | -0.2% | [-6.5%, +6.4%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 26.3 ms | 26.0 ms | -1.3% | [-8.7%, +6.8%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 41.9 ms | 42.1 ms | +0.4% | [-6.2%, +7.5%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.80 ms | 1.82 ms | +1.0% | [-4.2%, +6.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 134 µs | 142 µs | +5.4% | [-2.1%, +10.8%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 31.0 ms | 31.1 ms | +0.2% | [-4.0%, +4.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.79 ms | 5.79 ms | +0.1% | [-3.5%, +3.8%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.7 ms | 16.8 ms | +0.3% | [-5.7%, +6.7%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.75 ms | 4.76 ms | +0.1% | [-3.7%, +4.0%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.7 ms | 13.4 ms | -1.9% | [-6.8%, +3.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.74 ms | 4.74 ms | -0.1% | [-6.9%, +7.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 29.0 ms | 27.4 ms | -5.7% | [-16.0%, +6.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 44.0 ms | 43.4 ms | -1.3% | [-9.7%, +7.9%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.83 ms | 1.88 ms | +2.5% | [-6.3%, +11.5%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 351 µs | 311 µs | -11.5% | [-32.1%, +22.2%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 31.4 ms | 31.1 ms | -1.1% | [-8.3%, +6.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.94 ms | 7.98 ms | +0.5% | [-1.8%, +2.9%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 17.2 ms | 17.0 ms | -1.0% | [-5.5%, +3.7%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.91 ms | 8.01 ms | +1.3% | [-3.1%, +5.8%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 44.5 ms | 43.8 ms | -1.5% | [-6.3%, +3.5%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 212 ms | 211 ms | -0.1% | [-5.7%, +5.9%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 46.1 ms | 45.6 ms | -1.2% | [-4.1%, +1.8%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 224 ms | 226 ms | +0.9% | [-0.3%, +2.2%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 684.3 s → 718.7 s (+5.0%), the largest peak 1399 MB → 1410 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.5 s | +4.7% | 557 MB | 558 MB | +0.0% | ~ |
| `BVHRefit` | 3.9 s | 4.3 s | +10.1% | 484 MB | 497 MB | +2.6% | ~ |
| `BVHUnion` | 17.8 s | 19.4 s | +9.2% | 897 MB | 948 MB | +5.7% | ~ |
| `BVHUnionMixed` | 7.3 s | 7.9 s | +8.7% | 555 MB | 556 MB | +0.1% | ~ |
| `ClosestPrimitive` | 28.9 s | 30.3 s | +5.0% | 1297 MB | 1297 MB | +0.0% | ~ |
| `ClosestSurface` | 27.4 s | 29.0 s | +5.9% | 1220 MB | 1228 MB | +0.7% | ~ |
| `Domain` | 17.5 s | 18.1 s | +3.9% | 841 MB | 842 MB | +0.2% | ~ |
| `DomainObject` | 15.4 s | 16.1 s | +4.3% | 844 MB | 846 MB | +0.2% | ~ |
| `Fused` | 23.3 s | 25.0 s | +7.2% | 1127 MB | 1136 MB | +0.9% | ~ |
| `LaunchTuning` | 34.8 s | 37.3 s | +7.1% | 1304 MB | 1311 MB | +0.5% | ~ |
| `Mesh` | 14.3 s | 16.0 s | +12.2% | 784 MB | 784 MB | -0.1% | ~ |
| `MeshCSG` | 37.7 s | 39.5 s | +4.9% | 1255 MB | 1256 MB | +0.0% | ~ |
| `MeshClosestPoint` | 12.9 s | 14.2 s | +10.1% | 780 MB | 782 MB | +0.2% | ~ |
| `MeshReconcile` | 6.4 s | 6.5 s | +2.3% | 565 MB | 566 MB | +0.1% | ~ |
| `MeshTape` | 39.8 s | 42.8 s | +7.6% | 1287 MB | 1288 MB | +0.1% | ~ |
| `PointCloud` | 7.1 s | 7.5 s | +5.6% | 558 MB | 558 MB | -0.0% | ~ |
| `PointCloudBVHBuild` | 3.6 s | 3.9 s | +8.0% | 470 MB | 471 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 3.2 s | 3.3 s | +5.2% | 452 MB | 452 MB | +0.1% | ~ |
| `Ray` | 34.9 s | 36.7 s | +5.1% | 1318 MB | 1318 MB | -0.0% | ~ |
| `RayCSG` | 31.1 s | 32.4 s | +4.1% | 1183 MB | 1189 MB | +0.5% | ~ |
| `RayCrossing` | 44.6 s | 46.1 s | +3.3% | 1329 MB | 1331 MB | +0.2% | ~ |
| `RayParticles` | 47.4 s | 50.1 s | +5.8% | 1317 MB | 1320 MB | +0.2% | ~ |
| `RayShell` | 36.9 s | 37.7 s | +2.3% | 1280 MB | 1286 MB | +0.4% | ~ |
| `Regions` | 28.8 s | 30.2 s | +5.0% | 1070 MB | 1073 MB | +0.3% | ~ |
| `Render` | 34.6 s | 35.6 s | +3.0% | 1281 MB | 1285 MB | +0.3% | ~ |
| `Shapes` | 8.7 s | 8.9 s | +1.6% | 554 MB | 554 MB | -0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.9 s | +2.6% | 511 MB | 511 MB | +0.0% | ~ |
| `SphereCloudRays` | 27.5 s | 27.8 s | +1.4% | 1062 MB | 1064 MB | +0.2% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | +0.4% | 484 MB | 485 MB | +0.0% | ~ |
| `SurfaceParts` | 39.7 s | 41.3 s | +3.9% | 1399 MB | 1410 MB | +0.8% | ~ |
| `Tape` | 16.3 s | 17.1 s | +4.6% | 819 MB | 830 MB | +1.4% | ~ |
| `TapeBVHUnion` | 14.5 s | 14.8 s | +2.1% | 823 MB | 818 MB | -0.6% | ~ |
| **all, in both** | 684.3 s | 718.7 s | +5.0% | 1399 MB | 1410 MB | +0.8% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): Ray/mesh-polygons/intersects/double/host, Tape/sphere-handwritten/signedDistance/float/host

These probes compiled to the same code on both sides and were not run (26 cases): MeshReconcile (4), PointCloudHashGridBuild (2), Shapes (20)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 8c7958f
base-fingerprint: 444341ad48073521e98b9c88662429cb15c2889c
head: 3bc8201
head-fingerprint: 263791791f122733f874f9739b94c83cd96178ea
cases: 370
rounds: 2
reps: 3
date: 2026-11-18
-->

## Device comparison (CUDA): red

**Accepted:** The one-pixel DIFFER is the host/device shading difference of issue #343, also on main.

`3bc8201` against `8c7958f` (origin/main), 2026-11-18.

**VERDICT: host and device answers differ in 4 cases; investigate before merging (see Flagged above)**

Performance comparison: 0 major, 0 minor, 18 faster, 4 differ, 0 lost, 370 cases

- base: 8c7958f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3bc8201 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.54 ms | 1.51 ms | -1.5% | [-14.0%, +13.0%] | - | ~ | device 24.2x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 4.16 ms | 3.24 ms | -22.1% | [-29.6%, -13.9%] | - | faster | device 19.7x faster than the host (was 15.5x) |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.08 ms | +2.0% | [+1.5%, +2.5%] | - | ~ | device 19.1x faster than the host (was 19.7x) |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.33 ms | 3.22 ms | +38.2% | [-2.3%, +79.0%] | - | ~ | device 13.1x faster than the host (was 16.6x) |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 9.03 ms | 6.62 ms | -26.7% | [-47.0%, +10.9%] | - | ~ | device 6.8x faster than the host (was 5.5x) |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 12.4 ms | 13.2 ms | +6.0% | [-0.5%, +13.0%] | - | ~ | device 5.0x faster than the host (was 5.2x) |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 37.0 ms | 32.0 ms | -13.5% | [-30.2%, +8.8%] | - | ~ | device 1.2x faster than the host (was 1.0x) |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 50.6 ms | 50.4 ms | -0.4% | [-1.0%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 17.6 ms | 18.2 ms | +3.6% | [-5.4%, +13.0%] | - | ~ | device 1.2x faster than the host (was 1.2x) |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 41.9 ms | 43.3 ms | +3.4% | [-3.6%, +11.2%] | - | ~ | device 0.9x faster than the host (was 0.9x) |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 52.7 ms | 54.5 ms | +3.5% | [-0.2%, +7.5%] | - | ~ | device 0.8x faster than the host (was 0.9x) |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 103 ms | -7.1% | [-16.8%, +3.4%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.85 ms | 9.25 ms | +4.5% | [-11.0%, +22.3%] | +0.0% | ~ | device 5.0x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.86 ms | 9.94 ms | +0.9% | [-1.5%, +8.1%] | +0.0% | ~ | device 8.1x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 388 µs | 390 µs | +0.3% | [-0.5%, +1.2%] | - | ~ | device 15.1x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 844 µs | 843 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.95 ms | 4.91 ms | -0.8% | [-15.6%, +16.7%] | - | ~ | device 1.2x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 5.42 ms | 5.44 ms | +0.4% | [+0.0%, +0.7%] | - | ~ | device 1.6x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 1.02 ms | 946 µs | -7.1% | [-59.2%, +122.6%] | - | ~ | device 9.4x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 2.15 ms | 1.45 ms | -32.4% | [-46.9%, -10.4%] | - | faster | device 7.9x faster than the host (was 5.7x) |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.76 ms | 6.77 ms | +0.1% | [-0.0%, +6.5%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 8.47 ms | 7.59 ms | -10.4% | [-19.2%, -0.3%] | - | faster | device 1.6x faster than the host (was 1.4x) |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 909 µs | 951 µs | +4.6% | [-0.2%, +9.6%] | - | ~ | device 13.7x faster than the host (was 14.3x) |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.66 ms | 2.31 ms | -13.1% | [-23.7%, +0.3%] | - | ~ | device 7.7x faster than the host (was 6.9x) |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.49 ms | 9.47 ms | -0.2% | [-1.2%, +9.9%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.5 ms | 10.5 ms | -0.1% | [-9.1%, +9.9%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 861 µs | 858 µs | -0.4% | [-1.2%, +0.5%] | - | ~ | device 14.6x faster than the host (was 13.5x) |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.32 ms | 2.19 ms | -5.9% | [-12.5%, +1.7%] | - | ~ | device 7.6x faster than the host (was 7.2x) |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 11.1 ms | 10.4 ms | -6.9% | [-19.2%, +8.4%] | - | ~ | device 1.1x faster than the host (was 1.0x) |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 11.7 ms | 10.7 ms | -8.2% | [-27.2%, +17.7%] | - | ~ | device 1.6x faster than the host (was 1.5x) |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 313 µs | 537 µs | +71.4% | [-1.1%, +144.3%] | - | ~ | device 8.4x faster than the host (was 11.6x) |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 6.4x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 63.3 ms | 68.9 ms | +8.9% | [-10.9%, +34.0%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.08 ms | 1.07 ms | -0.6% | [-2.0%, +0.8%] | - | ~ | device 7.4x faster than the host (was 3.5x) |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 4.43 ms | 4.76 ms | +7.4% | [-0.0%, +15.0%] | - | ~ | device 1.9x faster than the host (was 2.0x) |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 603 ms | 619 ms | +2.6% | [-15.0%, +24.7%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.6 ms | 20.3 ms | -1.4% | [-14.0%, +12.8%] | - | ~ | device 4.5x faster than the host (was 5.8x) |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.81 ms | 7.53 ms | -14.5% | [-25.0%, -2.8%] | - | faster | device 6.0x faster than the host (was 5.2x); surface/leaf 2.69x (was 2.33x) |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 16.6 ms | 17.3 ms | +4.4% | [-14.0%, +25.1%] | - | ~ | device 8.2x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.24 ms | 6.20 ms | -0.6% | [-1.2%, +0.1%] | - | ~ | device 10.6x faster than the host; surface/leaf 2.79x (was 2.66x) |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 149 ms | 147 ms | -1.7% | [-18.6%, +18.9%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 55.9 ms | 56.0 ms | +0.1% | [-18.8%, +23.8%] | - | ~ | device 0.8x faster than the host (was 0.9x); surface/leaf 2.62x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 69.1 ms | 67.3 ms | -2.7% | [-20.0%, +18.2%] | - | ~ | device 2.6x faster than the host (was 2.1x) |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 25.6 ms | 27.4 ms | +6.9% | [-5.2%, +21.3%] | - | ~ | device 2.7x faster than the host (was 2.6x); surface/leaf 2.46x (was 2.70x) |
| `Domain/particles/labels/float/hostdevice` | 3.68 ms | 4.13 ms | +12.4% | [-2.8%, +28.3%] | - | ~ | device 3.6x faster than the host (was 4.1x) |
| `Domain/particles/labelAt/float/hostdevice` | 2.47 ms | 2.49 ms | +0.9% | [-15.1%, +19.7%] | - | ~ | device 3.0x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 118 µs | 122 µs | +3.3% | [-1.2%, +8.1%] | - | ~ | device 12.2x faster than the host (was 12.7x) |
| `Domain/particles/labels/double/hostdevice` | 7.92 ms | 7.91 ms | -0.0% | [-17.5%, +21.3%] | - | ~ | device 1.8x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.87 ms | 4.02 ms | +3.8% | [-13.7%, +24.3%] | - | ~ | device 1.8x faster than the host (was 1.9x) |
| `Domain/particles/surfaceGroups/double/hostdevice` | 133 µs | 130 µs | -2.9% | [-6.0%, +0.3%] | - | ~ | device 10.7x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 14.2 ms | 15.9 ms | +11.8% | [-10.0%, +40.2%] | - | ~ | device 2.9x faster than the host (was 3.3x) |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 9.63 ms | 10.0 ms | +4.1% | [-18.4%, +28.5%] | - | ~ | device 2.5x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 292 µs | 845 µs | +188.9% | [-14.8%, +426.3%] | - | ~ | device 1.5x faster than the host (was 2.3x); own/whole 18.85x (was 48.70x) |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 147 µs | 147 µs | +0.0% | [-1.6%, +1.7%] | - | ~ | device 2.5x faster than the host; own/whole 68.15x (was 65.48x) |
| `DomainObject/partsWhole/labels/double/hostdevice` | 24.7 ms | 22.9 ms | -7.2% | [-26.2%, +12.7%] | - | ~ | device 2.2x faster than the host (was 2.0x) |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 13.5 ms | 12.7 ms | -5.7% | [-21.6%, +10.7%] | - | ~ | device 2.0x faster than the host (was 1.9x) |
| `DomainObject/partsOwn/labels/double/hostdevice` | 356 µs | 792 µs | +122.3% | [-1.0%, +248.8%] | - | ~ | device 1.3x faster than the host (was 2.0x); own/whole 28.90x (was 69.21x) |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 470 µs | 220 µs | -53.2% | [-71.2%, +11.7%] | - | ~ | device 1.8x faster than the host (was 1.3x); own/whole 57.66x (was 28.65x) |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.2% | [-0.3%, +2.3%] | - | ~ | device 23.5x faster than the host; tape/fused 3.84x |
| `Fused/smooth/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.2% | [-0.6%, +1.0%] | - | ~ | device 45.7x faster than the host (was 47.9x); tape/fused 6.27x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.8 µs | 48.6 µs | -0.4% | [-1.4%, +0.7%] | - | ~ | device 9.1x faster than the host (was 8.6x); tape/fused 9.79x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 22.2 ms | 21.4 ms | -3.7% | [-27.3%, +26.8%] | - | ~ | device 5.7x faster than the host (was 5.5x); tape/fused 1.57x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 34.3 ms | 33.6 ms | -2.1% | [-20.8%, +20.4%] | - | ~ | device 3.6x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 670 µs | 670 µs | +0.0% | [-0.3%, +0.4%] | - | ~ | device 1.9x faster than the host (was 1.8x); tape/fused 1.28x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.77 ms | 1.35 ms | -23.5% | [-42.9%, +10.4%] | - | ~ | device 1.8x faster than the host (was 1.5x); tape/fused 1.28x (was 0.98x) |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-6.0%, +9.4%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 300 ms | 301 ms | +0.3% | [-21.9%, +29.0%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 301 ms | 303 ms | +0.8% | [-22.1%, +30.2%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 9.13 ms | 8.87 ms | -2.9% | [-24.7%, +23.5%] | +0.0% | ~ | device 10.3x faster than the host (was 9.9x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 10.7 ms | 9.39 ms | -11.9% | [-36.1%, +25.6%] | +0.0% | ~ | device 9.6x faster than the host (was 8.8x); default/this 0.94x (was 0.86x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 9.16 ms | 9.24 ms | +1.0% | [-18.3%, +24.3%] | +0.0% | ~ | device 9.7x faster than the host; default/this 0.96x (was 1.00x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.92 ms | 9.34 ms | +4.6% | [-14.8%, +27.3%] | +0.0% | ~ | device 10.6x faster than the host (was 10.2x); default/this 0.95x (was 1.02x) |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 718 µs | 705 µs | -1.9% | [-4.1%, +0.3%] | +0.0% | ~ | device 3.6x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.87 ms | 1.91 ms | +2.3% | [-6.7%, +11.9%] | +0.0% | ~ | device 1.4x faster than the host (was 1.4x); default/this 0.37x (was 0.38x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.13 ms | 1.06 ms | -5.5% | [-9.7%, -1.1%] | +0.0% | faster | device 2.4x faster than the host; default/this 0.66x (was 0.64x) |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 1.29 ms | 796 µs | -38.2% | [-56.8%, +3.8%] | +0.0% | ~ | device 3.2x faster than the host (was 2.3x); default/this 0.88x (was 0.56x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 729 µs | 729 µs | +0.0% | [-5.7%, +6.0%] | +0.0% | ~ | device 12.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 851 µs | 834 µs | -2.0% | [-4.2%, +0.3%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 739 µs | 739 µs | +0.1% | [-1.9%, +2.0%] | +0.0% | ~ | device 12.1x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 730 µs | 734 µs | +0.6% | [-0.4%, +1.7%] | +0.0% | ~ | device 12.0x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 155 ms | 154 ms | -0.7% | [-22.0%, +25.8%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 155 ms | 156 ms | +0.8% | [-20.4%, +27.6%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x); default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 159 ms | 163 ms | +2.1% | [-17.4%, +26.5%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x); default/this 0.95x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 159 ms | 164 ms | +2.8% | [-11.7%, +19.7%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x); default/this 0.94x (was 0.97x) |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.31 ms | 1.28 ms | -2.8% | [-10.7%, +5.9%] | +0.0% | ~ | device 2.1x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 3.49 ms | 3.79 ms | +8.7% | [-21.4%, +41.5%] | +0.0% | ~ | device 0.7x faster than the host (was 0.8x); default/this 0.34x (was 0.38x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 2.20 ms | 2.12 ms | -3.6% | [-19.2%, +15.4%] | +0.0% | ~ | device 1.3x faster than the host (was 1.2x); default/this 0.60x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.58 ms | +1.5% | [+0.4%, +3.6%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.81x (was 0.84x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.19 ms | 6.96 ms | -3.1% | [-7.0%, +0.8%] | +0.0% | ~ | device 1.3x faster than the host (was 1.3x) |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 8.27 ms | 8.22 ms | -0.5% | [-20.6%, +25.0%] | +0.0% | ~ | device 1.2x faster than the host (was 1.1x); default/this 0.85x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.87 ms | 7.72 ms | -1.9% | [-11.5%, +8.5%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.74 ms | 7.59 ms | -1.9% | [-9.6%, +6.5%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.92x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.99 ms | 9.49 ms | +5.6% | [-11.5%, +24.8%] | - | ~ | device 10.9x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 19.5 ms | 19.3 ms | -0.6% | [-13.8%, +14.2%] | - | ~ | device 6.3x faster than the host (was 6.9x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 158 ms | 160 ms | +1.6% | [-11.9%, +17.4%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 301 ms | 307 ms | +1.8% | [-11.9%, +18.1%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `MeshSDF/query-morton/float/hostdevice` | 27.3 ms | 27.2 ms | -0.4% | [-19.5%, +23.4%] | - | ~ | device 4.8x faster than the host (was 5.1x) |
| `MeshSDF/query-random/float/hostdevice` | 70.9 ms | 70.1 ms | -1.1% | [-17.6%, +18.5%] | - | ~ | device 2.7x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 159 ms | 157 ms | -1.4% | [-15.6%, +14.6%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `MeshSDF/query-random/double/hostdevice` | 252 ms | 252 ms | +0.2% | [-17.9%, +22.5%] | - | ~ | device 0.9x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 1.95 ms | 1.90 ms | -2.6% | [-25.2%, +27.5%] | - | ~ | device 7.0x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.84 ms | 3.75 ms | -2.3% | [-17.4%, +14.3%] | - | ~ | device 4.8x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.67 ms | 6.71 ms | +0.6% | [-17.5%, +22.7%] | - | ~ | device 5.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 67.9 ms | 70.3 ms | +3.5% | [-9.3%, +19.5%] | - | ~ | device 3.7x faster than the host (was 3.9x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 97.8 ms | 95.3 ms | -2.6% | [-17.5%, +13.9%] | - | ~ | device 2.9x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 255 ms | 250 ms | -1.8% | [-12.5%, +10.0%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 26.6 ms | 25.0 ms | -6.1% | [-18.0%, +9.3%] | - | ~ | device 2.1x faster than the host (was 2.3x); tape/fused 2.81x (was 2.55x) |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 28.5 ms | 27.9 ms | -2.2% | [-4.5%, +0.1%] | - | ~ | device 2.2x faster than the host; tape/fused 3.42x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 66.1 ms | 72.0 ms | +8.9% | [-11.2%, +31.2%] | - | ~ | device 3.8x faster than the host (was 4.4x); tape/fused 3.48x (was 3.86x) |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 18.9 ms | -2.5% | [-12.9%, +8.6%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 34.3 ms | 34.6 ms | +0.8% | [-12.7%, +16.7%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 40.5 ms | 39.8 ms | -1.9% | [-3.5%, -0.2%] | - | ~ | device 0.8x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 626 ms | 612 ms | -2.2% | [-18.8%, +18.2%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 958 ms | 947 ms | -1.2% | [-19.8%, +21.6%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.21 s | 2.20 s | -0.5% | [-18.2%, +20.7%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 193 ms | 198 ms | +2.5% | [-16.9%, +27.0%] | - | ~ | device 0.3x faster than the host; tape/fused 3.10x (was 3.25x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 316 ms | 319 ms | +0.8% | [-17.9%, +23.7%] | - | ~ | device 0.2x faster than the host (was 0.2x); tape/fused 2.97x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 624 ms | 596 ms | -4.5% | [-23.9%, +19.1%] | - | ~ | device 0.5x faster than the host (was 0.4x); tape/fused 3.69x (was 3.55x) |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 14.0 ms | 13.6 ms | -2.4% | [-21.6%, +21.5%] | - | ~ | device 11.7x faster than the host (was 9.1x) |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 68.6 ms | 68.6 ms | +0.0% | [-21.7%, +27.7%] | - | ~ | device 2.3x faster than the host (was 2.8x) |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 271 ms | 270 ms | -0.1% | [-19.1%, +23.5%] | - | ~ | device 0.5x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 256 ms | 253 ms | -1.1% | [-18.8%, +20.5%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.14 ms | 1.21 ms | +6.1% | [-0.0%, +12.2%] | - | ~ | device 16.5x faster than the host (was 17.6x) |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.37 ms | 1.28 ms | -6.5% | [-12.7%, +0.7%] | - | ~ | device 15.2x faster than the host (was 14.4x) |
| `MeshTape/mesh/frame/float/hostdevice` | 2.10 ms | 2.34 ms | +11.8% | [+0.3%, +23.4%] | - | **DIFFER** | device 7.2x faster than the host (was 8.0x); 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/tape/firstHit/float/hostdevice` | 4.55 ms | 3.52 ms | -22.8% | [-42.2%, +10.6%] | - | ~ | device 9.8x faster than the host (was 8.1x); tape/mesh 0.34x (was 0.25x) |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.85 ms | 2.84 ms | -0.1% | [-12.0%, +13.4%] | - | ~ | device 7.4x faster than the host; tape/mesh 0.45x (was 0.48x) |
| `MeshTape/tape/frame/float/hostdevice` | 4.88 ms | 4.93 ms | +1.1% | [-21.1%, +29.1%] | - | **DIFFER** | device 4.5x faster than the host; tape/mesh 0.48x (was 0.43x); 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/view/firstHit/float/hostdevice` | 3.96 ms | 3.68 ms | -7.0% | [-18.8%, +7.4%] | - | ~ | device 9.8x faster than the host (was 9.0x); view/mesh 0.33x (was 0.29x) |
| `MeshTape/view/frame/float/hostdevice` | 6.09 ms | 6.12 ms | +0.5% | [-18.0%, +22.9%] | - | **DIFFER** | device 3.9x faster than the host; view/mesh 0.38x (was 0.34x); 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/mesh/firstHit/double/hostdevice` | 30.0 ms | 27.1 ms | -9.6% | [-27.1%, +18.2%] | - | ~ | device 0.8x faster than the host (was 0.7x) |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 26.0 ms | 24.9 ms | -4.0% | [-21.7%, +18.6%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `MeshTape/mesh/frame/double/hostdevice` | 21.3 ms | 21.3 ms | -0.1% | [-15.5%, +18.1%] | - | ~ | device 0.8x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 35.6 ms | 35.4 ms | -0.7% | [-16.6%, +18.3%] | - | ~ | device 1.0x faster than the host; tape/mesh 0.77x (was 0.84x) |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 27.4 ms | 27.3 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 0.9x faster than the host (was 0.8x); tape/mesh 0.91x (was 0.95x) |
| `MeshTape/tape/frame/double/hostdevice` | 27.7 ms | 28.2 ms | +1.9% | [-17.9%, +25.8%] | - | ~ | device 1.0x faster than the host; tape/mesh 0.76x |
| `MeshTape/view/firstHit/double/hostdevice` | 36.9 ms | 35.4 ms | -4.0% | [-21.7%, +18.5%] | - | ~ | device 1.2x faster than the host; view/mesh 0.77x (was 0.81x) |
| `MeshTape/view/frame/double/hostdevice` | 30.0 ms | 29.8 ms | -0.8% | [-16.1%, +17.4%] | - | ~ | device 1.0x faster than the host; view/mesh 0.71x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 680 µs | 681 µs | +0.1% | [-0.4%, +5.3%] | - | ~ | device 11.0x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.94 ms | 1.81 ms | -6.4% | [-12.2%, +0.2%] | - | ~ | device 6.1x faster than the host (was 5.7x) |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 912 µs | 916 µs | +0.4% | [-7.9%, +9.4%] | - | ~ | device 10.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 5.07 ms | 5.14 ms | +1.5% | [-20.3%, +27.4%] | - | ~ | device 5.7x faster than the host (was 5.3x) |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 3.14 ms | 3.29 ms | +4.8% | [-8.7%, +19.6%] | - | ~ | device 5.4x faster than the host (was 5.8x) |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.84 ms | 7.88 ms | +0.5% | [-17.6%, +22.4%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 8.56 ms | 8.77 ms | +2.5% | [-16.4%, +25.1%] | - | ~ | device 1.4x faster than the host (was 1.4x) |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 14.8 ms | 13.7 ms | -7.3% | [-17.6%, +5.1%] | - | ~ | device 0.8x faster than the host (was 0.7x) |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 21.5 ms | 20.9 ms | -2.9% | [-5.6%, -0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.0 ms | 11.0 ms | -0.6% | [-17.6%, +20.1%] | - | ~ | device 1.6x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.22 ms | 1.22 ms | +0.1% | [-1.3%, +1.5%] | - | ~ | device 8.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.69 ms | 3.92 ms | +6.2% | [-11.9%, +26.7%] | - | ~ | device 3.5x faster than the host (was 3.7x) |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 481 µs | 442 µs | -8.1% | [-15.3%, +0.4%] | - | ~ | device 19.5x faster than the host (was 18.3x) |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.96 ms | 4.85 ms | -2.1% | [-16.5%, +15.3%] | - | ~ | device 7.2x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 5.05 ms | 4.90 ms | -3.1% | [-23.7%, +24.6%] | - | ~ | device 4.9x faster than the host (was 5.3x) |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.91 ms | 9.00 ms | +1.0% | [-17.8%, +24.2%] | - | ~ | device 1.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 17.5 ms | 16.7 ms | -4.3% | [-17.3%, +11.3%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.01 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 20.0 ms | 20.3 ms | +1.6% | [-20.1%, +29.6%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.4 ms | 14.1 ms | -1.8% | [-13.4%, +11.5%] | - | ~ | device 1.8x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.47 ms | 1.41 ms | -3.6% | [-6.3%, -0.9%] | - | faster | device 12.8x faster than the host (was 12.4x) |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 5.19 ms | 5.36 ms | +3.3% | [-16.3%, +26.7%] | - | ~ | device 5.5x faster than the host (was 5.8x) |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 788 µs | 835 µs | +5.8% | [-0.1%, +12.5%] | - | ~ | device 12.8x faster than the host (was 13.7x) |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.05 ms | 3.03 ms | -0.7% | [-10.5%, +10.2%] | - | ~ | device 7.6x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.80 ms | 4.68 ms | -2.5% | [-23.2%, +24.1%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 18.2 ms | 18.5 ms | +1.8% | [-17.0%, +24.6%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 19.9 ms | 21.5 ms | +8.2% | [-3.9%, +23.0%] | - | ~ | device 1.2x faster than the host (was 1.5x) |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.90 ms | 8.23 ms | -7.6% | [-26.2%, +17.7%] | - | ~ | device 1.1x faster than the host (was 1.1x) |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 13.4 ms | 13.3 ms | -0.9% | [-15.1%, +15.9%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 19.3 ms | 18.8 ms | -2.6% | [-19.8%, +18.6%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.38 ms | 2.49 ms | +4.9% | [-8.5%, +19.2%] | - | ~ | device 5.3x faster than the host (was 5.5x) |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.96 ms | 7.02 ms | +0.9% | [-18.0%, +24.1%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.14 ms | 1.23 ms | +7.9% | [-0.2%, +16.1%] | - | ~ | device 7.6x faster than the host (was 8.2x) |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.58 ms | 2.75 ms | +6.6% | [-11.4%, +27.0%] | - | ~ | device 8.1x faster than the host (was 8.7x) |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 7.31 ms | 7.29 ms | -0.3% | [-20.6%, +24.7%] | - | ~ | device 3.0x faster than the host (was 2.7x) |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.9 ms | 32.8 ms | -3.1% | [-13.4%, +9.1%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 64.6 ms | 64.5 ms | -0.1% | [-22.3%, +28.7%] | - | ~ | device 0.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 17.2 ms | 16.8 ms | -2.2% | [-20.0%, +20.0%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 19.7 ms | 19.3 ms | -2.2% | [-16.7%, +15.2%] | - | ~ | device 1.1x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 35.9 ms | 35.6 ms | -1.0% | [-16.9%, +17.7%] | - | ~ | device 0.6x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 13.2 ms | 13.2 ms | +0.1% | [-13.8%, +16.3%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 10.3 ms | 10.2 ms | -1.4% | [-14.7%, +14.1%] | +0.0% | ~ | device 3.4x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 13.8 ms | 13.2 ms | -4.6% | [-17.5%, +10.1%] | +0.0% | ~ | device 3.7x faster than the host (was 3.5x) |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 11.0 ms | 10.9 ms | -0.2% | [-9.5%, +10.4%] | +0.0% | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 755 µs | 725 µs | -4.1% | [-20.1%, +16.0%] | - | ~ | device 15.7x faster than the host; hand/api 0.89x (was 0.85x) |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.5%, +0.5%] | - | ~ | device 14.5x faster than the host; hand/api 0.96x (was 1.01x) |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 645 µs | 642 µs | -0.4% | [-0.6%, -0.1%] | - | ~ | device 17.4x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.30 ms | 1.23 ms | -5.7% | [-10.7%, -0.1%] | - | faster | device 15.3x faster than the host (was 14.5x) |
| `Ray/mesh/intersects-morton/float/hostdevice` | 622 µs | 599 µs | -3.6% | [-5.5%, -1.5%] | - | faster | device 16.3x faster than the host (was 15.8x) |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.13 ms | +1.1% | [+0.1%, +2.1%] | - | ~ | device 15.0x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.24 ms | 1.18 ms | -4.8% | [-8.8%, -0.4%] | - | faster | device 12.2x faster than the host (was 11.6x); triangles/polygons 0.61x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.62 ms | 2.53 ms | -3.6% | [-10.4%, +4.0%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.51x (was 0.49x) |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.12 ms | 1.22 ms | +9.3% | [-8.3%, +28.5%] | - | ~ | device 10.8x faster than the host (was 11.7x) |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.30 ms | 2.33 ms | +1.1% | [-9.8%, +13.2%] | - | ~ | device 9.6x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 812 µs | 617 µs | -24.1% | [-37.9%, -4.0%] | - | faster | device 2.4x faster than the host (was 1.9x); tape/fused 1.12x (was 1.66x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 739 µs | 679 µs | -8.2% | [-10.2%, -6.2%] | - | faster | device 2.7x faster than the host (was 2.5x); tape/fused 1.24x (was 1.54x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 1.35 ms | 692 µs | -48.6% | [-65.0%, -3.7%] | - | faster | device 3.8x faster than the host (was 2.4x) |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 1.14 ms | 842 µs | -26.2% | [-41.7%, -1.0%] | - | faster | device 3.7x faster than the host (was 2.8x) |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.5 ms | 16.4 ms | -6.6% | [-13.9%, +1.5%] | - | ~ | device 0.7x faster than the host (was 0.8x); hand/api 0.95x (was 0.98x) |
| `Ray/mesh/firstHit-random/double/hostdevice` | 24.8 ms | 26.8 ms | +8.0% | [-1.1%, +19.0%] | - | ~ | device 0.8x faster than the host (was 0.8x); hand/api 1.02x (was 1.10x) |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.1 ms | 15.5 ms | -9.5% | [-19.0%, +0.1%] | - | ~ | device 0.8x faster than the host (was 0.7x) |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.3 ms | 27.3 ms | -0.0% | [-1.7%, +1.7%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 13.4 ms | 13.7 ms | +2.6% | [-14.1%, +22.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 25.2 ms | 24.2 ms | -4.0% | [-8.3%, +0.6%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 15.4 ms | 15.1 ms | -1.9% | [-20.1%, +21.0%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.09x (was 1.14x) |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 22.2 ms | 24.2 ms | +8.8% | [-0.8%, +20.2%] | - | ~ | device 1.9x faster than the host (was 2.9x); triangles/polygons 1.11x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.1 ms | 13.8 ms | -9.1% | [-18.4%, +0.4%] | - | ~ | device 1.0x faster than the host (was 0.9x) |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 22.8 ms | -1.4% | [-3.2%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | +0.0% | [-0.7%, +9.3%] | - | ~ | device 1.3x faster than the host; tape/fused 1.50x (was 0.99x) |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.12 ms | 2.14 ms | +0.9% | [-13.2%, +17.4%] | - | ~ | device 0.9x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.23 ms | 1.87 ms | +51.8% | [-1.4%, +105.8%] | - | ~ | device 1.6x faster than the host (was 2.2x) |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.24 ms | 2.25 ms | +0.5% | [-12.9%, +16.0%] | - | ~ | device 1.4x faster than the host (was 1.4x) |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.78 ms | 2.78 ms | -0.0% | [-21.1%, +26.5%] | - | ~ | device 2.4x faster than the host; marched/traced 3.16x (was 3.40x) |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.19 ms | 3.21 ms | +0.6% | [-16.5%, +21.4%] | - | ~ | device 2.2x faster than the host; marched/traced 4.60x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.46 ms | 8.78 ms | -7.2% | [-12.0%, -1.8%] | - | faster | device 5.1x faster than the host (was 4.7x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 14.9 ms | 14.8 ms | -1.0% | [-13.3%, +13.1%] | - | ~ | device 3.5x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.46 ms | 2.50 ms | +1.3% | [-13.9%, +19.1%] | - | ~ | device 3.4x faster than the host (was 3.5x); marched/traced 12.26x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 6.01 ms | 5.92 ms | -1.5% | [-13.2%, +11.8%] | - | ~ | device 2.6x faster than the host; marched/traced 9.94x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.9 ms | 30.6 ms | +2.2% | [-10.3%, +16.9%] | - | ~ | device 2.2x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 58.6 ms | 58.8 ms | +0.3% | [-16.5%, +20.4%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 6.02 ms | 5.41 ms | -10.1% | [-31.3%, +21.1%] | - | ~ | device 2.9x faster than the host (was 2.7x) |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 9.33 ms | 9.27 ms | -0.7% | [-17.5%, +19.6%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 5.72 ms | 5.88 ms | +2.7% | [-14.3%, +22.4%] | - | ~ | device 1.4x faster than the host (was 1.3x); marched/traced 26.88x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.5 ms | 10.4 ms | -0.3% | [-3.7%, +3.2%] | - | ~ | device 0.8x faster than the host (was 0.7x); marched/traced 26.17x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 156 ms | 158 ms | +1.0% | [-18.6%, +24.8%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 275 ms | 273 ms | -0.9% | [-22.4%, +26.9%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.91 ms | 5.03 ms | +2.4% | [-15.4%, +23.4%] | - | ~ | device 1.8x faster than the host; marched/traced 21.35x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.4 ms | 13.1 ms | +5.5% | [-14.7%, +29.3%] | - | ~ | device 1.2x faster than the host (was 1.3x); marched/traced 17.64x (was 18.77x) |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 106 ms | 107 ms | +1.3% | [-18.7%, +26.3%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 233 ms | 231 ms | -0.8% | [-23.4%, +28.6%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 17.7 ms | 17.8 ms | +1.0% | [-15.0%, +19.8%] | - | ~ | device 0.9x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.5 ms | 25.3 ms | -4.8% | [-25.2%, +22.3%] | - | ~ | device 1.1x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 580 µs | 639 µs | +10.2% | [-3.0%, +23.6%] | - | ~ | device 18.4x faster than the host (was 20.0x) |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.20 ms | 1.72 ms | +43.1% | [-11.7%, +105.0%] | - | ~ | device 12.8x faster than the host (was 16.4x) |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 674 µs | 675 µs | +0.2% | [-0.7%, +1.2%] | - | ~ | device 17.1x faster than the host; hit/crossing 0.95x (was 0.86x) |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.88 ms | 1.38 ms | -26.7% | [-48.2%, +14.9%] | - | ~ | device 14.3x faster than the host (was 11.4x); hit/crossing 1.25x (was 0.64x) |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 897 µs | 1.42 ms | +58.0% | [-37.0%, +209.4%] | - | ~ | device 2.4x faster than the host (was 3.0x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 998 µs | 1.48 ms | +48.2% | [-25.7%, +147.3%] | - | ~ | device 2.5x faster than the host (was 3.1x) |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 884 µs | 637 µs | -28.0% | [-44.2%, -0.9%] | - | faster | device 3.1x faster than the host (was 2.4x); hit/crossing 2.23x (was 1.01x) |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 694 µs | 746 µs | +7.5% | [-4.7%, +21.3%] | - | ~ | device 3.2x faster than the host (was 3.5x); hit/crossing 1.98x (was 1.44x) |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.71 ms | 4.72 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 10.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 11.2 ms | 10.7 ms | -4.0% | [-15.3%, +9.4%] | - | ~ | device 6.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.28 ms | 4.87 ms | +13.7% | [-4.6%, +33.0%] | - | ~ | device 8.7x faster than the host (was 10.3x); hit/crossing 0.97x (was 1.10x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.78 ms | 10.1 ms | +3.6% | [-13.2%, +23.0%] | - | ~ | device 6.1x faster than the host (was 6.3x); hit/crossing 1.06x (was 1.14x) |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 59.7 ms | 56.2 ms | -5.8% | [-25.8%, +23.4%] | - | ~ | device 3.5x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.1 ms | 85.4 ms | +5.3% | [-17.6%, +33.2%] | - | ~ | device 2.3x faster than the host (was 2.4x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 46.7 ms | 45.9 ms | -1.7% | [-20.9%, +23.2%] | - | ~ | device 3.5x faster than the host (was 3.2x); hit/crossing 1.22x (was 1.28x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.5 ms | 63.1 ms | -0.7% | [-17.5%, +19.5%] | - | ~ | device 2.6x faster than the host; hit/crossing 1.35x (was 1.28x) |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 15.6 ms | 15.5 ms | -0.5% | [-19.5%, +22.8%] | - | ~ | device 1.2x faster than the host (was 1.0x) |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 28.7 ms | 28.7 ms | +0.2% | [-8.8%, +10.0%] | - | ~ | device 1.1x faster than the host (was 1.1x) |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 15.7 ms | -7.7% | [-30.0%, +23.7%] | - | ~ | device 1.1x faster than the host (was 1.0x); hit/crossing 0.99x (was 0.92x) |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.1 ms | 27.4 ms | +1.1% | [-16.8%, +20.6%] | - | ~ | device 0.7x faster than the host (was 0.8x); hit/crossing 1.05x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.45 ms | +4.5% | [-19.5%, +34.8%] | - | ~ | device 2.8x faster than the host (was 3.1x) |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.22 ms | 2.32 ms | +4.6% | [-11.3%, +22.4%] | - | ~ | device 2.2x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.03 ms | 1.11 ms | +7.5% | [-1.9%, +17.0%] | - | ~ | device 2.8x faster than the host (was 3.0x); hit/crossing 1.31x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.09 ms | 1.10 ms | +1.6% | [-0.8%, +18.7%] | - | ~ | device 2.2x faster than the host; hit/crossing 2.10x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 59.5 ms | 61.5 ms | +3.2% | [-15.4%, +26.3%] | - | ~ | device 1.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 116 ms | 116 ms | +0.2% | [-21.3%, +27.5%] | - | ~ | device 0.9x faster than the host (was 1.0x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 53.4 ms | 54.4 ms | +1.9% | [-16.9%, +24.7%] | - | ~ | device 1.1x faster than the host (was 1.2x); hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 105 ms | 103 ms | -2.0% | [-20.4%, +21.2%] | - | ~ | device 0.8x faster than the host; hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 520 ms | 489 ms | -6.0% | [-15.6%, +4.3%] | - | ~ | device 0.5x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 791 ms | 784 ms | -0.8% | [-18.2%, +20.5%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 420 ms | 414 ms | -1.3% | [-21.2%, +23.5%] | - | ~ | device 0.4x faster than the host (was 0.4x); hit/crossing 1.18x (was 1.24x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 578 ms | 553 ms | -4.4% | [-16.6%, +10.2%] | - | ~ | device 0.4x faster than the host (was 0.3x); hit/crossing 1.42x (was 1.37x) |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 5.02 ms | 3.85 ms | -23.3% | [-43.6%, +12.3%] | - | ~ | device 1.9x faster than the host (was 1.5x); marched/traced 3.46x (was 2.68x) |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 5.53 ms | 4.56 ms | -17.5% | [-29.3%, -2.4%] | - | faster | device 1.3x faster than the host (was 1.2x); marched/traced 9.33x (was 8.42x) |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 13.4 ms | 13.3 ms | -0.8% | [-19.3%, +22.2%] | - | ~ | device 4.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 46.5 ms | 42.6 ms | -8.5% | [-60.0%, +119.8%] | - | ~ | device 1.5x faster than the host (was 1.5x) |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 2.27 ms | 1.26 ms | -44.6% | [-62.6%, +3.8%] | - | ~ | device 2.5x faster than the host (was 1.7x) |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.91 ms | +3.2% | [-0.9%, +7.3%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 184 µs | 95.1 µs | -48.3% | [-65.1%, -0.8%] | - | faster | device 15.8x faster than the host (was 10.5x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 154 µs | 150 µs | -2.5% | [-4.1%, -0.8%] | - | ~ | device 11.4x faster than the host (was 10.9x) |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 797 µs | 795 µs | -0.3% | [-0.6%, +0.1%] | - | ~ | device 3.2x faster than the host (was 3.1x) |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 924 µs | 928 µs | +0.4% | [+0.1%, +0.7%] | - | ~ | device 2.8x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.0 ms | 36.4 ms | +3.9% | [-3.8%, +11.9%] | - | ~ | device 0.2x faster than the host (was 0.1x); marched/traced 2.81x (was 2.90x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 38.0 ms | 36.3 ms | -4.6% | [-9.3%, +0.5%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 5.93x (was 5.54x) |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 102 ms | 102 ms | +0.5% | [-2.2%, +3.3%] | - | ~ | device 0.6x faster than the host (was 0.5x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 211 ms | 215 ms | +2.0% | [-0.8%, +4.9%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 17.4 ms | 17.8 ms | +2.5% | [-12.3%, +18.5%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 22.0 ms | +0.4% | [-1.3%, +2.2%] | - | ~ | device 0.3x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.08 ms | 1.09 ms | +0.8% | [-1.7%, +3.4%] | - | ~ | device 2.2x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.36 ms | 2.48 ms | +5.1% | [-1.4%, +11.6%] | - | ~ | device 1.2x faster than the host (was 1.1x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 5.72 ms | -4.3% | [-8.9%, +0.2%] | - | ~ | device 0.7x faster than the host (was 0.5x) |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 5.54 ms | 5.55 ms | +0.1% | [-0.6%, +6.8%] | - | ~ | device 0.5x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 19.0 ms | 19.0 ms | -0.2% | [-1.4%, +1.0%] | - | ~ | device 4.0x faster than the host; marched/traced 3.46x (was 2.81x) |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 23.7 ms | 23.7 ms | +0.3% | [-7.5%, +19.3%] | - | ~ | device 2.3x faster than the host; marched/traced 4.45x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 53.5 ms | 65.8 ms | +22.9% | [-1.3%, +47.7%] | - | ~ | device 4.5x faster than the host (was 5.8x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 105 ms | 106 ms | +0.2% | [-19.4%, +24.7%] | - | ~ | device 2.9x faster than the host (was 3.0x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 12.8 ms | 12.7 ms | -1.0% | [-19.1%, +21.4%] | - | ~ | device 4.5x faster than the host (was 6.0x) |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.4 ms | +0.5% | [-0.7%, +1.8%] | - | ~ | device 4.5x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 304 ms | 296 ms | -2.9% | [-4.7%, -1.0%] | - | ~ | device 0.3x faster than the host; marched/traced 1.97x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 340 ms | 341 ms | +0.2% | [-1.9%, +2.5%] | - | ~ | device 0.2x faster than the host; marched/traced 2.96x (was 3.05x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 582 ms | 581 ms | -0.1% | [-5.0%, +4.9%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.04 s | 1.01 s | -2.9% | [-7.6%, +2.1%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 169 ms | 164 ms | -3.1% | [-8.0%, +2.2%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 226 ms | 227 ms | +0.5% | [-9.2%, +11.6%] | - | ~ | device 0.4x faster than the host (was 0.3x) |
| `Regions/particles/region/float/hostdevice` | 2.52 ms | 2.52 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 4.6x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.22 ms | 3.21 ms | -0.3% | [-1.2%, +0.6%] | - | ~ | device 6.3x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 107 ms | 100 ms | -6.2% | [-12.0%, +0.5%] | - | ~ | device 5.4x faster than the host (was 4.3x); each/one-pass 0.03x (was 0.03x) |
| `Regions/particles/domainLabels/float/hostdevice` | 3.85 ms | 4.13 ms | +7.4% | [-32.9%, +68.0%] | - | ~ | device 5.2x faster than the host (was 7.7x) |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.77 ms | 6.56 ms | -3.0% | [-6.9%, +0.8%] | - | ~ | device 6.1x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.57 ms | 7.61 ms | +36.7% | [-0.3%, +73.7%] | - | ~ | device 3.8x faster than the host (was 5.3x); regions/hit 0.86x (was 1.21x) |
| `Regions/particles/region/double/hostdevice` | 4.03 ms | 3.91 ms | -2.8% | [-8.3%, +3.2%] | - | ~ | device 3.1x faster than the host (was 2.9x) |
| `Regions/particles/regionDistances/double/hostdevice` | 5.80 ms | 5.55 ms | -4.4% | [-11.5%, +2.8%] | - | ~ | device 4.1x faster than the host (was 3.7x) |
| `Regions/particles/regionByRegion/double/hostdevice` | 142 ms | 155 ms | +8.6% | [-0.2%, +17.6%] | - | ~ | device 2.8x faster than the host (was 3.3x); each/one-pass 0.04x (was 0.04x) |
| `Regions/particles/domainLabels/double/hostdevice` | 5.54 ms | 5.61 ms | +1.3% | [-15.1%, +20.9%] | - | ~ | device 2.5x faster than the host (was 2.8x) |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 16.0 ms | 17.2 ms | +7.8% | [-1.9%, +17.8%] | - | ~ | device 2.2x faster than the host (was 2.4x) |
| `Regions/assembly/firstHit/double/hostdevice` | 12.6 ms | 12.6 ms | +0.1% | [-5.3%, +5.9%] | - | ~ | device 2.3x faster than the host; regions/hit 1.36x (was 1.27x) |
| `Render/csg/frame/float/hostdevice` | 2.34 ms | 2.39 ms | +2.1% | [-2.7%, +7.0%] | - | ~ | device 4.6x faster than the host (was 4.9x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.32 ms | 2.35 ms | +1.2% | [+0.3%, +2.1%] | - | ~ | device 4.5x faster than the host (was 4.9x) |
| `Render/mesh/frame/float/hostdevice` | 1.94 ms | 1.94 ms | +0.1% | [-15.5%, +18.9%] | - | **DIFFER** | device 13.6x faster than the host; 1 host and device answers DIFFER (max diff 0.11) |
| `Render/csg/frame/double/hostdevice` | 5.63 ms | 5.28 ms | -6.3% | [-12.8%, +0.2%] | - | ~ | device 2.6x faster than the host (was 2.5x) |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.28 ms | 5.56 ms | +5.2% | [-2.6%, +14.1%] | - | ~ | device 2.3x faster than the host (was 2.9x) |
| `Render/mesh/frame/double/hostdevice` | 21.1 ms | 19.7 ms | -6.5% | [-13.0%, +1.1%] | - | ~ | device 1.4x faster than the host (was 1.2x) |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 800 µs | 804 µs | +0.4% | [-0.3%, +1.1%] | +0.0% | ~ | device 18.1x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.66 ms | 2.69 ms | +1.1% | [-0.1%, +2.2%] | +0.0% | ~ | device 5.4x faster than the host (was 7.9x) |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 12.8 ms | 13.5 ms | +5.6% | [-0.4%, +11.6%] | +0.0% | ~ | device 5.0x faster than the host (was 3.9x) |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.6 ms | 10.7 ms | +1.2% | [-5.0%, +7.7%] | +0.0% | ~ | device 5.4x faster than the host (was 5.2x) |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ | device 1.2x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | -0.2% | [-0.5%, -0.0%] | +0.0% | ~ | device 1.6x faster than the host (was 1.6x) |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.2 ms | 14.6 ms | +3.2% | [-0.7%, +7.3%] | +0.0% | ~ | device 5.6x faster than the host (was 4.6x) |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.2 ms | 11.2 ms | -0.3% | [-1.1%, +0.4%] | +0.0% | ~ | device 4.8x faster than the host (was 4.4x) |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.01 ms | 3.02 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ | device 12.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.77 ms | 4.79 ms | +0.5% | [-0.3%, +1.3%] | +0.0% | ~ | device 8.0x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.9 ms | 18.4 ms | +9.2% | [-0.4%, +18.9%] | +0.0% | ~ | device 5.7x faster than the host (was 9.2x); tape/fused 1.21x (was 1.30x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.5 ms | 25.0 ms | +2.0% | [+0.9%, +3.0%] | +0.0% | ~ | device 5.5x faster than the host; tape/fused 1.19x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 21.9 ms | 22.2 ms | +1.3% | [+1.1%, +1.4%] | +0.0% | ~ | device 5.9x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 29.7 ms | 29.7 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | device 4.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 36.7 ms | 36.4 ms | -0.8% | [-1.5%, -0.1%] | +0.0% | ~ | device 0.8x faster than the host (was 1.0x) |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 37.8 ms | 37.8 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ | device 1.1x faster than the host (was 1.1x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 106 ms | 106 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ | device 1.3x faster than the host (was 1.2x); tape/fused 1.05x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 110 ms | 110 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 0.9x faster than the host; tape/fused 1.07x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 111 ms | 111 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 118 ms | 118 ms | -0.1% | [-1.0%, +0.9%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | device 12.2x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 25.9 ms | 25.8 ms | -0.2% | [-0.9%, +0.4%] | +0.0% | ~ | device 0.7x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 8.04 ms | 8.55 ms | +6.3% | [-3.7%, +16.5%] | - | ~ | device 13.8x faster than the host |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.92 ms | 5.43 ms | -8.4% | [-16.8%, -0.1%] | - | faster | device 11.0x faster than the host (was 10.0x); surface/leaf 1.58x (was 1.36x) |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 94.4 ms | 92.5 ms | -2.0% | [-4.0%, +0.0%] | - | ~ | device 3.8x faster than the host (was 3.7x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 61.5 ms | 59.4 ms | -3.3% | [-6.5%, +0.1%] | - | ~ | device 3.7x faster than the host (was 3.4x); surface/leaf 1.56x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 26.8 ms | 27.9 ms | +3.9% | [-0.4%, +8.3%] | - | ~ | device 7.1x faster than the host (was 5.5x) |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.46 ms | 9.30 ms | -1.7% | [-12.6%, +10.3%] | - | ~ | device 9.8x faster than the host (was 9.0x); surface/hit 3.00x (was 2.84x) |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 51.0 ms | 48.9 ms | -4.1% | [-8.0%, +0.1%] | - | ~ | device 2.2x faster than the host (was 2.1x) |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 32.2 ms | 32.2 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.8x faster than the host; surface/leaf 1.52x (was 1.58x) |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 789 ms | 792 ms | +0.3% | [-0.5%, +1.2%] | - | ~ | device 0.4x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 540 ms | 535 ms | -0.9% | [-2.7%, +1.0%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.48x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 300 ms | 303 ms | +0.8% | [-0.2%, +1.9%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 102 ms | 102 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 0.7x faster than the host; surface/hit 2.98x |
| `Tape/csg/signedDistance/float/hostdevice` | 184 µs | 184 µs | +0.3% | [+0.1%, +0.4%] | - | ~ | device 13.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.3 µs | 48.5 µs | +2.4% | [+2.0%, +2.9%] | - | ~ | device 22.9x faster than the host (was 23.9x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 301 µs | 302 µs | +0.3% | [-0.1%, +0.7%] | - | ~ | device 15.1x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.34 ms | 1.35 ms | +0.7% | [+0.4%, +1.1%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 65.0 µs | 65.0 µs | -0.0% | [-0.6%, +0.5%] | - | ~ | device 8.0x faster than the host (was 7.5x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 48.3 µs | 47.9 µs | -0.8% | [-1.7%, +0.1%] | - | ~ | device 5.1x faster than the host (was 4.7x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 652 µs | -0.1% | [-0.1%, -0.0%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 11.8x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 476 µs | 476 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.6 µs | 48.2 µs | -0.7% | [-1.0%, -0.5%] | - | ~ | device 27.7x faster than the host (was 26.0x) |
| `Tape/csg/signedDistance/double/hostdevice` | 859 µs | 857 µs | -0.3% | [-0.5%, -0.1%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.5x faster than the host (was 1.5x) |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.64 ms | 6.64 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 191 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.1% | [-0.1%, -0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 3.01 ms | +8.6% | [-0.1%, +17.3%] | - | ~ | device 2.2x faster than the host (was 2.4x) |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.85 ms | +3.9% | [-0.4%, +8.2%] | - | ~ | device 14.2x faster than the host (was 10.9x) |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.51 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.6x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.5 ms | 17.6 ms | +0.5% | [-1.9%, +2.9%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 6194.3 s → 6153.2 s (-0.7%), the largest peak 6549 MB → 6548 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 14.4 s | 13.5 s | -6.2% | 600 MB | 605 MB | +0.8% | ~ |
| `BVHRefit` | 7.3 s | 7.0 s | -3.9% | 506 MB | 507 MB | +0.1% | ~ |
| `BVHUnion` | 27.2 s | 26.0 s | -4.4% | 1025 MB | 1035 MB | +1.0% | ~ |
| `BVHUnionMixed` | 13.0 s | 11.6 s | -10.5% | 626 MB | 630 MB | +0.8% | ~ |
| `ClosestPrimitive` | 160.7 s | 131.4 s | -18.2% | 1348 MB | 1352 MB | +0.3% | ~ |
| `ClosestSurface` | 213.8 s | 197.2 s | -7.7% | 1843 MB | 1843 MB | +0.0% | ~ |
| `Domain` | 74.7 s | 61.6 s | -17.6% | 995 MB | 998 MB | +0.3% | ~ |
| `DomainObject` | 67.3 s | 58.5 s | -13.1% | 949 MB | 952 MB | +0.3% | ~ |
| `Fused` | 62.1 s | 56.2 s | -9.5% | 1158 MB | 1163 MB | +0.4% | ~ |
| `LaunchTuning` | 562.9 s | 557.7 s | -0.9% | 3859 MB | 3859 MB | +0.0% | ~ |
| `Mesh` | 24.4 s | 21.3 s | -12.7% | 802 MB | 810 MB | +1.0% | ~ |
| `MeshCSG` | 444.3 s | 434.1 s | -2.3% | 6285 MB | 6284 MB | -0.0% | ~ |
| `MeshClosestPoint` | 24.3 s | 19.7 s | -19.1% | 755 MB | 762 MB | +0.9% | ~ |
| `MeshReconcile` | 11.3 s | 10.5 s | -6.8% | 512 MB | 513 MB | +0.1% | ~ |
| `MeshTape` | 787.0 s | 788.1 s | +0.1% | 6549 MB | 6548 MB | -0.0% | ~ |
| `PointCloud` | 17.3 s | 12.9 s | -25.8% | 571 MB | 571 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 7.6 s | 7.3 s | -4.0% | 507 MB | 507 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 6.9 s | 6.7 s | -2.8% | 507 MB | 507 MB | +0.1% | ~ |
| `Ray` | 203.8 s | 199.1 s | -2.3% | 3756 MB | 3756 MB | +0.0% | ~ |
| `RayCSG` | 236.6 s | 232.9 s | -1.6% | 4139 MB | 4139 MB | +0.0% | ~ |
| `RayCrossing` | 452.8 s | 454.8 s | +0.4% | 4102 MB | 4102 MB | +0.0% | ~ |
| `RayParticles` | 857.8 s | 881.5 s | +2.8% | 5781 MB | 5780 MB | -0.0% | ~ |
| `RayShell` | 516.1 s | 527.4 s | +2.2% | 5488 MB | 5488 MB | -0.0% | ~ |
| `Regions` | 372.4 s | 383.4 s | +3.0% | 4128 MB | 4127 MB | -0.0% | ~ |
| `Render` | 258.9 s | 267.4 s | +3.3% | 6048 MB | 6047 MB | -0.0% | ~ |
| `Shapes` | 26.3 s | 27.0 s | +2.9% | 592 MB | 593 MB | +0.0% | ~ |
| `SphereCloudBVH` | 8.5 s | 9.1 s | +6.7% | 507 MB | 508 MB | +0.1% | ~ |
| `SphereCloudRays` | 184.1 s | 186.5 s | +1.3% | 4154 MB | 4152 MB | -0.0% | ~ |
| `SphereContacts` | 7.7 s | 7.9 s | +2.8% | 506 MB | 507 MB | +0.1% | ~ |
| `SurfaceParts` | 428.3 s | 439.8 s | +2.7% | 4397 MB | 4397 MB | +0.0% | ~ |
| `Tape` | 68.2 s | 67.9 s | -0.4% | 982 MB | 985 MB | +0.3% | ~ |
| `TapeBVHUnion` | 46.2 s | 47.0 s | +1.7% | 923 MB | 925 MB | +0.3% | ~ |
| **all, in both** | 6194.3 s | 6153.2 s | -0.7% | 6549 MB | 6548 MB | -0.0% | ~ |

Device kernels: 218 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (17 cases): BVHRefit/uniform/refit/double/hostdevice, BVHUnion/capsules-diagonal/query-morton/double/hostdevice, BVHUnion/cylinders-random/query-morton/double/hostdevice, Fused/csg/signedDistance/float/hostdevice, Fused/chain16/signedDistance/double/hostdevice, LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice, PointCloudBVH/uniform/closest-morton/float/hostdevice, PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice, Ray/mesh/firstHit-random/float/hostdevice, Ray/csg-fused/firstHit-morton/double/hostdevice, RayCrossing/mesh/firstCrossing-morton/float/hostdevice, RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice, RayCrossing/mesh/firstCrossing-random/double/hostdevice, RayCrossing/csg-tape/firstCrossing-random/double/hostdevice, RayParticles/box-shell-tape/step-random/double/hostdevice, RayShell/armadillo-tape/firstHit-random/float/hostdevice, RayShell/armadillo-tape/firstHit-random/double/hostdevice

These probes compiled to the same code on both sides and were not run (26 cases): MeshReconcile (2), PointCloudHashGridBuild (4), Shapes (20)

<!-- END receipt cuda -->
