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
base: 0a4fbcc
base-fingerprint: a2c485fc369aca048e00b4520e1a13072b6fe671
head: 814aa6d
head-fingerprint: 187f93ecca377fef3877a97bcb78e587e1fa56ba
cases: 120
rounds: 2
reps: 3
date: 2026-11-22
-->

## Host comparison: yellow

**Accepted:** Accept as cost of correctness fix

`814aa6d` against `0a4fbcc` (origin/main), 2026-11-22.

**VERDICT: no regression, but 5 checksums changed: check those answers (see Flagged above)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 120 cases

- base: 0a4fbcc (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 814aa6d (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 26.1 ms | 26.0 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.2 ms | 15.4 ms | +1.1% | [-0.1%, +2.2%] | +0.0% | ~ | checksum differs (13446.29576 -> 13446.22519) |
| `MeshCSG/smooth-tape/firstHit/float/host` | 317 ms | 319 ms | +0.4% | [+0.3%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 231 ms | 230 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.5 ms | 90.5 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 81.8 ms | 81.2 ms | -0.8% | [-1.6%, +0.0%] | +0.0% | ~ | tape/fused 2.83x |
| `MeshCSG/union-tape/firstHit/double/host` | 28.9 ms | 28.9 ms | +0.2% | [-2.0%, +2.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.0 ms | 17.0 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 356 ms | 354 ms | -0.3% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 246 ms | 246 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.0 ms | 96.9 ms | -0.0% | [-1.1%, +1.1%] | +0.0% | ~ | tape/fused 3.66x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.2 ms | 88.2 ms | +1.2% | [+0.4%, +1.9%] | +0.0% | ~ | tape/fused 2.79x |
| `MeshTape/mesh/firstHit/float/host` | 48.8 ms | 48.5 ms | -0.5% | [-1.4%, +0.3%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 49.1 ms | 48.8 ms | -0.6% | [-0.9%, -0.3%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.6 ms | 19.9 ms | +1.6% | [+0.6%, +2.6%] | +0.0% | ~ | checksum differs (30644.5577 -> 30644.64379) |
| `MeshTape/tape/firstHit/float/host` | 80.4 ms | 81.4 ms | +1.2% | [+0.4%, +2.0%] | +0.0% | ~ | mesh/tape 0.60x |
| `MeshTape/tape/firstCrossing/float/host` | 50.3 ms | 49.8 ms | -1.0% | [-1.7%, -0.2%] | +0.0% | ~ | mesh/tape 0.98x |
| `MeshTape/tape/frame/float/host` | 24.5 ms | 24.4 ms | -0.7% | [-4.6%, +3.4%] | +0.0% | ~ | mesh/tape 0.82x; checksum differs (30644.5577 -> 30644.64379) |
| `MeshTape/view/firstHit/float/host` | 83.4 ms | 83.8 ms | +0.4% | [-0.5%, +1.3%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/float/host` | 26.2 ms | 26.5 ms | +1.2% | [-1.3%, +3.7%] | +0.0% | ~ | mesh/view 0.75x; checksum differs (30644.5577 -> 30644.64379) |
| `MeshTape/mesh/firstHit/double/host` | 53.8 ms | 52.7 ms | -2.1% | [-2.8%, -1.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 52.5 ms | 53.1 ms | +1.1% | [-0.1%, +2.3%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.3 ms | 20.4 ms | +0.5% | [-0.2%, +1.3%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 86.0 ms | 86.3 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ | mesh/tape 0.61x |
| `MeshTape/tape/firstCrossing/double/host` | 54.8 ms | 54.4 ms | -0.7% | [-1.7%, +0.4%] | +0.0% | ~ | mesh/tape 0.98x |
| `MeshTape/tape/frame/double/host` | 26.3 ms | 26.8 ms | +2.0% | [+0.1%, +3.9%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 89.2 ms | 89.5 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ | mesh/view 0.59x |
| `MeshTape/view/frame/double/host` | 29.5 ms | 29.1 ms | -1.3% | [-2.0%, -0.7%] | +0.0% | ~ | mesh/view 0.70x |
| `Ray/mesh/firstHit/float/host` | 48.5 ms | 48.7 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ | hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.7 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.3 ms | 40.9 ms | +1.5% | [+0.3%, +2.7%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 77.6 ms | 78.2 ms | +0.7% | [+0.0%, +1.4%] | +0.0% | ~ | triangles/polygons 0.62x |
| `Ray/mesh-polygons/intersects/float/host` | 64.8 ms | 65.3 ms | +0.8% | [+0.4%, +1.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.17 ms | 5.24 ms | +1.3% | [-0.3%, +3.0%] | - | ~ | tape/fused 1.47x |
| `Ray/csg-tape/firstHit/float/host` | 7.59 ms | 7.68 ms | +1.1% | [+0.4%, +1.8%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.5 ms | 53.3 ms | +1.5% | [+0.3%, +2.7%] | +0.0% | ~ | hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled/double/host` | 50.5 ms | 51.1 ms | +1.1% | [+0.6%, +1.7%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.7 ms | 44.7 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.0 ms | 88.2 ms | +0.2% | [-1.2%, +1.6%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 72.4 ms | 72.8 ms | +0.5% | [+0.3%, +0.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.50 ms | 5.91 ms | +7.3% | [-0.6%, +15.3%] | - | ~ | tape/fused 1.34x (was 1.44x) |
| `Ray/csg-tape/firstHit/double/host` | 7.93 ms | 7.93 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.7 ms | 17.4 ms | -1.8% | [-2.7%, -0.8%] | - | ~ | marched/traced 6.89x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 120 ms | +0.1% | [-0.1%, +0.4%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.2 ms | 21.9 ms | -1.1% | [-2.8%, +0.7%] | +0.0% | ~ | marched/traced 6.58x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 149 ms | 144 ms | -2.9% | [-3.0%, -2.9%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.5 ms | 35.3 ms | -0.5% | [-0.9%, -0.2%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 18.9 ms | 19.0 ms | +0.8% | [-0.1%, +1.6%] | - | ~ | marched/traced 6.85x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 129 ms | 130 ms | +0.8% | [-3.6%, +5.2%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.7 ms | +1.4% | [+1.0%, +1.8%] | +0.0% | ~ | marched/traced 6.47x (was 7.56x) |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 177 ms | 154 ms | -13.2% | [-24.4%, +1.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.8 ms | 37.8 ms | -0.1% | [-0.9%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 48.3 ms | 48.3 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 48.1 ms | 48.3 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.57 ms | 7.58 ms | +0.1% | [-0.5%, +0.7%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.13 ms | 6.19 ms | +0.9% | [+0.2%, +1.7%] | +0.0% | ~ | hit/crossing 1.23x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 176 ms | 177 ms | +0.4% | [-0.0%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 156 ms | 156 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.4 ms | 68.5 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.2 ms | 56.2 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.1 ms | 53.1 ms | +1.8% | [+1.2%, +2.4%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.4 ms | 52.8 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.99 ms | 7.92 ms | -0.9% | [-2.7%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.31 ms | 6.30 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | hit/crossing 1.26x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 197 ms | 201 ms | +2.0% | [+1.7%, +2.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 174 ms | 178 ms | +2.5% | [+2.2%, +2.9%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 76.3 ms | 76.5 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 63.5 ms | 63.1 ms | -0.7% | [-1.0%, -0.4%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.1 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | marched/traced 4.67x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 51.5 ms | 51.6 ms | +0.3% | [-0.1%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.03 ms | 7.97 ms | -0.8% | [-1.4%, -0.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.98 ms | 1.99 ms | +0.5% | [-0.4%, +1.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.97 ms | 6.97 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.8 ms | 11.7 ms | -0.9% | [-2.4%, +0.7%] | +0.0% | ~ | marched/traced 5.10x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.3 ms | 59.8 ms | -0.7% | [-1.0%, -0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.61 ms | 8.63 ms | +0.2% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.03 ms | 2.03 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.22 ms | 7.25 ms | +0.5% | [-0.9%, +1.9%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 237 ms | 237 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 328 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 234 ms | 235 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 252 ms | 252 ms | +0.1% | [-0.0%, +0.1%] | +0.0% | ~ | marched/traced 1.45x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 365 ms | 366 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 251 ms | 251 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.31 ms | 7.29 ms | -0.2% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.5 ms | 12.6 ms | +1.3% | [+0.9%, +1.7%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 265 ms | 266 ms | +0.3% | [+0.3%, +0.4%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.5 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.1 ms | 30.3 ms | +0.6% | [+0.5%, +0.7%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 21.9 ms | 22.2 ms | +1.3% | [+0.5%, +2.2%] | +0.0% | ~ | regions/hit 1.36x |
| `Regions/particles/region/double/host` | 7.62 ms | 7.64 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.8 ms | 12.9 ms | +0.8% | [-0.0%, +1.7%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 282 ms | 282 ms | -0.0% | [-0.3%, +0.2%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.0 ms | 13.0 ms | +0.3% | [-0.1%, +0.8%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 31.9 ms | 32.4 ms | +1.5% | [+1.1%, +1.8%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.6 ms | 24.0 ms | +1.5% | [+1.1%, +1.9%] | +0.0% | ~ | regions/hit 1.35x |
| `Render/csg/frame/float/host` | 12.5 ms | 12.1 ms | -3.1% | [-6.1%, -0.0%] | +0.0% | faster |  |
| `Render/csg/frame-orthographic/float/host` | 12.5 ms | 12.5 ms | -0.0% | [-7.8%, +8.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.8 ms | 19.7 ms | -0.2% | [-2.1%, +1.7%] | +0.0% | ~ | checksum differs (30644.5577 -> 30644.64379) |
| `Render/csg/frame/double/host` | 14.0 ms | 14.0 ms | -0.3% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 13.8 ms | 13.8 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.4 ms | 21.0 ms | +2.9% | [+1.8%, +4.1%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.7 ms | 75.2 ms | +0.6% | [-0.8%, +2.1%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 294 ms | 293 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 296 ms | 295 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 85.6 ms | 85.6 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 343 ms | 340 ms | -0.9% | [-1.0%, -0.8%] | +0.0% | ~ | tape/fused 1.02x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 342 ms | 345 ms | +0.9% | [+0.5%, +1.3%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 144 ms | 143 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 89.9 ms | 89.6 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 130 ms | 130 ms | -0.2% | [-0.8%, +0.4%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 72.6 ms | 72.2 ms | -0.5% | [-1.1%, +0.2%] | +0.0% | ~ | surface/leaf 1.80x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 61.7 ms | 61.0 ms | -1.0% | [-1.6%, -0.4%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.0 ms | 34.3 ms | +0.7% | [+0.2%, +1.1%] | +0.0% | ~ | surface/hit 1.78x |
| `SurfaceParts/boxes/closestSurface/double/host` | 146 ms | 145 ms | -0.5% | [-1.0%, +0.1%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 92.7 ms | 90.2 ms | -2.7% | [-4.7%, -0.7%] | +0.0% | ~ | surface/leaf 1.61x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 143 ms | 143 ms | +0.3% | [+0.2%, +0.3%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 80.6 ms | 80.5 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 68.4 ms | 68.4 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 38.5 ms | 38.8 ms | +0.8% | [+0.4%, +1.1%] | +0.0% | ~ | surface/hit 1.76x |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 764.1 s → 751.7 s (-1.6%), the largest peak 1421 MB → 1421 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.8 s | 9.6 s | -1.7% | 558 MB | 558 MB | +0.0% | ~ |
| `BVHRefit` | 4.3 s | 4.3 s | -0.7% | 502 MB | 501 MB | -0.1% | ~ |
| `BVHUnion` | 19.4 s | 18.9 s | -2.7% | 954 MB | 954 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.7 s | 7.6 s | -0.7% | 559 MB | 558 MB | -0.1% | ~ |
| `ClosestPrimitive` | 30.2 s | 29.5 s | -2.3% | 1342 MB | 1341 MB | -0.0% | ~ |
| `ClosestSurface` | 28.7 s | 27.5 s | -4.3% | 1314 MB | 1314 MB | -0.0% | ~ |
| `ConnectedComponents` | 15.6 s | 15.3 s | -2.4% | 848 MB | 848 MB | +0.0% | ~ |
| `Domain` | 18.1 s | 17.7 s | -2.2% | 854 MB | 853 MB | -0.1% | ~ |
| `DomainObject` | 16.1 s | 15.7 s | -2.5% | 855 MB | 855 MB | +0.0% | ~ |
| `EBGrid` | 14.8 s | 14.2 s | -4.1% | 785 MB | 785 MB | +0.0% | ~ |
| `Fused` | 25.0 s | 23.9 s | -4.3% | 1230 MB | 1230 MB | +0.0% | ~ |
| `LaunchTuning` | 36.9 s | 36.1 s | -2.0% | 1324 MB | 1323 MB | -0.0% | ~ |
| `Mesh` | 15.8 s | 15.4 s | -2.2% | 789 MB | 789 MB | -0.0% | ~ |
| `MeshCSG` | 40.1 s | 39.5 s | -1.5% | 1277 MB | 1278 MB | +0.0% | ~ |
| `MeshClosestPoint` | 14.2 s | 13.9 s | -1.9% | 785 MB | 785 MB | +0.1% | ~ |
| `MeshIngest` | 7.0 s | 6.7 s | -3.8% | 569 MB | 569 MB | +0.0% | ~ |
| `MeshReconcile` | 7.0 s | 6.8 s | -2.8% | 569 MB | 569 MB | +0.0% | ~ |
| `MeshTape` | 42.3 s | 41.3 s | -2.4% | 1267 MB | 1270 MB | +0.3% | ~ |
| `PointCloud` | 7.4 s | 7.4 s | -0.6% | 561 MB | 561 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.7 s | -2.1% | 475 MB | 475 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | -0.1% | 456 MB | 456 MB | +0.0% | ~ |
| `QueryOrder` | 11.5 s | 11.1 s | -3.4% | 700 MB | 700 MB | -0.1% | ~ |
| `Ray` | 37.4 s | 36.5 s | -2.2% | 1341 MB | 1341 MB | +0.0% | ~ |
| `RayCSG` | 32.2 s | 31.3 s | -2.9% | 1211 MB | 1206 MB | -0.4% | ~ |
| `RayCrossing` | 45.3 s | 45.3 s | -0.1% | 1351 MB | 1351 MB | +0.0% | ~ |
| `RayParticles` | 48.9 s | 48.8 s | -0.1% | 1323 MB | 1323 MB | +0.0% | ~ |
| `RayShell` | 38.3 s | 37.3 s | -2.7% | 1283 MB | 1283 MB | -0.0% | ~ |
| `Regions` | 29.3 s | 29.0 s | -1.1% | 1086 MB | 1086 MB | +0.0% | ~ |
| `Render` | 36.1 s | 36.3 s | +0.6% | 1291 MB | 1291 MB | +0.0% | ~ |
| `Shapes` | 8.7 s | 8.8 s | +2.2% | 558 MB | 557 MB | -0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.8 s | -1.1% | 516 MB | 516 MB | +0.0% | ~ |
| `SphereCloudRays` | 28.1 s | 28.1 s | +0.2% | 1075 MB | 1074 MB | -0.1% | ~ |
| `SphereContacts` | 4.3 s | 4.2 s | -2.4% | 489 MB | 489 MB | +0.0% | ~ |
| `SurfaceParts` | 40.4 s | 40.5 s | +0.1% | 1421 MB | 1421 MB | +0.0% | ~ |
| `Tape` | 16.7 s | 16.7 s | +0.3% | 832 MB | 831 MB | -0.1% | ~ |
| `TapeBVHUnion` | 14.7 s | 14.7 s | -0.1% | 834 MB | 834 MB | -0.0% | ~ |
| **all, in both** | 764.1 s | 751.7 s | -1.6% | 1421 MB | 1421 MB | +0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): RayCrossing/drilled-mesh-tape/firstCrossing/double/host, RayParticles/armadillo-shell-fused/step/double/host

These probes compiled to the same code on both sides and were not run (212 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), ConnectedComponents (4), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Shapes (20), SphereCloudBVH (6), SphereContacts (2), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 0a4fbcc
base-fingerprint: a2c485fc369aca048e00b4520e1a13072b6fe671
head: 814aa6d
head-fingerprint: 187f93ecca377fef3877a97bcb78e587e1fa56ba
cases: 212
rounds: 2
reps: 3
date: 2026-11-22
-->

## Device comparison (CUDA): yellow

**Accepted:** Accept as cost of correctness fix

`814aa6d` against `0a4fbcc` (origin/main), 2026-11-22.

**VERDICT: 26 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 26 minor, 0 faster, 0 differ, 0 lost, 212 cases

- base: 0a4fbcc (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 814aa6d (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.21 ms | 8.23 ms | +0.2% | [-0.1%, +0.6%] | +0.0% | ~ | device 10.8x faster than the host (was 12.2x) |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.31 ms | 8.35 ms | +0.5% | [-0.7%, +1.6%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.21 ms | 8.19 ms | -0.2% | [-2.2%, +1.8%] | +0.0% | ~ | device 10.7x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.22 ms | 8.16 ms | -0.6% | [-1.3%, +0.0%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 703 µs | 704 µs | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | device 3.7x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.81 ms | 1.81 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.03 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.68x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 783 µs | 786 µs | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 3.3x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 730 µs | 731 µs | +0.1% | [+0.0%, +0.1%] | +0.0% | ~ | device 12.1x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 814 µs | 818 µs | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ | device 10.9x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 740 µs | 736 µs | -0.6% | [-0.9%, -0.4%] | +0.0% | ~ | device 12.1x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 720 µs | 733 µs | +1.8% | [+0.8%, +2.8%] | +0.0% | ~ | device 11.9x faster than the host; default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 161 ms | 159 ms | -1.1% | [-3.8%, +1.7%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 162 ms | 160 ms | -0.7% | [-1.7%, +0.2%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 161 ms | 161 ms | +0.2% | [-0.4%, +0.9%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.98x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 161 ms | 160 ms | -0.7% | [-2.9%, +1.4%] | +0.0% | ~ | device 0.6x faster than the host; default/this 0.99x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | -0.3% | [-0.7%, +0.2%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.81 ms | 2.79 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ | device 0.9x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.92 ms | 1.91 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.73x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.59 ms | +2.2% | [+2.0%, +2.4%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.22 ms | 7.24 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.30 ms | 7.28 ms | -0.3% | [-0.9%, +0.3%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.23 ms | 7.25 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.33 ms | 7.28 ms | -0.6% | [-1.0%, -0.2%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.01 ms | 2.04 ms | +1.4% | [+0.8%, +2.0%] | - | ~ | device 5.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.39 ms | 3.40 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.14 ms | 6.16 ms | +0.5% | [-0.2%, +1.1%] | - | ~ | device 4.7x faster than the host; checksum differs (215185.0491 -> 215184.6871) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.5 ms | 62.9 ms | -1.0% | [-2.3%, +0.4%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 92.7 ms | 93.6 ms | +0.9% | [-0.6%, +2.5%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 238 ms | 241 ms | +1.2% | [-0.3%, +3.1%] | - | ~ | device 3.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.8 ms | 24.6 ms | -0.8% | [-1.3%, -0.2%] | - | ~ | device 2.0x faster than the host; tape/fused 2.56x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.6 ms | 26.8 ms | +1.0% | [+0.0%, +2.0%] | - | ~ | device 2.2x faster than the host; tape/fused 3.49x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 66.5 ms | 65.5 ms | -1.6% | [-2.6%, -0.6%] | - | ~ | device 4.3x faster than the host (was 3.8x); tape/fused 3.68x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.4 ms | 20.8 ms | -3.0% | [-9.2%, +3.6%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 38.6 ms | 39.1 ms | +1.1% | [-2.2%, +4.5%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 41.6 ms | 42.4 ms | +1.9% | [-0.5%, +4.8%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 543 ms | 533 ms | -1.9% | [-7.2%, +3.8%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 844 ms | 837 ms | -0.8% | [-2.5%, +0.9%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.94 s | 1.96 s | +1.1% | [-0.4%, +2.5%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 190 ms | 201 ms | +6.2% | [+3.0%, +11.7%] | - | minor | device 0.2x faster than the host (was 0.3x); tape/fused 2.65x (was 2.86x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 311 ms | 307 ms | -1.4% | [-3.8%, +1.1%] | - | ~ | device 0.2x faster than the host; tape/fused 2.73x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 533 ms | 547 ms | +2.6% | [-2.4%, +7.6%] | - | ~ | device 0.5x faster than the host (was 0.5x); tape/fused 3.58x |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.29 ms | 1.31 ms | +1.6% | [+0.8%, +2.5%] | - | ~ | device 15.8x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.29 ms | 1.32 ms | +1.8% | [+1.7%, +1.9%] | - | ~ | device 15.7x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.13 ms | 2.15 ms | +0.7% | [+0.2%, +1.2%] | - | ~ | device 7.9x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.49 ms | 3.51 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 10.2x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.75 ms | 2.77 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 7.8x faster than the host; mesh/tape 0.48x |
| `MeshTape/tape/frame/float/hostdevice` | 4.41 ms | 4.42 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 4.9x faster than the host; mesh/tape 0.48x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.73 ms | 3.73 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 9.8x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.62 ms | 5.64 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 4.2x faster than the host; mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 28.5 ms | 30.2 ms | +6.1% | [+5.7%, +6.3%] | - | minor | device 0.7x faster than the host (was 0.8x) |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 28.5 ms | 30.2 ms | +6.2% | [+5.8%, +6.7%] | - | minor | device 0.7x faster than the host (was 0.7x) |
| `MeshTape/mesh/frame/double/hostdevice` | 24.5 ms | 25.8 ms | +5.3% | [+4.5%, +5.7%] | - | minor | device 0.7x faster than the host (was 0.7x) |
| `MeshTape/tape/firstHit/double/hostdevice` | 41.0 ms | 43.4 ms | +5.9% | [+5.6%, +6.2%] | - | minor | device 0.8x faster than the host (was 0.9x); mesh/tape 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 28.8 ms | 30.5 ms | +6.0% | [+5.7%, +6.3%] | - | minor | device 0.7x faster than the host (was 0.8x); mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 31.5 ms | 32.9 ms | +4.4% | [+3.9%, +5.2%] | - | minor | device 0.7x faster than the host (was 0.7x); mesh/tape 0.78x |
| `MeshTape/view/firstHit/double/hostdevice` | 41.0 ms | 43.4 ms | +5.8% | [+5.5%, +6.1%] | - | minor | device 0.9x faster than the host (was 0.9x); mesh/view 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 34.3 ms | 35.9 ms | +4.7% | [+3.9%, +5.6%] | - | minor | device 0.7x faster than the host (was 0.7x); mesh/view 0.72x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 691 µs | 702 µs | +1.5% | [+1.1%, +1.9%] | - | ~ | device 17.3x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.32 ms | +1.9% | [+0.8%, +3.1%] | - | ~ | device 15.2x faster than the host; hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 646 µs | 660 µs | +2.2% | [+0.3%, +4.2%] | - | ~ | device 17.2x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.24 ms | +1.3% | [+1.2%, +1.5%] | - | ~ | device 15.2x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 609 µs | 624 µs | +2.4% | [+1.7%, +3.4%] | - | ~ | device 15.8x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.11 ms | 1.16 ms | +4.1% | [+3.5%, +4.5%] | - | minor | device 14.5x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.20 ms | 1.19 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 11.8x faster than the host; triangles/polygons 0.59x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.48 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 9.6x faster than the host; triangles/polygons 0.53x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.08 ms | 1.08 ms | +0.3% | [-0.7%, +1.3%] | - | ~ | device 11.6x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 591 µs | 594 µs | +0.6% | [+0.3%, +0.8%] | - | ~ | device 2.5x faster than the host; tape/fused 1.19x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 690 µs | 658 µs | -4.6% | [-9.2%, +0.5%] | - | ~ | device 2.8x faster than the host (was 2.6x); tape/fused 1.29x (was 1.23x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 703 µs | 705 µs | +0.2% | [-0.1%, +0.6%] | - | ~ | device 3.6x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 845 µs | 847 µs | +0.3% | [-0.8%, +1.4%] | - | ~ | device 3.5x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.7 ms | 18.7 ms | +5.7% | [+5.3%, +6.1%] | - | minor | device 0.7x faster than the host (was 0.7x); hand/api 0.97x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 28.5 ms | 30.2 ms | +5.8% | [+5.4%, +6.3%] | - | minor | device 0.7x faster than the host (was 0.7x); hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 18.0 ms | +6.0% | [+5.0%, +7.1%] | - | minor | device 0.6x faster than the host (was 0.7x) |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 29.2 ms | +6.2% | [+5.9%, +6.6%] | - | minor | device 0.7x faster than the host (was 0.7x) |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 15.8 ms | +5.9% | [+5.4%, +6.5%] | - | minor | device 0.6x faster than the host (was 0.7x) |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 26.0 ms | +5.9% | [+5.6%, +6.4%] | - | minor | device 0.7x faster than the host (was 0.7x) |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 17.1 ms | 17.2 ms | +0.6% | [-0.8%, +2.0%] | - | ~ | device 0.8x faster than the host; triangles/polygons 1.09x (was 1.04x) |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.5 ms | 25.5 ms | +0.3% | [+0.3%, +0.4%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.18x (was 1.12x) |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.3 ms | 14.9 ms | -2.2% | [-3.7%, -0.7%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.1 ms | 22.9 ms | -0.6% | [-0.8%, -0.5%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 1.2x faster than the host (was 1.3x); tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.36 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | -0.2% | [-0.2%, -0.2%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.53 ms | 2.53 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 2.5x faster than the host (was 3.0x); marched/traced 3.57x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.98 ms | 2.98 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.3x faster than the host (was 2.7x); marched/traced 4.64x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.02 ms | 9.01 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 5.0x faster than the host (was 4.3x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.8 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.7x faster than the host (was 3.2x) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.16 ms | 2.16 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.9x faster than the host; marched/traced 13.51x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.43 ms | 5.42 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 2.8x faster than the host; marched/traced 10.24x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.1 ms | 29.1 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.5 ms | 55.5 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.24 ms | 5.21 ms | -0.5% | [-0.8%, -0.2%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.78 ms | 8.80 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 3.1x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.19 ms | 6.20 ms | +0.1% | [-0.6%, +0.9%] | - | ~ | device 1.3x faster than the host (was 1.2x); marched/traced 26.10x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | +0.4% | [+0.0%, +0.8%] | - | ~ | device 0.8x faster than the host (was 0.8x); marched/traced 23.99x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 160 ms | 162 ms | +1.3% | [-0.0%, +2.6%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 254 ms | 254 ms | -0.1% | [-1.3%, +1.2%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.88 ms | 4.88 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.8x faster than the host; marched/traced 22.87x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.2 ms | 12.3 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 1.3x faster than the host; marched/traced 17.72x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 112 ms | 112 ms | -0.8% | [-3.6%, +2.1%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 213 ms | 217 ms | +2.0% | [+1.0%, +2.9%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.5 ms | +0.6% | [-0.1%, +1.2%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.7 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 687 µs | 702 µs | +2.3% | [+1.6%, +3.3%] | - | ~ | device 17.3x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.30 ms | 1.32 ms | +1.1% | [+0.2%, +2.1%] | - | ~ | device 15.2x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 686 µs | 705 µs | +2.8% | [+2.0%, +3.4%] | - | ~ | device 17.2x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.29 ms | 1.31 ms | +1.6% | [+1.2%, +2.0%] | - | ~ | device 15.2x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 709 µs | 708 µs | -0.1% | [-0.4%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 842 µs | 841 µs | -0.1% | [-1.3%, +1.0%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 623 µs | 621 µs | -0.3% | [-0.5%, -0.0%] | - | ~ | device 3.2x faster than the host; hit/crossing 1.14x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 644 µs | 642 µs | -0.3% | [-0.9%, +0.2%] | - | ~ | device 3.6x faster than the host; hit/crossing 1.31x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.75 ms | 4.78 ms | +0.7% | [+0.7%, +0.8%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.3 ms | 10.3 ms | +0.6% | [+0.5%, +0.6%] | - | ~ | device 7.5x faster than the host (was 6.9x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.21 ms | +0.7% | [+0.5%, +0.9%] | - | ~ | device 10.0x faster than the host; hit/crossing 1.14x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.07 ms | 9.13 ms | +0.6% | [+0.5%, +0.7%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.5 ms | 55.9 ms | -1.1% | [-3.3%, +1.3%] | - | ~ | device 3.2x faster than the host (was 3.1x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.1 ms | 81.0 ms | -0.2% | [-2.0%, +1.7%] | - | ~ | device 2.3x faster than the host (was 2.6x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 49.4 ms | 48.9 ms | -1.0% | [-3.1%, +1.1%] | - | ~ | device 2.9x faster than the host; hit/crossing 1.14x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.9 ms | 64.0 ms | +0.1% | [-0.8%, +0.9%] | - | ~ | device 2.4x faster than the host (was 3.0x); hit/crossing 1.27x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.7 ms | 18.7 ms | +5.9% | [+5.5%, +6.8%] | - | minor | device 0.7x faster than the host (was 0.7x) |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 28.5 ms | 30.2 ms | +5.9% | [+5.6%, +6.4%] | - | minor | device 0.7x faster than the host (was 0.7x) |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.6 ms | 18.8 ms | +6.4% | [+5.7%, +6.8%] | - | minor | device 0.7x faster than the host (was 0.7x); hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 28.5 ms | 30.2 ms | +5.8% | [+5.5%, +6.3%] | - | minor | device 0.7x faster than the host (was 0.7x); hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.51 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.11 ms | 1.11 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 1.8x faster than the host; hit/crossing 1.26x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.15 ms | 1.16 ms | +0.8% | [-0.1%, +1.7%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.15x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.7 ms | 68.7 ms | +6.2% | [+6.1%, +6.9%] | - | minor | device 0.7x faster than the host (was 0.7x) |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 122 ms | 130 ms | +6.7% | [+5.7%, +7.2%] | - | minor | device 0.6x faster than the host (was 0.6x) |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.5 ms | 62.2 ms | +6.4% | [+5.9%, +6.7%] | - | minor | device 0.7x faster than the host (was 0.7x); hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 110 ms | 118 ms | +6.9% | [+5.0%, +7.9%] | - | minor | device 0.5x faster than the host (was 0.6x); hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 467 ms | 464 ms | -0.6% | [-10.2%, +10.4%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 707 ms | 703 ms | -0.6% | [-2.1%, +1.0%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 383 ms | 385 ms | +0.7% | [-6.2%, +6.0%] | - | ~ | device 0.4x faster than the host; hit/crossing 1.20x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 525 ms | 536 ms | +2.0% | [+0.6%, +3.5%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.31x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.53 ms | 3.60 ms | +1.8% | [-0.9%, +4.6%] | - | ~ | device 1.2x faster than the host; marched/traced 3.32x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.77 ms | 4.78 ms | +0.2% | [-1.2%, +1.7%] | - | ~ | device 0.9x faster than the host; marched/traced 5.55x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 11.9 ms | 12.0 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.6 ms | 26.5 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.26 ms | 1.23 ms | -2.3% | [-5.4%, +0.8%] | - | ~ | device 2.5x faster than the host (was 2.4x) |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.84 ms | -0.2% | [-1.0%, +0.7%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 96.7 µs | 98.4 µs | +1.7% | [+0.0%, +3.0%] | - | ~ | device 14.7x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 154 µs | 154 µs | -0.1% | [-0.5%, +0.3%] | - | ~ | device 10.5x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 795 µs | 798 µs | +0.4% | [+0.3%, +0.6%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 933 µs | 937 µs | +0.4% | [+0.1%, +0.8%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 36.4 ms | 34.5 ms | -5.3% | [-12.8%, +3.2%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 3.50x (was 3.32x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.7 ms | 36.9 ms | +0.4% | [-0.9%, +1.9%] | - | ~ | device 0.1x faster than the host; marched/traced 6.31x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 121 ms | 121 ms | -0.0% | [-3.0%, +3.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 234 ms | 233 ms | -0.7% | [-2.4%, +1.0%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.7 ms | 16.6 ms | -0.4% | [-8.0%, +8.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 22.0 ms | 21.7 ms | -1.5% | [-2.1%, -0.9%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.11 ms | 1.17 ms | +5.5% | [-0.6%, +11.9%] | - | ~ | device 1.3x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.45 ms | 2.52 ms | +2.7% | [+1.9%, +3.6%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.96 ms | 5.99 ms | +0.4% | [-0.0%, +0.9%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.71 ms | 6.71 ms | +0.0% | [-0.4%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.0 ms | 23.2 ms | +0.8% | [+0.0%, +1.6%] | - | ~ | device 2.2x faster than the host; marched/traced 2.69x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.6 ms | 28.3 ms | -1.0% | [-2.1%, +0.1%] | - | ~ | device 2.0x faster than the host; marched/traced 3.67x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.2 ms | 62.5 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 104 ms | 104 ms | -0.1% | [-2.0%, +1.8%] | - | ~ | device 2.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 14.0 ms | 13.8 ms | -0.8% | [-1.7%, +0.1%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.4 ms | 17.4 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 313 ms | 307 ms | -1.8% | [-7.7%, +4.2%] | - | ~ | device 0.2x faster than the host; marched/traced 1.80x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 346 ms | 349 ms | +1.0% | [-3.2%, +5.3%] | - | ~ | device 0.2x faster than the host; marched/traced 2.82x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 565 ms | 555 ms | -1.8% | [-3.2%, -0.3%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 987 ms | 984 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 0.4x faster than the host (was 0.2x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 189 ms | 193 ms | +2.1% | [-0.1%, +3.5%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 238 ms | 235 ms | -1.2% | [-6.1%, +3.9%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.75 ms | 2.76 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 2.7x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.76 ms | 3.76 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 3.5x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 90.0 ms | 89.3 ms | -0.8% | [-1.6%, -0.1%] | - | ~ | device 3.7x faster than the host (was 3.4x); one-pass/each 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.70 ms | 3.70 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.6x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.71 ms | 6.71 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.7x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.54 ms | 5.54 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.2x faster than the host; regions/hit 1.21x |
| `Regions/particles/region/double/hostdevice` | 3.36 ms | 3.35 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.84 ms | 5.89 ms | +0.9% | [+0.5%, +1.4%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 135 ms | 135 ms | +0.4% | [-0.2%, +1.1%] | - | ~ | device 2.3x faster than the host (was 2.5x); one-pass/each 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.88 ms | 5.89 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 2.3x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.7 ms | -0.4% | [-0.9%, +0.0%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 12.9 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 1.3x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.37 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.36 ms | 2.36 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.13 ms | 2.14 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 7.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.63 ms | 5.61 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.62 ms | 5.61 ms | -0.3% | [-0.6%, +0.0%] | - | ~ | device 1.4x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 24.5 ms | 25.8 ms | +5.7% | [+4.4%, +6.3%] | - | minor | device 0.7x faster than the host (was 0.7x) |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.01 ms | +0.2% | [-0.2%, +0.7%] | +0.0% | ~ | device 7.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.77 ms | 4.76 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 5.1x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.0 ms | 24.0 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.27x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.5 ms | 30.4 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 3.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.4 ms | 44.2 ms | -0.4% | [-0.7%, -0.2%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.6 ms | 45.6 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 122 ms | 122 ms | +0.2% | [-1.2%, +1.6%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 126 ms | 127 ms | +0.6% | [-3.8%, +5.1%] | +0.0% | ~ | device 0.8x faster than the host (was 0.8x); tape/fused 1.04x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 129 ms | 127 ms | -1.7% | [-3.2%, +1.0%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 133 ms | 132 ms | -0.9% | [-2.7%, +2.1%] | +0.0% | ~ | device 0.8x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.32 ms | 9.32 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 12.6x faster than the host (was 11.4x) |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.93 ms | 5.93 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 9.9x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 95.5 ms | 96.8 ms | +1.4% | [-0.1%, +2.9%] | - | ~ | device 3.5x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.3 ms | 63.4 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.5x faster than the host (was 3.2x); surface/leaf 1.53x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.2 ms | 29.3 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 6.2x faster than the host (was 5.0x) |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 10.0 ms | 10.0 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 7.1x faster than the host (was 6.8x); surface/hit 2.92x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.0 ms | 59.0 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.8x faster than the host (was 2.1x) |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | -0.1% | [-0.1%, -0.1%] | - | ~ | device 1.6x faster than the host; surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 788 ms | 786 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 529 ms | 524 ms | -1.0% | [-1.5%, -0.4%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.50x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 309 ms | 312 ms | +0.8% | [-0.6%, +2.1%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 121 ms | 126 ms | +4.4% | [+3.3%, +6.4%] | - | minor | device 0.6x faster than the host (was 0.6x); surface/hit 2.47x (was 2.56x) |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 3 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4912.3 s → 4908.3 s (-0.1%), the largest peak 6570 MB → 6574 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.1 s | 12.1 s | +0.1% | 613 MB | 614 MB | +0.1% | ~ |
| `BVHRefit` | 6.4 s | 6.4 s | +0.0% | 516 MB | 517 MB | +0.0% | ~ |
| `BVHUnion` | 22.8 s | 22.8 s | -0.1% | 1036 MB | 1036 MB | -0.0% | ~ |
| `BVHUnionMixed` | 10.1 s | 10.1 s | -0.0% | 632 MB | 631 MB | -0.1% | ~ |
| `ClosestPrimitive` | 106.6 s | 106.8 s | +0.2% | 1365 MB | 1365 MB | -0.0% | ~ |
| `ClosestSurface` | 149.6 s | 149.9 s | +0.2% | 1842 MB | 1844 MB | +0.1% | ~ |
| `ConnectedComponents` | 18.2 s | 18.3 s | +0.3% | 946 MB | 946 MB | +0.1% | ~ |
| `Domain` | 52.3 s | 52.4 s | +0.2% | 1001 MB | 1002 MB | +0.1% | ~ |
| `DomainObject` | 47.7 s | 47.9 s | +0.3% | 954 MB | 954 MB | +0.1% | ~ |
| `EBGrid` | 18.0 s | 18.0 s | -0.2% | 761 MB | 761 MB | -0.0% | ~ |
| `Fused` | 45.0 s | 45.1 s | +0.0% | 1173 MB | 1172 MB | -0.0% | ~ |
| `LaunchTuning` | 435.5 s | 436.6 s | +0.3% | 3876 MB | 3874 MB | -0.0% | ~ |
| `Mesh` | 18.7 s | 18.8 s | +0.5% | 819 MB | 819 MB | +0.0% | ~ |
| `MeshCSG` | 343.2 s | 343.5 s | +0.1% | 6308 MB | 6308 MB | -0.0% | ~ |
| `MeshClosestPoint` | 17.1 s | 17.0 s | -0.4% | 771 MB | 771 MB | -0.0% | ~ |
| `MeshIngest` | 8.8 s | 8.7 s | -0.4% | 544 MB | 544 MB | -0.1% | ~ |
| `MeshReconcile` | 8.8 s | 8.7 s | -0.6% | 521 MB | 521 MB | -0.0% | ~ |
| `MeshTape` | 654.2 s | 654.6 s | +0.1% | 6570 MB | 6574 MB | +0.1% | ~ |
| `PointCloud` | 11.2 s | 11.3 s | +0.5% | 572 MB | 572 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 6.1 s | 6.2 s | +0.7% | 517 MB | 517 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 5.4 s | 5.5 s | +0.0% | 517 MB | 517 MB | +0.0% | ~ |
| `QueryOrder` | 13.7 s | 13.7 s | -0.1% | 702 MB | 702 MB | -0.0% | ~ |
| `Ray` | 157.2 s | 157.2 s | +0.0% | 3769 MB | 3770 MB | +0.0% | ~ |
| `RayCSG` | 179.1 s | 179.1 s | +0.0% | 4155 MB | 4155 MB | +0.0% | ~ |
| `RayCrossing` | 342.0 s | 341.7 s | -0.1% | 4115 MB | 4118 MB | +0.1% | ~ |
| `RayParticles` | 699.7 s | 692.9 s | -1.0% | 5806 MB | 5799 MB | -0.1% | ~ |
| `RayShell` | 376.4 s | 376.7 s | +0.1% | 5500 MB | 5506 MB | +0.1% | ~ |
| `Regions` | 289.7 s | 290.4 s | +0.3% | 4138 MB | 4139 MB | +0.0% | ~ |
| `Render` | 215.0 s | 215.0 s | -0.0% | 6079 MB | 6083 MB | +0.1% | ~ |
| `Shapes` | 21.7 s | 21.7 s | +0.3% | 594 MB | 594 MB | -0.1% | ~ |
| `SphereCloudBVH` | 7.6 s | 7.6 s | +0.0% | 517 MB | 517 MB | -0.0% | ~ |
| `SphereCloudRays` | 150.1 s | 150.2 s | +0.1% | 4169 MB | 4175 MB | +0.1% | ~ |
| `SphereContacts` | 6.7 s | 6.6 s | -1.0% | 517 MB | 517 MB | +0.0% | ~ |
| `SurfaceParts` | 368.4 s | 367.6 s | -0.2% | 4416 MB | 4416 MB | +0.0% | ~ |
| `Tape` | 51.7 s | 52.2 s | +0.9% | 986 MB | 986 MB | -0.0% | ~ |
| `TapeBVHUnion` | 35.6 s | 35.1 s | -1.4% | 927 MB | 927 MB | -0.0% | ~ |
| **all, in both** | 4912.3 s | 4908.3 s | -0.1% | 6570 MB | 6574 MB | +0.1% | ~ |

Device kernels: 226 in both, 7 changed (4 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, TriMeshSDF<float, 4ul, 4ul> >, unsig…` | 168 → 168 | 1392 → 1392 | 108/116 → 112/120 | **spills** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<HandRolled<float>, Vec3T<float> const, float> >(A…` | 55 → 56 | 272 → 272 | 0/0 → 0/0 | **registers** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<Intersects<float, TriMeshSDF<float, 4ul, 4ul> >, …` | 56 → 58 | 272 → 272 | 0/0 → 0/0 | **registers** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, TriMeshSDF<float, 4ul, 4ul> >, unsig…` | 168 → 168 | 1392 → 1392 | 108/116 → 112/120 | **spills** |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 128 → 128 | 17136 → 17120 | 186/220 → 186/220 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 18928 → 18912 | 246/208 → 246/208 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 18720 → 18736 | 246/208 → 246/208 | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (37 cases): MeshCSG/smooth-tape/frame/float/hostdevice, MeshCSG/union-tape/frame/double/hostdevice, MeshCSG/smooth-fused/firstHit-morton/double/hostdevice, MeshTape/mesh/firstHit/double/hostdevice, MeshTape/mesh/firstCrossing/double/hostdevice, MeshTape/mesh/frame/double/hostdevice, MeshTape/tape/firstHit/double/hostdevice, MeshTape/tape/firstCrossing/double/hostdevice, MeshTape/tape/frame/double/hostdevice, MeshTape/view/firstHit/double/hostdevice, MeshTape/view/frame/double/hostdevice, Ray/mesh/intersects-morton/float/hostdevice, Ray/mesh/intersects-random/float/hostdevice, Ray/mesh/firstHit-morton/double/hostdevice, Ray/mesh/firstHit-random/double/hostdevice, Ray/mesh/firstHit-handrolled-morton/double/hostdevice, Ray/mesh/firstHit-handrolled-random/double/hostdevice, Ray/mesh/intersects-morton/double/hostdevice, Ray/mesh/intersects-random/double/hostdevice, RayCrossing/mesh/firstHit-morton/float/hostdevice, RayCrossing/mesh/firstCrossing-morton/float/hostdevice, RayCrossing/mesh/firstHit-morton/double/hostdevice, RayCrossing/mesh/firstHit-random/double/hostdevice, RayCrossing/mesh/firstCrossing-morton/double/hostdevice, RayCrossing/mesh/firstCrossing-random/double/hostdevice, RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice, RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice, RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice, RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice, RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice, RayParticles/armadillo/step-morton/float/hostdevice, RayShell/armadillo-fused/firstHit-morton/double/hostdevice, Render/mesh/frame/double/hostdevice, SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice, SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice, SurfaceParts/mesh-groups/firstSurface/double/hostdevice, SurfaceParts/mesh-groups/firstHit/double/hostdevice

These probes compiled to the same code on both sides and were not run (202 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (8), MeshClosestPoint (4), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), QueryOrder (6), Shapes (20), SphereCloudBVH (8), SphereContacts (2), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
