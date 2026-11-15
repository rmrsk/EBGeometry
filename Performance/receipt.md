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
base: 669e7f4
base-fingerprint: f7653fee8c5b2beba423cac303718dc728ad6b3e
head: f8fa061
head-fingerprint: a7395912050fbf764c17a1ed09114dd9cb1377f5
cases: 0
rounds: 2
reps: 3
date: 2026-11-15
-->

## Host comparison: green

**Accepted:** -

`f8fa061` against `669e7f4` (origin/main), 2026-11-15.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 610.2 s → 616.7 s (+1.1%), the largest peak 1399 MB → 1398 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.0 s | -0.6% | 555 MB | 555 MB | -0.1% | ~ |
| `BVHRefit` | 4.0 s | 3.9 s | -1.2% | 482 MB | 482 MB | -0.0% | ~ |
| `BVHUnion` | 17.8 s | 17.9 s | +0.7% | 894 MB | 893 MB | -0.1% | ~ |
| `BVHUnionMixed` | 7.3 s | 7.3 s | -0.5% | 555 MB | 555 MB | +0.0% | ~ |
| `ClosestPrimitive` | 28.7 s | 29.2 s | +1.6% | 1259 MB | 1259 MB | +0.0% | ~ |
| `ClosestSurface` | 27.0 s | 27.5 s | +2.1% | 1219 MB | 1219 MB | +0.0% | ~ |
| `Domain` | 17.6 s | 17.5 s | -0.4% | 838 MB | 838 MB | +0.0% | ~ |
| `Fused` | 23.3 s | 23.5 s | +1.0% | 1125 MB | 1124 MB | -0.1% | ~ |
| `LaunchTuning` | 34.9 s | 35.2 s | +1.0% | 1269 MB | 1269 MB | +0.0% | ~ |
| `Mesh` | 14.3 s | 14.5 s | +1.5% | 786 MB | 785 MB | -0.1% | ~ |
| `MeshCSG` | 37.0 s | 37.5 s | +1.2% | 1281 MB | 1281 MB | +0.0% | ~ |
| `MeshReconcile` | 6.4 s | 6.6 s | +4.2% | 565 MB | 564 MB | -0.1% | ~ |
| `PointCloud` | 7.0 s | 7.3 s | +4.1% | 557 MB | 558 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 3.6 s | 3.7 s | +3.4% | 469 MB | 469 MB | -0.0% | ~ |
| `PointCloudHashGridBuild` | 3.1 s | 3.2 s | +2.9% | 451 MB | 451 MB | -0.0% | ~ |
| `Ray` | 35.1 s | 35.0 s | -0.3% | 1316 MB | 1316 MB | -0.0% | ~ |
| `RayCSG` | 30.3 s | 30.7 s | +1.2% | 1113 MB | 1113 MB | +0.0% | ~ |
| `RayCrossing` | 43.5 s | 44.4 s | +1.9% | 1341 MB | 1341 MB | -0.0% | ~ |
| `RayParticles` | 46.6 s | 47.1 s | +1.1% | 1318 MB | 1318 MB | -0.0% | ~ |
| `RayShell` | 36.2 s | 36.7 s | +1.5% | 1238 MB | 1237 MB | -0.0% | ~ |
| `Regions` | 27.7 s | 28.3 s | +2.2% | 1065 MB | 1065 MB | -0.0% | ~ |
| `Render` | 35.0 s | 34.9 s | -0.1% | 1261 MB | 1261 MB | +0.0% | ~ |
| `Shapes` | 8.6 s | 8.7 s | +1.1% | 553 MB | 553 MB | -0.1% | ~ |
| `SphereCloudBVH` | 4.7 s | 4.8 s | +1.3% | 509 MB | 509 MB | -0.0% | ~ |
| `SphereCloudRays` | 27.2 s | 27.3 s | +0.3% | 1059 MB | 1059 MB | +0.0% | ~ |
| `SphereContacts` | 4.3 s | 4.4 s | +2.2% | 483 MB | 483 MB | +0.0% | ~ |
| `SurfaceParts` | 39.3 s | 39.4 s | +0.2% | 1399 MB | 1398 MB | -0.0% | ~ |
| `Tape` | 16.4 s | 16.6 s | +0.7% | 816 MB | 816 MB | +0.1% | ~ |
| `TapeBVHUnion` | 14.4 s | 14.7 s | +1.5% | 821 MB | 821 MB | +0.0% | ~ |
| **all, in both** | 610.2 s | 616.7 s | +1.1% | 1399 MB | 1398 MB | -0.0% | ~ |


