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
base: eb89c669
base-fingerprint: 4c8a55718a2c1fff0986eab16001598161c76227
head: 410f82a7
head-fingerprint: 1e0f461958307254a04eb0a9ea2890c216b99160
cases: 0
rounds: 2
reps: 3
date: 2026-11-24
-->

## Host comparison: green

**Accepted:** -

`410f82a7` against `eb89c669` (origin/main), 2026-11-24.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 761.1 s → 757.5 s (-0.5%), the largest peak 1419 MB → 1418 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.7 s | 9.6 s | -0.6% | 562 MB | 563 MB | +0.0% | ~ |
| `BVHRefit` | 4.3 s | 4.3 s | +0.6% | 503 MB | 503 MB | +0.0% | ~ |
| `BVHUnion` | 19.2 s | 19.1 s | -0.7% | 956 MB | 956 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.7 s | 7.7 s | +0.2% | 559 MB | 559 MB | -0.0% | ~ |
| `ClosestPrimitive` | 30.5 s | 29.9 s | -1.9% | 1348 MB | 1348 MB | -0.0% | ~ |
| `ClosestSurface` | 27.7 s | 27.6 s | -0.4% | 1317 MB | 1317 MB | +0.0% | ~ |
| `ConnectedComponents` | 15.4 s | 15.4 s | -0.1% | 854 MB | 854 MB | +0.0% | ~ |
| `Domain` | 17.9 s | 18.0 s | +0.2% | 860 MB | 860 MB | -0.1% | ~ |
| `DomainObject` | 15.8 s | 15.9 s | +0.5% | 861 MB | 860 MB | -0.0% | ~ |
| `EBGrid` | 14.4 s | 14.4 s | -0.0% | 784 MB | 784 MB | -0.0% | ~ |
| `Fused` | 24.5 s | 24.1 s | -1.6% | 1237 MB | 1237 MB | +0.0% | ~ |
| `LaunchTuning` | 37.1 s | 37.5 s | +0.9% | 1328 MB | 1328 MB | +0.0% | ~ |
| `Mesh` | 15.7 s | 15.5 s | -1.0% | 791 MB | 791 MB | +0.0% | ~ |
| `MeshCSG` | 39.4 s | 39.7 s | +0.8% | 1306 MB | 1306 MB | +0.0% | ~ |
| `MeshClosestPoint` | 14.3 s | 14.1 s | -1.5% | 786 MB | 787 MB | +0.1% | ~ |
| `MeshIngest` | 7.1 s | 6.9 s | -2.8% | 569 MB | 569 MB | +0.0% | ~ |
| `MeshReconcile` | 6.9 s | 6.8 s | -1.1% | 569 MB | 569 MB | -0.0% | ~ |
| `MeshTape` | 41.8 s | 42.1 s | +0.7% | 1261 MB | 1260 MB | -0.1% | ~ |
| `PointCloud` | 7.5 s | 7.3 s | -2.1% | 562 MB | 562 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.8 s | +0.7% | 476 MB | 477 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | +0.1% | 457 MB | 457 MB | -0.0% | ~ |
| `QueryOrder` | 11.4 s | 11.3 s | -0.1% | 707 MB | 707 MB | +0.0% | ~ |
| `Ray` | 36.8 s | 36.1 s | -1.8% | 1342 MB | 1342 MB | +0.0% | ~ |
| `RayCSG` | 31.6 s | 31.7 s | +0.3% | 1204 MB | 1205 MB | +0.1% | ~ |
| `RayCrossing` | 46.5 s | 45.1 s | -3.0% | 1360 MB | 1360 MB | +0.0% | ~ |
| `RayParticles` | 49.6 s | 48.6 s | -1.9% | 1334 MB | 1334 MB | +0.0% | ~ |
| `RayShell` | 37.9 s | 37.8 s | -0.4% | 1316 MB | 1312 MB | -0.3% | ~ |
| `Regions` | 29.2 s | 29.8 s | +2.0% | 1116 MB | 1115 MB | -0.0% | ~ |
| `Render` | 36.2 s | 36.3 s | +0.4% | 1300 MB | 1300 MB | -0.0% | ~ |
| `Shapes` | 8.7 s | 8.8 s | +0.2% | 559 MB | 558 MB | -0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.8 s | -0.1% | 518 MB | 518 MB | +0.1% | ~ |
| `SphereCloudRays` | 28.1 s | 28.1 s | -0.2% | 1077 MB | 1078 MB | +0.0% | ~ |
| `SphereContacts` | 4.4 s | 4.2 s | -3.4% | 490 MB | 490 MB | -0.0% | ~ |
| `SurfaceParts` | 40.6 s | 40.8 s | +0.3% | 1419 MB | 1418 MB | -0.1% | ~ |
| `Tape` | 16.9 s | 16.8 s | -0.6% | 838 MB | 838 MB | +0.1% | ~ |
| `TapeBVHUnion` | 14.6 s | 14.5 s | -0.9% | 839 MB | 839 MB | +0.0% | ~ |
| **all, in both** | 761.1 s | 757.5 s | -0.5% | 1419 MB | 1418 MB | -0.1% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (332 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), ConnectedComponents (4), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshCSG (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), MeshTape (16), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/Common/Environment.hpp

<!-- END receipt host -->
