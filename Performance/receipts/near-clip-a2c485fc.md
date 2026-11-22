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
base: b973065
base-fingerprint: a23b462307605142893524cfafdb9453ae008378
head: ffc0d6d
head-fingerprint: a2c485fc369aca048e00b4520e1a13072b6fe671
cases: 0
rounds: 2
reps: 3
date: 2026-11-22
-->

## Host comparison: green

**Accepted:** -

`ffc0d6d` against `b973065` (origin/main), 2026-11-22.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 759.1 s → 760.7 s (+0.2%), the largest peak 1421 MB → 1421 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.6 s | 9.6 s | -0.1% | 558 MB | 558 MB | -0.0% | ~ |
| `BVHRefit` | 4.3 s | 4.3 s | +0.1% | 501 MB | 501 MB | +0.0% | ~ |
| `BVHUnion` | 19.2 s | 19.4 s | +0.7% | 953 MB | 954 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.6 s | 7.6 s | -0.2% | 558 MB | 558 MB | -0.0% | ~ |
| `ClosestPrimitive` | 30.1 s | 30.2 s | +0.2% | 1342 MB | 1341 MB | -0.0% | ~ |
| `ClosestSurface` | 27.9 s | 28.2 s | +1.1% | 1314 MB | 1314 MB | +0.0% | ~ |
| `ConnectedComponents` | 15.4 s | 15.3 s | -0.8% | 848 MB | 848 MB | -0.0% | ~ |
| `Domain` | 18.1 s | 18.1 s | -0.1% | 855 MB | 854 MB | -0.0% | ~ |
| `DomainObject` | 15.9 s | 15.9 s | -0.2% | 854 MB | 855 MB | +0.1% | ~ |
| `EBGrid` | 14.6 s | 14.5 s | -0.5% | 784 MB | 784 MB | +0.1% | ~ |
| `Fused` | 24.5 s | 24.3 s | -0.7% | 1228 MB | 1228 MB | -0.0% | ~ |
| `LaunchTuning` | 37.0 s | 37.0 s | +0.1% | 1325 MB | 1326 MB | +0.0% | ~ |
| `Mesh` | 16.0 s | 15.8 s | -1.4% | 789 MB | 789 MB | -0.0% | ~ |
| `MeshCSG` | 39.9 s | 40.0 s | +0.2% | 1277 MB | 1277 MB | +0.0% | ~ |
| `MeshClosestPoint` | 14.3 s | 14.1 s | -1.5% | 784 MB | 784 MB | +0.0% | ~ |
| `MeshIngest` | 7.0 s | 7.1 s | +1.4% | 569 MB | 569 MB | -0.0% | ~ |
| `MeshReconcile` | 6.9 s | 6.9 s | -0.0% | 569 MB | 568 MB | -0.1% | ~ |
| `MeshTape` | 42.0 s | 42.4 s | +1.1% | 1268 MB | 1268 MB | +0.0% | ~ |
| `PointCloud` | 7.3 s | 7.3 s | +0.3% | 561 MB | 561 MB | -0.0% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.8 s | +0.2% | 474 MB | 474 MB | -0.0% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.2 s | -1.3% | 455 MB | 456 MB | +0.1% | ~ |
| `QueryOrder` | 11.3 s | 11.3 s | -0.3% | 700 MB | 700 MB | -0.0% | ~ |
| `Ray` | 36.5 s | 36.8 s | +0.7% | 1344 MB | 1344 MB | -0.0% | ~ |
| `RayCSG` | 31.9 s | 32.1 s | +0.7% | 1211 MB | 1211 MB | -0.0% | ~ |
| `RayCrossing` | 45.6 s | 45.5 s | -0.1% | 1351 MB | 1351 MB | +0.0% | ~ |
| `RayParticles` | 49.0 s | 49.5 s | +1.0% | 1323 MB | 1323 MB | +0.1% | ~ |
| `RayShell` | 37.3 s | 37.6 s | +0.8% | 1283 MB | 1283 MB | +0.0% | ~ |
| `Regions` | 29.5 s | 29.3 s | -0.7% | 1086 MB | 1087 MB | +0.1% | ~ |
| `Render` | 35.6 s | 36.1 s | +1.4% | 1291 MB | 1291 MB | +0.0% | ~ |
| `Shapes` | 8.7 s | 8.8 s | +1.0% | 557 MB | 558 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.9 s | 4.9 s | -0.2% | 516 MB | 516 MB | +0.0% | ~ |
| `SphereCloudRays` | 28.0 s | 28.0 s | +0.1% | 1074 MB | 1074 MB | -0.0% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | -0.1% | 488 MB | 488 MB | -0.0% | ~ |
| `SurfaceParts` | 40.5 s | 40.4 s | -0.2% | 1421 MB | 1421 MB | +0.0% | ~ |
| `Tape` | 16.6 s | 16.7 s | +0.2% | 832 MB | 831 MB | -0.1% | ~ |
| `TapeBVHUnion` | 14.7 s | 14.6 s | -0.6% | 834 MB | 834 MB | +0.0% | ~ |
| **all, in both** | 759.1 s | 760.7 s | +0.2% | 1421 MB | 1421 MB | +0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (332 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), ConnectedComponents (4), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshCSG (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), MeshTape (16), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
