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
base: fd5c8a5
base-fingerprint: 263791791f122733f874f9739b94c83cd96178ea
head: 638b801
head-fingerprint: 5656db0b16bfc5683b6b9eee2bc5640c58a4d0bd
cases: 300
rounds: 2
reps: 3
date: 2026-11-18
-->

## Host comparison: green

**Accepted:** -

`638b801` against `fd5c8a5` (origin/main), 2026-11-18.

**VERDICT: no regression (0 major, 0 minor across 300 cases)**

Performance comparison: 0 major, 0 minor, 16 faster, 0 differ, 0 lost, 300 cases

- base: fd5c8a5 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 638b801 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 435 ms | 341 ms | -21.6% | [-34.6%, -2.4%] | +0.0% | faster |  |
| `BVH/mesh-boxes/ray-any/float/host` | 182 ms | 182 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 530 ms | 537 ms | +1.3% | [-0.6%, +3.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 365 ms | 366 ms | +0.4% | [-2.4%, +3.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 193 ms | 191 ms | -0.8% | [-2.6%, +1.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 588 ms | 587 ms | -0.1% | [-3.6%, +3.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 58.7 ms | 58.9 ms | +0.4% | [-1.7%, +2.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 52.8 ms | 52.8 ms | -0.1% | [-2.6%, +4.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 67.9 ms | 67.9 ms | -0.1% | [-1.8%, +1.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 66.5 ms | 63.3 ms | -4.8% | [-17.7%, +9.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 63.6 ms | 63.7 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 78.4 ms | 78.9 ms | +0.7% | [-5.2%, +6.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 77.8 ms | 76.7 ms | -1.3% | [-3.6%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 96.1 ms | 98.1 ms | +2.1% | [-10.3%, +15.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 68.8 ms | 69.5 ms | +0.9% | [-0.2%, +2.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 124 ms | 125 ms | +0.5% | [-6.6%, +8.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 81.8 ms | 82.1 ms | +0.4% | [-0.4%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 131 ms | 136 ms | +4.1% | [-9.5%, +19.8%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 66.4 ms | 67.3 ms | +1.3% | [+0.6%, +2.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 106 ms | 107 ms | +0.9% | [-8.2%, +11.0%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 79.6 ms | 79.6 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 135 ms | 132 ms | -2.8% | [-4.1%, -1.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.9 ms | 20.2 ms | +1.6% | [+1.1%, +2.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 42.6 ms | 42.5 ms | -0.2% | [-1.2%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 79.1 ms | 81.7 ms | +3.4% | [-2.8%, +9.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.5 ms | 20.6 ms | +0.6% | [-0.9%, +2.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.0 ms | 50.3 ms | +11.7% | [-1.4%, +24.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 90.0 ms | 89.6 ms | -0.5% | [-3.1%, +2.2%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 150 ms | 149 ms | -0.6% | [-5.7%, +4.7%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 78.8 ms | 79.1 ms | +0.5% | [-2.3%, +3.3%] | +0.0% | ~ | surface/leaf 1.88x |
| `ClosestSurface/union/closestSurface/float/host` | 168 ms | 168 ms | -0.2% | [-6.0%, +6.1%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 83.0 ms | 82.0 ms | -1.3% | [-4.6%, +2.1%] | +0.0% | ~ | surface/leaf 2.05x |
| `ClosestSurface/cloud/closestSurface/double/host` | 167 ms | 171 ms | +2.5% | [+1.0%, +4.0%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 84.1 ms | 84.5 ms | +0.5% | [-0.8%, +1.7%] | +0.0% | ~ | surface/leaf 2.03x |
| `ClosestSurface/union/closestSurface/double/host` | 177 ms | 174 ms | -1.7% | [-3.3%, -0.2%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 86.2 ms | 83.9 ms | -2.7% | [-4.6%, -0.8%] | +0.0% | ~ | surface/leaf 2.07x |
| `Domain/particles/labels/float/host` | 13.8 ms | 13.8 ms | -0.3% | [-1.0%, +0.5%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.93 ms | 6.86 ms | -0.9% | [-2.1%, +0.3%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.38 ms | 1.38 ms | -0.4% | [-1.8%, +0.9%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.4 ms | 14.4 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.26 ms | 7.27 ms | +0.0% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.38 ms | 1.37 ms | -0.1% | [-0.1%, -0.1%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 40.2 ms | 40.2 ms | -0.0% | [-0.0%, +0.0%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.8 ms | 21.8 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 591 µs | 591 µs | +0.0% | [-2.3%, +2.4%] | +0.0% | ~ | own/whole 68.10x |
| `DomainObject/partsOwn/labelAt/float/host` | 312 µs | 310 µs | -0.7% | [-1.4%, -0.1%] | +0.0% | ~ | own/whole 70.38x |
| `DomainObject/partsWhole/labels/double/host` | 40.3 ms | 40.1 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.8 ms | 23.2 ms | +1.8% | [+1.6%, +2.1%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 888 µs | 848 µs | -4.5% | [-47.3%, +71.9%] | +0.0% | ~ | own/whole 47.36x (was 45.36x) |
| `DomainObject/partsOwn/labelAt/double/host` | 323 µs | 321 µs | -0.4% | [-0.8%, -0.1%] | +0.0% | ~ | own/whole 72.36x |
| `Fused/csg/signedDistance/float/host` | 4.57 ms | 4.57 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | tape/fused 2.87x |
| `Fused/smooth/signedDistance/float/host` | 12.6 ms | 12.6 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | tape/fused 2.04x |
| `Fused/chain16/signedDistance/float/host` | 990 µs | 1.01 ms | +2.4% | [+1.8%, +3.0%] | - | ~ | tape/fused 16.56x |
| `Fused/drilled-mesh/signedDistance/float/host` | 51.6 ms | 49.2 ms | -4.6% | [-6.2%, -3.0%] | +0.0% | faster | tape/fused 1.04x (was 0.98x) |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 50.5 ms | 51.2 ms | +1.3% | [-1.3%, +4.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.76 ms | 4.75 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | tape/fused 2.83x |
| `Fused/smooth/signedDistance/double/host` | 13.0 ms | 12.9 ms | -1.1% | [-2.4%, +0.3%] | - | ~ | tape/fused 2.11x |
| `Fused/chain16/signedDistance/double/host` | 3.99 ms | 3.99 ms | -0.0% | [-2.1%, +2.1%] | - | ~ | tape/fused 4.26x |
| `Fused/drilled-mesh/signedDistance/double/host` | 65.2 ms | 62.4 ms | -4.3% | [-9.5%, +0.9%] | +0.0% | ~ | tape/fused 1.06x (was 1.00x) |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 65.1 ms | 66.2 ms | +1.8% | [-1.0%, +4.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 88.7 ms | 87.5 ms | -1.4% | [-5.6%, +3.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 247 ms | 253 ms | +2.6% | [-4.9%, +10.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 120 ms | 117 ms | -2.0% | [-5.7%, +1.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 303 ms | 290 ms | -4.4% | [-10.8%, +3.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 74.8 ms | 75.5 ms | +0.9% | [-3.6%, +5.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 226 ms | 229 ms | +1.7% | [-0.2%, +3.0%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 103 ms | 103 ms | -0.8% | [-4.5%, +3.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 34.7 ms | 46.1 ms | +32.7% | [-8.4%, +77.6%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 856 ms | 892 ms | +4.2% | [-8.5%, +12.3%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 124 ms | 124 ms | -0.1% | [-2.5%, +2.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 81.7 ms | 78.4 ms | -4.1% | [-31.9%, +40.1%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 1.22 s | 1.22 s | -0.3% | [-14.4%, +16.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 28.1 ms | 27.9 ms | -0.6% | [-7.8%, +7.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 17.9 ms | 16.4 ms | -8.3% | [-16.2%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 360 ms | 351 ms | -2.6% | [-5.7%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 242 ms | 239 ms | -1.1% | [-1.7%, -0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 97.0 ms | 98.1 ms | +1.2% | [+1.1%, +1.3%] | +0.0% | ~ | tape/fused 3.57x (was 3.71x) |
| `MeshCSG/smooth-fused/frame/float/host` | 85.6 ms | 85.9 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ | tape/fused 2.78x |
| `MeshCSG/union-tape/firstHit/double/host` | 31.6 ms | 30.9 ms | -2.4% | [-5.7%, +1.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 18.1 ms | 18.4 ms | +1.7% | [+1.3%, +2.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 413 ms | 416 ms | +0.8% | [-3.8%, +5.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 262 ms | 262 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 106 ms | 116 ms | +9.3% | [-1.6%, +20.3%] | +0.0% | ~ | tape/fused 3.58x (was 3.88x) |
| `MeshCSG/smooth-fused/frame/double/host` | 92.1 ms | 91.0 ms | -1.1% | [-2.1%, -0.1%] | +0.0% | ~ | tape/fused 2.88x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 266 ms | 248 ms | -6.6% | [-7.4%, -5.7%] | +0.0% | faster |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 260 ms | 264 ms | +1.8% | [-1.5%, +5.2%] | +0.0% | ~ | closestPoint/signedDistance 0.94x (was 1.02x) |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 476 ms | 395 ms | -17.0% | [-36.4%, +9.6%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 267 ms | 282 ms | +5.6% | [-3.6%, +15.2%] | +0.0% | ~ | gradient/closestPoint 0.94x (was 0.97x) |
| `MeshClosestPoint/tri/signedDistance/double/host` | 293 ms | 291 ms | -0.6% | [-4.0%, +8.7%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 336 ms | 327 ms | -2.5% | [-7.1%, +2.5%] | +0.0% | ~ | closestPoint/signedDistance 0.89x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 405 ms | 414 ms | +2.1% | [-6.0%, +15.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 321 ms | 340 ms | +6.0% | [-3.0%, +15.9%] | +0.0% | ~ | gradient/closestPoint 0.96x (was 1.04x) |
| `MeshTape/mesh/firstHit/float/host` | 50.2 ms | 50.1 ms | -0.1% | [-2.9%, +2.8%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 50.3 ms | 49.7 ms | -1.2% | [-5.2%, +3.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 20.1 ms | 20.1 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 84.2 ms | 84.7 ms | +0.6% | [-4.9%, +6.4%] | +0.0% | ~ | tape/mesh 0.59x |
| `MeshTape/tape/firstCrossing/float/host` | 53.3 ms | 53.1 ms | -0.4% | [-7.3%, +6.9%] | +0.0% | ~ | tape/mesh 0.94x |
| `MeshTape/tape/frame/float/host` | 25.4 ms | 25.5 ms | +0.4% | [-4.1%, +5.0%] | +0.0% | ~ | tape/mesh 0.79x |
| `MeshTape/view/firstHit/float/host` | 87.4 ms | 86.4 ms | -1.2% | [-6.2%, +4.1%] | +0.0% | ~ | view/mesh 0.58x |
| `MeshTape/view/frame/float/host` | 27.0 ms | 27.1 ms | +0.1% | [-2.0%, +2.3%] | +0.0% | ~ | view/mesh 0.74x |
| `MeshTape/mesh/firstHit/double/host` | 59.3 ms | 58.4 ms | -1.5% | [-4.4%, +1.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 60.5 ms | 61.2 ms | +1.2% | [-6.8%, +9.6%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 21.7 ms | 21.6 ms | -0.7% | [-1.2%, -0.2%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 96.0 ms | 96.8 ms | +0.8% | [-1.0%, +2.6%] | +0.0% | ~ | tape/mesh 0.60x |
| `MeshTape/tape/firstCrossing/double/host` | 62.8 ms | 62.1 ms | -1.1% | [-8.1%, +6.3%] | +0.0% | ~ | tape/mesh 0.99x |
| `MeshTape/tape/frame/double/host` | 27.9 ms | 28.1 ms | +0.8% | [-0.4%, +2.0%] | +0.0% | ~ | tape/mesh 0.77x |
| `MeshTape/view/firstHit/double/host` | 99.1 ms | 95.0 ms | -4.1% | [-6.4%, -1.8%] | +0.0% | faster | view/mesh 0.62x |
| `MeshTape/view/frame/double/host` | 29.7 ms | 29.9 ms | +0.6% | [+0.2%, +0.9%] | +0.0% | ~ | view/mesh 0.72x |
| `PointCloudBVH/uniform/build/float/host` | 55.7 ms | 55.4 ms | -0.5% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 162 ms | 167 ms | +3.1% | [-0.5%, +6.7%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 398 ms | 396 ms | -0.7% | [-4.3%, +3.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 87.9 ms | 87.8 ms | -0.0% | [-2.2%, +2.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.96 ms | 6.93 ms | -0.3% | [-1.5%, +2.1%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 60.3 ms | 59.8 ms | -0.9% | [-2.4%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 184 ms | 185 ms | +0.2% | [-4.9%, +5.7%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 415 ms | 417 ms | +0.4% | [-1.9%, +2.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 98.8 ms | 99.5 ms | +0.7% | [-4.6%, +6.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.23 ms | 7.26 ms | +0.3% | [-3.1%, +3.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.2 ms | 13.1 ms | -0.5% | [-1.8%, +0.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 126 ms | 120 ms | -4.5% | [-7.5%, -1.3%] | - | faster |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 509 ms | 492 ms | -3.5% | [-6.5%, -0.4%] | - | faster |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 151 ms | 148 ms | -2.0% | [-3.2%, -0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 849 ms | 841 ms | -0.9% | [-4.2%, +2.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.3 ms | 15.1 ms | -1.1% | [-4.9%, +2.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 123 ms | 123 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 512 ms | 510 ms | -0.5% | [-1.5%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 162 ms | 166 ms | +2.5% | [-1.8%, +7.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 867 ms | 891 ms | +2.8% | [-3.2%, +9.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 57.6 ms | 57.8 ms | +0.4% | [-4.0%, +5.0%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 153 ms | 156 ms | +2.1% | [-8.4%, +13.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 296 ms | 289 ms | -2.3% | [-6.0%, +1.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 163 ms | 175 ms | +7.3% | [-1.6%, +16.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 161 ms | 157 ms | -2.5% | [-13.3%, +9.5%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 62.7 ms | 62.6 ms | -0.1% | [-5.5%, +5.6%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 184 ms | 167 ms | -9.1% | [-13.7%, -4.3%] | - | faster |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 318 ms | 318 ms | +0.2% | [-3.2%, +3.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 180 ms | 181 ms | +0.6% | [+0.1%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 177 ms | 180 ms | +1.9% | [-7.9%, +12.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 12.2 ms | 12.3 ms | +0.5% | [-5.3%, +6.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 118 ms | 118 ms | -0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 342 ms | 343 ms | +0.3% | [-2.6%, +3.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 170 ms | 170 ms | +0.1% | [-3.4%, +3.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 338 ms | 341 ms | +0.8% | [-2.5%, +4.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 14.3 ms | 13.9 ms | -2.5% | [-6.3%, +1.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 120 ms | 119 ms | -0.8% | [-1.0%, -0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 365 ms | 363 ms | -0.6% | [-2.7%, +1.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 184 ms | 183 ms | -0.4% | [-7.1%, +6.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 340 ms | 342 ms | +0.6% | [-5.0%, +6.5%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 37.6 ms | 38.0 ms | +1.1% | [-2.1%, +4.3%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 41.7 ms | 42.0 ms | +0.7% | [-1.8%, +3.4%] | +0.0% | ~ |  |
| `QueryOrder/random/mortonOrder/float/host` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | - | - | - | - | - | new | arrived/ordered 1.56x |
| `QueryOrder/random/mortonOrder/double/host` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | - | - | - | - | - | new | arrived/ordered 1.72x |
| `Ray/mesh/firstHit/float/host` | 50.9 ms | 51.5 ms | +1.2% | [-5.6%, +8.5%] | +0.0% | ~ | hand/api 0.97x (was 1.02x) |
| `Ray/mesh/firstHit-handrolled/float/host` | 52.0 ms | 49.9 ms | -4.1% | [-5.3%, -3.0%] | +0.0% | faster |  |
| `Ray/mesh/intersects/float/host` | 44.2 ms | 44.7 ms | +1.0% | [-0.5%, +2.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 112 ms | 101 ms | -10.0% | [-27.7%, +11.8%] | +0.0% | ~ | triangles/polygons 0.51x (was 0.45x) |
| `Ray/mesh-polygons/intersects/float/host` | 84.8 ms | 83.6 ms | -1.5% | [-14.9%, +14.9%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.32 ms | 5.31 ms | -0.2% | [-0.8%, +0.3%] | - | ~ | tape/fused 1.46x |
| `Ray/csg-tape/firstHit/float/host` | 7.69 ms | 7.74 ms | +0.7% | [-0.1%, +1.4%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 60.6 ms | 59.9 ms | -1.2% | [-11.7%, +11.0%] | +0.0% | ~ | hand/api 0.91x |
| `Ray/mesh/firstHit-handrolled/double/host` | 55.5 ms | 54.3 ms | -2.3% | [-5.4%, +4.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 59.7 ms | 51.3 ms | -14.1% | [-26.9%, +3.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 118 ms | 123 ms | +4.0% | [-2.2%, +10.6%] | +0.0% | ~ | triangles/polygons 0.49x (was 0.51x) |
| `Ray/mesh-polygons/intersects/double/host` | 116 ms | 105 ms | -9.1% | [-15.7%, -2.1%] | +0.0% | faster |  |
| `Ray/csg-fused/firstHit/double/host` | 5.64 ms | 5.60 ms | -0.7% | [-1.3%, -0.0%] | - | ~ | tape/fused 1.44x |
| `Ray/csg-tape/firstHit/double/host` | 8.09 ms | 8.05 ms | -0.5% | [-1.8%, +0.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.2 ms | 18.5 ms | +1.5% | [-2.1%, +5.2%] | - | ~ | marched/traced 6.60x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 122 ms | 122 ms | -0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.4 ms | 22.4 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | marched/traced 6.58x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 147 ms | 148 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 36.0 ms | 36.0 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.3 ms | 19.3 ms | +0.1% | [-1.8%, +2.0%] | - | ~ | marched/traced 6.66x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 128 ms | 128 ms | +0.0% | [-0.3%, +0.4%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 24.1 ms | 23.7 ms | -1.3% | [-2.1%, -0.6%] | +0.0% | ~ | marched/traced 6.51x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 154 ms | 155 ms | +0.2% | [-1.5%, +1.9%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 39.0 ms | 39.0 ms | -0.0% | [-1.6%, +1.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 50.3 ms | 51.0 ms | +1.3% | [-2.6%, +5.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 48.8 ms | 49.5 ms | +1.5% | [-4.2%, +4.9%] | +0.0% | ~ | hit/crossing 1.03x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.79 ms | 7.78 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.30 ms | 6.21 ms | -1.5% | [-3.6%, +0.8%] | +0.0% | ~ | hit/crossing 1.25x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 201 ms | 200 ms | -0.5% | [-3.5%, +2.6%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 190 ms | 180 ms | -5.5% | [-10.6%, -0.3%] | +0.0% | faster | hit/crossing 1.11x (was 1.06x) |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 86.2 ms | 84.0 ms | -2.6% | [-14.4%, +12.8%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 67.4 ms | 63.9 ms | -5.1% | [-11.6%, +1.5%] | +0.0% | ~ | hit/crossing 1.31x |
| `RayCrossing/mesh/firstHit/double/host` | 60.5 ms | 60.9 ms | +0.6% | [-7.2%, +9.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 56.5 ms | 57.6 ms | +2.0% | [-3.3%, +5.5%] | +0.0% | ~ | hit/crossing 1.06x |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.19 ms | 8.14 ms | -0.5% | [-3.1%, +2.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.42 ms | 6.40 ms | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ | hit/crossing 1.27x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 219 ms | 228 ms | +4.1% | [-1.2%, +9.6%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 200 ms | 191 ms | -4.3% | [-9.6%, +1.6%] | +0.0% | ~ | hit/crossing 1.20x (was 1.10x) |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 87.0 ms | 90.8 ms | +4.5% | [-2.7%, +11.8%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 78.1 ms | 78.9 ms | +1.0% | [-11.4%, +15.3%] | +0.0% | ~ | hit/crossing 1.15x (was 1.11x) |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.9 ms | 11.9 ms | -0.2% | [-1.5%, +1.1%] | +0.0% | ~ | marched/traced 4.83x (was 5.15x) |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 61.5 ms | 57.5 ms | -6.5% | [-7.8%, -5.1%] | +0.0% | faster |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.43 ms | 8.54 ms | +1.3% | [-0.9%, +3.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.02 ms | 2.03 ms | +0.7% | [-4.6%, +6.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.01 ms | 6.89 ms | -1.7% | [-2.9%, -0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.6 ms | 12.7 ms | +0.5% | [-2.6%, +3.6%] | +0.0% | ~ | marched/traced 5.74x (was 6.13x) |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 77.3 ms | 72.8 ms | -5.8% | [-13.8%, +3.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 9.49 ms | 9.48 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.36 ms | 2.09 ms | -11.5% | [-19.2%, -2.3%] | +0.0% | faster |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.60 ms | 7.47 ms | -1.6% | [-5.3%, +2.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 252 ms | 250 ms | -0.5% | [-1.1%, +0.2%] | +0.0% | ~ | marched/traced 1.50x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 378 ms | 375 ms | -0.9% | [-1.0%, -0.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 247 ms | 248 ms | +0.8% | [-0.8%, +2.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 267 ms | 266 ms | -0.3% | [-1.7%, +1.2%] | +0.0% | ~ | marched/traced 1.57x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 417 ms | 418 ms | +0.2% | [-7.2%, +8.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 264 ms | 271 ms | +2.8% | [+1.1%, +4.5%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.50 ms | 7.48 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.8 ms | 12.8 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 273 ms | 274 ms | +0.3% | [-1.3%, +1.9%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.7 ms | 12.7 ms | +0.3% | [-0.7%, +1.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 31.3 ms | 31.0 ms | -0.8% | [-1.9%, +0.4%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.5 ms | 22.4 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ | regions/hit 1.39x |
| `Regions/particles/region/double/host` | 7.76 ms | 7.87 ms | +1.5% | [+0.3%, +2.7%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 13.2 ms | 13.2 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 287 ms | 287 ms | -0.0% | [-0.0%, -0.0%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.3 ms | 13.3 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 33.4 ms | 32.9 ms | -1.7% | [-2.4%, -0.9%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 24.4 ms | 24.3 ms | -0.6% | [-1.3%, +0.0%] | +0.0% | ~ | regions/hit 1.35x |
| `Render/csg/frame/float/host` | 17.1 ms | 12.9 ms | -25.0% | [-40.9%, -0.2%] | +0.0% | faster |  |
| `Render/csg/frame-orthographic/float/host` | 12.3 ms | 12.7 ms | +3.3% | [-6.4%, +8.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.2 ms | 20.3 ms | +0.7% | [+0.3%, +1.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.4 ms | 14.4 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.3 ms | 14.2 ms | -0.6% | [-1.1%, +0.0%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.7 ms | 21.7 ms | -0.3% | [-1.8%, +1.1%] | +0.0% | ~ |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 34.2 ms | 34.1 ms | -0.2% | [-1.5%, +1.0%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 5.10 ms | 5.09 ms | -0.2% | [-0.4%, +0.1%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.26 ms | 4.27 ms | +0.2% | [-0.5%, +1.0%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.7 ms | 17.8 ms | +0.2% | [-0.2%, +0.6%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.45 ms | 4.40 ms | -1.2% | [-2.4%, +0.1%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.20 ms | 4.23 ms | +0.8% | [+0.2%, +1.3%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.29 ms | 2.31 ms | +0.7% | [-0.6%, +2.0%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.47 ms | 1.48 ms | +0.9% | [+0.4%, +1.4%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.07 ms | 2.09 ms | +1.0% | [+0.1%, +2.0%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 28.0 ms | 28.3 ms | +0.8% | [-0.1%, +1.8%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 47.3 ms | 47.6 ms | +0.6% | [-0.3%, +1.4%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.52 ms | 5.48 ms | -0.7% | [-0.8%, -0.6%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.57 ms | 4.59 ms | +0.4% | [-0.4%, +1.3%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 23.2 ms | 23.2 ms | -0.2% | [-1.0%, +0.7%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.64 ms | 4.59 ms | -1.1% | [-1.4%, -0.8%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.34 ms | 5.35 ms | +0.2% | [-0.2%, +0.6%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.54 ms | 2.55 ms | +0.3% | [-0.4%, +1.0%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.81 ms | 1.83 ms | +0.7% | [+0.4%, +1.1%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.14 ms | 2.13 ms | -0.7% | [-2.2%, +0.9%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 31.0 ms | 31.9 ms | +2.8% | [-0.5%, +6.1%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 61.1 ms | 61.4 ms | +0.5% | [-6.5%, +8.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 39.5 ms | 39.3 ms | -0.5% | [-6.5%, +6.0%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 99.1 ms | 99.5 ms | +0.3% | [-3.0%, +7.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 78.9 ms | 68.3 ms | -13.4% | [-23.6%, -0.8%] | +0.0% | faster |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 48.3 ms | 46.0 ms | -4.9% | [-7.6%, -2.1%] | +0.0% | faster |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 145 ms | 146 ms | +0.8% | [-5.7%, +7.9%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 94.8 ms | 95.2 ms | +0.5% | [-9.9%, +11.9%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 353 ms | 356 ms | +1.1% | [-6.4%, +9.2%] | +0.0% | ~ | tape/fused 0.98x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 356 ms | 350 ms | -1.7% | [-7.5%, +4.7%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 110 ms | 108 ms | -1.4% | [-3.9%, +1.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 458 ms | 416 ms | -9.1% | [-10.6%, -7.5%] | +0.0% | faster | tape/fused 1.03x (was 0.93x) |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 427 ms | 428 ms | +0.2% | [-5.5%, +6.4%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 186 ms | 189 ms | +1.6% | [-5.1%, +8.5%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 221 ms | 222 ms | +0.3% | [-2.9%, +3.7%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 147 ms | 147 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 91.4 ms | 91.4 ms | +0.1% | [-0.0%, +0.1%] | +0.0% | ~ | surface/leaf 1.61x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 143 ms | 141 ms | -1.8% | [-6.0%, +2.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 82.0 ms | 79.8 ms | -2.7% | [-6.3%, +1.2%] | +0.0% | ~ | surface/leaf 1.76x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 65.8 ms | 67.4 ms | +2.3% | [+0.8%, +3.8%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 37.0 ms | 38.9 ms | +5.1% | [-0.9%, +11.1%] | +0.0% | ~ | surface/hit 1.73x |
| `SurfaceParts/boxes/closestSurface/double/host` | 156 ms | 161 ms | +3.4% | [-10.0%, +18.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 93.2 ms | 93.0 ms | -0.2% | [-1.3%, +0.9%] | +0.0% | ~ | surface/leaf 1.74x (was 1.67x) |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 166 ms | 169 ms | +1.7% | [-7.2%, +11.3%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 97.1 ms | 96.1 ms | -1.1% | [-8.2%, +6.7%] | +0.0% | ~ | surface/leaf 1.76x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 79.1 ms | 78.0 ms | -1.4% | [-8.6%, +6.1%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 42.7 ms | 44.8 ms | +4.7% | [-3.4%, +13.2%] | +0.0% | ~ | surface/hit 1.74x (was 1.85x) |
| `Tape/csg/signedDistance/float/host` | 13.1 ms | 13.1 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.68 ms | 4.64 ms | -0.8% | [-2.0%, +0.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.8 ms | 25.7 ms | -0.6% | [-2.9%, +1.8%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 41.3 ms | 41.4 ms | +0.3% | [-1.7%, +2.3%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.79 ms | 1.77 ms | -1.0% | [-1.4%, -0.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 130 µs | 135 µs | +3.6% | [-1.8%, +9.7%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.6 ms | 30.9 ms | +0.8% | [-0.6%, +2.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.86 ms | 5.77 ms | -1.4% | [-2.8%, -0.0%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.7 ms | 16.8 ms | +0.7% | [-0.0%, +1.4%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.80 ms | 4.76 ms | -0.7% | [-2.3%, +0.9%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.4 ms | 13.4 ms | -0.0% | [-1.1%, +1.0%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.73 ms | 4.68 ms | -1.0% | [-3.6%, +1.7%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 27.1 ms | 27.2 ms | +0.3% | [-0.8%, +1.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 43.0 ms | 43.0 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.79 ms | 1.81 ms | +0.6% | [-1.7%, +3.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 334 µs | 373 µs | +11.4% | [-30.8%, +74.7%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.9 ms | 30.7 ms | -0.7% | [-1.6%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.93 ms | 7.90 ms | -0.4% | [-1.1%, +0.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.9 ms | 17.0 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.89 ms | 7.97 ms | +1.0% | [+0.3%, +1.6%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 43.0 ms | 42.9 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 205 ms | 205 ms | -0.1% | [-0.8%, +0.7%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 43.7 ms | 44.0 ms | +0.7% | [-0.2%, +1.5%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 222 ms | 222 ms | -0.4% | [-1.3%, +0.6%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 866.9 s → 763.8 s (-11.9%), the largest peak 1410 MB → 1411 MB; 1 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 10.4 s | 10.2 s | -2.3% | 558 MB | 559 MB | +0.3% | ~ |
| `BVHRefit` | 4.7 s | 4.4 s | -7.2% | 497 MB | 498 MB | +0.1% | ~ |
| `BVHUnion` | 27.6 s | 20.6 s | -25.6% | 948 MB | 949 MB | +0.1% | ~ |
| `BVHUnionMixed` | 8.6 s | 10.8 s | +25.1% | 555 MB | 555 MB | +0.1% | **MAJOR** |
| `ClosestPrimitive` | 34.0 s | 32.3 s | -4.8% | 1296 MB | 1298 MB | +0.1% | ~ |
| `ClosestSurface` | 32.3 s | 29.6 s | -8.2% | 1228 MB | 1297 MB | +5.6% | ~ |
| `Domain` | 19.7 s | 20.4 s | +3.2% | 842 MB | 843 MB | +0.1% | ~ |
| `DomainObject` | 17.4 s | 17.1 s | -1.6% | 846 MB | 847 MB | +0.1% | ~ |
| `Fused` | 29.6 s | 26.2 s | -11.7% | 1135 MB | 1206 MB | +6.2% | ~ |
| `LaunchTuning` | 44.7 s | 38.9 s | -12.9% | 1310 MB | 1280 MB | -2.3% | ~ |
| `Mesh` | 18.3 s | 16.8 s | -8.0% | 783 MB | 784 MB | +0.1% | ~ |
| `MeshCSG` | 52.2 s | 40.9 s | -21.8% | 1255 MB | 1261 MB | +0.5% | ~ |
| `MeshClosestPoint` | 17.5 s | 15.2 s | -13.0% | 780 MB | 780 MB | -0.0% | ~ |
| `MeshReconcile` | 6.9 s | 7.1 s | +3.1% | 566 MB | 567 MB | +0.2% | ~ |
| `MeshTape` | 52.4 s | 45.8 s | -12.6% | 1288 MB | 1279 MB | -0.7% | ~ |
| `PointCloud` | 9.6 s | 7.7 s | -19.5% | 558 MB | 558 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 5.3 s | 4.0 s | -24.8% | 471 MB | 471 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 4.5 s | 3.3 s | -26.9% | 452 MB | 452 MB | +0.2% | ~ |
| `QueryOrder` | - | 11.6 s | | - | 686 MB | | new |
| `Ray` | 48.6 s | 39.6 s | -18.6% | 1317 MB | 1292 MB | -1.9% | ~ |
| `RayCSG` | 49.1 s | 34.4 s | -30.0% | 1188 MB | 1194 MB | +0.5% | ~ |
| `RayCrossing` | 59.3 s | 47.8 s | -19.3% | 1331 MB | 1351 MB | +1.5% | ~ |
| `RayParticles` | 62.0 s | 53.8 s | -13.3% | 1319 MB | 1332 MB | +1.0% | ~ |
| `RayShell` | 45.7 s | 40.2 s | -12.0% | 1285 MB | 1267 MB | -1.4% | ~ |
| `Regions` | 38.1 s | 31.8 s | -16.5% | 1073 MB | 1073 MB | +0.0% | ~ |
| `Render` | 39.6 s | 38.4 s | -3.1% | 1285 MB | 1273 MB | -0.9% | ~ |
| `Shapes` | 12.1 s | 9.3 s | -23.6% | 554 MB | 555 MB | +0.2% | ~ |
| `SphereCloudBVH` | 5.7 s | 5.1 s | -11.3% | 511 MB | 512 MB | +0.2% | ~ |
| `SphereCloudRays` | 29.9 s | 29.4 s | -1.6% | 1063 MB | 1066 MB | +0.3% | ~ |
| `SphereContacts` | 5.8 s | 4.7 s | -18.8% | 484 MB | 486 MB | +0.3% | ~ |
| `SurfaceParts` | 42.8 s | 44.5 s | +4.0% | 1410 MB | 1411 MB | +0.1% | ~ |
| `Tape` | 17.3 s | 18.5 s | +6.9% | 830 MB | 829 MB | -0.1% | ~ |
| `TapeBVHUnion` | 15.2 s | 15.3 s | +0.6% | 818 MB | 826 MB | +1.0% | ~ |
| **all, in both** | 866.9 s | 763.8 s | -11.9% | 1410 MB | 1411 MB | +0.1% | ~ |


Flagged after 2 rounds, and judged on 10 (11 cases): BVHUnion/spheres/query-random/float/host, TriMeshSDF/w8/query-random/float/host, MeshSDF/query-random/float/host, MeshClosestPoint/tri/signedDistance/double/host, MeshClosestPoint/mesh/closestPoint/double/host, PointCloudBVH/uniform/closest-outside-random/float/host, Ray/mesh/firstHit-handrolled/double/host, RayCrossing/mesh/firstCrossing/float/host, RayCrossing/mesh/firstCrossing/double/host, Render/csg/frame-orthographic/float/host, SphereCloudBVH/polydisperse/closest-random/float/host

These probes compiled to the same code on both sides and were not run (8 cases): BVHRefit (2), MeshReconcile (4), PointCloudHashGridBuild (2)

The probes differ between the sides (each side ran its own): Performance/QueryOrder/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: fd5c8a5
base-fingerprint: 263791791f122733f874f9739b94c83cd96178ea
head: 638b801
head-fingerprint: 5656db0b16bfc5683b6b9eee2bc5640c58a4d0bd
cases: 402
rounds: 2
reps: 3
date: 2026-11-18
-->

## Device comparison (CUDA): red

**Accepted:** The host/device pixel disagreement (1 pixel in the float mesh frames, identical on the base fd5c8a5) is not ours; a known issue, worked on separately.

`638b801` against `fd5c8a5` (origin/main), 2026-11-18.

**VERDICT: host and device answers differ in 4 cases; investigate before merging (see Flagged above)**

Performance comparison: 0 major, 0 minor, 0 faster, 4 differ, 0 lost, 402 cases

- base: fd5c8a5 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 638b801 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.62 ms | 1.61 ms | -0.5% | [-1.7%, +0.7%] | - | ~ | device 30.9x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.42 ms | 3.40 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 25.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.04 ms | 1.05 ms | +0.7% | [-1.4%, +2.9%] | - | ~ | device 26.4x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.32 ms | 2.31 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 22.9x faster than the host (was 22.1x) |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.25 ms | 6.30 ms | +0.7% | [-1.1%, +2.6%] | - | ~ | device 9.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 12.0 ms | 12.0 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 7.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.3 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 1.6x faster than the host (was 1.5x) |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 56.9 ms | -0.2% | [-0.3%, -0.2%] | - | ~ | device 2.0x faster than the host (was 2.1x) |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.5 ms | 20.7 ms | +0.9% | [-1.3%, +3.1%] | - | ~ | device 1.3x faster than the host (was 1.6x) |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.8 ms | 47.9 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 1.1x faster than the host (was 1.0x) |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.1 ms | 62.2 ms | +1.8% | [-0.7%, +4.4%] | - | ~ | device 0.9x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 108 ms | 109 ms | +0.5% | [-3.1%, +4.1%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `BVHRefit/uniform/refit/float/hostdevice` | 10.6 ms | 9.59 ms | -9.2% | [-31.1%, +22.4%] | +0.0% | ~ | device 5.9x faster than the host (was 5.6x) |
| `BVHRefit/uniform/refit/double/hostdevice` | 10.9 ms | 10.8 ms | -0.7% | [-12.7%, +13.0%] | +0.0% | ~ | device 8.9x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 387 µs | 389 µs | +0.6% | [+0.2%, +1.0%] | - | ~ | device 16.1x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 845 µs | 846 µs | +0.1% | [-0.3%, +0.5%] | - | ~ | device 10.7x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.51 ms | 4.53 ms | +0.4% | [-0.6%, +1.5%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.67 ms | 4.68 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 2.0x faster than the host (was 2.1x) |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 590 µs | 590 µs | -0.1% | [-1.0%, +0.7%] | - | ~ | device 13.7x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 8.8x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.78 ms | 6.78 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.3x faster than the host (was 1.2x) |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.05 ms | 7.00 ms | -0.8% | [-0.8%, -0.7%] | - | ~ | device 2.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 907 µs | 909 µs | +0.2% | [-0.4%, +0.8%] | - | ~ | device 16.9x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.25 ms | 2.24 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 9.1x faster than the host (was 13.8x) |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.46 ms | 9.49 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.9x faster than the host (was 2.6x) |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 858 µs | 853 µs | -0.5% | [-1.0%, +0.0%] | - | ~ | device 13.9x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.08 ms | 2.08 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 8.2x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.10 ms | 9.09 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.82 ms | 9.82 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 315 µs | 313 µs | -0.5% | [-0.9%, -0.1%] | - | ~ | device 12.3x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 6.6x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.6x faster than the host (was 3.9x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.38 ms | 5.39 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 517 ms | 518 ms | +0.1% | [-0.9%, +1.2%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.1 ms | 20.1 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 4.3x faster than the host |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.33 ms | 8.33 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 4.9x faster than the host; surface/leaf 2.42x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.6 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 9.2x faster than the host (was 9.9x) |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.18 ms | 6.18 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 10.8x faster than the host; surface/leaf 2.52x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 147 ms | 147 ms | +0.1% | [-2.5%, +2.7%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 59.9 ms | 60.0 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 0.8x faster than the host (was 1.1x); surface/leaf 2.45x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 71.9 ms | 71.9 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.7x faster than the host (was 2.5x) |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.3x faster than the host (was 3.1x); surface/leaf 2.57x |
| `Domain/particles/labels/float/hostdevice` | 4.04 ms | 4.07 ms | +0.7% | [+0.1%, +1.3%] | - | ~ | device 4.9x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.72 ms | 2.72 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 3.8x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 121 µs | 121 µs | +0.4% | [-3.3%, +4.1%] | - | ~ | device 15.2x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.04 ms | 7.17 ms | +1.8% | [+1.1%, +2.4%] | - | ~ | device 2.9x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.40 ms | 3.40 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 3.0x faster than the host |
| `Domain/particles/surfaceGroups/double/hostdevice` | 127 µs | 126 µs | -0.7% | [-6.5%, +5.5%] | - | ~ | device 13.7x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 12.4 ms | 12.4 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 5.4x faster than the host (was 5.2x) |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.75 ms | 8.74 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 3.7x faster than the host (was 3.9x) |
| `DomainObject/partsOwn/labels/float/hostdevice` | 272 µs | 268 µs | -1.3% | [-2.0%, -0.6%] | - | ~ | device 4.3x faster than the host (was 4.1x); own/whole 46.09x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 148 µs | 150 µs | +1.4% | [+0.6%, +2.2%] | - | ~ | device 5.0x faster than the host (was 4.1x); own/whole 58.18x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 20.0 ms | 20.0 ms | -0.1% | [-0.7%, +0.6%] | - | ~ | device 3.5x faster than the host |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 11.1 ms | 11.1 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 3.2x faster than the host |
| `DomainObject/partsOwn/labels/double/hostdevice` | 396 µs | 387 µs | -2.4% | [-5.4%, +0.7%] | - | ~ | device 2.2x faster than the host; own/whole 51.74x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 231 µs | 231 µs | +0.3% | [-0.1%, +0.6%] | - | ~ | device 2.1x faster than the host; own/whole 48.03x |
| `Fused/csg/signedDistance/float/hostdevice` | 48.7 µs | 48.7 µs | +0.1% | [-3.3%, +3.7%] | - | ~ | device 29.3x faster than the host (was 30.6x); tape/fused 4.50x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.6 µs | 50.7 µs | +0.3% | [-3.5%, +4.3%] | - | ~ | device 55.8x faster than the host; tape/fused 7.12x |
| `Fused/chain16/signedDistance/float/hostdevice` | 49.0 µs | 47.8 µs | -2.4% | [-4.8%, +0.1%] | - | ~ | device 11.3x faster than the host (was 12.2x); tape/fused 9.96x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.5 ms | 17.6 ms | +0.9% | [-0.1%, +1.8%] | - | ~ | device 8.8x faster than the host; tape/fused 1.77x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.1 ms | 31.2 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 5.1x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 811 µs | 810 µs | -0.1% | [-0.3%, +0.0%] | - | ~ | device 2.0x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.56 ms | 1.56 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.1x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.11 ms | 2.11 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 286 ms | 285 ms | -0.3% | [-1.1%, +0.5%] | - | ~ | device 0.6x faster than the host; tape/fused 1.03x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 286 ms | 293 ms | +2.4% | [+0.3%, +4.5%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.22 ms | 8.21 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ | device 14.9x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.25 ms | 8.30 ms | +0.6% | [-0.8%, +2.0%] | +0.0% | ~ | device 15.0x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.23 ms | 8.23 ms | -0.0% | [-0.9%, +1.2%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.35 ms | 8.24 ms | -1.4% | [-2.3%, -0.4%] | +0.0% | ~ | device 18.1x faster than the host (was 16.0x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 682 µs | 684 µs | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | device 5.6x faster than the host (was 5.0x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.77 ms | 1.77 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.04 ms | -0.4% | [-1.1%, +0.4%] | +0.0% | ~ | device 3.3x faster than the host (was 3.7x); default/this 0.66x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 787 µs | 788 µs | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ | device 4.8x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 740 µs | 741 µs | +0.1% | [-0.6%, +0.9%] | +0.0% | ~ | device 16.7x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 834 µs | 838 µs | +0.5% | [+0.4%, +0.7%] | +0.0% | ~ | device 15.8x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 737 µs | 742 µs | +0.7% | [-0.0%, +1.5%] | +0.0% | ~ | device 16.7x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 726 µs | 731 µs | +0.6% | [-1.1%, +2.4%] | +0.0% | ~ | device 17.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 161 ms | 162 ms | +1.0% | [-3.2%, +5.3%] | +0.0% | ~ | device 0.8x faster than the host (was 0.8x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 159 ms | 158 ms | -0.8% | [-3.0%, +1.4%] | +0.0% | ~ | device 0.8x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 161 ms | 161 ms | -0.3% | [-1.3%, +0.8%] | +0.0% | ~ | device 0.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 162 ms | 165 ms | +2.1% | [-0.3%, +4.6%] | +0.0% | ~ | device 0.8x faster than the host; default/this 0.98x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ | device 2.5x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.81 ms | 2.80 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.95 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | device 1.8x faster than the host; default/this 0.71x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.56 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ | device 2.3x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.19 ms | 7.27 ms | +1.1% | [+0.6%, +1.6%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.30 ms | 7.26 ms | -0.5% | [-0.8%, -0.1%] | +0.0% | ~ | device 1.9x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.27 ms | 7.26 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ | device 1.9x faster than the host (was 2.3x); default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.33 ms | 7.31 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ | device 3.1x faster than the host (was 1.7x); default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.27 ms | 8.47 ms | +2.4% | [+1.2%, +3.7%] | - | ~ | device 15.1x faster than the host (was 15.9x) |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.9 ms | 16.9 ms | +0.2% | [-0.7%, +1.2%] | - | ~ | device 9.4x faster than the host (was 9.9x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 169 ms | 167 ms | -1.4% | [-1.7%, -1.1%] | - | ~ | device 0.8x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 283 ms | 281 ms | -0.9% | [-1.9%, +0.2%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.4 ms | 24.3 ms | -0.5% | [-1.2%, +0.1%] | - | ~ | device 6.3x faster than the host (was 5.5x) |
| `MeshSDF/query-random/float/hostdevice` | 58.0 ms | 58.1 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 3.1x faster than the host (was 3.3x) |
| `MeshSDF/query-morton/double/hostdevice` | 158 ms | 157 ms | -1.0% | [-3.7%, +1.9%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `MeshSDF/query-random/double/hostdevice` | 240 ms | 247 ms | +2.9% | [+1.3%, +4.4%] | - | ~ | device 0.8x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 1.98 ms | 1.98 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 5.9x faster than the host (was 5.4x) |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.34 ms | 3.33 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 4.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.05 ms | 6.02 ms | -0.5% | [-1.1%, +0.0%] | - | ~ | device 5.0x faster than the host (was 5.2x) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.1 ms | 62.8 ms | -0.5% | [-1.6%, +0.7%] | - | ~ | device 3.7x faster than the host (was 3.3x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 93.3 ms | 92.4 ms | -1.0% | [-1.6%, -0.4%] | - | ~ | device 2.4x faster than the host (was 2.5x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 237 ms | 240 ms | +1.2% | [+0.2%, +2.1%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.8 ms | 24.4 ms | -1.4% | [-1.7%, -1.2%] | - | ~ | device 2.1x faster than the host (was 2.4x); tape/fused 2.57x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.1 ms | 26.7 ms | -1.6% | [-3.1%, +0.0%] | - | ~ | device 2.9x faster than the host (was 2.3x); tape/fused 3.46x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.9 ms | 66.0 ms | +0.2% | [-1.6%, +2.1%] | - | ~ | device 4.8x faster than the host; tape/fused 3.63x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 20.7 ms | 21.1 ms | +1.7% | [-2.7%, +6.3%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 37.7 ms | 37.6 ms | -0.2% | [-1.4%, +1.1%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 40.2 ms | 40.7 ms | +1.2% | [-0.2%, +2.5%] | - | ~ | device 0.8x faster than the host (was 0.9x) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 539 ms | 526 ms | -2.4% | [-5.9%, +2.7%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 852 ms | 842 ms | -1.2% | [-2.5%, +0.3%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.98 s | 1.94 s | -2.4% | [-4.4%, -0.4%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 194 ms | 193 ms | -0.5% | [-2.7%, +1.8%] | - | ~ | device 0.3x faster than the host (was 0.3x); tape/fused 2.72x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 301 ms | 305 ms | +1.6% | [-1.0%, +4.1%] | - | ~ | device 0.2x faster than the host; tape/fused 2.76x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 546 ms | 524 ms | -3.9% | [-9.0%, +1.7%] | - | ~ | device 0.5x faster than the host (was 0.6x); tape/fused 3.69x |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 12.0 ms | 12.0 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 11.8x faster than the host (was 12.4x) |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 58.5 ms | 58.5 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.6x faster than the host |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 261 ms | 259 ms | -1.0% | [-3.1%, +1.2%] | - | ~ | device 0.5x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 239 ms | 240 ms | +0.3% | [-0.9%, +1.6%] | - | ~ | device 0.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.67 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ | device 22.3x faster than the host (was 17.3x) |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.31 ms | 5.31 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | device 12.3x faster than the host (was 11.7x) |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.28 ms | 1.28 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 15.3x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.28 ms | 1.28 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 15.2x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.08 ms | 2.08 ms | -0.2% | [-0.4%, +0.1%] | - | **DIFFER** | device 8.0x faster than the host; 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.34 ms | 3.34 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 10.2x faster than the host; tape/mesh 0.38x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.66 ms | 2.67 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 7.6x faster than the host; tape/mesh 0.48x |
| `MeshTape/tape/frame/float/hostdevice` | 4.31 ms | 4.29 ms | -0.5% | [-0.6%, -0.5%] | - | **DIFFER** | device 5.0x faster than the host; tape/mesh 0.48x; 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/view/firstHit/float/hostdevice` | 3.55 ms | 3.55 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 9.9x faster than the host; view/mesh 0.36x |
| `MeshTape/view/frame/float/hostdevice` | 5.46 ms | 5.46 ms | -0.1% | [-0.3%, +0.0%] | - | **DIFFER** | device 4.3x faster than the host; view/mesh 0.38x; 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/mesh/firstHit/double/hostdevice` | 27.4 ms | 27.4 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 27.4 ms | 27.4 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 23.8 ms | 23.7 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 39.1 ms | 39.1 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 0.9x faster than the host; tape/mesh 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 27.5 ms | 27.4 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 0.8x faster than the host; tape/mesh 1.00x |
| `MeshTape/tape/frame/double/hostdevice` | 30.2 ms | 30.4 ms | +0.5% | [-1.0%, +2.0%] | - | ~ | device 0.7x faster than the host; tape/mesh 0.78x |
| `MeshTape/view/firstHit/double/hostdevice` | 39.1 ms | 39.1 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 1.0x faster than the host (was 0.9x); view/mesh 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 32.9 ms | 33.1 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 0.7x faster than the host; view/mesh 0.72x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 680 µs | 683 µs | +0.5% | [-0.0%, +1.0%] | - | ~ | device 11.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.84 ms | 1.84 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 998 µs | 994 µs | -0.4% | [-0.6%, -0.1%] | - | ~ | device 9.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.59 ms | 4.59 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 5.8x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.95 ms | 2.97 ms | +0.7% | [-0.0%, +1.4%] | - | ~ | device 6.1x faster than the host (was 5.6x) |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.10 ms | 7.09 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.80 ms | 7.77 ms | -0.5% | [-0.9%, -0.0%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 12.0 ms | 11.9 ms | -0.5% | [-0.8%, -0.1%] | - | ~ | device 2.7x faster than the host (was 3.0x) |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.45 ms | 1.48 ms | +2.5% | [+1.2%, +3.8%] | - | ~ | device 13.1x faster than the host (was 18.7x) |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.88 ms | 3.89 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 7.9x faster than the host (was 6.3x) |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 525 µs | 530 µs | +1.0% | [+0.3%, +1.7%] | - | ~ | device 34.5x faster than the host (was 25.0x) |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.49 ms | 4.51 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 13.3x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.32 ms | 4.30 ms | -0.5% | [-1.1%, +0.0%] | - | ~ | device 9.5x faster than the host (was 12.0x) |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.04 ms | 8.12 ms | +0.9% | [-1.8%, +3.6%] | - | ~ | device 2.4x faster than the host (was 2.0x) |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.7 ms | -0.6% | [-1.4%, +0.3%] | - | ~ | device 1.2x faster than the host (was 1.3x) |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.02 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 3.6x faster than the host (was 2.6x) |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.6 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 2.2x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.9 ms | 15.1 ms | +0.9% | [-1.2%, +3.1%] | - | ~ | device 2.0x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.59 ms | +0.6% | [+0.0%, +1.1%] | - | ~ | device 13.5x faster than the host (was 10.8x) |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.63 ms | 4.62 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 5.4x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 828 µs | 844 µs | +1.9% | [+0.4%, +3.3%] | - | ~ | device 12.5x faster than the host (was 10.7x) |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.85 ms | 2.86 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 7.3x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.11 ms | 4.12 ms | +0.2% | [-0.6%, +1.0%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 21.0 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.94 ms | 8.92 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.6 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 2.1x faster than the host (was 1.5x) |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.3 ms | 20.3 ms | -0.4% | [-0.9%, +0.0%] | - | ~ | device 1.8x faster than the host (was 2.5x) |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.76 ms | 2.76 ms | -0.0% | [-1.5%, +1.4%] | - | ~ | device 4.9x faster than the host (was 7.2x) |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.72 ms | 6.74 ms | +0.3% | [+0.3%, +0.3%] | - | ~ | device 2.4x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.73 ms | 2.72 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 8.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.40 ms | 6.36 ms | -0.6% | [-0.7%, -0.6%] | - | ~ | device 3.1x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.8 ms | 33.9 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.1 ms | 68.7 ms | +0.9% | [+0.6%, +1.2%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.5 ms | +0.0% | [-1.1%, +1.1%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.0% | [-0.6%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.2 ms | 38.6 ms | +1.0% | [+0.2%, +1.8%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.66 ms | 9.67 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.9 ms | 12.8 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 3.9x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.28 ms | 3.27 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.61 ms | 2.57 ms | -1.6% | [-1.8%, -1.5%] | +0.0% | ~ | device 3.4x faster than the host (was 3.6x) |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.56 ms | 4.55 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.02 ms | 3.05 ms | +1.1% | [+0.5%, +1.7%] | +0.0% | ~ | device 3.3x faster than the host |
| `QueryOrder/random/mortonOrder/float/hostdevice` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-arrived/float/hostdevice` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-ordered/float/hostdevice` | - | - | - | - | - | new | arrived/ordered 1.25x |
| `QueryOrder/random/mortonOrder/double/hostdevice` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-arrived/double/hostdevice` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-ordered/double/hostdevice` | - | - | - | - | - | new | arrived/ordered 1.44x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 680 µs | 680 µs | +0.0% | [-1.6%, +1.8%] | - | ~ | device 27.9x faster than the host (was 22.1x); hand/api 0.95x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 19.3x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 641 µs | 644 µs | +0.4% | [-0.6%, +1.4%] | - | ~ | device 22.5x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.23 ms | +0.8% | [+0.2%, +1.4%] | - | ~ | device 20.1x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 611 µs | 610 µs | -0.2% | [-1.4%, +1.0%] | - | ~ | device 20.6x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.13 ms | 1.12 ms | -0.3% | [-1.3%, +0.8%] | - | ~ | device 20.4x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | -0.1% | [-0.9%, +0.7%] | - | ~ | device 16.1x faster than the host; triangles/polygons 0.58x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.49 ms | +0.6% | [+0.1%, +1.2%] | - | ~ | device 13.1x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.1% | [-0.8%, +1.0%] | - | ~ | device 15.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.17 ms | 2.18 ms | +0.4% | [-0.4%, +1.3%] | - | ~ | device 13.3x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 593 µs | 595 µs | +0.4% | [-0.1%, +0.9%] | - | ~ | device 3.0x faster than the host; tape/fused 1.15x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 699 µs | 700 µs | +0.1% | [-9.8%, +10.9%] | - | ~ | device 3.1x faster than the host (was 3.3x); tape/fused 1.22x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 687 µs | 685 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 4.9x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 854 µs | 857 µs | +0.3% | [-0.3%, +0.9%] | - | ~ | device 4.5x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.1 ms | +0.7% | [-0.0%, +1.5%] | - | ~ | device 0.9x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.3% | [-1.1%, +0.5%] | - | ~ | device 1.0x faster than the host (was 1.1x); hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 0.9x faster than the host (was 0.9x) |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.2 ms | 14.9 ms | -2.0% | [-2.4%, -1.7%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.4% | [-0.3%, +1.1%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.6% | [-1.6%, +0.4%] | - | ~ | device 1.2x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.4 ms | -0.4% | [-0.8%, -0.0%] | - | ~ | device 1.5x faster than the host (was 1.4x); triangles/polygons 1.13x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.1 ms | +1.0% | [-0.3%, +2.3%] | - | ~ | device 1.2x faster than the host (was 1.1x) |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 1.4x faster than the host (was 1.3x) |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | -0.0% | [-1.1%, +1.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.37 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 1.0x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 2.5x faster than the host (was 2.8x) |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.48 ms | 2.48 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.53 ms | 2.52 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 3.7x faster than the host; marched/traced 3.56x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.98 ms | 2.98 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 3.1x faster than the host; marched/traced 4.66x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.01 ms | 8.99 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 6.6x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 5.9x faster than the host (was 4.9x) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.19 ms | 2.20 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 6.9x faster than the host; marched/traced 13.26x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.45 ms | 5.46 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 5.3x faster than the host; marched/traced 10.19x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.2 ms | 29.1 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 4.0x faster than the host (was 3.7x) |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.6 ms | 55.6 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.7x faster than the host (was 3.3x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.26 ms | 5.26 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 3.9x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.85 ms | 8.84 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 4.3x faster than the host (was 4.2x) |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.19 ms | 6.14 ms | -0.8% | [-1.3%, -0.4%] | - | ~ | device 1.8x faster than the host; marched/traced 26.09x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.5 ms | 10.6 ms | +0.5% | [-0.0%, +1.0%] | - | ~ | device 1.1x faster than the host; marched/traced 24.26x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 160 ms | +0.9% | [-1.5%, +3.4%] | - | ~ | device 0.4x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 258 ms | 257 ms | -0.4% | [-1.4%, +0.6%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.89 ms | 4.88 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.4x faster than the host; marched/traced 22.64x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.7x faster than the host (was 1.8x); marched/traced 18.18x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 111 ms | 111 ms | -0.7% | [-2.7%, +1.4%] | - | ~ | device 0.8x faster than the host (was 0.9x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 218 ms | 222 ms | +1.8% | [+0.0%, +3.6%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.5 ms | +0.6% | [+0.6%, +0.7%] | - | ~ | device 1.1x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.4% | [-0.9%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 678 µs | 681 µs | +0.4% | [-1.1%, +2.0%] | - | ~ | device 23.6x faster than the host (was 22.5x) |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.8% | [-1.6%, +0.1%] | - | ~ | device 21.1x faster than the host (was 20.0x) |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 683 µs | 678 µs | -0.7% | [-1.2%, -0.2%] | - | ~ | device 29.3x faster than the host (was 23.5x); hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.29 ms | +1.2% | [+0.6%, +1.9%] | - | ~ | device 28.1x faster than the host; hit/crossing 0.99x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 690 µs | 688 µs | -0.3% | [-0.8%, +0.2%] | - | ~ | device 7.4x faster than the host (was 9.8x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 857 µs | 858 µs | +0.2% | [-0.5%, +0.8%] | - | ~ | device 6.8x faster than the host (was 6.5x) |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 620 µs | 620 µs | -0.0% | [-0.4%, +0.4%] | - | ~ | device 4.0x faster than the host; hit/crossing 1.11x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 640 µs | 643 µs | +0.5% | [-0.4%, +1.5%] | - | ~ | device 4.6x faster than the host; hit/crossing 1.33x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.71 ms | 4.72 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 13.5x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 9.7x faster than the host (was 9.0x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.17 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 13.4x faster than the host (was 14.1x); hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.10 ms | 9.08 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 8.4x faster than the host; hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.1 ms | 56.1 ms | -0.1% | [-1.1%, +1.0%] | - | ~ | device 3.8x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.3 ms | 80.7 ms | -0.8% | [-0.9%, -0.6%] | - | ~ | device 3.1x faster than the host (was 2.8x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 49.3 ms | 49.0 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 3.6x faster than the host; hit/crossing 1.14x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 64.3 ms | 63.9 ms | -0.6% | [-2.0%, +0.8%] | - | ~ | device 2.8x faster than the host (was 2.7x); hit/crossing 1.26x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 16.9 ms | 17.1 ms | +1.2% | [+0.5%, +1.9%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.4 ms | 27.6 ms | +0.5% | [+0.4%, +0.7%] | - | ~ | device 1.0x faster than the host (was 0.7x); hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 3.4x faster than the host (was 2.9x) |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.49 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 1.5x faster than the host (was 2.3x) |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.10 ms | -0.0% | [-0.6%, +0.6%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.26x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.9 ms | 64.6 ms | -0.5% | [-0.7%, -0.2%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 122 ms | 121 ms | -0.8% | [-2.4%, +0.9%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.6 ms | 58.6 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 112 ms | 111 ms | -1.1% | [-2.5%, +0.2%] | - | ~ | device 0.6x faster than the host (was 0.6x); hit/crossing 1.09x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 485 ms | 474 ms | -2.3% | [-9.6%, +5.5%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 706 ms | 697 ms | -1.3% | [-2.6%, +0.2%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 390 ms | 399 ms | +2.4% | [+1.8%, +2.9%] | - | ~ | device 0.5x faster than the host; hit/crossing 1.19x (was 1.25x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 533 ms | 528 ms | -0.9% | [-2.5%, +0.6%] | - | ~ | device 0.5x faster than the host (was 0.4x); hit/crossing 1.32x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.60 ms | 3.55 ms | -1.4% | [-5.0%, +2.5%] | - | ~ | device 1.6x faster than the host; marched/traced 3.38x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.82 ms | 4.80 ms | -0.4% | [-1.9%, +1.1%] | - | ~ | device 1.2x faster than the host (was 1.3x); marched/traced 5.53x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 5.2x faster than the host (was 5.4x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.5 ms | 26.5 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 2.9x faster than the host (was 2.8x) |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.26 ms | +1.9% | [+1.3%, +2.5%] | - | ~ | device 3.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.87 ms | 1.84 ms | -1.4% | [-1.8%, -1.0%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 96.2 µs | 95.3 µs | -1.0% | [-3.4%, +1.4%] | - | ~ | device 18.5x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 152 µs | 153 µs | +0.4% | [-0.8%, +1.5%] | - | ~ | device 13.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 795 µs | 797 µs | +0.2% | [-0.2%, +0.7%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 927 µs | 927 µs | -0.1% | [-0.1%, -0.1%] | - | ~ | device 3.6x faster than the host (was 3.2x) |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 33.8 ms | 35.1 ms | +3.7% | [-2.0%, +9.6%] | - | ~ | device 0.1x faster than the host (was 0.2x); marched/traced 3.48x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.9 ms | 36.8 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 0.1x faster than the host; marched/traced 6.40x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 120 ms | 122 ms | +2.2% | [+0.7%, +3.8%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 230 ms | 236 ms | +2.4% | [+1.0%, +3.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.0 ms | 16.2 ms | +1.1% | [-1.8%, +4.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 21.6 ms | -1.2% | [-1.5%, -0.9%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.05 ms | 1.11 ms | +5.0% | [-1.5%, +11.6%] | - | ~ | device 1.4x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.35 ms | 2.33 ms | -0.7% | [-1.3%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.96 ms | 5.99 ms | +0.5% | [-0.5%, +1.5%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.73 ms | 6.73 ms | +0.0% | [-0.7%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.1 ms | 23.1 ms | -0.2% | [-1.1%, +0.8%] | - | ~ | device 2.2x faster than the host; marched/traced 2.68x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.2 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 2.0x faster than the host; marched/traced 3.75x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.0 ms | 62.0 ms | +0.0% | [-0.7%, +0.7%] | - | ~ | device 3.7x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 104 ms | 106 ms | +1.3% | [+0.1%, +2.6%] | - | ~ | device 2.7x faster than the host (was 2.6x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 14.0 ms | 13.8 ms | -1.2% | [-1.7%, -0.8%] | - | ~ | device 3.7x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.5 ms | 17.5 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 3.6x faster than the host (was 3.2x) |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 310 ms | 311 ms | +0.4% | [-3.7%, +3.1%] | - | ~ | device 0.2x faster than the host; marched/traced 1.80x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 346 ms | 344 ms | -0.7% | [-3.6%, +2.4%] | - | ~ | device 0.2x faster than the host; marched/traced 2.87x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 567 ms | 560 ms | -1.4% | [-2.3%, -0.4%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 991 ms | 986 ms | -0.6% | [-1.0%, -0.1%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 190 ms | 187 ms | -1.8% | [-3.6%, +0.1%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 234 ms | 240 ms | +2.4% | [-1.1%, +6.1%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.75 ms | 2.75 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 2.8x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.72 ms | 3.72 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 3.6x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 97.1 ms | 96.2 ms | -0.9% | [-1.6%, -0.1%] | - | ~ | device 3.8x faster than the host (was 3.4x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.67 ms | 3.67 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 3.9x faster than the host (was 5.4x) |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.72 ms | 6.72 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 3.8x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.51 ms | 5.52 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 3.2x faster than the host; regions/hit 1.22x |
| `Regions/particles/region/double/hostdevice` | 3.75 ms | 3.74 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 2.1x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.81 ms | 5.78 ms | -0.5% | [-1.5%, +0.4%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 137 ms | 138 ms | +0.6% | [-0.5%, +1.7%] | - | ~ | device 2.3x faster than the host (was 2.7x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.93 ms | 5.89 ms | -0.7% | [-1.1%, -0.3%] | - | ~ | device 2.4x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.7 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 13.0 ms | 12.9 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 1.4x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.38 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.37 ms | 2.36 ms | -0.2% | [-0.2%, -0.2%] | - | ~ | device 3.1x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.08 ms | 2.08 ms | -0.1% | [-0.4%, +0.2%] | - | **DIFFER** | device 8.0x faster than the host; 1 host and device answers DIFFER (max diff 0.11) |
| `Render/csg/frame/double/hostdevice` | 5.62 ms | 5.61 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.63 ms | 5.62 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.7 ms | 23.8 ms | +0.2% | [-0.7%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 156 µs | 155 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 33.9x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.6 µs | 47.5 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 18.6x faster than the host |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 15.2x faster than the host |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 65.7x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 48.0 µs | 48.1 µs | +0.2% | [-0.6%, +1.1%] | - | ~ | device 15.3x faster than the host (was 16.1x) |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 47.9 µs | 48.1 µs | +0.3% | [-0.5%, +1.1%] | - | ~ | device 17.4x faster than the host (was 18.2x) |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 48.2 µs | 48.2 µs | -0.0% | [-0.5%, +0.4%] | - | ~ | device 9.7x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.9 µs | 47.9 µs | +0.1% | [-0.5%, +0.6%] | - | ~ | device 7.4x faster than the host |
| `Shapes/wedge/signedDistance/float/hostdevice` | 47.4 µs | 47.5 µs | +0.3% | [+0.1%, +0.4%] | - | ~ | device 8.6x faster than the host (was 7.9x) |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 556 µs | 554 µs | -0.3% | [-0.5%, -0.1%] | - | ~ | device 8.3x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.23 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 492 µs | 492 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 422 µs | +0.1% | [-0.0%, +0.1%] | - | ~ | device 1.9x faster than the host (was 2.2x) |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 3.4x faster than the host (was 2.6x) |
| `Shapes/tube/signedDistance/double/hostdevice` | 359 µs | 357 µs | -0.3% | [-0.5%, -0.0%] | - | ~ | device 3.1x faster than the host (was 3.3x) |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 669 µs | 669 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 414 µs | 413 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 339 µs | +0.1% | [+0.1%, +0.1%] | - | ~ | device 1.4x faster than the host (was 1.2x) |
| `Shapes/wedge/signedDistance/double/hostdevice` | 154 µs | 154 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.04 ms | 8.04 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 891 µs | 890 µs | -0.0% | [-0.8%, +0.7%] | +0.0% | ~ | device 10.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.69 ms | 2.70 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ | device 4.7x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.0 ms | 13.1 ms | +0.4% | [+0.3%, +0.6%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.5 ms | 14.4 ms | -0.5% | [-0.7%, -0.2%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ | device 3.9x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.01 ms | 2.99 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ | device 7.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.76 ms | 4.75 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ | device 5.3x faster than the host (was 5.1x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.8 ms | 16.8 ms | -0.0% | [-0.1%, -0.0%] | +0.0% | ~ | device 5.4x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.4 ms | 24.3 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ | device 4.2x faster than the host (was 3.8x); tape/fused 1.25x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 23.0 ms | 22.9 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ | device 4.0x faster than the host (was 4.2x) |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.5 ms | 30.5 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | device 3.4x faster than the host (was 3.2x) |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.0 ms | 44.2 ms | +0.5% | [-0.2%, +1.2%] | +0.0% | ~ | device 0.5x faster than the host (was 0.5x) |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.7 ms | 45.5 ms | -0.6% | [-0.7%, -0.5%] | +0.0% | ~ | device 0.6x faster than the host (was 0.8x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 125 ms | 124 ms | -0.6% | [-3.7%, +2.7%] | +0.0% | ~ | device 1.0x faster than the host (was 1.1x); tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 125 ms | 129 ms | +2.6% | [-1.3%, +4.0%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 126 ms | 129 ms | +2.2% | [+1.4%, +3.0%] | +0.0% | ~ | device 1.0x faster than the host (was 1.0x) |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 134 ms | 134 ms | -0.3% | [-2.2%, +1.6%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 16.1x faster than the host (was 15.2x) |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.2 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 0.9x faster than the host (was 0.9x) |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.32 ms | 9.33 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 15.4x faster than the host (was 17.1x) |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.93 ms | 5.94 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 18.5x faster than the host (was 13.9x); surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 96.0 ms | 96.7 ms | +0.7% | [-1.7%, +3.2%] | - | ~ | device 4.6x faster than the host (was 5.0x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.3 ms | 63.3 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 4.2x faster than the host; surface/leaf 1.53x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.2 ms | 29.2 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 6.9x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.96 ms | 9.94 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 10.1x faster than the host (was 9.6x); surface/hit 2.94x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.0 ms | 59.0 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.6x faster than the host (was 3.1x) |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.1x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 783 ms | 781 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 529 ms | 529 ms | -0.1% | [-0.9%, +0.8%] | - | ~ | device 0.5x faster than the host; surface/leaf 1.48x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 312 ms | 306 ms | -1.8% | [-3.2%, -0.4%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 120 ms | 122 ms | +1.1% | [-0.2%, +2.3%] | - | ~ | device 0.8x faster than the host; surface/hit 2.52x |
| `Tape/csg/signedDistance/float/hostdevice` | 219 µs | 219 µs | +0.3% | [-0.2%, +0.7%] | - | ~ | device 16.5x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 48.6 µs | 48.5 µs | -0.1% | [-3.5%, +3.5%] | - | ~ | device 33.6x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 361 µs | +0.4% | [-0.8%, +1.5%] | - | ~ | device 21.8x faster than the host (was 18.9x) |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 13.3x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 73.1 µs | 72.8 µs | -0.4% | [-2.1%, +1.3%] | - | ~ | device 9.7x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 48.6 µs | 48.8 µs | +0.3% | [-2.1%, +2.8%] | - | ~ | device 6.4x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 653 µs | 652 µs | -0.1% | [-0.4%, +0.3%] | - | ~ | device 15.9x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 134 µs | 134 µs | +0.1% | [-1.2%, +1.4%] | - | ~ | device 13.5x faster than the host (was 14.5x) |
| `Tape/chain16/signedDistance/float/hostdevice` | 477 µs | 477 µs | -0.2% | [-0.8%, +0.4%] | - | ~ | device 10.9x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 49.3 µs | 49.0 µs | -0.5% | [-4.6%, +3.8%] | - | ~ | device 30.7x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 860 µs | 859 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 4.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 811 µs | 811 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 4.3x faster than the host (was 4.1x) |
| `Tape/union64/signedDistance/double/hostdevice` | 6.66 ms | 6.66 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 5.6x faster than the host (was 3.1x) |
| `Tape/sphere/signedDistance/double/hostdevice` | 192 µs | 194 µs | +1.2% | [-0.7%, +3.2%] | - | ~ | device 7.1x faster than the host (was 7.7x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 164 µs | 171 µs | +4.0% | [-0.3%, +8.5%] | - | ~ | device 4.7x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 3.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 0.9x faster than the host (was 0.9x) |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.11 ms | 2.11 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 11.9x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.72 ms | 3.71 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 14.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.52 ms | 6.51 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 3.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 3.5x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 6195.4 s → 6203.8 s (+0.1%), the largest peak 6548 MB → 6548 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.6 s | 13.9 s | +2.4% | 605 MB | 605 MB | +0.1% | ~ |
| `BVHRefit` | 7.1 s | 7.3 s | +3.7% | 507 MB | 509 MB | +0.3% | ~ |
| `BVHUnion` | 26.3 s | 27.5 s | +4.5% | 1035 MB | 1036 MB | +0.1% | ~ |
| `BVHUnionMixed` | 11.6 s | 11.8 s | +2.1% | 630 MB | 631 MB | +0.1% | ~ |
| `ClosestPrimitive` | 131.8 s | 130.9 s | -0.7% | 1352 MB | 1353 MB | +0.1% | ~ |
| `ClosestSurface` | 196.5 s | 198.3 s | +0.9% | 1844 MB | 1843 MB | -0.0% | ~ |
| `Domain` | 61.4 s | 62.8 s | +2.4% | 998 MB | 998 MB | +0.1% | ~ |
| `DomainObject` | 59.1 s | 59.2 s | +0.1% | 952 MB | 952 MB | +0.0% | ~ |
| `Fused` | 56.7 s | 56.7 s | -0.0% | 1163 MB | 1164 MB | +0.0% | ~ |
| `LaunchTuning` | 563.3 s | 559.4 s | -0.7% | 3859 MB | 3859 MB | -0.0% | ~ |
| `Mesh` | 21.9 s | 21.9 s | +0.1% | 810 MB | 810 MB | +0.1% | ~ |
| `MeshCSG` | 433.3 s | 434.7 s | +0.3% | 6284 MB | 6285 MB | +0.0% | ~ |
| `MeshClosestPoint` | 19.6 s | 20.0 s | +2.2% | 762 MB | 763 MB | +0.1% | ~ |
| `MeshReconcile` | 10.4 s | 10.6 s | +1.8% | 513 MB | 513 MB | +0.0% | ~ |
| `MeshTape` | 794.1 s | 796.4 s | +0.3% | 6548 MB | 6548 MB | +0.0% | ~ |
| `PointCloud` | 12.8 s | 13.1 s | +2.2% | 571 MB | 572 MB | +0.2% | ~ |
| `PointCloudBVHBuild` | 7.4 s | 7.3 s | -1.4% | 508 MB | 509 MB | +0.4% | ~ |
| `PointCloudHashGridBuild` | 6.4 s | 6.5 s | +2.1% | 508 MB | 509 MB | +0.3% | ~ |
| `QueryOrder` | - | 17.0 s | | - | 694 MB | | new |
| `Ray` | 200.0 s | 207.1 s | +3.6% | 3755 MB | 3756 MB | +0.0% | ~ |
| `RayCSG` | 233.4 s | 232.3 s | -0.5% | 4139 MB | 4139 MB | +0.0% | ~ |
| `RayCrossing` | 453.1 s | 455.5 s | +0.5% | 4102 MB | 4102 MB | +0.0% | ~ |
| `RayParticles` | 891.5 s | 892.4 s | +0.1% | 5781 MB | 5780 MB | -0.0% | ~ |
| `RayShell` | 535.3 s | 532.6 s | -0.5% | 5488 MB | 5487 MB | -0.0% | ~ |
| `Regions` | 385.6 s | 385.5 s | -0.0% | 4128 MB | 4128 MB | +0.0% | ~ |
| `Render` | 267.7 s | 270.5 s | +1.0% | 6048 MB | 6047 MB | -0.0% | ~ |
| `Shapes` | 26.9 s | 27.4 s | +1.9% | 592 MB | 593 MB | +0.2% | ~ |
| `SphereCloudBVH` | 9.4 s | 8.9 s | -4.4% | 508 MB | 510 MB | +0.3% | ~ |
| `SphereCloudRays` | 188.5 s | 188.2 s | -0.2% | 4153 MB | 4153 MB | -0.0% | ~ |
| `SphereContacts` | 8.1 s | 8.2 s | +1.5% | 507 MB | 509 MB | +0.3% | ~ |
| `SurfaceParts` | 444.6 s | 442.0 s | -0.6% | 4397 MB | 4397 MB | -0.0% | ~ |
| `Tape` | 71.0 s | 68.9 s | -2.9% | 984 MB | 985 MB | +0.1% | ~ |
| `TapeBVHUnion` | 47.2 s | 46.2 s | -2.1% | 925 MB | 926 MB | +0.1% | ~ |
| **all, in both** | 6195.4 s | 6203.8 s | +0.1% | 6548 MB | 6548 MB | +0.0% | ~ |

Device kernels: 218 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 4 new, 0 gone. Spills are bytes stored/loaded per thread.


Flagged after 2 rounds, and judged on 10 (5 cases): LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice, MeshCSG/union-tape/firstHit-morton/double/hostdevice, MeshCSG/smooth-tape/firstHit-morton/double/hostdevice, RayShell/armadillo-tape/firstHit-morton/double/hostdevice, SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice

The probes differ between the sides (each side ran its own): Performance/QueryOrder/main.cpp

<!-- END receipt cuda -->
