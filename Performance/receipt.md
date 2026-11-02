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
verdict: red
base: ba83c75
base-fingerprint: 1759bc965c8f27541a01db9aadccd206bf503ecf
head: 567fd7f
head-fingerprint: af0dc67e87d746569185658574d36ee7de109ddf
cases: 184
rounds: 2
reps: 3
date: 2026-11-02
-->

## Host comparison: red

**Accepted:** The maintainer, for the PointCloudBVH sphere queries: "Accept as worth the side effect"; and for the PointCloudHashGrid build: "I accept the slower build".

`567fd7f` against `ba83c75` (origin/main), 2026-11-02.

**VERDICT: MAJOR regression in 11 cases; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 11 major, 1 minor, 28 faster, 0 differ, 0 lost, 184 cases

- base: ba83c75 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 567fd7f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 331 ms | 332 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 516 ms | +0.6% | [+0.4%, +0.9%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 349 ms | 348 ms | -0.5% | [-1.2%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 187 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 561 ms | 562 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.1 ms | 54.9 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.1 ms | 51.2 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.4 ms | 65.1 ms | +1.2% | [+0.5%, +1.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.2 ms | 54.7 ms | +0.9% | [+0.6%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.9 ms | 61.3 ms | +0.6% | [-2.0%, +3.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.3 ms | 72.2 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.4 ms | 74.2 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.5 ms | 77.6 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.5 ms | 66.6 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 99.8 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.7 ms | 82.8 ms | -1.0% | [-1.6%, -0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 110 ms | +1.4% | [-1.0%, +3.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.7 ms | 19.7 ms | -0.3% | [-1.4%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.3 ms | 45.0 ms | -0.6% | [-2.0%, +0.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.9 ms | 73.3 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.0 ms | 20.1 ms | +0.4% | [-0.3%, +1.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.8 ms | 47.1 ms | +3.0% | [+2.8%, +3.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.6 ms | 80.8 ms | -1.0% | [-2.6%, +0.6%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.46 ms | 4.48 ms | +0.6% | [-0.5%, +1.7%] | - | ~ | tape/fused 3.00x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.2 ms | -0.5% | [-1.2%, +0.3%] | - | ~ | tape/fused 2.09x |
| `Fused/chain16/signedDistance/float/host` | 961 µs | 963 µs | +0.2% | [-1.4%, +1.8%] | - | ~ | tape/fused 17.04x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.0 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.4 ms | 46.3 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.55 ms | 4.54 ms | -0.2% | [-1.2%, +0.7%] | - | ~ | tape/fused 2.93x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.7 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | tape/fused 2.11x |
| `Fused/chain16/signedDistance/double/host` | 3.86 ms | 3.89 ms | +0.7% | [-0.2%, +1.7%] | - | ~ | tape/fused 5.34x (was 4.30x) |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.0 ms | 52.8 ms | -0.5% | [-1.7%, +0.8%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.6 ms | 53.0 ms | -1.1% | [-3.4%, +1.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.2 ms | 82.7 ms | +0.7% | [-0.8%, +2.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 228 ms | 228 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 112 ms | 112 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 261 ms | 260 ms | -0.2% | [-1.5%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 69.9 ms | 69.8 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 215 ms | 217 ms | +1.0% | [-1.5%, +3.6%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.3 ms | 96.9 ms | +0.7% | [+0.2%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.4 ms | 29.9 ms | +1.5% | [+0.3%, +2.8%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 775 ms | 759 ms | -2.0% | [-5.0%, +1.1%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 117 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.1 ms | 43.4 ms | +0.8% | [-1.3%, +2.8%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 861 ms | 848 ms | -1.6% | [-3.9%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.5 ms | 26.5 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.0 ms | 26.6 ms | -1.3% | [-1.7%, -0.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 321 ms | +0.9% | [+0.8%, +1.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 238 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 89.8 ms | 90.3 ms | +0.5% | [+0.2%, +0.9%] | +0.0% | ~ | tape/fused 3.56x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.6 ms | 99.6 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | tape/fused 2.39x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.4 ms | 29.3 ms | -0.4% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 29.9 ms | 30.0 ms | +0.1% | [-1.4%, +1.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 359 ms | 358 ms | -0.3% | [-2.1%, +1.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 253 ms | 260 ms | +2.8% | [+0.9%, +4.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.9 ms | 97.9 ms | +0.1% | [-2.8%, +3.0%] | +0.0% | ~ | tape/fused 3.66x |
| `MeshCSG/smooth-fused/frame/double/host` | 108 ms | 108 ms | +0.1% | [-3.4%, +3.7%] | +0.0% | ~ | tape/fused 2.40x |
| `PointCloudBVH/uniform/build/float/host` | 67.4 ms | 52.5 ms | -22.1% | [-22.4%, -21.8%] | -0.0% | faster |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 150 ms | 142 ms | -5.2% | [-5.4%, -5.0%] | - | faster |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 374 ms | 349 ms | -6.7% | [-6.9%, -6.6%] | - | faster |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.5 ms | 77.8 ms | -0.8% | [-1.0%, -0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.71 ms | 6.64 ms | -1.1% | [-2.3%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 79.6 ms | 56.9 ms | -28.5% | [-29.4%, -27.6%] | -0.0% | faster |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 161 ms | 151 ms | -5.9% | [-6.4%, -5.4%] | - | faster |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 402 ms | 377 ms | -6.2% | [-6.3%, -6.1%] | - | faster |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.0 ms | 86.9 ms | -0.1% | [-1.1%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.12 ms | 6.89 ms | -3.3% | [-3.6%, -3.1%] | - | faster |  |
| `PointCloudHashGrid/uniform/build/float/host` | 10.5 ms | 13.5 ms | +27.8% | [+16.9%, +40.9%] | +19.9% | **MAJOR** | pool bytes up |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 125 ms | 118 ms | -5.6% | [-5.8%, -5.5%] | - | faster |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 534 ms | 474 ms | -11.3% | [-11.4%, -11.2%] | - | faster |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 141 ms | -6.9% | [-7.0%, -6.8%] | - | faster |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 905 ms | 798 ms | -11.8% | [-11.9%, -11.7%] | - | faster |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 14.7 ms | -1.2% | [-1.5%, -1.0%] | +12.5% | **MAJOR** | pool bytes up |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 126 ms | 120 ms | -4.7% | [-4.9%, -4.6%] | - | faster |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 554 ms | 491 ms | -11.4% | [-11.5%, -11.2%] | - | faster |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 154 ms | 144 ms | -6.8% | [-7.1%, -6.5%] | - | faster |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 942 ms | 788 ms | -16.3% | [-16.9%, -15.8%] | - | faster |  |
| `PointCloudBVH/sphere/build/float/host` | 66.5 ms | 53.7 ms | -19.2% | [-19.4%, -19.1%] | +1.1% | faster |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 113 ms | 128 ms | +13.3% | [+11.6%, +15.0%] | - | **MAJOR** |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 271 ms | 262 ms | -3.4% | [-3.8%, -3.0%] | - | faster |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 114 ms | 144 ms | +26.7% | [+26.1%, +27.4%] | - | **MAJOR** |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 109 ms | 129 ms | +17.9% | [+15.9%, +19.9%] | - | **MAJOR** | the points found differ (index checksum) |
| `PointCloudBVH/sphere/build/double/host` | 79.0 ms | 57.8 ms | -26.8% | [-27.0%, -26.7%] | +1.0% | faster |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 121 ms | 137 ms | +12.8% | [+12.3%, +13.3%] | - | **MAJOR** |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 298 ms | 288 ms | -3.3% | [-3.6%, -3.1%] | - | faster |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 127 ms | 161 ms | +26.5% | [+26.3%, +26.8%] | - | **MAJOR** |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 120 ms | 144 ms | +19.6% | [+18.9%, +20.3%] | - | **MAJOR** |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.77 ms | 11.5 ms | +31.7% | [+31.3%, +32.2%] | +19.9% | **MAJOR** | pool bytes up |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 143 ms | 114 ms | -20.5% | [-20.8%, -20.2%] | - | faster |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 402 ms | 318 ms | -20.9% | [-22.4%, -19.4%] | - | faster |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 252 ms | 157 ms | -37.5% | [-38.0%, -37.1%] | - | faster |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 359 ms | 320 ms | -11.0% | [-12.0%, -9.9%] | - | faster |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.4 ms | 13.6 ms | +9.3% | [+6.7%, +12.0%] | +12.5% | **MAJOR** | pool bytes up |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 146 ms | 117 ms | -19.9% | [-21.1%, -18.7%] | - | faster |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 424 ms | 348 ms | -17.9% | [-19.3%, -16.3%] | - | faster |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 287 ms | 166 ms | -42.3% | [-42.7%, -41.9%] | - | faster |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 373 ms | 308 ms | -17.4% | [-18.1%, -16.7%] | - | faster |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | - | - | - | - | - | new |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | - | - | - | - | - | new |  |
| `Ray/mesh/firstHit/float/host` | 46.3 ms | 46.3 ms | +0.1% | [-0.5%, +0.6%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.5 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.5 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 79.0 ms | 78.5 ms | -0.6% | [-2.1%, +0.8%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 66.7 ms | 65.5 ms | -1.8% | [-4.8%, +1.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.96 ms | 5.85 ms | -1.8% | [-3.5%, +0.0%] | - | ~ | tape/fused 1.35x |
| `Ray/csg-tape/firstHit/float/host` | 8.28 ms | 7.92 ms | -4.4% | [-5.2%, -3.6%] | +0.0% | faster |  |
| `Ray/mesh/firstHit/double/host` | 53.5 ms | 52.2 ms | -2.4% | [-3.7%, -1.2%] | +0.0% | ~ | hand/api 1.05x (was 0.96x) |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.2 ms | 54.6 ms | +6.6% | [-0.9%, +14.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.1 ms | 44.5 ms | -1.2% | [-2.2%, -0.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.3 ms | 87.2 ms | -0.1% | [-2.2%, +2.1%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.2 ms | 73.6 ms | +0.6% | [-0.6%, +1.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.96 ms | 5.97 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | tape/fused 1.39x |
| `Ray/csg-tape/firstHit/double/host` | 8.29 ms | 8.30 ms | +0.0% | [-1.0%, +1.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.5 ms | 18.8 ms | +7.1% | [+7.0%, +7.3%] | - | minor | marched/traced 6.36x (was 6.85x) |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 119 ms | -0.5% | [-1.0%, -0.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.8 ms | 21.9 ms | +0.4% | [-0.1%, +1.0%] | +0.0% | ~ | marched/traced 7.04x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 156 ms | 154 ms | -0.9% | [-2.2%, +0.5%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.2 ms | 35.2 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.4 ms | 19.7 ms | +1.7% | [-1.2%, +4.7%] | - | ~ | marched/traced 6.41x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 127 ms | +0.5% | [+0.2%, +0.7%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 23.5 ms | +0.1% | [-1.5%, +1.7%] | +0.0% | ~ | marched/traced 6.81x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 160 ms | 160 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.8 ms | 38.0 ms | +0.6% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.3 ms | 46.8 ms | +1.1% | [+0.6%, +1.6%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.4 ms | 46.9 ms | +1.1% | [-0.3%, +2.4%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.1 ms | 10.1 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.69 ms | 8.74 ms | +0.6% | [-0.1%, +1.2%] | +0.0% | ~ | hit/crossing 1.16x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 183 ms | 181 ms | -1.1% | [-2.3%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 161 ms | 161 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.9 ms | 69.1 ms | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 58.6 ms | 58.8 ms | +0.4% | [-6.1%, +7.2%] | +0.0% | ~ | hit/crossing 1.17x |
| `RayCrossing/mesh/firstHit/double/host` | 59.0 ms | 55.6 ms | -5.8% | [-19.5%, +11.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 58.9 ms | 59.0 ms | +0.2% | [-18.2%, +22.7%] | +0.0% | ~ | hit/crossing 0.94x (was 1.00x) |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.2 ms | 11.2 ms | -0.1% | [-2.9%, +2.7%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.64 ms | 9.53 ms | -1.2% | [-3.1%, +0.8%] | +0.0% | ~ | hit/crossing 1.18x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 229 ms | 228 ms | -0.3% | [-16.8%, +19.6%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 205 ms | 203 ms | -0.7% | [-19.0%, +21.8%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 88.0 ms | 87.7 ms | -0.4% | [-20.1%, +24.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 71.3 ms | 71.4 ms | +0.1% | [-16.7%, +20.1%] | +0.0% | ~ | hit/crossing 1.23x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.4 ms | 11.4 ms | +0.1% | [-7.5%, +8.3%] | +0.0% | ~ | marched/traced 4.65x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 53.3 ms | 53.1 ms | -0.4% | [-6.0%, +5.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.10 ms | 8.08 ms | -0.2% | [-2.4%, +1.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.94 ms | 1.93 ms | -0.4% | [-2.5%, +1.8%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.92 ms | 6.95 ms | +0.5% | [+0.4%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.2 ms | 11.9 ms | -2.5% | [-9.4%, +5.0%] | +0.0% | ~ | marched/traced 5.53x (was 5.20x) |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 63.5 ms | 65.8 ms | +3.6% | [-6.5%, +14.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.74 ms | 8.88 ms | +1.7% | [-2.2%, +5.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.97 ms | 1.98 ms | +0.4% | [-4.0%, +5.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.14 ms | 7.29 ms | +2.1% | [+0.9%, +3.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 241 ms | 241 ms | -0.1% | [-2.3%, +2.2%] | +0.0% | ~ | marched/traced 1.39x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 335 ms | 336 ms | +0.1% | [-3.2%, +3.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 250 ms | 250 ms | -0.1% | [-9.4%, +10.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 268 ms | 269 ms | +0.3% | [-10.0%, +11.8%] | +0.0% | ~ | marched/traced 1.66x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 452 ms | 447 ms | -1.0% | [-31.4%, +43.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 275 ms | 279 ms | +1.7% | [-14.6%, +20.9%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 24.8 ms | 24.5 ms | -1.0% | [-9.7%, +8.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 24.8 ms | 24.7 ms | -0.5% | [-10.2%, +10.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.0 ms | 19.9 ms | -0.4% | [-13.7%, +15.0%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 31.6 ms | 31.7 ms | +0.4% | [-8.8%, +10.5%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 31.5 ms | 31.6 ms | +0.2% | [-9.0%, +10.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.8 ms | 21.8 ms | +0.2% | [-10.6%, +12.6%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 13.3 ms | 13.5 ms | +1.4% | [-4.9%, +8.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.59 ms | 4.59 ms | +0.2% | [-2.9%, +3.4%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.5 ms | 25.6 ms | +0.4% | [-4.0%, +4.9%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 41.0 ms | 41.1 ms | +0.1% | [-4.2%, +4.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.74 ms | 1.79 ms | +3.1% | [-1.0%, +7.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 137 µs | 128 µs | -6.6% | [-17.5%, +5.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.9 ms | 30.8 ms | -0.3% | [-6.7%, +6.5%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.71 ms | 5.78 ms | +1.2% | [-1.7%, +4.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.5 ms | 16.4 ms | -0.8% | [-4.3%, +2.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.66 ms | 4.69 ms | +0.7% | [-1.9%, +3.3%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.3 ms | 13.3 ms | +0.3% | [-3.1%, +3.9%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.67 ms | 4.66 ms | -0.3% | [-1.9%, +1.4%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 27.0 ms | 26.9 ms | -0.3% | [-4.0%, +3.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 43.0 ms | 43.9 ms | +2.2% | [-3.3%, +7.9%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.79 ms | 1.81 ms | +1.2% | [-2.3%, +4.8%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 299 µs | 286 µs | -4.6% | [-16.1%, +7.9%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.5 ms | 30.4 ms | -0.4% | [-4.2%, +3.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.82 ms | 7.81 ms | -0.1% | [-2.3%, +2.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.6 ms | 20.8 ms | +25.3% | [+0.3%, +51.3%] | +0.0% | **MAJOR** |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.85 ms | 7.83 ms | -0.2% | [-2.8%, +2.4%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/PointCloudBVHBuild/main.cpp, Performance/PointCloudHashGridBuild/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: ba83c75
base-fingerprint: 1759bc965c8f27541a01db9aadccd206bf503ecf
head: 567fd7f
head-fingerprint: af0dc67e87d746569185658574d36ee7de109ddf
cases: 276
rounds: 2
reps: 3
date: 2026-11-02
-->

## Device comparison (CUDA): red

**Accepted:** The maintainer, for the PointCloudBVH sphere queries: "Yes, the slowdown is intentional due to consolidation of build algorithms".

`567fd7f` against `ba83c75` (origin/main), 2026-11-02.

**VERDICT: MAJOR regression in 8 cases; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 8 major, 6 minor, 20 faster, 0 differ, 0 lost, 276 cases

- base: ba83c75 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 567fd7f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.61 ms | +0.3% | [-0.7%, +1.3%] | - | ~ | device 28.2x faster than the host (was 22.8x) |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.40 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 18.8x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.07 ms | 1.06 ms | -0.5% | [-2.3%, +1.3%] | - | ~ | device 19.4x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.30 ms | 2.30 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 16.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.15 ms | 6.34 ms | +3.1% | [+1.7%, +4.4%] | - | minor | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.8 ms | -0.5% | [-1.9%, +0.9%] | - | ~ | device 5.4x faster than the host (was 5.6x) |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.4 ms | 33.3 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 56.9 ms | 56.9 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.6 ms | 20.9 ms | +1.8% | [-0.5%, +4.2%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.8 ms | 48.0 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.7 ms | 61.2 ms | -0.7% | [-1.4%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 109 ms | -2.2% | [-3.9%, -0.5%] | - | ~ | device 0.6x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 388 µs | 389 µs | +0.3% | [-0.2%, +0.8%] | - | ~ | device 15.2x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 844 µs | 843 µs | -0.1% | [-0.3%, +0.0%] | - | ~ | device 10.2x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.54 ms | 4.53 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.3x faster than the host (was 1.6x) |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.66 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 586 µs | 585 µs | -0.1% | [-0.9%, +0.6%] | - | ~ | device 12.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.37 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 8.4x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.76 ms | 6.76 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.03 ms | 7.00 ms | -0.4% | [-0.4%, -0.3%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 902 µs | 899 µs | -0.4% | [-0.9%, +0.0%] | - | ~ | device 14.5x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.22 ms | 2.22 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.50 ms | 9.47 ms | -0.4% | [-0.8%, +0.0%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.4% | [-0.8%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 319 µs | 319 µs | -0.3% | [-0.7%, +0.1%] | - | ~ | device 11.4x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 5.3x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.0 ms | 59.0 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.4x faster than the host (was 3.8x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.54 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 519 ms | 523 ms | +0.7% | [+0.5%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 48.1 µs | 48.0 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 24.5x faster than the host (was 23.8x); tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.0 µs | 50.0 µs | -0.1% | [-0.5%, +0.3%] | - | ~ | device 43.3x faster than the host (was 44.7x); tape/fused 7.22x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.4 µs | 48.4 µs | +0.1% | [-0.6%, +0.9%] | - | ~ | device 9.3x faster than the host; tape/fused 9.76x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.5 ms | 17.3 ms | -1.3% | [-1.7%, -0.9%] | - | ~ | device 6.8x faster than the host (was 9.1x); tape/fused 1.82x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.4 ms | 31.4 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 815 µs | 810 µs | -0.6% | [-1.3%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 277 ms | 279 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 0.5x faster than the host; tape/fused 1.02x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 288 ms | 284 ms | -1.1% | [-2.8%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.24 ms | 8.35 ms | +1.3% | [+0.0%, +2.5%] | +0.0% | ~ | device 10.8x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.29 ms | 8.33 ms | +0.5% | [-0.9%, +1.9%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.24 ms | 8.16 ms | -1.0% | [-2.6%, +0.5%] | +0.0% | ~ | device 11.0x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.21 ms | 8.26 ms | +0.6% | [+0.2%, +0.9%] | +0.0% | ~ | device 12.3x faster than the host (was 15.0x); default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 708 µs | 712 µs | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ | device 5.8x faster than the host (was 4.6x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.74 ms | 1.74 ms | +0.1% | [+0.0%, +0.1%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.41x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.04 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.68x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 851 µs | 844 µs | -0.8% | [-1.4%, -0.3%] | +0.0% | ~ | device 3.9x faster than the host; default/this 0.84x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 830 µs | 828 µs | -0.3% | [-1.1%, +0.5%] | +0.8% | ~ | device 10.7x faster than the host (was 11.6x) |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 954 µs | 961 µs | +0.7% | [+0.3%, +1.1%] | +0.8% | ~ | device 9.6x faster than the host (was 10.1x); default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 836 µs | 822 µs | -1.7% | [-2.0%, -1.4%] | +0.8% | ~ | device 10.8x faster than the host (was 11.5x); default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 810 µs | 807 µs | -0.4% | [-0.7%, -0.1%] | +0.8% | ~ | device 10.8x faster than the host (was 11.7x); default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 163 ms | +2.0% | [+1.1%, +2.9%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 157 ms | 163 ms | +3.4% | [+2.1%, +4.6%] | +0.0% | minor | device 0.6x faster than the host (was 0.6x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 163 ms | 164 ms | +0.6% | [-3.3%, +4.6%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 158 ms | 159 ms | +0.3% | [-1.0%, +1.6%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.03x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.76 ms | 1.76 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.91x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.93 ms | 1.93 ms | +0.0% | [-0.8%, +0.8%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.83x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.68 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ | device 2.2x faster than the host (was 3.2x); default/this 0.96x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.27 ms | 7.28 ms | +0.1% | [-0.5%, +0.7%] | +0.9% | ~ | device 1.3x faster than the host (was 1.3x) |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.33 ms | 7.33 ms | +0.0% | [-1.0%, +1.1%] | +0.9% | ~ | device 1.2x faster than the host (was 1.3x); default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.29 ms | 7.27 ms | -0.3% | [-0.9%, +0.4%] | +0.9% | ~ | device 1.3x faster than the host (was 1.4x); default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.39 ms | 7.34 ms | -0.6% | [-1.3%, +0.1%] | +0.9% | ~ | device 1.3x faster than the host (was 1.3x); default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.46 ms | 8.49 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 11.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.7 ms | 16.7 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 7.2x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 162 ms | 166 ms | +2.8% | [+0.2%, +5.4%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 280 ms | 281 ms | +0.2% | [-2.3%, +2.8%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.6 ms | 24.5 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 60.1 ms | 59.9 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 3.2x faster than the host (was 2.4x) |
| `MeshSDF/query-morton/double/hostdevice` | 155 ms | 157 ms | +1.4% | [+0.4%, +2.4%] | - | ~ | device 0.9x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 247 ms | 245 ms | -0.6% | [-0.8%, -0.3%] | - | ~ | device 1.0x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.88 ms | 2.92 ms | +1.3% | [+1.0%, +1.7%] | - | ~ | device 5.7x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.12 ms | 5.14 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 4.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.5 ms | 14.5 ms | -0.6% | [-1.6%, +0.4%] | - | ~ | device 5.9x faster than the host (was 5.6x) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 124 ms | 121 ms | -2.6% | [-5.4%, +0.2%] | - | ~ | device 2.5x faster than the host (was 2.3x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 175 ms | 171 ms | -2.2% | [-4.2%, -0.0%] | - | ~ | device 1.9x faster than the host (was 1.8x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 376 ms | 381 ms | +1.4% | [+0.7%, +2.2%] | - | ~ | device 3.0x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.1 ms | 24.9 ms | -0.7% | [-2.2%, +0.9%] | - | ~ | device 3.2x faster than the host (was 3.1x); tape/fused 4.86x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 26.8 ms | -0.4% | [-1.3%, +0.5%] | - | ~ | device 3.6x faster than the host (was 3.4x); tape/fused 6.38x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.5 ms | 71.9 ms | +0.6% | [+0.1%, +1.2%] | - | ~ | device 5.9x faster than the host; tape/fused 5.30x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.9 ms | 22.4 ms | -2.1% | [-4.1%, -0.0%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.5 ms | 44.0 ms | -1.1% | [-2.0%, -0.1%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/frame/double/hostdevice` | 96.0 ms | 95.8 ms | -0.2% | [-3.1%, +2.7%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 526 ms | 565 ms | +7.4% | [+6.5%, +8.4%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 860 ms | 864 ms | +0.5% | [-0.9%, +1.9%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.24 s | 3.27 s | +1.0% | [-0.6%, +2.7%] | - | ~ | device 0.4x faster than the host (was 0.3x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 205 ms | 201 ms | -1.8% | [-2.5%, -1.0%] | - | ~ | device 0.2x faster than the host; tape/fused 2.81x (was 2.56x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 300 ms | 320 ms | +6.8% | [+4.5%, +9.2%] | - | minor | device 0.2x faster than the host (was 0.2x); tape/fused 2.70x (was 2.87x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 602 ms | 596 ms | -1.1% | [-3.9%, +1.8%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 5.49x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 766 µs | 769 µs | +0.4% | [-0.2%, +1.0%] | - | ~ | device 18.7x faster than the host (was 17.1x) |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.20 ms | 2.18 ms | -1.0% | [-1.5%, -0.4%] | - | ~ | device 9.0x faster than the host (was 12.1x) |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.18 ms | 1.17 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 12.6x faster than the host (was 9.7x) |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.45 ms | 6.22 ms | -3.6% | [-3.8%, -3.4%] | - | faster | device 4.1x faster than the host (was 4.4x) |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 4.13 ms | 3.92 ms | -4.9% | [-5.1%, -4.7%] | - | faster | device 4.0x faster than the host (was 4.3x); the points found differ (index checksum) |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.08 ms | 7.17 ms | +1.2% | [+0.8%, +1.5%] | - | ~ | device 1.1x faster than the host (was 1.2x) |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.81 ms | 7.93 ms | +1.5% | [+0.7%, +2.3%] | - | ~ | device 1.4x faster than the host (was 1.8x) |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.4 ms | +1.6% | [+1.4%, +1.8%] | - | ~ | device 0.7x faster than the host (was 0.8x) |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.5 ms | 20.9 ms | +2.3% | [+2.3%, +2.4%] | - | ~ | device 1.3x faster than the host (was 1.5x) |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 12.0 ms | +0.8% | [+0.5%, +1.1%] | - | ~ | device 1.3x faster than the host (was 1.5x) |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.15 ms | 1.47 ms | -31.7% | [-32.1%, -31.2%] | - | faster | device 6.5x faster than the host (was 5.0x) |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.94 ms | 3.87 ms | -34.8% | [-35.1%, -34.5%] | - | faster | device 3.3x faster than the host (was 2.7x) |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 831 µs | 524 µs | -37.0% | [-37.6%, -36.3%] | - | faster | device 15.9x faster than the host (was 10.6x) |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 8.96 ms | 4.41 ms | -50.8% | [-51.4%, -50.2%] | - | faster | device 6.9x faster than the host (was 4.0x) |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 6.47 ms | 4.27 ms | -34.0% | [-34.4%, -33.6%] | - | faster | device 5.3x faster than the host (was 4.1x) |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.15 ms | 8.09 ms | -0.8% | [-1.3%, -0.2%] | - | ~ | device 1.2x faster than the host (was 1.3x) |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 16.0 ms | 15.9 ms | -0.4% | [-1.4%, +0.6%] | - | ~ | device 0.9x faster than the host (was 1.1x) |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.02 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.1x faster than the host (was 2.2x) |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 20.4 ms | 17.4 ms | -14.5% | [-15.0%, -14.1%] | - | faster | device 1.7x faster than the host (was 1.7x) |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.2 ms | 14.8 ms | -3.1% | [-4.2%, -2.0%] | - | faster | device 1.5x faster than the host (was 1.8x) |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.48 ms | 1.82 ms | +22.4% | [+21.1%, +23.8%] | - | **MAJOR** | device 9.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.67 ms | 5.53 ms | +18.3% | [+18.2%, +18.5%] | - | **MAJOR** | device 4.3x faster than the host (was 4.1x) |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 918 µs | -33.4% | [-34.3%, -32.5%] | - | faster | device 9.2x faster than the host (was 5.4x) |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.40 ms | 3.72 ms | +9.4% | [+9.2%, +9.5%] | - | minor | device 5.4x faster than the host (was 5.5x) |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.58 ms | 5.47 ms | +19.3% | [+18.7%, +19.9%] | - | **MAJOR** | device 4.1x faster than the host (was 4.3x); the points found differ (index checksum) |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.4 ms | 16.9 ms | +26.0% | [+25.2%, +26.8%] | - | **MAJOR** | device 1.0x faster than the host (was 1.0x) |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 17.3 ms | 21.5 ms | +24.4% | [+24.2%, +24.6%] | - | **MAJOR** | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 7.53 ms | 8.96 ms | +19.0% | [+18.7%, +19.2%] | - | **MAJOR** | device 1.0x faster than the host (was 1.1x) |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 11.3 ms | 14.6 ms | +29.3% | [+29.1%, +29.5%] | - | **MAJOR** | device 1.4x faster than the host (was 1.7x) |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 15.8 ms | 20.3 ms | +28.7% | [+28.0%, +29.5%] | - | **MAJOR** | device 1.2x faster than the host (was 1.3x) |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 4.88 ms | 2.74 ms | -43.9% | [-44.0%, -43.8%] | - | faster | device 4.7x faster than the host (was 3.7x) |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 16.0 ms | 6.73 ms | -58.1% | [-58.1%, -58.0%] | - | faster | device 2.3x faster than the host (was 2.1x) |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.62 ms | 1.38 ms | -14.9% | [-15.0%, -14.8%] | - | faster | device 6.6x faster than the host (was 7.5x) |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.60 ms | 2.71 ms | -51.7% | [-51.8%, -51.5%] | - | faster | device 8.2x faster than the host (was 5.0x) |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 13.2 ms | 6.42 ms | -51.5% | [-52.1%, -51.0%] | - | faster | device 3.1x faster than the host (was 1.9x) |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.9 ms | 33.8 ms | -0.4% | [-1.1%, +0.2%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.6 ms | 68.5 ms | -0.2% | [-0.7%, +0.4%] | - | ~ | device 0.2x faster than the host (was 0.6x) |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.5 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 0.5x faster than the host (was 0.7x) |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 21.0 ms | 20.7 ms | -1.3% | [-1.7%, -0.9%] | - | ~ | device 1.0x faster than the host (was 1.3x) |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 39.4 ms | 38.4 ms | -2.5% | [-3.1%, -2.0%] | - | ~ | device 0.5x faster than the host (was 0.6x) |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | - | - | - | - | - | new |  |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 678 µs | 674 µs | -0.5% | [-0.8%, -0.3%] | - | ~ | device 16.9x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.3% | [-1.1%, +0.4%] | - | ~ | device 14.6x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 652 µs | 648 µs | -0.7% | [-1.6%, +0.3%] | - | ~ | device 24.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.23 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 15.3x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 612 µs | 599 µs | -2.1% | [-3.2%, -1.0%] | - | ~ | device 16.3x faster than the host (was 19.5x) |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.13 ms | 1.13 ms | +0.1% | [-0.9%, +1.0%] | - | ~ | device 14.9x faster than the host (was 20.0x) |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.19 ms | +1.1% | [+0.3%, +1.8%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.56x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.2% | [-0.9%, +1.2%] | - | ~ | device 11.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 9.9x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 614 µs | 613 µs | -0.3% | [-5.7%, +5.5%] | - | ~ | device 2.5x faster than the host; tape/fused 1.15x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 700 µs | 665 µs | -4.9% | [-9.3%, -0.1%] | - | faster | device 2.7x faster than the host (was 2.6x); tape/fused 1.26x (was 1.20x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 708 µs | 706 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 837 µs | 841 µs | +0.5% | [-0.0%, +0.9%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.4% | [-1.1%, +2.0%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 0.7x faster than the host; hand/api 1.01x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.9% | [+0.3%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.6 ms | +0.6% | [+0.3%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.5 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.2x faster than the host (was 0.9x); triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 1.1x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.2 ms | +1.6% | [+0.8%, +2.3%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.1 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.47 ms | 1.48 ms | +0.5% | [+0.0%, +1.0%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.60 ms | 2.60 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.71 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.59 ms | 2.59 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 3.0x faster than the host; marched/traced 3.48x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.04 ms | 3.04 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.7x faster than the host; marched/traced 4.57x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.03 ms | 9.02 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 4.5x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.3x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.17 ms | 2.17 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 4.0x faster than the host; marched/traced 16.54x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.42 ms | 5.42 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.9x faster than the host; marched/traced 13.15x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 35.9 ms | 35.9 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.7 ms | 71.2 ms | -0.6% | [-1.1%, -0.2%] | - | ~ | device 1.9x faster than the host (was 1.7x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.24 ms | 5.23 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.80 ms | 8.80 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.22 ms | 6.27 ms | +0.8% | [+0.4%, +1.2%] | - | ~ | device 1.3x faster than the host; marched/traced 25.39x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.4% | [-0.1%, +0.8%] | - | ~ | device 0.8x faster than the host; marched/traced 23.92x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 159 ms | -0.1% | [-2.3%, +2.1%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 254 ms | 255 ms | +0.1% | [-2.5%, +2.7%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.85 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.6x faster than the host (was 1.8x); marched/traced 23.37x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.3x faster than the host; marched/traced 18.05x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 112 ms | 113 ms | +0.9% | [-2.2%, +4.0%] | - | ~ | device 1.0x faster than the host (was 0.7x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 221 ms | 220 ms | -0.2% | [-3.1%, +2.8%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.4 ms | +0.3% | [-0.9%, +1.5%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.7 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 670 µs | 673 µs | +0.4% | [-1.0%, +1.9%] | - | ~ | device 17.4x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.29 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 675 µs | 671 µs | -0.6% | [-1.2%, -0.0%] | - | ~ | device 17.4x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.5% | [-1.1%, +0.1%] | - | ~ | device 15.1x faster than the host; hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 704 µs | 706 µs | +0.2% | [+0.0%, +0.3%] | - | ~ | device 4.8x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 838 µs | 832 µs | -0.7% | [-1.2%, -0.2%] | - | ~ | device 4.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 616 µs | 616 µs | -0.0% | [-0.9%, +0.8%] | - | ~ | device 4.5x faster than the host; hit/crossing 1.15x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 635 µs | 638 µs | +0.5% | [-0.1%, +1.2%] | - | ~ | device 5.0x faster than the host (was 4.9x); hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.70 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 10.4x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 7.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.19 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 10.3x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.09 ms | 9.09 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 7.0x faster than the host (was 7.4x); hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 276 ms | 260 ms | -5.9% | [-12.2%, +1.1%] | - | ~ | device 0.7x faster than the host (was 0.6x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 214 ms | 217 ms | +1.1% | [-1.9%, +4.3%] | - | ~ | device 0.9x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 54.4 ms | 55.1 ms | +1.2% | [+0.4%, +2.1%] | - | ~ | device 2.7x faster than the host; hit/crossing 4.72x (was 5.08x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 70.8 ms | 70.2 ms | -0.9% | [-1.9%, +0.1%] | - | ~ | device 2.2x faster than the host; hit/crossing 3.08x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.0% | [-0.6%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.1 ms | 16.9 ms | -0.9% | [-1.1%, -0.7%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.4% | [-0.8%, -0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.72 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.29 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.34 ms | 1.35 ms | +0.3% | [-0.4%, +0.9%] | - | ~ | device 2.6x faster than the host; hit/crossing 2.02x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.6 ms | 64.6 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 121 ms | 122 ms | +0.2% | [-1.3%, +1.7%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.8 ms | 58.7 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 112 ms | 113 ms | +0.4% | [-0.5%, +1.3%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.08x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 484 ms | 462 ms | -4.6% | [-5.8%, -3.3%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 711 ms | 712 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 386 ms | 384 ms | -0.6% | [-1.5%, +0.4%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.20x (was 1.25x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 540 ms | 535 ms | -1.0% | [-4.0%, +2.0%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.33x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.59 ms | 3.59 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 1.2x faster than the host; marched/traced 5.50x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.75 ms | 4.77 ms | +0.5% | [-0.4%, +1.3%] | - | ~ | device 0.9x faster than the host; marched/traced 8.40x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.8 ms | 19.8 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.2 ms | 40.1 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.24 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.84 ms | 1.85 ms | +0.8% | [+0.5%, +1.2%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.4 µs | 93.0 µs | -2.5% | [-4.2%, -0.8%] | - | ~ | device 15.6x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 151 µs | 150 µs | -0.9% | [-1.2%, -0.5%] | - | ~ | device 11.0x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 798 µs | 798 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 937 µs | 936 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.8x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.3 ms | 33.8 ms | -4.1% | [-6.2%, -1.9%] | - | faster | device 0.1x faster than the host (was 0.1x); marched/traced 3.68x (was 3.48x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.5 ms | 37.3 ms | +2.3% | [+1.3%, +3.3%] | - | ~ | device 0.1x faster than the host; marched/traced 6.33x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 123 ms | 124 ms | +1.4% | [+0.7%, +2.0%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 236 ms | 236 ms | +0.0% | [-2.3%, +2.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.9 ms | 16.0 ms | -5.3% | [-8.2%, -2.3%] | - | faster | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 22.0 ms | 21.9 ms | -0.2% | [-1.6%, +1.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.08 ms | 1.08 ms | -0.8% | [-5.4%, +3.9%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.37 ms | 2.35 ms | -0.5% | [-2.0%, +1.0%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.97 ms | 5.95 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.74 ms | +0.3% | [-0.3%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.1 ms | 23.0 ms | -0.7% | [-1.6%, +0.3%] | - | ~ | device 2.2x faster than the host; marched/traced 4.40x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.6 ms | +0.9% | [+0.3%, +1.6%] | - | ~ | device 2.0x faster than the host; marched/traced 5.53x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 98.9 ms | 101 ms | +2.3% | [+2.2%, +2.4%] | - | ~ | device 2.2x faster than the host (was 2.9x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 158 ms | 158 ms | -0.2% | [-2.2%, +2.0%] | - | ~ | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.8 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 3.7x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.3 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 308 ms | 305 ms | -0.9% | [-2.4%, +0.5%] | - | ~ | device 0.2x faster than the host; marched/traced 1.84x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 340 ms | 357 ms | +4.9% | [+2.4%, +7.3%] | - | minor | device 0.2x faster than the host (was 0.2x); marched/traced 2.78x (was 2.91x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 566 ms | 562 ms | -0.7% | [-2.2%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 991 ms | 994 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 0.4x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 191 ms | 194 ms | +2.0% | [+0.8%, +3.2%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 247 ms | 237 ms | -4.1% | [-4.9%, -3.2%] | - | faster | device 0.2x faster than the host (was 0.2x) |
| `Render/csg/frame/float/hostdevice` | 4.59 ms | 4.59 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.1x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.60 ms | 4.58 ms | -0.3% | [-0.3%, -0.3%] | - | ~ | device 3.6x faster than the host (was 3.0x) |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.89 ms | +0.4% | [-0.6%, +1.4%] | - | ~ | device 9.0x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.97 ms | 7.97 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.93 ms | 7.94 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.3 ms | 23.3 ms | +0.2% | [-0.9%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 219 µs | 218 µs | -0.4% | [-0.9%, +0.0%] | - | ~ | device 11.6x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.2% | [-0.4%, +0.1%] | - | ~ | device 23.3x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 361 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 12.8x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.99 ms | 1.98 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 7.3x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.3 µs | 72.2 µs | -0.2% | [-0.4%, +0.1%] | - | ~ | device 7.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.5 µs | 47.5 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 5.0x faster than the host (was 5.3x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 653 µs | 653 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 10.6x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.2% | [-0.2%, -0.1%] | - | ~ | device 12.4x faster than the host (was 11.9x) |
| `Tape/chain16/signedDistance/float/hostdevice` | 474 µs | 472 µs | -0.3% | [-0.6%, -0.1%] | - | ~ | device 7.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.4 µs | +0.6% | [-0.0%, +1.2%] | - | ~ | device 27.6x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 854 µs | 855 µs | +0.1% | [+0.0%, +0.3%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.71 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 190 µs | -0.3% | [-0.4%, -0.2%] | - | ~ | device 3.2x faster than the host (was 2.9x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.0x faster than the host (was 1.9x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 2.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |


The probes differ between the sides (each side ran its own): Performance/PointCloudBVHBuild/main.cpp, Performance/PointCloudHashGridBuild/main.cpp

<!-- END receipt cuda -->
