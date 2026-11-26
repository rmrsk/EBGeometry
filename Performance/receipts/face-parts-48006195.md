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
base: 344147cc
base-fingerprint: 1e0f461958307254a04eb0a9ea2890c216b99160
head: b42803d5
head-fingerprint: 480061955ebed0a11d3b2660afa7564b0241908b
cases: 308
rounds: 2
reps: 3
date: 2026-11-26
-->

## Host comparison: green

**Accepted:** -

`b42803d5` against `344147cc` (origin/main), 2026-11-26.

**VERDICT: no regression (0 major, 0 minor across 308 cases)**

Performance comparison: 0 major, 0 minor, 5 faster, 0 differ, 0 lost, 308 cases

- base: 344147cc (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: b42803d5 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 342 ms | 344 ms | +0.8% | [+0.2%, +1.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 183 ms | 181 ms | -0.8% | [-1.7%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 523 ms | 523 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 357 ms | 355 ms | -0.6% | [-1.3%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 192 ms | 193 ms | +0.5% | [-0.6%, +1.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 584 ms | 581 ms | -0.6% | [-2.7%, +1.5%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 46.0 ms | 46.0 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 81.0 ms | 81.7 ms | +0.9% | [+0.6%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 58.0 ms | 57.9 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 52.4 ms | 52.3 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 66.2 ms | 66.2 ms | +0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 56.4 ms | 56.0 ms | -0.7% | [-1.5%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 62.4 ms | 62.5 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 73.9 ms | 73.8 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.0 ms | 74.2 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 88.7 ms | 83.4 ms | -6.0% | [-17.1%, +7.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.2 ms | 67.5 ms | +0.4% | [-0.4%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 109 ms | 108 ms | -1.0% | [-2.9%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 79.6 ms | 80.9 ms | +1.7% | [-0.3%, +3.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 122 ms | 124 ms | +1.4% | [-0.4%, +3.3%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 66.0 ms | 66.5 ms | +0.7% | [-0.3%, +1.8%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 99.1 ms | 99.4 ms | +0.3% | [-1.4%, +2.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 78.0 ms | 78.0 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 107 ms | 107 ms | -0.2% | [-1.9%, +1.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.7 ms | 19.8 ms | +0.4% | [-0.8%, +1.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 42.1 ms | 41.9 ms | -0.5% | [-0.7%, -0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 75.8 ms | 76.0 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.8 ms | 20.4 ms | -2.1% | [-2.4%, -1.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 44.6 ms | 44.6 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 85.8 ms | 85.5 ms | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 142 ms | 142 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 77.2 ms | 78.0 ms | +1.0% | [-0.4%, +2.4%] | +0.0% | ~ | surface/leaf 1.81x |
| `ClosestSurface/union/closestSurface/float/host` | 155 ms | 150 ms | -3.6% | [-7.0%, +0.2%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 80.0 ms | 80.5 ms | +0.7% | [-0.6%, +2.1%] | +0.0% | ~ | surface/leaf 1.86x (was 1.94x) |
| `ClosestSurface/cloud/closestSurface/double/host` | 165 ms | 164 ms | -0.7% | [-0.8%, -0.5%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 84.3 ms | 84.2 ms | -0.2% | [-0.2%, -0.2%] | +0.0% | ~ | surface/leaf 1.94x |
| `ClosestSurface/union/closestSurface/double/host` | 163 ms | 163 ms | +0.1% | [-3.2%, +3.6%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 83.6 ms | 83.0 ms | -0.7% | [-1.6%, +0.1%] | +0.0% | ~ | surface/leaf 1.97x |
| `ConnectedComponents/shells/build-coarse/float/host` | 153 ms | 152 ms | -1.0% | [-1.9%, -0.0%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/float/host` | 698 ms | 693 ms | -0.7% | [-2.1%, +0.6%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-coarse/double/host` | 164 ms | 160 ms | -2.2% | [-3.4%, -1.0%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/double/host` | 722 ms | 719 ms | -0.5% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `Domain/particles/labels/float/host` | 13.6 ms | 13.7 ms | +0.7% | [-1.1%, +2.5%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.80 ms | 6.88 ms | +1.3% | [+1.1%, +1.4%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.33 ms | 1.34 ms | +0.9% | [-0.2%, +2.1%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.5 ms | 14.3 ms | -1.8% | [-3.3%, -0.3%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.19 ms | 7.21 ms | +0.2% | [-1.4%, +1.8%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.31 ms | 1.32 ms | +0.6% | [-0.8%, +2.1%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 40.2 ms | 40.5 ms | +0.8% | [-0.2%, +1.9%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.5 ms | 21.5 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 564 µs | 567 µs | +0.5% | [+0.5%, +0.5%] | +0.0% | ~ | whole/own 71.52x |
| `DomainObject/partsOwn/labelAt/float/host` | 302 µs | 307 µs | +1.4% | [+0.2%, +2.6%] | +0.0% | ~ | whole/own 70.05x |
| `DomainObject/partsWhole/labels/double/host` | 40.0 ms | 40.0 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 23.0 ms | 22.8 ms | -0.6% | [-1.8%, +0.6%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 598 µs | 631 µs | +5.5% | [-2.4%, +13.5%] | +0.0% | ~ | whole/own 63.39x (was 66.87x) |
| `DomainObject/partsOwn/labelAt/double/host` | 320 µs | 322 µs | +0.6% | [-0.8%, +2.0%] | +0.0% | ~ | whole/own 70.82x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/host` | 66.0 ms | 65.9 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/host` | 61.6 ms | 61.3 ms | -0.5% | [-0.9%, +0.0%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/host` | 81.8 ms | 82.3 ms | +0.6% | [-2.2%, +3.6%] | +0.0% | ~ | grid/shuffled 0.80x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/host` | 70.1 ms | 70.7 ms | +0.8% | [-0.8%, +2.5%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/host` | 65.8 ms | 65.6 ms | -0.3% | [-1.9%, +1.3%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/host` | 95.3 ms | 95.9 ms | +0.7% | [-1.2%, +2.6%] | +0.0% | ~ | grid/shuffled 0.74x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/host` | 200 ms | 202 ms | +1.2% | [+0.2%, +2.2%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/host` | 187 ms | 188 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/host` | 279 ms | 278 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ | grid/shuffled 0.73x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/host` | 221 ms | 214 ms | -2.8% | [-4.0%, -1.6%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/host` | 201 ms | 200 ms | -0.5% | [-2.0%, +1.0%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/host` | 341 ms | 344 ms | +0.9% | [-1.7%, +3.5%] | +0.0% | ~ | grid/shuffled 0.62x (was 0.65x) |
| `Fused/csg/signedDistance/float/host` | 4.56 ms | 4.52 ms | -0.8% | [-1.4%, -0.2%] | - | ~ | tape/fused 2.90x |
| `Fused/smooth/signedDistance/float/host` | 12.4 ms | 12.6 ms | +1.0% | [-0.1%, +2.0%] | - | ~ | tape/fused 2.05x |
| `Fused/chain16/signedDistance/float/host` | 981 µs | 992 µs | +1.1% | [-0.0%, +2.2%] | - | ~ | tape/fused 16.59x |
| `Fused/drilled-mesh/signedDistance/float/host` | 48.7 ms | 48.9 ms | +0.5% | [-2.8%, +3.9%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 48.8 ms | 48.9 ms | +0.3% | [-2.2%, +9.7%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.59 ms | 4.61 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | tape/fused 2.86x |
| `Fused/smooth/signedDistance/double/host` | 12.9 ms | 12.9 ms | -0.3% | [-1.3%, +0.6%] | - | ~ | tape/fused 2.08x |
| `Fused/chain16/signedDistance/double/host` | 3.91 ms | 3.96 ms | +1.4% | [+0.0%, +2.8%] | - | ~ | tape/fused 4.21x |
| `Fused/drilled-mesh/signedDistance/double/host` | 58.1 ms | 56.6 ms | -2.7% | [-6.2%, +0.9%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 57.5 ms | 56.9 ms | -1.0% | [-1.9%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 85.0 ms | 85.1 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 239 ms | 239 ms | -0.2% | [-1.5%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 116 ms | 117 ms | +1.0% | [+0.3%, +1.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 286 ms | 289 ms | +1.0% | [-2.7%, +4.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 73.3 ms | 73.0 ms | -0.5% | [-1.0%, +0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 222 ms | 223 ms | +0.6% | [-0.1%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 99.1 ms | 99.1 ms | -0.0% | [-0.8%, +0.8%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 31.4 ms | 31.7 ms | +0.9% | [-0.3%, +2.1%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 848 ms | 821 ms | -3.2% | [-7.2%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 122 ms | 122 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 56.4 ms | 57.4 ms | +1.7% | [+0.4%, +3.1%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 986 ms | 1000 ms | +1.4% | [-1.0%, +3.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 27.6 ms | 27.5 ms | -0.3% | [-1.2%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 16.1 ms | 16.2 ms | +0.5% | [-0.9%, +2.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 334 ms | 336 ms | +0.5% | [-1.3%, +2.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 237 ms | 237 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 93.5 ms | 93.1 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ | tape/fused 3.61x |
| `MeshCSG/smooth-fused/frame/float/host` | 83.6 ms | 83.5 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | tape/fused 2.84x |
| `MeshCSG/union-tape/firstHit/double/host` | 30.2 ms | 30.3 ms | +0.2% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.9 ms | 17.8 ms | -0.6% | [-2.0%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 407 ms | 434 ms | +6.5% | [-5.3%, +18.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 264 ms | 266 ms | +0.7% | [-4.5%, +6.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 108 ms | 108 ms | +0.5% | [-7.0%, +8.4%] | +0.0% | ~ | tape/fused 4.02x (was 3.79x) |
| `MeshCSG/smooth-fused/frame/double/host` | 95.7 ms | 95.1 ms | -0.6% | [-11.2%, +11.3%] | +0.0% | ~ | tape/fused 2.79x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 262 ms | 262 ms | -0.0% | [-15.8%, +18.9%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 272 ms | 271 ms | -0.3% | [-20.4%, +24.9%] | +0.0% | ~ | signedDistance/closestPoint 0.97x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 468 ms | 438 ms | -6.3% | [-42.6%, +53.0%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 273 ms | 270 ms | -1.3% | [-15.6%, +15.5%] | +0.0% | ~ | closestPoint/gradient 1.00x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 323 ms | 327 ms | +1.4% | [-20.6%, +29.7%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 336 ms | 336 ms | -0.0% | [-27.1%, +36.9%] | +0.0% | ~ | signedDistance/closestPoint 0.97x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 530 ms | 516 ms | -2.5% | [-41.7%, +64.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 340 ms | 332 ms | -2.4% | [-26.3%, +28.9%] | +0.0% | ~ | closestPoint/gradient 1.01x |
| `MeshIngest/armadillo/read-soup/float/host` | 57.6 ms | 57.4 ms | -0.3% | [-5.5%, +5.1%] | - | ~ |  |
| `MeshIngest/armadillo/clean/float/host` | 14.5 ms | 14.7 ms | +1.3% | [-12.0%, +16.5%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/float/host` | 113 ms | 111 ms | -2.2% | [-19.3%, +18.6%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/float/host` | 118 ms | 118 ms | -0.1% | [-14.4%, +16.6%] | - | ~ |  |
| `MeshIngest/armadillo/read-soup/double/host` | 58.0 ms | 57.9 ms | -0.1% | [-7.1%, +7.3%] | - | ~ |  |
| `MeshIngest/armadillo/clean/double/host` | 15.9 ms | 15.5 ms | -2.9% | [-18.5%, +15.8%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/double/host` | 127 ms | 128 ms | +0.2% | [-19.5%, +24.7%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/double/host` | 142 ms | 138 ms | -3.0% | [-16.9%, +12.2%] | - | ~ |  |
| `MeshReconcile/armadillo/reconcile/float/host` | 102 ms | 104 ms | +1.9% | [-36.6%, +62.8%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 50.6 ms | 50.6 ms | -0.0% | [-22.2%, +28.6%] | +0.0% | ~ | reconcile/scheduled 2.05x |
| `MeshReconcile/armadillo/reconcile/double/host` | 139 ms | 139 ms | +0.6% | [-32.9%, +51.1%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 68.0 ms | 67.2 ms | -1.2% | [-18.6%, +20.4%] | +0.0% | ~ | reconcile/scheduled 2.07x |
| `MeshTape/mesh/firstHit/float/host` | 53.3 ms | 53.3 ms | -0.1% | [-9.9%, +10.7%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 55.1 ms | 54.7 ms | -0.7% | [-15.8%, +17.3%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 21.1 ms | 20.7 ms | -2.0% | [-8.1%, +4.7%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 86.5 ms | 85.9 ms | -0.6% | [-7.8%, +7.2%] | +0.0% | ~ | mesh/tape 0.62x |
| `MeshTape/tape/firstCrossing/float/host` | 54.5 ms | 53.8 ms | -1.4% | [-9.8%, +7.9%] | +0.0% | ~ | mesh/tape 1.02x |
| `MeshTape/tape/frame/float/host` | 25.5 ms | 25.4 ms | -0.3% | [-3.8%, +3.4%] | +0.0% | ~ | mesh/tape 0.81x |
| `MeshTape/view/firstHit/float/host` | 87.4 ms | 86.5 ms | -1.0% | [-3.6%, +1.7%] | +0.0% | ~ | mesh/view 0.62x |
| `MeshTape/view/frame/float/host` | 27.2 ms | 27.4 ms | +0.9% | [-2.1%, +4.0%] | +0.0% | ~ | mesh/view 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 59.0 ms | 58.2 ms | -1.4% | [-3.7%, +1.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 61.8 ms | 55.5 ms | -10.1% | [-16.6%, -3.0%] | +0.0% | faster |  |
| `MeshTape/mesh/frame/double/host` | 21.5 ms | 21.2 ms | -1.5% | [-2.5%, -0.4%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 92.8 ms | 94.3 ms | +1.6% | [+1.4%, +1.8%] | +0.0% | ~ | mesh/tape 0.62x |
| `MeshTape/tape/firstCrossing/double/host` | 59.5 ms | 59.9 ms | +0.6% | [-2.4%, +3.7%] | +0.0% | ~ | mesh/tape 0.93x (was 1.04x) |
| `MeshTape/tape/frame/double/host` | 28.2 ms | 28.0 ms | -0.8% | [-1.6%, -0.0%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 96.1 ms | 94.9 ms | -1.2% | [-1.7%, -0.7%] | +0.0% | ~ | mesh/view 0.61x |
| `MeshTape/view/frame/double/host` | 30.8 ms | 30.5 ms | -0.9% | [-1.2%, -0.7%] | +0.0% | ~ | mesh/view 0.69x |
| `PointCloudBVH/uniform/build/float/host` | 55.1 ms | 55.4 ms | +0.5% | [-0.2%, +1.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 157 ms | 154 ms | -1.4% | [-2.4%, -0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 380 ms | 379 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 83.1 ms | 83.4 ms | +0.4% | [+0.0%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.87 ms | 6.89 ms | +0.3% | [-0.2%, +0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 57.5 ms | 58.1 ms | +1.1% | [-0.9%, +3.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 186 ms | 188 ms | +1.3% | [-13.8%, +18.8%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 420 ms | 416 ms | -0.9% | [-5.3%, +3.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 109 ms | 118 ms | +8.3% | [-1.6%, +20.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.83 ms | 7.80 ms | -0.4% | [-13.3%, +14.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 14.0 ms | 13.5 ms | -3.4% | [-12.0%, +5.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 140 ms | 125 ms | -10.6% | [-22.5%, +4.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 501 ms | 501 ms | +0.1% | [-4.8%, +5.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 158 ms | 161 ms | +2.1% | [-13.8%, +20.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 877 ms | 876 ms | -0.1% | [-11.9%, +13.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 16.0 ms | 15.6 ms | -2.4% | [-11.0%, +7.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 127 ms | 127 ms | -0.3% | [-5.2%, +4.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 523 ms | 516 ms | -1.3% | [-5.0%, +2.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 160 ms | 161 ms | +0.4% | [-7.0%, +8.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 850 ms | 819 ms | -3.6% | [-6.7%, -0.2%] | - | faster |  |
| `PointCloudBVH/sphere/build/float/host` | 56.4 ms | 56.3 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 139 ms | 139 ms | -0.0% | [-0.8%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 306 ms | 285 ms | -6.8% | [-14.5%, +2.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 156 ms | 157 ms | +0.4% | [-2.9%, +3.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 141 ms | 141 ms | +0.3% | [-3.3%, +3.9%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 58.6 ms | 60.3 ms | +2.9% | [+1.2%, +4.7%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 160 ms | 157 ms | -1.6% | [-2.1%, -1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 310 ms | 309 ms | -0.3% | [-1.5%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 176 ms | 176 ms | +0.0% | [-0.8%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 160 ms | 161 ms | +0.3% | [-0.5%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.9 ms | 11.9 ms | -0.1% | [-1.9%, +1.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 117 ms | 117 ms | +0.2% | [-0.0%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 332 ms | 330 ms | -0.7% | [-1.8%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 162 ms | 161 ms | -0.2% | [-4.0%, +3.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 330 ms | 337 ms | +2.0% | [+0.7%, +3.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.9 ms | 13.8 ms | -0.5% | [-2.8%, +1.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 119 ms | 120 ms | +0.9% | [-0.9%, +2.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 361 ms | 361 ms | +0.0% | [-0.8%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 174 ms | 172 ms | -1.3% | [-1.8%, -0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 326 ms | 324 ms | -0.6% | [-0.7%, -0.5%] | - | ~ |  |
| `QueryOrder/random/mortonOrder/float/host` | 62.8 ms | 62.8 ms | +0.0% | [-0.4%, +0.5%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | 257 ms | 249 ms | -3.2% | [-16.6%, +12.9%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | 166 ms | 168 ms | +0.8% | [-0.8%, +2.4%] | +0.0% | ~ | arrived/ordered 1.48x (was 1.55x) |
| `QueryOrder/random/mortonOrder/double/host` | 64.3 ms | 64.4 ms | +0.1% | [-0.4%, +0.7%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | 296 ms | 298 ms | +0.9% | [-0.3%, +2.2%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | 177 ms | 180 ms | +1.8% | [-0.3%, +4.0%] | +0.0% | ~ | arrived/ordered 1.66x |
| `Ray/mesh/firstHit/float/host` | 52.1 ms | 50.5 ms | -3.0% | [-6.1%, +0.3%] | +0.0% | ~ | hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled/float/host` | 48.4 ms | 47.9 ms | -0.9% | [-2.1%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 42.2 ms | 42.1 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 86.5 ms | 86.3 ms | -0.3% | [-2.4%, +2.0%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 72.2 ms | 72.5 ms | +0.5% | [-0.6%, +1.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.29 ms | 5.28 ms | -0.3% | [-1.7%, +1.0%] | - | ~ | tape/fused 1.48x |
| `Ray/csg-tape/firstHit/float/host` | 7.75 ms | 7.80 ms | +0.6% | [-2.0%, +3.4%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 56.9 ms | 56.9 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled/double/host` | 54.8 ms | 54.8 ms | +0.1% | [-0.8%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 48.1 ms | 48.3 ms | +0.4% | [-0.9%, +1.7%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 105 ms | 105 ms | -0.0% | [-12.9%, +16.4%] | +0.0% | ~ | triangles/polygons 0.54x |
| `Ray/mesh-polygons/intersects/double/host` | 90.3 ms | 90.2 ms | -0.1% | [-7.6%, +8.0%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.57 ms | 5.64 ms | +1.1% | [+0.1%, +2.2%] | - | ~ | tape/fused 1.83x |
| `Ray/csg-tape/firstHit/double/host` | 10.3 ms | 10.3 ms | +0.3% | [-34.1%, +52.6%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 25.2 ms | 24.9 ms | -1.0% | [-45.4%, +79.0%] | - | ~ | marched/traced 4.97x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 124 ms | 124 ms | +0.2% | [-0.1%, +0.5%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.5 ms | 22.6 ms | +0.6% | [+0.1%, +1.0%] | +0.0% | ~ | marched/traced 6.58x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 147 ms | 149 ms | +1.0% | [-0.3%, +2.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 36.3 ms | 36.3 ms | -0.0% | [-1.7%, +1.7%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.3 ms | 19.6 ms | +1.8% | [+0.1%, +3.5%] | - | ~ | marched/traced 6.61x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 129 ms | 130 ms | +0.6% | [-0.3%, +1.5%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 24.1 ms | 24.1 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ | marched/traced 6.36x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 153 ms | 153 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.3 ms | 38.6 ms | +0.8% | [+0.2%, +1.4%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 51.1 ms | 50.9 ms | -0.5% | [-2.2%, +1.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 50.5 ms | 50.8 ms | +0.7% | [+0.6%, +0.9%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.72 ms | 7.68 ms | -0.5% | [-1.3%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.11 ms | 6.18 ms | +1.0% | [+0.6%, +1.4%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 187 ms | 186 ms | -0.5% | [-0.9%, -0.0%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 166 ms | 165 ms | -0.5% | [-0.9%, -0.0%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 72.3 ms | 72.2 ms | -0.2% | [-1.8%, +1.5%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 59.8 ms | 59.3 ms | -0.8% | [-3.1%, +1.5%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 57.9 ms | 57.7 ms | -0.3% | [-3.3%, +2.8%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 57.6 ms | 57.6 ms | -0.1% | [-2.4%, +2.3%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.98 ms | 7.97 ms | -0.1% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.33 ms | 6.32 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 216 ms | 215 ms | -0.7% | [-1.5%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 192 ms | 192 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 83.6 ms | 83.6 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 71.9 ms | 69.8 ms | -2.8% | [-5.5%, +0.0%] | +0.0% | ~ | hit/crossing 1.20x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.6 ms | 11.6 ms | +0.5% | [-3.0%, +4.1%] | +0.0% | ~ | marched/traced 4.95x (was 4.80x) |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 55.6 ms | 57.5 ms | +3.6% | [-3.9%, +11.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.22 ms | 8.42 ms | +2.4% | [-0.2%, +5.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.11 ms | 2.07 ms | -2.0% | [-3.1%, -0.9%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.11 ms | 7.11 ms | +0.1% | [-0.7%, +0.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.6 ms | 12.5 ms | -1.0% | [-2.6%, +0.6%] | +0.0% | ~ | marched/traced 5.51x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 68.7 ms | 68.9 ms | +0.3% | [-2.3%, +3.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 9.18 ms | 9.22 ms | +0.5% | [+0.2%, +0.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.13 ms | 2.13 ms | +0.4% | [-1.6%, +2.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.55 ms | 7.37 ms | -2.4% | [-6.6%, +2.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 246 ms | 248 ms | +0.6% | [-0.5%, +1.8%] | +0.0% | ~ | marched/traced 1.39x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 344 ms | 343 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 244 ms | 243 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 263 ms | 267 ms | +1.4% | [+0.5%, +2.3%] | +0.0% | ~ | marched/traced 1.51x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 405 ms | 403 ms | -0.5% | [-2.6%, +1.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 263 ms | 264 ms | +0.2% | [-1.7%, +2.0%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.47 ms | 7.50 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.7 ms | 12.7 ms | -0.3% | [-1.8%, +1.2%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 272 ms | 273 ms | +0.4% | [-0.5%, +1.4%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.6 ms | 12.7 ms | +0.9% | [-1.9%, +3.8%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.9 ms | 31.0 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.7 ms | 22.6 ms | -0.2% | [-1.7%, +1.3%] | +0.0% | ~ | regions/hit 1.37x |
| `Regions/particles/region/double/host` | 7.67 ms | 7.75 ms | +1.0% | [+0.4%, +1.6%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 13.0 ms | 12.9 ms | -0.5% | [-1.8%, +0.8%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 287 ms | 287 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.1 ms | 13.1 ms | +0.7% | [-0.4%, +1.8%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 33.4 ms | 32.6 ms | -2.5% | [-5.2%, +0.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 24.2 ms | 24.3 ms | +0.4% | [-1.6%, +2.6%] | +0.0% | ~ | regions/hit 1.34x |
| `Render/csg/frame/float/host` | 12.9 ms | 13.1 ms | +1.6% | [-7.0%, +11.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 12.7 ms | 12.8 ms | +0.6% | [-4.2%, +5.7%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.7 ms | 20.7 ms | +0.2% | [-3.3%, +3.8%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.3 ms | 14.1 ms | -1.4% | [-3.5%, +0.8%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.5 ms | 14.0 ms | -3.2% | [-4.6%, -1.7%] | +0.0% | faster |  |
| `Render/mesh/frame/double/host` | 21.2 ms | 21.1 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 59.4 ms | 60.2 ms | +1.4% | [-3.0%, +5.9%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 39.1 ms | 38.6 ms | -1.2% | [-5.7%, +3.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 98.7 ms | 98.7 ms | +0.1% | [-3.1%, +3.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 66.7 ms | 66.8 ms | +0.1% | [-2.0%, +2.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 44.4 ms | 44.3 ms | -0.2% | [-3.1%, +2.8%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 120 ms | 122 ms | +1.6% | [-2.2%, +5.6%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 78.6 ms | 79.0 ms | +0.6% | [+0.5%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 319 ms | 320 ms | +0.2% | [-2.4%, +2.9%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 316 ms | 319 ms | +1.0% | [-1.3%, +3.3%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 97.4 ms | 96.8 ms | -0.6% | [-0.9%, -0.3%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 384 ms | 390 ms | +1.7% | [+1.0%, +2.4%] | +0.0% | ~ | tape/fused 1.00x (was 1.04x) |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 397 ms | 392 ms | -1.3% | [-4.0%, +1.5%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 179 ms | 173 ms | -3.6% | [-5.6%, -1.6%] | +0.0% | faster |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 212 ms | 210 ms | -1.0% | [-2.7%, +0.6%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 148 ms | 148 ms | -0.1% | [-2.3%, +2.1%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 91.5 ms | 92.1 ms | +0.7% | [+0.2%, +1.2%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 135 ms | 136 ms | +0.7% | [-0.2%, +1.6%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 75.3 ms | 75.9 ms | +0.8% | [+0.6%, +1.0%] | +0.0% | ~ | surface/leaf 1.80x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 65.6 ms | 66.0 ms | +0.6% | [-2.9%, +4.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 36.5 ms | 36.7 ms | +0.6% | [-1.1%, +2.4%] | +0.0% | ~ | surface/hit 1.80x |
| `SurfaceParts/boxes/closestSurface/double/host` | 149 ms | 148 ms | -0.6% | [-2.3%, +1.1%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 93.2 ms | 93.2 ms | +0.0% | [-1.6%, +1.7%] | +0.0% | ~ | surface/leaf 1.59x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 155 ms | 160 ms | +3.3% | [-1.2%, +7.8%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 86.8 ms | 86.8 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ | surface/leaf 1.84x (was 1.78x) |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 75.7 ms | 75.7 ms | +0.0% | [-4.3%, +4.4%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 42.8 ms | 42.7 ms | -0.2% | [-3.0%, +2.8%] | +0.0% | ~ | surface/hit 1.77x |
| `Tape/csg/signedDistance/float/host` | 13.1 ms | 13.1 ms | +0.3% | [-0.6%, +1.3%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.63 ms | 4.65 ms | +0.4% | [-0.2%, +1.1%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.6 ms | 25.7 ms | +0.5% | [-1.7%, +2.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.9 ms | 40.8 ms | -0.2% | [-1.1%, +0.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.78 ms | 1.79 ms | +0.3% | [-1.0%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 144 µs | 131 µs | -9.2% | [-14.7%, -3.4%] | - | faster |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.7 ms | 30.7 ms | +0.1% | [-0.9%, +1.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.74 ms | 5.79 ms | +0.9% | [-1.5%, +3.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.5 ms | 16.5 ms | -0.1% | [-0.9%, +0.8%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.73 ms | 4.74 ms | +0.4% | [-0.0%, +0.8%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.3 ms | 13.2 ms | -1.2% | [-1.6%, -0.7%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.69 ms | 4.65 ms | -0.8% | [-2.8%, +1.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.8 ms | 26.8 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 43.0 ms | 42.8 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.80 ms | 1.81 ms | +0.7% | [-1.3%, +2.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 285 µs | 292 µs | +2.4% | [-3.6%, +9.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.5 ms | 30.5 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.89 ms | 7.86 ms | -0.3% | [-2.8%, +2.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.6 ms | 16.7 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.83 ms | 7.95 ms | +1.5% | [-0.2%, +3.3%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 42.9 ms | 43.1 ms | +0.4% | [-0.6%, +1.4%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 205 ms | 204 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 43.7 ms | 43.8 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 223 ms | 223 ms | -0.2% | [-1.9%, +1.6%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 782.6 s → 781.3 s (-0.2%), the largest peak 1418 MB → 1447 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 10.0 s | 10.0 s | -0.8% | 563 MB | 584 MB | +3.8% | ~ |
| `BVHRefit` | 4.9 s | 4.4 s | -11.1% | 503 MB | 511 MB | +1.6% | ~ |
| `BVHUnion` | 20.0 s | 19.7 s | -1.5% | 956 MB | 946 MB | -1.0% | ~ |
| `BVHUnionMixed` | 7.8 s | 7.8 s | -0.1% | 560 MB | 583 MB | +4.3% | ~ |
| `ClosestPrimitive` | 31.4 s | 31.0 s | -1.4% | 1348 MB | 1335 MB | -1.0% | ~ |
| `ClosestSurface` | 29.1 s | 29.1 s | -0.0% | 1317 MB | 1261 MB | -4.3% | ~ |
| `ConnectedComponents` | 16.0 s | 15.9 s | -1.1% | 855 MB | 829 MB | -3.0% | ~ |
| `Domain` | 18.5 s | 18.6 s | +0.6% | 861 MB | 894 MB | +3.8% | ~ |
| `DomainObject` | 16.6 s | 16.7 s | +0.7% | 860 MB | 829 MB | -3.6% | ~ |
| `EBGrid` | 15.1 s | 15.0 s | -0.6% | 784 MB | 783 MB | -0.1% | ~ |
| `Fused` | 25.4 s | 24.7 s | -2.6% | 1236 MB | 1232 MB | -0.4% | ~ |
| `LaunchTuning` | 38.6 s | 38.5 s | -0.3% | 1328 MB | 1261 MB | -5.0% | ~ |
| `Mesh` | 16.5 s | 16.3 s | -1.4% | 790 MB | 826 MB | +4.5% | ~ |
| `MeshCSG` | 40.1 s | 40.7 s | +1.6% | 1306 MB | 1313 MB | +0.6% | ~ |
| `MeshClosestPoint` | 14.6 s | 14.6 s | +0.4% | 786 MB | 824 MB | +4.9% | ~ |
| `MeshIngest` | 7.3 s | 7.3 s | -0.5% | 569 MB | 593 MB | +4.2% | ~ |
| `MeshReconcile` | 7.2 s | 7.0 s | -2.7% | 569 MB | 593 MB | +4.1% | ~ |
| `MeshTape` | 42.5 s | 42.6 s | +0.4% | 1260 MB | 1299 MB | +3.1% | ~ |
| `PointCloud` | 7.6 s | 7.4 s | -2.5% | 562 MB | 586 MB | +4.2% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.8 s | +0.0% | 477 MB | 475 MB | -0.4% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | -2.4% | 458 MB | 450 MB | -1.6% | ~ |
| `QueryOrder` | 11.7 s | 12.3 s | +4.6% | 708 MB | 723 MB | +2.2% | ~ |
| `Ray` | 37.1 s | 37.5 s | +1.2% | 1342 MB | 1304 MB | -2.8% | ~ |
| `RayCSG` | 32.2 s | 32.6 s | +1.2% | 1204 MB | 1222 MB | +1.5% | ~ |
| `RayCrossing` | 47.4 s | 46.6 s | -1.6% | 1360 MB | 1330 MB | -2.2% | ~ |
| `RayParticles` | 50.2 s | 50.7 s | +0.9% | 1334 MB | 1315 MB | -1.4% | ~ |
| `RayShell` | 38.9 s | 38.8 s | -0.3% | 1312 MB | 1259 MB | -4.0% | ~ |
| `Regions` | 30.3 s | 30.0 s | -0.8% | 1116 MB | 1106 MB | -0.9% | ~ |
| `Render` | 37.0 s | 36.5 s | -1.4% | 1300 MB | 1264 MB | -2.7% | ~ |
| `Shapes` | 8.8 s | 8.9 s | +0.4% | 558 MB | 584 MB | +4.6% | ~ |
| `SphereCloudBVH` | 4.9 s | 4.8 s | -2.3% | 518 MB | 528 MB | +2.0% | ~ |
| `SphereCloudRays` | 29.0 s | 28.8 s | -0.5% | 1077 MB | 1092 MB | +1.4% | ~ |
| `SphereContacts` | 4.3 s | 4.4 s | +1.5% | 490 MB | 494 MB | +0.7% | ~ |
| `SurfaceParts` | 42.5 s | 42.7 s | +0.5% | 1418 MB | 1447 MB | +2.0% | ~ |
| `Tape` | 17.1 s | 17.5 s | +2.3% | 838 MB | 852 MB | +1.6% | ~ |
| `TapeBVHUnion` | 14.9 s | 14.9 s | +0.2% | 839 MB | 830 MB | -1.0% | ~ |
| **all, in both** | 782.6 s | 781.3 s | -0.2% | 1418 MB | 1447 MB | +2.0% | ~ |


Flagged after 2 rounds, and judged on 10 (3 cases): ClosestPrimitive/csg/closestPrimitive/float/host, EBGrid/MeshSDF/armadillo/distance-shuffled/double/host, Fused/drilled-mesh-tape/signedDistance/float/host

These probes compiled to the same code on both sides and were not run (24 cases): PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 344147cc
base-fingerprint: 1e0f461958307254a04eb0a9ea2890c216b99160
head: b42803d5
head-fingerprint: 480061955ebed0a11d3b2660afa7564b0241908b
cases: 386
rounds: 2
reps: 3
date: 2026-11-26
-->

## Device comparison (CUDA): green

**Accepted:** -

`b42803d5` against `344147cc` (origin/main), 2026-11-26.

**VERDICT: no regression (0 major, 0 minor across 386 cases)**

Performance comparison: 0 major, 0 minor, 3 faster, 0 differ, 0 lost, 386 cases

- base: 344147cc (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: b42803d5 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.61 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 22.6x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.42 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 18.5x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.06 ms | +0.6% | [-2.3%, +3.5%] | - | ~ | device 19.4x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.30 ms | -0.5% | [-1.2%, +0.3%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.27 ms | 6.29 ms | +0.2% | [-0.4%, +0.9%] | - | ~ | device 7.1x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.8 ms | -0.6% | [-1.6%, +0.5%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.4 ms | 33.4 ms | +0.2% | [-0.6%, +1.0%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 57.1 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.6 ms | 20.5 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.8 ms | 48.1 ms | +0.6% | [+0.3%, +0.8%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 62.1 ms | 60.7 ms | -2.3% | [-4.0%, -0.6%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 110 ms | 109 ms | -1.0% | [-4.0%, +2.1%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.18 ms | 8.27 ms | +1.1% | [-0.5%, +2.6%] | +0.0% | ~ | device 5.1x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.88 ms | 9.87 ms | -0.1% | [-1.2%, +1.1%] | +0.0% | ~ | device 8.2x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 387 µs | 388 µs | +0.4% | [-0.0%, +0.8%] | - | ~ | device 15.1x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 845 µs | 843 µs | -0.2% | [-0.7%, +0.2%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.54 ms | +0.5% | [+0.2%, +0.9%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.64 ms | 4.66 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 585 µs | 584 µs | -0.3% | [-0.6%, +0.0%] | - | ~ | device 13.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.76 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.03 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 903 µs | 905 µs | +0.2% | [-0.3%, +0.7%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.23 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.46 ms | 9.47 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 852 µs | 849 µs | -0.4% | [-0.5%, -0.2%] | - | ~ | device 13.5x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.07 ms | 2.07 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 7.9x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.09 ms | 9.07 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.80 ms | 9.83 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 307 µs | 308 µs | +0.3% | [+0.2%, +0.4%] | - | ~ | device 11.8x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.35 ms | 1.35 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 6.0x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.43 ms | 5.43 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.6x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 521 ms | 518 ms | -0.5% | [-1.4%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.3 ms | 20.3 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 4.1x faster than the host |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.38 ms | 8.39 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 5.0x faster than the host (was 4.7x); surface/leaf 2.42x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.6 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 8.0x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.18 ms | 6.18 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 10.1x faster than the host; surface/leaf 2.53x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 147 ms | 143 ms | -2.2% | [-3.1%, -1.4%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.0 ms | 60.0 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.39x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 71.8 ms | 72.1 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 1.8x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.3x faster than the host; surface/leaf 2.58x |
| `Domain/particles/labels/float/hostdevice` | 4.06 ms | 4.06 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 3.4x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.74 ms | 2.75 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 2.6x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 114 µs | 114 µs | +0.0% | [-1.2%, +1.3%] | - | ~ | device 13.0x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.10 ms | 6.99 ms | -1.6% | [-2.1%, -1.2%] | - | ~ | device 2.0x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.72 ms | 3.52 ms | -5.5% | [-11.2%, +1.0%] | - | ~ | device 2.0x faster than the host (was 1.9x) |
| `Domain/particles/surfaceGroups/double/hostdevice` | 118 µs | 119 µs | +1.1% | [-1.3%, +3.6%] | - | ~ | device 12.4x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 13.2 ms | 13.1 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 3.4x faster than the host |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.88 ms | 8.89 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.6x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 267 µs | 267 µs | +0.1% | [-0.4%, +0.6%] | - | ~ | device 2.6x faster than the host (was 2.4x); whole/own 49.01x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 158 µs | 158 µs | -0.3% | [-1.3%, +0.8%] | - | ~ | device 2.4x faster than the host (was 2.1x); whole/own 56.24x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 19.5 ms | 19.5 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 11.4 ms | 11.4 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.1x faster than the host |
| `DomainObject/partsOwn/labels/double/hostdevice` | 401 µs | 391 µs | -2.5% | [-6.3%, +1.5%] | - | ~ | device 1.8x faster than the host (was 1.7x); whole/own 49.85x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 238 µs | 236 µs | -0.9% | [-2.2%, +0.4%] | - | ~ | device 1.6x faster than the host; whole/own 48.48x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/hostdevice` | 10.4 ms | 10.5 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 10.0x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/hostdevice` | 10.4 ms | 10.3 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ | device 9.3x faster than the host; grid/morton 1.01x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/hostdevice` | 17.4 ms | 17.4 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ | device 7.7x faster than the host; grid/shuffled 0.60x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/hostdevice` | 224 ms | 225 ms | +0.5% | [-2.4%, +3.4%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/hostdevice` | 166 ms | 165 ms | -0.2% | [-2.6%, +2.2%] | +0.0% | ~ | device 0.6x faster than the host (was 0.7x); grid/morton 1.36x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/hostdevice` | 287 ms | 287 ms | +0.0% | [-2.8%, +3.0%] | +0.0% | ~ | device 0.4x faster than the host; grid/shuffled 0.78x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/hostdevice` | 27.2 ms | 27.1 ms | -0.1% | [-1.2%, +1.2%] | +0.0% | ~ | device 4.0x faster than the host |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/hostdevice` | 29.4 ms | 29.1 ms | -0.7% | [-1.2%, -0.3%] | +0.0% | ~ | device 3.4x faster than the host; grid/morton 0.93x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/hostdevice` | 60.5 ms | 60.5 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 2.5x faster than the host (was 3.1x); grid/shuffled 0.45x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/hostdevice` | 203 ms | 204 ms | +0.9% | [-0.2%, +2.0%] | +0.0% | ~ | device 0.5x faster than the host (was 0.6x) |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/hostdevice` | 158 ms | 162 ms | +2.4% | [+0.3%, +4.5%] | +0.0% | ~ | device 0.6x faster than the host; grid/morton 1.26x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/hostdevice` | 248 ms | 250 ms | +0.8% | [+0.6%, +1.0%] | +0.0% | ~ | device 0.6x faster than the host; grid/shuffled 0.82x |
| `Fused/csg/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.2% | [-0.5%, +0.0%] | - | ~ | device 23.7x faster than the host; tape/fused 4.56x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.6 µs | 49.7 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 44.1x faster than the host; tape/fused 7.23x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.7 µs | 47.9 µs | +0.4% | [-0.3%, +1.1%] | - | ~ | device 9.0x faster than the host (was 9.9x); tape/fused 9.97x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.4 ms | 17.4 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 6.7x faster than the host; tape/fused 1.79x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.2 ms | 31.2 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 4.0x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 809 µs | -0.1% | [-0.1%, -0.1%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 285 ms | 286 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 0.4x faster than the host; tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 287 ms | 285 ms | -0.7% | [-0.9%, -0.5%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.20 ms | 8.31 ms | +1.4% | [+0.2%, +2.7%] | +0.0% | ~ | device 11.9x faster than the host (was 11.3x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.28 ms | 8.22 ms | -0.7% | [-1.7%, +0.3%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.19 ms | 8.25 ms | +0.8% | [-0.1%, +1.7%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.26 ms | 8.40 ms | +1.7% | [+0.1%, +3.3%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 705 µs | 708 µs | +0.3% | [+0.3%, +0.4%] | +0.0% | ~ | device 3.7x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.78 ms | 1.78 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.02 ms | 1.02 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.69x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 799 µs | 799 µs | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 3.3x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 739 µs | 740 µs | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 824 µs | 823 µs | -0.1% | [-1.0%, +0.8%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 742 µs | 748 µs | +0.9% | [+0.5%, +1.4%] | +0.0% | ~ | device 11.9x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 728 µs | 731 µs | +0.3% | [-0.1%, +0.8%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 161 ms | 163 ms | +0.6% | [-2.7%, +4.2%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 161 ms | 159 ms | -1.2% | [-3.6%, +1.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 161 ms | 163 ms | +1.4% | [+0.1%, +2.7%] | +0.0% | ~ | device 0.5x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 162 ms | 161 ms | -0.7% | [-2.8%, +1.4%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.45 ms | 1.45 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 1.8x faster than the host; default/this 0.96x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.93 ms | 1.92 ms | -0.6% | [-0.9%, -0.3%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.72x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.56 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.24 ms | 7.30 ms | +0.9% | [+0.2%, +1.5%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.30 ms | 7.30 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.27 ms | 7.23 ms | -0.5% | [-0.7%, -0.2%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.30 ms | 7.31 ms | +0.1% | [-0.5%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.46 ms | 8.43 ms | -0.3% | [-1.1%, +0.5%] | - | ~ | device 11.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.7 ms | +0.1% | [-0.8%, +0.9%] | - | ~ | device 7.1x faster than the host (was 7.9x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 166 ms | 169 ms | +1.8% | [+0.2%, +3.5%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 284 ms | 285 ms | +0.4% | [-1.4%, +2.2%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.8 ms | 24.5 ms | -1.2% | [-2.1%, -0.3%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.1 ms | 57.9 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 2.5x faster than the host (was 2.6x) |
| `MeshSDF/query-morton/double/hostdevice` | 155 ms | 156 ms | +1.0% | [-0.1%, +2.1%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 244 ms | 242 ms | -0.8% | [-1.6%, +0.0%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.05 ms | 2.05 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 5.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.44 ms | 3.45 ms | +0.1% | [-0.4%, +0.7%] | - | ~ | device 4.0x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.15 ms | 6.15 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 4.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 64.9 ms | 64.8 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 3.1x faster than the host (was 3.4x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 95.7 ms | 96.0 ms | +0.3% | [-1.3%, +1.9%] | - | ~ | device 2.4x faster than the host (was 2.3x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 244 ms | 237 ms | -2.8% | [-3.4%, -2.3%] | - | ~ | device 3.6x faster than the host (was 3.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.6 ms | 25.0 ms | +1.5% | [+0.1%, +3.0%] | - | ~ | device 2.0x faster than the host; tape/fused 2.60x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.6 ms | 27.0 ms | +1.4% | [+0.6%, +2.1%] | - | ~ | device 2.5x faster than the host; tape/fused 3.56x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.9 ms | 65.5 ms | -0.7% | [-1.0%, -0.4%] | - | ~ | device 4.7x faster than the host (was 4.0x); tape/fused 3.62x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 20.8 ms | 21.4 ms | +3.0% | [-0.8%, +7.0%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 39.0 ms | 38.9 ms | -0.5% | [-1.6%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 42.6 ms | 41.8 ms | -1.8% | [-3.7%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 531 ms | 535 ms | +0.6% | [-4.6%, +6.1%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 859 ms | 838 ms | -2.5% | [-3.3%, -1.7%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.95 s | 1.92 s | -1.4% | [-2.9%, +0.1%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 199 ms | 205 ms | +3.0% | [-0.3%, +6.3%] | - | ~ | device 0.2x faster than the host; tape/fused 2.61x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 303 ms | 311 ms | +2.7% | [-1.2%, +6.8%] | - | ~ | device 0.2x faster than the host; tape/fused 2.69x (was 2.84x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 544 ms | 535 ms | -1.6% | [-3.8%, +0.7%] | - | ~ | device 0.5x faster than the host; tape/fused 3.59x |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 12.0 ms | 12.0 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 10.9x faster than the host (was 9.9x) |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 58.6 ms | 58.5 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 2.5x faster than the host |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 262 ms | 261 ms | -0.2% | [-2.9%, +2.5%] | - | ~ | device 0.5x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 241 ms | 241 ms | -0.3% | [-1.9%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 15.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.32 ms | 5.32 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 10.4x faster than the host (was 10.8x) |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.31 ms | 1.31 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 15.7x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.32 ms | 1.32 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 15.7x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.16 ms | 2.15 ms | -0.2% | [-1.1%, +0.7%] | - | ~ | device 7.8x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.52 ms | 3.53 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 10.2x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.77 ms | 2.76 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 7.8x faster than the host; mesh/tape 0.48x |
| `MeshTape/tape/frame/float/hostdevice` | 4.41 ms | 4.42 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 5.0x faster than the host; mesh/tape 0.49x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.73 ms | 3.74 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 9.7x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.63 ms | 5.66 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 4.2x faster than the host; mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 30.3 ms | 30.1 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 30.2 ms | 30.4 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 26.0 ms | 25.9 ms | -0.6% | [-2.2%, +1.0%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 43.4 ms | 43.3 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 30.4 ms | 30.6 ms | +0.5% | [-0.1%, +1.0%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 33.0 ms | 32.8 ms | -0.5% | [-0.7%, -0.4%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.79x |
| `MeshTape/view/firstHit/double/hostdevice` | 43.3 ms | 43.2 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 0.9x faster than the host; mesh/view 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 36.0 ms | 36.1 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.7x faster than the host; mesh/view 0.72x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 680 µs | 684 µs | +0.7% | [-0.1%, +1.4%] | - | ~ | device 11.0x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.83 ms | 1.83 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 997 µs | 995 µs | -0.2% | [-0.4%, -0.0%] | - | ~ | device 9.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.61 ms | 4.59 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.96 ms | 2.96 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.07 ms | 7.11 ms | +0.6% | [+0.2%, +0.9%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.79 ms | 7.79 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.48 ms | 1.46 ms | -1.1% | [-4.4%, +2.3%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.86 ms | 3.90 ms | +1.0% | [+0.5%, +1.5%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 523 µs | 525 µs | +0.4% | [-0.1%, +0.8%] | - | ~ | device 15.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.55 ms | 4.47 ms | -1.7% | [-2.0%, -1.3%] | - | ~ | device 6.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.21 ms | 4.25 ms | +1.1% | [+0.6%, +1.5%] | - | ~ | device 5.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.03 ms | 8.15 ms | +1.6% | [+1.3%, +1.9%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.7 ms | 15.8 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.02 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.5 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.8 ms | 15.0 ms | +1.4% | [-0.6%, +3.3%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.57 ms | -0.9% | [-1.7%, -0.0%] | - | ~ | device 10.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.61 ms | 4.61 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 833 µs | 832 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.86 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 7.1x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.13 ms | 4.13 ms | -0.0% | [-0.8%, +0.7%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.5 ms | 16.6 ms | +0.4% | [-0.0%, +0.9%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 21.0 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.94 ms | 8.94 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.5 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.3 ms | 20.2 ms | -0.7% | [-0.8%, -0.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.73 ms | 2.71 ms | -0.7% | [-1.7%, +0.3%] | - | ~ | device 4.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.74 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.72 ms | 2.71 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 7.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.39 ms | 6.42 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.9 ms | 34.0 ms | +0.4% | [-0.3%, +1.1%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.7 ms | +0.4% | [+0.2%, +0.5%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.4 ms | -0.1% | [-1.0%, +0.8%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.8 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.2 ms | 38.1 ms | -0.2% | [-0.8%, +0.5%] | - | ~ | device 0.5x faster than the host |
| `QueryOrder/random/mortonOrder/float/hostdevice` | 16.9 ms | 16.9 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 3.5x faster than the host |
| `QueryOrder/armadillo/distance-arrived/float/hostdevice` | 9.13 ms | 9.10 ms | -0.4% | [-1.1%, +0.4%] | +0.0% | ~ | device 5.0x faster than the host |
| `QueryOrder/armadillo/distance-ordered/float/hostdevice` | 7.38 ms | 7.38 ms | +0.0% | [-1.5%, +1.6%] | +0.0% | ~ | device 5.4x faster than the host; arrived/ordered 1.23x |
| `QueryOrder/random/mortonOrder/double/hostdevice` | 17.6 ms | 17.6 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 3.4x faster than the host |
| `QueryOrder/armadillo/distance-arrived/double/hostdevice` | 112 ms | 113 ms | +1.2% | [+0.5%, +1.9%] | +0.0% | ~ | device 0.4x faster than the host |
| `QueryOrder/armadillo/distance-ordered/double/hostdevice` | 80.1 ms | 81.4 ms | +1.6% | [+0.5%, +2.6%] | +0.0% | ~ | device 0.5x faster than the host; arrived/ordered 1.39x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 705 µs | 704 µs | -0.2% | [-1.1%, +0.8%] | - | ~ | device 17.3x faster than the host; hand/api 0.93x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.32 ms | 1.31 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 15.2x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 656 µs | 657 µs | +0.2% | [-0.6%, +1.0%] | - | ~ | device 17.3x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.24 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 15.4x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 633 µs | 623 µs | -1.5% | [-2.9%, -0.0%] | - | ~ | device 15.8x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.16 ms | 1.16 ms | +0.6% | [-0.2%, +1.4%] | - | ~ | device 14.4x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.20 ms | 1.19 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 11.9x faster than the host; triangles/polygons 0.59x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.49 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 9.6x faster than the host; triangles/polygons 0.53x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | device 11.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 613 µs | 612 µs | -0.2% | [-5.2%, +5.0%] | - | ~ | device 2.5x faster than the host; tape/fused 1.16x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 662 µs | 693 µs | +4.7% | [-0.2%, +9.7%] | - | ~ | device 2.6x faster than the host (was 2.7x); tape/fused 1.23x (was 1.29x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 708 µs | 709 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 854 µs | 854 µs | -0.1% | [-0.8%, +0.7%] | - | ~ | device 3.5x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 18.7 ms | 18.7 ms | -0.2% | [-1.3%, +0.8%] | - | ~ | device 0.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 30.3 ms | 30.4 ms | +0.2% | [-0.3%, +0.8%] | - | ~ | device 0.7x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 18.1 ms | 18.0 ms | -0.1% | [-1.0%, +0.8%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 29.1 ms | 29.2 ms | +0.4% | [+0.2%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.8 ms | 15.7 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 25.9 ms | 25.9 ms | -0.0% | [-0.8%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 17.2 ms | 17.2 ms | -0.0% | [-0.6%, +0.6%] | - | ~ | device 0.8x faster than the host; triangles/polygons 1.09x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.5 ms | 25.5 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.19x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.1 ms | 15.0 ms | -0.6% | [-2.0%, +0.9%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 23.0 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.24 ms | 1.25 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 1.3x faster than the host; tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.36 ms | 2.37 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.53 ms | 2.53 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.6x faster than the host; marched/traced 3.55x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.98 ms | 2.98 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.4x faster than the host; marched/traced 4.65x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.01 ms | 9.01 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 5.0x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.9 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.16 ms | 2.16 ms | +0.3% | [+0.0%, +0.5%] | - | ~ | device 3.9x faster than the host; marched/traced 13.38x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.43 ms | 5.43 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.8x faster than the host; marched/traced 10.15x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 28.9 ms | 28.9 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.1 ms | 55.1 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.1x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.23 ms | 5.23 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.82 ms | 8.84 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.18 ms | 6.14 ms | -0.7% | [-1.1%, -0.3%] | - | ~ | device 1.2x faster than the host (was 1.3x); marched/traced 26.56x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 0.8x faster than the host (was 0.8x); marched/traced 24.41x (was 23.64x) |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 163 ms | +2.2% | [+0.9%, +3.6%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 250 ms | 258 ms | +2.9% | [+1.7%, +4.1%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.87 ms | 4.87 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 22.23x (was 23.17x) |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.3 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 1.3x faster than the host; marched/traced 18.02x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 113 ms | 108 ms | -4.2% | [-5.0%, -3.3%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 215 ms | 221 ms | +2.7% | [+2.1%, +3.2%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.4 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.8 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 711 µs | 709 µs | -0.3% | [-0.8%, +0.1%] | - | ~ | device 17.1x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.31 ms | 1.32 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 15.2x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 706 µs | 707 µs | +0.1% | [-0.8%, +1.1%] | - | ~ | device 17.2x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.31 ms | 1.32 ms | +0.6% | [+0.1%, +1.0%] | - | ~ | device 15.2x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 711 µs | 706 µs | -0.6% | [-0.9%, -0.2%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 849 µs | 853 µs | +0.5% | [-0.5%, +1.5%] | - | ~ | device 3.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 601 µs | 601 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.3x faster than the host; hit/crossing 1.18x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 626 µs | 626 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 3.7x faster than the host; hit/crossing 1.36x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.77 ms | 4.78 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 10.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.3 ms | 10.3 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 6.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.23 ms | 4.24 ms | +0.3% | [+0.2%, +0.3%] | - | ~ | device 9.9x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.16 ms | 9.16 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.8 ms | 56.7 ms | -0.2% | [-1.4%, +1.1%] | - | ~ | device 3.1x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.5 ms | 82.5 ms | +1.2% | [+0.2%, +2.3%] | - | ~ | device 2.5x faster than the host (was 2.3x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.2 ms | 48.2 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 3.0x faster than the host; hit/crossing 1.18x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.7 ms | 63.5 ms | -0.4% | [-1.1%, +0.4%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.30x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 18.7 ms | 18.6 ms | -0.5% | [-1.0%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 30.3 ms | 30.2 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 18.8 ms | 18.7 ms | -0.6% | [-1.3%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 30.2 ms | 30.2 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.50 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.10 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.8x faster than the host; hit/crossing 1.26x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 68.7 ms | 68.6 ms | -0.2% | [-0.7%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 130 ms | 130 ms | -0.2% | [-1.5%, +1.0%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 62.4 ms | 62.4 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 117 ms | 119 ms | +1.6% | [+0.9%, +2.2%] | - | ~ | device 0.5x faster than the host; hit/crossing 1.09x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 449 ms | 465 ms | +3.5% | [-1.2%, +8.4%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 717 ms | 710 ms | -1.0% | [-1.5%, -0.6%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 416 ms | 393 ms | -5.4% | [-7.7%, -3.2%] | - | faster | device 0.5x faster than the host (was 0.5x); hit/crossing 1.18x (was 1.08x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 532 ms | 525 ms | -1.3% | [-1.9%, -0.6%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.35x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.58 ms | 3.59 ms | +0.3% | [-2.7%, +3.3%] | - | ~ | device 1.5x faster than the host; marched/traced 3.33x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.77 ms | 4.77 ms | -0.0% | [-1.0%, +1.0%] | - | ~ | device 1.3x faster than the host (was 1.2x); marched/traced 5.56x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 5.2x faster than the host (was 5.5x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.5 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.25 ms | 1.24 ms | -0.3% | [-2.2%, +1.7%] | - | ~ | device 3.0x faster than the host (was 2.6x) |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.83 ms | -0.9% | [-1.4%, -0.4%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 99.3 µs | 96.9 µs | -2.4% | [-5.8%, +1.2%] | - | ~ | device 15.4x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 154 µs | 153 µs | -1.1% | [-2.6%, +0.4%] | - | ~ | device 11.1x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 798 µs | 798 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 933 µs | 934 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 36.2 ms | 35.3 ms | -2.6% | [-7.3%, +2.0%] | - | ~ | device 0.1x faster than the host (was 0.2x); marched/traced 3.44x (was 3.33x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.3 ms | 36.4 ms | +0.3% | [-1.2%, +1.7%] | - | ~ | device 0.2x faster than the host; marched/traced 6.47x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 121 ms | 121 ms | +0.3% | [-2.1%, +2.9%] | - | ~ | device 0.5x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 235 ms | 236 ms | +0.3% | [-1.3%, +2.0%] | - | ~ | device 0.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.7% | [-7.5%, +6.4%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.7 ms | 22.1 ms | +1.8% | [+0.2%, +3.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.13 ms | 1.14 ms | +0.5% | [-3.6%, +3.9%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.49 ms | 2.51 ms | +0.9% | [-0.1%, +1.8%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 6.02 ms | +0.6% | [-0.1%, +1.3%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.73 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.4 ms | 23.4 ms | -0.0% | [-1.4%, +1.4%] | - | ~ | device 2.1x faster than the host (was 2.9x); marched/traced 2.65x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.3 ms | -0.0% | [-1.6%, +1.6%] | - | ~ | device 1.9x faster than the host; marched/traced 3.70x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.3 ms | 62.0 ms | -0.4% | [-0.8%, -0.1%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 104 ms | 105 ms | +0.9% | [-0.2%, +2.0%] | - | ~ | device 2.5x faster than the host (was 2.3x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.8 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.5 ms | +0.3% | [-0.5%, +1.1%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 309 ms | 311 ms | +0.4% | [-2.2%, +3.2%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 1.84x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 351 ms | 355 ms | +1.0% | [-1.5%, +3.6%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 2.80x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 565 ms | 570 ms | +1.0% | [-1.2%, +3.3%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 999 ms | 992 ms | -0.7% | [-2.2%, +0.8%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 195 ms | 186 ms | -4.2% | [-6.2%, -2.2%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 238 ms | 240 ms | +1.1% | [-2.1%, +4.0%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.98 ms | 2.99 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 3.4x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.68 ms | 3.65 ms | -0.6% | [-0.7%, -0.6%] | - | ~ | device 4.9x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 97.4 ms | 97.1 ms | -0.3% | [-1.3%, +0.7%] | - | ~ | device 4.0x faster than the host; one-pass/each 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.72 ms | 3.73 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 5.0x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.77 ms | 6.78 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 5.1x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.63 ms | 5.61 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 4.2x faster than the host; regions/hit 1.21x |
| `Regions/particles/region/double/hostdevice` | 3.42 ms | 3.40 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 3.2x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.80 ms | 5.80 ms | -0.0% | [-0.8%, +0.7%] | - | ~ | device 3.4x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 119 ms | 119 ms | +0.6% | [+0.0%, +1.2%] | - | ~ | device 3.6x faster than the host (was 3.2x); one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.93 ms | 5.90 ms | -0.4% | [-1.2%, +0.4%] | - | ~ | device 2.8x faster than the host (was 3.0x) |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.8 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 1.9x faster than the host (was 2.0x) |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 12.9 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 1.7x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.37 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.36 ms | 2.36 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.8x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.15 ms | 2.14 ms | -0.3% | [-1.1%, +0.5%] | - | ~ | device 9.4x faster than the host (was 9.0x) |
| `Render/csg/frame/double/hostdevice` | 5.61 ms | 5.62 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 1.7x faster than the host (was 1.7x) |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.63 ms | 5.64 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 25.7 ms | 25.7 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 890 µs | 896 µs | +0.7% | [+0.3%, +1.0%] | +0.0% | ~ | device 11.0x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.65 ms | 2.65 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ | device 5.0x faster than the host (was 5.2x) |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.6 ms | 13.5 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.1 ms | 10.2 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.3 ms | 14.4 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.1 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.00 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 7.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.76 ms | 4.78 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ | device 5.1x faster than the host (was 5.9x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ | device 6.0x faster than the host; tape/fused 1.38x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.0 ms | +0.0% | [-0.0%, +0.0%] | +0.0% | ~ | device 5.5x faster than the host (was 4.0x); tape/fused 1.27x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 23.0 ms | 23.0 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.9x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.6 ms | 30.6 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 3.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.1 ms | 44.2 ms | +0.3% | [-0.5%, +1.2%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.8 ms | 45.6 ms | -0.5% | [-1.8%, +0.8%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 121 ms | 124 ms | +2.1% | [-2.1%, +3.1%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 125 ms | 128 ms | +1.7% | [-0.9%, +4.5%] | +0.0% | ~ | device 0.8x faster than the host (was 0.9x); tape/fused 1.04x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 126 ms | 129 ms | +2.3% | [+0.1%, +4.5%] | +0.0% | ~ | device 0.9x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 132 ms | 133 ms | +0.7% | [-0.5%, +2.0%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | device 14.4x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.1 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ | device 0.8x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.30 ms | 9.30 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 15.6x faster than the host |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.92 ms | 5.91 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 14.4x faster than the host (was 13.8x); surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 97.2 ms | 97.6 ms | +0.4% | [-1.1%, +1.9%] | - | ~ | device 4.3x faster than the host (was 4.5x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.9 ms | 63.9 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 4.2x faster than the host; surface/leaf 1.53x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.7 ms | 29.8 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 7.1x faster than the host (was 6.6x) |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 10.0 ms | 9.99 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 9.5x faster than the host; surface/hit 2.98x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.1 ms | 59.0 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.2x faster than the host (was 1.9x); surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 784 ms | 783 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 0.6x faster than the host (was 0.5x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 523 ms | 526 ms | +0.5% | [-0.1%, +1.0%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.49x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 313 ms | 317 ms | +1.1% | [-0.4%, +2.6%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 128 ms | 128 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 0.6x faster than the host; surface/hit 2.46x |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 218 µs | +0.2% | [+0.1%, +0.2%] | - | ~ | device 11.4x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.2% | [-0.6%, +0.1%] | - | ~ | device 24.6x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 359 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 12.8x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 9.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.4 µs | 72.2 µs | -0.3% | [-0.5%, -0.1%] | - | ~ | device 7.3x faster than the host (was 7.0x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 47.9 µs | +0.1% | [-1.5%, +1.6%] | - | ~ | device 4.6x faster than the host (was 5.3x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 651 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 10.8x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 11.9x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 477 µs | 477 µs | +0.1% | [-0.1%, +0.2%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.7 µs | 48.0 µs | -1.4% | [-2.9%, +0.1%] | - | ~ | device 27.3x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 859 µs | 858 µs | -0.1% | [-0.2%, -0.1%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.65 ms | 6.64 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 191 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 3.0x faster than the host (was 2.8x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.2% | [-0.2%, -0.2%] | - | ~ | device 1.9x faster than the host (was 2.0x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.12 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 8.2x faster than the host (was 8.5x) |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.71 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 11.1x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.50 ms | 6.51 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.5x faster than the host (was 2.9x) |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 5013.9 s → 5047.3 s (+0.7%), the largest peak 5452 MB → 5409 MB; 14 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.8 s | 21.1 s | +52.5% | 619 MB | 628 MB | +1.5% | **MAJOR** |
| `BVHRefit` | 7.4 s | 9.8 s | +32.5% | 523 MB | 528 MB | +0.9% | **MAJOR** |
| `BVHUnion` | 26.0 s | 35.8 s | +37.6% | 1042 MB | 1051 MB | +0.9% | **MAJOR** |
| `BVHUnionMixed` | 12.2 s | 16.3 s | +34.5% | 636 MB | 646 MB | +1.5% | **MAJOR** |
| `ClosestPrimitive` | 109.1 s | 136.0 s | +24.8% | 1371 MB | 1380 MB | +0.6% | **minor** |
| `ClosestSurface` | 142.2 s | 177.6 s | +24.9% | 1316 MB | 1316 MB | +0.0% | **minor** |
| `ConnectedComponents` | 20.7 s | 26.3 s | +27.4% | 951 MB | 960 MB | +1.0% | **MAJOR** |
| `Domain` | 52.4 s | 66.8 s | +27.6% | 1006 MB | 1016 MB | +0.9% | **MAJOR** |
| `DomainObject` | 51.5 s | 68.9 s | +33.7% | 959 MB | 968 MB | +0.9% | **MAJOR** |
| `EBGrid` | 22.1 s | 23.7 s | +7.2% | 766 MB | 776 MB | +1.2% | ~ |
| `Fused` | 52.2 s | 65.4 s | +25.3% | 1178 MB | 1187 MB | +0.7% | **MAJOR** |
| `LaunchTuning` | 485.3 s | 492.8 s | +1.5% | 3133 MB | 3133 MB | +0.0% | ~ |
| `Mesh` | 23.0 s | 31.3 s | +35.9% | 824 MB | 833 MB | +1.1% | **MAJOR** |
| `MeshCSG` | 379.1 s | 399.5 s | +5.4% | 5192 MB | 5193 MB | +0.0% | ~ |
| `MeshClosestPoint` | 20.1 s | 22.9 s | +13.7% | 776 MB | 785 MB | +1.1% | **minor** |
| `MeshIngest` | 10.9 s | 13.2 s | +21.3% | 549 MB | 558 MB | +1.7% | **minor** |
| `MeshReconcile` | 11.1 s | 14.1 s | +26.2% | 526 MB | 535 MB | +1.7% | **MAJOR** |
| `MeshTape` | 700.4 s | 677.1 s | -3.3% | 5452 MB | 5409 MB | -0.8% | ~ |
| `PointCloud` | 13.2 s | 13.7 s | +3.6% | 578 MB | 587 MB | +1.5% | ~ |
| `PointCloudBVHBuild` | 8.1 s | 8.3 s | +1.9% | 523 MB | 528 MB | +0.9% | ~ |
| `PointCloudHashGridBuild` | 6.8 s | 8.4 s | +24.7% | 523 MB | 528 MB | +0.9% | ~ |
| `QueryOrder` | 16.5 s | 16.7 s | +1.2% | 707 MB | 717 MB | +1.3% | ~ |
| `Ray` | 169.2 s | 180.5 s | +6.7% | 3030 MB | 3029 MB | -0.0% | ~ |
| `RayCSG` | 204.3 s | 224.3 s | +9.8% | 3368 MB | 3368 MB | +0.0% | ~ |
| `RayCrossing` | 416.6 s | 401.0 s | -3.7% | 3299 MB | 3300 MB | +0.0% | ~ |
| `RayParticles` | 469.7 s | 435.7 s | -7.2% | 4847 MB | 4847 MB | +0.0% | ~ |
| `RayShell` | 248.8 s | 253.3 s | +1.8% | 4603 MB | 4603 MB | -0.0% | ~ |
| `Regions` | 349.4 s | 321.0 s | -8.1% | 3363 MB | 3364 MB | +0.0% | ~ |
| `Render` | 266.9 s | 241.7 s | -9.4% | 4920 MB | 4920 MB | +0.0% | ~ |
| `Shapes` | 28.9 s | 27.8 s | -3.7% | 599 MB | 608 MB | +1.6% | ~ |
| `SphereCloudBVH` | 10.5 s | 9.3 s | -11.7% | 524 MB | 528 MB | +0.9% | ~ |
| `SphereCloudRays` | 185.0 s | 160.6 s | -13.2% | 3332 MB | 3331 MB | -0.0% | ~ |
| `SphereContacts` | 9.2 s | 9.0 s | -2.6% | 523 MB | 528 MB | +1.0% | ~ |
| `SurfaceParts` | 369.4 s | 336.5 s | -8.9% | 3493 MB | 3493 MB | +0.0% | ~ |
| `Tape` | 60.2 s | 58.3 s | -3.1% | 992 MB | 1001 MB | +1.0% | ~ |
| `TapeBVHUnion` | 41.8 s | 42.5 s | +1.6% | 932 MB | 941 MB | +0.9% | ~ |
| **all, in both** | 5013.9 s | 5047.3 s | +0.7% | 5452 MB | 5409 MB | -0.8% | ~ |

Device kernels: 226 in both, 2 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, RegionView<Tape<double, TriMeshSDF<…` | 168 → 168 | 20400 → 20464 | 2788/2348 → 2788/2348 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, RegionView<Tape<float, TriMeshSDF<fl…` | 168 → 168 | 14768 → 14832 | 2904/2136 → 2904/2136 | ~ |


Flagged after 2 rounds, and judged on 10 (3 cases): RayParticles/armadillo/step-morton/double/hostdevice, RayShell/armadillo-fused/firstHit-random/double/hostdevice, SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice

These probes compiled to the same code on both sides and were not run (28 cases): PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Shapes (20)

<!-- END receipt cuda -->
