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
base: 601747f
base-fingerprint: 79ff60a3870f9e06e9c79fcdade041da0e82d640
head: 9bcf4df
head-fingerprint: 01e9d00d88123d3b4251c2eb032724fc4be97258
cases: 12
rounds: 2
reps: 3
date: 2026-11-19
-->

## Host comparison: green

**Accepted:** -

`9bcf4df` against `601747f` (origin/main), 2026-11-19.

**VERDICT: no regression (0 major, 0 minor across 12 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 12 cases

- base: ? (?, simd ?, gpu ?, allocator ?)
- head: 9bcf4df (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/host` | - | - | - | - | - | new |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/host` | - | - | - | - | - | new | grid/morton 1.06x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/host` | - | - | - | - | - | new | grid/shuffled 0.73x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/host` | - | - | - | - | - | new |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/host` | - | - | - | - | - | new | grid/morton 1.26x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/host` | - | - | - | - | - | new | grid/shuffled 0.71x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/host` | - | - | - | - | - | new |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/host` | - | - | - | - | - | new | grid/morton 1.10x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/host` | - | - | - | - | - | new | grid/shuffled 0.45x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/host` | - | - | - | - | - | new |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/host` | - | - | - | - | - | new | grid/morton 1.17x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/host` | - | - | - | - | - | new | grid/shuffled 0.55x |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 784.7 s → 806.9 s (+2.8%), the largest peak 1413 MB → 1413 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 10.1 s | 10.3 s | +1.6% | 558 MB | 558 MB | +0.0% | ~ |
| `BVHRefit` | 4.6 s | 4.5 s | -2.1% | 497 MB | 497 MB | -0.1% | ~ |
| `BVHUnion` | 20.4 s | 20.3 s | -0.5% | 948 MB | 947 MB | -0.1% | ~ |
| `BVHUnionMixed` | 8.6 s | 8.2 s | -3.7% | 555 MB | 555 MB | +0.0% | ~ |
| `ClosestPrimitive` | 31.7 s | 32.4 s | +2.1% | 1298 MB | 1298 MB | -0.0% | ~ |
| `ClosestSurface` | 29.9 s | 29.8 s | -0.2% | 1297 MB | 1297 MB | -0.0% | ~ |
| `Domain` | 19.1 s | 19.2 s | +0.4% | 842 MB | 843 MB | +0.1% | ~ |
| `DomainObject` | 16.8 s | 17.0 s | +1.1% | 846 MB | 846 MB | -0.0% | ~ |
| `EBGrid` | - | 15.3 s | | - | 778 MB | | new |
| `Fused` | 26.1 s | 26.0 s | -0.3% | 1205 MB | 1205 MB | -0.0% | ~ |
| `LaunchTuning` | 39.7 s | 39.2 s | -1.2% | 1279 MB | 1279 MB | +0.0% | ~ |
| `Mesh` | 16.4 s | 16.5 s | +0.6% | 784 MB | 784 MB | -0.1% | ~ |
| `MeshCSG` | 42.6 s | 43.5 s | +1.9% | 1258 MB | 1261 MB | +0.2% | ~ |
| `MeshClosestPoint` | 14.6 s | 14.8 s | +1.6% | 779 MB | 779 MB | -0.0% | ~ |
| `MeshReconcile` | 6.8 s | 6.9 s | +1.0% | 566 MB | 565 MB | -0.2% | ~ |
| `MeshTape` | 48.2 s | 46.7 s | -3.1% | 1279 MB | 1278 MB | -0.0% | ~ |
| `PointCloud` | 7.8 s | 7.8 s | +0.1% | 558 MB | 558 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 4.1 s | 3.9 s | -2.8% | 471 MB | 472 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 3.4 s | 3.4 s | -0.8% | 452 MB | 452 MB | +0.0% | ~ |
| `QueryOrder` | 12.1 s | 12.1 s | +0.6% | 686 MB | 686 MB | +0.0% | ~ |
| `Ray` | 40.6 s | 41.0 s | +0.8% | 1292 MB | 1292 MB | -0.0% | ~ |
| `RayCSG` | 35.6 s | 37.2 s | +4.7% | 1189 MB | 1188 MB | -0.0% | ~ |
| `RayCrossing` | 49.5 s | 50.6 s | +2.1% | 1346 MB | 1347 MB | +0.0% | ~ |
| `RayParticles` | 51.9 s | 57.1 s | +9.9% | 1330 MB | 1330 MB | -0.0% | ~ |
| `RayShell` | 41.2 s | 43.8 s | +6.3% | 1266 MB | 1266 MB | +0.0% | ~ |
| `Regions` | 32.7 s | 34.8 s | +6.4% | 1073 MB | 1073 MB | +0.0% | ~ |
| `Render` | 39.4 s | 42.1 s | +6.9% | 1273 MB | 1274 MB | +0.0% | ~ |
| `Shapes` | 10.8 s | 10.0 s | -7.0% | 554 MB | 554 MB | +0.0% | ~ |
| `SphereCloudBVH` | 5.9 s | 5.9 s | -0.4% | 511 MB | 511 MB | -0.1% | ~ |
| `SphereCloudRays` | 31.0 s | 33.2 s | +7.2% | 1065 MB | 1065 MB | +0.0% | ~ |
| `SphereContacts` | 4.5 s | 4.7 s | +4.1% | 484 MB | 485 MB | +0.2% | ~ |
| `SurfaceParts` | 44.1 s | 47.2 s | +7.1% | 1413 MB | 1413 MB | +0.0% | ~ |
| `Tape` | 18.4 s | 20.0 s | +8.5% | 829 MB | 829 MB | -0.0% | ~ |
| `TapeBVHUnion` | 16.2 s | 16.9 s | +4.1% | 826 MB | 826 MB | +0.0% | ~ |
| **all, in both** | 784.7 s | 806.9 s | +2.8% | 1413 MB | 1413 MB | +0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (308 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), Fused (10), Mesh (12), MeshCSG (12), MeshClosestPoint (8), MeshReconcile (4), MeshTape (16), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/EBGrid/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 601747f
base-fingerprint: 79ff60a3870f9e06e9c79fcdade041da0e82d640
head: 9bcf4df
head-fingerprint: 01e9d00d88123d3b4251c2eb032724fc4be97258
cases: 12
rounds: 2
reps: 3
date: 2026-11-19
-->

## Device comparison (CUDA): green

**Accepted:** -

`9bcf4df` against `601747f` (origin/main), 2026-11-19.

**VERDICT: no regression (0 major, 0 minor across 12 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 12 cases

- base: ? (?, simd ?, gpu ?, allocator ?)
- head: 9bcf4df (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/hostdevice` | - | - | - | - | - | new |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/hostdevice` | - | - | - | - | - | new | grid/morton 1.01x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/hostdevice` | - | - | - | - | - | new | grid/shuffled 0.60x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/hostdevice` | - | - | - | - | - | new |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/hostdevice` | - | - | - | - | - | new | grid/morton 1.33x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/hostdevice` | - | - | - | - | - | new | grid/shuffled 0.76x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/hostdevice` | - | - | - | - | - | new |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/hostdevice` | - | - | - | - | - | new | grid/morton 0.93x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/hostdevice` | - | - | - | - | - | new | grid/shuffled 0.43x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/hostdevice` | - | - | - | - | - | new |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/hostdevice` | - | - | - | - | - | new | grid/morton 1.29x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/hostdevice` | - | - | - | - | - | new | grid/shuffled 0.81x |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 7346.7 s → 6240.9 s (-15.1%), the largest peak 6565 MB → 6565 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 15.1 s | 14.5 s | -3.7% | 606 MB | 605 MB | -0.0% | ~ |
| `BVHRefit` | 8.0 s | 7.5 s | -7.3% | 508 MB | 509 MB | +0.1% | ~ |
| `BVHUnion` | 30.2 s | 27.3 s | -9.7% | 1036 MB | 1036 MB | +0.0% | ~ |
| `BVHUnionMixed` | 12.6 s | 11.8 s | -7.0% | 631 MB | 631 MB | -0.0% | ~ |
| `ClosestPrimitive` | 148.2 s | 140.8 s | -5.0% | 1353 MB | 1353 MB | -0.0% | ~ |
| `ClosestSurface` | 214.7 s | 203.0 s | -5.4% | 1842 MB | 1842 MB | -0.0% | ~ |
| `Domain` | 71.0 s | 64.5 s | -9.0% | 999 MB | 999 MB | +0.0% | ~ |
| `DomainObject` | 67.0 s | 61.6 s | -8.1% | 952 MB | 952 MB | +0.1% | ~ |
| `EBGrid` | - | 20.4 s | | - | 753 MB | | new |
| `Fused` | 63.7 s | 58.5 s | -8.1% | 1163 MB | 1164 MB | +0.0% | ~ |
| `LaunchTuning` | 677.7 s | 557.7 s | -17.7% | 3856 MB | 3875 MB | +0.5% | ~ |
| `Mesh` | 24.1 s | 21.8 s | -9.6% | 810 MB | 810 MB | +0.0% | ~ |
| `MeshCSG` | 504.3 s | 438.5 s | -13.1% | 6302 MB | 6304 MB | +0.0% | ~ |
| `MeshClosestPoint` | 24.4 s | 20.6 s | -15.5% | 762 MB | 762 MB | -0.0% | ~ |
| `MeshReconcile` | 13.2 s | 11.0 s | -16.8% | 513 MB | 514 MB | +0.1% | ~ |
| `MeshTape` | 900.3 s | 755.7 s | -16.1% | 6565 MB | 6565 MB | -0.0% | ~ |
| `PointCloud` | 16.7 s | 13.9 s | -16.7% | 572 MB | 572 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 10.3 s | 7.8 s | -24.6% | 509 MB | 509 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 8.6 s | 7.1 s | -17.4% | 510 MB | 509 MB | -0.0% | ~ |
| `QueryOrder` | 19.4 s | 17.2 s | -11.2% | 694 MB | 694 MB | +0.1% | ~ |
| `Ray` | 211.0 s | 205.0 s | -2.8% | 3768 MB | 3768 MB | +0.0% | ~ |
| `RayCSG` | 267.6 s | 239.2 s | -10.6% | 4154 MB | 4154 MB | -0.0% | ~ |
| `RayCrossing` | 555.3 s | 452.5 s | -18.5% | 4115 MB | 4114 MB | -0.0% | ~ |
| `RayParticles` | 1013.8 s | 852.2 s | -15.9% | 5807 MB | 5805 MB | -0.0% | ~ |
| `RayShell` | 652.9 s | 528.7 s | -19.0% | 5500 MB | 5500 MB | +0.0% | ~ |
| `Regions` | 469.6 s | 392.4 s | -16.4% | 4138 MB | 4137 MB | -0.0% | ~ |
| `Render` | 323.2 s | 287.6 s | -11.0% | 6082 MB | 6083 MB | +0.0% | ~ |
| `Shapes` | 29.5 s | 29.4 s | -0.4% | 593 MB | 594 MB | +0.1% | ~ |
| `SphereCloudBVH` | 9.8 s | 9.5 s | -3.5% | 510 MB | 510 MB | +0.0% | ~ |
| `SphereCloudRays` | 234.0 s | 191.6 s | -18.1% | 4168 MB | 4168 MB | +0.0% | ~ |
| `SphereContacts` | 8.5 s | 8.4 s | -1.4% | 509 MB | 509 MB | +0.0% | ~ |
| `SurfaceParts` | 589.4 s | 484.2 s | -17.8% | 4414 MB | 4414 MB | -0.0% | ~ |
| `Tape` | 98.4 s | 71.2 s | -27.7% | 985 MB | 985 MB | -0.0% | ~ |
| `TapeBVHUnion` | 54.1 s | 48.3 s | -10.7% | 924 MB | 926 MB | +0.3% | ~ |
| **all, in both** | 7346.7 s | 6240.9 s | -15.1% | 6565 MB | 6565 MB | -0.0% | ~ |

Device kernels: 222 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 4 new, 0 gone. Spills are bytes stored/loaded per thread.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (402 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), Fused (10), LaunchTuning (24), Mesh (8), MeshCSG (18), MeshClosestPoint (4), MeshReconcile (2), MeshTape (16), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), QueryOrder (6), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Render (6), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/EBGrid/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt cuda -->
