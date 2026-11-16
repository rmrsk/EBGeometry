<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
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
base: d02baee
base-fingerprint: 8c75bde3d1f96257d466b8e7cb8c56164704950f
head: b2756fa
head-fingerprint: db167e0f22feeae8b3cf43e92dc20a313a139fd8
cases: 0
rounds: 2
reps: 3
date: 2026-11-16
-->

## Host comparison: green

**Accepted:** -

`b2756fa` against `d02baee` (origin/main), 2026-11-16.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 635.2 s → 625.6 s (-1.5%), the largest peak 1398 MB → 1398 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.2 s | 9.2 s | +0.5% | 556 MB | 556 MB | +0.1% | ~ |
| `BVHRefit` | 4.1 s | 3.9 s | -3.2% | 483 MB | 483 MB | +0.0% | ~ |
| `BVHUnion` | 18.5 s | 18.4 s | -0.6% | 894 MB | 894 MB | -0.0% | ~ |
| `BVHUnionMixed` | 7.4 s | 7.3 s | -1.2% | 555 MB | 555 MB | +0.0% | ~ |
| `ClosestPrimitive` | 30.0 s | 29.6 s | -1.5% | 1263 MB | 1263 MB | -0.0% | ~ |
| `ClosestSurface` | 28.6 s | 27.6 s | -3.4% | 1220 MB | 1219 MB | -0.0% | ~ |
| `Domain` | 18.3 s | 17.8 s | -2.7% | 839 MB | 839 MB | +0.0% | ~ |
| `Fused` | 24.2 s | 24.1 s | -0.8% | 1126 MB | 1126 MB | -0.1% | ~ |
| `LaunchTuning` | 36.6 s | 35.2 s | -3.8% | 1268 MB | 1268 MB | -0.0% | ~ |
| `Mesh` | 15.0 s | 14.9 s | -0.3% | 784 MB | 784 MB | -0.0% | ~ |
| `MeshCSG` | 39.0 s | 38.1 s | -2.5% | 1282 MB | 1282 MB | -0.0% | ~ |
| `MeshReconcile` | 7.0 s | 6.7 s | -4.4% | 565 MB | 565 MB | -0.0% | ~ |
| `PointCloud` | 7.6 s | 7.5 s | -1.0% | 558 MB | 558 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.7 s | -3.1% | 470 MB | 470 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | +1.1% | 451 MB | 451 MB | -0.1% | ~ |
| `Ray` | 35.8 s | 36.1 s | +0.8% | 1317 MB | 1317 MB | +0.0% | ~ |
| `RayCSG` | 31.9 s | 30.8 s | -3.4% | 1113 MB | 1114 MB | +0.0% | ~ |
| `RayCrossing` | 44.8 s | 45.1 s | +0.6% | 1341 MB | 1341 MB | -0.0% | ~ |
| `RayParticles` | 48.9 s | 48.0 s | -1.8% | 1326 MB | 1326 MB | -0.0% | ~ |
| `RayShell` | 37.8 s | 36.6 s | -3.2% | 1271 MB | 1271 MB | +0.0% | ~ |
| `Regions` | 28.4 s | 28.7 s | +1.1% | 1069 MB | 1069 MB | +0.0% | ~ |
| `Render` | 35.8 s | 35.5 s | -0.7% | 1267 MB | 1267 MB | -0.0% | ~ |
| `Shapes` | 9.0 s | 9.0 s | -0.4% | 553 MB | 553 MB | +0.0% | ~ |
| `SphereCloudBVH` | 4.9 s | 5.0 s | +1.2% | 510 MB | 510 MB | +0.0% | ~ |
| `SphereCloudRays` | 28.0 s | 27.5 s | -1.7% | 1059 MB | 1059 MB | +0.0% | ~ |
| `SphereContacts` | 4.4 s | 4.3 s | -1.8% | 484 MB | 483 MB | -0.0% | ~ |
| `SurfaceParts` | 41.4 s | 40.1 s | -3.1% | 1398 MB | 1398 MB | +0.1% | ~ |
| `Tape` | 16.8 s | 16.8 s | +0.1% | 817 MB | 817 MB | -0.1% | ~ |
| `TapeBVHUnion` | 14.7 s | 14.7 s | +0.4% | 822 MB | 822 MB | +0.0% | ~ |
| **all, in both** | 635.2 s | 625.6 s | -1.5% | 1398 MB | 1398 MB | +0.1% | ~ |


These probes compiled to the same code on both sides and were not run (270 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
