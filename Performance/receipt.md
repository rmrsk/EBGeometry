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
verdict: yellow
base: f7f1579
base-fingerprint: ad70aa3b84036fd709a9c01dad59244c3dc1cf44
head: 5c0e269
head-fingerprint: 901e9c5ca73f50de43c617ceb0712e1579634b6f
cases: 176
rounds: 2
reps: 3
date: 2026-11-02
-->

## Host comparison: yellow

**Accepted:** -

`5c0e269` against `f7f1579` (origin/main), 2026-11-02.

**VERDICT: 1 minor regression; rerun it with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 1 minor, 1 faster, 0 differ, 0 lost, 176 cases

- base: f7f1579 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 5c0e269 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 330 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 513 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 347 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 560 ms | 562 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.0 ms | 55.0 ms | +0.1% | [-0.6%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.7 ms | 51.2 ms | -0.9% | [-1.5%, -0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.1 ms | 65.3 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 55.0 ms | 54.6 ms | -0.8% | [-2.3%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.5 ms | 60.4 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.2 ms | 72.5 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.2 ms | 74.5 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.3 ms | 77.2 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.9 ms | 67.1 ms | +0.4% | [+0.3%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 99.9 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 84.2 ms | 83.5 ms | -0.8% | [-1.4%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 108 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.5 ms | 19.5 ms | +0.0% | [-1.5%, +1.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 46.2 ms | 44.9 ms | -2.8% | [-4.3%, -1.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.2 ms | 72.9 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.1 ms | 20.1 ms | -0.2% | [-2.2%, +1.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.9 ms | 45.8 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.2 ms | 83.2 ms | +1.1% | [+0.1%, +2.2%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.43 ms | 4.44 ms | +0.4% | [+0.0%, +0.7%] | - | ~ | tape/fused 2.91x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.1% | [-0.4%, +0.7%] | - | ~ | tape/fused 2.05x |
| `Fused/chain16/signedDistance/float/host` | 960 µs | 962 µs | +0.1% | [-1.3%, +1.6%] | - | ~ | tape/fused 16.79x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.6 ms | +1.1% | [+0.3%, +1.8%] | +0.0% | ~ | tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.2 ms | 46.2 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.58 ms | 4.58 ms | -0.0% | [-1.8%, +1.8%] | - | ~ | tape/fused 2.86x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.8 ms | +0.2% | [-0.5%, +0.8%] | - | ~ | tape/fused 2.07x |
| `Fused/chain16/signedDistance/double/host` | 3.87 ms | 3.87 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | tape/fused 4.19x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.9 ms | 52.4 ms | -0.8% | [-1.6%, -0.0%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.3 ms | 52.9 ms | -0.7% | [-2.2%, +0.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.9 ms | 82.7 ms | +1.0% | [+0.1%, +1.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 228 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 259 ms | 261 ms | +0.5% | [-0.4%, +1.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.0 ms | 70.0 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 213 ms | -0.9% | [-1.4%, -0.4%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.7 ms | 96.5 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.5 ms | 29.9 ms | +1.3% | [+1.1%, +1.5%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 757 ms | 754 ms | -0.4% | [-0.4%, -0.3%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.2 ms | 43.5 ms | -1.6% | [-3.7%, +0.4%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 848 ms | 845 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.5 ms | 26.7 ms | +0.6% | [-0.2%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.8 ms | 26.6 ms | -0.5% | [-1.9%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 318 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 238 ms | 238 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.0 ms | 90.2 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | tape/fused 3.53x |
| `MeshCSG/smooth-fused/frame/float/host` | 100.0 ms | 99.6 ms | -0.4% | [-1.4%, +0.6%] | +0.0% | ~ | tape/fused 2.39x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.7 ms | 29.8 ms | +0.2% | [-1.7%, +2.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.1 ms | 30.2 ms | +0.5% | [-0.3%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 354 ms | -0.9% | [-2.4%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 254 ms | 264 ms | +4.1% | [+3.8%, +4.5%] | +0.0% | minor |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 98.5 ms | 98.2 ms | -0.3% | [-3.0%, +2.4%] | +0.0% | ~ | tape/fused 3.60x |
| `MeshCSG/smooth-fused/frame/double/host` | 110 ms | 108 ms | -1.8% | [-3.7%, +0.1%] | +0.0% | ~ | tape/fused 2.44x (was 2.30x) |
| `PointCloudBVH/uniform/build/float/host` | 67.1 ms | 67.3 ms | +0.4% | [-0.3%, +1.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 152 ms | 150 ms | -1.3% | [-2.6%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 375 ms | 376 ms | +0.1% | [+0.1%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.7 ms | 78.5 ms | -0.2% | [-0.7%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.71 ms | 6.73 ms | +0.3% | [-0.2%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 80.4 ms | 79.8 ms | -0.7% | [-1.2%, -0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 161 ms | 161 ms | -0.3% | [-0.6%, -0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 403 ms | 414 ms | +2.7% | [-0.1%, +5.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.1 ms | 88.3 ms | +1.4% | [-0.5%, +3.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.09 ms | 7.17 ms | +1.1% | [+0.3%, +2.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 10.9 ms | 11.6 ms | +6.0% | [-4.4%, +19.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 125 ms | 125 ms | -0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 538 ms | 536 ms | -0.3% | [-0.5%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 152 ms | -0.1% | [-0.3%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 907 ms | 907 ms | +0.1% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.1 ms | 15.1 ms | +0.1% | [-0.8%, +1.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 126 ms | 126 ms | +0.0% | [-0.5%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 557 ms | 556 ms | -0.2% | [-0.3%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 155 ms | 154 ms | -1.0% | [-1.9%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 941 ms | 940 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 66.4 ms | 66.9 ms | +0.7% | [+0.0%, +1.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 112 ms | 112 ms | -0.1% | [-0.6%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 271 ms | 272 ms | +0.4% | [+0.4%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 113 ms | 114 ms | +0.4% | [-0.2%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 108 ms | 108 ms | -0.3% | [-0.5%, -0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 79.8 ms | 79.7 ms | -0.1% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 131 ms | 121 ms | -7.6% | [-14.4%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 308 ms | 299 ms | -3.0% | [-5.9%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 127 ms | 127 ms | +0.3% | [+0.0%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 122 ms | 121 ms | -0.9% | [-1.4%, -0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.76 ms | 8.79 ms | +0.2% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 144 ms | 144 ms | -0.2% | [-0.4%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 397 ms | 397 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 253 ms | 254 ms | +0.3% | [+0.1%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 357 ms | 356 ms | -0.4% | [-0.4%, -0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.4 ms | 12.3 ms | -1.4% | [-2.7%, -0.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 145 ms | 145 ms | -0.3% | [-0.6%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 418 ms | 418 ms | +0.0% | [-0.1%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 283 ms | 285 ms | +0.7% | [+0.3%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 372 ms | 371 ms | -0.5% | [-0.7%, -0.2%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.2 ms | 46.3 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.4 ms | 45.5 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.3 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 77.5 ms | 78.0 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.5 ms | 65.7 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.77 ms | 5.86 ms | +1.6% | [+0.6%, +2.7%] | - | ~ | tape/fused 1.35x |
| `Ray/csg-tape/firstHit/float/host` | 7.68 ms | 7.91 ms | +3.0% | [+1.8%, +4.2%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 51.4 ms | 51.9 ms | +1.0% | [+0.9%, +1.1%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.4 ms | 50.7 ms | -1.4% | [-1.8%, -0.9%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.7 ms | 44.8 ms | +0.2% | [-0.8%, +1.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 86.9 ms | 86.8 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.4 ms | 72.7 ms | -1.0% | [-2.1%, +0.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.96 ms | 5.97 ms | +0.2% | [-0.8%, +1.2%] | - | ~ | tape/fused 1.40x |
| `Ray/csg-tape/firstHit/double/host` | 8.41 ms | 8.34 ms | -0.8% | [-3.4%, +1.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.5 ms | 17.9 ms | -3.3% | [-6.3%, -0.2%] | - | faster | marched/traced 6.69x (was 6.42x) |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 120 ms | +0.8% | [+0.6%, +0.9%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 22.0 ms | -0.4% | [-1.2%, +0.4%] | +0.0% | ~ | marched/traced 7.03x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 154 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.4 ms | 35.2 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 20.3 ms | 19.9 ms | -1.9% | [-2.8%, -1.0%] | - | ~ | marched/traced 6.58x (was 6.21x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 131 ms | +3.8% | [-0.8%, +8.5%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 23.4 ms | -0.4% | [-0.8%, +0.0%] | +0.0% | ~ | marched/traced 6.88x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 159 ms | 161 ms | +0.7% | [+0.6%, +0.9%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.7 ms | 37.6 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 0.91x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 1.22x |
| `RayCrossing/mesh/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 0.88x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 1.22x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.3 ms | +2.8% | [+1.7%, +3.8%] | +0.0% | ~ | marched/traced 4.56x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.6 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.02 ms | 8.01 ms | -0.1% | [-1.4%, +1.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.94 ms | 1.91 ms | -1.5% | [-3.3%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.87 ms | 6.89 ms | +0.3% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.8 ms | +0.5% | [-2.2%, +3.2%] | +0.0% | ~ | marched/traced 5.19x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.3 ms | 61.2 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.66 ms | 8.65 ms | -0.2% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.92 ms | 1.92 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.17 ms | 7.15 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 238 ms | -0.3% | [-1.1%, +0.5%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 327 ms | 328 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 236 ms | -0.1% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 251 ms | 255 ms | +1.4% | [+1.2%, +1.6%] | +0.0% | ~ | marched/traced 1.45x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 371 ms | 370 ms | -0.2% | [-1.5%, +1.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 249 ms | 251 ms | +1.1% | [+1.0%, +1.2%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.6 ms | 23.6 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.8 ms | 24.1 ms | +1.3% | [-0.3%, +2.9%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.5 ms | 18.4 ms | -0.3% | [-1.3%, +0.7%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 29.6 ms | 29.5 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 29.5 ms | 29.5 ms | -0.0% | [-0.8%, +0.8%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.7 ms | 20.9 ms | +1.1% | [-0.0%, +2.2%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.58 ms | 4.51 ms | -1.5% | [-2.5%, -0.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.2 ms | 25.1 ms | -0.1% | [-2.0%, +1.8%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.1 ms | 40.2 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.74 ms | 1.74 ms | +0.1% | [-1.0%, +1.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 140 µs | 140 µs | -0.4% | [-3.7%, +3.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.9 ms | +0.1% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.63 ms | -0.4% | [-0.8%, -0.0%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.2 ms | -0.9% | [-1.3%, -0.4%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.63 ms | 4.71 ms | +1.6% | [-1.0%, +4.3%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.1 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.56 ms | 4.57 ms | +0.1% | [-0.6%, +0.9%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.2 ms | 26.4 ms | +0.5% | [-0.1%, +1.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 41.9 ms | 41.9 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.77 ms | 1.73 ms | -2.0% | [-4.2%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 284 µs | 279 µs | -1.8% | [-9.0%, +6.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.7 ms | 29.8 ms | +0.3% | [+0.0%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.72 ms | 7.76 ms | +0.6% | [+0.0%, +1.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.2 ms | 16.2 ms | -0.1% | [-1.0%, +0.7%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.71 ms | 7.76 ms | +0.8% | [+0.2%, +1.3%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/RayCrossing/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: f7f1579
base-fingerprint: ad70aa3b84036fd709a9c01dad59244c3dc1cf44
head: 5c0e269
head-fingerprint: 901e9c5ca73f50de43c617ceb0712e1579634b6f
cases: 260
rounds: 2
reps: 3
date: 2026-11-02
-->

## Device comparison (CUDA): yellow

**Accepted:** -

`5c0e269` against `f7f1579` (origin/main), 2026-11-02.

**VERDICT: 2 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 2 minor, 1 faster, 0 differ, 0 lost, 260 cases

- base: f7f1579 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 5c0e269 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.60 ms | 1.61 ms | +0.6% | [-0.0%, +1.3%] | - | ~ | device 22.5x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.41 ms | 3.39 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.08 ms | 1.07 ms | -1.2% | [-3.7%, +1.3%] | - | ~ | device 19.2x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.31 ms | -0.1% | [-0.9%, +0.7%] | - | ~ | device 16.5x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.20 ms | 6.21 ms | +0.2% | [-2.0%, +2.4%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.9 ms | -0.5% | [-3.3%, +2.4%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.2 ms | -0.2% | [-1.5%, +1.0%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 56.9 ms | 57.0 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.5 ms | 20.4 ms | -0.5% | [-1.8%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.0 ms | 48.0 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.6 ms | 61.3 ms | -0.5% | [-1.4%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 110 ms | +0.6% | [-2.2%, +3.4%] | - | ~ | device 0.6x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 388 µs | 388 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 15.1x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 845 µs | 846 µs | +0.1% | [-0.5%, +0.6%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.51 ms | 4.53 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.68 ms | 4.68 ms | +0.0% | [-0.6%, +0.6%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 584 µs | 586 µs | +0.3% | [-0.2%, +0.9%] | - | ~ | device 12.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 8.2x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.76 ms | 6.77 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.03 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 905 µs | 908 µs | +0.4% | [-0.3%, +1.0%] | - | ~ | device 14.2x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.24 ms | 2.23 ms | -0.3% | [-0.7%, +0.2%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.45 ms | 9.51 ms | +0.6% | [+0.0%, +1.2%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.5% | [-0.0%, +1.0%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 319 µs | 318 µs | -0.3% | [-0.5%, -0.1%] | - | ~ | device 11.4x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 5.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.8 ms | 59.0 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 3.8x faster than the host (was 3.3x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.54 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 516 ms | 518 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 48.1 µs | 47.8 µs | -0.6% | [-0.7%, -0.5%] | - | ~ | device 23.7x faster than the host; tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.0 µs | 49.7 µs | -0.7% | [-1.2%, -0.1%] | - | ~ | device 43.8x faster than the host (was 42.4x); tape/fused 7.26x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.2 µs | 48.0 µs | -0.4% | [-1.2%, +0.5%] | - | ~ | device 9.3x faster than the host; tape/fused 9.86x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.4 ms | 17.5 ms | +0.5% | [+0.2%, +0.9%] | - | ~ | device 9.1x faster than the host (was 6.8x); tape/fused 1.79x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.4 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.05x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 284 ms | 284 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 0.4x faster than the host (was 0.4x); tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 286 ms | 285 ms | -0.2% | [-1.2%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.32 ms | 8.22 ms | -1.2% | [-1.7%, -0.8%] | +0.0% | ~ | device 10.8x faster than the host (was 14.6x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.17 ms | 8.23 ms | +0.7% | [-0.1%, +1.5%] | +0.0% | ~ | device 10.8x faster than the host (was 11.7x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.13 ms | 8.20 ms | +0.8% | [+0.6%, +1.1%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.17 ms | 8.14 ms | -0.4% | [-2.6%, +1.9%] | +0.0% | ~ | device 11.0x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 708 µs | 712 µs | +0.6% | [+0.4%, +0.8%] | +0.0% | ~ | device 4.6x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.75 ms | 1.74 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.41x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.04 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.69x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 845 µs | 843 µs | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ | device 3.9x faster than the host; default/this 0.84x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 832 µs | 829 µs | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ | device 11.5x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 948 µs | 950 µs | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ | device 10.2x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 826 µs | 826 µs | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 11.5x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 808 µs | 807 µs | -0.2% | [-1.2%, +0.8%] | +0.0% | ~ | device 11.6x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 157 ms | 158 ms | +0.1% | [-2.0%, +2.3%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 158 ms | 164 ms | +3.7% | [+3.0%, +4.3%] | +0.0% | minor | device 0.5x faster than the host (was 0.6x); default/this 0.96x (was 1.00x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 164 ms | 164 ms | -0.1% | [-2.9%, +2.7%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.96x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 161 ms | 164 ms | +1.9% | [+0.4%, +3.5%] | +0.0% | ~ | device 0.5x faster than the host (was 0.6x); default/this 0.96x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.59 ms | -0.6% | [-0.8%, -0.4%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.76 ms | 1.76 ms | +0.0% | [-0.8%, +0.8%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.90x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.94 ms | 1.95 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.82x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.68 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.27 ms | 7.31 ms | +0.7% | [+0.2%, +1.1%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.35 ms | 7.40 ms | +0.7% | [-0.0%, +1.5%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.29 ms | 7.32 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.37 ms | 7.32 ms | -0.8% | [-1.2%, -0.3%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.50 ms | 8.48 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 11.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.8 ms | 16.9 ms | +0.9% | [-0.0%, +1.8%] | - | ~ | device 6.9x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 165 ms | 167 ms | +1.7% | [+0.4%, +3.1%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 288 ms | 286 ms | -0.4% | [-1.2%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.6 ms | 24.6 ms | +0.1% | [-1.1%, +1.4%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 60.4 ms | 60.5 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 2.5x faster than the host (was 2.4x) |
| `MeshSDF/query-morton/double/hostdevice` | 156 ms | 156 ms | -0.1% | [-1.1%, +0.9%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 246 ms | 245 ms | -0.5% | [-2.8%, +1.8%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.90 ms | 2.92 ms | +0.7% | [-0.1%, +1.5%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.14 ms | 5.12 ms | -0.5% | [-0.7%, -0.3%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.5 ms | 14.6 ms | +0.7% | [+0.4%, +1.0%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 119 ms | 119 ms | +0.1% | [-1.0%, +1.2%] | - | ~ | device 1.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 175 ms | 175 ms | +0.1% | [-1.0%, +1.2%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 377 ms | 376 ms | -0.1% | [-2.1%, +1.9%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.1 ms | 25.1 ms | -0.1% | [-1.1%, +0.8%] | - | ~ | device 1.9x faster than the host (was 2.1x); tape/fused 4.75x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.8 ms | 26.8 ms | -0.1% | [-0.9%, +0.8%] | - | ~ | device 2.1x faster than the host; tape/fused 6.54x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.6 ms | 71.8 ms | +0.3% | [-1.1%, +1.6%] | - | ~ | device 4.1x faster than the host; tape/fused 5.25x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 23.2 ms | 22.6 ms | -2.7% | [-6.7%, +1.4%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.6 ms | 44.8 ms | +0.5% | [-1.6%, +2.7%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 94.2 ms | 95.4 ms | +1.3% | [-0.0%, +2.7%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 551 ms | 538 ms | -2.2% | [-3.6%, -0.9%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 847 ms | 868 ms | +2.6% | [+0.8%, +4.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.24 s | 3.18 s | -1.9% | [-4.4%, +0.6%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 199 ms | 201 ms | +1.0% | [-1.7%, +3.9%] | - | ~ | device 0.2x faster than the host; tape/fused 2.67x (was 2.76x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 306 ms | 308 ms | +0.8% | [-0.2%, +1.9%] | - | ~ | device 0.2x faster than the host; tape/fused 2.82x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 599 ms | 599 ms | +0.0% | [-4.5%, +4.7%] | - | ~ | device 0.5x faster than the host; tape/fused 5.31x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 767 µs | 772 µs | +0.7% | [+0.3%, +1.1%] | - | ~ | device 10.6x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.22 ms | 2.22 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.17 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 8.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.46 ms | 6.46 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 4.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 4.13 ms | 4.14 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 4.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.11 ms | 7.10 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.84 ms | 7.80 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.5 ms | 20.5 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.16 ms | 2.18 ms | +0.8% | [-0.0%, +1.6%] | - | ~ | device 4.9x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.96 ms | 5.96 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 835 µs | 837 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 10.4x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 9.11 ms | 9.08 ms | -0.3% | [-1.6%, +0.9%] | - | ~ | device 3.9x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 6.45 ms | 6.49 ms | +0.6% | [-0.3%, +1.5%] | - | ~ | device 4.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.12 ms | 8.22 ms | +1.3% | [+0.1%, +2.5%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.8 ms | 16.0 ms | +1.4% | [+1.0%, +1.7%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.02 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.2x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 20.5 ms | 20.4 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.3 ms | 15.0 ms | -1.6% | [-2.8%, -0.4%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.50 ms | 1.50 ms | -0.5% | [-1.6%, +0.7%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.74 ms | 4.73 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.39 ms | 1.40 ms | +0.9% | [+0.1%, +1.7%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.40 ms | 3.40 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.63 ms | 4.63 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 4.1x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.5 ms | 13.4 ms | -0.2% | [-1.1%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 17.2 ms | 17.1 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 7.52 ms | 7.53 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 11.3 ms | 11.3 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 15.8 ms | 15.8 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 4.87 ms | 4.88 ms | +0.4% | [+0.3%, +0.4%] | - | ~ | device 3.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 16.1 ms | 16.1 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.62 ms | 1.62 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 7.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.62 ms | 5.62 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 5.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 13.4 ms | 13.3 ms | -0.6% | [-0.8%, -0.4%] | - | ~ | device 1.9x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 34.1 ms | 33.7 ms | -1.3% | [-2.0%, -0.5%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.3 ms | 68.5 ms | +0.3% | [-0.5%, +1.0%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.5 ms | +0.7% | [+0.2%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 21.0 ms | 21.0 ms | +0.1% | [-0.8%, +1.1%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.9 ms | 39.2 ms | +0.7% | [-0.5%, +2.0%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 686 µs | 672 µs | -2.1% | [-4.1%, -0.0%] | - | ~ | device 16.8x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 14.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 648 µs | 649 µs | +0.1% | [-0.9%, +1.1%] | - | ~ | device 17.1x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 607 µs | 607 µs | +0.1% | [-0.4%, +0.6%] | - | ~ | device 16.0x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | -0.5% | [-0.7%, -0.3%] | - | ~ | device 14.8x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.18 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.47 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 10.0x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.08 ms | +0.7% | [+0.3%, +1.1%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 597 µs | 631 µs | +5.6% | [+5.2%, +6.1%] | - | minor | device 2.4x faster than the host (was 2.5x); tape/fused 1.13x (was 1.18x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 698 µs | 664 µs | -4.7% | [-9.0%, -0.0%] | - | faster | device 2.8x faster than the host (was 2.6x); tape/fused 1.26x (was 1.21x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 707 µs | 710 µs | +0.4% | [+0.1%, +0.7%] | - | ~ | device 3.6x faster than the host (was 3.7x) |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 841 µs | 835 µs | -0.7% | [-0.8%, -0.5%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.3 ms | -0.6% | [-1.2%, -0.0%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.1% | [-0.6%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.3 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 14.9 ms | +0.2% | [-1.2%, +1.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.2% | [-1.0%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.5 ms | +0.2% | [-0.8%, +1.3%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.1% | [-0.7%, +0.6%] | - | ~ | device 1.0x faster than the host (was 1.0x); triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.4% | [-0.4%, +1.3%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 22.9 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.61 ms | 2.61 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.72 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.59 ms | 2.59 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 2.9x faster than the host; marched/traced 3.48x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 2.7x faster than the host; marched/traced 4.61x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.04 ms | 9.01 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 4.4x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.16 ms | 2.17 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 3.9x faster than the host; marched/traced 16.61x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.43 ms | 5.41 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 2.8x faster than the host; marched/traced 13.21x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 36.0 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.2 ms | 71.5 ms | +0.4% | [+0.3%, +0.6%] | - | ~ | device 1.6x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.23 ms | 5.24 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.80 ms | 8.80 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.24 ms | 6.26 ms | +0.2% | [-1.5%, +1.9%] | - | ~ | device 1.3x faster than the host; marched/traced 25.65x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 0.8x faster than the host; marched/traced 24.07x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 156 ms | 161 ms | +2.6% | [+1.4%, +3.9%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 255 ms | 255 ms | -0.0% | [-2.2%, +2.2%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.86 ms | 4.84 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 1.8x faster than the host; marched/traced 22.96x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.5% | [+0.4%, +0.6%] | - | ~ | device 1.3x faster than the host; marched/traced 17.83x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 114 ms | 111 ms | -2.5% | [-3.3%, -1.7%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 221 ms | 218 ms | -1.3% | [-2.5%, -0.0%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.4 ms | +0.1% | [-1.1%, +1.3%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.16x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.31x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 4.75x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 2.99x |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 2.02x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.15x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.35x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.54 ms | 3.55 ms | +0.2% | [-1.1%, +1.6%] | - | ~ | device 1.2x faster than the host; marched/traced 5.53x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.75 ms | 4.79 ms | +0.9% | [+0.6%, +1.2%] | - | ~ | device 0.9x faster than the host; marched/traced 8.38x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.7 ms | 19.6 ms | -0.5% | [-1.3%, +0.2%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.1 ms | 40.1 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.23 ms | 1.23 ms | -0.1% | [-1.8%, +1.7%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.84 ms | 1.85 ms | +0.2% | [-0.7%, +1.2%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.3 µs | 95.9 µs | +0.6% | [-1.5%, +2.7%] | - | ~ | device 14.7x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 150 µs | 152 µs | +1.8% | [-0.6%, +4.2%] | - | ~ | device 10.8x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 800 µs | 797 µs | -0.4% | [-0.5%, -0.3%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 935 µs | 937 µs | +0.3% | [+0.0%, +0.5%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.7 ms | 34.1 ms | -1.7% | [-4.5%, +1.2%] | - | ~ | device 0.1x faster than the host; marched/traced 3.62x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.7 ms | 36.8 ms | +0.3% | [-0.6%, +1.2%] | - | ~ | device 0.1x faster than the host; marched/traced 6.40x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 125 ms | 124 ms | -1.2% | [-5.0%, +2.8%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 236 ms | 235 ms | -0.1% | [-1.2%, +1.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.6 ms | 17.0 ms | +2.3% | [-2.0%, +6.5%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.7 ms | 21.7 ms | +0.0% | [-0.8%, +1.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.07 ms | 1.08 ms | +0.7% | [-3.3%, +5.0%] | - | ~ | device 1.3x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.38 ms | 2.35 ms | -1.1% | [-2.4%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 5.96 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.69 ms | 6.70 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.0 ms | 23.1 ms | +0.1% | [-0.8%, +1.1%] | - | ~ | device 2.2x faster than the host; marched/traced 4.35x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.6 ms | +0.9% | [+0.4%, +1.3%] | - | ~ | device 1.9x faster than the host; marched/traced 5.55x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 99.3 ms | 100 ms | +1.0% | [-1.4%, +3.4%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 159 ms | 159 ms | -0.6% | [-1.8%, +0.7%] | - | ~ | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.7 ms | 13.9 ms | +0.9% | [+0.2%, +1.5%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.5 ms | 17.3 ms | -1.2% | [-1.7%, -0.8%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 316 ms | 310 ms | -2.1% | [-2.5%, -1.6%] | - | ~ | device 0.2x faster than the host; marched/traced 1.83x (was 1.77x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 355 ms | 363 ms | +2.1% | [+0.9%, +3.4%] | - | ~ | device 0.2x faster than the host; marched/traced 2.74x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 559 ms | 568 ms | +1.5% | [+0.5%, +2.5%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.00 s | 994 ms | -0.9% | [-1.8%, -0.1%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 189 ms | 190 ms | +1.0% | [-3.3%, +5.5%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 236 ms | 237 ms | +0.5% | [-2.8%, +4.0%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.63 ms | 4.63 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 2.9x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.63 ms | 4.63 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.89 ms | 1.89 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 8.15 ms | 8.13 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 8.10 ms | 8.13 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.2 ms | 23.4 ms | +0.5% | [-0.4%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 217 µs | -0.1% | [-0.5%, +0.4%] | - | ~ | device 11.3x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 47.7 µs | -0.3% | [-0.4%, -0.2%] | - | ~ | device 24.4x faster than the host (was 23.4x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 361 µs | +0.0% | [-0.2%, +0.3%] | - | ~ | device 12.6x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.99 ms | 1.99 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 7.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.5 µs | 72.6 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.3% | [-1.4%, +0.8%] | - | ~ | device 4.7x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 652 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.2% | [+0.2%, +0.3%] | - | ~ | device 11.8x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 473 µs | 473 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.2 µs | 47.9 µs | -0.5% | [-1.5%, +0.4%] | - | ~ | device 27.1x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 855 µs | 854 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 3.0x faster than the host (was 3.1x) |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.6x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.71 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 190 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.77 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.0x faster than the host |


The probes differ between the sides (each side ran its own): Performance/RayCrossing/main.cpp

<!-- END receipt cuda -->
