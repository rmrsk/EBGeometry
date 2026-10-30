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
base: 21f24b4
base-fingerprint: 954ae65e6912091a01c701738e4204fd83116991
head: 97f5b37
head-fingerprint: 15e72c521a37e92d83e35f79c4f2bfb0d6428eec
cases: 152
rounds: 2
reps: 3
date: 2026-10-30
-->

## Host comparison: red

**Accepted:** Accept as noise.

`97f5b37` against `21f24b4` (origin/dev), 2026-10-30.

**VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 1 major, 1 minor, 0 faster, 0 differ, 0 lost, 152 cases

- base: 21f24b4 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 97f5b37 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 329 ms | 330 ms | +0.5% | [-0.0%, +1.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 516 ms | 514 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 347 ms | 346 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 186 ms | 186 ms | -0.1% | [-0.8%, +0.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 564 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.1 ms | 55.4 ms | +0.6% | [+0.3%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.2 ms | 51.3 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.4 ms | 64.5 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.8 ms | 54.3 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.4 ms | 60.5 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.2 ms | 72.2 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.2 ms | 74.4 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.8 ms | 77.8 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.8 ms | 66.8 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.5 ms | 100 ms | +0.6% | [-0.1%, +1.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.3 ms | 82.9 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 109 ms | -0.5% | [-1.5%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.3 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.8 ms | 45.3 ms | -1.0% | [-2.7%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.2 ms | 73.1 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.8 ms | 20.7 ms | -0.7% | [-2.3%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 46.5 ms | 46.0 ms | -1.1% | [-2.6%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.2 ms | 81.4 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.47 ms | 4.47 ms | +0.0% | [-1.7%, +1.7%] | - | ~ | tape/fused 2.79x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.2 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | tape/fused 1.96x |
| `Fused/chain16/signedDistance/float/host` | 965 µs | 971 µs | +0.7% | [-0.5%, +1.9%] | - | ~ | tape/fused 16.29x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.3 ms | 46.3 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.6 ms | +0.6% | [+0.4%, +0.7%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.52 ms | 4.53 ms | +0.2% | [-0.7%, +1.1%] | - | ~ | tape/fused 2.84x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.9 ms | +0.9% | [+0.4%, +1.5%] | - | ~ | tape/fused 1.95x |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.90 ms | +0.2% | [-1.5%, +1.9%] | - | ~ | tape/fused 4.09x |
| `Fused/drilled-mesh/signedDistance/double/host` | 54.5 ms | 53.5 ms | -1.9% | [-2.9%, -0.9%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 54.3 ms | 54.3 ms | -0.1% | [-1.3%, +1.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.1 ms | 82.1 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 230 ms | 231 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 266 ms | 265 ms | -0.3% | [-1.4%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.6 ms | 70.7 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 216 ms | 215 ms | -0.2% | [-1.5%, +1.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.8 ms | 96.4 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.2 ms | 30.1 ms | -0.2% | [-1.7%, +1.3%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 763 ms | 768 ms | +0.6% | [+0.3%, +1.0%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 45.1 ms | 44.4 ms | -1.6% | [-4.3%, +1.2%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 856 ms | 856 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 68.1 ms | 68.2 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 149 ms | 148 ms | -0.7% | [-2.4%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 373 ms | 374 ms | +0.3% | [-0.0%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.1 ms | 78.4 ms | +0.4% | [-0.5%, +1.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.68 ms | 6.73 ms | +0.7% | [+0.5%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 80.9 ms | 81.1 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 161 ms | 161 ms | -0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 400 ms | 402 ms | +0.5% | [+0.2%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 88.0 ms | 87.8 ms | -0.3% | [-1.1%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.17 ms | 7.16 ms | -0.1% | [-0.6%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 10.9 ms | 11.6 ms | +6.5% | [-2.0%, +16.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 123 ms | 123 ms | +0.2% | [-0.3%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 516 ms | 517 ms | +0.2% | [-0.4%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 153 ms | 153 ms | -0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 845 ms | 842 ms | -0.4% | [-0.8%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.8 ms | 15.2 ms | +2.3% | [-0.1%, +4.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 127 ms | 127 ms | +0.2% | [-0.5%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 546 ms | 547 ms | +0.2% | [+0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 157 ms | 156 ms | -0.3% | [-1.0%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 890 ms | 896 ms | +0.7% | [+0.5%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.4 ms | 67.3 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 109 ms | 109 ms | +0.0% | [-0.4%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 271 ms | 271 ms | +0.0% | [-0.4%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 111 ms | 111 ms | +0.1% | [-0.3%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 106 ms | 106 ms | +0.4% | [+0.3%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 80.5 ms | 80.3 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 120 ms | 121 ms | +0.3% | [-0.2%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 296 ms | 296 ms | +0.1% | [+0.0%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 128 ms | 128 ms | +0.1% | [-0.5%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 123 ms | 122 ms | -0.2% | [-0.5%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.80 ms | 8.92 ms | +1.4% | [+0.2%, +2.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 144 ms | 141 ms | -1.9% | [-4.0%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 388 ms | 388 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 258 ms | 252 ms | -2.5% | [-5.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 334 ms | 334 ms | +0.1% | [-0.0%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.8 ms | 12.6 ms | -1.6% | [-3.7%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 148 ms | 145 ms | -1.9% | [-4.7%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 433 ms | 433 ms | -0.1% | [-0.1%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 284 ms | 293 ms | +3.1% | [+1.9%, +4.3%] | - | minor |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 347 ms | 346 ms | -0.3% | [-0.3%, -0.3%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.7 ms | 46.4 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.7 ms | 45.7 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.2 ms | -0.5% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.6 ms | 78.0 ms | -0.7% | [-2.4%, +1.0%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/float/host` | 65.6 ms | 65.6 ms | +0.1% | [-0.6%, +0.9%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.91 ms | 6.56 ms | +11.0% | [+4.6%, +17.4%] | - | **MAJOR** | tape/fused 1.18x (was 1.32x) |
| `Ray/csg-tape/firstHit/float/host` | 7.81 ms | 7.76 ms | -0.6% | [-1.4%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 51.6 ms | 51.7 ms | +0.2% | [-0.8%, +1.1%] | +0.0% | ~ | hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.4 ms | 51.7 ms | +0.7% | [+0.3%, +1.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.0 ms | 45.2 ms | +0.4% | [-0.7%, +1.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.6 ms | 87.3 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/double/host` | 74.2 ms | 73.9 ms | -0.4% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.98 ms | 5.94 ms | -0.7% | [-1.4%, -0.0%] | - | ~ | tape/fused 1.38x |
| `Ray/csg-tape/firstHit/double/host` | 8.22 ms | 8.22 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 19.0 ms | 18.9 ms | -0.2% | [-1.2%, +0.8%] | - | ~ | marched/traced 6.13x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 116 ms | 116 ms | +0.2% | [-0.1%, +0.4%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 22.0 ms | -0.4% | [-1.3%, +0.5%] | +0.0% | ~ | marched/traced 7.26x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 160 ms | 160 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.5 ms | 35.5 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.2 ms | 19.1 ms | -0.5% | [-1.3%, +0.2%] | - | ~ | marched/traced 6.51x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 125 ms | 125 ms | -0.3% | [-0.6%, +0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.3 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ | marched/traced 7.32x (was 7.10x) |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 166 ms | 171 ms | +3.1% | [-0.3%, +6.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.7 ms | 37.7 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | +0.6% | [+0.5%, +0.6%] | +0.0% | ~ | marched/traced 4.65x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 52.1 ms | 51.8 ms | -0.6% | [-0.7%, -0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.15 ms | 8.03 ms | -1.5% | [-3.4%, +0.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.86 ms | 1.85 ms | -0.1% | [-1.5%, +1.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.93 ms | 6.91 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.6 ms | -0.3% | [-1.1%, +0.5%] | +0.0% | ~ | marched/traced 5.27x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.9 ms | 61.3 ms | +0.7% | [+0.3%, +1.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.72 ms | 8.60 ms | -1.4% | [-1.8%, -1.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.06 ms | 2.08 ms | +0.8% | [-0.6%, +2.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.09 ms | 7.18 ms | +1.3% | [+0.3%, +2.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 240 ms | 242 ms | +0.5% | [+0.0%, +0.9%] | +0.0% | ~ | marched/traced 1.40x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 337 ms | 338 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 240 ms | 240 ms | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 254 ms | 253 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | marched/traced 1.54x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 390 ms | 390 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 252 ms | 252 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.5 ms | 23.6 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.6 ms | 23.6 ms | -0.2% | [-1.5%, +1.0%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.0 ms | 18.8 ms | -1.1% | [-4.6%, +2.5%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 29.1 ms | 29.0 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 28.8 ms | 28.9 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.1 ms | 21.4 ms | +1.6% | [+0.9%, +2.4%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.5 ms | 12.5 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 5.72 ms | 4.54 ms | -20.5% | [-34.9%, +1.9%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.1 ms | 24.0 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.2 ms | 40.3 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.75 ms | 1.74 ms | -0.8% | [-1.8%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 133 µs | 132 µs | -0.4% | [-9.8%, +10.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.9 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.65 ms | 5.66 ms | +0.0% | [-0.5%, +0.5%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.0 ms | 15.8 ms | -1.1% | [-2.7%, +0.6%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.61 ms | 4.66 ms | +1.0% | [-0.3%, +2.3%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 12.9 ms | 12.8 ms | -0.7% | [-1.2%, -0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.56 ms | 4.56 ms | -0.1% | [-0.5%, +0.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 25.2 ms | 25.2 ms | -0.1% | [-0.9%, +0.7%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.0 ms | 42.2 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.76 ms | 1.76 ms | +0.3% | [-1.5%, +2.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 275 µs | 281 µs | +2.3% | [-4.5%, +9.9%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.0 ms | 29.8 ms | -0.7% | [-0.8%, -0.5%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.76 ms | 7.76 ms | -0.1% | [-0.6%, +0.5%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.0 ms | 16.0 ms | -0.3% | [-1.2%, +0.6%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.74 ms | 7.75 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/Fused/main.cpp, Performance/Tape/main.cpp

<!-- END receipt host -->
