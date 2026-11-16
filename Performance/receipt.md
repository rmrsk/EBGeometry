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
base: 4bf90e9
base-fingerprint: 30b6edfbe4d67af90dcaf2ebe131bc7df88464b1
head: ee80764
head-fingerprint: 68cd8f483a104437773ea71e1de1076b013ffcc6
cases: 8
rounds: 2
reps: 3
date: 2026-11-16
-->

## Host comparison: green

**Accepted:** -

`ee80764` against `4bf90e9` (origin/main), 2026-11-16.

**VERDICT: no regression (0 major, 0 minor across 8 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 8 cases

- base: ? (?, simd ?, gpu ?, allocator ?)
- head: ee80764 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `DomainObject/partsWhole/labels/float/host` | - | - | - | - | - | new |  |
| `DomainObject/partsWhole/labelAt/float/host` | - | - | - | - | - | new |  |
| `DomainObject/partsOwn/labels/float/host` | - | - | - | - | - | new | own/whole 70.69x |
| `DomainObject/partsOwn/labelAt/float/host` | - | - | - | - | - | new | own/whole 70.38x |
| `DomainObject/partsWhole/labels/double/host` | - | - | - | - | - | new |  |
| `DomainObject/partsWhole/labelAt/double/host` | - | - | - | - | - | new |  |
| `DomainObject/partsOwn/labels/double/host` | - | - | - | - | - | new | own/whole 67.83x |
| `DomainObject/partsOwn/labelAt/double/host` | - | - | - | - | - | new | own/whole 71.21x |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 616.5 s → 620.0 s (+0.6%), the largest peak 1398 MB → 1399 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.1 s | -0.2% | 556 MB | 556 MB | +0.1% | ~ |
| `BVHRefit` | 3.9 s | 3.9 s | -0.0% | 482 MB | 483 MB | +0.2% | ~ |
| `BVHUnion` | 17.8 s | 17.9 s | +0.5% | 894 MB | 896 MB | +0.1% | ~ |
| `BVHUnionMixed` | 7.3 s | 7.3 s | +0.5% | 556 MB | 556 MB | +0.1% | ~ |
| `ClosestPrimitive` | 28.9 s | 29.3 s | +1.3% | 1263 MB | 1265 MB | +0.1% | ~ |
| `ClosestSurface` | 27.1 s | 27.3 s | +0.7% | 1220 MB | 1218 MB | -0.1% | ~ |
| `Domain` | 17.5 s | 17.7 s | +1.1% | 839 MB | 840 MB | +0.1% | ~ |
| `DomainObject` | - | 15.5 s | | - | 843 MB | | new |
| `Fused` | 23.6 s | 23.8 s | +1.1% | 1126 MB | 1126 MB | +0.0% | ~ |
| `LaunchTuning` | 34.9 s | 35.8 s | +2.8% | 1268 MB | 1303 MB | +2.7% | ~ |
| `Mesh` | 14.4 s | 14.6 s | +1.7% | 784 MB | 784 MB | +0.1% | ~ |
| `MeshCSG` | 37.6 s | 38.2 s | +1.5% | 1282 MB | 1255 MB | -2.2% | ~ |
| `MeshReconcile` | 6.4 s | 6.7 s | +5.1% | 565 MB | 565 MB | +0.0% | ~ |
| `PointCloud` | 7.2 s | 7.2 s | -0.1% | 558 MB | 559 MB | +0.2% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.8 s | +0.1% | 470 MB | 470 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | -2.2% | 451 MB | 451 MB | +0.1% | ~ |
| `Ray` | 35.2 s | 35.0 s | -0.6% | 1318 MB | 1317 MB | -0.0% | ~ |
| `RayCSG` | 30.8 s | 30.8 s | -0.0% | 1113 MB | 1173 MB | +5.3% | ~ |
| `RayCrossing` | 45.0 s | 45.0 s | -0.1% | 1342 MB | 1334 MB | -0.5% | ~ |
| `RayParticles` | 47.7 s | 47.7 s | -0.1% | 1327 MB | 1319 MB | -0.6% | ~ |
| `RayShell` | 36.0 s | 36.4 s | +1.1% | 1270 MB | 1276 MB | +0.4% | ~ |
| `Regions` | 27.8 s | 28.1 s | +1.3% | 1069 MB | 1070 MB | +0.0% | ~ |
| `Render` | 35.1 s | 35.3 s | +0.6% | 1267 MB | 1282 MB | +1.2% | ~ |
| `Shapes` | 8.8 s | 8.9 s | +1.4% | 553 MB | 554 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.9 s | +0.8% | 510 MB | 511 MB | +0.2% | ~ |
| `SphereCloudRays` | 27.5 s | 27.2 s | -1.0% | 1060 MB | 1060 MB | +0.0% | ~ |
| `SphereContacts` | 4.2 s | 4.2 s | -0.1% | 483 MB | 484 MB | +0.2% | ~ |
| `SurfaceParts` | 39.4 s | 39.2 s | -0.5% | 1398 MB | 1399 MB | +0.1% | ~ |
| `Tape` | 16.5 s | 16.6 s | +0.7% | 818 MB | 819 MB | +0.1% | ~ |
| `TapeBVHUnion` | 14.6 s | 14.4 s | -1.3% | 822 MB | 823 MB | +0.1% | ~ |
| **all, in both** | 616.5 s | 620.0 s | +0.6% | 1398 MB | 1399 MB | +0.1% | ~ |


These probes compiled to the same code on both sides and were not run (270 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/DomainObject/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt host -->
