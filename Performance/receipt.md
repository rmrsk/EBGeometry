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
base: e43fe59
base-fingerprint: 1dc80b70de91c722b7232e58dbcc7d9e131dbbd1
head: df295cf
head-fingerprint: 057dd7f4da87e97c9c13841bccb1664f96e58730
cases: 120
rounds: 2
reps: 3
date: 2026-11-07
-->

## Host comparison: green

**Accepted:** -

`df295cf` against `e43fe59` (origin/main), 2026-11-07.

**VERDICT: no regression (0 major, 0 minor across 120 cases)**

Performance comparison: 0 major, 0 minor, 6 faster, 0 differ, 0 lost, 120 cases

- base: e43fe59 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: df295cf (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.6 ms | 19.2 ms | -1.9% | [-2.9%, -1.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 44.3 ms | 41.2 ms | -7.0% | [-7.3%, -6.8%] | +0.0% | faster |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.1 ms | 73.0 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.0 ms | 19.9 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 46.2 ms | 44.2 ms | -4.4% | [-6.1%, -2.6%] | +0.0% | faster |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.1 ms | 81.1 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.46 ms | 4.49 ms | +0.6% | [-0.0%, +1.2%] | - | ~ | tape/fused 2.86x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | tape/fused 2.02x |
| `Fused/chain16/signedDistance/float/host` | 974 µs | 970 µs | -0.4% | [-1.2%, +0.4%] | - | ~ | tape/fused 17.04x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.2 ms | 46.0 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.5 ms | 46.4 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.52 ms | 4.63 ms | +2.4% | [+1.6%, +3.2%] | - | ~ | tape/fused 2.85x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.9 ms | +0.6% | [-0.2%, +1.3%] | - | ~ | tape/fused 2.03x |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.91 ms | +0.4% | [-1.0%, +1.9%] | - | ~ | tape/fused 4.20x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.6 ms | 52.5 ms | -0.3% | [-1.1%, +0.5%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.9 ms | 52.8 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.5 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.8 ms | 27.0 ms | +1.0% | [+0.1%, +2.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 320 ms | 318 ms | -0.4% | [-0.7%, -0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 238 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.3 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | tape/fused 3.53x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.7 ms | 100 ms | +0.6% | [-0.0%, +1.2%] | +0.0% | ~ | tape/fused 2.38x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.5 ms | 29.7 ms | +0.4% | [-0.5%, +1.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 29.8 ms | 30.0 ms | +0.7% | [-0.2%, +1.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 356 ms | -0.3% | [-0.7%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 256 ms | 255 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.5 ms | 97.1 ms | -0.4% | [-1.4%, +0.7%] | +0.0% | ~ | tape/fused 3.67x |
| `MeshCSG/smooth-fused/frame/double/host` | 107 ms | 107 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | tape/fused 2.39x |
| `Ray/mesh/firstHit/float/host` | 46.7 ms | 46.5 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.5 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.5 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 79.1 ms | 78.7 ms | -0.4% | [-1.0%, +0.1%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.4 ms | 65.6 ms | +0.4% | [-0.1%, +0.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.25 ms | 5.28 ms | +0.5% | [-1.1%, +2.1%] | - | ~ | tape/fused 1.45x |
| `Ray/csg-tape/firstHit/float/host` | 7.73 ms | 7.64 ms | -1.2% | [-2.4%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.6 ms | 52.4 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.3 ms | 51.1 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.8 ms | 45.1 ms | +0.6% | [+0.2%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.0 ms | 87.6 ms | -0.4% | [-0.8%, -0.0%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.7 ms | 74.2 ms | +0.6% | [-0.3%, +1.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.50 ms | 5.49 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | tape/fused 1.42x |
| `Ray/csg-tape/firstHit/double/host` | 8.00 ms | 7.80 ms | -2.5% | [-3.6%, -1.4%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.3 ms | 17.5 ms | +1.1% | [+0.0%, +2.2%] | - | ~ | marched/traced 6.83x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 117 ms | 120 ms | +2.3% | [+1.7%, +2.8%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 21.9 ms | -0.7% | [-3.1%, +1.7%] | +0.0% | ~ | marched/traced 6.70x (was 6.98x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 154 ms | 147 ms | -4.7% | [-5.5%, -4.0%] | +0.0% | faster |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.3 ms | 35.3 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.6 ms | 18.7 ms | -4.7% | [-5.9%, -3.5%] | - | faster | marched/traced 6.81x (was 6.32x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 124 ms | 127 ms | +2.6% | [+2.4%, +2.9%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.0 ms | 23.3 ms | +1.2% | [+0.2%, +2.1%] | +0.0% | ~ | marched/traced 6.43x (was 6.95x) |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 160 ms | 150 ms | -6.3% | [-6.5%, -6.1%] | +0.0% | faster |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.6 ms | 37.7 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.3 ms | 46.4 ms | +0.4% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.3 ms | 46.3 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.82 ms | 7.66 ms | -2.0% | [-2.5%, -1.6%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.29 ms | 6.11 ms | -2.8% | [-3.7%, -2.0%] | +0.0% | ~ | hit/crossing 1.25x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 177 ms | 176 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 157 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 69.0 ms | 68.6 ms | -0.6% | [-0.7%, -0.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.4 ms | 56.5 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.4 ms | 52.4 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.3 ms | 52.3 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.87 ms | 7.94 ms | +0.9% | [-0.1%, +1.9%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.20 ms | 6.23 ms | +0.5% | [+0.2%, +0.7%] | +0.0% | ~ | hit/crossing 1.27x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 199 ms | 199 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 177 ms | 177 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.6 ms | 77.2 ms | -0.5% | [-1.0%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.3 ms | 64.2 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | hit/crossing 1.20x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.0 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ | marched/traced 4.68x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.8 ms | 51.6 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.03 ms | 8.00 ms | -0.3% | [-1.1%, +0.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.92 ms | 1.92 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.81 ms | 6.91 ms | +1.5% | [+1.1%, +1.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.6 ms | -0.8% | [-1.2%, -0.4%] | +0.0% | ~ | marched/traced 5.24x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.8 ms | 61.0 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.64 ms | 8.64 ms | -0.1% | [-0.6%, +0.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.92 ms | 1.93 ms | +0.7% | [-0.3%, +1.6%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.20 ms | 7.20 ms | -0.0% | [-0.7%, +0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 239 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 328 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 240 ms | 237 ms | -1.0% | [-1.7%, -0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 251 ms | 252 ms | +0.6% | [+0.4%, +0.8%] | +0.0% | ~ | marched/traced 1.46x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 369 ms | 369 ms | -0.0% | [-0.7%, +0.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 250 ms | 251 ms | +0.5% | [+0.4%, +0.6%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 18.5 ms | 18.8 ms | +1.7% | [-0.1%, +3.5%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 18.7 ms | 18.9 ms | +0.9% | [-0.2%, +1.9%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 18.5 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 21.5 ms | 21.3 ms | -0.8% | [-1.5%, -0.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.3 ms | 21.6 ms | +1.0% | [-0.2%, +2.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.3 ms | 20.6 ms | +1.1% | [+0.5%, +1.8%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.6 ms | 74.7 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 297 ms | 295 ms | -0.5% | [-0.7%, -0.4%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 298 ms | 297 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 87.1 ms | 86.4 ms | -0.8% | [-1.2%, -0.5%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 346 ms | 346 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 349 ms | 347 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.8 ms | -0.4% | [-1.7%, +0.9%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.57 ms | 4.57 ms | -0.0% | [-0.7%, +0.7%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.1 ms | 24.9 ms | -0.5% | [-0.9%, -0.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.4 ms | 39.8 ms | -1.4% | [-1.6%, -1.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.77 ms | 1.75 ms | -0.8% | [-1.6%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 138 µs | 141 µs | +2.0% | [-0.6%, +4.6%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.0 ms | 29.7 ms | -1.0% | [-1.5%, -0.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.73 ms | 5.68 ms | -0.8% | [-1.7%, +0.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.2 ms | 16.5 ms | +2.2% | [-0.5%, +4.9%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.74 ms | 4.64 ms | -2.1% | [-2.7%, -1.5%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 13.2 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.59 ms | 4.64 ms | +1.0% | [-0.7%, +2.8%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.2 ms | 26.1 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 41.9 ms | 41.8 ms | -0.3% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.76 ms | 1.78 ms | +1.6% | [-0.1%, +3.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 298 µs | 284 µs | -4.9% | [-9.0%, -0.5%] | - | faster |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.8 ms | 29.7 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.75 ms | 7.79 ms | +0.5% | [-0.4%, +1.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.2 ms | 16.4 ms | +1.1% | [+0.3%, +2.0%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.83 ms | 7.85 ms | +0.2% | [-2.0%, +2.5%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | - | - | - | - | - | new |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | - | - | - | - | - | new |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | - | - | - | - | - | new |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | - | - | - | - | - | new |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 444.6 s → 452.9 s (+1.9%), the largest peak 1269 MB → 1270 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 8.8 s | 8.9 s | +2.1% | 557 MB | 557 MB | -0.0% | ~ |
| `BVHRefit` | 3.8 s | 3.8 s | +0.4% | 456 MB | 456 MB | +0.0% | ~ |
| `BVHUnion` | 14.6 s | 15.1 s | +4.0% | 815 MB | 815 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.1 s | 7.2 s | +1.2% | 557 MB | 557 MB | +0.0% | ~ |
| `ClosestPrimitive` | 25.2 s | 25.4 s | +0.6% | 1207 MB | 1208 MB | +0.1% | ~ |
| `Fused` | 20.4 s | 21.1 s | +3.6% | 991 MB | 993 MB | +0.2% | ~ |
| `LaunchTuning` | 32.7 s | 33.7 s | +3.0% | 1108 MB | 1109 MB | +0.1% | ~ |
| `Mesh` | 14.0 s | 14.3 s | +1.8% | 770 MB | 770 MB | +0.0% | ~ |
| `MeshCSG` | 34.1 s | 34.9 s | +2.3% | 1153 MB | 1155 MB | +0.2% | ~ |
| `MeshReconcile` | 6.2 s | 6.2 s | +0.6% | 534 MB | 534 MB | +0.0% | ~ |
| `PointCloud` | 6.9 s | 7.2 s | +3.7% | 557 MB | 557 MB | -0.0% | ~ |
| `PointCloudBVHBuild` | 3.5 s | 3.6 s | +2.0% | 410 MB | 410 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.0 s | 3.0 s | +2.5% | 402 MB | 402 MB | -0.0% | ~ |
| `Ray` | 31.9 s | 32.4 s | +1.6% | 1146 MB | 1149 MB | +0.3% | ~ |
| `RayCSG` | 28.8 s | 29.7 s | +3.0% | 963 MB | 969 MB | +0.6% | ~ |
| `RayCrossing` | 41.1 s | 40.5 s | -1.4% | 1265 MB | 1270 MB | +0.4% | ~ |
| `RayParticles` | 43.3 s | 43.7 s | +1.0% | 1269 MB | 1269 MB | +0.0% | ~ |
| `RayShell` | 32.8 s | 33.6 s | +2.6% | 1058 MB | 1062 MB | +0.4% | ~ |
| `Render` | 31.1 s | 31.9 s | +2.8% | 1114 MB | 1115 MB | +0.0% | ~ |
| `Shapes` | 8.8 s | 8.9 s | +1.7% | 552 MB | 552 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.6 s | 4.7 s | +1.2% | 478 MB | 492 MB | +3.0% | ~ |
| `SphereCloudRays` | 24.2 s | 24.7 s | +2.0% | 962 MB | 959 MB | -0.3% | ~ |
| `SphereContacts` | 4.1 s | 4.2 s | +0.8% | 427 MB | 427 MB | -0.0% | ~ |
| `Tape` | 13.9 s | 14.2 s | +2.6% | 830 MB | 830 MB | -0.0% | ~ |
| `TapeBVHUnion` | - | 12.0 s | | - | 785 MB | | new |
| **all, in both** | 444.6 s | 452.9 s | +1.9% | 1269 MB | 1270 MB | +0.1% | ~ |


These probes compiled to the same code on both sides and were not run (112 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), Mesh (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20), SphereCloudBVH (6), SphereContacts (2)

The probes differ between the sides (each side ran its own): Performance/TapeBVHUnion/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: e43fe59
base-fingerprint: 1dc80b70de91c722b7232e58dbcc7d9e131dbbd1
head: df295cf
head-fingerprint: 057dd7f4da87e97c9c13841bccb1664f96e58730
cases: 212
rounds: 2
reps: 3
date: 2026-11-07
-->

## Device comparison (CUDA): red

**Accepted:** Accept

`df295cf` against `e43fe59` (origin/main), 2026-11-07.

**VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above)**

Performance comparison: 1 major, 2 minor, 12 faster, 0 differ, 0 lost, 212 cases

- base: e43fe59 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: df295cf (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 312 µs | 313 µs | +0.4% | [-0.1%, +0.8%] | - | ~ | device 11.7x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.79 ms | 1.30 ms | -27.3% | [-27.3%, -27.2%] | - | faster | device 6.3x faster than the host (was 4.9x) |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 3.3x faster than the host (was 3.8x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.59 ms | 5.39 ms | -3.7% | [-3.9%, -3.6%] | - | faster | device 1.6x faster than the host (was 1.8x) |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 518 ms | 518 ms | +0.1% | [-0.7%, +0.9%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.1 µs | +0.4% | [-0.0%, +0.8%] | - | ~ | device 22.8x faster than the host (was 23.5x); tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.9 µs | 49.7 µs | -0.3% | [-0.6%, +0.1%] | - | ~ | device 43.5x faster than the host; tape/fused 7.23x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.0 µs | 47.8 µs | -0.4% | [-1.1%, +0.4%] | - | ~ | device 9.6x faster than the host; tape/fused 9.97x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.2 ms | -0.8% | [-1.7%, +0.0%] | - | ~ | device 6.8x faster than the host (was 7.5x); tape/fused 1.81x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.4 ms | 31.1 ms | -0.8% | [-1.0%, -0.7%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 809 µs | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 284 ms | 285 ms | +0.6% | [+0.3%, +0.9%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 289 ms | 288 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.20 ms | 8.21 ms | +0.1% | [-1.2%, +1.4%] | +0.0% | ~ | device 10.8x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.30 ms | 8.27 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.27 ms | 8.23 ms | -0.4% | [-0.8%, -0.0%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.15 ms | 8.18 ms | +0.4% | [-0.4%, +1.3%] | +0.0% | ~ | device 10.9x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 674 µs | 671 µs | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ | device 3.7x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 586 µs | 1.75 ms | +199.3% | [+198.4%, +199.7%] | +0.0% | **MAJOR** | device 1.4x faster than the host (was 4.4x); default/this 0.38x (was 1.15x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.03 ms | -1.0% | [-1.5%, -0.4%] | +0.0% | ~ | device 2.4x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 786 µs | 811 µs | +3.2% | [+2.8%, +3.7%] | +0.0% | minor | device 3.1x faster than the host (was 3.2x); default/this 0.83x (was 0.86x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 738 µs | 738 µs | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | device 12.1x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 822 µs | 825 µs | +0.4% | [-0.4%, +1.1%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 739 µs | 741 µs | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 733 µs | 730 µs | -0.5% | [-0.9%, -0.1%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 161 ms | 161 ms | -0.0% | [-0.8%, +0.8%] | +0.0% | ~ | device 0.6x faster than the host (was 0.6x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 159 ms | 160 ms | +1.0% | [+0.5%, +1.5%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 165 ms | 162 ms | -1.7% | [-3.9%, +0.6%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 161 ms | 158 ms | -1.6% | [-2.7%, +1.7%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.76 ms | 2.78 ms | +0.7% | [+0.2%, +1.3%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.96 ms | 1.95 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.71x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.56 ms | +0.7% | [+0.3%, +1.1%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.26 ms | 7.30 ms | +0.5% | [+0.1%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.28 ms | 7.31 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.22 ms | 7.23 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.32 ms | 7.30 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.56 ms | 2.55 ms | -0.6% | [-1.6%, +0.4%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.33 ms | 4.29 ms | -1.1% | [-1.6%, -0.6%] | - | ~ | device 3.3x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.6 ms | 13.7 ms | +0.8% | [+0.4%, +1.1%] | - | ~ | device 3.9x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 65.4 ms | 62.6 ms | -4.2% | [-5.4%, -2.8%] | - | faster | device 3.2x faster than the host (was 3.1x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 96.2 ms | 92.4 ms | -3.9% | [-4.1%, -3.6%] | - | faster | device 2.4x faster than the host (was 2.2x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 254 ms | 251 ms | -1.0% | [-4.1%, +2.1%] | - | ~ | device 3.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.6 ms | 25.0 ms | +1.6% | [+1.5%, +1.7%] | - | ~ | device 2.0x faster than the host; tape/fused 2.51x (was 2.66x) |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.1 ms | 26.9 ms | -0.8% | [-1.5%, +0.0%] | - | ~ | device 2.2x faster than the host; tape/fused 3.44x (was 3.55x) |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.9 ms | 71.5 ms | -0.6% | [-1.6%, +0.4%] | - | ~ | device 4.7x faster than the host (was 4.1x); tape/fused 3.51x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.6 ms | 22.4 ms | -1.0% | [-4.7%, +2.8%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 43.4 ms | 44.4 ms | +2.3% | [+0.9%, +3.7%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 95.4 ms | 93.7 ms | -1.9% | [-4.5%, +0.9%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 550 ms | 520 ms | -5.6% | [-8.6%, -2.4%] | - | faster | device 0.4x faster than the host (was 0.5x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 848 ms | 845 ms | -0.4% | [-1.4%, +0.6%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.05 s | 2.06 s | +0.6% | [-1.2%, +2.5%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 194 ms | 198 ms | +2.4% | [-2.0%, +7.1%] | - | ~ | device 0.2x faster than the host; tape/fused 2.62x (was 2.84x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 311 ms | 296 ms | -4.7% | [-12.0%, +3.8%] | - | ~ | device 0.2x faster than the host (was 0.2x); tape/fused 2.85x (was 2.73x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 608 ms | 609 ms | +0.1% | [-8.0%, +8.8%] | - | ~ | device 0.5x faster than the host; tape/fused 3.38x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 678 µs | 679 µs | +0.1% | [-2.6%, +2.9%] | - | ~ | device 16.6x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 14.5x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 651 µs | 636 µs | -2.4% | [-3.1%, -1.7%] | - | ~ | device 17.5x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.22 ms | -0.1% | [-0.9%, +0.7%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 612 µs | 601 µs | -1.9% | [-2.5%, -1.3%] | - | ~ | device 16.1x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | +0.1% | [-0.8%, +1.0%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.19 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 12.0x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 10.0x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.19 ms | 2.18 ms | -0.4% | [-0.8%, -0.1%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 593 µs | 608 µs | +2.6% | [-0.0%, +5.2%] | - | ~ | device 2.5x faster than the host (was 2.5x); tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 719 µs | 661 µs | -8.1% | [-9.0%, -7.1%] | - | faster | device 2.8x faster than the host (was 2.5x); tape/fused 1.28x (was 1.18x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 673 µs | 676 µs | +0.4% | [+0.2%, +0.7%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 845 µs | 847 µs | +0.2% | [-0.9%, +1.3%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.3 ms | 27.5 ms | +0.5% | [+0.4%, +0.6%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 14.9 ms | +0.5% | [-0.5%, +1.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.9% | [-1.6%, -0.2%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.4 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.0 ms | 14.9 ms | -0.6% | [-1.0%, -0.2%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.1 ms | +0.3% | [-0.2%, +0.7%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.3x faster than the host; tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.36 ms | 2.38 ms | +0.7% | [+0.4%, +1.0%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.5% | [+0.5%, +0.6%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 3.0x faster than the host (was 2.6x); marched/traced 3.55x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.00 ms | 3.00 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.7x faster than the host (was 2.3x); marched/traced 4.61x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.00 ms | 9.01 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 4.2x faster than the host (was 4.9x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.8 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 3.2x faster than the host (was 3.7x) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.21 ms | 2.20 ms | -0.6% | [-0.6%, -0.5%] | - | ~ | device 3.8x faster than the host; marched/traced 13.22x (was 15.82x) |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.48 ms | 5.49 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 2.8x faster than the host; marched/traced 10.01x (was 12.71x) |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 34.9 ms | 29.0 ms | -16.9% | [-17.3%, -16.5%] | - | faster | device 2.3x faster than the host (was 2.0x) |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 69.6 ms | 55.0 ms | -21.0% | [-21.2%, -20.9%] | - | faster | device 2.0x faster than the host (was 1.7x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.22 ms | 5.23 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.71 ms | 8.76 ms | +0.7% | [+0.6%, +0.7%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.19 ms | 6.17 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 1.2x faster than the host (was 1.2x); marched/traced 26.02x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 0.7x faster than the host (was 0.8x); marched/traced 23.84x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 161 ms | 161 ms | +0.0% | [-0.9%, +1.0%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 257 ms | 252 ms | -2.0% | [-3.4%, -0.5%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.85 ms | 4.87 ms | +0.6% | [+0.3%, +0.9%] | - | ~ | device 1.8x faster than the host; marched/traced 23.05x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.3 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 1.3x faster than the host; marched/traced 17.31x (was 18.22x) |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 111 ms | 112 ms | +1.3% | [+0.2%, +2.4%] | - | ~ | device 0.6x faster than the host (was 0.7x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 223 ms | 213 ms | -4.6% | [-5.4%, -3.8%] | - | faster | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.2 ms | 19.2 ms | +0.5% | [+0.1%, +0.9%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 676 µs | 675 µs | -0.1% | [-1.9%, +1.7%] | - | ~ | device 17.1x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 675 µs | 674 µs | -0.2% | [-0.8%, +0.3%] | - | ~ | device 17.1x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.3% | [-1.2%, +0.5%] | - | ~ | device 15.1x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 680 µs | 728 µs | +7.1% | [+6.6%, +7.5%] | - | minor | device 3.5x faster than the host (was 3.9x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 835 µs | 844 µs | +1.0% | [+0.4%, +1.7%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 610 µs | 627 µs | +2.7% | [+2.4%, +2.9%] | - | ~ | device 3.2x faster than the host (was 3.3x); hit/crossing 1.16x (was 1.11x) |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 632 µs | 647 µs | +2.5% | [+2.1%, +2.9%] | - | ~ | device 3.6x faster than the host (was 3.8x); hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.69 ms | 4.69 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 6.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.17 ms | 4.18 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 9.9x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.06 ms | 9.09 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 6.7x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 57.8 ms | 55.9 ms | -3.2% | [-4.6%, -1.7%] | - | faster | device 3.8x faster than the host (was 3.0x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 84.2 ms | 79.6 ms | -5.5% | [-6.9%, -4.2%] | - | faster | device 2.4x faster than the host (was 2.2x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.5 ms | 48.7 ms | +0.4% | [-0.7%, +1.4%] | - | ~ | device 3.5x faster than the host (was 3.0x); hit/crossing 1.15x (was 1.19x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 64.0 ms | 63.1 ms | -1.3% | [-2.0%, -0.6%] | - | ~ | device 2.5x faster than the host; hit/crossing 1.26x (was 1.32x) |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.1% | [-0.7%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 16.9 ms | 17.0 ms | +0.5% | [+0.3%, +0.7%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 2.0x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.11 ms | 1.09 ms | -1.4% | [-1.6%, -1.1%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.27x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.17 ms | 1.16 ms | -1.1% | [-1.8%, -0.4%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.3 ms | 64.7 ms | +0.7% | [+0.4%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 121 ms | 122 ms | +1.2% | [+0.9%, +1.4%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.5 ms | 58.5 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.11x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 113 ms | 110 ms | -2.1% | [-3.1%, -1.0%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.11x (was 1.07x) |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 476 ms | 473 ms | -0.7% | [-4.2%, +3.0%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 710 ms | 718 ms | +1.2% | [-1.2%, +3.7%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 369 ms | 378 ms | +2.4% | [-1.7%, +6.4%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.25x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 536 ms | 532 ms | -0.9% | [-2.1%, +0.4%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.35x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.58 ms | 3.58 ms | -0.1% | [-2.3%, +2.2%] | - | ~ | device 1.2x faster than the host; marched/traced 3.35x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.79 ms | 4.76 ms | -0.6% | [-1.2%, +0.1%] | - | ~ | device 0.9x faster than the host; marched/traced 5.59x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.5 ms | 26.6 ms | +0.3% | [+0.2%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.25 ms | 1.25 ms | -0.3% | [-1.8%, +1.1%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.83 ms | 1.84 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 96.0 µs | 96.0 µs | -0.0% | [-0.7%, +0.6%] | - | ~ | device 15.3x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 151 µs | 150 µs | -0.3% | [-0.9%, +0.3%] | - | ~ | device 10.8x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 795 µs | 796 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 928 µs | 932 µs | +0.4% | [+0.2%, +0.6%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.3 ms | 35.1 ms | -0.7% | [-6.7%, +5.8%] | - | ~ | device 0.1x faster than the host; marched/traced 3.39x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.7 ms | 36.3 ms | -1.1% | [-3.1%, +1.0%] | - | ~ | device 0.1x faster than the host; marched/traced 6.41x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 121 ms | 119 ms | -1.8% | [-4.7%, +1.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 235 ms | 232 ms | -1.3% | [-2.3%, -0.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.4 ms | 16.5 ms | +0.4% | [-1.7%, +4.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.7 ms | 21.6 ms | -0.5% | [-1.1%, +0.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.10 ms | 1.12 ms | +1.8% | [-6.7%, +11.2%] | - | ~ | device 1.3x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.32 ms | 2.35 ms | +1.3% | [+1.3%, +1.3%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.98 ms | 6.03 ms | +1.0% | [+0.7%, +1.2%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.72 ms | 6.72 ms | +0.0% | [-0.9%, +0.9%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.3 ms | 23.6 ms | +1.3% | [-0.4%, +3.1%] | - | ~ | device 2.1x faster than the host; marched/traced 2.63x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.2 ms | 28.3 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 2.0x faster than the host; marched/traced 3.65x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.3 ms | 62.1 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 103 ms | 103 ms | +0.3% | [-0.9%, +1.5%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.7 ms | 13.7 ms | -0.3% | [-1.1%, +0.5%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.3 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 304 ms | 309 ms | +1.3% | [-2.0%, +5.6%] | - | ~ | device 0.2x faster than the host; marched/traced 1.83x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 350 ms | 346 ms | -1.1% | [-4.2%, +2.2%] | - | ~ | device 0.2x faster than the host; marched/traced 2.88x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 559 ms | 564 ms | +0.9% | [-3.0%, +5.0%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.00 s | 997 ms | -0.3% | [-1.2%, +0.6%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 192 ms | 187 ms | -2.6% | [-4.1%, -1.1%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 239 ms | 238 ms | -0.4% | [-5.4%, +4.9%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.14 ms | 4.22 ms | +1.9% | [+1.8%, +2.0%] | - | ~ | device 2.8x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.15 ms | 4.22 ms | +1.8% | [+1.6%, +1.9%] | - | ~ | device 2.8x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.89 ms | 1.90 ms | +1.0% | [-0.1%, +2.2%] | - | ~ | device 8.8x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.60 ms | 7.61 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 1.6x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.58 ms | 7.62 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 1.6x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.3 ms | 23.5 ms | +0.8% | [-0.1%, +1.8%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.01 ms | 3.00 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ | device 7.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.75 ms | 4.77 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 5.2x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.1 ms | 24.1 ms | -0.1% | [-0.1%, -0.1%] | +0.0% | ~ | device 3.8x faster than the host (was 4.0x); tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | +0.2% | [+0.0%, +0.3%] | +0.0% | ~ | device 4.2x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.3 ms | 30.3 ms | -0.1% | [-0.1%, -0.1%] | +0.0% | ~ | device 3.1x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.1 ms | 44.4 ms | +0.5% | [-0.3%, +1.4%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.6 ms | 45.6 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 121 ms | 123 ms | +1.6% | [-0.5%, +3.8%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.05x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 128 ms | 125 ms | -1.8% | [-4.1%, +0.7%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.07x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 128 ms | 129 ms | +0.6% | [-2.6%, +3.9%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 134 ms | 134 ms | +0.5% | [-3.3%, +4.3%] | +0.0% | ~ | device 0.7x faster than the host (was 0.8x) |
| `Tape/csg/signedDistance/float/hostdevice` | 219 µs | 218 µs | -0.1% | [-0.3%, -0.0%] | - | ~ | device 11.2x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 47.9 µs | +0.3% | [+0.2%, +0.3%] | - | ~ | device 23.7x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 360 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 2.01 ms | 1.56 ms | -22.3% | [-22.5%, -22.2%] | - | faster | device 9.1x faster than the host (was 7.1x) |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.3 µs | 72.2 µs | -0.1% | [-0.3%, -0.0%] | - | ~ | device 7.2x faster than the host (was 7.0x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.5 µs | 47.7 µs | +0.4% | [-0.5%, +1.4%] | - | ~ | device 5.0x faster than the host (was 5.2x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 651 µs | +0.1% | [-0.1%, +0.2%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.0% | [-0.1%, +0.0%] | - | ~ | device 11.9x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 476 µs | 476 µs | +0.0% | [-0.2%, +0.3%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.2 µs | 48.3 µs | +0.2% | [+0.1%, +0.4%] | - | ~ | device 27.5x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 857 µs | 858 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.73 ms | 1.74 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.76 ms | 6.64 ms | -1.7% | [-1.8%, -1.6%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 192 µs | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.8x faster than the host (was 2.9x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.0% | [-0.1%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | - | - | - | - | - | new |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | - | - | - | - | - | new |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 3007.0 s → 3468.9 s (+15.4%), the largest peak 5256 MB → 5633 MB; 10 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.0 s | 12.1 s | +0.2% | 535 MB | 535 MB | -0.0% | ~ |
| `BVHRefit` | 6.0 s | 6.0 s | +0.1% | 465 MB | 465 MB | +0.0% | ~ |
| `BVHUnion` | 19.6 s | 19.8 s | +0.8% | 854 MB | 854 MB | +0.0% | ~ |
| `BVHUnionMixed` | 10.1 s | 10.1 s | +0.2% | 558 MB | 558 MB | +0.0% | ~ |
| `ClosestPrimitive` | 113.7 s | 128.4 s | +12.9% | 1218 MB | 1220 MB | +0.1% | **minor** |
| `Fused` | 45.7 s | 48.0 s | +5.0% | 1030 MB | 1031 MB | +0.1% | ~ |
| `LaunchTuning` | 458.5 s | 497.8 s | +8.6% | 3385 MB | 3806 MB | +12.4% | **minor** |
| `Mesh` | 18.7 s | 18.8 s | +0.3% | 737 MB | 738 MB | +0.0% | ~ |
| `MeshCSG` | 335.9 s | 356.7 s | +6.2% | 4506 MB | 5100 MB | +13.2% | **minor** |
| `MeshReconcile` | 8.9 s | 9.0 s | +1.0% | 465 MB | 465 MB | +0.0% | ~ |
| `PointCloud` | 11.4 s | 11.6 s | +1.8% | 503 MB | 503 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 6.2 s | 6.1 s | -1.8% | 465 MB | 465 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 5.3 s | 5.3 s | +0.2% | 465 MB | 465 MB | +0.0% | ~ |
| `Ray` | 170.3 s | 184.7 s | +8.5% | 3281 MB | 3703 MB | +12.9% | **minor** |
| `RayCSG` | 205.3 s | 216.5 s | +5.4% | 3632 MB | 4086 MB | +12.5% | **minor** |
| `RayCrossing` | 388.4 s | 418.5 s | +7.7% | 3577 MB | 4019 MB | +12.4% | **minor** |
| `RayParticles` | 442.7 s | 623.5 s | +40.8% | 5256 MB | 5633 MB | +7.2% | **MAJOR** |
| `RayShell` | 278.4 s | 400.8 s | +43.9% | 4858 MB | 5347 MB | +10.1% | **MAJOR** |
| `Render` | 203.3 s | 210.5 s | +3.6% | 4338 MB | 4939 MB | +13.8% | **minor** |
| `Shapes` | 27.0 s | 26.1 s | -3.4% | 522 MB | 522 MB | -0.0% | ~ |
| `SphereCloudBVH` | 8.0 s | 7.8 s | -3.6% | 466 MB | 466 MB | +0.0% | ~ |
| `SphereCloudRays` | 163.6 s | 178.9 s | +9.3% | 3542 MB | 4098 MB | +15.7% | **minor** |
| `SphereContacts` | 7.6 s | 7.1 s | -5.6% | 465 MB | 465 MB | +0.0% | ~ |
| `Tape` | 60.2 s | 64.9 s | +7.8% | 853 MB | 854 MB | +0.0% | ~ |
| `TapeBVHUnion` | - | 41.8 s | | - | 795 MB | | new |
| **all, in both** | 3007.0 s | 3468.9 s | +15.4% | 5256 MB | 5633 MB | +7.2% | **minor** |

Device kernels: 154 in both, 37 changed (2 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 2 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const, …` | 255 → 40 | 11808 → 12928 | 114/132 → 230/368 | **registers spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float> >, unsigned int const, f…` | 168 → 168 | 13248 → 13248 | 728/720 → 728/740 | **spills** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double, Fused::Node<Fused::Comb…` | 168 → 168 | 9328 → 9376 | 16/4 → 16/4 | ~ |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double, TriMeshSDF<double, 4ul,…` | 168 → 168 | 9328 → 9392 | 20/4 → 20/4 | ~ |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double> >, Vec3T<double> const,…` | 168 → 168 | 9312 → 9360 | 16/4 → 16/4 | ~ |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float, TriMeshSDF<float, 4ul, 4u…` | 168 → 168 | 6672 → 6720 | 4/4 → 4/4 | ~ |
| `Fused` | `void parallelForKernel<128u, 384u, Apply<Distance<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 4480 → 4464 | 20/4 → 20/4 | ~ |
| `Fused` | `void parallelForKernel<128u, 384u, Apply<Distance<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 3008 → 3024 | 4/4 → 4/4 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 17184 → 17200 | 174/220 → 174/220 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 80 → 80 | 17520 → 17552 | 290/464 → 290/464 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 80 → 80 | 12656 → 12672 | 126/132 → 126/132 | ~ |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 128 → 128 | 17360 → 17376 | 186/220 → 186/220 | ~ |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 128 → 128 | 12400 → 12416 | 114/132 → 114/132 | ~ |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> cons…` | 40 → 40 | 17792 → 17808 | 518/916 → 518/916 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11904 → 11888 | 114/132 → 114/132 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19056 → 19056 | 1072/1084 → 1048/1072 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float, TriMeshSDF<float, 4ul, 4…` | 168 → 168 | 12992 → 13040 | 728/740 → 720/736 | ~ |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 17184 → 17200 | 174/220 → 174/220 | ~ |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 17056 → 17072 | 160/152 → 160/152 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<double, Tape<double, TriMeshSDF<dou…` | 168 → 168 | 16720 → 16736 | 48/36 → 48/36 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<double, Tape<double> >, Vec3T<doubl…` | 168 → 168 | 16368 → 16384 | 48/36 → 48/36 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<float, Tape<float, TriMeshSDF<float…` | 168 → 168 | 11456 → 11488 | 40/28 → 40/28 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<float, Tape<float> >, Vec3T<float> …` | 168 → 168 | 11424 → 11456 | 40/28 → 40/28 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 17184 → 17200 | 174/220 → 174/220 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11904 → 11888 | 114/132 → 114/132 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11872 → 11904 | 114/132 → 114/132 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 19392 → 19408 | 246/208 → 246/208 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 18736 → 18720 | 246/208 → 246/208 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13328 → 13344 | 178/160 → 178/160 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13216 → 13200 | 178/160 → 178/160 | ~ |
| `RayShell` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TapeFeature::ExactO…` | 168 → 168 | 19376 → 19392 | 236/224 → 236/224 | ~ |
| `RayShell` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TapeFeature::ExactOff…` | 168 → 168 | 13328 → 13360 | 172/176 → 172/176 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 18672 → 18672 | 1056/1080 → 1052/1076 | ~ |
| `SphereCloudRays` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, SphereCloudBVH<doub…` | 168 → 168 | 17152 → 17200 | 126/104 → 126/104 | ~ |
| `SphereCloudRays` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, SphereCloudBVH<float,…` | 168 → 168 | 12160 → 12176 | 102/88 → 102/88 | ~ |
| `Tape` | `void parallelForKernel<128u, 384u, Apply<Distance<double, Tape<double>, 16ul>, Vec3T<doubl…` | 168 → 168 | 4448 → 4464 | 40/28 → 40/28 | ~ |
| `Tape` | `void parallelForKernel<128u, 384u, Apply<Distance<double, Tape<double>, 1ul>, Vec3T<double…` | 168 → 168 | 4448 → 4432 | 16/4 → 16/4 | ~ |


Flagged after 2 rounds, and judged on 10 (6 cases): LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice, LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice, LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice, RayCrossing/csg-tape/firstHit-morton/float/hostdevice, RayParticles/armadillo-shell-fused/step-morton/double/hostdevice, RayShell/armadillo-tape/firstHit-morton/double/hostdevice

These probes compiled to the same code on both sides and were not run (118 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), Mesh (8), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Shapes (20), SphereCloudBVH (8), SphereContacts (2)

The probes differ between the sides (each side ran its own): Performance/TapeBVHUnion/main.cpp

<!-- END receipt cuda -->
