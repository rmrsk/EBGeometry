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
base: fd5c8a5
base-fingerprint: 263791791f122733f874f9739b94c83cd96178ea
head: 830ef35
head-fingerprint: 0c7b55bba42a834e0c2d2bb9490ef981d2554d68
cases: 48
rounds: 2
reps: 3
date: 2026-11-19
-->

## Host comparison: green

**Accepted:** -

`830ef35` against `fd5c8a5` (origin/main), 2026-11-19.

**VERDICT: no regression (0 major, 0 minor across 48 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 48 cases

- base: fd5c8a5 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 830ef35 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `DomainObject/partsWhole/labels/float/host` | 41.3 ms | 41.6 ms | +0.7% | [-5.8%, +7.6%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 22.2 ms | 22.3 ms | +0.5% | [-4.4%, +5.7%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 586 µs | 583 µs | -0.6% | [-8.8%, +8.2%] | +0.0% | ~ | whole/own 71.33x |
| `DomainObject/partsOwn/labelAt/float/host` | 316 µs | 318 µs | +0.6% | [-4.9%, +6.6%] | +0.0% | ~ | whole/own 70.35x |
| `DomainObject/partsWhole/labels/double/host` | 41.3 ms | 42.7 ms | +3.5% | [-7.7%, +15.6%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 24.2 ms | 30.3 ms | +25.1% | [-12.9%, +69.1%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 620 µs | 620 µs | +0.0% | [-9.9%, +10.9%] | +0.0% | ~ | whole/own 68.85x (was 66.54x) |
| `DomainObject/partsOwn/labelAt/double/host` | 335 µs | 334 µs | -0.3% | [-10.7%, +11.0%] | +0.0% | ~ | whole/own 90.53x (was 72.15x) |
| `MeshClosestPoint/tri/signedDistance/float/host` | 262 ms | 270 ms | +3.2% | [-19.5%, +31.8%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 271 ms | 277 ms | +2.0% | [-20.9%, +31.3%] | +0.0% | ~ | signedDistance/closestPoint 0.98x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 443 ms | 431 ms | -2.6% | [-42.2%, +69.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 254 ms | 257 ms | +1.1% | [-13.3%, +17.8%] | +0.0% | ~ | closestPoint/gradient 1.08x |
| `MeshClosestPoint/tri/signedDistance/double/host` | 307 ms | 304 ms | -1.0% | [-22.0%, +26.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 298 ms | 294 ms | -1.3% | [-18.5%, +19.8%] | +0.0% | ~ | signedDistance/closestPoint 1.03x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 407 ms | 377 ms | -7.4% | [-25.2%, +16.7%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 278 ms | 279 ms | +0.3% | [-6.6%, +7.7%] | +0.0% | ~ | closestPoint/gradient 1.05x |
| `MeshTape/mesh/firstHit/float/host` | 48.1 ms | 47.8 ms | -0.6% | [-4.8%, +3.7%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 47.6 ms | 47.6 ms | -0.0% | [-5.3%, +5.5%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 20.0 ms | 19.5 ms | -2.2% | [-4.7%, +0.3%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 79.9 ms | 80.0 ms | +0.2% | [-4.8%, +5.3%] | +0.0% | ~ | mesh/tape 0.60x |
| `MeshTape/tape/firstCrossing/float/host` | 49.8 ms | 50.4 ms | +1.1% | [-3.5%, +6.1%] | +0.0% | ~ | mesh/tape 0.94x |
| `MeshTape/tape/frame/float/host` | 24.4 ms | 24.8 ms | +1.7% | [-4.5%, +8.3%] | +0.0% | ~ | mesh/tape 0.79x (was 0.82x) |
| `MeshTape/view/firstHit/float/host` | 82.9 ms | 82.6 ms | -0.3% | [-5.4%, +5.0%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/float/host` | 26.3 ms | 26.6 ms | +1.1% | [-4.8%, +7.3%] | +0.0% | ~ | mesh/view 0.73x (was 0.76x) |
| `MeshTape/mesh/firstHit/double/host` | 54.5 ms | 54.7 ms | +0.3% | [-6.6%, +7.8%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 54.7 ms | 56.3 ms | +2.8% | [-6.8%, +13.2%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.7 ms | 21.3 ms | +2.6% | [-0.8%, +6.2%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 91.4 ms | 139 ms | +52.0% | [-9.8%, +119.8%] | +0.0% | ~ | mesh/tape 0.39x (was 0.60x) |
| `MeshTape/tape/firstCrossing/double/host` | 91.3 ms | 88.4 ms | -3.2% | [-56.7%, +120.2%] | +0.0% | ~ | mesh/tape 0.64x (was 0.60x) |
| `MeshTape/tape/frame/double/host` | 40.5 ms | 41.1 ms | +1.4% | [-51.0%, +109.9%] | +0.0% | ~ | mesh/tape 0.52x |
| `MeshTape/view/firstHit/double/host` | 143 ms | 145 ms | +1.2% | [-54.9%, +125.9%] | +0.0% | ~ | mesh/view 0.38x |
| `MeshTape/view/frame/double/host` | 44.2 ms | 29.4 ms | -33.6% | [-51.6%, +3.6%] | +0.0% | ~ | mesh/view 0.72x (was 0.47x) |
| `Regions/particles/region/float/host` | 7.32 ms | 7.35 ms | +0.3% | [+0.0%, +0.6%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.7 ms | +0.8% | [-0.5%, +2.2%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 269 ms | 268 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.6 ms | 12.6 ms | +0.5% | [-1.0%, +1.9%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.7 ms | 30.5 ms | -0.5% | [-2.0%, +1.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.2 ms | 22.3 ms | +0.5% | [-0.6%, +1.6%] | +0.0% | ~ | regions/hit 1.37x |
| `Regions/particles/region/double/host` | 7.66 ms | 7.64 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 13.0 ms | 13.0 ms | +0.7% | [-0.1%, +1.4%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 283 ms | 283 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.1 ms | 13.1 ms | -0.1% | [-1.2%, +0.9%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.5 ms | 32.3 ms | -0.8% | [-1.3%, -0.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.7 ms | 23.7 ms | +0.1% | [-0.4%, +1.6%] | +0.0% | ~ | regions/hit 1.36x |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 42.0 ms | 42.0 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 201 ms | 201 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 42.9 ms | 43.7 ms | +1.9% | [-1.3%, +5.2%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 218 ms | 218 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 791.5 s → 744.2 s (-6.0%), the largest peak 1409 MB → 1408 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 10.2 s | 9.7 s | -4.8% | 558 MB | 558 MB | -0.1% | ~ |
| `BVHRefit` | 4.6 s | 4.3 s | -6.7% | 497 MB | 497 MB | +0.0% | ~ |
| `BVHUnion` | 20.6 s | 19.2 s | -6.4% | 947 MB | 948 MB | +0.0% | ~ |
| `BVHUnionMixed` | 8.4 s | 7.7 s | -8.4% | 555 MB | 556 MB | +0.0% | ~ |
| `ClosestPrimitive` | 32.0 s | 30.3 s | -5.3% | 1297 MB | 1297 MB | +0.0% | ~ |
| `ClosestSurface` | 30.0 s | 28.4 s | -5.4% | 1228 MB | 1228 MB | -0.0% | ~ |
| `Domain` | 19.2 s | 18.1 s | -6.1% | 842 MB | 842 MB | +0.0% | ~ |
| `DomainObject` | 17.3 s | 15.8 s | -8.4% | 846 MB | 846 MB | +0.0% | ~ |
| `Fused` | 26.2 s | 24.5 s | -6.4% | 1136 MB | 1136 MB | +0.0% | ~ |
| `LaunchTuning` | 39.4 s | 36.6 s | -7.1% | 1311 MB | 1311 MB | +0.0% | ~ |
| `Mesh` | 16.5 s | 15.5 s | -6.1% | 784 MB | 784 MB | +0.1% | ~ |
| `MeshCSG` | 43.5 s | 40.1 s | -7.9% | 1255 MB | 1254 MB | -0.0% | ~ |
| `MeshClosestPoint` | 14.7 s | 14.4 s | -2.3% | 781 MB | 781 MB | -0.1% | ~ |
| `MeshReconcile` | 6.9 s | 6.7 s | -1.9% | 566 MB | 566 MB | +0.0% | ~ |
| `MeshTape` | 46.6 s | 43.1 s | -7.4% | 1287 MB | 1287 MB | +0.0% | ~ |
| `PointCloud` | 7.6 s | 7.5 s | -1.2% | 558 MB | 558 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.9 s | 3.7 s | -4.2% | 470 MB | 471 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 3.4 s | 3.3 s | -2.9% | 452 MB | 452 MB | +0.1% | ~ |
| `Ray` | 40.9 s | 38.2 s | -6.6% | 1318 MB | 1318 MB | +0.0% | ~ |
| `RayCSG` | 37.1 s | 34.1 s | -8.3% | 1188 MB | 1187 MB | -0.1% | ~ |
| `RayCrossing` | 51.2 s | 48.6 s | -5.1% | 1330 MB | 1330 MB | -0.0% | ~ |
| `RayParticles` | 55.6 s | 51.9 s | -6.7% | 1319 MB | 1318 MB | -0.1% | ~ |
| `RayShell` | 42.2 s | 39.7 s | -6.0% | 1285 MB | 1284 MB | -0.1% | ~ |
| `Regions` | 33.4 s | 31.5 s | -5.7% | 1073 MB | 1072 MB | -0.1% | ~ |
| `Render` | 42.0 s | 38.9 s | -7.3% | 1284 MB | 1284 MB | +0.1% | ~ |
| `Shapes` | 9.4 s | 9.6 s | +1.8% | 553 MB | 553 MB | -0.0% | ~ |
| `SphereCloudBVH` | 5.1 s | 5.2 s | +2.4% | 511 MB | 510 MB | -0.2% | ~ |
| `SphereCloudRays` | 32.9 s | 30.9 s | -6.0% | 1064 MB | 1062 MB | -0.1% | ~ |
| `SphereContacts` | 4.7 s | 4.6 s | -2.0% | 485 MB | 483 MB | -0.3% | ~ |
| `SurfaceParts` | 47.2 s | 47.1 s | -0.2% | 1409 MB | 1408 MB | -0.1% | ~ |
| `Tape` | 20.6 s | 18.6 s | -10.0% | 831 MB | 829 MB | -0.2% | ~ |
| `TapeBVHUnion` | 18.3 s | 16.4 s | -10.3% | 818 MB | 817 MB | -0.1% | ~ |
| **all, in both** | 791.5 s | 744.2 s | -6.0% | 1409 MB | 1408 MB | -0.1% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (1 cases): Regions/assembly/firstHit/double/host

These probes compiled to the same code on both sides and were not run (254 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20)

The probes differ between the sides (each side ran its own): Performance/DomainObject/main.cpp, Performance/MeshClosestPoint/main.cpp, Performance/MeshTape/main.cpp, Performance/Regions/main.cpp, Performance/TapeBVHUnion/main.cpp, Performance/Common/Case.hpp

<!-- END receipt host -->
