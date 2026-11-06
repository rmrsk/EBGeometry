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
head: 28ecc5c
head-fingerprint: fd9b02b4f864366cc121be1fb814eaabd82f7105
cases: 0
rounds: 2
reps: 3
date: 2026-11-06
-->

## Host comparison: green

**Accepted:** -

`28ecc5c` against `0075a47` (origin/main), 2026-11-06.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 449.0 s → 444.4 s (-1.0%), the largest peak 1264 MB → 1264 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.0 s | 9.0 s | +0.0% | 557 MB | 557 MB | +0.0% | ~ |
| `BVHRefit` | 3.8 s | 3.8 s | +0.2% | 454 MB | 454 MB | +0.0% | ~ |
| `BVHUnion` | 14.8 s | 14.8 s | -0.2% | 814 MB | 814 MB | +0.0% | ~ |
| `ClosestPrimitive` | 26.0 s | 26.0 s | -0.2% | 1166 MB | 1166 MB | -0.0% | ~ |
| `Fused` | 20.9 s | 20.8 s | -0.3% | 988 MB | 988 MB | -0.1% | ~ |
| `LaunchTuning` | 33.8 s | 33.2 s | -1.7% | 1108 MB | 1107 MB | -0.0% | ~ |
| `Mesh` | 14.3 s | 14.2 s | -0.5% | 774 MB | 775 MB | +0.1% | ~ |
| `MeshCSG` | 35.7 s | 35.0 s | -2.0% | 1154 MB | 1153 MB | -0.0% | ~ |
| `MeshReconcile` | 6.3 s | 6.4 s | +0.5% | 532 MB | 532 MB | -0.1% | ~ |
| `PointCloud` | 7.2 s | 7.1 s | -1.2% | 557 MB | 557 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.6 s | 3.6 s | +0.4% | 406 MB | 406 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.1 s | 3.1 s | +1.4% | 403 MB | 402 MB | -0.1% | ~ |
| `Ray` | 32.9 s | 32.0 s | -2.8% | 1146 MB | 1147 MB | +0.0% | ~ |
| `RayCSG` | 29.6 s | 29.0 s | -2.2% | 962 MB | 962 MB | +0.0% | ~ |
| `RayCrossing` | 42.0 s | 41.3 s | -1.6% | 1264 MB | 1264 MB | +0.0% | ~ |
| `RayParticles` | 44.1 s | 44.1 s | -0.1% | 1261 MB | 1261 MB | -0.0% | ~ |
| `RayShell` | 33.5 s | 33.1 s | -1.2% | 1056 MB | 1056 MB | +0.1% | ~ |
| `Render` | 31.7 s | 31.7 s | -0.1% | 1054 MB | 1054 MB | -0.0% | ~ |
| `Shapes` | 9.1 s | 8.9 s | -2.6% | 552 MB | 552 MB | +0.0% | ~ |
| `SphereCloudBVH` | 4.7 s | 4.7 s | -0.3% | 476 MB | 476 MB | +0.1% | ~ |
| `SphereCloudRays` | 24.7 s | 24.6 s | -0.4% | 962 MB | 962 MB | -0.0% | ~ |
| `SphereContacts` | 4.1 s | 4.1 s | +1.8% | 425 MB | 424 MB | -0.1% | ~ |
| `Tape` | 14.1 s | 14.0 s | -0.2% | 825 MB | 824 MB | -0.1% | ~ |
| **all, in both** | 449.0 s | 444.4 s | -1.0% | 1264 MB | 1264 MB | +0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (224 cases): BVH (6), BVHRefit (2), BVHUnion (12), ClosestPrimitive (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), Tape (20)

<!-- END receipt host -->
