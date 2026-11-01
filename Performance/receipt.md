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
base: 3db951f
base-fingerprint: 842404e655765c0b7f2428767c0e13ed1423660b
head: e1f44f2
head-fingerprint: ad70aa3b84036fd709a9c01dad59244c3dc1cf44
cases: 164
rounds: 2
reps: 3
date: 2026-11-01
-->

## Host comparison: yellow

**Accepted:** -

`e1f44f2` against `3db951f` (origin/main), 2026-11-01.

**VERDICT: 4 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 4 minor, 4 faster, 0 differ, 0 lost, 164 cases

- base: 3db951f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: e1f44f2 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 329 ms | 329 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 178 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 514 ms | 516 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 344 ms | 346 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 185 ms | 185 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 561 ms | 563 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.1 ms | 55.0 ms | -0.1% | [-0.6%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.2 ms | 51.2 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.1 ms | 63.9 ms | -0.3% | [-1.0%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.5 ms | 54.2 ms | -0.5% | [-1.0%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.2 ms | 60.7 ms | +0.8% | [-0.2%, +1.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.1 ms | 72.2 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.1 ms | 74.0 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.7 ms | 77.5 ms | -0.3% | [-1.2%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.6 ms | 66.8 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.8 ms | 99.2 ms | -0.6% | [-0.9%, -0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.9 ms | 82.4 ms | -0.5% | [-1.5%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 108 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.1 ms | 19.1 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.4 ms | 45.5 ms | +0.4% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.7 ms | 72.6 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.2 ms | 20.2 ms | +0.2% | [-1.0%, +1.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.8 ms | 45.9 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 83.0 ms | 84.3 ms | +1.7% | [-0.5%, +3.9%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.45 ms | 4.37 ms | -1.8% | [-2.5%, -1.2%] | - | ~ | tape/fused 2.84x |
| `Fused/smooth/signedDistance/float/host` | 12.2 ms | 12.2 ms | +0.0% | [-1.6%, +1.7%] | - | ~ | tape/fused 1.96x |
| `Fused/chain16/signedDistance/float/host` | 970 µs | 952 µs | -1.8% | [-2.0%, -1.6%] | - | ~ | tape/fused 16.65x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.3 ms | 46.6 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ | tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.2 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.53 ms | 4.57 ms | +0.8% | [-0.6%, +2.3%] | - | ~ | tape/fused 2.82x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.7 ms | -0.3% | [-0.5%, +0.0%] | - | ~ | tape/fused 1.97x |
| `Fused/chain16/signedDistance/double/host` | 3.86 ms | 3.88 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | tape/fused 4.11x |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.3 ms | 52.9 ms | -0.8% | [-1.5%, -0.1%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.7 ms | 53.3 ms | -0.7% | [-1.6%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.9 ms | 81.6 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 229 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 265 ms | 264 ms | -0.3% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.0 ms | 70.0 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 212 ms | 212 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 95.6 ms | 96.1 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.1 ms | 29.8 ms | -0.9% | [-1.4%, -0.3%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 758 ms | 757 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | +0.4% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.3 ms | 43.3 ms | +0.0% | [-1.7%, +1.8%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 847 ms | 843 ms | -0.5% | [-1.5%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | - | - | - | - | - | new |  |
| `MeshCSG/union-tape/frame/float/host` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-tape/frame/float/host` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | - | - | - | - | - | new | tape/fused 3.51x |
| `MeshCSG/smooth-fused/frame/float/host` | - | - | - | - | - | new | tape/fused 2.38x |
| `MeshCSG/union-tape/firstHit/double/host` | - | - | - | - | - | new |  |
| `MeshCSG/union-tape/frame/double/host` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-tape/frame/double/host` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | - | - | - | - | - | new | tape/fused 3.65x |
| `MeshCSG/smooth-fused/frame/double/host` | - | - | - | - | - | new | tape/fused 2.39x |
| `PointCloudBVH/uniform/build/float/host` | 68.4 ms | 67.9 ms | -0.7% | [-1.6%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 147 ms | 147 ms | +0.1% | [-0.3%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 369 ms | 371 ms | +0.5% | [+0.2%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.0 ms | 78.7 ms | +0.9% | [-0.1%, +1.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.58 ms | 6.61 ms | +0.6% | [-0.3%, +1.5%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 80.5 ms | 80.3 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 160 ms | 159 ms | -0.1% | [-1.4%, +1.2%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 400 ms | 399 ms | -0.3% | [-0.4%, -0.2%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 86.4 ms | 87.1 ms | +0.8% | [+0.3%, +1.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.19 ms | 7.21 ms | +0.2% | [-0.5%, +0.9%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.6 ms | 11.3 ms | -2.0% | [-4.4%, +0.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 123 ms | 123 ms | +0.1% | [-0.3%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 516 ms | 515 ms | -0.2% | [-0.3%, -0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 152 ms | +0.2% | [+0.2%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 837 ms | 837 ms | -0.0% | [-0.3%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 14.9 ms | 14.9 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 127 ms | 126 ms | -0.4% | [-0.9%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 545 ms | 545 ms | +0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 154 ms | 155 ms | +0.5% | [-0.2%, +1.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 888 ms | 888 ms | -0.0% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.2 ms | 67.7 ms | +0.8% | [-0.2%, +1.8%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 109 ms | 109 ms | +0.2% | [-0.7%, +1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 270 ms | 269 ms | -0.3% | [-1.0%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 111 ms | 111 ms | +0.3% | [-0.2%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 106 ms | 106 ms | +0.5% | [+0.2%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 79.9 ms | 79.9 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 119 ms | 120 ms | +0.9% | [+0.6%, +1.1%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 294 ms | 295 ms | +0.3% | [+0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 127 ms | 127 ms | -0.0% | [-0.3%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 121 ms | 121 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.74 ms | 8.79 ms | +0.6% | [-1.1%, +2.3%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 144 ms | 141 ms | -1.9% | [-4.0%, +0.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 393 ms | 393 ms | -0.0% | [-3.4%, +3.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 250 ms | 251 ms | +0.1% | [+0.0%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 333 ms | 334 ms | +0.2% | [-0.6%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.4 ms | 12.5 ms | +1.2% | [-0.6%, +3.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 144 ms | 144 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 418 ms | 418 ms | +0.1% | [-0.1%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 284 ms | 287 ms | +1.1% | [+0.2%, +1.9%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 345 ms | 344 ms | -0.3% | [-0.6%, +0.0%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.3 ms | 46.5 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.7 ms | 45.6 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.3 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 77.4 ms | 77.0 ms | -0.5% | [-0.9%, -0.2%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/float/host` | 65.0 ms | 65.5 ms | +0.8% | [+0.4%, +1.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.91 ms | 5.85 ms | -1.0% | [-3.1%, +1.2%] | - | ~ | tape/fused 1.30x |
| `Ray/csg-tape/firstHit/float/host` | 7.79 ms | 7.59 ms | -2.7% | [-3.3%, -2.0%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 51.1 ms | 51.2 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.0 ms | 50.6 ms | -0.7% | [-1.0%, -0.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.5 ms | 44.5 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 86.4 ms | 86.7 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/double/host` | 73.8 ms | 72.9 ms | -1.3% | [-1.5%, -1.0%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.98 ms | 5.94 ms | -0.6% | [-2.1%, +0.8%] | - | ~ | tape/fused 1.40x |
| `Ray/csg-tape/firstHit/double/host` | 8.21 ms | 8.32 ms | +1.3% | [+0.7%, +1.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.2 ms | 17.8 ms | -2.1% | [-6.9%, +3.0%] | - | ~ | marched/traced 6.70x (was 6.42x) |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 117 ms | 119 ms | +2.3% | [+2.0%, +2.6%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 22.2 ms | +0.8% | [-0.0%, +1.6%] | +0.0% | ~ | marched/traced 7.16x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 159 ms | 159 ms | -0.2% | [-3.6%, +3.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.7 ms | 35.8 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.1 ms | 20.6 ms | +7.9% | [+5.1%, +10.6%] | - | minor | marched/traced 6.07x (was 6.50x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 124 ms | 125 ms | +0.7% | [+0.2%, +1.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.1 ms | 23.5 ms | +1.7% | [+1.7%, +1.7%] | +0.0% | ~ | marched/traced 6.80x (was 7.19x) |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 167 ms | 160 ms | -3.9% | [-4.0%, -3.8%] | +0.0% | faster |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.6 ms | 37.7 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | marched/traced 4.64x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.6 ms | 51.4 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.01 ms | 8.02 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.84 ms | 1.93 ms | +4.9% | [+4.6%, +5.3%] | +0.0% | minor |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.81 ms | 6.79 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.6 ms | 11.6 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ | marched/traced 5.27x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.3 ms | 61.0 ms | -0.5% | [-1.6%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.62 ms | 8.57 ms | -0.6% | [-1.5%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.09 ms | 1.93 ms | -7.9% | [-8.6%, -7.2%] | +0.0% | faster |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.04 ms | 7.21 ms | +2.4% | [+1.0%, +3.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 241 ms | 240 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 337 ms | 327 ms | -2.8% | [-3.3%, -2.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 240 ms | 237 ms | -1.0% | [-1.6%, -0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 252 ms | 251 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ | marched/traced 1.46x (was 1.52x) |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 383 ms | 366 ms | -4.6% | [-5.2%, -3.9%] | +0.0% | faster |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 252 ms | 249 ms | -1.0% | [-1.6%, -0.5%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.8 ms | 23.7 ms | -0.6% | [-2.1%, +1.1%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.6 ms | 23.8 ms | +0.9% | [+0.0%, +1.8%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 18.5 ms | -0.2% | [-1.7%, +1.2%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 28.6 ms | 29.7 ms | +3.7% | [+3.6%, +3.8%] | +0.0% | minor |  |
| `Render/csg/frame-orthographic/double/host` | 28.8 ms | 29.9 ms | +3.8% | [+1.9%, +5.9%] | +0.0% | minor |  |
| `Render/mesh/frame/double/host` | 21.3 ms | 20.6 ms | -3.2% | [-4.5%, -1.8%] | +0.0% | faster |  |
| `Tape/csg/signedDistance/float/host` | 12.4 ms | 12.4 ms | -0.3% | [-1.0%, +0.3%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.54 ms | 4.57 ms | +0.6% | [-0.5%, +1.7%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.0 ms | 24.0 ms | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.2 ms | 40.2 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.73 ms | 1.73 ms | -0.0% | [-1.2%, +1.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 132 µs | 132 µs | -0.2% | [-9.7%, +10.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.0 ms | 29.9 ms | -0.5% | [-0.9%, -0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.65 ms | 5.63 ms | -0.4% | [-1.2%, +0.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 15.8 ms | 15.8 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.63 ms | 4.62 ms | -0.3% | [-1.1%, +0.4%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 12.9 ms | 12.9 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.56 ms | 4.59 ms | +0.6% | [-0.1%, +1.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 25.1 ms | 25.1 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.1 ms | 42.0 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.72 ms | 1.75 ms | +2.0% | [+0.7%, +3.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 259 µs | 258 µs | -0.5% | [-1.6%, +0.6%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.9 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.77 ms | 7.72 ms | -0.7% | [-1.8%, +0.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.0 ms | 16.0 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.74 ms | 7.75 ms | +0.0% | [-0.5%, +0.6%] | - | ~ |  |


The probes differ between the sides (each side ran its own): Performance/MeshCSG/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 3db951f
base-fingerprint: 842404e655765c0b7f2428767c0e13ed1423660b
head: e1f44f2
head-fingerprint: ad70aa3b84036fd709a9c01dad59244c3dc1cf44
cases: 236
rounds: 2
reps: 3
date: 2026-11-01
-->

## Device comparison (CUDA): yellow

**Accepted:** -

`e1f44f2` against `3db951f` (origin/main), 2026-11-01.

**VERDICT: 2 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 2 minor, 16 faster, 0 differ, 0 lost, 236 cases

- base: 3db951f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: e1f44f2 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.61 ms | 1.61 ms | +0.2% | [-0.3%, +0.8%] | - | ~ | device 22.7x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.43 ms | 3.41 ms | -0.5% | [-1.0%, +0.0%] | - | ~ | device 18.6x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.05 ms | 1.05 ms | -0.3% | [-2.8%, +2.2%] | - | ~ | device 19.6x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.30 ms | 2.31 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 16.3x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.26 ms | 6.26 ms | +0.1% | [-0.8%, +1.0%] | - | ~ | device 7.2x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.8 ms | -1.1% | [-2.4%, +0.3%] | - | ~ | device 5.4x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.3 ms | 33.3 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.2 ms | 56.9 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.7 ms | 20.4 ms | -1.5% | [-2.5%, -0.4%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.7 ms | 47.9 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.3 ms | 61.1 ms | -0.4% | [-1.4%, +0.5%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 108 ms | 110 ms | +1.6% | [-0.8%, +4.0%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 385 µs | 386 µs | +0.3% | [-0.1%, +0.8%] | - | ~ | device 14.9x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 846 µs | 846 µs | +0.0% | [-0.2%, +0.2%] | - | ~ | device 9.8x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.52 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.65 ms | 4.67 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 593 µs | 587 µs | -1.0% | [-2.0%, -0.1%] | - | ~ | device 12.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 8.2x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.76 ms | 6.77 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.00 ms | 7.02 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 902 µs | 902 µs | +0.0% | [-0.4%, +0.5%] | - | ~ | device 14.3x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.23 ms | 2.24 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 7.8x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.49 ms | 9.46 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 319 µs | 319 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 11.1x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 5.1x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.0 ms | 58.8 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 3.4x faster than the host (was 4.0x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.54 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 520 ms | 519 ms | -0.2% | [-0.8%, +0.5%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `Fused/csg/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 23.7x faster than the host (was 24.7x); tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.8 µs | 49.7 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 44.1x faster than the host; tape/fused 7.24x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.3% | [+0.0%, +0.6%] | - | ~ | device 10.0x faster than the host (was 8.9x); tape/fused 9.88x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.5 ms | 17.4 ms | -0.8% | [-1.3%, -0.3%] | - | ~ | device 6.8x faster than the host; tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.3 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.05x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.11x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 283 ms | 285 ms | +0.4% | [-1.5%, +2.4%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 283 ms | 287 ms | +1.2% | [+0.0%, +2.5%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.31 ms | 8.27 ms | -0.5% | [-1.4%, +0.4%] | +0.0% | ~ | device 10.9x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.30 ms | 8.23 ms | -0.8% | [-1.5%, -0.1%] | +0.0% | ~ | device 10.9x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.32 ms | 8.25 ms | -0.9% | [-2.6%, +0.9%] | +0.0% | ~ | device 10.9x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.28 ms | 8.28 ms | -0.0% | [-3.5%, +3.6%] | +0.0% | ~ | device 12.6x faster than the host (was 10.8x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 706 µs | 712 µs | +0.8% | [+0.6%, +0.9%] | +0.0% | ~ | device 4.6x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.75 ms | 1.74 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ | device 1.9x faster than the host (was 1.8x); default/this 0.41x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.04 ms | +0.9% | [+0.4%, +1.4%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.68x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 802 µs | 839 µs | +4.6% | [+4.1%, +5.0%] | +0.0% | minor | device 3.9x faster than the host (was 4.1x); default/this 0.85x (was 0.88x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 831 µs | 827 µs | -0.5% | [-1.3%, +0.3%] | +0.0% | ~ | device 11.6x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 953 µs | 946 µs | -0.8% | [-1.8%, +0.3%] | +0.0% | ~ | device 10.1x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 827 µs | 826 µs | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | device 11.6x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 808 µs | 805 µs | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ | device 11.7x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 159 ms | 159 ms | -0.2% | [-3.6%, +3.3%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 163 ms | 163 ms | -0.4% | [-3.0%, +2.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 159 ms | 164 ms | +3.1% | [-0.6%, +6.7%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.97x (was 1.00x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 166 ms | 164 ms | -1.3% | [-3.5%, +1.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.97x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.60 ms | 1.60 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.84 ms | 1.76 ms | -37.8% | [-38.0%, -37.7%] | +0.0% | faster | device 2.1x faster than the host (was 1.3x); default/this 0.91x (was 0.56x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.94 ms | 1.94 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.83x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.69 ms | 1.68 ms | -0.2% | [-0.5%, -0.0%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.28 ms | 7.32 ms | +0.6% | [-0.1%, +1.2%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.40 ms | 7.33 ms | -1.1% | [-1.8%, -0.3%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.28 ms | 7.28 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.33 ms | 7.35 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.35 ms | 8.41 ms | +0.7% | [-0.1%, +1.5%] | - | ~ | device 11.4x faster than the host (was 16.5x) |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.7 ms | 16.6 ms | -0.5% | [-1.9%, +0.9%] | - | ~ | device 8.3x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 163 ms | 168 ms | +3.0% | [-2.1%, +8.4%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 283 ms | 279 ms | -1.7% | [-2.3%, -1.0%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.5 ms | 24.5 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 59.9 ms | 60.0 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 2.4x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 159 ms | 157 ms | -0.7% | [-1.6%, +0.2%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 243 ms | 243 ms | -0.0% | [-1.1%, +1.0%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/union-tape/frame/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | - | - | - | - | - | new | tape/fused 4.89x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | - | - | - | - | - | new | tape/fused 6.43x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | - | - | - | - | - | new | tape/fused 5.31x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/union-tape/frame/double/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | - | - | - | - | - | new |  |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | - | - | - | - | - | new | tape/fused 2.86x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | - | - | - | - | - | new | tape/fused 2.74x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | - | - | - | - | - | new | tape/fused 5.57x |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 765 µs | 769 µs | +0.6% | [-0.0%, +1.1%] | - | ~ | device 10.6x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.22 ms | 2.22 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.17 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 8.4x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.45 ms | 6.46 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 4.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 4.11 ms | 4.13 ms | +0.4% | [+0.2%, +0.5%] | - | ~ | device 4.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.09 ms | 7.09 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.85 ms | 7.81 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.5 ms | 20.5 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 1.5x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.17 ms | 2.17 ms | -0.1% | [-1.1%, +0.9%] | - | ~ | device 4.9x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.95 ms | 5.96 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 835 µs | 832 µs | -0.3% | [-0.6%, +0.1%] | - | ~ | device 10.5x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 9.04 ms | 9.01 ms | -0.4% | [-1.1%, +0.2%] | - | ~ | device 3.9x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 6.48 ms | 6.46 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 4.2x faster than the host |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.11 ms | 8.15 ms | +0.5% | [+0.3%, +0.7%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 16.0 ms | 16.1 ms | +0.6% | [+0.2%, +1.0%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.04 ms | 4.03 ms | -0.1% | [-0.8%, +0.5%] | - | ~ | device 2.2x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 20.4 ms | 20.4 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.1 ms | 15.1 ms | +0.0% | [-0.7%, +0.7%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.51 ms | 1.50 ms | -0.2% | [-1.4%, +1.0%] | - | ~ | device 8.6x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.76 ms | 4.75 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.7% | [+0.3%, +1.1%] | - | ~ | device 5.4x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.40 ms | 3.40 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 5.4x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.62 ms | 4.63 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 4.1x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.5 ms | 13.3 ms | -0.8% | [-1.6%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 17.2 ms | 17.1 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 7.55 ms | 7.52 ms | -0.4% | [-0.8%, +0.0%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 11.3 ms | 11.3 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 15.9 ms | 15.9 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 4.88 ms | 4.86 ms | -0.4% | [-0.6%, -0.2%] | - | ~ | device 3.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 16.1 ms | 16.1 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.62 ms | 1.62 ms | +0.0% | [-0.6%, +0.7%] | - | ~ | device 7.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.64 ms | 5.62 ms | -0.3% | [-0.5%, -0.2%] | - | ~ | device 4.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 13.3 ms | 13.4 ms | +0.6% | [+0.4%, +0.7%] | - | ~ | device 1.9x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 33.9 ms | 33.9 ms | -0.0% | [-0.7%, +0.7%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.3 ms | 68.6 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.5 ms | 18.6 ms | +0.8% | [+0.2%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 21.0 ms | 21.0 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 39.2 ms | 39.2 ms | +0.1% | [-0.8%, +0.9%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 668 µs | 677 µs | +1.4% | [+0.2%, +2.6%] | - | ~ | device 16.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 14.6x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 643 µs | 640 µs | -0.4% | [-1.6%, +0.9%] | - | ~ | device 17.3x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | device 15.3x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 611 µs | 608 µs | -0.6% | [-4.2%, +3.3%] | - | ~ | device 15.9x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | +0.2% | [-1.1%, +1.6%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.18 ms | -0.6% | [-1.2%, +0.1%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 11.6x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.19 ms | 2.18 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 9.9x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 598 µs | 612 µs | +2.2% | [-0.9%, +5.3%] | - | ~ | device 2.5x faster than the host; tape/fused 1.16x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 664 µs | 730 µs | +9.9% | [+9.3%, +10.5%] | - | minor | device 2.5x faster than the host (was 2.8x); tape/fused 1.15x (was 1.27x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 704 µs | 707 µs | +0.4% | [-0.0%, +0.8%] | - | ~ | device 3.7x faster than the host (was 3.6x) |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 840 µs | 843 µs | +0.3% | [-0.3%, +0.9%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.2% | [-1.1%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.5% | [-0.9%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.8% | [-2.0%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.3% | [-1.7%, +1.1%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.4 ms | 24.4 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | +0.2% | [-0.6%, +1.1%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | +0.0% | [-0.7%, +0.7%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.59 ms | 2.62 ms | +1.0% | [+0.0%, +2.1%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 1.7x faster than the host (was 1.7x) |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.72 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.59 ms | 2.61 ms | +0.8% | [+0.7%, +0.9%] | - | ~ | device 3.0x faster than the host; marched/traced 3.46x (was 4.29x) |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.03 ms | 3.04 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 2.7x faster than the host; marched/traced 4.57x (was 5.72x) |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 11.1 ms | 9.03 ms | -18.8% | [-19.2%, -18.5%] | - | faster | device 4.4x faster than the host (was 3.4x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 17.4 ms | 13.9 ms | -20.0% | [-20.1%, -19.9%] | - | faster | device 3.2x faster than the host (was 2.5x) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.17 ms | -0.9% | [-1.1%, -0.7%] | - | ~ | device 3.9x faster than the host; marched/traced 16.57x (was 18.32x) |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.48 ms | 5.42 ms | -1.2% | [-1.4%, -1.0%] | - | ~ | device 2.8x faster than the host; marched/traced 13.19x (was 14.44x) |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 40.0 ms | 35.9 ms | -10.4% | [-10.4%, -10.3%] | - | faster | device 2.0x faster than the host (was 1.9x) |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 79.1 ms | 71.4 ms | -9.7% | [-10.1%, -9.3%] | - | faster | device 1.7x faster than the host (was 1.5x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.25 ms | 5.24 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.81 ms | 8.79 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.37 ms | 6.26 ms | -1.7% | [-2.2%, -1.1%] | - | ~ | device 1.3x faster than the host (was 1.1x); marched/traced 25.97x (was 30.04x) |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 0.8x faster than the host (was 0.7x); marched/traced 24.29x (was 29.46x) |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 191 ms | 163 ms | -15.0% | [-17.3%, -12.5%] | - | faster | device 0.3x faster than the host (was 0.2x) |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 313 ms | 258 ms | -17.5% | [-18.8%, -16.1%] | - | faster | device 0.2x faster than the host (was 0.1x) |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.86 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 1.8x faster than the host; marched/traced 22.25x (was 27.33x) |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.2 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.3x faster than the host; marched/traced 17.80x (was 20.83x) |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 133 ms | 108 ms | -18.4% | [-18.8%, -18.1%] | - | faster | device 0.7x faster than the host (was 0.6x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 253 ms | 217 ms | -14.4% | [-15.9%, -12.8%] | - | faster | device 0.6x faster than the host (was 0.5x) |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.4 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.7 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.62 ms | 3.53 ms | -2.4% | [-5.5%, +0.7%] | - | ~ | device 1.2x faster than the host; marched/traced 5.60x (was 5.81x) |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.79 ms | 4.79 ms | -0.1% | [-1.3%, +1.1%] | - | ~ | device 0.9x faster than the host; marched/traced 8.39x (was 8.66x) |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 21.0 ms | 19.8 ms | -5.9% | [-6.2%, -5.7%] | - | faster | device 2.4x faster than the host (was 2.3x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 41.5 ms | 40.2 ms | -3.3% | [-3.8%, -2.8%] | - | faster | device 1.4x faster than the host (was 1.3x) |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.26 ms | 1.25 ms | -0.7% | [-1.6%, +0.1%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.84 ms | 1.85 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 95.5 µs | 92.8 µs | -2.9% | [-4.0%, -1.9%] | - | ~ | device 15.5x faster than the host (was 14.8x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 162 µs | 151 µs | -6.8% | [-7.4%, -6.2%] | - | faster | device 10.9x faster than the host (was 10.3x) |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 806 µs | 800 µs | -0.7% | [-0.9%, -0.5%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 948 µs | 934 µs | -1.5% | [-1.7%, -1.2%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.9 ms | 35.3 ms | -1.7% | [-5.1%, +1.8%] | - | ~ | device 0.1x faster than the host; marched/traced 3.50x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.3 ms | 37.0 ms | +1.8% | [+0.3%, +3.4%] | - | ~ | device 0.1x faster than the host; marched/traced 6.41x (was 6.66x) |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 127 ms | 123 ms | -2.6% | [-3.5%, -1.6%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 242 ms | 237 ms | -2.0% | [-6.0%, +2.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 15.9 ms | 16.1 ms | +1.7% | [-0.6%, +3.9%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 22.0 ms | 21.8 ms | -0.7% | [-1.2%, -0.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.04 ms | 1.07 ms | +2.2% | [-2.0%, +6.5%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.33 ms | 2.34 ms | +0.3% | [-0.9%, +1.6%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.99 ms | 5.95 ms | -0.6% | [-1.0%, -0.3%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.69 ms | 6.70 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.2 ms | 23.0 ms | -0.9% | [-2.2%, +0.4%] | - | ~ | device 2.2x faster than the host; marched/traced 4.39x (was 4.67x) |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.2 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 1.9x faster than the host; marched/traced 5.54x (was 6.05x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 108 ms | 101 ms | -6.9% | [-8.9%, -4.9%] | - | faster | device 2.2x faster than the host (was 2.1x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 171 ms | 156 ms | -8.7% | [-11.7%, -5.6%] | - | faster | device 1.5x faster than the host (was 1.8x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.7 ms | -0.8% | [-1.6%, +0.1%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.4 ms | +0.3% | [-0.8%, +1.4%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 307 ms | 318 ms | +3.3% | [-2.3%, +9.4%] | - | ~ | device 0.2x faster than the host; marched/traced 1.77x (was 2.06x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 364 ms | 353 ms | -2.9% | [-4.1%, -1.7%] | - | ~ | device 0.2x faster than the host; marched/traced 2.82x (was 3.05x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 633 ms | 561 ms | -11.3% | [-13.2%, -9.4%] | - | faster | device 0.5x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.11 s | 995 ms | -10.2% | [-10.8%, -9.6%] | - | faster | device 0.2x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 193 ms | 188 ms | -2.5% | [-5.2%, +0.3%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 238 ms | 239 ms | +0.6% | [-2.8%, +4.3%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.57 ms | 4.63 ms | +1.5% | [+1.4%, +1.5%] | - | ~ | device 2.9x faster than the host (was 2.8x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.58 ms | 4.63 ms | +1.2% | [+1.1%, +1.2%] | - | ~ | device 2.9x faster than the host (was 2.8x) |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.89 ms | +0.9% | [-0.1%, +1.9%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 8.11 ms | 8.12 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 8.10 ms | 8.12 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.4 ms | 23.4 ms | -0.0% | [-1.5%, +1.6%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 217 µs | -0.2% | [-0.7%, +0.4%] | - | ~ | device 11.2x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | -0.1% | [-0.3%, +0.2%] | - | ~ | device 23.3x faster than the host (was 24.1x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 360 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.00 ms | 1.99 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 7.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.1 µs | 72.1 µs | +0.0% | [-0.4%, +0.4%] | - | ~ | device 7.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 47.4 µs | -0.7% | [-1.5%, +0.1%] | - | ~ | device 5.1x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 652 µs | -0.1% | [-0.2%, +0.1%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 12.0x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 474 µs | 473 µs | -0.0% | [-0.3%, +0.2%] | - | ~ | device 7.0x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.2% | [-0.4%, -0.1%] | - | ~ | device 27.3x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 853 µs | 853 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [+0.0%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.72 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 191 µs | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.0x faster than the host (was 2.8x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.77 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |


The probes differ between the sides (each side ran its own): Performance/MeshCSG/main.cpp

<!-- END receipt cuda -->
