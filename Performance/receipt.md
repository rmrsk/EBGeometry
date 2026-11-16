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
base: 4bf90e9
base-fingerprint: 30b6edfbe4d67af90dcaf2ebe131bc7df88464b1
head: b1aff5c
head-fingerprint: 632e23e401d383de2de3518cb5adf45362943bf4
cases: 0
rounds: 2
reps: 3
date: 2026-11-17
-->

## Host comparison: green

**Accepted:** -

`b1aff5c` against `4bf90e9` (origin/main), 2026-11-17.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 12 cores, 24 hardware threads, 31 GB. Compiled on hardware threads 0,1,2,3,4,5,6,7,8,9,10,11 (one per core), 12 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 733.0 s → 734.9 s (+0.2%), the largest peak 1398 MB → 1399 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 11.3 s | 11.2 s | -0.8% | 560 MB | 560 MB | +0.0% | ~ |
| `BVHRefit` | 5.2 s | 5.1 s | -0.7% | 488 MB | 488 MB | -0.1% | ~ |
| `BVHUnion` | 21.9 s | 21.9 s | +0.1% | 900 MB | 900 MB | +0.0% | ~ |
| `BVHUnionMixed` | 9.3 s | 9.3 s | -0.2% | 560 MB | 560 MB | +0.0% | ~ |
| `ClosestPrimitive` | 35.7 s | 35.6 s | -0.3% | 1269 MB | 1268 MB | -0.0% | ~ |
| `ClosestSurface` | 33.2 s | 33.4 s | +0.7% | 1224 MB | 1223 MB | -0.0% | ~ |
| `Domain` | 22.0 s | 22.0 s | +0.1% | 844 MB | 843 MB | -0.0% | ~ |
| `Fused` | 28.7 s | 29.0 s | +1.1% | 1129 MB | 1128 MB | -0.0% | ~ |
| `LaunchTuning` | 41.9 s | 42.0 s | +0.3% | 1271 MB | 1271 MB | -0.0% | ~ |
| `Mesh` | 17.9 s | 17.9 s | -0.3% | 789 MB | 789 MB | +0.1% | ~ |
| `MeshCSG` | 44.9 s | 44.9 s | +0.0% | 1288 MB | 1288 MB | -0.0% | ~ |
| `MeshReconcile` | 8.0 s | 8.0 s | -0.1% | 569 MB | 568 MB | -0.1% | ~ |
| `PointCloud` | 8.8 s | 8.9 s | +0.5% | 563 MB | 562 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 4.7 s | 4.7 s | +0.1% | 474 MB | 473 MB | -0.2% | ~ |
| `PointCloudHashGridBuild` | 4.1 s | 4.1 s | -0.3% | 456 MB | 456 MB | +0.0% | ~ |
| `Ray` | 41.0 s | 41.5 s | +1.1% | 1320 MB | 1320 MB | +0.0% | ~ |
| `RayCSG` | 36.9 s | 36.9 s | -0.1% | 1116 MB | 1117 MB | +0.0% | ~ |
| `RayCrossing` | 50.7 s | 50.9 s | +0.4% | 1344 MB | 1344 MB | -0.0% | ~ |
| `RayParticles` | 53.4 s | 53.6 s | +0.3% | 1329 MB | 1330 MB | +0.0% | ~ |
| `RayShell` | 42.1 s | 41.8 s | -0.7% | 1274 MB | 1274 MB | +0.0% | ~ |
| `Regions` | 32.6 s | 32.6 s | +0.1% | 1072 MB | 1072 MB | -0.0% | ~ |
| `Render` | 40.3 s | 40.8 s | +1.2% | 1271 MB | 1271 MB | +0.0% | ~ |
| `Shapes` | 10.5 s | 10.5 s | +0.8% | 558 MB | 558 MB | -0.1% | ~ |
| `SphereCloudBVH` | 5.8 s | 5.8 s | +0.7% | 513 MB | 514 MB | +0.1% | ~ |
| `SphereCloudRays` | 32.6 s | 32.5 s | -0.4% | 1063 MB | 1063 MB | -0.0% | ~ |
| `SphereContacts` | 5.1 s | 5.2 s | +1.6% | 487 MB | 487 MB | -0.1% | ~ |
| `SurfaceParts` | 47.1 s | 47.1 s | +0.0% | 1398 MB | 1399 MB | +0.1% | ~ |
| `Tape` | 19.8 s | 20.1 s | +1.4% | 822 MB | 822 MB | -0.0% | ~ |
| `TapeBVHUnion` | 17.5 s | 17.4 s | -0.2% | 826 MB | 826 MB | +0.0% | ~ |
| **all, in both** | 733.0 s | 734.9 s | +0.2% | 1398 MB | 1399 MB | +0.1% | ~ |


These probes compiled to the same code on both sides and were not run (270 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
