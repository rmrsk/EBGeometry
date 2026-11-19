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
head: aed632a
head-fingerprint: 637e4476fd651126106ddcc42bfbd0c730731231
cases: 0
rounds: 2
reps: 3
date: 2026-11-19
-->

## Host comparison: green

**Accepted:** -

`aed632a` against `fd5c8a5` (origin/main), 2026-11-19.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 835.5 s → 840.6 s (+0.6%), the largest peak 1409 MB → 1409 MB; 2 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.1 s | 10.5 s | -12.9% | 558 MB | 558 MB | +0.0% | ~ |
| `BVHRefit` | 6.0 s | 4.5 s | -25.1% | 496 MB | 497 MB | +0.1% | ~ |
| `BVHUnion` | 23.5 s | 22.0 s | -6.3% | 948 MB | 948 MB | +0.0% | ~ |
| `BVHUnionMixed` | 9.8 s | 8.3 s | -16.0% | 555 MB | 555 MB | +0.1% | ~ |
| `ClosestPrimitive` | 35.4 s | 34.4 s | -2.8% | 1297 MB | 1297 MB | +0.0% | ~ |
| `ClosestSurface` | 33.7 s | 32.7 s | -3.0% | 1228 MB | 1228 MB | +0.0% | ~ |
| `Domain` | 21.6 s | 20.0 s | -7.2% | 842 MB | 842 MB | +0.0% | ~ |
| `DomainObject` | 19.4 s | 18.4 s | -5.2% | 845 MB | 846 MB | +0.1% | ~ |
| `Fused` | 28.3 s | 29.3 s | +3.7% | 1136 MB | 1136 MB | -0.0% | ~ |
| `LaunchTuning` | 41.5 s | 42.9 s | +3.5% | 1310 MB | 1310 MB | -0.0% | ~ |
| `Mesh` | 17.5 s | 19.0 s | +8.5% | 784 MB | 784 MB | -0.1% | ~ |
| `MeshCSG` | 44.2 s | 45.9 s | +3.9% | 1256 MB | 1256 MB | +0.0% | ~ |
| `MeshClosestPoint` | 16.3 s | 16.5 s | +1.3% | 780 MB | 781 MB | +0.1% | ~ |
| `MeshReconcile` | 7.8 s | 7.9 s | +1.4% | 566 MB | 566 MB | +0.0% | ~ |
| `MeshTape` | 48.8 s | 50.3 s | +3.0% | 1289 MB | 1289 MB | +0.0% | ~ |
| `PointCloud` | 8.6 s | 8.4 s | -2.2% | 557 MB | 558 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 5.3 s | 4.4 s | -16.6% | 470 MB | 471 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 3.9 s | 3.7 s | -5.6% | 452 MB | 452 MB | +0.1% | ~ |
| `Ray` | 43.1 s | 43.0 s | -0.1% | 1317 MB | 1318 MB | +0.1% | ~ |
| `RayCSG` | 38.7 s | 39.3 s | +1.5% | 1188 MB | 1188 MB | +0.0% | ~ |
| `RayCrossing` | 53.6 s | 53.3 s | -0.6% | 1331 MB | 1331 MB | -0.0% | ~ |
| `RayParticles` | 57.2 s | 58.0 s | +1.5% | 1319 MB | 1320 MB | +0.1% | ~ |
| `RayShell` | 47.4 s | 44.3 s | -6.4% | 1285 MB | 1285 MB | -0.0% | ~ |
| `Regions` | 36.9 s | 36.6 s | -0.8% | 1073 MB | 1072 MB | -0.0% | ~ |
| `Render` | 41.7 s | 42.9 s | +2.9% | 1285 MB | 1285 MB | +0.0% | ~ |
| `Shapes` | 9.9 s | 12.5 s | +26.1% | 553 MB | 553 MB | +0.0% | **MAJOR** |
| `SphereCloudBVH` | 5.6 s | 5.7 s | +2.1% | 510 MB | 511 MB | +0.1% | ~ |
| `SphereCloudRays` | 31.5 s | 32.8 s | +4.1% | 1063 MB | 1063 MB | +0.0% | ~ |
| `SphereContacts` | 5.0 s | 4.7 s | -6.7% | 484 MB | 485 MB | +0.1% | ~ |
| `SurfaceParts` | 46.0 s | 49.1 s | +6.9% | 1409 MB | 1409 MB | -0.0% | ~ |
| `Tape` | 18.8 s | 20.2 s | +7.8% | 830 MB | 829 MB | -0.0% | ~ |
| `TapeBVHUnion` | 16.5 s | 18.8 s | +13.9% | 818 MB | 818 MB | -0.0% | **minor** |
| **all, in both** | 835.5 s | 840.6 s | +0.6% | 1409 MB | 1409 MB | -0.0% | ~ |


These probes compiled to the same code on both sides and were not run (302 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), Fused (10), Mesh (12), MeshCSG (12), MeshClosestPoint (8), MeshReconcile (4), MeshTape (16), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
