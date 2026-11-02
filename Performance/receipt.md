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
head: 5eec269
head-fingerprint: 139b0db678361b912e9d5bbc988f0a34804c9ec8
cases: 180
rounds: 2
reps: 3
date: 2026-11-02
-->

## Host comparison: yellow

**Accepted:** Yes, accept as noise.

`5eec269` against `f7f1579` (origin/main), 2026-11-02.

**VERDICT: 1 minor regression; rerun it with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 1 minor, 0 faster, 0 differ, 0 lost, 180 cases

- base: f7f1579 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 5eec269 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 331 ms | 331 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 177 ms | 178 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 512 ms | 513 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 347 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 187 ms | 186 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 562 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.2 ms | 55.2 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.1 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.9 ms | 64.3 ms | -1.0% | [-1.3%, -0.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.2 ms | 54.4 ms | +0.5% | [+0.4%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.0 ms | 60.6 ms | -0.6% | [-1.5%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.2 ms | 72.8 ms | +0.8% | [+0.3%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.0 ms | 74.0 ms | -0.0% | [-0.9%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.8 ms | 77.4 ms | -0.5% | [-1.4%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.6 ms | 66.5 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 101 ms | 101 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.6 ms | 83.3 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.7 ms | +1.4% | [+0.3%, +2.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 46.1 ms | 45.3 ms | -1.7% | [-2.2%, -1.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.0 ms | 72.8 ms | -0.3% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.1 ms | 20.0 ms | -0.5% | [-1.0%, -0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.9 ms | 46.5 ms | +1.2% | [-0.9%, +3.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.4 ms | 82.9 ms | +0.6% | [+0.2%, +1.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.45 ms | 4.49 ms | +1.0% | [+0.2%, +1.8%] | - | ~ | tape/fused 2.87x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | tape/fused 2.00x |
| `Fused/chain16/signedDistance/float/host` | 961 µs | 969 µs | +0.8% | [-0.0%, +1.7%] | - | ~ | tape/fused 16.64x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.1 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.2 ms | 46.3 ms | +0.3% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.60 ms | 4.59 ms | -0.2% | [-2.5%, +2.2%] | - | ~ | tape/fused 2.85x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | tape/fused 2.06x |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.87 ms | -0.6% | [-1.5%, +0.4%] | - | ~ | tape/fused 4.22x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.7 ms | 52.5 ms | -0.4% | [-1.3%, +0.4%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.7 ms | 52.4 ms | -0.5% | [-1.4%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.6 ms | 82.2 ms | +0.7% | [+0.5%, +0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 228 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 112 ms | 112 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 261 ms | +0.7% | [-0.9%, +2.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 69.6 ms | 69.9 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 215 ms | -0.1% | [-1.4%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.8 ms | 96.4 ms | -0.4% | [-0.6%, -0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.6 ms | 29.9 ms | +0.9% | [-0.3%, +2.1%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 756 ms | 754 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 117 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.8 ms | 43.1 ms | -1.5% | [-3.1%, +0.2%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 847 ms | 843 ms | -0.4% | [-1.2%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.6 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.5 ms | 26.6 ms | +0.4% | [+0.2%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 318 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 238 ms | 238 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.2 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | tape/fused 3.53x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.9 ms | 99.5 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ | tape/fused 2.39x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.6 ms | 29.5 ms | -0.3% | [-2.1%, +1.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.1 ms | 29.9 ms | -0.6% | [-1.1%, -0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 359 ms | +0.6% | [-0.6%, +1.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 258 ms | 259 ms | +0.1% | [-3.7%, +4.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.1 ms | 96.8 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ | tape/fused 3.71x |
| `MeshCSG/smooth-fused/frame/double/host` | 109 ms | 107 ms | -1.9% | [-3.6%, -0.3%] | +0.0% | ~ | tape/fused 2.42x |
| `PointCloudBVH/uniform/build/float/host` | 67.4 ms | 67.5 ms | +0.1% | [-0.9%, +1.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 150 ms | 151 ms | +0.6% | [+0.5%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 376 ms | 376 ms | -0.1% | [-0.4%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.8 ms | 78.8 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.73 ms | 6.74 ms | +0.2% | [-0.9%, +1.3%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 79.5 ms | 79.4 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 161 ms | 160 ms | -0.4% | [-0.9%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 402 ms | 402 ms | -0.0% | [-0.5%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.0 ms | 87.2 ms | +0.2% | [+0.0%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.18 ms | 7.12 ms | -0.7% | [-1.1%, -0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 10.8 ms | 11.7 ms | +8.3% | [+0.8%, +16.9%] | +0.0% | minor |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 125 ms | 124 ms | -0.1% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 536 ms | 536 ms | -0.2% | [-0.3%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 152 ms | +0.1% | [-0.0%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 905 ms | 907 ms | +0.2% | [+0.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 15.0 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 126 ms | 126 ms | -0.2% | [-0.5%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 555 ms | 555 ms | -0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 154 ms | 155 ms | +0.7% | [-0.3%, +1.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 938 ms | 947 ms | +1.0% | [+0.3%, +1.6%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 66.6 ms | 66.4 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 112 ms | 111 ms | -0.3% | [-0.8%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 271 ms | 272 ms | +0.3% | [+0.2%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 113 ms | 113 ms | -0.5% | [-0.8%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 108 ms | 108 ms | -0.2% | [-0.3%, -0.0%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 79.9 ms | 79.1 ms | -1.0% | [-1.2%, -0.8%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 121 ms | 121 ms | -0.1% | [-0.8%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 299 ms | 299 ms | -0.1% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 127 ms | 126 ms | -0.8% | [-1.3%, -0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 121 ms | 121 ms | -0.3% | [-1.4%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.76 ms | 8.88 ms | +1.4% | [+0.2%, +2.6%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 144 ms | 147 ms | +2.2% | [-0.0%, +4.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 397 ms | 396 ms | -0.2% | [-0.4%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 253 ms | 253 ms | -0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 356 ms | 356 ms | -0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.5 ms | 12.5 ms | -0.3% | [-1.3%, +0.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 149 ms | 145 ms | -2.5% | [-4.7%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 430 ms | 430 ms | +0.0% | [-0.0%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 289 ms | 286 ms | -1.0% | [-1.9%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 372 ms | 370 ms | -0.5% | [-0.8%, -0.3%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.2 ms | 46.3 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.5 ms | 45.5 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.3 ms | 40.5 ms | +0.4% | [+0.4%, +0.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 77.4 ms | 78.7 ms | +1.8% | [+1.3%, +2.3%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 66.0 ms | 66.3 ms | +0.5% | [-0.9%, +1.9%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.82 ms | 5.89 ms | +1.2% | [+1.0%, +1.4%] | - | ~ | tape/fused 1.34x |
| `Ray/csg-tape/firstHit/float/host` | 7.77 ms | 7.86 ms | +1.2% | [-0.5%, +3.1%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 51.3 ms | 51.8 ms | +0.9% | [+0.5%, +1.2%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 50.9 ms | 50.9 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.7 ms | 44.7 ms | -0.1% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 86.9 ms | 88.1 ms | +1.4% | [+0.7%, +2.1%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/double/host` | 73.5 ms | 73.1 ms | -0.5% | [-0.6%, -0.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.98 ms | 5.95 ms | -0.6% | [-1.0%, -0.2%] | - | ~ | tape/fused 1.39x |
| `Ray/csg-tape/firstHit/double/host` | 8.33 ms | 8.27 ms | -0.7% | [-1.1%, -0.3%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.9 ms | 17.6 ms | -1.9% | [-2.9%, -0.9%] | - | ~ | marched/traced 6.82x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 120 ms | +0.8% | [+0.7%, +0.9%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.9 ms | 22.0 ms | +0.4% | [-1.1%, +2.0%] | +0.0% | ~ | marched/traced 7.02x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 154 ms | +0.5% | [+0.3%, +0.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.2 ms | 35.2 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.7 ms | 19.6 ms | -0.8% | [-6.1%, +4.8%] | - | ~ | marched/traced 6.42x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 126 ms | -0.6% | [-1.2%, +0.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.2 ms | -0.9% | [-1.3%, -0.5%] | +0.0% | ~ | marched/traced 6.90x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 160 ms | 160 ms | +0.4% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.6 ms | 37.8 ms | +0.7% | [-0.2%, +1.6%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | - | - | - | - | - | new |  |
| `RayCrossing/mesh/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | - | - | - | - | - | new |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 1.17x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | - | - | - | - | - | new | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | - | - | - | - | - | new |  |
| `RayCrossing/mesh/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | - | - | - | - | - | new |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 1.18x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | - | - | - | - | - | new | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.0 ms | -0.6% | [-0.7%, -0.4%] | +0.0% | ~ | marched/traced 4.72x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.8 ms | +0.2% | [-0.8%, +1.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.21 ms | 8.01 ms | -2.4% | [-5.3%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.91 ms | 1.94 ms | +1.6% | [+0.3%, +2.8%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.87 ms | 6.87 ms | -0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.6 ms | 11.8 ms | +1.2% | [-0.7%, +3.1%] | +0.0% | ~ | marched/traced 5.21x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.3 ms | 61.3 ms | +1.8% | [+0.5%, +3.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.60 ms | 8.56 ms | -0.5% | [-1.5%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.93 ms | 1.92 ms | -0.6% | [-1.2%, +0.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.17 ms | 7.12 ms | -0.7% | [-1.2%, -0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 238 ms | 238 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 327 ms | 327 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 237 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 251 ms | 253 ms | +1.1% | [+0.9%, +1.4%] | +0.0% | ~ | marched/traced 1.45x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 367 ms | 366 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 249 ms | 252 ms | +1.1% | [+0.8%, +1.3%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.8 ms | 23.7 ms | -0.7% | [-0.8%, -0.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.6 ms | 23.7 ms | +0.5% | [+0.2%, +0.9%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.4 ms | 18.4 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 29.6 ms | 29.5 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 29.4 ms | 29.5 ms | +0.3% | [+0.3%, +0.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.8 ms | 20.6 ms | -1.2% | [-2.1%, -0.3%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.54 ms | 4.55 ms | +0.2% | [-1.6%, +2.1%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.9 ms | 24.6 ms | -1.3% | [-2.2%, -0.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.2 ms | 40.2 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.74 ms | 1.74 ms | +0.3% | [-0.6%, +1.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 138 µs | 139 µs | +0.5% | [-0.1%, +1.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 30.0 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.67 ms | 5.63 ms | -0.6% | [-1.2%, +0.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.1 ms | -0.8% | [-2.2%, +0.7%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.61 ms | 4.62 ms | +0.3% | [-0.9%, +1.4%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.0 ms | 13.1 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.53 ms | 4.56 ms | +0.5% | [-0.4%, +1.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.1 ms | 26.4 ms | +1.1% | [+0.9%, +1.3%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.1 ms | 42.0 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.74 ms | 1.74 ms | -0.1% | [-1.8%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 273 µs | 287 µs | +4.9% | [-1.4%, +12.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.7 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.75 ms | 7.77 ms | +0.3% | [-0.7%, +1.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.3 ms | 16.3 ms | +0.3% | [-0.6%, +1.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.75 ms | 7.69 ms | -0.8% | [-1.8%, +0.2%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/RayCrossing/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: f7f1579
base-fingerprint: ad70aa3b84036fd709a9c01dad59244c3dc1cf44
head: 5eec269
head-fingerprint: 139b0db678361b912e9d5bbc988f0a34804c9ec8
cases: 268
rounds: 2
reps: 3
date: 2026-11-02
-->

## Device comparison (CUDA): red

**Accepted:** Yes, accept as noise.

`5eec269` against `f7f1579` (origin/main), 2026-11-02.

**VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 1 major, 3 minor, 2 faster, 0 differ, 0 lost, 268 cases

- base: f7f1579 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 5eec269 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.61 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 22.6x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.40 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.07 ms | 1.06 ms | -1.0% | [-3.2%, +1.1%] | - | ~ | device 19.3x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.32 ms | 2.30 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.08 ms | 6.17 ms | +1.4% | [-0.6%, +3.4%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.8 ms | -0.6% | [-1.5%, +0.4%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.1 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 57.2 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.9 ms | 20.6 ms | -1.5% | [-3.2%, +0.2%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.9 ms | 47.9 ms | +0.1% | [-0.7%, +0.9%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 60.7 ms | 61.4 ms | +1.2% | [+0.2%, +2.2%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 107 ms | -4.1% | [-4.3%, -3.8%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 388 µs | 386 µs | -0.6% | [-1.1%, +0.0%] | - | ~ | device 15.2x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 844 µs | 845 µs | +0.1% | [-0.2%, +0.5%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.53 ms | 4.51 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.65 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 587 µs | 591 µs | +0.7% | [+0.3%, +1.1%] | - | ~ | device 12.8x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 8.2x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.77 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.04 ms | +0.5% | [+0.3%, +0.7%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 906 µs | 905 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.24 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.49 ms | 9.48 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 319 µs | 320 µs | +0.2% | [-0.4%, +0.7%] | - | ~ | device 11.3x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 5.1x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.4x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.54 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 523 ms | 519 ms | -0.8% | [-1.6%, +0.0%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.5 µs | +1.4% | [-0.2%, +3.0%] | - | ~ | device 22.9x faster than the host; tape/fused 4.48x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.3 µs | 49.7 µs | -1.1% | [-1.8%, -0.4%] | - | ~ | device 43.4x faster than the host; tape/fused 7.26x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.7 µs | 48.2 µs | +0.9% | [-0.1%, +1.8%] | - | ~ | device 9.0x faster than the host; tape/fused 9.84x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.5 ms | 17.5 ms | -0.0% | [-0.7%, +0.7%] | - | ~ | device 6.7x faster than the host; tape/fused 1.79x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.4 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 809 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.56 ms | 1.55 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 282 ms | 283 ms | +0.4% | [-2.6%, +3.6%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 288 ms | 287 ms | -0.2% | [-0.8%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.29 ms | 8.19 ms | -1.3% | [-2.2%, -0.3%] | +0.0% | ~ | device 10.8x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.35 ms | 8.28 ms | -0.9% | [-1.4%, -0.3%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.15 ms | 8.20 ms | +0.6% | [+0.3%, +1.0%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.22 ms | 8.11 ms | -1.3% | [-2.5%, -0.0%] | +0.0% | ~ | device 11.0x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 709 µs | 711 µs | +0.3% | [-0.0%, +0.7%] | +0.0% | ~ | device 4.6x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.74 ms | 1.74 ms | -0.1% | [-0.3%, -0.0%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.41x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.04 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.68x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 845 µs | 845 µs | +0.1% | [-0.6%, +0.7%] | +0.0% | ~ | device 3.9x faster than the host; default/this 0.84x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 827 µs | 826 µs | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 11.6x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 944 µs | 951 µs | +0.7% | [+0.3%, +1.2%] | +0.0% | ~ | device 10.1x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 831 µs | 831 µs | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 11.5x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 803 µs | 809 µs | +0.7% | [-0.6%, +2.0%] | +0.0% | ~ | device 11.6x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 159 ms | -0.4% | [-3.8%, +3.1%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 163 ms | 159 ms | -2.6% | [-3.5%, -1.6%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 163 ms | 163 ms | -0.1% | [-5.2%, +5.4%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 164 ms | 160 ms | -2.5% | [-4.6%, -0.4%] | +0.0% | ~ | device 0.6x faster than the host (was 0.5x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.76 ms | 1.75 ms | -0.7% | [-1.5%, +0.1%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.91x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.92 ms | 1.94 ms | +0.6% | [-0.1%, +1.3%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.83x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.68 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.30 ms | 7.28 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.32 ms | 7.28 ms | -0.5% | [-1.3%, +0.3%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.29 ms | 7.29 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.31 ms | 7.34 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.46 ms | 8.45 ms | -0.1% | [-0.9%, +0.7%] | - | ~ | device 11.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.9 ms | 16.8 ms | -0.8% | [-2.1%, +0.5%] | - | ~ | device 7.0x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 165 ms | 165 ms | +0.4% | [-0.4%, +1.1%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 285 ms | 285 ms | -0.2% | [-1.2%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.6 ms | 24.7 ms | +0.4% | [+0.2%, +0.7%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 60.4 ms | 60.6 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 2.4x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 156 ms | 161 ms | +3.2% | [+1.2%, +5.2%] | - | minor | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 242 ms | 242 ms | -0.2% | [-2.2%, +1.7%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.94 ms | 2.90 ms | -1.4% | [-2.0%, -0.9%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.13 ms | 5.12 ms | -0.3% | [-0.7%, +0.0%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.4 ms | 14.5 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 3.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 122 ms | 119 ms | -2.1% | [-4.2%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 175 ms | 173 ms | -1.3% | [-2.2%, -0.4%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 384 ms | 373 ms | -2.8% | [-6.2%, +0.8%] | - | ~ | device 2.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.9 ms | 24.5 ms | -1.4% | [-2.9%, +0.1%] | - | ~ | device 2.0x faster than the host; tape/fused 4.87x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 27.2 ms | +0.8% | [+0.5%, +1.2%] | - | ~ | device 2.1x faster than the host; tape/fused 6.38x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.1 ms | 70.6 ms | -0.7% | [-1.0%, -0.5%] | - | ~ | device 4.1x faster than the host; tape/fused 5.29x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.2 ms | 23.5 ms | +5.6% | [+1.1%, +10.3%] | - | minor | device 0.4x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.1 ms | 44.4 ms | +0.6% | [-0.8%, +2.0%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 93.1 ms | 93.0 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 564 ms | 562 ms | -0.4% | [-3.8%, +3.2%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 856 ms | 859 ms | +0.3% | [-0.8%, +1.6%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.21 s | 3.24 s | +0.9% | [-5.4%, +7.6%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 196 ms | 201 ms | +2.7% | [+1.7%, +3.8%] | - | ~ | device 0.2x faster than the host; tape/fused 2.79x (was 2.88x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 311 ms | 305 ms | -1.9% | [-6.4%, +3.0%] | - | ~ | device 0.2x faster than the host; tape/fused 2.82x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 573 ms | 592 ms | +3.5% | [+1.5%, +5.5%] | - | minor | device 0.5x faster than the host (was 0.5x); tape/fused 5.47x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 770 µs | 766 µs | -0.5% | [-1.1%, +0.0%] | - | ~ | device 10.8x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.22 ms | 2.22 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.17 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 8.3x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.45 ms | 6.46 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 4.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 4.13 ms | 4.12 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 4.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.09 ms | 7.09 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.83 ms | 7.80 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 1.8x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.5 ms | 20.5 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.18 ms | 2.18 ms | +0.1% | [-1.0%, +1.1%] | - | ~ | device 4.9x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.98 ms | 5.95 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 2.6x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 837 µs | 834 µs | -0.3% | [-0.6%, +0.1%] | - | ~ | device 10.5x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 9.18 ms | 9.04 ms | -1.5% | [-2.0%, -1.1%] | - | ~ | device 3.9x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 6.46 ms | 6.47 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 4.1x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.05 ms | 8.11 ms | +0.7% | [+0.4%, +1.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 15.9 ms | 15.9 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 1.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.01 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.2x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 20.3 ms | 20.5 ms | +0.7% | [-0.0%, +1.5%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.1 ms | 15.3 ms | +1.3% | [+0.3%, +2.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.50 ms | 1.49 ms | -0.5% | [-1.1%, +0.2%] | - | ~ | device 8.7x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.73 ms | 4.73 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.40 ms | 1.38 ms | -1.0% | [-2.1%, -0.0%] | - | ~ | device 5.3x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.40 ms | 3.39 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.61 ms | 4.61 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 4.2x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.4 ms | 13.4 ms | -0.3% | [-1.2%, +0.7%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 17.2 ms | 17.2 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 7.53 ms | 7.51 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 11.3 ms | 11.3 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 15.9 ms | 15.8 ms | -0.4% | [-1.6%, +0.8%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 4.85 ms | 4.88 ms | +0.5% | [+0.1%, +0.8%] | - | ~ | device 3.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 16.0 ms | 16.0 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.62 ms | 1.62 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 7.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.61 ms | 5.61 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 4.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 13.4 ms | 13.3 ms | -0.3% | [-0.7%, +0.0%] | - | ~ | device 1.9x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 34.2 ms | 33.9 ms | -0.9% | [-1.1%, -0.7%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.6 ms | 68.5 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.4 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 21.0 ms | 20.9 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 39.2 ms | 38.8 ms | -1.2% | [-2.0%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 676 µs | 673 µs | -0.4% | [-1.9%, +1.1%] | - | ~ | device 16.8x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.6% | [-1.3%, -0.0%] | - | ~ | device 14.6x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 648 µs | 648 µs | -0.0% | [-0.4%, +0.3%] | - | ~ | device 17.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 612 µs | 601 µs | -1.8% | [-2.6%, -1.1%] | - | ~ | device 16.2x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 14.8x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.19 ms | +0.6% | [+0.5%, +0.6%] | - | ~ | device 12.0x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.48 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 11.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.17 ms | 2.18 ms | +0.4% | [-0.0%, +0.9%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 614 µs | 613 µs | -0.2% | [-5.3%, +5.2%] | - | ~ | device 2.4x faster than the host; tape/fused 1.15x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 663 µs | 730 µs | +10.1% | [+9.7%, +10.6%] | - | **MAJOR** | device 2.5x faster than the host (was 2.8x); tape/fused 1.15x (was 1.27x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 709 µs | 706 µs | -0.4% | [-0.6%, -0.2%] | - | ~ | device 3.7x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 844 µs | 839 µs | -0.6% | [-1.0%, -0.2%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.9% | [-2.0%, +0.2%] | - | ~ | device 0.7x faster than the host; hand/api 1.01x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.7% | [+0.3%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.9% | [-1.4%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.4 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.4 ms | 16.5 ms | +0.4% | [+0.0%, +0.8%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 1.0x faster than the host (was 1.0x); triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.3% | [-0.4%, +1.0%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 23.2 ms | +0.4% | [+0.3%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.61 ms | 2.60 ms | -0.6% | [-0.9%, -0.4%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.59 ms | 1.60 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.73 ms | 2.72 ms | -0.4% | [-0.8%, +0.0%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.59 ms | 2.59 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.9x faster than the host; marched/traced 3.49x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.02 ms | 3.02 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.7x faster than the host; marched/traced 4.60x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.07 ms | 9.04 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.17 ms | 2.17 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 3.9x faster than the host; marched/traced 16.59x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.43 ms | 5.42 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 2.8x faster than the host; marched/traced 13.19x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 36.0 ms | 36.0 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.3 ms | 71.5 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 1.7x faster than the host (was 1.8x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.23 ms | 5.25 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 3.1x faster than the host (was 3.0x) |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.80 ms | 8.79 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.25 ms | 6.25 ms | -0.0% | [-0.8%, +0.8%] | - | ~ | device 1.2x faster than the host; marched/traced 25.28x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 0.8x faster than the host; marched/traced 24.34x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 160 ms | 158 ms | -1.4% | [-3.7%, +1.0%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 252 ms | 258 ms | +2.0% | [+0.7%, +3.4%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.84 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 22.90x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 1.3x faster than the host; marched/traced 18.09x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 110 ms | 111 ms | +0.5% | [-1.7%, +2.9%] | - | ~ | device 0.7x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 217 ms | 221 ms | +1.6% | [+0.1%, +3.1%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.2 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.8 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.15x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | - | - | - | - | - | new | hit/crossing 4.69x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | - | - | - | - | - | new | hit/crossing 2.99x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 2.02x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.09x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.09x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.18x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | - | - | - | - | - | new | hit/crossing 1.35x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.59 ms | 3.68 ms | +2.5% | [-0.6%, +5.8%] | - | ~ | device 1.1x faster than the host; marched/traced 5.42x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.76 ms | 4.76 ms | -0.0% | [-0.8%, +0.7%] | - | ~ | device 0.9x faster than the host; marched/traced 8.42x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.7 ms | 19.9 ms | +1.1% | [-0.1%, +2.3%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.2 ms | 40.1 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.24 ms | +0.3% | [-2.4%, +3.1%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.83 ms | 1.84 ms | +0.5% | [+0.1%, +1.0%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.0 µs | 91.5 µs | -3.7% | [-5.2%, -2.2%] | - | faster | device 16.1x faster than the host (was 15.5x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 151 µs | 151 µs | -0.3% | [-1.5%, +0.9%] | - | ~ | device 10.9x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 799 µs | 799 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 935 µs | 934 µs | -0.2% | [-0.4%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.4 ms | 35.6 ms | +0.7% | [-6.5%, +8.2%] | - | ~ | device 0.1x faster than the host; marched/traced 3.47x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.9 ms | 36.9 ms | -0.1% | [-2.1%, +1.9%] | - | ~ | device 0.1x faster than the host; marched/traced 6.28x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 124 ms | 124 ms | -0.5% | [-2.3%, +1.4%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 234 ms | 231 ms | -1.0% | [-3.4%, +1.5%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.5 ms | 16.3 ms | -1.6% | [-5.5%, +2.6%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 22.1 ms | 21.6 ms | -2.2% | [-2.6%, -1.8%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.03 ms | 1.08 ms | +4.7% | [-4.0%, +13.5%] | - | ~ | device 1.4x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.34 ms | 2.34 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.99 ms | 5.96 ms | -0.5% | [-0.6%, -0.4%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.71 ms | 6.73 ms | +0.3% | [-0.3%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.2 ms | 23.2 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 2.1x faster than the host; marched/traced 4.27x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.6 ms | 28.6 ms | +0.1% | [-0.8%, +0.9%] | - | ~ | device 1.9x faster than the host; marched/traced 5.49x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 99.8 ms | 99.1 ms | -0.6% | [-1.5%, +0.3%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 160 ms | 157 ms | -1.7% | [-2.7%, -0.8%] | - | ~ | device 1.6x faster than the host (was 1.5x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.7 ms | -0.9% | [-1.9%, +0.0%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.3 ms | -0.1% | [-1.0%, +0.8%] | - | ~ | device 3.3x faster than the host (was 3.1x) |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 306 ms | 304 ms | -0.8% | [-3.8%, +2.4%] | - | ~ | device 0.2x faster than the host; marched/traced 1.86x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 347 ms | 349 ms | +0.7% | [+0.2%, +1.1%] | - | ~ | device 0.2x faster than the host; marched/traced 2.82x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 566 ms | 566 ms | +0.0% | [-0.8%, +0.9%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.00 s | 985 ms | -1.8% | [-4.0%, +0.4%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 191 ms | 192 ms | +0.4% | [-2.2%, +3.0%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 241 ms | 241 ms | +0.2% | [-2.6%, +3.0%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.63 ms | 4.63 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.9x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.63 ms | 4.63 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.89 ms | +0.3% | [-0.6%, +1.2%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 8.15 ms | 8.14 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 8.12 ms | 8.12 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.3 ms | 23.2 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 218 µs | +0.4% | [+0.2%, +0.6%] | - | ~ | device 11.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | +0.1% | [-0.3%, +0.5%] | - | ~ | device 24.5x faster than the host (was 22.9x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 361 µs | +0.2% | [-0.2%, +0.6%] | - | ~ | device 12.6x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.99 ms | 1.99 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.4 µs | 72.5 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 6.7x faster than the host (was 7.0x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.4 µs | 47.8 µs | +0.8% | [+0.2%, +1.4%] | - | ~ | device 4.8x faster than the host (was 5.0x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 651 µs | -0.1% | [-0.1%, +0.0%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.1% | [+0.0%, +0.1%] | - | ~ | device 12.0x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 473 µs | 474 µs | +0.2% | [-0.2%, +0.7%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 27.7x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 854 µs | 855 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.72 ms | 6.72 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 190 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 2.7x faster than the host (was 2.8x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.77 ms | 2.78 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |


The probes differ between the sides (each side ran its own): Performance/RayCrossing/main.cpp

<!-- END receipt cuda -->
