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
base: 1429072
base-fingerprint: 6c3c437020716e933adeba2f122c30695cb6e811
head: a8e0b27
head-fingerprint: f681457ef8967ca27f3282b4d88851ccd482abea
cases: 60
rounds: 2
reps: 3
date: 2026-11-20
-->

## Host comparison: green

**Accepted:** -

`a8e0b27` against `1429072` (1429072), 2026-11-20.

**VERDICT: no regression (0 major, 0 minor across 60 cases)**

Performance comparison: 0 major, 0 minor, 2 faster, 0 differ, 0 lost, 60 cases

- base: 1429072 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: a8e0b27 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `Domain/particles/labels/float/host` | 14.0 ms | 13.8 ms | -1.3% | [-2.9%, +0.4%] | +1.1% | ~ |  |
| `Domain/particles/labelAt/float/host` | 6.93 ms | 6.94 ms | +0.1% | [-0.6%, +0.9%] | +1.1% | ~ |  |
| `Domain/particles/surfaceGroups/float/host` | 1.38 ms | 1.33 ms | -4.0% | [-5.0%, -3.0%] | +1.1% | faster |  |
| `Domain/particles/labels/double/host` | 14.4 ms | 14.5 ms | +0.5% | [+0.1%, +0.9%] | +0.9% | ~ |  |
| `Domain/particles/labelAt/double/host` | 7.31 ms | 7.29 ms | -0.3% | [-0.8%, +0.3%] | +0.9% | ~ |  |
| `Domain/particles/surfaceGroups/double/host` | 1.38 ms | 1.34 ms | -3.1% | [-3.5%, -2.6%] | +0.9% | faster |  |
| `DomainObject/partsWhole/labels/float/host` | 41.0 ms | 41.0 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/float/host` | 21.9 ms | 21.9 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/float/host` | 579 µs | 579 µs | +0.1% | [-0.1%, +0.4%] | +0.0% | ~ | whole/own 70.82x |
| `DomainObject/partsOwn/labelAt/float/host` | 309 µs | 309 µs | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | whole/own 71.04x |
| `DomainObject/partsWhole/labels/double/host` | 40.8 ms | 40.8 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `DomainObject/partsWhole/labelAt/double/host` | 23.3 ms | 23.3 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `DomainObject/partsOwn/labels/double/host` | 620 µs | 619 µs | -0.2% | [-1.8%, +1.4%] | +0.0% | ~ | whole/own 65.99x |
| `DomainObject/partsOwn/labelAt/double/host` | 322 µs | 322 µs | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ | whole/own 72.38x |
| `MeshCSG/union-tape/firstHit/float/host` | 28.7 ms | 28.6 ms | -0.1% | [-1.5%, +1.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 16.8 ms | 16.6 ms | -1.5% | [-3.6%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 364 ms | 365 ms | +0.4% | [-0.3%, +1.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 243 ms | 243 ms | -0.1% | [-1.5%, +1.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 96.6 ms | 96.4 ms | -0.2% | [-2.5%, +2.1%] | +0.0% | ~ | tape/fused 3.79x |
| `MeshCSG/smooth-fused/frame/float/host` | 85.1 ms | 85.1 ms | +0.0% | [-2.5%, +2.5%] | +0.0% | ~ | tape/fused 2.85x |
| `MeshCSG/union-tape/firstHit/double/host` | 31.7 ms | 32.4 ms | +2.5% | [-1.1%, +6.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 18.3 ms | 18.6 ms | +1.6% | [+0.5%, +2.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 427 ms | 421 ms | -1.3% | [-3.7%, +1.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 261 ms | 261 ms | -0.0% | [-1.3%, +1.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 105 ms | 106 ms | +0.9% | [-0.7%, +2.5%] | +0.0% | ~ | tape/fused 3.97x |
| `MeshCSG/smooth-fused/frame/double/host` | 91.5 ms | 91.8 ms | +0.3% | [-0.7%, +1.3%] | +0.0% | ~ | tape/fused 2.84x |
| `MeshTape/mesh/firstHit/float/host` | 55.1 ms | 54.4 ms | -1.4% | [-1.7%, -1.1%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 55.1 ms | 54.9 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 21.3 ms | 21.0 ms | -1.2% | [-2.1%, -0.3%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 88.8 ms | 89.5 ms | +0.7% | [-0.1%, +1.5%] | +0.0% | ~ | mesh/tape 0.61x |
| `MeshTape/tape/firstCrossing/float/host` | 57.3 ms | 57.1 ms | -0.4% | [-3.4%, +2.8%] | +0.0% | ~ | mesh/tape 0.96x |
| `MeshTape/tape/frame/float/host` | 26.5 ms | 26.6 ms | +0.2% | [-3.4%, +3.9%] | +0.0% | ~ | mesh/tape 0.79x |
| `MeshTape/view/firstHit/float/host` | 91.7 ms | 92.3 ms | +0.6% | [+0.0%, +1.1%] | +0.0% | ~ | mesh/view 0.59x |
| `MeshTape/view/frame/float/host` | 27.8 ms | 28.0 ms | +0.6% | [-0.0%, +1.2%] | +0.0% | ~ | mesh/view 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 61.3 ms | 61.2 ms | -0.2% | [-1.9%, +1.6%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 61.8 ms | 62.0 ms | +0.3% | [-1.4%, +2.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 22.0 ms | 21.9 ms | -0.5% | [-1.9%, +0.8%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 97.8 ms | 97.8 ms | -0.1% | [-1.8%, +1.7%] | +0.0% | ~ | mesh/tape 0.63x |
| `MeshTape/tape/firstCrossing/double/host` | 63.8 ms | 63.2 ms | -1.0% | [-1.5%, -0.4%] | +0.0% | ~ | mesh/tape 0.98x |
| `MeshTape/tape/frame/double/host` | 28.7 ms | 28.9 ms | +0.5% | [-2.5%, +3.7%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 101 ms | 101 ms | +0.5% | [-0.6%, +1.7%] | +0.0% | ~ | mesh/view 0.60x |
| `MeshTape/view/frame/double/host` | 30.6 ms | 31.2 ms | +1.9% | [+0.3%, +3.5%] | +0.0% | ~ | mesh/view 0.70x |
| `Regions/particles/region/float/host` | 7.50 ms | 7.49 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.8 ms | 12.8 ms | +0.5% | [+0.1%, +0.9%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 275 ms | 276 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.7 ms | 12.7 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 31.3 ms | 31.4 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.7 ms | 22.7 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ | regions/hit 1.38x |
| `Regions/particles/region/double/host` | 7.82 ms | 7.81 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 13.2 ms | 13.2 ms | +0.1% | [+0.0%, +0.3%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 294 ms | 293 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ | one-pass/each 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.3 ms | 13.3 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 33.1 ms | 33.3 ms | +0.6% | [+0.5%, +0.7%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 24.5 ms | 24.6 ms | +0.5% | [+0.0%, +1.0%] | +0.0% | ~ | regions/hit 1.36x |
| `Render/csg/frame/float/host` | 13.5 ms | 13.0 ms | -3.7% | [-7.7%, +0.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 13.3 ms | 13.4 ms | +1.2% | [-1.1%, +3.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 21.2 ms | 21.2 ms | +0.1% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.7 ms | 14.3 ms | -2.6% | [-4.9%, -0.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.1 ms | 14.1 ms | -0.3% | [-0.7%, +0.0%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 22.1 ms | 22.0 ms | -0.4% | [-1.9%, +1.0%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 6 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 783.4 s → 784.3 s (+0.1%), the largest peak 1418 MB → 1418 MB; 7 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.8 s | 10.7 s | -16.0% | 558 MB | 561 MB | +0.5% | ~ |
| `BVHRefit` | 5.2 s | 4.4 s | -15.7% | 497 MB | 498 MB | +0.3% | ~ |
| `BVHUnion` | 25.7 s | 20.4 s | -20.9% | 948 MB | 950 MB | +0.3% | ~ |
| `BVHUnionMixed` | 9.6 s | 7.8 s | -18.5% | 556 MB | 558 MB | +0.3% | ~ |
| `ClosestPrimitive` | 35.7 s | 33.5 s | -6.2% | 1299 MB | 1304 MB | +0.4% | ~ |
| `ClosestSurface` | 33.4 s | 30.0 s | -10.3% | 1297 MB | 1300 MB | +0.2% | ~ |
| `Domain` | 22.3 s | 19.4 s | -13.0% | 843 MB | 848 MB | +0.6% | ~ |
| `DomainObject` | 19.8 s | 17.3 s | -12.8% | 846 MB | 851 MB | +0.7% | ~ |
| `EBGrid` | 14.7 s | 15.1 s | +2.7% | 780 MB | 782 MB | +0.2% | ~ |
| `Fused` | 23.1 s | 25.5 s | +10.3% | 1208 MB | 1212 MB | +0.4% | **minor** |
| `LaunchTuning` | 34.6 s | 38.0 s | +9.9% | 1279 MB | 1287 MB | +0.6% | ~ |
| `Mesh` | 14.9 s | 16.8 s | +13.1% | 786 MB | 788 MB | +0.3% | ~ |
| `MeshCSG` | 36.8 s | 41.5 s | +12.6% | 1258 MB | 1265 MB | +0.5% | **minor** |
| `MeshClosestPoint` | 13.4 s | 14.7 s | +9.7% | 782 MB | 784 MB | +0.2% | ~ |
| `MeshIngest` | 6.6 s | 7.2 s | +10.2% | 568 MB | 567 MB | -0.2% | ~ |
| `MeshReconcile` | 6.4 s | 7.0 s | +10.8% | 568 MB | 569 MB | +0.1% | ~ |
| `MeshTape` | 39.1 s | 43.3 s | +10.6% | 1279 MB | 1281 MB | +0.1% | **minor** |
| `PointCloud` | 6.9 s | 7.6 s | +10.7% | 560 MB | 560 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 3.5 s | 3.9 s | +9.7% | 472 MB | 473 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 3.0 s | 3.3 s | +8.5% | 452 MB | 454 MB | +0.3% | ~ |
| `QueryOrder` | 10.6 s | 11.5 s | +7.8% | 692 MB | 695 MB | +0.4% | ~ |
| `Ray` | 34.1 s | 37.9 s | +11.1% | 1292 MB | 1306 MB | +1.1% | **minor** |
| `RayCSG` | 30.0 s | 33.3 s | +11.0% | 1194 MB | 1197 MB | +0.3% | **minor** |
| `RayCrossing` | 42.4 s | 47.1 s | +11.1% | 1344 MB | 1346 MB | +0.2% | **minor** |
| `RayParticles` | 45.5 s | 49.8 s | +9.6% | 1332 MB | 1335 MB | +0.2% | ~ |
| `RayShell` | 37.1 s | 38.4 s | +3.5% | 1266 MB | 1274 MB | +0.7% | ~ |
| `Regions` | 28.0 s | 34.7 s | +23.8% | 1074 MB | 1078 MB | +0.4% | **minor** |
| `Render` | 38.4 s | 37.4 s | -2.4% | 1273 MB | 1281 MB | +0.6% | ~ |
| `Shapes` | 9.0 s | 8.9 s | -1.1% | 555 MB | 556 MB | +0.2% | ~ |
| `SphereCloudBVH` | 5.5 s | 5.0 s | -8.3% | 510 MB | 513 MB | +0.5% | ~ |
| `SphereCloudRays` | 37.5 s | 29.1 s | -22.3% | 1065 MB | 1069 MB | +0.4% | ~ |
| `SphereContacts` | 4.8 s | 4.4 s | -7.7% | 484 MB | 486 MB | +0.4% | ~ |
| `SurfaceParts` | 50.5 s | 43.9 s | -13.0% | 1418 MB | 1418 MB | -0.0% | ~ |
| `Tape` | 22.3 s | 17.2 s | -22.6% | 830 MB | 827 MB | -0.3% | ~ |
| `TapeBVHUnion` | 20.1 s | 18.1 s | -10.1% | 825 MB | 829 MB | +0.6% | ~ |
| **all, in both** | 783.4 s | 784.3 s | +0.1% | 1418 MB | 1418 MB | -0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (268 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), EBGrid (12), Fused (10), Mesh (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