These probes compiled to the same code on both sides and were not run (270 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 669e7f4
base-fingerprint: f7653fee8c5b2beba423cac303718dc728ad6b3e
head: f8fa061
head-fingerprint: a7395912050fbf764c17a1ed09114dd9cb1377f5
cases: 0
rounds: 2
reps: 3
date: 2026-11-15
-->

## Device comparison (CUDA): green

**Accepted:** -

`f8fa061` against `669e7f4` (origin/main), 2026-11-15.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4715.2 s → 4722.0 s (+0.1%), the largest peak 5631 MB → 5632 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.8 s | 12.8 s | +0.6% | 597 MB | 597 MB | +0.0% | ~ |
| `BVHRefit` | 6.6 s | 6.5 s | -0.8% | 503 MB | 503 MB | -0.1% | ~ |
| `BVHUnion` | 24.3 s | 24.4 s | +0.4% | 1022 MB | 1021 MB | -0.2% | ~ |
| `BVHUnionMixed` | 10.7 s | 10.8 s | +1.1% | 622 MB | 622 MB | +0.0% | ~ |
| `ClosestPrimitive` | 127.3 s | 128.0 s | +0.6% | 1345 MB | 1344 MB | -0.0% | ~ |
| `ClosestSurface` | 190.9 s | 191.6 s | +0.4% | 1843 MB | 1844 MB | +0.1% | ~ |
| `Domain` | 58.9 s | 58.3 s | -1.1% | 992 MB | 992 MB | +0.0% | ~ |
| `Fused` | 52.4 s | 53.3 s | +1.7% | 1154 MB | 1155 MB | +0.1% | ~ |
| `LaunchTuning` | 535.6 s | 533.8 s | -0.3% | 3806 MB | 3806 MB | +0.0% | ~ |
| `Mesh` | 19.8 s | 20.2 s | +1.8% | 799 MB | 799 MB | +0.0% | ~ |
| `MeshCSG` | 379.3 s | 379.8 s | +0.1% | 5550 MB | 5550 MB | +0.0% | ~ |
| `MeshReconcile` | 9.9 s | 10.3 s | +3.7% | 510 MB | 510 MB | +0.0% | ~ |
| `PointCloud` | 12.3 s | 12.5 s | +1.3% | 568 MB | 568 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 7.2 s | 7.3 s | +0.8% | 503 MB | 503 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 6.4 s | 6.3 s | -0.3% | 503 MB | 503 MB | +0.0% | ~ |
| `Ray` | 188.3 s | 190.4 s | +1.1% | 3703 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 222.1 s | 222.3 s | +0.1% | 4086 MB | 4086 MB | +0.0% | ~ |
| `RayCrossing` | 434.3 s | 439.2 s | +1.1% | 4017 MB | 4017 MB | +0.0% | ~ |
| `RayParticles` | 673.4 s | 673.3 s | -0.0% | 5631 MB | 5632 MB | +0.0% | ~ |
| `RayShell` | 411.5 s | 411.8 s | +0.1% | 5346 MB | 5345 MB | -0.0% | ~ |
| `Regions` | 357.2 s | 356.3 s | -0.3% | 3840 MB | 3840 MB | +0.0% | ~ |
| `Render` | 235.9 s | 236.5 s | +0.3% | 5386 MB | 5387 MB | +0.0% | ~ |
| `Shapes` | 25.1 s | 25.5 s | +1.6% | 589 MB | 589 MB | +0.0% | ~ |
| `SphereCloudBVH` | 8.4 s | 8.7 s | +3.0% | 503 MB | 503 MB | +0.0% | ~ |
| `SphereCloudRays` | 187.6 s | 187.4 s | -0.1% | 4093 MB | 4093 MB | +0.0% | ~ |
| `SphereContacts` | 8.3 s | 8.2 s | -1.4% | 503 MB | 503 MB | +0.0% | ~ |
| `SurfaceParts` | 403.7 s | 402.3 s | -0.3% | 4242 MB | 4242 MB | +0.0% | ~ |
| `Tape` | 63.3 s | 62.6 s | -1.2% | 979 MB | 979 MB | -0.0% | ~ |
| `TapeBVHUnion` | 41.5 s | 41.3 s | -0.6% | 919 MB | 919 MB | +0.0% | ~ |
| **all, in both** | 4715.2 s | 4722.0 s | +0.1% | 5631 MB | 5632 MB | +0.0% | ~ |

Device kernels: 194 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.


These probes compiled to the same code on both sides and were not run (368 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), LaunchTuning (24), Mesh (8), MeshCSG (18), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Render (6), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
