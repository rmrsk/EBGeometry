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
head: 01732a2
head-fingerprint: 93eb5c20b0e3603ca6320e8f17ea7de57c54fcbf
cases: 4
rounds: 2
reps: 3
date: 2026-11-20
-->

## Host comparison: green

**Accepted:** -

`01732a2` against `1429072` (origin/main), 2026-11-20.

**VERDICT: no regression (0 major, 0 minor across 4 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 4 cases

- base: ? (?, simd ?, gpu ?, allocator ?)
- head: 01732a2 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `ConnectedComponents/shells/build-coarse/float/host` | - | - | - | - | - | new |  |
| `ConnectedComponents/shells/build-fine/float/host` | - | - | - | - | - | new |  |
| `ConnectedComponents/shells/build-coarse/double/host` | - | - | - | - | - | new |  |
| `ConnectedComponents/shells/build-fine/double/host` | - | - | - | - | - | new |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 750.8 s → 847.2 s (+12.8%), the largest peak 1419 MB → 1418 MB; 14 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.5 s | 12.3 s | +28.7% | 558 MB | 562 MB | +0.7% | **MAJOR** |
| `BVHRefit` | 4.3 s | 5.5 s | +29.2% | 497 MB | 499 MB | +0.4% | ~ |
| `BVHUnion` | 19.3 s | 26.6 s | +37.7% | 948 MB | 950 MB | +0.3% | **MAJOR** |
| `BVHUnionMixed` | 7.7 s | 9.8 s | +26.5% | 557 MB | 559 MB | +0.3% | **MAJOR** |
| `ClosestPrimitive` | 30.3 s | 42.9 s | +41.4% | 1300 MB | 1344 MB | +3.4% | **MAJOR** |
| `ClosestSurface` | 28.4 s | 38.6 s | +35.9% | 1298 MB | 1304 MB | +0.5% | **MAJOR** |
| `ConnectedComponents` | - | 21.0 s | | - | 844 MB | | new |
| `Domain` | 18.1 s | 23.3 s | +28.7% | 843 MB | 848 MB | +0.6% | **MAJOR** |
| `DomainObject` | 16.0 s | 22.2 s | +38.5% | 846 MB | 852 MB | +0.6% | **MAJOR** |
| `EBGrid` | 14.5 s | 22.2 s | +52.8% | 781 MB | 782 MB | +0.2% | **MAJOR** |
| `Fused` | 24.5 s | 35.8 s | +46.1% | 1208 MB | 1217 MB | +0.8% | **MAJOR** |
| `LaunchTuning` | 36.6 s | 45.0 s | +23.0% | 1279 MB | 1289 MB | +0.8% | **minor** |
| `Mesh` | 16.1 s | 22.5 s | +40.2% | 786 MB | 788 MB | +0.2% | **MAJOR** |
| `MeshCSG` | 39.8 s | 44.9 s | +13.0% | 1258 MB | 1268 MB | +0.8% | **minor** |
| `MeshClosestPoint` | 14.3 s | 19.8 s | +39.0% | 782 MB | 783 MB | +0.1% | **MAJOR** |
| `MeshIngest` | 7.0 s | 11.0 s | +57.4% | 569 MB | 567 MB | -0.3% | **MAJOR** |
| `MeshReconcile` | 7.0 s | 8.7 s | +24.2% | 569 MB | 568 MB | -0.2% | ~ |
| `MeshTape` | 42.2 s | 42.6 s | +0.9% | 1280 MB | 1272 MB | -0.6% | ~ |
| `PointCloud` | 7.4 s | 7.5 s | +1.2% | 560 MB | 560 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.9 s | 3.9 s | -0.2% | 472 MB | 473 MB | +0.3% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | -0.7% | 453 MB | 454 MB | +0.2% | ~ |
| `QueryOrder` | 11.4 s | 11.7 s | +3.0% | 692 MB | 697 MB | +0.6% | ~ |
| `Ray` | 37.1 s | 37.1 s | +0.0% | 1292 MB | 1307 MB | +1.1% | ~ |
| `RayCSG` | 32.5 s | 32.4 s | -0.4% | 1195 MB | 1214 MB | +1.6% | ~ |
| `RayCrossing` | 45.5 s | 46.0 s | +1.1% | 1344 MB | 1342 MB | -0.2% | ~ |
| `RayParticles` | 49.0 s | 49.3 s | +0.6% | 1332 MB | 1329 MB | -0.2% | ~ |
| `RayShell` | 38.2 s | 38.4 s | +0.6% | 1266 MB | 1276 MB | +0.8% | ~ |
| `Regions` | 29.1 s | 29.8 s | +2.3% | 1074 MB | 1080 MB | +0.5% | ~ |
| `Render` | 36.1 s | 36.0 s | -0.4% | 1273 MB | 1283 MB | +0.8% | ~ |
| `Shapes` | 9.0 s | 8.7 s | -2.7% | 555 MB | 556 MB | +0.1% | ~ |
| `SphereCloudBVH` | 5.0 s | 4.8 s | -3.1% | 512 MB | 513 MB | +0.3% | ~ |
| `SphereCloudRays` | 29.3 s | 27.9 s | -4.7% | 1065 MB | 1070 MB | +0.4% | ~ |
| `SphereContacts` | 4.2 s | 4.3 s | +1.3% | 486 MB | 487 MB | +0.2% | ~ |
| `SurfaceParts` | 42.2 s | 40.6 s | -3.8% | 1419 MB | 1418 MB | -0.0% | ~ |
| `Tape` | 17.1 s | 16.8 s | -1.4% | 830 MB | 826 MB | -0.5% | ~ |
| `TapeBVHUnion` | 14.8 s | 14.7 s | -0.8% | 826 MB | 830 MB | +0.4% | ~ |
| **all, in both** | 750.8 s | 847.2 s | +12.8% | 1419 MB | 1418 MB | -0.0% | **minor** |


These probes compiled to the same code on both sides and were not run (328 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshCSG (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), MeshTape (16), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/ConnectedComponents/main.cpp

<!-- END receipt host -->
