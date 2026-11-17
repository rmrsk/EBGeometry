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
base: 14f3222
base-fingerprint: f6a77e99a55a926ac48bef8ad62b5634073733db
head: 40a624c
head-fingerprint: 5c8f1f2e8f2f01ac153884975bc82e49bb9e3477
cases: 0
rounds: 2
reps: 3
date: 2026-11-17
-->

## Host comparison: green

**Accepted:** -

`40a624c` against `14f3222` (origin/main), 2026-11-17.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 12 cores, 24 hardware threads, 31 GB. Compiled on hardware threads 0,1,2,3,4,5,6,7,8,9,10,11 (one per core), 12 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 1107.4 s → 753.5 s (-32.0%), the largest peak 1399 MB → 1399 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 19.0 s | 11.2 s | -41.0% | 560 MB | 561 MB | +0.1% | ~ |
| `BVHRefit` | 8.1 s | 5.2 s | -35.5% | 488 MB | 488 MB | +0.1% | ~ |
| `BVHUnion` | 35.5 s | 21.8 s | -38.6% | 900 MB | 901 MB | +0.0% | ~ |
| `BVHUnionMixed` | 14.7 s | 9.3 s | -36.8% | 559 MB | 560 MB | +0.3% | ~ |
| `ClosestPrimitive` | 59.3 s | 35.6 s | -40.0% | 1269 MB | 1270 MB | +0.1% | ~ |
| `ClosestSurface` | 55.2 s | 33.5 s | -39.3% | 1223 MB | 1223 MB | -0.0% | ~ |
| `Domain` | 35.9 s | 21.8 s | -39.2% | 844 MB | 845 MB | +0.0% | ~ |
| `DomainObject` | 32.1 s | 19.6 s | -38.9% | 848 MB | 848 MB | -0.0% | ~ |
| `Fused` | 48.8 s | 28.9 s | -40.7% | 1129 MB | 1130 MB | +0.1% | ~ |
| `LaunchTuning` | 69.8 s | 42.2 s | -39.6% | 1306 MB | 1306 MB | +0.0% | ~ |
| `Mesh` | 29.0 s | 17.8 s | -38.8% | 788 MB | 789 MB | +0.1% | ~ |
| `MeshCSG` | 71.5 s | 45.1 s | -36.9% | 1262 MB | 1262 MB | -0.0% | ~ |
| `MeshReconcile` | 13.0 s | 8.0 s | -38.9% | 569 MB | 569 MB | +0.0% | ~ |
| `PointCloud` | 14.3 s | 8.8 s | -38.4% | 563 MB | 563 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 7.9 s | 4.6 s | -41.0% | 474 MB | 474 MB | -0.1% | ~ |
| `PointCloudHashGridBuild` | 6.6 s | 4.0 s | -38.7% | 456 MB | 456 MB | -0.0% | ~ |
| `Ray` | 60.1 s | 41.0 s | -31.7% | 1319 MB | 1320 MB | +0.1% | ~ |
| `RayCSG` | 57.4 s | 37.1 s | -35.3% | 1176 MB | 1177 MB | +0.1% | ~ |
| `RayCrossing` | 69.1 s | 50.6 s | -26.7% | 1335 MB | 1336 MB | +0.1% | ~ |
| `RayParticles` | 72.1 s | 53.6 s | -25.7% | 1322 MB | 1322 MB | +0.1% | ~ |
| `RayShell` | 58.7 s | 41.9 s | -28.6% | 1278 MB | 1279 MB | +0.1% | ~ |
| `Regions` | 46.9 s | 32.9 s | -29.7% | 1072 MB | 1073 MB | +0.1% | ~ |
| `Render` | 54.6 s | 40.4 s | -26.0% | 1284 MB | 1285 MB | +0.1% | ~ |
| `Shapes` | 18.3 s | 10.5 s | -42.5% | 558 MB | 558 MB | -0.0% | ~ |
| `SphereCloudBVH` | 9.5 s | 5.8 s | -38.8% | 514 MB | 514 MB | -0.1% | ~ |
| `SphereCloudRays` | 39.5 s | 32.4 s | -18.0% | 1063 MB | 1064 MB | +0.1% | ~ |
| `SphereContacts` | 7.3 s | 5.2 s | -28.5% | 487 MB | 488 MB | +0.2% | ~ |
| `SurfaceParts` | 50.3 s | 47.1 s | -6.3% | 1399 MB | 1399 MB | +0.0% | ~ |
| `Tape` | 23.5 s | 19.9 s | -15.6% | 822 MB | 823 MB | +0.1% | ~ |
| `TapeBVHUnion` | 19.5 s | 17.5 s | -10.2% | 826 MB | 828 MB | +0.2% | ~ |
| **all, in both** | 1107.4 s | 753.5 s | -32.0% | 1399 MB | 1399 MB | +0.0% | ~ |


These probes compiled to the same code on both sides and were not run (278 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
