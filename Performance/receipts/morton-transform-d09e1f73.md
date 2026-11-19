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
head: 5813a7e
head-fingerprint: d09e1f73e272cc4ad11609c8448c384fcd8f001e
cases: 306
rounds: 2
reps: 3
date: 2026-11-19
-->

## Host comparison: green

**Accepted:** -

`5813a7e` against `fd5c8a5` (origin/main), 2026-11-19.

**VERDICT: no regression (0 major, 0 minor across 306 cases)**

Performance comparison: 0 major, 0 minor, 6 faster, 0 differ, 0 lost, 306 cases

- base: fd5c8a5 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 5813a7e (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 347 ms | 347 ms | +0.1% | [-1.2%, +1.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 183 ms | 184 ms | +0.2% | [-1.2%, +1.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 535 ms | 534 ms | -0.1% | [-2.4%, +2.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 373 ms | 365 ms | -2.2% | [-2.9%, -1.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 194 ms | 194 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 586 ms | 592 ms | +1.0% | [-1.1%, +3.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 50.1 ms | 52.0 ms | +3.9% | [-8.8%, +18.8%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 84.5 ms | 84.6 ms | +0.1% | [-4.0%, +4.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 59.0 ms | 58.7 ms | -0.5% | [-2.1%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 54.0 ms | 55.4 ms | +2.6% | [-5.0%, +10.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 66.8 ms | 67.0 ms | +0.2% | [-2.0%, +2.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 58.4 ms | 58.9 ms | +0.8% | [-7.1%, +9.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 63.6 ms | 63.7 ms | +0.1% | [-1.7%, +2.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 78.2 ms | 78.7 ms | +0.6% | [-1.5%, +2.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 75.2 ms | 75.3 ms | +0.1% | [-0.8%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 88.1 ms | 87.9 ms | -0.2% | [-6.8%, +7.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 68.0 ms | 68.1 ms | +0.1% | [-0.9%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 110 ms | 111 ms | +1.0% | [-3.7%, +6.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 79.7 ms | 79.8 ms | +0.1% | [-2.5%, +2.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 128 ms | 131 ms | +2.0% | [-5.9%, +10.8%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 66.3 ms | 66.4 ms | +0.1% | [-1.8%, +2.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 103 ms | 103 ms | +0.4% | [-3.9%, +4.9%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 79.0 ms | 79.3 ms | +0.3% | [-1.2%, +1.9%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 115 ms | 114 ms | -0.4% | [-9.9%, +10.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 20.0 ms | 20.2 ms | +0.9% | [+0.2%, +1.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 42.9 ms | 42.7 ms | -0.5% | [-2.1%, +1.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 79.1 ms | 78.7 ms | -0.5% | [-5.3%, +4.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.6 ms | 20.6 ms | +0.0% | [-1.3%, +1.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.1 ms | 45.1 ms | +0.0% | [-1.4%, +1.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 93.6 ms | 90.9 ms | -2.8% | [-8.3%, +2.8%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 146 ms | 147 ms | +1.0% | [-0.5%, +2.5%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 78.9 ms | 78.9 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ | surface/leaf 1.87x |
| `ClosestSurface/union/closestSurface/float/host` | 155 ms | 156 ms | +0.9% | [-1.3%, +3.1%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 80.3 ms | 80.3 ms | -0.1% | [-2.3%, +2.2%] | +0.0% | ~ | surface/leaf 1.95x |
| `ClosestSurface/cloud/closestSurface/double/host` | 168 ms | 171 ms | +2.0% | [-0.1%, +4.2%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 84.6 ms | 86.0 ms | +1.6% | [+0.7%, +2.6%] | +0.0% | ~ | surface/leaf 1.99x |
| `ClosestSurface/union/closestSurface/double/host` | 172 ms | 173 ms | +0.8% | [+0.7%, +1.0%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 87.5 ms | 87.3 ms | -0.3% | [-1.2%, +0.6%] | +0.0% | ~ | surface/leaf 1.99x |
| `Domain/particles/labels/float/host` | 13.8 ms | 13.8 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.91 ms | 6.92 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.77 ms | 1.45 ms | -18.2% | [-35.4%, +8.8%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.4 ms | 14.4 ms | -0.1% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.30 ms | 7.27 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.38 ms | 1.37 ms | -0.6% | [-1.0%, -0.3%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 41.2 ms | 41.1 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 22.0 ms | 21.9 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 580 µs | 581 µs | +0.1% | [-0.8%, +1.0%] | +0.0% | ~ | own/whole 70.76x |
| `DomainObject/partsOwn/labelAt/float/host` | 311 µs | 308 µs | -1.0% | [-2.1%, +0.2%] | +0.0% | ~ | own/whole 70.99x |
| `DomainObject/partsWhole/labels/double/host` | 40.4 ms | 40.6 ms | +0.4% | [-1.3%, +2.1%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 23.1 ms | 23.0 ms | -0.3% | [-2.4%, +1.7%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 597 µs | 600 µs | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ | own/whole 67.73x |
| `DomainObject/partsOwn/labelAt/double/host` | 320 µs | 319 µs | -0.3% | [-3.3%, +2.7%] | +0.0% | ~ | own/whole 72.28x |
| `Fused/csg/signedDistance/float/host` | 4.52 ms | 4.53 ms | +0.4% | [-2.1%, +2.9%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/float/host` | 12.5 ms | 12.4 ms | -0.1% | [-2.3%, +2.1%] | - | ~ | tape/fused 2.07x |
| `Fused/chain16/signedDistance/float/host` | 988 µs | 987 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | tape/fused 16.95x |
| `Fused/drilled-mesh/signedDistance/float/host` | 49.6 ms | 49.4 ms | -0.6% | [-3.9%, +2.9%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 50.6 ms | 49.4 ms | -2.5% | [-7.9%, +3.5%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.65 ms | 4.60 ms | -1.1% | [-1.8%, -0.4%] | - | ~ | tape/fused 2.92x |
| `Fused/smooth/signedDistance/double/host` | 13.0 ms | 13.0 ms | -0.2% | [-2.3%, +2.0%] | - | ~ | tape/fused 2.07x |
| `Fused/chain16/signedDistance/double/host` | 3.99 ms | 3.94 ms | -1.3% | [-2.4%, -0.1%] | - | ~ | tape/fused 4.23x |
| `Fused/drilled-mesh/signedDistance/double/host` | 59.1 ms | 56.6 ms | -4.2% | [-8.4%, +0.2%] | +0.0% | ~ | tape/fused 1.06x (was 1.02x) |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 60.4 ms | 60.1 ms | -0.6% | [-1.6%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 85.6 ms | 86.5 ms | +1.0% | [+0.3%, +1.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 245 ms | 245 ms | +0.2% | [-2.7%, +3.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 118 ms | 117 ms | -0.9% | [-1.5%, -0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 300 ms | 318 ms | +5.9% | [-11.0%, +24.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 76.3 ms | 73.7 ms | -3.5% | [-8.7%, +2.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 257 ms | 251 ms | -2.6% | [-17.3%, +15.5%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 103 ms | 101 ms | -1.8% | [-5.6%, +2.4%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 39.2 ms | 32.7 ms | -16.4% | [-28.8%, +0.2%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 895 ms | 882 ms | -1.4% | [-4.2%, +1.4%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 124 ms | 123 ms | -0.5% | [-1.1%, +0.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 82.2 ms | 74.4 ms | -9.5% | [-16.3%, -2.5%] | - | faster |  |
| `MeshSDF/query-random/double/host` | 1.18 s | 1.14 s | -2.9% | [-17.2%, +15.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 28.0 ms | 27.8 ms | -0.7% | [-5.6%, +4.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 16.4 ms | 16.8 ms | +2.4% | [-1.7%, +6.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 346 ms | 347 ms | +0.4% | [-5.5%, +6.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 242 ms | 241 ms | -0.2% | [-2.1%, +1.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 96.1 ms | 96.9 ms | +0.8% | [-0.1%, +1.8%] | +0.0% | ~ | tape/fused 3.58x |
| `MeshCSG/smooth-fused/frame/float/host` | 85.2 ms | 84.5 ms | -0.8% | [-3.0%, +1.5%] | +0.0% | ~ | tape/fused 2.85x |
| `MeshCSG/union-tape/firstHit/double/host` | 30.9 ms | 31.0 ms | +0.4% | [-2.1%, +2.9%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 18.7 ms | 18.2 ms | -2.6% | [-4.7%, -0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 412 ms | 426 ms | +3.4% | [-8.5%, +16.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 258 ms | 259 ms | +0.4% | [-1.3%, +2.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 105 ms | 104 ms | -0.4% | [-2.2%, +1.4%] | +0.0% | ~ | tape/fused 4.09x (was 3.94x) |
| `MeshCSG/smooth-fused/frame/double/host` | 95.4 ms | 94.4 ms | -1.0% | [-7.8%, +6.5%] | +0.0% | ~ | tape/fused 2.75x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 259 ms | 286 ms | +10.7% | [-4.9%, +27.3%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 257 ms | 261 ms | +1.2% | [-2.3%, +5.0%] | +0.0% | ~ | closestPoint/signedDistance 1.10x (was 1.00x) |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 349 ms | 360 ms | +3.2% | [-0.8%, +7.6%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 262 ms | 249 ms | -5.2% | [-9.6%, -0.4%] | +0.0% | faster | gradient/closestPoint 1.05x (was 0.98x) |
| `MeshClosestPoint/tri/signedDistance/double/host` | 304 ms | 310 ms | +1.8% | [-1.9%, +5.6%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 314 ms | 320 ms | +2.1% | [+1.5%, +2.6%] | +0.0% | ~ | closestPoint/signedDistance 0.97x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 443 ms | 472 ms | +6.6% | [-1.0%, +15.0%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 314 ms | 306 ms | -2.7% | [-5.4%, -0.1%] | +0.0% | ~ | gradient/closestPoint 1.05x (was 1.00x) |
| `MeshReconcile/armadillo/reconcile/float/host` | 165 ms | 174 ms | +6.1% | [-2.5%, +15.4%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 49.3 ms | 47.1 ms | -4.4% | [-9.8%, +1.1%] | +0.0% | ~ | reconcile/scheduled 3.70x (was 3.33x) |
| `MeshReconcile/armadillo/reconcile/double/host` | 206 ms | 199 ms | -3.5% | [-7.2%, +0.3%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 66.2 ms | 66.7 ms | +0.8% | [-5.7%, +7.8%] | +0.0% | ~ | reconcile/scheduled 2.99x (was 3.12x) |
| `MeshTape/mesh/firstHit/float/host` | 50.3 ms | 51.0 ms | +1.4% | [-7.9%, +11.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 53.3 ms | 53.4 ms | +0.1% | [-9.3%, +10.6%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 21.7 ms | 21.4 ms | -1.2% | [-10.2%, +8.7%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 83.2 ms | 83.5 ms | +0.3% | [-4.4%, +5.4%] | +0.0% | ~ | tape/mesh 0.61x |
| `MeshTape/tape/firstCrossing/float/host` | 53.5 ms | 53.8 ms | +0.6% | [-3.1%, +4.5%] | +0.0% | ~ | tape/mesh 0.99x |
| `MeshTape/tape/frame/float/host` | 25.6 ms | 25.5 ms | -0.1% | [-1.7%, +4.9%] | +0.0% | ~ | tape/mesh 0.84x |
| `MeshTape/view/firstHit/float/host` | 84.8 ms | 85.5 ms | +0.8% | [-2.4%, +5.0%] | +0.0% | ~ | view/mesh 0.60x |
| `MeshTape/view/frame/float/host` | 27.0 ms | 27.3 ms | +1.3% | [-1.0%, +3.7%] | +0.0% | ~ | view/mesh 0.78x |
| `MeshTape/mesh/firstHit/double/host` | 57.8 ms | 56.8 ms | -1.7% | [-4.6%, +1.2%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 58.0 ms | 58.2 ms | +0.4% | [-0.8%, +1.6%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 21.2 ms | 21.7 ms | +2.0% | [+0.8%, +3.2%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 93.1 ms | 94.6 ms | +1.7% | [-0.6%, +3.9%] | +0.0% | ~ | tape/mesh 0.60x (was 0.62x) |
| `MeshTape/tape/firstCrossing/double/host` | 62.5 ms | 63.0 ms | +0.7% | [-0.0%, +1.5%] | +0.0% | ~ | tape/mesh 0.92x |
| `MeshTape/tape/frame/double/host` | 28.3 ms | 28.7 ms | +1.1% | [-0.6%, +2.9%] | +0.0% | ~ | tape/mesh 0.76x |
| `MeshTape/view/firstHit/double/host` | 97.6 ms | 96.3 ms | -1.3% | [-8.6%, +6.5%] | +0.0% | ~ | view/mesh 0.59x |
| `MeshTape/view/frame/double/host` | 30.5 ms | 30.6 ms | +0.4% | [-1.7%, +2.5%] | +0.0% | ~ | view/mesh 0.71x |
| `PointCloudBVH/uniform/build/float/host` | 56.6 ms | 55.9 ms | -1.2% | [-5.6%, +3.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 165 ms | 168 ms | +1.8% | [-7.3%, +12.0%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 394 ms | 393 ms | -0.4% | [-3.4%, +2.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 87.5 ms | 87.3 ms | -0.3% | [-7.9%, +7.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 7.10 ms | 7.17 ms | +1.0% | [-1.6%, +3.7%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 59.4 ms | 59.5 ms | +0.2% | [-3.4%, +4.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 252 ms | 183 ms | -27.2% | [-42.6%, -1.7%] | - | faster |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 420 ms | 455 ms | +8.2% | [-2.9%, +19.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 99.7 ms | 108 ms | +7.9% | [-2.7%, +18.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.60 ms | 7.60 ms | -0.1% | [-12.5%, +14.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.8 ms | 13.6 ms | -1.3% | [-10.3%, +8.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 129 ms | 127 ms | -1.4% | [-10.9%, +9.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 528 ms | 494 ms | -6.5% | [-14.2%, +2.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 149 ms | 149 ms | +0.6% | [-1.3%, +2.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 842 ms | 832 ms | -1.1% | [-3.2%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 15.2 ms | +1.1% | [-2.5%, +5.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 124 ms | 125 ms | +0.0% | [-0.6%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 512 ms | 509 ms | -0.6% | [-2.4%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 159 ms | 159 ms | -0.4% | [-4.8%, +4.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 871 ms | 879 ms | +0.8% | [-5.9%, +7.8%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 57.8 ms | 59.2 ms | +2.3% | [-2.1%, +6.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 152 ms | 152 ms | -0.0% | [-5.3%, +5.5%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 292 ms | 295 ms | +1.0% | [-2.2%, +4.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 163 ms | 162 ms | -0.8% | [-6.7%, +5.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 146 ms | 148 ms | +1.8% | [-7.1%, +11.4%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 60.1 ms | 60.0 ms | -0.1% | [-5.1%, +5.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 163 ms | 164 ms | +1.1% | [-5.7%, +8.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 317 ms | 315 ms | -0.6% | [-3.0%, +1.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 191 ms | 188 ms | -1.4% | [-4.2%, +1.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 168 ms | 171 ms | +2.1% | [-1.9%, +9.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.9 ms | 11.8 ms | -0.2% | [-3.1%, +2.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 118 ms | 120 ms | +0.9% | [-0.2%, +2.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 339 ms | 363 ms | +7.3% | [-3.3%, +18.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 163 ms | 163 ms | +0.1% | [-4.6%, +5.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 338 ms | 336 ms | -0.4% | [-3.6%, +3.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 14.1 ms | 14.0 ms | -0.1% | [-4.2%, +4.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 121 ms | 119 ms | -1.8% | [-3.0%, -0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 368 ms | 373 ms | +1.6% | [-2.0%, +5.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 193 ms | 187 ms | -2.8% | [-4.4%, -1.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 340 ms | 364 ms | +7.0% | [-0.4%, +14.7%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 38.6 ms | 38.9 ms | +0.6% | [-2.3%, +3.6%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 43.6 ms | 42.8 ms | -1.8% | [-7.7%, +4.4%] | +0.0% | ~ |  |
| `QueryOrder/random/mortonOrder/float/host` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | - | - | - | - | - | new | arrived/ordered 1.46x |
| `QueryOrder/random/mortonOrder/double/host` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | - | - | - | - | - | new | arrived/ordered 1.68x |
| `Ray/mesh/firstHit/float/host` | 50.4 ms | 49.7 ms | -1.2% | [-8.8%, +7.2%] | +0.0% | ~ | hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled/float/host` | 49.2 ms | 49.7 ms | +0.9% | [-7.8%, +10.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 43.5 ms | 44.9 ms | +3.4% | [-0.6%, +7.8%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 98.4 ms | 95.7 ms | -2.7% | [-14.0%, +9.6%] | +0.0% | ~ | triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects/float/host` | 77.3 ms | 85.4 ms | +10.5% | [-8.2%, +32.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.32 ms | 5.27 ms | -0.9% | [-1.8%, +0.1%] | - | ~ | tape/fused 1.49x (was 1.44x) |
| `Ray/csg-tape/firstHit/float/host` | 7.66 ms | 7.87 ms | +2.8% | [+1.1%, +4.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 65.1 ms | 60.4 ms | -7.3% | [-13.4%, -0.4%] | +0.0% | faster | hand/api 1.07x (was 0.99x) |
| `Ray/mesh/firstHit-handrolled/double/host` | 64.3 ms | 64.5 ms | +0.2% | [-15.4%, +18.8%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 48.3 ms | 49.2 ms | +1.9% | [-2.7%, +4.7%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 122 ms | 119 ms | -2.7% | [-14.4%, +10.3%] | +0.0% | ~ | triangles/polygons 0.51x (was 0.53x) |
| `Ray/mesh-polygons/intersects/double/host` | 96.4 ms | 99.6 ms | +3.3% | [-15.6%, +26.9%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.65 ms | 5.61 ms | -0.7% | [-1.5%, +0.1%] | - | ~ | tape/fused 1.43x |
| `Ray/csg-tape/firstHit/double/host` | 8.05 ms | 8.02 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.9 ms | 17.9 ms | -0.4% | [-2.6%, +1.8%] | - | ~ | marched/traced 6.89x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 124 ms | 123 ms | -1.0% | [-2.1%, +0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.7 ms | 22.6 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ | marched/traced 6.60x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 150 ms | 149 ms | -0.5% | [-1.8%, +0.9%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 36.5 ms | 36.3 ms | -0.5% | [-3.2%, +2.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.5 ms | 19.3 ms | -0.7% | [-3.0%, +1.6%] | - | ~ | marched/traced 6.73x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 130 ms | 130 ms | +0.3% | [-0.8%, +1.4%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 24.0 ms | 24.2 ms | +0.7% | [-0.2%, +1.7%] | +0.0% | ~ | marched/traced 6.40x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 155 ms | 155 ms | -0.0% | [-1.6%, +1.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.8 ms | 38.7 ms | -0.1% | [-2.1%, +2.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 50.2 ms | 52.1 ms | +3.9% | [-4.7%, +13.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 50.4 ms | 50.3 ms | -0.2% | [-9.2%, +9.7%] | +0.0% | ~ | hit/crossing 1.04x (was 1.00x) |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.79 ms | 7.78 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.19 ms | 6.28 ms | +1.4% | [-0.6%, +3.5%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 196 ms | 193 ms | -1.8% | [-9.2%, +6.0%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 174 ms | 170 ms | -2.4% | [-8.7%, +4.2%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 75.4 ms | 76.2 ms | +1.1% | [-5.6%, +8.5%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 61.4 ms | 62.1 ms | +1.2% | [-6.1%, +9.3%] | +0.0% | ~ | hit/crossing 1.23x |
| `RayCrossing/mesh/firstHit/double/host` | 66.5 ms | 59.5 ms | -10.6% | [-20.4%, +1.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 59.8 ms | 59.6 ms | -0.2% | [-5.4%, +5.4%] | +0.0% | ~ | hit/crossing 1.00x (was 1.11x) |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.04 ms | 8.03 ms | -0.1% | [-0.7%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 7.93 ms | 7.65 ms | -3.6% | [-32.7%, +39.5%] | +0.0% | ~ | hit/crossing 1.05x (was 1.01x) |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 226 ms | 235 ms | +4.0% | [-0.1%, +8.4%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 203 ms | 200 ms | -1.1% | [-7.1%, +5.5%] | +0.0% | ~ | hit/crossing 1.17x (was 1.11x) |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 90.6 ms | 89.0 ms | -1.8% | [-7.0%, +3.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 75.9 ms | 73.4 ms | -3.2% | [-9.5%, +3.3%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.6 ms | 11.6 ms | -0.0% | [-3.7%, +3.7%] | +0.0% | ~ | marched/traced 5.00x (was 4.84x) |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 56.2 ms | 58.1 ms | +3.3% | [-2.9%, +10.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.46 ms | 8.45 ms | -0.1% | [-4.7%, +4.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.96 ms | 2.21 ms | +12.5% | [-0.5%, +25.8%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 8.27 ms | 8.40 ms | +1.5% | [-29.8%, +46.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.8 ms | 12.6 ms | -1.3% | [-2.3%, -0.4%] | +0.0% | ~ | marched/traced 5.52x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 71.3 ms | 69.7 ms | -2.2% | [-12.6%, +8.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 9.25 ms | 9.30 ms | +0.5% | [-4.7%, +6.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.05 ms | 2.09 ms | +2.0% | [+1.0%, +3.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.36 ms | 7.35 ms | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 253 ms | 251 ms | -0.7% | [-1.7%, +0.2%] | +0.0% | ~ | marched/traced 1.44x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 358 ms | 360 ms | +0.5% | [-3.9%, +5.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 254 ms | 249 ms | -1.8% | [-6.1%, +2.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 270 ms | 268 ms | -0.6% | [-3.4%, +2.1%] | +0.0% | ~ | marched/traced 1.58x (was 1.53x) |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 413 ms | 425 ms | +2.8% | [-2.8%, +8.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 268 ms | 269 ms | +0.4% | [-1.0%, +1.8%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.47 ms | 7.47 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.8 ms | 12.8 ms | +0.5% | [-0.6%, +1.6%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 275 ms | 281 ms | +2.4% | [-2.6%, +7.4%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.7 ms | 12.7 ms | +0.3% | [-3.8%, +4.7%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 38.2 ms | 37.6 ms | -1.7% | [-32.2%, +43.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 27.7 ms | 27.6 ms | -0.4% | [-29.3%, +39.6%] | +0.0% | ~ | regions/hit 1.36x |
| `Regions/particles/region/double/host` | 10.9 ms | 11.0 ms | +0.4% | [-44.6%, +81.3%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 18.6 ms | 18.8 ms | +1.3% | [-46.1%, +89.4%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 293 ms | 296 ms | +1.1% | [+0.1%, +2.1%] | +0.0% | ~ | each/one-pass 0.06x |
| `Regions/particles/domainLabels/double/host` | 13.2 ms | 13.3 ms | +0.7% | [-1.8%, +3.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 33.2 ms | 33.3 ms | +0.2% | [-1.6%, +2.1%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 24.3 ms | 24.3 ms | +0.0% | [-1.9%, +2.0%] | +0.0% | ~ | regions/hit 1.37x |
| `Render/csg/frame/float/host` | 12.7 ms | 12.9 ms | +1.0% | [-5.8%, +8.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 12.7 ms | 12.4 ms | -2.2% | [-6.9%, +2.9%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.2 ms | 20.5 ms | +1.4% | [-0.2%, +3.0%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.4 ms | 14.6 ms | +1.3% | [+0.7%, +2.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.2 ms | 14.2 ms | -0.3% | [-1.6%, +1.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 22.0 ms | 21.4 ms | -2.8% | [-5.8%, +0.1%] | +0.0% | ~ |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 34.1 ms | 34.2 ms | +0.2% | [-0.5%, +1.0%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 5.11 ms | 5.16 ms | +1.1% | [-0.5%, +2.6%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.24 ms | 4.29 ms | +1.1% | [-0.4%, +2.6%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.6 ms | 17.6 ms | +0.0% | [-1.9%, +2.0%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.43 ms | 4.40 ms | -0.8% | [-1.1%, -0.5%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.17 ms | 4.22 ms | +1.1% | [-0.4%, +2.6%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.27 ms | 2.29 ms | +1.1% | [-0.7%, +2.9%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.47 ms | 1.46 ms | -0.6% | [-1.7%, +0.5%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.07 ms | 2.08 ms | +0.4% | [-1.2%, +2.1%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.6 ms | 27.9 ms | +0.8% | [-0.3%, +1.9%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 48.0 ms | 48.0 ms | -0.0% | [-0.3%, +0.3%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.59 ms | 5.48 ms | -1.9% | [-3.8%, +0.2%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.56 ms | 4.61 ms | +1.1% | [+0.5%, +1.7%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 23.1 ms | 23.2 ms | +0.7% | [-0.2%, +1.7%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.61 ms | 4.57 ms | -0.9% | [-1.6%, -0.3%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.37 ms | 5.29 ms | -1.5% | [-2.8%, -0.2%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.56 ms | 2.58 ms | +0.7% | [-0.1%, +1.6%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.82 ms | 1.81 ms | -0.5% | [-1.5%, +0.5%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.14 ms | 2.12 ms | -0.8% | [-3.9%, +2.4%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 31.3 ms | 31.4 ms | +0.3% | [-0.1%, +0.7%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 59.5 ms | 59.1 ms | -0.5% | [-1.6%, +0.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 38.7 ms | 39.8 ms | +2.8% | [-0.2%, +5.8%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 101 ms | 102 ms | +0.6% | [-1.6%, +2.8%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 68.2 ms | 67.1 ms | -1.7% | [-2.2%, -1.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 45.7 ms | 44.5 ms | -2.7% | [-4.3%, -1.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 121 ms | 122 ms | +0.1% | [-4.5%, +5.0%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 81.5 ms | 79.6 ms | -2.4% | [-6.7%, +2.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 322 ms | 332 ms | +3.2% | [-0.8%, +7.3%] | +0.0% | ~ | tape/fused 0.98x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 323 ms | 324 ms | +0.5% | [-0.4%, +1.3%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 97.9 ms | 97.7 ms | -0.2% | [-3.7%, +3.7%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 397 ms | 402 ms | +1.3% | [-3.1%, +5.9%] | +0.0% | ~ | tape/fused 0.97x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 394 ms | 392 ms | -0.5% | [-1.9%, +0.9%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 177 ms | 178 ms | +0.4% | [-1.7%, +2.7%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 211 ms | 209 ms | -0.7% | [-3.1%, +1.8%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 146 ms | 148 ms | +1.0% | [-0.1%, +2.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 91.5 ms | 91.5 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | surface/leaf 1.62x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 137 ms | 136 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 77.1 ms | 76.6 ms | -0.6% | [-3.3%, +2.2%] | +0.0% | ~ | surface/leaf 1.77x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 66.1 ms | 64.7 ms | -2.2% | [-5.0%, +0.8%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 41.4 ms | 35.9 ms | -13.1% | [-23.0%, -0.5%] | +0.0% | faster | surface/hit 1.80x (was 1.60x) |
| `SurfaceParts/boxes/closestSurface/double/host` | 149 ms | 148 ms | -1.0% | [-2.1%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 92.5 ms | 93.4 ms | +0.9% | [-0.4%, +2.3%] | +0.0% | ~ | surface/leaf 1.58x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 157 ms | 160 ms | +1.6% | [+0.0%, +3.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 88.7 ms | 89.6 ms | +1.0% | [-1.0%, +3.2%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 73.8 ms | 73.7 ms | -0.1% | [-2.9%, +2.7%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 42.1 ms | 40.9 ms | -3.1% | [-5.7%, -0.4%] | +0.0% | faster | surface/hit 1.80x (was 1.75x) |
| `Tape/csg/signedDistance/float/host` | 13.0 ms | 13.1 ms | +0.2% | [-1.2%, +1.7%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.67 ms | 4.63 ms | -1.0% | [-2.6%, +0.6%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.8 ms | 25.8 ms | -0.2% | [-1.5%, +1.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 41.2 ms | 41.4 ms | +0.5% | [-1.1%, +2.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.78 ms | 1.80 ms | +0.7% | [+0.5%, +0.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 135 µs | 143 µs | +6.0% | [-1.3%, +13.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.6 ms | 30.8 ms | +0.8% | [-0.7%, +2.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.77 ms | 5.74 ms | -0.6% | [-2.3%, +1.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.5 ms | 16.7 ms | +1.5% | [+0.5%, +2.4%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.76 ms | 4.72 ms | -0.6% | [-1.0%, -0.3%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.4 ms | 13.4 ms | +0.2% | [-0.5%, +1.0%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.59 ms | 4.61 ms | +0.4% | [-1.0%, +1.8%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 27.0 ms | 26.8 ms | -0.8% | [-1.3%, -0.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.9 ms | 42.9 ms | -0.0% | [-1.7%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.78 ms | 1.76 ms | -0.9% | [-1.7%, -0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 278 µs | 271 µs | -2.7% | [-10.3%, +5.9%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.8 ms | 30.8 ms | +0.0% | [-1.1%, +1.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.77 ms | 7.76 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.7 ms | 16.7 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.90 ms | 7.79 ms | -1.4% | [-1.6%, -1.2%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 42.8 ms | 42.9 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 204 ms | 204 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 44.0 ms | 43.6 ms | -0.9% | [-2.2%, +0.4%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 221 ms | 224 ms | +1.3% | [+0.4%, +2.3%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 866.9 s → 753.2 s (-13.1%), the largest peak 1410 MB → 1411 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 10.4 s | 10.6 s | +2.3% | 558 MB | 558 MB | +0.1% | ~ |
| `BVHRefit` | 4.7 s | 4.7 s | -0.6% | 497 MB | 497 MB | -0.0% | ~ |
| `BVHUnion` | 27.6 s | 21.2 s | -23.2% | 948 MB | 948 MB | +0.0% | ~ |
| `BVHUnionMixed` | 8.6 s | 8.5 s | -1.7% | 555 MB | 555 MB | -0.0% | ~ |
| `ClosestPrimitive` | 34.0 s | 33.2 s | -2.3% | 1296 MB | 1298 MB | +0.1% | ~ |
| `ClosestSurface` | 32.3 s | 30.9 s | -4.3% | 1228 MB | 1296 MB | +5.6% | ~ |
| `Domain` | 19.7 s | 19.9 s | +0.8% | 842 MB | 842 MB | -0.0% | ~ |
| `DomainObject` | 17.4 s | 17.6 s | +1.4% | 846 MB | 846 MB | +0.1% | ~ |
| `Fused` | 29.6 s | 27.0 s | -8.8% | 1135 MB | 1205 MB | +6.2% | ~ |
| `LaunchTuning` | 44.7 s | 39.8 s | -10.9% | 1310 MB | 1280 MB | -2.3% | ~ |
| `Mesh` | 18.3 s | 17.2 s | -6.1% | 783 MB | 784 MB | +0.0% | ~ |
| `MeshCSG` | 52.2 s | 41.9 s | -19.7% | 1255 MB | 1261 MB | +0.5% | ~ |
| `MeshClosestPoint` | 17.5 s | 15.5 s | -11.3% | 780 MB | 779 MB | -0.1% | ~ |
| `MeshReconcile` | 6.9 s | 7.3 s | +5.3% | 566 MB | 566 MB | +0.0% | ~ |
| `MeshTape` | 52.4 s | 43.5 s | -17.0% | 1288 MB | 1278 MB | -0.7% | ~ |
| `PointCloud` | 9.6 s | 8.0 s | -17.0% | 558 MB | 557 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 5.3 s | 4.1 s | -22.3% | 471 MB | 472 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 4.5 s | 3.5 s | -21.4% | 452 MB | 452 MB | +0.1% | ~ |
| `QueryOrder` | - | 12.0 s | | - | 686 MB | | new |
| `Ray` | 48.6 s | 37.9 s | -21.9% | 1317 MB | 1291 MB | -1.9% | ~ |
| `RayCSG` | 49.1 s | 33.5 s | -31.8% | 1188 MB | 1194 MB | +0.5% | ~ |
| `RayCrossing` | 59.3 s | 47.2 s | -20.4% | 1331 MB | 1349 MB | +1.4% | ~ |
| `RayParticles` | 62.0 s | 50.5 s | -18.5% | 1319 MB | 1332 MB | +1.0% | ~ |
| `RayShell` | 45.7 s | 38.8 s | -15.2% | 1285 MB | 1267 MB | -1.4% | ~ |
| `Regions` | 38.1 s | 30.2 s | -20.7% | 1073 MB | 1073 MB | -0.0% | ~ |
| `Render` | 39.6 s | 36.9 s | -6.8% | 1285 MB | 1273 MB | -0.9% | ~ |
| `Shapes` | 12.1 s | 9.0 s | -25.6% | 554 MB | 554 MB | +0.1% | ~ |
| `SphereCloudBVH` | 5.7 s | 5.0 s | -13.4% | 511 MB | 512 MB | +0.1% | ~ |
| `SphereCloudRays` | 29.9 s | 29.1 s | -2.4% | 1063 MB | 1065 MB | +0.2% | ~ |
| `SphereContacts` | 5.8 s | 4.3 s | -24.8% | 484 MB | 486 MB | +0.3% | ~ |
| `SurfaceParts` | 42.8 s | 43.4 s | +1.6% | 1410 MB | 1411 MB | +0.1% | ~ |
| `Tape` | 17.3 s | 17.4 s | +0.8% | 830 MB | 829 MB | -0.1% | ~ |
| `TapeBVHUnion` | 15.2 s | 15.4 s | +1.0% | 818 MB | 826 MB | +0.9% | ~ |
| **all, in both** | 866.9 s | 753.2 s | -13.1% | 1410 MB | 1411 MB | +0.1% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (7 cases): MeshTape/tape/frame/float/host, MeshTape/view/firstHit/float/host, PointCloudBVH/sphere/closest-outside-random/double/host, Ray/mesh/intersects/double/host, RayCrossing/csg-tape/firstHit/double/host, Render/csg/frame-orthographic/double/host, SphereCloudRays/polydisperse/firstHit/double/host

These probes compiled to the same code on both sides and were not run (2 cases): PointCloudHashGridBuild (2)

The probes differ between the sides (each side ran its own): Performance/QueryOrder/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: fd5c8a5
base-fingerprint: 263791791f122733f874f9739b94c83cd96178ea
head: 5813a7e
head-fingerprint: d09e1f73e272cc4ad11609c8448c384fcd8f001e
cases: 402
rounds: 2
reps: 3
date: 2026-11-19
-->

## Device comparison (CUDA): red

**Accepted:** The host/device pixel disagreement (1 pixel in the float mesh frames, identical on the base fd5c8a5) is not ours; a known issue, worked on separately.

`5813a7e` against `fd5c8a5` (origin/main), 2026-11-19.

**VERDICT: host and device answers differ in 4 cases; investigate before merging (see Flagged above)**

Performance comparison: 0 major, 0 minor, 11 faster, 4 differ, 0 lost, 402 cases

- base: fd5c8a5 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 5813a7e (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.62 ms | 1.61 ms | -0.6% | [-1.4%, +0.1%] | - | ~ | device 22.7x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.44 ms | 3.41 ms | -0.7% | [-1.2%, -0.2%] | - | ~ | device 18.5x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.06 ms | 1.05 ms | -1.0% | [-4.5%, +2.6%] | - | ~ | device 19.6x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.30 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.35 ms | 6.21 ms | -2.2% | [-3.7%, -0.7%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 12.0 ms | 11.9 ms | -0.8% | [-2.6%, +1.0%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 31.1 ms | 30.0 ms | -3.7% | [-17.8%, +12.4%] | - | ~ | device 1.2x faster than the host (was 1.2x) |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 46.8 ms | 47.0 ms | +0.5% | [-0.6%, +1.6%] | - | ~ | device 1.4x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 16.7 ms | 17.1 ms | +2.5% | [+0.1%, +4.9%] | - | ~ | device 1.2x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 41.5 ms | 43.4 ms | +4.7% | [-9.5%, +20.3%] | - | ~ | device 0.9x faster than the host (was 0.9x) |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 50.6 ms | 50.5 ms | -0.3% | [-1.5%, +0.9%] | - | ~ | device 0.9x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 90.8 ms | 91.2 ms | +0.4% | [-2.1%, +2.9%] | - | ~ | device 0.7x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.31 ms | 8.30 ms | -0.2% | [-1.7%, +1.4%] | +0.0% | ~ | device 4.9x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 10.0 ms | 9.89 ms | -1.2% | [-2.2%, -0.2%] | +0.0% | ~ | device 8.1x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 389 µs | 387 µs | -0.3% | [-1.1%, +0.6%] | - | ~ | device 15.1x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 849 µs | 860 µs | +1.3% | [-0.5%, +3.1%] | - | ~ | device 9.8x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.53 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.67 ms | 4.66 ms | -0.3% | [-0.8%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 585 µs | 585 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 12.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.39 ms | +0.5% | [-0.4%, +1.4%] | - | ~ | device 8.2x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.76 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.02 ms | 7.01 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 912 µs | 908 µs | -0.4% | [-0.6%, -0.2%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.24 ms | 2.23 ms | -0.1% | [-0.9%, +0.7%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.48 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 849 µs | 857 µs | +0.9% | [-0.4%, +2.2%] | - | ~ | device 13.4x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.09 ms | 2.07 ms | -1.1% | [-1.9%, -0.2%] | - | ~ | device 7.9x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.09 ms | 9.11 ms | +0.3% | [+0.0%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.82 ms | 9.84 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 313 µs | 312 µs | -0.3% | [-1.0%, +0.4%] | - | ~ | device 11.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 6.4x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 55.1 ms | 56.3 ms | +2.2% | [-3.1%, +7.5%] | - | ~ | device 3.5x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.07 ms | 1.07 ms | +0.0% | [-2.1%, +2.2%] | - | ~ | device 3.5x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 4.42 ms | 4.42 ms | +0.0% | [-2.0%, +2.1%] | - | ~ | device 2.0x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 519 ms | 518 ms | -0.2% | [-2.0%, +1.6%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 18.3 ms | 18.6 ms | +1.3% | [-2.4%, +5.2%] | - | ~ | device 4.4x faster than the host |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 7.77 ms | 7.76 ms | -0.0% | [-13.5%, +15.5%] | - | ~ | device 5.1x faster than the host; surface/leaf 2.39x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.2 ms | -2.6% | [-5.3%, +0.1%] | - | ~ | device 8.2x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.19 ms | 6.20 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 11.4x faster than the host (was 10.3x); surface/leaf 2.45x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 126 ms | 126 ms | -0.0% | [-1.6%, +1.6%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 49.4 ms | 49.4 ms | +0.0% | [-1.0%, +1.0%] | - | ~ | device 1.0x faster than the host (was 1.1x); surface/leaf 2.56x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 59.5 ms | 59.6 ms | +0.1% | [-1.9%, +2.2%] | - | ~ | device 2.8x faster than the host (was 2.1x) |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 23.2 ms | 23.3 ms | +0.1% | [-2.1%, +2.3%] | - | ~ | device 2.8x faster than the host; surface/leaf 2.56x |
| `Domain/particles/labels/float/hostdevice` | 3.50 ms | 3.58 ms | +2.5% | [-1.5%, +6.4%] | - | ~ | device 3.9x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.24 ms | 2.27 ms | +1.0% | [-0.4%, +2.3%] | - | ~ | device 3.2x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 113 µs | 110 µs | -2.8% | [-7.5%, +2.3%] | - | ~ | device 13.6x faster than the host (was 13.1x) |
| `Domain/particles/labels/double/hostdevice` | 7.28 ms | 7.18 ms | -1.4% | [-4.0%, +1.3%] | - | ~ | device 2.0x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.45 ms | 3.46 ms | +0.3% | [-0.8%, +1.4%] | - | ~ | device 2.1x faster than the host |
| `Domain/particles/surfaceGroups/double/hostdevice` | 128 µs | 129 µs | +0.6% | [-2.4%, +3.7%] | - | ~ | device 11.1x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 12.5 ms | 12.5 ms | -0.2% | [-1.1%, +0.7%] | - | ~ | device 3.6x faster than the host |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.77 ms | 8.79 ms | +0.2% | [-0.8%, +1.1%] | - | ~ | device 2.7x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 278 µs | 270 µs | -2.8% | [-5.4%, -0.2%] | - | ~ | device 2.5x faster than the host; own/whole 46.44x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 176 µs | 146 µs | -16.7% | [-26.5%, -3.7%] | - | faster | device 2.7x faster than the host (was 2.1x); own/whole 60.07x (was 49.98x) |
| `DomainObject/partsWhole/labels/double/hostdevice` | 20.1 ms | 22.3 ms | +10.9% | [-6.0%, +29.7%] | - | ~ | device 2.2x faster than the host (was 2.4x) |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 12.2 ms | 12.2 ms | -0.3% | [-21.4%, +26.3%] | - | ~ | device 2.1x faster than the host (was 2.4x) |
| `DomainObject/partsOwn/labels/double/hostdevice` | 395 µs | 396 µs | +0.2% | [-1.4%, +3.9%] | - | ~ | device 1.7x faster than the host (was 1.8x); own/whole 56.23x (was 50.77x) |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 211 µs | 207 µs | -2.0% | [-4.3%, +0.3%] | - | ~ | device 1.7x faster than the host (was 1.6x); own/whole 58.88x |
| `Fused/csg/signedDistance/float/hostdevice` | 48.2 µs | 47.6 µs | -1.2% | [-3.1%, +0.7%] | - | ~ | device 24.0x faster than the host; tape/fused 4.22x |
| `Fused/smooth/signedDistance/float/hostdevice` | 52.3 µs | 51.6 µs | -1.2% | [-15.7%, +15.8%] | - | ~ | device 42.6x faster than the host (was 41.2x); tape/fused 6.39x |
| `Fused/chain16/signedDistance/float/hostdevice` | 51.1 µs | 47.9 µs | -6.3% | [-12.8%, +1.2%] | - | ~ | device 8.9x faster than the host; tape/fused 9.94x (was 9.32x) |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 19.8 ms | 19.6 ms | -0.7% | [-17.0%, +19.6%] | - | ~ | device 6.0x faster than the host; tape/fused 1.62x (was 1.68x) |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 33.2 ms | 31.7 ms | -4.3% | [-16.9%, +12.2%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 667 µs | 667 µs | +0.0% | [-0.9%, +1.0%] | - | ~ | device 1.8x faster than the host; tape/fused 1.29x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.28 ms | 1.28 ms | -0.0% | [-0.9%, +0.9%] | - | ~ | device 1.9x faster than the host; tape/fused 1.36x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +9.5%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 261 ms | 263 ms | +0.7% | [+0.1%, +1.3%] | - | ~ | device 0.5x faster than the host; tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 263 ms | 263 ms | -0.1% | [-1.3%, +1.1%] | - | ~ | device 0.5x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 7.79 ms | 7.85 ms | +0.8% | [-11.6%, +14.8%] | +0.0% | ~ | device 11.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.38 ms | 8.25 ms | -1.5% | [-2.6%, -0.4%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.95x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.29 ms | 8.29 ms | +0.1% | [-1.8%, +1.9%] | +0.0% | ~ | device 12.9x faster than the host (was 12.4x); default/this 0.95x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.23 ms | 8.18 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ | device 15.5x faster than the host (was 10.8x); default/this 0.96x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 700 µs | 755 µs | +7.9% | [-1.5%, +17.7%] | +0.0% | ~ | device 3.4x faster than the host (was 3.7x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.80 ms | 1.79 ms | -0.2% | [-1.7%, +1.3%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.42x (was 0.39x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.06 ms | 1.04 ms | -1.8% | [-2.4%, -1.2%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.73x (was 0.66x) |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 802 µs | 784 µs | -2.2% | [-5.7%, +1.5%] | +0.0% | ~ | device 3.3x faster than the host (was 3.2x); default/this 0.96x (was 0.87x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 767 µs | 744 µs | -3.1% | [-5.1%, -1.0%] | +0.0% | faster | device 12.2x faster than the host (was 11.6x) |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 834 µs | 831 µs | -0.3% | [-1.1%, +0.4%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 739 µs | 756 µs | +2.3% | [+0.7%, +3.9%] | +0.0% | ~ | device 11.7x faster than the host; default/this 0.98x (was 1.04x) |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 733 µs | 726 µs | -0.9% | [-2.4%, +0.6%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 137 ms | 138 ms | +1.2% | [-0.2%, +2.6%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 141 ms | 136 ms | -3.6% | [-6.9%, -0.2%] | +0.0% | faster | device 0.7x faster than the host; default/this 1.02x (was 0.97x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 138 ms | 142 ms | +2.5% | [-0.2%, +5.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 141 ms | 139 ms | -1.9% | [-5.4%, +1.9%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x (was 0.97x) |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.22 ms | 1.22 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 2.2x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.81 ms | 2.83 ms | +0.6% | [-1.7%, +2.9%] | +0.0% | ~ | device 0.9x faster than the host; default/this 0.43x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.96 ms | 1.92 ms | -2.5% | [-5.8%, +1.0%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.64x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.47 ms | 1.49 ms | +1.5% | [-0.9%, +3.9%] | +0.0% | ~ | device 1.8x faster than the host; default/this 0.82x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.25 ms | 7.24 ms | -0.2% | [-0.5%, +0.3%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.30 ms | 7.28 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.25 ms | 7.25 ms | +0.0% | [-0.7%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.31 ms | 7.31 ms | +0.1% | [-0.7%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.45 ms | 8.41 ms | -0.5% | [-0.8%, -0.1%] | - | ~ | device 11.4x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 18.9 ms | 17.0 ms | -9.8% | [-18.7%, +1.3%] | - | ~ | device 7.0x faster than the host (was 8.5x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 147 ms | 144 ms | -1.8% | [-4.6%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 263 ms | 271 ms | +3.0% | [-0.6%, +6.5%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `MeshSDF/query-morton/float/hostdevice` | 24.5 ms | 25.8 ms | +5.6% | [-0.9%, +12.0%] | - | ~ | device 3.9x faster than the host (was 4.0x) |
| `MeshSDF/query-random/float/hostdevice` | 63.0 ms | 62.4 ms | -0.8% | [-9.4%, +8.6%] | - | ~ | device 2.4x faster than the host (was 2.3x) |
| `MeshSDF/query-morton/double/hostdevice` | 137 ms | 141 ms | +2.7% | [-3.4%, +8.9%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 244 ms | 230 ms | -5.8% | [-14.4%, +4.2%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 1.72 ms | 1.67 ms | -3.3% | [-5.3%, -1.3%] | - | faster | device 6.1x faster than the host (was 5.8x) |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.35 ms | 3.32 ms | -0.8% | [-1.3%, -0.3%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.44 ms | 6.44 ms | -0.0% | [-11.1%, +12.4%] | - | ~ | device 4.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 55.3 ms | 57.5 ms | +4.0% | [-0.9%, +9.0%] | - | ~ | device 4.3x faster than the host (was 4.1x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 84.2 ms | 84.1 ms | -0.2% | [-1.1%, +0.6%] | - | ~ | device 2.7x faster than the host (was 2.8x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 227 ms | 226 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 22.7 ms | 22.6 ms | -0.2% | [-18.0%, +21.4%] | - | ~ | device 2.2x faster than the host; tape/fused 2.54x (was 2.44x) |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 23.2 ms | 23.0 ms | -1.1% | [-1.6%, -0.5%] | - | ~ | device 2.5x faster than the host; tape/fused 3.65x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 61.0 ms | 60.3 ms | -1.0% | [-14.2%, +14.2%] | - | ~ | device 4.3x faster than the host; tape/fused 3.75x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 16.9 ms | 17.1 ms | +0.9% | [-2.2%, +4.2%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 30.7 ms | 31.4 ms | +2.2% | [+0.7%, +3.7%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 32.6 ms | 33.3 ms | +2.1% | [+1.3%, +2.9%] | - | ~ | device 0.9x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 520 ms | 528 ms | +1.5% | [-1.2%, +4.2%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 844 ms | 858 ms | +1.7% | [-0.1%, +3.5%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.03 s | 2.00 s | -1.5% | [-4.9%, +2.2%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 177 ms | 190 ms | +7.3% | [-9.8%, +26.2%] | - | ~ | device 0.3x faster than the host (was 0.3x); tape/fused 2.77x (was 2.93x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 298 ms | 302 ms | +1.4% | [-0.2%, +5.5%] | - | ~ | device 0.2x faster than the host; tape/fused 2.84x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 570 ms | 547 ms | -4.0% | [-8.0%, +0.1%] | - | ~ | device 0.5x faster than the host (was 0.6x); tape/fused 3.65x |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 12.1 ms | 11.8 ms | -2.8% | [-7.5%, +2.0%] | - | ~ | device 10.0x faster than the host (was 9.8x) |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 60.5 ms | 60.4 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 2.6x faster than the host (was 2.4x) |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 242 ms | 241 ms | -0.7% | [-0.8%, -0.5%] | - | ~ | device 0.5x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 229 ms | 230 ms | +0.4% | [+0.0%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.63 ms | 2.64 ms | +0.4% | [-0.7%, +1.6%] | +0.0% | ~ | device 16.1x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.37 ms | 5.36 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ | device 10.6x faster than the host |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.29 ms | 1.59 ms | +24.1% | [-1.0%, +49.4%] | - | ~ | device 12.7x faster than the host (was 15.0x) |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.28 ms | 1.28 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 15.0x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.07 ms | 2.08 ms | +0.3% | [-0.1%, +0.7%] | - | **DIFFER** | device 7.8x faster than the host; 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.33 ms | 3.33 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 10.1x faster than the host; tape/mesh 0.48x (was 0.39x) |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.67 ms | 2.67 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 7.5x faster than the host; tape/mesh 0.48x |
| `MeshTape/tape/frame/float/hostdevice` | 4.34 ms | 5.41 ms | +24.6% | [-1.7%, +51.2%] | - | **DIFFER** | device 4.1x faster than the host (was 4.9x); tape/mesh 0.38x (was 0.48x); 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/view/firstHit/float/hostdevice` | 3.55 ms | 3.56 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 9.6x faster than the host; view/mesh 0.45x (was 0.36x) |
| `MeshTape/view/frame/float/hostdevice` | 5.51 ms | 5.47 ms | -0.7% | [-1.1%, -0.3%] | - | **DIFFER** | device 4.2x faster than the host; view/mesh 0.38x; 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/mesh/firstHit/double/hostdevice` | 26.7 ms | 26.3 ms | -1.5% | [-18.0%, +21.1%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 22.6 ms | 22.6 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 0.9x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 19.8 ms | 19.4 ms | -2.2% | [-2.8%, -1.7%] | - | ~ | device 0.9x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 32.4 ms | 32.3 ms | -0.4% | [-0.5%, -0.4%] | - | ~ | device 1.1x faster than the host; tape/mesh 0.82x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 22.7 ms | 22.7 ms | -0.3% | [-1.2%, +0.5%] | - | ~ | device 0.9x faster than the host; tape/mesh 1.00x |
| `MeshTape/tape/frame/double/hostdevice` | 25.0 ms | 25.1 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 0.9x faster than the host; tape/mesh 0.77x |
| `MeshTape/view/firstHit/double/hostdevice` | 34.4 ms | 32.3 ms | -6.0% | [-11.5%, +0.1%] | - | ~ | device 1.1x faster than the host (was 1.0x); view/mesh 0.81x (was 0.78x) |
| `MeshTape/view/frame/double/hostdevice` | 27.3 ms | 27.5 ms | +0.7% | [-0.3%, +1.7%] | - | ~ | device 0.9x faster than the host; view/mesh 0.70x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 605 µs | 606 µs | +0.2% | [-0.0%, +0.3%] | - | ~ | device 12.4x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.81 ms | 1.81 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 5.8x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 873 µs | 1.51 ms | +72.8% | [-0.1%, +145.7%] | - | ~ | device 7.7x faster than the host (was 10.6x) |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.48 ms | 4.39 ms | -1.9% | [-4.8%, +1.1%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.90 ms | 2.91 ms | +0.3% | [-4.0%, +4.7%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.02 ms | 6.50 ms | -7.4% | [-16.6%, +2.0%] | - | ~ | device 1.2x faster than the host (was 1.1x) |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.84 ms | 7.80 ms | -0.6% | [-1.4%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 21.2 ms | 13.0 ms | -38.6% | [-56.3%, -0.2%] | - | faster | device 0.8x faster than the host (was 0.6x) |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | +0.0% | [-0.1%, +7.5%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 9.92 ms | 10.1 ms | +2.0% | [+0.8%, +3.2%] | - | ~ | device 1.6x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.20 ms | 1.22 ms | +1.1% | [-0.9%, +3.1%] | - | ~ | device 7.7x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.46 ms | 3.46 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 3.7x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 481 µs | 480 µs | -0.2% | [-15.7%, +18.0%] | - | ~ | device 17.4x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.50 ms | 4.59 ms | +2.1% | [+0.7%, +3.4%] | - | ~ | device 6.6x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.29 ms | 4.38 ms | +1.9% | [-0.7%, +4.6%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.13 ms | 8.15 ms | +0.2% | [-0.6%, +1.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.7 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 0.9x faster than the host (was 0.8x) |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.00 ms | -0.3% | [-0.4%, -0.3%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.5 ms | 17.7 ms | +1.1% | [-0.6%, +2.8%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 13.8 ms | 13.6 ms | -1.0% | [-1.7%, -0.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.50 ms | 1.48 ms | -1.4% | [-3.6%, +1.0%] | - | ~ | device 11.1x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.74 ms | 4.78 ms | +0.7% | [+0.4%, +0.9%] | - | ~ | device 5.0x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 835 µs | 832 µs | -0.4% | [-0.9%, +0.2%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.85 ms | 2.86 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 7.0x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.25 ms | 4.15 ms | -2.4% | [-5.3%, +0.6%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.5 ms | 16.6 ms | +0.8% | [-0.1%, +1.7%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.0 ms | 21.1 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.26 ms | 8.23 ms | -0.3% | [-15.8%, +18.0%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 13.5 ms | 13.5 ms | +0.0% | [-15.5%, +18.4%] | - | ~ | device 1.6x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 18.9 ms | 18.0 ms | -4.7% | [-16.3%, +9.2%] | - | ~ | device 1.3x faster than the host (was 1.3x) |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.28 ms | 2.26 ms | -0.8% | [-2.1%, +0.6%] | - | ~ | device 5.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.27 ms | 6.26 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 2.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.15 ms | 1.14 ms | -0.6% | [-1.3%, +0.0%] | - | ~ | device 8.0x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.43 ms | 2.43 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 8.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.41 ms | 6.41 ms | -0.0% | [-0.9%, +0.8%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 30.7 ms | 31.0 ms | +1.1% | [-15.7%, +21.1%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 56.6 ms | 56.6 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.4x faster than the host (was 0.3x) |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 15.3 ms | 15.2 ms | -0.4% | [-1.1%, +0.2%] | - | ~ | device 1.0x faster than the host (was 0.9x) |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 17.7 ms | 17.8 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 32.8 ms | 32.4 ms | -1.2% | [-2.4%, +0.1%] | - | ~ | device 0.9x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.7 ms | 12.3 ms | -3.6% | [-9.2%, +2.5%] | +0.0% | ~ | device 4.2x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.59 ms | 9.83 ms | +2.6% | [-0.9%, +6.0%] | +0.0% | ~ | device 4.9x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 13.4 ms | 13.0 ms | -2.8% | [-12.4%, +7.5%] | +0.0% | ~ | device 5.2x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.7 ms | 10.7 ms | +0.3% | [-4.3%, +5.1%] | +0.0% | ~ | device 5.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.56 ms | 3.60 ms | +1.2% | [-13.7%, +18.4%] | +0.0% | ~ | device 5.0x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.72 ms | 2.72 ms | -0.1% | [-9.6%, +10.4%] | +0.0% | ~ | device 5.3x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.94 ms | 4.98 ms | +0.9% | [-12.1%, +15.6%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.05 ms | 3.20 ms | +5.0% | [-0.4%, +10.5%] | +0.0% | ~ | device 5.3x faster than the host (was 3.7x) |
| `QueryOrder/random/mortonOrder/float/hostdevice` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-arrived/float/hostdevice` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-ordered/float/hostdevice` | - | - | - | - | - | new | arrived/ordered 1.25x |
| `QueryOrder/random/mortonOrder/double/hostdevice` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-arrived/double/hostdevice` | - | - | - | - | - | new |  |
| `QueryOrder/armadillo/distance-ordered/double/hostdevice` | - | - | - | - | - | new | arrived/ordered 1.39x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 671 µs | 678 µs | +1.0% | [-0.1%, +9.3%] | - | ~ | device 16.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.20 ms | -6.7% | [-12.9%, -0.4%] | - | faster | device 22.1x faster than the host; hand/api 1.03x (was 0.95x) |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 644 µs | 656 µs | +1.8% | [+1.6%, +2.0%] | - | ~ | device 26.2x faster than the host (was 27.8x) |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.23 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 23.4x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 615 µs | 615 µs | +0.1% | [-1.1%, +1.3%] | - | ~ | device 24.4x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.13 ms | 1.12 ms | -0.3% | [-1.3%, +0.7%] | - | ~ | device 23.5x faster than the host (was 24.4x) |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.18 ms | -0.7% | [-1.4%, +0.0%] | - | ~ | device 17.3x faster than the host (was 18.9x); triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.49 ms | 2.49 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 14.6x faster than the host (was 13.6x); triangles/polygons 0.48x (was 0.52x) |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.08 ms | 1.08 ms | -0.5% | [-1.1%, -0.0%] | - | ~ | device 15.7x faster than the host (was 16.9x) |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.19 ms | 2.18 ms | -0.5% | [-1.0%, +0.0%] | - | ~ | device 12.9x faster than the host (was 12.1x) |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 598 µs | 593 µs | -0.7% | [-1.4%, +0.0%] | - | ~ | device 3.1x faster than the host; tape/fused 1.15x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 769 µs | 734 µs | -4.5% | [-15.8%, +10.4%] | - | ~ | device 3.0x faster than the host; tape/fused 1.17x (was 1.11x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 685 µs | 684 µs | -0.2% | [-0.5%, +0.2%] | - | ~ | device 4.4x faster than the host (was 6.1x) |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 855 µs | 860 µs | +0.6% | [-0.1%, +1.3%] | - | ~ | device 4.2x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.0% | [-1.1%, +1.1%] | - | ~ | device 0.8x faster than the host; hand/api 0.91x (was 0.85x) |
| `Ray/mesh/firstHit-random/double/hostdevice` | 25.9 ms | 25.8 ms | -0.4% | [-9.6%, +9.5%] | - | ~ | device 0.9x faster than the host (was 0.8x); hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 14.5 ms | 15.4 ms | +6.4% | [-6.9%, +20.7%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 24.9 ms | 24.9 ms | -0.0% | [-18.0%, +21.7%] | - | ~ | device 0.8x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 12.3 ms | 13.5 ms | +9.7% | [-2.2%, +21.8%] | - | ~ | device 0.7x faster than the host (was 0.8x) |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.6 ms | 22.2 ms | -9.5% | [-18.7%, -0.3%] | - | faster | device 0.8x faster than the host (was 0.7x) |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.4 ms | 16.5 ms | +0.4% | [-0.4%, +10.4%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 22.3 ms | 22.4 ms | +0.3% | [-18.1%, +22.7%] | - | ~ | device 1.2x faster than the host; triangles/polygons 1.15x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 13.6 ms | 13.7 ms | +1.0% | [-17.6%, +23.7%] | - | ~ | device 1.0x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 22.5 ms | 20.8 ms | -7.2% | [-18.5%, +4.7%] | - | ~ | device 1.1x faster than the host (was 1.0x) |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.02 ms | 1.02 ms | -0.1% | [-0.9%, +0.6%] | - | ~ | device 1.6x faster than the host; tape/fused 1.19x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 1.95 ms | 1.95 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 1.0x faster than the host; tape/fused 1.07x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.22 ms | 1.22 ms | +0.0% | [-0.7%, +0.8%] | - | ~ | device 2.3x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.07 ms | 2.08 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.5x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.43 ms | 2.43 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.7x faster than the host; marched/traced 3.38x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.85 ms | 2.88 ms | +0.8% | [+0.2%, +1.5%] | - | ~ | device 2.4x faster than the host; marched/traced 4.82x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 8.19 ms | 8.20 ms | +0.2% | [-17.7%, +22.0%] | - | ~ | device 5.6x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.9 ms | +0.2% | [+0.2%, +0.2%] | - | ~ | device 3.7x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.20 ms | 2.22 ms | +0.8% | [+0.3%, +1.3%] | - | ~ | device 3.8x faster than the host; marched/traced 12.76x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.63 ms | 5.65 ms | +0.5% | [-0.5%, +1.5%] | - | ~ | device 4.1x faster than the host (was 2.8x); marched/traced 9.42x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 28.2 ms | 28.3 ms | +0.3% | [-6.1%, +7.2%] | - | ~ | device 3.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 53.2 ms | 53.2 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.9x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 4.93 ms | 4.96 ms | +0.5% | [-0.5%, +1.4%] | - | ~ | device 4.5x faster than the host (was 4.7x) |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.46 ms | 8.69 ms | +2.6% | [-0.3%, +5.6%] | - | ~ | device 4.7x faster than the host (was 4.9x) |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 5.76 ms | 5.74 ms | -0.3% | [-15.8%, +18.0%] | - | ~ | device 2.0x faster than the host; marched/traced 24.33x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.2x faster than the host; marched/traced 22.63x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 140 ms | 140 ms | -0.5% | [-0.9%, -0.0%] | - | ~ | device 0.4x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 240 ms | 240 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.51 ms | 4.57 ms | +1.4% | [-1.9%, +4.8%] | - | ~ | device 2.9x faster than the host (was 3.1x); marched/traced 20.68x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 11.5 ms | 11.5 ms | +0.0% | [-0.6%, +0.7%] | - | ~ | device 2.1x faster than the host (was 2.2x); marched/traced 17.52x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 94.7 ms | 94.6 ms | -0.1% | [-1.3%, +1.1%] | - | ~ | device 1.0x faster than the host (was 1.1x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 202 ms | 202 ms | +0.1% | [-0.8%, +1.1%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 16.1 ms | 16.0 ms | -1.1% | [-1.3%, -0.9%] | - | ~ | device 1.5x faster than the host (was 1.4x) |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 23.7 ms | 22.6 ms | -4.4% | [-8.4%, +0.0%] | - | ~ | device 1.8x faster than the host (was 1.7x) |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 584 µs | 635 µs | +8.7% | [-3.2%, +20.8%] | - | ~ | device 27.2x faster than the host (was 30.2x) |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.20 ms | -6.7% | [-13.2%, -0.2%] | - | faster | device 22.8x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 687 µs | 679 µs | -1.2% | [-3.9%, +1.6%] | - | ~ | device 26.2x faster than the host; hit/crossing 0.93x (was 0.85x) |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.29 ms | +0.9% | [+0.0%, +1.7%] | - | ~ | device 22.7x faster than the host (was 23.5x); hit/crossing 0.93x (was 1.01x) |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 696 µs | 707 µs | +1.6% | [-1.8%, +5.0%] | - | ~ | device 5.0x faster than the host (was 5.6x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 863 µs | 853 µs | -1.2% | [-2.9%, +0.6%] | - | ~ | device 4.7x faster than the host (was 4.9x) |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 619 µs | 689 µs | +11.2% | [-0.1%, +22.6%] | - | ~ | device 3.6x faster than the host (was 3.9x); hit/crossing 1.03x (was 1.12x) |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 640 µs | 662 µs | +3.4% | [-0.1%, +6.9%] | - | ~ | device 4.7x faster than the host (was 4.4x); hit/crossing 1.29x (was 1.35x) |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.72 ms | 4.72 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 10.5x faster than the host (was 12.3x) |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.18 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 10.2x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.09 ms | 10.3 ms | +13.8% | [-0.0%, +27.6%] | - | ~ | device 6.0x faster than the host (was 7.0x); hit/crossing 0.99x (was 1.12x) |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 48.8 ms | 51.7 ms | +5.8% | [-0.6%, +12.3%] | - | ~ | device 4.9x faster than the host (was 3.7x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 73.3 ms | 73.1 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 2.6x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 42.1 ms | 41.2 ms | -2.3% | [-3.4%, -1.1%] | - | ~ | device 4.6x faster than the host (was 3.7x); hit/crossing 1.25x (was 1.16x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 57.3 ms | 57.6 ms | +0.4% | [-0.9%, +1.7%] | - | ~ | device 2.7x faster than the host (was 3.5x); hit/crossing 1.27x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 15.6 ms | 15.5 ms | -0.5% | [-18.5%, +22.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 25.0 ms | 25.0 ms | -0.0% | [-17.5%, +21.1%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 15.4 ms | 15.5 ms | +0.5% | [-17.1%, +21.8%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 26.8 ms | -2.3% | [-4.1%, -0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.93x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.23 ms | 1.23 ms | +0.0% | [-0.9%, +0.9%] | - | ~ | device 2.2x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.09 ms | 2.08 ms | -0.7% | [-1.8%, +0.4%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.02 ms | 1.03 ms | +0.9% | [-0.4%, +2.2%] | - | ~ | device 2.0x faster than the host; hit/crossing 1.19x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.13 ms | 1.08 ms | -4.2% | [-8.1%, +0.0%] | - | ~ | device 2.2x faster than the host; hit/crossing 1.93x (was 1.86x) |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 55.4 ms | 58.7 ms | +5.8% | [-8.3%, +21.1%] | - | ~ | device 1.2x faster than the host (was 0.9x) |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 102 ms | 103 ms | +0.5% | [+0.4%, +0.6%] | - | ~ | device 0.7x faster than the host (was 1.0x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 48.2 ms | 48.2 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 0.9x faster than the host; hit/crossing 1.22x (was 1.15x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 93.4 ms | 93.3 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 511 ms | 477 ms | -6.6% | [-10.3%, -2.7%] | - | faster | device 0.5x faster than the host (was 0.3x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 724 ms | 719 ms | -0.7% | [-2.4%, +1.1%] | - | ~ | device 0.3x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 392 ms | 387 ms | -1.2% | [-6.8%, +5.0%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.23x (was 1.30x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 525 ms | 538 ms | +2.6% | [+1.6%, +3.6%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.34x (was 1.38x) |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.63 ms | 3.63 ms | -0.1% | [-2.3%, +2.3%] | - | ~ | device 1.2x faster than the host; marched/traced 3.30x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.79 ms | 4.77 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 0.9x faster than the host; marched/traced 5.57x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | -0.4% | [-1.1%, +0.3%] | - | ~ | device 4.0x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.6 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.9x faster than the host (was 3.0x) |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.26 ms | 1.24 ms | -1.5% | [-1.8%, -1.3%] | - | ~ | device 3.6x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.86 ms | +0.4% | [-1.2%, +2.0%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 96.9 µs | 96.6 µs | -0.4% | [-3.8%, +3.2%] | - | ~ | device 22.2x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 150 µs | 152 µs | +1.3% | [-1.4%, +4.1%] | - | ~ | device 16.5x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 794 µs | 801 µs | +0.9% | [+0.4%, +1.5%] | - | ~ | device 4.6x faster than the host (was 4.3x) |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 933 µs | 934 µs | +0.1% | [-0.9%, +1.2%] | - | ~ | device 4.5x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.7 ms | 36.1 ms | +1.0% | [-1.9%, +6.2%] | - | ~ | device 0.1x faster than the host; marched/traced 2.85x (was 3.16x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.7 ms | 37.1 ms | +1.1% | [-1.1%, +3.3%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 5.91x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 113 ms | 103 ms | -9.0% | [-14.7%, -2.4%] | - | faster | device 0.6x faster than the host (was 0.5x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 219 ms | 220 ms | +0.2% | [-7.3%, +8.3%] | - | ~ | device 0.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.4 ms | 16.4 ms | -0.1% | [-7.6%, +8.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.7 ms | 22.0 ms | +1.5% | [-0.5%, +3.5%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.07 ms | 1.07 ms | -0.6% | [-5.3%, +1.3%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.36 ms | 2.36 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 1.1x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.99 ms | 5.95 ms | -0.7% | [-1.5%, +0.2%] | - | ~ | device 0.6x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.73 ms | 6.71 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 0.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.1 ms | 23.6 ms | +2.2% | [+0.1%, +4.2%] | - | ~ | device 2.7x faster than the host (was 3.2x); marched/traced 2.63x (was 2.51x) |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 26.0 ms | 26.0 ms | +0.2% | [-17.2%, +21.3%] | - | ~ | device 2.9x faster than the host; marched/traced 3.82x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 58.0 ms | 62.0 ms | +7.0% | [-0.9%, +16.1%] | - | ~ | device 4.2x faster than the host (was 4.8x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 99.5 ms | 99.3 ms | -0.2% | [-11.4%, +12.5%] | - | ~ | device 2.8x faster than the host (was 2.7x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 12.7 ms | 13.8 ms | +8.6% | [-1.7%, +21.2%] | - | ~ | device 3.7x faster than the host (was 4.0x) |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.5 ms | 17.4 ms | -0.4% | [-0.8%, -0.0%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 285 ms | 289 ms | +1.3% | [-2.2%, +4.9%] | - | ~ | device 0.2x faster than the host; marched/traced 1.93x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 344 ms | 345 ms | +0.3% | [-3.4%, +4.1%] | - | ~ | device 0.2x faster than the host; marched/traced 2.94x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 556 ms | 558 ms | +0.4% | [+0.4%, +0.5%] | - | ~ | device 0.6x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.01 s | 1.01 s | +0.7% | [-0.6%, +2.1%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 174 ms | 180 ms | +3.9% | [-12.0%, +22.1%] | - | ~ | device 0.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 230 ms | 227 ms | -1.2% | [-14.0%, +13.1%] | - | ~ | device 0.4x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.61 ms | 2.60 ms | -0.6% | [-9.3%, +8.8%] | - | ~ | device 4.8x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.46 ms | 3.44 ms | -0.6% | [-13.7%, +14.3%] | - | ~ | device 6.4x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 94.0 ms | 94.5 ms | +0.6% | [-9.9%, +12.2%] | - | ~ | device 4.6x faster than the host; each/one-pass 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.70 ms | 3.71 ms | +0.3% | [-1.2%, +1.9%] | - | ~ | device 3.8x faster than the host (was 4.3x) |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.75 ms | 6.76 ms | +0.1% | [-1.8%, +2.1%] | - | ~ | device 3.7x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.59 ms | 5.58 ms | -0.2% | [-2.3%, +2.0%] | - | ~ | device 3.2x faster than the host; regions/hit 1.21x |
| `Regions/particles/region/double/hostdevice` | 3.58 ms | 3.54 ms | -1.1% | [-12.5%, +11.8%] | - | ~ | device 2.1x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.88 ms | 5.88 ms | +0.2% | [-2.9%, +3.3%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 132 ms | 133 ms | +0.3% | [-11.5%, +13.5%] | - | ~ | device 2.3x faster than the host (was 2.5x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.45 ms | 5.46 ms | +0.2% | [-14.5%, +17.5%] | - | ~ | device 2.6x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.3 ms | -3.4% | [-6.6%, -0.2%] | - | faster | device 1.6x faster than the host (was 1.6x) |
| `Regions/assembly/firstHit/double/hostdevice` | 12.5 ms | 12.1 ms | -3.1% | [-12.5%, +7.0%] | - | ~ | device 1.4x faster than the host (was 1.4x); regions/hit 1.26x |
| `Render/csg/frame/float/hostdevice` | 2.34 ms | 2.36 ms | +0.9% | [-2.0%, +3.9%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.32 ms | 2.32 ms | +0.2% | [-3.6%, +4.1%] | - | ~ | device 3.1x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.93 ms | 1.92 ms | -0.2% | [-15.9%, +18.2%] | - | **DIFFER** | device 8.6x faster than the host; 1 host and device answers DIFFER (max diff 0.11) |
| `Render/csg/frame/double/hostdevice` | 5.60 ms | 5.62 ms | +0.4% | [-0.1%, +0.8%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.28 ms | 5.26 ms | -0.4% | [-12.5%, +13.3%] | - | ~ | device 1.5x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 21.5 ms | 21.5 ms | -0.0% | [-17.2%, +21.1%] | - | ~ | device 0.8x faster than the host |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 144 µs | 143 µs | -0.2% | [-15.8%, +18.4%] | - | ~ | device 36.1x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.3 µs | 47.1 µs | -0.5% | [-1.4%, +0.4%] | - | ~ | device 18.5x faster than the host (was 19.2x) |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 50.3 µs | 49.9 µs | -0.8% | [-8.5%, +7.5%] | - | ~ | device 14.4x faster than the host |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 52.5 µs | 52.4 µs | -0.3% | [-15.4%, +17.6%] | - | ~ | device 61.0x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 47.7 µs | 47.7 µs | -0.0% | [-0.7%, +0.7%] | - | ~ | device 15.5x faster than the host |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 48.0 µs | 48.0 µs | +0.0% | [-0.2%, +0.2%] | - | ~ | device 17.5x faster than the host |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 47.8 µs | 48.1 µs | +0.6% | [+0.3%, +1.0%] | - | ~ | device 9.7x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 48.2 µs | 47.6 µs | -1.1% | [-1.9%, -0.3%] | - | ~ | device 7.3x faster than the host (was 7.7x) |
| `Shapes/wedge/signedDistance/float/hostdevice` | 47.4 µs | 47.6 µs | +0.4% | [-0.2%, +0.9%] | - | ~ | device 8.7x faster than the host |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 556 µs | 557 µs | +0.1% | [-0.2%, +0.5%] | - | ~ | device 8.2x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.24 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 493 µs | 492 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 421 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.0x faster than the host (was 1.9x) |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.5x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 358 µs | 358 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.3x faster than the host |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 669 µs | 670 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.6x faster than the host |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 414 µs | 414 µs | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.3x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 339 µs | -0.1% | [-0.1%, +0.0%] | - | ~ | device 1.3x faster than the host (was 1.5x) |
| `Shapes/wedge/signedDistance/double/hostdevice` | 154 µs | 154 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.8x faster than the host |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.02 ms | 8.03 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 892 µs | 885 µs | -0.8% | [-1.5%, -0.1%] | +0.0% | ~ | device 10.4x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.65 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.6 ms | 13.6 ms | -0.2% | [-1.3%, +0.9%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | +0.2% | [-1.8%, +2.1%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | -0.1% | [-1.0%, +0.9%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.2 ms | 11.1 ms | -1.1% | [-3.4%, +1.3%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.00 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 7.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.81 ms | 4.79 ms | -0.5% | [-1.9%, +1.0%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.8 ms | 16.8 ms | +0.1% | [-0.9%, +1.1%] | +0.0% | ~ | device 5.1x faster than the host; tape/fused 1.34x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.4 ms | 24.5 ms | +0.1% | [-2.4%, +2.6%] | +0.0% | ~ | device 3.7x faster than the host; tape/fused 1.23x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.4 ms | 22.5 ms | +0.5% | [-3.9%, +5.1%] | +0.0% | ~ | device 3.9x faster than the host (was 4.3x) |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.1 ms | 30.1 ms | -0.0% | [-2.8%, +2.7%] | +0.0% | ~ | device 3.1x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 40.3 ms | 40.4 ms | +0.3% | [-16.6%, +20.3%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 41.8 ms | 41.5 ms | -0.8% | [-17.7%, +19.4%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 116 ms | 114 ms | -2.0% | [-16.2%, +14.9%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 120 ms | 119 ms | -1.2% | [-15.3%, +15.4%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 121 ms | 119 ms | -2.0% | [-15.0%, +13.3%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 126 ms | 125 ms | -1.0% | [-12.3%, +11.8%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.49 ms | 1.49 ms | -0.1% | [-13.2%, +14.9%] | +0.0% | ~ | device 11.4x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 28.4 ms | 28.4 ms | +0.0% | [-17.4%, +21.2%] | +0.0% | ~ | device 0.7x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 8.53 ms | 8.53 ms | +0.0% | [-17.1%, +20.6%] | - | ~ | device 14.3x faster than the host (was 12.7x) |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.93 ms | 5.93 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 9.9x faster than the host; surface/leaf 1.44x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 96.2 ms | 95.2 ms | -1.0% | [-4.9%, +3.0%] | - | ~ | device 3.6x faster than the host (was 3.9x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 61.4 ms | 61.5 ms | +0.2% | [-6.2%, +7.0%] | - | ~ | device 3.6x faster than the host (was 3.3x); surface/leaf 1.55x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 28.0 ms | 28.1 ms | +0.2% | [-7.6%, +8.6%] | - | ~ | device 5.2x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.93 ms | 9.94 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 6.9x faster than the host; surface/hit 2.83x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 57.7 ms | 53.9 ms | -6.6% | [-17.4%, +4.7%] | - | ~ | device 2.0x faster than the host (was 1.9x) |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 35.5 ms | 35.5 ms | +0.2% | [-17.3%, +21.3%] | - | ~ | device 1.7x faster than the host; surface/leaf 1.52x (was 1.63x) |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 791 ms | 788 ms | -0.4% | [-1.6%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 530 ms | 531 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.48x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 306 ms | 304 ms | -0.7% | [-3.5%, +2.2%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 112 ms | 110 ms | -1.6% | [-16.4%, +16.2%] | - | ~ | device 0.6x faster than the host; surface/hit 2.76x |
| `Tape/csg/signedDistance/float/hostdevice` | 201 µs | 201 µs | +0.0% | [-15.8%, +18.8%] | - | ~ | device 12.3x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 49.0 µs | 47.5 µs | -3.0% | [-5.7%, -0.1%] | - | ~ | device 24.0x faster than the host (was 23.1x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 330 µs | 330 µs | -0.0% | [-16.1%, +19.3%] | - | ~ | device 13.9x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.46 ms | 1.45 ms | -0.4% | [-13.6%, +14.8%] | - | ~ | device 9.8x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 68.3 µs | 69.4 µs | +1.6% | [-7.1%, +11.3%] | - | ~ | device 7.3x faster than the host (was 7.8x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 51.5 µs | 47.5 µs | -7.8% | [-14.7%, +0.4%] | - | ~ | device 4.7x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 652 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.4% | [-0.8%, +0.1%] | - | ~ | device 11.9x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 476 µs | 476 µs | -0.1% | [-0.5%, +0.4%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.2 µs | 48.5 µs | +0.5% | [+0.1%, +0.9%] | - | ~ | device 26.4x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 857 µs | 857 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.1x faster than the host (was 3.0x) |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.65 ms | 6.65 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 193 µs | 191 µs | -0.6% | [-1.3%, +0.0%] | - | ~ | device 2.8x faster than the host (was 2.6x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.3% | [-0.4%, -0.2%] | - | ~ | device 2.2x faster than the host (was 1.9x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.62 ms | +0.6% | [-0.2%, +1.3%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.72 ms | 3.71 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 10.8x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.50 ms | 6.50 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.7 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 6195.4 s → 6217.4 s (+0.4%), the largest peak 6548 MB → 6548 MB; 1 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.6 s | 14.5 s | +6.4% | 605 MB | 606 MB | +0.1% | ~ |
| `BVHRefit` | 7.1 s | 7.5 s | +6.4% | 507 MB | 509 MB | +0.3% | ~ |
| `BVHUnion` | 26.3 s | 30.0 s | +13.9% | 1035 MB | 1037 MB | +0.1% | **minor** |
| `BVHUnionMixed` | 11.6 s | 12.2 s | +5.4% | 630 MB | 631 MB | +0.1% | ~ |
| `ClosestPrimitive` | 131.8 s | 134.2 s | +1.8% | 1352 MB | 1353 MB | +0.1% | ~ |
| `ClosestSurface` | 196.5 s | 199.8 s | +1.6% | 1844 MB | 1844 MB | +0.0% | ~ |
| `Domain` | 61.4 s | 62.3 s | +1.5% | 998 MB | 999 MB | +0.1% | ~ |
| `DomainObject` | 59.1 s | 59.6 s | +0.8% | 952 MB | 953 MB | +0.1% | ~ |
| `Fused` | 56.7 s | 57.1 s | +0.7% | 1163 MB | 1164 MB | +0.1% | ~ |
| `LaunchTuning` | 563.3 s | 565.8 s | +0.4% | 3859 MB | 3859 MB | -0.0% | ~ |
| `Mesh` | 21.9 s | 22.2 s | +1.3% | 810 MB | 811 MB | +0.1% | ~ |
| `MeshCSG` | 433.3 s | 442.8 s | +2.2% | 6284 MB | 6284 MB | +0.0% | ~ |
| `MeshClosestPoint` | 19.6 s | 19.8 s | +1.5% | 762 MB | 763 MB | +0.1% | ~ |
| `MeshReconcile` | 10.4 s | 10.6 s | +2.2% | 513 MB | 513 MB | +0.1% | ~ |
| `MeshTape` | 794.1 s | 794.4 s | +0.0% | 6548 MB | 6548 MB | +0.0% | ~ |
| `PointCloud` | 12.8 s | 13.6 s | +6.2% | 571 MB | 572 MB | +0.2% | ~ |
| `PointCloudBVHBuild` | 7.4 s | 7.5 s | +1.6% | 508 MB | 509 MB | +0.4% | ~ |
| `PointCloudHashGridBuild` | 6.4 s | 6.9 s | +7.6% | 508 MB | 509 MB | +0.4% | ~ |
| `QueryOrder` | - | 16.4 s | | - | 694 MB | | new |
| `Ray` | 200.0 s | 202.1 s | +1.1% | 3755 MB | 3755 MB | +0.0% | ~ |
| `RayCSG` | 233.4 s | 235.3 s | +0.8% | 4139 MB | 4139 MB | +0.0% | ~ |
| `RayCrossing` | 453.1 s | 463.3 s | +2.3% | 4102 MB | 4102 MB | +0.0% | ~ |
| `RayParticles` | 891.5 s | 881.3 s | -1.2% | 5781 MB | 5780 MB | -0.0% | ~ |
| `RayShell` | 535.3 s | 536.8 s | +0.3% | 5488 MB | 5488 MB | +0.0% | ~ |
| `Regions` | 385.6 s | 387.3 s | +0.4% | 4128 MB | 4128 MB | +0.0% | ~ |
| `Render` | 267.7 s | 272.7 s | +1.9% | 6048 MB | 6047 MB | -0.0% | ~ |
| `Shapes` | 26.9 s | 28.1 s | +4.3% | 592 MB | 593 MB | +0.2% | ~ |
| `SphereCloudBVH` | 9.4 s | 9.0 s | -4.2% | 508 MB | 510 MB | +0.4% | ~ |
| `SphereCloudRays` | 188.5 s | 187.5 s | -0.5% | 4153 MB | 4153 MB | -0.0% | ~ |
| `SphereContacts` | 8.1 s | 8.1 s | -0.4% | 507 MB | 509 MB | +0.3% | ~ |
| `SurfaceParts` | 444.6 s | 431.7 s | -2.9% | 4397 MB | 4397 MB | +0.0% | ~ |
| `Tape` | 71.0 s | 68.4 s | -3.6% | 984 MB | 986 MB | +0.1% | ~ |
| `TapeBVHUnion` | 47.2 s | 45.1 s | -4.3% | 925 MB | 926 MB | +0.1% | ~ |
| **all, in both** | 6195.4 s | 6217.4 s | +0.4% | 6548 MB | 6548 MB | +0.0% | ~ |

Device kernels: 218 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 4 new, 0 gone. Spills are bytes stored/loaded per thread.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (13 cases): DomainObject/partsOwn/labels/double/hostdevice, Fused/chain16/signedDistance/double/hostdevice, LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice, MeshCSG/smooth-fused/firstHit-random/double/hostdevice, PointCloudBVH/uniform/neighbors8-morton/double/hostdevice, PointCloudBVH/sphere/closest-random/double/hostdevice, Ray/mesh/firstHit-morton/float/hostdevice, Ray/mesh-polygons/firstHit-morton/double/hostdevice, RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice, RayParticles/armadillo-shell-tape/step-morton/double/hostdevice, RayParticles/armadillo/step-morton/double/hostdevice, SurfaceParts/boxes/closestLeaf/float/hostdevice, SurfaceParts/mesh-groups/firstHit/float/hostdevice

The probes differ between the sides (each side ran its own): Performance/QueryOrder/main.cpp

<!-- END receipt cuda -->
