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
base: 6df899f
base-fingerprint: db167e0f22feeae8b3cf43e92dc20a313a139fd8
head: a78306d
head-fingerprint: 2ebd9bed29e3a6ee9df443c1efe9a3bd39833f40
cases: 0
rounds: 2
reps: 3
date: 2026-11-16
-->

## Host comparison: green

**Accepted:** -

`a78306d` against `6df899f` (origin/main), 2026-11-16.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 610.1 s → 618.9 s (+1.4%), the largest peak 1398 MB → 1398 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.0 s | 9.0 s | -0.1% | 555 MB | 556 MB | +0.1% | ~ |
| `BVHRefit` | 3.9 s | 3.9 s | +0.2% | 483 MB | 483 MB | +0.0% | ~ |
| `BVHUnion` | 17.7 s | 17.9 s | +0.8% | 895 MB | 895 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.3 s | 7.3 s | -0.0% | 555 MB | 556 MB | +0.1% | ~ |
| `ClosestPrimitive` | 28.9 s | 29.3 s | +1.3% | 1263 MB | 1263 MB | -0.0% | ~ |
| `ClosestSurface` | 26.9 s | 27.6 s | +2.4% | 1220 MB | 1220 MB | +0.0% | ~ |
| `Domain` | 17.5 s | 17.7 s | +1.0% | 839 MB | 839 MB | -0.0% | ~ |
| `Fused` | 23.4 s | 23.6 s | +1.1% | 1126 MB | 1125 MB | -0.0% | ~ |
| `LaunchTuning` | 34.4 s | 35.5 s | +3.3% | 1268 MB | 1268 MB | -0.0% | ~ |
| `Mesh` | 14.2 s | 14.7 s | +3.3% | 783 MB | 783 MB | +0.0% | ~ |
| `MeshCSG` | 37.2 s | 37.8 s | +1.6% | 1282 MB | 1282 MB | +0.0% | ~ |
| `MeshReconcile` | 6.3 s | 6.7 s | +5.3% | 565 MB | 565 MB | +0.0% | ~ |
| `PointCloud` | 7.0 s | 7.2 s | +2.5% | 558 MB | 558 MB | -0.0% | ~ |
| `PointCloudBVHBuild` | 3.6 s | 3.8 s | +5.1% | 470 MB | 469 MB | -0.1% | ~ |
| `PointCloudHashGridBuild` | 3.1 s | 3.2 s | +3.2% | 451 MB | 451 MB | +0.1% | ~ |
| `Ray` | 34.4 s | 35.8 s | +3.8% | 1317 MB | 1318 MB | +0.0% | ~ |
| `RayCSG` | 30.2 s | 30.5 s | +0.8% | 1114 MB | 1114 MB | +0.0% | ~ |
| `RayCrossing` | 43.4 s | 44.4 s | +2.4% | 1342 MB | 1341 MB | -0.1% | ~ |
| `RayParticles` | 47.1 s | 47.5 s | +0.8% | 1328 MB | 1328 MB | -0.0% | ~ |
| `RayShell` | 36.5 s | 36.6 s | +0.2% | 1270 MB | 1270 MB | -0.0% | ~ |
| `Regions` | 27.8 s | 28.0 s | +0.7% | 1069 MB | 1069 MB | +0.0% | ~ |
| `Render` | 34.8 s | 35.1 s | +0.6% | 1266 MB | 1266 MB | +0.0% | ~ |
| `Shapes` | 8.8 s | 8.7 s | -1.4% | 553 MB | 553 MB | +0.0% | ~ |
| `SphereCloudBVH` | 4.9 s | 4.8 s | -0.6% | 510 MB | 510 MB | -0.1% | ~ |
| `SphereCloudRays` | 27.2 s | 27.3 s | +0.5% | 1059 MB | 1059 MB | +0.0% | ~ |
| `SphereContacts` | 4.2 s | 4.3 s | +3.0% | 484 MB | 483 MB | -0.1% | ~ |
| `SurfaceParts` | 39.2 s | 39.4 s | +0.5% | 1398 MB | 1398 MB | +0.0% | ~ |
| `Tape` | 16.5 s | 16.6 s | +0.3% | 817 MB | 817 MB | +0.0% | ~ |
| `TapeBVHUnion` | 14.4 s | 14.7 s | +2.2% | 822 MB | 821 MB | -0.1% | ~ |
| **all, in both** | 610.1 s | 618.9 s | +1.4% | 1398 MB | 1398 MB | +0.0% | ~ |


These probes compiled to the same code on both sides and were not run (270 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/BVH/main.cpp, Performance/BVHRefit/main.cpp, Performance/BVHUnion/main.cpp, Performance/BVHUnionMixed/main.cpp, Performance/ClosestPrimitive/main.cpp, Performance/ClosestSurface/main.cpp, Performance/Domain/main.cpp, Performance/Fused/main.cpp, Performance/LaunchTuning/main.cpp, Performance/Mesh/main.cpp, Performance/MeshCSG/main.cpp, Performance/MeshReconcile/main.cpp, Performance/PointCloud/main.cpp, Performance/PointCloudBVHBuild/main.cpp, Performance/PointCloudHashGridBuild/main.cpp, Performance/Ray/main.cpp, Performance/RayCSG/main.cpp, Performance/RayCrossing/main.cpp, Performance/RayParticles/main.cpp, Performance/RayShell/main.cpp, Performance/Regions/main.cpp, Performance/Render/main.cpp, Performance/Shapes/main.cpp, Performance/SphereCloudBVH/main.cpp, Performance/SphereCloudRays/main.cpp, Performance/SphereContacts/main.cpp, Performance/SurfaceParts/main.cpp, Performance/Tape/main.cpp, Performance/TapeBVHUnion/main.cpp, Performance/Common/Case.hpp, Performance/Common/Device.hpp, Performance/Common/Environment.hpp, Performance/Common/Format.hpp, Performance/Common/Inputs.hpp, Performance/Common/PerfHarness.hpp, Performance/Common/Probe.hpp, Performance/Common/Report.hpp, Performance/Common/Schema.hpp, Performance/Common/Statistics.hpp, Performance/Common/Suite.hpp

<!-- END receipt host -->
