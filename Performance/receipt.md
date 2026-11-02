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
head: 40bd6b6
head-fingerprint: 60bfcfc855044f2fa7a4133c206533c55abdfd03
cases: 184
rounds: 2
reps: 3
date: 2026-11-02
-->

## Host comparison: red

**Accepted:** The maintainer, for the PointCloudBVH sphere queries: "Accept as worth the side effect"; and for the PointCloudHashGrid build: "I accept the slower build".

`40bd6b6` against `ba83c75` (origin/main), 2026-11-02.

**VERDICT: MAJOR regression in 10 cases; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 10 major, 4 minor, 27 faster, 0 differ, 0 lost, 184 cases

- base: ba83c75 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 40bd6b6 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 331 ms | 330 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 181 ms | +1.7% | [-0.2%, +3.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 514 ms | 516 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 351 ms | 349 ms | -0.8% | [-2.5%, +1.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 187 ms | 186 ms | -0.1% | [-1.0%, +0.8%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 568 ms | 564 ms | -0.6% | [-2.0%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.0 ms | 55.3 ms | +0.5% | [-0.5%, +1.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.9 ms | 51.6 ms | -0.5% | [-3.0%, +2.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.3 ms | 65.0 ms | -0.5% | [-1.3%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.4 ms | 54.4 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 61.6 ms | 60.7 ms | -1.5% | [-4.2%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.1 ms | 73.0 ms | +1.4% | [+0.0%, +2.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.2 ms | 75.1 ms | +1.2% | [-0.5%, +2.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 78.1 ms | 78.5 ms | +0.6% | [-1.4%, +2.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.9 ms | 66.8 ms | -0.1% | [-0.9%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 103 ms | 101 ms | -1.0% | [-4.4%, +2.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.5 ms | 83.6 ms | +0.1% | [-1.6%, +1.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 109 ms | +1.1% | [-0.3%, +2.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.6 ms | 19.5 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 44.5 ms | 45.3 ms | +1.8% | [-0.1%, +3.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.2 ms | 72.8 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.0 ms | 20.0 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 46.7 ms | 51.5 ms | +10.3% | [-1.0%, +22.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 83.0 ms | 83.1 ms | +0.2% | [-1.1%, +1.4%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.50 ms | 4.45 ms | -1.0% | [-2.4%, +0.4%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | tape/fused 2.03x |
| `Fused/chain16/signedDistance/float/host` | 970 µs | 962 µs | -0.8% | [-1.6%, +0.0%] | - | ~ | tape/fused 16.91x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.2 ms | 46.0 ms | -0.4% | [-1.4%, +0.8%] | +0.0% | ~ | tape/fused 1.02x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.9 ms | 46.7 ms | -0.4% | [-2.3%, +1.5%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.56 ms | 4.52 ms | -1.0% | [-2.0%, +0.0%] | - | ~ | tape/fused 2.91x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | tape/fused 2.10x |
| `Fused/chain16/signedDistance/double/host` | 3.86 ms | 3.89 ms | +0.8% | [-0.3%, +1.8%] | - | ~ | tape/fused 4.18x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.2 ms | 53.4 ms | +2.3% | [+0.6%, +4.0%] | +0.0% | ~ | tape/fused 0.99x (was 1.04x) |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 54.3 ms | 52.9 ms | -2.7% | [-5.6%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 83.3 ms | 81.8 ms | -1.8% | [-4.0%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 230 ms | +0.2% | [-1.0%, +1.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 117 ms | +3.0% | [-0.2%, +6.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 264 ms | +1.6% | [+1.2%, +1.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.3 ms | 70.1 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 216 ms | 219 ms | +1.6% | [-0.6%, +3.9%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.7 ms | 96.6 ms | -0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.9 ms | 30.2 ms | +1.0% | [-0.4%, +2.5%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 770 ms | 778 ms | +1.1% | [-1.9%, +4.2%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 117 ms | 118 ms | +0.3% | [-0.1%, +0.8%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.6 ms | 43.8 ms | -1.7% | [-6.2%, +3.2%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 865 ms | 867 ms | +0.2% | [-4.7%, +5.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.7 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.2 ms | 27.6 ms | +1.5% | [-2.0%, +5.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 321 ms | 319 ms | -0.6% | [-1.5%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 241 ms | +0.6% | [-0.3%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.7 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 101 ms | 101 ms | -0.2% | [-2.1%, +1.8%] | +0.0% | ~ | tape/fused 2.39x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.5 ms | 29.9 ms | +1.2% | [-1.1%, +3.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 29.9 ms | 31.1 ms | +3.8% | [+1.7%, +5.9%] | +0.0% | minor |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 355 ms | 370 ms | +4.1% | [+3.1%, +5.2%] | +0.0% | minor |  |
| `MeshCSG/smooth-tape/frame/double/host` | 255 ms | 259 ms | +1.6% | [-1.6%, +4.9%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 98.5 ms | 98.1 ms | -0.4% | [-2.7%, +2.0%] | +0.0% | ~ | tape/fused 3.77x (was 3.60x) |
| `MeshCSG/smooth-fused/frame/double/host` | 109 ms | 110 ms | +0.6% | [-4.6%, +6.0%] | +0.0% | ~ | tape/fused 2.36x |
| `PointCloudBVH/uniform/build/float/host` | 68.9 ms | 52.9 ms | -23.2% | [-25.3%, -21.0%] | -0.0% | faster |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 150 ms | 143 ms | -4.7% | [-5.3%, -4.2%] | - | faster |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 379 ms | 352 ms | -7.0% | [-8.0%, -6.0%] | - | faster |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.8 ms | 77.9 ms | -1.1% | [-1.2%, -1.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.72 ms | 6.61 ms | -1.6% | [-1.9%, -1.3%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 81.8 ms | 55.9 ms | -31.6% | [-33.8%, -29.4%] | -0.0% | faster |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 161 ms | 153 ms | -5.3% | [-6.7%, -3.8%] | - | faster |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 404 ms | 379 ms | -6.3% | [-7.1%, -5.5%] | - | faster |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.1 ms | 86.9 ms | -0.2% | [-1.7%, +1.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.23 ms | 6.96 ms | -3.8% | [-6.3%, -1.1%] | - | faster |  |
| `PointCloudHashGrid/uniform/build/float/host` | 12.3 ms | 14.2 ms | +15.7% | [+9.0%, +22.5%] | +19.9% | **MAJOR** | pool bytes up |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 125 ms | 118 ms | -5.7% | [-6.4%, -4.9%] | - | faster |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 538 ms | 476 ms | -11.6% | [-12.4%, -10.8%] | - | faster |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 153 ms | 141 ms | -7.8% | [-9.0%, -6.7%] | - | faster |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 908 ms | 802 ms | -11.6% | [-12.4%, -10.9%] | - | faster |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.1 ms | 14.9 ms | -1.4% | [-2.5%, -0.4%] | +12.5% | **MAJOR** | pool bytes up |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 128 ms | 120 ms | -6.1% | [-7.5%, -4.7%] | - | faster |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 557 ms | 492 ms | -11.7% | [-12.1%, -11.2%] | - | faster |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 156 ms | 145 ms | -6.9% | [-8.1%, -5.8%] | - | faster |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 971 ms | 794 ms | -18.2% | [-19.0%, -17.5%] | - | faster |  |
| `PointCloudBVH/sphere/build/float/host` | 67.0 ms | 54.7 ms | -18.4% | [-19.5%, -17.3%] | +1.1% | faster |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 112 ms | 129 ms | +15.2% | [+14.8%, +15.6%] | - | **MAJOR** |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 273 ms | 262 ms | -4.1% | [-4.3%, -3.9%] | - | faster |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 113 ms | 144 ms | +27.0% | [+26.7%, +27.3%] | - | **MAJOR** |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 109 ms | 130 ms | +19.6% | [+19.0%, +20.2%] | - | **MAJOR** | the points found differ (index checksum) |
| `PointCloudBVH/sphere/build/double/host` | 80.5 ms | 58.1 ms | -27.9% | [-29.4%, -26.3%] | +1.0% | faster |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 123 ms | 138 ms | +12.1% | [+11.6%, +12.6%] | - | **MAJOR** |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 302 ms | 290 ms | -4.0% | [-5.1%, -2.9%] | - | faster |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 155 ms | 162 ms | +4.5% | [-11.8%, +28.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 122 ms | 145 ms | +19.0% | [+17.7%, +20.4%] | - | **MAJOR** |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.92 ms | 11.6 ms | +30.3% | [+25.3%, +35.5%] | +19.9% | **MAJOR** | pool bytes up |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 148 ms | 116 ms | -21.6% | [-22.5%, -20.6%] | - | faster |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 404 ms | 323 ms | -20.1% | [-22.1%, -18.1%] | - | faster |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 253 ms | 155 ms | -39.0% | [-39.2%, -38.7%] | - | faster |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 358 ms | 321 ms | -10.4% | [-10.5%, -10.4%] | - | faster |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.9 ms | 13.5 ms | +5.2% | [+1.1%, +9.7%] | +12.5% | **MAJOR** | pool bytes up |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 149 ms | 116 ms | -22.4% | [-23.8%, -20.9%] | - | faster |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 420 ms | 349 ms | -16.8% | [-17.3%, -16.2%] | - | faster |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 291 ms | 171 ms | -41.1% | [-43.8%, -38.4%] | - | faster |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 379 ms | 309 ms | -18.5% | [-19.6%, -17.3%] | - | faster |  |
| `PointCloudBVHBuild/uniform/rebuild/float/host` | - | - | - | - | - | new |  |
| `PointCloudBVHBuild/uniform/rebuild/double/host` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/host` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/host` | - | - | - | - | - | new |  |
| `Ray/mesh/firstHit/float/host` | 46.6 ms | 46.4 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.8 ms | 45.5 ms | -0.6% | [-0.9%, -0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.7 ms | 40.5 ms | -0.5% | [-0.7%, -0.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.9 ms | 78.9 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.9 ms | 65.2 ms | -1.1% | [-2.3%, +0.2%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.87 ms | 5.89 ms | +0.4% | [-0.5%, +1.3%] | - | ~ | tape/fused 1.35x |
| `Ray/csg-tape/firstHit/float/host` | 8.02 ms | 7.94 ms | -1.0% | [-2.5%, +0.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 53.8 ms | 52.9 ms | -1.8% | [-4.8%, +1.4%] | +0.0% | ~ | hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.7 ms | 50.9 ms | -1.5% | [-2.7%, -0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 46.3 ms | 44.9 ms | -3.0% | [-5.1%, -0.8%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.4 ms | 88.4 ms | -0.0% | [-1.2%, +1.2%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 75.8 ms | 74.7 ms | -1.4% | [-6.5%, +3.9%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.95 ms | 5.92 ms | -0.5% | [-1.7%, +0.8%] | - | ~ | tape/fused 1.41x |
| `Ray/csg-tape/firstHit/double/host` | 8.35 ms | 8.35 ms | -0.1% | [-1.0%, +0.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.3 ms | 18.7 ms | +2.3% | [+1.7%, +2.9%] | - | ~ | marched/traced 6.37x (was 6.61x) |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 121 ms | 119 ms | -1.4% | [-2.4%, -0.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 22.2 ms | +0.7% | [-2.3%, +3.8%] | +0.0% | ~ | marched/traced 6.97x (was 7.33x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 162 ms | 155 ms | -4.3% | [-9.0%, +0.9%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.3 ms | 35.2 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 18.7 ms | 20.0 ms | +7.3% | [+6.3%, +8.2%] | - | minor | marched/traced 6.35x (was 6.74x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 127 ms | +0.9% | [-0.1%, +1.9%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.5 ms | +0.5% | [-0.7%, +1.7%] | +0.0% | ~ | marched/traced 6.80x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 161 ms | 160 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.7 ms | 37.9 ms | +0.6% | [+0.2%, +1.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 48.2 ms | 47.3 ms | -1.9% | [-6.7%, +3.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.8 ms | 47.2 ms | +0.7% | [+0.2%, +1.2%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 10.2 ms | 10.2 ms | +0.2% | [-0.8%, +1.2%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 8.71 ms | 8.79 ms | +1.0% | [+0.8%, +1.1%] | +0.0% | ~ | hit/crossing 1.16x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 184 ms | 184 ms | -0.4% | [-1.4%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 162 ms | 161 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ | hit/crossing 1.14x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.8 ms | 69.7 ms | +1.4% | [-0.2%, +2.9%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.8 ms | 56.4 ms | -0.7% | [-1.1%, -0.2%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/mesh/firstHit/double/host` | 52.6 ms | 53.2 ms | +1.2% | [+1.0%, +1.3%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.7 ms | 53.1 ms | +0.8% | [-0.3%, +1.9%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 11.0 ms | 11.2 ms | +1.0% | [+0.7%, +1.3%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 9.41 ms | 9.40 ms | -0.0% | [-1.0%, +1.0%] | +0.0% | ~ | hit/crossing 1.19x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 212 ms | 207 ms | -2.4% | [-3.4%, -1.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 185 ms | 184 ms | -0.2% | [-1.2%, +0.9%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 78.3 ms | 77.9 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 65.7 ms | 66.3 ms | +0.9% | [-0.5%, +2.3%] | +0.0% | ~ | hit/crossing 1.18x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.2 ms | 11.2 ms | +0.2% | [-4.2%, +4.7%] | +0.0% | ~ | marched/traced 4.77x (was 4.61x) |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 53.6 ms | +3.6% | [+0.0%, +7.3%] | +0.0% | minor |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 7.96 ms | 8.00 ms | +0.6% | [-0.0%, +1.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.91 ms | 1.93 ms | +0.8% | [+0.7%, +1.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.86 ms | 6.94 ms | +1.2% | [+0.5%, +1.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.2 ms | 11.7 ms | -3.7% | [-7.6%, +0.5%] | +0.0% | ~ | marched/traced 5.76x (was 4.99x) |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.7 ms | 67.5 ms | +11.2% | [+0.8%, +21.7%] | +0.0% | **MAJOR** |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.62 ms | 8.92 ms | +3.5% | [-0.8%, +7.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.98 ms | 1.96 ms | -0.9% | [-3.9%, +2.2%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.15 ms | 7.23 ms | +1.1% | [+0.9%, +1.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 239 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 329 ms | 330 ms | +0.2% | [-0.6%, +0.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 238 ms | 240 ms | +1.0% | [-0.8%, +2.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 255 ms | 254 ms | -0.4% | [-0.8%, -0.1%] | +0.0% | ~ | marched/traced 1.45x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 373 ms | 370 ms | -0.8% | [-1.7%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 253 ms | 254 ms | +0.5% | [-0.5%, +1.6%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.7 ms | 23.5 ms | -0.6% | [-1.9%, +0.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.4 ms | 23.5 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 19.2 ms | +3.4% | [-0.9%, +7.7%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 30.4 ms | 30.1 ms | -1.1% | [-1.9%, -0.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 30.1 ms | 30.6 ms | +1.7% | [+0.1%, +3.4%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.6 ms | 20.4 ms | -0.9% | [-1.3%, -0.5%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.8 ms | 12.8 ms | +0.1% | [-1.3%, +1.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.54 ms | 4.55 ms | +0.2% | [-1.1%, +1.5%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.3 ms | 25.1 ms | -0.8% | [-3.2%, +1.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.8 ms | 40.3 ms | -1.2% | [-1.8%, -0.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.75 ms | 1.76 ms | +0.8% | [-0.7%, +2.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 133 µs | 134 µs | +0.4% | [-6.7%, +8.3%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 30.1 ms | +0.8% | [+0.0%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.66 ms | +0.0% | [-0.3%, +0.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.1 ms | 16.3 ms | +0.9% | [+0.6%, +1.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.68 ms | 4.66 ms | -0.6% | [-1.1%, -0.1%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.1 ms | +0.4% | [+0.3%, +0.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.62 ms | 4.56 ms | -1.3% | [-1.9%, -0.8%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.5 ms | 26.8 ms | +1.2% | [+0.4%, +2.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.3 ms | 43.0 ms | +1.8% | [+1.0%, +2.7%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.76 ms | 1.74 ms | -1.2% | [-2.0%, -0.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 276 µs | 287 µs | +3.7% | [-7.8%, +16.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.7 ms | 30.6 ms | +3.0% | [+1.7%, +4.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.80 ms | 7.80 ms | -0.0% | [-2.0%, +2.0%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.4 ms | 16.3 ms | -0.7% | [-1.3%, -0.0%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.79 ms | 7.72 ms | -0.8% | [-1.1%, -0.5%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/PointCloudBVHBuild/main.cpp, Performance/PointCloudHashGridBuild/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: ba83c75
base-fingerprint: 1759bc965c8f27541a01db9aadccd206bf503ecf
head: 40bd6b6
head-fingerprint: 60bfcfc855044f2fa7a4133c206533c55abdfd03
cases: 276
rounds: 2
reps: 3
date: 2026-11-02
-->

## Device comparison (CUDA): red

**Accepted:** The maintainer, for the PointCloudBVH sphere queries: "Yes, the slowdown is intentional due to consolidation of build algorithms".

`40bd6b6` against `ba83c75` (origin/main), 2026-11-02.

**VERDICT: MAJOR regression in 8 cases; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 8 major, 6 minor, 20 faster, 0 differ, 0 lost, 276 cases

- base: ba83c75 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 40bd6b6 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.61 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | device 22.7x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.42 ms | 3.41 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 18.9x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.04 ms | 1.04 ms | -0.6% | [-2.1%, +0.8%] | - | ~ | device 19.7x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.31 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 16.6x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.24 ms | 6.21 ms | -0.4% | [-1.0%, +0.3%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.8 ms | 12.0 ms | +1.6% | [+0.1%, +3.1%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.5 ms | 33.2 ms | -0.8% | [-1.0%, -0.5%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 56.9 ms | 57.0 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.6 ms | 20.7 ms | +0.5% | [-1.3%, +2.3%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 48.0 ms | 47.7 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.3 ms | 61.5 ms | +0.3% | [-0.4%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 106 ms | 111 ms | +4.6% | [+4.4%, +4.8%] | - | minor | device 0.6x faster than the host (was 0.6x) |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 387 µs | 388 µs | +0.2% | [-0.4%, +0.7%] | - | ~ | device 15.1x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 844 µs | 843 µs | -0.2% | [-0.4%, +0.1%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.51 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.65 ms | 4.68 ms | +0.8% | [+0.3%, +1.3%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 586 µs | 586 µs | +0.0% | [-0.9%, +1.0%] | - | ~ | device 12.8x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.37 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.76 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.00 ms | 7.01 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 902 µs | 903 µs | +0.2% | [+0.0%, +0.3%] | - | ~ | device 14.5x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.22 ms | 2.22 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.47 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.4% | [-0.9%, +0.0%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 319 µs | 320 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 11.4x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 5.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.0 ms | 59.0 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.55 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 519 ms | 518 ms | -0.2% | [-1.2%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.9 µs | +2.1% | [+0.6%, +3.7%] | - | ~ | device 23.2x faster than the host; tape/fused 4.43x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.1 µs | 50.1 µs | +0.1% | [-0.7%, +0.9%] | - | ~ | device 42.9x faster than the host; tape/fused 7.20x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.4 µs | 48.5 µs | +0.2% | [-0.2%, +0.6%] | - | ~ | device 9.7x faster than the host (was 9.1x); tape/fused 9.78x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.4 ms | 17.3 ms | -0.6% | [-0.8%, -0.4%] | - | ~ | device 6.8x faster than the host (was 8.1x); tape/fused 1.81x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.4 ms | 31.4 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.05x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.11 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 285 ms | 284 ms | -0.5% | [-1.3%, +0.4%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 288 ms | 286 ms | -0.7% | [-2.0%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.32 ms | 8.32 ms | +0.1% | [-0.6%, +0.7%] | +0.0% | ~ | device 10.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.21 ms | 8.26 ms | +0.7% | [-0.3%, +1.7%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.22 ms | 8.27 ms | +0.6% | [-0.1%, +1.2%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.28 ms | 8.18 ms | -1.3% | [-2.2%, -0.4%] | +0.0% | ~ | device 11.6x faster than the host (was 10.7x); default/this 1.02x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 708 µs | 709 µs | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | device 4.6x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.74 ms | 1.74 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.41x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.04 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.68x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 844 µs | 842 µs | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ | device 3.9x faster than the host; default/this 0.84x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 832 µs | 825 µs | -0.8% | [-0.9%, -0.8%] | +0.8% | ~ | device 10.7x faster than the host (was 11.5x) |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 951 µs | 959 µs | +0.9% | [+0.6%, +1.2%] | +0.8% | ~ | device 9.2x faster than the host (was 10.0x); default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 834 µs | 826 µs | -1.0% | [-1.5%, -0.5%] | +0.8% | ~ | device 10.9x faster than the host (was 11.4x); default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 803 µs | 803 µs | -0.0% | [-0.6%, +0.6%] | +0.8% | ~ | device 10.8x faster than the host (was 11.6x); default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 162 ms | 158 ms | -2.4% | [-5.9%, +1.3%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 162 ms | 163 ms | +0.7% | [-0.4%, +1.7%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.97x (was 1.00x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 165 ms | 156 ms | -5.2% | [-9.0%, -1.3%] | +0.0% | faster | device 0.6x faster than the host (was 0.5x); default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 163 ms | 163 ms | +0.1% | [-2.5%, +2.8%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.97x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 1.76 ms | 1.76 ms | +0.1% | [-0.7%, +0.8%] | +0.0% | ~ | device 2.1x faster than the host; default/this 0.91x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.94 ms | 1.93 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.83x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.69 ms | 1.68 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.31 ms | 7.27 ms | -0.5% | [-0.7%, -0.3%] | +0.9% | ~ | device 1.2x faster than the host (was 1.3x) |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.43 ms | 7.30 ms | -1.7% | [-2.0%, -1.4%] | +0.9% | ~ | device 1.2x faster than the host (was 1.3x); default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.29 ms | 7.26 ms | -0.5% | [-0.8%, -0.2%] | +0.9% | ~ | device 1.2x faster than the host (was 1.3x); default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.32 ms | 7.35 ms | +0.3% | [-0.1%, +0.8%] | +0.9% | ~ | device 1.2x faster than the host (was 1.3x); default/this 0.99x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.49 ms | 8.48 ms | -0.0% | [-0.8%, +0.7%] | - | ~ | device 11.1x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.8 ms | 16.7 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 7.0x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 165 ms | 164 ms | -0.9% | [-2.6%, +0.8%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 281 ms | 286 ms | +1.7% | [+0.9%, +2.4%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.4 ms | 24.7 ms | +1.4% | [+0.7%, +2.1%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 60.0 ms | 60.2 ms | +0.4% | [+0.4%, +0.5%] | - | ~ | device 2.4x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 158 ms | 158 ms | -0.3% | [-3.0%, +2.3%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 241 ms | 246 ms | +1.9% | [-0.1%, +3.9%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.90 ms | 2.93 ms | +1.3% | [+0.1%, +2.5%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 5.14 ms | 5.15 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 2.7x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 14.5 ms | 14.5 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 4.2x faster than the host (was 3.8x) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 124 ms | 122 ms | -1.6% | [-4.1%, +0.9%] | - | ~ | device 1.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 173 ms | 174 ms | +0.6% | [-1.3%, +2.5%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 377 ms | 377 ms | -0.1% | [-2.4%, +2.1%] | - | ~ | device 2.5x faster than the host (was 2.2x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.5 ms | 24.8 ms | +1.3% | [+0.1%, +2.5%] | - | ~ | device 2.2x faster than the host (was 2.0x); tape/fused 4.92x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 26.8 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 2.2x faster than the host; tape/fused 6.50x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.7 ms | 71.2 ms | -0.6% | [-2.3%, +1.1%] | - | ~ | device 4.1x faster than the host; tape/fused 5.29x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.3 ms | 23.4 ms | +4.7% | [-0.7%, +10.4%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.8 ms | 44.6 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 94.5 ms | 94.8 ms | +0.4% | [-2.2%, +3.0%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 548 ms | 560 ms | +2.2% | [+0.6%, +3.9%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 840 ms | 857 ms | +2.0% | [+1.4%, +2.6%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 3.19 s | 3.34 s | +4.6% | [+3.5%, +5.8%] | - | minor | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 200 ms | 195 ms | -2.9% | [-5.3%, -0.4%] | - | ~ | device 0.3x faster than the host (was 0.2x); tape/fused 2.88x (was 2.73x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 302 ms | 310 ms | +2.5% | [+1.7%, +3.4%] | - | ~ | device 0.2x faster than the host; tape/fused 2.77x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 608 ms | 580 ms | -4.6% | [-6.8%, -2.2%] | - | faster | device 0.5x faster than the host (was 0.5x); tape/fused 5.75x (was 5.24x) |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 766 µs | 761 µs | -0.6% | [-0.8%, -0.4%] | - | ~ | device 9.8x faster than the host (was 10.8x) |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.19 ms | 2.17 ms | -0.9% | [-1.2%, -0.7%] | - | ~ | device 4.7x faster than the host (was 5.8x) |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.16 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 7.8x faster than the host (was 8.4x) |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.44 ms | 6.21 ms | -3.5% | [-3.6%, -3.5%] | - | faster | device 4.0x faster than the host (was 4.4x) |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 4.12 ms | 3.93 ms | -4.5% | [-4.8%, -4.3%] | - | faster | device 3.9x faster than the host (was 4.2x); the points found differ (index checksum) |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.09 ms | 7.17 ms | +1.1% | [+0.8%, +1.4%] | - | ~ | device 1.1x faster than the host (was 1.2x) |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.82 ms | 7.91 ms | +1.1% | [+0.8%, +1.3%] | - | ~ | device 1.4x faster than the host (was 1.7x) |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.4 ms | +1.7% | [+1.4%, +2.1%] | - | ~ | device 0.7x faster than the host (was 0.8x) |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.5 ms | 21.0 ms | +2.5% | [+2.3%, +2.7%] | - | ~ | device 1.3x faster than the host (was 1.5x) |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 12.0 ms | +1.3% | [+0.9%, +1.7%] | - | ~ | device 1.3x faster than the host (was 1.5x) |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.16 ms | 1.45 ms | -33.0% | [-33.1%, -32.9%] | - | faster | device 6.5x faster than the host (was 4.9x) |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.95 ms | 3.87 ms | -35.0% | [-35.1%, -35.0%] | - | faster | device 3.4x faster than the host (was 2.7x) |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 832 µs | 523 µs | -37.1% | [-37.3%, -37.0%] | - | faster | device 16.1x faster than the host (was 10.5x) |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 9.04 ms | 4.47 ms | -50.5% | [-51.3%, -49.7%] | - | faster | device 6.8x faster than the host (was 3.9x) |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 6.42 ms | 4.24 ms | -34.0% | [-34.4%, -33.6%] | - | faster | device 5.3x faster than the host (was 4.1x) |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.09 ms | 8.17 ms | +1.0% | [+0.3%, +1.8%] | - | ~ | device 1.2x faster than the host (was 1.3x) |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 16.0 ms | 15.7 ms | -1.6% | [-2.2%, -1.0%] | - | ~ | device 0.8x faster than the host (was 1.0x) |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.00 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.1x faster than the host (was 2.2x) |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 20.3 ms | 17.5 ms | -13.6% | [-14.0%, -13.2%] | - | faster | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.2 ms | 14.8 ms | -2.7% | [-5.1%, -0.2%] | - | ~ | device 1.5x faster than the host (was 1.7x) |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.48 ms | 1.82 ms | +22.9% | [+21.9%, +23.9%] | - | **MAJOR** | device 8.9x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.68 ms | 5.54 ms | +18.5% | [+18.4%, +18.6%] | - | **MAJOR** | device 4.3x faster than the host (was 4.0x) |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 921 µs | -33.3% | [-33.8%, -32.8%] | - | faster | device 9.1x faster than the host (was 5.3x) |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.40 ms | 3.72 ms | +9.3% | [+9.0%, +9.6%] | - | minor | device 5.3x faster than the host (was 5.5x) |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.59 ms | 5.47 ms | +19.1% | [+19.0%, +19.3%] | - | **MAJOR** | device 4.1x faster than the host; the points found differ (index checksum) |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.5 ms | 16.8 ms | +23.7% | [+22.8%, +24.6%] | - | **MAJOR** | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 17.3 ms | 21.4 ms | +24.2% | [+23.8%, +24.7%] | - | **MAJOR** | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 7.52 ms | 8.96 ms | +19.1% | [+18.9%, +19.2%] | - | **MAJOR** | device 1.0x faster than the host (was 1.0x) |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 11.3 ms | 14.6 ms | +29.3% | [+29.2%, +29.5%] | - | **MAJOR** | device 1.4x faster than the host (was 1.7x) |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 15.9 ms | 20.4 ms | +28.4% | [+27.3%, +29.5%] | - | **MAJOR** | device 1.1x faster than the host (was 1.2x) |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 4.86 ms | 2.75 ms | -43.4% | [-44.1%, -42.7%] | - | faster | device 4.6x faster than the host (was 3.6x) |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 16.0 ms | 6.74 ms | -58.0% | [-58.1%, -57.9%] | - | faster | device 2.3x faster than the host (was 2.1x) |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.62 ms | 1.38 ms | -14.9% | [-15.2%, -14.6%] | - | faster | device 6.6x faster than the host (was 7.3x) |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.61 ms | 2.71 ms | -51.7% | [-51.8%, -51.7%] | - | faster | device 7.9x faster than the host (was 4.9x) |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 13.2 ms | 6.33 ms | -52.1% | [-53.0%, -51.3%] | - | faster | device 3.1x faster than the host (was 1.9x) |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.8 ms | 33.9 ms | +0.3% | [-0.6%, +1.2%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.6 ms | 68.4 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 0.2x faster than the host (was 0.6x) |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.4 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 0.5x faster than the host (was 0.7x) |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.9 ms | 20.8 ms | -0.6% | [-1.1%, -0.0%] | - | ~ | device 1.0x faster than the host (was 1.3x) |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.8 ms | 37.9 ms | -2.4% | [-3.4%, -1.3%] | - | ~ | device 0.5x faster than the host (was 0.7x) |
| `PointCloudBVHBuild/uniform/build/float/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudBVHBuild/uniform/rebuild/float/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudBVHBuild/uniform/build/double/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudBVHBuild/uniform/rebuild/double/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/build/float/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/rebuild/float/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/build/double/hostdevice` | - | - | - | - | - | new |  |
| `PointCloudHashGridBuild/uniform/rebuild/double/hostdevice` | - | - | - | - | - | new |  |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 679 µs | 671 µs | -1.2% | [-1.6%, -0.9%] | - | ~ | device 16.8x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 14.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 639 µs | 644 µs | +0.8% | [+0.2%, +1.5%] | - | ~ | device 17.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.22 ms | -0.5% | [-1.1%, +0.2%] | - | ~ | device 15.3x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 612 µs | 606 µs | -1.0% | [-1.7%, -0.3%] | - | ~ | device 16.1x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.11 ms | 1.11 ms | -0.1% | [-1.0%, +0.9%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.19 ms | +0.6% | [+0.2%, +0.9%] | - | ~ | device 12.0x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.19 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 595 µs | 632 µs | +6.1% | [+5.7%, +6.5%] | - | minor | device 2.4x faster than the host (was 2.5x); tape/fused 1.12x (was 1.19x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 697 µs | 664 µs | -4.8% | [-9.8%, +0.8%] | - | ~ | device 2.8x faster than the host (was 2.6x); tape/fused 1.26x (was 1.20x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 709 µs | 708 µs | -0.3% | [-0.8%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 839 µs | 834 µs | -0.7% | [-1.0%, -0.4%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.3% | [-1.1%, +0.5%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.3 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 16.9 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.1% | [-0.4%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 14.9 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.6% | [+0.4%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.4 ms | 16.5 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.02x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.6 ms | 24.4 ms | -0.7% | [-1.2%, -0.2%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.5% | [-1.6%, +0.6%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 22.9 ms | -0.7% | [-1.1%, -0.2%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 1.1x faster than the host; tape/fused 1.09x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.59 ms | 2.61 ms | +0.7% | [+0.3%, +1.2%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.71 ms | -0.4% | [-0.8%, -0.0%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.59 ms | 2.59 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.9x faster than the host; marched/traced 3.49x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.04 ms | 3.04 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 2.7x faster than the host; marched/traced 4.57x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.04 ms | 9.03 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.17 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 3.9x faster than the host; marched/traced 16.59x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.42 ms | 5.41 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 2.8x faster than the host; marched/traced 13.26x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 36.0 ms | 36.0 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 71.6 ms | 71.8 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 1.6x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.25 ms | 5.24 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.82 ms | 8.79 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.23 ms | 6.20 ms | -0.5% | [-0.8%, -0.1%] | - | ~ | device 1.3x faster than the host; marched/traced 26.42x (was 25.18x) |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 0.8x faster than the host; marched/traced 24.12x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 157 ms | 164 ms | +4.4% | [+2.9%, +5.9%] | - | minor | device 0.3x faster than the host (was 0.3x) |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 258 ms | 255 ms | -1.0% | [-2.4%, +0.5%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.84 ms | 4.85 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 22.78x (was 23.52x) |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 1.3x faster than the host; marched/traced 18.03x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 114 ms | 110 ms | -3.0% | [-3.2%, -2.9%] | - | faster | device 0.7x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 218 ms | 219 ms | +0.4% | [-1.5%, +2.3%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.4 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.7 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 673 µs | 676 µs | +0.5% | [+0.1%, +0.8%] | - | ~ | device 17.0x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 15.4x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 673 µs | 676 µs | +0.3% | [-0.5%, +1.2%] | - | ~ | device 17.1x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 708 µs | 710 µs | +0.2% | [-0.2%, +0.6%] | - | ~ | device 4.7x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 837 µs | 840 µs | +0.4% | [-0.1%, +1.0%] | - | ~ | device 4.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 614 µs | 614 µs | -0.0% | [-0.4%, +0.3%] | - | ~ | device 4.4x faster than the host; hit/crossing 1.16x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 637 µs | 637 µs | +0.0% | [-0.6%, +0.6%] | - | ~ | device 4.8x faster than the host; hit/crossing 1.32x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.70 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 10.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 7.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.19 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 10.2x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.07 ms | 9.07 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 268 ms | 265 ms | -1.0% | [-5.0%, +3.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 216 ms | 216 ms | +0.1% | [-3.3%, +3.5%] | - | ~ | device 0.9x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 55.3 ms | 55.5 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 2.6x faster than the host; hit/crossing 4.78x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 70.9 ms | 70.7 ms | -0.3% | [-1.5%, +1.0%] | - | ~ | device 2.2x faster than the host; hit/crossing 3.05x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 16.9 ms | 17.1 ms | +1.2% | [+0.3%, +2.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.1% | [-0.8%, +0.5%] | - | ~ | device 0.7x faster than the host (was 0.8x); hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.73 ms | 2.71 ms | -0.6% | [-0.7%, -0.5%] | - | ~ | device 1.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.29 ms | 1.29 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.24x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.34 ms | 1.35 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 2.6x faster than the host; hit/crossing 2.01x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.6 ms | 64.7 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 123 ms | 121 ms | -2.1% | [-4.0%, -0.2%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.8 ms | 58.7 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 112 ms | 111 ms | -0.2% | [-2.5%, +2.1%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.08x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 485 ms | 464 ms | -4.3% | [-10.5%, +2.4%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 707 ms | 727 ms | +2.8% | [+1.2%, +4.4%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 400 ms | 379 ms | -5.2% | [-11.7%, +1.8%] | - | ~ | device 0.4x faster than the host (was 0.4x); hit/crossing 1.23x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 537 ms | 534 ms | -0.6% | [-4.0%, +3.0%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.36x (was 1.32x) |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.56 ms | 3.62 ms | +1.7% | [-0.4%, +3.8%] | - | ~ | device 1.2x faster than the host; marched/traced 5.41x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.78 ms | 4.79 ms | +0.3% | [-0.2%, +0.9%] | - | ~ | device 0.9x faster than the host; marched/traced 8.37x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 19.8 ms | 19.6 ms | -1.1% | [-1.5%, -0.7%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 40.2 ms | 40.1 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.23 ms | 1.24 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.83 ms | 1.83 ms | -0.2% | [-1.4%, +1.1%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 99.3 µs | 93.2 µs | -6.1% | [-8.7%, -3.4%] | - | faster | device 15.3x faster than the host (was 14.6x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 150 µs | 151 µs | +0.3% | [-1.0%, +1.7%] | - | ~ | device 11.0x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 798 µs | 799 µs | +0.1% | [-0.1%, +0.4%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 933 µs | 933 µs | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.1 ms | 34.7 ms | -1.1% | [-7.1%, +5.5%] | - | ~ | device 0.1x faster than the host; marched/traced 3.55x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.2 ms | 37.0 ms | +2.0% | [+1.5%, +2.5%] | - | ~ | device 0.1x faster than the host; marched/traced 6.28x (was 6.51x) |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 125 ms | 123 ms | -1.4% | [-3.2%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 236 ms | 232 ms | -1.7% | [-2.9%, -0.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.6 ms | 16.7 ms | +0.6% | [-2.3%, +3.5%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 21.7 ms | -0.7% | [-1.4%, -0.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.10 ms | 1.05 ms | -4.8% | [-7.5%, -2.0%] | - | faster | device 1.4x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.39 ms | 2.36 ms | -1.1% | [-2.4%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 5.96 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.73 ms | 6.70 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.1 ms | 23.1 ms | +0.1% | [-0.4%, +0.7%] | - | ~ | device 2.2x faster than the host; marched/traced 4.28x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.6 ms | 28.5 ms | -0.5% | [-1.2%, +0.3%] | - | ~ | device 1.9x faster than the host; marched/traced 5.42x (was 5.59x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 100.0 ms | 99.0 ms | -1.0% | [-2.8%, +0.9%] | - | ~ | device 2.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 160 ms | 154 ms | -3.6% | [-4.1%, -3.1%] | - | faster | device 1.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.8 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.5 ms | +1.4% | [+0.7%, +2.0%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 311 ms | 301 ms | -3.4% | [-6.6%, +0.1%] | - | ~ | device 0.2x faster than the host; marched/traced 1.88x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 356 ms | 361 ms | +1.3% | [+0.1%, +2.6%] | - | ~ | device 0.2x faster than the host; marched/traced 2.78x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 574 ms | 567 ms | -1.3% | [-3.1%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 993 ms | 1.00 s | +1.0% | [-0.2%, +2.2%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 183 ms | 196 ms | +6.7% | [+4.3%, +9.2%] | - | minor | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 234 ms | 243 ms | +4.1% | [-0.2%, +8.5%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `Render/csg/frame/float/hostdevice` | 4.58 ms | 4.59 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.59 ms | 4.59 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.89 ms | +0.3% | [-0.0%, +0.7%] | - | ~ | device 8.8x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.97 ms | 7.96 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.93 ms | 7.94 ms | +0.1% | [+0.0%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.4 ms | 23.3 ms | -0.8% | [-1.2%, -0.4%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 217 µs | +0.1% | [-0.1%, +0.2%] | - | ~ | device 11.6x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.3% | [-0.6%, +0.1%] | - | ~ | device 23.7x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 361 µs | 361 µs | +0.0% | [-0.1%, +0.2%] | - | ~ | device 12.8x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.98 ms | 1.98 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 7.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.4 µs | 72.3 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 7.0x faster than the host (was 7.4x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.5 µs | 47.4 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 5.0x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 653 µs | 652 µs | -0.1% | [-0.3%, +0.0%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 11.8x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 474 µs | 474 µs | +0.2% | [+0.0%, +0.3%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.1 µs | +0.1% | [-0.3%, +0.5%] | - | ~ | device 27.0x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 856 µs | 853 µs | -0.3% | [-0.6%, -0.1%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.6x faster than the host (was 1.5x) |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.72 ms | 6.71 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 190 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.9x faster than the host (was 3.1x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.0x faster than the host (was 1.8x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |


The probes differ between the sides (each side ran its own): Performance/PointCloudBVHBuild/main.cpp, Performance/PointCloudHashGridBuild/main.cpp

<!-- END receipt cuda -->
