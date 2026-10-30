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
base: 21f24b4
base-fingerprint: 954ae65e6912091a01c701738e4204fd83116991
head: 8f90e16
head-fingerprint: 39675652ec063ec2cbaa9642714f87d3a7af3d99
cases: 152
rounds: 2
reps: 3
date: 2026-10-30
-->

## Host comparison: yellow

**Accepted:** Accept as noise.

`8f90e16` against `21f24b4` (origin/dev), 2026-10-30.

**VERDICT: 5 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 5 minor, 4 faster, 0 differ, 0 lost, 152 cases

- base: 21f24b4 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8f90e16 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 361 ms | 430 ms | +19.0% | [-12.7%, +55.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 192 ms | 192 ms | +0.3% | [-7.6%, +8.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 536 ms | 546 ms | +1.9% | [-3.7%, +7.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 359 ms | 364 ms | +1.2% | [-3.4%, +6.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 191 ms | 189 ms | -0.7% | [-1.8%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 585 ms | 583 ms | -0.3% | [-3.7%, +3.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 56.5 ms | 56.5 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 52.2 ms | 52.1 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 65.0 ms | 65.3 ms | +0.5% | [-0.3%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 57.8 ms | 56.0 ms | -3.1% | [-5.6%, -0.4%] | +0.0% | faster |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 63.1 ms | 61.6 ms | -2.4% | [-5.1%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 73.8 ms | 73.8 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 75.6 ms | 75.8 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 85.7 ms | 85.0 ms | -0.9% | [-9.2%, +8.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 68.2 ms | 68.4 ms | +0.3% | [-0.7%, +1.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 105 ms | 105 ms | +0.0% | [-3.0%, +3.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 84.8 ms | 85.6 ms | +0.9% | [-1.2%, +3.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 121 ms | 127 ms | +5.1% | [+1.8%, +8.4%] | +0.0% | minor |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.8 ms | 19.7 ms | -0.7% | [-1.5%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 47.1 ms | 47.2 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 76.9 ms | 77.0 ms | +0.1% | [-2.9%, +3.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 21.3 ms | 21.2 ms | -0.1% | [-1.9%, +1.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 47.3 ms | 47.1 ms | -0.5% | [-1.2%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 89.3 ms | 89.1 ms | -0.2% | [-7.1%, +7.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.56 ms | 4.49 ms | -1.5% | [-2.0%, -1.0%] | - | ~ | tape/fused 2.83x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | tape/fused 2.00x |
| `Fused/chain16/signedDistance/float/host` | 986 µs | 989 µs | +0.3% | [-1.7%, +2.3%] | - | ~ | tape/fused 22.24x (was 16.49x) |
| `Fused/drilled-mesh/signedDistance/float/host` | 48.2 ms | 49.0 ms | +1.8% | [-0.7%, +4.2%] | +0.0% | ~ | tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 48.1 ms | 48.8 ms | +1.4% | [+0.0%, +2.7%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.50 ms | 4.55 ms | +1.2% | [+0.1%, +2.4%] | - | ~ | tape/fused 3.78x (was 3.97x) |
| `Fused/smooth/signedDistance/double/host` | 13.0 ms | 12.8 ms | -1.0% | [-1.7%, -0.4%] | - | ~ | tape/fused 2.53x (was 3.09x) |
| `Fused/chain16/signedDistance/double/host` | 3.97 ms | 3.96 ms | -0.1% | [-1.1%, +0.9%] | - | ~ | tape/fused 7.08x (was 6.07x) |
| `Fused/drilled-mesh/signedDistance/double/host` | 58.0 ms | 56.9 ms | -1.9% | [-3.8%, +0.1%] | +0.0% | ~ | tape/fused 1.04x (was 1.20x) |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 69.6 ms | 59.0 ms | -15.2% | [-30.1%, +6.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 84.6 ms | 84.3 ms | -0.3% | [-1.9%, +1.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 240 ms | 248 ms | +3.6% | [+1.9%, +5.3%] | +0.0% | minor |  |
| `TriMeshSDF/w4/build/double/host` | 120 ms | 115 ms | -4.0% | [-9.1%, +1.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 292 ms | 282 ms | -3.5% | [-6.3%, -0.5%] | +0.0% | faster |  |
| `TriMeshSDF/w8/build/float/host` | 72.0 ms | 72.0 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 228 ms | 224 ms | -1.7% | [-2.7%, -0.7%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 99.1 ms | 99.3 ms | +0.2% | [-1.1%, +1.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 32.2 ms | 32.4 ms | +0.7% | [+0.2%, +1.2%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 854 ms | 830 ms | -2.8% | [-6.2%, +0.9%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 121 ms | 121 ms | +0.2% | [-1.8%, +2.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 55.9 ms | 59.4 ms | +6.2% | [+2.8%, +9.8%] | - | minor |  |
| `MeshSDF/query-random/double/host` | 1.00 s | 977 ms | -2.8% | [-6.9%, +1.7%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 70.9 ms | 70.2 ms | -0.9% | [-3.6%, +1.8%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 158 ms | 158 ms | +0.0% | [-2.3%, +2.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 388 ms | 390 ms | +0.7% | [-0.8%, +2.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 81.2 ms | 82.4 ms | +1.4% | [+0.7%, +2.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.75 ms | 6.88 ms | +1.8% | [+0.0%, +3.6%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 87.2 ms | 86.8 ms | -0.4% | [-7.8%, +7.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 176 ms | 176 ms | -0.3% | [-2.7%, +2.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 413 ms | 413 ms | +0.0% | [-0.7%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 94.4 ms | 94.1 ms | -0.3% | [-2.3%, +1.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.33 ms | 7.53 ms | +2.8% | [-1.1%, +6.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 10.4 ms | 11.4 ms | +9.8% | [-1.3%, +21.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 128 ms | 132 ms | +2.9% | [-0.1%, +5.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 530 ms | 534 ms | +0.9% | [+0.4%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 157 ms | 156 ms | -0.6% | [-1.1%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 883 ms | 871 ms | -1.4% | [-1.9%, -0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 15.1 ms | +0.3% | [-7.6%, +9.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 136 ms | 135 ms | -0.2% | [-1.8%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 564 ms | 565 ms | +0.2% | [-0.7%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 165 ms | 167 ms | +0.9% | [-0.6%, +2.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 1.03 s | 978 ms | -5.3% | [-10.2%, -0.3%] | - | faster |  |
| `PointCloudBVH/sphere/build/float/host` | 70.8 ms | 70.9 ms | +0.1% | [-1.5%, +1.8%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 117 ms | 120 ms | +2.1% | [-0.2%, +4.5%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 285 ms | 282 ms | -0.9% | [-1.1%, -0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 115 ms | 115 ms | -0.6% | [-1.4%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 110 ms | 111 ms | +1.0% | [-1.0%, +3.1%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 83.4 ms | 82.7 ms | -0.8% | [-1.0%, -0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 133 ms | 132 ms | -0.8% | [-1.4%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 305 ms | 303 ms | -0.6% | [-1.7%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 139 ms | 168 ms | +21.1% | [-4.8%, +48.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 133 ms | 134 ms | +0.7% | [-4.1%, +5.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 9.06 ms | 9.28 ms | +2.5% | [+1.1%, +3.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 148 ms | 148 ms | -0.1% | [-4.9%, +5.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 407 ms | 408 ms | +0.3% | [-3.0%, +3.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 257 ms | 248 ms | -3.3% | [-6.6%, -0.1%] | - | faster |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 357 ms | 344 ms | -3.5% | [-6.9%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 13.3 ms | 13.3 ms | -0.1% | [-2.9%, +2.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 154 ms | 153 ms | -0.3% | [-6.3%, +6.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 441 ms | 443 ms | +0.4% | [-1.5%, +2.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 320 ms | 301 ms | -5.8% | [-13.0%, +2.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 374 ms | 364 ms | -2.5% | [-6.0%, +1.2%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 48.4 ms | 48.1 ms | -0.7% | [-0.8%, -0.5%] | +0.0% | ~ | hand/api 1.01x (was 0.96x) |
| `Ray/mesh/firstHit-handrolled/float/host` | 46.5 ms | 48.5 ms | +4.3% | [+0.7%, +8.1%] | +0.0% | minor |  |
| `Ray/mesh/intersects/float/host` | 42.8 ms | 42.5 ms | -0.6% | [-2.3%, +1.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 86.0 ms | 85.2 ms | -0.9% | [-4.0%, +2.5%] | +0.0% | ~ | triangles/polygons 0.56x |
| `Ray/mesh-polygons/intersects/float/host` | 71.5 ms | 71.5 ms | +0.0% | [-2.4%, +2.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.93 ms | 5.98 ms | +0.8% | [-0.3%, +2.0%] | - | ~ | tape/fused 1.31x |
| `Ray/csg-tape/firstHit/float/host` | 7.76 ms | 7.82 ms | +0.8% | [+0.0%, +1.5%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 55.3 ms | 63.3 ms | +14.5% | [-1.8%, +31.6%] | +0.0% | ~ | hand/api 0.88x (was 1.02x) |
| `Ray/mesh/firstHit-handrolled/double/host` | 56.2 ms | 56.0 ms | -0.4% | [-5.2%, +4.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 47.9 ms | 48.4 ms | +1.2% | [-1.6%, +4.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 151 ms | 141 ms | -6.9% | [-49.1%, +70.8%] | +0.0% | ~ | triangles/polygons 0.45x (was 0.37x) |
| `Ray/mesh-polygons/intersects/double/host` | 114 ms | 118 ms | +3.2% | [-42.7%, +84.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 6.09 ms | 6.10 ms | +0.1% | [-2.1%, +2.4%] | - | ~ | tape/fused 1.39x |
| `Ray/csg-tape/firstHit/double/host` | 8.38 ms | 8.45 ms | +0.9% | [-1.3%, +3.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 19.1 ms | 19.2 ms | +0.4% | [-5.1%, +6.1%] | - | ~ | marched/traced 6.22x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 119 ms | +0.1% | [-2.1%, +2.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.8 ms | 22.6 ms | -0.6% | [-1.9%, +0.6%] | +0.0% | ~ | marched/traced 8.04x (was 7.36x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 167 ms | 182 ms | +8.5% | [+7.2%, +9.9%] | +0.0% | minor |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 38.4 ms | 38.4 ms | -0.1% | [-8.2%, +8.7%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 20.8 ms | 20.7 ms | -0.4% | [-9.0%, +8.9%] | - | ~ | marched/traced 6.53x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 137 ms | 135 ms | -1.5% | [-8.3%, +5.7%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 25.4 ms | 25.3 ms | -0.2% | [-8.3%, +8.7%] | +0.0% | ~ | marched/traced 7.08x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 181 ms | 179 ms | -1.0% | [-10.0%, +8.9%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 40.4 ms | 40.4 ms | +0.2% | [-9.7%, +11.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/float/host` | 12.3 ms | 12.6 ms | +2.8% | [-8.9%, +16.3%] | +0.0% | ~ | marched/traced 5.15x (was 5.32x) |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 65.4 ms | 65.1 ms | -0.4% | [-28.4%, +39.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.93 ms | 8.99 ms | +0.7% | [-14.9%, +18.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.07 ms | 2.06 ms | -0.5% | [-16.5%, +18.7%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.24 ms | 7.30 ms | +0.9% | [-8.1%, +10.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 13.3 ms | 13.4 ms | +0.3% | [-16.3%, +20.0%] | +0.0% | ~ | marched/traced 6.13x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 82.7 ms | 81.9 ms | -1.0% | [-31.1%, +43.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 9.71 ms | 9.49 ms | -2.2% | [-14.8%, +11.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.29 ms | 2.29 ms | -0.2% | [-9.1%, +9.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.41 ms | 7.44 ms | +0.4% | [-5.2%, +6.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 261 ms | 258 ms | -1.3% | [-8.3%, +6.5%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 363 ms | 356 ms | -1.9% | [-6.5%, +3.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 245 ms | 246 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 264 ms | 261 ms | -1.1% | [-2.3%, +0.2%] | +0.0% | ~ | marched/traced 1.62x (was 1.57x) |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 413 ms | 422 ms | +2.1% | [+0.6%, +3.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 261 ms | 261 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 24.1 ms | 24.1 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 24.4 ms | 23.9 ms | -2.1% | [-4.7%, +0.7%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.5 ms | 19.5 ms | -0.1% | [-5.3%, +5.5%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 29.4 ms | 29.4 ms | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 29.4 ms | 29.4 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 22.7 ms | 22.6 ms | -0.7% | [-7.8%, +7.0%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.7 ms | 12.7 ms | +0.4% | [-2.6%, +3.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.59 ms | 4.60 ms | +0.4% | [-2.3%, +3.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.7 ms | 24.7 ms | -0.1% | [-2.6%, +2.4%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 41.6 ms | 41.7 ms | +0.2% | [-2.4%, +2.9%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.78 ms | 1.77 ms | -0.1% | [-1.4%, +1.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 139 µs | 144 µs | +3.3% | [-16.0%, +26.2%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.8 ms | 30.9 ms | +0.1% | [-2.3%, +2.5%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.72 ms | 5.75 ms | +0.5% | [-1.3%, +2.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 22.0 ms | +35.2% | [-2.9%, +74.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 5.22 ms | 5.23 ms | +0.3% | [-17.2%, +21.3%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 17.9 ms | 17.2 ms | -3.7% | [-42.6%, +64.3%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 5.72 ms | 6.66 ms | +16.4% | [-31.6%, +85.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 40.0 ms | 32.5 ms | -18.8% | [-52.3%, +50.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 52.8 ms | 51.8 ms | -2.0% | [-31.7%, +39.7%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 2.75 ms | 2.70 ms | -1.8% | [-52.7%, +105.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 513 µs | 480 µs | -6.4% | [-63.7%, +156.0%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 45.3 ms | 45.6 ms | +0.7% | [-48.6%, +95.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 8.22 ms | 8.28 ms | +0.8% | [-7.9%, +10.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 24.1 ms | 28.0 ms | +16.4% | [-48.4%, +143.1%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 8.77 ms | 8.64 ms | -1.4% | [-18.3%, +19.2%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/BVH/main.cpp, Performance/BVHUnion/main.cpp, Performance/ClosestPrimitive/main.cpp, Performance/Fused/main.cpp, Performance/LaunchTuning/main.cpp, Performance/PointCloud/main.cpp, Performance/Ray/main.cpp, Performance/RayCSG/main.cpp, Performance/RayShell/main.cpp, Performance/Tape/main.cpp, Performance/Common/Inputs.hpp, Performance/Common/Statistics.hpp, Performance/Common/Suite.hpp

<!-- END receipt host -->
