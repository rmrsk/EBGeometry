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
verdict: yellow
base: baf4da2
base-fingerprint: 6af8d836690017a737dfb1e86605c1553739e80e
head: a13371f
head-fingerprint: 4c8a55718a2c1fff0986eab16001598161c76227
cases: 34
rounds: 2
reps: 3
date: 2026-11-24
-->

## Host comparison: yellow

**Accepted:** Comments only: the Render probe compiled differently only by `__LINE__` constants, and float CSG frames run at two speeds from one run to the next; a layout draw, not a cost.

`a13371f` against `baf4da2` (origin/main), 2026-11-24.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 1 faster, 0 differ, 0 lost, 34 cases

- base: baf4da2 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: a13371f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 27.9 ms | 28.0 ms | +0.5% | [-0.1%, +1.1%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 16.6 ms | 16.4 ms | -1.0% | [-4.1%, +2.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 343 ms | 342 ms | -0.0% | [-0.7%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 242 ms | 242 ms | -0.2% | [-0.8%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 96.1 ms | 96.6 ms | +0.5% | [+0.2%, +0.9%] | +0.0% | ~ | tape/fused 3.54x |
| `MeshCSG/smooth-fused/frame/float/host` | 85.8 ms | 85.8 ms | -0.0% | [-0.8%, +0.7%] | +0.0% | ~ | tape/fused 2.82x |
| `MeshCSG/union-tape/firstHit/double/host` | 31.0 ms | 30.9 ms | -0.3% | [-1.1%, +0.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 18.1 ms | 18.2 ms | +0.6% | [-0.2%, +1.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 394 ms | 393 ms | -0.1% | [-0.6%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 257 ms | 257 ms | -0.2% | [-1.1%, +0.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 104 ms | 104 ms | +0.8% | [-0.1%, +1.7%] | +0.0% | ~ | tape/fused 3.76x |
| `MeshCSG/smooth-fused/frame/double/host` | 92.0 ms | 91.6 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ | tape/fused 2.80x |
| `MeshTape/mesh/firstHit/float/host` | 52.5 ms | 52.5 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 52.7 ms | 52.9 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 20.7 ms | 20.7 ms | +0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 85.5 ms | 85.4 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ | mesh/tape 0.62x |
| `MeshTape/tape/firstCrossing/float/host` | 53.7 ms | 53.5 ms | -0.2% | [-1.1%, +0.7%] | +0.0% | ~ | mesh/tape 0.99x |
| `MeshTape/tape/frame/float/host` | 27.3 ms | 25.2 ms | -7.8% | [-9.6%, -6.0%] | +0.0% | faster | mesh/tape 0.82x (was 0.76x) |
| `MeshTape/view/firstHit/float/host` | 88.6 ms | 89.0 ms | +0.5% | [-0.3%, +1.2%] | +0.0% | ~ | mesh/view 0.59x |
| `MeshTape/view/frame/float/host` | 27.8 ms | 27.7 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ | mesh/view 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 59.1 ms | 58.9 ms | -0.3% | [-1.7%, +1.0%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 59.6 ms | 58.9 ms | -1.3% | [-2.7%, +0.2%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 21.8 ms | 21.5 ms | -1.2% | [-2.7%, +0.2%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 95.5 ms | 95.9 ms | +0.5% | [-1.9%, +2.9%] | +0.0% | ~ | mesh/tape 0.61x |
| `MeshTape/tape/firstCrossing/double/host` | 60.8 ms | 61.4 ms | +0.9% | [+0.0%, +1.8%] | +0.0% | ~ | mesh/tape 0.96x |
| `MeshTape/tape/frame/double/host` | 28.4 ms | 28.4 ms | -0.2% | [-0.7%, +0.4%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 98.9 ms | 99.0 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | mesh/view 0.60x |
| `MeshTape/view/frame/double/host` | 31.0 ms | 31.3 ms | +1.0% | [+0.6%, +1.4%] | +0.0% | ~ | mesh/view 0.69x |
| `Render/csg/frame/float/host` | 12.9 ms | 12.9 ms | +0.1% | [-7.5%, +8.3%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 12.3 ms | 13.3 ms | +8.4% | [+0.2%, +8.5%] | +0.0% | minor |  |
| `Render/mesh/frame/float/host` | 20.9 ms | 20.8 ms | -0.2% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.6 ms | 14.5 ms | -0.8% | [-1.5%, -0.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 14.3 ms | 14.3 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 21.8 ms | 21.9 ms | +0.2% | [-0.5%, +0.9%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 836.0 s → 789.0 s (-5.6%), the largest peak 1418 MB → 1418 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.7 s | 9.9 s | +2.4% | 562 MB | 563 MB | +0.1% | ~ |
| `BVHRefit` | 4.3 s | 4.4 s | +0.3% | 503 MB | 503 MB | +0.1% | ~ |
| `BVHUnion` | 19.3 s | 19.6 s | +1.6% | 956 MB | 956 MB | -0.0% | ~ |
| `BVHUnionMixed` | 7.8 s | 8.3 s | +6.1% | 559 MB | 559 MB | -0.1% | ~ |
| `ClosestPrimitive` | 30.8 s | 31.6 s | +2.9% | 1347 MB | 1348 MB | +0.0% | ~ |
| `ClosestSurface` | 28.5 s | 29.2 s | +2.7% | 1318 MB | 1317 MB | -0.1% | ~ |
| `ConnectedComponents` | 15.4 s | 15.8 s | +2.3% | 854 MB | 855 MB | +0.1% | ~ |
| `Domain` | 18.2 s | 18.5 s | +1.8% | 860 MB | 860 MB | +0.0% | ~ |
| `DomainObject` | 16.0 s | 16.7 s | +4.2% | 860 MB | 860 MB | +0.0% | ~ |
| `EBGrid` | 14.8 s | 15.4 s | +4.1% | 784 MB | 784 MB | +0.0% | ~ |
| `Fused` | 25.0 s | 25.5 s | +1.9% | 1237 MB | 1236 MB | -0.1% | ~ |
| `LaunchTuning` | 40.3 s | 38.4 s | -4.8% | 1328 MB | 1328 MB | +0.0% | ~ |
| `Mesh` | 16.2 s | 16.1 s | -0.4% | 790 MB | 791 MB | +0.1% | ~ |
| `MeshCSG` | 50.0 s | 40.9 s | -18.2% | 1306 MB | 1306 MB | +0.0% | ~ |
| `MeshClosestPoint` | 15.2 s | 14.9 s | -1.8% | 786 MB | 786 MB | -0.1% | ~ |
| `MeshIngest` | 7.7 s | 7.3 s | -4.2% | 569 MB | 568 MB | -0.1% | ~ |
| `MeshReconcile` | 8.5 s | 7.0 s | -18.3% | 569 MB | 569 MB | +0.1% | ~ |
| `MeshTape` | 50.7 s | 43.3 s | -14.7% | 1260 MB | 1261 MB | +0.0% | ~ |
| `PointCloud` | 8.4 s | 7.7 s | -8.6% | 562 MB | 562 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 3.9 s | 4.0 s | +2.2% | 476 MB | 476 MB | -0.1% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.4 s | +3.0% | 457 MB | 457 MB | -0.0% | ~ |
| `QueryOrder` | 12.6 s | 12.2 s | -2.7% | 707 MB | 707 MB | +0.0% | ~ |
| `Ray` | 45.2 s | 38.0 s | -15.9% | 1342 MB | 1342 MB | +0.0% | ~ |
| `RayCSG` | 38.8 s | 32.9 s | -15.2% | 1204 MB | 1205 MB | +0.1% | ~ |
| `RayCrossing` | 52.1 s | 47.2 s | -9.4% | 1360 MB | 1360 MB | -0.0% | ~ |
| `RayParticles` | 59.8 s | 51.4 s | -14.0% | 1334 MB | 1334 MB | +0.0% | ~ |
| `RayShell` | 41.4 s | 39.2 s | -5.4% | 1314 MB | 1314 MB | -0.0% | ~ |
| `Regions` | 31.6 s | 30.2 s | -4.3% | 1116 MB | 1116 MB | +0.0% | ~ |
| `Render` | 37.8 s | 37.1 s | -2.1% | 1300 MB | 1299 MB | -0.0% | ~ |
| `Shapes` | 9.1 s | 9.2 s | +2.1% | 559 MB | 559 MB | +0.0% | ~ |
| `SphereCloudBVH` | 5.0 s | 5.0 s | +0.1% | 518 MB | 518 MB | +0.0% | ~ |
| `SphereCloudRays` | 29.0 s | 29.1 s | +0.2% | 1077 MB | 1077 MB | -0.0% | ~ |
| `SphereContacts` | 4.8 s | 4.5 s | -6.8% | 490 MB | 490 MB | +0.0% | ~ |
| `SurfaceParts` | 42.2 s | 43.0 s | +1.8% | 1418 MB | 1418 MB | -0.0% | ~ |
| `Tape` | 17.5 s | 17.1 s | -2.4% | 839 MB | 837 MB | -0.2% | ~ |
| `TapeBVHUnion` | 15.0 s | 15.1 s | +0.4% | 839 MB | 839 MB | -0.0% | ~ |
| **all, in both** | 836.0 s | 789.0 s | -5.6% | 1418 MB | 1418 MB | -0.0% | ~ |


Flagged after 2 rounds, and judged on 10 (1 cases): Render/csg/frame-orthographic/float/host

These probes compiled to the same code on both sides and were not run (298 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), ConnectedComponents (4), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
