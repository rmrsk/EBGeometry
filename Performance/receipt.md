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
base: a3d9443
base-fingerprint: ccacc81e220bf9f8b02e3f92a267fc124862d6b0
head: cd912d0
head-fingerprint: 1af8bc3b9e684b83ed82fa8ad34ba45b57fd75e2
cases: 158
rounds: 2
reps: 3
date: 2026-11-08
-->

## Host comparison: yellow

**Accepted:** Price of the algorithm

`cd912d0` against `a3d9443` (origin/main), 2026-11-08.

**VERDICT: 10 minor regressions; look at them before accepting (see Flagged above)**

Performance comparison: 0 major, 10 minor, 4 faster, 0 differ, 0 lost, 158 cases

- base: a3d9443 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: cd912d0 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHRefit/uniform/refit/float/host` | 41.7 ms | 42.0 ms | +0.7% | [-0.4%, +1.8%] | +0.0% | ~ |  |
| `BVHRefit/uniform/refit/double/host` | 76.2 ms | 75.6 ms | -0.8% | [-1.3%, -0.4%] | +0.0% | ~ |  |
| `BVHUnion/spheres/build/float/host` | 55.0 ms | 56.1 ms | +2.1% | [+1.9%, +2.4%] | +6.0% | minor | pool bytes up |
| `BVHUnion/spheres/query-random/float/host` | 51.3 ms | 51.3 ms | +0.0% | [-0.3%, +0.2%] | +6.0% | minor | pool bytes up |
| `BVHUnion/spheres/build/double/host` | 64.9 ms | 64.5 ms | -0.7% | [-1.0%, -0.4%] | +3.8% | minor | pool bytes up |
| `BVHUnion/spheres/query-random/double/host` | 54.8 ms | 55.0 ms | +0.3% | [-0.3%, +1.1%] | +3.8% | minor | pool bytes up |
| `BVHUnion/capsules-diagonal/build/float/host` | 60.3 ms | 60.5 ms | +0.3% | [+0.2%, +0.6%] | +4.9% | minor | pool bytes up |
| `BVHUnion/capsules-diagonal/query-random/float/host` | 72.4 ms | 72.3 ms | -0.1% | [-0.3%, +0.1%] | +4.9% | minor | pool bytes up |
| `BVHUnion/capsules-diagonal/build/double/host` | 74.1 ms | 71.9 ms | -2.9% | [-3.3%, -2.7%] | +3.0% | minor | pool bytes up |
| `BVHUnion/capsules-diagonal/query-random/double/host` | 77.5 ms | 77.6 ms | +0.2% | [-0.5%, +0.4%] | +3.0% | minor | pool bytes up |
| `BVHUnion/cylinders-random/build/float/host` | 66.9 ms | 64.4 ms | -3.8% | [-4.1%, -3.5%] | +3.8% | minor | pool bytes up |
| `BVHUnion/cylinders-random/query-random/float/host` | 100 ms | 101 ms | +0.5% | [-0.3%, +0.8%] | +3.8% | minor | pool bytes up |
| `BVHUnion/cylinders-random/build/double/host` | 83.2 ms | 76.8 ms | -7.6% | [-7.7%, -7.5%] | +2.2% | faster |  |
| `BVHUnion/cylinders-random/query-random/double/host` | 109 ms | 108 ms | -0.4% | [-0.9%, +0.0%] | +2.2% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/float/host` | 64.0 ms | 64.1 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/host` | 95.6 ms | 95.6 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/build/double/host` | 76.2 ms | 76.2 ms | -0.0% | [-0.3%, +0.3%] | +0.0% | ~ |  |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/host` | 101 ms | 102 ms | +0.9% | [+0.3%, +1.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.5 ms | 19.4 ms | -0.8% | [-1.7%, +0.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.0 ms | 41.3 ms | +0.8% | [+0.5%, +1.2%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 72.6 ms | 72.5 ms | -0.1% | [-0.2%, -0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.2 ms | 20.2 ms | +0.1% | [-0.3%, +0.6%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 43.6 ms | 43.5 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 81.2 ms | 80.9 ms | -0.4% | [-1.1%, +0.2%] | +0.0% | ~ |  |
| `ClosestSurface/cloud/closestSurface/float/host` | - | - | - | - | - | new |  |
| `ClosestSurface/cloud/closestLeaf/float/host` | - | - | - | - | - | new | surface/leaf 1.84x |
| `ClosestSurface/union/closestSurface/float/host` | - | - | - | - | - | new |  |
| `ClosestSurface/union/closestLeaf/float/host` | - | - | - | - | - | new | surface/leaf 1.90x |
| `ClosestSurface/cloud/closestSurface/double/host` | - | - | - | - | - | new |  |
| `ClosestSurface/cloud/closestLeaf/double/host` | - | - | - | - | - | new | surface/leaf 1.89x |
| `ClosestSurface/union/closestSurface/double/host` | - | - | - | - | - | new |  |
| `ClosestSurface/union/closestLeaf/double/host` | - | - | - | - | - | new | surface/leaf 1.91x |
| `Fused/csg/signedDistance/float/host` | 4.52 ms | 4.46 ms | -1.4% | [-2.7%, -0.0%] | - | ~ | tape/fused 2.90x |
| `Fused/smooth/signedDistance/float/host` | 12.3 ms | 12.3 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | tape/fused 2.01x |
| `Fused/chain16/signedDistance/float/host` | 979 µs | 981 µs | +0.2% | [-1.7%, +2.0%] | - | ~ | tape/fused 16.61x |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.0 ms | 46.0 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.4 ms | 46.3 ms | -0.2% | [-0.5%, +0.1%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.60 ms | 4.64 ms | +1.0% | [+0.4%, +1.6%] | - | ~ | tape/fused 3.56x (was 2.85x) |
| `Fused/smooth/signedDistance/double/host` | 12.8 ms | 12.8 ms | -0.1% | [-0.4%, +0.3%] | - | ~ | tape/fused 2.03x |
| `Fused/chain16/signedDistance/double/host` | 3.87 ms | 3.88 ms | +0.4% | [-0.4%, +1.2%] | - | ~ | tape/fused 4.24x |
| `Fused/drilled-mesh/signedDistance/double/host` | 52.5 ms | 52.5 ms | +0.2% | [-0.2%, +0.5%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 52.4 ms | 53.1 ms | +1.4% | [+1.1%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.5 ms | 26.7 ms | +0.8% | [+0.7%, +1.0%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 26.7 ms | 27.0 ms | +0.9% | [+0.6%, +1.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 319 ms | 318 ms | -0.3% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 238 ms | 238 ms | -0.1% | [-0.3%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 91.0 ms | 90.2 ms | -0.9% | [-2.0%, +0.2%] | +0.0% | ~ | tape/fused 3.53x |
| `MeshCSG/smooth-fused/frame/float/host` | 99.7 ms | 99.8 ms | +0.2% | [-0.3%, +0.6%] | +0.0% | ~ | tape/fused 2.39x |
| `MeshCSG/union-tape/firstHit/double/host` | 29.3 ms | 29.5 ms | +0.7% | [+0.4%, +0.9%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 30.0 ms | 30.0 ms | -0.1% | [-1.8%, +1.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 355 ms | 354 ms | -0.4% | [-1.0%, +0.2%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 254 ms | 253 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 96.6 ms | 97.0 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ | tape/fused 3.65x |
| `MeshCSG/smooth-fused/frame/double/host` | 109 ms | 107 ms | -2.1% | [-4.1%, -0.1%] | +0.0% | ~ | tape/fused 2.38x |
| `MeshReconcile/armadillo/reconcile/float/host` | 131 ms | 131 ms | +0.0% | [-0.4%, +0.5%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.3 ms | 41.4 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | reconcile/scheduled 3.17x |
| `MeshReconcile/armadillo/reconcile/double/host` | 155 ms | 156 ms | +0.2% | [-1.2%, +1.6%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 56.3 ms | 56.0 ms | -0.5% | [-0.9%, -0.2%] | +0.0% | ~ | reconcile/scheduled 2.78x |
| `Ray/mesh/firstHit/float/host` | 46.4 ms | 46.3 ms | -0.2% | [-0.4%, +0.0%] | +0.0% | ~ | hand/api 0.99x |
| `Ray/mesh/firstHit-handrolled/float/host` | 45.6 ms | 45.7 ms | +0.4% | [-0.2%, +0.9%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 40.4 ms | 40.4 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 78.6 ms | 78.4 ms | -0.3% | [-0.5%, +0.0%] | +0.0% | ~ | triangles/polygons 0.59x |
| `Ray/mesh-polygons/intersects/float/host` | 65.5 ms | 65.5 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.21 ms | 5.18 ms | -0.6% | [-1.1%, -0.2%] | - | ~ | tape/fused 1.48x |
| `Ray/csg-tape/firstHit/float/host` | 7.79 ms | 7.65 ms | -1.8% | [-3.8%, +0.2%] | +0.0% | ~ |  |
| `Ray/mesh/firstHit/double/host` | 52.1 ms | 52.9 ms | +1.6% | [+1.0%, +2.2%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/double/host` | 51.5 ms | 51.5 ms | +0.0% | [-0.6%, +0.6%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 44.9 ms | 45.0 ms | +0.1% | [-0.5%, +0.8%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 88.4 ms | 88.7 ms | +0.3% | [-1.3%, +1.9%] | +0.0% | ~ | triangles/polygons 0.60x |
| `Ray/mesh-polygons/intersects/double/host` | 75.2 ms | 74.1 ms | -1.5% | [-2.9%, -0.1%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.52 ms | 5.50 ms | -0.4% | [-0.5%, -0.4%] | - | ~ | tape/fused 1.42x (was 1.48x) |
| `Ray/csg-tape/firstHit/double/host` | 8.18 ms | 7.81 ms | -4.5% | [-6.2%, -2.8%] | +0.0% | faster |  |
| `RayCSG/scene-fused/firstHit/float/host` | 17.7 ms | 17.8 ms | +0.6% | [-0.5%, +1.6%] | - | ~ | marched/traced 6.67x |
| `RayCSG/scene-fused/firstHit-marched/float/host` | 120 ms | 119 ms | -0.7% | [-0.8%, -0.6%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/float/host` | 21.6 ms | 22.3 ms | +2.9% | [+2.8%, +2.9%] | +0.0% | ~ | marched/traced 6.50x (was 6.87x) |
| `RayCSG/scene-tape/firstHit-marched/float/host` | 149 ms | 145 ms | -2.7% | [-2.8%, -2.5%] | +0.0% | ~ |  |
| `RayCSG/crowd-tape/firstHit/float/host` | 35.2 ms | 35.5 ms | +0.9% | [+0.7%, +1.0%] | +0.0% | ~ |  |
| `RayCSG/scene-fused/firstHit/double/host` | 19.0 ms | 19.0 ms | +0.4% | [-0.7%, +1.4%] | - | ~ | marched/traced 6.64x |
| `RayCSG/scene-fused/firstHit-marched/double/host` | 127 ms | 126 ms | -0.2% | [-0.4%, -0.0%] | - | ~ |  |
| `RayCSG/scene-tape/firstHit/double/host` | 23.4 ms | 23.4 ms | +0.1% | [-0.5%, +0.8%] | +0.0% | ~ | marched/traced 6.41x (was 6.69x) |
| `RayCSG/scene-tape/firstHit-marched/double/host` | 156 ms | 150 ms | -3.9% | [-4.7%, -3.2%] | +0.0% | faster |  |
| `RayCSG/crowd-tape/firstHit/double/host` | 37.9 ms | 37.8 ms | -0.2% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 46.4 ms | 46.3 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 46.2 ms | 46.4 ms | +0.4% | [+0.0%, +0.7%] | +0.0% | ~ | hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.61 ms | 7.80 ms | +2.6% | [+0.2%, +5.0%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.08 ms | 6.10 ms | +0.4% | [+0.2%, +0.5%] | +0.0% | ~ | hit/crossing 1.28x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 176 ms | 176 ms | +0.0% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 157 ms | 157 ms | -0.0% | [-0.1%, +0.0%] | +0.0% | ~ | hit/crossing 1.13x |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 68.9 ms | 68.6 ms | -0.4% | [-0.6%, -0.1%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 56.6 ms | 56.3 ms | -0.4% | [-0.6%, -0.2%] | +0.0% | ~ | hit/crossing 1.22x |
| `RayCrossing/mesh/firstHit/double/host` | 52.7 ms | 52.2 ms | -0.9% | [-1.3%, -0.5%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 53.0 ms | 54.8 ms | +3.5% | [-2.8%, +10.0%] | +0.0% | ~ | hit/crossing 0.95x (was 1.00x) |
| `RayCrossing/csg-tape/firstHit/double/host` | 7.88 ms | 7.85 ms | -0.3% | [-2.3%, +1.7%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 6.23 ms | 6.21 ms | -0.4% | [-1.3%, +0.6%] | +0.0% | ~ | hit/crossing 1.27x |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 204 ms | 202 ms | -0.9% | [-2.2%, +0.6%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 180 ms | 183 ms | +1.7% | [-0.6%, +4.0%] | +0.0% | ~ | hit/crossing 1.10x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 78.1 ms | 80.7 ms | +3.3% | [-1.1%, +7.7%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 64.7 ms | 64.8 ms | +0.3% | [-0.4%, +1.0%] | +0.0% | ~ | hit/crossing 1.24x (was 1.21x) |
| `RayParticles/armadillo-shell-tape/step/float/host` | 11.0 ms | 11.1 ms | +0.1% | [-0.4%, +0.7%] | +0.0% | ~ | marched/traced 4.68x |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 52.2 ms | 51.8 ms | -0.6% | [-2.2%, +0.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.27 ms | 8.11 ms | -1.9% | [-5.9%, +2.3%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 1.93 ms | 1.93 ms | +0.5% | [-1.6%, +2.5%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 6.88 ms | 6.96 ms | +1.2% | [+0.8%, +1.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.0 ms | 11.8 ms | -1.7% | [-5.6%, +2.5%] | +0.0% | ~ | marched/traced 5.18x |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 60.7 ms | 61.1 ms | +0.6% | [-0.4%, +1.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.55 ms | 8.57 ms | +0.3% | [-0.1%, +0.6%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 1.92 ms | 1.96 ms | +1.7% | [-0.5%, +3.9%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.19 ms | 7.15 ms | -0.5% | [-0.6%, -0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 237 ms | 237 ms | +0.2% | [+0.1%, +0.4%] | +0.0% | ~ | marched/traced 1.38x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 328 ms | 328 ms | +0.0% | [-0.5%, +0.5%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 236 ms | 237 ms | +0.2% | [-0.2%, +0.7%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 254 ms | 250 ms | -1.6% | [-1.7%, -1.5%] | +0.0% | ~ | marched/traced 1.47x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 367 ms | 367 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 251 ms | 248 ms | -1.2% | [-1.4%, -1.0%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 19.0 ms | 19.1 ms | +0.6% | [+0.3%, +0.9%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 19.1 ms | 19.0 ms | -1.0% | [-2.1%, +0.1%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 18.6 ms | 18.9 ms | +1.8% | [+1.0%, +2.6%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 27.0 ms | 21.3 ms | -21.3% | [-35.6%, +1.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 21.1 ms | 21.3 ms | +0.9% | [+0.3%, +1.5%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.5 ms | 20.6 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/float/host` | 56.8 ms | 56.8 ms | +0.1% | [-0.3%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/float/host` | 36.6 ms | 36.6 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/float/host` | 92.4 ms | 92.2 ms | -0.3% | [-0.4%, -0.1%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/build/double/host` | 64.2 ms | 64.3 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/rebuild/double/host` | 42.7 ms | 42.8 ms | +0.1% | [-0.6%, +0.8%] | +0.0% | ~ |  |
| `SphereCloudBVH/polydisperse/closest-random/double/host` | 107 ms | 107 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/float/host` | 74.7 ms | 74.7 ms | -0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/float/host` | 295 ms | 295 ms | +0.1% | [-0.2%, +0.3%] | +0.0% | ~ | tape/fused 1.00x |
| `SphereCloudRays/bed-in-box/firstHit/float/host` | 296 ms | 297 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ |  |
| `SphereCloudRays/polydisperse/firstHit/double/host` | 86.5 ms | 86.5 ms | +0.0% | [-0.4%, +0.4%] | +0.0% | ~ |  |
| `SphereCloudRays/bed-in-box-fused/firstHit/double/host` | 345 ms | 343 ms | -0.5% | [-0.8%, -0.3%] | +0.0% | ~ | tape/fused 1.01x |
| `SphereCloudRays/bed-in-box/firstHit/double/host` | 347 ms | 348 ms | +0.0% | [-0.6%, +0.7%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/float/host` | 167 ms | 167 ms | -0.1% | [-0.7%, +0.5%] | +0.0% | ~ |  |
| `SphereContacts/polydisperse/force-morton/double/host` | 191 ms | 190 ms | -0.5% | [-0.8%, -0.2%] | +0.0% | ~ |  |
| `Tape/csg/signedDistance/float/host` | 12.9 ms | 12.9 ms | +0.5% | [-0.7%, +1.6%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/float/host` | 4.56 ms | 4.59 ms | +0.7% | [+0.1%, +1.2%] | - | ~ |  |
| `Tape/smooth/signedDistance/float/host` | 24.9 ms | 24.7 ms | -0.5% | [-1.1%, +0.1%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/float/host` | 40.0 ms | 39.9 ms | -0.1% | [-0.4%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/float/host` | 1.75 ms | 1.75 ms | +0.0% | [-1.3%, +1.4%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/float/host` | 134 µs | 137 µs | +3.0% | [-2.9%, +9.4%] | - | ~ |  |
| `Tape/sphere/signedDistance-x16/float/host` | 29.9 ms | 29.9 ms | +0.1% | [-0.1%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/float/host` | 5.68 ms | 5.69 ms | +0.3% | [-0.3%, +0.8%] | - | ~ |  |
| `Tape/chain16/signedDistance/float/host` | 16.2 ms | 16.3 ms | +0.6% | [+0.5%, +0.7%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/float/host` | 4.67 ms | 4.66 ms | -0.1% | [-0.9%, +0.7%] | - | ~ |  |
| `Tape/csg/signedDistance/double/host` | 13.1 ms | 16.5 ms | +26.0% | [-0.7%, +52.8%] | +0.0% | ~ |  |
| `Tape/csg-handwritten/signedDistance/double/host` | 4.60 ms | 4.58 ms | -0.4% | [-2.7%, +1.9%] | - | ~ |  |
| `Tape/smooth/signedDistance/double/host` | 26.4 ms | 26.0 ms | -1.3% | [-1.8%, -0.8%] | +0.0% | ~ |  |
| `Tape/union64/signedDistance/double/host` | 49.6 ms | 41.7 ms | -15.9% | [-27.5%, +0.2%] | +0.0% | ~ |  |
| `Tape/sphere/signedDistance/double/host` | 1.76 ms | 1.75 ms | -0.4% | [-1.8%, +1.0%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance/double/host` | 302 µs | 271 µs | -10.3% | [-16.9%, -3.1%] | - | faster |  |
| `Tape/sphere/signedDistance-x16/double/host` | 29.7 ms | 29.9 ms | +0.7% | [+0.4%, +0.9%] | +0.0% | ~ |  |
| `Tape/sphere-handwritten/signedDistance-x16/double/host` | 7.72 ms | 7.74 ms | +0.3% | [+0.2%, +0.4%] | - | ~ |  |
| `Tape/chain16/signedDistance/double/host` | 16.2 ms | 16.5 ms | +1.7% | [+0.9%, +2.5%] | +0.0% | ~ |  |
| `Tape/chain16-handwritten/signedDistance/double/host` | 7.77 ms | 7.75 ms | -0.3% | [-0.7%, +0.1%] | - | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/host` | 41.9 ms | 41.8 ms | -0.2% | [-0.6%, +0.1%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/host` | 218 ms | 216 ms | -0.9% | [-1.2%, -0.7%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/host` | 42.6 ms | 42.6 ms | +0.0% | [-0.2%, +0.3%] | +0.0% | ~ |  |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/host` | 213 ms | 213 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ |  |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 461.9 s → 464.8 s (+0.6%), the largest peak 1282 MB → 1284 MB; 1 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 8.8 s | 8.8 s | -0.0% | 557 MB | 555 MB | -0.3% | ~ |
| `BVHRefit` | 3.8 s | 3.8 s | +1.0% | 457 MB | 458 MB | +0.0% | ~ |
| `BVHUnion` | 14.6 s | 17.7 s | +21.1% | 815 MB | 821 MB | +0.7% | **minor** |
| `BVHUnionMixed` | 7.1 s | 7.1 s | +0.3% | 554 MB | 554 MB | -0.0% | ~ |
| `ClosestPrimitive` | 25.1 s | 25.2 s | +0.4% | 1218 MB | 1220 MB | +0.2% | ~ |
| `ClosestSurface` | - | 24.4 s | | - | 1125 MB | | new |
| `Fused` | 20.9 s | 20.8 s | -0.4% | 1014 MB | 1020 MB | +0.6% | ~ |
| `LaunchTuning` | 32.9 s | 32.9 s | -0.1% | 1121 MB | 1123 MB | +0.2% | ~ |
| `Mesh` | 14.0 s | 14.0 s | -0.2% | 769 MB | 771 MB | +0.3% | ~ |
| `MeshCSG` | 34.4 s | 34.3 s | -0.3% | 1168 MB | 1170 MB | +0.2% | ~ |
| `MeshReconcile` | 6.1 s | 6.0 s | -0.6% | 537 MB | 537 MB | +0.1% | ~ |
| `PointCloud` | 6.9 s | 6.9 s | -0.6% | 557 MB | 557 MB | +0.0% | ~ |
| `PointCloudBVHBuild` | 3.5 s | 3.4 s | -1.1% | 413 MB | 414 MB | +0.3% | ~ |
| `PointCloudHashGridBuild` | 3.0 s | 2.9 s | -2.3% | 403 MB | 402 MB | -0.3% | ~ |
| `Ray` | 32.7 s | 32.1 s | -1.7% | 1161 MB | 1164 MB | +0.2% | ~ |
| `RayCSG` | 29.3 s | 29.1 s | -0.9% | 980 MB | 982 MB | +0.2% | ~ |
| `RayCrossing` | 40.6 s | 40.7 s | +0.2% | 1282 MB | 1283 MB | +0.1% | ~ |
| `RayParticles` | 44.1 s | 43.8 s | -0.7% | 1282 MB | 1284 MB | +0.2% | ~ |
| `RayShell` | 33.6 s | 33.5 s | -0.4% | 1074 MB | 1076 MB | +0.1% | ~ |
| `Render` | 31.9 s | 31.8 s | -0.3% | 1127 MB | 1128 MB | +0.1% | ~ |
| `Shapes` | 8.8 s | 9.0 s | +1.8% | 552 MB | 551 MB | -0.1% | ~ |
| `SphereCloudBVH` | 4.6 s | 4.7 s | +2.4% | 488 MB | 488 MB | +0.1% | ~ |
| `SphereCloudRays` | 24.6 s | 25.4 s | +3.4% | 975 MB | 978 MB | +0.3% | ~ |
| `SphereContacts` | 4.0 s | 4.1 s | +1.9% | 430 MB | 454 MB | +5.6% | ~ |
| `Tape` | 14.5 s | 14.5 s | +0.1% | 827 MB | 820 MB | -0.8% | ~ |
| `TapeBVHUnion` | 12.1 s | 12.3 s | +1.7% | 801 MB | 804 MB | +0.5% | ~ |
| **all, in both** | 461.9 s | 464.8 s | +0.6% | 1282 MB | 1284 MB | +0.2% | ~ |


The base's build was reused from the cache (the same code, compilers and settings); its build costs are those recorded when it was built.

Flagged after 2 rounds, and judged on 10 (10 cases): BVHUnion/spheres/build/float/host, BVHUnion/spheres/query-random/float/host, BVHUnion/spheres/build/double/host, BVHUnion/spheres/query-random/double/host, BVHUnion/capsules-diagonal/build/float/host, BVHUnion/capsules-diagonal/query-random/float/host, BVHUnion/capsules-diagonal/build/double/host, BVHUnion/capsules-diagonal/query-random/double/host, BVHUnion/cylinders-random/build/float/host, BVHUnion/cylinders-random/query-random/float/host

These probes compiled to the same code on both sides and were not run (82 cases): BVH (6), Mesh (12), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), Shapes (20)

The probes differ between the sides (each side ran its own): Performance/ClosestPrimitive/main.cpp, Performance/ClosestSurface/main.cpp

<!-- END receipt host -->

<!-- BEGIN receipt cuda -->
<!--receipt
version: 3
side: cuda
verdict: green
base: a3d9443
base-fingerprint: ccacc81e220bf9f8b02e3f92a267fc124862d6b0
head: cd912d0
head-fingerprint: 1af8bc3b9e684b83ed82fa8ad34ba45b57fd75e2
cases: 250
rounds: 2
reps: 3
date: 2026-11-08
-->

## Device comparison (CUDA): green

**Accepted:** -

`cd912d0` against `a3d9443` (origin/main), 2026-11-08.

**VERDICT: no regression (0 major, 0 minor across 250 cases)**

Performance comparison: 0 major, 0 minor, 4 faster, 0 differ, 0 lost, 250 cases

- base: a3d9443 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- head: cd912d0 (gcc 13.3.0, simd none, gpu CUDA, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVHRefit/uniform/refit/float/hostdevice` | 9.88 ms | 8.31 ms | -15.9% | [-27.3%, -0.4%] | +0.0% | faster | device 4.9x faster than the host (was 4.2x) |
| `BVHRefit/uniform/refit/double/hostdevice` | 9.81 ms | 9.95 ms | +1.5% | [+0.9%, +2.1%] | +0.0% | ~ | device 8.1x faster than the host |
| `BVHUnion/spheres/query-morton/float/hostdevice` | 387 µs | 386 µs | -0.1% | [-1.1%, +0.8%] | - | ~ | device 15.1x faster than the host |
| `BVHUnion/spheres/query-random/float/hostdevice` | 840 µs | 839 µs | -0.1% | [-0.6%, +0.4%] | - | ~ | device 10.1x faster than the host |
| `BVHUnion/spheres/query-morton/double/hostdevice` | 4.53 ms | 4.51 ms | -0.4% | [-0.5%, -0.2%] | - | ~ | device 1.3x faster than the host |
| `BVHUnion/spheres/query-random/double/hostdevice` | 4.66 ms | 4.67 ms | +0.3% | [-0.3%, +0.9%] | - | ~ | device 1.9x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/float/hostdevice` | 580 µs | 576 µs | -0.7% | [-1.1%, -0.2%] | - | ~ | device 13.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/float/hostdevice` | 1.36 ms | 1.37 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 8.3x faster than the host |
| `BVHUnion/capsules-diagonal/query-morton/double/hostdevice` | 6.76 ms | 6.76 ms | -0.1% | [-0.1%, -0.0%] | - | ~ | device 1.1x faster than the host |
| `BVHUnion/capsules-diagonal/query-random/double/hostdevice` | 6.99 ms | 7.03 ms | +0.5% | [+0.4%, +0.6%] | - | ~ | device 1.7x faster than the host |
| `BVHUnion/cylinders-random/query-morton/float/hostdevice` | 894 µs | 896 µs | +0.2% | [-0.1%, +0.5%] | - | ~ | device 14.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/float/hostdevice` | 2.21 ms | 2.21 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 8.0x faster than the host |
| `BVHUnion/cylinders-random/query-morton/double/hostdevice` | 9.48 ms | 9.45 ms | -0.3% | [-0.5%, -0.1%] | - | ~ | device 1.4x faster than the host |
| `BVHUnion/cylinders-random/query-random/double/hostdevice` | 10.1 ms | 10.1 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.8x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/float/hostdevice` | 842 µs | 843 µs | +0.1% | [-0.2%, +0.3%] | - | ~ | device 13.6x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/float/hostdevice` | 2.03 ms | 2.04 ms | +0.2% | [+0.1%, +0.2%] | - | ~ | device 8.1x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-morton/double/hostdevice` | 9.11 ms | 9.10 ms | -0.1% | [-0.5%, +0.3%] | - | ~ | device 1.3x faster than the host |
| `BVHUnionMixed/spheres-capsules-cylinders/query-random/double/hostdevice` | 9.82 ms | 9.80 ms | -0.2% | [-0.4%, -0.0%] | - | ~ | device 1.7x faster than the host |
| `ClosestPrimitive/csg/closestPrimitive/float/hostdevice` | 313 µs | 314 µs | +0.4% | [+0.3%, +0.5%] | - | ~ | device 11.5x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/float/hostdevice` | 1.30 ms | 1.30 ms | -0.2% | [-0.2%, -0.1%] | - | ~ | device 6.2x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/hostdevice` | 58.9 ms | 58.9 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 3.4x faster than the host (was 3.8x) |
| `ClosestPrimitive/csg/closestPrimitive/double/hostdevice` | 1.31 ms | 1.31 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.9x faster than the host |
| `ClosestPrimitive/parts/closestPrimitive/double/hostdevice` | 5.38 ms | 5.39 ms | +0.1% | [-0.1%, +0.3%] | - | ~ | device 1.6x faster than the host |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/hostdevice` | 518 ms | 518 ms | +0.1% | [-0.5%, +0.6%] | - | ~ | device 0.4x faster than the host |
| `ClosestSurface/cloud/closestSurface/float/hostdevice` | - | - | - | - | - | new |  |
| `ClosestSurface/cloud/closestLeaf/float/hostdevice` | - | - | - | - | - | new | surface/leaf 2.41x |
| `ClosestSurface/union/closestSurface/float/hostdevice` | - | - | - | - | - | new |  |
| `ClosestSurface/union/closestLeaf/float/hostdevice` | - | - | - | - | - | new | surface/leaf 2.49x |
| `ClosestSurface/cloud/closestSurface/double/hostdevice` | - | - | - | - | - | new |  |
| `ClosestSurface/cloud/closestLeaf/double/hostdevice` | - | - | - | - | - | new | surface/leaf 2.42x |
| `ClosestSurface/union/closestSurface/double/hostdevice` | - | - | - | - | - | new |  |
| `ClosestSurface/union/closestLeaf/double/hostdevice` | - | - | - | - | - | new | surface/leaf 2.54x |
| `Fused/csg/signedDistance/float/hostdevice` | 47.9 µs | 47.9 µs | -0.0% | [-0.2%, +0.1%] | - | ~ | device 24.1x faster than the host; tape/fused 4.56x |
| `Fused/smooth/signedDistance/float/hostdevice` | 49.9 µs | 49.9 µs | -0.2% | [-0.3%, -0.0%] | - | ~ | device 43.2x faster than the host (was 44.8x); tape/fused 7.20x |
| `Fused/chain16/signedDistance/float/hostdevice` | 48.0 µs | 47.9 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 9.6x faster than the host; tape/fused 9.94x |
| `Fused/drilled-mesh/signedDistance/float/hostdevice` | 17.3 ms | 17.3 ms | -0.3% | [-0.8%, +0.3%] | - | ~ | device 7.5x faster than the host; tape/fused 1.81x |
| `Fused/drilled-mesh-tape/signedDistance/float/hostdevice` | 31.1 ms | 31.2 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 3.7x faster than the host |
| `Fused/csg/signedDistance/double/hostdevice` | 810 µs | 810 µs | -0.0% | [-0.0%, +0.0%] | - | ~ | device 1.5x faster than the host; tape/fused 1.06x |
| `Fused/smooth/signedDistance/double/hostdevice` | 1.55 ms | 1.55 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 1.6x faster than the host; tape/fused 1.12x |
| `Fused/chain16/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 0.6x faster than the host; tape/fused 1.01x |
| `Fused/drilled-mesh/signedDistance/double/hostdevice` | 282 ms | 281 ms | -0.5% | [-2.9%, +1.9%] | - | ~ | device 0.4x faster than the host; tape/fused 1.03x |
| `Fused/drilled-mesh-tape/signedDistance/double/hostdevice` | 285 ms | 289 ms | +1.5% | [+0.5%, +2.4%] | - | ~ | device 0.4x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r384/float/hostdevice` | 8.31 ms | 8.24 ms | -0.8% | [-1.2%, -0.4%] | +0.0% | ~ | device 10.7x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/float/hostdevice` | 8.35 ms | 8.25 ms | -1.2% | [-2.3%, -0.2%] | +0.0% | ~ | device 10.7x faster than the host (was 11.6x); default/this 1.00x |
| `LaunchTuning/mesh/signedDistance-b128-r768/float/hostdevice` | 8.25 ms | 8.19 ms | -0.7% | [-1.1%, -0.3%] | +0.0% | ~ | device 10.8x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b256-r384/float/hostdevice` | 8.28 ms | 8.22 ms | -0.8% | [-1.1%, -0.5%] | +0.0% | ~ | device 11.9x faster than the host (was 10.8x); default/this 1.00x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/float/hostdevice` | 677 µs | 679 µs | +0.2% | [+0.1%, +0.3%] | +0.0% | ~ | device 3.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/float/hostdevice` | 1.75 ms | 1.75 ms | -0.2% | [-0.5%, +0.0%] | +0.0% | ~ | device 1.5x faster than the host; default/this 0.39x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/float/hostdevice` | 1.03 ms | 1.03 ms | -0.0% | [-0.8%, +0.8%] | +0.0% | ~ | device 2.5x faster than the host; default/this 0.66x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/float/hostdevice` | 811 µs | 810 µs | -0.0% | [-0.7%, +0.6%] | +0.0% | ~ | device 3.2x faster than the host; default/this 0.84x |
| `LaunchTuning/cloud/closestPoint-b128-r384/float/hostdevice` | 735 µs | 737 µs | +0.2% | [-0.4%, +0.9%] | +0.0% | ~ | device 12.0x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/float/hostdevice` | 826 µs | 830 µs | +0.4% | [-0.3%, +1.1%] | +0.0% | ~ | device 10.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r768/float/hostdevice` | 738 µs | 740 µs | +0.3% | [-0.3%, +1.0%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/float/hostdevice` | 734 µs | 728 µs | -0.9% | [-1.5%, -0.3%] | +0.0% | ~ | device 12.0x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r384/double/hostdevice` | 160 ms | 163 ms | +2.0% | [-0.5%, +4.5%] | +0.0% | ~ | device 0.5x faster than the host |
| `LaunchTuning/mesh/signedDistance-b128-r0/double/hostdevice` | 158 ms | 161 ms | +2.2% | [-0.2%, +4.6%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/mesh/signedDistance-b128-r768/double/hostdevice` | 162 ms | 165 ms | +2.1% | [-0.3%, +4.5%] | +0.0% | ~ | device 0.5x faster than the host; default/this 0.99x |
| `LaunchTuning/mesh/signedDistance-b256-r384/double/hostdevice` | 163 ms | 162 ms | -0.9% | [-2.3%, +0.5%] | +0.0% | ~ | device 0.6x faster than the host; default/this 1.01x |
| `LaunchTuning/csg-tape/firstHit-b128-r384/double/hostdevice` | 1.39 ms | 1.38 ms | -0.2% | [-0.4%, +0.1%] | +0.0% | ~ | device 1.9x faster than the host |
| `LaunchTuning/csg-tape/firstHit-b128-r0/double/hostdevice` | 2.76 ms | 2.77 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ | device 1.0x faster than the host; default/this 0.50x |
| `LaunchTuning/csg-tape/firstHit-b128-r768/double/hostdevice` | 1.94 ms | 1.95 ms | +0.6% | [-0.3%, +1.6%] | +0.0% | ~ | device 1.3x faster than the host; default/this 0.71x |
| `LaunchTuning/csg-tape/firstHit-b256-r384/double/hostdevice` | 1.56 ms | 1.56 ms | +0.4% | [+0.1%, +0.7%] | +0.0% | ~ | device 1.7x faster than the host; default/this 0.89x |
| `LaunchTuning/cloud/closestPoint-b128-r384/double/hostdevice` | 7.26 ms | 7.27 ms | +0.1% | [-0.3%, +0.5%] | +0.0% | ~ | device 1.2x faster than the host |
| `LaunchTuning/cloud/closestPoint-b128-r0/double/hostdevice` | 7.25 ms | 7.29 ms | +0.4% | [-0.9%, +1.8%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b128-r768/double/hostdevice` | 7.27 ms | 7.25 ms | -0.3% | [-0.5%, -0.0%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `LaunchTuning/cloud/closestPoint-b256-r384/double/hostdevice` | 7.31 ms | 7.29 ms | -0.2% | [-0.7%, +0.2%] | +0.0% | ~ | device 1.2x faster than the host; default/this 1.00x |
| `MeshCSG/union-tape/firstHit-morton/float/hostdevice` | 2.53 ms | 2.52 ms | -0.3% | [-0.7%, +0.2%] | - | ~ | device 4.1x faster than the host |
| `MeshCSG/union-tape/firstHit-random/float/hostdevice` | 4.27 ms | 4.29 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 3.3x faster than the host |
| `MeshCSG/union-tape/frame/float/hostdevice` | 13.8 ms | 13.8 ms | -0.1% | [-0.7%, +0.4%] | - | ~ | device 3.9x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/float/hostdevice` | 63.2 ms | 63.0 ms | -0.2% | [-1.0%, +0.6%] | - | ~ | device 3.2x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/float/hostdevice` | 92.8 ms | 92.6 ms | -0.2% | [-1.5%, +1.0%] | - | ~ | device 2.3x faster than the host |
| `MeshCSG/smooth-tape/frame/float/hostdevice` | 252 ms | 251 ms | -0.5% | [-3.4%, +2.4%] | - | ~ | device 3.6x faster than the host |
| `MeshCSG/smooth-fused/firstHit-morton/float/hostdevice` | 25.1 ms | 24.8 ms | -1.1% | [-1.7%, -0.5%] | - | ~ | device 2.0x faster than the host; tape/fused 2.54x |
| `MeshCSG/smooth-fused/firstHit-random/float/hostdevice` | 26.9 ms | 27.0 ms | +0.3% | [-0.3%, +0.8%] | - | ~ | device 2.2x faster than the host; tape/fused 3.43x |
| `MeshCSG/smooth-fused/frame/float/hostdevice` | 71.0 ms | 71.4 ms | +0.5% | [-0.2%, +1.3%] | - | ~ | device 4.1x faster than the host; tape/fused 3.52x |
| `MeshCSG/union-tape/firstHit-morton/double/hostdevice` | 22.3 ms | 22.4 ms | +0.7% | [-2.4%, +3.9%] | - | ~ | device 0.5x faster than the host |
| `MeshCSG/union-tape/firstHit-random/double/hostdevice` | 44.7 ms | 44.9 ms | +0.3% | [+0.1%, +0.4%] | - | ~ | device 0.3x faster than the host |
| `MeshCSG/union-tape/frame/double/hostdevice` | 94.4 ms | 94.1 ms | -0.3% | [-2.7%, +1.4%] | - | ~ | device 0.6x faster than the host |
| `MeshCSG/smooth-tape/firstHit-morton/double/hostdevice` | 536 ms | 540 ms | +0.8% | [-0.8%, +2.4%] | - | ~ | device 0.4x faster than the host |
| `MeshCSG/smooth-tape/firstHit-random/double/hostdevice` | 861 ms | 848 ms | -1.5% | [-3.5%, +0.6%] | - | ~ | device 0.3x faster than the host (was 0.3x) |
| `MeshCSG/smooth-tape/frame/double/hostdevice` | 2.05 s | 2.04 s | -0.3% | [-2.1%, +1.7%] | - | ~ | device 0.4x faster than the host (was 0.4x) |
| `MeshCSG/smooth-fused/firstHit-morton/double/hostdevice` | 198 ms | 200 ms | +0.7% | [-0.6%, +5.3%] | - | ~ | device 0.2x faster than the host; tape/fused 2.70x |
| `MeshCSG/smooth-fused/firstHit-random/double/hostdevice` | 307 ms | 305 ms | -0.9% | [-6.3%, +5.0%] | - | ~ | device 0.2x faster than the host; tape/fused 2.78x |
| `MeshCSG/smooth-fused/frame/double/hostdevice` | 599 ms | 592 ms | -1.2% | [-4.9%, +2.6%] | - | ~ | device 0.6x faster than the host (was 0.6x); tape/fused 3.45x |
| `MeshReconcile/armadillo/reconcile-scheduled/float/hostdevice` | 2.66 ms | 2.66 ms | +0.1% | [-0.0%, +0.2%] | +0.0% | ~ | device 16.1x faster than the host (was 15.6x) |
| `MeshReconcile/armadillo/reconcile-scheduled/double/hostdevice` | 5.31 ms | 5.31 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 10.4x faster than the host |
| `Ray/mesh/firstHit-morton/float/hostdevice` | 676 µs | 672 µs | -0.5% | [-1.6%, +0.5%] | - | ~ | device 16.8x faster than the host; hand/api 0.97x |
| `Ray/mesh/firstHit-random/float/hostdevice` | 1.28 ms | 1.28 ms | -0.0% | [-0.5%, +0.4%] | - | ~ | device 14.5x faster than the host; hand/api 0.96x |
| `Ray/mesh/firstHit-handrolled-morton/float/hostdevice` | 643 µs | 653 µs | +1.5% | [+0.3%, +2.7%] | - | ~ | device 17.0x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/float/hostdevice` | 1.22 ms | 1.23 ms | +0.4% | [-0.2%, +1.0%] | - | ~ | device 15.1x faster than the host |
| `Ray/mesh/intersects-morton/float/hostdevice` | 611 µs | 617 µs | +1.1% | [-1.0%, +3.3%] | - | ~ | device 15.8x faster than the host |
| `Ray/mesh/intersects-random/float/hostdevice` | 1.12 ms | 1.12 ms | -0.1% | [-0.8%, +0.6%] | - | ~ | device 14.9x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/float/hostdevice` | 1.19 ms | 1.18 ms | -0.4% | [-1.0%, +0.2%] | - | ~ | device 12.1x faster than the host; triangles/polygons 0.57x |
| `Ray/mesh-polygons/firstHit-random/float/hostdevice` | 2.50 ms | 2.47 ms | -0.9% | [-1.2%, -0.7%] | - | ~ | device 9.9x faster than the host; triangles/polygons 0.52x |
| `Ray/mesh-polygons/intersects-morton/float/hostdevice` | 1.07 ms | 1.07 ms | -0.3% | [-1.0%, +0.4%] | - | ~ | device 11.8x faster than the host |
| `Ray/mesh-polygons/intersects-random/float/hostdevice` | 2.18 ms | 2.18 ms | -0.2% | [-0.8%, +0.4%] | - | ~ | device 9.8x faster than the host |
| `Ray/csg-fused/firstHit-morton/float/hostdevice` | 609 µs | 608 µs | -0.1% | [-6.1%, +6.1%] | - | ~ | device 2.5x faster than the host; tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/float/hostdevice` | 660 µs | 723 µs | +9.5% | [-8.5%, +10.0%] | - | ~ | device 2.5x faster than the host (was 2.7x); tape/fused 1.17x (was 1.28x) |
| `Ray/csg-tape/firstHit-morton/float/hostdevice` | 677 µs | 678 µs | +0.2% | [-0.2%, +0.6%] | - | ~ | device 3.7x faster than the host |
| `Ray/csg-tape/firstHit-random/float/hostdevice` | 847 µs | 845 µs | -0.3% | [-0.8%, +0.3%] | - | ~ | device 3.5x faster than the host |
| `Ray/mesh/firstHit-morton/double/hostdevice` | 16.9 ms | 16.9 ms | +0.2% | [-0.5%, +0.8%] | - | ~ | device 0.7x faster than the host; hand/api 1.01x |
| `Ray/mesh/firstHit-random/double/hostdevice` | 27.3 ms | 27.5 ms | +0.4% | [+0.1%, +0.8%] | - | ~ | device 0.7x faster than the host; hand/api 1.00x |
| `Ray/mesh/firstHit-handrolled-morton/double/hostdevice` | 17.0 ms | 17.1 ms | +0.3% | [-0.2%, +0.8%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/firstHit-handrolled-random/double/hostdevice` | 27.4 ms | 27.3 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-morton/double/hostdevice` | 14.8 ms | 15.0 ms | +1.4% | [+0.6%, +2.2%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh/intersects-random/double/hostdevice` | 24.5 ms | 24.5 ms | +0.2% | [-0.5%, +1.0%] | - | ~ | device 0.7x faster than the host |
| `Ray/mesh-polygons/firstHit-morton/double/hostdevice` | 16.6 ms | 16.5 ms | -0.4% | [-1.4%, +0.7%] | - | ~ | device 0.9x faster than the host; triangles/polygons 1.03x |
| `Ray/mesh-polygons/firstHit-random/double/hostdevice` | 24.5 ms | 24.5 ms | -0.1% | [-0.3%, +0.2%] | - | ~ | device 1.1x faster than the host; triangles/polygons 1.12x |
| `Ray/mesh-polygons/intersects-morton/double/hostdevice` | 15.1 ms | 14.9 ms | -1.4% | [-2.5%, -0.1%] | - | ~ | device 0.9x faster than the host |
| `Ray/mesh-polygons/intersects-random/double/hostdevice` | 23.0 ms | 23.1 ms | +0.1% | [-0.8%, +1.0%] | - | ~ | device 1.0x faster than the host |
| `Ray/csg-fused/firstHit-morton/double/hostdevice` | 1.25 ms | 1.25 ms | -0.1% | [-0.4%, +0.2%] | - | ~ | device 1.3x faster than the host; tape/fused 1.12x |
| `Ray/csg-fused/firstHit-random/double/hostdevice` | 2.36 ms | 2.39 ms | +0.9% | [+0.6%, +1.3%] | - | ~ | device 0.8x faster than the host; tape/fused 1.04x |
| `Ray/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.1% | [-0.2%, +0.3%] | - | ~ | device 1.9x faster than the host (was 2.0x) |
| `Ray/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | +0.2% | [-0.2%, +0.6%] | - | ~ | device 1.3x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/float/hostdevice` | 2.54 ms | 2.54 ms | +0.1% | [-0.1%, +0.2%] | - | ~ | device 2.9x faster than the host; marched/traced 3.56x |
| `RayCSG/scene-fused/firstHit-random/float/hostdevice` | 2.99 ms | 3.00 ms | +0.2% | [+0.2%, +0.2%] | - | ~ | device 2.7x faster than the host; marched/traced 4.62x |
| `RayCSG/scene-fused/firstHit-marched-morton/float/hostdevice` | 9.02 ms | 9.03 ms | +0.1% | [-0.0%, +0.3%] | - | ~ | device 4.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/float/hostdevice` | 13.8 ms | 13.9 ms | +0.1% | [-0.0%, +0.2%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/float/hostdevice` | 2.20 ms | 2.20 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 3.9x faster than the host; marched/traced 13.16x |
| `RayCSG/scene-tape/firstHit-random/float/hostdevice` | 5.48 ms | 5.49 ms | +0.1% | [-0.1%, +0.4%] | - | ~ | device 2.8x faster than the host; marched/traced 10.00x |
| `RayCSG/scene-tape/firstHit-marched-morton/float/hostdevice` | 28.9 ms | 28.9 ms | -0.1% | [-0.2%, -0.0%] | - | ~ | device 2.3x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/float/hostdevice` | 55.0 ms | 54.9 ms | -0.3% | [-0.8%, +0.1%] | - | ~ | device 2.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/float/hostdevice` | 5.22 ms | 5.22 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 3.0x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/float/hostdevice` | 8.76 ms | 8.76 ms | -0.0% | [-0.3%, +0.2%] | - | ~ | device 3.2x faster than the host |
| `RayCSG/scene-fused/firstHit-morton/double/hostdevice` | 6.18 ms | 6.15 ms | -0.4% | [-0.6%, -0.3%] | - | ~ | device 1.3x faster than the host; marched/traced 26.28x |
| `RayCSG/scene-fused/firstHit-random/double/hostdevice` | 10.6 ms | 10.6 ms | -0.0% | [-0.4%, +0.4%] | - | ~ | device 0.8x faster than the host; marched/traced 24.39x |
| `RayCSG/scene-fused/firstHit-marched-morton/double/hostdevice` | 159 ms | 162 ms | +1.6% | [+0.4%, +2.9%] | - | ~ | device 0.3x faster than the host |
| `RayCSG/scene-fused/firstHit-marched-random/double/hostdevice` | 256 ms | 257 ms | +0.5% | [-1.5%, +2.6%] | - | ~ | device 0.2x faster than the host |
| `RayCSG/scene-tape/firstHit-morton/double/hostdevice` | 4.87 ms | 4.87 ms | -0.0% | [-0.4%, +0.3%] | - | ~ | device 1.8x faster than the host; marched/traced 23.04x |
| `RayCSG/scene-tape/firstHit-random/double/hostdevice` | 12.3 ms | 12.3 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 1.3x faster than the host; marched/traced 17.44x |
| `RayCSG/scene-tape/firstHit-marched-morton/double/hostdevice` | 113 ms | 112 ms | -0.6% | [-1.9%, +0.6%] | - | ~ | device 0.6x faster than the host |
| `RayCSG/scene-tape/firstHit-marched-random/double/hostdevice` | 215 ms | 214 ms | -0.5% | [-2.7%, +1.7%] | - | ~ | device 0.5x faster than the host |
| `RayCSG/crowd-tape/firstHit-morton/double/hostdevice` | 19.3 ms | 19.4 ms | +0.1% | [-0.4%, +0.6%] | - | ~ | device 0.8x faster than the host |
| `RayCSG/crowd-tape/firstHit-random/double/hostdevice` | 26.7 ms | 26.8 ms | +0.5% | [-0.0%, +1.0%] | - | ~ | device 1.0x faster than the host |
| `RayCrossing/mesh/firstHit-morton/float/hostdevice` | 677 µs | 675 µs | -0.3% | [-1.1%, +0.5%] | - | ~ | device 17.1x faster than the host |
| `RayCrossing/mesh/firstHit-random/float/hostdevice` | 1.29 ms | 1.27 ms | -1.3% | [-1.4%, -1.1%] | - | ~ | device 15.0x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/float/hostdevice` | 669 µs | 675 µs | +0.8% | [+0.1%, +1.4%] | - | ~ | device 17.1x faster than the host; hit/crossing 1.00x |
| `RayCrossing/mesh/firstCrossing-random/float/hostdevice` | 1.28 ms | 1.27 ms | -0.6% | [-1.4%, +0.1%] | - | ~ | device 15.1x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/float/hostdevice` | 726 µs | 728 µs | +0.3% | [-0.1%, +0.7%] | - | ~ | device 3.6x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/float/hostdevice` | 849 µs | 850 µs | +0.1% | [-0.7%, +0.9%] | - | ~ | device 3.5x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/float/hostdevice` | 625 µs | 625 µs | -0.0% | [-0.5%, +0.4%] | - | ~ | device 3.2x faster than the host; hit/crossing 1.17x |
| `RayCrossing/csg-tape/firstCrossing-random/float/hostdevice` | 647 µs | 650 µs | +0.5% | [+0.4%, +0.6%] | - | ~ | device 3.6x faster than the host; hit/crossing 1.31x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/float/hostdevice` | 4.70 ms | 4.69 ms | -0.0% | [-0.2%, +0.1%] | - | ~ | device 10.1x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/float/hostdevice` | 10.2 ms | 10.2 ms | -0.0% | [-0.1%, +0.1%] | - | ~ | device 6.9x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/float/hostdevice` | 4.18 ms | 4.17 ms | -0.1% | [-0.3%, +0.0%] | - | ~ | device 9.9x faster than the host; hit/crossing 1.12x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/float/hostdevice` | 9.08 ms | 9.09 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 6.8x faster than the host; hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/float/hostdevice` | 56.5 ms | 55.4 ms | -1.9% | [-4.6%, +0.9%] | - | ~ | device 3.5x faster than the host (was 3.1x) |
| `RayCrossing/smooth-mesh-tape/firstHit-random/float/hostdevice` | 81.6 ms | 81.7 ms | +0.0% | [-0.9%, +1.0%] | - | ~ | device 2.3x faster than the host (was 2.4x) |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/float/hostdevice` | 47.7 ms | 48.1 ms | +0.8% | [+0.1%, +1.5%] | - | ~ | device 3.5x faster than the host (was 3.0x); hit/crossing 1.15x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/float/hostdevice` | 63.5 ms | 63.6 ms | +0.3% | [-0.1%, +0.7%] | - | ~ | device 2.4x faster than the host; hit/crossing 1.28x |
| `RayCrossing/mesh/firstHit-morton/double/hostdevice` | 17.1 ms | 17.2 ms | +0.8% | [-0.7%, +2.5%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstHit-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.0% | [-0.2%, +0.2%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/mesh/firstCrossing-morton/double/hostdevice` | 17.0 ms | 17.0 ms | +0.1% | [-0.3%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.01x |
| `RayCrossing/mesh/firstCrossing-random/double/hostdevice` | 27.4 ms | 27.4 ms | +0.0% | [-0.4%, +0.5%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.00x |
| `RayCrossing/csg-tape/firstHit-morton/double/hostdevice` | 1.39 ms | 1.39 ms | +0.4% | [+0.1%, +0.7%] | - | ~ | device 2.0x faster than the host |
| `RayCrossing/csg-tape/firstHit-random/double/hostdevice` | 2.49 ms | 2.49 ms | +0.2% | [-0.0%, +0.4%] | - | ~ | device 1.3x faster than the host |
| `RayCrossing/csg-tape/firstCrossing-morton/double/hostdevice` | 1.10 ms | 1.10 ms | -0.2% | [-0.6%, +0.3%] | - | ~ | device 1.9x faster than the host; hit/crossing 1.27x |
| `RayCrossing/csg-tape/firstCrossing-random/double/hostdevice` | 1.16 ms | 1.15 ms | -0.2% | [-0.3%, -0.0%] | - | ~ | device 2.1x faster than the host; hit/crossing 2.16x |
| `RayCrossing/drilled-mesh-tape/firstHit-morton/double/hostdevice` | 64.5 ms | 64.8 ms | +0.5% | [-0.0%, +1.1%] | - | ~ | device 0.7x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstHit-random/double/hostdevice` | 122 ms | 122 ms | +0.1% | [-2.1%, +2.3%] | - | ~ | device 0.6x faster than the host |
| `RayCrossing/drilled-mesh-tape/firstCrossing-morton/double/hostdevice` | 58.4 ms | 58.6 ms | +0.4% | [-0.3%, +1.1%] | - | ~ | device 0.7x faster than the host; hit/crossing 1.11x |
| `RayCrossing/drilled-mesh-tape/firstCrossing-random/double/hostdevice` | 110 ms | 110 ms | -0.3% | [-1.4%, +0.9%] | - | ~ | device 0.6x faster than the host; hit/crossing 1.11x |
| `RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice` | 492 ms | 480 ms | -2.4% | [-6.8%, +3.0%] | - | ~ | device 0.4x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstHit-random/double/hostdevice` | 706 ms | 713 ms | +0.9% | [-1.0%, +2.8%] | - | ~ | device 0.3x faster than the host |
| `RayCrossing/smooth-mesh-tape/firstCrossing-morton/double/hostdevice` | 389 ms | 371 ms | -4.5% | [-7.7%, -1.2%] | - | faster | device 0.4x faster than the host (was 0.4x); hit/crossing 1.29x |
| `RayCrossing/smooth-mesh-tape/firstCrossing-random/double/hostdevice` | 528 ms | 533 ms | +0.9% | [-2.9%, +4.9%] | - | ~ | device 0.3x faster than the host; hit/crossing 1.34x |
| `RayParticles/armadillo-shell-tape/step-morton/float/hostdevice` | 3.55 ms | 3.49 ms | -1.7% | [-2.5%, -0.9%] | - | ~ | device 1.2x faster than the host; marched/traced 3.45x |
| `RayParticles/armadillo-shell-tape/step-random/float/hostdevice` | 4.76 ms | 4.79 ms | +0.5% | [-0.3%, +1.3%] | - | ~ | device 0.9x faster than the host; marched/traced 5.55x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/float/hostdevice` | 12.1 ms | 12.0 ms | -0.5% | [-0.9%, -0.0%] | - | ~ | device 3.9x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/float/hostdevice` | 26.5 ms | 26.5 ms | +0.1% | [+0.0%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/float/hostdevice` | 1.24 ms | 1.27 ms | +2.4% | [+0.8%, +4.0%] | - | ~ | device 2.4x faster than the host |
| `RayParticles/armadillo-shell-fused/step-random/float/hostdevice` | 1.85 ms | 1.86 ms | +0.5% | [-0.2%, +1.2%] | - | ~ | device 1.7x faster than the host |
| `RayParticles/armadillo/step-morton/float/hostdevice` | 94.8 µs | 93.7 µs | -1.2% | [-2.0%, -0.4%] | - | ~ | device 15.6x faster than the host (was 15.0x) |
| `RayParticles/armadillo/step-random/float/hostdevice` | 150 µs | 152 µs | +1.3% | [+1.2%, +1.5%] | - | ~ | device 10.8x faster than the host |
| `RayParticles/box-shell-tape/step-morton/float/hostdevice` | 794 µs | 792 µs | -0.3% | [-0.4%, -0.1%] | - | ~ | device 3.0x faster than the host |
| `RayParticles/box-shell-tape/step-random/float/hostdevice` | 929 µs | 927 µs | -0.2% | [-0.4%, +0.0%] | - | ~ | device 2.7x faster than the host |
| `RayParticles/armadillo-shell-tape/step-morton/double/hostdevice` | 35.1 ms | 34.2 ms | -2.4% | [-5.8%, +1.1%] | - | ~ | device 0.1x faster than the host; marched/traced 3.53x |
| `RayParticles/armadillo-shell-tape/step-random/double/hostdevice` | 36.4 ms | 37.1 ms | +1.9% | [+0.5%, +3.4%] | - | ~ | device 0.1x faster than the host; marched/traced 6.37x |
| `RayParticles/armadillo-shell-tape/step-marched-morton/double/hostdevice` | 122 ms | 121 ms | -0.8% | [-3.2%, +1.5%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/armadillo-shell-tape/step-marched-random/double/hostdevice` | 239 ms | 236 ms | -1.1% | [-1.7%, -0.4%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo-shell-fused/step-morton/double/hostdevice` | 16.7 ms | 16.4 ms | -2.3% | [-6.5%, +2.3%] | - | ~ | device 0.2x faster than the host (was 0.2x) |
| `RayParticles/armadillo-shell-fused/step-random/double/hostdevice` | 21.7 ms | 22.0 ms | +1.2% | [+0.3%, +2.1%] | - | ~ | device 0.2x faster than the host |
| `RayParticles/armadillo/step-morton/double/hostdevice` | 1.06 ms | 1.06 ms | -0.3% | [-1.6%, +1.1%] | - | ~ | device 1.4x faster than the host |
| `RayParticles/armadillo/step-random/double/hostdevice` | 2.34 ms | 2.34 ms | -0.1% | [-2.6%, +2.5%] | - | ~ | device 0.7x faster than the host (was 0.7x) |
| `RayParticles/box-shell-tape/step-morton/double/hostdevice` | 5.97 ms | 5.99 ms | +0.3% | [-0.4%, +0.9%] | - | ~ | device 0.4x faster than the host |
| `RayParticles/box-shell-tape/step-random/double/hostdevice` | 6.69 ms | 6.69 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/float/hostdevice` | 23.2 ms | 23.8 ms | +2.2% | [+2.1%, +2.4%] | - | ~ | device 2.1x faster than the host; marched/traced 2.62x |
| `RayShell/armadillo-tape/firstHit-random/float/hostdevice` | 28.3 ms | 28.4 ms | +0.5% | [-0.8%, +1.8%] | - | ~ | device 1.9x faster than the host; marched/traced 3.67x |
| `RayShell/armadillo-tape/firstHit-marched-morton/float/hostdevice` | 62.4 ms | 62.4 ms | -0.0% | [-0.6%, +0.6%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/float/hostdevice` | 103 ms | 104 ms | +1.0% | [-0.7%, +2.7%] | - | ~ | device 2.4x faster than the host (was 2.3x) |
| `RayShell/armadillo-fused/firstHit-morton/float/hostdevice` | 13.8 ms | 13.7 ms | -0.2% | [-1.3%, +0.8%] | - | ~ | device 3.6x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/float/hostdevice` | 17.3 ms | 17.3 ms | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.2x faster than the host |
| `RayShell/armadillo-tape/firstHit-morton/double/hostdevice` | 305 ms | 299 ms | -1.7% | [-7.7%, +4.6%] | - | ~ | device 0.2x faster than the host; marched/traced 1.92x |
| `RayShell/armadillo-tape/firstHit-random/double/hostdevice` | 345 ms | 348 ms | +0.8% | [-0.7%, +2.4%] | - | ~ | device 0.2x faster than the host; marched/traced 2.86x |
| `RayShell/armadillo-tape/firstHit-marched-morton/double/hostdevice` | 567 ms | 573 ms | +1.1% | [+0.1%, +2.2%] | - | ~ | device 0.4x faster than the host |
| `RayShell/armadillo-tape/firstHit-marched-random/double/hostdevice` | 1.01 s | 995 ms | -1.1% | [-1.6%, -0.5%] | - | ~ | device 0.2x faster than the host (was 0.3x) |
| `RayShell/armadillo-fused/firstHit-morton/double/hostdevice` | 189 ms | 184 ms | -2.6% | [-3.0%, -2.2%] | - | ~ | device 0.3x faster than the host |
| `RayShell/armadillo-fused/firstHit-random/double/hostdevice` | 234 ms | 233 ms | -0.5% | [-1.3%, +0.4%] | - | ~ | device 0.2x faster than the host |
| `Render/csg/frame/float/hostdevice` | 4.26 ms | 4.26 ms | +0.1% | [-0.0%, +0.1%] | - | ~ | device 2.6x faster than the host (was 2.7x) |
| `Render/csg/frame-orthographic/float/hostdevice` | 4.27 ms | 4.26 ms | -0.1% | [-0.4%, +0.1%] | - | ~ | device 2.7x faster than the host |
| `Render/mesh/frame/float/hostdevice` | 1.88 ms | 1.87 ms | -0.3% | [-0.3%, -0.2%] | - | ~ | device 8.9x faster than the host |
| `Render/csg/frame/double/hostdevice` | 7.64 ms | 7.68 ms | +0.6% | [+0.3%, +1.0%] | - | ~ | device 1.7x faster than the host |
| `Render/csg/frame-orthographic/double/hostdevice` | 7.64 ms | 7.64 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 1.6x faster than the host |
| `Render/mesh/frame/double/hostdevice` | 23.5 ms | 23.3 ms | -0.8% | [-2.2%, +0.6%] | - | ~ | device 0.7x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/float/hostdevice` | 884 µs | 882 µs | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ | device 10.6x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/float/hostdevice` | 2.64 ms | 2.64 ms | -0.0% | [-0.2%, +0.1%] | +0.0% | ~ | device 4.8x faster than the host |
| `SphereCloudBVH/polydisperse/build/float/hostdevice` | 13.5 ms | 13.5 ms | -0.1% | [-0.3%, +0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/float/hostdevice` | 10.2 ms | 10.2 ms | -0.0% | [-0.4%, +0.3%] | +0.0% | ~ | device 3.5x faster than the host |
| `SphereCloudBVH/polydisperse/closest-morton/double/hostdevice` | 13.1 ms | 13.0 ms | -0.3% | [-0.5%, -0.2%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereCloudBVH/polydisperse/closest-random/double/hostdevice` | 14.3 ms | 14.4 ms | +0.5% | [+0.2%, +0.8%] | +0.0% | ~ | device 1.0x faster than the host |
| `SphereCloudBVH/polydisperse/build/double/hostdevice` | 14.0 ms | 13.9 ms | -0.1% | [-0.2%, +0.1%] | +0.0% | ~ | device 3.7x faster than the host |
| `SphereCloudBVH/polydisperse/rebuild/double/hostdevice` | 11.0 ms | 11.0 ms | +0.0% | [-0.1%, +0.2%] | +0.0% | ~ | device 3.8x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/float/hostdevice` | 3.00 ms | 3.01 ms | +0.4% | [+0.3%, +0.5%] | +0.0% | ~ | device 7.3x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/float/hostdevice` | 4.74 ms | 4.77 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ | device 5.1x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/float/hostdevice` | 16.7 ms | 16.7 ms | -0.2% | [-0.3%, -0.1%] | +0.0% | ~ | device 5.1x faster than the host; tape/fused 1.38x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/float/hostdevice` | 24.2 ms | 24.1 ms | -0.1% | [-0.3%, +0.0%] | +0.0% | ~ | device 3.8x faster than the host; tape/fused 1.26x |
| `SphereCloudRays/bed-in-box/firstHit-morton/float/hostdevice` | 22.9 ms | 22.9 ms | +0.0% | [+0.0%, +0.1%] | +0.0% | ~ | device 3.8x faster than the host (was 4.2x) |
| `SphereCloudRays/bed-in-box/firstHit-random/float/hostdevice` | 30.3 ms | 30.4 ms | +0.0% | [-0.0%, +0.1%] | +0.0% | ~ | device 3.0x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-morton/double/hostdevice` | 44.4 ms | 44.3 ms | -0.2% | [-0.7%, +0.3%] | +0.0% | ~ | device 0.5x faster than the host |
| `SphereCloudRays/polydisperse/firstHit-random/double/hostdevice` | 45.5 ms | 45.6 ms | +0.3% | [-0.3%, +0.8%] | +0.0% | ~ | device 0.6x faster than the host |
| `SphereCloudRays/bed-in-box-fused/firstHit-morton/double/hostdevice` | 125 ms | 121 ms | -3.5% | [-4.6%, -2.4%] | +0.0% | faster | device 0.7x faster than the host (was 0.8x); tape/fused 1.04x |
| `SphereCloudRays/bed-in-box-fused/firstHit-random/double/hostdevice` | 127 ms | 126 ms | -0.8% | [-3.0%, +1.5%] | +0.0% | ~ | device 0.8x faster than the host; tape/fused 1.05x |
| `SphereCloudRays/bed-in-box/firstHit-morton/double/hostdevice` | 131 ms | 126 ms | -3.5% | [-4.6%, -2.4%] | +0.0% | faster | device 0.7x faster than the host (was 0.7x) |
| `SphereCloudRays/bed-in-box/firstHit-random/double/hostdevice` | 133 ms | 132 ms | -0.9% | [-1.1%, -0.7%] | +0.0% | ~ | device 0.8x faster than the host |
| `SphereContacts/polydisperse/force-morton/float/hostdevice` | 1.59 ms | 1.59 ms | +0.1% | [-0.1%, +0.3%] | +0.0% | ~ | device 10.7x faster than the host |
| `SphereContacts/polydisperse/force-morton/double/hostdevice` | 31.1 ms | 31.1 ms | -0.0% | [-0.0%, +0.0%] | +0.0% | ~ | device 0.6x faster than the host |
| `Tape/csg/signedDistance/float/hostdevice` | 218 µs | 218 µs | +0.1% | [+0.1%, +0.2%] | - | ~ | device 11.3x faster than the host |
| `Tape/csg-handwritten/signedDistance/float/hostdevice` | 47.8 µs | 47.8 µs | +0.1% | [-0.3%, +0.5%] | - | ~ | device 23.9x faster than the host |
| `Tape/smooth/signedDistance/float/hostdevice` | 360 µs | 359 µs | -0.1% | [-0.4%, +0.2%] | - | ~ | device 12.8x faster than the host |
| `Tape/union64/signedDistance/float/hostdevice` | 1.56 ms | 1.56 ms | -0.3% | [-0.4%, -0.1%] | - | ~ | device 9.1x faster than the host |
| `Tape/sphere/signedDistance/float/hostdevice` | 72.3 µs | 72.2 µs | -0.2% | [-0.7%, +0.3%] | - | ~ | device 7.1x faster than the host |
| `Tape/sphere-handwritten/signedDistance/float/hostdevice` | 47.4 µs | 47.5 µs | +0.2% | [-0.2%, +0.7%] | - | ~ | device 4.7x faster than the host |
| `Tape/sphere/signedDistance-x16/float/hostdevice` | 651 µs | 651 µs | +0.0% | [-0.0%, +0.0%] | - | ~ | device 10.5x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/float/hostdevice` | 133 µs | 133 µs | -0.1% | [-0.2%, +0.0%] | - | ~ | device 12.1x faster than the host |
| `Tape/chain16/signedDistance/float/hostdevice` | 477 µs | 476 µs | -0.1% | [-0.3%, +0.0%] | - | ~ | device 7.5x faster than the host |
| `Tape/chain16-handwritten/signedDistance/float/hostdevice` | 48.6 µs | 48.3 µs | -0.6% | [-2.1%, +0.9%] | - | ~ | device 26.4x faster than the host |
| `Tape/csg/signedDistance/double/hostdevice` | 858 µs | 858 µs | +0.0% | [-0.1%, +0.2%] | - | ~ | device 3.0x faster than the host (was 3.1x) |
| `Tape/csg-handwritten/signedDistance/double/hostdevice` | 810 µs | 811 µs | +0.1% | [-0.0%, +0.3%] | - | ~ | device 1.5x faster than the host |
| `Tape/smooth/signedDistance/double/hostdevice` | 1.74 ms | 1.74 ms | +0.1% | [+0.1%, +0.1%] | - | ~ | device 2.8x faster than the host |
| `Tape/union64/signedDistance/double/hostdevice` | 6.65 ms | 6.65 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance/double/hostdevice` | 192 µs | 192 µs | -0.1% | [-0.3%, +0.1%] | - | ~ | device 3.0x faster than the host |
| `Tape/sphere-handwritten/signedDistance/double/hostdevice` | 163 µs | 163 µs | -0.0% | [-0.2%, +0.2%] | - | ~ | device 2.0x faster than the host |
| `Tape/sphere/signedDistance-x16/double/hostdevice` | 2.78 ms | 2.78 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |
| `Tape/sphere-handwritten/signedDistance-x16/double/hostdevice` | 2.50 ms | 2.50 ms | +0.0% | [-0.0%, +0.1%] | - | ~ | device 0.8x faster than the host |
| `Tape/chain16/signedDistance/double/hostdevice` | 2.12 ms | 2.12 ms | -0.1% | [-0.2%, +0.1%] | - | ~ | device 1.8x faster than the host |
| `Tape/chain16-handwritten/signedDistance/double/hostdevice` | 2.10 ms | 2.10 ms | +0.0% | [+0.0%, +0.0%] | - | ~ | device 0.9x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/float/hostdevice` | 1.61 ms | 1.61 ms | +0.1% | [+0.0%, +0.1%] | - | ~ | device 8.1x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/float/hostdevice` | 3.70 ms | 3.71 ms | +0.2% | [-0.1%, +0.4%] | - | ~ | device 10.8x faster than the host |
| `TapeBVHUnion/maps64-bvh/signedDistance/double/hostdevice` | 6.51 ms | 6.50 ms | -0.1% | [-0.2%, +0.0%] | - | ~ | device 2.0x faster than the host |
| `TapeBVHUnion/maps64-as-written/signedDistance/double/hostdevice` | 17.8 ms | 17.8 ms | -0.0% | [-0.0%, +0.0%] | - | ~ | device 2.4x faster than the host |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 3514.6 s → 3509.1 s (-0.2%), the largest peak 5633 MB → 5632 MB; 1 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 12.1 s | 12.1 s | -0.1% | 537 MB | 540 MB | +0.4% | ~ |
| `BVHRefit` | 6.1 s | 6.1 s | -0.9% | 469 MB | 470 MB | +0.3% | ~ |
| `BVHUnion` | 19.9 s | 23.6 s | +18.6% | 857 MB | 964 MB | +12.4% | **minor** |
| `BVHUnionMixed` | 10.2 s | 10.2 s | -0.2% | 561 MB | 563 MB | +0.4% | ~ |
| `ClosestPrimitive` | 127.5 s | 128.5 s | +0.8% | 1234 MB | 1237 MB | +0.2% | ~ |
| `ClosestSurface` | - | 184.2 s | | - | 1766 MB | | new |
| `Fused` | 48.6 s | 49.1 s | +0.9% | 1045 MB | 1047 MB | +0.1% | ~ |
| `LaunchTuning` | 499.2 s | 506.5 s | +1.5% | 3807 MB | 3806 MB | -0.0% | ~ |
| `Mesh` | 18.8 s | 19.2 s | +2.5% | 741 MB | 742 MB | +0.2% | ~ |
| `MeshCSG` | 353.6 s | 356.0 s | +0.7% | 5102 MB | 5107 MB | +0.1% | ~ |
| `MeshReconcile` | 9.0 s | 9.0 s | -0.1% | 469 MB | 470 MB | +0.3% | ~ |
| `PointCloud` | 11.6 s | 11.6 s | -0.1% | 506 MB | 508 MB | +0.3% | ~ |
| `PointCloudBVHBuild` | 6.3 s | 6.2 s | -2.2% | 469 MB | 470 MB | +0.3% | ~ |
| `PointCloudHashGridBuild` | 5.5 s | 5.5 s | -0.1% | 469 MB | 470 MB | +0.3% | ~ |
| `Ray` | 186.0 s | 185.3 s | -0.3% | 3704 MB | 3703 MB | -0.0% | ~ |
| `RayCSG` | 216.6 s | 219.8 s | +1.5% | 4085 MB | 4086 MB | +0.0% | ~ |
| `RayCrossing` | 421.2 s | 419.7 s | -0.4% | 4019 MB | 4019 MB | +0.0% | ~ |
| `RayParticles` | 624.1 s | 623.3 s | -0.1% | 5633 MB | 5632 MB | -0.0% | ~ |
| `RayShell` | 400.1 s | 377.2 s | -5.7% | 5346 MB | 5346 MB | -0.0% | ~ |
| `Render` | 210.1 s | 213.8 s | +1.8% | 4939 MB | 4941 MB | +0.0% | ~ |
| `Shapes` | 25.6 s | 27.1 s | +5.8% | 526 MB | 527 MB | +0.3% | ~ |
| `SphereCloudBVH` | 8.0 s | 7.9 s | -1.3% | 469 MB | 471 MB | +0.3% | ~ |
| `SphereCloudRays` | 179.5 s | 178.2 s | -0.7% | 4096 MB | 4096 MB | -0.0% | ~ |
| `SphereContacts` | 7.4 s | 7.0 s | -5.1% | 469 MB | 470 MB | +0.3% | ~ |
| `Tape` | 65.0 s | 63.2 s | -2.8% | 868 MB | 870 MB | +0.3% | ~ |
| `TapeBVHUnion` | 42.4 s | 42.8 s | +1.0% | 809 MB | 811 MB | +0.3% | ~ |
| **all, in both** | 3514.6 s | 3509.1 s | -0.2% | 5633 MB | 5632 MB | -0.0% | ~ |

Device kernels: 156 in both, 4 changed (0 flagged: registers changed, spills grew, or the stack grew by more than 10% and 256 bytes), 10 new, 0 gone. Spills are bytes stored/loaded per thread.

| unit | kernel | registers | stack | spills | flag |
|---|---|---:|---:|---:|---|
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double, TriMeshSDF<double, 4ul…` | 168 → 168 | 19088 → 19104 | 1048/1072 → 1048/1072 | ~ |
| `MeshCSG` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float, TriMeshSDF<float, 4ul, 4…` | 168 → 168 | 13072 → 13088 | 720/736 → 720/736 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<double, Tape<double> >, unsigned int const,…` | 168 → 168 | 18704 → 18720 | 1052/1076 → 1052/1076 | ~ |
| `Render` | `void parallelForKernel<128u, 384u, Apply<Pixel<float, Tape<float> >, unsigned int const, f…` | 168 → 168 | 13280 → 13296 | 728/740 → 728/740 | ~ |


Flagged after 2 rounds, and judged on 10 (4 cases): MeshCSG/union-tape/frame/double/hostdevice, MeshCSG/smooth-fused/firstHit-morton/double/hostdevice, Ray/csg-fused/firstHit-random/float/hostdevice, RayCrossing/smooth-mesh-tape/firstHit-morton/double/hostdevice

These probes compiled to the same code on both sides and were not run (88 cases): BVH (12), Mesh (8), PointCloud (40), PointCloudBVHBuild (4), PointCloudHashGridBuild (4), Shapes (20)

The probes differ between the sides (each side ran its own): Performance/ClosestPrimitive/main.cpp, Performance/ClosestSurface/main.cpp

<!-- END receipt cuda -->
