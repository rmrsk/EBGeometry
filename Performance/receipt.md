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
verdict: yellow
base: c1dd04c
base-fingerprint: f9509f114405c142432bb25ea482a05dd1f1efdf
head: 31f7a01
head-fingerprint: eb69a7e3bccddc2ac0d8879ed8618df6c8084558
cases: 18
rounds: 2
reps: 3
date: 2026-11-13
-->

## Host comparison: yellow

**Accepted:** inlining side effects

`31f7a01` against `c1dd04c` (origin/main), 2026-11-13.

**VERDICT: 2 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 2 minor, 0 faster, 0 differ, 0 lost, 18 cases

- base: c1dd04c (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 31f7a01 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.6 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.2 ms | 26.9 ms | -1.0% | [-1.4%, -0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 318 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 238 ms | 239 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.3 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ | tape/fused 3.52x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.6 ms | 100 ms | +0.6% | [+0.2%, +1.1%] | +0.0% | ~ | tape/fused 2.38x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.5 ms | 29.6 ms | +0.0% | [-1.7%, +1.8%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 29.9 ms | 29.9 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 351 ms | 352 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 253 ms | 254 ms | +0.3% | [+0.0%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 96.4 ms | 96.4 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ | tape/fused 3.65x |
| `MeshCSG/smooth-fused/frame/double/host` | 106 ms | 107 ms | +0.4% | [+0.3%, +0.6%] | +0.0% | ~ | tape/fused 2.38x |
| `Render/csg/frame/float/host` | 19.5 ms | 20.8 ms | +6.9% | [+6.2%, +8.1%] | +0.0% | minor |  |
| `Render/csg/frame-orthographic/float/host` | 19.6 ms | 20.9 ms | +6.6% | [+5.5%, +7.2%] | +0.0% | minor |  |
| `Render/mesh/frame/float/host` | 18.3 ms | 18.3 ms | -0.0% | [-0.8%, +0.8%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 21.8 ms | 21.5 ms | -1.7% | [-2.7%, -0.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.7 ms | 21.4 ms | -1.4% | [-2.3%, -0.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.5 ms | 20.6 ms | +0.3% | [+0.1%, +0.5%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 619.9 s → 621.7 s (+0.3%), the largest peak 1406 MB → 1407 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.1 s | +0.2% | 556 MB | 555 MB | -0.0% | ~ |
| `BVHRefit` | 3.9 s | 3.9 s | -0.2% | 482 MB | 483 MB | +0.1% | ~ |
| `BVHUnion` | 18.0 s | 17.9 s | -0.7% | 897 MB | 896 MB | -0.1% | ~ |
| `BVHUnionMixed` | 7.2 s | 7.2 s | +0.0% | 556 MB | 555 MB | -0.1% | ~ |
| `ClosestPrimitive` | 27.9 s | 28.3 s | +1.6% | 1263 MB | 1263 MB | +0.0% | ~ |
| `ClosestSurface` | 27.4 s | 27.4 s | +0.0% | 1221 MB | 1221 MB | -0.0% | ~ |
| `Domain` | 17.6 s | 17.7 s | +0.7% | 836 MB | 838 MB | +0.1% | ~ |
| `Fused` | 23.8 s | 23.7 s | -0.5% | 1126 MB | 1126 MB | +0.0% | ~ |
| `LaunchTuning` | 35.5 s | 35.9 s | +1.2% | 1271 MB | 1272 MB | +0.1% | ~ |
| `Mesh` | 14.6 s | 14.9 s | +1.6% | 784 MB | 784 MB | +0.0% | ~ |
| `MeshCSG` | 37.5 s | 37.9 s | +1.0% | 1282 MB | 1284 MB | +0.1% | ~ |
| `MeshReconcile` | 6.6 s | 6.5 s | -1.4% | 565 MB | 565 MB | +0.0% | ~ |
| `PointCloud` | 7.4 s | 7.2 s | -2.4% | 557 MB | 557 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.7 s | 3.8 s | +2.7% | 468 MB | 468 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | -0.2% | 452 MB | 452 MB | +0.0% | ~ |
| `Ray` | 35.8 s | 35.1 s | -2.1% | 1317 MB | 1318 MB | +0.1% | ~ |
| `RayCSG` | 32.2 s | 31.6 s | -1.6% | 1113 MB | 1112 MB | -0.1% | ~ |
| `RayCrossing` | 44.0 s | 45.2 s | +2.6% | 1342 MB | 1341 MB | -0.0% | ~ |
| `RayParticles` | 47.2 s | 47.3 s | +0.3% | 1316 MB | 1312 MB | -0.3% | ~ |
| `RayShell` | 36.3 s | 36.3 s | -0.1% | 1241 MB | 1274 MB | +2.7% | ~ |
| `Regions` | 28.4 s | 28.8 s | +1.3% | 1066 MB | 1066 MB | +0.0% | ~ |
| `Render` | 35.0 s | 35.3 s | +0.7% | 1264 MB | 1265 MB | +0.1% | ~ |
| `Shapes` | 9.1 s | 9.2 s | +2.1% | 554 MB | 554 MB | -0.1% | ~ |
| `SphereCloudBVH` | 4.9 s | 4.9 s | -0.2% | 508 MB | 508 MB | +0.0% | ~ |
| `SphereCloudRays` | 27.6 s | 27.3 s | -1.1% | 1060 MB | 1061 MB | +0.0% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | -0.7% | 482 MB | 482 MB | -0.1% | ~ |
| `SurfaceParts` | 40.3 s | 40.2 s | -0.3% | 1406 MB | 1407 MB | +0.1% | ~ |
| `Tape` | 16.6 s | 16.6 s | +0.5% | 819 MB | 819 MB | -0.0% | ~ |
| `TapeBVHUnion` | 14.7 s | 14.8 s | +1.1% | 821 MB | 821 MB | +0.0% | ~ |
| **all, in both** | 619.9 s | 621.7 s | +0.3% | 1406 MB | 1407 MB | +0.1% | ~ |


Flagged after 2 rounds, and judged on 10 (2 cases): Render/csg/frame/float/host, Render/csg/frame-orthographic/float/host

These probes compiled to the same code on both sides and were not run (252 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: c1dd04c
base-fingerprint: f9509f114405c142432bb25ea482a05dd1f1efdf
head: 31f7a01
head-fingerprint: eb69a7e3bccddc2ac0d8879ed8618df6c8084558
cases: 24
rounds: 2
reps: 3
date: 2026-11-13
-->

## Device comparison (CUDA): green

**Accepted:** -

`31f7a01` against `c1dd04c` (origin/main), 2026-11-13.

**VERDICT: no regression (0 major, 0 minor across 24 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 24 cases

- base: c1dd04c (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 31f7a01 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.53 ms | 2.53 ms | -0.1% | [-1.0%, +0.7%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.26 ms | 4.30 ms | +0.9% | [+0.8%, +1.0%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.6 ms | 13.7 ms | +0.7% | [+0.1%, +1.2%] | - | ~ | device 4.0x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 62.9 ms | 63.2 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 93.7 ms | 92.6 ms | -1.2% | [-2.1%, -0.3%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 249 ms | 250 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.1 ms | 24.6 ms | -1.8% | [-2.0%, -1.7%] | - | ~ | device 2.0x faster than the host; tape/fused 2.57x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.1 ms | 27.1 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 2.2x faster than the host; tape/fused 3.42x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 70.9 ms | 72.3 ms | +2.0% | [+0.7%, +3.2%] | - | ~ | device 4.0x faster than the host; tape/fused 3.45x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.2 ms | 22.3 ms | +0.7% | [-1.4%, +2.8%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 43.6 ms | 43.8 ms | +0.4% | [-0.5%, +1.2%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 93.3 ms | 96.4 ms | +3.4% | [-1.2%, +8.1%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 534 ms | 524 ms | -2.0% | [-4.9%, +1.1%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 837 ms | 846 ms | +1.1% | [-0.4%, +2.5%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.00 s | 2.05 s | +2.3% | [+1.0%, +3.7%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 212 ms | 189 ms | -10.7% | [-14.4%, -6.8%] | - | faster | device 0.3x faster than the host (was 0.2x); tape/fused 2.76x (was 2.52x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 311 ms | 304 ms | -2.3% | [-7.4%, +3.1%] | - | ~ | device 0.2x faster than the host; tape/fused 2.78x (was 2.69x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 586 ms | 595 ms | +1.5% | [-0.6%, +3.5%] | - | ~ | device 0.7x faster than the host (was 0.6x); tape/fused 3.44x |
| `Render/csg/frame/float/hostdevice` | 3.99 ms | 3.98 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 2.9x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 3.98 ms | 3.98 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.97 ms | 1.96 ms | -0.6% | [-1.4%, +0.3%] | - | ~ | device 8.6x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.66 ms | 7.67 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.7x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.65 ms | 7.65 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 1.7x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.5 ms | 23.6 ms | +0.3% | [-0.5%, +1.2%] | - | ~ | device 0.7x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4705.6 s → 4705.7 s (+0.0%), the largest peak 5632 MB → 5632 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.9 s | 12.9 s | -0.0% | 596 MB | 596 MB | -0.1% | ~ |
| `BVHRefit` | 6.5 s | 6.5 s | -0.1% | 503 MB | 503 MB | +0.1% | ~ |
| `BVHUnion` | 24.2 s | 24.3 s | +0.4% | 1021 MB | 1021 MB | -0.0% | ~ |
| `BVHUnionMixed` | 10.7 s | 10.8 s | +0.5% | 621 MB | 621 MB | -0.0% | ~ |
| `ClosestPrimitive` | 129.2 s | 128.1 s | -0.9% | 1343 MB | 1343 MB | +0.0% | ~ |
| `ClosestSurface` | 192.2 s | 192.0 s | -0.1% | 1844 MB | 1843 MB | -0.0% | ~ |
| `Domain` | 58.6 s | 58.3 s | -0.5% | 990 MB | 990 MB | +0.0% | ~ |
| `Fused` | 52.6 s | 52.8 s | +0.4% | 1154 MB | 1153 MB | -0.0% | ~ |
| `LaunchTuning` | 534.5 s | 535.7 s | +0.2% | 3806 MB | 3806 MB | +0.0% | ~ |
| `Mesh` | 20.7 s | 20.8 s | +0.4% | 797 MB | 798 MB | +0.1% | ~ |
| `MeshCSG` | 370.1 s | 373.8 s | +1.0% | 5160 MB | 5161 MB | +0.0% | ~ |
| `MeshReconcile` | 10.0 s | 10.1 s | +1.4% | 508 MB | 508 MB | +0.0% | ~ |
| `PointCloud` | 12.7 s | 12.9 s | +0.9% | 566 MB | 566 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 7.0 s | 7.1 s | +1.2% | 503 MB | 504 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 6.2 s | 6.3 s | +1.6% | 503 MB | 503 MB | +0.0% | ~ |
| `Ray` | 189.7 s | 190.6 s | +0.5% | 3703 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 219.7 s | 220.9 s | +0.5% | 4086 MB | 4086 MB | +0.0% | ~ |
| `RayCrossing` | 435.6 s | 434.9 s | -0.2% | 4017 MB | 4017 MB | +0.0% | ~ |
| `RayParticles` | 676.9 s | 672.4 s | -0.7% | 5632 MB | 5632 MB | +0.0% | ~ |
| `RayShell` | 411.0 s | 409.9 s | -0.3% | 5346 MB | 5345 MB | -0.0% | ~ |
| `Regions` | 356.8 s | 358.4 s | +0.4% | 3835 MB | 3835 MB | +0.0% | ~ |
| `Render` | 227.8 s | 227.6 s | -0.1% | 5216 MB | 5216 MB | +0.0% | ~ |
| `Shapes` | 25.5 s | 25.5 s | +0.0% | 587 MB | 587 MB | +0.1% | ~ |
| `SphereCloudBVH` | 8.4 s | 8.6 s | +1.8% | 503 MB | 504 MB | +0.1% | ~ |
| `SphereCloudRays` | 186.8 s | 187.1 s | +0.2% | 4093 MB | 4093 MB | -0.0% | ~ |
| `SphereContacts` | 8.2 s | 8.0 s | -2.3% | 503 MB | 503 MB | +0.1% | ~ |
| `SurfaceParts` | 406.8 s | 404.2 s | -0.6% | 4241 MB | 4242 MB | +0.0% | ~ |
| `Tape` | 63.4 s | 64.3 s | +1.4% | 977 MB | 978 MB | +0.1% | ~ |
| `TapeBVHUnion` | 40.9 s | 41.1 s | +0.5% | 918 MB | 918 MB | +0.0% | ~ |
| **all, in both** | 4705.6 s | 4705.7 s | +0.0% | 5632 MB | 5632 MB | +0.0% | ~ |

Device kernels: 194 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.


These probes compiled to the same code on both sides and were not run (344 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), LaunchTuning (24), Mesh (8), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
