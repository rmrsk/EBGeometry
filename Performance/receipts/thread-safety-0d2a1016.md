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
base: 0a4fbcc
base-fingerprint: a2c485fc369aca048e00b4520e1a13072b6fe671
head: 08654e9
head-fingerprint: 0d2a1016339f8ad9cee0f0b14a125cbc4fb4b6b8
cases: 164
rounds: 2
reps: 3
date: 2026-11-23
-->

## Host comparison: green

**Accepted:** -

`08654e9` against `0a4fbcc` (0a4fbcc), 2026-11-23.

**VERDICT: no regression (0 major, 0 minor across 164 cases)**

Performance comparison: 0 major, 0 minor, 6 faster, 0 differ, 0 lost, 164 cases

- base: 0a4fbcc (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- head: 08654e9 (gcc 13.3.0, simd avx2, gpu host, allocator glibc, heap only, freed memory returned before each repetition)
- CPU: 12th Gen Intel(R) Core(TM) i9-12900K
- Thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

| case | base | head | change | 95% interval | pool | verdict | note |
|---|---:|---:|---:|---:|---:|---|---|
| `BVH/mesh-boxes/ray-nearest/float/host` | 338 ms | 336 ms | -0.6% | [-0.9%, -0.4%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/float/host` | 179 ms | 181 ms | +1.1% | [+0.6%, +1.5%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/float/host` | 521 ms | 517 ms | -0.9% | [-1.1%, -0.6%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-nearest/double/host` | 350 ms | 350 ms | +0.0% | [-0.9%, +0.9%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/ray-any/double/host` | 187 ms | 188 ms | +0.6% | [+0.1%, +1.1%] | +0.0% | ~ |  |
| `BVH/mesh-boxes/point-nearest/double/host` | 564 ms | 565 ms | +0.1% | [-0.7%, +1.0%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/float/host` | 19.7 ms | 19.4 ms | -1.6% | [-1.8%, -1.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/float/host` | 41.3 ms | 41.3 ms | -0.1% | [-1.0%, +0.8%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/float/host` | 73.5 ms | 73.0 ms | -0.7% | [-1.0%, -0.5%] | +0.0% | ~ |  |
| `ClosestPrimitive/csg/closestPrimitive/double/host` | 20.2 ms | 20.7 ms | +2.7% | [+1.6%, +4.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/parts/closestPrimitive/double/host` | 44.7 ms | 44.0 ms | -1.5% | [-2.1%, -0.9%] | +0.0% | ~ |  |
| `ClosestPrimitive/drilled-mesh/closestPrimitive/double/host` | 82.2 ms | 81.9 ms | -0.3% | [-1.3%, +0.6%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/float/host` | 64.1 ms | 64.5 ms | +0.6% | [-0.6%, +1.8%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/float/host` | 60.5 ms | 60.6 ms | +0.1% | [-1.5%, +1.8%] | +0.0% | ~ | grid/morton 1.06x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/float/host` | 79.0 ms | 79.2 ms | +0.3% | [-0.6%, +1.2%] | +0.0% | ~ | grid/shuffled 0.81x |
| `EBGrid/TriMeshSDF/armadillo/distance-grid/double/host` | 68.4 ms | 69.4 ms | +1.4% | [-0.2%, +3.1%] | +0.0% | ~ |  |
| `EBGrid/TriMeshSDF/armadillo/distance-morton/double/host` | 64.3 ms | 64.4 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/TriMeshSDF/armadillo/distance-shuffled/double/host` | 90.2 ms | 89.6 ms | -0.7% | [-1.6%, +0.2%] | +0.0% | ~ | grid/shuffled 0.77x |
| `EBGrid/MeshSDF/armadillo/distance-grid/float/host` | 200 ms | 197 ms | -1.7% | [-5.3%, +2.1%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/float/host` | 187 ms | 183 ms | -1.9% | [-5.8%, +2.2%] | +0.0% | ~ | grid/morton 1.08x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/float/host` | 262 ms | 262 ms | +0.4% | [-0.5%, +1.3%] | +0.0% | ~ | grid/shuffled 0.75x |
| `EBGrid/MeshSDF/armadillo/distance-grid/double/host` | 208 ms | 207 ms | -0.2% | [-1.7%, +1.3%] | +0.0% | ~ |  |
| `EBGrid/MeshSDF/armadillo/distance-morton/double/host` | 193 ms | 195 ms | +1.0% | [-2.4%, +4.6%] | +0.0% | ~ | grid/morton 1.07x |
| `EBGrid/MeshSDF/armadillo/distance-shuffled/double/host` | 315 ms | 311 ms | -1.2% | [-10.0%, +6.1%] | +0.0% | ~ | grid/shuffled 0.67x |
| `Fused/csg/signedDistance/float/host` | 4.53 ms | 4.49 ms | -0.9% | [-1.6%, -0.3%] | - | ~ |  |
| `Fused/smooth/signedDistance/float/host` | 12.4 ms | 12.8 ms | +2.6% | [-1.8%, +7.0%] | - | ~ |  |
| `Fused/chain16/signedDistance/float/host` | 970 µs | 984 µs | +1.4% | [+0.4%, +2.4%] | - | ~ |  |
| `Fused/drilled-mesh/signedDistance/float/host` | 46.8 ms | 46.4 ms | -0.9% | [-2.5%, +0.8%] | +0.0% | ~ | tape/fused 1.00x |
| `Fused/drilled-mesh-tape/signedDistance/float/host` | 46.9 ms | 46.6 ms | -0.6% | [-1.7%, +0.6%] | +0.0% | ~ |  |
| `Fused/csg/signedDistance/double/host` | 4.62 ms | 4.61 ms | -0.3% | [-0.4%, -0.2%] | - | ~ |  |
| `Fused/smooth/signedDistance/double/host` | 12.9 ms | 12.8 ms | -1.2% | [-1.7%, -0.6%] | - | ~ |  |
| `Fused/chain16/signedDistance/double/host` | 3.91 ms | 3.90 ms | -0.2% | [-2.2%, +1.8%] | - | ~ |  |
| `Fused/drilled-mesh/signedDistance/double/host` | 53.4 ms | 53.5 ms | +0.2% | [-1.2%, +1.6%] | +0.0% | ~ | tape/fused 1.01x |
| `Fused/drilled-mesh-tape/signedDistance/double/host` | 53.9 ms | 53.9 ms | +0.0% | [-1.2%, +1.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/float/host` | 83.9 ms | 83.6 ms | -0.4% | [-1.8%, +1.1%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/float/host` | 233 ms | 234 ms | +0.1% | [-2.3%, +2.5%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/build/double/host` | 115 ms | 114 ms | -1.1% | [-1.6%, -0.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w4/query-random/double/host` | 325 ms | 269 ms | -17.1% | [-30.4%, +2.2%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/build/float/host` | 72.3 ms | 73.5 ms | +1.6% | [-1.4%, +4.7%] | +0.0% | ~ |  |
| `TriMeshSDF/w8/query-random/float/host` | 225 ms | 219 ms | -2.8% | [-3.9%, -1.8%] | +0.0% | ~ |  |
| `MeshSDF/build/float/host` | 99.6 ms | 123 ms | +23.8% | [-2.1%, +50.0%] | +0.0% | ~ |  |
| `MeshSDF/refit/float/host` | 29.7 ms | 46.9 ms | +57.9% | [-4.2%, +122.7%] | - | ~ |  |
| `MeshSDF/query-random/float/host` | 787 ms | 859 ms | +9.2% | [-0.1%, +18.7%] | +0.0% | ~ |  |
| `MeshSDF/build/double/host` | 154 ms | 151 ms | -1.8% | [-37.2%, +54.7%] | +0.0% | ~ |  |
| `MeshSDF/refit/double/host` | 57.9 ms | 124 ms | +114.1% | [-41.5%, +383.7%] | - | ~ |  |
| `MeshSDF/query-random/double/host` | 933 ms | 882 ms | -5.5% | [-12.4%, +2.5%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/firstHit/float/host` | 26.3 ms | 28.9 ms | +9.6% | [-0.3%, +19.7%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/float/host` | 15.6 ms | 15.8 ms | +1.2% | [-2.5%, +2.8%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/float/host` | 324 ms | 331 ms | +2.1% | [-1.1%, +5.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/float/host` | 235 ms | 237 ms | +0.9% | [-0.5%, +7.4%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/float/host` | 91.4 ms | 91.4 ms | -0.0% | [-2.0%, +2.0%] | +0.0% | ~ | tape/fused 3.62x |
| `MeshCSG/smooth-fused/frame/float/host` | 84.1 ms | 82.0 ms | -2.4% | [-3.3%, -1.4%] | +0.0% | ~ | tape/fused 2.89x (was 2.79x) |
| `MeshCSG/union-tape/firstHit/double/host` | 29.2 ms | 29.4 ms | +0.5% | [-1.6%, +2.6%] | +0.0% | ~ |  |
| `MeshCSG/union-tape/frame/double/host` | 20.2 ms | 17.3 ms | -14.4% | [-26.1%, +1.3%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/firstHit/double/host` | 368 ms | 375 ms | +1.6% | [-3.2%, +6.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-tape/frame/double/host` | 313 ms | 332 ms | +6.2% | [-34.8%, +68.7%] | +0.0% | ~ |  |
| `MeshCSG/smooth-fused/firstHit/double/host` | 113 ms | 99.7 ms | -12.1% | [-24.4%, +4.1%] | +0.0% | ~ | tape/fused 3.76x (was 3.25x) |
| `MeshCSG/smooth-fused/frame/double/host` | 87.8 ms | 87.2 ms | -0.7% | [-1.7%, +0.4%] | +0.0% | ~ | tape/fused 3.80x (was 3.56x) |
| `MeshClosestPoint/tri/signedDistance/float/host` | 236 ms | 238 ms | +0.7% | [-2.6%, +4.1%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/float/host` | 281 ms | 234 ms | -16.9% | [-26.7%, -4.2%] | +0.0% | faster | signedDistance/closestPoint 1.02x (was 0.84x) |
| `MeshClosestPoint/mesh/closestPoint/float/host` | 497 ms | 478 ms | -3.8% | [-43.0%, +48.8%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/float/host` | 330 ms | 332 ms | +0.7% | [-44.5%, +82.3%] | +0.0% | ~ | closestPoint/gradient 0.70x (was 0.85x) |
| `MeshClosestPoint/tri/signedDistance/double/host` | 363 ms | 351 ms | -3.5% | [-43.0%, +63.8%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/closestPoint/double/host` | 300 ms | 347 ms | +15.7% | [-20.3%, +61.0%] | +0.0% | ~ | signedDistance/closestPoint 1.01x (was 1.21x) |
| `MeshClosestPoint/mesh/closestPoint/double/host` | 359 ms | 347 ms | -3.4% | [-9.6%, +3.4%] | +0.0% | ~ |  |
| `MeshClosestPoint/tri/gradient/double/host` | 294 ms | 268 ms | -8.8% | [-17.4%, +1.6%] | +0.0% | ~ | closestPoint/gradient 1.30x (was 1.02x) |
| `MeshIngest/armadillo/read-soup/float/host` | 53.4 ms | 54.4 ms | +1.9% | [+1.6%, +2.2%] | - | ~ |  |
| `MeshIngest/armadillo/clean/float/host` | 12.7 ms | 13.0 ms | +2.0% | [-2.7%, +6.8%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/float/host` | 97.3 ms | 96.3 ms | -1.0% | [-3.0%, +1.0%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/float/host` | 103 ms | 104 ms | +0.3% | [-1.3%, +1.9%] | - | ~ |  |
| `MeshIngest/armadillo/read-soup/double/host` | 54.7 ms | 54.9 ms | +0.5% | [-0.1%, +1.1%] | - | ~ |  |
| `MeshIngest/armadillo/clean/double/host` | 13.6 ms | 13.8 ms | +1.7% | [-0.6%, +4.1%] | - | ~ |  |
| `MeshIngest/armadillo/read-dcel/double/host` | 107 ms | 107 ms | -0.3% | [-1.4%, +0.7%] | +0.0% | ~ |  |
| `MeshIngest/armadillo/read-triangles/double/host` | 117 ms | 115 ms | -0.9% | [-1.5%, -0.3%] | - | ~ |  |
| `MeshReconcile/armadillo/reconcile/float/host` | 74.3 ms | 76.0 ms | +2.3% | [-0.6%, +5.2%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/float/host` | 41.7 ms | 43.0 ms | +2.9% | [+0.8%, +5.2%] | +0.0% | ~ | reconcile/scheduled 1.77x |
| `MeshReconcile/armadillo/reconcile/double/host` | 96.3 ms | 98.9 ms | +2.6% | [-1.4%, +6.8%] | +0.0% | ~ |  |
| `MeshReconcile/armadillo/reconcile-scheduled/double/host` | 57.1 ms | 56.4 ms | -1.1% | [-2.9%, +0.8%] | +0.0% | ~ | reconcile/scheduled 1.75x (was 1.69x) |
| `MeshTape/mesh/firstHit/float/host` | 48.9 ms | 48.8 ms | -0.2% | [-1.5%, +1.1%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/float/host` | 54.1 ms | 48.9 ms | -9.6% | [-16.9%, -1.1%] | +0.0% | faster |  |
| `MeshTape/mesh/frame/float/host` | 20.7 ms | 20.5 ms | -1.3% | [-6.8%, +4.7%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/float/host` | 81.3 ms | 82.3 ms | +1.3% | [-1.3%, +4.0%] | +0.0% | ~ | mesh/tape 0.59x |
| `MeshTape/tape/firstCrossing/float/host` | 56.3 ms | 52.0 ms | -7.6% | [-15.8%, +2.3%] | +0.0% | ~ | mesh/tape 0.94x |
| `MeshTape/tape/frame/float/host` | 24.2 ms | 24.5 ms | +0.9% | [-3.2%, +5.2%] | +0.0% | ~ | mesh/tape 0.84x |
| `MeshTape/view/firstHit/float/host` | 84.1 ms | 84.2 ms | +0.1% | [-0.7%, +0.9%] | +0.0% | ~ | mesh/view 0.58x |
| `MeshTape/view/frame/float/host` | 26.4 ms | 26.6 ms | +0.9% | [-2.2%, +4.0%] | +0.0% | ~ | mesh/view 0.77x |
| `MeshTape/mesh/firstHit/double/host` | 54.1 ms | 55.4 ms | +2.3% | [-18.4%, +18.4%] | +0.0% | ~ |  |
| `MeshTape/mesh/firstCrossing/double/host` | 53.2 ms | 54.0 ms | +1.5% | [-0.4%, +3.5%] | +0.0% | ~ |  |
| `MeshTape/mesh/frame/double/host` | 20.5 ms | 20.4 ms | -0.3% | [-0.6%, +0.0%] | +0.0% | ~ |  |
| `MeshTape/tape/firstHit/double/host` | 88.0 ms | 89.0 ms | +1.1% | [+0.5%, +1.8%] | +0.0% | ~ | mesh/tape 0.62x |
| `MeshTape/tape/firstCrossing/double/host` | 56.9 ms | 55.8 ms | -2.0% | [-3.3%, -0.7%] | +0.0% | ~ | mesh/tape 0.97x (was 0.93x) |
| `MeshTape/tape/frame/double/host` | 26.7 ms | 26.7 ms | +0.3% | [-0.8%, +1.4%] | +0.0% | ~ | mesh/tape 0.76x |
| `MeshTape/view/firstHit/double/host` | 93.7 ms | 94.2 ms | +0.6% | [-5.7%, +7.0%] | +0.0% | ~ | mesh/view 0.59x |
| `MeshTape/view/frame/double/host` | 29.9 ms | 29.5 ms | -1.4% | [-3.3%, +0.5%] | +0.0% | ~ | mesh/view 0.69x |
| `QueryOrder/random/mortonOrder/float/host` | 62.3 ms | 61.7 ms | -0.9% | [-3.2%, +1.3%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/float/host` | 226 ms | 227 ms | +0.4% | [-1.4%, +2.1%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/float/host` | 224 ms | 212 ms | -5.5% | [-43.4%, +59.8%] | +0.0% | ~ | arrived/ordered 1.07x (was 1.01x) |
| `QueryOrder/random/mortonOrder/double/host` | 77.3 ms | 78.2 ms | +1.1% | [-30.8%, +47.6%] | - | ~ |  |
| `QueryOrder/armadillo/distance-arrived/double/host` | 357 ms | 408 ms | +14.3% | [-38.5%, +98.6%] | +0.0% | ~ |  |
| `QueryOrder/armadillo/distance-ordered/double/host` | 228 ms | 228 ms | -0.0% | [-38.9%, +64.0%] | +0.0% | ~ | arrived/ordered 1.79x (was 1.57x) |
| `Ray/mesh/firstHit/float/host` | 60.4 ms | 64.1 ms | +6.1% | [-31.3%, +61.3%] | +0.0% | ~ | hand/api 0.97x |
| `Ray/mesh/firstHit-handrolled/float/host` | 59.2 ms | 62.3 ms | +5.2% | [-36.5%, +71.3%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/float/host` | 52.1 ms | 49.5 ms | -5.0% | [-36.3%, +43.3%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/float/host` | 118 ms | 89.4 ms | -24.3% | [-47.1%, +19.9%] | +0.0% | ~ | triangles/polygons 0.72x (was 0.51x) |
| `Ray/mesh-polygons/intersects/float/host` | 74.9 ms | 68.3 ms | -8.9% | [-20.1%, +5.5%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/float/host` | 5.32 ms | 5.17 ms | -2.8% | [-5.7%, +0.3%] | - | ~ | tape/fused 1.50x |
| `Ray/csg-tape/firstHit/float/host` | 8.00 ms | 7.76 ms | -3.1% | [-5.1%, -1.1%] | +0.0% | faster |  |
| `Ray/mesh/firstHit/double/host` | 53.8 ms | 54.4 ms | +1.0% | [+0.1%, +2.0%] | +0.0% | ~ | hand/api 0.95x |
| `Ray/mesh/firstHit-handrolled/double/host` | 53.0 ms | 51.9 ms | -2.0% | [-3.0%, -0.9%] | +0.0% | ~ |  |
| `Ray/mesh/intersects/double/host` | 45.7 ms | 45.7 ms | -0.0% | [-1.8%, +1.9%] | +0.0% | ~ |  |
| `Ray/mesh-polygons/firstHit/double/host` | 101 ms | 91.7 ms | -9.4% | [-18.2%, +1.5%] | +0.0% | ~ | triangles/polygons 0.59x (was 0.53x) |
| `Ray/mesh-polygons/intersects/double/host` | 76.5 ms | 81.6 ms | +6.6% | [-1.3%, +14.6%] | +0.0% | ~ |  |
| `Ray/csg-fused/firstHit/double/host` | 5.51 ms | 5.57 ms | +1.0% | [-0.4%, +2.4%] | - | ~ | tape/fused 1.43x |
| `Ray/csg-tape/firstHit/double/host` | 7.89 ms | 7.98 ms | +1.1% | [+0.6%, +1.6%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstHit/float/host` | 50.4 ms | 50.3 ms | -0.2% | [-5.4%, +5.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/float/host` | 50.1 ms | 51.5 ms | +2.9% | [+1.0%, +4.8%] | +0.0% | ~ | hit/crossing 0.98x |
| `RayCrossing/csg-tape/firstHit/float/host` | 7.63 ms | 7.62 ms | -0.0% | [-1.9%, +1.9%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/float/host` | 6.24 ms | 6.29 ms | +0.7% | [-1.1%, +2.6%] | +0.0% | ~ | hit/crossing 1.21x |
| `RayCrossing/drilled-mesh-tape/firstHit/float/host` | 184 ms | 183 ms | -0.8% | [-4.0%, +2.5%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/float/host` | 165 ms | 158 ms | -4.0% | [-6.8%, -1.0%] | +0.0% | faster | hit/crossing 1.16x (was 1.12x) |
| `RayCrossing/smooth-mesh-tape/firstHit/float/host` | 69.7 ms | 70.9 ms | +1.6% | [-0.1%, +3.3%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/float/host` | 59.7 ms | 58.3 ms | -2.3% | [-6.8%, +2.5%] | +0.0% | ~ | hit/crossing 1.22x (was 1.17x) |
| `RayCrossing/mesh/firstHit/double/host` | 57.9 ms | 53.8 ms | -7.1% | [-14.4%, +1.2%] | +0.0% | ~ |  |
| `RayCrossing/mesh/firstCrossing/double/host` | 73.9 ms | 54.4 ms | -26.4% | [-39.3%, -7.8%] | +0.0% | faster | hit/crossing 0.99x (was 0.78x) |
| `RayCrossing/csg-tape/firstHit/double/host` | 9.98 ms | 10.0 ms | +0.5% | [-36.7%, +59.0%] | +0.0% | ~ |  |
| `RayCrossing/csg-tape/firstCrossing/double/host` | 8.55 ms | 7.91 ms | -7.4% | [-34.5%, +26.1%] | +0.0% | ~ | hit/crossing 1.27x (was 1.17x) |
| `RayCrossing/drilled-mesh-tape/firstHit/double/host` | 202 ms | 206 ms | +2.3% | [-5.8%, +43.2%] | +0.0% | ~ |  |
| `RayCrossing/drilled-mesh-tape/firstCrossing/double/host` | 178 ms | 184 ms | +3.4% | [-0.4%, +7.3%] | +0.0% | ~ | hit/crossing 1.12x |
| `RayCrossing/smooth-mesh-tape/firstHit/double/host` | 83.9 ms | 83.4 ms | -0.7% | [-14.1%, +15.0%] | +0.0% | ~ |  |
| `RayCrossing/smooth-mesh-tape/firstCrossing/double/host` | 66.7 ms | 65.7 ms | -1.5% | [-5.5%, +2.8%] | +0.0% | ~ | hit/crossing 1.27x |
| `RayParticles/armadillo-shell-tape/step/float/host` | 12.3 ms | 11.7 ms | -5.3% | [-10.4%, -0.1%] | +0.0% | faster | marched/traced 5.34x (was 4.73x) |
| `RayParticles/armadillo-shell-tape/step-marched/float/host` | 58.3 ms | 62.4 ms | +6.9% | [-15.4%, +34.4%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/float/host` | 8.90 ms | 8.74 ms | -1.8% | [-7.3%, +3.9%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/float/host` | 2.05 ms | 2.05 ms | +0.1% | [-1.4%, +1.6%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/float/host` | 7.14 ms | 6.99 ms | -2.1% | [-3.5%, -0.7%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-tape/step/double/host` | 12.3 ms | 11.9 ms | -3.7% | [-17.8%, +3.5%] | +0.0% | ~ | marched/traced 5.36x (was 5.05x) |
| `RayParticles/armadillo-shell-tape/step-marched/double/host` | 62.4 ms | 63.7 ms | +2.1% | [-2.4%, +6.8%] | +0.0% | ~ |  |
| `RayParticles/armadillo-shell-fused/step/double/host` | 8.87 ms | 8.77 ms | -1.1% | [-3.6%, +1.5%] | +0.0% | ~ |  |
| `RayParticles/armadillo/step/double/host` | 2.15 ms | 2.04 ms | -4.7% | [-9.4%, +0.4%] | +0.0% | ~ |  |
| `RayParticles/box-shell-tape/step/double/host` | 7.78 ms | 7.63 ms | -2.0% | [-7.1%, +3.1%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/float/host` | 251 ms | 249 ms | -0.7% | [-3.6%, +2.3%] | +0.0% | ~ | marched/traced 1.40x |
| `RayShell/armadillo-tape/firstHit-marched/float/host` | 346 ms | 349 ms | +0.8% | [-5.0%, +7.0%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/float/host` | 240 ms | 235 ms | -1.9% | [-2.5%, -1.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-tape/firstHit/double/host` | 257 ms | 256 ms | -0.1% | [-1.1%, +0.9%] | +0.0% | ~ | marched/traced 1.47x |
| `RayShell/armadillo-tape/firstHit-marched/double/host` | 380 ms | 377 ms | -0.6% | [-1.6%, +0.4%] | +0.0% | ~ |  |
| `RayShell/armadillo-fused/firstHit/double/host` | 256 ms | 254 ms | -0.6% | [-1.3%, +0.2%] | +0.0% | ~ |  |
| `Render/csg/frame/float/host` | 12.5 ms | 12.6 ms | +0.6% | [-7.9%, +10.0%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/float/host` | 13.2 ms | 12.9 ms | -1.8% | [-3.6%, +0.0%] | +0.0% | ~ |  |
| `Render/mesh/frame/float/host` | 20.1 ms | 19.7 ms | -1.6% | [-3.5%, +0.3%] | +0.0% | ~ |  |
| `Render/csg/frame/double/host` | 14.2 ms | 14.2 ms | -0.0% | [-1.4%, +1.4%] | +0.0% | ~ |  |
| `Render/csg/frame-orthographic/double/host` | 13.9 ms | 13.7 ms | -1.2% | [-2.2%, -0.2%] | +0.0% | ~ |  |
| `Render/mesh/frame/double/host` | 20.5 ms | 20.6 ms | +0.5% | [+0.1%, +1.0%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestSurface/float/host` | 144 ms | 145 ms | +0.4% | [-0.2%, +1.0%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/float/host` | 90.2 ms | 90.3 ms | +0.2% | [-0.8%, +1.1%] | +0.0% | ~ | surface/leaf 1.60x |
| `SurfaceParts/mesh-groups/closestSurface/float/host` | 133 ms | 131 ms | -1.1% | [-2.4%, +0.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/float/host` | 72.7 ms | 73.6 ms | +1.3% | [-0.1%, +2.7%] | +0.0% | ~ | surface/leaf 1.79x |
| `SurfaceParts/mesh-groups/firstSurface/float/host` | 62.2 ms | 61.3 ms | -1.4% | [-2.6%, -0.3%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/float/host` | 34.4 ms | 34.9 ms | +1.6% | [-0.4%, +3.6%] | +0.0% | ~ | surface/hit 1.76x |
| `SurfaceParts/boxes/closestSurface/double/host` | 153 ms | 148 ms | -3.6% | [-8.4%, +1.6%] | +0.0% | ~ |  |
| `SurfaceParts/boxes/closestLeaf/double/host` | 93.7 ms | 91.2 ms | -2.6% | [-3.6%, -1.7%] | +0.0% | ~ | surface/leaf 1.62x |
| `SurfaceParts/mesh-groups/closestSurface/double/host` | 148 ms | 146 ms | -1.4% | [-1.8%, -0.9%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/closestLeaf/double/host` | 88.3 ms | 85.5 ms | -3.1% | [-8.2%, +2.4%] | +0.0% | ~ | surface/leaf 1.70x |
| `SurfaceParts/mesh-groups/firstSurface/double/host` | 70.9 ms | 69.1 ms | -2.6% | [-4.8%, -0.2%] | +0.0% | ~ |  |
| `SurfaceParts/mesh-groups/firstHit/double/host` | 39.2 ms | 39.1 ms | -0.2% | [-1.7%, +1.4%] | +0.0% | ~ | surface/hit 1.77x |

### Build costs: Probe builds (Release)

Machine: 16 cores, 24 hardware threads, 63 GB. Compiled on hardware threads 0,2,4,6,8,10,12,14 (one per fast core), 8 at a time.

Reported, not judged: these flags do not enter the verdict. CPU time of each compile (user and system, every process the compiler ran) and its peak memory; minor at 10% more, MAJOR at 25% (and at least 2 s or 100 MB). Total CPU 764.3 s → 777.7 s (+1.8%), the largest peak 1421 MB → 1419 MB; 0 unit(s) flagged.

| unit | CPU base | CPU head | change | peak base | peak head | change | flag |
|---|---:|---:|---:|---:|---:|---:|---|
| `BVH` | 9.6 s | 9.7 s | +0.6% | 557 MB | 563 MB | +1.0% | ~ |
| `BVHRefit` | 4.3 s | 4.4 s | +3.3% | 501 MB | 503 MB | +0.3% | ~ |
| `BVHUnion` | 19.2 s | 19.7 s | +3.0% | 953 MB | 955 MB | +0.2% | ~ |
| `BVHUnionMixed` | 7.7 s | 7.7 s | +0.2% | 558 MB | 559 MB | +0.2% | ~ |
| `ClosestPrimitive` | 30.4 s | 31.0 s | +1.8% | 1341 MB | 1348 MB | +0.6% | ~ |
| `ClosestSurface` | 27.9 s | 28.1 s | +0.5% | 1314 MB | 1318 MB | +0.3% | ~ |
| `ConnectedComponents` | 15.5 s | 15.6 s | +0.5% | 847 MB | 854 MB | +0.8% | ~ |
| `Domain` | 17.9 s | 18.2 s | +1.6% | 854 MB | 860 MB | +0.7% | ~ |
| `DomainObject` | 15.8 s | 16.0 s | +1.6% | 855 MB | 860 MB | +0.6% | ~ |
| `EBGrid` | 14.5 s | 14.8 s | +1.9% | 783 MB | 783 MB | +0.0% | ~ |
| `Fused` | 24.8 s | 24.8 s | +0.0% | 1229 MB | 1236 MB | +0.6% | ~ |
| `LaunchTuning` | 36.6 s | 37.7 s | +2.8% | 1326 MB | 1328 MB | +0.2% | ~ |
| `Mesh` | 15.8 s | 15.9 s | +0.8% | 789 MB | 791 MB | +0.2% | ~ |
| `MeshCSG` | 39.5 s | 41.0 s | +3.8% | 1278 MB | 1306 MB | +2.3% | ~ |
| `MeshClosestPoint` | 14.2 s | 14.3 s | +0.7% | 785 MB | 786 MB | +0.2% | ~ |
| `MeshIngest` | 7.0 s | 7.2 s | +2.1% | 568 MB | 569 MB | +0.1% | ~ |
| `MeshReconcile` | 6.8 s | 6.9 s | +0.5% | 569 MB | 569 MB | +0.0% | ~ |
| `MeshTape` | 41.7 s | 42.2 s | +1.1% | 1268 MB | 1259 MB | -0.7% | ~ |
| `PointCloud` | 7.5 s | 7.6 s | +0.5% | 561 MB | 562 MB | +0.2% | ~ |
| `PointCloudBVHBuild` | 3.9 s | 3.9 s | +0.6% | 474 MB | 476 MB | +0.4% | ~ |
| `PointCloudHashGridBuild` | 3.2 s | 3.3 s | +1.5% | 455 MB | 457 MB | +0.5% | ~ |
| `QueryOrder` | 11.5 s | 11.4 s | -0.6% | 700 MB | 708 MB | +1.1% | ~ |
| `Ray` | 36.6 s | 36.7 s | +0.3% | 1344 MB | 1342 MB | -0.2% | ~ |
| `RayCSG` | 32.0 s | 32.6 s | +1.8% | 1211 MB | 1208 MB | -0.2% | ~ |
| `RayCrossing` | 47.4 s | 47.2 s | -0.5% | 1351 MB | 1360 MB | +0.7% | ~ |
| `RayParticles` | 50.1 s | 51.0 s | +1.9% | 1323 MB | 1334 MB | +0.8% | ~ |
| `RayShell` | 37.7 s | 38.9 s | +3.4% | 1282 MB | 1319 MB | +2.9% | ~ |
| `Regions` | 29.4 s | 29.8 s | +1.5% | 1086 MB | 1095 MB | +0.8% | ~ |
| `Render` | 36.0 s | 37.3 s | +3.8% | 1291 MB | 1300 MB | +0.7% | ~ |
| `Shapes` | 8.9 s | 10.2 s | +14.6% | 557 MB | 558 MB | +0.2% | ~ |
| `SphereCloudBVH` | 4.8 s | 5.3 s | +10.6% | 515 MB | 518 MB | +0.5% | ~ |
| `SphereCloudRays` | 28.2 s | 29.0 s | +2.8% | 1074 MB | 1077 MB | +0.3% | ~ |
| `SphereContacts` | 4.3 s | 4.3 s | -0.0% | 488 MB | 490 MB | +0.4% | ~ |
| `SurfaceParts` | 41.4 s | 42.0 s | +1.3% | 1421 MB | 1419 MB | -0.1% | ~ |
| `Tape` | 17.2 s | 17.2 s | +0.1% | 830 MB | 837 MB | +0.8% | ~ |
| `TapeBVHUnion` | 15.0 s | 14.9 s | -0.2% | 834 MB | 839 MB | +0.6% | ~ |
| **all, in both** | 764.3 s | 777.7 s | +1.8% | 1421 MB | 1419 MB | -0.1% | ~ |


Flagged after 2 rounds, and judged on 10 (8 cases): ClosestPrimitive/csg/closestPrimitive/double/host, EBGrid/MeshSDF/armadillo/distance-shuffled/double/host, MeshCSG/union-tape/frame/float/host, MeshCSG/smooth-tape/frame/float/host, MeshTape/mesh/firstHit/double/host, QueryOrder/armadillo/distance-arrived/float/host, RayCrossing/drilled-mesh-tape/firstHit/double/host, RayParticles/armadillo-shell-tape/step/double/host

These probes compiled to the same code on both sides and were not run (168 cases): BVHRefit (2), BVHUnion (12), BVHUnionMixed (4), ClosestSurface (8), ConnectedComponents (4), Domain (6), DomainObject (8), PointCloud (40), PointCloudBVHBuild (2), PointCloudHashGridBuild (2), RayCSG (10), Regions (12), Shapes (20), SphereCloudBVH (6), SphereCloudRays (6), SphereContacts (2), Tape (20), TapeBVHUnion (4)

<!-- END receipt host -->
