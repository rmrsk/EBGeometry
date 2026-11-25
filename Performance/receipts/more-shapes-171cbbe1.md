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
head: 3c3b059f
head-fingerprint: 171cbbe1656fcd5eca9cde2307e905ee05233db6
cases: 358
rounds: 2
reps: 3
date: 2026-11-25
-->

## Host comparison: yellow

**Accepted:** Accept as borderline acceptable regression in order to support the new shapes.

`3c3b059f` against `344147cc` (origin/main), 2026-11-25.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 4 faster, 0 differ, 0 lost, 358 cases

- base: 344147cc (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3c3b059f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 330 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 180 ms | 178 ms | -1.0% | [-2.3%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 518 ms | +0.9% | [-0.2%, +2.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 348 ms | 347 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 187 ms | 187 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 573 ms | 575 ms | +0.3% | [-3.6%, +4.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 43.0 ms | 44.8 ms | +4.1% | [-6.0%, +14.8%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 78.5 ms | 77.9 ms | -0.7% | [-5.4%, +4.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 57.2 ms | 57.6 ms | +0.7% | [-2.8%, +4.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 52.4 ms | 52.4 ms | +0.0% | [-3.7%, +3.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.4 ms | 65.8 ms | +0.7% | [-3.5%, +5.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 56.0 ms | 56.3 ms | +0.6% | [-3.7%, +5.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.8 ms | 61.7 ms | -0.2% | [-4.2%, +4.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 74.8 ms | 74.1 ms | -0.9% | [-4.3%, +2.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 73.4 ms | 74.0 ms | +0.9% | [-2.6%, +4.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 80.0 ms | 79.6 ms | -0.4% | [-6.1%, +5.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 65.8 ms | 66.2 ms | +0.6% | [-3.4%, +4.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 104 ms | 103 ms | -0.8% | [-6.3%, +5.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 78.3 ms | 78.7 ms | +0.5% | [-2.8%, +4.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 112 ms | 113 ms | +0.6% | [-5.6%, +7.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 65.5 ms | 64.9 ms | -1.0% | [-5.3%, +3.6%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 97.5 ms | 97.9 ms | +0.4% | [-3.0%, +3.9%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 77.1 ms | 77.1 ms | +0.1% | [-3.3%, +3.5%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 103 ms | 104 ms | +0.2% | [-5.4%, +6.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.6 ms | 19.8 ms | +0.7% | [-2.4%, +3.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 43.0 ms | 41.7 ms | -3.1% | [-5.6%, -0.6%] | +0.0% | faster |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 74.8 ms | 74.4 ms | -0.5% | [-4.9%, +4.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.8 ms | 20.4 ms | -2.0% | [-5.8%, +1.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 49.4 ms | 44.4 ms | -10.2% | [-18.3%, -0.7%] | +0.0% | faster |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.9 ms | 84.2 ms | +1.6% | [-1.5%, +4.8%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 143 ms | 142 ms | -0.5% | [-3.3%, +2.4%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 77.0 ms | 77.3 ms | +0.3% | [-3.8%, +4.5%] | +0.0% | ~ | surface/leaf 1.84x |
| `ClosestSurface/union/closestSurface/float/host` | 149 ms | 150 ms | +0.4% | [-3.3%, +4.3%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 79.0 ms | 78.2 ms | -1.0% | [-4.4%, +2.6%] | +0.0% | ~ | surface/leaf 1.91x |
| `ClosestSurface/cloud/closestSurface/double/host` | 161 ms | 159 ms | -1.0% | [-5.4%, +3.6%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 82.0 ms | 81.8 ms | -0.2% | [-3.7%, +3.5%] | +0.0% | ~ | surface/leaf 1.95x |
| `ClosestSurface/union/closestSurface/double/host` | 157 ms | 156 ms | -0.4% | [-5.2%, +4.7%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 82.8 ms | 82.7 ms | -0.1% | [-4.2%, +4.0%] | +0.0% | ~ | surface/leaf 1.89x |
| `ConnectedComponents/shells/build-coarse/float/host` | 152 ms | 151 ms | -0.6% | [-5.2%, +4.2%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/float/host` | 674 ms | 677 ms | +0.5% | [-3.7%, +4.9%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-coarse/double/host` | 159 ms | 154 ms | -3.4% | [-6.9%, +0.4%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/double/host` | 691 ms | 677 ms | -2.1% | [-2.5%, -1.7%] | +0.0% | ~ |  |
| `Domain/particles/labels/float/host` | 13.7 ms | 13.7 ms | +0.1% | [-2.4%, +2.5%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.84 ms | 6.82 ms | -0.2% | [-2.1%, +1.7%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.33 ms | 1.32 ms | -0.1% | [-1.5%, +1.4%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.3 ms | 13.9 ms | -2.3% | [-4.2%, -0.4%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.17 ms | 7.07 ms | -1.4% | [-3.9%, +1.2%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.33 ms | 1.33 ms | +0.1% | [-2.0%, +2.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 40.3 ms | 40.2 ms | -0.3% | [-3.6%, +3.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.7 ms | 21.7 ms | -0.1% | [-1.9%, +1.8%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 575 µs | 570 µs | -0.8% | [-1.7%, +0.2%] | +0.0% | ~ | whole/own 70.43x |
| `DomainObject/partsOwn/labelAt/float/host` | 308 µs | 305 µs | -1.0% | [-2.9%, +0.8%] | +0.0% | ~ | whole/own 71.09x |
| `DomainObject/partsWhole/labels/double/host` | 40.3 ms | 40.5 ms | +0.7% | [-4.9%, +6.7%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.9 ms | 22.9 ms | -0.4% | [-3.5%, +2.9%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 586 µs | 591 µs | +0.7% | [-1.8%, +3.4%] | +0.0% | ~ | whole/own 68.66x |
| `DomainObject/partsOwn/labelAt/double/host` | 321 µs | 319 µs | -0.6% | [-1.8%, +0.6%] | +0.0% | ~ | whole/own 71.56x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/host` | 65.0 ms | 65.1 ms | +0.1% | [-4.5%, +4.9%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/host` | 60.8 ms | 60.7 ms | -0.1% | [-2.9%, +2.8%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/host` | 79.2 ms | 78.8 ms | -0.5% | [-5.0%, +4.3%] | +0.0% | ~ | grid/shuffled 0.83x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/host` | 69.7 ms | 69.8 ms | +0.1% | [-4.1%, +4.5%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/host` | 63.6 ms | 64.9 ms | +2.1% | [-1.0%, +5.1%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/host` | 88.4 ms | 88.4 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | grid/shuffled 0.79x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/host` | 191 ms | 192 ms | +0.5% | [-1.1%, +2.1%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/host` | 179 ms | 178 ms | -0.7% | [-1.1%, -0.2%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/host` | 264 ms | 263 ms | -0.3% | [-5.6%, +5.3%] | +0.0% | ~ | grid/shuffled 0.73x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/host` | 209 ms | 209 ms | +0.0% | [-5.7%, +6.0%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/host` | 192 ms | 196 ms | +2.1% | [-4.5%, +8.9%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/host` | 316 ms | 309 ms | -2.2% | [-14.4%, +11.9%] | +0.0% | ~ | grid/shuffled 0.68x |
| `Fused/csg/signedDistance/float/host` | 4.55 ms | 4.53 ms | -0.4% | [-1.3%, +0.5%] | - | ~ | tape/fused 2.85x |
| `Fused/smooth/signedDistance/float/host` | 12.5 ms | 12.4 ms | -0.7% | [-2.5%, +1.1%] | - | ~ | tape/fused 2.03x |
| `Fused/chain16/signedDistance/float/host` | 973 µs | 977 µs | +0.4% | [-1.6%, +2.6%] | - | ~ | tape/fused 16.83x (was 21.67x) |
| `Fused/drilled-mesh/signedDistance/float/host` | 47.4 ms | 47.1 ms | -0.6% | [-5.2%, +4.2%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 47.4 ms | 47.4 ms | +0.1% | [-4.4%, +4.8%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.68 ms | 4.62 ms | -1.3% | [-3.0%, +0.5%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/double/host` | 12.9 ms | 12.9 ms | +0.2% | [-1.3%, +1.8%] | - | ~ | tape/fused 2.08x |
| `Fused/chain16/signedDistance/double/host` | 3.92 ms | 3.92 ms | -0.2% | [-1.4%, +1.0%] | - | ~ | tape/fused 4.22x |
| `Fused/drilled-mesh/signedDistance/double/host` | 55.4 ms | 55.0 ms | -0.8% | [-7.4%, +6.6%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 55.4 ms | 55.3 ms | -0.1% | [-8.6%, +9.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 84.2 ms | 84.1 ms | -0.1% | [-3.3%, +3.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 234 ms | 234 ms | +0.0% | [-4.1%, +4.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 115 ms | 116 ms | +0.7% | [-0.8%, +2.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 272 ms | 268 ms | -1.2% | [-7.8%, +6.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 72.2 ms | 72.0 ms | -0.2% | [-3.3%, +3.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 218 ms | 220 ms | +0.9% | [-4.5%, +6.5%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 98.3 ms | 98.6 ms | +0.3% | [-2.7%, +3.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 31.3 ms | 30.2 ms | -3.6% | [-13.7%, +7.7%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 789 ms | 792 ms | +0.4% | [-7.1%, +8.4%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 120 ms | 120 ms | +0.2% | [-2.7%, +3.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 45.6 ms | 45.9 ms | +0.8% | [-10.8%, +13.6%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 863 ms | 901 ms | +4.5% | [-1.1%, +10.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 27.1 ms | 27.1 ms | -0.0% | [-6.6%, +7.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.5 ms | 15.8 ms | +1.7% | [-0.8%, +4.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 320 ms | 327 ms | +2.3% | [-0.5%, +5.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 231 ms | 231 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 91.0 ms | 90.8 ms | -0.3% | [-1.3%, +0.8%] | +0.0% | ~ | tape/fused 3.60x |
| `MeshCSG/smooth-fused/frame/float/host` | 81.6 ms | 81.4 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ | tape/fused 2.84x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.1 ms | 29.1 ms | -0.1% | [-1.0%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.1 ms | 17.2 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 359 ms | 363 ms | +0.9% | [-0.8%, +2.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 246 ms | 248 ms | +0.5% | [-1.0%, +1.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.1 ms | 99.3 ms | +2.2% | [+0.5%, +4.0%] | +0.0% | ~ | tape/fused 3.65x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.6 ms | 87.1 ms | -0.5% | [-1.1%, +0.0%] | +0.0% | ~ | tape/fused 2.84x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 229 ms | 229 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 232 ms | 232 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ | signedDistance/closestPoint 0.99x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 330 ms | 310 ms | -6.0% | [-11.4%, +0.2%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 238 ms | 238 ms | -0.0% | [-4.9%, +5.1%] | +0.0% | ~ | closestPoint/gradient 0.98x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 271 ms | 275 ms | +1.5% | [-5.1%, +8.6%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 277 ms | 274 ms | -1.1% | [-8.2%, +6.4%] | +0.0% | ~ | signedDistance/closestPoint 1.01x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 373 ms | 392 ms | +5.1% | [-0.8%, +11.7%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 275 ms | 274 ms | -0.4% | [-5.7%, +5.2%] | +0.0% | ~ | closestPoint/gradient 1.00x |
| `MeshIngest/armadillo/read-soup/float/host` | 54.5 ms | 54.7 ms | +0.5% | [-3.5%, +4.7%] | - | ~ |  |
| `MeshIngest/armadillo/clean/float/host` | 13.1 ms | 13.3 ms | +1.8% | [-4.4%, +8.5%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/float/host` | 99.5 ms | 101 ms | +1.1% | [-1.7%, +3.8%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/float/host` | 104 ms | 105 ms | +1.1% | [-2.6%, +4.9%] | - | ~ |  |
| `MeshIngest/armadillo/read-soup/double/host` | 55.1 ms | 55.2 ms | +0.3% | [-2.3%, +3.0%] | - | ~ |  |
| `MeshIngest/armadillo/clean/double/host` | 14.0 ms | 14.2 ms | +1.3% | [-5.7%, +8.8%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/double/host` | 109 ms | 110 ms | +0.7% | [-4.1%, +5.6%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/double/host` | 120 ms | 120 ms | -0.1% | [-4.6%, +4.7%] | - | ~ |  |
| `MeshReconcile/armadillo/reconcile/float/host` | 76.8 ms | 76.8 ms | +0.1% | [-6.5%, +7.2%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 42.7 ms | 43.3 ms | +1.3% | [-3.6%, +6.5%] | +0.0% | ~ | reconcile/scheduled 1.77x |
| `MeshReconcile/armadillo/reconcile/double/host` | 103 ms | 103 ms | +0.6% | [-10.6%, +13.2%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 58.6 ms | 58.8 ms | +0.3% | [-6.1%, +7.0%] | +0.0% | ~ | reconcile/scheduled 1.76x |
| `MeshTape/mesh/firstHit/float/host` | 51.1 ms | 52.6 ms | +2.9% | [-1.9%, +8.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 51.7 ms | 49.8 ms | -3.5% | [-7.9%, +1.1%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 20.0 ms | 20.4 ms | +2.4% | [+0.2%, +4.7%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 82.1 ms | 83.3 ms | +1.5% | [-2.7%, +5.9%] | +0.0% | ~ | mesh/tape 0.63x |
| `MeshTape/tape/firstCrossing/float/host` | 52.0 ms | 52.1 ms | +0.3% | [-5.1%, +5.9%] | +0.0% | ~ | mesh/tape 0.96x (was 0.99x) |
| `MeshTape/tape/frame/float/host` | 25.7 ms | 26.0 ms | +1.0% | [-4.6%, +6.7%] | +0.0% | ~ | mesh/tape 0.79x |
| `MeshTape/view/firstHit/float/host` | 86.0 ms | 87.1 ms | +1.3% | [-4.5%, +7.4%] | +0.0% | ~ | mesh/view 0.60x |
| `MeshTape/view/frame/float/host` | 26.8 ms | 27.4 ms | +2.0% | [-0.3%, +4.3%] | +0.0% | ~ | mesh/view 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 57.9 ms | 57.7 ms | -0.4% | [-6.1%, +5.6%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 56.2 ms | 56.2 ms | -0.1% | [-6.4%, +6.5%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 21.1 ms | 20.9 ms | -1.2% | [-4.6%, +2.2%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 90.7 ms | 90.4 ms | -0.4% | [-5.0%, +4.5%] | +0.0% | ~ | mesh/tape 0.64x |
| `MeshTape/tape/firstCrossing/double/host` | 55.5 ms | 56.1 ms | +1.0% | [-4.4%, +6.7%] | +0.0% | ~ | mesh/tape 1.00x |
| `MeshTape/tape/frame/double/host` | 27.1 ms | 27.4 ms | +1.4% | [-0.5%, +3.2%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 93.2 ms | 94.0 ms | +0.8% | [-3.8%, +5.7%] | +0.0% | ~ | mesh/view 0.61x |
| `MeshTape/view/frame/double/host` | 29.9 ms | 30.3 ms | +1.0% | [-1.4%, +3.5%] | +0.0% | ~ | mesh/view 0.69x |
| `PointCloudBVH/uniform/build/float/host` | 54.1 ms | 54.0 ms | -0.3% | [-2.8%, +2.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 150 ms | 150 ms | +0.4% | [-5.0%, +6.0%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 375 ms | 375 ms | -0.1% | [-4.9%, +5.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 83.3 ms | 83.3 ms | -0.0% | [-6.4%, +6.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.80 ms | 6.80 ms | +0.0% | [-2.3%, +2.5%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 57.9 ms | 56.7 ms | -2.1% | [-4.2%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 168 ms | 165 ms | -1.3% | [-9.1%, +7.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 400 ms | 400 ms | +0.2% | [-2.2%, +2.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 90.4 ms | 91.3 ms | +0.9% | [-5.4%, +7.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.20 ms | 7.05 ms | -2.1% | [-4.9%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.4 ms | 13.6 ms | +1.7% | [-0.2%, +3.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 119 ms | 119 ms | -0.0% | [-1.9%, +2.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 483 ms | 480 ms | -0.7% | [-3.2%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 145 ms | 147 ms | +1.5% | [-3.5%, +6.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 814 ms | 816 ms | +0.3% | [-1.7%, +2.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.1 ms | 15.0 ms | -0.5% | [-2.1%, +1.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 122 ms | 123 ms | +0.7% | [-0.7%, +2.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 499 ms | 500 ms | +0.2% | [-1.8%, +2.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 151 ms | 153 ms | +0.8% | [-6.5%, +8.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 818 ms | 833 ms | +1.8% | [-3.5%, +7.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 55.4 ms | 55.2 ms | -0.5% | [-3.0%, +2.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 135 ms | 136 ms | +1.0% | [-6.5%, +9.0%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 280 ms | 278 ms | -0.8% | [-3.9%, +2.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 152 ms | 152 ms | +0.3% | [-3.0%, +3.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 137 ms | 137 ms | -0.1% | [-2.8%, +2.7%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 58.5 ms | 58.2 ms | -0.6% | [-3.2%, +2.0%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 149 ms | 151 ms | +1.4% | [-6.0%, +9.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 304 ms | 305 ms | +0.2% | [-1.7%, +2.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 176 ms | 171 ms | -3.1% | [-11.1%, +5.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 158 ms | 161 ms | +1.5% | [-9.0%, +13.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.8 ms | 11.7 ms | -0.3% | [-3.2%, +2.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 117 ms | 116 ms | -0.6% | [-3.5%, +2.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 327 ms | 326 ms | -0.3% | [-4.3%, +3.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 159 ms | 158 ms | -0.1% | [-2.6%, +2.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 325 ms | 329 ms | +1.2% | [-0.1%, +2.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.8 ms | 13.8 ms | +0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 117 ms | 117 ms | +0.1% | [-1.5%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 360 ms | 354 ms | -1.6% | [-3.4%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 168 ms | 172 ms | +2.7% | [+0.7%, +4.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 321 ms | 323 ms | +0.7% | [-0.6%, +2.1%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.9 ms | 36.0 ms | +0.2% | [-2.3%, +2.8%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.5 ms | 39.2 ms | +1.6% | [-0.9%, +3.0%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 10.3 ms | 8.37 ms | -18.5% | [-42.8%, +35.8%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.31 ms | 9.42 ms | +1.2% | [-0.6%, +4.8%] | +0.0% | ~ |  |
| `QueryOrder/random/mortonOrder/float/host` | 61.9 ms | 61.8 ms | -0.1% | [-1.8%, +1.5%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | 231 ms | 229 ms | -0.8% | [-5.8%, +4.6%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | 162 ms | 163 ms | +0.1% | [-2.2%, +2.5%] | +0.0% | ~ | arrived/ordered 1.41x |
| `QueryOrder/random/mortonOrder/double/host` | 63.0 ms | 63.1 ms | +0.2% | [-1.6%, +2.1%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | 281 ms | 289 ms | +3.1% | [-0.9%, +7.5%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | 173 ms | 175 ms | +0.9% | [-1.5%, +3.4%] | +0.0% | ~ | arrived/ordered 1.66x |
| `Ray/mesh/firstHit/float/host` | 49.7 ms | 50.4 ms | +1.4% | [-3.7%, +6.7%] | +0.0% | ~ | hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled/float/host` | 46.7 ms | 47.2 ms | +1.2% | [-2.2%, +4.8%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 41.4 ms | 41.2 ms | -0.6% | [-3.6%, +2.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 85.2 ms | 82.2 ms | -3.6% | [-13.1%, +7.4%] | +0.0% | ~ | triangles/polygons 0.61x (was 0.58x) |
| `Ray/mesh-polygons/intersects/float/host` | 70.8 ms | 70.7 ms | -0.1% | [-10.9%, +12.0%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.23 ms | 5.30 ms | +1.4% | [-1.3%, +4.1%] | - | ~ | tape/fused 1.47x |
| `Ray/csg-tape/firstHit/float/host` | 7.75 ms | 7.81 ms | +0.7% | [-0.2%, +1.7%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 56.5 ms | 55.9 ms | -1.0% | [-5.7%, +3.8%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 54.3 ms | 54.2 ms | -0.1% | [-6.3%, +6.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 47.0 ms | 47.3 ms | +0.7% | [-4.0%, +5.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 96.7 ms | 96.6 ms | -0.1% | [-12.2%, +13.5%] | +0.0% | ~ | triangles/polygons 0.58x |
| `Ray/mesh-polygons/intersects/double/host` | 80.0 ms | 79.2 ms | -1.0% | [-11.7%, +11.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.53 ms | 5.65 ms | +2.1% | [+2.0%, +2.2%] | - | ~ | tape/fused 1.42x |
| `Ray/csg-tape/firstHit/double/host` | 7.89 ms | 8.01 ms | +1.6% | [+0.1%, +3.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.6 ms | 17.7 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | marched/traced 6.81x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 121 ms | 120 ms | -0.5% | [-2.2%, +1.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 22.1 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | marched/traced 6.62x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 146 ms | 146 ms | -0.0% | [-1.6%, +1.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.6 ms | 35.7 ms | +0.3% | [-1.2%, +1.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.2 ms | 19.1 ms | -0.8% | [-1.7%, +0.3%] | - | ~ | marched/traced 6.63x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 128 ms | 126 ms | -1.0% | [-2.7%, +0.8%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.7 ms | 23.3 ms | -1.5% | [-3.2%, +0.2%] | +0.0% | ~ | marched/traced 6.51x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 153 ms | 152 ms | -1.0% | [-3.8%, +2.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.4 ms | 38.1 ms | -0.7% | [-3.2%, +2.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 50.2 ms | 50.9 ms | +1.3% | [-3.8%, +6.8%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 50.4 ms | 52.1 ms | +3.4% | [-0.5%, +7.5%] | +0.0% | ~ | hit/crossing 0.98x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.68 ms | 7.68 ms | +0.1% | [-1.9%, +2.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.23 ms | 6.27 ms | +0.6% | [+0.6%, +0.7%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 184 ms | 185 ms | +0.7% | [-5.0%, +6.7%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 163 ms | 164 ms | +0.5% | [-4.8%, +6.2%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 70.6 ms | 70.3 ms | -0.3% | [-3.7%, +3.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 57.5 ms | 57.9 ms | +0.6% | [-2.9%, +4.3%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayCrossing/mesh/firstHit/double/host` | 55.6 ms | 55.3 ms | -0.5% | [-6.4%, +5.9%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 54.9 ms | 55.2 ms | +0.6% | [-4.1%, +5.4%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.03 ms | 8.21 ms | +2.2% | [-2.4%, +6.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.28 ms | 6.27 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | hit/crossing 1.31x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 211 ms | 209 ms | -1.1% | [-6.7%, +4.8%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 213 ms | 187 ms | -12.3% | [-26.2%, +6.3%] | +0.0% | ~ | hit/crossing 1.12x (was 0.99x) |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 82.7 ms | 82.0 ms | -0.9% | [-9.3%, +8.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 69.1 ms | 73.5 ms | +6.4% | [-9.8%, +24.2%] | +0.0% | ~ | hit/crossing 1.11x (was 1.20x) |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.4 ms | 11.3 ms | -0.8% | [-2.7%, +1.2%] | +0.0% | ~ | marched/traced 4.78x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 53.1 ms | 54.1 ms | +2.0% | [-3.1%, +7.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.16 ms | 8.27 ms | +1.3% | [-1.1%, +3.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.03 ms | 2.01 ms | -0.9% | [-4.0%, +2.2%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.96 ms | 6.80 ms | -2.4% | [-3.0%, -1.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.4 ms | 12.1 ms | -2.3% | [-8.4%, +4.3%] | +0.0% | ~ | marched/traced 5.35x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 64.2 ms | 64.6 ms | +0.5% | [-8.3%, +10.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.83 ms | 8.86 ms | +0.3% | [-4.0%, +4.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.08 ms | 2.07 ms | -0.5% | [-3.3%, +2.2%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.27 ms | 7.11 ms | -2.3% | [-3.3%, -1.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 242 ms | 241 ms | -0.6% | [-2.2%, +1.1%] | +0.0% | ~ | marched/traced 1.40x (was 1.50x) |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 364 ms | 337 ms | -7.3% | [-16.6%, +3.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 243 ms | 247 ms | +1.4% | [-1.4%, +4.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 267 ms | 258 ms | -3.7% | [-6.7%, -0.6%] | +0.0% | faster | marched/traced 1.52x (was 1.45x) |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 387 ms | 390 ms | +0.8% | [-7.2%, +9.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 257 ms | 261 ms | +1.5% | [-1.2%, +4.1%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.39 ms | 7.36 ms | -0.4% | [-1.8%, +1.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 13.1 ms | 12.6 ms | -4.2% | [-8.2%, +0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 268 ms | 268 ms | +0.1% | [-1.9%, +2.0%] | +0.0% | ~ | one-pass/each 0.05x (was 0.05x) |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.6 ms | +0.8% | [+0.0%, +1.5%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.8 ms | 30.9 ms | +0.1% | [-1.2%, +1.5%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.1 ms | 22.3 ms | +0.7% | [-0.5%, +1.8%] | +0.0% | ~ | regions/hit 1.38x |
| `Regions/particles/region/double/host` | 7.71 ms | 7.72 ms | +0.1% | [-2.0%, +2.2%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.9 ms | 12.9 ms | -0.5% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 287 ms | 285 ms | -0.9% | [-3.6%, +2.0%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.2 ms | 13.1 ms | -0.2% | [-2.4%, +2.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 33.0 ms | 32.8 ms | -0.7% | [-4.8%, +3.6%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 24.1 ms | 24.2 ms | +0.7% | [-1.5%, +2.9%] | +0.0% | ~ | regions/hit 1.35x |
| `Render/csg/frame/float/host` | 12.8 ms | 13.3 ms | +4.1% | [-2.5%, +11.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 12.7 ms | 13.2 ms | +3.6% | [-0.6%, +8.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.3 ms | 20.5 ms | +0.8% | [-3.2%, +5.0%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.3 ms | 14.1 ms | -1.8% | [-3.6%, -0.1%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.1 ms | 13.9 ms | -1.6% | [-3.6%, +0.6%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.1 ms | 21.7 ms | +2.8% | [+0.5%, +5.4%] | +0.0% | ~ |  |
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
| `ShapeFamilies/sampled-grid/signedDistance/float/host` | - | - | - | - | - | new |  |
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
| `ShapeFamilies/sampled-grid/signedDistance/double/host` | - | - | - | - | - | new |  |
| `ShapeFamilies/ray-forms/firstHit/double/host` | - | - | - | - | - | new |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 33.9 ms | 34.0 ms | +0.3% | [-1.5%, +2.1%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 5.11 ms | 5.08 ms | -0.5% | [-1.0%, -0.0%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.24 ms | 4.25 ms | +0.2% | [-0.6%, +1.0%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.7 ms | 17.7 ms | -0.4% | [-1.6%, +0.7%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.38 ms | 4.44 ms | +1.5% | [+0.9%, +2.0%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.23 ms | 4.20 ms | -0.8% | [-2.8%, +1.3%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.28 ms | 2.24 ms | -1.8% | [-3.1%, +1.8%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.47 ms | 1.48 ms | +0.6% | [+0.1%, +1.0%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.09 ms | 2.10 ms | +0.3% | [-0.5%, +1.1%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.9 ms | 28.0 ms | +0.1% | [-1.7%, +1.9%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 47.2 ms | 47.5 ms | +0.5% | [-0.2%, +1.3%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.50 ms | 5.45 ms | -1.0% | [-1.4%, -0.6%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.60 ms | 4.54 ms | -1.2% | [-2.3%, -0.1%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 23.0 ms | 22.8 ms | -0.5% | [-1.0%, +0.0%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.59 ms | 4.66 ms | +1.4% | [-0.9%, +3.7%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.33 ms | 5.31 ms | -0.4% | [-2.2%, +1.5%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.57 ms | 2.53 ms | -1.2% | [-2.3%, -0.1%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.82 ms | 1.81 ms | -0.6% | [-0.9%, -0.4%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.11 ms | 2.11 ms | -0.3% | [-1.5%, +0.9%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 30.7 ms | 30.9 ms | +0.5% | [-0.5%, +1.4%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 58.6 ms | 58.8 ms | +0.3% | [-0.6%, +1.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 37.9 ms | 37.9 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 97.5 ms | 97.3 ms | -0.2% | [-2.2%, +1.8%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 66.2 ms | 66.2 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 43.7 ms | 44.4 ms | +1.5% | [-0.1%, +3.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 118 ms | 119 ms | +1.1% | [+0.4%, +1.9%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 78.2 ms | 78.9 ms | +0.8% | [+0.4%, +1.3%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 318 ms | 316 ms | -0.6% | [-1.8%, +0.5%] | +0.0% | ~ | tape/fused 0.97x (was 1.00x) |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 318 ms | 306 ms | -3.8% | [-7.7%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 92.0 ms | 91.8 ms | -0.2% | [-9.9%, +10.5%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 366 ms | 367 ms | +0.4% | [-7.2%, +8.5%] | +0.0% | ~ | tape/fused 1.07x (was 1.02x) |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 374 ms | 393 ms | +5.0% | [-12.5%, +25.1%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 170 ms | 172 ms | +0.7% | [-4.5%, +6.3%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 202 ms | 201 ms | -0.3% | [-9.1%, +9.4%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 145 ms | 142 ms | -1.9% | [-3.7%, +0.0%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 90.5 ms | 90.8 ms | +0.3% | [-1.5%, +2.2%] | +0.0% | ~ | surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 133 ms | 134 ms | +0.5% | [-3.4%, +4.6%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 74.8 ms | 75.4 ms | +0.8% | [-0.0%, +1.7%] | +0.0% | ~ | surface/leaf 1.77x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 63.7 ms | 64.5 ms | +1.4% | [-0.0%, +2.8%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 36.9 ms | 36.1 ms | -2.1% | [-4.1%, -0.1%] | +0.0% | ~ | surface/hit 1.79x (was 1.73x) |
| `SurfaceParts/boxes/closestSurface/double/host` | 149 ms | 147 ms | -0.9% | [-1.9%, +0.3%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 91.6 ms | 91.9 ms | +0.3% | [-1.5%, +2.2%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 150 ms | 150 ms | -0.2% | [-5.4%, +5.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 84.2 ms | 84.1 ms | -0.1% | [-5.6%, +5.8%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 71.3 ms | 72.1 ms | +1.1% | [-4.0%, +6.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 40.9 ms | 41.0 ms | +0.1% | [-5.1%, +5.6%] | +0.0% | ~ | surface/hit 1.76x |
| `Tape/csg/signedDistance/float/host` | 13.1 ms | 12.9 ms | -1.4% | [-1.9%, -0.8%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.59 ms | 4.59 ms | +0.1% | [-0.9%, +1.0%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.1 ms | 25.2 ms | +0.5% | [-2.0%, +3.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.2 ms | 40.5 ms | +0.7% | [-0.3%, +1.8%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.79 ms | 1.78 ms | -0.6% | [-2.9%, +1.8%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 140 µs | 144 µs | +2.8% | [-1.8%, +7.8%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.6 ms | 30.5 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.70 ms | 5.76 ms | +0.9% | [-0.2%, +2.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 21.1 ms | 16.4 ms | -22.0% | [-35.5%, -1.4%] | +0.0% | faster |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.69 ms | 4.73 ms | +0.8% | [-0.9%, +2.4%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.4 ms | 13.3 ms | -1.1% | [-2.2%, +0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.74 ms | 4.61 ms | -2.8% | [-4.1%, -1.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.8 ms | 26.9 ms | +0.4% | [-0.6%, +1.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 43.2 ms | 42.8 ms | -1.0% | [-2.0%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.79 ms | 1.77 ms | -0.9% | [-3.2%, +1.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 281 µs | 284 µs | +1.4% | [-8.4%, +7.9%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.8 ms | 30.2 ms | -2.1% | [-3.5%, -0.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.89 ms | 7.94 ms | +0.7% | [-0.2%, +1.5%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.6 ms | 16.5 ms | -0.6% | [-1.5%, +0.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.81 ms | 7.88 ms | +0.8% | [+0.7%, +1.0%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 42.6 ms | 42.6 ms | +0.0% | [-1.7%, +1.8%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 201 ms | 218 ms | +8.1% | [+7.5%, +8.7%] | +0.0% | minor |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 51.0 ms | 43.1 ms | -15.3% | [-26.8%, +0.3%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 219 ms | 214 ms | -2.2% | [-2.6%, -1.8%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 765.0 s → 807.6 s (+5.6%), the largest peak 1417 MB → 1628 MB; 6 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.8 s | 9.7 s | -1.1% | 563 MB | 604 MB | +7.3% | ~ |
| `BVHRefit` | 4.3 s | 4.4 s | +0.4% | 503 MB | 514 MB | +2.1% | ~ |
| `BVHUnion` | 19.4 s | 19.6 s | +0.8% | 956 MB | 941 MB | -1.5% | ~ |
| `BVHUnionMixed` | 7.7 s | 7.8 s | +0.7% | 559 MB | 590 MB | +5.5% | ~ |
| `ClosestPrimitive` | 30.6 s | 32.8 s | +7.0% | 1348 MB | 1330 MB | -1.3% | ~ |
| `ClosestSurface` | 28.2 s | 30.6 s | +8.2% | 1318 MB | 1331 MB | +1.0% | ~ |
| `ConnectedComponents` | 15.4 s | 16.7 s | +8.4% | 854 MB | 881 MB | +3.1% | ~ |
| `Domain` | 18.1 s | 19.5 s | +7.5% | 860 MB | 1032 MB | +19.9% | **minor** |
| `DomainObject` | 16.0 s | 17.3 s | +8.2% | 860 MB | 919 MB | +6.8% | ~ |
| `EBGrid` | 14.6 s | 14.7 s | +0.4% | 784 MB | 791 MB | +1.0% | ~ |
| `Fused` | 24.8 s | 26.3 s | +6.1% | 1236 MB | 1312 MB | +6.1% | ~ |
| `LaunchTuning` | 37.5 s | 39.8 s | +6.0% | 1328 MB | 1336 MB | +0.5% | ~ |
| `Mesh` | 16.1 s | 15.9 s | -0.7% | 791 MB | 834 MB | +5.4% | ~ |
| `MeshCSG` | 40.1 s | 42.8 s | +6.8% | 1306 MB | 1330 MB | +1.8% | ~ |
| `MeshClosestPoint` | 14.3 s | 14.5 s | +1.2% | 787 MB | 833 MB | +5.8% | ~ |
| `MeshIngest` | 7.0 s | 7.1 s | +1.3% | 569 MB | 595 MB | +4.6% | ~ |
| `MeshReconcile` | 7.0 s | 6.9 s | -1.5% | 569 MB | 612 MB | +7.5% | ~ |
| `MeshTape` | 41.9 s | 45.2 s | +8.1% | 1261 MB | 1358 MB | +7.7% | ~ |
| `PointCloud` | 7.5 s | 7.5 s | -0.3% | 562 MB | 593 MB | +5.5% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.9 s | +3.1% | 477 MB | 478 MB | +0.3% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | +0.6% | 457 MB | 453 MB | -0.9% | ~ |
| `QueryOrder` | 11.3 s | 11.7 s | +3.8% | 708 MB | 722 MB | +2.0% | ~ |
| `Ray` | 36.7 s | 38.7 s | +5.4% | 1342 MB | 1340 MB | -0.2% | ~ |
| `RayCSG` | 32.2 s | 34.4 s | +6.9% | 1204 MB | 1271 MB | +5.6% | ~ |
| `RayCrossing` | 45.8 s | 49.1 s | +7.3% | 1360 MB | 1478 MB | +8.6% | ~ |
| `RayParticles` | 49.5 s | 52.1 s | +5.3% | 1334 MB | 1498 MB | +12.3% | **minor** |
| `RayShell` | 37.6 s | 40.4 s | +7.3% | 1312 MB | 1329 MB | +1.3% | ~ |
| `Regions` | 29.8 s | 31.5 s | +5.9% | 1116 MB | 1285 MB | +15.2% | **minor** |
| `Render` | 35.9 s | 38.5 s | +7.0% | 1300 MB | 1329 MB | +2.2% | ~ |
| `ShapeFamilies` | - | 9.9 s | | - | 605 MB | | new |
| `Shapes` | 8.8 s | 8.9 s | +1.3% | 559 MB | 610 MB | +9.2% | ~ |
| `SphereCloudBVH` | 4.8 s | 5.0 s | +4.3% | 518 MB | 531 MB | +2.5% | ~ |
| `SphereCloudRays` | 28.4 s | 30.1 s | +5.9% | 1078 MB | 1229 MB | +14.0% | **minor** |
| `SphereContacts` | 4.3 s | 4.3 s | +1.8% | 490 MB | 497 MB | +1.3% | ~ |
| `SurfaceParts` | 40.8 s | 43.3 s | +6.1% | 1417 MB | 1628 MB | +14.9% | **minor** |
| `Tape` | 16.8 s | 17.6 s | +4.9% | 838 MB | 963 MB | +14.9% | **minor** |
| `TapeBVHUnion` | 14.7 s | 15.7 s | +6.8% | 839 MB | 854 MB | +1.8% | ~ |
| **all, in both** | 765.0 s | 807.6 s | +5.6% | 1417 MB | 1628 MB | +14.9% | **minor** |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (7 cases): MeshTape/tape/firstCrossing/double/host, MeshTape/tape/frame/double/host, PointCloudBVHBuild/uniform/rebuild/double/host, PointCloudHashGridBuild/uniform/rebuild/double/host, Shapes/cut-sphere/signedDistance/float/host, Tape/sphere-handwritten/signedDistance/double/host, TapeBVHUnion/maps64-as-written/signedDistance/float/host

The probes differ between the sides (each side ran its own): Performance/ShapeFamilies/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 344147cc
base-fingerprint: 1e0f461958307254a04eb0a9ea2890c216b99160
head: 3c3b059f
head-fingerprint: 171cbbe1656fcd5eca9cde2307e905ee05233db6
cases: 442
rounds: 2
reps: 3
date: 2026-11-25
-->

## Device comparison (CUDA): red

**Accepted:** Accept as borderline acceptable regression in order to support the new shapes.

`3c3b059f` against `344147cc` (origin/main), 2026-11-25.

**VERDICT: MAJOR regression in 2 cases; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 2 major, 12 minor, 15 faster, 0 differ, 0 lost, 442 cases

- base: 344147cc (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3c3b059f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.61 ms | +0.2% | [-0.7%, +1.0%] | - | ~ | device 23.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.39 ms | 3.40 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 22.0x faster than the host (was 18.9x) |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.05 ms | -0.2% | [-0.7%, +0.4%] | - | ~ | device 19.8x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.30 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 16.8x faster than the host (was 18.4x) |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.24 ms | 6.29 ms | +0.9% | [-0.3%, +2.1%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 12.0 ms | +1.2% | [-0.7%, +3.1%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.4 ms | 33.2 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.1 ms | 57.1 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 21.1 ms | 20.9 ms | -0.9% | [-1.9%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.0 ms | 47.9 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.8 ms | 62.0 ms | +0.3% | [-3.0%, +3.6%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 106 ms | -4.0% | [-5.0%, -3.0%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.63 ms | 8.84 ms | +2.5% | [-38.4%, +75.6%] | +0.0% | ~ | device 5.3x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 10.9 ms | 10.3 ms | -4.9% | [-9.1%, -0.4%] | +0.0% | faster | device 8.9x faster than the host (was 8.3x) |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 386 µs | 388 µs | +0.6% | [+0.1%, +1.2%] | - | ~ | device 15.3x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 844 µs | 844 µs | -0.1% | [-0.1%, +0.0%] | - | ~ | device 10.3x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.54 ms | +0.4% | [-0.0%, +0.9%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.67 ms | 4.68 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 585 µs | 583 µs | -0.3% | [-0.7%, -0.0%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.37 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 8.4x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.76 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.00 ms | 7.01 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 907 µs | 903 µs | -0.4% | [-0.6%, -0.1%] | - | ~ | device 14.5x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.24 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.51 ms | 9.47 ms | -0.4% | [-0.8%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 849 µs | 850 µs | +0.2% | [-0.0%, +0.3%] | - | ~ | device 13.6x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.07 ms | 2.06 ms | -0.4% | [-0.4%, -0.3%] | - | ~ | device 8.0x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.11 ms | 9.07 ms | -0.4% | [-0.4%, -0.3%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.78 ms | 9.81 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 308 µs | 318 µs | +3.3% | [+2.9%, +4.1%] | - | minor | device 12.0x faster than the host (was 12.4x) |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.35 ms | 1.43 ms | +5.6% | [+5.4%, +5.9%] | - | minor | device 5.8x faster than the host (was 6.4x) |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 59.2 ms | +0.5% | [+0.4%, +0.6%] | - | ~ | device 3.8x faster than the host (was 3.6x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.43 ms | 5.43 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.7x faster than the host (was 1.7x) |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 524 ms | 521 ms | -0.7% | [-1.6%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.3 ms | 20.4 ms | +0.6% | [+0.4%, +0.7%] | - | ~ | device 4.2x faster than the host |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.39 ms | 8.46 ms | +0.9% | [+0.7%, +1.0%] | - | ~ | device 4.8x faster than the host; surface/leaf 2.41x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.5 ms | -0.5% | [-0.7%, -0.3%] | - | ~ | device 9.1x faster than the host (was 8.4x) |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.18 ms | 6.23 ms | +0.9% | [+0.7%, +1.1%] | - | ~ | device 10.8x faster than the host; surface/leaf 2.49x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 146 ms | 146 ms | -0.4% | [-1.9%, +1.2%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.0 ms | 60.0 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.43x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 71.5 ms | 72.0 ms | +0.7% | [+0.4%, +1.0%] | - | ~ | device 2.4x faster than the host (was 1.8x) |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.3x faster than the host; surface/leaf 2.57x |
| `Domain/particles/labels/float/hostdevice` | 4.06 ms | 3.83 ms | -5.6% | [-6.0%, -5.2%] | - | faster | device 3.9x faster than the host (was 3.6x) |
| `Domain/particles/labelAt/float/hostdevice` | 2.76 ms | 2.79 ms | +1.0% | [+0.5%, +1.5%] | - | ~ | device 2.7x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 115 µs | 115 µs | +0.5% | [-1.7%, +2.9%] | - | ~ | device 13.0x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.04 ms | 7.66 ms | +8.9% | [+8.4%, +10.0%] | - | minor | device 1.9x faster than the host (was 2.2x) |
| `Domain/particles/labelAt/double/hostdevice` | 3.51 ms | 3.49 ms | -0.5% | [-1.1%, +0.1%] | - | ~ | device 2.2x faster than the host |
| `Domain/particles/surfaceGroups/double/hostdevice` | 114 µs | 114 µs | -0.3% | [-2.1%, +1.5%] | - | ~ | device 13.3x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 13.1 ms | 11.6 ms | -11.9% | [-12.3%, -11.5%] | - | faster | device 4.0x faster than the host (was 3.6x) |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.88 ms | 9.05 ms | +1.9% | [+1.5%, +2.3%] | - | ~ | device 2.7x faster than the host (was 2.8x) |
| `DomainObject/partsOwn/labels/float/hostdevice` | 267 µs | 258 µs | -3.4% | [-3.8%, -3.0%] | - | faster | device 2.6x faster than the host (was 2.5x); whole/own 44.81x (was 49.12x) |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 159 µs | 170 µs | +6.9% | [+6.4%, +7.8%] | - | minor | device 2.2x faster than the host (was 2.9x); whole/own 53.19x (was 55.77x) |
| `DomainObject/partsWhole/labels/double/hostdevice` | 19.5 ms | 19.8 ms | +1.6% | [+1.4%, +1.8%] | - | ~ | device 2.5x faster than the host |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 11.4 ms | 11.4 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `DomainObject/partsOwn/labels/double/hostdevice` | 392 µs | 390 µs | -0.6% | [-1.5%, +0.3%] | - | ~ | device 1.8x faster than the host; whole/own 50.66x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 236 µs | 236 µs | +0.0% | [-0.4%, +0.4%] | - | ~ | device 1.5x faster than the host (was 1.6x); whole/own 48.16x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/hostdevice` | 10.3 ms | 10.4 ms | +0.8% | [+0.4%, +1.2%] | +0.0% | ~ | device 10.2x faster than the host (was 11.5x) |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/hostdevice` | 10.4 ms | 10.4 ms | +0.1% | [-1.0%, +1.2%] | +0.0% | ~ | device 9.6x faster than the host; grid/morton 1.00x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/hostdevice` | 17.4 ms | 17.5 ms | +0.9% | [-0.0%, +1.9%] | +0.0% | ~ | device 7.8x faster than the host (was 9.3x); grid/shuffled 0.59x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/hostdevice` | 221 ms | 223 ms | +1.0% | [-2.2%, +4.1%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/hostdevice` | 164 ms | 164 ms | -0.0% | [-1.9%, +1.9%] | +0.0% | ~ | device 0.6x faster than the host; grid/morton 1.36x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/hostdevice` | 284 ms | 293 ms | +3.0% | [+2.5%, +3.5%] | +0.0% | ~ | device 0.4x faster than the host (was 0.5x); grid/shuffled 0.76x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/hostdevice` | 27.2 ms | 27.0 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ | device 4.5x faster than the host (was 4.0x) |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/hostdevice` | 29.2 ms | 29.1 ms | -0.3% | [-1.2%, +0.6%] | +0.0% | ~ | device 3.5x faster than the host; grid/morton 0.93x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/hostdevice` | 60.4 ms | 60.5 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ | device 2.6x faster than the host (was 2.7x); grid/shuffled 0.45x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/hostdevice` | 211 ms | 203 ms | -3.8% | [-7.3%, -0.2%] | +0.0% | faster | device 0.7x faster than the host (was 0.5x) |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/hostdevice` | 158 ms | 158 ms | +0.2% | [-1.7%, +3.8%] | +0.0% | ~ | device 0.9x faster than the host (was 0.9x); grid/morton 1.28x (was 1.33x) |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/hostdevice` | 249 ms | 249 ms | +0.0% | [-2.1%, +2.2%] | +0.0% | ~ | device 0.7x faster than the host; grid/shuffled 0.81x (was 0.85x) |
| `Fused/csg/signedDistance/float/hostdevice` | 48.0 µs | 48.0 µs | +0.0% | [-0.6%, +0.7%] | - | ~ | device 24.3x faster than the host; tape/fused 4.72x (was 4.55x) |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.8 µs | 49.9 µs | +0.2% | [-0.0%, +0.3%] | - | ~ | device 45.5x faster than the host; tape/fused 7.76x (was 7.21x) |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.7 µs | 47.9 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 9.0x faster than the host (was 9.6x); tape/fused 10.67x (was 9.99x) |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.4 ms | +0.6% | [-0.1%, +1.4%] | - | ~ | device 6.9x faster than the host; tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.2 ms | 31.4 ms | +0.5% | [+0.4%, +0.6%] | - | ~ | device 5.0x faster than the host (was 4.4x) |
| `Fused/csg/signedDistance/double/hostdevice` | 809 µs | 810 µs | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.5x faster than the host (was 1.8x); tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 283 ms | 285 ms | +1.0% | [-0.7%, +2.7%] | - | ~ | device 0.4x faster than the host; tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 282 ms | 284 ms | +0.6% | [-0.8%, +2.1%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.23 ms | 8.28 ms | +0.6% | [-0.1%, +1.2%] | +0.0% | ~ | device 11.1x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.35 ms | 8.31 ms | -0.4% | [-1.6%, +0.8%] | +0.0% | ~ | device 11.0x faster than the host (was 12.1x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.22 ms | 8.23 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ | device 11.1x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.28 ms | 8.27 ms | -0.1% | [-1.7%, +1.5%] | +0.0% | ~ | device 11.1x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 704 µs | 690 µs | -2.1% | [-2.4%, -1.7%] | +0.0% | ~ | device 3.8x faster than the host (was 3.7x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.79 ms | 1.77 ms | -1.1% | [-1.1%, -1.0%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.02 ms | 1.03 ms | +0.4% | [+0.0%, +0.8%] | +0.0% | ~ | device 2.5x faster than the host (was 2.6x); default/this 0.67x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 798 µs | 818 µs | +2.5% | [+2.1%, +2.9%] | +0.0% | ~ | device 3.3x faster than the host; default/this 0.84x (was 0.88x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 753 µs | 747 µs | -0.7% | [-1.0%, -0.4%] | +0.0% | ~ | device 12.5x faster than the host (was 15.1x) |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 821 µs | 828 µs | +0.8% | [+0.3%, +1.3%] | +0.0% | ~ | device 10.9x faster than the host (was 11.2x); default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 743 µs | 742 µs | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 12.2x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 733 µs | 729 µs | -0.6% | [-1.8%, +0.7%] | +0.0% | ~ | device 12.3x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 162 ms | 162 ms | -0.3% | [-1.9%, +1.4%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 159 ms | 160 ms | +0.5% | [-2.9%, +4.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 165 ms | 159 ms | -3.9% | [-5.9%, -1.9%] | +0.0% | faster | device 0.6x faster than the host; default/this 1.02x (was 0.98x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 162 ms | 161 ms | -0.5% | [-2.2%, +1.1%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.40 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.45 ms | 2.81 ms | +93.0% | [+92.4%, +93.3%] | +0.0% | **MAJOR** | device 1.3x faster than the host (was 2.3x); default/this 0.50x (was 0.95x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.93 ms | 1.92 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | device 1.6x faster than the host (was 1.4x); default/this 0.73x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.56 ms | +0.5% | [+0.2%, +0.9%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.22 ms | 7.26 ms | +0.7% | [+0.1%, +1.3%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.30 ms | 7.31 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ | device 1.5x faster than the host (was 1.3x); default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.25 ms | 7.25 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.32 ms | 7.28 ms | -0.7% | [-0.8%, -0.5%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.47 ms | 8.38 ms | -1.1% | [-2.2%, +0.0%] | - | ~ | device 11.6x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.8 ms | 16.6 ms | -0.7% | [-2.4%, +1.1%] | - | ~ | device 7.9x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 168 ms | 167 ms | -0.5% | [-3.0%, +2.1%] | - | ~ | device 0.6x faster than the host (was 0.7x) |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 282 ms | 277 ms | -1.6% | [-2.6%, -0.7%] | - | ~ | device 0.5x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.6 ms | 24.7 ms | +0.4% | [+0.1%, +0.8%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.0 ms | 58.0 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 3.3x faster than the host (was 2.5x) |
| `MeshSDF/query-morton/double/hostdevice` | 159 ms | 157 ms | -1.1% | [-1.8%, -0.3%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `MeshSDF/query-random/double/hostdevice` | 249 ms | 248 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.06 ms | 2.06 ms | -0.1% | [-1.0%, +0.8%] | - | ~ | device 5.2x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.45 ms | 3.45 ms | +0.1% | [-0.7%, +0.8%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.14 ms | 6.25 ms | +1.7% | [+1.5%, +1.8%] | - | ~ | device 4.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 64.8 ms | 65.1 ms | +0.4% | [-0.8%, +1.7%] | - | ~ | device 3.3x faster than the host (was 3.5x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 95.3 ms | 96.1 ms | +0.8% | [+0.4%, +1.1%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 242 ms | 233 ms | -3.9% | [-4.5%, -3.2%] | - | faster | device 4.0x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.4 ms | 24.8 ms | +1.4% | [+0.7%, +2.0%] | - | ~ | device 2.3x faster than the host (was 2.1x); tape/fused 2.63x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 26.6 ms | -1.4% | [-2.2%, -0.6%] | - | ~ | device 2.5x faster than the host; tape/fused 3.62x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.0 ms | 65.5 ms | +0.6% | [+0.4%, +0.9%] | - | ~ | device 4.1x faster than the host (was 4.4x); tape/fused 3.55x (was 3.72x) |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.2% | [-7.4%, +8.1%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 39.1 ms | 39.1 ms | +0.1% | [-3.3%, +3.5%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 42.0 ms | 41.6 ms | -1.0% | [-2.9%, +1.9%] | - | ~ | device 1.0x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 533 ms | 539 ms | +1.2% | [-3.2%, +5.8%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 850 ms | 853 ms | +0.3% | [-0.2%, +0.9%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.02 s | 1.93 s | -4.4% | [-5.8%, -2.9%] | - | faster | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 206 ms | 204 ms | -0.8% | [-5.0%, +5.7%] | - | ~ | device 0.3x faster than the host; tape/fused 2.64x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 303 ms | 308 ms | +1.7% | [+0.9%, +2.5%] | - | ~ | device 0.2x faster than the host; tape/fused 2.77x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 539 ms | 548 ms | +1.7% | [+0.7%, +2.8%] | - | ~ | device 0.5x faster than the host (was 0.6x); tape/fused 3.52x (was 3.75x) |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 11.9 ms | 12.0 ms | +0.3% | [-0.6%, +1.1%] | - | ~ | device 10.7x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 58.4 ms | 58.5 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 2.7x faster than the host (was 2.9x) |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 260 ms | 257 ms | -1.3% | [-2.3%, -0.4%] | - | ~ | device 0.6x faster than the host (was 0.5x) |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 241 ms | 243 ms | +0.6% | [-1.5%, +2.8%] | - | ~ | device 0.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ | device 17.3x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.32 ms | 5.32 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 11.6x faster than the host |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.32 ms | 1.32 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 16.5x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.32 ms | 1.32 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 16.2x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.13 ms | 2.14 ms | +0.6% | [+0.4%, +0.7%] | - | ~ | device 8.1x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.52 ms | 3.51 ms | -0.3% | [-0.3%, -0.3%] | - | ~ | device 10.5x faster than the host; mesh/tape 0.38x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.77 ms | 2.77 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 9.7x faster than the host; mesh/tape 0.48x |
| `MeshTape/tape/frame/float/hostdevice` | 4.40 ms | 4.50 ms | +2.3% | [+1.8%, +2.7%] | - | ~ | device 6.2x faster than the host; mesh/tape 0.48x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.73 ms | 3.76 ms | +0.6% | [+0.5%, +0.7%] | - | ~ | device 13.7x faster than the host (was 9.9x); mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.62 ms | 5.69 ms | +1.2% | [+0.9%, +1.5%] | - | ~ | device 4.9x faster than the host (was 4.4x); mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 30.1 ms | 30.1 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 30.3 ms | 30.3 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 25.8 ms | 25.7 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 43.3 ms | 43.5 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 0.9x faster than the host (was 0.9x); mesh/tape 0.69x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 30.4 ms | 30.6 ms | +0.7% | [+0.3%, +1.1%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 33.0 ms | 32.9 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.78x |
| `MeshTape/view/firstHit/double/hostdevice` | 43.5 ms | 43.2 ms | -0.8% | [-1.2%, -0.4%] | - | ~ | device 0.9x faster than the host (was 0.9x); mesh/view 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 36.4 ms | 35.9 ms | -1.4% | [-2.8%, +0.1%] | - | ~ | device 0.7x faster than the host (was 0.7x); mesh/view 0.71x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 680 µs | 680 µs | +0.0% | [-0.5%, +0.5%] | - | ~ | device 11.3x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.83 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 5.8x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 997 µs | 994 µs | -0.3% | [-0.7%, +0.1%] | - | ~ | device 9.5x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.60 ms | 4.60 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 5.6x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.96 ms | 2.97 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.06 ms | 7.08 ms | +0.3% | [+0.0%, +0.7%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.78 ms | 7.83 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.4 ms | +0.2% | [-0.0%, +0.3%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.43 ms | 1.46 ms | +2.0% | [+0.1%, +4.0%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.88 ms | 3.88 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 529 µs | 523 µs | -1.1% | [-1.6%, -0.6%] | - | ~ | device 16.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.56 ms | 4.56 ms | -0.1% | [-1.1%, +0.9%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.17 ms | 4.26 ms | +2.0% | [+0.2%, +3.9%] | - | ~ | device 5.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.07 ms | 8.05 ms | -0.2% | [-1.2%, +0.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.8 ms | -0.5% | [-0.8%, -0.1%] | - | ~ | device 0.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.00 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.6 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.4% | [-1.5%, +0.7%] | - | ~ | device 1.6x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.58 ms | -0.1% | [-1.6%, +1.4%] | - | ~ | device 10.5x faster than the host (was 13.0x) |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.61 ms | 4.60 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 6.8x faster than the host (was 5.7x) |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 832 µs | 832 µs | +0.0% | [-0.4%, +0.4%] | - | ~ | device 10.6x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.87 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 7.0x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.14 ms | 4.12 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 5.6x faster than the host (was 6.3x) |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.6 ms | 16.6 ms | +0.1% | [-1.3%, +1.5%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 21.1 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.94 ms | 8.94 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.6 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.2 ms | 20.2 ms | -0.2% | [-0.8%, +0.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.73 ms | 2.74 ms | +0.6% | [-0.2%, +1.4%] | - | ~ | device 4.7x faster than the host (was 4.8x) |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.74 ms | 6.72 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 2.4x faster than the host (was 2.5x) |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.37 ms | -0.6% | [-0.8%, -0.5%] | - | ~ | device 6.8x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.71 ms | 2.72 ms | +0.7% | [+0.4%, +1.0%] | - | ~ | device 8.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.43 ms | 6.45 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 3.3x faster than the host (was 3.1x) |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.6 ms | 33.7 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.3 ms | 68.5 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.3 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.7 ms | 20.9 ms | +0.6% | [+0.3%, +0.9%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.0 ms | 38.1 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.67 ms | 9.68 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host (was 3.8x) |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.9 ms | 12.9 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 3.9x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.29 ms | 3.27 ms | -0.5% | [-1.0%, -0.1%] | +0.0% | ~ | device 3.8x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.58 ms | 2.58 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.57 ms | 4.57 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.07 ms | 3.05 ms | -0.4% | [-1.5%, +0.6%] | +0.0% | ~ | device 3.3x faster than the host |
| `QueryOrder/random/mortonOrder/float/hostdevice` | 16.9 ms | 16.9 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 3.6x faster than the host |
| `QueryOrder/armadillo/distance-arrived/float/hostdevice` | 9.13 ms | 9.16 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ | device 5.0x faster than the host |
| `QueryOrder/armadillo/distance-ordered/float/hostdevice` | 7.34 ms | 7.30 ms | -0.5% | [-1.7%, +0.7%] | +0.0% | ~ | device 5.5x faster than the host (was 6.5x); arrived/ordered 1.25x |
| `QueryOrder/random/mortonOrder/double/hostdevice` | 17.7 ms | 17.6 ms | -0.7% | [-1.5%, +0.2%] | - | ~ | device 3.6x faster than the host (was 4.4x) |
| `QueryOrder/armadillo/distance-arrived/double/hostdevice` | 113 ms | 111 ms | -1.4% | [-2.6%, -0.1%] | +0.0% | ~ | device 0.4x faster than the host |
| `QueryOrder/armadillo/distance-ordered/double/hostdevice` | 78.9 ms | 79.2 ms | +0.3% | [-2.4%, +3.2%] | +0.0% | ~ | device 0.5x faster than the host; arrived/ordered 1.40x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 707 µs | 704 µs | -0.5% | [-0.9%, -0.1%] | - | ~ | device 17.6x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.31 ms | 1.31 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 15.7x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 658 µs | 662 µs | +0.6% | [-0.7%, +1.9%] | - | ~ | device 17.4x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.24 ms | +1.1% | [+0.7%, +1.5%] | - | ~ | device 15.4x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 626 µs | 627 µs | +0.2% | [-0.7%, +1.2%] | - | ~ | device 16.0x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.16 ms | 1.15 ms | -0.8% | [-1.3%, -0.3%] | - | ~ | device 14.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.19 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.59x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.49 ms | 2.49 ms | +0.3% | [-0.0%, +0.7%] | - | ~ | device 9.7x faster than the host (was 10.0x); triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 12.0x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 10.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 592 µs | 594 µs | +0.3% | [-0.1%, +0.6%] | - | ~ | device 2.5x faster than the host; tape/fused 1.16x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 659 µs | 693 µs | +5.2% | [-0.1%, +10.4%] | - | ~ | device 2.6x faster than the host (was 2.9x); tape/fused 1.24x (was 1.29x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 708 µs | 689 µs | -2.7% | [-2.9%, -2.5%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 849 µs | 857 µs | +1.0% | [+0.4%, +1.6%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 18.6 ms | 18.8 ms | +1.0% | [+0.8%, +1.2%] | - | ~ | device 0.7x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 30.2 ms | 30.2 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 0.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 18.1 ms | 18.0 ms | -0.5% | [-1.3%, +0.4%] | - | ~ | device 0.8x faster than the host (was 0.7x) |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 29.1 ms | 29.2 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.8 ms | 15.9 ms | +0.4% | [-0.2%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 26.0 ms | 26.0 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 17.3 ms | 17.1 ms | -1.2% | [-1.5%, -0.8%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.10x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.4 ms | 25.5 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.18x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 22.9 ms | 23.1 ms | +1.0% | [+0.3%, +1.7%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.24 ms | 1.25 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.37 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 2.0x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.53 ms | 2.53 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.7x faster than the host (was 2.6x); marched/traced 3.55x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.98 ms | 2.98 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 2.4x faster than the host; marched/traced 4.66x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 8.99 ms | 9.00 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 5.0x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.9 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.16 ms | 2.19 ms | +1.7% | [+1.5%, +2.0%] | - | ~ | device 4.0x faster than the host; marched/traced 13.33x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.42 ms | 5.54 ms | +2.2% | [+2.0%, +2.3%] | - | ~ | device 2.9x faster than the host; marched/traced 9.93x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.0 ms | 29.2 ms | +0.9% | [+0.8%, +0.9%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.1 ms | 55.0 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 2.1x faster than the host (was 2.4x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.23 ms | 5.33 ms | +1.9% | [+1.9%, +2.0%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.81 ms | 8.99 ms | +2.0% | [+1.9%, +2.2%] | - | ~ | device 3.4x faster than the host (was 3.5x) |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.14 ms | 6.14 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 1.1x faster than the host (was 1.3x); marched/traced 26.08x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 0.7x faster than the host (was 0.8x); marched/traced 24.08x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 163 ms | 160 ms | -1.5% | [-4.9%, +2.1%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 254 ms | 255 ms | +0.4% | [-2.6%, +3.6%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.86 ms | 4.87 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.8x faster than the host; marched/traced 22.06x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.3 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 1.3x faster than the host; marched/traced 17.79x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 111 ms | 107 ms | -2.8% | [-5.0%, -0.6%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 217 ms | 219 ms | +0.7% | [-1.1%, +2.5%] | - | ~ | device 0.6x faster than the host (was 0.5x) |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.2 ms | -0.7% | [-1.7%, +0.3%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.8 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 705 µs | 709 µs | +0.5% | [-1.7%, +2.8%] | - | ~ | device 17.5x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.32 ms | 1.31 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 15.4x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 713 µs | 706 µs | -1.0% | [-1.7%, -0.4%] | - | ~ | device 19.9x faster than the host (was 17.3x); hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.32 ms | 1.32 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 15.4x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 707 µs | 671 µs | -5.0% | [-5.3%, -4.6%] | - | faster | device 3.9x faster than the host (was 3.7x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 849 µs | 845 µs | -0.5% | [-1.2%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 603 µs | 628 µs | +4.2% | [+3.7%, +4.5%] | - | minor | device 3.9x faster than the host (was 4.1x); hit/crossing 1.07x (was 1.17x) |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 627 µs | 649 µs | +3.5% | [+3.3%, +4.0%] | - | minor | device 4.5x faster than the host (was 4.7x); hit/crossing 1.30x (was 1.35x) |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.76 ms | 4.78 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 10.3x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.3 ms | 10.3 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 7.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.23 ms | 4.21 ms | -0.5% | [-0.6%, -0.5%] | - | ~ | device 11.1x faster than the host (was 10.2x); hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.16 ms | 9.11 ms | -0.5% | [-0.6%, -0.4%] | - | ~ | device 6.9x faster than the host (was 7.7x); hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.0 ms | 57.5 ms | +2.7% | [+1.5%, +3.9%] | - | ~ | device 3.1x faster than the host (was 3.2x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.8 ms | 82.4 ms | +0.8% | [-0.7%, +2.3%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.5 ms | 48.5 ms | +0.0% | [-1.1%, +1.1%] | - | ~ | device 3.0x faster than the host; hit/crossing 1.19x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.8 ms | 63.3 ms | -0.7% | [-1.2%, -0.3%] | - | ~ | device 3.1x faster than the host (was 2.5x); hit/crossing 1.30x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 18.6 ms | 18.6 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 30.1 ms | 30.1 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 18.8 ms | 18.7 ms | -0.5% | [-1.2%, +0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 30.1 ms | 30.3 ms | +0.7% | [-0.0%, +1.4%] | - | ~ | device 0.7x faster than the host (was 0.7x); hit/crossing 0.99x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.12 ms | +1.5% | [+0.8%, +2.2%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.25x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.17 ms | +0.9% | [+0.6%, +1.2%] | - | ~ | device 2.1x faster than the host (was 2.1x); hit/crossing 2.13x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 68.6 ms | 68.8 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 0.8x faster than the host (was 0.7x) |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 129 ms | 129 ms | +0.6% | [-0.1%, +1.2%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 62.0 ms | 62.1 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 0.9x faster than the host; hit/crossing 1.11x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 119 ms | 118 ms | -0.8% | [-1.6%, -0.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 481 ms | 476 ms | -1.1% | [-4.1%, +2.0%] | - | ~ | device 0.5x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 716 ms | 709 ms | -0.9% | [-2.1%, +0.3%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 404 ms | 396 ms | -2.0% | [-9.0%, +5.3%] | - | ~ | device 0.5x faster than the host (was 0.5x); hit/crossing 1.20x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 542 ms | 530 ms | -2.1% | [-3.2%, -0.9%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.34x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.54 ms | 3.58 ms | +1.3% | [-0.1%, +2.7%] | - | ~ | device 1.2x faster than the host (was 1.2x); marched/traced 3.36x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.83 ms | 4.79 ms | -0.9% | [-1.4%, -0.4%] | - | ~ | device 1.0x faster than the host; marched/traced 5.58x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | +0.6% | [-0.0%, +1.1%] | - | ~ | device 4.0x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.5 ms | 26.7 ms | +0.7% | [+0.6%, +0.9%] | - | ~ | device 2.1x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.26 ms | 1.25 ms | -0.7% | [-1.7%, +0.3%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.86 ms | 1.85 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 98.0 µs | 97.8 µs | -0.2% | [-2.0%, +1.7%] | - | ~ | device 15.1x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 154 µs | 156 µs | +1.7% | [-1.3%, +4.9%] | - | ~ | device 10.9x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 797 µs | 804 µs | +0.8% | [+0.6%, +1.1%] | - | ~ | device 3.0x faster than the host (was 3.1x) |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 936 µs | 939 µs | +0.3% | [+0.3%, +0.4%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 36.4 ms | 36.1 ms | -0.8% | [-2.5%, +0.9%] | - | ~ | device 0.1x faster than the host; marched/traced 3.40x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.5 ms | 36.7 ms | +0.7% | [-0.1%, +1.4%] | - | ~ | device 0.1x faster than the host; marched/traced 6.34x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 122 ms | 123 ms | +0.5% | [-2.2%, +3.3%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 234 ms | 233 ms | -0.5% | [-3.5%, +2.5%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.4 ms | 16.5 ms | +0.5% | [-4.7%, +6.3%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 21.8 ms | -0.5% | [-1.4%, +0.5%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.14 ms | 1.13 ms | -1.0% | [-3.3%, +1.4%] | - | ~ | device 1.5x faster than the host (was 1.3x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.53 ms | 2.52 ms | -0.6% | [-3.0%, +1.8%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.96 ms | 6.01 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.70 ms | 6.72 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.2 ms | 23.4 ms | +1.0% | [-1.3%, +3.4%] | - | ~ | device 2.2x faster than the host; marched/traced 2.68x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.4 ms | 28.3 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 2.0x faster than the host; marched/traced 3.71x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 61.9 ms | 62.8 ms | +1.5% | [+0.8%, +2.1%] | - | ~ | device 3.6x faster than the host (was 3.8x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 103 ms | 105 ms | +1.5% | [-1.6%, +4.6%] | - | ~ | device 2.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.8 ms | +0.2% | [-0.3%, +0.8%] | - | ~ | device 3.7x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.6 ms | 17.4 ms | -0.8% | [-1.2%, -0.3%] | - | ~ | device 3.6x faster than the host (was 3.2x) |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 300 ms | 305 ms | +1.5% | [-4.3%, +7.6%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 1.81x (was 1.89x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 358 ms | 348 ms | -3.0% | [-6.4%, +0.5%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 2.82x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 567 ms | 552 ms | -2.7% | [-3.4%, -1.9%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.00 s | 982 ms | -2.0% | [-2.7%, -1.2%] | - | ~ | device 0.4x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 189 ms | 191 ms | +0.7% | [-1.6%, +3.0%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 235 ms | 232 ms | -1.5% | [-2.8%, -0.1%] | - | ~ | device 0.3x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.99 ms | 2.78 ms | -6.9% | [-7.2%, -6.6%] | - | faster | device 2.8x faster than the host (was 2.6x) |
| `Regions/particles/regionDistances/float/hostdevice` | 3.64 ms | 3.42 ms | -6.0% | [-6.2%, -5.9%] | - | faster | device 3.9x faster than the host (was 3.7x) |
| `Regions/particles/regionByRegion/float/hostdevice` | 97.7 ms | 82.7 ms | -15.4% | [-15.5%, -15.2%] | - | faster | device 3.7x faster than the host (was 3.1x); one-pass/each 0.04x (was 0.04x) |
| `Regions/particles/domainLabels/float/hostdevice` | 3.72 ms | 3.42 ms | -8.0% | [-8.9%, -7.1%] | - | faster | device 4.1x faster than the host (was 3.8x) |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.78 ms | 6.89 ms | +1.7% | [+1.6%, +1.8%] | - | ~ | device 3.8x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.63 ms | 5.53 ms | -1.7% | [-1.7%, -1.6%] | - | ~ | device 3.4x faster than the host (was 3.2x); regions/hit 1.25x (was 1.20x) |
| `Regions/particles/region/double/hostdevice` | 3.42 ms | 3.28 ms | -4.0% | [-4.4%, -3.7%] | - | faster | device 2.4x faster than the host (was 2.3x) |
| `Regions/particles/regionDistances/double/hostdevice` | 5.81 ms | 5.80 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 2.4x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 119 ms | 133 ms | +11.8% | [+11.3%, +12.5%] | - | **MAJOR** | device 3.3x faster than the host (was 3.5x); one-pass/each 0.04x (was 0.05x) |
| `Regions/particles/domainLabels/double/hostdevice` | 5.92 ms | 5.93 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 2.4x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.8 ms | +0.4% | [-0.1%, +0.8%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 12.9 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.4x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.47 ms | +4.2% | [+4.0%, +4.4%] | - | minor | device 3.8x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.36 ms | 2.46 ms | +4.3% | [+4.0%, +4.5%] | - | minor | device 3.8x faster than the host (was 3.9x) |
| `Render/mesh/frame/float/hostdevice` | 2.14 ms | 2.15 ms | +0.7% | [+0.2%, +1.2%] | - | ~ | device 8.0x faster than the host (was 8.5x) |
| `Render/csg/frame/double/hostdevice` | 5.62 ms | 5.68 ms | +1.1% | [+0.9%, +1.2%] | - | ~ | device 1.4x faster than the host (was 1.8x) |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.62 ms | 5.67 ms | +1.0% | [+0.7%, +1.3%] | - | ~ | device 1.8x faster than the host (was 1.5x) |
| `Render/mesh/frame/double/hostdevice` | 26.4 ms | 25.9 ms | -2.1% | [-2.6%, -1.6%] | - | ~ | device 0.7x faster than the host |
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
| `ShapeFamilies/sampled-grid-morton/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/sampled-grid-random/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
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
| `ShapeFamilies/sampled-grid-morton/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/sampled-grid-random/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `ShapeFamilies/ray-forms/firstHit/double/hostdevice` | - | - | - | - | - | new |  |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 155 µs | 155 µs | +0.1% | [-0.3%, +0.6%] | - | ~ | device 33.4x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 47.6 µs | 47.6 µs | +0.1% | [-0.3%, +0.6%] | - | ~ | device 19.5x faster than the host (was 18.4x) |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.1% | [-0.4%, +0.7%] | - | ~ | device 15.1x faster than the host (was 16.1x) |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 48.0 µs | 48.1 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 66.8x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 17.5x faster than the host (was 16.7x) |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 47.8 µs | 48.0 µs | +0.4% | [+0.3%, +0.5%] | - | ~ | device 16.8x faster than the host |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.2% | [-0.4%, -0.1%] | - | ~ | device 9.9x faster than the host |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.7 µs | 47.7 µs | +0.0% | [-0.4%, +0.5%] | - | ~ | device 7.7x faster than the host |
| `Shapes/wedge/signedDistance/float/hostdevice` | 48.2 µs | 47.7 µs | -1.0% | [-2.3%, +0.2%] | - | ~ | device 9.3x faster than the host (was 8.3x) |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 555 µs | 551 µs | -0.7% | [-1.1%, -0.2%] | - | ~ | device 8.4x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.23 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 492 µs | 492 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.1x faster than the host |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 422 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.9x faster than the host (was 2.1x) |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 357 µs | 358 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.6x faster than the host (was 2.2x) |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 669 µs | 670 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 413 µs | 414 µs | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.4x faster than the host (was 1.3x) |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 340 µs | +0.3% | [-0.1%, +0.8%] | - | ~ | device 1.3x faster than the host |
| `Shapes/wedge/signedDistance/double/hostdevice` | 154 µs | 154 µs | -0.2% | [-0.2%, -0.1%] | - | ~ | device 3.1x faster than the host |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.04 ms | 8.02 ms | -0.2% | [-0.8%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 892 µs | 889 µs | -0.4% | [-1.0%, +0.1%] | +0.0% | ~ | device 10.6x faster than the host (was 12.1x) |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.65 ms | 2.66 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.6 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.1 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.3 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | -0.2% | [-0.2%, -0.2%] | +0.0% | ~ | device 4.0x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | -0.0% | [-0.0%, +0.0%] | +0.0% | ~ | device 4.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 2.99 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ | device 7.4x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.78 ms | 4.76 ms | -0.4% | [-0.6%, -0.3%] | +0.0% | ~ | device 5.4x faster than the host (was 5.2x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 5.2x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.0 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | device 4.4x faster than the host (was 3.9x); tape/fused 1.27x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 23.0 ms | 22.9 ms | -0.5% | [-0.7%, -0.4%] | +0.0% | ~ | device 4.3x faster than the host (was 3.9x) |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.6 ms | 30.4 ms | -0.6% | [-0.7%, -0.6%] | +0.0% | ~ | device 3.1x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.0 ms | 44.0 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 1.1x faster than the host (was 0.5x) |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.4 ms | 45.7 ms | +0.5% | [+0.0%, +1.0%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 122 ms | 125 ms | +2.6% | [-0.4%, +5.6%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 126 ms | 127 ms | +0.7% | [-1.7%, +3.0%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 126 ms | 130 ms | +2.8% | [+1.5%, +4.2%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 134 ms | 133 ms | -0.7% | [-3.4%, +2.1%] | +0.0% | ~ | device 0.8x faster than the host (was 1.1x) |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | device 10.9x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.2 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ | device 0.6x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.29 ms | 9.30 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 13.2x faster than the host (was 11.8x) |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.91 ms | 5.92 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 14.4x faster than the host (was 11.3x); surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 97.3 ms | 97.4 ms | +0.1% | [-1.0%, +1.3%] | - | ~ | device 4.1x faster than the host (was 4.0x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.8 ms | 63.5 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 3.6x faster than the host; surface/leaf 1.53x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.8 ms | 30.1 ms | +1.1% | [+0.9%, +1.3%] | - | ~ | device 5.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.99 ms | 9.99 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 7.1x faster than the host (was 8.0x); surface/hit 3.01x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.0 ms | 59.0 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.6x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 784 ms | 788 ms | +0.6% | [+0.5%, +0.7%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 528 ms | 531 ms | +0.6% | [-0.1%, +1.3%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.48x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 315 ms | 313 ms | -0.6% | [-1.8%, +0.7%] | - | ~ | device 0.5x faster than the host (was 0.6x) |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 128 ms | 127 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 0.6x faster than the host; surface/hit 2.46x |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 227 µs | +3.8% | [+3.0%, +4.0%] | - | minor | device 13.9x faster than the host (was 14.3x) |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 47.8 µs | -0.4% | [-0.8%, +0.1%] | - | ~ | device 23.4x faster than the host (was 24.8x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 387 µs | +7.7% | [+7.4%, +8.1%] | - | minor | device 15.3x faster than the host (was 16.5x) |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.58 ms | +1.2% | [+1.1%, +1.4%] | - | ~ | device 9.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.0 µs | 76.8 µs | +6.6% | [+4.8%, +7.7%] | - | minor | device 8.5x faster than the host (was 9.3x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.6 µs | 47.9 µs | +0.6% | [+0.1%, +1.1%] | - | ~ | device 5.4x faster than the host (was 4.8x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 651 µs | -0.1% | [-0.1%, -0.0%] | - | ~ | device 10.9x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.1% | [-0.1%, +0.0%] | - | ~ | device 12.1x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 477 µs | 511 µs | +7.1% | [+6.8%, +7.4%] | - | minor | device 9.3x faster than the host (was 10.0x) |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.1 µs | +0.1% | [-0.2%, +0.5%] | - | ~ | device 28.2x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 858 µs | 858 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.65 ms | 6.64 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 2.1x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 192 µs | +0.4% | [+0.3%, +0.6%] | - | ~ | device 3.1x faster than the host (was 2.9x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.77 ms | 2.77 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.12 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.64 ms | +1.7% | [+1.5%, +1.8%] | - | ~ | device 8.1x faster than the host (was 8.4x) |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.70 ms | 3.80 ms | +2.6% | [+2.5%, +2.7%] | - | ~ | device 10.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.50 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.6x faster than the host (was 2.9x) |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4677.0 s → 6156.1 s (+31.6%), the largest peak 5452 MB → 6531 MB; 20 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.6 s | 13.8 s | +1.2% | 618 MB | 626 MB | +1.3% | ~ |
| `BVHRefit` | 7.2 s | 7.4 s | +2.8% | 523 MB | 531 MB | +1.6% | ~ |
| `BVHUnion` | 25.4 s | 26.2 s | +3.1% | 1042 MB | 1050 MB | +0.8% | ~ |
| `BVHUnionMixed` | 11.3 s | 11.5 s | +1.2% | 636 MB | 644 MB | +1.2% | ~ |
| `ClosestPrimitive` | 106.2 s | 132.0 s | +24.3% | 1371 MB | 1559 MB | +13.7% | **minor** |
| `ClosestSurface` | 138.5 s | 177.6 s | +28.3% | 1316 MB | 1703 MB | +29.4% | **MAJOR** |
| `ConnectedComponents` | 20.5 s | 22.6 s | +10.3% | 952 MB | 1050 MB | +10.3% | **minor** |
| `Domain` | 50.8 s | 63.6 s | +25.3% | 1007 MB | 1127 MB | +12.0% | **MAJOR** |
| `DomainObject` | 49.4 s | 61.6 s | +24.7% | 959 MB | 1058 MB | +10.4% | **minor** |
| `EBGrid` | 19.7 s | 20.3 s | +2.9% | 767 MB | 774 MB | +0.9% | ~ |
| `Fused` | 50.6 s | 59.9 s | +18.3% | 1178 MB | 1279 MB | +8.6% | **minor** |
| `LaunchTuning` | 454.7 s | 572.6 s | +25.9% | 3134 MB | 3896 MB | +24.3% | **MAJOR** |
| `Mesh` | 21.7 s | 22.3 s | +3.0% | 824 MB | 831 MB | +0.9% | ~ |
| `MeshCSG` | 366.9 s | 448.3 s | +22.2% | 5192 MB | 6430 MB | +23.8% | **minor** |
| `MeshClosestPoint` | 19.5 s | 19.6 s | +0.7% | 776 MB | 783 MB | +0.9% | ~ |
| `MeshIngest` | 10.8 s | 10.6 s | -1.8% | 549 MB | 558 MB | +1.6% | ~ |
| `MeshReconcile` | 10.7 s | 10.8 s | +1.0% | 526 MB | 535 MB | +1.6% | ~ |
| `MeshTape` | 650.1 s | 828.2 s | +27.4% | 5452 MB | 6531 MB | +19.8% | **MAJOR** |
| `PointCloud` | 12.6 s | 13.1 s | +4.0% | 577 MB | 587 MB | +1.7% | ~ |
| `PointCloudBVHBuild` | 7.1 s | 7.5 s | +5.7% | 523 MB | 532 MB | +1.6% | ~ |
| `PointCloudHashGridBuild` | 6.5 s | 7.0 s | +7.0% | 523 MB | 532 MB | +1.6% | ~ |
| `QueryOrder` | 16.1 s | 16.4 s | +2.2% | 708 MB | 715 MB | +1.0% | ~ |
| `Ray` | 165.6 s | 202.7 s | +22.4% | 3029 MB | 3791 MB | +25.1% | **MAJOR** |
| `RayCSG` | 200.0 s | 237.1 s | +18.6% | 3368 MB | 4232 MB | +25.6% | **MAJOR** |
| `RayCrossing` | 388.3 s | 480.9 s | +23.8% | 3300 MB | 4102 MB | +24.3% | **minor** |
| `RayParticles` | 422.2 s | 755.0 s | +78.8% | 4847 MB | 5868 MB | +21.1% | **MAJOR** |
| `RayShell` | 243.1 s | 460.7 s | +89.5% | 4603 MB | 5570 MB | +21.0% | **MAJOR** |
| `Regions` | 314.6 s | 401.5 s | +27.6% | 3363 MB | 4124 MB | +22.6% | **MAJOR** |
| `Render` | 237.5 s | 291.9 s | +22.9% | 4920 MB | 6115 MB | +24.3% | **minor** |
| `ShapeFamilies` | - | 27.2 s | | - | 612 MB | | new |
| `Shapes` | 27.5 s | 29.0 s | +5.3% | 599 MB | 608 MB | +1.6% | ~ |
| `SphereCloudBVH` | 8.7 s | 9.5 s | +9.2% | 524 MB | 532 MB | +1.6% | ~ |
| `SphereCloudRays` | 157.1 s | 184.2 s | +17.2% | 3332 MB | 4078 MB | +22.4% | **minor** |
| `SphereContacts` | 8.7 s | 8.5 s | -2.5% | 523 MB | 531 MB | +1.6% | ~ |
| `SurfaceParts` | 336.5 s | 424.4 s | +26.1% | 3494 MB | 4460 MB | +27.7% | **MAJOR** |
| `Tape` | 56.7 s | 70.1 s | +23.7% | 992 MB | 1092 MB | +10.1% | **minor** |
| `TapeBVHUnion` | 40.6 s | 47.9 s | +18.1% | 932 MB | 1032 MB | +10.7% | **minor** |
| **all, in both** | 4677.0 s | 6156.1 s | +31.6% | 5452 MB | 6531 MB | +19.8% | **MAJOR** |

Device kernels: 226 in both, 94 changed (68 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 6 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double, Fused::Node<Fused::Comb…` | 168 → 168 | 9328 → 10384 | 16/4 → 20/36 | **spills stack** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double, TriMeshSDF<double, 4ul,…` | 168 → 168 | 9360 → 10368 | 20/4 → 28/40 | **spills stack** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double> >, Vec3T<double> const,…` | 168 → 168 | 9328 → 10368 | 16/4 → 20/36 | **spills stack** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float, Fused::Node<Fused::Combin…` | 168 → 168 | 6672 → 7136 | 4/4 → 4/20 | **spills** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float, TriMeshSDF<float, 4ul, 4u…` | 168 → 168 | 6640 → 7120 | 4/4 → 4/20 | **spills** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float> >, Vec3T<float> const, Le…` | 168 → 168 | 6656 → 7152 | 4/4 → 4/20 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, BVHUnion<double, 4ul, …` | 168 → 168 | 7600 → 7600 | 20/4 → 24/20 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, SphereCloudBVH<double,…` | 168 → 168 | 7584 → 7616 | 20/4 → 24/20 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, BVHUnion<float, 4ul, Sph…` | 168 → 168 | 4624 → 4688 | 4/4 → 4/12 | **spills** |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, SphereCloudBVH<float, 4u…` | 168 → 168 | 4624 → 4720 | 4/4 → 4/12 | **spills** |
| `Domain` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)0>, Vec3T<double> const,…` | 168 → 168 | 4448 → 4480 | 228/272 → 256/300 | **spills** |
| `Domain` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)0>, Vec3T<float> const, A…` | 168 → 168 | 3104 → 3120 | 180/232 → 192/260 | **spills** |
| `DomainObject` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)0>, Vec3T<double> const,…` | 168 → 168 | 4192 → 4224 | 228/272 → 256/300 | **spills** |
| `DomainObject` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)0>, Vec3T<float> const, A…` | 168 → 168 | 2976 → 2992 | 180/232 → 192/260 | **spills** |
| `Fused` | `void parallelForKernel<128u, 384u, Apply<Distance<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 4448 → 4512 | 20/4 → 24/20 | **spills** |
| `Fused` | `void parallelForKernel<128u, 384u, Apply<Distance<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 3024 → 3072 | 4/4 → 4/12 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 17168 | 178/216 → 174/236 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11920 | 114/132 → 114/140 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 80 → 80 | 17328 → 17536 | 282/460 → 306/516 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 80 → 80 | 12320 → 12416 | 126/132 → 130/140 | **spills** |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 128 → 128 | 17152 → 17376 | 178/216 → 190/236 | **spills** |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 128 → 128 | 12064 → 12208 | 114/132 → 114/140 | **spills** |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> cons…` | 255 → 40 | 16512 → 17808 | 166/216 → 706/1300 | **registers spills** |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const, …` | 40 → 40 | 12608 → 12656 | 230/368 → 246/412 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17296 | 174/220 → 174/236 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11888 → 12032 | 114/132 → 114/140 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19248 → 19568 | 1500/1412 → 1556/1476 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17296 | 174/220 → 174/236 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11920 → 12032 | 114/132 → 114/140 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, RegionView<Tape<double, TriMeshSDF<…` | 168 → 168 | 20400 → 20784 | 2788/2348 → 2892/2348 | **spills** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19248 → 19568 | 1500/1412 → 1556/1476 | **spills** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 17168 | 178/216 → 174/236 | **spills** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11920 | 114/132 → 114/140 | **spills** |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16832 → 17072 | 164/148 → 160/152 | **spills** |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11712 → 11856 | 132/132 → 132/140 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17072 | 174/220 → 174/236 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 17184 | 178/216 → 174/236 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11920 → 11968 | 114/132 → 114/140 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11872 | 114/132 → 114/140 | **spills** |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13328 → 13360 | 178/160 → 178/168 | **spills** |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13152 → 13216 | 178/160 → 178/168 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)1>, Vec3T<double> const,…` | 168 → 168 | 4160 → 4208 | 340/352 → 368/380 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)3>, Vec3T<double> const,…` | 168 → 168 | 4208 → 4240 | 228/272 → 256/300 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)1>, Vec3T<float> const, R…` | 168 → 168 | 2944 → 2960 | 236/268 → 248/296 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)3>, Vec3T<float> const, R…` | 168 → 168 | 2976 → 2992 | 180/232 → 192/260 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, false>, Vec3T<double> const, Reg…` | 168 → 168 | 16944 → 17168 | 178/192 → 174/212 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, false>, Vec3T<float> const, Regio…` | 168 → 168 | 11824 → 11920 | 114/120 → 114/128 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 19120 → 19504 | 1508/1376 → 1556/1476 | **spills** |
| `SphereCloudRays` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, SphereCloudBVH<doub…` | 168 → 168 | 17152 → 17184 | 126/104 → 126/120 | **spills** |
| `SphereCloudRays` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, SphereCloudBVH<float,…` | 168 → 168 | 12032 → 11824 | 102/88 → 118/148 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, BVHUnion<double, …` | 168 → 168 | 7616 → 7792 | 36/120 → 40/140 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, BVHUnion<double, …` | 168 → 168 | 7632 → 7792 | 28/128 → 32/148 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, TriMeshSDF<double…` | 168 → 168 | 7648 → 7792 | 68/120 → 40/140 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, TriMeshSDF<double…` | 168 → 168 | 7664 → 7792 | 60/128 → 32/148 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<float, Tape<float, BVHUnion<float, 4ul…` | 168 → 168 | 4688 → 4864 | 36/132 → 40/120 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<float, Tape<float, BVHUnion<float, 4ul…` | 168 → 168 | 4688 → 4800 | 28/140 → 32/112 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<float, Tape<float, TriMeshSDF<float, 4…` | 168 → 168 | 4704 → 4832 | 36/132 → 40/120 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<float, Tape<float, TriMeshSDF<float, 4…` | 168 → 168 | 4704 → 4768 | 28/140 → 32/112 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17296 | 174/196 → 174/212 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17040 → 17296 | 274/276 → 274/292 | **spills** |

34 more changed kernels are in the records.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (18 cases): BVHRefit/uniform/refit/float/hostdevice, ClosestPrimitive/csg/closestPrimitive/float/hostdevice, ClosestPrimitive/parts/closestPrimitive/float/hostdevice, Domain/particles/labels/double/hostdevice, DomainObject/partsOwn/labelAt/float/hostdevice, EBGrid/MeshSDF/armadillo/distance-morton/double/hostdevice, LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice, MeshCSG/union-tape/frame/double/hostdevice, MeshCSG/smooth-fused/firstHit-morton/double/hostdevice, RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice, RayCrossing/csg-tape/firstCrossing-random/float/hostdevice, Regions/particles/regionByRegion/double/hostdevice, Render/csg/frame/float/hostdevice, Render/csg/frame-orthographic/float/hostdevice, Tape/csg/signedDistance/float/hostdevice, Tape/smooth/signedDistance/float/hostdevice, Tape/sphere/signedDistance/float/hostdevice, Tape/chain16/signedDistance/float/hostdevice

The probes differ between the sides (each side ran its own): Performance/ShapeFamilies/main.cpp

<!-- END receipt cuda -->
