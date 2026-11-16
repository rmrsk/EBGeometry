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
base: dc46970
base-fingerprint: 2ebd9bed29e3a6ee9df443c1efe9a3bd39833f40
head: be2cfbb
head-fingerprint: 30b6edfbe4d67af90dcaf2ebe131bc7df88464b1
cases: 18
rounds: 2
reps: 3
date: 2026-11-16
-->

## Host comparison: yellow

**Accepted:** the default render changes on purpose: the fill light and the shoulder change every frame's pixels; the minor slowdown of the frame cases is the cost of the added shading.

`be2cfbb` against `dc46970` (origin/main), 2026-11-16.

**VERDICT: 4 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 4 minor, 0 faster, 0 differ, 0 lost, 18 cases

- base: dc46970 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: be2cfbb (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: AMD Ryzen 9 5900X 12-Core Processor
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 29.2 ms | 29.3 ms | +0.4% | [-0.0%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 36.0 ms | 36.6 ms | +1.7% | [+1.4%, +1.9%] | +0.0% | ~ | checksum differs (13258.03817 -> 13446.26047) |
| `MeshCSG/smooth-tape/firstHit/float/host` | 364 ms | 363 ms | -0.1% | [-0.7%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 279 ms | 279 ms | -0.2% | [-0.6%, +0.2%] | +0.0% | ~ | checksum differs (13367.93495 -> 13533.37743) |
| `MeshCSG/smooth-fused/firstHit/float/host` | 100 ms | 100 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ | tape/fused 3.62x |
| `MeshCSG/smooth-fused/frame/float/host` | 115 ms | 118 ms | +2.1% | [-0.4%, +4.6%] | +0.0% | ~ | tape/fused 2.37x; checksum differs (13369.81935 -> 13534.36873) |
| `MeshCSG/union-tape/firstHit/double/host` | 31.8 ms | 31.6 ms | -0.6% | [-1.5%, +0.4%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 39.6 ms | 39.7 ms | +0.2% | [-0.6%, +1.0%] | +0.0% | ~ | checksum differs (13258.0063 -> 13446.23666) |
| `MeshCSG/smooth-tape/firstHit/double/host` | 400 ms | 400 ms | +0.1% | [+0.1%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 302 ms | 302 ms | +0.3% | [+0.2%, +0.4%] | +0.0% | ~ | checksum differs (13367.9308 -> 13533.37728) |
| `MeshCSG/smooth-fused/firstHit/double/host` | 109 ms | 109 ms | +0.3% | [-0.3%, +0.9%] | +0.0% | ~ | tape/fused 3.66x |
| `MeshCSG/smooth-fused/frame/double/host` | 129 ms | 130 ms | +0.2% | [+0.1%, +0.2%] | +0.0% | ~ | tape/fused 2.33x; checksum differs (13369.81911 -> 13534.37633) |
| `Render/csg/frame/float/host` | 29.8 ms | 29.4 ms | -1.4% | [-2.0%, -0.7%] | +0.0% | ~ | checksum differs (27718.0892 -> 28892.12834) |
| `Render/csg/frame-orthographic/float/host` | 29.5 ms | 29.2 ms | -1.1% | [-1.7%, -0.5%] | +0.0% | ~ | checksum differs (27409.89965 -> 28633.18579) |
| `Render/mesh/frame/float/host` | 22.0 ms | 22.8 ms | +3.5% | [+2.6%, +3.8%] | +0.0% | minor | checksum differs (30139.52904 -> 30644.34942) |
| `Render/csg/frame/double/host` | 32.1 ms | 33.9 ms | +5.4% | [+4.9%, +5.8%] | +0.0% | minor | checksum differs (27718.0849 -> 28892.12791) |
| `Render/csg/frame-orthographic/double/host` | 32.3 ms | 33.9 ms | +5.0% | [+3.9%, +5.9%] | +0.0% | minor | checksum differs (27409.89926 -> 28633.18537) |
| `Render/mesh/frame/double/host` | 24.2 ms | 25.1 ms | +3.7% | [+3.3%, +4.1%] | +0.0% | minor | checksum differs (30139.62606 -> 30644.63076) |

### Build costs: Probe builds (Release)

Machine: 12 cores, 24 hardware threads, 31 GB. Compiled on hardware threads 0,1,2,3,4,5,6,7,8,9,10,11 (one per core), 12 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 751.3 s → 734.6 s (-2.2%), the largest peak 1398 MB → 1399 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 11.7 s | 11.4 s | -2.6% | 560 MB | 560 MB | -0.0% | ~ |
| `BVHRefit` | 5.3 s | 5.2 s | -1.6% | 488 MB | 487 MB | -0.1% | ~ |
| `BVHUnion` | 22.5 s | 21.9 s | -2.8% | 900 MB | 900 MB | +0.1% | ~ |
| `BVHUnionMixed` | 9.5 s | 9.2 s | -2.3% | 560 MB | 560 MB | -0.0% | ~ |
| `ClosestPrimitive` | 36.9 s | 35.8 s | -2.9% | 1268 MB | 1267 MB | -0.1% | ~ |
| `ClosestSurface` | 34.4 s | 33.3 s | -3.0% | 1224 MB | 1224 MB | +0.0% | ~ |
| `Domain` | 22.8 s | 21.8 s | -4.6% | 844 MB | 844 MB | +0.0% | ~ |
| `Fused` | 30.0 s | 29.2 s | -2.7% | 1127 MB | 1128 MB | +0.1% | ~ |
| `LaunchTuning` | 42.9 s | 42.0 s | -2.1% | 1271 MB | 1271 MB | +0.0% | ~ |
| `Mesh` | 18.2 s | 18.1 s | -0.8% | 789 MB | 789 MB | -0.0% | ~ |
| `MeshCSG` | 46.2 s | 45.0 s | -2.6% | 1287 MB | 1288 MB | +0.1% | ~ |
| `MeshReconcile` | 8.2 s | 8.1 s | -0.3% | 569 MB | 568 MB | -0.1% | ~ |
| `PointCloud` | 9.1 s | 9.0 s | -1.1% | 563 MB | 563 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 4.8 s | 4.7 s | -1.8% | 473 MB | 473 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 4.3 s | 4.1 s | -5.3% | 456 MB | 456 MB | -0.0% | ~ |
| `Ray` | 42.3 s | 41.1 s | -2.7% | 1320 MB | 1320 MB | +0.0% | ~ |
| `RayCSG` | 38.1 s | 36.8 s | -3.5% | 1116 MB | 1116 MB | -0.0% | ~ |
| `RayCrossing` | 52.0 s | 50.6 s | -2.7% | 1344 MB | 1345 MB | +0.1% | ~ |
| `RayParticles` | 55.3 s | 53.5 s | -3.2% | 1330 MB | 1329 MB | -0.1% | ~ |
| `RayShell` | 42.4 s | 41.7 s | -1.8% | 1274 MB | 1274 MB | +0.0% | ~ |
| `Regions` | 33.5 s | 32.8 s | -2.2% | 1071 MB | 1072 MB | +0.0% | ~ |
| `Render` | 41.1 s | 41.2 s | +0.4% | 1271 MB | 1271 MB | +0.0% | ~ |
| `Shapes` | 10.6 s | 10.5 s | -1.1% | 558 MB | 558 MB | -0.0% | ~ |
| `SphereCloudBVH` | 5.8 s | 5.8 s | -0.2% | 513 MB | 513 MB | +0.0% | ~ |
| `SphereCloudRays` | 32.8 s | 32.3 s | -1.3% | 1062 MB | 1063 MB | +0.1% | ~ |
| `SphereContacts` | 5.2 s | 5.2 s | -0.6% | 487 MB | 487 MB | -0.0% | ~ |
| `SurfaceParts` | 47.7 s | 47.0 s | -1.4% | 1398 MB | 1399 MB | +0.1% | ~ |
| `Tape` | 20.1 s | 20.0 s | -0.7% | 822 MB | 822 MB | +0.1% | ~ |
| `TapeBVHUnion` | 17.8 s | 17.3 s | -2.6% | 826 MB | 826 MB | +0.0% | ~ |
| **all, in both** | 751.3 s | 734.6 s | -2.2% | 1398 MB | 1399 MB | +0.1% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (4 cases): Render/mesh/frame/float/host, Render/csg/frame/double/host, Render/csg/frame-orthographic/double/host, Render/mesh/frame/double/host

These probes compiled to the same code on both sides and were not run (252 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: red
base: dc46970
base-fingerprint: 2ebd9bed29e3a6ee9df443c1efe9a3bd39833f40
head: be2cfbb
head-fingerprint: 30b6edfbe4d67af90dcaf2ebe131bc7df88464b1
cases: 24
rounds: 4
reps: 3
date: 2026-11-16
-->

## Device comparison (CUDA): red

**Accepted:** the default render changes on purpose: the fill light and the shoulder change every frame's pixels; the one pixel of Render/mesh/frame/float that differs is an existing host/device normal difference at a mesh edge (20 levels without the fill), which the fill raises over the probe's limit (#343).

`be2cfbb` against `dc46970` (origin/main), 2026-11-16.

**VERDICT: host and device answers differ in 1 case; investigate before merging (see Flagged above)**

Performance comparison: 0 major, 0 minor, 0 faster, 1 differ, 0 lost, 24 cases

- base: dc46970 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: be2cfbb (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: AMD Ryzen 9 5900X 12-Core Processor
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 1.82 ms | 1.84 ms | +1.2% | [+0.1%, +1.9%] | - | ~ | device 5.8x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 2.67 ms | 2.68 ms | +0.5% | [-0.7%, +1.2%] | - | ~ | device 5.2x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 9.10 ms | 9.04 ms | -0.6% | [-1.3%, -0.1%] | - | ~ | device 7.1x faster than the host; checksum differs (212135.6679 -> 215184.54) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 42.7 ms | 43.4 ms | +1.7% | [-1.3%, +3.2%] | - | ~ | device 4.8x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 57.3 ms | 57.4 ms | +0.2% | [-1.0%, +1.4%] | - | ~ | device 3.9x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 173 ms | 171 ms | -1.1% | [-3.0%, +2.0%] | - | ~ | device 5.0x faster than the host; checksum differs (213917.8326 -> 216561.4797) |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 21.4 ms | 21.2 ms | -0.6% | [-1.0%, +0.1%] | - | ~ | device 2.4x faster than the host; tape/fused 2.05x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 18.8 ms | 18.5 ms | -1.7% | [-2.9%, +0.1%] | - | ~ | device 3.2x faster than the host; tape/fused 3.11x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 59.2 ms | 58.6 ms | -0.9% | [-2.5%, +1.5%] | - | ~ | device 5.4x faster than the host; tape/fused 2.92x; checksum differs (213951.0518 -> 216584.3999) |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 16.2 ms | 16.2 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 26.2 ms | 26.0 ms | -0.6% | [-2.9%, +0.8%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 60.9 ms | 62.8 ms | +3.0% | [-2.8%, +9.3%] | - | ~ | device 1.1x faster than the host; checksum differs (212133.3948 -> 215182.686) |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 421 ms | 421 ms | -0.0% | [-3.7%, +4.3%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 531 ms | 534 ms | +0.6% | [-0.6%, +2.6%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.32 s | 1.34 s | +0.9% | [-5.3%, +4.3%] | - | ~ | device 0.7x faster than the host; checksum differs (213917.701 -> 216561.3753) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 163 ms | 164 ms | +0.6% | [-2.1%, +3.9%] | - | ~ | device 0.3x faster than the host; tape/fused 2.57x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 214 ms | 210 ms | -1.6% | [-4.1%, -0.0%] | - | ~ | device 0.3x faster than the host; tape/fused 2.54x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 498 ms | 474 ms | -4.8% | [-11.8%, +4.3%] | - | ~ | device 0.7x faster than the host (was 0.7x); tape/fused 2.82x (was 2.66x); checksum differs (213950.8768 -> 216584.2909) |
| `Render/csg/frame/float/hostdevice` | 2.27 ms | 2.28 ms | +0.7% | [+0.5%, +1.0%] | - | ~ | device 6.7x faster than the host; checksum differs (197096.378 -> 205452.818) |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.27 ms | 2.28 ms | +0.6% | [+0.5%, +0.9%] | - | ~ | device 6.7x faster than the host; checksum differs (194925.7287 -> 203610.8902) |
| `Render/mesh/frame/float/hostdevice` | 1.26 ms | 1.25 ms | -0.8% | [-11.5%, +1.0%] | - | **DIFFER** | device 13.9x faster than the host; 1 host and device answers DIFFER (max diff 0.11); checksum differs (214324.6688 -> 217922.3399) |
| `Render/csg/frame/double/hostdevice` | 4.26 ms | 4.33 ms | +1.6% | [+1.4%, +2.0%] | - | ~ | device 3.9x faster than the host; checksum differs (197096.379 -> 205452.8189) |
| `Render/csg/frame-orthographic/double/hostdevice` | 4.26 ms | 4.31 ms | +1.4% | [+1.1%, +1.6%] | - | ~ | device 3.9x faster than the host; checksum differs (194925.718 -> 203610.887) |
| `Render/mesh/frame/double/hostdevice` | 13.4 ms | 13.4 ms | +0.1% | [-1.1%, +3.5%] | - | ~ | device 1.4x faster than the host; checksum differs (214324.674 -> 217922.1769) |

### Build costs: Probe builds (Release)

Machine: 12 cores, 24 hardware threads, 31 GB. Compiled on hardware threads 0,1,2,3,4,5,6,7,8,9,10,11 (one per core), 5 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 5919.8 s → 5836.9 s (-1.4%), the largest peak 6043 MB → 6079 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 15.6 s | 15.4 s | -1.0% | 598 MB | 599 MB | +0.2% | ~ |
| `BVHRefit` | 8.5 s | 8.4 s | -1.9% | 504 MB | 503 MB | -0.0% | ~ |
| `BVHUnion` | 28.5 s | 28.2 s | -1.2% | 1023 MB | 1023 MB | -0.0% | ~ |
| `BVHUnionMixed` | 13.5 s | 13.2 s | -1.7% | 625 MB | 624 MB | -0.2% | ~ |
| `ClosestPrimitive` | 154.7 s | 153.2 s | -0.9% | 1346 MB | 1345 MB | -0.0% | ~ |
| `ClosestSurface` | 227.2 s | 225.7 s | -0.7% | 1842 MB | 1843 MB | +0.1% | ~ |
| `Domain` | 72.8 s | 71.8 s | -1.4% | 993 MB | 993 MB | -0.1% | ~ |
| `Fused` | 65.4 s | 64.7 s | -1.1% | 1156 MB | 1156 MB | +0.0% | ~ |
| `LaunchTuning` | 671.8 s | 666.7 s | -0.8% | 3805 MB | 3805 MB | +0.0% | ~ |
| `Mesh` | 25.3 s | 25.1 s | -0.8% | 800 MB | 800 MB | +0.1% | ~ |
| `MeshCSG` | 522.7 s | 520.9 s | -0.4% | 6043 MB | 6079 MB | +0.6% | ~ |
| `MeshReconcile` | 13.1 s | 12.8 s | -3.0% | 510 MB | 510 MB | -0.0% | ~ |
| `PointCloud` | 14.6 s | 14.4 s | -1.0% | 569 MB | 569 MB | -0.0% | ~ |
| `PointCloudBVHBuild` | 9.1 s | 8.9 s | -2.0% | 504 MB | 505 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 8.3 s | 8.2 s | -1.7% | 504 MB | 504 MB | +0.1% | ~ |
| `Ray` | 238.9 s | 239.3 s | +0.2% | 3702 MB | 3702 MB | +0.0% | ~ |
| `RayCSG` | 281.7 s | 280.9 s | -0.3% | 4084 MB | 4084 MB | -0.0% | ~ |
| `RayCrossing` | 542.1 s | 533.2 s | -1.6% | 4016 MB | 4017 MB | +0.0% | ~ |
| `RayParticles` | 830.4 s | 816.3 s | -1.7% | 5631 MB | 5631 MB | +0.0% | ~ |
| `RayShell` | 482.8 s | 490.7 s | +1.6% | 5345 MB | 5345 MB | +0.0% | ~ |
| `Regions` | 446.6 s | 433.8 s | -2.9% | 3833 MB | 3834 MB | +0.0% | ~ |
| `Render` | 319.6 s | 315.8 s | -1.2% | 5790 MB | 5896 MB | +1.8% | ~ |
| `Shapes` | 36.7 s | 35.9 s | -2.3% | 591 MB | 590 MB | -0.1% | ~ |
| `SphereCloudBVH` | 11.2 s | 10.8 s | -3.2% | 504 MB | 504 MB | +0.1% | ~ |
| `SphereCloudRays` | 226.8 s | 221.1 s | -2.5% | 4092 MB | 4092 MB | +0.0% | ~ |
| `SphereContacts` | 10.8 s | 10.2 s | -5.1% | 504 MB | 504 MB | +0.1% | ~ |
| `SurfaceParts` | 509.8 s | 483.2 s | -5.2% | 4240 MB | 4241 MB | +0.0% | ~ |
| `Tape` | 78.9 s | 76.8 s | -2.7% | 979 MB | 980 MB | +0.1% | ~ |
| `TapeBVHUnion` | 52.6 s | 51.4 s | -2.4% | 920 MB | 920 MB | +0.1% | ~ |
| **all, in both** | 5919.8 s | 5836.9 s | -1.4% | 6043 MB | 6079 MB | +0.6% | ~ |

Device kernels: 194 in both, 7 changed (7 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Fused::Node<Fused::SmoothCombineOp<…` | 168 → 168 | 5232 → 5232 | 1100/896 → 1100/920 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19760 → 19760 | 1388/1348 → 1388/1372 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Fused::Node<Fused::SmoothCombineOp<f…` | 168 → 168 | 3184 → 3184 | 348/428 → 424/448 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float, TriMeshSDF<float, 4ul, 4…` | 168 → 168 | 13488 → 13488 | 1292/1072 → 1292/1084 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 19312 → 19376 | 1388/1348 → 1388/1372 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, TriMeshSDF<double, 4ul, 4ul> >, uns…` | 168 → 168 | 2096 → 2096 | 808/768 → 808/792 | **spills** |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float> >, unsigned int const, f…` | 168 → 168 | 13744 → 13744 | 1292/1072 → 1292/1084 | **spills** |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (344 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), LaunchTuning (24), Mesh (8), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
