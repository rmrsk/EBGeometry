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
base: e649f31
base-fingerprint: f6e7bf66f73492cc9a6b59de18c72ad4f1b00bda
head: a138ef6
head-fingerprint: 954ae65e6912091a01c701738e4204fd83116991
cases: 152
rounds: 2
reps: 3
date: 2026-10-30
-->

## Host comparison: red

**Accepted:** Accept as noise.

`a138ef6` against `e649f31` (origin/dev), 2026-10-30.

**VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 1 major, 1 minor, 2 faster, 0 differ, 0 lost, 152 cases

- base: e649f31 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: a138ef6 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 328 ms | 331 ms | +0.7% | [+0.2%, +1.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.0% | [+0.0%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 513 ms | 512 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 344 ms | 346 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 185 ms | -0.4% | [-0.4%, -0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 561 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.2 ms | 55.2 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.0 ms | 51.3 ms | +0.5% | [+0.3%, +0.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.1 ms | 64.2 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.5 ms | 54.8 ms | +0.6% | [-0.3%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.0 ms | 60.4 ms | +0.5% | [-0.7%, +1.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 73.2 ms | 73.3 ms | +0.1% | [-1.8%, +2.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.0 ms | 74.3 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.5 ms | 77.5 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.7 ms | 66.4 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.7 ms | 99.2 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.8 ms | 83.2 ms | +0.5% | [+0.0%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 109 ms | +0.8% | [+0.4%, +1.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.3 ms | -0.8% | [-1.6%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 48.4 ms | 45.7 ms | -5.7% | [-13.0%, +2.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.0 ms | 73.0 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.8 ms | 20.6 ms | -0.8% | [-1.7%, +0.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.9 ms | 46.0 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.9 ms | 83.4 ms | +0.6% | [-1.8%, +3.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.43 ms | 4.44 ms | +0.2% | [-2.6%, +3.1%] | - | ~ | tape/fused 2.80x |
| `Fused/smooth/signedDistance/float/host` | 12.2 ms | 12.4 ms | +1.8% | [-0.0%, +3.6%] | - | ~ | tape/fused 1.93x |
| `Fused/chain16/signedDistance/float/host` | 960 µs | 951 µs | -0.9% | [-2.0%, +0.1%] | - | ~ | tape/fused 16.68x |
| `Fused/drilled-mesh/signedDistance/float/host` | 45.9 ms | 46.2 ms | +0.5% | [+0.3%, +0.8%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.1 ms | 46.4 ms | +0.5% | [+0.4%, +0.7%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.50 ms | 4.48 ms | -0.5% | [-2.0%, +1.0%] | - | ~ | tape/fused 2.89x |
| `Fused/smooth/signedDistance/double/host` | 12.6 ms | 13.1 ms | +4.0% | [+3.4%, +4.5%] | - | minor | tape/fused 1.92x (was 1.99x) |
| `Fused/chain16/signedDistance/double/host` | 3.86 ms | 3.84 ms | -0.6% | [-2.1%, +1.0%] | - | ~ | tape/fused 4.17x |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.3 ms | 53.0 ms | -0.7% | [-1.7%, +0.4%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.5 ms | 53.2 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.8 ms | 81.7 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 230 ms | 229 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 265 ms | 265 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.5 ms | 70.2 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 214 ms | 213 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.2 ms | 96.3 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.8 ms | 29.7 ms | -0.1% | [-1.0%, +0.8%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 759 ms | 765 ms | +0.8% | [+0.2%, +1.4%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.9 ms | 43.5 ms | -3.1% | [-3.9%, -2.2%] | - | faster |  |
| `MeshSDF/query-random/double/host` | 844 ms | 846 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 68.6 ms | 67.8 ms | -1.1% | [-2.0%, -0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 147 ms | 146 ms | -0.1% | [-0.5%, +0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 370 ms | 371 ms | +0.3% | [+0.1%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.3 ms | 78.6 ms | +0.5% | [-0.5%, +1.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.64 ms | 6.64 ms | -0.0% | [-0.7%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 86.7 ms | 80.6 ms | -7.1% | [-13.4%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 160 ms | 159 ms | -0.6% | [-1.7%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 403 ms | 403 ms | -0.0% | [-1.9%, +1.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 89.1 ms | 88.2 ms | -1.0% | [-4.2%, +2.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.27 ms | 7.20 ms | -1.0% | [-3.3%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.5 ms | 10.7 ms | -6.6% | [-15.1%, +1.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 124 ms | 124 ms | +0.5% | [-2.2%, +3.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 514 ms | 514 ms | -0.1% | [-0.3%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 152 ms | +0.2% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 842 ms | 841 ms | -0.1% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.1 ms | 14.9 ms | -1.3% | [-3.9%, +1.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 126 ms | 127 ms | +0.3% | [+0.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 543 ms | 545 ms | +0.3% | [+0.1%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 156 ms | 154 ms | -0.9% | [-1.2%, -0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 895 ms | 888 ms | -0.9% | [-1.5%, -0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.7 ms | 67.3 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 109 ms | 109 ms | +0.8% | [+0.6%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 275 ms | 271 ms | -1.2% | [-3.4%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 111 ms | 112 ms | +0.6% | [+0.6%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 106 ms | 107 ms | +0.9% | [+0.4%, +1.5%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 80.6 ms | 80.8 ms | +0.3% | [-0.8%, +1.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 120 ms | 120 ms | +0.2% | [-0.9%, +1.3%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 296 ms | 296 ms | -0.0% | [-0.6%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 157 ms | 128 ms | -18.6% | [-31.5%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 121 ms | 127 ms | +4.8% | [-0.6%, +10.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.82 ms | 9.07 ms | +2.8% | [+2.4%, +3.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 141 ms | 145 ms | +2.9% | [-0.4%, +6.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 396 ms | 393 ms | -0.6% | [-4.1%, +3.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 251 ms | 256 ms | +1.8% | [+1.3%, +2.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 334 ms | 333 ms | -0.3% | [-1.0%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.6 ms | 12.6 ms | -0.4% | [-2.0%, +1.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 147 ms | 144 ms | -2.2% | [-4.5%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 418 ms | 421 ms | +0.7% | [+0.1%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 288 ms | 284 ms | -1.3% | [-2.6%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 345 ms | 343 ms | -0.5% | [-0.8%, -0.2%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.6 ms | 46.5 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.5 ms | 45.7 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.3 ms | 40.2 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 77.8 ms | 77.5 ms | -0.4% | [-1.3%, +0.4%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/float/host` | 65.3 ms | 65.2 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.88 ms | 5.90 ms | +0.4% | [-0.3%, +1.0%] | - | ~ | tape/fused 1.33x |
| `Ray/csg-tape/firstHit/float/host` | 7.74 ms | 7.82 ms | +1.0% | [+0.1%, +1.9%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 51.5 ms | 51.3 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ | hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.3 ms | 51.2 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.6 ms | 44.9 ms | +0.7% | [+0.0%, +1.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 86.8 ms | 86.1 ms | -0.8% | [-1.6%, +0.0%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 72.8 ms | 73.0 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.95 ms | 6.00 ms | +0.8% | [-0.2%, +1.7%] | - | ~ | tape/fused 1.37x (was 1.42x) |
| `Ray/csg-tape/firstHit/double/host` | 8.48 ms | 8.21 ms | -3.1% | [-4.7%, -1.5%] | +0.0% | faster |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.7 ms | 18.7 ms | -0.1% | [-3.5%, +3.5%] | - | ~ | marched/traced 6.18x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 116 ms | 116 ms | -0.1% | [-0.2%, -0.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 22.0 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ | marched/traced 7.29x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 160 ms | 161 ms | +0.4% | [-0.2%, +1.1%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.5 ms | 35.8 ms | +0.8% | [-0.7%, +2.4%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.4 ms | 19.1 ms | -1.2% | [-2.4%, -0.0%] | - | ~ | marched/traced 6.51x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 125 ms | 125 ms | -0.5% | [-1.0%, -0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.3 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ | marched/traced 7.12x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 166 ms | 166 ms | -0.1% | [-0.9%, +0.7%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.7 ms | 37.6 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ | marched/traced 4.70x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.8 ms | 52.0 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.09 ms | 8.02 ms | -0.9% | [-1.4%, -0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.84 ms | 1.85 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.87 ms | 6.89 ms | +0.3% | [-0.7%, +1.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.6 ms | 11.6 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ | marched/traced 5.20x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.3 ms | 60.5 ms | -1.4% | [-2.2%, -0.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.69 ms | 8.82 ms | +1.5% | [-1.5%, +4.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.06 ms | 2.38 ms | +15.9% | [+2.0%, +29.8%] | +0.0% | **MAJOR** |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.16 ms | 7.15 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 241 ms | 240 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ | marched/traced 1.40x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 336 ms | 337 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 239 ms | 239 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 252 ms | 253 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ | marched/traced 1.51x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 382 ms | 383 ms | +0.4% | [-0.8%, +1.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 251 ms | 251 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.8 ms | 23.7 ms | -0.4% | [-1.4%, +0.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.5 ms | 23.5 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.1 ms | 19.2 ms | +0.5% | [-2.7%, +4.0%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 28.9 ms | 30.5 ms | +5.6% | [-0.3%, +11.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 28.7 ms | 29.0 ms | +0.8% | [+0.6%, +1.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.8 ms | 21.3 ms | -2.5% | [-4.6%, -0.3%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.4 ms | 12.5 ms | +0.5% | [+0.0%, +0.9%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.52 ms | 4.55 ms | +0.7% | [-0.4%, +1.9%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 23.9 ms | 24.0 ms | +0.4% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.2 ms | 40.5 ms | +0.9% | [+0.2%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.73 ms | 1.74 ms | +0.5% | [-0.9%, +1.8%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 132 µs | 134 µs | +1.7% | [-8.7%, +13.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.8 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.62 ms | -0.7% | [-1.3%, -0.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 15.8 ms | 15.9 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.60 ms | 4.62 ms | +0.4% | [-0.1%, +1.0%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 12.9 ms | 12.9 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.51 ms | 4.53 ms | +0.4% | [-0.1%, +0.9%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 25.0 ms | 25.0 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.1 ms | 42.1 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.73 ms | 1.75 ms | +1.1% | [-1.1%, +3.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 276 µs | 272 µs | -1.6% | [-9.2%, +6.7%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.1 ms | 29.8 ms | -0.9% | [-1.7%, -0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.72 ms | 7.75 ms | +0.3% | [-0.0%, +0.7%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.0 ms | 16.0 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.75 ms | 7.72 ms | -0.4% | [-0.4%, -0.3%] | - | ~ |  |

<!-- END receipt host -->
