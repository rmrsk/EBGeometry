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
verdict: yellow
base: 344147cc
base-fingerprint: 1e0f461958307254a04eb0a9ea2890c216b99160
head: 8f68163a
head-fingerprint: 79251e22a16b68e7b8cfafb786566a9b91ea9555
cases: 356
rounds: 2
reps: 3
date: 2026-11-26
-->

## Host comparison: yellow

**Accepted:** Accept as borderline acceptable regression in order to support the new shapes.

`8f68163a` against `344147cc` (origin/main), 2026-11-26.

**VERDICT: 2 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 2 minor, 1 faster, 0 differ, 0 lost, 356 cases

- base: 344147cc (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8f68163a (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 332 ms | 330 ms | -0.6% | [-0.7%, -0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 177 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 513 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 348 ms | 349 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 193 ms | +3.8% | [-0.2%, +7.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 564 ms | 563 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.8 ms | 41.7 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 76.2 ms | 76.1 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.0 ms | 56.0 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.2 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.4 ms | 64.5 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.6 ms | 54.9 ms | +0.4% | [-0.7%, +1.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.6 ms | 60.6 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.3 ms | 72.2 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 71.9 ms | 72.6 ms | +1.0% | [+0.6%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.6 ms | 77.3 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.5 ms | 64.6 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 100 ms | -0.2% | [-1.3%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 76.8 ms | 77.1 ms | +0.4% | [+0.3%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | -0.3% | [-1.6%, +1.0%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 63.6 ms | 63.9 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 96.0 ms | 95.6 ms | -0.5% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 75.9 ms | 75.9 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 102 ms | 101 ms | -1.2% | [-2.7%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.7 ms | 20.6 ms | +4.4% | [+3.4%, +5.0%] | +0.0% | minor |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.2 ms | 41.1 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.1 ms | 72.9 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.5 ms | 20.2 ms | -1.6% | [-2.2%, -1.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.9 ms | 42.2 ms | -4.0% | [-4.9%, -3.0%] | +0.0% | faster |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.2 ms | 80.8 ms | -0.5% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 142 ms | 141 ms | -0.9% | [-2.8%, +0.9%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 76.5 ms | 76.6 ms | +0.2% | [-1.8%, +2.1%] | +0.0% | ~ | surface/leaf 1.84x |
| `ClosestSurface/union/closestSurface/float/host` | 146 ms | 146 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 76.8 ms | 77.3 ms | +0.7% | [-1.1%, +2.5%] | +0.0% | ~ | surface/leaf 1.89x |
| `ClosestSurface/cloud/closestSurface/double/host` | 158 ms | 157 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 81.0 ms | 81.3 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ | surface/leaf 1.94x |
| `ClosestSurface/union/closestSurface/double/host` | 153 ms | 155 ms | +1.1% | [+0.1%, +2.2%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 81.5 ms | 81.7 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ | surface/leaf 1.90x |
| `ConnectedComponents/shells/build-coarse/float/host` | 148 ms | 149 ms | +0.8% | [-0.6%, +2.3%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/float/host` | 658 ms | 663 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-coarse/double/host` | 154 ms | 153 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/double/host` | 687 ms | 680 ms | -1.0% | [-1.4%, -0.5%] | +0.0% | ~ |  |
| `Domain/particles/labels/float/host` | 13.5 ms | 13.6 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.79 ms | 6.79 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.31 ms | 1.31 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.2 ms | 14.1 ms | -0.7% | [-1.1%, -0.2%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.14 ms | 7.12 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.31 ms | 1.32 ms | +0.4% | [-0.7%, +1.5%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 39.5 ms | 39.3 ms | -0.5% | [-0.6%, -0.3%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.2 ms | 21.4 ms | +1.2% | [+0.1%, +2.2%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 558 µs | 573 µs | +2.8% | [+0.8%, +4.9%] | +0.0% | ~ | whole/own 68.53x (was 70.77x) |
| `DomainObject/partsOwn/labelAt/float/host` | 309 µs | 305 µs | -1.3% | [-3.2%, +0.7%] | +0.0% | ~ | whole/own 70.31x |
| `DomainObject/partsWhole/labels/double/host` | 39.4 ms | 39.6 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.6 ms | 22.6 ms | +0.0% | [-1.3%, +1.4%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 580 µs | 579 µs | -0.1% | [-1.7%, +1.5%] | +0.0% | ~ | whole/own 68.33x |
| `DomainObject/partsOwn/labelAt/double/host` | 314 µs | 318 µs | +1.2% | [+1.0%, +1.4%] | +0.0% | ~ | whole/own 71.06x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/host` | 63.7 ms | 63.4 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/host` | 59.3 ms | 59.4 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/host` | 77.1 ms | 77.1 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | grid/shuffled 0.82x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/host` | 68.4 ms | 68.7 ms | +0.4% | [-0.5%, +1.3%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/host` | 63.2 ms | 63.4 ms | +0.4% | [+0.2%, +0.5%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/host` | 88.2 ms | 88.3 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | grid/shuffled 0.78x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/host` | 190 ms | 191 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/host` | 178 ms | 179 ms | +0.8% | [-0.1%, +1.6%] | +0.0% | ~ | grid/morton 1.06x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/host` | 257 ms | 259 ms | +0.7% | [+0.6%, +0.8%] | +0.0% | ~ | grid/shuffled 0.74x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/host` | 201 ms | 202 ms | +0.4% | [+0.4%, +0.5%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/host` | 189 ms | 187 ms | -0.8% | [-1.7%, -0.0%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/host` | 292 ms | 292 ms | -0.0% | [-0.9%, +0.8%] | +0.0% | ~ | grid/shuffled 0.69x |
| `Fused/csg/signedDistance/float/host` | 4.50 ms | 4.52 ms | +0.4% | [-0.5%, +1.4%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | tape/fused 2.05x |
| `Fused/chain16/signedDistance/float/host` | 982 µs | 981 µs | -0.1% | [-1.6%, +1.5%] | - | ~ | tape/fused 16.67x |
| `Fused/drilled-mesh/signedDistance/float/host` | 45.9 ms | 46.0 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.8 ms | 46.3 ms | -0.9% | [-1.8%, -0.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.61 ms | 4.58 ms | -0.7% | [-2.5%, +1.2%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | tape/fused 2.08x |
| `Fused/chain16/signedDistance/double/host` | 3.88 ms | 3.87 ms | -0.2% | [-1.0%, +0.5%] | - | ~ | tape/fused 4.32x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.3 ms | 52.5 ms | +0.3% | [-0.3%, +1.0%] | +0.0% | ~ | tape/fused 1.03x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.2 ms | 53.9 ms | +1.4% | [-0.9%, +3.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.7 ms | 82.8 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 230 ms | 228 ms | -0.6% | [-0.8%, -0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 114 ms | 113 ms | -0.7% | [-1.4%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 262 ms | 262 ms | +0.1% | [-0.9%, +1.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.8 ms | 70.6 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 213 ms | -0.7% | [-1.2%, -0.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 97.0 ms | 96.7 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 28.9 ms | 29.1 ms | +0.6% | [+0.0%, +1.1%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 762 ms | 759 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | +0.4% | [+0.2%, +0.7%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.3 ms | 43.3 ms | -2.4% | [-4.6%, -0.1%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 858 ms | 858 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.2 ms | 26.3 ms | +0.4% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.7 ms | 15.7 ms | +0.1% | [-0.7%, +1.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 319 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 231 ms | 231 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.8 ms | +0.4% | [+0.1%, +0.6%] | +0.0% | ~ | tape/fused 3.51x |
| `MeshCSG/smooth-fused/frame/float/host` | 82.1 ms | 81.4 ms | -0.8% | [-0.9%, -0.6%] | +0.0% | ~ | tape/fused 2.84x |
| `MeshCSG/union-tape/firstHit/double/host` | 28.7 ms | 29.0 ms | +1.0% | [+0.9%, +1.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.0 ms | 17.2 ms | +1.3% | [+0.4%, +2.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 360 ms | 357 ms | -1.1% | [-1.5%, -0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 246 ms | 246 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.3 ms | 98.5 ms | +1.1% | [-0.5%, +2.8%] | +0.0% | ~ | tape/fused 3.62x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.1 ms | 87.0 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | tape/fused 2.83x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 230 ms | 229 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 231 ms | 233 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ | signedDistance/closestPoint 0.98x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 307 ms | 308 ms | +0.5% | [-0.4%, +1.4%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 232 ms | 232 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ | closestPoint/gradient 1.00x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 262 ms | 260 ms | -1.0% | [-1.5%, -0.4%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 265 ms | 263 ms | -0.5% | [-1.6%, +0.6%] | +0.0% | ~ | signedDistance/closestPoint 0.99x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 341 ms | 342 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 266 ms | 265 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ | closestPoint/gradient 0.99x |
| `MeshIngest/armadillo/read-soup/float/host` | 53.9 ms | 53.9 ms | -0.1% | [-0.9%, +0.7%] | - | ~ |  |
| `MeshIngest/armadillo/clean/float/host` | 12.4 ms | 12.5 ms | +0.2% | [-0.8%, +1.2%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/float/host` | 95.6 ms | 95.4 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/float/host` | 103 ms | 102 ms | -1.1% | [-2.8%, +0.7%] | - | ~ |  |
| `MeshIngest/armadillo/read-soup/double/host` | 54.5 ms | 54.2 ms | -0.5% | [-1.6%, +0.7%] | - | ~ |  |
| `MeshIngest/armadillo/clean/double/host` | 13.6 ms | 13.5 ms | -0.8% | [-1.7%, +0.1%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/double/host` | 106 ms | 106 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/double/host` | 115 ms | 115 ms | -0.2% | [-1.0%, +0.6%] | - | ~ |  |
| `MeshReconcile/armadillo/reconcile/float/host` | 73.5 ms | 73.2 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.2 ms | 42.3 ms | +2.7% | [+0.5%, +5.0%] | +0.0% | ~ | reconcile/scheduled 1.73x |
| `MeshReconcile/armadillo/reconcile/double/host` | 94.6 ms | 95.0 ms | +0.4% | [-0.5%, +1.3%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.2 ms | 56.8 ms | +1.0% | [+0.5%, +1.4%] | +0.0% | ~ | reconcile/scheduled 1.67x |
| `MeshTape/mesh/firstHit/float/host` | 49.0 ms | 48.8 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 48.9 ms | 48.7 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.7 ms | 19.6 ms | -0.7% | [-1.3%, +0.0%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 80.5 ms | 81.2 ms | +0.9% | [+0.3%, +1.5%] | +0.0% | ~ | mesh/tape 0.60x |
| `MeshTape/tape/firstCrossing/float/host` | 50.8 ms | 50.5 ms | -0.6% | [-1.4%, +0.2%] | +0.0% | ~ | mesh/tape 0.96x |
| `MeshTape/tape/frame/float/host` | 24.9 ms | 25.0 ms | +0.7% | [+0.3%, +1.0%] | +0.0% | ~ | mesh/tape 0.78x |
| `MeshTape/view/firstHit/float/host` | 83.2 ms | 84.1 ms | +1.1% | [+0.1%, +2.0%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/float/host` | 26.0 ms | 26.2 ms | +0.7% | [+0.1%, +1.4%] | +0.0% | ~ | mesh/view 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 53.6 ms | 53.2 ms | -0.7% | [-2.0%, +0.6%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 53.7 ms | 53.0 ms | -1.4% | [-1.8%, -1.1%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.5 ms | 20.6 ms | +0.5% | [-0.1%, +1.2%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 87.1 ms | 88.2 ms | +1.3% | [+0.6%, +2.0%] | +0.0% | ~ | mesh/tape 0.60x |
| `MeshTape/tape/firstCrossing/double/host` | 55.5 ms | 55.7 ms | +0.3% | [-0.8%, +1.5%] | +0.0% | ~ | mesh/tape 0.95x |
| `MeshTape/tape/frame/double/host` | 27.0 ms | 27.2 ms | +0.7% | [-0.5%, +1.9%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 92.7 ms | 91.2 ms | -1.6% | [-2.5%, -0.7%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/double/host` | 29.2 ms | 29.6 ms | +1.5% | [+0.1%, +2.8%] | +0.0% | ~ | mesh/view 0.69x |
| `PointCloudBVH/uniform/build/float/host` | 53.1 ms | 54.0 ms | +1.7% | [+0.9%, +2.4%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 145 ms | 144 ms | -0.4% | [-1.4%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 363 ms | 362 ms | -0.4% | [-1.0%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 81.6 ms | 80.3 ms | -1.5% | [-3.0%, +0.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.74 ms | 6.74 ms | -0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.2 ms | 55.3 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 159 ms | 160 ms | +0.4% | [-0.6%, +1.4%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 393 ms | 392 ms | -0.3% | [-0.7%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.5 ms | 87.5 ms | +0.0% | [-0.9%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.94 ms | 6.96 ms | +0.2% | [-0.6%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.6 ms | 13.9 ms | +2.6% | [-0.5%, +5.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 117 ms | 117 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 476 ms | 476 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 141 ms | 141 ms | -0.2% | [-0.5%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 804 ms | 802 ms | -0.3% | [-0.7%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.1 ms | 14.9 ms | -1.2% | [-3.3%, +1.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 121 ms | 121 ms | -0.2% | [-1.1%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 493 ms | 493 ms | -0.1% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 146 ms | +1.0% | [-0.1%, +2.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 803 ms | 801 ms | -0.3% | [-0.8%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.1 ms | 54.8 ms | +1.4% | [+0.3%, +2.5%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 131 ms | 130 ms | -0.8% | [-2.4%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 271 ms | 269 ms | -0.7% | [-1.4%, -0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 149 ms | 149 ms | -0.2% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 134 ms | 134 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.2 ms | 57.0 ms | -0.2% | [-1.6%, +1.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 142 ms | 141 ms | -1.1% | [-2.3%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 303 ms | 300 ms | -1.0% | [-1.8%, -0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 164 ms | 164 ms | +0.3% | [-0.7%, +1.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 153 ms | 150 ms | -2.0% | [-3.8%, -0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.9 ms | 11.9 ms | +0.3% | [-3.5%, +4.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 115 ms | 116 ms | +0.9% | [+0.2%, +1.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 318 ms | 318 ms | -0.1% | [-0.6%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 157 ms | 156 ms | -0.7% | [-1.7%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 322 ms | 321 ms | -0.4% | [-0.9%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.6 ms | 13.8 ms | +1.5% | [-0.1%, +3.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 116 ms | 116 ms | +0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 348 ms | 349 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 165 ms | 167 ms | +0.9% | [+0.5%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 311 ms | 311 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.3 ms | 35.3 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.2 ms | 38.3 ms | +0.2% | [-0.6%, +1.1%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.68 ms | 8.62 ms | -0.8% | [-6.9%, +5.8%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.27 ms | 9.26 ms | -0.1% | [-1.3%, +1.1%] | +0.0% | ~ |  |
| `QueryOrder/random/mortonOrder/float/host` | 61.3 ms | 60.8 ms | -0.9% | [-1.6%, -0.2%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | 223 ms | 225 ms | +0.6% | [+0.3%, +1.0%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | 161 ms | 163 ms | +1.0% | [-1.0%, +3.0%] | +0.0% | ~ | arrived/ordered 1.38x |
| `QueryOrder/random/mortonOrder/double/host` | 62.4 ms | 62.5 ms | +0.2% | [+0.1%, +0.3%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | 265 ms | 267 ms | +0.7% | [-0.2%, +1.5%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | 170 ms | 171 ms | +0.5% | [-0.2%, +1.2%] | +0.0% | ~ | arrived/ordered 1.56x |
| `Ray/mesh/firstHit/float/host` | 48.7 ms | 48.5 ms | -0.4% | [-0.9%, +0.0%] | +0.0% | ~ | hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled/float/host` | 46.1 ms | 46.4 ms | +0.6% | [-0.9%, +2.0%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.5 ms | 41.0 ms | +1.2% | [+0.5%, +1.8%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 79.3 ms | 78.8 ms | -0.6% | [-2.2%, +1.0%] | +0.0% | ~ | triangles/polygons 0.62x |
| `Ray/mesh-polygons/intersects/float/host` | 66.3 ms | 69.8 ms | +5.3% | [-1.3%, +12.0%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.24 ms | 5.21 ms | -0.5% | [-2.2%, +1.2%] | - | ~ | tape/fused 1.47x |
| `Ray/csg-tape/firstHit/float/host` | 7.69 ms | 7.63 ms | -0.7% | [-1.3%, -0.1%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 53.5 ms | 53.5 ms | -0.1% | [-1.5%, +1.2%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.9 ms | 51.8 ms | -0.1% | [-1.0%, +0.9%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.3 ms | 45.1 ms | -0.5% | [-1.3%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 89.7 ms | 91.3 ms | +1.9% | [-1.3%, +5.1%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/double/host` | 74.7 ms | 74.4 ms | -0.4% | [-2.8%, +2.0%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.56 ms | 5.53 ms | -0.6% | [-1.7%, +0.6%] | - | ~ | tape/fused 1.43x |
| `Ray/csg-tape/firstHit/double/host` | 7.97 ms | 7.91 ms | -0.7% | [-1.5%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.6 ms | 17.1 ms | -2.6% | [-2.8%, -2.4%] | - | ~ | marched/traced 6.96x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 119 ms | -0.3% | [-0.4%, -0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 21.7 ms | -1.4% | [-2.9%, +0.2%] | +0.0% | ~ | marched/traced 6.70x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 145 ms | 145 ms | -0.0% | [-0.8%, +0.7%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.3 ms | 35.3 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.1 ms | 19.1 ms | -0.2% | [-0.8%, +0.3%] | - | ~ | marched/traced 6.62x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 126 ms | -0.0% | [-0.4%, +0.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.8 ms | 23.5 ms | -1.1% | [-2.8%, +0.6%] | +0.0% | ~ | marched/traced 6.41x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 151 ms | 151 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.7 ms | 37.8 ms | +0.3% | [-1.0%, +1.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 48.9 ms | 48.7 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 48.6 ms | 48.6 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.54 ms | 7.52 ms | -0.2% | [-1.2%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.28 ms | 6.12 ms | -2.5% | [-6.6%, +1.8%] | +0.0% | ~ | hit/crossing 1.23x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 179 ms | 180 ms | +0.6% | [-0.3%, +1.6%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 158 ms | 159 ms | +0.2% | [-1.0%, +1.5%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 69.0 ms | 69.4 ms | +0.5% | [-0.7%, +1.7%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.9 ms | 56.8 ms | -0.2% | [-1.6%, +1.1%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 53.6 ms | 53.7 ms | +0.2% | [-1.5%, +1.9%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 53.8 ms | 54.0 ms | +0.3% | [+0.0%, +0.7%] | +0.0% | ~ | hit/crossing 0.99x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.99 ms | 7.97 ms | -0.2% | [-1.3%, +0.9%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.32 ms | 6.35 ms | +0.4% | [-0.5%, +1.3%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 204 ms | 207 ms | +1.5% | [-1.1%, +4.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 181 ms | 186 ms | +2.4% | [+0.6%, +4.2%] | +0.0% | ~ | hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 79.2 ms | 78.1 ms | -1.3% | [-3.4%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.4 ms | 64.5 ms | +0.2% | [-0.9%, +1.3%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ | marched/traced 4.67x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.9 ms | 51.7 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.12 ms | 8.00 ms | -1.5% | [-3.3%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.99 ms | 2.00 ms | +0.5% | [-0.6%, +1.5%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.06 ms | 6.90 ms | -2.3% | [-3.6%, -1.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.8 ms | 11.9 ms | +1.3% | [-0.4%, +2.9%] | +0.0% | ~ | marched/traced 5.29x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.3 ms | 63.0 ms | +2.8% | [-2.1%, +7.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.70 ms | 8.75 ms | +0.6% | [-1.5%, +2.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.05 ms | 2.09 ms | +2.0% | [+1.5%, +2.6%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.38 ms | 7.29 ms | -1.3% | [-1.6%, -1.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 244 ms | 239 ms | -1.8% | [-4.7%, +1.2%] | +0.0% | ~ | marched/traced 1.49x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 356 ms | 357 ms | +0.3% | [-14.0%, +17.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 247 ms | 244 ms | -1.2% | [-8.3%, +6.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 264 ms | 263 ms | -0.4% | [-7.6%, +7.3%] | +0.0% | ~ | marched/traced 1.54x (was 1.48x) |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 392 ms | 405 ms | +3.4% | [-11.3%, +20.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 257 ms | 257 ms | +0.1% | [-3.7%, +4.0%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.43 ms | 7.49 ms | +0.7% | [-0.1%, +1.6%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.7 ms | 12.7 ms | -0.1% | [-2.3%, +2.3%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 271 ms | 271 ms | -0.1% | [-3.7%, +3.5%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.6 ms | 12.6 ms | -0.1% | [-2.4%, +2.4%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 31.2 ms | 30.9 ms | -0.8% | [-5.0%, +3.7%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.4 ms | 25.0 ms | +11.2% | [-4.2%, +27.3%] | +0.0% | ~ | regions/hit 1.24x (was 1.39x) |
| `Regions/particles/region/double/host` | 7.71 ms | 7.72 ms | +0.1% | [-2.0%, +2.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 18.9 ms | 28.1 ms | +48.7% | [-0.5%, +144.6%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 298 ms | 288 ms | -3.5% | [-9.8%, +3.4%] | +0.0% | ~ | one-pass/each 0.10x (was 0.06x) |
| `Regions/particles/domainLabels/double/host` | 13.5 ms | 13.4 ms | -0.5% | [-6.1%, +5.5%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 33.7 ms | 33.5 ms | -0.7% | [-8.8%, +8.1%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 24.7 ms | 24.7 ms | +0.0% | [-6.1%, +6.5%] | +0.0% | ~ | regions/hit 1.36x |
| `Render/csg/frame/float/host` | 13.5 ms | 13.5 ms | +0.2% | [-6.3%, +7.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 13.1 ms | 13.2 ms | +1.1% | [-1.4%, +8.6%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.8 ms | 20.7 ms | -0.4% | [-11.2%, +11.6%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 18.9 ms | 14.4 ms | -23.6% | [-40.6%, +4.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.7 ms | 14.2 ms | -3.3% | [-10.4%, +4.6%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.9 ms | 21.8 ms | -0.5% | [-12.8%, +13.7%] | +0.0% | ~ |  |
| `ShapeFamilies/icosahedron/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/pyramid/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/box-frame/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/hyperboloid/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/paraboloid/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/superellipsoid/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/solid-angle/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/vesica/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/helix/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/gyroid/signedDistance/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/ray-forms/firstHit/float/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/icosahedron/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/pyramid/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/box-frame/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/hyperboloid/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/paraboloid/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/superellipsoid/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/solid-angle/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/vesica/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/helix/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/gyroid/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/ray-forms/firstHit/double/host` | - | - | - | - | - | new |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 33.9 ms | 33.7 ms | -0.6% | [-4.7%, +3.7%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 5.05 ms | 5.06 ms | +0.2% | [-1.8%, +2.2%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.23 ms | 4.24 ms | +0.1% | [-1.7%, +2.0%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.6 ms | 17.6 ms | +0.3% | [-3.5%, +4.3%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.41 ms | 4.35 ms | -1.3% | [-2.2%, -0.5%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.13 ms | 4.16 ms | +0.8% | [-1.2%, +2.9%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.29 ms | 2.33 ms | +1.5% | [-3.0%, +6.2%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.46 ms | 1.46 ms | +0.5% | [-1.3%, +2.4%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.07 ms | 2.09 ms | +1.4% | [+0.2%, +2.7%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.7 ms | 28.1 ms | +1.2% | [-2.0%, +4.5%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 47.4 ms | 47.3 ms | -0.2% | [-3.9%, +3.6%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.48 ms | 5.48 ms | +0.0% | [-2.4%, +2.5%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.53 ms | 4.50 ms | -0.6% | [-3.2%, +2.1%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 23.0 ms | 22.9 ms | -0.3% | [-4.8%, +4.4%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.65 ms | 4.62 ms | -0.6% | [-3.5%, +2.3%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.27 ms | 5.39 ms | +2.2% | [-1.2%, +5.7%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.54 ms | 2.58 ms | +1.6% | [-2.9%, +6.2%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.80 ms | 1.99 ms | +10.5% | [-3.5%, +25.0%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.12 ms | 2.09 ms | -1.2% | [-2.9%, +0.5%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 30.9 ms | 30.8 ms | -0.4% | [-4.8%, +4.2%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 59.3 ms | 59.5 ms | +0.4% | [-7.1%, +8.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 38.1 ms | 38.4 ms | +0.8% | [-8.0%, +10.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 99.2 ms | 103 ms | +4.0% | [-8.7%, +18.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 66.3 ms | 66.0 ms | -0.5% | [-7.7%, +7.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 43.6 ms | 44.2 ms | +1.3% | [-8.2%, +11.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 119 ms | 120 ms | +0.8% | [-16.2%, +21.4%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 79.1 ms | 81.2 ms | +2.6% | [-9.8%, +16.3%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 312 ms | 317 ms | +1.5% | [-4.9%, +8.1%] | +0.0% | ~ | tape/fused 0.98x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 311 ms | 310 ms | -0.4% | [-8.1%, +8.1%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 93.9 ms | 93.7 ms | -0.2% | [-12.8%, +14.3%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 371 ms | 371 ms | -0.0% | [-10.7%, +11.9%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 374 ms | 372 ms | -0.6% | [-9.9%, +9.5%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 173 ms | 170 ms | -2.2% | [-6.9%, +3.0%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 202 ms | 205 ms | +1.7% | [-4.2%, +8.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 147 ms | 146 ms | -0.7% | [-3.3%, +1.9%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 90.7 ms | 91.6 ms | +1.0% | [-1.7%, +3.7%] | +0.0% | ~ | surface/leaf 1.59x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 135 ms | 134 ms | -1.0% | [-6.2%, +4.7%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 74.7 ms | 74.5 ms | -0.3% | [-5.0%, +4.5%] | +0.0% | ~ | surface/leaf 1.79x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 63.4 ms | 63.3 ms | -0.1% | [-4.8%, +4.7%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 35.4 ms | 35.3 ms | -0.4% | [-5.1%, +4.5%] | +0.0% | ~ | surface/hit 1.79x |
| `SurfaceParts/boxes/closestSurface/double/host` | 146 ms | 145 ms | -0.5% | [-2.0%, +1.0%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 91.6 ms | 92.1 ms | +0.6% | [-1.1%, +2.4%] | +0.0% | ~ | surface/leaf 1.58x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 150 ms | 151 ms | +0.4% | [-5.2%, +6.3%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 84.1 ms | 87.1 ms | +3.6% | [-1.8%, +9.2%] | +0.0% | ~ | surface/leaf 1.73x (was 1.78x) |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 73.5 ms | 74.5 ms | +1.3% | [-1.2%, +3.7%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 41.5 ms | 41.1 ms | -1.0% | [-7.3%, +5.9%] | +0.0% | ~ | surface/hit 1.81x |
| `Tape/csg/signedDistance/float/host` | 13.0 ms | 13.0 ms | -0.3% | [-2.1%, +1.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.66 ms | 4.65 ms | -0.2% | [-1.0%, +0.7%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.5 ms | 25.2 ms | -1.1% | [-4.1%, +2.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.6 ms | 40.7 ms | +0.1% | [-3.4%, +3.7%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.77 ms | 1.79 ms | +0.9% | [-0.1%, +1.8%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 133 µs | 133 µs | +0.4% | [-8.5%, +10.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.9 ms | 30.3 ms | -1.9% | [-3.8%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.73 ms | 5.82 ms | +1.6% | [+0.3%, +2.9%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.5 ms | 16.4 ms | -1.0% | [-3.8%, +2.0%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.70 ms | 4.71 ms | +0.2% | [-1.2%, +1.6%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.3 ms | 13.2 ms | -1.0% | [-4.3%, +2.5%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.62 ms | 4.64 ms | +0.5% | [-0.8%, +1.7%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.7 ms | 26.6 ms | -0.3% | [-3.7%, +3.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.7 ms | 42.7 ms | +0.1% | [-2.3%, +2.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.81 ms | 1.76 ms | -2.5% | [-3.1%, -1.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 295 µs | 294 µs | -0.3% | [-2.0%, +1.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.3 ms | 30.4 ms | +0.3% | [-2.5%, +3.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.81 ms | 7.81 ms | -0.0% | [-2.0%, +2.0%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.4 ms | 16.7 ms | +1.6% | [-1.7%, +4.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.84 ms | 7.76 ms | -1.1% | [-1.8%, -0.3%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 42.7 ms | 42.6 ms | -0.1% | [-2.1%, +2.0%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 205 ms | 220 ms | +7.7% | [+6.3%, +8.9%] | +0.0% | minor |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 43.2 ms | 44.4 ms | +2.8% | [+1.7%, +3.9%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 219 ms | 216 ms | -1.4% | [-2.8%, -0.0%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 765.0 s → 796.5 s (+4.1%), the largest peak 1417 MB → 1616 MB; 6 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.8 s | 9.7 s | -0.6% | 563 MB | 616 MB | +9.5% | ~ |
| `BVHRefit` | 4.3 s | 4.4 s | +0.5% | 503 MB | 513 MB | +1.9% | ~ |
| `BVHUnion` | 19.4 s | 19.2 s | -1.0% | 956 MB | 941 MB | -1.5% | ~ |
| `BVHUnionMixed` | 7.7 s | 7.8 s | +0.9% | 559 MB | 588 MB | +5.2% | ~ |
| `ClosestPrimitive` | 30.6 s | 32.4 s | +5.8% | 1348 MB | 1316 MB | -2.4% | ~ |
| `ClosestSurface` | 28.2 s | 29.6 s | +4.9% | 1318 MB | 1314 MB | -0.3% | ~ |
| `ConnectedComponents` | 15.4 s | 16.5 s | +6.9% | 854 MB | 866 MB | +1.4% | ~ |
| `Domain` | 18.1 s | 19.1 s | +5.1% | 860 MB | 1008 MB | +17.1% | **minor** |
| `DomainObject` | 16.0 s | 17.0 s | +5.8% | 860 MB | 897 MB | +4.2% | ~ |
| `EBGrid` | 14.6 s | 14.4 s | -1.2% | 784 MB | 790 MB | +0.7% | ~ |
| `Fused` | 24.8 s | 25.5 s | +3.0% | 1236 MB | 1289 MB | +4.3% | ~ |
| `LaunchTuning` | 37.5 s | 39.2 s | +4.3% | 1328 MB | 1329 MB | +0.1% | ~ |
| `Mesh` | 16.1 s | 15.7 s | -2.4% | 791 MB | 832 MB | +5.2% | ~ |
| `MeshCSG` | 40.1 s | 41.6 s | +3.7% | 1306 MB | 1326 MB | +1.5% | ~ |
| `MeshClosestPoint` | 14.3 s | 14.2 s | -0.3% | 787 MB | 831 MB | +5.6% | ~ |
| `MeshIngest` | 7.0 s | 6.9 s | -0.8% | 569 MB | 626 MB | +10.1% | ~ |
| `MeshReconcile` | 7.0 s | 6.8 s | -3.6% | 569 MB | 595 MB | +4.6% | ~ |
| `MeshTape` | 41.9 s | 44.1 s | +5.2% | 1261 MB | 1342 MB | +6.4% | ~ |
| `PointCloud` | 7.5 s | 7.6 s | +0.8% | 562 MB | 592 MB | +5.3% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.9 s | +2.4% | 477 MB | 477 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | -1.5% | 457 MB | 452 MB | -1.0% | ~ |
| `QueryOrder` | 11.3 s | 11.5 s | +1.7% | 708 MB | 721 MB | +1.9% | ~ |
| `Ray` | 36.7 s | 38.5 s | +4.9% | 1342 MB | 1335 MB | -0.5% | ~ |
| `RayCSG` | 32.2 s | 33.8 s | +5.0% | 1204 MB | 1273 MB | +5.7% | ~ |
| `RayCrossing` | 45.8 s | 47.7 s | +4.2% | 1360 MB | 1459 MB | +7.2% | ~ |
| `RayParticles` | 49.5 s | 52.4 s | +5.9% | 1334 MB | 1476 MB | +10.6% | **minor** |
| `RayShell` | 37.6 s | 39.6 s | +5.3% | 1312 MB | 1324 MB | +0.9% | ~ |
| `Regions` | 29.8 s | 32.0 s | +7.4% | 1116 MB | 1257 MB | +12.7% | **minor** |
| `Render` | 35.9 s | 38.4 s | +6.7% | 1300 MB | 1326 MB | +2.0% | ~ |
| `ShapeFamilies` | - | 8.6 s | | - | 588 MB | | new |
| `Shapes` | 8.8 s | 8.9 s | +1.3% | 559 MB | 589 MB | +5.3% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.8 s | +0.1% | 518 MB | 530 MB | +2.4% | ~ |
| `SphereCloudRays` | 28.4 s | 29.8 s | +4.9% | 1078 MB | 1217 MB | +12.9% | **minor** |
| `SphereContacts` | 4.3 s | 4.3 s | +1.7% | 490 MB | 496 MB | +1.2% | ~ |
| `SurfaceParts` | 40.8 s | 43.1 s | +5.5% | 1417 MB | 1616 MB | +14.0% | **minor** |
| `Tape` | 16.8 s | 17.4 s | +3.6% | 838 MB | 950 MB | +13.3% | **minor** |
| `TapeBVHUnion` | 14.7 s | 15.6 s | +6.4% | 839 MB | 837 MB | -0.2% | ~ |
| **all, in both** | 765.0 s | 796.5 s | +4.1% | 1417 MB | 1616 MB | +14.0% | **minor** |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (4 cases): ClosestPrimitive/csg/closestPrimitive/float/host, Render/csg/frame-orthographic/float/host, SurfaceParts/mesh-groups/firstSurface/double/host, TapeBVHUnion/maps64-as-written/signedDistance/float/host

The probes differ between the sides (each side ran its own): Performance/ShapeFamilies/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 344147cc
base-fingerprint: 1e0f461958307254a04eb0a9ea2890c216b99160
head: 8f68163a
head-fingerprint: 79251e22a16b68e7b8cfafb786566a9b91ea9555
cases: 438
rounds: 2
reps: 3
date: 2026-11-26
-->

## Device comparison (CUDA): red

**Accepted:** Accept as borderline acceptable regression in order to support the new shapes.

`8f68163a` against `344147cc` (origin/main), 2026-11-26.

**VERDICT: MAJOR regression in 2 cases; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 2 major, 15 minor, 9 faster, 0 differ, 0 lost, 438 cases

- base: 344147cc (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8f68163a (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.60 ms | 1.61 ms | +0.5% | [-0.1%, +1.0%] | - | ~ | device 22.8x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.42 ms | 3.42 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.06 ms | 1.07 ms | +1.6% | [-2.0%, +5.3%] | - | ~ | device 19.3x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.30 ms | 2.31 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 16.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.28 ms | 6.23 ms | -0.8% | [-3.2%, +1.5%] | - | ~ | device 7.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 12.0 ms | 11.8 ms | -1.4% | [-2.2%, -0.6%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.5 ms | 33.5 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 56.9 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.4 ms | 20.7 ms | +1.5% | [+0.5%, +2.6%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.9 ms | 47.9 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 60.7 ms | 61.3 ms | +1.1% | [+0.6%, +1.5%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 106 ms | -3.9% | [-4.6%, -3.2%] | - | faster | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.43 ms | 8.44 ms | +0.1% | [-2.1%, +2.5%] | +0.0% | ~ | device 5.2x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 10.1 ms | 9.85 ms | -2.7% | [-5.0%, -0.3%] | +0.0% | ~ | device 8.5x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 387 µs | 387 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 16.2x faster than the host (was 15.2x) |
| `BVHUnion/spheres/query-random/float/hostdevice` | 843 µs | 844 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.51 ms | 4.52 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.65 ms | 4.65 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 585 µs | 584 µs | -0.2% | [-0.6%, +0.3%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.38 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.76 ms | 6.77 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.03 ms | 7.02 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 902 µs | 902 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.23 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.50 ms | 9.49 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 848 µs | 849 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 13.5x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.07 ms | 2.07 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 8.0x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.09 ms | 9.07 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.82 ms | 9.84 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 308 µs | 319 µs | +3.6% | [+3.2%, +3.8%] | - | minor | device 11.6x faster than the host (was 12.4x) |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.35 ms | 1.41 ms | +4.2% | [+4.0%, +4.3%] | - | minor | device 6.0x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 59.4 ms | +0.9% | [+0.7%, +1.1%] | - | ~ | device 3.4x faster than the host (was 3.6x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.43 ms | 5.45 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 522 ms | 518 ms | -0.8% | [-1.9%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.3 ms | 20.5 ms | +0.7% | [+0.6%, +0.9%] | - | ~ | device 4.5x faster than the host (was 4.2x) |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.39 ms | 8.47 ms | +1.0% | [+0.9%, +1.1%] | - | ~ | device 4.7x faster than the host; surface/leaf 2.41x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.5 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 8.6x faster than the host (was 9.8x) |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.18 ms | 6.23 ms | +0.8% | [+0.7%, +0.9%] | - | ~ | device 10.1x faster than the host; surface/leaf 2.49x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 145 ms | 147 ms | +1.1% | [-0.3%, +2.6%] | - | ~ | device 0.6x faster than the host (was 0.5x) |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.0 ms | 60.0 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.7x faster than the host (was 0.8x); surface/leaf 2.45x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 71.6 ms | 71.9 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 1.8x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.3x faster than the host; surface/leaf 2.57x |
| `Domain/particles/labels/float/hostdevice` | 4.04 ms | 4.05 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 3.5x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.76 ms | 2.90 ms | +5.2% | [+4.9%, +5.5%] | - | minor | device 2.6x faster than the host (was 2.7x) |
| `Domain/particles/surfaceGroups/float/hostdevice` | 114 µs | 116 µs | +1.1% | [-0.0%, +2.2%] | - | ~ | device 15.2x faster than the host (was 13.4x) |
| `Domain/particles/labels/double/hostdevice` | 7.05 ms | 7.60 ms | +7.7% | [+6.7%, +9.3%] | - | minor | device 1.9x faster than the host (was 2.1x) |
| `Domain/particles/labelAt/double/hostdevice` | 3.50 ms | 3.26 ms | -6.9% | [-7.6%, -6.2%] | - | faster | device 2.2x faster than the host (was 2.1x) |
| `Domain/particles/surfaceGroups/double/hostdevice` | 117 µs | 115 µs | -1.5% | [-5.0%, +2.1%] | - | ~ | device 13.0x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 13.2 ms | 14.0 ms | +6.6% | [+6.3%, +6.9%] | - | minor | device 3.3x faster than the host (was 3.6x) |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.89 ms | 9.47 ms | +6.6% | [+6.3%, +6.9%] | - | minor | device 2.6x faster than the host (was 2.7x) |
| `DomainObject/partsOwn/labels/float/hostdevice` | 266 µs | 277 µs | +4.4% | [+3.6%, +4.8%] | - | minor | device 2.3x faster than the host (was 2.5x); whole/own 50.65x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 158 µs | 170 µs | +7.3% | [+6.8%, +8.0%] | - | minor | device 2.0x faster than the host (was 2.3x); whole/own 55.74x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 19.4 ms | 19.7 ms | +1.7% | [+1.6%, +1.9%] | - | ~ | device 2.4x faster than the host (was 2.7x) |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 11.4 ms | 10.7 ms | -5.8% | [-6.2%, -5.3%] | - | faster | device 2.3x faster than the host (was 2.2x) |
| `DomainObject/partsOwn/labels/double/hostdevice` | 393 µs | 391 µs | -0.5% | [-1.7%, +0.8%] | - | ~ | device 1.8x faster than the host (was 1.7x); whole/own 50.43x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 238 µs | 234 µs | -1.5% | [-2.4%, -0.6%] | - | ~ | device 1.5x faster than the host (was 1.7x); whole/own 45.73x (was 47.81x) |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/hostdevice` | 10.4 ms | 10.6 ms | +1.6% | [+0.9%, +2.3%] | +0.0% | ~ | device 9.9x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/hostdevice` | 10.3 ms | 10.3 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ | device 9.5x faster than the host (was 10.4x); grid/morton 1.03x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/hostdevice` | 17.5 ms | 17.4 ms | -0.7% | [-1.5%, +0.2%] | +0.0% | ~ | device 8.9x faster than the host (was 8.0x); grid/shuffled 0.61x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/hostdevice` | 231 ms | 225 ms | -2.8% | [-6.8%, +1.4%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/hostdevice` | 166 ms | 165 ms | -1.1% | [-2.5%, +0.2%] | +0.0% | ~ | device 0.7x faster than the host (was 0.6x); grid/morton 1.37x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/hostdevice` | 291 ms | 289 ms | -0.4% | [-4.2%, +3.5%] | +0.0% | ~ | device 0.4x faster than the host (was 0.5x); grid/shuffled 0.78x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/hostdevice` | 27.0 ms | 27.1 ms | +0.5% | [+0.0%, +1.0%] | +0.0% | ~ | device 4.1x faster than the host (was 4.5x) |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/hostdevice` | 29.3 ms | 29.3 ms | -0.1% | [-1.0%, +0.9%] | +0.0% | ~ | device 3.5x faster than the host; grid/morton 0.93x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/hostdevice` | 60.6 ms | 60.6 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 2.7x faster than the host (was 2.6x); grid/shuffled 0.45x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/hostdevice` | 204 ms | 203 ms | -0.6% | [-2.4%, +1.2%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/hostdevice` | 156 ms | 157 ms | +0.8% | [-1.6%, +3.4%] | +0.0% | ~ | device 0.6x faster than the host; grid/morton 1.29x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/hostdevice` | 247 ms | 254 ms | +2.6% | [+0.6%, +4.6%] | +0.0% | ~ | device 0.7x faster than the host; grid/shuffled 0.80x (was 0.83x) |
| `Fused/csg/signedDistance/float/hostdevice` | 48.1 µs | 47.9 µs | -0.6% | [-0.8%, -0.3%] | - | ~ | device 24.0x faster than the host; tape/fused 4.78x (was 4.54x) |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.9 µs | 49.8 µs | -0.3% | [-0.6%, -0.0%] | - | ~ | device 43.3x faster than the host; tape/fused 7.65x (was 7.19x) |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.6 µs | 47.8 µs | +0.4% | [+0.2%, +0.7%] | - | ~ | device 9.4x faster than the host (was 8.9x); tape/fused 10.69x (was 10.01x) |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.4 ms | 17.4 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 6.8x faster than the host; tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.2 ms | 31.4 ms | +0.7% | [+0.6%, +0.7%] | - | ~ | device 3.8x faster than the host (was 4.8x) |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 281 ms | 280 ms | -0.4% | [-2.5%, +1.8%] | - | ~ | device 0.4x faster than the host; tape/fused 1.02x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 286 ms | 285 ms | -0.3% | [-1.2%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.22 ms | 8.29 ms | +0.8% | [-0.2%, +1.9%] | +0.0% | ~ | device 10.8x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.31 ms | 8.32 ms | +0.1% | [-0.6%, +0.9%] | +0.0% | ~ | device 10.9x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.23 ms | 8.21 ms | -0.3% | [-0.9%, +0.4%] | +0.0% | ~ | device 12.6x faster than the host (was 10.9x); default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.18 ms | 8.28 ms | +1.2% | [-0.3%, +2.7%] | +0.0% | ~ | device 10.9x faster than the host (was 12.3x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 708 µs | 680 µs | -4.0% | [-4.2%, -3.8%] | +0.0% | faster | device 3.8x faster than the host (was 3.7x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.78 ms | 1.75 ms | -1.6% | [-1.9%, -1.4%] | +0.0% | ~ | device 1.5x faster than the host (was 1.5x); default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.03 ms | +0.8% | [+0.3%, +1.4%] | +0.0% | ~ | device 2.6x faster than the host; default/this 0.66x (was 0.69x) |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 800 µs | 787 µs | -1.7% | [-2.4%, -0.9%] | +0.0% | ~ | device 3.3x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 748 µs | 745 µs | -0.4% | [-1.4%, +0.6%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 830 µs | 837 µs | +0.8% | [-0.3%, +1.9%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 741 µs | 740 µs | -0.1% | [-1.2%, +0.9%] | +0.0% | ~ | device 12.2x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 725 µs | 726 µs | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ | device 12.2x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 160 ms | -0.3% | [-2.2%, +1.6%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 161 ms | 160 ms | -0.2% | [-1.4%, +1.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 162 ms | 163 ms | +0.6% | [-1.9%, +3.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 162 ms | 162 ms | +0.1% | [-1.2%, +1.4%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | +0.4% | [+0.0%, +0.7%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.45 ms | 2.80 ms | +92.8% | [+92.2%, +93.4%] | +0.0% | **MAJOR** | device 1.0x faster than the host (was 1.9x); default/this 0.50x (was 0.96x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.92 ms | 1.91 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.73x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.56 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.24 ms | 7.25 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.31 ms | 7.30 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.26 ms | 7.22 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.26 ms | 7.32 ms | +0.8% | [+0.8%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.45 ms | 8.36 ms | -1.1% | [-1.8%, -0.3%] | - | ~ | device 11.5x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.8 ms | 16.7 ms | -0.2% | [-1.1%, +0.8%] | - | ~ | device 7.6x faster than the host (was 8.7x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 167 ms | 166 ms | -0.3% | [-3.3%, +2.9%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 283 ms | 279 ms | -1.5% | [-3.1%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.4 ms | 24.5 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 4.1x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.0 ms | 58.1 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 159 ms | 160 ms | +0.6% | [-0.7%, +1.9%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `MeshSDF/query-random/double/hostdevice` | 244 ms | 241 ms | -1.1% | [-4.4%, +2.3%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.05 ms | 2.06 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 5.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.44 ms | 3.44 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.15 ms | 6.18 ms | +0.5% | [-0.1%, +1.2%] | - | ~ | device 4.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 65.2 ms | 64.4 ms | -1.2% | [-2.3%, -0.1%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 96.0 ms | 93.9 ms | -2.2% | [-3.2%, -1.2%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 243 ms | 239 ms | -1.8% | [-3.4%, -0.2%] | - | ~ | device 3.6x faster than the host (was 3.7x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.5 ms | 24.8 ms | +1.1% | [+0.1%, +2.2%] | - | ~ | device 2.0x faster than the host; tape/fused 2.59x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.1 ms | 27.0 ms | -0.2% | [-2.2%, +1.9%] | - | ~ | device 2.2x faster than the host; tape/fused 3.48x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.8 ms | 65.3 ms | -0.7% | [-2.0%, +0.7%] | - | ~ | device 4.6x faster than the host (was 4.0x); tape/fused 3.65x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.5 ms | 22.1 ms | +2.9% | [-5.8%, +12.3%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 38.9 ms | 38.8 ms | -0.3% | [-1.2%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 42.0 ms | 42.5 ms | +1.0% | [-1.8%, +3.9%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 545 ms | 542 ms | -0.5% | [-3.4%, +2.5%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 849 ms | 857 ms | +0.9% | [+0.2%, +1.7%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.94 s | 1.97 s | +1.6% | [+0.4%, +2.7%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 209 ms | 197 ms | -5.7% | [-11.0%, -0.3%] | - | faster | device 0.3x faster than the host (was 0.2x); tape/fused 2.75x (was 2.60x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 308 ms | 306 ms | -0.5% | [-3.7%, +2.8%] | - | ~ | device 0.2x faster than the host (was 0.2x); tape/fused 2.80x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 560 ms | 556 ms | -0.7% | [-3.9%, +2.6%] | - | ~ | device 0.5x faster than the host; tape/fused 3.55x |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 12.0 ms | 12.0 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 11.1x faster than the host (was 10.0x) |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 58.5 ms | 58.5 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.5x faster than the host |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 257 ms | 259 ms | +0.6% | [-1.7%, +3.0%] | - | ~ | device 0.5x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 244 ms | 245 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | device 16.4x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.32 ms | 5.32 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 11.1x faster than the host |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.32 ms | 1.32 ms | -0.0% | [-1.2%, +1.1%] | - | ~ | device 15.8x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.32 ms | 1.31 ms | -0.5% | [-0.9%, -0.2%] | - | ~ | device 16.1x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.14 ms | 2.14 ms | +0.2% | [-0.9%, +1.2%] | - | ~ | device 7.9x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.52 ms | 3.52 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 10.3x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.77 ms | 2.77 ms | +0.2% | [-0.0%, +0.3%] | - | ~ | device 7.9x faster than the host; mesh/tape 0.47x |
| `MeshTape/tape/frame/float/hostdevice` | 4.39 ms | 4.49 ms | +2.1% | [+1.9%, +2.3%] | - | ~ | device 4.9x faster than the host; mesh/tape 0.48x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.73 ms | 3.77 ms | +0.8% | [+0.7%, +0.9%] | - | ~ | device 9.8x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.63 ms | 5.68 ms | +0.9% | [+0.5%, +1.3%] | - | ~ | device 4.2x faster than the host; mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 30.3 ms | 30.2 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 0.7x faster than the host (was 0.9x) |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 30.1 ms | 30.1 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 25.8 ms | 25.9 ms | +0.4% | [-0.1%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 43.4 ms | 43.6 ms | +0.4% | [+0.1%, +0.8%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.69x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 30.7 ms | 30.4 ms | -1.0% | [-1.2%, -0.7%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 32.8 ms | 33.1 ms | +0.7% | [+0.5%, +1.0%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.78x |
| `MeshTape/view/firstHit/double/hostdevice` | 43.4 ms | 43.4 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 0.9x faster than the host; mesh/view 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 36.0 ms | 36.3 ms | +0.9% | [+0.2%, +1.6%] | - | ~ | device 0.7x faster than the host; mesh/view 0.71x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 681 µs | 679 µs | -0.3% | [-0.8%, +0.2%] | - | ~ | device 11.0x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.83 ms | 1.83 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 995 µs | 997 µs | +0.2% | [-0.2%, +0.6%] | - | ~ | device 9.4x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.59 ms | 4.61 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.97 ms | 2.96 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.09 ms | 7.08 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.79 ms | 7.82 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.46 ms | 1.48 ms | +1.4% | [-0.5%, +3.2%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.88 ms | 3.88 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 524 µs | 524 µs | +0.1% | [-0.1%, +0.4%] | - | ~ | device 15.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.55 ms | 4.52 ms | -0.6% | [-1.4%, +0.2%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.21 ms | 4.23 ms | +0.5% | [-3.3%, +4.4%] | - | ~ | device 5.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.12 ms | 8.14 ms | +0.3% | [-0.4%, +1.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.8 ms | -0.3% | [-1.1%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.01 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.5 ms | 17.6 ms | +0.4% | [-0.1%, +0.8%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.9% | [-0.6%, +2.4%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.57 ms | 1.59 ms | +1.3% | [+0.3%, +2.3%] | - | ~ | device 10.4x faster than the host (was 10.7x) |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.61 ms | 4.60 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 834 µs | 836 µs | +0.3% | [-0.3%, +0.8%] | - | ~ | device 10.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.86 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.13 ms | 4.11 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.5 ms | 16.5 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.0 ms | 21.1 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.93 ms | 8.93 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.5 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.1 ms | 20.2 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.75 ms | 2.79 ms | +1.2% | [+0.3%, +2.0%] | - | ~ | device 4.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.73 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.39 ms | 1.38 ms | -0.7% | [-1.4%, -0.1%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.71 ms | 2.71 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 7.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.47 ms | 6.37 ms | -1.6% | [-2.8%, -0.4%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.8 ms | 33.6 ms | -0.7% | [-1.7%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.3 ms | 68.5 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.3 ms | 18.5 ms | +0.7% | [+0.3%, +1.0%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.7 ms | 20.8 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.2 ms | 38.0 ms | -0.6% | [-2.0%, +0.8%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.64 ms | 9.67 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.9 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.27 ms | 3.26 ms | -0.3% | [-1.3%, +0.6%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.61 ms | 2.58 ms | -1.2% | [-1.2%, -1.1%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.56 ms | 4.56 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.07 ms | 3.07 ms | -0.1% | [-1.1%, +1.0%] | +0.0% | ~ | device 3.3x faster than the host |
| `QueryOrder/random/mortonOrder/float/hostdevice` | 16.9 ms | 16.9 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 3.5x faster than the host |
| `QueryOrder/armadillo/distance-arrived/float/hostdevice` | 9.13 ms | 9.13 ms | +0.1% | [-0.8%, +0.9%] | +0.0% | ~ | device 4.9x faster than the host |
| `QueryOrder/armadillo/distance-ordered/float/hostdevice` | 7.35 ms | 7.37 ms | +0.2% | [-1.0%, +1.5%] | +0.0% | ~ | device 5.4x faster than the host; arrived/ordered 1.24x |
| `QueryOrder/random/mortonOrder/double/hostdevice` | 17.6 ms | 17.5 ms | -0.4% | [-0.8%, -0.0%] | - | ~ | device 3.5x faster than the host |
| `QueryOrder/armadillo/distance-arrived/double/hostdevice` | 113 ms | 112 ms | -1.2% | [-3.4%, +1.0%] | +0.0% | ~ | device 0.5x faster than the host (was 0.4x) |
| `QueryOrder/armadillo/distance-ordered/double/hostdevice` | 79.9 ms | 79.4 ms | -0.6% | [-2.6%, +1.4%] | +0.0% | ~ | device 0.5x faster than the host; arrived/ordered 1.40x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 719 µs | 707 µs | -1.6% | [-5.0%, +1.9%] | - | ~ | device 17.4x faster than the host; hand/api 0.92x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.32 ms | 1.32 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 15.2x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 659 µs | 649 µs | -1.5% | [-2.0%, -1.1%] | - | ~ | device 17.5x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.23 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 15.3x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 627 µs | 630 µs | +0.5% | [-0.6%, +1.5%] | - | ~ | device 15.7x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.16 ms | 1.16 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 14.5x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.20 ms | 1.19 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 11.9x faster than the host; triangles/polygons 0.59x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.49 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 9.6x faster than the host; triangles/polygons 0.53x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.7% | [+0.5%, +1.0%] | - | ~ | device 11.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.17 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 9.9x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 609 µs | 609 µs | -0.1% | [-5.9%, +6.1%] | - | ~ | device 2.5x faster than the host (was 2.5x); tape/fused 1.13x (was 1.17x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 728 µs | 693 µs | -4.9% | [-9.8%, +0.0%] | - | ~ | device 2.6x faster than the host (was 2.5x); tape/fused 1.22x (was 1.17x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 710 µs | 687 µs | -3.2% | [-3.9%, -2.5%] | - | faster | device 3.6x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 850 µs | 844 µs | -0.7% | [-0.8%, -0.6%] | - | ~ | device 3.5x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 18.7 ms | 18.7 ms | +0.3% | [-1.0%, +1.6%] | - | ~ | device 0.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 30.3 ms | 30.2 ms | -0.6% | [-1.2%, +0.1%] | - | ~ | device 0.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 18.0 ms | 18.1 ms | +0.5% | [+0.4%, +0.6%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 29.2 ms | 29.2 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.9 ms | 15.8 ms | -0.9% | [-1.5%, -0.2%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 26.1 ms | 25.9 ms | -0.7% | [-1.3%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 17.2 ms | 17.2 ms | +0.1% | [-1.2%, +1.4%] | - | ~ | device 0.8x faster than the host; triangles/polygons 1.09x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.5 ms | 25.5 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.18x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.1 ms | +1.0% | [-1.0%, +2.9%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 22.9 ms | -0.4% | [-1.3%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.24 ms | 1.25 ms | +0.5% | [-0.1%, +1.2%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.38 ms | 2.36 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 0.8x faster than the host; tape/fused 1.06x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.50 ms | +0.2% | [-0.7%, +1.0%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.53 ms | 2.53 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.9x faster than the host (was 2.5x); marched/traced 3.56x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.98 ms | 2.98 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.7x faster than the host (was 2.3x); marched/traced 4.65x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.00 ms | 9.01 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 4.2x faster than the host (was 5.0x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 3.1x faster than the host (was 3.7x) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.16 ms | 2.21 ms | +2.6% | [+2.4%, +2.8%] | - | ~ | device 3.8x faster than the host; marched/traced 13.47x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.42 ms | 5.56 ms | +2.6% | [+2.4%, +2.8%] | - | ~ | device 2.7x faster than the host; marched/traced 10.16x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 28.9 ms | 29.8 ms | +2.9% | [+2.9%, +3.0%] | - | ~ | device 2.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.0 ms | 56.5 ms | +2.8% | [+2.7%, +2.9%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.24 ms | 5.35 ms | +2.2% | [+1.9%, +2.5%] | - | ~ | device 2.9x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.81 ms | 9.00 ms | +2.1% | [+2.0%, +2.1%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.15 ms | 6.18 ms | +0.5% | [-0.4%, +1.3%] | - | ~ | device 1.3x faster than the host; marched/traced 26.11x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.5 ms | 10.6 ms | +0.8% | [+0.5%, +1.1%] | - | ~ | device 0.8x faster than the host (was 0.8x); marched/traced 24.24x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 161 ms | +1.4% | [+0.2%, +2.5%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 251 ms | 256 ms | +2.2% | [+0.8%, +3.6%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.86 ms | 4.87 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 1.8x faster than the host; marched/traced 22.06x (was 22.75x) |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.3 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 1.3x faster than the host; marched/traced 17.95x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 111 ms | 108 ms | -2.7% | [-3.9%, -1.5%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 216 ms | 221 ms | +2.3% | [+1.3%, +3.3%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.1 ms | -1.3% | [-1.7%, -0.9%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.8 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 707 µs | 706 µs | -0.1% | [-0.5%, +0.2%] | - | ~ | device 17.6x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.31 ms | 1.31 ms | -0.2% | [-1.3%, +0.8%] | - | ~ | device 15.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 708 µs | 705 µs | -0.5% | [-0.8%, -0.1%] | - | ~ | device 17.4x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.31 ms | 1.31 ms | -0.7% | [-0.8%, -0.6%] | - | ~ | device 15.6x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 709 µs | 668 µs | -5.8% | [-6.0%, -5.5%] | - | faster | device 3.9x faster than the host (was 3.7x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 847 µs | 843 µs | -0.4% | [-0.7%, -0.2%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 601 µs | 616 µs | +2.5% | [+2.3%, +2.6%] | - | ~ | device 3.3x faster than the host; hit/crossing 1.08x (was 1.18x) |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 627 µs | 641 µs | +2.1% | [+1.5%, +2.7%] | - | ~ | device 3.7x faster than the host; hit/crossing 1.32x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.76 ms | 4.76 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 12.1x faster than the host (was 10.3x) |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.3 ms | 10.3 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 7.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.23 ms | 4.21 ms | -0.6% | [-0.7%, -0.4%] | - | ~ | device 10.2x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.17 ms | 9.09 ms | -0.9% | [-1.0%, -0.7%] | - | ~ | device 7.0x faster than the host; hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.4 ms | 58.0 ms | +2.9% | [+0.4%, +5.4%] | - | ~ | device 3.0x faster than the host (was 3.5x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 82.4 ms | 83.7 ms | +1.6% | [+0.5%, +2.7%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.3 ms | 48.9 ms | +1.3% | [+0.3%, +2.3%] | - | ~ | device 3.0x faster than the host; hit/crossing 1.19x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.9 ms | 64.4 ms | +0.8% | [+0.4%, +1.1%] | - | ~ | device 3.0x faster than the host (was 2.5x); hit/crossing 1.30x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 18.8 ms | 18.6 ms | -1.0% | [-1.6%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 30.2 ms | 30.3 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 18.8 ms | 18.8 ms | +0.4% | [-0.8%, +1.7%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 30.2 ms | 30.2 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.51 ms | +0.4% | [+0.1%, +0.8%] | - | ~ | device 1.2x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.11 ms | +1.2% | [+0.6%, +1.8%] | - | ~ | device 1.8x faster than the host; hit/crossing 1.25x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.17 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 68.8 ms | 68.9 ms | +0.1% | [-0.6%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 131 ms | 130 ms | -0.7% | [-1.3%, +0.0%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 62.3 ms | 62.1 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.11x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 119 ms | 117 ms | -1.1% | [-3.0%, +0.8%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 452 ms | 453 ms | +0.2% | [-2.1%, +2.5%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 704 ms | 703 ms | -0.2% | [-1.3%, +0.9%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 400 ms | 397 ms | -0.8% | [-6.9%, +6.9%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.14x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 523 ms | 528 ms | +1.0% | [+0.3%, +1.8%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.33x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.59 ms | 3.62 ms | +0.7% | [-1.0%, +2.2%] | - | ~ | device 1.2x faster than the host; marched/traced 3.34x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.81 ms | 4.81 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 0.9x faster than the host; marched/traced 5.55x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.1 ms | +1.0% | [+0.0%, +2.0%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.5 ms | 26.7 ms | +0.6% | [+0.5%, +0.7%] | - | ~ | device 2.1x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.23 ms | -1.0% | [-2.4%, +0.5%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.85 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 96.4 µs | 96.9 µs | +0.5% | [-1.3%, +2.4%] | - | ~ | device 14.9x faster than the host (was 15.6x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 155 µs | 155 µs | -0.2% | [-1.6%, +1.3%] | - | ~ | device 10.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 796 µs | 802 µs | +0.8% | [+0.7%, +0.8%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 935 µs | 934 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.8 ms | 34.7 ms | -0.2% | [-1.9%, +4.7%] | - | ~ | device 0.1x faster than the host; marched/traced 3.51x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.4 ms | 36.7 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 0.1x faster than the host; marched/traced 6.36x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 121 ms | 122 ms | +0.7% | [-0.7%, +2.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 231 ms | 233 ms | +0.7% | [-2.3%, +3.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.9 ms | 16.6 ms | -1.7% | [-7.3%, +4.3%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 21.7 ms | -1.1% | [-3.5%, +1.3%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.11 ms | 1.08 ms | -2.8% | [-8.1%, +2.8%] | - | ~ | device 1.4x faster than the host (was 1.3x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.51 ms | 2.52 ms | +0.6% | [-1.9%, +3.2%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.97 ms | 6.00 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.76 ms | 6.70 ms | -0.9% | [-1.0%, -0.8%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.0 ms | 23.6 ms | +2.6% | [+1.7%, +3.5%] | - | ~ | device 2.1x faster than the host; marched/traced 2.64x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.4 ms | +0.3% | [-0.8%, +1.4%] | - | ~ | device 1.9x faster than the host; marched/traced 3.73x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.2 ms | 62.5 ms | +0.4% | [-0.3%, +1.2%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 105 ms | 106 ms | +1.5% | [+0.5%, +2.4%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.9 ms | 14.1 ms | +1.4% | [-0.4%, +3.3%] | - | ~ | device 3.6x faster than the host (was 3.7x) |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.4 ms | -0.1% | [-1.2%, +1.0%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 311 ms | 300 ms | -3.6% | [-8.8%, +2.1%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 1.89x (was 1.78x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 359 ms | 349 ms | -2.7% | [-5.7%, +0.3%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 2.84x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 555 ms | 567 ms | +2.2% | [+1.5%, +2.9%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 994 ms | 990 ms | -0.5% | [-1.0%, +0.1%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 189 ms | 192 ms | +1.6% | [-0.0%, +3.3%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 238 ms | 235 ms | -1.3% | [-5.1%, +2.7%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | 3.01 ms | 2.90 ms | -3.6% | [-4.3%, -2.9%] | - | faster | device 2.6x faster than the host (was 2.5x) |
| `Regions/particles/regionDistances/float/hostdevice` | 3.66 ms | 3.66 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.5x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 97.8 ms | 89.4 ms | -8.6% | [-9.0%, -8.2%] | - | faster | device 3.3x faster than the host (was 3.0x); one-pass/each 0.04x (was 0.04x) |
| `Regions/particles/domainLabels/float/hostdevice` | 3.73 ms | 3.75 ms | +0.5% | [+0.3%, +0.7%] | - | ~ | device 3.7x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.76 ms | 6.89 ms | +1.8% | [+1.8%, +1.9%] | - | ~ | device 3.6x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.61 ms | 5.56 ms | -0.8% | [-1.0%, -0.6%] | - | ~ | device 3.2x faster than the host; regions/hit 1.24x |
| `Regions/particles/region/double/hostdevice` | 3.43 ms | 3.36 ms | -2.0% | [-2.3%, -1.6%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.81 ms | 5.81 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 119 ms | 133 ms | +11.8% | [+11.1%, +12.7%] | - | **MAJOR** | device 2.3x faster than the host (was 2.6x); one-pass/each 0.04x (was 0.05x) |
| `Regions/particles/domainLabels/double/hostdevice` | 5.89 ms | 5.93 ms | +0.7% | [+0.4%, +1.0%] | - | ~ | device 2.3x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.8 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 12.9 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 1.3x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.46 ms | +3.7% | [+3.5%, +3.9%] | - | minor | device 3.0x faster than the host (was 3.1x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.36 ms | 2.45 ms | +3.8% | [+3.4%, +4.0%] | - | minor | device 3.0x faster than the host (was 3.1x) |
| `Render/mesh/frame/float/hostdevice` | 2.13 ms | 2.13 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 8.0x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.62 ms | 5.67 ms | +0.9% | [+0.7%, +1.1%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.63 ms | 5.66 ms | +0.5% | [+0.4%, +0.6%] | - | ~ | device 1.5x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 25.8 ms | 25.9 ms | +0.5% | [-0.7%, +1.8%] | - | ~ | device 0.7x faster than the host |
| `ShapeFamilies/icosahedron/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/pyramid/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/box-frame/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/hyperboloid/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/paraboloid/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/superellipsoid/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/solid-angle/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/vesica/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/helix/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/gyroid/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/ray-forms/firstHit/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/icosahedron/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/pyramid/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/box-frame/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/hyperboloid/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/paraboloid/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/superellipsoid/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/solid-angle/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/vesica/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/helix/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/gyroid/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/ray-forms/firstHit/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 156 µs | 155 µs | -0.4% | [-1.0%, +0.1%] | - | ~ | device 33.9x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.6 µs | 47.5 µs | -0.3% | [-0.6%, -0.1%] | - | ~ | device 19.3x faster than the host |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.9 µs | 48.1 µs | +0.3% | [+0.2%, +0.5%] | - | ~ | device 15.9x faster than the host (was 15.4x) |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 67.3x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 47.7 µs | 47.9 µs | +0.5% | [-0.1%, +1.1%] | - | ~ | device 16.1x faster than the host (was 16.7x) |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.2% | [-0.7%, +0.2%] | - | ~ | device 17.4x faster than the host (was 18.6x) |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 47.8 µs | 48.0 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 10.1x faster than the host (was 10.8x) |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.3% | [-0.7%, +0.2%] | - | ~ | device 8.4x faster than the host |
| `Shapes/wedge/signedDistance/float/hostdevice` | 47.6 µs | 48.1 µs | +1.2% | [+0.1%, +2.2%] | - | ~ | device 9.5x faster than the host (was 9.1x) |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 556 µs | 556 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 8.7x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.24 ms | 7.23 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 492 µs | 492 µs | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.2x faster than the host |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 421 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.5x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 358 µs | 357 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.4x faster than the host |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 670 µs | 669 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 414 µs | 415 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.4x faster than the host (was 1.3x) |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 339 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `Shapes/wedge/signedDistance/double/hostdevice` | 154 µs | 154 µs | +0.1% | [+0.1%, +0.1%] | - | ~ | device 3.3x faster than the host (was 2.9x) |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.02 ms | 8.01 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 895 µs | 889 µs | -0.7% | [-1.6%, +0.2%] | +0.0% | ~ | device 10.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.66 ms | 2.66 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.3 ms | 14.4 ms | +0.7% | [+0.2%, +1.2%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.1 ms | 11.0 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ | device 4.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.01 ms | 3.00 ms | -0.3% | [-0.7%, -0.0%] | +0.0% | ~ | device 7.4x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.77 ms | 4.77 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 5.3x faster than the host; tape/fused 1.38x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.0 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host (was 4.3x); tape/fused 1.27x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 23.0 ms | 23.0 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.6 ms | 30.6 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.1x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.2 ms | 44.2 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.4 ms | 45.6 ms | +0.4% | [-0.0%, +0.9%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 122 ms | 123 ms | +1.0% | [-0.7%, +2.7%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 126 ms | 127 ms | +0.2% | [-3.0%, +3.5%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.06x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 130 ms | 127 ms | -1.9% | [-3.2%, -0.5%] | +0.0% | ~ | device 0.7x faster than the host (was 1.1x) |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 133 ms | 134 ms | +0.5% | [-2.0%, +3.2%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 11.0x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.1 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ | device 0.6x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.29 ms | 9.30 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 13.1x faster than the host (was 12.6x) |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.91 ms | 5.91 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 11.9x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 98.2 ms | 97.2 ms | -1.0% | [-1.8%, -0.1%] | - | ~ | device 3.8x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.9 ms | 63.3 ms | -0.9% | [-1.0%, -0.8%] | - | ~ | device 3.5x faster than the host (was 3.2x); surface/leaf 1.54x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.7 ms | 29.8 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 5.5x faster than the host (was 5.0x) |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 10.00 ms | 9.98 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 7.3x faster than the host; surface/hit 2.99x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.1 ms | 59.0 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.8x faster than the host (was 2.1x) |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.5x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 781 ms | 782 ms | +0.1% | [-0.4%, +0.7%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 523 ms | 526 ms | +0.5% | [-0.1%, +1.2%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.49x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 313 ms | 316 ms | +0.8% | [-0.7%, +2.3%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 128 ms | 128 ms | +0.2% | [-0.7%, +1.2%] | - | ~ | device 0.6x faster than the host; surface/hit 2.46x |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 229 µs | +4.9% | [+4.6%, +5.1%] | - | minor | device 11.0x faster than the host (was 11.4x) |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 24.4x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 381 µs | +6.0% | [+5.8%, +6.2%] | - | minor | device 12.2x faster than the host (was 12.8x) |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.57 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 9.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 71.9 µs | 74.9 µs | +4.2% | [+3.9%, +4.5%] | - | minor | device 7.2x faster than the host (was 7.7x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 47.7 µs | -0.0% | [-0.3%, +0.2%] | - | ~ | device 5.1x faster than the host (was 4.9x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 652 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 10.8x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.1% | [-0.2%, -0.1%] | - | ~ | device 11.9x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 477 µs | 511 µs | +7.2% | [+7.0%, +7.3%] | - | minor | device 7.2x faster than the host (was 7.8x) |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.3 µs | 48.2 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 26.4x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 858 µs | 859 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 3.2x faster than the host (was 3.1x) |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 812 µs | 810 µs | -0.2% | [-0.4%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.64 ms | 6.64 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 191 µs | +0.2% | [+0.2%, +0.3%] | - | ~ | device 3.0x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.77 ms | 2.78 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.12 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.70 ms | 3.84 ms | +3.6% | [+3.3%, +3.7%] | - | minor | device 10.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.51 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.7 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4677.0 s → 6122.8 s (+30.9%), the largest peak 5452 MB → 6409 MB; 19 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.6 s | 13.9 s | +2.1% | 618 MB | 625 MB | +1.2% | ~ |
| `BVHRefit` | 7.2 s | 7.3 s | +2.5% | 523 MB | 530 MB | +1.4% | ~ |
| `BVHUnion` | 25.4 s | 27.2 s | +6.9% | 1042 MB | 1050 MB | +0.8% | ~ |
| `BVHUnionMixed` | 11.3 s | 11.5 s | +1.2% | 636 MB | 643 MB | +1.1% | ~ |
| `ClosestPrimitive` | 106.2 s | 132.8 s | +25.0% | 1371 MB | 1537 MB | +12.1% | **MAJOR** |
| `ClosestSurface` | 138.5 s | 179.1 s | +29.3% | 1316 MB | 1664 MB | +26.4% | **MAJOR** |
| `ConnectedComponents` | 20.5 s | 22.3 s | +8.8% | 952 MB | 1038 MB | +9.1% | ~ |
| `Domain` | 50.8 s | 64.5 s | +27.0% | 1007 MB | 1112 MB | +10.5% | **MAJOR** |
| `DomainObject` | 49.4 s | 62.7 s | +26.9% | 959 MB | 1047 MB | +9.2% | **MAJOR** |
| `EBGrid` | 19.7 s | 20.4 s | +3.6% | 767 MB | 773 MB | +0.8% | ~ |
| `Fused` | 50.6 s | 59.7 s | +18.0% | 1178 MB | 1267 MB | +7.6% | **minor** |
| `LaunchTuning` | 454.7 s | 573.6 s | +26.2% | 3134 MB | 3876 MB | +23.7% | **MAJOR** |
| `Mesh` | 21.7 s | 22.1 s | +2.1% | 824 MB | 830 MB | +0.8% | ~ |
| `MeshCSG` | 366.9 s | 450.5 s | +22.8% | 5192 MB | 6406 MB | +23.4% | **minor** |
| `MeshClosestPoint` | 19.5 s | 20.5 s | +5.5% | 776 MB | 782 MB | +0.8% | ~ |
| `MeshIngest` | 10.8 s | 10.9 s | +0.9% | 549 MB | 557 MB | +1.5% | ~ |
| `MeshReconcile` | 10.7 s | 10.8 s | +0.8% | 526 MB | 534 MB | +1.6% | ~ |
| `MeshTape` | 650.1 s | 824.3 s | +26.8% | 5452 MB | 6409 MB | +17.6% | **MAJOR** |
| `PointCloud` | 12.6 s | 13.7 s | +8.8% | 577 MB | 586 MB | +1.6% | ~ |
| `PointCloudBVHBuild` | 7.1 s | 7.8 s | +9.9% | 523 MB | 531 MB | +1.5% | ~ |
| `PointCloudHashGridBuild` | 6.5 s | 7.2 s | +10.3% | 523 MB | 531 MB | +1.4% | ~ |
| `QueryOrder` | 16.1 s | 16.6 s | +3.4% | 708 MB | 714 MB | +0.9% | ~ |
| `Ray` | 165.6 s | 205.2 s | +23.9% | 3029 MB | 3775 MB | +24.6% | **minor** |
| `RayCSG` | 200.0 s | 243.3 s | +21.6% | 3368 MB | 4204 MB | +24.8% | **minor** |
| `RayCrossing` | 388.3 s | 465.3 s | +19.8% | 3300 MB | 4081 MB | +23.7% | **minor** |
| `RayParticles` | 422.2 s | 748.2 s | +77.2% | 4847 MB | 5830 MB | +20.3% | **MAJOR** |
| `RayShell` | 243.1 s | 458.7 s | +88.7% | 4603 MB | 5578 MB | +21.2% | **MAJOR** |
| `Regions` | 314.6 s | 390.7 s | +24.2% | 3363 MB | 4094 MB | +21.7% | **minor** |
| `Render` | 237.5 s | 283.0 s | +19.2% | 4920 MB | 6180 MB | +25.6% | **MAJOR** |
| `ShapeFamilies` | - | 25.7 s | | - | 595 MB | | new |
| `Shapes` | 27.5 s | 28.9 s | +5.0% | 599 MB | 607 MB | +1.5% | ~ |
| `SphereCloudBVH` | 8.7 s | 9.4 s | +7.6% | 524 MB | 531 MB | +1.4% | ~ |
| `SphereCloudRays` | 157.1 s | 179.8 s | +14.5% | 3332 MB | 4061 MB | +21.9% | **minor** |
| `SphereContacts` | 8.7 s | 8.5 s | -2.4% | 523 MB | 531 MB | +1.5% | ~ |
| `SurfaceParts` | 336.5 s | 424.0 s | +26.0% | 3494 MB | 4323 MB | +23.7% | **MAJOR** |
| `Tape` | 56.7 s | 70.0 s | +23.6% | 992 MB | 1080 MB | +8.9% | **minor** |
| `TapeBVHUnion` | 40.6 s | 48.2 s | +18.9% | 932 MB | 1021 MB | +9.5% | **minor** |
| **all, in both** | 4677.0 s | 6122.8 s | +30.9% | 5452 MB | 6409 MB | +17.6% | **MAJOR** |

Device kernels: 226 in both, 97 changed (71 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 6 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double, Fused::Node<Fused::Comb…` | 168 → 168 | 9328 → 10384 | 16/4 → 16/36 | **spills stack** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double, TriMeshSDF<double, 4ul,…` | 168 → 168 | 9360 → 10400 | 20/4 → 24/40 | **spills stack** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double> >, Vec3T<double> const,…` | 168 → 168 | 9328 → 10368 | 16/4 → 16/36 | **spills stack** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float, Fused::Node<Fused::Combin…` | 168 → 168 | 6672 → 7120 | 4/4 → 4/20 | **spills** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float, TriMeshSDF<float, 4ul, 4u…` | 168 → 168 | 6640 → 7120 | 4/4 → 4/20 | **spills** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float> >, Vec3T<float> const, Le…` | 168 → 168 | 6656 → 7152 | 4/4 → 4/20 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, BVHUnion<double, 4ul, …` | 168 → 168 | 7600 → 7632 | 20/4 → 20/20 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, BVHUnion<double, 4ul, …` | 168 → 168 | 7680 → 7856 | 48/152 → 68/148 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, SphereCloudBVH<double,…` | 168 → 168 | 7584 → 7616 | 20/4 → 20/20 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, SphereCloudBVH<double,…` | 168 → 168 | 7680 → 7840 | 48/152 → 68/148 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, BVHUnion<float, 4ul, Sph…` | 168 → 168 | 4624 → 4688 | 4/4 → 4/12 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, SphereCloudBVH<float, 4u…` | 168 → 168 | 4624 → 4720 | 4/4 → 4/12 | **spills** |
| `Domain` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)0>, Vec3T<double> const,…` | 168 → 168 | 4448 → 4480 | 228/272 → 256/300 | **spills** |
| `Domain` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)0>, Vec3T<float> const, A…` | 168 → 168 | 3104 → 3104 | 180/232 → 208/260 | **spills** |
| `DomainObject` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)0>, Vec3T<double> const,…` | 168 → 168 | 4192 → 4224 | 228/272 → 256/300 | **spills** |
| `DomainObject` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)0>, Vec3T<float> const, A…` | 168 → 168 | 2976 → 2976 | 180/232 → 208/260 | **spills** |
| `Fused` | `void parallelForKernel<128u, 384u, Apply<Distance<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 4448 → 4512 | 20/4 → 20/20 | **spills** |
| `Fused` | `void parallelForKernel<128u, 384u, Apply<Distance<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 3024 → 3088 | 4/4 → 4/12 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 17184 | 178/216 → 186/236 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11936 | 114/132 → 114/140 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 80 → 80 | 17328 → 17552 | 282/460 → 302/512 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 80 → 80 | 12320 → 12432 | 126/132 → 126/140 | **spills** |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 128 → 128 | 17152 → 17376 | 178/216 → 186/236 | **spills** |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 128 → 128 | 12064 → 12176 | 114/132 → 114/140 | **spills** |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> cons…` | 255 → 40 | 16512 → 17776 | 166/216 → 702/1292 | **registers spills** |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const, …` | 40 → 40 | 12608 → 12672 | 230/368 → 242/408 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17264 | 174/220 → 190/236 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11888 → 12000 | 114/132 → 114/140 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19248 → 19568 | 1500/1412 → 1556/1476 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17264 | 174/220 → 190/236 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11920 → 12032 | 114/132 → 114/140 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, RegionView<Tape<double, TriMeshSDF<…` | 168 → 168 | 20400 → 20784 | 2788/2348 → 2892/2348 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19248 → 19568 | 1500/1412 → 1556/1476 | **spills** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 17184 | 178/216 → 186/236 | **spills** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11920 | 114/132 → 114/140 | **spills** |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16832 → 17088 | 164/148 → 172/152 | **spills** |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11712 → 11856 | 132/132 → 132/140 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17072 | 174/220 → 190/236 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 17184 | 178/216 → 186/236 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11920 → 11952 | 114/132 → 114/140 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11872 | 114/132 → 114/140 | **spills** |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 18720 → 18768 | 246/208 → 258/208 | **spills** |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13328 → 13360 | 178/160 → 178/168 | **spills** |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13152 → 13232 | 178/160 → 178/168 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)1>, Vec3T<double> const,…` | 168 → 168 | 4160 → 4208 | 340/352 → 368/380 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)3>, Vec3T<double> const,…` | 168 → 168 | 4208 → 4240 | 228/272 → 256/300 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)1>, Vec3T<float> const, R…` | 168 → 168 | 2944 → 2944 | 236/268 → 264/296 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)3>, Vec3T<float> const, R…` | 168 → 168 | 2976 → 2976 | 180/232 → 208/260 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, false>, Vec3T<double> const, Reg…` | 168 → 168 | 16944 → 17184 | 178/192 → 186/212 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, false>, Vec3T<float> const, Regio…` | 168 → 168 | 11824 → 11936 | 114/120 → 114/128 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 19120 → 19504 | 1508/1376 → 1556/1476 | **spills** |
| `SphereCloudRays` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, SphereCloudBVH<doub…` | 168 → 168 | 17152 → 17184 | 126/104 → 126/120 | **spills** |
| `SphereCloudRays` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, SphereCloudBVH<float,…` | 168 → 168 | 12032 → 11824 | 102/88 → 118/148 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, BVHUnion<double, …` | 168 → 168 | 7616 → 7792 | 36/120 → 72/140 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, BVHUnion<double, …` | 168 → 168 | 7632 → 7792 | 28/128 → 64/148 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, TriMeshSDF<double…` | 168 → 168 | 7648 → 7792 | 68/120 → 72/140 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, TriMeshSDF<double…` | 168 → 168 | 7664 → 7792 | 60/128 → 64/148 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<float, Tape<float, BVHUnion<float, 4ul…` | 168 → 168 | 4688 → 4864 | 36/132 → 40/120 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<float, Tape<float, BVHUnion<float, 4ul…` | 168 → 168 | 4688 → 4800 | 28/140 → 32/112 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<float, Tape<float, TriMeshSDF<float, 4…` | 168 → 168 | 4704 → 4832 | 36/132 → 40/120 | **spills** |

37 more changed kernels are in the records.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (20 cases): ClosestPrimitive/csg/closestPrimitive/float/hostdevice, ClosestPrimitive/parts/closestPrimitive/float/hostdevice, Domain/particles/labelAt/float/hostdevice, Domain/particles/labels/double/hostdevice, DomainObject/partsWhole/labels/float/hostdevice, DomainObject/partsWhole/labelAt/float/hostdevice, DomainObject/partsOwn/labels/float/hostdevice, DomainObject/partsOwn/labelAt/float/hostdevice, LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice, RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice, RayParticles/armadillo-shell-tape/step-morton/float/hostdevice, RayParticles/armadillo-shell-tape/step-morton/double/hostdevice, Regions/particles/regionByRegion/double/hostdevice, Render/csg/frame/float/hostdevice, Render/csg/frame-orthographic/float/hostdevice, Tape/csg/signedDistance/float/hostdevice, Tape/smooth/signedDistance/float/hostdevice, Tape/sphere/signedDistance/float/hostdevice, Tape/chain16/signedDistance/float/hostdevice, TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice

The probes differ between the sides (each side ran its own): Performance/ShapeFamilies/main.cpp

<!-- END receipt cuda -->
