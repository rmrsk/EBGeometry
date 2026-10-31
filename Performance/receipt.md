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
base: 8d784d1
base-fingerprint: 39675652ec063ec2cbaa9642714f87d3a7af3d99
head: 4c78c1f
head-fingerprint: 4ab6011b1e8931edacd0f1fdeea3a52292c20223
cases: 152
rounds: 2
reps: 3
date: 2026-10-31
-->

## Host comparison: red

**Accepted:** Accept as noise

`4c78c1f` against `8d784d1` (origin/dev), 2026-10-31.

**VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 1 major, 2 minor, 4 faster, 0 differ, 0 lost, 152 cases

- base: 8d784d1 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 4c78c1f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 329 ms | 329 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 178 ms | 177 ms | -0.3% | [-0.3%, -0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 514 ms | 515 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 344 ms | 345 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 185 ms | 189 ms | +2.2% | [-0.3%, +4.8%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 562 ms | 568 ms | +1.2% | [+0.2%, +2.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 54.9 ms | 54.9 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.2 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 63.7 ms | 63.9 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.7 ms | 54.5 ms | -0.4% | [-1.6%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.1 ms | 60.1 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.1 ms | 72.1 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.2 ms | 74.0 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.7 ms | 77.5 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.4 ms | 66.7 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.4 ms | 99.9 ms | +0.5% | [-0.0%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.8 ms | 82.8 ms | +0.1% | [-0.9%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 108 ms | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.2 ms | -1.2% | [-1.9%, -0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 45.7 ms | 44.5 ms | -2.6% | [-6.0%, +0.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.0 ms | 72.5 ms | -0.7% | [-1.1%, -0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.7 ms | 20.3 ms | -2.3% | [-3.6%, -0.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.9 ms | 48.7 ms | +6.1% | [+0.1%, +12.1%] | +0.0% | minor |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.5 ms | 83.6 ms | +1.4% | [-1.7%, +4.6%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.46 ms | 4.40 ms | -1.4% | [-2.3%, -0.4%] | - | ~ | tape/fused 2.82x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | tape/fused 1.94x |
| `Fused/chain16/signedDistance/float/host` | 962 µs | 967 µs | +0.6% | [-1.5%, +2.7%] | - | ~ | tape/fused 16.27x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.2 ms | 45.9 ms | -0.7% | [-1.0%, -0.4%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.4 ms | 46.3 ms | -0.3% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.51 ms | 4.54 ms | +0.7% | [-0.6%, +1.9%] | - | ~ | tape/fused 2.82x |
| `Fused/smooth/signedDistance/double/host` | 12.9 ms | 12.8 ms | -0.9% | [-1.2%, -0.5%] | - | ~ | tape/fused 1.96x |
| `Fused/chain16/signedDistance/double/host` | 3.87 ms | 3.89 ms | +0.5% | [-0.3%, +1.4%] | - | ~ | tape/fused 4.10x |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.2 ms | 53.3 ms | +0.2% | [-0.7%, +1.1%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.0 ms | 53.1 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 81.7 ms | 81.7 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 228 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 112 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 266 ms | 264 ms | -0.7% | [-3.0%, +1.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.5 ms | 70.3 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 214 ms | 216 ms | +0.9% | [+0.4%, +1.3%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.3 ms | 96.1 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.7 ms | 30.6 ms | +3.2% | [+0.8%, +5.6%] | - | minor |  |
| `MeshSDF/query-random/float/host` | 772 ms | 764 ms | -1.0% | [-2.3%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.8 ms | 43.9 ms | -2.1% | [-5.2%, +1.2%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 850 ms | 846 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 67.9 ms | 68.0 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 147 ms | 148 ms | +0.1% | [-0.3%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 371 ms | 372 ms | +0.3% | [+0.0%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.3 ms | 78.1 ms | -0.2% | [-0.9%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.66 ms | 6.60 ms | -1.0% | [-1.1%, -0.8%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 81.0 ms | 80.4 ms | -0.7% | [-1.3%, -0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 160 ms | 159 ms | -0.7% | [-1.7%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 412 ms | 400 ms | -2.7% | [-5.8%, +0.5%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.1 ms | 87.6 ms | +0.5% | [+0.5%, +0.6%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.15 ms | 7.17 ms | +0.3% | [-0.5%, +1.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 11.8 ms | 10.7 ms | -9.4% | [-17.7%, -0.9%] | +0.0% | faster |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 123 ms | 123 ms | -0.2% | [-0.7%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 516 ms | 516 ms | -0.1% | [-0.8%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 152 ms | 152 ms | +0.3% | [+0.0%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 842 ms | 839 ms | -0.3% | [-0.7%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 15.0 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 126 ms | 128 ms | +0.9% | [+0.1%, +1.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 546 ms | 546 ms | -0.1% | [-0.4%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 155 ms | 157 ms | +1.4% | [+0.4%, +2.3%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 890 ms | 894 ms | +0.4% | [+0.2%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.4 ms | 67.1 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 109 ms | 109 ms | +0.1% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 270 ms | 269 ms | -0.1% | [-0.4%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 111 ms | 112 ms | +0.2% | [+0.0%, +0.4%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 106 ms | 106 ms | +0.2% | [-0.2%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 79.7 ms | 80.4 ms | +0.8% | [+0.2%, +1.4%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 120 ms | 120 ms | -0.1% | [-0.6%, +0.3%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 295 ms | 294 ms | -0.3% | [-0.4%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 127 ms | 127 ms | +0.1% | [-0.3%, +0.6%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 122 ms | 122 ms | +0.2% | [-0.1%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 9.58 ms | 8.84 ms | -7.7% | [-14.6%, +0.4%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 144 ms | 144 ms | +0.1% | [-4.0%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 399 ms | 399 ms | +0.1% | [+0.0%, +0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 250 ms | 250 ms | +0.0% | [-0.2%, +0.3%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 332 ms | 333 ms | +0.1% | [-0.7%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.5 ms | 12.3 ms | -1.6% | [-3.3%, +0.2%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 147 ms | 147 ms | +0.0% | [-4.2%, +4.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 425 ms | 418 ms | -1.7% | [-3.2%, -0.2%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 283 ms | 286 ms | +1.1% | [+0.4%, +1.8%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 349 ms | 344 ms | -1.4% | [-2.6%, -0.2%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.4 ms | 46.6 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.7 ms | 45.9 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.6 ms | 40.4 ms | -0.4% | [-0.5%, -0.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.5 ms | 77.3 ms | -1.6% | [-2.9%, -0.2%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/float/host` | 65.3 ms | 65.3 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.87 ms | 5.82 ms | -0.9% | [-1.8%, +0.0%] | - | ~ | tape/fused 1.35x |
| `Ray/csg-tape/firstHit/float/host` | 7.82 ms | 7.82 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 51.5 ms | 51.2 ms | -0.7% | [-1.2%, -0.1%] | +0.0% | ~ | hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.1 ms | 51.2 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.7 ms | 44.6 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 86.8 ms | 86.2 ms | -0.7% | [-1.2%, -0.1%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/double/host` | 73.5 ms | 74.1 ms | +0.8% | [-1.3%, +2.9%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 9.09 ms | 5.92 ms | -34.8% | [-51.6%, -0.4%] | - | faster | tape/fused 1.40x (was 0.91x) |
| `Ray/csg-tape/firstHit/double/host` | 8.30 ms | 8.31 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.6 ms | 18.8 ms | +0.7% | [-3.7%, +5.3%] | - | ~ | marched/traced 6.24x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 116 ms | 117 ms | +0.8% | [-0.3%, +1.9%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.7 ms | 22.0 ms | -3.2% | [-4.9%, -1.4%] | +0.0% | faster | marched/traced 7.26x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 161 ms | 160 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.4 ms | 35.4 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.2 ms | 19.3 ms | +0.6% | [-0.2%, +1.4%] | - | ~ | marched/traced 6.46x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 125 ms | 125 ms | +0.2% | [-0.1%, +0.4%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.3 ms | 23.3 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | marched/traced 7.09x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 165 ms | 165 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.5 ms | 37.5 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.2 ms | +1.5% | [+0.7%, +2.2%] | +0.0% | ~ | marched/traced 4.59x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.6 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.02 ms | 7.97 ms | -0.6% | [-0.8%, -0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.86 ms | 1.84 ms | -1.1% | [-2.8%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.88 ms | 6.82 ms | -1.0% | [-1.1%, -0.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.5 ms | -1.5% | [-1.8%, -1.2%] | +0.0% | ~ | marched/traced 5.27x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.5 ms | 60.8 ms | +0.5% | [-0.4%, +1.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.60 ms | 8.53 ms | -0.8% | [-1.1%, -0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.06 ms | 2.11 ms | +2.4% | [+1.5%, +3.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.15 ms | 7.05 ms | -1.4% | [-1.8%, -1.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 243 ms | 243 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | marched/traced 1.39x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 337 ms | 337 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 243 ms | 240 ms | -1.1% | [-3.1%, +0.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 253 ms | 252 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ | marched/traced 1.53x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 386 ms | 386 ms | -0.1% | [-1.7%, +1.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 251 ms | 252 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.8 ms | 23.6 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.8 ms | 23.7 ms | -0.4% | [-1.7%, +0.8%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.5 ms | 18.6 ms | +0.7% | [-0.6%, +2.0%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 28.9 ms | 28.7 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 29.0 ms | 28.6 ms | -1.5% | [-2.0%, -1.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.7 ms | 21.0 ms | -3.1% | [-6.0%, -0.0%] | +0.0% | faster |  |
| `Tape/csg/signedDistance/float/host` | 12.5 ms | 12.4 ms | -0.6% | [-0.8%, -0.3%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.51 ms | 4.50 ms | -0.2% | [-0.5%, +0.1%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.2 ms | 23.9 ms | -1.1% | [-1.6%, -0.6%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.2 ms | 40.4 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.73 ms | 1.73 ms | +0.3% | [-0.7%, +1.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 132 µs | 132 µs | -0.1% | [-8.6%, +9.2%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.9 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.66 ms | +0.0% | [-0.1%, +0.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 15.9 ms | 15.7 ms | -1.0% | [-1.1%, -0.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.70 ms | 4.63 ms | -1.6% | [-2.9%, -0.4%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 12.9 ms | 12.8 ms | -0.6% | [-0.9%, -0.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.56 ms | 4.55 ms | -0.2% | [-1.3%, +0.9%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 25.3 ms | 25.1 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 41.8 ms | 42.0 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.76 ms | 1.76 ms | +0.3% | [-4.2%, +5.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 262 µs | 290 µs | +10.4% | [+10.2%, +10.7%] | - | **MAJOR** |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.9 ms | 30.1 ms | +0.8% | [-0.1%, +1.7%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.72 ms | 7.69 ms | -0.3% | [-0.5%, -0.2%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.0 ms | 15.9 ms | -0.6% | [-0.9%, -0.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.82 ms | 7.74 ms | -1.0% | [-1.8%, -0.3%] | - | ~ |  |

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 8d784d1
base-fingerprint: 39675652ec063ec2cbaa9642714f87d3a7af3d99
head: 4c78c1f
head-fingerprint: 4ab6011b1e8931edacd0f1fdeea3a52292c20223
cases: 218
rounds: 2
reps: 3
date: 2026-10-31
-->

## Device comparison (CUDA): red

**Accepted:** known smaller failures to be fixed soon

`4c78c1f` against `8d784d1` (origin/dev), 2026-10-31.

**VERDICT: host and device answers differ in 6 cases; investigate before merging (see Flagged above)**

Performance comparison: 0 major, 1 minor, 2 faster, 6 differ, 0 lost, 218 cases

- base: 8d784d1 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 4c78c1f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest-morton/float/hostdevice` | 1.62 ms | 1.60 ms | -1.2% | [-3.2%, +0.9%] | - | ~ | device 22.8x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/float/hostdevice` | 3.40 ms | 3.43 ms | +0.8% | [+0.1%, +1.4%] | - | ~ | device 18.5x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/float/hostdevice` | 1.03 ms | 1.05 ms | +1.6% | [-0.7%, +3.8%] | - | ~ | device 19.7x faster than the host |
| `BVH/mesh-boxes/ray-any-random/float/hostdevice` | 2.32 ms | 2.30 ms | -0.6% | [-1.3%, +0.1%] | - | ~ | device 16.4x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/float/hostdevice` | 6.24 ms | 6.35 ms | +1.8% | [-1.5%, +5.1%] | - | ~ | device 7.1x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/float/hostdevice` | 11.9 ms | 11.9 ms | +0.3% | [-1.1%, +1.7%] | - | ~ | device 5.3x faster than the host |
| `BVH/mesh-boxes/ray-nearest-morton/double/hostdevice` | 33.6 ms | 33.4 ms | -0.6% | [-0.8%, -0.4%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-nearest-random/double/hostdevice` | 57.1 ms | 57.0 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 1.1x faster than the host |
| `BVH/mesh-boxes/ray-any-morton/double/hostdevice` | 20.7 ms | 20.8 ms | +0.6% | [-2.4%, +3.7%] | - | ~ | device 1.0x faster than the host |
| `BVH/mesh-boxes/ray-any-random/double/hostdevice` | 47.9 ms | 48.0 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `BVH/mesh-boxes/point-nearest-morton/double/hostdevice` | 61.1 ms | 61.7 ms | +1.0% | [-0.4%, +2.4%] | - | ~ | device 0.7x faster than the host |
| `BVH/mesh-boxes/point-nearest-random/double/hostdevice` | 111 ms | 109 ms | -1.7% | [-3.3%, -0.1%] | - | ~ | device 0.6x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 389 µs | 388 µs | -0.3% | [-1.4%, +0.8%] | - | ~ | device 14.7x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 845 µs | 847 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 9.9x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.51 ms | 4.52 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.65 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 589 µs | 592 µs | +0.6% | [+0.2%, +1.0%] | - | ~ | device 12.8x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.38 ms | 1.38 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 8.2x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.78 ms | 6.78 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.01 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 905 µs | 904 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 14.2x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.24 ms | 2.24 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 7.9x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.50 ms | 9.47 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | -0.3% | [-0.9%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 319 µs | 320 µs | +0.4% | [+0.1%, +0.6%] | - | ~ | device 11.1x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.72 ms | 1.72 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 5.1x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 3.4x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.54 ms | 5.54 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 520 ms | 517 ms | -0.5% | [-1.5%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.8 µs | 48.6 µs | +1.7% | [+0.1%, +3.3%] | - | ~ | device 23.1x faster than the host; tape/fused 4.48x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.9 µs | 49.7 µs | -0.4% | [-0.5%, -0.3%] | - | ~ | device 44.8x faster than the host; tape/fused 7.24x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.3% | [-0.7%, +0.1%] | - | ~ | device 8.7x faster than the host (was 9.7x); tape/fused 9.89x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.4 ms | +0.6% | [+0.1%, +1.1%] | - | ~ | device 6.8x faster than the host (was 8.3x); tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.3 ms | 31.3 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 809 µs | 811 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.5x faster than the host; tape/fused 1.05x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host (was 1.6x); tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 286 ms | 283 ms | -1.1% | [-2.8%, +0.7%] | - | ~ | device 0.4x faster than the host; tape/fused 1.02x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 285 ms | 288 ms | +0.9% | [+0.1%, +1.8%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.30 ms | 8.22 ms | -0.9% | [-1.5%, -0.3%] | +0.0% | ~ | device 12.7x faster than the host (was 10.7x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.28 ms | 8.33 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.27 ms | 8.21 ms | -0.7% | [-0.8%, -0.5%] | +0.0% | ~ | device 11.8x faster than the host (was 10.8x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.15 ms | 8.26 ms | +1.3% | [-0.5%, +3.2%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 704 µs | 703 µs | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | device 4.6x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.75 ms | 1.74 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.40x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.03 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.69x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 799 µs | 792 µs | -0.9% | [-1.0%, -0.8%] | +0.0% | ~ | device 4.1x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 828 µs | 828 µs | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ | device 11.5x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 949 µs | 949 µs | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ | device 10.1x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 828 µs | 831 µs | +0.4% | [-0.4%, +1.1%] | +0.0% | ~ | device 11.5x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 803 µs | 806 µs | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ | device 11.6x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 163 ms | 160 ms | -1.7% | [-4.0%, +0.6%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 158 ms | 162 ms | +2.2% | [-1.1%, +5.6%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x (was 1.03x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 164 ms | 161 ms | -1.9% | [-5.7%, +2.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 160 ms | 160 ms | -0.1% | [-2.4%, +2.2%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.59 ms | 1.61 ms | +0.8% | [+0.3%, +1.4%] | +0.0% | ~ | device 2.3x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.84 ms | 2.85 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.56x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.93 ms | 1.94 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ | device 1.9x faster than the host; default/this 0.83x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.68 ms | 1.69 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 2.2x faster than the host; default/this 0.95x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.29 ms | 7.30 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.30 ms | 7.28 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.31 ms | 7.32 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.36 ms | 7.33 ms | -0.4% | [-1.2%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 8.41 ms | 8.42 ms | +0.1% | [-1.0%, +1.1%] | - | ~ | device 11.4x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 16.8 ms | 16.6 ms | -0.9% | [-1.8%, +0.1%] | - | ~ | device 7.2x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 170 ms | 169 ms | -0.7% | [-1.6%, +0.1%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 287 ms | 288 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `MeshSDF/query-morton/float/hostdevice` | 24.7 ms | 24.4 ms | -1.1% | [-2.3%, +0.2%] | - | ~ | device 4.0x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 59.8 ms | 60.0 ms | +0.4% | [+0.2%, +0.5%] | - | ~ | device 2.4x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 158 ms | 157 ms | -0.3% | [-2.1%, +1.5%] | - | ~ | device 0.6x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 243 ms | 247 ms | +1.3% | [-0.2%, +2.9%] | - | ~ | device 0.6x faster than the host |
| `PointCloudBVH/uniform/closest-morton/float/hostdevice` | 766 µs | 762 µs | -0.5% | [-1.1%, +0.1%] | - | ~ | device 10.8x faster than the host |
| `PointCloudBVH/uniform/closest-random/float/hostdevice` | 2.22 ms | 2.22 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 5.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/float/hostdevice` | 1.17 ms | 1.17 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 8.4x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/float/hostdevice` | 6.46 ms | 6.45 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 4.3x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/float/hostdevice` | 4.13 ms | 4.12 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 4.2x faster than the host |
| `PointCloudBVH/uniform/closest-morton/double/hostdevice` | 7.10 ms | 7.08 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/uniform/closest-random/double/hostdevice` | 7.80 ms | 7.80 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/uniform/neighbor-morton/double/hostdevice` | 13.2 ms | 13.2 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `PointCloudBVH/uniform/neighbors8-morton/double/hostdevice` | 20.5 ms | 20.5 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `PointCloudBVH/uniform/closest8-morton/double/hostdevice` | 11.9 ms | 11.9 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 1.5x faster than the host (was 1.5x) |
| `PointCloudHashGrid/uniform/closest-morton/float/hostdevice` | 2.16 ms | 2.17 ms | +0.5% | [-0.3%, +1.2%] | - | ~ | device 5.0x faster than the host (was 4.8x) |
| `PointCloudHashGrid/uniform/closest-random/float/hostdevice` | 5.95 ms | 5.96 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 2.7x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/float/hostdevice` | 831 µs | 832 µs | +0.2% | [-0.0%, +0.5%] | - | ~ | device 10.6x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/float/hostdevice` | 8.97 ms | 9.01 ms | +0.4% | [-0.1%, +0.9%] | - | ~ | device 3.9x faster than the host (was 3.7x) |
| `PointCloudHashGrid/uniform/closest8-morton/float/hostdevice` | 6.40 ms | 6.42 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 4.1x faster than the host (was 4.0x) |
| `PointCloudHashGrid/uniform/closest-morton/double/hostdevice` | 8.11 ms | 8.09 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/uniform/closest-random/double/hostdevice` | 16.0 ms | 15.9 ms | -0.6% | [-1.0%, -0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudHashGrid/uniform/neighbor-morton/double/hostdevice` | 4.02 ms | 4.03 ms | +0.3% | [-0.5%, +1.1%] | - | ~ | device 2.2x faster than the host |
| `PointCloudHashGrid/uniform/neighbors8-morton/double/hostdevice` | 20.3 ms | 20.5 ms | +0.9% | [+0.5%, +1.3%] | - | ~ | device 1.7x faster than the host |
| `PointCloudHashGrid/uniform/closest8-morton/double/hostdevice` | 15.1 ms | 15.1 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest-morton/float/hostdevice` | 1.51 ms | 1.49 ms | -1.0% | [-1.8%, -0.2%] | - | ~ | device 8.6x faster than the host |
| `PointCloudBVH/sphere/closest-random/float/hostdevice` | 4.75 ms | 4.76 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 4.0x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/float/hostdevice` | 1.38 ms | 1.38 ms | +0.2% | [-0.5%, +0.8%] | - | ~ | device 5.4x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/float/hostdevice` | 3.40 ms | 3.40 ms | -0.0% | [-0.5%, +0.5%] | - | ~ | device 5.5x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/float/hostdevice` | 4.63 ms | 4.62 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 4.1x faster than the host |
| `PointCloudBVH/sphere/closest-morton/double/hostdevice` | 13.4 ms | 13.4 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 1.0x faster than the host |
| `PointCloudBVH/sphere/closest-random/double/hostdevice` | 17.2 ms | 17.2 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudBVH/sphere/neighbor-morton/double/hostdevice` | 7.54 ms | 7.54 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.1x faster than the host |
| `PointCloudBVH/sphere/neighbors8-morton/double/hostdevice` | 11.3 ms | 11.3 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `PointCloudBVH/sphere/closest8-morton/double/hostdevice` | 15.8 ms | 15.8 ms | +0.2% | [-0.0%, +0.3%] | - | ~ | device 1.2x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/float/hostdevice` | 4.86 ms | 4.85 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 3.6x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/float/hostdevice` | 16.0 ms | 16.1 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/float/hostdevice` | 1.61 ms | 1.62 ms | +0.7% | [-0.2%, +1.5%] | - | ~ | device 7.3x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/float/hostdevice` | 5.61 ms | 5.60 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 4.9x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/float/hostdevice` | 13.3 ms | 13.4 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 1.9x faster than the host |
| `PointCloudHashGrid/sphere/closest-morton/double/hostdevice` | 34.0 ms | 34.0 ms | +0.1% | [-1.2%, +1.5%] | - | ~ | device 0.5x faster than the host |
| `PointCloudHashGrid/sphere/closest-random/double/hostdevice` | 68.4 ms | 68.2 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 0.6x faster than the host |
| `PointCloudHashGrid/sphere/neighbor-morton/double/hostdevice` | 18.4 ms | 18.5 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `PointCloudHashGrid/sphere/neighbors8-morton/double/hostdevice` | 20.9 ms | 21.0 ms | +0.5% | [+0.3%, +0.7%] | - | ~ | device 1.3x faster than the host |
| `PointCloudHashGrid/sphere/closest8-morton/double/hostdevice` | 39.3 ms | 39.4 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 672 µs | 675 µs | +0.4% | [+0.3%, +0.6%] | - | ~ | device 16.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 14.5x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 646 µs | 641 µs | -0.7% | [-1.6%, +0.3%] | - | ~ | device 17.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | +0.1% | [-0.8%, +1.0%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 607 µs | 604 µs | -0.5% | [-1.5%, +0.5%] | - | ~ | device 16.0x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.11 ms | -0.4% | [-1.7%, +0.9%] | - | ~ | device 15.0x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.49 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.51x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.1% | [-0.7%, +0.9%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.17 ms | -0.5% | [-0.6%, -0.5%] | - | ~ | device 10.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 630 µs | 595 µs | -5.6% | [-5.9%, -5.3%] | - | faster | device 2.5x faster than the host (was 2.4x); tape/fused 1.18x (was 1.11x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 730 µs | 701 µs | -4.0% | [-8.9%, +1.0%] | - | ~ | device 2.6x faster than the host; tape/fused 1.19x (was 1.15x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 702 µs | 705 µs | +0.4% | [+0.1%, +0.7%] | - | ~ | device 3.7x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 841 µs | 838 µs | -0.3% | [-0.6%, -0.0%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.1 ms | +0.9% | [+0.6%, +1.3%] | - | ~ | device 0.7x faster than the host; hand/api 0.99x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.6 ms | 27.5 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.6% | [-0.3%, +1.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.4 ms | 24.5 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.6 ms | 16.4 ms | -0.7% | [-1.2%, -0.1%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.04x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.5% | [-0.7%, +1.7%] | - | ~ | device 1.0x faster than the host (was 1.1x); triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 15.0 ms | -0.2% | [-1.1%, +0.8%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 22.9 ms | 23.1 ms | +0.8% | [+0.7%, +1.0%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.48 ms | 1.48 ms | +0.2% | [-0.7%, +1.1%] | - | ~ | device 1.1x faster than the host; tape/fused 1.08x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.60 ms | 2.61 ms | +0.5% | [+0.3%, +0.6%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.60 ms | 1.60 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 1.7x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.72 ms | 2.72 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 1.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.59 ms | 2.59 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.5x faster than the host; marched/traced 4.30x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.03 ms | 3.02 ms | -0.2% | [-0.3%, -0.2%] | - | ~ | device 2.3x faster than the host; marched/traced 5.74x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 11.1 ms | 11.1 ms | -0.0% | [-0.3%, +0.3%] | - | **DIFFER** | device 3.4x faster than the host; 1 host and device answers DIFFER (max diff 3.14) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 17.3 ms | 17.4 ms | +0.1% | [-0.1%, +0.4%] | - | **DIFFER** | device 2.5x faster than the host; 1 host and device answers DIFFER (max diff 3.14) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.19 ms | 2.19 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 3.8x faster than the host; marched/traced 18.33x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.49 ms | 5.49 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.8x faster than the host; marched/traced 14.39x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 39.9 ms | 40.1 ms | +0.4% | [-0.1%, +1.0%] | - | **DIFFER** | device 1.9x faster than the host; 1 host and device answers DIFFER (max diff 3.14) |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 79.2 ms | 79.0 ms | -0.2% | [-0.7%, +0.4%] | - | **DIFFER** | device 1.5x faster than the host; 1 host and device answers DIFFER (max diff 3.14) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.25 ms | 5.27 ms | +0.4% | [-0.0%, +0.8%] | - | **DIFFER** | device 2.9x faster than the host; 1 host and device answers DIFFER (max diff 1.41) |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.84 ms | 8.85 ms | +0.1% | [-0.1%, +0.3%] | - | **DIFFER** | device 3.1x faster than the host; 1 host and device answers DIFFER (max diff 1.41) |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.30 ms | 6.32 ms | +0.3% | [-0.8%, +1.5%] | - | ~ | device 1.2x faster than the host; marched/traced 30.89x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.7 ms | 10.6 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 0.8x faster than the host; marched/traced 29.19x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 198 ms | 195 ms | -1.3% | [-3.1%, +0.5%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 311 ms | 310 ms | -0.4% | [-1.0%, +0.1%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.85 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.8x faster than the host; marched/traced 26.50x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.1 ms | 12.1 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 1.3x faster than the host; marched/traced 20.89x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 128 ms | 129 ms | +0.6% | [+0.5%, +0.7%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 252 ms | 253 ms | +0.4% | [-0.4%, +1.1%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.2 ms | -1.1% | [-1.8%, -0.4%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.65 ms | 3.57 ms | -2.2% | [-4.5%, +0.1%] | - | ~ | device 1.2x faster than the host (was 1.2x); marched/traced 5.89x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.80 ms | 4.83 ms | +0.7% | [-0.8%, +2.2%] | - | ~ | device 0.9x faster than the host; marched/traced 8.59x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 21.1 ms | 21.0 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 2.3x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 41.4 ms | 41.5 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 1.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.25 ms | 1.26 ms | +1.1% | [-1.0%, +3.3%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.85 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 96.0 µs | 94.5 µs | -1.6% | [-3.8%, +0.7%] | - | ~ | device 15.2x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 163 µs | 163 µs | -0.2% | [-1.1%, +0.8%] | - | ~ | device 10.1x faster than the host (was 10.5x) |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 809 µs | 806 µs | -0.4% | [-0.5%, -0.2%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 948 µs | 948 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.8 ms | 34.6 ms | -3.2% | [-9.5%, +3.4%] | - | ~ | device 0.1x faster than the host; marched/traced 3.69x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.5 ms | 36.9 ms | +1.2% | [+0.6%, +1.8%] | - | ~ | device 0.1x faster than the host; marched/traced 6.56x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 132 ms | 128 ms | -3.4% | [-5.3%, -1.6%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 241 ms | 242 ms | +0.3% | [-2.1%, +2.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 15.8 ms | 16.5 ms | +4.4% | [+1.1%, +7.7%] | - | minor | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.7 ms | 21.6 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.07 ms | 1.08 ms | +1.1% | [-5.0%, +7.5%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.34 ms | 2.36 ms | +0.6% | [+0.4%, +0.8%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 5.99 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.73 ms | 6.70 ms | -0.4% | [-0.8%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.2x faster than the host; marched/traced 4.74x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.4 ms | +0.4% | [-0.3%, +1.0%] | - | ~ | device 1.9x faster than the host; marched/traced 6.03x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 109 ms | 108 ms | -0.4% | [-1.3%, +0.6%] | - | ~ | device 2.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 173 ms | 171 ms | -1.4% | [-3.3%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.9 ms | 13.7 ms | -0.9% | [-1.3%, -0.4%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.3 ms | -0.5% | [-1.0%, +0.0%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 296 ms | 296 ms | -0.2% | [-1.9%, +1.6%] | - | ~ | device 0.2x faster than the host; marched/traced 2.15x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 344 ms | 348 ms | +1.1% | [-3.3%, +5.9%] | - | ~ | device 0.2x faster than the host; marched/traced 3.17x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 633 ms | 636 ms | +0.5% | [-1.4%, +2.4%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.10 s | 1.10 s | +0.2% | [-1.0%, +1.4%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 193 ms | 189 ms | -2.3% | [-4.1%, -0.4%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 238 ms | 235 ms | -1.3% | [-2.3%, -0.2%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.57 ms | 4.58 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.8x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.58 ms | 4.58 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 2.8x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.87 ms | 1.88 ms | +0.6% | [+0.0%, +1.1%] | - | ~ | device 8.8x faster than the host |
| `Render/csg/frame/double/hostdevice` | 8.12 ms | 8.10 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 8.10 ms | 8.10 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.3 ms | 23.1 ms | -0.6% | [-1.5%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 218 µs | -0.0% | [-0.6%, +0.5%] | - | ~ | device 10.9x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.1% | [-0.2%, -0.0%] | - | ~ | device 23.9x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 361 µs | 360 µs | -0.4% | [-0.6%, -0.3%] | - | ~ | device 12.8x faster than the host (was 12.3x) |
| `Tape/union64/signedDistance/float/hostdevice` | 1.99 ms | 1.99 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 7.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.3 µs | 72.3 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 7.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 48.6 µs | 48.2 µs | -0.8% | [-2.3%, +0.6%] | - | ~ | device 4.9x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 651 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 11.5x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 473 µs | 473 µs | -0.0% | [-0.3%, +0.3%] | - | ~ | device 7.1x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 48.1 µs | +0.6% | [+0.2%, +1.0%] | - | ~ | device 27.0x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 854 µs | 854 µs | +0.0% | [-0.2%, +0.2%] | - | ~ | device 3.0x faster than the host (was 3.1x) |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.71 ms | 6.72 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 190 µs | 191 µs | +0.2% | [+0.1%, +0.4%] | - | ~ | device 2.9x faster than the host (was 2.8x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.8x faster than the host (was 1.9x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |

<!-- END receipt cuda -->
