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
head: 4e65f47
head-fingerprint: 76cdf04a08ce9fd6332a9f605de6f0de83148ff2
cases: 0
rounds: 2
reps: 3
date: 2026-11-19
-->

## Host comparison: green

**Accepted:** -

`4e65f47` against `fd5c8a5` (origin/main), 2026-11-19.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 835.5 s → 717.2 s (-14.2%), the largest peak 1409 MB → 1410 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.1 s | 9.5 s | -21.0% | 558 MB | 558 MB | +0.0% | ~ |
| `BVHRefit` | 6.0 s | 4.3 s | -28.6% | 496 MB | 497 MB | +0.1% | ~ |
| `BVHUnion` | 23.5 s | 19.4 s | -17.2% | 948 MB | 948 MB | +0.0% | ~ |
| `BVHUnionMixed` | 9.8 s | 7.7 s | -21.2% | 555 MB | 555 MB | +0.1% | ~ |
| `ClosestPrimitive` | 35.4 s | 30.7 s | -13.4% | 1297 MB | 1297 MB | +0.0% | ~ |
| `ClosestSurface` | 33.7 s | 29.0 s | -14.0% | 1228 MB | 1228 MB | +0.0% | ~ |
| `Domain` | 21.6 s | 18.5 s | -14.4% | 842 MB | 842 MB | -0.0% | ~ |
| `DomainObject` | 19.4 s | 16.1 s | -16.8% | 845 MB | 846 MB | +0.1% | ~ |
| `Fused` | 28.3 s | 24.9 s | -12.1% | 1136 MB | 1136 MB | -0.0% | ~ |
| `LaunchTuning` | 41.5 s | 36.9 s | -11.2% | 1310 MB | 1310 MB | +0.0% | ~ |
| `Mesh` | 17.5 s | 15.8 s | -9.8% | 784 MB | 784 MB | -0.0% | ~ |
| `MeshCSG` | 44.2 s | 39.6 s | -10.3% | 1256 MB | 1256 MB | +0.0% | ~ |
| `MeshClosestPoint` | 16.3 s | 14.6 s | -10.3% | 780 MB | 781 MB | +0.1% | ~ |
| `MeshReconcile` | 7.8 s | 6.5 s | -15.7% | 566 MB | 566 MB | +0.0% | ~ |
| `MeshTape` | 48.8 s | 42.2 s | -13.7% | 1289 MB | 1289 MB | +0.0% | ~ |
| `PointCloud` | 8.6 s | 7.4 s | -13.7% | 557 MB | 558 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 5.3 s | 3.9 s | -26.6% | 470 MB | 471 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 3.9 s | 3.3 s | -16.4% | 452 MB | 452 MB | +0.1% | ~ |
| `Ray` | 43.1 s | 37.0 s | -14.0% | 1317 MB | 1318 MB | +0.1% | ~ |
| `RayCSG` | 38.7 s | 32.6 s | -15.8% | 1188 MB | 1188 MB | +0.1% | ~ |
| `RayCrossing` | 53.6 s | 46.2 s | -13.7% | 1331 MB | 1331 MB | +0.0% | ~ |
| `RayParticles` | 57.2 s | 49.7 s | -13.0% | 1319 MB | 1319 MB | +0.0% | ~ |
| `RayShell` | 47.4 s | 37.7 s | -20.5% | 1285 MB | 1285 MB | -0.0% | ~ |
| `Regions` | 36.9 s | 29.6 s | -19.7% | 1073 MB | 1073 MB | +0.0% | ~ |
| `Render` | 41.7 s | 35.7 s | -14.4% | 1285 MB | 1286 MB | +0.1% | ~ |
| `Shapes` | 9.9 s | 9.0 s | -9.2% | 553 MB | 554 MB | +0.1% | ~ |
| `SphereCloudBVH` | 5.6 s | 4.9 s | -12.3% | 510 MB | 511 MB | +0.1% | ~ |
| `SphereCloudRays` | 31.5 s | 28.0 s | -11.2% | 1063 MB | 1064 MB | +0.0% | ~ |
| `SphereContacts` | 5.0 s | 4.3 s | -14.2% | 484 MB | 485 MB | +0.1% | ~ |
| `SurfaceParts` | 46.0 s | 40.7 s | -11.5% | 1409 MB | 1410 MB | +0.1% | ~ |
| `Tape` | 18.8 s | 16.8 s | -10.7% | 830 MB | 830 MB | +0.0% | ~ |
| `TapeBVHUnion` | 16.5 s | 14.7 s | -10.9% | 818 MB | 818 MB | +0.0% | ~ |
| **all, in both** | 835.5 s | 717.2 s | -14.2% | 1409 MB | 1410 MB | +0.1% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (302 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), Fused (10), Mesh (12), MeshCSG (12), MeshClosestPoint (8), MeshReconcile (4), MeshTape (16), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
