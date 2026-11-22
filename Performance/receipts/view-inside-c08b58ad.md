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
base: 1429072
base-fingerprint: 6c3c437020716e933adeba2f122c30695cb6e811
head: 2b3ae35
head-fingerprint: c08b58ad39bca2aeae0dc89c83b175e23eae9a89
cases: 34
rounds: 2
reps: 3
date: 2026-11-22
-->

## Host comparison: green

**Accepted:** -

`2b3ae35` against `1429072` (1429072), 2026-11-22.

**VERDICT: no regression (0 major, 0 minor across 34 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 34 cases

- base: 1429072 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 2b3ae35 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 26.2 ms | 26.3 ms | +0.3% | [-0.5%, +1.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.5 ms | 15.7 ms | +1.4% | [+0.2%, +2.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 318 ms | 320 ms | +0.8% | [+0.2%, +1.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 230 ms | 231 ms | +0.0% | [-0.5%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.4 ms | 90.1 ms | -0.3% | [-0.5%, -0.1%] | +0.0% | ~ | tape/fused 3.55x |
| `MeshCSG/smooth-fused/frame/float/host` | 81.2 ms | 81.8 ms | +0.8% | [+0.3%, +1.3%] | +0.0% | ~ | tape/fused 2.82x |
| `MeshCSG/union-tape/firstHit/double/host` | 28.5 ms | 28.6 ms | +0.3% | [-0.4%, +1.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.1 ms | 17.1 ms | -0.2% | [-2.0%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 355 ms | -0.6% | [-0.8%, -0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 245 ms | 245 ms | +0.2% | [-0.1%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 96.7 ms | 96.6 ms | -0.1% | [-0.4%, +0.1%] | +0.0% | ~ | tape/fused 3.68x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.0 ms | 86.9 ms | -0.1% | [-0.2%, +0.0%] | +0.0% | ~ | tape/fused 2.82x |
| `MeshTape/mesh/firstHit/float/host` | 48.6 ms | 48.2 ms | -0.8% | [-1.4%, -0.2%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 48.4 ms | 48.2 ms | -0.4% | [-1.2%, +0.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.5 ms | 19.5 ms | -0.1% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 80.2 ms | 80.1 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ | mesh/tape 0.60x |
| `MeshTape/tape/firstCrossing/float/host` | 49.8 ms | 49.6 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ | mesh/tape 0.97x |
| `MeshTape/tape/frame/float/host` | 24.9 ms | 24.9 ms | +0.0% | [-3.5%, +3.6%] | +0.0% | ~ | mesh/tape 0.78x |
| `MeshTape/view/firstHit/float/host` | 82.9 ms | 83.2 ms | +0.3% | [-0.0%, +0.7%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/float/host` | 25.8 ms | 25.9 ms | +0.5% | [+0.2%, +0.7%] | +0.0% | ~ | mesh/view 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 52.1 ms | 52.4 ms | +0.5% | [-0.5%, +1.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 52.4 ms | 53.0 ms | +1.2% | [-0.5%, +2.8%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.5 ms | 20.3 ms | -1.3% | [-2.9%, +0.3%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 86.2 ms | 86.6 ms | +0.4% | [-1.5%, +2.4%] | +0.0% | ~ | mesh/tape 0.61x |
| `MeshTape/tape/firstCrossing/double/host` | 54.1 ms | 55.2 ms | +2.1% | [+1.0%, +3.2%] | +0.0% | ~ | mesh/tape 0.96x |
| `MeshTape/tape/frame/double/host` | 26.5 ms | 26.7 ms | +0.8% | [+0.0%, +1.7%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 90.8 ms | 88.9 ms | -2.1% | [-3.6%, -0.5%] | +0.0% | ~ | mesh/view 0.59x |
| `MeshTape/view/frame/double/host` | 28.5 ms | 28.7 ms | +0.5% | [-0.5%, +1.5%] | +0.0% | ~ | mesh/view 0.71x |
| `Render/csg/frame/float/host` | 12.6 ms | 12.1 ms | -4.1% | [-8.7%, +1.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 12.7 ms | 12.5 ms | -1.4% | [-9.4%, +7.4%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.6 ms | 19.5 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 13.9 ms | 14.0 ms | +0.7% | [-0.1%, +1.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 13.8 ms | 13.8 ms | +0.3% | [+0.2%, +0.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.3 ms | 20.2 ms | -0.4% | [-0.9%, +0.2%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 6 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 738.0 s → 740.4 s (+0.3%), the largest peak 1418 MB → 1419 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.4 s | 9.8 s | +4.0% | 558 MB | 559 MB | +0.1% | ~ |
| `BVHRefit` | 4.2 s | 4.3 s | +3.0% | 497 MB | 498 MB | +0.1% | ~ |
| `BVHUnion` | 18.7 s | 19.3 s | +3.2% | 948 MB | 948 MB | -0.0% | ~ |
| `BVHUnionMixed` | 7.5 s | 7.7 s | +2.7% | 557 MB | 558 MB | +0.2% | ~ |
| `ClosestPrimitive` | 29.2 s | 30.4 s | +4.2% | 1297 MB | 1342 MB | +3.5% | ~ |
| `ClosestSurface` | 27.1 s | 28.2 s | +4.2% | 1298 MB | 1301 MB | +0.3% | ~ |
| `Domain` | 17.5 s | 18.0 s | +3.0% | 843 MB | 843 MB | +0.0% | ~ |
| `DomainObject` | 15.5 s | 16.0 s | +2.9% | 846 MB | 847 MB | +0.1% | ~ |
| `EBGrid` | 14.1 s | 14.5 s | +2.5% | 781 MB | 782 MB | +0.2% | ~ |
| `Fused` | 24.1 s | 25.0 s | +3.8% | 1208 MB | 1214 MB | +0.5% | ~ |
| `LaunchTuning` | 36.3 s | 36.7 s | +0.9% | 1279 MB | 1280 MB | +0.0% | ~ |
| `Mesh` | 15.5 s | 16.1 s | +3.7% | 786 MB | 787 MB | +0.1% | ~ |
| `MeshCSG` | 38.6 s | 39.8 s | +3.2% | 1259 MB | 1259 MB | -0.0% | ~ |
| `MeshClosestPoint` | 14.2 s | 14.7 s | +3.0% | 782 MB | 783 MB | +0.1% | ~ |
| `MeshIngest` | 6.8 s | 7.3 s | +6.7% | 569 MB | 567 MB | -0.2% | ~ |
| `MeshReconcile` | 6.6 s | 6.9 s | +4.2% | 569 MB | 569 MB | +0.1% | ~ |
| `MeshTape` | 41.0 s | 41.5 s | +1.0% | 1280 MB | 1269 MB | -0.8% | ~ |
| `PointCloud` | 7.2 s | 7.5 s | +4.8% | 560 MB | 561 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 3.7 s | 3.8 s | +0.9% | 472 MB | 472 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 3.2 s | 3.3 s | +3.9% | 452 MB | 453 MB | +0.2% | ~ |
| `QueryOrder` | 11.1 s | 11.1 s | -0.4% | 692 MB | 692 MB | +0.1% | ~ |
| `Ray` | 35.5 s | 36.1 s | +1.8% | 1292 MB | 1292 MB | +0.1% | ~ |
| `RayCSG` | 31.6 s | 32.0 s | +1.4% | 1192 MB | 1203 MB | +0.9% | ~ |
| `RayCrossing` | 44.1 s | 44.4 s | +0.6% | 1345 MB | 1339 MB | -0.4% | ~ |
| `RayParticles` | 48.0 s | 48.1 s | +0.2% | 1332 MB | 1327 MB | -0.4% | ~ |
| `RayShell` | 36.5 s | 37.1 s | +1.8% | 1266 MB | 1267 MB | +0.0% | ~ |
| `Regions` | 29.3 s | 28.1 s | -3.9% | 1073 MB | 1074 MB | +0.1% | ~ |
| `Render` | 37.8 s | 35.5 s | -6.1% | 1273 MB | 1273 MB | -0.0% | ~ |
| `Shapes` | 8.4 s | 8.6 s | +2.8% | 555 MB | 556 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.6 s | 4.7 s | +2.0% | 512 MB | 512 MB | +0.0% | ~ |
| `SphereCloudRays` | 28.8 s | 27.9 s | -3.0% | 1065 MB | 1066 MB | +0.1% | ~ |
| `SphereContacts` | 4.2 s | 4.2 s | -1.1% | 486 MB | 486 MB | +0.1% | ~ |
| `SurfaceParts` | 43.1 s | 40.4 s | -6.3% | 1418 MB | 1419 MB | +0.1% | ~ |
| `Tape` | 19.1 s | 16.7 s | -12.9% | 830 MB | 829 MB | -0.1% | ~ |
| `TapeBVHUnion` | 15.3 s | 14.8 s | -3.7% | 826 MB | 826 MB | +0.1% | ~ |
| **all, in both** | 738.0 s | 740.4 s | +0.3% | 1418 MB | 1419 MB | +0.1% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (294 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 1429072
base-fingerprint: 6c3c437020716e933adeba2f122c30695cb6e811
head: 2b3ae35
head-fingerprint: c08b58ad39bca2aeae0dc89c83b175e23eae9a89
cases: 40
rounds: 2
reps: 3
date: 2026-11-22
-->

## Device comparison (CUDA): green

**Accepted:** -

`2b3ae35` against `1429072` (1429072), 2026-11-22.

**VERDICT: no regression (0 major, 0 minor across 40 cases)**

Performance comparison: 0 major, 0 minor, 2 faster, 0 differ, 0 lost, 40 cases

- base: 1429072 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 2b3ae35 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.01 ms | 2.01 ms | -0.0% | [-0.9%, +0.9%] | - | ~ | device 5.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.37 ms | 3.37 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.14 ms | 6.16 ms | +0.2% | [-0.7%, +1.1%] | - | ~ | device 4.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 62.9 ms | 62.7 ms | -0.3% | [-1.1%, +0.5%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 92.7 ms | 92.5 ms | -0.3% | [-1.4%, +0.9%] | - | ~ | device 2.3x faster than the host (was 2.5x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 242 ms | 244 ms | +0.8% | [-1.7%, +3.3%] | - | ~ | device 3.4x faster than the host (was 3.6x) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.0 ms | 24.7 ms | -1.0% | [-2.0%, -0.1%] | - | ~ | device 2.0x faster than the host; tape/fused 2.54x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.7 ms | 26.4 ms | -1.1% | [-1.5%, -0.6%] | - | ~ | device 2.4x faster than the host (was 2.6x); tape/fused 3.50x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.5 ms | 65.0 ms | -0.8% | [-2.6%, +1.0%] | - | ~ | device 3.9x faster than the host (was 4.3x); tape/fused 3.75x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.0 ms | 20.2 ms | -3.6% | [-4.7%, -2.5%] | - | faster | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 38.4 ms | 38.1 ms | -0.8% | [-1.1%, -0.5%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 41.3 ms | 41.2 ms | -0.2% | [-4.6%, +4.4%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 523 ms | 517 ms | -1.1% | [-4.0%, +1.8%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 840 ms | 837 ms | -0.4% | [-2.0%, +1.3%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.94 s | 1.97 s | +1.6% | [+0.6%, +2.6%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 196 ms | 187 ms | -4.7% | [-8.1%, -1.1%] | - | faster | device 0.3x faster than the host (was 0.2x); tape/fused 2.77x (was 2.67x) |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 305 ms | 296 ms | -3.0% | [-6.8%, +1.1%] | - | ~ | device 0.2x faster than the host; tape/fused 2.83x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 541 ms | 551 ms | +1.9% | [-2.9%, +6.9%] | - | ~ | device 0.5x faster than the host (was 0.6x); tape/fused 3.58x |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.30 ms | 1.29 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 15.9x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.30 ms | 1.29 ms | -0.9% | [-1.1%, -0.7%] | - | ~ | device 15.9x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.13 ms | 2.14 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 7.8x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.49 ms | 3.50 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 10.2x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.74 ms | 2.75 ms | +0.2% | [+0.1%, +0.4%] | - | ~ | device 7.8x faster than the host; mesh/tape 0.47x |
| `MeshTape/tape/frame/float/hostdevice` | 4.40 ms | 4.41 ms | +0.3% | [-0.0%, +0.7%] | - | ~ | device 5.0x faster than the host; mesh/tape 0.49x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.73 ms | 3.74 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 9.7x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.55 ms | 5.58 ms | +0.4% | [+0.3%, +0.6%] | - | ~ | device 4.2x faster than the host; mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 28.5 ms | 28.5 ms | +0.2% | [-0.0%, +0.3%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 28.5 ms | 28.5 ms | +0.2% | [-0.3%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 24.4 ms | 24.6 ms | +0.8% | [+0.1%, +1.6%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 40.9 ms | 40.9 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 0.9x faster than the host; mesh/tape 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 28.8 ms | 28.7 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 31.4 ms | 31.4 ms | +0.2% | [+0.0%, +0.3%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.78x |
| `MeshTape/view/firstHit/double/hostdevice` | 41.0 ms | 41.0 ms | +0.1% | [-0.6%, +0.8%] | - | ~ | device 0.9x faster than the host; mesh/view 0.69x |
| `MeshTape/view/frame/double/hostdevice` | 34.5 ms | 34.3 ms | -0.5% | [-1.6%, +0.7%] | - | ~ | device 0.7x faster than the host; mesh/view 0.72x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.37 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.36 ms | 2.36 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.13 ms | 2.12 ms | -0.2% | [-1.2%, +0.8%] | - | ~ | device 7.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.62 ms | 5.60 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.61 ms | 5.62 ms | +0.1% | [+0.1%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 24.5 ms | 24.5 ms | +0.2% | [-1.0%, +1.3%] | - | ~ | device 0.7x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 2 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4683.3 s → 4555.8 s (-2.7%), the largest peak 6566 MB → 6570 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.0 s | 11.8 s | -9.3% | 605 MB | 605 MB | -0.1% | ~ |
| `BVHRefit` | 6.8 s | 6.1 s | -10.0% | 509 MB | 510 MB | +0.1% | ~ |
| `BVHUnion` | 24.7 s | 22.2 s | -10.1% | 1028 MB | 1028 MB | -0.0% | ~ |
| `BVHUnionMixed` | 11.1 s | 9.8 s | -11.6% | 623 MB | 624 MB | +0.1% | ~ |
| `ClosestPrimitive` | 114.8 s | 101.4 s | -11.6% | 1353 MB | 1354 MB | +0.1% | ~ |
| `ClosestSurface` | 165.3 s | 142.5 s | -13.8% | 1844 MB | 1844 MB | +0.0% | ~ |
| `Domain` | 57.6 s | 47.2 s | -18.0% | 991 MB | 991 MB | +0.0% | ~ |
| `DomainObject` | 50.6 s | 43.8 s | -13.5% | 944 MB | 945 MB | +0.0% | ~ |
| `EBGrid` | 18.8 s | 17.1 s | -8.8% | 753 MB | 753 MB | +0.0% | ~ |
| `Fused` | 48.4 s | 43.7 s | -9.8% | 1163 MB | 1164 MB | +0.0% | ~ |
| `LaunchTuning` | 419.9 s | 409.8 s | -2.4% | 3876 MB | 3876 MB | -0.0% | ~ |
| `Mesh` | 18.6 s | 18.0 s | -3.1% | 810 MB | 810 MB | +0.0% | ~ |
| `MeshCSG` | 330.3 s | 322.0 s | -2.5% | 6302 MB | 6308 MB | +0.1% | ~ |
| `MeshClosestPoint` | 16.6 s | 16.5 s | -1.0% | 762 MB | 763 MB | +0.1% | ~ |
| `MeshIngest` | 8.5 s | 8.4 s | -0.9% | 535 MB | 536 MB | +0.1% | ~ |
| `MeshReconcile` | 8.4 s | 8.3 s | -0.9% | 512 MB | 512 MB | +0.0% | ~ |
| `MeshTape` | 618.8 s | 611.1 s | -1.3% | 6566 MB | 6570 MB | +0.1% | ~ |
| `PointCloud` | 10.3 s | 10.2 s | -0.3% | 564 MB | 564 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 5.7 s | 5.7 s | -0.4% | 510 MB | 510 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 5.3 s | 5.3 s | -0.1% | 510 MB | 510 MB | +0.1% | ~ |
| `QueryOrder` | 13.3 s | 13.2 s | -0.7% | 694 MB | 694 MB | +0.1% | ~ |
| `Ray` | 148.7 s | 147.5 s | -0.8% | 3769 MB | 3769 MB | -0.0% | ~ |
| `RayCSG` | 170.7 s | 169.6 s | -0.6% | 4155 MB | 4155 MB | +0.0% | ~ |
| `RayCrossing` | 334.1 s | 331.5 s | -0.8% | 4117 MB | 4117 MB | -0.0% | ~ |
| `RayParticles` | 612.5 s | 600.0 s | -2.0% | 5807 MB | 5808 MB | +0.0% | ~ |
| `RayShell` | 330.6 s | 322.3 s | -2.5% | 5501 MB | 5502 MB | +0.0% | ~ |
| `Regions` | 281.4 s | 278.4 s | -1.1% | 4138 MB | 4139 MB | +0.0% | ~ |
| `Render` | 203.6 s | 202.7 s | -0.4% | 6083 MB | 6079 MB | -0.1% | ~ |
| `Shapes` | 19.7 s | 19.9 s | +0.8% | 585 MB | 586 MB | +0.2% | ~ |
| `SphereCloudBVH` | 7.1 s | 7.0 s | -0.6% | 510 MB | 510 MB | +0.1% | ~ |
| `SphereCloudRays` | 144.7 s | 143.1 s | -1.1% | 4170 MB | 4170 MB | +0.0% | ~ |
| `SphereContacts` | 6.6 s | 6.6 s | -0.5% | 509 MB | 510 MB | +0.1% | ~ |
| `SurfaceParts` | 371.8 s | 368.7 s | -0.8% | 4416 MB | 4416 MB | +0.0% | ~ |
| `Tape` | 49.8 s | 49.4 s | -0.9% | 977 MB | 978 MB | +0.1% | ~ |
| `TapeBVHUnion` | 35.5 s | 35.0 s | -1.4% | 918 MB | 918 MB | -0.0% | ~ |
| **all, in both** | 4683.3 s | 4555.8 s | -2.7% | 6566 MB | 6570 MB | +0.1% | ~ |

Device kernels: 226 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (374 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), EBGrid (12), Fused (10), LaunchTuning (24), Mesh (8), MeshClosestPoint (4), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), QueryOrder (6), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
