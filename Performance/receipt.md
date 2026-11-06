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
base: 160782e
base-fingerprint: 2672bc09d5a6f2f5401c396931a62fe9caac005d
head: bd6a8b8
head-fingerprint: 40cd8db2cb69b0f5842ba3be13aa26f18553b4a9
cases: 0
rounds: 2
reps: 3
date: 2026-11-06
-->

## Host comparison: green

**Accepted:** -

`bd6a8b8` against `160782e` (origin/main), 2026-11-06.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 439.5 s → 444.8 s (+1.2%), the largest peak 1264 MB → 1264 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 8.7 s | 8.9 s | +1.9% | 558 MB | 557 MB | -0.1% | ~ |
| `BVHRefit` | 3.8 s | 3.8 s | +0.5% | 454 MB | 454 MB | +0.0% | ~ |
| `BVHUnion` | 14.5 s | 14.8 s | +2.0% | 815 MB | 815 MB | -0.0% | ~ |
| `ClosestPrimitive` | 25.5 s | 26.3 s | +3.3% | 1166 MB | 1166 MB | -0.0% | ~ |
| `Fused` | 20.4 s | 20.8 s | +2.1% | 988 MB | 988 MB | -0.0% | ~ |
| `LaunchTuning` | 32.7 s | 33.1 s | +1.3% | 1108 MB | 1108 MB | +0.0% | ~ |
| `Mesh` | 14.0 s | 14.2 s | +1.4% | 774 MB | 775 MB | +0.2% | ~ |
| `MeshCSG` | 34.0 s | 34.9 s | +2.7% | 1153 MB | 1153 MB | -0.0% | ~ |
| `MeshReconcile` | 6.1 s | 6.3 s | +3.6% | 532 MB | 532 MB | +0.0% | ~ |
| `PointCloud` | 6.9 s | 7.2 s | +4.1% | 558 MB | 557 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 3.4 s | 3.6 s | +4.6% | 406 MB | 406 MB | -0.0% | ~ |
| `PointCloudHashGridBuild` | 3.0 s | 3.1 s | +2.5% | 403 MB | 403 MB | +0.0% | ~ |
| `Ray` | 32.0 s | 32.2 s | +0.5% | 1147 MB | 1147 MB | -0.0% | ~ |
| `RayCSG` | 29.1 s | 29.5 s | +1.3% | 962 MB | 962 MB | -0.1% | ~ |
| `RayCrossing` | 40.8 s | 41.3 s | +1.4% | 1264 MB | 1264 MB | -0.0% | ~ |
| `RayParticles` | 43.6 s | 43.5 s | -0.3% | 1261 MB | 1261 MB | -0.0% | ~ |
| `RayShell` | 33.4 s | 33.1 s | -0.7% | 1056 MB | 1056 MB | +0.0% | ~ |
| `Render` | 31.8 s | 31.9 s | +0.4% | 1054 MB | 1053 MB | -0.0% | ~ |
| `Shapes` | 8.8 s | 8.9 s | +0.9% | 552 MB | 552 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.5 s | 4.6 s | +2.5% | 476 MB | 476 MB | -0.0% | ~ |
| `SphereCloudRays` | 24.5 s | 24.7 s | +0.8% | 963 MB | 963 MB | -0.0% | ~ |
| `SphereContacts` | 4.1 s | 4.1 s | -0.6% | 424 MB | 424 MB | -0.0% | ~ |
| `Tape` | 14.0 s | 14.0 s | +0.2% | 825 MB | 826 MB | +0.2% | ~ |
| **all, in both** | 439.5 s | 444.8 s | +1.2% | 1264 MB | 1264 MB | -0.0% | ~ |


These probes compiled to the same code on both sides and were not run (224 cases): BVH (6), BVHRefit (2), BVHUnion (12), ClosestPrimitive (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), Tape (20)

<!-- END receipt host -->
