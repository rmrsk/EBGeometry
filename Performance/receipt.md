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
base: 69cd244
base-fingerprint: 5c8f1f2e8f2f01ac153884975bc82e49bb9e3477
head: 6f7967a
head-fingerprint: 3adc6f11e3922774f885720dc4cd9db57d17a9fe
cases: 278
rounds: 2
reps: 3
date: 2026-11-17
-->

## Host comparison: green

**Accepted:** -

`6f7967a` against `69cd244` (origin/main), 2026-11-17.

**VERDICT: no regression (0 major, 0 minor across 278 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 278 cases

- base: 69cd244 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 6f7967a (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 337 ms | 336 ms | -0.3% | [-3.5%, +3.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 180 ms | 181 ms | +0.3% | [-3.1%, +3.8%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 522 ms | 524 ms | +0.4% | [-2.5%, +3.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 353 ms | 354 ms | +0.3% | [-3.5%, +4.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 190 ms | 190 ms | +0.0% | [-3.6%, +3.8%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 574 ms | 575 ms | +0.2% | [-3.7%, +4.2%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | 45.3 ms | 44.5 ms | -1.7% | [-14.9%, +13.8%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 78.5 ms | 78.8 ms | +0.4% | [-5.9%, +7.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 57.3 ms | 57.3 ms | -0.1% | [-4.2%, +4.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 52.7 ms | 52.5 ms | -0.4% | [-4.8%, +4.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 66.1 ms | 65.9 ms | -0.3% | [-4.3%, +3.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 56.9 ms | 56.0 ms | -1.6% | [-5.6%, +2.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.6 ms | 62.0 ms | +0.5% | [-3.6%, +4.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 74.1 ms | 74.6 ms | +0.7% | [-4.1%, +5.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 73.5 ms | 73.2 ms | -0.4% | [-4.3%, +3.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 80.3 ms | 80.9 ms | +0.7% | [-6.8%, +8.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.1 ms | 65.6 ms | -0.7% | [-4.6%, +3.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 104 ms | 103 ms | -0.5% | [-5.1%, +4.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 78.3 ms | 78.6 ms | +0.3% | [-3.5%, +4.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 113 ms | 115 ms | +1.8% | [-7.6%, +12.1%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 65.6 ms | 65.8 ms | +0.3% | [-3.8%, +4.5%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 98.6 ms | 98.8 ms | +0.2% | [-5.9%, +6.8%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 77.8 ms | 77.5 ms | -0.4% | [-3.8%, +3.2%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 107 ms | 107 ms | -0.6% | [-10.6%, +10.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.7 ms | 19.6 ms | -0.3% | [-4.2%, +3.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.7 ms | 42.5 ms | +1.7% | [-3.8%, +7.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 75.3 ms | 75.1 ms | -0.2% | [-5.8%, +5.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.5 ms | 20.4 ms | -0.2% | [-3.2%, +2.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 44.1 ms | 44.7 ms | +1.4% | [-1.7%, +4.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 85.5 ms | 85.2 ms | -0.3% | [-8.9%, +9.1%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 142 ms | 142 ms | +0.4% | [-1.9%, +2.8%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 77.3 ms | 76.7 ms | -0.8% | [-4.1%, +2.7%] | +0.0% | ~ | surface/leaf 1.86x |
| `ClosestSurface/union/closestSurface/float/host` | 149 ms | 149 ms | -0.3% | [-4.4%, +4.0%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 78.6 ms | 78.1 ms | -0.7% | [-5.6%, +4.4%] | +0.0% | ~ | surface/leaf 1.91x |
| `ClosestSurface/cloud/closestSurface/double/host` | 160 ms | 160 ms | -0.1% | [-4.5%, +4.6%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 82.3 ms | 82.5 ms | +0.1% | [-3.9%, +4.3%] | +0.0% | ~ | surface/leaf 1.94x |
| `ClosestSurface/union/closestSurface/double/host` | 161 ms | 158 ms | -1.7% | [-8.3%, +5.4%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 83.2 ms | 83.1 ms | -0.1% | [-5.6%, +5.6%] | +0.0% | ~ | surface/leaf 1.90x |
| `Domain/particles/labels/float/host` | 13.7 ms | 13.7 ms | +0.1% | [-1.7%, +1.9%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 8.78 ms | 8.77 ms | -0.2% | [-37.3%, +59.0%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.36 ms | 1.36 ms | +0.3% | [-3.0%, +3.7%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.3 ms | 14.3 ms | -0.3% | [-2.2%, +1.6%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.23 ms | 7.21 ms | -0.2% | [-2.3%, +1.9%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.36 ms | 1.38 ms | +1.6% | [-0.8%, +4.0%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 40.2 ms | 40.2 ms | -0.1% | [-3.8%, +3.8%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.6 ms | 21.7 ms | +0.5% | [-2.0%, +3.0%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 570 µs | 565 µs | -0.9% | [-4.1%, +2.4%] | +0.0% | ~ | own/whole 71.16x |
| `DomainObject/partsOwn/labelAt/float/host` | 304 µs | 304 µs | -0.1% | [-0.9%, +0.7%] | +0.0% | ~ | own/whole 71.34x |
| `DomainObject/partsWhole/labels/double/host` | 40.1 ms | 40.1 ms | -0.1% | [-3.4%, +3.3%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.9 ms | 23.0 ms | +0.2% | [-0.9%, +1.3%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 580 µs | 589 µs | +1.5% | [-1.2%, +4.3%] | +0.0% | ~ | own/whole 68.03x |
| `DomainObject/partsOwn/labelAt/double/host` | 318 µs | 317 µs | -0.3% | [-1.8%, +1.1%] | +0.0% | ~ | own/whole 72.49x |
| `Fused/csg/signedDistance/float/host` | 4.50 ms | 4.50 ms | +0.2% | [-1.6%, +2.0%] | - | ~ | tape/fused 2.85x |
| `Fused/smooth/signedDistance/float/host` | 12.5 ms | 12.4 ms | -0.8% | [-2.4%, +0.9%] | - | ~ | tape/fused 2.02x |
| `Fused/chain16/signedDistance/float/host` | 979 µs | 981 µs | +0.2% | [-1.8%, +2.2%] | - | ~ | tape/fused 16.47x |
| `Fused/drilled-mesh/signedDistance/float/host` | 47.8 ms | 47.6 ms | -0.5% | [-6.7%, +6.0%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 47.5 ms | 47.6 ms | +0.4% | [-4.7%, +5.7%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.55 ms | 4.58 ms | +0.7% | [-1.5%, +2.8%] | - | ~ | tape/fused 2.89x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | +0.3% | [-3.8%, +4.5%] | - | ~ | tape/fused 2.07x |
| `Fused/chain16/signedDistance/double/host` | 3.94 ms | 3.88 ms | -1.7% | [-5.1%, +2.0%] | - | ~ | tape/fused 4.22x |
| `Fused/drilled-mesh/signedDistance/double/host` | 55.4 ms | 55.0 ms | -0.7% | [-8.7%, +7.9%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 56.2 ms | 55.5 ms | -1.1% | [-9.7%, +8.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 84.4 ms | 84.5 ms | +0.1% | [-3.9%, +4.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 235 ms | 237 ms | +0.5% | [-5.3%, +6.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 115 ms | 116 ms | +0.5% | [-2.7%, +3.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 271 ms | 274 ms | +1.0% | [-7.7%, +10.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 72.1 ms | 72.3 ms | +0.2% | [-3.8%, +4.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 219 ms | 221 ms | +0.9% | [-5.0%, +7.0%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 98.9 ms | 99.0 ms | +0.1% | [-4.4%, +4.8%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 31.4 ms | 31.6 ms | +0.7% | [-15.2%, +19.6%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 802 ms | 799 ms | -0.4% | [-10.4%, +10.8%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 120 ms | 121 ms | +0.4% | [-3.9%, +4.8%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 49.9 ms | 50.1 ms | +0.5% | [-20.6%, +27.7%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 926 ms | 930 ms | +0.4% | [-14.5%, +18.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 27.4 ms | 27.6 ms | +0.9% | [-3.4%, +5.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.8 ms | 27.9 ms | +0.6% | [-2.2%, +3.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 329 ms | 328 ms | -0.1% | [-5.5%, +5.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 243 ms | 244 ms | +0.1% | [-3.9%, +4.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 92.9 ms | 93.2 ms | +0.3% | [-4.8%, +5.7%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 104 ms | 105 ms | +1.0% | [-4.2%, +6.3%] | +0.0% | ~ | tape/fused 2.32x |
| `MeshCSG/union-tape/firstHit/double/host` | 30.6 ms | 30.3 ms | -0.9% | [-7.8%, +6.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.7 ms | 30.7 ms | +0.1% | [-4.7%, +5.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 372 ms | 371 ms | -0.3% | [-8.9%, +9.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 259 ms | 258 ms | -0.4% | [-4.2%, +3.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 100 ms | 100 ms | -0.0% | [-4.7%, +4.9%] | +0.0% | ~ | tape/fused 3.70x |
| `MeshCSG/smooth-fused/frame/double/host` | 111 ms | 112 ms | +0.3% | [-4.0%, +4.8%] | +0.0% | ~ | tape/fused 2.31x |
| `MeshReconcile/armadillo/reconcile/float/host` | 138 ms | 138 ms | +0.2% | [-11.9%, +13.9%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 42.9 ms | 42.9 ms | -0.1% | [-7.2%, +7.7%] | +0.0% | ~ | reconcile/scheduled 3.21x |
| `MeshReconcile/armadillo/reconcile/double/host` | 166 ms | 167 ms | +0.5% | [-14.0%, +17.5%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 59.4 ms | 59.3 ms | -0.2% | [-9.2%, +9.5%] | +0.0% | ~ | reconcile/scheduled 2.82x |
| `PointCloudBVH/uniform/build/float/host` | 54.4 ms | 54.9 ms | +1.0% | [-3.8%, +6.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 150 ms | 150 ms | -0.3% | [-8.5%, +8.7%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 373 ms | 374 ms | +0.2% | [-4.9%, +5.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 82.3 ms | 82.6 ms | +0.4% | [-4.2%, +5.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.91 ms | 6.91 ms | +0.0% | [-3.5%, +3.7%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 57.4 ms | 57.1 ms | -0.4% | [-6.0%, +5.4%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 165 ms | 166 ms | +0.4% | [-8.7%, +10.5%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 405 ms | 405 ms | +0.0% | [-3.9%, +4.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 92.0 ms | 91.0 ms | -1.1% | [-9.4%, +7.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.98 ms | 7.04 ms | +0.9% | [-3.0%, +4.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.9 ms | 13.9 ms | +0.1% | [-3.4%, +3.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 119 ms | 119 ms | +0.2% | [-2.0%, +2.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 484 ms | 485 ms | +0.0% | [-3.1%, +3.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 145 ms | 146 ms | +0.2% | [-3.8%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 817 ms | 818 ms | +0.1% | [-3.9%, +4.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.1 ms | 15.5 ms | +2.6% | [+0.0%, +5.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 123 ms | 122 ms | -0.9% | [-5.0%, +3.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 504 ms | 503 ms | -0.2% | [-4.0%, +3.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 149 ms | 149 ms | +0.2% | [-5.5%, +6.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 788 ms | 787 ms | -0.2% | [-5.0%, +5.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 55.9 ms | 55.9 ms | -0.0% | [-5.4%, +5.7%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 136 ms | 136 ms | -0.1% | [-9.3%, +10.0%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 278 ms | 277 ms | -0.5% | [-6.9%, +6.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 153 ms | 154 ms | +0.7% | [-4.7%, +6.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 139 ms | 140 ms | +0.4% | [-4.4%, +5.5%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 58.5 ms | 58.6 ms | +0.3% | [-4.8%, +5.6%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 149 ms | 148 ms | -0.5% | [-11.3%, +11.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 323 ms | 307 ms | -5.1% | [-13.1%, +4.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 171 ms | 171 ms | +0.2% | [-9.0%, +10.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 157 ms | 157 ms | +0.3% | [-8.2%, +9.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 12.2 ms | 12.2 ms | -0.6% | [-2.4%, +1.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 117 ms | 117 ms | -0.0% | [-3.8%, +3.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 327 ms | 326 ms | -0.1% | [-5.2%, +5.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 158 ms | 158 ms | +0.1% | [-3.7%, +4.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 328 ms | 329 ms | +0.1% | [-4.1%, +4.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 14.1 ms | 14.0 ms | -0.8% | [-4.0%, +2.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 119 ms | 118 ms | -0.5% | [-3.7%, +2.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 357 ms | 357 ms | -0.1% | [-4.6%, +4.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 171 ms | 172 ms | +0.3% | [-6.7%, +7.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 322 ms | 321 ms | -0.3% | [-5.9%, +5.7%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 36.2 ms | 36.2 ms | -0.1% | [-5.0%, +5.1%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 39.7 ms | 39.5 ms | -0.5% | [-5.0%, +4.1%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.28 ms | 8.22 ms | -0.7% | [-1.3%, +9.0%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.48 ms | 9.48 ms | -0.0% | [-1.5%, +1.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 47.8 ms | 47.7 ms | -0.1% | [-5.4%, +5.6%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 47.0 ms | 47.0 ms | -0.1% | [-5.4%, +5.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 41.5 ms | 42.0 ms | +1.2% | [-3.4%, +6.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 82.9 ms | 83.7 ms | +1.0% | [-9.1%, +12.0%] | +0.0% | ~ | triangles/polygons 0.57x |
| `Ray/mesh-polygons/intersects/float/host` | 69.5 ms | 70.3 ms | +1.1% | [-10.2%, +13.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.32 ms | 5.77 ms | +8.4% | [-2.3%, +19.2%] | - | ~ | tape/fused 1.34x (was 1.45x) |
| `Ray/csg-tape/firstHit/float/host` | 7.72 ms | 7.74 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 54.5 ms | 54.8 ms | +0.5% | [-6.8%, +8.4%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 53.3 ms | 53.4 ms | +0.1% | [-7.6%, +8.5%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 46.8 ms | 46.6 ms | -0.4% | [-7.4%, +7.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 95.7 ms | 94.5 ms | -1.2% | [-14.6%, +14.6%] | +0.0% | ~ | triangles/polygons 0.58x |
| `Ray/mesh-polygons/intersects/double/host` | 80.3 ms | 80.9 ms | +0.7% | [-14.1%, +17.9%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.57 ms | 5.57 ms | +0.0% | [-2.2%, +2.3%] | - | ~ | tape/fused 1.43x |
| `Ray/csg-tape/firstHit/double/host` | 7.98 ms | 7.97 ms | -0.2% | [-1.7%, +1.4%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.7 ms | 17.7 ms | +0.1% | [-2.1%, +2.4%] | - | ~ | marched/traced 6.82x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 121 ms | 121 ms | -0.5% | [-3.5%, +2.5%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.2 ms | 22.4 ms | +0.7% | [-1.5%, +3.0%] | +0.0% | ~ | marched/traced 6.61x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 148 ms | 148 ms | -0.2% | [-3.7%, +3.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 36.0 ms | 36.1 ms | +0.2% | [-2.7%, +3.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 20.0 ms | 20.0 ms | +0.0% | [-2.4%, +2.5%] | - | ~ | marched/traced 6.32x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 128 ms | 126 ms | -1.2% | [-4.2%, +1.8%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 24.2 ms | 24.1 ms | -0.5% | [-1.4%, +0.3%] | +0.0% | ~ | marched/traced 6.30x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 153 ms | 152 ms | -0.9% | [-3.5%, +1.8%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.3 ms | 38.7 ms | +0.9% | [-1.8%, +3.6%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 47.7 ms | 47.7 ms | +0.0% | [-5.5%, +5.8%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 47.8 ms | 48.2 ms | +0.8% | [-3.9%, +5.8%] | +0.0% | ~ | hit/crossing 0.99x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.79 ms | 7.81 ms | +0.3% | [-1.3%, +1.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.33 ms | 6.33 ms | +0.1% | [-1.5%, +1.8%] | +0.0% | ~ | hit/crossing 1.23x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 185 ms | 184 ms | -0.4% | [-5.8%, +5.4%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 162 ms | 163 ms | +0.4% | [-4.8%, +5.9%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 70.9 ms | 71.0 ms | +0.1% | [-6.1%, +6.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 58.2 ms | 58.2 ms | -0.1% | [-6.5%, +6.9%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 54.3 ms | 54.8 ms | +1.0% | [-7.1%, +9.8%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 54.6 ms | 54.8 ms | +0.4% | [-7.4%, +8.9%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.02 ms | 7.98 ms | -0.5% | [-2.4%, +1.6%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.37 ms | 6.38 ms | +0.1% | [-2.3%, +2.6%] | +0.0% | ~ | hit/crossing 1.25x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 209 ms | 209 ms | -0.2% | [-7.9%, +8.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 186 ms | 186 ms | -0.3% | [-8.3%, +8.4%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 81.8 ms | 81.3 ms | -0.6% | [-9.8%, +9.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 68.5 ms | 68.1 ms | -0.6% | [-10.8%, +11.0%] | +0.0% | ~ | hit/crossing 1.19x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.3 ms | 11.3 ms | -0.4% | [-4.3%, +3.7%] | +0.0% | ~ | marched/traced 4.77x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 53.6 ms | 53.8 ms | +0.4% | [-6.4%, +7.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.17 ms | 8.25 ms | +0.9% | [-2.2%, +4.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.99 ms | 1.96 ms | -1.4% | [-5.4%, +2.8%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.90 ms | 6.96 ms | +0.9% | [-0.5%, +2.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.1 ms | 12.1 ms | -0.0% | [-5.9%, +6.2%] | +0.0% | ~ | marched/traced 5.26x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 64.0 ms | 63.7 ms | -0.4% | [-9.2%, +9.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.97 ms | 8.89 ms | -1.0% | [-8.1%, +6.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.97 ms | 1.95 ms | -1.2% | [-6.3%, +4.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.42 ms | 7.21 ms | -2.9% | [-5.1%, -0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 245 ms | 247 ms | +0.6% | [-3.0%, +4.4%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 338 ms | 338 ms | -0.2% | [-5.9%, +5.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 243 ms | 241 ms | -0.7% | [-4.5%, +3.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 260 ms | 261 ms | +0.3% | [-4.5%, +5.3%] | +0.0% | ~ | marched/traced 1.48x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 387 ms | 386 ms | -0.3% | [-8.7%, +9.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 258 ms | 258 ms | -0.3% | [-4.9%, +4.5%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.43 ms | 7.42 ms | -0.0% | [-1.9%, +1.8%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.8 ms | 12.7 ms | -0.4% | [-2.0%, +1.1%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 269 ms | 269 ms | +0.3% | [-2.8%, +3.5%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.6 ms | 12.7 ms | +0.3% | [-1.7%, +2.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.9 ms | 30.9 ms | -0.1% | [-4.0%, +3.9%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.7 ms | 22.6 ms | -0.3% | [-2.1%, +1.6%] | +0.0% | ~ | regions/hit 1.37x |
| `Regions/particles/region/double/host` | 7.72 ms | 7.71 ms | -0.2% | [-2.2%, +1.9%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 13.1 ms | 13.1 ms | -0.2% | [-2.2%, +1.9%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 283 ms | 282 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.0 ms | 13.0 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.6 ms | 32.3 ms | -0.9% | [-1.6%, -0.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.4 ms | 23.5 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | regions/hit 1.38x |
| `Render/csg/frame/float/host` | 19.9 ms | 19.4 ms | -2.6% | [-5.2%, +0.1%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 19.7 ms | 19.2 ms | -2.6% | [-7.0%, +2.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.3 ms | 19.9 ms | -1.7% | [-3.3%, +0.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 22.6 ms | 22.2 ms | -1.9% | [-3.7%, -0.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 22.2 ms | 22.2 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.6 ms | 21.6 ms | +0.2% | [-2.5%, +3.1%] | +0.0% | ~ |  |
| `Shapes/ellipsoid/signedDistance/float/host` | 33.0 ms | 33.2 ms | +0.7% | [+0.4%, +1.0%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/float/host` | 5.01 ms | 5.02 ms | +0.2% | [-0.5%, +0.9%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/float/host` | 4.23 ms | 4.19 ms | -0.9% | [-1.9%, +0.2%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/float/host` | 17.4 ms | 17.3 ms | -0.3% | [-1.1%, +0.5%] | - | ~ |  |
| `Shapes/tube/signedDistance/float/host` | 4.33 ms | 4.32 ms | -0.2% | [-0.5%, +0.2%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/float/host` | 4.18 ms | 4.18 ms | +0.1% | [-0.5%, +0.8%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/float/host` | 2.27 ms | 2.25 ms | -0.7% | [-2.8%, +1.4%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/float/host` | 1.47 ms | 1.46 ms | -0.5% | [-1.6%, +0.6%] | - | ~ |  |
| `Shapes/wedge/signedDistance/float/host` | 2.08 ms | 2.09 ms | +0.4% | [-0.2%, +0.9%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/float/host` | 27.5 ms | 27.3 ms | -0.9% | [-2.2%, +0.3%] | - | ~ |  |
| `Shapes/ellipsoid/signedDistance/double/host` | 46.2 ms | 46.2 ms | +0.1% | [-0.0%, +0.3%] | - | ~ |  |
| `Shapes/capped-cone/signedDistance/double/host` | 5.45 ms | 5.41 ms | -0.8% | [-1.9%, +0.3%] | - | ~ |  |
| `Shapes/round-cone/signedDistance/double/host` | 4.51 ms | 4.50 ms | -0.2% | [-1.7%, +1.4%] | - | ~ |  |
| `Shapes/polygon-prism/signedDistance/double/host` | 22.5 ms | 22.5 ms | -0.2% | [-1.0%, +0.7%] | - | ~ |  |
| `Shapes/tube/signedDistance/double/host` | 4.56 ms | 4.54 ms | -0.3% | [-0.5%, -0.2%] | - | ~ |  |
| `Shapes/capped-torus/signedDistance/double/host` | 5.26 ms | 5.35 ms | +1.7% | [-0.2%, +3.6%] | - | ~ |  |
| `Shapes/cut-sphere/signedDistance/double/host` | 2.53 ms | 2.49 ms | -1.6% | [-1.7%, -1.5%] | - | ~ |  |
| `Shapes/cut-hollow-sphere/signedDistance/double/host` | 1.80 ms | 1.79 ms | -0.6% | [-2.9%, +1.7%] | - | ~ |  |
| `Shapes/wedge/signedDistance/double/host` | 2.09 ms | 2.07 ms | -1.0% | [-2.5%, +0.6%] | - | ~ |  |
| `Shapes/ray-forms/firstHit/double/host` | 30.1 ms | 30.3 ms | +0.5% | [-0.5%, +1.1%] | - | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.5 ms | 56.4 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.4 ms | 36.4 ms | -0.0% | [-1.2%, +1.2%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.2 ms | 92.5 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.0 ms | 64.1 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.2 ms | 42.3 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 107 ms | 107 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.9 ms | 75.6 ms | +1.0% | [+0.9%, +1.1%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 296 ms | 297 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 298 ms | 298 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 87.2 ms | 87.1 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 348 ms | 348 ms | +0.1% | [-1.3%, +1.4%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 350 ms | 353 ms | +0.9% | [-0.3%, +2.1%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 167 ms | 166 ms | -0.6% | [-0.9%, -0.4%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 192 ms | 192 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 144 ms | 144 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 90.0 ms | 90.4 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ | surface/leaf 1.59x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 130 ms | 130 ms | +0.0% | [-0.9%, +1.0%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 73.0 ms | 73.1 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 61.3 ms | 61.5 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.3 ms | 34.4 ms | +0.2% | [-1.3%, +1.7%] | +0.0% | ~ | surface/hit 1.79x |
| `SurfaceParts/boxes/closestSurface/double/host` | 146 ms | 146 ms | +0.3% | [-0.6%, +1.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 90.8 ms | 90.7 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | surface/leaf 1.61x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 146 ms | 147 ms | +0.7% | [-0.8%, +2.1%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 82.0 ms | 81.4 ms | -0.7% | [-0.9%, -0.4%] | +0.0% | ~ | surface/leaf 1.80x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 68.4 ms | 68.5 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 38.9 ms | 38.9 ms | -0.1% | [-1.2%, +1.0%] | +0.0% | ~ | surface/hit 1.76x |
| `Tape/csg/signedDistance/float/host` | 12.8 ms | 12.8 ms | +0.4% | [-0.8%, +1.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.57 ms | 4.56 ms | -0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.1 ms | 24.9 ms | -0.9% | [-1.9%, +0.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 39.9 ms | 39.8 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.77 ms | 1.75 ms | -1.3% | [-2.7%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 131 µs | 134 µs | +1.8% | [-4.2%, +8.5%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.9 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.71 ms | 5.64 ms | -1.2% | [-3.3%, +1.0%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.2 ms | 16.2 ms | -0.2% | [-1.0%, +0.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.71 ms | 4.62 ms | -1.8% | [-3.5%, -0.1%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.2 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.62 ms | 4.59 ms | -0.7% | [-1.5%, +0.8%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.5 ms | 26.6 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.0 ms | 42.0 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.77 ms | 1.78 ms | +0.3% | [-3.0%, +3.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 284 µs | 292 µs | +2.8% | [-7.4%, +14.5%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.9 ms | 30.0 ms | +0.6% | [-0.3%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.79 ms | 7.81 ms | +0.2% | [-0.5%, +0.9%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.4 ms | 16.3 ms | -0.6% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.77 ms | 7.74 ms | -0.4% | [-0.6%, -0.2%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.9 ms | 41.9 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 200 ms | 200 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 48.3 ms | 43.0 ms | -11.0% | [-19.5%, -0.4%] | +0.0% | faster |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 217 ms | 218 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 652.4 s → 651.9 s (-0.1%), the largest peak 1399 MB → 1398 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.2 s | 9.6 s | +4.4% | 557 MB | 557 MB | +0.0% | ~ |
| `BVHRefit` | 4.1 s | 4.0 s | -2.2% | 484 MB | 484 MB | -0.1% | ~ |
| `BVHUnion` | 18.7 s | 18.5 s | -0.6% | 896 MB | 896 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.5 s | 7.4 s | -1.9% | 556 MB | 556 MB | +0.0% | ~ |
| `ClosestPrimitive` | 30.0 s | 30.2 s | +0.8% | 1296 MB | 1296 MB | +0.0% | ~ |
| `ClosestSurface` | 28.2 s | 28.3 s | +0.1% | 1218 MB | 1218 MB | -0.0% | ~ |
| `Domain` | 18.3 s | 18.0 s | -1.8% | 840 MB | 840 MB | +0.0% | ~ |
| `DomainObject` | 16.2 s | 15.9 s | -2.1% | 843 MB | 843 MB | -0.0% | ~ |
| `Fused` | 24.7 s | 24.3 s | -1.6% | 1126 MB | 1126 MB | +0.0% | ~ |
| `LaunchTuning` | 36.8 s | 36.2 s | -1.6% | 1303 MB | 1303 MB | +0.0% | ~ |
| `Mesh` | 15.3 s | 15.2 s | -0.8% | 784 MB | 784 MB | +0.0% | ~ |
| `MeshCSG` | 39.1 s | 39.8 s | +1.8% | 1255 MB | 1255 MB | +0.0% | ~ |
| `MeshReconcile` | 6.7 s | 6.6 s | -1.7% | 565 MB | 565 MB | +0.0% | ~ |
| `PointCloud` | 7.6 s | 7.8 s | +2.3% | 559 MB | 559 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 3.9 s | 3.7 s | -3.0% | 470 MB | 470 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | +1.7% | 452 MB | 452 MB | -0.1% | ~ |
| `Ray` | 36.7 s | 36.5 s | -0.5% | 1317 MB | 1317 MB | +0.0% | ~ |
| `RayCSG` | 31.5 s | 31.3 s | -0.5% | 1174 MB | 1174 MB | +0.0% | ~ |
| `RayCrossing` | 44.9 s | 46.0 s | +2.4% | 1333 MB | 1334 MB | +0.0% | ~ |
| `RayParticles` | 49.1 s | 48.4 s | -1.5% | 1318 MB | 1318 MB | -0.0% | ~ |
| `RayShell` | 37.5 s | 36.9 s | -1.6% | 1277 MB | 1277 MB | +0.0% | ~ |
| `Regions` | 28.8 s | 29.0 s | +0.5% | 1070 MB | 1069 MB | -0.0% | ~ |
| `Render` | 35.8 s | 35.8 s | +0.0% | 1282 MB | 1282 MB | -0.1% | ~ |
| `Shapes` | 8.9 s | 9.0 s | +0.5% | 554 MB | 554 MB | -0.0% | ~ |
| `SphereCloudBVH` | 4.9 s | 4.8 s | -1.5% | 511 MB | 510 MB | -0.0% | ~ |
| `SphereCloudRays` | 27.9 s | 27.9 s | +0.2% | 1060 MB | 1060 MB | -0.0% | ~ |
| `SphereContacts` | 4.4 s | 4.4 s | +0.7% | 484 MB | 484 MB | +0.0% | ~ |
| `SurfaceParts` | 41.1 s | 41.1 s | -0.0% | 1399 MB | 1398 MB | -0.0% | ~ |
| `Tape` | 16.7 s | 17.2 s | +2.9% | 818 MB | 818 MB | +0.0% | ~ |
| `TapeBVHUnion` | 14.6 s | 14.7 s | +0.5% | 823 MB | 823 MB | +0.0% | ~ |
| **all, in both** | 652.4 s | 651.9 s | -0.1% | 1399 MB | 1398 MB | -0.0% | ~ |


Flagged after 2 rounds, and judged on 10 (3 cases): PointCloudHashGridBuild/uniform/rebuild/float/host, Shapes/ray-forms/firstHit/double/host, Tape/csg-handwritten/signedDistance/double/host

<!-- END receipt host -->
