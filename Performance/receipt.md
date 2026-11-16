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
base: b20851b
base-fingerprint: 1c0f10afe259b64b8e299f247428e810a3b3b6eb
head: ae0a14f
head-fingerprint: 8c75bde3d1f96257d466b8e7cb8c56164704950f
cases: 18
rounds: 2
reps: 3
date: 2026-11-16
-->

## Host comparison: yellow

**Accepted:** Side effect of rendering.

`ae0a14f` against `b20851b` (origin/main), 2026-11-16.

**VERDICT: 1 minor regression; look at it before accepting (see Flagged above)**

Performance comparison: 0 major, 1 minor, 4 faster, 0 differ, 0 lost, 18 cases

- base: b20851b (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: ae0a14f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit/float/host` | 26.7 ms | 26.7 ms | -0.3% | [-1.4%, +0.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 27.0 ms | 26.7 ms | -1.2% | [-2.2%, -0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 317 ms | -0.7% | [-1.6%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 237 ms | 237 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 90.2 ms | 90.4 ms | +0.3% | [-0.0%, +0.6%] | +0.0% | ~ | tape/fused 3.51x |
| `MeshCSG/smooth-fused/frame/float/host` | 101 ms | 101 ms | -0.3% | [-0.6%, -0.0%] | +0.0% | ~ | tape/fused 2.35x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.4 ms | 29.2 ms | -0.7% | [-0.9%, -0.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.0 ms | 29.7 ms | -1.0% | [-1.5%, -0.5%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 353 ms | 352 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 253 ms | 253 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 97.3 ms | 97.8 ms | +0.5% | [-0.3%, +1.3%] | +0.0% | ~ | tape/fused 3.60x |
| `MeshCSG/smooth-fused/frame/double/host` | 109 ms | 109 ms | -0.4% | [-0.8%, -0.0%] | +0.0% | ~ | tape/fused 2.34x |
| `Render/csg/frame/float/host` | 20.9 ms | 19.0 ms | -9.3% | [-11.0%, -7.5%] | +0.0% | faster |  |
| `Render/csg/frame-orthographic/float/host` | 21.0 ms | 19.4 ms | -7.6% | [-10.5%, -4.8%] | +0.0% | faster |  |
| `Render/mesh/frame/float/host` | 18.4 ms | 19.3 ms | +4.8% | [+3.3%, +5.4%] | +0.0% | minor |  |
| `Render/csg/frame/double/host` | 22.7 ms | 21.5 ms | -4.9% | [-6.0%, -3.8%] | +0.0% | faster |  |
| `Render/csg/frame-orthographic/double/host` | 22.5 ms | 21.6 ms | -4.2% | [-5.1%, -3.3%] | +0.0% | faster |  |
| `Render/mesh/frame/double/host` | 20.7 ms | 20.7 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 612.8 s → 625.9 s (+2.1%), the largest peak 1398 MB → 1398 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.1 s | +0.1% | 555 MB | 556 MB | +0.1% | ~ |
| `BVHRefit` | 3.9 s | 3.9 s | -0.6% | 482 MB | 483 MB | +0.1% | ~ |
| `BVHUnion` | 17.9 s | 18.4 s | +2.5% | 894 MB | 895 MB | +0.1% | ~ |
| `BVHUnionMixed` | 7.3 s | 7.3 s | -0.1% | 555 MB | 556 MB | +0.1% | ~ |
| `ClosestPrimitive` | 28.9 s | 29.4 s | +1.7% | 1262 MB | 1263 MB | +0.1% | ~ |
| `ClosestSurface` | 27.0 s | 27.7 s | +2.7% | 1219 MB | 1219 MB | -0.0% | ~ |
| `Domain` | 17.5 s | 17.8 s | +1.8% | 839 MB | 839 MB | +0.0% | ~ |
| `Fused` | 23.4 s | 23.9 s | +2.5% | 1125 MB | 1126 MB | +0.1% | ~ |
| `LaunchTuning` | 34.9 s | 36.0 s | +3.3% | 1268 MB | 1268 MB | -0.1% | ~ |
| `Mesh` | 14.3 s | 16.0 s | +11.5% | 785 MB | 784 MB | -0.2% | ~ |
| `MeshCSG` | 37.5 s | 38.8 s | +3.2% | 1282 MB | 1281 MB | -0.1% | ~ |
| `MeshReconcile` | 6.4 s | 6.7 s | +5.6% | 565 MB | 565 MB | +0.0% | ~ |
| `PointCloud` | 7.2 s | 7.3 s | +1.9% | 558 MB | 558 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.6 s | 3.8 s | +5.9% | 469 MB | 470 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.1 s | 3.3 s | +6.4% | 451 MB | 451 MB | +0.0% | ~ |
| `Ray` | 34.8 s | 35.5 s | +2.0% | 1317 MB | 1317 MB | +0.0% | ~ |
| `RayCSG` | 30.8 s | 31.8 s | +3.3% | 1113 MB | 1113 MB | +0.0% | ~ |
| `RayCrossing` | 43.5 s | 44.5 s | +2.2% | 1341 MB | 1341 MB | -0.0% | ~ |
| `RayParticles` | 46.8 s | 47.7 s | +1.9% | 1314 MB | 1326 MB | +0.8% | ~ |
| `RayShell` | 35.9 s | 36.6 s | +2.0% | 1270 MB | 1271 MB | +0.1% | ~ |
| `Regions` | 27.8 s | 28.9 s | +3.9% | 1066 MB | 1069 MB | +0.2% | ~ |
| `Render` | 34.7 s | 35.7 s | +2.8% | 1262 MB | 1267 MB | +0.4% | ~ |
| `Shapes` | 8.5 s | 8.8 s | +3.3% | 553 MB | 553 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.8 s | -0.8% | 510 MB | 510 MB | +0.0% | ~ |
| `SphereCloudRays` | 27.6 s | 27.2 s | -1.5% | 1058 MB | 1059 MB | +0.0% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | +0.1% | 483 MB | 484 MB | +0.0% | ~ |
| `SurfaceParts` | 40.0 s | 39.4 s | -1.5% | 1398 MB | 1398 MB | +0.0% | ~ |
| `Tape` | 16.6 s | 16.6 s | -0.1% | 817 MB | 818 MB | +0.1% | ~ |
| `TapeBVHUnion` | 14.7 s | 14.8 s | +0.7% | 821 MB | 822 MB | +0.1% | ~ |
| **all, in both** | 612.8 s | 625.9 s | +2.1% | 1398 MB | 1398 MB | +0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (1 cases): Render/mesh/frame/float/host

These probes compiled to the same code on both sides and were not run (252 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), Mesh (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: b20851b
base-fingerprint: 1c0f10afe259b64b8e299f247428e810a3b3b6eb
head: ae0a14f
head-fingerprint: 8c75bde3d1f96257d466b8e7cb8c56164704950f
cases: 24
rounds: 2
reps: 3
date: 2026-11-16
-->

## Device comparison (CUDA): green

**Accepted:** -

`ae0a14f` against `b20851b` (origin/main), 2026-11-16.

**VERDICT: no regression (0 major, 0 minor across 24 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 24 cases

- base: b20851b (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: ae0a14f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.52 ms | 2.54 ms | +0.8% | [+0.3%, +1.2%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.28 ms | 4.27 ms | -0.0% | [-0.8%, +0.7%] | - | ~ | device 3.3x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.8 ms | 14.0 ms | +1.2% | [+1.1%, +1.2%] | - | ~ | device 3.9x faster than the host (was 5.1x) |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.7 ms | 62.9 ms | -1.2% | [-2.1%, -0.2%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 94.1 ms | 91.7 ms | -2.6% | [-4.1%, -1.0%] | - | ~ | device 2.7x faster than the host (was 2.6x) |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 251 ms | 250 ms | -0.5% | [-1.6%, +0.5%] | - | ~ | device 3.3x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 24.6 ms | 24.9 ms | +1.3% | [+0.3%, +2.2%] | - | ~ | device 2.0x faster than the host; tape/fused 2.53x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.8 ms | 26.9 ms | +0.5% | [+0.3%, +0.8%] | - | ~ | device 2.2x faster than the host; tape/fused 3.41x (was 3.52x) |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 73.2 ms | 73.5 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 4.4x faster than the host (was 4.1x); tape/fused 3.40x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.3 ms | 22.8 ms | +2.1% | [-3.5%, +6.3%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.5 ms | 43.9 ms | -1.3% | [-3.0%, +0.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 94.5 ms | 93.8 ms | -0.8% | [-4.0%, +2.7%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 523 ms | 538 ms | +2.9% | [-0.9%, +7.3%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 856 ms | 837 ms | -2.2% | [-3.6%, -0.8%] | - | ~ | device 0.3x faster than the host (was 0.4x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.05 s | 2.04 s | -0.5% | [-2.2%, +1.3%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 195 ms | 201 ms | +2.8% | [-2.8%, +8.8%] | - | ~ | device 0.2x faster than the host; tape/fused 2.68x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 302 ms | 307 ms | +1.6% | [-1.5%, +4.9%] | - | ~ | device 0.2x faster than the host; tape/fused 2.73x (was 2.84x) |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 576 ms | 582 ms | +1.0% | [-0.2%, +2.3%] | - | ~ | device 0.5x faster than the host; tape/fused 3.50x |
| `Render/csg/frame/float/hostdevice` | 4.14 ms | 4.24 ms | +2.3% | [+2.3%, +2.4%] | - | ~ | device 2.7x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.15 ms | 4.25 ms | +2.4% | [+2.3%, +2.5%] | - | ~ | device 2.7x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.00 ms | 2.05 ms | +2.6% | [+1.4%, +3.7%] | - | ~ | device 8.2x faster than the host (was 8.4x) |
| `Render/csg/frame/double/hostdevice` | 7.72 ms | 7.88 ms | +2.1% | [+1.8%, +2.3%] | - | ~ | device 1.6x faster than the host (was 1.6x) |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.73 ms | 7.88 ms | +1.9% | [+1.7%, +2.0%] | - | ~ | device 1.6x faster than the host (was 1.6x) |
| `Render/mesh/frame/double/hostdevice` | 23.4 ms | 23.4 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 0.7x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 4785.1 s → 4750.7 s (-0.7%), the largest peak 5632 MB → 6045 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.0 s | 12.8 s | -1.3% | 598 MB | 599 MB | +0.1% | ~ |
| `BVHRefit` | 6.6 s | 6.6 s | -0.2% | 504 MB | 504 MB | +0.2% | ~ |
| `BVHUnion` | 24.6 s | 24.7 s | +0.4% | 1023 MB | 1023 MB | +0.0% | ~ |
| `BVHUnionMixed` | 10.9 s | 10.7 s | -1.2% | 624 MB | 624 MB | +0.1% | ~ |
| `ClosestPrimitive` | 129.2 s | 128.2 s | -0.7% | 1346 MB | 1346 MB | +0.0% | ~ |
| `ClosestSurface` | 194.4 s | 192.4 s | -1.0% | 1844 MB | 1843 MB | -0.1% | ~ |
| `Domain` | 59.6 s | 58.5 s | -1.8% | 992 MB | 992 MB | -0.0% | ~ |
| `Fused` | 52.9 s | 52.4 s | -1.0% | 1157 MB | 1157 MB | +0.0% | ~ |
| `LaunchTuning` | 541.7 s | 538.4 s | -0.6% | 3806 MB | 3806 MB | -0.0% | ~ |
| `Mesh` | 20.1 s | 20.1 s | +0.0% | 800 MB | 801 MB | +0.1% | ~ |
| `MeshCSG` | 384.4 s | 398.1 s | +3.6% | 5550 MB | 6045 MB | +8.9% | ~ |
| `MeshReconcile` | 10.0 s | 10.3 s | +3.3% | 511 MB | 511 MB | +0.1% | ~ |
| `PointCloud` | 12.3 s | 12.3 s | -0.1% | 569 MB | 570 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 7.3 s | 7.2 s | -1.5% | 504 MB | 505 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 6.3 s | 6.3 s | +1.3% | 504 MB | 505 MB | +0.1% | ~ |
| `Ray` | 191.5 s | 187.4 s | -2.2% | 3703 MB | 3703 MB | +0.0% | ~ |
| `RayCSG` | 224.8 s | 221.2 s | -1.6% | 4086 MB | 4086 MB | +0.0% | ~ |
| `RayCrossing` | 439.4 s | 435.9 s | -0.8% | 4017 MB | 4017 MB | +0.0% | ~ |
| `RayParticles` | 688.4 s | 675.1 s | -1.9% | 5632 MB | 5632 MB | -0.0% | ~ |
| `RayShell` | 417.3 s | 408.1 s | -2.2% | 5346 MB | 5346 MB | -0.0% | ~ |
| `Regions` | 361.4 s | 357.5 s | -1.1% | 3835 MB | 3835 MB | -0.0% | ~ |
| `Render` | 237.4 s | 254.3 s | +7.1% | 5386 MB | 5791 MB | +7.5% | ~ |
| `Shapes` | 25.4 s | 25.8 s | +1.6% | 590 MB | 591 MB | +0.1% | ~ |
| `SphereCloudBVH` | 8.5 s | 8.5 s | +0.8% | 504 MB | 505 MB | +0.1% | ~ |
| `SphereCloudRays` | 190.7 s | 188.7 s | -1.0% | 4093 MB | 4093 MB | -0.0% | ~ |
| `SphereContacts` | 8.4 s | 8.2 s | -2.6% | 504 MB | 504 MB | +0.2% | ~ |
| `SurfaceParts` | 412.9 s | 399.6 s | -3.2% | 4242 MB | 4242 MB | -0.0% | ~ |
| `Tape` | 64.0 s | 59.3 s | -7.4% | 980 MB | 980 MB | +0.0% | ~ |
| `TapeBVHUnion` | 41.9 s | 42.0 s | +0.2% | 920 MB | 921 MB | +0.0% | ~ |
| **all, in both** | 4785.1 s | 4750.7 s | -0.7% | 5632 MB | 6045 MB | +7.3% | ~ |

Device kernels: 194 in both, 8 changed (1 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, TriMeshSDF<float, 4ul, 4ul> >, unsig…` | 168 → 168 | 1200 → 1392 | 36/36 → 96/100 | **spills** |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Fused::Node<Fused::SmoothCombineOp<…` | 168 → 168 | 5040 → 5232 | 1116/976 → 1100/896 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19440 → 19760 | 1456/1428 → 1388/1348 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Fused::Node<Fused::SmoothCombineOp<f…` | 168 → 168 | 2992 → 3184 | 376/500 → 348/428 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float, TriMeshSDF<float, 4ul, 4…` | 168 → 168 | 13424 → 13488 | 1296/1108 → 1292/1072 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 19120 → 19312 | 1456/1428 → 1388/1348 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, TriMeshSDF<double, 4ul, 4ul> >, uns…` | 168 → 168 | 1904 → 2096 | 808/808 → 808/768 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float> >, unsigned int const, f…` | 168 → 168 | 13616 → 13744 | 1296/1108 → 1292/1072 | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (2 cases): MeshCSG/union-tape/firstHit-morton/double/hostdevice, MeshCSG/smooth-tape/firstHit-morton/double/hostdevice

These probes compiled to the same code on both sides and were not run (344 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), Fused (10), LaunchTuning (24), Mesh (8), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Ray (28), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt cuda -->
