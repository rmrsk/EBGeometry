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
base: b4fb4c6
base-fingerprint: 6c3c437020716e933adeba2f122c30695cb6e811
head: 9cab426
head-fingerprint: 570f0a4e56d677aab74e9c092b0d5dfeac3ddaf9
cases: 316
rounds: 2
reps: 3
date: 2026-11-21
-->

## Host comparison: green

**Accepted:** -

`9cab426` against `b4fb4c6` (origin/main), 2026-11-21.

**VERDICT: no regression (0 major, 0 minor across 316 cases)**

Performance comparison: 0 major, 0 minor, 4 faster, 0 differ, 0 lost, 316 cases

- base: b4fb4c6 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 9cab426 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 332 ms | 332 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 515 ms | 515 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 348 ms | 349 ms | +0.4% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 563 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 42.4 ms | 41.3 ms | -2.7% | [-6.3%, +1.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.0 ms | 75.2 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.2 ms | 56.0 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.5 ms | 51.8 ms | +0.5% | [-0.9%, +1.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.9 ms | 65.2 ms | +0.5% | [-0.7%, +1.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.7 ms | 55.0 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.0 ms | 60.5 ms | -0.9% | [-1.3%, -0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.4 ms | 72.4 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 72.0 ms | 72.0 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 78.1 ms | 77.2 ms | -1.2% | [-2.1%, -0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 64.6 ms | 64.5 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 100 ms | +0.0% | [-0.9%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 76.9 ms | 77.2 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 108 ms | -0.4% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 63.7 ms | 63.8 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 95.4 ms | 96.2 ms | +0.8% | [-0.2%, +1.7%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 76.1 ms | 75.9 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 102 ms | 101 ms | -1.4% | [-2.4%, -0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.5 ms | 19.8 ms | +1.5% | [+0.7%, +2.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.3 ms | 41.0 ms | -0.6% | [-0.8%, -0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 79.3 ms | 72.8 ms | -8.3% | [-15.3%, -0.0%] | +0.0% | faster |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.2 ms | 19.9 ms | -1.5% | [-2.0%, -1.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.6 ms | 43.6 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.8 ms | 81.4 ms | -0.4% | [-2.8%, +2.0%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 139 ms | 140 ms | +0.2% | [-1.1%, +1.4%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 76.5 ms | 75.5 ms | -1.2% | [-2.5%, +0.2%] | +0.0% | ~ | surface/leaf 1.85x |
| `ClosestSurface/union/closestSurface/float/host` | 146 ms | 146 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 78.4 ms | 76.3 ms | -2.7% | [-5.0%, -0.2%] | +0.0% | ~ | surface/leaf 1.92x |
| `ClosestSurface/cloud/closestSurface/double/host` | 157 ms | 157 ms | -0.1% | [-1.0%, +0.9%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 80.0 ms | 80.1 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | surface/leaf 1.96x |
| `ClosestSurface/union/closestSurface/double/host` | 154 ms | 153 ms | -0.5% | [-1.2%, +0.1%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 80.4 ms | 81.1 ms | +0.8% | [+0.1%, +1.5%] | +0.0% | ~ | surface/leaf 1.89x |
| `Domain/particles/labels/float/host` | 13.5 ms | 13.5 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 7.44 ms | 6.77 ms | -9.1% | [-16.9%, +0.2%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.35 ms | 1.36 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.0 ms | 14.0 ms | +0.0% | [-1.6%, +1.6%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.09 ms | 7.15 ms | +0.8% | [-0.0%, +1.6%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.35 ms | 1.36 ms | +0.4% | [+0.2%, +0.7%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 39.4 ms | 39.5 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.2 ms | 21.3 ms | +0.1% | [-1.1%, +1.4%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 559 µs | 558 µs | -0.2% | [-2.1%, +1.8%] | +0.0% | ~ | whole/own 70.77x |
| `DomainObject/partsOwn/labelAt/float/host` | 303 µs | 304 µs | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ | whole/own 69.83x |
| `DomainObject/partsWhole/labels/double/host` | 39.2 ms | 39.2 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.4 ms | 22.5 ms | +0.1% | [-0.9%, +1.0%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 582 µs | 587 µs | +0.8% | [-0.5%, +2.1%] | +0.0% | ~ | whole/own 66.90x |
| `DomainObject/partsOwn/labelAt/double/host` | 315 µs | 315 µs | -0.0% | [-1.1%, +1.1%] | +0.0% | ~ | whole/own 71.37x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/host` | 63.5 ms | 63.3 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/host` | 59.4 ms | 59.2 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/host` | 77.4 ms | 77.1 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ | grid/shuffled 0.82x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/host` | 67.9 ms | 67.9 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/host` | 63.2 ms | 63.5 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/host` | 88.5 ms | 89.1 ms | +0.6% | [-0.3%, +1.6%] | +0.0% | ~ | grid/shuffled 0.76x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/host` | 194 ms | 190 ms | -2.2% | [-3.5%, -0.8%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/host` | 177 ms | 179 ms | +0.8% | [+0.1%, +1.5%] | +0.0% | ~ | grid/morton 1.06x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/host` | 255 ms | 257 ms | +1.0% | [+0.1%, +1.9%] | +0.0% | ~ | grid/shuffled 0.74x (was 0.76x) |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/host` | 201 ms | 201 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/host` | 187 ms | 187 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/host` | 288 ms | 290 ms | +0.8% | [+0.4%, +1.1%] | +0.0% | ~ | grid/shuffled 0.69x |
| `Fused/csg/signedDistance/float/host` | 4.50 ms | 4.46 ms | -0.9% | [-1.9%, +0.1%] | - | ~ | tape/fused 2.91x |
| `Fused/smooth/signedDistance/float/host` | 12.5 ms | 12.3 ms | -1.6% | [-2.5%, -0.6%] | - | ~ | tape/fused 2.03x |
| `Fused/chain16/signedDistance/float/host` | 959 µs | 978 µs | +1.9% | [+0.1%, +3.7%] | - | ~ | tape/fused 16.68x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.0 ms | 45.9 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.1 ms | 46.4 ms | +0.6% | [+0.5%, +0.6%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.63 ms | 4.59 ms | -0.9% | [-1.4%, -0.5%] | - | ~ | tape/fused 2.83x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | tape/fused 2.05x |
| `Fused/chain16/signedDistance/double/host` | 3.87 ms | 3.87 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | tape/fused 4.24x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.2 ms | 52.1 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.3 ms | 52.6 ms | +0.6% | [-0.1%, +1.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 83.0 ms | 83.2 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 228 ms | -0.2% | [-0.5%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 115 ms | 114 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 261 ms | 259 ms | -0.8% | [-1.6%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 71.2 ms | 71.0 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 214 ms | 213 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 98.0 ms | 98.2 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 28.4 ms | 28.5 ms | +0.4% | [-1.7%, +2.5%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 759 ms | 757 ms | -0.3% | [-1.0%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 117 ms | -0.3% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 41.8 ms | 42.0 ms | +0.5% | [-1.1%, +2.0%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 846 ms | 844 ms | -0.2% | [-1.1%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.2 ms | 26.2 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.6 ms | 15.4 ms | -1.2% | [-5.5%, +3.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 319 ms | +0.4% | [+0.4%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 232 ms | 230 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.1 ms | 90.6 ms | +0.5% | [-0.1%, +1.2%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 81.5 ms | 81.3 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | tape/fused 2.83x |
| `MeshCSG/union-tape/firstHit/double/host` | 28.7 ms | 28.7 ms | +0.1% | [-1.1%, +1.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 16.9 ms | 17.3 ms | +2.3% | [+1.1%, +3.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 355 ms | 357 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 245 ms | 245 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 98.1 ms | 98.9 ms | +0.8% | [-0.4%, +2.0%] | +0.0% | ~ | tape/fused 3.61x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.0 ms | 86.9 ms | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ | tape/fused 2.83x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 229 ms | 228 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 231 ms | 232 ms | +0.2% | [-0.4%, +0.7%] | +0.0% | ~ | signedDistance/closestPoint 0.98x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 305 ms | 304 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 232 ms | 232 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | closestPoint/gradient 1.00x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 258 ms | 259 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 263 ms | 262 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ | signedDistance/closestPoint 0.99x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 337 ms | 339 ms | +0.8% | [-0.7%, +2.4%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 264 ms | 264 ms | -0.1% | [-2.0%, +1.8%] | +0.0% | ~ | closestPoint/gradient 1.00x |
| `MeshReconcile/armadillo/reconcile/float/host` | 72.7 ms | 72.8 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.1 ms | 41.1 ms | -0.0% | [-0.8%, +0.7%] | +0.0% | ~ | reconcile/scheduled 1.77x |
| `MeshReconcile/armadillo/reconcile/double/host` | 93.4 ms | 93.7 ms | +0.3% | [-1.6%, +2.2%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 55.8 ms | 56.5 ms | +1.2% | [-0.0%, +2.4%] | +0.0% | ~ | reconcile/scheduled 1.66x |
| `MeshTape/mesh/firstHit/float/host` | 48.1 ms | 48.2 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 48.3 ms | 48.1 ms | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.7 ms | 19.6 ms | -0.7% | [-1.5%, +0.2%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 80.3 ms | 80.0 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ | mesh/tape 0.60x |
| `MeshTape/tape/firstCrossing/float/host` | 49.9 ms | 49.7 ms | -0.3% | [-1.0%, +0.5%] | +0.0% | ~ | mesh/tape 0.97x |
| `MeshTape/tape/frame/float/host` | 24.8 ms | 24.0 ms | -3.3% | [-4.1%, -2.5%] | +0.0% | faster | mesh/tape 0.81x |
| `MeshTape/view/firstHit/float/host` | 83.2 ms | 82.9 ms | -0.3% | [-0.9%, +0.2%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/float/host` | 25.8 ms | 26.5 ms | +2.6% | [+1.4%, +3.8%] | +0.0% | ~ | mesh/view 0.74x (was 0.76x) |
| `MeshTape/mesh/firstHit/double/host` | 52.5 ms | 53.1 ms | +1.2% | [+0.2%, +2.1%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 52.7 ms | 52.7 ms | -0.1% | [-1.2%, +1.1%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.3 ms | 20.3 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 86.3 ms | 86.6 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | mesh/tape 0.61x |
| `MeshTape/tape/firstCrossing/double/host` | 54.5 ms | 54.3 ms | -0.3% | [-0.9%, +0.4%] | +0.0% | ~ | mesh/tape 0.97x |
| `MeshTape/tape/frame/double/host` | 26.5 ms | 26.7 ms | +0.9% | [-0.8%, +2.6%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 89.6 ms | 89.7 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | mesh/view 0.59x |
| `MeshTape/view/frame/double/host` | 28.7 ms | 29.0 ms | +0.9% | [-0.4%, +2.3%] | +0.0% | ~ | mesh/view 0.70x |
| `PointCloudBVH/uniform/build/float/host` | 53.1 ms | 53.3 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 143 ms | +0.2% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 359 ms | 360 ms | +0.2% | [-0.3%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 80.1 ms | 80.8 ms | +0.9% | [+0.3%, +1.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.82 ms | 6.72 ms | -1.4% | [-2.5%, -0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.9 ms | 55.6 ms | -0.5% | [-1.7%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 155 ms | 156 ms | +0.7% | [+0.1%, +1.4%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 395 ms | 391 ms | -0.9% | [-1.4%, -0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.2 ms | 87.3 ms | +1.3% | [+0.3%, +2.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.86 ms | 7.00 ms | +2.1% | [+1.5%, +2.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.9 ms | 14.5 ms | +3.7% | [-1.1%, +8.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 117 ms | 117 ms | -0.2% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 473 ms | 472 ms | -0.0% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 141 ms | 141 ms | +0.4% | [-0.2%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 801 ms | 801 ms | +0.0% | [-0.4%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 15.0 ms | +0.8% | [-1.4%, +3.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | -0.5% | [-0.9%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 491 ms | 491 ms | +0.1% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 145 ms | +0.4% | [-0.5%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 800 ms | 797 ms | -0.3% | [-0.5%, -0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.5 ms | 54.3 ms | -0.4% | [-0.8%, -0.0%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 129 ms | 129 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 268 ms | 268 ms | +0.3% | [-0.5%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 149 ms | 150 ms | +0.3% | [-0.7%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 136 ms | 134 ms | -1.4% | [-1.5%, -1.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.0 ms | 57.0 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 138 ms | 139 ms | +0.9% | [+0.4%, +1.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 299 ms | 297 ms | -0.6% | [-0.7%, -0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 162 ms | 162 ms | -0.5% | [-0.6%, -0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 150 ms | 149 ms | -0.8% | [-1.9%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 13.4 ms | 11.9 ms | -11.3% | [-22.7%, +3.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 114 ms | 114 ms | +0.2% | [-0.1%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 314 ms | 315 ms | +0.2% | [-0.2%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 155 ms | 155 ms | -0.1% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 320 ms | 320 ms | -0.0% | [-0.1%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.4 ms | 13.4 ms | +0.3% | [-1.0%, +1.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 115 ms | 116 ms | +0.2% | [-0.0%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 346 ms | 346 ms | +0.2% | [-0.4%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 164 ms | 164 ms | +0.3% | [+0.3%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 309 ms | -0.1% | [-0.8%, +0.7%] | - | ~ |  |
| `QueryOrder/random/mortonOrder/float/host` | 60.9 ms | 60.8 ms | -0.0% | [-0.1%, +0.0%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | 223 ms | 223 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | 160 ms | 161 ms | +0.8% | [+0.2%, +1.3%] | +0.0% | ~ | arrived/ordered 1.39x |
| `QueryOrder/random/mortonOrder/double/host` | 62.6 ms | 62.1 ms | -0.7% | [-1.6%, +0.2%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | 263 ms | 261 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | 171 ms | 174 ms | +1.9% | [-0.7%, +4.4%] | +0.0% | ~ | arrived/ordered 1.50x |
| `Ray/mesh/firstHit/float/host` | 48.6 ms | 48.6 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | hand/api 0.93x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.5 ms | 45.4 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.9 ms | 40.7 ms | -0.6% | [-1.3%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 77.9 ms | 79.3 ms | +1.8% | [+0.3%, +3.4%] | +0.0% | ~ | triangles/polygons 0.61x |
| `Ray/mesh-polygons/intersects/float/host` | 66.0 ms | 66.1 ms | +0.1% | [-2.4%, +2.7%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.23 ms | 5.17 ms | -1.1% | [-2.6%, +0.4%] | - | ~ | tape/fused 1.48x |
| `Ray/csg-tape/firstHit/float/host` | 7.58 ms | 7.66 ms | +1.0% | [-0.1%, +2.1%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 53.2 ms | 52.8 ms | -0.9% | [-2.5%, +0.8%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 50.4 ms | 51.3 ms | +1.7% | [+1.1%, +2.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.7 ms | 44.5 ms | -0.5% | [-1.3%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 89.0 ms | 88.3 ms | -0.8% | [-1.3%, -0.3%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 72.9 ms | 73.0 ms | +0.2% | [-0.4%, +2.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.69 ms | 5.47 ms | -3.9% | [-7.5%, -0.1%] | - | faster | tape/fused 1.47x (was 1.41x) |
| `Ray/csg-tape/firstHit/double/host` | 8.03 ms | 8.04 ms | +0.1% | [-3.8%, +4.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.7 ms | 18.1 ms | +1.8% | [+0.1%, +3.6%] | - | ~ | marched/traced 6.63x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 120 ms | -0.0% | [-0.3%, +0.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.8 ms | 22.2 ms | +2.0% | [+1.4%, +2.6%] | +0.0% | ~ | marched/traced 6.79x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 149 ms | 151 ms | +1.0% | [-0.2%, +2.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.6 ms | 35.5 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 18.8 ms | 19.1 ms | +1.4% | [-0.9%, +3.7%] | - | ~ | marched/traced 6.67x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 127 ms | 127 ms | -0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.9 ms | 23.6 ms | -1.1% | [-1.8%, -0.3%] | +0.0% | ~ | marched/traced 6.61x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 155 ms | 156 ms | +0.9% | [+0.4%, +1.4%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.9 ms | 37.7 ms | -0.5% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 48.1 ms | 48.3 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 48.5 ms | 48.4 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.52 ms | 7.57 ms | +0.8% | [+0.2%, +1.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.07 ms | 6.10 ms | +0.5% | [-0.0%, +1.0%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 177 ms | 177 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 157 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.6 ms | 68.7 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 57.1 ms | 56.3 ms | -1.3% | [-1.5%, -1.2%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.7 ms | 53.2 ms | +0.8% | [+0.7%, +1.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 53.0 ms | 52.6 ms | -0.9% | [-1.7%, +0.0%] | +0.0% | ~ | hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.87 ms | 7.89 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.28 ms | 6.28 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 198 ms | 198 ms | -0.0% | [-0.9%, +0.9%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 175 ms | 175 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.4 ms | 77.1 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 63.6 ms | 63.9 ms | +0.5% | [-0.2%, +1.2%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | marched/traced 4.65x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.4 ms | 51.6 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 7.99 ms | 8.00 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.99 ms | 1.99 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.98 ms | 6.91 ms | -1.1% | [-2.3%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.8 ms | 11.9 ms | +0.9% | [-0.5%, +2.3%] | +0.0% | ~ | marched/traced 5.12x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 62.3 ms | 61.0 ms | -2.1% | [-5.5%, +1.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.67 ms | 8.58 ms | -1.0% | [-1.7%, -0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.04 ms | 2.03 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.21 ms | 7.24 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 236 ms | 236 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | marched/traced 1.39x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 328 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 235 ms | 235 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 252 ms | 253 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ | marched/traced 1.45x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 368 ms | 367 ms | -0.3% | [-1.1%, +0.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 251 ms | 250 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.33 ms | 7.32 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.5 ms | -0.7% | [-1.6%, +0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 266 ms | 266 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.4 ms | -1.0% | [-1.8%, -0.1%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.4 ms | 30.3 ms | -0.3% | [-1.8%, +1.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.0 ms | 21.9 ms | -0.4% | [-2.1%, +1.3%] | +0.0% | ~ | regions/hit 1.38x |
| `Regions/particles/region/double/host` | 7.64 ms | 7.64 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.9 ms | 12.9 ms | -0.5% | [-0.6%, -0.4%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 283 ms | 282 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.0 ms | 13.0 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 31.9 ms | 31.9 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.8 ms | 23.8 ms | +0.2% | [-0.9%, +1.2%] | +0.0% | ~ | regions/hit 1.34x |
| `Render/csg/frame/float/host` | 12.2 ms | 12.6 ms | +3.0% | [-7.3%, +8.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 12.1 ms | 12.2 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.8 ms | 19.6 ms | -1.3% | [-3.0%, +0.5%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.0 ms | 14.0 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 13.7 ms | 13.8 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.2 ms | 20.3 ms | +0.3% | [-1.0%, +1.6%] | +0.0% | ~ |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 32.9 ms | 32.9 ms | +0.0% | [-0.0%, +0.1%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 4.98 ms | 4.96 ms | -0.4% | [-1.1%, +0.3%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.20 ms | 4.16 ms | -0.8% | [-1.5%, -0.1%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.2 ms | 17.4 ms | +1.0% | [-0.1%, +2.2%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.32 ms | 4.37 ms | +1.1% | [+0.2%, +2.1%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.11 ms | 4.11 ms | -0.1% | [-1.1%, +0.8%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.24 ms | 2.32 ms | +3.3% | [-0.2%, +6.9%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.44 ms | 1.44 ms | +0.4% | [-0.4%, +1.3%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.16 ms | 2.05 ms | -4.9% | [-9.4%, +0.0%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.1 ms | 27.2 ms | +0.2% | [+0.1%, +0.3%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 46.1 ms | 46.3 ms | +0.3% | [+0.0%, +0.6%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.40 ms | 5.43 ms | +0.6% | [-0.5%, +1.7%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.46 ms | 4.45 ms | -0.2% | [-0.5%, +0.1%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 22.5 ms | 22.6 ms | +0.7% | [+0.1%, +1.4%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.54 ms | 4.54 ms | -0.0% | [-1.6%, +1.6%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.36 ms | 5.34 ms | -0.4% | [-2.0%, +1.3%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.50 ms | 2.48 ms | -0.8% | [-2.2%, +0.6%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.77 ms | 1.78 ms | +0.3% | [-0.0%, +0.6%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.08 ms | 2.08 ms | +0.4% | [-0.3%, +1.1%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 30.7 ms | 30.1 ms | -2.0% | [-4.3%, +0.4%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.4 ms | 56.8 ms | +0.7% | [-0.3%, +1.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.4 ms | 36.4 ms | +0.0% | [-0.9%, +0.9%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.2 ms | 92.3 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 63.9 ms | 64.0 ms | +0.1% | [-1.4%, +1.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 41.4 ms | 41.6 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 106 ms | 106 ms | -0.6% | [-1.7%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.8 ms | 74.4 ms | -0.5% | [-0.5%, -0.4%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 295 ms | 296 ms | +0.2% | [-0.5%, +1.0%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 295 ms | 295 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 86.3 ms | 85.6 ms | -0.8% | [-1.9%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 342 ms | 344 ms | +0.5% | [-0.3%, +1.3%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 344 ms | 346 ms | +0.7% | [-0.6%, +2.0%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 166 ms | 166 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 190 ms | 190 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 144 ms | 143 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 89.9 ms | 89.4 ms | -0.6% | [-0.9%, -0.2%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 130 ms | 130 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 72.4 ms | 72.4 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ | surface/leaf 1.80x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 61.2 ms | 61.0 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.1 ms | 34.0 ms | -0.5% | [-1.0%, +0.1%] | +0.0% | ~ | surface/hit 1.79x |
| `SurfaceParts/boxes/closestSurface/double/host` | 145 ms | 145 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 91.2 ms | 91.1 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ | surface/leaf 1.59x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 144 ms | 144 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 81.4 ms | 80.8 ms | -0.7% | [-1.1%, -0.4%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 69.6 ms | 68.9 ms | -1.0% | [-1.5%, -0.4%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 38.4 ms | 38.6 ms | +0.6% | [-0.0%, +1.2%] | +0.0% | ~ | surface/hit 1.78x |
| `Tape/csg/signedDistance/float/host` | 13.0 ms | 13.0 ms | -0.1% | [-1.4%, +1.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.55 ms | 4.57 ms | +0.3% | [-0.1%, +0.8%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.0 ms | 24.9 ms | -0.5% | [-1.6%, +0.7%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.0 ms | 39.9 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.73 ms | 1.74 ms | +0.6% | [-0.9%, +2.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 135 µs | 129 µs | -4.4% | [-8.5%, -0.1%] | - | faster |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.1 ms | 30.2 ms | +0.2% | [-1.0%, +1.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.71 ms | 5.68 ms | -0.6% | [-1.4%, +0.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.3 ms | +0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.67 ms | 4.68 ms | +0.1% | [-1.1%, +1.4%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.0 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.60 ms | 4.64 ms | +0.9% | [-0.6%, +2.4%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.2 ms | 26.3 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.1 ms | 41.8 ms | -0.8% | [-0.9%, -0.7%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.76 ms | 1.78 ms | +1.4% | [+0.3%, +2.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 291 µs | 274 µs | -5.9% | [-17.3%, +7.6%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.7 ms | 29.7 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.80 ms | 7.88 ms | +1.0% | [-0.0%, +2.0%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.3 ms | 16.4 ms | +0.6% | [-0.3%, +1.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.75 ms | 7.73 ms | -0.3% | [-0.7%, +0.2%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.9 ms | 42.6 ms | +1.5% | [-0.3%, +3.3%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 201 ms | 201 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 42.6 ms | 42.4 ms | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 218 ms | 219 ms | +0.4% | [+0.0%, +0.8%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 748.6 s → 710.4 s (-5.1%), the largest peak 1418 MB → 1420 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.8 s | 9.3 s | -4.6% | 558 MB | 559 MB | +0.0% | ~ |
| `BVHRefit` | 4.3 s | 4.2 s | -1.8% | 497 MB | 497 MB | -0.1% | ~ |
| `BVHUnion` | 19.6 s | 18.6 s | -5.1% | 948 MB | 948 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.7 s | 7.5 s | -2.4% | 557 MB | 557 MB | -0.0% | ~ |
| `ClosestPrimitive` | 30.5 s | 29.2 s | -4.4% | 1297 MB | 1296 MB | -0.1% | ~ |
| `ClosestSurface` | 28.2 s | 27.0 s | -4.5% | 1299 MB | 1301 MB | +0.2% | ~ |
| `Domain` | 18.2 s | 17.5 s | -4.1% | 843 MB | 846 MB | +0.3% | ~ |
| `DomainObject` | 16.2 s | 15.5 s | -4.7% | 847 MB | 847 MB | +0.0% | ~ |
| `EBGrid` | 14.7 s | 14.0 s | -4.6% | 781 MB | 780 MB | -0.1% | ~ |
| `Fused` | 25.0 s | 23.7 s | -5.0% | 1210 MB | 1210 MB | +0.0% | ~ |
| `LaunchTuning` | 37.1 s | 35.1 s | -5.3% | 1279 MB | 1278 MB | -0.1% | ~ |
| `Mesh` | 16.2 s | 15.3 s | -6.0% | 786 MB | 786 MB | -0.0% | ~ |
| `MeshCSG` | 39.9 s | 37.4 s | -6.4% | 1259 MB | 1256 MB | -0.3% | ~ |
| `MeshClosestPoint` | 14.6 s | 13.7 s | -6.1% | 782 MB | 782 MB | -0.0% | ~ |
| `MeshIngest` | 7.2 s | 6.8 s | -6.2% | 569 MB | 569 MB | -0.0% | ~ |
| `MeshReconcile` | 7.0 s | 6.5 s | -6.9% | 569 MB | 568 MB | -0.1% | ~ |
| `MeshTape` | 41.6 s | 39.4 s | -5.2% | 1279 MB | 1280 MB | +0.1% | ~ |
| `PointCloud` | 7.4 s | 7.0 s | -5.8% | 561 MB | 561 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.9 s | 3.6 s | -7.5% | 472 MB | 472 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.1 s | -8.5% | 453 MB | 452 MB | -0.1% | ~ |
| `QueryOrder` | 11.4 s | 10.7 s | -6.7% | 692 MB | 692 MB | -0.1% | ~ |
| `Ray` | 36.5 s | 34.2 s | -6.2% | 1292 MB | 1291 MB | -0.1% | ~ |
| `RayCSG` | 32.4 s | 30.1 s | -6.9% | 1195 MB | 1197 MB | +0.2% | ~ |
| `RayCrossing` | 46.1 s | 42.8 s | -7.3% | 1344 MB | 1346 MB | +0.2% | ~ |
| `RayParticles` | 48.6 s | 46.3 s | -4.7% | 1332 MB | 1334 MB | +0.2% | ~ |
| `RayShell` | 37.7 s | 35.4 s | -6.2% | 1267 MB | 1265 MB | -0.2% | ~ |
| `Regions` | 29.3 s | 27.7 s | -5.7% | 1074 MB | 1077 MB | +0.3% | ~ |
| `Render` | 35.9 s | 34.5 s | -3.8% | 1273 MB | 1271 MB | -0.2% | ~ |
| `Shapes` | 8.8 s | 8.3 s | -6.3% | 555 MB | 555 MB | -0.0% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.5 s | -5.5% | 512 MB | 512 MB | +0.1% | ~ |
| `SphereCloudRays` | 27.9 s | 27.1 s | -2.8% | 1066 MB | 1068 MB | +0.2% | ~ |
| `SphereContacts` | 4.2 s | 4.1 s | -4.5% | 486 MB | 486 MB | +0.0% | ~ |
| `SurfaceParts` | 40.7 s | 39.9 s | -1.8% | 1418 MB | 1420 MB | +0.1% | ~ |
| `Tape` | 16.9 s | 16.4 s | -3.4% | 831 MB | 832 MB | +0.1% | ~ |
| `TapeBVHUnion` | 14.8 s | 14.2 s | -3.9% | 826 MB | 826 MB | +0.0% | ~ |
| **all, in both** | 748.6 s | 710.4 s | -5.1% | 1418 MB | 1420 MB | +0.1% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): Ray/mesh-polygons/intersects/double/host, Render/csg/frame/float/host

These probes compiled to the same code on both sides and were not run (12 cases): MeshIngest (8), PointCloudBVHBuild (2), PointCloudHashGridBuild (2)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: b4fb4c6
base-fingerprint: 6c3c437020716e933adeba2f122c30695cb6e811
head: 9cab426
head-fingerprint: 570f0a4e56d677aab74e9c092b0d5dfeac3ddaf9
cases: 406
rounds: 2
reps: 3
date: 2026-11-21
-->

## Device comparison (CUDA): green

**Accepted:** -

`9cab426` against `b4fb4c6` (origin/main), 2026-11-21.

**VERDICT: no regression (0 major, 0 minor across 406 cases)**

Performance comparison: 0 major, 0 minor, 3 faster, 0 differ, 0 lost, 406 cases

- base: b4fb4c6 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 9cab426 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.61 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | device 22.6x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.42 ms | 3.40 ms | -0.7% | [-1.1%, -0.3%] | - | ~ | device 18.7x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.03 ms | 1.06 ms | +2.6% | [-0.9%, +6.1%] | - | ~ | device 19.3x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.32 ms | +0.6% | [+0.2%, +1.1%] | - | ~ | device 16.5x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.32 ms | 6.27 ms | -0.8% | [-3.2%, +1.7%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 12.1 ms | 12.1 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 5.2x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.3 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 56.9 ms | 57.2 ms | +0.5% | [+0.4%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.8 ms | 21.1 ms | +1.3% | [-1.3%, +4.0%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.7 ms | 48.0 ms | +0.6% | [+0.3%, +0.9%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.7 ms | 61.0 ms | -1.3% | [-2.4%, -0.1%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 110 ms | +1.3% | [-1.5%, +4.3%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.29 ms | 8.15 ms | -1.8% | [-4.5%, +1.1%] | +0.0% | ~ | device 5.0x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.91 ms | 9.78 ms | -1.4% | [-3.4%, +0.7%] | +0.0% | ~ | device 8.2x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 389 µs | 388 µs | -0.4% | [-0.7%, -0.1%] | - | ~ | device 15.1x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 848 µs | 849 µs | +0.1% | [-0.2%, +0.5%] | - | ~ | device 9.9x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.52 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.68 ms | 4.66 ms | -0.4% | [-1.0%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 589 µs | 591 µs | +0.4% | [-0.3%, +1.1%] | - | ~ | device 12.8x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.41 ms | 1.41 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.78 ms | 6.77 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.00 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 914 µs | 917 µs | +0.4% | [+0.1%, +0.7%] | - | ~ | device 14.1x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.29 ms | 2.29 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 7.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.49 ms | 9.50 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 858 µs | 859 µs | +0.1% | [-0.3%, +0.5%] | - | ~ | device 13.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.10 ms | 2.09 ms | -0.3% | [-0.4%, -0.3%] | - | ~ | device 7.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.14 ms | 9.10 ms | -0.5% | [-1.1%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.80 ms | 9.81 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 314 µs | 315 µs | +0.4% | [+0.1%, +0.7%] | - | ~ | device 11.8x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 6.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.0 ms | 58.9 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 3.9x faster than the host (was 3.3x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.39 ms | 5.39 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 520 ms | 518 ms | -0.4% | [-2.5%, +1.8%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.1 ms | 20.1 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 4.1x faster than the host |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.33 ms | 8.32 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 4.7x faster than the host; surface/leaf 2.42x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.6 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 8.1x faster than the host (was 10.3x) |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.18 ms | 6.18 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 10.0x faster than the host; surface/leaf 2.52x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 147 ms | 146 ms | -1.2% | [-2.8%, +0.5%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.1 ms | 60.0 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.43x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 71.9 ms | 71.8 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.3x faster than the host; surface/leaf 2.56x |
| `Domain/particles/labels/float/hostdevice` | 4.01 ms | 4.02 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 3.5x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.72 ms | 2.72 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 2.7x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 121 µs | 121 µs | +0.2% | [-1.0%, +1.5%] | - | ~ | device 12.4x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.07 ms | 7.07 ms | -0.0% | [-2.4%, +2.4%] | - | ~ | device 2.0x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.61 ms | 3.61 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Domain/particles/surfaceGroups/double/hostdevice` | 129 µs | 128 µs | -0.6% | [-3.0%, +1.9%] | - | ~ | device 11.0x faster than the host (was 10.5x) |
| `DomainObject/partsWhole/labels/float/hostdevice` | 12.4 ms | 12.4 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 3.6x faster than the host |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.75 ms | 8.72 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 2.7x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 270 µs | 270 µs | +0.3% | [-0.1%, +0.7%] | - | ~ | device 2.5x faster than the host; whole/own 45.81x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 152 µs | 151 µs | -0.7% | [-2.0%, +0.7%] | - | ~ | device 2.4x faster than the host (was 2.2x); whole/own 57.81x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 20.0 ms | 20.1 ms | +0.7% | [+0.4%, +0.9%] | - | ~ | device 2.3x faster than the host |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 11.7 ms | 11.7 ms | -0.4% | [-0.9%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `DomainObject/partsOwn/labels/double/hostdevice` | 391 µs | 395 µs | +0.9% | [-0.3%, +2.2%] | - | ~ | device 1.7x faster than the host (was 1.8x); whole/own 50.94x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 233 µs | 234 µs | +0.5% | [+0.2%, +0.9%] | - | ~ | device 1.6x faster than the host; whole/own 49.94x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/hostdevice` | 10.4 ms | 10.4 ms | -0.2% | [-1.1%, +0.8%] | +0.0% | ~ | device 10.0x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/hostdevice` | 10.3 ms | 10.3 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ | device 9.3x faster than the host; grid/morton 1.01x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/hostdevice` | 17.9 ms | 17.7 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ | device 6.9x faster than the host; grid/shuffled 0.59x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/hostdevice` | 225 ms | 224 ms | -0.0% | [-1.1%, +1.1%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/hostdevice` | 165 ms | 163 ms | -1.5% | [-2.8%, -0.3%] | +0.0% | ~ | device 0.6x faster than the host; grid/morton 1.38x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/hostdevice` | 293 ms | 283 ms | -3.5% | [-5.1%, -1.8%] | +0.0% | faster | device 0.4x faster than the host (was 0.4x); grid/shuffled 0.79x (was 0.77x) |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/hostdevice` | 27.3 ms | 27.2 ms | -0.2% | [-1.1%, +0.7%] | +0.0% | ~ | device 3.9x faster than the host |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/hostdevice` | 29.4 ms | 29.1 ms | -0.9% | [-1.7%, -0.1%] | +0.0% | ~ | device 3.4x faster than the host; grid/morton 0.94x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/hostdevice` | 64.1 ms | 64.0 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 2.5x faster than the host; grid/shuffled 0.42x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/hostdevice` | 202 ms | 206 ms | +1.8% | [+1.1%, +2.5%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/hostdevice` | 158 ms | 157 ms | -1.1% | [-3.7%, +1.7%] | +0.0% | ~ | device 0.6x faster than the host; grid/morton 1.31x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/hostdevice` | 249 ms | 249 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 0.6x faster than the host; grid/shuffled 0.83x |
| `Fused/csg/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.4% | [-0.7%, +0.0%] | - | ~ | device 24.5x faster than the host (was 23.6x); tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.7 µs | 49.8 µs | +0.2% | [-0.5%, +0.9%] | - | ~ | device 44.6x faster than the host; tape/fused 7.21x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.6 µs | 47.8 µs | -1.6% | [-3.2%, -0.1%] | - | ~ | device 9.1x faster than the host; tape/fused 9.96x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.8 ms | 17.7 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 6.6x faster than the host; tape/fused 1.76x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.2 ms | 31.2 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.5x faster than the host (was 1.5x); tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.56 ms | 1.55 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 285 ms | 287 ms | +0.7% | [-0.6%, +2.0%] | - | ~ | device 0.4x faster than the host; tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 287 ms | 286 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.24 ms | 8.25 ms | +0.2% | [-0.9%, +1.3%] | +0.0% | ~ | device 10.7x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.37 ms | 8.27 ms | -1.2% | [-1.5%, -0.9%] | +0.0% | ~ | device 11.4x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.18 ms | 8.25 ms | +0.8% | [+0.0%, +1.6%] | +0.0% | ~ | device 10.8x faster than the host (was 12.0x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.26 ms | 8.22 ms | -0.4% | [-3.0%, +2.2%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 703 µs | 704 µs | +0.1% | [-0.6%, +0.9%] | +0.0% | ~ | device 3.6x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.81 ms | 1.81 ms | -0.1% | [-0.1%, -0.1%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.03 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.68x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 782 µs | 783 µs | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ | device 3.3x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 732 µs | 734 µs | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ | device 12.1x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 815 µs | 809 µs | -0.7% | [-1.4%, -0.1%] | +0.0% | ~ | device 11.0x faster than the host; default/this 0.91x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 732 µs | 736 µs | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 727 µs | 727 µs | -0.1% | [-0.8%, +0.7%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 156 ms | -2.1% | [-5.8%, +1.6%] | +0.0% | ~ | device 0.8x faster than the host (was 0.6x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 162 ms | 159 ms | -2.4% | [-6.1%, +1.4%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 162 ms | 160 ms | -1.0% | [-5.1%, +3.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 159 ms | 161 ms | +1.4% | [-0.6%, +3.4%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.97x (was 1.00x) |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | -0.1% | [-0.7%, +0.4%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.81 ms | 2.81 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.49x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.92 ms | 1.92 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.72x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.55 ms | +0.5% | [+0.2%, +0.7%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.25 ms | 7.23 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.31 ms | 7.28 ms | -0.5% | [-1.1%, +0.2%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.27 ms | 7.25 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.28 ms | 7.31 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.46 ms | 8.50 ms | +0.4% | [-0.3%, +1.2%] | - | ~ | device 11.0x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.8 ms | 16.9 ms | +0.8% | [-0.5%, +2.1%] | - | ~ | device 7.3x faster than the host (was 7.8x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 168 ms | 172 ms | +2.7% | [+1.1%, +4.3%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 288 ms | 286 ms | -0.7% | [-2.3%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.8 ms | 24.7 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 60.5 ms | 60.5 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 163 ms | 155 ms | -5.0% | [-5.5%, -4.5%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `MeshSDF/query-random/double/hostdevice` | 244 ms | 241 ms | -1.3% | [-4.6%, +2.2%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.01 ms | 2.01 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 5.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.36 ms | 3.37 ms | +0.4% | [-0.6%, +1.5%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.15 ms | 6.14 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 4.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 62.7 ms | 62.7 ms | -0.0% | [-0.9%, +0.9%] | - | ~ | device 3.4x faster than the host (was 3.2x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 92.5 ms | 93.1 ms | +0.7% | [-1.1%, +2.6%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 238 ms | 244 ms | +2.2% | [+0.2%, +4.1%] | - | ~ | device 3.5x faster than the host (was 3.6x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.7 ms | 24.9 ms | +0.7% | [+0.2%, +1.1%] | - | ~ | device 2.0x faster than the host; tape/fused 2.52x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.7 ms | 26.9 ms | +0.8% | [-0.1%, +1.6%] | - | ~ | device 2.2x faster than the host; tape/fused 3.46x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.2 ms | 65.5 ms | +0.4% | [-0.8%, +1.7%] | - | ~ | device 4.3x faster than the host (was 3.9x); tape/fused 3.72x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.1 ms | 20.4 ms | -3.3% | [-7.4%, +1.0%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 38.2 ms | 38.0 ms | -0.6% | [-2.2%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 40.5 ms | 40.8 ms | +0.6% | [-2.1%, +3.3%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 531 ms | 513 ms | -3.4% | [-6.1%, -0.6%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 851 ms | 850 ms | -0.1% | [-1.4%, +1.2%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.98 s | 1.98 s | +0.4% | [-1.6%, +2.4%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 190 ms | 194 ms | +2.1% | [-0.8%, +4.2%] | - | ~ | device 0.3x faster than the host; tape/fused 2.64x (was 2.79x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 306 ms | 313 ms | +2.2% | [-0.3%, +4.8%] | - | ~ | device 0.2x faster than the host; tape/fused 2.72x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 548 ms | 545 ms | -0.5% | [-2.1%, +1.0%] | - | ~ | device 0.5x faster than the host (was 0.6x); tape/fused 3.64x |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 12.1 ms | 12.0 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 10.3x faster than the host (was 11.3x) |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 61.2 ms | 61.0 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 261 ms | 260 ms | -0.4% | [-2.6%, +1.9%] | - | ~ | device 0.5x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 246 ms | 247 ms | +0.1% | [-0.7%, +0.9%] | - | ~ | device 0.6x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 15.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.32 ms | 5.33 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 10.2x faster than the host |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.29 ms | 1.30 ms | +0.7% | [+0.3%, +1.2%] | - | ~ | device 15.8x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.29 ms | 1.30 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 15.8x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.15 ms | 2.14 ms | -0.3% | [-1.2%, +0.7%] | - | ~ | device 7.9x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.50 ms | 3.50 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 10.2x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.74 ms | 2.75 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 7.8x faster than the host; mesh/tape 0.47x |
| `MeshTape/tape/frame/float/hostdevice` | 4.40 ms | 4.40 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 4.9x faster than the host; mesh/tape 0.49x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.73 ms | 3.73 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 9.7x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.58 ms | 5.57 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 4.2x faster than the host; mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 28.5 ms | 28.6 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 28.4 ms | 28.5 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 0.8x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 24.7 ms | 24.4 ms | -1.1% | [-2.1%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 40.8 ms | 41.0 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 0.9x faster than the host; mesh/tape 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 28.8 ms | 28.7 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 31.6 ms | 31.5 ms | -0.3% | [-1.2%, +0.6%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.78x |
| `MeshTape/view/firstHit/double/hostdevice` | 41.0 ms | 41.2 ms | +0.5% | [-0.3%, +1.3%] | - | ~ | device 0.9x faster than the host; mesh/view 0.69x |
| `MeshTape/view/frame/double/hostdevice` | 34.4 ms | 34.4 ms | -0.1% | [-1.0%, +0.9%] | - | ~ | device 0.7x faster than the host; mesh/view 0.71x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 683 µs | 679 µs | -0.6% | [-1.2%, +0.0%] | - | ~ | device 11.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.85 ms | 1.85 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 5.6x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 999 µs | 995 µs | -0.4% | [-0.6%, -0.1%] | - | ~ | device 9.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.74 ms | 4.75 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 3.08 ms | 3.07 ms | -0.5% | [-0.7%, -0.2%] | - | ~ | device 5.0x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.05 ms | 7.10 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.81 ms | 7.77 ms | -0.4% | [-0.9%, -0.0%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.4 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 21.0 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.48 ms | 1.45 ms | -2.1% | [-5.1%, +1.0%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.88 ms | 3.89 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 524 µs | 521 µs | -0.6% | [-0.8%, -0.4%] | - | ~ | device 15.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.58 ms | 4.54 ms | -1.0% | [-1.2%, -0.8%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.41 ms | 4.41 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 5.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.12 ms | 8.13 ms | +0.1% | [-0.7%, +1.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.9 ms | 15.8 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.00 ms | 4.01 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.8 ms | 17.9 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.0 ms | 15.1 ms | +1.0% | [-0.5%, +2.4%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.58 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.66 ms | 4.65 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 836 µs | 831 µs | -0.6% | [-0.9%, -0.3%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.94 ms | 2.94 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 6.8x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.29 ms | 4.31 ms | +0.6% | [-0.1%, +1.4%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.5 ms | 16.5 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 20.9 ms | -0.5% | [-0.7%, -0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.96 ms | 8.91 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.5 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.1 ms | 20.3 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.74 ms | 2.73 ms | -0.2% | [-0.8%, +0.5%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.72 ms | 6.72 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.74 ms | 2.75 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 7.8x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.57 ms | 6.52 ms | -0.7% | [-1.5%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 34.0 ms | 33.7 ms | -0.9% | [-1.6%, -0.2%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.4 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.4 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.8 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.0 ms | 38.2 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 0.5x faster than the host |
| `QueryOrder/random/mortonOrder/float/hostdevice` | 16.9 ms | 16.8 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 3.5x faster than the host |
| `QueryOrder/armadillo/distance-arrived/float/hostdevice` | 9.34 ms | 9.39 ms | +0.6% | [-0.2%, +1.4%] | +0.0% | ~ | device 4.8x faster than the host |
| `QueryOrder/armadillo/distance-ordered/float/hostdevice` | 7.26 ms | 7.30 ms | +0.5% | [-1.2%, +2.2%] | +0.0% | ~ | device 5.4x faster than the host; arrived/ordered 1.29x |
| `QueryOrder/random/mortonOrder/double/hostdevice` | 17.6 ms | 17.5 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 3.4x faster than the host |
| `QueryOrder/armadillo/distance-arrived/double/hostdevice` | 111 ms | 113 ms | +1.8% | [+0.4%, +3.3%] | +0.0% | ~ | device 0.4x faster than the host |
| `QueryOrder/armadillo/distance-ordered/double/hostdevice` | 79.1 ms | 78.2 ms | -1.2% | [-2.2%, -0.1%] | +0.0% | ~ | device 0.5x faster than the host; arrived/ordered 1.44x (was 1.40x) |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 688 µs | 685 µs | -0.4% | [-1.4%, +0.6%] | - | ~ | device 17.6x faster than the host; hand/api 0.95x (was 0.92x) |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.30 ms | 1.30 ms | -0.5% | [-1.1%, +0.1%] | - | ~ | device 15.3x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 636 µs | 652 µs | +2.6% | [+2.1%, +3.1%] | - | ~ | device 17.3x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | -0.3% | [-0.4%, -0.3%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 608 µs | 605 µs | -0.5% | [-1.1%, +0.1%] | - | ~ | device 16.1x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | -0.6% | [-1.0%, -0.3%] | - | ~ | device 14.8x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.19 ms | +0.4% | [-0.3%, +1.0%] | - | ~ | device 11.8x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.50 ms | 2.49 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 9.6x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.08 ms | 1.08 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.17 ms | -0.6% | [-0.7%, -0.5%] | - | ~ | device 9.9x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 608 µs | 611 µs | +0.4% | [-4.9%, +6.0%] | - | ~ | device 2.5x faster than the host; tape/fused 1.15x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 690 µs | 686 µs | -0.5% | [-9.4%, +9.4%] | - | ~ | device 2.6x faster than the host; tape/fused 1.23x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 704 µs | 705 µs | +0.1% | [-0.3%, +0.6%] | - | ~ | device 3.6x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 840 µs | 846 µs | +0.7% | [-0.5%, +1.9%] | - | ~ | device 3.5x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.6 ms | 17.6 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 0.7x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 28.4 ms | 28.6 ms | +0.8% | [+0.7%, +0.8%] | - | ~ | device 0.7x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.2% | [-0.9%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.6 ms | 27.4 ms | -0.5% | [-0.9%, -0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | -0.1% | [-1.0%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 17.2 ms | 17.1 ms | -0.7% | [-1.6%, +0.3%] | - | ~ | device 0.8x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.5 ms | 25.4 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.4% | [+0.0%, +0.8%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.24 ms | 1.25 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.38 ms | 2.37 ms | -0.5% | [-1.1%, +0.1%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.48 ms | -0.6% | [-0.7%, -0.5%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.52 ms | 2.53 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 3.0x faster than the host; marched/traced 3.56x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.98 ms | 2.98 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.7x faster than the host; marched/traced 4.65x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.00 ms | 8.99 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.16 ms | 2.16 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 3.9x faster than the host; marched/traced 13.53x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.42 ms | 5.43 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.8x faster than the host; marched/traced 10.25x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.1 ms | 29.2 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 2.7x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.7 ms | 55.7 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.0x faster than the host (was 2.1x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.23 ms | 5.22 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.80 ms | 8.79 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.15 ms | 6.15 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 1.3x faster than the host; marched/traced 25.92x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.5 ms | 10.6 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 0.8x faster than the host; marched/traced 24.07x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 161 ms | 159 ms | -1.0% | [-2.8%, +0.8%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 255 ms | 255 ms | -0.2% | [-3.2%, +3.0%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.88 ms | 4.89 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.8x faster than the host; marched/traced 22.02x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.2 ms | -0.5% | [-0.5%, -0.4%] | - | ~ | device 1.3x faster than the host; marched/traced 17.79x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 108 ms | 108 ms | -0.6% | [-1.8%, +0.6%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 217 ms | 218 ms | +0.0% | [-3.0%, +3.2%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.5 ms | 19.3 ms | -1.1% | [-1.5%, -0.7%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.8 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 683 µs | 687 µs | +0.6% | [+0.1%, +1.0%] | - | ~ | device 17.6x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.30 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 15.4x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 692 µs | 690 µs | -0.4% | [-0.8%, -0.1%] | - | ~ | device 17.6x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.29 ms | 1.30 ms | +0.5% | [+0.1%, +0.8%] | - | ~ | device 15.4x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 705 µs | 705 µs | +0.0% | [-0.2%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 846 µs | 844 µs | -0.3% | [-1.4%, +0.8%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 621 µs | 621 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.2x faster than the host; hit/crossing 1.13x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 641 µs | 641 µs | -0.0% | [-0.5%, +0.4%] | - | ~ | device 3.7x faster than the host; hit/crossing 1.32x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.75 ms | 4.75 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.3 ms | 10.3 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.17 ms | 4.17 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 10.2x faster than the host; hit/crossing 1.14x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.09 ms | 9.08 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 55.8 ms | 56.9 ms | +2.0% | [+0.6%, +3.4%] | - | ~ | device 3.1x faster than the host (was 3.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 82.1 ms | 80.9 ms | -1.4% | [-3.2%, +0.4%] | - | ~ | device 2.3x faster than the host (was 2.8x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.7 ms | 49.5 ms | +1.8% | [+0.7%, +2.8%] | - | ~ | device 2.9x faster than the host; hit/crossing 1.15x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 64.2 ms | 64.1 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 2.4x faster than the host (was 2.7x); hit/crossing 1.26x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.6 ms | 17.6 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 28.5 ms | 28.4 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.6 ms | 17.6 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 28.5 ms | 28.6 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.40 ms | +0.5% | [+0.4%, +0.7%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.11 ms | 1.10 ms | -0.7% | [-1.1%, -0.3%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.27x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.7 ms | 64.8 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 122 ms | 122 ms | -0.3% | [-1.3%, +0.7%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.7 ms | 58.8 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 112 ms | 110 ms | -1.8% | [-3.1%, -0.6%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 476 ms | 489 ms | +2.9% | [-2.1%, +8.0%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 705 ms | 710 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 383 ms | 365 ms | -4.7% | [-9.6%, +0.6%] | - | ~ | device 0.4x faster than the host (was 0.4x); hit/crossing 1.34x (was 1.24x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 535 ms | 545 ms | +1.8% | [-0.0%, +3.7%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.30x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.56 ms | 3.53 ms | -0.7% | [-2.3%, +0.8%] | - | ~ | device 1.2x faster than the host; marched/traced 3.41x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.80 ms | 4.79 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 0.9x faster than the host; marched/traced 5.54x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.5 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.26 ms | 1.24 ms | -1.7% | [-2.3%, -1.1%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.83 ms | -0.9% | [-1.5%, -0.2%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 97.8 µs | 97.4 µs | -0.4% | [-2.2%, +1.4%] | - | ~ | device 14.9x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 155 µs | 154 µs | -0.6% | [-1.0%, -0.1%] | - | ~ | device 10.9x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 797 µs | 799 µs | +0.3% | [+0.0%, +0.5%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 929 µs | 931 µs | +0.2% | [-0.0%, +0.5%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.3 ms | 35.7 ms | +1.2% | [-3.1%, +5.6%] | - | ~ | device 0.1x faster than the host; marched/traced 3.37x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 37.1 ms | 36.5 ms | -1.5% | [-2.6%, -0.3%] | - | ~ | device 0.1x faster than the host; marched/traced 6.38x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 122 ms | 120 ms | -0.9% | [-1.9%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 231 ms | 233 ms | +0.6% | [-0.4%, +1.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.4 ms | 16.2 ms | -1.4% | [-3.3%, +0.5%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 21.8 ms | -0.0% | [-0.9%, +0.9%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.08 ms | 1.08 ms | +0.7% | [-1.7%, +3.1%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.44 ms | 2.42 ms | -0.5% | [-1.3%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 6.02 ms | 5.95 ms | -1.1% | [-1.7%, -0.6%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.69 ms | 6.73 ms | +0.6% | [+0.5%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 22.9 ms | 23.2 ms | +1.1% | [+0.1%, +2.2%] | - | ~ | device 2.1x faster than the host; marched/traced 2.67x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.5 ms | 28.2 ms | -0.9% | [-2.2%, +0.3%] | - | ~ | device 1.9x faster than the host; marched/traced 3.75x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.1 ms | 61.9 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 104 ms | 106 ms | +1.6% | [+0.6%, +2.6%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.9 ms | 13.8 ms | -1.0% | [-1.7%, -0.3%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.5 ms | 17.6 ms | +0.3% | [-0.8%, +1.4%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 304 ms | 303 ms | -0.3% | [-6.8%, +6.9%] | - | ~ | device 0.2x faster than the host; marched/traced 1.87x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 351 ms | 344 ms | -2.0% | [-4.3%, +0.5%] | - | ~ | device 0.2x faster than the host; marched/traced 2.85x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 559 ms | 567 ms | +1.4% | [+0.7%, +2.0%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.00 s | 981 ms | -2.1% | [-4.4%, +0.2%] | - | ~ | device 0.2x faster than the host (was 0.4x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 188 ms | 192 ms | +2.1% | [-1.5%, +5.9%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 232 ms | 236 ms | +2.1% | [-0.9%, +5.2%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.76 ms | 2.76 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | device 2.7x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.77 ms | 3.77 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 3.4x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 90.0 ms | 90.4 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 3.2x faster than the host; one-pass/each 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.71 ms | 3.71 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 3.7x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.71 ms | 6.72 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.54 ms | 5.53 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 3.2x faster than the host; regions/hit 1.21x |
| `Regions/particles/region/double/hostdevice` | 3.35 ms | 3.35 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.90 ms | 5.91 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 135 ms | 136 ms | +0.7% | [-0.6%, +2.1%] | - | ~ | device 2.5x faster than the host (was 2.2x); one-pass/each 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.88 ms | 5.90 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 2.3x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.8 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 12.9 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.3x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.37 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.35 ms | 2.36 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.13 ms | 2.13 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 7.8x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.60 ms | 5.62 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.62 ms | 5.61 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 24.6 ms | 24.5 ms | -0.7% | [-1.0%, -0.3%] | - | ~ | device 0.7x faster than the host |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 155 µs | 155 µs | -0.1% | [-0.1%, +0.0%] | - | ~ | device 33.1x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.7 µs | 47.4 µs | -0.6% | [-0.6%, -0.5%] | - | ~ | device 18.7x faster than the host |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.3% | [+0.2%, +0.4%] | - | ~ | device 14.8x faster than the host |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 48.3 µs | 48.1 µs | -0.2% | [-0.6%, +0.1%] | - | ~ | device 64.9x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 48.0 µs | 48.2 µs | +0.5% | [-0.4%, +1.4%] | - | ~ | device 15.5x faster than the host |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 48.1 µs | 48.0 µs | -0.3% | [-1.0%, +0.5%] | - | ~ | device 17.5x faster than the host |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.3% | [+0.0%, +0.5%] | - | ~ | device 9.6x faster than the host (was 9.9x) |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.7 µs | 48.0 µs | +0.6% | [+0.1%, +1.2%] | - | ~ | device 7.5x faster than the host |
| `Shapes/wedge/signedDistance/float/hostdevice` | 47.6 µs | 47.8 µs | +0.5% | [+0.2%, +0.9%] | - | ~ | device 8.4x faster than the host (was 8.2x) |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 554 µs | 553 µs | -0.3% | [-0.4%, -0.2%] | - | ~ | device 8.2x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.23 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 492 µs | 492 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 421 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 358 µs | 358 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.3x faster than the host |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 669 µs | 669 µs | +0.0% | [+0.0%, +0.1%] | - | ~ | device 1.6x faster than the host (was 1.7x) |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 414 µs | 414 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 339 µs | +0.2% | [-0.0%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `Shapes/wedge/signedDistance/double/hostdevice` | 155 µs | 154 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.9x faster than the host (was 2.8x) |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.02 ms | 8.00 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 886 µs | 883 µs | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ | device 10.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.67 ms | 2.66 ms | -0.4% | [-0.6%, -0.3%] | +0.0% | ~ | device 4.7x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.1 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.3 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 13.9 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 2.99 ms | 3.02 ms | +0.8% | [+0.5%, +1.1%] | +0.0% | ~ | device 7.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.78 ms | 4.77 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | device 5.1x faster than the host (was 6.3x); tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.0 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.27x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | -0.1% | [-0.1%, -0.1%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.5 ms | 30.4 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.1 ms | 44.1 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.5 ms | 45.9 ms | +0.8% | [+0.2%, +1.4%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 121 ms | 123 ms | +1.9% | [+0.0%, +3.9%] | +0.0% | ~ | device 0.7x faster than the host (was 0.7x); tape/fused 1.05x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 126 ms | 129 ms | +3.0% | [+0.6%, +4.2%] | +0.0% | ~ | device 0.8x faster than the host (was 0.8x); tape/fused 1.03x (was 1.07x) |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 126 ms | 130 ms | +2.7% | [+2.2%, +3.1%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 135 ms | 133 ms | -1.5% | [-3.9%, +1.0%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.60 ms | 1.59 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ | device 10.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.1 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ | device 0.6x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.32 ms | 9.33 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 11.6x faster than the host |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.93 ms | 5.93 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 9.9x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 96.1 ms | 95.8 ms | -0.3% | [-2.1%, +1.6%] | - | ~ | device 3.6x faster than the host (was 3.9x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.3 ms | 63.3 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.4x faster than the host (was 3.2x); surface/leaf 1.51x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.2 ms | 29.2 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 5.0x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.99 ms | 9.99 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 6.8x faster than the host; surface/hit 2.93x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.0 ms | 59.1 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.3x faster than the host (was 2.0x) |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.5x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 782 ms | 787 ms | +0.6% | [+0.0%, +1.1%] | - | ~ | device 0.4x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 530 ms | 527 ms | -0.5% | [-1.3%, +0.3%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.49x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 310 ms | 315 ms | +1.8% | [+0.7%, +3.0%] | - | ~ | device 0.5x faster than the host (was 0.6x) |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 121 ms | 120 ms | -0.3% | [-2.2%, +1.6%] | - | ~ | device 0.6x faster than the host; surface/hit 2.62x |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 217 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 11.2x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.0% | [-0.4%, +0.5%] | - | ~ | device 24.3x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 359 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 12.8x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.57 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 9.0x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.0 µs | 71.8 µs | -0.3% | [-0.3%, -0.3%] | - | ~ | device 7.2x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.6 µs | 47.5 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 4.9x faster than the host (was 5.2x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 652 µs | +0.1% | [+0.0%, +0.1%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 12.2x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 476 µs | 476 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.2 µs | 48.3 µs | +0.2% | [-0.3%, +0.8%] | - | ~ | device 26.4x faster than the host (was 28.0x) |
| `Tape/csg/signedDistance/double/hostdevice` | 858 µs | 858 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.1x faster than the host (was 3.0x) |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 811 µs | 810 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.64 ms | 6.65 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 192 µs | +0.2% | [+0.0%, +0.4%] | - | ~ | device 3.2x faster than the host (was 3.0x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.2% | [-0.3%, -0.2%] | - | ~ | device 1.8x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host (was 1.0x) |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.71 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 10.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.51 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4566.1 s → 4567.6 s (+0.0%), the largest peak 6566 MB → 6566 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 11.9 s | 11.8 s | -0.6% | 604 MB | 605 MB | +0.3% | ~ |
| `BVHRefit` | 6.2 s | 6.1 s | -0.7% | 509 MB | 509 MB | +0.0% | ~ |
| `BVHUnion` | 22.3 s | 22.2 s | -0.8% | 1028 MB | 1028 MB | +0.0% | ~ |
| `BVHUnionMixed` | 9.9 s | 9.8 s | -0.5% | 623 MB | 623 MB | +0.0% | ~ |
| `ClosestPrimitive` | 101.5 s | 101.3 s | -0.1% | 1353 MB | 1357 MB | +0.3% | ~ |
| `ClosestSurface` | 142.7 s | 142.7 s | +0.0% | 1844 MB | 1843 MB | -0.1% | ~ |
| `Domain` | 47.3 s | 47.2 s | -0.1% | 991 MB | 993 MB | +0.2% | ~ |
| `DomainObject` | 44.0 s | 43.9 s | -0.0% | 945 MB | 946 MB | +0.1% | ~ |
| `EBGrid` | 17.1 s | 17.2 s | +0.3% | 753 MB | 753 MB | +0.0% | ~ |
| `Fused` | 43.8 s | 43.8 s | +0.0% | 1164 MB | 1164 MB | +0.1% | ~ |
| `LaunchTuning` | 411.0 s | 410.6 s | -0.1% | 3876 MB | 3876 MB | +0.0% | ~ |
| `Mesh` | 18.0 s | 18.2 s | +0.7% | 810 MB | 811 MB | +0.1% | ~ |
| `MeshCSG` | 322.6 s | 322.5 s | -0.0% | 6306 MB | 6306 MB | +0.0% | ~ |
| `MeshClosestPoint` | 16.5 s | 16.5 s | +0.1% | 762 MB | 762 MB | +0.0% | ~ |
| `MeshIngest` | 8.5 s | 8.5 s | +0.1% | 536 MB | 535 MB | -0.1% | ~ |
| `MeshReconcile` | 8.3 s | 8.3 s | -0.0% | 512 MB | 512 MB | -0.0% | ~ |
| `MeshTape` | 613.2 s | 613.1 s | -0.0% | 6566 MB | 6566 MB | +0.0% | ~ |
| `PointCloud` | 10.2 s | 10.2 s | -0.1% | 564 MB | 564 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 5.6 s | 5.6 s | -0.0% | 510 MB | 510 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 5.2 s | 5.2 s | +0.7% | 509 MB | 509 MB | +0.0% | ~ |
| `QueryOrder` | 13.1 s | 13.1 s | -0.1% | 694 MB | 694 MB | -0.1% | ~ |
| `Ray` | 147.7 s | 147.5 s | -0.1% | 3770 MB | 3769 MB | -0.0% | ~ |
| `RayCSG` | 170.0 s | 169.6 s | -0.2% | 4155 MB | 4155 MB | +0.0% | ~ |
| `RayCrossing` | 330.5 s | 330.7 s | +0.1% | 4116 MB | 4116 MB | -0.0% | ~ |
| `RayParticles` | 605.8 s | 606.1 s | +0.1% | 5808 MB | 5808 MB | -0.0% | ~ |
| `RayShell` | 323.5 s | 324.7 s | +0.4% | 5501 MB | 5502 MB | +0.0% | ~ |
| `Regions` | 278.8 s | 278.5 s | -0.1% | 4138 MB | 4139 MB | +0.0% | ~ |
| `Render` | 202.2 s | 202.8 s | +0.3% | 6084 MB | 6083 MB | -0.0% | ~ |
| `Shapes` | 18.9 s | 18.7 s | -1.0% | 585 MB | 586 MB | +0.0% | ~ |
| `SphereCloudBVH` | 6.9 s | 6.9 s | +0.6% | 510 MB | 510 MB | +0.0% | ~ |
| `SphereCloudRays` | 143.4 s | 143.3 s | -0.0% | 4170 MB | 4169 MB | -0.0% | ~ |
| `SphereContacts` | 6.6 s | 6.6 s | -0.0% | 509 MB | 509 MB | -0.0% | ~ |
| `SurfaceParts` | 368.3 s | 369.0 s | +0.2% | 4416 MB | 4416 MB | -0.0% | ~ |
| `Tape` | 49.7 s | 50.0 s | +0.5% | 977 MB | 979 MB | +0.2% | ~ |
| `TapeBVHUnion` | 35.1 s | 35.0 s | -0.1% | 918 MB | 919 MB | +0.1% | ~ |
| **all, in both** | 4566.1 s | 4567.6 s | +0.0% | 6566 MB | 6566 MB | +0.0% | ~ |

Device kernels: 226 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (3 cases): MeshCSG/smooth-tape/frame/float/hostdevice, MeshCSG/smooth-fused/firstHit-morton/double/hostdevice, SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice

These probes compiled to the same code on both sides and were not run (8 cases): PointCloudBVHBuild (4), PointCloudHashGridBuild (4)

<!-- END receipt cuda -->
