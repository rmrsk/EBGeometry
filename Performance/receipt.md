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
verdict: green
base: 9a82671
base-fingerprint: 5b762623a2468955cf1da5e76311c603f57f1727
head: 7c2679b
head-fingerprint: 444341ad48073521e98b9c88662429cb15c2889c
cases: 182
rounds: 2
reps: 3
date: 2026-11-17
-->

## Host comparison: green

**Accepted:** -

`7c2679b` against `9a82671` (origin/main), 2026-11-17.

**VERDICT: no regression (0 major, 0 minor across 182 cases)**

Performance comparison: 0 major, 0 minor, 13 faster, 0 differ, 0 lost, 182 cases

- base: 9a82671 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 7c2679b (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 24.1 ms | 24.4 ms | +1.2% | [-33.3%, +52.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 51.8 ms | 53.6 ms | +3.5% | [-34.5%, +61.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 109 ms | 104 ms | -5.1% | [-50.1%, +85.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 26.9 ms | 27.3 ms | +1.6% | [-41.3%, +75.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 57.9 ms | 57.7 ms | -0.3% | [-39.2%, +63.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 197 ms | 94.0 ms | -52.2% | [-73.9%, +30.9%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | 139 ms | 140 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | 76.7 ms | 75.4 ms | -1.7% | [-2.5%, -0.8%] | +0.0% | ~ | surface/leaf 1.85x |
| `ClosestSurface/union/closestSurface/float/host` | 146 ms | 146 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/float/host` | 76.6 ms | 76.9 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ | surface/leaf 1.90x |
| `ClosestSurface/cloud/closestSurface/double/host` | 157 ms | 157 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | 80.8 ms | 81.4 ms | +0.7% | [+0.4%, +1.0%] | +0.0% | ~ | surface/leaf 1.93x |
| `ClosestSurface/union/closestSurface/double/host` | 154 ms | 153 ms | -0.9% | [-1.4%, -0.5%] | +0.0% | ~ |  |
| `ClosestSurface/union/closestLeaf/double/host` | 80.9 ms | 81.0 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | surface/leaf 1.89x |
| `Domain/particles/labels/float/host` | 13.6 ms | 13.4 ms | -1.0% | [-1.6%, -0.5%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.79 ms | 6.80 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.36 ms | 1.36 ms | -0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `Domain/particles/labels/double/host` | 14.0 ms | 14.1 ms | +0.6% | [-0.1%, +1.3%] | +0.0% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.15 ms | 7.15 ms | +0.1% | [-0.6%, +0.7%] | +0.0% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.38 ms | 1.35 ms | -1.5% | [-2.6%, -0.4%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labels/float/host` | 39.5 ms | 39.4 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 22.1 ms | 21.4 ms | -2.8% | [-7.4%, +2.1%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 567 µs | 564 µs | -0.5% | [-2.1%, +1.2%] | +0.0% | ~ | own/whole 69.89x |
| `DomainObject/partsOwn/labelAt/float/host` | 303 µs | 304 µs | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ | own/whole 70.64x (was 72.84x) |
| `DomainObject/partsWhole/labels/double/host` | 39.4 ms | 39.5 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 22.5 ms | 22.6 ms | +0.4% | [-0.1%, +0.8%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 587 µs | 627 µs | +6.8% | [-0.5%, +14.1%] | +0.0% | ~ | own/whole 63.02x (was 67.17x) |
| `DomainObject/partsOwn/labelAt/double/host` | 315 µs | 320 µs | +1.5% | [+0.6%, +2.4%] | +0.0% | ~ | own/whole 70.70x |
| `Fused/csg/signedDistance/float/host` | 4.51 ms | 4.50 ms | -0.2% | [-1.4%, +1.0%] | - | ~ | tape/fused 2.85x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | tape/fused 2.04x |
| `Fused/chain16/signedDistance/float/host` | 967 µs | 980 µs | +1.4% | [+0.1%, +2.6%] | - | ~ | tape/fused 16.53x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.1 ms | 46.1 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.4 ms | 46.3 ms | -0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.64 ms | 4.62 ms | -0.4% | [-2.7%, +2.0%] | - | ~ | tape/fused 2.86x |
| `Fused/smooth/signedDistance/double/host` | 12.7 ms | 12.8 ms | +0.9% | [+0.2%, +1.5%] | - | ~ | tape/fused 2.07x |
| `Fused/chain16/signedDistance/double/host` | 3.89 ms | 3.87 ms | -0.5% | [-1.3%, +0.3%] | - | ~ | tape/fused 4.16x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.7 ms | 52.7 ms | -0.1% | [-0.7%, +0.6%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.9 ms | 53.1 ms | +0.4% | [-0.7%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.8 ms | 26.0 ms | -3.0% | [-3.2%, -2.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.0 ms | 15.3 ms | -43.3% | [-44.5%, -42.1%] | +0.0% | faster |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 318 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 238 ms | 230 ms | -3.2% | [-3.4%, -3.1%] | +0.0% | faster |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.8 ms | 90.4 ms | -0.4% | [-0.5%, -0.3%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 102 ms | 81.2 ms | -20.1% | [-20.4%, -19.9%] | +0.0% | faster | tape/fused 2.83x (was 2.34x) |
| `MeshCSG/union-tape/firstHit/double/host` | 29.5 ms | 28.5 ms | -3.3% | [-3.6%, -2.9%] | +0.0% | faster |  |
| `MeshCSG/union-tape/frame/double/host` | 30.0 ms | 17.1 ms | -43.0% | [-43.5%, -42.5%] | +0.0% | faster |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 357 ms | +0.0% | [-1.1%, +1.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 254 ms | 246 ms | -3.1% | [-3.4%, -2.8%] | +0.0% | faster |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.6 ms | 96.9 ms | -0.7% | [-1.2%, -0.2%] | +0.0% | ~ | tape/fused 3.68x |
| `MeshCSG/smooth-fused/frame/double/host` | 109 ms | 86.9 ms | -20.1% | [-20.4%, -19.8%] | +0.0% | faster | tape/fused 2.83x (was 2.33x) |
| `MeshTape/mesh/firstHit/float/host` | - | - | - | - | - | new |  |
| `MeshTape/mesh/firstCrossing/float/host` | - | - | - | - | - | new |  |
| `MeshTape/mesh/frame/float/host` | - | - | - | - | - | new |  |
| `MeshTape/tape/firstHit/float/host` | - | - | - | - | - | new | tape/mesh 0.60x |
| `MeshTape/tape/firstCrossing/float/host` | - | - | - | - | - | new | tape/mesh 0.96x |
| `MeshTape/tape/frame/float/host` | - | - | - | - | - | new | tape/mesh 0.79x |
| `MeshTape/view/firstHit/float/host` | - | - | - | - | - | new | view/mesh 0.58x |
| `MeshTape/view/frame/float/host` | - | - | - | - | - | new | view/mesh 0.74x |
| `MeshTape/mesh/firstHit/double/host` | - | - | - | - | - | new |  |
| `MeshTape/mesh/firstCrossing/double/host` | - | - | - | - | - | new |  |
| `MeshTape/mesh/frame/double/host` | - | - | - | - | - | new |  |
| `MeshTape/tape/firstHit/double/host` | - | - | - | - | - | new | tape/mesh 0.60x |
| `MeshTape/tape/firstCrossing/double/host` | - | - | - | - | - | new | tape/mesh 0.96x |
| `MeshTape/tape/frame/double/host` | - | - | - | - | - | new | tape/mesh 0.75x |
| `MeshTape/view/firstHit/double/host` | - | - | - | - | - | new | view/mesh 0.59x |
| `MeshTape/view/frame/double/host` | - | - | - | - | - | new | view/mesh 0.70x |
| `Ray/mesh/firstHit/float/host` | 46.6 ms | 47.5 ms | +1.9% | [+1.4%, +2.4%] | +0.0% | ~ | hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.7 ms | 45.7 ms | +0.1% | [-0.2%, +0.5%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.6 ms | +0.4% | [+0.2%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.8 ms | 78.7 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/float/host` | 65.9 ms | 66.7 ms | +1.1% | [-0.2%, +2.4%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.72 ms | 5.23 ms | -8.4% | [-15.9%, +0.5%] | - | ~ | tape/fused 1.47x (was 1.34x) |
| `Ray/csg-tape/firstHit/float/host` | 7.66 ms | 7.68 ms | +0.3% | [-0.7%, +1.4%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.5 ms | 52.8 ms | +0.6% | [+0.2%, +1.0%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.2 ms | 51.1 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.7 ms | 45.2 ms | +1.0% | [+0.9%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 86.9 ms | 87.4 ms | +0.6% | [+0.3%, +0.8%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 73.6 ms | 73.3 ms | -0.4% | [-0.8%, +0.0%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.50 ms | 5.92 ms | +7.6% | [-0.4%, +15.7%] | - | ~ | tape/fused 1.35x (was 1.43x) |
| `Ray/csg-tape/firstHit/double/host` | 7.88 ms | 7.97 ms | +1.0% | [+1.0%, +1.1%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.5 ms | 17.8 ms | +1.7% | [+1.3%, +2.1%] | - | ~ | marched/traced 6.75x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 119 ms | 120 ms | +1.0% | [+0.9%, +1.1%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 22.0 ms | 22.0 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | marched/traced 6.77x |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 145 ms | 149 ms | +2.5% | [+2.5%, +2.6%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.2 ms | 35.4 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.7 ms | 19.2 ms | -2.4% | [-2.9%, -1.8%] | - | ~ | marched/traced 6.60x (was 6.33x) |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 125 ms | 127 ms | +1.7% | [+1.5%, +1.9%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.9 ms | 23.4 ms | -2.5% | [-3.7%, -1.2%] | +0.0% | ~ | marched/traced 6.43x |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 150 ms | 150 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.8 ms | 37.9 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.5 ms | 46.2 ms | -0.6% | [-0.7%, -0.4%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.2 ms | 46.3 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.66 ms | 7.82 ms | +2.2% | [+1.4%, +3.0%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.20 ms | 6.08 ms | -2.0% | [-3.3%, -0.6%] | +0.0% | ~ | hit/crossing 1.29x (was 1.24x) |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 178 ms | 176 ms | -1.0% | [-1.2%, -0.8%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 157 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.7 ms | 68.9 ms | +0.2% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.4 ms | 56.8 ms | +0.6% | [+0.1%, +1.2%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayCrossing/mesh/firstHit/double/host` | 53.0 ms | 53.9 ms | +1.7% | [-1.6%, +5.0%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 52.3 ms | 52.2 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ | hit/crossing 1.03x |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.94 ms | 8.00 ms | +0.7% | [-0.1%, +1.5%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.36 ms | 6.43 ms | +1.1% | [-1.4%, +3.7%] | +0.0% | ~ | hit/crossing 1.24x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 200 ms | 201 ms | +0.2% | [-0.0%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 179 ms | 178 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 77.8 ms | 77.6 ms | -0.1% | [-0.9%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.1 ms | 65.4 ms | +2.0% | [+1.3%, +2.7%] | +0.0% | ~ | hit/crossing 1.19x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.1 ms | 11.3 ms | +2.1% | [+0.3%, +3.9%] | +0.0% | ~ | marched/traced 4.62x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 52.0 ms | 52.1 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 7.97 ms | 8.00 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.92 ms | 1.92 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.88 ms | 6.83 ms | -0.7% | [-0.9%, -0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 11.7 ms | 11.7 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ | marched/traced 5.15x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 61.0 ms | 60.3 ms | -1.1% | [-1.7%, -0.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.53 ms | 8.66 ms | +1.5% | [+0.4%, +2.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.95 ms | 1.91 ms | -1.8% | [-3.3%, -0.3%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.15 ms | 7.12 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 240 ms | 238 ms | -0.9% | [-1.2%, -0.5%] | +0.0% | ~ | marched/traced 1.37x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 327 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 237 ms | 237 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 255 ms | 251 ms | -1.2% | [-1.4%, -1.1%] | +0.0% | ~ | marched/traced 1.47x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 367 ms | 369 ms | +0.5% | [+0.3%, +0.8%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 251 ms | 250 ms | -0.6% | [-0.7%, -0.4%] | +0.0% | ~ |  |
| `Regions/particles/region/float/host` | 7.34 ms | 7.32 ms | -0.3% | [-1.0%, +0.4%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.6 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 265 ms | 266 ms | +0.3% | [+0.1%, +0.6%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.4 ms | -0.7% | [-0.9%, -0.5%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.4 ms | 30.4 ms | +0.0% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.1 ms | 22.2 ms | +0.4% | [-0.4%, +1.2%] | +0.0% | ~ | regions/hit 1.37x |
| `Regions/particles/region/double/host` | 7.66 ms | 7.66 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.9 ms | 12.9 ms | -0.1% | [-0.1%, -0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 281 ms | 282 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.0 ms | 13.0 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.4 ms | 32.1 ms | -0.7% | [-0.9%, -0.6%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.7 ms | 23.7 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ | regions/hit 1.35x |
| `Render/csg/frame/float/host` | 19.5 ms | 12.7 ms | -34.8% | [-37.4%, -32.2%] | +0.0% | faster |  |
| `Render/csg/frame-orthographic/float/host` | 20.2 ms | 12.5 ms | -37.8% | [-40.8%, -34.7%] | +0.0% | faster |  |
| `Render/mesh/frame/float/host` | 20.0 ms | 21.3 ms | +6.6% | [-4.5%, +17.8%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 22.0 ms | 14.2 ms | -35.3% | [-36.2%, -34.4%] | +0.0% | faster |  |
| `Render/csg/frame-orthographic/double/host` | 22.0 ms | 13.8 ms | -37.3% | [-38.1%, -36.6%] | +0.0% | faster |  |
| `Render/mesh/frame/double/host` | 21.3 ms | 20.2 ms | -5.4% | [-5.7%, -5.2%] | +0.0% | faster |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 75.0 ms | 74.9 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 297 ms | 296 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 297 ms | 296 ms | -0.4% | [-0.6%, -0.3%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 87.1 ms | 87.1 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 348 ms | 346 ms | -0.6% | [-1.0%, -0.1%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 350 ms | 348 ms | -0.6% | [-1.1%, -0.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 144 ms | 145 ms | +1.1% | [+0.8%, +1.4%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 89.6 ms | 89.7 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ | surface/leaf 1.62x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 130 ms | 130 ms | +0.3% | [+0.1%, +0.4%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 72.7 ms | 72.6 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ | surface/leaf 1.80x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 61.3 ms | 61.3 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 33.9 ms | 34.1 ms | +0.6% | [+0.4%, +0.9%] | +0.0% | ~ | surface/hit 1.80x |
| `SurfaceParts/boxes/closestSurface/double/host` | 145 ms | 145 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 90.7 ms | 91.1 ms | +0.5% | [+0.4%, +0.6%] | +0.0% | ~ | surface/leaf 1.59x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 145 ms | 145 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 81.6 ms | 81.3 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ | surface/leaf 1.78x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 68.6 ms | 68.1 ms | -0.9% | [-1.5%, -0.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 38.8 ms | 38.4 ms | -1.0% | [-1.2%, -0.7%] | +0.0% | ~ | surface/hit 1.77x |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.8 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.58 ms | 4.61 ms | +0.5% | [-0.8%, +1.7%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 25.0 ms | 25.0 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 39.9 ms | 39.9 ms | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.76 ms | 1.73 ms | -1.2% | [-1.8%, -0.6%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 133 µs | 139 µs | +4.8% | [-3.3%, +13.9%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.8 ms | 29.8 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.66 ms | 5.67 ms | +0.2% | [+0.1%, +0.3%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.3 ms | 16.2 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.67 ms | 4.64 ms | -0.6% | [-0.9%, -0.3%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.2 ms | 13.2 ms | +0.5% | [+0.3%, +0.8%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.65 ms | 4.64 ms | -0.2% | [-1.1%, +0.7%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.5 ms | 26.6 ms | +0.4% | [+0.3%, +0.4%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 41.9 ms | 41.8 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.79 ms | 1.78 ms | -0.6% | [-3.9%, +2.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 287 µs | 268 µs | -6.6% | [-10.5%, -2.6%] | - | faster |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.9 ms | 29.8 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.73 ms | 7.76 ms | +0.3% | [-0.0%, +0.7%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.4 ms | 16.1 ms | -1.8% | [-1.9%, -1.7%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.78 ms | 7.77 ms | -0.1% | [-0.8%, +0.5%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.9 ms | 41.8 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 201 ms | 201 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 43.1 ms | 42.9 ms | -0.5% | [-1.4%, +0.3%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 218 ms | 218 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 643.5 s → 931.0 s (+44.7%), the largest peak 1399 MB → 1398 MB; 26 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.2 s | 17.5 s | +90.9% | 557 MB | 558 MB | +0.1% | **MAJOR** |
| `BVHRefit` | 3.9 s | 7.3 s | +85.8% | 483 MB | 484 MB | +0.1% | **MAJOR** |
| `BVHUnion` | 17.8 s | 32.3 s | +81.8% | 896 MB | 896 MB | +0.0% | **MAJOR** |
| `BVHUnionMixed` | 7.3 s | 10.3 s | +40.7% | 555 MB | 556 MB | +0.1% | **MAJOR** |
| `ClosestPrimitive` | 28.8 s | 44.1 s | +53.0% | 1297 MB | 1298 MB | +0.1% | **MAJOR** |
| `ClosestSurface` | 27.1 s | 51.6 s | +90.6% | 1219 MB | 1221 MB | +0.1% | **MAJOR** |
| `Domain` | 17.5 s | 33.0 s | +89.0% | 841 MB | 841 MB | +0.1% | **MAJOR** |
| `DomainObject` | 15.5 s | 24.6 s | +58.7% | 844 MB | 844 MB | +0.0% | **MAJOR** |
| `Fused` | 23.3 s | 33.7 s | +44.8% | 1126 MB | 1126 MB | -0.0% | **MAJOR** |
| `LaunchTuning` | 34.9 s | 54.1 s | +55.1% | 1303 MB | 1304 MB | +0.1% | **MAJOR** |
| `Mesh` | 14.3 s | 24.9 s | +74.3% | 784 MB | 784 MB | +0.0% | **MAJOR** |
| `MeshCSG` | 37.7 s | 50.1 s | +33.0% | 1258 MB | 1255 MB | -0.3% | **MAJOR** |
| `MeshClosestPoint` | 12.9 s | 21.6 s | +67.4% | 780 MB | 780 MB | -0.0% | **MAJOR** |
| `MeshReconcile` | 6.3 s | 9.8 s | +55.1% | 565 MB | 566 MB | +0.0% | **MAJOR** |
| `MeshTape` | - | 47.5 s | | - | 1287 MB | | new |
| `PointCloud` | 7.1 s | 13.3 s | +86.9% | 558 MB | 558 MB | -0.1% | **MAJOR** |
| `PointCloudBVHBuild` | 3.6 s | 7.0 s | +92.0% | 470 MB | 470 MB | +0.0% | **MAJOR** |
| `PointCloudHashGridBuild` | 3.2 s | 4.1 s | +29.0% | 452 MB | 452 MB | +0.0% | ~ |
| `Ray` | 34.9 s | 40.2 s | +14.9% | 1316 MB | 1318 MB | +0.1% | **minor** |
| `RayCSG` | 30.8 s | 36.6 s | +19.1% | 1176 MB | 1183 MB | +0.6% | **minor** |
| `RayCrossing` | 44.8 s | 49.8 s | +11.1% | 1331 MB | 1328 MB | -0.2% | **minor** |
| `RayParticles` | 47.6 s | 57.0 s | +19.8% | 1317 MB | 1316 MB | -0.1% | **minor** |
| `RayShell` | 36.1 s | 39.4 s | +9.0% | 1278 MB | 1280 MB | +0.1% | ~ |
| `Regions` | 27.8 s | 32.5 s | +16.9% | 1070 MB | 1070 MB | -0.0% | **minor** |
| `Render` | 35.0 s | 48.7 s | +39.3% | 1282 MB | 1280 MB | -0.1% | **MAJOR** |
| `Shapes` | 8.9 s | 9.6 s | +8.4% | 554 MB | 554 MB | -0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 5.3 s | +11.5% | 511 MB | 511 MB | +0.0% | ~ |
| `SphereCloudRays` | 27.2 s | 44.0 s | +61.6% | 1061 MB | 1062 MB | +0.1% | **MAJOR** |
| `SphereContacts` | 4.3 s | 4.2 s | -0.4% | 484 MB | 484 MB | -0.0% | ~ |
| `SurfaceParts` | 40.0 s | 75.9 s | +89.6% | 1399 MB | 1398 MB | -0.1% | **MAJOR** |
| `Tape` | 16.6 s | 24.3 s | +45.8% | 819 MB | 818 MB | -0.1% | **MAJOR** |
| `TapeBVHUnion` | 14.4 s | 24.3 s | +68.6% | 822 MB | 823 MB | +0.0% | **MAJOR** |
| **all, in both** | 643.5 s | 931.0 s | +44.7% | 1399 MB | 1398 MB | -0.1% | **MAJOR** |


These probes compiled to the same code on both sides and were not run (120 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), Mesh (12), MeshClosestPoint (8), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20), SphereCloudBVH (6), SphereContacts (2)

The probes differ between the sides (each side ran its own): Performance/MeshTape/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: 9a82671
base-fingerprint: 5b762623a2468955cf1da5e76311c603f57f1727
head: 7c2679b
head-fingerprint: 444341ad48073521e98b9c88662429cb15c2889c
cases: 274
rounds: 2
reps: 3
date: 2026-11-17
-->

## Device comparison (CUDA): red

**Accepted:** The one-pixel DIFFER is the host/device shading difference of issue #343, also on main. The three minor regressions: accept as change in algorithm.

`7c2679b` against `9a82671` (origin/main), 2026-11-17.

**VERDICT: host and device answers differ in 4 cases; investigate before merging (see Flagged above)**

Performance comparison: 0 major, 3 minor, 18 faster, 4 differ, 0 lost, 274 cases

- base: 9a82671 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 7c2679b (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 314 µs | 313 µs | -0.4% | [-0.6%, -0.1%] | - | ~ | device 11.6x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 6.3x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.39 ms | 5.39 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.6x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 525 ms | 523 ms | -0.2% | [-1.8%, +1.3%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.1 ms | 20.1 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 4.1x faster than the host |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.34 ms | 8.34 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 4.7x faster than the host; surface/leaf 2.42x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 15.6 ms | 15.6 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 7.9x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.20 ms | 6.20 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 10.0x faster than the host; surface/leaf 2.52x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 147 ms | 144 ms | -1.6% | [-3.0%, -0.2%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 60.0 ms | 60.1 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.41x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 72.0 ms | 71.9 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.8x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.0 ms | 28.0 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 2.3x faster than the host; surface/leaf 2.57x |
| `Domain/particles/labels/float/hostdevice` | 4.10 ms | 4.10 ms | +0.2% | [-0.5%, +0.8%] | - | ~ | device 3.4x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.72 ms | 2.73 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 2.6x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 118 µs | 122 µs | +2.9% | [+1.2%, +4.6%] | - | ~ | device 12.4x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.20 ms | 7.41 ms | +3.0% | [-0.4%, +6.4%] | - | ~ | device 1.9x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.41 ms | 3.40 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `Domain/particles/surfaceGroups/double/hostdevice` | 128 µs | 127 µs | -1.4% | [-4.0%, +1.3%] | - | ~ | device 11.0x faster than the host |
| `DomainObject/partsWhole/labels/float/hostdevice` | 12.4 ms | 12.4 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.76 ms | 8.76 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.6x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 270 µs | 271 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 2.4x faster than the host (was 2.5x); own/whole 45.69x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 147 µs | 148 µs | +0.7% | [-0.1%, +1.5%] | - | ~ | device 2.4x faster than the host (was 2.5x); own/whole 59.33x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 19.8 ms | 19.8 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 2.4x faster than the host |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 11.1 ms | 11.1 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 2.2x faster than the host |
| `DomainObject/partsOwn/labels/double/hostdevice` | 388 µs | 399 µs | +3.0% | [+1.2%, +4.8%] | - | ~ | device 1.6x faster than the host (was 1.8x); own/whole 49.50x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 231 µs | 230 µs | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.6x faster than the host (was 1.7x); own/whole 48.24x |
| `Fused/csg/signedDistance/float/hostdevice` | 48.0 µs | 48.1 µs | +0.3% | [-0.0%, +0.7%] | - | ~ | device 23.5x faster than the host; tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.0 µs | 50.0 µs | -0.0% | [-0.7%, +0.6%] | - | ~ | device 43.1x faster than the host; tape/fused 7.19x |
| `Fused/chain16/signedDistance/float/hostdevice` | 47.9 µs | 48.9 µs | +2.1% | [-0.2%, +4.4%] | - | ~ | device 9.4x faster than the host (was 9.1x); tape/fused 9.74x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.3 ms | +0.4% | [-0.4%, +1.1%] | - | ~ | device 6.7x faster than the host; tape/fused 1.80x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.1 ms | 31.1 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 4.2x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 1.6x faster than the host (was 1.5x); tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 284 ms | 286 ms | +0.8% | [-0.9%, +2.6%] | - | ~ | device 0.4x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 285 ms | 289 ms | +1.1% | [+0.8%, +1.4%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.22 ms | 8.26 ms | +0.4% | [-0.7%, +1.6%] | +0.0% | ~ | device 10.7x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.22 ms | 8.34 ms | +1.5% | [+1.0%, +1.9%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.31 ms | 8.24 ms | -0.8% | [-2.3%, +0.7%] | +0.0% | ~ | device 10.8x faster than the host (was 12.7x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.31 ms | 8.20 ms | -1.3% | [-3.0%, +0.4%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 684 µs | 701 µs | +2.6% | [+2.2%, +2.9%] | +0.0% | ~ | device 3.7x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.73 ms | 1.79 ms | +3.4% | [+3.1%, +3.5%] | +0.0% | minor | device 1.4x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.04 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.68x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 787 µs | 804 µs | +2.2% | [+1.6%, +2.8%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.87x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 745 µs | 741 µs | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 841 µs | 841 µs | -0.0% | [-0.1%, +0.1%] | +0.0% | ~ | device 10.6x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 742 µs | 742 µs | +0.0% | [-0.7%, +0.8%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 729 µs | 729 µs | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.02x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 159 ms | 162 ms | +2.0% | [-0.4%, +2.5%] | +0.0% | ~ | device 0.6x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 162 ms | 161 ms | -0.6% | [-2.2%, +1.0%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 162 ms | 161 ms | -0.7% | [-2.2%, +0.8%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 161 ms | 163 ms | +1.2% | [-0.9%, +3.3%] | +0.0% | ~ | device 0.5x faster than the host; default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.39 ms | -0.2% | [-0.6%, +0.3%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.78 ms | 2.78 ms | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.94 ms | 1.97 ms | +1.3% | [+0.6%, +1.9%] | +0.0% | ~ | device 1.4x faster than the host (was 1.4x); default/this 0.71x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.55 ms | 1.56 ms | +0.4% | [+0.3%, +0.4%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.24 ms | 7.28 ms | +0.6% | [+0.2%, +1.0%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.32 ms | 7.32 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.23 ms | 7.23 ms | +0.0% | [-0.7%, +0.7%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.01x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.29 ms | 7.30 ms | +0.1% | [-0.2%, +0.4%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.53 ms | 1.98 ms | -21.5% | [-21.8%, -21.1%] | - | faster | device 5.0x faster than the host (was 4.1x) |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.29 ms | 3.35 ms | -21.9% | [-22.2%, -21.6%] | - | faster | device 4.0x faster than the host (was 3.2x) |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.9 ms | 6.05 ms | -56.5% | [-56.7%, -56.3%] | - | faster | device 4.7x faster than the host (was 3.9x) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.4 ms | 63.2 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 92.3 ms | 92.5 ms | +0.2% | [-1.5%, +1.9%] | - | ~ | device 2.6x faster than the host (was 2.3x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 254 ms | 240 ms | -5.6% | [-7.3%, -3.9%] | - | faster | device 3.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.7 ms | 24.7 ms | -0.0% | [-1.1%, +1.1%] | - | ~ | device 2.0x faster than the host; tape/fused 2.56x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 26.6 ms | -1.1% | [-2.0%, -0.2%] | - | ~ | device 2.2x faster than the host; tape/fused 3.48x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 73.0 ms | 64.4 ms | -11.7% | [-12.5%, -10.9%] | - | faster | device 3.9x faster than the host; tape/fused 3.72x (was 3.48x) |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.2 ms | 20.0 ms | -9.7% | [-11.6%, -7.7%] | - | faster | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.8 ms | 37.9 ms | -15.2% | [-18.3%, -12.1%] | - | faster | device 0.4x faster than the host (was 0.3x) |
| `MeshCSG/union-tape/frame/double/hostdevice` | 98.8 ms | 40.6 ms | -58.9% | [-59.3%, -58.5%] | - | faster | device 0.7x faster than the host (was 0.6x) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 547 ms | 552 ms | +0.8% | [-1.0%, +2.7%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 863 ms | 845 ms | -2.1% | [-3.2%, -1.0%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.05 s | 1.96 s | -4.6% | [-7.0%, -2.1%] | - | faster | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 193 ms | 194 ms | +0.2% | [-3.4%, +3.9%] | - | ~ | device 0.3x faster than the host; tape/fused 2.85x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 313 ms | 299 ms | -4.4% | [-10.1%, +1.4%] | - | ~ | device 0.2x faster than the host (was 0.2x); tape/fused 2.83x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 589 ms | 550 ms | -6.7% | [-8.3%, -5.1%] | - | faster | device 0.5x faster than the host (was 0.5x); tape/fused 3.56x |
| `MeshTape/mesh/firstHit/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshTape/mesh/frame/float/hostdevice` | - | - | - | - | - | **DIFFER** | new; 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/tape/firstHit/float/hostdevice` | - | - | - | - | - | new | tape/mesh 0.38x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | - | - | - | - | - | new | tape/mesh 0.48x |
| `MeshTape/tape/frame/float/hostdevice` | - | - | - | - | - | **DIFFER** | new; tape/mesh 0.49x; 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/view/firstHit/float/hostdevice` | - | - | - | - | - | new | view/mesh 0.36x |
| `MeshTape/view/frame/float/hostdevice` | - | - | - | - | - | **DIFFER** | new; view/mesh 0.38x; 1 host and device answers DIFFER (max diff 0.11) |
| `MeshTape/mesh/firstHit/double/hostdevice` | - | - | - | - | - | new |  |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | - | - | - | - | - | new |  |
| `MeshTape/mesh/frame/double/hostdevice` | - | - | - | - | - | new |  |
| `MeshTape/tape/firstHit/double/hostdevice` | - | - | - | - | - | new | tape/mesh 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | - | - | - | - | - | new | tape/mesh 1.00x |
| `MeshTape/tape/frame/double/hostdevice` | - | - | - | - | - | new | tape/mesh 0.78x |
| `MeshTape/view/firstHit/double/hostdevice` | - | - | - | - | - | new | view/mesh 0.70x |
| `MeshTape/view/frame/double/hostdevice` | - | - | - | - | - | new | view/mesh 0.72x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 679 µs | 674 µs | -0.8% | [-1.6%, +0.0%] | - | ~ | device 16.8x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 14.6x faster than the host; hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 645 µs | 649 µs | +0.7% | [-0.3%, +1.7%] | - | ~ | device 17.3x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.22 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 15.5x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 603 µs | 606 µs | +0.6% | [+0.1%, +1.0%] | - | ~ | device 16.1x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.11 ms | 1.12 ms | +0.8% | [+0.4%, +1.2%] | - | ~ | device 15.0x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.18 ms | 1.18 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.47 ms | 2.47 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.08 ms | +0.5% | [+0.0%, +0.9%] | - | ~ | device 11.7x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.19 ms | +0.4% | [-0.1%, +1.0%] | - | ~ | device 9.7x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 612 µs | 613 µs | +0.1% | [-5.8%, +6.4%] | - | ~ | device 2.4x faster than the host; tape/fused 1.14x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 699 µs | 726 µs | +4.0% | [-8.6%, +9.5%] | - | ~ | device 2.5x faster than the host (was 2.6x); tape/fused 1.18x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 685 µs | 701 µs | +2.3% | [+2.0%, +2.6%] | - | ~ | device 3.6x faster than the host (was 3.8x) |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 842 µs | 856 µs | +1.7% | [+1.3%, +2.0%] | - | ~ | device 3.5x faster than the host (was 3.6x) |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.5 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.1 ms | 17.0 ms | -0.6% | [-1.7%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.5 ms | 27.4 ms | -0.5% | [-1.0%, -0.1%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.9 ms | 14.9 ms | +0.5% | [-0.5%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.7 ms | +0.7% | [+0.5%, +0.9%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.5 ms | 16.5 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.6 ms | +0.3% | [-0.1%, +0.6%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.1 ms | 15.0 ms | -0.5% | [-1.5%, +0.5%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.0 ms | -0.2% | [-1.1%, +0.8%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.24 ms | 1.24 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 1.3x faster than the host; tape/fused 1.11x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.36 ms | 2.37 ms | +0.7% | [+0.3%, +1.2%] | - | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.4% | [-0.9%, +0.1%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | +0.1% | [-0.6%, +0.9%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.56 ms | 2.56 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 2.5x faster than the host (was 3.0x); marched/traced 3.53x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.00 ms | 3.00 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 2.3x faster than the host (was 2.7x); marched/traced 4.61x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.00 ms | 9.02 ms | +0.2% | [-0.1%, +0.6%] | - | ~ | device 4.9x faster than the host (was 4.4x) |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.8 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 3.7x faster than the host (was 3.2x) |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.19 ms | +0.5% | [+0.4%, +0.5%] | - | ~ | device 3.9x faster than the host; marched/traced 13.33x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.46 ms | 5.47 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 2.8x faster than the host; marched/traced 10.18x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.1 ms | 29.2 ms | +0.2% | [-0.0%, +0.5%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.1 ms | 55.7 ms | +1.0% | [+0.8%, +1.3%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.23 ms | 5.26 ms | +0.6% | [+0.3%, +0.9%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.76 ms | 8.82 ms | +0.7% | [+0.3%, +1.1%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.14 ms | 6.21 ms | +1.0% | [+0.6%, +1.4%] | - | ~ | device 1.1x faster than the host (was 1.3x); marched/traced 25.86x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.1% | [-0.3%, -0.0%] | - | ~ | device 0.7x faster than the host (was 0.8x); marched/traced 24.01x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 160 ms | 160 ms | +0.4% | [-1.0%, +1.8%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 255 ms | 253 ms | -0.5% | [-0.9%, -0.0%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.87 ms | 4.86 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 1.8x faster than the host; marched/traced 22.47x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.2 ms | -0.4% | [-0.7%, -0.1%] | - | ~ | device 1.3x faster than the host; marched/traced 17.75x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 110 ms | 109 ms | -0.4% | [-4.2%, +3.5%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 214 ms | 217 ms | +1.6% | [-0.5%, +3.6%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.4 ms | 19.5 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.8 ms | 26.7 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 678 µs | 669 µs | -1.3% | [-2.4%, -0.3%] | - | ~ | device 17.3x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.29 ms | -0.1% | [-0.5%, +0.4%] | - | ~ | device 14.9x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 674 µs | 675 µs | +0.1% | [-0.6%, +0.8%] | - | ~ | device 17.1x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.28 ms | +0.4% | [+0.0%, +0.9%] | - | ~ | device 15.0x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 711 µs | 709 µs | -0.2% | [-0.8%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 850 µs | 857 µs | +0.7% | [-1.0%, +2.5%] | - | ~ | device 3.5x faster than the host (was 3.6x) |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 626 µs | 614 µs | -1.9% | [-2.5%, -1.4%] | - | ~ | device 3.2x faster than the host; hit/crossing 1.16x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 646 µs | 633 µs | -2.0% | [-2.4%, -1.6%] | - | ~ | device 3.6x faster than the host; hit/crossing 1.35x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.69 ms | 4.71 ms | +0.5% | [+0.3%, +0.7%] | - | ~ | device 10.0x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 6.8x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.19 ms | 4.18 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 9.9x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.10 ms | 9.07 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 6.7x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.1 ms | 55.4 ms | -1.1% | [-2.3%, +0.0%] | - | ~ | device 3.3x faster than the host (was 3.1x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.5 ms | 81.6 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 2.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 47.9 ms | 48.9 ms | +2.2% | [+0.6%, +3.7%] | - | ~ | device 3.0x faster than the host; hit/crossing 1.13x (was 1.17x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.5 ms | 64.2 ms | +1.1% | [+0.6%, +1.5%] | - | ~ | device 2.4x faster than the host (was 2.7x); hit/crossing 1.27x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 17.1 ms | +1.2% | [+0.1%, +2.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.5 ms | 27.5 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 16.9 ms | -0.6% | [-1.6%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.02x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.4 ms | 27.4 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.40 ms | 1.39 ms | -0.6% | [-0.7%, -0.6%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.51 ms | 2.49 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.10 ms | -0.6% | [-1.0%, -0.1%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.26x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.15 ms | 1.15 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.16x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.4 ms | 64.3 ms | -0.2% | [-0.6%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 121 ms | 122 ms | +0.1% | [-1.1%, +1.2%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.5 ms | 58.5 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.10x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 111 ms | 112 ms | +0.9% | [-0.7%, +2.5%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.09x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 465 ms | 480 ms | +3.4% | [+1.0%, +6.1%] | - | minor | device 0.4x faster than the host (was 0.4x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 720 ms | 709 ms | -1.5% | [-3.2%, +0.3%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 398 ms | 357 ms | -10.3% | [-12.9%, -7.6%] | - | faster | device 0.4x faster than the host (was 0.4x); hit/crossing 1.34x (was 1.17x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 538 ms | 531 ms | -1.3% | [-3.3%, +0.8%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.34x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.58 ms | 3.62 ms | +1.2% | [-2.3%, +4.8%] | - | ~ | device 1.2x faster than the host; marched/traced 3.32x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.78 ms | 4.77 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 0.9x faster than the host; marched/traced 5.57x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.0 ms | 12.0 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.5 ms | 26.6 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.26 ms | 1.25 ms | -0.8% | [-2.5%, +1.0%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.84 ms | 1.86 ms | +1.1% | [+0.0%, +2.2%] | - | ~ | device 1.8x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 97.3 µs | 97.6 µs | +0.3% | [-0.7%, +1.4%] | - | ~ | device 14.9x faster than the host |
| `RayParticles/armadillo/step-random/float/hostdevice` | 154 µs | 151 µs | -2.0% | [-3.2%, -0.9%] | - | ~ | device 10.8x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 795 µs | 794 µs | -0.2% | [-0.6%, +0.2%] | - | ~ | device 3.1x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 927 µs | 927 µs | -0.0% | [-0.3%, +0.2%] | - | ~ | device 2.8x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 34.7 ms | 35.4 ms | +2.2% | [-4.8%, +10.1%] | - | ~ | device 0.1x faster than the host (was 0.1x); marched/traced 3.49x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.6 ms | 36.8 ms | +0.6% | [-1.0%, +2.1%] | - | ~ | device 0.1x faster than the host; marched/traced 6.30x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 122 ms | 124 ms | +1.3% | [-0.1%, +2.7%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 230 ms | 232 ms | +0.8% | [+0.4%, +1.3%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.2 ms | 16.5 ms | +2.3% | [+1.9%, +2.7%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.8 ms | 22.1 ms | +1.3% | [+0.5%, +2.2%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.08 ms | 1.11 ms | +2.6% | [+1.1%, +4.2%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.37 ms | 2.34 ms | -1.2% | [-2.5%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.97 ms | 5.97 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.71 ms | 6.71 ms | -0.0% | [-0.8%, +0.7%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.2 ms | 23.1 ms | -0.6% | [-1.9%, +0.7%] | - | ~ | device 2.2x faster than the host; marched/traced 2.71x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.2 ms | 28.5 ms | +1.0% | [+0.6%, +1.5%] | - | ~ | device 1.9x faster than the host; marched/traced 3.70x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.7 ms | 62.6 ms | -0.2% | [-0.9%, +0.6%] | - | ~ | device 3.5x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 104 ms | 105 ms | +0.9% | [+0.2%, +1.7%] | - | ~ | device 2.3x faster than the host (was 2.5x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.8 ms | +0.3% | [-1.0%, +1.7%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.5 ms | +1.1% | [+0.2%, +2.0%] | - | ~ | device 3.1x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 309 ms | 296 ms | -4.2% | [-4.6%, -3.7%] | - | faster | device 0.2x faster than the host (was 0.2x); marched/traced 1.92x (was 1.82x) |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 357 ms | 346 ms | -3.0% | [-5.8%, -0.2%] | - | faster | device 0.2x faster than the host (was 0.2x); marched/traced 2.88x (was 2.77x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 562 ms | 567 ms | +1.0% | [-0.6%, +2.7%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 989 ms | 997 ms | +0.9% | [-0.5%, +2.3%] | - | ~ | device 0.2x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 193 ms | 184 ms | -4.7% | [-7.9%, -1.3%] | - | faster | device 0.3x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 239 ms | 236 ms | -1.0% | [-2.8%, +0.7%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.73 ms | 2.73 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 2.8x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.71 ms | 3.71 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 3.5x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 89.2 ms | 88.7 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 4.4x faster than the host (was 3.5x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.74 ms | 3.75 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 3.6x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.49 ms | 6.70 ms | +3.3% | [+3.1%, +3.5%] | - | minor | device 3.7x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.43 ms | 5.52 ms | +1.8% | [+1.5%, +2.0%] | - | ~ | device 3.2x faster than the host; regions/hit 1.21x |
| `Regions/particles/region/double/hostdevice` | 3.34 ms | 3.33 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.84 ms | 5.85 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 125 ms | 125 ms | +0.3% | [-0.7%, +1.3%] | - | ~ | device 3.0x faster than the host (was 2.5x); each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.95 ms | 5.93 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 2.3x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.7 ms | 15.8 ms | +0.7% | [+0.3%, +1.1%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 13.0 ms | 12.9 ms | -0.4% | [-0.8%, +0.0%] | - | ~ | device 1.3x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 4.28 ms | 2.38 ms | -44.5% | [-44.6%, -44.5%] | - | faster | device 3.0x faster than the host (was 2.7x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.29 ms | 2.37 ms | -44.7% | [-44.8%, -44.7%] | - | faster | device 3.0x faster than the host (was 2.7x) |
| `Render/mesh/frame/float/hostdevice` | 2.04 ms | 2.09 ms | +2.4% | [+2.3%, +2.5%] | - | **DIFFER** | device 7.8x faster than the host (was 8.3x); 1 host and device answers DIFFER (max diff 0.11) |
| `Render/csg/frame/double/hostdevice` | 7.97 ms | 5.62 ms | -29.5% | [-29.6%, -29.4%] | - | faster | device 1.4x faster than the host (was 1.6x) |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.96 ms | 5.62 ms | -29.5% | [-29.5%, -29.4%] | - | faster | device 1.4x faster than the host (was 1.6x) |
| `Render/mesh/frame/double/hostdevice` | 23.5 ms | 23.7 ms | +1.2% | [+0.5%, +1.8%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.01 ms | 2.99 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ | device 7.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.73 ms | 4.75 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 5.1x faster than the host; tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.2 ms | 24.2 ms | -0.0% | [-0.2%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 23.0 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.3 ms | 30.5 ms | +0.5% | [+0.4%, +0.6%] | +0.0% | ~ | device 3.0x faster than the host (was 3.3x) |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.1 ms | 44.1 ms | +0.2% | [-0.2%, +0.6%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.7 ms | 45.6 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 123 ms | 122 ms | -0.8% | [-3.5%, +2.0%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.05x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 125 ms | 127 ms | +2.1% | [-1.1%, +3.5%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.06x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 129 ms | 129 ms | -0.7% | [-4.7%, +3.5%] | +0.0% | ~ | device 0.8x faster than the host (was 0.7x) |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 136 ms | 136 ms | -0.2% | [-1.9%, +1.6%] | +0.0% | ~ | device 0.7x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.31 ms | 9.33 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 11.4x faster than the host |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.93 ms | 5.93 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 9.9x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 96.7 ms | 96.5 ms | -0.3% | [-1.7%, +1.2%] | - | ~ | device 3.8x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 63.4 ms | 63.4 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 3.2x faster than the host (was 3.5x); surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.1 ms | 29.2 ms | +0.2% | [+0.1%, +0.3%] | - | ~ | device 5.0x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 9.90 ms | 9.94 ms | +0.4% | [+0.2%, +0.5%] | - | ~ | device 6.8x faster than the host; surface/hit 2.94x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 59.0 ms | 59.0 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.2x faster than the host (was 2.6x) |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 38.9 ms | 38.9 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 1.5x faster than the host (was 1.6x); surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 783 ms | 782 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 0.4x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 527 ms | 529 ms | +0.4% | [-0.0%, +0.8%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.48x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 309 ms | 307 ms | -0.8% | [-3.3%, +1.9%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 119 ms | 119 ms | +0.4% | [-2.1%, +2.9%] | - | ~ | device 0.6x faster than the host; surface/hit 2.57x |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 219 µs | +0.2% | [+0.0%, +0.3%] | - | ~ | device 11.2x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.9 µs | 47.8 µs | -0.3% | [-0.6%, -0.1%] | - | ~ | device 23.3x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 359 µs | -0.0% | [-0.4%, +0.3%] | - | ~ | device 12.6x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 9.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.4 µs | 72.3 µs | -0.1% | [-0.7%, +0.5%] | - | ~ | device 6.9x faster than the host (was 7.3x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.3 µs | 47.5 µs | +0.3% | [+0.2%, +0.5%] | - | ~ | device 4.8x faster than the host (was 5.0x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 651 µs | -0.1% | [-0.3%, -0.0%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.0% | [-0.1%, +0.1%] | - | ~ | device 11.6x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 476 µs | 476 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.5 µs | 48.6 µs | +0.1% | [-0.2%, +0.4%] | - | ~ | device 27.2x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 858 µs | 859 µs | +0.1% | [+0.0%, +0.2%] | - | ~ | device 3.1x faster than the host |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 811 µs | 810 µs | -0.1% | [-0.3%, -0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.65 ms | 6.65 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 192 µs | 191 µs | -0.1% | [-0.1%, +0.0%] | - | ~ | device 2.8x faster than the host (was 3.0x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 164 µs | 163 µs | -0.2% | [-0.3%, -0.1%] | - | ~ | device 2.0x faster than the host (was 1.8x) |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.70 ms | -0.3% | [-0.4%, -0.1%] | - | ~ | device 11.0x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.51 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4847.3 s → 5229.8 s (+7.9%), the largest peak 6080 MB → 6285 MB; 4 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.9 s | 12.8 s | -0.6% | 600 MB | 600 MB | +0.0% | ~ |
| `BVHRefit` | 6.6 s | 6.6 s | +0.1% | 506 MB | 506 MB | +0.0% | ~ |
| `BVHUnion` | 24.4 s | 24.5 s | +0.5% | 1025 MB | 1026 MB | +0.0% | ~ |
| `BVHUnionMixed` | 10.9 s | 10.9 s | -0.6% | 625 MB | 626 MB | +0.1% | ~ |
| `ClosestPrimitive` | 128.9 s | 126.6 s | -1.8% | 1347 MB | 1348 MB | +0.1% | ~ |
| `ClosestSurface` | 188.6 s | 190.6 s | +1.0% | 1844 MB | 1844 MB | +0.0% | ~ |
| `Domain` | 59.3 s | 59.9 s | +1.1% | 994 MB | 995 MB | +0.1% | ~ |
| `DomainObject` | 56.4 s | 57.3 s | +1.7% | 949 MB | 949 MB | +0.0% | ~ |
| `Fused` | 54.2 s | 54.1 s | -0.1% | 1158 MB | 1158 MB | +0.0% | ~ |
| `LaunchTuning` | 537.1 s | 545.1 s | +1.5% | 3806 MB | 3859 MB | +1.4% | ~ |
| `Mesh` | 20.2 s | 20.1 s | -0.5% | 802 MB | 802 MB | +0.0% | ~ |
| `MeshCSG` | 399.9 s | 420.9 s | +5.3% | 6080 MB | 6285 MB | +3.4% | ~ |
| `MeshClosestPoint` | 18.0 s | 18.2 s | +0.8% | 755 MB | 755 MB | +0.0% | ~ |
| `MeshReconcile` | 10.2 s | 10.1 s | -1.1% | 512 MB | 513 MB | +0.1% | ~ |
| `MeshTape` | - | 770.7 s | | - | 6549 MB | | new |
| `PointCloud` | 12.4 s | 12.8 s | +3.1% | 571 MB | 571 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 7.0 s | 7.1 s | +1.2% | 506 MB | 507 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 6.5 s | 6.5 s | -0.8% | 506 MB | 507 MB | +0.1% | ~ |
| `Ray` | 188.9 s | 193.2 s | +2.3% | 3703 MB | 3756 MB | +1.4% | ~ |
| `RayCSG` | 221.7 s | 228.0 s | +2.8% | 4086 MB | 4140 MB | +1.3% | ~ |
| `RayCrossing` | 439.0 s | 443.5 s | +1.0% | 4017 MB | 4102 MB | +2.1% | ~ |
| `RayParticles` | 677.7 s | 863.9 s | +27.5% | 5632 MB | 5781 MB | +2.6% | **MAJOR** |
| `RayShell` | 419.3 s | 516.1 s | +23.1% | 5346 MB | 5488 MB | +2.7% | **minor** |
| `Regions` | 363.3 s | 372.4 s | +2.5% | 3835 MB | 4128 MB | +7.6% | ~ |
| `Render` | 255.1 s | 258.2 s | +1.2% | 5896 MB | 6048 MB | +2.6% | ~ |
| `Shapes` | 26.2 s | 26.6 s | +1.5% | 592 MB | 592 MB | +0.1% | ~ |
| `SphereCloudBVH` | 8.6 s | 8.8 s | +2.2% | 507 MB | 507 MB | +0.1% | ~ |
| `SphereCloudRays` | 187.7 s | 184.2 s | -1.9% | 4093 MB | 4153 MB | +1.5% | ~ |
| `SphereContacts` | 8.3 s | 7.9 s | -4.6% | 506 MB | 506 MB | +0.1% | ~ |
| `SurfaceParts` | 399.6 s | 429.3 s | +7.4% | 4241 MB | 4398 MB | +3.7% | ~ |
| `Tape` | 57.4 s | 67.5 s | +17.6% | 981 MB | 982 MB | +0.1% | **minor** |
| `TapeBVHUnion` | 40.9 s | 46.1 s | +12.7% | 922 MB | 922 MB | +0.1% | **minor** |
| **all, in both** | 4847.3 s | 5229.8 s | +7.9% | 6080 MB | 6285 MB | +3.4% | ~ |

Device kernels: 202 in both, 42 changed (10 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 16 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Fused::Node<Fused::SmoothCombine…` | 168 → 168 | 2800 → 2808 | 110/248 → 112/244 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Fused::Node<Fused::SmoothCombineO…` | 120 → 118 | 1856 → 1856 | 0/0 → 0/0 | **registers** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Fused::Node<Fused::SmoothCombineOp<…` | 168 → 168 | 5232 → 5360 | 1100/920 → 1212/996 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19760 → 19248 | 1388/1372 → 1532/1476 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Fused::Node<Fused::SmoothCombineOp<f…` | 168 → 168 | 3184 → 3248 | 424/448 → 656/744 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float, TriMeshSDF<float, 4ul, 4…` | 168 → 168 | 13488 → 13552 | 1292/1084 → 1428/1172 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 19376 → 19120 | 1388/1372 → 1508/1376 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, TriMeshSDF<double, 4ul, 4ul> >, uns…` | 168 → 168 | 2096 → 2160 | 808/792 → 992/912 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float> >, unsigned int const, f…` | 168 → 168 | 13744 → 13424 | 1292/1084 → 1428/1172 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, TriMeshSDF<float, 4ul, 4ul> >, unsig…` | 168 → 168 | 1392 → 1392 | 96/100 → 108/116 | **spills** |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 17200 → 16960 | 174/220 → 174/220 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 12128 → 11840 | 114/132 → 114/132 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 80 → 80 | 17552 → 17328 | 290/464 → 290/464 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 80 → 80 | 12672 → 12352 | 126/132 → 126/132 | ~ |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 128 → 128 | 17376 → 17136 | 186/220 → 186/220 | ~ |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 128 → 128 | 12416 → 12096 | 114/132 → 114/132 | ~ |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> cons…` | 40 → 40 | 17808 → 17584 | 518/916 → 518/916 | ~ |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const, …` | 40 → 40 | 12928 → 12608 | 230/368 → 230/368 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17536 → 17072 | 174/220 → 174/220 | ~ |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 17200 → 16960 | 174/220 → 174/220 | ~ |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 12128 → 11840 | 114/132 → 114/132 | ~ |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 17072 → 16848 | 160/152 → 160/152 | ~ |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 12048 → 11728 | 132/132 → 132/132 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<double, Tape<double, TriMeshSDF<dou…` | 168 → 168 | 16736 → 16256 | 48/36 → 48/36 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<double, Tape<double> >, Vec3T<doubl…` | 168 → 168 | 16384 → 16160 | 48/36 → 48/36 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<float, Tape<float, TriMeshSDF<float…` | 168 → 168 | 11488 → 11472 | 40/28 → 40/28 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<float, Tape<float> >, Vec3T<float> …` | 168 → 168 | 11456 → 11392 | 40/28 → 40/28 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17536 → 17072 | 174/220 → 174/220 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 17200 → 16960 | 174/220 → 174/220 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11904 → 11840 | 114/132 → 114/132 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 19408 → 18912 | 246/208 → 246/208 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13200 → 13136 | 178/160 → 178/160 | ~ |
| `RayShell` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TapeFeature::ExactO…` | 168 → 168 | 19392 → 18912 | 236/224 → 236/224 | ~ |
| `RayShell` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TapeFeature::ExactOff…` | 168 → 168 | 13360 → 13344 | 172/176 → 172/176 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, false>, Vec3T<double> const, Reg…` | 168 → 168 | 17184 → 16960 | 174/196 → 174/196 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, true>, Vec3T<double> const, Regi…` | 168 → 168 | 17072 → 16832 | 8/8 → 8/8 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, false>, Vec3T<float> const, Regio…` | 168 → 168 | 12128 → 11840 | 114/120 → 114/120 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, true>, Vec3T<float> const, Region…` | 168 → 168 | 12240 → 11904 | 8/8 → 8/8 | ~ |
| `SphereCloudRays` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, SphereCloudBVH<doub…` | 168 → 168 | 17200 → 17184 | 126/104 → 126/104 | ~ |
| `SphereCloudRays` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, SphereCloudBVH<float,…` | 168 → 168 | 12176 → 12016 | 102/88 → 102/88 | ~ |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17536 → 17072 | 174/196 → 174/196 | ~ |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17536 → 17072 | 274/276 → 274/276 | ~ |


Flagged after 2 rounds, and judged on 10 (6 cases): LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice, LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice, Ray/csg-fused/firstHit-random/float/hostdevice, RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice, Regions/assembly/firstRegionChange/float/hostdevice, SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice

These probes compiled to the same code on both sides and were not run (122 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), Mesh (8), MeshClosestPoint (4), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Shapes (20), SphereCloudBVH (8), SphereContacts (2)

The probes differ between the sides (each side ran its own): Performance/MeshTape/main.cpp

<!-- END receipt cuda -->
