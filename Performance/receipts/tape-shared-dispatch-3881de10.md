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
base: 6371afb
base-fingerprint: af8d04cce8f9fe3d2fb898d4969c550453d83000
head: 56b5fa5
head-fingerprint: 3881de105e7cce457e1d9444bbe37ec9d2ffea07
cases: 0
rounds: 2
reps: 3
date: 2026-11-24
-->

## Host comparison: green

**Accepted:** -

`56b5fa5` against `6371afb` (origin/main), 2026-11-24.

### Performance comparison

Every probe compiled to the same code on both sides; no case was run.

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 781.1 s → 761.7 s (-2.5%), the largest peak 1418 MB → 1418 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 10.5 s | 9.7 s | -7.4% | 562 MB | 562 MB | +0.1% | ~ |
| `BVHRefit` | 4.4 s | 4.4 s | -0.1% | 503 MB | 503 MB | -0.1% | ~ |
| `BVHUnion` | 20.5 s | 19.1 s | -6.7% | 956 MB | 956 MB | -0.0% | ~ |
| `BVHUnionMixed` | 7.8 s | 7.9 s | +1.0% | 559 MB | 560 MB | +0.1% | ~ |
| `ClosestPrimitive` | 30.6 s | 30.2 s | -1.4% | 1348 MB | 1348 MB | +0.0% | ~ |
| `ClosestSurface` | 31.0 s | 28.0 s | -9.6% | 1318 MB | 1318 MB | -0.0% | ~ |
| `ConnectedComponents` | 15.9 s | 15.4 s | -2.9% | 854 MB | 855 MB | +0.0% | ~ |
| `Domain` | 19.0 s | 18.0 s | -5.3% | 860 MB | 861 MB | +0.1% | ~ |
| `DomainObject` | 16.9 s | 15.9 s | -6.4% | 860 MB | 860 MB | +0.0% | ~ |
| `EBGrid` | 15.0 s | 14.6 s | -2.8% | 784 MB | 784 MB | +0.0% | ~ |
| `Fused` | 25.0 s | 24.3 s | -2.7% | 1237 MB | 1236 MB | -0.0% | ~ |
| `LaunchTuning` | 38.2 s | 37.1 s | -2.8% | 1327 MB | 1328 MB | +0.1% | ~ |
| `Mesh` | 16.4 s | 15.7 s | -4.1% | 790 MB | 791 MB | +0.1% | ~ |
| `MeshCSG` | 40.4 s | 39.5 s | -2.1% | 1305 MB | 1306 MB | +0.1% | ~ |
| `MeshClosestPoint` | 14.7 s | 14.4 s | -2.1% | 787 MB | 786 MB | -0.0% | ~ |
| `MeshIngest` | 7.3 s | 7.2 s | -0.6% | 569 MB | 569 MB | +0.0% | ~ |
| `MeshReconcile` | 6.9 s | 7.0 s | +2.0% | 569 MB | 569 MB | +0.0% | ~ |
| `MeshTape` | 42.4 s | 41.5 s | -2.3% | 1260 MB | 1261 MB | +0.1% | ~ |
| `PointCloud` | 7.6 s | 7.4 s | -1.9% | 562 MB | 562 MB | +0.1% | ~ |
| `PointCloudBVHBuild` | 3.9 s | 3.8 s | -3.2% | 476 MB | 477 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 3.6 s | 3.2 s | -11.4% | 456 MB | 457 MB | +0.2% | ~ |
| `QueryOrder` | 11.8 s | 11.3 s | -4.3% | 707 MB | 707 MB | -0.0% | ~ |
| `Ray` | 36.7 s | 37.1 s | +1.1% | 1342 MB | 1342 MB | -0.0% | ~ |
| `RayCSG` | 32.0 s | 31.6 s | -1.2% | 1204 MB | 1204 MB | +0.0% | ~ |
| `RayCrossing` | 45.9 s | 45.2 s | -1.6% | 1360 MB | 1360 MB | -0.0% | ~ |
| `RayParticles` | 49.7 s | 49.6 s | -0.3% | 1334 MB | 1334 MB | +0.0% | ~ |
| `RayShell` | 38.5 s | 38.0 s | -1.5% | 1314 MB | 1314 MB | +0.0% | ~ |
| `Regions` | 29.9 s | 29.2 s | -2.2% | 1116 MB | 1116 MB | +0.0% | ~ |
| `Render` | 36.8 s | 36.1 s | -1.7% | 1299 MB | 1300 MB | +0.0% | ~ |
| `Shapes` | 8.8 s | 8.9 s | +0.9% | 558 MB | 559 MB | +0.1% | ~ |
| `SphereCloudBVH` | 4.8 s | 4.9 s | +2.7% | 518 MB | 518 MB | +0.0% | ~ |
| `SphereCloudRays` | 29.0 s | 28.4 s | -2.2% | 1077 MB | 1077 MB | +0.1% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | -0.4% | 490 MB | 490 MB | -0.1% | ~ |
| `SurfaceParts` | 42.1 s | 41.2 s | -2.1% | 1418 MB | 1418 MB | +0.0% | ~ |
| `Tape` | 17.9 s | 16.8 s | -6.1% | 837 MB | 838 MB | +0.0% | ~ |
| `TapeBVHUnion` | 14.9 s | 14.8 s | -0.6% | 839 MB | 839 MB | +0.0% | ~ |
| **all, in both** | 781.1 s | 761.7 s | -2.5% | 1418 MB | 1418 MB | +0.0% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

These probes compiled to the same code on both sides and were not run (332 cases): BVH (6), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestPrimitive (6), ClosestSurface (8), ConnectedComponents (4), Domain (6), DomainObject (8), EBGrid (12), Fused (10), Mesh (12), MeshCSG (12), MeshClosestPoint (8), MeshIngest (8), MeshReconcile (4), MeshTape (16), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), QueryOrder (6), Ray (14), RayCSG (10), RayCrossing (16), RayParticles (10), RayShell (6), Regions (12), Render (6), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), SurfaceParts (12), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: yellow
base: 6371afb
base-fingerprint: af8d04cce8f9fe3d2fb898d4969c550453d83000
head: 56b5fa5
head-fingerprint: 3881de105e7cce457e1d9444bbe37ec9d2ffea07
cases: 274
rounds: 2
reps: 3
date: 2026-11-24
-->

## Device comparison (CUDA): yellow

**Accepted:** -

`56b5fa5` against `6371afb` (origin/main), 2026-11-24.

**VERDICT: 6 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 6 minor, 5 faster, 0 differ, 0 lost, 274 cases

