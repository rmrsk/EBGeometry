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
verdict: green
base: d22c602
base-fingerprint: b3d98b4c4254e9729fc5d48c3cb8238021bad874
head: 40e7a91
head-fingerprint: 1dc80b70de91c722b7232e58dbcc7d9e131dbbd1
cases: 132
rounds: 2
reps: 3
date: 2026-11-07
-->

## Host comparison: green

**Accepted:** -

`40e7a91` against `d22c602` (origin/main), 2026-11-07.

**VERDICT: no regression (0 major, 0 minor across 132 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 132 cases

- base: d22c602 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 40e7a91 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/build/float/host` | 55.0 ms | 55.0 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.4 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/double/host` | 64.8 ms | 64.8 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/spheres/query-random/double/host` | 64.2 ms | 54.9 ms | -14.5% | [-25.4%, +0.1%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.6 ms | 60.4 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.3 ms | 72.4 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.9 ms | 74.1 ms | -1.2% | [-1.5%, -0.8%] | +0.0% | ~ |  |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.6 ms | 77.9 ms | +0.4% | [-0.5%, +1.3%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/float/host` | 66.8 ms | 67.1 ms | +0.4% | [-0.1%, +1.0%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/float/host` | 99.8 ms | 100 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/build/double/host` | 83.3 ms | 83.4 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 108 ms | 108 ms | +0.4% | [-0.0%, +0.7%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | - | - | - | - | - | new |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | - | - | - | - | - | new |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | - | - | - | - | - | new |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | - | - | - | - | - | new |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.6 ms | 19.6 ms | +0.2% | [-0.7%, +1.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 44.3 ms | 44.3 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.2 ms | 73.2 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.0 ms | 20.0 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 45.9 ms | 45.9 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.4 ms | 81.3 ms | -1.3% | [-2.4%, -0.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.48 ms | 4.48 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | tape/fused 2.87x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | tape/fused 2.04x |
| `Fused/chain16/signedDistance/float/host` | 976 µs | 968 µs | -0.9% | [-2.2%, +0.4%] | - | ~ | tape/fused 16.69x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.1 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.3 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.62 ms | 4.67 ms | +1.2% | [-0.4%, +2.8%] | - | ~ | tape/fused 2.82x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | +0.4% | [-0.0%, +0.9%] | - | ~ | tape/fused 2.05x |
| `Fused/chain16/signedDistance/double/host` | 3.91 ms | 3.92 ms | +0.3% | [-1.2%, +1.9%] | - | ~ | tape/fused 4.16x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.6 ms | 52.7 ms | +0.2% | [-0.8%, +1.1%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.5 ms | 52.6 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.6 ms | +0.1% | [-0.5%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.5 ms | 27.1 ms | +2.2% | [+1.2%, +3.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 320 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.2 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ | tape/fused 3.54x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.7 ms | 99.7 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.6 ms | 30.0 ms | +1.3% | [-0.3%, +3.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 29.7 ms | 29.7 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 356 ms | 355 ms | -0.2% | [-0.4%, -0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 255 ms | 256 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.1 ms | 97.8 ms | +0.8% | [+0.2%, +1.4%] | +0.0% | ~ | tape/fused 3.63x |
| `MeshCSG/smooth-fused/frame/double/host` | 107 ms | 107 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ | tape/fused 2.39x |
| `Ray/mesh/firstHit/float/host` | 47.0 ms | 46.4 ms | -1.3% | [-2.8%, +0.3%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.6 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.3 ms | 40.2 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.6 ms | 78.7 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.4 ms | 65.8 ms | +0.6% | [+0.2%, +1.0%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.21 ms | 5.18 ms | -0.4% | [-1.1%, +0.3%] | - | ~ | tape/fused 1.47x |
| `Ray/csg-tape/firstHit/float/host` | 7.62 ms | 7.65 ms | +0.3% | [-0.6%, +1.3%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.7 ms | 52.3 ms | -0.8% | [-2.2%, +0.6%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.2 ms | 51.0 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.8 ms | 44.7 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.2 ms | 87.7 ms | -0.6% | [-1.6%, +0.4%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.3 ms | 73.4 ms | +0.2% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.54 ms | 5.49 ms | -1.0% | [-1.4%, -0.5%] | - | ~ | tape/fused 1.44x |
| `Ray/csg-tape/firstHit/double/host` | 7.90 ms | 7.89 ms | -0.1% | [-1.0%, +0.8%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.4 ms | 17.4 ms | +0.4% | [-1.6%, +2.4%] | - | ~ | marched/traced 6.68x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 117 ms | 116 ms | -0.3% | [-0.4%, -0.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 21.9 ms | -1.3% | [-3.1%, +0.6%] | +0.0% | ~ | marched/traced 7.40x (was 6.95x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 162 ms | +5.1% | [-0.1%, +10.4%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.3 ms | 35.3 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.3 ms | 19.1 ms | -1.0% | [-2.5%, +0.5%] | - | ~ | marched/traced 6.48x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 123 ms | 124 ms | +0.5% | [+0.3%, +0.7%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.0 ms | 23.0 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | marched/traced 6.96x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 159 ms | 160 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.4 ms | 37.5 ms | +0.2% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.2 ms | 46.7 ms | +0.9% | [+0.2%, +1.7%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.4 ms | 46.1 ms | -0.7% | [-1.0%, -0.3%] | +0.0% | ~ | hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.79 ms | 7.83 ms | +0.5% | [-0.0%, +1.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.28 ms | 6.22 ms | -0.9% | [-1.8%, +0.1%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 177 ms | 177 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 158 ms | 157 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.8 ms | 68.7 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.6 ms | 56.4 ms | -0.3% | [-1.2%, +0.5%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.1 ms | 52.5 ms | +0.8% | [+0.1%, +1.4%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.3 ms | 52.0 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ | hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.84 ms | 7.87 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.21 ms | 6.21 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ | hit/crossing 1.27x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 199 ms | 199 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 177 ms | 177 ms | +0.2% | [-0.4%, +0.9%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.7 ms | 78.8 ms | +1.4% | [+0.7%, +2.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.3 ms | 64.6 ms | +0.4% | [-0.3%, +1.2%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | -0.3% | [-1.4%, +0.8%] | +0.0% | ~ | marched/traced 4.68x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.8 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.00 ms | 7.96 ms | -0.5% | [-0.6%, -0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.93 ms | 1.92 ms | -0.7% | [-1.6%, +0.2%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.79 ms | 6.81 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.6 ms | 11.6 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | marched/traced 5.23x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.0 ms | 60.9 ms | -0.3% | [-1.4%, +0.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.66 ms | 8.63 ms | -0.4% | [-1.9%, +1.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.92 ms | 1.96 ms | +2.2% | [+0.5%, +3.9%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.19 ms | 7.18 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 239 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 329 ms | 329 ms | +0.1% | [-0.7%, +0.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 238 ms | 237 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 251 ms | 251 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | marched/traced 1.47x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 370 ms | 369 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 250 ms | 250 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 18.6 ms | 18.8 ms | +1.1% | [-0.1%, +2.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 18.6 ms | 18.8 ms | +0.8% | [-0.2%, +1.7%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.5 ms | 18.7 ms | +1.3% | [+0.8%, +1.8%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 21.6 ms | 21.4 ms | -0.9% | [-1.5%, -0.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 22.0 ms | 21.3 ms | -3.0% | [-4.9%, -1.0%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.4 ms | 20.5 ms | +0.3% | [-0.5%, +1.0%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.8 ms | 74.8 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 298 ms | 298 ms | -0.0% | [-0.9%, +0.9%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 299 ms | 297 ms | -0.7% | [-0.9%, -0.5%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 86.9 ms | 87.3 ms | +0.5% | [-0.2%, +1.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 344 ms | 344 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 348 ms | 348 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.8 ms | 12.9 ms | +0.6% | [-0.2%, +1.5%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.58 ms | 4.56 ms | -0.6% | [-1.5%, +0.4%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.1 ms | 25.1 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.5 ms | 40.4 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.76 ms | 1.75 ms | -0.3% | [-2.1%, +1.5%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 138 µs | 142 µs | +2.4% | [-0.7%, +10.2%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.1 ms | 29.9 ms | -0.6% | [-1.5%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.67 ms | 5.65 ms | -0.4% | [-0.7%, -0.0%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.2 ms | 16.1 ms | -0.6% | [-1.6%, +0.4%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.67 ms | 4.69 ms | +0.3% | [-1.0%, +1.5%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.2 ms | +0.8% | [-0.2%, +1.8%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.62 ms | 4.60 ms | -0.5% | [-2.6%, +1.8%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.4 ms | 26.3 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 42.2 ms | 42.1 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.79 ms | 1.78 ms | -0.6% | [-3.0%, +1.8%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 300 µs | 286 µs | -4.6% | [-11.2%, +2.3%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.9 ms | 29.7 ms | -0.7% | [-1.6%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.75 ms | 7.77 ms | +0.3% | [-0.2%, +0.7%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.4 ms | 16.3 ms | -0.2% | [-1.0%, +0.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.80 ms | 7.74 ms | -0.7% | [-1.3%, -0.1%] | - | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 436.8 s → 436.0 s (-0.2%), the largest peak 1264 MB → 1269 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 8.8 s | 8.7 s | -0.8% | 557 MB | 557 MB | -0.1% | ~ |
| `BVHRefit` | 3.8 s | 3.8 s | +0.4% | 454 MB | 455 MB | +0.4% | ~ |
| `BVHUnion` | 14.5 s | 14.6 s | +0.6% | 814 MB | 815 MB | +0.1% | ~ |
| `BVHUnionMixed` | - | 7.1 s | | - | 557 MB | | new |
| `ClosestPrimitive` | 25.6 s | 25.2 s | -1.5% | 1166 MB | 1207 MB | +3.5% | ~ |
| `Fused` | 20.4 s | 20.5 s | +0.5% | 988 MB | 993 MB | +0.5% | ~ |
| `LaunchTuning` | 32.5 s | 32.5 s | +0.0% | 1106 MB | 1108 MB | +0.2% | ~ |
| `Mesh` | 14.0 s | 14.0 s | -0.0% | 775 MB | 768 MB | -0.9% | ~ |
| `MeshCSG` | 33.9 s | 33.9 s | +0.2% | 1153 MB | 1153 MB | -0.1% | ~ |
| `MeshReconcile` | 6.1 s | 6.0 s | -0.8% | 532 MB | 534 MB | +0.4% | ~ |
| `PointCloud` | 6.9 s | 6.9 s | -0.4% | 557 MB | 557 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.4 s | 3.5 s | +1.8% | 406 MB | 410 MB | +0.8% | ~ |
| `PointCloudHashGridBuild` | 2.9 s | 3.0 s | +0.2% | 403 MB | 402 MB | -0.2% | ~ |
| `Ray` | 31.8 s | 31.9 s | +0.2% | 1147 MB | 1147 MB | +0.0% | ~ |
| `RayCSG` | 28.6 s | 28.6 s | -0.2% | 962 MB | 964 MB | +0.2% | ~ |
| `RayCrossing` | 40.9 s | 40.5 s | -0.8% | 1264 MB | 1264 MB | +0.0% | ~ |
| `RayParticles` | 42.9 s | 42.9 s | -0.0% | 1261 MB | 1269 MB | +0.6% | ~ |
| `RayShell` | 32.6 s | 33.0 s | +1.2% | 1056 MB | 1058 MB | +0.2% | ~ |
| `Render` | 31.4 s | 30.9 s | -1.8% | 1054 MB | 1113 MB | +5.7% | ~ |
| `Shapes` | 8.8 s | 8.8 s | -0.4% | 552 MB | 552 MB | -0.1% | ~ |
| `SphereCloudBVH` | 4.6 s | 4.6 s | +1.3% | 476 MB | 489 MB | +2.8% | ~ |
| `SphereCloudRays` | 24.5 s | 24.1 s | -1.3% | 963 MB | 962 MB | -0.1% | ~ |
| `SphereContacts` | 4.1 s | 4.1 s | +0.4% | 424 MB | 427 MB | +0.7% | ~ |
| `Tape` | 13.8 s | 13.9 s | +0.7% | 826 MB | 830 MB | +0.5% | ~ |
| **all, in both** | 436.8 s | 436.0 s | -0.2% | 1264 MB | 1269 MB | +0.4% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (1 cases): Tape/sphere-handwritten/signedDistance/float/host

These probes compiled to the same code on both sides and were not run (96 cases): BVH (6), BVHRefit (2), Mesh (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20), SphereCloudBVH (6), SphereContacts (2)

The probes differ between the sides (each side ran its own): Performance/BVHUnion/main.cpp, Performance/BVHUnionMixed/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: d22c602
base-fingerprint: b3d98b4c4254e9729fc5d48c3cb8238021bad874
head: 40e7a91
head-fingerprint: 1dc80b70de91c722b7232e58dbcc7d9e131dbbd1
cases: 224
rounds: 2
reps: 3
date: 2026-11-07
-->

## Device comparison (CUDA): green

**Accepted:** -

`40e7a91` against `d22c602` (origin/main), 2026-11-07.

**VERDICT: no regression (0 major, 0 minor across 224 cases)**

Performance comparison: 0 major, 0 minor, 4 faster, 0 differ, 0 lost, 224 cases

- base: d22c602 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 40e7a91 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHUnion/spheres/query-morton/float/hostdevice` | 385 µs | 386 µs | +0.0% | [-0.5%, +0.6%] | - | ~ | device 15.3x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 840 µs | 844 µs | +0.4% | [+0.1%, +0.8%] | - | ~ | device 10.0x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.52 ms | 4.53 ms | +0.4% | [-0.2%, +0.9%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.67 ms | 4.64 ms | -0.6% | [-0.8%, -0.3%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 579 µs | 580 µs | +0.3% | [-0.5%, +1.1%] | - | ~ | device 13.0x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.37 ms | 1.37 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.77 ms | 6.78 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 7.01 ms | 7.02 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 895 µs | 895 µs | +0.1% | [-0.3%, +0.4%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.20 ms | 2.21 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.46 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 1.8x faster than the host (was 1.9x) |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | - | - | - | - | - | new |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | - | - | - | - | - | new |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | - | - | - | - | - | new |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | - | - | - | - | - | new |  |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 313 µs | 313 µs | -0.2% | [-0.5%, +0.2%] | - | ~ | device 11.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.79 ms | 1.79 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 4.9x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.0 ms | 59.0 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.60 ms | 5.61 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 1.8x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 526 ms | 520 ms | -1.1% | [-1.4%, -0.8%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 48.6 µs | 48.0 µs | -1.1% | [-2.2%, +0.0%] | - | ~ | device 23.6x faster than the host; tape/fused 4.56x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.0 µs | 49.6 µs | -0.7% | [-0.8%, -0.6%] | - | ~ | device 43.1x faster than the host; tape/fused 7.24x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 9.0x faster than the host (was 9.3x); tape/fused 9.96x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.3 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 6.7x faster than the host; tape/fused 1.82x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.4 ms | 31.4 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.5x faster than the host (was 1.5x); tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.5x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 280 ms | 289 ms | +2.9% | [+1.4%, +4.5%] | - | ~ | device 0.4x faster than the host; tape/fused 0.98x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 282 ms | 284 ms | +0.7% | [-1.0%, +2.3%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.28 ms | 8.22 ms | -0.7% | [-2.0%, +0.7%] | +0.0% | ~ | device 10.8x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.25 ms | 8.36 ms | +1.4% | [+1.0%, +1.7%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.20 ms | 8.23 ms | +0.4% | [-0.4%, +1.1%] | +0.0% | ~ | device 10.8x faster than the host (was 11.1x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.30 ms | 8.30 ms | +0.0% | [-2.3%, +2.4%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 676 µs | 675 µs | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 587 µs | 586 µs | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | device 4.4x faster than the host; default/this 1.15x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.04 ms | +0.5% | [-0.1%, +1.0%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 784 µs | 787 µs | +0.5% | [-0.5%, +1.4%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.86x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 737 µs | 743 µs | +0.7% | [-0.3%, +1.7%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 830 µs | 825 µs | -0.7% | [-1.5%, +0.1%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 739 µs | 742 µs | +0.5% | [-0.4%, +1.4%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 734 µs | 727 µs | -0.9% | [-2.2%, +0.3%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 160 ms | -0.2% | [-3.6%, +3.4%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 160 ms | 157 ms | -2.2% | [-4.3%, -0.1%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 162 ms | 165 ms | +1.8% | [+1.1%, +2.5%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.97x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 162 ms | 160 ms | -1.2% | [-6.2%, +4.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.38 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ | device 2.0x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.75 ms | 2.77 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.96 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.71x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.55 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.26 ms | 7.24 ms | -0.2% | [-1.0%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.27 ms | 7.28 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.27 ms | 7.22 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.33 ms | 7.31 ms | -0.3% | [-1.0%, +0.5%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | -0.4% | [-1.6%, +0.9%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.33 ms | 4.33 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.7 ms | 13.7 ms | -0.1% | [-0.7%, +0.6%] | - | ~ | device 3.9x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 65.2 ms | 65.1 ms | -0.1% | [-2.1%, +1.9%] | - | ~ | device 3.1x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 96.1 ms | 94.4 ms | -1.7% | [-2.2%, -1.3%] | - | ~ | device 2.3x faster than the host (was 2.5x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 250 ms | 252 ms | +0.6% | [-0.6%, +1.8%] | - | ~ | device 3.4x faster than the host (was 3.6x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.0 ms | 25.0 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 2.0x faster than the host; tape/fused 2.60x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.8 ms | 26.8 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.2x faster than the host; tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.2 ms | 71.5 ms | +0.5% | [-1.0%, +2.0%] | - | ~ | device 4.1x faster than the host (was 4.3x); tape/fused 3.52x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.4 ms | 23.1 ms | +3.2% | [-1.8%, +8.5%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.1 ms | 44.4 ms | +0.6% | [-0.1%, +1.3%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 97.3 ms | 93.0 ms | -4.4% | [-8.3%, -0.2%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 555 ms | 521 ms | -6.1% | [-8.6%, -3.6%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 848 ms | 846 ms | -0.3% | [-1.5%, +1.0%] | - | ~ | device 0.3x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.08 s | 2.10 s | +0.9% | [-4.3%, +6.4%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 194 ms | 197 ms | +1.4% | [-2.0%, +4.8%] | - | ~ | device 0.2x faster than the host; tape/fused 2.65x (was 2.86x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 304 ms | 313 ms | +3.1% | [-3.0%, +9.7%] | - | ~ | device 0.2x faster than the host (was 0.2x); tape/fused 2.70x (was 2.79x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 599 ms | 601 ms | +0.4% | [-2.5%, +3.4%] | - | ~ | device 0.5x faster than the host; tape/fused 3.48x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 674 µs | 668 µs | -0.7% | [-1.3%, -0.2%] | - | ~ | device 16.9x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 14.6x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 648 µs | 647 µs | -0.2% | [-1.6%, +1.3%] | - | ~ | device 17.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.23 ms | +1.0% | [-0.7%, +2.7%] | - | ~ | device 15.0x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 615 µs | 604 µs | -1.7% | [-3.6%, +0.3%] | - | ~ | device 16.1x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | -0.3% | [-0.9%, +0.2%] | - | ~ | device 14.8x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.18 ms | -0.8% | [-1.1%, -0.5%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 11.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.19 ms | 2.18 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 9.9x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 594 µs | 593 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 2.5x faster than the host; tape/fused 1.14x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 691 µs | 657 µs | -4.9% | [-9.1%, -0.2%] | - | faster | device 2.8x faster than the host (was 2.6x); tape/fused 1.28x (was 1.23x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 672 µs | 674 µs | +0.3% | [+0.1%, +0.4%] | - | ~ | device 3.7x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 847 µs | 843 µs | -0.5% | [-1.2%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | -0.2% | [-0.9%, +0.5%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.3 ms | 27.6 ms | +0.9% | [-0.1%, +1.8%] | - | ~ | device 0.7x faster than the host; hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.6% | [+0.4%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.5% | [-1.1%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.3% | [-0.6%, +0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.6% | [-1.3%, +0.2%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.13x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.6% | [-1.8%, +0.6%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.38 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.48 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.6x faster than the host; marched/traced 3.54x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.99 ms | 3.00 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.3x faster than the host; marched/traced 4.62x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.00 ms | 8.99 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 4.9x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.8 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.21 ms | 2.21 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 3.8x faster than the host; marched/traced 15.84x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.47 ms | 5.48 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.8x faster than the host; marched/traced 12.72x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 34.9 ms | 35.0 ms | +0.4% | [+0.4%, +0.4%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 69.5 ms | 69.7 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.22 ms | 5.22 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.72 ms | 8.71 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.21 ms | 6.20 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 1.2x faster than the host; marched/traced 25.98x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.5 ms | 10.6 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 0.8x faster than the host; marched/traced 23.98x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 158 ms | 161 ms | +1.7% | [-0.1%, +3.6%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 254 ms | 254 ms | -0.1% | [-1.6%, +1.6%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.88 ms | 4.87 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 23.00x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.2 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.3x faster than the host; marched/traced 17.80x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 112 ms | 112 ms | -0.3% | [-3.6%, +3.1%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 220 ms | 218 ms | -0.8% | [-2.3%, +0.8%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.4 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.8 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 673 µs | 673 µs | -0.1% | [-1.0%, +0.8%] | - | ~ | device 17.2x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 673 µs | 670 µs | -0.5% | [-0.9%, +0.0%] | - | ~ | device 17.3x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.2% | [-0.4%, +0.7%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 681 µs | 682 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 837 µs | 843 µs | +0.7% | [+0.1%, +1.4%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 609 µs | 610 µs | +0.2% | [+0.2%, +0.3%] | - | ~ | device 3.3x faster than the host; hit/crossing 1.12x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 632 µs | 631 µs | -0.1% | [-0.5%, +0.2%] | - | ~ | device 3.8x faster than the host; hit/crossing 1.34x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.68 ms | 4.69 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.17 ms | 4.17 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 10.0x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.07 ms | 9.08 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 59.0 ms | 58.3 ms | -1.2% | [-2.6%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 84.2 ms | 83.9 ms | -0.3% | [-1.2%, +0.6%] | - | ~ | device 2.2x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.5 ms | 48.8 ms | +0.6% | [-0.4%, +1.6%] | - | ~ | device 3.0x faster than the host (was 3.3x); hit/crossing 1.19x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.6 ms | 63.7 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 2.4x faster than the host; hit/crossing 1.32x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.4% | [+0.0%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.9% | [-1.3%, -0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.5%, +0.8%] | - | ~ | device 2.0x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.11 ms | 1.11 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.25x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.14x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.5 ms | 64.6 ms | +0.2% | [-0.7%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 122 ms | 123 ms | +0.3% | [-1.7%, +2.3%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.6 ms | 58.6 ms | -0.0% | [-0.6%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 110 ms | 110 ms | -0.2% | [-1.1%, +0.6%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 469 ms | 483 ms | +3.1% | [-1.9%, +9.7%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 707 ms | 709 ms | +0.2% | [-1.0%, +1.5%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 377 ms | 383 ms | +1.4% | [-3.8%, +6.8%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.26x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 533 ms | 532 ms | -0.2% | [-1.3%, +0.9%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.33x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.62 ms | 3.59 ms | -1.0% | [-2.0%, +0.1%] | - | ~ | device 1.2x faster than the host; marched/traced 3.35x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.75 ms | 4.79 ms | +0.8% | [+0.3%, +1.3%] | - | ~ | device 0.9x faster than the host; marched/traced 5.55x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | +0.3% | [+0.0%, +0.6%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.5 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.25 ms | 1.25 ms | +0.6% | [-0.5%, +1.7%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.85 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 94.4 µs | 94.7 µs | +0.3% | [-1.1%, +1.8%] | - | ~ | device 15.5x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 152 µs | 150 µs | -1.2% | [-2.9%, +0.5%] | - | ~ | device 11.0x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 796 µs | 793 µs | -0.4% | [-0.6%, -0.3%] | - | ~ | device 2.9x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 928 µs | 928 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.9 ms | 34.3 ms | -2.0% | [-3.7%, -0.2%] | - | ~ | device 0.1x faster than the host; marched/traced 3.61x (was 3.45x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.7 ms | 36.6 ms | -0.2% | [-1.5%, +1.0%] | - | ~ | device 0.1x faster than the host; marched/traced 6.33x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 121 ms | 124 ms | +2.4% | [+0.3%, +4.6%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 231 ms | 232 ms | +0.6% | [-0.5%, +1.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.2 ms | 16.3 ms | +0.6% | [-6.7%, +8.3%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 21.8 ms | -0.1% | [-1.4%, +1.3%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.05 ms | 1.04 ms | -0.5% | [-1.7%, +0.7%] | - | ~ | device 1.4x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.35 ms | 2.36 ms | +0.6% | [-0.2%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.96 ms | 5.97 ms | +0.2% | [-0.6%, +0.9%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.71 ms | 6.71 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 22.9 ms | 23.2 ms | +1.2% | [+0.7%, +1.7%] | - | ~ | device 2.2x faster than the host; marched/traced 2.68x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.2 ms | 28.3 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 1.9x faster than the host; marched/traced 3.66x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 61.9 ms | 62.1 ms | +0.4% | [-0.2%, +0.9%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 105 ms | 104 ms | -1.3% | [-2.3%, -0.3%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.7 ms | 13.8 ms | +0.7% | [-0.8%, +2.2%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.4 ms | +0.2% | [-1.1%, +1.6%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 304 ms | 303 ms | -0.5% | [-3.6%, +2.8%] | - | ~ | device 0.2x faster than the host; marched/traced 1.86x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 345 ms | 353 ms | +2.3% | [+0.4%, +4.2%] | - | ~ | device 0.2x faster than the host; marched/traced 2.80x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 554 ms | 562 ms | +1.5% | [-0.8%, +3.9%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 996 ms | 989 ms | -0.7% | [-2.1%, +0.6%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 189 ms | 189 ms | -0.3% | [-3.0%, +2.6%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 245 ms | 238 ms | -2.6% | [-5.1%, -0.1%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.14 ms | 4.14 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.15 ms | 4.15 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.88 ms | +0.0% | [-0.9%, +1.0%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.59 ms | 7.59 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 1.6x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.58 ms | 7.58 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 1.6x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.3 ms | 23.3 ms | -0.3% | [-1.3%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.00 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ | device 7.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.75 ms | 4.76 ms | +0.2% | [-0.0%, +0.3%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | +0.1% | [-0.0%, +0.1%] | +0.0% | ~ | device 5.2x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.1 ms | 24.1 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.4 ms | 30.4 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.1 ms | 44.2 ms | +0.2% | [-0.3%, +0.8%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.8 ms | 45.8 ms | -0.2% | [-0.9%, +0.6%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 126 ms | 122 ms | -3.2% | [-4.5%, -1.8%] | +0.0% | faster | device 0.7x faster than the host (was 0.7x); tape/fused 1.05x (was 1.02x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 128 ms | 127 ms | -0.5% | [-4.0%, +3.1%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 128 ms | 128 ms | -0.2% | [-3.0%, +2.7%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 133 ms | 133 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 219 µs | +0.6% | [+0.4%, +0.8%] | - | ~ | device 11.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.7 µs | -0.4% | [-0.6%, -0.2%] | - | ~ | device 23.4x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 359 µs | -0.1% | [-0.3%, +0.0%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.01 ms | 2.00 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.3 µs | 72.0 µs | -0.4% | [-0.7%, +0.0%] | - | ~ | device 7.0x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.4 µs | 47.4 µs | -0.0% | [-0.5%, +0.5%] | - | ~ | device 5.0x faster than the host (was 4.7x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 651 µs | +0.0% | [-0.1%, +0.2%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 11.9x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 475 µs | 476 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 7.4x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.4 µs | 48.3 µs | -0.1% | [-0.8%, +0.6%] | - | ~ | device 26.5x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 857 µs | 858 µs | +0.1% | [+0.0%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.73 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.76 ms | 6.75 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 191 µs | +0.2% | [+0.2%, +0.3%] | - | ~ | device 3.0x faster than the host (was 3.1x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.0x faster than the host (was 2.1x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.77 ms | 2.77 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 2989.6 s → 3002.3 s (+0.4%), the largest peak 5256 MB → 5256 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.1 s | 12.1 s | +0.1% | 532 MB | 535 MB | +0.5% | ~ |
| `BVHRefit` | 6.0 s | 6.1 s | +1.5% | 461 MB | 465 MB | +0.9% | ~ |
| `BVHUnion` | 19.8 s | 19.8 s | -0.3% | 842 MB | 854 MB | +1.4% | ~ |
| `BVHUnionMixed` | - | 10.1 s | | - | 556 MB | | new |
| `ClosestPrimitive` | 115.0 s | 114.0 s | -0.8% | 1216 MB | 1218 MB | +0.2% | ~ |
| `Fused` | 45.2 s | 46.0 s | +1.8% | 1028 MB | 1030 MB | +0.3% | ~ |
| `LaunchTuning` | 459.5 s | 459.7 s | +0.0% | 3385 MB | 3385 MB | +0.0% | ~ |
| `Mesh` | 19.0 s | 18.8 s | -1.1% | 733 MB | 737 MB | +0.5% | ~ |
| `MeshCSG` | 331.8 s | 333.0 s | +0.4% | 4505 MB | 4505 MB | -0.0% | ~ |
| `MeshReconcile` | 8.8 s | 9.0 s | +2.2% | 461 MB | 465 MB | +0.8% | ~ |
| `PointCloud` | 11.3 s | 11.8 s | +4.3% | 500 MB | 503 MB | +0.6% | ~ |
| `PointCloudBVHBuild` | 6.0 s | 6.3 s | +4.0% | 461 MB | 465 MB | +0.8% | ~ |
| `PointCloudHashGridBuild` | 5.3 s | 5.3 s | -0.7% | 461 MB | 465 MB | +0.9% | ~ |
| `Ray` | 171.2 s | 170.5 s | -0.4% | 3281 MB | 3281 MB | -0.0% | ~ |
| `RayCSG` | 207.5 s | 207.8 s | +0.1% | 3632 MB | 3632 MB | +0.0% | ~ |
| `RayCrossing` | 386.2 s | 387.5 s | +0.3% | 3577 MB | 3577 MB | +0.0% | ~ |
| `RayParticles` | 439.2 s | 445.2 s | +1.4% | 5256 MB | 5256 MB | +0.0% | ~ |
| `RayShell` | 277.7 s | 280.6 s | +1.1% | 4858 MB | 4858 MB | -0.0% | ~ |
| `Render` | 202.8 s | 202.0 s | -0.4% | 4338 MB | 4338 MB | -0.0% | ~ |
| `Shapes` | 27.3 s | 27.6 s | +0.8% | 520 MB | 523 MB | +0.6% | ~ |
| `SphereCloudBVH` | 8.3 s | 8.0 s | -3.4% | 461 MB | 466 MB | +0.9% | ~ |
| `SphereCloudRays` | 162.3 s | 163.8 s | +0.9% | 3542 MB | 3542 MB | +0.0% | ~ |
| `SphereContacts` | 7.5 s | 7.6 s | +1.6% | 461 MB | 465 MB | +0.9% | ~ |
| `Tape` | 60.0 s | 60.0 s | +0.1% | 850 MB | 853 MB | +0.4% | ~ |
| **all, in both** | 2989.6 s | 3002.3 s | +0.4% | 5256 MB | 5256 MB | +0.0% | ~ |

Device kernels: 146 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 8 new, 6 gone. Spills are bytes stored/loaded per thread.


Flagged after 2 rounds, and judged on 10 (2 cases): RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice, RayParticles/armadillo/step-morton/float/hostdevice

These probes compiled to the same code on both sides and were not run (102 cases): BVH (12), BVHRefit (2), Mesh (8), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Shapes (20), SphereCloudBVH (8), SphereContacts (2)

The probes differ between the sides (each side ran its own): Performance/BVHUnion/main.cpp, Performance/BVHUnionMixed/main.cpp

<!-- END receipt cuda -->
