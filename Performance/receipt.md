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
base: 0075a47
base-fingerprint: 40cd8db2cb69b0f5842ba3be13aa26f18553b4a9
head: da8d6ba
head-fingerprint: b3d98b4c4254e9729fc5d48c3cb8238021bad874
cases: 0
rounds: 2
reps: 3
date: 2026-11-06
-->

## Host comparison: green

**Accepted:** -

`da8d6ba` against `0075a47` (origin/main), 2026-11-06.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 511.8 s → 509.2 s (-0.5%), the largest peak 1263 MB → 1263 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 10.4 s | 10.2 s | -2.3% | 556 MB | 556 MB | +0.0% | ~ |
| `BVHRefit` | 4.2 s | 4.2 s | -0.4% | 453 MB | 453 MB | -0.0% | ~ |
| `BVHUnion` | 17.1 s | 16.7 s | -2.3% | 814 MB | 814 MB | -0.0% | ~ |
| `ClosestPrimitive` | 30.0 s | 29.8 s | -0.8% | 1165 MB | 1166 MB | +0.0% | ~ |
| `Fused` | 24.0 s | 23.7 s | -0.9% | 987 MB | 988 MB | +0.0% | ~ |
| `LaunchTuning` | 38.5 s | 38.2 s | -1.0% | 1106 MB | 1107 MB | +0.1% | ~ |
| `Mesh` | 16.6 s | 16.2 s | -2.2% | 774 MB | 774 MB | -0.1% | ~ |
| `MeshCSG` | 39.9 s | 39.5 s | -1.0% | 1152 MB | 1152 MB | +0.0% | ~ |
| `MeshReconcile` | 7.2 s | 7.1 s | -1.8% | 532 MB | 531 MB | -0.1% | ~ |
| `PointCloud` | 8.3 s | 8.0 s | -2.9% | 556 MB | 556 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 4.1 s | 3.9 s | -3.7% | 405 MB | 405 MB | -0.0% | ~ |
| `PointCloudHashGridBuild` | 3.5 s | 3.4 s | -1.2% | 402 MB | 402 MB | -0.0% | ~ |
| `Ray` | 37.1 s | 37.1 s | -0.1% | 1146 MB | 1146 MB | +0.0% | ~ |
| `RayCSG` | 33.7 s | 33.7 s | -0.0% | 961 MB | 962 MB | +0.1% | ~ |
| `RayCrossing` | 47.6 s | 47.7 s | +0.3% | 1263 MB | 1263 MB | +0.0% | ~ |
| `RayParticles` | 50.5 s | 50.3 s | -0.3% | 1260 MB | 1260 MB | +0.0% | ~ |
| `RayShell` | 38.2 s | 38.2 s | -0.2% | 1054 MB | 1055 MB | +0.1% | ~ |
| `Render` | 36.3 s | 36.5 s | +0.6% | 1052 MB | 1053 MB | +0.0% | ~ |
| `Shapes` | 10.2 s | 10.1 s | -0.9% | 551 MB | 551 MB | +0.1% | ~ |
| `SphereCloudBVH` | 5.0 s | 5.1 s | +1.1% | 475 MB | 475 MB | -0.0% | ~ |
| `SphereCloudRays` | 29.0 s | 29.0 s | -0.2% | 961 MB | 962 MB | +0.0% | ~ |
| `SphereContacts` | 4.6 s | 4.5 s | -0.3% | 424 MB | 424 MB | +0.1% | ~ |
| `Tape` | 15.9 s | 16.2 s | +1.8% | 825 MB | 824 MB | -0.1% | ~ |
| **all, in both** | 511.8 s | 509.2 s | -0.5% | 1263 MB | 1263 MB | +0.0% | ~ |


These probes compiled to the same code on both sides and were not run (224 cases): BVH (6), BVHRefit (2), BVHUnion (12), ClosestPrimitive (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), Tape (20)

<!-- END receipt host -->
