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
base: ff1cec5
base-fingerprint: 4843ceb3249f1cd989cdb0277e59255b66e41054
head: e6d87e8
head-fingerprint: af8d04cce8f9fe3d2fb898d4969c550453d83000
cases: 0
rounds: 2
reps: 3
date: 2026-11-23
-->

## Host comparison: green

**Accepted:** -

`e6d87e8` against `ff1cec5` (origin/main), 2026-11-23.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 762.1 s → 769.1 s (+0.9%), the largest peak 1419 MB → 1419 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.7 s | 9.7 s | +0.3% | 562 MB | 563 MB | +0.0% | ~ |
| `BVHRefit` | 4.3 s | 4.4 s | +1.0% | 503 MB | 503 MB | +0.1% | ~ |
| `BVHUnion` | 19.1 s | 19.3 s | +1.4% | 956 MB | 956 MB | -0.0% | ~ |
| `BVHUnionMixed` | 7.7 s | 7.8 s | +0.8% | 559 MB | 559 MB | -0.0% | ~ |
| `ClosestPrimitive` | 30.2 s | 30.3 s | +0.3% | 1348 MB | 1348 MB | +0.0% | ~ |
| `ClosestSurface` | 27.8 s | 28.0 s | +0.9% | 1317 MB | 1318 MB | +0.1% | ~ |
| `ConnectedComponents` | 15.4 s | 16.0 s | +4.4% | 854 MB | 854 MB | +0.0% | ~ |
| `Domain` | 18.5 s | 18.2 s | -1.4% | 860 MB | 860 MB | +0.1% | ~ |
| `DomainObject` | 15.9 s | 16.2 s | +2.0% | 860 MB | 860 MB | -0.0% | ~ |
| `EBGrid` | 14.4 s | 15.0 s | +4.2% | 784 MB | 784 MB | +0.0% | ~ |
| `Fused` | 24.2 s | 24.4 s | +0.8% | 1236 MB | 1237 MB | +0.1% | ~ |
| `LaunchTuning` | 37.5 s | 38.0 s | +1.5% | 1328 MB | 1328 MB | +0.1% | ~ |
| `Mesh` | 16.3 s | 15.9 s | -2.2% | 791 MB | 791 MB | +0.0% | ~ |
| `MeshCSG` | 39.1 s | 40.2 s | +2.8% | 1305 MB | 1306 MB | +0.0% | ~ |
| `MeshClosestPoint` | 14.4 s | 14.4 s | -0.5% | 786 MB | 786 MB | -0.0% | ~ |
| `MeshIngest` | 7.0 s | 7.2 s | +2.7% | 569 MB | 569 MB | -0.0% | ~ |
| `MeshReconcile` | 6.8 s | 6.8 s | +0.1% | 569 MB | 569 MB | +0.0% | ~ |
| `MeshTape` | 41.4 s | 42.6 s | +2.8% | 1261 MB | 1260 MB | -0.0% | ~ |
| `PointCloud` | 7.3 s | 7.4 s | +1.7% | 562 MB | 562 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.7 s | 3.8 s | +1.7% | 477 MB | 476 MB | -0.0% | ~ |
| `PointCloudHashGridBuild` | 3.2 s | 3.3 s | +3.1% | 457 MB | 457 MB | -0.0% | ~ |
| `QueryOrder` | 11.5 s | 11.6 s | +0.3% | 707 MB | 708 MB | +0.0% | ~ |
| `Ray` | 37.0 s | 37.2 s | +0.6% | 1342 MB | 1342 MB | +0.1% | ~ |
| `RayCSG` | 31.7 s | 32.0 s | +0.9% | 1204 MB | 1204 MB | -0.1% | ~ |
| `RayCrossing` | 45.2 s | 45.5 s | +0.6% | 1360 MB | 1360 MB | +0.0% | ~ |
| `RayParticles` | 48.9 s | 48.9 s | +0.0% | 1334 MB | 1334 MB | +0.0% | ~ |
| `RayShell` | 37.9 s | 38.3 s | +1.0% | 1316 MB | 1315 MB | -0.1% | ~ |
| `Regions` | 29.7 s | 30.4 s | +2.5% | 1094 MB | 1116 MB | +2.0% | ~ |
| `Render` | 36.9 s | 36.2 s | -1.7% | 1300 MB | 1300 MB | -0.0% | ~ |
| `Shapes` | 8.8 s | 8.8 s | -0.3% | 559 MB | 559 MB | +0.0% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.8 s | +1.6% | 518 MB | 518 MB | -0.0% | ~ |
| `SphereCloudRays` | 28.4 s | 28.2 s | -0.5% | 1077 MB | 1077 MB | +0.0% | ~ |
| `SphereContacts` | 4.3 s | 4.4 s | +2.7% | 490 MB | 490 MB | +0.1% | ~ |
| `SurfaceParts` | 41.6 s | 42.0 s | +1.0% | 1419 MB | 1419 MB | -0.0% | ~ |
| `Tape` | 16.8 s | 17.0 s | +1.3% | 838 MB | 838 MB | +0.0% | ~ |
| `TapeBVHUnion` | 14.7 s | 14.7 s | -0.4% | 838 MB | 839 MB | +0.0% | ~ |
| **all, in both** | 762.1 s | 769.1 s | +0.9% | 1419 MB | 1419 MB | -0.0% | ~ |


These probes compiled to the same code on both sides and were not run (332 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), ConnectedComponents (4), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshCSG (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), MeshTape (16), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
