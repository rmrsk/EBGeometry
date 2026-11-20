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
base: f368fa5
base-fingerprint: 01e9d00d88123d3b4251c2eb032724fc4be97258
head: 5bd279a
head-fingerprint: 1aeec8e6c1b5b926277e2361946c660ea082fc68
cases: 0
rounds: 2
reps: 3
date: 2026-11-20
-->

## Host comparison: green

**Accepted:** -

`5bd279a` against `f368fa5` (origin/main), 2026-11-20.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 736.4 s → 735.7 s (-0.1%), the largest peak 1414 MB → 1413 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.4 s | 9.4 s | +0.4% | 559 MB | 559 MB | +0.0% | ~ |
| `BVHRefit` | 4.3 s | 4.3 s | +0.5% | 498 MB | 498 MB | +0.0% | ~ |
| `BVHUnion` | 19.2 s | 19.4 s | +0.7% | 949 MB | 949 MB | -0.0% | ~ |
| `BVHUnionMixed` | 7.6 s | 7.7 s | +1.2% | 556 MB | 556 MB | -0.0% | ~ |
| `ClosestPrimitive` | 30.0 s | 30.1 s | +0.4% | 1299 MB | 1299 MB | -0.0% | ~ |
| `ClosestSurface` | 28.1 s | 27.9 s | -0.6% | 1297 MB | 1297 MB | -0.0% | ~ |
| `Domain` | 18.0 s | 18.0 s | -0.1% | 843 MB | 843 MB | +0.0% | ~ |
| `DomainObject` | 16.0 s | 15.9 s | -0.4% | 848 MB | 848 MB | -0.1% | ~ |
| `EBGrid` | 14.2 s | 14.3 s | +0.8% | 779 MB | 779 MB | -0.0% | ~ |
| `Fused` | 24.4 s | 24.7 s | +1.2% | 1206 MB | 1206 MB | +0.0% | ~ |
| `LaunchTuning` | 36.5 s | 37.3 s | +2.2% | 1280 MB | 1280 MB | +0.1% | ~ |
| `Mesh` | 15.8 s | 15.5 s | -2.0% | 785 MB | 785 MB | -0.1% | ~ |
| `MeshCSG` | 39.4 s | 39.0 s | -1.0% | 1261 MB | 1261 MB | -0.0% | ~ |
| `MeshClosestPoint` | 13.9 s | 13.9 s | +0.1% | 780 MB | 780 MB | +0.0% | ~ |
| `MeshReconcile` | 6.6 s | 6.6 s | +0.3% | 567 MB | 567 MB | -0.0% | ~ |
| `MeshTape` | 42.1 s | 41.9 s | -0.5% | 1280 MB | 1280 MB | +0.0% | ~ |
| `PointCloud` | 7.5 s | 7.3 s | -3.1% | 558 MB | 559 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.8 s | -0.1% | 472 MB | 472 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.2 s | -2.0% | 453 MB | 453 MB | -0.0% | ~ |
| `QueryOrder` | 11.4 s | 11.3 s | -1.1% | 687 MB | 687 MB | +0.0% | ~ |
| `Ray` | 36.1 s | 36.3 s | +0.6% | 1293 MB | 1293 MB | -0.0% | ~ |
| `RayCSG` | 32.2 s | 32.2 s | -0.1% | 1189 MB | 1189 MB | -0.0% | ~ |
| `RayCrossing` | 45.9 s | 45.2 s | -1.7% | 1349 MB | 1350 MB | +0.0% | ~ |
| `RayParticles` | 49.3 s | 48.8 s | -1.1% | 1331 MB | 1331 MB | +0.0% | ~ |
| `RayShell` | 37.9 s | 38.1 s | +0.6% | 1267 MB | 1267 MB | +0.0% | ~ |
| `Regions` | 29.2 s | 29.6 s | +1.2% | 1075 MB | 1074 MB | -0.0% | ~ |
| `Render` | 35.8 s | 35.8 s | +0.2% | 1274 MB | 1274 MB | -0.0% | ~ |
| `Shapes` | 8.8 s | 8.8 s | -0.4% | 555 MB | 555 MB | -0.0% | ~ |
| `SphereCloudBVH` | 4.9 s | 4.8 s | -1.3% | 512 MB | 512 MB | -0.0% | ~ |
| `SphereCloudRays` | 28.0 s | 27.9 s | -0.4% | 1066 MB | 1066 MB | -0.0% | ~ |
| `SphereContacts` | 4.4 s | 4.3 s | -1.1% | 486 MB | 486 MB | +0.1% | ~ |
| `SurfaceParts` | 40.5 s | 40.5 s | -0.2% | 1414 MB | 1413 MB | -0.0% | ~ |
| `Tape` | 16.9 s | 16.9 s | -0.0% | 830 MB | 830 MB | -0.0% | ~ |
| `TapeBVHUnion` | 14.9 s | 14.9 s | +0.3% | 826 MB | 827 MB | +0.1% | ~ |
| **all, in both** | 736.4 s | 735.7 s | -0.1% | 1414 MB | 1413 MB | -0.0% | ~ |


These probes compiled to the same code on both sides and were not run (320 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshCSG (12), MeshClosestPoint (8), MeshReconcile (4), MeshTape (16), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
