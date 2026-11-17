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
head: e45a1e6
head-fingerprint: f6a77e99a55a926ac48bef8ad62b5634073733db
cases: 8
rounds: 2
reps: 3
date: 2026-11-17
-->

## Host comparison: green

**Accepted:** -

`e45a1e6` against `4bf90e9` (origin/main), 2026-11-17.

**VERDICT: no regression (0 major, 0 minor across 8 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 8 cases

- base: ? (?, simd ?, gpu ?, allocator ?)
- head: e45a1e6 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `DomainObject/partsWhole/labels/float/host` | - | - | - | - | - | new |  |
| `DomainObject/partsWhole/labelAt/float/host` | - | - | - | - | - | new |  |
| `DomainObject/partsOwn/labels/float/host` | - | - | - | - | - | new | own/whole 69.76x |
| `DomainObject/partsOwn/labelAt/float/host` | - | - | - | - | - | new | own/whole 71.38x |
| `DomainObject/partsWhole/labels/double/host` | - | - | - | - | - | new |  |
| `DomainObject/partsWhole/labelAt/double/host` | - | - | - | - | - | new |  |
| `DomainObject/partsOwn/labels/double/host` | - | - | - | - | - | new | own/whole 67.12x |
| `DomainObject/partsOwn/labelAt/double/host` | - | - | - | - | - | new | own/whole 71.72x |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 616.5 s → 614.4 s (-0.4%), the largest peak 1398 MB → 1399 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.0 s | -0.8% | 556 MB | 556 MB | +0.1% | ~ |
| `BVHRefit` | 3.9 s | 3.9 s | -1.0% | 482 MB | 484 MB | +0.3% | ~ |
| `BVHUnion` | 17.8 s | 17.7 s | -0.7% | 894 MB | 895 MB | +0.1% | ~ |
| `BVHUnionMixed` | 7.3 s | 7.3 s | -0.5% | 556 MB | 556 MB | +0.1% | ~ |
| `ClosestPrimitive` | 28.9 s | 28.9 s | +0.1% | 1263 MB | 1264 MB | +0.1% | ~ |
| `ClosestSurface` | 27.1 s | 27.0 s | -0.5% | 1220 MB | 1218 MB | -0.2% | ~ |
| `Domain` | 17.5 s | 17.6 s | +0.2% | 839 MB | 840 MB | +0.2% | ~ |
| `DomainObject` | - | 15.4 s | | - | 843 MB | | new |
| `Fused` | 23.6 s | 23.3 s | -1.1% | 1126 MB | 1127 MB | +0.0% | ~ |
| `LaunchTuning` | 34.9 s | 35.0 s | +0.3% | 1268 MB | 1303 MB | +2.7% | ~ |
| `Mesh` | 14.4 s | 14.3 s | -0.6% | 784 MB | 784 MB | +0.1% | ~ |
| `MeshCSG` | 37.6 s | 37.9 s | +0.8% | 1282 MB | 1255 MB | -2.1% | ~ |
| `MeshReconcile` | 6.4 s | 6.5 s | +1.2% | 565 MB | 566 MB | +0.1% | ~ |
| `PointCloud` | 7.2 s | 7.2 s | -1.2% | 558 MB | 559 MB | +0.2% | ~ |
| `PointCloudBVHBuild` | 3.8 s | 3.9 s | +1.2% | 470 MB | 470 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.2 s | -3.4% | 451 MB | 452 MB | +0.2% | ~ |
| `Ray` | 35.2 s | 35.0 s | -0.5% | 1318 MB | 1316 MB | -0.1% | ~ |
| `RayCSG` | 30.8 s | 30.4 s | -1.2% | 1113 MB | 1174 MB | +5.4% | ~ |
| `RayCrossing` | 45.0 s | 44.8 s | -0.5% | 1342 MB | 1335 MB | -0.5% | ~ |
| `RayParticles` | 47.7 s | 47.2 s | -1.1% | 1327 MB | 1319 MB | -0.6% | ~ |
| `RayShell` | 36.0 s | 36.6 s | +1.6% | 1270 MB | 1276 MB | +0.4% | ~ |
| `Regions` | 27.8 s | 27.8 s | -0.0% | 1069 MB | 1070 MB | +0.0% | ~ |
| `Render` | 35.1 s | 34.8 s | -1.0% | 1267 MB | 1282 MB | +1.2% | ~ |
| `Shapes` | 8.8 s | 8.8 s | -0.4% | 553 MB | 554 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.7 s | -2.3% | 510 MB | 510 MB | +0.2% | ~ |
| `SphereCloudRays` | 27.5 s | 27.2 s | -1.2% | 1060 MB | 1060 MB | +0.0% | ~ |
| `SphereContacts` | 4.2 s | 4.3 s | +1.8% | 483 MB | 484 MB | +0.2% | ~ |
| `SurfaceParts` | 39.4 s | 39.1 s | -0.6% | 1398 MB | 1399 MB | +0.1% | ~ |
| `Tape` | 16.5 s | 16.6 s | +0.2% | 818 MB | 819 MB | +0.1% | ~ |
| `TapeBVHUnion` | 14.6 s | 14.5 s | -0.6% | 822 MB | 822 MB | +0.0% | ~ |
| **all, in both** | 616.5 s | 614.4 s | -0.4% | 1398 MB | 1399 MB | +0.1% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (270 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/DomainObject/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 4bf90e9
base-fingerprint: 30b6edfbe4d67af90dcaf2ebe131bc7df88464b1
head: d94a68b
head-fingerprint: f6a77e99a55a926ac48bef8ad62b5634073733db
cases: 8
rounds: 2
reps: 3
date: 2026-11-17
-->

## Device comparison (CUDA): green

**Accepted:** -

`d94a68b` against `4bf90e9` (origin/main), 2026-11-17.

**VERDICT: no regression (0 major, 0 minor across 8 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 8 cases

- base: ? (?, simd ?, gpu ?, allocator ?)
- head: d94a68b (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `DomainObject/partsWhole/labels/float/hostdevice` | - | - | - | - | - | new |  |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | - | - | - | - | - | new |  |
| `DomainObject/partsOwn/labels/float/hostdevice` | - | - | - | - | - | new | own/whole 46.08x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | - | - | - | - | - | new | own/whole 58.94x |
| `DomainObject/partsWhole/labels/double/hostdevice` | - | - | - | - | - | new |  |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | - | - | - | - | - | new |  |
| `DomainObject/partsOwn/labels/double/hostdevice` | - | - | - | - | - | new | own/whole 50.62x |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | - | - | - | - | - | new | own/whole 48.13x |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4754.0 s → 4750.9 s (-0.1%), the largest peak 6080 MB → 6080 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.8 s | 12.9 s | +0.6% | 599 MB | 600 MB | +0.2% | ~ |
| `BVHRefit` | 6.6 s | 6.5 s | -0.8% | 505 MB | 505 MB | +0.1% | ~ |
| `BVHUnion` | 24.6 s | 24.4 s | -0.8% | 1024 MB | 1024 MB | -0.0% | ~ |
| `BVHUnionMixed` | 10.9 s | 10.9 s | -0.1% | 624 MB | 624 MB | -0.1% | ~ |
| `ClosestPrimitive` | 127.9 s | 126.1 s | -1.4% | 1346 MB | 1347 MB | +0.0% | ~ |
| `ClosestSurface` | 191.9 s | 188.8 s | -1.7% | 1844 MB | 1844 MB | +0.0% | ~ |
| `Domain` | 57.9 s | 59.3 s | +2.3% | 993 MB | 994 MB | +0.1% | ~ |
| `DomainObject` | - | 56.5 s | | - | 948 MB | | new |
| `Fused` | 52.6 s | 54.3 s | +3.3% | 1157 MB | 1158 MB | +0.0% | ~ |
| `LaunchTuning` | 538.1 s | 536.3 s | -0.3% | 3806 MB | 3806 MB | +0.0% | ~ |
| `Mesh` | 20.0 s | 20.1 s | +0.2% | 801 MB | 801 MB | +0.0% | ~ |
| `MeshCSG` | 396.4 s | 398.5 s | +0.5% | 6080 MB | 6080 MB | -0.0% | ~ |
| `MeshReconcile` | 10.1 s | 10.6 s | +5.2% | 511 MB | 512 MB | +0.1% | ~ |
| `PointCloud` | 12.5 s | 12.7 s | +1.6% | 570 MB | 570 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 7.2 s | 7.2 s | -0.2% | 505 MB | 506 MB | +0.2% | ~ |
| `PointCloudHashGridBuild` | 6.5 s | 6.4 s | -1.5% | 505 MB | 506 MB | +0.2% | ~ |
| `Ray` | 189.2 s | 187.5 s | -0.9% | 3703 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 223.8 s | 220.7 s | -1.4% | 4086 MB | 4085 MB | -0.0% | ~ |
| `RayCrossing` | 434.9 s | 436.5 s | +0.4% | 4017 MB | 4017 MB | -0.0% | ~ |
| `RayParticles` | 673.4 s | 678.5 s | +0.7% | 5632 MB | 5632 MB | -0.0% | ~ |
| `RayShell` | 409.9 s | 415.1 s | +1.3% | 5345 MB | 5346 MB | +0.0% | ~ |
| `Regions` | 359.9 s | 358.7 s | -0.3% | 3835 MB | 3835 MB | +0.0% | ~ |
| `Render` | 254.4 s | 254.9 s | +0.2% | 5896 MB | 5896 MB | +0.0% | ~ |
| `Shapes` | 25.9 s | 25.9 s | +0.0% | 591 MB | 591 MB | +0.0% | ~ |
| `SphereCloudBVH` | 8.6 s | 8.3 s | -3.2% | 505 MB | 506 MB | +0.2% | ~ |
| `SphereCloudRays` | 188.7 s | 188.3 s | -0.2% | 4093 MB | 4093 MB | -0.0% | ~ |
| `SphereContacts` | 8.2 s | 8.4 s | +2.5% | 504 MB | 505 MB | +0.2% | ~ |
| `SurfaceParts` | 399.0 s | 395.1 s | -1.0% | 4242 MB | 4242 MB | +0.0% | ~ |
| `Tape` | 60.0 s | 57.4 s | -4.3% | 980 MB | 981 MB | +0.0% | ~ |
| `TapeBVHUnion` | 42.0 s | 40.7 s | -3.1% | 921 MB | 921 MB | +0.0% | ~ |
| **all, in both** | 4754.0 s | 4750.9 s | -0.1% | 6080 MB | 6080 MB | -0.0% | ~ |

Device kernels: 194 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 4 new, 0 gone. Spills are bytes stored/loaded per thread.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (368 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), LaunchTuning (24), Mesh (8), MeshCSG (18), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Render (6), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/DomainObject/main.cpp, Performance/Common/Inputs.hpp

<!-- END receipt cuda -->
