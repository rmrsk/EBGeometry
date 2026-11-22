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

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: b4fb4c6
base-fingerprint: 6c3c437020716e933adeba2f122c30695cb6e811
head: ac23d11
head-fingerprint: f681457ef8967ca27f3282b4d88851ccd482abea
cases: 66
rounds: 2
reps: 3
date: 2026-11-21
-->

## Device comparison (CUDA): green

**Accepted:** -

`ac23d11` against `b4fb4c6` (origin/main), 2026-11-21.

**VERDICT: no regression (0 major, 0 minor across 66 cases)**

Performance comparison: 0 major, 0 minor, 3 faster, 0 differ, 0 lost, 66 cases

- base: b4fb4c6 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: ac23d11 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `Domain/particles/labels/float/hostdevice` | 4.00 ms | 4.03 ms | +0.6% | [+0.4%, +0.9%] | - | ~ | device 3.4x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.72 ms | 2.71 ms | -0.4% | [-0.6%, -0.3%] | - | ~ | device 2.7x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 122 µs | 114 µs | -6.9% | [-9.8%, -3.9%] | - | faster | device 13.0x faster than the host (was 12.3x) |
| `Domain/particles/labels/double/hostdevice` | 7.06 ms | 7.13 ms | +1.0% | [+0.6%, +1.4%] | - | ~ | device 2.0x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.60 ms | 3.61 ms | +0.1% | [-0.2%, +0.5%] | - | ~ | device 2.0x faster than the host |
| `Domain/particles/surfaceGroups/double/hostdevice` | 127 µs | 114 µs | -10.6% | [-11.0%, -10.1%] | - | faster | device 13.0x faster than the host (was 11.0x) |
| `DomainObject/partsWhole/labels/float/hostdevice` | 12.4 ms | 12.4 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 3.6x faster than the host |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.70 ms | 8.71 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 2.7x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 267 µs | 270 µs | +0.8% | [+0.1%, +1.5%] | - | ~ | device 2.3x faster than the host (was 2.4x); whole/own 46.06x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 151 µs | 151 µs | +0.1% | [-1.3%, +1.5%] | - | ~ | device 2.3x faster than the host; whole/own 57.79x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 20.0 ms | 20.0 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 2.3x faster than the host |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 11.7 ms | 11.7 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 2.1x faster than the host |
| `DomainObject/partsOwn/labels/double/hostdevice` | 384 µs | 382 µs | -0.4% | [-2.5%, +1.8%] | - | ~ | device 1.8x faster than the host; whole/own 52.35x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 234 µs | 235 µs | +0.4% | [-0.3%, +1.0%] | - | ~ | device 1.5x faster than the host (was 1.6x); whole/own 50.01x |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.00 ms | 2.00 ms | +0.2% | [-0.4%, +0.9%] | - | ~ | device 5.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.39 ms | 3.39 ms | -0.0% | [-0.3%, +0.3%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.12 ms | 6.13 ms | +0.2% | [+0.0%, +0.5%] | - | ~ | device 4.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.6 ms | 63.5 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 3.4x faster than the host (was 3.1x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 91.8 ms | 93.6 ms | +1.9% | [+1.1%, +2.8%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 240 ms | 239 ms | -0.2% | [-2.2%, +1.9%] | - | ~ | device 3.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.7 ms | 24.9 ms | +0.8% | [-0.2%, +1.9%] | - | ~ | device 2.0x faster than the host; tape/fused 2.55x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.7 ms | 26.8 ms | +0.6% | [-0.4%, +1.6%] | - | ~ | device 2.2x faster than the host; tape/fused 3.49x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.8 ms | 65.6 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 4.3x faster than the host (was 3.9x); tape/fused 3.64x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.0 ms | 20.4 ms | -2.8% | [-6.2%, +0.8%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 38.5 ms | 38.0 ms | -1.2% | [-1.6%, -0.9%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 40.8 ms | 40.9 ms | +0.2% | [-1.3%, +1.7%] | - | ~ | device 0.7x faster than the host (was 0.9x) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 538 ms | 527 ms | -2.1% | [-5.8%, +1.9%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 841 ms | 834 ms | -0.9% | [-1.5%, -0.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.94 s | 1.93 s | -0.7% | [-2.7%, +1.3%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 190 ms | 192 ms | +1.2% | [-1.8%, +4.4%] | - | ~ | device 0.3x faster than the host; tape/fused 2.74x (was 2.83x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 295 ms | 291 ms | -1.3% | [-3.3%, +0.6%] | - | ~ | device 0.2x faster than the host; tape/fused 2.86x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 560 ms | 535 ms | -4.4% | [-5.1%, -3.7%] | - | faster | device 0.5x faster than the host (was 0.5x); tape/fused 3.60x (was 3.46x) |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.29 ms | 1.30 ms | +0.5% | [+0.1%, +1.0%] | - | ~ | device 15.8x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.30 ms | 1.29 ms | -0.7% | [-1.5%, +0.0%] | - | ~ | device 15.8x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.15 ms | 2.15 ms | -0.2% | [-1.1%, +0.7%] | - | ~ | device 7.8x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.49 ms | 3.50 ms | +0.4% | [+0.3%, +0.5%] | - | ~ | device 10.1x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.75 ms | 2.75 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 7.8x faster than the host; mesh/tape 0.47x |
| `MeshTape/tape/frame/float/hostdevice` | 4.42 ms | 4.41 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 4.9x faster than the host; mesh/tape 0.49x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.73 ms | 3.73 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 9.7x faster than the host (was 11.0x); mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.58 ms | 5.63 ms | +0.9% | [+0.7%, +1.1%] | - | ~ | device 4.2x faster than the host; mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 28.5 ms | 28.4 ms | -0.5% | [-0.7%, -0.3%] | - | ~ | device 0.8x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 28.5 ms | 28.5 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 24.5 ms | 24.3 ms | -0.6% | [-1.1%, -0.0%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 41.0 ms | 40.9 ms | -0.4% | [-0.5%, -0.3%] | - | ~ | device 0.9x faster than the host; mesh/tape 0.69x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 28.8 ms | 28.8 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 31.3 ms | 31.4 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.77x |
| `MeshTape/view/firstHit/double/hostdevice` | 41.0 ms | 41.1 ms | +0.2% | [-0.2%, +0.7%] | - | ~ | device 0.9x faster than the host; mesh/view 0.69x |
| `MeshTape/view/frame/double/hostdevice` | 34.3 ms | 34.4 ms | +0.4% | [-1.0%, +1.8%] | - | ~ | device 0.7x faster than the host; mesh/view 0.71x |
| `Regions/particles/region/float/hostdevice` | 2.77 ms | 2.76 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 2.7x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.78 ms | 3.76 ms | -0.4% | [-0.8%, +0.0%] | - | ~ | device 3.4x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 89.6 ms | 89.4 ms | -0.2% | [-1.3%, +0.8%] | - | ~ | device 3.3x faster than the host (was 4.0x); one-pass/each 0.04x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.70 ms | 3.71 ms | +0.4% | [+0.2%, +0.5%] | - | ~ | device 3.6x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.70 ms | 6.70 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.55 ms | 5.54 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 3.2x faster than the host; regions/hit 1.21x |
| `Regions/particles/region/double/hostdevice` | 3.35 ms | 3.34 ms | -0.3% | [-0.8%, +0.3%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.91 ms | 5.87 ms | -0.7% | [-1.7%, +0.2%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 135 ms | 134 ms | -0.5% | [-1.2%, +0.2%] | - | ~ | device 2.2x faster than the host (was 2.5x); one-pass/each 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.90 ms | 5.92 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 2.3x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 15.8 ms | 15.7 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.9 ms | 12.9 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 1.4x faster than the host; regions/hit 1.22x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.37 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.36 ms | 2.36 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.14 ms | 2.14 ms | +0.3% | [-1.0%, +1.7%] | - | ~ | device 7.8x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.61 ms | 5.61 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 1.5x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.61 ms | 5.62 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 24.5 ms | 24.4 ms | -0.4% | [-1.0%, +0.2%] | - | ~ | device 0.7x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 2 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4554.8 s → 4634.3 s (+1.7%), the largest peak 6566 MB → 6569 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 11.8 s | 11.8 s | -0.1% | 605 MB | 609 MB | +0.7% | ~ |
| `BVHRefit` | 6.1 s | 6.1 s | -0.0% | 509 MB | 511 MB | +0.4% | ~ |
| `BVHUnion` | 22.2 s | 22.1 s | -0.1% | 1027 MB | 1032 MB | +0.4% | ~ |
| `BVHUnionMixed` | 9.8 s | 9.8 s | +0.1% | 623 MB | 627 MB | +0.6% | ~ |
| `ClosestPrimitive` | 101.2 s | 101.1 s | -0.1% | 1353 MB | 1357 MB | +0.3% | ~ |
| `ClosestSurface` | 142.7 s | 142.6 s | -0.1% | 1843 MB | 1844 MB | +0.1% | ~ |
| `Domain` | 47.2 s | 47.1 s | -0.0% | 991 MB | 995 MB | +0.4% | ~ |
| `DomainObject` | 43.8 s | 43.7 s | -0.2% | 944 MB | 949 MB | +0.5% | ~ |
| `EBGrid` | 17.2 s | 17.1 s | -0.1% | 753 MB | 757 MB | +0.5% | ~ |
| `Fused` | 43.8 s | 43.7 s | -0.3% | 1163 MB | 1168 MB | +0.4% | ~ |
| `LaunchTuning` | 410.2 s | 409.4 s | -0.2% | 3876 MB | 3876 MB | +0.0% | ~ |
| `Mesh` | 18.0 s | 18.0 s | +0.2% | 810 MB | 814 MB | +0.5% | ~ |
| `MeshCSG` | 321.6 s | 321.5 s | -0.0% | 6303 MB | 6303 MB | -0.0% | ~ |
| `MeshClosestPoint` | 16.5 s | 16.4 s | -0.4% | 762 MB | 767 MB | +0.6% | ~ |
| `MeshIngest` | 8.4 s | 8.5 s | +0.5% | 535 MB | 539 MB | +0.8% | ~ |
| `MeshReconcile` | 8.3 s | 8.3 s | -0.2% | 512 MB | 517 MB | +0.8% | ~ |
| `MeshTape` | 612.9 s | 612.8 s | -0.0% | 6566 MB | 6569 MB | +0.0% | ~ |
| `PointCloud` | 10.2 s | 10.2 s | -0.2% | 564 MB | 569 MB | +0.8% | ~ |
| `PointCloudBVHBuild` | 5.7 s | 5.7 s | -0.4% | 509 MB | 512 MB | +0.4% | ~ |
| `PointCloudHashGridBuild` | 5.2 s | 5.2 s | -0.6% | 509 MB | 512 MB | +0.4% | ~ |
| `QueryOrder` | 13.2 s | 13.2 s | -0.0% | 694 MB | 698 MB | +0.6% | ~ |
| `Ray` | 147.5 s | 147.6 s | +0.1% | 3769 MB | 3770 MB | +0.0% | ~ |
| `RayCSG` | 169.5 s | 169.7 s | +0.1% | 4155 MB | 4155 MB | +0.0% | ~ |
| `RayCrossing` | 329.6 s | 330.9 s | +0.4% | 4116 MB | 4116 MB | +0.0% | ~ |
| `RayParticles` | 601.7 s | 652.6 s | +8.5% | 5808 MB | 5808 MB | -0.0% | ~ |
| `RayShell` | 322.9 s | 350.8 s | +8.7% | 5501 MB | 5501 MB | -0.0% | ~ |
| `Regions` | 277.9 s | 278.6 s | +0.2% | 4139 MB | 4138 MB | -0.0% | ~ |
| `Render` | 201.9 s | 202.5 s | +0.3% | 6082 MB | 6083 MB | +0.0% | ~ |
| `Shapes` | 19.0 s | 20.1 s | +5.7% | 585 MB | 589 MB | +0.7% | ~ |
| `SphereCloudBVH` | 6.9 s | 6.7 s | -2.8% | 510 MB | 512 MB | +0.4% | ~ |
| `SphereCloudRays` | 143.2 s | 144.3 s | +0.7% | 4170 MB | 4169 MB | -0.0% | ~ |
| `SphereContacts` | 6.7 s | 6.5 s | -3.1% | 509 MB | 511 MB | +0.4% | ~ |
| `SurfaceParts` | 367.4 s | 365.6 s | -0.5% | 4416 MB | 4416 MB | -0.0% | ~ |
| `Tape` | 49.5 s | 49.6 s | +0.2% | 978 MB | 982 MB | +0.4% | ~ |
| `TapeBVHUnion` | 35.1 s | 34.4 s | -1.9% | 918 MB | 923 MB | +0.5% | ~ |
| **all, in both** | 4554.8 s | 4634.3 s | +1.7% | 6566 MB | 6569 MB | +0.0% | ~ |

Device kernels: 226 in both, 2 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, RegionView<Tape<double, TriMeshSDF<…` | 168 → 168 | 20400 → 20464 | 2788/2364 → 2788/2364 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, RegionView<Tape<float, TriMeshSDF<fl…` | 168 → 168 | 14704 → 14768 | 2896/2116 → 2896/2116 | ~ |


These probes compiled to the same code on both sides and were not run (348 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), EBGrid (12), Fused (10), LaunchTuning (24), Mesh (8), MeshClosestPoint (4), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), QueryOrder (6), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
