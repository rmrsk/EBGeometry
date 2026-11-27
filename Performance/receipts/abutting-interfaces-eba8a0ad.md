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
base: 235ade11
base-fingerprint: 628a0fbb74054238a176bfe2ff0d5ea2ed942bd9
head: 1d8217d2
head-fingerprint: eba8a0ad527cf2abf95c5174e2b267a071892c2a
cases: 364
rounds: 2
reps: 3
date: 2026-11-27
-->

## Host comparison: yellow

**Accepted:** -

`1d8217d2` against `235ade11` (235ade111ec13aa3372a475bf21cbb56351de576), 2026-11-27.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 6 faster, 0 differ, 0 lost, 364 cases

- base: 235ade11 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 1d8217d2 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 329 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 514 ms | 512 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 349 ms | 350 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 187 ms | 186 ms | -0.5% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 564 ms | 563 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.9 ms | 41.5 ms | -0.8% | [-1.2%, -0.3%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 76.2 ms | 75.8 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.0 ms | 56.2 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 56.0 ms | 51.3 ms | -8.5% | [-15.8%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.7 ms | 64.6 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.9 ms | 54.7 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.5 ms | 60.6 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.6 ms | 72.4 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 72.7 ms | 72.7 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.3 ms | 77.7 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.6 ms | 64.5 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 101 ms | +0.9% | [-0.1%, +2.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 77.9 ms | 77.5 ms | -0.6% | [-1.8%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 64.1 ms | 63.9 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 95.8 ms | 95.5 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 75.8 ms | 76.0 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 101 ms | 101 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.5 ms | +0.6% | [-0.2%, +1.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.1 ms | 39.4 ms | -4.1% | [-4.4%, -3.8%] | +0.0% | faster |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.1 ms | 73.1 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.1 ms | 20.6 ms | +2.6% | [+1.8%, +3.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.4 ms | 43.3 ms | -0.4% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.1 ms | 80.8 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 141 ms | 139 ms | -1.3% | [-2.2%, -0.4%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 77.1 ms | 77.0 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ | surface/leaf 1.81x |
| `ClosestSurface/union/closestSurface/float/host` | 146 ms | 146 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 76.8 ms | 76.9 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ | surface/leaf 1.90x |
| `ClosestSurface/cloud/closestSurface/double/host` | 156 ms | 159 ms | +1.4% | [+0.0%, +2.9%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 80.2 ms | 80.4 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | surface/leaf 1.97x |
| `ClosestSurface/union/closestSurface/double/host` | 153 ms | 153 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 81.6 ms | 80.9 ms | -0.9% | [-1.0%, -0.7%] | +0.0% | ~ | surface/leaf 1.89x |
| `ConnectedComponents/shells/build-coarse/float/host` | 148 ms | 150 ms | +1.3% | [+1.1%, +1.6%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/float/host` | 662 ms | 666 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-coarse/double/host` | 152 ms | 155 ms | +1.5% | [+0.0%, +2.9%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/double/host` | 679 ms | 683 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `Domain/particles/labels/float/host` | 13.5 ms | 13.7 ms | +1.9% | [+1.7%, +2.0%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.77 ms | 6.77 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.31 ms | 1.30 ms | -1.2% | [-2.4%, -0.0%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.0 ms | 13.9 ms | -0.5% | [-1.4%, +0.4%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.12 ms | 7.22 ms | +1.4% | [+0.9%, +2.0%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.33 ms | 1.30 ms | -1.8% | [-3.1%, -0.6%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 39.4 ms | 39.5 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.1 ms | 21.1 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 563 µs | 561 µs | -0.5% | [-1.9%, +1.0%] | +0.0% | ~ | whole/own 70.41x |
| `DomainObject/partsOwn/labelAt/float/host` | 305 µs | 305 µs | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ | whole/own 69.20x |
| `DomainObject/partsWhole/labels/double/host` | 39.7 ms | 39.6 ms | -0.2% | [-1.3%, +0.8%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.6 ms | 22.7 ms | +0.3% | [+0.3%, +0.4%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 583 µs | 578 µs | -0.8% | [-2.3%, +0.8%] | +0.0% | ~ | whole/own 68.50x |
| `DomainObject/partsOwn/labelAt/double/host` | 318 µs | 316 µs | -0.7% | [-1.6%, +0.3%] | +0.0% | ~ | whole/own 71.92x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/host` | 63.6 ms | 63.7 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/host` | 59.1 ms | 59.2 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/host` | 77.2 ms | 77.4 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | grid/shuffled 0.82x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/host` | 68.1 ms | 68.3 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/host` | 63.3 ms | 63.4 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/host` | 88.6 ms | 89.1 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ | grid/shuffled 0.77x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/host` | 190 ms | 190 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/host` | 178 ms | 178 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/host` | 257 ms | 257 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | grid/shuffled 0.74x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/host` | 202 ms | 202 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/host` | 187 ms | 187 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/host` | 290 ms | 291 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ | grid/shuffled 0.69x |
| `Fused/csg/signedDistance/float/host` | 4.45 ms | 4.49 ms | +0.8% | [+0.0%, +1.7%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.2 ms | -1.4% | [-2.5%, -0.2%] | - | ~ | tape/fused 2.06x |
| `Fused/chain16/signedDistance/float/host` | 967 µs | 984 µs | +1.7% | [+0.0%, +3.4%] | - | ~ | tape/fused 16.71x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.0 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.4 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.60 ms | 4.56 ms | -0.7% | [-1.6%, +0.1%] | - | ~ | tape/fused 2.90x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | tape/fused 2.06x |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.95 ms | +1.6% | [+0.0%, +3.2%] | - | ~ | tape/fused 4.14x |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.2 ms | 53.3 ms | +0.1% | [-2.5%, +2.8%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.3 ms | 53.7 ms | +0.8% | [-0.3%, +1.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 83.1 ms | 83.1 ms | -0.0% | [-0.8%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 228 ms | -0.2% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 260 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.9 ms | 71.3 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 213 ms | 213 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 98.1 ms | 97.7 ms | -0.4% | [-1.2%, +0.4%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 28.8 ms | 28.4 ms | -1.6% | [-2.1%, -1.1%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 763 ms | 759 ms | -0.5% | [-1.0%, -0.1%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 119 ms | +0.9% | [+0.6%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 42.7 ms | 43.8 ms | +2.7% | [-0.3%, +5.9%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 856 ms | 853 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.5 ms | 26.8 ms | +1.0% | [-1.2%, +3.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.7 ms | 15.5 ms | -1.2% | [-1.9%, -0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 318 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 232 ms | 231 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.6 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ | tape/fused 3.51x |
| `MeshCSG/smooth-fused/frame/float/host` | 81.6 ms | 81.4 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | tape/fused 2.84x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.2 ms | 29.4 ms | +0.8% | [-2.4%, +3.9%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.2 ms | 17.0 ms | -0.8% | [-1.3%, -0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 356 ms | 357 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 246 ms | 245 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.9 ms | 97.8 ms | -0.0% | [-1.1%, +1.1%] | +0.0% | ~ | tape/fused 3.65x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.2 ms | 87.7 ms | +0.6% | [-0.2%, +1.3%] | +0.0% | ~ | tape/fused 2.79x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 229 ms | 229 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 232 ms | 232 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | signedDistance/closestPoint 0.99x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 305 ms | 305 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 232 ms | 232 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | closestPoint/gradient 1.00x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 261 ms | 260 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 264 ms | 264 ms | +0.2% | [-0.3%, +0.8%] | +0.0% | ~ | signedDistance/closestPoint 0.99x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 341 ms | 343 ms | +0.4% | [-1.0%, +1.7%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 265 ms | 264 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ | closestPoint/gradient 1.00x |
| `MeshIngest/armadillo/read-soup/float/host` | 54.0 ms | 53.2 ms | -1.5% | [-2.5%, -0.4%] | - | ~ |  |
| `MeshIngest/armadillo/clean/float/host` | 12.5 ms | 12.5 ms | +0.2% | [-0.2%, +0.6%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/float/host` | 95.6 ms | 96.1 ms | +0.5% | [+0.4%, +0.7%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/float/host` | 103 ms | 103 ms | +0.1% | [-0.2%, +0.3%] | - | ~ |  |
| `MeshIngest/armadillo/read-soup/double/host` | 53.9 ms | 54.3 ms | +0.8% | [+0.3%, +1.3%] | - | ~ |  |
| `MeshIngest/armadillo/clean/double/host` | 13.5 ms | 13.6 ms | +0.4% | [-1.4%, +2.1%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/double/host` | 106 ms | 106 ms | -0.6% | [-1.2%, +0.1%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/double/host` | 115 ms | 115 ms | -0.2% | [-0.6%, +0.2%] | - | ~ |  |
| `MeshReconcile/armadillo/reconcile/float/host` | 73.3 ms | 73.1 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.4 ms | 41.2 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ | reconcile/scheduled 1.77x |
| `MeshReconcile/armadillo/reconcile/double/host` | 93.9 ms | 94.2 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.0 ms | 56.1 ms | +0.1% | [-1.3%, +1.6%] | +0.0% | ~ | reconcile/scheduled 1.68x |
| `MeshTape/mesh/firstHit/float/host` | 48.5 ms | 48.4 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 48.6 ms | 48.5 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.6 ms | 19.9 ms | +2.0% | [-0.3%, +4.3%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 80.4 ms | 80.2 ms | -0.3% | [-0.3%, -0.2%] | +0.0% | ~ | mesh/tape 0.60x |
| `MeshTape/tape/firstCrossing/float/host` | 49.9 ms | 49.8 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ | mesh/tape 0.98x |
| `MeshTape/tape/frame/float/host` | 25.0 ms | 25.2 ms | +0.7% | [+0.4%, +1.1%] | +0.0% | ~ | mesh/tape 0.79x |
| `MeshTape/view/firstHit/float/host` | 83.9 ms | 84.8 ms | +1.1% | [-1.3%, +3.6%] | +0.0% | ~ | mesh/view 0.57x |
| `MeshTape/view/frame/float/host` | 26.3 ms | 26.4 ms | +0.1% | [-0.5%, +0.6%] | +0.0% | ~ | mesh/view 0.76x |
| `MeshTape/mesh/firstHit/double/host` | 53.0 ms | 53.3 ms | +0.6% | [+0.6%, +0.7%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 53.1 ms | 53.4 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.3 ms | 20.4 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 87.4 ms | 87.4 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | mesh/tape 0.61x |
| `MeshTape/tape/firstCrossing/double/host` | 55.6 ms | 55.1 ms | -0.9% | [-1.9%, +0.1%] | +0.0% | ~ | mesh/tape 0.97x |
| `MeshTape/tape/frame/double/host` | 26.8 ms | 27.2 ms | +1.6% | [+0.8%, +2.3%] | +0.0% | ~ | mesh/tape 0.75x |
| `MeshTape/view/firstHit/double/host` | 91.0 ms | 89.7 ms | -1.4% | [-1.9%, -1.0%] | +0.0% | ~ | mesh/view 0.59x |
| `MeshTape/view/frame/double/host` | 29.3 ms | 29.6 ms | +1.0% | [+0.6%, +1.4%] | +0.0% | ~ | mesh/view 0.69x |
| `PointCloudBVH/uniform/build/float/host` | 53.0 ms | 52.9 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 145 ms | 143 ms | -1.2% | [-2.6%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 363 ms | 362 ms | -0.0% | [-0.6%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 80.2 ms | 80.4 ms | +0.3% | [+0.1%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.74 ms | 6.73 ms | -0.2% | [-0.4%, +0.0%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 56.0 ms | 55.6 ms | -0.9% | [-2.6%, +0.9%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 158 ms | 158 ms | +0.2% | [-0.3%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 394 ms | 393 ms | -0.3% | [-0.7%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.3 ms | 87.4 ms | +0.1% | [-1.2%, +1.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.98 ms | 7.01 ms | +0.4% | [-0.9%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 17.4 ms | 14.0 ms | -19.9% | [-32.0%, -2.7%] | +0.0% | faster |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 117 ms | 117 ms | -0.2% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 474 ms | 474 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 141 ms | 141 ms | -0.2% | [-0.4%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 801 ms | 801 ms | -0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 15.4 ms | +2.7% | [-0.4%, +5.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 121 ms | 121 ms | +0.3% | [-0.4%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 492 ms | 492 ms | -0.0% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 144 ms | +0.1% | [-0.5%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 798 ms | 798 ms | -0.0% | [-0.1%, -0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.2 ms | 54.2 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 129 ms | 129 ms | -0.2% | [-0.4%, +0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 268 ms | 268 ms | -0.2% | [-0.4%, +0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 149 ms | 149 ms | -0.1% | [-0.2%, +0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 134 ms | 134 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.1 ms | 56.8 ms | -0.5% | [-1.1%, +0.0%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 141 ms | 140 ms | -0.1% | [-0.7%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 298 ms | 298 ms | +0.1% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 163 ms | 164 ms | +0.1% | [-0.6%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 149 ms | 149 ms | -0.0% | [-0.5%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 15.1 ms | 12.1 ms | -20.0% | [-34.6%, +3.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 115 ms | 114 ms | -0.5% | [-1.2%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 316 ms | 316 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 156 ms | 155 ms | -0.6% | [-1.3%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 321 ms | 321 ms | -0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.7 ms | 13.8 ms | +1.0% | [-1.0%, +3.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 116 ms | +0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 346 ms | 348 ms | +0.7% | [-0.1%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 164 ms | 166 ms | +1.1% | [-0.1%, +2.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 309 ms | -0.0% | [-0.5%, +0.5%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.4 ms | 35.4 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.1 ms | 38.2 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.60 ms | 8.66 ms | +0.7% | [-5.1%, +6.7%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.79 ms | 9.66 ms | -1.3% | [-7.1%, +4.9%] | +0.0% | ~ |  |
| `QueryOrder/random/mortonOrder/float/host` | 60.8 ms | 60.7 ms | -0.1% | [-0.2%, +0.1%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | 223 ms | 223 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | 160 ms | 160 ms | -0.0% | [-0.7%, +0.7%] | +0.0% | ~ | arrived/ordered 1.39x |
| `QueryOrder/random/mortonOrder/double/host` | 62.8 ms | 62.5 ms | -0.5% | [-1.2%, +0.3%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | 263 ms | 266 ms | +1.2% | [+0.3%, +2.2%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | 171 ms | 171 ms | -0.1% | [-0.8%, +0.5%] | +0.0% | ~ | arrived/ordered 1.56x |
| `Ray/mesh/firstHit/float/host` | 51.3 ms | 48.6 ms | -5.3% | [-10.2%, +0.2%] | +0.0% | ~ | hand/api 0.94x (was 0.89x) |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.8 ms | 45.7 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.5 ms | 40.7 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.3 ms | 78.7 ms | +0.6% | [-0.1%, +1.3%] | +0.0% | ~ | triangles/polygons 0.62x (was 0.66x) |
| `Ray/mesh-polygons/intersects/float/host` | 65.3 ms | 65.4 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.24 ms | 5.29 ms | +1.0% | [-0.6%, +2.7%] | - | ~ | tape/fused 1.43x |
| `Ray/csg-tape/firstHit/float/host` | 7.67 ms | 7.56 ms | -1.4% | [-2.1%, -0.7%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 53.6 ms | 53.4 ms | -0.3% | [-1.1%, +0.5%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.3 ms | 51.6 ms | +0.6% | [+0.5%, +0.8%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.2 ms | 45.2 ms | +0.1% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 89.1 ms | 89.3 ms | +0.2% | [-0.9%, +1.4%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.7 ms | 74.3 ms | +0.8% | [+0.3%, +1.2%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.52 ms | 5.54 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | tape/fused 1.43x |
| `Ray/csg-tape/firstHit/double/host` | 7.89 ms | 7.91 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.2 ms | 17.4 ms | +1.1% | [+0.1%, +2.1%] | - | ~ | marched/traced 6.84x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 119 ms | -0.2% | [-0.6%, +0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.9 ms | 22.0 ms | +0.4% | [-0.7%, +1.6%] | +0.0% | ~ | marched/traced 6.58x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 144 ms | 145 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.1 ms | 35.4 ms | +0.7% | [+0.6%, +0.8%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.5 ms | 18.9 ms | -3.0% | [-3.4%, -2.6%] | - | faster | marched/traced 6.67x (was 6.47x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 126 ms | +0.0% | [-0.5%, +0.5%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.7 ms | 23.5 ms | -1.0% | [-2.7%, +0.8%] | +0.0% | ~ | marched/traced 6.44x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 150 ms | 151 ms | +1.1% | [+0.6%, +1.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.6 ms | 37.5 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 48.7 ms | 48.5 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 48.5 ms | 48.5 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.57 ms | 7.61 ms | +0.5% | [+0.0%, +0.9%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.07 ms | 6.11 ms | +0.7% | [-0.3%, +1.7%] | +0.0% | ~ | hit/crossing 1.25x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 178 ms | 177 ms | -0.4% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 157 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.8 ms | 68.9 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.4 ms | 56.5 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.9 ms | 53.1 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 53.1 ms | 53.0 ms | -0.2% | [-1.3%, +1.0%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.89 ms | 7.99 ms | +1.3% | [-1.2%, +3.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.27 ms | 6.29 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | hit/crossing 1.27x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 200 ms | 200 ms | +0.4% | [+0.2%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 178 ms | 177 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.7 ms | 77.7 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.4 ms | 64.1 ms | -0.5% | [-1.0%, +0.0%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.2 ms | 11.1 ms | -1.0% | [-2.2%, +0.3%] | +0.0% | ~ | marched/traced 4.64x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.6 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.01 ms | 8.00 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.02 ms | 2.02 ms | -0.2% | [-1.7%, +1.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.91 ms | 6.97 ms | +0.8% | [+0.7%, +1.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.8 ms | 11.7 ms | -1.1% | [-1.9%, -0.4%] | +0.0% | ~ | marched/traced 5.20x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.9 ms | 60.8 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.63 ms | 8.64 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.03 ms | 2.05 ms | +1.1% | [+0.2%, +2.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.24 ms | 7.05 ms | -2.7% | [-3.6%, -1.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 239 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 328 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 238 ms | 237 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 253 ms | 251 ms | -1.0% | [-1.2%, -0.9%] | +0.0% | ~ | marched/traced 1.47x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 368 ms | 368 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 252 ms | 249 ms | -1.1% | [-1.4%, -0.8%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.34 ms | 7.33 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.5 ms | 11.8 ms | -6.1% | [-6.3%, -5.8%] | +0.0% | faster |  |
| `Regions/particles/regionByRegion/float/host` | 264 ms | 265 ms | +0.4% | [+0.3%, +0.4%] | +0.0% | ~ | one-pass/each 0.04x (was 0.05x) |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.5 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.3 ms | 30.4 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.1 ms | 22.3 ms | +0.9% | [-1.4%, +3.2%] | +0.0% | ~ | regions/hit 1.36x |
| `Regions/particles/region/double/host` | 7.64 ms | 7.63 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.9 ms | 12.4 ms | -3.9% | [-3.9%, -3.8%] | +0.0% | faster |  |
| `Regions/particles/regionByRegion/double/host` | 282 ms | 284 ms | +0.9% | [+0.7%, +1.1%] | +0.0% | ~ | one-pass/each 0.04x (was 0.05x) |
| `Regions/particles/domainLabels/double/host` | 13.0 ms | 13.1 ms | +0.6% | [-0.4%, +1.5%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.1 ms | 32.2 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 24.1 ms | 23.8 ms | -1.4% | [-2.2%, -0.6%] | +0.0% | ~ | regions/hit 1.35x |
| `Render/csg/frame/float/host` | 13.1 ms | 13.1 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 13.0 ms | 13.0 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.5 ms | 19.5 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.5 ms | 14.0 ms | -3.0% | [-6.0%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 13.9 ms | 14.2 ms | +2.5% | [-0.2%, +5.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.5 ms | 20.7 ms | +1.3% | [-1.0%, +3.6%] | +0.0% | ~ |  |
| `ShapeFamilies/icosahedron/signedDistance/float/host` | 16.6 ms | 16.6 ms | -0.3% | [-0.9%, +0.3%] | - | ~ |  |
| `ShapeFamilies/pyramid/signedDistance/float/host` | 12.0 ms | 12.0 ms | +0.1% | [+0.0%, +0.3%] | - | ~ |  |
| `ShapeFamilies/box-frame/signedDistance/float/host` | 3.28 ms | 3.27 ms | -0.1% | [-0.6%, +0.3%] | - | ~ |  |
| `ShapeFamilies/hyperboloid/signedDistance/float/host` | 17.0 ms | 17.0 ms | +0.3% | [-0.3%, +0.8%] | - | ~ |  |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/float/host` | 17.4 ms | 17.3 ms | -0.2% | [-1.0%, +0.6%] | - | ~ |  |
| `ShapeFamilies/paraboloid/signedDistance/float/host` | 10.4 ms | 10.4 ms | +0.1% | [-0.0%, +0.3%] | - | ~ |  |
| `ShapeFamilies/superellipsoid/signedDistance/float/host` | 10.1 ms | 10.1 ms | -0.1% | [-0.2%, +0.1%] | - | ~ |  |
| `ShapeFamilies/solid-angle/signedDistance/float/host` | 967 µs | 973 µs | +0.6% | [+0.2%, +1.0%] | - | ~ |  |
| `ShapeFamilies/vesica/signedDistance/float/host` | 2.01 ms | 2.01 ms | +0.0% | [-0.6%, +0.7%] | - | ~ |  |
| `ShapeFamilies/helix/signedDistance/float/host` | 27.1 ms | 27.2 ms | +0.2% | [-1.0%, +1.4%] | - | ~ |  |
| `ShapeFamilies/gyroid/signedDistance/float/host` | 5.11 ms | 5.12 ms | +0.3% | [-0.4%, +0.9%] | - | ~ |  |
| `ShapeFamilies/ray-forms/firstHit/float/host` | 35.8 ms | 36.0 ms | +0.5% | [-0.0%, +1.1%] | - | ~ |  |
| `ShapeFamilies/icosahedron/signedDistance/double/host` | 16.8 ms | 16.5 ms | -2.1% | [-3.2%, -1.0%] | - | ~ |  |
| `ShapeFamilies/pyramid/signedDistance/double/host` | 12.2 ms | 12.1 ms | -0.3% | [-0.8%, +0.2%] | - | ~ |  |
| `ShapeFamilies/box-frame/signedDistance/double/host` | 3.28 ms | 3.25 ms | -0.9% | [-1.6%, -0.3%] | - | ~ |  |
| `ShapeFamilies/hyperboloid/signedDistance/double/host` | 27.6 ms | 27.6 ms | +0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/double/host` | 29.0 ms | 28.9 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `ShapeFamilies/paraboloid/signedDistance/double/host` | 12.8 ms | 12.8 ms | +0.2% | [-0.2%, +0.6%] | - | ~ |  |
| `ShapeFamilies/superellipsoid/signedDistance/double/host` | 16.5 ms | 16.4 ms | -0.7% | [-2.6%, +1.2%] | - | ~ |  |
| `ShapeFamilies/solid-angle/signedDistance/double/host` | 1.14 ms | 1.14 ms | -0.7% | [-1.4%, +0.0%] | - | ~ |  |
| `ShapeFamilies/vesica/signedDistance/double/host` | 2.46 ms | 2.47 ms | +0.3% | [-0.3%, +0.8%] | - | ~ |  |
| `ShapeFamilies/helix/signedDistance/double/host` | 33.8 ms | 33.8 ms | +0.0% | [-0.7%, +0.8%] | - | ~ |  |
| `ShapeFamilies/gyroid/signedDistance/double/host` | 10.9 ms | 10.9 ms | +0.5% | [-0.1%, +1.2%] | - | ~ |  |
| `ShapeFamilies/ray-forms/firstHit/double/host` | 44.6 ms | 40.0 ms | -10.5% | [-19.4%, +0.6%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 33.0 ms | 33.0 ms | +0.2% | [-0.0%, +0.5%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 5.02 ms | 5.03 ms | +0.2% | [-0.2%, +0.6%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.17 ms | 4.18 ms | +0.2% | [-0.0%, +0.5%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.4 ms | 17.2 ms | -0.9% | [-1.6%, -0.2%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.33 ms | 4.33 ms | -0.2% | [-1.5%, +1.1%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.13 ms | 4.10 ms | -0.6% | [-1.3%, +0.0%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.28 ms | 2.21 ms | -2.8% | [-5.6%, +0.1%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.44 ms | 1.45 ms | +0.1% | [-0.8%, +0.9%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.04 ms | 2.06 ms | +0.7% | [-0.1%, +1.5%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.2 ms | 27.2 ms | +0.2% | [-0.1%, +0.5%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 46.2 ms | 46.2 ms | +0.1% | [+0.0%, +0.2%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.35 ms | 5.42 ms | +1.4% | [-0.0%, +2.8%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.48 ms | 4.48 ms | -0.1% | [-1.0%, +0.9%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 22.4 ms | 22.5 ms | +0.5% | [-0.1%, +1.2%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.63 ms | 4.53 ms | -2.1% | [-2.8%, -1.3%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.29 ms | 5.29 ms | -0.1% | [-1.6%, +1.3%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.52 ms | 2.54 ms | +0.5% | [-1.0%, +2.1%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.78 ms | 1.77 ms | -0.4% | [-0.5%, -0.3%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.07 ms | 2.09 ms | +0.7% | [+0.4%, +1.0%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 30.0 ms | 30.3 ms | +0.7% | [+0.4%, +1.0%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.5 ms | 56.5 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.7 ms | 36.3 ms | -1.2% | [-3.8%, +1.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 93.0 ms | 92.0 ms | -1.1% | [-2.4%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 63.5 ms | 63.6 ms | +0.2% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 41.8 ms | 41.8 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 107 ms | 107 ms | +0.2% | [-0.8%, +1.3%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.8 ms | 74.9 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 295 ms | 296 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 295 ms | 297 ms | +1.0% | [+0.5%, +1.4%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 86.7 ms | 86.6 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 346 ms | 345 ms | -0.3% | [-1.3%, +0.7%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 347 ms | 349 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 166 ms | 166 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 190 ms | 190 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 145 ms | 144 ms | -0.9% | [-2.1%, +0.3%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 91.7 ms | 89.7 ms | -2.2% | [-3.4%, -1.0%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 131 ms | 130 ms | -0.5% | [-0.6%, -0.3%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 72.7 ms | 72.5 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ | surface/leaf 1.80x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 61.6 ms | 61.5 ms | -0.2% | [-1.1%, +0.8%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.1 ms | 34.3 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ | surface/hit 1.79x |
| `SurfaceParts/boxes/closestSurface/double/host` | 144 ms | 145 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 90.8 ms | 90.6 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 145 ms | 145 ms | +0.1% | [-0.8%, +0.9%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 82.1 ms | 82.4 ms | +0.3% | [-1.8%, +2.5%] | +0.0% | ~ | surface/leaf 1.76x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 69.7 ms | 69.6 ms | -0.1% | [-1.8%, +1.6%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 39.5 ms | 39.8 ms | +0.7% | [-2.2%, +3.7%] | +0.0% | ~ | surface/hit 1.75x |
| `Tape/csg/signedDistance/float/host` | 13.0 ms | 12.9 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.63 ms | 4.54 ms | -1.9% | [-3.7%, +0.0%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.1 ms | 25.1 ms | -0.2% | [-1.8%, +1.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 39.9 ms | 40.0 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.76 ms | 1.76 ms | -0.0% | [-1.1%, +1.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 138 µs | 133 µs | -3.8% | [-7.0%, -0.5%] | - | faster |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.2 ms | 30.0 ms | -0.7% | [-1.7%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.67 ms | 5.67 ms | -0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.4 ms | 16.4 ms | +0.3% | [-1.5%, +2.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.70 ms | 4.66 ms | -0.9% | [-1.9%, +0.1%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.2 ms | +1.4% | [+0.7%, +2.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.56 ms | 4.62 ms | +1.5% | [-0.0%, +3.0%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.5 ms | 26.4 ms | -0.5% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 41.9 ms | 42.0 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.79 ms | 1.77 ms | -1.4% | [-2.5%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 286 µs | 292 µs | +2.3% | [+0.7%, +3.9%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.2 ms | 30.0 ms | -0.6% | [-2.5%, +1.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.83 ms | 7.80 ms | -0.5% | [-1.8%, +0.9%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.4 ms | 16.4 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.82 ms | 7.74 ms | -1.0% | [-2.3%, +0.4%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.9 ms | 42.2 ms | +0.7% | [+0.1%, +2.0%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 200 ms | 217 ms | +8.6% | [+7.8%, +9.0%] | +0.0% | minor |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 43.2 ms | 43.7 ms | +1.0% | [-0.7%, +2.8%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 213 ms | 214 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `Weld/particles/region/float/host` | - | - | - | - | - | new |  |
| `Weld/particles/regionDistances/float/host` | - | - | - | - | - | new |  |
| `Weld/blocks/region/float/host` | - | - | - | - | - | new |  |
| `Weld/blocks/closestSurface/float/host` | - | - | - | - | - | new |  |
| `Weld/particles/region/double/host` | - | - | - | - | - | new |  |
| `Weld/particles/regionDistances/double/host` | - | - | - | - | - | new |  |
| `Weld/blocks/region/double/host` | - | - | - | - | - | new |  |
| `Weld/blocks/closestSurface/double/host` | - | - | - | - | - | new |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 957.7 s → 810.2 s (-15.4%), the largest peak 1618 MB → 1621 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.6 s | 9.7 s | -28.5% | 609 MB | 607 MB | -0.4% | ~ |
| `BVHRefit` | 7.5 s | 4.4 s | -41.7% | 516 MB | 518 MB | +0.5% | ~ |
| `BVHUnion` | 21.8 s | 19.1 s | -12.3% | 942 MB | 964 MB | +2.4% | ~ |
| `BVHUnionMixed` | 9.6 s | 7.8 s | -18.9% | 591 MB | 584 MB | -1.1% | ~ |
| `ClosestPrimitive` | 38.5 s | 32.7 s | -15.0% | 1323 MB | 1315 MB | -0.6% | ~ |
| `ClosestSurface` | 36.3 s | 29.4 s | -19.0% | 1343 MB | 1339 MB | -0.3% | ~ |
| `ConnectedComponents` | 21.8 s | 16.4 s | -24.9% | 868 MB | 878 MB | +1.1% | ~ |
| `Domain` | 24.4 s | 19.1 s | -21.5% | 1009 MB | 1025 MB | +1.6% | ~ |
| `DomainObject` | 18.9 s | 16.9 s | -10.8% | 897 MB | 914 MB | +1.9% | ~ |
| `EBGrid` | 15.9 s | 14.3 s | -10.0% | 793 MB | 823 MB | +3.8% | ~ |
| `Fused` | 31.1 s | 25.6 s | -17.6% | 1327 MB | 1273 MB | -4.0% | ~ |
| `LaunchTuning` | 50.1 s | 39.2 s | -21.8% | 1336 MB | 1320 MB | -1.2% | ~ |
| `Mesh` | 17.9 s | 16.0 s | -10.5% | 836 MB | 827 MB | -1.0% | ~ |
| `MeshCSG` | 56.6 s | 41.6 s | -26.5% | 1332 MB | 1320 MB | -0.8% | ~ |
| `MeshClosestPoint` | 16.4 s | 14.3 s | -12.9% | 834 MB | 825 MB | -1.1% | ~ |
| `MeshIngest` | 7.8 s | 7.0 s | -9.8% | 622 MB | 608 MB | -2.3% | ~ |
| `MeshReconcile` | 8.0 s | 6.9 s | -14.0% | 626 MB | 623 MB | -0.5% | ~ |
| `MeshTape` | 51.6 s | 44.2 s | -14.3% | 1344 MB | 1350 MB | +0.4% | ~ |
| `PointCloud` | 9.8 s | 7.6 s | -23.2% | 594 MB | 587 MB | -1.1% | ~ |
| `PointCloudBVHBuild` | 4.5 s | 4.0 s | -11.3% | 480 MB | 483 MB | +0.5% | ~ |
| `PointCloudHashGridBuild` | 4.2 s | 3.4 s | -19.1% | 456 MB | 457 MB | +0.4% | ~ |
| `QueryOrder` | 14.1 s | 11.6 s | -17.8% | 724 MB | 733 MB | +1.3% | ~ |
| `Ray` | 45.7 s | 38.3 s | -16.2% | 1342 MB | 1327 MB | -1.1% | ~ |
| `RayCSG` | 40.6 s | 34.7 s | -14.6% | 1282 MB | 1259 MB | -1.8% | ~ |
| `RayCrossing` | 62.0 s | 48.3 s | -22.2% | 1459 MB | 1471 MB | +0.8% | ~ |
| `RayParticles` | 63.0 s | 52.1 s | -17.3% | 1479 MB | 1491 MB | +0.8% | ~ |
| `RayShell` | 50.0 s | 40.0 s | -20.1% | 1329 MB | 1315 MB | -1.1% | ~ |
| `Regions` | 34.4 s | 32.3 s | -6.3% | 1254 MB | 1262 MB | +0.7% | ~ |
| `Render` | 40.5 s | 38.4 s | -5.0% | 1331 MB | 1318 MB | -1.0% | ~ |
| `ShapeFamilies` | 10.5 s | 8.8 s | -15.6% | 618 MB | 615 MB | -0.5% | ~ |
| `Shapes` | 9.4 s | 9.0 s | -4.2% | 621 MB | 585 MB | -5.7% | ~ |
| `SphereCloudBVH` | 5.2 s | 4.8 s | -7.5% | 533 MB | 535 MB | +0.4% | ~ |
| `SphereCloudRays` | 31.2 s | 30.2 s | -3.3% | 1218 MB | 1230 MB | +0.9% | ~ |
| `SphereContacts` | 4.6 s | 4.5 s | -2.6% | 499 MB | 501 MB | +0.5% | ~ |
| `SurfaceParts` | 45.4 s | 43.8 s | -3.5% | 1618 MB | 1621 MB | +0.1% | ~ |
| `Tape` | 18.5 s | 17.8 s | -3.8% | 948 MB | 962 MB | +1.6% | ~ |
| `TapeBVHUnion` | 16.2 s | 16.0 s | -1.0% | 840 MB | 850 MB | +1.2% | ~ |
| `Weld` | - | 19.0 s | | - | 1027 MB | | new |
| **all, in both** | 957.7 s | 810.2 s | -15.4% | 1618 MB | 1621 MB | +0.1% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): TapeBVHUnion/maps64-bvh/signedDistance/float/host, TapeBVHUnion/maps64-as-written/signedDistance/float/host

The probes differ between the sides (each side ran its own): Performance/Weld/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 235ade11
base-fingerprint: 628a0fbb74054238a176bfe2ff0d5ea2ed942bd9
head: 1d8217d2
head-fingerprint: eba8a0ad527cf2abf95c5174e2b267a071892c2a
cases: 446
rounds: 2
reps: 3
date: 2026-11-27
-->

## Device comparison (CUDA): green

**Accepted:** -

`1d8217d2` against `235ade11` (235ade111ec13aa3372a475bf21cbb56351de576), 2026-11-27.

**VERDICT: no regression (0 major, 0 minor across 446 cases)**

Performance comparison: 0 major, 0 minor, 2 faster, 0 differ, 0 lost, 446 cases

- base: 235ade11 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 1d8217d2 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.61 ms | +0.2% | [-0.7%, +1.1%] | - | ~ | device 22.6x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.40 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 18.7x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.09 ms | +3.7% | [-0.4%, +8.0%] | - | ~ | device 18.8x faster than the host (was 19.7x) |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.31 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.30 ms | 6.16 ms | -2.3% | [-6.0%, +1.6%] | - | ~ | device 7.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.9 ms | +0.2% | [-0.8%, +1.3%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.2 ms | 33.3 ms | +0.2% | [-0.6%, +1.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.1 ms | 56.9 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.5 ms | 20.6 ms | +0.4% | [-0.3%, +1.2%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.9 ms | 47.9 ms | -0.0% | [-0.7%, +0.7%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 62.8 ms | 61.4 ms | -2.2% | [-3.5%, -0.8%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 110 ms | +0.7% | [-2.3%, +3.8%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.27 ms | 8.26 ms | -0.1% | [-2.2%, +2.1%] | +0.0% | ~ | device 4.9x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.78 ms | 9.96 ms | +1.8% | [+1.4%, +2.1%] | +0.0% | ~ | device 8.1x faster than the host (was 8.4x) |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 388 µs | 388 µs | +0.1% | [-0.8%, +1.0%] | - | ~ | device 15.0x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 844 µs | 843 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.51 ms | 4.53 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.67 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 583 µs | 584 µs | +0.1% | [-0.4%, +0.6%] | - | ~ | device 13.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.37 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.76 ms | 6.78 ms | +0.3% | [-0.0%, +0.7%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.03 ms | 7.02 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 906 µs | 902 µs | -0.4% | [-0.6%, -0.3%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.23 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.44 ms | 9.48 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 850 µs | 851 µs | +0.1% | [-0.1%, +0.2%] | - | ~ | device 13.5x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.07 ms | 2.07 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 8.0x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.09 ms | 9.08 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.79 ms | 9.83 ms | +0.4% | [-0.2%, +0.9%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 315 µs | 313 µs | -0.6% | [-0.9%, -0.4%] | - | ~ | device 11.7x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.41 ms | 1.41 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 5.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.1 ms | 59.2 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 3.3x faster than the host (was 4.1x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.43 ms | 5.43 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 522 ms | 521 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.5 ms | 20.5 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 4.9x faster than the host (was 4.1x) |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.41 ms | 8.41 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 5.2x faster than the host; surface/leaf 2.44x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.6 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 8.2x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.20 ms | 6.20 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 10.0x faster than the host; surface/leaf 2.51x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 145 ms | 148 ms | +2.0% | [+0.5%, +3.5%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 59.9 ms | 60.0 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.46x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 72.0 ms | 72.0 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.3x faster than the host; surface/leaf 2.57x |
| `Domain/particles/labels/float/hostdevice` | 3.71 ms | 3.73 ms | +0.6% | [+0.1%, +1.0%] | - | ~ | device 3.8x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.91 ms | 2.91 ms | -0.2% | [-1.0%, +0.5%] | - | ~ | device 2.5x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 115 µs | 112 µs | -2.3% | [-4.1%, -0.4%] | - | ~ | device 13.2x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.74 ms | 7.52 ms | -2.8% | [-3.5%, -2.0%] | - | ~ | device 1.9x faster than the host (was 1.8x) |
| `Domain/particles/labelAt/double/hostdevice` | 3.32 ms | 3.33 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.2x faster than the host |
| `Domain/particles/surfaceGroups/double/hostdevice` | 117 µs | 116 µs | -0.7% | [-3.0%, +1.6%] | - | ~ | device 13.1x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 12.0 ms | 12.2 ms | +1.5% | [+0.7%, +2.2%] | - | ~ | device 3.7x faster than the host |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 9.47 ms | 9.46 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 2.5x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 241 µs | 247 µs | +2.1% | [+2.0%, +2.3%] | - | ~ | device 2.8x faster than the host; whole/own 49.50x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 171 µs | 171 µs | +0.0% | [-0.3%, +0.4%] | - | ~ | device 2.1x faster than the host (was 2.3x); whole/own 55.42x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 19.4 ms | 19.4 ms | +0.0% | [-0.6%, +0.6%] | - | ~ | device 2.4x faster than the host |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 10.8 ms | 10.8 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 2.3x faster than the host |
| `DomainObject/partsOwn/labels/double/hostdevice` | 413 µs | 406 µs | -1.7% | [-3.8%, +0.4%] | - | ~ | device 1.7x faster than the host (was 1.6x); whole/own 47.89x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 231 µs | 233 µs | +0.7% | [-0.3%, +1.8%] | - | ~ | device 1.6x faster than the host; whole/own 46.28x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/hostdevice` | 10.3 ms | 10.4 ms | +0.9% | [+0.0%, +1.8%] | +0.0% | ~ | device 9.9x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/hostdevice` | 10.4 ms | 10.4 ms | -0.1% | [-1.2%, +1.1%] | +0.0% | ~ | device 9.2x faster than the host; grid/morton 1.00x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/hostdevice` | 17.5 ms | 17.4 ms | -0.7% | [-1.6%, +0.1%] | +0.0% | ~ | device 6.9x faster than the host; grid/shuffled 0.60x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/hostdevice` | 225 ms | 220 ms | -2.0% | [-3.0%, -1.0%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/hostdevice` | 161 ms | 161 ms | -0.2% | [-2.4%, +2.1%] | +0.0% | ~ | device 0.6x faster than the host; grid/morton 1.37x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/hostdevice` | 286 ms | 284 ms | -0.6% | [-2.1%, +1.0%] | +0.0% | ~ | device 0.4x faster than the host; grid/shuffled 0.78x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/hostdevice` | 27.1 ms | 27.0 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ | device 4.0x faster than the host |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/hostdevice` | 29.2 ms | 29.2 ms | -0.1% | [-0.9%, +0.7%] | +0.0% | ~ | device 3.4x faster than the host; grid/morton 0.92x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/hostdevice` | 60.7 ms | 60.8 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | device 2.6x faster than the host (was 2.5x); grid/shuffled 0.44x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/hostdevice` | 204 ms | 202 ms | -1.3% | [-1.9%, -0.6%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/hostdevice` | 154 ms | 155 ms | +0.3% | [-1.6%, +2.2%] | +0.0% | ~ | device 0.6x faster than the host; grid/morton 1.30x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/hostdevice` | 250 ms | 250 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | device 0.6x faster than the host; grid/shuffled 0.81x |
| `Fused/csg/signedDistance/float/hostdevice` | 48.1 µs | 48.0 µs | -0.2% | [-0.4%, -0.0%] | - | ~ | device 23.1x faster than the host; tape/fused 4.76x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.5 µs | 49.8 µs | +0.6% | [-0.1%, +1.3%] | - | ~ | device 44.6x faster than the host; tape/fused 7.58x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.1% | [-0.4%, +0.1%] | - | ~ | device 9.3x faster than the host; tape/fused 10.64x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.5 ms | 17.4 ms | -0.1% | [-1.3%, +1.1%] | - | ~ | device 6.7x faster than the host; tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.4 ms | 31.4 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 282 ms | 281 ms | -0.2% | [-1.8%, +1.4%] | - | ~ | device 0.4x faster than the host (was 0.5x); tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 279 ms | 280 ms | +0.5% | [-1.1%, +2.1%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.27 ms | 8.32 ms | +0.6% | [-0.4%, +1.6%] | +0.0% | ~ | device 10.6x faster than the host (was 12.0x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.36 ms | 8.33 ms | -0.3% | [-1.6%, +1.0%] | +0.0% | ~ | device 10.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.19 ms | 8.21 ms | +0.2% | [-1.3%, +1.7%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.30 ms | 8.24 ms | -0.7% | [-2.0%, +0.6%] | +0.0% | ~ | device 10.8x faster than the host (was 11.6x); default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 687 µs | 686 µs | -0.2% | [-1.3%, +0.9%] | +0.0% | ~ | device 3.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.76 ms | 1.76 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.06 ms | 1.06 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ | device 2.4x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 797 µs | 799 µs | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | device 3.2x faster than the host (was 3.3x); default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 741 µs | 736 µs | -0.7% | [-1.4%, -0.0%] | +0.0% | ~ | device 12.1x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 830 µs | 826 µs | -0.5% | [-1.0%, +0.1%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 740 µs | 743 µs | +0.4% | [-0.7%, +1.6%] | +0.0% | ~ | device 12.0x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 724 µs | 731 µs | +0.9% | [-0.0%, +1.9%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 163 ms | +1.8% | [-1.5%, +5.2%] | +0.0% | ~ | device 0.5x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 162 ms | 161 ms | -0.4% | [-2.8%, +2.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 160 ms | 160 ms | -0.3% | [-1.5%, +1.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 160 ms | 162 ms | +1.6% | [-1.2%, +4.5%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.40 ms | 1.39 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.44 ms | 1.45 ms | +0.4% | [+0.2%, +0.7%] | +0.0% | ~ | device 1.8x faster than the host; default/this 0.96x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.92 ms | 1.91 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.73x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.56 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.24 ms | 7.26 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.29 ms | 7.28 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.24 ms | 7.24 ms | +0.0% | [-0.8%, +0.8%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.30 ms | 7.31 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.41 ms | 8.36 ms | -0.6% | [-1.5%, +0.3%] | - | ~ | device 11.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.7 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 7.1x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 166 ms | 163 ms | -1.9% | [-5.0%, +1.3%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 283 ms | 284 ms | +0.2% | [-0.9%, +1.4%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.8 ms | 24.5 ms | -1.0% | [-2.3%, +0.3%] | - | ~ | device 4.0x faster than the host (was 4.3x) |
| `MeshSDF/query-random/float/hostdevice` | 58.1 ms | 58.0 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 2.5x faster than the host (was 2.7x) |
| `MeshSDF/query-morton/double/hostdevice` | 155 ms | 157 ms | +0.9% | [-1.1%, +2.9%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 243 ms | 245 ms | +0.7% | [-0.5%, +1.9%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.06 ms | 2.06 ms | +0.1% | [-0.9%, +1.0%] | - | ~ | device 5.0x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.48 ms | 3.45 ms | -0.9% | [-2.0%, +0.2%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.21 ms | 6.19 ms | -0.4% | [-1.0%, +0.3%] | - | ~ | device 4.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 64.5 ms | 64.8 ms | +0.5% | [-0.2%, +1.1%] | - | ~ | device 3.5x faster than the host (was 3.1x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 94.9 ms | 95.0 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 239 ms | 235 ms | -1.5% | [-2.1%, -0.9%] | - | ~ | device 3.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.8 ms | 24.5 ms | -1.2% | [-2.1%, -0.3%] | - | ~ | device 2.0x faster than the host; tape/fused 2.65x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.3 ms | 26.8 ms | -1.7% | [-3.1%, -0.2%] | - | ~ | device 2.2x faster than the host; tape/fused 3.54x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.4 ms | 66.4 ms | +1.4% | [+1.3%, +1.6%] | - | ~ | device 3.8x faster than the host (was 4.0x); tape/fused 3.54x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.2 ms | 21.9 ms | +3.6% | [-5.7%, +13.1%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 39.5 ms | 39.2 ms | -0.8% | [-1.2%, -0.4%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 41.9 ms | 42.1 ms | +0.5% | [-1.7%, +2.8%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 534 ms | 541 ms | +1.1% | [-5.7%, +6.3%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 858 ms | 852 ms | -0.6% | [-3.9%, +2.8%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.94 s | 1.97 s | +1.4% | [+1.2%, +1.7%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 197 ms | 203 ms | +2.7% | [-1.4%, +9.5%] | - | ~ | device 0.2x faster than the host; tape/fused 2.67x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 299 ms | 303 ms | +1.5% | [-1.9%, +4.9%] | - | ~ | device 0.2x faster than the host; tape/fused 2.81x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 543 ms | 552 ms | +1.7% | [+0.1%, +3.3%] | - | ~ | device 0.5x faster than the host; tape/fused 3.56x |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 12.0 ms | 12.0 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 9.9x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 58.6 ms | 58.5 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 3.0x faster than the host (was 2.5x) |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 259 ms | 263 ms | +1.4% | [+0.1%, +2.7%] | - | ~ | device 0.6x faster than the host (was 0.5x) |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 245 ms | 245 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 15.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.31 ms | 5.31 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | device 10.5x faster than the host |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.32 ms | 1.32 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 15.6x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.32 ms | 1.31 ms | -0.5% | [-1.3%, +0.3%] | - | ~ | device 15.7x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.13 ms | 2.14 ms | +0.8% | [+0.4%, +1.3%] | - | ~ | device 7.9x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.52 ms | 3.52 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 10.3x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.77 ms | 2.77 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 7.8x faster than the host; mesh/tape 0.47x |
| `MeshTape/tape/frame/float/hostdevice` | 4.44 ms | 4.46 ms | +0.5% | [+0.1%, +0.8%] | - | ~ | device 4.9x faster than the host; mesh/tape 0.48x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.75 ms | 3.75 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 9.7x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.73 ms | 5.73 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 4.1x faster than the host; mesh/view 0.37x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 30.2 ms | 30.2 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 30.2 ms | 30.2 ms | +0.2% | [-0.3%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 25.9 ms | 25.9 ms | +0.2% | [-1.0%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 43.4 ms | 43.4 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.69x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 30.6 ms | 30.6 ms | -0.0% | [-0.7%, +0.7%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 33.0 ms | 33.1 ms | +0.3% | [-0.4%, +1.1%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.78x |
| `MeshTape/view/firstHit/double/hostdevice` | 43.3 ms | 43.6 ms | +0.5% | [+0.2%, +0.9%] | - | ~ | device 0.9x faster than the host; mesh/view 0.69x |
| `MeshTape/view/frame/double/hostdevice` | 36.1 ms | 35.9 ms | -0.5% | [-0.7%, -0.3%] | - | ~ | device 0.7x faster than the host; mesh/view 0.72x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 680 µs | 685 µs | +0.7% | [+0.1%, +1.3%] | - | ~ | device 11.0x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.82 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 995 µs | 996 µs | +0.1% | [-0.1%, +0.4%] | - | ~ | device 9.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.60 ms | 4.60 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.97 ms | 2.96 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.06 ms | 7.07 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.80 ms | 7.77 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.4 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.46 ms | 1.47 ms | +0.8% | [-1.2%, +2.9%] | - | ~ | device 6.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.90 ms | 3.89 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 525 µs | 524 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 15.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.50 ms | 4.52 ms | +0.3% | [-0.4%, +1.0%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.25 ms | 4.21 ms | -0.7% | [-1.8%, +0.3%] | - | ~ | device 5.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.06 ms | 8.12 ms | +0.8% | [-0.6%, +2.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.7 ms | 15.8 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.02 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.6 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.9 ms | 14.9 ms | -0.5% | [-1.8%, +0.8%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.59 ms | +1.0% | [+0.3%, +1.7%] | - | ~ | device 10.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.61 ms | 4.60 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 832 µs | 833 µs | +0.1% | [-0.3%, +0.6%] | - | ~ | device 10.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.86 ms | +0.2% | [-0.0%, +0.3%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.14 ms | 4.12 ms | -0.3% | [-0.4%, -0.3%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.6% | [-2.4%, +1.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 21.1 ms | +0.2% | [-0.0%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.94 ms | 8.96 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.6 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.2 ms | 20.3 ms | +0.6% | [-0.3%, +1.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.74 ms | 2.79 ms | +1.7% | [+0.5%, +3.1%] | - | ~ | device 4.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.72 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.1% | [-1.1%, +1.2%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.73 ms | 2.73 ms | +0.1% | [-0.7%, +0.8%] | - | ~ | device 7.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.39 ms | 6.42 ms | +0.5% | [-0.0%, +1.0%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.7 ms | 33.8 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.4 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.5 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.7 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.4 ms | 38.2 ms | -0.6% | [-1.4%, +0.3%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.65 ms | 9.67 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.8 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.28 ms | 3.26 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ | device 3.7x faster than the host (was 4.6x) |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.56 ms | 2.60 ms | +1.6% | [+1.5%, +1.7%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.58 ms | 4.56 ms | -0.6% | [-1.4%, +0.3%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.04 ms | 3.06 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ | device 3.3x faster than the host |
| `QueryOrder/random/mortonOrder/float/hostdevice` | 16.9 ms | 16.9 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 3.5x faster than the host |
| `QueryOrder/armadillo/distance-arrived/float/hostdevice` | 9.18 ms | 9.22 ms | +0.4% | [+0.0%, +0.7%] | +0.0% | ~ | device 4.9x faster than the host |
| `QueryOrder/armadillo/distance-ordered/float/hostdevice` | 7.31 ms | 7.41 ms | +1.4% | [+0.6%, +2.2%] | +0.0% | ~ | device 5.4x faster than the host; arrived/ordered 1.24x |
| `QueryOrder/random/mortonOrder/double/hostdevice` | 17.6 ms | 17.6 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 3.4x faster than the host |
| `QueryOrder/armadillo/distance-arrived/double/hostdevice` | 113 ms | 112 ms | -0.8% | [-2.7%, +1.1%] | +0.0% | ~ | device 0.4x faster than the host |
| `QueryOrder/armadillo/distance-ordered/double/hostdevice` | 80.2 ms | 80.8 ms | +0.8% | [-4.7%, +6.6%] | +0.0% | ~ | device 0.5x faster than the host; arrived/ordered 1.39x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 702 µs | 713 µs | +1.6% | [+1.4%, +1.8%] | - | ~ | device 17.1x faster than the host; hand/api 0.91x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.31 ms | 1.31 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 15.4x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 651 µs | 650 µs | -0.1% | [-1.4%, +1.2%] | - | ~ | device 17.5x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.23 ms | +0.4% | [+0.0%, +0.7%] | - | ~ | device 15.5x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 625 µs | 628 µs | +0.5% | [-0.1%, +1.2%] | - | ~ | device 15.7x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.16 ms | 1.16 ms | -0.1% | [-0.9%, +0.6%] | - | ~ | device 14.5x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.20 ms | 1.20 ms | -0.2% | [-0.2%, -0.2%] | - | ~ | device 11.8x faster than the host; triangles/polygons 0.60x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.49 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 9.6x faster than the host; triangles/polygons 0.53x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 11.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.17 ms | 2.18 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 602 µs | 601 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 2.5x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 704 µs | 740 µs | +5.1% | [-0.2%, +10.9%] | - | ~ | device 2.4x faster than the host (was 2.6x); tape/fused 1.13x (was 1.19x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 671 µs | 670 µs | -0.2% | [-1.3%, +1.0%] | - | ~ | device 3.7x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 838 µs | 834 µs | -0.4% | [-1.0%, +0.2%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 18.6 ms | 18.7 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 0.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 30.4 ms | 30.2 ms | -0.4% | [-0.6%, -0.3%] | - | ~ | device 0.7x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 18.0 ms | 18.2 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 29.3 ms | 29.1 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.9 ms | 15.8 ms | -1.1% | [-1.6%, -0.7%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 26.0 ms | 25.9 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 17.2 ms | 17.1 ms | -0.7% | [-1.2%, -0.1%] | - | ~ | device 0.8x faster than the host; triangles/polygons 1.10x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.6 ms | 25.5 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.19x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.1 ms | 15.0 ms | -0.9% | [-2.6%, +0.9%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | +0.0% | [-0.6%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.24 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 1.3x faster than the host; tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.36 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 0.8x faster than the host; tape/fused 1.06x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 2.9x faster than the host; marched/traced 3.53x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.98 ms | 2.98 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 2.7x faster than the host; marched/traced 4.65x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.00 ms | 8.98 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 4.2x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.9 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.25 ms | 2.25 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 3.8x faster than the host; marched/traced 12.82x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.52 ms | 5.51 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.9x faster than the host (was 2.8x); marched/traced 9.85x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.0 ms | 28.9 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 54.1 ms | 54.3 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 2.1x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.32 ms | 5.32 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.94 ms | 8.92 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.18 ms | 6.17 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.1x faster than the host (was 1.3x); marched/traced 26.32x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.5 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 0.7x faster than the host (was 0.8x); marched/traced 24.45x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 158 ms | 162 ms | +2.6% | [+0.3%, +4.9%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 255 ms | 258 ms | +0.8% | [-0.1%, +1.8%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.86 ms | 4.85 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 22.92x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.1 ms | 12.2 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 1.3x faster than the host; marched/traced 17.87x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 112 ms | 111 ms | -1.0% | [-3.0%, +1.1%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 213 ms | 218 ms | +2.7% | [+1.3%, +4.1%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.2 ms | -0.4% | [-1.0%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.7 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 709 µs | 699 µs | -1.5% | [-2.5%, -0.5%] | - | ~ | device 17.4x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.31 ms | 1.31 ms | -0.2% | [-0.9%, +0.4%] | - | ~ | device 15.3x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 706 µs | 709 µs | +0.5% | [-0.8%, +1.8%] | - | ~ | device 17.2x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.31 ms | 1.31 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 15.3x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 676 µs | 677 µs | +0.2% | [-0.2%, +0.5%] | - | ~ | device 3.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 839 µs | 838 µs | -0.1% | [-1.2%, +0.9%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 614 µs | 614 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 3.2x faster than the host; hit/crossing 1.10x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 640 µs | 638 µs | -0.4% | [-0.8%, -0.0%] | - | ~ | device 3.7x faster than the host; hit/crossing 1.31x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.78 ms | 4.78 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.3 ms | 10.3 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 6.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.23 ms | 4.24 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 9.9x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.15 ms | 9.16 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 57.5 ms | 57.5 ms | -0.0% | [-2.3%, +2.3%] | - | ~ | device 3.0x faster than the host (was 3.3x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 83.3 ms | 83.5 ms | +0.2% | [-0.6%, +1.0%] | - | ~ | device 2.7x faster than the host (was 2.3x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 49.4 ms | 49.0 ms | -0.7% | [-1.1%, -0.4%] | - | ~ | device 3.0x faster than the host; hit/crossing 1.17x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 64.0 ms | 64.4 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 2.4x faster than the host; hit/crossing 1.30x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 18.7 ms | 18.7 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 30.3 ms | 30.4 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 18.6 ms | 18.6 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 30.2 ms | 30.2 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.49 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.12 ms | 1.12 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.8x faster than the host; hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.17 ms | 1.17 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.13x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 68.8 ms | 68.6 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 131 ms | 130 ms | -1.3% | [-1.8%, -0.7%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 62.2 ms | 62.1 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 118 ms | 118 ms | -0.1% | [-2.3%, +2.1%] | - | ~ | device 0.5x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 459 ms | 448 ms | -2.5% | [-7.9%, +3.4%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 710 ms | 702 ms | -1.1% | [-2.0%, -0.1%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 409 ms | 397 ms | -3.0% | [-5.9%, +0.0%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 532 ms | 523 ms | -1.7% | [-4.1%, +0.8%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.34x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.54 ms | 3.58 ms | +1.3% | [-0.6%, +3.1%] | - | ~ | device 1.2x faster than the host; marched/traced 3.38x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.76 ms | 4.78 ms | +0.4% | [-0.5%, +1.2%] | - | ~ | device 0.9x faster than the host; marched/traced 5.58x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.1 ms | 12.1 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.8 ms | 26.7 ms | -0.5% | [-0.6%, -0.4%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.23 ms | 1.23 ms | -0.2% | [-1.4%, +1.1%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.85 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 99.0 µs | 95.6 µs | -3.5% | [-5.2%, -1.7%] | - | faster | device 15.5x faster than the host (was 14.8x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 155 µs | 154 µs | -0.4% | [-1.7%, +1.0%] | - | ~ | device 11.0x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 804 µs | 805 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 941 µs | 941 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.4 ms | 34.9 ms | +1.3% | [-3.4%, +6.4%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 3.50x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.3 ms | 36.4 ms | +0.4% | [-0.2%, +1.1%] | - | ~ | device 0.1x faster than the host; marched/traced 6.36x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 120 ms | 122 ms | +1.9% | [-0.2%, +4.0%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 232 ms | 232 ms | -0.3% | [-1.4%, +0.9%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 17.7 ms | 15.8 ms | -11.0% | [-14.4%, -7.4%] | - | faster | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 21.8 ms | -0.4% | [-1.3%, +0.6%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.12 ms | 1.18 ms | +5.0% | [-1.8%, +11.9%] | - | ~ | device 1.3x faster than the host (was 1.3x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.55 ms | 2.53 ms | -0.6% | [-2.1%, +0.8%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 5.98 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.74 ms | 6.70 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.3 ms | 23.2 ms | -0.4% | [-1.1%, +0.4%] | - | ~ | device 2.2x faster than the host; marched/traced 2.71x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.3 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.0x faster than the host; marched/traced 3.77x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.6 ms | 62.8 ms | +0.2% | [-0.5%, +1.0%] | - | ~ | device 3.5x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 106 ms | 107 ms | +0.8% | [+0.2%, +1.5%] | - | ~ | device 2.2x faster than the host (was 2.5x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.9 ms | 13.7 ms | -1.4% | [-1.7%, -1.1%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.5 ms | 17.5 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 309 ms | 304 ms | -1.5% | [-5.3%, +2.6%] | - | ~ | device 0.2x faster than the host; marched/traced 1.89x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 347 ms | 351 ms | +1.1% | [-3.0%, +5.4%] | - | ~ | device 0.2x faster than the host; marched/traced 2.82x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 584 ms | 576 ms | -1.5% | [-4.1%, +1.3%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1000 ms | 990 ms | -1.0% | [-2.2%, +0.2%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 190 ms | 192 ms | +0.8% | [-2.3%, +3.9%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 237 ms | 240 ms | +1.3% | [-4.0%, +6.7%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.80 ms | 2.79 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 2.7x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.32 ms | 3.33 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 4.0x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 80.3 ms | 80.2 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 3.7x faster than the host (was 3.9x); one-pass/each 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.43 ms | 3.39 ms | -1.1% | [-1.3%, -0.8%] | - | ~ | device 4.0x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.83 ms | 6.84 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.53 ms | 5.53 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.2x faster than the host; regions/hit 1.24x |
| `Regions/particles/region/double/hostdevice` | 3.57 ms | 3.56 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 2.1x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.96 ms | 5.89 ms | -1.1% | [-1.7%, -0.5%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 127 ms | 124 ms | -2.4% | [-3.0%, -1.8%] | - | ~ | device 2.9x faster than the host (was 2.4x); one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/hostdevice` | 6.09 ms | 6.08 ms | -0.1% | [-0.8%, +0.5%] | - | ~ | device 2.2x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.7 ms | 15.8 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 12.9 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 1.3x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.46 ms | 2.47 ms | +0.3% | [-0.0%, +0.5%] | - | ~ | device 2.9x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.44 ms | 2.45 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 2.9x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.15 ms | 2.14 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 7.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.66 ms | 5.67 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.67 ms | 5.67 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 25.9 ms | 25.8 ms | -0.5% | [-1.8%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `ShapeFamilies/icosahedron/signedDistance/float/hostdevice` | 393 µs | 393 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 17.0x faster than the host |
| `ShapeFamilies/pyramid/signedDistance/float/hostdevice` | 207 µs | 207 µs | +0.3% | [+0.2%, +0.5%] | - | ~ | device 23.0x faster than the host |
| `ShapeFamilies/box-frame/signedDistance/float/hostdevice` | 48.2 µs | 48.0 µs | -0.5% | [-0.7%, -0.2%] | - | ~ | device 25.9x faster than the host |
| `ShapeFamilies/hyperboloid/signedDistance/float/hostdevice` | 270 µs | 269 µs | -0.4% | [-0.5%, -0.2%] | - | ~ | device 18.7x faster than the host |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/float/hostdevice` | 277 µs | 277 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 17.7x faster than the host |
| `ShapeFamilies/paraboloid/signedDistance/float/hostdevice` | 48.7 µs | 48.2 µs | -0.9% | [-2.1%, +0.3%] | - | ~ | device 66.2x faster than the host |
| `ShapeFamilies/superellipsoid/signedDistance/float/hostdevice` | 90.6 µs | 90.5 µs | -0.1% | [-0.4%, +0.3%] | - | ~ | device 40.7x faster than the host |
| `ShapeFamilies/solid-angle/signedDistance/float/hostdevice` | 47.7 µs | 47.7 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 10.0x faster than the host |
| `ShapeFamilies/vesica/signedDistance/float/hostdevice` | 48.2 µs | 48.0 µs | -0.4% | [-0.9%, +0.1%] | - | ~ | device 17.6x faster than the host |
| `ShapeFamilies/helix/signedDistance/float/hostdevice` | 297 µs | 297 µs | -0.1% | [-0.1%, +0.0%] | - | ~ | device 31.9x faster than the host |
| `ShapeFamilies/gyroid/signedDistance/float/hostdevice` | 47.8 µs | 48.1 µs | +0.5% | [+0.1%, +0.9%] | - | ~ | device 37.3x faster than the host (was 38.8x) |
| `ShapeFamilies/ray-forms/firstHit/float/hostdevice` | 2.26 ms | 2.25 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 3.6x faster than the host |
| `ShapeFamilies/icosahedron/signedDistance/double/hostdevice` | 8.51 ms | 8.51 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `ShapeFamilies/pyramid/signedDistance/double/hostdevice` | 4.46 ms | 4.46 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `ShapeFamilies/box-frame/signedDistance/double/hostdevice` | 908 µs | 906 µs | -0.2% | [-0.4%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `ShapeFamilies/hyperboloid/signedDistance/double/hostdevice` | 25.0 ms | 25.0 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 0.3x faster than the host |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/double/hostdevice` | 25.2 ms | 25.2 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 0.3x faster than the host |
| `ShapeFamilies/paraboloid/signedDistance/double/hostdevice` | 1.77 ms | 1.77 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.2x faster than the host |
| `ShapeFamilies/superellipsoid/signedDistance/double/hostdevice` | 5.00 ms | 5.00 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.2x faster than the host |
| `ShapeFamilies/solid-angle/signedDistance/double/hostdevice` | 380 µs | 380 µs | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.5x faster than the host (was 1.6x) |
| `ShapeFamilies/vesica/signedDistance/double/hostdevice` | 885 µs | 885 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.2x faster than the host |
| `ShapeFamilies/helix/signedDistance/double/hostdevice` | 14.6 ms | 14.6 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 0.9x faster than the host |
| `ShapeFamilies/gyroid/signedDistance/double/hostdevice` | 1.13 ms | 1.13 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 3.5x faster than the host |
| `ShapeFamilies/ray-forms/firstHit/double/hostdevice` | 10.4 ms | 10.4 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 155 µs | 155 µs | +0.0% | [+0.0%, +0.1%] | - | ~ | device 33.1x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.3 µs | 47.5 µs | +0.4% | [+0.4%, +0.4%] | - | ~ | device 18.7x faster than the host |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.0% | [-0.1%, +0.2%] | - | ~ | device 14.8x faster than the host (was 16.0x) |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 48.2 µs | 48.3 µs | +0.2% | [-0.4%, +0.8%] | - | ~ | device 63.9x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.1% | [-0.4%, +0.7%] | - | ~ | device 15.9x faster than the host |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 48.1 µs | 48.0 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 17.0x faster than the host |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.3% | [+0.1%, +0.4%] | - | ~ | device 9.7x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.9 µs | 47.7 µs | -0.4% | [-0.8%, +0.0%] | - | ~ | device 7.4x faster than the host |
| `Shapes/wedge/signedDistance/float/hostdevice` | 47.9 µs | 47.6 µs | -0.6% | [-1.7%, +0.5%] | - | ~ | device 8.4x faster than the host |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 558 µs | 556 µs | -0.4% | [-0.8%, +0.0%] | - | ~ | device 8.2x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.23 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 492 µs | 492 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.1x faster than the host |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 422 µs | 422 µs | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.9x faster than the host |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 358 µs | 358 µs | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.4x faster than the host (was 2.2x) |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 669 µs | 669 µs | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.6x faster than the host |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 414 µs | 413 µs | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.4x faster than the host (was 1.3x) |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 339 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.4x faster than the host (was 1.5x) |
| `Shapes/wedge/signedDistance/double/hostdevice` | 155 µs | 154 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.04 ms | 8.01 ms | -0.3% | [-0.7%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 891 µs | 891 µs | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ | device 10.4x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.67 ms | 2.66 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.6 ms | 13.5 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.3 ms | 14.3 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.1 ms | 11.0 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 2.99 ms | 2.99 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 7.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.74 ms | 4.74 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ | device 5.1x faster than the host; tape/fused 1.38x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.0 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.27x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 23.1 ms | 23.0 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.5 ms | 30.5 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.1 ms | 44.1 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.9 ms | 45.8 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 125 ms | 125 ms | -0.3% | [-3.3%, +2.9%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.02x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 126 ms | 126 ms | +0.2% | [-2.2%, +2.7%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 129 ms | 127 ms | -1.5% | [-2.1%, -0.8%] | +0.0% | ~ | device 0.7x faster than the host (was 0.7x) |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 134 ms | 132 ms | -1.9% | [-4.1%, +0.3%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | device 10.6x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.2 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | device 0.6x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.31 ms | 9.30 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 13.1x faster than the host |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.92 ms | 5.92 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 13.1x faster than the host (was 10.6x); surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 96.9 ms | 96.8 ms | -0.1% | [-2.4%, +2.2%] | - | ~ | device 3.7x faster than the host (was 4.1x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.6 ms | 63.6 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 3.2x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 30.0 ms | 30.0 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 5.0x faster than the host (was 7.1x) |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.98 ms | 9.98 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 7.0x faster than the host; surface/hit 3.01x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.1 ms | 59.0 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 2.0x faster than the host (was 1.8x) |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.5x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 784 ms | 787 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 0.4x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 527 ms | 528 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.49x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 320 ms | 312 ms | -2.6% | [-3.5%, -1.6%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 129 ms | 127 ms | -1.0% | [-1.7%, -0.3%] | - | ~ | device 0.6x faster than the host (was 0.6x); surface/hit 2.45x |
| `Tape/csg/signedDistance/float/hostdevice` | 229 µs | 229 µs | +0.1% | [-0.1%, +0.2%] | - | ~ | device 10.6x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 47.8 µs | +0.2% | [+0.1%, +0.2%] | - | ~ | device 23.6x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 379 µs | 377 µs | -0.3% | [-0.9%, +0.2%] | - | ~ | device 12.1x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 9.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 75.5 µs | 75.4 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 6.6x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.6 µs | -0.5% | [-1.5%, +0.4%] | - | ~ | device 4.8x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 653 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.2% | [-0.1%, +0.4%] | - | ~ | device 11.9x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 508 µs | 508 µs | +0.2% | [-0.0%, +0.3%] | - | ~ | device 7.1x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.2% | [-0.3%, +0.7%] | - | ~ | device 26.4x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 859 µs | 859 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.64 ms | 6.63 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 192 µs | 192 µs | +0.2% | [+0.0%, +0.4%] | - | ~ | device 3.1x faster than the host (was 2.9x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 164 µs | +0.5% | [-0.2%, +1.2%] | - | ~ | device 1.7x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.59 ms | 1.59 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 8.2x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.79 ms | 3.80 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 10.6x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.51 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.7 ms | 17.7 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.5x faster than the host |
| `Weld/particles/region/float/hostdevice` | - | - | - | - | - | new |  |
| `Weld/particles/regionDistances/float/hostdevice` | - | - | - | - | - | new |  |
| `Weld/blocks/region/float/hostdevice` | - | - | - | - | - | new |  |
| `Weld/blocks/closestSurface/float/hostdevice` | - | - | - | - | - | new |  |
| `Weld/particles/region/double/hostdevice` | - | - | - | - | - | new |  |
| `Weld/particles/regionDistances/double/hostdevice` | - | - | - | - | - | new |  |
| `Weld/blocks/region/double/hostdevice` | - | - | - | - | - | new |  |
| `Weld/blocks/closestSurface/double/hostdevice` | - | - | - | - | - | new |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 6644.6 s → 4712.7 s (-29.1%), the largest peak 5380 MB → 5377 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 17.3 s | 13.8 s | -20.3% | 634 MB | 636 MB | +0.3% | ~ |
| `BVHRefit` | 10.9 s | 7.3 s | -32.9% | 536 MB | 537 MB | +0.3% | ~ |
| `BVHUnion` | 34.7 s | 26.1 s | -24.8% | 1057 MB | 1059 MB | +0.2% | ~ |
| `BVHUnionMixed` | 16.2 s | 11.5 s | -28.7% | 652 MB | 654 MB | +0.3% | ~ |
| `ClosestPrimitive` | 180.1 s | 111.6 s | -38.0% | 1546 MB | 1548 MB | +0.2% | ~ |
| `ClosestSurface` | 208.0 s | 141.3 s | -32.0% | 1422 MB | 1424 MB | +0.2% | ~ |
| `ConnectedComponents` | 28.5 s | 22.1 s | -22.6% | 1047 MB | 1050 MB | +0.2% | ~ |
| `Domain` | 81.4 s | 54.4 s | -33.1% | 1121 MB | 1124 MB | +0.2% | ~ |
| `DomainObject` | 71.6 s | 52.3 s | -27.0% | 1056 MB | 1058 MB | +0.2% | ~ |
| `EBGrid` | 26.2 s | 20.3 s | -22.7% | 782 MB | 784 MB | +0.2% | ~ |
| `Fused` | 82.2 s | 53.9 s | -34.4% | 1276 MB | 1278 MB | +0.1% | ~ |
| `LaunchTuning` | 624.5 s | 449.9 s | -28.0% | 3080 MB | 3082 MB | +0.1% | ~ |
| `Mesh` | 30.2 s | 22.4 s | -25.8% | 840 MB | 841 MB | +0.2% | ~ |
| `MeshCSG` | 491.7 s | 359.3 s | -26.9% | 4935 MB | 4936 MB | +0.0% | ~ |
| `MeshClosestPoint` | 29.9 s | 19.7 s | -34.1% | 792 MB | 793 MB | +0.2% | ~ |
| `MeshIngest` | 18.1 s | 11.0 s | -39.1% | 566 MB | 568 MB | +0.3% | ~ |
| `MeshReconcile` | 15.1 s | 10.8 s | -28.5% | 544 MB | 545 MB | +0.3% | ~ |
| `MeshTape` | 919.9 s | 653.8 s | -28.9% | 5380 MB | 5377 MB | -0.0% | ~ |
| `PointCloud` | 20.0 s | 13.2 s | -33.9% | 596 MB | 598 MB | +0.3% | ~ |
| `PointCloudBVHBuild` | 11.5 s | 7.5 s | -34.2% | 536 MB | 538 MB | +0.4% | ~ |
| `PointCloudHashGridBuild` | 8.3 s | 6.8 s | -17.6% | 536 MB | 538 MB | +0.4% | ~ |
| `QueryOrder` | 27.6 s | 16.6 s | -39.9% | 723 MB | 725 MB | +0.2% | ~ |
| `Ray` | 215.1 s | 165.5 s | -23.1% | 3002 MB | 3003 MB | +0.0% | ~ |
| `RayCSG` | 276.5 s | 198.0 s | -28.4% | 3266 MB | 3266 MB | -0.0% | ~ |
| `RayCrossing` | 522.5 s | 389.0 s | -25.5% | 3258 MB | 3256 MB | -0.1% | ~ |
| `RayParticles` | 591.6 s | 416.9 s | -29.5% | 4711 MB | 4711 MB | +0.0% | ~ |
| `RayShell` | 302.9 s | 241.7 s | -20.2% | 4477 MB | 4477 MB | +0.0% | ~ |
| `Regions` | 467.6 s | 317.1 s | -32.2% | 3345 MB | 3345 MB | -0.0% | ~ |
| `Render` | 343.7 s | 234.1 s | -31.9% | 4745 MB | 4745 MB | +0.0% | ~ |
| `ShapeFamilies` | 26.0 s | 23.6 s | -9.3% | 603 MB | 605 MB | +0.2% | ~ |
| `Shapes` | 33.3 s | 23.9 s | -28.2% | 617 MB | 618 MB | +0.3% | ~ |
| `SphereCloudBVH` | 9.6 s | 9.4 s | -2.5% | 536 MB | 538 MB | +0.3% | ~ |
| `SphereCloudRays` | 242.3 s | 157.3 s | -35.1% | 3221 MB | 3257 MB | +1.1% | ~ |
| `SphereContacts` | 15.4 s | 8.6 s | -44.2% | 536 MB | 537 MB | +0.3% | ~ |
| `SurfaceParts` | 492.0 s | 337.9 s | -31.3% | 3400 MB | 3400 MB | +0.0% | ~ |
| `Tape` | 92.9 s | 60.4 s | -35.0% | 1089 MB | 1091 MB | +0.2% | ~ |
| `TapeBVHUnion` | 59.2 s | 43.5 s | -26.6% | 1030 MB | 1032 MB | +0.2% | ~ |
| `Weld` | - | 80.7 s | | - | 1126 MB | | new |
| **all, in both** | 6644.6 s | 4712.7 s | -29.1% | 5380 MB | 5377 MB | -0.0% | ~ |

Device kernels: 232 in both, 10 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 6 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `Domain` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)0>, Vec3T<double> const,…` | 168 → 168 | 4512 → 4528 | 256/300 → 228/272 | ~ |
| `Domain` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)0>, Vec3T<float> const, A…` | 168 → 168 | 3104 → 3104 | 184/260 → 156/232 | ~ |
| `DomainObject` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)0>, Vec3T<double> const,…` | 168 → 168 | 4256 → 4272 | 256/300 → 228/272 | ~ |
| `DomainObject` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)0>, Vec3T<float> const, A…` | 168 → 168 | 2976 → 2976 | 184/260 → 156/232 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)1>, Vec3T<double> const,…` | 168 → 168 | 4240 → 4256 | 368/380 → 340/352 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)2>, Vec3T<double> const,…` | 168 → 168 | 4208 → 4208 | 44/28 → 40/24 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)3>, Vec3T<double> const,…` | 168 → 168 | 4272 → 4288 | 256/300 → 228/272 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)1>, Vec3T<float> const, R…` | 168 → 168 | 2944 → 2944 | 240/296 → 212/268 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)2>, Vec3T<float> const, R…` | 168 → 168 | 2896 → 2896 | 32/20 → 28/16 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)3>, Vec3T<float> const, R…` | 168 → 168 | 2976 → 2976 | 184/260 → 156/232 | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): MeshCSG/smooth-tape/firstHit-morton/double/hostdevice, MeshCSG/smooth-fused/firstHit-morton/double/hostdevice

The probes differ between the sides (each side ran its own): Performance/Weld/main.cpp

<!-- END receipt cuda -->
