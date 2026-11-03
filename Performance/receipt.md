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
base: 4c87e5d
base-fingerprint: 60bfcfc855044f2fa7a4133c206533c55abdfd03
head: 0997853
head-fingerprint: 83ce77fa4c17a149f4eefac9983ef062abf7eaae
cases: 186
rounds: 2
reps: 3
date: 2026-11-03
-->

## Host comparison: green

**Accepted:** -

`0997853` against `4c87e5d` (origin/main), 2026-11-03.

**VERDICT: no regression (0 major, 0 minor across 186 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 186 cases

- base: 4c87e5d (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 0997853 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 331 ms | 330 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 179 ms | 179 ms | +0.5% | [-0.0%, +0.9%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 513 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 349 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 561 ms | 561 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/float/host` | - | - | - | - | - | new |  |
| `BVHRefit/uniform/refit/double/host` | - | - | - | - | - | new |  |
| `BVHUnion/spheres/build/float/host` | 55.3 ms | 55.2 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.4 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.1 ms | 65.3 ms | +0.3% | [-0.3%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.4 ms | 55.0 ms | +1.0% | [+0.5%, +1.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.6 ms | 60.5 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.4 ms | 72.7 ms | +0.4% | [-0.5%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 75.3 ms | 75.5 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.6 ms | 77.7 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.6 ms | 67.1 ms | -0.7% | [-0.9%, -0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.8 ms | 100 ms | +0.6% | [+0.1%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.8 ms | 83.9 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.5 ms | 19.5 ms | -0.2% | [-1.5%, +1.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.7 ms | 45.2 ms | -1.2% | [-2.1%, -0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.7 ms | 72.9 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 19.9 ms | 20.0 ms | +0.2% | [-0.8%, +1.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 47.0 ms | 47.1 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 80.9 ms | 80.8 ms | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.47 ms | 4.47 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | tape/fused 2.02x |
| `Fused/chain16/signedDistance/float/host` | 968 µs | 969 µs | +0.1% | [-0.0%, +0.1%] | - | ~ | tape/fused 16.63x (was 18.12x) |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.7 ms | +1.2% | [+0.1%, +2.3%] | +0.0% | ~ | tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.5 ms | 46.3 ms | -0.4% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.56 ms | 4.53 ms | -0.6% | [-1.2%, -0.1%] | - | ~ | tape/fused 2.89x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | -0.2% | [-0.5%, -0.0%] | - | ~ | tape/fused 2.07x |
| `Fused/chain16/signedDistance/double/host` | 3.86 ms | 3.86 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | tape/fused 4.21x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.2 ms | 52.5 ms | +0.7% | [+0.2%, +1.1%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.4 ms | 52.6 ms | -1.4% | [-2.8%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.4 ms | 82.4 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 228 ms | 229 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 261 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.6 ms | 70.3 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 214 ms | -0.3% | [-1.3%, +0.6%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.5 ms | 96.7 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.8 ms | 30.1 ms | +0.9% | [+0.0%, +1.7%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 758 ms | 755 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.1 ms | 43.8 ms | -0.6% | [-4.7%, +3.8%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 858 ms | 855 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.7 ms | 26.7 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.1 ms | 26.9 ms | -0.4% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 320 ms | 321 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 240 ms | 239 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.3 ms | -0.1% | [-0.1%, +0.0%] | +0.0% | ~ | tape/fused 3.55x |
| `MeshCSG/smooth-fused/frame/float/host` | 101 ms | 99.9 ms | -0.8% | [-1.1%, -0.4%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.5 ms | 29.8 ms | +0.8% | [-0.6%, +2.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.1 ms | 30.4 ms | +1.0% | [+0.1%, +1.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 354 ms | 359 ms | +1.4% | [+0.9%, +1.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 255 ms | 254 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.3 ms | 96.9 ms | -0.4% | [-0.9%, +0.0%] | +0.0% | ~ | tape/fused 3.70x |
| `MeshCSG/smooth-fused/frame/double/host` | 110 ms | 109 ms | -1.1% | [-2.9%, +0.8%] | +0.0% | ~ | tape/fused 2.33x |
| `PointCloudBVH/uniform/build/float/host` | 53.1 ms | 53.1 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 143 ms | 143 ms | +0.6% | [+0.0%, +1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 352 ms | 352 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 77.7 ms | 78.0 ms | +0.4% | [+0.2%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.65 ms | 6.61 ms | -0.5% | [-1.1%, +0.0%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 56.4 ms | 56.5 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 154 ms | 155 ms | +0.5% | [+0.1%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 379 ms | 379 ms | -0.1% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.6 ms | 87.5 ms | +1.0% | [+0.8%, +1.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 6.90 ms | 6.88 ms | -0.2% | [-1.1%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 13.6 ms | 13.6 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 118 ms | 118 ms | -0.3% | [-0.4%, -0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 476 ms | 476 ms | -0.1% | [-0.1%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 142 ms | 141 ms | -0.0% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 800 ms | 799 ms | -0.1% | [-0.1%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.8 ms | 14.8 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 120 ms | 120 ms | +0.2% | [+0.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 497 ms | 496 ms | -0.2% | [-0.3%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 145 ms | 146 ms | +0.2% | [-0.6%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 790 ms | 788 ms | -0.3% | [-0.9%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 54.2 ms | 54.4 ms | +0.4% | [+0.0%, +0.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 129 ms | 128 ms | -0.6% | [-1.4%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 263 ms | 262 ms | -0.6% | [-0.9%, -0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 144 ms | 144 ms | +0.3% | [+0.1%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 129 ms | 129 ms | +0.2% | [+0.1%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 58.3 ms | 57.9 ms | -0.8% | [-2.4%, +0.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 137 ms | 137 ms | -0.0% | [-0.8%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 289 ms | 289 ms | +0.0% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 162 ms | 161 ms | -0.1% | [-1.0%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 143 ms | 144 ms | +0.5% | [-0.1%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 11.7 ms | 11.6 ms | -0.9% | [-1.6%, -0.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 116 ms | 114 ms | -1.9% | [-2.2%, -1.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 319 ms | 318 ms | -0.2% | [-1.8%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 157 ms | 155 ms | -1.2% | [-2.0%, -0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 320 ms | 321 ms | +0.3% | [+0.2%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.6 ms | 13.6 ms | +0.5% | [-0.5%, +1.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 116 ms | 117 ms | +0.9% | [-0.2%, +1.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 348 ms | 348 ms | -0.0% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 166 ms | 168 ms | +1.0% | [+0.5%, +1.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 308 ms | 308 ms | -0.2% | [-0.6%, +0.2%] | - | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | 35.1 ms | 35.1 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | 38.8 ms | 38.8 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | 8.20 ms | 8.32 ms | +1.5% | [-0.7%, +3.7%] | +0.0% | ~ |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | 9.38 ms | 9.28 ms | -1.0% | [-1.9%, -0.1%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.4 ms | 46.3 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.6 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.3 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.4 ms | 78.7 ms | +0.3% | [-0.3%, +1.0%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.4 ms | 65.9 ms | +0.7% | [+0.2%, +1.3%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.93 ms | 5.91 ms | -0.4% | [-1.0%, +0.1%] | - | ~ | tape/fused 1.34x |
| `Ray/csg-tape/firstHit/float/host` | 7.95 ms | 7.94 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.3 ms | 52.4 ms | +0.1% | [-0.5%, +0.8%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.3 ms | 51.3 ms | -0.1% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.7 ms | 44.8 ms | +0.2% | [-0.6%, +1.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.6 ms | 87.6 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.7 ms | 74.4 ms | +0.9% | [+0.3%, +1.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 6.02 ms | 6.00 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | tape/fused 1.38x |
| `Ray/csg-tape/firstHit/double/host` | 8.37 ms | 8.26 ms | -1.3% | [-2.6%, -0.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.8 ms | 18.7 ms | -0.5% | [-1.7%, +0.6%] | - | ~ | marched/traced 6.36x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 119 ms | -0.1% | [-0.1%, +0.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.8 ms | 21.8 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ | marched/traced 7.08x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 155 ms | 155 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.3 ms | 35.4 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 18.9 ms | 19.1 ms | +0.9% | [-0.1%, +2.0%] | - | ~ | marched/traced 6.64x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 127 ms | +0.6% | [+0.4%, +0.8%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 23.4 ms | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ | marched/traced 6.83x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 160 ms | 160 ms | -0.1% | [-0.1%, -0.1%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.0 ms | 37.9 ms | -0.3% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.4 ms | 46.3 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.3 ms | 46.3 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.3 ms | 10.2 ms | -0.5% | [-1.5%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.76 ms | 8.79 ms | +0.4% | [-0.1%, +1.0%] | +0.0% | ~ | hit/crossing 1.16x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 181 ms | 181 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 161 ms | 161 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 69.1 ms | 68.9 ms | -0.4% | [-1.5%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.5 ms | 56.4 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.8 ms | 52.7 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.7 ms | 52.6 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.1 ms | 11.1 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.37 ms | 9.40 ms | +0.4% | [-0.1%, +0.8%] | +0.0% | ~ | hit/crossing 1.19x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 206 ms | 209 ms | +1.2% | [+0.4%, +2.0%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 184 ms | 184 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | hit/crossing 1.14x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 79.4 ms | 79.0 ms | -0.5% | [-3.0%, +2.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 67.0 ms | 65.2 ms | -2.7% | [-4.0%, -1.3%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.0 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | marched/traced 4.73x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.6 ms | 52.0 ms | +0.8% | [+0.1%, +1.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 7.96 ms | 7.97 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.91 ms | 1.92 ms | +0.5% | [-0.2%, +1.2%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.89 ms | 6.96 ms | +1.0% | [+0.7%, +1.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.9 ms | 11.7 ms | -1.5% | [-4.1%, +1.1%] | +0.0% | ~ | marched/traced 5.21x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.1 ms | 61.1 ms | -0.1% | [-1.2%, +1.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.64 ms | 8.61 ms | -0.3% | [-1.8%, +1.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.95 ms | 1.95 ms | +0.1% | [-2.0%, +2.2%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.25 ms | 7.24 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 238 ms | 237 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 327 ms | 327 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 235 ms | 237 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 253 ms | 253 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ | marched/traced 1.48x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 373 ms | 375 ms | +0.5% | [-0.7%, +1.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 251 ms | 252 ms | +0.4% | [-0.1%, +0.8%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.4 ms | 23.5 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.7 ms | 23.5 ms | -0.6% | [-1.3%, +0.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.1 ms | 18.6 ms | -2.5% | [-5.9%, +1.0%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.0 ms | 30.0 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 30.0 ms | 29.8 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.7 ms | 20.4 ms | -1.2% | [-1.9%, -0.4%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.56 ms | 4.52 ms | -0.9% | [-1.4%, -0.4%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.9 ms | 24.8 ms | -0.6% | [-1.2%, -0.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.2 ms | 40.2 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.73 ms | 1.73 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 134 µs | 126 µs | -6.4% | [-11.6%, -0.5%] | - | faster |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.0 ms | 29.9 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.67 ms | 5.67 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 17.5 ms | 16.1 ms | -8.2% | [-15.9%, +0.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.64 ms | 4.64 ms | -0.1% | [-0.6%, +0.4%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.1 ms | -0.4% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.52 ms | 4.54 ms | +0.6% | [-0.8%, +2.0%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.4 ms | 26.3 ms | -0.2% | [-1.3%, +1.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.3 ms | 42.2 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.76 ms | 1.72 ms | -2.1% | [-3.5%, -0.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 262 µs | 256 µs | -2.2% | [-4.6%, +0.3%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.6 ms | 29.6 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.77 ms | 7.74 ms | -0.4% | [-0.9%, +0.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.1 ms | 16.3 ms | +0.9% | [+0.6%, +1.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.75 ms | 7.78 ms | +0.4% | [+0.0%, +0.8%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/BVHRefit/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 4c87e5d
base-fingerprint: 60bfcfc855044f2fa7a4133c206533c55abdfd03
head: 0997853
head-fingerprint: 83ce77fa4c17a149f4eefac9983ef062abf7eaae
cases: 278
rounds: 2
reps: 3
date: 2026-11-03
-->

## Device comparison (CUDA): yellow

**Accepted:** Accept as noise

`0997853` against `4c87e5d` (origin/main), 2026-11-03.

**VERDICT: 6 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 6 minor, 5 faster, 0 differ, 0 lost, 278 cases

- base: 4c87e5d (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 0997853 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.60 ms | -0.7% | [-1.5%, +0.1%] | - | ~ | device 22.8x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.40 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.04 ms | -0.8% | [-2.9%, +1.3%] | - | ~ | device 19.7x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.30 ms | -0.4% | [-1.2%, +0.4%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.19 ms | 6.15 ms | -0.5% | [-1.5%, +0.4%] | - | ~ | device 7.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.7 ms | 11.8 ms | +0.4% | [-0.4%, +1.3%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.4 ms | 33.4 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 56.9 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.8 ms | 20.5 ms | -1.5% | [-2.5%, -0.4%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.9 ms | 47.8 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 62.0 ms | 60.9 ms | -1.7% | [-3.8%, +0.3%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 108 ms | 109 ms | +0.8% | [-3.0%, +4.8%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `BVHRefit/uniform/refit/float/hostdevice` | - | - | - | - | - | new |  |
| `BVHRefit/uniform/refit/double/hostdevice` | - | - | - | - | - | new |  |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 385 µs | 386 µs | +0.2% | [-0.6%, +1.1%] | - | ~ | device 15.3x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 840 µs | 841 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.53 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.68 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 581 µs | 578 µs | -0.4% | [-1.3%, +0.4%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.36 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.76 ms | 6.77 ms | +0.2% | [-0.5%, +0.8%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.03 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 900 µs | 898 µs | -0.2% | [-0.6%, +0.1%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.45 ms | 9.47 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.2% | [-0.5%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 320 µs | 319 µs | -0.4% | [-0.7%, -0.1%] | - | ~ | device 11.3x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.3% | [+0.2%, +0.3%] | - | ~ | device 5.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.8 ms | 58.9 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 3.4x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.55 ms | 5.54 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 519 ms | 523 ms | +0.7% | [+0.7%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.3% | [-0.7%, +0.2%] | - | ~ | device 23.8x faster than the host; tape/fused 4.56x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.6 µs | 49.6 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 43.0x faster than the host; tape/fused 7.29x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.0 µs | 48.4 µs | +0.8% | [+0.0%, +1.5%] | - | ~ | device 8.9x faster than the host (was 9.4x); tape/fused 9.79x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.2 ms | 17.4 ms | +0.7% | [+0.0%, +1.4%] | - | ~ | device 6.7x faster than the host (was 7.6x); tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.4 ms | 31.3 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 809 µs | 810 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 285 ms | 280 ms | -1.6% | [-3.4%, +0.2%] | - | ~ | device 0.4x faster than the host; tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 279 ms | 278 ms | -0.4% | [-2.3%, +1.5%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.26 ms | 8.21 ms | -0.5% | [-1.8%, +0.7%] | +0.0% | ~ | device 10.8x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.37 ms | 8.29 ms | -0.9% | [-1.6%, -0.2%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.24 ms | 8.22 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | device 10.7x faster than the host (was 11.6x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.23 ms | 8.22 ms | -0.1% | [-1.6%, +1.4%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 689 µs | 689 µs | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ | device 4.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.73 ms | 1.73 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.06 ms | 1.06 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ | device 3.1x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 799 µs | 799 µs | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | device 4.1x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 831 µs | 838 µs | +0.8% | [+0.1%, +1.4%] | +0.0% | ~ | device 10.5x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 963 µs | 966 µs | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 9.1x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 829 µs | 832 µs | +0.4% | [+0.4%, +0.5%] | +0.0% | ~ | device 10.6x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 813 µs | 813 µs | +0.0% | [-1.1%, +1.2%] | +0.0% | ~ | device 10.7x faster than the host (was 11.3x); default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 161 ms | 158 ms | -1.5% | [-4.7%, +2.0%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 161 ms | 160 ms | -0.6% | [-1.9%, +0.7%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 168 ms | 164 ms | -2.6% | [-4.8%, -0.5%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.97x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 159 ms | 159 ms | -0.6% | [-1.8%, +0.7%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.77 ms | 1.77 ms | +0.0% | [-0.4%, +0.5%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.90x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.94 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.82x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.68 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.27 ms | 7.30 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.31 ms | 7.30 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.27 ms | 7.29 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.31 ms | 7.32 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.41 ms | 8.41 ms | -0.0% | [-1.1%, +1.0%] | - | ~ | device 11.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.5 ms | 16.5 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 8.9x faster than the host (was 7.1x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 165 ms | 165 ms | +0.3% | [-4.0%, +4.7%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 276 ms | 283 ms | +2.6% | [+1.2%, +4.1%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.5 ms | 24.4 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 58.5 ms | 58.7 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 2.5x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 158 ms | 160 ms | +1.3% | [+0.5%, +2.1%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 242 ms | 247 ms | +2.0% | [+0.5%, +3.5%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.93 ms | 2.93 ms | +0.0% | [-1.9%, +2.0%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.14 ms | 5.13 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.5 ms | 14.6 ms | +1.1% | [+0.3%, +1.9%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 123 ms | 121 ms | -2.0% | [-6.4%, +2.8%] | - | ~ | device 1.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 176 ms | 174 ms | -1.0% | [-3.6%, +1.6%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 376 ms | 381 ms | +1.2% | [-0.0%, +2.5%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.8 ms | 25.0 ms | +0.5% | [-0.2%, +1.3%] | - | ~ | device 2.0x faster than the host; tape/fused 4.83x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 26.7 ms | -0.7% | [-1.7%, +0.3%] | - | ~ | device 2.2x faster than the host; tape/fused 6.53x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 72.5 ms | 70.7 ms | -2.5% | [-4.5%, -0.4%] | - | ~ | device 4.9x faster than the host (was 4.1x); tape/fused 5.38x (was 5.18x) |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.2 ms | 22.7 ms | +2.1% | [-1.8%, +6.3%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.7 ms | 45.1 ms | +0.9% | [-1.2%, +3.1%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 94.4 ms | 96.6 ms | +2.3% | [+1.9%, +2.7%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 539 ms | 566 ms | +4.9% | [+0.8%, +9.3%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 853 ms | 864 ms | +1.3% | [-0.3%, +3.0%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.29 s | 3.26 s | -1.0% | [-3.2%, +1.3%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 203 ms | 197 ms | -2.9% | [-5.6%, -0.2%] | - | ~ | device 0.2x faster than the host; tape/fused 2.88x (was 2.66x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 314 ms | 313 ms | -0.5% | [-6.1%, +5.3%] | - | ~ | device 0.2x faster than the host; tape/fused 2.76x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 584 ms | 599 ms | +2.5% | [-0.0%, +5.0%] | - | ~ | device 0.5x faster than the host; tape/fused 5.44x (was 5.63x) |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 763 µs | 763 µs | +0.0% | [-0.4%, +0.4%] | - | ~ | device 9.8x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.17 ms | 2.17 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 4.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.17 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 7.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.20 ms | 6.20 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 3.87 ms | 3.88 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.17 ms | 7.16 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.87 ms | 7.90 ms | +0.5% | [+0.3%, +0.6%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.4 ms | 13.4 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 21.0 ms | 21.0 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 12.0 ms | 12.0 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 1.44 ms | 1.47 ms | +2.2% | [-0.3%, +4.8%] | - | ~ | device 6.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 3.89 ms | 3.89 ms | -0.1% | [-0.7%, +0.6%] | - | ~ | device 3.3x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 527 µs | 524 µs | -0.6% | [-1.4%, +0.3%] | - | ~ | device 16.2x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 4.43 ms | 4.39 ms | -0.9% | [-1.6%, -0.3%] | - | ~ | device 6.9x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 4.30 ms | 4.31 ms | +0.1% | [-0.8%, +1.0%] | - | ~ | device 5.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.10 ms | 8.10 ms | +0.0% | [-2.6%, +2.6%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 15.8 ms | -0.2% | [-0.7%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.00 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 17.6 ms | 17.7 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 14.8 ms | 14.9 ms | +1.0% | [+0.4%, +1.5%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.80 ms | 1.82 ms | +1.3% | [+0.7%, +1.8%] | - | ~ | device 9.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 5.53 ms | 5.53 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 4.3x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 919 µs | 917 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 9.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.70 ms | 3.70 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 5.47 ms | 5.46 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 4.1x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 16.8 ms | 16.8 ms | -0.4% | [-1.2%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 21.4 ms | 21.4 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 8.96 ms | 8.96 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 14.6 ms | 14.6 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 20.2 ms | 20.4 ms | +0.7% | [-0.0%, +1.4%] | - | ~ | device 1.1x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 2.75 ms | 2.75 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 4.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 6.74 ms | 6.73 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.39 ms | 1.38 ms | -0.8% | [-1.4%, -0.2%] | - | ~ | device 6.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 2.71 ms | 2.71 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 7.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 6.51 ms | 6.53 ms | +0.4% | [-0.3%, +1.2%] | - | ~ | device 2.9x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.8 ms | 33.7 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 0.4x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.5 ms | 68.4 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 0.2x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.4 ms | -0.4% | [-0.8%, -0.1%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.8 ms | 20.9 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.3 ms | 38.0 ms | -0.6% | [-1.9%, +0.7%] | - | ~ | device 0.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | 12.8 ms | 12.8 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | 9.64 ms | 9.67 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | 12.8 ms | 12.8 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 3.6x faster than the host |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | 10.2 ms | 10.2 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | 3.28 ms | 3.29 ms | +0.2% | [-0.3%, +0.8%] | +0.0% | ~ | device 3.7x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | 2.60 ms | 2.60 ms | -0.1% | [-1.6%, +1.4%] | +0.0% | ~ | device 3.5x faster than the host |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | 4.55 ms | 4.56 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | device 2.6x faster than the host |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | 3.06 ms | 3.04 ms | -0.6% | [-1.8%, +0.7%] | +0.0% | ~ | device 3.3x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 667 µs | 670 µs | +0.5% | [-0.8%, +1.7%] | - | ~ | device 16.9x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.29 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 14.5x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 637 µs | 643 µs | +1.0% | [+0.2%, +1.7%] | - | ~ | device 17.3x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.23 ms | -0.2% | [-1.0%, +0.7%] | - | ~ | device 15.1x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 604 µs | 613 µs | +1.6% | [+1.2%, +2.1%] | - | ~ | device 15.8x faster than the host (was 16.4x) |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | -0.2% | [-1.0%, +0.5%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.18 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.48 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 11.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.17 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 611 µs | 594 µs | -2.9% | [-6.0%, +0.4%] | - | ~ | device 2.6x faster than the host (was 2.5x); tape/fused 1.16x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 692 µs | 728 µs | +5.2% | [+0.7%, +10.2%] | - | minor | device 2.5x faster than the host (was 2.7x); tape/fused 1.15x (was 1.21x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 693 µs | 688 µs | -0.8% | [-1.0%, -0.6%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 835 µs | 837 µs | +0.2% | [-0.3%, +0.7%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 16.9 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 0.7x faster than the host; hand/api 1.01x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.9% | [+0.4%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.3% | [-0.7%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 15.2 ms | +1.4% | [-0.1%, +2.9%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.4 ms | 24.5 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.4 ms | 16.5 ms | +0.8% | [-0.1%, +1.7%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.02x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.4 ms | 24.4 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.13x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.1 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.47 ms | -0.6% | [-1.1%, -0.2%] | - | ~ | device 1.1x faster than the host; tape/fused 1.09x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.61 ms | 2.60 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.73 ms | 2.72 ms | -0.3% | [-0.7%, +0.0%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.56 ms | 2.56 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.9x faster than the host; marched/traced 3.53x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.7x faster than the host; marched/traced 4.61x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.03 ms | 9.04 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 4.4x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.17 ms | 2.18 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 3.9x faster than the host; marched/traced 16.44x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.46 ms | 5.46 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.8x faster than the host; marched/traced 13.15x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 35.9 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.2 ms | 71.8 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 1.6x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.25 ms | 5.23 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.81 ms | 8.82 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.20 ms | 6.24 ms | +0.7% | [+0.0%, +1.4%] | - | ~ | device 1.2x faster than the host; marched/traced 25.88x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 0.8x faster than the host; marched/traced 24.02x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 165 ms | 161 ms | -1.9% | [-3.5%, -0.3%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 255 ms | 255 ms | -0.1% | [-1.3%, +1.1%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.85 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 1.8x faster than the host; marched/traced 23.35x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.5% | [-0.1%, +1.0%] | - | ~ | device 1.3x faster than the host; marched/traced 17.58x (was 18.19x) |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 112 ms | 113 ms | +0.9% | [-1.8%, +3.6%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 221 ms | 215 ms | -2.9% | [-3.4%, -2.4%] | - | ~ | device 0.6x faster than the host (was 0.5x) |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.4 ms | +0.1% | [-0.9%, +1.1%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 671 µs | 678 µs | +1.0% | [+0.3%, +1.8%] | - | ~ | device 17.0x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 673 µs | 676 µs | +0.4% | [-0.3%, +1.2%] | - | ~ | device 17.1x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.29 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 14.9x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 688 µs | 694 µs | +0.8% | [+0.3%, +1.3%] | - | ~ | device 4.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 839 µs | 836 µs | -0.4% | [-1.1%, +0.3%] | - | ~ | device 4.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 629 µs | 629 µs | +0.1% | [-0.3%, +0.4%] | - | ~ | device 4.4x faster than the host; hit/crossing 1.10x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 648 µs | 644 µs | -0.6% | [-0.8%, -0.3%] | - | ~ | device 4.8x faster than the host; hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.71 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 10.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.18 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 10.1x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.09 ms | 9.11 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 270 ms | 266 ms | -1.2% | [-3.9%, +1.5%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 220 ms | 212 ms | -3.5% | [-4.1%, -2.9%] | - | faster | device 0.9x faster than the host (was 0.8x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 55.4 ms | 54.9 ms | -0.9% | [-2.3%, +0.6%] | - | ~ | device 2.6x faster than the host; hit/crossing 4.85x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 71.1 ms | 71.1 ms | -0.1% | [-1.4%, +1.3%] | - | ~ | device 2.2x faster than the host (was 2.3x); hit/crossing 2.98x (was 3.09x) |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.3% | [-0.4%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 16.9 ms | 16.9 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.3 ms | 27.5 ms | +0.4% | [-0.5%, +1.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.73 ms | 2.72 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.29 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.35 ms | 1.34 ms | -0.3% | [-0.4%, -0.1%] | - | ~ | device 2.6x faster than the host; hit/crossing 2.03x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.7 ms | 64.5 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 121 ms | 123 ms | +1.7% | [+0.8%, +2.6%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.5 ms | 58.7 ms | +0.5% | [-0.1%, +1.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 112 ms | 112 ms | -0.0% | [-1.5%, +1.5%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 503 ms | 472 ms | -6.2% | [-10.8%, -1.3%] | - | faster | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 707 ms | 705 ms | -0.3% | [-1.1%, +0.5%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 384 ms | 383 ms | -0.4% | [-2.8%, +2.1%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.23x (was 1.31x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 535 ms | 541 ms | +1.1% | [-1.1%, +3.4%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.30x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.56 ms | 3.64 ms | +2.4% | [-1.9%, +7.1%] | - | ~ | device 1.2x faster than the host; marched/traced 5.40x (was 5.58x) |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.80 ms | 4.74 ms | -1.1% | [-2.7%, +0.5%] | - | ~ | device 0.9x faster than the host; marched/traced 8.48x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.9 ms | 19.7 ms | -1.0% | [-2.2%, +0.3%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.2 ms | 40.2 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.26 ms | 1.22 ms | -3.2% | [-4.1%, -2.2%] | - | faster | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.84 ms | 1.85 ms | +0.5% | [-0.4%, +1.4%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 94.0 µs | 96.7 µs | +2.9% | [-1.1%, +7.1%] | - | ~ | device 15.1x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 153 µs | 151 µs | -1.2% | [-2.6%, +0.1%] | - | ~ | device 10.9x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 797 µs | 801 µs | +0.5% | [+0.2%, +0.9%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 934 µs | 936 µs | +0.2% | [-0.1%, +0.6%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.7 ms | 34.8 ms | -2.5% | [-6.9%, +2.0%] | - | ~ | device 0.1x faster than the host; marched/traced 3.57x (was 3.38x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.5 ms | 36.4 ms | -0.2% | [-1.3%, +0.9%] | - | ~ | device 0.1x faster than the host; marched/traced 6.48x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 120 ms | 124 ms | +3.1% | [+0.1%, +6.2%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 238 ms | 236 ms | -0.8% | [-4.6%, +3.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 17.3 ms | 16.2 ms | -6.3% | [-8.6%, -4.0%] | - | faster | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 21.8 ms | -0.2% | [-1.1%, +0.8%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.09 ms | 1.05 ms | -3.2% | [-4.3%, -2.0%] | - | faster | device 1.4x faster than the host (was 1.3x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.37 ms | 2.32 ms | -2.2% | [-3.0%, -1.4%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.97 ms | 6.00 ms | +0.6% | [+0.5%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.71 ms | 6.70 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.0 ms | 23.1 ms | +0.4% | [+0.1%, +0.8%] | - | ~ | device 2.2x faster than the host; marched/traced 4.30x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.4 ms | 28.5 ms | +0.6% | [-0.7%, +2.0%] | - | ~ | device 1.9x faster than the host; marched/traced 5.59x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 98.9 ms | 99.4 ms | +0.5% | [-2.1%, +3.2%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 159 ms | 159 ms | +0.1% | [-1.4%, +1.6%] | - | ~ | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.7 ms | 13.8 ms | +0.6% | [-0.4%, +1.6%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.3 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 293 ms | 303 ms | +3.2% | [+1.4%, +5.0%] | - | minor | device 0.2x faster than the host (was 0.2x); marched/traced 1.85x (was 1.93x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 337 ms | 345 ms | +2.6% | [+1.5%, +3.8%] | - | ~ | device 0.2x faster than the host; marched/traced 2.88x (was 3.02x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 566 ms | 560 ms | -1.0% | [-1.8%, -0.2%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.01 s | 994 ms | -2.1% | [-3.5%, -0.7%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 186 ms | 193 ms | +3.6% | [+2.7%, +4.4%] | - | minor | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 233 ms | 245 ms | +5.4% | [+4.6%, +6.2%] | - | minor | device 0.2x faster than the host (was 0.2x) |
| `Render/csg/frame/float/hostdevice` | 4.57 ms | 4.57 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.57 ms | 4.58 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.89 ms | 1.88 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.94 ms | 7.95 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.94 ms | 7.95 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.4 ms | 23.2 ms | -0.6% | [-0.8%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 218 µs | +0.5% | [+0.3%, +0.7%] | - | ~ | device 11.3x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.7 µs | -0.2% | [-0.3%, -0.2%] | - | ~ | device 23.8x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 361 µs | 362 µs | +0.3% | [+0.2%, +0.5%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.02 ms | 2.02 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.5 µs | 72.8 µs | +0.4% | [+0.3%, +0.4%] | - | ~ | device 7.2x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 47.8 µs | +0.2% | [-0.7%, +1.1%] | - | ~ | device 6.0x faster than the host (was 4.8x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 653 µs | +0.0% | [-0.1%, +0.2%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.1% | [-0.0%, +0.1%] | - | ~ | device 11.9x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 474 µs | 474 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.7 µs | 48.1 µs | -1.2% | [-3.0%, +0.7%] | - | ~ | device 27.3x faster than the host (was 26.1x) |
| `Tape/csg/signedDistance/double/hostdevice` | 854 µs | 855 µs | +0.1% | [+0.1%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.72 ms | 6.71 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 191 µs | +0.1% | [-0.0%, +0.1%] | - | ~ | device 2.7x faster than the host (was 2.9x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.2% | [-0.4%, -0.1%] | - | ~ | device 1.9x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |


The probes differ between the sides (each side ran its own): Performance/BVHRefit/main.cpp

<!-- END receipt cuda -->
