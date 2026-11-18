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
head: 6e2448f
head-fingerprint: 70b0164adf8459187833b348393ae8f2190d3f7b
cases: 276
rounds: 2
reps: 3
date: 2026-11-18
-->

## Host comparison: green

**Accepted:** -

`6e2448f` against `8c7958f` (origin/main), 2026-11-18.

**VERDICT: no regression (0 major, 0 minor across 276 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 276 cases

- base: 8c7958f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 6e2448f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 332 ms | 331 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.2% | [-0.9%, +1.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 515 ms | 515 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 348 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 187 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 563 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.9 ms | 42.2 ms | +0.7% | [-0.3%, +1.7%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 76.0 ms | 76.2 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.1 ms | 56.1 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.6 ms | 51.3 ms | -0.5% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.4 ms | 64.7 ms | +0.4% | [-0.1%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 55.1 ms | 56.1 ms | +1.8% | [-0.7%, +4.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.4 ms | 60.9 ms | +0.8% | [+0.4%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.8 ms | 72.7 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 71.8 ms | 72.1 ms | +0.4% | [-0.2%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.5 ms | 77.9 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.4 ms | 64.5 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 101 ms | 100 ms | -1.0% | [-1.9%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 76.9 ms | 76.8 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 109 ms | +0.5% | [-0.0%, +1.0%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 64.3 ms | 64.2 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 96.0 ms | 95.8 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 76.0 ms | 76.3 ms | +0.3% | [-0.5%, +1.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 101 ms | 101 ms | -0.1% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.3 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.0 ms | 41.8 ms | +1.8% | [+0.6%, +3.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.3 ms | 73.2 ms | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.0 ms | 20.0 ms | +0.2% | [-0.8%, +1.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.6 ms | 43.5 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.5 ms | 81.7 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 140 ms | 139 ms | -0.7% | [-1.5%, -0.0%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 76.5 ms | 76.0 ms | -0.6% | [-2.0%, +0.8%] | +0.0% | ~ | surface/leaf 1.83x |
| `ClosestSurface/union/closestSurface/float/host` | 147 ms | 146 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 77.4 ms | 76.5 ms | -1.1% | [-1.8%, -0.5%] | +0.0% | ~ | surface/leaf 1.91x |
| `ClosestSurface/cloud/closestSurface/double/host` | 157 ms | 157 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 81.3 ms | 80.8 ms | -0.7% | [-0.9%, -0.4%] | +0.0% | ~ | surface/leaf 1.95x |
| `ClosestSurface/union/closestSurface/double/host` | 154 ms | 153 ms | -0.6% | [-1.1%, +0.0%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 81.2 ms | 81.8 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ | surface/leaf 1.87x |
| `Domain/particles/labels/float/host` | 13.5 ms | 13.6 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.77 ms | 6.78 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.36 ms | 1.36 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.0 ms | 14.1 ms | +0.6% | [-0.6%, +1.8%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.13 ms | 7.19 ms | +0.8% | [+0.3%, +1.3%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.36 ms | 1.38 ms | +0.9% | [-0.7%, +2.6%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 39.5 ms | 39.5 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.4 ms | 21.3 ms | -0.4% | [-1.9%, +1.0%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 559 µs | 564 µs | +0.8% | [-0.7%, +2.4%] | +0.0% | ~ | own/whole 70.06x |
| `DomainObject/partsOwn/labelAt/float/host` | 301 µs | 302 µs | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ | own/whole 70.45x |
| `DomainObject/partsWhole/labels/double/host` | 39.4 ms | 39.3 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.6 ms | 26.6 ms | +18.0% | [-1.5%, +37.8%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 588 µs | 578 µs | -1.8% | [-4.6%, +1.1%] | +0.0% | ~ | own/whole 67.94x |
| `DomainObject/partsOwn/labelAt/double/host` | 313 µs | 312 µs | -0.1% | [-2.3%, +2.3%] | +0.0% | ~ | own/whole 85.19x (was 72.16x) |
| `Fused/csg/signedDistance/float/host` | 4.50 ms | 4.49 ms | -0.1% | [-2.0%, +1.9%] | - | ~ | tape/fused 2.91x |
| `Fused/smooth/signedDistance/float/host` | 12.2 ms | 12.2 ms | +0.5% | [-2.0%, +3.1%] | - | ~ | tape/fused 2.10x |
| `Fused/chain16/signedDistance/float/host` | 961 µs | 987 µs | +2.7% | [+1.5%, +4.0%] | - | ~ | tape/fused 16.75x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.4 ms | 46.7 ms | +0.8% | [-0.4%, +2.0%] | +0.0% | ~ | tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.6 ms | 46.5 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.66 ms | 4.56 ms | -2.2% | [-3.3%, -1.1%] | - | ~ | tape/fused 2.92x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.9 ms | +1.4% | [-0.3%, +3.1%] | - | ~ | tape/fused 2.09x |
| `Fused/chain16/signedDistance/double/host` | 3.87 ms | 3.89 ms | +0.5% | [-0.5%, +1.4%] | - | ~ | tape/fused 4.28x |
| `Fused/drilled-mesh/signedDistance/double/host` | 54.4 ms | 54.1 ms | -0.4% | [-3.2%, +2.4%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.3 ms | 54.4 ms | +2.2% | [-0.4%, +4.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 83.1 ms | 82.5 ms | -0.7% | [-1.3%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 236 ms | 232 ms | -1.9% | [-6.1%, +2.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 114 ms | 115 ms | +0.7% | [-2.1%, +3.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 282 ms | 273 ms | -3.1% | [-13.6%, +9.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 72.4 ms | 72.3 ms | -0.2% | [-4.5%, +4.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 220 ms | 219 ms | -0.4% | [-5.9%, +5.4%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 99.4 ms | 99.8 ms | +0.4% | [-4.3%, +5.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.9 ms | 30.9 ms | +3.4% | [-5.3%, +12.9%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 768 ms | 787 ms | +2.5% | [-1.8%, +6.8%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 119 ms | 119 ms | -0.4% | [-2.4%, +1.7%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.3 ms | 45.3 ms | +2.3% | [-3.9%, +8.6%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 879 ms | 890 ms | +1.2% | [-3.5%, +6.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.1 ms | 25.8 ms | -1.1% | [-1.6%, -0.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.6 ms | 15.6 ms | +0.1% | [-4.2%, +4.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 323 ms | 320 ms | -1.2% | [-2.7%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 232 ms | 233 ms | +0.4% | [-0.8%, +1.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 91.2 ms | 90.7 ms | -0.6% | [-1.6%, +0.3%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 82.1 ms | 81.4 ms | -0.8% | [-1.7%, +0.1%] | +0.0% | ~ | tape/fused 2.86x |
| `MeshCSG/union-tape/firstHit/double/host` | 32.2 ms | 31.9 ms | -0.8% | [-2.5%, +5.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.5 ms | 17.4 ms | -0.4% | [-5.0%, +4.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 361 ms | 361 ms | -0.1% | [-1.6%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 247 ms | 246 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.3 ms | 97.6 ms | +0.4% | [-0.8%, +1.6%] | +0.0% | ~ | tape/fused 3.69x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.4 ms | 88.6 ms | +1.4% | [-0.5%, +3.3%] | +0.0% | ~ | tape/fused 2.78x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 233 ms | 235 ms | +0.8% | [-2.3%, +3.8%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 236 ms | 234 ms | -0.8% | [-3.4%, +1.7%] | +0.0% | ~ | closestPoint/signedDistance 1.00x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 329 ms | 313 ms | -4.9% | [-13.1%, +4.6%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 235 ms | 234 ms | -0.2% | [-1.7%, +1.3%] | +0.0% | ~ | gradient/closestPoint 1.00x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 266 ms | 265 ms | -0.3% | [-2.3%, +1.8%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 277 ms | 270 ms | -2.5% | [-8.8%, +4.4%] | +0.0% | ~ | closestPoint/signedDistance 0.98x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 358 ms | 357 ms | -0.5% | [-7.7%, +7.2%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 269 ms | 272 ms | +1.0% | [-3.0%, +5.1%] | +0.0% | ~ | gradient/closestPoint 0.99x (was 1.03x) |
| `MeshTape/mesh/firstHit/float/host` | 47.3 ms | 46.4 ms | -1.9% | [-3.3%, -0.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 48.3 ms | 47.3 ms | -2.2% | [-7.3%, +3.3%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.7 ms | 19.7 ms | -0.0% | [-5.6%, +5.9%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 78.9 ms | 78.6 ms | -0.4% | [-2.6%, +1.8%] | +0.0% | ~ | tape/mesh 0.59x |
| `MeshTape/tape/firstCrossing/float/host` | 49.8 ms | 48.5 ms | -2.5% | [-5.2%, +0.3%] | +0.0% | ~ | tape/mesh 0.97x |
| `MeshTape/tape/frame/float/host` | 24.4 ms | 24.3 ms | -0.3% | [-4.8%, +4.7%] | +0.0% | ~ | tape/mesh 0.81x |
| `MeshTape/view/firstHit/float/host` | 85.3 ms | 81.6 ms | -4.3% | [-10.0%, +2.1%] | +0.0% | ~ | view/mesh 0.57x |
| `MeshTape/view/frame/float/host` | 26.2 ms | 26.0 ms | -0.9% | [-3.2%, +1.4%] | +0.0% | ~ | view/mesh 0.76x |
| `MeshTape/mesh/firstHit/double/host` | 62.5 ms | 62.6 ms | +0.1% | [-4.6%, +7.7%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 55.3 ms | 55.0 ms | -0.5% | [-9.2%, +8.9%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.9 ms | 20.9 ms | +0.1% | [-6.3%, +6.8%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 89.3 ms | 88.0 ms | -1.4% | [-5.3%, +2.7%] | +0.0% | ~ | tape/mesh 0.71x |
| `MeshTape/tape/firstCrossing/double/host` | 56.0 ms | 54.5 ms | -2.6% | [-5.1%, -0.1%] | +0.0% | ~ | tape/mesh 1.01x |
| `MeshTape/tape/frame/double/host` | 27.0 ms | 26.5 ms | -2.0% | [-4.0%, +0.1%] | +0.0% | ~ | tape/mesh 0.79x |
| `MeshTape/view/firstHit/double/host` | 93.2 ms | 92.1 ms | -1.2% | [-6.6%, +4.6%] | +0.0% | ~ | view/mesh 0.68x |
| `MeshTape/view/frame/double/host` | 29.2 ms | 28.5 ms | -2.4% | [-5.5%, +0.9%] | +0.0% | ~ | view/mesh 0.73x |
| `PointCloudBVH/uniform/build/float/host` | 53.8 ms | 53.6 ms | -0.4% | [-1.9%, +1.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 145 ms | 146 ms | +0.3% | [-1.4%, +2.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 366 ms | 369 ms | +0.8% | [-0.8%, +2.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 80.5 ms | 81.0 ms | +0.5% | [-0.0%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.80 ms | 6.83 ms | +0.4% | [-0.8%, +1.6%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.6 ms | 55.7 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 161 ms | 157 ms | -2.5% | [-5.0%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 400 ms | 400 ms | +0.1% | [-1.3%, +1.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 88.3 ms | 88.0 ms | -0.4% | [-1.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.91 ms | 6.89 ms | -0.3% | [-1.0%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.9 ms | 13.6 ms | -1.6% | [-4.6%, +1.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 119 ms | +1.0% | [+0.4%, +1.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 482 ms | 480 ms | -0.4% | [-1.5%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 144 ms | 144 ms | -0.3% | [-1.8%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 808 ms | 809 ms | +0.1% | [-1.2%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 14.9 ms | -0.5% | [-2.1%, +1.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 121 ms | 121 ms | +0.0% | [-1.2%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 496 ms | 500 ms | +0.7% | [-0.9%, +2.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 146 ms | 146 ms | -0.1% | [-1.5%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 784 ms | 804 ms | +2.5% | [-2.8%, +8.1%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 55.3 ms | 54.3 ms | -1.9% | [-3.4%, -0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 131 ms | 132 ms | +0.6% | [-2.0%, +3.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 273 ms | 272 ms | -0.4% | [-2.3%, +1.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 152 ms | 150 ms | -1.3% | [-3.6%, +1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 138 ms | 137 ms | -0.5% | [-1.5%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.7 ms | 57.7 ms | +0.1% | [-3.1%, +3.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 146 ms | 143 ms | -2.2% | [-5.9%, +1.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 303 ms | 302 ms | -0.4% | [-2.0%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 165 ms | 165 ms | -0.2% | [-2.9%, +2.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 153 ms | 151 ms | -1.6% | [-4.0%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.6 ms | 11.8 ms | +1.5% | [-0.8%, +3.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 115 ms | 116 ms | +0.4% | [-1.3%, +2.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 325 ms | 322 ms | -0.9% | [-3.8%, +2.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 156 ms | 156 ms | -0.4% | [-1.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 322 ms | 322 ms | -0.0% | [-1.0%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.8 ms | 13.7 ms | -0.1% | [-2.0%, +1.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 117 ms | 117 ms | -0.3% | [-1.5%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 352 ms | 351 ms | -0.1% | [-1.1%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 165 ms | 166 ms | +0.3% | [-0.6%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 313 ms | 313 ms | -0.1% | [-0.9%, +0.7%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.6 ms | 35.9 ms | +0.8% | [-1.1%, +2.7%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.8 ms | 38.5 ms | -0.8% | [-3.1%, +1.6%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 48.3 ms | 46.6 ms | -3.6% | [-7.9%, +1.1%] | +0.0% | ~ | hand/api 0.99x (was 0.95x) |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.8 ms | 46.1 ms | +0.7% | [-0.9%, +2.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 41.0 ms | 40.6 ms | -1.0% | [-3.0%, +1.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 85.7 ms | 82.0 ms | -4.3% | [-14.6%, +7.7%] | +0.0% | ~ | triangles/polygons 0.57x |
| `Ray/mesh-polygons/intersects/float/host` | 68.0 ms | 68.6 ms | +0.8% | [-6.0%, +8.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.21 ms | 5.20 ms | -0.0% | [-1.1%, +1.0%] | - | ~ | tape/fused 1.46x |
| `Ray/csg-tape/firstHit/float/host` | 7.82 ms | 7.59 ms | -2.9% | [-4.2%, -1.6%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 53.7 ms | 53.8 ms | +0.2% | [-4.1%, +4.7%] | +0.0% | ~ | hand/api 0.98x (was 1.03x) |
| `Ray/mesh/firstHit-handrolled/double/host` | 55.1 ms | 52.7 ms | -4.3% | [-13.2%, +6.0%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 46.0 ms | 45.4 ms | -1.3% | [-4.9%, +2.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 96.7 ms | 90.1 ms | -6.9% | [-16.1%, +4.3%] | +0.0% | ~ | triangles/polygons 0.60x (was 0.56x) |
| `Ray/mesh-polygons/intersects/double/host` | 75.8 ms | 75.4 ms | -0.5% | [-3.3%, +2.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.52 ms | 5.47 ms | -0.9% | [-1.6%, -0.1%] | - | ~ | tape/fused 1.45x |
| `Ray/csg-tape/firstHit/double/host` | 7.96 ms | 7.92 ms | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.9 ms | 17.5 ms | -2.0% | [-3.7%, -0.3%] | - | ~ | marched/traced 6.85x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 120 ms | -0.5% | [-1.2%, +0.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.7 ms | 22.0 ms | +1.0% | [+0.8%, +1.3%] | +0.0% | ~ | marched/traced 6.63x (was 6.95x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 151 ms | 146 ms | -3.7% | [-4.2%, -3.3%] | +0.0% | faster |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 36.0 ms | 35.6 ms | -1.1% | [-2.4%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.2 ms | 19.0 ms | -1.2% | [-2.0%, -0.3%] | - | ~ | marched/traced 6.73x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 128 ms | 128 ms | -0.1% | [-1.6%, +1.5%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 23.4 ms | -0.7% | [-2.1%, +0.7%] | +0.0% | ~ | marched/traced 6.52x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 153 ms | 152 ms | -0.7% | [-4.2%, +2.9%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.6 ms | 38.6 ms | -0.2% | [-4.4%, +4.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 48.5 ms | 48.9 ms | +0.7% | [-8.2%, +10.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 49.4 ms | 50.3 ms | +1.8% | [-11.3%, +16.6%] | +0.0% | ~ | hit/crossing 0.97x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.69 ms | 7.68 ms | -0.1% | [-2.5%, +2.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.18 ms | 6.14 ms | -0.7% | [-1.8%, +0.4%] | +0.0% | ~ | hit/crossing 1.25x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 192 ms | 192 ms | +0.0% | [-15.1%, +17.8%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 171 ms | 171 ms | +0.3% | [-16.1%, +19.7%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 74.5 ms | 75.3 ms | +1.1% | [-11.6%, +15.7%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 61.4 ms | 61.9 ms | +0.9% | [-13.9%, +18.1%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 56.7 ms | 57.3 ms | +1.1% | [-13.2%, +17.7%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 57.7 ms | 58.7 ms | +1.7% | [-16.8%, +23.8%] | +0.0% | ~ | hit/crossing 0.98x |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.03 ms | 7.98 ms | -0.6% | [-2.4%, +1.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.38 ms | 6.35 ms | -0.4% | [-2.2%, +1.4%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 219 ms | 217 ms | -1.1% | [-15.3%, +15.4%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 195 ms | 194 ms | -0.5% | [-17.0%, +19.1%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 87.0 ms | 87.1 ms | +0.1% | [-18.7%, +23.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 74.3 ms | 74.1 ms | -0.2% | [-22.4%, +27.8%] | +0.0% | ~ | hit/crossing 1.17x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.6 ms | 11.6 ms | +0.6% | [-7.9%, +9.9%] | +0.0% | ~ | marched/traced 4.93x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 56.6 ms | 57.3 ms | +1.1% | [-15.0%, +20.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.57 ms | 8.36 ms | -2.5% | [-12.4%, +8.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.02 ms | 1.98 ms | -2.0% | [-8.9%, +5.5%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.84 ms | 6.92 ms | +1.2% | [-2.4%, +4.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.3 ms | 12.3 ms | -0.2% | [-8.5%, +8.9%] | +0.0% | ~ | marched/traced 5.67x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 68.9 ms | 69.8 ms | +1.3% | [-20.9%, +29.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 9.14 ms | 9.62 ms | +5.3% | [-9.2%, +21.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.00 ms | 2.05 ms | +2.7% | [-7.0%, +13.2%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.19 ms | 7.32 ms | +1.7% | [-0.2%, +3.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 247 ms | 249 ms | +0.6% | [-6.8%, +8.6%] | +0.0% | ~ | marched/traced 1.42x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 356 ms | 353 ms | -0.9% | [-14.5%, +15.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 246 ms | 246 ms | +0.1% | [-6.9%, +7.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 266 ms | 266 ms | -0.1% | [-7.5%, +7.7%] | +0.0% | ~ | marched/traced 1.55x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 408 ms | 413 ms | +1.2% | [-16.6%, +22.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 260 ms | 262 ms | +0.8% | [-6.9%, +9.2%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.41 ms | 7.60 ms | +2.6% | [+0.4%, +4.9%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.7 ms | 12.8 ms | +1.0% | [-1.0%, +3.1%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 271 ms | 272 ms | +0.4% | [-2.9%, +3.9%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.6 ms | 12.6 ms | +0.2% | [-1.8%, +2.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 31.1 ms | 31.0 ms | -0.5% | [-4.1%, +3.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.5 ms | 22.5 ms | -0.2% | [-3.2%, +2.8%] | +0.0% | ~ | regions/hit 1.38x |
| `Regions/particles/region/double/host` | 7.82 ms | 7.80 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 13.1 ms | 13.1 ms | +0.1% | [-2.1%, +2.3%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 289 ms | 288 ms | -0.2% | [-3.2%, +2.9%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.3 ms | 13.3 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.8 ms | 33.1 ms | +0.7% | [-1.9%, +3.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 24.1 ms | 24.1 ms | +0.2% | [-3.3%, +3.9%] | +0.0% | ~ | regions/hit 1.37x |
| `Render/csg/frame/float/host` | 12.8 ms | 12.2 ms | -5.1% | [-9.8%, +0.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 12.7 ms | 12.2 ms | -4.0% | [-10.1%, +2.8%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.9 ms | 20.5 ms | +2.7% | [-5.7%, +11.8%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.2 ms | 14.3 ms | +0.9% | [-1.1%, +3.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 15.2 ms | 14.1 ms | -6.7% | [-13.2%, +0.7%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.1 ms | 21.1 ms | +0.2% | [-6.7%, +7.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 58.9 ms | 59.1 ms | +0.4% | [-8.1%, +9.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 38.2 ms | 38.3 ms | +0.1% | [-9.7%, +11.0%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 102 ms | 100 ms | -1.5% | [-17.3%, +17.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 67.1 ms | 67.3 ms | +0.3% | [-8.5%, +9.8%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 44.6 ms | 44.6 ms | +0.0% | [-9.8%, +10.9%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 124 ms | 123 ms | -0.6% | [-23.8%, +29.6%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 84.7 ms | 84.9 ms | +0.2% | [-20.5%, +26.3%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 335 ms | 337 ms | +0.5% | [-20.6%, +27.2%] | +0.0% | ~ | tape/fused 1.03x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 348 ms | 348 ms | -0.0% | [-14.3%, +16.1%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 101 ms | 100 ms | -0.6% | [-23.2%, +28.9%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 404 ms | 415 ms | +2.8% | [-18.7%, +30.8%] | +0.0% | ~ | tape/fused 0.99x (was 1.03x) |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 414 ms | 411 ms | -1.0% | [-21.6%, +24.8%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 180 ms | 183 ms | +1.8% | [-11.2%, +16.7%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 209 ms | 213 ms | +2.2% | [-10.7%, +17.4%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 149 ms | 147 ms | -1.3% | [-4.1%, +1.5%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 91.7 ms | 91.4 ms | -0.4% | [-4.3%, +3.7%] | +0.0% | ~ | surface/leaf 1.61x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 140 ms | 138 ms | -1.2% | [-11.5%, +10.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 77.5 ms | 78.1 ms | +0.7% | [-11.8%, +14.9%] | +0.0% | ~ | surface/leaf 1.77x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 65.8 ms | 65.7 ms | -0.2% | [-13.1%, +14.7%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 38.5 ms | 37.3 ms | -3.1% | [-14.2%, +8.8%] | +0.0% | ~ | surface/hit 1.76x (was 1.71x) |
| `SurfaceParts/boxes/closestSurface/double/host` | 149 ms | 148 ms | -0.4% | [-3.8%, +3.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 93.2 ms | 92.6 ms | -0.6% | [-3.9%, +2.7%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 159 ms | 159 ms | -0.1% | [-16.4%, +19.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 91.0 ms | 91.1 ms | +0.1% | [-17.6%, +21.8%] | +0.0% | ~ | surface/leaf 1.75x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 75.3 ms | 73.4 ms | -2.6% | [-16.5%, +13.9%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 42.2 ms | 42.5 ms | +0.7% | [-14.0%, +17.6%] | +0.0% | ~ | surface/hit 1.73x (was 1.79x) |
| `Tape/csg/signedDistance/float/host` | 13.1 ms | 13.1 ms | -0.2% | [-2.3%, +2.0%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.65 ms | 4.63 ms | -0.4% | [-4.4%, +3.7%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.5 ms | 25.7 ms | +0.4% | [-4.1%, +5.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.8 ms | 40.9 ms | +0.1% | [-4.1%, +4.5%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.79 ms | 1.79 ms | -0.2% | [-4.4%, +4.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 146 µs | 128 µs | -12.2% | [-22.6%, +0.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.6 ms | 30.6 ms | -0.0% | [-3.3%, +3.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.72 ms | 5.73 ms | +0.1% | [-1.6%, +1.8%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.4 ms | 16.5 ms | +0.7% | [-3.8%, +5.4%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.75 ms | 4.72 ms | -0.5% | [-1.3%, +0.2%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.4 ms | 13.3 ms | -0.9% | [-4.2%, +2.5%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.63 ms | 4.69 ms | +1.2% | [-2.1%, +4.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 27.3 ms | 27.0 ms | -1.2% | [-6.0%, +3.8%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 51.2 ms | 42.7 ms | -16.7% | [-31.4%, +4.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.78 ms | 1.80 ms | +1.2% | [-1.3%, +3.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 298 µs | 304 µs | +1.9% | [-4.1%, +8.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.8 ms | 30.6 ms | -0.9% | [-4.2%, +2.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.80 ms | 7.82 ms | +0.2% | [-1.9%, +2.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.7 ms | 16.6 ms | -0.6% | [-5.3%, +4.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.83 ms | 7.85 ms | +0.2% | [-2.9%, +3.4%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 43.0 ms | 42.8 ms | -0.6% | [-4.6%, +3.6%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 205 ms | 205 ms | -0.0% | [-3.8%, +4.0%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 44.1 ms | 43.7 ms | -1.0% | [-5.8%, +4.2%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 221 ms | 221 ms | +0.2% | [-3.6%, +4.2%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 684.3 s → 708.6 s (+3.6%), the largest peak 1399 MB → 1409 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.5 s | +5.1% | 557 MB | 558 MB | +0.1% | ~ |
| `BVHRefit` | 3.9 s | 4.5 s | +13.2% | 484 MB | 496 MB | +2.6% | ~ |
| `BVHUnion` | 17.8 s | 19.2 s | +8.2% | 897 MB | 948 MB | +5.7% | ~ |
| `BVHUnionMixed` | 7.3 s | 7.8 s | +7.2% | 555 MB | 556 MB | +0.1% | ~ |
| `ClosestPrimitive` | 28.9 s | 29.9 s | +3.7% | 1297 MB | 1297 MB | -0.0% | ~ |
| `ClosestSurface` | 27.4 s | 28.3 s | +3.3% | 1220 MB | 1228 MB | +0.6% | ~ |
| `Domain` | 17.5 s | 18.0 s | +3.0% | 841 MB | 842 MB | +0.2% | ~ |
| `DomainObject` | 15.4 s | 15.9 s | +3.0% | 844 MB | 846 MB | +0.2% | ~ |
| `Fused` | 23.3 s | 24.5 s | +5.0% | 1127 MB | 1136 MB | +0.8% | ~ |
| `LaunchTuning` | 34.8 s | 36.6 s | +4.9% | 1304 MB | 1311 MB | +0.5% | ~ |
| `Mesh` | 14.3 s | 15.3 s | +6.9% | 784 MB | 784 MB | -0.0% | ~ |
| `MeshCSG` | 37.7 s | 38.9 s | +3.3% | 1255 MB | 1256 MB | +0.1% | ~ |
| `MeshClosestPoint` | 12.9 s | 14.0 s | +8.8% | 780 MB | 781 MB | +0.1% | ~ |
| `MeshReconcile` | 6.4 s | 6.4 s | +1.1% | 565 MB | 566 MB | +0.1% | ~ |
| `MeshTape` | 39.8 s | 41.5 s | +4.4% | 1287 MB | 1288 MB | +0.1% | ~ |
| `PointCloud` | 7.1 s | 7.3 s | +3.6% | 558 MB | 558 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.6 s | 3.9 s | +5.9% | 470 MB | 471 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 3.2 s | 3.3 s | +3.0% | 452 MB | 452 MB | +0.1% | ~ |
| `Ray` | 34.9 s | 36.2 s | +3.5% | 1318 MB | 1318 MB | -0.0% | ~ |
| `RayCSG` | 31.1 s | 32.3 s | +3.9% | 1183 MB | 1188 MB | +0.4% | ~ |
| `RayCrossing` | 44.6 s | 46.0 s | +3.2% | 1329 MB | 1331 MB | +0.1% | ~ |
| `RayParticles` | 47.4 s | 48.9 s | +3.2% | 1317 MB | 1319 MB | +0.2% | ~ |
| `RayShell` | 36.9 s | 37.5 s | +1.6% | 1280 MB | 1285 MB | +0.4% | ~ |
| `Regions` | 28.8 s | 29.3 s | +1.6% | 1070 MB | 1073 MB | +0.3% | ~ |
| `Render` | 34.6 s | 35.5 s | +2.6% | 1281 MB | 1284 MB | +0.3% | ~ |
| `Shapes` | 8.7 s | 8.9 s | +1.5% | 554 MB | 554 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.8 s | +1.7% | 511 MB | 511 MB | +0.0% | ~ |
| `SphereCloudRays` | 27.5 s | 27.8 s | +1.4% | 1062 MB | 1064 MB | +0.2% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | -0.2% | 484 MB | 484 MB | +0.0% | ~ |
| `SurfaceParts` | 39.7 s | 40.8 s | +2.8% | 1399 MB | 1409 MB | +0.7% | ~ |
| `Tape` | 16.3 s | 16.8 s | +2.7% | 819 MB | 830 MB | +1.4% | ~ |
| `TapeBVHUnion` | 14.5 s | 14.9 s | +2.5% | 823 MB | 818 MB | -0.6% | ~ |
| **all, in both** | 684.3 s | 708.6 s | +3.6% | 1399 MB | 1409 MB | +0.7% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): MeshCSG/union-tape/firstHit/double/host, MeshTape/mesh/firstHit/double/host

These probes compiled to the same code on both sides and were not run (26 cases): MeshReconcile (4), PointCloudHashGridBuild (2), Shapes (20)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 8c7958f
base-fingerprint: 444341ad48073521e98b9c88662429cb15c2889c
head: 6e2448f
head-fingerprint: 70b0164adf8459187833b348393ae8f2190d3f7b
cases: 370
rounds: 2
reps: 3
date: 2026-11-18
-->

## Device comparison (CUDA): red

**Accepted:** The one-pixel DIFFER is the host/device shading difference of issue #343, also on main.

`6e2448f` against `8c7958f` (origin/main), 2026-11-18.

**VERDICT: host and device answers differ in 4 cases; investigate before merging (see Flagged above)**

Performance comparison: 0 major, 0 minor, 15 faster, 4 differ, 0 lost, 370 cases

- base: 8c7958f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 6e2448f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.60 ms | 1.50 ms | -6.2% | [-13.0%, +0.7%] | - | ~ | device 24.9x faster than the host (was 23.2x) |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.41 ms | 3.30 ms | -3.3% | [-25.5%, +7.6%] | - | ~ | device 21.2x faster than the host (was 18.8x) |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.24 ms | 1.06 ms | -14.7% | [-27.3%, +2.7%] | - | ~ | device 19.6x faster than the host (was 16.9x) |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.33 ms | +0.9% | [-0.4%, +27.0%] | - | ~ | device 16.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.34 ms | 6.28 ms | -1.0% | [-9.6%, +3.4%] | - | ~ | device 7.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 12.2 ms | 11.9 ms | -2.0% | [-4.0%, -0.1%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 30.5 ms | 33.2 ms | +8.8% | [-0.6%, +20.2%] | - | ~ | device 1.1x faster than the host (was 1.2x) |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 52.2 ms | 52.1 ms | -0.3% | [-17.2%, +20.0%] | - | ~ | device 1.3x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 19.1 ms | 19.0 ms | -0.6% | [-16.9%, +18.6%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 44.7 ms | 45.6 ms | +2.0% | [-9.6%, +15.2%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 57.3 ms | 58.8 ms | +2.7% | [-11.0%, +18.5%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 105 ms | 102 ms | -3.4% | [-12.6%, +6.8%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.26 ms | 8.40 ms | +1.8% | [-1.0%, +4.6%] | +0.0% | ~ | device 5.1x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 11.3 ms | 9.99 ms | -11.2% | [-22.3%, +2.9%] | +0.0% | ~ | device 8.5x faster than the host (was 7.7x) |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 386 µs | 387 µs | +0.3% | [-0.0%, +0.5%] | - | ~ | device 15.3x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 844 µs | 848 µs | +0.5% | [-0.2%, +1.1%] | - | ~ | device 10.3x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.53 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.63 ms | 4.66 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 589 µs | 588 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 12.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.38 ms | +0.7% | [+0.2%, +1.1%] | - | ~ | device 8.4x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 7.03 ms | 6.78 ms | -3.6% | [-7.1%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.03 ms | 7.01 ms | -0.3% | [-0.3%, -0.3%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 908 µs | 909 µs | +0.1% | [-1.2%, +1.3%] | - | ~ | device 14.6x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.23 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.50 ms | 9.49 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.2 ms | +1.0% | [-0.9%, +3.0%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 850 µs | 849 µs | -0.1% | [-0.5%, +0.2%] | - | ~ | device 13.5x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.09 ms | 2.07 ms | -0.6% | [-1.7%, +0.6%] | - | ~ | device 7.9x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.84 ms | 9.07 ms | -7.9% | [-14.3%, -0.5%] | - | faster | device 1.3x faster than the host (was 1.2x) |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.83 ms | 9.80 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 314 µs | 314 µs | -0.1% | [-0.4%, +0.1%] | - | ~ | device 11.8x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 6.3x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.4 ms | 60.2 ms | +1.3% | [-1.5%, +4.2%] | - | ~ | device 3.3x faster than the host (was 4.1x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.19 ms | 1.19 ms | -0.1% | [-17.6%, +21.0%] | - | ~ | device 3.2x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 4.92 ms | 5.05 ms | +2.5% | [-12.6%, +20.6%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 534 ms | 532 ms | -0.4% | [-6.9%, +6.6%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 21.1 ms | 19.6 ms | -7.3% | [-14.0%, +0.0%] | - | ~ | device 4.3x faster than the host (was 4.1x) |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 7.63 ms | 7.65 ms | +0.3% | [-16.3%, +20.2%] | - | ~ | device 5.3x faster than the host (was 5.5x); surface/leaf 2.56x (was 2.77x) |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 16.2 ms | 15.5 ms | -4.2% | [-8.0%, -0.1%] | - | faster | device 8.6x faster than the host (was 8.1x) |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.18 ms | 6.18 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 10.6x faster than the host; surface/leaf 2.51x (was 2.62x) |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 139 ms | 152 ms | +9.0% | [-0.7%, +19.6%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 57.1 ms | 57.0 ms | -0.1% | [-10.1%, +11.0%] | - | ~ | device 0.8x faster than the host; surface/leaf 2.66x (was 2.44x) |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 73.2 ms | 68.4 ms | -6.7% | [-13.2%, +0.1%] | - | ~ | device 1.9x faster than the host (was 1.7x) |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 26.1 ms | 27.6 ms | +5.7% | [-2.8%, +15.6%] | - | ~ | device 2.4x faster than the host (was 2.5x); surface/leaf 2.48x (was 2.81x) |
| `Domain/particles/labels/float/hostdevice` | 3.87 ms | 4.48 ms | +15.7% | [-0.7%, +33.8%] | - | ~ | device 3.2x faster than the host (was 3.8x) |
| `Domain/particles/labelAt/float/hostdevice` | 2.62 ms | 2.49 ms | -5.1% | [-17.0%, +7.7%] | - | ~ | device 3.0x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 117 µs | 117 µs | -0.5% | [-5.9%, +5.1%] | - | ~ | device 12.7x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.09 ms | 7.12 ms | +0.4% | [-3.8%, +4.8%] | - | ~ | device 2.0x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 4.07 ms | 3.44 ms | -15.4% | [-28.1%, +2.2%] | - | ~ | device 2.1x faster than the host (was 1.8x) |
| `Domain/particles/surfaceGroups/double/hostdevice` | 127 µs | 130 µs | +1.9% | [+0.7%, +3.2%] | - | ~ | device 10.8x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 13.9 ms | 14.6 ms | +4.9% | [-19.0%, +34.5%] | - | ~ | device 3.2x faster than the host |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.50 ms | 8.48 ms | -0.2% | [-6.6%, +6.6%] | - | ~ | device 2.7x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 252 µs | 253 µs | +0.4% | [-11.1%, +13.5%] | - | ~ | device 2.6x faster than the host (was 2.5x); own/whole 57.47x (was 55.01x) |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 147 µs | 147 µs | -0.1% | [-1.5%, +1.3%] | - | ~ | device 2.5x faster than the host (was 2.3x); own/whole 57.71x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 19.8 ms | 22.5 ms | +13.5% | [-0.8%, +28.0%] | - | ~ | device 2.2x faster than the host (was 2.4x) |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 11.3 ms | 12.6 ms | +11.5% | [-3.0%, +26.5%] | - | ~ | device 2.0x faster than the host (was 2.2x) |
| `DomainObject/partsOwn/labels/double/hostdevice` | 383 µs | 444 µs | +15.9% | [-1.0%, +34.2%] | - | ~ | device 1.6x faster than the host (was 1.7x); own/whole 50.74x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 222 µs | 221 µs | -0.5% | [-7.1%, +6.6%] | - | ~ | device 1.7x faster than the host; own/whole 57.10x (was 50.95x) |
| `Fused/csg/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.2% | [-0.2%, +0.6%] | - | ~ | device 23.9x faster than the host; tape/fused 3.84x |
| `Fused/smooth/signedDistance/float/hostdevice` | 52.7 µs | 49.0 µs | -7.0% | [-13.2%, -0.1%] | - | faster | device 44.3x faster than the host (was 41.1x); tape/fused 6.20x (was 5.73x) |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.7 µs | 48.3 µs | +1.2% | [-0.8%, +3.2%] | - | ~ | device 9.2x faster than the host; tape/fused 9.08x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.7 ms | 17.9 ms | +1.0% | [-3.9%, +6.0%] | - | ~ | device 7.2x faster than the host (was 6.6x); tape/fused 1.74x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.2 ms | 31.1 ms | -0.1% | [-9.3%, +18.6%] | - | ~ | device 3.9x faster than the host (was 4.0x) |
| `Fused/csg/signedDistance/double/hostdevice` | 750 µs | 751 µs | +0.1% | [-14.4%, +17.1%] | - | ~ | device 1.6x faster than the host; tape/fused 1.14x (was 1.31x) |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.44 ms | 1.44 ms | +0.0% | [-14.7%, +17.3%] | - | ~ | device 1.7x faster than the host; tape/fused 1.21x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.07 ms | 2.01 ms | -2.8% | [-8.6%, +3.3%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 0.93x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 292 ms | 278 ms | -4.6% | [-10.2%, +1.3%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x (was 0.96x) |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 279 ms | 280 ms | +0.6% | [-4.0%, +5.3%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.20 ms | 7.79 ms | -5.0% | [-11.1%, +1.1%] | +0.0% | ~ | device 11.4x faster than the host (was 10.8x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 9.33 ms | 9.64 ms | +3.3% | [-21.2%, +34.1%] | +0.0% | ~ | device 9.4x faster than the host (was 9.7x); default/this 0.81x (was 0.88x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 9.60 ms | 8.24 ms | -14.2% | [-25.4%, +0.8%] | +0.0% | ~ | device 10.7x faster than the host (was 9.5x); default/this 0.95x (was 0.85x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.28 ms | 8.78 ms | +6.0% | [-0.4%, +12.5%] | +0.0% | ~ | device 10.2x faster than the host (was 10.8x); default/this 0.89x (was 0.99x) |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 708 µs | 1.31 ms | +84.9% | [-4.1%, +177.9%] | +0.0% | ~ | device 2.5x faster than the host (was 3.7x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.79 ms | 1.80 ms | +0.6% | [-0.3%, +1.4%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.73x (was 0.40x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.03 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ | device 2.5x faster than the host; default/this 1.27x (was 0.68x) |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 828 µs | 838 µs | +1.3% | [-5.0%, +7.9%] | +0.0% | ~ | device 3.0x faster than the host; default/this 1.56x (was 0.86x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 737 µs | 743 µs | +0.8% | [-0.4%, +2.0%] | +0.0% | ~ | device 12.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 819 µs | 828 µs | +1.0% | [+0.0%, +2.0%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 739 µs | 737 µs | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | device 12.5x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 733 µs | 723 µs | -1.3% | [-2.0%, -0.7%] | +0.0% | ~ | device 12.2x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 153 ms | 150 ms | -1.9% | [-11.4%, +8.8%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 153 ms | 151 ms | -1.5% | [-10.7%, +8.7%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 162 ms | 152 ms | -6.0% | [-12.7%, +0.7%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x); default/this 0.99x (was 0.95x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 159 ms | 163 ms | +2.2% | [-0.7%, +5.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.92x (was 0.96x) |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.31 ms | 1.35 ms | +3.0% | [-5.3%, +12.3%] | +0.0% | ~ | device 2.0x faster than the host (was 2.0x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.80 ms | 2.79 ms | -0.1% | [-0.9%, +0.7%] | +0.0% | ~ | device 0.9x faster than the host; default/this 0.48x (was 0.47x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.97 ms | 1.95 ms | -1.2% | [-3.1%, +0.7%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.69x (was 0.66x) |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.52 ms | 1.51 ms | -0.4% | [-5.0%, +4.4%] | +0.0% | ~ | device 1.8x faster than the host; default/this 0.89x (was 0.86x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.22 ms | 7.23 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.31 ms | 7.28 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.25 ms | 7.24 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.29 ms | 7.35 ms | +0.9% | [+0.7%, +1.1%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.98x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.37 ms | 8.44 ms | +0.8% | [+0.3%, +1.3%] | - | ~ | device 11.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.7 ms | 16.7 ms | +0.3% | [-0.6%, +1.1%] | - | ~ | device 7.1x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 157 ms | 157 ms | +0.4% | [-12.9%, +15.5%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 280 ms | 276 ms | -1.2% | [-4.3%, +2.1%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.6 ms | 24.3 ms | -1.2% | [-1.9%, -0.5%] | - | ~ | device 4.5x faster than the host (was 4.0x) |
| `MeshSDF/query-random/float/hostdevice` | 59.5 ms | 61.9 ms | +4.0% | [-2.2%, +10.3%] | - | ~ | device 2.4x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 152 ms | 149 ms | -2.1% | [-8.1%, +4.5%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 241 ms | 245 ms | +1.5% | [-2.7%, +7.6%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 1.82 ms | 1.83 ms | +0.4% | [-15.5%, +19.4%] | - | ~ | device 5.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.35 ms | 3.35 ms | -0.2% | [-1.5%, +1.0%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.05 ms | 6.06 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 4.8x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 61.0 ms | 63.5 ms | +4.0% | [-1.4%, +9.7%] | - | ~ | device 3.2x faster than the host (was 3.4x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 87.8 ms | 89.1 ms | +1.6% | [-7.4%, +11.3%] | - | ~ | device 2.5x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 231 ms | 241 ms | +4.8% | [-2.1%, +12.0%] | - | ~ | device 3.4x faster than the host (was 3.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 20.7 ms | 24.9 ms | +19.9% | [-1.2%, +21.5%] | - | ~ | device 2.4x faster than the host (was 2.5x); tape/fused 2.55x (was 2.95x) |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 24.8 ms | 25.0 ms | +0.9% | [-13.6%, +17.9%] | - | ~ | device 2.4x faster than the host; tape/fused 3.56x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.5 ms | 61.3 ms | -6.5% | [-13.0%, +0.1%] | - | ~ | device 4.3x faster than the host (was 3.9x); tape/fused 3.94x (was 3.52x) |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 18.9 ms | 20.5 ms | +8.1% | [-19.3%, +40.6%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 34.6 ms | 34.1 ms | -1.6% | [-19.0%, +19.0%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 36.6 ms | 37.0 ms | +1.0% | [-16.9%, +22.0%] | - | ~ | device 0.8x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 531 ms | 515 ms | -3.0% | [-3.4%, -2.7%] | - | faster | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 888 ms | 871 ms | -1.9% | [-7.8%, +4.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.09 s | 2.05 s | -1.7% | [-8.7%, +6.0%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 185 ms | 193 ms | +4.2% | [-1.4%, +10.0%] | - | ~ | device 0.3x faster than the host; tape/fused 2.67x (was 2.87x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 307 ms | 310 ms | +0.8% | [-4.9%, +6.7%] | - | ~ | device 0.2x faster than the host; tape/fused 2.81x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 556 ms | 558 ms | +0.5% | [-10.0%, +12.1%] | - | ~ | device 0.5x faster than the host (was 0.6x); tape/fused 3.68x |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 12.2 ms | 12.4 ms | +1.5% | [-2.2%, +5.3%] | - | ~ | device 9.6x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 62.7 ms | 62.5 ms | -0.2% | [-9.2%, +9.6%] | - | ~ | device 2.4x faster than the host |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 292 ms | 263 ms | -9.9% | [-18.9%, +1.1%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 270 ms | 252 ms | -6.6% | [-19.3%, +8.6%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.28 ms | 1.20 ms | -5.8% | [-11.8%, +0.2%] | - | ~ | device 17.1x faster than the host (was 15.7x) |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.29 ms | 1.41 ms | +9.5% | [-0.4%, +19.5%] | - | ~ | device 14.5x faster than the host (was 15.8x) |
| `MeshTape/mesh/frame/float/hostdevice` | 2.08 ms | 2.14 ms | +2.8% | [+0.9%, +4.7%] | - | **DIFFER** | device 8.1x faster than the host; 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/tape/firstHit/float/hostdevice` | 2.92 ms | 3.33 ms | +14.1% | [-12.2%, +26.2%] | - | ~ | device 11.5x faster than the host; tape/mesh 0.36x (was 0.44x) |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.81 ms | 3.33 ms | +18.7% | [-9.7%, +50.2%] | - | ~ | device 6.3x faster than the host (was 7.7x); tape/mesh 0.42x (was 0.46x) |
| `MeshTape/tape/frame/float/hostdevice` | 5.46 ms | 4.31 ms | -21.1% | [-35.3%, +1.0%] | - | **DIFFER** | device 5.0x faster than the host (was 4.1x); tape/mesh 0.50x (was 0.38x); 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/view/firstHit/float/hostdevice` | 3.68 ms | 3.55 ms | -3.5% | [-6.9%, +0.1%] | - | ~ | device 10.2x faster than the host; view/mesh 0.34x |
| `MeshTape/view/frame/float/hostdevice` | 5.46 ms | 6.11 ms | +12.0% | [-0.2%, +24.2%] | - | **DIFFER** | device 4.1x faster than the host (was 4.3x); view/mesh 0.35x (was 0.38x); 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/mesh/firstHit/double/hostdevice` | 27.4 ms | 28.6 ms | +4.2% | [-3.2%, +29.4%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 26.2 ms | 26.3 ms | +0.1% | [-8.9%, +10.1%] | - | ~ | device 0.8x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 22.4 ms | 22.2 ms | -1.0% | [-11.7%, +11.0%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `MeshTape/tape/firstHit/double/hostdevice` | 37.0 ms | 38.7 ms | +4.7% | [-1.0%, +10.9%] | - | ~ | device 0.9x faster than the host (was 1.0x); tape/mesh 0.74x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 22.7 ms | 23.4 ms | +2.9% | [-5.3%, +21.4%] | - | ~ | device 0.9x faster than the host; tape/mesh 1.12x |
| `MeshTape/tape/frame/double/hostdevice` | 29.1 ms | 27.5 ms | -5.3% | [-17.4%, +7.8%] | - | ~ | device 0.9x faster than the host (was 0.8x); tape/mesh 0.81x (was 0.77x) |
| `MeshTape/view/firstHit/double/hostdevice` | 37.4 ms | 35.6 ms | -4.9% | [-18.2%, +9.7%] | - | ~ | device 1.1x faster than the host (was 1.1x); view/mesh 0.80x (was 0.73x) |
| `MeshTape/view/frame/double/hostdevice` | 31.5 ms | 31.9 ms | +1.1% | [-5.7%, +8.5%] | - | ~ | device 0.9x faster than the host; view/mesh 0.70x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 643 µs | 650 µs | +1.1% | [-9.1%, +12.5%] | - | ~ | device 13.0x faster than the host (was 15.3x) |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.84 ms | 2.11 ms | +14.5% | [-0.3%, +29.3%] | - | ~ | device 6.3x faster than the host (was 9.1x) |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 994 µs | 996 µs | +0.2% | [-0.2%, +0.6%] | - | ~ | device 10.7x faster than the host (was 11.3x) |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.61 ms | 4.91 ms | +6.5% | [-1.0%, +14.1%] | - | ~ | device 6.1x faster than the host (was 6.6x) |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.96 ms | 2.96 ms | -0.2% | [-1.2%, +0.8%] | - | ~ | device 6.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.09 ms | 7.09 ms | +0.0% | [-0.7%, +0.7%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 9.11 ms | 7.79 ms | -14.5% | [-25.7%, +0.5%] | - | ~ | device 1.7x faster than the host (was 1.5x) |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 14.7 ms | 14.7 ms | +0.1% | [-16.8%, +20.5%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 21.0 ms | 20.8 ms | -0.9% | [-1.8%, -0.1%] | - | ~ | device 1.6x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 12.4 ms | 11.0 ms | -11.2% | [-21.4%, -0.1%] | - | faster | device 1.8x faster than the host (was 1.5x) |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.33 ms | 1.31 ms | -1.9% | [-18.4%, +17.7%] | - | ~ | device 8.6x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.69 ms | 3.69 ms | +0.0% | [-11.1%, +12.5%] | - | ~ | device 4.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 523 µs | 523 µs | +0.2% | [-0.3%, +0.6%] | - | ~ | device 18.6x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.50 ms | 4.54 ms | +1.0% | [+0.2%, +1.8%] | - | ~ | device 8.1x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.27 ms | 4.63 ms | +8.5% | [-1.6%, +18.7%] | - | ~ | device 6.1x faster than the host (was 6.7x) |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.84 ms | 8.20 ms | -7.3% | [-13.8%, +0.2%] | - | ~ | device 1.5x faster than the host (was 1.4x) |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.9 ms | 15.8 ms | -0.5% | [-0.7%, -0.4%] | - | ~ | device 1.0x faster than the host (was 1.0x) |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 3.89 ms | 3.89 ms | -0.0% | [-5.6%, +6.0%] | - | ~ | device 2.5x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.5 ms | 17.6 ms | +0.7% | [-11.2%, +22.1%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 13.8 ms | 13.8 ms | +0.1% | [-13.6%, +16.0%] | - | ~ | device 2.0x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.50 ms | 1.49 ms | -0.1% | [-12.0%, +13.7%] | - | ~ | device 13.1x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.68 ms | 5.31 ms | +13.6% | [-1.4%, +28.9%] | - | ~ | device 5.4x faster than the host (was 6.2x) |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 833 µs | 835 µs | +0.3% | [-0.2%, +0.7%] | - | ~ | device 11.9x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.54 ms | 3.35 ms | -5.5% | [-32.6%, +34.8%] | - | ~ | device 7.1x faster than the host (was 6.8x) |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.12 ms | 4.12 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 6.7x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.9 ms | 18.4 ms | +9.3% | [-4.3%, +23.4%] | - | ~ | device 1.1x faster than the host (was 1.2x) |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 18.1 ms | 18.2 ms | +0.6% | [-1.3%, +23.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.81 ms | 9.07 ms | +3.0% | [+0.2%, +5.8%] | - | ~ | device 1.2x faster than the host (was 1.3x) |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.6 ms | +0.4% | [-0.5%, +1.3%] | - | ~ | device 1.7x faster than the host (was 1.8x) |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 18.8 ms | 20.7 ms | +9.8% | [-0.6%, +21.9%] | - | ~ | device 1.5x faster than the host (was 1.6x) |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.64 ms | 2.48 ms | -5.9% | [-17.8%, +6.9%] | - | ~ | device 6.3x faster than the host (was 5.7x) |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 7.64 ms | 6.99 ms | -8.5% | [-21.5%, +8.0%] | - | ~ | device 2.7x faster than the host (was 2.5x) |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.26 ms | 1.27 ms | +0.5% | [-16.7%, +21.3%] | - | ~ | device 8.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.71 ms | 2.57 ms | -5.4% | [-10.7%, -0.1%] | - | faster | device 10.3x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 7.00 ms | 6.52 ms | -6.9% | [-14.4%, +1.8%] | - | ~ | device 3.8x faster than the host (was 3.5x) |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 31.3 ms | 28.4 ms | -9.2% | [-15.6%, +19.9%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 61.0 ms | 64.7 ms | +6.2% | [-11.3%, +21.1%] | - | ~ | device 0.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 17.5 ms | 16.9 ms | -3.3% | [-18.2%, +13.5%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.4 ms | 21.3 ms | +4.7% | [-1.4%, +11.1%] | - | ~ | device 1.2x faster than the host (was 1.3x) |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 37.6 ms | 36.6 ms | -2.6% | [-9.7%, +4.9%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.5 ms | 12.4 ms | -0.9% | [-4.7%, +3.1%] | +0.0% | ~ | device 3.9x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.69 ms | 9.60 ms | -0.9% | [-1.4%, -0.3%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 14.2 ms | +11.5% | [-0.3%, +23.5%] | +0.0% | ~ | device 3.6x faster than the host (was 3.9x) |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ | device 4.0x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 685 µs | 676 µs | -1.4% | [-2.5%, -0.2%] | - | ~ | device 21.0x faster than the host (was 27.3x); hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.35 ms | 1.36 ms | +0.5% | [-10.6%, +13.1%] | - | ~ | device 16.6x faster than the host; hand/api 0.90x (was 0.85x) |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 651 µs | 647 µs | -0.6% | [-2.0%, +0.8%] | - | ~ | device 20.1x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.16 ms | 1.22 ms | +5.6% | [-0.4%, +12.3%] | - | ~ | device 18.5x faster than the host (was 21.1x) |
| `Ray/mesh/intersects-morton/float/hostdevice` | 610 µs | 989 µs | +62.0% | [-1.6%, +126.9%] | - | ~ | device 13.1x faster than the host (was 18.2x) |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | +0.5% | [-0.4%, +1.3%] | - | ~ | device 18.5x faster than the host (was 17.9x) |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.32 ms | 1.32 ms | -0.0% | [-19.3%, +24.1%] | - | ~ | device 12.5x faster than the host (was 13.7x); triangles/polygons 0.51x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.98 ms | +20.3% | [-0.0%, +40.7%] | - | ~ | device 10.0x faster than the host (was 11.8x); triangles/polygons 0.46x (was 0.55x) |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.01 ms | 1.07 ms | +5.3% | [-0.8%, +12.1%] | - | ~ | device 14.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.19 ms | 2.12 ms | -3.4% | [-6.8%, +0.1%] | - | ~ | device 12.8x faster than the host (was 11.6x) |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 597 µs | 653 µs | +9.5% | [-0.7%, +19.8%] | - | ~ | device 2.6x faster than the host (was 2.6x); tape/fused 1.08x (was 1.20x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 716 µs | 732 µs | +2.3% | [-0.7%, +25.5%] | - | ~ | device 2.5x faster than the host (was 2.6x); tape/fused 1.17x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 714 µs | 708 µs | -0.9% | [-5.6%, +4.1%] | - | ~ | device 4.0x faster than the host (was 3.9x) |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 856 µs | 856 µs | +0.0% | [-0.7%, +0.7%] | - | ~ | device 4.6x faster than the host (was 4.3x) |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.1 ms | 16.9 ms | -0.7% | [-2.1%, +0.7%] | - | ~ | device 0.8x faster than the host (was 0.7x); hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 26.6 ms | 26.6 ms | +0.2% | [-6.4%, +7.2%] | - | ~ | device 0.8x faster than the host; hand/api 1.03x (was 0.94x) |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.5% | [-1.2%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 25.0 ms | 27.5 ms | +9.8% | [-0.2%, +21.8%] | - | ~ | device 0.9x faster than the host (was 0.9x) |
| `Ray/mesh/intersects-morton/double/hostdevice` | 13.6 ms | 13.7 ms | +1.2% | [-17.5%, +24.2%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `Ray/mesh/intersects-random/double/hostdevice` | 21.1 ms | 22.0 ms | +4.1% | [-16.3%, +20.2%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 15.2 ms | 15.0 ms | -1.0% | [-17.9%, +19.2%] | - | ~ | device 1.3x faster than the host; triangles/polygons 1.13x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.4 ms | 24.5 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 1.7x faster than the host (was 1.2x); triangles/polygons 1.09x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 14.3 ms | 14.2 ms | -1.0% | [-11.4%, +10.5%] | - | ~ | device 1.0x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 22.9 ms | 22.9 ms | +0.0% | [-0.8%, +0.9%] | - | ~ | device 1.1x faster than the host (was 1.0x) |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.24 ms | 1.24 ms | +0.1% | [-0.9%, +1.1%] | - | ~ | device 1.4x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.36 ms | 2.36 ms | +0.0% | [-1.2%, +1.3%] | - | ~ | device 1.0x faster than the host (was 1.0x); tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.38 ms | 1.38 ms | -0.0% | [-0.8%, +0.8%] | - | ~ | device 2.5x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.48 ms | -0.7% | [-1.2%, -0.2%] | - | ~ | device 1.6x faster than the host (was 2.2x) |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.53 ms | 2.53 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 3.1x faster than the host (was 2.8x); marched/traced 3.56x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.99 ms | 3.00 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 2.6x faster than the host; marched/traced 4.63x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 8.99 ms | 9.01 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 5.8x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.9 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 4.4x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.19 ms | 2.19 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 4.3x faster than the host; marched/traced 13.26x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.46 ms | 5.46 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.3x faster than the host; marched/traced 9.94x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 28.3 ms | 29.1 ms | +2.9% | [-0.2%, +6.3%] | - | ~ | device 2.7x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 54.2 ms | 54.3 ms | +0.1% | [-4.4%, +4.8%] | - | ~ | device 2.4x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.08 ms | 5.06 ms | -0.2% | [-7.0%, +6.9%] | - | ~ | device 3.3x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.63 ms | 8.63 ms | +0.1% | [-4.7%, +5.0%] | - | ~ | device 3.6x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.18 ms | 5.74 ms | -7.2% | [-15.0%, +0.7%] | - | ~ | device 1.5x faster than the host (was 1.2x); marched/traced 26.16x (was 24.33x) |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.4 ms | 10.6 ms | +1.8% | [+0.8%, +2.7%] | - | ~ | device 0.8x faster than the host (was 0.8x); marched/traced 23.45x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 150 ms | 150 ms | -0.2% | [-12.6%, +13.9%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 247 ms | 249 ms | +1.0% | [-5.4%, +7.8%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.64 ms | 4.63 ms | -0.2% | [-10.1%, +10.9%] | - | ~ | device 2.3x faster than the host; marched/traced 23.63x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 11.8 ms | 11.8 ms | +0.7% | [-6.9%, +8.9%] | - | ~ | device 1.6x faster than the host; marched/traced 18.31x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 112 ms | 109 ms | -2.4% | [-6.6%, +2.0%] | - | ~ | device 0.7x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 216 ms | 217 ms | +0.4% | [-2.5%, +3.3%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.5 ms | +1.0% | [+0.3%, +1.7%] | - | ~ | device 0.9x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.9 ms | +0.6% | [+0.3%, +0.9%] | - | ~ | device 1.1x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 675 µs | 676 µs | +0.0% | [-0.2%, +0.3%] | - | ~ | device 17.6x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 15.8x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 675 µs | 677 µs | +0.3% | [-0.7%, +1.3%] | - | ~ | device 18.9x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 18.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 696 µs | 698 µs | +0.3% | [-1.9%, +2.4%] | - | ~ | device 5.4x faster than the host (was 4.1x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 851 µs | 851 µs | +0.1% | [-1.0%, +1.2%] | - | ~ | device 4.1x faster than the host (was 4.3x) |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 620 µs | 619 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 3.5x faster than the host; hit/crossing 1.13x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 639 µs | 639 µs | +0.0% | [-0.9%, +0.9%] | - | ~ | device 4.1x faster than the host; hit/crossing 1.33x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.71 ms | 4.72 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 12.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 8.6x faster than the host (was 8.2x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.17 ms | -0.2% | [-0.5%, -0.0%] | - | ~ | device 11.8x faster than the host (was 12.7x); hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.09 ms | 9.08 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 8.3x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 52.8 ms | 52.7 ms | -0.1% | [-8.0%, +10.5%] | - | ~ | device 3.5x faster than the host (was 3.7x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.6 ms | 80.4 ms | -1.5% | [-2.8%, -0.1%] | - | ~ | device 2.9x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.7 ms | 49.6 ms | +1.9% | [-0.2%, +4.0%] | - | ~ | device 3.6x faster than the host (was 3.9x); hit/crossing 1.06x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.6 ms | 63.7 ms | +0.3% | [-0.7%, +1.2%] | - | ~ | device 3.0x faster than the host; hit/crossing 1.26x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.6% | [-1.2%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 0.9x faster than the host (was 0.9x) |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 16.9 ms | 16.9 ms | -0.0% | [-0.9%, +0.9%] | - | ~ | device 0.8x faster than the host (was 0.9x); hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 0.9x faster than the host (was 0.9x); hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.5x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.49 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 1.7x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.10 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 2.2x faster than the host; hit/crossing 1.26x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 2.4x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.3 ms | 64.4 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 121 ms | 123 ms | +1.6% | [+0.5%, +2.8%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.7 ms | 58.4 ms | -0.4% | [-0.8%, -0.1%] | - | ~ | device 0.9x faster than the host (was 0.9x); hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 112 ms | 112 ms | -0.2% | [-1.1%, +0.7%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.09x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 468 ms | 467 ms | -0.1% | [-4.6%, +4.7%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 729 ms | 713 ms | -2.2% | [-4.0%, -0.5%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 408 ms | 370 ms | -9.3% | [-11.7%, -6.9%] | - | faster | device 0.4x faster than the host (was 0.4x); hit/crossing 1.26x (was 1.15x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 544 ms | 538 ms | -1.1% | [-3.5%, +1.4%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.33x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.61 ms | 3.68 ms | +1.7% | [+0.8%, +2.6%] | - | ~ | device 1.2x faster than the host; marched/traced 3.26x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.80 ms | 4.76 ms | -0.8% | [-1.7%, +0.0%] | - | ~ | device 0.9x faster than the host; marched/traced 5.58x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 4.0x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.5 ms | 26.5 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.26 ms | 1.25 ms | -1.1% | [-2.0%, -0.2%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.84 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 97.3 µs | 96.7 µs | -0.5% | [-2.6%, +1.6%] | - | ~ | device 15.3x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 154 µs | 152 µs | -1.2% | [-2.7%, +0.3%] | - | ~ | device 10.8x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 794 µs | 793 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.0x faster than the host (was 3.1x) |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 926 µs | 926 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 36.0 ms | 34.3 ms | -4.8% | [-8.3%, -1.1%] | - | faster | device 0.1x faster than the host (was 0.1x); marched/traced 3.34x (was 3.24x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 38.9 ms | 37.3 ms | -4.2% | [-9.9%, +2.2%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 6.34x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 117 ms | 114 ms | -1.9% | [-10.0%, +6.8%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 243 ms | 236 ms | -2.7% | [-8.3%, +3.4%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 17.0 ms | 17.6 ms | +3.4% | [-3.8%, +13.7%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 23.4 ms | 21.9 ms | -6.4% | [-12.0%, -0.1%] | - | faster | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.10 ms | 1.05 ms | -4.5% | [-8.1%, -0.5%] | - | faster | device 1.4x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.36 ms | 2.34 ms | -0.8% | [-1.5%, -0.2%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 6.65 ms | 5.95 ms | -10.6% | [-19.1%, -0.1%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.55 ms | 6.56 ms | +0.2% | [-4.5%, +5.2%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 21.2 ms | 22.8 ms | +7.5% | [-3.6%, +20.8%] | - | ~ | device 2.2x faster than the host (was 2.4x); marched/traced 2.60x (was 2.81x) |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 26.5 ms | 26.6 ms | +0.5% | [-13.0%, +16.2%] | - | ~ | device 2.1x faster than the host; marched/traced 3.99x (was 3.84x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 59.5 ms | 59.2 ms | -0.5% | [-9.7%, +9.5%] | - | ~ | device 3.8x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 101 ms | 106 ms | +4.7% | [-0.4%, +10.2%] | - | ~ | device 2.4x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 14.0 ms | 12.8 ms | -9.0% | [-17.6%, -0.1%] | - | faster | device 4.2x faster than the host (was 3.7x) |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 18.0 ms | 18.0 ms | +0.0% | [-7.9%, +8.7%] | - | ~ | device 3.1x faster than the host (was 3.5x) |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 300 ms | 305 ms | +1.7% | [-1.5%, +4.2%] | - | ~ | device 0.2x faster than the host; marched/traced 2.00x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 362 ms | 352 ms | -2.6% | [-5.2%, +0.0%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 2.82x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 590 ms | 609 ms | +3.3% | [-2.0%, +7.8%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.01 s | 994 ms | -1.6% | [-3.1%, +0.0%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 178 ms | 177 ms | -0.2% | [-14.7%, +16.4%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 231 ms | 231 ms | -0.1% | [-9.8%, +11.1%] | - | ~ | device 0.3x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.57 ms | 2.57 ms | -0.2% | [-11.4%, +12.4%] | - | ~ | device 3.0x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.57 ms | 3.23 ms | -9.7% | [-13.4%, +17.5%] | - | ~ | device 4.1x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 96.8 ms | 96.1 ms | -0.7% | [-15.2%, +16.2%] | - | ~ | device 3.6x faster than the host (was 3.7x); each/one-pass 0.03x (was 0.04x) |
| `Regions/particles/domainLabels/float/hostdevice` | 3.42 ms | 3.42 ms | +0.0% | [-17.0%, +20.5%] | - | ~ | device 4.2x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 7.41 ms | 7.14 ms | -3.6% | [-14.7%, +9.5%] | - | ~ | device 3.7x faster than the host (was 3.8x) |
| `Regions/assembly/firstHit/float/hostdevice` | 5.64 ms | 5.64 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 3.2x faster than the host; regions/hit 1.27x (was 1.31x) |
| `Regions/particles/region/double/hostdevice` | 3.64 ms | 3.53 ms | -2.8% | [-9.8%, +4.4%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.90 ms | 5.83 ms | -1.2% | [-2.5%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 138 ms | 139 ms | +1.3% | [-12.7%, +17.4%] | - | ~ | device 2.2x faster than the host (was 2.4x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.03 ms | 5.06 ms | +0.5% | [-2.1%, +3.3%] | - | ~ | device 3.2x faster than the host (was 3.0x) |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.5 ms | 16.1 ms | +4.4% | [-0.8%, +10.0%] | - | ~ | device 2.1x faster than the host (was 1.6x) |
| `Regions/assembly/firstHit/double/hostdevice` | 12.3 ms | 11.6 ms | -5.6% | [-10.7%, +0.2%] | - | ~ | device 1.5x faster than the host (was 1.4x); regions/hit 1.39x (was 1.26x) |
| `Render/csg/frame/float/hostdevice` | 2.38 ms | 2.31 ms | -3.0% | [-6.0%, +0.1%] | - | ~ | device 3.1x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.30 ms | 2.29 ms | -0.0% | [-1.6%, +1.5%] | - | ~ | device 3.1x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.77 ms | 1.76 ms | -0.7% | [-0.8%, -0.6%] | - | **DIFFER** | device 9.4x faster than the host; 1 host and device answers DIFFER (max diff 0.11) |
| `Render/csg/frame/double/hostdevice` | 5.26 ms | 5.13 ms | -2.5% | [-11.8%, +8.2%] | - | ~ | device 1.6x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.77 ms | 4.94 ms | -14.3% | [-21.7%, -5.7%] | - | faster | device 1.7x faster than the host (was 1.4x) |
| `Render/mesh/frame/double/hostdevice` | 20.2 ms | 19.5 ms | -3.7% | [-8.4%, +1.3%] | - | ~ | device 0.9x faster than the host (was 0.9x) |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 798 µs | 793 µs | -0.6% | [-1.6%, +0.5%] | +0.0% | ~ | device 11.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.64 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.2 ms | 13.3 ms | +0.7% | [-5.5%, +7.3%] | +0.0% | ~ | device 3.9x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.3 ms | 10.4 ms | +1.4% | [-0.9%, +12.3%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.2 ms | 13.0 ms | -1.3% | [-1.9%, -0.7%] | +0.0% | ~ | device 0.8x faster than the host (was 0.8x) |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 15.6 ms | 13.5 ms | -13.4% | [-24.7%, -0.0%] | +0.0% | faster | device 1.1x faster than the host (was 0.9x) |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 13.8 ms | 13.8 ms | -0.0% | [-4.2%, +4.3%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.1 ms | 11.2 ms | +0.7% | [-1.6%, +3.2%] | +0.0% | ~ | device 3.9x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.01 ms | 3.01 ms | -0.2% | [-0.9%, +0.6%] | +0.0% | ~ | device 7.4x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.81 ms | 5.29 ms | +9.9% | [-0.1%, +20.0%] | +0.0% | ~ | device 4.7x faster than the host (was 5.1x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.8 ms | 16.9 ms | +0.2% | [-1.6%, +5.4%] | +0.0% | ~ | device 5.3x faster than the host; tape/fused 1.41x (was 1.34x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 25.6 ms | 24.5 ms | -4.3% | [-9.6%, +1.4%] | +0.0% | ~ | device 4.3x faster than the host (was 3.7x); tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.6 ms | 23.8 ms | +5.4% | [-3.7%, +14.8%] | +0.0% | ~ | device 4.1x faster than the host (was 4.4x) |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 32.7 ms | 30.9 ms | -5.5% | [-16.9%, +8.0%] | +0.0% | ~ | device 3.5x faster than the host (was 3.2x) |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 40.2 ms | 39.8 ms | -0.8% | [-14.8%, +15.9%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 43.3 ms | 41.6 ms | -3.9% | [-15.8%, +8.9%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 107 ms | 113 ms | +4.9% | [-1.1%, +11.1%] | +0.0% | ~ | device 0.9x faster than the host (was 0.9x); tape/fused 1.25x (was 1.08x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 116 ms | 112 ms | -3.4% | [-9.2%, +3.0%] | +0.0% | ~ | device 0.9x faster than the host; tape/fused 1.25x (was 1.14x) |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 115 ms | 141 ms | +22.2% | [-7.1%, +53.7%] | +0.0% | ~ | device 0.7x faster than the host (was 0.8x) |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 132 ms | 141 ms | +6.5% | [-16.2%, +34.8%] | +0.0% | ~ | device 0.8x faster than the host (was 0.9x) |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.49 ms | 1.51 ms | +1.5% | [-7.7%, +10.5%] | +0.0% | ~ | device 12.5x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 29.1 ms | 28.9 ms | -0.7% | [-20.4%, +24.1%] | +0.0% | ~ | device 0.7x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.50 ms | 8.78 ms | -7.5% | [-26.6%, +17.0%] | - | ~ | device 12.8x faster than the host (was 12.4x) |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.89 ms | 5.45 ms | -7.6% | [-24.9%, +14.7%] | - | ~ | device 12.0x faster than the host (was 11.5x); surface/leaf 1.61x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 104 ms | 104 ms | -0.8% | [-4.2%, +2.8%] | - | ~ | device 3.5x faster than the host (was 3.4x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 70.7 ms | 66.5 ms | -6.0% | [-12.2%, +0.8%] | - | ~ | device 3.3x faster than the host (was 3.9x); surface/leaf 1.56x (was 1.48x) |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 28.8 ms | 30.8 ms | +6.9% | [-0.5%, +15.4%] | - | ~ | device 5.3x faster than the host (was 6.7x) |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 11.3 ms | 11.1 ms | -1.3% | [-19.8%, +19.7%] | - | ~ | device 7.5x faster than the host (was 6.4x); surface/hit 2.76x (was 2.55x) |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.0 ms | 58.8 ms | -0.4% | [-10.2%, +11.2%] | - | ~ | device 2.1x faster than the host |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.2 ms | 38.4 ms | +0.5% | [-8.8%, +9.9%] | - | ~ | device 1.8x faster than the host; surface/leaf 1.53x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 871 ms | 808 ms | -7.1% | [-16.6%, +4.2%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 546 ms | 554 ms | +1.3% | [-1.3%, +4.1%] | - | ~ | device 0.4x faster than the host (was 0.5x); surface/leaf 1.46x (was 1.59x) |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 312 ms | 317 ms | +1.7% | [-1.9%, +6.5%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 105 ms | 103 ms | -1.2% | [-4.5%, +2.3%] | - | ~ | device 0.9x faster than the host (was 0.8x); surface/hit 3.07x |
| `Tape/csg/signedDistance/float/hostdevice` | 184 µs | 184 µs | -0.1% | [-0.9%, +0.8%] | - | ~ | device 14.9x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 48.4 µs | 48.5 µs | +0.2% | [-3.2%, +3.7%] | - | ~ | device 27.1x faster than the host (was 26.1x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 302 µs | 304 µs | +0.7% | [+0.3%, +1.0%] | - | ~ | device 17.2x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.85 ms | 1.35 ms | -27.0% | [-42.9%, +1.0%] | - | ~ | device 13.0x faster than the host (was 11.7x) |
| `Tape/sphere/signedDistance/float/hostdevice` | 69.3 µs | 66.4 µs | -4.1% | [-10.9%, +3.5%] | - | ~ | device 8.9x faster than the host (was 8.6x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 48.9 µs | 48.0 µs | -2.0% | [-4.9%, +1.0%] | - | ~ | device 6.5x faster than the host (was 5.6x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 598 µs | 599 µs | +0.1% | [-16.4%, +20.0%] | - | ~ | device 15.1x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 122 µs | 123 µs | +1.0% | [-16.2%, +21.7%] | - | ~ | device 14.3x faster than the host (was 13.6x) |
| `Tape/chain16/signedDistance/float/hostdevice` | 437 µs | 439 µs | +0.3% | [-16.1%, +20.0%] | - | ~ | device 9.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.8 µs | 48.8 µs | -0.2% | [-0.8%, +0.4%] | - | ~ | device 29.3x faster than the host (was 28.2x) |
| `Tape/csg/signedDistance/double/hostdevice` | 980 µs | 858 µs | -12.5% | [-22.2%, -0.1%] | - | faster | device 3.5x faster than the host (was 3.1x) |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 957 µs | +18.2% | [-0.0%, +36.4%] | - | ~ | device 1.5x faster than the host (was 1.8x) |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.1x faster than the host (was 3.3x) |
| `Tape/union64/signedDistance/double/hostdevice` | 7.53 ms | 7.71 ms | +2.4% | [-11.5%, +20.0%] | - | ~ | device 2.2x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 192 µs | 192 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 3.6x faster than the host (was 3.3x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 164 µs | 163 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.6x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 3.01 ms | 3.02 ms | +0.5% | [-14.2%, +17.7%] | - | ~ | device 2.6x faster than the host (was 2.8x) |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 0.9x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 1.90 ms | 1.87 ms | -1.4% | [-14.3%, +42.1%] | - | ~ | device 2.1x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.22 ms | 2.15 ms | -3.3% | [-10.0%, +4.1%] | - | ~ | device 1.0x faster than the host (was 1.0x) |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 2.26 ms | 1.81 ms | -20.1% | [-44.6%, +24.0%] | - | ~ | device 9.7x faster than the host (was 7.6x) |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.97 ms | 4.07 ms | +2.6% | [-11.9%, +19.1%] | - | ~ | device 13.7x faster than the host (was 12.8x) |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.96 ms | +6.8% | [-0.2%, +13.9%] | - | ~ | device 2.3x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 18.5 ms | 17.1 ms | -7.3% | [-14.1%, +0.0%] | - | ~ | device 3.2x faster than the host (was 3.0x) |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 6194.3 s → 5997.6 s (-3.2%), the largest peak 6549 MB → 6549 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 14.4 s | 13.5 s | -6.6% | 600 MB | 605 MB | +0.8% | ~ |
| `BVHRefit` | 7.3 s | 6.9 s | -4.6% | 506 MB | 507 MB | +0.1% | ~ |
| `BVHUnion` | 27.2 s | 25.7 s | -5.5% | 1025 MB | 1035 MB | +1.0% | ~ |
| `BVHUnionMixed` | 13.0 s | 11.4 s | -12.3% | 626 MB | 630 MB | +0.7% | ~ |
| `ClosestPrimitive` | 160.7 s | 128.8 s | -19.8% | 1348 MB | 1353 MB | +0.4% | ~ |
| `ClosestSurface` | 213.8 s | 190.7 s | -10.8% | 1843 MB | 1844 MB | +0.1% | ~ |
| `Domain` | 74.7 s | 59.7 s | -20.1% | 995 MB | 998 MB | +0.3% | ~ |
| `DomainObject` | 67.3 s | 57.4 s | -14.8% | 949 MB | 952 MB | +0.3% | ~ |
| `Fused` | 62.1 s | 55.3 s | -10.9% | 1158 MB | 1163 MB | +0.4% | ~ |
| `LaunchTuning` | 562.9 s | 544.1 s | -3.3% | 3859 MB | 3859 MB | +0.0% | ~ |
| `Mesh` | 24.4 s | 21.2 s | -12.9% | 802 MB | 810 MB | +1.0% | ~ |
| `MeshCSG` | 444.3 s | 419.9 s | -5.5% | 6285 MB | 6285 MB | +0.0% | ~ |
| `MeshClosestPoint` | 24.3 s | 19.0 s | -21.7% | 755 MB | 762 MB | +1.0% | ~ |
| `MeshReconcile` | 11.3 s | 10.2 s | -10.2% | 512 MB | 513 MB | +0.1% | ~ |
| `MeshTape` | 787.0 s | 766.5 s | -2.6% | 6549 MB | 6549 MB | +0.0% | ~ |
| `PointCloud` | 17.3 s | 12.6 s | -27.7% | 571 MB | 571 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 7.6 s | 7.3 s | -4.0% | 507 MB | 507 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 6.9 s | 6.5 s | -5.5% | 507 MB | 507 MB | +0.1% | ~ |
| `Ray` | 203.8 s | 196.1 s | -3.8% | 3756 MB | 3756 MB | +0.0% | ~ |
| `RayCSG` | 236.6 s | 227.9 s | -3.7% | 4139 MB | 4140 MB | +0.0% | ~ |
| `RayCrossing` | 452.8 s | 439.8 s | -2.9% | 4102 MB | 4102 MB | +0.0% | ~ |
| `RayParticles` | 857.8 s | 858.1 s | +0.0% | 5781 MB | 5781 MB | +0.0% | ~ |
| `RayShell` | 516.1 s | 516.8 s | +0.1% | 5488 MB | 5488 MB | +0.0% | ~ |
| `Regions` | 372.4 s | 373.3 s | +0.2% | 4128 MB | 4128 MB | +0.0% | ~ |
| `Render` | 258.9 s | 258.9 s | +0.0% | 6048 MB | 6043 MB | -0.1% | ~ |
| `Shapes` | 26.3 s | 25.9 s | -1.3% | 592 MB | 593 MB | +0.0% | ~ |
| `SphereCloudBVH` | 8.5 s | 8.8 s | +2.6% | 507 MB | 508 MB | +0.1% | ~ |
| `SphereCloudRays` | 184.1 s | 183.5 s | -0.3% | 4154 MB | 4154 MB | -0.0% | ~ |
| `SphereContacts` | 7.7 s | 7.9 s | +2.1% | 506 MB | 507 MB | +0.1% | ~ |
| `SurfaceParts` | 428.3 s | 429.3 s | +0.2% | 4397 MB | 4397 MB | +0.0% | ~ |
| `Tape` | 68.2 s | 68.1 s | -0.2% | 982 MB | 985 MB | +0.3% | ~ |
| `TapeBVHUnion` | 46.2 s | 46.6 s | +0.8% | 923 MB | 925 MB | +0.3% | ~ |
| **all, in both** | 6194.3 s | 5997.6 s | -3.2% | 6549 MB | 6549 MB | +0.0% | ~ |

Device kernels: 218 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.


Flagged after 2 rounds, and judged on 10 (25 cases): BVH/mesh-boxes/ray-nearest-random/float/hostdevice, BVH/mesh-boxes/ray-any-random/float/hostdevice, BVH/mesh-boxes/point-nearest-morton/float/hostdevice, Fused/drilled-mesh-tape/signedDistance/float/hostdevice, MeshSDF/query-random/double/hostdevice, MeshCSG/smooth-fused/firstHit-morton/float/hostdevice, MeshTape/tape/firstHit/float/hostdevice, MeshTape/mesh/firstHit/double/hostdevice, MeshTape/tape/firstCrossing/double/hostdevice, PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice, PointCloudBVH/sphere/closest-random/double/hostdevice, PointCloudHashGrid/sphere/closest-morton/double/hostdevice, PointCloudHashGrid/sphere/closest-random/double/hostdevice, Ray/csg-fused/firstHit-random/float/hostdevice, Ray/mesh/intersects-random/double/hostdevice, RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice, RayParticles/armadillo-shell-fused/step-morton/double/hostdevice, RayShell/armadillo-tape/firstHit-morton/double/hostdevice, RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice, Regions/particles/regionDistances/float/hostdevice, SphereCloudBVH/polydisperse/rebuild/float/hostdevice, SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice, SphereContacts/polydisperse/force-morton/float/hostdevice, SurfaceParts/mesh-groups/firstSurface/double/hostdevice, Tape/chain16/signedDistance/double/hostdevice

These probes compiled to the same code on both sides and were not run (26 cases): MeshReconcile (2), PointCloudHashGridBuild (4), Shapes (20)

<!-- END receipt cuda -->
