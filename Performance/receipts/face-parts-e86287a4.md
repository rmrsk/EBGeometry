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
head: 8e2b3266
head-fingerprint: e86287a4b012022f3f191214014da3cfa96437ea
cases: 308
rounds: 2
reps: 3
date: 2026-11-26
-->

## Host comparison: green

**Accepted:** -

`8e2b3266` against `344147cc` (344147cc), 2026-11-26.

**VERDICT: no regression (0 major, 0 minor across 308 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 308 cases

- base: 344147cc (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8e2b3266 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 331 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 177 ms | 184 ms | +3.4% | [-0.0%, +6.9%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 514 ms | 513 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 348 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 187 ms | +0.6% | [-0.2%, +1.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 563 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.7 ms | 41.7 ms | -0.0% | [-1.1%, +1.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 76.4 ms | 75.8 ms | -0.8% | [-1.4%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.3 ms | 55.9 ms | -0.7% | [-1.2%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.2 ms | 51.3 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.3 ms | 64.2 ms | -0.1% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.8 ms | 55.4 ms | +1.2% | [+0.1%, +2.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.6 ms | 60.6 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.1 ms | 72.2 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 72.2 ms | 71.9 ms | -0.5% | [-1.0%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.4 ms | 77.4 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.4 ms | 64.7 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.7 ms | 100 ms | +0.5% | [-0.6%, +1.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 76.7 ms | 77.2 ms | +0.6% | [-0.2%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 109 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 63.6 ms | 63.6 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 95.8 ms | 96.1 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 76.0 ms | 76.1 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 101 ms | 101 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.5 ms | +0.4% | [-0.6%, +1.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.6 ms | 41.2 ms | -1.0% | [-2.3%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.9 ms | 72.9 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.4 ms | 20.0 ms | -2.1% | [-3.2%, -1.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.9 ms | 43.5 ms | -1.0% | [-1.7%, -0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.3 ms | 80.8 ms | -0.6% | [-0.9%, -0.3%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 139 ms | 139 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 76.7 ms | 76.5 ms | -0.3% | [-1.5%, +1.0%] | +0.0% | ~ | surface/leaf 1.82x |
| `ClosestSurface/union/closestSurface/float/host` | 146 ms | 146 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 76.5 ms | 76.5 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | surface/leaf 1.91x |
| `ClosestSurface/cloud/closestSurface/double/host` | 156 ms | 157 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 80.1 ms | 80.2 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | surface/leaf 1.96x |
| `ClosestSurface/union/closestSurface/double/host` | 154 ms | 155 ms | +1.1% | [-0.8%, +3.0%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 80.6 ms | 80.8 ms | +0.3% | [-0.0%, +0.7%] | +0.0% | ~ | surface/leaf 1.92x |
| `ConnectedComponents/shells/build-coarse/float/host` | 147 ms | 147 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/float/host` | 659 ms | 656 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-coarse/double/host` | 153 ms | 151 ms | -1.2% | [-1.4%, -1.0%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/double/host` | 685 ms | 675 ms | -1.5% | [-1.6%, -1.4%] | +0.0% | ~ |  |
| `Domain/particles/labels/float/host` | 13.5 ms | 13.6 ms | +1.0% | [+0.1%, +1.9%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.77 ms | 6.77 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.31 ms | 1.31 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.1 ms | 14.1 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.12 ms | 7.11 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.31 ms | 1.32 ms | +0.4% | [+0.2%, +0.7%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 40.1 ms | 39.5 ms | -1.5% | [-2.8%, -0.1%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.1 ms | 21.4 ms | +1.2% | [+0.4%, +2.0%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 563 µs | 565 µs | +0.3% | [-1.0%, +1.5%] | +0.0% | ~ | whole/own 69.93x |
| `DomainObject/partsOwn/labelAt/float/host` | 301 µs | 303 µs | +0.6% | [+0.3%, +1.0%] | +0.0% | ~ | whole/own 70.63x |
| `DomainObject/partsWhole/labels/double/host` | 39.4 ms | 39.4 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.6 ms | 22.5 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 580 µs | 582 µs | +0.4% | [-0.7%, +1.5%] | +0.0% | ~ | whole/own 67.75x |
| `DomainObject/partsOwn/labelAt/double/host` | 314 µs | 315 µs | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ | whole/own 71.36x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/host` | 63.5 ms | 63.4 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/host` | 59.2 ms | 59.2 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/host` | 77.4 ms | 77.3 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ | grid/shuffled 0.82x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/host` | 67.7 ms | 67.8 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/host` | 63.2 ms | 63.1 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/host` | 88.4 ms | 88.1 ms | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ | grid/shuffled 0.77x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/host` | 190 ms | 190 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/host` | 178 ms | 178 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/host` | 256 ms | 256 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | grid/shuffled 0.74x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/host` | 201 ms | 201 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/host` | 187 ms | 189 ms | +0.8% | [-0.2%, +1.8%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/host` | 290 ms | 291 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ | grid/shuffled 0.69x |
| `Fused/csg/signedDistance/float/host` | 4.56 ms | 4.50 ms | -1.3% | [-2.4%, -0.1%] | - | ~ | tape/fused 2.86x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.4 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | tape/fused 2.01x |
| `Fused/chain16/signedDistance/float/host` | 977 µs | 983 µs | +0.5% | [-0.6%, +1.8%] | - | ~ | tape/fused 16.60x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.0 ms | 46.1 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.4 ms | 46.2 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.57 ms | 4.59 ms | +0.5% | [-1.0%, +2.1%] | - | ~ | tape/fused 2.86x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | tape/fused 2.06x |
| `Fused/chain16/signedDistance/double/host` | 3.90 ms | 3.87 ms | -0.7% | [-2.7%, +1.4%] | - | ~ | tape/fused 4.23x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.4 ms | 52.4 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.0 ms | 52.9 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.7 ms | 82.7 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 228 ms | 228 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 114 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 261 ms | 262 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.9 ms | 70.9 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 214 ms | 213 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 97.0 ms | 96.8 ms | -0.2% | [-0.2%, -0.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 28.6 ms | 28.9 ms | +1.1% | [+0.1%, +2.1%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 758 ms | 760 ms | +0.2% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 42.8 ms | 43.8 ms | +2.4% | [+1.0%, +3.8%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 858 ms | 856 ms | -0.2% | [-1.6%, +1.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.1 ms | 26.2 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.3 ms | 15.5 ms | +1.5% | [+0.1%, +2.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 318 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 230 ms | 230 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.4 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | tape/fused 3.51x |
| `MeshCSG/smooth-fused/frame/float/host` | 81.3 ms | 81.2 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | tape/fused 2.84x |
| `MeshCSG/union-tape/firstHit/double/host` | 28.7 ms | 29.0 ms | +1.2% | [-0.1%, +2.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.4 ms | 17.2 ms | -1.4% | [-3.3%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 359 ms | 357 ms | -0.5% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 246 ms | 246 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.0 ms | 97.4 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ | tape/fused 3.67x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.3 ms | 87.3 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ | tape/fused 2.82x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 228 ms | 228 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 231 ms | 232 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | signedDistance/closestPoint 0.98x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 306 ms | 306 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 232 ms | 232 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | closestPoint/gradient 1.00x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 260 ms | 262 ms | +0.6% | [+0.0%, +1.2%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 264 ms | 263 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ | signedDistance/closestPoint 0.99x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 339 ms | 341 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 265 ms | 264 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | closestPoint/gradient 1.00x |
| `MeshIngest/armadillo/read-soup/float/host` | 53.4 ms | 53.1 ms | -0.5% | [-1.1%, +0.1%] | - | ~ |  |
| `MeshIngest/armadillo/clean/float/host` | 12.5 ms | 12.6 ms | +0.4% | [-1.2%, +2.2%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/float/host` | 95.7 ms | 95.6 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/float/host` | 102 ms | 103 ms | +0.4% | [-0.5%, +1.2%] | - | ~ |  |
| `MeshIngest/armadillo/read-soup/double/host` | 54.9 ms | 54.1 ms | -1.3% | [-1.9%, -0.7%] | - | ~ |  |
| `MeshIngest/armadillo/clean/double/host` | 13.6 ms | 13.5 ms | -1.4% | [-2.6%, -0.1%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/double/host` | 106 ms | 106 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/double/host` | 115 ms | 116 ms | +0.5% | [-0.0%, +1.0%] | - | ~ |  |
| `MeshReconcile/armadillo/reconcile/float/host` | 73.5 ms | 73.4 ms | -0.2% | [-1.3%, +0.9%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.2 ms | 41.3 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ | reconcile/scheduled 1.78x |
| `MeshReconcile/armadillo/reconcile/double/host` | 93.6 ms | 93.5 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.1 ms | 55.9 ms | -0.4% | [-0.9%, +0.0%] | +0.0% | ~ | reconcile/scheduled 1.67x |
| `MeshTape/mesh/firstHit/float/host` | 48.6 ms | 48.4 ms | -0.4% | [-0.8%, -0.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 48.6 ms | 48.5 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.9 ms | 19.5 ms | -2.1% | [-2.4%, -1.9%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 80.3 ms | 80.0 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ | mesh/tape 0.60x |
| `MeshTape/tape/firstCrossing/float/host` | 49.8 ms | 49.9 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ | mesh/tape 0.97x |
| `MeshTape/tape/frame/float/host` | 24.7 ms | 24.3 ms | -1.7% | [-4.6%, +1.4%] | +0.0% | ~ | mesh/tape 0.80x |
| `MeshTape/view/firstHit/float/host` | 83.2 ms | 83.3 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/float/host` | 26.1 ms | 25.9 ms | -0.7% | [-0.9%, -0.6%] | +0.0% | ~ | mesh/view 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 53.3 ms | 53.2 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 53.3 ms | 53.4 ms | +0.2% | [-0.6%, +0.9%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.6 ms | 20.4 ms | -1.0% | [-1.8%, -0.1%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 87.8 ms | 87.5 ms | -0.3% | [-1.4%, +0.8%] | +0.0% | ~ | mesh/tape 0.61x |
| `MeshTape/tape/firstCrossing/double/host` | 55.2 ms | 54.9 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ | mesh/tape 0.97x |
| `MeshTape/tape/frame/double/host` | 26.7 ms | 26.5 ms | -0.5% | [-1.3%, +0.4%] | +0.0% | ~ | mesh/tape 0.77x |
| `MeshTape/view/firstHit/double/host` | 90.1 ms | 89.8 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ | mesh/view 0.59x |
| `MeshTape/view/frame/double/host` | 29.1 ms | 29.1 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | mesh/view 0.70x |
| `PointCloudBVH/uniform/build/float/host` | 52.9 ms | 53.0 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 143 ms | +0.2% | [+0.2%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 361 ms | 362 ms | +0.4% | [+0.2%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 80.4 ms | 80.7 ms | +0.4% | [-0.0%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.74 ms | 6.75 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.3 ms | 55.2 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 157 ms | 157 ms | -0.3% | [-0.6%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 392 ms | 393 ms | +0.4% | [+0.2%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.1 ms | 87.2 ms | +0.2% | [-0.6%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.01 ms | 7.02 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.7 ms | 14.0 ms | +2.4% | [-1.8%, +6.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 117 ms | 118 ms | +0.8% | [+0.7%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 473 ms | 476 ms | +0.7% | [+0.6%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 141 ms | 142 ms | +0.3% | [+0.0%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 800 ms | 800 ms | -0.0% | [-0.1%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 15.0 ms | +0.9% | [+0.4%, +1.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 121 ms | 121 ms | -0.1% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 491 ms | 494 ms | +0.5% | [+0.2%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 144 ms | +0.1% | [-0.3%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 798 ms | 788 ms | -1.3% | [-1.6%, -1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.1 ms | 58.0 ms | +7.1% | [-0.4%, +14.7%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 133 ms | 130 ms | -1.9% | [-5.2%, +1.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 269 ms | 271 ms | +0.9% | [-0.4%, +2.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 149 ms | 150 ms | +0.4% | [+0.4%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 134 ms | 134 ms | +0.2% | [+0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 56.8 ms | 57.0 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 142 ms | 140 ms | -1.5% | [-2.8%, -0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 298 ms | 299 ms | +0.3% | [+0.1%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 163 ms | 163 ms | -0.1% | [-0.9%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 149 ms | 149 ms | +0.4% | [-1.3%, +2.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 12.1 ms | 11.8 ms | -2.4% | [-3.5%, -1.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 114 ms | 114 ms | +0.2% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 318 ms | 317 ms | -0.3% | [-0.6%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 155 ms | -0.3% | [-0.4%, -0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 320 ms | 320 ms | -0.0% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.6 ms | 13.6 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 115 ms | -0.1% | [-0.1%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 347 ms | 347 ms | +0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 165 ms | 164 ms | -0.2% | [-0.8%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 308 ms | 309 ms | +0.2% | [+0.0%, +0.4%] | - | ~ |  |
| `QueryOrder/random/mortonOrder/float/host` | 60.8 ms | 61.1 ms | +0.6% | [+0.3%, +0.9%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | 223 ms | 223 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | 160 ms | 160 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | arrived/ordered 1.40x |
| `QueryOrder/random/mortonOrder/double/host` | 62.3 ms | 62.4 ms | +0.2% | [+0.0%, +0.5%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | 263 ms | 263 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | 170 ms | 170 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ | arrived/ordered 1.54x |
| `Ray/mesh/firstHit/float/host` | 48.9 ms | 48.5 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ | hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled/float/host` | 46.0 ms | 45.7 ms | -0.6% | [-1.0%, -0.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.5 ms | 40.3 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.4 ms | 78.5 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | triangles/polygons 0.62x |
| `Ray/mesh-polygons/intersects/float/host` | 65.4 ms | 65.5 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.21 ms | 5.26 ms | +1.0% | [-0.1%, +2.2%] | - | ~ | tape/fused 1.45x |
| `Ray/csg-tape/firstHit/float/host` | 7.73 ms | 7.63 ms | -1.3% | [-3.1%, +0.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 53.1 ms | 53.2 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.3 ms | 51.6 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.0 ms | 45.0 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.5 ms | 89.0 ms | +0.6% | [+0.3%, +1.0%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.7 ms | 73.3 ms | -0.5% | [-1.6%, +0.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.52 ms | 5.49 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | tape/fused 1.46x |
| `Ray/csg-tape/firstHit/double/host` | 7.90 ms | 8.02 ms | +1.6% | [+0.6%, +2.6%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.5 ms | 16.9 ms | -3.5% | [-4.7%, -2.2%] | - | faster | marched/traced 7.04x (was 6.83x) |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 119 ms | -0.4% | [-0.5%, -0.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.8 ms | 22.0 ms | +0.9% | [-0.4%, +2.3%] | +0.0% | ~ | marched/traced 6.59x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 144 ms | 145 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.4 ms | 35.4 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 22.4 ms | 19.3 ms | -13.9% | [-25.5%, +1.8%] | - | ~ | marched/traced 6.61x (was 5.68x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 127 ms | 128 ms | +0.2% | [-1.5%, +2.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.7 ms | 23.7 ms | +0.2% | [-1.2%, +1.6%] | +0.0% | ~ | marched/traced 6.35x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 155 ms | 151 ms | -2.5% | [-5.0%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.8 ms | 37.9 ms | +0.3% | [-0.5%, +1.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 48.5 ms | 48.6 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 48.6 ms | 50.0 ms | +3.0% | [-0.5%, +6.5%] | +0.0% | ~ | hit/crossing 0.97x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.68 ms | 7.70 ms | +0.3% | [-2.5%, +3.2%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.16 ms | 6.11 ms | -0.8% | [-1.3%, -0.2%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 179 ms | 178 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 156 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ | hit/crossing 1.14x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.7 ms | 69.2 ms | +0.8% | [+0.6%, +1.0%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.4 ms | 56.4 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | hit/crossing 1.23x |
| `RayCrossing/mesh/firstHit/double/host` | 53.1 ms | 53.4 ms | +0.7% | [+0.1%, +1.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 53.3 ms | 53.4 ms | +0.3% | [-1.0%, +1.6%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.00 ms | 8.01 ms | +0.2% | [-2.4%, +2.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.32 ms | 6.33 ms | +0.1% | [-1.7%, +0.9%] | +0.0% | ~ | hit/crossing 1.27x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 201 ms | 201 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 179 ms | 179 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.3 ms | 77.1 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.3 ms | 64.1 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ | hit/crossing 1.20x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.2 ms | +1.4% | [+0.5%, +2.3%] | +0.0% | ~ | marched/traced 4.61x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 52.0 ms | 51.8 ms | -0.4% | [-1.4%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.01 ms | 7.99 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.99 ms | 1.99 ms | -0.0% | [-1.2%, +1.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.93 ms | 6.94 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.9 ms | 11.8 ms | -1.2% | [-2.1%, -0.4%] | +0.0% | ~ | marched/traced 5.20x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.2 ms | 61.2 ms | -0.0% | [-1.5%, +1.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.73 ms | 8.66 ms | -0.8% | [-1.9%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.03 ms | 2.06 ms | +1.2% | [+0.5%, +1.8%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.26 ms | 7.22 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 240 ms | 239 ms | -0.3% | [-1.1%, +0.6%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 330 ms | 327 ms | -0.7% | [-1.3%, +0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 236 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 253 ms | 253 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ | marched/traced 1.46x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 369 ms | 371 ms | +0.4% | [-0.6%, +1.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 253 ms | 257 ms | +1.4% | [-1.3%, +4.1%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.33 ms | 7.32 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.6 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 266 ms | 267 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.5 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.4 ms | 30.3 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.4 ms | 22.5 ms | +0.4% | [+0.4%, +0.4%] | +0.0% | ~ | regions/hit 1.35x |
| `Regions/particles/region/double/host` | 7.70 ms | 7.64 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.8 ms | 12.9 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 282 ms | 282 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.0 ms | 13.0 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.4 ms | 32.3 ms | -0.3% | [-1.1%, +0.6%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.7 ms | 23.8 ms | +0.6% | [-0.5%, +1.7%] | +0.0% | ~ | regions/hit 1.36x |
| `Render/csg/frame/float/host` | 12.6 ms | 13.2 ms | +5.0% | [-2.9%, +10.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 13.1 ms | 13.3 ms | +1.7% | [-5.9%, +10.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.5 ms | 19.6 ms | +0.6% | [+0.4%, +0.8%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.3 ms | 14.0 ms | -2.4% | [-2.6%, -2.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.0 ms | 13.8 ms | -1.3% | [-1.5%, -1.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.4 ms | 20.3 ms | -0.5% | [-0.6%, -0.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.2 ms | 56.1 ms | -0.3% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.4 ms | 36.2 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.1 ms | 92.1 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 63.2 ms | 63.4 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 41.9 ms | 41.8 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 107 ms | 107 ms | -0.1% | [-1.0%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.4 ms | 74.9 ms | +0.6% | [+0.4%, +0.8%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 294 ms | 297 ms | +1.0% | [-0.0%, +2.0%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 295 ms | 297 ms | +0.5% | [-0.0%, +1.0%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 86.3 ms | 86.5 ms | +0.3% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 346 ms | 345 ms | -0.4% | [-0.4%, -0.3%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 347 ms | 348 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 166 ms | 166 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 190 ms | 190 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 144 ms | 144 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 89.7 ms | 89.9 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 130 ms | 130 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 72.4 ms | 72.7 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ | surface/leaf 1.79x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 61.6 ms | 61.4 ms | -0.3% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.7 ms | 34.3 ms | -1.0% | [-2.3%, +0.4%] | +0.0% | ~ | surface/hit 1.79x |
| `SurfaceParts/boxes/closestSurface/double/host` | 147 ms | 145 ms | -1.1% | [-2.4%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 92.4 ms | 90.7 ms | -1.8% | [-3.5%, -0.1%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 145 ms | 144 ms | -0.2% | [-0.3%, +0.0%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 81.3 ms | 81.1 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 68.8 ms | 68.7 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 39.1 ms | 39.6 ms | +1.2% | [-0.3%, +2.6%] | +0.0% | ~ | surface/hit 1.74x |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.58 ms | 4.54 ms | -0.8% | [-1.3%, -0.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.9 ms | 25.0 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 39.8 ms | 39.8 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.74 ms | 1.76 ms | +1.1% | [+0.5%, +1.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 130 µs | 130 µs | +0.3% | [-7.0%, +7.9%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.8 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.67 ms | 5.68 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.3 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.65 ms | 4.65 ms | -0.1% | [-0.2%, +0.0%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.1 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.61 ms | 4.59 ms | -0.4% | [-1.6%, +0.8%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.2 ms | 26.3 ms | +0.3% | [-0.6%, +1.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.0 ms | 42.2 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.78 ms | 1.79 ms | +0.9% | [-1.2%, +2.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 294 µs | 282 µs | -4.0% | [-11.9%, +4.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.9 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.75 ms | 7.74 ms | -0.1% | [-0.6%, +0.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.4 ms | 16.4 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.82 ms | 7.77 ms | -0.6% | [-1.8%, +0.6%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.9 ms | 41.8 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 201 ms | 200 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 42.5 ms | 42.6 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 219 ms | 218 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 774.7 s → 766.2 s (-1.1%), the largest peak 1418 MB → 1446 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.9 s | 9.7 s | -2.5% | 563 MB | 584 MB | +3.7% | ~ |
| `BVHRefit` | 4.3 s | 4.4 s | +2.1% | 503 MB | 511 MB | +1.5% | ~ |
| `BVHUnion` | 19.6 s | 19.5 s | -0.6% | 956 MB | 946 MB | -1.0% | ~ |
| `BVHUnionMixed` | 7.7 s | 7.7 s | -0.7% | 559 MB | 584 MB | +4.4% | ~ |
| `ClosestPrimitive` | 31.1 s | 30.2 s | -2.9% | 1348 MB | 1336 MB | -0.9% | ~ |
| `ClosestSurface` | 28.5 s | 28.2 s | -0.9% | 1317 MB | 1261 MB | -4.3% | ~ |
| `ConnectedComponents` | 15.7 s | 15.5 s | -1.4% | 855 MB | 829 MB | -3.0% | ~ |
| `Domain` | 18.3 s | 18.0 s | -1.3% | 860 MB | 894 MB | +3.8% | ~ |
| `DomainObject` | 16.5 s | 16.2 s | -1.4% | 860 MB | 830 MB | -3.6% | ~ |
| `EBGrid` | 15.3 s | 14.7 s | -3.8% | 784 MB | 782 MB | -0.2% | ~ |
| `Fused` | 25.0 s | 24.9 s | -0.8% | 1236 MB | 1233 MB | -0.3% | ~ |
| `LaunchTuning` | 38.2 s | 37.3 s | -2.2% | 1328 MB | 1261 MB | -5.0% | ~ |
| `Mesh` | 16.2 s | 15.8 s | -2.5% | 791 MB | 826 MB | +4.4% | ~ |
| `MeshCSG` | 40.6 s | 40.2 s | -0.8% | 1306 MB | 1313 MB | +0.5% | ~ |
| `MeshClosestPoint` | 14.9 s | 14.5 s | -2.9% | 786 MB | 824 MB | +4.9% | ~ |
| `MeshIngest` | 7.4 s | 7.1 s | -3.9% | 569 MB | 593 MB | +4.3% | ~ |
| `MeshReconcile` | 7.1 s | 6.8 s | -4.2% | 569 MB | 593 MB | +4.2% | ~ |
| `MeshTape` | 43.3 s | 42.3 s | -2.3% | 1261 MB | 1300 MB | +3.1% | ~ |
| `PointCloud` | 7.6 s | 7.5 s | -1.2% | 562 MB | 586 MB | +4.4% | ~ |
| `PointCloudBVHBuild` | 3.9 s | 3.8 s | -2.5% | 476 MB | 475 MB | -0.3% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | -0.8% | 457 MB | 450 MB | -1.4% | ~ |
| `QueryOrder` | 11.6 s | 11.5 s | -0.5% | 708 MB | 723 MB | +2.1% | ~ |
| `Ray` | 37.4 s | 36.6 s | -2.1% | 1342 MB | 1304 MB | -2.8% | ~ |
| `RayCSG` | 32.0 s | 31.9 s | -0.2% | 1204 MB | 1222 MB | +1.5% | ~ |
| `RayCrossing` | 46.0 s | 46.0 s | -0.1% | 1360 MB | 1330 MB | -2.3% | ~ |
| `RayParticles` | 49.6 s | 49.8 s | +0.3% | 1334 MB | 1316 MB | -1.3% | ~ |
| `RayShell` | 38.2 s | 38.2 s | -0.0% | 1312 MB | 1259 MB | -4.1% | ~ |
| `Regions` | 29.7 s | 29.4 s | -1.0% | 1115 MB | 1106 MB | -0.8% | ~ |
| `Render` | 36.5 s | 36.5 s | -0.0% | 1300 MB | 1265 MB | -2.7% | ~ |
| `Shapes` | 9.0 s | 8.9 s | -1.7% | 559 MB | 584 MB | +4.4% | ~ |
| `SphereCloudBVH` | 4.9 s | 4.9 s | -0.0% | 518 MB | 528 MB | +1.9% | ~ |
| `SphereCloudRays` | 28.4 s | 28.2 s | -0.7% | 1078 MB | 1092 MB | +1.4% | ~ |
| `SphereContacts` | 4.4 s | 4.4 s | -0.5% | 490 MB | 494 MB | +0.7% | ~ |
| `SurfaceParts` | 40.8 s | 40.8 s | -0.0% | 1418 MB | 1446 MB | +2.0% | ~ |
| `Tape` | 16.9 s | 16.7 s | -1.0% | 838 MB | 851 MB | +1.6% | ~ |
| `TapeBVHUnion` | 14.7 s | 14.6 s | -0.1% | 840 MB | 830 MB | -1.2% | ~ |
| **all, in both** | 774.7 s | 766.2 s | -1.1% | 1418 MB | 1446 MB | +2.0% | ~ |


The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (3 cases): RayCrossing/csg-tape/firstCrossing/double/host, Render/csg/frame/float/host, Render/csg/frame-orthographic/float/host

These probes compiled to the same code on both sides and were not run (24 cases): PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 344147cc
base-fingerprint: 1e0f461958307254a04eb0a9ea2890c216b99160
head: 8e2b3266
head-fingerprint: e86287a4b012022f3f191214014da3cfa96437ea
cases: 386
rounds: 2
reps: 3
date: 2026-11-26
-->

## Device comparison (CUDA): green

**Accepted:** -

`8e2b3266` against `344147cc` (344147cc), 2026-11-26.

**VERDICT: no regression (0 major, 0 minor across 386 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 386 cases

- base: 344147cc (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8e2b3266 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.63 ms | 1.63 ms | -0.1% | [-1.7%, +1.5%] | - | ~ | device 23.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.40 ms | +0.0% | [-0.9%, +0.9%] | - | ~ | device 19.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.06 ms | 1.07 ms | +1.4% | [+1.3%, +1.6%] | - | ~ | device 19.2x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.33 ms | 2.30 ms | -1.3% | [-1.8%, -0.9%] | - | ~ | device 17.2x faster than the host (was 16.5x) |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.24 ms | 6.20 ms | -0.7% | [-2.2%, +0.9%] | - | ~ | device 8.1x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.8 ms | 11.8 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.4 ms | 33.3 ms | -0.2% | [-1.2%, +0.7%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.2 ms | 57.2 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.2x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.9 ms | 20.8 ms | -0.2% | [-1.0%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.0 ms | 47.8 ms | -0.4% | [-0.8%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 62.0 ms | 60.6 ms | -2.2% | [-3.5%, -0.8%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 111 ms | +1.6% | [+1.0%, +2.2%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.39 ms | 8.35 ms | -0.4% | [-1.8%, +1.0%] | +0.0% | ~ | device 5.2x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 10.0 ms | 10.1 ms | +0.6% | [-1.6%, +2.9%] | +0.0% | ~ | device 8.4x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 386 µs | 387 µs | +0.2% | [-0.2%, +0.7%] | - | ~ | device 15.3x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 841 µs | 844 µs | +0.3% | [-0.4%, +1.0%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.53 ms | 4.53 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.69 ms | 4.66 ms | -0.5% | [-0.7%, -0.3%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 585 µs | 585 µs | -0.0% | [-0.7%, +0.7%] | - | ~ | device 13.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.75 ms | 6.75 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.02 ms | 7.03 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 907 µs | 909 µs | +0.3% | [-0.1%, +0.7%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.23 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.45 ms | 9.49 ms | +0.3% | [-0.0%, +0.7%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 846 µs | 850 µs | +0.4% | [+0.2%, +0.6%] | - | ~ | device 13.6x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.07 ms | 2.07 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 8.0x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.10 ms | 9.10 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.83 ms | 9.83 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 308 µs | 307 µs | -0.2% | [-0.4%, -0.0%] | - | ~ | device 12.0x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.35 ms | 1.35 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 6.1x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.7 ms | 59.0 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 3.4x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.43 ms | 5.43 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 523 ms | 521 ms | -0.4% | [-1.2%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.3 ms | 20.3 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 4.2x faster than the host |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.39 ms | 8.38 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 4.8x faster than the host; surface/leaf 2.42x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.6 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 8.8x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.18 ms | 6.18 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 10.1x faster than the host; surface/leaf 2.52x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 146 ms | 148 ms | +1.2% | [-0.2%, +2.6%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.0 ms | 60.0 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 0.9x faster than the host (was 0.7x); surface/leaf 2.46x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 72.0 ms | 72.0 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.3x faster than the host; surface/leaf 2.57x |
| `Domain/particles/labels/float/hostdevice` | 4.03 ms | 4.05 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 3.5x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.76 ms | 2.75 ms | -0.3% | [-0.7%, -0.0%] | - | ~ | device 2.7x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 117 µs | 114 µs | -2.7% | [-4.4%, -1.0%] | - | ~ | device 13.1x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.02 ms | 6.94 ms | -1.0% | [-1.8%, -0.2%] | - | ~ | device 2.1x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.52 ms | 3.51 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 2.1x faster than the host |
| `Domain/particles/surfaceGroups/double/hostdevice` | 117 µs | 116 µs | -0.8% | [-2.2%, +0.5%] | - | ~ | device 13.0x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 13.2 ms | 13.1 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 3.5x faster than the host |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.89 ms | 8.88 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 2.7x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 266 µs | 266 µs | +0.3% | [-0.0%, +0.7%] | - | ~ | device 2.5x faster than the host (was 2.6x); whole/own 49.34x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 159 µs | 157 µs | -1.2% | [-2.0%, -0.3%] | - | ~ | device 2.2x faster than the host; whole/own 56.53x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 19.6 ms | 19.5 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 2.4x faster than the host |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 11.4 ms | 11.4 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | device 2.2x faster than the host |
| `DomainObject/partsOwn/labels/double/hostdevice` | 401 µs | 393 µs | -1.9% | [-5.6%, +1.8%] | - | ~ | device 1.8x faster than the host; whole/own 49.56x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 236 µs | 236 µs | -0.2% | [-1.5%, +1.0%] | - | ~ | device 1.6x faster than the host; whole/own 48.21x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/hostdevice` | 10.6 ms | 10.5 ms | -0.5% | [-1.3%, +0.3%] | +0.0% | ~ | device 10.0x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/hostdevice` | 10.3 ms | 10.4 ms | +0.4% | [-1.1%, +1.9%] | +0.0% | ~ | device 9.6x faster than the host; grid/morton 1.01x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/hostdevice` | 17.4 ms | 17.5 ms | +0.5% | [-0.4%, +1.3%] | +0.0% | ~ | device 9.0x faster than the host; grid/shuffled 0.60x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/hostdevice` | 222 ms | 228 ms | +2.5% | [+1.0%, +3.9%] | +0.0% | ~ | device 0.5x faster than the host (was 0.5x) |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/hostdevice` | 166 ms | 165 ms | -0.7% | [-2.6%, +1.2%] | +0.0% | ~ | device 0.6x faster than the host (was 0.8x); grid/morton 1.38x (was 1.34x) |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/hostdevice` | 285 ms | 293 ms | +2.7% | [+1.1%, +3.8%] | +0.0% | ~ | device 0.4x faster than the host; grid/shuffled 0.78x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/hostdevice` | 27.3 ms | 27.4 ms | +0.5% | [+0.1%, +0.8%] | +0.0% | ~ | device 4.0x faster than the host |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/hostdevice` | 29.2 ms | 28.9 ms | -1.1% | [-1.6%, -0.6%] | +0.0% | ~ | device 3.5x faster than the host (was 3.7x); grid/morton 0.95x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/hostdevice` | 60.5 ms | 60.5 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ | device 2.5x faster than the host; grid/shuffled 0.45x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/hostdevice` | 205 ms | 206 ms | +0.1% | [-5.4%, +5.8%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x) |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/hostdevice` | 157 ms | 161 ms | +2.4% | [+0.6%, +4.3%] | +0.0% | ~ | device 0.6x faster than the host; grid/morton 1.28x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/hostdevice` | 248 ms | 250 ms | +1.0% | [-0.8%, +2.8%] | +0.0% | ~ | device 0.7x faster than the host; grid/shuffled 0.82x |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 24.9x faster than the host; tape/fused 4.57x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.7 µs | 49.5 µs | -0.4% | [-0.6%, -0.3%] | - | ~ | device 44.7x faster than the host; tape/fused 7.25x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.7 µs | 47.7 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 9.8x faster than the host (was 9.1x); tape/fused 10.01x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.4 ms | 17.5 ms | +0.3% | [-0.8%, +1.4%] | - | ~ | device 6.9x faster than the host; tape/fused 1.79x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.2 ms | 31.2 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 3.9x faster than the host (was 5.2x) |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 281 ms | 285 ms | +1.7% | [+0.0%, +3.3%] | - | ~ | device 0.4x faster than the host; tape/fused 0.98x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 281 ms | 280 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.25 ms | 8.27 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ | device 11.0x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.21 ms | 8.32 ms | +1.4% | [-0.3%, +3.1%] | +0.0% | ~ | device 12.2x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.27 ms | 8.22 ms | -0.6% | [-1.2%, +0.1%] | +0.0% | ~ | device 13.0x faster than the host (was 12.3x); default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.15 ms | 8.24 ms | +1.1% | [+0.2%, +1.9%] | +0.0% | ~ | device 11.3x faster than the host (was 11.8x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 705 µs | 704 µs | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.78 ms | 1.79 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.02 ms | 1.03 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 2.6x faster than the host; default/this 0.69x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 799 µs | 800 µs | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 3.3x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 745 µs | 744 µs | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ | device 12.2x faster than the host (was 14.6x) |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 824 µs | 823 µs | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ | device 11.0x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 744 µs | 746 µs | +0.3% | [-0.8%, +1.5%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 725 µs | 730 µs | +0.8% | [-0.8%, +2.4%] | +0.0% | ~ | device 12.3x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 160 ms | -0.5% | [-2.5%, +1.6%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 159 ms | 161 ms | +1.1% | [+0.2%, +2.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 160 ms | 161 ms | +1.0% | [-1.2%, +3.3%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x); default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 159 ms | 162 ms | +1.8% | [+0.1%, +3.6%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ | device 2.4x faster than the host (was 1.9x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.46 ms | 1.45 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.96x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.92 ms | 1.93 ms | +0.7% | [+0.0%, +1.3%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.72x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.55 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.21 ms | 7.24 ms | +0.5% | [+0.4%, +0.6%] | +0.0% | ~ | device 1.5x faster than the host (was 1.3x) |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.25 ms | 7.29 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.28 ms | 7.22 ms | -0.7% | [-1.0%, -0.5%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.27 ms | 7.28 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.38 ms | 8.42 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 11.4x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.6 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 8.6x faster than the host (was 7.4x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 164 ms | 167 ms | +2.0% | [-0.8%, +4.9%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 286 ms | 288 ms | +0.7% | [-0.3%, +1.7%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.7 ms | 24.6 ms | -0.4% | [-0.8%, -0.0%] | - | ~ | device 4.6x faster than the host (was 4.1x) |
| `MeshSDF/query-random/float/hostdevice` | 57.9 ms | 57.9 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 3.2x faster than the host (was 2.6x) |
| `MeshSDF/query-morton/double/hostdevice` | 157 ms | 157 ms | -0.4% | [-2.1%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 245 ms | 245 ms | -0.1% | [-2.2%, +2.1%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.06 ms | 2.06 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 5.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.48 ms | 3.46 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 4.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.16 ms | 6.14 ms | -0.4% | [-1.1%, +0.3%] | - | ~ | device 4.8x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.6 ms | 65.1 ms | +2.4% | [+1.3%, +3.4%] | - | ~ | device 3.6x faster than the host (was 3.4x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 94.6 ms | 94.4 ms | -0.2% | [-1.6%, +1.3%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 239 ms | 237 ms | -0.8% | [-2.4%, +0.8%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.7 ms | 24.5 ms | -0.7% | [-2.3%, +0.8%] | - | ~ | device 2.1x faster than the host; tape/fused 2.66x (was 2.58x) |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 27.2 ms | +1.1% | [-0.4%, +2.8%] | - | ~ | device 2.2x faster than the host (was 2.6x); tape/fused 3.47x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.1 ms | 65.4 ms | +0.5% | [-0.2%, +1.3%] | - | ~ | device 4.2x faster than the host; tape/fused 3.63x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 20.3 ms | 21.1 ms | +3.7% | [-4.4%, +12.6%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 38.7 ms | 39.1 ms | +1.1% | [-0.5%, +2.7%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 42.1 ms | 42.4 ms | +0.7% | [-0.7%, +2.1%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 542 ms | 559 ms | +3.2% | [-0.9%, +7.3%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 843 ms | 849 ms | +0.7% | [-0.4%, +1.8%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.96 s | 1.95 s | -0.6% | [-1.6%, +0.3%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 198 ms | 196 ms | -1.1% | [-6.2%, +4.4%] | - | ~ | device 0.3x faster than the host; tape/fused 2.85x (was 2.73x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 312 ms | 310 ms | -0.8% | [-4.9%, +3.6%] | - | ~ | device 0.2x faster than the host; tape/fused 2.74x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 539 ms | 548 ms | +1.6% | [-0.8%, +4.1%] | - | ~ | device 0.5x faster than the host (was 0.5x); tape/fused 3.56x |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 12.1 ms | 12.1 ms | +0.3% | [-0.5%, +1.2%] | - | ~ | device 10.1x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 58.4 ms | 58.5 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.5x faster than the host (was 2.8x) |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 256 ms | 263 ms | +2.5% | [+2.0%, +3.1%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 245 ms | 247 ms | +0.8% | [-3.3%, +5.1%] | - | ~ | device 0.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | device 17.1x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.32 ms | 5.31 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | device 11.8x faster than the host |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.32 ms | 1.32 ms | +0.0% | [-0.9%, +1.0%] | - | ~ | device 16.2x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.31 ms | 1.32 ms | +0.4% | [-0.3%, +1.1%] | - | ~ | device 15.8x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.15 ms | 2.15 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 8.0x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.52 ms | 3.53 ms | +0.2% | [+0.2%, +0.2%] | - | ~ | device 10.4x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.77 ms | 2.76 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 8.1x faster than the host; mesh/tape 0.48x |
| `MeshTape/tape/frame/float/hostdevice` | 4.41 ms | 4.39 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 5.1x faster than the host; mesh/tape 0.49x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.73 ms | 3.73 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 10.0x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.62 ms | 5.67 ms | +0.8% | [+0.6%, +1.1%] | - | ~ | device 4.3x faster than the host; mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 30.4 ms | 30.2 ms | -0.5% | [-1.5%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 30.3 ms | 30.4 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 0.7x faster than the host (was 0.8x) |
| `MeshTape/mesh/frame/double/hostdevice` | 25.8 ms | 26.1 ms | +1.0% | [+0.6%, +1.5%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 43.4 ms | 43.2 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 0.9x faster than the host; mesh/tape 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 30.6 ms | 30.5 ms | -0.2% | [-1.2%, +0.8%] | - | ~ | device 0.7x faster than the host; mesh/tape 1.00x |
| `MeshTape/tape/frame/double/hostdevice` | 33.1 ms | 32.9 ms | -0.7% | [-1.6%, +0.3%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.79x |
| `MeshTape/view/firstHit/double/hostdevice` | 43.5 ms | 43.3 ms | -0.4% | [-0.8%, -0.0%] | - | ~ | device 0.9x faster than the host; mesh/view 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 36.2 ms | 36.1 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 0.7x faster than the host; mesh/view 0.72x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 682 µs | 680 µs | -0.3% | [-1.1%, +0.5%] | - | ~ | device 11.4x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.83 ms | 1.83 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 5.8x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 997 µs | 996 µs | -0.1% | [-0.5%, +0.4%] | - | ~ | device 9.6x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.61 ms | 4.60 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 5.6x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.96 ms | 2.96 ms | -0.0% | [-0.9%, +0.8%] | - | ~ | device 5.4x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.07 ms | 7.07 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.80 ms | 7.78 ms | -0.3% | [-0.7%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 0.9x faster than the host (was 0.8x) |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 12.0 ms | 12.0 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.45 ms | 1.47 ms | +1.2% | [+0.9%, +1.5%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.89 ms | 3.87 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 3.4x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 523 µs | 524 µs | +0.2% | [-0.4%, +0.8%] | - | ~ | device 16.2x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.55 ms | 4.57 ms | +0.5% | [-1.0%, +2.0%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.19 ms | 4.18 ms | -0.3% | [-2.6%, +2.2%] | - | ~ | device 5.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.13 ms | 8.17 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.8 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 0.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.01 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.5 ms | 17.6 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.7 ms | 14.9 ms | +1.6% | [+0.5%, +2.6%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.59 ms | 1.58 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 10.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.61 ms | 4.61 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 835 µs | 829 µs | -0.7% | [-1.2%, -0.2%] | - | ~ | device 10.5x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.85 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 7.1x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.13 ms | 4.11 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.7 ms | 16.5 ms | -1.0% | [-2.0%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.0 ms | 21.0 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.94 ms | 8.98 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.5 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.2 ms | 20.2 ms | -0.1% | [-1.0%, +0.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.75 ms | 2.73 ms | -0.9% | [-2.3%, +0.5%] | - | ~ | device 4.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.72 ms | 6.74 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 2.4x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.2% | [-0.7%, +1.2%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.72 ms | 2.71 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 8.3x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.43 ms | 6.42 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 3.1x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.9 ms | 33.9 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.4 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.3 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.7 ms | 20.9 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.8 ms | 38.0 ms | -2.1% | [-3.6%, -0.5%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `QueryOrder/random/mortonOrder/float/hostdevice` | 16.9 ms | 16.9 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `QueryOrder/armadillo/distance-arrived/float/hostdevice` | 9.17 ms | 9.16 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | device 5.0x faster than the host |
| `QueryOrder/armadillo/distance-ordered/float/hostdevice` | 7.41 ms | 7.42 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ | device 5.5x faster than the host; arrived/ordered 1.23x |
| `QueryOrder/random/mortonOrder/double/hostdevice` | 17.6 ms | 17.6 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.5x faster than the host |
| `QueryOrder/armadillo/distance-arrived/double/hostdevice` | 111 ms | 113 ms | +1.6% | [-0.3%, +3.6%] | +0.0% | ~ | device 0.4x faster than the host |
| `QueryOrder/armadillo/distance-ordered/double/hostdevice` | 79.2 ms | 79.9 ms | +0.9% | [-2.0%, +3.8%] | +0.0% | ~ | device 0.5x faster than the host; arrived/ordered 1.41x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 711 µs | 704 µs | -0.9% | [-1.7%, -0.2%] | - | ~ | device 17.6x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.31 ms | 1.32 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | device 15.5x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 660 µs | 660 µs | +0.1% | [-0.8%, +1.0%] | - | ~ | device 20.3x faster than the host (was 17.3x) |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.24 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 15.5x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 633 µs | 629 µs | -0.6% | [-1.2%, -0.0%] | - | ~ | device 15.9x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.16 ms | 1.16 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.19 ms | +0.4% | [-0.2%, +1.1%] | - | ~ | device 12.2x faster than the host; triangles/polygons 0.59x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.49 ms | 2.48 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 10.0x faster than the host; triangles/polygons 0.53x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.08 ms | +0.5% | [-0.3%, +1.2%] | - | ~ | device 11.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 10.2x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 594 µs | 609 µs | +2.5% | [-0.8%, +5.7%] | - | ~ | device 2.5x faster than the host; tape/fused 1.16x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 728 µs | 725 µs | -0.4% | [-0.7%, -0.1%] | - | ~ | device 2.5x faster than the host; tape/fused 1.18x (was 1.37x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 704 µs | 706 µs | +0.3% | [-0.1%, +0.6%] | - | ~ | device 3.6x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 996 µs | 852 µs | -14.4% | [-25.5%, +0.5%] | - | ~ | device 3.6x faster than the host (was 3.2x) |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 18.8 ms | 18.7 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 0.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 30.3 ms | 30.2 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 0.7x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 18.1 ms | 18.2 ms | +0.5% | [-0.4%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 29.2 ms | 29.1 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.8 ms | 15.9 ms | +0.3% | [-0.4%, +1.0%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 26.1 ms | 26.0 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 17.1 ms | 17.2 ms | +0.7% | [-0.6%, +2.0%] | - | ~ | device 1.0x faster than the host (was 0.9x); triangles/polygons 1.09x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.4 ms | 25.4 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.19x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.1 ms | +0.5% | [-1.2%, +2.3%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 23.1 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 1.1x faster than the host (was 1.0x) |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.36 ms | 2.37 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.1% | [-0.8%, +0.5%] | - | ~ | device 2.0x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.48 ms | 2.49 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.53 ms | 2.53 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.7x faster than the host (was 2.6x); marched/traced 3.56x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.98 ms | 2.98 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.4x faster than the host (was 2.3x); marched/traced 4.65x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.01 ms | 9.01 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 5.1x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 3.8x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.16 ms | 2.16 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 4.0x faster than the host; marched/traced 13.42x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.43 ms | 5.43 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.9x faster than the host; marched/traced 10.16x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 28.9 ms | 29.0 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.4x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.0 ms | 55.1 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 2.3x faster than the host (was 2.1x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.24 ms | 5.23 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.81 ms | 8.82 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.13 ms | 6.14 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.3x faster than the host (was 1.3x); marched/traced 26.24x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.5 ms | 10.5 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 0.8x faster than the host (was 0.8x); marched/traced 24.38x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 160 ms | 161 ms | +0.7% | [-1.6%, +3.0%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 257 ms | 257 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.87 ms | 4.86 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 1.8x faster than the host; marched/traced 22.60x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.3x faster than the host; marched/traced 17.58x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 110 ms | 110 ms | -0.3% | [-2.7%, +2.2%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 219 ms | 214 ms | -2.2% | [-3.5%, -1.0%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.4 ms | -0.2% | [-0.5%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.7 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 703 µs | 709 µs | +0.9% | [+0.6%, +1.2%] | - | ~ | device 17.4x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.32 ms | 1.32 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 15.5x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 704 µs | 708 µs | +0.6% | [-0.2%, +1.4%] | - | ~ | device 17.5x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.31 ms | 1.31 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 15.6x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 708 µs | 707 µs | -0.1% | [-0.3%, +0.0%] | - | ~ | device 3.7x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 857 µs | 853 µs | -0.4% | [-1.6%, +0.9%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 600 µs | 601 µs | +0.2% | [-0.0%, +0.4%] | - | ~ | device 3.3x faster than the host; hit/crossing 1.18x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 626 µs | 627 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 3.8x faster than the host; hit/crossing 1.36x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.76 ms | 4.77 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 10.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.3 ms | 10.3 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 7.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.24 ms | 4.23 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 10.1x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.16 ms | 9.16 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 6.9x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.7 ms | 56.9 ms | +0.5% | [-2.2%, +3.2%] | - | ~ | device 3.5x faster than the host (was 3.2x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 82.0 ms | 82.2 ms | +0.2% | [-0.9%, +1.3%] | - | ~ | device 2.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.1 ms | 48.2 ms | +0.3% | [-1.7%, +2.3%] | - | ~ | device 3.4x faster than the host (was 3.1x); hit/crossing 1.18x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.4 ms | 63.3 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 2.5x faster than the host (was 2.9x); hit/crossing 1.30x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 18.8 ms | 18.8 ms | -0.4% | [-0.9%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 30.1 ms | 30.5 ms | +1.1% | [+0.9%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 18.7 ms | 18.7 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 30.2 ms | 30.3 ms | +0.0% | [-0.6%, +0.7%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.49 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.10 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.27x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.14x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 68.6 ms | 68.8 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 129 ms | 129 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 62.3 ms | 63.4 ms | +1.8% | [-0.1%, +3.8%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.09x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 118 ms | 118 ms | +0.5% | [-0.8%, +1.9%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.09x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 467 ms | 480 ms | +2.6% | [-4.5%, +9.6%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 715 ms | 714 ms | -0.3% | [-2.1%, +1.6%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 394 ms | 401 ms | +1.7% | [-3.7%, +6.2%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.20x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 533 ms | 522 ms | -2.0% | [-4.3%, +0.3%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.37x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.56 ms | 3.57 ms | +0.5% | [-2.5%, +3.6%] | - | ~ | device 1.2x faster than the host; marched/traced 3.35x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.82 ms | 4.82 ms | +0.1% | [-0.7%, +0.9%] | - | ~ | device 0.9x faster than the host; marched/traced 5.53x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 4.0x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.5 ms | 26.6 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 2.1x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.27 ms | 1.25 ms | -1.5% | [-1.8%, -1.3%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.86 ms | +0.6% | [+0.1%, +1.2%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 97.7 µs | 99.2 µs | +1.6% | [-0.7%, +3.8%] | - | ~ | device 14.7x faster than the host (was 15.3x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 154 µs | 155 µs | +0.7% | [-0.5%, +2.0%] | - | ~ | device 10.9x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 797 µs | 795 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 935 µs | 935 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 36.1 ms | 35.4 ms | -2.0% | [-7.8%, +3.9%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 3.43x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 37.2 ms | 36.1 ms | -2.8% | [-6.1%, +0.7%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 6.53x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 121 ms | 122 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 237 ms | 236 ms | -0.3% | [-1.6%, +1.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.1 ms | 16.2 ms | +0.7% | [-2.2%, +5.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 21.7 ms | -0.8% | [-1.8%, +0.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.21 ms | 1.19 ms | -2.1% | [-2.8%, -1.4%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.52 ms | 2.51 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 6.00 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.74 ms | 6.74 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 0.5x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.3 ms | 23.0 ms | -1.2% | [-2.7%, +0.4%] | - | ~ | device 2.2x faster than the host; marched/traced 2.69x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.5 ms | 28.4 ms | -0.3% | [-1.2%, +0.5%] | - | ~ | device 2.0x faster than the host (was 2.1x); marched/traced 3.68x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 61.4 ms | 61.8 ms | +0.7% | [-0.2%, +1.7%] | - | ~ | device 4.1x faster than the host (was 4.6x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 103 ms | 105 ms | +1.3% | [-0.3%, +3.0%] | - | ~ | device 2.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.9 ms | 13.9 ms | -0.2% | [-1.1%, +0.7%] | - | ~ | device 3.7x faster than the host (was 4.3x) |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.5 ms | 17.4 ms | -0.5% | [-1.0%, +0.1%] | - | ~ | device 3.8x faster than the host (was 3.2x) |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 308 ms | 313 ms | +1.5% | [-0.8%, +3.9%] | - | ~ | device 0.2x faster than the host; marched/traced 1.80x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 351 ms | 356 ms | +1.3% | [-0.3%, +3.1%] | - | ~ | device 0.2x faster than the host; marched/traced 2.82x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 566 ms | 564 ms | -0.3% | [-3.0%, +2.6%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 987 ms | 1.00 s | +1.6% | [+1.0%, +2.1%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 188 ms | 191 ms | +1.8% | [+0.2%, +3.4%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 239 ms | 237 ms | -1.2% | [-2.9%, +0.5%] | - | ~ | device 0.3x faster than the host |
| `Regions/particles/region/float/hostdevice` | 3.01 ms | 2.99 ms | -0.6% | [-1.3%, +0.2%] | - | ~ | device 2.6x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.65 ms | 3.65 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 3.6x faster than the host (was 4.5x) |
| `Regions/particles/regionByRegion/float/hostdevice` | 97.5 ms | 98.3 ms | +0.8% | [-0.0%, +1.6%] | - | ~ | device 3.7x faster than the host (was 4.0x); one-pass/each 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.72 ms | 3.72 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 3.8x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.79 ms | 6.78 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 3.8x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.62 ms | 5.61 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 3.2x faster than the host; regions/hit 1.21x |
| `Regions/particles/region/double/hostdevice` | 3.43 ms | 3.43 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 2.5x faster than the host (was 2.3x) |
| `Regions/particles/regionDistances/double/hostdevice` | 5.80 ms | 5.83 ms | +0.5% | [-0.9%, +2.0%] | - | ~ | device 2.4x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 119 ms | 119 ms | +0.0% | [-1.0%, +1.0%] | - | ~ | device 2.9x faster than the host (was 3.1x); one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.91 ms | 5.88 ms | -0.5% | [-0.7%, -0.2%] | - | ~ | device 2.4x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.8 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 12.9 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.4x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.37 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.1x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.35 ms | 2.35 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.1x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.14 ms | 2.13 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 8.1x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.63 ms | 5.63 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.62 ms | 5.62 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 26.0 ms | 26.1 ms | +0.3% | [-0.7%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 888 µs | 887 µs | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 10.6x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.65 ms | 2.65 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 4.9x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 1.0x faster than the host (was 1.0x) |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | -0.0% | [-0.0%, +0.0%] | +0.0% | ~ | device 4.0x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 4.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.02 ms | 3.01 ms | -0.5% | [-1.3%, +0.3%] | +0.0% | ~ | device 7.4x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.77 ms | 4.76 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ | device 5.2x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ | device 5.4x faster than the host; tape/fused 1.38x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.0 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 3.9x faster than the host; tape/fused 1.27x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 23.0 ms | 23.0 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 4.3x faster than the host (was 4.1x) |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.6 ms | 30.6 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 3.1x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.1 ms | 44.0 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.7 ms | 45.7 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 122 ms | 123 ms | +1.0% | [-1.0%, +3.0%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 125 ms | 126 ms | +0.6% | [-1.8%, +3.2%] | +0.0% | ~ | device 0.9x faster than the host (was 0.8x); tape/fused 1.08x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 128 ms | 129 ms | +0.8% | [-2.8%, +4.6%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 132 ms | 136 ms | +2.6% | [+1.0%, +4.4%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | device 10.9x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.1 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ | device 0.6x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.29 ms | 9.29 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 11.6x faster than the host |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.92 ms | 5.91 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 12.2x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 95.8 ms | 97.7 ms | +2.0% | [+1.4%, +2.7%] | - | ~ | device 3.7x faster than the host (was 4.0x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.9 ms | 64.0 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 3.4x faster than the host (was 3.6x); surface/leaf 1.53x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.8 ms | 29.8 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 6.9x faster than the host (was 5.8x) |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 10.00 ms | 10.0 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 6.9x faster than the host; surface/hit 2.97x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.0 ms | 59.1 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.5x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 785 ms | 786 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 529 ms | 528 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.49x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 315 ms | 316 ms | +0.3% | [-0.5%, +1.1%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 128 ms | 130 ms | +1.2% | [+0.9%, +1.5%] | - | ~ | device 0.6x faster than the host; surface/hit 2.44x |
| `Tape/csg/signedDistance/float/hostdevice` | 219 µs | 219 µs | +0.3% | [-0.3%, +0.9%] | - | ~ | device 13.9x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 48.5 µs | +1.6% | [+0.0%, +3.3%] | - | ~ | device 23.6x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 358 µs | 359 µs | +0.1% | [-0.3%, +0.5%] | - | ~ | device 12.8x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | -0.4% | [-1.0%, +0.2%] | - | ~ | device 9.3x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 71.8 µs | 71.9 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 7.1x faster than the host (was 7.6x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 48.0 µs | +0.4% | [-0.9%, +1.7%] | - | ~ | device 4.8x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 652 µs | +0.2% | [+0.0%, +0.3%] | - | ~ | device 10.9x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 13.6x faster than the host (was 11.8x) |
| `Tape/chain16/signedDistance/float/hostdevice` | 477 µs | 478 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.5 µs | 48.2 µs | -0.4% | [-0.7%, -0.2%] | - | ~ | device 30.2x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 859 µs | 859 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 3.8x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 813 µs | 810 µs | -0.4% | [-0.7%, -0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.64 ms | 6.64 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 191 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 3.2x faster than the host (was 2.9x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.1x faster than the host (was 1.9x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.77 ms | 2.78 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.5x faster than the host (was 2.7x) |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 8.3x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.70 ms | 3.71 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 11.3x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.50 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.5x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4677.7 s → 4683.1 s (+0.1%), the largest peak 5453 MB → 5409 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.6 s | 13.6 s | -0.1% | 619 MB | 628 MB | +1.5% | ~ |
| `BVHRefit` | 7.2 s | 7.2 s | +0.0% | 523 MB | 528 MB | +1.0% | ~ |
| `BVHUnion` | 25.7 s | 26.2 s | +2.0% | 1042 MB | 1051 MB | +0.9% | ~ |
| `BVHUnionMixed` | 11.3 s | 11.3 s | +0.1% | 636 MB | 646 MB | +1.5% | ~ |
| `ClosestPrimitive` | 105.9 s | 105.7 s | -0.2% | 1371 MB | 1380 MB | +0.6% | ~ |
| `ClosestSurface` | 138.8 s | 141.8 s | +2.2% | 1316 MB | 1316 MB | +0.0% | ~ |
| `ConnectedComponents` | 20.3 s | 20.6 s | +1.6% | 952 MB | 961 MB | +1.0% | ~ |
| `Domain` | 50.8 s | 51.4 s | +1.2% | 1007 MB | 1016 MB | +0.9% | ~ |
| `DomainObject` | 49.5 s | 49.6 s | +0.2% | 959 MB | 968 MB | +0.9% | ~ |
| `EBGrid` | 20.0 s | 20.2 s | +0.8% | 767 MB | 776 MB | +1.2% | ~ |
| `Fused` | 51.9 s | 51.5 s | -0.8% | 1178 MB | 1187 MB | +0.7% | ~ |
| `LaunchTuning` | 452.3 s | 453.7 s | +0.3% | 3134 MB | 3133 MB | -0.0% | ~ |
| `Mesh` | 22.2 s | 22.2 s | +0.2% | 824 MB | 833 MB | +1.1% | ~ |
| `MeshCSG` | 366.8 s | 368.0 s | +0.3% | 5194 MB | 5192 MB | -0.0% | ~ |
| `MeshClosestPoint` | 19.6 s | 19.8 s | +1.1% | 776 MB | 785 MB | +1.2% | ~ |
| `MeshIngest` | 10.7 s | 10.9 s | +1.5% | 549 MB | 558 MB | +1.7% | ~ |
| `MeshReconcile` | 10.6 s | 10.7 s | +0.9% | 526 MB | 535 MB | +1.7% | ~ |
| `MeshTape` | 649.6 s | 651.3 s | +0.3% | 5453 MB | 5409 MB | -0.8% | ~ |
| `PointCloud` | 13.0 s | 12.7 s | -2.5% | 578 MB | 587 MB | +1.6% | ~ |
| `PointCloudBVHBuild` | 7.4 s | 7.5 s | +0.4% | 523 MB | 528 MB | +1.0% | ~ |
| `PointCloudHashGridBuild` | 6.5 s | 6.6 s | +1.2% | 523 MB | 528 MB | +0.9% | ~ |
| `QueryOrder` | 16.2 s | 16.4 s | +1.6% | 708 MB | 717 MB | +1.3% | ~ |
| `Ray` | 165.6 s | 165.7 s | +0.1% | 3030 MB | 3030 MB | +0.0% | ~ |
| `RayCSG` | 200.8 s | 200.0 s | -0.4% | 3368 MB | 3368 MB | +0.0% | ~ |
| `RayCrossing` | 388.9 s | 389.2 s | +0.1% | 3299 MB | 3300 MB | +0.0% | ~ |
| `RayParticles` | 421.4 s | 421.0 s | -0.1% | 4847 MB | 4846 MB | -0.0% | ~ |
| `RayShell` | 242.4 s | 243.2 s | +0.3% | 4604 MB | 4604 MB | +0.0% | ~ |
| `Regions` | 315.6 s | 315.9 s | +0.1% | 3364 MB | 3364 MB | -0.0% | ~ |
| `Render` | 238.4 s | 234.7 s | -1.5% | 4920 MB | 4920 MB | +0.0% | ~ |
| `Shapes` | 27.4 s | 27.2 s | -0.8% | 599 MB | 608 MB | +1.5% | ~ |
| `SphereCloudBVH` | 9.0 s | 9.2 s | +2.5% | 524 MB | 529 MB | +1.0% | ~ |
| `SphereCloudRays` | 157.0 s | 157.3 s | +0.2% | 3332 MB | 3332 MB | +0.0% | ~ |
| `SphereContacts` | 8.6 s | 8.7 s | +1.2% | 523 MB | 528 MB | +0.9% | ~ |
| `SurfaceParts` | 334.7 s | 333.4 s | -0.4% | 3493 MB | 3493 MB | -0.0% | ~ |
| `Tape` | 57.5 s | 57.7 s | +0.2% | 992 MB | 1001 MB | +0.9% | ~ |
| `TapeBVHUnion` | 40.5 s | 40.9 s | +0.9% | 932 MB | 941 MB | +1.0% | ~ |
| **all, in both** | 4677.7 s | 4683.1 s | +0.1% | 5453 MB | 5409 MB | -0.8% | ~ |

Device kernels: 226 in both, 2 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, RegionView<Tape<double, TriMeshSDF<…` | 168 → 168 | 20400 → 20464 | 2788/2348 → 2788/2348 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, RegionView<Tape<float, TriMeshSDF<fl…` | 168 → 168 | 14768 → 14832 | 2904/2136 → 2904/2136 | ~ |


The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (5 cases): EBGrid/TriMeshSDF/armadillo/distance-grid/double/hostdevice, EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/hostdevice, RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice, RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice, RayParticles/armadillo-shell-fused/step-morton/double/hostdevice

These probes compiled to the same code on both sides and were not run (28 cases): PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Shapes (20)

<!-- END receipt cuda -->