- base: 6371afb (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: 56b5fa5 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 317 µs | 309 µs | -2.6% | [-2.7%, -2.4%] | - | ~ | device 12.3x faster than the host (was 11.7x) |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.31 ms | 1.36 ms | +3.7% | [+3.6%, +3.8%] | - | minor | device 6.0x faster than the host (was 6.2x) |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 59.9 ms | 59.8 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 3.3x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.3% | [-0.0%, +0.6%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.39 ms | 5.44 ms | +1.0% | [+0.8%, +1.1%] | - | ~ | device 1.6x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 529 ms | 529 ms | -0.1% | [-1.2%, +1.0%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | 20.4 ms | 20.8 ms | +1.8% | [+0.5%, +3.1%] | - | ~ | device 4.0x faster than the host |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | 8.35 ms | 8.40 ms | +0.6% | [+0.6%, +0.6%] | - | ~ | device 4.7x faster than the host; surface/leaf 2.47x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | 16.1 ms | 15.6 ms | -2.8% | [-2.9%, -2.7%] | - | ~ | device 8.0x faster than the host |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | 6.19 ms | 6.19 ms | +0.0% | [+0.0%, +0.1%] | - | ~ | device 10.0x faster than the host; surface/leaf 2.52x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | 149 ms | 147 ms | -1.3% | [-2.6%, +0.1%] | - | ~ | device 0.5x faster than the host |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | 61.0 ms | 60.9 ms | -0.1% | [-0.2%, -0.1%] | - | ~ | device 0.7x faster than the host; surface/leaf 2.41x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | 73.0 ms | 72.9 ms | -0.0% | [-0.0%, -0.0%] | - | ~ | device 1.7x faster than the host |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | 28.5 ms | 28.6 ms | +0.6% | [+0.0%, +1.2%] | - | ~ | device 2.2x faster than the host; surface/leaf 2.55x |
| `Domain/particles/labels/float/hostdevice` | 4.11 ms | 4.09 ms | -0.4% | [-1.0%, +0.1%] | - | ~ | device 3.4x faster than the host |
| `Domain/particles/labelAt/float/hostdevice` | 2.73 ms | 2.77 ms | +1.5% | [+1.0%, +2.0%] | - | ~ | device 2.6x faster than the host |
| `Domain/particles/surfaceGroups/float/hostdevice` | 114 µs | 114 µs | +0.3% | [-0.9%, +1.6%] | - | ~ | device 12.9x faster than the host |
| `Domain/particles/labels/double/hostdevice` | 7.01 ms | 7.03 ms | +0.2% | [-1.3%, +1.7%] | - | ~ | device 2.0x faster than the host |
| `Domain/particles/labelAt/double/hostdevice` | 3.66 ms | 3.78 ms | +3.2% | [+2.8%, +3.8%] | - | minor | device 1.9x faster than the host (was 2.0x) |
| `Domain/particles/surfaceGroups/double/hostdevice` | 117 µs | 114 µs | -2.5% | [-4.2%, -0.9%] | - | ~ | device 13.1x faster than the host (was 12.6x) |
| `DomainObject/partsWhole/labels/float/hostdevice` | 12.4 ms | 13.4 ms | +8.1% | [+3.9%, +8.6%] | - | minor | device 3.4x faster than the host (was 3.6x) |
| `DomainObject/partsWhole/labelAt/float/hostdevice` | 8.78 ms | 8.97 ms | +2.1% | [+1.7%, +6.7%] | - | ~ | device 2.6x faster than the host |
| `DomainObject/partsOwn/labels/float/hostdevice` | 260 µs | 277 µs | +6.5% | [+5.8%, +7.1%] | - | minor | device 2.4x faster than the host (was 2.5x); whole/own 48.55x |
| `DomainObject/partsOwn/labelAt/float/hostdevice` | 156 µs | 162 µs | +3.7% | [+2.6%, +4.6%] | - | minor | device 2.2x faster than the host (was 2.3x); whole/own 55.36x |
| `DomainObject/partsWhole/labels/double/hostdevice` | 20.3 ms | 20.4 ms | +0.8% | [-0.2%, +1.9%] | - | ~ | device 2.3x faster than the host |
| `DomainObject/partsWhole/labelAt/double/hostdevice` | 12.2 ms | 12.3 ms | +1.1% | [-1.2%, +3.4%] | - | ~ | device 2.0x faster than the host |
| `DomainObject/partsOwn/labels/double/hostdevice` | 392 µs | 376 µs | -4.1% | [-9.0%, +1.0%] | - | ~ | device 1.9x faster than the host (was 1.7x); whole/own 54.25x (was 51.61x) |
| `DomainObject/partsOwn/labelAt/double/hostdevice` | 236 µs | 233 µs | -1.1% | [-1.6%, -0.7%] | - | ~ | device 1.6x faster than the host; whole/own 53.02x |
| `Fused/csg/signedDistance/float/hostdevice` | 48.1 µs | 48.0 µs | -0.3% | [-0.3%, -0.2%] | - | ~ | device 23.5x faster than the host; tape/fused 4.54x |
| `Fused/smooth/signedDistance/float/hostdevice` | 50.2 µs | 50.8 µs | +1.2% | [-0.4%, +2.8%] | - | ~ | device 43.3x faster than the host; tape/fused 7.08x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.3 µs | 48.1 µs | -0.4% | [-0.7%, -0.1%] | - | ~ | device 8.9x faster than the host (was 9.2x); tape/fused 9.91x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 18.1 ms | 18.1 ms | -0.2% | [-1.6%, +1.2%] | - | ~ | device 6.5x faster than the host; tape/fused 1.76x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.7 ms | 31.8 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.5x faster than the host (was 1.6x); tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.11 ms | 2.10 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 279 ms | 285 ms | +2.1% | [+0.2%, +4.1%] | - | ~ | device 0.4x faster than the host; tape/fused 0.98x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 281 ms | 281 ms | -0.1% | [-0.7%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.47 ms | 8.55 ms | +1.0% | [-3.6%, +5.8%] | +0.0% | ~ | device 10.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.33 ms | 8.44 ms | +1.3% | [+0.4%, +2.2%] | +0.0% | ~ | device 11.6x faster than the host (was 10.7x); default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.22 ms | 8.22 ms | +0.1% | [-0.8%, +0.9%] | +0.0% | ~ | device 12.0x faster than the host (was 11.3x); default/this 1.04x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.21 ms | 8.42 ms | +2.5% | [+1.7%, +3.4%] | +0.0% | ~ | device 10.5x faster than the host (was 11.0x); default/this 1.02x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 694 µs | 700 µs | +0.8% | [+0.1%, +1.6%] | +0.0% | ~ | device 3.7x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.80 ms | 1.77 ms | -1.5% | [-1.6%, -1.4%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.06 ms | 1.03 ms | -2.6% | [-3.1%, -2.0%] | +0.0% | ~ | device 2.5x faster than the host (was 2.4x); default/this 0.68x (was 0.66x) |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 796 µs | 791 µs | -0.6% | [-1.2%, -0.1%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 735 µs | 734 µs | -0.1% | [-1.8%, +1.6%] | +0.0% | ~ | device 12.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 821 µs | 821 µs | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 10.8x faster than the host; default/this 0.90x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 736 µs | 737 µs | +0.1% | [-0.5%, +0.6%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 726 µs | 726 µs | -0.0% | [-0.5%, +0.4%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 164 ms | 167 ms | +2.1% | [+0.0%, +4.2%] | +0.0% | ~ | device 0.5x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 161 ms | 162 ms | +0.3% | [-0.1%, +0.8%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.03x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 165 ms | 163 ms | -0.9% | [-2.8%, +1.0%] | +0.0% | ~ | device 0.5x faster than the host; default/this 1.03x (was 1.00x) |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 163 ms | 163 ms | -0.2% | [-1.7%, +1.3%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.03x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.40 ms | 1.39 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.78 ms | 1.45 ms | -47.8% | [-48.0%, -47.6%] | +0.0% | faster | device 1.8x faster than the host (was 1.0x); default/this 0.96x (was 0.50x) |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.91 ms | 1.95 ms | +2.4% | [+2.0%, +2.7%] | +0.0% | ~ | device 1.4x faster than the host; default/this 0.71x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.58 ms | +1.1% | [+0.5%, +1.8%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.88x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.28 ms | 7.22 ms | -0.8% | [-0.9%, -0.6%] | +0.0% | ~ | device 1.3x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.32 ms | 7.29 ms | -0.4% | [-0.8%, +0.1%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.27 ms | 7.25 ms | -0.2% | [-0.9%, +0.5%] | +0.0% | ~ | device 1.3x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.32 ms | 7.31 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 1.2x faster than the host; default/this 0.99x |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.01 ms | 2.04 ms | +1.6% | [+1.2%, +2.0%] | - | ~ | device 5.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 3.40 ms | 3.47 ms | +2.3% | [+1.9%, +2.7%] | - | ~ | device 4.0x faster than the host (was 4.2x) |
| `MeshCSG/union-tape/frame/float/hostdevice` | 6.14 ms | 6.13 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 4.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 64.3 ms | 65.5 ms | +1.8% | [+1.3%, +2.4%] | - | ~ | device 3.2x faster than the host (was 3.1x) |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 95.1 ms | 95.4 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 241 ms | 245 ms | +1.9% | [+0.4%, +3.3%] | - | ~ | device 3.4x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.4 ms | 25.4 ms | -0.0% | [-1.8%, +1.8%] | - | ~ | device 2.2x faster than the host (was 2.0x); tape/fused 2.58x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 27.1 ms | 27.7 ms | +2.3% | [-0.1%, +4.7%] | - | ~ | device 2.1x faster than the host (was 2.5x); tape/fused 3.45x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 67.1 ms | 67.2 ms | +0.2% | [-1.3%, +1.7%] | - | ~ | device 3.8x faster than the host; tape/fused 3.65x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.5 ms | 21.7 ms | -3.4% | [-10.3%, +4.0%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 39.1 ms | 39.9 ms | +2.1% | [+0.6%, +3.6%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/union-tape/frame/double/hostdevice` | 42.2 ms | 43.3 ms | +2.7% | [-0.5%, +5.9%] | - | ~ | device 0.7x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 553 ms | 559 ms | +1.1% | [-7.2%, +10.1%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 866 ms | 848 ms | -2.1% | [-3.9%, -0.3%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 1.96 s | 2.01 s | +2.8% | [+0.2%, +5.6%] | - | ~ | device 0.4x faster than the host (was 0.5x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 193 ms | 196 ms | +1.5% | [+0.2%, +2.7%] | - | ~ | device 0.2x faster than the host; tape/fused 2.85x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 314 ms | 304 ms | -3.3% | [-8.3%, +1.8%] | - | ~ | device 0.2x faster than the host (was 0.2x); tape/fused 2.79x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 559 ms | 546 ms | -2.4% | [-5.9%, +1.1%] | - | ~ | device 0.5x faster than the host (was 0.5x); tape/fused 3.69x (was 3.50x) |
| `MeshTape/mesh/firstHit/float/hostdevice` | 1.32 ms | 1.32 ms | +0.5% | [+0.2%, +0.9%] | - | ~ | device 15.7x faster than the host |
| `MeshTape/mesh/firstCrossing/float/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [-0.5%, +0.7%] | - | ~ | device 15.7x faster than the host |
| `MeshTape/mesh/frame/float/hostdevice` | 2.14 ms | 2.16 ms | +1.3% | [+0.6%, +2.0%] | - | ~ | device 7.8x faster than the host |
| `MeshTape/tape/firstHit/float/hostdevice` | 3.51 ms | 3.52 ms | +0.3% | [+0.2%, +0.4%] | - | ~ | device 10.2x faster than the host; mesh/tape 0.38x |
| `MeshTape/tape/firstCrossing/float/hostdevice` | 2.77 ms | 2.76 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 7.8x faster than the host; mesh/tape 0.48x |
| `MeshTape/tape/frame/float/hostdevice` | 4.41 ms | 4.43 ms | +0.3% | [-0.3%, +1.0%] | - | ~ | device 4.9x faster than the host; mesh/tape 0.49x |
| `MeshTape/view/firstHit/float/hostdevice` | 3.72 ms | 3.74 ms | +0.6% | [+0.4%, +0.8%] | - | ~ | device 9.7x faster than the host; mesh/view 0.35x |
| `MeshTape/view/frame/float/hostdevice` | 5.67 ms | 5.68 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 4.2x faster than the host; mesh/view 0.38x |
| `MeshTape/mesh/firstHit/double/hostdevice` | 30.8 ms | 30.7 ms | -0.4% | [-1.3%, +0.5%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/firstCrossing/double/hostdevice` | 30.8 ms | 30.4 ms | -1.3% | [-2.2%, -0.3%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/mesh/frame/double/hostdevice` | 26.1 ms | 26.2 ms | +0.3% | [-0.8%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `MeshTape/tape/firstHit/double/hostdevice` | 44.3 ms | 44.0 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 0.8x faster than the host; mesh/tape 0.70x |
| `MeshTape/tape/firstCrossing/double/hostdevice` | 30.7 ms | 30.8 ms | +0.5% | [-0.9%, +1.9%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.99x |
| `MeshTape/tape/frame/double/hostdevice` | 33.6 ms | 33.7 ms | +0.2% | [-1.0%, +1.4%] | - | ~ | device 0.7x faster than the host; mesh/tape 0.78x |
| `MeshTape/view/firstHit/double/hostdevice` | 44.1 ms | 44.0 ms | -0.2% | [-1.3%, +1.0%] | - | ~ | device 0.8x faster than the host; mesh/view 0.70x |
| `MeshTape/view/frame/double/hostdevice` | 36.5 ms | 36.3 ms | -0.6% | [-0.7%, -0.4%] | - | ~ | device 0.7x faster than the host; mesh/view 0.72x |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 705 µs | 701 µs | -0.5% | [-1.1%, +0.2%] | - | ~ | device 17.3x faster than the host; hand/api 0.93x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.31 ms | 1.31 ms | -0.1% | [-0.8%, +0.5%] | - | ~ | device 15.3x faster than the host (was 16.4x); hand/api 0.94x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 660 µs | 653 µs | -1.0% | [-1.3%, -0.6%] | - | ~ | device 17.4x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.24 ms | 1.23 ms | -1.1% | [-2.5%, +0.4%] | - | ~ | device 15.4x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 632 µs | 629 µs | -0.6% | [-2.4%, +1.2%] | - | ~ | device 15.7x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.16 ms | 1.16 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 14.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.19 ms | -0.1% | [-0.9%, +0.7%] | - | ~ | device 12.0x faster than the host; triangles/polygons 0.59x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.49 ms | 2.48 ms | -0.4% | [-0.6%, -0.1%] | - | ~ | device 9.8x faster than the host; triangles/polygons 0.53x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | +0.7% | [+0.2%, +1.2%] | - | ~ | device 12.0x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.17 ms | -0.1% | [-0.6%, +0.3%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 617 µs | 617 µs | +0.1% | [-5.9%, +6.5%] | - | ~ | device 2.4x faster than the host; tape/fused 1.13x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 734 µs | 736 µs | +0.3% | [+0.1%, +0.5%] | - | ~ | device 2.5x faster than the host; tape/fused 1.17x |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 692 µs | 700 µs | +1.1% | [+1.0%, +1.1%] | - | ~ | device 3.6x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 845 µs | 858 µs | +1.6% | [+1.2%, +1.9%] | - | ~ | device 3.5x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 19.1 ms | 18.9 ms | -1.1% | [-2.0%, -0.3%] | - | ~ | device 0.7x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 30.7 ms | 30.7 ms | +0.2% | [-0.4%, +0.8%] | - | ~ | device 0.7x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 18.5 ms | 18.3 ms | -0.9% | [-2.3%, +0.6%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 29.7 ms | 29.4 ms | -0.9% | [-1.2%, -0.6%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 16.3 ms | 15.9 ms | -2.1% | [-3.5%, -0.7%] | - | ~ | device 0.6x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 26.4 ms | 26.0 ms | -1.7% | [-1.8%, -1.5%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 17.5 ms | 17.4 ms | -0.3% | [-1.3%, +0.6%] | - | ~ | device 0.8x faster than the host; triangles/polygons 1.08x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 25.9 ms | 25.7 ms | -0.8% | [-1.7%, +0.1%] | - | ~ | device 1.0x faster than the host; triangles/polygons 1.20x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.4 ms | 15.3 ms | -0.7% | [-3.6%, +2.3%] | - | ~ | device 0.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.5 ms | 23.4 ms | -0.7% | [-0.9%, -0.5%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.24 ms | -0.8% | [-1.4%, -0.1%] | - | ~ | device 1.3x faster than the host (was 1.2x); tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.37 ms | 2.38 ms | +0.4% | [-0.7%, +1.4%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 1.9x faster than the host |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.48 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.5x faster than the host; marched/traced 3.64x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 3.00 ms | 2.99 ms | -0.3% | [-0.6%, -0.1%] | - | ~ | device 2.3x faster than the host; marched/traced 4.65x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.03 ms | 9.25 ms | +2.5% | [+0.0%, +4.9%] | - | ~ | device 4.8x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.9 ms | 13.9 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 3.7x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.18 ms | 2.16 ms | -1.0% | [-1.3%, -0.8%] | - | ~ | device 3.9x faster than the host; marched/traced 13.57x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.48 ms | 5.42 ms | -1.1% | [-1.3%, -0.8%] | - | ~ | device 2.8x faster than the host; marched/traced 10.23x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 29.7 ms | 29.3 ms | -1.4% | [-1.9%, -0.9%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 56.1 ms | 55.4 ms | -1.2% | [-1.6%, -0.9%] | - | ~ | device 2.0x faster than the host (was 2.5x) |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.23 ms | 5.25 ms | +0.3% | [+0.2%, +0.5%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.76 ms | 8.82 ms | +0.7% | [+0.6%, +0.8%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.19 ms | 6.22 ms | +0.5% | [-0.1%, +1.1%] | - | ~ | device 1.3x faster than the host; marched/traced 25.98x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 0.8x faster than the host; marched/traced 24.52x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 160 ms | 162 ms | +1.0% | [-3.8%, +6.1%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 259 ms | 260 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.90 ms | 4.87 ms | -0.6% | [-0.9%, -0.3%] | - | ~ | device 1.8x faster than the host; marched/traced 22.84x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.5 ms | +1.8% | [-0.6%, +4.2%] | - | ~ | device 1.2x faster than the host; marched/traced 17.52x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 111 ms | 111 ms | +0.5% | [-1.4%, +2.4%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 218 ms | 219 ms | +0.4% | [-2.9%, +3.8%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.5 ms | 19.8 ms | +1.1% | [+0.1%, +2.2%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 27.0 ms | 27.2 ms | +0.8% | [+0.1%, +1.6%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 711 µs | 710 µs | -0.2% | [-0.7%, +0.3%] | - | ~ | device 17.1x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.30 ms | 1.32 ms | +1.1% | [+0.8%, +1.4%] | - | ~ | device 15.2x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 705 µs | 705 µs | +0.0% | [-0.7%, +0.7%] | - | ~ | device 17.2x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.31 ms | 1.31 ms | -0.3% | [-0.7%, +0.1%] | - | ~ | device 15.3x faster than the host; hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 687 µs | 705 µs | +2.7% | [+2.0%, +3.3%] | - | ~ | device 3.6x faster than the host (was 3.8x) |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 843 µs | 852 µs | +1.1% | [+0.8%, +1.5%] | - | ~ | device 3.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 614 µs | 627 µs | +2.2% | [+2.0%, +2.3%] | - | ~ | device 3.1x faster than the host; hit/crossing 1.12x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 636 µs | 654 µs | +2.7% | [+2.1%, +3.3%] | - | ~ | device 3.6x faster than the host; hit/crossing 1.30x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.77 ms | 4.77 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.3 ms | 10.3 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.22 ms | 4.21 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 10.1x faster than the host; hit/crossing 1.13x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.36 ms | 9.12 ms | -2.5% | [-5.0%, +0.1%] | - | ~ | device 6.8x faster than the host (was 7.4x); hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 57.3 ms | 58.2 ms | +1.6% | [-0.9%, +4.2%] | - | ~ | device 3.0x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 82.6 ms | 83.9 ms | +1.6% | [+0.3%, +2.9%] | - | ~ | device 2.5x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 49.9 ms | 48.6 ms | -2.5% | [-2.9%, -2.2%] | - | ~ | device 3.0x faster than the host (was 2.9x); hit/crossing 1.20x (was 1.15x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 65.1 ms | 64.6 ms | -0.8% | [-2.0%, +0.5%] | - | ~ | device 2.8x faster than the host (was 2.4x); hit/crossing 1.30x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 18.9 ms | 18.7 ms | -1.0% | [-2.3%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 30.7 ms | 30.9 ms | +0.7% | [+0.0%, +1.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 19.1 ms | 18.9 ms | -0.9% | [-1.9%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 0.99x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 30.7 ms | 30.6 ms | -0.4% | [-1.0%, +0.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | -0.2% | [-0.5%, +0.2%] | - | ~ | device 1.9x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.50 ms | 2.48 ms | -0.6% | [-1.1%, -0.1%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.10 ms | +0.2% | [+0.2%, +0.3%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.26x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.16 ms | -0.3% | [-0.8%, +0.2%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.14x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 69.7 ms | 69.6 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 132 ms | 132 ms | +0.3% | [-0.5%, +1.1%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 63.1 ms | 63.0 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.11x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 121 ms | 119 ms | -1.8% | [-2.3%, -1.3%] | - | ~ | device 0.5x faster than the host; hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 476 ms | 473 ms | -0.7% | [-7.4%, +6.4%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 714 ms | 722 ms | +1.0% | [-0.7%, +2.8%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 392 ms | 394 ms | +0.7% | [-3.1%, +4.6%] | - | ~ | device 0.4x faster than the host (was 0.4x); hit/crossing 1.20x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 536 ms | 534 ms | -0.4% | [-2.2%, +1.5%] | - | ~ | device 0.3x faster than the host (was 0.3x); hit/crossing 1.35x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.56 ms | 3.57 ms | +0.3% | [-1.1%, +1.7%] | - | ~ | device 1.2x faster than the host; marched/traced 3.42x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.80 ms | 4.79 ms | -0.2% | [-1.0%, +0.5%] | - | ~ | device 1.1x faster than the host (was 0.9x); marched/traced 5.67x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.2 ms | 12.2 ms | -0.2% | [-3.3%, +3.0%] | - | ~ | device 4.8x faster than the host (was 6.8x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.8 ms | 27.1 ms | +1.3% | [+0.2%, +2.4%] | - | ~ | device 2.6x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.25 ms | 1.25 ms | +0.6% | [-2.7%, +4.0%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.85 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 2.3x faster than the host (was 2.1x) |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 96.4 µs | 98.2 µs | +1.9% | [+0.8%, +3.0%] | - | ~ | device 17.4x faster than the host (was 17.9x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 155 µs | 157 µs | +1.2% | [+0.2%, +2.2%] | - | ~ | device 13.6x faster than the host (was 13.1x) |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 799 µs | 798 µs | -0.2% | [-0.5%, +0.1%] | - | ~ | device 3.5x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 935 µs | 936 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 3.2x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 37.3 ms | 34.5 ms | -7.5% | [-10.5%, -4.4%] | - | faster | device 0.2x faster than the host (was 0.1x); marched/traced 3.53x (was 3.33x) |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 37.1 ms | 37.1 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 6.41x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 124 ms | 122 ms | -1.8% | [-3.2%, -0.3%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 232 ms | 238 ms | +2.3% | [+1.4%, +3.3%] | - | ~ | device 0.3x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.8 ms | 16.6 ms | -1.4% | [-4.9%, +4.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.9 ms | 22.4 ms | +2.4% | [-0.1%, +4.9%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.23 ms | 1.18 ms | -4.2% | [-8.3%, +0.0%] | - | ~ | device 1.6x faster than the host (was 1.4x) |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.49 ms | 2.53 ms | +1.5% | [+0.5%, +2.5%] | - | ~ | device 0.8x faster than the host (was 0.9x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 6.02 ms | 6.00 ms | -0.3% | [-1.3%, +0.7%] | - | ~ | device 0.6x faster than the host (was 0.5x) |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.71 ms | 6.70 ms | -0.1% | [-0.3%, +0.1%] | - | ~ | device 0.5x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.6 ms | 23.7 ms | +0.5% | [-1.4%, +2.3%] | - | ~ | device 2.7x faster than the host (was 2.8x); marched/traced 2.64x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.7 ms | 29.0 ms | +0.8% | [+0.7%, +0.9%] | - | ~ | device 2.4x faster than the host; marched/traced 3.65x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 63.1 ms | 62.5 ms | -0.9% | [-1.1%, -0.7%] | - | ~ | device 4.5x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 105 ms | 106 ms | +0.4% | [-1.1%, +2.1%] | - | ~ | device 2.8x faster than the host (was 3.0x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 14.2 ms | 14.1 ms | -1.0% | [-3.0%, +1.1%] | - | ~ | device 4.5x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.8 ms | 17.7 ms | -0.5% | [-1.9%, +0.9%] | - | ~ | device 4.1x faster than the host (was 3.9x) |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 304 ms | 305 ms | +0.4% | [-2.6%, +5.6%] | - | ~ | device 0.2x faster than the host; marched/traced 1.88x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 359 ms | 351 ms | -2.4% | [-4.2%, -0.5%] | - | ~ | device 0.2x faster than the host (was 0.2x); marched/traced 2.91x (was 2.80x) |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 578 ms | 574 ms | -0.7% | [-1.2%, -0.2%] | - | ~ | device 0.5x faster than the host (was 0.5x) |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.01 s | 1.02 s | +1.3% | [-0.0%, +2.7%] | - | ~ | device 0.2x faster than the host |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 189 ms | 193 ms | +2.4% | [-1.7%, +6.6%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 241 ms | 236 ms | -2.1% | [-3.5%, -0.8%] | - | ~ | device 0.2x faster than the host |
| `Regions/particles/region/float/hostdevice` | 2.75 ms | 2.76 ms | +0.2% | [+0.2%, +0.2%] | - | ~ | device 2.7x faster than the host |
| `Regions/particles/regionDistances/float/hostdevice` | 3.74 ms | 3.67 ms | -1.8% | [-2.3%, -1.3%] | - | ~ | device 3.5x faster than the host |
| `Regions/particles/regionByRegion/float/hostdevice` | 90.6 ms | 98.7 ms | +8.9% | [+8.2%, +9.8%] | - | minor | device 3.1x faster than the host (was 3.2x); one-pass/each 0.04x (was 0.04x) |
| `Regions/particles/domainLabels/float/hostdevice` | 3.70 ms | 3.73 ms | +0.8% | [+0.4%, +1.2%] | - | ~ | device 3.6x faster than the host |
| `Regions/assembly/firstRegionChange/float/hostdevice` | 6.97 ms | 6.80 ms | -2.5% | [-5.8%, +1.1%] | - | ~ | device 3.7x faster than the host |
| `Regions/assembly/firstHit/float/hostdevice` | 5.52 ms | 5.60 ms | +1.5% | [+1.2%, +1.7%] | - | ~ | device 3.2x faster than the host; regions/hit 1.21x (was 1.26x) |
| `Regions/particles/region/double/hostdevice` | 3.55 ms | 3.60 ms | +1.7% | [+1.3%, +2.0%] | - | ~ | device 2.1x faster than the host |
| `Regions/particles/regionDistances/double/hostdevice` | 6.21 ms | 5.86 ms | -5.7% | [-10.0%, -1.1%] | - | faster | device 2.3x faster than the host (was 2.2x) |
| `Regions/particles/regionByRegion/double/hostdevice` | 135 ms | 116 ms | -13.5% | [-13.6%, -13.4%] | - | faster | device 2.8x faster than the host (was 2.2x); one-pass/each 0.05x (was 0.05x) |
| `Regions/particles/domainLabels/double/hostdevice` | 5.88 ms | 5.90 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 2.3x faster than the host |
| `Regions/assembly/firstRegionChange/double/hostdevice` | 16.0 ms | 16.0 ms | +0.2% | [-2.6%, +3.0%] | - | ~ | device 1.6x faster than the host |
| `Regions/assembly/firstHit/double/hostdevice` | 13.4 ms | 12.9 ms | -3.6% | [-4.1%, -3.0%] | - | faster | device 1.3x faster than the host (was 1.3x); regions/hit 1.24x (was 1.19x) |
| `Render/csg/frame/float/hostdevice` | 2.38 ms | 2.37 ms | -0.3% | [-0.6%, -0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/csg/frame-orthographic/float/hostdevice` | 2.37 ms | 2.36 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 3.0x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 2.15 ms | 2.13 ms | -0.7% | [-1.2%, -0.2%] | - | ~ | device 7.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 5.61 ms | 5.63 ms | +0.2% | [+0.0%, +0.4%] | - | ~ | device 1.4x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 5.62 ms | 5.62 ms | +0.0% | [-0.3%, +0.3%] | - | ~ | device 1.4x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 26.6 ms | 26.3 ms | -0.8% | [-2.7%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.01 ms | 3.01 ms | -0.0% | [-0.6%, +0.5%] | +0.0% | ~ | device 7.2x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.71 ms | 4.74 ms | +0.8% | [+0.5%, +1.1%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 17.0 ms | 17.0 ms | -0.0% | [-2.4%, +2.5%] | +0.0% | ~ | device 5.0x faster than the host (was 5.8x); tape/fused 1.37x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.6 ms | 24.5 ms | -0.2% | [-1.9%, +1.7%] | +0.0% | ~ | device 4.6x faster than the host (was 3.9x); tape/fused 1.28x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 23.4 ms | 23.2 ms | -0.9% | [-1.8%, +0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 31.1 ms | 31.3 ms | +0.9% | [+0.2%, +1.6%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.9 ms | 44.6 ms | -0.6% | [-1.0%, -0.1%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 46.3 ms | 46.5 ms | +0.5% | [+0.2%, +0.7%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 125 ms | 125 ms | +0.3% | [-3.1%, +3.8%] | +0.0% | ~ | device 0.7x faster than the host; tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 130 ms | 127 ms | -2.1% | [-4.9%, +0.9%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.09x (was 1.04x) |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 130 ms | 130 ms | +0.5% | [-2.2%, +3.3%] | +0.0% | ~ | device 0.7x faster than the host |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 134 ms | 138 ms | +2.9% | [+1.6%, +4.2%] | +0.0% | ~ | device 0.7x faster than the host |
| `SurfaceParts/boxes/closestSurface/float/hostdevice` | 9.33 ms | 9.30 ms | -0.3% | [-0.3%, -0.3%] | - | ~ | device 12.0x faster than the host (was 12.6x) |
| `SurfaceParts/boxes/closestLeaf/float/hostdevice` | 5.93 ms | 5.91 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 10.1x faster than the host; surface/leaf 1.57x |
| `SurfaceParts/mesh-groups/closestSurface/float/hostdevice` | 96.9 ms | 99.7 ms | +2.9% | [+2.4%, +3.3%] | - | ~ | device 3.6x faster than the host (was 3.7x) |
| `SurfaceParts/mesh-groups/closestLeaf/float/hostdevice` | 64.3 ms | 65.0 ms | +1.1% | [+0.8%, +1.3%] | - | ~ | device 3.4x faster than the host (was 3.2x); surface/leaf 1.53x |
| `SurfaceParts/mesh-groups/firstSurface/float/hostdevice` | 29.7 ms | 30.0 ms | +0.8% | [-0.2%, +1.8%] | - | ~ | device 4.9x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/float/hostdevice` | 10.0 ms | 9.98 ms | -0.2% | [-0.3%, -0.1%] | - | ~ | device 7.4x faster than the host (was 7.0x); surface/hit 3.00x |
| `SurfaceParts/boxes/closestSurface/double/hostdevice` | 60.0 ms | 59.8 ms | -0.3% | [-0.7%, +0.0%] | - | ~ | device 1.8x faster than the host (was 1.9x) |
| `SurfaceParts/boxes/closestLeaf/double/hostdevice` | 39.4 ms | 39.4 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 1.6x faster than the host (was 1.5x); surface/leaf 1.52x |
| `SurfaceParts/mesh-groups/closestSurface/double/hostdevice` | 796 ms | 796 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 0.4x faster than the host |
| `SurfaceParts/mesh-groups/closestLeaf/double/hostdevice` | 534 ms | 538 ms | +0.7% | [+0.4%, +0.9%] | - | ~ | device 0.4x faster than the host; surface/leaf 1.48x |
| `SurfaceParts/mesh-groups/firstSurface/double/hostdevice` | 320 ms | 319 ms | -0.3% | [-2.0%, +1.4%] | - | ~ | device 0.5x faster than the host |
| `SurfaceParts/mesh-groups/firstHit/double/hostdevice` | 128 ms | 128 ms | -0.2% | [-1.2%, +0.8%] | - | ~ | device 0.6x faster than the host; surface/hit 2.50x |
| `Tape/csg/signedDistance/float/hostdevice` | 217 µs | 218 µs | +0.5% | [+0.3%, +0.6%] | - | ~ | device 11.5x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 48.0 µs | 48.1 µs | +0.3% | [+0.2%, +0.5%] | - | ~ | device 24.6x faster than the host (was 23.5x) |
| `Tape/smooth/signedDistance/float/hostdevice` | 359 µs | 359 µs | -0.0% | [-0.1%, +0.1%] | - | ~ | device 12.6x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | -0.3% | [-0.4%, -0.1%] | - | ~ | device 9.2x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.2 µs | 72.2 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 7.8x faster than the host (was 7.2x) |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.9 µs | +0.2% | [-0.8%, +1.2%] | - | ~ | device 4.6x faster than the host (was 4.8x) |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 652 µs | 653 µs | +0.1% | [-0.1%, +0.3%] | - | ~ | device 10.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | +0.1% | [+0.1%, +0.2%] | - | ~ | device 11.9x faster than the host (was 12.3x) |
| `Tape/chain16/signedDistance/float/hostdevice` | 476 µs | 476 µs | +0.2% | [+0.0%, +0.3%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.3 µs | 48.4 µs | +0.0% | [-0.8%, +0.8%] | - | ~ | device 27.5x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 858 µs | 859 µs | +0.1% | [+0.1%, +0.1%] | - | ~ | device 3.1x faster than the host (was 3.0x) |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 811 µs | 810 µs | -0.2% | [-0.4%, -0.0%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.65 ms | 6.65 ms | +0.0% | [-0.1%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 191 µs | 191 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 3.0x faster than the host (was 3.1x) |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 1.9x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.77 ms | -0.2% | [-0.3%, -0.2%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.0%] | - | ~ | device 0.8x faster than the host (was 0.8x) |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.13 ms | 2.13 ms | +0.1% | [-0.2%, +0.4%] | - | ~ | device 1.7x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.11 ms | 2.10 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 0.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | -0.2% | [-0.3%, +0.0%] | - | ~ | device 8.2x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.71 ms | 3.70 ms | -0.2% | [-0.4%, -0.1%] | - | ~ | device 10.8x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.51 ms | -0.0% | [-0.1%, -0.0%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 18.0 ms | 17.8 ms | -1.3% | [-2.7%, +0.1%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 6175.3 s → 4713.4 s (-23.7%), the largest peak 6575 MB → 5452 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 13.8 s | 13.6 s | -1.1% | 619 MB | 619 MB | +0.0% | ~ |
| `BVHRefit` | 7.2 s | 7.2 s | -0.3% | 523 MB | 523 MB | +0.0% | ~ |
| `BVHUnion` | 26.7 s | 25.7 s | -3.7% | 1042 MB | 1042 MB | +0.0% | ~ |
| `BVHUnionMixed` | 11.4 s | 11.5 s | +0.2% | 636 MB | 636 MB | +0.0% | ~ |
| `ClosestPrimitive` | 129.6 s | 107.1 s | -17.3% | 1371 MB | 1371 MB | +0.0% | ~ |
| `ClosestSurface` | 194.4 s | 140.1 s | -28.0% | 1844 MB | 1316 MB | -28.6% | ~ |
| `ConnectedComponents` | 20.7 s | 20.5 s | -1.1% | 951 MB | 951 MB | +0.1% | ~ |
| `Domain` | 60.1 s | 51.2 s | -14.8% | 1006 MB | 1006 MB | -0.0% | ~ |
| `DomainObject` | 58.0 s | 49.7 s | -14.4% | 959 MB | 959 MB | +0.0% | ~ |
| `EBGrid` | 20.4 s | 20.3 s | -0.9% | 766 MB | 766 MB | +0.0% | ~ |
| `Fused` | 56.6 s | 51.7 s | -8.7% | 1178 MB | 1178 MB | +0.0% | ~ |
| `LaunchTuning` | 556.9 s | 456.2 s | -18.1% | 3874 MB | 3133 MB | -19.1% | ~ |
| `Mesh` | 22.0 s | 21.9 s | -0.7% | 824 MB | 824 MB | +0.0% | ~ |
| `MeshCSG` | 428.3 s | 374.0 s | -12.7% | 6308 MB | 5191 MB | -17.7% | ~ |
| `MeshClosestPoint` | 19.5 s | 19.5 s | +0.2% | 776 MB | 776 MB | +0.0% | ~ |
| `MeshIngest` | 10.6 s | 10.5 s | -1.3% | 549 MB | 549 MB | +0.0% | ~ |
| `MeshReconcile` | 10.5 s | 10.5 s | -0.5% | 526 MB | 526 MB | +0.0% | ~ |
| `MeshTape` | 780.7 s | 655.1 s | -16.1% | 6575 MB | 5452 MB | -17.1% | ~ |
| `PointCloud` | 12.9 s | 12.6 s | -2.1% | 578 MB | 578 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 7.3 s | 7.4 s | +2.6% | 524 MB | 524 MB | +0.0% | ~ |
| `PointCloudHashGridBuild` | 6.7 s | 6.5 s | -2.8% | 523 MB | 523 MB | +0.0% | ~ |
| `QueryOrder` | 16.2 s | 16.2 s | -0.1% | 708 MB | 708 MB | +0.0% | ~ |
| `Ray` | 197.7 s | 166.4 s | -15.8% | 3770 MB | 3029 MB | -19.7% | ~ |
| `RayCSG` | 232.3 s | 202.1 s | -13.0% | 4156 MB | 3368 MB | -19.0% | ~ |
| `RayCrossing` | 446.9 s | 390.7 s | -12.6% | 4118 MB | 3300 MB | -19.9% | ~ |
| `RayParticles` | 875.9 s | 427.8 s | -51.2% | 5800 MB | 4847 MB | -16.4% | ~ |
| `RayShell` | 530.1 s | 244.2 s | -53.9% | 5506 MB | 4604 MB | -16.4% | ~ |
| `Regions` | 379.0 s | 317.8 s | -16.1% | 4140 MB | 3364 MB | -18.8% | ~ |
| `Render` | 264.0 s | 236.8 s | -10.3% | 6083 MB | 4920 MB | -19.1% | ~ |
| `Shapes` | 27.7 s | 27.5 s | -0.6% | 599 MB | 599 MB | -0.0% | ~ |
| `SphereCloudBVH` | 9.0 s | 9.3 s | +3.7% | 524 MB | 524 MB | +0.0% | ~ |
| `SphereCloudRays` | 187.0 s | 156.9 s | -16.1% | 4176 MB | 3332 MB | -20.2% | ~ |
| `SphereContacts` | 8.3 s | 8.7 s | +4.7% | 523 MB | 523 MB | +0.0% | ~ |
| `SurfaceParts` | 435.0 s | 337.3 s | -22.5% | 4417 MB | 3493 MB | -20.9% | ~ |
| `Tape` | 67.0 s | 57.5 s | -14.1% | 992 MB | 992 MB | -0.0% | ~ |
| `TapeBVHUnion` | 44.6 s | 41.3 s | -7.4% | 932 MB | 932 MB | +0.0% | ~ |
| **all, in both** | 6175.3 s | 4713.4 s | -23.7% | 6575 MB | 5452 MB | -17.1% | ~ |

Device kernels: 226 in both, 75 changed (9 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 0 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 16960 | 174/220 → 178/216 | **spills** |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> cons…` | 40 → 255 | 17584 → 16512 | 518/916 → 166/216 | **registers** |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, RegionView<Tape<float, TriMeshSDF<fl…` | 168 → 168 | 14768 → 14768 | 2896/2116 → 2904/2136 | **spills** |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 16960 | 174/220 → 178/216 | **spills** |
| `RayCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16848 → 16832 | 160/152 → 164/148 | **spills** |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 168 → 168 | 16960 → 16960 | 174/220 → 178/216 | **spills** |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, false>, Vec3T<double> const, Reg…` | 168 → 168 | 16944 → 16944 | 174/196 → 178/192 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, TriMeshSDF<double…` | 168 → 168 | 7696 → 7648 | 36/120 → 68/120 | **spills** |
| `SurfaceParts` | `void parallelForKernel<128u, 384u, Apply<PointQuery<double, Tape<double, TriMeshSDF<double…` | 168 → 168 | 7712 → 7664 | 28/128 → 60/128 | **spills** |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double, Fused::Node<Fused::Comb…` | 168 → 168 | 9376 → 9328 | 16/4 → 16/4 | ~ |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double, TriMeshSDF<double, 4ul,…` | 168 → 168 | 9392 → 9360 | 20/4 → 20/4 | ~ |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<double, Tape<double> >, Vec3T<double> const,…` | 168 → 168 | 9360 → 9328 | 16/4 → 16/4 | ~ |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float, Fused::Node<Fused::Combin…` | 168 → 168 | 6704 → 6672 | 4/4 → 4/4 | ~ |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float, TriMeshSDF<float, 4ul, 4u…` | 168 → 168 | 6720 → 6640 | 4/4 → 4/4 | ~ |
| `ClosestPrimitive` | `void parallelForKernel<128u, 384u, Apply<Leaf<float, Tape<float> >, Vec3T<float> const, Le…` | 168 → 168 | 6704 → 6656 | 4/4 → 4/4 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, BVHUnion<double, 4ul, …` | 168 → 168 | 7632 → 7600 | 20/4 → 20/4 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, BVHUnion<double, 4ul, …` | 168 → 168 | 7744 → 7680 | 48/152 → 48/152 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, SphereCloudBVH<double,…` | 168 → 168 | 7648 → 7584 | 20/4 → 20/4 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<double, Tape<double, SphereCloudBVH<double,…` | 168 → 168 | 7728 → 7680 | 48/152 → 48/152 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, BVHUnion<float, 4ul, Sph…` | 168 → 168 | 4704 → 4624 | 4/4 → 4/4 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, BVHUnion<float, 4ul, Sph…` | 168 → 168 | 4848 → 4752 | 44/164 → 44/164 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, SphereCloudBVH<float, 4u…` | 168 → 168 | 4704 → 4624 | 4/4 → 4/4 | ~ |
| `ClosestSurface` | `void parallelForKernel<128u, 384u, Apply<Query<float, Tape<float, SphereCloudBVH<float, 4u…` | 168 → 168 | 4816 → 4768 | 44/164 → 44/164 | ~ |
| `Domain` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)0>, Vec3T<float> const, A…` | 168 → 168 | 3072 → 3104 | 180/232 → 180/232 | ~ |
| `DomainObject` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)0>, Vec3T<float> const, A…` | 168 → 168 | 2944 → 2976 | 180/232 → 180/232 | ~ |
| `Fused` | `void parallelForKernel<128u, 384u, Apply<Distance<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 4464 → 4448 | 20/4 → 20/4 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11824 → 11840 | 114/132 → 114/132 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 80 → 80 | 17344 → 17328 | 290/464 → 282/460 | ~ |
| `LaunchTuning` | `void parallelForKernel<128u, 768u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 80 → 80 | 12336 → 12320 | 126/132 → 126/132 | ~ |
| `LaunchTuning` | `void parallelForKernel<256u, 384u, Apply<FirstHit<double, Tape<double> >, Vec3T<double> co…` | 128 → 128 | 17120 → 17152 | 186/220 → 178/216 | ~ |
| `LaunchTuning` | `void parallelForKernelFree<128u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const, …` | 40 → 40 | 12624 → 12608 | 230/368 → 230/368 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17072 → 17040 | 174/220 → 174/220 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11904 → 11888 | 114/132 → 114/132 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19248 → 19248 | 1532/1476 → 1500/1412 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<double, Tape<double, TriMeshSDF<dou…` | 168 → 168 | 16256 → 16240 | 48/36 → 48/36 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, RegionView<Tape<double, TriMeshS…` | 168 → 168 | 16976 → 16960 | 8/8 → 8/8 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17072 → 17040 | 174/220 → 174/220 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, RegionView<Tape<float, TriMeshSDF…` | 168 → 168 | 12064 → 12032 | 8/8 → 8/8 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11888 → 11920 | 114/132 → 114/132 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, RegionView<Tape<double, TriMeshSDF<…` | 168 → 168 | 20464 → 20400 | 2788/2364 → 2788/2348 | ~ |
| `MeshTape` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19248 → 19248 | 1532/1476 → 1500/1412 | ~ |
| `Ray` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11824 → 11840 | 114/132 → 114/132 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<double, Tape<double, TriMeshSDF<dou…` | 168 → 168 | 16256 → 16240 | 48/36 → 48/36 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<double, Tape<double> >, Vec3T<doubl…` | 168 → 168 | 16144 → 16112 | 48/36 → 48/36 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstCrossing<float, Tape<float> >, Vec3T<float> …` | 168 → 168 | 11392 → 11376 | 40/28 → 40/28 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TriMeshSDF<double, …` | 168 → 168 | 17072 → 17040 | 174/220 → 174/220 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TriMeshSDF<float, 4ul…` | 168 → 168 | 11888 → 11920 | 114/132 → 114/132 | ~ |
| `RayCrossing` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float> >, Vec3T<float> const…` | 168 → 168 | 11824 → 11840 | 114/132 → 114/132 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 18912 → 18928 | 246/208 → 246/208 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<double, Tape<double, TapeFeature::ExactOf…` | 168 → 168 | 18736 → 18720 | 246/208 → 246/208 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13344 → 13328 | 178/160 → 178/160 | ~ |
| `RayParticles` | `void parallelForKernel<128u, 384u, Apply<Collide<float, Tape<float, TapeFeature::ExactOffs…` | 168 → 168 | 13136 → 13152 | 178/160 → 178/160 | ~ |
| `RayShell` | `void parallelForKernel<128u, 384u, Apply<FirstHit<double, Tape<double, TapeFeature::ExactO…` | 168 → 168 | 18896 → 18912 | 236/224 → 236/224 | ~ |
| `RayShell` | `void parallelForKernel<128u, 384u, Apply<FirstHit<float, Tape<float, TapeFeature::ExactOff…` | 168 → 168 | 13344 → 13328 | 172/176 → 172/176 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)1>, Vec3T<double> const,…` | 168 → 168 | 4176 → 4160 | 340/352 → 340/352 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<double, (Operation)2>, Vec3T<double> const,…` | 168 → 168 | 4128 → 4144 | 44/28 → 44/28 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)1>, Vec3T<float> const, R…` | 168 → 168 | 2912 → 2944 | 236/268 → 236/268 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)2>, Vec3T<float> const, R…` | 168 → 168 | 2864 → 2880 | 32/20 → 32/20 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<Query<float, (Operation)3>, Vec3T<float> const, R…` | 168 → 168 | 2944 → 2976 | 180/232 → 180/232 | ~ |
| `Regions` | `void parallelForKernel<128u, 384u, Apply<RayQuery<double, true>, Vec3T<double> const, Regi…` | 168 → 168 | 16832 → 16816 | 8/8 → 8/8 | ~ |

15 more changed kernels are in the records.


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (9 cases): ClosestPrimitive/parts/closestPrimitive/float/hostdevice, Domain/particles/labelAt/double/hostdevice, DomainObject/partsWhole/labels/float/hostdevice, DomainObject/partsWhole/labelAt/float/hostdevice, DomainObject/partsOwn/labels/float/hostdevice, DomainObject/partsOwn/labelAt/float/hostdevice, RayParticles/armadillo-shell-fused/step-morton/double/hostdevice, RayShell/armadillo-tape/firstHit-morton/double/hostdevice, Regions/particles/regionByRegion/float/hostdevice

These probes compiled to the same code on both sides and were not run (140 cases): BVH (12), BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), EBGrid (12), Mesh (8), MeshClosestPoint (4), MeshReconcile (2), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), QueryOrder (6), Shapes (20), SphereCloudBVH (8), SphereContacts (2)

<!-- END receipt cuda -->
