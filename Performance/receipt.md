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
base: 529281e
base-fingerprint: 057dd7f4da87e97c9c13841bccb1664f96e58730
head: e458fa8
head-fingerprint: 5b1fba5d060b8afb075afe40c1def3e39c0ed207
cases: 120
rounds: 2
reps: 3
date: 2026-11-08
-->

## Host comparison: yellow

**Accepted:** Accept it as a side effect of algorithmic change

`e458fa8` against `529281e` (origin/main), 2026-11-08.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 1 faster, 0 differ, 0 lost, 120 cases

- base: 529281e (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: e458fa8 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.4 ms | 19.8 ms | +2.3% | [+1.6%, +3.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.2 ms | 41.1 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 76.1 ms | 73.2 ms | -3.9% | [-7.7%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.1 ms | 20.2 ms | +0.7% | [-0.3%, +1.7%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.4 ms | 43.6 ms | +0.6% | [+0.4%, +0.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.3 ms | 81.4 ms | -1.0% | [-2.9%, +0.9%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/float/host` | 4.48 ms | 4.47 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | tape/fused 2.88x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | tape/fused 2.02x |
| `Fused/chain16/signedDistance/float/host` | 968 µs | 970 µs | +0.2% | [-0.1%, +0.6%] | - | ~ | tape/fused 16.77x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.3 ms | 46.1 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.3 ms | 46.3 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.62 ms | 4.56 ms | -1.3% | [-2.5%, -0.2%] | - | ~ | tape/fused 2.89x |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | tape/fused 2.03x |
| `Fused/chain16/signedDistance/double/host` | 3.91 ms | 3.96 ms | +1.1% | [-1.4%, +3.8%] | - | ~ | tape/fused 4.14x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.9 ms | 52.3 ms | -1.0% | [-1.5%, -0.5%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.3 ms | 52.6 ms | -1.3% | [-2.0%, -0.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 28.2 ms | 26.5 ms | -5.9% | [-11.1%, -0.0%] | +0.0% | faster |  |
| `MeshCSG/union-tape/frame/float/host` | 27.4 ms | 26.7 ms | -2.5% | [-4.7%, -0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 318 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.5 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 100 ms | 100 ms | -0.0% | [-0.8%, +0.8%] | +0.0% | ~ | tape/fused 2.38x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.5 ms | 29.3 ms | -0.7% | [-1.2%, -0.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.6 ms | 30.3 ms | -1.0% | [-1.9%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 357 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 254 ms | 255 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.0 ms | 97.4 ms | +0.4% | [-0.3%, +1.0%] | +0.0% | ~ | tape/fused 3.67x |
| `MeshCSG/smooth-fused/frame/double/host` | 107 ms | 107 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ | tape/fused 2.39x |
| `Ray/mesh/firstHit/float/host` | 46.4 ms | 46.3 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.6 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.3 ms | 40.4 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.5 ms | 78.7 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.6 ms | 65.7 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.25 ms | 5.21 ms | -0.7% | [-0.9%, -0.6%] | - | ~ | tape/fused 1.46x |
| `Ray/csg-tape/firstHit/float/host` | 7.68 ms | 7.62 ms | -0.8% | [-1.7%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.4 ms | 52.6 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.5 ms | 51.0 ms | -1.1% | [-2.4%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.0 ms | 44.8 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.1 ms | 87.5 ms | -0.6% | [-1.3%, +0.1%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 74.2 ms | 73.4 ms | -1.1% | [-1.7%, -0.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.49 ms | 5.53 ms | +0.8% | [+0.2%, +2.0%] | - | ~ | tape/fused 1.43x |
| `Ray/csg-tape/firstHit/double/host` | 7.81 ms | 7.90 ms | +1.2% | [+0.8%, +1.6%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.4 ms | 17.3 ms | -0.6% | [-1.8%, +0.5%] | - | ~ | marched/traced 6.96x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 120 ms | +0.5% | [+0.4%, +0.5%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 21.9 ms | -0.6% | [-2.1%, +0.8%] | +0.0% | ~ | marched/traced 6.84x (was 6.61x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 146 ms | 150 ms | +2.7% | [+2.7%, +2.8%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.3 ms | 35.5 ms | +0.5% | [-0.4%, +1.4%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 18.8 ms | 18.6 ms | -1.1% | [-1.9%, -0.3%] | - | ~ | marched/traced 6.80x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 127 ms | 127 ms | -0.3% | [-0.7%, +0.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.5 ms | 23.4 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ | marched/traced 6.61x (was 6.39x) |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 150 ms | 155 ms | +3.1% | [+2.7%, +3.6%] | +0.0% | minor |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.9 ms | 37.9 ms | -0.0% | [-1.0%, +1.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.4 ms | 46.5 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.3 ms | 46.5 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.67 ms | 7.77 ms | +1.4% | [-0.0%, +2.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.21 ms | 6.26 ms | +0.9% | [-0.3%, +2.2%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 176 ms | 177 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 157 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.9 ms | 68.8 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.3 ms | 56.4 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.3 ms | 52.4 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.2 ms | 52.4 ms | +0.4% | [+0.1%, +0.6%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.91 ms | 7.95 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.27 ms | 6.24 ms | -0.5% | [-1.2%, +0.2%] | +0.0% | ~ | hit/crossing 1.28x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 200 ms | 200 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 177 ms | 178 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 78.0 ms | 77.9 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.5 ms | 64.9 ms | +0.6% | [+0.0%, +1.1%] | +0.0% | ~ | hit/crossing 1.20x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.1 ms | +0.6% | [-0.3%, +1.6%] | +0.0% | ~ | marched/traced 4.66x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.7 ms | 51.7 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.00 ms | 8.08 ms | +1.0% | [+0.4%, +1.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.93 ms | 1.94 ms | +0.1% | [-1.4%, +1.7%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.02 ms | 6.97 ms | -0.7% | [-1.3%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.6 ms | 11.7 ms | +0.4% | [+0.0%, +0.7%] | +0.0% | ~ | marched/traced 5.23x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.9 ms | 61.1 ms | -1.3% | [-2.3%, -0.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.63 ms | 8.61 ms | -0.2% | [-1.0%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.98 ms | 1.96 ms | -1.1% | [-2.1%, -0.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.17 ms | 7.16 ms | -0.1% | [-0.1%, -0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 236 ms | -1.1% | [-1.2%, -1.0%] | +0.0% | ~ | marched/traced 1.39x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 327 ms | 328 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 235 ms | -0.9% | [-1.3%, -0.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 253 ms | 254 ms | +0.5% | [+0.3%, +0.8%] | +0.0% | ~ | marched/traced 1.46x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 369 ms | 370 ms | +0.3% | [-0.0%, +0.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 251 ms | 252 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 18.9 ms | 19.0 ms | +0.7% | [+0.1%, +1.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 19.0 ms | 18.8 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 18.5 ms | -0.6% | [-0.9%, -0.4%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 21.4 ms | 21.2 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.5 ms | 21.1 ms | -1.9% | [-2.5%, -1.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.5 ms | 20.5 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 75.2 ms | 74.9 ms | -0.5% | [-1.6%, +0.6%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 296 ms | 297 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 296 ms | 298 ms | +0.8% | [+0.5%, +1.2%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 87.0 ms | 87.0 ms | -0.0% | [-0.8%, +0.8%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 348 ms | 346 ms | -0.7% | [-0.8%, -0.7%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 346 ms | 346 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.58 ms | 4.57 ms | -0.3% | [-0.9%, +0.3%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.1 ms | 24.9 ms | -0.8% | [-1.4%, -0.2%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 39.8 ms | 39.9 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.77 ms | 1.76 ms | -0.6% | [-2.3%, +1.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 143 µs | 141 µs | -1.6% | [-5.2%, +2.1%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.7 ms | 30.0 ms | +1.1% | [+0.4%, +1.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.67 ms | +0.2% | [-0.3%, +0.6%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.3 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.65 ms | 4.65 ms | +0.2% | [-0.3%, +0.7%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.1 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.58 ms | 4.63 ms | +1.2% | [+0.2%, +2.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.2 ms | 26.0 ms | -0.8% | [-1.0%, -0.5%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 41.9 ms | 41.8 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.79 ms | 1.75 ms | -2.1% | [-3.4%, -0.8%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 303 µs | 289 µs | -4.7% | [-13.3%, +4.3%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.7 ms | 30.1 ms | +1.5% | [+1.0%, +2.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.79 ms | 7.75 ms | -0.4% | [-1.4%, +0.5%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.2 ms | 16.4 ms | +0.8% | [+0.4%, +1.3%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.78 ms | 7.80 ms | +0.2% | [-1.2%, +1.6%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 42.0 ms | 41.9 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 216 ms | 217 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 42.6 ms | 42.5 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 214 ms | 216 ms | +1.0% | [+0.8%, +1.3%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 455.8 s → 464.7 s (+2.0%), the largest peak 1270 MB → 1270 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 8.8 s | 9.0 s | +2.2% | 557 MB | 557 MB | +0.0% | ~ |
| `BVHRefit` | 3.8 s | 3.8 s | -0.6% | 456 MB | 456 MB | +0.0% | ~ |
| `BVHUnion` | 14.6 s | 14.9 s | +2.1% | 815 MB | 815 MB | -0.0% | ~ |
| `BVHUnionMixed` | 7.1 s | 7.1 s | +0.7% | 557 MB | 557 MB | +0.1% | ~ |
| `ClosestPrimitive` | 24.9 s | 25.4 s | +2.0% | 1208 MB | 1209 MB | +0.0% | ~ |
| `Fused` | 20.5 s | 20.9 s | +2.0% | 995 MB | 996 MB | +0.1% | ~ |
| `LaunchTuning` | 32.5 s | 34.1 s | +4.9% | 1109 MB | 1108 MB | -0.1% | ~ |
| `Mesh` | 14.2 s | 14.4 s | +2.0% | 770 MB | 770 MB | -0.0% | ~ |
| `MeshCSG` | 33.8 s | 34.6 s | +2.4% | 1155 MB | 1155 MB | +0.0% | ~ |
| `MeshReconcile` | 6.1 s | 6.3 s | +3.3% | 535 MB | 535 MB | +0.0% | ~ |
| `PointCloud` | 6.9 s | 7.2 s | +4.1% | 558 MB | 558 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.5 s | 3.6 s | +3.4% | 410 MB | 410 MB | -0.0% | ~ |
| `PointCloudHashGridBuild` | 3.0 s | 3.1 s | +3.8% | 402 MB | 402 MB | -0.1% | ~ |
| `Ray` | 31.9 s | 32.3 s | +1.2% | 1150 MB | 1149 MB | -0.0% | ~ |
| `RayCSG` | 28.8 s | 29.9 s | +3.9% | 954 MB | 969 MB | +1.6% | ~ |
| `RayCrossing` | 40.1 s | 40.9 s | +2.0% | 1270 MB | 1270 MB | +0.0% | ~ |
| `RayParticles` | 43.1 s | 44.0 s | +1.9% | 1269 MB | 1269 MB | -0.0% | ~ |
| `RayShell` | 32.9 s | 33.4 s | +1.4% | 1062 MB | 1062 MB | -0.0% | ~ |
| `Render` | 31.7 s | 31.7 s | -0.1% | 1115 MB | 1115 MB | +0.0% | ~ |
| `Shapes` | 8.9 s | 9.0 s | +1.8% | 552 MB | 552 MB | +0.0% | ~ |
| `SphereCloudBVH` | 4.6 s | 4.6 s | +0.3% | 489 MB | 489 MB | +0.0% | ~ |
| `SphereCloudRays` | 24.3 s | 24.6 s | +1.2% | 960 MB | 961 MB | +0.0% | ~ |
| `SphereContacts` | 4.0 s | 4.1 s | +1.9% | 428 MB | 428 MB | +0.0% | ~ |
| `Tape` | 14.2 s | 14.1 s | -0.7% | 829 MB | 828 MB | -0.2% | ~ |
| `TapeBVHUnion` | 11.8 s | 11.9 s | +0.8% | 785 MB | 784 MB | -0.0% | ~ |
| **all, in both** | 455.8 s | 464.7 s | +2.0% | 1270 MB | 1270 MB | +0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): Ray/csg-fused/firstHit/double/host, RayCSG/scene-tape/firstHit-marched/double/host

These probes compiled to the same code on both sides and were not run (112 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), Mesh (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20), SphereCloudBVH (6), SphereContacts (2)

The probes differ between the sides (each side ran its own): Performance/ClosestPrimitive/main.cpp, Performance/RayCSG/main.cpp, Performance/Tape/main.cpp, Performance/TapeBVHUnion/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 529281e
base-fingerprint: 057dd7f4da87e97c9c13841bccb1664f96e58730
head: e458fa8
head-fingerprint: 5b1fba5d060b8afb075afe40c1def3e39c0ed207
cases: 212
rounds: 2
reps: 3
date: 2026-11-08
-->

## Device comparison (CUDA): green

**Accepted:** -

`e458fa8` against `529281e` (origin/main), 2026-11-08.

**VERDICT: no regression (0 major, 0 minor across 212 cases)**

Performance comparison: 0 major, 0 minor, 2 faster, 0 differ, 0 lost, 212 cases

- base: 529281e (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: e458fa8 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 314 µs | 313 µs | -0.3% | [-0.7%, +0.1%] | - | ~ | device 11.8x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 6.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.40 ms | 5.39 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 520 ms | 521 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 0.4x faster than the host |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.2% | [-0.2%, +0.6%] | - | ~ | device 23.5x faster than the host (was 24.5x); tape/fused 4.53x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.7 µs | 49.9 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 43.2x faster than the host; tape/fused 7.23x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.9 µs | 48.0 µs | +0.4% | [-0.0%, +0.7%] | - | ~ | device 9.7x faster than the host; tape/fused 9.92x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.2 ms | 17.3 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 6.7x faster than the host; tape/fused 1.81x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.2 ms | 31.2 ms | -0.1% | [-0.1%, +0.0%] | - | ~ | device 3.8x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 282 ms | 281 ms | -0.3% | [-2.8%, +2.3%] | - | ~ | device 0.4x faster than the host; tape/fused 1.02x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 287 ms | 285 ms | -0.6% | [-1.7%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.21 ms | 8.30 ms | +1.1% | [+0.8%, +1.4%] | +0.0% | ~ | device 10.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.26 ms | 8.28 ms | +0.3% | [-0.5%, +1.0%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.24 ms | 8.35 ms | +1.4% | [-0.2%, +3.0%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.27 ms | 8.29 ms | +0.3% | [-0.5%, +1.2%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 678 µs | 676 µs | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ | device 3.8x faster than the host (was 3.7x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.75 ms | 1.76 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 1.5x faster than the host (was 1.4x); default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.03 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ | device 2.6x faster than the host (was 2.4x); default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 813 µs | 808 µs | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ | device 3.3x faster than the host (was 3.1x); default/this 0.84x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 744 µs | 740 µs | -0.6% | [-0.7%, -0.4%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 826 µs | 828 µs | +0.3% | [-0.4%, +0.9%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 738 µs | 739 µs | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 729 µs | 727 µs | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | device 12.1x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 162 ms | 159 ms | -1.5% | [-2.6%, -0.5%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 158 ms | 158 ms | -0.0% | [-2.2%, +2.1%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 162 ms | 163 ms | +0.4% | [-1.9%, +2.7%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 161 ms | 158 ms | -1.6% | [-4.7%, +1.7%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 1.9x faster than the host (was 2.0x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.77 ms | 2.78 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ | device 0.9x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.95 ms | 1.93 ms | -0.7% | [-1.2%, -0.2%] | +0.0% | ~ | device 1.4x faster than the host (was 1.4x); default/this 0.72x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.56 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.25 ms | 7.23 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.32 ms | 7.33 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.22 ms | 7.23 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.29 ms | 7.31 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.54 ms | 2.52 ms | -0.7% | [-1.1%, -0.2%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.29 ms | 4.29 ms | +0.1% | [-1.2%, +1.4%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.7 ms | 13.7 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.9x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 62.8 ms | 63.3 ms | +0.7% | [-0.1%, +1.6%] | - | ~ | device 3.6x faster than the host (was 3.2x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 93.1 ms | 92.5 ms | -0.6% | [-2.1%, +0.8%] | - | ~ | device 2.6x faster than the host (was 2.3x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 250 ms | 254 ms | +1.6% | [+0.3%, +2.9%] | - | ~ | device 3.4x faster than the host (was 3.6x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.9 ms | 24.5 ms | -1.5% | [-2.2%, -0.7%] | - | ~ | device 2.0x faster than the host; tape/fused 2.58x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 26.8 ms | -0.7% | [-1.4%, -0.0%] | - | ~ | device 2.2x faster than the host; tape/fused 3.46x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.2 ms | 71.3 ms | +0.1% | [-0.8%, +0.9%] | - | ~ | device 4.1x faster than the host; tape/fused 3.56x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.5 ms | 23.0 ms | +2.0% | [-4.1%, +8.3%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.5 ms | 44.8 ms | +0.9% | [-3.2%, +5.1%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 92.3 ms | 94.4 ms | +2.3% | [-0.8%, +5.5%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 529 ms | 551 ms | +4.1% | [-2.2%, +10.8%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 852 ms | 849 ms | -0.3% | [-1.5%, +0.9%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.03 s | 2.04 s | +0.1% | [-1.4%, +1.7%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 197 ms | 199 ms | +1.0% | [-3.1%, +5.2%] | - | ~ | device 0.2x faster than the host; tape/fused 2.77x (was 2.69x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 299 ms | 308 ms | +3.3% | [-2.3%, +9.0%] | - | ~ | device 0.2x faster than the host; tape/fused 2.75x (was 2.85x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 590 ms | 605 ms | +2.5% | [-1.2%, +5.2%] | - | ~ | device 0.5x faster than the host; tape/fused 3.37x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 672 µs | 672 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 16.8x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 14.6x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 643 µs | 647 µs | +0.7% | [-0.9%, +2.3%] | - | ~ | device 17.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.22 ms | -0.7% | [-1.1%, -0.3%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 605 µs | 610 µs | +0.8% | [+0.4%, +1.2%] | - | ~ | device 16.0x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.11 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 15.1x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.48 ms | 2.49 ms | +0.3% | [-0.0%, +0.7%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.51x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.08 ms | 1.07 ms | -0.1% | [-0.9%, +0.6%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 593 µs | 612 µs | +3.3% | [-0.0%, +6.6%] | - | ~ | device 2.5x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 690 µs | 657 µs | -4.8% | [-9.3%, +0.2%] | - | ~ | device 2.8x faster than the host (was 2.6x); tape/fused 1.29x (was 1.23x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 675 µs | 677 µs | +0.3% | [-0.1%, +0.6%] | - | ~ | device 3.7x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 847 µs | 851 µs | +0.4% | [-0.6%, +1.4%] | - | ~ | device 3.5x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.1 ms | +1.3% | [+0.7%, +1.9%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.6 ms | 27.5 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.9 ms | 17.1 ms | +0.8% | [+0.4%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.3 ms | 27.5 ms | +0.6% | [+0.6%, +0.7%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.7% | [-0.3%, +1.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.4% | [-0.2%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.4 ms | -0.5% | [-2.0%, +1.1%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.04x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 14.9 ms | 15.1 ms | +1.2% | [+0.5%, +1.9%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 22.9 ms | 22.9 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.38 ms | 2.38 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.48 ms | 2.50 ms | +0.4% | [+0.0%, +0.8%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.9x faster than the host; marched/traced 3.55x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.00 ms | 2.99 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 2.7x faster than the host; marched/traced 4.63x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.00 ms | 9.02 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.9 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.20 ms | 2.20 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 3.8x faster than the host; marched/traced 13.22x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.48 ms | 5.49 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.8x faster than the host; marched/traced 10.06x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.1 ms | 29.1 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.1 ms | 55.2 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 2.1x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.24 ms | 5.22 ms | -0.5% | [-0.7%, -0.3%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.77 ms | 8.75 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.18 ms | 6.16 ms | -0.3% | [-1.2%, +0.5%] | - | ~ | device 1.3x faster than the host (was 1.2x); marched/traced 25.95x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.0% | [-0.6%, +0.6%] | - | ~ | device 0.8x faster than the host (was 0.7x); marched/traced 24.49x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 160 ms | +0.3% | [-0.9%, +1.5%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 252 ms | 259 ms | +2.5% | [+0.2%, +5.0%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.87 ms | 4.88 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.8x faster than the host; marched/traced 21.93x (was 23.04x) |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.3 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.3x faster than the host; marched/traced 17.63x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 112 ms | 107 ms | -4.6% | [-6.3%, -2.8%] | - | faster | device 0.6x faster than the host (was 0.6x) |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 217 ms | 217 ms | +0.1% | [-1.3%, +2.3%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.2 ms | -0.8% | [-1.1%, -0.5%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.7 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 671 µs | 673 µs | +0.3% | [-0.5%, +1.1%] | - | ~ | device 17.1x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.2% | [-0.5%, +0.0%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 674 µs | 676 µs | +0.3% | [-0.8%, +1.5%] | - | ~ | device 17.1x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.4% | [-0.9%, +0.2%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 729 µs | 727 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 849 µs | 847 µs | -0.3% | [-1.1%, +0.5%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 624 µs | 624 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 3.2x faster than the host; hit/crossing 1.16x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 648 µs | 650 µs | +0.3% | [-0.0%, +0.6%] | - | ~ | device 3.6x faster than the host; hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.69 ms | 4.69 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 6.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.18 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 9.9x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.08 ms | 9.08 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 6.7x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.0 ms | 56.3 ms | +0.6% | [-0.7%, +1.9%] | - | ~ | device 3.4x faster than the host (was 3.1x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.5 ms | 81.8 ms | +0.4% | [-1.0%, +1.9%] | - | ~ | device 2.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 48.6 ms | 48.0 ms | -1.2% | [-1.9%, -0.6%] | - | ~ | device 3.0x faster than the host; hit/crossing 1.17x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.3 ms | 63.4 ms | +0.2% | [-0.1%, +0.5%] | - | ~ | device 2.5x faster than the host (was 2.7x); hit/crossing 1.29x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.1 ms | 16.9 ms | -0.7% | [-1.7%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.5 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.40 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.1% | [-0.5%, +0.2%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.09 ms | 1.10 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.27x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.15 ms | 1.16 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.4 ms | 64.6 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 123 ms | 121 ms | -1.4% | [-2.7%, -0.1%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.4 ms | 58.5 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 111 ms | 113 ms | +1.9% | [+0.3%, +3.6%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.07x (was 1.11x) |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 464 ms | 458 ms | -1.3% | [-6.7%, +4.7%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 713 ms | 707 ms | -0.9% | [-2.3%, +0.5%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 394 ms | 390 ms | -1.2% | [-8.2%, +6.6%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.18x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 532 ms | 522 ms | -1.9% | [-4.0%, +0.3%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.35x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.65 ms | 3.59 ms | -1.5% | [-5.3%, +2.4%] | - | ~ | device 1.2x faster than the host; marched/traced 3.35x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.79 ms | 4.78 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 0.9x faster than the host; marched/traced 5.57x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | +0.1% | [-0.6%, +0.7%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.6 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.25 ms | 1.23 ms | -1.3% | [-3.5%, +0.9%] | - | ~ | device 2.5x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.84 ms | 1.84 ms | -0.1% | [-0.6%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 93.5 µs | 95.6 µs | +2.2% | [+0.0%, +4.4%] | - | ~ | device 15.5x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 149 µs | 150 µs | +0.2% | [-0.8%, +1.2%] | - | ~ | device 11.2x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 797 µs | 794 µs | -0.3% | [-0.8%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 933 µs | 928 µs | -0.6% | [-0.9%, -0.3%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.4 ms | 33.7 ms | -4.8% | [-7.4%, -2.2%] | - | faster | device 0.1x faster than the host (was 0.1x); marched/traced 3.57x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 37.2 ms | 36.7 ms | -1.5% | [-3.3%, +0.3%] | - | ~ | device 0.1x faster than the host; marched/traced 6.30x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 124 ms | 120 ms | -2.7% | [-4.8%, -0.7%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 231 ms | 231 ms | +0.2% | [-0.6%, +1.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.0 ms | 16.5 ms | +3.1% | [-4.1%, +10.6%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 21.9 ms | +0.8% | [-0.3%, +1.8%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.11 ms | 1.12 ms | +1.7% | [-6.4%, +10.4%] | - | ~ | device 1.3x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.36 ms | 2.33 ms | -1.4% | [-2.8%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.97 ms | 5.98 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.69 ms | 6.71 ms | +0.2% | [-0.6%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.0 ms | 23.4 ms | +2.1% | [+0.2%, +3.9%] | - | ~ | device 2.1x faster than the host; marched/traced 2.66x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.6 ms | +1.0% | [+0.5%, +1.6%] | - | ~ | device 1.9x faster than the host; marched/traced 3.67x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.2 ms | 62.4 ms | +0.3% | [-1.0%, +1.5%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 104 ms | 105 ms | +1.2% | [+0.3%, +2.2%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.6 ms | 13.8 ms | +1.4% | [+1.0%, +1.9%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.5 ms | +0.3% | [-0.7%, +1.3%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 305 ms | 296 ms | -2.7% | [-5.2%, -0.2%] | - | ~ | device 0.2x faster than the host; marched/traced 1.90x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 349 ms | 356 ms | +2.0% | [-0.6%, +4.0%] | - | ~ | device 0.2x faster than the host; marched/traced 2.77x (was 2.87x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 569 ms | 564 ms | -0.9% | [-1.6%, -0.1%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1000 ms | 986 ms | -1.4% | [-3.3%, +0.5%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 188 ms | 184 ms | -2.2% | [-3.3%, -1.1%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 241 ms | 234 ms | -2.9% | [-5.7%, +0.1%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.21 ms | 4.24 ms | +0.6% | [+0.5%, +0.8%] | - | ~ | device 2.7x faster than the host (was 2.7x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.22 ms | 4.24 ms | +0.5% | [+0.5%, +0.5%] | - | ~ | device 2.6x faster than the host (was 2.7x) |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.89 ms | +0.7% | [-0.1%, +1.5%] | - | ~ | device 8.8x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.62 ms | 7.65 ms | +0.4% | [+0.2%, +0.5%] | - | ~ | device 1.6x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.61 ms | 7.61 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 1.6x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.3 ms | 23.3 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.01 ms | 2.99 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ | device 7.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.76 ms | 4.77 ms | +0.3% | [-0.2%, +0.7%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 5.1x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.1 ms | 24.1 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host (was 5.4x); tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | -0.1% | [-0.1%, -0.0%] | +0.0% | ~ | device 4.2x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.3 ms | 30.3 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ | device 3.2x faster than the host (was 3.0x) |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.2 ms | 44.2 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.7 ms | 45.9 ms | +0.5% | [-1.6%, +2.5%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 122 ms | 124 ms | +1.5% | [-1.4%, +3.2%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 125 ms | 128 ms | +2.3% | [+0.7%, +4.0%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 128 ms | 128 ms | -0.0% | [-2.8%, +2.7%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 131 ms | 134 ms | +2.3% | [-0.2%, +4.7%] | +0.0% | ~ | device 0.7x faster than the host (was 0.8x) |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 218 µs | -0.3% | [-0.7%, +0.1%] | - | ~ | device 11.3x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.7 µs | 47.8 µs | +0.2% | [+0.0%, +0.5%] | - | ~ | device 23.5x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 358 µs | 360 µs | +0.7% | [+0.6%, +0.9%] | - | ~ | device 12.7x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 9.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.1 µs | 72.4 µs | +0.5% | [+0.1%, +0.9%] | - | ~ | device 7.0x faster than the host (was 7.6x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.4 µs | 47.5 µs | +0.2% | [-0.0%, +0.5%] | - | ~ | device 4.8x faster than the host (was 5.8x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 651 µs | -0.1% | [-0.1%, -0.1%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 11.6x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 478 µs | 476 µs | -0.4% | [-0.7%, -0.1%] | - | ~ | device 7.6x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.1 µs | 48.2 µs | +0.1% | [-0.3%, +0.5%] | - | ~ | device 28.3x faster than the host (was 27.2x) |
| `Tape/csg/signedDistance/double/hostdevice` | 857 µs | 859 µs | +0.2% | [+0.1%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.64 ms | 6.65 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 192 µs | 191 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 2.8x faster than the host (was 2.9x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | +0.2% | [+0.1%, +0.3%] | - | ~ | device 1.8x faster than the host (was 1.9x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.71 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 10.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.50 ms | 6.51 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.6x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 3501.5 s → 3516.6 s (+0.4%), the largest peak 5633 MB → 5632 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.0 s | 12.1 s | +0.6% | 535 MB | 535 MB | +0.1% | ~ |
| `BVHRefit` | 6.0 s | 6.0 s | -0.2% | 465 MB | 465 MB | -0.0% | ~ |
| `BVHUnion` | 19.5 s | 19.7 s | +1.2% | 854 MB | 854 MB | +0.0% | ~ |
| `BVHUnionMixed` | 10.1 s | 10.1 s | -0.7% | 558 MB | 558 MB | +0.0% | ~ |
| `ClosestPrimitive` | 127.2 s | 127.8 s | +0.5% | 1220 MB | 1221 MB | +0.0% | ~ |
| `Fused` | 48.1 s | 48.2 s | +0.1% | 1031 MB | 1032 MB | +0.0% | ~ |
| `LaunchTuning` | 498.1 s | 499.3 s | +0.2% | 3806 MB | 3806 MB | +0.0% | ~ |
| `Mesh` | 18.7 s | 18.7 s | -0.2% | 737 MB | 738 MB | +0.1% | ~ |
| `MeshCSG` | 352.4 s | 354.8 s | +0.7% | 5100 MB | 5104 MB | +0.1% | ~ |
| `MeshReconcile` | 8.7 s | 8.9 s | +1.7% | 465 MB | 465 MB | +0.0% | ~ |
| `PointCloud` | 11.1 s | 11.4 s | +2.2% | 503 MB | 503 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 6.2 s | 6.1 s | -1.3% | 465 MB | 466 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 5.4 s | 5.3 s | -0.7% | 465 MB | 466 MB | +0.0% | ~ |
| `Ray` | 184.0 s | 186.3 s | +1.3% | 3703 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 214.4 s | 216.0 s | +0.8% | 4086 MB | 4086 MB | -0.0% | ~ |
| `RayCrossing` | 420.5 s | 420.2 s | -0.1% | 4019 MB | 4019 MB | -0.0% | ~ |
| `RayParticles` | 624.2 s | 624.6 s | +0.1% | 5633 MB | 5632 MB | -0.0% | ~ |
| `RayShell` | 400.5 s | 403.8 s | +0.8% | 5347 MB | 5346 MB | -0.0% | ~ |
| `Render` | 209.3 s | 211.1 s | +0.9% | 4938 MB | 4942 MB | +0.1% | ~ |
| `Shapes` | 26.0 s | 25.7 s | -1.4% | 523 MB | 523 MB | +0.0% | ~ |
| `SphereCloudBVH` | 8.0 s | 7.8 s | -2.4% | 466 MB | 466 MB | +0.0% | ~ |
| `SphereCloudRays` | 178.0 s | 178.9 s | +0.5% | 4098 MB | 4095 MB | -0.1% | ~ |
| `SphereContacts` | 7.2 s | 7.4 s | +2.7% | 465 MB | 465 MB | +0.0% | ~ |
| `Tape` | 64.1 s | 64.5 s | +0.7% | 854 MB | 854 MB | +0.0% | ~ |
| `TapeBVHUnion` | 41.5 s | 41.7 s | +0.4% | 795 MB | 795 MB | +0.1% | ~ |
| **all, in both** | 3501.5 s | 3516.6 s | +0.4% | 5633 MB | 5632 MB | -0.0% | ~ |

Device kernels: 156 in both, 4 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19056 → 19072 | 1048/1072 → 1048/1072 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float, TriMeshSDF<float, 4ul, 4…` | 168 → 168 | 13040 → 13056 | 720/736 → 720/736 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 18672 → 18688 | 1052/1076 → 1052/1076 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float> >, unsigned int const, f…` | 168 → 168 | 13248 → 13264 | 728/740 → 728/740 | ~ |

### Build costs: Heaviest test builds (Debug, assertions)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 5139.2 s → 5150.6 s (+0.2%), the largest peak 8976 MB → 8976 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `TestRayAnalytic` | 4.9 s | 4.8 s | -1.0% | 460 MB | 460 MB | +0.1% | ~ |
| `TestRayCSG` | 244.3 s | 244.2 s | -0.1% | 6093 MB | 6092 MB | -0.0% | ~ |
| `TestRayMesh` | 9.0 s | 9.0 s | -0.5% | 575 MB | 575 MB | +0.0% | ~ |
| `TestRayOffsets` | 2140.4 s | 2134.7 s | -0.3% | 8976 MB | 8976 MB | +0.0% | ~ |
| `TestRender` | 7.7 s | 7.7 s | +0.2% | 835 MB | 836 MB | +0.1% | ~ |
| `TestRenderDevice` | 2595.4 s | 2612.7 s | +0.7% | 8906 MB | 8905 MB | -0.0% | ~ |
| `TestTape` | 111.5 s | 111.7 s | +0.1% | 2982 MB | 2983 MB | +0.0% | ~ |
| `TestTapeClosest` | 26.0 s | 25.8 s | -0.8% | 966 MB | 966 MB | +0.0% | ~ |
| **all, in both** | 5139.2 s | 5150.6 s | +0.2% | 8976 MB | 8976 MB | +0.0% | ~ |

Device kernels: 33 in both, 1 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `TestTape` | `void parallelForKernel<128u, 384u, Apply<TapeQuery<double, Tape<double, Tape<double> > >, …` | 168 → 168 | 10560 → 10576 | 28/0 → 28/0 | ~ |


Flagged after 2 rounds, and judged on 10 (5 cases): MeshCSG/smooth-fused/frame/double/hostdevice, RayCSG/scene-tape/firstHit-marched-random/double/hostdevice, RayShell/armadillo-tape/firstHit-random/double/hostdevice, SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice, SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice

These probes compiled to the same code on both sides and were not run (118 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), Mesh (8), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Shapes (20), SphereCloudBVH (8), SphereContacts (2)

The probes differ between the sides (each side ran its own): Performance/ClosestPrimitive/main.cpp, Performance/RayCSG/main.cpp, Performance/Tape/main.cpp, Performance/TapeBVHUnion/main.cpp

<!-- END receipt cuda -->
