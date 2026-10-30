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
base: 8918460
base-fingerprint: 4473d6178a01cc7c1bc00dbbcf1c64262b94c98f
head: 52b304b
head-fingerprint: f6e7bf66f73492cc9a6b59de18c72ad4f1b00bda
cases: 152
rounds: 2
reps: 3
date: 2026-10-30
-->

## Host comparison: yellow

**Accepted:** Accept as noise.

`52b304b` against `8918460` (origin/dev), 2026-10-30.

**VERDICT: 3 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 3 minor, 2 faster, 0 differ, 0 lost, 152 cases

- base: 8918460 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 52b304b (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 337 ms | 342 ms | +1.4% | [+0.2%, +2.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 183 ms | 182 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 525 ms | 534 ms | +1.6% | [-1.5%, +4.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 364 ms | 358 ms | -1.8% | [-4.2%, +0.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 190 ms | 190 ms | +0.4% | [-1.5%, +2.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 579 ms | 584 ms | +1.0% | [-2.3%, +4.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.7 ms | 57.3 ms | +1.1% | [-3.2%, +5.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 53.9 ms | 52.7 ms | -2.3% | [-8.7%, +4.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.5 ms | 65.7 ms | +0.3% | [-2.9%, +3.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 57.6 ms | 56.4 ms | -2.1% | [-10.1%, +6.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 62.0 ms | 61.9 ms | -0.2% | [-3.1%, +2.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 75.2 ms | 77.1 ms | +2.6% | [-5.6%, +11.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 75.6 ms | 75.6 ms | -0.1% | [-4.3%, +4.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 81.4 ms | 84.7 ms | +4.1% | [-8.4%, +17.8%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 68.1 ms | 67.6 ms | -0.6% | [-3.6%, +2.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 107 ms | 103 ms | -4.5% | [-13.2%, +5.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 84.5 ms | 84.6 ms | +0.1% | [-2.6%, +3.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 118 ms | 119 ms | +1.2% | [-13.4%, +17.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.7 ms | 19.6 ms | -0.3% | [-3.0%, +2.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 46.4 ms | 46.6 ms | +0.3% | [-4.2%, +5.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 75.4 ms | 75.1 ms | -0.3% | [-5.5%, +5.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 21.1 ms | 21.3 ms | +0.6% | [-2.6%, +3.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 47.0 ms | 47.5 ms | +1.1% | [-3.3%, +5.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 88.2 ms | 90.4 ms | +2.5% | [-13.6%, +21.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.54 ms | 4.53 ms | -0.2% | [-2.4%, +2.1%] | - | ~ | tape/fused 2.79x |
| `Fused/smooth/signedDistance/float/host` | 12.4 ms | 12.4 ms | -0.1% | [-2.2%, +1.9%] | - | ~ | tape/fused 1.96x |
| `Fused/chain16/signedDistance/float/host` | 968 µs | 974 µs | +0.6% | [-0.9%, +2.1%] | - | ~ | tape/fused 16.70x |
| `Fused/drilled-mesh/signedDistance/float/host` | 49.5 ms | 49.2 ms | -0.6% | [-10.6%, +10.8%] | +0.0% | ~ | tape/fused 0.98x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 48.0 ms | 48.3 ms | +0.7% | [-6.0%, +8.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.53 ms | 4.56 ms | +0.6% | [-1.3%, +2.4%] | - | ~ | tape/fused 2.83x |
| `Fused/smooth/signedDistance/double/host` | 13.0 ms | 16.5 ms | +27.3% | [-2.8%, +58.0%] | - | ~ | tape/fused 1.54x (was 1.96x) |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.89 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | tape/fused 4.16x |
| `Fused/drilled-mesh/signedDistance/double/host` | 57.8 ms | 57.7 ms | -0.2% | [-12.3%, +13.3%] | +0.0% | ~ | tape/fused 1.07x (was 1.03x) |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 59.8 ms | 61.6 ms | +2.9% | [-3.1%, +9.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 85.7 ms | 107 ms | +24.7% | [-4.2%, +54.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 241 ms | 236 ms | -2.1% | [-3.9%, -0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 116 ms | 117 ms | +0.7% | [-0.7%, +2.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 295 ms | 288 ms | -2.6% | [-4.7%, -0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 72.2 ms | 72.1 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 223 ms | 227 ms | +1.7% | [+0.8%, +2.5%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 99.1 ms | 98.5 ms | -0.6% | [-3.2%, +2.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 32.2 ms | 32.7 ms | +1.6% | [-6.3%, +10.0%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 802 ms | 806 ms | +0.5% | [-5.4%, +6.6%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 120 ms | 121 ms | +0.9% | [+0.5%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 48.0 ms | 48.5 ms | +1.0% | [-5.5%, +8.2%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 951 ms | 939 ms | -1.3% | [-9.7%, +8.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 70.0 ms | 70.8 ms | +1.2% | [-0.2%, +2.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 158 ms | 156 ms | -0.8% | [-2.4%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 389 ms | 392 ms | +0.6% | [-3.1%, +4.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 83.7 ms | 83.3 ms | -0.4% | [-6.1%, +5.8%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.87 ms | 6.85 ms | -0.4% | [-1.5%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 101 ms | 102 ms | +0.3% | [-30.3%, +44.7%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 208 ms | 255 ms | +22.8% | [-30.9%, +102.6%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 491 ms | 453 ms | -7.6% | [-28.3%, +21.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 96.2 ms | 94.6 ms | -1.7% | [-12.9%, +11.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.29 ms | 7.36 ms | +1.0% | [-0.4%, +2.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.7 ms | 11.4 ms | -3.1% | [-10.5%, +4.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 126 ms | 127 ms | +0.4% | [-4.3%, +5.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 530 ms | 525 ms | -0.9% | [-2.9%, +1.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 159 ms | 161 ms | +1.1% | [-1.8%, +4.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 900 ms | 937 ms | +4.0% | [+2.8%, +5.3%] | - | minor |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.3 ms | 15.7 ms | +2.6% | [-1.3%, +6.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 132 ms | 132 ms | +0.7% | [-4.2%, +5.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 555 ms | 563 ms | +1.5% | [-0.3%, +3.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 160 ms | 158 ms | -1.3% | [-3.5%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 934 ms | 925 ms | -1.0% | [-4.5%, +2.7%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 69.3 ms | 69.1 ms | -0.3% | [-2.2%, +1.7%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 114 ms | 115 ms | +0.7% | [-3.0%, +4.6%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 280 ms | 281 ms | +0.2% | [-2.1%, +2.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 114 ms | 114 ms | +0.7% | [-0.6%, +2.0%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 111 ms | 109 ms | -2.0% | [-6.5%, +2.9%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 84.9 ms | 84.9 ms | +0.0% | [-7.5%, +8.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 133 ms | 133 ms | -0.5% | [-12.5%, +13.3%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 304 ms | 304 ms | -0.2% | [-2.6%, +2.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 147 ms | 143 ms | -2.7% | [-19.2%, +17.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 136 ms | 137 ms | +0.7% | [-14.3%, +18.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 9.60 ms | 9.40 ms | -2.1% | [-9.4%, +6.0%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 150 ms | 149 ms | -0.6% | [-9.7%, +9.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 419 ms | 408 ms | -2.6% | [-7.0%, +1.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 266 ms | 262 ms | -1.4% | [-2.7%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 350 ms | 345 ms | -1.6% | [-1.7%, -1.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.2 ms | 13.1 ms | -1.2% | [-6.2%, +4.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 151 ms | 156 ms | +3.8% | [-2.0%, +9.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 453 ms | 439 ms | -3.1% | [-5.4%, -0.6%] | - | faster |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 325 ms | 324 ms | -0.3% | [-1.3%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 368 ms | 372 ms | +1.0% | [-2.2%, +4.2%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 51.3 ms | 50.7 ms | -1.1% | [-5.6%, +3.9%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 50.1 ms | 49.8 ms | -0.6% | [-11.4%, +11.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 42.8 ms | 42.5 ms | -0.7% | [-7.3%, +6.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 89.3 ms | 89.1 ms | -0.2% | [-10.1%, +11.4%] | +0.0% | ~ | triangles/polygons 0.57x |
| `Ray/mesh-polygons/intersects/float/host` | 77.9 ms | 72.5 ms | -7.0% | [-10.9%, -3.0%] | +0.0% | faster |  |
| `Ray/csg-fused/firstHit/float/host` | 6.04 ms | 6.02 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | tape/fused 1.33x |
| `Ray/csg-tape/firstHit/float/host` | 7.94 ms | 8.04 ms | +1.2% | [+0.4%, +2.1%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 54.8 ms | 54.0 ms | -1.4% | [-3.8%, +1.1%] | +0.0% | ~ | hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled/double/host` | 54.2 ms | 54.0 ms | -0.4% | [-1.0%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 46.3 ms | 47.5 ms | +2.5% | [-0.4%, +5.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 95.9 ms | 104 ms | +8.6% | [+5.3%, +12.1%] | +0.0% | minor | triangles/polygons 0.52x (was 0.57x) |
| `Ray/mesh-polygons/intersects/double/host` | 81.8 ms | 83.7 ms | +2.3% | [-11.4%, +17.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 6.06 ms | 6.05 ms | -0.2% | [-2.1%, +1.7%] | - | ~ | tape/fused 1.37x |
| `Ray/csg-tape/firstHit/double/host` | 8.24 ms | 8.29 ms | +0.6% | [-0.3%, +1.4%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.5 ms | 18.7 ms | +1.1% | [+0.2%, +2.0%] | - | ~ | marched/traced 6.41x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 120 ms | -0.3% | [-0.9%, +0.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.8 ms | 22.8 ms | +0.1% | [+0.0%, +0.1%] | +0.0% | ~ | marched/traced 7.20x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 166 ms | 164 ms | -0.7% | [-1.3%, -0.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 36.6 ms | 36.9 ms | +0.7% | [+0.1%, +1.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.7 ms | 20.0 ms | +1.7% | [+0.3%, +3.1%] | - | ~ | marched/traced 6.46x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 129 ms | 130 ms | +0.3% | [-0.7%, +1.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 24.3 ms | 24.2 ms | -0.2% | [-1.9%, +1.5%] | +0.0% | ~ | marched/traced 6.92x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 168 ms | 168 ms | -0.0% | [-1.2%, +1.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.8 ms | 38.8 ms | +2.6% | [+1.8%, +3.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.5 ms | 11.3 ms | -1.9% | [-4.4%, +0.8%] | +0.0% | ~ | marched/traced 4.84x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 55.3 ms | 54.6 ms | -1.3% | [-9.4%, +7.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.24 ms | 8.11 ms | -1.6% | [-3.4%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.92 ms | 1.92 ms | -0.0% | [-1.0%, +1.0%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.05 ms | 7.07 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.3 ms | 12.2 ms | -1.2% | [-3.9%, +1.6%] | +0.0% | ~ | marched/traced 5.55x (was 5.82x) |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 71.7 ms | 67.5 ms | -5.9% | [-14.6%, +3.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 9.20 ms | 9.08 ms | -1.3% | [-6.4%, +3.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.14 ms | 2.20 ms | +2.7% | [-2.5%, +8.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.19 ms | 7.30 ms | +1.5% | [-0.4%, +3.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 248 ms | 249 ms | +0.3% | [-2.1%, +2.8%] | +0.0% | ~ | marched/traced 1.43x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 348 ms | 356 ms | +2.4% | [-2.5%, +7.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 244 ms | 245 ms | +0.1% | [-1.8%, +2.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 260 ms | 259 ms | -0.4% | [-2.7%, +2.0%] | +0.0% | ~ | marched/traced 1.59x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 405 ms | 411 ms | +1.5% | [-2.8%, +6.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 259 ms | 258 ms | -0.2% | [-2.8%, +2.5%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.9 ms | 24.1 ms | +0.5% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 24.0 ms | 24.1 ms | +0.1% | [-2.5%, +2.8%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.3 ms | 19.8 ms | +2.6% | [-4.5%, +10.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 29.5 ms | 29.7 ms | +0.6% | [-2.4%, +3.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 29.8 ms | 29.6 ms | -0.5% | [-2.2%, +1.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.4 ms | 22.0 ms | +3.0% | [+2.9%, +3.2%] | +0.0% | minor |  |
| `Tape/csg/signedDistance/float/host` | 12.5 ms | 12.6 ms | +1.4% | [+0.6%, +2.3%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.56 ms | 4.63 ms | +1.6% | [+0.3%, +3.1%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.4 ms | 24.3 ms | -0.7% | [-3.9%, +2.7%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.8 ms | 41.2 ms | +1.2% | [-1.7%, +4.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.76 ms | 1.78 ms | +0.7% | [-0.2%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 127 µs | 128 µs | +0.7% | [-1.3%, +2.8%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.7 ms | 30.4 ms | -1.0% | [-1.9%, -0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.75 ms | 5.68 ms | -1.3% | [-2.5%, -0.0%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.0 ms | 16.3 ms | +1.9% | [+1.6%, +2.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.68 ms | 4.68 ms | +0.1% | [-2.0%, +2.3%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.0 ms | 12.9 ms | -0.9% | [-1.7%, -0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.60 ms | 4.62 ms | +0.5% | [-0.5%, +1.4%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 25.4 ms | 25.4 ms | +0.2% | [-1.7%, +2.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 43.2 ms | 43.6 ms | +0.7% | [-1.6%, +3.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.75 ms | 1.75 ms | +0.2% | [-1.6%, +2.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 269 µs | 264 µs | -1.8% | [-4.5%, +1.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.8 ms | 30.6 ms | -0.6% | [-1.5%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.84 ms | 7.84 ms | +0.0% | [-1.6%, +1.7%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.2 ms | 16.2 ms | +0.2% | [-0.6%, +1.1%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.82 ms | 7.83 ms | +0.1% | [-0.9%, +1.2%] | - | ~ |  |

<!-- END receipt host -->
