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
head: 0321f6f
head-fingerprint: f9509f114405c142432bb25ea482a05dd1f1efdf
cases: 30
rounds: 2
reps: 3
date: 2026-11-12
-->

## Host comparison: green

**Accepted:** -

`0321f6f` against `d0008c1` (origin/main), 2026-11-12.

**VERDICT: no regression (0 major, 0 minor across 30 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 30 cases

- base: d0008c1 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 0321f6f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.7 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.1 ms | 27.2 ms | +0.5% | [+0.2%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 318 ms | -0.1% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 238 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.6 ms | 90.7 ms | +0.1% | [-0.8%, +1.1%] | +0.0% | ~ | tape/fused 3.50x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.5 ms | 101 ms | +1.3% | [+0.8%, +1.8%] | +0.0% | ~ | tape/fused 2.36x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.3 ms | 29.2 ms | -0.1% | [-1.2%, +1.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.1 ms | 30.2 ms | +0.3% | [-2.1%, +2.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 355 ms | 353 ms | -0.7% | [-1.8%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 253 ms | 253 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.1 ms | 96.7 ms | -0.4% | [-0.8%, -0.0%] | +0.0% | ~ | tape/fused 3.65x |
| `MeshCSG/smooth-fused/frame/double/host` | 106 ms | 107 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ | tape/fused 2.38x |
| `Regions/particles/region/float/host` | 7.33 ms | 7.32 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.6 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 265 ms | 264 ms | -0.5% | [-0.6%, -0.4%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.4 ms | -0.8% | [-1.4%, -0.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 29.9 ms | 30.2 ms | +0.8% | [+0.7%, +0.9%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 21.9 ms | 22.0 ms | +0.5% | [+0.2%, +0.9%] | +0.0% | ~ | regions/hit 1.37x |
| `Regions/particles/region/double/host` | 7.64 ms | 7.63 ms | -0.1% | [-0.5%, +0.3%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.9 ms | 12.9 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 281 ms | 282 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | 13.0 ms | 13.0 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.2 ms | 32.5 ms | +0.9% | [+0.1%, +1.7%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.6 ms | 23.7 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ | regions/hit 1.37x |
| `Render/csg/frame/float/host` | 19.5 ms | 19.6 ms | +0.5% | [-1.3%, +2.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 19.3 ms | 19.5 ms | +1.0% | [+0.5%, +1.6%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.2 ms | 18.7 ms | +2.5% | [+0.0%, +5.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 25.7 ms | 21.8 ms | -15.3% | [-26.9%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.7 ms | 21.9 ms | +1.0% | [-0.9%, +2.9%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.5 ms | 20.7 ms | +0.9% | [+0.1%, +1.8%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 615.9 s → 623.7 s (+1.3%), the largest peak 1406 MB → 1406 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.4 s | +3.3% | 555 MB | 556 MB | +0.1% | ~ |
| `BVHRefit` | 3.9 s | 4.0 s | +1.9% | 483 MB | 483 MB | +0.0% | ~ |
| `BVHUnion` | 17.7 s | 18.0 s | +1.9% | 896 MB | 896 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.2 s | 7.3 s | +1.1% | 556 MB | 555 MB | -0.1% | ~ |
| `ClosestPrimitive` | 27.7 s | 28.1 s | +1.6% | 1262 MB | 1263 MB | +0.1% | ~ |
| `ClosestSurface` | 27.4 s | 27.5 s | +0.6% | 1221 MB | 1221 MB | +0.0% | ~ |
| `Domain` | 17.5 s | 17.9 s | +2.3% | 836 MB | 837 MB | +0.1% | ~ |
| `Fused` | 23.4 s | 24.6 s | +5.1% | 1126 MB | 1126 MB | +0.1% | ~ |
| `LaunchTuning` | 35.3 s | 36.4 s | +3.1% | 1270 MB | 1271 MB | +0.1% | ~ |
| `Mesh` | 14.3 s | 14.8 s | +3.1% | 784 MB | 784 MB | +0.1% | ~ |
| `MeshCSG` | 37.3 s | 37.8 s | +1.2% | 1281 MB | 1284 MB | +0.2% | ~ |
| `MeshReconcile` | 6.5 s | 6.7 s | +3.1% | 565 MB | 565 MB | +0.1% | ~ |
| `PointCloud` | 7.2 s | 7.4 s | +3.1% | 557 MB | 557 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 3.7 s | 3.8 s | +1.6% | 468 MB | 468 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.3 s | -2.0% | 451 MB | 451 MB | +0.1% | ~ |
| `Ray` | 35.7 s | 35.6 s | -0.1% | 1317 MB | 1318 MB | +0.1% | ~ |
| `RayCSG` | 31.3 s | 31.6 s | +1.1% | 1113 MB | 1113 MB | +0.1% | ~ |
| `RayCrossing` | 43.7 s | 44.0 s | +0.7% | 1342 MB | 1342 MB | -0.0% | ~ |
| `RayParticles` | 47.5 s | 47.7 s | +0.5% | 1315 MB | 1316 MB | +0.0% | ~ |
| `RayShell` | 36.4 s | 36.3 s | -0.3% | 1241 MB | 1241 MB | +0.0% | ~ |
| `Regions` | 28.0 s | 28.3 s | +1.3% | 1065 MB | 1066 MB | +0.1% | ~ |
| `Render` | 34.8 s | 34.9 s | +0.5% | 1263 MB | 1264 MB | +0.1% | ~ |
| `Shapes` | 9.1 s | 9.2 s | +1.2% | 554 MB | 554 MB | +0.0% | ~ |
| `SphereCloudBVH` | 4.9 s | 4.8 s | -1.9% | 507 MB | 508 MB | +0.0% | ~ |
| `SphereCloudRays` | 27.5 s | 27.8 s | +1.3% | 1060 MB | 1060 MB | +0.0% | ~ |
| `SphereContacts` | 4.3 s | 4.4 s | +2.3% | 482 MB | 482 MB | +0.0% | ~ |
| `SurfaceParts` | 40.0 s | 40.6 s | +1.5% | 1406 MB | 1406 MB | -0.0% | ~ |
| `Tape` | 16.8 s | 16.7 s | -0.4% | 819 MB | 819 MB | -0.0% | ~ |
| `TapeBVHUnion` | 14.6 s | 14.8 s | +1.3% | 820 MB | 821 MB | +0.0% | ~ |
| **all, in both** | 615.9 s | 623.7 s | +1.3% | 1406 MB | 1406 MB | -0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (240 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: d0008c1
base-fingerprint: 288e7982d874dba46dbf1425b20f6c00a5c00063
head: 0321f6f
head-fingerprint: f9509f114405c142432bb25ea482a05dd1f1efdf
cases: 36
rounds: 2
reps: 3
date: 2026-11-12
-->

## Device comparison (CUDA): green

**Accepted:** -

`0321f6f` against `d0008c1` (origin/main), 2026-11-12.

**VERDICT: no regression (0 major, 0 minor across 36 cases)**

Performance comparison: 0 major, 0 minor, 3 faster, 0 differ, 0 lost, 36 cases

- base: d0008c1 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 0321f6f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.54 ms | 2.36 ms | -7.1% | [-14.9%, +0.7%] | - | ~ | device 4.4x faster than the host (was 4.1x) |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.32 ms | 4.23 ms | -1.9% | [-2.3%, -1.6%] | - | ~ | device 3.3x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.8 ms | 13.7 ms | -0.4% | [-1.6%, +0.7%] | - | ~ | device 4.0x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 58.0 ms | 55.3 ms | -4.7% | [-11.6%, +2.7%] | - | ~ | device 3.6x faster than the host (was 3.8x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 82.5 ms | 81.9 ms | -0.7% | [-4.0%, +2.7%] | - | ~ | device 2.6x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 233 ms | 237 ms | +1.3% | [-1.7%, +4.5%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 22.7 ms | 22.7 ms | -0.0% | [-17.2%, +20.3%] | - | ~ | device 2.2x faster than the host; tape/fused 2.44x (was 2.56x) |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 22.9 ms | 22.6 ms | -1.6% | [-6.7%, +3.7%] | - | ~ | device 2.6x faster than the host; tape/fused 3.63x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 61.2 ms | 62.3 ms | +1.9% | [-1.1%, +5.0%] | - | ~ | device 5.2x faster than the host (was 4.8x); tape/fused 3.80x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.0 ms | 17.5 ms | -16.5% | [-26.3%, -3.9%] | - | faster | device 0.6x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 36.4 ms | 37.0 ms | +1.6% | [-1.1%, +4.2%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 77.5 ms | 77.4 ms | -0.1% | [-2.3%, +2.1%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 527 ms | 529 ms | +0.3% | [-4.5%, +5.2%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 861 ms | 844 ms | -2.0% | [-2.5%, -1.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.06 s | 2.06 s | +0.0% | [-3.0%, +3.1%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 181 ms | 171 ms | -5.5% | [-6.9%, -4.1%] | - | faster | device 0.3x faster than the host (was 0.3x); tape/fused 3.10x (was 2.92x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 294 ms | 291 ms | -0.8% | [-2.0%, +0.5%] | - | ~ | device 0.2x faster than the host; tape/fused 2.90x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 595 ms | 595 ms | -0.0% | [-2.2%, +2.2%] | - | ~ | device 0.5x faster than the host (was 0.6x); tape/fused 3.46x |
| `Regions/particles/region/float/hostdevice` | 2.32 ms | 2.33 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 3.2x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.16 ms | 3.15 ms | -0.3% | [-0.9%, +0.4%] | - | ~ | device 4.8x faster than the host (was 4.5x) |
| `Regions/particles/regionByRegion/float/hostdevice` | 101 ms | 100 ms | -0.4% | [-0.8%, +0.1%] | - | ~ | device 3.3x faster than the host (was 3.1x); each/one-pass 0.03x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.14 ms | 3.13 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 4.3x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.38 ms | 6.21 ms | -2.8% | [-5.5%, +0.2%] | - | ~ | device 4.0x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.47 ms | 5.48 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 3.2x faster than the host; regions/hit 1.13x |
| `Regions/particles/region/double/hostdevice` | 3.46 ms | 3.39 ms | -1.9% | [-3.1%, -0.7%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.86 ms | 5.87 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 132 ms | 132 ms | -0.4% | [-1.0%, +0.3%] | - | ~ | device 2.3x faster than the host (was 2.4x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.18 ms | 5.11 ms | -1.4% | [-3.4%, +0.6%] | - | ~ | device 2.7x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 13.8 ms | 13.8 ms | +0.4% | [-1.0%, +1.0%] | - | ~ | device 1.8x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 12.1 ms | 11.4 ms | -6.2% | [-13.6%, +2.4%] | - | ~ | device 1.5x faster than the host (was 1.4x); regions/hit 1.21x (was 1.13x) |
| `Render/csg/frame/float/hostdevice` | 4.05 ms | 3.92 ms | -3.2% | [-5.6%, -0.7%] | - | faster | device 2.9x faster than the host (was 2.8x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.08 ms | 4.02 ms | -1.6% | [-3.7%, +0.4%] | - | ~ | device 2.8x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.65 ms | 1.66 ms | +0.1% | [-1.1%, +1.2%] | - | ~ | device 10.1x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.16 ms | 7.17 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 1.8x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.19 ms | 7.19 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 19.4 ms | 19.3 ms | -0.6% | [-2.0%, +0.9%] | - | ~ | device 0.9x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4724.2 s → 4728.9 s (+0.1%), the largest peak 5632 MB → 5632 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.0 s | 12.9 s | -0.9% | 596 MB | 596 MB | +0.0% | ~ |
| `BVHRefit` | 6.5 s | 6.6 s | +0.5% | 503 MB | 503 MB | +0.0% | ~ |
| `BVHUnion` | 24.4 s | 24.7 s | +0.9% | 1021 MB | 1021 MB | +0.0% | ~ |
| `BVHUnionMixed` | 10.7 s | 10.8 s | +0.4% | 621 MB | 621 MB | +0.0% | ~ |
| `ClosestPrimitive` | 128.1 s | 129.0 s | +0.8% | 1343 MB | 1343 MB | +0.0% | ~ |
| `ClosestSurface` | 192.6 s | 193.0 s | +0.2% | 1844 MB | 1844 MB | -0.0% | ~ |
| `Domain` | 58.1 s | 58.3 s | +0.3% | 990 MB | 990 MB | +0.0% | ~ |
| `Fused` | 52.6 s | 52.7 s | +0.3% | 1153 MB | 1154 MB | +0.0% | ~ |
| `LaunchTuning` | 536.2 s | 536.7 s | +0.1% | 3806 MB | 3806 MB | +0.0% | ~ |
| `Mesh` | 20.6 s | 20.7 s | +0.9% | 798 MB | 797 MB | -0.0% | ~ |
| `MeshCSG` | 372.5 s | 370.5 s | -0.5% | 5160 MB | 5161 MB | +0.0% | ~ |
| `MeshReconcile` | 10.1 s | 10.0 s | -1.3% | 508 MB | 508 MB | +0.0% | ~ |
| `PointCloud` | 12.7 s | 12.7 s | +0.4% | 566 MB | 566 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 7.3 s | 7.3 s | -0.6% | 503 MB | 503 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 6.5 s | 6.6 s | +1.5% | 503 MB | 503 MB | +0.1% | ~ |
| `Ray` | 191.0 s | 191.7 s | +0.4% | 3703 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 221.6 s | 222.2 s | +0.2% | 4086 MB | 4085 MB | -0.0% | ~ |
| `RayCrossing` | 436.8 s | 437.3 s | +0.1% | 4017 MB | 4017 MB | -0.0% | ~ |
| `RayParticles` | 674.9 s | 674.3 s | -0.1% | 5632 MB | 5632 MB | +0.0% | ~ |
| `RayShell` | 412.1 s | 413.4 s | +0.3% | 5345 MB | 5346 MB | +0.0% | ~ |
| `Regions` | 362.6 s | 360.7 s | -0.5% | 3878 MB | 3835 MB | -1.1% | ~ |
| `Render` | 228.1 s | 230.1 s | +0.9% | 5216 MB | 5216 MB | +0.0% | ~ |
| `Shapes` | 26.0 s | 25.6 s | -1.5% | 587 MB | 587 MB | +0.1% | ~ |
| `SphereCloudBVH` | 8.6 s | 8.7 s | +1.1% | 503 MB | 504 MB | +0.0% | ~ |
| `SphereCloudRays` | 189.6 s | 190.2 s | +0.3% | 4093 MB | 4093 MB | +0.0% | ~ |
| `SphereContacts` | 8.3 s | 8.4 s | +1.0% | 503 MB | 503 MB | +0.0% | ~ |
| `SurfaceParts` | 407.5 s | 407.6 s | +0.0% | 4242 MB | 4242 MB | -0.0% | ~ |
| `Tape` | 64.0 s | 64.4 s | +0.6% | 977 MB | 977 MB | +0.0% | ~ |
| `TapeBVHUnion` | 41.2 s | 41.8 s | +1.4% | 918 MB | 918 MB | +0.0% | ~ |
| **all, in both** | 4724.2 s | 4728.9 s | +0.1% | 5632 MB | 5632 MB | +0.0% | ~ |

Device kernels: 194 in both, 2 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, true>, Vec3T<double> const, Regi…` | 168 → 168 | 16992 → 17072 | 8/8 → 8/8 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, true>, Vec3T<float> const, Region…` | 168 → 168 | 12208 → 12240 | 8/8 → 8/8 | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (1 cases): Regions/assembly/firstRegionChange/double/hostdevice

These probes compiled to the same code on both sides and were not run (332 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), LaunchTuning (24), Mesh (8), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
