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
base: 1d94fed
base-fingerprint: 5b1fba5d060b8afb075afe40c1def3e39c0ed207
head: 3dc911a
head-fingerprint: ccacc81e220bf9f8b02e3f92a267fc124862d6b0
cases: 120
rounds: 2
reps: 3
date: 2026-11-08
-->

## Host comparison: green

**Accepted:** -

`3dc911a` against `1d94fed` (origin/main), 2026-11-08.

**VERDICT: no regression (0 major, 0 minor across 120 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 120 cases

- base: 1d94fed (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3dc911a (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.6 ms | 19.6 ms | -0.3% | [-1.3%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.0 ms | 41.1 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.8 ms | 72.5 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.1 ms | 20.1 ms | +0.2% | [-0.9%, +1.4%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.7 ms | 43.6 ms | -0.0% | [-0.8%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.4 ms | 81.0 ms | -0.5% | [-0.5%, -0.4%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.47 ms | 4.48 ms | +0.1% | [-0.5%, +0.8%] | - | ~ | tape/fused 2.87x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | tape/fused 2.02x |
| `Fused/chain16/signedDistance/float/host` | 979 µs | 987 µs | +0.9% | [-1.6%, +3.4%] | - | ~ | tape/fused 16.51x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.4 ms | 46.1 ms | -0.5% | [-0.9%, -0.2%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.2 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.54 ms | 4.64 ms | +2.0% | [+0.2%, +3.9%] | - | ~ | tape/fused 2.82x |
| `Fused/smooth/signedDistance/double/host` | 12.9 ms | 12.8 ms | -0.8% | [-2.0%, +0.3%] | - | ~ | tape/fused 2.06x |
| `Fused/chain16/signedDistance/double/host` | 3.91 ms | 3.94 ms | +0.9% | [-0.9%, +2.6%] | - | ~ | tape/fused 4.14x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.3 ms | 52.1 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.0 ms | 52.8 ms | -0.4% | [-1.3%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.7 ms | 26.5 ms | -0.5% | [-1.4%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.6 ms | 26.7 ms | +0.1% | [-0.5%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 319 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 238 ms | 239 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.1 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ | tape/fused 3.54x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.7 ms | 99.8 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | tape/fused 2.39x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.5 ms | 29.6 ms | +0.5% | [-0.5%, +1.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 29.9 ms | 30.1 ms | +0.7% | [-0.2%, +1.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 355 ms | -0.6% | [-1.9%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 254 ms | 253 ms | -0.1% | [-0.2%, -0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 96.9 ms | 97.0 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ | tape/fused 3.66x |
| `MeshCSG/smooth-fused/frame/double/host` | 107 ms | 107 ms | +0.5% | [-0.0%, +0.9%] | +0.0% | ~ | tape/fused 2.37x |
| `Ray/mesh/firstHit/float/host` | 46.2 ms | 46.4 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.7 ms | 45.5 ms | -0.3% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.3 ms | 40.3 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 79.1 ms | 78.5 ms | -0.8% | [-1.9%, +0.4%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.7 ms | 65.7 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.21 ms | 5.16 ms | -1.0% | [-2.0%, -0.0%] | - | ~ | tape/fused 1.48x |
| `Ray/csg-tape/firstHit/float/host` | 7.58 ms | 7.65 ms | +0.9% | [+0.7%, +1.2%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.3 ms | 52.4 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.7 ms | 51.1 ms | -1.1% | [-1.7%, -0.5%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.6 ms | 44.7 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 87.0 ms | 87.5 ms | +0.5% | [+0.0%, +0.9%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.7 ms | 73.8 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.52 ms | 5.51 ms | -0.1% | [-1.0%, +0.8%] | - | ~ | tape/fused 1.46x |
| `Ray/csg-tape/firstHit/double/host` | 7.90 ms | 8.03 ms | +1.6% | [+1.2%, +1.9%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.3 ms | 17.6 ms | +1.8% | [+0.5%, +3.0%] | - | ~ | marched/traced 6.82x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 120 ms | -0.0% | [-0.2%, +0.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 21.8 ms | -1.6% | [-2.2%, -1.1%] | +0.0% | ~ | marched/traced 6.83x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 150 ms | 149 ms | -0.6% | [-0.9%, -0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.3 ms | 35.2 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 18.9 ms | 19.0 ms | +0.6% | [+0.3%, +1.0%] | - | ~ | marched/traced 6.67x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 126 ms | 127 ms | +0.4% | [+0.1%, +0.8%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 24.5 ms | +4.8% | [-0.6%, +10.3%] | +0.0% | ~ | marched/traced 6.34x (was 6.62x) |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 155 ms | 155 ms | +0.4% | [-0.4%, +1.1%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 38.0 ms | 37.9 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.4 ms | 46.4 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.3 ms | 46.4 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.82 ms | 7.75 ms | -1.0% | [-2.5%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.22 ms | 6.26 ms | +0.7% | [-0.3%, +1.7%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 177 ms | 177 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 157 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.7 ms | 68.8 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.7 ms | 56.5 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.2 ms | 52.7 ms | +1.0% | [+0.3%, +1.6%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.4 ms | 52.6 ms | +0.4% | [+0.1%, +0.8%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.12 ms | 7.96 ms | -2.0% | [-2.6%, -1.5%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.27 ms | 6.22 ms | -0.9% | [-1.1%, -0.7%] | +0.0% | ~ | hit/crossing 1.28x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 201 ms | 203 ms | +1.4% | [+0.9%, +1.9%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 178 ms | 179 ms | +0.6% | [+0.5%, +0.7%] | +0.0% | ~ | hit/crossing 1.14x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.9 ms | 77.5 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.9 ms | 64.6 ms | -0.3% | [-1.2%, +0.6%] | +0.0% | ~ | hit/crossing 1.20x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | marched/traced 4.67x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.6 ms | 51.8 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 7.98 ms | 8.16 ms | +2.2% | [-0.3%, +4.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.93 ms | 1.95 ms | +0.6% | [-0.1%, +1.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.98 ms | 6.86 ms | -1.6% | [-2.2%, -1.0%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.8 ms | +0.1% | [-1.5%, +1.7%] | +0.0% | ~ | marched/traced 5.20x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.2 ms | 61.1 ms | -0.1% | [-1.0%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.77 ms | 8.78 ms | +0.2% | [-2.8%, +3.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.96 ms | 1.94 ms | -1.2% | [-2.9%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.18 ms | 7.20 ms | +0.2% | [-0.5%, +1.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 236 ms | 237 ms | +0.5% | [+0.5%, +0.6%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 328 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 235 ms | 235 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 254 ms | 253 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ | marched/traced 1.46x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 369 ms | 371 ms | +0.6% | [-0.2%, +1.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 253 ms | 251 ms | -0.6% | [-0.7%, -0.5%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 18.7 ms | 19.0 ms | +1.2% | [+0.7%, +1.8%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 19.1 ms | 19.1 ms | +0.4% | [-0.5%, +1.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.5 ms | 18.6 ms | +0.6% | [+0.4%, +0.7%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 21.2 ms | 21.5 ms | +1.2% | [+0.8%, +1.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.3 ms | 21.4 ms | +0.6% | [-0.0%, +1.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.5 ms | 20.5 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.8 ms | 75.1 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 296 ms | 296 ms | -0.2% | [-0.3%, -0.0%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 298 ms | 298 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 86.5 ms | 86.6 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 348 ms | 346 ms | -0.7% | [-1.9%, +0.5%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 346 ms | 346 ms | +0.1% | [-0.9%, +1.1%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.8 ms | -0.7% | [-0.9%, -0.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.56 ms | 4.57 ms | +0.0% | [-0.8%, +0.8%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.0 ms | 24.9 ms | -0.6% | [-1.3%, +0.0%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.1 ms | 39.8 ms | -0.7% | [-1.3%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.76 ms | 1.77 ms | +0.6% | [-0.9%, +2.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 137 µs | 136 µs | -1.3% | [-13.5%, +12.7%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 30.2 ms | 29.9 ms | -1.0% | [-1.4%, -0.5%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.67 ms | 5.66 ms | -0.1% | [-0.1%, -0.1%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.3 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.60 ms | 4.66 ms | +1.3% | [-0.1%, +2.7%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.1 ms | -0.8% | [-1.7%, +0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.57 ms | 4.60 ms | +0.7% | [+0.2%, +1.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.2 ms | 26.4 ms | +0.9% | [+0.1%, +1.8%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 47.5 ms | 41.9 ms | -11.7% | [-21.7%, +1.0%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.76 ms | 1.77 ms | +0.8% | [-0.1%, +1.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 280 µs | 288 µs | +2.8% | [-3.9%, +12.6%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.7 ms | 29.7 ms | -0.2% | [-0.2%, -0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.74 ms | 7.78 ms | +0.6% | [+0.0%, +1.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.3 ms | 16.3 ms | +0.2% | [-1.1%, +1.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.73 ms | 7.80 ms | +0.9% | [-0.1%, +1.9%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.8 ms | 41.8 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 217 ms | 218 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 42.6 ms | 42.5 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 214 ms | 214 ms | -0.4% | [-0.8%, +0.0%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 462.9 s → 462.0 s (-0.2%), the largest peak 1270 MB → 1282 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 8.8 s | 8.7 s | -0.9% | 557 MB | 557 MB | +0.1% | ~ |
| `BVHRefit` | 3.8 s | 3.8 s | -1.0% | 456 MB | 457 MB | +0.3% | ~ |
| `BVHUnion` | 14.7 s | 14.6 s | -0.8% | 815 MB | 816 MB | +0.1% | ~ |
| `BVHUnionMixed` | 7.1 s | 7.1 s | -0.4% | 557 MB | 554 MB | -0.6% | ~ |
| `ClosestPrimitive` | 25.1 s | 25.2 s | +0.4% | 1208 MB | 1218 MB | +0.8% | ~ |
| `Fused` | 20.7 s | 20.9 s | +0.7% | 995 MB | 1013 MB | +1.8% | ~ |
| `LaunchTuning` | 33.2 s | 33.1 s | -0.4% | 1109 MB | 1121 MB | +1.1% | ~ |
| `Mesh` | 14.1 s | 14.0 s | -0.5% | 770 MB | 771 MB | +0.2% | ~ |
| `MeshCSG` | 34.9 s | 34.5 s | -1.3% | 1155 MB | 1168 MB | +1.1% | ~ |
| `MeshReconcile` | 6.1 s | 6.1 s | -0.1% | 535 MB | 537 MB | +0.4% | ~ |
| `PointCloud` | 6.9 s | 6.9 s | +0.1% | 558 MB | 557 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 3.5 s | 3.5 s | +0.5% | 410 MB | 413 MB | +0.8% | ~ |
| `PointCloudHashGridBuild` | 3.0 s | 3.0 s | +0.4% | 402 MB | 402 MB | +0.1% | ~ |
| `Ray` | 32.7 s | 32.1 s | -1.9% | 1149 MB | 1161 MB | +1.1% | ~ |
| `RayCSG` | 29.4 s | 29.2 s | -0.4% | 970 MB | 981 MB | +1.1% | ~ |
| `RayCrossing` | 40.6 s | 40.8 s | +0.6% | 1270 MB | 1282 MB | +0.9% | ~ |
| `RayParticles` | 43.9 s | 43.7 s | -0.6% | 1269 MB | 1282 MB | +1.0% | ~ |
| `RayShell` | 33.3 s | 34.2 s | +2.6% | 1062 MB | 1074 MB | +1.1% | ~ |
| `Render` | 32.2 s | 32.2 s | +0.1% | 1115 MB | 1126 MB | +1.0% | ~ |
| `Shapes` | 9.0 s | 8.9 s | -1.2% | 552 MB | 552 MB | -0.1% | ~ |
| `SphereCloudBVH` | 4.7 s | 4.6 s | -1.3% | 490 MB | 488 MB | -0.2% | ~ |
| `SphereCloudRays` | 24.7 s | 24.6 s | -0.6% | 960 MB | 976 MB | +1.6% | ~ |
| `SphereContacts` | 4.2 s | 4.1 s | -3.0% | 428 MB | 430 MB | +0.5% | ~ |
| `Tape` | 14.2 s | 14.3 s | +0.7% | 831 MB | 829 MB | -0.2% | ~ |
| `TapeBVHUnion` | 12.1 s | 12.1 s | +0.0% | 785 MB | 801 MB | +2.0% | ~ |
| **all, in both** | 462.9 s | 462.0 s | -0.2% | 1270 MB | 1282 MB | +1.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (1 cases): Tape/sphere-handwritten/signedDistance/double/host

These probes compiled to the same code on both sides and were not run (112 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), Mesh (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20), SphereCloudBVH (6), SphereContacts (2)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 1d94fed
base-fingerprint: 5b1fba5d060b8afb075afe40c1def3e39c0ed207
head: 3dc911a
head-fingerprint: ccacc81e220bf9f8b02e3f92a267fc124862d6b0
cases: 212
rounds: 2
reps: 3
date: 2026-11-08
-->

## Device comparison (CUDA): green

**Accepted:** -

`3dc911a` against `1d94fed` (origin/main), 2026-11-08.

**VERDICT: no regression (0 major, 0 minor across 212 cases)**

Performance comparison: 0 major, 0 minor, 4 faster, 0 differ, 0 lost, 212 cases

- base: 1d94fed (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 3dc911a (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 312 µs | 314 µs | +0.6% | [+0.4%, +0.7%] | - | ~ | device 11.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 6.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.8 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 3.3x faster than the host (was 3.7x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.39 ms | 5.38 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 520 ms | 517 ms | -0.6% | [-1.4%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 47.9 µs | -0.0% | [-0.4%, +0.3%] | - | ~ | device 23.8x faster than the host; tape/fused 4.56x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.8 µs | 49.8 µs | -0.0% | [-0.3%, +0.2%] | - | ~ | device 43.9x faster than the host; tape/fused 7.22x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.8 µs | 48.0 µs | +0.4% | [-0.0%, +0.8%] | - | ~ | device 9.5x faster than the host; tape/fused 9.92x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.5 ms | 17.4 ms | -0.8% | [-1.9%, +0.2%] | - | ~ | device 6.7x faster than the host (was 8.6x); tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.2 ms | 31.2 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 280 ms | 284 ms | +1.3% | [+0.3%, +2.4%] | - | ~ | device 0.4x faster than the host; tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 284 ms | 284 ms | -0.1% | [-2.2%, +2.2%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.30 ms | 8.31 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | device 10.7x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.35 ms | 8.35 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.18 ms | 8.33 ms | +1.8% | [+1.2%, +2.5%] | +0.0% | ~ | device 10.6x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.24 ms | 8.24 ms | -0.1% | [-1.8%, +1.7%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 678 µs | 679 µs | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.75 ms | 1.75 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.03 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.66x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 808 µs | 814 µs | +0.7% | [-0.0%, +1.4%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.83x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 740 µs | 742 µs | +0.2% | [-0.3%, +0.8%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 823 µs | 824 µs | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 741 µs | 739 µs | -0.3% | [-1.1%, +0.4%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 729 µs | 723 µs | -0.8% | [-1.7%, +0.2%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 161 ms | +1.0% | [+0.7%, +1.3%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 156 ms | 157 ms | +0.8% | [-1.4%, +3.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 163 ms | 165 ms | +1.2% | [-0.1%, +2.5%] | +0.0% | ~ | device 0.5x faster than the host (was 0.6x); default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 163 ms | 161 ms | -1.2% | [-5.8%, +3.5%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.40 ms | +0.6% | [+0.2%, +0.9%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.76 ms | 2.77 ms | +0.2% | [-0.3%, +0.7%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.94 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.72x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.55 ms | -0.5% | [-0.7%, -0.2%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.24 ms | 7.24 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.32 ms | 7.29 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.23 ms | 7.25 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.29 ms | 7.31 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.51 ms | 2.53 ms | +0.9% | [+0.1%, +1.7%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.29 ms | 4.29 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.7 ms | 13.8 ms | +0.2% | [-0.3%, +0.8%] | - | ~ | device 3.9x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.1 ms | 62.9 ms | -0.2% | [-1.3%, +0.8%] | - | ~ | device 3.2x faster than the host (was 3.6x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 92.3 ms | 92.4 ms | +0.2% | [-2.6%, +3.0%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 252 ms | 247 ms | -2.2% | [-4.1%, -0.2%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.8 ms | 25.0 ms | +0.7% | [-0.7%, +2.2%] | - | ~ | device 2.0x faster than the host; tape/fused 2.52x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.4 ms | 26.8 ms | -2.0% | [-3.3%, -0.7%] | - | ~ | device 2.2x faster than the host; tape/fused 3.45x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.2 ms | 71.1 ms | -0.1% | [-2.0%, +1.9%] | - | ~ | device 4.1x faster than the host (was 4.7x); tape/fused 3.47x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.2 ms | 21.8 ms | -2.0% | [-5.0%, +1.2%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 43.8 ms | 44.4 ms | +1.3% | [+0.5%, +2.1%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 94.2 ms | 94.5 ms | +0.3% | [-2.5%, +3.2%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 544 ms | 549 ms | +0.9% | [-0.5%, +2.4%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 850 ms | 849 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.01 s | 2.06 s | +2.1% | [-0.7%, +5.0%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 204 ms | 198 ms | -2.9% | [-3.6%, -2.1%] | - | ~ | device 0.2x faster than the host; tape/fused 2.77x (was 2.67x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 318 ms | 312 ms | -1.9% | [-3.7%, -0.2%] | - | ~ | device 0.2x faster than the host; tape/fused 2.72x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 600 ms | 584 ms | -2.7% | [-3.6%, -1.8%] | - | ~ | device 0.5x faster than the host (was 0.5x); tape/fused 3.52x (was 3.36x) |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 676 µs | 676 µs | +0.0% | [-1.1%, +1.2%] | - | ~ | device 16.8x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.4% | [-0.9%, +0.2%] | - | ~ | device 14.6x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 640 µs | 644 µs | +0.7% | [-0.8%, +2.2%] | - | ~ | device 17.3x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.23 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 15.1x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 607 µs | 602 µs | -0.8% | [-1.7%, +0.1%] | - | ~ | device 16.1x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | -0.1% | [-0.7%, +0.5%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.48 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.08 ms | 1.08 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 613 µs | 609 µs | -0.7% | [-6.6%, +5.5%] | - | ~ | device 2.5x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 723 µs | 661 µs | -8.6% | [-9.0%, -8.2%] | - | faster | device 2.7x faster than the host (was 2.5x); tape/fused 1.28x (was 1.16x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 675 µs | 676 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.8x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 837 µs | 845 µs | +1.1% | [+0.0%, +2.1%] | - | ~ | device 3.6x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.1 ms | 17.1 ms | -0.2% | [-1.0%, +0.5%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.5% | [-1.3%, +0.4%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.1% | [-0.3%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.3 ms | 27.5 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 14.9 ms | +0.0% | [-0.7%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.4 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.6 ms | +0.4% | [-0.8%, +1.5%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.4 ms | 24.5 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 14.9 ms | 15.2 ms | +1.6% | [+0.3%, +2.9%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 22.9 ms | -0.0% | [-0.9%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.24 ms | 1.25 ms | +0.6% | [+0.1%, +1.0%] | - | ~ | device 1.3x faster than the host; tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.38 ms | 2.37 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.40 ms | +0.6% | [+0.5%, +0.7%] | - | ~ | device 2.0x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.2% | [-0.7%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 3.0x faster than the host; marched/traced 3.55x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.00 ms | 2.99 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 2.7x faster than the host; marched/traced 4.63x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.00 ms | 9.00 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.20 ms | 2.19 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 3.9x faster than the host; marched/traced 13.19x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.48 ms | 5.48 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.8x faster than the host; marched/traced 10.05x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.0 ms | 28.9 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.2 ms | 55.0 ms | -0.3% | [-0.5%, -0.0%] | - | ~ | device 2.0x faster than the host (was 2.1x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.23 ms | 5.22 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.77 ms | 8.75 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.17 ms | 6.22 ms | +0.9% | [+0.7%, +1.2%] | - | ~ | device 1.3x faster than the host; marched/traced 25.43x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 0.8x faster than the host; marched/traced 24.49x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 158 ms | -0.5% | [-3.1%, +2.2%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 257 ms | 260 ms | +0.9% | [-1.1%, +2.9%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.88 ms | 4.86 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 23.07x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.3 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 1.3x faster than the host; marched/traced 17.63x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 113 ms | 112 ms | -0.8% | [-1.8%, +0.1%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 215 ms | 216 ms | +0.6% | [-0.5%, +1.6%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.2 ms | 19.3 ms | +0.3% | [-0.0%, +0.7%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.7 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 674 µs | 675 µs | +0.1% | [-0.6%, +0.8%] | - | ~ | device 17.1x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 15.1x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 676 µs | 677 µs | +0.1% | [-0.4%, +0.7%] | - | ~ | device 17.3x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.29 ms | 1.28 ms | -0.7% | [-1.1%, -0.4%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 728 µs | 728 µs | +0.0% | [-0.5%, +0.5%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 865 µs | 854 µs | -1.3% | [-2.9%, +0.2%] | - | ~ | device 3.6x faster than the host (was 3.9x) |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 627 µs | 628 µs | +0.2% | [+0.0%, +0.5%] | - | ~ | device 3.3x faster than the host; hit/crossing 1.16x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 650 µs | 649 µs | -0.0% | [-0.5%, +0.5%] | - | ~ | device 3.7x faster than the host; hit/crossing 1.31x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.70 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.18 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 10.0x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.09 ms | 9.09 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 6.7x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.9 ms | 55.8 ms | -2.0% | [-5.0%, +1.2%] | - | ~ | device 3.2x faster than the host (was 3.1x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.6 ms | 82.0 ms | +0.5% | [-0.3%, +1.2%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.0 ms | 48.3 ms | +0.4% | [-0.0%, +0.9%] | - | ~ | device 3.5x faster than the host (was 3.0x); hit/crossing 1.16x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.7 ms | 63.4 ms | -0.4% | [-0.9%, +0.2%] | - | ~ | device 2.4x faster than the host; hit/crossing 1.29x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.7% | [-1.6%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.3 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.6% | [-1.2%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.3 ms | 27.4 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.40 ms | 1.40 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.1% | [-0.8%, +0.5%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.10 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.27x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | +0.3% | [-0.5%, +1.0%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.3 ms | 64.5 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 121 ms | 123 ms | +1.0% | [-1.0%, +3.1%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.4 ms | 58.4 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 112 ms | 112 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 479 ms | 477 ms | -0.4% | [-6.5%, +6.4%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 716 ms | 700 ms | -2.2% | [-3.2%, -1.2%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 415 ms | 377 ms | -9.1% | [-15.8%, -2.1%] | - | faster | device 0.4x faster than the host (was 0.4x); hit/crossing 1.26x (was 1.15x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 527 ms | 526 ms | -0.2% | [-0.6%, +0.1%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.33x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.58 ms | 3.55 ms | -0.8% | [-2.4%, +0.7%] | - | ~ | device 1.2x faster than the host; marched/traced 3.40x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.77 ms | 4.74 ms | -0.5% | [-1.1%, +0.2%] | - | ~ | device 0.9x faster than the host; marched/traced 5.61x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.1 ms | +0.2% | [-0.8%, +1.1%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.6 ms | +0.0% | [-0.1%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.24 ms | -0.5% | [-1.6%, +0.5%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.84 ms | -0.8% | [-1.4%, -0.2%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 94.0 µs | 95.5 µs | +1.6% | [-0.1%, +3.3%] | - | ~ | device 14.8x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 149 µs | 152 µs | +1.4% | [+0.3%, +2.5%] | - | ~ | device 10.8x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 794 µs | 794 µs | -0.1% | [-0.5%, +0.4%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 927 µs | 927 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 38.2 ms | 35.6 ms | -6.6% | [-12.4%, -0.7%] | - | faster | device 0.1x faster than the host (was 0.1x); marched/traced 3.41x (was 3.17x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.8 ms | 36.9 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 0.1x faster than the host; marched/traced 6.36x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 121 ms | 122 ms | +0.6% | [-0.6%, +1.8%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 232 ms | 234 ms | +1.1% | [-3.3%, +5.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.5 ms | 16.1 ms | -2.4% | [-5.6%, +0.9%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 21.9 ms | +0.3% | [-0.8%, +1.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.06 ms | 1.07 ms | +0.8% | [-4.1%, +4.0%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.35 ms | 2.33 ms | -0.8% | [-1.3%, -0.3%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 6.00 ms | 5.96 ms | -0.6% | [-1.6%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.71 ms | 6.69 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 22.9 ms | 23.5 ms | +2.5% | [+0.6%, +4.5%] | - | ~ | device 2.1x faster than the host; marched/traced 2.64x (was 2.73x) |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.4 ms | 28.3 ms | -0.1% | [-1.0%, +0.7%] | - | ~ | device 1.9x faster than the host; marched/traced 3.69x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.5 ms | 62.1 ms | -0.7% | [-1.2%, -0.1%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 104 ms | 105 ms | +0.3% | [-0.7%, +1.2%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.7 ms | -0.6% | [-1.4%, +0.2%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.3 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 296 ms | 297 ms | +0.6% | [-1.8%, +3.2%] | - | ~ | device 0.2x faster than the host; marched/traced 1.87x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 360 ms | 347 ms | -3.7% | [-5.4%, -2.1%] | - | faster | device 0.2x faster than the host (was 0.2x); marched/traced 2.82x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 558 ms | 556 ms | -0.4% | [-1.3%, +0.4%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 991 ms | 979 ms | -1.3% | [-3.1%, +0.6%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 189 ms | 190 ms | +0.5% | [-1.0%, +2.0%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 240 ms | 240 ms | +0.3% | [-1.3%, +1.9%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.24 ms | 4.25 ms | +0.3% | [+0.3%, +0.3%] | - | ~ | device 2.8x faster than the host (was 2.7x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.24 ms | 4.26 ms | +0.5% | [+0.5%, +0.6%] | - | ~ | device 3.5x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.89 ms | 1.90 ms | +0.3% | [-0.2%, +0.9%] | - | ~ | device 8.8x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.68 ms | 7.65 ms | -0.5% | [-0.6%, -0.3%] | - | ~ | device 1.6x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.61 ms | 7.64 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 1.6x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.4 ms | 23.4 ms | +0.3% | [-0.5%, +1.2%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.01 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | device 7.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.74 ms | 4.75 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | device 5.2x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.1 ms | 24.1 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | -0.0% | [-0.0%, +0.0%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.3 ms | 30.3 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.3 ms | 43.9 ms | -0.9% | [-1.5%, -0.3%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.6 ms | 45.8 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 123 ms | 123 ms | -0.2% | [-3.4%, +3.1%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 126 ms | 126 ms | -0.1% | [-3.7%, +3.7%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.07x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 128 ms | 127 ms | -0.9% | [-2.2%, +0.4%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 136 ms | 135 ms | -0.3% | [-1.6%, +0.9%] | +0.0% | ~ | device 0.7x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 218 µs | +0.2% | [+0.0%, +0.4%] | - | ~ | device 11.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.7 µs | -0.1% | [-0.3%, +0.0%] | - | ~ | device 23.7x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 360 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 12.6x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 9.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.8 µs | 72.2 µs | -0.8% | [-1.4%, -0.2%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.5 µs | 47.5 µs | +0.0% | [-0.2%, +0.2%] | - | ~ | device 5.0x faster than the host (was 4.7x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 652 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.2% | [+0.1%, +0.2%] | - | ~ | device 12.0x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 475 µs | 476 µs | +0.2% | [-0.0%, +0.5%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.4 µs | +0.5% | [+0.0%, +1.0%] | - | ~ | device 26.5x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 859 µs | 858 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.64 ms | 6.64 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 192 µs | 191 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.0x faster than the host (was 2.1x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.71 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 10.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.51 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 3510.9 s → 3515.9 s (+0.1%), the largest peak 5632 MB → 5633 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.1 s | 12.1 s | -0.4% | 535 MB | 538 MB | +0.6% | ~ |
| `BVHRefit` | 6.0 s | 6.0 s | +0.7% | 465 MB | 469 MB | +0.8% | ~ |
| `BVHUnion` | 19.7 s | 19.9 s | +1.0% | 854 MB | 857 MB | +0.3% | ~ |
| `BVHUnionMixed` | 10.1 s | 10.1 s | +0.2% | 557 MB | 559 MB | +0.4% | ~ |
| `ClosestPrimitive` | 127.3 s | 127.6 s | +0.2% | 1221 MB | 1234 MB | +1.1% | ~ |
| `Fused` | 48.5 s | 48.9 s | +0.9% | 1031 MB | 1045 MB | +1.3% | ~ |
| `LaunchTuning` | 498.4 s | 498.1 s | -0.1% | 3806 MB | 3807 MB | +0.0% | ~ |
| `Mesh` | 18.8 s | 18.8 s | -0.1% | 737 MB | 741 MB | +0.5% | ~ |
| `MeshCSG` | 352.6 s | 358.1 s | +1.6% | 5104 MB | 5102 MB | -0.1% | ~ |
| `MeshReconcile` | 9.0 s | 9.0 s | -0.8% | 465 MB | 469 MB | +0.7% | ~ |
| `PointCloud` | 11.7 s | 11.6 s | -1.6% | 503 MB | 506 MB | +0.7% | ~ |
| `PointCloudBVHBuild` | 6.2 s | 6.3 s | +1.1% | 466 MB | 469 MB | +0.7% | ~ |
| `PointCloudHashGridBuild` | 5.4 s | 5.3 s | -2.1% | 466 MB | 469 MB | +0.7% | ~ |
| `Ray` | 183.2 s | 186.1 s | +1.6% | 3703 MB | 3704 MB | +0.0% | ~ |
| `RayCSG` | 219.5 s | 215.4 s | -1.8% | 4086 MB | 4085 MB | -0.0% | ~ |
| `RayCrossing` | 419.0 s | 418.7 s | -0.1% | 4019 MB | 4019 MB | +0.0% | ~ |
| `RayParticles` | 623.5 s | 625.3 s | +0.3% | 5632 MB | 5633 MB | +0.0% | ~ |
| `RayShell` | 402.0 s | 401.3 s | -0.2% | 5346 MB | 5346 MB | +0.0% | ~ |
| `Render` | 210.3 s | 210.3 s | -0.0% | 4942 MB | 4938 MB | -0.1% | ~ |
| `Shapes` | 25.8 s | 25.6 s | -0.8% | 523 MB | 526 MB | +0.6% | ~ |
| `SphereCloudBVH` | 7.9 s | 7.8 s | -1.0% | 466 MB | 469 MB | +0.7% | ~ |
| `SphereCloudRays` | 178.9 s | 179.2 s | +0.1% | 4093 MB | 4096 MB | +0.1% | ~ |
| `SphereContacts` | 7.2 s | 7.2 s | -0.8% | 465 MB | 469 MB | +0.7% | ~ |
| `Tape` | 65.5 s | 65.0 s | -0.8% | 854 MB | 868 MB | +1.6% | ~ |
| `TapeBVHUnion` | 42.0 s | 42.2 s | +0.5% | 795 MB | 809 MB | +1.7% | ~ |
| **all, in both** | 3510.9 s | 3515.9 s | +0.1% | 5632 MB | 5633 MB | +0.0% | ~ |

Device kernels: 156 in both, 4 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19072 → 19088 | 1048/1072 → 1048/1072 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float, TriMeshSDF<float, 4ul, 4…` | 168 → 168 | 13056 → 13072 | 720/736 → 720/736 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 18688 → 18704 | 1052/1076 → 1052/1076 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float> >, unsigned int const, f…` | 168 → 168 | 13264 → 13280 | 728/740 → 728/740 | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (3 cases): RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice, RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice, RayParticles/armadillo/step-morton/double/hostdevice

These probes compiled to the same code on both sides and were not run (118 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), Mesh (8), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Shapes (20), SphereCloudBVH (8), SphereContacts (2)

<!-- END receipt cuda -->
