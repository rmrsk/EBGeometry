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
base: df4c34f
base-fingerprint: 5d94b849fdfbdf57bde54ec7585eaa9d2b61a0d6
head: 7e6a0b2
head-fingerprint: a23b462307605142893524cfafdb9453ae008378
cases: 34
rounds: 2
reps: 3
date: 2026-11-22
-->

## Host comparison: green

**Accepted:** -

`7e6a0b2` against `df4c34f` (df4c34f), 2026-11-22.

**VERDICT: no regression (0 major, 0 minor across 34 cases)**

Performance comparison: 0 major, 0 minor, 2 faster, 0 differ, 0 lost, 34 cases

- base: df4c34f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 7e6a0b2 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 26.1 ms | 26.1 ms | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.4 ms | 15.4 ms | +0.2% | [-2.7%, +3.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 317 ms | 317 ms | +0.0% | [-0.1%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 231 ms | 230 ms | -0.2% | [-0.5%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.3 ms | 89.8 ms | -0.5% | [-0.7%, -0.2%] | +0.0% | ~ | tape/fused 3.53x |
| `MeshCSG/smooth-fused/frame/float/host` | 81.9 ms | 81.1 ms | -1.1% | [-1.8%, -0.3%] | +0.0% | ~ | tape/fused 2.84x |
| `MeshCSG/union-tape/firstHit/double/host` | 28.5 ms | 28.4 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 17.1 ms | 17.2 ms | +0.7% | [-1.0%, +2.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 357 ms | 354 ms | -0.7% | [-1.4%, -0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 245 ms | 245 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.2 ms | 96.3 ms | -0.9% | [-1.7%, -0.2%] | +0.0% | ~ | tape/fused 3.68x |
| `MeshCSG/smooth-fused/frame/double/host` | 87.1 ms | 86.8 ms | -0.4% | [-0.7%, -0.1%] | +0.0% | ~ | tape/fused 2.82x |
| `MeshTape/mesh/firstHit/float/host` | 48.3 ms | 48.8 ms | +1.2% | [-0.2%, +2.6%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 48.3 ms | 48.4 ms | +0.2% | [-0.1%, +0.5%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/float/host` | 19.5 ms | 19.5 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 79.7 ms | 80.1 ms | +0.5% | [+0.3%, +0.7%] | +0.0% | ~ | mesh/tape 0.61x |
| `MeshTape/tape/firstCrossing/float/host` | 49.5 ms | 49.5 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ | mesh/tape 0.98x |
| `MeshTape/tape/frame/float/host` | 23.9 ms | 24.4 ms | +2.0% | [-0.1%, +4.2%] | +0.0% | ~ | mesh/tape 0.80x |
| `MeshTape/view/firstHit/float/host` | 88.9 ms | 83.1 ms | -6.5% | [-11.9%, -0.3%] | +0.0% | faster | mesh/view 0.59x (was 0.54x) |
| `MeshTape/view/frame/float/host` | 25.9 ms | 25.9 ms | +0.0% | [-0.2%, +0.2%] | +0.0% | ~ | mesh/view 0.75x |
| `MeshTape/mesh/firstHit/double/host` | 52.3 ms | 52.5 ms | +0.3% | [+0.0%, +0.5%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 52.5 ms | 52.6 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.1 ms | 20.4 ms | +1.5% | [+0.8%, +2.2%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 85.8 ms | 87.1 ms | +1.5% | [+1.1%, +2.0%] | +0.0% | ~ | mesh/tape 0.60x |
| `MeshTape/tape/firstCrossing/double/host` | 54.5 ms | 55.1 ms | +1.2% | [+0.1%, +2.2%] | +0.0% | ~ | mesh/tape 0.95x |
| `MeshTape/tape/frame/double/host` | 26.7 ms | 26.4 ms | -1.2% | [-1.5%, -0.9%] | +0.0% | ~ | mesh/tape 0.77x |
| `MeshTape/view/firstHit/double/host` | 88.7 ms | 90.0 ms | +1.5% | [+0.6%, +2.4%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/double/host` | 29.1 ms | 29.2 ms | +0.5% | [-0.6%, +1.6%] | +0.0% | ~ | mesh/view 0.70x |
| `Render/csg/frame/float/host` | 13.3 ms | 12.5 ms | -6.1% | [-9.7%, -2.4%] | +0.0% | faster |  |
| `Render/csg/frame-orthographic/float/host` | 13.0 ms | 12.5 ms | -3.8% | [-10.4%, +3.4%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 19.7 ms | 19.5 ms | -1.0% | [-1.4%, -0.6%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.0 ms | 14.0 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 13.7 ms | 13.8 ms | +0.8% | [+0.3%, +1.3%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.1 ms | 20.2 ms | +0.2% | [+0.2%, +0.3%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 6 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 726.2 s → 729.1 s (+0.4%), the largest peak 1421 MB → 1421 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.5 s | 9.4 s | -1.2% | 558 MB | 558 MB | +0.0% | ~ |
| `BVHRefit` | 4.2 s | 4.3 s | +0.7% | 501 MB | 501 MB | +0.0% | ~ |
| `BVHUnion` | 18.7 s | 18.7 s | +0.3% | 953 MB | 953 MB | +0.0% | ~ |
| `BVHUnionMixed` | 7.6 s | 7.6 s | +0.1% | 558 MB | 558 MB | -0.0% | ~ |
| `ClosestPrimitive` | 29.3 s | 29.2 s | -0.3% | 1350 MB | 1341 MB | -0.6% | ~ |
| `ClosestSurface` | 27.1 s | 27.1 s | -0.3% | 1312 MB | 1314 MB | +0.1% | ~ |
| `ConnectedComponents` | 15.0 s | 15.1 s | +0.5% | 847 MB | 848 MB | +0.1% | ~ |
| `Domain` | 17.5 s | 17.6 s | +0.4% | 852 MB | 853 MB | +0.2% | ~ |
| `DomainObject` | 15.6 s | 15.4 s | -0.9% | 855 MB | 855 MB | -0.0% | ~ |
| `EBGrid` | 14.0 s | 14.1 s | +0.5% | 782 MB | 783 MB | +0.1% | ~ |
| `Fused` | 23.5 s | 23.7 s | +0.7% | 1221 MB | 1228 MB | +0.6% | ~ |
| `LaunchTuning` | 35.0 s | 35.3 s | +0.6% | 1299 MB | 1325 MB | +2.0% | ~ |
| `Mesh` | 15.1 s | 15.2 s | +0.3% | 789 MB | 789 MB | -0.0% | ~ |
| `MeshCSG` | 37.6 s | 37.9 s | +0.7% | 1276 MB | 1277 MB | +0.1% | ~ |
| `MeshClosestPoint` | 13.6 s | 13.7 s | +0.8% | 784 MB | 784 MB | +0.1% | ~ |
| `MeshIngest` | 6.7 s | 6.7 s | +0.6% | 568 MB | 568 MB | +0.0% | ~ |
| `MeshReconcile` | 6.5 s | 6.5 s | +0.7% | 568 MB | 568 MB | +0.0% | ~ |
| `MeshTape` | 39.4 s | 40.0 s | +1.4% | 1277 MB | 1268 MB | -0.7% | ~ |
| `PointCloud` | 7.0 s | 7.1 s | +1.6% | 561 MB | 561 MB | -0.1% | ~ |
| `PointCloudBVHBuild` | 3.6 s | 3.7 s | +2.2% | 474 MB | 475 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 3.1 s | 3.1 s | +2.2% | 455 MB | 455 MB | +0.0% | ~ |
| `QueryOrder` | 10.7 s | 10.8 s | +0.9% | 700 MB | 700 MB | +0.1% | ~ |
| `Ray` | 34.4 s | 34.7 s | +0.8% | 1315 MB | 1344 MB | +2.2% | ~ |
| `RayCSG` | 30.5 s | 30.7 s | +0.6% | 1212 MB | 1210 MB | -0.1% | ~ |
| `RayCrossing` | 42.8 s | 43.0 s | +0.6% | 1350 MB | 1351 MB | +0.1% | ~ |
| `RayParticles` | 46.1 s | 46.5 s | +0.9% | 1333 MB | 1323 MB | -0.8% | ~ |
| `RayShell` | 35.5 s | 35.4 s | -0.3% | 1282 MB | 1283 MB | +0.0% | ~ |
| `Regions` | 27.4 s | 27.5 s | +0.3% | 1085 MB | 1086 MB | +0.0% | ~ |
| `Render` | 34.6 s | 34.4 s | -0.5% | 1290 MB | 1291 MB | +0.0% | ~ |
| `Shapes` | 8.2 s | 8.2 s | -0.1% | 557 MB | 557 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.5 s | 4.5 s | +0.8% | 515 MB | 516 MB | +0.2% | ~ |
| `SphereCloudRays` | 27.0 s | 27.1 s | +0.4% | 1074 MB | 1074 MB | +0.1% | ~ |
| `SphereContacts` | 4.0 s | 4.1 s | +1.4% | 488 MB | 488 MB | +0.0% | ~ |
| `SurfaceParts` | 40.2 s | 40.2 s | -0.1% | 1421 MB | 1421 MB | -0.0% | ~ |
| `Tape` | 16.3 s | 16.3 s | +0.3% | 831 MB | 830 MB | -0.0% | ~ |
| `TapeBVHUnion` | 14.2 s | 14.3 s | +0.7% | 833 MB | 834 MB | +0.0% | ~ |
| **all, in both** | 726.2 s | 729.1 s | +0.4% | 1421 MB | 1421 MB | -0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (298 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), ConnectedComponents (4), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: df4c34f
base-fingerprint: 5d94b849fdfbdf57bde54ec7585eaa9d2b61a0d6
head: 7e6a0b2
head-fingerprint: a23b462307605142893524cfafdb9453ae008378
cases: 40
rounds: 2
reps: 3
date: 2026-11-22
-->

## Device comparison (CUDA): green

**Accepted:** -

`7e6a0b2` against `df4c34f` (df4c34f), 2026-11-22.

**VERDICT: no regression (0 major, 0 minor across 40 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 40 cases

- base: df4c34f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 7e6a0b2 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.01 ms | 2.00 ms | -0.7% | [-1.3%, -0.1%] | - | ~ | device 5.2x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.37 ms | 3.36 ms | -0.3% | [-0.9%, +0.4%] | - | ~ | device 4.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.16 ms | 6.14 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 4.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.3 ms | 63.5 ms | +0.3% | [-0.0%, +0.7%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 91.9 ms | 91.9 ms | -0.0% | [-2.1%, +2.1%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 243 ms | 243 ms | +0.0% | [-2.3%, +2.5%] | - | ~ | device 3.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.6 ms | 24.8 ms | +0.8% | [-0.3%, +2.0%] | - | ~ | device 2.0x faster than the host; tape/fused 2.56x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.8 ms | 27.0 ms | +0.5% | [-0.5%, +1.6%] | - | ~ | device 2.2x faster than the host; tape/fused 3.40x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 64.9 ms | 65.9 ms | +1.5% | [+0.5%, +2.5%] | - | ~ | device 3.8x faster than the host; tape/fused 3.69x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 21.3 ms | 22.4 ms | +5.3% | [-2.3%, +13.5%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 38.3 ms | 38.5 ms | +0.5% | [-0.7%, +1.6%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 41.3 ms | 41.6 ms | +0.8% | [-0.5%, +2.1%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 533 ms | 534 ms | +0.3% | [-2.1%, +3.5%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 838 ms | 841 ms | +0.3% | [-0.6%, +1.3%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.95 s | 1.94 s | -0.4% | [-2.8%, +1.9%] | - | ~ | device 0.5x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 192 ms | 191 ms | -0.4% | [-1.5%, +0.8%] | - | ~ | device 0.3x faster than the host; tape/fused 2.80x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 299 ms | 301 ms | +0.6% | [-1.7%, +3.3%] | - | ~ | device 0.2x faster than the host; tape/fused 2.80x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 548 ms | 524 ms | -4.4% | [-9.7%, +1.3%] | - | ~ | device 0.5x faster than the host (was 0.5x); tape/fused 3.71x (was 3.56x) |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.29 ms | 1.29 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 15.8x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.29 ms | 1.30 ms | +0.5% | [+0.2%, +0.7%] | - | ~ | device 15.8x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.15 ms | 2.13 ms | -1.1% | [-1.9%, -0.3%] | - | ~ | device 7.9x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.49 ms | 3.50 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 10.2x faster than the host; mesh/tape 0.37x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.75 ms | 2.75 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 7.8x faster than the host; mesh/tape 0.47x |
| `MeshTape/tape/frame/float/hostdevice` | 4.40 ms | 4.40 ms | +0.0% | [-0.3%, +0.4%] | - | ~ | device 4.9x faster than the host; mesh/tape 0.48x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.73 ms | 3.73 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 9.6x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.62 ms | 5.62 ms | +0.0% | [-0.5%, +0.6%] | - | ~ | device 4.2x faster than the host; mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 28.5 ms | 28.5 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 28.4 ms | 28.5 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 24.5 ms | 24.6 ms | +0.5% | [+0.2%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 41.0 ms | 41.0 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 0.9x faster than the host; mesh/tape 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 28.7 ms | 28.7 ms | +0.0% | [-0.2%, +0.3%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 31.4 ms | 31.3 ms | -0.4% | [-1.3%, +0.4%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.79x |
| `MeshTape/view/firstHit/double/hostdevice` | 41.0 ms | 41.0 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 0.9x faster than the host; mesh/view 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 34.1 ms | 34.2 ms | +0.2% | [-0.3%, +0.7%] | - | ~ | device 0.7x faster than the host; mesh/view 0.72x |
| `Render/csg/frame/float/hostdevice` | 2.37 ms | 2.38 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.36 ms | 2.36 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.12 ms | 2.13 ms | +0.2% | [-0.4%, +0.7%] | - | ~ | device 7.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.60 ms | 5.62 ms | +0.4% | [+0.2%, +0.6%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.62 ms | 5.62 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 24.7 ms | 24.5 ms | -0.9% | [-2.1%, +0.3%] | - | ~ | device 0.7x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 2 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4604.8 s → 4601.0 s (-0.1%), the largest peak 6568 MB → 6570 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 11.9 s | 11.9 s | -0.5% | 612 MB | 612 MB | +0.0% | ~ |
| `BVHRefit` | 6.2 s | 6.2 s | -0.5% | 516 MB | 516 MB | +0.1% | ~ |
| `BVHUnion` | 22.4 s | 22.3 s | -0.5% | 1036 MB | 1037 MB | +0.0% | ~ |
| `BVHUnionMixed` | 10.0 s | 9.9 s | -0.5% | 631 MB | 631 MB | +0.1% | ~ |
| `ClosestPrimitive` | 102.0 s | 101.7 s | -0.3% | 1365 MB | 1365 MB | +0.0% | ~ |
| `ClosestSurface` | 143.0 s | 142.8 s | -0.1% | 1844 MB | 1844 MB | -0.0% | ~ |
| `ConnectedComponents` | 18.6 s | 18.5 s | -0.6% | 946 MB | 946 MB | +0.1% | ~ |
| `Domain` | 46.2 s | 46.1 s | -0.2% | 1001 MB | 1001 MB | +0.0% | ~ |
| `DomainObject` | 43.6 s | 43.4 s | -0.3% | 953 MB | 954 MB | +0.1% | ~ |
| `EBGrid` | 17.3 s | 17.4 s | +0.1% | 761 MB | 762 MB | +0.1% | ~ |
| `Fused` | 43.8 s | 43.7 s | -0.3% | 1172 MB | 1173 MB | +0.0% | ~ |
| `LaunchTuning` | 412.6 s | 412.0 s | -0.2% | 3876 MB | 3876 MB | -0.0% | ~ |
| `Mesh` | 18.7 s | 18.7 s | -0.2% | 818 MB | 819 MB | +0.0% | ~ |
| `MeshCSG` | 324.6 s | 324.3 s | -0.1% | 6306 MB | 6308 MB | +0.0% | ~ |
| `MeshClosestPoint` | 16.0 s | 16.0 s | +0.0% | 771 MB | 771 MB | +0.0% | ~ |
| `MeshIngest` | 8.5 s | 8.5 s | +0.0% | 543 MB | 544 MB | +0.1% | ~ |
| `MeshReconcile` | 8.4 s | 8.4 s | +0.0% | 520 MB | 521 MB | +0.1% | ~ |
| `MeshTape` | 616.0 s | 616.9 s | +0.1% | 6568 MB | 6570 MB | +0.0% | ~ |
| `PointCloud` | 10.4 s | 10.4 s | -0.3% | 572 MB | 572 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 5.8 s | 5.7 s | -0.9% | 516 MB | 517 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 5.1 s | 5.0 s | -0.4% | 516 MB | 517 MB | +0.1% | ~ |
| `QueryOrder` | 12.9 s | 12.8 s | -0.2% | 702 MB | 702 MB | +0.0% | ~ |
| `Ray` | 148.5 s | 148.4 s | -0.1% | 3770 MB | 3769 MB | -0.0% | ~ |
| `RayCSG` | 170.5 s | 170.5 s | -0.0% | 4155 MB | 4155 MB | -0.0% | ~ |
| `RayCrossing` | 332.6 s | 332.4 s | -0.1% | 4116 MB | 4116 MB | -0.0% | ~ |
| `RayParticles` | 605.2 s | 605.0 s | -0.0% | 5808 MB | 5807 MB | -0.0% | ~ |
| `RayShell` | 330.6 s | 330.2 s | -0.1% | 5501 MB | 5502 MB | +0.0% | ~ |
| `Regions` | 279.5 s | 278.5 s | -0.3% | 4139 MB | 4138 MB | -0.0% | ~ |
| `Render` | 203.0 s | 203.0 s | +0.0% | 6083 MB | 6079 MB | -0.1% | ~ |
| `Shapes` | 18.7 s | 19.5 s | +4.2% | 593 MB | 594 MB | +0.1% | ~ |
| `SphereCloudBVH` | 7.2 s | 6.9 s | -3.1% | 516 MB | 517 MB | +0.2% | ~ |
| `SphereCloudRays` | 144.2 s | 143.9 s | -0.2% | 4170 MB | 4170 MB | +0.0% | ~ |
| `SphereContacts` | 6.7 s | 6.7 s | -0.8% | 516 MB | 516 MB | +0.1% | ~ |
| `SurfaceParts` | 368.6 s | 368.5 s | -0.0% | 4416 MB | 4416 MB | -0.0% | ~ |
| `Tape` | 50.2 s | 50.0 s | -0.4% | 987 MB | 986 MB | -0.1% | ~ |
| `TapeBVHUnion` | 35.0 s | 34.9 s | -0.5% | 927 MB | 927 MB | +0.0% | ~ |
| **all, in both** | 4604.8 s | 4601.0 s | -0.1% | 6568 MB | 6570 MB | +0.0% | ~ |

Device kernels: 226 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

The head's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): MeshCSG/smooth-tape/firstHit-morton/double/hostdevice, MeshCSG/smooth-fused/firstHit-random/double/hostdevice

These probes compiled to the same code on both sides and were not run (374 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), EBGrid (12), Fused (10), LaunchTuning (24), Mesh (8), MeshClosestPoint (4), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), QueryOrder (6), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
