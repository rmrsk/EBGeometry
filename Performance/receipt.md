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
head: 1323a18
head-fingerprint: 6e34f8b2f6c9a9ef06498a8f9e315155656f4a20
cases: 30
rounds: 2
reps: 3
date: 2026-11-12
-->

## Host comparison: green

**Accepted:** -

`1323a18` against `d0008c1` (origin/main), 2026-11-12.

**VERDICT: no regression (0 major, 0 minor across 30 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 30 cases

- base: d0008c1 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 1323a18 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 26.6 ms | 26.7 ms | +0.2% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.2 ms | 27.1 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 324 ms | +1.3% | [+0.9%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 239 ms | 241 ms | +0.6% | [-0.4%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 91.2 ms | +0.9% | [-0.0%, +1.9%] | +0.0% | ~ | tape/fused 3.55x |
| `MeshCSG/smooth-fused/frame/float/host` | 102 ms | 100 ms | -1.4% | [-4.0%, +1.2%] | +0.0% | ~ | tape/fused 2.40x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.2 ms | 29.2 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 29.9 ms | 30.0 ms | +0.3% | [-0.8%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 357 ms | +0.2% | [-2.8%, +3.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 259 ms | 255 ms | -1.5% | [-4.1%, +1.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.3 ms | 98.3 ms | +1.1% | [-0.4%, +2.5%] | +0.0% | ~ | tape/fused 3.63x |
| `MeshCSG/smooth-fused/frame/double/host` | 106 ms | 107 ms | +1.0% | [+0.6%, +1.5%] | +0.0% | ~ | tape/fused 2.37x |
| `Regions/particles/region/float/host` | 7.35 ms | 7.31 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/float/host` | 12.6 ms | 12.6 ms | +0.1% | [-0.4%, +0.6%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/float/host` | 265 ms | 264 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/float/host` | 12.5 ms | 12.5 ms | -0.3% | [-0.4%, -0.2%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/float/host` | 30.0 ms | 30.4 ms | +1.1% | [+0.1%, +2.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/float/host` | 21.9 ms | 21.9 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ | regions/hit 1.39x |
| `Regions/particles/region/double/host` | 7.64 ms | 7.57 ms | -0.9% | [-1.9%, +0.1%] | +0.0% | ~ |  |
| `Regions/particles/regionDistances/double/host` | 12.9 ms | 13.0 ms | +1.1% | [-0.1%, +2.3%] | +0.0% | ~ |  |
| `Regions/particles/regionByRegion/double/host` | 282 ms | 282 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | each/one-pass 0.05x |
| `Regions/particles/domainLabels/double/host` | 12.9 ms | 12.9 ms | +0.2% | [-1.2%, +1.7%] | +0.0% | ~ |  |
| `Regions/assembly/firstRegionChange/double/host` | 32.1 ms | 32.5 ms | +1.4% | [+0.7%, +2.0%] | +0.0% | ~ |  |
| `Regions/assembly/firstHit/double/host` | 23.6 ms | 23.7 ms | +0.5% | [-0.4%, +1.4%] | +0.0% | ~ | regions/hit 1.37x |
| `Render/csg/frame/float/host` | 19.7 ms | 19.6 ms | -0.8% | [-1.8%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 19.5 ms | 19.7 ms | +1.0% | [-0.2%, +2.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.4 ms | 18.4 ms | -0.3% | [-1.7%, +1.2%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 21.8 ms | 21.8 ms | -0.1% | [-1.8%, +1.6%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.9 ms | 22.1 ms | +0.7% | [-2.7%, +4.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.7 ms | 20.7 ms | +0.0% | [-0.9%, +0.9%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 615.9 s → 621.8 s (+1.0%), the largest peak 1406 MB → 1406 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.1 s | +0.7% | 555 MB | 555 MB | +0.1% | ~ |
| `BVHRefit` | 3.9 s | 3.9 s | +0.8% | 483 MB | 483 MB | +0.0% | ~ |
| `BVHUnion` | 17.7 s | 18.1 s | +2.3% | 896 MB | 897 MB | +0.1% | ~ |
| `BVHUnionMixed` | 7.2 s | 7.3 s | +1.0% | 556 MB | 555 MB | -0.1% | ~ |
| `ClosestPrimitive` | 27.7 s | 28.4 s | +2.4% | 1262 MB | 1262 MB | +0.0% | ~ |
| `ClosestSurface` | 27.4 s | 27.5 s | +0.5% | 1221 MB | 1221 MB | +0.1% | ~ |
| `Domain` | 17.5 s | 17.7 s | +1.2% | 836 MB | 837 MB | +0.0% | ~ |
| `Fused` | 23.4 s | 24.0 s | +2.4% | 1126 MB | 1125 MB | -0.1% | ~ |
| `LaunchTuning` | 35.3 s | 36.1 s | +2.1% | 1270 MB | 1271 MB | +0.0% | ~ |
| `Mesh` | 14.3 s | 14.6 s | +2.2% | 784 MB | 785 MB | +0.1% | ~ |
| `MeshCSG` | 37.3 s | 37.9 s | +1.6% | 1281 MB | 1284 MB | +0.2% | ~ |
| `MeshReconcile` | 6.5 s | 6.6 s | +1.5% | 565 MB | 565 MB | +0.0% | ~ |
| `PointCloud` | 7.2 s | 7.5 s | +4.6% | 557 MB | 557 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 3.7 s | 3.8 s | +1.1% | 468 MB | 468 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.2 s | -2.9% | 451 MB | 451 MB | +0.0% | ~ |
| `Ray` | 35.7 s | 35.0 s | -2.0% | 1317 MB | 1318 MB | +0.1% | ~ |
| `RayCSG` | 31.3 s | 32.1 s | +2.7% | 1113 MB | 1112 MB | -0.0% | ~ |
| `RayCrossing` | 43.7 s | 44.7 s | +2.3% | 1342 MB | 1341 MB | -0.0% | ~ |
| `RayParticles` | 47.5 s | 47.1 s | -0.7% | 1315 MB | 1316 MB | +0.1% | ~ |
| `RayShell` | 36.4 s | 36.3 s | -0.3% | 1241 MB | 1241 MB | +0.0% | ~ |
| `Regions` | 28.0 s | 28.6 s | +2.4% | 1065 MB | 1066 MB | +0.1% | ~ |
| `Render` | 34.8 s | 34.7 s | -0.2% | 1263 MB | 1264 MB | +0.1% | ~ |
| `Shapes` | 9.1 s | 9.2 s | +0.9% | 554 MB | 554 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.9 s | 4.8 s | -1.4% | 507 MB | 508 MB | +0.1% | ~ |
| `SphereCloudRays` | 27.5 s | 27.7 s | +0.6% | 1060 MB | 1060 MB | +0.0% | ~ |
| `SphereContacts` | 4.3 s | 4.4 s | +2.2% | 482 MB | 482 MB | +0.1% | ~ |
| `SurfaceParts` | 40.0 s | 40.3 s | +0.7% | 1406 MB | 1406 MB | -0.0% | ~ |
| `Tape` | 16.8 s | 16.6 s | -0.6% | 819 MB | 819 MB | +0.0% | ~ |
| `TapeBVHUnion` | 14.6 s | 14.6 s | +0.1% | 820 MB | 820 MB | +0.0% | ~ |
| **all, in both** | 615.9 s | 621.8 s | +1.0% | 1406 MB | 1406 MB | -0.0% | ~ |


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
head: 1323a18
head-fingerprint: 6e34f8b2f6c9a9ef06498a8f9e315155656f4a20
cases: 36
rounds: 2
reps: 3
date: 2026-11-12
-->

## Device comparison (CUDA): green

**Accepted:** -

`1323a18` against `d0008c1` (origin/main), 2026-11-12.

**VERDICT: no regression (0 major, 0 minor across 36 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 36 cases

- base: d0008c1 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 1323a18 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.34 ms | 2.35 ms | +0.2% | [-14.1%, +17.0%] | - | ~ | device 4.4x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.29 ms | 3.97 ms | -7.5% | [-14.3%, -0.6%] | - | faster | device 3.5x faster than the host (was 3.2x) |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.8 ms | 13.8 ms | +0.2% | [-0.5%, +0.9%] | - | ~ | device 3.9x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 54.2 ms | 58.6 ms | +8.2% | [-2.6%, +19.1%] | - | ~ | device 3.5x faster than the host (was 3.7x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 82.8 ms | 82.9 ms | +0.1% | [-4.0%, +4.3%] | - | ~ | device 2.6x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 237 ms | 233 ms | -1.7% | [-4.1%, +0.8%] | - | ~ | device 3.5x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 22.5 ms | 22.9 ms | +1.7% | [-16.2%, +23.6%] | - | ~ | device 2.2x faster than the host; tape/fused 2.56x (was 2.41x) |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 22.6 ms | 22.5 ms | -0.3% | [-4.5%, +4.2%] | - | ~ | device 2.6x faster than the host; tape/fused 3.68x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 60.8 ms | 61.5 ms | +1.1% | [-0.9%, +3.1%] | - | ~ | device 5.6x faster than the host (was 5.3x); tape/fused 3.80x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 20.1 ms | 19.6 ms | -2.7% | [-19.9%, +17.5%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 35.8 ms | 35.9 ms | +0.4% | [-1.5%, +2.4%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 79.0 ms | 79.6 ms | +0.8% | [-1.5%, +3.0%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 538 ms | 528 ms | -1.7% | [-6.0%, +2.6%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 840 ms | 849 ms | +1.1% | [-0.4%, +2.5%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.00 s | 1.99 s | -0.7% | [-1.9%, +0.4%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 173 ms | 176 ms | +1.4% | [-2.7%, +4.5%] | - | ~ | device 0.3x faster than the host; tape/fused 3.00x (was 3.10x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 296 ms | 297 ms | +0.6% | [-1.6%, +2.8%] | - | ~ | device 0.2x faster than the host; tape/fused 2.85x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 584 ms | 589 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 0.5x faster than the host (was 0.6x); tape/fused 3.38x |
| `Regions/particles/region/float/hostdevice` | 2.34 ms | 2.32 ms | -0.8% | [-2.0%, +0.4%] | - | ~ | device 3.2x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.13 ms | 3.13 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 4.1x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 100 ms | 100 ms | +0.1% | [-0.7%, +0.8%] | - | ~ | device 2.9x faster than the host; each/one-pass 0.03x |
| `Regions/particles/domainLabels/float/hostdevice` | 3.14 ms | 3.11 ms | -0.8% | [-1.9%, +0.3%] | - | ~ | device 4.4x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.35 ms | 6.38 ms | +0.4% | [-5.0%, +6.2%] | - | ~ | device 3.9x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.46 ms | 5.47 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 3.2x faster than the host; regions/hit 1.16x |
| `Regions/particles/region/double/hostdevice` | 3.43 ms | 3.44 ms | +0.3% | [-1.7%, +2.4%] | - | ~ | device 2.2x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 5.86 ms | 5.84 ms | -0.4% | [-0.7%, -0.0%] | - | ~ | device 2.3x faster than the host |
| `Regions/particles/regionByRegion/double/hostdevice` | 132 ms | 132 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 2.6x faster than the host (was 2.3x); each/one-pass 0.04x |
| `Regions/particles/domainLabels/double/hostdevice` | 5.11 ms | 5.06 ms | -1.1% | [-2.0%, -0.1%] | - | ~ | device 2.7x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 14.4 ms | 14.1 ms | -2.2% | [-8.4%, +4.7%] | - | ~ | device 1.8x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 11.6 ms | 12.0 ms | +2.9% | [-4.9%, +11.3%] | - | ~ | device 1.5x faster than the host; regions/hit 1.17x (was 1.24x) |
| `Render/csg/frame/float/hostdevice` | 4.08 ms | 4.11 ms | +0.9% | [-0.1%, +1.9%] | - | ~ | device 2.8x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 3.98 ms | 4.02 ms | +1.1% | [-3.4%, +5.9%] | - | ~ | device 2.8x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.65 ms | 1.67 ms | +1.0% | [+0.2%, +1.7%] | - | ~ | device 10.1x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.47 ms | 7.20 ms | -3.7% | [-7.6%, +0.4%] | - | ~ | device 1.8x faster than the host (was 1.7x) |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.19 ms | 7.18 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 1.8x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 19.3 ms | 19.4 ms | +0.4% | [-0.5%, +1.3%] | - | ~ | device 0.9x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4724.2 s → 4717.4 s (-0.1%), the largest peak 5632 MB → 5632 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.0 s | 13.0 s | -0.2% | 596 MB | 596 MB | -0.0% | ~ |
| `BVHRefit` | 6.5 s | 6.7 s | +2.3% | 503 MB | 503 MB | +0.0% | ~ |
| `BVHUnion` | 24.4 s | 24.4 s | -0.4% | 1021 MB | 1021 MB | +0.0% | ~ |
| `BVHUnionMixed` | 10.7 s | 10.9 s | +1.5% | 621 MB | 621 MB | +0.0% | ~ |
| `ClosestPrimitive` | 128.1 s | 128.9 s | +0.7% | 1343 MB | 1343 MB | +0.0% | ~ |
| `ClosestSurface` | 192.6 s | 192.7 s | +0.1% | 1844 MB | 1844 MB | -0.0% | ~ |
| `Domain` | 58.1 s | 59.2 s | +1.9% | 990 MB | 991 MB | +0.0% | ~ |
| `Fused` | 52.6 s | 52.6 s | +0.1% | 1153 MB | 1154 MB | +0.0% | ~ |
| `LaunchTuning` | 536.2 s | 542.1 s | +1.1% | 3806 MB | 3806 MB | -0.0% | ~ |
| `Mesh` | 20.6 s | 20.7 s | +0.4% | 798 MB | 797 MB | -0.0% | ~ |
| `MeshCSG` | 372.5 s | 370.1 s | -0.6% | 5160 MB | 5161 MB | +0.0% | ~ |
| `MeshReconcile` | 10.1 s | 10.0 s | -1.8% | 508 MB | 508 MB | +0.0% | ~ |
| `PointCloud` | 12.7 s | 12.7 s | +0.1% | 566 MB | 566 MB | -0.0% | ~ |
| `PointCloudBVHBuild` | 7.3 s | 7.0 s | -4.9% | 503 MB | 503 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 6.5 s | 6.1 s | -5.2% | 503 MB | 503 MB | +0.1% | ~ |
| `Ray` | 191.0 s | 189.7 s | -0.7% | 3703 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 221.6 s | 220.2 s | -0.6% | 4086 MB | 4086 MB | +0.0% | ~ |
| `RayCrossing` | 436.8 s | 434.9 s | -0.4% | 4017 MB | 4017 MB | -0.0% | ~ |
| `RayParticles` | 674.9 s | 674.3 s | -0.1% | 5632 MB | 5632 MB | +0.0% | ~ |
| `RayShell` | 412.1 s | 411.9 s | -0.0% | 5345 MB | 5345 MB | -0.0% | ~ |
| `Regions` | 362.6 s | 357.1 s | -1.5% | 3878 MB | 3835 MB | -1.1% | ~ |
| `Render` | 228.1 s | 227.8 s | -0.1% | 5216 MB | 5216 MB | -0.0% | ~ |
| `Shapes` | 26.0 s | 26.0 s | -0.1% | 587 MB | 587 MB | +0.1% | ~ |
| `SphereCloudBVH` | 8.6 s | 8.4 s | -1.7% | 503 MB | 504 MB | +0.0% | ~ |
| `SphereCloudRays` | 189.6 s | 187.1 s | -1.4% | 4093 MB | 4093 MB | -0.0% | ~ |
| `SphereContacts` | 8.3 s | 8.5 s | +3.0% | 503 MB | 503 MB | +0.0% | ~ |
| `SurfaceParts` | 407.5 s | 408.3 s | +0.2% | 4242 MB | 4242 MB | -0.0% | ~ |
| `Tape` | 64.0 s | 64.7 s | +1.0% | 977 MB | 978 MB | +0.0% | ~ |
| `TapeBVHUnion` | 41.2 s | 41.5 s | +0.6% | 918 MB | 918 MB | +0.0% | ~ |
| **all, in both** | 4724.2 s | 4717.4 s | -0.1% | 5632 MB | 5632 MB | +0.0% | ~ |

Device kernels: 194 in both, 2 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, true>, Vec3T<double> const, Regi…` | 168 → 168 | 16992 → 17072 | 8/8 → 8/8 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<float, true>, Vec3T<float> const, Region…` | 168 → 168 | 12208 → 12240 | 8/8 → 8/8 | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (1 cases): MeshCSG/smooth-fused/firstHit-morton/double/hostdevice

These probes compiled to the same code on both sides and were not run (332 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), LaunchTuning (24), Mesh (8), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
