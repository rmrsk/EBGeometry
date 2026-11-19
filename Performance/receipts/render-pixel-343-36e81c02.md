<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: BSD-3-Clause
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
base: fd5c8a5
base-fingerprint: 263791791f122733f874f9739b94c83cd96178ea
head: 08916a3
head-fingerprint: 36e81c02363ee96a6da83fcca3c080330d0a197e
cases: 120
rounds: 2
reps: 3
date: 2026-11-19
-->

## Host comparison: yellow

**Accepted:** I accept the cost as a tie rule. The checksum change is the actual bug this PR is meant to resolve.

`08916a3` against `fd5c8a5` (origin/main), 2026-11-19.

**VERDICT: 7 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 7 minor, 4 faster, 0 differ, 0 lost, 120 cases

- base: fd5c8a5 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 08916a3 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 27.9 ms | 28.1 ms | +0.7% | [-10.1%, +12.8%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 16.0 ms | 16.6 ms | +3.9% | [-8.9%, +18.0%] | +0.0% | ~ | checksum differs (13446.26047 -> 13446.29576) |
| `MeshCSG/smooth-tape/firstHit/float/host` | 335 ms | 327 ms | -2.5% | [-9.0%, +4.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | -0.1% | [-1.1%, +1.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 92.2 ms | 94.2 ms | +2.1% | [+0.5%, +3.7%] | +0.0% | ~ | tape/fused 3.47x (was 3.64x) |
| `MeshCSG/smooth-fused/frame/float/host` | 82.4 ms | 82.7 ms | +0.4% | [-2.6%, +3.4%] | +0.0% | ~ | tape/fused 2.89x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.2 ms | 29.7 ms | +1.6% | [-1.9%, +5.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.6 ms | 17.5 ms | -0.3% | [-3.2%, +2.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 374 ms | 371 ms | -0.9% | [-5.7%, +4.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 250 ms | 252 ms | +0.6% | [-0.7%, +2.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.4 ms | 98.0 ms | +0.6% | [-0.2%, +1.4%] | +0.0% | ~ | tape/fused 3.78x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.6 ms | 87.8 ms | +0.2% | [-0.8%, +1.3%] | +0.0% | ~ | tape/fused 2.87x |
| `MeshTape/mesh/firstHit/float/host` | 46.8 ms | 48.9 ms | +4.6% | [+3.3%, +4.9%] | +0.0% | minor |  |
| `MeshTape/mesh/firstCrossing/float/host` | 47.1 ms | 49.2 ms | +4.5% | [+2.9%, +6.2%] | +0.0% | minor |  |
| `MeshTape/mesh/frame/float/host` | 19.8 ms | 20.6 ms | +3.7% | [-2.9%, +10.9%] | +0.0% | ~ | checksum differs (30644.34942 -> 30644.5577) |
| `MeshTape/tape/firstHit/float/host` | 88.7 ms | 91.0 ms | +2.6% | [-15.9%, +25.6%] | +0.0% | ~ | tape/mesh 0.54x |
| `MeshTape/tape/firstCrossing/float/host` | 58.3 ms | 61.7 ms | +5.9% | [-26.7%, +51.5%] | +0.0% | ~ | tape/mesh 0.80x |
| `MeshTape/tape/frame/float/host` | 25.4 ms | 24.7 ms | -2.9% | [-6.4%, +0.7%] | +0.0% | ~ | tape/mesh 0.83x (was 0.78x); checksum differs (30644.27095 -> 30644.5577) |
| `MeshTape/view/firstHit/float/host` | 81.4 ms | 83.9 ms | +3.2% | [+2.3%, +4.3%] | +0.0% | minor | view/mesh 0.58x |
| `MeshTape/view/frame/float/host` | 26.4 ms | 26.4 ms | +0.1% | [-2.5%, +2.9%] | +0.0% | ~ | view/mesh 0.78x (was 0.75x); checksum differs (30644.27095 -> 30644.5577) |
| `MeshTape/mesh/firstHit/double/host` | 53.4 ms | 54.1 ms | +1.3% | [+0.2%, +5.6%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 53.3 ms | 54.0 ms | +1.4% | [+0.5%, +3.3%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 21.1 ms | 20.5 ms | -3.2% | [-4.2%, -2.3%] | +0.0% | faster |  |
| `MeshTape/tape/firstHit/double/host` | 87.5 ms | 87.8 ms | +0.4% | [-0.7%, +1.4%] | +0.0% | ~ | tape/mesh 0.62x |
| `MeshTape/tape/firstCrossing/double/host` | 58.0 ms | 55.7 ms | -4.0% | [-7.5%, -0.4%] | +0.0% | faster | tape/mesh 0.97x (was 0.92x) |
| `MeshTape/tape/frame/double/host` | 26.4 ms | 26.9 ms | +1.8% | [+0.6%, +3.0%] | +0.0% | ~ | tape/mesh 0.76x (was 0.80x) |
| `MeshTape/view/firstHit/double/host` | 90.6 ms | 91.6 ms | +1.0% | [+0.2%, +3.7%] | +0.0% | ~ | view/mesh 0.59x |
| `MeshTape/view/frame/double/host` | 29.2 ms | 29.7 ms | +1.5% | [-4.4%, +7.8%] | +0.0% | ~ | view/mesh 0.69x (was 0.72x) |
| `Ray/mesh/firstHit/float/host` | 47.0 ms | 49.2 ms | +4.5% | [+1.4%, +5.8%] | +0.0% | minor | hand/api 0.96x (was 1.00x) |
| `Ray/mesh/firstHit-handrolled/float/host` | 47.3 ms | 47.4 ms | +0.2% | [-4.2%, +4.9%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 41.8 ms | 42.5 ms | +1.5% | [-1.3%, +4.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 88.3 ms | 86.0 ms | -2.6% | [-6.3%, +1.2%] | +0.0% | ~ | triangles/polygons 0.57x (was 0.53x) |
| `Ray/mesh-polygons/intersects/float/host` | 69.9 ms | 73.8 ms | +5.6% | [-0.7%, +12.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.23 ms | 5.25 ms | +0.5% | [-1.2%, +2.3%] | - | ~ | tape/fused 1.47x |
| `Ray/csg-tape/firstHit/float/host` | 7.67 ms | 7.72 ms | +0.6% | [-1.5%, +2.8%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 55.2 ms | 55.2 ms | +0.0% | [-7.2%, +7.8%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 53.7 ms | 53.6 ms | -0.1% | [-6.8%, +7.1%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 46.8 ms | 47.7 ms | +2.0% | [-5.8%, +10.5%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 92.5 ms | 92.8 ms | +0.4% | [-3.9%, +4.9%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/double/host` | 77.3 ms | 77.1 ms | -0.3% | [-2.7%, +2.2%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.52 ms | 5.61 ms | +1.5% | [+1.1%, +2.0%] | - | ~ | tape/fused 1.40x |
| `Ray/csg-tape/firstHit/double/host` | 7.95 ms | 7.87 ms | -1.0% | [-1.9%, -0.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.6 ms | 17.4 ms | -1.2% | [-1.6%, -0.7%] | - | ~ | marched/traced 6.91x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 121 ms | 120 ms | -0.6% | [-1.3%, +0.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.1 ms | 21.9 ms | -0.9% | [-1.6%, -0.2%] | +0.0% | ~ | marched/traced 6.67x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 147 ms | 146 ms | -0.8% | [-2.0%, +0.4%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.6 ms | 35.5 ms | -0.3% | [-2.1%, +1.5%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.2 ms | 21.6 ms | +12.5% | [-1.4%, +26.7%] | - | ~ | marched/traced 5.97x (was 6.72x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 129 ms | 129 ms | -0.0% | [-2.9%, +3.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.8 ms | 23.7 ms | -0.3% | [-3.8%, +3.2%] | +0.0% | ~ | marched/traced 6.38x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 154 ms | 151 ms | -1.6% | [-3.2%, +0.1%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.9 ms | 37.8 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 48.2 ms | 49.4 ms | +2.3% | [+0.4%, +4.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.9 ms | 49.2 ms | +4.9% | [+4.0%, +7.5%] | +0.0% | minor | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.81 ms | 7.55 ms | -3.3% | [-5.6%, -0.8%] | +0.0% | faster |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.16 ms | 6.12 ms | -0.8% | [-1.7%, +0.2%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 182 ms | 181 ms | -0.5% | [-1.9%, +0.9%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 159 ms | 159 ms | +0.1% | [-1.8%, +2.2%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 69.3 ms | 69.4 ms | +0.2% | [-1.1%, +1.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.8 ms | 56.7 ms | -0.2% | [-1.5%, +1.2%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 56.2 ms | 54.4 ms | -3.3% | [-12.2%, +7.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 53.7 ms | 55.7 ms | +3.6% | [-2.9%, +10.4%] | +0.0% | ~ | hit/crossing 0.98x (was 1.05x) |
| `RayCrossing/csg-tape/firstHit/double/host` | 8.15 ms | 8.03 ms | -1.4% | [-2.2%, -0.6%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.31 ms | 6.31 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | hit/crossing 1.27x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 212 ms | 205 ms | -3.0% | [-10.2%, +5.1%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 180 ms | 185 ms | +2.8% | [-2.9%, +8.6%] | +0.0% | ~ | hit/crossing 1.11x (was 1.17x) |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 79.3 ms | 80.8 ms | +2.0% | [-3.4%, +7.5%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 68.2 ms | 66.1 ms | -3.1% | [-8.5%, +3.0%] | +0.0% | ~ | hit/crossing 1.22x (was 1.16x) |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.3 ms | 11.5 ms | +1.3% | [-4.5%, +7.5%] | +0.0% | ~ | marched/traced 4.52x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 52.0 ms | 51.8 ms | -0.4% | [-1.4%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.01 ms | 8.02 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.94 ms | 2.02 ms | +4.2% | [+2.5%, +6.0%] | +0.0% | minor |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.91 ms | 6.93 ms | +0.3% | [-0.6%, +1.2%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.1 ms | 11.9 ms | -1.6% | [-4.7%, +1.7%] | +0.0% | ~ | marched/traced 5.40x (was 5.20x) |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 63.0 ms | 64.4 ms | +2.3% | [-7.3%, +12.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.97 ms | 8.65 ms | -3.6% | [-6.5%, -0.5%] | +0.0% | faster |  |
| `RayParticles/armadillo/step/double/host` | 1.96 ms | 2.07 ms | +5.4% | [+3.6%, +8.2%] | +0.0% | minor |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.21 ms | 7.22 ms | +0.2% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 239 ms | 238 ms | -0.2% | [-1.7%, +1.3%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 330 ms | 329 ms | -0.4% | [-1.1%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 241 ms | 238 ms | -1.4% | [-3.0%, +0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 254 ms | 263 ms | +3.4% | [-0.1%, +7.0%] | +0.0% | ~ | marched/traced 1.42x (was 1.48x) |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 376 ms | 374 ms | -0.5% | [-3.2%, +2.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 258 ms | 254 ms | -1.8% | [-2.3%, -1.4%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.33 ms | 7.48 ms | +2.1% | [+1.7%, +2.4%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.7 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 267 ms | 267 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.6 ms | 12.5 ms | -0.3% | [-1.1%, +0.5%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.6 ms | 30.6 ms | +0.2% | [-0.9%, +1.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.0 ms | 22.2 ms | +1.1% | [-0.1%, +2.3%] | +0.0% | ~ | regions/hit 1.38x |
| `Regions/particles/region/double/host` | 7.69 ms | 7.68 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.9 ms | 13.0 ms | +0.8% | [+0.1%, +1.6%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 284 ms | 283 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | 12.9 ms | 13.0 ms | +0.5% | [-0.2%, +1.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.5 ms | 32.3 ms | -0.6% | [-1.6%, +0.5%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.7 ms | 23.5 ms | -0.8% | [-1.0%, -0.6%] | +0.0% | ~ | regions/hit 1.37x |
| `Render/csg/frame/float/host` | 13.1 ms | 12.7 ms | -2.6% | [-7.2%, +1.9%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 12.0 ms | 12.6 ms | +4.6% | [-0.1%, +9.4%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.2 ms | 19.5 ms | +1.7% | [+1.3%, +2.1%] | +0.0% | ~ | checksum differs (30644.34942 -> 30644.5577) |
| `Render/csg/frame/double/host` | 14.3 ms | 14.1 ms | -1.0% | [-2.5%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 13.9 ms | 13.8 ms | -1.1% | [-1.2%, -0.9%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.6 ms | 20.6 ms | +0.1% | [-3.0%, +3.2%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 76.2 ms | 75.8 ms | -0.5% | [-2.5%, +1.5%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 316 ms | 320 ms | +1.2% | [-9.8%, +13.4%] | +0.0% | ~ | tape/fused 0.95x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 303 ms | 305 ms | +0.5% | [-2.9%, +4.0%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 96.3 ms | 90.4 ms | -6.1% | [-15.8%, +5.5%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 363 ms | 365 ms | +0.7% | [-5.5%, +7.4%] | +0.0% | ~ | tape/fused 0.97x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 355 ms | 355 ms | +0.0% | [-1.7%, +1.8%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 144 ms | 145 ms | +0.9% | [-0.1%, +2.0%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 90.9 ms | 91.5 ms | +0.8% | [-1.6%, +3.2%] | +0.0% | ~ | surface/leaf 1.58x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 134 ms | 133 ms | -0.6% | [-4.6%, +3.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 74.3 ms | 73.6 ms | -1.0% | [-2.8%, +0.8%] | +0.0% | ~ | surface/leaf 1.81x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 63.3 ms | 63.2 ms | -0.1% | [-4.0%, +3.9%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.7 ms | 34.4 ms | -0.7% | [-2.2%, +2.4%] | +0.0% | ~ | surface/hit 1.84x |
| `SurfaceParts/boxes/closestSurface/double/host` | 147 ms | 145 ms | -1.5% | [-2.5%, -0.5%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 90.9 ms | 91.3 ms | +0.5% | [-0.3%, +1.3%] | +0.0% | ~ | surface/leaf 1.58x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 148 ms | 147 ms | -0.1% | [-2.6%, +2.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 85.0 ms | 84.1 ms | -1.1% | [-7.6%, +5.9%] | +0.0% | ~ | surface/leaf 1.75x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 68.6 ms | 69.5 ms | +1.4% | [+0.4%, +2.5%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 39.8 ms | 40.2 ms | +0.9% | [-2.7%, +4.6%] | +0.0% | ~ | surface/hit 1.73x |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 875.8 s → 971.5 s (+10.9%), the largest peak 1409 MB → 1413 MB; 13 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.2 s | 10.0 s | -18.4% | 558 MB | 558 MB | -0.0% | ~ |
| `BVHRefit` | 5.6 s | 4.4 s | -21.3% | 496 MB | 497 MB | +0.1% | ~ |
| `BVHUnion` | 25.5 s | 23.2 s | -9.0% | 948 MB | 948 MB | +0.0% | ~ |
| `BVHUnionMixed` | 10.1 s | 7.9 s | -22.1% | 556 MB | 556 MB | +0.0% | ~ |
| `ClosestPrimitive` | 40.1 s | 37.9 s | -5.3% | 1296 MB | 1297 MB | +0.0% | ~ |
| `ClosestSurface` | 37.5 s | 38.0 s | +1.1% | 1227 MB | 1228 MB | +0.1% | ~ |
| `Domain` | 26.2 s | 21.0 s | -19.9% | 843 MB | 843 MB | +0.0% | ~ |
| `DomainObject` | 20.8 s | 18.1 s | -12.9% | 846 MB | 846 MB | -0.0% | ~ |
| `Fused` | 33.1 s | 33.5 s | +1.0% | 1136 MB | 1137 MB | +0.1% | ~ |
| `LaunchTuning` | 46.5 s | 50.6 s | +8.9% | 1310 MB | 1312 MB | +0.1% | ~ |
| `Mesh` | 20.7 s | 24.6 s | +18.7% | 784 MB | 784 MB | +0.0% | **minor** |
| `MeshCSG` | 48.8 s | 56.5 s | +15.8% | 1255 MB | 1255 MB | +0.0% | **minor** |
| `MeshClosestPoint` | 18.5 s | 19.4 s | +4.9% | 781 MB | 781 MB | +0.0% | ~ |
| `MeshReconcile` | 8.3 s | 9.0 s | +7.8% | 566 MB | 566 MB | +0.1% | ~ |
| `MeshTape` | 50.1 s | 64.0 s | +27.8% | 1288 MB | 1288 MB | -0.0% | **MAJOR** |
| `PointCloud` | 10.4 s | 11.5 s | +10.7% | 558 MB | 559 MB | +0.2% | ~ |
| `PointCloudBVHBuild` | 4.1 s | 5.1 s | +24.9% | 471 MB | 471 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.4 s | 5.6 s | +64.1% | 452 MB | 452 MB | -0.0% | **MAJOR** |
| `Ray` | 43.9 s | 52.7 s | +20.1% | 1317 MB | 1318 MB | +0.0% | **minor** |
| `RayCSG` | 38.7 s | 52.0 s | +34.5% | 1188 MB | 1183 MB | -0.4% | **MAJOR** |
| `RayCrossing` | 53.7 s | 66.4 s | +23.6% | 1331 MB | 1331 MB | +0.0% | **minor** |
| `RayParticles` | 57.6 s | 74.4 s | +29.1% | 1319 MB | 1319 MB | -0.0% | **MAJOR** |
| `RayShell` | 44.7 s | 53.2 s | +19.1% | 1285 MB | 1286 MB | +0.0% | **minor** |
| `Regions` | 34.4 s | 41.9 s | +21.9% | 1073 MB | 1073 MB | +0.0% | **minor** |
| `Render` | 41.0 s | 47.3 s | +15.4% | 1284 MB | 1285 MB | +0.1% | **minor** |
| `Shapes` | 11.3 s | 13.5 s | +19.6% | 554 MB | 554 MB | +0.1% | **minor** |
| `SphereCloudBVH` | 5.5 s | 7.9 s | +43.5% | 510 MB | 511 MB | +0.2% | **MAJOR** |
| `SphereCloudRays` | 31.8 s | 32.1 s | +1.0% | 1063 MB | 1064 MB | +0.1% | ~ |
| `SphereContacts` | 4.9 s | 4.5 s | -9.0% | 484 MB | 485 MB | +0.1% | ~ |
| `SurfaceParts` | 49.0 s | 49.2 s | +0.4% | 1409 MB | 1413 MB | +0.3% | ~ |
| `Tape` | 19.9 s | 19.2 s | -3.0% | 831 MB | 830 MB | -0.1% | ~ |
| `TapeBVHUnion` | 17.3 s | 16.6 s | -4.3% | 818 MB | 818 MB | +0.0% | ~ |
| **all, in both** | 875.8 s | 971.5 s | +10.9% | 1409 MB | 1413 MB | +0.3% | **minor** |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (11 cases): MeshTape/mesh/firstHit/float/host, MeshTape/mesh/firstCrossing/float/host, MeshTape/view/firstHit/float/host, MeshTape/mesh/firstHit/double/host, MeshTape/mesh/firstCrossing/double/host, MeshTape/view/firstHit/double/host, Ray/mesh/firstHit/float/host, RayCrossing/mesh/firstCrossing/float/host, RayParticles/armadillo/step/float/host, RayParticles/armadillo/step/double/host, SurfaceParts/mesh-groups/firstHit/float/host

These probes compiled to the same code on both sides and were not run (182 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), Fused (10), Mesh (12), MeshClosestPoint (8), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20), SphereCloudBVH (6), SphereContacts (2), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: fd5c8a5
base-fingerprint: 263791791f122733f874f9739b94c83cd96178ea
head: 08916a3
head-fingerprint: 36e81c02363ee96a6da83fcca3c080330d0a197e
cases: 212
rounds: 2
reps: 3
date: 2026-11-19
-->

## Device comparison (CUDA): yellow

**Accepted:** I accept the cost as a tie rule. The checksum change is the actual bug this PR is meant to resolve.

`08916a3` against `fd5c8a5` (origin/main), 2026-11-19.

**VERDICT: 7 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 7 minor, 9 faster, 0 differ, 0 lost, 212 cases

- base: fd5c8a5 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 08916a3 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.28 ms | 7.87 ms | -5.0% | [-12.7%, +3.0%] | +0.0% | ~ | device 11.4x faster than the host (was 10.8x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.23 ms | 8.36 ms | +1.5% | [+1.1%, +1.9%] | +0.0% | ~ | device 10.7x faster than the host (was 11.0x); default/this 0.94x (was 1.01x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.23 ms | 8.20 ms | -0.4% | [-1.2%, +0.5%] | +0.0% | ~ | device 10.9x faster than the host; default/this 0.96x (was 1.01x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.09 ms | 8.20 ms | +1.4% | [-0.3%, +3.1%] | +0.0% | ~ | device 11.2x faster than the host (was 12.6x); default/this 0.96x (was 1.02x) |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 712 µs | 691 µs | -2.9% | [-6.9%, +1.4%] | +0.0% | ~ | device 3.8x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.76 ms | 1.76 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.04 ms | 1.06 ms | +2.2% | [+1.7%, +2.7%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.65x (was 0.69x) |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 778 µs | 785 µs | +1.0% | [+0.1%, +6.6%] | +0.0% | ~ | device 3.3x faster than the host; default/this 0.88x (was 0.92x) |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 738 µs | 739 µs | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ | device 12.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 845 µs | 846 µs | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | device 10.9x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 765 µs | 736 µs | -3.8% | [-6.2%, -1.3%] | +0.0% | faster | device 12.3x faster than the host (was 11.8x); default/this 1.01x (was 0.97x) |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 725 µs | 734 µs | +1.2% | [+0.4%, +2.1%] | +0.0% | ~ | device 12.3x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 136 ms | 138 ms | +1.6% | [-0.3%, +2.7%] | +0.0% | ~ | device 0.7x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 139 ms | 147 ms | +5.5% | [-1.8%, +12.9%] | +0.0% | ~ | device 0.6x faster than the host (was 0.7x); default/this 0.94x (was 0.98x) |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 141 ms | 145 ms | +2.8% | [-3.3%, +9.1%] | +0.0% | ~ | device 0.6x faster than the host (was 0.7x); default/this 0.95x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 138 ms | 139 ms | +0.2% | [-2.4%, +3.3%] | +0.0% | ~ | device 0.7x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.23 ms | 1.22 ms | -1.1% | [-3.7%, +1.7%] | +0.0% | ~ | device 2.2x faster than the host (was 3.1x) |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.98 ms | 2.95 ms | -0.9% | [-10.4%, +9.3%] | +0.0% | ~ | device 0.9x faster than the host; default/this 0.41x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.91 ms | 1.87 ms | -2.1% | [-3.2%, -0.9%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.65x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.47 ms | 1.46 ms | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ | device 1.8x faster than the host; default/this 0.83x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 6.59 ms | 6.59 ms | -0.1% | [-19.5%, +24.0%] | +0.0% | ~ | device 1.4x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.31 ms | 7.25 ms | -0.8% | [-1.2%, -0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.91x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.26 ms | 7.27 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.91x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.29 ms | 7.29 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.90x |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.00 ms | 2.02 ms | +1.0% | [-0.1%, +2.1%] | - | ~ | device 5.3x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.35 ms | 3.38 ms | +0.9% | [-0.0%, +1.8%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.69 ms | 6.16 ms | -7.9% | [-15.9%, +1.7%] | - | ~ | device 4.8x faster than the host (was 4.4x); checksum differs (215184.54 -> 215185.0491) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 60.1 ms | 60.9 ms | +1.4% | [-11.2%, +16.5%] | - | ~ | device 3.4x faster than the host (was 3.6x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 93.8 ms | 85.7 ms | -8.6% | [-14.3%, -2.0%] | - | faster | device 2.6x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 233 ms | 238 ms | +2.3% | [-6.9%, +12.0%] | - | ~ | device 4.4x faster than the host (was 3.9x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 20.3 ms | 20.4 ms | +0.2% | [-0.8%, +4.1%] | - | ~ | device 2.5x faster than the host; tape/fused 2.99x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 25.8 ms | 23.1 ms | -10.1% | [-18.4%, -0.2%] | - | faster | device 3.4x faster than the host (was 3.1x); tape/fused 3.70x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 56.1 ms | 61.5 ms | +9.6% | [-1.0%, +14.3%] | - | ~ | device 4.7x faster than the host; tape/fused 3.87x (was 4.15x) |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 19.5 ms | 19.0 ms | -2.5% | [-17.7%, +15.8%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 32.0 ms | 32.5 ms | +1.6% | [-1.9%, +5.1%] | - | ~ | device 0.6x faster than the host (was 0.6x) |
| `MeshCSG/union-tape/frame/double/hostdevice` | 34.0 ms | 36.7 ms | +7.9% | [-1.3%, +17.4%] | - | ~ | device 1.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 582 ms | 570 ms | -2.0% | [-4.2%, +0.3%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 939 ms | 953 ms | +1.5% | [-5.2%, +8.2%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.01 s | 2.04 s | +1.2% | [-2.3%, +4.7%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 179 ms | 176 ms | -1.7% | [-6.9%, +3.5%] | - | ~ | device 0.4x faster than the host; tape/fused 3.23x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 301 ms | 296 ms | -1.6% | [-4.7%, +1.6%] | - | ~ | device 0.3x faster than the host (was 0.3x); tape/fused 3.22x (was 3.12x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 557 ms | 560 ms | +0.6% | [-5.7%, +7.6%] | - | ~ | device 0.7x faster than the host (was 0.6x); tape/fused 3.63x |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.29 ms | 1.23 ms | -4.7% | [-10.7%, +1.5%] | - | ~ | device 22.2x faster than the host (was 20.0x) |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.28 ms | 1.30 ms | +1.6% | [+0.7%, +2.5%] | - | ~ | device 21.1x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.11 ms | 2.14 ms | +1.6% | [+1.2%, +1.9%] | - | ~ | device 10.6x faster than the host (was 11.0x) |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.33 ms | 3.49 ms | +4.8% | [-9.2%, +20.2%] | - | ~ | device 11.3x faster than the host; tape/mesh 0.35x (was 0.39x) |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 3.37 ms | 2.75 ms | -18.4% | [-32.8%, +3.9%] | - | ~ | device 10.3x faster than the host (was 7.9x); tape/mesh 0.47x (was 0.38x) |
| `MeshTape/tape/frame/float/hostdevice` | 4.31 ms | 4.43 ms | +2.8% | [+2.3%, +3.5%] | - | ~ | device 5.0x faster than the host; tape/mesh 0.48x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.54 ms | 3.73 ms | +5.3% | [+5.1%, +5.5%] | - | minor | device 9.8x faster than the host; view/mesh 0.33x (was 0.36x) |
| `MeshTape/view/frame/float/hostdevice` | 6.21 ms | 5.59 ms | -10.0% | [-19.6%, +2.3%] | - | ~ | device 5.6x faster than the host (was 4.8x); view/mesh 0.38x (was 0.34x) |
| `MeshTape/mesh/firstHit/double/hostdevice` | 25.1 ms | 25.9 ms | +3.5% | [-14.6%, +25.3%] | - | ~ | device 1.1x faster than the host (was 1.0x) |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 25.5 ms | 24.8 ms | -2.5% | [-14.2%, +25.0%] | - | ~ | device 0.9x faster than the host (was 0.8x) |
| `MeshTape/mesh/frame/double/hostdevice` | 19.8 ms | 20.2 ms | +2.0% | [+1.5%, +2.5%] | - | ~ | device 0.9x faster than the host (was 1.1x) |
| `MeshTape/tape/firstHit/double/hostdevice` | 34.1 ms | 35.2 ms | +3.5% | [-4.7%, +12.4%] | - | ~ | device 1.3x faster than the host (was 1.1x); tape/mesh 0.74x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 22.7 ms | 23.7 ms | +4.5% | [+2.4%, +5.3%] | - | minor | device 1.0x faster than the host; tape/mesh 1.05x (was 1.12x) |
| `MeshTape/tape/frame/double/hostdevice` | 27.8 ms | 26.6 ms | -4.4% | [-13.4%, +6.5%] | - | ~ | device 1.0x faster than the host; tape/mesh 0.76x (was 0.71x) |
| `MeshTape/view/firstHit/double/hostdevice` | 34.4 ms | 34.3 ms | -0.5% | [-7.4%, +7.3%] | - | ~ | device 1.6x faster than the host (was 1.1x); view/mesh 0.76x (was 0.73x) |
| `MeshTape/view/frame/double/hostdevice` | 27.7 ms | 28.5 ms | +2.7% | [+1.4%, +4.1%] | - | ~ | device 0.9x faster than the host; view/mesh 0.71x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 625 µs | 632 µs | +1.0% | [-13.8%, +18.7%] | - | ~ | device 20.0x faster than the host (was 18.8x); hand/api 1.03x (was 1.06x) |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.12 ms | 1.14 ms | +1.7% | [-4.7%, +7.9%] | - | ~ | device 17.8x faster than the host (was 16.9x); hand/api 1.07x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 664 µs | 650 µs | -2.2% | [-2.6%, -1.8%] | - | ~ | device 18.1x faster than the host (was 17.5x) |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.23 ms | -0.3% | [-0.9%, +0.4%] | - | ~ | device 16.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 615 µs | 610 µs | -0.8% | [-1.9%, +0.4%] | - | ~ | device 16.9x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 15.4x faster than the host (was 16.1x) |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.20 ms | +0.8% | [+0.4%, +1.3%] | - | ~ | device 12.6x faster than the host; triangles/polygons 0.53x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.49 ms | 2.49 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 10.4x faster than the host; triangles/polygons 0.46x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.68 ms | 1.07 ms | -36.0% | [-53.2%, +0.6%] | - | ~ | device 12.8x faster than the host (was 9.1x) |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.19 ms | 2.18 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 10.4x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 634 µs | 615 µs | -3.0% | [-5.9%, -0.0%] | - | ~ | device 2.5x faster than the host (was 2.4x); tape/fused 1.16x (was 1.10x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 747 µs | 679 µs | -9.2% | [-13.0%, -5.2%] | - | faster | device 2.8x faster than the host (was 2.6x); tape/fused 1.56x (was 1.15x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 698 µs | 711 µs | +1.9% | [-2.5%, +6.5%] | - | ~ | device 3.8x faster than the host (was 3.9x) |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 863 µs | 1.06 ms | +23.1% | [-0.7%, +47.3%] | - | ~ | device 3.2x faster than the host (was 3.8x) |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.6 ms | +4.3% | [+3.5%, +6.6%] | - | minor | device 0.7x faster than the host; hand/api 0.80x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 23.4 ms | 25.9 ms | +10.7% | [-3.0%, +25.3%] | - | ~ | device 0.9x faster than the host (was 0.9x); hand/api 0.96x (was 1.22x) |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 13.9 ms | 14.2 ms | +1.8% | [+0.9%, +2.6%] | - | ~ | device 1.0x faster than the host (was 0.9x) |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 28.5 ms | 24.9 ms | -12.9% | [-23.6%, -1.5%] | - | faster | device 1.1x faster than the host (was 0.9x) |
| `Ray/mesh/intersects-morton/double/hostdevice` | 15.3 ms | 14.7 ms | -3.8% | [-12.4%, +5.8%] | - | ~ | device 1.0x faster than the host (was 0.8x) |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.6 ms | 24.8 ms | +0.8% | [-1.1%, +2.6%] | - | ~ | device 1.0x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.3 ms | 17.1 ms | +4.9% | [+1.9%, +12.4%] | - | minor | device 0.9x faster than the host (was 0.9x); triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.1 ms | 24.4 ms | -2.5% | [-17.6%, +13.0%] | - | ~ | device 2.2x faster than the host (was 1.7x); triangles/polygons 1.06x (was 0.93x) |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 14.9 ms | 15.0 ms | +0.7% | [-0.5%, +10.6%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 21.0 ms | 19.2 ms | -8.9% | [-17.6%, +1.8%] | - | ~ | device 2.1x faster than the host (was 2.0x) |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.03 ms | 1.04 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 2.4x faster than the host; tape/fused 1.24x (was 1.31x) |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 1.97 ms | 1.96 ms | -0.6% | [-0.9%, -0.4%] | - | ~ | device 1.2x faster than the host (was 1.4x); tape/fused 1.07x (was 1.19x) |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.35 ms | 1.28 ms | -4.7% | [-16.3%, +9.2%] | - | ~ | device 3.0x faster than the host (was 2.7x) |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.35 ms | 2.09 ms | -11.0% | [-19.7%, -0.1%] | - | faster | device 2.0x faster than the host (was 1.8x) |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.48 ms | 3.26 ms | +31.4% | [-2.7%, +66.6%] | - | ~ | device 2.5x faster than the host (was 3.4x); marched/traced 2.81x (was 3.32x) |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.46 ms | 2.94 ms | -15.1% | [-27.0%, +0.5%] | - | ~ | device 3.1x faster than the host (was 2.8x); marched/traced 4.71x (was 4.00x) |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 8.25 ms | 9.19 ms | +11.4% | [-0.0%, +25.2%] | - | ~ | device 6.5x faster than the host (was 7.1x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | +0.0% | [-0.8%, +1.3%] | - | ~ | device 3.8x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.19 ms | 2.19 ms | -0.2% | [-2.3%, +2.6%] | - | ~ | device 3.9x faster than the host; marched/traced 13.19x (was 15.46x) |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 6.49 ms | 5.94 ms | -8.4% | [-29.5%, +21.7%] | - | ~ | device 3.7x faster than the host (was 3.5x); marched/traced 10.11x (was 9.10x) |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 33.9 ms | 28.9 ms | -14.9% | [-31.3%, +8.3%] | - | ~ | device 3.2x faster than the host (was 2.7x) |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 59.0 ms | 60.1 ms | +1.8% | [-16.2%, +22.9%] | - | ~ | device 2.7x faster than the host (was 2.5x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.29 ms | 5.40 ms | +2.1% | [-12.9%, +19.3%] | - | ~ | device 4.1x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.49 ms | 8.66 ms | +2.0% | [-0.8%, +4.8%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 5.77 ms | 5.73 ms | -0.6% | [-14.8%, +15.9%] | - | ~ | device 1.6x faster than the host (was 1.8x); marched/traced 27.00x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.0x faster than the host (was 1.1x); marched/traced 23.39x (was 24.39x) |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 153 ms | 155 ms | +0.8% | [-16.1%, +21.2%] | - | ~ | device 0.4x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 258 ms | 248 ms | -4.1% | [-11.2%, +4.2%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.86 ms | 4.64 ms | -4.5% | [-15.6%, +8.5%] | - | ~ | device 2.5x faster than the host (was 2.7x); marched/traced 22.09x (was 19.88x) |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.8 ms | 12.6 ms | -1.9% | [-19.0%, +19.2%] | - | ~ | device 1.7x faster than the host; marched/traced 16.09x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 96.6 ms | 102 ms | +6.1% | [-2.8%, +15.4%] | - | ~ | device 0.9x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 211 ms | 203 ms | -4.0% | [-8.7%, +1.1%] | - | ~ | device 0.8x faster than the host (was 0.7x) |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 16.2 ms | 16.3 ms | +0.8% | [-0.3%, +1.9%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 22.8 ms | 24.1 ms | +5.7% | [-1.1%, +12.7%] | - | ~ | device 1.6x faster than the host (was 1.6x) |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 672 µs | 686 µs | +2.1% | [+1.1%, +9.7%] | - | ~ | device 18.0x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.23 ms | -4.3% | [-10.7%, +2.2%] | - | ~ | device 21.4x faster than the host (was 19.9x) |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 680 µs | 691 µs | +1.6% | [+1.3%, +1.8%] | - | ~ | device 23.4x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.30 ms | +1.6% | [+0.5%, +2.8%] | - | ~ | device 21.2x faster than the host; hit/crossing 0.94x (was 1.00x) |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 1.30 ms | 732 µs | -43.6% | [-63.9%, +13.0%] | - | ~ | device 5.3x faster than the host (was 3.9x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 876 µs | 1.47 ms | +68.3% | [-4.8%, +144.2%] | - | ~ | device 4.2x faster than the host (was 5.0x) |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 623 µs | 694 µs | +11.3% | [-0.5%, +23.2%] | - | ~ | device 3.9x faster than the host (was 4.7x); hit/crossing 1.06x (was 2.08x) |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 648 µs | 641 µs | -1.1% | [-1.9%, -0.4%] | - | ~ | device 4.9x faster than the host (was 5.1x); hit/crossing 2.30x (was 1.35x) |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 5.62 ms | 4.75 ms | -15.5% | [-26.9%, +0.1%] | - | ~ | device 13.8x faster than the host (was 13.4x) |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.3 ms | +0.5% | [+0.3%, +0.6%] | - | ~ | device 10.2x faster than the host (was 9.7x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.84 ms | +15.9% | [-0.1%, +31.9%] | - | ~ | device 12.5x faster than the host (was 13.9x); hit/crossing 0.98x (was 1.34x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 10.0 ms | 9.71 ms | -3.2% | [-17.8%, +14.5%] | - | ~ | device 9.4x faster than the host (was 8.6x); hit/crossing 1.06x (was 1.02x) |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.3 ms | 54.0 ms | -4.1% | [-10.5%, +2.9%] | - | ~ | device 4.5x faster than the host (was 4.0x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 80.6 ms | 80.4 ms | -0.2% | [-6.8%, +6.8%] | - | ~ | device 3.2x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 45.5 ms | 43.9 ms | -3.6% | [-15.7%, +10.5%] | - | ~ | device 4.9x faster than the host (was 4.3x); hit/crossing 1.23x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 64.7 ms | 66.8 ms | +3.2% | [-10.9%, +19.4%] | - | ~ | device 3.1x faster than the host (was 3.8x); hit/crossing 1.20x (was 1.25x) |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 15.5 ms | 18.1 ms | +17.0% | [-5.8%, +44.6%] | - | ~ | device 0.9x faster than the host (was 1.0x) |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 25.2 ms | 25.9 ms | +2.9% | [-11.1%, +18.7%] | - | ~ | device 1.2x faster than the host (was 1.3x) |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 16.9 ms | 17.6 ms | +3.8% | [+0.4%, +20.3%] | - | minor | device 0.7x faster than the host (was 0.7x); hit/crossing 1.03x (was 0.91x) |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 28.3 ms | 23.9 ms | -15.4% | [-18.3%, -12.4%] | - | faster | device 1.3x faster than the host (was 1.0x); hit/crossing 1.08x (was 0.89x) |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.23 ms | 1.22 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 3.3x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.09 ms | 2.09 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.04 ms | 1.13 ms | +8.6% | [-2.7%, +20.3%] | - | ~ | device 2.5x faster than the host (was 2.6x); hit/crossing 1.08x (was 1.17x) |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.14 ms | 1.16 ms | +1.9% | [-0.5%, +11.0%] | - | ~ | device 2.1x faster than the host (was 2.2x); hit/crossing 1.79x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 56.2 ms | 53.2 ms | -5.4% | [-9.1%, -1.5%] | - | faster | device 1.2x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 105 ms | 105 ms | +0.4% | [-5.0%, +6.1%] | - | ~ | device 0.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 48.5 ms | 50.6 ms | +4.2% | [-0.7%, +9.2%] | - | ~ | device 1.3x faster than the host (was 1.1x); hit/crossing 1.05x (was 1.16x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 95.7 ms | 95.6 ms | -0.1% | [-4.7%, +4.7%] | - | ~ | device 0.9x faster than the host (was 1.1x); hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 488 ms | 491 ms | +0.7% | [-5.2%, +7.1%] | - | ~ | device 0.5x faster than the host (was 0.6x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 740 ms | 736 ms | -0.4% | [-5.8%, +5.2%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 386 ms | 410 ms | +6.1% | [-8.1%, +22.0%] | - | ~ | device 0.6x faster than the host (was 0.5x); hit/crossing 1.20x (was 1.26x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 533 ms | 538 ms | +0.9% | [-4.6%, +6.2%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.37x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.61 ms | 3.60 ms | -0.2% | [-3.0%, +2.7%] | - | ~ | device 1.5x faster than the host (was 1.4x); marched/traced 3.40x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.94 ms | 5.64 ms | +14.3% | [-6.0%, +36.1%] | - | ~ | device 1.0x faster than the host; marched/traced 4.72x (was 5.38x) |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.3 ms | +2.2% | [+0.1%, +4.2%] | - | ~ | device 4.0x faster than the host (was 4.2x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.6 ms | +0.2% | [-0.1%, +9.8%] | - | ~ | device 2.1x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.27 ms | 1.26 ms | -0.9% | [-3.1%, +1.5%] | - | ~ | device 2.6x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.85 ms | +0.0% | [-0.7%, +0.8%] | - | ~ | device 1.8x faster than the host (was 2.7x) |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 98.3 µs | 95.6 µs | -2.8% | [-4.6%, -0.9%] | - | ~ | device 27.4x faster than the host (was 22.2x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 153 µs | 155 µs | +1.2% | [-2.1%, +4.8%] | - | ~ | device 17.0x faster than the host (was 17.8x) |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 796 µs | 796 µs | +0.1% | [-0.4%, +0.5%] | - | ~ | device 4.6x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 926 µs | 931 µs | +0.5% | [+0.2%, +0.8%] | - | ~ | device 4.1x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.9 ms | 35.5 ms | +1.8% | [-4.9%, +8.6%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 2.92x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.9 ms | 36.6 ms | -0.8% | [-2.6%, +1.0%] | - | ~ | device 0.1x faster than the host; marched/traced 5.90x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 105 ms | 104 ms | -0.7% | [-4.0%, +4.3%] | - | ~ | device 0.5x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 214 ms | 216 ms | +1.2% | [-2.6%, +5.1%] | - | ~ | device 0.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.3 ms | 16.8 ms | +3.6% | [-3.3%, +8.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 21.9 ms | +0.3% | [-1.0%, +1.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.05 ms | 1.10 ms | +4.8% | [-0.3%, +10.3%] | - | ~ | device 1.4x faster than the host (was 1.5x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.33 ms | 2.43 ms | +4.4% | [+3.4%, +5.5%] | - | minor | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.97 ms | 5.96 ms | -0.1% | [-0.9%, +0.7%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.51 ms | 6.53 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 20.8 ms | 24.5 ms | +17.8% | [-0.0%, +38.4%] | - | ~ | device 2.1x faster than the host (was 2.5x); marched/traced 2.28x (was 2.58x) |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 25.9 ms | 23.7 ms | -8.4% | [-18.6%, +4.0%] | - | ~ | device 2.4x faster than the host (was 2.2x); marched/traced 3.96x (was 3.63x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 53.7 ms | 55.8 ms | +4.0% | [-0.2%, +8.1%] | - | ~ | device 4.6x faster than the host (was 4.3x) |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 94.0 ms | 94.1 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 2.7x faster than the host (was 2.6x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 12.6 ms | 12.7 ms | +0.6% | [-15.8%, +20.2%] | - | ~ | device 4.1x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.2 ms | 17.2 ms | +0.3% | [-3.3%, +4.0%] | - | ~ | device 3.3x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 292 ms | 286 ms | -2.3% | [-5.0%, +0.5%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 2.03x (was 1.92x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 352 ms | 342 ms | -2.8% | [-7.1%, +1.7%] | - | ~ | device 0.2x faster than the host; marched/traced 2.98x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 563 ms | 580 ms | +3.0% | [-0.4%, +6.4%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.04 s | 1.02 s | -2.4% | [-9.0%, +4.8%] | - | ~ | device 0.3x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 168 ms | 166 ms | -1.4% | [-2.1%, -0.7%] | - | ~ | device 0.3x faster than the host (was 0.4x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 218 ms | 216 ms | -0.9% | [-3.9%, +2.2%] | - | ~ | device 0.3x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.49 ms | 2.48 ms | -0.4% | [-5.1%, +4.5%] | - | ~ | device 3.2x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.20 ms | 3.19 ms | -0.5% | [-1.0%, -0.0%] | - | ~ | device 4.2x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 105 ms | 100 ms | -5.0% | [-10.1%, +0.7%] | - | ~ | device 3.3x faster than the host; each/one-pass 0.03x (was 0.03x) |
| `Regions/particles/domainLabels/float/hostdevice` | 3.84 ms | 3.81 ms | -0.8% | [-32.2%, +45.5%] | - | ~ | device 3.9x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.61 ms | 6.60 ms | -0.2% | [-5.9%, +5.9%] | - | ~ | device 3.9x faster than the host (was 5.7x) |
| `Regions/assembly/firstHit/float/hostdevice` | 5.61 ms | 5.59 ms | -0.3% | [-1.8%, +1.3%] | - | ~ | device 3.3x faster than the host (was 3.5x); regions/hit 1.18x |
| `Regions/particles/region/double/hostdevice` | 3.72 ms | 3.76 ms | +1.1% | [-1.9%, +4.1%] | - | ~ | device 2.1x faster than the host (was 2.1x) |
| `Regions/particles/regionDistances/double/hostdevice` | 5.98 ms | 6.00 ms | +0.4% | [-0.3%, +1.1%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 146 ms | 144 ms | -1.7% | [-3.6%, +0.2%] | - | ~ | device 2.4x faster than the host; each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.29 ms | 5.39 ms | +1.9% | [-7.2%, +11.6%] | - | ~ | device 2.7x faster than the host (was 2.8x) |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 16.5 ms | +4.4% | [-0.6%, +9.5%] | - | ~ | device 1.5x faster than the host (was 1.6x) |
| `Regions/assembly/firstHit/double/hostdevice` | 11.5 ms | 11.5 ms | +0.2% | [-1.0%, +1.3%] | - | ~ | device 1.5x faster than the host; regions/hit 1.44x (was 1.38x) |
| `Render/csg/frame/float/hostdevice` | 2.29 ms | 2.31 ms | +0.7% | [-0.8%, +2.2%] | - | ~ | device 3.2x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.27 ms | 2.28 ms | +0.4% | [-0.5%, +1.4%] | - | ~ | device 3.2x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.08 ms | 2.15 ms | +3.1% | [+2.1%, +11.1%] | - | minor | device 7.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.99 ms | 5.23 ms | -12.7% | [-22.7%, -1.3%] | - | faster | device 1.6x faster than the host (was 1.4x) |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.23 ms | 5.18 ms | -1.0% | [-11.1%, +10.4%] | - | ~ | device 1.6x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 19.8 ms | 20.4 ms | +2.9% | [+2.1%, +3.7%] | - | ~ | device 0.9x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 2.66 ms | 2.66 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ | device 8.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.65 ms | 4.65 ms | +0.0% | [-1.0%, +1.1%] | +0.0% | ~ | device 5.5x faster than the host (was 5.3x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.8 ms | 16.8 ms | +0.2% | [+0.0%, +5.1%] | +0.0% | ~ | device 5.2x faster than the host; tape/fused 1.32x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.7 ms | 24.6 ms | -0.5% | [-2.9%, +0.4%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.23x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.2 ms | 22.2 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ | device 4.4x faster than the host (was 4.2x) |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 29.8 ms | 30.1 ms | +1.1% | [-0.2%, +2.4%] | +0.0% | ~ | device 3.1x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 36.6 ms | 36.8 ms | +0.5% | [-0.6%, +1.6%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 38.0 ms | 37.8 ms | -0.4% | [-1.4%, +0.6%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 112 ms | 107 ms | -4.6% | [-9.1%, +0.4%] | +0.0% | ~ | device 0.9x faster than the host (was 0.8x); tape/fused 1.05x (was 1.00x) |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 113 ms | 113 ms | +0.0% | [-4.5%, +4.8%] | +0.0% | ~ | device 0.9x faster than the host (was 1.0x); tape/fused 1.05x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 112 ms | 112 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 119 ms | 119 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ | device 0.9x faster than the host (was 0.9x) |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 7.77 ms | 9.15 ms | +17.7% | [-0.1%, +35.6%] | - | ~ | device 12.7x faster than the host (was 15.8x) |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.92 ms | 6.39 ms | +8.0% | [-0.1%, +16.2%] | - | ~ | device 10.1x faster than the host (was 10.7x); surface/leaf 1.43x (was 1.31x) |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 96.0 ms | 96.1 ms | +0.0% | [-5.4%, +5.8%] | - | ~ | device 4.3x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 62.0 ms | 61.6 ms | -0.6% | [-5.5%, +4.6%] | - | ~ | device 3.8x faster than the host (was 3.4x); surface/leaf 1.56x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 26.9 ms | 27.0 ms | +0.3% | [+0.1%, +0.6%] | - | ~ | device 7.1x faster than the host (was 5.5x) |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.92 ms | 9.43 ms | -5.0% | [-10.5%, +0.5%] | - | ~ | device 7.8x faster than the host (was 7.3x); surface/hit 2.87x (was 2.71x) |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 49.1 ms | 50.1 ms | +1.9% | [-0.2%, +4.1%] | - | ~ | device 2.2x faster than the host |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 32.3 ms | 32.3 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.1x faster than the host (was 1.9x); surface/leaf 1.55x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 798 ms | 832 ms | +4.3% | [-1.9%, +10.6%] | - | ~ | device 0.5x faster than the host (was 0.6x) |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 533 ms | 541 ms | +1.5% | [-1.3%, +4.4%] | - | ~ | device 0.5x faster than the host (was 0.4x); surface/leaf 1.54x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 302 ms | 309 ms | +2.2% | [-1.2%, +5.6%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 102 ms | 102 ms | +0.6% | [-0.1%, +1.3%] | - | ~ | device 0.7x faster than the host; surface/hit 3.02x |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 6354.4 s → 6435.9 s (+1.3%), the largest peak 6549 MB → 6566 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.3 s | 13.6 s | +2.1% | 605 MB | 605 MB | +0.0% | ~ |
| `BVHRefit` | 7.0 s | 7.1 s | +1.4% | 507 MB | 507 MB | -0.0% | ~ |
| `BVHUnion` | 26.0 s | 26.1 s | +0.4% | 1035 MB | 1035 MB | +0.0% | ~ |
| `BVHUnionMixed` | 11.2 s | 11.5 s | +2.6% | 630 MB | 630 MB | +0.0% | ~ |
| `ClosestPrimitive` | 128.2 s | 136.3 s | +6.3% | 1352 MB | 1352 MB | +0.0% | ~ |
| `ClosestSurface` | 205.5 s | 203.0 s | -1.2% | 1843 MB | 1844 MB | +0.0% | ~ |
| `Domain` | 61.4 s | 60.0 s | -2.3% | 998 MB | 998 MB | +0.0% | ~ |
| `DomainObject` | 57.0 s | 57.5 s | +0.9% | 951 MB | 952 MB | +0.0% | ~ |
| `Fused` | 55.4 s | 56.1 s | +1.3% | 1163 MB | 1163 MB | -0.0% | ~ |
| `LaunchTuning` | 585.7 s | 583.7 s | -0.3% | 3859 MB | 3876 MB | +0.4% | ~ |
| `Mesh` | 21.9 s | 21.3 s | -2.4% | 810 MB | 810 MB | +0.0% | ~ |
| `MeshCSG` | 460.2 s | 451.4 s | -1.9% | 6285 MB | 6303 MB | +0.3% | ~ |
| `MeshClosestPoint` | 19.4 s | 19.7 s | +1.9% | 762 MB | 762 MB | +0.0% | ~ |
| `MeshReconcile` | 10.3 s | 10.4 s | +0.8% | 513 MB | 513 MB | +0.0% | ~ |
| `MeshTape` | 817.3 s | 820.8 s | +0.4% | 6549 MB | 6566 MB | +0.3% | ~ |
| `PointCloud` | 12.9 s | 12.7 s | -1.8% | 571 MB | 572 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 7.1 s | 7.1 s | -0.2% | 508 MB | 508 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 6.7 s | 6.7 s | +0.9% | 508 MB | 508 MB | +0.0% | ~ |
| `Ray` | 227.8 s | 221.5 s | -2.8% | 3756 MB | 3769 MB | +0.4% | ~ |
| `RayCSG` | 261.4 s | 253.5 s | -3.0% | 4139 MB | 4155 MB | +0.4% | ~ |
| `RayCrossing` | 475.5 s | 472.5 s | -0.6% | 4102 MB | 4116 MB | +0.4% | ~ |
| `RayParticles` | 905.7 s | 941.5 s | +4.0% | 5781 MB | 5806 MB | +0.4% | ~ |
| `RayShell` | 544.6 s | 545.3 s | +0.1% | 5488 MB | 5501 MB | +0.2% | ~ |
| `Regions` | 381.1 s | 389.6 s | +2.2% | 4128 MB | 4138 MB | +0.3% | ~ |
| `Render` | 269.9 s | 275.2 s | +1.9% | 6047 MB | 6083 MB | +0.6% | ~ |
| `Shapes` | 26.7 s | 27.5 s | +2.9% | 592 MB | 593 MB | +0.0% | ~ |
| `SphereCloudBVH` | 8.7 s | 9.6 s | +10.4% | 508 MB | 508 MB | +0.0% | ~ |
| `SphereCloudRays` | 184.9 s | 192.9 s | +4.3% | 4153 MB | 4169 MB | +0.4% | ~ |
| `SphereContacts` | 8.0 s | 8.5 s | +6.5% | 507 MB | 507 MB | +0.0% | ~ |
| `SurfaceParts` | 438.8 s | 476.0 s | +8.5% | 4397 MB | 4416 MB | +0.4% | ~ |
| `Tape` | 68.1 s | 70.6 s | +3.7% | 985 MB | 985 MB | +0.0% | ~ |
| `TapeBVHUnion` | 46.6 s | 46.6 s | +0.0% | 925 MB | 925 MB | +0.0% | ~ |
| **all, in both** | 6354.4 s | 6435.9 s | +1.3% | 6549 MB | 6566 MB | +0.3% | ~ |

Device kernels: 218 in both, 30 changed (10 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<double, TriMeshSDF<double, 4ul, 4ul…` | 88 → 90 | 288 → 288 | 0/0 → 0/0 | **registers** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<float, TriMeshSDF<float, 4ul, 4ul> …` | 59 → 61 | 272 → 272 | 0/0 → 0/0 | **registers** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, TriMeshSDF<double, 4ul, 4ul> >, …` | 88 → 90 | 288 → 288 | 0/0 → 0/0 | **registers** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, TriMeshSDF<float, 4ul, 4ul> >, Ve…` | 59 → 61 | 272 → 272 | 0/0 → 0/0 | **registers** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, TriMeshSDF<double, 4ul, 4ul> >, …` | 88 → 90 | 288 → 288 | 0/0 → 0/0 | **registers** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, TriMeshSDF<float, 4ul, 4ul> >, Ve…` | 59 → 61 | 272 → 272 | 0/0 → 0/0 | **registers** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<double, TriMeshSDF<double, 4ul, 4ul…` | 88 → 90 | 288 → 288 | 0/0 → 0/0 | **registers** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<float, TriMeshSDF<float, 4ul, 4ul> …` | 59 → 61 | 272 → 272 | 0/0 → 0/0 | **registers** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, TriMeshSDF<double, 4ul, 4ul> >, …` | 88 → 90 | 288 → 288 | 0/0 → 0/0 | **registers** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, TriMeshSDF<float, 4ul, 4ul> >, Ve…` | 59 → 61 | 272 → 272 | 0/0 → 0/0 | **registers** |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11824 | 114/132 → 114/132 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 80 → 80 | 17328 → 17344 | 290/464 → 290/464 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 80 → 80 | 12352 → 12336 | 126/132 → 126/132 | ~ |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 128 → 128 | 12096 → 12064 | 114/132 → 114/132 | ~ |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const, …` | 40 → 40 | 12608 → 12624 | 230/368 → 230/368 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11888 → 11904 | 114/132 → 114/132 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Fused::Node<Fused::SmoothCombineOp<f…` | 168 → 168 | 3248 → 3248 | 656/744 → 656/644 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, RegionView<Tape<double, TriMeshS…` | 168 → 168 | 16992 → 16976 | 8/8 → 8/8 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, TriMeshSDF<double, 4ul, 4ul> >, uns…` | 168 → 168 | 2160 → 2160 | 992/912 → 980/884 | ~ |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11824 | 114/132 → 114/132 | ~ |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11728 → 11712 | 132/132 → 132/132 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<double, Tape<double> >, Vec3T<doubl…` | 168 → 168 | 16160 → 16144 | 48/36 → 48/36 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11840 → 11824 | 114/132 → 114/132 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 18912 → 18928 | 246/208 → 246/208 | ~ |
| `RayShell` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TapeFeature::ExactO…` | 168 → 168 | 18912 → 18896 | 236/224 → 236/224 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, false>, Vec3T<double> const, Reg…` | 168 → 168 | 16960 → 16944 | 174/196 → 174/196 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, false>, Vec3T<float> const, Regio…` | 168 → 168 | 11840 → 11824 | 114/120 → 114/120 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, TriMeshSDF<double, 4ul, 4ul> >, uns…` | 168 → 168 | 2160 → 2160 | 992/912 → 980/884 | ~ |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11888 → 11904 | 114/120 → 114/120 | ~ |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11888 → 11904 | 178/164 → 178/164 | ~ |


Flagged after 2 rounds, and judged on 10 (27 cases): LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice, LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice, LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice, MeshCSG/smooth-fused/firstHit-morton/float/hostdevice, MeshCSG/smooth-fused/frame/float/hostdevice, MeshTape/tape/firstHit/float/hostdevice, MeshTape/tape/frame/float/hostdevice, MeshTape/view/firstHit/float/hostdevice, MeshTape/mesh/firstCrossing/double/hostdevice, MeshTape/tape/firstCrossing/double/hostdevice, Ray/mesh/firstHit-random/float/hostdevice, Ray/mesh/firstHit-morton/double/hostdevice, Ray/mesh-polygons/firstHit-morton/double/hostdevice, Ray/mesh-polygons/intersects-morton/double/hostdevice, RayCSG/scene-fused/firstHit-marched-random/float/hostdevice, RayCSG/scene-tape/firstHit-morton/float/hostdevice, RayCrossing/mesh/firstHit-morton/float/hostdevice, RayCrossing/mesh/firstCrossing-morton/double/hostdevice, RayCrossing/csg-tape/firstCrossing-random/double/hostdevice, RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice, RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice, RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice, RayParticles/armadillo-shell-fused/step-morton/double/hostdevice, RayParticles/armadillo/step-random/double/hostdevice, Render/mesh/frame/float/hostdevice, SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice, SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice

These probes compiled to the same code on both sides and were not run (184 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), Fused (10), Mesh (8), MeshClosestPoint (4), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Shapes (20), SphereCloudBVH (8), SphereContacts (2), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
