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
base: 601747f
base-fingerprint: 79ff60a3870f9e06e9c79fcdade041da0e82d640
head: 5ceac21
head-fingerprint: abd24c65a30e4072ba49e254e11f60ab1a61e170
cases: 244
rounds: 2
reps: 3
date: 2026-11-19
-->

## Host comparison: green

**Accepted:** -

`5ceac21` against `601747f` (origin/main), 2026-11-19.

**VERDICT: no regression (0 major, 0 minor across 244 cases)**

Performance comparison: 0 major, 0 minor, 3 faster, 0 differ, 0 lost, 244 cases

- base: 601747f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 5ceac21 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 338 ms | 342 ms | +1.2% | [-2.0%, +4.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 180 ms | 182 ms | +1.2% | [+0.2%, +2.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 521 ms | 525 ms | +0.8% | [-0.4%, +2.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 358 ms | 353 ms | -1.4% | [-4.5%, +1.9%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 190 ms | 190 ms | +0.0% | [-3.5%, +3.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 571 ms | 572 ms | +0.1% | [-2.3%, +2.5%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 44.0 ms | 44.2 ms | +0.6% | [-6.2%, +7.8%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 78.9 ms | 78.8 ms | -0.1% | [-6.8%, +7.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 57.5 ms | 57.0 ms | -0.8% | [-3.0%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 52.1 ms | 52.0 ms | -0.3% | [-1.6%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.2 ms | 65.2 ms | -0.0% | [-2.5%, +2.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 55.4 ms | 55.5 ms | +0.2% | [-2.5%, +3.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.4 ms | 62.0 ms | +0.9% | [-0.9%, +2.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.5 ms | 74.1 ms | +2.2% | [-0.5%, +4.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 73.7 ms | 73.6 ms | -0.1% | [-2.9%, +2.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 81.4 ms | 81.5 ms | +0.2% | [-7.7%, +8.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.8 ms | 65.8 ms | -1.6% | [-5.4%, +2.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 104 ms | 103 ms | -0.5% | [-7.4%, +6.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 78.7 ms | 78.3 ms | -0.5% | [-2.4%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 116 ms | 114 ms | -1.4% | [-9.4%, +7.5%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 64.7 ms | 64.6 ms | -0.1% | [-2.4%, +2.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 98.1 ms | 96.9 ms | -1.2% | [-4.4%, +2.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 76.9 ms | 77.2 ms | +0.4% | [-1.2%, +2.0%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 104 ms | 104 ms | +0.3% | [-4.1%, +4.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.6 ms | 19.7 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.9 ms | 41.5 ms | -1.1% | [-2.9%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 74.5 ms | 74.9 ms | +0.6% | [-2.4%, +3.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.3 ms | 20.3 ms | +0.1% | [-0.9%, +1.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 44.2 ms | 44.4 ms | +0.3% | [-0.9%, +1.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 83.6 ms | 84.4 ms | +1.0% | [-3.7%, +6.0%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 142 ms | 142 ms | -0.0% | [-2.4%, +2.5%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 77.5 ms | 77.6 ms | +0.2% | [-3.6%, +4.1%] | +0.0% | ~ | surface/leaf 1.84x |
| `ClosestSurface/union/closestSurface/float/host` | 152 ms | 150 ms | -1.3% | [-7.4%, +5.2%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 78.6 ms | 78.6 ms | -0.1% | [-5.4%, +5.5%] | +0.0% | ~ | surface/leaf 1.91x |
| `ClosestSurface/cloud/closestSurface/double/host` | 159 ms | 160 ms | +0.5% | [-2.3%, +3.4%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 81.8 ms | 81.3 ms | -0.5% | [-2.5%, +1.5%] | +0.0% | ~ | surface/leaf 1.97x |
| `ClosestSurface/union/closestSurface/double/host` | 157 ms | 156 ms | -0.7% | [-4.5%, +3.3%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 82.1 ms | 81.5 ms | -0.8% | [-2.5%, +1.0%] | +0.0% | ~ | surface/leaf 1.92x |
| `Domain/particles/labels/float/host` | 13.6 ms | 13.5 ms | -0.5% | [-0.6%, -0.4%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.87 ms | 6.81 ms | -0.9% | [-2.3%, +0.6%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.38 ms | 1.38 ms | +0.5% | [-2.0%, +3.1%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.1 ms | 14.3 ms | +0.9% | [-0.3%, +2.0%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.19 ms | 9.16 ms | +27.4% | [-0.2%, +55.5%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.39 ms | 1.37 ms | -1.1% | [-3.9%, +1.7%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 39.9 ms | 40.0 ms | +0.1% | [-1.6%, +1.8%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.6 ms | 21.5 ms | -0.2% | [-2.0%, +1.6%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 565 µs | 570 µs | +0.9% | [-1.8%, +3.7%] | +0.0% | ~ | whole/own 70.08x |
| `DomainObject/partsOwn/labelAt/float/host` | 305 µs | 306 µs | +0.4% | [-0.7%, +1.5%] | +0.0% | ~ | whole/own 70.36x |
| `DomainObject/partsWhole/labels/double/host` | 39.6 ms | 41.6 ms | +4.9% | [-0.0%, +9.9%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.7 ms | 22.7 ms | +0.0% | [-0.7%, +0.8%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 581 µs | 592 µs | +1.9% | [-0.6%, +4.4%] | +0.0% | ~ | whole/own 70.22x |
| `DomainObject/partsOwn/labelAt/double/host` | 319 µs | 320 µs | +0.3% | [-2.0%, +2.7%] | +0.0% | ~ | whole/own 71.14x |
| `Fused/csg/signedDistance/float/host` | 4.52 ms | 4.52 ms | -0.1% | [-2.1%, +1.9%] | - | ~ | tape/fused 3.02x |
| `Fused/smooth/signedDistance/float/host` | 12.4 ms | 12.4 ms | -0.1% | [-2.0%, +1.8%] | - | ~ | tape/fused 2.18x |
| `Fused/chain16/signedDistance/float/host` | 976 µs | 990 µs | +1.4% | [-1.3%, +4.2%] | - | ~ | tape/fused 25.30x |
| `Fused/drilled-mesh/signedDistance/float/host` | 47.1 ms | 47.2 ms | +0.1% | [-3.9%, +4.3%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 47.9 ms | 47.6 ms | -0.6% | [-3.0%, +1.9%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.66 ms | 4.69 ms | +0.6% | [-0.9%, +2.2%] | - | ~ | tape/fused 3.59x |
| `Fused/smooth/signedDistance/double/host` | 12.9 ms | 12.9 ms | +0.3% | [-1.1%, +1.7%] | - | ~ | tape/fused 2.56x |
| `Fused/chain16/signedDistance/double/host` | 3.94 ms | 3.99 ms | +1.2% | [-0.2%, +2.6%] | - | ~ | tape/fused 6.35x |
| `Fused/drilled-mesh/signedDistance/double/host` | 56.4 ms | 55.1 ms | -2.2% | [-10.0%, +6.6%] | +0.0% | ~ | tape/fused 0.97x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 56.3 ms | 53.6 ms | -4.8% | [-9.6%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 84.3 ms | 84.4 ms | +0.0% | [-2.6%, +2.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 234 ms | 237 ms | +1.3% | [-4.8%, +7.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 114 ms | 114 ms | +0.3% | [-1.6%, +2.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 272 ms | 271 ms | -0.6% | [-6.6%, +5.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 71.5 ms | 71.5 ms | -0.0% | [-2.5%, +2.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 218 ms | 218 ms | +0.0% | [-3.7%, +3.9%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 98.7 ms | 98.1 ms | -0.6% | [-3.2%, +2.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 32.0 ms | 32.2 ms | +0.6% | [-17.9%, +23.0%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 789 ms | 796 ms | +0.9% | [-7.2%, +9.6%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 120 ms | 118 ms | -1.1% | [-3.2%, +1.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 50.4 ms | 44.1 ms | -12.4% | [-25.0%, +4.7%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 928 ms | 947 ms | +2.1% | [-10.8%, +16.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.7 ms | 26.6 ms | -0.4% | [-3.9%, +3.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.7 ms | 16.1 ms | +2.1% | [-0.1%, +4.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 330 ms | 329 ms | -0.2% | [-5.4%, +5.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 235 ms | 243 ms | +3.1% | [-2.1%, +8.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 92.2 ms | 93.7 ms | +1.6% | [-0.3%, +3.5%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 82.8 ms | 82.4 ms | -0.5% | [-3.2%, +2.3%] | +0.0% | ~ | tape/fused 2.94x (was 2.84x) |
| `MeshCSG/union-tape/firstHit/double/host` | 29.6 ms | 29.2 ms | -1.3% | [-5.8%, +3.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.5 ms | 17.4 ms | -0.5% | [-4.8%, +4.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 370 ms | 372 ms | +0.5% | [-6.4%, +7.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 252 ms | 250 ms | -0.6% | [-4.7%, +3.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 102 ms | 101 ms | -0.1% | [-4.4%, +4.2%] | +0.0% | ~ | tape/fused 3.66x |
| `MeshCSG/smooth-fused/frame/double/host` | 88.6 ms | 88.4 ms | -0.3% | [-3.1%, +2.5%] | +0.0% | ~ | tape/fused 2.83x |
| `MeshClosestPoint/tri/signedDistance/float/host` | 234 ms | 234 ms | -0.2% | [-4.2%, +4.0%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 238 ms | 237 ms | -0.3% | [-4.0%, +3.6%] | +0.0% | ~ | signedDistance/closestPoint 0.99x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 326 ms | 322 ms | -1.0% | [-9.2%, +8.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 242 ms | 241 ms | -0.2% | [-6.4%, +6.3%] | +0.0% | ~ | closestPoint/gradient 0.98x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 276 ms | 275 ms | -0.4% | [-8.2%, +8.0%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 278 ms | 277 ms | -0.4% | [-7.5%, +7.3%] | +0.0% | ~ | signedDistance/closestPoint 0.99x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 373 ms | 374 ms | +0.3% | [-11.1%, +13.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 291 ms | 283 ms | -2.8% | [-15.3%, +12.0%] | +0.0% | ~ | closestPoint/gradient 0.98x |
| `MeshIngest/armadillo/read-soup/float/host` | - | - | - | - | - | new |  |
| `MeshIngest/armadillo/clean/float/host` | - | - | - | - | - | new |  |
| `MeshIngest/armadillo/read-dcel/float/host` | - | - | - | - | - | new |  |
| `MeshIngest/armadillo/read-triangles/float/host` | - | - | - | - | - | new |  |
| `MeshIngest/armadillo/read-soup/double/host` | - | - | - | - | - | new |  |
| `MeshIngest/armadillo/clean/double/host` | - | - | - | - | - | new |  |
| `MeshIngest/armadillo/read-dcel/double/host` | - | - | - | - | - | new |  |
| `MeshIngest/armadillo/read-triangles/double/host` | - | - | - | - | - | new |  |
| `MeshReconcile/armadillo/reconcile/float/host` | 136 ms | 79.4 ms | -41.7% | [-46.2%, -36.8%] | +0.0% | faster |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 43.2 ms | 43.7 ms | +1.3% | [-7.6%, +10.9%] | +0.0% | ~ | reconcile/scheduled 1.81x (was 3.15x) |
| `MeshReconcile/armadillo/reconcile/double/host` | 167 ms | 104 ms | -37.8% | [-46.2%, -28.1%] | +0.0% | faster |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 59.1 ms | 59.0 ms | -0.1% | [-8.1%, +8.5%] | +0.0% | ~ | reconcile/scheduled 1.76x (was 2.83x) |
| `MeshTape/mesh/firstHit/float/host` | 49.6 ms | 49.8 ms | +0.5% | [-4.0%, +5.2%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 50.0 ms | 49.8 ms | -0.4% | [-3.5%, +2.8%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 20.0 ms | 19.9 ms | -0.3% | [-3.1%, +2.5%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 84.4 ms | 84.1 ms | -0.3% | [-7.6%, +7.8%] | +0.0% | ~ | mesh/tape 0.59x |
| `MeshTape/tape/firstCrossing/float/host` | 52.8 ms | 56.7 ms | +7.2% | [-4.5%, +20.2%] | +0.0% | ~ | mesh/tape 0.88x (was 0.95x) |
| `MeshTape/tape/frame/float/host` | 25.6 ms | 25.0 ms | -2.3% | [-4.8%, +0.4%] | +0.0% | ~ | mesh/tape 0.80x |
| `MeshTape/view/firstHit/float/host` | 85.7 ms | 85.8 ms | +0.1% | [-5.6%, +6.2%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/float/host` | 26.7 ms | 26.6 ms | -0.4% | [-5.1%, +4.5%] | +0.0% | ~ | mesh/view 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 55.3 ms | 55.4 ms | +0.1% | [-6.7%, +7.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 55.0 ms | 54.6 ms | -0.7% | [-6.3%, +5.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 21.1 ms | 20.8 ms | -1.3% | [-3.8%, +1.1%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 89.3 ms | 89.3 ms | -0.1% | [-4.6%, +4.7%] | +0.0% | ~ | mesh/tape 0.62x |
| `MeshTape/tape/firstCrossing/double/host` | 57.0 ms | 56.9 ms | -0.3% | [-6.2%, +6.0%] | +0.0% | ~ | mesh/tape 0.96x |
| `MeshTape/tape/frame/double/host` | 27.3 ms | 27.3 ms | +0.3% | [-3.1%, +3.8%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 94.8 ms | 94.7 ms | -0.2% | [-8.5%, +8.9%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/double/host` | 30.0 ms | 29.7 ms | -1.1% | [-6.5%, +4.5%] | +0.0% | ~ | mesh/view 0.70x |
| `QueryOrder/random/mortonOrder/float/host` | 62.3 ms | 62.0 ms | -0.5% | [-4.3%, +3.3%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | 238 ms | 242 ms | +1.5% | [-10.6%, +15.2%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | 164 ms | 166 ms | +1.2% | [-3.1%, +5.8%] | +0.0% | ~ | arrived/ordered 1.46x |
| `QueryOrder/random/mortonOrder/double/host` | 63.0 ms | 63.0 ms | -0.1% | [-1.8%, +1.7%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | 278 ms | 280 ms | +0.8% | [-7.8%, +10.1%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | 176 ms | 175 ms | -0.6% | [-3.5%, +2.5%] | +0.0% | ~ | arrived/ordered 1.60x |
| `Ray/mesh/firstHit/float/host` | 50.0 ms | 49.8 ms | -0.5% | [-4.3%, +3.6%] | +0.0% | ~ | hand/api 0.96x (was 0.94x) |
| `Ray/mesh/firstHit-handrolled/float/host` | 46.8 ms | 48.0 ms | +2.6% | [-4.1%, +9.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 41.8 ms | 41.7 ms | -0.3% | [-6.2%, +6.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 84.5 ms | 84.6 ms | +0.1% | [-11.7%, +13.4%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 131 ms | 133 ms | +1.6% | [-20.4%, +35.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.30 ms | 5.29 ms | -0.2% | [-2.2%, +1.9%] | - | ~ | tape/fused 1.46x |
| `Ray/csg-tape/firstHit/float/host` | 7.71 ms | 7.72 ms | +0.1% | [-1.7%, +2.0%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 55.1 ms | 54.9 ms | -0.4% | [-6.4%, +6.1%] | +0.0% | ~ | hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled/double/host` | 53.1 ms | 52.9 ms | -0.3% | [-6.1%, +5.9%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 46.6 ms | 46.3 ms | -0.6% | [-6.9%, +6.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 177 ms | 180 ms | +1.6% | [-17.8%, +29.8%] | +0.0% | ~ | triangles/polygons 0.31x |
| `Ray/mesh-polygons/intersects/double/host` | 92.4 ms | 88.4 ms | -4.3% | [-9.8%, +1.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.57 ms | 5.62 ms | +0.9% | [+0.2%, +1.6%] | - | ~ | tape/fused 1.42x |
| `Ray/csg-tape/firstHit/double/host` | 8.00 ms | 7.97 ms | -0.3% | [-1.7%, +1.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.8 ms | 18.4 ms | +3.7% | [-1.5%, +9.2%] | - | ~ | marched/traced 7.85x (was 6.91x) |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 123 ms | 145 ms | +17.8% | [-2.0%, +38.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 28.6 ms | 29.1 ms | +1.9% | [-36.8%, +63.5%] | +0.0% | ~ | marched/traced 6.52x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 184 ms | 190 ms | +3.2% | [-31.6%, +55.7%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 43.9 ms | 44.1 ms | +0.4% | [-29.8%, +43.8%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 27.9 ms | 27.5 ms | -1.3% | [-46.4%, +82.5%] | - | ~ | marched/traced 5.51x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 152 ms | 152 ms | +0.1% | [-24.3%, +32.4%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 31.4 ms | 30.7 ms | -2.0% | [-37.7%, +55.1%] | +0.0% | ~ | marched/traced 7.08x (was 6.12x) |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 192 ms | 218 ms | +13.3% | [-29.6%, +76.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 49.5 ms | 49.7 ms | +0.3% | [-34.4%, +54.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 67.4 ms | 67.3 ms | -0.1% | [-39.1%, +64.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 69.1 ms | 68.0 ms | -1.6% | [-40.8%, +64.7%] | +0.0% | ~ | hit/crossing 0.99x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.00 ms | 12.7 ms | +26.7% | [-0.1%, +70.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 7.76 ms | 7.75 ms | -0.0% | [-34.5%, +52.4%] | +0.0% | ~ | hit/crossing 1.63x (was 1.29x) |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 250 ms | 262 ms | +4.9% | [-40.5%, +81.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 232 ms | 227 ms | -2.1% | [-45.0%, +75.2%] | +0.0% | ~ | hit/crossing 1.15x (was 1.08x) |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 101 ms | 97.1 ms | -4.1% | [-43.2%, +61.0%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 79.3 ms | 69.4 ms | -12.5% | [-34.7%, +21.9%] | +0.0% | ~ | hit/crossing 1.40x (was 1.28x) |
| `RayCrossing/mesh/firstHit/double/host` | 64.8 ms | 64.5 ms | -0.4% | [-12.3%, +12.8%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 63.2 ms | 64.5 ms | +2.0% | [-8.4%, +13.7%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.26 ms | 8.20 ms | -0.8% | [-5.2%, +3.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.62 ms | 6.57 ms | -0.8% | [-4.1%, +2.7%] | +0.0% | ~ | hit/crossing 1.25x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 234 ms | 232 ms | -0.7% | [-8.1%, +7.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 203 ms | 203 ms | +0.1% | [-5.2%, +5.7%] | +0.0% | ~ | hit/crossing 1.14x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 88.0 ms | 85.7 ms | -2.7% | [-5.3%, -0.0%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 71.7 ms | 72.4 ms | +1.0% | [-2.8%, +5.0%] | +0.0% | ~ | hit/crossing 1.18x (was 1.23x) |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.5 ms | 11.6 ms | +0.3% | [-1.3%, +2.0%] | +0.0% | ~ | marched/traced 4.80x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 55.3 ms | 55.6 ms | +0.6% | [-2.1%, +3.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.35 ms | 8.46 ms | +1.3% | [-0.1%, +2.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.07 ms | 2.05 ms | -1.3% | [-3.2%, +0.8%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.12 ms | 7.11 ms | -0.2% | [-2.7%, +2.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.3 ms | 12.4 ms | +0.3% | [-1.5%, +2.1%] | +0.0% | ~ | marched/traced 5.54x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 67.5 ms | 68.5 ms | +1.6% | [-2.8%, +6.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 9.08 ms | 9.14 ms | +0.7% | [-1.1%, +2.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.15 ms | 2.12 ms | -1.5% | [-2.4%, -0.6%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.40 ms | 7.32 ms | -1.1% | [-2.7%, +0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 243 ms | 243 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ | marched/traced 1.44x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 349 ms | 350 ms | +0.3% | [-2.6%, +3.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 252 ms | 249 ms | -1.2% | [-6.4%, +4.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 307 ms | 286 ms | -6.8% | [-23.1%, +14.2%] | +0.0% | ~ | marched/traced 1.88x (was 1.75x) |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 537 ms | 539 ms | +0.3% | [-39.4%, +66.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 276 ms | 272 ms | -1.4% | [-10.0%, +8.0%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.57 ms | 7.56 ms | -0.1% | [-2.7%, +2.6%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 13.1 ms | 12.7 ms | -3.3% | [-5.6%, -0.9%] | +0.0% | faster |  |
| `Regions/particles/regionByRegion/float/host` | 274 ms | 276 ms | +0.6% | [-2.7%, +4.0%] | +0.0% | ~ | one-pass/each 0.05x (was 0.05x) |
| `Regions/particles/domainLabels/float/host` | 12.6 ms | 12.6 ms | -0.2% | [-2.4%, +2.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 31.3 ms | 31.1 ms | -0.6% | [-2.8%, +1.7%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.6 ms | 22.4 ms | -0.9% | [-2.8%, +1.0%] | +0.0% | ~ | regions/hit 1.39x |
| `Regions/particles/region/double/host` | 7.73 ms | 7.75 ms | +0.2% | [-1.5%, +2.0%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 13.2 ms | 13.1 ms | -1.3% | [-2.5%, -0.2%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 292 ms | 290 ms | -0.7% | [-1.1%, -0.3%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.3 ms | 13.2 ms | -0.8% | [-1.5%, -0.1%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 33.3 ms | 32.8 ms | -1.7% | [-4.5%, +1.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 24.2 ms | 24.2 ms | +0.1% | [-2.3%, +2.6%] | +0.0% | ~ | regions/hit 1.35x |
| `Render/csg/frame/float/host` | 12.4 ms | 12.9 ms | +3.9% | [-0.9%, +8.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 12.9 ms | 12.4 ms | -3.8% | [-7.8%, +0.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.7 ms | 20.6 ms | -0.4% | [-1.2%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.1 ms | 14.3 ms | +0.9% | [-0.2%, +1.9%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.0 ms | 14.1 ms | +0.5% | [-1.1%, +2.0%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.1 ms | 21.2 ms | +0.8% | [+0.2%, +1.4%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 79.1 ms | 80.3 ms | +1.6% | [+0.9%, +2.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 316 ms | 320 ms | +1.2% | [+0.6%, +1.9%] | +0.0% | ~ | tape/fused 1.09x (was 1.00x) |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 316 ms | 349 ms | +10.5% | [-0.4%, +21.5%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 97.8 ms | 98.6 ms | +0.8% | [-0.0%, +1.7%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 392 ms | 400 ms | +2.2% | [+0.2%, +4.2%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 397 ms | 399 ms | +0.7% | [-0.6%, +1.9%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 148 ms | 148 ms | -0.0% | [-2.1%, +2.0%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 92.3 ms | 92.3 ms | +0.0% | [-2.2%, +2.3%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 150 ms | 150 ms | +0.2% | [-18.1%, +22.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 91.5 ms | 104 ms | +13.6% | [-29.9%, +76.0%] | +0.0% | ~ | surface/leaf 1.45x (was 1.64x) |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 74.0 ms | 74.2 ms | +0.3% | [-23.5%, +31.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 42.3 ms | 41.7 ms | -1.3% | [-27.1%, +33.8%] | +0.0% | ~ | surface/hit 1.78x |
| `SurfaceParts/boxes/closestSurface/double/host` | 155 ms | 157 ms | +1.5% | [-9.1%, +13.3%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 98.2 ms | 99.1 ms | +0.9% | [-8.1%, +10.7%] | +0.0% | ~ | surface/leaf 1.58x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 186 ms | 189 ms | +1.4% | [-23.1%, +35.0%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 125 ms | 112 ms | -10.5% | [-45.1%, +52.3%] | +0.0% | ~ | surface/leaf 1.68x (was 1.48x) |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 84.4 ms | 103 ms | +21.6% | [-23.3%, +81.6%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 48.2 ms | 50.0 ms | +3.7% | [-24.8%, +42.1%] | +0.0% | ~ | surface/hit 2.05x (was 1.75x) |
| `Tape/csg/signedDistance/float/host` | 13.8 ms | 13.7 ms | -0.8% | [-12.0%, +12.0%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 5.60 ms | 4.84 ms | -13.5% | [-30.4%, +10.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 26.9 ms | 27.1 ms | +0.8% | [-10.4%, +13.4%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 50.5 ms | 53.1 ms | +5.1% | [-25.5%, +49.9%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 2.76 ms | 2.77 ms | +0.3% | [-51.5%, +107.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 171 µs | 168 µs | -1.9% | [-39.7%, +57.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 32.2 ms | 34.4 ms | +6.9% | [-10.5%, +26.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 6.37 ms | 6.44 ms | +1.1% | [-17.5%, +22.8%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 25.1 ms | 25.0 ms | -0.0% | [-51.2%, +104.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 5.35 ms | 5.46 ms | +2.0% | [-21.4%, +32.1%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 16.6 ms | 16.8 ms | +1.4% | [-34.5%, +56.3%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 5.54 ms | 5.51 ms | -0.6% | [-27.8%, +36.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 32.7 ms | 33.2 ms | +1.3% | [-31.9%, +49.8%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 52.7 ms | 52.9 ms | +0.3% | [-32.2%, +49.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 2.81 ms | 2.84 ms | +0.8% | [-54.8%, +116.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 546 µs | 594 µs | +8.7% | [-61.4%, +188.3%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 46.2 ms | 45.8 ms | -0.7% | [-51.0%, +101.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 8.45 ms | 8.39 ms | -0.7% | [-12.8%, +12.6%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 25.2 ms | 25.4 ms | +0.5% | [-51.5%, +108.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 8.35 ms | 8.39 ms | +0.5% | [-11.6%, +14.0%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 51.7 ms | 55.3 ms | +6.8% | [-29.7%, +58.9%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 218 ms | 267 ms | +22.4% | [-12.5%, +62.3%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 46.1 ms | 46.3 ms | +0.3% | [-10.2%, +12.2%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 236 ms | 236 ms | +0.1% | [-12.2%, +14.1%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 758.5 s → 748.4 s (-1.3%), the largest peak 1413 MB → 1418 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 10.0 s | 9.9 s | -0.7% | 559 MB | 557 MB | -0.3% | ~ |
| `BVHRefit` | 4.5 s | 4.4 s | -1.7% | 497 MB | 496 MB | -0.3% | ~ |
| `BVHUnion` | 20.5 s | 19.9 s | -3.1% | 948 MB | 947 MB | -0.1% | ~ |
| `BVHUnionMixed` | 8.1 s | 7.9 s | -2.4% | 555 MB | 556 MB | +0.2% | ~ |
| `ClosestPrimitive` | 32.1 s | 31.3 s | -2.4% | 1298 MB | 1297 MB | -0.1% | ~ |
| `ClosestSurface` | 30.3 s | 28.7 s | -5.1% | 1296 MB | 1296 MB | +0.0% | ~ |
| `Domain` | 19.3 s | 18.5 s | -3.9% | 842 MB | 842 MB | -0.1% | ~ |
| `DomainObject` | 17.0 s | 16.4 s | -3.7% | 846 MB | 846 MB | -0.1% | ~ |
| `Fused` | 26.5 s | 25.4 s | -4.4% | 1205 MB | 1207 MB | +0.2% | ~ |
| `LaunchTuning` | 38.8 s | 37.5 s | -3.2% | 1280 MB | 1279 MB | -0.1% | ~ |
| `Mesh` | 16.7 s | 16.2 s | -2.9% | 784 MB | 785 MB | +0.2% | ~ |
| `MeshCSG` | 41.3 s | 40.9 s | -1.1% | 1260 MB | 1257 MB | -0.3% | ~ |
| `MeshClosestPoint` | 15.1 s | 14.9 s | -1.7% | 779 MB | 781 MB | +0.2% | ~ |
| `MeshIngest` | - | 7.2 s | | - | 568 MB | | new |
| `MeshReconcile` | 7.0 s | 6.8 s | -3.0% | 566 MB | 568 MB | +0.5% | ~ |
| `MeshTape` | 43.5 s | 43.2 s | -0.8% | 1279 MB | 1279 MB | +0.0% | ~ |
| `PointCloud` | 7.9 s | 7.6 s | -3.5% | 558 MB | 560 MB | +0.5% | ~ |
| `PointCloudBVHBuild` | 4.0 s | 3.9 s | -3.1% | 472 MB | 471 MB | -0.1% | ~ |
| `PointCloudHashGridBuild` | 3.4 s | 3.4 s | -1.7% | 452 MB | 452 MB | -0.2% | ~ |
| `QueryOrder` | 11.6 s | 11.6 s | -0.8% | 687 MB | 691 MB | +0.6% | ~ |
| `Ray` | 37.4 s | 38.1 s | +1.8% | 1292 MB | 1291 MB | -0.1% | ~ |
| `RayCSG` | 33.4 s | 33.3 s | -0.1% | 1188 MB | 1192 MB | +0.4% | ~ |
| `RayCrossing` | 47.5 s | 47.0 s | -1.1% | 1349 MB | 1343 MB | -0.4% | ~ |
| `RayParticles` | 50.5 s | 50.8 s | +0.5% | 1330 MB | 1332 MB | +0.1% | ~ |
| `RayShell` | 39.3 s | 39.0 s | -0.7% | 1266 MB | 1266 MB | -0.0% | ~ |
| `Regions` | 30.3 s | 30.6 s | +1.2% | 1073 MB | 1073 MB | -0.0% | ~ |
| `Render` | 37.5 s | 37.3 s | -0.6% | 1273 MB | 1272 MB | -0.0% | ~ |
| `Shapes` | 9.1 s | 9.1 s | +0.2% | 555 MB | 555 MB | +0.1% | ~ |
| `SphereCloudBVH` | 5.1 s | 5.0 s | -1.6% | 512 MB | 511 MB | -0.1% | ~ |
| `SphereCloudRays` | 29.5 s | 28.7 s | -2.5% | 1066 MB | 1065 MB | -0.1% | ~ |
| `SphereContacts` | 4.3 s | 4.6 s | +7.5% | 486 MB | 485 MB | -0.2% | ~ |
| `SurfaceParts` | 43.8 s | 43.6 s | -0.5% | 1413 MB | 1418 MB | +0.3% | ~ |
| `Tape` | 17.7 s | 17.6 s | -0.4% | 829 MB | 830 MB | +0.1% | ~ |
| `TapeBVHUnion` | 15.6 s | 15.3 s | -1.6% | 826 MB | 826 MB | +0.0% | ~ |
| **all, in both** | 758.5 s | 748.4 s | -1.3% | 1413 MB | 1418 MB | +0.3% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): Ray/mesh-polygons/intersects/float/host, Ray/mesh-polygons/firstHit/double/host

These probes compiled to the same code on both sides and were not run (72 cases): PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20), SphereCloudBVH (6), SphereContacts (2)

The probes differ between the sides (each side ran its own): Performance/MeshIngest/main.cpp

<!-- END receipt host -->
