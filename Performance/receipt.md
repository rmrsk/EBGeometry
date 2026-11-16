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
base: 1f4517c
base-fingerprint: a7395912050fbf764c17a1ed09114dd9cb1377f5
head: 53dac04
head-fingerprint: 1c0f10afe259b64b8e299f247428e810a3b3b6eb
cases: 0
rounds: 2
reps: 3
date: 2026-11-15
-->

## Host comparison: green

**Accepted:** -

`53dac04` against `1f4517c` (origin/main), 2026-11-15.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 610.5 s → 618.6 s (+1.3%), the largest peak 1398 MB → 1398 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.0 s | 9.0 s | -0.3% | 555 MB | 555 MB | -0.0% | ~ |
| `BVHRefit` | 3.9 s | 3.9 s | -0.1% | 482 MB | 483 MB | +0.1% | ~ |
| `BVHUnion` | 17.8 s | 17.9 s | +0.7% | 894 MB | 894 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.4 s | 7.3 s | -1.5% | 555 MB | 555 MB | +0.0% | ~ |
| `ClosestPrimitive` | 28.7 s | 29.5 s | +2.6% | 1259 MB | 1262 MB | +0.2% | ~ |
| `ClosestSurface` | 26.9 s | 27.5 s | +2.2% | 1219 MB | 1219 MB | +0.0% | ~ |
| `Domain` | 17.5 s | 17.8 s | +1.5% | 838 MB | 838 MB | +0.0% | ~ |
| `Fused` | 23.3 s | 23.9 s | +2.4% | 1124 MB | 1124 MB | +0.0% | ~ |
| `LaunchTuning` | 34.5 s | 35.1 s | +1.7% | 1268 MB | 1268 MB | -0.0% | ~ |
| `Mesh` | 14.4 s | 14.5 s | +0.9% | 785 MB | 785 MB | +0.1% | ~ |
| `MeshCSG` | 37.1 s | 37.6 s | +1.5% | 1281 MB | 1283 MB | +0.1% | ~ |
| `MeshReconcile` | 6.3 s | 6.6 s | +3.8% | 564 MB | 564 MB | -0.0% | ~ |
| `PointCloud` | 7.0 s | 7.2 s | +2.3% | 558 MB | 558 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.6 s | 3.7 s | +3.2% | 469 MB | 469 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 3.1 s | 3.3 s | +4.5% | 451 MB | 451 MB | +0.0% | ~ |
| `Ray` | 34.5 s | 35.3 s | +2.2% | 1317 MB | 1317 MB | +0.0% | ~ |
| `RayCSG` | 30.3 s | 30.5 s | +0.5% | 1114 MB | 1112 MB | -0.1% | ~ |
| `RayCrossing` | 43.8 s | 44.3 s | +1.0% | 1342 MB | 1342 MB | +0.0% | ~ |
| `RayParticles` | 47.3 s | 48.2 s | +2.0% | 1318 MB | 1314 MB | -0.3% | ~ |
| `RayShell` | 35.8 s | 36.4 s | +1.9% | 1238 MB | 1238 MB | +0.0% | ~ |
| `Regions` | 27.6 s | 28.3 s | +2.3% | 1065 MB | 1066 MB | +0.1% | ~ |
| `Render` | 34.9 s | 34.9 s | +0.0% | 1261 MB | 1262 MB | +0.1% | ~ |
| `Shapes` | 8.8 s | 8.7 s | -1.4% | 553 MB | 553 MB | -0.0% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.9 s | +1.4% | 509 MB | 509 MB | -0.0% | ~ |
| `SphereCloudRays` | 27.2 s | 27.5 s | +1.2% | 1058 MB | 1058 MB | +0.0% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | -0.4% | 483 MB | 483 MB | +0.0% | ~ |
| `SurfaceParts` | 39.2 s | 39.3 s | +0.3% | 1398 MB | 1398 MB | -0.0% | ~ |
| `Tape` | 16.6 s | 16.7 s | +0.5% | 816 MB | 817 MB | +0.1% | ~ |
| `TapeBVHUnion` | 14.6 s | 14.5 s | -0.9% | 820 MB | 822 MB | +0.1% | ~ |
| **all, in both** | 610.5 s | 618.6 s | +1.3% | 1398 MB | 1398 MB | -0.0% | ~ |


