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
base: 5b39cbb
base-fingerprint: 4ab6011b1e8931edacd0f1fdeea3a52292c20223
head: 3791e5a
head-fingerprint: 59130b91cbb90f3926949df3226bf4d112d28a84
cases: 152
rounds: 2
reps: 3
date: 2026-10-31
-->

## Host comparison: yellow

**Accepted:** Accept as noise

`3791e5a` against `5b39cbb` (origin/main), 2026-10-31.

**VERDICT: 2 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)**

Performance comparison: 0 major, 2 minor, 3 faster, 0 differ, 0 lost, 152 cases

- base: 5b39cbb (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3791e5a (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 330 ms | 329 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 177 ms | 178 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 515 ms | 516 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 345 ms | 344 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 185 ms | 185 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 563 ms | 564 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.4 ms | 55.2 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.1 ms | 51.9 ms | +1.6% | [+0.3%, +2.8%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.0 ms | 63.9 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 54.6 ms | 54.6 ms | +0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.3 ms | 60.1 ms | -0.3% | [-1.2%, +0.6%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.5 ms | 72.2 ms | -0.5% | [-1.0%, +0.0%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.2 ms | 74.3 ms | +0.1% | [-0.8%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.4 ms | 77.5 ms | +0.1% | [-0.8%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 67.0 ms | 66.7 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.6 ms | 101 ms | +1.1% | [+0.7%, +1.6%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 82.8 ms | 83.0 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 110 ms | +1.3% | [+0.3%, +2.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.3 ms | 19.4 ms | +0.6% | [+0.0%, +1.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 44.5 ms | 44.5 ms | -0.1% | [-4.3%, +4.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.8 ms | 72.6 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.2 ms | 20.1 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.7 ms | 45.8 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.7 ms | 81.9 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.46 ms | 4.44 ms | -0.6% | [-2.0%, +0.8%] | - | ~ | tape/fused 2.81x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.4 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | tape/fused 1.94x |
| `Fused/chain16/signedDistance/float/host` | 969 µs | 970 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | tape/fused 16.24x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.2 ms | 45.9 ms | -0.7% | [-1.2%, -0.2%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.2 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.55 ms | 4.55 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | tape/fused 2.86x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.7 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | tape/fused 1.98x |
| `Fused/chain16/signedDistance/double/host` | 3.87 ms | 3.88 ms | +0.2% | [-1.2%, +1.7%] | - | ~ | tape/fused 4.12x |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.5 ms | 54.5 ms | +2.0% | [-0.3%, +4.3%] | +0.0% | ~ | tape/fused 0.99x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 54.0 ms | 53.7 ms | -0.6% | [-1.3%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 82.1 ms | 82.3 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 229 ms | 229 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 113 ms | 113 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 263 ms | 262 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 70.6 ms | 70.5 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 214 ms | 215 ms | +0.7% | [+0.1%, +1.3%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 96.5 ms | 96.1 ms | -0.4% | [-1.3%, +0.5%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.1 ms | 29.7 ms | -1.1% | [-2.3%, -0.0%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 769 ms | 756 ms | -1.6% | [-3.4%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 118 ms | 118 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 43.9 ms | 44.0 ms | +0.3% | [-1.0%, +1.7%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 840 ms | 843 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/build/float/host` | 68.0 ms | 68.3 ms | +0.4% | [+0.0%, +0.7%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/float/host` | 147 ms | 147 ms | -0.1% | [-0.3%, +0.0%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/float/host` | 369 ms | 371 ms | +0.3% | [-0.0%, +0.7%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/float/host` | 78.5 ms | 79.0 ms | +0.5% | [-2.0%, +3.1%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/float/host` | 6.61 ms | 6.59 ms | -0.3% | [-1.1%, +0.4%] | - | ~ |  |
| `PointCloudBVH/uniform/build/double/host` | 80.7 ms | 80.6 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `PointCloudBVH/uniform/all1nn/double/host` | 159 ms | 158 ms | -0.4% | [-1.0%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/all8nn/double/host` | 399 ms | 400 ms | +0.1% | [-0.1%, +0.3%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-random/double/host` | 87.0 ms | 87.3 ms | +0.3% | [-0.4%, +0.9%] | - | ~ |  |
| `PointCloudBVH/uniform/closest-outside-random/double/host` | 7.16 ms | 7.21 ms | +0.6% | [-0.7%, +2.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/float/host` | 12.1 ms | 10.7 ms | -11.5% | [-21.1%, -1.2%] | +0.0% | faster |  |
| `PointCloudHashGrid/uniform/all1nn/float/host` | 123 ms | 123 ms | +0.0% | [-0.5%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/float/host` | 514 ms | 515 ms | +0.2% | [-0.1%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/float/host` | 153 ms | 153 ms | -0.1% | [-1.1%, +1.0%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/float/host` | 841 ms | 841 ms | +0.0% | [-0.7%, +0.8%] | - | ~ |  |
| `PointCloudHashGrid/uniform/build/double/host` | 15.0 ms | 15.0 ms | +0.4% | [-0.1%, +0.9%] | +0.0% | ~ |  |
| `PointCloudHashGrid/uniform/all1nn/double/host` | 127 ms | 126 ms | -0.2% | [-0.9%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/uniform/all8nn/double/host` | 544 ms | 544 ms | -0.1% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-random/double/host` | 155 ms | 156 ms | +0.4% | [+0.1%, +0.7%] | - | ~ |  |
| `PointCloudHashGrid/uniform/closest-outside-random/double/host` | 891 ms | 890 ms | -0.0% | [-0.9%, +0.8%] | - | ~ |  |
| `PointCloudBVH/sphere/build/float/host` | 67.5 ms | 67.4 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/float/host` | 109 ms | 109 ms | +0.4% | [-0.2%, +1.0%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/float/host` | 270 ms | 269 ms | -0.4% | [-0.6%, -0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/float/host` | 112 ms | 111 ms | -0.4% | [-1.2%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/float/host` | 106 ms | 106 ms | -0.1% | [-0.2%, +0.1%] | - | ~ |  |
| `PointCloudBVH/sphere/build/double/host` | 80.2 ms | 80.5 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ |  |
| `PointCloudBVH/sphere/all1nn/double/host` | 119 ms | 119 ms | +0.3% | [-0.3%, +0.9%] | - | ~ |  |
| `PointCloudBVH/sphere/all8nn/double/host` | 294 ms | 295 ms | +0.3% | [+0.1%, +0.5%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-random/double/host` | 127 ms | 127 ms | -0.5% | [-1.1%, +0.2%] | - | ~ |  |
| `PointCloudBVH/sphere/closest-outside-random/double/host` | 122 ms | 122 ms | -0.1% | [-0.2%, +0.0%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/float/host` | 8.99 ms | 8.80 ms | -2.1% | [-4.0%, -0.1%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/float/host` | 144 ms | 141 ms | -2.3% | [-4.5%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/float/host` | 398 ms | 399 ms | +0.3% | [+0.1%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/float/host` | 251 ms | 250 ms | -0.2% | [-0.6%, +0.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/float/host` | 333 ms | 332 ms | -0.2% | [-0.9%, +0.6%] | - | ~ |  |
| `PointCloudHashGrid/sphere/build/double/host` | 12.5 ms | 12.6 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `PointCloudHashGrid/sphere/all1nn/double/host` | 148 ms | 147 ms | -0.2% | [-5.2%, +5.1%] | - | ~ |  |
| `PointCloudHashGrid/sphere/all8nn/double/host` | 418 ms | 418 ms | +0.1% | [-0.2%, +0.5%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-random/double/host` | 284 ms | 287 ms | +0.9% | [+0.4%, +1.4%] | - | ~ |  |
| `PointCloudHashGrid/sphere/closest-outside-random/double/host` | 344 ms | 345 ms | +0.3% | [+0.0%, +0.6%] | - | ~ |  |
| `Ray/mesh/firstHit/float/host` | 46.2 ms | 46.5 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.6 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.7 ms | 40.2 ms | -1.1% | [-1.8%, -0.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 77.3 ms | 77.2 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/float/host` | 65.1 ms | 65.3 ms | +0.4% | [-0.1%, +0.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.81 ms | 5.92 ms | +2.0% | [-1.0%, +4.9%] | - | ~ | tape/fused 1.31x |
| `Ray/csg-tape/firstHit/float/host` | 7.79 ms | 7.75 ms | -0.4% | [-1.2%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 51.5 ms | 51.3 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ | hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.3 ms | 51.0 ms | -0.8% | [-1.3%, -0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.6 ms | 44.6 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.5 ms | 86.1 ms | -1.5% | [-2.4%, -0.7%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.2 ms | 74.0 ms | +1.0% | [+0.2%, +1.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.99 ms | 5.92 ms | -1.2% | [-3.3%, +0.9%] | - | ~ | tape/fused 1.40x |
| `Ray/csg-tape/firstHit/double/host` | 8.30 ms | 8.29 ms | -0.2% | [-1.2%, +0.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 18.5 ms | 18.6 ms | +0.4% | [-1.9%, +2.7%] | - | ~ | marched/traced 6.23x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 116 ms | 116 ms | +0.2% | [+0.0%, +0.3%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 22.1 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | marched/traced 7.24x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 160 ms | 160 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.4 ms | 35.5 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.2 ms | 19.1 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | marched/traced 6.49x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 124 ms | 124 ms | -0.1% | [-0.3%, +0.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 23.4 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ | marched/traced 7.06x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 166 ms | 165 ms | -0.1% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.6 ms | 37.7 ms | +0.4% | [+0.3%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ | marched/traced 4.66x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.9 ms | 51.7 ms | -0.5% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.00 ms | 7.99 ms | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.85 ms | 1.86 ms | +0.7% | [+0.6%, +0.8%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.82 ms | 6.79 ms | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.6 ms | -0.7% | [-2.3%, +1.0%] | +0.0% | ~ | marched/traced 5.31x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.1 ms | 61.7 ms | +0.9% | [+0.5%, +1.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.99 ms | 8.56 ms | -4.8% | [-9.0%, -0.2%] | +0.0% | faster |  |
| `RayParticles/armadillo/step/double/host` | 2.10 ms | 2.12 ms | +0.8% | [-0.8%, +2.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.06 ms | 7.10 ms | +0.4% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 242 ms | 242 ms | -0.3% | [-1.1%, +0.5%] | +0.0% | ~ | marched/traced 1.39x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 336 ms | 337 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 241 ms | 240 ms | -0.4% | [-1.5%, +0.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 253 ms | 252 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | marched/traced 1.51x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 381 ms | 380 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 251 ms | 251 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 23.6 ms | 23.6 ms | -0.2% | [-1.1%, +0.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 23.5 ms | 23.7 ms | +0.9% | [-0.1%, +1.9%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.4 ms | 19.4 ms | +5.0% | [+2.9%, +7.0%] | +0.0% | minor |  |
| `Render/csg/frame/double/host` | 28.7 ms | 28.6 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 28.7 ms | 28.6 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.1 ms | 21.0 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.4 ms | 12.5 ms | +0.9% | [-0.0%, +1.8%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.50 ms | 4.52 ms | +0.4% | [-0.4%, +1.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.0 ms | 24.0 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.4 ms | 40.4 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.76 ms | 1.74 ms | -1.1% | [-2.5%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 120 µs | 132 µs | +9.9% | [+0.2%, +20.7%] | - | minor |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.8 ms | 30.0 ms | +0.7% | [+0.3%, +1.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.65 ms | 5.66 ms | +0.1% | [-0.2%, +0.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.0 ms | 15.7 ms | -1.3% | [-2.2%, -0.4%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.62 ms | 4.64 ms | +0.3% | [-0.6%, +1.2%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 12.9 ms | 13.0 ms | +1.1% | [-0.1%, +2.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.58 ms | 4.58 ms | -0.1% | [-0.6%, +0.4%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 25.0 ms | 25.2 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 49.8 ms | 42.0 ms | -15.6% | [-27.2%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.74 ms | 1.74 ms | -0.1% | [-1.8%, +1.8%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 265 µs | 256 µs | -3.5% | [-4.8%, -2.1%] | - | faster |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.8 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.73 ms | 7.73 ms | +0.0% | [-0.4%, +0.5%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.0 ms | 16.0 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.72 ms | 7.72 ms | +0.0% | [-0.1%, +0.1%] | - | ~ |  |

<!-- END receipt host -->
