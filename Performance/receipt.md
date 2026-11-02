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
base: f7f1579
base-fingerprint: ad70aa3b84036fd709a9c01dad59244c3dc1cf44
head: a420d36
head-fingerprint: 18b1811a9abaa5942d1f6929d5ccbb0757b9df3b
cases: 176
rounds: 2
reps: 3
date: 2026-11-02
-->

## Host comparison: green

**Accepted:** -

`a420d36` against `f7f1579` (origin/main), 2026-11-02.

**VERDICT: no regression (0 major, 0 minor across 176 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 176 cases

- base: f7f1579 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: a420d36 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 331 ms | 330 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 512 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 346 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 564 ms | 562 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.0 ms | 55.0 ms | -0.0% | [-0.8%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.2 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.0 ms | 64.6 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.5 ms | 54.5 ms | -0.0% | [-0.8%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.2 ms | 60.4 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 73.8 ms | 72.1 ms | -2.3% | [-4.8%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.1 ms | 74.2 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.7 ms | 77.5 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.6 ms | 67.1 ms | +0.8% | [+0.7%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 100 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.4 ms | 83.3 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 110 ms | +0.6% | [-0.8%, +2.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 21.3 ms | 19.7 ms | -7.5% | [-13.8%, -0.2%] | +0.0% | faster |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 46.0 ms | 44.8 ms | -2.5% | [-5.3%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.2 ms | 73.3 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.1 ms | 20.1 ms | +0.3% | [-0.5%, +1.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 46.0 ms | 46.0 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.8 ms | 82.4 ms | +0.7% | [-2.5%, +4.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.45 ms | 4.50 ms | +1.0% | [+0.6%, +1.4%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.2 ms | -0.5% | [-1.5%, +0.6%] | - | ~ | tape/fused 2.04x |
| `Fused/chain16/signedDistance/float/host` | 969 µs | 959 µs | -1.0% | [-1.7%, -0.3%] | - | ~ | tape/fused 16.91x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.0 ms | 46.2 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.4 ms | 46.4 ms | -0.1% | [-0.8%, +0.7%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.56 ms | 4.55 ms | -0.2% | [-2.0%, +1.7%] | - | ~ | tape/fused 2.87x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.8 ms | +0.4% | [-0.1%, +0.8%] | - | ~ | tape/fused 2.09x |
| `Fused/chain16/signedDistance/double/host` | 3.88 ms | 3.89 ms | +0.2% | [-0.3%, +0.8%] | - | ~ | tape/fused 4.18x |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.0 ms | 53.4 ms | +0.8% | [+0.3%, +1.4%] | +0.0% | ~ | tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.4 ms | 52.7 ms | -1.2% | [-1.9%, -0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.3 ms | 82.1 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 228 ms | 228 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 112 ms | 113 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 259 ms | -0.4% | [-0.8%, +0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.0 ms | 70.0 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 216 ms | 216 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.8 ms | 96.3 ms | -0.6% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.4 ms | 29.8 ms | -1.8% | [-4.9%, +1.5%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 756 ms | 759 ms | +0.4% | [-0.3%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 117 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.8 ms | 43.5 ms | -0.7% | [-0.9%, -0.4%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 846 ms | 846 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.4 ms | 26.6 ms | +0.7% | [+0.6%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.5 ms | 26.6 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 317 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 238 ms | -0.5% | [-0.6%, -0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.0 ms | 89.9 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ | tape/fused 3.53x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.9 ms | 99.7 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ | tape/fused 2.38x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.6 ms | 29.7 ms | +0.5% | [-1.2%, +2.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.1 ms | 29.8 ms | -0.7% | [-1.4%, -0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 353 ms | 353 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 254 ms | 260 ms | +2.2% | [+0.1%, +4.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 100 ms | 98.2 ms | -2.0% | [-6.6%, +2.9%] | +0.0% | ~ | tape/fused 3.60x |
| `MeshCSG/smooth-fused/frame/double/host` | 109 ms | 107 ms | -1.3% | [-3.5%, +0.9%] | +0.0% | ~ | tape/fused 2.42x (was 2.34x) |
| `PointCloudBVH/uniform/build/float/host` | 67.2 ms | 67.1 ms | -0.1% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 151 ms | 150 ms | -0.7% | [-1.0%, -0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 376 ms | 377 ms | +0.4% | [+0.3%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.7 ms | 78.6 ms | -0.1% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.72 ms | 6.70 ms | -0.4% | [-0.8%, -0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 80.0 ms | 80.0 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 161 ms | 161 ms | +0.0% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 403 ms | 403 ms | -0.2% | [-0.6%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.2 ms | 87.2 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.12 ms | 7.10 ms | -0.2% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.8 ms | 11.6 ms | -2.2% | [-5.4%, +1.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 125 ms | 125 ms | -0.5% | [-1.3%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 537 ms | 536 ms | -0.2% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 152 ms | -0.2% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 906 ms | 905 ms | -0.1% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 14.9 ms | +0.1% | [-1.0%, +1.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 127 ms | 126 ms | -0.5% | [-0.9%, -0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 555 ms | 556 ms | +0.2% | [+0.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 157 ms | 155 ms | -1.6% | [-2.4%, -0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 947 ms | 942 ms | -0.5% | [-1.2%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.0 ms | 66.9 ms | -0.2% | [-1.3%, +1.0%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 112 ms | 113 ms | +0.4% | [-1.0%, +1.9%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 272 ms | 272 ms | +0.0% | [-0.8%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 113 ms | 113 ms | +0.0% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 108 ms | 108 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 79.3 ms | 79.6 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 121 ms | 121 ms | -0.3% | [-1.1%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 298 ms | 298 ms | +0.1% | [-0.0%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 127 ms | 127 ms | -0.2% | [-1.3%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 121 ms | 121 ms | +0.0% | [-1.3%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.78 ms | 8.88 ms | +1.2% | [-1.3%, +3.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 144 ms | 144 ms | +0.2% | [-0.0%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 403 ms | 410 ms | +1.9% | [+0.4%, +3.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 253 ms | 253 ms | -0.1% | [-0.8%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 356 ms | 357 ms | +0.1% | [-0.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.4 ms | 12.4 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 145 ms | 148 ms | +2.6% | [+0.4%, +4.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 425 ms | 425 ms | -0.0% | [-3.1%, +3.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 288 ms | 287 ms | -0.2% | [-1.6%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 372 ms | 371 ms | -0.1% | [-0.2%, +0.1%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.3 ms | 46.5 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.4 ms | 45.4 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.3 ms | 40.4 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.1 ms | 78.3 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.3 ms | 65.6 ms | +0.4% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.96 ms | 5.86 ms | -1.5% | [-4.7%, +1.8%] | - | ~ | tape/fused 1.35x (was 1.30x) |
| `Ray/csg-tape/firstHit/float/host` | 7.73 ms | 7.94 ms | +2.7% | [+1.8%, +3.7%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 51.2 ms | 52.0 ms | +1.6% | [+0.9%, +2.3%] | +0.0% | ~ | hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.2 ms | 51.3 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.5 ms | 44.8 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 86.8 ms | 87.1 ms | +0.4% | [-0.3%, +1.1%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.3 ms | 72.7 ms | -0.8% | [-1.4%, -0.2%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.99 ms | 5.95 ms | -0.6% | [-1.3%, +0.1%] | - | ~ | tape/fused 1.40x |
| `Ray/csg-tape/firstHit/double/host` | 8.32 ms | 8.30 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.5 ms | 18.3 ms | -0.9% | [-1.6%, -0.2%] | - | ~ | marched/traced 6.55x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 120 ms | +0.8% | [+0.7%, +1.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 21.8 ms | -0.8% | [-1.7%, +0.0%] | +0.0% | ~ | marched/traced 7.09x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 154 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.2 ms | 35.2 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.7 ms | 19.3 ms | -2.3% | [-6.9%, +2.5%] | - | ~ | marched/traced 6.53x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 126 ms | +0.1% | [-0.5%, +0.7%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.4 ms | +0.1% | [-0.8%, +0.9%] | +0.0% | ~ | marched/traced 6.84x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 160 ms | 160 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.7 ms | 37.7 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 0.99x |
| `RayCrossing/csg-tape/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 0.92x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 1.22x |
| `RayCrossing/mesh/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 0.89x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.0 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ | marched/traced 4.71x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.9 ms | +0.5% | [-0.4%, +1.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 7.99 ms | 7.97 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.91 ms | 1.92 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.89 ms | 6.81 ms | -1.2% | [-2.3%, -0.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.8 ms | +0.1% | [-1.7%, +1.9%] | +0.0% | ~ | marched/traced 5.18x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.0 ms | 60.9 ms | -0.1% | [-1.7%, +1.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.67 ms | 8.53 ms | -1.7% | [-2.6%, -0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.94 ms | 1.94 ms | +0.0% | [-1.8%, +1.9%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.14 ms | 7.16 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 239 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 327 ms | -0.3% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 238 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 252 ms | 253 ms | +0.7% | [+0.5%, +0.9%] | +0.0% | ~ | marched/traced 1.45x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 370 ms | 367 ms | -0.8% | [-1.6%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 249 ms | 253 ms | +1.2% | [+0.8%, +1.6%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.6 ms | 23.7 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.9 ms | 23.7 ms | -0.7% | [-1.8%, +0.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 18.5 ms | -0.7% | [-1.9%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 29.6 ms | 29.6 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 29.5 ms | 29.7 ms | +0.6% | [-0.9%, +2.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.8 ms | 20.9 ms | +0.5% | [+0.0%, +1.0%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.8 ms | 13.0 ms | +1.3% | [+0.3%, +2.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.53 ms | 4.52 ms | -0.3% | [-1.7%, +1.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.8 ms | 24.9 ms | +0.6% | [-0.0%, +1.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.4 ms | 40.1 ms | -0.8% | [-1.1%, -0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.76 ms | 1.76 ms | +0.2% | [-1.9%, +2.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 139 µs | 138 µs | -0.9% | [-2.0%, +0.2%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.2 ms | 29.9 ms | -1.0% | [-2.1%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.64 ms | 5.65 ms | +0.1% | [-0.5%, +0.7%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.2 ms | 16.2 ms | -0.1% | [-1.0%, +0.7%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.61 ms | 4.59 ms | -0.6% | [-1.9%, +0.8%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.0 ms | 13.1 ms | +0.4% | [-0.6%, +1.5%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.59 ms | 4.57 ms | -0.5% | [-1.6%, +0.6%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.5 ms | 26.6 ms | +0.4% | [-0.3%, +1.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.0 ms | 42.0 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.75 ms | 1.78 ms | +1.3% | [+0.3%, +2.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 284 µs | 287 µs | +1.1% | [+0.6%, +1.6%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.7 ms | 29.6 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.71 ms | 7.76 ms | +0.6% | [+0.4%, +0.7%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.3 ms | 16.2 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.73 ms | 7.75 ms | +0.3% | [-0.3%, +0.9%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/RayCrossing/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: f7f1579
base-fingerprint: ad70aa3b84036fd709a9c01dad59244c3dc1cf44
head: a420d36
head-fingerprint: 18b1811a9abaa5942d1f6929d5ccbb0757b9df3b
cases: 260
rounds: 2
reps: 3
date: 2026-11-02
-->

## Device comparison (CUDA): yellow

**Accepted:** Accept as noise.

`a420d36` against `f7f1579` (origin/main), 2026-11-02.

**VERDICT: 5 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 5 minor, 1 faster, 0 differ, 0 lost, 260 cases

- base: f7f1579 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: a420d36 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.60 ms | 1.62 ms | +0.7% | [+0.5%, +0.9%] | - | ~ | device 22.5x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.44 ms | 3.41 ms | -1.0% | [-1.2%, -0.7%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.06 ms | 1.04 ms | -1.2% | [-3.7%, +1.4%] | - | ~ | device 19.7x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.30 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 16.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.10 ms | 6.16 ms | +1.0% | [-0.1%, +2.1%] | - | ~ | device 7.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.8 ms | -0.1% | [-1.1%, +0.9%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.2 ms | 33.2 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.2 ms | 57.1 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.5 ms | 20.8 ms | +1.1% | [-1.0%, +3.3%] | - | ~ | device 1.0x faster than the host (was 1.0x) |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.0 ms | 47.7 ms | -0.7% | [-1.4%, -0.1%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 60.7 ms | 61.2 ms | +0.9% | [+0.1%, +1.8%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 108 ms | -2.1% | [-3.6%, -0.6%] | - | ~ | device 0.6x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 386 µs | 388 µs | +0.6% | [+0.2%, +1.0%] | - | ~ | device 15.2x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 849 µs | 846 µs | -0.4% | [-0.8%, -0.0%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.52 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.67 ms | 4.67 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 584 µs | 586 µs | +0.3% | [-0.0%, +0.7%] | - | ~ | device 12.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 8.2x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.75 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.00 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 908 µs | 903 µs | -0.5% | [-0.6%, -0.4%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.24 ms | 2.24 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.48 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 319 µs | 320 µs | +0.3% | [+0.2%, +0.4%] | - | ~ | device 11.4x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 5.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 59.0 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 3.3x faster than the host (was 3.8x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.54 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 521 ms | 518 ms | -0.6% | [-1.5%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.2% | [-0.0%, +0.4%] | - | ~ | device 23.5x faster than the host; tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.8 µs | 49.9 µs | +0.3% | [-0.4%, +0.9%] | - | ~ | device 44.1x faster than the host; tape/fused 7.21x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.7 µs | 47.9 µs | +0.3% | [-0.1%, +0.7%] | - | ~ | device 9.3x faster than the host; tape/fused 9.89x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.5 ms | 17.4 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 6.7x faster than the host (was 8.3x); tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.3 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 809 µs | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.5x faster than the host (was 1.4x); tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.6x faster than the host (was 1.5x); tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 283 ms | 282 ms | -0.3% | [-1.0%, +0.5%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 286 ms | 285 ms | -0.2% | [-1.2%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.17 ms | 8.18 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | device 12.6x faster than the host (was 10.8x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.33 ms | 8.32 ms | -0.1% | [-1.0%, +0.7%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.22 ms | 8.19 ms | -0.4% | [-1.7%, +0.9%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.17 ms | 8.25 ms | +1.0% | [+0.2%, +1.8%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 706 µs | 710 µs | +0.5% | [-0.0%, +1.1%] | +0.0% | ~ | device 4.7x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.74 ms | 1.74 ms | +0.0% | [+0.0%, +0.0%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.41x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.04 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.68x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 844 µs | 849 µs | +0.5% | [+0.4%, +0.7%] | +0.0% | ~ | device 3.9x faster than the host; default/this 0.84x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 830 µs | 828 µs | -0.1% | [-0.7%, +0.4%] | +0.0% | ~ | device 11.5x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 953 µs | 946 µs | -0.7% | [-1.6%, +0.2%] | +0.0% | ~ | device 10.1x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 831 µs | 828 µs | -0.5% | [-0.8%, -0.1%] | +0.0% | ~ | device 11.6x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 805 µs | 805 µs | +0.0% | [-0.6%, +0.7%] | +0.0% | ~ | device 11.6x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 162 ms | 158 ms | -2.3% | [-5.3%, +0.9%] | +0.0% | ~ | device 0.6x faster than the host (was 0.5x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 161 ms | 159 ms | -1.0% | [-1.9%, -0.1%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 163 ms | 163 ms | -0.0% | [-1.0%, +1.0%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.97x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 161 ms | 157 ms | -2.5% | [-4.5%, -0.5%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.61 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.76 ms | 1.76 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.91x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.93 ms | 1.93 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.83x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.68 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.28 ms | 7.28 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.41 ms | 7.35 ms | -0.8% | [-1.0%, -0.7%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.31 ms | 7.31 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.35 ms | 7.33 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.51 ms | 8.43 ms | -0.9% | [-1.8%, -0.1%] | - | ~ | device 11.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.7 ms | +0.6% | [+0.4%, +0.8%] | - | ~ | device 9.5x faster than the host (was 7.1x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 166 ms | 167 ms | +0.6% | [-1.7%, +2.9%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 284 ms | 287 ms | +1.2% | [+0.2%, +2.2%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.7 ms | 24.6 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 60.4 ms | 60.7 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 2.4x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 159 ms | 155 ms | -2.4% | [-3.9%, -0.8%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 241 ms | 247 ms | +2.4% | [+1.3%, +3.5%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.95 ms | 2.90 ms | -1.9% | [-2.4%, -1.5%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.12 ms | 5.12 ms | +0.1% | [-0.9%, +1.1%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.6 ms | 14.4 ms | -0.8% | [-1.0%, -0.6%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 122 ms | 121 ms | -0.7% | [-1.8%, +0.4%] | - | ~ | device 1.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 174 ms | 173 ms | -0.4% | [-2.8%, +2.1%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 382 ms | 378 ms | -0.9% | [-2.8%, +1.0%] | - | ~ | device 2.3x faster than the host (was 2.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.5 ms | 24.5 ms | +0.3% | [-0.1%, +0.8%] | - | ~ | device 2.0x faster than the host; tape/fused 4.95x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.6 ms | 26.7 ms | +0.4% | [-0.6%, +1.3%] | - | ~ | device 2.1x faster than the host; tape/fused 6.50x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.4 ms | 71.4 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 4.6x faster than the host (was 4.9x); tape/fused 5.30x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.5 ms | 23.1 ms | +2.7% | [-1.9%, +7.4%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 43.9 ms | 45.1 ms | +2.7% | [+0.6%, +4.8%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 93.7 ms | 92.1 ms | -1.7% | [-2.9%, -0.6%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 559 ms | 559 ms | +0.0% | [-5.5%, +5.7%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 867 ms | 868 ms | +0.1% | [-1.7%, +2.0%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.16 s | 3.17 s | +0.3% | [-0.9%, +1.5%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 192 ms | 196 ms | +1.7% | [+0.2%, +3.1%] | - | ~ | device 0.3x faster than the host; tape/fused 2.86x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 300 ms | 300 ms | -0.2% | [-3.8%, +3.8%] | - | ~ | device 0.2x faster than the host; tape/fused 2.90x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 579 ms | 584 ms | +0.9% | [-2.9%, +4.8%] | - | ~ | device 0.5x faster than the host; tape/fused 5.42x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 765 µs | 768 µs | +0.4% | [+0.0%, +0.7%] | - | ~ | device 10.7x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.22 ms | 2.22 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 5.8x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.17 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 8.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.46 ms | 6.46 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 4.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 4.13 ms | 4.12 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 4.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.10 ms | 7.09 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.80 ms | 7.83 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.5 ms | 20.5 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.2% | [-0.7%, +0.4%] | - | ~ | device 1.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.17 ms | 2.18 ms | +0.8% | [+0.5%, +1.2%] | - | ~ | device 4.9x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.95 ms | 5.95 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 837 µs | 835 µs | -0.2% | [-0.4%, +0.1%] | - | ~ | device 10.4x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 9.13 ms | 9.05 ms | -0.9% | [-2.1%, +0.2%] | - | ~ | device 3.9x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 6.45 ms | 6.49 ms | +0.6% | [+0.3%, +0.8%] | - | ~ | device 4.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.13 ms | 8.01 ms | -1.5% | [-2.9%, -0.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 16.0 ms | 15.9 ms | -0.5% | [-1.7%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.01 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 2.2x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 20.4 ms | 20.4 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.2 ms | 14.9 ms | -1.6% | [-2.7%, -0.4%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.50 ms | 1.50 ms | -0.1% | [-1.2%, +0.9%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.74 ms | 4.74 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.40 ms | 1.40 ms | +0.4% | [-0.4%, +1.3%] | - | ~ | device 5.2x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.39 ms | 3.40 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.60 ms | 4.62 ms | +0.4% | [-0.5%, +1.3%] | - | ~ | device 4.1x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.3 ms | 13.4 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 17.2 ms | 17.2 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 7.54 ms | 7.52 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 11.3 ms | 11.3 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 15.9 ms | 15.9 ms | -0.1% | [-1.9%, +1.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 4.87 ms | 4.87 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 3.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 16.1 ms | 16.1 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.61 ms | 1.62 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 7.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.61 ms | 5.61 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 4.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 13.3 ms | 13.3 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.9 ms | 33.7 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.3 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.5 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.9 ms | 21.0 ms | +0.6% | [+0.3%, +0.8%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 39.3 ms | 39.1 ms | -0.4% | [-1.5%, +0.8%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 673 µs | 674 µs | +0.2% | [-0.8%, +1.2%] | - | ~ | device 16.9x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 14.5x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 649 µs | 647 µs | -0.2% | [-0.7%, +0.3%] | - | ~ | device 17.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.23 ms | +0.3% | [-0.7%, +1.2%] | - | ~ | device 15.4x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 615 µs | 607 µs | -1.3% | [-2.1%, -0.4%] | - | ~ | device 16.0x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.11 ms | 1.12 ms | +0.4% | [-0.2%, +0.9%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 12.0x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.8% | [+0.2%, +1.4%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 614 µs | 596 µs | -3.0% | [-5.6%, -0.3%] | - | faster | device 2.5x faster than the host (was 2.4x); tape/fused 1.19x (was 1.15x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 732 µs | 700 µs | -4.4% | [-9.3%, +0.5%] | - | ~ | device 2.7x faster than the host (was 2.5x); tape/fused 1.21x (was 1.15x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 707 µs | 711 µs | +0.7% | [+0.4%, +0.9%] | - | ~ | device 3.6x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 843 µs | 845 µs | +0.3% | [-0.2%, +0.8%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.5 ms | +0.3% | [-0.5%, +1.2%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.1 ms | 16.9 ms | -0.9% | [-1.4%, -0.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.8 ms | 14.9 ms | +0.9% | [-0.5%, +2.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.6 ms | +0.3% | [-1.0%, +1.6%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.02x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.4 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.13x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.2 ms | 15.2 ms | +0.1% | [-0.3%, +0.6%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 22.9 ms | -0.8% | [-1.1%, -0.5%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.47 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 1.1x faster than the host; tape/fused 1.09x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.62 ms | 2.62 ms | +0.0% | [-0.6%, +0.6%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.71 ms | 2.72 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.59 ms | 2.59 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.9x faster than the host; marched/traced 3.48x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.7x faster than the host; marched/traced 4.60x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.03 ms | 9.03 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.16 ms | 2.16 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 3.9x faster than the host; marched/traced 16.62x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.41 ms | 5.42 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.8x faster than the host; marched/traced 13.23x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 35.9 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.5 ms | 71.7 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 1.6x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.22 ms | 5.23 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.79 ms | 8.80 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.25 ms | 6.31 ms | +0.9% | [+0.4%, +1.5%] | - | ~ | device 1.2x faster than the host; marched/traced 25.28x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 0.8x faster than the host; marched/traced 24.32x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 160 ms | +0.2% | [-3.3%, +3.8%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 258 ms | 257 ms | -0.4% | [-1.2%, +0.4%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.85 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.8x faster than the host; marched/traced 23.66x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.3x faster than the host; marched/traced 17.77x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 112 ms | 115 ms | +2.9% | [-0.2%, +6.2%] | - | ~ | device 0.6x faster than the host (was 0.7x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 221 ms | 217 ms | -2.0% | [-2.8%, -1.1%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.4 ms | -0.0% | [-0.8%, +0.8%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.7 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.16x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.32x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 4.85x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 2.97x |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 2.02x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.09x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.26x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.31x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.62 ms | 3.52 ms | -2.6% | [-4.7%, -0.3%] | - | ~ | device 1.2x faster than the host; marched/traced 5.59x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.75 ms | 4.77 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 0.9x faster than the host; marched/traced 8.41x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.7 ms | 19.7 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.2 ms | 40.1 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.23 ms | 1.25 ms | +1.8% | [-0.3%, +3.9%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.83 ms | -1.1% | [-1.5%, -0.7%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 94.3 µs | 93.3 µs | -1.1% | [-4.3%, +2.2%] | - | ~ | device 15.5x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 148 µs | 153 µs | +3.1% | [+1.5%, +4.8%] | - | minor | device 10.7x faster than the host (was 11.2x) |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 800 µs | 799 µs | -0.1% | [-0.4%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 934 µs | 934 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.9 ms | 35.6 ms | +1.9% | [-2.2%, +6.2%] | - | ~ | device 0.1x faster than the host; marched/traced 3.48x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.4 ms | 36.5 ms | +0.3% | [-0.7%, +1.2%] | - | ~ | device 0.1x faster than the host; marched/traced 6.43x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 125 ms | 124 ms | -0.9% | [-2.5%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 234 ms | 234 ms | +0.1% | [-1.9%, +2.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 15.7 ms | 16.9 ms | +7.7% | [+2.2%, +13.2%] | - | minor | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.7 ms | 22.0 ms | +1.3% | [+0.6%, +2.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.04 ms | 1.07 ms | +2.8% | [-0.7%, +6.4%] | - | ~ | device 1.4x faster than the host (was 1.5x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.36 ms | 2.36 ms | +0.1% | [-2.1%, +2.3%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.96 ms | 5.95 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.69 ms | 6.70 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.1 ms | 23.0 ms | -0.3% | [-1.2%, +0.5%] | - | ~ | device 2.2x faster than the host; marched/traced 4.33x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.5 ms | 28.5 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 1.9x faster than the host; marched/traced 5.62x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 99.8 ms | 99.8 ms | -0.0% | [-1.0%, +1.0%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 160 ms | 160 ms | +0.2% | [-1.3%, +1.8%] | - | ~ | device 1.6x faster than the host (was 1.5x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.7 ms | 13.7 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.4 ms | -0.3% | [-1.1%, +0.4%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 291 ms | 306 ms | +5.3% | [+1.4%, +9.3%] | - | minor | device 0.2x faster than the host (was 0.2x); marched/traced 1.85x (was 1.94x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 350 ms | 351 ms | +0.5% | [-0.4%, +1.3%] | - | ~ | device 0.2x faster than the host; marched/traced 2.86x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 564 ms | 565 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 996 ms | 1.00 s | +0.9% | [-0.9%, +2.7%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 183 ms | 191 ms | +4.1% | [+4.0%, +4.2%] | - | minor | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 230 ms | 237 ms | +3.1% | [+0.2%, +6.0%] | - | minor | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.63 ms | 4.63 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.63 ms | 4.63 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.9x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.89 ms | 1.89 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 8.13 ms | 8.13 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 8.11 ms | 8.11 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.3 ms | 23.4 ms | +0.3% | [-0.9%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 218 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 11.4x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 23.4x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 360 µs | -0.1% | [-0.2%, -0.1%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.00 ms | 1.99 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 7.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.4 µs | 72.6 µs | +0.3% | [+0.1%, +0.4%] | - | ~ | device 7.0x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 48.3 µs | +1.3% | [-0.7%, +3.3%] | - | ~ | device 4.8x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 653 µs | 653 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 12.1x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 472 µs | 474 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 48.0 µs | +0.7% | [+0.2%, +1.2%] | - | ~ | device 27.1x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 854 µs | 855 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.72 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 190 µs | +0.0% | [+0.0%, +0.1%] | - | ~ | device 2.7x faster than the host (was 2.9x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.0x faster than the host (was 1.8x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.77 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |


The probes differ between the sides (each side ran its own): Performance/RayCrossing/main.cpp

<!-- END receipt cuda -->
