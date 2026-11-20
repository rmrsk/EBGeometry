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
head: 32b817b
head-fingerprint: 2529c6bbeddb1f253b11cecae959307d8c932eb6
cases: 34
rounds: 2
reps: 3
date: 2026-11-20
-->

## Host comparison: green

**Accepted:** -

`32b817b` against `1429072` (origin/main), 2026-11-20.

**VERDICT: no regression (0 major, 0 minor across 34 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 34 cases

- base: 1429072 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 32b817b (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 27.1 ms | 27.0 ms | -0.4% | [-2.5%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.7 ms | 15.6 ms | -1.0% | [-3.9%, +2.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 328 ms | 331 ms | +0.9% | [-5.9%, +8.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 236 ms | 235 ms | -0.5% | [-4.4%, +3.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 93.1 ms | 92.4 ms | -0.9% | [-4.8%, +3.3%] | +0.0% | ~ | tape/fused 3.58x |
| `MeshCSG/smooth-fused/frame/float/host` | 82.8 ms | 83.0 ms | +0.2% | [-2.8%, +3.3%] | +0.0% | ~ | tape/fused 2.83x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.9 ms | 30.2 ms | +0.9% | [-4.2%, +6.3%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.8 ms | 17.6 ms | -0.9% | [-3.9%, +2.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 372 ms | 377 ms | +1.3% | [-3.9%, +6.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 249 ms | 249 ms | +0.0% | [-2.0%, +2.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 101 ms | 99.0 ms | -1.8% | [-6.2%, +2.9%] | +0.0% | ~ | tape/fused 3.81x (was 3.69x) |
| `MeshCSG/smooth-fused/frame/double/host` | 89.0 ms | 88.8 ms | -0.2% | [-3.2%, +2.8%] | +0.0% | ~ | tape/fused 2.80x |
| `MeshTape/mesh/firstHit/float/host` | 50.2 ms | 50.6 ms | +0.8% | [-7.0%, +9.2%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 48.4 ms | 48.6 ms | +0.5% | [+0.0%, +1.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.5 ms | 19.6 ms | +0.6% | [-0.2%, +1.4%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 81.2 ms | 83.0 ms | +2.2% | [-0.9%, +5.4%] | +0.0% | ~ | mesh/tape 0.61x |
| `MeshTape/tape/firstCrossing/float/host` | 56.7 ms | 53.3 ms | -6.1% | [-11.6%, +0.2%] | +0.0% | ~ | mesh/tape 0.91x (was 0.85x) |
| `MeshTape/tape/frame/float/host` | 25.0 ms | 24.7 ms | -1.3% | [-5.1%, +2.4%] | +0.0% | ~ | mesh/tape 0.79x |
| `MeshTape/view/firstHit/float/host` | 86.5 ms | 85.8 ms | -0.9% | [-6.8%, +5.5%] | +0.0% | ~ | mesh/view 0.59x |
| `MeshTape/view/frame/float/host` | 26.5 ms | 27.3 ms | +3.3% | [-1.6%, +8.5%] | +0.0% | ~ | mesh/view 0.72x |
| `MeshTape/mesh/firstHit/double/host` | 56.1 ms | 56.5 ms | +0.8% | [-7.1%, +9.7%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 55.9 ms | 55.1 ms | -1.5% | [-8.2%, +5.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.9 ms | 20.9 ms | -0.0% | [-2.3%, +2.3%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 90.0 ms | 89.6 ms | -0.5% | [-3.9%, +3.0%] | +0.0% | ~ | mesh/tape 0.63x |
| `MeshTape/tape/firstCrossing/double/host` | 57.7 ms | 56.9 ms | -1.4% | [-9.7%, +7.9%] | +0.0% | ~ | mesh/tape 0.97x |
| `MeshTape/tape/frame/double/host` | 27.1 ms | 27.4 ms | +0.9% | [-3.9%, +5.8%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 93.1 ms | 91.0 ms | -2.3% | [-6.9%, +2.6%] | +0.0% | ~ | mesh/view 0.62x (was 0.60x) |
| `MeshTape/view/frame/double/host` | 29.2 ms | 29.3 ms | +0.3% | [-3.2%, +3.7%] | +0.0% | ~ | mesh/view 0.71x |
| `Render/csg/frame/float/host` | 13.3 ms | 12.7 ms | -4.2% | [-9.1%, +0.7%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 12.7 ms | 13.1 ms | +3.1% | [-1.1%, +7.6%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.2 ms | 20.2 ms | -0.1% | [-0.8%, +0.6%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.2 ms | 14.2 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.0 ms | 14.0 ms | +0.3% | [-1.5%, +2.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.2 ms | 21.2 ms | -0.4% | [-2.7%, +2.0%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 6 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 738.0 s → 712.5 s (-3.5%), the largest peak 1418 MB → 1418 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.4 s | 9.4 s | -0.4% | 558 MB | 558 MB | -0.0% | ~ |
| `BVHRefit` | 4.2 s | 4.2 s | +0.0% | 497 MB | 497 MB | -0.0% | ~ |
| `BVHUnion` | 18.7 s | 18.6 s | -0.5% | 948 MB | 948 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.5 s | 7.5 s | -0.2% | 557 MB | 557 MB | +0.1% | ~ |
| `ClosestPrimitive` | 29.2 s | 29.0 s | -0.7% | 1297 MB | 1300 MB | +0.2% | ~ |
| `ClosestSurface` | 27.1 s | 26.9 s | -0.6% | 1298 MB | 1297 MB | -0.0% | ~ |
| `Domain` | 17.5 s | 17.4 s | -0.5% | 843 MB | 843 MB | -0.1% | ~ |
| `DomainObject` | 15.5 s | 15.4 s | -0.8% | 846 MB | 846 MB | -0.0% | ~ |
| `EBGrid` | 14.1 s | 13.9 s | -1.5% | 781 MB | 780 MB | -0.0% | ~ |
| `Fused` | 24.1 s | 23.5 s | -2.2% | 1208 MB | 1208 MB | +0.0% | ~ |
| `LaunchTuning` | 36.3 s | 35.3 s | -2.8% | 1279 MB | 1279 MB | +0.0% | ~ |
| `Mesh` | 15.5 s | 15.1 s | -2.3% | 786 MB | 786 MB | -0.0% | ~ |
| `MeshCSG` | 38.6 s | 37.6 s | -2.7% | 1259 MB | 1259 MB | -0.0% | ~ |
| `MeshClosestPoint` | 14.2 s | 13.6 s | -4.6% | 782 MB | 782 MB | -0.1% | ~ |
| `MeshIngest` | 6.8 s | 6.7 s | -2.4% | 569 MB | 568 MB | -0.0% | ~ |
| `MeshReconcile` | 6.6 s | 6.5 s | -2.6% | 569 MB | 568 MB | -0.1% | ~ |
| `MeshTape` | 41.0 s | 39.7 s | -3.3% | 1280 MB | 1279 MB | -0.0% | ~ |
| `PointCloud` | 7.2 s | 7.0 s | -2.5% | 560 MB | 560 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 3.7 s | 3.6 s | -4.1% | 472 MB | 472 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.2 s | 3.1 s | -3.1% | 452 MB | 452 MB | +0.1% | ~ |
| `QueryOrder` | 11.1 s | 10.8 s | -2.6% | 692 MB | 692 MB | -0.0% | ~ |
| `Ray` | 35.5 s | 34.7 s | -2.1% | 1292 MB | 1292 MB | -0.0% | ~ |
| `RayCSG` | 31.6 s | 30.6 s | -3.1% | 1192 MB | 1193 MB | +0.1% | ~ |
| `RayCrossing` | 44.1 s | 42.8 s | -3.0% | 1345 MB | 1345 MB | +0.0% | ~ |
| `RayParticles` | 48.0 s | 46.3 s | -3.6% | 1332 MB | 1332 MB | +0.0% | ~ |
| `RayShell` | 36.5 s | 35.6 s | -2.3% | 1266 MB | 1266 MB | -0.0% | ~ |
| `Regions` | 29.3 s | 27.6 s | -5.9% | 1073 MB | 1074 MB | +0.1% | ~ |
| `Render` | 37.8 s | 34.6 s | -8.6% | 1273 MB | 1274 MB | +0.0% | ~ |
| `Shapes` | 8.4 s | 8.3 s | -1.0% | 555 MB | 556 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.6 s | 4.5 s | -1.1% | 512 MB | 512 MB | +0.0% | ~ |
| `SphereCloudRays` | 28.8 s | 27.1 s | -5.8% | 1065 MB | 1065 MB | +0.0% | ~ |
| `SphereContacts` | 4.2 s | 4.1 s | -3.4% | 486 MB | 486 MB | +0.0% | ~ |
| `SurfaceParts` | 43.1 s | 40.9 s | -5.0% | 1418 MB | 1418 MB | +0.0% | ~ |
| `Tape` | 19.1 s | 16.4 s | -14.4% | 830 MB | 830 MB | -0.0% | ~ |
| `TapeBVHUnion` | 15.3 s | 14.2 s | -7.3% | 826 MB | 826 MB | +0.0% | ~ |
| **all, in both** | 738.0 s | 712.5 s | -3.5% | 1418 MB | 1418 MB | +0.0% | ~ |


These probes compiled to the same code on both sides and were not run (294 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: 1429072
base-fingerprint: 6c3c437020716e933adeba2f122c30695cb6e811
head: 32b817b
head-fingerprint: 2529c6bbeddb1f253b11cecae959307d8c932eb6
cases: 40
rounds: 2
reps: 3
date: 2026-11-20
-->

## Device comparison (CUDA): green

**Accepted:** -

`32b817b` against `1429072` (1429072), 2026-11-20.

**VERDICT: no regression (0 major, 0 minor across 40 cases)**

Performance comparison: 0 major, 0 minor, 1 faster, 0 differ, 0 lost, 40 cases

- base: 1429072 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 32b817b (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.03 ms | 2.02 ms | -0.3% | [-0.9%, +0.3%] | - | ~ | device 5.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.38 ms | 3.40 ms | +0.7% | [-0.4%, +1.8%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.14 ms | 6.12 ms | -0.2% | [-0.5%, +0.1%] | - | ~ | device 4.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 62.9 ms | 63.0 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 92.1 ms | 92.9 ms | +0.9% | [-0.7%, +2.5%] | - | ~ | device 2.8x faster than the host (was 2.3x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 240 ms | 237 ms | -1.4% | [-2.4%, -0.4%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.7 ms | 24.7 ms | +0.2% | [-0.2%, +0.5%] | - | ~ | device 2.0x faster than the host; tape/fused 2.55x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.5 ms | 26.6 ms | +0.7% | [+0.1%, +1.2%] | - | ~ | device 2.2x faster than the host; tape/fused 3.49x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 65.8 ms | 65.5 ms | -0.5% | [-2.4%, +1.4%] | - | ~ | device 3.9x faster than the host (was 4.0x); tape/fused 3.61x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.3 ms | 21.9 ms | +2.6% | [-2.9%, +8.2%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 38.3 ms | 38.0 ms | -0.9% | [-1.5%, -0.2%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 41.1 ms | 42.2 ms | +2.8% | [-1.0%, +6.8%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 559 ms | 540 ms | -3.3% | [-5.2%, -1.4%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 846 ms | 843 ms | -0.3% | [-2.1%, +1.5%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.97 s | 1.97 s | +0.2% | [-1.5%, +1.9%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 197 ms | 194 ms | -1.5% | [-2.6%, -0.4%] | - | ~ | device 0.3x faster than the host; tape/fused 2.79x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 302 ms | 310 ms | +2.8% | [+0.3%, +5.3%] | - | ~ | device 0.2x faster than the host; tape/fused 2.72x (was 2.80x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 532 ms | 538 ms | +1.3% | [-0.1%, +2.6%] | - | ~ | device 0.5x faster than the host; tape/fused 3.66x |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.30 ms | 1.29 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 15.8x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.30 ms | 1.29 ms | -0.4% | [-0.7%, -0.2%] | - | ~ | device 15.9x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.14 ms | 2.12 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 7.9x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.50 ms | 3.50 ms | -0.0% | [-0.2%, +0.2%] | - | ~ | device 10.2x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.75 ms | 2.75 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 7.8x faster than the host; mesh/tape 0.47x |
| `MeshTape/tape/frame/float/hostdevice` | 4.40 ms | 4.40 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 4.9x faster than the host; mesh/tape 0.48x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.74 ms | 3.73 ms | -0.2% | [-0.4%, +0.1%] | - | ~ | device 9.7x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.53 ms | 5.56 ms | +0.7% | [+0.5%, +0.9%] | - | ~ | device 4.2x faster than the host; mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 28.5 ms | 28.5 ms | -0.1% | [-0.6%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 28.6 ms | 28.5 ms | -0.4% | [-0.9%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 24.5 ms | 24.4 ms | -0.3% | [-1.0%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 40.9 ms | 40.8 ms | -0.3% | [-0.4%, -0.2%] | - | ~ | device 0.9x faster than the host; mesh/tape 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 28.7 ms | 28.7 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 31.5 ms | 31.6 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.77x |
| `MeshTape/view/firstHit/double/hostdevice` | 40.9 ms | 40.9 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 0.9x faster than the host; mesh/view 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 34.5 ms | 34.1 ms | -1.0% | [-1.8%, -0.2%] | - | ~ | device 0.7x faster than the host; mesh/view 0.72x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.37 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.36 ms | 2.36 ms | -0.0% | [-0.1%, +0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.15 ms | 2.14 ms | -0.4% | [-0.9%, +0.2%] | - | ~ | device 7.8x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.62 ms | 5.61 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.62 ms | 5.61 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.4x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 24.6 ms | 24.5 ms | -0.3% | [-1.5%, +0.9%] | - | ~ | device 0.7x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 2 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4683.3 s → 4578.3 s (-2.2%), the largest peak 6566 MB → 6570 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.0 s | 11.8 s | -9.4% | 605 MB | 603 MB | -0.4% | ~ |
| `BVHRefit` | 6.8 s | 6.1 s | -10.3% | 509 MB | 509 MB | +0.0% | ~ |
| `BVHUnion` | 24.7 s | 22.1 s | -10.4% | 1028 MB | 1028 MB | -0.0% | ~ |
| `BVHUnionMixed` | 11.1 s | 9.8 s | -11.9% | 623 MB | 623 MB | -0.0% | ~ |
| `ClosestPrimitive` | 114.8 s | 101.2 s | -11.9% | 1353 MB | 1353 MB | +0.1% | ~ |
| `ClosestSurface` | 165.3 s | 142.2 s | -14.0% | 1844 MB | 1844 MB | +0.0% | ~ |
| `Domain` | 57.6 s | 47.1 s | -18.2% | 991 MB | 991 MB | +0.0% | ~ |
| `DomainObject` | 50.6 s | 43.8 s | -13.4% | 944 MB | 944 MB | -0.0% | ~ |
| `EBGrid` | 18.8 s | 17.1 s | -8.8% | 753 MB | 753 MB | -0.1% | ~ |
| `Fused` | 48.4 s | 43.9 s | -9.3% | 1163 MB | 1164 MB | +0.0% | ~ |
| `LaunchTuning` | 419.9 s | 409.3 s | -2.5% | 3876 MB | 3876 MB | +0.0% | ~ |
| `Mesh` | 18.6 s | 18.0 s | -3.0% | 810 MB | 810 MB | -0.0% | ~ |
| `MeshCSG` | 330.3 s | 322.1 s | -2.5% | 6302 MB | 6306 MB | +0.1% | ~ |
| `MeshClosestPoint` | 16.6 s | 16.5 s | -1.1% | 762 MB | 762 MB | -0.0% | ~ |
| `MeshIngest` | 8.5 s | 8.5 s | -0.7% | 535 MB | 535 MB | +0.0% | ~ |
| `MeshReconcile` | 8.4 s | 8.3 s | -1.2% | 512 MB | 513 MB | +0.0% | ~ |
| `MeshTape` | 618.8 s | 622.4 s | +0.6% | 6566 MB | 6570 MB | +0.1% | ~ |
| `PointCloud` | 10.3 s | 10.2 s | -0.5% | 564 MB | 564 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 5.7 s | 5.6 s | -1.2% | 510 MB | 510 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 5.3 s | 5.2 s | -1.5% | 510 MB | 510 MB | +0.0% | ~ |
| `QueryOrder` | 13.3 s | 13.2 s | -0.5% | 694 MB | 694 MB | +0.0% | ~ |
| `Ray` | 148.7 s | 147.5 s | -0.8% | 3769 MB | 3770 MB | +0.0% | ~ |
| `RayCSG` | 170.7 s | 169.8 s | -0.5% | 4155 MB | 4155 MB | +0.0% | ~ |
| `RayCrossing` | 334.1 s | 339.9 s | +1.7% | 4117 MB | 4116 MB | -0.0% | ~ |
| `RayParticles` | 612.5 s | 604.0 s | -1.4% | 5807 MB | 5807 MB | +0.0% | ~ |
| `RayShell` | 330.6 s | 324.9 s | -1.7% | 5501 MB | 5500 MB | -0.0% | ~ |
| `Regions` | 281.4 s | 278.3 s | -1.1% | 4138 MB | 4139 MB | +0.0% | ~ |
| `Render` | 203.6 s | 202.1 s | -0.8% | 6083 MB | 6080 MB | -0.0% | ~ |
| `Shapes` | 19.7 s | 19.0 s | -3.4% | 585 MB | 585 MB | +0.0% | ~ |
| `SphereCloudBVH` | 7.1 s | 7.0 s | -1.7% | 510 MB | 510 MB | +0.0% | ~ |
| `SphereCloudRays` | 144.7 s | 143.0 s | -1.2% | 4170 MB | 4170 MB | +0.0% | ~ |
| `SphereContacts` | 6.6 s | 6.6 s | -0.2% | 509 MB | 509 MB | +0.0% | ~ |
| `SurfaceParts` | 371.8 s | 367.3 s | -1.2% | 4416 MB | 4416 MB | +0.0% | ~ |
| `Tape` | 49.8 s | 49.5 s | -0.6% | 977 MB | 978 MB | +0.1% | ~ |
| `TapeBVHUnion` | 35.5 s | 35.0 s | -1.3% | 918 MB | 918 MB | -0.0% | ~ |
| **all, in both** | 4683.3 s | 4578.3 s | -2.2% | 6566 MB | 6570 MB | +0.1% | ~ |

Device kernels: 226 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (1 cases): MeshCSG/smooth-tape/frame/double/hostdevice

These probes compiled to the same code on both sides and were not run (374 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), EBGrid (12), Fused (10), LaunchTuning (24), Mesh (8), MeshClosestPoint (4), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), QueryOrder (6), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