These probes compiled to the same code on both sides and were not run (270 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 1f4517c
base-fingerprint: a7395912050fbf764c17a1ed09114dd9cb1377f5
head: 53dac04
head-fingerprint: 1c0f10afe259b64b8e299f247428e810a3b3b6eb
cases: 0
rounds: 2
reps: 3
date: 2026-11-15
-->

## Device comparison (CUDA): green

**Accepted:** -

`53dac04` against `1f4517c` (origin/main), 2026-11-15.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4731.8 s → 4747.2 s (+0.3%), the largest peak 5631 MB → 5632 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.9 s | 13.1 s | +2.0% | 597 MB | 598 MB | +0.2% | ~ |
| `BVHRefit` | 6.6 s | 6.6 s | +0.8% | 503 MB | 504 MB | +0.2% | ~ |
| `BVHUnion` | 24.2 s | 24.8 s | +2.4% | 1022 MB | 1024 MB | +0.1% | ~ |
| `BVHUnionMixed` | 10.7 s | 11.0 s | +2.7% | 622 MB | 624 MB | +0.2% | ~ |
| `ClosestPrimitive` | 128.4 s | 129.9 s | +1.2% | 1344 MB | 1346 MB | +0.1% | ~ |
| `ClosestSurface` | 192.2 s | 194.8 s | +1.4% | 1844 MB | 1843 MB | -0.0% | ~ |
| `Domain` | 57.9 s | 58.3 s | +0.7% | 992 MB | 993 MB | +0.2% | ~ |
| `Fused` | 52.0 s | 52.7 s | +1.3% | 1155 MB | 1156 MB | +0.1% | ~ |
| `LaunchTuning` | 537.6 s | 538.2 s | +0.1% | 3806 MB | 3806 MB | +0.0% | ~ |
| `Mesh` | 20.0 s | 20.2 s | +0.8% | 799 MB | 800 MB | +0.1% | ~ |
| `MeshCSG` | 381.4 s | 382.8 s | +0.4% | 5550 MB | 5550 MB | +0.0% | ~ |
| `MeshReconcile` | 10.1 s | 9.9 s | -1.2% | 509 MB | 510 MB | +0.2% | ~ |
| `PointCloud` | 12.7 s | 12.7 s | +0.7% | 568 MB | 569 MB | +0.2% | ~ |
| `PointCloudBVHBuild` | 7.1 s | 7.3 s | +3.7% | 503 MB | 504 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 6.5 s | 6.3 s | -3.1% | 503 MB | 504 MB | +0.2% | ~ |
| `Ray` | 191.1 s | 191.1 s | -0.0% | 3703 MB | 3703 MB | -0.0% | ~ |
| `RayCSG` | 220.7 s | 223.1 s | +1.1% | 4086 MB | 4086 MB | +0.0% | ~ |
| `RayCrossing` | 433.8 s | 439.1 s | +1.2% | 4017 MB | 4017 MB | +0.0% | ~ |
| `RayParticles` | 674.0 s | 676.1 s | +0.3% | 5631 MB | 5632 MB | +0.0% | ~ |
| `RayShell` | 412.3 s | 414.0 s | +0.4% | 5346 MB | 5345 MB | -0.0% | ~ |
| `Regions` | 358.8 s | 359.7 s | +0.3% | 3834 MB | 3835 MB | +0.0% | ~ |
| `Render` | 236.0 s | 237.3 s | +0.5% | 5387 MB | 5386 MB | -0.0% | ~ |
| `Shapes` | 25.3 s | 25.4 s | +0.6% | 589 MB | 591 MB | +0.2% | ~ |
| `SphereCloudBVH` | 8.3 s | 8.6 s | +4.4% | 503 MB | 504 MB | +0.2% | ~ |
| `SphereCloudRays` | 189.2 s | 187.1 s | -1.1% | 4093 MB | 4093 MB | -0.0% | ~ |
| `SphereContacts` | 8.2 s | 8.4 s | +2.4% | 503 MB | 504 MB | +0.2% | ~ |
| `SurfaceParts` | 407.4 s | 405.0 s | -0.6% | 4242 MB | 4242 MB | +0.0% | ~ |
| `Tape` | 64.0 s | 62.5 s | -2.5% | 979 MB | 980 MB | +0.1% | ~ |
| `TapeBVHUnion` | 42.6 s | 41.0 s | -3.8% | 919 MB | 920 MB | +0.1% | ~ |
| **all, in both** | 4731.8 s | 4747.2 s | +0.3% | 5631 MB | 5632 MB | +0.0% | ~ |

Device kernels: 194 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.


These probes compiled to the same code on both sides and were not run (368 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), LaunchTuning (24), Mesh (8), MeshCSG (18), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Render (6), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
