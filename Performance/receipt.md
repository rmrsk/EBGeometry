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
base: eb1d2c4
base-fingerprint: 3adc6f11e3922774f885720dc4cd9db57d17a9fe
head: 8bc806f
head-fingerprint: 5b762623a2468955cf1da5e76311c603f57f1727
cases: 34
rounds: 2
reps: 3
date: 2026-11-17
-->

## Host comparison: green

**Accepted:** -

`8bc806f` against `eb1d2c4` (origin/main), 2026-11-17.

**VERDICT: no regression (0 major, 0 minor across 34 cases)**

Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 34 cases

- base: eb1d2c4 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8bc806f (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `TriMeshSDF/w4/build/float/host` | 83.8 ms | 83.4 ms | -0.4% | [-1.3%, +0.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 230 ms | 230 ms | -0.3% | [-0.9%, +0.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 115 ms | 114 ms | -0.5% | [-0.8%, -0.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 260 ms | 261 ms | +0.2% | [-0.4%, +0.8%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 71.4 ms | 71.5 ms | +0.2% | [+0.0%, +0.4%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 214 ms | 214 ms | -0.3% | [-0.8%, +0.3%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 97.9 ms | 97.6 ms | -0.3% | [-0.8%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 30.9 ms | 29.1 ms | -6.0% | [-11.7%, +0.5%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 766 ms | 763 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 119 ms | 119 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 44.4 ms | 44.4 ms | -0.1% | [-2.5%, +2.4%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 883 ms | 866 ms | -2.0% | [-3.9%, -0.0%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/signedDistance/float/host` | - | - | - | - | - | new |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | - | - | - | - | - | new | closestPoint/signedDistance 0.99x |
| `MeshClosestPoint/mesh/closestPoint/float/host` | - | - | - | - | - | new |  |
| `MeshClosestPoint/tri/gradient/float/host` | - | - | - | - | - | new | gradient/closestPoint 1.00x |
| `MeshClosestPoint/tri/signedDistance/double/host` | - | - | - | - | - | new |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | - | - | - | - | - | new | closestPoint/signedDistance 1.00x |
| `MeshClosestPoint/mesh/closestPoint/double/host` | - | - | - | - | - | new |  |
| `MeshClosestPoint/tri/gradient/double/host` | - | - | - | - | - | new | gradient/closestPoint 1.00x |
| `Ray/mesh/firstHit/float/host` | 46.6 ms | 46.8 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.9 ms | 45.6 ms | -0.8% | [-1.3%, -0.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.4 ms | -0.1% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 79.0 ms | 79.1 ms | +0.1% | [-0.0%, +0.3%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.9 ms | 66.1 ms | +0.3% | [-0.2%, +0.8%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.26 ms | 5.22 ms | -0.7% | [-1.9%, +0.5%] | - | ~ | tape/fused 1.47x |
| `Ray/csg-tape/firstHit/float/host` | 7.60 ms | 7.65 ms | +0.6% | [-0.5%, +1.8%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.6 ms | 52.3 ms | -0.6% | [-1.1%, -0.1%] | +0.0% | ~ | hand/api 0.98x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.6 ms | 51.3 ms | -0.5% | [-1.0%, -0.0%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.0 ms | 45.1 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.6 ms | 88.5 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/double/host` | 73.6 ms | 73.8 ms | +0.3% | [-0.6%, +1.3%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.53 ms | 5.57 ms | +0.8% | [-0.3%, +2.0%] | - | ~ | tape/fused 1.42x |
| `Ray/csg-tape/firstHit/double/host` | 7.93 ms | 7.89 ms | -0.5% | [-0.7%, -0.3%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 641.7 s → 639.5 s (-0.4%), the largest peak 1399 MB → 1400 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.1 s | 9.0 s | -1.0% | 557 MB | 558 MB | +0.2% | ~ |
| `BVHRefit` | 4.0 s | 3.9 s | -1.8% | 484 MB | 484 MB | +0.1% | ~ |
| `BVHUnion` | 18.2 s | 18.1 s | -0.2% | 895 MB | 896 MB | +0.1% | ~ |
| `BVHUnionMixed` | 7.3 s | 7.3 s | -0.4% | 556 MB | 556 MB | -0.1% | ~ |
| `ClosestPrimitive` | 29.9 s | 29.2 s | -2.1% | 1296 MB | 1297 MB | +0.1% | ~ |
| `ClosestSurface` | 27.7 s | 27.6 s | -0.6% | 1218 MB | 1219 MB | +0.1% | ~ |
| `Domain` | 18.0 s | 17.9 s | -1.1% | 840 MB | 841 MB | +0.1% | ~ |
| `DomainObject` | 15.7 s | 15.6 s | -1.1% | 843 MB | 844 MB | +0.0% | ~ |
| `Fused` | 24.5 s | 23.7 s | -3.1% | 1126 MB | 1126 MB | +0.0% | ~ |
| `LaunchTuning` | 36.0 s | 35.9 s | -0.2% | 1303 MB | 1303 MB | +0.0% | ~ |
| `Mesh` | 15.1 s | 14.5 s | -3.7% | 784 MB | 784 MB | +0.0% | ~ |
| `MeshCSG` | 38.7 s | 38.4 s | -0.8% | 1255 MB | 1258 MB | +0.2% | ~ |
| `MeshClosestPoint` | - | 13.4 s | | - | 780 MB | | new |
| `MeshReconcile` | 6.5 s | 6.7 s | +2.5% | 565 MB | 566 MB | +0.1% | ~ |
| `PointCloud` | 7.4 s | 7.4 s | -0.8% | 560 MB | 559 MB | -0.2% | ~ |
| `PointCloudBVHBuild` | 3.9 s | 3.8 s | -3.3% | 470 MB | 471 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 3.3 s | 3.2 s | -2.8% | 452 MB | 452 MB | +0.0% | ~ |
| `Ray` | 35.4 s | 35.3 s | -0.2% | 1317 MB | 1316 MB | -0.0% | ~ |
| `RayCSG` | 31.6 s | 31.5 s | -0.5% | 1174 MB | 1176 MB | +0.2% | ~ |
| `RayCrossing` | 44.2 s | 44.6 s | +0.8% | 1333 MB | 1331 MB | -0.2% | ~ |
| `RayParticles` | 48.7 s | 48.0 s | -1.5% | 1318 MB | 1317 MB | -0.0% | ~ |
| `RayShell` | 37.2 s | 36.9 s | -0.8% | 1276 MB | 1278 MB | +0.1% | ~ |
| `Regions` | 28.5 s | 28.7 s | +0.9% | 1070 MB | 1070 MB | +0.0% | ~ |
| `Render` | 35.0 s | 35.8 s | +2.2% | 1282 MB | 1282 MB | +0.0% | ~ |
| `Shapes` | 8.8 s | 8.8 s | +0.4% | 554 MB | 554 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.9 s | +2.1% | 510 MB | 511 MB | +0.0% | ~ |
| `SphereCloudRays` | 27.5 s | 27.7 s | +1.0% | 1060 MB | 1061 MB | +0.1% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | -0.7% | 484 MB | 484 MB | +0.1% | ~ |
| `SurfaceParts` | 39.5 s | 39.7 s | +0.6% | 1399 MB | 1400 MB | +0.1% | ~ |
| `Tape` | 16.5 s | 16.6 s | +0.5% | 819 MB | 819 MB | +0.0% | ~ |
| `TapeBVHUnion` | 14.4 s | 14.5 s | +0.3% | 822 MB | 823 MB | +0.0% | ~ |
| **all, in both** | 641.7 s | 639.5 s | -0.4% | 1399 MB | 1400 MB | +0.1% | ~ |


These probes compiled to the same code on both sides and were not run (252 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), Fused (10), MeshCSG (12), MeshReconcile (4), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/MeshClosestPoint/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: eb1d2c4
base-fingerprint: 3adc6f11e3922774f885720dc4cd9db57d17a9fe
head: 8bc806f
head-fingerprint: 5b762623a2468955cf1da5e76311c603f57f1727
cases: 40
rounds: 2
reps: 3
date: 2026-11-17
-->

## Device comparison (CUDA): green

**Accepted:** -

`8bc806f` against `eb1d2c4` (origin/main), 2026-11-17.

**VERDICT: no regression (0 major, 0 minor across 40 cases)**

Performance comparison: 0 major, 0 minor, 3 faster, 0 differ, 0 lost, 40 cases

- base: eb1d2c4 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 8bc806f (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `TriMeshSDF/w4/query-morton/float/hostdevice` | 9.44 ms | 9.42 ms | -0.1% | [-12.9%, +13.9%] | - | ~ | device 10.2x faster than the host |
| `TriMeshSDF/w4/query-random/float/hostdevice` | 20.2 ms | 20.2 ms | +0.1% | [-1.1%, +1.4%] | - | ~ | device 6.0x faster than the host |
| `TriMeshSDF/w4/query-morton/double/hostdevice` | 163 ms | 167 ms | +2.7% | [-0.3%, +5.8%] | - | ~ | device 0.6x faster than the host |
| `TriMeshSDF/w4/query-random/double/hostdevice` | 310 ms | 293 ms | -5.5% | [-10.6%, -0.4%] | - | faster | device 0.4x faster than the host (was 0.4x) |
| `MeshSDF/query-morton/float/hostdevice` | 29.1 ms | 29.3 ms | +0.9% | [-1.2%, +3.0%] | - | ~ | device 3.5x faster than the host |
| `MeshSDF/query-random/float/hostdevice` | 74.5 ms | 73.3 ms | -1.6% | [-7.1%, +4.2%] | - | ~ | device 2.1x faster than the host |
| `MeshSDF/query-morton/double/hostdevice` | 159 ms | 159 ms | +0.1% | [-3.0%, +3.2%] | - | ~ | device 0.7x faster than the host |
| `MeshSDF/query-random/double/hostdevice` | 268 ms | 267 ms | -0.4% | [-1.6%, +0.7%] | - | ~ | device 0.6x faster than the host |
| `MeshClosestPoint/tri/closestPoint/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshClosestPoint/mesh/closestPoint/float/hostdevice` | - | - | - | - | - | new |  |
| `MeshClosestPoint/tri/closestPoint/double/hostdevice` | - | - | - | - | - | new |  |
| `MeshClosestPoint/mesh/closestPoint/double/hostdevice` | - | - | - | - | - | new |  |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 580 µs | 675 µs | +16.3% | [-1.4%, +34.0%] | - | ~ | device 17.4x faster than the host (was 19.8x); hand/api 0.96x (was 1.25x) |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.40 ms | 1.33 ms | -5.7% | [-15.8%, +6.4%] | - | ~ | device 14.4x faster than the host (was 13.8x); hand/api 0.93x (was 0.88x) |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 724 µs | 651 µs | -10.1% | [-18.7%, +0.5%] | - | ~ | device 17.6x faster than the host (was 15.8x) |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.23 ms | 1.23 ms | -0.2% | [-14.4%, +5.9%] | - | ~ | device 15.4x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 685 µs | 609 µs | -11.0% | [-20.5%, +0.8%] | - | ~ | device 16.4x faster than the host (was 14.8x) |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.20 ms | 1.12 ms | -6.8% | [-12.8%, +0.0%] | - | ~ | device 15.0x faster than the host (was 14.2x) |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.31 ms | 1.26 ms | -3.5% | [-17.5%, +13.3%] | - | ~ | device 12.0x faster than the host (was 11.3x); triangles/polygons 0.53x (was 0.44x) |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.75 ms | 2.77 ms | +0.7% | [-0.4%, +1.9%] | - | ~ | device 9.1x faster than the host; triangles/polygons 0.48x (was 0.51x) |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.24 ms | 1.19 ms | -4.3% | [-14.7%, +6.4%] | - | ~ | device 10.9x faster than the host (was 10.4x) |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.39 ms | 2.35 ms | -1.7% | [-4.6%, +1.3%] | - | ~ | device 9.4x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 772 µs | 693 µs | -10.3% | [-26.5%, +12.2%] | - | ~ | device 2.1x faster than the host (was 2.1x); tape/fused 1.40x (was 1.12x) |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 743 µs | 755 µs | +1.7% | [-0.9%, +4.4%] | - | ~ | device 2.4x faster than the host; tape/fused 1.30x (was 1.34x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 866 µs | 968 µs | +11.7% | [-4.2%, +33.8%] | - | ~ | device 2.7x faster than the host (was 3.1x) |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 997 µs | 980 µs | -1.8% | [-24.7%, +28.6%] | - | ~ | device 3.2x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 18.3 ms | 20.3 ms | +11.0% | [-0.2%, +25.0%] | - | ~ | device 0.6x faster than the host (was 0.6x); hand/api 0.81x (was 0.89x) |
| `Ray/mesh/firstHit-random/double/hostdevice` | 26.4 ms | 26.3 ms | -0.4% | [-1.9%, +1.3%] | - | ~ | device 0.8x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 16.3 ms | 16.3 ms | +0.1% | [-1.3%, +1.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 26.4 ms | 26.5 ms | +0.3% | [-0.4%, +1.0%] | - | ~ | device 0.8x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.1 ms | 14.1 ms | +0.2% | [-0.7%, +1.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 26.6 ms | 25.4 ms | -4.4% | [-19.8%, +14.8%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 15.9 ms | 15.8 ms | -0.6% | [-1.9%, +0.8%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.28x (was 1.15x) |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 23.5 ms | 23.6 ms | +0.4% | [-0.7%, +1.4%] | - | ~ | device 1.1x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 14.3 ms | 14.6 ms | +2.3% | [-0.6%, +5.3%] | - | ~ | device 0.9x faster than the host (was 1.0x) |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 25.0 ms | 22.2 ms | -11.4% | [-20.4%, -0.3%] | - | faster | device 1.1x faster than the host (was 1.0x) |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.23 ms | 1.03 ms | -16.4% | [-17.8%, -14.9%] | - | faster | device 1.5x faster than the host (was 1.3x); tape/fused 1.33x (was 1.10x) |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.18 ms | 2.06 ms | -5.5% | [-11.3%, +0.5%] | - | ~ | device 1.0x faster than the host (was 0.9x); tape/fused 1.11x (was 1.01x) |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.35 ms | 1.36 ms | +0.8% | [-8.6%, +12.3%] | - | ~ | device 2.0x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.20 ms | 2.29 ms | +4.3% | [-0.7%, +9.8%] | - | ~ | device 1.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 5005.9 s → 5160.4 s (+3.1%), the largest peak 6080 MB → 6080 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.0 s | 14.0 s | +7.5% | 600 MB | 600 MB | +0.1% | ~ |
| `BVHRefit` | 6.7 s | 6.9 s | +3.0% | 506 MB | 506 MB | +0.1% | ~ |
| `BVHUnion` | 24.9 s | 25.8 s | +3.7% | 1025 MB | 1026 MB | +0.1% | ~ |
| `BVHUnionMixed` | 11.0 s | 11.4 s | +3.9% | 625 MB | 625 MB | +0.1% | ~ |
| `ClosestPrimitive` | 128.8 s | 134.0 s | +4.0% | 1347 MB | 1348 MB | +0.0% | ~ |
| `ClosestSurface` | 192.7 s | 202.5 s | +5.1% | 1844 MB | 1844 MB | -0.0% | ~ |
| `Domain` | 60.0 s | 62.4 s | +4.0% | 994 MB | 994 MB | +0.0% | ~ |
| `DomainObject` | 56.9 s | 59.7 s | +5.0% | 948 MB | 949 MB | +0.1% | ~ |
| `Fused` | 55.3 s | 56.8 s | +2.7% | 1158 MB | 1158 MB | +0.0% | ~ |
| `LaunchTuning` | 558.3 s | 574.0 s | +2.8% | 3806 MB | 3806 MB | +0.0% | ~ |
| `Mesh` | 20.3 s | 21.1 s | +3.5% | 801 MB | 802 MB | +0.0% | ~ |
| `MeshCSG` | 410.9 s | 426.2 s | +3.7% | 6080 MB | 6080 MB | +0.0% | ~ |
| `MeshClosestPoint` | - | 19.4 s | | - | 755 MB | | new |
| `MeshReconcile` | 10.7 s | 10.8 s | +0.8% | 512 MB | 512 MB | +0.1% | ~ |
| `PointCloud` | 12.6 s | 12.6 s | -0.0% | 570 MB | 571 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 7.3 s | 7.5 s | +2.7% | 506 MB | 506 MB | +0.1% | ~ |
| `PointCloudHashGridBuild` | 6.6 s | 6.5 s | -2.1% | 506 MB | 506 MB | +0.1% | ~ |
| `Ray` | 193.5 s | 204.0 s | +5.4% | 3703 MB | 3702 MB | -0.0% | ~ |
| `RayCSG` | 226.5 s | 236.3 s | +4.3% | 4085 MB | 4085 MB | +0.0% | ~ |
| `RayCrossing` | 451.6 s | 466.5 s | +3.3% | 4017 MB | 4017 MB | +0.0% | ~ |
| `RayParticles` | 703.2 s | 723.9 s | +2.9% | 5632 MB | 5631 MB | -0.0% | ~ |
| `RayShell` | 440.4 s | 451.1 s | +2.4% | 5346 MB | 5346 MB | +0.0% | ~ |
| `Regions` | 377.5 s | 388.7 s | +3.0% | 3835 MB | 3835 MB | +0.0% | ~ |
| `Render` | 269.4 s | 276.3 s | +2.6% | 5896 MB | 5896 MB | +0.0% | ~ |
| `Shapes` | 26.2 s | 28.0 s | +7.2% | 592 MB | 592 MB | +0.1% | ~ |
| `SphereCloudBVH` | 8.7 s | 9.2 s | +5.9% | 506 MB | 507 MB | +0.1% | ~ |
| `SphereCloudRays` | 200.8 s | 202.5 s | +0.8% | 4093 MB | 4092 MB | -0.0% | ~ |
| `SphereContacts` | 8.8 s | 8.5 s | -3.2% | 505 MB | 506 MB | +0.2% | ~ |
| `SurfaceParts` | 419.7 s | 429.5 s | +2.3% | 4241 MB | 4241 MB | -0.0% | ~ |
| `Tape` | 61.2 s | 61.7 s | +0.8% | 981 MB | 982 MB | +0.1% | ~ |
| `TapeBVHUnion` | 42.5 s | 42.0 s | -1.2% | 921 MB | 922 MB | +0.1% | ~ |
| **all, in both** | 5005.9 s | 5160.4 s | +3.1% | 6080 MB | 6080 MB | +0.0% | ~ |

Device kernels: 198 in both, 0 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 4 new, 0 gone. Spills are bytes stored/loaded per thread.


Flagged after 2 rounds, and judged on 10 (1 cases): Ray/mesh/firstHit-handrolled-random/float/hostdevice

These probes compiled to the same code on both sides and were not run (340 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), Domain (6), DomainObject (8), Fused (10), LaunchTuning (24), MeshCSG (18), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), RayCSG (20), RayCrossing (32), RayParticles (20), RayShell (12), Regions (12), Render (6), Shapes (20), SphereCloudBVH (8), SphereCloudRays (12), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

The probes differ between the sides (each side ran its own): Performance/MeshClosestPoint/main.cpp

<!-- END receipt cuda -->
