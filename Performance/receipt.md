<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
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
base: 6928e4e
base-fingerprint: 0ee78d5210969340a6894041b440fce47fd0195d
head: 2f624f6
head-fingerprint: 26cfab37bc47c3a758ded1fcbdb6fddff84b4bff
cases: 140
rounds: 3
reps: 5
date: 2026-10-25
-->

## Host comparison: green

**Accepted:** -

`2f624f6` against `6928e4e` (origin/dev), 2026-10-25.

**VERDICT: no regression (0 major, 0 minor across 140 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 140 cases

- base: 6928e4e (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 2f624f6 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 329 ms | 329 ms | +0.0% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | -0.3% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 515 ms | 515 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 346 ms | 347 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 559 ms | 560 ms | +0.1% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 54.7 ms | 54.6 ms | -0.2% | [-0.9%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.1 ms | 51.2 ms | +0.2% | [-0.0%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 63.8 ms | 63.7 ms | -0.1% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.4 ms | 54.8 ms | +0.7% | [-0.8%, +34.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.3 ms | 60.1 ms | -0.4% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.2 ms | 72.0 ms | -0.2% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 73.7 ms | 73.9 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.6 ms | 77.1 ms | -0.5% | [-0.8%, -0.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.0 ms | 67.0 ms | +0.1% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 99.9 ms | -0.3% | [-0.8%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.1 ms | 82.3 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 108 ms | +0.5% | [-0.1%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.2 ms | 19.2 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 43.6 ms | 43.7 ms | +0.2% | [-5.2%, +14.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 81.1 ms | 80.9 ms | -0.1% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.4 ms | 20.5 ms | +0.1% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.5 ms | 43.4 ms | -0.3% | [-8.0%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 91.5 ms | 92.1 ms | +0.6% | [-0.1%, +2.9%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.40 ms | 4.41 ms | +0.3% | [-0.2%, +1.3%] | - | ~ | tape/fused 2.87x |
| `Fused/smooth/signedDistance/float/host` | 12.2 ms | 12.2 ms | -0.1% | [-0.7%, +0.1%] | - | ~ | tape/fused 1.96x |
| `Fused/chain16/signedDistance/float/host` | 954 µs | 969 µs | +1.5% | [-3.2%, +1.9%] | - | ~ | tape/fused 16.14x |
| `Fused/drilled-mesh/signedDistance/float/host` | 47.7 ms | 47.8 ms | +0.1% | [-0.0%, +1.3%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 48.0 ms | 47.9 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.53 ms | 4.58 ms | +1.2% | [-0.3%, +1.5%] | - | ~ | tape/fused 2.91x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | -0.2% | [-0.8%, +0.9%] | - | ~ | tape/fused 2.09x |
| `Fused/chain16/signedDistance/double/host` | 3.90 ms | 3.88 ms | -0.6% | [-1.4%, +0.7%] | - | ~ | tape/fused 4.29x |
| `Fused/drilled-mesh/signedDistance/double/host` | 56.1 ms | 56.2 ms | +0.1% | [-1.7%, +1.6%] | +0.0% | ~ | tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 56.0 ms | 55.7 ms | -0.5% | [-0.6%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.9 ms | 82.1 ms | +0.2% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 237 ms | 237 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 112 ms | 112 ms | -0.2% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 278 ms | 278 ms | +0.1% | [-0.9%, +0.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.2 ms | 70.0 ms | -0.3% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 207 ms | 206 ms | -0.5% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.5 ms | 96.5 ms | +0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.8 ms | 29.8 ms | +0.1% | [-1.8%, +1.8%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 759 ms | 759 ms | -0.0% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | +0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.4 ms | 44.2 ms | +1.8% | [-1.4%, +5.4%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 848 ms | 846 ms | -0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 67.1 ms | 67.0 ms | -0.2% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 147 ms | 147 ms | -0.1% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 373 ms | 373 ms | -0.0% | [-0.1%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.3 ms | 78.2 ms | -0.0% | [-0.3%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.64 ms | 6.68 ms | +0.5% | [-0.2%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/float/host` | 144 ms | 143 ms | -0.1% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/float/host` | 340 ms | 340 ms | +0.1% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/float/host` | 182 ms | 182 ms | -0.1% | [-0.1%, +0.0%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 79.8 ms | 79.7 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 162 ms | 160 ms | -1.2% | [-1.7%, -0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 401 ms | 401 ms | -0.1% | [-0.5%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.4 ms | 87.3 ms | -0.2% | [-1.0%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.12 ms | 7.11 ms | -0.1% | [-1.0%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbor-morton/double/host` | 150 ms | 151 ms | +0.1% | [-0.2%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/neighbors8-morton/double/host` | 347 ms | 348 ms | +0.1% | [-0.3%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest8-morton/double/host` | 190 ms | 189 ms | -0.2% | [-0.8%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.4 ms | 11.4 ms | -0.1% | [-1.0%, +3.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 122 ms | 122 ms | +0.0% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 512 ms | 513 ms | +0.2% | [-0.3%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 152 ms | +0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 846 ms | 844 ms | -0.2% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/float/host` | 119 ms | 119 ms | +0.1% | [-0.2%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/host` | 489 ms | 489 ms | +0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/float/host` | 348 ms | 348 ms | +0.0% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 15.1 ms | +0.7% | [-0.2%, +7.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 126 ms | 126 ms | +0.2% | [-0.5%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 545 ms | 545 ms | +0.1% | [-0.9%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 155 ms | 155 ms | +0.2% | [-0.9%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 884 ms | 887 ms | +0.3% | [-0.0%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbor-morton/double/host` | 121 ms | 121 ms | +0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/host` | 497 ms | 497 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest8-morton/double/host` | 351 ms | 352 ms | +0.0% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 66.7 ms | 66.5 ms | -0.2% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 109 ms | 109 ms | +0.2% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 269 ms | 270 ms | +0.3% | [-0.3%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 112 ms | 112 ms | +0.0% | [-0.1%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 106 ms | 106 ms | +0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/float/host` | 107 ms | 107 ms | -0.0% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/float/host` | 240 ms | 240 ms | +0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/float/host` | 190 ms | 190 ms | -0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 79.2 ms | 79.3 ms | +0.1% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 120 ms | 120 ms | +0.4% | [-0.3%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 294 ms | 295 ms | +0.2% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 127 ms | 127 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 121 ms | 121 ms | -0.2% | [-0.5%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbor-morton/double/host` | 112 ms | 112 ms | +0.2% | [-0.5%, +2.3%] | - | ~ |  |
| `PointCloudBVH/sphere/neighbors8-morton/double/host` | 243 ms | 245 ms | +0.5% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest8-morton/double/host` | 199 ms | 199 ms | +0.0% | [-0.8%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.78 ms | 8.74 ms | -0.4% | [-36.2%, +17.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 140 ms | 146 ms | +4.3% | [-4.4%, +4.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 398 ms | 386 ms | -3.2% | [-3.6%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 250 ms | 250 ms | -0.0% | [-1.0%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 329 ms | 330 ms | +0.2% | [-1.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/float/host` | 139 ms | 139 ms | +0.0% | [-4.4%, +4.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/host` | 365 ms | 377 ms | +3.3% | [-0.1%, +3.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/float/host` | 347 ms | 347 ms | -0.0% | [-0.1%, +2.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.2 ms | 12.2 ms | -0.1% | [-0.8%, +2.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 144 ms | 150 ms | +4.2% | [-4.2%, +5.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 418 ms | 430 ms | +3.0% | [-2.8%, +3.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 282 ms | 282 ms | -0.2% | [-0.7%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 345 ms | 344 ms | -0.5% | [-0.9%, -0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbor-morton/double/host` | 140 ms | 140 ms | -0.1% | [-4.4%, +4.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/host` | 384 ms | 384 ms | -0.1% | [-3.2%, +3.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest8-morton/double/host` | 351 ms | 351 ms | -0.0% | [-2.4%, +2.4%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | - | - | - | - | - | new | hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled/float/host` | - | - | - | - | - | new |  |
| `Ray/mesh/intersects/float/host` | - | - | - | - | - | new |  |
| `Ray/csg-fused/firstHit/float/host` | - | - | - | - | - | new | tape/fused 1.53x |
| `Ray/csg-tape/firstHit/float/host` | - | - | - | - | - | new |  |
| `Ray/mesh/firstHit/double/host` | - | - | - | - | - | new | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | - | - | - | - | - | new |  |
| `Ray/mesh/intersects/double/host` | - | - | - | - | - | new |  |
| `Ray/csg-fused/firstHit/double/host` | - | - | - | - | - | new | tape/fused 1.48x |
| `Ray/csg-tape/firstHit/double/host` | - | - | - | - | - | new |  |
| `Tape/csg/signedDistance/float/host` | 12.7 ms | 12.7 ms | -0.2% | [-1.4%, +1.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.46 ms | 4.46 ms | +0.1% | [-1.0%, +0.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 23.9 ms | 23.8 ms | -0.3% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.3 ms | 40.2 ms | -0.2% | [-8.9%, -0.0%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.73 ms | 1.74 ms | +0.7% | [-1.6%, +1.5%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 126 µs | 126 µs | +0.0% | [-1.2%, +9.3%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.7 ms | 29.7 ms | +0.0% | [-0.1%, +1.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.65 ms | 5.65 ms | -0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 15.6 ms | 15.6 ms | +0.3% | [-0.9%, +0.8%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.63 ms | 4.65 ms | +0.5% | [-0.6%, +1.5%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.4 ms | 13.3 ms | -0.3% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.52 ms | 4.52 ms | +0.0% | [-0.4%, +2.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.4 ms | 26.5 ms | +0.3% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 41.8 ms | 41.6 ms | -0.5% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.73 ms | 1.73 ms | -0.4% | [-3.2%, +1.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 284 µs | 285 µs | +0.1% | [-10.9%, +11.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.8 ms | -0.1% | [-1.1%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.71 ms | 7.72 ms | +0.1% | [-0.1%, +0.9%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.8 ms | 16.6 ms | -0.9% | [-1.9%, -0.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.73 ms | 7.72 ms | -0.2% | [-0.8%, +0.1%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/Ray/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 6928e4e
base-fingerprint: 0ee78d5210969340a6894041b440fce47fd0195d
head: 2f624f6
head-fingerprint: 26cfab37bc47c3a758ded1fcbdb6fddff84b4bff
cases: 128
rounds: 3
reps: 5
date: 2026-10-25
-->

## Device comparison (CUDA): red

**Accepted:** 2 rays graze a drill rim within the march tolerance; host and device round differently. Expected for marched rays.

`2f624f6` against `6928e4e` (origin/dev), 2026-10-25.

**VERDICT: host and device answers differ in 4 cases

<!-- END receipt cuda -->
