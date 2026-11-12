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
base: d0008c1
base-fingerprint: 288e7982d874dba46dbf1425b20f6c00a5c00063
head: 730a063
head-fingerprint: 518d55e430dcc18ec41ddffdd4aefecc191fbff7
cases: 30
rounds: 2
reps: 3
date: 2026-11-12
-->

## Host comparison: green

**Accepted:** -

`730a063` against `d0008c1` (origin/main), 2026-11-12.

**VERDICT: no regression (0 major, 0 minor across 30 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 30 cases

- base: d0008c1 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 730a063 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 26.7 ms | 26.7 ms | +0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.1 ms | 27.3 ms | +0.4% | [-0.5%, +1.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 318 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 239 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 90.3 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ | tape/fused 3.53x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.9 ms | 99.8 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.5 ms | 29.4 ms | -0.3% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 29.8 ms | 30.0 ms | +0.7% | [-0.8%, +2.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 354 ms | 352 ms | -0.3% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 253 ms | 253 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 96.8 ms | 97.0 ms | +0.2% | [+0.2%, +0.2%] | +0.0% | ~ | tape/fused 3.63x |
| `MeshCSG/smooth-fused/frame/double/host` | 107 ms | 106 ms | -0.4% | [-0.8%, -0.0%] | +0.0% | ~ | tape/fused 2.38x |
| `Regions/particles/region/float/host` | 7.34 ms | 7.32 ms | -0.4% | [-0.5%, -0.2%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.6 ms | +0.3% | [-0.0%, +0.5%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 265 ms | 264 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.4 ms | -0.4% | [-0.9%, +0.1%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.2 ms | 30.4 ms | +0.7% | [-1.0%, +2.5%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 22.1 ms | 21.9 ms | -0.8% | [-1.6%, +0.1%] | +0.0% | ~ | regions/hit 1.39x |
| `Regions/particles/region/double/host` | 7.64 ms | 7.65 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.9 ms | 12.9 ms | +0.1% | [+0.0%, +0.2%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 281 ms | 282 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.0 ms | 13.0 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.1 ms | 32.3 ms | +0.7% | [+0.1%, +1.3%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.6 ms | 23.8 ms | +0.9% | [+0.4%, +1.3%] | +0.0% | ~ | regions/hit 1.36x |
| `Render/csg/frame/float/host` | 19.5 ms | 19.7 ms | +1.3% | [-0.0%, +2.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 19.6 ms | 19.8 ms | +0.8% | [+0.1%, +1.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.4 ms | 18.2 ms | -0.7% | [-1.9%, +0.5%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 22.9 ms | 22.0 ms | -4.1% | [-8.6%, +0.8%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.7 ms | 21.6 ms | -0.6% | [-1.5%, +0.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.7 ms | 20.5 ms | -0.7% | [-1.8%, +0.3%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 615.9 s → 617.4 s (+0.3%), the largest peak 1406 MB → 1406 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.1 s | -0.1% | 555 MB | 556 MB | +0.1% | ~ |
| `BVHRefit` | 3.9 s | 4.0 s | +1.3% | 483 MB | 482 MB | -0.0% | ~ |
| `BVHUnion` | 17.7 s | 17.9 s | +0.9% | 896 MB | 896 MB | +0.1% | ~ |
| `BVHUnionMixed` | 7.2 s | 7.2 s | -0.2% | 556 MB | 555 MB | -0.1% | ~ |
| `ClosestPrimitive` | 27.7 s | 27.9 s | +0.9% | 1262 MB | 1262 MB | +0.0% | ~ |
| `ClosestSurface` | 27.4 s | 27.2 s | -0.6% | 1221 MB | 1221 MB | +0.0% | ~ |
| `Domain` | 17.5 s | 17.6 s | +0.5% | 836 MB | 837 MB | +0.0% | ~ |
| `Fused` | 23.4 s | 23.6 s | +0.7% | 1126 MB | 1126 MB | +0.0% | ~ |
| `LaunchTuning` | 35.3 s | 35.9 s | +1.5% | 1270 MB | 1271 MB | +0.0% | ~ |
| `Mesh` | 14.3 s | 14.5 s | +1.2% | 784 MB | 785 MB | +0.2% | ~ |
| `MeshCSG` | 37.3 s | 37.9 s | +1.4% | 1281 MB | 1282 MB | +0.0% | ~ |
| `MeshReconcile` | 6.5 s | 6.5 s | +0.8% | 565 MB | 565 MB | +0.0% | ~ |
| `PointCloud` | 7.2 s | 7.4 s | +3.2% | 557 MB | 557 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.7 s | 3.7 s | -1.0% | 468 MB | 468 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.2 s | -4.3% | 451 MB | 451 MB | +0.1% | ~ |
| `Ray` | 35.7 s | 34.8 s | -2.4% | 1317 MB | 1317 MB | +0.0% | ~ |
| `RayCSG` | 31.3 s | 31.7 s | +1.2% | 1113 MB | 1113 MB | +0.0% | ~ |
| `RayCrossing` | 43.7 s | 43.7 s | -0.1% | 1342 MB | 1341 MB | -0.0% | ~ |
| `RayParticles` | 47.5 s | 46.9 s | -1.1% | 1315 MB | 1316 MB | +0.1% | ~ |
| `RayShell` | 36.4 s | 36.4 s | +0.0% | 1241 MB | 1240 MB | -0.0% | ~ |
| `Regions` | 28.0 s | 28.7 s | +2.5% | 1065 MB | 1066 MB | +0.1% | ~ |
| `Render` | 34.8 s | 34.8 s | +0.2% | 1263 MB | 1264 MB | +0.1% | ~ |
| `Shapes` | 9.1 s | 9.3 s | +2.2% | 554 MB | 554 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.9 s | 4.9 s | -0.2% | 507 MB | 508 MB | +0.1% | ~ |
| `SphereCloudRays` | 27.5 s | 27.3 s | -0.7% | 1060 MB | 1060 MB | -0.0% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | +0.3% | 482 MB | 483 MB | +0.1% | ~ |
| `SurfaceParts` | 40.0 s | 40.1 s | +0.3% | 1406 MB | 1406 MB | +0.0% | ~ |
| `Tape` | 16.8 s | 16.6 s | -0.7% | 819 MB | 819 MB | -0.0% | ~ |
| `TapeBVHUnion` | 14.6 s | 14.5 s | -0.2% | 820 MB | 821 MB | +0.0% | ~ |
| **all, in both** | 615.9 s | 617.4 s | +0.3% | 1406 MB | 1406 MB | +0.0% | ~ |


These probes compiled to the same code on both sides and were not run (240 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: d0008c1
base-fingerprint: 288e7982d874dba46dbf1425b20f6c00a5c00063
head: 730a063
head-fingerprint: 518d55e430dcc18ec41ddffdd4aefecc191fbff7
cases: 36
rounds: 2
reps: 3
date: 2026-11-12
-->

## Device comparison (CUDA): green

**Accepted:** -

`730a063` against `d0008c1` (origin/main), 2026-11-12.

**VERDICT: no regression (0 major, 0 minor across 36 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 36 cases

- base: d0008c1 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 730a063 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.37 ms | 2.35 ms | -0.9% | [-15.3%, +15.7%] | - | ~ | device 4.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.29 ms | 4.28 ms | -0.1% | [-1.2%, +1.0%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.8 ms | 13.7 ms | -0.4% | [-0.6%, -0.3%] | - | ~ | device 3.9x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 59.1 ms | 57.4 ms | -2.9% | [-15.5%, +11.7%] | - | ~ | device 3.5x faster than the host (was 3.4x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 82.9 ms | 82.7 ms | -0.2% | [-2.1%, +1.7%] | - | ~ | device 2.9x faster than the host (was 2.6x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 236 ms | 233 ms | -1.2% | [-2.7%, +0.3%] | - | ~ | device 3.5x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.8 ms | 23.5 ms | -5.0% | [-11.6%, +1.7%] | - | ~ | device 2.1x faster than the host (was 2.0x); tape/fused 2.44x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 22.7 ms | 24.7 ms | +8.8% | [-2.5%, +20.5%] | - | ~ | device 2.4x faster than the host (was 2.6x); tape/fused 3.35x (was 3.65x) |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 61.2 ms | 62.1 ms | +1.5% | [-1.4%, +4.5%] | - | ~ | device 4.8x faster than the host (was 5.4x); tape/fused 3.75x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 18.4 ms | 18.5 ms | +0.6% | [-2.1%, +3.4%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 36.2 ms | 36.1 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 81.8 ms | 76.8 ms | -6.1% | [-7.3%, -4.9%] | - | faster | device 0.7x faster than the host (was 0.7x) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 534 ms | 515 ms | -3.6% | [-7.2%, +0.3%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 838 ms | 848 ms | +1.2% | [-0.1%, +2.5%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.05 s | 2.04 s | -0.6% | [-2.1%, +1.0%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 174 ms | 174 ms | -0.1% | [-4.4%, +4.3%] | - | ~ | device 0.3x faster than the host; tape/fused 2.96x (was 3.06x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 293 ms | 295 ms | +0.5% | [-1.4%, +2.5%] | - | ~ | device 0.2x faster than the host; tape/fused 2.88x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 595 ms | 594 ms | -0.3% | [-3.7%, +3.4%] | - | ~ | device 0.6x faster than the host (was 0.5x); tape/fused 3.44x |
| `Regions/particles/region/float/hostdevice` | 2.33 ms | 2.31 ms | -0.8% | [-2.5%, +0.9%] | - | ~ | device 3.3x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.14 ms | 3.15 ms | +0.4% | [+0.1%, +0.6%] | - | ~ | device 5.1x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 100 ms | 100 ms | +0.1% | [-1.3%, +1.6%] | - | ~ | device 3.0x faster than the host; each/one-pass 0.03x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.12 ms | 3.13 ms | +0.3% | [-0.6%, +1.3%] | - | ~ | device 4.4x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.19 ms | 6.34 ms | +2.4% | [+0.4%, +4.4%] | - | ~ | device 4.0x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.47 ms | 5.49 ms | +0.3% | [+0.1%, +0.5%] | - | ~ | device 3.2x faster than the host; regions/hit 1.16x |
| `Regions/particles/region/double/hostdevice` | 3.43 ms | 3.38 ms | -1.4% | [-3.9%, +1.1%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.86 ms | 5.88 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 132 ms | 132 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 2.3x faster than the host (was 2.5x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.19 ms | 5.15 ms | -0.7% | [-2.3%, +1.0%] | - | ~ | device 2.7x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 14.8 ms | 14.3 ms | -2.7% | [-12.1%, +8.1%] | - | ~ | device 1.8x faster than the host (was 1.7x) |
| `Regions/assembly/firstHit/double/hostdevice` | 11.9 ms | 11.6 ms | -2.4% | [-8.3%, +3.9%] | - | ~ | device 1.5x faster than the host (was 1.5x); regions/hit 1.24x |
| `Render/csg/frame/float/hostdevice` | 3.93 ms | 3.92 ms | -0.2% | [-2.5%, +2.1%] | - | ~ | device 2.9x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 3.94 ms | 3.99 ms | +1.3% | [-0.5%, +3.2%] | - | ~ | device 2.8x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.66 ms | 1.66 ms | -0.2% | [-0.7%, +0.2%] | - | ~ | device 10.1x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.18 ms | 7.19 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 1.8x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.21 ms | 7.34 ms | +1.8% | [-0.2%, +3.7%] | - | ~ | device 1.7x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 19.3 ms | 19.3 ms | -0.3% | [-1.8%, +1.3%] | - | ~ | device 0.9x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4724.2 s → 4706.5 s (-0.4%), the largest peak 5632 MB → 5632 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.0 s | 12.9 s | -0.6% | 596 MB | 596 MB | +0.0% | ~ |
| `BVHRefit` | 6.5 s | 6.5 s | +0.2% | 503 MB | 503 MB | +0.0% | ~ |
| `BVHUnion` | 24.4 s | 24.5 s | +0.5% | 1021 MB | 1021 MB | -0.0% | ~ |
| `BVHUnionMixed` | 10.7 s | 10.7 s | +0.3% | 621 MB | 621 MB | +0.0% | ~ |
| `ClosestPrimitive` | 128.1 s | 128.2 s | +0.1% | 1343 MB | 1343 MB | +0.0% | ~ |
| `ClosestSurface` | 192.6 s | 192.1 s | -0.2% | 1844 MB | 1844 MB | +0.0% | ~ |
| `Domain` | 58.1 s | 58.7 s | +1.0% | 990 MB | 990 MB | +0.0% | ~ |
| `Fused` | 52.6 s | 52.7 s | +0.2% | 1153 MB | 1154 MB | +0.0% | ~ |
| `LaunchTuning` | 536.2 s | 536.2 s | -0.0% | 3806 MB | 3806 MB | -0.0% | ~ |
| `Mesh` | 20.6 s | 20.6 s | +0.2% | 798 MB | 798 MB | +0.0% | ~ |
| `MeshCSG` | 372.5 s | 373.1 s | +0.2% | 5160 MB | 5162 MB | +0.0% | ~ |
| `MeshReconcile` | 10.1 s | 10.2 s | +0.9% | 508 MB | 508 MB | -0.1% | ~ |
| `PointCloud` | 12.7 s | 12.5 s | -1.8% | 566 MB | 566 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 7.3 s | 7.2 s | -1.1% | 503 MB | 503 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 6.5 s | 6.3 s | -3.2% | 503 MB | 503 MB | +0.1% | ~ |
| `Ray` | 191.0 s | 188.8 s | -1.1% | 3703 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 221.6 s | 220.6 s | -0.5% | 4086 MB | 4086 MB | +0.0% | ~ |
| `RayCrossing` | 436.8 s | 434.2 s | -0.6% | 4017 MB | 4017 MB | -0.0% | ~ |
| `RayParticles` | 674.9 s | 675.8 s | +0.1% | 5632 MB | 5632 MB | +0.0% | ~ |
| `RayShell` | 412.1 s | 410.5 s | -0.4% | 5345 MB | 5345 MB | -0.0% | ~ |
| `Regions` | 362.6 s | 356.0 s | -1.8% | 3878 MB | 3835 MB | -1.1% | ~ |
| `Render` | 228.1 s | 226.2 s | -0.9% | 5216 MB | 5216 MB | +0.0% | ~ |
| `Shapes` | 26.0 s | 25.5 s | -1.9% | 587 MB | 587 MB | +0.1% | ~ |
| `SphereCloudBVH` | 8.6 s | 8.6 s | +0.5% | 503 MB | 504 MB | +0.0% | ~ |
| `SphereCloudRays` | 189.6 s | 187.7 s | -1.0% | 4093 MB | 4093 MB | -0.0% | ~ |
| `SphereContacts` | 8.3 s | 8.3 s | +0.5% | 503 MB | 503 MB | +0.0% | ~ |
| `SurfaceParts` | 407.5 s | 405.9 s | -0.4% | 4242 MB | 4242 MB | +0.0% | ~ |
| `Tape` | 64.0 s | 64.4 s | +0.6% | 977 MB | 977 MB | +0.0% | ~ |
| `TapeBVHUnion` | 41.2 s | 41.4 s | +0.6% | 918 MB | 918 MB | +0.0% | ~ |
| **all, in both** | 4724.2 s | 4706.5 s | -0.4% | 5632 MB | 5632 MB | +0.0% | ~ |

Device kernels: 194 in both, 2 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, true>, Vec3T<double> const, Regi…` | 168 → 168 | 16992 → 17072 | 8/8 → 8/8 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, true>, Vec3T<float> const, Region…` | 168 → 168 | 12208 → 12240 | 8/8 → 8/8 | ~ |


These probes compiled to the same code on both sides and were not run (332 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), LaunchTuning (24), Mesh (8), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
