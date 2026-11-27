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
head: 9963dc61
head-fingerprint: 4164bfeff5de8a073749d06c618e84dec51da9d5
cases: 366
rounds: 2
reps: 3
date: 2026-11-27
-->

## Host comparison: yellow

**Accepted:** Accept as acceptable side effect of the feature

`9963dc61` against `235ade11` (235ade11), 2026-11-27.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 5 faster, 0 differ, 0 lost, 366 cases

- base: 235ade11 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 9963dc61 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 330 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 177 ms | 177 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 513 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 350 ms | +0.8% | [-0.2%, +1.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 563 ms | 562 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 41.5 ms | 41.7 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 75.4 ms | 75.8 ms | +0.5% | [+0.0%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.2 ms | 55.9 ms | -0.6% | [-1.2%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.5 ms | 51.8 ms | +0.6% | [-1.0%, +2.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.5 ms | 64.7 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 55.4 ms | 54.4 ms | -1.7% | [-3.0%, -0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.5 ms | 60.6 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.4 ms | 72.7 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 73.0 ms | 72.6 ms | -0.5% | [-1.6%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.5 ms | 77.6 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 65.1 ms | 64.6 ms | -0.9% | [-1.2%, -0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 101 ms | 100 ms | -0.8% | [-1.5%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 77.3 ms | 77.5 ms | +0.3% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 64.0 ms | 63.8 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 96.0 ms | 96.3 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 76.2 ms | 76.2 ms | -0.1% | [-0.9%, +0.7%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 100 ms | 100 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.3 ms | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.5 ms | 39.4 ms | -5.1% | [-6.6%, -3.6%] | +0.0% | faster |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.3 ms | 73.0 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 23.8 ms | 20.4 ms | -14.5% | [-25.9%, +0.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.3 ms | 43.7 ms | +0.8% | [-0.3%, +2.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.1 ms | 81.2 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 139 ms | 140 ms | +1.2% | [+0.9%, +1.6%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 77.0 ms | 76.4 ms | -0.8% | [-2.0%, +0.5%] | +0.0% | ~ | surface/leaf 1.84x |
| `ClosestSurface/union/closestSurface/float/host` | 146 ms | 146 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 76.6 ms | 77.0 ms | +0.5% | [-0.0%, +1.0%] | +0.0% | ~ | surface/leaf 1.90x |
| `ClosestSurface/cloud/closestSurface/double/host` | 157 ms | 156 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 80.6 ms | 80.7 ms | +0.2% | [-0.9%, +1.2%] | +0.0% | ~ | surface/leaf 1.94x |
| `ClosestSurface/union/closestSurface/double/host` | 154 ms | 153 ms | -0.6% | [-0.9%, -0.2%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 81.5 ms | 81.1 ms | -0.5% | [-1.8%, +0.7%] | +0.0% | ~ | surface/leaf 1.89x |
| `ConnectedComponents/shells/build-coarse/float/host` | 148 ms | 148 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/float/host` | 660 ms | 662 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-coarse/double/host` | 152 ms | 154 ms | +1.1% | [+0.7%, +1.4%] | +0.0% | ~ |  |
| `ConnectedComponents/shells/build-fine/double/host` | 679 ms | 685 ms | +0.9% | [+0.8%, +1.1%] | +0.0% | ~ |  |
| `Domain/particles/labels/float/host` | 13.4 ms | 13.6 ms | +1.4% | [-1.3%, +4.2%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.72 ms | 6.77 ms | +0.6% | [-0.1%, +1.3%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.30 ms | 1.29 ms | -0.8% | [-1.0%, -0.6%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.1 ms | 13.9 ms | -1.7% | [-2.6%, -0.8%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.10 ms | 7.11 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.32 ms | 1.31 ms | -1.2% | [-2.6%, +0.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 39.4 ms | 39.6 ms | +0.6% | [-0.0%, +1.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.2 ms | 21.2 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 559 µs | 562 µs | +0.6% | [-1.4%, +2.6%] | +0.0% | ~ | whole/own 70.43x |
| `DomainObject/partsOwn/labelAt/float/host` | 298 µs | 298 µs | +0.1% | [-2.0%, +2.2%] | +0.0% | ~ | whole/own 70.94x |
| `DomainObject/partsWhole/labels/double/host` | 39.3 ms | 39.4 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.5 ms | 22.4 ms | -0.7% | [-1.0%, -0.4%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 590 µs | 579 µs | -1.9% | [-2.7%, -1.0%] | +0.0% | ~ | whole/own 67.99x |
| `DomainObject/partsOwn/labelAt/double/host` | 315 µs | 318 µs | +1.0% | [-2.0%, +4.0%] | +0.0% | ~ | whole/own 70.33x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/host` | 63.4 ms | 63.5 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/host` | 59.5 ms | 59.2 ms | -0.5% | [-1.2%, +0.3%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/host` | 77.6 ms | 77.3 ms | -0.3% | [-1.3%, +0.6%] | +0.0% | ~ | grid/shuffled 0.82x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/host` | 69.0 ms | 68.8 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/host` | 64.2 ms | 63.8 ms | -0.7% | [-0.9%, -0.5%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/host` | 89.2 ms | 89.2 ms | -0.0% | [-1.0%, +0.9%] | +0.0% | ~ | grid/shuffled 0.77x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/host` | 190 ms | 192 ms | +0.7% | [+0.1%, +1.2%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/host` | 178 ms | 178 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/host` | 256 ms | 257 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | grid/shuffled 0.75x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/host` | 202 ms | 201 ms | -0.4% | [-1.1%, +0.4%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/host` | 188 ms | 187 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/host` | 290 ms | 290 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | grid/shuffled 0.69x |
| `Fused/csg/signedDistance/float/host` | 4.43 ms | 4.44 ms | +0.1% | [-2.9%, +3.2%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | tape/fused 2.01x |
| `Fused/chain16/signedDistance/float/host` | 973 µs | 960 µs | -1.3% | [-4.1%, +1.5%] | - | ~ | tape/fused 17.02x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.2 ms | 46.2 ms | -0.0% | [-0.9%, +0.9%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.2 ms | 46.4 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.55 ms | 4.57 ms | +0.5% | [-1.9%, +3.0%] | - | ~ | tape/fused 2.90x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | -0.3% | [-1.4%, +0.7%] | - | ~ | tape/fused 2.10x |
| `Fused/chain16/signedDistance/double/host` | 3.90 ms | 3.90 ms | -0.0% | [-2.3%, +2.3%] | - | ~ | tape/fused 4.20x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.5 ms | 52.7 ms | +0.4% | [-0.4%, +1.3%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.9 ms | 53.0 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.7 ms | 82.6 ms | -0.1% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 228 ms | 230 ms | +0.8% | [+0.4%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 259 ms | -0.5% | [-1.0%, +0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 71.1 ms | 70.6 ms | -0.7% | [-1.2%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 213 ms | 213 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 97.5 ms | 97.6 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.5 ms | 29.0 ms | -1.7% | [-3.6%, +0.3%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 759 ms | 759 ms | -0.1% | [-0.8%, +0.7%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 118 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 42.6 ms | 42.9 ms | +0.8% | [+0.3%, +1.4%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 850 ms | 863 ms | +1.6% | [+0.9%, +2.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.1 ms | 26.3 ms | +0.7% | [+0.2%, +1.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.6 ms | 15.5 ms | -0.9% | [-1.4%, -0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 320 ms | 319 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 231 ms | 231 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.2 ms | 91.2 ms | +1.1% | [+0.3%, +1.8%] | +0.0% | ~ | tape/fused 3.50x |
| `MeshCSG/smooth-fused/frame/float/host` | 81.7 ms | 81.6 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | tape/fused 2.83x |
| `MeshCSG/union-tape/firstHit/double/host` | 28.9 ms | 28.9 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.2 ms | 17.1 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 359 ms | 355 ms | -0.9% | [-1.5%, -0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 245 ms | 245 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.6 ms | 97.1 ms | -0.5% | [-0.9%, -0.0%] | +0.0% | ~ | tape/fused 3.66x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.2 ms | 87.2 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | tape/fused 2.81x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 229 ms | 229 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 232 ms | 232 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | signedDistance/closestPoint 0.99x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 307 ms | 307 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 232 ms | 232 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | closestPoint/gradient 1.00x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 261 ms | 261 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 263 ms | 267 ms | +1.5% | [+0.9%, +2.2%] | +0.0% | ~ | signedDistance/closestPoint 0.98x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 344 ms | 340 ms | -1.1% | [-2.0%, -0.3%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 264 ms | 264 ms | +0.4% | [-0.7%, +1.4%] | +0.0% | ~ | closestPoint/gradient 1.01x |
| `MeshIngest/armadillo/read-soup/float/host` | 53.8 ms | 53.7 ms | -0.1% | [-1.4%, +1.2%] | - | ~ |  |
| `MeshIngest/armadillo/clean/float/host` | 12.6 ms | 12.5 ms | -0.2% | [-1.9%, +1.5%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/float/host` | 96.0 ms | 95.9 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/float/host` | 103 ms | 103 ms | +0.2% | [-0.3%, +0.7%] | - | ~ |  |
| `MeshIngest/armadillo/read-soup/double/host` | 54.3 ms | 55.0 ms | +1.1% | [+0.6%, +1.7%] | - | ~ |  |
| `MeshIngest/armadillo/clean/double/host` | 13.6 ms | 13.4 ms | -1.2% | [-2.4%, +0.1%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/double/host` | 106 ms | 106 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/double/host` | 115 ms | 115 ms | +0.2% | [+0.0%, +0.4%] | - | ~ |  |
| `MeshReconcile/armadillo/reconcile/float/host` | 73.0 ms | 73.3 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.4 ms | 41.7 ms | +0.8% | [-0.7%, +2.3%] | +0.0% | ~ | reconcile/scheduled 1.76x |
| `MeshReconcile/armadillo/reconcile/double/host` | 94.1 ms | 93.2 ms | -1.0% | [-1.9%, -0.0%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.1 ms | 55.7 ms | -0.8% | [-1.7%, +0.2%] | +0.0% | ~ | reconcile/scheduled 1.67x |
| `MeshTape/mesh/firstHit/float/host` | 48.5 ms | 48.4 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 48.5 ms | 49.0 ms | +0.9% | [-0.7%, +2.6%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.6 ms | 19.5 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 80.3 ms | 80.1 ms | -0.2% | [-0.3%, -0.2%] | +0.0% | ~ | mesh/tape 0.60x |
| `MeshTape/tape/firstCrossing/float/host` | 49.8 ms | 49.8 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | mesh/tape 0.98x |
| `MeshTape/tape/frame/float/host` | 25.0 ms | 24.9 ms | -0.2% | [-0.3%, -0.2%] | +0.0% | ~ | mesh/tape 0.78x |
| `MeshTape/view/firstHit/float/host` | 83.3 ms | 83.7 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/float/host` | 26.3 ms | 26.2 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ | mesh/view 0.74x |
| `MeshTape/mesh/firstHit/double/host` | 53.1 ms | 53.6 ms | +0.8% | [-0.4%, +2.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 53.2 ms | 53.2 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.3 ms | 20.6 ms | +1.1% | [+0.9%, +1.3%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 87.6 ms | 87.8 ms | +0.3% | [-0.9%, +1.5%] | +0.0% | ~ | mesh/tape 0.61x |
| `MeshTape/tape/firstCrossing/double/host` | 55.3 ms | 55.8 ms | +0.9% | [+0.1%, +1.7%] | +0.0% | ~ | mesh/tape 0.95x |
| `MeshTape/tape/frame/double/host` | 27.2 ms | 26.9 ms | -0.9% | [-2.5%, +0.7%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 89.6 ms | 90.8 ms | +1.4% | [+0.4%, +2.3%] | +0.0% | ~ | mesh/view 0.59x |
| `MeshTape/view/frame/double/host` | 29.4 ms | 29.4 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | mesh/view 0.70x |
| `PointCloudBVH/uniform/build/float/host` | 53.1 ms | 53.1 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 144 ms | +0.5% | [-0.2%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 361 ms | 361 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 80.6 ms | 80.3 ms | -0.4% | [-0.8%, +0.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.72 ms | 6.81 ms | +1.4% | [+0.2%, +2.6%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 55.3 ms | 55.4 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 157 ms | 157 ms | -0.1% | [-0.6%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 392 ms | 393 ms | +0.2% | [+0.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 88.5 ms | 88.1 ms | -0.4% | [-1.5%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.97 ms | 7.01 ms | +0.6% | [-0.4%, +1.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 14.2 ms | 13.5 ms | -4.9% | [-12.6%, +3.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | +0.3% | [-1.1%, +1.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 475 ms | 476 ms | +0.2% | [-0.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 141 ms | 141 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 801 ms | 803 ms | +0.2% | [-0.2%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.2 ms | 14.9 ms | -2.2% | [-4.0%, -0.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 121 ms | 120 ms | -0.3% | [-0.5%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 492 ms | 491 ms | -0.2% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 144 ms | 144 ms | -0.1% | [-0.2%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 801 ms | 800 ms | -0.2% | [-0.5%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.4 ms | 55.1 ms | +1.2% | [-0.6%, +2.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 129 ms | 129 ms | +0.3% | [+0.2%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 269 ms | 268 ms | -0.4% | [-0.9%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 149 ms | 149 ms | -0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 134 ms | 134 ms | +0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 56.9 ms | 57.0 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 141 ms | 140 ms | -0.3% | [-0.8%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 298 ms | 300 ms | +0.4% | [-0.4%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 165 ms | 165 ms | -0.5% | [-2.1%, +1.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 149 ms | 149 ms | -0.0% | [-0.6%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.7 ms | 12.0 ms | +2.8% | [-1.0%, +3.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 114 ms | 114 ms | +0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 316 ms | 316 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 156 ms | 155 ms | -0.3% | [-0.6%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 322 ms | 321 ms | -0.3% | [-0.8%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.5 ms | 13.3 ms | -1.6% | [-3.4%, +0.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 116 ms | 115 ms | -0.1% | [-0.2%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 346 ms | 347 ms | +0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 164 ms | 165 ms | +0.7% | [-0.4%, +1.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 309 ms | 309 ms | -0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 38.8 ms | 35.7 ms | -7.9% | [-16.5%, +2.5%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.1 ms | 38.0 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.68 ms | 8.34 ms | -3.9% | [-8.1%, +0.8%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.52 ms | 9.29 ms | -2.4% | [-4.6%, -0.2%] | +0.0% | ~ |  |
| `QueryOrder/random/mortonOrder/float/host` | 60.8 ms | 60.8 ms | -0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | 223 ms | 223 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | 160 ms | 160 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | arrived/ordered 1.40x |
| `QueryOrder/random/mortonOrder/double/host` | 62.2 ms | 62.2 ms | +0.1% | [-0.3%, +0.5%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | 263 ms | 263 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | 171 ms | 171 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | arrived/ordered 1.54x |
| `Ray/mesh/firstHit/float/host` | 48.8 ms | 48.7 ms | -0.2% | [-1.2%, +0.7%] | +0.0% | ~ | hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.8 ms | 45.8 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.4 ms | -0.2% | [-1.1%, +0.8%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.5 ms | 78.3 ms | -0.3% | [-1.0%, +0.5%] | +0.0% | ~ | triangles/polygons 0.62x |
| `Ray/mesh-polygons/intersects/float/host` | 65.5 ms | 65.8 ms | +0.5% | [+0.3%, +0.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.20 ms | 5.27 ms | +1.3% | [+0.9%, +3.7%] | - | ~ | tape/fused 1.43x (was 1.49x) |
| `Ray/csg-tape/firstHit/float/host` | 7.75 ms | 7.55 ms | -2.5% | [-5.7%, +0.9%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 53.5 ms | 53.5 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ | hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.4 ms | 51.5 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.8 ms | 45.1 ms | +0.7% | [+0.2%, +1.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.6 ms | 89.1 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.8 ms | 73.7 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.49 ms | 5.52 ms | +0.5% | [-0.5%, +1.6%] | - | ~ | tape/fused 1.44x |
| `Ray/csg-tape/firstHit/double/host` | 8.01 ms | 7.98 ms | -0.4% | [-2.2%, +1.6%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.2 ms | 17.3 ms | +0.9% | [-0.6%, +2.5%] | - | ~ | marched/traced 6.84x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 118 ms | -0.2% | [-0.2%, -0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.9 ms | 21.8 ms | -0.2% | [-1.1%, +0.7%] | +0.0% | ~ | marched/traced 6.63x (was 6.95x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 152 ms | 145 ms | -4.9% | [-9.7%, +0.4%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.1 ms | 35.3 ms | +0.7% | [+0.5%, +0.8%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.5 ms | 18.8 ms | -3.6% | [-4.7%, -2.5%] | - | faster | marched/traced 6.72x (was 6.49x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 126 ms | -0.1% | [-0.8%, +0.5%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.1 ms | -1.1% | [-2.1%, -0.1%] | +0.0% | ~ | marched/traced 6.52x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 150 ms | 151 ms | +0.6% | [+0.6%, +0.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.5 ms | 37.4 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 48.4 ms | 48.5 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 48.4 ms | 48.6 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.56 ms | 7.53 ms | -0.4% | [-1.4%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.04 ms | 6.09 ms | +0.8% | [-0.1%, +1.6%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 178 ms | 177 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 158 ms | 156 ms | -0.7% | [-1.2%, -0.3%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.7 ms | 68.9 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.3 ms | 56.4 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 53.1 ms | 52.8 ms | -0.6% | [-1.2%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 53.0 ms | 53.0 ms | -0.0% | [-0.7%, +0.6%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.85 ms | 7.89 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.29 ms | 6.28 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 200 ms | 201 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 178 ms | 179 ms | +0.7% | [-0.6%, +2.0%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.4 ms | 77.4 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 65.8 ms | 64.6 ms | -1.9% | [-4.2%, +0.4%] | +0.0% | ~ | hit/crossing 1.20x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.2 ms | 12.7 ms | +13.8% | [-1.5%, +29.0%] | +0.0% | ~ | marched/traced 4.06x (was 4.63x) |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.9 ms | 51.7 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.00 ms | 7.99 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.00 ms | 2.02 ms | +1.0% | [+0.8%, +1.2%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.89 ms | 6.87 ms | -0.3% | [-1.2%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.8 ms | 11.7 ms | -0.7% | [-1.7%, +0.3%] | +0.0% | ~ | marched/traced 5.20x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.3 ms | 60.7 ms | -1.0% | [-1.8%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.66 ms | 8.79 ms | +1.5% | [-0.4%, +3.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.09 ms | 2.02 ms | -3.3% | [-5.7%, -0.8%] | +0.0% | faster |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.31 ms | 7.12 ms | -2.7% | [-3.9%, -1.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 238 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 329 ms | 328 ms | -0.3% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 239 ms | 237 ms | -0.8% | [-1.2%, -0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 257 ms | 251 ms | -2.2% | [-3.2%, -1.2%] | +0.0% | ~ | marched/traced 1.47x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 371 ms | 369 ms | -0.6% | [-1.0%, -0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 253 ms | 250 ms | -1.1% | [-1.6%, -0.7%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.33 ms | 7.30 ms | -0.4% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 11.8 ms | -6.5% | [-7.8%, -5.2%] | +0.0% | faster |  |
| `Regions/particles/regionByRegion/float/host` | 265 ms | 266 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | one-pass/each 0.04x (was 0.05x) |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.6 ms | +0.7% | [+0.6%, +0.8%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.2 ms | 30.5 ms | +0.7% | [+0.6%, +0.9%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.2 ms | 22.0 ms | -0.6% | [-1.1%, -0.0%] | +0.0% | ~ | regions/hit 1.38x |
| `Regions/particles/region/double/host` | 7.60 ms | 7.59 ms | -0.1% | [-1.0%, +0.9%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.8 ms | 12.3 ms | -4.5% | [-5.5%, -3.4%] | +0.0% | faster |  |
| `Regions/particles/regionByRegion/double/host` | 282 ms | 283 ms | +0.4% | [+0.4%, +0.4%] | +0.0% | ~ | one-pass/each 0.04x (was 0.05x) |
| `Regions/particles/domainLabels/double/host` | 13.0 ms | 12.9 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.1 ms | 32.1 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.7 ms | 23.7 ms | +0.2% | [-0.5%, +1.0%] | +0.0% | ~ | regions/hit 1.35x |
| `Render/csg/frame/float/host` | 13.1 ms | 13.1 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 13.1 ms | 13.0 ms | -0.5% | [-0.6%, -0.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.6 ms | 19.5 ms | -0.5% | [-1.0%, -0.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.0 ms | 14.0 ms | -0.5% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 13.8 ms | 14.4 ms | +4.5% | [-0.3%, +9.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.7 ms | 20.6 ms | -0.6% | [-2.2%, +1.0%] | +0.0% | ~ |  |
| `ShapeFamilies/icosahedron/signedDistance/float/host` | 16.6 ms | 16.6 ms | -0.0% | [-1.2%, +1.1%] | - | ~ |  |
| `ShapeFamilies/pyramid/signedDistance/float/host` | 11.9 ms | 12.0 ms | +0.9% | [-0.3%, +2.1%] | - | ~ |  |
| `ShapeFamilies/box-frame/signedDistance/float/host` | 3.28 ms | 3.26 ms | -0.7% | [-3.8%, +2.6%] | - | ~ |  |
| `ShapeFamilies/hyperboloid/signedDistance/float/host` | 17.0 ms | 16.9 ms | -0.4% | [-0.4%, -0.3%] | - | ~ |  |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/float/host` | 17.2 ms | 17.2 ms | +0.2% | [-0.8%, +1.2%] | - | ~ |  |
| `ShapeFamilies/paraboloid/signedDistance/float/host` | 10.3 ms | 10.4 ms | +1.0% | [+0.0%, +2.0%] | - | ~ |  |
| `ShapeFamilies/superellipsoid/signedDistance/float/host` | 10.1 ms | 10.1 ms | -0.2% | [-0.3%, -0.0%] | - | ~ |  |
| `ShapeFamilies/solid-angle/signedDistance/float/host` | 988 µs | 981 µs | -0.6% | [-1.6%, +0.4%] | - | ~ |  |
| `ShapeFamilies/vesica/signedDistance/float/host` | 2.01 ms | 2.00 ms | -0.4% | [-0.8%, +0.1%] | - | ~ |  |
| `ShapeFamilies/helix/signedDistance/float/host` | 27.0 ms | 27.1 ms | +0.3% | [-0.2%, +0.8%] | - | ~ |  |
| `ShapeFamilies/gyroid/signedDistance/float/host` | 5.09 ms | 5.04 ms | -0.9% | [-2.2%, +0.4%] | - | ~ |  |
| `ShapeFamilies/ray-forms/firstHit/float/host` | 35.8 ms | 35.8 ms | -0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `ShapeFamilies/icosahedron/signedDistance/double/host` | 16.7 ms | 16.6 ms | -0.2% | [-1.5%, +1.1%] | - | ~ |  |
| `ShapeFamilies/pyramid/signedDistance/double/host` | 12.2 ms | 12.2 ms | +0.4% | [+0.0%, +0.7%] | - | ~ |  |
| `ShapeFamilies/box-frame/signedDistance/double/host` | 3.31 ms | 3.27 ms | -1.2% | [-2.1%, -0.3%] | - | ~ |  |
| `ShapeFamilies/hyperboloid/signedDistance/double/host` | 27.6 ms | 27.6 ms | +0.1% | [+0.1%, +0.1%] | - | ~ |  |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/double/host` | 29.0 ms | 29.0 ms | +0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `ShapeFamilies/paraboloid/signedDistance/double/host` | 12.7 ms | 12.8 ms | +0.7% | [+0.1%, +1.2%] | - | ~ |  |
| `ShapeFamilies/superellipsoid/signedDistance/double/host` | 16.4 ms | 16.3 ms | -0.3% | [-1.4%, +0.9%] | - | ~ |  |
| `ShapeFamilies/solid-angle/signedDistance/double/host` | 1.16 ms | 1.15 ms | -0.5% | [-1.0%, -0.0%] | - | ~ |  |
| `ShapeFamilies/vesica/signedDistance/double/host` | 2.45 ms | 2.46 ms | +0.4% | [-0.9%, +1.7%] | - | ~ |  |
| `ShapeFamilies/helix/signedDistance/double/host` | 33.7 ms | 33.7 ms | -0.1% | [-0.2%, +0.1%] | - | ~ |  |
| `ShapeFamilies/gyroid/signedDistance/double/host` | 10.8 ms | 10.8 ms | -0.2% | [-2.8%, +2.4%] | - | ~ |  |
| `ShapeFamilies/ray-forms/firstHit/double/host` | 40.0 ms | 40.3 ms | +0.8% | [+0.5%, +1.2%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 33.0 ms | 33.3 ms | +0.9% | [-0.0%, +1.8%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 5.00 ms | 5.05 ms | +1.0% | [+0.3%, +1.8%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.19 ms | 4.19 ms | -0.0% | [-0.6%, +0.6%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.1 ms | 17.4 ms | +1.5% | [+1.1%, +2.0%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.33 ms | 4.27 ms | -1.3% | [-1.6%, -0.9%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.14 ms | 4.14 ms | +0.0% | [-1.3%, +1.3%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.24 ms | 2.27 ms | +1.5% | [-1.2%, +4.1%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.43 ms | 1.44 ms | +0.7% | [-0.2%, +1.5%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.08 ms | 2.08 ms | +0.3% | [-1.5%, +2.2%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.6 ms | 27.2 ms | -1.5% | [-2.9%, -0.1%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 46.4 ms | 46.2 ms | -0.5% | [-0.9%, -0.1%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.35 ms | 5.42 ms | +1.3% | [+0.3%, +2.4%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.47 ms | 4.46 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 22.4 ms | 22.4 ms | -0.2% | [-0.3%, +0.0%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.52 ms | 4.46 ms | -1.4% | [-2.4%, -0.4%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.27 ms | 5.23 ms | -0.8% | [-3.0%, +1.5%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.50 ms | 2.53 ms | +1.2% | [-2.0%, +4.5%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.77 ms | 1.77 ms | +0.3% | [-0.4%, +1.0%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.09 ms | 2.11 ms | +0.8% | [-0.6%, +2.3%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 30.2 ms | 30.2 ms | +0.1% | [-0.1%, +0.4%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.8 ms | 56.1 ms | -1.3% | [-2.4%, -0.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.6 ms | 36.4 ms | -0.5% | [-1.2%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.2 ms | 92.5 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 63.5 ms | 63.4 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.8 ms | 41.9 ms | -2.0% | [-3.4%, -0.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 107 ms | 108 ms | +1.4% | [+0.5%, +2.2%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 75.2 ms | 74.9 ms | -0.4% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 295 ms | 296 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 296 ms | 296 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 86.9 ms | 86.5 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 346 ms | 347 ms | +0.1% | [-0.7%, +0.8%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 349 ms | 350 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 167 ms | 167 ms | -0.4% | [-1.3%, +0.5%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 191 ms | 191 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 144 ms | 146 ms | +0.8% | [-0.6%, +2.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 90.1 ms | 89.7 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ | surface/leaf 1.62x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 130 ms | 130 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 72.8 ms | 72.6 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ | surface/leaf 1.79x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 62.1 ms | 61.5 ms | -1.0% | [-2.4%, +0.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.2 ms | 35.0 ms | +2.2% | [+0.5%, +3.8%] | +0.0% | ~ | surface/hit 1.76x (was 1.82x) |
| `SurfaceParts/boxes/closestSurface/double/host` | 144 ms | 145 ms | +0.6% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 90.7 ms | 90.9 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 147 ms | 145 ms | -1.6% | [-3.1%, +0.1%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 82.7 ms | 81.8 ms | -1.1% | [-2.5%, +0.3%] | +0.0% | ~ | surface/leaf 1.77x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 69.2 ms | 69.5 ms | +0.6% | [+0.4%, +0.7%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 39.6 ms | 39.4 ms | -0.6% | [-1.4%, +0.3%] | +0.0% | ~ | surface/hit 1.77x |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.8 ms | -1.2% | [-1.3%, -1.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.59 ms | 4.62 ms | +0.6% | [-0.1%, +1.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.0 ms | 24.8 ms | -1.1% | [-1.7%, -0.4%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.0 ms | 39.9 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.76 ms | 1.76 ms | +0.1% | [-2.3%, +2.5%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 131 µs | 133 µs | +1.5% | [-2.0%, +5.2%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.8 ms | 29.9 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.67 ms | 5.70 ms | +0.6% | [-0.1%, +1.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.4 ms | 16.3 ms | -0.6% | [-2.1%, +1.0%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.74 ms | 4.74 ms | -0.0% | [-0.6%, +0.6%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.2 ms | +0.8% | [+0.6%, +1.0%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.62 ms | 4.65 ms | +0.8% | [-1.5%, +3.0%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.5 ms | 26.7 ms | +0.7% | [-0.6%, +2.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.0 ms | 41.8 ms | -0.5% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.79 ms | 1.80 ms | +0.3% | [-3.0%, +3.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 283 µs | 286 µs | +1.1% | [-4.6%, +7.3%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.1 ms | 29.8 ms | -0.7% | [-2.1%, +0.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.77 ms | 7.81 ms | +0.5% | [-0.4%, +1.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.5 ms | 16.4 ms | -0.5% | [-1.3%, +0.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.78 ms | 7.78 ms | -0.0% | [-1.0%, +1.0%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.9 ms | 42.1 ms | +0.7% | [+0.5%, +0.8%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 200 ms | 217 ms | +8.2% | [+7.9%, +8.6%] | +0.0% | minor |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 42.8 ms | 43.5 ms | +1.8% | [+1.6%, +2.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 213 ms | 214 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `Weld/particles/region/float/host` | - | - | - | - | - | new |  |
| `Weld/particles/regionDistances/float/host` | - | - | - | - | - | new |  |
| `Weld/blocks/region/float/host` | - | - | - | - | - | new |  |
| `Weld/blocks/closestSurface/float/host` | - | - | - | - | - | new |  |
| `Weld/blocks/regionChanges/float/host` | - | - | - | - | - | new |  |
| `Weld/particles/region/double/host` | - | - | - | - | - | new |  |
| `Weld/particles/regionDistances/double/host` | - | - | - | - | - | new |  |
| `Weld/blocks/region/double/host` | - | - | - | - | - | new |  |
| `Weld/blocks/closestSurface/double/host` | - | - | - | - | - | new |  |
| `Weld/blocks/regionChanges/double/host` | - | - | - | - | - | new |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 957.7 s → 810.9 s (-15.3%), the largest peak 1618 MB → 1621 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.6 s | 9.8 s | -27.5% | 609 MB | 607 MB | -0.4% | ~ |
| `BVHRefit` | 7.5 s | 4.4 s | -41.6% | 516 MB | 518 MB | +0.5% | ~ |
| `BVHUnion` | 21.8 s | 19.2 s | -11.9% | 942 MB | 964 MB | +2.3% | ~ |
| `BVHUnionMixed` | 9.6 s | 7.8 s | -18.7% | 591 MB | 585 MB | -1.1% | ~ |
| `ClosestPrimitive` | 38.5 s | 32.5 s | -15.7% | 1323 MB | 1315 MB | -0.6% | ~ |
| `ClosestSurface` | 36.3 s | 29.8 s | -17.9% | 1343 MB | 1337 MB | -0.4% | ~ |
| `ConnectedComponents` | 21.8 s | 16.4 s | -24.6% | 868 MB | 879 MB | +1.2% | ~ |
| `Domain` | 24.4 s | 19.0 s | -21.9% | 1009 MB | 1025 MB | +1.6% | ~ |
| `DomainObject` | 18.9 s | 16.9 s | -10.9% | 897 MB | 914 MB | +1.9% | ~ |
| `EBGrid` | 15.9 s | 14.4 s | -9.5% | 793 MB | 823 MB | +3.8% | ~ |
| `Fused` | 31.1 s | 26.0 s | -16.4% | 1327 MB | 1273 MB | -4.0% | ~ |
| `LaunchTuning` | 50.1 s | 39.1 s | -22.1% | 1336 MB | 1320 MB | -1.2% | ~ |
| `Mesh` | 17.9 s | 16.2 s | -9.3% | 836 MB | 827 MB | -1.0% | ~ |
| `MeshCSG` | 56.6 s | 42.8 s | -24.5% | 1332 MB | 1320 MB | -0.8% | ~ |
| `MeshClosestPoint` | 16.4 s | 14.3 s | -13.1% | 834 MB | 825 MB | -1.0% | ~ |
| `MeshIngest` | 7.8 s | 7.2 s | -8.4% | 622 MB | 608 MB | -2.4% | ~ |
| `MeshReconcile` | 8.0 s | 6.9 s | -13.9% | 626 MB | 623 MB | -0.5% | ~ |
| `MeshTape` | 51.6 s | 44.1 s | -14.5% | 1344 MB | 1350 MB | +0.4% | ~ |
| `PointCloud` | 9.8 s | 7.5 s | -23.9% | 594 MB | 587 MB | -1.2% | ~ |
| `PointCloudBVHBuild` | 4.5 s | 3.9 s | -12.3% | 480 MB | 483 MB | +0.5% | ~ |
| `PointCloudHashGridBuild` | 4.2 s | 3.3 s | -21.2% | 456 MB | 457 MB | +0.3% | ~ |
| `QueryOrder` | 14.1 s | 11.5 s | -18.1% | 724 MB | 733 MB | +1.4% | ~ |
| `Ray` | 45.7 s | 38.7 s | -15.2% | 1342 MB | 1327 MB | -1.1% | ~ |
| `RayCSG` | 40.6 s | 34.2 s | -15.8% | 1282 MB | 1260 MB | -1.8% | ~ |
| `RayCrossing` | 62.0 s | 48.7 s | -21.5% | 1459 MB | 1471 MB | +0.8% | ~ |
| `RayParticles` | 63.0 s | 51.6 s | -18.1% | 1479 MB | 1491 MB | +0.8% | ~ |
| `RayShell` | 50.0 s | 40.0 s | -20.2% | 1329 MB | 1316 MB | -1.0% | ~ |
| `Regions` | 34.4 s | 31.4 s | -8.7% | 1254 MB | 1263 MB | +0.7% | ~ |
| `Render` | 40.5 s | 38.9 s | -4.0% | 1331 MB | 1318 MB | -1.0% | ~ |
| `ShapeFamilies` | 10.5 s | 8.6 s | -17.8% | 618 MB | 614 MB | -0.6% | ~ |
| `Shapes` | 9.4 s | 8.9 s | -5.0% | 621 MB | 586 MB | -5.7% | ~ |
| `SphereCloudBVH` | 5.2 s | 4.9 s | -6.0% | 533 MB | 535 MB | +0.4% | ~ |
| `SphereCloudRays` | 31.2 s | 30.1 s | -3.5% | 1218 MB | 1229 MB | +0.9% | ~ |
| `SphereContacts` | 4.6 s | 4.4 s | -4.9% | 499 MB | 501 MB | +0.6% | ~ |
| `SurfaceParts` | 45.4 s | 43.9 s | -3.3% | 1618 MB | 1621 MB | +0.1% | ~ |
| `Tape` | 18.5 s | 17.8 s | -3.8% | 948 MB | 963 MB | +1.6% | ~ |
| `TapeBVHUnion` | 16.2 s | 15.9 s | -1.7% | 840 MB | 849 MB | +1.1% | ~ |
| `Weld` | - | 28.6 s | | - | 1239 MB | | new |
| **all, in both** | 957.7 s | 810.9 s | -15.3% | 1618 MB | 1621 MB | +0.1% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (4 cases): PointCloudHashGrid/sphere/build/float/host, Ray/csg-fused/firstHit/float/host, Shapes/cut-sphere/signedDistance/float/host, TapeBVHUnion/maps64-as-written/signedDistance/float/host

The probes differ between the sides (each side ran its own): Performance/Weld/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 235ade11
base-fingerprint: 628a0fbb74054238a176bfe2ff0d5ea2ed942bd9
head: 9963dc61
head-fingerprint: 4164bfeff5de8a073749d06c618e84dec51da9d5
cases: 448
rounds: 2
reps: 3
date: 2026-11-27
-->

## Device comparison (CUDA): green

**Accepted:** -

`9963dc61` against `235ade11` (235ade11), 2026-11-27.

**VERDICT: no regression (0 major, 0 minor across 448 cases)**

Performance comparison: 0 major, 0 minor, 3 faster, 0 differ, 0 lost, 448 cases

- base: 235ade11 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 9963dc61 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.60 ms | 1.61 ms | +0.3% | [-0.3%, +1.0%] | - | ~ | device 22.6x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.41 ms | 3.42 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 18.5x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.07 ms | 1.06 ms | -1.3% | [-3.1%, +0.5%] | - | ~ | device 19.4x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.30 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.16 ms | 6.22 ms | +0.9% | [-0.2%, +1.9%] | - | ~ | device 7.4x faster than the host (was 8.2x) |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.8 ms | 11.9 ms | +0.4% | [-0.4%, +1.3%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.3 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 56.9 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.5 ms | 20.7 ms | +0.6% | [-0.0%, +1.3%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.1 ms | 47.8 ms | -0.7% | [-1.1%, -0.3%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.7 ms | 61.7 ms | -0.1% | [-2.5%, +2.3%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 110 ms | 111 ms | +0.2% | [-1.0%, +1.5%] | - | ~ | device 0.6x faster than the host |
| `BVHRefit/uniform/refit/float/hostdevice` | 8.29 ms | 8.13 ms | -1.9% | [-2.8%, -1.1%] | +0.0% | ~ | device 5.0x faster than the host |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.80 ms | 9.73 ms | -0.7% | [-1.5%, +0.2%] | +0.0% | ~ | device 8.4x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 387 µs | 387 µs | -0.1% | [-0.5%, +0.4%] | - | ~ | device 15.2x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 842 µs | 845 µs | +0.3% | [+0.0%, +0.6%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.52 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.64 ms | 4.67 ms | +0.7% | [+0.5%, +1.0%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 582 µs | 584 µs | +0.4% | [-0.1%, +0.9%] | - | ~ | device 12.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.78 ms | 6.78 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.00 ms | 7.01 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 905 µs | 903 µs | -0.3% | [-0.5%, -0.1%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.23 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.44 ms | -0.4% | [-0.8%, +0.0%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 848 µs | 848 µs | +0.0% | [-0.2%, +0.2%] | - | ~ | device 13.5x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.06 ms | 2.07 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 8.0x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.08 ms | 9.09 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.85 ms | 9.83 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 314 µs | 314 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 11.6x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.41 ms | 1.41 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 5.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.2 ms | 59.2 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.30 ms | 1.30 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.43 ms | 5.43 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 516 ms | 522 ms | +1.1% | [-0.1%, +2.3%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.5 ms | 20.5 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 5.5x faster than the host (was 4.7x) |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.42 ms | 8.42 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 4.8x faster than the host (was 4.6x); surface/leaf 2.43x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.6 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 8.2x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.20 ms | 6.20 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 10.0x faster than the host; surface/leaf 2.51x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 146 ms | 147 ms | +1.0% | [-0.9%, +2.8%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.0 ms | 60.1 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.45x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 71.9 ms | 71.6 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.4x faster than the host (was 2.3x); surface/leaf 2.56x |
| `Domain/particles/labels/float/hostdevice` | 3.71 ms | 3.75 ms | +1.0% | [+0.3%, +1.6%] | - | ~ | device 4.0x faster than the host (was 3.7x) |
| `Domain/particles/labelAt/float/hostdevice` | 2.91 ms | 2.91 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 2.5x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 114 µs | 115 µs | +0.8% | [-1.3%, +2.9%] | - | ~ | device 12.8x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.78 ms | 7.57 ms | -2.6% | [-4.0%, -1.2%] | - | ~ | device 1.9x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.32 ms | 3.31 ms | -0.3% | [-0.4%, -0.1%] | - | ~ | device 2.2x faster than the host |
| `Domain/particles/surfaceGroups/double/hostdevice` | 116 µs | 119 µs | +2.1% | [+0.7%, +3.6%] | - | ~ | device 12.4x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 12.1 ms | 12.3 ms | +1.2% | [+0.9%, +1.5%] | - | ~ | device 3.7x faster than the host |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 9.51 ms | 9.48 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 240 µs | 247 µs | +2.6% | [+1.5%, +3.7%] | - | ~ | device 2.6x faster than the host (was 2.7x); whole/own 49.71x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 170 µs | 171 µs | +0.7% | [-0.3%, +1.7%] | - | ~ | device 2.1x faster than the host; whole/own 55.38x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 19.4 ms | 19.4 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 2.4x faster than the host |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 10.8 ms | 10.8 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 2.3x faster than the host |
| `DomainObject/partsOwn/labels/double/hostdevice` | 396 µs | 403 µs | +1.7% | [+0.5%, +4.8%] | - | ~ | device 1.7x faster than the host; whole/own 48.21x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 229 µs | 229 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.5x faster than the host; whole/own 47.09x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/hostdevice` | 10.4 ms | 10.5 ms | +1.1% | [+0.2%, +2.0%] | +0.0% | ~ | device 9.9x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/hostdevice` | 10.4 ms | 10.3 ms | -0.1% | [-2.3%, +2.1%] | +0.0% | ~ | device 9.3x faster than the host; grid/morton 1.01x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/hostdevice` | 17.4 ms | 17.5 ms | +0.7% | [-0.4%, +1.7%] | +0.0% | ~ | device 6.9x faster than the host; grid/shuffled 0.60x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/hostdevice` | 223 ms | 224 ms | +0.8% | [-1.6%, +3.3%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/hostdevice` | 165 ms | 168 ms | +2.0% | [-0.1%, +4.2%] | +0.0% | ~ | device 0.6x faster than the host; grid/morton 1.33x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/hostdevice` | 290 ms | 290 ms | +0.1% | [-0.8%, +1.0%] | +0.0% | ~ | device 0.4x faster than the host; grid/shuffled 0.77x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/hostdevice` | 27.1 ms | 27.0 ms | -0.5% | [-0.6%, -0.4%] | +0.0% | ~ | device 4.0x faster than the host |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/hostdevice` | 29.1 ms | 29.2 ms | +0.5% | [+0.5%, +0.6%] | +0.0% | ~ | device 3.6x faster than the host (was 3.4x); grid/morton 0.92x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/hostdevice` | 60.7 ms | 61.1 ms | +0.5% | [+0.3%, +0.8%] | +0.0% | ~ | device 2.4x faster than the host; grid/shuffled 0.44x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/hostdevice` | 205 ms | 203 ms | -1.0% | [-3.4%, +1.5%] | +0.0% | ~ | device 0.5x faster than the host |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/hostdevice` | 158 ms | 158 ms | -0.2% | [-2.3%, +1.9%] | +0.0% | ~ | device 0.6x faster than the host; grid/morton 1.29x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/hostdevice` | 251 ms | 249 ms | -0.8% | [-2.0%, +0.4%] | +0.0% | ~ | device 0.7x faster than the host; grid/shuffled 0.82x |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 23.9x faster than the host; tape/fused 4.77x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.7 µs | 49.8 µs | +0.2% | [-0.2%, +0.6%] | - | ~ | device 42.7x faster than the host; tape/fused 7.62x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.3% | [-0.2%, +0.8%] | - | ~ | device 9.0x faster than the host; tape/fused 10.61x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.4 ms | 17.4 ms | -0.2% | [-1.2%, +0.9%] | - | ~ | device 6.7x faster than the host; tape/fused 1.81x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.4 ms | 31.4 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 4.1x faster than the host (was 4.8x) |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.56 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.5x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 283 ms | 283 ms | -0.2% | [-1.3%, +1.0%] | - | ~ | device 0.4x faster than the host; tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 284 ms | 282 ms | -0.8% | [-1.6%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.22 ms | 8.23 ms | +0.1% | [-1.2%, +1.4%] | +0.0% | ~ | device 10.8x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.32 ms | 8.27 ms | -0.6% | [-1.2%, +0.1%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.22 ms | 8.21 ms | -0.0% | [-0.7%, +0.6%] | +0.0% | ~ | device 11.9x faster than the host (was 10.7x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.30 ms | 8.27 ms | -0.3% | [-1.5%, +0.9%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 686 µs | 686 µs | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.76 ms | 1.75 ms | -0.3% | [-0.3%, -0.2%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.06 ms | 1.06 ms | +0.2% | [-0.9%, +1.3%] | +0.0% | ~ | device 2.4x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 800 µs | 797 µs | -0.4% | [-0.4%, -0.3%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 739 µs | 744 µs | +0.7% | [+0.2%, +1.1%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 835 µs | 834 µs | -0.1% | [-1.3%, +1.1%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 743 µs | 743 µs | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 723 µs | 733 µs | +1.4% | [+0.7%, +2.0%] | +0.0% | ~ | device 11.9x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 161 ms | 162 ms | +0.8% | [-2.0%, +3.6%] | +0.0% | ~ | device 0.5x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 159 ms | 158 ms | -0.3% | [-2.2%, +1.7%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 163 ms | 161 ms | -1.6% | [-2.1%, -1.1%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 161 ms | 164 ms | +1.7% | [+1.3%, +2.0%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [+0.0%, +0.1%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.45 ms | 1.45 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | device 1.8x faster than the host; default/this 0.96x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.91 ms | 1.93 ms | +1.2% | [+0.7%, +1.7%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.72x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.56 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.25 ms | 7.26 ms | +0.2% | [-0.5%, +0.8%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.32 ms | 7.29 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.23 ms | 7.23 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.28 ms | 7.31 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.37 ms | 8.47 ms | +1.1% | [+0.6%, +1.6%] | - | ~ | device 11.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.8 ms | 16.6 ms | -0.7% | [-1.2%, -0.2%] | - | ~ | device 7.1x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 165 ms | 166 ms | +1.0% | [+0.6%, +1.4%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 276 ms | 280 ms | +1.5% | [-1.3%, +4.5%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.7 ms | 24.4 ms | -1.2% | [-1.9%, -0.6%] | - | ~ | device 4.4x faster than the host (was 4.0x) |
| `MeshSDF/query-random/float/hostdevice` | 58.0 ms | 58.0 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.5x faster than the host (was 2.7x) |
| `MeshSDF/query-morton/double/hostdevice` | 162 ms | 156 ms | -3.7% | [-5.2%, -2.2%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `MeshSDF/query-random/double/hostdevice` | 245 ms | 245 ms | +0.2% | [-1.1%, +1.5%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.06 ms | 2.05 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 5.0x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.46 ms | 3.45 ms | -0.4% | [-0.8%, +0.1%] | - | ~ | device 4.0x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.19 ms | 6.20 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 4.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 65.0 ms | 64.2 ms | -1.2% | [-2.0%, -0.4%] | - | ~ | device 3.1x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 94.4 ms | 93.9 ms | -0.5% | [-2.1%, +1.1%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 235 ms | 241 ms | +2.4% | [+1.7%, +3.0%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.8 ms | 24.8 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 2.0x faster than the host; tape/fused 2.60x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.1 ms | 27.0 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 2.2x faster than the host; tape/fused 3.48x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 66.8 ms | 65.6 ms | -1.7% | [-2.3%, -1.1%] | - | ~ | device 3.9x faster than the host; tape/fused 3.67x (was 3.52x) |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.7 ms | 21.9 ms | +1.0% | [-1.6%, +3.8%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 38.7 ms | 38.8 ms | +0.5% | [-1.1%, +2.1%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 41.5 ms | 41.3 ms | -0.5% | [-1.4%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 537 ms | 551 ms | +2.6% | [-2.0%, +7.4%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 865 ms | 862 ms | -0.3% | [-3.0%, +2.5%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.95 s | 2.00 s | +2.8% | [+2.3%, +3.3%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 205 ms | 200 ms | -2.8% | [-8.7%, +3.2%] | - | ~ | device 0.2x faster than the host (was 0.2x); tape/fused 2.76x (was 2.61x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 296 ms | 300 ms | +1.4% | [-0.7%, +3.5%] | - | ~ | device 0.2x faster than the host; tape/fused 2.87x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 535 ms | 532 ms | -0.6% | [-4.4%, +3.2%] | - | ~ | device 0.5x faster than the host; tape/fused 3.76x (was 3.64x) |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | 12.1 ms | 12.0 ms | -0.6% | [-1.0%, -0.2%] | - | ~ | device 9.9x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | 58.5 ms | 58.6 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.5x faster than the host |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | 257 ms | 262 ms | +2.1% | [+0.7%, +3.4%] | - | ~ | device 0.5x faster than the host |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | 240 ms | 240 ms | -0.1% | [-2.2%, +2.1%] | - | ~ | device 0.7x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ | device 15.8x faster than the host |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.30 ms | 5.31 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 10.6x faster than the host |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.31 ms | 1.32 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 15.7x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.32 ms | 1.32 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 15.6x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.15 ms | 2.13 ms | -0.8% | [-2.0%, +0.3%] | - | ~ | device 7.9x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.51 ms | 3.52 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 10.2x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.77 ms | 2.77 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 7.7x faster than the host; mesh/tape 0.47x |
| `MeshTape/tape/frame/float/hostdevice` | 4.47 ms | 4.46 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 4.9x faster than the host; mesh/tape 0.48x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.75 ms | 3.75 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 9.7x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.71 ms | 5.75 ms | +0.6% | [+0.2%, +1.1%] | - | ~ | device 4.1x faster than the host; mesh/view 0.37x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 30.3 ms | 30.3 ms | +0.2% | [-0.7%, +1.0%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 30.2 ms | 30.1 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 25.9 ms | 25.7 ms | -0.8% | [-1.8%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 43.6 ms | 43.4 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 30.5 ms | 30.5 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 33.4 ms | 32.9 ms | -1.6% | [-2.4%, -0.7%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.78x |
| `MeshTape/view/firstHit/double/hostdevice` | 43.3 ms | 43.5 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 0.9x faster than the host; mesh/view 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 35.9 ms | 36.1 ms | +0.5% | [-0.0%, +1.1%] | - | ~ | device 0.7x faster than the host; mesh/view 0.71x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 684 µs | 683 µs | -0.2% | [-0.5%, +0.2%] | - | ~ | device 11.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 1.82 ms | 1.82 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 996 µs | 996 µs | +0.1% | [-0.5%, +0.6%] | - | ~ | device 9.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 4.61 ms | 4.60 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 2.98 ms | 2.96 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.11 ms | 7.08 ms | -0.3% | [-1.1%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.77 ms | 7.78 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.3 ms | 13.3 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.9 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.44 ms | 1.46 ms | +0.8% | [-0.3%, +1.9%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.89 ms | 3.87 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 526 µs | 523 µs | -0.6% | [-0.8%, -0.3%] | - | ~ | device 15.9x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.53 ms | 4.51 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 6.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.16 ms | 4.19 ms | +0.6% | [-0.9%, +2.2%] | - | ~ | device 5.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.17 ms | 8.12 ms | -0.7% | [-2.4%, +0.9%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.8 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.01 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.5 ms | 17.6 ms | +0.6% | [+0.0%, +1.1%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.2 ms | 14.9 ms | -1.5% | [-2.4%, -0.5%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.58 ms | 1.60 ms | +1.2% | [+0.5%, +1.8%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.60 ms | 4.61 ms | +0.1% | [-0.4%, +0.7%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 832 µs | 833 µs | +0.1% | [-0.3%, +0.5%] | - | ~ | device 10.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 2.86 ms | 2.86 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 6.9x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.14 ms | 4.12 ms | -0.5% | [-0.7%, -0.2%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.5 ms | 16.4 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.1 ms | 21.0 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.96 ms | 8.94 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.6 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.2 ms | 20.2 ms | +0.0% | [-0.7%, +0.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.77 ms | 2.75 ms | -0.5% | [-1.8%, +0.7%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.73 ms | 6.73 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.71 ms | 2.71 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 7.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.40 ms | 6.40 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 3.0x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.8 ms | 33.7 ms | -0.3% | [-1.0%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.5 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.5 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.7 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.0 ms | 38.4 ms | +0.9% | [+0.2%, +1.7%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.7 ms | 12.8 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.67 ms | 9.65 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.8 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.27 ms | 3.27 ms | -0.0% | [-0.9%, +0.9%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.56 ms | 2.58 ms | +0.9% | [+0.4%, +1.4%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.59 ms | 4.56 ms | -0.8% | [-1.1%, -0.6%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.05 ms | 3.05 ms | +0.1% | [-0.9%, +1.0%] | +0.0% | ~ | device 3.3x faster than the host |
| `QueryOrder/random/mortonOrder/float/hostdevice` | 16.8 ms | 16.9 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 3.5x faster than the host |
| `QueryOrder/armadillo/distance-arrived/float/hostdevice` | 9.15 ms | 9.06 ms | -1.0% | [-1.6%, -0.4%] | +0.0% | ~ | device 5.0x faster than the host |
| `QueryOrder/armadillo/distance-ordered/float/hostdevice` | 7.43 ms | 7.36 ms | -0.9% | [-1.8%, +0.1%] | +0.0% | ~ | device 5.4x faster than the host; arrived/ordered 1.23x |
| `QueryOrder/random/mortonOrder/double/hostdevice` | 17.6 ms | 17.5 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 3.4x faster than the host |
| `QueryOrder/armadillo/distance-arrived/double/hostdevice` | 112 ms | 113 ms | +0.8% | [+0.1%, +1.5%] | +0.0% | ~ | device 0.4x faster than the host |
| `QueryOrder/armadillo/distance-ordered/double/hostdevice` | 81.4 ms | 80.8 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ | device 0.5x faster than the host; arrived/ordered 1.40x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 707 µs | 708 µs | +0.1% | [-0.6%, +0.9%] | - | ~ | device 17.2x faster than the host; hand/api 0.93x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.32 ms | 1.31 ms | -0.5% | [-1.0%, -0.1%] | - | ~ | device 15.2x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 654 µs | 657 µs | +0.4% | [-0.5%, +1.3%] | - | ~ | device 17.3x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.24 ms | 1.24 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 620 µs | 628 µs | +1.3% | [+0.9%, +1.7%] | - | ~ | device 15.7x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.16 ms | 1.16 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 14.6x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.20 ms | +0.6% | [-0.1%, +1.4%] | - | ~ | device 11.8x faster than the host; triangles/polygons 0.59x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.48 ms | +0.4% | [+0.0%, +0.7%] | - | ~ | device 9.6x faster than the host; triangles/polygons 0.53x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.5% | [+0.0%, +1.0%] | - | ~ | device 11.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.17 ms | 2.18 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 617 µs | 619 µs | +0.3% | [-6.0%, +7.1%] | - | ~ | device 2.4x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 701 µs | 703 µs | +0.2% | [-9.9%, +11.5%] | - | ~ | device 2.6x faster than the host; tape/fused 1.18x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 670 µs | 669 µs | -0.1% | [-0.4%, +0.1%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 834 µs | 833 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 18.6 ms | 18.7 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 0.7x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 30.2 ms | 30.2 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 0.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 18.0 ms | 18.0 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 29.1 ms | 29.1 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.8 ms | 15.9 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 26.0 ms | 26.0 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 17.1 ms | 17.2 ms | +0.8% | [+0.3%, +1.3%] | - | ~ | device 0.8x faster than the host; triangles/polygons 1.08x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.5 ms | 25.4 ms | -0.5% | [-0.6%, -0.4%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.19x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.2 ms | 15.0 ms | -1.2% | [-2.0%, -0.5%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 23.0 ms | -0.4% | [-1.1%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.24 ms | -0.4% | [-1.2%, +0.4%] | - | ~ | device 1.3x faster than the host; tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.36 ms | 2.37 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.40 ms | 1.39 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.50 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 2.9x faster than the host; marched/traced 3.54x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.98 ms | 2.98 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.7x faster than the host; marched/traced 4.65x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.03 ms | 8.99 ms | -0.4% | [-0.6%, -0.3%] | - | ~ | device 4.2x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.9 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.25 ms | 2.25 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.8x faster than the host; marched/traced 12.88x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.51 ms | 5.52 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 2.9x faster than the host (was 2.8x); marched/traced 9.84x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 28.9 ms | 29.0 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 54.2 ms | 54.3 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 2.1x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.33 ms | 5.32 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 3.0x faster than the host (was 2.9x) |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.93 ms | 8.93 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.2x faster than the host (was 3.1x) |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.15 ms | 6.14 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 1.1x faster than the host (was 1.3x); marched/traced 25.81x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 0.7x faster than the host (was 0.8x); marched/traced 24.29x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 161 ms | 159 ms | -1.8% | [-2.4%, -1.2%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 257 ms | 257 ms | +0.2% | [-1.1%, +1.5%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.85 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 23.15x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.5% | [+0.2%, +0.9%] | - | ~ | device 1.3x faster than the host; marched/traced 17.46x (was 18.13x) |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 111 ms | 112 ms | +1.1% | [-2.9%, +5.3%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 221 ms | 214 ms | -3.1% | [-3.9%, -2.4%] | - | faster | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.3 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.8 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 709 µs | 708 µs | -0.2% | [-0.8%, +0.4%] | - | ~ | device 17.1x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.32 ms | 1.32 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 15.1x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 700 µs | 708 µs | +1.2% | [-0.1%, +2.4%] | - | ~ | device 17.1x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.32 ms | 1.31 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 15.4x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 678 µs | 678 µs | -0.1% | [-0.5%, +0.4%] | - | ~ | device 3.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 831 µs | 835 µs | +0.5% | [+0.3%, +0.6%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 615 µs | 614 µs | -0.1% | [-0.5%, +0.2%] | - | ~ | device 3.2x faster than the host; hit/crossing 1.10x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 640 µs | 639 µs | -0.0% | [-0.4%, +0.3%] | - | ~ | device 3.7x faster than the host; hit/crossing 1.31x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.79 ms | 4.78 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.3 ms | 10.3 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 6.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.23 ms | 4.24 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 9.9x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.15 ms | 9.15 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 57.5 ms | 57.3 ms | -0.5% | [-1.0%, +0.1%] | - | ~ | device 3.0x faster than the host (was 3.2x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 83.1 ms | 82.7 ms | -0.5% | [-1.2%, +0.3%] | - | ~ | device 2.5x faster than the host (was 2.3x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 49.5 ms | 49.2 ms | -0.6% | [-1.4%, +0.3%] | - | ~ | device 2.9x faster than the host; hit/crossing 1.16x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 64.3 ms | 64.4 ms | +0.2% | [-0.7%, +1.0%] | - | ~ | device 2.7x faster than the host (was 2.4x); hit/crossing 1.28x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 18.8 ms | 18.7 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 30.2 ms | 30.2 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 18.6 ms | 18.9 ms | +1.2% | [+0.6%, +1.7%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 30.3 ms | 30.2 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.51 ms | 2.50 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.11 ms | 1.11 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 1.8x faster than the host; hit/crossing 1.25x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.17 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.14x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 68.8 ms | 68.9 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 130 ms | 129 ms | -0.5% | [-1.4%, +0.3%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 62.0 ms | 62.1 ms | +0.2% | [-0.0%, +0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.11x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 118 ms | 119 ms | +1.0% | [+0.3%, +1.8%] | - | ~ | device 0.5x faster than the host; hit/crossing 1.08x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 490 ms | 475 ms | -3.2% | [-8.2%, +2.2%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 712 ms | 722 ms | +1.4% | [+0.9%, +1.8%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 412 ms | 425 ms | +3.3% | [-3.1%, +9.8%] | - | ~ | device 0.3x faster than the host (was 0.4x); hit/crossing 1.12x (was 1.19x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 523 ms | 522 ms | -0.3% | [-1.3%, +0.8%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.38x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.55 ms | 3.62 ms | +2.0% | [+0.7%, +3.6%] | - | ~ | device 1.2x faster than the host; marched/traced 3.35x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.77 ms | 4.77 ms | -0.2% | [-1.7%, +1.4%] | - | ~ | device 0.9x faster than the host; marched/traced 5.60x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.1 ms | +0.8% | [+0.4%, +1.3%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.7 ms | 26.7 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.23 ms | 1.25 ms | +1.6% | [-1.0%, +4.2%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.86 ms | 1.85 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 96.0 µs | 98.6 µs | +2.6% | [+1.4%, +3.9%] | - | ~ | device 15.0x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 155 µs | 155 µs | -0.2% | [-0.9%, +0.6%] | - | ~ | device 10.5x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 802 µs | 802 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 940 µs | 942 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.6x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.0 ms | 34.9 ms | -0.2% | [-2.2%, +1.8%] | - | ~ | device 0.1x faster than the host; marched/traced 3.51x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.8 ms | 36.5 ms | -0.7% | [-1.5%, +0.1%] | - | ~ | device 0.1x faster than the host; marched/traced 6.38x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 119 ms | 122 ms | +2.6% | [+1.5%, +3.6%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 236 ms | 233 ms | -1.1% | [-2.8%, +0.5%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.2 ms | 16.3 ms | +0.1% | [-7.1%, +7.6%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.6 ms | 21.8 ms | +1.0% | [-0.0%, +2.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.15 ms | 1.18 ms | +2.5% | [+0.2%, +4.8%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.49 ms | 2.51 ms | +0.8% | [+0.0%, +1.6%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 5.98 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.73 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.2 ms | 23.2 ms | +0.1% | [-1.2%, +1.4%] | - | ~ | device 2.2x faster than the host; marched/traced 2.70x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.4 ms | 28.2 ms | -0.6% | [-1.2%, +0.0%] | - | ~ | device 2.0x faster than the host; marched/traced 3.78x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.6 ms | 62.7 ms | +0.2% | [-0.6%, +1.0%] | - | ~ | device 4.4x faster than the host (was 3.6x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 105 ms | 107 ms | +1.8% | [-0.6%, +4.3%] | - | ~ | device 2.3x faster than the host (was 2.5x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.8 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.5 ms | +0.8% | [+0.3%, +1.3%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 313 ms | 309 ms | -1.5% | [-4.5%, +1.5%] | - | ~ | device 0.2x faster than the host; marched/traced 1.84x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 341 ms | 347 ms | +1.9% | [+1.3%, +2.6%] | - | ~ | device 0.2x faster than the host; marched/traced 2.85x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 562 ms | 570 ms | +1.4% | [+0.8%, +2.0%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 981 ms | 990 ms | +1.0% | [-0.8%, +2.8%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 190 ms | 194 ms | +2.2% | [-0.8%, +5.4%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 240 ms | 248 ms | +3.2% | [-0.6%, +7.2%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `Regions/particles/region/float/hostdevice` | 2.80 ms | 2.80 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 2.7x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.31 ms | 3.33 ms | +0.5% | [+0.1%, +0.8%] | - | ~ | device 4.0x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 80.3 ms | 80.3 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 3.6x faster than the host; one-pass/each 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.42 ms | 3.40 ms | -0.6% | [-0.9%, -0.4%] | - | ~ | device 4.0x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.84 ms | 6.84 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 4.2x faster than the host (was 3.7x) |
| `Regions/assembly/firstHit/float/hostdevice` | 5.53 ms | 5.54 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.2x faster than the host; regions/hit 1.23x |
| `Regions/particles/region/double/hostdevice` | 3.55 ms | 3.55 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 2.1x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.94 ms | 5.94 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 128 ms | 125 ms | -2.7% | [-3.6%, -1.8%] | - | ~ | device 2.7x faster than the host; one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/hostdevice` | 6.06 ms | 6.09 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 2.2x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.8 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 12.9 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.4x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.46 ms | 2.47 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 2.9x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.45 ms | 2.45 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.13 ms | 2.13 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 7.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.65 ms | 5.66 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.65 ms | 5.66 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 25.8 ms | 25.7 ms | -0.1% | [-0.9%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `ShapeFamilies/icosahedron/signedDistance/float/hostdevice` | 394 µs | 393 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 17.0x faster than the host |
| `ShapeFamilies/pyramid/signedDistance/float/hostdevice` | 207 µs | 208 µs | +0.5% | [+0.2%, +0.8%] | - | ~ | device 22.7x faster than the host |
| `ShapeFamilies/box-frame/signedDistance/float/hostdevice` | 48.0 µs | 48.1 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 25.9x faster than the host |
| `ShapeFamilies/hyperboloid/signedDistance/float/hostdevice` | 270 µs | 270 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 18.6x faster than the host |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/float/hostdevice` | 277 µs | 277 µs | +0.0% | [-0.3%, +0.3%] | - | ~ | device 17.7x faster than the host |
| `ShapeFamilies/paraboloid/signedDistance/float/hostdevice` | 48.4 µs | 48.3 µs | -0.2% | [-0.8%, +0.4%] | - | ~ | device 65.8x faster than the host |
| `ShapeFamilies/superellipsoid/signedDistance/float/hostdevice` | 90.4 µs | 90.7 µs | +0.3% | [+0.1%, +0.4%] | - | ~ | device 40.7x faster than the host |
| `ShapeFamilies/solid-angle/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.0% | [-0.4%, +0.3%] | - | ~ | device 9.8x faster than the host |
| `ShapeFamilies/vesica/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.2% | [-0.0%, +0.4%] | - | ~ | device 18.0x faster than the host |
| `ShapeFamilies/helix/signedDistance/float/hostdevice` | 297 µs | 297 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 31.8x faster than the host |
| `ShapeFamilies/gyroid/signedDistance/float/hostdevice` | 48.1 µs | 48.1 µs | +0.1% | [-0.8%, +1.0%] | - | ~ | device 37.9x faster than the host |
| `ShapeFamilies/ray-forms/firstHit/float/hostdevice` | 2.26 ms | 2.27 ms | +0.7% | [+0.7%, +0.8%] | - | ~ | device 3.6x faster than the host |
| `ShapeFamilies/icosahedron/signedDistance/double/hostdevice` | 8.51 ms | 8.51 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `ShapeFamilies/pyramid/signedDistance/double/hostdevice` | 4.46 ms | 4.46 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `ShapeFamilies/box-frame/signedDistance/double/hostdevice` | 907 µs | 909 µs | +0.2% | [-0.1%, +0.4%] | - | ~ | device 1.5x faster than the host |
| `ShapeFamilies/hyperboloid/signedDistance/double/hostdevice` | 25.0 ms | 25.0 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 0.4x faster than the host |
| `ShapeFamilies/hyperboloid-two-sheets/signedDistance/double/hostdevice` | 25.2 ms | 25.2 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 0.3x faster than the host |
| `ShapeFamilies/paraboloid/signedDistance/double/hostdevice` | 1.77 ms | 1.77 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.2x faster than the host |
| `ShapeFamilies/superellipsoid/signedDistance/double/hostdevice` | 5.00 ms | 5.00 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.2x faster than the host |
| `ShapeFamilies/solid-angle/signedDistance/double/hostdevice` | 380 µs | 381 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 1.6x faster than the host |
| `ShapeFamilies/vesica/signedDistance/double/hostdevice` | 885 µs | 885 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.1x faster than the host |
| `ShapeFamilies/helix/signedDistance/double/hostdevice` | 14.6 ms | 14.6 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 0.9x faster than the host |
| `ShapeFamilies/gyroid/signedDistance/double/hostdevice` | 1.13 ms | 1.13 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 3.5x faster than the host |
| `ShapeFamilies/ray-forms/firstHit/double/hostdevice` | 10.4 ms | 10.5 ms | +0.2% | [+0.0%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `Shapes/ellipsoid/signedDistance/float/hostdevice` | 155 µs | 156 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 33.0x faster than the host |
| `Shapes/capped-cone/signedDistance/float/hostdevice` | 48.0 µs | 47.4 µs | -1.3% | [-2.6%, +0.2%] | - | ~ | device 18.7x faster than the host (was 18.1x) |
| `Shapes/round-cone/signedDistance/float/hostdevice` | 47.9 µs | 47.7 µs | -0.5% | [-0.8%, -0.2%] | - | ~ | device 14.8x faster than the host (was 15.7x) |
| `Shapes/polygon-prism/signedDistance/float/hostdevice` | 48.2 µs | 48.1 µs | -0.2% | [-0.6%, +0.3%] | - | ~ | device 65.2x faster than the host |
| `Shapes/tube/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.3% | [+0.0%, +0.7%] | - | ~ | device 16.1x faster than the host |
| `Shapes/capped-torus/signedDistance/float/hostdevice` | 48.1 µs | 48.0 µs | -0.3% | [-0.4%, -0.2%] | - | ~ | device 17.6x faster than the host |
| `Shapes/cut-sphere/signedDistance/float/hostdevice` | 48.0 µs | 47.8 µs | -0.3% | [-0.7%, +0.0%] | - | ~ | device 9.9x faster than the host (was 9.5x) |
| `Shapes/cut-hollow-sphere/signedDistance/float/hostdevice` | 47.8 µs | 47.6 µs | -0.4% | [-0.6%, -0.2%] | - | ~ | device 8.1x faster than the host (was 7.4x) |
| `Shapes/wedge/signedDistance/float/hostdevice` | 47.6 µs | 47.9 µs | +0.5% | [-0.2%, +1.2%] | - | ~ | device 8.1x faster than the host (was 8.7x) |
| `Shapes/ray-forms/firstHit/float/hostdevice` | 557 µs | 557 µs | +0.1% | [-0.6%, +0.8%] | - | ~ | device 8.1x faster than the host |
| `Shapes/ellipsoid/signedDistance/double/hostdevice` | 7.23 ms | 7.24 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `Shapes/capped-cone/signedDistance/double/hostdevice` | 492 µs | 492 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `Shapes/round-cone/signedDistance/double/hostdevice` | 421 µs | 421 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Shapes/polygon-prism/signedDistance/double/hostdevice` | 1.69 ms | 1.69 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Shapes/tube/signedDistance/double/hostdevice` | 358 µs | 358 µs | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.3x faster than the host |
| `Shapes/capped-torus/signedDistance/double/hostdevice` | 669 µs | 669 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host |
| `Shapes/cut-sphere/signedDistance/double/hostdevice` | 415 µs | 413 µs | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.3x faster than the host (was 1.4x) |
| `Shapes/cut-hollow-sphere/signedDistance/double/hostdevice` | 339 µs | 339 µs | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `Shapes/wedge/signedDistance/double/hostdevice` | 155 µs | 155 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 2.8x faster than the host (was 2.7x) |
| `Shapes/ray-forms/firstHit/double/hostdevice` | 8.02 ms | 8.04 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 891 µs | 886 µs | -0.6% | [-1.3%, +0.2%] | +0.0% | ~ | device 10.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.66 ms | 2.67 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ | device 4.7x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.6 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ | device 3.6x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.0 ms | 13.0 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.4 ms | 14.4 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 14.0 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ | device 3.9x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.01 ms | 2.99 ms | -0.7% | [-1.3%, +0.0%] | +0.0% | ~ | device 7.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.74 ms | 4.75 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | device 5.2x faster than the host; tape/fused 1.38x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.0 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.27x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 23.0 ms | 23.0 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.5 ms | 30.5 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 3.1x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.6 ms | 44.2 ms | -0.8% | [-1.8%, +0.3%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.8 ms | 45.7 ms | -0.3% | [-0.7%, +0.2%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 125 ms | 122 ms | -2.6% | [-2.8%, -2.4%] | +0.0% | ~ | device 0.7x faster than the host (was 0.7x); tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 130 ms | 125 ms | -3.9% | [-4.1%, -3.8%] | +0.0% | faster | device 0.8x faster than the host (was 0.8x); tape/fused 1.05x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 130 ms | 126 ms | -2.9% | [-3.1%, -2.6%] | +0.0% | ~ | device 0.7x faster than the host (was 0.7x) |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 135 ms | 132 ms | -2.4% | [-3.7%, -1.0%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 10.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.2 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ | device 0.6x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.31 ms | 9.31 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 11.3x faster than the host |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.92 ms | 5.92 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 9.9x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 98.1 ms | 96.8 ms | -1.3% | [-2.0%, -0.7%] | - | ~ | device 3.6x faster than the host (was 3.4x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.6 ms | 63.6 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 3.2x faster than the host (was 3.3x); surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.9 ms | 29.9 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 5.4x faster than the host (was 4.9x) |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.98 ms | 9.98 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 6.9x faster than the host; surface/hit 3.00x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.1 ms | 59.1 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.5x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 781 ms | 786 ms | +0.7% | [-0.2%, +1.5%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 528 ms | 530 ms | +0.5% | [-0.3%, +1.3%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.48x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 315 ms | 319 ms | +1.4% | [-0.3%, +3.1%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 128 ms | 128 ms | +0.1% | [-2.6%, +2.8%] | - | ~ | device 0.6x faster than the host; surface/hit 2.49x |
| `Tape/csg/signedDistance/float/hostdevice` | 229 µs | 229 µs | +0.3% | [-0.5%, +1.1%] | - | ~ | device 10.7x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.1% | [-0.1%, +0.4%] | - | ~ | device 23.4x faster than the host (was 24.1x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 379 µs | 379 µs | +0.1% | [+0.1%, +0.2%] | - | ~ | device 12.0x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.55 ms | -0.3% | [-0.7%, +0.2%] | - | ~ | device 9.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 75.4 µs | 75.6 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 7.0x faster than the host (was 6.7x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.6 µs | 47.9 µs | +0.7% | [+0.2%, +1.2%] | - | ~ | device 5.3x faster than the host (was 4.9x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 653 µs | 653 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.2% | [-0.0%, +0.4%] | - | ~ | device 11.5x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 508 µs | 508 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 7.2x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.3 µs | 48.1 µs | -0.4% | [-0.9%, +0.1%] | - | ~ | device 27.3x faster than the host (was 26.3x) |
| `Tape/csg/signedDistance/double/hostdevice` | 858 µs | 860 µs | +0.2% | [+0.1%, +0.2%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.73 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.64 ms | 6.64 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 192 µs | 192 µs | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.0x faster than the host (was 1.8x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.59 ms | 1.59 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 8.5x faster than the host (was 8.2x) |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.80 ms | 3.81 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 10.7x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.50 ms | 6.49 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.7 ms | 17.7 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.7x faster than the host (was 2.4x) |
| `Weld/particles/region/float/hostdevice` | - | - | - | - | - | new |  |
| `Weld/particles/regionDistances/float/hostdevice` | - | - | - | - | - | new |  |
| `Weld/blocks/region/float/hostdevice` | - | - | - | - | - | new |  |
| `Weld/blocks/closestSurface/float/hostdevice` | - | - | - | - | - | new |  |
| `Weld/blocks/regionChanges/float/hostdevice` | - | - | - | - | - | new |  |
| `Weld/particles/region/double/hostdevice` | - | - | - | - | - | new |  |
| `Weld/particles/regionDistances/double/hostdevice` | - | - | - | - | - | new |  |
| `Weld/blocks/region/double/hostdevice` | - | - | - | - | - | new |  |
| `Weld/blocks/closestSurface/double/hostdevice` | - | - | - | - | - | new |  |
| `Weld/blocks/regionChanges/double/hostdevice` | - | - | - | - | - | new |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 6644.6 s → 4719.8 s (-29.0%), the largest peak 5380 MB → 5378 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 17.3 s | 13.8 s | -20.2% | 634 MB | 636 MB | +0.2% | ~ |
| `BVHRefit` | 10.9 s | 7.3 s | -32.9% | 536 MB | 538 MB | +0.4% | ~ |
| `BVHUnion` | 34.7 s | 25.8 s | -25.5% | 1057 MB | 1059 MB | +0.1% | ~ |
| `BVHUnionMixed` | 16.2 s | 11.5 s | -28.9% | 652 MB | 654 MB | +0.3% | ~ |
| `ClosestPrimitive` | 180.1 s | 109.9 s | -39.0% | 1546 MB | 1548 MB | +0.2% | ~ |
| `ClosestSurface` | 208.0 s | 140.9 s | -32.2% | 1422 MB | 1425 MB | +0.2% | ~ |
| `ConnectedComponents` | 28.5 s | 22.3 s | -21.7% | 1047 MB | 1050 MB | +0.2% | ~ |
| `Domain` | 81.4 s | 54.3 s | -33.3% | 1121 MB | 1125 MB | +0.3% | ~ |
| `DomainObject` | 71.6 s | 52.1 s | -27.3% | 1056 MB | 1058 MB | +0.3% | ~ |
| `EBGrid` | 26.2 s | 20.5 s | -21.8% | 782 MB | 784 MB | +0.2% | ~ |
| `Fused` | 82.2 s | 53.9 s | -34.5% | 1276 MB | 1278 MB | +0.1% | ~ |
| `LaunchTuning` | 624.5 s | 450.5 s | -27.9% | 3080 MB | 3081 MB | +0.0% | ~ |
| `Mesh` | 30.2 s | 22.0 s | -27.3% | 840 MB | 841 MB | +0.2% | ~ |
| `MeshCSG` | 491.7 s | 361.8 s | -26.4% | 4935 MB | 4936 MB | +0.0% | ~ |
| `MeshClosestPoint` | 29.9 s | 19.5 s | -34.6% | 792 MB | 793 MB | +0.2% | ~ |
| `MeshIngest` | 18.1 s | 11.1 s | -38.4% | 566 MB | 568 MB | +0.3% | ~ |
| `MeshReconcile` | 15.1 s | 10.9 s | -27.7% | 544 MB | 545 MB | +0.3% | ~ |
| `MeshTape` | 919.9 s | 659.3 s | -28.3% | 5380 MB | 5378 MB | -0.0% | ~ |
| `PointCloud` | 20.0 s | 13.0 s | -35.2% | 596 MB | 598 MB | +0.3% | ~ |
| `PointCloudBVHBuild` | 11.5 s | 7.5 s | -35.0% | 536 MB | 538 MB | +0.4% | ~ |
| `PointCloudHashGridBuild` | 8.3 s | 6.9 s | -16.2% | 536 MB | 538 MB | +0.4% | ~ |
| `QueryOrder` | 27.6 s | 16.3 s | -41.0% | 723 MB | 725 MB | +0.2% | ~ |
| `Ray` | 215.1 s | 166.9 s | -22.4% | 3002 MB | 3004 MB | +0.0% | ~ |
| `RayCSG` | 276.5 s | 196.7 s | -28.9% | 3266 MB | 3266 MB | +0.0% | ~ |
| `RayCrossing` | 522.5 s | 390.5 s | -25.3% | 3258 MB | 3257 MB | -0.1% | ~ |
| `RayParticles` | 591.6 s | 415.6 s | -29.7% | 4711 MB | 4711 MB | +0.0% | ~ |
| `RayShell` | 302.9 s | 238.6 s | -21.2% | 4477 MB | 4478 MB | +0.0% | ~ |
| `Regions` | 467.6 s | 316.7 s | -32.3% | 3345 MB | 3344 MB | -0.0% | ~ |
| `Render` | 343.7 s | 232.8 s | -32.3% | 4745 MB | 4746 MB | +0.0% | ~ |
| `ShapeFamilies` | 26.0 s | 23.5 s | -9.7% | 603 MB | 605 MB | +0.3% | ~ |
| `Shapes` | 33.3 s | 24.4 s | -26.7% | 617 MB | 618 MB | +0.2% | ~ |
| `SphereCloudBVH` | 9.6 s | 9.2 s | -4.3% | 536 MB | 538 MB | +0.3% | ~ |
| `SphereCloudRays` | 242.3 s | 157.7 s | -34.9% | 3221 MB | 3257 MB | +1.1% | ~ |
| `SphereContacts` | 15.4 s | 8.8 s | -42.8% | 536 MB | 538 MB | +0.3% | ~ |
| `SurfaceParts` | 492.0 s | 344.3 s | -30.0% | 3400 MB | 3400 MB | +0.0% | ~ |
| `Tape` | 92.9 s | 60.5 s | -34.9% | 1089 MB | 1091 MB | +0.2% | ~ |
| `TapeBVHUnion` | 59.2 s | 42.6 s | -28.1% | 1030 MB | 1032 MB | +0.2% | ~ |
| `Weld` | - | 192.9 s | | - | 3411 MB | | new |
| **all, in both** | 6644.6 s | 4719.8 s | -29.0% | 5380 MB | 5378 MB | -0.0% | ~ |

Device kernels: 232 in both, 10 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 8 new, 0 gone. Spills are bytes stored/loaded per thread.

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

Flagged after 2 rounds, and judged on 10 (2 cases): DomainObject/partsOwn/labels/double/hostdevice, RayParticles/armadillo-shell-tape/step-morton/float/hostdevice

The probes differ between the sides (each side ran its own): Performance/Weld/main.cpp

<!-- END receipt cuda -->
