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
base: b4fb4c6
base-fingerprint: 6c3c437020716e933adeba2f122c30695cb6e811
head: 30779eb
head-fingerprint: 2c24312267f8151d9f0c33d3b07d8e117be57aa2
cases: 4
rounds: 2
reps: 3
date: 2026-11-20
-->

## Host comparison: green

**Accepted:** -

`30779eb` against `b4fb4c6` (origin/main), 2026-11-20.

**VERDICT: no regression (0 major, 0 minor across 4 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 4 cases

- base: ? (?, simd ?, gpu ?, allocator ?)
- head: 30779eb (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
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

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 750.8 s → 753.9 s (+0.4%), the largest peak 1419 MB → 1418 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.5 s | 9.9 s | +4.0% | 558 MB | 562 MB | +0.6% | ~ |
| `BVHRefit` | 4.3 s | 4.3 s | +0.8% | 497 MB | 499 MB | +0.5% | ~ |
| `BVHUnion` | 19.3 s | 20.0 s | +3.6% | 948 MB | 951 MB | +0.3% | ~ |
| `BVHUnionMixed` | 7.7 s | 7.7 s | -0.5% | 557 MB | 559 MB | +0.3% | ~ |
| `ClosestPrimitive` | 30.3 s | 30.8 s | +1.6% | 1300 MB | 1344 MB | +3.4% | ~ |
| `ClosestSurface` | 28.4 s | 28.7 s | +1.2% | 1298 MB | 1306 MB | +0.6% | ~ |
| `ConnectedComponents` | - | 15.7 s | | - | 844 MB | | new |
| `Domain` | 18.1 s | 18.4 s | +1.6% | 843 MB | 849 MB | +0.7% | ~ |
| `DomainObject` | 16.0 s | 16.6 s | +3.3% | 846 MB | 851 MB | +0.6% | ~ |
| `EBGrid` | 14.5 s | 15.0 s | +3.2% | 781 MB | 783 MB | +0.3% | ~ |
| `Fused` | 24.5 s | 25.4 s | +3.6% | 1208 MB | 1217 MB | +0.8% | ~ |
| `LaunchTuning` | 36.6 s | 37.1 s | +1.3% | 1279 MB | 1289 MB | +0.8% | ~ |
| `Mesh` | 16.1 s | 16.0 s | -0.7% | 786 MB | 789 MB | +0.3% | ~ |
| `MeshCSG` | 39.8 s | 40.7 s | +2.4% | 1258 MB | 1269 MB | +0.8% | ~ |
| `MeshClosestPoint` | 14.3 s | 14.6 s | +2.3% | 782 MB | 783 MB | +0.1% | ~ |
| `MeshIngest` | 7.0 s | 6.9 s | -1.0% | 569 MB | 568 MB | -0.2% | ~ |
| `MeshReconcile` | 7.0 s | 6.9 s | -0.5% | 569 MB | 568 MB | -0.1% | ~ |
| `MeshTape` | 42.2 s | 42.0 s | -0.5% | 1280 MB | 1272 MB | -0.6% | ~ |
| `PointCloud` | 7.4 s | 7.4 s | +0.4% | 560 MB | 560 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.9 s | 3.9 s | -1.2% | 472 MB | 473 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | -0.9% | 453 MB | 454 MB | +0.2% | ~ |
| `QueryOrder` | 11.4 s | 11.6 s | +1.9% | 692 MB | 696 MB | +0.6% | ~ |
| `Ray` | 37.1 s | 36.8 s | -0.9% | 1292 MB | 1307 MB | +1.2% | ~ |
| `RayCSG` | 32.5 s | 32.2 s | -1.0% | 1195 MB | 1214 MB | +1.7% | ~ |
| `RayCrossing` | 45.5 s | 45.7 s | +0.5% | 1344 MB | 1342 MB | -0.2% | ~ |
| `RayParticles` | 49.0 s | 49.5 s | +1.1% | 1332 MB | 1329 MB | -0.3% | ~ |
| `RayShell` | 38.2 s | 38.4 s | +0.4% | 1266 MB | 1276 MB | +0.8% | ~ |
| `Regions` | 29.1 s | 29.9 s | +2.7% | 1074 MB | 1079 MB | +0.5% | ~ |
| `Render` | 36.1 s | 36.0 s | -0.4% | 1273 MB | 1283 MB | +0.8% | ~ |
| `Shapes` | 9.0 s | 8.8 s | -1.5% | 555 MB | 556 MB | +0.1% | ~ |
| `SphereCloudBVH` | 5.0 s | 4.8 s | -3.6% | 512 MB | 514 MB | +0.4% | ~ |
| `SphereCloudRays` | 29.3 s | 28.1 s | -4.1% | 1065 MB | 1070 MB | +0.4% | ~ |
| `SphereContacts` | 4.2 s | 4.3 s | +1.5% | 486 MB | 487 MB | +0.3% | ~ |
| `SurfaceParts` | 42.2 s | 40.7 s | -3.7% | 1419 MB | 1418 MB | -0.0% | ~ |
| `Tape` | 17.1 s | 16.7 s | -2.1% | 830 MB | 827 MB | -0.4% | ~ |
| `TapeBVHUnion` | 14.8 s | 14.7 s | -0.8% | 826 MB | 830 MB | +0.4% | ~ |
| **all, in both** | 750.8 s | 753.9 s | +0.4% | 1419 MB | 1418 MB | -0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (328 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshCSG (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), MeshTape (16), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/ConnectedComponents/main.cpp

<!-- END receipt host -->
