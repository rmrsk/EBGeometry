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
head: 5497b3be
head-fingerprint: 532f1e6aa7bc81e6db09b1d613789851d37c2b6b
cases: 356
rounds: 2
reps: 3
date: 2026-11-26
-->

## Host comparison: yellow

**Accepted:** Yes, the performance regressions are acceptable.

`5497b3be` against `344147cc` (origin/main), 2026-11-26.

**VERDICT: 3 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 3 minor, 3 faster, 0 differ, 0 lost, 356 cases

- base: 344147cc (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 5497b3be (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 331 ms | 330 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 513 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 349 ms | +0.4% | [+0.0%, +0.8%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 187 ms | 187 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 564 ms | 567 ms | +0.5% | [-0.4%, +1.5%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.8 ms | 42.4 ms | +1.4% | [+0.7%, +2.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 76.3 ms | 75.7 ms | -0.9% | [-1.2%, -0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.7 ms | 56.2 ms | -0.9% | [-1.5%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.4 ms | 51.3 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.8 ms | 65.0 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 55.0 ms | 55.0 ms | -0.1% | [-0.8%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.0 ms | 60.8 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.6 ms | 72.6 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 72.4 ms | 72.8 ms | +0.5% | [+0.2%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 79.5 ms | 81.1 ms | +2.0% | [-4.2%, +8.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 65.5 ms | 65.7 ms | +0.3% | [-2.0%, +2.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 101 ms | +0.8% | [-0.7%, +2.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 78.1 ms | 77.4 ms | -0.9% | [-2.4%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 111 ms | 109 ms | -1.7% | [-2.9%, -0.4%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 64.1 ms | 64.1 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 95.6 ms | 95.6 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 76.1 ms | 76.4 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 102 ms | 101 ms | -1.1% | [-3.1%, +1.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.5 ms | 20.2 ms | +3.6% | [+2.8%, +4.4%] | +0.0% | minor |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.2 ms | 41.1 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.6 ms | 74.4 ms | +1.0% | [-1.1%, +3.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.9 ms | 20.4 ms | -2.4% | [-3.9%, -0.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.7 ms | 42.0 ms | -4.1% | [-4.6%, -3.5%] | +0.0% | faster |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.9 ms | 81.2 ms | -0.9% | [-1.1%, -0.6%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 139 ms | 139 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 75.7 ms | 76.0 ms | +0.4% | [+0.4%, +0.4%] | +0.0% | ~ | surface/leaf 1.83x |
| `ClosestSurface/union/closestSurface/float/host` | 146 ms | 146 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 77.6 ms | 76.6 ms | -1.2% | [-3.1%, +0.7%] | +0.0% | ~ | surface/leaf 1.91x |
| `ClosestSurface/cloud/closestSurface/double/host` | 157 ms | 157 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 80.7 ms | 80.7 ms | +0.0% | [-0.8%, +0.8%] | +0.0% | ~ | surface/leaf 1.95x |
| `ClosestSurface/union/closestSurface/double/host` | 154 ms | 154 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 81.0 ms | 81.0 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | surface/leaf 1.90x |
| `ConnectedComponents/shells/build-coarse/float/host` | 148 ms | 151 ms | +1.7% | [-0.4%, +3.7%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/float/host` | 667 ms | 666 ms | -0.2% | [-1.6%, +1.2%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-coarse/double/host` | 155 ms | 156 ms | +0.9% | [-1.4%, +3.3%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/double/host` | 695 ms | 690 ms | -0.8% | [-1.3%, -0.3%] | +0.0% | ~ |  |
| `Domain/particles/labels/float/host` | 13.6 ms | 13.6 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.85 ms | 6.81 ms | -0.6% | [-2.2%, +1.1%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.31 ms | 1.32 ms | +1.1% | [-0.1%, +2.3%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.3 ms | 14.2 ms | -1.0% | [-2.0%, +0.0%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.11 ms | 7.13 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.31 ms | 1.32 ms | +1.0% | [+0.2%, +1.8%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 39.5 ms | 39.4 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.4 ms | 21.3 ms | -0.8% | [-1.7%, +0.2%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 566 µs | 562 µs | -0.8% | [-1.0%, -0.6%] | +0.0% | ~ | whole/own 70.08x |
| `DomainObject/partsOwn/labelAt/float/host` | 305 µs | 304 µs | -0.4% | [-1.9%, +1.2%] | +0.0% | ~ | whole/own 70.04x |
| `DomainObject/partsWhole/labels/double/host` | 39.4 ms | 39.6 ms | +0.7% | [-0.0%, +1.3%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.5 ms | 22.7 ms | +0.7% | [+0.3%, +1.1%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 585 µs | 584 µs | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ | whole/own 67.81x |
| `DomainObject/partsOwn/labelAt/double/host` | 315 µs | 318 µs | +0.8% | [-0.6%, +2.2%] | +0.0% | ~ | whole/own 71.49x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/host` | 64.7 ms | 63.9 ms | -1.2% | [-1.8%, -0.7%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/host` | 59.5 ms | 60.0 ms | +0.8% | [-0.4%, +2.0%] | +0.0% | ~ | grid/morton 1.06x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/host` | 78.1 ms | 77.8 ms | -0.3% | [-1.7%, +1.1%] | +0.0% | ~ | grid/shuffled 0.82x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/host` | 68.5 ms | 68.8 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/host` | 63.6 ms | 64.0 ms | +0.5% | [-0.4%, +1.4%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/host` | 89.5 ms | 89.5 ms | -0.0% | [-1.3%, +1.3%] | +0.0% | ~ | grid/shuffled 0.77x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/host` | 193 ms | 193 ms | +0.0% | [-1.7%, +1.8%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/host` | 180 ms | 185 ms | +2.9% | [-0.7%, +6.5%] | +0.0% | ~ | grid/morton 1.04x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/host` | 274 ms | 273 ms | -0.1% | [-10.2%, +11.0%] | +0.0% | ~ | grid/shuffled 0.71x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/host` | 210 ms | 210 ms | -0.2% | [-5.5%, +5.5%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/host` | 199 ms | 197 ms | -1.1% | [-5.6%, +3.4%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/host` | 321 ms | 314 ms | -2.4% | [-12.6%, +8.7%] | +0.0% | ~ | grid/shuffled 0.67x |
| `Fused/csg/signedDistance/float/host` | 4.56 ms | 4.50 ms | -1.4% | [-2.3%, -0.5%] | - | ~ | tape/fused 2.90x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.5 ms | +0.9% | [-0.3%, +2.1%] | - | ~ | tape/fused 2.02x |
| `Fused/chain16/signedDistance/float/host` | 1.00 ms | 989 µs | -1.3% | [-1.9%, -0.6%] | - | ~ | tape/fused 16.46x |
| `Fused/drilled-mesh/signedDistance/float/host` | 47.8 ms | 48.1 ms | +0.6% | [-6.6%, +8.4%] | +0.0% | ~ | tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 48.1 ms | 47.6 ms | -0.9% | [-6.4%, +5.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.62 ms | 4.60 ms | -0.5% | [-1.2%, +0.3%] | - | ~ | tape/fused 2.85x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | tape/fused 2.08x |
| `Fused/chain16/signedDistance/double/host` | 3.92 ms | 3.94 ms | +0.4% | [-0.4%, +1.3%] | - | ~ | tape/fused 4.18x |
| `Fused/drilled-mesh/signedDistance/double/host` | 56.0 ms | 54.1 ms | -3.3% | [-11.8%, +6.2%] | +0.0% | ~ | tape/fused 1.03x (was 0.99x) |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 55.7 ms | 55.9 ms | +0.3% | [-8.1%, +9.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 84.4 ms | 84.2 ms | -0.2% | [-2.6%, +2.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 235 ms | 234 ms | -0.2% | [-4.7%, +4.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 115 ms | 115 ms | +0.3% | [-1.4%, +1.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 275 ms | 273 ms | -0.9% | [-8.7%, +7.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 72.6 ms | 71.9 ms | -0.9% | [-2.8%, +1.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 218 ms | 218 ms | -0.1% | [-4.0%, +3.9%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 98.2 ms | 98.0 ms | -0.2% | [-2.0%, +1.6%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.5 ms | 30.6 ms | +0.4% | [-9.1%, +10.9%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 800 ms | 801 ms | +0.2% | [-7.7%, +8.7%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 120 ms | 121 ms | +1.3% | [-0.9%, +3.6%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 52.1 ms | 50.8 ms | -2.6% | [-23.5%, +22.3%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 1.03 s | 1.12 s | +8.8% | [-26.8%, +57.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 28.2 ms | 29.0 ms | +2.7% | [-12.5%, +20.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 17.1 ms | 17.1 ms | +0.5% | [-12.7%, +15.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 322 ms | +1.0% | [+0.8%, +1.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 232 ms | 233 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.9 ms | 91.5 ms | +0.7% | [-0.1%, +1.5%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 81.6 ms | 82.9 ms | +1.5% | [+0.6%, +2.4%] | +0.0% | ~ | tape/fused 2.81x |
| `MeshCSG/union-tape/firstHit/double/host` | 31.6 ms | 32.0 ms | +1.3% | [-15.4%, +21.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 20.7 ms | 20.7 ms | -0.3% | [-28.1%, +37.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 416 ms | 407 ms | -2.1% | [-24.1%, +26.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 271 ms | 265 ms | -1.9% | [-16.6%, +15.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 108 ms | 100.0 ms | -7.1% | [-17.6%, +5.4%] | +0.0% | ~ | tape/fused 4.07x (was 3.86x) |
| `MeshCSG/smooth-fused/frame/double/host` | 87.3 ms | 87.6 ms | +0.4% | [-0.3%, +1.1%] | +0.0% | ~ | tape/fused 3.03x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 231 ms | 229 ms | -0.7% | [-1.9%, +0.6%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 241 ms | 233 ms | -3.4% | [-6.8%, +0.3%] | +0.0% | ~ | signedDistance/closestPoint 0.99x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 434 ms | 372 ms | -14.4% | [-45.2%, +41.8%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 258 ms | 264 ms | +2.5% | [-15.3%, +24.2%] | +0.0% | ~ | closestPoint/gradient 0.88x (was 0.93x) |
| `MeshClosestPoint/tri/signedDistance/double/host` | 298 ms | 304 ms | +1.9% | [-21.0%, +31.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 293 ms | 294 ms | +0.3% | [-17.0%, +21.2%] | +0.0% | ~ | signedDistance/closestPoint 1.03x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 446 ms | 393 ms | -12.0% | [-35.9%, +26.0%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 315 ms | 312 ms | -0.9% | [-22.7%, +28.5%] | +0.0% | ~ | closestPoint/gradient 0.94x |
| `MeshIngest/armadillo/read-soup/float/host` | 53.7 ms | 53.7 ms | -0.1% | [-0.5%, +0.2%] | - | ~ |  |
| `MeshIngest/armadillo/clean/float/host` | 12.5 ms | 12.6 ms | +0.7% | [+0.2%, +1.2%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/float/host` | 96.8 ms | 97.2 ms | +0.4% | [-0.5%, +1.2%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/float/host` | 103 ms | 105 ms | +1.8% | [-0.9%, +4.4%] | - | ~ |  |
| `MeshIngest/armadillo/read-soup/double/host` | 54.4 ms | 54.6 ms | +0.4% | [-0.4%, +1.2%] | - | ~ |  |
| `MeshIngest/armadillo/clean/double/host` | 13.8 ms | 13.8 ms | +0.3% | [-0.8%, +1.4%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/double/host` | 107 ms | 107 ms | +0.4% | [-0.5%, +1.4%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/double/host` | 116 ms | 116 ms | +0.5% | [+0.0%, +0.9%] | - | ~ |  |
| `MeshReconcile/armadillo/reconcile/float/host` | 75.2 ms | 73.6 ms | -2.2% | [-3.6%, -0.7%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.8 ms | 41.5 ms | -0.7% | [-2.2%, +0.8%] | +0.0% | ~ | reconcile/scheduled 1.77x |
| `MeshReconcile/armadillo/reconcile/double/host` | 95.4 ms | 95.6 ms | +0.2% | [-1.5%, +2.0%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.6 ms | 56.8 ms | +0.3% | [-0.8%, +1.5%] | +0.0% | ~ | reconcile/scheduled 1.68x |
| `MeshTape/mesh/firstHit/float/host` | 53.1 ms | 48.7 ms | -8.4% | [-12.4%, -4.0%] | +0.0% | faster |  |
| `MeshTape/mesh/firstCrossing/float/host` | 48.8 ms | 48.6 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.7 ms | 19.7 ms | +0.0% | [-2.0%, +2.1%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 80.5 ms | 81.3 ms | +0.9% | [+0.2%, +1.5%] | +0.0% | ~ | mesh/tape 0.60x (was 0.66x) |
| `MeshTape/tape/firstCrossing/float/host` | 50.1 ms | 50.3 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ | mesh/tape 0.97x |
| `MeshTape/tape/frame/float/host` | 24.1 ms | 25.2 ms | +4.7% | [+3.9%, +5.3%] | +0.0% | minor | mesh/tape 0.78x (was 0.82x) |
| `MeshTape/view/firstHit/float/host` | 83.8 ms | 83.8 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ | mesh/view 0.58x (was 0.63x) |
| `MeshTape/view/frame/float/host` | 26.2 ms | 26.3 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ | mesh/view 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 53.8 ms | 53.7 ms | -0.1% | [-0.9%, +0.7%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 55.9 ms | 55.4 ms | -0.9% | [-8.5%, +7.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 21.1 ms | 21.0 ms | -0.5% | [-5.4%, +4.7%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 97.4 ms | 92.0 ms | -5.6% | [-17.5%, +8.8%] | +0.0% | ~ | mesh/tape 0.58x (was 0.55x) |
| `MeshTape/tape/firstCrossing/double/host` | 63.6 ms | 63.7 ms | +0.1% | [-22.5%, +29.0%] | +0.0% | ~ | mesh/tape 0.87x |
| `MeshTape/tape/frame/double/host` | 28.4 ms | 28.7 ms | +0.7% | [-7.0%, +9.1%] | +0.0% | ~ | mesh/tape 0.73x |
| `MeshTape/view/firstHit/double/host` | 101 ms | 100 ms | -0.1% | [-17.8%, +21.4%] | +0.0% | ~ | mesh/view 0.53x |
| `MeshTape/view/frame/double/host` | 30.5 ms | 31.1 ms | +1.9% | [-6.2%, +10.8%] | +0.0% | ~ | mesh/view 0.68x |
| `PointCloudBVH/uniform/build/float/host` | 56.3 ms | 56.4 ms | +0.0% | [-10.0%, +11.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 168 ms | 173 ms | +2.9% | [-17.4%, +29.4%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 405 ms | 386 ms | -4.7% | [-17.3%, +10.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 92.3 ms | 92.8 ms | +0.6% | [-21.7%, +28.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 7.09 ms | 7.09 ms | -0.0% | [-8.4%, +9.2%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 60.0 ms | 60.4 ms | +0.7% | [-13.4%, +17.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 183 ms | 193 ms | +5.6% | [-12.7%, +29.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 417 ms | 418 ms | +0.3% | [-10.3%, +12.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 106 ms | 105 ms | -1.1% | [-28.8%, +36.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.43 ms | 7.41 ms | -0.3% | [-10.8%, +11.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 14.9 ms | 14.4 ms | -3.6% | [-12.7%, +7.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 123 ms | 123 ms | -0.1% | [-8.3%, +8.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 496 ms | 495 ms | -0.3% | [-8.0%, +8.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 158 ms | 155 ms | -1.8% | [-19.2%, +19.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 873 ms | 874 ms | +0.0% | [-15.3%, +18.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.7 ms | 15.8 ms | +0.4% | [-7.2%, +8.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 124 ms | 126 ms | +1.9% | [-4.7%, +8.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 506 ms | 507 ms | +0.3% | [-5.1%, +5.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 154 ms | 155 ms | +0.2% | [-10.8%, +12.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 812 ms | 817 ms | +0.6% | [-2.8%, +4.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 55.4 ms | 55.3 ms | -0.2% | [-3.5%, +3.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 134 ms | 135 ms | +0.4% | [-7.7%, +9.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 276 ms | 278 ms | +0.4% | [-4.8%, +6.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 152 ms | 152 ms | -0.3% | [-3.6%, +3.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 137 ms | 137 ms | -0.3% | [-3.6%, +3.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 57.9 ms | 58.3 ms | +0.7% | [-2.5%, +3.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 149 ms | 148 ms | -0.5% | [-10.3%, +10.5%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 316 ms | 304 ms | -3.6% | [-10.4%, +3.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 197 ms | 192 ms | -2.3% | [-28.9%, +34.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 181 ms | 183 ms | +1.2% | [-30.1%, +46.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 12.5 ms | 12.2 ms | -2.6% | [-9.8%, +5.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 120 ms | 124 ms | +3.5% | [-8.6%, +16.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 339 ms | 340 ms | +0.1% | [-12.7%, +14.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 176 ms | 175 ms | -0.9% | [-21.0%, +24.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 344 ms | 343 ms | -0.2% | [-12.5%, +13.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 14.5 ms | 14.4 ms | -1.0% | [-13.5%, +13.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 121 ms | 120 ms | -0.6% | [-8.1%, +7.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 369 ms | 369 ms | -0.2% | [-10.9%, +11.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 205 ms | 203 ms | -0.9% | [-32.2%, +44.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 347 ms | 348 ms | +0.4% | [-19.3%, +24.7%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 37.7 ms | 37.9 ms | +0.3% | [-12.5%, +14.9%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 40.7 ms | 40.8 ms | +0.3% | [-12.3%, +14.5%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 7.56 ms | 7.71 ms | +1.9% | [-12.9%, +19.7%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.30 ms | 9.08 ms | -2.4% | [-4.8%, +0.2%] | +0.0% | ~ |  |
| `QueryOrder/random/mortonOrder/float/host` | 61.9 ms | 61.8 ms | -0.2% | [-3.7%, +3.4%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | 230 ms | 230 ms | +0.2% | [-5.6%, +6.4%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | 163 ms | 163 ms | +0.2% | [-3.4%, +3.9%] | +0.0% | ~ | arrived/ordered 1.41x |
| `QueryOrder/random/mortonOrder/double/host` | 62.8 ms | 63.0 ms | +0.4% | [-1.6%, +2.5%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | 276 ms | 276 ms | -0.2% | [-8.8%, +9.4%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | 173 ms | 176 ms | +1.5% | [-2.8%, +5.9%] | +0.0% | ~ | arrived/ordered 1.57x |
| `Ray/mesh/firstHit/float/host` | 49.7 ms | 49.9 ms | +0.4% | [-4.7%, +5.7%] | +0.0% | ~ | hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled/float/host` | 46.9 ms | 46.8 ms | -0.2% | [-4.4%, +4.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 41.3 ms | 41.3 ms | +0.2% | [-3.9%, +4.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 83.3 ms | 82.3 ms | -1.2% | [-11.0%, +9.9%] | +0.0% | ~ | triangles/polygons 0.61x |
| `Ray/mesh-polygons/intersects/float/host` | 69.3 ms | 69.2 ms | -0.2% | [-10.4%, +11.2%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.24 ms | 5.24 ms | -0.2% | [-2.7%, +2.5%] | - | ~ | tape/fused 1.45x |
| `Ray/csg-tape/firstHit/float/host` | 7.78 ms | 7.57 ms | -2.6% | [-3.8%, -1.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 55.2 ms | 55.1 ms | -0.3% | [-5.8%, +5.6%] | +0.0% | ~ | hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled/double/host` | 53.3 ms | 53.0 ms | -0.7% | [-7.2%, +6.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 46.5 ms | 46.5 ms | +0.2% | [-6.2%, +7.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 95.9 ms | 95.0 ms | -1.0% | [-13.6%, +13.5%] | +0.0% | ~ | triangles/polygons 0.58x |
| `Ray/mesh-polygons/intersects/double/host` | 78.9 ms | 79.1 ms | +0.1% | [-12.5%, +14.7%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.56 ms | 5.59 ms | +0.4% | [-1.7%, +2.6%] | - | ~ | tape/fused 1.43x |
| `Ray/csg-tape/firstHit/double/host` | 7.99 ms | 7.98 ms | -0.2% | [-1.6%, +1.3%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.7 ms | 17.1 ms | -3.5% | [-4.7%, -2.2%] | - | faster | marched/traced 7.04x (was 6.83x) |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 121 ms | 120 ms | -0.4% | [-2.3%, +1.5%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.9 ms | 21.9 ms | +0.1% | [-1.5%, +1.8%] | +0.0% | ~ | marched/traced 6.68x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 146 ms | 147 ms | +0.7% | [-1.2%, +2.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.7 ms | 35.5 ms | -0.5% | [-2.1%, +1.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.1 ms | 19.2 ms | +0.5% | [+0.0%, +1.0%] | - | ~ | marched/traced 6.62x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 128 ms | 127 ms | -0.9% | [-2.5%, +0.8%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.9 ms | 23.7 ms | -0.8% | [-1.4%, -0.2%] | +0.0% | ~ | marched/traced 6.44x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 152 ms | 152 ms | +0.1% | [-1.7%, +1.8%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.0 ms | 38.0 ms | +0.2% | [-2.1%, +2.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 49.7 ms | 49.7 ms | -0.0% | [-4.8%, +5.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 50.5 ms | 49.9 ms | -1.2% | [-7.1%, +5.2%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.65 ms | 7.57 ms | -1.1% | [-2.4%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.26 ms | 6.18 ms | -1.2% | [-4.6%, +2.1%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 198 ms | 188 ms | -5.4% | [-17.5%, +9.6%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 178 ms | 175 ms | -1.9% | [-20.5%, +21.7%] | +0.0% | ~ | hit/crossing 1.07x (was 1.11x) |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 71.7 ms | 76.5 ms | +6.7% | [-8.1%, +22.9%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.9 ms | 56.9 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | hit/crossing 1.35x (was 1.26x) |
| `RayCrossing/mesh/firstHit/double/host` | 53.3 ms | 53.8 ms | +0.9% | [-0.4%, +2.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 53.5 ms | 53.5 ms | +0.0% | [-0.9%, +0.9%] | +0.0% | ~ | hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.93 ms | 7.91 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.29 ms | 6.29 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 202 ms | 204 ms | +1.1% | [+0.0%, +2.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 184 ms | 184 ms | +0.2% | [-1.8%, +2.1%] | +0.0% | ~ | hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.8 ms | 79.1 ms | +1.6% | [+1.5%, +1.7%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 65.0 ms | 65.1 ms | +0.1% | [-0.5%, +0.6%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.0 ms | -0.9% | [-1.5%, -0.3%] | +0.0% | ~ | marched/traced 4.94x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 53.7 ms | 54.5 ms | +1.5% | [-6.2%, +9.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.26 ms | 8.39 ms | +1.6% | [-2.7%, +6.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.03 ms | 2.01 ms | -1.2% | [-4.5%, +2.2%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.05 ms | 6.98 ms | -0.9% | [-3.5%, +1.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.9 ms | 11.9 ms | +0.5% | [-1.6%, +2.6%] | +0.0% | ~ | marched/traced 5.15x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 62.3 ms | 61.5 ms | -1.4% | [-3.5%, +0.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.72 ms | 8.76 ms | +0.5% | [-0.7%, +1.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.06 ms | 2.07 ms | +0.9% | [-1.3%, +3.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.29 ms | 7.32 ms | +0.4% | [-1.0%, +1.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 240 ms | 238 ms | -0.7% | [-1.4%, -0.1%] | +0.0% | ~ | marched/traced 1.39x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 330 ms | 332 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 239 ms | 236 ms | -1.0% | [-1.6%, -0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 255 ms | 255 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ | marched/traced 1.47x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 374 ms | 373 ms | -0.2% | [-1.0%, +0.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 253 ms | 252 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.35 ms | 7.36 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.6 ms | -0.5% | [-1.4%, +0.5%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 266 ms | 266 ms | -0.1% | [-0.3%, -0.0%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.6 ms | +0.9% | [+0.4%, +1.4%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.5 ms | 30.5 ms | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.1 ms | 22.0 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ | regions/hit 1.38x |
| `Regions/particles/region/double/host` | 7.63 ms | 7.65 ms | +0.2% | [-0.6%, +1.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.8 ms | 13.0 ms | +1.0% | [-0.0%, +2.0%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 282 ms | 283 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.1 ms | 13.0 ms | -0.7% | [-1.7%, +0.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.9 ms | 32.0 ms | -2.6% | [-3.1%, -2.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.7 ms | 23.8 ms | +0.4% | [-1.2%, +2.0%] | +0.0% | ~ | regions/hit 1.34x |
| `Render/csg/frame/float/host` | 12.7 ms | 13.2 ms | +3.8% | [-0.4%, +8.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 13.1 ms | 13.1 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.7 ms | 19.6 ms | -0.3% | [-1.5%, +0.9%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.2 ms | 14.1 ms | -0.9% | [-2.2%, +0.5%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.0 ms | 13.8 ms | -1.9% | [-2.3%, -1.4%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.6 ms | 20.7 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ |  |
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
| `Shapes/ellipsoid/signedDistance/float/host` | 33.2 ms | 33.5 ms | +0.8% | [-0.9%, +2.6%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 5.07 ms | 5.11 ms | +0.9% | [-0.4%, +2.2%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.24 ms | 4.24 ms | +0.0% | [-2.1%, +2.3%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.4 ms | 17.4 ms | +0.2% | [+0.0%, +0.4%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.33 ms | 4.37 ms | +0.9% | [-0.6%, +2.5%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.14 ms | 4.14 ms | +0.0% | [-2.1%, +2.2%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.30 ms | 2.33 ms | +1.3% | [-0.9%, +3.5%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.44 ms | 1.46 ms | +1.2% | [+0.1%, +2.2%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.07 ms | 2.09 ms | +1.3% | [+0.3%, +2.3%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.5 ms | 27.7 ms | +0.5% | [-0.5%, +1.5%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 46.3 ms | 46.2 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.39 ms | 5.39 ms | -0.1% | [-0.7%, +0.5%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.47 ms | 4.50 ms | +0.9% | [-0.3%, +2.1%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 22.4 ms | 22.5 ms | +0.1% | [-0.6%, +0.7%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 5.59 ms | 5.92 ms | +5.8% | [-31.4%, +60.7%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.32 ms | 5.26 ms | -1.1% | [-1.4%, -0.8%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.51 ms | 2.50 ms | -0.3% | [-1.6%, +1.0%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.78 ms | 1.80 ms | +0.9% | [-0.3%, +2.1%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.08 ms | 2.04 ms | -1.6% | [-2.3%, -1.0%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 30.4 ms | 30.5 ms | +0.2% | [-0.9%, +1.4%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.7 ms | 56.7 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.5 ms | 36.3 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.7 ms | 92.5 ms | -0.2% | [-0.9%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 63.8 ms | 63.9 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 41.9 ms | 42.3 ms | +1.2% | [-0.5%, +2.9%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 108 ms | 107 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.9 ms | 74.9 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 311 ms | 301 ms | -3.1% | [-7.4%, +1.7%] | +0.0% | ~ | tape/fused 1.03x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 315 ms | 311 ms | -1.2% | [-9.3%, +7.5%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 92.9 ms | 91.1 ms | -2.0% | [-12.0%, +9.4%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 367 ms | 370 ms | +0.8% | [-10.1%, +13.0%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 371 ms | 369 ms | -0.5% | [-11.4%, +11.8%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 169 ms | 171 ms | +0.7% | [-2.3%, +3.9%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 201 ms | 202 ms | +0.5% | [-5.2%, +6.5%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 145 ms | 145 ms | -0.2% | [-2.0%, +1.6%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 90.5 ms | 91.3 ms | +0.9% | [-0.2%, +2.0%] | +0.0% | ~ | surface/leaf 1.59x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 133 ms | 133 ms | +0.3% | [-2.6%, +3.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 74.3 ms | 74.0 ms | -0.4% | [-3.4%, +2.6%] | +0.0% | ~ | surface/leaf 1.80x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 63.7 ms | 63.3 ms | -0.7% | [-4.7%, +3.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 35.4 ms | 35.0 ms | -1.1% | [-5.5%, +3.6%] | +0.0% | ~ | surface/hit 1.81x |
| `SurfaceParts/boxes/closestSurface/double/host` | 146 ms | 145 ms | -0.7% | [-2.4%, +1.1%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 91.6 ms | 92.4 ms | +0.9% | [-0.5%, +2.2%] | +0.0% | ~ | surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 153 ms | 150 ms | -2.4% | [-9.5%, +5.4%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 84.8 ms | 86.7 ms | +2.2% | [-3.3%, +8.1%] | +0.0% | ~ | surface/leaf 1.73x (was 1.81x) |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 71.8 ms | 72.0 ms | +0.3% | [-4.4%, +5.3%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 40.7 ms | 41.1 ms | +1.1% | [-4.8%, +7.4%] | +0.0% | ~ | surface/hit 1.75x |
| `Tape/csg/signedDistance/float/host` | 13.0 ms | 13.1 ms | +0.3% | [-0.9%, +1.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.59 ms | 4.67 ms | +1.8% | [+0.1%, +3.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.4 ms | 25.1 ms | -1.1% | [-2.0%, -0.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.2 ms | 40.3 ms | +0.3% | [-1.6%, +2.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.75 ms | 1.79 ms | +1.8% | [+0.3%, +3.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 129 µs | 135 µs | +4.0% | [-1.4%, +9.7%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.2 ms | 30.2 ms | +0.0% | [-1.8%, +1.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.73 ms | 5.80 ms | +1.2% | [-0.7%, +3.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.5 ms | 16.3 ms | -1.4% | [-3.5%, +0.7%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.68 ms | 4.67 ms | -0.1% | [-1.4%, +1.2%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.1 ms | -0.3% | [-2.3%, +1.7%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.63 ms | 4.63 ms | +0.0% | [-0.3%, +0.4%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.4 ms | 26.6 ms | +1.0% | [-1.4%, +3.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.5 ms | 42.0 ms | -1.1% | [-3.0%, +0.8%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.81 ms | 1.78 ms | -1.4% | [-4.4%, +1.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 285 µs | 275 µs | -3.4% | [-9.8%, +3.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.5 ms | 30.2 ms | -0.9% | [-2.7%, +0.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.82 ms | 7.87 ms | +0.6% | [-1.5%, +2.8%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.3 ms | 16.4 ms | +0.9% | [-0.2%, +1.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.82 ms | 7.85 ms | +0.3% | [-1.1%, +1.8%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 43.2 ms | 42.8 ms | -0.9% | [-4.3%, +2.6%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 201 ms | 217 ms | +8.1% | [+7.8%, +8.4%] | +0.0% | minor |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 43.8 ms | 44.7 ms | +2.2% | [-1.2%, +5.6%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 229 ms | 218 ms | -4.7% | [-10.8%, +2.2%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 765.0 s → 802.3 s (+4.9%), the largest peak 1417 MB → 1615 MB; 6 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.8 s | 9.7 s | -0.5% | 563 MB | 616 MB | +9.5% | ~ |
| `BVHRefit` | 4.3 s | 4.4 s | +0.6% | 503 MB | 513 MB | +1.9% | ~ |
| `BVHUnion` | 19.4 s | 19.4 s | -0.1% | 956 MB | 942 MB | -1.5% | ~ |
| `BVHUnionMixed` | 7.7 s | 7.8 s | +1.0% | 559 MB | 588 MB | +5.2% | ~ |
| `ClosestPrimitive` | 30.6 s | 32.7 s | +6.9% | 1348 MB | 1316 MB | -2.3% | ~ |
| `ClosestSurface` | 28.2 s | 30.3 s | +7.4% | 1318 MB | 1314 MB | -0.3% | ~ |
| `ConnectedComponents` | 15.4 s | 16.5 s | +7.1% | 854 MB | 866 MB | +1.3% | ~ |
| `Domain` | 18.1 s | 19.3 s | +6.4% | 860 MB | 1007 MB | +17.1% | **minor** |
| `DomainObject` | 16.0 s | 17.2 s | +7.3% | 860 MB | 896 MB | +4.2% | ~ |
| `EBGrid` | 14.6 s | 14.9 s | +2.1% | 784 MB | 790 MB | +0.8% | ~ |
| `Fused` | 24.8 s | 25.9 s | +4.8% | 1236 MB | 1289 MB | +4.3% | ~ |
| `LaunchTuning` | 37.5 s | 39.3 s | +4.6% | 1328 MB | 1330 MB | +0.1% | ~ |
| `Mesh` | 16.1 s | 16.2 s | +0.6% | 791 MB | 833 MB | +5.3% | ~ |
| `MeshCSG` | 40.1 s | 41.9 s | +4.6% | 1306 MB | 1326 MB | +1.5% | ~ |
| `MeshClosestPoint` | 14.3 s | 14.4 s | +1.1% | 787 MB | 831 MB | +5.6% | ~ |
| `MeshIngest` | 7.0 s | 7.3 s | +4.3% | 569 MB | 626 MB | +10.1% | ~ |
| `MeshReconcile` | 7.0 s | 7.0 s | -0.6% | 569 MB | 595 MB | +4.6% | ~ |
| `MeshTape` | 41.9 s | 44.7 s | +6.7% | 1261 MB | 1342 MB | +6.4% | ~ |
| `PointCloud` | 7.5 s | 7.5 s | -0.8% | 562 MB | 592 MB | +5.3% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.9 s | +0.7% | 477 MB | 477 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | -0.9% | 457 MB | 452 MB | -1.1% | ~ |
| `QueryOrder` | 11.3 s | 11.6 s | +2.2% | 708 MB | 721 MB | +1.9% | ~ |
| `Ray` | 36.7 s | 39.3 s | +7.0% | 1342 MB | 1336 MB | -0.5% | ~ |
| `RayCSG` | 32.2 s | 34.0 s | +5.7% | 1204 MB | 1273 MB | +5.8% | ~ |
| `RayCrossing` | 45.8 s | 48.3 s | +5.7% | 1360 MB | 1459 MB | +7.2% | ~ |
| `RayParticles` | 49.5 s | 51.8 s | +4.7% | 1334 MB | 1476 MB | +10.6% | **minor** |
| `RayShell` | 37.6 s | 39.9 s | +6.0% | 1312 MB | 1324 MB | +0.9% | ~ |
| `Regions` | 29.8 s | 31.7 s | +6.4% | 1116 MB | 1257 MB | +12.7% | **minor** |
| `Render` | 35.9 s | 38.1 s | +6.0% | 1300 MB | 1326 MB | +2.0% | ~ |
| `ShapeFamilies` | - | 8.6 s | | - | 588 MB | | new |
| `Shapes` | 8.8 s | 9.0 s | +2.1% | 559 MB | 589 MB | +5.3% | ~ |
| `SphereCloudBVH` | 4.8 s | 5.0 s | +4.0% | 518 MB | 530 MB | +2.4% | ~ |
| `SphereCloudRays` | 28.4 s | 29.7 s | +4.5% | 1078 MB | 1217 MB | +12.9% | **minor** |
| `SphereContacts` | 4.3 s | 4.3 s | +2.0% | 490 MB | 496 MB | +1.2% | ~ |
| `SurfaceParts` | 40.8 s | 43.0 s | +5.4% | 1417 MB | 1615 MB | +14.0% | **minor** |
| `Tape` | 16.8 s | 17.4 s | +3.6% | 838 MB | 950 MB | +13.3% | **minor** |
| `TapeBVHUnion` | 14.7 s | 15.5 s | +5.9% | 839 MB | 837 MB | -0.3% | ~ |
| **all, in both** | 765.0 s | 802.3 s | +4.9% | 1417 MB | 1615 MB | +14.0% | **minor** |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (3 cases): ClosestPrimitive/csg/closestPrimitive/float/host, MeshTape/tape/frame/float/host, TapeBVHUnion/maps64-as-written/signedDistance/float/host

The probes differ between the sides (each side ran its own): Performance/ShapeFamilies/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 344147cc
base-fingerprint: 1e0f461958307254a04eb0a9ea2890c216b99160
head: 5497b3be
head-fingerprint: 532f1e6aa7bc81e6db09b1d613789851d37c2b6b
cases: 438
rounds: 2
reps: 3
date: 2026-11-26
-->

## Device comparison (CUDA): yellow

**Accepted:** Yes, the performance regressions are acceptable.

`5497b3be` against `344147cc` (origin/main), 2026-11-26.

**VERDICT: 15 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 15 minor, 13 faster, 0 differ, 0 lost, 438 cases

- base: 344147cc (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 5497b3be (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.63 ms | 1.61 ms | -1.0% | [-3.0%, +1.1%] | - | ~ | device 25.9x faster than the host (was 22.8x) |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.42 ms | 3.43 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 18.7x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.08 ms | 1.06 ms | -1.8% | [-5.4%, +1.9%] | - | ~ | device 19.7x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.32 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 16.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.26 ms | 6.22 ms | -0.7% | [-1.0%, -0.3%] | - | ~ | device 7.4x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.9 ms | -0.2% | [-1.7%, +1.4%] | - | ~ | device 5.5x faster than the host (was 6.0x) |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.5 ms | 33.3 ms | -0.4% | [-1.1%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 56.8 ms | 57.0 ms | +0.4% | [-0.0%, +0.9%] | - | ~ | device 1.2x faster than the host (was 1.4x) |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.4 ms | 20.8 ms | +1.8% | [+1.5%, +2.2%] | - | ~ | device 1.2x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.9 ms | 47.7 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 1.0x faster than the host (was 0.9x) |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 60.7 ms | 61.1 ms | +0.8% | [+0.5%, +1.0%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 108 ms | 109 ms | +0.2% | [-2.1%, +2.7%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.77 ms | 8.79 ms | +0.2% | [-1.8%, +2.3%] | +0.0% | ~ | device 5.5x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 10.2 ms | 10.2 ms | -0.2% | [-1.8%, +1.5%] | +0.0% | ~ | device 8.6x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 387 µs | 389 µs | +0.5% | [+0.2%, +0.8%] | - | ~ | device 15.4x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 844 µs | 843 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 10.2x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.51 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.65 ms | 4.66 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 585 µs | 581 µs | -0.7% | [-0.9%, -0.4%] | - | ~ | device 13.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.37 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 8.5x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.76 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.02 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 905 µs | 911 µs | +0.7% | [+0.4%, +1.0%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.23 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.48 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 848 µs | 850 µs | +0.2% | [-0.2%, +0.7%] | - | ~ | device 13.7x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.07 ms | 2.07 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 8.0x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.11 ms | 9.10 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.80 ms | 9.81 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.7x faster than the host (was 1.8x) |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 307 µs | 313 µs | +2.0% | [+1.7%, +2.2%] | - | ~ | device 11.8x faster than the host (was 12.3x) |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.35 ms | 1.41 ms | +4.3% | [+4.1%, +4.5%] | - | minor | device 6.0x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 59.2 ms | +0.5% | [+0.3%, +0.6%] | - | ~ | device 3.5x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 3.0x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.43 ms | 5.43 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 519 ms | 517 ms | -0.4% | [-1.8%, +1.1%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.3 ms | 20.5 ms | +1.0% | [+0.9%, +1.2%] | - | ~ | device 5.5x faster than the host (was 4.5x) |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.39 ms | 8.42 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 4.8x faster than the host; surface/leaf 2.44x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.6 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 9.8x faster than the host (was 8.7x) |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.18 ms | 6.19 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 11.0x faster than the host (was 10.7x); surface/leaf 2.51x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 148 ms | 148 ms | +0.2% | [-2.3%, +2.7%] | - | ~ | device 0.6x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.0 ms | 60.0 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 0.9x faster than the host (was 0.8x); surface/leaf 2.46x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 72.0 ms | 72.0 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.8x faster than the host (was 1.9x) |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 2.3x faster than the host; surface/leaf 2.57x |
| `Domain/particles/labels/float/hostdevice` | 4.06 ms | 3.72 ms | -8.4% | [-8.8%, -7.9%] | - | faster | device 4.4x faster than the host (was 3.6x) |
| `Domain/particles/labelAt/float/hostdevice` | 2.76 ms | 2.91 ms | +5.7% | [+5.4%, +5.9%] | - | minor | device 2.6x faster than the host (was 2.7x) |
| `Domain/particles/surfaceGroups/float/hostdevice` | 115 µs | 115 µs | +0.1% | [-3.1%, +3.2%] | - | ~ | device 13.4x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.05 ms | 7.75 ms | +10.0% | [+8.2%, +11.4%] | - | minor | device 1.9x faster than the host (was 2.1x) |
| `Domain/particles/labelAt/double/hostdevice` | 3.50 ms | 3.31 ms | -5.3% | [-5.8%, -4.9%] | - | faster | device 2.2x faster than the host (was 2.1x) |
| `Domain/particles/surfaceGroups/double/hostdevice` | 115 µs | 116 µs | +0.6% | [-2.1%, +3.5%] | - | ~ | device 13.2x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 13.2 ms | 12.0 ms | -8.8% | [-9.1%, -8.5%] | - | faster | device 3.9x faster than the host (was 3.5x) |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.88 ms | 9.47 ms | +6.7% | [+6.5%, +7.0%] | - | minor | device 2.5x faster than the host (was 2.7x) |
| `DomainObject/partsOwn/labels/float/hostdevice` | 266 µs | 242 µs | -9.1% | [-10.0%, -8.1%] | - | faster | device 2.8x faster than the host (was 2.6x); whole/own 49.69x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 159 µs | 170 µs | +7.4% | [+7.0%, +8.7%] | - | minor | device 2.1x faster than the host (was 2.3x); whole/own 55.57x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 19.6 ms | 19.5 ms | -0.6% | [-1.4%, +0.2%] | - | ~ | device 2.5x faster than the host (was 2.8x) |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 11.4 ms | 10.7 ms | -5.6% | [-5.6%, -5.5%] | - | faster | device 2.7x faster than the host (was 2.6x) |
| `DomainObject/partsOwn/labels/double/hostdevice` | 391 µs | 394 µs | +0.8% | [-0.6%, +2.3%] | - | ~ | device 1.8x faster than the host; whole/own 49.34x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 236 µs | 232 µs | -1.7% | [-2.2%, -1.1%] | - | ~ | device 1.6x faster than the host (was 1.7x); whole/own 46.28x (was 48.18x) |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/hostdevice` | 10.5 ms | 10.4 ms | -0.7% | [-0.8%, -0.5%] | +0.0% | ~ | device 10.3x faster than the host (was 12.3x) |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/hostdevice` | 10.4 ms | 10.3 ms | -0.4% | [-0.6%, -0.3%] | +0.0% | ~ | device 11.0x faster than the host (was 10.1x); grid/morton 1.01x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/hostdevice` | 17.4 ms | 17.5 ms | +0.7% | [-0.1%, +1.5%] | +0.0% | ~ | device 8.5x faster than the host (was 8.8x); grid/shuffled 0.59x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/hostdevice` | 226 ms | 222 ms | -2.2% | [-3.5%, -0.8%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/hostdevice` | 164 ms | 165 ms | +0.1% | [-2.1%, +2.4%] | +0.0% | ~ | device 0.7x faster than the host (was 0.6x); grid/morton 1.35x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/hostdevice` | 289 ms | 290 ms | +0.5% | [-2.9%, +3.9%] | +0.0% | ~ | device 0.4x faster than the host; grid/shuffled 0.76x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/hostdevice` | 27.3 ms | 27.1 ms | -0.7% | [-1.0%, -0.5%] | +0.0% | ~ | device 5.2x faster than the host (was 4.6x) |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/hostdevice` | 29.2 ms | 29.1 ms | -0.3% | [-1.0%, +0.5%] | +0.0% | ~ | device 3.5x faster than the host; grid/morton 0.93x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/hostdevice` | 60.5 ms | 60.5 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ | device 2.8x faster than the host (was 2.5x); grid/shuffled 0.45x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/hostdevice` | 204 ms | 208 ms | +2.1% | [-2.0%, +6.4%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/hostdevice` | 158 ms | 158 ms | -0.1% | [-2.9%, +2.7%] | +0.0% | ~ | device 0.9x faster than the host (was 0.7x); grid/morton 1.32x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/hostdevice` | 253 ms | 251 ms | -0.8% | [-1.8%, +0.3%] | +0.0% | ~ | device 0.7x faster than the host; grid/shuffled 0.83x |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.2% | [-0.0%, +0.5%] | - | ~ | device 24.0x faster than the host (was 24.9x); tape/fused 4.76x (was 4.56x) |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.7 µs | 49.7 µs | -1.9% | [-3.5%, -0.3%] | - | ~ | device 53.6x faster than the host (was 49.4x); tape/fused 7.60x (was 7.08x) |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.1 µs | 48.7 µs | +1.2% | [-0.8%, +3.3%] | - | ~ | device 10.9x faster than the host (was 9.3x); tape/fused 10.43x (was 9.91x) |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.5 ms | 17.5 ms | -0.2% | [-2.2%, +2.0%] | - | ~ | device 6.8x faster than the host; tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.2 ms | 31.4 ms | +0.8% | [+0.6%, +1.0%] | - | ~ | device 4.6x faster than the host (was 4.9x) |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.1% | [-0.1%, +0.0%] | - | ~ | device 1.5x faster than the host (was 2.0x); tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 282 ms | 281 ms | -0.5% | [-2.7%, +1.7%] | - | ~ | device 0.5x faster than the host (was 0.4x); tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 280 ms | 279 ms | -0.3% | [-1.6%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.30 ms | 8.26 ms | -0.5% | [-1.3%, +0.4%] | +0.0% | ~ | device 11.0x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.39 ms | 8.26 ms | -1.5% | [-2.6%, -0.5%] | +0.0% | ~ | device 11.5x faster than the host (was 10.9x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.23 ms | 8.21 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ | device 11.1x faster than the host (was 12.4x); default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.26 ms | 8.22 ms | -0.6% | [-0.9%, -0.2%] | +0.0% | ~ | device 11.2x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 709 µs | 688 µs | -2.9% | [-4.0%, -1.8%] | +0.0% | ~ | device 3.8x faster than the host (was 3.7x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.79 ms | 1.75 ms | -1.8% | [-2.0%, -1.5%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.02 ms | 1.06 ms | +3.6% | [+3.2%, +4.2%] | +0.0% | minor | device 2.5x faster than the host (was 2.6x); default/this 0.65x (was 0.69x) |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 797 µs | 798 µs | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 3.3x faster than the host; default/this 0.86x (was 0.89x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 745 µs | 743 µs | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ | device 12.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 821 µs | 822 µs | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ | device 11.1x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 743 µs | 739 µs | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ | device 12.5x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 735 µs | 731 µs | -0.5% | [-1.6%, +0.6%] | +0.0% | ~ | device 12.2x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 158 ms | 158 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 161 ms | 162 ms | +0.4% | [-1.5%, +2.4%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 165 ms | 162 ms | -2.0% | [-3.5%, -0.4%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 163 ms | 165 ms | +0.9% | [-1.1%, +3.0%] | +0.0% | ~ | device 0.7x faster than the host (was 0.6x); default/this 0.96x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 2.0x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.45 ms | 1.45 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ | device 2.0x faster than the host (was 1.9x); default/this 0.96x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.93 ms | 1.92 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.73x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.56 ms | +0.4% | [+0.3%, +0.6%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.24 ms | 7.24 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.29 ms | 7.28 ms | -0.1% | [-1.0%, +0.7%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.22 ms | 7.23 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.31 ms | 7.32 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.41 ms | 8.41 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 11.4x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.9 ms | +1.4% | [+1.3%, +1.4%] | - | ~ | device 7.2x faster than the host (was 7.5x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 167 ms | 167 ms | +0.0% | [-4.3%, +4.5%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 282 ms | 282 ms | -0.1% | [-1.2%, +1.1%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.6 ms | 24.7 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 4.1x faster than the host (was 4.5x) |
| `MeshSDF/query-random/float/hostdevice` | 57.9 ms | 58.0 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 2.6x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 158 ms | 161 ms | +1.7% | [-0.7%, +4.1%] | - | ~ | device 0.6x faster than the host (was 0.7x) |
| `MeshSDF/query-random/double/hostdevice` | 244 ms | 243 ms | -0.5% | [-1.2%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.04 ms | 2.05 ms | +0.4% | [-0.3%, +1.1%] | - | ~ | device 5.2x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.47 ms | 3.45 ms | -0.6% | [-1.2%, -0.0%] | - | ~ | device 4.3x faster than the host (was 4.1x) |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.17 ms | 6.18 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 4.8x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 64.4 ms | 64.4 ms | -0.0% | [-2.0%, +2.0%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 96.5 ms | 94.4 ms | -2.2% | [-3.7%, -0.6%] | - | ~ | device 2.6x faster than the host (was 2.3x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 240 ms | 238 ms | -0.9% | [-2.9%, +1.2%] | - | ~ | device 3.9x faster than the host (was 3.6x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.8 ms | 24.6 ms | -0.7% | [-1.5%, +0.1%] | - | ~ | device 2.2x faster than the host (was 2.1x); tape/fused 2.62x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.7 ms | 26.7 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 2.3x faster than the host (was 2.5x); tape/fused 3.54x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.8 ms | 65.7 ms | -0.1% | [-0.9%, +0.6%] | - | ~ | device 4.1x faster than the host (was 4.4x); tape/fused 3.62x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.9 ms | 21.7 ms | -0.7% | [-3.7%, +2.4%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 39.5 ms | 38.5 ms | -2.4% | [-3.5%, -1.3%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 41.8 ms | 42.0 ms | +0.6% | [-2.6%, +3.8%] | - | ~ | device 0.7x faster than the host (was 0.8x) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 542 ms | 556 ms | +2.6% | [-3.5%, +8.8%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 859 ms | 856 ms | -0.4% | [-1.0%, +0.2%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.95 s | 1.97 s | +0.7% | [-1.0%, +2.5%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 199 ms | 205 ms | +3.3% | [-0.7%, +6.4%] | - | ~ | device 0.2x faster than the host (was 0.3x); tape/fused 2.71x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 312 ms | 305 ms | -2.4% | [-2.8%, -2.0%] | - | ~ | device 0.2x faster than the host; tape/fused 2.81x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 539 ms | 534 ms | -0.9% | [-3.9%, +2.1%] | - | ~ | device 0.5x faster than the host (was 0.5x); tape/fused 3.69x |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 12.1 ms | 12.0 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 10.7x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 58.5 ms | 58.4 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 2.5x faster than the host (was 2.6x) |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 264 ms | 263 ms | -0.4% | [-1.3%, +0.4%] | - | ~ | device 0.5x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 240 ms | 238 ms | -0.9% | [-1.8%, +0.0%] | - | ~ | device 0.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 18.4x faster than the host (was 17.7x) |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.31 ms | 5.31 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 11.9x faster than the host |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.32 ms | 1.32 ms | +0.0% | [-0.9%, +1.0%] | - | ~ | device 16.0x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 16.1x faster than the host (was 17.9x) |
| `MeshTape/mesh/frame/float/hostdevice` | 2.12 ms | 2.13 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 8.1x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.52 ms | 3.52 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 10.4x faster than the host; mesh/tape 0.38x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.77 ms | 2.77 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 8.0x faster than the host; mesh/tape 0.47x |
| `MeshTape/tape/frame/float/hostdevice` | 4.42 ms | 4.45 ms | +0.9% | [+0.5%, +1.3%] | - | ~ | device 5.0x faster than the host; mesh/tape 0.48x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.74 ms | 3.75 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 9.9x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.65 ms | 5.69 ms | +0.6% | [+0.2%, +1.1%] | - | ~ | device 4.3x faster than the host; mesh/view 0.37x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 30.3 ms | 30.4 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 30.3 ms | 30.2 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 26.0 ms | 25.8 ms | -0.5% | [-1.6%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 43.5 ms | 43.4 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 0.9x faster than the host (was 0.9x); mesh/tape 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 30.6 ms | 30.5 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 0.9x faster than the host (was 0.8x); mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 32.9 ms | 32.9 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 0.7x faster than the host (was 0.8x); mesh/tape 0.79x |
| `MeshTape/view/firstHit/double/hostdevice` | 43.5 ms | 43.4 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.0x faster than the host (was 0.9x); mesh/view 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 36.1 ms | 35.8 ms | -0.7% | [-1.2%, -0.3%] | - | ~ | device 0.7x faster than the host; mesh/view 0.72x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 684 µs | 682 µs | -0.4% | [-1.0%, +0.2%] | - | ~ | device 11.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.83 ms | 1.83 ms | +0.4% | [+0.0%, +0.8%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 994 µs | 994 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 9.6x faster than the host (was 10.0x) |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.60 ms | 4.60 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 5.6x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.97 ms | 2.97 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.09 ms | 7.10 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.79 ms | 7.80 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.4 ms | +0.3% | [-0.5%, +1.1%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.45 ms | 1.47 ms | +1.3% | [+0.7%, +2.0%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.87 ms | 3.90 ms | +0.7% | [+0.5%, +0.8%] | - | ~ | device 3.4x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 530 µs | 523 µs | -1.2% | [-2.4%, -0.0%] | - | ~ | device 16.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.52 ms | 4.51 ms | -0.2% | [-1.3%, +1.0%] | - | ~ | device 6.9x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.30 ms | 4.26 ms | -0.9% | [-1.8%, +0.1%] | - | ~ | device 5.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.08 ms | 8.05 ms | -0.3% | [-1.4%, +0.9%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.9 ms | 15.7 ms | -1.2% | [-2.2%, -0.2%] | - | ~ | device 0.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.02 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.6 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.7% | [-1.8%, +0.5%] | - | ~ | device 1.8x faster than the host (was 1.6x) |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.58 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 10.5x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.61 ms | 4.60 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 831 µs | 834 µs | +0.3% | [-0.2%, +0.9%] | - | ~ | device 10.4x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.85 ms | 2.86 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 7.1x faster than the host (was 7.6x) |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.14 ms | 4.16 ms | +0.5% | [+0.3%, +0.7%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.5 ms | 16.6 ms | +1.0% | [+0.2%, +1.7%] | - | ~ | device 1.0x faster than the host (was 1.2x) |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.0 ms | 21.1 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.94 ms | 8.96 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.5 ms | 14.5 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.5x faster than the host (was 1.5x) |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.2 ms | 20.2 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.74 ms | 2.71 ms | -1.0% | [-1.7%, -0.2%] | - | ~ | device 4.8x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.74 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 2.4x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.71 ms | 2.71 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 8.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.42 ms | 6.43 ms | +0.2% | [-0.4%, +0.7%] | - | ~ | device 3.1x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.6 ms | 33.9 ms | +0.7% | [+0.2%, +1.2%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.5 ms | 68.3 ms | -0.2% | [-1.0%, +0.5%] | - | ~ | device 0.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.4 ms | -0.8% | [-1.1%, -0.5%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.8 ms | -0.0% | [-0.7%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.0 ms | 38.5 ms | +1.4% | [+0.7%, +2.0%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.70 ms | 9.67 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ | device 3.7x faster than the host (was 3.6x) |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.9 ms | 12.9 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 4.0x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.29 ms | 3.27 ms | -0.8% | [-0.9%, -0.7%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.58 ms | 2.59 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | device 3.4x faster than the host (was 3.3x) |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.58 ms | 4.57 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.05 ms | 3.06 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | device 3.2x faster than the host |
| `QueryOrder/random/mortonOrder/float/hostdevice` | 16.9 ms | 16.9 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 3.6x faster than the host |
| `QueryOrder/armadillo/distance-arrived/float/hostdevice` | 9.17 ms | 9.11 ms | -0.7% | [-1.9%, +0.6%] | +0.0% | ~ | device 5.1x faster than the host |
| `QueryOrder/armadillo/distance-ordered/float/hostdevice` | 7.46 ms | 7.39 ms | -0.9% | [-1.3%, -0.6%] | +0.0% | ~ | device 5.5x faster than the host; arrived/ordered 1.23x |
| `QueryOrder/random/mortonOrder/double/hostdevice` | 17.5 ms | 17.6 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.5x faster than the host |
| `QueryOrder/armadillo/distance-arrived/double/hostdevice` | 112 ms | 113 ms | +0.8% | [+0.1%, +1.6%] | +0.0% | ~ | device 0.4x faster than the host |
| `QueryOrder/armadillo/distance-ordered/double/hostdevice` | 78.1 ms | 79.5 ms | +1.8% | [-1.6%, +5.2%] | +0.0% | ~ | device 0.5x faster than the host; arrived/ordered 1.42x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 706 µs | 701 µs | -0.7% | [-1.5%, +0.1%] | - | ~ | device 17.7x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.31 ms | 1.31 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 15.5x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 652 µs | 658 µs | +0.9% | [+0.4%, +1.5%] | - | ~ | device 17.6x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.24 ms | 1.24 ms | +0.0% | [-0.9%, +1.0%] | - | ~ | device 15.5x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 626 µs | 622 µs | -0.6% | [-1.0%, -0.1%] | - | ~ | device 16.2x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.16 ms | 1.17 ms | +0.8% | [+0.6%, +1.0%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.19 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.59x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.49 ms | 2.49 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 9.9x faster than the host (was 11.4x); triangles/polygons 0.53x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | -0.7% | [-0.9%, -0.4%] | - | ~ | device 12.1x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 10.1x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 597 µs | 613 µs | +2.6% | [-0.1%, +5.4%] | - | ~ | device 2.5x faster than the host (was 2.6x); tape/fused 1.09x (was 1.19x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 726 µs | 691 µs | -4.8% | [-9.1%, -0.5%] | - | faster | device 2.6x faster than the host (was 2.9x); tape/fused 1.21x (was 1.16x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 708 µs | 671 µs | -5.2% | [-5.5%, -4.9%] | - | faster | device 3.7x faster than the host (was 3.6x) |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 845 µs | 835 µs | -1.3% | [-2.0%, -0.5%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 18.7 ms | 18.6 ms | -0.3% | [-1.3%, +0.8%] | - | ~ | device 0.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 30.2 ms | 30.2 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 0.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 18.1 ms | 18.1 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 29.0 ms | 29.2 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.9 ms | 15.8 ms | -0.8% | [-1.2%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 26.0 ms | 26.1 ms | +0.4% | [-0.5%, +1.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 17.2 ms | 17.2 ms | -0.0% | [-1.4%, +1.4%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.08x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.6 ms | 25.5 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.19x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | -0.6% | [-1.3%, +0.2%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 23.1 ms | +0.2% | [-0.5%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.37 ms | -0.0% | [-0.7%, +0.7%] | - | ~ | device 0.8x faster than the host; tape/fused 1.06x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.38 ms | 1.39 ms | +0.4% | [+0.2%, +0.5%] | - | ~ | device 2.0x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.50 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.53 ms | 2.53 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 3.0x faster than the host (was 2.6x); marched/traced 3.56x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.99 ms | 2.97 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 2.8x faster than the host (was 2.3x); marched/traced 4.66x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.01 ms | 9.01 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 5.3x faster than the host (was 6.1x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 4.4x faster than the host (was 5.0x) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.16 ms | 2.25 ms | +4.3% | [+4.0%, +4.5%] | - | minor | device 3.8x faster than the host (was 4.0x); marched/traced 12.87x (was 13.41x) |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.43 ms | 5.53 ms | +1.9% | [+1.6%, +2.1%] | - | ~ | device 3.6x faster than the host; marched/traced 9.82x (was 10.15x) |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.0 ms | 29.0 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 2.8x faster than the host (was 2.5x) |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.1 ms | 54.3 ms | -1.4% | [-1.4%, -1.3%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.24 ms | 5.33 ms | +1.8% | [+1.5%, +2.0%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.82 ms | 8.94 ms | +1.4% | [+1.3%, +1.5%] | - | ~ | device 3.1x faster than the host (was 3.2x) |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.20 ms | 6.14 ms | -0.9% | [-1.8%, +0.0%] | - | ~ | device 1.3x faster than the host; marched/traced 26.00x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 0.8x faster than the host (was 0.8x); marched/traced 24.01x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 160 ms | +0.5% | [-1.4%, +2.5%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 257 ms | 254 ms | -1.1% | [-2.2%, -0.0%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.87 ms | 4.86 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 22.68x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 1.3x faster than the host; marched/traced 18.00x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 110 ms | 110 ms | +0.4% | [-3.4%, +4.4%] | - | ~ | device 0.7x faster than the host (was 0.8x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 215 ms | 219 ms | +2.0% | [-0.3%, +2.6%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.3 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.6 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 1.1x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 701 µs | 709 µs | +1.1% | [+0.3%, +2.0%] | - | ~ | device 17.5x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.31 ms | 1.31 ms | +0.4% | [-0.4%, +1.1%] | - | ~ | device 15.8x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 709 µs | 703 µs | -0.8% | [-3.0%, +1.4%] | - | ~ | device 20.9x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.31 ms | 1.32 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 18.9x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 709 µs | 676 µs | -4.7% | [-5.0%, -4.3%] | - | faster | device 4.9x faster than the host (was 4.7x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 855 µs | 840 µs | -1.8% | [-2.4%, -1.2%] | - | ~ | device 4.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 604 µs | 614 µs | +1.6% | [+1.0%, +2.3%] | - | ~ | device 4.0x faster than the host (was 4.2x); hit/crossing 1.10x (was 1.17x) |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 625 µs | 640 µs | +2.3% | [+1.9%, +2.8%] | - | ~ | device 4.5x faster than the host (was 4.6x); hit/crossing 1.31x (was 1.37x) |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.77 ms | 4.78 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 11.5x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.3 ms | 10.3 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 7.0x faster than the host (was 7.5x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.24 ms | 4.23 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 10.2x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.16 ms | 9.16 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 6.9x faster than the host; hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.4 ms | 57.8 ms | +2.4% | [-0.1%, +5.1%] | - | ~ | device 3.4x faster than the host (was 3.2x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 82.4 ms | 83.4 ms | +1.1% | [+0.7%, +1.5%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.6 ms | 48.9 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 3.0x faster than the host; hit/crossing 1.18x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.4 ms | 64.3 ms | +1.4% | [+0.8%, +1.9%] | - | ~ | device 2.7x faster than the host (was 2.6x); hit/crossing 1.30x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 18.6 ms | 18.7 ms | +0.5% | [+0.2%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 30.2 ms | 30.3 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 18.6 ms | 18.7 ms | +0.0% | [-0.7%, +0.8%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 30.2 ms | 30.2 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.12 ms | +1.8% | [+1.5%, +2.1%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.14x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 68.5 ms | 68.5 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 131 ms | 130 ms | -0.6% | [-1.3%, +0.2%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 62.1 ms | 62.1 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 0.8x faster than the host (was 0.9x); hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 119 ms | 117 ms | -1.6% | [-2.7%, -0.4%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 511 ms | 486 ms | -4.9% | [-10.8%, +1.8%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 709 ms | 704 ms | -0.7% | [-2.1%, +0.8%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 393 ms | 411 ms | +4.7% | [-0.1%, +9.6%] | - | ~ | device 0.4x faster than the host (was 0.4x); hit/crossing 1.18x (was 1.30x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 530 ms | 530 ms | +0.0% | [-1.7%, +1.7%] | - | ~ | device 0.3x faster than the host (was 0.3x); hit/crossing 1.33x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.62 ms | 3.65 ms | +0.7% | [-3.7%, +5.3%] | - | ~ | device 1.2x faster than the host; marched/traced 3.32x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.75 ms | 4.77 ms | +0.4% | [+0.0%, +0.8%] | - | ~ | device 0.9x faster than the host (was 1.2x); marched/traced 5.60x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.1 ms | +0.7% | [+0.4%, +0.9%] | - | ~ | device 4.0x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.5 ms | 26.7 ms | +0.9% | [+0.5%, +1.2%] | - | ~ | device 2.1x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.24 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 2.5x faster than the host (was 3.0x) |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.86 ms | 1.86 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 1.9x faster than the host (was 1.8x) |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 99.1 µs | 97.7 µs | -1.5% | [-2.3%, -0.6%] | - | ~ | device 18.6x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 157 µs | 155 µs | -1.3% | [-2.6%, -0.1%] | - | ~ | device 13.1x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 798 µs | 802 µs | +0.5% | [+0.4%, +0.7%] | - | ~ | device 3.7x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 937 µs | 944 µs | +0.8% | [+0.4%, +1.2%] | - | ~ | device 3.3x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 36.0 ms | 35.1 ms | -2.7% | [-5.6%, +0.2%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 3.47x (was 3.35x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.3 ms | 36.2 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 0.2x faster than the host; marched/traced 6.38x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 121 ms | 122 ms | +0.9% | [-0.9%, +2.6%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 231 ms | 231 ms | +0.3% | [-1.1%, +2.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.0 ms | 16.5 ms | +3.3% | [-1.1%, +8.0%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 21.9 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.17 ms | 1.14 ms | -2.1% | [-4.1%, +2.8%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.52 ms | 2.51 ms | -0.3% | [-1.6%, +1.0%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.95 ms | 5.98 ms | +0.4% | [-0.2%, +1.1%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.71 ms | 6.72 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.1 ms | 23.0 ms | -0.3% | [-1.8%, +1.3%] | - | ~ | device 4.0x faster than the host (was 2.7x); marched/traced 2.73x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.3 ms | +0.1% | [-0.8%, +0.9%] | - | ~ | device 2.6x faster than the host; marched/traced 3.72x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.3 ms | 62.9 ms | +1.0% | [+0.3%, +1.7%] | - | ~ | device 4.7x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 104 ms | 105 ms | +0.9% | [+0.2%, +1.6%] | - | ~ | device 3.5x faster than the host (was 3.2x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.9 ms | 13.8 ms | -1.1% | [-2.0%, -0.2%] | - | ~ | device 4.9x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.5 ms | +0.2% | [-1.0%, +1.3%] | - | ~ | device 4.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 309 ms | 304 ms | -1.6% | [-5.3%, +2.4%] | - | ~ | device 0.2x faster than the host; marched/traced 1.85x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 358 ms | 350 ms | -2.2% | [-4.7%, +0.3%] | - | ~ | device 0.2x faster than the host; marched/traced 2.81x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 564 ms | 563 ms | -0.3% | [-2.3%, +1.7%] | - | ~ | device 0.5x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 993 ms | 985 ms | -0.7% | [-1.2%, -0.3%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 185 ms | 189 ms | +1.8% | [+0.3%, +3.4%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 240 ms | 238 ms | -0.7% | [-2.5%, +1.0%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | 3.00 ms | 2.80 ms | -6.7% | [-7.0%, -6.3%] | - | faster | device 2.8x faster than the host (was 2.6x) |
| `Regions/particles/regionDistances/float/hostdevice` | 3.66 ms | 3.30 ms | -9.8% | [-10.6%, -9.0%] | - | faster | device 4.1x faster than the host (was 3.7x) |
| `Regions/particles/regionByRegion/float/hostdevice` | 97.5 ms | 79.9 ms | -18.1% | [-18.6%, -17.6%] | - | faster | device 4.3x faster than the host (was 3.2x); one-pass/each 0.04x (was 0.04x) |
| `Regions/particles/domainLabels/float/hostdevice` | 3.72 ms | 3.41 ms | -8.4% | [-8.5%, -8.2%] | - | faster | device 4.2x faster than the host (was 4.0x) |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.77 ms | 6.84 ms | +1.0% | [+0.8%, +1.3%] | - | ~ | device 3.8x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.63 ms | 5.53 ms | -1.8% | [-1.8%, -1.7%] | - | ~ | device 3.3x faster than the host (was 3.2x); regions/hit 1.24x |
| `Regions/particles/region/double/hostdevice` | 3.41 ms | 3.57 ms | +4.5% | [+4.2%, +5.2%] | - | minor | device 2.2x faster than the host (was 2.3x) |
| `Regions/particles/regionDistances/double/hostdevice` | 5.79 ms | 5.94 ms | +2.5% | [+2.1%, +3.0%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 119 ms | 127 ms | +7.0% | [+6.5%, +8.2%] | - | minor | device 2.5x faster than the host (was 2.6x); one-pass/each 0.05x (was 0.05x) |
| `Regions/particles/domainLabels/double/hostdevice` | 5.92 ms | 6.07 ms | +2.6% | [+2.0%, +3.1%] | - | ~ | device 2.3x faster than the host (was 2.4x) |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.8 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 12.9 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 1.4x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.46 ms | +3.7% | [+3.6%, +3.9%] | - | minor | device 3.0x faster than the host (was 3.1x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.36 ms | 2.44 ms | +3.7% | [+3.6%, +3.9%] | - | minor | device 3.0x faster than the host (was 3.1x) |
| `Render/mesh/frame/float/hostdevice` | 2.15 ms | 2.14 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 9.8x faster than the host (was 7.9x) |
| `Render/csg/frame/double/hostdevice` | 5.62 ms | 5.66 ms | +0.7% | [+0.4%, +0.9%] | - | ~ | device 1.8x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.63 ms | 5.67 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 1.4x faster than the host (was 1.7x) |
| `Render/mesh/frame/double/hostdevice` | 25.8 ms | 25.8 ms | -0.2% | [-1.2%, +0.7%] | - | ~ | device 0.7x faster than the host |
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
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 155 µs | 155 µs | +0.1% | [+0.0%, +0.3%] | - | ~ | device 33.2x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.8 µs | 47.6 µs | -0.4% | [-0.6%, -0.1%] | - | ~ | device 19.2x faster than the host |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.9 µs | 47.9 µs | -0.1% | [-0.2%, +0.1%] | - | ~ | device 15.1x faster than the host |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.2% | [-0.1%, +0.4%] | - | ~ | device 66.0x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 48.1 µs | 47.8 µs | -0.6% | [-1.7%, +0.4%] | - | ~ | device 16.4x faster than the host |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 48.0 µs | 48.0 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 17.4x faster than the host |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 47.9 µs | 48.1 µs | +0.5% | [+0.3%, +0.8%] | - | ~ | device 9.6x faster than the host (was 10.5x) |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.7 µs | 47.7 µs | -0.1% | [-0.3%, +0.0%] | - | ~ | device 7.4x faster than the host (was 8.2x) |
| `Shapes/wedge/signedDistance/float/hostdevice` | 47.7 µs | 47.7 µs | +0.0% | [-0.4%, +0.5%] | - | ~ | device 8.5x faster than the host |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 559 µs | 556 µs | -0.5% | [-1.3%, +0.2%] | - | ~ | device 8.3x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.23 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 492 µs | 493 µs | +0.2% | [-0.0%, +0.5%] | - | ~ | device 2.1x faster than the host (was 2.2x) |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 421 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.0x faster than the host (was 1.9x) |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.4x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 358 µs | 358 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.3x faster than the host (was 2.9x) |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 671 µs | 669 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 2.0x faster than the host (was 1.7x) |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 414 µs | 413 µs | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.3x faster than the host (was 1.4x) |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 339 µs | +0.0% | [+0.0%, +0.1%] | - | ~ | device 1.5x faster than the host (was 1.3x) |
| `Shapes/wedge/signedDistance/double/hostdevice` | 154 µs | 154 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.1x faster than the host (was 2.8x) |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.01 ms | 8.01 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 892 µs | 892 µs | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ | device 10.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.65 ms | 2.66 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 5.1x faster than the host (was 4.8x) |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.6 ms | 13.6 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ | device 3.9x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ | device 0.9x faster than the host (was 0.8x) |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 4.0x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 4.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.02 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ | device 7.4x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.79 ms | 4.76 ms | -0.7% | [-0.8%, -0.6%] | +0.0% | ~ | device 5.2x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 5.3x faster than the host; tape/fused 1.38x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.1 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 4.0x faster than the host; tape/fused 1.27x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 23.0 ms | 23.1 ms | +0.3% | [+0.3%, +0.3%] | +0.0% | ~ | device 3.9x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.6 ms | 30.5 ms | -0.5% | [-0.7%, -0.4%] | +0.0% | ~ | device 3.4x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.3 ms | 44.1 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.6 ms | 45.8 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 123 ms | 122 ms | -0.6% | [-2.7%, +1.5%] | +0.0% | ~ | device 0.8x faster than the host (was 0.8x); tape/fused 1.05x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 130 ms | 125 ms | -3.9% | [-4.4%, -3.4%] | +0.0% | faster | device 0.8x faster than the host (was 0.8x); tape/fused 1.08x (was 1.02x) |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 127 ms | 128 ms | +1.2% | [-0.8%, +3.2%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 132 ms | 135 ms | +2.3% | [+0.3%, +4.4%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 10.9x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.1 ms | -0.1% | [-0.1%, +0.0%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x) |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.29 ms | 9.31 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 12.1x faster than the host (was 13.2x) |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.91 ms | 5.92 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 10.4x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 98.3 ms | 96.7 ms | -1.6% | [-2.5%, -0.8%] | - | ~ | device 4.0x faster than the host (was 4.9x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.9 ms | 63.6 ms | -0.5% | [-0.7%, -0.2%] | - | ~ | device 3.6x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.8 ms | 30.0 ms | +0.6% | [+0.4%, +0.9%] | - | ~ | device 5.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 10.0 ms | 9.99 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 7.2x faster than the host (was 7.4x); surface/hit 3.00x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.1 ms | 59.1 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.9x faster than the host (was 1.9x) |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.5x faster than the host (was 1.6x); surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 785 ms | 781 ms | -0.6% | [-1.1%, +0.0%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 525 ms | 530 ms | +1.0% | [+0.8%, +1.2%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.47x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 315 ms | 312 ms | -0.9% | [-1.8%, -0.0%] | - | ~ | device 0.5x faster than the host (was 0.6x) |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 128 ms | 129 ms | +0.3% | [-0.5%, +1.2%] | - | ~ | device 0.6x faster than the host; surface/hit 2.43x |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 228 µs | +4.6% | [+4.5%, +4.8%] | - | minor | device 10.9x faster than the host (was 11.3x) |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.3% | [+0.0%, +0.5%] | - | ~ | device 28.6x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 378 µs | +5.3% | [+5.2%, +5.4%] | - | minor | device 12.1x faster than the host (was 12.8x) |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.55 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 9.3x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 71.9 µs | 75.5 µs | +5.0% | [+4.8%, +7.4%] | - | minor | device 6.7x faster than the host (was 7.3x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 48.3 µs | +1.2% | [+0.3%, +2.1%] | - | ~ | device 4.9x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 653 µs | +0.4% | [+0.3%, +0.5%] | - | ~ | device 10.8x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 12.0x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 476 µs | 508 µs | +6.6% | [+6.3%, +6.8%] | - | minor | device 7.3x faster than the host (was 7.7x) |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 27.4x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 858 µs | 858 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host (was 1.5x) |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.65 ms | 6.65 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 192 µs | +0.5% | [+0.4%, +0.5%] | - | ~ | device 3.0x faster than the host (was 3.1x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.8x faster than the host (was 1.9x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.59 ms | -1.2% | [-1.4%, -1.0%] | - | ~ | device 8.4x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.80 ms | +2.6% | [+2.4%, +2.8%] | - | ~ | device 11.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.50 ms | 6.50 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.7 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 3.0x faster than the host (was 2.6x) |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4677.0 s → 4949.4 s (+5.8%), the largest peak 5452 MB → 5377 MB; 11 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.6 s | 15.0 s | +10.3% | 618 MB | 625 MB | +1.2% | ~ |
| `BVHRefit` | 7.2 s | 7.5 s | +4.2% | 523 MB | 530 MB | +1.4% | ~ |
| `BVHUnion` | 25.4 s | 26.8 s | +5.3% | 1042 MB | 1051 MB | +0.9% | ~ |
| `BVHUnionMixed` | 11.3 s | 12.3 s | +8.2% | 636 MB | 643 MB | +1.1% | ~ |
| `ClosestPrimitive` | 106.2 s | 125.1 s | +17.8% | 1371 MB | 1537 MB | +12.1% | **minor** |
| `ClosestSurface` | 138.5 s | 157.4 s | +13.7% | 1316 MB | 1413 MB | +7.3% | **minor** |
| `ConnectedComponents` | 20.5 s | 22.5 s | +9.7% | 952 MB | 1038 MB | +9.1% | ~ |
| `Domain` | 50.8 s | 56.9 s | +12.1% | 1007 MB | 1112 MB | +10.5% | **minor** |
| `DomainObject` | 49.4 s | 54.8 s | +11.0% | 959 MB | 1047 MB | +9.2% | **minor** |
| `EBGrid` | 19.7 s | 20.7 s | +4.9% | 767 MB | 773 MB | +0.8% | ~ |
| `Fused` | 50.6 s | 63.6 s | +25.7% | 1178 MB | 1267 MB | +7.6% | **MAJOR** |
| `LaunchTuning` | 454.7 s | 483.0 s | +6.2% | 3134 MB | 3081 MB | -1.7% | ~ |
| `Mesh` | 21.7 s | 24.2 s | +11.8% | 824 MB | 830 MB | +0.8% | **minor** |
| `MeshCSG` | 366.9 s | 387.1 s | +5.5% | 5192 MB | 4935 MB | -4.9% | ~ |
| `MeshClosestPoint` | 19.5 s | 24.9 s | +28.0% | 776 MB | 782 MB | +0.8% | **MAJOR** |
| `MeshIngest` | 10.8 s | 12.8 s | +18.0% | 549 MB | 557 MB | +1.4% | ~ |
| `MeshReconcile` | 10.7 s | 14.1 s | +31.4% | 526 MB | 534 MB | +1.6% | **MAJOR** |
| `MeshTape` | 650.1 s | 683.7 s | +5.2% | 5452 MB | 5377 MB | -1.4% | ~ |
| `PointCloud` | 12.6 s | 17.8 s | +41.4% | 577 MB | 586 MB | +1.6% | **MAJOR** |
| `PointCloudBVHBuild` | 7.1 s | 10.9 s | +53.6% | 523 MB | 531 MB | +1.5% | **MAJOR** |
| `PointCloudHashGridBuild` | 6.5 s | 7.6 s | +16.9% | 523 MB | 530 MB | +1.4% | ~ |
| `QueryOrder` | 16.1 s | 23.2 s | +44.8% | 708 MB | 714 MB | +0.9% | **MAJOR** |
| `Ray` | 165.6 s | 179.3 s | +8.3% | 3029 MB | 3002 MB | -0.9% | ~ |
| `RayCSG` | 200.0 s | 210.4 s | +5.2% | 3368 MB | 3266 MB | -3.0% | ~ |
| `RayCrossing` | 388.3 s | 405.6 s | +4.4% | 3300 MB | 3259 MB | -1.2% | ~ |
| `RayParticles` | 422.2 s | 429.4 s | +1.7% | 4847 MB | 4711 MB | -2.8% | ~ |
| `RayShell` | 243.1 s | 253.3 s | +4.2% | 4603 MB | 4477 MB | -2.7% | ~ |
| `Regions` | 314.6 s | 324.5 s | +3.2% | 3363 MB | 3345 MB | -0.5% | ~ |
| `Render` | 237.5 s | 241.6 s | +1.7% | 4920 MB | 4745 MB | -3.6% | ~ |
| `ShapeFamilies` | - | 24.6 s | | - | 595 MB | | new |
| `Shapes` | 27.5 s | 24.5 s | -10.8% | 599 MB | 607 MB | +1.5% | ~ |
| `SphereCloudBVH` | 8.7 s | 9.5 s | +8.8% | 524 MB | 531 MB | +1.4% | ~ |
| `SphereCloudRays` | 157.1 s | 159.9 s | +1.7% | 3332 MB | 3221 MB | -3.3% | ~ |
| `SphereContacts` | 8.7 s | 8.9 s | +2.2% | 523 MB | 531 MB | +1.5% | ~ |
| `SurfaceParts` | 336.5 s | 346.9 s | +3.1% | 3494 MB | 3399 MB | -2.7% | ~ |
| `Tape` | 56.7 s | 60.4 s | +6.6% | 992 MB | 1080 MB | +8.9% | ~ |
| `TapeBVHUnion` | 40.6 s | 43.1 s | +6.3% | 932 MB | 1021 MB | +9.5% | ~ |
| **all, in both** | 4677.0 s | 4949.4 s | +5.8% | 5452 MB | 5377 MB | -1.4% | ~ |

Device kernels: 226 in both, 100 changed (76 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 6 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double, Fused::Node<Fused::Comb…` | 168 → 168 | 9328 → 10512 | 16/4 → 16/36 | **spills stack** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double, TriMeshSDF<double, 4ul,…` | 168 → 168 | 9360 → 10496 | 20/4 → 20/36 | **spills stack** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double> >, Vec3T<double> const,…` | 168 → 168 | 9328 → 10528 | 16/4 → 16/36 | **spills stack** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float, Fused::Node<Fused::Combin…` | 168 → 168 | 6672 → 7120 | 4/4 → 4/20 | **spills** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float, TriMeshSDF<float, 4ul, 4u…` | 168 → 168 | 6640 → 7088 | 4/4 → 4/20 | **spills** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float> >, Vec3T<float> const, Le…` | 168 → 168 | 6656 → 7088 | 4/4 → 4/20 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, BVHUnion<double, 4ul, …` | 168 → 168 | 7600 → 7632 | 20/4 → 20/20 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, BVHUnion<double, 4ul, …` | 168 → 168 | 7680 → 7760 | 48/152 → 76/200 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, SphereCloudBVH<double,…` | 168 → 168 | 7584 → 7632 | 20/4 → 20/20 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, SphereCloudBVH<double,…` | 168 → 168 | 7680 → 7776 | 48/152 → 76/200 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, BVHUnion<float, 4ul, Sph…` | 168 → 168 | 4624 → 4656 | 4/4 → 4/12 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, BVHUnion<float, 4ul, Sph…` | 168 → 168 | 4752 → 4752 | 44/164 → 48/164 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, SphereCloudBVH<float, 4u…` | 168 → 168 | 4624 → 4656 | 4/4 → 4/12 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, SphereCloudBVH<float, 4u…` | 168 → 168 | 4768 → 4784 | 44/164 → 48/164 | **spills** |
| `Domain` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)0>, Vec3T<double> const,…` | 168 → 168 | 4448 → 4512 | 228/272 → 256/300 | **spills** |
| `Domain` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)0>, Vec3T<float> const, A…` | 168 → 168 | 3104 → 3104 | 180/232 → 184/260 | **spills** |
| `DomainObject` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)0>, Vec3T<double> const,…` | 168 → 168 | 4192 → 4256 | 228/272 → 256/300 | **spills** |
| `DomainObject` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)0>, Vec3T<float> const, A…` | 168 → 168 | 2976 → 2976 | 180/232 → 184/260 | **spills** |
| `Fused` | `void parallelForKernel<128u, 384u, Apply<Distance<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 4448 → 4528 | 20/4 → 20/20 | **spills** |
| `Fused` | `void parallelForKernel<128u, 384u, Apply<Distance<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 3024 → 3072 | 4/4 → 4/12 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 17168 | 178/216 → 186/236 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11920 | 114/132 → 114/140 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 80 → 80 | 17328 → 17552 | 282/460 → 298/508 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 80 → 80 | 12320 → 12400 | 126/132 → 126/140 | **spills** |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 128 → 128 | 17152 → 17360 | 178/216 → 186/236 | **spills** |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 128 → 128 | 12064 → 12144 | 114/132 → 114/140 | **spills** |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> cons…` | 255 → 255 | 16512 → 16736 | 166/216 → 174/236 | **spills** |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const, …` | 40 → 40 | 12608 → 12672 | 230/368 → 238/404 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17280 | 174/220 → 190/236 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11888 → 11984 | 114/132 → 114/140 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19248 → 19568 | 1500/1412 → 1532/1476 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17280 | 174/220 → 190/236 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11920 → 11984 | 114/132 → 114/140 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19248 → 19568 | 1500/1412 → 1532/1476 | **spills** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 17168 | 178/216 → 186/236 | **spills** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11904 | 114/132 → 114/140 | **spills** |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16832 → 17072 | 164/148 → 172/168 | **spills** |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11712 → 11824 | 132/132 → 140/140 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17072 | 174/220 → 190/236 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 17184 | 178/216 → 186/236 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11920 → 11936 | 114/132 → 114/140 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11856 | 114/132 → 114/140 | **spills** |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 18928 → 18960 | 246/208 → 246/224 | **spills** |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 18720 → 18768 | 246/208 → 258/224 | **spills** |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13328 → 13360 | 178/160 → 178/168 | **spills** |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13152 → 13168 | 178/160 → 178/168 | **spills** |
| `RayShell` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TapeFeature::ExactO…` | 168 → 168 | 18912 → 19152 | 236/224 → 236/240 | **spills** |
| `RayShell` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TapeFeature::ExactOff…` | 168 → 168 | 13328 → 13440 | 172/176 → 172/184 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)1>, Vec3T<double> const,…` | 168 → 168 | 4160 → 4240 | 340/352 → 368/380 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)3>, Vec3T<double> const,…` | 168 → 168 | 4208 → 4272 | 228/272 → 256/300 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)1>, Vec3T<float> const, R…` | 168 → 168 | 2944 → 2944 | 236/268 → 240/296 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)3>, Vec3T<float> const, R…` | 168 → 168 | 2976 → 2976 | 180/232 → 184/260 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, false>, Vec3T<double> const, Reg…` | 168 → 168 | 16944 → 17168 | 178/192 → 186/212 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, false>, Vec3T<float> const, Regio…` | 168 → 168 | 11824 → 11904 | 114/120 → 114/128 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 19120 → 19440 | 1508/1376 → 1476/1412 | **spills** |
| `Shapes` | `void parallelForKernel<128u, 384u, Apply<Distance<double, PolygonPrismSDF<double> >, Vec3T…` | 36 → 42 | 48 → 48 | 0/0 → 0/0 | **registers** |
| `Shapes` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Fused::Node<Fused::CombineOp<tru…` | 132 → 138 | 1336 → 1328 | 0/0 → 0/0 | **registers** |
| `Shapes` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Fused::Node<Fused::CombineOp<true…` | 82 → 80 | 1024 → 992 | 0/0 → 0/0 | **registers** |
| `SphereCloudRays` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, SphereCloudBVH<doub…` | 168 → 168 | 17152 → 17168 | 126/104 → 158/128 | **spills** |
| `SphereCloudRays` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, SphereCloudBVH<float,…` | 168 → 168 | 12032 → 11904 | 102/88 → 102/96 | **spills** |

40 more changed kernels are in the records.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (20 cases): ClosestPrimitive/parts/closestPrimitive/float/hostdevice, Domain/particles/labelAt/float/hostdevice, Domain/particles/labels/double/hostdevice, DomainObject/partsWhole/labelAt/float/hostdevice, DomainObject/partsOwn/labelAt/float/hostdevice, LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice, MeshCSG/smooth-fused/firstHit-morton/double/hostdevice, RayCSG/scene-tape/firstHit-morton/float/hostdevice, RayCSG/scene-tape/firstHit-marched-random/double/hostdevice, RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice, RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice, RayParticles/armadillo/step-morton/double/hostdevice, Regions/particles/region/double/hostdevice, Regions/particles/regionByRegion/double/hostdevice, Render/csg/frame/float/hostdevice, Render/csg/frame-orthographic/float/hostdevice, Tape/csg/signedDistance/float/hostdevice, Tape/smooth/signedDistance/float/hostdevice, Tape/sphere/signedDistance/float/hostdevice, Tape/chain16/signedDistance/float/hostdevice

The probes differ between the sides (each side ran its own): Performance/ShapeFamilies/main.cpp

<!-- END receipt cuda -->
