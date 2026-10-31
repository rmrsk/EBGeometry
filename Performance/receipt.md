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
base: 2c0c57d
base-fingerprint: 59130b91cbb90f3926949df3226bf4d112d28a84
head: 8d920a3
head-fingerprint: 842404e655765c0b7f2428767c0e13ed1423660b
cases: 152
rounds: 2
reps: 3
date: 2026-10-31
-->

## Host comparison: red

**Accepted:** Accept as noise

`8d920a3` against `2c0c57d` (origin/main), 2026-10-31.

**VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 1 major, 0 minor, 0 faster, 0 differ, 0 lost, 152 cases

- base: 2c0c57d (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8d920a3 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 329 ms | 330 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 177 ms | 177 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 514 ms | 515 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 345 ms | 346 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 185 ms | 185 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 562 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.1 ms | 55.0 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.7 ms | 51.2 ms | -1.0% | [-2.3%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.0 ms | 63.8 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.5 ms | 54.5 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.1 ms | 59.8 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.0 ms | 72.2 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 73.7 ms | 74.0 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.4 ms | 77.5 ms | +0.1% | [-0.9%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.4 ms | 66.5 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 100 ms | +0.0% | [-0.9%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.2 ms | 82.4 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 109 ms | +0.5% | [-0.8%, +1.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.1 ms | 19.3 ms | +0.9% | [-0.1%, +1.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 44.5 ms | 46.9 ms | +5.6% | [-3.8%, +15.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.6 ms | 72.6 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.3 ms | 20.1 ms | -0.9% | [-2.1%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 46.0 ms | 45.7 ms | -0.8% | [-1.1%, -0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.5 ms | 84.0 ms | +1.9% | [-0.1%, +3.9%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.39 ms | 4.41 ms | +0.5% | [-1.2%, +2.1%] | - | ~ | tape/fused 2.82x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | tape/fused 1.94x |
| `Fused/chain16/signedDistance/float/host` | 963 µs | 960 µs | -0.2% | [-1.5%, +1.0%] | - | ~ | tape/fused 16.46x |
| `Fused/drilled-mesh/signedDistance/float/host` | 45.9 ms | 46.0 ms | +0.2% | [+0.2%, +0.2%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.2 ms | 46.1 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.54 ms | 4.54 ms | +0.0% | [-1.0%, +1.1%] | - | ~ | tape/fused 2.84x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | -0.3% | [-0.7%, +0.2%] | - | ~ | tape/fused 1.97x |
| `Fused/chain16/signedDistance/double/host` | 3.86 ms | 3.87 ms | +0.2% | [-0.6%, +1.1%] | - | ~ | tape/fused 4.13x |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.2 ms | 53.3 ms | +0.2% | [-0.7%, +1.1%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.3 ms | 53.6 ms | +0.5% | [-0.0%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.7 ms | 81.7 ms | -0.1% | [-0.9%, +0.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 234 ms | +2.5% | [-0.3%, +5.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 112 ms | 112 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 264 ms | 263 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.1 ms | 70.0 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 213 ms | 212 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 95.5 ms | 95.9 ms | +0.5% | [-0.0%, +1.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.7 ms | 30.5 ms | +2.5% | [+2.1%, +2.9%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 756 ms | 757 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.0 ms | 43.0 ms | -2.4% | [-5.3%, +0.6%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 849 ms | 841 ms | -0.8% | [-1.6%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 68.2 ms | 67.6 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 150 ms | 147 ms | -1.6% | [-3.3%, +0.1%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 369 ms | 370 ms | +0.2% | [-0.0%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.6 ms | 79.7 ms | +1.4% | [+0.4%, +2.4%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.61 ms | 6.65 ms | +0.6% | [+0.2%, +1.0%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 80.4 ms | 80.2 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 158 ms | 158 ms | -0.2% | [-0.7%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 398 ms | 400 ms | +0.3% | [+0.0%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.3 ms | 87.1 ms | -0.2% | [-1.1%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.16 ms | 7.18 ms | +0.3% | [-0.3%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 10.4 ms | 11.7 ms | +11.9% | [+3.3%, +21.9%] | +0.0% | **MAJOR** |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 122 ms | 122 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 514 ms | 513 ms | -0.2% | [-0.3%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 152 ms | -0.3% | [-1.2%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 839 ms | 840 ms | +0.1% | [-0.4%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 15.0 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 127 ms | 127 ms | +0.2% | [-0.1%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 544 ms | 544 ms | -0.0% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 155 ms | 155 ms | -0.4% | [-1.1%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 884 ms | 891 ms | +0.8% | [+0.7%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.2 ms | 67.1 ms | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 109 ms | 109 ms | -0.2% | [-1.1%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 270 ms | 270 ms | -0.0% | [-0.2%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 111 ms | 112 ms | +0.2% | [-0.4%, +0.7%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 106 ms | 106 ms | +0.2% | [+0.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 80.1 ms | 79.7 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 120 ms | 120 ms | +0.6% | [-0.1%, +1.3%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 294 ms | 296 ms | +0.7% | [+0.5%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 127 ms | 128 ms | +0.2% | [+0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 122 ms | 121 ms | -0.9% | [-1.8%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 9.65 ms | 9.56 ms | -0.9% | [-15.2%, +15.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 144 ms | 141 ms | -2.3% | [-4.6%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 399 ms | 399 ms | +0.1% | [-0.4%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 252 ms | 250 ms | -0.8% | [-1.6%, -0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 331 ms | 333 ms | +0.7% | [+0.2%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.4 ms | 12.6 ms | +1.8% | [-0.2%, +3.8%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 144 ms | 144 ms | -0.1% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 417 ms | 418 ms | +0.3% | [-0.2%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 283 ms | 283 ms | +0.3% | [-0.2%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 344 ms | 345 ms | +0.3% | [-0.5%, +1.2%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.3 ms | 46.5 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.7 ms | 45.7 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.6 ms | +0.4% | [-0.0%, +0.9%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 77.9 ms | 77.1 ms | -1.0% | [-1.4%, -0.7%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/float/host` | 65.0 ms | 66.1 ms | +1.7% | [+0.7%, +2.7%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.75 ms | 5.81 ms | +1.1% | [-0.1%, +2.4%] | - | ~ | tape/fused 1.34x |
| `Ray/csg-tape/firstHit/float/host` | 7.72 ms | 7.76 ms | +0.5% | [-2.7%, +3.8%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 51.2 ms | 51.5 ms | +0.6% | [-0.2%, +1.5%] | +0.0% | ~ | hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.0 ms | 50.8 ms | -0.2% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.6 ms | 44.7 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 86.3 ms | 86.4 ms | +0.1% | [-1.5%, +1.8%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 74.7 ms | 72.8 ms | -2.6% | [-5.0%, -0.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.93 ms | 5.97 ms | +0.7% | [+0.1%, +1.3%] | - | ~ | tape/fused 1.37x |
| `Ray/csg-tape/firstHit/double/host` | 8.22 ms | 8.21 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.6 ms | 18.1 ms | -2.6% | [-7.4%, +2.5%] | - | ~ | marched/traced 6.43x (was 6.21x) |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 116 ms | 117 ms | +0.9% | [+0.7%, +1.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 22.1 ms | +0.6% | [+0.1%, +1.2%] | +0.0% | ~ | marched/traced 7.21x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 160 ms | 159 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.4 ms | 35.7 ms | +0.6% | [+0.2%, +1.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.3 ms | 18.8 ms | -2.5% | [-3.3%, -1.6%] | - | ~ | marched/traced 6.59x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 125 ms | 124 ms | -0.8% | [-0.9%, -0.7%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 23.2 ms | -1.1% | [-1.7%, -0.5%] | +0.0% | ~ | marched/traced 7.24x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 166 ms | 168 ms | +1.5% | [+0.5%, +2.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.7 ms | 37.6 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.2 ms | 11.1 ms | -0.9% | [-2.1%, +0.3%] | +0.0% | ~ | marched/traced 4.65x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.6 ms | 51.6 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.11 ms | 7.99 ms | -1.5% | [-3.0%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.85 ms | 1.84 ms | -0.7% | [-1.8%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.80 ms | 6.78 ms | -0.3% | [-2.1%, +1.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.6 ms | 11.5 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | marched/traced 5.23x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.7 ms | 60.4 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.87 ms | 8.60 ms | -3.1% | [-6.7%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.11 ms | 2.08 ms | -1.7% | [-1.9%, -1.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.03 ms | 7.06 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 242 ms | 241 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ | marched/traced 1.40x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 337 ms | 337 ms | +0.1% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 240 ms | 239 ms | -0.3% | [-0.3%, -0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 252 ms | 254 ms | +0.7% | [+0.3%, +1.1%] | +0.0% | ~ | marched/traced 1.51x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 382 ms | 383 ms | +0.3% | [-1.0%, +1.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 251 ms | 250 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.8 ms | 23.7 ms | -0.6% | [-1.8%, +0.5%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.6 ms | 23.7 ms | +0.5% | [-0.2%, +1.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 18.4 ms | -1.1% | [-2.3%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 28.8 ms | 28.8 ms | +0.1% | [-0.7%, +0.8%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 28.7 ms | 29.0 ms | +1.3% | [+1.3%, +1.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.0 ms | 21.3 ms | +1.5% | [-0.3%, +3.3%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.5 ms | 12.4 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.55 ms | 4.53 ms | -0.5% | [-1.2%, +0.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 23.9 ms | 23.8 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.5 ms | 40.3 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.74 ms | 1.75 ms | +0.2% | [-2.2%, +2.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 134 µs | 132 µs | -1.9% | [-8.5%, +5.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.9 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.63 ms | 5.65 ms | +0.5% | [-0.5%, +1.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 15.9 ms | 15.8 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.61 ms | 4.63 ms | +0.4% | [-1.0%, +1.8%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 12.8 ms | 12.9 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.58 ms | 4.57 ms | -0.1% | [-0.7%, +0.6%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 25.1 ms | 25.2 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.0 ms | 42.0 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.74 ms | 1.75 ms | +0.3% | [-2.0%, +2.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 269 µs | 259 µs | -3.8% | [-9.0%, +1.7%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 30.0 ms | 29.9 ms | -0.3% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.75 ms | 7.74 ms | -0.1% | [-0.5%, +0.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.0 ms | 16.0 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.74 ms | 7.75 ms | +0.0% | [-0.5%, +0.6%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/RayCSG/main.cpp, Performance/Common/Device.hpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 2c0c57d
base-fingerprint: 59130b91cbb90f3926949df3226bf4d112d28a84
head: 8d920a3
head-fingerprint: 842404e655765c0b7f2428767c0e13ed1423660b
cases: 218
rounds: 2
reps: 3
date: 2026-10-31
-->

## Device comparison (CUDA): yellow

**Accepted:** Accept as noise

`8d920a3` against `2c0c57d` (origin/main), 2026-10-31.

**VERDICT: 7 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 7 minor, 0 faster, 0 differ, 0 lost, 218 cases

- base: 2c0c57d (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8d920a3 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.62 ms | 1.60 ms | -1.1% | [-1.9%, -0.3%] | - | ~ | device 27.0x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.41 ms | +0.2% | [-0.3%, +0.8%] | - | ~ | device 20.4x faster than the host (was 21.4x) |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.06 ms | +0.9% | [-0.6%, +2.4%] | - | ~ | device 20.9x faster than the host (was 27.4x) |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.31 ms | 2.30 ms | -0.3% | [-0.8%, +0.3%] | - | ~ | device 17.9x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.23 ms | 6.12 ms | -1.7% | [-4.5%, +1.3%] | - | ~ | device 8.1x faster than the host (was 9.0x) |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.8 ms | 11.8 ms | -0.3% | [-0.9%, +0.4%] | - | ~ | device 6.2x faster than the host (was 6.4x) |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.5 ms | 33.3 ms | -0.5% | [-0.8%, -0.3%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.0 ms | 57.0 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 1.2x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.6 ms | 20.7 ms | +0.7% | [-0.8%, +2.2%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.8 ms | 48.0 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 62.2 ms | 61.5 ms | -1.2% | [-1.2%, -1.1%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 109 ms | 109 ms | -0.0% | [-2.5%, +2.6%] | - | ~ | device 0.6x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 387 µs | 388 µs | +0.1% | [+0.0%, +0.1%] | - | ~ | device 15.0x faster than the host (was 17.1x) |
| `BVHUnion/spheres/query-random/float/hostdevice` | 845 µs | 847 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 9.9x faster than the host (was 13.8x) |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.54 ms | 4.52 ms | -0.5% | [-0.9%, +0.0%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.67 ms | 4.65 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 2.6x faster than the host (was 2.8x) |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 589 µs | 590 µs | +0.3% | [+0.0%, +0.5%] | - | ~ | device 18.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 12.0x faster than the host (was 8.4x) |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.75 ms | 6.77 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.00 ms | 7.00 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 907 µs | 903 µs | -0.4% | [-0.6%, -0.3%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.24 ms | 2.24 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.45 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 319 µs | 319 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 11.2x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 5.3x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.0 ms | 58.8 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 3.5x faster than the host (was 4.1x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.55 ms | 5.54 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 519 ms | 521 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 24.3x faster than the host (was 25.2x); tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.9 µs | 49.7 µs | -0.2% | [-0.4%, -0.0%] | - | ~ | device 45.5x faster than the host; tape/fused 7.24x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.1 µs | 47.9 µs | -0.5% | [-1.2%, +0.2%] | - | ~ | device 9.7x faster than the host (was 9.2x); tape/fused 9.89x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.4 ms | 17.4 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 7.0x faster than the host (was 8.6x); tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.4 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 5.0x faster than the host (was 5.8x) |
| `Fused/csg/signedDistance/double/hostdevice` | 811 µs | 813 µs | +0.3% | [-0.1%, +0.6%] | - | ~ | device 2.1x faster than the host; tape/fused 1.05x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.7x faster than the host (was 1.6x); tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 282 ms | 285 ms | +1.2% | [+0.8%, +1.6%] | - | ~ | device 0.4x faster than the host; tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 280 ms | 283 ms | +1.3% | [-1.6%, +4.1%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.25 ms | 8.36 ms | +1.2% | [+1.0%, +1.5%] | +0.0% | ~ | device 14.5x faster than the host (was 11.1x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.33 ms | 8.28 ms | -0.7% | [-1.8%, +0.5%] | +0.0% | ~ | device 11.1x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.26 ms | 8.25 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ | device 11.1x faster than the host (was 12.3x); default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.29 ms | 8.23 ms | -0.8% | [-2.2%, +0.6%] | +0.0% | ~ | device 11.2x faster than the host; default/this 1.02x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 701 µs | 703 µs | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | device 4.7x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.74 ms | 1.75 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.03 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.68x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 793 µs | 799 µs | +0.8% | [+0.6%, +1.1%] | +0.0% | ~ | device 4.1x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 830 µs | 829 µs | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 11.7x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 953 µs | 941 µs | -1.3% | [-1.7%, -1.0%] | +0.0% | ~ | device 10.3x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 837 µs | 826 µs | -1.2% | [-1.6%, -0.8%] | +0.0% | ~ | device 11.8x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 801 µs | 807 µs | +0.8% | [-0.1%, +1.7%] | +0.0% | ~ | device 11.9x faster than the host (was 12.3x); default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 162 ms | +1.4% | [-1.4%, +4.3%] | +0.0% | ~ | device 0.7x faster than the host (was 0.6x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 161 ms | 162 ms | +0.6% | [-3.0%, +4.3%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 157 ms | 161 ms | +2.2% | [+0.3%, +4.1%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 160 ms | 158 ms | -0.8% | [-1.4%, -0.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.02x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.59 ms | 1.59 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.86 ms | 2.85 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.56x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.94 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.82x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.69 ms | 1.69 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.30 ms | 7.34 ms | +0.6% | [+0.1%, +1.2%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.42 ms | 7.38 ms | -0.6% | [-1.6%, +0.3%] | +0.0% | ~ | device 1.4x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.29 ms | 7.27 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ | device 1.4x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.34 ms | 7.37 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.32 ms | 8.45 ms | +1.5% | [+0.9%, +2.1%] | - | ~ | device 12.0x faster than the host (was 14.0x) |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.6 ms | 16.6 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 7.5x faster than the host (was 9.5x) |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 168 ms | 165 ms | -1.9% | [-2.7%, -1.2%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 282 ms | 283 ms | +0.3% | [-1.7%, +2.4%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `MeshSDF/query-morton/float/hostdevice` | 24.4 ms | 24.6 ms | +0.9% | [+0.5%, +1.4%] | - | ~ | device 4.2x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 59.9 ms | 59.7 ms | -0.4% | [-1.1%, +0.4%] | - | ~ | device 2.7x faster than the host (was 2.9x) |
| `MeshSDF/query-morton/double/hostdevice` | 157 ms | 156 ms | -0.6% | [-4.2%, +3.1%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 244 ms | 245 ms | +0.7% | [-2.0%, +3.6%] | - | ~ | device 0.7x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 772 µs | 771 µs | -0.2% | [-0.8%, +0.5%] | - | ~ | device 10.7x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.22 ms | 2.22 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 5.8x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.17 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 8.5x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.45 ms | 6.45 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 4.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 4.12 ms | 4.13 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 4.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.09 ms | 7.07 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.81 ms | 7.80 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.5 ms | 20.5 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.5x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.17 ms | 2.17 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 5.0x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.96 ms | 5.96 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 2.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 833 µs | 832 µs | -0.1% | [-0.4%, +0.3%] | - | ~ | device 10.8x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 9.11 ms | 9.12 ms | +0.1% | [-0.9%, +1.0%] | - | ~ | device 4.0x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 6.45 ms | 6.43 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 4.5x faster than the host (was 4.2x) |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.11 ms | 8.18 ms | +0.8% | [-0.0%, +1.6%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 16.0 ms | 16.0 ms | -0.3% | [-0.4%, -0.3%] | - | ~ | device 1.1x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.01 ms | 4.02 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 2.2x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 20.3 ms | 20.4 ms | +0.7% | [+0.3%, +1.1%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.3 ms | 15.0 ms | -1.5% | [-2.8%, -0.2%] | - | ~ | device 1.8x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.51 ms | 1.50 ms | -0.6% | [-2.1%, +1.0%] | - | ~ | device 9.0x faster than the host (was 8.6x) |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.75 ms | 4.75 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 4.1x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.2% | [-0.3%, +0.8%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.40 ms | 3.40 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 5.6x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.60 ms | 4.60 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 4.2x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.4 ms | 13.4 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 17.2 ms | 17.2 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 7.53 ms | 7.52 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 11.3 ms | 11.3 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 2.4x faster than the host (was 1.9x) |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 15.9 ms | 15.9 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 4.86 ms | 4.86 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 16.1 ms | 16.1 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.1x faster than the host (was 2.3x) |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.61 ms | 1.61 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 7.5x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.62 ms | 5.62 ms | +0.0% | [-0.5%, +0.5%] | - | ~ | device 5.0x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 13.4 ms | 13.3 ms | -0.5% | [-1.0%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 34.2 ms | 34.1 ms | -0.1% | [-1.8%, +1.7%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.5 ms | 68.7 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.5 ms | +0.5% | [+0.1%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 21.0 ms | 21.0 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 38.8 ms | 39.2 ms | +1.1% | [+1.0%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 679 µs | 674 µs | -0.7% | [-1.7%, +0.4%] | - | ~ | device 17.2x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.0% | [-0.9%, +1.0%] | - | ~ | device 14.6x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 642 µs | 647 µs | +0.8% | [+0.0%, +1.5%] | - | ~ | device 17.5x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | -0.2% | [-0.4%, +0.0%] | - | ~ | device 15.3x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 613 µs | 612 µs | -0.2% | [-1.3%, +0.9%] | - | ~ | device 16.1x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.11 ms | -0.5% | [-1.2%, +0.1%] | - | ~ | device 15.5x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.19 ms | +0.8% | [+0.1%, +1.5%] | - | ~ | device 12.2x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 10.0x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.3% | [-0.9%, +1.5%] | - | ~ | device 11.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 10.1x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 593 µs | 631 µs | +6.4% | [+5.4%, +7.4%] | - | minor | device 2.4x faster than the host (was 2.5x); tape/fused 1.11x (was 1.19x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 730 µs | 730 µs | -0.0% | [-0.5%, +0.5%] | - | ~ | device 2.6x faster than the host; tape/fused 1.15x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 704 µs | 703 µs | -0.3% | [-0.5%, +0.0%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 840 µs | 837 µs | -0.3% | [-1.2%, +0.5%] | - | ~ | device 3.7x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 16.9 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 14.9 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.7 ms | +0.6% | [+0.2%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.6 ms | 16.7 ms | +0.8% | [+0.1%, +1.4%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.01x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 1.1x faster than the host (was 1.0x); triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.1 ms | 15.0 ms | -0.4% | [-1.4%, +0.6%] | - | ~ | device 0.9x faster than the host (was 1.2x) |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 23.0 ms | -0.6% | [-0.9%, -0.4%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.61 ms | 2.60 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.71 ms | 2.72 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.60 ms | 2.59 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 3.0x faster than the host (was 2.5x); marched/traced 4.29x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.03 ms | 3.03 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.7x faster than the host (was 2.3x); marched/traced 5.74x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 11.1 ms | 11.1 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 3.5x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 17.4 ms | 17.4 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 2.6x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.19 ms | 2.19 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 3.9x faster than the host; marched/traced 18.22x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.49 ms | 5.50 ms | +0.2% | [+0.2%, +0.2%] | - | ~ | device 2.8x faster than the host; marched/traced 14.37x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 39.9 ms | 39.8 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 2.2x faster than the host (was 1.9x) |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 78.6 ms | 79.0 ms | +0.5% | [-0.4%, +1.4%] | - | ~ | device 1.6x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.26 ms | 5.25 ms | -0.2% | [-0.2%, -0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.79 ms | 8.81 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 3.3x faster than the host (was 3.1x) |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.29 ms | 6.27 ms | -0.5% | [-1.2%, +0.3%] | - | ~ | device 1.1x faster than the host (was 1.3x); marched/traced 30.80x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.7 ms | +0.9% | [+0.6%, +1.2%] | - | ~ | device 0.7x faster than the host (was 0.8x); marched/traced 29.57x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 194 ms | 193 ms | -0.6% | [-4.6%, +3.5%] | - | ~ | device 0.2x faster than the host (was 0.3x) |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 313 ms | 315 ms | +0.6% | [+0.2%, +1.1%] | - | ~ | device 0.1x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.86 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 1.8x faster than the host; marched/traced 26.27x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.1 ms | -0.3% | [-0.4%, -0.1%] | - | ~ | device 1.3x faster than the host; marched/traced 20.68x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 129 ms | 128 ms | -1.2% | [-2.8%, +0.3%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 251 ms | 251 ms | -0.0% | [-1.1%, +1.0%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.2 ms | 19.4 ms | +0.8% | [-0.2%, +1.9%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.4% | [-0.8%, -0.1%] | - | ~ | device 1.1x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.65 ms | 3.60 ms | -1.4% | [-1.7%, -1.1%] | - | ~ | device 1.2x faster than the host; marched/traced 5.92x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.76 ms | 4.78 ms | +0.5% | [-0.5%, +1.6%] | - | ~ | device 0.9x faster than the host; marched/traced 8.65x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 21.1 ms | 21.3 ms | +0.9% | [-0.1%, +1.8%] | - | ~ | device 2.3x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 41.5 ms | 41.4 ms | -0.3% | [-0.5%, +0.0%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.25 ms | 1.24 ms | -0.9% | [-2.7%, +1.1%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.85 ms | -0.3% | [-1.7%, +1.1%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 93.2 µs | 96.2 µs | +3.3% | [+2.5%, +4.0%] | - | minor | device 15.2x faster than the host (was 15.7x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 161 µs | 164 µs | +1.6% | [+0.7%, +2.6%] | - | ~ | device 10.1x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 807 µs | 808 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 3.1x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 947 µs | 950 µs | +0.4% | [+0.1%, +0.7%] | - | ~ | device 2.9x faster than the host (was 2.8x) |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.9 ms | 36.1 ms | +3.6% | [+3.1%, +4.1%] | - | minor | device 0.1x faster than the host (was 0.1x); marched/traced 3.56x (was 3.76x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 37.1 ms | 36.5 ms | -1.7% | [-3.0%, -0.3%] | - | ~ | device 0.1x faster than the host; marched/traced 6.65x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 131 ms | 129 ms | -1.9% | [-4.9%, +1.3%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 242 ms | 243 ms | +0.4% | [-1.3%, +2.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.1 ms | 17.0 ms | +5.5% | [+2.9%, +8.3%] | - | minor | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 21.8 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.08 ms | 1.07 ms | -0.7% | [-2.9%, +1.7%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.33 ms | 2.38 ms | +1.7% | [+0.0%, +3.5%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.95 ms | 5.97 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.73 ms | 6.76 ms | +0.4% | [+0.4%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.0 ms | 23.1 ms | +0.5% | [-0.5%, +1.5%] | - | ~ | device 2.2x faster than the host; marched/traced 4.60x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.2 ms | 28.6 ms | +1.5% | [+1.2%, +1.8%] | - | ~ | device 2.0x faster than the host; marched/traced 6.02x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 107 ms | 106 ms | -1.1% | [-3.4%, +1.3%] | - | ~ | device 2.5x faster than the host (was 3.0x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 170 ms | 172 ms | +1.1% | [-1.4%, +3.7%] | - | ~ | device 2.0x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.8 ms | +0.2% | [-1.2%, +1.6%] | - | ~ | device 5.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.2 ms | 17.3 ms | +0.6% | [+0.1%, +1.2%] | - | ~ | device 4.7x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 296 ms | 312 ms | +5.5% | [+4.8%, +6.1%] | - | minor | device 0.2x faster than the host (was 0.3x); marched/traced 2.07x (was 2.18x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 346 ms | 356 ms | +3.0% | [+0.3%, +5.8%] | - | minor | device 0.2x faster than the host; marched/traced 3.08x (was 3.20x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 644 ms | 646 ms | +0.4% | [-1.6%, +2.4%] | - | ~ | device 0.5x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.10 s | 1.10 s | -0.8% | [-2.6%, +1.1%] | - | ~ | device 0.3x faster than the host (was 0.4x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 183 ms | 191 ms | +4.4% | [+1.6%, +7.3%] | - | minor | device 0.4x faster than the host (was 0.5x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 239 ms | 236 ms | -1.1% | [-3.4%, +1.3%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `Render/csg/frame/float/hostdevice` | 4.58 ms | 4.58 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 4.7x faster than the host (was 4.5x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.57 ms | 4.59 ms | +0.4% | [+0.0%, +0.7%] | - | ~ | device 4.3x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.89 ms | 1.90 ms | +0.5% | [-0.5%, +1.6%] | - | ~ | device 14.3x faster than the host (was 12.6x) |
| `Render/csg/frame/double/hostdevice` | 8.10 ms | 8.12 ms | +0.2% | [+0.0%, +0.5%] | - | ~ | device 2.7x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 8.10 ms | 8.10 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.5x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.3 ms | 23.4 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 0.9x faster than the host (was 1.0x) |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 217 µs | +0.0% | [-0.3%, +0.4%] | - | ~ | device 14.8x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 47.9 µs | +0.3% | [+0.1%, +0.6%] | - | ~ | device 29.9x faster than the host (was 26.8x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 361 µs | 360 µs | -0.2% | [-0.4%, -0.1%] | - | ~ | device 14.3x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.00 ms | 2.00 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 8.9x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.5 µs | 72.4 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 8.6x faster than the host (was 9.2x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 48.3 µs | 48.8 µs | +0.9% | [-0.5%, +2.4%] | - | ~ | device 5.5x faster than the host (was 6.6x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 652 µs | +0.1% | [-0.1%, +0.2%] | - | ~ | device 13.7x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 13.7x faster than the host (was 13.3x) |
| `Tape/chain16/signedDistance/float/hostdevice` | 473 µs | 474 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 9.4x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 48.0 µs | +0.4% | [+0.4%, +0.5%] | - | ~ | device 30.3x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 855 µs | 854 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 4.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 811 µs | 810 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 3.6x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.71 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 2.6x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 190 µs | -0.2% | [-0.3%, -0.2%] | - | ~ | device 3.4x faster than the host (was 3.2x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 2.6x faster than the host (was 2.4x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.1x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |


The probes differ between the sides (each side ran its own): Performance/RayCSG/main.cpp, Performance/Common/Device.hpp

<!-- END receipt cuda -->
